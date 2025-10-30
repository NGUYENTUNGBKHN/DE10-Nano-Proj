/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file version.c
 * @brief バージョン番号を格納しています。
 * @date 2018.03.06 Created.
 */
/****************************************************************************/
#include "systemdef.h"
#include "version.h"

u8 ex_firmware_version[64];
u8 ex_boot_version[3];

ST_ROM_VERSION ex_stRomVer __attribute__ ((section ("ROMNO"),used)) = {
	//*** Farmware Ver. ***

	{"18SEP20 M01001"},
//		   │  │└***:アルゴ Ver
//		   │  └─**:I/F Main Ver
//		   └───更新日付 DDMMMYY ※MMMは月の省略英語

	//*** ROM ID ***
	{"3561M0""1"},
//		  │  └──基板改定:*
//		  └────固定 ID(電子図番+M+*)
//						I/F MAIN:"3561M0"
//						I/F DOWN:"3561MD"
//						2nd BOOT:"3561MB"
};


/* End of file */
