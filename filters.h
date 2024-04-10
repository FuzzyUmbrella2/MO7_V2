#ifndef FILTERS_H_
#define FILTERS_H_

#include "defines.h"

typedef struct buffer
{
	int index;
	double bufferR[ORDER];
	double bufferL[ORDER];
} buffer;

void setupFilters();
void updateInput();
void outputData();
void regular();

void FIR(double* terms);
void IIR();

#endif
