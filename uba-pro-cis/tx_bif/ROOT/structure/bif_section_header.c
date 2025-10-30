/******************************************************************************/
/*! @addtogroup Main
    @file       bif_section_header.c
    @brief      Boot I/Fセクションヘッダーデータ。
    @date       2018/01/25
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
#include "common.h"
#include "memorymap.h"

/************************** EXTERNAL FUNCTIONS *************************/

/***********************************************************************/

const BA_SectionHeader ex_bif_header    __attribute__ ((section ("BIF_SECTION_HEADER"),used))  =
{
	BA_SYMBOL,                		/* File Header						*/
	0x00000000,						/* CRC-32							*/
	0x0000,							/* CRC-16		 					*/
	0x0002,							/* section no						*/
	SWAP_ENDIAN(0x00180100 - DDR_START_ADDRESS),		/* Program start address			*/
	SWAP_ENDIAN(0x0027FFFF - DDR_START_ADDRESS),		/*    	  end address				*/
	0x00000000,						/* Program size						*/
	"BIF ",							/* section name						*/
	0x00000000,						/* reserve							*/
	0x00000000,						/* reserve							*/
};

/* EOF */
