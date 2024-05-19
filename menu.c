#include "menu.h"
#include "coefficients.h"
#include "filters.h"

u32 status_reg1 = 0;		// Buffer to store status register of I2S
u8 is_data_ready = 0;	// unsigned 8 bit integer used as a boolean to indicate if new audio sample is available from I2S
						// 1: new sample available
						// 0: new sample not (yet) available

void menu()
{
	u8 inp = 0x00;
	u32 CntrlRegister;

	/* Turn off all LEDs */
	//Xil_Out32(LED_BASE, 0);

	CntrlRegister = XUartPs_ReadReg(UART_BASEADDR, XUARTPS_CR_OFFSET);

	XUartPs_WriteReg(UART_BASEADDR, XUARTPS_CR_OFFSET,
				  ((CntrlRegister & ~XUARTPS_CR_EN_DIS_MASK) |
				   XUARTPS_CR_TX_EN | XUARTPS_CR_RX_EN));

	xil_printf("\r\n\r\n");
	xil_printf("Embedded LMS Filtering Demo\r\n");
	xil_printf("Enter 's' to stream pure audio, 'f' to select float FIR mode and\n 'd' to select double FIR mode.\r\n");
	xil_printf("----------------------------------------\r\n");

	// Wait for input from UART via the terminal
	while (!XUartPs_IsReceiveData(UART_BASEADDR));
	inp = XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);
	// Select function based on UART input
	switch(inp){
	case 's':
		xil_printf("STREAMING AUDIO\r\n");
		xil_printf("Press 'q' to return to the main menu\r\n");
		loopRegular();
		break;
	case 'f':
		printf("ENTERING FIR MODE\n");
		printf("Press 'q' to return to the main menu\n");
		loopFIR();
	case 'i':
		printf("ENTERING IIR MODE\n");
		printf("Press 'q' to return to the main menu\n");
		loopIIR();
		break;
	default:
		menu();
		break;
	}
}

void loopRegular()
{
	while (!XUartPs_IsReceiveData(UART_BASEADDR)){
    	/*
    	 * Wait for a new audio sample to be available (48KHz)
    	 */
    	while (is_data_ready == 0) {
    		// A new audio sample is available when bit21 of I2S_STATUS_REG becomes 1
    		// (see https://byu-cpe.github.io/ecen427/documentation/audio-hw/)
    		status_reg1 = Xil_In32(I2S_STATUS_REG);
    		// Read bit21 of register I2S_STATUS_REG
    		is_data_ready = (status_reg1 >> 21 ) & 1;
    	}
    	is_data_ready = 0;
    	// Clear bit21 of I2S_STATUS_REG, i.e. set bit21 to 0
    	status_reg1 = status_reg1 & (u32)(~(1<<21));
    	Xil_Out32(I2S_STATUS_REG, status_reg1);

    	timerStart();
    	updateInput();
   		regular();
    	outputData();
    	timerEnd();
	}
	if(XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET) == 'q') menu();
	loopRegular();
}

void loopFIR()
{
	while (!XUartPs_IsReceiveData(UART_BASEADDR)){
    	/*
    	 * Wait for a new audio sample to be available (48KHz)
    	 */
    	while (is_data_ready == 0) {
    		// A new audio sample is available when bit21 of I2S_STATUS_REG becomes 1
    		// (see https://byu-cpe.github.io/ecen427/documentation/audio-hw/)
    		status_reg1 = Xil_In32(I2S_STATUS_REG);
    		// Read bit21 of register I2S_STATUS_REG
    		is_data_ready = (status_reg1 >> 21 ) & 1;
    	}
    	is_data_ready = 0;
    	// Clear bit21 of I2S_STATUS_REG, i.e. set bit21 to 0
    	status_reg1 = status_reg1 & (u32)(~(1<<21));
    	Xil_Out32(I2S_STATUS_REG, status_reg1);

    	timerStart();
    	updateInput();
    	FIR(terms);
    	outputData();
    	timerEnd();
	}
	if(XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET) == 'q') menu();
	loopFIR();
}


void loopIIR()
{
	while (!XUartPs_IsReceiveData(UART_BASEADDR)){
    	/*
    	 * Wait for a new audio sample to be available (48KHz)
    	 */
    	while (is_data_ready == 0) {
    		// A new audio sample is available when bit21 of I2S_STATUS_REG becomes 1
    		// (see https://byu-cpe.github.io/ecen427/documentation/audio-hw/)
    		status_reg1 = Xil_In32(I2S_STATUS_REG);
    		// Read bit21 of register I2S_STATUS_REG
    		is_data_ready = (status_reg1 >> 21 ) & 1;
    	}
    	is_data_ready = 0;
    	// Clear bit21 of I2S_STATUS_REG, i.e. set bit21 to 0
    	status_reg1 = status_reg1 & (u32)(~(1<<21));
    	Xil_Out32(I2S_STATUS_REG, status_reg1);

    	timerStart();
    	updateInput();
    	IIR(num, den);
    	outputData();
    	timerEnd();
	}
	if(XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET) == 'q') menu();
	loopIIR();
}
