/******************************************************************************/
/*! @addtogroup Group2
    @file       alt_low_level.h
    @brief      
    @date       2025/11/14
    @author     Development Dept at Tokyo (nguyen-thanh-tung@jcm-hq.co.jp)
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/
#ifndef _ALT_LOW_LEVEL_H_
#define _ALT_LOW_LEVEL_H_
#ifdef __cplusplus
extern "C"
{
#endif
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
typedef enum {
	SMP_CORE0 = 0,		// Core0
	SMP_CORE1,		// Core1
	SMP_MAX,		// Core数
} SMP_CORE_T;
/* CODE */
extern UINT32 tx_alt_ttb[SMP_MAX][0x1000];
extern void alt_low_level_init(void);
extern void alt_setup_mmu_table(SMP_CORE_T core);
extern void alt_low_level_enable_mmu(void);
extern void alt_low_level_start_cpu1(void);

#ifdef __cplusplus
}
#endif
#endif

