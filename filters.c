#include "filters.h"
#include "math.h"

#define SIZE (ORDER+1)

typedef struct buffer
{
	u8 index;
	double bufferR[SIZE];
	double bufferL[SIZE];
	double gain;
	double gainOutputRight;
	double gainOutputLeft;
} buffer;


buffer* input;
buffer* filterOutput;
buffer* bandpassAdd;

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
	input->gain = 1;
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

	input->bufferR[index] = (double)in_right;
	input->bufferL[index] = (double)in_left;
}

void updateOutput(double valueR, double valueL, u8 nmr)
{
	buffer* output = (filterOutput+nmr);
	output->index = (output->index + 1) % (ORDER+1);

	output->bufferR[output->index] = valueR;
	output->bufferL[output->index] = valueL;
}

void outputData(double out_right, double out_left)
{
	Xil_Out32(I2S_DATA_TX_R_REG, ((s32)out_right)>>8);
	Xil_Out32(I2S_DATA_TX_L_REG, ((s32)out_left)>>8);
}

void FIR(double* terms, u8 nmr)
{
	u8 index = input->index;
	double sumR = 0;
	double sumL = 0;
	for(u8 i=0; i<=ORDER; i++)
	{
			u8 idx = (index - i + SIZE) % SIZE; // Correct circular buffer indexing
			sumR += input->bufferR[idx] * terms[i];
			sumL += input->bufferL[idx] * terms[i];
	}

	updateOutput(sumR, sumL, nmr);
	gain(1);
}

void IIR(double* num, double* den, u8 nmr)
{
	buffer* output = (filterOutput+nmr);
	u8 indexIn = input->index;
	u8 indexOut = output->index;
	double sumR = 0, sumL = 0;


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


void IIR2(double* num, double* den, double* num2, double* den2, u8 nmr)
{
	buffer* output = (bandpassAdd+0);
	u8 indexIn = input->index;
	u8 indexOut = output->index;
	double sumR = 0, sumL = 0;


	for(u8 i=0; i<=ORDER; i++)	// x[n] first filter
	{
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

	for(u8 i = 0; i<ORDER; i++)	// y[n] first filter
	{
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
	// update output of first filter
	output->index = (output->index + 1) % (ORDER+1);
	output->bufferR[output->index] = sumR;
	output->bufferL[output->index] = sumL;


	// second filter
	output = (bandpassAdd+1);
	buffer* in = (bandpassAdd+0);
	indexIn = in->index;
	indexOut = output->index;
	sumR = 0;
	sumL = 0;

	for(u8 i=0; i<=ORDER; i++)	//x[n] second filter
	{
		sumR += in->bufferR[indexIn] * num2[i];
		sumL += in->bufferL[indexIn] * num2[i];
			if(indexIn>0)
		{
			indexIn--;
		}
		else
		{
			indexIn = ORDER;
		}
	}

	for(u8 i = 0; i<ORDER; i++)	//y[n] second filter
	{
		sumR -= output->bufferR[indexOut] * den2[i+1];
		sumL -= output->bufferL[indexOut] * den2[i+1];
			if(indexOut>0)
		{
			indexOut--;
		}
		else
		{
			indexOut = ORDER;
		}
	}
	// update output of first filter
	output->index = (output->index + 1) % (ORDER+1);
	output->bufferR[output->index] = sumR;
	output->bufferL[output->index] = sumL;

	//update output
	updateOutput(sumR, sumL, nmr);
	(filterOutput+nmr)->gainOutputRight = sumR*(filterOutput+nmr)->gain;
	(filterOutput+nmr)->gainOutputLeft = sumL*(filterOutput+nmr)->gain;
}


void dBToFloat (s16 dB, u8 nmr)	//dB needs to be multiplied with 10
{
	if (dB > 120) {dB = 120;}
	else if (dB < -120) {dB = -120;}
	double gain = (double)pow(10,(dB/200.0));
	//printf("gain = %.2f\n", gain);
	(filterOutput+nmr)->gain = gain;
}

void gain(u8 nmr)
{
	buffer* outputPtr = filterOutput+nmr;
	u8 index = outputPtr->index;
	double gain = outputPtr->gain;

	outputPtr->gainOutputRight = outputPtr->bufferR[index]*gain;
	outputPtr->gainOutputLeft = outputPtr->bufferL[index]*gain;
}

void adder()
{
	double sumR = 0, sumL = 0;
	for(u8 i = 0; i<AMOUNT; i++)
	{
		sumR += (filterOutput+i)->gainOutputRight;
		sumL += (filterOutput+i)->gainOutputLeft;
	}

	outputData(sumR, sumL);
}
