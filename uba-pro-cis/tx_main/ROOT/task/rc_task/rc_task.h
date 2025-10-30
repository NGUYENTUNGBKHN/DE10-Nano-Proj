/******************************************************************************/
/*! @addtogroup Group1
    @file       rc_task.h
    @brief      rc task header file
    @date       2018/01/15
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2013/03/21 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#pragma once
#if defined(UBA_RTQ)		/* '18-05-01 */

extern T_MSG_BASIC rc_msg;

#define REWAIT_RDY_OFF		0
#define REWAIT_RDY_ON		1
#define REWAIT_RDY_WAIT		2

enum _RC_SEQ
{
	RC_SEQ_CPU_RESET		= 0x5000,
	RC_SEQ_IDLE				= 0x5100,
	RC_SEQ_WAIT_RESPONSE	= 0x5101,
	RC_SEQ_DL_IDLE			= 0x5200,
	RC_SEQ_DL_WAIT_RESPONSE = 0x5201,
	RC_SEQ_REWAIT_READY		= 0x5300,
};

enum _RC_REWAIT_READY_SEQ
{
	RC_RDYSEQ_IDLE				= 0x00,
	RC_RDYSEQ_WAIT_READY		= 0x01,
	RC_RDYSEQ_CONFIRM_READY		= 0x02,
	RC_RDYSEQ_ENCRYPTION_KEY	= 0x03,
	RC_RDYSEQ_ENCRYPTION_NUM	= 0x04,
	RC_RDYSEQ_DL_IDLE			= 0x05,
	RC_RDYSEQ_DL_WAIT_RESPONSE	= 0x06,
	RC_RDYSEQ_REWAIT_READY		= 0x07,
};

enum _RC_LISTEN
{
	RC_LISTEN_ACK = 0x00,
    RC_LISTEN_BUSY,
    RC_LISTEN_DL_READY,
    RC_LISTEN_INVALID,
	RC_LISTEN_CHECKSUM_ERROR,
	RC_LISTEN_ERROR,	//not use UBA500
	RC_LISTEN_TMOUT = 0xFF,
};
#define RC_SYNC								0x24	/* 送信開始コード */

/* recv_cb_func()->stat */
#define	UART_STAT_RECV			0x1
#define	UART_STAT_TRANSRDY		0x2
#define	UART_STAT_TMO			0x4
#define	UART_STAT_PARITY		0x8
#define	UART_STAT_TRANEMPTY		0x10

/* command to rc */
#define CMD_RC_DEL_REQ						0x10
#define CMD_RC_STATUS_REQ					0x11

#define CMD_RC_RESET_REQ					0x40
#define CMD_RC_RESET_SKIP_REQ				0x41
#define CMD_RC_CANCEL_REQ					0x45
#define CMD_RC_REJECT_REQ					0x46
#define CMD_RC_PAUSE_REQ					0x47
#define CMD_RC_PAYOUT_REQ					0x48
#define CMD_RC_STACK_REQ					0x49
#define CMD_RC_COLLECT_REQ					0x4A
#define	CMD_RC_SOL_REQ						0x4B
#define CMD_RC_FEED_REQ						0x4C
#define CMD_RC_FLAPPER_REQ					0x4D
#define CMD_RC_SENSOR_REQ					0x4E
#define CMD_RC_DRUM_REQ						0x4F

#define CMD_RC_DISPLAY_REQ					0x50
#define CMD_RC_WU_RESET_REQ					0x51
#define CMD_RC_SENSOR_ACTIVE_REQ			0x52
#define	CMD_RC_SENSOR_CONDITION_REQ			0x53

#define CMD_RC_WRITE_SERIAL_NO_REQ			0x60
#define CMD_RC_READ_SERIAL_NO_REQ			0x61

#define CMD_RC_WRITE_MAINTE_SERIAL_NO_REQ	0x70
#define CMD_RC_READ_MAINTE_SERIAL_NO_REQ	0x71

#define CMD_RC_DL_START_REQ					0x80
#define CMD_RC_DL_DATA_REQ					0x81
#define CMD_RC_DL_CHECK_REQ					0x82
#define CMD_RC_DL_END_REQ					0x83

#define CMD_RC_WRITE_EDITION_REQ			0x90
#define CMD_RC_READ_EDITION_REQ				0x91

#define CMD_RC_DIAG_END_REQ					0x95
#define CMD_RC_DIAG_MOT_FWD_REQ				0x96

#define CMD_RC_GET_RECYCLE_SETTING_REQ		0xA9
#define CMD_RC_RESUME_REQ					0xAB
#define CMD_RC_VERSION_REQ					0xAC
#define	CMD_RC_ERROR_CLEAR_REQ				0xAD
#define CMD_RC_SW_CLEAR_REQ					0xAE

#define CMD_RC_ERROR_DETAIL_REQ				0xB0
#define CMD_RC_GET_DIPSW_REQ				0xB1

#define CMD_RC_BOOT_VERSION_REQ				0xC0
#define	CMD_RC_ENABLE_DRUM_REQ				0xC8
#define CMD_RC_RECYCLE_SETTING_REQ			0xC9
#define	CMD_RC_SET_CURRENT_COUNT_REQ		0xCA
#define CMD_RC_SET_MOTOR_SPEED_REQ			0xCB

#define CMD_RC_RETRY_BILL_DIR_REQ			0xD0
#define CMD_RC_FEED_BOX_REQ					0xD1
#define CMD_RC_DRUM_GAP_ADJ_REQ				0xD2
#define CMD_RC_BILLBACK_DRUM_PAYOUT_REQ		0xD3
#define CMD_RC_FEED_BOX_DRUM_PAYOUT_REQ		0xD4
#define CMD_RC_BOX_SEARCH_REQ				0xD5
#define CMD_RC_PREFEED_STACK_REQ			0xD6
#define CMD_RC_LAST_FEED_CASHBOX_REQ		0xD7
#define CMD_RC_FORCE_STACK_DRUM_REQ			0xD8
#define CMD_RC_BILLBACK_REQ					0xD9
#define CMD_RC_STATE_REQ					0xDA
#define CMD_RC_MODE_REQ						0xDB

#define	CMD_RC_FRAM_READ_REQ				0xF1
#define	CMD_RC_FRAM_CHECK_REQ				0xF2

#define	CMD_RC_DRUM_TAPE_ADJ_REQ			0xF3
#define	CMD_RC_START_ADJ_REQ				0xF4
#define	CMD_RC_GET_ADJ_DATA_REQ				0xF5
#define	CMD_RC_SENS_ADJ_WRITE_FRAM_REQ		0xF6
#define	CMD_RC_SENS_ADJ_READ_FRAM_REQ		0xF7
#define	CMD_RC_PERFORM_TEST_WRITE_FRAM_REQ	0xF8
#define	CMD_RC_PERFORM_TEST_READ_FRAM_REQ	0xF9
#define	CMD_RC_ERROR_COUNT_CLEAR_REQ		0xFA

#define	CMD_RC_GET_MOTOR_SPEED_REQ			0xFC

//#if defined(RC_BOARD_GREEN)		/* '24-05-13 */
	#define	CMD_RC_GET_POS_ONOFF_REQ			0xA0
	#define	CMD_RC_SET_POS_DA_REQ				0xA1
	#define	CMD_RC_SET_POS_GAIN_REQ				0xA2
	#define	CMD_RC_GET_CONFIGURATION_REQ		0xA8
	#define	CMD_RS_FLAPPER_REQ					0xE0
	#define	CMD_RS_DISPLAY_REQ					0xE1
//#endif

//#if defined(UBA_RTQ_ICB)
#define	CMD_RC_RFID_TEST_REQ				0xE6
#define	CMD_RC_RFID_READ_REQ				0xE7
#define	CMD_RC_RFID_WRITE_REQ				0xE8
#define	CMD_RC_RFID_RESET_REQ				0xE9
//#endif

/* response from rc */
#define CMD_RC_DEL_RSP						0x10
#define CMD_RC_STATUS_RSP					0x11

#define CMD_RC_RESET_RSP					0x40
#define CMD_RC_RESET_SKIP_RSP				0x41
#define CMD_RC_CANCEL_RSP					0x45
#define CMD_RC_REJECT_RSP					0x46
#define CMD_RC_PAUSE_RSP					0x47
#define CMD_RC_PAYOUT_RSP					0x48
#define CMD_RC_STACK_RSP					0x49
#define CMD_RC_COLLECT_RSP					0x4A
#define	CMD_RC_SOL_RSP						0x4B
#define CMD_RC_FEED_RSP						0x4C
#define CMD_RC_FLAPPER_RSP					0x4D
#define CMD_RC_SENSOR_RSP					0x4E
#define CMD_RC_DRUM_RSP						0x4F

#define CMD_RC_DISPLAY_RSP					0x50
#define CMD_RC_WU_RESET_RSP					0x51
#define CMD_RC_SENSOR_ACTIVE_RSP			0x52
#define	CMD_RC_SENSOR_CONDITION_RSP			0x53

#define CMD_RC_WRITE_SERIAL_NO_RSP			0x60
#define CMD_RC_READ_SERIAL_NO_RSP			0x61

#define CMD_RC_WRITE_MAINTENANCE_SERIAL_NO_RSP	0x70
#define CMD_RC_READ_MAINTENANCE_SERIAL_NO_RSP	0x71

#define CMD_RC_DL_START_RSP					0x80
#define CMD_RC_DL_DATA_RSP					0x81
#define CMD_RC_DL_CHECK_RSP					0x82
#define CMD_RC_DL_END_RSP					0x83
#define CMD_RC_WRITE_EDITION_RSP			0x90
#define CMD_RC_READ_EDITION_RSP				0x91
#define CMD_RC_DIAG_END_RSP					0x95
#define CMD_RC_DIAG_MOT_FWD_RSP				0x96
#define CMD_RC_GET_RECYCLE_SETTING_RSP		0xA9
#define CMD_RC_RESUME_RSP					0xAB
#define CMD_RC_VERSION_RSP					0xAC
#define	CMD_RC_ERROR_CLEAR_RSP				0xAD
#define CMD_RC_SW_CLEAR_RSP					0xAE

#define CMD_RC_ERROR_DETAIL_RSP				0xB0
#define CMD_RC_GET_DIPSW_RSP				0xB1

#define CMD_RC_BOOT_VERSION_RSP				0xC0
#define	CMD_RC_ENABLE_DRUM_RSP				0xC8
#define CMD_RC_RECYCLE_SETTING_RSP			0xC9
#define	CMD_RC_SET_CURRENT_COUNT_RSP		0xCA
#define CMD_RC_SET_MOTOR_SPEED_RSP			0xCB

#define CMD_RC_RETRY_BILL_DIR_RSP			0xD0
#define CMD_RC_FEED_BOX_RSP					0xD1
#define CMD_RC_DRUM_GAP_ADJ_RSP				0xD2
#define CMD_RC_BILLBACK_DRUM_PAYOUT_RSP		0xD3
#define CMD_RC_FEED_BOX_DRUM_PAYOUT_RSP		0xD4

#define CMD_RC_SEARCH_RSP					0xD5
#define CMD_RC_SEARCH_PRM_OUT_DRUM			0x00
#define CMD_RC_SEARCH_PRM_IN_DRUM			0x01

#define CMD_RC_PREFEED_STACK_RSP			0xD6
#define CMD_RC_LAST_FEED_CASHBOX_RSP		0xD7
#define CMD_RC_FORCE_STACK_DRUM_RSP			0xD8
#define CMD_RC_BILLBACK_RSP					0xD9
#define CMD_RC_STATE_RSP					0xDA
#define CMD_RC_MODE_RSP						0xDB

#define	CMD_RC_FRAM_READ_RSP				0xF1
#define	CMD_RC_FRAM_CHECK_RSP				0xF2

#define	CMD_RC_DRUM_TAPE_ADJ_RSP			0xF3
#define	CMD_RC_START_ADJ_RSP				0xF4
#define	CMD_RC_GET_ADJ_DATA_RSP				0xF5
#define	CMD_RC_SENS_ADJ_WRITE_FRAM_RSP		0xF6
#define	CMD_RC_SENS_ADJ_READ_FRAM_RSP		0xF7
#define	CMD_RC_PERFORM_TEST_WRITE_FRAM_RSP	0xF8
#define	CMD_RC_PERFORM_TEST_READ_FRAM_RSP	0xF9
#define	CMD_RC_ERROR_COUNT_CLEAR_RSP		0xFA

#define	CMD_RC_GET_MOTOR_SPEED_RSP			0xFC

//#if defined(RC_BOARD_GREEN)		/* '24-05-13 */
	#define	CMD_RC_GET_POS_ONOFF_RSP			0xA0
	#define	CMD_RC_SET_POS_DA_RSP				0xA1
	#define	CMD_RC_SET_POS_GAIN_RSP				0xA2
	#define	CMD_RC_GET_CONFIGURATION_RSP		0xA8
	#define	CMD_RS_FLAPPER_RSP					0xE0
	#define	CMD_RS_DISPLAY_RSP					0xE1
//#endif

//#if defined(UBA_RTQ_ICB)
	#define	CMD_RC_RFID_TEST_RSP				0xE6
	#define	CMD_RC_RFID_READ_RSP				0xE7
	#define	CMD_RC_RFID_WRITE_RSP				0xE8
	#define	CMD_RC_RFID_RESET_RSP				0xE9
//#endif

#define	CMD_RC_ENC_KEY						0x01					/* Encryption Key			*/
#define	CMD_RC_ENC_NUM						0x02					/* Encryption Number		*/


#define		RC_DOWNLOAD_DATA_START			0x00E00000											/* Recycler program area start address		*/
#define		RC_DOWNLOAD_DATA_SIZE			0xF0000L											/* Recycler program data size				*/
#define		RC_DOWNLOAD_DATA_END			(RC_DOWNLOAD_DATA_START + RC_DOWNLOAD_DATA_SIZE)	/* Recycler program area end address		*/
#define		RC_DOWNLOAD_DATA_SUM			(RC_DOWNLOAD_DATA_END - 2L)							/* Recycler program area checksum address	*/
#define		RC_DOWNLOAD_DATA_ROMID			(RC_DOWNLOAD_DATA_START + 0xB000L)					/* Recycler program RomID address			*/
#define		RC_DOWNLOAD_DATA_BLOCK			0x50L												/* Recycler program block size of download	*/

#endif // UBA_RTQ
