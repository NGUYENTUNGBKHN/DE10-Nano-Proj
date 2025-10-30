/******************************************************************************/
/*! @addtogroup bif
    @file       rom_header.c
    @date       2018/01/25
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * rom_header.c
 *
 *  Created on: 2018/01/25
 *      Author: suzuki-hiroyuki
 */
#include "common.h"
// section _ROM_HEADER : start = 0x180000, size = 0x100
// ROM file header [0x00-0x17]
// 検索用"VFM20"
const unsigned char file_header[0x18] __attribute__ ((section ("ROM_HEADER"),used))  =
{
#if (defined(PRJ_IVIZION2) && (BV_UNIT_TYPE>=WS2_MODEL))
	'V','F','M','2','0',' ',		/* (0000) File Header						*/
#else
	'V','F','M','2','1',' ',		/* (0000) File Header						*/
#endif
	0x00,0x08,0x00,0x00,			/* (0006) Program start address				*/
	0x00,0xff,0xff,0xff,			/* (000a)   	  end address				*/
	0x00,0x01,						/* (000e) Boot File number 					*/
	0x00,0x00,0x00,0x00,			/* (0010) 									*/
	0x00,0x00,0x00,0x00				/* (0014) 									*/
};
