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

#ifndef _SRC_ID003_H_
#define _SRC_ID003_H_

#define	ID003_ADD_04	0x04 //2025-01-17

enum _ID003_MODE
{
	ID003_MODE_POWERUP = 1,							/* status : ID003_STS_POWERUP */
	ID003_MODE_POWERUP_BILLIN_AT,					/* status : ID003_STS_POWERUP_BILLIN_AT */
	ID003_MODE_POWERUP_BILLIN_SK,					/* status : ID003_STS_POWERUP_BILLIN_SK */
	ID003_MODE_POWERUP_ERROR,						/* status : ID003_STS_STACKER_FULL ..... ID003_STS_FAILURE */
	ID003_MODE_POWERUP_INITIAL,			/*  5 */	/* status : ID003_STS_INITIALIZE */
	ID003_MODE_POWERUP_INITIAL_STACK,				/* status : ID003_STS_INITIALIZE */
	ID003_MODE_POWERUP_INITIAL_VEND,				/* status : ID003_STS_VEND_VALID */
	ID003_MODE_POWERUP_INITIAL_VEND_ACK,			/* status : ID003_STS_VEND_VALID */
	ID003_MODE_POWERUP_INITIAL_REJECT,				/* status : ID003_STS_INITIALIZE */
	ID003_MODE_POWERUP_INITIAL_PAUSE,	/* 10 */	/* status : ID003_STS_PAUSE */
	ID003_MODE_INITIAL,								/* status : ID003_STS_INITIALIZE */
	ID003_MODE_INITIAL_STACK,						/* status : ID003_STS_INITIALIZE */
	ID003_MODE_INITIAL_REJECT,			/* 15 */	/* status : ID003_STS_INITIALIZE */
	ID003_MODE_INITIAL_PAUSE,						/* status : ID003_STS_PAUSE */
	ID003_MODE_AUTO_RECOVERY,						/* status : ID003_STS_INITIALIZE *///通常動作時
	ID003_MODE_AUTO_RECOVERY_STACK,					/* status : ID003_STS_INITIALIZE */
	ID003_MODE_AUTO_RECOVERY_REJECT,				/* status : ID003_STS_INITIALIZE */
	ID003_MODE_AUTO_RECOVERY_PAUSE,		/* 20 */	/* status : ID003_STS_PAUSE */
	ID003_MODE_DISABLE,								/* status : ID003_STS_DISABLE */
	ID003_MODE_DISABLE_REJECT,						/* status : ID003_STS_DISABLE */
	ID003_MODE_ENABLE_WAIT_POLL,					/* status : ID003_STS_ENABLE */
	ID003_MODE_ENABLE,								/* status : ID003_STS_ENABLE */
	ID003_MODE_ENABLE_REJECT,			/* 25 */	/* status : ID003_STS_ENABLE */
	ID003_MODE_ACCEPT,								/* status : ID003_STS_ACCEPTING */
//	ID003_MODE_ACCEPT_POLL_RECEIVED,				/* status : ID003_STS_ACCEPTING */
	ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT,			/* status : ID003_STS_ACCEPTING */
	ID003_MODE_ESCROW,								/* status : ID003_STS_ESCROW */
	ID003_MODE_ESCROW_WAIT_CMD,			/* 30 */	/* status : ID003_STS_ESCROW */
	ID003_MODE_HOLD1,								/* status : ID003_STS_HOLDING */
	ID003_MODE_HOLD2,								/* status : ID003_STS_HOLDING */
	ID003_MODE_STACK,								/* status : ID003_STS_STACKING */
	ID003_MODE_PAUSE,								/* status : ID003_STS_PAUSE */
	ID003_MODE_VEND,					/* 35 */	/* status : ID003_STS_VEND_VALID */
	ID003_MODE_VEND_FULL,							/* status : ID003_STS_VEND_VALID */
	ID003_MODE_WAIT_VEND_ACK,						/* status : ID003_STS_VEND_VALID */
	ID003_MODE_WAIT_VEND_ACK_FULL,					/* status : ID003_STS_VEND_VALID */
	ID003_MODE_STACKED,								/* status : ID003_STS_STACKED */
	ID003_MODE_STACKED_FULL,			/* 40 */	/* status : ID003_STS_STACKED */
//	ID003_MODE_STACK_FINISH,						/* status : ID003_STS_STACKED */
	ID003_MODE_REJECT_WAIT_ACCEPT_RSP,				/* status : ID003_STS_REJECTING */
	ID003_MODE_REJECT,								/* status : ID003_STS_REJECTING */
	ID003_MODE_REJECT_WAIT_POLL,					/* status : ID003_STS_REJECTING */
	ID003_MODE_REJECT_WAIT_NOTE_REMOVED,			/* status : ID003_STS_REJECTING */
	ID003_MODE_RETURN,					/* 45 */	/* status : ID003_STS_RETURNING */
	ID003_MODE_LOG_ACCESS,							/* status : ID003_STS_LOG_ACCESS */
//	ID003_MODE_DISABLE_BOOKMARK,					/* status : ID003_STS_DISABLE */
//	ID003_MODE_DISABLE_REJECT_BOOKMARK,				/* status : ID003_STS_DISABLE */
//	ID003_MODE_ENABLE_BOOKMARK,						/* status : ID003_STS_ENABLE */
//	ID003_MODE_ENABLE_REJECT_BOOKMARK,				/* status : ID003_STS_ENABLE */
//	ID003_MODE_ACCEPT_BOOKMARK,						/* status : ID003_STS_ACCEPTING */
//	ID003_MODE_STACK_BOOKMARK,						/* status : ID003_STS_ACCEPTING */
//	ID003_MODE_PAUSE_BOOKMARK,						/* status : ID003_STS_PAUSE */
//	ID003_MODE_STACKED_BOOKMARK,					/* status : ID003_STS_STACKED */
//	ID003_MODE_WAIT_POLL_BEFOR_FULL,				/* [For KF] status : ID003_STS_ENABLE or ID003_STS_DISABLE */
//	ID003_MODE_CLEANING,							/* status : ID003_STS_CLEANING */
	ID003_MODE_ERROR,								/* status : ID003_STS_STACKER_FULL ..... ID003_STS_FAILURE */
//	ID003_MODE_NEARLY_FULL,							/* status : ID003_STS_NEARLY_FULL	 */
	ID003_MODE_WAIT_PAUSE,
	ID003_MODE_SIGNATURE_BUSY,			/* 50 */
	ID003_MODE_SIGNATURE_END,
	ID003_MODE_SHA1_HASH_BUSY,
	ID003_MODE_SHA1_HASH_END,
	ID003_MODE_POWERUP_SIGNATURE_BUSY,
	ID003_MODE_POWERUP_SIGNATURE_END,
	ID003_MODE_POWERUP_SHA1_HASH_BUSY,
	ID003_MODE_POWERUP_SHA1_HASH_END,
	ID003_MODE_SYSTEM_ERROR,						/* status : ID003_FAILURE_EXT_ROM */
	ID003_MODE_DISABLE_REJECT_2ND, //2023-09-05
	ID003_MODE_ACCEPT_WAIT_POLL_FOR_ESCROW,			/* status :  *///2024-03-05

#if defined(UBA_RTQ)
	ID003_MODE_POWERUP_INITIAL_PAYVALID,
    ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK,	
	ID003_MODE_COLLECT,					/* 60 */
	ID003_MODE_COLLECTED_WAIT_POLL,
	ID003_MODE_RETURN_TO_BOX,			//エマージェンシーストップで紙幣取り込み中
	/* PAYOUT STATUS */
	ID003_MODE_PAYOUT,					/* 63 */
	ID003_MODE_PAYOUT_RETURN_NOTE,
	ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL,
	ID003_MODE_PAYSTAY,
	ID003_MODE_PAYSTAY_WAIT_POLL,
	ID003_MODE_PAYVALID,
	ID003_MODE_PAYVALID_ERROR,
	ID003_MODE_WAIT_PAYVALID_ACK,
	ID003_MODE_WAIT_PAYVALID_ACK_ERROR,
	ID003_MODE_AFTER_PAYVALID_ACK_ENABLE,	//すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのEnableとは分けている
	ID003_MODE_AFTER_PAYVALID_ACK_DISABLE,	//すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのDisableとは分けている
	ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT,//払い出し継続
	/**/
//	ID003_MODE_RC_ERROR,				/* 75 *///not use UBA500
	ID003_MODE_DIAGNOSTIC,				/* 76 */
	ID003_MODE_RETURN_ERROR,	//	Pay out紙幣がエマージョエンシ―ストップで、回収、識別エラーで返却中
#endif // UBA_RTQ
	ID003_MODE_END,
};


#define ID003_SYNC						0xFC	/* 送信開始コード */
#define ID003_SYNC_GLOBAL				0xF0


typedef struct _ID003_CMD_BUFFER
{
	u8	sync;
	u8	length;			/* size */
	u8	cmd;			/* command */
	u8	data[250];		/* addition data */
	u8	crc1;
	u8	crc2;
} ID003_CMD_BUFFER;

#if defined(UBA_RTQ)
typedef struct _ID003_CMD_BUFFER_EXT_S
{
	u8 sync;		
	u8 length;
	u8 ext_cmd;
	u8 unit;
	union
	{
		u8 sst1;
		u8 cmd;
	}cmd_rsp;
	u8 data[248];
	u8 crc1;
	u8 crc2;
}_ID003_CMD_BUFFER_EXT_T, *_ID003_CMD_BUFFER_EXT_P;
#endif // UBA_RTQ

enum _ID003_LISTEN
{
	ID003_LISTEN_TMOUT = 0,
	ID003_LISTEN_COMMAND,
	ID003_LISTEN_CRC_ERROR,
	ID003_LISTEN_LENGTH_ERROR,
};

/*--------------------------------------------------------------*/
/*					id003 Command Code							*/
/*--------------------------------------------------------------*/
#define	ID003_CMD_ENQ					0x05	/* ENQ */
#define	ID003_CMD_STS_REQUEST			0x11	/* ステータスリクエスト */
#define	ID003_CMD_RESET					0x40	/* リセットコマンド */
#define	ID003_CMD_STACK1				0x41	/* STACK1コマンド */
#define	ID003_CMD_STACK2				0x42	/* STACK2コマンド */
#define	ID003_CMD_RETURN				0x43	/* 返却コマンド */
#define	ID003_CMD_HOLD					0x44	/* 状態保持コマンド */
#define	ID003_CMD_WAIT					0x45	/* 状態保留コマンド */
#define	ID003_CMD_STACK3				0x49	/* STACK3コマンド */
#define	ID003_CMD_INVALID				0x4B	/* コマンド無効 */
#define	ID003_CMD_BOOKMARK				0x4A	/* BOOK MARKコマンド */
#define	ID003_CMD_BOOKMARK_CANCEL		0x4B	/* BOOK MARK CANCELコマンド */
#define	ID003_CMD_ACK					0x50	/* ACK */
#define	ID003_CMD_ENABLE				0xC0	/* 受取設定コマンド */
#define	ID003_CMD_SECURITY				0xC1	/* セキュリティ設定コマンド */
#define	ID003_CMD_COMMUNICATION			0xC2	/* 通信モード設定コマンド */
#define	ID003_CMD_INHIBIT				0xC3	/* INHIBIT設定コマンド */
#define	ID003_CMD_DIRECTION				0xC4	/* 受取方向設定コマンド */
#define	ID003_CMD_OPTIONAL_FUNC			0xC5	/* オプション設定コマンド */
#define	ID003_CMD_BARCODE_FUNC			0xC6	/* バーコードクーポンのタイプ・キャラクタ長設定コマンド */
#define	ID003_CMD_BARCODE_INHIBIT		0xC7	/* 紙幣・バーコードクーポンのINHIBIT設定コマンド */
#define	ID003_CMD_COUNTRY_TYPE			0xCB	/* Country Type Setting */
#define	ID003_CMD_SET_ICB_MC			0xCC	/*	IT BOX用 M/C No from HOST				*/
#define	ID003_CMD_NEARLY_FULL_LIMIT		0xB0	/* Nearly Full detection number of notes setting */
#define	ID003_CMD_GET_ENABLE			0x80	/* 受取設定リード */
#define	ID003_CMD_GET_SECURITY			0x81	/* セキュリティ設定リード */
#define	ID003_CMD_GET_COMMUNICATION		0x82	/* 通信モード設定リード */
#define	ID003_CMD_GET_INHIBIT			0x83	/* 受取設定リード */
#define	ID003_CMD_GET_DIRECTION			0x84	/* 受取方向設定リード */
#define	ID003_CMD_GET_OPTIONAL_FUNC		0x85	/* オプション設定リード */
#define	ID003_CMD_GET_BARCODE_FUNC		0x86	/* バーコードクーポンのタイプ・キャラクタ長設定リード */
#define	ID003_CMD_GET_BARCODE_INHIBIT	0x87	/* 紙幣・バーコードクーポンのINHIBIT設定リード */
#define	ID003_CMD_GET_VERSION			0x88	/* バージョンリード */
#define	ID003_CMD_GET_BOOT_VERSION		0x89	/* ブートバージョンリード */
#define	ID003_CMD_GET_CURRENCY_ASSING	0x8A	/* カレンシーデータリード */
#define	ID003_CMD_GET_COUNTRY_TYPE		0x8B	/* Read Country Type Setting */
#define	ID003_CMD_GET_SECRET_NUMBER		0x8F	/* Secret Numberリード *//* '12-05-26 */
#define	ID003_CMD_GET_NEARLY_FULL_LIMIT	0x90	/* Nearly Full detection number of notes request */
#define	ID003_CMD_GET_SERIAL_NUMBER		0x91	/* Serial Number request */

#if defined(UBA_RTQ)
#define ID003_CMD_GET_UNIT_INFO	0x92	/* Unit Info Extra */
#endif // UBA_RTQ
#define ID003_CMD_GET_REVISION_NUMBER	0x93	/* Revision Number request *///おそらく拡張コマンドではない

#if defined(UBA_RTQ)	//#if defined(UBA_ID003_ENC)
#define ID003_CMD_SET_ENC_MODE          0xC8    /* Encryption mode setting */
#define ID003_CMD_GET_ENC_MODE          0x98    /* Encryption mode request */
#endif // UBA_ID003_ENC

#define	ID003_CMD_GET_BOX_CAPACITY		0x96	/* BOX Capacity Request */
//#define	ID003_CMD_AUTHENTICATION		0x97	/* AUTHENTICATION CODE REQUEST */

#if defined(UBA_RTQ)
#define	ID003_CMD_GET_SENSOR_STATUS		0x9B
#endif

#define ID003_CMD_GET_SHA1_HASH			0xDB
#define ID003_CMD_PROGRAM_SIGNATURE		0xDC

#define	ID003_CMD_DOWNLOAD_REQUEST		0xD0	/* ダウンロード開始リクエスト         */
#define	ID003_CMD_DOWNLOAD_DATA			0xD1	/* ダウンロードデータコマンド         */
#define	ID003_CMD_DOWNLOAD_END_REQUEST	0xD2	/* ダウンロード終了リクエスト         */
#define	ID003_CMD_BAUDRATE_REQUEST		0xD5	/* Daudrateリクエスト                 */
#define	ID003_CMD_DIFFERENTIAL_DOWNLOAD	0xD6	/* Differentialダウンロードリクエスト */

#define	ID003_CMD_LOGDATA_INITIALIZE	0xE0	/* ログデータ初期化コマンド */
#define	ID003_CMD_LOGDATA_OFFSET		0xE1	/* ログデータオフセット位置指定コマンド */
#define	ID003_CMD_LOGDATA_REQUEST		0xE2	/* ログデータ取得コマンド */
#define	ID003_CMD_LOGDATA_END			0xE3	/* ログデータ完了コマンド */
#define	ID003_CMD_TOTAL_DENOMINATION	0xE4	/* ログデータ完了コマンド */


//名前は戻した
#define	ID003_CMD_EXT_RC				0xF0	/* 拡張コマンド（ユニット） */
#define	ID003_CMD_UNIT_RC				0x20	/* ユニット（ＲＣ） */
#define	ID003_UNIT_ID_RECYCLER			0x20	/* ユニット種別(RECYCLER)							*/
#define	ID003_UNIT_ID_RECYCLER_RSUNIT	0x24	/* ユニット種別(RECYCLER + RS unit)					*/
#define	ID003_CMD_EXT_RC_STATUS			0x1A	/* 拡張コマンド ステータス要求 */
#define	ID003_CMD_EXT_RC_PAYOUT			0x4A	/* 拡張コマンド 払出コマンド */
#define	ID003_CMD_EXT_RC_COLLECT		0x4B	/* 拡張コマンド 回収コマンド */
#define	ID003_CMD_EXT_RC_CLEAR			0x4C	/* 拡張コマンド エラー解除コマンド */// not use only use RC200
#define	ID003_CMD_EXT_RC_E_STOP			0x4D	/* 拡張コマンド 緊急停止コマンド */
#define	ID003_CMD_EXT_RC_MC_SET			0xD0	/* 拡張コマンド リサイクル金種設定コマンド */
#define	ID003_CMD_EXT_RC_KEY_SET		0xD1	/* 拡張コマンド キー許可・禁止コマンド */
#define	ID003_CMD_EXT_RC_COUNT_SET		0xD2	/* 拡張コマンド 還流枚数上限設定コマンド */
#define	ID003_CMD_EXT_RC_REFILL_SET		0xD4	/* 拡張コマンド 補充モード設定コマンド *//* '12-05-27 */
#define	ID003_CMD_EXT_RC_ENABLE_SET		0xD6	/* 拡張コマンド(RC) RCユニット設定コマンド			*/
#define ID003_CMD_EXT_RC_CURRENT		0xE2	/* 拡張コマンド */
#define	ID003_CMD_EXT_RC_MC_REQ			0x90	/* 拡張コマンド リサイクル金種要求コマンド */
#define	ID003_CMD_EXT_RC_KEY_REQ		0x91	/* 拡張コマンド キー状態要求コマンド */
#define	ID003_CMD_EXT_RC_COUNT_REQ		0x92	/* 拡張コマンド 還流上限枚数リード */
#define	ID003_CMD_EXT_RC_VERSION_REQ	0x93	/* 拡張コマンド バージョンリード */
#define	ID003_CMD_EXT_RC_REFILL_REQ		0x94	/* 拡張コマンド 補充モード設定リードコマンド *//* '12-05-27 */
#define	ID003_CMD_EXT_RC_CURRENCY		0x95	/* 拡張コマンド(RC) リサイクル金種リードコマンド	*///2024-07-04
#define	ID003_CMD_EXT_RC_ENABLE_REQ		0x96	/* 拡張コマンド(RC) RCユニット設定リードコマンド	*///UBA_MUST
#define	ID003_CMD_EXT_RC_TOTAL			0xA0	/* 拡張コマンド リサイクル合計器要求コマンド */
#define	ID003_CMD_EXT_RC_TOTAL_CLEAR	0xA1	/* 拡張コマンド リサイクル合計器クリア コマンド */
#define	ID003_CMD_EXT_RC_CURRENT_REQ	0xA2	/* 拡張コマンド リサイクル補充枚数要求 コマンド */
#define	ID003_CMD_EXT_RC_BATTERY_CLEAR	0xA3	/* 拡張コマンド(RC) Battery Conditionクリアコマンド	*/

#define	ID003_CMD_EXT_RC_DLD_STATUS		0x2A	/* 拡張コマンド(RC) Downloadステータス要求コマンド	*///UBA_MUST

#define	ID003_CMD_DLE					0x10	/* 拡張コマンド Data Link Escape				*///UBA_MUST
#define	ID003_CMD_DLE_ENCKEY_SET		0x01	/* 拡張コマンド Encryption Key Setting			*///UBA_MUST
#define	ID003_CMD_DLE_ENCNUM_SET		0x02	/* 拡張コマンド Encryption Number Setting		*///UBA_MUST

// このコマンド自体暗号化される様だ
//#define	ID003_CMD_DLE_KEY_NUM_SET		0x03	/* 拡張コマンド UBA500は直接書いているが 0x03というKeyとNumberを一度に設定するコマンドがある		*///UBA_MUST

#define ID003_CMD_EXT_RC_EXSTATUS		0x3A 	/* 拡張コマンド ステータス要求	細かく表示			*/


/* <<<<< Extension Command >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
#define	ID003_CMD_EXTENSION				0xFF	/* 拡張コマンド */

#if defined(UBA_RTQ)		/* '20-12-14 */
#define	ID003_CMD_EX_DIAG_MODE_START	0x40	/* 拡張コマンド Diagnostic Mode Start */
#define	ID003_CMD_EX_DIAG_TEST			0x41	/* 拡張コマンド Diagnostic Test Start/End */
#define	ID003_CMD_EX_DIAG_MODE_END		0x43	/* 拡張コマンド Diagnostic Mode End */
#define	ID003_CMD_EX_DIAG_STATUS		0x44	/* 拡張コマンド Diagnostic Status */
#endif

#define	ID003_CMD_ICB_FUNCTION			0xB1	/* ICB FUNCTION SETTING */

#define	ID003_CMD_GET_MACHINE_NUMBER	0x8C	/* MACHINE NUMBER REQUEST */
#define	ID003_CMD_GET_BOX_NUMBER		0x8E	/* BOX NUMBER REQUEST */

#define ID003_CMD_GET_2D_BARCODE_SETTING	0xA0
#define ID003_CMD_RXD_2D_BARCODE			0xA6

/* <<<<< Extension Command >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

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

#if defined(UBA_RTQ)
	#define	ID003_STS_PAYING				0x20	/* 払出中 */
	#define	ID003_STS_COLLECTING			0x21	/* 回収中 */
	#define	ID003_STS_COLLECTED				0x22	/* 回収完了 */
	#define	ID003_STS_PAY_VALID				0x23	/* 払出確定 */
	#define	ID003_STS_PAY_STAY				0x24	/* 払出紙幣残留 */
	#define	ID003_STS_RETURN_TO_BOX			0x25	/* 残留紙幣回収中 */
	#define	ID003_STS_RETURN_PAY_OUT		0x26	/* 払出し異常 */
	//#if defined(UBA_RS)
	#define	ID003_STS_NOTE_REMAIN			0x2E	/* 払出し残留紙幣あり	RS専用機能	*/
	//#endif
	#define	ID003_STS_RETURN_ERROR			0x2F	/* 残留紙幣回収エラー */
	#define	ID003_STS_RC_ERROR				0x4C	/* RC異常					*/
	#define	ID003_FAILURE_RC_ROM			0xC6
	#define	ID003_FAILURE_RC_COMM			0xC8
#endif // UBA_RTQ

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
#define ID003_STS_SHA1_HASH_BUSY_OR_END	0xDB
#define ID003_STS_SIGNATURE_BUSY		0xDE
#define ID003_STS_SIGNATURE_END			0xDF

/* <<<<< Extension Status >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
#define	ID003_STS_EXTENSION				0xFF	/* 拡張ステータス */

#if defined(UBA_RTQ)
#define ID003_STS_EXT_DIAG_READY		0x00	/* diagnostic ready */
#define ID003_STS_EXT_DIAG_BUSY			0x80	/* diagnostic busy */
#endif // UBA_RTQ

#define ID003_STS_EXT_LOG_ACCESS_READY	0x06	/*  */
#define ID003_STS_EXT_LOG_ACCESS_END	0x03	/*  */
#define ID003_STS_EXT_LOG_ACCESS_BUSY	0x80	/*  */

#define	ID003_STS_EXT_RC_DISCONNECT		0x00	/* 拡張ステータス(RC) 未接続						*/
#define	ID003_STS_EXT_RC_NORMAL			0x10	/* 拡張ステータス(RC) 動作可能状態					*/
#define	ID003_STS_EXT_RC_EMPTY			0x11	/* 拡張ステータス(RC) 紙幣なし						*/
#define	ID003_STS_EXT_RC_FULL			0x12	/* 拡張ステータス(RC) 紙幣フル						*/
#define	ID003_STS_EXT_RC_BUSY			0x1F	/* 拡張ステータス(RC) ビジー						*/
#define	ID003_STS_EXT_RC_JAM			0x40	/* 拡張ステータス(RC) ジャム発生					*/
#define	ID003_STS_EXT_RC_DOOR_OPEN		0x41	/* 拡張ステータス(RC) 扉開							*/
#define	ID003_STS_EXT_RC_MOTOR_ERROR	0x42	/* 拡張ステータス(RC) モータエラー					*/
#define	ID003_STS_EXT_RC_EEPROM_ERROR	0x43	/* 拡張ステータス(RC) EEPROMエラー					*/
#define	ID003_STS_EXT_RC_NOTE_ERROR		0x44	/* 拡張ステータス(RC) PAYOUT NOTE ERROR				*/
#define	ID003_STS_EXT_RC_BOX_OPEN		0x45	/* 拡張ステータス(RC) RC BOXオープン				*/
#define	ID003_STS_EXT_RC_EEROR			0x4A	/* 拡張ステータス(RC) その他エラー					*/
#define	ID003_STS_EXT_RC_REMOVED_PWROFF	0x50	/* 拡張ステータス(RC) REMOVED UNIT ON POWER OFF		*/
#define	ID003_STS_EXT_RC_EXCHANGED_UNIT	0x51	/* 拡張ステータス(RC) EXCHANGED_UNIT				*/
#define	ID003_STS_EXT_RC_LOW_BATTERY	0x52	/* 拡張ステータス(RC) LOW BATTERY					*/

/* <<<<< Extension Status >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */


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
#define ID003_FAILURE_ICB_RW						0x02	/* ICB read write failure */
#define ID003_FAILURE_ICB_SETTING					0x03	/* ICB setting failure */
#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
	#define ID003_FAILURE_ICB_RTQ					0x04	/* RTQでの書き込みエラー */
#endif
#define ID003_FAILURE_ICB_SUM						0x07	/* ICB checksum failure */
#define ID003_FAILURE_ICB_MC_NUMBER					0x08	/* ICB box machine number failure */
#define ID003_FAILURE_ICB_NOT_INITIALIZED			0x09	/* ICB box used */

#define	ID003_FAILURE_STACKER_MOTOR					0xA2	/* 収納モータ異常 */
#define	ID003_FAILURE_FEED_MOTOR_SPEED				0xA5	/* 搬送モータスピード異常 */
#define	ID003_FAILURE_FEED_MOTOR					0xA6	/* 搬送モータ異常 */
//#define	ID003_FAILURE_SOLENOID						0xA8	/* Solenoid Failure */
#define	ID003_FAILURE_APB_UNIT						0xAF	/* APB Unit Failure *//* UBA10,iPRO,UBA500に合わせる A9->AF*/
#define	ID003_FAILURE_BOX_READY						0xAB	/* Cash box not ready */
#define	ID003_FAILURE_HEAD							0xAF	/* Validator head remove */
#define	ID003_FAILURE_BOOT_ROM						0xB0	/* Boot ROM異常 */
#define	ID003_FAILURE_EXT_ROM						0xB1	/* 外部ROM異常 */
#define	ID003_FAILURE_RAM							0xB2	/* RAM異常 */
#define	ID003_FAILURE_EXT_ROM_WRITE					0xB3	/* External ROM writing failure */
#define ID003_FAILURE_CENTERING						0xAF	/* Width brings system error for EBA-30 *//* UBA,iPRO,UBA500に合わせる C1->AF*/
//#define ID003_FAILURE_STACKER_CONNECT				0xC3	/* Width brings system error for EBA-30 */


/*--------------------------------------------------------------*/
/*					id003 Externtion Status				*/
/*--------------------------------------------------------------*/
//#define	ID003_EX_DIAG_READY					0x40	/* DIAG READY */
//#define	ID003_EX_DIAG_BUSY					0x41	/* DIAG BUSY */


/*--------------------------------------------------------------*/
/*					id003 Optional Function						*/
/*--------------------------------------------------------------*/
#define ID003_OPTION_RECOVERY						0x0002
#define ID003_OPTION_24CHAR_TICKET					0x0008
//#define ID003_OPTION_NEARLY_FULL					0x0020
#define ID003_OPTION_ENTRANCE_SENSOR				0x0040
#define ID003_OPTION_ENCRYPTION						0x0080
#define	ID003_OPTION_CHANGE_PAYVALID_TIMING			0x0100
#if defined(UBA_RTQ)	/* '21-10-11 */
	#define	ID003_OPTION_SPRAY_MODE						0x0200
	//#if defined(UBA_RS)
	#define ID003_OPTION_REMAIN_NOTE					0x4000	//RS専用機能
	//#endif
#endif


/*--------------------------------------------------------------*/
/*					id003 Download Status Code					*/
/*--------------------------------------------------------------*/
#define ID003_DOWNLOAD_STS_READY					0x00
#define ID003_DOWNLOAD_STS_READY_DIFFERENTIAL		0x01
#define ID003_DOWNLOAD_STS_BUSY						0x80

/*--------------------------------------------------------------*/
/*					id003 Barcode								*/
/*--------------------------------------------------------------*/
#define ID003_BARCODE_ESCROW_CODE					0x6F

/*--------------------------------------------------------------*/
/*					id003 stacking info							*/
/*--------------------------------------------------------------*/
#define ID003_STACKING_INFO_BUSY					0x01
#define ID003_STACKING_INFO_WAIT_STS				0x02

#if defined(UBA_RTQ)		/* '24-08-01 */
/*--------------------------------------------------------------*/
/*					id003 paying info							*/
/*--------------------------------------------------------------*/
#define	ID003_PAYING_INFO_BUSY						0x01
#define	ID003_PAYING_INFO_WAIT_STS					0x02


/*--------------------------------------------------------------*/
/*					id003 collecting info						*/
/*--------------------------------------------------------------*/
#define	ID003_COLLECTING_INFO_BUSY					0x01
#define	ID003_COLLECTING_INFO_WAIT_STS				0x02
#endif // UBA_RTQ

/*--------------------------------------------------------------*/
/*					id003 Signature Code					    */
/*--------------------------------------------------------------*/
#define ID003_SIGNATURE_SHA1						0x01
#define ID003_SIGNATURE_CRC16						0x04

/*--------------------------------------------------------------*/
/*					id003 ICB Code							    */
/*--------------------------------------------------------------*/
#define ID003_SUBCMD_ICB_REQ		0x80
#define ID003_SUBCMD_MC_NO_REQ		0x8C
#define ID003_SUBCMD_ICB_SET		0xC0
#define ID003_ICB_ENABLE_STATUS		0x00
#define ID003_ICB_DISABLE_STATUS	0x01
/*--------------------------------------------------------------*/
/*					Function*/
/*--------------------------------------------------------------*/
extern u8 id003_main(void);
#if defined(UBA_RTQ)
extern void id003_set_escrow_for_payout( u8 escrow_code );
#endif // UBA_ID003_ENC

extern void uart_init_id003(void);
extern u8   uart_get_bufstatus_id003(void);
extern void uart_txd_stall_id003(void);
extern void uart_txd_active_id003(void);
extern void update_random_seed(unsigned int offset);

/*--------------------------------------------------------------*/
/*		For Blue Wave DX										*/
/*--------------------------------------------------------------*/
extern	bool	condition_denomi_inhibit_id003(u32 denomi_code, u32 direction);

 
#if defined(UBA_RTQ) //2025-01-16
/* 2019/11/18 */
	/* General Error */
	#define ID003_STS_EXT_RC_BOOTIF_AREA			0x70
	#define ID003_STS_EXT_RC_IF_AREA				0x71
	#define ID003_STS_EXT_RC_FRAM_AREA				0x72
	/* RFID error */
	#define	ID003_STS_EXT_RC_RFID_UNIT				0x73
	#define	ID003_STS_EXT_RC_RFID_COMM_ERROR		0x74
	/* Timeout Error */
	#define ID003_STS_EXT_RC_FEED1_TIMEOUT			0x80
	#define ID003_STS_EXT_RC_FEED2_TIMEOUT			0x81
	#define ID003_STS_EXT_RC_FLAP1_TIMEOUT			0x82
	#define ID003_STS_EXT_RC_FLAP2_TIMEOUT			0x83
	#define ID003_STS_EXT_RC_TWIN_DRUM1_TIMEOUT		0x84
	#define ID003_STS_EXT_RC_TWIN_DRUM2_TIMEOUT		0x85
	#define ID003_STS_EXT_RC_QUAD_DRUM1_TIMEOUT		0x86
	#define ID003_STS_EXT_RC_QUAD_DRUM2_TIMEOUT		0x87
	#define	ID003_STS_EXT_RS_FLAP_TIMEOUT			0x88
	/* Motor Lock Error */
	#define ID003_STS_EXT_RC_FEED1_MOTOR_LOCK		0x90
	#define ID003_STS_EXT_RC_FEED2_MOTOR_LOCK		0x91
	#define ID003_STS_EXT_RC_FLAP1_LEVER_FAIL		0x92
	#define ID003_STS_EXT_RC_FLAP2_LEVER_FAIL		0x93
	#define ID003_STS_EXT_RC_FLAP1_MOTOR_LOCK		0x94
	#define ID003_STS_EXT_RC_FLAP2_MOTOR_LOCK		0x95
	#define ID003_STS_EXT_RC_TWIN_DRUM1_MOTOR_LOCK	0x96
	#define ID003_STS_EXT_RC_TWIN_DRUM2_MOTOR_LOCK	0x97
	#define ID003_STS_EXT_RC_QUAD_DRUM1_MOTOR_LOCK	0x98
	#define ID003_STS_EXT_RC_QUAD_DRUM2_MOTOR_LOCK	0x99
	#define	ID003_STS_EXT_RS_FLAP_LEVER_FAIL		0x9A
	#define	ID003_STS_EXT_RS_FLAP_MOTOR_LOCK		0x9B
	/* Jam Error */
	#define ID003_STS_EXT_RC_FEED1_JAM_AT_DR		0xA0
	#define ID003_STS_EXT_RC_FEED2_JAM_AT_DR		0xA1
	#define ID003_STS_EXT_RC_FEED1_JAM_AT_TR		0xA2
	#define ID003_STS_EXT_RC_FEED2_JAM_AT_TR		0xA3
	#define ID003_STS_EXT_RC_TWIN_DRUM1_JAM			0xA4
	#define ID003_STS_EXT_RC_TWIN_DRUM2_JAM			0xA5
	#define ID003_STS_EXT_RC_QUAD_DRUM1_JAM			0xA6
	#define ID003_STS_EXT_RC_QUAD_DRUM2_JAM			0xA7
	/* Motor Speed Error */
	#define ID003_STS_EXT_RC_FEED1_SPEED_CHECK		0xB0
	#define ID003_STS_EXT_RC_FEED2_SPEED_CHECK		0xB1
	#define ID003_STS_EXT_RC_TWIN_DRUM1_SPEED_SEL	0xB2
	#define ID003_STS_EXT_RC_TWIN_DRUM2_SPEED_SEL	0xB3
	#define ID003_STS_EXT_RC_QUAD_DRUM1_SPEED_SEL	0xB4
	#define ID003_STS_EXT_RC_QUAD_DRUM2_SPEED_SEL	0xB5
	/* Full Error */
	#define ID003_STS_EXT_RC_TWIN_DRUM1_FULL		0xC0
	#define ID003_STS_EXT_RC_TWIN_DRUM2_FULL		0xC1
	#define ID003_STS_EXT_RC_QUAD_DRUM1_FULL		0xC2
	#define ID003_STS_EXT_RC_QUAD_DRUM2_FULL		0xC3
	/* Empty Error */
	#define ID003_STS_EXT_RC_TWIN_DRUM1_EMPTY		0xD0
	#define ID003_STS_EXT_RC_TWIN_DRUM2_EMPTY		0xD1
	#define ID003_STS_EXT_RC_QUAD_DRUM1_EMPTY		0xD2
	#define ID003_STS_EXT_RC_QUAD_DRUM2_EMPTY		0xD3
	/* Note Error*/
	#define ID003_STS_EXT_RC_FEED1_DOUBLE_BILL		0xE0
	#define ID003_STS_EXT_RC_FEED2_DOUBLE_BILL		0xE1
	#define ID003_STS_EXT_RC_FEED1_SHORT_BILL		0xE2
	#define ID003_STS_EXT_RC_FEED2_SHORT_BILL		0xE3

	#define	ID003_STS_EXT_RC_DOWNLOAD_READY			0x00
	#define	ID003_STS_EXT_RC_DOWNLOAD_BUSY			0x01
	
#endif //end UBA_RTQ


#endif
/*--- End of File ---*/
