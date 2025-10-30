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

typedef struct {
	u8 bif_section1_crc_result;
	u8 bif_section2_crc_result;
	u8 rom_crc_result;
}ROM_SECTION_STATUS;

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
typedef struct RESBUFF_STRUCTURE {
	u8 sync;
	u16 dlen;
	u8 cmd;
	u8 sst1;
	u8 sst2;
	u8 sst3;
	u8 sst4;
	u8 unit_sst;
	u16 err_code;
	u8 data[2048];
} _cline_resbuff;


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
	/* reserve */				/* [051]~[063] */
} __attribute__((packed)) CLINE_STATUS_TBL;
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

#define SERIAL_NUMBER_SIZE 12

/*----------------------------------------------------------------------*/
/* LED(IOEX)                                                            */
/*----------------------------------------------------------------------*/
typedef struct _LED
{
	u8 red;
	u8 green;
	u8 blue;
	u8 bezel;
} LED;

/*----------------------------------------------------------------------*/
/* Machine Setting                                                      */
/*----------------------------------------------------------------------*/
typedef struct _Settings{
	u8 bar_enable;
	u8 cheat_detect_enable;
	u8 partial_precomp_enable;
	u8 ld_mode;
} Settings;
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




/************************************************************/
/*															*/
/*	時計構造体													*/
/*															*/
/************************************************************/
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


#if defined(_PROTOCOL_ENABLE_ID0G8)
typedef struct __MUSB_PKT_INFO_0G8 {
	u16 length_info;
	u16 offset_r;
	u16 offset_w;
	u16	remain_data;
	u32 write_size;

//#if defined(ID0G8_NEW_COM)
	u8	current_cmd_count;
//#endif

}MUSB_PKT_INFO_0G8;
#endif





#endif

