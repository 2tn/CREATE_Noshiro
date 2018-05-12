/*
 * format.c
 *
 *  Created on: 2015/08/06
 *      Author: Yusuke
 */

#include "format.h"
#include "LPC11xx.h"
#include <stdio.h>
//150byte送信までしか保証できないよー
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
