#ifndef FILTERS_H_
#define FILTERS_H_

#include "defines.h"

#include "xparameters.h"
#include "platform.h"
#include "xil_io.h"
#include "stdlib.h"
#include <stdio.h>
#include "audio.h"

void setupFilters();
void updateInput();

void FIR(double* terms, u8 bandNmr);
void IIR(double* numerator, double* denumerator, u8 bandNmr);
void IIR2(double* num, double* den, double* num2, double* den2, u8 nmr);
void dBToFloat(s16 dB, u8 bandNmr);
void adder();


//====== INSTRUCTIONS ======
//Put timerStart() after a new audio signal has been detected
//Put timerEnd() after your function has completed all processes

void timerStart();
void timerEnd();


#endif
