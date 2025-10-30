/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_i2c_iox.h
    @brief      I2C I/O expander, analog mux driver.
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
#pragma once
#include "js_oswapi.h"


/* Public Definitions ----------------------------------------------------------- */
/* return value */
#define E_I2C_SEND    -4096          /* 0xE000: I2C送信失敗エラー                   */
#define E_ARG         -8192          /* 0xF000: 引数エラー                   */
/* slave address */

/* Public Functions ----------------------------------------------------------- */

/* I2C 1 */
INT32 _hal_i2c3_init_iox(void);
INT32 _hal_i2c3_write_led_red(UINT8 data);
INT32 _hal_i2c3_write_led_green(UINT8 data);
INT32 _hal_i2c3_write_led_blue(UINT8 data);
INT32 _hal_i2c3_read_dipsw1(UINT8 *dipsw_value);
INT32 _hal_i2c3_read_dipsw2(UINT8 *dipsw_value);
INT32 _hal_i2c3_init_tca9535(UINT8 devno);
INT32 _hal_i2c3_read_dipsw(UINT8 dipsw_no, UINT8 *dipsw_value);
INT32 _hal_i2c3_write_led_bezel(UINT16 data);
s32 _hal_i2c3_read_bezel(u16 *data);

//2023-12-22
INT32 _hal_i2c3_read_write_p0(u8 data, u8 change_bit);
int _hal_i2c3_for_led_tca9535(u8 data, u8 change_bit);

/* EOF */
