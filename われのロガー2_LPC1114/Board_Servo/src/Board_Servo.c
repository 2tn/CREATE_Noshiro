/*
 ===============================================================================
 Name        : Board_Servo.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#define SERVO_ON()	(LPC_TMR16B0->MR0 = 22)
#define SERVO_OFF()	(LPC_TMR16B0->MR0 = 8)
#endif

#include <cr_section_macros.h>

#include <stdio.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

uint16_t count = 0;
void Servo_Configuration();

void SysTick_Handler(void) {
	if (count == 2000) {
		SERVO_ON();
		printf("ON\n");
		count++;
	} else if (count == 4000) {
		SERVO_OFF();
		printf("OFF\n");
		count = 0;
	} else {
		count++;
	}
}

int c=0;
int d=0;
void TIMER16_0_IRQHandler(void) {
	uint8_t IR = LPC_TMR16B0->IR;
	if (((IR >> 0) & 0x1) == 0x1) {
		c++;
		if(c==100){
			c=0;
			printf("MR0\n");
		}
		LPC_GPIO1->DATA |= (1 << 5);
		LPC_TMR16B0->IR |= (1 << 0);
	} else if (((IR >> 1) & 0x1) == 0x1) {
		d++;
		if(d==100){
			d=0;
			printf("MR1\n");
		}
		LPC_TMR16B0->IR |= (1 << 1);
		LPC_GPIO1->DATA &= ~(1 << 5);
	}
}

int main(void) {
	Servo_Configuration();

	volatile static uint32_t period;
	period = SystemCoreClock / 1000;
	SysTick_Config(period);
	// Force the counter to be placed into memory
	volatile static int i = 0;
	// Enter an infinite loop, just incrementing a counter
	while (1) {
		i++;
	}
	return 0;
}

void Servo_Configuration() {
	LPC_GPIO1->DIR |= (1 << 5);

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7);
	LPC_TMR16B0->PR = 4800 - 1;	//10kHz
	LPC_TMR16B0->MCR |= (1 << 0);
	LPC_TMR16B0->MCR |= (1 << 3);
	LPC_TMR16B0->MCR |= (1 << 4);
	LPC_TMR16B0->MR0 = 5; 		//5～24まで
	LPC_TMR16B0->MR1 = 200;
	LPC_TMR16B0->TCR = 1;

	NVIC_EnableIRQ(TIMER_16_0_IRQn);
}
