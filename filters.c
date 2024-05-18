#include "filters.h"

buffer* input;
buffer* output;


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
	input->dBgain = 0;
	input->gain = 0;

	for(int i = 0; i<=ORDER; i++)
	{
		input->bufferR[i] = 0.0f;
		input->bufferL[i] = 0.0f;
	}



	output = malloc(/*AMOUNT**/sizeof(buffer));
	if (output==NULL)
	{
		printf("Error\n");
		return;
	}

	output->index = 0;
	output->size = ORDER+1;
	output->dBgain = 0;
	output->gain = 0;

	for(int i = 0; i<=ORDER; i++)
	{
		output->bufferR[i] = 0.0f;
		output->bufferL[i] = 0.0f;
	}

/*	for(int nmr = 0; nmr<AMOUNT; nmr++)
	{
		(output+nmr)->index = 0;
		(output+nmr)->size = ORDER+1;
		(output+nmr)->dBgain = 0;
		(output+nmr)->gain = 0;
		for(int i = 0; i<=ORDER; i++)
		{
			(output+nmr)->bufferR[i] = 0.0f;
			(output+nmr)->bufferL[i] = 0.0f;
		}
	}*/

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

void updateOutput(float valueR, float valueL)
{
	output->index = (output->index + 1) % output->size;

	output->bufferR[output->index] = valueR;
	output->bufferL[output->index] = valueL;
}

void outputData()
{
	int index = output->index;

/*	if(output->bufferR[index] > INT32_MAX)
	{
		printf("Clipping\n");
	}*/

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

void FIR(float* terms)
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

	updateOutput(sumR, sumL);
}

void IIR(float* num, float* den)
{
	int indexIn = input->index;
	int indexOut = output->index;
	float sumR = 0, sumL = 0;


	for(int i=0; i<=ORDER; i++)
	{
		int idx = (indexIn - i + input->size) % input->size; // Correct circular buffer indexing
		sumR += input->bufferR[idx] * num[i];
		sumL += input->bufferL[idx] * num[i];
	}

	for(int i = 0; i<ORDER; i++)
	{
		int idx = (indexOut - i + output->size) % output->size; // Correct circular buffer indexing
		sumR -= output->bufferR[idx] * den[i+1];
		sumL -= output->bufferL[idx] * den[i+1];
	}

	updateOutput(sumR, sumL);
}

float* gain(int dBgain, int nmr)
{
	int index = (output+nmr)->index;
	float gain = 0;
	float outGain[2];
	float* ptr = outGain;

	if(dBgain!=(output+nmr)->dBgain)
	{
		gain = 10^(dBgain/20);
		(output+nmr)->dBgain = dBgain;
		(output+nmr)->gain = gain;
	}

	outGain[0]= ((output+nmr)->bufferR[index])*gain;
	outGain[1] = ((output+nmr)->bufferL[index])*gain;

	return ptr;
}


XTime tStart = 0, tEnd = 0;
XTime summedTime = 0;
u32 iteration = 0;
const u32 maxIterations = samples+2;
float finalAnswer;


void timerStart() {
	if (iteration < samples) {
		XTime_GetTime(&tStart);
	}
}

void timerEnd() {
	if (iteration < maxIterations) {
		if (iteration < samples) {//taking another sample
				XTime_GetTime(&tEnd);
				summedTime = summedTime + (u32)(tEnd - tStart); //summing function
				//printf("read: %d\n", ((int)(tEnd - tStart) / CPuS));
		}
		else if (iteration == samples) { //calculating
			finalAnswer = (float)summedTime / (float)samples / (float)CPuS;

		}
		else { //printing (not done at the same time as calculating due to performance)
			printf("process takes %f us\n",finalAnswer);
		}
		iteration++;
	}
}
