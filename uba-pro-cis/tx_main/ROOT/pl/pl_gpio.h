/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_gpio.h
    @brief      XGPIO control header
    @date       2018/05/29
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/05/29 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"


#ifndef _PL_GPIO_HEADER_H		/* prevent circular inclusions */
#define _PL_GPIO_HEADER_H		/* by using protection macros */

#define FPGA_SCANBIN_IRQ_INTR_ID	OSW_INT_FPGA_IRQ29
#define FPGA_SCANMLT_IRQ_INTR_ID	OSW_INT_FPGA_IRQ30

#define FEED_ENC_IRQ_INTR_ID	OSW_INT_FPGA_IRQ36
#define STACKER_ENC_IRQ_INTR_ID	OSW_INT_FPGA_IRQ37

#define PB_ENC_IRQ_INTR_ID	OSW_INT_FPGA_IRQ38 //not use

/* Public Functions ----------------------------------------------------------- */
void _pl_gpio_init(void);

#endif /* _PL_GPIO_HEADER_H */
/* EOF */
