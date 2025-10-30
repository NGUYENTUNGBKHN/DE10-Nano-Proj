/******************************************************************************/
/*! @addtogroup Group1
    @file       sub_functions.h
    @brief      sub fanctions header
    @date       2018/02/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/
#pragma once
/***************************** Include Files *********************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/
extern T_MSG_BASIC ex_main_msg;

/*----------------------------------------------------------*/
/*			Public Functions								*/
/*----------------------------------------------------------*/
extern void _kernel_synch_cache(void);
extern void program_error(void);
extern void _soft_reset(void);
extern void set_test_ld_mode(u8 set);
extern void set_test_allacc_mode(u8 set);
extern void set_test_allrej_mode(u8 set);


extern void set_test_mode(void);

extern void bif_device_usb_download_smp(void);
extern bool is_test_mode(void);
extern bool is_task_status_busy(void);

extern bool is_all_accept_mode(void);
extern bool is_all_reject_mode(void);

extern bool is_ld_mode(void);

extern bool is_box_set(void);
extern bool is_uv_led_check(void);
extern bool is_cisa_lvds_lock(void);
extern bool is_cisb_lvds_lock(void);

extern void _main_init_performance_test_mio(void);
extern void terminate_main_sys(void);
extern void bif_if_download_smp(void);
extern void bif_device_usb_download_smp(void);
extern void bif_host_usb_download_smp(void);
extern void set_gpio_irq(void);
extern u32 write_fram_tmpadj(void);
extern void setup_i2c0(void);
extern void setup_i2c3(void);
extern bool is_icb_enable(void);
extern ER rfid_check_system_information(void);
extern u8 get_reject_code_icb(u8 reject);

extern u32 is_cleaning_required(void);
extern u32 is_position_sensor_da_and_gain_max(void);
extern u16 culc_fram_adj_sum(void);
extern u8 check_fram_tmpadj_sum(void);
extern u16 calc_check_sum(u8 *ptr, u16 length);
extern void calc_sha1_reset(u8* start, u8* end, u8* seed_value, u8* result);
extern bool calc_sha1_func(void);

extern u8 _main_check_fram_adj_sum(void);
extern void unit_style_check(void);
extern u8 unit_voltage_check_uba(void);
extern bool is_legacy_mode_enable(void);
extern void _debug_system_error(u16 task_id, u16 error_no, u16 tmsg, u16 arg1, u8 fatal_err);
