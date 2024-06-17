#ifndef __DEFINES_H_
#define __DEFINES_H_

#define UART_BASEADDR XPAR_PS7_UART_0_BASEADDR

#define ORDER 8
#define AMOUNT 3

// The defines for the SH1106 screen
#define SlaveAddress 0x3c				// Adress to which the data is send
#define IIC_SCLK_RATE 400000			// the clock rate of the screen and IIC
#define UsedFont u8g2_font_t0_11_me		// The font used to print

#endif
