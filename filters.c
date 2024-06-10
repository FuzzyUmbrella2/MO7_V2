#include "filters.h"
#include "math.h"

#define SIZE (ORDER+1)

typedef struct buffer
{
	u8 index;
	float bufferR[SIZE];
	float bufferL[SIZE];
	float gain;
	float gainOutputRight;
	float gainOutputLeft;
} buffer;


buffer* input;
buffer* filterOutput;

void gain(u8 bandNmr);

void setupFilters()
{
	input = malloc(sizeof(buffer));
	if (input==NULL)
	{
		printf("Error\n");
		return;
	}

	input->index = 0;
	input->gain = 0;
	input->gainOutputRight = 0;
	input->gainOutputLeft = 0;

	for(u8 i = 0; i<=ORDER; i++)
	{
		input->bufferR[i] = 0.0f;
		input->bufferL[i] = 0.0f;
	}



	filterOutput = malloc(AMOUNT*sizeof(buffer));
	if (filterOutput==NULL)
	{
		printf("Error\n");
		return;
	}

	for(u8 nmr = 0; nmr<AMOUNT; nmr++)
	{
		(filterOutput+nmr)->index = 0;
		(filterOutput+nmr)->gain = 1;
		(filterOutput+nmr)->gainOutputRight = 0;
		(filterOutput+nmr)->gainOutputLeft = 0;
		for(u8 i = 0; i<=ORDER; i++)
		{
			(filterOutput+nmr)->bufferR[i] = 0.0f;
			(filterOutput+nmr)->bufferL[i] = 0.0f;
		}
	}

	bandpassAdd = malloc(2*sizeof(buffer));
	if (bandpassAdd==NULL)
	{
		printf("Error\n");
		return;
	}

	for(u8 nmr = 0; nmr<2; nmr++)
	{
		(bandpassAdd+nmr)->index = 0;
		(bandpassAdd+nmr)->gain = 1;
		(bandpassAdd+nmr)->gainOutputRight = 0;
		(bandpassAdd+nmr)->gainOutputLeft = 0;
		for(u8 i = 0; i<=ORDER; i++)
		{
			(bandpassAdd+nmr)->bufferR[i] = 0.0f;
			(bandpassAdd+nmr)->bufferL[i] = 0.0f;
		}
	}

	printf("succesfull init\n");
}

void updateInput()
{
	s32 in_right = Xil_In32(I2S_DATA_RX_R_REG)<<8;
	s32 in_left = Xil_In32(I2S_DATA_RX_L_REG)<<8;

	u8 index = input->index;
	if(index+1>ORDER)
	{
		index = 0;
		input->index=0;
	}
	else
	{
		index++;
		input->index++;
	}

	input->bufferR[index] = (float)in_right;
	input->bufferL[index] = (float)in_left;
}

void updateOutput(float valueR, float valueL, u8 nmr)
{
	buffer* output = (filterOutput+nmr);
	output->index = (output->index + 1) % (ORDER+1);

	output->bufferR[output->index] = valueR;
	output->bufferL[output->index] = valueL;
}

void outputData(float out_right, float out_left)
{
	Xil_Out32(I2S_DATA_TX_R_REG, ((s32)out_right)>>8);
	Xil_Out32(I2S_DATA_TX_L_REG, ((s32)out_left)>>8);
}

void FIR(float* terms, u8 nmr)
{
	u8 index = input->index;
	float sumR = 0;
	float sumL = 0;
	for(u8 i=0; i<=ORDER; i++)
	{
			u8 idx = (index - i + SIZE) % SIZE; // Correct circular buffer indexing
			sumR += input->bufferR[idx] * terms[i];
			sumL += input->bufferL[idx] * terms[i];
	}

	updateOutput(sumR, sumL, nmr);
	gain(1);
}

void IIR(float* num, float* den, u8 nmr)
{
	buffer* output = (filterOutput+nmr);
	u8 indexIn = input->index;
	u8 indexOut = output->index;
	float sumR = 0, sumL = 0;


	for(u8 i=0; i<=ORDER; i++)
	{
		//u8 indexIn= (indexIn - 1 + SIZE) % SIZE; // Correct circular buffer indexing
		sumR += input->bufferR[indexIn] * num[i];
		sumL += input->bufferL[indexIn] * num[i];

		if(indexIn>0)
		{
			indexIn--;
		}
		else
		{
			indexIn = ORDER;
		}
	}

	for(u8 i = 0; i<ORDER; i++)
	{
		//u8 idx = (indexOut - i + SIZE) % SIZE; // Correct circular buffer indexing
		sumR -= output->bufferR[indexOut] * den[i+1];
		sumL -= output->bufferL[indexOut] * den[i+1];

		if(indexOut>0)
		{
			indexOut--;
		}
		else
		{
			indexOut = ORDER;
		}
	}
	updateOutput(sumR, sumL, nmr);
	gain(nmr);
}

void dBToFloat (s16 dB, u8 nmr)	//dB needs to be multiplied with 10
{
	float gain = (float)pow(10,(dB/100.0));
	(filterOutput+nmr)->gain = gain;
}

void gain(u8 nmr)
{
	buffer* outputPtr = filterOutput+nmr;
	u8 index = outputPtr->index;
	float gain = outputPtr->gain;

	outputPtr->gainOutputRight = outputPtr->bufferR[index]*gain;
	outputPtr->gainOutputLeft = outputPtr->bufferL[index]*gain;
}

void adder()
{
	float sumR = 0, sumL = 0;
	for(u8 i = 0; i<AMOUNT; i++)
	{
		sumR += (filterOutput+i)->gainOutputRight;
		sumL += (filterOutput+i)->gainOutputLeft;
	}

	outputData(sumR, sumL);
}
