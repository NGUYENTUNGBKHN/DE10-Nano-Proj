/**************************************************************************/
/*                                                                        */
/*            Copyright (c) 1996-2018 by Express Logic Inc.               */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of Express   */
/*  Logic, Inc.  All rights, title, ownership, or other interests         */
/*  in the software remain the property of Express Logic, Inc.  This      */
/*  software may only be used in accordance with the corresponding        */
/*  license agreement.  Any unauthorized use, duplication, transmission,  */
/*  distribution, or disclosure of this software is expressly forbidden.  */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of Express Logic, Inc.                                */
/*                                                                        */
/*  Express Logic, Inc. reserves the right to modify this software        */
/*  without notice.                                                       */
/*                                                                        */
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               www.expresslogic.com          */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX SMP Component                                                 */
/**                                                                       */
/**   Altera Cyclone V low-level initialization                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Ensure Cyclone V hardware is selected */
#ifndef soc_cv_av
#define soc_cv_av
#endif
#include "common.h"
#include <kernel.h>
//#include "baule17_section.h"
#include "memorymap.h"
#include "kernel_inc.h"
#include "tx_alt_low_level.h"
#include "alt_cache.h"
#include "alt_clock_manager.h"
#include "alt_globaltmr.h"
#include "tx_api.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "js_l2c310_reg.h"
#include "cpu_api.h"
#include <stdint.h>

#include "debug.h"

typedef enum {
	SMP_CORE0 = 0,		// Core0
	SMP_CORE1,		// Core1
	SMP_MAX,		// Core数
} SMP_CORE_T;

/* SCU control register */
#define SCU_CTRL (0x00)		// SCU制御レジスタ(SCU_BASEからのオフセットアドレス), 19/04/22
#define SCU_INVALIDATE (0x0c)		// SCU全無効化レジスタ
#define SCU_CTRL_EN (1 << 0)		// SCUイネーブルビット, 19/04/22
#define SCU_INVALIDATE_ALL (0x0000ffff)		// SCU全タグRAM無効化

/* reset manager */
#define MPUMODRST             0xFFD05010U
#define MPUMODRST_CPU1        (1<<1)

extern void *__tx_interrupt_vector;

/* ThreadX timer interrupt */
extern void _tx_timer_interrupt(void);

/* timer interrupt handler */
static void tx_alt_timer_interrupt(uint32_t icciar, void *data)
{
	/* clear pending interrupt */
	alt_gpt_int_if_pending_clear(SYS_TIMER);
	
	/* call ThreadX timer interrupt handler */
	_tx_timer_interrupt();
}


/* MMU translation table - 16KB aligned at 16KB boundary */
//UINT32 __attribute__ ((aligned (0x4000))) tx_alt_ttb[SMP_MAX][0x1000];		// CORE0とCORE1のMMUテーブルを別とする, 19/04/24
UINT32 tx_alt_ttb[SMP_MAX][0x1000] __attribute__ ((aligned (0x4000), section(".bss.oc_ram"), zero_init));		// CORE0とCORE1のMMUテーブルを別とする, 19/04/24

#define ACTLR_FW (1 << 0)
#define ACTLR_SMP (1 << 6)
#define ACTLR_L1 (1 << 1)
#define ACTLR_L2 (1 << 2)
#define ACTLR_EXCL (1 << 7)		// 排他的キャッシュビット, 20/07/31
#define ACTLR_FL0W (1 << 3)		// Full Line 0 Writeビット, 20/07/31


/* Auxilliary Control Register */
register uint32_t ACTLR __asm("cp15:0:c1:c0:1");		// コプロセッサ-補助コントロールレジスタ, 19/11/14
static inline __attribute__((always_inline)) uint32_t ACTLR_READ(void)
{
	uint32_t ret = ACTLR;

	return ret;
}
static inline __attribute__((always_inline)) void ACTLR_WRITE(uint32_t val)
{
	ACTLR = val;
}

void tx_alt_smp_enable(void)
{
	// enable cache/TLB maintenance broadcast, L1/L2データプリフェッチを禁止、フルライン0ライトを許可、L2も同様の設定をおこなう, 20/08/03
	ACTLR_WRITE(ACTLR_READ() | ACTLR_SMP | ACTLR_FW | ACTLR_FL0W);
	__asm volatile ("DSB");
	__asm volatile ("ISB");
}

// MMU変換テーブル設定, 19/04/19
void tx_alt_setup_mmu_table(SMP_CORE_T core)
{
	/* clear .BSS section */
	memset( &tx_alt_ttb[core], 0x00, sizeof(tx_alt_ttb[core]));

	// BOOT_ROM:0x0～0x10000
	// 未使用エリア:0x10000～0x100000
	// QSPI:0x100000～0x2100000
	// SDRAM:0x2100000～0x80BFFFF
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x00000000, 0x03000000, (MMU_CB_SHARED | MMU_CB_WBACK | MMU_CB_TEX0_WALLOC));
	// SDRAM(IMAGEDATA):0x04000000～0x08000000
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x03000000, 0x04D00000, MMU_CB_BUFF);
#else
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x00000000, 0x05000000, (MMU_CB_SHARED | MMU_CB_WBACK | MMU_CB_TEX0_WALLOC));
	// SDRAM(IMAGEDATA):0x05000000～0x08000000
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x05000000, 0x03000000, MMU_CB_BUFF);
	//mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x05000000, 0x03000000, MMU_CB_NONE);
#endif
	// 0x07D00000-0x070FFFFFまでの設定を変更
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x07D00000, 0x00200000, MMU_CB_NONE );
	// SDRAM(FPGA LOG):0x08000000～0x80FFFFF
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x07F00000, 0x00200000, MMU_CB_NONE );
	// SDRAM(予約,未実装):0x8100000～0x7FFFFFFF
	// 未使用エリア:0x80000000～0xBFFFFFFF
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x08100000, 0xB7F00000, MMU_CB_NONE );
	// 内蔵モジュールエリア:0xC0000000～0xFFFFFFFF
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0xC0000000, 0x40000000, MMU_CB_NONE );
}

// L2C310制御, 20/07/30
#define L2C_INSTRUCTION_PREFETCH (1 << 29)		// 命令プリフェッチ許可ビット
void tx_alt_cache_l2_instruction_prefetch_enable(void)
{
	uint32_t l2c_prefetch_ctrl = IOREG32(L2C_BASE, L2C_REG15_PREFETCH_CTRL);
	
	l2c_prefetch_ctrl |= L2C_INSTRUCTION_PREFETCH;		// 命令プリフェッチビットセット
	IOREG32(L2C_BASE, L2C_REG15_PREFETCH_CTRL) = l2c_prefetch_ctrl;
}

#if 0 //2024-04-08　エージング暴走問題の対策として試しに入れたが、効果がないので元に戻した
void tx_alt_cache_l2_instruction_prefetch_disable(void)
{
	uint32_t l2c_prefetch_ctrl = IOREG32(L2C_BASE, L2C_REG15_PREFETCH_CTRL);
	
	l2c_prefetch_ctrl &= ~L2C_INSTRUCTION_PREFETCH;		// 命令プリフェッチビットセット
	IOREG32(L2C_BASE, L2C_REG15_PREFETCH_CTRL) = l2c_prefetch_ctrl;
}
#endif


#define L2C_EXCLUSIVE_CONFIG (1 << 12)		// 排他的キャッシュ許可ビット
void tx_alt_cache_l2_exclusive_enable(void)
{
	uint32_t l2c_auxiliary_ctrl = IOREG32(L2C_BASE, L2C_REG1_AUX);
	
	l2c_auxiliary_ctrl |= L2C_EXCLUSIVE_CONFIG;		// 排他的キャッシュビットセット
	IOREG32(L2C_BASE, L2C_REG1_AUX) = l2c_auxiliary_ctrl;
}

void tx_alt_cache_l2_exclusive_disable(void)
{
	uint32_t l2c_auxiliary_ctrl = IOREG32(L2C_BASE, L2C_REG1_AUX);
	
	l2c_auxiliary_ctrl &= ~L2C_EXCLUSIVE_CONFIG;		// 排他的キャッシュビットクリア
	IOREG32(L2C_BASE, L2C_REG1_AUX) = l2c_auxiliary_ctrl;
}

#define L2C_FULL_LINE_ZERO_WRITE (1 << 0)		// フルライン0書込み許可ビット
void tx_alt_cache_l2_full_line_zero_write_enable(void)
{
	uint32_t l2c_auxiliary_ctrl = IOREG32(L2C_BASE, L2C_REG1_AUX);
	
	l2c_auxiliary_ctrl |= L2C_FULL_LINE_ZERO_WRITE;		// フルライン0書込みビットセット
	IOREG32(L2C_BASE, L2C_REG1_AUX) = l2c_auxiliary_ctrl;
}

// L1キャッシュ許可
void tx_alt_cache_l1_system_enable(void)
{
	/* Parity should be turned on before anything else. */
	alt_cache_l1_parity_enable();
	alt_cache_l1_instruction_enable();
	alt_cache_l1_data_enable();
	alt_cache_l1_branch_enable();
}

// L2キャッシュ許可
void tx_alt_cache_l2_system_enable(void)
{
	//alt_cache_l2_init();
#if 0 //2024-04-08 エージング暴走問題の対策として試しに入れたが、効果がないので元に戻した
	tx_alt_cache_l2_instruction_prefetch_disable();
#else
	tx_alt_cache_l2_instruction_prefetch_enable();		// 命令プリフェッチ有効(データプリフェッチは有効にしない), 20/07/31
#endif

	tx_alt_cache_l2_exclusive_disable();		// L2排他的キャッシュ無効, 20/07/30
	tx_alt_cache_l2_full_line_zero_write_enable();		// フルライン0書込み有効, 20/07/31
	alt_cache_l2_parity_enable();
	alt_cache_l2_enable();
}

// L1/L2キャッシュ許可
void tx_alt_cache_system_enable(void)
{
	tx_alt_cache_l1_system_enable();		// L1キャッシュ許可
	
	tx_alt_cache_l2_system_enable();		// L2キャッシュ許可
}

/* system initialization */
void tx_alt_initialize_low_level(void)
{
	uint32_t freq;
	T_DINH dinh = {0};
	
	alt_cache_l2_init();

	// MMUとキャッシュの禁止(Core0/1とも), 19/04/19
    alt_cache_system_disable();		// Core0はL1/L2キャッシュ無効, 19/04/22
	mmu_disable();		// MMUを無効
	
	// SCU初期設定(Core0のみ), 19/04/22
	IOREG32(SCU_BASE, SCU_CTRL) = (IOREG32(SCU_BASE, SCU_CTRL) & ~SCU_CTRL_EN);		// SCU停止
	IOREG32(SCU_BASE, SCU_INVALIDATE) = SCU_INVALIDATE_ALL;		// SCUタグRAM全無効
	
	// MMU変換テーブル設定(Core0側)
	tx_alt_setup_mmu_table(SMP_CORE0);
	
	// MMUの許可(Core0/1とも), 19/04/19
//	mmu_config((UINT32)tx_alt_ttb[SMP_CORE0] | MMU_CFG_SHARED | MMU_CFG_INNER_WB_ALLOC | MMU_CFG_RGN_WB_WALLOC);		// MMU変換テーブルベースアドレス設定
	mmu_config((UINT32)tx_alt_ttb[SMP_CORE0] | MMU_CFG_SHARED);		// MMU変換テーブルベースアドレス設定
	mmu_enable();		// MMUを有効
	
	// SCU稼働
	IOREG32(SCU_BASE, SCU_CTRL) = (IOREG32(SCU_BASE, SCU_CTRL) | SCU_CTRL_EN);
	
	// キャッシュの許可(Core0/1とも), 19/04/19
	tx_alt_cache_system_enable();		// Core0はL1/L2キャッシュ有効, 20/07/31
	
	// SMPモードセット(Core0/1とも), 19/04/22
	tx_alt_smp_enable();
	
	/* Initialize GIC interrupt controller(Core0のみ) */
	alt_int_global_init();
	alt_int_global_enable();
	/* Initialize CPU interface of the GIC for CPU0 */
	alt_int_cpu_init();
	alt_int_dist_priority_set(ALT_INT_INTERRUPT_SGI0, IPL_USER_LOWEST);		// SGI0割込みレベル設定, 19/05/29
	alt_int_cpu_enable();
	
	// 64bitグローバルタイマ稼働, 19/04/26
	alt_globaltmr_init();		// タイマ開始
	
	/* Configure system timer: */
	freq = alt_clk_ext_clk_freq_get(ALT_CLK_OSC1);
	/* set counter */
	alt_gpt_counter_set(ALT_GPT_OSC1_TMR0, freq / TX_TIMER_TICKS_PER_SECOND);
	/* set periodic mode */
	alt_gpt_mode_set(ALT_GPT_OSC1_TMR0, ALT_GPT_RESTART_MODE_PERIODIC);
	/* set prescaler */
	alt_gpt_prescaler_set(ALT_GPT_OSC1_TMR0, 0);
	/* clear pending interrupts */
	alt_gpt_int_if_pending_clear(ALT_GPT_OSC1_TMR0);
	/* enable timer interrupts */
	alt_gpt_int_enable(ALT_GPT_OSC1_TMR0);
	/* start timer */
	// alt_gpt_tmr_start(ALT_GPT_OSC1_TMR0);		// システムタイマの開始をMAINタスクに移動, 19/05/20
	/* install timer interrupt handler */
	//alt_int_isr_register(ALT_INT_INTERRUPT_TIMER_OSC1_0_IRQ, tx_alt_timer_interrupt, NULL);
	dinh.inhatr = TA_HLNG;
	dinh.inthdr = tx_alt_timer_interrupt;
	def_inh(ALT_INT_INTERRUPT_TIMER_OSC1_0_IRQ, &dinh);
	alt_int_dist_priority_set(ALT_INT_INTERRUPT_TIMER_OSC1_0_IRQ, IPL_KERNEL_NORMAL);		// システムタイマ割込みレベル設定, 19/05/29
	/* route SPI interrupt to CPU0 */
	alt_int_dist_target_set(ALT_INT_INTERRUPT_TIMER_OSC1_0_IRQ, 1);
	/* enable the interrupt */
	alt_int_dist_enable(ALT_INT_INTERRUPT_TIMER_OSC1_0_IRQ);

	/* boot the cpu1: */
	if ((uintptr_t) &__tx_interrupt_vector != 0) {
		/* jump to interrupt vector base address at reset */
		*((uint32_t *) 0) = 0xE51FF004U;  /* LDR pc, [pc,#-4] */
		*((uint32_t *) 4) = (uintptr_t) &__tx_interrupt_vector;
		
		/* flush data cache */
		alt_cache_system_clean((void *) 0, ALT_CACHE_LINE_SIZE);
	}
	
	/* ensure we're in sync */
	__asm volatile ("DSB");
	
	/* release cpu1 from reset */
	// CPU1リセット解除をmain()に移動, 19/05/30
	//static volatile uint32_t *const mpumodrst = (uint32_t *) MPUMODRST;  //2024-11-21 1core
	//*mpumodrst &= ~MPUMODRST_CPU1;	 //2024-11-21 1core
}

/* low-level initialization for cpu1 */
void _tx_alt_smp_initialize_low_level(void)
{
	// MMUとキャッシュの禁止(Core0/1とも), 19/04/19
    alt_cache_l1_disable_all();		// Core1はL1キャッシュ無効(L2キャッシュは操作しない), 19/04/22
	mmu_disable();		// MMUを無効
	
	/* Place CPU1 specific initialization code here! */
	
	// MMU変換テーブル設定(Core1側)
	tx_alt_setup_mmu_table(SMP_CORE1);
	
	// MMUの許可(Core0/1とも), 19/04/19
//	mmu_config((UINT32)tx_alt_ttb[SMP_CORE1] | MMU_CFG_SHARED | MMU_CFG_INNER_WB_ALLOC | MMU_CFG_RGN_WB_WALLOC);		// MMU変換テーブルベースアドレス設定
	mmu_config((UINT32)tx_alt_ttb[SMP_CORE1] | MMU_CFG_SHARED);		// MMU変換テーブルベースアドレス設定
	mmu_enable();		// MMUを有効
	
	// キャッシュの許可(Core0/1とも), 19/04/19
	tx_alt_cache_l1_system_enable();		// Core1はL1キャッシュ有効(L2キャッシュは操作しない), 20/07/31
	
	// SMPモードセット(Core0/1とも), 19/04/22
	tx_alt_smp_enable();
	
	/* Initialize CPU interface of the GIC for CPU1 */
	if(ALT_E_SUCCESS == alt_int_cpu_init())
	{
		alt_int_dist_priority_set(ALT_INT_INTERRUPT_SGI0, IPL_USER_LOWEST);		// SGI0割込みレベル設定, 19/05/29
		alt_int_cpu_enable();
	}
}

// L1/L2キャッシュパージ, 20/07/28
void tx_alt_cache_purge_all(void)
{
	alt_cache_l1_data_clean_all();		// L1キャッシュをライトバック
	alt_cache_l2_purge_all();			// L2キャッシュをライトバック&無効化
	alt_cache_l2_sync();				// L2キャッシュ同期
	alt_cache_l1_data_purge_all();		// L1キャッシュをライトバック&無効化
}

