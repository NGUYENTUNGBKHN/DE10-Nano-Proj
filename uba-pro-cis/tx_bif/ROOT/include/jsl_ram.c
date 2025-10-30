/******************************************************************************/
/*! @addtogroup BIF
    @file       jsl_ram.c
    @brief      JSL ware variable
    @date       2021/03/15
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/03/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

#ifndef SRC_INCLUDE_JSL_RAM_H_
#define SRC_INCLUDE_JSL_RAM_H_

#include "js_oswapi_cfg.h"
#include "js_oswapi.h"
#include "js_io.h"
#include "i2c/js_i2c.h"
#include "spi/js_spi.h"
#include "uart/js_uart.h"
#include "cortex_a9/cpu_api.h"
#include "gpio/js_gpio.h"
#include "ptimer/js_ptimer_reg.h"
#include "rstmgr/js_rstmgr_reg.h"
#include "crc16/js_crc16.h"
#include "qspi_flash/js_qspi_flash.h"
#include "spi_fram/js_spi_fram.h"
#include "sdrc/js_sdr_reg.h"
#include "l3regs/js_l3regs_reg.h"
#include "intc/js_intc_reg.h"

#include "hal_gpio_reg.h"
#include "reg_cyclone5.h"

#if defined(EXT)
	#define EXTERN extern
#else
	#undef EXTERN
	#define EXTERN
#endif

EXTERN I2C_HANDLE hI2c0;
EXTERN I2C_HANDLE hI2c3;
EXTERN SPI_HANDLE hSpi;
#ifndef  FLASH_QSPI_MODE
EXTERN QSPI_FLASH_HANDLE hQFlash;
#endif
EXTERN SPI_FRAM_HANDLE hFram;
EXTERN UART_HANDLE hUart0;

#endif
