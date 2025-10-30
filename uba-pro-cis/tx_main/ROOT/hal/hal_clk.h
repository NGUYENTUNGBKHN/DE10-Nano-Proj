/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_clk.h
    @brief      SoC動作クロック制御ドライバファイル。
    @date       2021/12/17
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/12/17 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#include "soc_cv_av/alt_clock_manager.h"

typedef enum
{
	MPU_CLOCK_HIGH = 1,
	MPU_CLOCK_LOW
} MPU_CLOCK;
/* Public Functions ----------------------------------------------------------- */
void set_main_pll(alt_freq_t new_freq);
void set_peri_pll(alt_freq_t new_freq);
void disable_unused_peripheral(void);
void change_mpu_clock(MPU_CLOCK clock);
void set_mpu_clock(alt_freq_t new_fre);
void set_main_pll_down(u32 div);
void get_pll_info(void);
/* EOF */
