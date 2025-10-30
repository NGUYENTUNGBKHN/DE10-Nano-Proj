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
 * @file systemdef.h
 * @brief システム定義
 * @date 2012.11.16 Created.
 */
/****************************************************************************/
#ifndef __SYSTEMDEF_H_INCLUDED__
#define __SYSTEMDEF_H_INCLUDED__

#include <stdint.h>
//#include <stdio.h>
#include "jcm_typedef.h"				/* JCM型定義ヘッダファイル */

#if defined(__cplusplus)
extern "C" { // }
#endif

#include "kernel.h"
#if defined(PRJ_OS_UC3)
#include "CycloneV_uC3.h"
#else
#endif
#if 0
#include "ivizion2_fpgaio.h"			/* BAU-LE17 FPGA I/O定義 */

#include "version.h"

#include "rtc.h"
#include "clock.h"
#include "kernel_inc.h"				/* カーネルヘッダファイル */

#include "common_inc.h"							/* 共通関数ヘッダファイル */
#include "common_ram.h"							/* 共通変数ヘッダファイル */
#endif

#if 0
#undef ID_CODE0
#undef ID_CODE1

#define ID_CODE0	(1)
#define ID_CODE1	(0)
#endif

#include "js_oswapi.h"
#include "js_intc_reg.h"
#include "js_io.h"
#include "js_uart.h"
#include "js_crc16.h"
#include "js_gpio.h"
#if 0
#include "crc.h"
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __SYSTEMDEF_H_INCLUDED__ */

