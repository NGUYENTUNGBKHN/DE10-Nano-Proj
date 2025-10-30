/******************************************************************************/
/*! @addtogroup Main
    @file       com_ram.c
    @brief      common variable
    @date       2018/03/05
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

#ifndef SRC_MAIN_INCLUDE_COM_RAM_H_
#define SRC_MAIN_INCLUDE_COM_RAM_H_

#include "debug.h"
#include "soc_cv_av/alt_clock_manager.h"
#include "custom.h"
#include "common.h"
#include "struct.h"

#if defined(EXT)
	#define EXTERN extern
#else
	#undef EXTERN
	#define EXTERN
#endif
EXTERN RTC_INFO ex_rtc_clock;

EXTERN u16 ex_rom_crc;
EXTERN u8 ex_main_task_mode1;
EXTERN u8 ex_main_task_mode2;

EXTERN u16 ex_display_task_mode;
EXTERN u16 ex_dline_task_mode;
EXTERN u16 ex_otg_task_mode;

EXTERN u16 ex_main_task_mode;
EXTERN u16 ex_bezel_task_mode;
EXTERN u16 ex_cline_task_mode;
EXTERN u16 ex_display_task_mode;
EXTERN u16 ex_fusb_det_task;
EXTERN u16 ex_dipsw_task_mode;
EXTERN u16 ex_otg_task_mode;
EXTERN u16 ex_subline_task_mode;
EXTERN u16 ex_timer_task;
EXTERN u16 ex_timer_task_mode;
EXTERN u16 ex_uart01_cb_task_mode;
EXTERN u16 ex_usb0_cb_task_mode;
EXTERN u16 ex_usb1_cb_task_mode;


/*----------------------------------------------------------------------*/
/* Software Infomation Variables                                        */
/*----------------------------------------------------------------------*/
#define MODEL_LENGTH		16
#if (COUNTRY_SIZE > 2)
 #define COUNTRY_LENGTH		20
#else
 #define COUNTRY_LENGTH		8
#endif
#define PROTOCOL_LENGTH		16
#define VERSION_LENGTH		8
#define DATE_LENGTH			8
EXTERN u8 ex_model[MODEL_LENGTH];
EXTERN u8 ex_country[COUNTRY_LENGTH];
EXTERN u8 ex_protocol[PROTOCOL_LENGTH];
EXTERN u8 ex_version[VERSION_LENGTH];
EXTERN u8 ex_date[DATE_LENGTH];
EXTERN u8 ex_smrt_id;
/*----------------------------------------------------------------------*/
/* Setting Variables                                                    */
/*----------------------------------------------------------------------*/
EXTERN u8 ex_dipsw1;
EXTERN u8 ex_dipsw2;
EXTERN u8 ex_operating_mode;
EXTERN u16 ex_abnormal_code;
EXTERN u8 ex_led_mask;
EXTERN u8 ex_led_mode;

/*----------------------------------------------------------------------*/
/* Timer Variables                                                      */
/*----------------------------------------------------------------------*/
EXTERN u32 ex_timer_task_tick;

EXTERN double debug_time;

#if 0
EXTERN MRX_CLOCK ex_mrx_clock;
#else
EXTERN RTC_INFO ex_rtc_clock;
#endif


/*----------------------------------------------------------------------*/
/* Motor control Variables                                              */
/*----------------------------------------------------------------------*/
EXTERN u16 _ir_line_cmd_monitor_time;

EXTERN TMP_SENSOR_AD ex_tmp[8];
enum TMP_POS{
	TMP_IC_CISA = 0,
	TMP_IC_CISB,
	TMP_IC_OUT,
	TMP_IC_HMOT,
	TMP_IC_SMOT,
	TMP_IC_MAX,
};

/*----------------------------------------------------------*/
/*			status table									*/
/*----------------------------------------------------------*/
EXTERN CLINE_STATUS_TBL ex_cline_status_tbl;
EXTERN CLINE_DOWNLOAD_CONTROL ex_cline_download_control;
EXTERN ROM_SECTION_STATUS ex_section_status;


/*----------------------------------------------------------------------*/
/* LED(IOEX)                                                            */
/*----------------------------------------------------------------------*/
#endif /* SRC_MAIN_INCLUDE_COM_RAM_H_ */

/*----------------------------------------------------------------------*/
/* Monitor 													         	*/
/*----------------------------------------------------------------------*/
EXTERN HAL_STATUS_TABLE ex_hal_status;


/*----------------------------------------------------------------------*/
/* PLL Infomaiton 													   	*/
/*----------------------------------------------------------------------*/
EXTERN u8 clock_enabled[ALT_CLK_H2F_USER2+1];
EXTERN u32 clock_frequency[ALT_CLK_H2F_USER2+1];
EXTERN u8 safe_enabled[2];

EXTERN	bool red_on;		// LED
EXTERN	bool green_on;		// LED

#if defined(_PROTOCOL_ENABLE_ID0G8)

EXTERN int ex_dfu_mode; //2022-11-07

//2022-08-01 EXTERN u8 	ex_download_erace_dfu[800];
//2022-07-14 EXTERN u8	*ex_usb_p_dl_write_dfu;		/* ﾀﾞｳﾝﾛｰﾄﾞ先 書き込み ｱﾄﾞﾚｽ(ｷｬｯｼｭ無効空間に変換後) *//* 500のTool Suiteダウンロードと同じ概念*/
//2022-08-01 EXTERN u8	ex_usb_p_dl_write_dfu;	//2022-07-14 ポインタでない方がいいのでは	/* ﾀﾞｳﾝﾛｰﾄﾞ先 書き込み ｱﾄﾞﾚｽ(ｷｬｯｼｭ無効空間に変換後) *//*  500のTool Suiteダウンロードと同じ概念*/



EXTERN	u8		Dfu_Error_State;					/*	ON: DFU Error Status */


/* GSA-USBフラグ *//* ここから整理開始 *//* 2020-12-04	*/


/* DFU I/F現内部ステート */
EXTERN  u8 DucDfuCurState;
/* DFU I/F次内部ステート */
EXTERN  u8 DucDfuNxtState;

/* DFU I/F現ステータス */
EXTERN  u8 DucDfuStatus;

/* USBコマンド番号 */
EXTERN  u8 DucUsbCmdNo;

EXTERN  u8 ex_dowload_data_recived_0g8;


/* プログラムモードフラグ */
EXTERN	u8 DucDfuProgramModeFlag;	/* modeフラグ(HID or DFU)	*//* use */


/* 要求アップロードサイズ */
EXTERN	u16 DusUpReqLen;	/* use */

/* 実際のアップロードサイズ */
EXTERN	u16 DusUpActualLen;	/* use */


/* 読み込みアドレス */
EXTERN	u8* PucReadAddr;	/* use */

/* ダウンロードサイズ */
EXTERN	u32 DuwDnBlockSize;

/* Detachタイマー(Detachタイムアウトを検出するために使用) */
EXTERN	u16	Cm01Detach;

EXTERN	u16	DusDetachTimeout;	//2022-08-05

/*  総ダウンロードバイト数 */
EXTERN	u32	DuwTotalDnloadBytes;

EXTERN	u32	DuwTotalDnloadBytes_target; /* 2022-08-01 */


/*  Signatureチェックフラグ */
EXTERN	u8	DucSigChkFlag_DFU;	/* use *//* ダウンロードファイルのHeader確認に使用*/


EXTERN u8 ex_usb_dfu_disconect_0g8;


#endif
/* EOF */
