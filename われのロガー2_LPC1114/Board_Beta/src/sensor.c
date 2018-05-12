/*
 * sensor.c
 *
 *  Created on: 2015/07/19
 *      Author: Yusuke
 */
#include "LPC11xx.h"
#include "sensor.h"
#include "interface.h"
#include <stdio.h>

void Sensor_initialize() {
	CS_MPU_9250_LOW()
	SPI0_Write(0x6B, 0x00);
	CS_MPU_9250_HIGH()
	CS_MPU_9250_LOW()
	SPI0_Write(0x37, 0x02);
	CS_MPU_9250_HIGH()
	CS_MPU_9250_LOW()
	SPI0_Write(0x1B, 0x18);
	CS_MPU_9250_HIGH()
	CS_MPU_9250_LOW()
	SPI0_Write(0x1C, 0x18);
	CS_MPU_9250_HIGH()

	I2C_Write(CTRL_REG1, 0x00);
	I2C_Write(PT_DATA_CFG, 0x07);
	//I2C_Write(CTRL_REG3, (1 << 5));
	//I2C_Write(CTRL_REG4, (1 << 7));
	//I2C_Write(CTRL_REG5, (1 << 7));
	//I2C_Write(INT_SOURCE, (1 << 7));
	I2C_Write(CTRL_REG1, 0x02);
}

void MPU_9250(int *accel, int *gyro) {
	//TODO:地磁気も早く実装しろ―！！
	uint8_t buf[6];
	CS_MPU_9250_LOW()
	SPI0_Read(ACCEL_XOUT_H, 6, &buf[0]);
	int i;
	for (i = 0; i < 3; i++) {
		int tmp = buf[i * 2] << 8 | buf[i * 2 + 1];
		if (tmp >= 32768) {
			*accel = tmp - 65536;
		} else {
			*accel = tmp;
		}
		accel++;

		tmp = buf[i * 2 + 6] << 8 | buf[i * 2 + 7];
		if (tmp >= 32768) {
			*gyro = tmp - 65536;
		} else {
			*gyro = tmp;
		}
		gyro++;
	}
	CS_MPU_9250_HIGH()
}
void MPL3115A2(uint32_t *pressure, uint16_t *temperature) {
	uint8_t buf[5];
	I2C_Read(OUT_P_MSB, 5, &buf[0]);
	I2C_Write(CTRL_REG1, 0x02);
	*pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
	*temperature = (buf[3] << 4) | (buf[4] >> 4);
}

void ADXL375(uint16_t *bigaccel) {
	//TODO:新しいセンサーに期待！！
}

void GPS(uint32_t *latitude, uint32_t *longitude, uint32_t *time, uint32_t *RTC) {
	//TODO:はよVAIO返せ
}
