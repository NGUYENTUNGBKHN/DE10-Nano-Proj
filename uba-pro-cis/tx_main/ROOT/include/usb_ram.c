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
#include "debug.h"
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
enum _DLINE_MODE
{
	/* Normal Operation(I/F) Mode */
	DLINE_MODE_NORMAL = 1,
	/* Performance Test Mode */
	DLINE_MODE_TEST_STANDBY,
	DLINE_MODE_TEST_EXEC,
	DLINE_MODE_TEST_ERROR,
	DLINE_MODE_ATEST_INITIAL,
	DLINE_MODE_ATEST_ENABLE,
	DLINE_MODE_ATEST_ACCEPT,
	DLINE_MODE_ATEST_STACK,
	DLINE_MODE_ATEST_REJECT,
	DLINE_MODE_ATEST_ERROR,
	DLINE_MODE_PTEST_PAYOUT,
	DLINE_MODE_PTEST_COLLECT,
};

enum DLINE_PROGRAMID {
	DLINE_SLEEP = 0,
	DLINE_INITIALIZE,
	DLINE_ANALYZE,
	DLINE_ANALYZE_SUITE,
	DLINE_RXD_WAITING
};
/*< 	Definition of "pc.mode" memory 		>*/
#define	BIT_FUSB_MODE							0xffff
#define	BIT_FUSB_MODE_TESTMODE_REQUEST			0x8000
#define	BIT_FUSB_MODE_BILL_DATA_REQUEST			0x4000
#define BIT_FUSB_MODE_ADJUSTMENT_REQUEST		0x2000
#define BIT_FUSB_MODE_ACCLOAD_REQUEST			0x1000
#define BIT_FUSB_MODE_ICB_FUNCTION_REQUEST		0x0800
#define BIT_FUSB_MODE_NJ_FUNCTION_REQUEST		0x0400
#define BIT_FUSB_MODE_DATA_COLLECTION_REQUEST	0x0200
#define BIT_FUSB_MODE_SETTING_REQUEST			0x0100
#define	BIT_FUSB_MODE_EVENTLOG_REQUEST			0x0080
#define	BIT_FUSB_MODE_TRACE_REQUEST				0x0040
#define	BIT_FUSB_MODE_CONDITION_REQUEST			0x0020
#define	BIT_FUSB_MODE_COMMLOG_REQUEST			0x0010
#define	BIT_FUSB_MODE_JDL_REQUEST				0x0008
#define	BIT_FUSB_MODE_FPGA_EVENTLOG_REQUEST		0x0004
#define BIT_FUSB_MODE_AUTHENTICATION_REQUEST	0x0002
#if defined(UBA_RTQ)
#define BIT_FUSB_MODE_RECYCLE_REQUSET			0x0001
#endif // UBA_RTQ
/* #define								0x0001	*/

/*< 	Definition of "pc.status" memory 		>*/
#define	BIT_FUSB_USE_DATA_BUFFER_BUSY	0x8000
#define	BIT_FUSB_USE_DATA_BUFFER		0x4000
#define	BIT_FUSB_02BOARD_GAIN_REQ		0x2000
#define	BIT_FUSB_02BOARD_DA_REQ			0x1000
#define	BIT_FUSB_CIS_DATA_SENDING		0x0800
#define	BIT_FUSB_WAVE_DATA_SENDING		0x0400
#define	BIT_FUSB_CALUC_DATA_SENDING		0x0200
#define	BIT_FUSB_ACCLOAD_DATA_SENDING	0x0100
/* #define								0x0080	*/
/* #define								0x0040	*/
/* #define								0x0020	*/
/* #define								0x0010	*/
#define	BIT_FUSB_COMMAND_HOLD			0x0008
#define	BIT_FUSB_COMMAND_WAITING		0x0004
#define	BIT_FUSB_BIG_DATA_LAST_PACKET	0x0002
#define	BIT_FUSB_BIG_DATA_SEND_AFTER	0x0001
#define	BIT_FUSB_BIG_DATA_SENDING		(BIT_FUSB_CIS_DATA_SENDING | BIT_FUSB_WAVE_DATA_SENDING)
/*!
 Front USB device flg
 */
extern struct _front_usb ex_front_usb;

/*<<	Service ID			>>*/
enum USB_SERVICE_ID {
	FUSB_SERVICE_ID_DOWNLOAD = 0x01,
	FUSB_SERVICE_ID_ACCLOAD = 0x02,
	FUSB_SERVICE_ID_TESTMODE = 0x03,
	FUSB_SERVICE_ID_UTILITY_MODE = 0x04,
	FUSB_SERVICE_ID_EVENTLOG = 0x07,
	FUSB_SERVICE_ID_CONDITION = 0x0A,
	FUSB_SERVICE_ID_TRACE = 0x0D,
	FUSB_SERVICE_ID_COMMLOG = 0x0E,
	FUSB_SERVICE_ID_DATA_COLLECTION = 0xfc,
	FUSB_SERVICE_ID_ALL_MODE = 0xff
};

/*<<	Phase Number(ACCLOAD)		>>*/
#define	FUSB_PHASE_ACCLOAD				0xd1

/*<<	Phase Number(IF Function)		>>*/
#define	FUSB_PHASE_ID024_CRC_ALGORITHM	0x11

/*<<	COMMAND				>>*/
#define	CMD_DATA_RECEIVED				0x01
#define	CMD_CALCULATE_RUN				0x02
#define FUSB_CMD_HASH_START				0x01
#define FUSB_CMD_HASH_BUSY				0xFF
#define FUSB_CMD_HASH_END				0x00

/*<<	COMMAND(Image)		>>*/
#define	FUSB_CMD_IMAGE_RECEIVED_DATA	0x02
#define	FUSB_CMD_IMAGE_DISTORTION		0x03

/*<<	Service ID			>>*/
#define	FUSB_SERVICE_ID_DOWNLOAD				0x01
#define	FUSB_SERVICE_ID_ACCLOAD					0x02
#define	FUSB_SERVICE_ID_TESTMODE				0x03
#define	FUSB_SERVICE_ID_UTILITY_MODE			0x04
#define	FUSB_SERVICE_ID_EVENTLOG				0x07
#define	FUSB_SERVICE_ID_CONDITION				0x0A
#define	FUSB_SERVICE_ID_JDL						0x0C
#define	FUSB_SERVICE_ID_DATA_COLLECTION			0xfc
#define	FUSB_SERVICE_ID_ALL_MODE				0xff

/*<<	Length			>>*/
#define	FUSB_HEADER_SIZE				0x06
enum TESTMODE_NUMBER {
	MODE_TESTMODE_REQUEST = 0x61,
	MODE_WAVE_DATA_REQUEST = 0x32,
	MODE_CALUCLATE_DATA_REQUEST,
	MODE_WAVE_DATA_TRANSACTION,
	MODE_ADJUSTMENT_SENSOR_DATA = 0x52,
	MODE_ADJUSTMENT_EEPROM_RW = 0x53,
	MODE_ADJUSTMENT_VALUE = 0x54,
	MODE_ADJUSTMENT_SAMPLING = 0x55,
	MODE_ADJUSTMENT_TEMPERATURE = 0x56,
#if defined(UBA_RTQ)
	MODE_RECYCLE_REQUEST	= 0x70,
#endif // UBA_RTQ
	MODE_ACCLOAD_REQUEST = 0x71,
	MODE_ICB_FUNCTION = 0x72,
	MODE_IF_SETTINGS = 0x74,
	MODE_DATA_COLLECTION_REQUEST = 0x81,
#if 1//
	MODE_DATA_AUTHENTICATION_REQUEST = 0x80,
#else
	MODE_DATA_AUTHENTICATION_REQUEST = 0x83,
#endif
	MODE_EVENT_LOG_REQUEST = 0xa0, /* '13-10-01 Event Log */
	MODE_TRACE_REQUEST = 0xa1,
	MODE_COMMLOG_REQUEST = 0xa2,
	MODE_JDL_REQUEST = 0xa3,
	MODE_FPGA_EVENT_LOG_REQUEST = 0xa4,
	MODE_CONDITION_ENABLE_DENOMI_REQUEST = 0x91,
	MODE_CONDITION_ERROR_CODE_REQUEST = 0x92,
	MODE_CONDITION_MAINTENANCE_REQUEST = 0x93,
	MODE_END
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

#if defined(USB_REAR_USE)
EXTERN u8 ex_rear_usb_write_buffer[USB_BUFFER_SIZE];
EXTERN u8 ex_rear_usb_read_buffer[USB_BUFFER_SIZE];
EXTERN u32 ex_rear_usb_write_size;
EXTERN int ex_rear_usb_read_size;
EXTERN u8 *ex_rear_usb_pbuf;
EXTERN u8 ex_rear_usb_buf_change;
EXTERN u8 ex_rear_usb_write_busy;
#endif

EXTERN u8 ex_operation_usb_write_buffer[USB_BUFFER_SIZE];
EXTERN u8 ex_operation_usb_read_buffer[USB_BUFFER_SIZE];
EXTERN u32 ex_operation_usb_write_size;
EXTERN int ex_operation_usb_read_size;

#endif
