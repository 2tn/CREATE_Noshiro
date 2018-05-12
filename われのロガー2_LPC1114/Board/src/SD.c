/*
 * SD.c
 *
 *  Created on: 2015/08/06
 *      Author: Yusuke
 */

//TODO:はよ書け!!!
#include "fatfs/ff.h"
#include "LPC11xx.h"
#include "SD.h"

volatile FATFS FatFs;
FRESULT fr; /* 戻り値 */
SDState sstate = t_initial;

FRESULT SD_initialize(FIL *fil) {	//ファイルが存在したらだんだんインクリメントするやつ
	fr = f_mount(&FatFs, "", 0);
	int fcount = 0;
	fr = 1;
	while (fr) {	//TODO:
		TCHAR filename[8] = "0000.txt";
		ItoC(fcount, &filename);
		fr = f_open(fil, "0004.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
		switch (fr) {
		case FR_NOT_READY:
			sstate = t_notassert;
			return fr;
			break;
		case FR_OK:
			sstate = t_assert;
			break;
		}
		fcount++;
	}
	return 0;
}

FRESULT SD_save(FIL *fil) {
	fr = f_sync(fil);
	return fr;
}
FRESULT SD_close(FIL *fil) {
	fr = f_close(fil);
	return fr;
}
FRESULT SD_write(uint8_t *data, uint16_t len, FIL *fil) {
	UINT length = len;
	UINT bw;
	fr = f_lseek(fil, f_size(fil));
	fr = f_write(fil, data, length, &bw);
	return fr;
}

void ItoC(uint16_t num, TCHAR *res) {	//atoi使えないのなんなんだー
	int i;
	for (i = 4; i > 0; i--) {
		*res = ((num % nzyou(i)) / nzyou(i - 1)) + 48;
		res++;
	}
}
int nzyou(int n) {	//math.hはつかえん
	int res = 1;
	int i;
	for (i = 0; i < n; ++i) {
		res *= 10;
	}
	return res;
}
