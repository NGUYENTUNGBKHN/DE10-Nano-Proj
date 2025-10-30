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

#include "soc_cv_av/alt_clock_manager.h"
#include "custom.h"
#include "common.h"
#include "struct.h"
#include "fpga.h"

#if defined(EXT)
	#define EXTERN extern
#else
	#undef EXTERN
	#define EXTERN
#endif

EXTERN u16 ex_rom_crc;
EXTERN u8 ex_main_task_mode1;
EXTERN u8 ex_main_task_mode2;

EXTERN u8 ex_uba710;
EXTERN u8 ex_main_task_mode1_alarm_back;
EXTERN u8 ex_main_task_mode2_alarm_back;

EXTERN u16 ex_main_task_mode;
EXTERN u16 ex_apb_task_seq;
EXTERN u16 ex_bezel_task_mode;
EXTERN u16 ex_centering_task_seq;
EXTERN u16 ex_cline_task_mode;
EXTERN u16 ex_discrimination_task_mode;
EXTERN u16 ex_display_task_mode;
EXTERN u16 ex_dline_task_mode;
EXTERN u16 ex_feed_task_seq;
EXTERN u16 ex_fram_task_mode;
EXTERN u16 ex_fusb_det_task;
EXTERN u16 ex_dipsw_task_mode;
EXTERN u16 ex_icb_task_seq;
EXTERN u16 ex_mgu_task_mode;
EXTERN u16 ex_motor_task_mode;
EXTERN u16 ex_otg_task_mode;
EXTERN u16 ex_rfid_task_mode;
EXTERN u16 ex_sensor_task_mode;
EXTERN u16 ex_stacker_task_seq;
EXTERN u16 ex_subline_task_mode;
EXTERN u16 ex_timer_task;
EXTERN u16 ex_timer_task_mode;
EXTERN u16 ex_uart01_cb_task_mode;
EXTERN u16 ex_usb0_cb_task_mode;
EXTERN u16 ex_usb2_cb_task_mode;
EXTERN u16 ex_power_task_mode;
EXTERN u16 ex_shutter_task_seq;

/*----------------------------------------------------------------------*/
/* Task status Variables                                                */
/*----------------------------------------------------------------------*/
#define TASK_ST_MAIN			0x00000001
#define TASK_ST_MUSB			0x00000002
#define TASK_ST_LINE			0x00000004
#define TASK_ST_FEED			0x00000008
#define TASK_ST_STACKER			0x00000010
#define TASK_ST_DISCRIMINATION	0x00000020
#define TASK_ST_SENSOR			0x00000040
#define TASK_ST_TIMER			0x00000080
#define TASK_ST_DISPLAY			0x00000100
#define TASK_ST_CENTERING		0x00000200
#define TASK_ST_APB				0x00000400
#define TASK_ST_IDLE			0x00000800
#define TASK_ST_CIS_INIT		0x00001000
#define TASK_ST_MGU				0x00002000
#define TASK_ST_SHUTTER			0x00004000
#define TASK_ST_SIDE			0x00008000
#define TASK_ST_ICB				0x00010000
#define TASK_ST_MAG_INIT		0x00020000
#if defined(UBA_RTQ)
#define TASK_ST_RC				0x00040000
#endif // UBA_RTQ
#define TASK_ST_ALL				0xFFFFFFFF

enum{
	MULTI_MAIN			=0x00,
	MULTI_DLINE,
	MULTI_LINE,
	MULTI_FEED,
	MULTI_STACKER,
	MULTI_DISCRIMINATION,
	MULTI_SENSOR,
	MULTI_TIMER,
	MULTI_DISPLAY,
	MULTI_CENTERING,
	MULTI_APB,
	MULTI_IDLE,
	MULTI_CIS,
	MULTI_MGU,
	MULTI_SHUTTER,
	MULTI_ICB,
	MULTI_END
};


typedef struct _TASK_STATUS
{
	u32 busy;
	u32 alarm;
} TASK_STATUS;
typedef struct _MULTI_JOB
{
	/* ログに使用するので、並び替え、サイズ変更禁止 */
	u32 busy;
	u32 reject;
	u32 alarm;
	u32 denomi;
	u32 direction;

	u32 code[MULTI_END];
	u32 sequence[MULTI_END];
	u32 sensor[MULTI_END];
	// add
} MULTI_JOB;


EXTERN MULTI_JOB ex_multi_job;

/*----------------------------------------------------------------------*/
/* MAIN Task Variables                                                  */
/*----------------------------------------------------------------------*/
EXTERN u16 ex_main_test_no;
EXTERN u8 ex_main_reset_flag;
EXTERN u8 ex_main_pause_flag;
EXTERN u8 ex_main_reject_flag;
EXTERN u8 ex_test_finish;
EXTERN u8 ex_main_search_flag;
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
/* System Flag	for Bar ticket	                                   		*/
/*----------------------------------------------------------------------*/
EXTERN u8 ex_system; //バーチケット受け取りのSS/SU確認に使用
#define	BIT_SS_UNIT						0x08		/*	SS Style 				*/
#define	BIT_SU_UNIT						0x04		/*	SU Style 				*/
//#define	BIT_SH_UNIT					0x02		/*	SH Style 				*/
#define	BIT_LD_UNIT						0x01		/*	LD Style 				*/

#define	BIT_STACKER_UNIT_FLAG			( BIT_SS_UNIT\
										| BIT_SU_UNIT\
										| BIT_LD_UNIT\
										)

/*----------------------------------------------------------------------*/
/* Setting Variables                                                    */
/*----------------------------------------------------------------------*/
EXTERN u8 ex_dipsw1;	//2025-05-13 UBA500同様に常に更新
EXTERN u8 ex_dipsw2;	//not use 2025-05-13 起動時の1回のみ

EXTERN bool ex_dipsw1_run;	//2025-02-26
EXTERN u8 ex_operating_mode;
EXTERN u16 ex_abnormal_code;

/*----------------------------------------------------------------------*/
/* Timer Variables                                                      */
/*----------------------------------------------------------------------*/
EXTERN u32 ex_timer_task_tick;
EXTERN u64 ex_current_utc_time;

EXTERN u64 ex_rfid_setTime_cnt; 			// count rfid set time 1 = 100ms
EXTERN u8 ex_rfid_flag_setTimeInit_done; 	// flag allow counter variable ex_rfid_setTime_cnt
EXTERN volatile RTC_INFO ex_rtc_clock;		//RTQでは使用していないが一部処理残す RFID用

/*----------------------------------------------------------*/
/*			status table									*/
/*----------------------------------------------------------*/
EXTERN CLINE_STATUS_TBL ex_cline_status_tbl;
EXTERN RECOVERY_INFO ex_recovery_info;
EXTERN CLINE_DOWNLOAD_CONTROL ex_cline_download_control;

EXTERN u32 ex_download_start_flg;
#define DL_START_NOT           0 /* ダウンロード未開始     */
#define DL_START_NORMAL        1 /* 通常ダウンロード開始   */
#define DL_START_DIFFERENTIAL  2 /* 差分ダウンロード開始   */


/*----------------------------------------------------------*/
/*			Tool Suite										*/
/*----------------------------------------------------------*/
EXTERN T_SUITE_ITEM ex_suite_item;
#if defined(USB_REAR_USE)
EXTERN T_SUITE_ITEM ex_suite_item_rear;
#endif // USB_REAR_USE
EXTERN testmode ex_dline_testmode;

/*----------------------------------------------------------------------*/
/* Motor control Variables                                              */
/*----------------------------------------------------------------------*/
EXTERN u16 _ir_line_cmd_monitor_time;

EXTERN u16 _ir_motor_disable_time;

EXTERN ST_FEED_MOTOR_CTRL _ir_feed_motor_ctrl;

typedef struct _TASK_TIMOUT
{
	u32 time;		//セットするカウンタの値
	u32 time_out;	//カウンタとして使用
	u8 time_init;	//1でカウンタの値をカウンタにセット
} TASK_TIMOUT;
EXTERN TASK_TIMOUT _ir_feed_ctrl;


EXTERN ST_STACKER_MOTOR_CTRL _ir_stacker_motor_ctrl;
EXTERN u16 _ir_stacker_ctrl_time_out;
EXTERN u32 ex_feed_motor_test_speed;
EXTERN ST_MOTOR_HOME_POSITION_CTRL _ir_centering_motor_ctrl;
EXTERN u16 _ir_centering_ctrl_time_out;
EXTERN ST_PB_MOTOR_CTRL _ir_apb_motor_ctrl;
EXTERN ST_SHUTTER_MOTOR_CTRL _ir_shutter_motor_ctrl;
EXTERN u16 _ir_shutter_ctrl_time_out;

typedef struct _MOTOR_LIMIT_STACKER_TABLE
{
	s16 tempic_ad;
	u16 limit1_1st;	/* Fwd 1st	*///SS not use //RTQ use
	u16 limit1_2nd;	/* Fed 2nd	*/
	u16 limit2_1st;	/* Rev 1st	*/
} MOTOR_LIMIT_STACKER_TABLE;

#define STACKER_AD_NUMBER	22
EXTERN u8 motor_limit_stacker_table_index;


/*----------------------------------------------------------------------*/
/* Motor speed control Variables                                        */
/*----------------------------------------------------------------------*/
EXTERN FEED_SPEED_CTRL ex_feed_motor_speed[FEED_SPEED_END];

EXTERN u16 ex_stacker_full_timer;
EXTERN u16 ex_stacker_full_pulse;

EXTERN ST_FEED_MOTOR_CTRL _debug_feed_motor_ctrl;
//EXTERN ST_MOTOR_HOME_POSITION_CTRL _debug_centering_motor_ctrl;
EXTERN ST_PB_MOTOR_CTRL _debug_apb_motor_ctrl;	//2023-06-20

EXTERN u16 ex_centering_count;
EXTERN u16 ex_stacker_count;
EXTERN u16 ex_apb_count;
EXTERN u16 ex_shutter_count;
EXTERN bool ex_start_jdl;

/*----------------------------------------------------------------------*/
/* Position sensor Variables                                            */
/*----------------------------------------------------------------------*/
EXTERN POSITION_SENSOR ex_position_da;
// bit 0:LOW GAIN, BIT 1:HIGH GAIN
EXTERN u8 ex_position_ga;
EXTERN POSITION_SENSOR ex_position_da_adj;
EXTERN POS_ADJUSTMENT_TMP ex_position_tmp;
EXTERN POS_ADJUSTMENT_TMP ex_position_tmp_bk;
EXTERN MAG_SENSOR_VAL ex_mag_adj;

EXTERN u16 ex_position_sensor;
EXTERN u16 ex_adjust_position_sensor;
EXTERN u8 ex_validation_sensor;

#define POSI_ALL_OFF			0x0000	// use 2024-02-14
#define POSI_ENTRANCE			0x0001
#define POSI_CENTERING			0x0002
#define POSI_APB_IN				0x0004
#define POSI_APB_OUT			0x0008
#define POSI_EXIT				0x0010

/* sensor_list[]の並びと下記のビットアサインが一致しないとだめなはず */
#define POSI_APB_HOME 			0x0020
#define POSI_CENTERING_HOME 	0x0040 // POSI_CENTERING_OPENを廃止してHOMEに変更
#define POSI_BOX_HOM 			0x0080
#define POSI_SHUTTER_OPEN 		0x0100	// OPEN状態
#define POSI_500BOX 			0x0200	// Boxありセンサとして使用
#define POSI_VALIDATION 		0x1000
#define POSI_ALL 				0x101F
#define POSI_ALL_WITHOUT_CIS 	0x001F

#define VALIDATION_1ST_SENSOR	0x01
#define VALIDATION_2ED_SENSOR	0x02

#define SENSOR_ENTRANCE			((ex_position_sensor & POSI_ENTRANCE) == POSI_ENTRANCE)
#define SENSOR_CENTERING		((ex_position_sensor & POSI_CENTERING) == POSI_CENTERING)
#define SENSOR_APB_IN			((ex_position_sensor & POSI_APB_IN) == POSI_APB_IN)
#define SENSOR_APB_OUT			((ex_position_sensor & POSI_APB_OUT) == POSI_APB_OUT)
#define SENSOR_EXIT				((ex_position_sensor & POSI_EXIT) == POSI_EXIT)			/* 0x0010 */
#define SENSOR_FEED_ALL_OFF		((ex_position_sensor & (POSI_ENTRANCE|POSI_CENTERING|SENSOR_APB_IN|SENSOR_APB_OUT|POSI_EXIT)) == 0x0000)
#if defined(UBA_RTQ)
	#define SENSOR_PUSHER_HOME		(ex_rc_status.sst1B.bit.stacker_home == 1)	
#else
	#define SENSOR_PUSHER_HOME		((ex_position_sensor & POSI_BOX_HOM) == POSI_BOX_HOM)	/* 0x0080 */
#endif
#define SENSOR_APB_HOME			((ex_position_sensor & POSI_APB_HOME) == POSI_APB_HOME)					/* 0x0020 */
#define SENSOR_CENTERING_HOME	((ex_position_sensor & POSI_CENTERING_HOME) == POSI_CENTERING_HOME)		/* 0x0040 */
#define SENSOR_SHUTTER_OPEN		((ex_position_sensor & POSI_SHUTTER_OPEN) == POSI_SHUTTER_OPEN)			/* 0x0100 */

#define SENSOR_ADJ_ENTRANCE		((ex_adjust_position_sensor & POSI_ENTRANCE) == POSI_ENTRANCE)
#define SENSOR_ADJ_CENTERING	((ex_adjust_position_sensor & POSI_CENTERING) == POSI_CENTERING)
#define SENSOR_ADJ_APB_IN		((ex_adjust_position_sensor & POSI_APB_IN) == POSI_APB_IN)
#define SENSOR_ADJ_APB_OUT		((ex_adjust_position_sensor & POSI_APB_OUT) == POSI_APB_OUT)
#define SENSOR_ADJ_EXIT			((ex_adjust_position_sensor & POSI_EXIT) == POSI_EXIT)

#if defined(UBA_RTQ)
	#define SENSOR_BOX 				(ex_rc_status.sst1B.bit.box_detect == 1)
#else
	#define SENSOR_BOX				((ex_position_sensor & POSI_500BOX) == POSI_500BOX)
#endif // UBA_RTQ

//2024-02-14 offに統一 #define SENSOR_VALIDATION		(ex_validation_sensor & (VALIDATION_1ST_SENSOR|VALIDATION_2ED_SENSOR))

//#if defined(UBA_RTQ)
//#define SENSOR_VALIDATION (ex_validation_sensor & (VALIDATION_1ST_SENSOR|VALIDATION_2ED_SENSOR))
//#endif // UBA_RTQ

#define SENSOR_VALIDATION_OFF	(ex_validation_sensor == 0) //2024-02-14
#define SENSOR_ONLY_ENTRANCE_ON			((ex_position_sensor & POSI_ALL ) ==  POSI_ENTRANCE) //2024-02-14
#define SENSOR_ALL_OFF					((ex_position_sensor & POSI_ALL ) ==  POSI_ALL_OFF) //2024-02-14
#define SENSOR_ALL_OFF_WITHOUT_CIS		((ex_position_sensor & POSI_ALL_WITHOUT_CIS) ==  POSI_ALL_OFF) //2024-02-14


// ex_position_ga 0がLow gain, 1がHigh Gain
#define GAIN_ENTRANCE 			((ex_position_ga & POSI_ENTRANCE) == POSI_ENTRANCE)
#define GAIN_CENTERING 			((ex_position_ga & POSI_CENTERING) == POSI_CENTERING)
#define GAIN_APB_IN 			((ex_position_ga & POSI_APB_IN) == POSI_APB_IN)
#define GAIN_APB_OUT 			((ex_position_ga & POSI_APB_OUT) == POSI_APB_OUT)
#define GAIN_EXIT 				((ex_position_ga & POSI_EXIT) == POSI_EXIT)
//ex_position_ga 0がLow gain, 1がHigh Gain
#define IS_HIGH_GAIN_ENTRANCE 	(ex_position_ga & POSI_ENTRANCE ? 1 : 0)
#define IS_HIGH_GAIN_CENTERING 	(ex_position_ga & POSI_CENTERING ? 1 : 0)
#define IS_HIGH_GAIN_APB_IN 	(ex_position_ga & POSI_APB_IN ? 1 : 0)
#define IS_HIGH_GAIN_APB_OUT 	(ex_position_ga & POSI_APB_OUT ? 1 : 0)
#define IS_HIGH_GAIN_EXIT 		(ex_position_ga & POSI_EXIT ? 1 : 0)

// Sensor ON/OFF
EXTERN FEED_PULSE_COUNT ex_feed_pulse_count;
EXTERN POSITION_AREA ex_position_pulse_count;
EXTERN POSITION_AREA exbk_prev_position_pulse_count;

/*----------------------------------------------------------------------*/
/* Temperature												         	*/
/*----------------------------------------------------------------------*/
typedef struct _Temperature{
	s16 cis_a;
	s16 cis_b;
	s16 outer;
	s16 hmot; //not exist
	s16 smot; //not exist
} Temperature;
EXTERN Temperature ex_temperature;

/*----------------------------------------------------------------------*/
/* Legacy Mode															*/
/*----------------------------------------------------------------------*/
typedef struct _Mode2Setting{
	u8 legacy_mode;
	u8 legacy_mode_chk;
	u8 reserved[14];
} Mode2Setting;
EXTERN Mode2Setting ex_mode2_setting;

/*----------------------------------------------------------------------*/
/* Adjustment												         	*/
/*----------------------------------------------------------------------*/
EXTERN ADJUSTMENT_DATA ex_adjustment_data;
EXTERN u16 ex_adjustment_fram_busy;
#define ADJ_FRAM_BUSY		0x01

/*----------------------------------------------------------------------*/
/* CIS Adjustment											         	*/
/*----------------------------------------------------------------------*/
EXTERN u32 ex_ad_sequence_mode;
EXTERN CIS_ADJUSTMENT_TBL ex_cis_adjustment_tbl;

#define CIS_ADJ_JUB_NONE			0x00
#define CIS_ADJ_JUB_ONE_SHOT		0x01
#define CIS_ADJ_JUB_ADJ_PAPER		0x01
#define CIS_ADJ_JUB_ADJ_NON_PAPER	0x02
#define CIS_ADJ_JUB_AD				0x04
#define CIS_ADJ_JUB_BC				0x08
#define CIS_ADJ_JUB_WC				0x10
#define CIS_ADJ_JUB_EEP_ACCCESS		0x20

/*----------------------------------------------------------------------*/
/* MAG Variables  		                                               */
/*----------------------------------------------------------------------*/
/* Mag Sensor */
EXTERN MAG_SENS_TEMP_ADJ_INFO s_sens_tempadj_ul_mag;
EXTERN MAG_SENS_TEMP_ADJ_INFO s_sens_tempadj_ur_mag;

EXTERN u16 ex_fram_sum;
EXTERN u16 ex_crc16_seed;
EXTERN u32 ex_sha1_seed[5];
/*----------------------------------------------------------------------*/
/* I/F Variables  		                                               */
/*----------------------------------------------------------------------*/
EXTERN u8 ex_rom_sha1[20];

/* Validation*/
EXTERN BV_MEMORY ex_validation;
#if DEBUG_VALIDATION_RESULT
EXTERN u16 ex_barcode_charactor_count;
#endif


/*--- バーコード関連変数 J9.5 ---*/
typedef struct _Barcom{
	u8 length;	/* Bar Length */
	u8 length2;	/* Bar Length */
	u8 length3;	/* Bar Length */
	u8 length4;	/* Bar Length */
	u8 enable;	/* Bar Enable/Disable 1:札のみ 2:ﾊﾞｰｸｰﾎﾟﾝ 3:両方*/
	u8 type;	/* Bar code type 1:i2of5,2:QR */
	u8 side;	/* Bar code direction 0:up, 1:down, 2:both */
} Barcom;
EXTERN Barcom ex_Barcom;
#define BARCODE_LENGTH	128
EXTERN u8 ex_barcode[BARCODE_LENGTH];
EXTERN u8 ex_bar_type;
EXTERN u8 ex_bar_length[2];
EXTERN u16 ex_bar_reject_code;

/*----------------------------------------------------------------------*/
/* PC Monitor Information									         	*/
/*----------------------------------------------------------------------*/
EXTERN MONITOR_INFO ex_monitor_info;

/*----------------------------------------------------------------------*/
/* Data Collection                                                      */
/*----------------------------------------------------------------------*/
EXTERN DATA_COLLECTION_INFO ex_collection_data;

/*----------------------------------------------------------------------*/
/* Machine Setting                                                      */
/*----------------------------------------------------------------------*/

	EXTERN bool ex_is_uba_mode;		/*Low mode, high mode*/
	EXTERN bool ex_is_cis_high;		//2024-05-28
	EXTERN bool ex_wid_reject_uba;	/*幅よせエラーでの返却*/
	EXTERN u8 ex_reject_retry_uba;	/*返却リトライは幅よせも動かすので、main管理にするかも*/

//	EXTERN bool ex_cheat_occurred;			/*識別センサが使用できないので、使用保留*/
	EXTERN u8 	ex_100msec_timer;
	EXTERN u8 	ex_100msec_motor;	//2024-11-21 1core
	EXTERN u8 	ex_100msec_wait;

	EXTERN u8 	ex_validation_sensor_on_count;

	EXTERN u16 	ex_1msec_timer;	//2022-01-15
	#define APB_REV_TIME		20		/* 20msec */
	EXTERN u16 pb_rev_time;

	#define PB_ENCODER_LIMIT	40	/* 1回転で40パルス以上はcheat */
	EXTERN u8 ex_pb_encoder_count;	/* PB1回転のエンコーダパルス数*/
	#define CENTERING_CONFIRM_HOME_OUT_TIME		40

	EXTERN u8 ex_2nd_note_uba;		/* 2022-02-21 */
	EXTERN bool	ex_centor_motor_run;		/* 幅よせモータ起動開始フラグ		*/
	EXTERN u16	ex_centor_motor_run_time;	/* 幅よせモータ動作タイムアウトタイマ	*/
	EXTERN bool	ex_centor_home_out;			/* 現在がHomeがHome Outかの情報フラグ		*/
	EXTERN u16	ex_centor_home_out_time;	/* Home Out状態の時間監視					*/

	EXTERN u8 ex_cis; //2024-06-04a

	#if defined(UBA_LOG)
	#define EX_BACK_LOG		20	//2025-07-30
	EXTERN u8 ex_back_log_index;
	EXTERN u8 ex_back_log[EX_BACK_LOG]; //パワーリカバリ用のフラグ遷移確認用

	#define EX_MODE_LOG		20	//2025-07-30
	EXTERN u8 ex_mode_log_index;
	EXTERN u8 ex_mode_log[EX_MODE_LOG]; //mode確認用

	#define EX_FREE_UBA		10
	EXTERN u8 ex_free_uba[EX_FREE_UBA];	/* backup用のログ尾として使用 *///ID-003 パワーリカバリ

	#define EX_FREE_UBA_DATA		50  //ID-003用
	EXTERN u8 ex_free_uba_no;
	EXTERN u8 ex_free_uba_data1[EX_FREE_UBA_DATA];
	EXTERN u8 ex_free_uba_data2[EX_FREE_UBA_DATA];
	EXTERN u8 ex_free_uba_data3[EX_FREE_UBA_DATA];
	#endif //end UBA_LOG

	#if defined(UBA_RTQ_AZ_LOG) //2023-09-05
	EXTERN u8 ex_fram_log_enable;
	#define	FRAM_UBA		20
	EXTERN u8 ex_fram_uba[FRAM_UBA];
	#endif

EXTERN u16 _ir_icb_ctrl_time_out;

/*----------------------------------------------------------------------*/
/* EEPROM BUFFER											         	*/
/*----------------------------------------------------------------------*/
#define EEPROM_MAX_SIZE		0x1000	/* S24C32C 4096 word */
EXTERN u8 ex_eeprom_data[EEPROM_MAX_SIZE];

/*----------------------------------------------------------------------*/
/* RFID 													         	*/
/*----------------------------------------------------------------------*/
EXTERN RFID_UNIT ex_rfid_unit;
EXTERN RFID_INFO ex_rfid_info;
/* [Transmitting and Reception memory] */
#define		EX_RFID_BLOCK_SIZE			4			/* Tag block */
EXTERN u8 ex_rfid_rx_data[2048];
EXTERN	u8	rfid_tx_rmb_no;							/* rmb = Read Multiple Block */
EXTERN	u8	rfid_tx_rmb_length;

#if 0
/* [Write data 256byte] */
#define		EX_RFID_TAG_BLOCK			60			/* Tag block */
#define		EX_RFID_TAG_DATA			5			/* Tag block address(1byte) +  Tag data save(4byte) */
EXTERN	u8	ex_rfid_wsb_set_data[EX_RFID_TAG_BLOCK][EX_RFID_TAG_DATA];		/* wsb = Write Single Block */
#endif

/* [Write data 2Kbyte] */
#define		EX_RFID_2K_BLOCK_SIZE		8			/* Tag block */
#define		EX_RFID_2K_TAG_BLOCK		250			/* Tag block */
#define		EX_RFID_2K_TAG_DATA			9			/* Tag block address(1byte) +  Tag data save(8byte) */
EXTERN	u8	ex_rfid_2k_wsb_set_data[EX_RFID_2K_TAG_BLOCK][EX_RFID_2K_TAG_DATA];		/* wsb = Write Single Block */
EXTERN	u8	ex_rfid_wsb_write_cnt;
EXTERN	u8	ex_rfid_wsb_set_cnt;

/*----------------------------------------------------------------------*/
/* Monitor 													         	*/
/*----------------------------------------------------------------------*/
EXTERN HAL_STATUS_TABLE ex_hal_status;

/*----------------------------------------------------------------------*/
/* Para Operation 													   	*/
/*----------------------------------------------------------------------*/
EXTERN MULTI_JOB ex_multi_job_alarm_backup;


/*----------------------------------------------------------------------*/
/* PLL Infomaiton 													   	*/
/*----------------------------------------------------------------------*/
EXTERN u8 clock_enabled[ALT_CLK_H2F_USER2+1];
EXTERN u32 clock_frequency[ALT_CLK_H2F_USER2+1];
EXTERN u8 safe_enabled[2];

#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
typedef struct _FRAM_LOG_BACKUP{
	u8 main_task_mode2;
	u8 main_task_mode1;
	u16 dummy;
	u16 stat;
	u16 port;
} FRAM_LOG_BACKUP;
EXTERN FRAM_LOG_BACKUP exbk_fram_log;
EXTERN u8 ex_fram_log_enable;
#endif



//#if (_DEBUG_FPGA_CLOCK_NOT_STOP==1)
EXTERN u8 ex_fpga_dummy_clk;
//#endif
/*
 * author suzuki-hiroyuki
 *
 * To change this generated comment edit the template variable "comment":
 * Window > Preferences > C/C++ > Editor > Templates.
 */


#include "icb_struct.h"


struct _Smrtdat
{
	u16	denomi[20];				/* ＢＯＸ内に収納されている各金種別紙幣の枚数	*/
	u16	denomi_reserved;		/*												*/
	u16	total;					/* 合計(Ticketを除くBox収納枚数)				*/
	u16	coupon;					/* クーポン券の総数(Box収納枚数) 				*/
	u16	totalin;				/* 挿入枚数の合計（含む返却）'03-06-16 			*/
	u8	cinfo[20];				/* クーポンインフォメーション 					*/
	u8	err[20];				/* 各エラーＮＯ．毎の発生回数 					*/
	u8	gameno[20];				/* ゲーム機ＮＯ． 								*/
	u8	boxno[20];				/* カセットＢＯＸのＮＯ． 						*/
	u8	ver[8];					/* Ver.											*/
	u8	rw_ver[20];				/* RFID-R/W firmware Ver.						*/
	u32	restim;					/* カセットボックスを外した日時 				*/
	u32	settim;					/* カセットＢＯＸをセットした日時 				*/
	u32	initim;					/* 集計データを初期化した日時 					*/
	u8	flg;					/* 集計データフラグ								*/
	u8	assign;					/* Currency Assign table No.					*/
	u8	id;						/* ID											*/
	u8	sum;					/* チェックサム									*/
};
struct _Smrtdat2
{
	u8 rej[5][3];				/* エスクロ位置からの返却コード別返却回数 (Rej2&4,6&7,8&9,10&13,14&15) */
	u8 rej_dummy;
	u8 crncy[20];				/* Country code(2bits) & Denomination(6bits)	*/
	u8 assign2;					/* Currency Assign table No.					*/
	u8 assign3;					/* Currency Assign table No.					*/
	u8 assign4;					/* Currency Assign table No.					*/
	u8 assign5;					/* Currency Assign table No.					*/
	u8 ticket_rej[8];			/* Ticket返却コード別返却回数 (Rej1,4,5,8,9,10,11)		*/
	u8 model[4];				/* model										*/
	u8 serial[6];				/* Serial number								*/
	u8 box_size;				/* BOX Size										*/
	u8 sum2;					/* チェックサム									*/
};
EXTERN struct _Smrtdat Smrtdat;			//RTQでは使用方法が異なっているので注意,RTQからのデータをこの変数に保存して、Boxの状態などを確認しているが、
										//その後、Smrtdat_framにコピーして、Smrtdat_framを更新している
										//こっちは、通信の受信バッファ的に使用する
EXTERN struct _Smrtdat2 Smrtdat2;

/*<<		ICB recovery Flag			>>*/
#if 1//#if (_RFID_BACK_DATA28==1) //ICBrecovery.data[] //(_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
	#define	ICB_MAX_BACK_COUNTER	17			/*		*/
#else
	#define	ICB_MAX_BACK_COUNTER	30			/*	30 iVIZION2	*/	
#endif

#define	ICB_RFID_BLOCK_SIZE		4			/*	4 iVIZION2	*/
#define	ICB_RECOVERY_MAX_BLOCK	7			/*	8 -> 7 iVIZION2	*/
typedef struct _ICBrecovery
{
	u8	BLK;		//送信するべきICBデータアサイン
	u8	_BLK;
	u8	data[ICB_RFID_BLOCK_SIZE * ICB_RECOVERY_MAX_BLOCK];	//実際のデータ
}ICBrecovery;
//EXTERN u8 ex_ICB_rfid_write_data_bufer[256];
//EXTERN u8 ex_ICB_rfid_read_data_bufer[256];
EXTERN u8 ex_rfid_2k_write_data_bufer[2000];
EXTERN u8 ex_rfid_2k_read_data_bufer[2000];
typedef struct _icb_setting_data
{
	u8 enable;
	u8 icb_ticket[8];
	u8 gameno[20];
	u8 boxno[20];
	u8 padding1[3];
	u32 initim;
	u8 reserved1[7];
	u8 sum;
} icb_setting_data;
EXTERN icb_setting_data ex_icb_setting; //FRAMにも書いているので、並び、サイズ変更など注意
/*<<	M/C setting ticket number of char		>>*/
#define	BAR_MC_LNG 16
#define	ICBTicket			ex_icb_setting.icb_ticket
#define	ex_ICB_gameno		ex_icb_setting.gameno	/* Head側のマシン番号*/
#define	ex_ICB_boxno		ex_icb_setting.boxno
#define	Savinitim			ex_icb_setting.initim
#define	ex_icb_Savsum		ex_icb_setting.sum
/*	ICB Function Flag 0x55の時のみINHIBIT状態	*/
#define	ex_ICB_systemInhiStaus		ex_icb_setting.enable
#define INHIBIT_ICB	(u8)0x55


#if defined(UBA_RTQ_ICB)
	#define RFID_DENOMI			0x01
	#define RFID_DENOMI_UNIT	0x02
	#define RFID_BACK			0x03

	#define RFID_BACK_ACCEPT	0x01
	#define RFID_BACK_COLECT	0x02
	#define RFID_BACK_PAYOUT	0x03

	EXTERN	bool ex_rtq_rfid_data; 	//ex_rtq_rfid_data
	//#if defined(NEW_RFID)	//2025-07-04
	EXTERN u8 ex_rtq_rfid_write_disable;			//ex_rtq_rfid_write_disable //0以外はRFID書き込みへ遷移禁止
	EXTERN u16 ex_rtq_rfid_res_totaltime;	//ex_rtq_rfid_res_totaltime //RTQとの通信でRFID書き込みコマンドのレスポンスでENQが続いている時間
	//#endif

	struct _Smrtdat_fram
	{
		u16	denomi[20];		//use		/* ＢＯＸ内に収納されている各金種別紙幣の枚数	*/
		u16	denomi_reserved;		/*												*/
		u16	total;					/* 合計(Ticketを除くBox収納枚数)				*/
		u16	coupon;					/* クーポン券の総数(Box収納枚数) 				*/
		u16	totalin;				/* 挿入枚数の合計（含む返却）'03-06-16 			*/
		u8	cinfo[20];				/* クーポンインフォメーション 					*/
		u8	err[20];				/* 各エラーＮＯ．毎の発生回数 					*/
		u8	gameno[20];				/* ゲーム機ＮＯ． 								*/
		u8	boxno[20];				/* カセットＢＯＸのＮＯ． 						*/
		u8	ver[8];					/* Ver.											*/
		u8	rw_ver[20];				/* RFID-R/W firmware Ver.						*/
		u32	restim;					/* カセットボックスを外した日時 				*/
		u32	settim;					/* カセットＢＯＸをセットした日時 				*/
		u32	initim;					/* 集計データを初期化した日時 					*/
		u8	flg;					/* 集計データフラグ								*/
		u8	assign;					/* Currency Assign table No.					*/
		u8	id;						/* ID											*/
		u8	sum;			//use		/* チェックサム									*/
	};
	EXTERN struct _Smrtdat_fram Smrtdat_fram; //172

	//#if defined(RFID_RECOVER)
	struct _Smrtdat_fram_bk //2025-08-03
	{
		u8  mode;
		u8  unit;
	};

	//動作時は、Smrtdat_fram_bk をFRAMに書き込んでバックアップ
	//電源ONでインタフェースで Smrtdat_fram_bk の情報をもとに必要であれば Smrtdat_fram_bk_power にコピー
	//イニシャル動作の最後で、Smrtdat_fram_bk_power を元にリカバリを行う
	//イニシャルの最後にHeadのFRAM領域に書き込むが、実際のRTQ側への書き込み命令は待機時
	EXTERN struct _Smrtdat_fram_bk Smrtdat_fram_bk;			//RTQのRFIDのリカバリ用 バックアップしている
	EXTERN struct _Smrtdat_fram_bk Smrtdat_fram_bk_power;	//RTQのRFIDのリカバリ用 バックアップしていない、ID-003で必要ならSmrtdat_fram_bkをコピーして、イニシャルの最後に書き込み
	//#endif

#endif

#if !defined(UBA_RTQ_ICB) //2025-08-21
	typedef struct _icb_backup_data
	{
		u32 write_flg;
		u8 save_num;
		u8 send_num;
		ICBrecovery block[ICB_MAX_BACK_COUNTER];
	} icb_backup_data;
	EXTERN icb_backup_data ex_icb_recovery; //516
	#define ex_Info_ICBrecovery ex_icb_recovery.write_flg
	#define ex_ICBsave_num ex_icb_recovery.save_num
	#define ex_ICBsend_num ex_icb_recovery.send_num
	#define ex_ICBrecovery ex_icb_recovery.block
#else
	typedef struct _icb_backup_data
	{
		u32 write_flg;
		u8 save_num;
		u8 send_num;
		ICBrecovery block[ICB_MAX_BACK_COUNTER];
	} icb_backup_data;
	EXTERN icb_backup_data ex_icb_recovery_blank; //516 RTQでは使用しないが、SSの為にあける為のサイズ
#endif


typedef struct _ICB_ALARM_BACKUP
{
	u32 mode1;
	u32 mode2;
	u32 rsp_msg;
	u32 code;
	u32 seq;
	u32 sensor;
}ICB_ALARM_BACKUP;
EXTERN ICB_ALARM_BACKUP ex_icb_alarm_backup;

/* タスク間の引数にしてもいいかも */
EXTERN u16 ex_rom_crc16;
EXTERN u32 ex_rom_crc32;

#if (LOOPBACK_UBA==1)
EXTERN u8 ex_loopback_error;
#endif

#if (defined(_PROTOCOL_ENABLE_ID003))
EXTERN struct	_Authentication	ex_Authentication;
EXTERN u16	ex_authentication_sum;
#endif

EXTERN u32 ex_fpga_version;

EXTERN u8	s_id003_2nd_note; //use line task
EXTERN u32	s_id003_2nd_note_code; //use line task

EXTERN	bool red_on;		// LED
EXTERN	bool green_on;		// LED
EXTERN u8 ex_uba_ore_current;
EXTERN u8	ex_force_stack_retry;	//2023-11-27
EXTERN u16 _cyc_validation_mode; //2024-02-26
EXTERN u16 ex_rc_skew_pulse;	//2024-08-19

#if defined(UBA_RTQ)
/*------------------------------------------------*/
/*-- Software informarion						--*/
/*------------------------------------------------*/
EXTERN u8 ex_model_bk[16]; //2025-01-21 いる ok
EXTERN u8 ex_country_bk[8];
EXTERN u8 ex_protocol_bk[8];
//not use EXTERN u8 ex_version_bk[8];
//not use EXTERN u8 ex_date_bk[8];


/*------------------------------------------------*/
/*-- Recycler情報								--*/
/*------------------------------------------------*/
struct	_recyclerSoftInfo
{
	u8	BootRomid[28];							/* Recycler Boot Software Version	*/
	u8	FlashRomid[28];							/* Recycler Flash Software Version	*/
	u8	FlashCheckSum[2];						/* Recycler Flash Software CheckSum	*/
};
EXTERN struct _recyclerSoftInfo	RecycleSoftInfo; //2025-01-21 いらない ok
EXTERN struct _recyclerSoftInfo	RecycleSoftInfo_bk; //2025-01-21 いらない ok


/* リサイクル金種情報(バックアップする) */
/*< リサイクル金種情報 <紙幣情報> >*/
struct	_billInfo
{
	u8	Length;				/* 紙幣長			*/
	u8	Max;				/* 許容最小紙幣長	*/
	u8	Min;				/* 許容最大紙幣長	*/
};
EXTERN struct	_billInfo	BillInfo;


/*< リサイクル金種情報 <設定情報> >*/
struct	_recycleDenomi
{
	u8	BoxNumber;				/* Box Number		*/
	u8	Data1;					/* 設定金種 DATA1	*/
	u8	Data2;					/* 設定金種 DATA2	*/
	u8	RecycleLimit;			/* 還流上限枚数		*/
	struct	_billInfo	BillInfo;
	u8	PayoutCode;
	u8	RecycleCurrent;			/* 保有枚数			*/
};
EXTERN struct _recycleDenomi	cRecycleDenomi;


/*< リサイクル金種情報 >*/
struct	_recycleSettingInfo
{
	struct _recycleDenomi	DenomiInfo[4];
	u8	key;
};
EXTERN struct _recycleSettingInfo	RecycleSettingInfo;	 //2025-01-21 いらないけど覚えた方がいい 37byte
EXTERN struct _recycleSettingInfo	RecycleSettingInfo_bk;  //2025-01-21 いらない //バックアップは必要ない ok
// UBA500もバックアップエリアにしているが実際は,起動時にRTQからの設定を受信してそれを設定しているだけ。


/*------------------------------------------------*/
/*-- I/F Total Count(枚数管理)					--*/
/*------------------------------------------------*/
struct	_rcAcclogIF
{
	u32	AcceptCount;							/* 受取枚数						*/
	u32	PayoutCount;							/* 払出し枚数					*/
	u32	CollectCount;							/* 回収枚数						*/
};

struct	_rcLogdatIF
{
	struct	_rcAcclogIF	rcLogIF[4];				/* [0] : Recycle box No.1, [1] : Recycle box No.2	*/
};
EXTERN	struct _rcLogdatIF	rcLogdatIF;  //2025-01-21 いる ok

EXTERN u8 rc_before_mode;  //2025-01-21 いる ok
EXTERN u8 rc_before_model;  //2025-01-21 いる ok

/*------------------------------------------------*/
/*-- HEAD SPEED情報								--*/
/*------------------------------------------------*/
EXTERN u16 uba_feed_speed_fwd;  //2025-01-21 いらない ok
EXTERN u16 uba_feed_speed_rev;	//2025-01-21 いらない ok

/*------------------------------------------------*/
/*-- Recycler情報								--*/
/*------------------------------------------------*/
struct _MENTE_SERIALNO_DATA
{
	u8 fram_param;
	u8 version[2];
	u8 date[8];
	u8 serial_no[12];
	u8 read_end;
};
EXTERN struct _MENTE_SERIALNO_DATA read_mente_serailno_data[3]; //おそらくバックアップの必要ない
//起動時やイニシャル時にRTQから取得している


//not use UBA500RTQ EXTERN struct _MENTE_SERIALNO_DATA write_mente_serailno_data[3];

//起動時に上記の変数をコピーしてバックアップしている
//RTQのみであり、リサイクラを入れ替えられた時の判定用なので、生産時に対応してなくてもいい
EXTERN struct _MENTE_SERIALNO_DATA read_mente_serailno_data_bk[3]; //バックアップは必要な可能性が高い


//not use UBA500RTQ EXTERN struct _MENTE_SERIALNO_DATA write_mente_serailno_data_bk[3];
//not use UBA500RTQ only use A_PRO EXTERN struct _MENTE_SERIALNO_DATA store_mente_serailno_data[3];

/*------------------------------------------------*/
/*-- 暗号関連									--*/
/*------------------------------------------------*/
#define RC_GC2_ROUNDS          2
#define RC_GC2_BLOCK_LEN       8
EXTERN u8 ex_encryption_number;  //2025-01-21 いる NG バックアップ必要ない様な RTQとの通信Pay out暗号化
EXTERN u8 ex_encryption_key[8];
EXTERN u8 ex_encryption_key_no;
EXTERN u8 ex_cbc_context[RC_GC2_BLOCK_LEN];
EXTERN u8 ex_cbc_context_work[RC_GC2_BLOCK_LEN];
EXTERN u8 ex_last_encryption_cmd;//2025-01-21 いる NG バックアップ必要ない様な RTQとの通信Pay out暗号化


//EXTERN u8 ex_rc_after_jam;		// 1:drum動作中にJam発生のの可能性あり//2025-01-21 いる NG CLINE_STATUS_TBL へ移動
EXTERN u8 ex_line_stacker_home;
EXTERN u8 ex_line_box_detect;	//2024-07-16 /* FLAG box - home sensor detect */
#endif


#if defined(UBA_RTQ)
	#define REWAIT_RDY_OFF		0
	#define REWAIT_RDY_ON		1
	#define REWAIT_RDY_WAIT		2

	EXTERN RC_TX_BUFFER _rc_tx_buff;
	EXTERN RC_RX_BUFFER _rc_rx_buff;
	EXTERN RC_TX_ENCRYPTION_BUFFER _rc_tx_enc_buff;
	EXTERN u8 ex_main_payout_flag;
	EXTERN u8 ex_main_collect_flag; //コレクト動作したいが、別のタスクが動作しているので保留状態,動作開始でクリア
	//EXTERN u8 ex_main_sw_collect_flag; //UBA500はこれとex_rc_collect_swの2種類あるのでex_rc_collect_swに統一する
	EXTERN u8 ex_main_emergency_flag;
	/* ステータス情報(バックアップしない) */
	// Unit Status(SST1A)
	typedef struct _RC_STATUS
	{
		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 reset				:1;				// Reset(Reset処理中)
				u8 busy					:1;				// Busy(コマンド実行中)
				u8 error				:1;				// Error(Error処理中)
				u8 initial				:1;				// Initialize(初期化中)
				u8 pause				:1;				// Pause(一時停止)
				u8 download				:1;				// Download Busy状態
				u8 warning				:1;				// Warning発生中
				u8 quad					:1;				// RC-Quad接続信号
			}bit;
		}sst1A;

		// Unit Status(SST1B)
		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 download_ready		:1;				// Download Ready状態
				u8 collect_sw			:1;				// 回収SW状態							[ 1度検知した場合はクリアコマンド受信まで状態を保持]
				u8 stacker_home			:1;				// 収納Box押し板Home検知センサー		[ 0:Home以外	1:Home ]
				u8 box_detect			:1;				// 収納Box検知センサー					[ 0:Boxなし		1:Boxあり ]
				u8 stat_bit0			:1;				// 入出金状態							[ 0:IDLE, 1:入金中(Vend前), 2:入金中(Vend後), 3:出金中(Vend前), 4:出金中(Vend後), 5:回収中, 6:Reject ]
				u8 stat_bit1			:1;				//
				u8 stat_bit2			:1;				//
				u8 stat_bit3			:1;				//
			}bit;
		}sst1B;

		// Unit Status(SST2_1A)
		#define	RC1_POS1_POS2_POS3			0x38						//
		#define	RC1_POS2_POS3				0x30						//
		#define	RC1_POS1_POS2				0x18						//
		#define	RC_POS1_ON					ex_rc_status.sst21A.bit.pos_sen1 == 1
		#define	RC_POS2_ON					ex_rc_status.sst21A.bit.pos_sen2 == 1
		#define	RC_POS3_ON					ex_rc_status.sst21A.bit.pos_sen3 == 1

		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 flap1_senA			:1;				// フラッパーセンサーA				(RC-Twin)		[ 0:未検知　	1:検知 ]
				u8 flap1_senB			:1;				// フラッパーセンサーB				(RC-Twin)		[ 0:未検知　	1:検知 ]
				u8 flap1_senRC1			:1;				// フラッパーセンサーRC1			(RC-Twin)		[ 0:未検知　	1:検知 ]
				u8 pos_sen1				:1;				// ポジションセンサー1				(RC-Twin)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_sen2				:1;				// ポジションセンサー2				(RC-Twin)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_sen3				:1;				// ポジションセンサー3				(RC-Twin)		[ 0:紙幣なし	1:紙幣あり ]
				u8 battery_low			:1;				// battery for optional function					[ 0:低下		1:正常 ]
				u8 u1_detect_pdw		:1;				// unit1 detect during power down					[ 1:脱履歴あり	1:脱履歴なし ]
			}bit;
		}sst21A;

		// Unit Status(SST2_1B)
		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 u1_detect_dbl		:1;				// 重券検知										[ 1度検知した場合はクリアコマンド受信まで状態を保持]
				u8 u1_detect			:1;				// ユニット検知					(RC-Twin)		[ 0:未検知　	1:検知 ]
				u8 flmot1_state_lo		:1;				// フラッパー1モータ制御状態(L)	(RC-Twin)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 flmot1_state_hi		:1;				// フラッパー1モータ制御状態(H)	(RC-Twin)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 fmot3_state_lo		:1;				// feed motor3 state(L)							[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 fmot3_state_hi		:1;				// feed motor3 state(H)							[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 flap1_lever			:1;				// TWINフラッパーレバー			(RC-Twin)		[ 0:未検知　	1:検知 ]
				u8 flap2_lever			:1;				// QUADフラッパーレバー			(RC-Quad)		[ 0:未検知　	1:検知 ]
			}bit;
		}sst21B;

		// Unit Status(SST2_2A)
		#define	RC2_POS4_POS5_POS6			0x38						//
		#define	RC_POS4_ON					ex_rc_status.sst22A.bit.pos_sen4 == 1
		#define	RC_POS5_ON					ex_rc_status.sst22A.bit.pos_sen5 == 1
		#define	RC_POS6_ON					ex_rc_status.sst22A.bit.pos_sen6 == 1

		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 flap2_senC			:1;				// フラッパーセンサーC				(RC-Quad)		[ 0:未検知　	1:検知 ]
				u8 flap2_senD			:1;				// フラッパーセンサーD				(RC-Quad)		[ 0:未検知　	1:検知 ]
				u8 flap2_senRC2			:1;				// フラッパーセンサーRC2			(RC-Quad)		[ 0:未検知　	1:検知 ]
				u8 pos_sen4				:1;				// ポジションセンサー4				(RC-Quad)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_sen5				:1;				// ポジションセンサー5				(RC-Quad)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_sen6				:1;				// ポジションセンサー6				(RC-Quad)		[ 0:紙幣なし	1:紙幣あり ]
				u8 battery_detect		:1;				// battery detect									[ 1:batteryなし	0:batteryあり ]
				u8 u2_detect_pdw		:1;				// unit2 detect during power down					[ 1:脱履歴あり	1:脱履歴なし ]
			}bit;
		}sst22A;

		// Unit Status(SST2_2B)
		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 u2_detect_dbl		:1;				// 重券検知										[ 1度検知した場合はクリアコマンド受信まで状態を保持]
				u8 u2_detect			:1;				// ユニット検知					(RC-Quad)		[ 0:未検知　	1:検知 ]
				u8 flmot2_state_lo		:1;				// フラッパー2モータ制御状態(L)	(RC-Quad)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 flmot2_state_hi		:1;				// フラッパー2モータ制御状態(H)	(RC-Quad)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 exbox_detect			:1;				// exbox detect
				u8 ex_box_sol_state		:1;				// exbox sol state
				u8 pos_senTRP			:1;				// position sensor TRP for exbox
				u8 pos_senEND			:1;				// position sensor END for exbox
			}bit;
		}sst22B;

		// Unit Status(SST3_1A)
		#define	RC1_POSA_POSB_POSC			0x07						//
		#define	RC_POSA_ON					ex_rc_status.sst31A.bit.pos_senA == 1
		#define	RC_POSB_ON					ex_rc_status.sst31A.bit.pos_senB == 1
		#define	RC_POSC_ON					ex_rc_status.sst31A.bit.pos_senC == 1

		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 pos_senA				:1;				// ポジションセンサーA				(RC-Twin)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_senB				:1;				// ポジションセンサーB				(RC-Twin)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_senC				:1;				// ポジションセンサーC				(RC-Twin)		[ 0:紙幣なし	1:紙幣あり ]
				u8 u1_d1_full			:1;				// ドラム1フル						(RC-Twin)		[ 0:未検知		1:フル状態 ]
				u8 u1_d2_full			:1;				// ドラム2フル						(RC-Twin)		[ 0:未検知		1:フル状態 ]
				u8 u1_d1_empty			:1;				// ドラム1エンプティ				(RC-Twin)		[ 0:未検知		1:エンプティ状態 ]
				u8 u1_d2_empty			:1;				// ドラム2エンプティ				(RC-Twin)		[ 0:未検知		1:エンプティ状態 ]
				u8 reserve7				:1;				//
			}bit;
		}sst31A;

		// Unit Status(SST3_1B)
		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 reserve0				:1;				//
				u8 u1_sol_state			:1;				// unit1 sol state
				u8 fmot1_lo				:1;				// 搬送1モータ制御状態(L)		(RC-Twin)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 fmot1_hi				:1;				// 搬送1モータ制御状態(H)
				u8 u1_dmot1_lo			:1;				// ドラム1モータ制御状態(L)		(RC-Twin)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 u1_dmot1_hi			:1;				// ドラム1モータ制御状態(H)
				u8 u1_dmot2_lo			:1;				// ドラム2モータ制御状態(L)		(RC-Twin)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 u1_dmot2_hi			:1;				// ドラム2モータ制御状態(H)
			}bit;
		}sst31B;

		// Unit Status(SST3_2A)
		#define	RC2_POSD_POSE_POSF			0x07						//
		#define	RC_POSD_ON					ex_rc_status.sst32A.bit.pos_senD == 1
		#define	RC_POSE_ON					ex_rc_status.sst32A.bit.pos_senE == 1
		#define	RC_POSF_ON					ex_rc_status.sst32A.bit.pos_senF == 1

		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 pos_senD				:1;				// ポジションセンサーD				(RC-Quad)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_senE				:1;				// ポジションセンサーE				(RC-Quad)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_senF				:1;				// ポジションセンサーF				(RC-Quad)		[ 0:紙幣なし	1:紙幣あり ]
				u8 u2_d1_full			:1;				// ドラム1フル						(RC-Quad)		[ 0:未検知		1:フル状態 ]
				u8 u2_d2_full			:1;				// ドラム2フル						(RC-Quad)		[ 0:未検知		1:フル状態 ]
				u8 u2_d1_empty			:1;				// ドラム1エンプティ				(RC-Quad)		[ 0:未検知		1:エンプティ状態 ]
				u8 u2_d2_empty			:1;				// ドラム2エンプティ				(RC-Quad)		[ 0:未検知		1:エンプティ状態 ]
				u8 reserve7				:1;				//
			}bit;
		}sst32A;

		// Unit Status(SST3_2B)
		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
	//#if defined(A_PRO)	/* '22-04-19 */
				u8 collect_empty		:1;				// 回収動作時のEMPTY検出
	//#else
	//			u8 reserve0				:1;				//
	//#endif
				u8 u2_sol_state			:1;				// unit2 sol state
				u8 fmot2_lo				:1;				// 搬送2モータ制御状態(L)		(RC-Quad)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 fmot2_hi				:1;				// 搬送2モータ制御状態(H)
				u8 u2_dmot1_lo			:1;				// ドラム1モータ制御状態(L)		(RC-Quad)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 u2_dmot1_hi			:1;				// ドラム1モータ制御状態(H)
				u8 u2_dmot2_lo			:1;				// ドラム2モータ制御状態(L)		(RC-Quad)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 u2_dmot2_hi			:1;				// ドラム2モータ制御状態(H)
			}bit;
		}sst32B;

	//#if defined(RC_BOARD_GREEN)
		#define RS_POS1_POS2_POS3_POSRD		0x0F
		#define	RS_POS1_ON					ex_rc_status.sst4A.bit.pos_sen1 == 1
		#define	RS_POS2_ON					ex_rc_status.sst4A.bit.pos_sen2 == 1
		#define	RS_POS3_ON					ex_rc_status.sst4A.bit.pos_sen3 == 1
		#define	RS_REMAIN_ON				ex_rc_status.sst4A.bit.pos_senR == 1
		#define	RS_FLAP_IN_POS				ex_rc_status.sst4A.bit.flap_sen1 == 1
		#define	RS_FLAP_OUT_POS				ex_rc_status.sst4A.bit.flap_sen2 == 1

		union
		{
		// Unit Status(SST4A)
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 pos_sen1				:1;				// ポジションセンサー1				(RS)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_sen2				:1;				// ポジションセンサー2				(RS)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_sen3				:1;				// ポジションセンサー3				(RS)		[ 0:紙幣なし	1:紙幣あり ]
				u8 pos_senR				:1;				// 残留検知センサー					(RS)		[ 0:紙幣なし	1:紙幣あり ]
				u8 flmot_state_lo		:1;				// フラッパーモータ制御状態(L)		(RS)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 flmot_state_hi		:1;				// フラッパーモータ制御状態(H)		(RS)		[ 0:停止	1:正転		2:逆転		3:ブレーキ ]
				u8 flap_sen1			:1;				// フラッパーセンサー1				(RS)		[ 0:未検知　	1:検知 ]
				u8 flap_sen2			:1;				// フラッパーセンサー2				(RS)		[ 0:未検知　	1:検知 ]
			}bit;
		}sst4A;

		// Unit Status(SST4B)
		union
		{
			u8 byte;						/*  Byte Access */
			struct
			{
				u8 reserve0				:1;				//
				u8 reserve1				:1;				//
				u8 reserve2				:1;				//
				u8 reserve3				:1;				//
				u8 reserve4				:1;				//
				u8 reserve5				:1;				//
				u8 reserve6				:1;				//
				u8 reserve7				:1;				//
			}bit;
		}sst4B;
	//#endif  // RC_BOARD_GREEN

	}RC_STATUS;


	/*< 払出し(回収)動作関連 >*/
	struct	_operationDenomi
	{
		u8 unit;			// 払出し(回収) Recycle Unit No.
		u8 unit_bk;			// 基本 mode_acceptのみ使用、stackコマンド受信後にこれを unit にコピー
		u8 count;			// 払出し(回収) 枚数
		u8 code;
		u8 remain;
		u8 mode;
		u8 pre_feed;		// プレフィード有無
		u8 unit_emergency;  // エマージェンシーストップ時の払い出しUnnit No.
		u8 unit_retry;		// リトライ動作時の払出しUnit No.
	};
	EXTERN	struct _operationDenomi	OperationDenomi;


	EXTERN volatile RC_STATUS ex_rc_status;

	EXTERN u32 ex_rc_dl_offset;
	EXTERN u32 ex_rc_rewait_rdy_exec_command;
	EXTERN u16 _ir_feed_motor_da_control;
	EXTERN volatile u16 ex_rc_ready_timeout;
	EXTERN volatile u16 ex_rc_detect_time;
	EXTERN u8 ex_rc_detect_next_box_open;
	EXTERN u16 ex_rc_task_seq;
	EXTERN u16 ex_rc_task_rewait_rdy_seq;
	EXTERN u8 ex_rc_dip_sw;
	EXTERN u8 ex_rc_error_flag;
	EXTERN u8 ex_rc_error_status;
	EXTERN u8 ex_rc_warning_status;	//UBA700RTQで新規に追加 2025-08-19
									//イニシャルでドラムの位置決めがうまくいなない時に、
									//2回に1回程度、RTQのステータスがエラーより先にワーニングになり、その後エラーとなる場合がある。
									//ex_rc_error_statusを共通使用していた為、先に発生するワーニングはmainに通知するが、
									//その後のエラーはmianに通知していなかった。そのため、イニシャルステータスのままとなっていた
									//(開発内 評価)
	EXTERN u8 ex_rc_aging_seq;
	EXTERN u8 ex_rc_aging_counter;

	EXTERN u8 ex_rc_exchanged_unit;
	EXTERN u8 ex_rc_exchanged_unit_powerup;
	EXTERN u8 ex_pre_feed_after_jam;

	EXTERN u8 ex_rc_collect_sw; //コレクト動作する場合は動作の最後にクリア、それ以外にキャンセルの場合、main側でメッセージ受信直後にクリア
	EXTERN u8 ex_rc_powerup_error;
	EXTERN u8 ex_rc_enable;
	EXTERN u8 ex_rc_data_lock;
	EXTERN u8 ex_rc_internal_jam_flag;
	EXTERN u8 ex_rc_internal_jam_flag_bk;
	EXTERN u8 ex_init_add_enc_flg;
	EXTERN u8 ex_rc_download_ready;
	EXTERN u8 ex_rc_rewait_rdy_flg;
	EXTERN u8 ex_rc_flap_test_flg;
	EXTERN u16 ex_rc_flap_test_time;

	EXTERN u16 ex_rc_error_code;

	EXTERN u8 ex_rc_option_battery;						/* Flag detect battery */
//	EXTERN u8 ex_rc_option_battery_low_detect;			/* RC-TQ用　Battery Low detect flag *///2025-01-21 いる NG ex_cline_status_tbl へ

	/*--------------------------------------------------*/
	/*		RC Downlaod status通知機能		'24-09-13	*/
	/*--------------------------------------------------*/
	EXTERN u8 ex_rc_download_stat;		// status
	EXTERN u8 ex_rc_download_flag;		// flag

	/*--------------------------------------------------*/
	/*		RC Skew->Cash box機能						*/
	/*--------------------------------------------------*/
	EXTERN u8 ex_rc_skew_validation_on;

	//EXTERN u8 ex_rc_skew_detect;

	/*--------------------------------------------------*/
	/*		Diagnostic 機能								*/
	/*--------------------------------------------------*/
	EXTERN u8 ex_diag_status;
	EXTERN u8 ex_diag_emg;

	struct _SHIP_EDITION_DATA
	{
		u8 head[1];
		u8 main[1];
		u8 twin[1];
		u8 quad[1];
		u8 read_end;
	};
	EXTERN struct _SHIP_EDITION_DATA read_editionno_data;
	EXTERN struct _SHIP_EDITION_DATA write_editionno_data;

	/* TEST MODE */
	EXTERN u8 ex_rc_test_type;
	EXTERN u8 ex_rc_test_maintenance;

	struct _SHIP_SERIALNO_DATA
	{
		u8 fram_param;
		u8 version[2];
		u8 date[8];
		u8 serial_no[12];
		u8 read_end;
	};
	#if 1 //2024-10-16
	//UBA500と異なりセマフォ処理なので、RTQからの読み込み完了とHeadの読み込み完了の両方を待つ必要がある
	//bit処理に変更
		#define READ_NONE			0x00

		#define READ_RTQ_EXEC		0x01
		#define READ_HEAD_EXEC		0x02
		#define READ_RTQ_HEAD_EXEC	0x03

		#define READ_ERR			0x0C

		#define READ_RTQ_END		0x10
		#define READ_HEAD_END		0x20
		#define READ_RTQ_HEAD_END	0x30
	#else
		#define READ_NONE			0x00
		#define READ_EXEC			0x01
		#define READ_ERR			0x02
		#define READ_END			0xFF
	#endif


	EXTERN struct _SHIP_SERIALNO_DATA read_serailno_data;
	EXTERN struct _SHIP_SERIALNO_DATA write_serailno_data;

	// センサー調整
	EXTERN u8 ex_sens_adj_end;
	EXTERN u8 ex_sens_adj_fram_end;
	#define ADJ_NONE			0x00
	#define ADJ_EXEC			0x01
	#define ADJ_ERR				0x02
	#define ADJ_WRITE_END		0xF0
	#define ADJ_READ_EXEC		0xFE
	#define ADJ_READ_END		0xFF

	struct _RC_SENSOR_ADJ_DATA
	{
		u8 ad600da;
		u8 ad800da;
		u8 da;
		u8 ad600[2];
		u8 ad800[2];
		u8 ad[2];
	};
	EXTERN u8 ex_sens_adj_err_data[5];
	#define RC_SENSOR_MAX	17
	EXTERN struct _RC_SENSOR_ADJ_DATA ex_rc_adj_data[RC_SENSOR_MAX];
	/*
	0 : pos1
	1 : pos2
	2 : pos3
	3 : posA
	4 : posB
	5 : posC
	6 : tape1
	7 : tape2
	8 : exbox
	9 : pos4
	10 : pos5
	11 : pos6
	12 : posD
	13 : posE
	14 : posF
	15 : tape3
	16 : tape4
	*/
	EXTERN u8 ex_rc_retry_flg; //TRUE RTQがエラーだろうと無視して動作させる
	EXTERN u8 ex_rc_retry_count;
	EXTERN u8 ex_rc_motor_speed[7][2];
	EXTERN u8 ex_rc_motor_duty[7];
	EXTERN u8 ex_rc_test_status;
	#define RC_BUSY_STATUS		0x00
	#define RC_STANDBY_STATUS	0x10
	EXTERN u8 ex_rc_flap_test_flg;
	EXTERN u16 ex_rc_flap_test_time;
	EXTERN u16 ex_rc_flap_test_time_send;
	EXTERN u8 ex_test_end_flg;
	EXTERN u8 ex_fram_check;
	EXTERN u16 ex_perform_test_data[20];
	EXTERN u8 ex_perform_test_fram_end;
	#define PTEST_NONE			0x00
	#define PTEST_EXEC			0x01
	#define PTEST_ERR			0x02
	#define PTEST_WRITE_END		0xF0
	#define PTEST_READ_EXEC		0xFE
	#define PTEST_READ_END		0xFF

	/* ------- */

	/* RC FRAM LOG*/
	#define FRAM_LOG_DATA_MAX	200
	struct _RC_FRAM_LOG_DATA
	{
		u16 read_offset;
		u8 read_length;
		u8 read_data[FRAM_LOG_DATA_MAX];
		u8 wait_flg;
	};
	EXTERN struct _RC_FRAM_LOG_DATA rc_fram_log;

		//#if defined(RC_BOARD_GREEN)		/* '23-09-20 */
		/*--------------------------------------------------*/
		/*		RS UNIT										*/
		/*--------------------------------------------------*/
		#define	RC_OLD_BOARD			0
		#define	RC_NEW_BOARD			1

		#define	RS_CONNECT				1
		#define	RS_NOT_CONNECT			0

		#define	NOT_CONNECT_RFID		0
		#define	CONNECT_RFID			1

		struct _RC_CONFIGURATION
		{
			u8 unit_type;			/* 0:None		1:RS UNIT  		*/
			u8 rfid_module;			/* 0:None		1:RFID MODULE	*/
			u8 board_type;			/* 0:Old board	1:New board		*/
			//#if defined(UBA_RS)
			u8 unit_type_bk;		/* 0:None		1:RS UNIT  		*//*パワーリカバリ用ではなく、RSエージングテスト用 */
			//#endif
		};
		EXTERN struct _RC_CONFIGURATION ex_rc_configuration;

		struct _RC_NEW_ADJUSTMENT_DATA
		{
			u8 pos[3];
			u8 da[26];
			u8 gain[3];
		};
		EXTERN struct _RC_NEW_ADJUSTMENT_DATA ex_rc_new_adjustment_data;

		/* RC RFID varialbe*/
		EXTERN u8 ex_rc_rfid_read_data_bufer[256];

		//#endif /* RC_BOARD_GREEN */

//2025-01-16a 下記、最新ソース移植により
EXTERN	u8	ConcurrentPayoutFlag;		//2025-01-21 不明		
EXTERN	u8	CurrentBoxNumberIndx;						
EXTERN	u8	OperationDenomiCount[4];			/* Amount to pay 	*/	
EXTERN	u8	OperationDenomiBoxNumber[4];		/* Box number 		*/		
#define	RS_NOTE_REMAIN_NONE			0x00
#define RS_NOTE_REMAIN_CHECK		0x01 //ID-003のpayoutコマンド受信で有効
#define	RS_NOTE_REMAIN_CONFIRM		0x02 //待機状態で札をRSセンサで検知している、ID-003の通知に使用

EXTERN u8 ex_rs_payout_remain_flag;	/* 残留通知I/F通知フラグ */
EXTERN bool ex_rs_payout_disp_on; //UBA700RTQオリジナル 2025-06-26


//#if defined(ID003_SENSOR)
//not use EXTERN u8 ex_position_dirt_head_check;	//使用している様で実際はフラグセットしても何もしていない
//not use  EXTERN u8 ex_position_dirt_rc_check;		//使用している様で実際はフラグセットしても何もしていない
EXTERN u8 ex_position_dirt[4];
//not use EXTERN u8 ex_rc_position_current_da[16];
//not use EXTERN u8 ex_rc_position_shipment_da[16];
//#endif

EXTERN u8 ex_rc_timeout;	//2025-06-20 //UBA700RTQで新規作成
EXTERN bool ex_rc_timeout_error;	//2025-07-22 //UBA700RTQで新規作成

#endif /* UBA_RTQ */

#if 1	/* '22-08-23 */	//暗号化強化でシークレット番号がランダムになった //UBA_ID003_ENC
EXTERN	u8	Random_Secret_no[3];
EXTERN	u32 random_seed;
EXTERN	u32 jdl_logdat_chksum;
EXTERN u8 ex_main_secret_number_change_flag;
#endif


#endif /* SRC_MAIN_INCLUDE_COM_RAM_H_ */
/* EOF */
