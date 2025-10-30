/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_i2c_eeprom.h
    @brief      I2C EEPROM
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
******************************************************************************/



/* Public Definitions ----------------------------------------------------------- */
/* return value */
#define E_I2C_SEND    -4096          /* 0xE000: I2C送信失敗エラー                   */
#define E_I2C_PARM    -8192          /* 0xF000: 引数エラー                   */
/* slave address */

/* Public Functions ----------------------------------------------------------- */

/* I2C 3 */
ER _hal_read_eeprom(u8 *dst, u16 addr, u16 len);
ER _hal_write_eeprom(u8 *dst, u16 addr, u16 len);

/* EOF */
