/*
 * mag_sensor.c
 *
 *  Created on: 2022/12/22
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
#define MAG_GAIN_RETRY 3

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/



#if MAG1_ENABLE

u8 _sensor_temp_check_ad(u16 *gain_val, u16 ad_val, MAG_SENS_TEMP_ADJ_INFO *adj_val, u16 ad_rv, u16 range, u8 gain_max, u8 gain_min)
{
	u8 rtn = 0;

	adj_val->count++;
	if (ad_val == ad_rv)
	{ /* 基準値と同じ */
		adj_val->end_flag = 1;
		rtn = 1;
	}
	else if (ad_val < ad_rv)
	{ /* 基準値より低い */
		if (adj_val->count == 1)
		{ /* 1回目 */
			if (*gain_val >= gain_max)
			{ /* DA MAX */
				adj_val->end_flag = 2;
				rtn = 1;
			}
			else
			{
				/* da + range */
				adj_val->last_gain = *gain_val;
				adj_val->last_ad = ad_val;
				if ((*gain_val + range) > gain_max)
				{
					*gain_val = gain_max;
				}
				else
				{
					*gain_val += range;
				}
			}
		}
		else
		{ /* 2回目以降 */
			if (adj_val->last_ad >= ad_rv)
			{ /* 前回は基準値以上 */
				if ((ad_rv - ad_val) < (adj_val->last_ad - ad_rv))
				{ /* 今回のAD値のほうが近い */
					adj_val->end_flag = 3;
					rtn = 1;
				}
				else
				{ /* 前回のAD値のほうが近い */	/* 同じ場合はADが高いほうを優先 */
					*gain_val = adj_val->last_gain;
					adj_val->end_flag = 4;
					rtn = 1;
				}
			}
			else
			{ /* 前回も基準値より低い */
				if (*gain_val >= gain_max)
				{ /* DA MAX */
					adj_val->end_flag = 5;
					rtn = 1;
				}
				else
				{
					/* da + range */
					adj_val->last_gain = *gain_val;
					adj_val->last_ad = ad_val;
					if ((*gain_val + range) > gain_max)
					{
						*gain_val = gain_max;
					}
					else
					{
						*gain_val += range;
					}
				}
			}
		}
	}
	else
	{ /* 基準値より高い */
		if (adj_val->count == 1)
		{ /* 1回目 */
			if (*gain_val <= gain_min)
			{ /* DA MIN */
				adj_val->end_flag = 6;
				rtn = 1;
			}
			else
			{
				/* da - range */
				adj_val->last_gain = *gain_val;
				adj_val->last_ad = ad_val;
				if ((*gain_val < range)
				 || (*gain_val - range) < gain_min)
				{
					*gain_val = gain_min;
				}
				else
				{
					*gain_val -= range;
				}
			}
		}
		else
		{ /* 2回目以降 */
			if (adj_val->last_ad <= ad_rv)
			{ /* 前回は基準値以下 */
				if ((ad_val - ad_rv) <= (ad_rv - adj_val->last_ad))
				{ /* 今回のAD値のほうが近い */	/* 同じ場合はADが高いほうを優先 */
					adj_val->end_flag = 7;
					rtn = 1;
				}
				else
				{ /* 前回のAD値のほうが近い */
					*gain_val = adj_val->last_gain;
					adj_val->end_flag = 8;
					rtn = 1;
				}
			}
			else
			{ /* 前回も基準値より高い */
				if (*gain_val <= gain_min)
				{ /* DA MIN */
					adj_val->end_flag = 9;
					rtn = 1;
				}
				else
				{
					/* da - range */
					adj_val->last_gain = *gain_val;
					adj_val->last_ad = ad_val;
					if ((*gain_val < range)
					 || (*gain_val - range) < gain_min)
					{
						*gain_val = gain_min;
					}
					else
					{
						*gain_val -= range;
					}
				}
			}
		}
	}
	return rtn;
}

u8 set_mag_gain(void)
{
	u8 rtn = ALARM_CODE_I2C; //最終的にはエラーにはしていない
	u8 cnt;

	for (cnt = 0; cnt < MAG_GAIN_RETRY; cnt++)
	{
		if (!_hal_mag_gain_write_left(ex_mag_adj.ul_gain)) //2023-07-19
		{
			rtn = ALARM_CODE_OK; /* success */
			break;
		}
	}

	for (cnt = 0; cnt < MAG_GAIN_RETRY; cnt++)
	{
		if (!_hal_mag_gain_write_right(ex_mag_adj.ur_gain))
		{
			rtn = ALARM_CODE_OK; /* success */
			break;
		}
	}

	return rtn;
}

u8 set_mag_adj(void)
{
	u8 rtn;
	u16 end_cnt = 0;
	/* Adjust ul_mag */
	if (s_sens_tempadj_ul_mag.end_flag == 0)
	{
		if (rtn = _sensor_temp_check_ad(&ex_mag_adj.ul_gain, ex_mag_adj.ul_adj_max, &s_sens_tempadj_ul_mag, MAG_ADJ_RV, 1, MAG_GAIN_MAX, MAG_GAIN_MIN))
		{
			end_cnt++;
		}
	}
	else
	{
		end_cnt++;
	}

	/* Adjust ur_mag */
	if (s_sens_tempadj_ur_mag.end_flag == 0)
	{
		if (rtn = _sensor_temp_check_ad(&ex_mag_adj.ur_gain, ex_mag_adj.ur_adj_max, &s_sens_tempadj_ur_mag, MAG_ADJ_RV, 1, MAG_GAIN_MAX, MAG_GAIN_MIN))
		{
			end_cnt++;
		}
	}
	else
	{
		end_cnt++;
	}
	set_mag_gain();

	return end_cnt;
}

#endif
