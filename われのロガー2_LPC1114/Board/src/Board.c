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
#include "SD.h"

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

SENSORDATA data;
EVENT_TIME etime = { 0, 0, 0, 0, 0 };
volatile FIL fil;
volatile static uint32_t count = 0;
volatile static uint8_t trigger = 0;

void SysTick_Handler(void) {
	if (count == 10) {
		//10ms
		MPL3115A2(&data.press, &data.temp);
		MPU_9250(&data.accel, &data.gyro);
		Loop();

		trigger = 1;
		count = 0;
	} else {
		if (count % 2) {
			MPU_9250(&data.accel, &data.gyro);
		}
		//1ms
		disk_timerproc();

		count++;
		trigger = 2;
	}
}
//FATFS FatFs;
int main(void) {
#if DEBUG
	printf("Hello World\n");
#endif
	volatile static uint32_t period;
	period = SystemCoreClock / 1000;
	SysTick_Config(period);
	disk_timerproc();
	FRESULT fr = SD_initialize(&fil);

	Timer_initialize();
	SPI0_Initialization();
	I2C_Initialization();
	UART_initialize();
	Sensor_initialize();
	Servo_initialize();
	Valve_initialize();

	int i = 0;	// Force the counter to be placed into memory
	// Enter an infinite loop, just incrementing a counter
	while (1) {
	}
	return 0;
}
