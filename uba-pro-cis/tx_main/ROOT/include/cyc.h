/******************************************************************************/
/*! @addtogroup Group1
    @file       cyc.h
    @brief      cycle handler header
    @date       2018/02/26
    @author     Development Dept at Tokyo
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

/*----- CIS Position Sequence Code ----------------------------------*/
//_cyc_validation_mode
// 連続取り込みのタイミング呼び出すと識別エラーとなるので,とりあえず廃止にしておく #define VALIDATION_CHECK_MODE_WAIT			0x0200
#define VALIDATION_CHECK_MODE_DISABLE		0x0100
#define VALIDATION_CHECK_MODE_RUN			0x0200

//_cyc_validation_status
#define VALIDATION_ST_INI		    0x0000
#define VALIDATION_ST_RUN	        0x0100
#define VALIDATION_ST_RUN_END	    0x0200
#define VALIDATION_ST_WAIT_RUN	    0x0300
#define VALIDATION_ST_RUN_RETRY	    0x0400

/*----------------------------------------------------------*/
/*			Public Functions								*/
/*----------------------------------------------------------*/
void systimer_init(void);
void _cyc_sensor_proc(void);
void _sensor_ctrl_config(u16 seq_no, u16 off_time, u16 on_time);
void _cyc_validation_proc(void);
void _validation_ctrl_set_mode(u16 mode);

/* EOF */
