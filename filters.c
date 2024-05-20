#include "filters.h"

buffer* input;
buffer* filterOutput;


#define TEST 40000

void setupFilters()
{
	input = malloc(sizeof(buffer));
	if (input==NULL)
	{
		printf("Error\n");
		return;
	}

	input->index = 0;
	input->size = ORDER+1;
	input->gain = 0;
	input->gainOutputRight = 0;
	input->gainOutputLeft = 0;

	for(int i = 0; i<=ORDER; i++)
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

	for(int nmr = 0; nmr<AMOUNT; nmr++)
	{
		(filterOutput+nmr)->index = 0;
		(filterOutput+nmr)->size = ORDER+1;
		(filterOutput+nmr)->gain = 1;
		(filterOutput+nmr)->gainOutputRight = 0;
		(filterOutput+nmr)->gainOutputLeft = 0;
		for(int i = 0; i<=ORDER; i++)
		{
			(filterOutput+nmr)->bufferR[i] = 0.0f;
			(filterOutput+nmr)->bufferL[i] = 0.0f;
		}
	}

	printf("succesfull init\n");
}

void updateInput()
{
	s32 in_right = Xil_In32(I2S_DATA_RX_R_REG)<<8;
	s32 in_left = Xil_In32(I2S_DATA_RX_L_REG)<<8;

	int index = input->index;
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

void updateOutput(float valueR, float valueL, int nmr)
{
	buffer* outputPtr = filterOutput+nmr;
	outputPtr->index = (outputPtr->index + 1) % outputPtr->size;
	outputPtr->bufferR[outputPtr->index] = valueR;
	outputPtr->bufferL[outputPtr->index] = valueL;
}

void outputData(float out_right, float out_left)
{
	Xil_Out32(I2S_DATA_TX_R_REG, ((s32)out_right)>>8);
	Xil_Out32(I2S_DATA_TX_L_REG, ((s32)out_left)>>8);
}

void FIR(float* terms, int nmr)
{
	int index = input->index;
	float sumR = 0;
	float sumL = 0;
	for(int i=0; i<=ORDER; i++)
	{
			int idx = (index - i + input->size) % input->size; // Correct circular buffer indexing
			sumR += input->bufferR[idx] * terms[i];
			sumL += input->bufferL[idx] * terms[i];
	}

	updateOutput(sumR, sumL, nmr);
}

void IIR(float* num, float* den, int nmr)
{
	int indexIn = input->index;
	int indexOut = filterOutput->index;
	float sumR = 0, sumL = 0;


	for(int i=0; i<=ORDER; i++)
	{
		int idx = (indexIn - i + input->size) % input->size; // Correct circular buffer indexing
		sumR += input->bufferR[idx] * num[i];
		sumL += input->bufferL[idx] * num[i];
	}

	for(int i = 0; i<ORDER; i++)
	{
		int idx = (indexOut - i + filterOutput->size) % filterOutput->size; // Correct circular buffer indexing
		sumR -= filterOutput->bufferR[idx] * den[i+1];
		sumL -= filterOutput->bufferL[idx] * den[i+1];
	}

	updateOutput(sumR, sumL, nmr);
}

void dBToFloat (int dB, int nmr)
{
	float gain = 10^(dB/20);
	(filterOutput+nmr)->gain = gain;
}

void gain(int nmr)
{
	buffer* outputPtr = filterOutput+nmr;
	int index = outputPtr->index;
	float gain = outputPtr->gain;

	outputPtr->gainOutputRight = outputPtr->bufferR[index]*gain;
	outputPtr->gainOutputLeft = outputPtr->bufferL[index]*gain;
}
