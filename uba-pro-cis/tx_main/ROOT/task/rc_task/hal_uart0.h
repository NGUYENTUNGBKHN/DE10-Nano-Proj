/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_uart0.h
    @brief      RC T/Q serial communication peripheral header file.
    @date       2023/10/17
    @author     JCM. Tokyo R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
    @par        Revision
    $Id$
    @par        Copyright (C)
    2021-20xx Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/08/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from RBA-40C project
******************************************************************************/

#pragma once

#ifndef _JS_UART_RC_H_
#define _JS_UART_RC_H_

#include "rc_task.h"
/***************************** Include Files *********************************/

/************************** Function Prototypes ******************************/

extern u8 is_uart0_active;
extern u8 uart_send_rc(RC_TX_BUFFER *txbuf);
extern u8 uart_send_rc_encode(RC_TX_ENCRYPTION_BUFFER *txbuf_enc);
extern u8 uart_listen_rc(RC_RX_BUFFER *rxbuf);
extern void uart_init_rc(void);
extern u8 uart_rc_is_txd_active(void);
extern u8 UART_change_baudrate(u32 baudrate);

#endif /*_JS_UART_RC_H_*/
