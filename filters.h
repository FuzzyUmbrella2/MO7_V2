#ifndef FILTERS_H_
#define FILTERS_H_

#include "defines.h"

void setupFilters();
void updateInput();

void FIR(float* terms, u8 bandNmr);
void IIR(float* numerator, float* denumerator, u8 bandNmr);
void dBToFloat(s16 dB, u8 bandNmr);
void adder();


//====== INSTRUCTIONS ======
//Put timerStart() after a new audio signal has been detected
//Put timerEnd() after your function has completed all processes

void timerStart();
void timerEnd();


#endif
