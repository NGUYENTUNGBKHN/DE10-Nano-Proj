/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_i2c_dp.h
    @brief      I2C Digital Potentiometer
    @date       2018/03/14
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/06/01 Development Dept at Tokyo
      -# Initial Version
      -# MCP4651T-104E/ST with digital potentiometer
******************************************************************************/



/* Public Definitions ----------------------------------------------------------- */
/* return value */
#define E_I2C_SEND    -4096          /* 0xE000: I2C送信失敗エラー                   */
/* slave address */

/* Public Functions ----------------------------------------------------------- */

/* I2C_0 */

ER _hal_uv_gain_write(u8 tap);
ER _hal_uv1_gain_write(u8 tap);

/* EOF */
