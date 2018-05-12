/*
 ===============================================================================
 Name        : Board_GPS.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>

#include <stdio.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#define SSPSR_TFE       (0x1<<0)
#define SSPSR_TNF       (0x1<<1)
#define SSPSR_RNE       (0x1<<2)
#define SSPSR_RFF       (0x1<<3)
#define SSPSR_BSY       (0x1<<4)

#define READ_FLAG         	0x80
#define WRITE_FLAG        	0x00

#define	GPS_CS_LOW()	{LPC_GPIO2->DATA &= ~(1<<8);}
#define	GPS_CS_HIGH()	{LPC_GPIO2->DATA |= (1<<8);}

void SPI0_Init();
uint8_t SPI0_Transfer(uint8_t send);
int states = 0;
void GPS(uint32_t *data);

void PIOINT2_IRQHandler(void) {
	LPC_GPIO2->IC |= (1 << 0);
	uint32_t GPSdata[3];
	GPS(&GPSdata);
	printf("%d %d %d\n", GPSdata[0], GPSdata[1], GPSdata[2]);
}
int main(void) {

	printf("Hello World\n");

	SPI0_Init();
	printf("%d\n", SPI0_Transfer(0x00));
	// Force the counter to be placed into memory
	volatile static int i = 0;
	// Enter an infinite loop, just incrementing a counter
	while (1) {
		i++;
	}
	return 0;
}
void SPI0_Init() {

	LPC_SYSCON->PRESETCTRL |= (0x1 << 0);
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1 << 11);
	LPC_SYSCON->SSP0CLKDIV = 0x02; /* Divided by 2 */
	LPC_IOCON->PIO0_8 |= 0x01; /* SSP MISO */
	LPC_IOCON->PIO0_9 |= 0x01; /* SSP MOSI */
	LPC_IOCON->SCK_LOC = 0x02;
	LPC_IOCON->PIO0_6 = 0x02; /* P0.6 function 2 is SSP clock, need to*/

	LPC_SSP0->CPSR = 0x00;
	LPC_SSP0->CR0 = (0x0707);
	/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
	LPC_SSP0->CR1 = (1 << 1);

	//GPS Configuration
	LPC_IOCON->PIO2_0 |= (1 << 3);
	LPC_IOCON->PIO2_8 |= (1 << 3);
	GPS_CS_HIGH()
	LPC_GPIO2->DIR |= (1 << 8);

	NVIC_EnableIRQ (EINT2_IRQn);
	LPC_GPIO2->DIR &= ~(1 << 0);
	LPC_GPIO2->IBE &= ~(1 << 0);
	LPC_GPIO2->IEV |= (1 << 0);
	LPC_GPIO2->IE |= (1 << 0);
	//End GPS Configuration
}

uint8_t SPI0_Transfer(uint8_t send) {
	while ((LPC_SSP0->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
		;
	LPC_SSP0->DR = send;
	while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
		;
	return LPC_SSP0->DR;
}

void GPS(uint32_t *data) {
	GPS_CS_LOW()
	SPI0_Transfer(0x03);
	GPS_CS_HIGH()
	uint8_t res[12];
	GPS_CS_LOW()
	res[0] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[1] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[2] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[3] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[4] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[5] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[6] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[7] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[8] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[9] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[10] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()
	GPS_CS_LOW()
	res[11] = SPI0_Transfer(0x3);
	GPS_CS_HIGH()

	*data = (res[0] << 24) | (res[1] << 16) | (res[2] << 8) | res[3];
	data++;
	*data = (res[4] << 24) | (res[5] << 16) | (res[6] << 8) | res[7];
	data++;
	*data = (res[8] << 24) | (res[9] << 16) | (res[10] << 8) | res[11];
	data++;
}
