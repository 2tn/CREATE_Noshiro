/*
 * SD.h
 *
 *  Created on: 2015/08/06
 *      Author: Yusuke
 */

#ifndef SD_H_
#define SD_H_

#endif /* SD_H_ */
#include "fatfs/ff.h"

extern void ItoC(uint16_t num, TCHAR *res);
extern int nzyou(int n);
extern FRESULT SD_initialize(FIL *fil);
extern FRESULT SD_save(FIL *fil);
extern FRESULT SD_close(FIL *fil);
extern FRESULT SD_write(uint8_t *data, uint16_t len, FIL *fil);

typedef enum SDState {
	t_initial = 0, t_notassert, t_assert,
} SDState;
