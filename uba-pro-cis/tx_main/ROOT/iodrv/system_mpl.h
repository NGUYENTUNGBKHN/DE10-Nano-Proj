/****************************************************************************/
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
 * @file system_mpl.h
 * @brief システムメモリプール関数 ヘッダファイル
 * @date 2018.06.06 Created.
 */
/****************************************************************************/
#ifndef SYSTEM_MPL_H
#define SYSTEM_MPL_H

#include <stdint.h>

/****************************************************************/
/*							関数宣言							*/
/****************************************************************/
void *get_system_mpl(uint32_t size);
void release_system_mpl(void *p);


#endif		// SYSTEM_MPL_H
/* End of file */
