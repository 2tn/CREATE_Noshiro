/*
 * comm.c
 *
 *  Created on: 2015/07/21
 *      Author: Yusuke
 */

/*
 ===============================================================================
 Name        : LPC1114FN28_UARTtest.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

//This is echo-back program.
#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include "comm.h"
#include "interface.h"
#include "format.h"
// TODO: insert other include files here

// TODO: insert other definitions and declarations here

void UART_IRQHandler(void) {
	uint8_t c = UART_Read();
	uint8_t data = c & (0b111111);
	uint8_t rawtype = c >> 6;
	switch (rawtype) {
	// TODO : タイマー仕掛けてーたいむあうとさせてー
	case 0b00:
		COMM_STATUS = data;
		break;
	case 0b01:
		if (!COMM_STATUS) {
			break;
		}
		RdataA6[A6length] = data;
		A6length++;
		break;
	case 0b10:
		if (!COMM_STATUS) {
			break;
		}
		RdataB6[B6length] = data;
		B6length++;
		break;
	case 0b11:
		if (!COMM_STATUS) {
			break;
		}
		C6to8(&RdataA6, A6length, &RdataA, &Alength);
		C6to8(&RdataB6, B6length, &RdataB, &Blength);

		switch (COMM_STATUS) {
		case 0x20:	//使用しないらしい
			ACK();
			break;
		case 0x21:	//とりあえずpingだけ実装してみた―
			UART_SendChar(0x01);	//ping返信
			UART_SendChar(0xC0);	//まあ一応EOPもね
			break;
		case 0x22:	//ACKだぜ！
			printf("ACK\n");
			break;
		case 0x23:	//ステータス欲しい
		{
			uint8_t status[8] =
					{ 0x00, 0x00, 0x32, 0xd2, 0x32, 0xfd, 0xd7, 0x21 };
			Send(0x03, &nulldata, &status, 0, 8);
		}
			break;
		case 0x24:	//サーボOFFにして
			SERVO_OFF();
			ACK();
			break;
		case 0x25:	//サーボONにしよう！！
			SERVO_ON();
			ACK();
			break;
		case 0x2e:	//NACK
			printf("NACK\n");
			break;
		case 0x2F:	//リセットしてほしいやつ
			if ((RdataA[0] == 0x4d) && (RdataB[0] == 0x1E)) {
				ACK();
				ResetISR();
			} else {
				NACK();
			}
			break;
		default:
			break;
		}
		//変数たちの初期化
		COMM_STATUS = 0;
		A6length = 0;
		B6length = 0;
		break;
	default:
		break;
	}
}

//150byte送信までしか保証できないよー
void Send(uint8_t command, uint8_t *dataA, uint8_t *dataB, uint8_t Alen,
		uint8_t Blen) {
	uint8_t sendlen = 0;
	uint8_t senddata[150] = { 0 };
	Encode(command,dataA,dataB,Alen,Blen,&senddata,&sendlen);
	int i;
	for (i = 0; i <sendlen; i++) {
		UART_SendChar(senddata[i]);
	}
}

void ACK() {
	UART_SendChar(0x02);	//ACK
	UART_SendChar(0xC0);
}
void NACK() {
	UART_SendChar(0x0E);	//NACK
	UART_SendChar(0xC0);
}
