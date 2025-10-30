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
 * @file kernel_config.h
 * @brief カーネル設定の関数を格納しています。
 * @date 2018.01.05 Created.
 */
/****************************************************************************/
#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#include <stdbool.h>
#include "kernel.h"
#include "MP_GIC.h"
#include "js_oswapi.h"

// ThreadX-uitron4はISRを未サポートのため構造体を宣言する, 19/04/10
typedef struct {
	ATR isratr;
	VP_INT exinf;
	INTNO intno;
	FP isr;
	UINT imask;
} T_CISR;

/****************************************************************/
/*						外部変数宣言								*/
/****************************************************************/
#ifdef TX_ENABLE_EVENT_TRACE
extern UCHAR	event_buffer[65536];
#endif
/****************************************************************/
/*						関数宣言								*/
/****************************************************************/
bool system_mpl(void* p);
void kernel_config(void);


#endif		// KERNEL_CONFIG_H
