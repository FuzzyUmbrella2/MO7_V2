#ifndef FILTERS_H_
#define FILTERS_H_

#include "defines.h"

typedef struct buffer
{
	int index;
	int size;
	float bufferR[ORDER+1];
	float bufferL[ORDER+1];
	int dBgain;
	float gain;
} buffer;

void setupFilters();
void updateInput();
void outputData();
void regular();

void FIR(float* terms);
void IIR(float* numerator, float* denumerator);
float* gain(int dB_gain, int bandNmr);


//====== INSTRUCTIONS ======
//Put timerStart() after a new audio signal has been detected
//Put timerEnd() after your function has completed all processes

#define samples 100 //change this number to change the amount of times the timer will run

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xtime_l.h"

// #define CPS 0x135F1B40
#define CPuS 325

void timerStart();
void timerEnd();


#endif
