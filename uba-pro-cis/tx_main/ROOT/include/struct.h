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
 * @file struct.h
 * @brief 共通構造体定義
 * @date 2020/03/11 Created.
 */
/****************************************************************************/
#ifndef __STRUCT_H_INCLUDED__
#define __STRUCT_H_INCLUDED__

/**** STRUCTURES ********************************************************************************/
#pragma push
#pragma pack(1)



/*----------------------------------------------------------*/
/*			Section Header										*/
/*----------------------------------------------------------*/
typedef struct {
	u8 fileheader[8];			/* Section Header						*/
	u32 crc32;					/* CRC-32							*/
	u16 crc16;					/* CRC-16		 					*/
	u16 sec_no;					/* section no						*/
	u32 startadr;				/* Program start address			*/
	u32 endadr;					/*    	  end address				*/
	u32 sec_size;				/* Program size						*/
	u8 sec_name[4];				/* section name						*/
	u32 dummy3;					/* reserve							*/
	u32 dummy4;					/* reserve							*/
}BA_SectionHeader;


/************************************************************/
/*															*/
/*	ダウンロードチェック用構造体							*/
/*															*/
/************************************************************/
typedef struct t_dl_section_info{
    u32   start_adr;   /* Section開始アドレス               */
    u32   end_adr;     /* Section終了アドレス               */
    u32   size;        /* セクションサイズ                  */
    u32   rcv_size;    /* 受信サイズ                        */
                       /* (Base毎のダウンロード用)          */
    u32   base_count;
    u8    sec_count;
}T_DL_SECTION_INFO;
typedef struct t_dl_chk_flg{
    u8   ChkSignature;                /* Signatureチェックフラグ           */
    u8   ChkRomHeader;                /* ROMヘッダチェックフラグ           */
    u8   ChkSectionHeader;            /* Versionチェックフラグ             */
    u8   ChkBifSection;               /* BIFエリアチェックフラグ */
    //u8   ChkBaseHeader;               /* BaseデータのHeaderチェックフラグ  */
}T_DL_CHK_FLG;


#define D_SIG_CHECK_DO_NOT            0 /* Not Done               */
#define D_SIG_CHECK_OK                1 /* Done                   */
#define D_SIG_CHECK_NG                2 /* Error                  */

#define D_SEC_CHECK_DO_NOT            0 /* チェック未実施                         */
#define D_SEC_CHECK_OK                1 /* チェック結果OK                         */
#define D_SEC_CHECK_WAIT_WRITE        2 /* セクション書き込み待ち                 */

#define D_ROM_CHECK_DO_NOT            0 /* チェック未実施         */
#define D_ROM_CHECK_WAIT_WRITE        1
#define D_ROM_CHECK_OK                2 /* 書き込み完了結果OK     */
#define D_ROM_CHECK_ID_MISMATCH       3 /* チェック結果ID不一致   */
#define D_ROM_CHECK_FIRST_SECTION     4 /* 第1セクションチェック待ち   */

#define D_BASE_CHECK_DO_NOT           0 /* チェック未実施         */
#define D_BASE_CHECK_WAIT_WRITE       1 /* Header書き込み待ち     */
#define D_BASE_CHECK_OK               2 /* チェック結果OK         */
#define D_BASE_CHECK_NOT_BASE         3

#define D_BIF_CHECK_DO_NOT            0 /* チェック未実施         */
#define D_BIF_CHECK_OK				  1 /* チェック結果OK         */
#define D_BIF_CHECK_WAIT_WRITE_BIF1   2 /* BIF書き込み待ち(BIF1CRCエラーの為BIF1を先に書く)     */
#define D_BIF_CHECK_WAIT_WRITE_BIF2   3 /* BIF書き込み待ち(BIF2CRCエラーの為BIF2を先に書く)     */


typedef struct {
	u8  is_start_received;		/* Download start command received  */
	u8  busy;					/* QSPI writing                     */
	u8  is_end_sent;			/* QSPI writing                     */
	u16 program_crc;            /* Download Data promgram area crc  */
	u16 file_crc;               /* Download Data file crc           */
	u32 baudrate;               /* UART baudrate                    */
}CLINE_DOWNLOAD_CONTROL;


/* UART受信バッファ設定 */
#define APL_UARTBUF_SIZE                    (65536 + 12)	// 12Byte(STX～SST5,BCC,ETX) + 64KByte(Data Max) = 65548Byte
#define APL_UARTBUF_NUM                     (1)			// MRX-CISはコマンドが複数同時に来ることはないので1つのバッファとする

/* UART受信バッファ構造体 */
typedef struct uart_drv_buf_tag {
	unsigned char aucBuffer[APL_UARTBUF_SIZE];
	unsigned long ulBufPtr;
	unsigned long ulUseFlag;
	int nlistenCode;
} UART_DRV_BUF;

/****************************************************************/
/**
 * @struct RESBUFF_STRUCTURE
 * @brief レスポンスバッファの構造体
 */
/****************************************************************/
typedef struct CMDBUFF_STRUCTURE {
	u8 sync;
	u16 dlen;
	u8 cmd;
	u8 data[2048];
} _cline_cmdbuff;

/****************************************************************/
/**
 * @struct RESBUFF_STRUCTURE
 * @brief レスポンスバッファの構造体
 */
/****************************************************************/
#if defined(_PROTOCOL_ENABLE_ID003)
typedef struct _CLINE_STATUS_TBL
{
	u8 protocol_select;			/* [000]       */
	u8 if_select;				/* [001]       */
	u16 line_task_mode;			/* [002]~[003] */
	u16 status;					/* [004]~[005] */
	u16 escrow_code;			/* [006]~[007] */
	u16 reject_code;			/* [008]~[009] */
	u16 error_code;				/* [010]~[011] */
	u16 dipsw_disable;			/* [012]~[013] */
	u32 bill_disable;			/* [014]~[017] */
	u32 bill_disable_mask;		/* [018]~[021] */
	u32 bill_escrow;			/* [022]~[025] */
	u32 security_level;			/* [026]~[029] */
	u16 comm_mode;				/* [030]~[031] */
	u16 accept_disable;			/* [032]~[033] */
	u16 direction_disable;		/* [034]~[035] */
	u16 option;					/* [036]~[037] */
	u8 barcode_type;			/* [038]       */
	u8 barcode_length;			/* [039]       */
	u8 barcode_inhibit;			/* [040]       */
	u32 comm_addr;				/* [041]~[044] */
	u8 encryption;				/* [045]       */
	u8 log_access_mode;			/* [046]       */
	u8 log_access_status;		/* [047]       */
	u16 store_task_mode;		/* [048]~[049] */
	u8 country_setting;			/* [050]       */

	u8 ex_Barcode_recovery_icb[28];	/* [051]~[078] */	// リカバリで使用,Escrowに対するAck未受信のパワーリカバリ
	u8 id003_escrow_payout;		//パワーリカバリでも使用するはず //2025-01-21 いるかも
	u8 ex_rc_after_jam;
	u8 ex_rc_option_battery_low_detect;
	//use 82byte
	//now max 92byte
	//reserve 10byte
	/* reserve */				/* [051]~[063] */
} __attribute__((packed)) CLINE_STATUS_TBL;	//use backup RECOVERY_INFO_OFFSET でサイズを定義しているので、増やさない事
#endif

typedef struct _RECOVERY_INFO
{
	u8  step;				//use UBA700, UBA500 use
	u16  back_fwd_pulse; 	//use UBA700, UBA500 use
//#if defined(UBA_RTQ)
	u8 unit;       /* 入金先、出金元(動作に使用する) *///ソフトダウンロードで保持しなくてもよい //UBA500も同じ構造体
	u8 unit_count; /* カウントに使用する *///ソフトダウンロードで保持しなくてもよい //UBA500も同じ構造体
	u8 count;      /* RC保有枚数。入出金前後でRC保有枚数を比較するため保持 */ //UBA500も同じ構造体
	u8 stack_mode;	//リカバリではない、通常処理のVend2の処理早い版 UBA_MUSTUBA＿MUST // UBA500も同じ構造体
//#endif
} __attribute__((packed)) RECOVERY_INFO; //use backup // bkex_status_tbl_buff にコピーしてFRAMへ読み書きしているので、サイズを変えないこと

//CLINE_STATUS_TBL と RECOVERY_INFO を合わせて bkex_status_tbl_buff にコピーしてFRAMへ読み書きしているので、サイズを変えないこと

#pragma pop


/********************************************************************
* mail box message
********************************************************************/
typedef struct _T_MSG_BASIC
{
	struct T_MSG	*next;			/* pointer to next message */
	u32				sender_id;		/* task id of sender */
	u32				mpf_id;			/* fixed-sided memory pool id */
	u32				tmsg_code;		/* task message code */
	u32				arg1;			/* argument 1 */
	u32				arg2;			/* argument 2 */
	u32				arg3;			/* argument 3 */
	u32				arg4;			/* argument 4 */
} T_MSG_BASIC;


typedef struct __DLINE_PKT_INFO {
	u16 total_len;
	u16 offset_r;
	u32 offset_w;
	u8	cnt;
}DLINE_PKT_INFO;

typedef struct _USB_COMMAND_SEQUENCE
{
	u8 command;
	u8 enq_count;
	u16 seq_sent;
	u16 seq_received;
} USB_COMMAND_SEQUENCE;


typedef	struct
{
	u8	serviceID;
	u16	length;
	u8	modeID;
	u8	phase;
	u8	command;
#if defined(UBA_RTQ)
	u8 data[28];
#endif // UBA_RTQ
} T_FUSB_MESSAGE;

typedef	struct
{
	u16	mode;
	u16	status;
	T_FUSB_MESSAGE	mess;
} T_PC_COMMAND;
typedef struct	_front_usb
{
	u16				status;
	u16				mode;
	T_PC_COMMAND	pc;
	u8				option_data[8];
}T_FRONT_USB;
typedef struct _suite_item
{
	u8		curent_service;
	u16		service_list;
} T_SUITE_ITEM;


typedef struct _ST_FEED_MOTOR_CTRL
{
	u16		drive_pulse;			/* drive pulse count */
	u16		pulse;					/* forward pulse count */
//	u16		event_pulse;			/* event pulse *///2025-09-24 廃止
	u16		lock_count;				/* motor lock count */
	u16		run_time;				/* running time */
	u16		stop_time;				/* motor stop time */
	u16		speed;					/* motor speed */
	u16		pwm;					/* fix pwm value */
	u16		reject;					/* reject flag */
	u16		mode;					/* */
	u16		max_current;			/* max current limit */
	u8		over_pulse;				/* 規定パルス以上進んでいる */
	u16		speed_check_pulse;		//2025-09-24
	u16		speed_check_time;		//2025-09-24
} ST_FEED_MOTOR_CTRL;

typedef struct _ST_STACKER_MOTOR_CTRL
{
	u16		drive_pulse;			/* drive pulse count */
	u16		pulse;					/* forward pulse count */
	u16		lock_count;				/* motor lock count */// use
	u16		run_time;				/* running time */// use
	u16		stop_time;				/* motor stop time */// use
	u8		full_check;				/* stacker full check flag*///use
	u8		peakload_flag;			/* stacker peak-load area check flag (stacker full) *///use
	u16		peakload_time;			/* stacker peak-load area time (stacker full) *///use
	u8		speed;					/* motor speed *///not use
	u8		mode;					/* */
	u8		wait_pulse_count;		/* Home outを抜けてからのパルス数*///use
	u16		event_pulse_up;			/* テスト的に追加 */
	u16		event_pulse_down;		/* テスト的に追加 */
	u16		event_pulse;			/* event pulse *///use
	u16		init_flag;				/* event initial flag */// UBA_WS
	u16		init_value;				/* event initial value *///	UBA_WS 


} ST_STACKER_MOTOR_CTRL;

typedef struct _ST_MOTOR_HOME_POSITION_CTRL
{
	u16		drive_home_cnt;
	u16		home_cnt;				/* motor stop pulse */
	u16		open_cnt;				/* motor stop pulse */
	u16		close_cnt;				/* motor stop pulse */
	u16		event_time;				/* event time */
	u16		prev_time;				/* previous time */
	u16		run_time;				/* running time */
	u16		stop_time;				/* motor stop time */
	u8		speed;					/* motor speed */
	u8		mode;					/* */
	u8		status;
	u8		max_current;			/* max current limit */
} ST_MOTOR_HOME_POSITION_CTRL;


typedef struct _ST_PB_MOTOR_CTRL
{
	u8		drive_home_cnt;			
	u16		drive_pulse;			/* drive pulse count */
	u8		home_cnt;				/* motor stop pulse */
	u16		pulse;					/* forward pulse count */
	u16		event_pulse;			/* event pulse */
	u16		lock_count;				/* motor lock count */
	u16		event_time;				/* event time */
	u16		prev_time;				/* previous time */
	u16		run_time;				/* running time */
	u16		stop_time;				/* motor stop time */
//	u8		speed;					/* motor speed */
	u8		mode;					/* */
	u8		status;
	u8		max_current;			/* max current limit */
	u8		pwm;					/* fix pwm value */	
	/* 2022-10-03 */
	u16		init_flag;				/* event initial flag */
	u16		init_value;				/* event initial value */

} ST_PB_MOTOR_CTRL;


typedef struct _ST_SHUTTER_MOTOR_CTRL
{
	u8		drive_home_cnt;			
	u16		drive_pulse;			/* drive pulse count */
	u8		home_cnt;				/* motor stop pulse */
	u16		pulse;					/* forward pulse count */
	u16		event_pulse;			/* event pulse */
	u16		lock_count;				/* motor lock count */
	u16		event_time;				/* event time */
	u16		prev_time;				/* previous time */
	u16		run_time;				/* running time */
	u16		stop_time;				/* motor stop time */
//	u8		speed;					/* motor speed */
	u8		mode;					/* */
	u8		status;
	u8		max_current;			/* max current limit */
	u8		pwm;					/* fix pwm value */

} ST_SHUTTER_MOTOR_CTRL;


/********************************************************************
* position sensor Variables
********************************************************************/
typedef	struct _AREA
{
    u16 start;
    u16 end;
} AREA;
typedef struct _POSITION_AREA
{
	AREA entrance;
	AREA centering;
	AREA apb_in;
	AREA apb_out;
	AREA exit;
} POSITION_AREA;

/********************************************************************
* Varidation Struct
********************************************************************/
typedef union _FITNESS_STRUCT
{
	u32 all;
	struct
	{
		u32 result		: 8;	//結果 	(0:ATMフィット／1:フィット／2:アンフィット)
		u32 level		: 8;	//レベル(0:無し／1～100:設定値)
		u32 threshold_1	: 8;	//閾値	(ATMフィットとフィット間)
		u32 threshold_2	: 8;	//閾値	(フィットとアンフィット間)
	} bit;
} FITNESS_STRUCT, *PFITNESS_STRUCT;

typedef struct
{
	u8		Ref_or_Trans;		//透過か反射か 0反射　1透過
	u8		note_scan_side;		//識別前の面 0:上CISで受像 1:下で受像
	u8		Enable_or_Disable;	//0有効：1無効
	u8      note_scan_dir;		//スキャン方向 0正方　1逆
	float	main_element_pitch;	//素子ピッチ
	u16		main_effective_range_min;	//主走査方向有効画素範囲開始位置
	u16		main_effective_range_max;	//主走査方向有効画素範囲終了位置
	s16		main_offset;		//搬送路センターの画素番号を設定
	u8		sub_sampling_pitch;	//副走査方向のサンプリングピッチ
	s8  	sub_offset;			//サブブロック単位のドット数
	u8		data_type;			//データ長
	u8		planeblank1;		//ブランク
	u16		main_all_pix;		//このプレーンの全素子数
	u8		planeblank2[4];		//ブランク4バイト
	u32		Address_Period;		//そのセンサーデータのPeriod
	u32		Address_Offset;		//そのセンサーデータのオフセット

} ST_PLANE_INFOMATION_STRUCT;

//識鑑別処理、中間情報記録用の構造体****
//外形検知
typedef struct
{
	u32	proc_time;	//処理時間
	u8	reserve[60];

} ST_MID_RES_OUTLINE_STRUCT;

//識別NN
typedef struct
{
	u32	proc_time;			//処理時間
	float output_max_val;	//最大発火値
	float output_2nd_val;	//第2発火値
	float softmax_val;		//ソフトマックス値
	float error_val;		//エラー値
	u16   max_node_num;		//最大発火のノード番号
	u16   _2nd_node_num;	//最大発火のノード番号
	u32	  result_jcm_id;	//識別結果のJCM-ID
	u8	reserve[36];

} ST_MID_RES_NN_IDENT_STRUCT;


//角折れ
typedef struct
{
	u32	proc_time;			//処理時間
	u16	long_side_left_up;	//長辺
	u16	short_side_left_up;	//短辺
	u32	area_left_up;		//面積

	u16	long_side_left_down;//長辺
	u16	short_side_left_down;//短辺
	u32	area_left_down;		//面積

	u16	long_side_right_up;	//長辺
	u16	short_side_right_up;//短辺
	u32	area_right_up;		//面積

	u16	long_side_right_down;//長辺
	u16	short_side_right_down;//短辺
	u32	area_right_down;	//面積

	u8	judge_left_up;		//判定結果
	u8	judge_left_down;	//
	u8	judge_right_up;		//
	u8	judge_right_down;	//

	u8	reserve[24];

} ST_MID_RES_DOR_EAR_STRUCT;

//裂け
typedef struct
{
	u32	proc_time;			//処理時間
	float width;			//幅
	float depth;			//深さ
	u8	type;				//形状
	u8	judge;				//判定結果
	u8	reserve[50];

} ST_MID_RES_TEAR_STRUCT;

//ダイノート
typedef struct
{
	u32	proc_time;	//処理時間
	u32	area;		//面積
	float raito;	//割合
	u8 judge;		//判定結果
	u8	reserve[51];

} ST_MID_RES_DYE_NOTE_STRUCT;

//汚れ
typedef struct
{
	u32	proc_time;	//処理時間
	float plane_distance[5];	//平面との距離
	u8 result;
	u8	reserve[39];

} ST_MID_RES_SOILING_STRUCT;

//脱色
typedef struct
{
	u32	proc_time;	//処理時間
	float plane_distance[5];	//平面との距離
	u8 result;
	u8	reserve[39];

} ST_MID_RES_DE_INKD_STRUCT;

//染み
typedef struct
{
	u32	proc_time;	//処理時間
	float area;		//面積
	float diameter;	//直径
	float total_area;//合計面積
	u16   err_code;	//エラーコード
	u8    judge;	//判定結果
	u8	reserve[45];

} ST_MID_RES_STAIN_STRUCT;

//3色比　4色比　IRチェック　MCIR
typedef struct
{
	u32	proc_time;	//処理時間
	u16 invalid_count;	//インバリカウント
	u8	level;
	u8  result;
	u8	reserve[56];

} ST_MID_RES_CIR_SERIES_STRUCT;


//重券
typedef struct
{
	u32	proc_time;	//処理時間
	s32	invalid_count;	//インバリカウント
	u8  result;
	u8	reserve[55];

} ST_MID_RES_DOUBLE_CHECK_STRUCT;

//NN1　NN2
typedef struct
{
	u32	proc_time;	//処理時間
	float genuine_out_put_val;		//本物発火値
	float counterfeit_out_put_val;	//偽物発火値
	u8	result;
	u8	calc_res_level;
	u8	thr_level;
	u8	reserve[49];

} ST_MID_RES_VALI_NN_STRUCT;


/********************************************************************
* CIS Sensor Data
********************************************************************/
#define SUB_ADJ_LINE						64//<-32
#define HDRTBL_SIZE			8
#define HDRTBL_SIZE_10BIT	4
#define SCAN_BLOCK_SIZE		(424 + 1)
#define MAIN_SCAN_LINE		720
#define MAG_SCAN_LINE		111
#define MAG_LR_SIZE			2
#define CHECK_VALIDATION_SUB_LINE		5

typedef struct _TMP_AD_TBL{
	/* outside */
	u8	cis_red_ref_u[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 赤 上
	u8	cis_gre_ref_u[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 緑 上
	u8 cis_blu_ref_u[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 青 上
	u8	cis_ir1_ref_u[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 IR1(810) 上
	u8	cis_ir2_ref_u[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 IR2(940) 上
	u8 cis_fl_ref_u[HDRTBL_SIZE + MAIN_SCAN_LINE];		// 反射 UV 上
	u8	c6_led_u[HDRTBL_SIZE + MAIN_SCAN_LINE];
	u8	c7_led_u[HDRTBL_SIZE + MAIN_SCAN_LINE];
	u8	c8_led_u[HDRTBL_SIZE + MAIN_SCAN_LINE];
	u8 c9_led_u[HDRTBL_SIZE + MAIN_SCAN_LINE];
	/* inside */
	u8	cis_red_ref_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 赤 下
	u8	cis_gre_ref_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 緑 下
	u8 cis_blu_ref_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 青 下
	u8	cis_ir1_ref_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 IR1(810) 下
	u8	cis_ir2_ref_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 反射 IR2(940) 下
	u8 cis_fl_ref_d[HDRTBL_SIZE + MAIN_SCAN_LINE];		// 反射 UV 下
	u8	cis_red_pen_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 透過 赤 上
	u8	cis_gre_pen_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 透過 緑 上
	u8	cis_ir1_pen_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 透過 IR1(810) 上
	u8	cis_ir2_pen_d[HDRTBL_SIZE + MAIN_SCAN_LINE];	// 透過 IR2(940) 上
} TMP_AD_TBL;
typedef struct _ADJ_SENSOR_DATA{
	TMP_AD_TBL tmp_ad_tbl[SUB_ADJ_LINE];
} ADJ_SENSOR_DATA;

typedef struct _TMP_VALIDATION_SENSOR_TBL{
	u8 check_1st_sensor[HDRTBL_SIZE + MAIN_SCAN_LINE];
	u8 check_2ed_sensor[HDRTBL_SIZE + MAIN_SCAN_LINE];
} TMP_VALIDATION_SENSOR_TBL;
typedef struct _TMP_VALIDATION_SENSOR_DATA{
	TMP_VALIDATION_SENSOR_TBL tmp_validation_tbl[CHECK_VALIDATION_SUB_LINE];
} TMP_VALIDATION_SENSOR_DATA;

typedef struct _TMP_10BIT_AD_TBL{
	/* outside */
	u16	cis_red_ref_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 赤 上
	u16	cis_gre_ref_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 緑 上
	u16 cis_blu_ref_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 青 上
	u16	cis_ir1_ref_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 IR1(810) 上
	u16	cis_ir2_ref_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 IR2(940) 上
	u16 cis_fl_ref_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];		// 反射 UV 上
	u16 c6_led_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];
	u16 c7_led_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];
	u16 c8_led_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];			// 黒　上
	u16 c9_led_u[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];			// 黒　下
	/* inside */
	u16	cis_red_ref_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 赤 下
	u16	cis_gre_ref_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 緑 下
	u16 cis_blu_ref_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 青 下
	u16	cis_ir1_ref_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 IR1(810) 下
	u16	cis_ir2_ref_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 反射 IR2(940) 下
	u16 cis_fl_ref_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];		// 反射 UV 下
	u16	cis_red_pen_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 透過 赤 上
	u16	cis_gre_pen_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 透過 緑 上
	u16	cis_ir1_pen_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 透過 IR1(810) 上
	u16	cis_ir2_pen_d[HDRTBL_SIZE_10BIT + MAIN_SCAN_LINE];	// 透過 IR2(940) 上
} TMP_10BIT_AD_TBL;
typedef struct _ROW_SENSOR_DATA{
	TMP_10BIT_AD_TBL tmp_10bit_ad_tbl[SUB_ADJ_LINE];
} ROW_SENSOR_DATA;

#define UL_MAG_BYTE		4
#define UR_MAG_BYTE		5
typedef struct _TMP_MAG_AD_TBL{
	/* mag */
	u16 mag[HDRTBL_SIZE_10BIT + MAG_LR_SIZE + 2];	// 1:header 8byte / left 2byte / right 2byte / reserved 2byte
} TMP_MAG_AD_TBL;
typedef struct _MAG_SENSOR_DATA{
	TMP_MAG_AD_TBL tmp_mag_ad_tbl[MAG_SCAN_LINE];
} MAG_SENSOR_DATA;
typedef struct _BC_DATA {
	/* black 1 */
	u16 black_data1_u[MAIN_SCAN_LINE];
	u16 black_data1_d[MAIN_SCAN_LINE];
	/* black 2 */
	u16 black_data2_u[MAIN_SCAN_LINE];
	u16 black_data2_d[MAIN_SCAN_LINE];
} BC_DATA;
typedef struct _WC_DATA {
	/* outside */
	u16 red_ref_u[MAIN_SCAN_LINE];
	u16 gre_ref_u[MAIN_SCAN_LINE];
	u16 blu_ref_u[MAIN_SCAN_LINE];
	u16 ir1_ref_u[MAIN_SCAN_LINE];
	u16 ir2_ref_u[MAIN_SCAN_LINE];
	u16 fl_ref_u[MAIN_SCAN_LINE];
	u16 c6_led_u[MAIN_SCAN_LINE]; // not used
	u16 c7_led_u[MAIN_SCAN_LINE]; // not used
	u16 c8_led_u[MAIN_SCAN_LINE]; // reserve1
	u16 c9_led_u[MAIN_SCAN_LINE]; // reserve4
	/* inside */
	u16 red_ref_d[MAIN_SCAN_LINE];
	u16 gre_ref_d[MAIN_SCAN_LINE];
	u16 blu_ref_d[MAIN_SCAN_LINE];
	u16 ir1_ref_d[MAIN_SCAN_LINE];
	u16 ir2_ref_d[MAIN_SCAN_LINE];
	u16 fl_ref_d[MAIN_SCAN_LINE];
	u16 red_pen_d[MAIN_SCAN_LINE];
	u16 gre_pen_d[MAIN_SCAN_LINE];
	u16 ir1_pen_d[MAIN_SCAN_LINE];
	u16 ir2_pen_d[MAIN_SCAN_LINE];
} WC_DATA;

typedef union
{
#if defined(__ARMCC_VERSION)
	u32 LWORD;
	struct
	{
		u32 CH0 :10;
		u32 CH1 :10;
		u32 CH2 :10;
		u32 :2;
	} BIT;
#else
	uint32_t LWORD;
	struct
	{
		uint32_t CH0 :10;
		uint32_t CH1 :10;
		uint32_t CH2 :10;
		uint32_t :2;
	} BIT;
#endif
} CIS_SHADING_UNION;

typedef struct _CISA_BLACK_STRUCT{
	CIS_SHADING_UNION TBL0[288];//RLS_RED
	CIS_SHADING_UNION TBL1[288];//RLS_GRE
	CIS_SHADING_UNION TBL2[288];//RLS_BLU
	CIS_SHADING_UNION TBL3[288];//RLS_IR1
	CIS_SHADING_UNION TBL4[288];//RLS_IR2
	CIS_SHADING_UNION TBL5[288];//RLS_FL
	CIS_SHADING_UNION TBL6[288];//TLS_RED
	CIS_SHADING_UNION TBL7[288];//TLS_GRE
	CIS_SHADING_UNION TBL8[288];//TLS_IR1
	CIS_SHADING_UNION TBL9[288];//TLS_IR2
} CISA_BLACK_STRUCT, *PCISA_BLACK_STRUCT;
typedef struct _CISA_WHITE_STRUCT{
	CIS_SHADING_UNION TBL0[288];//RLS_RED
	CIS_SHADING_UNION TBL1[288];//RLS_GRE
	CIS_SHADING_UNION TBL2[288];//RLS_BLU
	CIS_SHADING_UNION TBL3[288];//RLS_IR1
	CIS_SHADING_UNION TBL4[288];//RLS_IR2
	CIS_SHADING_UNION TBL5[288];//RLS_FL
	CIS_SHADING_UNION TBL6[288];//TLS_RED
	CIS_SHADING_UNION TBL7[288];//TLS_GRE
	CIS_SHADING_UNION TBL8[288];//TLS_IR1
	CIS_SHADING_UNION TBL9[288];//TLS_IR2
} CISA_WHITE_STRUCT, *PCISA_WHITE_STRUCT;
typedef struct _CISB_BLACK_STRUCT{
	CIS_SHADING_UNION TBL0[288];//RLS_RED
	CIS_SHADING_UNION TBL1[288];//RLS_GRE
	CIS_SHADING_UNION TBL2[288];//RLS_BLU
	CIS_SHADING_UNION TBL3[288];//RLS_IR1
	CIS_SHADING_UNION TBL4[288];//RLS_IR2
	CIS_SHADING_UNION TBL5[288];//RLS_FL
	CIS_SHADING_UNION TBL6[288];//TLS_RED
	CIS_SHADING_UNION TBL7[288];//TLS_GRE
	CIS_SHADING_UNION TBL8[288];//TLS_IR1
	CIS_SHADING_UNION TBL9[288];//TLS_IR2
} CISB_BLACK_STRUCT, *PCISB_BLACK_STRUCT;
typedef struct _CISB_WHITE_STRUCT{
	CIS_SHADING_UNION TBL0[288];//RLS_RED
	CIS_SHADING_UNION TBL1[288];//RLS_GRE
	CIS_SHADING_UNION TBL2[288];//RLS_BLU
	CIS_SHADING_UNION TBL3[288];//RLS_IR1
	CIS_SHADING_UNION TBL4[288];//RLS_IR2
	CIS_SHADING_UNION TBL5[288];//RLS_FL
	CIS_SHADING_UNION TBL6[288];//TLS_RED
	CIS_SHADING_UNION TBL7[288];//TLS_GRE
	CIS_SHADING_UNION TBL8[288];//TLS_IR1
	CIS_SHADING_UNION TBL9[288];//TLS_IR2
} CISB_WHITE_STRUCT, *PCISB_WHITE_STRUCT;

typedef struct _CIS_ADJUSTMENT_DA{
	/* outside */
	u8 red_ref_da_u;
	u8 gre_ref_da_u;
	u8 blu_ref_da_u;
	u8 ir1_ref_da_u;
	u8 ir2_ref_da_u;
	u8 fl_ref_da_u;
	u8 red_pen_da_u;
	u8 gre_pen_da_u;
	u8 ir1_pen_da_u;
	u8 ir2_pen_da_u;
	/* inside */
	u8 red_ref_da_d;
	u8 gre_ref_da_d;
	u8 blu_ref_da_d;
	u8 ir1_ref_da_d;
	u8 ir2_ref_da_d;
	u8 fl_ref_da_d;
	u8 red_pen_da_d; // not used
	u8 gre_pen_da_d; // not used
	u8 ir1_pen_da_d; // not used
	u8 ir2_pen_da_d; // not used
} CIS_ADJUSTMENT_DA, *PCIS_ADJUSTMENT_DA;

typedef struct _CIS_ADJUSTMENT_TIME{
	/* outside */
	u16 red_ref_time_u;
	u16 gre_ref_time_u;
	u16 blu_ref_time_u; // not used
	u16 ir1_ref_time_u;
	u16 ir2_ref_time_u;
	u16 fl_ref_time_u;
	u16 red_pen_time_u;
	u16 gre_pen_time_u;
	u16 ir1_pen_time_u;
	u16 ir2_pen_time_u;
	/* inside */
	u16 red_ref_time_d;
	u16 gre_ref_time_d;
	u16 blu_ref_time_d; // not used
	u16 ir1_ref_time_d;
	u16 ir2_ref_time_d;
	u16 fl_ref_time_d; // not used
	u16 red_pen_time_d; // not used
	u16 gre_pen_time_d; // not used
	u16 ir1_pen_time_d; // not used
	u16 ir2_pen_time_d; // not used
} CIS_ADJUSTMENT_TIME, *PCIS_ADJUSTMENT_TIME;

typedef struct _CIS_ADJUSTMENT_PGA{
	/* outside */
	float red_ref_pga_u;
	float gre_ref_pga_u;
	float blu_ref_pga_u; // not used
	float ir1_ref_pga_u;
	float ir2_ref_pga_u;
	float fl_ref_pga_u;
	float red_pen_pga_u; // not used
	float gre_pen_pga_u; // not used
	float ir1_pen_pga_u; // not used
	float ir2_pen_pga_u; // not used
	/* inside */
	float red_ref_pga_d;
	float gre_ref_pga_d;
	float blu_ref_pga_d;
	float ir1_ref_pga_d;
	float ir2_ref_pga_d;
	float fl_ref_pga_d;
	float red_pen_pga_d;
	float gre_pen_pga_d;
	float ir1_pen_pga_d;
	float ir2_pen_pga_d;
} CIS_ADJUSTMENT_PGA, *PCIS_ADJUSTMENT_PGA;

enum WHITE_ERIA{
	WHITE_ERIA_MIN,
	WHITE_ERIA_MAX,
};

typedef struct _AFE_ADJUSTMENT_AGAIN{
	u8 shg0_u[4];
	u8 shg0_d[4];
} AFE_ADJUSTMENT_AGAIN, *PAFE_ADJUSTMENT_AGAIN;

typedef struct _AFE_ADJUSTMENT_AOFFSET{
	u8 offdac_u[4];
	u8 offdac_d[4];
} AFE_ADJUSTMENT_AOFFSET, *PAFE_ADJUSTMENT_AOFFSET;

typedef struct _AFE_ADJUSTMENT_DGAIN{
	u8 dgain_u[4];
	u8 dgain_d[4];
} AFE_ADJUSTMENT_DGAIN, *PAFE_ADJUSTMENT_DGAIN;

typedef struct _CIS_ADJUSTMENT_AD{
	/* outside */
	u16 red_ref_ad_u;
	u16 gre_ref_ad_u;
	u16 blu_ref_ad_u; // not used
	u16 ir1_ref_ad_u;
	u16 ir2_ref_ad_u;
	u16 fl_ref_ad_u; // not used
	u16 red_pen_ad_u; // not used
	u16 gre_pen_ad_u; // not used
	u16 ir1_pen_ad_u; // not used
	u16 ir2_pen_ad_u; // not used
	/* inside */
	u16 red_ref_ad_d;
	u16 gre_ref_ad_d;
	u16 blu_ref_ad_d; // not used
	u16 ir1_ref_ad_d;
	u16 ir2_ref_ad_d;
	u16 fl_ref_ad_d; // not used
	u16 red_pen_ad_d;
	u16 gre_pen_ad_d;
	u16 ir1_pen_ad_d;
	u16 ir2_pen_ad_d;
} CIS_ADJUSTMENT_AD, *PCIS_ADJUSTMENT_AD;
typedef struct _CIS_ADJUSTMENT_ERIA{
	u16 left_pix_u[2];
	u16 right_pix_u[2];
	u16 left_pix_d[2];
	u16 right_pix_d[2];
} CIS_ADJUSTMENT_ERIA, *PCIS_ADJUSTMENT_ERIA;

typedef struct _CIS_ADJUSTMENT_TMP{
	CIS_ADJUSTMENT_PGA		cis_pga;
	CIS_ADJUSTMENT_ERIA		cis_tmp_eria;
	CIS_ADJUSTMENT_AD		cis_tmp_ad;
	CIS_ADJUSTMENT_DA		cis_tmp_da;
	CIS_ADJUSTMENT_DA		cis_tmp_sled;
	CIS_ADJUSTMENT_TIME		cis_tmp_time;
} CIS_ADJUSTMENT_TMP, *PCIS_ADJUSTMENT_TMP;

typedef struct _POS_ADJUSTMENT_TMP{
	u8 tmp_entrance;
	u8 tmp_centering;
	u8 tmp_apb_in;
	u8 tmp_apb_out;
	u8 tmp_exit;
	u8 tmp_ga;
	u16 tmp_sum;
} POS_ADJUSTMENT_TMP, *PPOS_ADJUSTMENT_TMP;

typedef struct _DATA_SERIAL{
	u8 date[8];
	u8 pc;
	u8 factory;
	u8 model;
	u8 type;
	u8 unique_number[4];
} DATA_SERIAL, *PDATA_SERIAL;

#define SERIAL_NUMBER_SIZE 12
typedef struct _ADJUSTMENT_INFO{
	u8 id[6];
	u8 version[2];
	u8 date[8];
	u8 serial_no[SERIAL_NUMBER_SIZE];
} ADJUSTMENT_INFO, *PADJUSTMENT_INFO;
typedef struct _ADJUSTMENT_VALUE{
	u8 pos_entrance_da;
	u8 pos_centering_da;
	u8 pos_apb_in_da;
	u8 pos_apb_out_da;
	u8 pos_exit_da;
	u8 pos_gain;
	u8 reserved[2];
} ADJUSTMENT_VALUE, *PADJUSTMENT_VALUE;
typedef struct _MAG_SENSOR_VAL
{
	u16 ul_gain; //使用は1byte
	u16 ur_gain; //使用は1byte

	u16 ul_adj_max; //使用は1byte
	u16 ur_adj_max; //使用は1byte
} MAG_SENSOR_VAL;

typedef struct _ADJUSTMENT_DATA{
	DATA_SERIAL	data_serial; //初期流動番号
	ADJUSTMENT_INFO factory_info;
	ADJUSTMENT_INFO maintenance_info;
	ADJUSTMENT_VALUE factory_value;
	ADJUSTMENT_VALUE maintenance_value;
	MAG_SENSOR_VAL mag_adj_value;
	u8 reserved[32];
} ADJUSTMENT_DATA, *PADJUSTMENT_DATA;

typedef struct _UV_SENSOR_ADJ
{
	u16 gain[2];
	u16 da[2];
	u16 offset[2];
} UV_SENSOR_ADJ;

typedef struct _CIS_ADJUSTMENT_DATA{
	BC_DATA		cis_bc;
	WC_DATA		cis_wc;

	CIS_ADJUSTMENT_DA		cis_da;
	CIS_ADJUSTMENT_DA		cis_sled;
	CIS_ADJUSTMENT_TIME		cis_time;
	AFE_ADJUSTMENT_AGAIN	afe_again;
	AFE_ADJUSTMENT_DGAIN	afe_dgain;
	AFE_ADJUSTMENT_AOFFSET	afe_aoffset;

	UV_SENSOR_ADJ 			point_uv_adj; //UV Variables
} CIS_ADJUSTMENT_DATA, *PCIS_ADJUSTMENT_DATA;


/*----------------------------------------------------------------------*/
/* MAG sensor variables										'19-02-26	*/
/*----------------------------------------------------------------------*/
#define MAG_GAIN_DEFAULT				128		/* 60 times */
#define MAG_GAIN_MAX					0xFF	/*  */
#define MAG_GAIN_MIN					0x00	/*  */
#define MAG_ADJ_RV						217		/* 2.8V *//* reference value for MAG AD */
#define MAG_AD_RV						127		/* 1.65V */
#define MAG_CALC_NOIZE_CUT_POINT		5		/* 5 point */

#define MAG_AD_ERROR					MAG_AD_RV		/* 磁気あり基板で磁気なしの実機に組み込まれた時の対策 *//* MAGがついていないでも74ぐらいになる時もある*/

/* Temperature Adjustment */
typedef struct _MAG_SENS_TEMP_ADJ_INFO
{
	u8 end_flag;
	u16 count;
	u8 backup_gain;
	u8 last_gain;
	u16 last_ad;
} MAG_SENS_TEMP_ADJ_INFO;
enum{
	UL_MAG,
	UR_MAG
};


/********************************************************************
* feed motor speed contrl
********************************************************************/
typedef struct _FEED_SPEED_CTRL
{
	u16		set_speed;				/* set speed */
	u16		limit_speed;			/* limit speed */
	u16		measuring_speed;		/* measuring speed */
	u8		set_flag;				/* set flag */
} FEED_SPEED_CTRL;

/********************************************************************
* feed motor pulse measure
********************************************************************/
typedef	struct _FEED_PULSE_COUNT
{
    u16 total;
    u16 fwd;
    u16 rev;
} FEED_PULSE_COUNT;

/********************************************************************
* stacker motor speed contrl
********************************************************************/
typedef struct _STACKER_SPEED_CTRL
{
	u16		set_speed;				/* set speed */
	u16		limit_speed;			/* limit speed */
	u16		measuring_speed;		/* measuring speed */
	u8		set_flag;				/* set flag */
} STACKER_SPEED_CTRL;

/********************************************************************
* position sensor Variables
********************************************************************/
//センサ調整Toolをivizion2と併用している為、下記のサイズを変えるととTool側が動作しなくなる可能性がある
//その為、使用していない変数も削除禁止
typedef struct _POSITION_SENSOR
{
	u8 entrance;
	u8 centering;
	u8 apb_in;
	u8 apb_out;
	u8 exit;

	//2023-06-02 Dont Delete, Dont Add
	u8 box_home;	//not use but Dont delete for sensor adjustmet Tool
	u8 box_nfull;	//not use but Dont delete
	u8 box_12;		//not use but Dont delete
	u8 threshold;	//not use but Dont delete DA1-5はポジションではなくUVなので使用しない

	u8 ent_threshold; 	/* 入口閾値設定値 */
	u8 ext_threshold;	/* その他閾値設置値 */
} POSITION_SENSOR;


/************************************************************/
/*															*/
/*	温度チェック用構造体											*/
/*															*/
/************************************************************/
typedef struct _TMP_SENSOR_AD
{
	u16 BIN;
	s16 CEL;
} TMP_SENSOR_AD;

/*----------------------------------------------------------------------*/
/* CIS Adjustment											         	*/
/*----------------------------------------------------------------------*/
typedef struct _CIS_ADJUSTMENT_TBL{
	u8 sequence;
	u8 scan_count;
	u8 data_count;
	u8 weit;
	u8 busy;
	u8 reseved[2];
} CIS_ADJUSTMENT_TBL, *PCIS_ADJUSTMENT_TBL;

/*----------------------------------------------------------------------*/
/* Validate a banknote                                                  */
/*----------------------------------------------------------------------*/
typedef struct _BAR_RES_MEMORY
{
	int barcode_1d_character_length;
	float highest_value[450];
	float second_highest_value[450];
	int barcode_1d_characters[450];
	float output_layerinput[28][10];

	int barcode_2d_error_correction;
	int barcode_2d_character_length;
	int barcode_2d_characters[450];

} BAR_RES_MEMORY, *PBAR_RES_MEMORY;
typedef struct _ST_UV_WAVE_STRUCT
{
	u8 enable[2];
	u16 wave_length[2];
	u8 uv_wave[2][424];
} ST_UV_WAVE_STRUCT, *PST_UV_WAVE_STRUCT;
typedef struct _BV_MEMORY
{
	u16 denomi;
	u16 direction;
	u16 reject_code;
	u16 bill_length;
	u16 pulse_count;
	u16 dammmy[1];

	u32 compare_flag;
	u32 fitness_flag;
	u32 block_compare_flag;
	u8  start;
	u8 dammmy2[3];

	BAR_RES_MEMORY	barcode_result;

	ST_UV_WAVE_STRUCT uv_wave;

	u16 mag_enable[2];
	u8 ul_mag[450];
	u8 ur_mag[450];
	u16 ul_mag_amount[5];
	u16 ur_mag_amount[5];
	u16 ul_mag_reult[5];
	u16 ur_mag_reult[5];
} BV_MEMORY, *PBV_MEMORY;

/*----------------------------------------------------------------------*/
/* Data Collection                                                      */
/*----------------------------------------------------------------------*/
typedef struct _DATA_COLLECTION_INFO{
	u8 enable;		/* mode */
	u8 data_exist;	/* data flag*/
	u8 data_result;	/* data check flag*/
} DATA_COLLECTION_INFO;

typedef struct _MONITOR_INFO{
//	u8 send_status;		/* data flag*/
	u8 data_exist;		/* data flag*/
} MONITOR_INFO;


/*----------------------------------------------------------------------*/
/* USB Suite                                                            */
/*----------------------------------------------------------------------*/
struct _self_check
{
    bool stacker_unit_initial;
    bool head_unit_initial;
    bool global_variable_initial;
    bool external_rom_check;
};
/*<<	USB communication buffer	>>*/
typedef struct _dline_testmode{
	u8	phase;
	int	request;
	u8	status[64];
} dline_testmode;
typedef struct _sw_testmode{
	u8	phase;
	int	request;
} sw_testmode;


enum TEST_RESULT
{
	TEST_RESULT_NG				= 0x01,
	TEST_RESULT_OK				= 0x10,
	TEST_NOT_YET				= 0xFF,
};

typedef struct _testmode{
	u8	action;				/*	testmode control status		*/
	u8	option;				/*	testmode option */
	u16	test_no;
	u32	test_temp1;
	u32	test_temp2;
	struct _sw_testmode	sw;
	struct _dline_testmode	dline;

	u8	test_start;
	u8	test_result;
	u16	time1;
	u16	time2;
	u16	time_count;

} testmode;


typedef struct
{
	u32 year;
	u32 month;
	u32 day;
	u32 hour;
	u32 minute;
	u32 second;
	u32 week;
} RTC_INFO;
/*----------------------------------------------------------------------*/
/* Machine Setting                                                      */
/*----------------------------------------------------------------------*/
enum HAL_STATUS
{
	HAL_STATUS_UNK = 0,
	HAL_STATUS_OK,
	HAL_STATUS_NG
};
typedef	struct _HAL_STATUS_TABLE{
	// i2c
	enum HAL_STATUS i2c0;
	enum HAL_STATUS i2c3;
	// usb
	enum HAL_STATUS usb0;
	enum HAL_STATUS usb1;
	// fram(spi)
	enum HAL_STATUS fram;
	// motor
	enum HAL_STATUS	hmot;
	enum HAL_STATUS	smot;
	enum HAL_STATUS	cmot;
	// eeprom
	enum HAL_STATUS eeprom;
	// dipsw
	enum HAL_STATUS dipsw1;
	enum HAL_STATUS dipsw2;
	// bezel
	enum HAL_STATUS bezel;
	// led
	enum HAL_STATUS led_red;
	enum HAL_STATUS led_green;
	enum HAL_STATUS led_blue;
	// side adc(i2c)
	enum HAL_STATUS	side_adc;
	// power voltage adc(i2c)
	enum HAL_STATUS	power_adc;
	// tmp ic(i2c)
	enum HAL_STATUS	tmp_cisa;
	enum HAL_STATUS	tmp_cisb;
	enum HAL_STATUS	tmp_out;
	enum HAL_STATUS	tmp_hmot;
	enum HAL_STATUS	tmp_smot;
	// cis
	enum HAL_STATUS cisa;
	enum HAL_STATUS cisb;
	// rfid
	enum HAL_STATUS rfid;
} HAL_STATUS_TABLE;

/*----------------------------------------------------------------------*/
/* System Option		                                                */
/*----------------------------------------------------------------------*/
typedef struct _SYSTEM_OPTION{
	bool rfid_unit;
	bool rfid_tag;
	bool ss_unit;
	bool su_unit;
	bool sh_unit;
} SYSTEM_OPTION;
/*----------------------------------------------------------------------*/
/* RFID UNIT                                                      */
/*----------------------------------------------------------------------*/
enum RFID_VERSION
{
	RFID_VERSION_UNK = 0,
	RFID_VERSION_NRW3 = 3,
	RFID_VERSION_NRW4,
	RFID_VERSION_NRW5
};
typedef struct _RFID_UNIT{
	u16 usr_ver;
	u16 mkr_ver;
} RFID_UNIT;
/*----------------------------------------------------------------------*/
/* RFID INFORMATION                                                      */
/*----------------------------------------------------------------------*/
typedef struct _RFID_INFO{
	u8 dsfid;
	u8 uid[8];
	u8 afi;
	u8 blocknum;
	u8 blocksize;
	u8 icref;
} RFID_INFO;

/*----------------------------------------------------------------------*/
/* ICB BACKUP DATA                                                      */
/*----------------------------------------------------------------------*/
typedef struct _ICB_BACKUP{
	u8 enable_key[8];
	u8 machine_no[20];
	u8 box_no[20];
	u32 time;
	u8 sum;
} ICB_BACKUP;

/*----------------------------------------------------------------------*/
/* データコレクション                                                     */
/*----------------------------------------------------------------------*/
enum DATA_EXIST
{
	DATA_NONE,
	DATA_EXIST,
	DATA_REQEST,
	DATA_ERROR,
};
enum DATA_RESULT
{
	DATA_SUCCESS,
	DATA_ILLIGAL_SAMPLING,
	DATA_EDGE_ERROR,
};


/*----------------------------------------------------------------------*/
/* Authentication                                                     */
/*----------------------------------------------------------------------*/
#define	AUTHENTICATION_CUSTOMER_KEY_SIZE	4
#define	AUTHENTICATION_NUMBER_KEY_SIZE		8
#define	AUTHENTICATION_CODE_SIZE			4
typedef struct
{
	u8	seed_v1[2];
	u8	seed_v2[2];
	u8	seed_v3[2];
	u8	crc_v1[2];
	u8	crc_v2[2];
	u8	crc_v3[2];
	u16	crc_work;
} T_Authentication;

struct	_Authentication
{
	int	functionStatus;	/*	*/
	u8	customerKEY[AUTHENTICATION_CUSTOMER_KEY_SIZE];	/* Customer key(4 byte) 	*/
	u8	numberKEY[AUTHENTICATION_NUMBER_KEY_SIZE];		/* Number key(8 byte)		*/
	u8	code[AUTHENTICATION_CODE_SIZE];					/* Authentication code		*/
};



typedef struct _RC_TX_BUFFER //same UBA500
{
	u8 start_code;		/* $ : 0x24			*/
	u8 length;			/* length			*/
	u8 cmd;				/* command			*/
	u8 sst[5];			/* SST1 - SST3-2	*/
	u8 data[256];		/* data				*/
	u8 sum;				/* sum				*/
}RC_TX_BUFFER;
typedef struct _RC_RX_BUFFER //same UBA500
{
	u8 start_code;		/* $ : 0x24			*/
	u8 length;			/* length			*/
	u8 del_cmd;			/* del command		*/
	u8 cmd;				/* command			*/
#if 1//#if defined(RC_BOARD_GREEN)
	u8 sst[12];			/* SST1-1 -> SST4-2 */
#else	
	u8 sst[10];			/* SST1-1 - SST3-2B	*/
#endif	
	u8 res;				/* response			*/
	u8 data[256];		/* data				*/
	u8 sum;				/* sum				*/
}RC_RX_BUFFER;


#if defined(UBA_RTQ)

typedef struct _RC_TX_ENCRYPTION_BUFFER
{
	u8 start_code;		/* $ : 0x24			*/
	u8 length;			/* length			*/
	u8 del_cmd;			/* del command		*/
	u8 cmd;				/* command			*/
	u8 sst[5];			/* SST1 - SST3-2	*/
	u8 data[256];		/* data				*/
	u8 sum;				/* sum				*/
}RC_TX_ENCRYPTION_BUFFER;

#endif // UBA_RTQ


/*********************************/
/*			BEZEL CONTROL		 */
/*********************************/
typedef struct _BEZEL_LED_INFO{
	u32 mode;		/* mode 			*///u32 tmsg_codeの変換
	u32 count;		/* blink count 		*///u32 arg1
	s32 on_time;
	s32 off_time;
	s32 on_off_time;
	s32 wait_time;
	u16 s_bezel_count;	// 実際に点滅した回数
	u8 bezel_led_tm;
	bool bezel_on;		// LED状態
}_BEZEL_LED_INFO;

#endif

