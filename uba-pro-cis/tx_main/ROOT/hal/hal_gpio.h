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
 * @brief LEDの点灯／消灯のハードウェアレイヤー
 * @date 2018.01.26 Created
 */
/****************************************************************************/
#pragma once

extern void setup_gpio(void);
extern void set_gpio_irq(void);

extern void init_external_reset(void);
extern void init_power_fail(void);
extern u8 __hal_su_select_read(void);
extern u8 __hal_reset_det_read(void);


extern u8 __hal_fusb_dect_read(void);
extern u8 is_fusb_dect_on(void);
extern void __hal_if_select_rs232c(void);
extern void _intr_external_reset(void *arg);
extern void _intr_low_voltage(void *arg);

#if 1
/** \brief GPIO pin is at logic low.*/
#define GPIO_PIN_LOW				(0x0U)
/** \brief GPIO pin is at logic high.*/
#define GPIO_PIN_HIGH				(0x1U)
#else
/** \brief GPIO pin is at logic low.*/
#define GPIO_PIN_LOW				(0x1U)
/** \brief GPIO pin is at logic high.*/
#define GPIO_PIN_HIGH				(0x0U)
#endif
/* End of file */
