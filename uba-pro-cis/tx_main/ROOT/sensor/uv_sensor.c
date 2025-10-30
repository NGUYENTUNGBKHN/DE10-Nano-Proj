/*
 * uv_sensor.c
 *
 *  Created on: 2021/10/05
 *      Author: yuji-kenta
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
#include "jsl_ram.c"

#include "sensor.h"
#include "hal_i2c_dp.h"
#include "pl/pl_pos.h"

/************************** PRIVATE DEFINITIONS *************************/
#define UV_DA_RETRY 3

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/



u8 set_uv_da(void) //呼び出し元のset_uv_adj(void) でUBA700,710の切り分けをしているので、この中での切り分けは必要ない
{
	u8 rtn = ALARM_CODE_I2C;  //最終的にはエラーにはしていない
	u8 cnt;

	for (cnt = 0; cnt < UV_DA_RETRY; cnt++)
	{
		if (!_pl_set_uv_da())
		{
			rtn = ALARM_CODE_OK; /* success */
			break;
		}
	}
#if POINT_UV2_ENABLE
	for (cnt = 0; cnt < UV_DA_RETRY; cnt++)
	{
		if (!_pl_set_uv1_da())
		{
			rtn = ALARM_CODE_OK; /* success */
			break;
		}
	}
#endif
	return rtn;
}


u8 set_uv_gain(void) //呼び出し元のset_uv_adj(void) でUBA700,710の切り分けをしているので、この中での切り分けは必要ない
{
	u8 rtn = ALARM_CODE_I2C;  //最終的にはエラーにはしていない
	u8 cnt;

	for (cnt = 0; cnt < UV_DA_RETRY; cnt++)
	{
		if (!_hal_uv_gain_write((u8)(ex_cis_adjustment_data.point_uv_adj.gain[0] & 0xff)))
		{
			rtn = ALARM_CODE_OK; /* success */
			break;
		}
	}
#if POINT_UV2_ENABLE
	for (cnt = 0; cnt < UV_DA_RETRY; cnt++)
	{
		if (!_hal_uv1_gain_write((u8)(ex_cis_adjustment_data.point_uv_adj.gain[1] & 0xff)))
		{
			rtn = ALARM_CODE_OK; /* success */
			break;
		}
	}
#endif

	return rtn;
}

u8 set_uv_adj(void)
{
	u8 rtn = 0;
	rtn = set_uv_da();
	if (rtn != ALARM_CODE_OK)
	{
		return rtn;
	}
	rtn = set_uv_gain();
	if (rtn != ALARM_CODE_OK)
	{
		return rtn;
	}
	return rtn;
}

/* EOF */
