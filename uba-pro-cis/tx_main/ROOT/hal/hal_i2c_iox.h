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

#define BIT_F_LED_RED	0x08
#define BIT_F_LED_GREEN	0x10
#define BIT_F_LED_BLUE	0x20
#define LED_COLOR_RED (BIT_F_LED_RED)
#define LED_COLOR_YELLOW (BIT_F_LED_RED|BIT_F_LED_GREEN)
#define LED_COLOR_GREEN (BIT_F_LED_GREEN)
#define LED_COLOR_BLUE (BIT_F_LED_BLUE)
#define LED_COLOR_PURPLE (BIT_F_LED_RED|BIT_F_LED_BLUE)
#define LED_COLOR_WHITE (BIT_F_LED_RED|BIT_F_LED_GREEN|BIT_F_LED_BLUE)
#define LED_COLOR_OFF (0)
/* Public Functions ----------------------------------------------------------- */

/* I2C 1 */
INT32 _hal_i2c3_init_iox(void);

INT32 _hal_i2c3_read_dipsw1(UINT8 *dipsw_value);

s32 _hal_i2c3_read_bezel(u16 *data);

/* EOF */
