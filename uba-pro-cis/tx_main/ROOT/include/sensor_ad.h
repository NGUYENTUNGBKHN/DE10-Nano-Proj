/******************************************************************************/
/*! @addtogroup Group1
    @file       sensor_ad.h
    @brief      main sensor function header
    @date       2021/04/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/04/24 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#ifndef _SRC_INCLUDE_SENSOR_AD_H_
#define _SRC_INCLUDE_SENSOR_AD_H_

/*############################################################################*/
/*#                                                                          #*/
/*#  Functions                                                               #*/
/*#                                                                          #*/
/*############################################################################*/
extern u8 set_position_da(void);
extern u8 set_position_ga(void);
extern u8 set_uv_gain(void);
extern u8 set_uv_da(void);
extern void start_ad(void);
extern void stop_ad(void);
extern void _ir_ad_feed_motor_pulse(void);
extern void change_ad_sampling_mode(u16 mode);

#endif /* _SRC_INCLUDE_SENSOR_AD_H_ */
