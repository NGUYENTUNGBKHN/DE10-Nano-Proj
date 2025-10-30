/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_motor.h
    @brief      motor control header
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


/*----------------------------------------------------------*/
/*			Public Functions								*/
/*----------------------------------------------------------*/
void _pl_motor_init(void);
void _pl_motor_final(void);

void _pl_gpio_set_enmt_uba(u8 set);

void _pl_feed_motor_speed_conrol(u8 speed, MOT_CMD_T mode);

void _pl_feed_motor_stop(void);
void _pl_feed_motor_cw(u8 speed);
void _pl_feed_motor_ccw(u8 speed);
void _pl_feed_enc_interrupt_enable(void);
void _pl_feed_enc_interrupt_disable(void);

void _pl_stacker_motor_stop(void);

void _pl_stacker_motor_cw_uba(void);
void _pl_stacker_motor_ccw_uba(void);

void _pl_stacker_enc_interrupt_enable(void);
void _pl_stacker_enc_interrupt_disable(void);
void _pl_apb_motor_speed_conrol(u8 pwm, MOT_CMD_T mode);
void _pl_apb_motor_stop(void);
void _pl_apb_motor_cw(u8 speed);
void _pl_apb_motor_ccw(u8 speed);

void _pl_centering_motor_speed_conrol(u8 speed, MOT_CMD_T mode);

void _pl_centering_motor_stop(void);
void _pl_centering_motor_cw(u8 speed);
void _pl_centering_motor_ccw(u8 speed);
u8 _pl_set_hmot_cur_uba(void);
u8 _pl_set_smot_cur_uba(u8 mode);
u8 _pl_set_cmot_cur(void);

/* EOF */
