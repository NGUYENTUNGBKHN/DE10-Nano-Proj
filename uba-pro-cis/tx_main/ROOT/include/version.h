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
 * @file version.h
 * @brief バージョン番号
 * @date 2019.12.11 Created.
 */
/****************************************************************************/
#ifndef _VERSION_H_
#define _VERSION_H_

#define ROM_VER_SOFT_SIZE	32
#define ROM_VER_ID_SIZE		32

typedef struct
{
	u8 cSoftVer[ROM_VER_SOFT_SIZE];
	u8 cId[ROM_VER_ID_SIZE];
}ST_ROM_VERSION;

extern ST_ROM_VERSION ex_stRomVer;
extern u8 ex_firmware_version[64];
extern u8 ex_boot_version[3];

#endif					// _VERSION_H_
