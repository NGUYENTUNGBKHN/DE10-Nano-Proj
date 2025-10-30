/*
 * position_sensor.c
 *
 *  Created on: 2018/02/27
 *      Author: suzuki-hiroyuki
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "pl/pl.h"
#include "pl/pl_pos.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

#include "sensor.h"

#define POSITION_DA_RETRY 3
#define POSITION_DA_VALUE 0xFF

u8 set_position_da(void)
{
	u8 rtn = 0;

	rtn = _pl_pos_init_da();
	if (rtn != ALARM_CODE_OK)
	{
		return rtn;
	}

	return rtn;
}

u8 set_position_ga(void)
{
	u8 rtn = 0;

	rtn = _pl_position_sensor_gain(ex_position_ga);
	if (rtn != E_OK)
	{
		rtn =  ALARM_CODE_I2C; //エラーは存在しない
	}

	return rtn;
}
/* EOF */
