/******************************************************************************/
/*! @addtogroup Main
    @file       template_blank_section_header.c
    @brief      TEMPLATE BLANKセクションヘッダーデータ。
    @date       2018/01/26
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
#include "common.h"
#include "memorymap.h"

/************************** EXTERNAL FUNCTIONS *************************/

/***********************************************************************/

const BA_SectionHeader ex_template_blank_header     __attribute__ ((section ("TEMPLATE_BLANK_SECTION_HEADER"),used))  =
{
	BA_SYMBOL,                		/* File Header						*/
	0x00000000,						/* CRC-32							*/
	0x0000,							/* CRC-16		 					*/
	0x0008,							/* section no( 0xffff == last section)	*/
	SWAP_ENDIAN(0x00890000 - DDR_START_ADDRESS),		/* Program start address			*/
	//SWAP_ENDIAN(0x00EFFFFF - DDR_START_ADDRESS),		/*    	  end address				*/
	SWAP_ENDIAN(0x00DEFFFF - DDR_START_ADDRESS),		/*    	  end address				*///RCダウンロードファイルの為に後ろを64k空けた
	0x00000000,						/* Program size						*/
	"BLAN",							/* section name						*/
	0x00000000,						/* reserve							*/
	0x00000000,						/* reserve							*/
};

/* EOF */
