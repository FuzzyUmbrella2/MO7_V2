#include "filters.h"

buffer* input;
buffer* output;


#define TEST 40000

s32 data[TEST];
u32 dataIndex;
int done;

void setupFilters()
{
	input = malloc(sizeof(buffer));
	if (input==NULL)
	{
		printf("Error\n");
		return;
	}

	output = malloc(AMOUNT*sizeof(buffer));
	if (output==NULL)
	{
		printf("Error\n");
		return;
	}

	input->index = 0;
	output->index = 0;

	for(int i = 0; i<ORDER; i++)
	{
		input->bufferR[i] = 0.0f;
		input->bufferL[i] = 0.0f;
		output->bufferR[i] = 0.0f;
		output->bufferL[i] = 0.0f;
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

	input->bufferR[index] = (double)in_right;
	input->bufferL[index] = (double)in_left;
}

void updateOutput(double valueR, double valueL)
{
	int indexOut = output->index;

	if(indexOut+1>ORDER)
	{
		indexOut = 0;
		output->index=0;
	}
	else
	{
		indexOut++;
		output->index++;
	}

	output->bufferR[indexOut] = valueR;
	output->bufferL[indexOut] = valueL;
}

void outputData()
{
	int index = output->index;

	if(output->bufferR[index]>(2^32))
	{
		printf("Clipping\n");
	}

	s32 out_right = (s32)(output->bufferR[index]);
	s32 out_left = (s32)(output->bufferL[index]);

	Xil_Out32(I2S_DATA_TX_R_REG, out_right>>8);
	Xil_Out32(I2S_DATA_TX_L_REG, out_left>>8);
}

void regular()
{
	int index = input->index;
	updateOutput(input->bufferR[index], input->bufferL[index]);
}

void FIR(double* terms)
{
	int index = input->index;
	double sumR = 0;
	double sumL = 0;
	double newR, newL;
	for(int i=0; i<ORDER; i++)
	{
		if((index+i)<ORDER)
		{
			newR = input->bufferR[index+i]*terms[i];
			newL = input->bufferL[index+i]*terms[i];
		}
		else
		{
			newR = input->bufferR[index+i-ORDER]*terms[i];
			newL = input->bufferL[index+i-ORDER]*terms[i];
		}
		sumR = sumR+newR;
		sumL = sumL+newL;
	}

	updateOutput(sumR, sumL);
}

void IIR(float* termsA, float* termsB)
{
	int indexIn = input->index;
	int indexOut = output->index;
	float sumR, sumL, newR, newL;

	sumR = input->bufferR[indexIn]*termsA[0];
	sumL = input->bufferL[indexIn]*termsA[0];

	for(int i=1; i<ORDER; i++)
	{
		if((indexIn-i)>=0)
		{
			newR = input->bufferR[indexIn-i]*termsA[i];
			newL = input->bufferL[indexIn-i]*termsA[i];
		}
		else
		{
			newR = input->bufferR[ORDER-i]*termsA[i];
			newL = input->bufferL[ORDER-i]*termsA[i];
		}

		sumR = sumR+newR;
		sumL = sumL+newL;


		if ((indexOut-i)>=0)
		{
			newR = output->bufferR[indexOut-i]*termsB[i-1];
			newL = output->bufferL[indexOut-i]*termsB[i-1];
		}
		else
		{
			newR = output->bufferR[ORDER-i]*termsB[i-1];
			newL = output->bufferL[ORDER-i]*termsB[i-1];
		}

		sumR = sumR-newR;
		sumL = sumL-newL;
	}
}
