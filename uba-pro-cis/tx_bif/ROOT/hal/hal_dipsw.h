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
 * @file hal_dipsw.c
 * @brief DIPSWのハードウェアレイヤーのヘッダファイル
 * @date 2018.01.26 Created
 */
/****************************************************************************/
#if !defined(__HAL_DIPSW_H_INCLUDED__)
#define __HAL_DIPSW_H_INCLUDED__

#include "hal_gpio_reg.h"
/****************************************************************/
/*							関数宣言							*/
/****************************************************************/
u32 _hal_read_dipsw1(u8 *dipsw_value);
u32 _hal_read_dipsw2(u8 *dipsw_value);

#endif /* __HAL_DIPSW_H_INCLUDED__ */

