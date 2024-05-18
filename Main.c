#include "defines.h"
#include "filters.h"


u32 status_reg = 0;		// Buffer to store status register of I2S
u8 is_data_ready = 0;	// unsigned 8 bit integer used as a boolean to indicate if new audio sample is available from I2S
						// 1: new sample available
						// 0: new sample not (yet) available

float terms[ORDER+1] = {
		0.000896176498665450017334921373191036764,
		0.003136617746484260220440898336846657912,
		0.007449467148734819146871366513096290873,
		0.013918741253322231346545301278183615068,
		0.021809295274729485064613143663336813916,
		0.029577325165467873574476342923844640609,
		0.035309922933913460985078103249179548584,
		0.037425105137752219530700159566549700685,
		0.035309922933913460985078103249179548584,
		0.029577325165467873574476342923844640609,
		0.021809295274729485064613143663336813916,
		0.013918741253322231346545301278183615068,
		0.007449467148734819146871366513096290873,
		0.003136617746484260220440898336846657912,
		0.000896176498665450017334921373191036764
};


float num[ORDER+1] = {
		0.000002292499055621388168423871109502521,
		0.000013754994333728328163510279402714787,
		0.000034387485834320827185039276541189679,
		0.000045849981112427765062543316698651097,
		0.000034387485834320820408775698506786966,
		0.000013754994333728328163510279402714787,
		0.000002292499055621388168423871109502521,
		0,
		0,
		0,
		0,
		0,
		0,
		0
};

float den[ORDER+1] = {
		1,
		 -5.044198728582690804955745988991111516953,
		 10.667126097609660462239844491705298423767,
		-12.097945382098838962292575160972774028778,
		  7.757009272749176176375840441323816776276,
		 -2.664974270881526940968342387350276112556,
		  0.383129731143777740864919678642763756216,
		  0,
		  0,
		  0,
		  0,
		  0,
		  0,
		  0,
		  0
};

void menu();

void loopRegular();
void loopFIR();
void loopIIR();

int main()
{
    init_platform();

	IicConfig(XPAR_XIICPS_1_DEVICE_ID);

	//Configure the Audio Codec's PLL
	AudioPllConfig();

	//Configure the Line in and Line out ports.
	//Call LineInLineOutConfig() for a configuration that
	//enables the HP jack too.
	//AudioConfigureJacks();
	LineinLineoutConfig();

	xil_printf("ADAU1761 configured\n\r");

    setupFilters();

    menu();

    //converter();
    return 0;
}


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
    		status_reg = Xil_In32(I2S_STATUS_REG);
    		// Read bit21 of register I2S_STATUS_REG
    		is_data_ready = (status_reg >> 21 ) & 1;
    	}
    	is_data_ready = 0;
    	// Clear bit21 of I2S_STATUS_REG, i.e. set bit21 to 0
    	status_reg = status_reg & (u32)(~(1<<21));
    	Xil_Out32(I2S_STATUS_REG, status_reg);

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
    		status_reg = Xil_In32(I2S_STATUS_REG);
    		// Read bit21 of register I2S_STATUS_REG
    		is_data_ready = (status_reg >> 21 ) & 1;
    	}
    	is_data_ready = 0;
    	// Clear bit21 of I2S_STATUS_REG, i.e. set bit21 to 0
    	status_reg = status_reg & (u32)(~(1<<21));
    	Xil_Out32(I2S_STATUS_REG, status_reg);

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
    		status_reg = Xil_In32(I2S_STATUS_REG);
    		// Read bit21 of register I2S_STATUS_REG
    		is_data_ready = (status_reg >> 21 ) & 1;
    	}
    	is_data_ready = 0;
    	// Clear bit21 of I2S_STATUS_REG, i.e. set bit21 to 0
    	status_reg = status_reg & (u32)(~(1<<21));
    	Xil_Out32(I2S_STATUS_REG, status_reg);

    	timerStart();
    	updateInput();
    	IIR(num, den);
    	outputData();
    	timerEnd();
	}
	if(XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET) == 'q') menu();
	loopIIR();
}
