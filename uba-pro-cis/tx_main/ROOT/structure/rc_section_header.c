/******************************************************************************/
/*! @addtogroup Main
    @file       template_blank_section_header.c
    @brief      RCダウンロードファイル用セクションヘッダーデータ。
    @date       2025/06/13
    @author     
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
#include "common.h"
#include "memorymap.h"

/************************** EXTERNAL FUNCTIONS *************************/

/***********************************************************************/
/* RCダウンロードファイル用 */
const BA_SectionHeader rc_section_header     __attribute__ ((section ("RC_SECTION_HEADER_BASE"),used))  =
{
	BA_SYMBOL,                		/* File Header						*/
	0x00000000,						/* CRC-32							*/
	0x0000,							/* CRC-16		 					*/
	0x0009,							/* section no( 0xffff == last section)	*/
	SWAP_ENDIAN(0x00DF0000 - DDR_START_ADDRESS),		/* Program start address			*/
	SWAP_ENDIAN(0x00EFFFFF - DDR_START_ADDRESS),		/*    	  end address				*/
	0x00000000,						/* Program size						*/
	"RC  ",							/* section name						*/
	0x00000000,						/* reserve							*/
	0x00000000,						/* reserve							*/
};

/* EOF */
