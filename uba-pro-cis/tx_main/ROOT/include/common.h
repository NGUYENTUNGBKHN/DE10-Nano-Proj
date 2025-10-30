/*
 * common.h
 *
 *  Created on: 2018/01/25
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_INCLUDE_COMMON_H_
#define SRC_INCLUDE_COMMON_H_

#include "struct.h"
#include "typedefine.h"
#include "architecture.h"

#include "hwlib.h"
#include "alt_interrupt_common.h"

#include "debug.h"

/**
 * @brife 割込みタイプ
**/
typedef enum {
	IRQ_LEVEL_HIGH = 0,
	IRQ_EDGE_RISING = 1,
} irq_type_t;
/*----------------------------------------------------------*/
/*			Setting enable/disable value					*/
/*----------------------------------------------------------*/
#define ENABLE	1
#define DISABLE	0

/*----------------------------------------------------------*/
/*			Return value									*/
/*----------------------------------------------------------*/
#define RET_OK	0
#define RET_NG	1

/*----------------------------------------------------------*/
/*			BASIC ASCII										*/
/*----------------------------------------------------------*/
#define ENQ		0x05		/*	Enquiry 				*/
#define ACK		0x06		/*	Acknowledge 			*/
#define NAK		0x15		/*	Negative Acknowledge 	*/


// Section Header Symbol
#define JCM_PRODUCT_ID		0x011B
#define BA_SYMBOL		"VFM21   "

// Endian swap
#define SWAP_ENDIAN(adr)	( (((u32)(adr) & 0xff000000) >> 24) | (((u32)(adr) & 0xff0000) >> 8) | (((u32)(adr) & 0xff00) << 8) | (((u32)(adr) & 0xff) << 24) )
#define SWAP_ENDIAN32(val)	( (((u32)(val) & 0xff000000) >> 24) | (((u32)(val) & 0xff0000) >> 8) | (((u32)(val) & 0xff00) << 8) | (((u32)(val) & 0xff) << 24) )
#define SWAP_ENDIAN16(val)	( (((u16)(val) & 0xff00) >> 8) | (((u16)(val) & 0xff) << 8) )

/*==========================================================*/
/*			Task wait time                                  */
/*==========================================================*/
#define TASK_WAIT_TIME			20	/* 20msec */
#define TASK_WAIT_TIME_DISPLAY	10	/* 10msec */
#define TASK_WAIT_TIME_BEZEL	100	/* 100msec */
#define TASK_WAIT_TIME_TIMER	10	/* 10msec */
#define TASK_WAIT_TIME_DLINE	2	/*  2msec */
#define TASK_WAIT_TIME_MGU		1000	/*  1000msec */
#define TASK_WAIT_TIME_DIP		200	/*  1000msec */
#define TASK_WAIT_TIME_SIDE		200	/* 200msec */
#define TASK_WAIT_TIME_SENSOR	2	/* 2msec */
#define TASK_WAIT_TIME_SIDE_ACTIVE		1	/* 1msec */

/*----------------------------------------------------------*/
/*			Mode											*/
/*----------------------------------------------------------*/
#if defined(UBA_RTQ)
	#define	MODE1_RCINIT				0xA0
	enum _RCINIT_MODE2
	{
		RCINIT_MODE2_CPU_RESET = 1,			//   1
		RCINIT_MODE2_FW_CHECK,				//   2
		RCINIT_MODE2_ENCRYPTION_INIT,		//   3
		RCINIT_MODE2_WAIT_SETTING,			//   4
		RCINIT_MODE2_DL_START = 5,			//   5
		RCINIT_MODE2_DL_DATA,				//   6
		RCINIT_MODE2_DL_END,				//   7
		RCINIT_MODE2_BOX_REMOVED_CHECK,		//	 8
		RCINIT_MODE2_PREFEED_STACK,			//   9
	};
#endif

#define MODE1_POWERUP					0x01
enum _POWERUP_MODE2
{
	POWERUP_MODE2_FRAM_READ = 1,
	POWERUP_MODE2_CLEAR_ICB_SETTING,
	POWERUP_MODE2_MGU_READ,
	POWERUP_MODE2_DIPSW_INIT,
	POWERUP_MODE2_SENSOR_INIT,
	POWERUP_MODE2_WAIT_REQ,
	POWERUP_MODE2_SENSOR_ACTIVE,
	POWERUP_MODE2_STACKER_HOME,
	POWERUP_MODE2_SEARCH_BILL,
	POWERUP_MODE2_ALARM_BOX,
	POWERUP_MODE2_ALARM_CONFIRM_BOX,
	POWERUP_MODE2_MAG_INIT,
	//#if !defined(UBA_RTQ)//#if defined(GLI) 2023-12-04
	POWERUP_MODE2_ALARM_RECEIVE_RESET_GLI, //not use RTQ
	//#endif
#if defined(UBA_RTQ)
	POWERUP_MODE2_RC_SENSOR_ACTIVE,
	POWERUP_MODE2_RC_SEARCH_BILL,
	POWERUP_MODE2_ALARM_RC_UNIT,
	POWERUP_MODE2_ALARM_CONFIRM_RC_UNIT,
#endif // UBA_RTQ
};


#define MODE1_INIT						0x02
enum _INIT_MODE2
{
	INIT_MODE2_SENSOR_ACTIVE = 1,
	INIT_MODE2_CIS_INITIALIZE, //2
	INIT_MODE2_STACKER,				//3
	INIT_MODE2_FEED,				//4
	INIT_MODE2_CENTERING,			//5
	INIT_MODE2_APB,					//6
	INIT_MODE2_ICB,					//7

	INIT_MODE2_WAIT_REQ,			//8
	INIT_MODE2_WAIT_REMAIN_REQ,		//9
	INIT_MODE2_FORCE_FEED_STACK,	//10 0x0A
	INIT_MODE2_FORCE_STACK,			//11 0x0B
	INIT_MODE2_WAIT_REJECT_REQ,		//12 not use  0x0C
	INIT_MODE2_FEED_REJECT,			//13  0x0D
	INIT_MODE2_NOTE_STAY,			//14  0x0E
	INIT_MODE2_WAIT_RESET_REQ,		//15  0x0F
	INIT_MODE2_REJECT_APB_HOME,		//16  0x10

	INIT_MODE2_SHUTTER,	//17  0x11
	INIT_MODE2_INITIAL_POSITION,	//18  0x12

#if defined(UBA_RTQ)
	//ならびはUBA500と異なっているが
	INIT_MODE2_RC,
	INIT_MODE2_RC_WAIT_LAST_FEED,
	INIT_MODE2_RC_WAIT_RECOVERY_DRUM_GAP_ADJ,
	INIT_MODE2_RC_WAIT_RECOVERY_BACK,
	INIT_MODE2_RC_WAIT_RECOVERY_STACK_HOME,
	INIT_MODE2_RC_WAIT_RECOVERY_INIT_RC,
	INIT_MODE2_RC_WAIT_RECOVERY_BACK_BOX,
	INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM,
	INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM_FOR_COLLECT,
	INIT_MODE2_RC_WAIT_RECOVERY_FRONT_BACK_BOX__FRONT,
	INIT_MODE2_RC_WAIT_RECOVERY_FRONT_BACK_BOX__INIT_RC1,
	INIT_MODE2_RC_WAIT_RECOVERY_PAYDRUM_BOX__PAYDRUM_BOX,
	INIT_MODE2_RC_WAIT_RECOVERY_BOX_SEARCH_BOX__BOX_SEARCH,
	INIT_MODE2_RC_WAIT_RECOVERY_BOX_SEARCH_BOX__BOX,
	INIT_MODE2_RC_WAIT_RECOVERY_STACK,
	INIT_MODE2_RC_WAIT_RECOVERY_BILL_BACK,
	INIT_MODE2_RC_WAIT_RECOVERY_BILL_BACK_FOR_COLLECT,
#endif // UBA_RTQ
};

#define MODE1_DISABLE					0x03
enum _DISABLE_MODE2
{
	DISABLE_MODE2_WAIT_REQ = 1,
	DISABLE_MODE2_WAIT_REJECT_REQ,
};

#define MODE1_ENABLE					0x04
enum _ENABLE_MODE2
{
	ENABLE_MODE2_WAIT_BILL_IN = 1,
	ENABLE_MODE2_WAIT_REQ,
	ENABLE_MODE2_WAIT_REJECT_REQ,
};

#define MODE1_ACTIVE_DISABLE					0x13
enum _ACTIVE_DISABLE_MODE2
{
	ACTIVE_DISABLE_MODE2_WAIT_REJECT_REQ = 1,
	ACTIVE_DISABLE_MODE2_STACKER_HALF,
	ACTIVE_DISABLE_MODE2_CENTERING_HOME,
	ACTIVE_DISABLE_MODE2_SHUTTER_OPEN,
	ACTIVE_DISABLE_MODE2_FEED_REJECT,
	ACTIVE_DISABLE_MODE2_PB_CLOSE,
};

#define MODE1_ADJUST					0x12
enum _ADJUST_MODE2
{
	ADJUST_MODE2_DISABLE_TEMP_ADJ,
	ADJUST_MODE2_ENABLE_TEMP_ADJ,
};

#define MODE1_ACTIVE_ENABLE					0x14
enum _ACTIVE_ENABLE_MODE2
{
	ACTIVE_ENABLE_MODE2_WAIT_REQ = 1,
	ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ,
	ACTIVE_ENABLE_MODE2_STACKER_HALF,

	ACTIVE_ENABLE_MODE2_CENTERING_HOME,
	ACTIVE_ENABLE_MODE2_SHUTTER_OPEN,
	ACTIVE_ENABLE_MODE2_PB_CLOSE,
};

#define MODE1_ACCEPT					0x05
enum _ACCEPT_MODE2
{
	ACCEPT_MODE2_SENSOR_ACTIVE = 1,
	ACCEPT_MODE2_PB_CENTERING_HOME,
	ACCEPT_MODE2_FEED_CENTERING,
	ACCEPT_MODE2_CENTERING,
	ACCEPT_MODE2_FEED_ESCROW,
	ACCEPT_MODE2_DISCRIMINATION,
	ACCEPT_MODE2_WAIT_REQ,
	ACCEPT_MODE2_WAIT_REJECT_REQ,
	ACCEPT_MODE2_WAIT_PB_START,
	ACCEPT_MODE2_WAIT_FEED_START,
	ACCEPT_MODE2_WAIT_ACCEPT_REQ,
#if defined(UBA_RTQ)
	ACCEPT_MODE2_WAIT_RC_RSP,
	ACCEPT_MODE2_INIT_RC,
#endif // UBA_RTQ
};

#define MODE1_STACK						0x06
enum _STACK_MODE2
{
	STACK_MODE2_WAIT_STACK_START = 1, 		// 押しメカ起動待ち
	STACK_MODE2_WAIT_STACK_TOP,
	STACK_MODE2_STACK_HOME,
	//2023-08-10 入口の再確認は止める STACK_MODE2_STACK_HOME_WAIT_NOTE,	//入口センサONを監視
	STACK_MODE2_WAIT_REQ_FIRST,			//連続動作のLINE OK待ち
	STACK_MODE2_STACK_RETRY,
	STACK_MODE2_WAIT_REQ,
	STACK_MODE2_WAIT_REJECT_REQ,
	STACK_MODE2_WAIT_REJECT_SHUTTER_OPEN,//#if defined(LOW_POWER_3)
#if !defined(UBA_RTQ)
	STACK_MODE2_FEED_STACK,			//only SS, not use RTQ
	STACK_MODE2_WAIT_SHUTTER_OPEN_LD_MODE,
	STACK_MODE2_FEED_REV_REQ,	//only SS, not use RTQ //#if defined(HIGH_SECURITY_MODE)
	STACK_MODE2_FEED_STACK_LOW, //2022-12-23
#else //RTQ
	STACK_MODE2_FEED_RC_STACK,	//RTQ搬送 UBA500から変えた、一部処理を STACK_MODE2_RC_FEED_BOX へ
	STACK_MODE2_RC_FEED_BOX,	//BOX搬送( STACK_MODE2_FEED_RC_STACK を分離)
	STACK_MODE2_RC_PREFEED_STACK,
	STACK_MODE2_RC_RETRY_REV,
	STACK_MODE2_RC_RETRY_STACK_HOME,
	STACK_MODE2_RC_RETRY_INIT_RC,
	STACK_MODE2_RC_RETRY_FEED_BOX,
	STACK_MODE2_RC_RETRY_FEED_BOX_REV,
#endif // UBA_RTQ

#if (DATA_COLLECTION_DEBUG==1)
	STACK_MODE2_FEED_DATA_COLLECTION,
#endif

};

#define MODE1_REJECT					0x07
enum _REJECT_MODE2
{
	REJECT_MODE2_SENSOR_ACTIVE = 1,
	REJECT_MODE2_FEED_REJECT,
	REJECT_MODE2_STACKER_HALF_PB_CLOSE,
	REJECT_MODE2_WAIT_WID,
	REJECT_MODE2_WAIT_REQ,
	REJECT_MODE2_NOTE_REMOVED_WAIT_SENSOR_ACTIVE,
};
#define MODE1_REJECT_STANDBY			0x17
enum _REJECT_STANDBY_MODE2
{
	REJECT_STANDBY_MODE2_NOTE_STAY = 1,
	REJECT_STANDBY_MODE2_WAIT_REJECT_REQ,
	REJECT_STANDBY_MODE2_CONFIRM_NOTE_STAY,
};


#define MODE1_PAYOUT_STANDBY			0x18
enum _PAYOUT_STANDBY_MODE2
{
	PAYOUT_STANDBY_MODE2_NOTE_STAY = 1,
	PAYOUT_STANDBY_MODE2_WAIT_REQ,
	PAYOUT_STANDBY_MODE2_WAIT_PAYOUT_HOLD,
};

#define MODE1_PAYOUT_HANGING			0x28
enum _PAYOUT_HANGING_MODE2
{
	PAYOUT_HANGING_MODE2_SENSOR_ACTIVE = 1,
	PAYOUT_HANGING_MODE2_FEED,
	PAYOUT_HANGING_MODE2_APB,
};
#define MODE1_PAYOUT_COLLECT			0x38
enum _PAYOUT_COLLECT_MODE2
{
	PAYOUT_COLLECT_MODE2_SENSOR_ACTIVE = 1,
	PAYOUT_COLLECT_MODE2_FEED,
	PAYOUT_COLLECT_MODE2_APB,
	PAYOUT_COLLECT_MODE2_WAIT_REQ,
};
#define MODE1_PAYOUT_HANGING_STANDBY	0x48
enum _PAYOUT_HANGING_STANDBY_MODE2
{
	PAYOUT_HANGING_STANDBY_MODE2_NOTE_STAY = 1,
	PAYOUT_HANGING_STANDBY_MODE2_WAIT_REQ,
};

#if defined(UBA_RTQ)
// ***** MODE1_COLLECT ***** //
#define	MODE1_COLLECT					0x49
enum _COLLECT_MODE2
{
	COLLECT_MODE2_SENSOR_ACTIVE = 0x01,			//   1
	COLLECT_MODE2_INIT_TRANSPORT,				//   2
	COLLECT_MODE2_INIT_RC,						//   3
	COLLECT_MODE2_WAIT_RC_RSP,					//   4
	COLLECT_MODE2_WAIT_STACK_START = 0x10,		//  16
	COLLECT_MODE2_WAIT_STACK_TOP,				//  17
	COLLECT_MODE2_STACK_HOME,					//  18
	COLLECT_MODE2_STACK_RETRY,					//  19
	COLLECT_MODE2_WAIT_REQ = 0x20,				//  32
	COLLECT_MODE2_RC_PREFEED_STACK = 0x30,		//  48
	COLLECT_MODE2_RC_RETRY_FWD,					//  49
	COLLECT_MODE2_RC_RETRY_REV,					//  50
	COLLECT_MODE2_RC_RETRY_STACK_HOME,			//  51
	COLLECT_MODE2_RC_RETRY_INIT_RC,				//  52
	COLLECT_MODE2_RC_RETRY_FEED_BOX,			//  53
};
#endif // UBA_RTQ


#define MODE1_ALARM						0x0F
enum _ALARM_MODE2
{
	ALARM_MODE2_WAIT_REQ = 1,
	ALARM_MODE2_STACKER_FULL = 0x10,
	ALARM_MODE2_STACKER_JAM = 0x20,
	ALARM_MODE2_LOST_BILL = 0x30,
	ALARM_MODE2_STACKER_FAIL = 0x40,
	ALARM_MODE2_FEED_SPEED = 0x50,
	ALARM_MODE2_FEED_FAIL = 0x60,

	ALARM_MODE2_PUSHER_HOME = 0x70,

	ALARM_MODE2_ACCEPTOR_JAM = 0x80,
	ALARM_MODE2_CONFIRM_AT_JAM,
	ALARM_MODE2_ACCEPTOR_JAM_CIS,	//2024-06-09

	ALARM_MODE2_APB_FAIL = 0x90,
	ALARM_MODE2_SHUTTER_FAIL = 0x91, //2023-01-19

	ALARM_MODE2_BOX = 0xA0,
	ALARM_MODE2_CONFIRM_BOX,
	ALARM_MODE2_CHEAT= 0xC0,
	ALARM_MODE2_CENTERING_FAIL = 0xD0,

	ALARM_MODE2_WAIT_SENSOR_ACTIVE = 0xE1,	// ポジションセンサのDA再設定待ちに使用
	ALARM_MODE2_RFID,
	ALARM_MODE2_FRAM,
	ALARM_MODE2_MAG,
	ALARM_MODE2_UV,
	ALARM_MODE2_I2C,
	ALARM_MODE2_TMP_I2C,
	ALARM_MODE2_SPI,
	ALARM_MODE2_PL_SPI,
	ALARM_MODE2_CISA_OFF,
	ALARM_MODE2_CISB_OFF,
	ALARM_MODE2_CIS_ENCODER,
	ALARM_MODE2_CIS_TEMPERATURE,

#if defined(UBA_RTQ)
	ALARM_MODE2_RC_ERROR,
	ALARM_MODE2_CONFIRM_RC_ERROR,
	ALARM_MODE2_CONFIRM_RC_UNIT,
#endif
};

#define MODE1_ACTIVE_ALARM						0x1F
enum _ACTIVE_ALARM_MODE2
{
	ACTIVE_ALARM_MODE2_ICB_ERROR_RSP = 1,
	ACTIVE_ALARM_MODE2_TMP_READ //2024-05-28
};

#define MODE1_TEST_STANDBY				0xE0
enum _TEST_STANDBY_MODE2
{
	TEST_STANDBY_MODE2_SENSOR_INIT = 1,
	TEST_STANDBY_MODE2_STANDBY,
	TEST_STANDBY_MODE2_AGING_INIT_WAIT,
	TEST_STANDBY_MODE2_WAIT,
	TEST_STANDBY_MODE2_ALARM,
	TEST_STANDBY_MODE2_GD_IDENTIFIER_CLEAR,
	TEST_STANDBY_MODE2_DIPSW,
	TEST_STANDBY_MODE2_MAG_WRITE
};
#define MODE1_TEST_ACTIVE				0xF0
enum _TEST_ACTIVE_MODE2
{
	TEST_ACTIVE_MODE2_SENSOR_ACTIVE,		/* 0 */
	TEST_ACTIVE_MODE2_FEED_MOTOR_FWD,		/* 1 */
	TEST_ACTIVE_MODE2_FEED_MOTOR_REV,		/* 2 */
	TEST_ACTIVE_MODE2_APB,					/* 5 */

	TEST_ACTIVE_MODE2_CENTERING_OPEN_UBA,		/* 9 */
	TEST_ACTIVE_MODE2_CENTERING_CLOSE_UBA,		/* 10 */

	TEST_ACTIVE_MODE2_SENSOR,					/* 11 */

	TEST_ACTIVE_MODE2_AGING_INIT,				/* 13 */
	TEST_ACTIVE_MODE2_AGING_SENSOR_ACTIVE,		/* 14 */
	TEST_ACTIVE_MODE2_AGING_CIS_INITIALIZE,		/* 15 */
	TEST_ACTIVE_MODE2_AGING_FEED_CENTERING,		/* 16 */
	TEST_ACTIVE_MODE2_AGING_CENTERING,			/* 17 */
	TEST_ACTIVE_MODE2_AGING_FEED_ESCROW,		/* 18 */
	TEST_ACTIVE_MODE2_AGING_FEED_APB,			/* 19 */

	TEST_ACTIVE_MODE2_AGING_APB_CLOSE,			/* 21 */

	TEST_ACTIVE_MODE2_AGING_STACKER_HOME,		/* 23 */
	TEST_ACTIVE_MODE2_AGING_STACKER,			/* 24 */

	TEST_ACTIVE_MODE2_STACKER_MOTOR_FWD,		/* 26 */
//	TEST_ACTIVE_MODE2_STACKER_MOTOR_REV,		/* 27 */
	TEST_ACTIVE_MODE2_STACKER,					/* 28 */
	TEST_ACTIVE_MODE2_STACKER_HOME,				/* 29 */

	TEST_ACTIVE_MODE2_STACKER_RETRY,			//2024-04-09

	TEST_ACTIVE_MODE2_SHUTTER,					/* 34 */
	TEST_ACTIVE_MODE2_SHUTTER_CLOSE,			/* 35 */
	TEST_ACTIVE_MODE2_AGING_SHUTTER_CLOSE,		/* 36 *//* use */
	TEST_ACTIVE_MODE2_AGING_SHUTTER_OPEN,		/* 37 *//* use */
	TEST_ACTIVE_MODE2_RFID_UBA,					/* 38 */
	TEST_ACTIVE_MODE2_AGING_APB_OPEN,
#if defined(UBA_RTQ)
	TEST_ACTIVE_MODE2_COMMUNICATION,					/* 40 */
	TEST_ACTIVE_MODE2_DIPSW,							/* 41 */
	TEST_ACTIVE_MODE2_SW_LED,							/* 42 */

	TEST_ACTIVE_MODE2_TWIN_FEED_FWD,					/* 43 */
	TEST_ACTIVE_MODE2_TWIN_FEED_REV,					/* 44 */
	TEST_ACTIVE_MODE2_TWIN_FLAP,						/* 45 */
	TEST_ACTIVE_MODE2_TWIN_POS_SEN1,						/* 46 */
	TEST_ACTIVE_MODE2_TWIN_POS_SEN2,						/* 47 */
	TEST_ACTIVE_MODE2_TWIN_SOL,						/* 48 */
	TEST_ACTIVE_MODE2_TWIN_DRUM1,						/* 49 */
	TEST_ACTIVE_MODE2_TWIN_DRUM2,						/* 50 */
	
	TEST_ACTIVE_MODE2_QUAD_FEED_FWD,					/* 51 */
	TEST_ACTIVE_MODE2_QUAD_FEED_REV,					/* 52 */
	TEST_ACTIVE_MODE2_QUAD_FLAP,						/* 53 */
	TEST_ACTIVE_MODE2_QUAD_POS_SEN1,					/* 54 */
	TEST_ACTIVE_MODE2_QUAD_POS_SEN2,					/* 55 */
	TEST_ACTIVE_MODE2_QUAD_SOL,						/* 56 */
	TEST_ACTIVE_MODE2_QUAD_DRUM1,						/* 57 */
	TEST_ACTIVE_MODE2_QUAD_DRUM2,						/* 58 */

	TEST_ACTIVE_MODE2_WRITE_SERIALNO,					/* 59 */
	TEST_ACTIVE_MODE2_READ_SERIALNO,					/* 60 */
	TEST_ACTIVE_MODE2_START_SENS_ADJ,					/* 61 */
	TEST_ACTIVE_MODE2_READ_SENS_ADJ_DATA,				/* 62 */
	TEST_ACTIVE_MODE2_TWIN_FLAP_USB,					/* 63 */
	TEST_ACTIVE_MODE2_QUAD_FLAP_USB,					/* 64 */
	TEST_ACTIVE_MODE2_DRUM1_TAPE_POS_ADJ,				/* 65 */
	TEST_ACTIVE_MODE2_DRUM2_TAPE_POS_ADJ,				/* 66 */
	TEST_ACTIVE_MODE2_FRAM_CHECK,						/* 67 */
	TEST_ACTIVE_MODE2_SENS_ADJ_WRITE_FRAM,				/* 68 */
	TEST_ACTIVE_MODE2_SENS_ADJ_READ_FRAM,				/* 69 */
	TEST_ACTIVE_MODE2_PERFORM_TEST_WRITE_FRAM,			/* 70 */
	TEST_ACTIVE_MODE2_PERFORM_TEST_READ_FRAM,			/* 71 */
	TEST_ACTIVE_MODE2_WRITE_EDITIONNO,					/* 72 */
	TEST_ACTIVE_MODE2_READ_EDITIONNO,					/* 73 */
	TEST_ACTIVE_MODE2_RC_WAIT_SENSOR_ADJ,				/* 74 */// RC_BOARD_GREEN
#endif //  UBA_RTQ
//#if defined(UBA_RTQ_ICB)
	TEST_ACTIVE_MODE2_RC_RS,							/* 75 */
	TEST_ACTIVE_MODE2_RC_RFID,							/* 76 */
	//#if defined(UBA_RS)
	TEST_ACTIVE_MODE2_RC_RS_FLAP,						/* 77 */
	TEST_ACTIVE_MODE2_RC_RS_FLAP_USB,					/* 78 */
	TEST_ACTIVE_MODE2_RC_RS_SEN,						/* 79 */
	//#endif
//#endif
};

enum _TEST_NO
{
	TEST_STANDBY = 0,						/* standby */
	TEST_ACCEPT,							/* accept test */
	TEST_ACCEPT_LD,							/* accept test (ld mode) */
	TEST_ACCEPT_ALLACC,						/* accept test (all accept mode) */
	TEST_ACCEPT_LD_ALLACC,					/* accept test (ld & all accept mode) */
	TEST_REJECT,							/* reject test */
	TEST_REJECT_LD,							/* reject test (ld mode) */
	TEST_AGING,								/* aging test */
//	TEST_AGING_LD,							/* aging test (ld mode) */
	TEST_FEED_MOTOR_FWD,					/* feed motor forward test */
	TEST_FEED_MOTOR_REV,					/* feed motor reverse test */
	TEST_STACKER_MOTOR_FWD,					/* stacker motor forward test */
	TEST_STACKER,							/* stacker test */
	TEST_STACKER_HOME,						/* stacker test */
	TEST_APB,								/* anti pull back test */
	TEST_APB_CLOSE,								/* anti pull back test */
	TEST_CENTERING,							/* centering test */
	TEST_CENTERING_CLOSE,					/* centering test */
	TEST_SENSOR,							/* sensor test */
	TEST_DIPSW1,							/* DipSW1 test */

	TEST_SHUTTER,
	TEST_SHUTTER_CLOSE,
	TEST_RFID_UBA,
	TEST_REJECT_CENT_OPEN,					/* reject test (centering open)*///2024-12-01 生産CIS初期流動に使用

#if defined(UBA_RTQ)
	TEST_RC_AGING,							/* aging test for rc	*/
	TEST_RC_AGING_FACTORY,					/* aging test for rc	*/
	TEST_RC_SERIAL,							/* rc serial no			*/
	TEST_RC_DENOMI,							/* rc denomi setting	*/
	TEST_RC1,								/* commmon test for rc	*//* DIP 1, 2, 5 */
	TEST_RC1_USB,							/* commmon test for rc	*/
	TEST_RC2,								/* each test for rc		*//* DIP 1, 2, 6 */
	TEST_RC2_USB,							/* each test for rc		*/
	TEST_RC3_USB,							/* ship test for rc		*/
	TEST_RS_USB, //RC_BOARD_GREEN  
	TEST_RS,	//UBA_RTQ_ICB
#endif // UBA_RTQ
};

/*==========================================================*/
/*			Event flag                                      */
/*==========================================================*/
/*----- UART Control ----------------------------------*/
#define EVT_UART_RCV					0x00000002			/* UART Received */
#define EVT_UART_EMP					0x00000004			/* UART Trans empty */
#define EVT_UART_ERR					0x00000008			/* UART Communication err */
//#define EVT_UART_DISCON					0x00000010			/* UART Dis Connection err */

/*----- USB Control ----------------------------------*/
#define EVT_USB_CON						0x00000001			/* USB Connect/Disconnect */
#define EVT_USB_RCV						0x00000002			/* USB Received */
#define EVT_USB_EMP						0x00000004			/* USB Trans empty */

/*----- OTG Download control ----------------------------------*/
#define EVT_OTG_DOWNLOAD_READY			0x00000001			/* OTG download file ready */

/*----- Power Voltage control ----------------------------------*/
#define EVT_POWER_VOLTAGE				0x00000001			/* fram write log */
#define EVT_EXTERNAL_RESET				0x00000002			/* fram write log */
#define EVT_PLL_LOCK					0x00000004			/* fram write log */

/*----------------------------------------------------------*/
/*			Mode											*/
/*----------------------------------------------------------*/
#define MODE1_DOWNLOAD					0x01
enum _DOWNLOAD_MODE2
{
	DOWNLOAD_MODE2_WAIT_REQ = 1,
	DOWNLOAD_MODE2_IF,
	DOWNLOAD_MODE2_USB,
	DOWNLOAD_MODE2_SUBLINE_USB,
};

#if defined(UBA_RTQ)
	// ***** MODE1_PAYOUT ***** //
	#define MODE1_PAYOUT					0x08
	enum _PAYOUT_MODE2
	{
		PAYOUT_MODE2_SENSOR_ACTIVE = 0x01,			//   1
		PAYOUT_MODE2_INIT_TRANSPORT,				//   2
		PAYOUT_MODE2_INIT_RC,						//   3
		PAYOUT_MODE2_WAIT_RC_RSP,					//   4
		PAYOUT_MODE2_FEED_RC_PAYOUT,				//   5
		PAYOUT_MODE2_FEED_REJECT_STOP_WAIT_WID,		//	 6
		PAYOUT_MODE2_NOTE_STAY,						//   7
		PAYOUT_MODE2_WAIT_STACK_START = 0x10,		//  16
		PAYOUT_MODE2_WAIT_STACK_TOP,				//  17
		PAYOUT_MODE2_STACK_HOME,					//  18
		PAYOUT_MODE2_STACK_RETRY,					//  19
		PAYOUT_MODE2_DUMMY_FEED,					//	20 テストモードエージング時のみ
		PAYOUT_MODE2_WAIT_REQ = 0x20,				//  32
		PAYOUT_MODE2_RC_PREFEED_STACK = 0x30,		//  48
		PAYOUT_MODE2_RC_RETRY_FWD,					//  49
		PAYOUT_MODE2_RC_RETRY_REV,					//  50
		PAYOUT_MODE2_RC_RETRY_STACK_HOME,			//  51
		PAYOUT_MODE2_RC_RETRY_INIT_RC,				//  52
		PAYOUT_MODE2_RC_RETRY_FEED_BOX,				//  53
		//#if defined(ID003_SPECK64)
		PAYOUT_MODE2_RC_WAIT_PB_CLOSE
		//#endif
	};
#endif 

/* PERFORMANCE TEST MODE DIPSW */
#define DIPSW1_PERFORMANCE_TEST				0x80		/* performance test */
#define DIPSW1_FEED_MOTOR_FWD_TEST			0x01		/* feed motor forward test */
#define DIPSW1_FEED_MOTOR_REV_TEST			0x02		/* feed motor revers test */
#define DIPSW1_STACK_TEST					0x04		/* staking test */
#define DIPSW1_STACKER_MOTOR_FWD_TEST		0x05		/* stacker motor forward test */
#define DIPSW1_ACCEPT_LD_TEST				0x07		/* accept test (ld mode) */
#define DIPSW1_AGING_TEST					0x08		/* aging test */
//#define DIPSW1_AGING_LD_TEST				0x09		/* aging test (ld mode) */
#define DIPSW1_ACCEPT_TEST					0x0F		/* accept test */
#define DIPSW1_APB_TEST						0x10		/* anti pull back test */
#define DIPSW1_CENTERING_TEST				0x11		/* centering test */
#define DIPSW1_CENTERING_CLOSE_TEST 		0x15
#define DIPSW1_ACCEPT_LD_ALLACC_TEST		0x17		/* accept test (ld & all accept mode) */
#define DIPSW1_SHUTTER_CLOSE_TEST			0x21		/* Shutter test DIP 1,6にする*/
#define DIPSW1_ACCEPT_ALLACC_TEST			0x2F		/* accept test (all accept mode) */
#define DIPSW1_SHUTTER_TEST					0x30		/* Shutter test DIP 2,5は別で使用される可能性があるので、DIP 5,6にする*/
#define DIPSW1_REJECT_LD_TEST				0x37		/* reject test (ld mode) */
#if 0//
	#define DIPSW1_CCTALK_ADDRESS_INIT			0x3E		/* initialize ccTalk address */
	#define DIPSW1_CCTALK_ENCRYPTION_INIT		0x3F		/* initialize ccTalk encryption code */
#endif
#define DIPSW1_SENSOR_TEST					0x40		/* sensor test */
#define DIPSW1_APB_CLOSE_TEST				0x50		/* anti pull back test DIP-SW 5,7 */
#define DIPSW1_REJECT_TEST					0x5F		/* reject test */
#define DIPSW1_RFID_UBA_TEST				0x70		/* Shutter test DIP 5,6,7にする*///2023-03-16
#define DIPSW1_DIPSW1_TEST					0x7F		/* DipSW1 test */
#define DIPSW1_AUTHENTICATION_MODE			0xC3		/* authentication set mode */

#if defined(UBA_RTQ)
	#define	DIPSW1_RC_TEST1						0x13		/* test1 for RC */
	#define	DIPSW1_RC_TEST2						0x23		/* test2 for RC */
	//#if defined(UBA_RTQ_ICB)
	#define DIPSW1_RS_TEST						0x33		/* test rfid	*///RTQの生産で使用
	//#endif // 
	#define	DIPSW1_RC_SERIAL_STORE				0x3C		/* store RC serial no				*/
	#define	DIPSW1_RC_DENOMI					0x3D		/* recycle denomi(default) 			*/

	#define	DIPSW1_RC_AGING_TEST				0x4F		/* accept & payout aging test		*/
	#define	DIPSW1_RC_AGING_FACTORY				0x6F		/* accept & payout aging test(factory)	*/

	// RC dispsw //
	#define DIPSWRC_COMMUNICATION				0x00		/* communication between rc with header */
	#define DIPSWRC_SW_LED						0x10		/* display led */
	#define DIPSWRC_DIPSW						0x20		/* dipsw */
	#define	DIPSWRC_BTR_FEED_FWD				0x01		/* feed0 fwd test					*/
	#define	DIPSWRC_BTR_FEED_REV				0x02		/* feed0 rev test					*/
	#define	DIPSWRC_EXBOX_SOL					0x04		/* exbox sol test					*/

	#define	DIPSWRC_TWIN_FEED_FWD				0x01		/* feed fwd test(RC-Twin)			*/
	#define	DIPSWRC_TWIN_FEED_REV				0x02		/* feed rev test(RC-Twin)			*/
	#define	DIPSWRC_TWIN_FLAP					0x04		/* flapper test(RC-Twin)			*/
	#define	DIPSWRC_TWIN_SEN1					0x08		/* position sensor1 test(RC-Twin)	*/
	#define	DIPSWRC_TWIN_SEN2					0x09		/* position sensor2 test(RC-Twin)	*/
	#define	DIPSWRC_TWIN_SOL					0x10		/* sol test(RC-Twin)				*/
	#define	DIPSWRC_TWIN_DRUM1					0x20		/* drum1 test(RC-Twin)				*/
	#define	DIPSWRC_TWIN_DRUM2					0x40		/* drum2 test(RC-Twin)				*/

	#define	DIPSWRC_QUAD_FEED_FWD				0x81		/* feed fwd test(RC-Quad)			*/
	#define	DIPSWRC_QUAD_FEED_REV				0x82		/* feed rev test(RC-Quad)			*/
	#define	DIPSWRC_QUAD_FLAP					0x84		/* flapper test(RC-Quad)			*/
	#define	DIPSWRC_QUAD_SEN1					0x88		/* position sensor1 test(RC-Quad)	*/
	#define	DIPSWRC_QUAD_SEN2					0x89		/* position sensor2 test(RC-Quad)	*/
	#define	DIPSWRC_QUAD_SOL					0x90		/* sol test(RC-Quad)				*/
	#define	DIPSWRC_QUAD_DRUM1					0xA0		/* drum1 test(RC-Quad)				*/
	#define	DIPSWRC_QUAD_DRUM2					0xC0		/* drum2 test(RC-Quad)				*/

	#define DIPSWRC_WRITE_SERIAL_NO				0x01		/* write serial no					*/
	#define DIPSWRC_READ_SERIAL_NO				0x02		/* read serial no					*/
	#define DIPSWRC_START_SENS_ADJ				0x04		/* start sensor adj					*/
	#define DIPSWRC_GET_SENS_ADJ_DATA			0x08		/* get sesnor adj data				*/
	#define DIPSWRC_TWIN_FLAP_USB				0x10		/* flapper test USB(RC-Twin)		*/
	#define DIPSWRC_QUAD_FLAP_USB				0x20		/* flapper test USB(RC-Quad)		*/
	#define DIPSWRC_DRUM1_TAPE_POS_ADJ			0x40		/* drum1 tape position adj			*/
	#define DIPSWRC_DRUM2_TAPE_POS_ADJ			0x50		/* drum2 tape position adj			*/
	#define DIPSWRC_FRAM_CHECK					0x60		/* fram check						*/
	#define DIPSWRC_SENS_ADJ_WRITE_FRAM			0x61		/* sensor adj write fram			*/
	#define DIPSWRC_SENS_ADJ_READ_FRAM			0x62		/* sensor adj read fram				*/
	#define DIPSWRC_PERFORM_TEST_WRITE_FRAM		0x63		/* sensor adj write fram			*/
	#define DIPSWRC_PERFORM_TEST_READ_FRAM		0x64		/* sensor adj read fram				*/
	#define DIPSWRC_WRITE_EDITION_NO			0x70		/* write edition no					*/
	#define DIPSWRC_READ_EDITION_NO				0x71		/* read edition no					*/
	//#if defined(RC_BOARD_GREEN)
	#define	DIPSWRC_GET_POS_ONOFF				0x81
	#define	DIPSWRC_SET_POS_DA					0x82
	#define	DIPSWRC_SET_POS_GAIN				0x83
	//#endif
	//#if defined(UBA_RTQ_ICB)
	#define	DIPSWRC_RS_FLAP						0x04		/* flapper test(RS)					*/
	#define	DIPSWRC_RC_RFID						0x40		/* RFID test //生産で使用	背面DIP-SWの設定(7 ON)		*/
	#define	DIPSWRC_RS_SEN						0x08		/* position senor test(RS)			*///#if defined(UBA_RS)
	//#endif
#endif


/*----------------------------------------------------------*/
/*			Motor											*/
/*----------------------------------------------------------*/
enum _MOTOR_MODE
{
	MOTOR_STOP = 0x00,		/* motor stop */
	MOTOR_FWD,				/* motor forward direction */
	MOTOR_REV,				/* motor reverse direction */
	MOTOR_FREE,				/* motor free */
	MOTOR_BRAKE,			/* motor brake */
	MOTOR_BRAKE_FWD,		/* motor brake from forward *///use feed/stacker
	MOTOR_BRAKE_REV,		/* motor brake from reverse *///use feed/stacker
};

/* feed motor speed */
enum _MOTOR_SPEED
{
	FEED_MOTOR_SPEED_STOP = 0x00,	/* motor stop */
	FEED_MOTOR_SPEED_PWM,			/* motor speed PWM100% */
	FEED_MOTOR_SPEED_FULL,			/* motor speed PWM100% */
	FEED_MOTOR_SPEED_600MM,			/* motor speed 600mm/s */
#if defined(UBA_RTQ)
	FEED_MOTOR_SPEED_550M,			/* motor speed 550mm/s */
#endif // UBA_RTQ
	FEED_MOTOR_SPEED_300MM,			/* motor speed 300mm/s */
	FEED_MOTOR_SPEED_350MM,			/* motor speed 350mm/s */
};
#define TARGET_SPEED_CYCLE_600MM (21205)
#if defined(UBA_RTQ) //2025-09-26
//	#define TARGET_SPEED_CYCLE_5500MM (23133)
	#define TARGET_SPEED_CYCLE_5500MM (23560)	//540
//	#define TARGET_SPEED_CYCLE_5500MM (24005)	//530
#endif // UBA_RTQ
#define TARGET_SPEED_CYCLE_300MM (42411)
#define TARGET_SPEED_CYCLE_350MM (36351)
/* motor speed */
#define MOTOR_MAX_SPEED				100
#define MOTOR_MIN_SPEED				0

#define NEXT_MOTOR_START_WAIT_TIME	10			/* next motor start waiting time : 100msec */
//#define FEED_MOTOR_STOP_TIME		70			/* feed motor stop time : 70msec */
#define FEED_MOTOR_STOP_TIME		1			/* feed motor stop time : 70msec */
#define STACKER_MOTOR_STOP_TIME		1			/* stacker motor stop time : 70msec *//* 1msecでオシロ上は3.8msec */
#define APB_MOTOR_STOP_TIME			1			/* APB motor stop time : 70msec */
#define CENTERING_MOTOR_STOP_TIME	1			/* centering motor stop time : 70msec */
#define STACKER_NEXT_MOTOR_START_WAIT_TIME	70	/* next motor start waiting time : 180msec -> 120msec -> 70msec*///2023-10-19
#define SHUTTER_MOTOR_STOP_TIME		1			/* shutter motor stop time :  */

/*----------------------------------------------------------*/
/*			Status LED										*/
/*----------------------------------------------------------*/
enum DISP_PATTERN{
	STATUS_LED_OFF = 0,
	STATUS_LED_ON,
	STATUS_LED_BLINK
};

enum DISP_COLOR
{
	DISP_COLOR_OFF = 0,
	DISP_COLOR_RED,
	DISP_COLOR_GREEN,
	DISP_COLOR_RED_GREEN,
	DISP_COLOR_RED_OFF,
	DISP_COLOR_GREEN_OFF,
	DISP_COLOR_INIT, //2023-01-31
	DIPS_COLOR_MAX
} ;


/*----------------------------------------------------------*/
/*			Internal Error Code								*/
/*----------------------------------------------------------*/
enum _IERR_CODE
{
	IERR_CODE_OK = 0,		/*  */
	IERR_CODE_PARAM,		/* parameter error */
	IERR_CODE_BUSY,			/* busy */
};


/*----------------------------------------------------------*/
/*			Reject Code										*/
/*----------------------------------------------------------*/
enum _REJECT_CODE
{
	REJECT_CODE_OK = 0,
	REJECT_CODE_SKEW,								/* skew */
	REJECT_CODE_MAG_PATTERN,						/* mag pattern */
	REJECT_CODE_MAG_AMOUNT,							/* mag amount */
	REJECT_CODE_ACCEPTOR_STAY_PAPER,				/* acceptor stay paper */
	REJECT_CODE_XRATE,					/*  5 */	/* x-rate */
	REJECT_CODE_INSERT_CANCEL,						/* insert cancel */
	REJECT_CODE_FEED_SLIP,							/* feed slip */
	REJECT_CODE_FEED_MOTOR_LOCK,					/* feed motor lock */
	REJECT_CODE_FEED_TIMEOUT,						/* feed time out */
	REJECT_CODE_APB_HOME,				/* 10 */	/* apb home position error */
	REJECT_CODE_CENTERING_HOME,						/* centering home position error */
	REJECT_CODE_PRECOMP,							/* precompare */
	REJECT_CODE_PATTERN,							/* photo pattern */
	REJECT_CODE_PHOTO_LEVEL,						/* photo level */
	REJECT_CODE_INHIBIT,							/* inhibit */
	REJECT_CODE_ESCROW_TIMEOUT,			/* 20 */	/* escrow time out */
	REJECT_CODE_RETURN,								/* receive return */
	REJECT_CODE_OPERATION,							/* operation error */
	REJECT_CODE_LENGTH,								/* paper length out range */
	REJECT_CODE_PAPER_SHORT,			/* 25 */	/* paper too short */
	REJECT_CODE_PAPER_LONG,							/* paper too long */
	REJECT_CODE_SYNC,								/* sync errror */
	REJECT_CODE_DYENOTE, 							/* dye note */
	REJECT_CODE_HOLE,								/* hole note */
	REJECT_CODE_TEAR,					/* 30 */	/* tear note */
	REJECT_CODE_DOG_EAR,							/* dog ear */
	REJECT_CODE_COUNTERFEIT,						/* counterfeit */
	REJECT_CODE_FAKE_MCIR,							/* Rejected by a MCIR check */
	REJECT_CODE_FAKE_M3C,							/* Rejected by a M3C check */
	REJECT_CODE_FAKE_M4C,				/* 35 */	/* Rejected by a M4C check */
	REJECT_CODE_FAKE_IR,							/* Rejected by a IR check */
	REJECT_CODE_THREAD,								/* security thread */
	REJECT_CODE_LOST_BILL,							/* lost bill */
	REJECT_CODE_BOX,								/* box set error */
	REJECT_CODE_STACKER_HOME,						/* stacker pusher home position error */
	REJECT_CODE_STACKER_STAY_PAPER,					/* stacker stay paper */
	REJECT_CODE_STRING_DOWN,						/* string ad down */
	REJECT_CODE_STRING_UP,							/* string ad up */
	REJECT_CODE_SIDE_LOW_SENSITIVITY,				/* side sensor low sensitivity */
	REJECT_CODE_SIDE_HIGH_SENSITIVITY,				/* side sensor high sensitivity */
	REJECT_CODE_UV,									/* uv */
#if defined(UBA_RTQ)
	REJECT_CODE_NO_RECYCLE,							/* No Recycle */// 還流金種だが還流されない場合に使用する
	REJECT_CODE_NO_PAYOUT,							/* No Recycle */// 還流金種だが還流されない場合に使用する. UBA500RTQは調査用にJDLに作成したとこ事、UBA700はとりあえずカウントさせない
#endif
	/* BAR coupon : 0x40～ */
	REJECT_CODE_BAR_NC = 0x40,						/* [BAR] need configuration */
	REJECT_CODE_BAR_UN,					/* 65 */	/* [BAR] unknown code */
	REJECT_CODE_BAR_SH,								/* [BAR] under or over read */
	REJECT_CODE_BAR_ST,								/* [BAR] start bit missing */
	REJECT_CODE_BAR_SP,								/* [BAR] stop bit missing */
	REJECT_CODE_BAR_TP,								/* [BAR] type not enable */
	REJECT_CODE_BAR_XR,					/* 70 */	/* [BAR] x-rate */
	REJECT_CODE_BAR_PHV,							/* [BAR] photo level */
	REJECT_CODE_BAR_DIN,							/* [BAR] reverse */
	REJECT_CODE_BAR_LG,								/* [BAR] length out of range */
	REJECT_CODE_BAR_NG,								/* [BAR] invalid coupon */
	REJECT_CODE_BAR_MC,					/* 75 */	/* [BAR] setting coupon */
	/* TEST : 0xF0～ */
	REJECT_CODE_SAMPLING_MISSING = 0xF0,			/* [TEST] data collection sampling missing */
	REJECT_CODE_UNKNOWN
};
/*----------------------------------------------------------*/
/*			Alarm Code										*/
/*----------------------------------------------------------*/
enum _ALARM_CODE   //JDLでも使用しているので、追加、削除　禁止、使用しなくなっても残す
{
	ALARM_CODE_OK = 0,
/*  */
	/* Initialization-related */
	ALARM_CODE_FRAM,								/* FRAM error */
	ALARM_CODE_MAG,									/* MAG setting error */
	ALARM_CODE_I2C,									/* I2C communication error */
	ALARM_CODE_TMP_I2C,								/* I2C communication error */
	ALARM_CODE_SPI,									/* PS SPI communication error */
	ALARM_CODE_PL_SPI,								/* PL SPI communication error */
	ALARM_CODE_CISA_OFF,							/* CISA off */
	ALARM_CODE_CISB_OFF,							/* CISB off */
	ALARM_CODE_UV,									/* UV error */
	ALARM_CODE_CIS_ENCODER,							/* CIS encoder not count up */

	ALARM_CODE_STACKER_FULL,						/* stacker Full */
	ALARM_CODE_STACKER_MOTOR_LOCK,					/* stacker motor lock */
	ALARM_CODE_STACKER_GEAR,						/* stacker gear */
	ALARM_CODE_STACKER_TIMEOUT,						/* stacker time out */
	ALARM_CODE_STACKER_HOME,						/* stacker pusher home position error */

	ALARM_CODE_FEED_OTHER_SENSOR_SK,				/* other sensor ON (JAM : bill in stacker) */
	ALARM_CODE_FEED_SLIP_SK,				        /* feed slip (JAM : bill in stackr) */
	ALARM_CODE_FEED_TIMEOUT_SK,						/* feed time out (JAM : bill in stackr) */
	ALARM_CODE_FEED_LOST_BILL,						/* lost bill */ /* not use RTQ */
	ALARM_CODE_FEED_MOTOR_LOCK_SK,					/* feed motor lock (JAM : bill in stackr) */

	ALARM_CODE_FEED_OTHER_SENSOR_AT,				/* other sensor ON (acceptor JAM) */
	ALARM_CODE_FEED_SLIP_AT,						/* feed slip (acceptor JAM) */
	ALARM_CODE_FEED_TIMEOUT_AT,						/* feed time out (acceptor JAM) */
	ALARM_CODE_FEED_MOTOR_LOCK_AT,					/* feed motor lock (acceptor JAM) */

	ALARM_CODE_FEED_MOTOR_SPEED_LOW,				/* feed motor low speed */
	ALARM_CODE_FEED_MOTOR_SPEED_HIGH,		        /* feed motor high speed */

	ALARM_CODE_FEED_MOTOR_LOCK,						/* feed motor lock */

	ALARM_CODE_APB_TIMEOUT,							/* APB time out */
	ALARM_CODE_APB_HOME,							/* APB home position error */
	ALARM_CODE_APB_HOME_STOP,						/* APB home position stop error */

	ALARM_CODE_BOX,									/* BOX set error */
	ALARM_CODE_BOX_INIT,							/* BOX set error (Box not set when initialize) */

	ALARM_CODE_CHEAT,					        	/* cheat detection */

	ALARM_CODE_CENTERING_TIMEOUT,					/* centering time out */
	ALARM_CODE_CENTERING_HOME_STOP,					/* centering home position stop error */

	ALARM_CODE_SHUTTER_TIMEOUT,
	ALARM_CODE_SHUTTER_HOME_STOP,

//2025-01-16 RTQと名前が被っているので	ALARM_CODE_RFID_UNIT,					/* ICB有効でRFIDユニットがない場合 */
	ALARM_CODE_RFID_UNIT_MAIN,
	ALARM_CODE_RFID_ICB_SETTING, 			/* ICB無効でICBユニットがある場合 */

//2025-01-16 RTQと名前が被っているので		ALARM_CODE_RFID_ICB_COMMUNICTION, 		/* ICB読み込み、書込み失敗 */
	ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN, 		/* ICB読み込み、書込み失敗 */

	ALARM_CODE_RFID_ICB_DATA, 				/* ICBチェックサムデータ異常 */
	ALARM_CODE_RFID_ICB_NUMBER_MISMATCH,	/* ICB別のゲーム機にセットされていたBOXです */
	ALARM_CODE_RFID_ICB_NOT_INITIALIZE, 	/* ICB集計データセーブ済みのBOXです */
	ALARM_CODE_ICB_FORCED_QUIT,				/* ICBタスクのシーケンス異常						*/
	ALARM_CODE_RFID_ICB_MC_INVALID,	 		/* ICBマシンナンバー無効 */

	ALARM_CODE_EXTERNAL_RESET,
	ALARM_CODE_PLL_LOCK,
	ALARM_CODE_POWER_OFF,

	ALARM_CODE_FEED_FORCED_QUIT,				/* feed forced quit */
	ALARM_CODE_STACKER_FORCED_QUIT,				/* stacker forced quit */
	ALARM_CODE_APB_FORCED_QUIT,			        /* APB forced quit */
	ALARM_CODE_CENTERING_FORCED_QUIT,			/* centering forced quit */
	ALARM_CODE_SHUTTER_FORCED_QUIT,

	ALARM_CODE_CIS_TEMPERATURE,  //2024-05-28
	ALARM_CODE_FEED_CIS_AT,		//2024-06-09 ホスト通知専用なので、その他のタスク通知は通常のAcceptor JAM関係のまま
								//表示タスク、JDL,ICB関係は通常のAcceptor JAMのままなので、登録必要なし

#if defined(UBA_RTQ)
	ALARM_CODE_RC_ERROR = 0x70,
	ALARM_CODE_RC_ROM,				//UBA500もコードはあるが使用していない可能性あり
	ALARM_CODE_RC_REMOVED,
	ALARM_CODE_RC_COM,
	ALARM_CODE_RC_DWERR,
	ALARM_CODE_RC_POS,
	ALARM_CODE_RC_TRANSPORT,
	ALARM_CODE_RC_TIMEOUT,
	ALARM_CODE_RC_DENOMINATION,
	ALARM_CODE_RC_EMPTY,
	ALARM_CODE_RC_DOUBLE,
	ALARM_CODE_RC_FULL,
	ALARM_CODE_RC_EXCHAGED,
	ALARM_CODE_RC_FORCED_QUIT,	//not use		        	/* RC forced quit */
	//#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
	ALARM_CODE_RC_RFID,
	//#endif
#endif

};


/*----------------------------------------------------------*/
/*			Operating Mode									*/
/*----------------------------------------------------------*/
#define OPERATING_MODE_NORMAL				0x00		/* Normal Operation(I/F) Mode */
//#define OPERATING_MODE_LD					0x01		/* LD Mode */
#define OPERATING_MODE_TEST					0x04		/* Performance Test Mode */
#define OPERATING_MODE_COLLECTION			0x10		/* Data Collection Mode */
#define OPERATING_MODE_TEST_LD				0x20		/* [Test Mode] LD Mode */
#define OPERATING_MODE_TEST_ALL_ACCEPT		0x40		/* [Test Mode] All Accept Mode */
#define OPERATING_MODE_TEST_ALL_REJECT		0x80		/* [Test Mode] All Reject Mode */

/*----------------------------------------------------------*/
/*			Select Interface								*/
/*----------------------------------------------------------*/
enum{
	IF_SELECT_NONE = 0,					/* Reserved */
	IF_SELECT_TTL,					/* TTL */
	IF_SELECT_USB,					/* ID-008 USB */
};

/*----------------------------------------------------------*/
/*			Select Protocol									*/
/*----------------------------------------------------------*/
enum _PROTOCOL_SELECT
{
	PROTOCOL_SELECT_ID003   = 0x03,
	PROTOCOL_SELECT_ID008   = 0x08,
	PROTOCOL_SELECT_ID064GD = 0x64,
	PROTOCOL_SELECT_ID0E3   = 0xe3,
	PROTOCOL_SELECT_ID0G8 	= 0xF8,
	PROTOCOL_SELECT_TEST   = 0xFF,
};


/*----------------------------------------------------------*/
/*			MBX Message Code								*/
/*----------------------------------------------------------*/
/* Task Code */
#define TMSG_TCODE_MASK				0xFF00
#define TMSG_TCODE_MAIN				0x0100
#define TMSG_TCODE_DLINE			0x0200
#define TMSG_TCODE_CLINE			0x0300
#define TMSG_TCODE_FEED				0x0400
#define TMSG_TCODE_STAC				0x0500
#define TMSG_TCODE_DISC				0x0600
#define TMSG_TCODE_SENS				0x0700
#define TMSG_TCODE_TIME				0x0800
#define TMSG_TCODE_DISP				0x0900
#define TMSG_TCODE_CENT				0x0A00
#define TMSG_TCODE_APB				0x0B00
#define TMSG_TCODE_MOT				0x0C00
#define TMSG_TCODE_IDLE				0x0D00
#define TMSG_TCODE_SUBLINE			0x0E00
#define TMSG_TCODE_FRAM				0x1000
#if defined(UBA_RTQ)
	#define TMSG_TCODE_FUSB_DECT		0x1100
#endif // uBA_RTQ
#define TMSG_TCODE_UART01CB			0x1200
#define TMSG_TCODE_USB0CB			0x1300
#define TMSG_TCODE_USB2CB			0x1400
#define TMSG_TCODE_MGU				0x1500
#define TMSG_TCODE_SDC				0x1600
#define TMSG_TCODE_RFID				0x1700
#define TMSG_TCODE_ICB				0x1800
#define TMSG_TCODE_OTG				0x1900
#define TMSG_TCODE_DIPSW			0x1A00
#define TMSG_TCODE_TEST_SW			0x1B00
#define TMSG_TCODE_POWER			0x1C00
#define TMSG_TCODE_SDC_DET			0x1D00
//#define TMSG_TCODE_SIDE				0x1E00
#define TMSG_TCODE_SHUTTER			0x1F00

#define TMSG_TCODE_RC				0x2000
#define TMSG_TCODE_RC_UARTCB		0x2100

#define TMSG_MCODE_MASK				0x00FF

#define TMSG_COMMON_STATUS			0x0080

/* Common function Code */
typedef enum _TMSG_SUB
{
	TMSG_SUB_SUCCESS = 0,
	TMSG_SUB_START,
	TMSG_SUB_ACCEPT,
	TMSG_SUB_REJECT,
	TMSG_SUB_VEND,
	TMSG_SUB_PAUSE,
	TMSG_SUB_RESUME,
	TMSG_SUB_END_INIT_STACK,
	TMSG_SUB_PAYOUT,
	TMSG_SUB_COLLECT,
	TMSG_SUB_REMAIN,
	TMSG_SUB_INTERIM,			/* interim termination */

	TMSG_SUB_INIT_REJECT_REQUEST,	//2025-02-10
	TMSG_SUB_INIT_REJECT_HANGING,	//2025-02-10
	TMSG_SUB_INIT_REJECT_REMOVE,	//2025-02-10

	TMSG_SUB_CONNECT,
	TMSG_SUB_RECEIVE,
	TMSG_SUB_EMPTY,
	TMSG_SUB_DOWNLOAD,
	TMSG_SUB_ENABLE_NEXT,
	TMSG_SUB_EXIT_OUT,
	TMSG_SUB_RETRY_REQUEST,
	TMSG_SUB_STACKING_ENTRY_ON,
	TMSG_SUB_ACCEPTING,
	TMSG_SUB_WAIT,
	TMSG_SUB_FEED_REJECT_RETRY,
	TMSG_SUB_BUSY,
#if defined(UBA_RTQ)
	TMSG_SUB_START_TASK,
	TMSG_SUB_PAYSTAY,
	TMSG_SUB_PAYVALID,
	TMSG_SUB_PAYVALID_ERROR,	//通常動作時のみ使用に統一、パワーリカバリ時は使用しない
	TMSG_SUB_COLLECTED,
	TMSG_SUB_RC_REMAIN,
	TMSG_SUB_SEARCH_STILL_BOX,
	TMSG_SUB_RC_RESET,
	TMSG_SUB_RETURN_PAYOUT_NOTE,
	TMSG_SUB_RETURN_PAYOUT_COLLECTED,
	TMSG_SUB_DOWNLOAD_READY,	//おそらく使ってない可能性が高いがUBA500に合わせる
#endif

	TMSG_SUB_ALARM = 0xFF,
}TMSG_SUB;

/* CONNECTION Task (CLINE Task or DLINE Task) */
enum _TMSG_CONNECTION
{
	TMSG_CONN_INITIAL = 1,
	TMSG_CONN_DOWNLOAD,
	TMSG_CONN_SOFT_RESET,
	TMSG_CONN_SEARCH_BILL,
	TMSG_CONN_RESET,
	TMSG_CONN_DISABLE,
	TMSG_CONN_ENABLE,
	TMSG_CONN_ACCEPT,
	TMSG_CONN_STACK,
	TMSG_CONN_REJECT,
	TMSG_CONN_PAYOUT,
	TMSG_CONN_SET_STATUS,
	TMSG_CONN_NOTICE = TMSG_COMMON_STATUS,
	TMSG_CONN_STATUS = TMSG_COMMON_STATUS,
	TMSG_CONN_SIGNATURE,
#if defined(UBA_RTQ)
	TMSG_CONN_RC_TDIPSW,
	TMSG_CONN_COLLECT,
	TMSG_CONN_RC_INFO,
	TMSG_CONN_RC_SEARCH_BILL,
	TMSG_CONN_RC_CURRENT,
	TMSG_CONN_RC_EMERGENCY,
	TMSG_CONN_RC_PAYOUT,
	TMSG_CONN_RC_PAYSTAY,
	TMSG_CONN_RC_PAYVALID,
	TMSG_CONN_RC_RECOVERY,
	TMSG_CONN_PAYOUT_RETURN,
	TMSG_CONN_RC_ENABLE,
	TMSG_CONN_LINE_OPEN,
#endif  // UBA_RTQ
	TMSG_CONN_TEST = 0x00F0,
	TMSG_CONN_TEST_FINISH,
	TMSG_CONN_CIS_AD,
	TMSG_CONN_RESET_DA,
	TMSG_CONN_1ST_NOTE_DONE,
};

enum _RESET_TYPE
{
	RESET_TYPE_NORMAL = 0,
//	#if defined(UBA_RTQ)		/* '22-02-15 */
	RESET_TYPE_WAIT_PAYVALID_ACK,
	RESET_TYPE_WAIT_PAYSTAY_POLL,
//	#endif
};

enum _SEARCH_TYPE
{
	SEARCH_TYPE_NORMAL = 0,
	SEARCH_TYPE_WINDOW,
};


/* DLINE Task */
#define TMSG_DLINE_INITIAL_REQ			((TMSG_TCODE_MAIN)|TMSG_CONN_INITIAL)
#define TMSG_DLINE_SEARCH_BILL_REQ		((TMSG_TCODE_DLINE)|TMSG_CONN_SEARCH_BILL)
#define TMSG_DLINE_SEARCH_BILL_RSP		((TMSG_TCODE_MAIN)|TMSG_CONN_SEARCH_BILL)
#define TMSG_DLINE_RESET_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_RESET)
#define TMSG_DLINE_RESET_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_RESET)
#define TMSG_DLINE_DISABLE_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_DISABLE)
#define TMSG_DLINE_DISABLE_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_DISABLE)
#define TMSG_DLINE_ENABLE_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_ENABLE)
#define TMSG_DLINE_ENABLE_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_ENABLE)
#define TMSG_DLINE_ACCEPT_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_ACCEPT)
#define TMSG_DLINE_ACCEPT_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_ACCEPT)
#define TMSG_DLINE_STACK_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_STACK)
#define TMSG_DLINE_STACK_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_STACK)
#define TMSG_DLINE_REJECT_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_REJECT)
#define TMSG_DLINE_REJECT_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_REJECT)
#define TMSG_DLINE_PAYOUT_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_PAYOUT)
#define TMSG_DLINE_PAYOUT_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_PAYOUT)
#define TMSG_DLINE_STATUS_INFO			((TMSG_TCODE_MAIN)|TMSG_CONN_STATUS)
#define TMSG_DLINE_TEST_REQ				((TMSG_TCODE_DLINE)|TMSG_CONN_TEST)
#define TMSG_DLINE_TEST_RSP				((TMSG_TCODE_MAIN)|TMSG_CONN_TEST)
#define TMSG_DLINE_TEST_FINISH_REQ		((TMSG_TCODE_DLINE)|TMSG_CONN_TEST_FINISH)
#define TMSG_DLINE_TEST_FINISH_RSP		((TMSG_TCODE_MAIN)|TMSG_CONN_TEST_FINISH)

#if defined(UBA_RTQ)
	#define TMSG_DLINE_TEST_DIPSW_REQ	((TMSG_TCODE_DLINE)|TMSG_CONN_RC_TDIPSW)
	#define TMSG_DLINE_TEST_DIPSW_RSP	((TMSG_TCODE_MAIN) |TMSG_CONN_RC_TDIPSW)
	#define TMSG_DLINE_COLLECT_REQ		((TMSG_TCODE_DLINE)|TMSG_CONN_COLLECT)
	#define TMSG_DLINE_COLLECT_RSP		((TMSG_TCODE_MAIN)|TMSG_CONN_COLLECT)
	#define TMSG_DLINE_RC_SEARCH_BILL_REQ 	((TMSG_TCODE_DLINE)|TMSG_CONN_RC_SEARCH_BILL)
	#define TMSG_DLINE_RC_SEARCH_BILL_RSP 	((TMSG_TCODE_MAIN) |TMSG_CONN_RC_SEARCH_BILL)
	// Dline test
	#define TMSG_DLINE_TEST_PAYOUT_REQ		((TMSG_TCODE_DLINE)|TMSG_CONN_RC_PAYOUT)
	#define TMSG_DLINE_TEST_PAYOUT_RSP		((TMSG_TCODE_MAIN)|TMSG_CONN_RC_PAYOUT)
#endif // UBA_RTQ

/* CLINE Task */
#define TMSG_CLINE_INITIAL_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_INITIAL)
#define TMSG_CLINE_SEARCH_BILL_REQ		((TMSG_TCODE_CLINE)|TMSG_CONN_SEARCH_BILL)
#define TMSG_CLINE_SEARCH_BILL_RSP		((TMSG_TCODE_MAIN)|TMSG_CONN_SEARCH_BILL)
#define TMSG_CLINE_RESET_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_RESET)
#define TMSG_CLINE_RESET_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_RESET)
#define TMSG_CLINE_DISABLE_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_DISABLE)
#define TMSG_CLINE_DISABLE_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_DISABLE)
#define TMSG_CLINE_ENABLE_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_ENABLE)
#define TMSG_CLINE_ENABLE_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_ENABLE)
#define TMSG_CLINE_ACCEPT_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_ACCEPT)
#define TMSG_CLINE_ACCEPT_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_ACCEPT)
#define TMSG_CLINE_STACK_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_STACK)
#define TMSG_CLINE_STACK_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_STACK)
#define TMSG_CLINE_REJECT_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_REJECT)
#define TMSG_CLINE_REJECT_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_REJECT)
#define TMSG_CLINE_PAYOUT_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_PAYOUT)
#define TMSG_CLINE_PAYOUT_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_PAYOUT)
#define TMSG_CLINE_SET_STATUS			((TMSG_TCODE_CLINE)|TMSG_CONN_SET_STATUS)
#define TMSG_CLINE_STATUS_INFO			((TMSG_TCODE_MAIN)|TMSG_CONN_STATUS)
#define TMSG_CLINE_1ST_NOTE_DONE_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_1ST_NOTE_DONE)
#define TMSG_CLINE_1ST_NOTE_DONE_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_1ST_NOTE_DONE)

#if defined(UBA_RTQ)
	#define TMSG_CLINE_COLLECT_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_COLLECT)
	#define TMSG_CLINE_COLLECT_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_COLLECT)
	#define TMSG_CLINE_RC_INFO_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_RC_INFO)
	#define TMSG_CLINE_RC_INFO_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_RC_INFO)
	#define TMSG_CLINE_RC_SEARCH_BILL_REQ 	((TMSG_TCODE_CLINE)|TMSG_CONN_RC_SEARCH_BILL)
	#define TMSG_CLINE_RC_SEARCH_BILL_RSP 	((TMSG_TCODE_MAIN) |TMSG_CONN_RC_SEARCH_BILL)

	#define TMSG_LINE_CURRENT_COUNT_REQ	((TMSG_TCODE_CLINE) |TMSG_CONN_RC_CURRENT)
	#define TMSG_LINE_CURRENT_COUNT_RSP	((TMSG_TCODE_MAIN) |TMSG_CONN_RC_CURRENT)

	#define	TMSG_CLINE_RC_RECOVERY_REQ		((TMSG_TCODE_CLINE)|TMSG_CONN_RC_RECOVERY)
	#define	TMSG_CLINE_RC_RECOVERY_RSP		((TMSG_TCODE_MAIN)|TMSG_CONN_RC_RECOVERY)

	#define TMSG_CLINE_EMERGENCY_REQ		((TMSG_TCODE_CLINE)|TMSG_CONN_RC_EMERGENCY)

	#define TMSG_CLINE_PAYVALID_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_RC_PAYVALID)
	#define TMSG_CLINE_PAYVALID_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_RC_PAYVALID)
	#define TMSG_CLINE_PAYSTAY_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_RC_PAYSTAY)
	#define TMSG_CLINE_PAYSTAY_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_RC_PAYSTAY)

	#define TMSG_LINE_PAYOUT_RETURNED_REQ	((TMSG_TCODE_CLINE)|TMSG_CONN_PAYOUT_RETURN) //名前がよくないが、Payout紙幣の払い出し失敗で収納成功
	#define TMSG_LINE_PAYOUT_RETURNED_RSP	((TMSG_TCODE_MAIN)|TMSG_CONN_PAYOUT_RETURN)
	#define	TMSG_LINE_RC_ENABLE_REQ		((TMSG_TCODE_CLINE)|TMSG_CONN_RC_ENABLE)
	#define TMSG_LINE_RC_ENABLE_RSP		((TMSG_TCODE_MAIN)|TMSG_CONN_RC_ENABLE)

	#define TMSG_CLINE_UART_OPEM_REQ				((TMSG_TCODE_MAIN)|TMSG_CONN_LINE_OPEN)

#endif // UBA_RTQ


enum _VEND_POSITION
{
	VEND_POSITION_1 = 0,
	VEND_POSITION_2,
#if defined(UBA_RTQ)
	VEND_POSITION_3,
#endif // UBA_RTQ
};

enum _POWERUP_STAT
{
	/*
	 * normal powerup
	 */
	POWERUP_STAT_NORMAL = 0,
	/*
	 * bill in acceptor
	 */
	POWERUP_STAT_REJECT,
	/*
	 * bill in stacker
	 */
	POWERUP_STAT_FEED_STACK,	//stackコマンド受信前,ただし電源ON後Stacker側に紙幣があるので、とりあえず収納
	/*
	 * recovery
	 */
	POWERUP_STAT_RECOVER_FEED_STACK, //stackコマンド受信後で、紙幣をStackerで検知できている,EXITで紙幣検知できている、基本的にリカバリでVend
	POWERUP_STAT_RECOVER_STACK,		//押し込み開始していて、基本的にリカバリでvend
	POWERUP_STAT_RECOVER_SEARCH_NON, //GLI Error
	//POWERUP_STAT_RECOVER_SEARCH_REJECT, //さがしたらHeadに紙幣があった, 処理が同じなのでPOWERUP_STAT_REJECT　に統一
	//POWERUP_STAT_RECOVER_SEARCH_STACK, //POWERUP_STAT_RECOVER_FEED_STACKと処理が同じなので、廃止する//stackコマンド受信後で、紙幣をStackerで検知できている
	//バックアップシーケンスは同じで、電源ON時にそのまま紙幣があったか、探した後に紙幣があったかの違いで
	//その後の処理は同様、ソース見にくいので、廃止
	//POWERUP_STAT_RECOVER_NOTICE_JAM,
	#if defined(UBA_RTQ)
	POWERUP_STAT_RECOVER_PAYOUT,
	POWERUP_STAT_RECOVER_RC, //not use
	POWERUP_STAT_RECOVER_CHEAT,
	POWERUP_STAT_RECOVER_NO_COUNT,
	POWERUP_STAT_RECOVER_COLLECT, //not use ID003
	#endif // UBA_RTQ
};

enum _GENERATION_TYPE
{
	GENERATION_NONE = 0,
	GENERATION_FLEX_ENCRYPTION_TABLE,
};

/* DIPSW Task */
#define TMSG_DIPSW_INIT_REQ					((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_DIPSW_INIT_RSP					((TMSG_TCODE_DIPSW)|0x0001)
//#define TMSG_TEST_SW_NOTICE					((TMSG_TCODE_DIPSW)|0x0002)

/* UART01CB Task */
#define TMSG_UART01CB_CALLBACK_INFO			((TMSG_TCODE_UART01CB)|TMSG_COMMON_STATUS)

/* USB0CB Task */
#define TMSG_USB0CB_CALLBACK_INFO			((TMSG_TCODE_USB0CB)|TMSG_COMMON_STATUS)

/* USB1CB Task */
#define TMSG_USB2CB_CALLBACK_INFO			((TMSG_TCODE_USB2CB)|TMSG_COMMON_STATUS)

/* SUBLINE Task */
#define TMSG_SUBLINE_DOWNLOAD_REQ			((TMSG_TCODE_SUBLINE)|TMSG_CONN_DOWNLOAD)
#define TMSG_SUBLINE_DOWNLOAD_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_DOWNLOAD)
#define TMSG_SUBLINE_SOFT_RESET_REQ			((TMSG_TCODE_SUBLINE)|TMSG_CONN_SOFT_RESET)
#define TMSG_SUBLINE_SOFT_RESET_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_SOFT_RESET)

/* OTG Task */
#define TMSG_OTG_NOTICE						((TMSG_TCODE_OTG)|TMSG_CONN_NOTICE)

#if defined(UBA_RTQ)
/* FUCB_DECT Task */
#define TMSG_FUSB_DECT_INITIAL_REQ			((TMSG_TCODE_MAIN)|TMSG_CONN_INITIAL)
#define TMSG_FUSB_DECT_NOTICE				((TMSG_TCODE_FUSB_DECT)|TMSG_CONN_NOTICE)
#endif // UBA_RTQ

/* POWER Task */
#define TMSG_POWER_NOTICE					((TMSG_TCODE_POWER)|TMSG_CONN_NOTICE)
#define TMSG_RESET_NOTICE					((TMSG_TCODE_POWER)|TMSG_CONN_RESET)
#define TMSG_PLL_LOCK						((TMSG_TCODE_POWER)|(TMSG_CONN_NOTICE+1))

/* SDC DET Task */
#define TMSG_SDC_DET_CALLBACK_INFO			((TMSG_TCODE_SDC_DET)|TMSG_COMMON_STATUS)

/* TIMER Task */
#define TMSG_TIMER_TIMES_UP			((TMSG_TCODE_TIME)|0x0001)
#define TMSG_TIMER_SET_TIMER		((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_TIMER_CANCEL_TIMER		((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_TIMER_CANCEL_TIMER_2	((TMSG_TCODE_MAIN)|0x0003)				/* for ID-003 transaction */
#define TMSG_TIMER_STATUS_REQ		((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_TIMER_STATUS_INFO		((TMSG_TCODE_TIME)|TMSG_COMMON_STATUS)

#if defined(UBA_RTQ)
#define TMSG_TIMER_RFID_RTQ			((TMSG_TCODE_TIME)|0x0004)
#endif

#define	MAX_TIMER	64

/* TIMER ID */
enum _TIMER_ID
{
	TIMER_ID_DIPSW_READ = 1,
	TIMER_ID_TEMP_ADJ,
	TIMER_ID_ESCROW_STATUS_REQ1,
	TIMER_ID_ESCROW_STATUS_REQ2,
	TIMER_ID_ESCROW_HOLD1,
	TIMER_ID_ESCROW_HOLD2,
	#if defined(_PROTOCOL_ENABLE_ID0G8)
	TIMER_ID_EVENT_SEND,
	#else
	TIMER_ID_ENQ_SEND,
	#endif
	TIMER_ID_STATUS_WAIT1,
	TIMER_ID_STATUS_WAIT2,
	TIMER_ID_RECOVER_WAIT,
	TIMER_ID_AGING_WAIT,
	TIMER_ID_DATA_WAIT,
	TIMER_ID_STACK_HALF,
	TIMER_ID_NOTE_STAY_WAIT,
    TIMER_ID_LINE_WAIT,					// 16 /* LINEタスクから要求待ちのためのタイマ *///2023-11-28
	TIMER_ID_CONFIRM_TEMP, //2024-05-28
	TIMER_ID_RC_CHECK,
	TIMER_ID_RC_TIMEOUT,
#if defined(UBA_RTQ)
	TIMER_ID_RC_STATUS,
	TIMER_ID_RC_RESET_SKIP,
	TIMER_ID_RC_RESET_SKIP_ERROR,
	TIMER_ID_SEQ,	//2025-10-22
#endif // UBA_RTQ
//#if defined(UBA_RS)
	TIMER_ID_RS_CONTROL_LED,
	TIMER_ID_RS_IF_REMAIN_TIMER,
//#endif
	TIMER_ID_TEMP,		//2025-09-12
	/* Upper limit 64 */
};


/* 10msec timer */
#define WAIT_TIME_DIPSW_READ			100		/* wait 1000msec */
#define WAIT_TIME_TEMP_ADJ_SUCCESS		18000	/* wait 3minute */
#define WAIT_TIME_TEMP_ADJ_FAILURE		20		/* wait 200msec */
#define WAIT_TIME_TEMP_ADJ_SKIP			20		/* wait 200msec */
#define WAIT_TIME_SEND_ENQ				20		/* wait 200msec */
#define WAIT_TIME_RESEND_ENQ			25		/* wait 250msec */
#define WAIT_TIME_STATUS_WAIT			300		/* wait 3sec    */
#define WAIT_TIME_RECOVER				50		/* wait 500msec */
//2025-03-10 #define	WAIT_TIME_STACKER_HALF			200		/* wait 2000msec */
#define WAIT_TIME_STACK_HALF			300		/* wait 3sec *///2025-03-10
#define WAIT_TIME_DATA_WAIT				500		/* wait 5sec    */
#define WAIT_TIME_LINE_REQ				15		/* wait 150ms *///2023-11-28

#if defined(UBA_RTQ)
	#define WAIT_TIME_RC_CHECK			15			/* wait 150msec */
	#define	WAIT_TIME_RC_FLAP_CHECK		1			/* wait 10msec */
	#define WAIT_TIME_RC_BUSY_CHECK		1			/* wait 10msec */
	#define WAIT_TIME_RC_TIMEOUT		3000		/* wait 30sec */
	//#if defined(UBA_RS)
	#define	WAIT_TIME_CHECK_REMAIN	12000		/* wait 2min	*/
	//#endif
	#if defined(UBA_RTQ_ICB)
	#define WAIT_TIME_RC_RFID			100		//1s
	#endif
#endif


/* DISPLAY Task */
#define TMSG_DISP_LED_OFF				((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_DISP_LED_ON				((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_DISP_LED_DENOMI			((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_DISP_LED_REJECT			((TMSG_TCODE_MAIN)|0x0004)
#define TMSG_DISP_LED_DOWNLOAD			((TMSG_TCODE_MAIN)|0x000C)
#define TMSG_DISP_LED_DOWNLOAD_COMPLETE	((TMSG_TCODE_MAIN)|0x000D)
#define TMSG_DISP_LED_ALARM				((TMSG_TCODE_MAIN)|0x000F)

#define TMSG_DISP_STATUS_REQ			((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_DISP_STATUS_INFO			((TMSG_TCODE_DISP)|TMSG_COMMON_STATUS)

#define TMSG_DISP_BEZEL_LED_OFF			((TMSG_TCODE_MAIN)|0x0010)
#define TMSG_DISP_BEZEL_LED_ON			((TMSG_TCODE_MAIN)|0x0011)
#define TMSG_DISP_BEZEL_TEST_RUNNING	((TMSG_TCODE_MAIN)|0x0012)
#define TMSG_DISP_BEZEL_ABNORMAL		((TMSG_TCODE_MAIN)|0x0013)
#define TMSG_DISP_BEZEL_DEMO			((TMSG_TCODE_MAIN)|0x0014)
#define TMSG_DISP_BEZEL_BLINK			((TMSG_TCODE_MAIN)|0x0015)


enum _DISP_CTRL
{
	DISP_CTRL_TEST_STANDBY = 0,
#if defined(UBA_RTQ)
	DISP_CTRL_TEST_RC_GREEN,
	DISP_CTRL_TEST_RC_RED,
	DISP_CTRL_TEST_RC_ON,
	DISP_CTRL_TEST_RC_OFF,
#endif
};

/* MOTOR Task */
#define TMSG_MOTOR_FEED_FWD_REQ			((TMSG_TCODE_FEED)|0x0001)
#define TMSG_MOTOR_FEED_REV_REQ			((TMSG_TCODE_FEED)|0x0002)
#define TMSG_MOTOR_FEED_STOP			((TMSG_TCODE_FEED)|0x0003)
#define TMSG_MOTOR_STACKER_FWD_REQ		((TMSG_TCODE_STAC)|0x0011)
#define TMSG_MOTOR_STACKER_REV_REQ		((TMSG_TCODE_STAC)|0x0012)
#define TMSG_MOTOR_STACKER_STOP			((TMSG_TCODE_STAC)|0x0013)
#define TMSG_MOTOR_CENTERING_FWD_REQ	((TMSG_TCODE_CENT)|0x0021)
#define TMSG_MOTOR_CENTERING_REV_REQ	((TMSG_TCODE_CENT)|0x0022)
#define TMSG_MOTOR_CENTERING_STOP		((TMSG_TCODE_CENT)|0x0023)
#define TMSG_MOTOR_APB_FWD_REQ			((TMSG_TCODE_APB )|0x0031)
#define TMSG_MOTOR_APB_REV_REQ			((TMSG_TCODE_APB )|0x0032)
#define TMSG_MOTOR_APB_STOP				((TMSG_TCODE_APB )|0x0033)
#define TMSG_MOTOR_SHUTTER_FWD_REQ			((TMSG_TCODE_SHUTTER )|0x0041)
#define TMSG_MOTOR_SHUTTER_REV_REQ			((TMSG_TCODE_SHUTTER )|0x0042)
#define TMSG_MOTOR_SHUTTER_STOP				((TMSG_TCODE_SHUTTER )|0x0043)

enum _FEED_SPEED
{
	FEED_SPEED_CENTERING,
	FEED_SPEED_ESCROW,
	FEED_SPEED_APB,
	FEED_SPEED_STACK,
	FEED_SPEED_REJECT,
	FEED_SPEED_PAYOUT,
	FEED_SPEED_FREE_RUN,
	FEED_SPEED_DISABLE,
	FEED_SPEED_END,
};

/* SENSOR Task */
#define TMSG_SENSOR_INIT_REQ			((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_SENSOR_INIT_RSP			((TMSG_TCODE_SENS)|0x0001)
#define TMSG_SENSOR_ACTIVE_REQ			((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_SENSOR_ACTIVE_RSP			((TMSG_TCODE_SENS)|0x0002)
#define TMSG_SENSOR_STANDBY_REQ			((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_SENSOR_STANDBY_RSP			((TMSG_TCODE_SENS)|0x0003)
#define TMSG_SENSOR_TEMP_ADJ_REQ		((TMSG_TCODE_MAIN)|0x0005)
#define TMSG_SENSOR_TEMP_ADJ_RSP		((TMSG_TCODE_SENS)|0x0005)
#define TMSG_SENSOR_CIS_ACTIVE_REQ		((TMSG_TCODE_MAIN)|0x0009)
#define TMSG_SENSOR_CIS_ACTIVE_RSP		((TMSG_TCODE_SENS)|0x0009)
#define TMSG_SENSOR_STATUS_REQ			((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_SENSOR_STATUS_INFO			((TMSG_TCODE_SENS)|TMSG_COMMON_STATUS)

/* SENSOR Temperature Adjustment result */
enum _TEMP_ADJ_RESULT
{
	TEMP_ADJ_SUCCESS = 0,
	TEMP_ADJ_SENSOR_SHIFT,
	TEMP_ADJ_OVER_RUN,
	TEMP_ADJ_ERROR,
};

/* FRAM Task */
enum _TMSG_FRAM_SUB
{
	TMSG_FRAM_READ = 0,
	TMSG_FRAM_WRITE,
};
#define TMSG_FRAM_READ_REQ			((TMSG_TCODE_MAIN)|TMSG_FRAM_READ)
#define TMSG_FRAM_READ_RSP			((TMSG_TCODE_FRAM)|TMSG_FRAM_READ)
#define TMSG_FRAM_WRITE_REQ			((TMSG_TCODE_MAIN)|TMSG_FRAM_WRITE)
#define TMSG_FRAM_WRITE_RSP			((TMSG_TCODE_FRAM)|TMSG_FRAM_WRITE)

/* FRAM type */
enum _FRAM_AREA
{
	FRAM_ADJ = 0,
	FRAM_ADJ_TMP,
	FRAM_CIS_ADJ,
	FRAM_CIS_ADJ_TMP,
	FRAM_IF,
	FRAM_LOG,
	FRAM_ALL,
	FRAM_ADJ_INF,
	FRAM_POS,
	FRAM_ADJ_DS,
	FRAM_MODE_SETTING, //not use
	FRAM_POWER_RECOVER,
	FRAM_ICB_SETTING,
	FRAM_ICB_RECOVER,
	FRAM_IF_AUTHENTICATION,
#if defined(UBA_RTQ)
	FRAM_RTQ,
	#if defined(UBA_RTQ_ICB)
	FRAM_ICB_RECOVER_RTQ,
	#endif
#endif
};

#if defined(UBA_RTQ)
enum _FRAM_RC_AREA
{
	FRAM_RC_SOFT_INFO,
	FRAM_RC_BEFORE_STA,
	FRAM_RC_LOG_IF,
	FRAM_RC_MENTEN_SERI,
	FRAM_RC_EDITION, //2024-10-16
	FRAM_RC_ALL,
};
#endif // UBA_RTQ

/* FRAM SETTING CLEAR or not */
enum _FRAM_SETTING
{
	FRAM_SETTING_KEEP = 0,
	FRAM_SETTING_CLEAR
};
/* MGU Task */
enum _TMSG_MGU_SUB
{
	TMSG_MGU_READ = 0,
	TMSG_MGU_WRITE,
};
#define TMSG_MGU_READ_REQ			((TMSG_TCODE_MAIN)|TMSG_MGU_READ)
#define TMSG_MGU_READ_RSP			((TMSG_TCODE_MGU)|TMSG_MGU_READ)
#define TMSG_MGU_WRITE_REQ			((TMSG_TCODE_MAIN)|TMSG_MGU_WRITE)
#define TMSG_MGU_WRITE_RSP			((TMSG_TCODE_MGU)|TMSG_MGU_WRITE)

enum _MGU_AREA
{
	MGU_TMP = 0,
	MGU_LOG,
};

/* RFID Task */
enum _TMSG_RFID_SUB
{
	TMSG_RFID_READ = 0,
	TMSG_RFID_WRITE,
	TMSG_RFID_RESET,
	
	TMSG_RFID_CLEAR_DEMO,
	TMSG_RFID_READ_DEMO,

};
#define TMSG_RFID_STATUS_REQ		((TMSG_TCODE_RFID)|TMSG_COMMON_STATUS) //not use RTQ
#define TMSG_RFID_STATUS_INFO		((TMSG_TCODE_ICB)|TMSG_COMMON_STATUS)
#define TMSG_RFID_READ_REQ			((TMSG_TCODE_RFID)|TMSG_RFID_READ) //not use RTQ
#define TMSG_RFID_READ_RSP			((TMSG_TCODE_ICB)|TMSG_RFID_READ)
#define TMSG_RFID_WRITE_REQ			((TMSG_TCODE_RFID)|TMSG_RFID_WRITE) //not use RTQ
#define TMSG_RFID_WRITE_RSP			((TMSG_TCODE_ICB)|TMSG_RFID_WRITE)
#define TMSG_RFID_RESET_REQ			((TMSG_TCODE_RFID)|TMSG_RFID_RESET) //RTQは品質規格用でのみ使用
#define TMSG_RFID_RESET_RSP			((TMSG_TCODE_ICB)|TMSG_RFID_RESET)

#define TMSG_RFID_CLEAR_DEMO_REQ			((TMSG_TCODE_RFID)|TMSG_RFID_CLEAR_DEMO)
#define TMSG_RFID_READ_DEMO_REQ				((TMSG_TCODE_RFID)|TMSG_RFID_READ_DEMO)

enum _RFID_TYPE
{
	RFID_ICB,
};

/* ICB Task */
#define TMSG_ICB_INITIAL_REQ		((TMSG_TCODE_MAIN)|0x0001)	// MAINから
#define TMSG_ICB_INITIAL_RSP		((TMSG_TCODE_ICB)|0x0001)
#define TMSG_ICB_ACCEPT_REQ			((TMSG_TCODE_MAIN)|0x0003)	// MAINから
#define TMSG_ICB_ACCEPT_RSP			((TMSG_TCODE_ICB)|0x0003)
#define TMSG_ICB_REJECT_REQ			((TMSG_TCODE_MAIN)|0x0004)	// MAINから
#define TMSG_ICB_REJECT_RSP			((TMSG_TCODE_ICB)|0x0004)
#define TMSG_ICB_REJECT_TICKET_REQ	((TMSG_TCODE_MAIN)|0x0005)	// MAINから
#define TMSG_ICB_REJECT_TICKET_RSP	((TMSG_TCODE_ICB)|0x0005)
#define TMSG_ICB_ERROR_CODE_REQ		((TMSG_TCODE_MAIN)|0x0006)	// MAINから
#define TMSG_ICB_ERROR_CODE_RSP		((TMSG_TCODE_ICB)|0x0006)
#define TMSG_ICB_SET_TIME_REQ		((TMSG_TCODE_MAIN)|0x0008)	// MAINから
#define TMSG_ICB_SET_TIME_RSP		((TMSG_TCODE_ICB)|0x0008)
#define TIME_ICB_TIME_SET			300000		/* 30minutes *//* 本番は30分 20 = 200msec */

#if defined(UBA_RTQ_ICB)
#define TMSG_ICB_ACCEPT_RTQ_REQ			((TMSG_TCODE_MAIN)|0x000B)	// MAINから
#define TMSG_ICB_ACCEPT_RTQ_RSP			((TMSG_TCODE_ICB)|0x000B)
#endif

/* SENSOR Bill position */
enum _BILL_IN
{
	BILL_IN_NON = 0,
	BILL_IN_ENTRANCE,
	BILL_IN_ACCEPTOR,
	BILL_IN_STACKER,
#if defined(UBA_RTQ)
	BILL_IN_RC,
#endif
};

/* FEED Task */
#define TMSG_FEED_INITIAL_REQ		((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_FEED_INITIAL_RSP		((TMSG_TCODE_FEED)|0x0001)
#define TMSG_FEED_CENTERING_REQ		((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_FEED_CENTERING_RSP		((TMSG_TCODE_FEED)|0x0002)
#define TMSG_FEED_ESCROW_REQ		((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_FEED_ESCROW_RSP		((TMSG_TCODE_FEED)|0x0003)
#define TMSG_FEED_APB_REQ			((TMSG_TCODE_MAIN)|0x0004)
#define TMSG_FEED_APB_RSP			((TMSG_TCODE_FEED)|0x0004)

#define TMSG_FEED_REV_CHECK_BILL_REQ ((TMSG_TCODE_MAIN)|0x0005)	//#if defined(HIGH_SECURITY_MODE)
#define TMSG_FEED_REV_CHECK_BILL_RSP ((TMSG_TCODE_FEED)|0x0005)

#define TMSG_FEED_FORCE_STACK_REQ	((TMSG_TCODE_MAIN)|0x0006)
#define TMSG_FEED_FORCE_STACK_RSP	((TMSG_TCODE_FEED)|0x0006)
#define TMSG_FEED_REJECT_REQ		((TMSG_TCODE_MAIN)|0x0007)
#define TMSG_FEED_REJECT_RSP		((TMSG_TCODE_FEED)|0x0007)
//TODO:
#define TMSG_FEED_FORCE_REV_REQ	((TMSG_TCODE_MAIN)|0x0008) //2024-03-18a ID-003 Disable, Forced return of bills detected at entrance
#define TMSG_FEED_FORCE_REV_RSP	((TMSG_TCODE_FEED)|0x0008)

#define TMSG_FEED_SEARCH_REQ		((TMSG_TCODE_MAIN)|0x000A)
#define TMSG_FEED_SEARCH_RSP		((TMSG_TCODE_FEED)|0x000A)

#if defined(UBA_RTQ)
#define TMSG_ENTRY_BACK_REQ			((TMSG_TCODE_MAIN)|0x000B)	//100msec搬送逆転させる処理、どれほど必要かは不明
#define TMSG_ENTRY_BACK_RSP			((TMSG_TCODE_FEED)|0x000B)
#endif

#define TMSG_FEED_AGING_REQ			((TMSG_TCODE_MAIN)|0x000E)
#define TMSG_FEED_AGING_RSP			((TMSG_TCODE_FEED)|0x000E)
#define TMSG_FEED_FREERUN_REQ		((TMSG_TCODE_MAIN)|0x000F)
#define TMSG_FEED_FREERUN_RSP		((TMSG_TCODE_FEED)|0x000F)

#define TMSG_FEED_STATUS_REQ		((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_FEED_STATUS_INFO		((TMSG_TCODE_FEED)|TMSG_COMMON_STATUS)

#if defined(UBA_RTQ)
#define TMSG_FEED_RC_STACK_REQ			((TMSG_TCODE_MAIN)|0x0012)
#define TMSG_FEED_RC_STACK_RSP			((TMSG_TCODE_FEED)|0x0012)
#define TMSG_FEED_RC_PAYOUT_REQ			((TMSG_TCODE_MAIN)|0x0013)
#define TMSG_FEED_RC_PAYOUT_RSP			((TMSG_TCODE_FEED)|0x0013)
#define TMSG_FEED_RC_COLLECT_REQ		((TMSG_TCODE_MAIN)|0x0014)
#define TMSG_FEED_RC_COLLECT_RSP		((TMSG_TCODE_FEED)|0x0014)
#define TMSG_FEED_RC_FORCE_PAYOUT_REQ 	((TMSG_TCODE_MAIN)|0x0015)
#define TMSG_FEED_RC_FORCE_PAYOUT_RSP 	((TMSG_TCODE_FEED)|0x0015)
#define TMSG_FEED_RC_FORCE_STACK_REQ 	((TMSG_TCODE_MAIN)|0x0016)
#define TMSG_FEED_RC_FORCE_STACK_RSP 	((TMSG_TCODE_FEED)|0x0016)
#define TMSG_FEED_RC_BILLBACK_REQ		((TMSG_TCODE_MAIN)|0x0017)
#define TMSG_FEED_RC_BILLBACK_RSP		((TMSG_TCODE_MAIN)|0x0017)
#define TMSG_FEED_RC_PAYOUT_STOP_REQ	((TMSG_TCODE_MAIN)|0x0019)
#define TMSG_FEED_RC_PAYOUT_STOP_RSP	((TMSG_TCODE_FEED)|0x0019)
//#if defined(UBA_RS)
#define TMSG_FEED_RS_FORCE_PAYOUT_REQ	((TMSG_TCODE_MAIN)|0x0021)
#define TMSG_FEED_RS_FORCE_PAYOUT_RSP	((TMSG_TCODE_FEED)|0x0021)
//#endif
#endif // UBA_RTQ

enum _FEED_REJECT_OPTION
{
	FEED_REJECT_OPTION_NORMAL = 0x00,

	FEED_REJECT_OPTION_INITIAL 		= 0x20,	//use
	FEED_REJECT_OPTION_AFTER_ESCROW = 0x40,	//use
	FEED_REJECT_OPTION_RETRY      	= 0x80,	//use
};

#if defined(UBA_RTQ)	
enum _FEED_PAYOUT_OPTION
{
	FEED_PAYOUT_OPTION_NORMAL	= 0x00,
	FEED_PAYOUT_OPTION_RETRY	= 0x11,
};
#endif


enum _STACK_OPTION_SS
{
    SS_NON = 0,
	SS_PULL_HALF,	/* 通常時の戻し半押し位置*/
	SS_BILL_STACK,	/* 通常時の押し込み*/

};
#define SENSOR_STACKER_HOME		SENSOR_PUSHER_HOME	/* 名前を流用したいので、とりあえずここで置き換え、将来的には統一 *//* UBA_GPIO*/

/* 下記2つがとくに大切 */
#if defined(UBA_RTQ)
	#define STACKER_TOP_PULSE			   1053 //1150×0.916
	#define STACKER_FULL_DA					824	//900×0.916	// メカ指定 押し込み頂点 付近での電流制限
#else
	//SS
	#define STACKER_TOP_PULSE				797		//870×0.916 /*pulse*/// メカ指定 押し込み頂点 決定幅寄せ			/* 2019-06-19 */
	#define STACKER_FULL_DA					522		//570×0.916 /*pulse*/// メカ指定 押し込み頂点 付近での電流制限	/* 2019-06-19 */
#endif


enum _FEED_AGING
{
	FEED_AGING_CENTERING = 0,
	FEED_AGING_ESCROW,
	FEED_AGING_APB,
	FEED_AGING_STACK,
	FEED_AGING_REJECT,
	FEED_AGING_PAYOUT,
};

enum _FEED_SEARCH_OPTION
{
	FEED_SEARCH_OPTION_NORMAL = 0x00,
	FEED_SEARCH_OPTION_WINDOW = 0x01,
};

#if defined(UBA_RTQ)
enum _FEED_SEARCH_DIRECTION
{
	FEED_SEARCH_IN = 0x00,
	FEED_SEARCH_OUT = 0x01,
};
#endif // UBA_RTQ

#define FEED_SUCCESS			0x00
#define FEED_REJECT				0x01
#define FEED_PAUSE				0x02
#define FEED_RESUME				0x03
#define FEED_ALARM				0x0F

/* CENTERING Task */
#define TMSG_CENTERING_HOME_REQ		((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_CENTERING_HOME_RSP		((TMSG_TCODE_CENT)|0x0001)
#define TMSG_CENTERING_EXEC_REQ		((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_CENTERING_EXEC_RSP		((TMSG_TCODE_CENT)|0x0002)
#define TMSG_CENTERING_CLOSE_REQ	((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_CENTERING_CLOSE_RSP	((TMSG_TCODE_CENT)|0x0003)
#define TMSG_CENTERING_OPEN_REQ		((TMSG_TCODE_MAIN)|0x0004)
#define TMSG_CENTERING_OPEN_RSP		((TMSG_TCODE_CENT)|0x0004)
#define TMSG_CENTERING_HOME_OUT_RETRY_REQ		((TMSG_TCODE_MAIN)|0x0005)
#define TMSG_CENTERING_HOME_OUT_RETRY_RSP		((TMSG_TCODE_CENT)|0x0005)


#define TMSG_CENTERING_FREERUN_REQ	((TMSG_TCODE_MAIN)|0x000F)
#define TMSG_CENTERING_FREERUN_RSP	((TMSG_TCODE_CENT)|0x000F)
#define TMSG_CENTERING_STATUS_REQ	((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_CENTERING_STATUS_INFO	((TMSG_TCODE_CENT)|TMSG_COMMON_STATUS)

#define CENTERING_EXEC_TIME_MEASURE	0x01

/* APB Task */
#define TMSG_APB_HOME_REQ			((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_APB_HOME_RSP			((TMSG_TCODE_APB )|0x0001)
#define TMSG_APB_EXEC_REQ			((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_APB_EXEC_RSP			((TMSG_TCODE_APB )|0x0002)
#define TMSG_APB_CLOSE_REQ			((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_APB_CLOSE_RSP			((TMSG_TCODE_APB )|0x0003)
#define TMSG_APB_INITIAL_REQ		((TMSG_TCODE_MAIN)|0x0004)
#define TMSG_APB_INITIAL_RSP		((TMSG_TCODE_APB )|0x0004)
#define TMSG_APB_FREERUN_REQ		((TMSG_TCODE_MAIN)|0x000F)
#define TMSG_APB_FREERUN_RSP		((TMSG_TCODE_MAIN)|0x000F)
#define TMSG_APB_STATUS_REQ			((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_APB_STATUS_INFO		((TMSG_TCODE_APB )|TMSG_COMMON_STATUS)

/* Shutter Task */
#define TMSG_SHUTTER_INITIAL_OPEN_REQ	((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_SHUTTER_INITIAL_OPEN_RSP	((TMSG_TCODE_SHUTTER )|0x0001)
#define TMSG_SHUTTER_OPEN_REQ			((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_SHUTTER_OPEN_RSP			((TMSG_TCODE_SHUTTER )|0x0002)
#define TMSG_SHUTTER_CLOSE_REQ			((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_SHUTTER_CLOSE_RSP			((TMSG_TCODE_SHUTTER )|0x0003)
#define TMSG_SHUTTER_FREERUN_REQ		((TMSG_TCODE_MAIN)|0x0004)
#define TMSG_SHUTTER_FREERUN_RSP		((TMSG_TCODE_SHUTTER)|0x0004)
#define TMSG_SHUTTER_STATUS_REQ			((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_SHUTTER_STATUS_INFO		((TMSG_TCODE_SHUTTER )|TMSG_COMMON_STATUS)



/* STACKER Task */
#define TMSG_STACKER_HOME_REQ		((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_STACKER_HOME_RSP		((TMSG_TCODE_STAC)|0x0001)
#define TMSG_STACKER_EXEC_REQ		((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_STACKER_EXEC_RSP		((TMSG_TCODE_STAC)|0x0002)
#define TMSG_STACKER_HALF_REQ		((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_STACKER_HALF_RSP		((TMSG_TCODE_STAC)|0x0003)
#define TMSG_STACKER_PULL_REQ			((TMSG_TCODE_MAIN)|0x0004)//押し込みが問題ない時の戻し動作
#define TMSG_STACKER_PULL_RSP			((TMSG_TCODE_STAC)|0x0004)
#define TMSG_STACKER_EXEC_RE_REQ		((TMSG_TCODE_MAIN)|0x0005)
#define TMSG_STACKER_EXEC_RE_RSP		((TMSG_TCODE_STAC)|0x0005)
#define TMSG_STACKER_EXEC_NG_PULL_REQ	((TMSG_TCODE_MAIN)|0x0006)//押し込みが問題の時の戻し動作
#define TMSG_STACKER_EXEC_NG_PULL_RSP	((TMSG_TCODE_STAC)|0x0006)

#define TMSG_STACKER_FREERUN_REQ	((TMSG_TCODE_MAIN)|0x000F)
#define TMSG_STACKER_FREERUN_RSP	((TMSG_TCODE_STAC)|0x000F)
#define TMSG_STACKER_STATUS_REQ		((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_STACKER_STATUS_INFO	((TMSG_TCODE_STAC)|TMSG_COMMON_STATUS)

#if defined(UBA_RTQ)
#define TMSG_STACKER_BACK_REQ		((TMSG_TCODE_MAIN)|0x0010)
#define TMSG_STACKER_BACK_RSP		((TMSG_TCODE_STAC)|0x0010)
#endif // UBA_RTQ


/* DISCRIMINATION Task */
#define TMSG_VALIDATION_REQ			((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_VALIDATION_RSP			((TMSG_TCODE_DISC)|0x0001)
#define TMSG_VALIDATION_REVERSE_REQ	((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_VALIDATION_REVERSE_RSP	((TMSG_TCODE_DISC)|0x0002)
#define TMSG_SIGNATURE_REQ          ((TMSG_TCODE_MAIN)|0x0003)
#define TMSG_SIGNATURE_RSP          ((TMSG_TCODE_DISC)|0x0003)
#define TMSG_CIS_INITIALIZE_REQ		((TMSG_TCODE_MAIN)|0x0004)
#define TMSG_CIS_INITIALIZE_RSP		((TMSG_TCODE_DISC)|0x0004)
#define TMSG_DATA_COLLECTION_REQ	((TMSG_TCODE_MAIN)|0x000A)
#define TMSG_DATA_COLLECTION_RSP	((TMSG_TCODE_DISC)|0x000A)
#define TMSG_IMAGE_INITIALIZE_REQ		((TMSG_TCODE_MAIN)|0x000B)
#define TMSG_IMAGE_INITIALIZE_RSP		((TMSG_TCODE_DISC)|0x000B)
#define TMSG_VALIDATION_STATUS_REQ	((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_VALIDATION_STATUS_INFO	((TMSG_TCODE_DISC)|TMSG_COMMON_STATUS)

enum _MAG_INIT_MODE
{
	MAG_INIT_MODE_POWERUP = 1,
	MAG_INIT_MODE_STANDBY,
};

enum _SIGNATURE_ALGORITHM
{
	SIGNATURE_NONE = 0,
	SIGNATURE_CRC16,
	SIGNATURE_CRC32,
	SIGNATURE_SHA1
};

#if defined(UBA_RTQ)	/* '18-05-01 */
/* RC Task */
#define TMSG_RC_INIT_REQ					((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_RC_INIT_RSP					((TMSG_TCODE_RC)  |0x0001)

#define TMSG_RC_DEL_REQ						((TMSG_TCODE_MAIN)|0x0010)
#define TMSG_RC_DEL_RSP						((TMSG_TCODE_RC)  |0x0010)
//not use #define TMSG_RC_STATUS_REQ					((TMSG_TCODE_MAIN)|0x0011)
//not use #define TMSG_RC_STATUS_RSP					((TMSG_TCODE_RC)  |0x0011)
#define TMSG_RC_RESET_REQ					((TMSG_TCODE_MAIN)|0x0040)
#define TMSG_RC_RESET_RSP					((TMSG_TCODE_RC)  |0x0040)
#define TMSG_RC_RESET_SKIP_REQ				((TMSG_TCODE_MAIN)|0x0041)
#define TMSG_RC_RESET_SKIP_RSP				((TMSG_TCODE_RC)  |0x0041)
#define TMSG_RC_CANCEL_REQ					((TMSG_TCODE_MAIN)|0x0045)
#define TMSG_RC_CANCEL_RSP					((TMSG_TCODE_RC)  |0x0045)
#define TMSG_RC_REJECT_REQ					((TMSG_TCODE_MAIN)|0x0046)
#define TMSG_RC_REJECT_RSP					((TMSG_TCODE_RC)  |0x0046)
#define TMSG_RC_PAUSE_REQ					((TMSG_TCODE_MAIN)|0x0047)
#define TMSG_RC_PAUSE_RSP					((TMSG_TCODE_RC)  |0x0047)
#define TMSG_RC_PAYOUT_REQ					((TMSG_TCODE_MAIN)|0x0048)
#define TMSG_RC_PAYOUT_RSP					((TMSG_TCODE_RC)  |0x0048)
#define TMSG_RC_STACK_REQ					((TMSG_TCODE_MAIN)|0x0049)
#define TMSG_RC_STACK_RSP					((TMSG_TCODE_RC)  |0x0049)
#define TMSG_RC_COLLECT_REQ					((TMSG_TCODE_MAIN)|0x004A)
#define TMSG_RC_COLLECT_RSP					((TMSG_TCODE_RC)  |0x004A)
#define	TMSG_RC_SOL_REQ						((TMSG_TCODE_MAIN)|0x004B)
#define	TMSG_RC_SOL_RSP						((TMSG_TCODE_RC)  |0x004B)
#define TMSG_RC_FEED_REQ					((TMSG_TCODE_MAIN)|0x004C)
#define TMSG_RC_FEED_RSP					((TMSG_TCODE_RC)  |0x004C)
#define TMSG_RC_FLAPPER_REQ					((TMSG_TCODE_MAIN)|0x004D)
#define TMSG_RC_FLAPPER_RSP					((TMSG_TCODE_RC)  |0x004D)
#define TMSG_RC_SENSOR_REQ					((TMSG_TCODE_MAIN)|0x004E)
#define TMSG_RC_SENSOR_RSP					((TMSG_TCODE_RC)  |0x004E)
#define TMSG_RC_DRUM_REQ					((TMSG_TCODE_MAIN)|0x004F)
#define TMSG_RC_DRUM_RSP					((TMSG_TCODE_RC)  |0x004F)
#define TMSG_RC_DISPLAY_REQ					((TMSG_TCODE_MAIN)|0x0050)
#define TMSG_RC_DISPLAY_RSP					((TMSG_TCODE_RC)  |0x0050)
#define TMSG_RC_WU_RESET_REQ				((TMSG_TCODE_MAIN)|0x0051)
#define TMSG_RC_WU_RESET_RSP				((TMSG_TCODE_RC)  |0x0051)
#define	TMSG_RC_SENSOR_ACTIVE_REQ			((TMSG_TCODE_MAIN)|0x0052)
#define	TMSG_RC_SENSOR_ACTIVE_RSP			((TMSG_TCODE_RC)  |0x0052)
#define	TMSG_RC_SENSOR_CONDITION_REQ		((TMSG_TCODE_MAIN)|0x0053)
#define	TMSG_RC_SENSOR_CONDITION_RSP		((TMSG_TCODE_RC)  |0x0053)
#define TMSG_RC_WRITE_SERIALNO_REQ			((TMSG_TCODE_MAIN)|0x0060)
#define TMSG_RC_WRITE_SERIALNO_RSP			((TMSG_TCODE_RC)  |0x0060)
#define TMSG_RC_READ_SERIALNO_REQ			((TMSG_TCODE_MAIN)|0x0061)
#define TMSG_RC_READ_SERIALNO_RSP			((TMSG_TCODE_RC)  |0x0061)
#define TMSG_RC_WRITE_MAINTE_SERIALNO_REQ	((TMSG_TCODE_MAIN)|0x0070)
#define TMSG_RC_WRITE_MAINTE_SERIALNO_RSP	((TMSG_TCODE_RC)  |0x0070)
#define TMSG_RC_READ_MAINTE_SERIALNO_REQ	((TMSG_TCODE_MAIN)|0x0071)
#define TMSG_RC_READ_MAINTE_SERIALNO_RSP	((TMSG_TCODE_RC)  |0x0071)
#define TMSG_RC_DL_START_REQ				((TMSG_TCODE_MAIN)|0x0080)
#define TMSG_RC_DL_START_RSP				((TMSG_TCODE_RC)  |0x0080)
#define TMSG_RC_DL_DATA_REQ					((TMSG_TCODE_MAIN)|0x0081)
#define TMSG_RC_DL_DATA_RSP					((TMSG_TCODE_RC)  |0x0081)
#define TMSG_RC_DL_CHECK_REQ				((TMSG_TCODE_MAIN)|0x0082)
#define TMSG_RC_DL_CHECK_RSP				((TMSG_TCODE_RC)  |0x0082)
#define TMSG_RC_DL_END_REQ					((TMSG_TCODE_MAIN)|0x0083)
#define TMSG_RC_DL_END_RSP					((TMSG_TCODE_RC)  |0x0083)
#define	TMSG_RC_POLL_CHANGE_REQ				((TMSG_TCODE_MAIN)|0x0085)
#define	TMSG_RC_POLL_CHANGE_RSP				((TMSG_TCODE_RC)  |0x0085)
#define	TMSG_RC_REWAIT_READY_REQ			((TMSG_TCODE_MAIN)|0x0086)
#define	TMSG_RC_REWAIT_READY_RSP			((TMSG_TCODE_RC)  |0x0086)
#define TMSG_RC_WRITE_EDITION_REQ			((TMSG_TCODE_MAIN)|0x0090)
#define TMSG_RC_WRITE_EDITION_RSP			((TMSG_TCODE_RC)  |0x0090)
#define TMSG_RC_READ_EDITION_REQ			((TMSG_TCODE_MAIN)|0x0091)
#define TMSG_RC_READ_EDITION_RSP			((TMSG_TCODE_RC)  |0x0091)
#define TMSG_RC_DIAG_END_REQ				((TMSG_TCODE_MAIN)|0x0095)
#define TMSG_RC_DIAG_END_RSP				((TMSG_TCODE_RC)  |0x0095)
#define TMSG_RC_DIAG_MOT_FWD_REQ			((TMSG_TCODE_MAIN)|0x0096)
#define TMSG_RC_DIAG_MOT_FWD_RSP			((TMSG_TCODE_RC)  |0x0096)
#define TMSG_RC_GET_RECYCLE_SETTING_REQ		((TMSG_TCODE_MAIN)|0x00A9)
#define TMSG_RC_GET_RECYCLE_SETTING_RSP		((TMSG_TCODE_RC)  |0x00A9)
#define TMSG_RC_RESUME_REQ					((TMSG_TCODE_MAIN)|0x00AB)
#define TMSG_RC_RESUME_RSP					((TMSG_TCODE_RC)  |0x00AB)
#define TMSG_RC_VERSION_REQ					((TMSG_TCODE_MAIN)|0x00AC)
#define TMSG_RC_VERSION_RSP					((TMSG_TCODE_RC)  |0x00AC)
#define	TMSG_RC_ERROR_CLEAR_REQ				((TMSG_TCODE_MAIN)|0x00AD)
#define	TMSG_RC_ERROR_CLEAR_RSP				((TMSG_TCODE_RC)  |0x00AD)
#define TMSG_RC_SW_COLLECT_REQ				((TMSG_TCODE_MAIN)|0x00AE)
#define TMSG_RC_SW_COLLECT_RSP				((TMSG_TCODE_RC)  |0x00AE)
#define TMSG_RC_ERROR_DETAIL_REQ			((TMSG_TCODE_MAIN)|0x00B0)
#define TMSG_RC_ERROR_DETAIL_RSP			((TMSG_TCODE_RC)  |0x00B0)
#define TMSG_RC_GET_DIPSW_REQ				((TMSG_TCODE_MAIN)|0x00B1)
#define TMSG_RC_GET_DIPSW_RSP				((TMSG_TCODE_RC)  |0x00B1)
#define TMSG_RC_BOOT_VERSION_REQ			((TMSG_TCODE_MAIN)|0x00C0)
#define TMSG_RC_BOOT_VERSION_RSP			((TMSG_TCODE_RC)  |0x00C0)
#define TMSG_RC_RECYCLE_ENABLE_REQ			((TMSG_TCODE_MAIN)|0x00C8)
#define TMSG_RC_RECYCLE_ENABLE_RSP			((TMSG_TCODE_RC)  |0x00C8)
#define TMSG_RC_RECYCLE_SETTING_REQ			((TMSG_TCODE_MAIN)|0x00C9)
#define TMSG_RC_RECYCLE_SETTING_RSP			((TMSG_TCODE_RC)  |0x00C9)
#define TMSG_RC_CURRENT_COUNT_SETTING_REQ	((TMSG_TCODE_MAIN)|0x00CA)
#define TMSG_RC_CURRENT_COUNT_SETTING_RSP	((TMSG_TCODE_RC)  |0x00CA)
#define TMSG_RC_SET_MOTOR_SPEED_REQ			((TMSG_TCODE_MAIN)|0x00CB)
#define TMSG_RC_SET_MOTOR_SPEED_RSP			((TMSG_TCODE_RC)  |0x00CB)
#define TMSG_RC_RETRY_BILL_DIR_REQ			((TMSG_TCODE_MAIN)|0x00D0)
#define TMSG_RC_RETRY_BILL_DIR_RSP			((TMSG_TCODE_RC)  |0x00D0)
#define TMSG_RC_FEED_BOX_REQ				((TMSG_TCODE_MAIN)|0x00D1)
#define TMSG_RC_FEED_BOX_RSP				((TMSG_TCODE_RC)  |0x00D1)
#define TMSG_RC_DRUM_GAP_ADJ_REQ			((TMSG_TCODE_MAIN)|0x00D2)
#define TMSG_RC_DRUM_GAP_ADJ_RSP			((TMSG_TCODE_RC)  |0x00D2)
#define TMSG_RC_BILLBACK_DRUM_PAYOUT_REQ	((TMSG_TCODE_MAIN)|0x00D3)
#define TMSG_RC_BILLBACK_DRUM_PAYOUT_RSP	((TMSG_TCODE_RC)  |0x00D3)
#define TMSG_RC_FEEDBOX_DRUM_PAYOUT_REQ		((TMSG_TCODE_MAIN)|0x00D4)
#define TMSG_RC_FEEDBOX_DRUM_PAYOUT_RSP		((TMSG_TCODE_RC)  |0x00D4)
#define TMSG_RC_SEARCH_REQ					((TMSG_TCODE_MAIN)|0x00D5)
#define TMSG_RC_SEARCH_RSP					((TMSG_TCODE_RC)  |0x00D5)
#define TMSG_RC_PREFEED_STACK_REQ			((TMSG_TCODE_MAIN)|0x00D6)
#define TMSG_RC_PREFEED_STACK_RSP			((TMSG_TCODE_RC)  |0x00D6)
#define TMSG_RC_LAST_FEED_CASHBOX_REQ		((TMSG_TCODE_MAIN)|0x00D7)
#define TMSG_RC_LAST_FEED_CASHBOX_RSP		((TMSG_TCODE_RC)  |0x00D7)
#define TMSG_RC_FORCE_STACK_DRUM_REQ		((TMSG_TCODE_MAIN)|0x00D8)
#define TMSG_RC_FORCE_STACK_DRUM_RSP		((TMSG_TCODE_RC)  |0x00D8)
#define TMSG_RC_BILLBACK2_REQ				((TMSG_TCODE_MAIN)|0x00D9)
#define TMSG_RC_BILLBACK2_RSP				((TMSG_TCODE_RC)  |0x00D9)
#define TMSG_RC_STATE_REQ					((TMSG_TCODE_MAIN)|0x00DA)
#define TMSG_RC_STATE_RSP					((TMSG_TCODE_RC)  |0x00DA)
#define TMSG_RC_MODE_REQ					((TMSG_TCODE_MAIN)|0x00DB)
#define TMSG_RC_MODE_RSP					((TMSG_TCODE_RC)  |0x00DB)
//#if defined(UBA_RS)
	#define TMSG_RS_FLAPPER_REQ					((TMSG_TCODE_MAIN)|0x00E0)
	#define TMSG_RS_FLAPPER_RSP					((TMSG_TCODE_RC)  |0x00E0)
	#define TMSG_RS_DISPLAY_REQ					((TMSG_TCODE_MAIN)|0x00E1)
	#define TMSG_RS_DISPLAY_RSP					((TMSG_TCODE_RC)  |0x00E1)
//#endif
#define TMSG_RC_FRAM_READ_REQ				((TMSG_TCODE_MAIN)|0x00F1)
#define TMSG_RC_FRAM_READ_RSP				((TMSG_TCODE_RC)  |0x00F1)
#define TMSG_RC_FRAM_CHECK_REQ				((TMSG_TCODE_MAIN)|0x00F2)
#define TMSG_RC_FRAM_CHECK_RSP				((TMSG_TCODE_RC)  |0x00F2)
#define TMSG_RC_DRUM_TAPE_POS_ADJ_REQ		((TMSG_TCODE_MAIN)|0x00F3)
#define TMSG_RC_DRUM_TAPE_POS_ADJ_RSP		((TMSG_TCODE_RC)  |0x00F3)
#define TMSG_RC_START_SENS_ADJ_REQ			((TMSG_TCODE_MAIN)|0x00F4)
#define TMSG_RC_START_SENS_ADJ_RSP			((TMSG_TCODE_RC)  |0x00F4)
#define TMSG_RC_READ_SENS_ADJ_DATA_REQ		((TMSG_TCODE_MAIN)|0x00F5)
#define TMSG_RC_READ_SENS_ADJ_DATA_RSP		((TMSG_TCODE_RC)  |0x00F5)

#define TMSG_RC_SENS_ADJ_WRITE_FRAM_REQ		((TMSG_TCODE_MAIN)|0x00F6)
#define TMSG_RC_SENS_ADJ_WRITE_FRAM_RSP		((TMSG_TCODE_RC)  |0x00F6)
#define TMSG_RC_SENS_ADJ_READ_FRAM_REQ		((TMSG_TCODE_MAIN)|0x00F7)
#define TMSG_RC_SENS_ADJ_READ_FRAM_RSP		((TMSG_TCODE_RC)  |0x00F7)
#define TMSG_RC_PERFORM_TEST_WRITE_FRAM_REQ	((TMSG_TCODE_MAIN)|0x00F8)
#define TMSG_RC_PERFORM_TEST_WRITE_FRAM_RSP	((TMSG_TCODE_RC)  |0x00F8)
#define TMSG_RC_PERFORM_TEST_READ_FRAM_REQ	((TMSG_TCODE_MAIN)|0x00F9)
#define TMSG_RC_PERFORM_TEST_READ_FRAM_RSP	((TMSG_TCODE_RC)  |0x00F9)
#define TMSG_RC_ERROR_COUNT_CLEAR_REQ		((TMSG_TCODE_MAIN)|0x00FA)
#define TMSG_RC_ERROR_COUNT_CLEAR_RSP		((TMSG_TCODE_RC)  |0x00FA)

#define TMSG_RC_GET_MOTOR_SPEED_REQ			((TMSG_TCODE_MAIN)|0x00FC)
#define TMSG_RC_GET_MOTOR_SPEED_RSP			((TMSG_TCODE_RC)  |0x00FC)

//#if defined(RC_BOARD_GREEN)
	#define TMSG_RC_GET_POS_ONOFF_REQ			((TMSG_TCODE_MAIN)|0x00A0)
	#define TMSG_RC_GET_POS_ONOFF_RSP			((TMSG_TCODE_RC)  |0x00A0)
	#define TMSG_RC_SET_POS_DA_REQ				((TMSG_TCODE_MAIN)|0x00A1)
	#define TMSG_RC_GET_POS_DA_RSP				((TMSG_TCODE_RC)  |0x00A1)
	#define TMSG_RC_GET_POS_GAIN_REQ			((TMSG_TCODE_MAIN)|0x00A2)
	#define TMSG_RC_GET_POS_GAIN_RSP			((TMSG_TCODE_RC)  |0x00A2)
	#define TMSG_RC_GET_CONFIGURATION_REQ		((TMSG_TCODE_MAIN)|0x00A8)
	#define TMSG_RC_GET_CONFIGURATION_RSP		((TMSG_TCODE_RC)  |0x00A8)
//#endif // RC_BOARD_GREEN
//#if defined(UBA_RTQ_ICB)
	#define TMSG_RC_RFID_TEST_REQ				((TMSG_TCODE_MAIN)|0x00E6) //生産で使用
	#define TMSG_RC_RFID_TEST_RSP				((TMSG_TCODE_RC)  |0x00E6) //生産で使用
	#define TMSG_RC_RFID_READ_REQ				((TMSG_TCODE_MAIN)|0x00E7)
	#define TMSG_RC_RFID_READ_RSP				((TMSG_TCODE_RC)  |0x00E7)
	#define TMSG_RC_RFID_WRITE_REQ				((TMSG_TCODE_MAIN)|0x00E8)
	#define TMSG_RC_RFID_WRITE_RSP				((TMSG_TCODE_RC)  |0x00E8)
	#define TMSG_RC_RFID_RESET_REQ				((TMSG_TCODE_MAIN)|0x00E9)
	#define TMSG_RC_RFID_RESET_RSP				((TMSG_TCODE_RC)  |0x00E9)
//#endif // UBA_RTQ_ICB
#define TMSG_RC_DISCONNECT_INFO				((TMSG_TCODE_MAIN)|0x00FE)
#define TMSG_RC_STATUS_INFO					((TMSG_TCODE_MAIN)|0x00FF)

#define	TMSG_RC_ENC_KEY						0x01					/* Encryption Key			*/
#define	TMSG_RC_ENC_NUM						0x02					/* Encryption Number		*/
#define	TMSG_RC_ENC_KEY_LENGTH				0x0D					/* Encryption Key length	*/
#define	TMSG_RC_ENC_NUM_LENGTH				0x0D					/* Encryption Number length	*/

#define	RC_GC2_BARREL_R(x)					(((x) >> 3) | ((x) << 5))
#define	RC_GC2_BARREL_L(x)					(((x) << 3) | ((x) >> 5))


#endif


#if defined(UBA_RTQ)	/* '19-04-19 */
// RCリカバリ
enum _RC_RECOVERY_TYPE
{
	GAP_ADJ_DRUM = 1,	//not use
	BILLBACK_FEEDBOX,	//use main_task
	FEED_BOX,			//use main_task
	BILLBACK_PAYDRUM_FEED_BOX_FOR_STACK,	//use main_task
	BILLBACK_PAYDRUM_FEED_BOX_FOR_PAYOUT, 	//use main_task
	BILL_FRONT_BACK_BOX,	//not use
	PAYDRUM_FEED_BOX,		 //use main_task
	BOX_SEARCH_FEED_BOX,	//not use
	BILL_DRUM_JAM,			//not use
	BILL_CHEAT,				//use ID003, main_task
};

#define SEARCH_BILL_IN  0
#define SEARCH_BILL_OUT 1

/* RC UART Task */
#define TMSG_RC_UART_CB_CALLBACK_INFO			((TMSG_TCODE_RC_UARTCB)|TMSG_COMMON_STATUS)
#endif
/*----------------------------------------------------------*/
/*			Event Flag										*/
/*----------------------------------------------------------*/
#define EVT_ALL_BIT						0xFFFFFFFF

/*----- Feed Motor control ------------------------------*/

#define EVT_FEED_SENSOR					0x00000001			/* sensor detection */
#define EVT_FEED_OVER_PULSE				0x00000002			/* over specified pulse */

#define EVT_FEED_MOTOR_STOP				0x00000100			/* feed motor stop */
#define EVT_FEED_TIMEOUT				0x00010000			/* time out */
#define EVT_FEED_MOTOR_LOCK				0x00020000			/* feed motor lock */
#define EVT_FEED_MOTOR_RUNAWAY			0x00040000			/* feed motor runaway */

#define	EVT_FEED_CIS_SKEW				0x00080000			/* cis sensor edge sampled skew */
#define	EVT_FEED_CIS_MLT				0x00100000			/* cis sensor mlt sampled */
#define EVT_FEED_CIS_SHORT_EDGE			0x00200000			/* cis sensor edge short paper */
#define EVT_FEED_CIS 					0x00300000			/* feed motor lock *///2024-01-30

/*----- Stacker Motor control ------------------------------*/
#define STACKER_MOTOR_LOCK_TIME			50					/* 50msec */

#define EVT_STACKER_SENSOR				0x00000001			/* sensor detection */
#define EVT_STACKER_DRIVE_PULSE_OVER	0x00000002			/* over specified pulse */
#define EVT_STACKER_MOTOR_STOP			0x00000100			/* stacker motor stop */
#define EVT_STACKER_TIMEOUT				0x00010000			/* time out */
#define EVT_STACKER_MOTOR_LOCK			0x00020000			/* stacker motor lock */
#define EVT_STACKER_MOTOR_RUNAWAY		0x00040000			/* stacker motor runaway */

/*----- Centering Motor control ----------------------------*/
#define CENTERING_MOTOR_MAX_CURRENT		84					/*  1.08V(0-3.3V) 0.52A*/
#define EVT_CENTERING_SENSOR				0x00000001			/* sensor detection */
#define EVT_CENTERING_HOME_INTR				0x00000004			/* home interrupt */
#define EVT_CENTERING_RUN_TIMEOUT			0x00000002			
#define EVT_CENTERING_HOME_OUT_INTR			0x00000008			/* home out */
#define EVT_CENTERING_HOME_OUT_INTR_10MSEC	0x00000020			/* 10msec home out *//* ���悹close��p *//* 2021-05-27	*/


#define EVT_CENTERING_MOTOR_STOP		0x00000100			/* stacker motor stop */
#define EVT_CENTERING_TIMEOUT			0x00010000			/* time out */
#define EVT_CENTERING_MOTOR_RUNAWAY		0x00040000			/* stacker motor runaway */

/*----- APB Motor control ----------------------------------*/
#define PB_MOTOR_MAX_CURRENT			84					/*  1.08V(0-3.3V) 0.52A	*/
#define EVT_APB_SENSOR					0x00000001			/* sensor detection */
#define EVT_APB_OVER_PULSE				0x00000002			/* over specified pulse */
#define EVT_APB_HOME_INTR				0x00000004			/* home interrupt */

#define EVT_APB_MOTOR_STOP				0x00000100			/* stacker motor stop */
#define EVT_APB_TIMEOUT					0x00010000			/* time out */
#define EVT_APB_REV_TIME				0x00000200			/* reverse time */



/*----- Shutter Motor control ----------------------------------*/
#define SHUTTER_MOTOR_MAX_CURRENT			84					/* 1.08A:(0-3.3V) 0.52A*/
/*----- Shutter Motor control ----------------------------------*/
#define EVT_SHUTTER_SENSOR					0x00000001			/* sensor detection */
#define EVT_SHUTTER_MOTOR_STOP				0x00000100			/* stacker motor stop */
#define EVT_SHUTTER_TIMEOUT					0x00010000			/* time out */

/*----- FRAM ----------------------------------*/
#define EVT_FRAM_WRITE					0x00000001			/* 2023-12-04 */
#define FEED_MOTOR_LOCK_TIME			400					/* 400msec */


/*----- Sensor control -------------------------------------*/
#define	EVT_SENSOR_INIT					0x00000001			/* sensor init */
#define	EVT_SENSOR_SHIFT				0x00000002			/* sensor shift */
#define	EVT_SENSOR_POSITION_AD			0x00000004			/* done position ad */

/*----- Sensor Sequence Code ----------------------------------*/
#define SENSOR_SEQ_INIT					0x4100
#define SENSOR_SEQ_LED_ON				0x4200
#define SENSOR_SEQ_POSI_AD				0x4300
#define SENSOR_SEQ_LED_BLINK			0x4400
#define SENSOR_SEQ_POSI_ADJ				0x4600
#define SENSOR_SEQ_POSI_ADJ_DETECT		0x4700


/*----- RFID control -------------------------------------*/
#define	EVT_RFID_EMPTY					0x00000001			/* RFID send complete */
#define	EVT_RFID_RCV					0x00000002			/* RFID received */
#define	EVT_RFID_ERR					0x00000004			/* RFID error */

/*----- SDC control -------------------------------------*/
#define	EVT_SDC_INSERT					0x00000001			/* SD card detect */
#define	EVT_SDC_MOUNT					0x00000002			/* SD card mount */
#define	EVT_SDC_EJECT					0x00000004			/* SD card eject */

/*----- ICB control -------------------------------------*/
#define	EVT_ICB_INIT					0x00000001			/* ICB init */
#define	EVT_ICB_DATA_READ				0x00000002			/* ICB Data read complete */
#define	EVT_ICB_DATA_WRITE				0x00000004			/* ICB Data write complete */
#define	EVT_ICB_DATA_READ_FAIL			0x00000008			/* ICB Data read error */
#define	EVT_ICB_DATA_WRITE_FAIL			0x00000010			/* ICB Data write error */
#define EVT_ICB_TIMEOUT					0x00010000			/* time out */
//#define	EVT_FRAM_DATA_READ				0x00020000			/* FRAM Data read complete */
#define	EVT_FRAM_DATA_WRITE				0x00040000			/* FRAM Data write complete */
//#define	EVT_FRAM_DATA_READ_FAIL			0x00080000			/* FRAM Data read error */
#define	EVT_FRAM_DATA_WRITE_FAIL		0x00100000			/* FRAM Data write error */

enum _ValidationStart
{
	VALIDATION_STOP = 0,
	VALIDATION_STRT,
};

/*----------------------------------------------------------*/
/*			PERIPHERAL SETTINGS								*/
/*----------------------------------------------------------*/
#define I2C_RETRY_COUNT		10

/*----------------------------------------------------------*/
/*			FPGA CONTROL									*/
/*----------------------------------------------------------*/
#define PL_DISABLE				0
#define PL_ENABLE				1

/*----------------------------------------------------------*/
/*			LOG FUNCTION									*/
/*----------------------------------------------------------*/
#define _ENABLE_JDL       1

/*----------------------------------------------------------*/
/*			TICKET											*/
/*----------------------------------------------------------*/
enum _BarcodeType
{
	BARCODE_TYPE_ICB = 0,
	BARCODE_TYPE_TITO =1,
	BARCODE_TYPE_QR = 4,
	BARCODE_TYPE_COUNT,
	BARCODE_TYPE_INVALID,
};

enum _BarcodeSide
{
	BARCODE_SIDE_UP = 0,
	BARCODE_SIDE_DOWN,
	BARCODE_SIDE_BOTH,
	BARCODE_SIDE_COUNT
};

/*----------------------------------------------------------*/
/*			ICB												*/
/*----------------------------------------------------------*/

#define	ICB_SELECT_BY_TICKET	11
#define	ICB_NO_SELECT			12
#define	ICB_SUB_FUNCTION		ICB_SELECT_BY_TICKET

/*----------------------------------------------------------*/
/*			DEBUG CONTROL									*/
/*----------------------------------------------------------*/
#include "debug.h"
#define DUMMY_ACCESS(p) ((void *)p)


/*----------------------------------------------------------*/
/*			SYSTEM CLOCK									*/
/*----------------------------------------------------------*/
// 20211215 ThreadX System Timer MPU →　OSC
/* use OSC0 timer */
#define SYS_TIMER_INT ALT_INT_INTERRUPT_TIMER_OSC1_0_IRQ
#define SYS_TIMER ALT_GPT_OSC1_TMR0
#define _DEBUG_MPU_CLOCK (400*1000*1000)


enum _MOTOR_STATE
{
	MOTOR_STATE_STOP = 1,
	MOTOR_STATE_BRAKE,
	MOTOR_STATE_FWD,
	MOTOR_STATE_FWD_WAIT,
	MOTOR_STATE_REV,
	MOTOR_STATE_REV_WAIT,
};

//2024-05-28
#define CIS_TEMP_ABN_THD_SET	63	/* 63℃ */
#define CIS_TEMP_ABN_THD_CLEAR	61	/* 61℃ */


#endif /* SRC_INCLUDE_COMMON_H_ */

