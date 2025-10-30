/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_rfid.h
    @brief      RFID(UART) control
    @date       2018/02/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/
#pragma once

#define RFID_CARRIER_OFF (0x00)
#define RFID_CARRIER_ON  (0x01)

#define	RFID_HEAD_BLOCK				(0)
#define	RFID_ALL_BLOCK_LENGTH		(58)		/* 58 x 4 = 232 byte */
//#define	RFID_MAX_READ_BLOCK_LENGTH	(16)
#define	RFID_MAX_READ_BLOCK_LENGTH	(64)
#define	RFID_MAX_WRITE_BLOCK_LENGTH	(1)

void _intr_rfid_tready(void );
void bcccal(u8* data,u16 len, u8 *bcc);
void _intr_rfid_receive(void );
void _pl_intr_rfid_txd(void);
void _pl_intr_rfid_rxd(void);
void __pl_rfid_reset_on_proc(void);
void __pl_rfid_reset_off_proc(void);
void __pl_rfid_baudrate_proc(u8 highspeed);
void rfid_tx_data_receive_proc(u16 length);
void rfid_tx_data_send_proc(u16 length);
void rfid_read_ver_proc(void);
void rfid_rf_control_proc(u8 carrier);
void rfid_baudrate_proc(u8 baudrate);
void rfid_inventory_proc(void);
void rfid_get_system_information_proc(void);
void rfid_read_sblock_proc(u8 block_start);
void rfid_read_mblock_proc(u8 block_start, u8 count);
//void rfid_write_sblock_proc(void);
void _pl_rfid_init(void);
void _pl_rfid_txd_enable(void);
void _pl_rfid_txd_disable(void);
void _pl_rfid_rxd_enable(void);
void _pl_rfid_rxd_disable(void);
void _pl_rfid_final(void);

// fast
void rfid_fast_inventory_proc(void);
void rfid_fast_read_mblock_proc(u8 block_start, u8 count);

u8 rfid_get_system_information_receive_data_proc(void);
u8 rfid_read_ver_receive_data_proc(void);
u8 rfid_inventory_receive_data_proc(void);
//u8 rfid_read_sblock_receive_data_proc(u8 block_start);
//u8 rfid_read_mblock_receive_data_proc(u8 block_start, u8 count);
u8 rfid_write_sblock_receive_data_proc(void);
void rfid_read_sblock_receive_err_response_proc(void);
void rfid_read_mblock_receive_err_response_proc(void);

/* EOF */

