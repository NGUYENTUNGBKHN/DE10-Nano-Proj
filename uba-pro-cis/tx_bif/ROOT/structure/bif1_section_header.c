/******************************************************************************/
/*! @addtogroup Main
    @file       bif1_section_header.c
    @date       2018/01/25
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * bif1_section_header.c
 *
 *  Created on: 2018/01/25
 *      Author: suzuki-hiroyuki
 */

// Boot I/F 1セクションヘッダーデータ。
#include "common.h"
#include "memorymap.h"

/*------ internal section header -----*/
const BA_SectionHeader ex_bif1_Header   __attribute__ ((section ("BIF1_SECTION_HEADER"),used))  =
{
	BA_SYMBOL,                		/* File Header						*/
	0x00000000,						/* CRC-32							*/
	0x0000,							/* CRC-16		 					*/
	0xffff,							/* section no						*/
	SWAP_ENDIAN(0x00180200 - DDR_START_ADDRESS),		/* Program start address			*/
	SWAP_ENDIAN(0x001FFFFF - DDR_START_ADDRESS),		/*    	  end address				*/
	0x00000000,						/* Program size						*/
	"BIF1",							/* section name						*/
	0x00000000,						/* reserve							*/
	0x00000000,						/* reserve							*/
};

