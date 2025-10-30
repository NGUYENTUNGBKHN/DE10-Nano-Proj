/******************************************************************************/
/*! @addtogroup Group1
    @file       rc_if.h
    @brief      rc if header file
    @date       2018/01/15
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
******************************************************************************/
#if defined(UBA_RTQ)
/* Model */
#define		RC_MODEL_TWIN					2						/* RC-Twin model							*/
#define		RC_MODEL_QUAD					4						/* RC-Quad model							*/


/* RC-Twin(RC-Quad) max count */
#define		RECYCLER_BILL_MAX				30						/* max is 30 pices							*/

/* Common */
#define		RC_TWIN							0x01					/* RC-Twin									*/
#define		RC_QUAD							0x02					/* RC-Quad									*/
#define		RC_EXBOX						0x03					/* RC-EXBOX									*/
#define		RC_DRUM1						0x01					/* Drum1									*/
#define		RC_DRUM2						0x02					/* Drum2									*/

/* RC-Twin(RC-Quad) mode */
#define		HEAD_IF_MODE					0x00					/* UBA I/F Mode								*/
#define		HEAD_TEST_MODE					0x01					/* UBA Test Mode							*/

/* RC-Twin(RC-Quad) LED color control */
#define		DISP_OFF						0x00					/* 表示OFF									*/
#define		DISP_ON							0x01					/* 表示ON									*/

#define		COLOR_NONE						0x00					/* None										*/
#define		COLOR_GREEN						0x01					/* Green									*/
#define		COLOR_MAGENTA					0x02					/* Magenta									*/
#define		COLOR_SYAN						0x03					/* Syan										*/
#define		COLOR_BLUE						0x04					/* Blue										*/
#define		COLOR_YELLOW					0x05					/* Yellow									*/
#define		COLOR_RED						0x06					/* Red										*/
#define		COLOR_WHITE						0x07					/* White									*/

//2025-05-08
#define		ONLY_RC_UNIT					0x00
#define		RC_RS_UNIT						0x01
#define		ONLY_RS_UNIT					0x02


#if defined(UBA_RTQ)	/* '19-01-15 */
/* RC-Twin(RC-Quad) Operation mode		*/
#define		RC_MODE_STACK					1
#define		RC_MODE_PAYOUT					2
#define		RC_MODE_COLLECT					3
#define		TOTAL_CLEAR			    		4
#endif


/* RC-Twin(RC-Quad) feed motor */
#define		RC_FEED1						0x01					/* feed1 motor(RC-Twin)						*/
#define		RC_FEED2						0x02					/* feed2 motor(RC-Quad)						*/
#define		RC_FEED3						0x03					/* feed3 motor(RC-back)						*/

#define		RC_MOTOR_STOP					0x00					/* RC-Twin(RC-Quad)モータ停止				*/
#define		RC_MOTOR_FWD					0x01					/* RC-Twin(RC-Quad)モータ正転動作			*/
#define		RC_MOTOR_REV					0x02					/* RC-Twin(RC-Quad)モータ逆転動作			*/
#define		RC_MOTOR_INIT					0x03					/* RC-Twin(RC-Quad)モータイニシャル動作		*/


/* RC-Twin(RC-Quad) drum motor */
#define		RC_DRUM_STOP					0x00					/* RC-Twin(RC-Quad)ドラムモータ動作停止			*/
#define		RC_DRUM_START					0x01					/* RC-Twin(RC-Quad)ドラムモータ動作開始			*/
#define		RC_DRUM_INIT					0x02					/* RC-Twin(RC-Quad)ドラムモータイニシャル開始	*/

#define		RC_CASH_BOX						0x00					/* Cash Box									*/
#define		RC_TWIN_DRUM1					0x01					/* Drum1									*/
#define		RC_TWIN_DRUM2					0x02					/* Drum2									*/
#define		RC_QUAD_DRUM1					0x03					/* Drum3									*/
#define		RC_QUAD_DRUM2					0x04					/* Drum4									*/

#define		RC_MOT_FEED1					0x01					/* feed1 motor(RC-Twin)						*/
#define		RC_MOT_FEED2					0x02					/* feed2 motor(RC-Quad)						*/
#define		RC_MOT_FEED3					0x03					/* feed3 motor(RC-back)						*/
#define		RC_MOT_TWIN_DRUM1				0x04					/* Drum1									*/
#define		RC_MOT_TWIN_DRUM2				0x05					/* Drum2									*/
#define		RC_MOT_QUAD_DRUM1				0x06					/* Drum3									*/
#define		RC_MOT_QUAD_DRUM2				0x07					/* Drum4									*/


/* RC-Twin(RC-Quad) sol */
#define		RC_SOL_OFF						0x00					/* SOL OFF									*/
#define		RC_SOL_ON						0x01					/* SOL ON									*/


/* RC-Twin(RC-Quad) flapper position */
#define		RC_FLAP1_POS_HEAD_TO_RC			0x01					/* RC-Twin(RC-Quad)フラッパー HEAD to RC位置	*/
#define		RC_FLAP1_POS_RC_TO_BOX			0x02					/* RC-Twin(RC-Quad)フラッパー RC to BOX位置		*/
#define		RC_FLAP1_POS_HEAD_TO_BOX		0x03					/* RC-Twin(RC-Quad)フラッパー HEAD to BOX位置	*/
#define		RC_FLAP2_POS_RC_TO_BOX			0x01					/* RC-Twin(RC-Quad)フラッパー RC to BOX位置		*/
#define		RC_FLAP2_POS_HEAD_TO_RC			0x02					/* RC-Twin(RC-Quad)フラッパー HEAD to RC位置	*/
#define		RC_FLAP2_POS_HEAD_TO_BOX		0x03					/* RC-Twin(RC-Quad)フラッパー HEAD to BOX位置	*/
#define		RC_FLAP_POS_INIT				0x04					/* RC-Twin(RC-Quad)フラッパー イニシャル位置	*/


/* RC-Twin(RC-Quad) State */
#define		RC_STATE_IDLE					0x00					/* 入出金終了								*/
#define		RC_STATE_DEPOSIT_BEFORE_VEND	0x01					/* 入金中(VEND送信前) 						*/
#define		RC_STATE_DEPOSIT_AFTER_VEND		0x02					/* 入金中(VEND送信後) 						*/
#define		RC_STATE_DESPENSE_BEFORE_VEND	0x03					/* 出金中(VEND送信前) 						*/
#define		RC_STATE_DESPENSE_AFTER_VEND	0x04					/* 出金中(VEND送信後) 						*/
#define		RC_STATE_COLLECTION				0x05					/* 回収中									*/
#define		RC_STATE_REJECT					0x06					/* 返却中									*/
#define		RC_STATE_ABNORMAL				0x07					/* エラー発生中								*/
#define		RC_STATE_INITIALIZE				0x08					/* イニシャル動作開始						*/

/* RC-Twin(RC-Quad) Retry bill direction */
#define		RC_RETRY_STACK_DIR				0x00					/* 補充方向									*/
#define		RC_RETRY_PAYOUT_DIR				0x01					/* 払出し方向								*/


/* RC-Twin(RC-Quad) sensor test */
#define		RC_SENSOR_OFF					0x00					/* RC-Twin(RC-Quad)センサー動作停止			*/
#define		RC_SENSOR_ON					0x01					/* RC-Twin(RC-Quad)センサー動作開始			*/
#define		RC_SENSOR_BLINK					0x02					/* RC-Twin(RC-Quad)センサー点滅開始			*//* '19-06-19 */

#define		RC_TWIN_TRANSPORT_POS			0x01					/* RC-Twin搬送路ポジションセンサー			*/
#define		RC_TWIN_DRUM_POS				0x02					/* RC-Twinドラムポジションセンサー			*/
#define		RC_QUAD_TRANSPORT_POS			0x03					/* RC-Quad搬送路ポジションセンサー			*/
#define		RC_QUAD_DRUM_POS				0x04					/* RC-Quadドラムポジションセンサー			*/
//#if defined(UBA_RS)
#define		RC_RS_TRANSPORT_POS				0x05                    /* RS-Unit搬送路ポジションセンサー			*/
//#endif


// Initialize Type
#define		INITIAL_NORAML					0x00					/* Initialize Normal 		*/
#define		INITIAL_RECOVERY_DISPENSE_NONE	0x01					/* Initialize Stack			*/
#define		INITIAL_RECOVERY_DISPENSE_LIFT	0x02					/* Initialize Stack			*/
#define		INITIAL_RECOVERY_STACK			0x03					/* Initialize Reject		*/
#define		INITIAL_RECOVERY_LASTFEED		0x04

//#if defined(UBA_RTQ_ICB)
	/* RS flapper position */
	#define		RS_FLAP_POS_IN					0x01					/* RSフラッパー　入金位置					*/
	#define		RS_FLAP_POS_OUT					0x02					/* RSフラッパー　出金位置					*/
	#define		RS_FLAP_POS_INIT				0x04					/* RSフラッパー　イニシャル位置				*/

	/* RC RFID operation mode */
	#define		RFID_TEST_START					0x01					/* RC RFIDテスト開始						*/
	#define		RFID_TEST_END					0x00					/* RC RFIDテスト終了						*/

	#define		RFID_RUN						0x00					/* RFID通信開始								*/
	#define		RFID_ENQ						0x05					/* RFID1データ取得							*/
//#endif // UBA_RTQ_ICB

/* RC-Twin(RC-Quad) error */
#if defined(UBA_RTQ) //2024-07-12 UBA500ではRCを付けていない、通常のエラーと被る為にRCを追加した様だ
	// RTQとの通信でエラー取得 ex_rc_error_code ID-003の詳細エラー用に使用している 
    #define	ALARM_CODE_RC_OK						0
    #define	ALARM_CODE_BOOTIF_AREA				1					/* boot I/F area program error			*/
	#define	ALARM_CODE_RC_IF_AREA					2					/* main I/F area program error			*/// mainをエラー被っているので ALARM_CODE_IF_AREA -> ALARM_CODE_RC_IF_AREA
    #define	ALARM_CODE_FRAM_AREA				    3					/* FRAM(Sensor data) error				*/
    #define	ALARM_CODE_FEED1_TIMEOUT			    4					/* RC-Twin : feed1 timeout				*/
    #define	ALARM_CODE_FEED2_TIMEOUT			    5					/* RC-Quad : feed2 timeout				*/
    #define	ALARM_CODE_FEED1_MOTOR_LOCK			6					/* RC-Twin : feed1 motor lock			*/
    #define	ALARM_CODE_FEED2_MOTOR_LOCK			7					/* RC-Quad : feed2 motor lock			*/
    #define	ALARM_CODE_FEED1_JAM_AT_TR			8					/* RC-Twin : feed1 jam in transport		*/
    #define	ALARM_CODE_FEED2_JAM_AT_TR			9					/* RC-Quad : feed2 jam in transport		*/
    #define	ALARM_CODE_FEED1_JAM_AT_DR			10					/* RC-Twin : feed1 jam in drum			*/
    #define	ALARM_CODE_FEED2_JAM_AT_DR			11					/* RC-Quad : feed2 jam in drum			*/
    #define	ALARM_CODE_FEED1_SPEED_CHECK		    12					/* RC-Twin : feed1 speed check			*/
    #define	ALARM_CODE_FEED2_SPEED_CHECK		    13					/* RC-Twin : feed2 speed check			*/
    #define	ALARM_CODE_FEED1_DOUBLE_BILL		    14					/* RC-Twin : feed1 double bill check	*/
    #define	ALARM_CODE_FEED2_DOUBLE_BILL		    15					/* RC-Quad : feed2 double bill check	*/
    #define	ALARM_CODE_FEED1_SHORT_BILL			16					/* RC-Twin : feed1 short bill check		*/
    #define	ALARM_CODE_FEED2_SHORT_BILL			17					/* RC-Quad : feed2 short bill check		*/

    #define	ALARM_CODE_TWIN_DRUM1_TIMEOUT		18					/* RC-Twin : drum1 timeout				*/
    #define	ALARM_CODE_TWIN_DRUM2_TIMEOUT		19					/* RC-Twin : drum2 timeout				*/
    #define	ALARM_CODE_QUAD_DRUM1_TIMEOUT		20					/* RC-Quad : quad drum1 timeout			*/
    #define	ALARM_CODE_QUAD_DRUM2_TIMEOUT		21					/* RC-Quad : quad drum2 timeout			*/
    #define	ALARM_CODE_DISCONNECT_TWIN			22					/* RC-Twin : disconnected				*/
    #define	ALARM_CODE_DISCONNECT_QUAD			23					/* RC-Quad : disconnected				*/
    #define	ALARM_CODE_TWIN_DRUM1_MOTOR_LOCK	    24					/* RC-Twin : drum1 motor lock			*/
    #define	ALARM_CODE_TWIN_DRUM2_MOTOR_LOCK	    25					/* RC-Twin : drum2 motor lock			*/
    #define	ALARM_CODE_QUAD_DRUM1_MOTOR_LOCK	    26					/* RC-Quad : drum1 motor lock			*/
    #define	ALARM_CODE_QUAD_DRUM2_MOTOR_LOCK	    27					/* RC-Quad : drum2 motor lock			*/
    #define	ALARM_CODE_TWIN_DRUM1_JAM			28					/* RC-Twin : drum1 jam					*/
    #define	ALARM_CODE_TWIN_DRUM2_JAM			29					/* RC-Twin : drum2 jam					*/
    #define	ALARM_CODE_QUAD_DRUM1_JAM			30					/* RC-Quad : drum1 jam					*/
    #define	ALARM_CODE_QUAD_DRUM2_JAM			31					/* RC-Quad : drum2 jam					*/
    #define	ALARM_CODE_TWIN_DRUM1_FULL			32					/* RC-Twin : drum1 full					*/
    #define	ALARM_CODE_TWIN_DRUM2_FULL			33					/* RC-Twin : drum2 full					*/
    #define	ALARM_CODE_QUAD_DRUM1_FULL			34					/* RC-Quad : drum1 full					*/
    #define	ALARM_CODE_QUAD_DRUM2_FULL			35					/* RC-Quad : drum2 full					*/
    #define	ALARM_CODE_TWIN_DRUM1_EMPTY			36					/* RC-Twin : drum1 empty				*/
    #define	ALARM_CODE_TWIN_DRUM2_EMPTY			37					/* RC-Twin : drum2 empty				*/
    #define	ALARM_CODE_QUAD_DRUM1_EMPTY			38					/* RC-Quad : drum1 empty				*/
    #define	ALARM_CODE_QUAD_DRUM2_EMPTY			39					/* RC-Quad : drum2 empty				*/
    #define	ALARM_CODE_TWIN_DRUM1_SPEED_SEL		40					/* RC-Twin : drum1 spped select			*/
    #define	ALARM_CODE_TWIN_DRUM2_SPEED_SEL		41					/* RC-Twin : drum2 spped select			*/
    #define	ALARM_CODE_QUAD_DRUM1_SPEED_SEL		42					/* RC-Quad : drum1 spped select			*/
    #define	ALARM_CODE_QUAD_DRUM2_SPEED_SEL		43					/* RC-Quad : drum2 spped select			*/

    #define	ALARM_CODE_FLAP1_TIMEOUT			    44					/* RC-Twin : flapper1 timeout			*/
    #define	ALARM_CODE_FLAP2_TIMEOUT			    45					/* RC-Quad : flapper2 timeout			*/
    #define	ALARM_CODE_FLAP1_LEVER_FAIL			46					/* RC-Twin : flapper1 lever fail		*/
    #define	ALARM_CODE_FLAP2_LEVER_FAIL			47					/* RC-Quad : flapper2 lever fail		*/
    #define	ALARM_CODE_FLAP1_MOTOR_LOCK			48					/* RC-Twin : flapper1 lmotor lock		*/
    #define	ALARM_CODE_FLAP2_MOTOR_LOCK			49					/* RC-Quad : flapper2 lmotor lock		*/
    #define	ALARM_CODE_ILLIGAL_COMMAND			50					/* Illigal command						*/

    #define	ALARM_CODE_TWIN_DRUM1_JAM_IN		    51					/* RC-Twin : drum1 jam					*/
    #define	ALARM_CODE_TWIN_DRUM2_JAM_IN		    52					/* RC-Twin : drum2 jam					*/
    #define	ALARM_CODE_QUAD_DRUM1_JAM_IN		    53					/* RC-Quad : drum1 jam					*/
    #define	ALARM_CODE_QUAD_DRUM2_JAM_IN		    54					/* RC-Quad : drum2 jam					*/

	//RS対応で下記を追加
	#define	ALARM_CODE_RFID_UNIT				55	/* 55 : RFID unit			*/// mainとかぶっているので検討必要　こっちはRTQの通信で受信データのエラー詳細
	#define	ALARM_CODE_RFID_ICB_COMMUNICTION	56	/* 56 : RFID communication	*/// mainとかぶっているので検討必要 こっちはRTQの通信で受信データのエラー詳細

	/* RS flapper motor Error */
	#define ALARM_CODE_RS_FLAP_TIMEOUT          57                  /* 57 : RS : flapper timeout    */
	#define ALARM_CODE_RS_FLAP_LEVER_FAIL       58                  /* 58 : RS : flapper lever fail */
	#define ALARM_CODE_RS_FLAP_MOTOR_LOCK       59                  /* 59 : RS : flapper motor lock */
	#define	ALARM_CODE_HEAD_DETECT_ERROR		60					/* 60 : Head detect error				*/

#endif

#define	ALARM_CODE_ILLIGAL_OPERATION		    99					/* Illigal operation					*/


#endif
