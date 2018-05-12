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
#include "SD.h"
#include "logger.h"
extern FIL fil;
extern enum SDState sstate;

void Sensor_initialize() {

	//CS Select
	CS_MPU_9250_LOW()
	SPI0_Write(PWR_MGMT_1, 0x80);
	CS_MPU_9250_HIGH()
	volatile uint32_t i;
	for (i = 0; i < 5000; ++i) {
	}
	CS_MPU_9250_LOW()
	SPI0_Write(ACCEL_CONFIG, 0x18);
	CS_MPU_9250_HIGH()
	CS_MPU_9250_LOW()
	SPI0_Write(GYRO_CONFIG, 0x18);
	CS_MPU_9250_HIGH()

	volatile uint8_t buf1[2];
	SPI0_Read(ACCEL_CONFIG, 2, &buf1[0]);

	I2C_Write(CTRL_REG1, 0x00);
	I2C_Write(PT_DATA_CFG, 0x07);
	//I2C_Write(CTRL_REG3, (1 << 5));
	//I2C_Write(CTRL_REG4, (1 << 7));
	//I2C_Write(CTRL_REG5, (1 << 7));
	//I2C_Write(INT_SOURCE, (1 << 7));
	I2C_Write(CTRL_REG1, 0x02);

	//チャージの奴
	LPC_GPIO0->DIR |= (1 << 11);
}

void MPU_9250(ACCEL *accel, GYRO *gyro) {
	if (sstate == t_initial) {
		return;
	}
	//TODO:地磁気も早く実装しろ―！！
	int acceldata[3];
	int gyrodata[3];
	volatile uint8_t buf1[6];	//わけないとヌーン
	volatile uint8_t buf2[6];
	uint32_t t = time();
	CS_MPU_9250_LOW()
	SPI0_Read(ACCEL_XOUT_H, 6, &buf1[0]);
	CS_MPU_9250_HIGH()
	volatile uint16_t i;
	for (i = 0; i < 500; ++i)
	CS_MPU_9250_LOW()
	SPI0_Read(GYRO_XOUT_H, 6, &buf2[0]);
	CS_MPU_9250_HIGH()

	for (i = 0; i < 3; i++) {
		int tmp = buf1[i * 2] << 8 | buf1[i * 2 + 1];
		if (tmp >= 32768) {
			acceldata[i] = tmp - 65536;
		} else {
			acceldata[i] = tmp;
		}

		tmp = buf2[i * 2] << 8 | buf2[i * 2 + 1];
		if (tmp >= 32768) {
			gyrodata[i] = tmp - 65536;
		} else {
			gyrodata[i] = tmp;
		}
	}
	accel->time = t;
	accel->x = acceldata[0];
	accel->y = acceldata[1];
	accel->z = acceldata[2];
	gyro->time = t;
	gyro->x = gyrodata[0];
	gyro->y = gyrodata[1];
	gyro->z = gyrodata[2];

	volatile uint8_t i1, i2;
	i1 = 0;
	while (!((accelbuff[i1].x == 0) && (accelbuff[i1].y == 0)
			&& (accelbuff[i1].z == 0))) {
		i1++;
	}

	accelbuff[i1].time = t;
	accelbuff[i1].x = acceldata[0];
	accelbuff[i1].y = acceldata[1];
	accelbuff[i1].z = acceldata[2];

	if (i1 == 9) {
		Datatype d = d_accel;
		Save_buff(d);
		int j;
		for (j = 0; j < 10; ++j) {
			accelbuff[j].time = 0;
			accelbuff[j].x = 0;
			accelbuff[j].y = 0;
			accelbuff[j].z = 0;
		}
	}
	i2 = 0;
	while (!((gyrobuff[i2].x == 0) && (gyrobuff[i2].y == 0)
			&& (gyrobuff[i2].z == 0))) {
		i2++;
	}

	gyrobuff[i2].time = t;
	gyrobuff[i2].x = gyrodata[0];
	gyrobuff[i2].y = gyrodata[1];
	gyrobuff[i2].z = gyrodata[2];

	if (i2 == 9) {
		Datatype d = d_gyro;
		Save_buff(d);
		int j;
		for (j = 0; j < 10; ++j) {
			gyrobuff[j].time = 0;
			gyrobuff[j].x = 0;
			gyrobuff[j].y = 0;
			gyrobuff[j].z = 0;
		}
	}
}
void MPL3115A2(PRESS *pressure, TEMP *temperature) {
	if (sstate == t_initial) {
		return;
	}
	uint8_t buf[5];
	uint32_t t = time();
	I2C_Read(OUT_P_MSB, 5, &buf[0]);
	I2C_Write(CTRL_REG1, 0x02);
	pressure->press = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
	pressure->time = t;
	temperature->temp = (buf[3] << 4) | (buf[4] >> 4);
	temperature->time = t;

	volatile uint8_t i = 0;
	while (!((pressbuff[i].press == 0) && (tempbuff[i].temp == 0))) {
		i++;
	}

	pressbuff[i].time = t;
	pressbuff[i].press = pressure->press;
	tempbuff[i].time = t;
	tempbuff[i].temp = temperature->temp;

	if (i == 9) {
		Datatype d = d_press_temp;
		Save_buff(d);
		int j;
		for (j = 0; j < 10; ++j) {
			pressbuff[j].time = 0;
			pressbuff[j].press = 0;
			tempbuff[j].time = 0;
			tempbuff[j].temp = 0;
		}
	}

}

void ADXL375(uint16_t *bigaccel) {
	//TODO:新しいセンサーに期待！！
}

void GPS(uint32_t *latitude, uint32_t *longitude, uint32_t *time,
		uint32_t *RTC) {
	//TODO:はよVAIO返せ
}
