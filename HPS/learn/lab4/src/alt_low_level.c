/******************************************************************************/
/*! @addtogroup Group2
    @file       alt_low_level.c
    @brief      
    @date       2025/11/14
    @author     Development Dept at Tokyo (nguyen-thanh-tung@jcm-hq.co.jp)
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/
/*******************************************************************************
**                                INCLUDES
*******************************************************************************/
#include "alt_low_level.h"
#include "alt_cache.h"
#include "alt_clock_manager.h"
#include "alt_globaltmr.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "js_l2c310_reg.h"
#include "cpu_api.h"
#include <stdint.h>
#include <string.h>
#include "hw_init.h"
#include <stdio.h>
#include "alt_mmu.h"
/*******************************************************************************
**                       INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define CPU1_START_ADDR *(volatile unsigned int *)0xFFFFFE00
#define CPU1_LOAD_ADDR 0x00100000U
extern void *Reset_Handler;

#define MPUMODRST        0xFFD05010U
#define MPUMODRST_CPU1   (1<<1)
#define MMU_CB_DEVICE    0x00000012
/*******************************************************************************
**                      COMMON VARIABLE DEFINITIONS
*******************************************************************************/


/*******************************************************************************
**                      INTERNAL VARIABLE DEFINITIONS
*******************************************************************************/
UINT32 tx_alt_ttb[SMP_MAX][0x1000] __attribute__ ((aligned (0x4000), section(".bss.oc_ram"), zero_init));

/*******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/


/*******************************************************************************
**                          FUNCTION DEFINITIONS
*******************************************************************************/
#define L2C_INSTRUCTION_PREFETCH (1 << 29)		// 命令プリフェッチ許可ビット
void tx_alt_cache_l2_instruction_prefetch_enable(void)
{
	uint32_t l2c_prefetch_ctrl = IOREG32(L2C_BASE, L2C_REG15_PREFETCH_CTRL);

	l2c_prefetch_ctrl |= L2C_INSTRUCTION_PREFETCH;		// 命令プリフェッチビットセット
	IOREG32(L2C_BASE, L2C_REG15_PREFETCH_CTRL) = l2c_prefetch_ctrl;
}

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
	tx_alt_cache_l2_instruction_prefetch_enable();		// 命令プリフェッチ有効(データプリフェッチは有効にしない), 20/07/31


	tx_alt_cache_l2_exclusive_disable();		// L2排他的キャッシュ無効, 20/07/30
	tx_alt_cache_l2_full_line_zero_write_enable();		// フルライン0書込み有効, 20/07/31
	alt_cache_l2_parity_enable();
	alt_cache_l2_enable();
}

void tx_alt_cache_system_enable(void)
{
	tx_alt_cache_l1_system_enable();		// L1キャッシュ許可

	tx_alt_cache_l2_system_enable();		// L2キャッシュ許可
}

void alt_low_level_init(void)
{
	hw_init();
	alt_cache_l2_init();
}

void alt_setup_mmu_table(SMP_CORE_T core)
{
	alt_cache_system_disable();		// Core0はL1/L2キャッシュ無効, 19/04/22
	mmu_disable();
	/* clear .BSS section */
	memset( &tx_alt_ttb[core], 0x00, sizeof(tx_alt_ttb[core]));

	// BOOT_ROM:0x0～0x10000
	// 未使用エリア:0x10000～0x100000
	// QSPI:0x100000～0x2100000
	// SDRAM:0x2100000～0x80BFFFF
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x00000000, 0x05000000, (MMU_CB_SHARED | MMU_CB_WBACK | MMU_CB_TEX0_WALLOC));
	// SDRAM(IMAGEDATA):0x05000000～0x08000000
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x05000000, 0x03000000, MMU_CB_BUFF);
	//mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x05000000, 0x03000000, MMU_CB_NONE);
	// 0x07D00000-0x070FFFFFまでの設定を変更
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x07D00000, 0x00200000, MMU_CB_NONE );
	// SDRAM(FPGA LOG):0x08000000～0x80FFFFF
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x07F00000, 0x00200000, MMU_CB_NONE );
	// SDRAM(予約,未実装):0x8100000～0x7FFFFFFF
	// 未使用エリア:0x80000000～0xBFFFFFFF
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0x08100000, 0xB7F00000, MMU_CB_NONE );
	// 内蔵モジュールエリア:0xC0000000～0xFFFFFFFF
	mmu_tbl_set( (UINT32 *)tx_alt_ttb[core], 0xC0000000, 0x40000000, MMU_CB_NONE );

	mmu_config((UINT32)tx_alt_ttb[core] | MMU_CFG_SHARED);
    mmu_enable();
    tx_alt_cache_system_enable();
}


void alt_low_level_enable_mmu(void)
{
    unsigned int sctlr;

    __asm {
        MRC p15, 0, sctlr, c1, c0, 0
    }

    sctlr |= 0x1;

    __asm {
        MCR p15, 0, sctlr, c1, c0, 0
        ISB
    }
}

void alt_low_level_start_cpu1(void)
{
//	printf("TTB entry for 0xFFD00000 = %08x\n", tx_alt_ttb[0][(0xFFD00000 >> 20)]);
    static volatile uint32_t *const mpumodrst = (uint32_t *)MPUMODRST; // 2024-11-21 1core
    if ((uintptr_t)&Reset_Handler != 0)
    {
        /* jump to interrupt vector base address at reset */
        *((uint32_t *)0) = 0xE51FF004U; /* LDR pc, [pc,#-4] */
        *((uint32_t *)4) = (uintptr_t)&Reset_Handler;

        /* flush data cache */
        alt_cache_system_clean((void *)0, ALT_CACHE_LINE_SIZE);
    }
    __asm volatile ("DSB");
    *mpumodrst &= ~MPUMODRST_CPU1; // 2024-11-21 1core
    __asm volatile ("DSB");
}

/******************************** End of file *********************************/

