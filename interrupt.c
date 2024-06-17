#include "interrupt.h"
#include "xscugic.h"
#include "xtmrctr.h"
#include "coefficients.h"
#include "xgpio.h"
#include "filters.h"


XScuGic GicInst;
XTmrCtr sampleTimer;
XGpio BtnGpio;

void interruptHandlerSample()
{
	XScuGic_Disable(&GicInst, TIMER0_GI_ID);
	timerStart();
	updateInput();
	IIR(numDerek, denDerek, 0);
	IIR2(numJordi1, denJordi1, numJordi2, denJordi2, 1);
	IIR(numMees, denMees, 2);
	//IIR(numDaan, denDaan, 0);
	adder();
	timerEnd();
	XScuGic_Enable(&GicInst, TIMER0_GI_ID);
}

void interruptHandlesGPIO()
{
	static u8 selFilter = 0, nmr = 0;
	static s8 dBgain[AMOUNT] = {0};
	//for(int i = 0; i<AMOUNT; i++) {dBgain[i] = 0;}
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BtnGpio, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BtnGpio) & BTN_INT) != BTN_INT) {
		return;
	}
	// Reset if centre button pressed
	int BtnValue = XGpio_DiscreteRead(&BtnGpio, 1);
	// print on the display while the btn is pressed
	switch (BtnValue)
	{
	case 0b0001:
		for (int i = 0; i<AMOUNT; i++)
		{dBgain[i] = 0; dBToFloat(0, i);}
		selFilter = 0;
		EqGuiConfig();
	break;
	case 0b0010:
		nmr = (selFilter == 0) ? selectFilter(&selFilter) : backFilter(&selFilter);
		break;
	case 0b0100:
		if (selFilter == 0)
		{
			moveSelect(0);
		}
		else
		{
			if(dBgain[nmr]+1>12) {dBgain[nmr] = 12;}
			else {dBgain[nmr] = dBgain[nmr]+1;}
			dbUp();
			dBToFloat(dBgain[nmr]*10, nmr);
			//printf("nmr = %d\n", nmr);
			//printf("dBgain = %d\n", dBgain[nmr]);
		}
		break;
	case 0b1000:
		if (selFilter == 0)
		{
			moveSelect(1);
		}
		else
		{
			if(dBgain[nmr]-1<-12) {dBgain[nmr] = -12;}
			else {dBgain[nmr] = dBgain[nmr]-1;}
			dbDown();
			dBToFloat(dBgain[nmr]*10, nmr);
			//printf("nmr = %d\n", nmr);
			//printf("dBgain = %d\n", dBgain[nmr]);
		}
		break;
		break;
	default:
	}
	(void) XGpio_InterruptClear(&BtnGpio, BTN_INT);
	// Enable GPIO interrupts
	XGpio_InterruptEnable(&BtnGpio, BTN_INT);
}


void initInterrupt()
{
    /*--------------------------Get the config of the interrupt controller-----------------------------*/
    XScuGic_Config *IntcConfig = XScuGic_LookupConfig(GIC_ID);
    if (NULL == IntcConfig) {
    	printf("\n\r XScuGic_LookupConfig() failed\n");
    	return;
    }

    /*-----------------------------initialize the initerrupt controller--------------------------------*/
    int Status = XScuGic_CfgInitialize(&GicInst, IntcConfig,IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS) {
    	printf("\n\r XScuGic_CfgInitialize() failed\n");
    	return;
    }

	/*------------------------------------initialize the timer-----------------------------------------*/
    Status =  XTmrCtr_Initialize(&sampleTimer, TIMER0_ID);
    if (Status != XST_SUCCESS) {
    	printf("Could not initialize timer \n\r");
    }

    // link the timer and the interrupt it triggers
    XTmrCtr_SetHandler(&sampleTimer, (XTmrCtr_Handler)interruptHandlerSample, (void*) 0x12345678);


    /*-----------------------------------initialize the GPIO-------------------------------------------*/
    Status = XGpio_Initialize(&BtnGpio, GPIO_ID);
    if (Status != XST_SUCCESS) {
    	printf("Could not initialize GPIO \n\r");
    }
	XGpio_SetDataDirection(&BtnGpio, 1, 0xF);	//channel 1 and all inputs
	// Enable the interrupt on the GPIO side
	XGpio_InterruptEnable(&BtnGpio, BTN_INT);
	XGpio_InterruptGlobalEnable(&BtnGpio);


    /*------------set prioroty to place 160 (aka 0xA0) and the trigger to risign edge (aka 0x3)----------*/
    XScuGic_SetPriorityTriggerType(&GicInst, TIMER0_GI_ID, 0x98, 0x3);
    XScuGic_SetPriorityTriggerType(&GicInst, GPIO_GI_ID, 0xA8, 0X3);

    /*-------------------------connect the interrupt controller to the timer-----------------------------*/
    Status = XScuGic_Connect(&GicInst, TIMER0_GI_ID,(Xil_InterruptHandler)XTmrCtr_InterruptHandler,&sampleTimer);
    if (Status != XST_SUCCESS) {
    	printf("\n\r XScuGic_Connect() failed");
    }

    /*-------------------------connect the interrupt controller to the GPIO-----------------------------*/
    Status = XScuGic_Connect(&GicInst, GPIO_GI_ID,(Xil_InterruptHandler)interruptHandlesGPIO, (void *)&BtnGpio);
    if (Status != XST_SUCCESS) {
    	printf("\n\r XScuGic_Connect() failed");
    }

	// Enable the interrupt on the GPIO with another mask
	XGpio_InterruptEnable(&BtnGpio, 1);
	XGpio_InterruptGlobalEnable(&BtnGpio);

    printf("interrupt init succesfull\n");

    /*-----------------------------enable the interrupt controller-------------------------------------*/
    XScuGic_Enable(&GicInst, TIMER0_GI_ID);
    XScuGic_Enable(&GicInst, GPIO_GI_ID);

    // Initialize the exception table.
    Xil_ExceptionInit();

    // Register the interrupt controller handler with the exception table.
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&GicInst);

    // Enable exceptions.
    Xil_ExceptionEnable();

    //set the timer to 48 kHz
    XTmrCtr_SetOptions(&sampleTimer, 0,   XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
    XTmrCtr_SetResetValue(&sampleTimer, 0, TMR_LOAD(0.000020833333333333)); // 48 kHz
    XTmrCtr_Start(&sampleTimer, 0);
}
