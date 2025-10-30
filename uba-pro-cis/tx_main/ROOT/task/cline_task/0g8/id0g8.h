/******************************************************************************/
/*! @addtogroup Group1
    @file       id0g8.h
    @brief      id0g8 header file
    @date       
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

#ifndef _SRC_ID0G8_H_
#define _SRC_ID0G8_H_

enum _ID0G8_MODE
{
	ID0G8_MODE_POWERUP = 0x00,
	ID0G8_MODE_POWERUP_BILLIN_AT,
	ID0G8_MODE_POWERUP_BILLIN_SK,
	ID0G8_MODE_POWERUP_ERROR,
	ID0G8_MODE_POWERUP_INITIAL,
	ID0G8_MODE_POWERUP_INITIAL_REJECT,
	ID0G8_MODE_POWERUP_INITIAL_PAUSE,
	ID0G8_MODE_POWERUP_INITIAL_STACK,				//ivizion2で新規追加

	ID0G8_MODE_INITIAL,
	ID0G8_MODE_INITIAL_STACK,
	ID0G8_MODE_INITIAL_REJECT,
	ID0G8_MODE_INITIAL_PAUSE,
	ID0G8_MODE_AUTO_RECOVERY,
	ID0G8_MODE_AUTO_RECOVERY_STACK,
	ID0G8_MODE_AUTO_RECOVERY_REJECT,
	ID0G8_MODE_AUTO_RECOVERY_PAUSE,

	ID0G8_MODE_DISABLE,
	ID0G8_MODE_ENABLE,

	ID0G8_MODE_DISABLE_REJECT = 30, //現状存在しないはず(AcceptedのAck待ちで取り込んでいない為)
	ID0G8_MODE_ENABLE_REJECT, //現状存在しないはず(AcceptedのAck待ちで取り込んでいない為)

	ID0G8_MODE_ACCEPT,
	ID0G8_MODE_ESCROW_WAIT_ACK,

	ID0G8_MODE_ESCROW,								/* Ack受信済み*/

	ID0G8_MODE_STACK,
	ID0G8_MODE_PAUSE,
	//ID0G8_MODE_VEND,								//UBA500のVendをID0G8_MODE_WAIT_ACCEPTED_ACKにした為、これはなくなった/* status(0x1B) : ID003_STS_VEND_VALID *//* 24	*/

	ID0G8_MODE_REJECT_WAIT_ACCEPT_RSP,
	ID0G8_MODE_REJECT,
	ID0G8_MODE_RETURN,
	
	ID0G8_MODE_WAIT_ACCEPTED_ACK = 50,
	ID0G8_MODE_WAIT_REMOVED_ACK,
	ID0G8_MODE_ERROR,

	ID0G8_MODE_SYSTEM_ERROR, //2022-08-17 003にあわせてとりあえず入れる

	ID0G8_MODE_END

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
/*					id0g8 Command Code							*/
/*--------------------------------------------------------------*/
//#define ID0G8_CMD_DFU_DETACH						((u8)0x00)
#define ID0G8_CMD_ACK								((u8)0x01)			// TID Event ackowlegement
#define ID0G8_CMD_ENABLE							((u8)0x02)
#define ID0G8_CMD_DISABLE							((u8)0x03)
#define ID0G8_CMD_SELF_TEST							((u8)0x04)
#define ID0G8_CMD_GAT_REPORT						((u8)0x05)
#define ID0G8_CMD_CALC_CRC							((u8)0x08)
#define ID0G8_CMD_NUM_OF_NOTE_DATA					((u8)0x80)
#define ID0G8_CMD_READ_NOTE_DATA_TABLE				((u8)0x81)
#define ID0G8_CMD_EXTEND_TIMEOUT					((u8)0x82)
#define ID0G8_CMD_ACCEPT_NOTE_TICKET				((u8)0x83)
#define ID0G8_CMD_RETURN_NOTE_TICKET				((u8)0x84)

/*--------------------------------------------------------------*/
/*			id0g8 Event Code	Accepter -> Controler			*/
/*--------------------------------------------------------------*/
#define ID0G8_EVT_POWER_STATUS						((u8)0x06)
#define ID0G8_EVT_GAT_DATA							((u8)0x07)
#define ID0G8_EVT_CRC_DATA							((u8)0x09)
#define ID0G8_EVT_DEVICE_STATE						((u8)0x0A)
#define ID0G8_EVT_NUM_OF_NOTE_DATA					((u8)0x80)
#define ID0G8_EVT_READ_NOTE_DATA_TABLE				((u8)0x81)
#define ID0G8_EVT_FAIL_STATUS						((u8)0x85)
#define ID0G8_EVT_NOTE_VALIDATED					((u8)0x86)			// TID Event
#define ID0G8_EVT_TICKET_VALIDATED					((u8)0x87)			// TID Event
#define ID0G8_EVT_NOTE_TICKET_STATUS				((u8)0x88)			// TID Event
#define ID0G8_EVT_STACKER_STATUS					((u8)0x89)			// TID Event





/*--------------------------------------------------------------*/
/*					Function*/
/*--------------------------------------------------------------*/
extern u8 id0g8_main(void);
extern void uart_init_id0g8(void);
extern u8   uart_get_bufstatus_id0g8(void);






#endif
/*--- End of File ---*/
