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
 * @file int_system_timer.c
 * @brief
 * @date 2018.01.05 Created.
 */
/****************************************************************************/
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"

#define EXTERN extern
#include "system_timer.h"
#include <time.h>
#include "common.h"
#include "custom.h"
#include "cyc.h"
#ifdef _ENABLE_JDL
#include "jdl.h"
#endif	/* _ENABLE_JDL */

#include "systemdef.h"					//2022-02-17 test
#include "cyclonev_sysmgr_reg_def.h"	//2022-02-17 test
#include "hal_gpio_reg.h"				//2022-02-17 test

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"

u32 _system_clock_tick_count = 0;		/* システム時刻タイマー */
u32 _ir_system_timer_tick_count = 0;

void clock_count(void);



/****************************************************************/
/**
 * @brif FPGAタイムスタンプ値取得
 */
/****************************************************************/
u32 system_timer_get_timestamp(void)
{
#if 0
	return FPGA_REG.TIMSTMP;		// FPGAタイムスタンプ
#else
	return 0xFFFFFFFF;
#endif
}
/************************************************************************************************/
/* FUNCTION   : osw_timer_isr                                                                   */
/*                                                                                              */
/* DESCRIPTION: 1ms Timer割り込み                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/

extern volatile uint32_t osw_sys_timer;
static void osw_timer_isr(void)
{
	osw_sys_timer++;
}
/****************************************************************/
/**
 * @brief システムタイマ処理(1ms)
 */
/****************************************************************/
void _intr_system_timer_proc(UArg32 arg)
{
	osw_timer_isr();
	ex_current_utc_time++;		// UTC時間更新

	if (_ir_line_cmd_monitor_time != 0)
	{
		_ir_line_cmd_monitor_time--;
	}
	if (_ir_motor_disable_time != 0)
	{
		_ir_motor_disable_time--;
	}
	if (_ir_feed_motor_ctrl.stop_time != 0)
	{
		_ir_feed_motor_ctrl.stop_time--;
	}

	if((_ir_feed_ctrl.time_init != 0) && (_ir_feed_ctrl.time != 0))
	{
		_ir_feed_ctrl.time_out = _ir_feed_ctrl.time;
		_ir_feed_ctrl.time = 0;
		_ir_feed_ctrl.time_init = 0;
	}

	if (_ir_feed_ctrl.time_out != 0)
	{
		_ir_feed_ctrl.time_out--;
		if (_ir_feed_ctrl.time_out == 0)
		{
			iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_TIMEOUT);
		}
	}

	if ((_ir_feed_motor_ctrl.mode == MOTOR_FWD) || (_ir_feed_motor_ctrl.mode == MOTOR_REV))
	{
		_ir_feed_motor_ctrl.lock_count++;
		if (_ir_feed_motor_ctrl.lock_count >= FEED_MOTOR_LOCK_TIME)
		{
			iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_MOTOR_LOCK);
		}
	}
	if (_ir_stacker_motor_ctrl.stop_time != 0)
	{
		_ir_stacker_motor_ctrl.stop_time--;
	}
	if (_ir_stacker_ctrl_time_out != 0)
	{
		_ir_stacker_ctrl_time_out--;
		if (_ir_stacker_ctrl_time_out == 0)
		{
			_ir_stacker_motor_ctrl.event_pulse = _ir_stacker_motor_ctrl.pulse;
			iset_flg(ID_STACKER_CTRL_FLAG, EVT_STACKER_TIMEOUT);
		}
	}

	if ((_ir_stacker_motor_ctrl.mode == MOTOR_FWD) || (_ir_stacker_motor_ctrl.mode == MOTOR_REV))
	{
		_ir_stacker_motor_ctrl.lock_count++;
		if (_ir_stacker_motor_ctrl.lock_count >= STACKER_MOTOR_LOCK_TIME)
		{
			_ir_stacker_motor_ctrl.event_pulse = _ir_stacker_motor_ctrl.pulse;
			iset_flg(ID_STACKER_CTRL_FLAG, EVT_STACKER_MOTOR_LOCK);
		}
	}


	ex_dline_testmode.time_count++;

	if (ex_centor_motor_run != 0)
	{
		ex_centor_motor_run_time--;
		if (ex_centor_motor_run_time == 0)
		{
			iset_flg(ID_CENTERING_CTRL_FLAG, EVT_CENTERING_RUN_TIMEOUT);
		}
	}

	if( ex_centor_home_out == 1 )
	{
	/* Home out*/
		if( ex_centor_home_out_time == CENTERING_CONFIRM_HOME_OUT_TIME )
		{
			iset_flg(ID_CENTERING_CTRL_FLAG, EVT_CENTERING_HOME_OUT_INTR);
		}
		else if( ex_centor_home_out_time < CENTERING_CONFIRM_HOME_OUT_TIME )
		{

			if( ex_centor_home_out_time == 10 )	/* 2021-05-27 */
			{
				/* 幅よせclose専用 */
				iset_flg(ID_CENTERING_CTRL_FLAG, EVT_CENTERING_HOME_OUT_INTR_10MSEC);	/* 2021-05-27 */
			}
			ex_centor_home_out_time++;
		}
	}
	else
	{
	/* Home */
		if(ex_centor_home_out_time != 0)
		{
			ex_centor_home_out_time = 0;
			iset_flg(ID_CENTERING_CTRL_FLAG, EVT_CENTERING_HOME_INTR);
		}
	}

	if (_ir_centering_motor_ctrl.stop_time != 0)
	{
		_ir_centering_motor_ctrl.stop_time--;
	}
	if (_ir_centering_ctrl_time_out != 0)
	{
		_ir_centering_ctrl_time_out--;
		if (_ir_centering_ctrl_time_out == 0)
		{
			iset_flg(ID_CENTERING_CTRL_FLAG, EVT_CENTERING_TIMEOUT);
		}
	}

	if( ex_1msec_timer != 0)
	{
		--ex_1msec_timer;
	}

	/* PB */
	if (_ir_apb_motor_ctrl.stop_time != 0)
	{
		_ir_apb_motor_ctrl.stop_time--;
	}
	if (pb_rev_time != 0)
	{
		pb_rev_time--;
		if (pb_rev_time == 0)
		{
			iset_flg(ID_APB_CTRL_FLAG, EVT_APB_REV_TIME);
		}
	}
	/* Shutter */
	if (_ir_shutter_motor_ctrl.stop_time != 0)
	{
		_ir_shutter_motor_ctrl.stop_time--;
	}
	if (_ir_shutter_ctrl_time_out != 0)
	{
		_ir_shutter_ctrl_time_out--;
		if (_ir_shutter_ctrl_time_out == 0)
		{
			iset_flg(ID_SHUTTER_CTRL_FLAG, EVT_SHUTTER_TIMEOUT);
		}
	}

	/* Stacker full */
	if (_ir_stacker_motor_ctrl.peakload_flag)
	{
		_ir_stacker_motor_ctrl.peakload_time++;	// 押し終わり付近の時間計測 Full check用に使用
	}

	#if defined(UBA_RTQ)
	if(ex_rc_detect_time != 0)
	{
		ex_rc_detect_time--;
	}
	if (ex_rc_ready_timeout != 0)
	{
		ex_rc_ready_timeout--;
	}
	if(ex_rc_flap_test_flg)
	{
		ex_rc_flap_test_time++;
	}
	#endif



	if (_ir_icb_ctrl_time_out != 0)
	{
		_ir_icb_ctrl_time_out--;
		if (_ir_icb_ctrl_time_out == 0)
		{
			ipsnd_dtq(ID_ICB_DTQ, (VP_INT)ICB_LISTEN_TMOUT);
		}
	}

	_cyc_sensor_proc();							/* check sensor */

	//#if (_DEBUG_CIS_AS_A_POSITION==1)
	_cyc_validation_proc();						/* check sensor */
	//#endif

	// 10msに一回の呼び出し
	switch ((_ir_system_timer_tick_count % 10))
	{
	case 0:
#ifdef _ENABLE_JDL
		jdl_posiana_update();
#endif /* _ENABLE_JDL */
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		/* Timer Task tick */
		if (ex_timer_task_tick >= 0xFFFF)
		{
			ex_timer_task_tick = 0;
		}
		else
		{
			ex_timer_task_tick++;
		}
		break;
	default:
		break;
	}
#ifdef _ENABLE_JDL
	jdl_tick();
#endif	/* _ENABLE_JDL */
	
	_ir_system_timer_tick_count++;
	if (_ir_system_timer_tick_count >= 10)
	{
		_ir_system_timer_tick_count = 0;
	}
	clock_count();
//	// システム時刻タイマー
//	_system_clock_tick_count ++;
//	// 1sec？
//	if(_system_clock_tick_count % 1000 == 0){
//		clock_count();
//	}

#if defined(_PROTOCOL_ENABLE_ID0G8)
	if(Cm01Detach)
	{
		Cm01Detach--;
	}
#endif


}


/*1s間隔でコール*/
void clock_count(void)
{
	u8 fUpdateMonth;
	// システム時刻タイマー
	_system_clock_tick_count ++;
	if(_system_clock_tick_count % 1000 == 0)
	{
		_system_clock_tick_count = 0;
		ex_rtc_clock.second++;
		if(ex_rtc_clock.second > 59) // 0秒～59秒
		{
			ex_rtc_clock.second = 0;
			ex_rtc_clock.minute++;
		}
		if(ex_rtc_clock.minute > 59) // 0分～59分
		{
			ex_rtc_clock.minute = 0;
			ex_rtc_clock.hour++;
		}
		if(ex_rtc_clock.hour > 23) // 0時～23時
		{
			ex_rtc_clock.hour = 0;
			ex_rtc_clock.day++;
		}
		if (ex_rtc_clock.month == 2)
		{
			/* うるう年の判定 */
			// 4で割り切れる年はうるう年
			// 100で割り切れる年はうるう年ではない
			// でも 400で割り切れる年はうるう年
			if (ex_rtc_clock.year % 400 == 0 || (ex_rtc_clock.year % 4 == 0 && ex_rtc_clock.year % 100 != 0))
			{
				if(ex_rtc_clock.day > 29)// うるう年 1日～29日
				{
					fUpdateMonth = 1;
				}
			}
			else {
				if(ex_rtc_clock.day > 28)// 平年　1日～28日
				{
					fUpdateMonth = 1;
				}
			}
		}
		else if (ex_rtc_clock.month == 1 || ex_rtc_clock.month == 3 ||
			ex_rtc_clock.month == 5 || ex_rtc_clock.month == 7 ||
			ex_rtc_clock.month == 10 || ex_rtc_clock.month == 12)
		{
			if(ex_rtc_clock.day > 31)// 1日～31日
			{
				fUpdateMonth = 1;
			}
		}
		else {
			if(ex_rtc_clock.day > 30)// 1日～30日
			{
				fUpdateMonth = 1;
			}
		}
		if (fUpdateMonth == 1)
		{
			ex_rtc_clock.day = 1;
			ex_rtc_clock.month++;
		}
		if(ex_rtc_clock.month > 12)// 1月～12月
		{
			ex_rtc_clock.month = 1;
			ex_rtc_clock.year++;
		}
	}
}

/* End of File */
