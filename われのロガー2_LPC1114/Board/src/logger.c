/*
 * logger.c
 *
 *  Created on: 2015/08/06
 *      Author: Yusuke
 */
#include "LPC11xx.h"
#include "sensor.h"
#include "logger.h"
#include "interface.h"
#include "SD.h"
#include "format.h"
#include <stdio.h>
extern SENSORDATA data;
extern EVENT_TIME etime;
extern FIL fil;
uint8_t loopcount = 0;
extern SDState sstate;
HisyoState hstate = s_initial;

void Timer_initialize() {

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 9);
	LPC_TMR32B0->PR = 48000 - 1;	//1kHz
	LPC_TMR32B0->MCR |= (1 << 0);
	LPC_TMR32B0->MCR |= (1 << 1);
	LPC_TMR32B0->MR0 = 100;
	LPC_TMR32B0->TCR = 0;

	NVIC_EnableIRQ (TIMER_32_0_IRQn);
	NVIC_SetPriority(TIMER_32_0_IRQn, 1);

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 10);
	LPC_TMR32B1->PR = 48000 - 1;	//1kHz
	LPC_TMR32B1->TCR = 1;

	LPC_GPIO3->DIR |= (1 << 3);

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 8);
	LPC_TMR16B1->PR = 48000 - 1;	//1kHz
	LPC_TMR16B1->MCR |= (1 << 0);
	LPC_TMR16B1->MCR |= (1 << 3);
	LPC_TMR16B1->MCR |= (1 << 4);
	LPC_TMR16B1->MR0 = 500; 		//5～24まで
	LPC_TMR16B1->MR1 = 60000;
	LPC_TMR16B1->TCR = 1;

	NVIC_EnableIRQ (TIMER_16_1_IRQn);
	NVIC_SetPriority(TIMER_16_1_IRQn, 0);
}

//timerたりてないから暫定処置
void TIMER32_0_IRQHandler(void) {
	//LEDはPIO0_11,PIO1_5,PIO3_3
	uint8_t IR = LPC_TMR32B0->IR;
	if (((IR >> 0) & 0x1) == 0x1) {
		LPC_TMR32B0->IR |= (1 << 0);
		LPC_GPIO3->DATA ^= (1 << 3);
	}
}
void TIMER16_1_IRQHandler(void) {
	uint8_t IR = LPC_TMR16B1->IR;
	if (((IR >> 0) & 0x1) == 0x1) {
		VALVE_CLOSE();
		LPC_TMR16B1->IR |= (1 << 0);
	} else if (((IR >> 1) & 0x1) == 0x1) {
		LPC_TMR16B1->IR |= (1 << 1);
		VALVE_OPEN();
	}
}
uint32_t time() {
	return LPC_TMR32B1->TC;
}

void Loop() { //10msに一回シュコシュコしてくださーい

	if (sstate == t_initial) {
		return;
	}
	loopcount++;
	if (loopcount == 100) {
		SD_save(&fil);
		Save_buff (d_status);
		loopcount = 0;
	}
	switch (hstate) {
	case s_initial: {	//点火検知セクション
		static double accel_fifo[ACCEL_FIFO_DATALENGTH];
		static int over_accel = 0;

		double sum;
		int i;
		double x = (double) data.accel.x / 2048;
		double y = (double) data.accel.y / 2048;
		double z = (double) data.accel.z / 2048;
		sum = x * x + y * y + z * z;

		//データを平均化する
		double average = 0;
		for (i = ACCEL_FIFO_DATALENGTH - 1; i > 0; i--) {
			accel_fifo[i] = accel_fifo[i - 1];
			average += accel_fifo[i - 1];
		}
		accel_fifo[0] = sum;
		average += accel_fifo[0];
		average /= ACCEL_FIFO_DATALENGTH;

		//一定時間、加速度のオーバーを検知し続ける
		if (average >= (IGNITION_ACCEL * IGNITION_ACCEL)) {
			over_accel++;
		} else {
			over_accel = 0;
		}

		//条件がそろったら検知
		if (over_accel >= (IGNITION_TIME / INTERVAL)) {
			LPC_TMR32B0->TC = 0;
			LPC_TMR32B0->MR0 = 500;
			LPC_TMR32B0->TCR = 1;
			LPC_TMR16B1->TCR = 0;

			Save_buff (d_status);
			etime.ignition_time = time();
			hstate = s_detectignition;
			Save_buff(d_status);
#if DEBUG
			printf("IGNITION:%d\n", etime.ignition_time);
#endif
		}
		//TODO:debug
		static int count_t = 0;
		count_t++;
		if (count_t == 100) {
#if DEBUG
			printf("1\n");
#endif
			count_t = 0;
		}
		//end debug
	}
		break;

	case s_detectignition: {	//パラシュート解放
		VALVE_OPEN();
		LPC_TMR32B0->MR0 = 500;
		int i;
		static uint32_t check_count = 0;
		static uint32_t press_fifo[PRESS_FIFO_DATALENGTH];
		static uint64_t press_fall_count = 0;

		//PRESS_FIFO_DATALENGTH分だけ平均化
		for (i = PRESS_FIFO_DATALENGTH - 1; i > 0; i--) {
			press_fifo[i] = press_fifo[i - 1];
		}
		press_fifo[0] = data.press.press;
		long gap = press_fifo[0] - press_fifo[PRESS_FIFO_DATALENGTH - 1];

		//気圧の平均値が上昇していたらカウント
		if (gap >= 0) {
			press_fall_count <<= 1;
			press_fall_count++;
			press_fall_count &= (((uint64_t) 1 << 50) - 1);
		} else {
			press_fall_count <<= 1;
			press_fall_count &= (((uint64_t) 1 << 50) - 1);

		}

		//気圧の平均値が上昇した回数をカウント
		uint8_t count = 0;
		for (i = 0; i < 50; ++i) {
			count += ((press_fall_count >> i) & (0x1));
		}

		check_count++;
		if (check_count <= 100) {
			break;
		}

		//気圧の平均値が上昇した回数がPRESS_FALL_RATEを超えたら検知とする。あと強制開散もね！
		if (count >= (50 * PRESS_FALL_RATE)) {
			Save_buff (d_status);
			hstate = s_detectfalling;
			Save_buff(d_status);
			LPC_TMR32B0->TC = 0;
			LPC_TMR32B0->MR0 = 1000;
#if DEBUG
			printf("FALLING:%d\n", time());
#endif
		} else if (time() - etime.ignition_time > FORCED_OPENING_TIME) {
			Save_buff (d_status);
			hstate = s_detectfalling;
			Save_buff(d_status);
			LPC_TMR32B0->TC = 0;
			LPC_TMR32B0->MR0 = 1000;
#if DEBUG
			printf("FORCED FALLING:%d\n", time());
#endif
		}

		//TODO:debug
		static int count_t = 0;
		count_t++;
		if (count_t == 100) {
#if DEBUG
			printf("%d\n", count);
#endif
			count_t = 0;
		}
		//end debug
		return;
	}
		break;
	case s_detectfalling:
		LPC_TMR32B0->MR0 = 1000;
		Save_buff (d_status);
		etime.releaseparachute_time = time();
		hstate = s_releaseparachute;
		Save_buff(d_status);
#if DEBUG
		printf("OPENING:%d\n", time());
#endif
		SERVO_ON();
		break;
	case s_releaseparachute:	//TODO:ほんとは気圧検知にしたいんだけどめんどくさそー
		if (time() - etime.releaseparachute_time < CLOSE_VALVE_TIME)//時間経ってからねー
			break;
		VALVE_CLOSE();
		Save_buff(d_status);
		etime.closevalvetime = time();
		hstate = s_closevalve;
		Save_buff(d_status);
		LPC_TMR32B0->TC = 0;
		LPC_TMR32B0->MR0 = 2000;
#if DEBUG
		printf("CLOSE VALVE:%d\n", time());
#endif
		break;
	case s_closevalve:
		VALVE_CLOSE();
		LPC_TMR32B0->MR0 = 2000;
		if (time() - etime.closevalvetime < 16000)	//時間経ってからねー
			break;
		static uint8_t landing_count = 0;
		double sum;
		double x = (double) data.accel.x / 2048;
		double y = (double) data.accel.y / 2048;
		double z = (double) data.accel.z / 2048;
		sum = x * x + y * y + z * z;
		if ((sum >= 0.64) && (sum <= 1.44)) {
			landing_count++;
		} else {
			landing_count = 0;
		}
		//着地認定
		if (landing_count >= 50) {
			Save_buff(d_status);
			etime.landing_time = time();
			hstate = s_detectlanding;
			Save_buff(d_status);
#if DEBUG
			printf("LANDING:%d\n", time());
#endif
		} else if (time() - etime.closevalvetime >= FORCED_LANDING_TIME) {
			Save_buff(d_status);
			etime.landing_time = time();
			hstate = s_detectlanding;
			Save_buff(d_status);
#if DEBUG
			printf("FORCED_LANDING:%d\n", time());
#endif
		}
		break;
	case s_detectlanding:
		LPC_TMR32B0->TCR = 0;	//LEDきるやつ
		LPC_TMR32B0->TC = 0;
		LPC_GPIO3->DATA &= ~(1 << 3);
		if (time() - etime.landing_time < 10000)
			break;
		Save_buff(d_status);
		etime.stopdatasave_time = time();
		hstate = s_stopdatasave;
		Save_buff(d_status);
#if DEBUG
		printf("DATASAVE:%d\n", time());
#endif
		break;
	case s_stopdatasave:
		break;
	}
}

void Save_buff(Datatype d) {

	switch (d) {
	case d_accel: {
		uint8_t dataA[1];
		dataA[0] = 10;
		uint8_t dataB[100];
		uint8_t data[200];
		uint8_t datalength;
		int i;
		for (i = 0; i < BUFFSIZE; ++i) {
			dataB[i * 10] = accelbuff[i].time >> 24;
			dataB[i * 10 + 1] = accelbuff[i].time >> 16;
			dataB[i * 10 + 2] = accelbuff[i].time >> 8;
			dataB[i * 10 + 3] = accelbuff[i].time;
			dataB[i * 10 + 4] = accelbuff[i].x >> 8;
			dataB[i * 10 + 5] = accelbuff[i].x;
			dataB[i * 10 + 6] = accelbuff[i].y >> 8;
			dataB[i * 10 + 7] = accelbuff[i].y;
			dataB[i * 10 + 8] = accelbuff[i].z >> 8;
			dataB[i * 10 + 9] = accelbuff[i].z;
		}
		Encode(0x11, &dataA, &dataB, 1, 100, &data, &datalength);
		SD_write(&data, datalength, &fil);
		break;
	}
	case d_gyro: {
		uint8_t dataA[1];
		dataA[0] = 10;
		uint8_t dataB[100];
		uint8_t data[200];
		uint8_t datalength;
		int i;
		for (i = 0; i < BUFFSIZE; ++i) {
			dataB[i * 10] = gyrobuff[i].time >> 24;
			dataB[i * 10 + 1] = gyrobuff[i].time >> 16;
			dataB[i * 10 + 2] = gyrobuff[i].time >> 8;
			dataB[i * 10 + 3] = gyrobuff[i].time;
			dataB[i * 10 + 4] = gyrobuff[i].x >> 8;
			dataB[i * 10 + 5] = gyrobuff[i].x;
			dataB[i * 10 + 6] = gyrobuff[i].y >> 8;
			dataB[i * 10 + 7] = gyrobuff[i].y;
			dataB[i * 10 + 8] = gyrobuff[i].z >> 8;
			dataB[i * 10 + 9] = gyrobuff[i].z;
		}
		Encode(0x12, &dataA, &dataB, 1, 100, &data, &datalength);
		SD_write(&data, datalength, &fil);
		break;
	}
	case d_mag:

		break;
	case d_bigaccel:

		break;
	case d_press_temp: {
		uint8_t dataA[1];
		dataA[0] = 10;
		uint8_t dataB[100];
		uint8_t data[200];
		uint8_t datalength;
		int i;
		for (i = 0; i < BUFFSIZE; ++i) {
			dataB[i * 9] = pressbuff[i].time >> 24;
			dataB[i * 9 + 1] = pressbuff[i].time >> 16;
			dataB[i * 9 + 2] = pressbuff[i].time >> 8;
			dataB[i * 9 + 3] = pressbuff[i].time;
			dataB[i * 9 + 4] = pressbuff[i].press >> 16;
			dataB[i * 9 + 5] = pressbuff[i].press >> 8;
			dataB[i * 9 + 6] = pressbuff[i].press;
			dataB[i * 9 + 7] = tempbuff[i].temp >> 8;
			dataB[i * 9 + 8] = tempbuff[i].temp;
		}
		Encode(0x16, &dataA, &dataB, 1, 90, &data, &datalength);
		SD_write(&data, datalength, &fil);
		break;
	}
	case d_status: {
		uint8_t dataA[1];
		uint8_t dataB[5];
		uint8_t data[50];
		uint8_t datalength;
		uint32_t t = time();
		uint32_t status = 0;
		dataB[0] = t >> 24;
		dataB[1] = t >> 16;
		dataB[2] = t >> 8;
		dataB[3] = t;
		//HisyoState(3bit)、電磁弁、サーボ、充電状態、ぐらいかなあ？？？
		dataB[4] = hstate << 3;
		if ((LPC_GPIO2->DATA >> 6) & 0x1) {
			dataB[4] |= (1 << 2);
			//電磁弁OPEN
		}
		if (LPC_TMR16B0->MR0 == 22) {
			dataB[4] |= (1 << 1);
			//サーボON
		}
		if (!((LPC_GPIO3->DATA >> 3) & 0x1)) {
			dataB[4] |= (1 << 0);
			//充電ON
		}
		Encode(0x03, &dataA, &dataB, 0, 5, &data, &datalength);
		SD_write(&data, datalength, &fil);
	}

	default:
		break;
	}
}
