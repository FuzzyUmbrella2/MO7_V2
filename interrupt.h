#ifndef __INTERRUPT_H_
#define __INTERRUPT_H_

#include "xil_printf.h"
#include "xil_io.h"
#include "stdlib.h"
#include <stdio.h>
#include "sleep.h"
#include "xparameters.h"
#include "filters.h"
#include "SH1106_Screen.h"

#define GIC_ID			XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TIMER0_ID		XPAR_TMRCTR_0_DEVICE_ID
#define TIMER0_GI_ID	XPAR_FABRIC_TMRCTR_0_VEC_ID
#define GPIO_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define GPIO_GI_ID		XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define BTN_INT 		XGPIO_IR_CH1_MASK							// Mask that is used in enableing and disableing the gpio interrupt



#define MAX_COUNT  0xFFFFFFFF
#define PB_FRQ 100000000 //100MHz
#define TMR_LOAD(per)  (u32)((int)MAX_COUNT + 2 - (int)(per * (float)PB_FRQ)) //per is period in sec

void initInterrupt();


#endif
