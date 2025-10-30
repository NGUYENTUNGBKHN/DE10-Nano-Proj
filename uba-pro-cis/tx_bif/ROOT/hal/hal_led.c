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

/****************************************************************/
/**
 * @brief GPIO OFF<->ON
 */
/****************************************************************/
void _Gpio_toggle( UINT16 gpio_id)
{
	if(Gpio_in(gpio_id) == __HAL_LED_ON)
	{
		Gpio_out(gpio_id, __HAL_LED_OFF);
	}
	else
	{
		Gpio_out(gpio_id, __HAL_LED_ON);
	}
}

#if defined(PRJ_IVIZION2)
/************************** Variable declaration *****************************/
/****************************************************************/
/**
 * @brief LED RED ON
 */
/****************************************************************/
void __hal_led_red_on()
{
	// IOEX ch1-3
	_hal_i2c3_write_led_red(__HAL_LED_ON);
}

/****************************************************************/
/**
 * @brief LED RED OFF
 */
/****************************************************************/
void __hal_led_red_off()
{
	// IOEX ch1-3
	_hal_i2c3_write_led_red(__HAL_LED_OFF);
}
/****************************************************************/
/**
 * @brief LED GREEN ON
 */
/****************************************************************/
void __hal_led_green_on()
{
	// IOEX ch1-4
	_hal_i2c3_write_led_green(__HAL_LED_ON);
}

/****************************************************************/
/**
 * @brief LED GREEN OFF
 */
/****************************************************************/
void __hal_led_green_off()
{
	// IOEX ch1-4
	_hal_i2c3_write_led_green(__HAL_LED_OFF);
}

/****************************************************************/
/**
 * @brief LED BLUE ON
 */
/****************************************************************/
void __hal_led_blue_on()
{
	// IOEX ch1-5
	_hal_i2c3_write_led_blue(__HAL_LED_ON);
}

/****************************************************************/
/**
 * @brief LED BLUE OFF
 */
/****************************************************************/
void __hal_led_blue_off()
{
	// IOEX ch1-5
	_hal_i2c3_write_led_blue(__HAL_LED_OFF);
}
#endif

//#else
//	TARGET_PROJECT_NOT_DEFINED_ERROR
//#endif

/* End of file */
