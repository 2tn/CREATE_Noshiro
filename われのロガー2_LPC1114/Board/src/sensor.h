/*
 * sensor.h
 *
 *  Created on: 2015/07/19
 *      Author: Yusuke
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#define INTERVAL	10
#define BUFFSIZE	10

#define	CS_MPU_9250_LOW()	{LPC_GPIO2->DATA &= ~(1<<10);}
#define	CS_MPU_9250_HIGH()	{LPC_GPIO2->DATA |= (1<<10);}
#define	CS_ADXL375_LOW()	{LPC_GPIO1->DATA &= ~(1<<2);}
#define	CS_ADXL375_HIGH()	{LPC_GPIO1->DATA |= (1<<2);}

#define  WHO_AM_I       0x75
#define  ACCEL_XOUT_H	0x3B
#define  GYRO_XOUT_H	0x43

#define STATUS           	0x00
#define OUT_P_MSB        	0x01
#define OUT_P_CSB        	0x02
#define OUT_P_LSB        	0x03
#define OUT_T_MSB        	0x04
#define OUT_T_LSB        	0x05
#define DR_STATUS        	0x06
#define OUT_P_DELTA_MSB  	0x07
#define OUT_P_DELTA_CSB  	0x08
#define OUT_P_DELTA_LSB  	0x09
#define OUT_T_DELTA_MSB  	0x0A
#define OUT_T_DELTA_LSB  	0x0B
#define CTRL_REG1       	0x26
#define CTRL_REG2        	0x27
#define CTRL_REG3        	0x28
#define CTRL_REG4        	0x29
#define CTRL_REG5        	0x2A
#define PT_DATA_CFG      	0x13
#define INT_SOURCE			0x12

#define GYRO_CONFIG			0x1B
#define ACCEL_CONFIG      	0x1C
#define ACCEL_CONFIG2      	0x1D
#define PWR_MGMT_1        	0x6B
#endif /* SENSOR_H_ */

typedef struct {
	uint32_t time;
	int x;
	int y;
	int z;
} ACCEL;

typedef struct {
	uint32_t time;
	int x;
	int y;
	int z;
} GYRO;

typedef struct {
	uint32_t time;
	int x;
	int y;
	int z;
} MAG;

typedef struct {
	uint32_t time;
	int x;
	int y;
	int z;
} BIGACCEL;

typedef struct {
	uint32_t time;
	uint32_t press;
} PRESS;

typedef struct {
	uint32_t time;
	uint16_t temp;
} TEMP;

typedef struct {
	ACCEL accel;
	GYRO gyro;
	MAG mag;
	BIGACCEL bigaccel;
	PRESS press;
	TEMP temp;
} SENSORDATA;

ACCEL accelbuff[BUFFSIZE];
GYRO gyrobuff[BUFFSIZE];
MAG magbuff[BUFFSIZE];
BIGACCEL bigaccelbuff[BUFFSIZE];
PRESS pressbuff[BUFFSIZE];
TEMP tempbuff[BUFFSIZE];

extern void Sensor_initialize();
extern void MPU_9250(ACCEL *accel, GYRO *gyro);
void MPL3115A2(PRESS *pressure, TEMP *temperature);
extern void ADXL375(uint16_t *bigaccel);
extern void GPS(uint32_t *latitude, uint32_t *longitude, uint32_t *time,
		uint32_t *RTC);
