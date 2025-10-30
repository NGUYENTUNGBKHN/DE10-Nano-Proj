/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_pos.h
    @brief      cis sensor header file
    @date       2019/04/22
    @author     yuji-kenta
    @par        Revision
    @par        Copyright (C)
    2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2019/04/22 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#pragma once

#ifndef _PL_POS_HEADER_H		/* prevent circular inclusions */
#define _PL_POS_HEADER_H		/* by using protection macros */

u8 _pl_dac_wait_ready(void);
u8 _pl_pos_init_da(void);

u8 _pl_set_entrance_posi_da(void);
u8 _pl_set_centering_posi_da(void);
u8 _pl_set_apb_in_posi_da(void);
u8 _pl_set_apb_out_posi_da(void);
u8 _pl_set_exit_posi_da(void);
u8 _pl_set_box12_posi_da(void);
u8 _pl_set_box_home_posi_da(void);
u8 _pl_set_nfull_posi_da(void);
u8 _pl_set_uv_da(void);/* 2022-01-25 */
u8 _pl_set_uv1_da(void);

u8 _pl_set_ent_threshold_posi_da(void);
u8 _pl_set_ext_threshold_posi_da(void);
u8 _pl_position_sensor_gain(u8 gain);
// motor current
u8 _pl_set_feed_motor_current(void);
u8 _pl_set_stacker_motor_current(void);
u8 _pl_set_centering_motor_current(void);
u8 _pl_set_uv_da(void);

#endif
