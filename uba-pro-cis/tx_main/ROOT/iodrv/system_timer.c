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
#include <kernel.h>		// kernel.h内でEXTERN宣言を操作するので、EXTERNの前でincludeする, 18/09/14
#include <time.h>
#define EXTERN extern

#include "kernel_inc.h"
#include "system_timer.h"
#include "alt_globaltmr.h"
#include "js_oswapi.h"

#define EXT
#include "com_ram.c"
struct {
	uint32_t tick_count;		// タイマカウント
} system_timer;		// システムタイマ, 18/10/15


/**
 * @brife 64ビットグローバルタイマ値取得
**/
uint64_t system_timer_get_ns(void)
{
	uint64_t time = alt_globaltmr_get64();		// 200MHz(5ns)のカウント値
	
	time *= 5U;		// nsに変換
	return time;
}

uint32_t system_timer_get_ms(void)
{
	uint64_t time = alt_globaltmr_get64();		// 200MHz(5ns)のカウント値
	uint32_t ms = time / 200000U;		// msに変換
	
	return ms;
}

uint32_t system_timer_get_us(void)
{
	uint64_t time = alt_globaltmr_get64();		// 200MHz(5ns)のカウント値
	uint32_t us = time / 200U;		// TRACEX用(us)に変換
	
	return us;
}

uint32_t system_timer_tracex(void)
{
	return system_timer_get_us();
}

struct tm *system_timer_get_localtime(struct tm *time)
{
	struct tm *ret = NULL;
	uint64_t utc_time;
	//time_t millisec = 0;		// ミリ秒
	time_t local = 0;		// ミリ秒を秒に変換
	utc_time = system_timer_read();
	if (utc_time != 0) {
		//millisec = utc_time % 1000u;		// ミリ秒
		local = utc_time / 1000u;		// ミリ秒を秒に変換
		localtime_r(&local, time);
		time->tm_mon++;		// 0～11を1～12に補正
		time->tm_hour += 9;		// TZ=JST-9補正
		time->tm_year += 1900;		// 1900年からの経過年を実年に補正
		ret = time;
	}
	
	return ret;
}

/**
* @brife 64ビットＵＴＣタイマ値取得(us)
**/
int64_t system_timer_get_local_us(void)
{
	int64_t time = system_timer_read();		// ms

	time *= 1000;
	return time;
}

/****************************************************************/
/**
 * @brief システムタイマ値更新
 * @note 本関数は、複数個所で呼び出さない事
 */
/****************************************************************/
void system_timer_update_1ms(void)
{
	if (ex_current_utc_time != 0) 	// リセットコマンドにより時間受信済みのとき
	{
		ex_current_utc_time++;		// UTC時間更新
	}
}

/****************************************************************/
/**
 * @brief システムタイマ値更新
 */
/****************************************************************/
void system_timer_update(uint64_t utc_time)
{
	uint32_t isr = OSW_ISR_global_disable();
	ex_current_utc_time = utc_time;
	OSW_ISR_global_restore(isr);
}

/****************************************************************/
/**
 * @brief システムタイマ値取得
 * @note 1970/1/1 00:00:00
 */
/****************************************************************/
uint64_t system_timer_get(void)
{
	return ex_current_utc_time;
}

/****************************************************************/
/**
 * @brief システムタイマ値読み出し(UTC 1ms)
 * @note 1970/1/1 00:00:00
 */
/****************************************************************/
u64 system_timer_read(void)
{
	uint64_t time;
	
	do
	{
		time = system_timer_get();
	}while (time != system_timer_get());
	
	return time;

}


/* End of File */
