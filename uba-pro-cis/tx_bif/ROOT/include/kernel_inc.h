/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file kernel_inc.h
 * @brief カーネルヘッダファイル
 * @date 2013.1.21 Created.
 */
/****************************************************************************/
#if !defined(__KERNEL_INC_H_INCLUDED__)
#define __KERNEL_INC_H_INCLUDED__

#include "reg_cyclone5.h"
#include "jcm_typedef.h"
/****************************************************************/
/**
 * @enum TUD
 * @brief タスクＩＤ
 */
/****************************************************************/
#if 1
enum TID
{
// Task ID
	ID_MAIN_TASK = (1),
	ID_DLINE_TASK,
	ID_CLINE_TASK,
	ID_TIMER_TASK,
	ID_DISPLAY_TASK,
	ID_IDLE_TASK,
	ID_BEZEL_TASK,
	ID_OTG_TASK,
	ID_SUBLINE_TASK,
	ID_DIPSW_TASK,
	ID_SENSOR_TASK,
	ID_FRAM_TASK,
	ID_MOTOR_TASK,
	ID_MGU_TASK,
	ID_DISCRIMINATION_TASK,
	ID_SDC_TASK,
	ID_RFID_TASK,
	ID_ICB_TASK,
	ID_FEED_TASK,
	ID_CENTERING_TASK,
	ID_APB_TASK,
	ID_STACKER_TASK,
	ID_UART01_CB_TASK,
	ID_USB0_CB_TASK,
	ID_USB1_CB_TASK,
	ID_FUSB_DET_TASK,
	ID_USB_HOST_APP_TASK,
	ID_WDT_TASK,
	TID_MAX = 64,		// ID自動割当て用を確保
};
#else
enum TID
{
	TID_MAIN = (1),		// 1
	TID_CLINE ,			// 2
	TID_MLINE,			// 3
	TID_DLINE,			// 4
	TID_BV,				// 5
	TID_SENS,			// 6
	TID_FLASH,			// 7
	TID_TIMER,			// 8
	TID_WDT,			// 9
	TID_TEMPLATE,		// 10
	TID_UARTCB,			// 11					UARTコールバックタスク
	TID_USBCB,			// 12					USBコールバックタスク
	TID_USBCB2,			// 13					USBコールバックタスク2
	TID_DISP,			// 14
	TID_EDGE,			// 15
	TID_IDENT,			// 16
	TID_OCR,			// 17
	TID_VALIDATION,	    // 18
	TID_VALIDATION1,    // 19
	TID_PF,				// 20
	TID_HEAVY,			// 21
};
#endif


/****************************************************************/
/**
 * @enum DTQID
 * @brief 				メモリーボックス定義
 */
/****************************************************************/
// Mail box ID
enum MBXID
{
	ID_MAIN_MBX = (1),
	ID_DLINE_MBX,
	ID_CLINE_MBX,
	ID_TIMER_MBX,
	ID_DISPLAY_MBX,
	ID_IDLE_MBX,
	ID_BEZEL_MBX,
	ID_SUBLINE_MBX,
	ID_DIPSW_MBX,
	ID_SENSOR_MBX,
	ID_UART01_CB_MBX,
	ID_USB0_CB_MBX,
	ID_USB1_CB_MBX,
	ID_FUSB_DET_MBX,
	ID_MOTOR_MBX,
	ID_FRAM_MBX,
	ID_MGU_MBX,
	ID_SDC_MBX,
	ID_RFID_MBX,
	ID_ICB_MBX,
	ID_DISCRIMINATION_MBX,
	ID_APB_MBX,
	ID_CENTERING_MBX,
	ID_FEED_MBX,
	ID_STACKER_MBX,
	MBXID_MAX =(30)
};

/****************************************************************/
/**
 * @enum DTQID
 * @brief 				データキューＩＤ定義
 */
/****************************************************************/
enum DTQID
{
#if 1
	ID_UART_RX_DTQ = (1),	// 1
	ID_USB0_RX_DTQ,			// 2
	ID_USB1_RX_DTQ,			// 3
#endif
	DTQID_MAX = 10,
};


/****************************************************************/
/**
 * @enum MPFID
 * @brief 				固定長メモリプールＩＤ定義
 */
/****************************************************************/
enum
{
	ID_MBX_MPF = (1),		// 1
	ID_MBX_MAIN_MPF,		// 2
	MPFID_MAX = 10,
};


/****************************************************************/
/**
 * @enum MPLID
 * @brief 				可変長メモリプールＩＤ定義
 */
/****************************************************************/
enum
{
	ID_SYSTEM_MPL = (1),	// 1
	MPLID_MAX = 10,
};

/****************************************************************/
/**
 * @brief FLGID
 * @brief フラグＩＤ定義
 */
/****************************************************************/
enum FLGID
{
	ID_OTG_CTRL_FLAG = (1),	// 1
	ID_SENSOR_FLAG,			// 2
	ID_FEED_CTRL_FLAG,		// 3
	ID_CENTERING_CTRL_FLAG,	// 4
	ID_APB_CTRL_FLAG,		// 5
	ID_UART01_CB_FLAG,		// 6
	ID_USB0_CB_FLAG,		// 7
	ID_USB1_CB_FLAG,		// 8
	ID_FUSB_DET_FLAG,		// 9
	ID_STACKER_CTRL_FLAG,	// 10
	ID_FRAM_CTRL_FLAG,		// 11
	ID_SD_CTRL_FLAG,		// 12
	ID_RES_CTRL_FLAG,		// 13
	ID_FLGID_WDT,			// 14
	FLGID_MAX = (30)
};

#define FLGPTN_ALL				(0xffffffff)

// FLGID_FAN
#define FLGPTN_FAN1_NORMAL		(0x00000001)
#define FLGPTN_FAN1_STOP		(0x00000002)
#define FLGPTN_FAN2_NORMAL		(0x00000004)
#define FLGPTN_FAN2_STOP		(0x00000008)
#define FLGPTN_FAN1_ALARM		(0x10000000)
#define FLGPTN_FAN2_ALARM		(0x20000000)


// FLGID_SENS
#define FLGPTN_PS0_0			(0x00000001)
#define FLGPTN_PS1_0			(0x00000010)
#if 0
#define FLGPTN_PS0_1			(0x00000002)
#define FLGPTN_PS0_2			(0x00000004)
#define FLGPTN_PS0_3			(0x00000008)

#define FLGPTN_PS1_1			(0x00000020)
#define FLGPTN_PS1_2			(0x00000040)
#define FLGPTN_PS1_3			(0x00000080)

#define FLGPTN_PS2_0			(0x00000100)
#define FLGPTN_PS2_1			(0x00000200)
#define FLGPTN_PS2_2			(0x00000400)
#define FLGPTN_PS2_3			(0x00000800)
#endif

#define FLGPTN_FEED_START				(0x00000800)
#define FLGPTN_CIS_COMPLETE				(0x00001000)
#define FLGPTN_ENTER_NOTE				(0x00002000)
#define FLGPTN_BILL_TMOUT				(0x00004000)		// 札間タイムアウト, 18/07/30
#define FLGPTN_TKNS_COMPLETE			(0x00020000)
#define FLGPTN_MAG_COMPLETE				(0x00040000)
#define FLGPTN_DAC_COMPLETE				(0x00100000)		// FLGID_DAC
#define FLGPTN_CAP_REF_COMPLETE			(0x08000000)		// 静電容量センサスキャン完了　FLGID_CAP_REF
#define FLGPTN_TKNS_REF_DATA_COMPLETE	(0x10000000)		// メカ厚み調整データ採取完了, FLGID_TKNS_REF
#define FLGPTN_THERMO					(0x20000000)		// 温度センサ更新フラグ
#define FLGPTN_TKNS_HOLE_IC				(0x40000000)		// メカ厚みホールIC		FLGID_TKNS_HOLE_IC
#define FLGPTN_TKNS_REF_COMPLETE		(0x80000000)		// メカ厚み基準ローラーデータ採取完了	FLGID_TKNS_REF


/****************************************************************/
/**
 * @brief CYCID
 * @brief 周期タイマＩＤ定義
 */
/****************************************************************/
enum CYCID
{
	ID_SYSTEM_CYC  = (1), // 1
	ID_LED_CYC,			 // 2
	ID_DIPSW_CYC,		 // 3
	CYCID_MAX = 10,
};

/****************************************************************/
/**
 * @brief ISRID
 * @brief 割り込みサービスルーチンＩＤ定義
 */
/****************************************************************/
#if defined(PRJ_OS_UC3)
enum ISRID
{
	ISRID_IRQ0 = (1),	// 1
	ISRID_IRQ1,			// 2
	ISRID_IRQ2,			// 3
	ISRID_IRQ3,			// 4
	ISRID_IRQ4,			// 5
	ISRID_IRQ5,			// 6
	ISRID_IRQ6,			// 7
	ISRID_IRQ7,			// 8
	ISRID_IRQ8,			// 9
	ISRID_IRQ9,			// 10
	ISRID_IRQ10,		// 11
	ISRID_IRQ11,		// 12
	ISRID_IRQ12,		// 13
	ISRID_IRQ15,		// 14
	ISRID_IRQ16,		// 15
	ISRID_IRQ17,		// 16
	ISRID_IRQ18,		// 17
	ISRID_IRQ19,		// 18
	ISRID_IRQ20,		// 19
};
#elif defined(PRJ_OS_THREADX_UITRON)
enum ISRID
{
	ISRID_IRQ0 = (1),			// 1
	ISRID_IRQ1,					// 2
	ISRID_IRQ2,					// 3
	ISRID_IRQ3,					// 4
	ISRID_IRQ4,					// 5
	ISRID_IRQ5,					// 6
	ISRID_IRQ6,					// 7
	ISRID_IRQ7,					// 8
	ISRID_IRQ8,					// 9
	ISRID_IRQ9,					// 10
	ISRID_IRQ10,				// 11
	ISRID_IRQ11,				// 12
	ISRID_IRQ12,				// 13
	ISRID_IRQ15,				// 14
	ISRID_IRQ16,				// 15
	ISRID_IRQ17,				// 16
	ISRID_IRQ18,				// 17
	ISRID_IRQ19,				// 18
	ISRID_IRQ20,				// 19

	ISRID_WDT,					// 20

	ISRID_MAX = 30,				// ID自動割当て用を確保
};
// 割込みレベル設定, 18/09/05
enum {
	IPL_MASK = 0xF0,        /* priority mask for ICCICR register */
	IPL_KERNEL_HIGHEST = (0x20),		// カーネル割込み優先度最高レベル
	IPL_KERNEL_HIGHER = (0x30),
	IPL_KERNEL_HIGH = (0x40),
	IPL_KERNEL_NORMAL = (0x50),
	IPL_KERNEL_LOW = (0x60),
	IPL_KERNEL_LOWER = (0x70),
	IPL_KERNEL_LOWEST = (0x80),
	IPL_USER_HIGHEST = (0x90),		// ユーザ割込み優先度最高レベル
	IPL_USER_HIGHER = (0xa0),
	IPL_USER_HIGH = (0xb0),
	IPL_USER_NORMAL = (0xc0),
	IPL_USER_LOW = (0xd0),
	IPL_USER_LOWER = (0xe0),
	IPL_USER_LOWEST = (0xf0),
	IPL_USER_MASK = (0xff),
};
#endif

/****************************************************************/
/**
 * @brief SEMID
 * @brief セマフォＩＤ定義
 */
/****************************************************************/
enum SEMID
{
	ID_I2C0_SEM = 1,		// I2Cアクセスセマフォ
	ID_I2C3_SEM,			// I2Cアクセスセマフォ
	ID_STATE_TBL_SEM,
	SEMID_MAX = 10,
};

/****************************************************************
*					タスク優先順位定義							*
****************************************************************/
#define	TPRIORITY_LOWEST	(TPRIORITY_NORMAL + 2)
#define	TPRIORITY_LOW		(TPRIORITY_NORMAL + 1)
#define	TPRIORITY_NORMAL	(4)
#define	TPRIORITY_HIGH		(TPRIORITY_NORMAL - 1)
#define	TPRIORITY_HIGHEST	(TPRIORITY_NORMAL - 2)

#define TPRIORITY_EDGE (TPRIORITY_TEMPLATE + 1)
#define TPRIORITY_IDENT (TPRIORITY_EDGE + 1)
#define TPRIORITY_OCR (TPRIORITY_IDENT + 1)
#define	TPRIORITY_TEMPLATE (TPRIORITY_NORMAL + 1)
#define	TPRIORITY_TEMPLATE_HIGH (TPRIORITY_TEMPLATE + 1)
#define	TPRIORITY_TEMPLATE_MIDDEL (TPRIORITY_TEMPLATE_HIGH + 1)
#define	TPRIORITY_TEMPLATE_LOW (TPRIORITY_TEMPLATE_MIDDEL + 1)
#define	TPRIORITY_TEMPLATE_HEAVY (TPRIORITY_TEMPLATE_LOW + 1)

typedef struct MSG_STRUCTURE
{
	u32 tid;									/* タスクＩＤ */
	u32 cmd;									/* 機能コード */
	u32 param[5];								/* パラメータ */
} TSK_MSG;

//typedef struct MSG_STRUCTURE TSK_MSG;			/* メッセージ */
#define TSK_MSG_SIZE (sizeof(TSK_MSG))			/* メッセージサイズ */

#define DTQ_DEF_CNT (40)						/* データキューカウントデフォルト値 = 40 */

#define MPL_SYSTEM_SIZE		(1024 * 1024 * 3)	/* 3MByte ※OCRアルゴが3M程度必要になる為、256KB→3Mに変更 */

//#define TASK_STACK_SIZE		(2048)				/* タスクスタックサイズ     */
#define TASK_STACK_SIZE		(4096)				/* タスクスタックサイズ  現状1KBも使ってないが増やしておく 2019/2/28  */

#endif /* __KERNEL_INC_H_INCLUDED__ */

/* End of File */

