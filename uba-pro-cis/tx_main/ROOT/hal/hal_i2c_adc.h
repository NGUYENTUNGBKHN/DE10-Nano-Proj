/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_i2c_adc.h
    @brief      I2C A/D converter
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
      -# ADC081C021CIMK with A/D converter
******************************************************************************/



/* Public Definitions ----------------------------------------------------------- */
/* return value */
#define E_I2C_SEND    -4096          /* 0xE000: I2C送信失敗エラー                   */
/* slave address */

/* Public Functions ----------------------------------------------------------- */

/* I2C_0 */
ER _hal_voltage_ad_read(u8 *value);

/* EOF */
