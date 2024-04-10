#include "xgpio.h"
#include "xuartps.h"
#include "xiicps.h"
#include "xparameters.h"
#include "audio.h"
#include "xil_io.h"
#include "stdlib.h"
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#define UART_BASEADDR XPAR_PS7_UART_0_BASEADDR

#define ORDER 30
#define AMOUNT 1
