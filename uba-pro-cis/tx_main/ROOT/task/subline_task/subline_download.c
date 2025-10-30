/******************************************************************************/
/*! @addtogroup BIF
    @file       subline_download.c
    @date       2018/01/19
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * subline_download.c
 * BIFアプリケーションのUSB受信データでダウンロード処理を行う。
 *  Created on: 2018/01/19
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "js_oswapi.h"
#include "typedefine.h"
#include "memorymap.h"
#include "hal_wdt.h"
#include "crc.h"
#include "hal_operation_usb.h"
#include "subline_usb.h"
#include "subline_download.h"
#include "subline_suite.h"

#include "alt_qspi.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define DOWNLOAD_DATA_WRITE_SIZE (64*1024)
/*==============================================================================*/
/* デバッグトレース宣言(有効にするとトレース出力あり)							*/
/*==============================================================================*/
#define	DBG_ERR()			//osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#define DBG_TRACE1(...)		//osw_printf(__VA_ARGS__)
#define DBG_TRACE2(...)		//osw_printf(__VA_ARGS__)
#if DBG_ERR_ALL_ENABLE
#ifdef DBG_ERR
#undef DBG_ERR
#define	DBG_ERR() osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#endif
#endif
/*-- StatusResponse(Ready)ﾒｯｾｰｼﾞ --*/
static const u8	ex_mes_res_status_ready[]={
	RES_ID_STATUS, /* byte0: ﾚｽﾎﾟﾝｽｺｰﾄﾞ */
	1,             /* byte1: ﾚｽﾎﾟﾝｽﾃﾞｰﾀｻｲｽﾞ */
	0x00           /* byte2: ﾚｽﾎﾟﾝｽﾃﾞｰﾀ (Readyを報告) */
};

/*-- StatusResponse(Busy)ﾒｯｾｰｼﾞ --*/
static const u8	ex_mes_res_status_busy[]={
	RES_ID_DNLOAD_START, /* byte0: ﾚｽﾎﾟﾝｽｺｰﾄﾞ */
	1,                   /* byte1: ﾚｽﾎﾟﾝｽﾃﾞｰﾀｻｲｽﾞ */
	0x01                 /* byte2: ﾚｽﾎﾟﾝｽﾃﾞｰﾀ (Busyを報告) */
};

/*-- DownLoadStartResponce(Sucess)ﾒｯｾｰｼﾞ --*/
static const u8	ex_mes_res_download_start_success[]={
	RES_ID_DNLOAD_START, /* byte0: ﾚｽﾎﾟﾝｽｺｰﾄﾞ */
	1,                   /* byte1: ﾚｽﾎﾟﾝｽﾃﾞｰﾀｻｲｽﾞ */
	0x00                 /* byte2: ﾚｽﾎﾟﾝｽﾃﾞｰﾀ (Successを報告) */
};

/*-- DownLoadDataResponce(Sucess)ﾒｯｾｰｼﾞ --*/
static const u8	ex_mes_res_download_data_success[]={
	RES_ID_DNLOAD_DATA,	/* byte0: ﾚｽﾎﾟﾝｽｺｰﾄﾞ */
	1,							/* byte1: ﾚｽﾎﾟﾝｽﾃﾞｰﾀｻｲｽﾞ */
	0x00						/* byte2: ﾚｽﾎﾟﾝｽﾃﾞｰﾀ (Successを報告) */
};
/*-- DownLoadDataRespose(Fail)ﾒｯｾｰｼﾞ --*/
static const u8	ex_mes_res_download_data_fail[]={
	RES_ID_DNLOAD_DATA,	/* byte0: ﾚｽﾎﾟﾝｽｺｰﾄﾞ */
	1,							/* byte1: ﾚｽﾎﾟﾝｽﾃﾞｰﾀｻｲｽﾞ */
	0x01						/* byte2: ﾚｽﾎﾟﾝｽﾃﾞｰﾀ (Failを報告) */
};
/************************** Function Prototypes ******************************/
extern void OperationUsbReqSendData(void);
extern void _subline_set_mode(u16 mode);
static u32 _dl_trans_adr_noncashe(u32 adr);
static u8 _dl_verify_command_format(void);
static void _dl_send_data(const u8 *pt, u32 len);
static void _dl_proc_download_data(void);
static void _dl_cmd_analyze(void);
static void _dl_cmdD0(void);
static void _dl_cmdD1(void);
static void _dl_cmdD2(void);
static void _dl_cmdD3(void);
static void _dl_cmdD4(void);
static void _dl_cmd40(void);
static void _dl_get_adr_dl(void);
static u8 _dl_check_file_header(u8 *rxbuf);
static u8 _write_rom(u8 *pdata, u32 len);
/************************** Variable declaration *****************************/
extern const unsigned char software_ver[64];
static u8	ex_main_work_req;
static u8	*ex_p_dl_start;		/* ﾀﾞｳﾝﾛｰﾄﾞ先 先頭 ｱﾄﾞﾚｽ(ｷｬｯｼｭ無効空間に変換後) */
static u8	*ex_p_dl_write;		/* ﾀﾞｳﾝﾛｰﾄﾞ先 書き込み ｱﾄﾞﾚｽ(ｷｬｯｼｭ無効空間に変換後) */
static u8	ex_flag_started;
static u8	ex_flag_1stdldata;
static u16	ex_file_crc_result;
static u8 *ex_download_image;
static u8 *ex_download_written;

/*-- UBC⇔PC ﾒｯｾｰｼﾞ定義 --*/
/* ｺﾏﾝﾄﾞ */
#define IDX_CMD				0		/* byte0: COMMAND CODE						*/
#define IDX_LEN				1		/* byte1: (DLDataｺﾏﾝﾄﾞ以外)ﾒｯｾｰｼﾞ全長		*/
#define IDX_LEN_H			1		/* byte1: (DLDataｺﾏﾝﾄﾞ)ﾒｯｾｰｼﾞ全長 上位byte	*/
#define IDX_LEN_L			2		/* byte2: (DLDataｺﾏﾝﾄﾞ)ﾒｯｾｰｼﾞ全長 下位byte	*/
#define IDX_DATA			2		/* byte2: (DLDataｺﾏﾝﾄﾞ以外)DATA開始位置 	*/
#define IDX_DLDATA			3		/* byte3: (DLDataｺﾏﾝﾄﾞ)DATA開始位置 		*/
/* ﾚｽﾎﾟﾝｽ */
#define IDX_RES				0		/* byte0: RESPONSE CODE						*/

/*-- ﾀﾞｳﾝﾛｰﾄﾞﾌｧｲﾙﾌｫｰﾏｯﾄ --*/
#define ADROFS_START		0x06	/*  6~ 9byte: ﾀﾞｳﾝﾛｰﾄﾞ先先頭絶対ｱﾄﾞﾚｽ		*/
#define ADROFS_END			0x0A	/* 10~13byte: ﾀﾞｳﾝﾛｰﾄﾞ先終了絶対ｱﾄﾞﾚｽ		*/
#define WADROFS_VER			0x0E	/* 14~15byte: ﾊﾞｰｼﾞｮﾝ情報へのｱﾄﾞﾚｽへのｱﾄﾞﾚｽ */


u8 _write_rom(u8 *pdata, u32 len)
{
	UINT32 address;
	UINT32 i;
	QSPI_BUF_INFO qspi_buf;

	UINT32 dst_address;
	UINT32 src_address;
	const UINT32 blocksize = 0x10000;

	src_address = (UINT32)pdata;
	dst_address = (UINT32)ex_p_dl_write + DDR_START_ADDRESS;

	for(int cnt = 0; cnt < len/blocksize; cnt++)
	{
		// memory compare 1 sector(4K byte)
		if(memcmp((const _PTR)src_address, (const _PTR)dst_address,blocksize) != 0)
		{
			// write 4K byte
			//xil_printf("START:INT8 _write_rom(pdata, = 0x%08x, len = 0x%0x); \r\n",pdata, len);

			address = (UINT32)ex_p_dl_write;

			//irq_disable();
			qspi_buf.buf = (u8 *)src_address;
			qspi_buf.addr = address;
			qspi_buf.len = blocksize;
			qspi_buf.byte_count = &i;
			if( QSPI_Flash_Write( &hQFlash, &qspi_buf, QSPI_FLASH_ERASE_AUTO ) == FALSE ){
				DBG_ERR();
				return(FALSE);
			};
			//xil_printf("END:INT8 _write_rom(pdata, = 0x%08x, len = 0x%0x); \r\n",pdata, len);
		}
		/* 書き込みｱﾄﾞﾚｽ進める */
		ex_p_dl_write += blocksize;
		src_address += blocksize;
		dst_address += blocksize;
		OSW_TSK_sleep(1);
	}
	if(len%blocksize)
	{
		address = (UINT32)ex_p_dl_write;
		//irq_disable();
		qspi_buf.buf = (u8 *)pdata;
		qspi_buf.addr = address;
		qspi_buf.len = len%blocksize;
		qspi_buf.byte_count = &i;
		if( QSPI_Flash_Write( &hQFlash, &qspi_buf, QSPI_FLASH_ERASE_AUTO ) == FALSE ){
			DBG_ERR();
			return(FALSE);
		};
		/* 書き込みｱﾄﾞﾚｽ進める */
		ex_p_dl_write += len%blocksize;
	}

	return(TRUE);
}
/*********************************************************************//**
 * @brief		Main ROM update
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void subline_download(void)
{
	u8	ret;

	ex_main_work_req = 0;

	/* Receive data */
	ret = subline_receive_data();
	if (ret == SUBLINE_RECEIVE_COMPLETE)
	{
		_dl_cmd_analyze();
		if(ex_main_work_req == CMD_ID_DNLOAD_DATA)
		{
			_dl_proc_download_data();
		}
		ex_operation_usb_read_size = 0;
	}

	/* Send data */
	if(ex_operation_usb_write_size)
	{
		OperationUsbReqSendData();
	}

	if(ex_main_work_req == CMD_ID_DNLOAD_START)
	{
	}
	/* QSPI Flash operation */
	if(ex_main_work_req == CMD_ID_DNLOAD_END_CONF)
	{
		// write flash
		if(!_write_rom((u8 *)ex_download_written, ex_download_image - ex_download_written))
		{
			_subline_set_mode(SUBLINE_MODE_DOWNLOAD_ERROR);
		}
	}
	if (ex_main_work_req == CMD_ID_RESET)
	{
		_subline_set_mode(SUBLINE_MODE_DOWNLOAD_WAIT_RESET);
	}
}

/*********************************************************************//**
 * @brief		Analyze received data
 *
 *
 *
 * @param[in]	num received data size.
 * @return 		None
 **********************************************************************/
static void _dl_cmd_analyze(void)
{
	if(_dl_verify_command_format())
	{
		switch( ex_operation_usb_read_buffer[IDX_CMD] )
		{
		case CMD_ID_DNLOAD_DATA:
			osw_printf("START:_dl_cmdD2(); \r\n");
			_dl_cmdD2();
			break;
		case CMD_ID_STATUS:
			osw_printf("START:_dl_cmdD0(); \r\n");
			_dl_cmdD0();
			break;
		case CMD_ID_DNLOAD_START:
			osw_printf("START:_dl_cmdD1(); \r\n");
			_dl_cmdD1();
			break;
		case CMD_ID_DNLOAD_END_CONF:
			osw_printf("START:_dl_cmdD3(); \r\n");
			_dl_cmdD3();
			break;
		case CMD_ID_VERSION:
			osw_printf("START:_dl_cmdD4(); \r\n");
			_dl_cmdD4();
			break;
		case CMD_ID_RESET:
			osw_printf("START:_dl_cmd40(); \r\n");
			_dl_cmd40();
			break;
		default:
			return;
		}
	}
	return;
}

/*********************************************************************//**
 * @brief		Verify specified download command
 *
 *
 *
 * @param[in]	None
 * @return 		Valid or Invalid
 * @retval 		TRUE Valid Download command
 * @retval 		FALSE Invalid data
 **********************************************************************/
static u8 _dl_verify_command_format()
{
	u16 length;
	//1:check cmd
	switch( ex_operation_usb_read_buffer[IDX_CMD] )
	{
	case CMD_ID_DNLOAD_DATA:
	case CMD_ID_STATUS:
	case CMD_ID_DNLOAD_START:
	case CMD_ID_DNLOAD_END_CONF:
	case CMD_ID_VERSION:
	case CMD_ID_RESET:
		/* NEXT */
		break;
	default:
		return FALSE;
	}

	//2:check length
	switch( ex_operation_usb_read_buffer[IDX_CMD] )
	{
	case CMD_ID_DNLOAD_DATA:
		length = ex_operation_usb_read_buffer[IDX_LEN_H] << 8;
		length += ex_operation_usb_read_buffer[IDX_LEN_L];
		if(ex_operation_usb_read_size != length + 3)
		{
			return FALSE;
		}
		/* NEXT */
		break;
	case CMD_ID_STATUS:
	case CMD_ID_DNLOAD_START:
	case CMD_ID_DNLOAD_END_CONF:
	case CMD_ID_VERSION:
	case CMD_ID_RESET:
		length = ex_operation_usb_read_buffer[IDX_LEN];
		if(ex_operation_usb_read_size != length + 2)
		{
			return FALSE;
		}
		/* NEXT */
		break;
	default:
		return FALSE;
	}
	return TRUE;
}



/*------------------------------------------------------*/
/*=== ｺﾏﾝﾄﾞ処理関数	<D0h:StatusRequest処理 >		====*/
/*-- <引数>	なし										*/
/*------------------------------------------------------*/
static void _dl_cmdD0(void)
{
	if( ex_main_work_req ){
		/*----- BUSY時 -----*/
		/* StatusResponse"BUSY"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_status_busy, sizeof(ex_mes_res_status_busy));
	}
	else{
		/*----- NotBUSY時 -----*/
		/* StatusResponse"Ready"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_status_ready, sizeof(ex_mes_res_status_ready));
	}
}

/*------------------------------------------------------*/
/*=== ｺﾏﾝﾄﾞ処理関数	<D1h:DownloadStart処理 >		====*/
/*-- <引数>	なし										*/
/*------------------------------------------------------*/
static void _dl_cmdD1(void)
{
	if( ex_main_work_req ){
		/*----- BUSY時 -----*/
		/* StatusResponse"BUSY"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_status_busy, sizeof(ex_mes_res_status_busy));
	}
	else{
		/*----- NotBUSY時 -----*/
		/* ｺﾏﾝﾄﾞ処理要求発行 */
		ex_main_work_req = CMD_ID_DNLOAD_START;
		/*==============================*/

		/* DownloadResponse"Success"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_download_start_success, sizeof(ex_mes_res_download_start_success));
		/* DownloadStart実行済みを保存 */
		ex_flag_started = 1;
		/* ﾌﾗｸﾞ ex_flag_1stdldataセット */
		ex_flag_1stdldata = 1;
	}
}

/*------------------------------------------------------*/
/*=== ｺﾏﾝﾄﾞ処理関数	<D2h:DownloadData処理 >			====*/
/*-- <引数>	なし										*/
/*------------------------------------------------------*/
static void _dl_cmdD2(void)
{
	if( ex_main_work_req ){
		/*----- BUSY時 -----*/
		/* StatusResponse"BUSY"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_status_busy, sizeof(ex_mes_res_status_busy));
	}
	else if(!ex_flag_started)
	{
		/*-----DownloadStartｺﾏﾝﾄﾞ未受理 -----*/
		/* DownloadStartｺﾏﾝﾄﾞを受理後でないと受付けない */
		_dl_send_data((const u8 *)ex_mes_res_status_busy, sizeof(ex_mes_res_status_busy));
	}
	else{
		/*----- NotBUSY時 -----*/
		/*----- DownloadStartｺﾏﾝﾄﾞ受理済 -----*/
		if( ex_flag_1stdldata ){
			/*----- 最初のDownloadDataｺﾏﾝﾄﾞ受信時 -----*/
			/* (1)ｺﾏﾝﾄﾞ長ﾁｪｯｸ (最低でもﾛｰﾄﾞｴﾝﾄﾞｱﾄﾞﾚｽﾌｨｰﾙﾄﾞまで無いとだめ) */
			if( ex_operation_usb_read_size < 14-3 ){
				/*---- ｺﾏﾝﾄﾞ長不正時 -----*/
				// ステータスLEDエラー表示（ダウンロードファイルエラー）
				_subline_set_mode(SUBLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR);
				osw_printf("Download file error\n");
				return;
			}
			/* (2)ﾌｧｲﾙﾍｯﾀﾞﾁｪｯｸ */
			if( _dl_check_file_header(ex_operation_usb_read_buffer) != TRUE ){
				/*---- ﾌｧｲﾙﾍｯﾀﾞﾁｪｯｸ ｴﾗｰ時 -----*/
				_dl_send_data((const u8 *)ex_mes_res_download_data_fail, sizeof(ex_mes_res_download_data_fail));
				// ステータスLEDエラー表示（ダウンロードファイルエラー）
				_subline_set_mode(SUBLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR);
				osw_printf("Download file error\n");
				return;
			}
			/* (3)ﾀﾞｳﾝﾛｰﾄﾞ先 先頭/終了ｱﾄﾞﾚｽ取得 */
			_dl_get_adr_dl();
		}
		else{
			/*----- 2回目以降のDownloadDataｺﾏﾝﾄﾞ受信時 -----*/

		}
		/* ｺﾏﾝﾄﾞ発行 */
		ex_main_work_req = CMD_ID_DNLOAD_DATA;				/* ｺﾏﾝﾄﾞD2 		*/

		/* DownloadResponse"Success"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_download_data_success, sizeof(ex_mes_res_download_data_success));
	}
}

/*------------------------------------------------------*/
/*=== ｺﾏﾝﾄﾞ処理関数	<D3h:DownloadEndConfirm処理 >	====*/
/*-- <引数>	なし										*/
/*------------------------------------------------------*/
static void _dl_cmdD3(void)
{
	u8	crc_flag;

	if( ex_main_work_req ){
		/*----- BUSY時 -----*/
		/* StatusResponse"BUSY"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_status_busy, sizeof(ex_mes_res_status_busy));
	}
	else{
		/*----- NotBUSY時 -----*/
		/* ｺﾏﾝﾄﾞ処理要求発行 */
		ex_main_work_req = CMD_ID_DNLOAD_END_CONF;
        crc_flag = 0;
		/* DownloadEndStatusを返信(FILE CRCを返す) */
        ex_operation_usb_write_buffer[IDX_RES] = RES_ID_DNLOAD_END_CONF;	                /* byte0:ﾚｽﾎﾟﾝｽｺｰﾄﾞ */
		ex_operation_usb_write_buffer[IDX_LEN] = 3;								/* byte1:ﾚｽﾎﾟﾝｽﾃﾞｰﾀ長 */
		ex_operation_usb_write_buffer[IDX_DATA] = crc_flag;						/* byte2:ﾚｽﾎﾟﾝｽﾃﾞｰﾀ (CRC結果を報告) */
		ex_operation_usb_write_buffer[IDX_DATA+1] = *((u8 *)&ex_file_crc_result+1);	/* byte3:CRC結果(上位ﾊﾞｲﾄ)を入れる */
		ex_operation_usb_write_buffer[IDX_DATA+2] = *((u8 *)&ex_file_crc_result);	/* byte4:CRC結果(下位ﾊﾞｲﾄ)を入れる */
		/* 返信ﾒｯｾｰｼﾞ作成 & 返信 */
		_dl_send_data((const u8 *)ex_operation_usb_write_buffer, 5);							/* 全長5ﾊﾞｲﾄ送信 */
		//xil_printf("CRC:_dl_cmdD3();crc=0x%04x \r\n", ex_file_crc_result);
	}
}

static void _dl_cmdD4(void)
{
	u32		len;
	UINT32 byte;

	if( ex_main_work_req ){
		/*----- BUSY時 -----*/
		/* StatusResponse"BUSY"を返信 */
		_dl_send_data((const u8 *)ex_mes_res_status_busy, sizeof(ex_mes_res_status_busy));
	}
	else{
		/*----- NotBUSY時 -----*/
		// DONE:メインのバージョンアドレス取得
		memcpy((UINT8 *)&(ex_usb_write_buffer[IDX_DATA]),software_ver, sizeof(software_ver));
		len = sizeof(software_ver);
		/* 返信ﾒｯｾｰｼﾞ生成 */
		ex_operation_usb_write_buffer[IDX_RES] = RES_ID_VERSION;	/* byte0:ﾚｽﾎﾟﾝｽｺｰﾄﾞ */
		ex_operation_usb_write_buffer[IDX_LEN] = (u8)len;					/* byte1:ﾚｽﾎﾟﾝｽﾃﾞｰﾀ長 */
		/* 返信ﾒｯｾｰｼﾞ作成 & 返信 */
		_dl_send_data((const u8 *)ex_operation_usb_write_buffer, byte+2);		/* 送信 */
	}
}

/*------------------------------------------------------*/
/*=== ｺﾏﾝﾄﾞ処理関数	<40h:ResetCommand処理 >			====*/
/*-- <引数>	なし										*/
/*------------------------------------------------------*/
static void _dl_cmd40(void)
{
	ex_main_work_req = CMD_ID_RESET;				/* ｺﾏﾝﾄﾞ40 		*/
}

/*------------------------------------------------------*/
/*=== Main側処理関数	<DownLoadDataｺﾏﾝﾄﾞ処理>		====*/
/*-- <引数>	なし										*/
/*-- <返値> TRUE: 成功									*/
/*--		FALSE:失敗									*/
/*------------------------------------------------------*/
static void _dl_proc_download_data(void)
{
	u32 num_write;
	bool crc_clear = false;

	if( ex_flag_1stdldata ){
		/*----- 最初のﾀﾞｳﾝﾛｰﾄﾞｺﾏﾝﾄﾞ -----*/
		/* ﾌﾗｯｸﾞﾘｾﾄ */
		ex_flag_1stdldata = 0;

		//ex_download_image = (u8 *)DOWNLOAD_IMAGE_BUF;
		//ex_download_written = (u8 *)DOWNLOAD_IMAGE_BUF;
		// only clear crc value
		crc_clear = true;
	}

	/* 書き込み長 求める*/
	num_write = ex_operation_usb_read_size - 3;	/* ﾍｯﾀﾞ=3bytes */

	// copy download data to image buffer
	memcpy(ex_download_image, &ex_operation_usb_read_buffer[IDX_DLDATA],num_write);
	ex_file_crc_result = _calc_crc((u8 *)ex_download_image, (u32)num_write, crc_clear);
	ex_download_image += num_write;

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	if(ex_download_image - ex_download_written > DOWNLOAD_DATA_WRITE_SIZE)
	{
		// write flash
		if(!_write_rom((u8 *)ex_download_written, DOWNLOAD_DATA_WRITE_SIZE))
		{
			_subline_set_mode(SUBLINE_MODE_DOWNLOAD_ERROR);
			return;
		}
		ex_download_written += DOWNLOAD_DATA_WRITE_SIZE;
	}
}

/*------------------------------------------------------*/
/*=== PCへ返信										====*/
/*-- <引数>	u8   *pt	送信ﾃﾞｰﾀへのﾎﾟｲﾝﾀ				*/
/*--		UINT len	送信長							*/
/*------------------------------------------------------*/
static void _dl_send_data(const u8 *pt, u32 len)
{
	u32	i;

	if(pt != ex_operation_usb_write_buffer)
	{
		for(i=0; i<len; i++){
			ex_operation_usb_write_buffer[i] = *pt++;
		}
	}

	/* set send length */
	ex_operation_usb_write_size = len;
}

/*------------------------------------------------------*/
/*=== ﾀﾞｳﾝﾛｰﾄﾞﾌｧｲﾙ先頭のﾍｯﾀﾞのﾁｪｯｸ					====*/
/*-- <引数>	なし										*/
/*-- <返値>	TRUE: ﾌｧｲﾙﾍｯﾀﾞﾁｪｯｸOK						*/
/*-- 		FALSE:ﾌｧｲﾙﾍｯﾀﾞﾁｪｯｸNG						*/
/*------------------------------------------------------*/
static u8 _dl_check_file_header(u8 *rxbuf)
{
	/* ﾀﾞｳﾝﾛｰﾄﾞﾃﾞｰﾀ長は 1~30720bytes */
	/* =ｺﾏﾝﾄﾞ全長は 4~30723bytes */
	if( ex_operation_usb_read_size < 4 ) return(FALSE);
	if( ex_operation_usb_read_size > 30723 ) return(FALSE);

	/* 識別子文字列が "VFM21"でなければDLしない */
	if(	rxbuf[IDX_DLDATA + 0] != 'V' ||
		rxbuf[IDX_DLDATA + 1] != 'F' ||
		rxbuf[IDX_DLDATA + 2] != 'M' ||
		rxbuf[IDX_DLDATA + 3] != '2' ||
		rxbuf[IDX_DLDATA + 4] != '1' )
	{
		return(FALSE);
	}

	return(TRUE);
}

/*------------------------------------------------------*/
/*=== ﾀﾞｳﾝﾛｰﾄﾞ先ｱﾄﾞﾚｽ情報取得(受信ﾃﾞｰﾀから取得)		====*/
/*-- <引数>	なし										*/
/*-- <返値>	なし										*/
/*--  変数p_dl_start, p_dl_writeへ値をｾｯﾄ				*/
/*--  変数p_dl_endへ値をｾｯﾄ								*/
/*--  ※出力ｱﾄﾞﾚｽは,ｷｬｯｼｭ無効空間のｱﾄﾞﾚｽへ変換する		*/
/*------------------------------------------------------*/
static void _dl_get_adr_dl(void)
{
	ex_p_dl_start = (u8 *)_dl_trans_adr_noncashe( (u32)0x80000 );
	ex_p_dl_write = (u8 *)_dl_trans_adr_noncashe( (u32)ex_p_dl_start );
}

/*------------------------------------------------------*/
/*=== <<ｲﾝﾗｲﾝ関数>>	ｷｬｯｼｭ有効空間のｱﾄﾞﾚｽは,				*/
/*					ｷｬｯｼｭ無効空間ｱﾄﾞﾚｽへ変換する		*/
/*-- <引数>	u32 a : 変換するｱﾄﾞﾚｽ						*/
/*-- <返値> 変換後のｱﾄﾞﾚｽ								*/
/*------------------------------------------------------*/
static u32 _dl_trans_adr_noncashe(u32 adr)
{
	return(adr);
}
/**
 * @}
 */

