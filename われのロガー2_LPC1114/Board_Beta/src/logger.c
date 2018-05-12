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
#include <stdio.h>

extern SENSORDATA data;
extern EVENT_TIME etime;
enum HisyoState hstate = s_initial;

void Timer_initialize(){

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 9);
	LPC_TMR32B0->PR = 4800 - 1;	//10kHz
	LPC_TMR32B0->MCR |= (1 << 0);
	LPC_TMR32B0->MCR |= (1 << 1);
	LPC_TMR32B0->MR0 = 10; 		//5～24まで
	LPC_TMR32B0->TCR = 1;


	NVIC_EnableIRQ(TIMER_32_0_IRQn);
	NVIC_SetPriority(TIMER_32_0_IRQn,1);

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 10);
	LPC_TMR32B1->PR = 48000 - 1;	//1kHz
	LPC_TMR32B1->TCR = 1;

}

//timerたりてないから暫定処置
void TIMER32_0_IRQHandler(void) {
	uint8_t IR = LPC_TMR32B0->IR;
	if (((IR >> 0) & 0x1) == 0x1) {
		LPC_TMR32B0->IR |= (1 << 0);
	}
}


uint32_t time(){
	return LPC_TMR32B1->TC;
}

void Loop() { //10msに一回シュコシュコしてくださーい

	switch (hstate) {
	case s_initial: {	//点火検知セクション
		static double accel_fifo[ACCEL_FIFO_DATALENGTH];
		static int over_accel = 0;

		double sum;
		int i;
		for (i = 0; i < 3; i++) {
			double val = (double) data.accel[i] / 2048;
			sum += (val * val);
		}
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
			etime.ignition_time = time();
			hstate = s_detectignition;
			printf("IGNITION:%d\n", etime.ignition_time);
		}
	}
		break;

	case s_detectignition: {	//パラシュート解放
		int i;
		static uint32_t check_count = 0;
		static uint32_t press_fifo[PRESS_FIFO_DATALENGTH];
		static uint64_t press_fall_count = 0;

		//PRESS_FIFO_DATALENGTH分だけ平均化
		for (i = PRESS_FIFO_DATALENGTH - 1; i > 0; i--) {
			press_fifo[i] = press_fifo[i - 1];
		}
		press_fifo[0] = data.press;
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
		//printf("%d\n", count);

		check_count++;
		if (check_count <= 100) {
			break;
		}

		//気圧の平均値が上昇した回数がPRESS_FALL_RATEを超えたら検知とする。あと強制開散もね！
		if (count >= (50 * PRESS_FALL_RATE)) {
			hstate = s_detectfalling;
			printf("FALLING:%d\n", time());
		} else if (time() - etime.ignition_time > FORCED_OPENING_TIME) {
			hstate = s_detectfalling;
			printf("FORCED FALLING:%d\n", time());
		}

		//TODO:debug
		static int count_t = 0;
		count_t++;
		if (count_t == 100) {
			printf("%d\n", count);
			count_t = 0;
		}
		//end debug
		return;
	}
		break;
	case s_detectfalling:
		etime.releaseparachute_time = time();
		hstate = s_releaseparachute;
		printf("OPENING:%d\n", time());
		SERVO_ON();
		break;
	case s_releaseparachute:	//TODO:ほんとは気圧検知にしたいんだけどめんどくさそー
		if (time() - etime.releaseparachute_time < CLOSE_VALVE_TIME)	//時間経ってからねー
			break;
		VALVE_CLOSE();
		etime.closevalvetime = time();
		hstate = s_closevalve;
		printf("CLOSE VALVE:%d\n", time());
		break;
	case s_closevalve:
		if (time() - etime.closevalvetime < 16000)	//時間経ってからねー
			break;
		static uint8_t landing_count = 0;
		double sum;
		int i;
		for (i = 0; i < 3; i++) {
			double val = (double) data.accel[i] / 2048;
			sum += (val * val);
		}
		if ((sum >= 0.64) && (sum <= 1.44)) {
			landing_count++;
		} else {
			landing_count = 0;
		}
		//着地認定
		if (landing_count >= 50) {
			etime.landing_time = time();
			hstate = s_detectlanding;
			printf("LANDING:%d\n", time());
		} else if (time() - etime.closevalvetime >= FORCED_LANDING_TIME) {
			etime.landing_time = time();
			hstate = s_detectlanding;
			printf("FORCED_LANDING:%d\n", time());
		}
		break;
	case s_detectlanding:
		if (time() - etime.landing_time < 10000)
			break;
		etime.stopdatasave_time = time();
		hstate = s_stopdatasave;
		printf("DATASAVE:%d\n", time());
		break;
	case s_stopdatasave:
		break;
	}
}
