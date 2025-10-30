/******************************************************************************/
/*! @addtogroup Main
    @file       dline_condition.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_condition.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */
// TODO:デバイスコンディションプロトコルスタック実装ファイル。
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "cyc.h"
#include "motor_ctrl.h"
#include "dline_jdl.h"
#include "hal.h"
#include "../jdl/include/jdl.h"
#include "sub_functions.h"
#include "task/cline_task/003/id003.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

#include "dline_condition.h"

/*===== JCM Device Log (JDL) definition =====*/
typedef struct _currency_information{
		u8 country_code;
		u8 denomi_base;
		u8 denomi_exp;
		u8 info0;
		u8 info1;
		u8 info2;
} currency_infomation;
extern const currency_infomation currency_info[DENOMI_SIZE];

enum	CONDITION_EEPROM_CMD_NUMBER
{
	CMD_START	= 0x01,
};

enum	CONDITION_RES_NUMBER
{
	RES_OK		= 0x00,
	RES_END		= 0xFF,
};

enum TESTMODE_CMD_NUMBER
{
	CMD_NONE				= 0x00,
	CMD_RUN					= 0x01,
	CMD_ENQ					= 0x05,
	CMD_POLL				= 0x11,
	CMD_STOP				= 0xff
};

enum TESTMODE_RES_NUMBER
{
	RES_DATA	= 0x00,
	RES_NG		= 0x80,
	RES_BUSY	= 0x01,
	RES_ACK		= 0x06,
	RES_NAK		= 0x15,
};

/*--------------------------------------------------------------*/
/*					id003 Reject Code							*/
/*--------------------------------------------------------------*/
#define	ID003_REJECT_INSERTION						0x71	/* 挿入異常 */
#define	ID003_REJECT_MAG							0x72	/* マグ異常 */
#define	ID003_REJECT_REMAINING_BILL_IN_ACCEPTOR		0x73	/* 紙幣の残留(アクセプタ部) */
#define	ID003_REJECT_XRATE							0x74	/* 補正/振幅率異常 */
#define	ID003_REJECT_FEED							0x75	/* 搬送異常 */
#define	ID003_REJECT_PRECOMP						0x76	/* 金種判定異常 */
#define	ID003_REJECT_PATTERN1						0x77	/* フォトパタン異常1 */
#define	ID003_REJECT_PHOTO_LEVEL					0x78	/* フォトレベル異常 */
#define	ID003_REJECT_INHIBIT						0x79	/* INHIBIT信号 or エスクロタイムアウト */
/*#define ID003_REJECT_RESERVE						0x7A*/	/* Reserve */
#define	ID003_REJECT_OPERATION						0x7B	/* オペレーション異常 */
#define	ID003_REJECT_REMAINING_BILL_IN_STACKER		0x7C	/* 紙幣の残留(スタッカー部) */
#define	ID003_REJECT_LENGTH							0x7D	/* 長さエラー */
#define	ID003_REJECT_PATTERN2						0x7E	/* フォトパタン異常2 */
#define	ID003_REJECT_COUNTERFEIT					0x7F	/* 真券特徴異常 */


/*--------------------------------------------------------------*/
/*					id003 Barcode Reject Code					*/
/*--------------------------------------------------------------*/
#define	ID003_REJECT_BAR_NC							0x91	/* [BAR] need configuration */
#define	ID003_REJECT_BAR_UN							0x92	/* [BAR] unknown code */
#define	ID003_REJECT_BAR_SH							0x93	/* [BAR] under or over read */
#define	ID003_REJECT_BAR_ST							0x94	/* [BAR] start bit missing */
#define	ID003_REJECT_BAR_SP							0x95	/* [BAR] stop bit missing */
#define	ID003_REJECT_BAR_TP							0x96	/* [BAR] type not enable */
#define	ID003_REJECT_BAR_XR							0x97	/* [BAR] x-rate */
#define	ID003_REJECT_BAR_PHV						0x98	/* [BAR] photo level */
#define	ID003_REJECT_BAR_DIN						0x9B	/* [BAR] reverse */
#define	ID003_REJECT_BAR_LG							0x9D	/* [BAR] length out of range */


/*--------------------------------------------------------------*/
/*					id003 Failure Code							*/
/*--------------------------------------------------------------*/
#define	ID003_FAILURE_STACKER_MOTOR					0xA2	/* 収納モータ異常 */
#define	ID003_FAILURE_FEED_MOTOR_SPEED				0xA5	/* 搬送モータスピード異常 */
#define	ID003_FAILURE_FEED_MOTOR					0xA6	/* 搬送モータ異常 */
#define	ID003_FAILURE_SOLENOID						0xA8	/* Solenoid Failure */
#define	ID003_FAILURE_APB_UNIT						0xAF	/* APB Unit Failure *//* UBA10,iPRO,UBA500に合わせる A9->AF*/
#define	ID003_FAILURE_BOX_READY						0xAB	/* Cash box not ready */
#define	ID003_FAILURE_HEAD							0xAF	/* Validator head remove */
#define	ID003_FAILURE_BOOT_ROM						0xB0	/* Boot ROM異常 */
#define	ID003_FAILURE_EXT_ROM						0xB1	/* 外部ROM異常 */
#define	ID003_FAILURE_RAM							0xB2	/* RAM異常 */
#define	ID003_FAILURE_EXT_ROM_WRITE					0xB3	/* External ROM writing failure */

#define ID003_FAILURE_CENTERING						0xAF	/* Width brings system error for EBA-30 *//* UBA10,iPRO,UBA500に合わせる C1->AF*/
//#define ID003_FAILURE_STACKER_CONNECT				0xC3	/* Width brings system error for EBA-30 */
#define DATA_RESPONSE_COMPLETE_OFF	0						/* データ送信完了フラグ : OFF */
#define DATA_RESPONSE_COMPLETE_ON	1						/* データ送信完了フラグ : ON 	*/

/*--------------------------------------------------------------*/
/*					id003 Status Code							*/
/*--------------------------------------------------------------*/
#define	ID003_STS_ENABLE				0x11	/* 受取許可状態 */
#define	ID003_STS_ACCEPTING				0x12	/* 紙幣受取中 */
#define	ID003_STS_ESCROW				0x13	/* エスクロ */
#define	ID003_STS_STACKING				0x14	/* 収納中 */
#define	ID003_STS_VEND_VALID			0x15	/* 紙幣受取確定 */
#define	ID003_STS_STACKED				0x16	/* 収納終了 */
#define	ID003_STS_REJECTING				0x17	/* 受取不可紙幣の返却中 */
#define	ID003_STS_RETURNING				0x18	/* 返却中 */
#define	ID003_STS_HOLDING				0x19	/* 紙幣保持中 */
#define	ID003_STS_DISABLE				0x1A	/* 受け取り禁止 */
#define	ID003_STS_INITIALIZE			0x1B	/* 初期化中 */
#define	ID003_STS_NEARLY_FULL			0x1C	/* Nearly Full状態 */
//#define	ID003_STS_PAYING				0x20	/* 払出中 */
//#define	ID003_STS_COLLECTING			0x21	/* 回収中 */
//#define	ID003_STS_COLLECTED				0x22	/* 回収完了 */
//#define	ID003_STS_PAY_VALID				0x23	/* 払出確定 */
//#define	ID003_STS_PAY_STAY				0x24	/* 払出紙幣残留 */
//#define	ID003_STS_RETURN_TO_BOX			0x25	/* 残留紙幣回収中 */
//#define	ID003_STS_RETURN_PAY_OUT		0x26	/* 払出し異常 */
//#define	ID003_STS_RETURN_ERROR			0x2F	/* 残留紙幣回収エラー */
#define	ID003_STS_POWERUP				0x40	/* 正常な場合のパワーアップ状態 */
#define	ID003_STS_POWERUP_BILLIN_AT		0x41	/* 電源投入時に鑑別ヘッド部に紙幣が残っている状態 */
#define	ID003_STS_POWERUP_BILLIN_SK		0x42	/* 電源投入時に収納部に紙幣が残っている状態 */
#define	ID003_STS_STACKER_FULL			0x43	/* スタッカーフル */
#define	ID003_STS_STACKER_BOX_REMOVE	0x44	/* スタッカーBOX未装着 */
#define	ID003_STS_JAM_IN_AT				0x45	/* 搬送ジャム */
#define	ID003_STS_JAM_IN_SK				0x46	/* 収納ジャム */
#define	ID003_STS_PAUSE					0x47	/* 一時停止中 */
#define	ID003_STS_CHEATED				0x48	/* チート発生 */
#define	ID003_STS_FAILURE				0x49	/* 故障、異常 */
#define	ID003_STS_COMMUNICATION_ERROR	0x4A	/* 通信異常 */
#define ID003_STS_DOWNLOAD_END			0xD3	/* ダウンロード終了 */
#define ID003_STS_DOWNLOAD				0xD4	/* ダウンロード中状態 */

/* <<<<< Extension Status >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
#define	ID003_STS_EXTENSION				0xFF	/* 拡張ステータス */

#define ID003_STS_EXT_LOG_ACCESS_READY	0x06	/*  */
#define ID003_STS_EXT_LOG_ACCESS_END	0x03	/*  */
#define ID003_STS_EXT_LOG_ACCESS_BUSY	0x80	/*  */

#define	ID003_STS_EXT_DISCONNECT		0x00	/* 拡張ステータス 未接続 */
#define	ID003_STS_EXT_NORMAL			0x10	/* 拡張ステータス 動作可能状態 */
#define	ID003_STS_EXT_EMPTY				0x11	/* 拡張ステータス 紙幣なし */
#define	ID003_STS_EXT_FULL				0x12	/* 拡張ステータス 紙幣フル */
#define	ID003_STS_EXT_BUSY				0x1F	/* 拡張ステータス ビジー */
#define	ID003_STS_EXT_JAM				0x40	/* 拡張ステータス ジャム発生 */
#define	ID003_STS_EXT_DOOR_OPEN			0x41	/* 拡張ステータス 扉開 */
#define	ID003_STS_EXT_MOTOR_ERROR		0x42	/* 拡張ステータス モータエラー */
#define	ID003_STS_EXT_EEPROM_ERROR		0x43	/* 拡張ステータス EEPROMエラー */
#define	ID003_STS_EXT_OTHER_EEROR		0x4A	/* 拡張ステータス その他エラー */
#define	ID003_STS_EXT_BOXKEY_OPEN		0x4D	/* 拡張ステータス Box Key Open */
#define	ID003_STS_EXT_STACKER_LOSS		0x4E	/* 拡張ステータ スタッカーが取り外された */
#define	ID003_STS_EXT_MD100_DISPENSE	0x24	/* 拡張ステータス 払い出し */
#define	ID003_STS_EXT_MD100_DISPENSED	0x25	/* 拡張ステータス 払い出し完了 */
#define	ID003_STS_EXT_MD100_D_COMPLETE	0x26	/* 拡張ステータス 紙幣抜き取り完了 */
#define	ID003_STS_EXT_MD100_D_ABORT		0x28	/* 拡張ステータス 払い出し中止 */
#define	ID003_STS_EXT_MD100_D_NO_NOTE	0x29	/* 拡張ステータス 払い出し開始から出口センサにかかるまで */
#define	ID003_STS_EXT_MD100_D_FAILURE	0x2B	/* 拡張ステータス 払い出しエラー */
static	u8	phase_enable_denomi_read(u8 cmd);
static	u8	check_denomination_table(void);
static	u8	phase_error_code_read(u8 cmd);
static	u8	convert_reject_code(u16 reject_code);
static	u8	convert_error_code(u16 alarm_code);
static	u8	phase_maintenance_read(u8 cmd);


u8	Data_response_complete_flag;
u8	Enable_denomination_table[DENOMI];						/*	有効金種テーブル		*/
u8	Enable_denomination_index;

/************************** External functions *******************************/
extern void set_response_1data(u8 cmd);
/******************************************************************************/
/*! @brief Adjustment Request Function.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void front_usb_condition_request(void)
{
	u8 response = 0;

	switch(ex_front_usb.pc.mess.modeID)
	{
	case	MODE_CONDITION_ENABLE_DENOMI_REQUEST:
			response = phase_enable_denomi_read(ex_front_usb.pc.mess.command);
			break;
	case	MODE_CONDITION_ERROR_CODE_REQUEST:
			response = phase_error_code_read(ex_front_usb.pc.mess.command);
			break;
	case	MODE_CONDITION_MAINTENANCE_REQUEST:
			response = phase_maintenance_read(ex_front_usb.pc.mess.command);
			break;
	default:
			response = NAK;
			break;
	}

	if(response != 0)
	{
		set_response_1data(response);
	}
}


/******************************************************************************/
/*! @brief Get Conndiciton Error Code Read.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static	u8	phase_enable_denomi_read(u8 cmd)
{
	u8	response = 0;
	u8	cnt;
	u8	*write_addr;

	switch(cmd)
	{
	case	CMD_START:
			Data_response_complete_flag = DATA_RESPONSE_COMPLETE_OFF;
			Enable_denomination_index = 0;
			for(cnt = 0;cnt < DENOMI; cnt++)
			{
				Enable_denomination_table[cnt] = 0;
			}

			ex_usb_write_size = (FUSB_HEADER_SIZE);
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_ACK;
			break;
	case	CMD_ENQ:
			if(Data_response_complete_flag == DATA_RESPONSE_COMPLETE_OFF)
			{
				check_denomination_table();

				write_addr = &(*(ex_usb_write_buffer + 6));

				ex_usb_write_size = (FUSB_HEADER_SIZE);

				for(cnt = Enable_denomination_index; cnt < DENOMI; cnt++)
				{
					if(Enable_denomination_table[cnt] == 1 && currency_info[cnt].denomi_base != 0)
					{
						*write_addr++ = currency_info[cnt].country_code;
						*write_addr++ = currency_info[cnt].denomi_base;			/* 金種情報整数部の設定 */
						*write_addr++ = currency_info[cnt].denomi_exp;			/* 金種情報指数部の設定 */
						*write_addr++ = currency_info[cnt].info0;				/* 金種付加情報部バイト1の設定 */
						*write_addr++ = currency_info[cnt].info1;				/* 金種付加情報部バイト2の設定 */
						*write_addr++ = currency_info[cnt].info2;				/* 金種付加情報部バイト3の設定 */

						ex_usb_write_size += 6;
					}
				}
				if(cnt >= DENOMI)
				{
					Data_response_complete_flag = DATA_RESPONSE_COMPLETE_ON;			/* データ送信完了フラグをONに設定。 */
				}
				else
				{
					Enable_denomination_index = cnt;
				}

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = RES_OK;
			}
			else
			{
				ex_usb_write_size = (FUSB_HEADER_SIZE);
				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = RES_END;
			}
			break;
	default:
			break;
	}

	return(response);
}


static	u8	check_denomination_table(void)
{
	u8	fileno;
	u8	ret = 0;

	for(fileno = 0; fileno < DENOMI; fileno++)
	{
		switch(ex_cline_status_tbl.protocol_select)
		{
#if defined(_PROTOCOL_ENABLE_ID003) || defined(_PROTOCOL_ENABLE_ID003V) || defined(_PROTOCOL_ENABLE_ID003GD)
		case	PROTOCOL_SELECT_ID003:
			if(condition_denomi_inhibit_id003(fileno, 0) == 0)
			{
				Enable_denomination_table[fileno] = 1;	/* Enable denomi	*/
			}
			break;
#endif
#if defined(_PROTOCOL_ENABLE_ID064GD)
		case	PROTOCOL_SELECT_ID064GD:
			if(condition_denomi_inhibit_id064GD(fileno, 0) == 0)
			{
				Enable_denomination_table[fileno] = 1;	/* Enable denomi	*/
			}
			break;
#endif
		default:
			break;
		}
	}

	return(ret);
}


/******************************************************************************/
/*! @brief Get Conndiciton Error Code Read.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static	u8	phase_error_code_read(u8 cmd)
{
	u8	response = 0;

	switch(cmd)
	{
	case	CMD_ENQ:
			/* Abnormal Error */
			if(ex_abnormal_code != 0)
			{
				if(convert_error_code(ex_abnormal_code) != 0)
				{
					ex_usb_write_size = (1 + FUSB_HEADER_SIZE);
				}
				else
				{
					ex_usb_write_size = (FUSB_HEADER_SIZE);
				}

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			}
			/* Reject Error */
			else
			{
				if(convert_reject_code(ex_validation.reject_code) != 0)
				{
					ex_usb_write_size = (1 + FUSB_HEADER_SIZE);
				}
				else
				{
					ex_usb_write_size = (FUSB_HEADER_SIZE);
				}

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			}

			break;
	default:
			response = RES_NAK;
			break;
	}

	return(response);
}


static	u8	convert_reject_code(u16 reject_code)
{
	u8 current_reject_code = 0;

	switch (reject_code)
	{
	case	REJECT_CODE_OK:
			break;
	case	REJECT_CODE_INSERT_CANCEL:
	case	REJECT_CODE_SKEW:
			current_reject_code = ID003_REJECT_INSERTION;					/* 挿入異常 */
			break;
	case	REJECT_CODE_MAG_PATTERN:
	case	REJECT_CODE_MAG_AMOUNT:
			current_reject_code = ID003_REJECT_MAG;							/* マグ異常 */
			break;
	case	REJECT_CODE_ACCEPTOR_STAY_PAPER:
			current_reject_code = ID003_REJECT_REMAINING_BILL_IN_ACCEPTOR;	/* 紙幣の残留(アクセプタ部) */
			break;
	case REJECT_CODE_STACKER_STAY_PAPER:
			current_reject_code = ID003_REJECT_REMAINING_BILL_IN_STACKER;	/* 紙幣の残留(スタッカー部) */
			break;
	case	REJECT_CODE_XRATE:
#if defined(_PROTOCOL_ENABLE_ID003GD)
			current_reject_code = ID003_REJECT_PATTERN1;
#else
			current_reject_code = ID003_REJECT_XRATE;						/* 補正/振幅率異常 */
#endif /* _PROTOCOL_ENABLE_ID003GD */
			break;
	case	REJECT_CODE_FEED_SLIP:
	case	REJECT_CODE_FEED_MOTOR_LOCK:
	case	REJECT_CODE_FEED_TIMEOUT:
	case	REJECT_CODE_APB_HOME:
	case	REJECT_CODE_CENTERING_HOME:
	case	REJECT_CODE_LOST_BILL:
			current_reject_code = ID003_REJECT_FEED;						/* 搬送異常 */
			break;
	case	REJECT_CODE_PRECOMP:
			current_reject_code = ID003_REJECT_PRECOMP;						/* 金種判定異常 */
			break;
	case	REJECT_CODE_PATTERN:
			current_reject_code = ID003_REJECT_PATTERN1;					/* フォトパタン異常1 */
			break;
	case	REJECT_CODE_PHOTO_LEVEL:
			current_reject_code = ID003_REJECT_PHOTO_LEVEL;					/* フォトレベル異常 */
			break;
	case	REJECT_CODE_INHIBIT:
	case	REJECT_CODE_ESCROW_TIMEOUT:
	case	REJECT_CODE_RETURN:
			current_reject_code = ID003_REJECT_INHIBIT;						/* INHIBIT信号 or エスクロタイムアウト */
			break;
	case	REJECT_CODE_OPERATION:
			current_reject_code = ID003_REJECT_OPERATION;					/* オペレーション異常 */
			break;
	case	REJECT_CODE_LENGTH:
	case 	REJECT_CODE_PAPER_SHORT:
	case 	REJECT_CODE_PAPER_LONG:
			current_reject_code = ID003_REJECT_LENGTH;						/* 長さエラー */
			break;
	case 	REJECT_CODE_COUNTERFEIT:
	case 	REJECT_CODE_THREAD:
			current_reject_code = ID003_REJECT_COUNTERFEIT;					/* 真券特徴異常 */
			break;
	case 	REJECT_CODE_BAR_NC:
			current_reject_code = ID003_REJECT_BAR_NC;
			break;
	case 	REJECT_CODE_BAR_UN:
			current_reject_code = ID003_REJECT_BAR_UN;
			break;
	case 	REJECT_CODE_BAR_SH:
			current_reject_code = ID003_REJECT_BAR_SH;
			break;
	case 	REJECT_CODE_BAR_ST:
			current_reject_code = ID003_REJECT_BAR_ST;
			break;
	case 	REJECT_CODE_BAR_SP:
			current_reject_code = ID003_REJECT_BAR_SP;
			break;
	case 	REJECT_CODE_BAR_TP:
			current_reject_code = ID003_REJECT_BAR_TP;
			break;
	case 	REJECT_CODE_BAR_XR:
			current_reject_code = ID003_REJECT_BAR_XR;
			break;
	case 	REJECT_CODE_BAR_PHV:
			current_reject_code = ID003_REJECT_BAR_PHV;
			break;
	case 	REJECT_CODE_BAR_DIN:
			current_reject_code = ID003_REJECT_BAR_DIN;
			break;
	case 	REJECT_CODE_BAR_LG:
			current_reject_code = ID003_REJECT_BAR_LG;
			break;
	case 	REJECT_CODE_BAR_NG:
			current_reject_code = ID003_REJECT_PATTERN2;
			break;
	case 	REJECT_CODE_BAR_MC:
			current_reject_code = ID003_REJECT_COUNTERFEIT;
			break;
	default:					/* other */
			current_reject_code = ID003_REJECT_FEED;
			break;
	}

	if(current_reject_code != 0)
	{
		*(ex_usb_write_buffer + 5) = 0x17;
		*(ex_usb_write_buffer + 6) = current_reject_code;
	}
	else
	{
		*(ex_usb_write_buffer + 5) = current_reject_code;
	}

	return(current_reject_code);
}


static	u8	convert_error_code(u16 alarm_code)
{
	u8 current_abnormal_code = 0;
	u8 current_abnormal_data = 0;

	switch (alarm_code)
	{
	case	ALARM_CODE_OK:
			break;
	case	ALARM_CODE_STACKER_FULL:
			current_abnormal_code = ID003_STS_STACKER_FULL;
			break;
	case	ALARM_CODE_FEED_OTHER_SENSOR_SK:
	case	ALARM_CODE_FEED_SLIP_SK:
	case	ALARM_CODE_FEED_TIMEOUT_SK:
	case	ALARM_CODE_FEED_LOST_BILL:
	case 	ALARM_CODE_FEED_MOTOR_LOCK_SK: //2024-02-13
			current_abnormal_code = ID003_STS_JAM_IN_SK;
			break;
	case	ALARM_CODE_FEED_MOTOR_SPEED_LOW:
	case	ALARM_CODE_FEED_MOTOR_SPEED_HIGH:
			current_abnormal_code = ID003_STS_FAILURE;
			current_abnormal_data = ID003_FAILURE_FEED_MOTOR_SPEED;
			break;
	case	ALARM_CODE_FEED_MOTOR_LOCK:
			current_abnormal_code = ID003_STS_FAILURE;
			current_abnormal_data = ID003_FAILURE_FEED_MOTOR;
			break;
	case	ALARM_CODE_FEED_OTHER_SENSOR_AT:
	case	ALARM_CODE_FEED_SLIP_AT:
	case	ALARM_CODE_FEED_TIMEOUT_AT:
	case	ALARM_CODE_FEED_MOTOR_LOCK_AT:
			current_abnormal_code = ID003_STS_JAM_IN_AT;
			break;
	case	ALARM_CODE_APB_HOME:
	case	ALARM_CODE_APB_TIMEOUT:
	case	ALARM_CODE_APB_HOME_STOP:
			current_abnormal_code = ID003_STS_FAILURE;
			current_abnormal_data = ID003_FAILURE_APB_UNIT;
			break;
	case	ALARM_CODE_CHEAT:
			current_abnormal_code = ID003_STS_CHEATED;
			break;
	case	ALARM_CODE_CENTERING_TIMEOUT:
	case	ALARM_CODE_CENTERING_HOME_STOP:
			current_abnormal_code = ID003_STS_FAILURE;
			current_abnormal_data = ID003_FAILURE_CENTERING;
			break;
	default:					/* other */
			current_abnormal_code = ID003_STS_EXT_OTHER_EEROR;
			break;
	}

	if(current_abnormal_code != 0)
	{
		*(ex_usb_write_buffer + 5) = current_abnormal_code;
		*(ex_usb_write_buffer + 6) = current_abnormal_data;
	}
	else
	{
		*(ex_usb_write_buffer + 5) = current_abnormal_code;
	}

	return(current_abnormal_code);
}



/******************************************************************************/
/*! @brief Get Conndiciton Error Code Read.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static	u8	phase_maintenance_read(u8 cmd)
{
	u8	response = 0;
	u8	ret;

	switch(cmd)
	{
	case	CMD_ENQ:
			ret = is_cleaning_required();

			ex_usb_write_size = (FUSB_HEADER_SIZE);
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = ret;
			break;
	default:
			response = RES_NAK;
			break;
	}

	return(response);
}
/*--- End of File ---*/
