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

#define EXT
#include "com_ram.c"

MRX_CLOCK ex_mrx_clock;
u32 _system_clock_tick_count = 0;		/* システム時刻タイマー */
u32 _ir_system_timer_tick_count = 0;
u32 ex_current_utc_time = 0;

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

/****************************************************************/
/**
 * @brif 64ビットＵＴＣタイマ値取得(us)
 */
/****************************************************************/
s64 system_timer_get_local_us(void)
{
	s64 time = ex_current_utc_time;		// ms

	time *= 1000;
	return time;
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
	// 10msに一回の呼び出し
	switch ((_ir_system_timer_tick_count % 10))
	{
	case 0:
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
	if( Cm01Detach != 0)
	{
		if( (--Cm01Detach) == 0 )
		{
			
		}
	}
#endif

}


/*1s間隔でコール*/
void clock_count(void)
{
	u8 fUpdateMonth;

	// システム時刻タイマー
	_system_clock_tick_count ++;
	ex_mrx_clock.msec++;
	if(_system_clock_tick_count % 1000 == 0)
	{
		_system_clock_tick_count = 0;
		ex_mrx_clock.msec = 0;

		ex_mrx_clock.sec++;
		if(ex_mrx_clock.sec > 59) // 0秒～59秒
		{
			ex_mrx_clock.sec = 0;
			ex_mrx_clock.min++;
		}

		if(ex_mrx_clock.min > 59) // 0分～59分
		{
			ex_mrx_clock.min = 0;
			ex_mrx_clock.hour++;
		}
	
		if(ex_mrx_clock.hour > 23) // 0時～23時
		{
			ex_mrx_clock.hour = 0;
			ex_mrx_clock.day++;
		}

		if (ex_mrx_clock.month == 2)
		{
			/* うるう年の判定 */
			// 4で割り切れる年はうるう年
			// 100で割り切れる年はうるう年ではない
			// でも 400で割り切れる年はうるう年
			if (ex_mrx_clock.year % 400 == 0 || (ex_mrx_clock.year % 4 == 0 && ex_mrx_clock.year % 100 != 0))
			{
				if(ex_mrx_clock.day > 29)// うるう年 1日～29日
				{
					fUpdateMonth = 1;
				}
			}
			else {
				if(ex_mrx_clock.day > 28)// 平年　1日～28日
				{
					fUpdateMonth = 1;
				}
			}
		}
		else if (ex_mrx_clock.month == 1 || ex_mrx_clock.month == 3 ||
			ex_mrx_clock.month == 5 || ex_mrx_clock.month == 7 ||
			ex_mrx_clock.month == 10 || ex_mrx_clock.month == 12)
		{
			if(ex_mrx_clock.day > 31)// 1日～31日
			{
				fUpdateMonth = 1;
			}
		}
		else {
			if(ex_mrx_clock.day > 30)// 1日～30日
			{
				fUpdateMonth = 1;
			}
		}
		if (fUpdateMonth == 1)
		{
			ex_mrx_clock.day = 1;
			ex_mrx_clock.month++;
		}

		if(ex_mrx_clock.month > 12)// 1月～12月
		{
			ex_mrx_clock.month = 1;
			ex_mrx_clock.year++;
		}
	}

#if 0	// DEBUG 時:分:秒 printf
	printf("%02d:%02d:%02d\n", ex_mrx_clock.hour, ex_mrx_clock.min, ex_mrx_clock.sec);
#endif
#if 0	// DEBUG 年月日 時:分:秒 printf
	printf("%04d/%02d/%02d %02d:%02d:%02d\n",
			ex_mrx_clock.year,
			ex_mrx_clock.month,
			ex_mrx_clock.day,
			ex_mrx_clock.hour,
			ex_mrx_clock.min,
			ex_mrx_clock.sec);
#endif
}

/* End of File */
