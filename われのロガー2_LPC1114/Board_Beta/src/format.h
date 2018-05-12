/*
 * format.h
 *
 *  Created on: 2015/08/06
 *      Author: Yusuke
 */

#ifndef FORMAT_H_
#define FORMAT_H_


#include "LPC11xx.h"

#endif /* FORMAT_H_ */
void Encode(uint8_t command, uint8_t *dataA, uint8_t *dataB, uint8_t Alen,
		uint8_t Blen, uint8_t *outdata, uint8_t *outlen);
void C6to8(uint8_t *data6, uint8_t len6, uint8_t *data8, uint8_t *len8);
void C8to6(uint8_t *data6, uint8_t *len6, uint8_t *data8, uint8_t len8);
