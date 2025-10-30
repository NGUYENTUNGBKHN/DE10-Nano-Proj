/******************************************************************************/
/*! @addtogroup Group1
    @file       motor_ctrl.h
    @brief      motor control header
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


#define	MOT_PWM_STACKER_FULL			100			/* 100.0% ON:0.100ms, OFF:0.000ms */

	#define MOT_PWM_STACKER_12000RPM_SPEED	0x1200			/* 12000RPM */
	#define MOT_PWM_STACKER_10000RPM_SPEED  0x1000			/* 10000RPM */
	#define MOT_PWM_STACKER_9000RPM_SPEED   0x0900			/*  9000RPM */
	#define MOT_PWM_STACKER_8000RPM_SPEED   0x0800			/*  8000RPM */
	#define MOT_PWM_STACKER_6000RPM_SPEED   0x0600			/*  6000RPM */
	#define MOT_PWM_STACKER_5500RPM_SPEED   0x0550			/*  5500RPM */
	#define MOT_PWM_STACKER_4500RPM_SPEED   0x0450			/*  4500RPM */
	#define MOT_PWM_STACKER_4000RPM_SPEED   0x0400			/*  4000RPM */
	#define MOT_PWM_STACKER_9500RPM_SPEED   0x0950			/*  9500RPM */
/*----------------------------------------------------------*/
/*			Public Functions								*/
/*----------------------------------------------------------*/
extern u8 init_motor_max_current(void);
/* feed motor control */
extern u8 motor_ctrl_feed_fwd(u16 speed, u16 drive_pulse);
extern u8 motor_ctrl_feed_rev_3(u16 speed, u16 drive_pulse, u8 reject_flag);
extern u8 motor_ctrl_feed_rev(u16 speed, u16 drive_pulse);

extern void motor_ctrl_feed_stop(void);
extern void motor_ctrl_feed_set_pulse(u16 drive_pulse);
extern bool is_motor_ctrl_feed_stop(void);

/* stacker motor control */
extern u8 motor_ctrl_stacker_fwd(u16 speed, u16 drive_pulse, u16 current_max);
extern u8 motor_ctrl_stacker_fwd2(u16 speed, u16 drive_pulse, u16 current_max);
extern u8 motor_ctrl_stacker_rev2(u16 speed, u16 drive_pulse, u16 current_max);
extern u8 motor_ctrl_stacker_rev(u16 speed, u16 drive_pulse, u16 current_max);

extern void motor_ctrl_stacker_stop(void);
extern u32 motor_ctrl_stacker_brake(u16 retry);
extern u32 motor_ctrl_stacker_free(u16 retry);
extern u32 motor_ctrl_stacker_polling(u16 retry);
extern bool is_motor_ctrl_stacker_stop(void);

extern void motor_ctrl_stacker_set_pulse(u16 drive_pulse);
extern void motor_ctrl_stacker_set_full(u16 full_check);


/* centering motor control */
 extern u8 motor_ctrl_centering_fwd(u8 speed, u8 drive_home_cnt, u16 timeout);
extern u8 motor_ctrl_centering_rev(u8 speed, u8 drive_home_cnt, u16 timeout);


extern void motor_ctrl_centering_stop(void);
extern bool is_motor_ctrl_centering_stop(void);
extern u8 motor_ctrl_centering_set_max_current(u16 current_max);

/* apb motor control */
extern u8 motor_ctrl_apb_fwd(u8 pwm, u16 drive_home_cnt);
extern u8 motor_ctrl_apb_rev(u8 pwm, u16 drive_home_cnt);
extern void motor_ctrl_apb_stop(void);
extern bool is_motor_ctrl_apb_stop(void);

extern u8 motor_ctrl_pb_set_max_current(u16 current_max);
extern u8 motor_ctrl_shutter_set_max_current(u16 current_max);

extern void motor_ctrl_stacker_set_full_check(u16 drive_pulse, u8 full_check);
#if defined(UBA_RTQ)
extern void motor_ctrl_stacker_set_drive_check(u16 drive_pulse);
#endif




