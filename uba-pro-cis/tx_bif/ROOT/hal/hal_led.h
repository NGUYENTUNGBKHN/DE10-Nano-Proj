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
 * @file hal_led.c
 * @brief LEDの点灯／消灯のハードウェアレイヤーのヘッダファイル
 * @date 2018.01.26 Created
 */
/****************************************************************************/
#if !defined(__HAL_LED_H_INCLUDED__)
#define __HAL_LED_H_INCLUDED__

#include "hal_gpio_reg.h"
#include "custom.h"

/****************************************************************/
/*							関数宣言							*/
/****************************************************************/
#if defined(PRJ_IVIZION2)
void __hal_led_red_on(void);
void __hal_led_red_off(void);
void __hal_led_green_on(void);
void __hal_led_green_off(void);
void __hal_led_blue_on(void);
void __hal_led_blue_off(void);
#else
//	TARGET_PROJECT_NOT_DEFINED_ERROR
#endif

/****************************************************************/
/*					ＬＥＤ　ＯＮ／ＯＦＦ定義					*/
/****************************************************************/
#define __HAL_LED_OFF	(1)
#define __HAL_LED_ON	(0)
#define __HAL_BEZEL_OFF	(0)
#define __HAL_BEZEL_ON	(1)

#endif /* __HAL_LED_H_INCLUDED__ */

