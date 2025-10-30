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
 * @file wdt_task.h
 * @brief ＷＤＴタスクのヘッダファイル
 * @date 2018/01/25 Created.
 */
/****************************************************************************/
#if !defined(__WDT_TASK_H_INCLUDED__)
#define __WDT_TASK_H_INCLUDED__

#include <stdbool.h>
#include <stdint.h>

/****************************************************************/
/*						関数宣言								*/
/****************************************************************/
void isr_wdt(void);
void wdt_task(void);
bool wdt_get_flag(void);
uint32_t wdt_get_powerup_factor(void);
#endif /* __WDT_TASK_H_INCLUDED__ */


/* End of file */

