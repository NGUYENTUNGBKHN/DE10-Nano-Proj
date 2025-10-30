/******************************************************************************/
/*! @addtogroup Main
    @file       version_section_header.c
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

const BA_SectionHeader ex_version_header    __attribute__ ((section ("VERSION_SECTION_HEADER"),used))  =
{
	BA_SYMBOL,                		/* File Header						*/
	0x00000000,						/* CRC-32							*/
	0x0000,							/* CRC-16		 					*/
	0x0006,							/* section no						*/
	SWAP_ENDIAN(0x00680000 - DDR_START_ADDRESS),		/* Program start address			*/
	SWAP_ENDIAN(0x0068FFFF - DDR_START_ADDRESS),		/*    	  end address				*/
	0x00000000,						/* Program size						*/
	"VER ",							/* section name						*/
	0x00000000,						/* reserve							*/
	0x00000000,						/* reserve							*/
};

/* EOF */
