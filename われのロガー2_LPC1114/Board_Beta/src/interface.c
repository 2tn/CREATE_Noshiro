/*
 * comm.c
 *
 *  Created on: 2015/07/19
 *      Author: Yusuke
 */

#include "LPC11xx.h"
#include "interface.h"
#include "i2c.h"

uint8_t src_addr[SSP_BUFSIZE];
uint8_t dest_addr[SSP_BUFSIZE];
extern volatile uint32_t I2CCount;
extern volatile uint8_t I2CMasterBuffer[BUFSIZE];
extern volatile uint8_t I2CSlaveBuffer[BUFSIZE];
extern volatile uint32_t I2CMasterState;
extern volatile uint32_t I2CReadLength, I2CWriteLength;
extern volatile uint8_t status;

void UART_initialize() {
	uint32_t DL;
	LPC_IOCON->PIO3_0 |= 0x03;
	LPC_IOCON->PIO3_1 |= 0x03;
	LPC_IOCON->RXD_LOC |= 0x02;
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12);
	LPC_SYSCON->UARTCLKDIV = 0x01;
	DL = (SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV)
			/ (16 * BAUDRATE * LPC_SYSCON->UARTCLKDIV);
	LPC_UART->LCR |= (1 << 7);
	LPC_UART->DLM = DL / 256;
	LPC_UART->DLL = DL % 256;
	LPC_UART->LCR &= ~(1 << 7);
	LPC_UART->LCR = 0x03;
	LPC_UART->FCR = 0x07;

	NVIC_EnableIRQ(UART_IRQn);
	LPC_UART->IER |= 0x01;
}

void UART_SendChar(char data) {
	while (!(LPC_UART->LSR & (1 << 5)))
		;
	LPC_UART->THR = data;
}

void UART_SendString(char *str) {
	while (*str != '\0') {
		UART_SendChar(*str);
		str++;
	}
}

uint8_t UART_Read() {
	while (!(LPC_UART->LSR & 0x01))
		;
	return LPC_UART->RBR;
}

void I2C_Initialization() {
	I2CInit((uint32_t) I2CMASTER);
}

void I2C_Write(uint8_t address, uint8_t data) {

	I2CWriteLength = 3;
	I2CReadLength = 0;
	I2CMasterBuffer[0] = MPL3115A2_ADDR << 1;
	I2CMasterBuffer[1] = address; /* address */
	I2CMasterBuffer[2] = data;
	I2CEngine();
}

void I2C_Read(uint8_t address, uint8_t length, uint8_t *data) {
	int i;
	for (i = 0; i < BUFSIZE; i++) {
		I2CSlaveBuffer[i] = 0x00;
	}
	/* Write SLA(W), address, SLA(R), and read one byte back. */
	I2CWriteLength = 2;
	I2CReadLength = length;
	I2CMasterBuffer[0] = MPL3115A2_ADDR << 1;
	I2CMasterBuffer[1] = address; /* address */
	I2CMasterBuffer[2] = (MPL3115A2_ADDR << 1) | RD_BIT;
	I2CEngine();
	while (!status) {
	};
	for (i = 0; i < length; ++i) {
		*data = I2CSlaveBuffer[i];
		data++;
	}
}

void SPI0_Initialization() {

	LPC_SYSCON->PRESETCTRL |= (0x1 << 0);
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1 << 11);
	LPC_SYSCON->SSP0CLKDIV = 0x02; /* Divided by 2 */
	LPC_IOCON->PIO0_8 |= 0x01; /* SSP MISO */
	LPC_IOCON->PIO0_9 |= 0x01; /* SSP MOSI */
	LPC_IOCON->SCK_LOC = 0x02;
	LPC_IOCON->PIO0_6 = 0x02; /* P0.6 function 2 is SSP clock, need to*/

	LPC_SSP0->CPSR = 0x00;
	LPC_SSP0->CR0 = (0x0007);
	/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
	LPC_SSP0->CR1 = (1 << 1);
	LPC_GPIO2->DIR |= (1 << 10);
	LPC_GPIO1->DIR |= (1 << 2);

}

void SPI0_Read(uint8_t address, uint8_t length, uint8_t *data) {
	src_addr[0] = address | READ_FLAG;
	SPI0_Send((uint8_t *) src_addr, 1);
	SPI0_Receive(data, length);
}

void SPI0_Write(uint8_t address, uint8_t data) {
	src_addr[0] = address | WRITE_FLAG;
	SPI0_Send((uint8_t *) src_addr, 1);
	src_addr[0] = data;
	SPI0_Send((uint8_t *) src_addr, 1);
}

void SPI0_Send(uint8_t *buf, uint32_t Length) {
	uint32_t i;
	uint8_t Dummy = Dummy;

	for (i = 0; i < Length; i++) {
		while ((LPC_SSP0->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
			;
		LPC_SSP0->DR = *buf;
		buf++;
		while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
			;
		Dummy = LPC_SSP0->DR;
	}
	return;
}
void SPI0_Receive(uint8_t *buf, uint32_t Length) {
	uint32_t i;
	for (i = 0; i < Length; i++) {
		LPC_SSP0->DR = 0xFF;
		while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
			;
		*buf = LPC_SSP0->DR;
		buf++;
	}
	return;
}

void Servo_initialize() {
	LPC_GPIO1->DIR |= (1 << 5);

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7);
	LPC_TMR16B0->PR = 4800 - 1;	//10kHz
	LPC_TMR16B0->MCR |= (1 << 0);
	LPC_TMR16B0->MCR |= (1 << 3);
	LPC_TMR16B0->MCR |= (1 << 4);
	LPC_TMR16B0->MR0 = 8; 		//5～24まで
	LPC_TMR16B0->MR1 = 200;
	LPC_TMR16B0->TCR = 1;

	NVIC_EnableIRQ(TIMER_16_0_IRQn);
	NVIC_SetPriority(TIMER_16_0_IRQn, 0);
}

void TIMER16_0_IRQHandler(void) {
	uint8_t IR = LPC_TMR16B0->IR;
	if (((IR >> 0) & 0x1) == 0x1) {
		LPC_GPIO1->DATA |= (1 << 5);
		LPC_TMR16B0->IR |= (1 << 0);
	} else if (((IR >> 1) & 0x1) == 0x1) {
		LPC_TMR16B0->IR |= (1 << 1);
		LPC_GPIO1->DATA &= ~(1 << 5);
	}
}

void Valve_initialize() {
	VALVE_OPEN();
	LPC_GPIO2->DIR |= (1 << 6);
}
