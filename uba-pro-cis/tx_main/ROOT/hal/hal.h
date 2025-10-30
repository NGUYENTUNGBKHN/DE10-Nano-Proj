/******************************************************************************/
/*! @addtogroup Group1
    @file       hal.h
    @brief      Motor cotrol(feed/centering/APB)
    @date       2012/09/27
    @author     T.Yokoyama
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2012/09/27 T.Yokoyama
      -# Initial Version
******************************************************************************/
#pragma once
#include "itron.h"
#include "memorymap.h"

// hal_gpio_io.c



u8 __hal_5v_dect_read(void);
u8 __hal_su_select_read(void);
u8 __hal_reset_det_read(void);



u8 __hal_fusb_dect_read(void);
s32 is_usb0_connected(void);

// hal_status_leds.c
void _hal_status_led(u8 color);

// hal_usb.c
void FrontUSBConnect(void);
void RearUSBConnect(void);
void USBDisconnect(void);
// hal_operation_usb.c
void OperationUSBConnect(void);
//void OperationUSBDisconnect(void);

#define	USB_PORT_CNT		2
#define	USB0_BASE			0xFFB00000
#define	USB1_BASE			0xFFB40000
/* EOF */
