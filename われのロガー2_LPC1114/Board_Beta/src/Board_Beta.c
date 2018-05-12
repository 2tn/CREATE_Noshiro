/*
 ===============================================================================
 Name        : Board_Beta.c
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

#include "sensor.h"
#include "interface.h"
#include "logger.h"
#include "format.h"

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
SENSORDATA data;
EVENT_TIME etime = { 0, 0, 0, 0, 0 };

volatile static uint32_t count = 0;
volatile static uint8_t trigger = 0;

void SysTick_Handler(void) {
	if (count == 10) {
		trigger = 1;
		count = 0;
	} else {
		count++;
		trigger = 2;
	}
}

int main(void) {
	printf("Hello World\n");

	volatile static uint32_t period;
	period = SystemCoreClock / 1000;
	SysTick_Config(period);

	Timer_initialize();
	SPI0_Initialization();
	I2C_Initialization();
	UART_initialize();
	Sensor_initialize();
	Servo_initialize();
	int i = 0;	// Force the counter to be placed into memory
	// Enter an infinite loop, just incrementing a counter
	while (1) {
		switch (trigger) {
		case 1:	//10ms
			MPL3115A2(&data.press, &data.temp);
			MPU_9250(&data.accel[0], &data.gyro[0]);
			Loop();
			break;
		case 2:	//1ms
			MPU_9250(&data.accel[0], &data.gyro[0]);
			break;
		default:
			break;
		}
	}
	return 0;
}
