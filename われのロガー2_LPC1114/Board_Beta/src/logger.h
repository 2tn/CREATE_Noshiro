/*
 * logger.h
 *
 *  Created on: 2015/08/06
 *      Author: Yusuke
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#define ACCEL_FIFO_DATALENGTH	5
#define PRESS_FIFO_DATALENGTH	50
#define PRESS_FALL_RATE			0.95
#define FORCED_OPENING_TIME		10000
#define CLOSE_VALVE_TIME		10000
#define FORCED_LANDING_TIME		300000

#define IGNITION_ACCEL		2
#define IGNITION_TIME		500
extern void Loop();
extern void Timer_initialize();
void TIMER32_0_IRQHandler(void);
uint32_t time();

typedef struct {
	uint32_t ignition_time;
	uint32_t releaseparachute_time;
	uint32_t closevalvetime;
	uint32_t landing_time;
	uint32_t stopdatasave_time;
} EVENT_TIME;

enum HisyoState {
	s_initial = 0,
	s_detectignition,
	s_detectfalling,
	s_releaseparachute,
	s_closevalve,
	s_detectlanding,
	s_stopdatasave
};

#endif /* LOGGER_H_ */
