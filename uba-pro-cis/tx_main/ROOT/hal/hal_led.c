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
#include "systemdef.h"

#define EXTERN extern

#define EXT
#include "com_ram.c"
#include "usb_ram.c"
#include "jsl_ram.c"

#include "hal_led.h"
#include "hal_i2c_iox.h"


//UBA_ORE_LED
/****************************************************************/
/**
 * @brief LED Orange ON
 */
/****************************************************************/
void __hal_led_orange_on()
{
	// IOEX ch1-5
	if(ex_uba_ore_current == 0 || ex_uba_ore_current == 0xFF)
	{
		ex_uba_ore_current = 1;		
		_hal_i2c3_for_led_tca9535(1, 0x20);
	}
}

/****************************************************************/
/**
 * @brief LED Orange OFF
 */
/****************************************************************/
void __hal_led_orange_off()
{
	// IOEX ch1-5
	if(ex_uba_ore_current != 0 || ex_uba_ore_current == 0xFF)
	{
		ex_uba_ore_current = 0;
		_hal_i2c3_for_led_tca9535(0, 0x20);		
	}
}


/* End of file */
