/*
 * comm.h
 *
 *  Created on: 2015/07/21
 *      Author: Yusuke
 */

#ifndef COMM_H_
#define COMM_H_

#endif /* COMM_H_ */

#define IER_RBR         (0x01<<0)
#define IER_THRE        (0x01<<1)
#define IER_RLS         (0x01<<2)
#define IER_ABEO        (0x01<<8)
#define IER_ABTO        (0x01<<9)

#define IIR_PEND        0x01
#define IIR_RLS         0x03
#define IIR_RDA         0x02
#define IIR_CTI         0x06
#define IIR_THRE        0x01
#define IIR_ABEO        (0x01<<8)
#define IIR_ABTO        (0x01<<9)

uint8_t COMM_STATUS = 0;

uint8_t nulldata[0];

uint8_t RdataA6[200];
uint8_t RdataB6[200];
uint8_t A6length = 0;
uint8_t B6length = 0;

uint8_t RdataA[150];
uint8_t RdataB[150];
uint8_t Alength = 0;
uint8_t Blength = 0;

extern void Send(uint8_t command, uint8_t *dataA, uint8_t *dataB, uint8_t Alen,
		uint8_t Blen) ;
extern void ACK();
extern void NACK();
void UART_IRQHandler(void) ;
