/******************************************************************************/
/*! @addtogroup Main
    @file       hal_bezel.c
    @brief      Bezel LED driver.
    @brief      ベゼルLED発光制御ドライバファイル。
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_bezel.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "i2c/js_i2c.h"
#include "common.h"
#include "hal_i2c_iox.h"
#include "hal_bezel.h"
#include "hal_led.h"

#include "custom.h"	//2023-01-27

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
#if defined(PRJ_IVIZION2)

/*********************************************************************//**
 * @brief		Control bezel LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
u32 _hal_write_bezel_led(u32 data)
{
#if 0
	// 20210803 resume i2c access
	UINT32 result = E_OK;
	UINT32 value = __HAL_BEZEL_ON;
	if(data == 0)
	{
		value = __HAL_BEZEL_OFF;
	}
	if(ex_led_out_signal.bezel != data)
	{
		ex_led_out_signal.bezel = data;
		result =  _hal_i2c3_write_led_bezel(value);
	}

#if (HAL_STATUS_ENABLE==1)
	if(result != E_OK)
	{
		ex_hal_status.bezel = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.bezel = HAL_STATUS_OK;
	}
#endif
	return result;
#else
	UINT32 result = E_OK;
	UINT32 value = __HAL_BEZEL_ON;
	if(data == 0)
	{
		value = __HAL_BEZEL_OFF;
	}
	result =  _hal_i2c3_write_led_bezel(value);

#if (HAL_STATUS_ENABLE==1)
	if(result != E_OK)
	{
		ex_hal_status.bezel = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.bezel = HAL_STATUS_OK;
	}
#endif
	return result;
#endif
}

#endif

/* EOF */

