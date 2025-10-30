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
extern void init_intr_fusb_dect(u32 high_edge);
extern void clear_intr_fusb_dect(void);
extern void __hal_sd_card_dect_toggle(void);
extern void init_test_sw(void);
extern void init_power_fail(void);
extern u8 __hal_su_select_read(void);
extern u8 __hal_reset_det_read(void);
extern u8 __hal_pf_det_read(void);
extern u8 __hal_rfid_int_read(void);
extern u8 __hal_sd_card_dect_read(void);
extern u8 __hal_24v_dect_read(void);
extern u8 __hal_fusb_dect_read(void);
#if defined(PRJ_IVIZION2)
extern void _hal_cen_open_interrupt_enable(void);
extern void _hal_cen_open_interrupt_disable(void);
extern void _hal_cen_close_interrupt_enable(void);
extern void _hal_cen_close_interrupt_disable(void);
extern u8 __hal_if_set_read(void);
#endif
extern u8 is_fusb_dect_on(void);
extern u8 is_external_inhibit_on(void);
extern void __hal_if_select_pc(void);
extern void __hal_if_select_rs232c(void);
extern void __hal_if_select_cctalk(void);
extern void __hal_if_select_off(void);
extern void _intr_low_voltage(void *arg);
extern void _intr_fusb_dect(void *arg);
//extern void _intr_test_sw(void *arg);
/* End of file */
