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

#define	RFID_2K_ALL_BLOCK_LENGTH		(250)		/* 250 x 8 = 2000 byte */
#define	RFID_2K_MAX_READ_BLOCK_LENGTH	(2)
#define	RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH	(250)
#define	RFID_2K_MAX_WRITE_BLOCK_LENGTH	(1)

// 2k

void rfid_2k_read_sblock_proc(u8 block_start);
void rfid_2k_read_mblock_proc(u8 block_start, u8 count);
void rfid_2k_fast_read_sblock_proc(u8 block_start);
void rfid_2k_fast_read_mblock_proc(u8 block_start, u8 count);
void rfid_2k_read_mblock_unlimited_proc(u8 block_start, u8 count);
void rfid_2k_fast_read_mblock_unlimited_proc(u8 block_start, u8 count);
void rfid_2k_write_sblock_proc(void);
void rfid_2k_fast_write_sblock_proc(void);

u8 rfid_2k_read_sblock_receive_data_proc(u8 block_start);
u8 rfid_2k_read_mblock_receive_data_proc(u8 block_start, u8 count);
u8 rfid_2k_read_mblock_receive_data_proc(u8 block_start, u8 length);

/* EOF */

