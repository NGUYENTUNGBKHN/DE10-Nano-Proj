/******************************************************************************/
/*! @addtogroup BOOT
    @file       boot com_ram.c
    @brief      common variable
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

#ifndef SRC_BOOT_INCLUDE_USB_RAM_H_
#define SRC_BOOT_INCLUDE_USB_RAM_H_

// GRUSB
#include "grp_cyclonev_tg.h"
#include "grcomm.h"
#include "comm_def.h"
#include "perid.h"

#include "usb_cdc_buffer.h"

#if defined(EXT)
	#define EXTERN extern
#else
	#undef EXTERN
	#define EXTERN
#endif

enum DLINE_STATE {
	DLINE_NO_DATA = 0,
	DLINE_RECEIVE_OK,
	DLINE_RECEIVE_COMPLETE,
	DLINE_PACKET_ERR = 0xFF,
};

#define USB_BUFFER_SIZE 			0x8000//0xFFFF

EXTERN T_FRONT_USB ex_front_usb;
EXTERN T_FRONT_USB ex_operation_usb;
EXTERN DLINE_PKT_INFO ex_dline_pkt;
EXTERN T_SUITE_ITEM ex_subline_suite_item;
EXTERN DLINE_PKT_INFO ex_subline_pkt;
EXTERN USB_COMMAND_SEQUENCE ex_operation_usb_command;
EXTERN USB_COMMAND_SEQUENCE ex_usb_command;

EXTERN u8 ex_usb_write_buffer[USB_BUFFER_SIZE];
EXTERN u8 ex_usb_read_buffer[USB_BUFFER_SIZE];
EXTERN u32 ex_usb_write_size;
EXTERN int ex_usb_read_size;
EXTERN u8 *ex_usb_pbuf;
EXTERN u8 ex_usb_buf_change;
EXTERN u8 ex_usb_write_busy;
/* 変数定義 */

EXTERN u8 ex_rear_usb_write_buffer[USB_BUFFER_SIZE];
EXTERN u8 ex_rear_usb_read_buffer[USB_BUFFER_SIZE];
EXTERN u32 ex_rear_usb_write_size;
EXTERN int ex_rear_usb_read_size;
EXTERN u8 *ex_rear_usb_pbuf;
EXTERN u8 ex_rear_usb_buf_change;
EXTERN u8 ex_rear_usb_write_busy;

EXTERN u8 ex_operation_usb_write_buffer[USB_BUFFER_SIZE];
EXTERN u8 ex_operation_usb_read_buffer[USB_BUFFER_SIZE];
EXTERN u32 ex_operation_usb_write_size;
EXTERN int ex_operation_usb_read_size;

#endif
