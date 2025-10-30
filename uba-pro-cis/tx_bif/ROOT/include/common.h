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
#include "custom.h"	//2023-01-27
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

#define ES_MODEL	0
#define WS_MODEL	1
#define WS2_MODEL	2
#define CS_MODEL	3
#define MP_MODEL	100

#define DATA_COLLECTION_DEBUG		1


#if defined(PRJ_IVIZION2)
	//#define BV_UNIT_TYPE ES_MODEL
	//#define BV_UNIT_TYPE 	WS_MODEL
	//#define BV_UNIT_TYPE 	WS2_MODEL
	#define BV_UNIT_TYPE 	CS_MODEL
#else
	#define BV_UNIT_TYPE CS_MODEL
#endif

// Section Header Symbol
#if (defined(PRJ_IVIZION2) && (BV_UNIT_TYPE>=WS2_MODEL))
	#define JCM_PRODUCT_ID		0x0119
	#define BA_SYMBOL		"VFM20   "
#else
	#define JCM_PRODUCT_ID		0x011B
	#define BA_SYMBOL		"VFM21   "
#endif
// Endian swap
#define SWAP_ENDIAN(adr)	( (((u32)(adr) & 0xff000000) >> 24) | (((u32)(adr) & 0xff0000) >> 8) | (((u32)(adr) & 0xff00) << 8) | (((u32)(adr) & 0xff) << 24) )

/*==========================================================*/
/*			Task wait time                                  */
/*==========================================================*/
#define TASK_WAIT_TIME			20	/* 20msec */

#if 1 //2023-12-25
	#define TASK_WAIT_TIME_DISPLAY	100	/* 100msec */
#else
	#define TASK_WAIT_TIME_DISPLAY	10	/* 10msec */
#endif

#define TASK_WAIT_TIME_BEZEL	50	/* 50msec */
#define TASK_WAIT_TIME_TIMER	10	/* 10msec */
#define TASK_WAIT_TIME_DLINE	2	/*  2msec */
#define TASK_WAIT_TIME_MGU		1000	/*  1000msec */
#define TASK_WAIT_TIME_DIP		200	/*  200msec */


/*==========================================================*/
/*			Event flag                                      */
/*==========================================================*/
/*----- UART Control ----------------------------------*/
#define EVT_UART_RCV					0x00000002			/* UART Received */
#define EVT_UART_EMP					0x00000004			/* UART Trans empty */
#define EVT_UART_ERR					0x00000008			/* UART Communication err */

/*----- USB Control ----------------------------------*/
#define EVT_USB_CON						0x00000001			/* USB Connect/Disconnect */
#define EVT_USB_RCV						0x00000002			/* USB Received */
#define EVT_USB_EMP						0x00000004			/* USB Trans empty */

/*----- OTG Download control ----------------------------------*/
#define EVT_OTG_DOWNLOAD_READY			0x00000001			/* OTG download file ready */

/*----- Front USB Connect Detect ----------------------------------*/
#define EVT_FUSB_DECT_INTR				0x00000001			/* Front USB connect */

/*----- Power Voltage control ----------------------------------*/
#define EVT_FRAM_VOLTAGE				0x00000001			/* fram write log */

/*----- SD control ----------------------------------*/
#define EVT_SD_SET						0x00000001			/* SD set */

/*----- SD control ----------------------------------*/
#define EVT_DET_RES						0x00000001			/* Reset Signal Detect */

/*----------------------------------------------------------*/
/*			Mode											*/
/*----------------------------------------------------------*/
/* Mode 1 Oparation */
#define MODE1_SYSINI					0x00


#define MODE1_DOWNLOAD					0x01
enum _DOWNLOAD_MODE2
{
	DOWNLOAD_MODE2_WAIT_REQ = 1,
	DOWNLOAD_MODE2_IF,
	DOWNLOAD_MODE2_USB,
	DOWNLOAD_MODE2_SUBLINE_USB,
};

#define MODE1_ALARM						0x0F
enum _ALARM_MODE2
{
	ALARM_MODE2_WAIT_REQ = 1,
	ALARM_MODE2_STACKER_FULL = 0x10,
	ALARM_MODE2_STACKER_JAM = 0x20,
	ALARM_MODE2_LOST_BILL = 0x30,
	ALARM_MODE2_FEED_SPEED = 0x50,
	ALARM_MODE2_FEED_FAIL = 0x60,
	ALARM_MODE2_ACCEPTOR_JAM = 0x80,
	ALARM_MODE2_CONFIRM_AT_JAM,
	ALARM_MODE2_APB_FAIL = 0x90,
	ALARM_MODE2_CHEAT= 0xC0,
	ALARM_MODE2_CENTERING_FAIL = 0xD0,
	ALARM_MODE2_CONFIRM_STACKER_JAM
};

/*----------------------------------------------------------*/
/*			Dip switch										*/
/*----------------------------------------------------------*/
#define DIPSW1			0x01
#define DIPSW2			0x02
#define DIPSW3			0x03

/* PERFORMANCE TEST MODE DIPSW */
#define DIPSW1_PERFORMANCE_TEST				0x80		/* performance test */

/* OPERATING MODE DIPSW */
#define DIPSW2_USE_			0x1		/* use  simulator */
/*----------------------------------------------------------*/
/*			Bezel LED										*/
/*----------------------------------------------------------*/
enum{
	BEZEL_LED1,
	BEZEL_LED2,
	BEZEL_LED3,
	BEZEL_LED4,
	BEZEL_LED5,
};
/*----------------------------------------------------------*/
/*			Status LED										*/
/*----------------------------------------------------------*/
enum DISP_PATTERN{
	STATUS_LED_OFF = 0,
	STATUS_LED_ON,
	STATUS_LED_BLINK
};

#if 1 //uba
	enum DISP_COLOR
	{
		DISP_COLOR_OFF = 0,
		DISP_COLOR_RED,
		DISP_COLOR_GREEN,
		DISP_COLOR_RED_GREEN,
		DISP_COLOR_RED_OFF,
		DISP_COLOR_GREEN_OFF,
		DISP_COLOR_INIT, //2023-01-31
		DISP_COLOR_WHITE,
		DIPS_COLOR_MAX
	} ;
#else
	enum DISP_COLOR
	{
		DISP_COLOR_OFF = 0,
		DISP_COLOR_RED = 1,
		DISP_COLOR_YELLOW,
		DISP_COLOR_GREEN,
		DISP_COLOR_BLUE,
		DISP_COLOR_PURPLE,
		DISP_COLOR_WHITE,
	};
#endif

#define DISP_COLOR_MODE_OFF		0x00	/* LED制御 OFF */
#define DISP_COLOR_MODE_ON		0x01	/* LED制御 ON */
#define DISP_COLOR_MODE_REJECT	0x02	/* LED制御 Display reject code */
#define DISP_COLOR_MODE_ALARM	0x03	/* LED制御 Display alarm code */
#define DISP_COLOR_MODE_DENOMI	0x10	/* LED制御 Display denomi code */

#define DISP_COLOR_OFF_COUNT	(s16)-3	/* 1s : 点滅表示再開までの時間 */

/*----------------------------------------------------------*/
/*			Watch dog										*/
/*----------------------------------------------------------*/
//Watchdog time out in 1000 ms
#define WDT_INTERRUPT_TIMEOUT 	1000000	/* 1s */
//Watchdog warn in 3 seconds
#define WDT_WARNING_VALUE 		2000000

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
/*			Alarm Code										*/
/*----------------------------------------------------------*/
enum _ALARM_CODE
{
	ALARM_CODE_OK = 0,
/*  */
	/* Initialization-related */
	ALARM_CODE_BOOT_AREA,							/* ROM check BOOT-AREA error */
	ALARM_CODE_BOOTIF_AREA,							/* ROM check BOOTIF-AREA error */
	ALARM_CODE_IF_AREA,								/* ROM check IF-AREA error */
	ALARM_CODE_COUNTRY_AREA,						/* ROM check Country-AREA error */
	ALARM_CODE_IN_RAM,								/* internal RAM check error */
	ALARM_CODE_EX_RAM,								/* external RAM check error */
	ALARM_CODE_ROM_WRITE,							/* ROM write error */
	ALARM_CODE_FRAM,								/* FRAM error */
	ALARM_CODE_DOWNLOAD,				/* 10 */	/* download file error */
	ALARM_CODE_MAG,									/* MAG setting error */
	ALARM_CODE_I2C,									/* I2C communication error */
	ALARM_CODE_SPI,									/* PS SPI communication error */
	ALARM_CODE_PL_SPI,								/* PL SPI communication error */
	ALARM_CODE_SETUP_WRONG,							/* setup wrong */
/* SYSTEM Error 1 (wait reset) */
	ALARM_CODE_STACKER_FULL,						/* stacker Full */
	ALARM_CODE_FEED_OTHER_SENSOR_SK,	/* 15 */	/* other sensor ON (JAM : bill in stacker) */
	ALARM_CODE_FEED_SLIP_SK,				        /* feed slip (JAM : bill in stackr) */
	ALARM_CODE_FEED_TIMEOUT_SK,						/* feed time out (JAM : bill in stackr) */
	ALARM_CODE_FEED_LOST_BILL,						/* lost bill */ /* <<<<<<<<<<<<<<<<<<<<<<<< JAM : bill in stacker か要検討 >>>>>>>>>>>>>>>>>>>>>>>>>>> */
	ALARM_CODE_FEED_MOTOR_LOCK_SK,		/* 20 */	/* feed motor lock (JAM : bill in stackr) */
	ALARM_CODE_FEED_MOTOR_SPEED_LOW,	/* 25 */	/* feed motor low speed */
	ALARM_CODE_FEED_MOTOR_SPEED_HIGH,		        /* feed motor high speed */
	ALARM_CODE_FEED_MOTOR_LOCK,						/* feed motor lock */
	ALARM_CODE_FEED_NFAULT,							/* motor driver N-Fault (at feed sequence) */
	ALARM_CODE_STACKER_MOTOR_LOCK,		/* 20 */	/* stacker motor lock */
	ALARM_CODE_STACKER_NFAULT,						/* motor driver N-Fault (at stack sequence) */
	ALARM_CODE_STACKER_GEAR,						/* stacker gear */
	ALARM_CODE_STACKER_TIMEOUT,						/* stacker time out */
	/* recycle processing for ADP >>>>>>>>>>>>>>>>>>>>>>>>>>*/
	ALARM_CODE_DISPENSE_STACKER_JAM,	/* 30 */	/* dispense jam in stacker */
	ALARM_CODE_DISPENSE_NO_NOTE,			        /* bill not come to head exit sensor */
	ALARM_CODE_DISPENSE_NOTE_REMOVE,				/* bill not come to head exit sensor */
/* SYSTEM Error 2 (auto recovery) */
	ALARM_CODE_STACKER_HOME,						/* stacker pusher home position error */
	ALARM_CODE_FEED_OTHER_SENSOR_AT,				/* other sensor ON (acceptor JAM) */
	ALARM_CODE_FEED_SLIP_AT,						/* feed slip (acceptor JAM) */
	ALARM_CODE_FEED_TIMEOUT_AT,			/* 35 */	/* feed time out (acceptor JAM) */
	ALARM_CODE_FEED_MOTOR_LOCK_AT,					/* feed motor lock (acceptor JAM) */
	ALARM_CODE_FEED_NFAULT_AT,						/* motor driver N-Fault (acceptor JAM) */
	ALARM_CODE_APB_TIMEOUT,							/* APB time out */
	ALARM_CODE_APB_HOME,							/* APB home position error */
	ALARM_CODE_APB_HOME_REMOVAL,		/* 40 */	/* APB home position removal error */
	ALARM_CODE_APB_HOME_STOP,						/* APB home position stop error */
	ALARM_CODE_BOX,									/* BOX set error */
	ALARM_CODE_BOX_INIT,							/* BOX set error (Box not set when initialize) */
	ALARM_CODE_STACKER_CONNECTION,					/* stacker connection error */
	ALARM_CODE_CHEAT,					        	/* cheat detection */
	ALARM_CODE_CENTERING_TIMEOUT,					/* centering time out */
	ALARM_CODE_CENTERING_HOME,						/* centering home position error */
	ALARM_CODE_CENTERING_HOME_REMOVAL,	/* 45 */	/* centering home position removal error */
	ALARM_CODE_CENTERING_HOME_STOP,					/* centering home position stop error */
	ALARM_CODE_STACKER_KEY,							/* stacker key open */
	ALARM_CODE_STACKER_LOSS,						/* stacker unit loss */
	ALARM_CODE_SIDE_LOW_LEVEL,						/* side sensor low level */
	ALARM_CODE_SIDE_HIGH_LEVEL,						/* side sensor high level */
	/* Initialization-related */
	ALARM_CODE_UV,						/* 55 */	/* UV error */
	ALARM_CODE_UV_LOW_LEVEL,							/* UV low level */

	/* recycle processing for ADP >>>>>>>>>>>>>>>>>>>>>>>>>>*/
	ALARM_CODE_DISPENSE_OTHERSENS_AT,				/* dispense jam in acceptor (other sensor ON) */
	ALARM_CODE_DISPENSE_SLIP_AT,		        	/* dispense jam in acceptor (feed slip) */
	ALARM_CODE_DISPENSE_TIMEOUT_AT,					/* dispense jam in acceptor (feed timeout) */
	ALARM_CODE_COMMUNICATION_LOSS,					/* id008 communication loss */

/* Internal Error 内部エラー : 0x40～ */
	ALARM_CODE_FEED_RUNAWAY = 0x40,		/* 64 */	/* feed motor runaway */
	ALARM_CODE_STACKER_RUNAWAY,			/* 65 */	/* stacker motor runaway */
	ALARM_CODE_APB_RUNAWAY,				/* 65 */	/* APB motor runaway */
	ALARM_CODE_CENTERING_RUNAWAY,					/* centering motor runaway */

	ALARM_CODE_FEED_FORCED_QUIT,					/* feed forced quit */
	ALARM_CODE_STACKER_FORCED_QUIT,					/* stacker forced quit */
	ALARM_CODE_APB_FORCED_QUIT,			        	/* APB forced quit */
	ALARM_CODE_CENTERING_FORCED_QUIT,				/* centering forced quit */
	#if (CLEANING_CARD == 1)
	ALARM_CODE_CLEANING_CARD_DETECTED,
	#endif
};


/*----------------------------------------------------------*/
/*			Operating Mode									*/
/*----------------------------------------------------------*/
#define OPERATING_MODE_WAIT_REQ				0x00		/* Wait Download Request Mode */
#define OPERATING_MODE_IF_DOWNLOAD			0x01		/* Download Mode(I/F) Mode */
#define OPERATING_MODE_USB_DOWNLOAD			0x02		/* Download Mode(USB) Mode */
#define OPERATING_MODE_FILE_DOWNLOAD		0x04		/* Download Mode(FILE) Mode */
#define OPERATING_MODE_SUBLINE_DOWNLOAD		0x08		/* Download Mode(SUBLINE USB) Mode */
#define OPERATING_MODE_IF_DIFF_DOWNLOAD		0x10		/* Download Mode(I/F) Mode */

/*----------------------------------------------------------*/
/*			Select Interface								*/
/*----------------------------------------------------------*/
enum{
	IF_SELECT_PHOTOCOUPLER = 0,		/* Photocoupler */
	IF_SELECT_TTL,					/* TTL */
	IF_SELECT_RS232C,				/* RS232C */
	IF_SELECT_CCTALK,				/* ccTalk */
	IF_SELECT_USB,					/* ID-008 USB */
	IF_SELECT_NONE
};

/*----------------------------------------------------------*/
/*			Select Protocol									*/
/*----------------------------------------------------------*/
enum _PROTOCOL_SELECT
{
	PROTOCOL_SELECT_ID008   = 0x08,
	PROTOCOL_SELECT_ID003 	= 0x03,
	PROTOCOL_SELECT_ID0G8 	= 0xF8,
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
#define TMSG_TCODE_FUSB_DECT		0x1100
#define TMSG_TCODE_UART01CB			0x1200
#define TMSG_TCODE_USB0CB			0x1300
#define TMSG_TCODE_USB1CB			0x1400
#define TMSG_TCODE_MGU				0x1500
#define TMSG_TCODE_SDC				0x1600
#define TMSG_TCODE_RFID				0x1700
#define TMSG_TCODE_ICB				0x1800
#define TMSG_TCODE_OTG				0x1900
#define TMSG_TCODE_DIPSW			0x1A00

#define TMSG_MCODE_MASK				0x00FF

#define TMSG_COMMON_STATUS			0x0080

/* Common function Code */
typedef enum _TMSG_SUB
{
	TMSG_SUB_SUCCESS = 0,
	TMSG_SUB_START,
	TMSG_SUB_STOP,
	TMSG_SUB_ACCEPT,
	TMSG_SUB_REJECT,
	TMSG_SUB_VEND,
	TMSG_SUB_PAUSE,
	TMSG_SUB_RESUME,
	TMSG_SUB_COLLECT,
	TMSG_SUB_REMAIN,
	TMSG_SUB_INTERIM,			/* interim termination */
	TMSG_SUB_SLEEP,
	TMSG_SUB_CONNECT,
	TMSG_SUB_RECEIVE,
	TMSG_SUB_EMPTY,
	TMSG_SUB_BOOKMARK,
	TMSG_SUB_SAMPLING,			/* interim termination */
	TMSG_SUB_DOWNLOAD,
	TMSG_SUB_ALARM = 0xFF,
}TMSG_SUB;

/* CONNECTION Task (CLINE Task or DLINE Task) */
enum _TMSG_CONNECTION
{
	TMSG_CONN_INITIAL = 1,
	TMSG_CONN_DOWNLOAD,
	TMSG_CONN_WRITE_FLASH,
	TMSG_CONN_DOWNLOAD_COMPLETE,
	TMSG_CONN_DOWNLOAD_CALC_CRC,
	TMSG_CONN_SOFT_RESET,
	TMSG_CONN_NOTICE = TMSG_COMMON_STATUS,
};


/* DLINE Task */
#define TMSG_DLINE_INITIAL_REQ				((TMSG_TCODE_MAIN)|TMSG_CONN_INITIAL)
#define TMSG_DLINE_NOTICE					((TMSG_TCODE_MAIN)|TMSG_CONN_NOTICE)
#define TMSG_DLINE_DOWNLOAD_REQ				((TMSG_TCODE_DLINE)|TMSG_CONN_DOWNLOAD)
#define TMSG_DLINE_DOWNLOAD_RSP				((TMSG_TCODE_MAIN)|TMSG_CONN_DOWNLOAD)
#define TMSG_DLINE_DOWNLOAD_COMPLETE_REQ	((TMSG_TCODE_DLINE)|TMSG_CONN_DOWNLOAD_COMPLETE)
#define TMSG_DLINE_DOWNLOAD_COMPLETE_RSP	((TMSG_TCODE_MAIN)|TMSG_CONN_DOWNLOAD_COMPLETE)
#define TMSG_DLINE_SOFT_RESET_REQ			((TMSG_TCODE_DLINE)|TMSG_CONN_SOFT_RESET)
#define TMSG_DLINE_SOFT_RESET_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_SOFT_RESET)

/* CLINE Task */
#define TMSG_CLINE_INITIAL_REQ				((TMSG_TCODE_MAIN)|TMSG_CONN_INITIAL)
#define TMSG_CLINE_DOWNLOAD_REQ				((TMSG_TCODE_CLINE)|TMSG_CONN_DOWNLOAD)
#define TMSG_CLINE_DOWNLOAD_RSP				((TMSG_TCODE_MAIN)|TMSG_CONN_DOWNLOAD)
#define TMSG_CLINE_WRITE_FLASH_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_WRITE_FLASH)
#define TMSG_CLINE_WRITE_FLASH_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_WRITE_FLASH)
#define TMSG_CLINE_DOWNLOAD_CALC_CRC_REQ	((TMSG_TCODE_CLINE)|TMSG_CONN_DOWNLOAD_CALC_CRC)
#define TMSG_CLINE_DOWNLOAD_CALC_CRC_RSP	((TMSG_TCODE_MAIN)|TMSG_CONN_DOWNLOAD_CALC_CRC)
#define TMSG_CLINE_SOFT_RESET_REQ			((TMSG_TCODE_CLINE)|TMSG_CONN_SOFT_RESET)
#define TMSG_CLINE_SOFT_RESET_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_SOFT_RESET)
#define TMSG_CLINE_STATUS_INFO				((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)

/* DIPSW Task */
#define TMSG_DIPSW_INFO						((TMSG_TCODE_DIPSW)|TMSG_COMMON_STATUS)
#define TMSG_DIPSW_INIT_REQ					((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_DIPSW_INIT_RSP					((TMSG_TCODE_DIPSW)|0x0001)
#define TMSG_DIPSW_STATUS_REQ				((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_DIPSW_STATUS_INFO				((TMSG_TCODE_DIPSW)|TMSG_COMMON_STATUS)

/* UART01CB Task */
#define TMSG_UART01CB_CALLBACK_INFO			((TMSG_TCODE_UART01CB)|TMSG_COMMON_STATUS)

/* USB0CB Task */
#define TMSG_USB0CB_CALLBACK_INFO			((TMSG_TCODE_USB0CB)|TMSG_COMMON_STATUS)

/* USB1CB Task */
#define TMSG_USB1CB_CALLBACK_INFO			((TMSG_TCODE_USB1CB)|TMSG_COMMON_STATUS)

/* SUBLINE Task */
#define TMSG_SUBLINE_INITIAL_REQ			((TMSG_TCODE_MAIN)|TMSG_CONN_INITIAL)
#define TMSG_SUBLINE_NOTICE					((TMSG_TCODE_MAIN)|TMSG_CONN_NOTICE)
#define TMSG_SUBLINE_DOWNLOAD_REQ			((TMSG_TCODE_SUBLINE)|TMSG_CONN_DOWNLOAD)
#define TMSG_SUBLINE_DOWNLOAD_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_DOWNLOAD)
#define TMSG_SUBLINE_DOWNLOAD_COMPLETE_REQ	((TMSG_TCODE_SUBLINE)|TMSG_CONN_DOWNLOAD_COMPLETE)
#define TMSG_SUBLINE_DOWNLOAD_COMPLETE_RSP	((TMSG_TCODE_MAIN)|TMSG_CONN_DOWNLOAD_COMPLETE)
#define TMSG_SUBLINE_SOFT_RESET_REQ			((TMSG_TCODE_SUBLINE)|TMSG_CONN_SOFT_RESET)
#define TMSG_SUBLINE_SOFT_RESET_RSP			((TMSG_TCODE_MAIN)|TMSG_CONN_SOFT_RESET)

/* OTG Task */
#define TMSG_OTG_NOTICE						((TMSG_TCODE_OTG)|TMSG_CONN_NOTICE)

/* FUCB_DECT Task */
#define TMSG_FUSB_DECT_INITIAL_REQ			((TMSG_TCODE_MAIN)|TMSG_CONN_INITIAL)
#define TMSG_FUSB_DECT_NOTICE				((TMSG_TCODE_FUSB_DECT)|TMSG_CONN_NOTICE)

enum _POWERUP_STAT
{
	/*
	 * normal powerup
	 */
	POWERUP_STAT_NORMAL = 0,
};

/* TIMER Task */
#define TMSG_TIMER_TIMES_UP			((TMSG_TCODE_TIME)|0x0001)
#define TMSG_TIMER_SET_TIMER		((TMSG_TCODE_MAIN)|0x0001)
#define TMSG_TIMER_CANCEL_TIMER		((TMSG_TCODE_MAIN)|0x0002)
#define TMSG_TIMER_STATUS_REQ		((TMSG_TCODE_MAIN)|TMSG_COMMON_STATUS)
#define TMSG_TIMER_STATUS_INFO		((TMSG_TCODE_TIME)|TMSG_COMMON_STATUS)

#define	MAX_TIMER	16
/* TIMER ID */
enum _TIMER_ID
{
	TIMER_ID_STATUS_WAIT1,
	TIMER_ID_FUSB_BOUNCE,
	TIMER_ID_DATA_WAIT,
	TIMER_ID_SLEEP_MODE,		/* '13-01-17 */
	/* Upper limit 16 */
};

/* 10msec timer */
#define WAIT_TIME_DIPSW_READ			100		/* wait 1000msec */
#define WAIT_TIME_STATUS_WAIT			300		/* wait 3sec    */
#define WAIT_TIME_FUSB_BOUNCE			2		/* wait 20msec */
#define WAIT_TIME_DATA_WAIT				500		/* wait 5sec    */

/* DISPLAY Task */
#define TMSG_DISP_LED_OFF				((TMSG_TCODE_MAIN)|0x0001)
//#define TMSG_DISP_LED_ON				((TMSG_TCODE_MAIN)|0x0002)


#if 1 //2023-12-22
#define TMSG_DISP_LED_DOWNLOAD_ING		((TMSG_TCODE_MAIN)|0x000B)
#endif

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

enum _DISP_CTRL
{
	DISP_CTRL_TEST_STANDBY = 0,
};

/*----------------------------------------------------------*/
/*			Event Flag										*/
/*----------------------------------------------------------*/
#define EVT_ALL_BIT						0xFFFFFFFF
/*----------------------------------------------------------*/
/*			Call from										*/
/*----------------------------------------------------------*/
#define FROM_IF                0
#define FROM_BIF               1


/*----------------------------------------------------------*/
/*			PERIPHERAL SETTINGS								*/
/*----------------------------------------------------------*/
#define I2C_RETRY_COUNT		10

/*----------------------------------------------------------*/
/*			DEBUG CONTROL									*/
/*----------------------------------------------------------*/
#include "debug.h"


#endif /* SRC_INCLUDE_COMMON_H_ */
