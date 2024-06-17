#include "platform.h"
#include "xparameters.h"
#include "xuartps.h"
#include "xil_io.h"
#include "stdlib.h"
#include <stdio.h>

#include "audio.h"
#include "xiicps.h"
#include "xil_printf.h"

#include "defines.h"

#include "SH1106_Screen.h"
#include "filters.h"
#include "xstatus.h"
#include "interrupt.h"

int main()
{
    init_platform();

    printf("initializing software\n");

	int status = IicConfig(XPAR_XIICPS_1_DEVICE_ID);
	if (status != XST_SUCCESS) { printf("IIC failed\n");}
	else {printf("IIC succesfull\n");}
	//Configure the Audio Codec's PLL
	AudioPllConfig();

	printf("Codec config succesfull\n");

	//Configure the Line in and Line out ports
	LineinLineoutConfig();
	printf("Line in/out config succesfull\n");

	xil_printf("ADAU1761 configured\n\r");

    setupFilters();

    dBToFloat(-60, 0);
    dBToFloat(-60, 1);
    dBToFloat(-60, 2);

    initDisplay();
    EqGuiConfig();

    initInterrupt();

    while(1){}

    //converter();
    return 0;
}
