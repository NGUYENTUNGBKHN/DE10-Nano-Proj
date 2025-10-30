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
extern void program_error(void);
extern void _soft_reset(void);
extern ALT_STATUS_CODE qspi_software_reset(void);
extern bool is_task_status_busy(void);
extern void setup_i2c0(void);
extern void setup_i2c3(void);

extern u16 calc_check_sum(u8 *ptr, u16 length);


extern void program_error(void);
extern void _debug_system_error(u16 task_id, u16 error_no, u16 tmsg, u16 arg1, u8 fatal_err);
