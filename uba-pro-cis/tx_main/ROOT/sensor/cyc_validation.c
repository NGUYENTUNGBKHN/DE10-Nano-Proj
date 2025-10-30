/******************************************************************************/
/*! @addtogroup Group2
 @file       cyc_validation.c
 @brief      validation process of cycle handler
 @date       2023/06/06
 @author     yuji-kenta
 @par        Revision
 @par        Copyright (C)
 2023 Japan CashMachine Co, Limited. All rights reserved.
 *******************************************************************************
 @par        History
 - 2023/06/06 Development Dept at Tokyo
 -# Initial Version
 ******************************************************************************/

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

#include "pl/pl_cis.h"
#include "sensor.h"
#include "cyc.h"

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/
u16 _cyc_validation_tick_count;
u16 _cyc_validation_status;
u8 ex_wait_count;
//u8 ex_test_gpio;

typedef struct _cyc_validation_task_setting
{
	u16 mode;
} cyc_validation_task_setting;
cyc_validation_task_setting _cyc_validation_task_setting_value;
/************************** PRIVATE FUNCTIONS *************************/
void _cyc_check_validation_sensor(void);
static void _update_validation_ctrl_from_task(void);

/******************************************************************************/
/*! @brief check validation sensor procedure
 @return         none
 @exception      none
 ******************************************************************************/
#if 1//#if (_DEBUG_CIS_AS_A_POSITION==1)
void _cyc_validation_proc(void)
{
	_update_validation_ctrl_from_task();

	switch(_cyc_validation_mode)
	{
	case VALIDATION_CHECK_MODE_DISABLE:	/* validation cannot check */
		_cyc_validation_tick_count = 0;
		ex_validation_sensor = 0;	//cheatエラー後、クリアする処理が必要なので、ここでクリア
		ex_validation_sensor_on_count = 0;
		break;

#if 0 // 連続取り込みのタイミングで_pl_validation_sensor_offon(0); が邪魔している
	case VALIDATION_CHECK_MODE_WAIT:	/* validation check wait */
		if(_cyc_validation_tick_count)
		{
		//紙幣をすでに取り込み開始時にこれを呼び出すと識別エラーとなる
			_pl_validation_sensor_offon(0);
		}
		_cyc_validation_status = VALIDATION_CHECK_STATUS_WAIT;
		_cyc_validation_tick_count = 0;
		ex_validation_sensor = 0;	//cheatエラー後、クリアする処理が必要なので、ここでクリア
		break;
#endif

	case VALIDATION_CHECK_MODE_RUN:	/* validation check active */
		//サンプリング開始から1msec後、データ取得するとデータが間に合わない事あるので、
		//マージンとして3msec後にデータを確認するように変更した
		//全体としては5msecに1回サンプリング
		//2024-01-30
		if(_cyc_validation_status == VALIDATION_ST_INI)
		{
		//CIS Posiiotnセンサ化 開始直前、点灯待ち、点灯開始の最初の1回のみ
			ex_wait_count++;
			if(ex_wait_count > 1) //開始後2ms待ち
			{
				if(_pl_validation_sensor_offon(1))	/* validation ON */
				{
					_cyc_validation_status = VALIDATION_ST_RUN; //点灯開始、サンプリング完了待ち(時間)
					ex_wait_count = 0;
				}
				else
				{
					_cyc_validation_status = VALIDATION_ST_RUN_RETRY; //点灯失敗、点灯待ち
				}
			}
		}
		else if(_cyc_validation_status == VALIDATION_ST_RUN_RETRY)
		{
		//点灯失敗、点灯待ち
			if(_pl_validation_sensor_offon(1))	/* validation ON */
			{
				_cyc_validation_status = VALIDATION_ST_RUN; //点灯開始、サンプリング完了待ち(時間)
				ex_wait_count = 0;
			}
			else
			{
				_cyc_validation_status = VALIDATION_ST_RUN_RETRY;//点灯失敗、点灯待ち
			}
		}
		else if(_cyc_validation_status == VALIDATION_ST_RUN) //点灯開始、サンプリング完了待ち(時間)
		{
		//サンプリング完了待ち(時間) 2msec + 1msec
			ex_wait_count++;
			if(ex_wait_count > 1)
			{
				_cyc_validation_status = VALIDATION_ST_RUN_END;	
			}
		}

		else if(_cyc_validation_status == VALIDATION_ST_RUN_END)
		{
			if(_pl_validation_sensor_offon(0))	/* validation sensor OFF */
			{
				_cyc_check_validation_sensor();
				_cyc_validation_tick_count++;

				_cyc_validation_status = VALIDATION_ST_WAIT_RUN; //点灯開始待ち
				ex_wait_count = 0;
			#if 0
				if(ex_test_gpio)
				{
					Gpio_out( GPIO_28, 1 );
					ex_test_gpio = 0;
				}
				else
				{
					Gpio_out( GPIO_28, 0 );
					ex_test_gpio = 1;
				}
			#endif	
			}
		}

		else if(_cyc_validation_status == VALIDATION_ST_WAIT_RUN)
		{
		 //点灯開始待ち
			ex_wait_count++;
			if(ex_wait_count > 1)
			{
				if(_pl_validation_sensor_offon(1))	/* validation ON */
				{
					_cyc_validation_status = VALIDATION_ST_RUN;  //点灯開始、サンプリング完了待ち(時間)
					ex_wait_count = 0;
				}
				else
				{
					_cyc_validation_status = VALIDATION_ST_RUN_RETRY;//点灯失敗、点灯待ち
				}
			}
		}
		break;

	default:
		break;
	}

}
void _cyc_check_validation_sensor(void)
{
	u8 retry = SENSOR_CHATTERING_ELIMINATE;
	u16 pix = 0;
	u8 line = 0;
	u8 sensor_val;
	u8 sensor = 0;

	//ex_validation_sensor = 0;
	sensor = 0;

	for(pix = 26+5; pix < 695-5; pix++)
	{
		sensor_val = 0;
		retry = SENSOR_CHATTERING_ELIMINATE;
		for(line = 0; line < CHECK_VALIDATION_SUB_LINE; line++)
		{
			//if(!(ex_validation_sensor & VALIDATION_1ST_SENSOR))
			if(!(sensor & VALIDATION_1ST_SENSOR))
			{
				sensor_val = ex_tmp_validation_data.tmp_validation_tbl[line].check_1st_sensor[HDRTBL_SIZE + pix];
				if(sensor_val < 160) //IR
				{
					retry--;
					if(!retry)
					{
						//ex_validation_sensor |= VALIDATION_1ST_SENSOR;
						sensor |= VALIDATION_1ST_SENSOR;
					}
				}
				else
				{
					retry = SENSOR_CHATTERING_ELIMINATE;
				}
			}
		}
		sensor_val = 0;
		retry = SENSOR_CHATTERING_ELIMINATE;
		for(line = 0; line < CHECK_VALIDATION_SUB_LINE; line++)
		{
			//if(!(ex_validation_sensor & VALIDATION_2ED_SENSOR))
			if(!(sensor & VALIDATION_2ED_SENSOR))
			{
				sensor_val = ex_tmp_validation_data.tmp_validation_tbl[line].check_2ed_sensor[HDRTBL_SIZE + pix];
				if(sensor_val < 120) //Green
				{
					retry--;
					if(!retry)
					{
						//ex_validation_sensor |= VALIDATION_2ED_SENSOR;
						sensor |= VALIDATION_2ED_SENSOR;
					}
				}
				else
				{
					retry = SENSOR_CHATTERING_ELIMINATE;
				}
			}
		}
	}

	//2024-01-30
	if(sensor == (VALIDATION_1ST_SENSOR | VALIDATION_2ED_SENSOR) )
	{
		++ex_validation_sensor_on_count;
		if( ex_validation_sensor_on_count > 1 ) /* 10m *//* 1 sequence 5msec*/
		{
			if(ex_validation_sensor != (VALIDATION_1ST_SENSOR | VALIDATION_2ED_SENSOR))
			{
				ex_validation_sensor = (VALIDATION_1ST_SENSOR | VALIDATION_2ED_SENSOR);
				iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_CIS);
			}
			else
			{
				ex_validation_sensor = (VALIDATION_1ST_SENSOR | VALIDATION_2ED_SENSOR);
			}
		}
	}
	else
	{
		ex_validation_sensor_on_count = 0;
		if(ex_validation_sensor == (VALIDATION_1ST_SENSOR | VALIDATION_2ED_SENSOR))
		{
			ex_validation_sensor = 0;
			iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_CIS);
		}
		else
		{
			ex_validation_sensor = 0;
		}
	}

}

static void _update_validation_ctrl_from_task(void)
{
	switch(_cyc_validation_task_setting_value.mode)
	{
	case VALIDATION_CHECK_MODE_DISABLE:
// 連続取り込みのタイミング呼び出すと識別エラーとなるので,とりあえず廃止にしておく	case VALIDATION_CHECK_MODE_WAIT:
	case VALIDATION_CHECK_MODE_RUN:
		_cyc_validation_mode = _cyc_validation_task_setting_value.mode;
		_cyc_validation_status = VALIDATION_ST_INI;
		ex_wait_count = 0;
		memset(&_cyc_validation_task_setting_value, 0, sizeof(_cyc_validation_task_setting_value));
		break;
	default:
		break;
	}
}

void _validation_ctrl_set_mode(u16 mode)
{
	_cyc_validation_task_setting_value.mode = mode;
}

#endif
