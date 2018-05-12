/*
 ===============================================================================
 Name        : Board_TWE-Strong.c
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

#define BAUDRATE	57600

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
void UART_initialize();
void UART_SendChar(char data);
void UART_SendString(char *str);
uint8_t UART_Read();

volatile static uint32_t count = 0;
void SysTick_Handler(void) {
	if (count == 1000) {
		UART_SendString("1000\n");
		count = 0;
	} else {
		count++;
	}
}

void UART_IRQHandler(void) {
	uint8_t c = UART_Read();
	UART_SendChar(c);
}

int main(void) {
	volatile static uint32_t period;
	period = SystemCoreClock / 1000;
	SysTick_Config(period);

	UART_initialize();

	// Force the counter to be placed into memory
	volatile static int i = 0;
	// Enter an infinite loop, just incrementing a counter
	while (1) {
		i++;
	}
	return 0;
}
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

	NVIC_EnableIRQ (UART_IRQn);
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
