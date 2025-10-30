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
 * @file wdt_task.h
 * @brief ＷＤＴタスク
 * @date 2018/01/25 Created.
 */
/****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "kernel_config.h"
#include "common.h"
#include "sub_functions.h"
#include "hal_bezel.h"
#include "hal.h"
#include "alt_watchdog.h"
#include "socal/hps.h"
#include "socal/socal.h"
#include "socal/alt_sysmgr.h"
#include "alt_interrupt.h"
#include "alt_reset_manager.h"
#include "wdt_task.h"

#define EXT
#include "com_ram.c"

#define INTNO_WDT ALT_INT_INTERRUPT_WDOG1_IRQ		// WDT割込み番号
#define ALT_WDOG ALT_WDOG1
#define ALT_WDOG_INIT ALT_WDOG1_INIT
#define ALT_SYSMGR_WDDBG_MOD_CLR_MSK ALT_SYSMGR_WDDBG_MOD_1_CLR_MSK
#define ALT_SYSMGR_WDDBG_MOD_SET ALT_SYSMGR_WDDBG_MOD_1_SET
#define ALT_SYSMGR_WDDBG_MOD_E_PAUSEEITHER ALT_SYSMGR_WDDBG_MOD_1_E_PAUSEEITHER
#define ALT_RESET_EVENT_L4WDRST ALT_RESET_EVENT_L4WD1RST

enum {
	FLGPTN_NULL = 0,
	// FLGID_WDT
	FLGPTN_WDT_TIMEOUT = 0x00000001,		// WDTタイムアウト
};
OSW_ISR_HANDLE hwdt;
/****************************************************************/
/**
 * @brief WDT割込み処理
 */
/****************************************************************/
void isr_wdt(void)
{
//	alt_wdog_int_clear(ALT_WDOG);
	alt_int_dist_disable(INTNO_WDT);		// 割込みフラグをクリアすると割込みカウントに戻るため、フラグをクリアせず割込み禁止とする
	iset_flg(ID_FLGID_WDT, FLGPTN_WDT_TIMEOUT);		// WDTフラグセット
}
T_CISR cisr_wdt = {TA_HLNG, NULL, INTNO_WDT, isr_wdt, IPL_USER_NORMAL};		// WDTタイムアウト割込み

/****************************************************************/
/**
 * @brief WDTの初期設定
 */
/****************************************************************/
void wdt_init(void)
{
	uint32_t regdata = 0;

	alt_wdog_init();		// WDTモジュール初期化
#if 1	//2024-08-07 1core 2024-11-21 V026
	//割り込みは5.3s周期なので、5.3sに1回はWDTタスクがアクティブにならないとResetになる
	alt_wdog_counter_set(ALT_WDOG_INIT, ALT_WDOG_TIMEOUT128M);		// WDTイニシャルカウンタ設定(25MHz)//5.3s設置 //実測だと6sではかからなくて8sでは確実にかかる
	alt_wdog_counter_set(ALT_WDOG, ALT_WDOG_TIMEOUT128M);		// WDTリピートカウンタ設定(25MHz) //5.3s設定
#else
	alt_wdog_counter_set(ALT_WDOG_INIT, ALT_WDOG_TIMEOUT32M);		// WDTイニシャルカウンタ設定(32Mカウント / 25MHz = 1.342s)
	alt_wdog_counter_set(ALT_WDOG, ALT_WDOG_TIMEOUT32M);		// WDTリピートカウンタ設定(32Mカウント / 25MHz = 1.342s)
#endif
	alt_wdog_response_mode_set(ALT_WDOG, ALT_WDOG_INT_THEN_RESET);		// 割込み->ウォームリセット設定
	
	// SYSMGR.WDDBGで、デバッグ時にWDTを停止するように設定
	regdata = alt_read_word(ALT_SYSMGR_WDDBG_ADDR);
	regdata &= ALT_SYSMGR_WDDBG_MOD_CLR_MSK ;
	regdata |= ALT_SYSMGR_WDDBG_MOD_SET(ALT_SYSMGR_WDDBG_MOD_E_PAUSEEITHER);
	alt_write_word(ALT_SYSMGR_WDDBG_ADDR, regdata);

#if 0
	cre_isr(ISRID_WDT, &cisr_wdt);
	set_int_typ(INTNO_WDT, IRQ_LEVEL_HIGH);
	ena_int(INTNO_WDT);
#else
	OSW_ISR_create( &hwdt, INTNO_WDT, (osw_isr_func)isr_wdt );
	OSW_ISR_set_priority(INTNO_WDT, IPL_USER_HIGH);
	OSW_ISR_enable(INTNO_WDT);
#endif
	
	alt_wdog_start(ALT_WDOG);		// WDTスタート
}

/****************************************************************/
/**
 * @brief WDTクリア
 */
/****************************************************************/
void wdt_clear(void)
{
	alt_wdog_reset(ALT_WDOG);		// WDTカウントリセット
	alt_int_dist_enable(INTNO_WDT);		// WDT割込み許可
}

/****************************************************************/
/**
 * @brief WDTタスク
 */
/****************************************************************/
void wdt_task(void)
{
	ER er = E_OK;
	FLGPTN ptn = 0;

	tx_thread_vfp_enable();		// VFPレジスタをコンテキスト対象とするためVFP許可とする, 20/07/20

	wdt_init();		// WDTの初期設定

	while (true) {
		er = wai_flg(ID_FLGID_WDT, FLGPTN_WDT_TIMEOUT, TWF_ORW, &ptn);		// WDTフラグ待ち
		if (er == E_OK) {		// フラグがセットされたら
			wdt_clear();		// WDTクリア
		}
	}
}
/* End if file */
