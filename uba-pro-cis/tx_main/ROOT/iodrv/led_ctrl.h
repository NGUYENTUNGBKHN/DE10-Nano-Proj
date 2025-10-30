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
 * @file led_ctrl.c
 * @brief ＬＥＤ制御関数 ヘッダファイル
 * @date 2018.03.05 Created
 */
/****************************************************************************/
#if !defined(__LED_CTRL_H_INCLUDED__)
#define __LED_CTRL_H_INCLUDED__

/****************************************************************/
/*						関数宣言								*/
/****************************************************************/
void set_led_flik(u32 led_no);
void set_led_on(u32 led_no);
void set_led_off(u32 led_no);
void set_led_all_off(void);
void set_led_flash_write_mode(void);


#endif /* __LED_CTRL_H_INCLUDED__ */

/* End of file */


