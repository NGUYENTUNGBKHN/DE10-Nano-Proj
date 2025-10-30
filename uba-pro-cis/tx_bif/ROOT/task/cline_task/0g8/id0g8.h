/******************************************************************************/
/*! @addtogroup Group1
    @file       id003.h
    @brief      id003 header file
    @date       2013/03/21
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

enum _ID0G8_MODE
{
	ID0G8_MODE_WAIT_REQ = 1,
	ID0G8_MODE_WAIT_DOWNLOAD_START_RSP,
	ID0G8_MODE_DOWNLOAD,
	ID0G8_MODE_DOWNLOAD_BUSY,
	ID0G8_MODE_DOWNLOAD_CALC_CRC,
	ID0G8_MODE_DOWNLOAD_COMPLETE,
	ID0G8_MODE_DOWNLOAD_COMPLETE_ERROR,
	ID0G8_MODE_DOWNLOAD_ERROR,
	ID0G8_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR,
	ID0G8_MODE_DOWNLOAD_WAIT_RESET,
	ID0G8_MODE_QUIT
};


#define ID0G8_SYNC						0xFC	/* 送信開始コード */
#define ID0G8_SYNC_GLOBAL				0xF0


typedef struct _ID0G8_CMD_BUFFER
{
	u8	sync;
	u8	length;			/* size */
	u8	cmd;			/* command */
	u8	data[250];		/* addition data */
	u8	crc1;
	u8	crc2;
} ID0G8_CMD_BUFFER;

enum _ID0G8_LISTEN
{
	ID0G8_LISTEN_TMOUT = 0,
	ID0G8_LISTEN_COMMAND,
	ID0G8_LISTEN_CRC_ERROR,
	ID0G8_LISTEN_LENGTH_ERROR,
};

/*--------------------------------------------------------------*/
/*					id003 Command Code							*/
/*--------------------------------------------------------------*/
#define	ID0G8_CMD_ENQ					0x05	/* ENQ */
#define	ID0G8_CMD_STS_REQUEST			0x11	/* ステータスリクエスト */
#define	ID0G8_CMD_ACK					0x50	/* ACK */
#define	ID0G8_CMD_DOWNLOAD_REQUEST		0xD0	/* ダウンロード開始リクエスト         */
#define	ID0G8_CMD_DOWNLOAD_DATA			0xD1	/* ダウンロードデータコマンド         */
#define	ID0G8_CMD_DOWNLOAD_END_REQUEST	0xD2	/* ダウンロード終了リクエスト         */
#define	ID0G8_CMD_BAUDRATE_REQUEST		0xD5	/* Daudrateリクエスト              */
#define	ID0G8_CMD_DIFFERENTIAL_DOWNLOAD	0xD6	/* Differentialダウンロードリクエスト */
/* <<<<< Extension Command >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*--------------------------------------------------------------*/
/*					id003 Status Code							*/
/*--------------------------------------------------------------*/
#define	ID0G8_STS_FAILURE				0x49	/* 故障、異常 */
#define	ID0G8_STS_COMMUNICATION_ERROR	0x4A	/* 通信異常 */
#define ID0G8_STS_DOWNLOAD_END			0xD3	/* ダウンロード終了 */
#define ID0G8_STS_DOWNLOAD				0xD4	/* ダウンロード中状態 */

/* <<<<< Extension Status >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
#define	ID0G8_STS_EXTENSION				0xFF	/* 拡張ステータス */
/* <<<<< Extension Status >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */


/*--------------------------------------------------------------*/
/*					id003 Failure Code							*/
/*--------------------------------------------------------------*/
#define	ID0G8_FAILURE_STACKER_MOTOR					0xA2	/* 収納モータ異常 */
#define	ID0G8_FAILURE_FEED_MOTOR_SPEED				0xA5	/* 搬送モータスピード異常 */
#define	ID0G8_FAILURE_FEED_MOTOR					0xA6	/* 搬送モータ異常 */
#define	ID0G8_FAILURE_SOLENOID						0xA8	/* Solenoid Failure */
#define	ID0G8_FAILURE_APB_UNIT						0xA9	/* APB Unit Failure */
#define	ID0G8_FAILURE_BOX_READY						0xAB	/* Cash box not ready */
#define	ID0G8_FAILURE_HEAD							0xAF	/* Validator head remove */
#define	ID0G8_FAILURE_BOOT_ROM						0xB0	/* Boot ROM異常 */
#define	ID0G8_FAILURE_EXT_ROM						0xB1	/* 外部ROM異常 */
#define	ID0G8_FAILURE_RAM							0xB2	/* RAM異常 */
#define	ID0G8_FAILURE_EXT_ROM_WRITE					0xB3	/* External ROM writing failure */

#define ID0G8_FAILURE_CENTERING						0xC1	/* Width brings system error for EBA-30 */
#define ID0G8_FAILURE_STACKER_CONNECT				0xC3	/* Width brings system error for EBA-30 */


/*--------------------------------------------------------------*/
/*					id003 Download Status Code					*/
/*--------------------------------------------------------------*/
#define ID0G8_DOWNLOAD_STS_READY					0x00
#define ID0G8_DOWNLOAD_STS_READY_DIFFERENTIAL		0x01
#define ID0G8_DOWNLOAD_STS_BUSY						0x80
/*--------------------------------------------------------------*/
/*					Function*/
/*--------------------------------------------------------------*/
extern u8 id003_main(void);


//extern u8   uart_send_id003(u8 *txbuf, u8 buflen);
extern u8   uart_send_plain_id003(u8 *txbuf, u8 buflen);
//extern u8   uart_listen_id003(u8 *rxbuf, u8 buflen, u8 *size);
extern void uart_init_id003(void);
extern u8   uart_get_bufstatus_id003(void);
extern void uart_txd_stall_id003(void);
extern void uart_txd_active_id003(void);


/*--------------------------------------------------------------*/
/*		For Blue Wave DX										*/
/*--------------------------------------------------------------*/
extern	bool	condition_denomi_inhibit_id003(u32 denomi_code, u32 direction);


/*--- End of File ---*/
