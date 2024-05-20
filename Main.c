#include "defines.h"
#include "filters.h"
#include "coefficients.h"


u32 status_reg = 0;		// Buffer to store status register of I2S
u8 is_data_ready = 0;	// unsigned 8 bit integer used as a boolean to indicate if new audio sample is available from I2S
						// 1: new sample available
						// 0: new sample not (yet) available

void loop();

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

    loop();

    //converter();
    return 0;
}

void loop()
{
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

	loop();
}
