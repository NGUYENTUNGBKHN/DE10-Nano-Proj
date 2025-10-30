/******************************************************************************/
/*! @addtogroup Main
    @file       main_data_section_header.c
    @brief      Soft versionセクションヘッダーデータ。
    @date       2022/04/05
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2022 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
#include "common.h"
#include "memorymap.h"

/************************** EXTERNAL FUNCTIONS *************************/

/***********************************************************************/

const BA_SectionHeader ex_main_data_header    __attribute__ ((section ("MAIN_DATA_SECTION_HEADER"),used))  =
{
	BA_SYMBOL,                		/* File Header						*/
	0x00000000,						/* CRC-32							*/
	0x0000,							/* CRC-16		 					*/
	0x0004,							/* section no						*/
	SWAP_ENDIAN(0x00470000 - DDR_START_ADDRESS),		/* Program start address			*/
	SWAP_ENDIAN(0x0047FFFF - DDR_START_ADDRESS),		/*    	  end address				*/
	0x00000000,						/* Program size						*/
	"DATA",							/* section name						*/
	0x00000000,						/* reserve							*/
	0x00000000,						/* reserve							*/
};

/* EOF */
