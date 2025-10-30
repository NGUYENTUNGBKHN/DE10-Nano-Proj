/******************************************************************************/
/*! @addtogroup Main
    @file       hal_status_leds.c
    @brief      ステータスLED(3光源マルチカラー)発光制御ドライバファイル。
    @date       2018/02/27
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

#include "hal_led.h"

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/************************** Private Definitions *****************************/

/*********************************************************************//**
 * @brief		Set status LED
 * @param[in]	u8 color Red on/off
 * @return 		None
 **********************************************************************/
#if defined(PRJ_IVIZION2)
void _hal_status_led_red(u8 on)
{
	if(on)
	{
		__hal_led_red_on();
	}
	else
	{
		__hal_led_red_off();
	}
}

/*********************************************************************//**
 * @brief		Set status LED
 * @param[in]	u8 color Green on/off
 * @return 		None
 **********************************************************************/
void _hal_status_led_green(u8 on)
{
	if(on)
	{
		__hal_led_green_on();
	}
	else
	{
		__hal_led_green_off();
	}
}
/*********************************************************************//**
 * @brief		Set status LED
 * @param[in]	u8 color Blue on/off
 * @return 		None
 **********************************************************************/
void _hal_status_led_blue(u8 on)
{
	if(on)
	{
		__hal_led_blue_on();
	}
	else
	{
		__hal_led_blue_off();
	}
}
#endif

/*********************************************************************//**
 * @brief		Set status LED
 * @param[in]	u8 color Red on/off
 * @return 		None
 **********************************************************************/
void _hal_status_led(u8 color)
{
	switch(color)
	{
		case DISP_COLOR_INIT://2023-01-31
			_hal_i2c3_read_write_p0(0, 0x08);	/* ハード的にはLowで点灯 *//* Red */
			_hal_i2c3_read_write_p0(0, 0x10);	/* ハード的にはLowで点灯 *//* Green */
			_hal_i2c3_read_write_p0(0, 0x20);	/* ハード的にはLowで点灯 *//* Orenge */

			_hal_i2c3_for_led_tca9535(0, 0x08);	/* 消灯 */
			_hal_i2c3_for_led_tca9535(0, 0x10);	/* 消灯 */
			red_on = 0;
			green_on = 0;
			break;

		case DISP_COLOR_RED:
			_hal_i2c3_for_led_tca9535(1, 0x08);	/* ハード的にはLowで点灯 */
			red_on = 1;
			break;

		case DISP_COLOR_GREEN:
			_hal_i2c3_for_led_tca9535(1, 0x10);	/* ハード的にはLowで点灯 */
			green_on = 1;
			break;

		case DISP_COLOR_RED_GREEN:
			_hal_i2c3_for_led_tca9535(1, 0x08);
			_hal_i2c3_for_led_tca9535(1, 0x10);
			red_on = 1;
			green_on = 1;
			break;

		case DISP_COLOR_RED_OFF:
			_hal_i2c3_for_led_tca9535(0, 0x08);
			red_on = 0;
			break;

		case DISP_COLOR_GREEN_OFF:
			_hal_i2c3_for_led_tca9535(0, 0x10);
			green_on = 0;
			break;

		case DISP_COLOR_OFF:
		default:
			_hal_i2c3_for_led_tca9535(0, 0x08);
			_hal_i2c3_for_led_tca9535(0, 0x10);			
			red_on = 0;
			green_on = 0;
			break;
	}
}
/* EOF */
