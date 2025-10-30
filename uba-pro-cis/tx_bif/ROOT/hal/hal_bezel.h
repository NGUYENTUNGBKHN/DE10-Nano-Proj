/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_bezel.h
    @brief      Bezel LED driver.
    @date       2018/03/14
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/14 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/


/* Public Functions ----------------------------------------------------------- */


// hal_i2c_iox.c
//void _hal_bezel_led_control(u8 bz_num, u8 set);
u32 _hal_write_bezel_led(u32 data);
/* EOF */
