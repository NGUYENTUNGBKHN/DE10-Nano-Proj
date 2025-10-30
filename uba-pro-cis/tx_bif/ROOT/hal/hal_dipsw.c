/******************************************************************************/
/*! @addtogroup Main
    @file       hal_dipsw.c
    @brief      ディップスイッチ(１：受取金種8極、２：H/W設定8極、３：MAGゲイン）ドライバファイル。
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_dipsw.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "js_oswapi.h"
#include "js_io.h"
#include "hal_dipsw.h"
#include "hal_i2c_iox.h"

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/



/*********************************************************************//**
 * @brief		Read Dip switch Value(tca9535)
 * @param[out]	dipsw_value dipsw value
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
u32 _hal_read_dipsw1(u8 *dipsw_value)
{
	u32 ercd = E_OK;
	// ディップスイッチの設定を確認
	ercd = _hal_i2c3_read_dipsw1(dipsw_value);
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.dipsw1 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.dipsw1 = HAL_STATUS_OK;
	}
#endif

	return ercd;
}

/*********************************************************************//**
 * @brief		Read Dip switch Value(tca9535)
 * @param[out]	dipsw_value dipsw value
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
u32 _hal_read_dipsw2(u8 *dipsw_value)
{
	UINT32 ercd = E_OK;
	// ディップスイッチの設定を確認
	ercd = _hal_i2c3_read_dipsw2(dipsw_value);
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.dipsw2 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.dipsw2 = HAL_STATUS_OK;
	}
#endif

	return ercd;
}
/* EOF */
