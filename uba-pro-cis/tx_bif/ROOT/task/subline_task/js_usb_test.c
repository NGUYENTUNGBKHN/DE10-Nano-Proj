/****************************************************************************/
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2018                          */
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
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の                                                         */
/* 企業機密情報含んでいます。                                                                                                                           */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を                                                     */
/* 公開も複製も行いません。                                                                                                                                */
/*                                                                          */
/****************************************************************************/
/******************************************************************************/
/**
 * MODEL NAME : RBA-40 CIS
 * @file js_uart_test.c
 * @brief シリアル通信ペリフェラル実装ファイル。
 * @date 2018/01/24
 * @author JCM. Tokyo R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
*******************************************************************************///
/***************************** Include Files *********************************/
#include "kernel.h"
#include <string.h>
#include "common.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "hal.h"
#include "js_usb_test.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

/*==============================================================================*/
/* デバッグトレース宣言(有効にするとトレース出力あり)							*/
/*==============================================================================*/
#define	DBG_ERR()			//osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#define DBG_TRACE1(...)		//osw_printf(__VA_ARGS__)
#define DBG_TRACE2(...)		//osw_printf(__VA_ARGS__)
#if DBG_ERR_ALL_ENABLE
#ifdef DBG_ERR
#undef DBG_ERR
#define	DBG_ERR() osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#endif
#endif


/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/

void usb_test_init(void)
{
	OperationUSBConnect();
}

void usb_test_terminate(void)
{
	OperationUSBDisconnect();
}

/* EOF */
