/*
 * sensor.h
 *
 *  Created on: 2015/07/19
 *      Author: Yusuke
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#define INTERVAL	10
#define	CS_MPU_9250_LOW()	{LPC_GPIO2->DATA &= ~(1<<10);}
#define	CS_MPU_9250_HIGH()	{LPC_GPIO2->DATA |= (1<<10);}
#define	CS_ADXL375_LOW()	{LPC_GPIO1->DATA &= ~(1<<2);}
#define	CS_ADXL375_HIGH()	{LPC_GPIO1->DATA |= (1<<2);}

#define  WHO_AM_I       0x75
#define  ACCEL_XOUT_H	0x3B

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
#endif /* SENSOR_H_ */
extern void Sensor_initialize();
extern void MPU_9250(int *accel, int *gyro);
extern void MPL3115A2(uint32_t *pressure, uint16_t *temperature);
extern void ADXL375(uint16_t *bigaccel);
extern void GPS(uint32_t *latitude, uint32_t *longitude, uint32_t *time,
		uint32_t *RTC);

typedef struct {
	int accel[3];
	int gyro[3];
	int mag[3];
	int bigaccel[3];
	uint32_t press;
	uint16_t temp;
} SENSORDATA;
