/******************************************************************************/
/*! @addtogroup Group1
    @file       js_uart_id003.h
    @brief      uart id003 fanctions header
    @date       2021/08/05
    @author     H.Suzuki
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

#ifndef _JS_UART_ID003_H_
#define _JS_UART_ID003_H_

/***************************** Include Files *********************************/
#include "id003.h"
/************************** Function Prototypes ******************************/

//extern u8 uart_send_id003(u8 *txbuf, u8 txlen);
//extern u8 uart_listen_id003(u8 *rxbuf, u8 buflen, u8 *size);
extern u8 uart_send_id003(ID003_CMD_BUFFER *txbuf);
extern u8 uart_listen_id003(ID003_CMD_BUFFER *rxbuf);
extern void uart_init_id003(void);
extern u8 UART_is_txd_active(void);
extern u8 UART_change_baudrate(uint32_t baudrate);
extern void interface_disable_id003(void);

#endif
