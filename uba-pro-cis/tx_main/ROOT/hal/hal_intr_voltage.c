/******************************************************************************/
/*! @addtogroup Main
    @file       hal_int_voltage.c
    @brief      Interrupt functions for 24V power low
    @date       2018/03/20
    @author     H. Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/20 H.Suzuki
      -# Initial Version
******************************************************************************/

/***************************** Include Files *********************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "common.h"
#include "custom.h"
#include "sensor_ad.h"

#include "hal_gpio_reg.h"
#include "gpio/js_gpio.h"

#define EXT
#include "com_ram.c"

/************************** Private Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/*********************************************************************//**
 * @brief		Power down detection proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _intr_low_voltage(void *arg)
{
	// backup power recovery flag
	iset_flg(ID_POWER_FLAG, EVT_POWER_VOLTAGE);

	GpioIsr_clear(GPIO_VDET);
}


/* EOF */
