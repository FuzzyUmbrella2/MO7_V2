#ifndef FILTERS_H_
#define FILTERS_H_

#include "defines.h"

typedef struct buffer
{
	int index;
	int size;
	float bufferR[ORDER+1];
	float bufferL[ORDER+1];
	float gain;
	float gainOutputRight;
	float gainOutputLeft;
} buffer;

void setupFilters();
void updateInput();
void outputData(float out_right, float out_left);

void FIR(float* terms, int nmr);
void IIR(float* numerator, float* denumerator, int nmr);
void dBToFloat(int dB, int nmr);
void gain(int bandNmr);


//====== INSTRUCTIONS ======
//Put timerStart() after a new audio signal has been detected
//Put timerEnd() after your function has completed all processes

void timerStart();
void timerEnd();


#endif
