/*
 ===============================================================================
 Name        : Board_Communication.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif
#define BAUDRATE	115200
#include <cr_section_macros.h>

#include <stdio.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

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

void UART_Configuration();
void UART_putc(char data);
void UART_puts(char *data);
uint8_t UART_getc();
void C6to8(uint8_t *data6, uint8_t len6, uint8_t *data8, uint8_t *len8);
void C8to6(uint8_t *data6, uint8_t *len6, uint8_t *data8, uint8_t len8);
void Send(uint8_t command, uint8_t *dataA, uint8_t *dataB, uint8_t Alen,
		uint8_t Blen);
void ACK();
void NACK();

void UART_IRQHandler(void) {
	uint8_t c = UART_getc();
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
			UART_putc(0x01);	//ping返信
			UART_putc(0xC0);	//まあ一応EOPもね
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
			ACK();
			break;
		case 0x25:	//サーボONにしよう！！
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

int main(void) {

	UART_Configuration();
	//UART_puts("Hello World\n");

	uint8_t data_A[1] = { 0x06 };
	uint8_t data_B[1] = { 0x04 };
	Send(0x04, &data_A, &data_B, 1, 1);
	// Force the counter to be placed into memory
	volatile static int i = 0;
	// Enter an infinite loop, just incrementing a counter
	while (1) {
		i++;
	}
	return 0;
}

void UART_Configuration() {
	uint32_t DL;
	LPC_IOCON->PIO3_0 = 0x03;
	LPC_IOCON->PIO3_1 = 0x03;
	LPC_IOCON->RXD_LOC = 0x02;
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12);
	LPC_SYSCON->UARTCLKDIV = 0x01;
	DL = (SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV)
			/ (16 * BAUDRATE * LPC_SYSCON->UARTCLKDIV);
	LPC_UART->LCR |= (1 << 7);
	LPC_UART->DLM = DL / 256;
	LPC_UART->DLL = DL % 256;
	LPC_UART->LCR &= ~(1 << 7);
	LPC_UART->LCR = 0x03;
	LPC_UART->FCR = 0x07;
	LPC_UART->IER |= 0x01;

	NVIC_EnableIRQ(UART_IRQn);
	NVIC_SetPriority(UART_IRQn, 3);
}

void UART_putc(char data) {
	while (!(LPC_UART->LSR & (1 << 5)))
		;
	LPC_UART->THR = data;
}

void UART_puts(char *data) {
	while (*data != '\0') {
		UART_putc(*data);
		data++;
	}
}

uint8_t UART_getc() {
	while (!(LPC_UART->LSR & 0x01))
		;
	return LPC_UART->RBR;
}

void C6to8(uint8_t *data6, uint8_t len6, uint8_t *data8, uint8_t *len8) {
	register int si = 0;
	register int di = 0;
	*len8 = len6 * 3 / 4;		//†常識的長さ†ならオーバーフローしない
	for (; di <= (*len8); di++) {
		switch (di % 3) {
		case 0:
			data8[di] = data6[si++] << 2;
			data8[di] |= data6[si] >> 4;
			break;
		case 1:
			data8[di] = data6[si++] << 4;
			data8[di] |= data6[si] >> 2;
			break;
		case 2:
			data8[di] = data6[si++] << 6;
			data8[di] |= data6[si++];
			break;
		}
	}
}
void C8to6(uint8_t *data6, uint8_t *len6, uint8_t *data8, uint8_t len8) {
	register int si = 0;
	register int di = 0;
	register char rest;
	for (; si <= len8; si++) {
		switch (si % 3) {
		case 0:
			if (si >= len8) {
				data6[di++] = 0 >> 2;
				di--;
			} else {
				data6[di++] = data8[si] >> 2;
				rest = data8[si] & 0b000011;
			}
			break;
		case 1:
			if (si >= len8) {
				data6[di++] = (rest << 4) | (0 >> 4);
			} else {
				data6[di++] = (rest << 4) | (data8[si] >> 4);
				rest = data8[si] & 0b001111;
			}
			break;
		case 2:
			if (si >= len8) {
				data6[di++] = (rest << 2) | (0 >> 6);
				data6[di++] = 0 & 0b111111;
				di--;
			} else {
				data6[di++] = (rest << 2) | (data8[si] >> 6);
				data6[di++] = data8[si] & 0b111111;
			}
			break;
		}
	}
	//TODO:終端の処理
	*len6 = di;
}

void Send(uint8_t command, uint8_t *dataA, uint8_t *dataB, uint8_t Alen,
		uint8_t Blen) {
	uint8_t sendlen = 0;
	uint8_t senddata[150] = { 0 };
	Encode(command,dataA,dataB,Alen,Blen,&senddata,&sendlen);
	int i;
	for (i = 0; i <sendlen; i++) {
		UART_putc(senddata[i]);
	}
}

void Encode(uint8_t command, uint8_t *dataA, uint8_t *dataB, uint8_t Alen,
		uint8_t Blen, uint8_t *outdata, uint8_t *outlen) {
	uint8_t dataA6[200] = { 0 };
	uint8_t dataB6[200] = { 0 };
	uint8_t A6len = 0;
	uint8_t B6len = 0;
	if (Alen) {
		C8to6(&dataA6, &A6len, dataA, Alen);
	}
	if (Blen) {
		C8to6(&dataB6, &B6len, dataB, Blen);
	}
	*outlen = 2 + A6len + B6len;
	outdata[0] = command;
	if (Alen) {
		int i;
		for (i = 0; i < A6len; i++) {
			outdata[1 + i] = (dataA6[i] | (0b01000000));
		}

	}
	if (Blen) {
		int i;
		for (i = 0; i < B6len; i++) {
			outdata[1 + A6len + i] = (dataB6[i] | (0b10000000));
		}
	}
	outdata[1 + A6len + B6len] = 0xC0;

}

void ACK() {
	UART_putc(0x02);	//ACK
	UART_putc(0xC0);
}
void NACK() {
	UART_putc(0x0E);	//NACK
	UART_putc(0xC0);
}
