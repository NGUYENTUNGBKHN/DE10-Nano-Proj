/******************************************************************************/
/*! @addtogroup Group2
    @file       
    @brief      Main process for boot I/F
    @date       2012/09/14
    @author     
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************

******************************************************************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "itron.h"
#include "common.h"
#include "custom.h"
#include "id0g8.h"
#include  "IF_ID0G8.def"
#include "com_dfu_id0g8.h"
#include "com_dfu_def.h"	// 

#include "memorymap.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"


extern UINT8* l_pucDtSndBufDfu0G8;		// 
extern UINT16 l_usSndDataSizeDfu0G8;	// 


u8	ex_down_start_0g8;

static u8	ex_flag_1stdldata;	/*  */

static u8 *ex_download_image;	/*  */
static u8 *ex_download_written;	/*  */
#define DOWNLOAD_DATA_WRITE_SIZE (64*1024)		/*  */


static u8	*ex_p_dl_write;		//  /* ﾀﾞｳﾝﾛｰﾄﾞ先 書き込み ｱﾄﾞﾚｽ(ｷｬｯｼｭ無効空間に変換後) */
extern u8 l_aulID0G8_rx_buff[COM_DFU_BUF_SIZE];		// 
INT8 _write_rom_0g8(u8 *pdata, u32 len); // 
u8 _dl_check_file_header_0g8(void); //use


#if !defined(ID0G8_BOOTIF)
T_MSG_BASIC cline_msg_0g8;
#endif


/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/

u8	DucDfuCurState_before;


#if defined(ID0G8_LOG)
extern void tetsuya_line(u8 type, u8 state);
#define IWASAKI_COUNT_MAX     50
u8	iwasaki_line_count;
u8	iwasaki_line[IWASAKI_COUNT_MAX];
#endif

#if defined(ID0G8_BOOTIF)
void fid0g8_download_main( void );// 
#else
static void fid0g8_download_main( void );
#endif

void SetAppIdleDetachableState(void);
void ProcUsbBusResetInDfu(void);
void ProcUnsupportedReqInDfu(void);
void ProcAppNormalEnumeratingState(void);
void ProcAppIdleDetachableState(void);
void ProcAppDetachIdleState(void);
void ProcAppDetachResetState(void);
void ProcDfuNormalEnumeratingState(void);
void ProcDfuIdleState(void);
void ProcDfuWaitGetStatus1State(void);
void ProcDfuWaitPollTimeoutState(void);

void ProcDfuWaitGetStatus2State(void);
void ProcDfuDnloadIdleState(void);
void ProcDfuWritingBufferingState(void);
void ProcDfuLastBkWritingState(void);
void ProcDfuManifestSyncState(void);
void ProcDfuManifestState(void);

void ProcDfuManifestWaitResetState(void);
void ProcDfuUploadIdleState(void);
void ProcDfuErrorState(void);
void ProcDfuErrorEnumeratingState(void);

void _intr_pusb_receive_id0g8_dfu(void);



u16 _DL_calc_crc_for_if( u8 *data, u32 start_adr, u32 end_adr, u8 setting, u16 init_value )
{

}

#if !defined(ID0G8_BOOTIF) // boot ifはすでに別ファイルで存在する
INT8 _read_rom(u8 *dst, u8 *offset, u32 len)
{
	UINT32 i;
	QSPI_BUF_INFO info;

	info.buf = (u8 *)dst;
	info.addr = (u32)offset;
	info.len = len;
	info.byte_count = &i;
#if 1//#ifdef  FLASH_QSPI_MODE
	if(ALT_E_ERROR == alt_qspi_read((void *)info.buf
			, info.addr
			, info.len))
	{
		return(FALSE);
	}
#else
	if( QSPI_Flash_Read( &hQFlash, &info ) == FALSE ){
		return(FALSE);
	};
#endif

	return(TRUE);
}
/*------------------------------------------------------*/
/*  ダウンロード後のQSPIからDDR3への展開                 */
/*  <引数> なし                                         */
/*  <返値> TRUE :OK                             		*/
/*------------------------------------------------------*/
u8 _download_copy_to_ddr3(void)
{
	u8 *dst;
	u8 *src;
	const u32 block_size = (64*1024);
	u32 copy_size;
	u32 cnt;

	copy_size = DuwTotalDnloadBytes_target;

	dst = (u8 *)BILL_NOTE_IMAGE_TOP_BASE;
	src = (u8 *)(PROGRAM_START_ADRESS - DDR_START_ADDRESS);
	cnt = copy_size / block_size;

	while(cnt--)
	{
		if(!_read_rom((u8 *)dst, (u8 *)src, block_size))
		{
			return FALSE;
		}
		copy_size -= block_size;
		src += block_size;
		dst += block_size;
		_hal_feed_wdt();
	}

	if(copy_size % block_size)
	{
		if(!_read_rom((u8 *)dst, (u8 *)src, copy_size % block_size))
		{
			return FALSE;
		}
		copy_size -= copy_size % block_size;
		src += copy_size % block_size;
		dst += copy_size % block_size;
		_hal_feed_wdt();
	}

	return TRUE;
}

/*------------------------------------------------------*/
/*  ダウンロード後のCRCチェック                           */
/*  <引数> なし                                         */
/*  <返値> TRUE :OK                             */
/*------------------------------------------------------*/
u8 download_calc_crc(void)
{
	u32 rom_size;
	u16 tmp_crc;
	if(!_download_copy_to_ddr3())
	{
		return(DFU_STATUS_ERR_VERIFY);
	}

	rom_size = DuwTotalDnloadBytes_target;

	ex_cline_download_control.program_crc = _calc_crc( (u8 *)BILL_NOTE_IMAGE_TOP_BASE, rom_size - 2, TRUE );
	ex_cline_download_control.file_crc = _calc_crc( (u8 *)BILL_NOTE_IMAGE_TOP_BASE + rom_size - 2, 2,   FALSE );

    tmp_crc = *(u8 *)(BILL_NOTE_IMAGE_TOP_BASE + rom_size - 2);
    tmp_crc = (tmp_crc << 8) & 0xff00;
    tmp_crc = tmp_crc + *(u8 *)(BILL_NOTE_IMAGE_TOP_BASE + rom_size - 1);

    if( tmp_crc != ex_cline_download_control.program_crc )
    {
		return(DFU_STATUS_ERR_VERIFY);
    }
	return(DFU_STATUS_OK);
}
#endif


u8 _dl_check_file_header_0g8(void)
{

	u32 download_start=0;
	u32 download_end=0;
	u8 result;


    if( DuwDnBlockSize < 31 )
	{
		return(DFU_STATUS_ERR_TARGET);
	}

	//fid0g8_check_file_header(void)
	if( l_aulID0G8_rx_buff[0] != 'V' ||
		l_aulID0G8_rx_buff[1] != 'F' ||
		l_aulID0G8_rx_buff[2] != 'M' ||
		l_aulID0G8_rx_buff[3] != '2' ||
		l_aulID0G8_rx_buff[4] != '0' )
	{
		return(DFU_STATUS_ERR_TARGET);
	}

	/* BootI/Fのバージョンをチェック */
	/* 0x1A～0x1E(26～30) */
	//fid0g8_download_CheckRomHeader();
	if( l_aulID0G8_rx_buff[26] != 'I' ||
		l_aulID0G8_rx_buff[27] != 'D' ||
		l_aulID0G8_rx_buff[28] != '0' ||
		l_aulID0G8_rx_buff[29] != 'G' ||
		l_aulID0G8_rx_buff[30] != '8' )
	{
		return(DFU_STATUS_ERR_TARGET);
	}

	/* ダウンロードサイズ確認 */
	download_start = (u32)((u32)(l_aulID0G8_rx_buff[6]<<24) + (u32)(l_aulID0G8_rx_buff[7]<<16)
				+ (u32)(l_aulID0G8_rx_buff[8]<<8) + (u32)(l_aulID0G8_rx_buff[9]) );  

	download_end = (u32)((u32)(l_aulID0G8_rx_buff[10]<<24) + (u32)(l_aulID0G8_rx_buff[11]<<16)
				+ (u32)(l_aulID0G8_rx_buff[12]<<8) + (u32)(l_aulID0G8_rx_buff[13]) );  

	DuwTotalDnloadBytes_target = download_end - download_start + 1;

	return(CHECK_OK);

}


static u8 _dl_proc_download_data_0g8(void) // 
{
	u32 num_write;
	u8 result = DFU_STATUS_OK;

	if( ex_flag_1stdldata ){
		/*----- 最初のﾀﾞｳﾝﾛｰﾄﾞｺﾏﾝﾄﾞ -----*/
		ex_flag_1stdldata = 0;

		result = _dl_check_file_header_0g8();
		if( result != 0 )
		{
			return result;
		}

		ex_p_dl_write = (u8 *)( (u32)0x80000 ); /* 書き込み先先頭offset Boot IF */

		#if defined(ID0G8_BOOTIF)
		ex_download_image = (u8 *)DOWNLOAD_IMAGE_BUF;
		ex_download_written = (u8 *)DOWNLOAD_IMAGE_BUF;
		#else
		ex_download_image = (u8 *)BILL_NOTE_IMAGE_TOP_BASE;
		ex_download_written = (u8 *)BILL_NOTE_IMAGE_TOP_BASE;
		#endif
	}

	/* 書き込み長 求める*/
	num_write = DuwDnBlockSize;

	// copy download data to image buffer
	memcpy(ex_download_image, &l_aulID0G8_rx_buff[0],num_write); // 

	ex_download_image += num_write; // 

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	//if( DuwTotalDnloadBytes >= 0xF80000 - 0x20000)
	if( DuwTotalDnloadBytes >= DuwTotalDnloadBytes_target - 0x20000)	// a
	{
		if(!_write_rom_0g8((u8 *)ex_download_written, ex_download_image - ex_download_written))
		{
			return(DFU_STATUS_ERR_TARGET);
		}
		DuwTotalDnloadBytes += (ex_download_image - ex_download_written);
		ex_download_written += (ex_download_image - ex_download_written);
	}
	else if(ex_download_image - ex_download_written > DOWNLOAD_DATA_WRITE_SIZE)
	{
		// write flash
		if(!_write_rom_0g8((u8 *)ex_download_written, DOWNLOAD_DATA_WRITE_SIZE))
		{
			return(DFU_STATUS_ERR_TARGET);
		}
		ex_download_written += DOWNLOAD_DATA_WRITE_SIZE;
		DuwTotalDnloadBytes +=  DOWNLOAD_DATA_WRITE_SIZE;
	}
	return result;
}




INT8 _write_rom_0g8(u8 *pdata, u32 len)
{
	UINT32 address;
	UINT32 i;
	QSPI_BUF_INFO qspi_buf;

	ALT_STATUS_CODE stat;
	UINT32 dst_address;
	UINT32 src_address;
	const UINT32 blocksize = 0x10000;

	alt_qspi_sram_partition_set(0x01);

	src_address = (UINT32)pdata;	/* 書き込みデータバッファアドレス */
	dst_address = (UINT32)ex_p_dl_write + DDR_START_ADDRESS;

	for(int cnt = 0; cnt < len/blocksize; cnt++)
	{
		// memory compare 1 sector(4K byte)
		if(memcmp((const _PTR)src_address, (const _PTR)dst_address,blocksize) != 0)
		{
			address = (UINT32)ex_p_dl_write;
			qspi_buf.buf = (u8 *)src_address;
			qspi_buf.addr = address;
			qspi_buf.len = blocksize;
			qspi_buf.byte_count = &i;
			stat = alt_qspi_replace(qspi_buf.addr
					, (u8 *)qspi_buf.buf
					, qspi_buf.len
					, (char *)QSPI_REPLACE_BUF
					, QSPI_REPLACE_SIZE);
			if(stat != ALT_E_SUCCESS)
			{
				return(FALSE);
			}
		}
		/* 書き込みｱﾄﾞﾚｽ進める */
		ex_p_dl_write += blocksize;
		src_address += blocksize;
		dst_address += blocksize;
	}
	if(len%blocksize)
	{
		address = (UINT32)ex_p_dl_write;
		//irq_disable();
		qspi_buf.buf = (u8 *)pdata;
		qspi_buf.addr = address;
		qspi_buf.len = len%blocksize;
		qspi_buf.byte_count = &i;
		stat = alt_qspi_replace(qspi_buf.addr
				, (u8 *)qspi_buf.buf
				, qspi_buf.len
				, (char *)QSPI_REPLACE_BUF
				, QSPI_REPLACE_SIZE);
		if(stat != ALT_E_SUCCESS)
		{
			return(FALSE);
		}
		/* 書き込みｱﾄﾞﾚｽ進める */
		ex_p_dl_write += len%blocksize;
	}

	return(TRUE);
}



/*------------------------------------------------------*/
/*  I/F領域正常時の初期化処理                           */
/*    I/F領域からダウンロード開始コマンドによりBIFへ    */
/*    移動した場合の初期化処理                          */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
//void Fid003_download_init( void )
void Fid0g8_download_init( void )
{
    /* ダウンロード関係変数の初期化 */

	#if defined(ID0G8_BOOTIF)// 

	Cm01Detach = 0;
	DucDfuProgramModeFlag = PROGRAM_MODE_DFU;
	DucDfuStatus = DFU_STATUS_ERR_FIRMWARE;	/* 0x0A	*/
	DucDfuCurState = DFU_ISTATE_DFU_ERROR_ENUMERATING; /* Set Configuから遷移させる場合*/// same as UBA
	DucDfuNxtState = DFU_ISTATE_DFU_ERROR_ENUMERATING; /* Set Configuから遷移させる場合*/// same as UBA

	#else

	/* HID -> DFU */
	Cm01Detach = DusDetachTimeout;
	DucDfuCurState = DFU_ISTATE_APP_DETACH_IDLE;	//same uba //* AppDetach-Idle state 			*//* Detach受信からの処理	*//* Bus Reset待ちへ */
	fid0g8_download_main();

	#endif
 
    return;
}

/*------------------------------------------------------*/
/*  BIFメイン関数                                       */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
#if defined(ID0G8_BOOTIF)
void fid0g8_download_main( void )	// iPRO _GSA_DFU_IF_main
#else
static void fid0g8_download_main( void )	// iPRO _GSA_DFU_IF_main
#endif
{
    u8 rtn;
	u8 test;
    u16 test_crc;

	#if !defined(ID0G8_BOOTIF)
	ER ercd;
	T_MSG_BASIC *tmsg_pt;
	#endif


    for( ;; )
    {

	#if !defined(ID0G8_BOOTIF)
		ercd = prcv_mbx(ID_CLINE_MBX, (T_MSG **)&tmsg_pt);
		if (ercd == E_OK)
		{
		/* Receiving task message */
			memcpy(&cline_msg_0g8, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(cline_msg_0g8.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_cline_system_error(1, 3);
			}
			_id0g8_msg_proc();
		}
	#endif

		if(ex_usb_dfu_disconect_0g8 == 1) //2022-09-21
		{
			_soft_reset();	
		}

		_intr_pusb_receive_id0g8_dfu();

		#if defined(ID0G8_LOG)
		tetsuya_line(DucDfuCurState, 0);	/* main */
		#endif

		/* DFU I/F現内部ステートにより処理を分岐 */
		switch (DucDfuCurState) {
		/* AppNormal-Enumerating state */
		case DFU_ISTATE_APP_NORMAL_ENUMERATING:	/* 0x00 */
			/* AppNormal-Enumeratingステート処理 */
			ProcAppNormalEnumeratingState();
			/* switch文を抜ける */
			break;
		/* AppIdle-Not-Detachable state */
	#if 0
		case DFU_ISTATE_APP_IDLE_NOT_DETACHABLE:	/* not use	*//* 0x01 *//* not use iPROも使用していない */
			/* AppIdle-Not-Detachableステート処理 */
			ProcAppIdleNotDetachableState();		/* not use iPROも使用していない */
			/* switch文を抜ける */
			break;
	#endif

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

		/* 割込みから直接0x04にするか、下記の処理を使用して0x02,0x03と遷移していく方法のどちらがいいか検討する */
		/* 調査結論としては、割込みから直接0x03にする, 0x02から遷移した場合、Detachコマンド後のBus Resetが早くてDetachコマンドが上書きされて、上手く遷移できない為 */
		/* 正し、タイムアウトでの処理戻しの為にこのケース分は必要 */
		/* AppIdle-Detachable state */
		case DFU_ISTATE_APP_IDLE_DETACHABLE:	/* HID modeからのDetach 時 step1(USB Detachコマンドをクリア(CMD_NUM_DFU_DETACH)	*//* 0x02 */
			/* AppIdle-Detachableステート処理 */
			ProcAppIdleDetachableState();		/* Detachコマンド受信で、タイムアウトの設定と0x03へ遷移*/
			/* switch文を抜ける */
			break;

/*-----------------------------------------------------------------------------------------------------------------------------------------*/

		/* AppDetach-Idle state */
		case DFU_ISTATE_APP_DETACH_IDLE:	/* HID modeからのDetach 時 step1(USB Bus Resetを確認 クリア)	*//* 0x03 */
			/* AppDetach-Idleステート処理 */
			ProcAppDetachIdleState();		/* Bus Resetで0x04へ遷移、タイムアウトの場合は0x02へ */
			/* switch文を抜ける */
			break;

		/* AppDetach-Reset state */
		case DFU_ISTATE_APP_DETACH_RESET:	/* HID modeからのDetach 時 step2(ディスクリプタの初期化DFU, USB disconnect)	*//* 0x04 */
			/* AppDetach-Resetステート処理 */
			ProcAppDetachResetState();	/* この中でDUFのディスクリプタの設定とUSB Disconnectを行う	*/
			/* switch文を抜ける */
			break;
		/* DfuNormal-Enumerating state */
		case DFU_ISTATE_DFU_NORMAL_ENUMERATING:	/* 0x05 */
			/* DfuNormal-Enumeratingステート処理 */
			ProcDfuNormalEnumeratingState();	/* Set config待ち、大切な処理ではないので、このシーケンスを廃止してもいいかも*/
			/* switch文を抜ける */
			break;
		/* DfuIdle state */
		case DFU_ISTATE_DFU_IDLE:/* 0x06*/
			/* DfuIdleステート処理 */
			ProcDfuIdleState();
			/* switch文を抜ける */
			break;
	#if 0	/* UBA500は、0x06から0x08に遷移させる為、これは使用しない */
		/* DfuFirstBk-Buffering state */
		case DFU_ISTATE_DFU_FIRST_BK_BUFFERING:/* 0x07 *//* これ使用しているか、確認*//* 0x06からの遷移廃止により、この処理に入ることはなくなる*/
			/* DfuFirstBk-Bufferingステート処理 */
			ProcDfuFirstBkBufferingState();
			/* switch文を抜ける */
			break;
	#endif
		/* DfuWait-GetStatus1 state */
		case DFU_ISTATE_DFU_WAIT_GETSTATUS1:/* 0x08 */
			/* DfuWait-GetStatus1ステート処理 */
			ProcDfuWaitGetStatus1State();
			/* switch文を抜ける */
			break;
		/* DfuWait-PollTimeout state */
		case DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT:	/* 0x09 */	/* Flash書き込み処理 *//* 0x09 *//* DFU_ISTATE_DFU_WAIT_GETSTATUS1でDFU_GETSTATUSリクエスト受信時 */
			/* DfuWait-PollTimeoutステート処理 */
			ProcDfuWaitPollTimeoutState();	/* DFUのDown Loadの実行部 Flash書き込み処理	*/
			/* switch文を抜ける */
			break;
		/* DfuWait-GetStatus2 state */
		case DFU_ISTATE_DFU_WAIT_GETSTATUS2:	/* 0x0A *//* DFU_ISTATE_DFU_WAIT_POLL_TIMEOUTでダウンロードのブロック完了時 */
			/* DfuWait-GetStatus2ステート処理 */
			ProcDfuWaitGetStatus2State();
			/* switch文を抜ける */
			break;
		/* DfuDnload-Idle state */
		case DFU_ISTATE_DFU_DNLOAD_IDLE:	/* 0x0B*//* DFU_ISTATE_DFU_WAIT_GETSTATUS2でDFU_GETSTATUSリクエスト受信時など	*/
			/* DfuDnload-Idleステート処理 */
			ProcDfuDnloadIdleState();
			/* switch文を抜ける */
			break;
		/* DfuWriting-Buffering state */
		case DFU_ISTATE_DFU_WRITING_BUFFERING:	/* 0x0C (12)*/
			/* DfuWriting-Bufferingステート処理 */
			ProcDfuWritingBufferingState();
			/* switch文を抜ける */
			break;
		/* DfuLastBk-Writing state */
		case DFU_ISTATE_DFU_LAST_BK_WRITING: /* 0x0D *//* 13 *//* DFU_ISTATE_DFU_DNLOAD_IDLEでダウンロードブロックサイズが0の場合 */
			/* DfuLastBk-Writingステート処理 */
			ProcDfuLastBkWritingState();
			/* switch文を抜ける */
			break;
		/* DfuManifest-Sync state */
		case DFU_ISTATE_DFU_MANIFEST_SYNC:	/* 0x0E *//* 14 *//* DFU_ISTATE_DFU_LAST_BK_WRITINGから無条件で */
			/* DfuManifest-Syncステート処理 */
			ProcDfuManifestSyncState();
			/* switch文を抜ける */
			break;
		/* DfuManifest state */
		case DFU_ISTATE_DFU_MANIFEST:  /* 0x0F*//* 15 *//* DFU_ISTATE_DFU_MANIFEST_SYNCでDFU_GETSTATUSリクエスト受信時 *//* ダウンロード後のCRC確認処理 */
			/* DfuManifestステート処理 */
			ProcDfuManifestState();
			/* switch文を抜ける */
			break;
		case DFU_ISTATE_DFU_MANIFEST_WAIT_RESET:	/* 0x10 *//* 16 */
			/* DfuManifest-Wait-Resetステート処理 */
			ProcDfuManifestWaitResetState();	/* Bus Reset待ち */
			/* switch文を抜ける */
			break;
		/* DfuUpload-Idle state */
		case DFU_ISTATE_DFU_UPLOAD_IDLE:	/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */
			/* DfuUpload-Idleステート処理 */
			ProcDfuUploadIdleState();
			/* switch文を抜ける */
			break;
		/* DfuError state */
		case DFU_ISTATE_DFU_ERROR:			/* 0x12 */
			/* DfuErrorステート処理 */
			ProcDfuErrorState();
			/* switch文を抜ける */
			break;
		/* DfuError-Enumerating state */
		case DFU_ISTATE_DFU_ERROR_ENUMERATING:
			/* DfuError-Enumeratingステート処理 */
			ProcDfuErrorEnumeratingState();
			/* switch文を抜ける */
			break;
		}

		/* DFU I/Fの現内部ステートにより処理を分岐 */
		switch (DucDfuCurState) {
		/* AppNormal-Enumerating state */
		case DFU_ISTATE_APP_NORMAL_ENUMERATING:	/* 0x00 */
		/* AppIdle-Not-Detachable state */
		case DFU_ISTATE_APP_IDLE_NOT_DETACHABLE:	/* not use	*//* 0x01 *//* not use iPROも使用していない */
		/* AppIdle-Detachable state */
		case DFU_ISTATE_APP_IDLE_DETACHABLE:/* 0x02 */
			/* 処理を抜ける(これらのステートの場合、ループを抜ける) */
	#if defined(ID0G8_BOOTIF)
		case DFU_ISTATE_DFU_ERROR:
		case DFU_ISTATE_DFU_IDLE:
		case DFU_ISTATE_DFU_ERROR_ENUMERATING:
	#endif
			return;
		}
	}
    return;
}


/*
 *  処理概要（機能説明）
 *	　AppIdle-Not-Detachableステート設定処理。
 *
 *	1) DFU I/F現内部ステートをAppIdle-Not-Detachableステートに設定する。
 *	2) DFU I/F次内部ステートをAppIdle-Not-Detachableステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
#if 0
void SetAppIdleNotDetachableState(void)/* not use iPROも使用していない */
{
	/* DFU I/F現内部ステートをAppIdle-Not-Detachableステートに設定 */
	DucDfuCurState = DFU_ISTATE_APP_IDLE_NOT_DETACHABLE;	/* not use	*//* 0x01 *//* not use iPROも使用していない */

	/* DFU I/F次内部ステートをAppIdle-Not-Detachableステートに設定 */
	DucDfuNxtState = DFU_ISTATE_APP_IDLE_NOT_DETACHABLE;	/* not use	*//* 0x01 *//* not use iPROも使用していない */
}
#endif

/*
 *  処理概要（機能説明）
 *	　AppIdle-Detachableステート設定処理。
 *
 *	1) DFU I/F現内部ステートをAppIdle-Detachableステートに設定する。
 *	2) DFU I/F次内部ステートをAppIdle-Detachableステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void SetAppIdleDetachableState(void)
{
	/* DFU I/F現内部ステートをAppIdle-Detachableステートに設定 */
	DucDfuCurState = DFU_ISTATE_APP_IDLE_DETACHABLE;/* 0x02 */

	/* DFU I/F次内部ステートをAppIdle-Not-Detachableステートに設定 */
	DucDfuNxtState = DFU_ISTATE_APP_IDLE_DETACHABLE;/* 0x02 */
}

/*
 *  処理概要（機能説明）
 *	　DFUプログラムモード時のUSB Bus Reset受信処理。
 *
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) Flash ROMフラグが正常な場合、エラーチェック処理。
 *	   ・エラーチェック結果が正常な場合、プログラムモードをアプリケーションプログラムモードに設定。
 *	   ・エラーチェック結果が異常な場合、Flash ROMフラグを異常に設定。
 *  2) 上記2)のコメントは間違え
 *     DFUプログラムモード時にUSBリセットを受信するとエニュマレーション後にDFUのERRORステータスにする
 *
 *	3) DISCONNECT処理。
 *	4) リアUSB通信割り込み禁止。
 *	5) システムリセット処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcUsbBusResetInDfu(void)	/* DFUプログラムモード時のUSB Bus Reset受信処理	*//* UBAはこの処理でReset処理している、iPROはResetを廃止している*//*UBA500はエラーでない時はResetでエラーの時はそのままでもいいかも*/
{
	/* チェック結果(CHECK_OK:エラーなし, CHECK_NG:エラーあり) */
	u8 DucResult;

	/* USBコマンド番号を0(コマンド番号なし)に設定 */
	DucUsbCmdNo = CMD_NUM_NONE;

	/* UBAと同じCPU Resetにする*/
	/* iPROはReset処理を、ソフトリリース後、市場で使用時に廃止しているようだ、理由不明*/
	_soft_reset();
    while( 1 );

}

/*
 *  処理概要（機能説明）
 *	　DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/FのステータスをerrSTALLEDPKTに設定する。
 *	3) DFU I/F現内部ステートをDfuErrorステートに設定する。
 *	4) DFU I/F次内部ステートをDfuErrorステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcUnsupportedReqInDfu(void)
{
	/* USBコマンド番号を0(コマンド番号なし)に設定 */
	DucUsbCmdNo = CMD_NUM_NONE;
#if 0	/* 2013-07-09	*/
	/* DFU I/FのステータスをerrSTALLEDPKTに設定 */
	DucDfuStatus = DFU_STATUS_ERR_STALLEDPKT;

	/* DFU I/F現内部ステートをDfuErrorステートに設定 */
	DucDfuCurState = DFU_ISTATE_DFU_ERROR;

	/* DFU I/F次内部ステートをDfuErrorステートに設定 */
	DucDfuNxtState = DFU_ISTATE_DFU_ERROR;
#endif
}

/*
 *  処理概要（機能説明）
 *	　AppNormal-Enumeratingステート処理。
 *
 *	　処理なし。
 *	※AppIdle-Detachableステートへの状態遷移やUSB Bus Resetの受信処理は。
 *	HID I/Fのタイマ割り込みで行う。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcAppNormalEnumeratingState(void)
{
}


/*
 *  処理概要（機能説明）
 *	　AppIdle-Detachableステート処理。
 *
 *	[1] DFU_DETACHリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) Detachタイマーを開始。
 *	3) DFU I/F現内部ステートをAppDetach-Idleステートに設定する。
 *	4) DFU I/F次内部ステートをAppDetach-Idleステートに設定する。
 *	※AppIdle-Not-Detachableステートへの状態遷移はメインループ内で行う。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcAppIdleDetachableState(void)	/* HID modeからのDetach 時 step1(USB Detachコマンドをクリア(CMD_NUM_DFU_DETACH)	*//* 0x02 */
{
	/* DFU_DETACHリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_DETACH) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* Detachタイマーを開始 */
		Cm01Detach = DusDetachTimeout;

		/* DFU I/F現内部ステートをAppDetach-Idleステートに設定 */
		DucDfuCurState = DFU_ISTATE_APP_DETACH_IDLE;/* 0x03 */

		/* DFU I/F次内部ステートをAppDetach-Idleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_APP_DETACH_IDLE;/* 0x03 */
	}
}

/*
 *  処理概要（機能説明）
 *	　AppDetach-Idleステート処理。
 *
 *	[1] USBバスリセット受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) Detachタイマーを停止。
 *	3) DFU I/F現内部ステートをAppDetach-Resetステートに設定する。
 *	4) DFU I/F次内部ステートをAppDetach-Resetステートに設定する。
 *	[2] Detachタイムアウト発生時
 *	1) DFU I/F現内部ステートをAppIdle-Detachableステートに設定する。
 *	2) DFU I/F次内部ステートをAppIdle-Detachableステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcAppDetachIdleState(void)	/* HID modeからのDetach 時 step1(USB Bus Resetを確認 クリア)	*//* 0x03 */
{
	/* USB Bus Reset受信の場合？ イニシャル動作中にDetachコマンド受信した場合( HID->DFUへ ) */

	if( DucUsbCmdNo == CMD_NUM_USB_BUS_RESET ){

		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* Detachタイマーを停止 */
		Cm01Detach = 0;

		/* DFU I/F現内部ステートをAppDetach-Resetステートに設定 */
		DucDfuCurState = DFU_ISTATE_APP_DETACH_RESET;/* 0x04 */

		/* DFU I/F次内部ステートをAppDetach-Resetステートに設定 */
		DucDfuNxtState = DFU_ISTATE_APP_DETACH_RESET;/* 0x04 */

		/* 処理を抜ける */
		return;
	}

	/* Detachタイムアウト発生時？ */
	if (Cm01Detach == 0) {
		/* DFU I/F現内部ステートをAppIdle-Detachableステートに設定 */
		DucDfuCurState = DFU_ISTATE_APP_IDLE_DETACHABLE;/* 0x02 */

		/* DFU I/F次内部ステートをAppIdle-Detachableステートに設定 */
		DucDfuNxtState = DFU_ISTATE_APP_IDLE_DETACHABLE;/* 0x02 */

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　AppDetach-Resetステート処理。
 *  HID modeからのDetach step2
 *	1) DFUプログラムモード時のGSA-USB関連変数初期化処理。
 *	2) DISCONNECT & CONNECT処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcAppDetachResetState(void) /* HID modeからのDetach 時 step2(ディスクリプタの初期化DFU, USB disconnect)	*//* 0x04 */
{

	/* プログラムモードフラグをDFUプログラムモードに設定 */
	DucDfuProgramModeFlag = PROGRAM_MODE_DFU;

	/* DFU I/FのステータスをOKに設定 DFU Status OK*/
	DucDfuStatus = DFU_STATUS_OK;	/* 0x00	*/

	/* DFU I/F現内部ステートをDfuNormal-Enumeratingステートに設定 */
	DucDfuCurState = DFU_ISTATE_DFU_NORMAL_ENUMERATING;	/* 0x05	*/

	/* DFU I/F次内部ステートをDfuNormal-Enumeratingステートに設定 */
	DucDfuNxtState = DFU_ISTATE_DFU_NORMAL_ENUMERATING;	/* 0x05	*/


}

/*
 *  処理概要（機能説明）
 *	　DfuNormal-Enumeratingステート処理。
 *
 *	[1] SET_CONFIGURATIONリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuNormalEnumeratingState(void)	/* 0x05 *//* Set config待ち、大切な処理ではないので、このシーケンスを廃止してもいいかも */
{
	/* SET_CONFIGURATIONリクエスト受信時？ */
	#if !defined(ID0G8_BOOTIF)
	if (DucUsbCmdNo == CMD_NUM_SET_CONFIGURATION)/* これは必要*/
// 	 この条件がないと、USB通信が安定する前に次のシーケンスに移ると
//   次のシーケンスでUSB通信確立前のBus Resetを検出してCPUリセットになる
	#else
//	if (DucUsbCmdNo == CMD_NUM_SET_CONFIGURATION)/* iPROとタイミングが異なるので、場合によっては、このシーケンスを廃止にするかも*/
	//Boot IFではそもそも、このシーケンスになる事がない、IFでHIDからDFUモードへ遷移する時に通るのみ
	//その為、条件はどちらでもいい
	if(1)
	#endif
	{

		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuIdleステート処理。
 *
 *	[1] DFU_DNLOADリクエスト受信開始コマンド受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) 実際のダウンロードブロック番号を0で初期化。
 *	3) 総ダウンロードバイト数にダウンロードブロックサイズを設定。
 *	4) DFU I/F現内部ステートをDfuFirstBk-Bufferingステートに設定する。
 *	5) DFU I/F次内部ステートをDfuFirstBk-Bufferingステートに設定する。
 *	[2] DFU_UPLOADリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuUpload-Idleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuUpload-Idleステートに設定する。
 *	[3] DFU_GETSTATUSリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	[4] DFU_ABORTリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	[5] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[6] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuIdleState(void)/* 0x06*/
{
	/* DFU_DNLOADリクエスト受信開始コマンド受信時？ */
	/* iPROのUSBの処理の特殊な所は、1つの通信でデータステージでフラグを更新し*/
	/* 場合によっては、その後のステータスステージでフラグをさらに更新している所 */
	/* 例 */
	/* 0x21 0x01のダウンロードデータのデータステージで DucUsbCmdNo = CMD_NUM_DFU_DNLOAD_START */
	/* その後のステータスステージで					   DucUsbCmdNo = CMD_NUM_DFU_DNLOAD_END   */

	/* UBA500はこの関数と次のシーケンスのProcDfuFirstBkBufferingStateを1つにする */
	/* 理由はiPROのようにデータステージとステータスステージでシーケンスを遷移させる事が難しい*/

//	if (DucUsbCmdNo == CMD_NUM_DFU_DNLOAD_END) {		/* 0x21 0x01 DFU_DNLOAD iPROはendはステータスステージで設定する *//* 2020-10-05a */
	if (ex_dowload_data_recived_0g8 == CMD_NUM_DFU_DNLOAD_END) {		/* 0x21 0x01 DFU_DNLOAD iPROはendはステータスステージで設定する *//* 2020-10-05a */
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
	//	DucUsbCmdNo = CMD_NUM_NONE;
		ex_dowload_data_recived_0g8 = CMD_NUM_NONE;


		/* DFU I/F現内部ステートをDfuWait-GetStatus1ステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_WAIT_GETSTATUS1;/* 0x08 */

		/* DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT;	/* 0x09 */	/* Flash書き込み処理 *//* 0x09 *//* DFU_ISTATE_DFU_WAIT_GETSTATUS1でDFU_GETSTATUSリクエスト受信時 */


		/* 本来のProcDfuIdleStateでの処理も追加*/
		/* 総ダウンロードバイト数にダウンロードブロックサイズを設定 */
		DuwTotalDnloadBytes = 0;	/* 2020-10-27 *//* 軽いチェックの為であり、このサイズを見てダウンロードを止めているわけではない */


		ex_flag_1stdldata = 1;


		/* SignatureチェックフラグをSIG_CHECK_OFFでリセット */
		DucSigChkFlag_DFU = SIG_CHECK_OFF;	/* ダウンロードファイルのHeader確認有効*/

		/* 処理を抜ける */
		return;
	}

	/* DFU_UPLOADリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_UPLOAD) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuUpload-Idleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */

		/* DFU I/F次内部ステートをDfuUpload-Idleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */

		/* 処理を抜ける */
		return;
	}

	/* DFU_GETSTATUSリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_GETSTATUS) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* 処理を抜ける */
		return;
	}

	/* DFU_ABORTリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_ABORT) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* 処理を抜ける */
		return;
	}

	//とりあえず、復活させて確認する #if !defined(ID0G8_BOOTIF)
	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_DETACH) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuFirstBk-Bufferingステート処理。
 *
 *	[1] DFU_DNLOADリクエスト受信完了コマンド受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuWait-GetStatus1ステートに設定する。
 *	3) DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定する。
 *	4) 実際のダウンロードブロック番号をインクリメント。
 *	[2] DFU_ABORTリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	[3] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[4] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
#if 0 //defined(DFU_DOWN)	/* UBA500は、0x06から0x08に遷移させる為、これは使用しない */
void ProcDfuFirstBkBufferingState(void)	/* 0x07 */
{
	/* DFU_DNLOADリクエスト受信完了コマンド受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_DNLOAD_END) {		/* 0x21 0x01 DFU_DNLOAD iPROはendはステータスステージで設定する *//* 2020-10-05a */
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuWait-GetStatus1ステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_WAIT_GETSTATUS1;/* 0x08 */

		/* DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT;	/* 0x09 */	/* Flash書き込み処理 *//* 0x09 *//* DFU_ISTATE_DFU_WAIT_GETSTATUS1でDFU_GETSTATUSリクエスト受信時 */

		/* 処理を抜ける */
		return;
	}

	/* DFU_ABORTリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_ABORT) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}
#endif

/*
 *  処理概要（機能説明）
 *	　DfuWait-GetStatus1ステート処理。
 *
 *	[1] DFU_GETSTATUSリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuWait-PollTimeoutステートに設定する。
 *	3) DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定する。
 *	[2] DFU_ABORTリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	[3] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[4] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuWaitGetStatus1State(void)
{
	/* DFU_GETSTATUSリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_GETSTATUS) {		/* 0x21 0x03 Get Status */

		/* Get status受信まで書き込み開始処理に遷移しない理由不明、*/

		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* ここでようやく、書き込み処理へ遷移 */
		/* DFU I/F現内部ステートをDfuWait-PollTimeoutステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT;	/* 0x09 */	/* Flash書き込み処理 *//* 0x09 *//* DFU_ISTATE_DFU_WAIT_GETSTATUS1でDFU_GETSTATUSリクエスト受信時 */

		/* DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT;	/* 0x09 */	/* Flash書き込み処理 *//* 0x09 *//* DFU_ISTATE_DFU_WAIT_GETSTATUS1でDFU_GETSTATUSリクエスト受信時 */

		/* 処理を抜ける */
		return;
	}

	/* DFU_ABORTリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_ABORT) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuWait-PollTimeoutステート処理。
 *
 *	[1] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[2] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *	[3] ダウンロードブロック処理時
 *	1) ダウンロードブロック処理を行う。
 *	2) ダウンロードブロック処理結果に応じて状態遷移を行う。
 *	　・OKの場合、DfuWait-GetStatus2ステートに遷移(次ステート：DfuDnload-Idle)。
 *	　・errFILEの場合、DfuErrorステートに遷移(次ステート：DfuError)。
 *	　・errADDRESSの場合、DfuErrorステートに遷移(次ステート：DfuError)。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuWaitPollTimeoutState(void)
{
	/* ブロック処理結果(DFU_STATUS_OK:エラーなし, それ以外:エラーあり) */
	u8 DucDnBlockResult;

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}

	DucDnBlockResult = _dl_proc_download_data_0g8(); 		//  新しい書き込み関数

	/* ダウンロードブロック処理結果により処理を分岐 */
	switch (DucDnBlockResult) {
	/* OK */
	case DFU_STATUS_OK:
		/* DFU I/F現内部ステートをDfuWait-GetStatus2ステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_WAIT_GETSTATUS2;	/* 0x0A *//* DFU_ISTATE_DFU_WAIT_POLL_TIMEOUTでダウンロードのブロック完了時 */

		/* DFU I/F次内部ステートをDfuDnload-Idleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_DNLOAD_IDLE;	/* 0x0B*//* DFU_ISTATE_DFU_WAIT_GETSTATUS2でDFU_GETSTATUSリクエスト受信時など	*/

		/* switch文を抜ける */
		break;
	/* errTARGET */
	case DFU_STATUS_ERR_TARGET:
		/* DFU I/FのステータスをerrTARGETに設定 */
		DucDfuStatus = DFU_STATUS_ERR_TARGET;

		/* DFU I/F現内部ステートをDfuErrorステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_ERROR;

		/* DFU I/F次内部ステートをDfuErrorステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_ERROR;

		_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_ROM_WRITE, ex_operating_mode, 0, 0); /* ROM write error */

		/* switch文を抜ける */
		break;
	/* errADDRESS */
	case DFU_STATUS_ERR_ADDRESS:
		/* DFU I/FのステータスをerrADDRESSに設定 */
		DucDfuStatus = DFU_STATUS_ERR_ADDRESS;

		/* DFU I/F現内部ステートをDfuErrorステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_ERROR;

		/* DFU I/F次内部ステートをDfuErrorステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_ERROR;

		_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_ROM_WRITE, ex_operating_mode, 0, 0); /* ROM write error */

		/* switch文を抜ける */
		break;
	}
}



/*
 *  処理概要（機能説明）
 *	　DfuWait-GetStatus2ステート処理。
 *
 *	[1] DFU_GETSTATUSリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuDnload-Idleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuDnload-Idleステートに設定する。
 *	[2] DFU_ABORTリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	[3] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[4] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuWaitGetStatus2State(void)
{
	/* DFU_GETSTATUSリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_GETSTATUS) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuDnload-Idleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_DNLOAD_IDLE;	/* 0x0B*//* DFU_ISTATE_DFU_WAIT_GETSTATUS2でDFU_GETSTATUSリクエスト受信時など	*/

		/* DFU I/F次内部ステートをDfuDnload-Idleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_DNLOAD_IDLE;	/* 0x0B*//* DFU_ISTATE_DFU_WAIT_GETSTATUS2でDFU_GETSTATUSリクエスト受信時など	*/

		/* 処理を抜ける */
		return;
	}

	/* DFU_ABORTリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_ABORT) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuDnload-Idleステート処理。
 *
 *	[1] DFU_DNLOADリクエスト受信開始コマンド受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	a) ダウンロードブロックサイズが0以上の場合
 *	a1) DFU I/F現内部ステートをDfuWriting-Bufferingステートに設定する。
 *	a2) DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定する。
 *	b) ダウンロードブロックサイズが0の場合
 *	b1) DFU I/F現内部ステートをDfuLastBk-Writingステートに設定する。
 *	b2) DFU I/F次内部ステートをDfuManifestステートに設定する。
 *	[2] DFU_ABORTリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	[3] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[4] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuDnloadIdleState(void)	/* 0x0B*//* ProcDfuIdleState, ProcDfuFirstBkBufferingState と処理は似ている*/
{

//2022-07-29a	if (DucUsbCmdNo == CMD_NUM_DFU_DNLOAD_END) {		/* 0x21 0x01 DFU_DNLOAD iPROはendはステータスステージで設定する *//* 2020-10-05a */
	if (ex_dowload_data_recived_0g8 == CMD_NUM_DFU_DNLOAD_END) {		/* 0x21 0x01 DFU_DNLOAD iPROはendはステータスステージで設定する *//* 2020-10-05a */

		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		//DucUsbCmdNo = CMD_NUM_NONE;
		ex_dowload_data_recived_0g8 = CMD_NUM_NONE;

		/* ダウンロードブロックサイズが0以上の場合？ */
		/* 実際は1回の受信データのサイズ */
		if (DuwDnBlockSize > 0) {
			/* DFU I/F現内部ステートをDfuWriting-Bufferingステートに設定 */
			DucDfuCurState = DFU_ISTATE_DFU_WRITING_BUFFERING;	/* 0x0C (12)*/

			/* DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定 */
			DucDfuNxtState = DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT;	/* 0x09 */	/* Flash書き込み処理 *//* 0x09 *//* DFU_ISTATE_DFU_WAIT_GETSTATUS1でDFU_GETSTATUSリクエスト受信時 */

		}
		/* ダウンロードブロックサイズが0の場合？ */
		else {

			DucUsbCmdNo = CMD_NUM_NONE; /*  a *//* 次にステータスリクエストがくるので,フラグを消しておく */

			/* DFU I/F現内部ステートをDfuLastBk-Writingステートに設定 */
			DucDfuCurState = DFU_ISTATE_DFU_LAST_BK_WRITING;/* 0x0D *//* 13 *//* DFU_ISTATE_DFU_DNLOAD_IDLEでダウンロードブロックサイズが0の場合 */

			/* DFU I/F次内部ステートをDfuManifestステートに設定 */
			DucDfuNxtState = DFU_ISTATE_DFU_MANIFEST;  /* 0x0F*//* 15 *//* DFU_ISTATE_DFU_MANIFEST_SYNCでDFU_GETSTATUSリクエスト受信時 *//* ダウンロード後のCRC確認処理へ */
		}

		/* 処理を抜ける */
		return;
	}

	/* DFU_ABORTリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_ABORT) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuWriting-Bufferingステート処理。
 *
 *	1) DFU I/F現内部ステートをDfuWait-GetStatus1ステートに設定する。
 *	2) DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuWritingBufferingState(void)
{
	/* DFU I/F現内部ステートをDfuWait-GetStatus1ステートに設定 */
	DucDfuCurState = DFU_ISTATE_DFU_WAIT_GETSTATUS1;/* 0x08 */

	/* DFU I/F次内部ステートをDfuWait-PollTimeoutステートに設定 */
	DucDfuNxtState = DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT;	/* 0x09 */	/* Flash書き込み処理 *//* 0x09 *//* DFU_ISTATE_DFU_WAIT_GETSTATUS1でDFU_GETSTATUSリクエスト受信時 */
}

/*
 *  処理概要（機能説明）
 *	　DfuLastBk-Writingステート処理。
 *
 *	1) DFU I/F現内部ステートをDfuManifest-Syncステートに設定する。
 *	2) DFU I/F次内部ステートをDfuManifestステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuLastBkWritingState(void)
{
	/* DFU I/F現内部ステートをDfuManifest-Syncステートに設定 */
	DucDfuCurState = DFU_ISTATE_DFU_MANIFEST_SYNC;	/* 0x0E *//* 14 *//* DFU_ISTATE_DFU_LAST_BK_WRITINGから無条件で */

	/* DFU I/F次内部ステートをDfuManifestステートに設定 */
	DucDfuNxtState = DFU_ISTATE_DFU_MANIFEST;  /* 0x0F*//* 15 *//* DFU_ISTATE_DFU_MANIFEST_SYNCでDFU_GETSTATUSリクエスト受信時 *//* ダウンロード後のCRC確認処理へ */
}

/*
 *  処理概要（機能説明）
 *	　DfuManifest-Syncステート処理。
 *
 *	[1] DFU_GETSTATUSリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuManifestステートに設定する。
 *	3) DFU I/F次内部ステートをDfuManifestステートに設定する。
 *	[2] DFU_ABORTリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	[3] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[4] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuManifestSyncState(void)		/* 0x0E */
{
	/* DFU_GETSTATUSリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_GETSTATUS) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuManifestステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_MANIFEST;  /* 0x0F*//* 15 *//* DFU_ISTATE_DFU_MANIFEST_SYNCでDFU_GETSTATUSリクエスト受信時 *//* ダウンロード後のCRC確認処理へ */

		/* DFU I/F次内部ステートをDfuManifestステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_MANIFEST;  /* 0x0F*//* 15 *//* DFU_ISTATE_DFU_MANIFEST_SYNCでDFU_GETSTATUSリクエスト受信時 *//* ダウンロード後のCRC確認処理へ */

		/* 処理を抜ける */
		return;
	}

	/* DFU_ABORTリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_ABORT) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuManifestステート処理。
 *
 *	[1] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[2] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *	[3] ダウンロード処理結果チェック
 *	1) ダウンロード未完チェック処理を行う。
 *	2) 外部ROMチェック処理を行う。
 *	3) チェック結果に応じて状態遷移を行う。
 *	　・OKの場合、DfuManifest-Wait-Resetステートに遷移(次ステート：DfuManifest-Wait-Reset)。
 *	　・errNOTDONEの場合、DfuErrorステートに遷移(次ステート：DfuError)。
 *	　・errVERIFYの場合、DfuErrorステートに遷移(次ステート：DfuError)。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuManifestState(void)		/* 0x0F*//* 書き込み後のFlash CRC確認 */
{
	/* チェック結果 */
	u8 DucResult;

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}


	if(DuwTotalDnloadBytes != DuwTotalDnloadBytes_target ) // a
	{
		/* チェック結果をerrNOTDONEに設定 */
		DucResult = DFU_STATUS_ERR_ADDRESS;

	}
	/* 書き込みアドレスがエンドアドレス以上の場合(ダウンロード完了の場合)？ */
	else {

		/* 外部ROM(フラッシュROM)エラーチェック */
		DucResult = download_calc_crc();

		if(DucResult != DFU_STATUS_OK)
		{
			DucResult = DFU_STATUS_ERR_VERIFY;
		}
	}

	/* チェック結果により処理を分岐 */
	switch (DucResult) {
	/* OK */
	case DFU_STATUS_OK:
		/* DFU I/F現内部ステートをDfuManifest-Wait-Resetステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_MANIFEST_WAIT_RESET;	/* 0x10 *//* 16 *//* Bus Reset待ちへ*/

		/* DFU I/F次内部ステートをDfuManifest-Wait-Syncステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_MANIFEST_WAIT_RESET;	/* 0x10 *//* 16 *//* Bus Reset待ちへ*/

		/* switch文を抜ける */
		break;
	/* errNOTDONE */
	case DFU_STATUS_ERR_ADDRESS:
		/* DFU I/FのステータスをerrNOTDONEに設定 */
		DucDfuStatus = DFU_STATUS_ERR_NOTDONE;	/* Status 0x09 ダウンロードサイズ不適切 */

		/* DFU I/F現内部ステートをDfuErrorステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_ERROR;

		/* DFU I/F次内部ステートをDfuErrorステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_ERROR;

		_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_DOWNLOAD, ex_operating_mode, 0, 0);  /* dwonload file error */

		/* switch文を抜ける */
		break;
	/* errVERIFY */
	case DFU_STATUS_ERR_VERIFY:
		/* DFU I/FのステータスをerrVERIFYに設定 */
		DucDfuStatus = DFU_STATUS_ERR_VERIFY;	/* Status 0x07 CRCエラー関係*/

		/* DFU I/F現内部ステートをDfuErrorステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_ERROR;

		/* DFU I/F次内部ステートをDfuErrorステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_ERROR;

		_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, ex_operating_mode, 0, 0); /* ROM check IF-AREA error */

		/* switch文を抜ける */
		break;
	}
}


/*
 *  処理概要（機能説明）
 *	　DfuManifest-Wait-Resetステート処理。
 *
 *	[1] USBバスリセット受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) プログラムモードフラグをアプリケーションプログラムモードに設定。
 *	3) DISCONNECT処理。
 *	4) リアUSB通信割り込み禁止。
 *	5) システムリセット処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuManifestWaitResetState(void)
{
	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* プログラムモードフラグをアプリケーションプログラムモードに設定 ( DUF->HIDへ ) */
		DucDfuProgramModeFlag = PROGRAM_MODE_APPLICATION;	/* ここだとダウンロード完了後の1回めのディスクリプタはまにあわなくてDFUになる、どうするか保留 *//* Bus ResetCMD_NUM_USB_BUS_RESETの代入を割込みのコネクト完了から1つフェーズの前のReset Eventにするかも*/

		_soft_reset();

	    while( 1 );		/* これがなくてもResetはかかる*/

	}
}

/*
 *  処理概要（機能説明）
 *	　DfuUpload-Idleステート処理。
 *
 *	[1] DFU_UPLOADリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	a) 実際の読み込みサイズが要求サイズより小さい場合
 *	a1) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	a2) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	b) 実際の読み込みサイズが要求サイズより小さくない場合
 *	b1) DFU I/F現内部ステートをDfuUpload-Idleステートに設定する。
 *	b2) DFU I/F次内部ステートをDfuUpload-Idleステートに設定する。
 *	[2] DFU_GETSTATUSリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuManifestステートに設定する。
 *	3) DFU I/F次内部ステートをDfuManifestステートに設定する。
 *	[3] DFU_ABORTリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	3) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	[4] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[5] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuUploadIdleState(void)	/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */
{
	/* DFU_UPLOADリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_UPLOAD) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* 実際の読み込みサイズが要求サイズより小さい場合？ *//* uploadの最終フェーズ* *//* リクエストより、送るサイズが小さい(Flashの最後)*/
		if (DusUpActualLen < DusUpReqLen) {

			#if 0 // 2022-09-29
			// 受信割り込み data_send_upload で遷移させる様に変更
			/* DFU I/F現内部ステートをDfuIdleステートに設定 */
			DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

			/* DFU I/F次内部ステートをDfuIdleステートに設定 */
			DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/
			#endif

		}
		/* 実際の読み込みサイズが要求サイズより小さくない場合？ */
		else {
			/* DFU I/F現内部ステートをDfuUpload-Idleステートに設定 */
			DucDfuCurState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */

			/* DFU I/F次内部ステートをDfuUpload-Idleステートに設定 */
			DucDfuNxtState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */
		}

		/* 処理を抜ける */
		return;
	}

	/* DFU_GETSTATUSリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_GETSTATUS) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuUpload-Idleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */

		/* DFU I/F次内部ステートをDfuUpload-Idleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */

		/* 処理を抜ける */
		return;
	}

	/* DFU_ABORTリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_ABORT) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/


		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuErrorステート処理。
 *
 *	[1] DFU_CLRSTATUSリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/FステータスをOKに設定する。
 *	3) DFU I/F現内部ステートをDfuIdleステートに設定する。
 *	4) DFU I/F次内部ステートをDfuIdleステートに設定する。
 *	[2] DFU_GETSTATUSリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuManifestステートに設定する。
 *	3) DFU I/F次内部ステートをDfuManifestステートに設定する。
 *	[3] USBバスリセット受信時
 *	1) DFUプログラムモード時のUSB Bus Reset受信処理。
 *	[4] 未サポートリクエスト受信時
 *	1) DFUプログラムモード時の未サポートリクエスト受信処理。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuErrorState(void)
{
	/* DFU_CLRSTATUSリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_CLRSTATUS) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/FのステータスをOKに設定 */
		DucDfuStatus = DFU_STATUS_OK;

		/* DFU I/F現内部ステートをDfuIdleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* DFU I/F次内部ステートをDfuIdleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

		/* 処理を抜ける */
		return;
	}

	/* DFU_GETSTATUSリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_DFU_GETSTATUS) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuErrorステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_ERROR;

		/* DFU I/F次内部ステートをDfuErrorステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_ERROR;

		/* 処理を抜ける */
		return;
	}

	/* USB Bus Reset受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_USB_BUS_RESET) {
		/* DFUプログラムモード時のUSB Bus Reset受信処理 */
		ProcUsbBusResetInDfu();

		/* 処理を抜ける */
		return;
	}

	/* 未サポートリクエスト受信の場合？ */
	if (DucUsbCmdNo == CMD_NUM_UNSUPPORTED) {
		/* DFUプログラムモード時の未サポートリクエスト受信処理 */
		ProcUnsupportedReqInDfu();

		/* 処理を抜ける */
		return;
	}
}

/*
 *  処理概要（機能説明）
 *	　DfuError-Enumeratingステート処理。
 *
 *	[1] SET_CONFIGURATIONリクエスト受信時
 *	1) USBコマンド番号をコマンド番号なしに設定。
 *	2) DFU I/F現内部ステートをDfuErrorステートに設定する。
 *	3) DFU I/F次内部ステートをDfuErrorステートに設定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void ProcDfuErrorEnumeratingState(void)
{
	/* SET_CONFIGURATIONリクエスト受信時？ */
	if (DucUsbCmdNo == CMD_NUM_SET_CONFIGURATION) {
		/* USBコマンド番号を0(コマンド番号なし)に設定 */
		DucUsbCmdNo = CMD_NUM_NONE;

		/* DFU I/F現内部ステートをDfuErrorステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_ERROR;

		/* DFU I/F次内部ステートをDfuErrorステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_ERROR;
	}
}


void _intr_pusb_receive_id0g8_dfu(void)/* Front USBでは”_usb_musb_receive_data”　*//* 2020-10-08 */
{

}


#if defined(ID0G8_LOG)

void tetsuya_line(u8 type, u8 state)
{

	if( iwasaki_line_count > IWASAKI_COUNT_MAX -2 ) 
	{
		return;
	}

	if( DucDfuCurState_before == type ) 
	{
		return;
	}

	DucDfuCurState_before = type;
	iwasaki_line[iwasaki_line_count] = type;
	iwasaki_line_count++;

}

#endif

/*
 *  処理概要（機能説明）
 *	　USB Bus Reset通知処理。
 *
 *	　DFU I/F現内部ステートに応じて紙幣鑑別処理部へのUSB Bus Resetの通知を決定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void SignalUsbBusResetCmd(void)
{

	/* DFU I/F現内部ステートに応じて処理を分岐 */
	switch (DucDfuCurState) {
	/* AppIdle-Not-Detachable state */
	case DFU_ISTATE_APP_IDLE_NOT_DETACHABLE:
	/* AppIdle-Detachable state */
	case DFU_ISTATE_APP_IDLE_DETACHABLE:
	/* AppDetach-Idle state */
	case DFU_ISTATE_APP_DETACH_IDLE:
	/* DfuIdle state */
	case DFU_ISTATE_DFU_IDLE:
	/* DfuFirstBk-Buffering state */
//	case DFU_ISTATE_DFU_FIRST_BK_BUFFERING:
	/* DfuWait-GetStatus1 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS1:
	/* DfuWait-PollTimeout state */
	case DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT:
	/* DfuWait-GetStatus2 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS2:
	/* DfuDnload-Idle state */
	case DFU_ISTATE_DFU_DNLOAD_IDLE:
	/* DfuWriting-Buffering state */
	case DFU_ISTATE_DFU_WRITING_BUFFERING:
	/* DfuLastBk-Writing state */
	case DFU_ISTATE_DFU_LAST_BK_WRITING:
	/* DfuManifest-Sync state */
	case DFU_ISTATE_DFU_MANIFEST_SYNC:
	/* DfuManifest state */
	case DFU_ISTATE_DFU_MANIFEST:
	/* DfuManifest-Wait-Reset state */
	case DFU_ISTATE_DFU_MANIFEST_WAIT_RESET:
	/* DfuUpload-Idle state */
	case DFU_ISTATE_DFU_UPLOAD_IDLE:
	/* DfuError state */
	case DFU_ISTATE_DFU_ERROR:

		/* USB Bus Reset用USBコマンド番号の設定 */
		DucUsbCmdNo = CMD_NUM_USB_BUS_RESET;
		/* switch文を抜ける */
		break;
	/* AppNormal-Enumerating state */
	case DFU_ISTATE_APP_NORMAL_ENUMERATING:
	/* AppDetach-Reset state */
	case DFU_ISTATE_APP_DETACH_RESET:
	/* DfuNormal-Enumerating state */
	case DFU_ISTATE_DFU_NORMAL_ENUMERATING:
	/* DfuError-Enumerating state */
	case DFU_ISTATE_DFU_ERROR_ENUMERATING:
		/* switch文を抜ける(何も処理しない) */
		break;
	}
}


/* Data stageで下記を返せるようにする	*/
/* 0x00, 0x10, 0x27, 0x00, 0x04, 0x00, ? */
/* 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 	*/
/* 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 	*/
u8 GetDfuNextState(void);
void data_send_status(void)	/* for get status */
{
	/* iPROではGenControlInRes_DFU_GETSTATUS */
	/* 次ステート */
	u8 DucNextState;

	/* DFU I/Fの次ステートの取得処理 */
	DucNextState = GetDfuNextState();

	l_pucDtSndBufDfu0G8[0] = DucDfuStatus;

	if(ex_down_start_0g8 == 1)
	{
		ex_down_start_0g8 = 2;
		l_pucDtSndBufDfu0G8[1] = 0xB8;	/* 3s*/
		l_pucDtSndBufDfu0G8[2] = 0x0B;	/* 3s*/
		l_pucDtSndBufDfu0G8[3] = 0x00;
	}
	else
	{
		l_pucDtSndBufDfu0G8[1] = 0x00;
		l_pucDtSndBufDfu0G8[2] = 0x00;
		l_pucDtSndBufDfu0G8[3] = 0x00;
	}
	l_pucDtSndBufDfu0G8[4] = DucNextState;	/* download idle or busy */
	l_pucDtSndBufDfu0G8[5] = 0x00;
	l_usSndDataSizeDfu0G8 = GRCOMD_DFU_GETSTATUS_SIZE;
	DucUsbCmdNo = CMD_NUM_DFU_GETSTATUS;

	return;
}

/*
 *  処理概要（機能説明）
 *	　DFU I/Fの次ステートの取得処理。
 *
 *	　DFU I/Fの次内部ステートを取得し、外部ステート値に変換する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		u8 : 次ステート
 */
u8 GetDfuNextState(void)	/* For Get Status *///  
{
	/* 次ステート */
	u8 DucNextState;

	if(ex_dowload_data_recived_0g8 == CMD_NUM_DFU_DNLOAD_END)	// 
	{
		DucNextState = DFU_OSTATE_DFU_DNBUSY;	/* 0x04 */
		return DucNextState;	
	}


	/* DFU I/Fの次内部ステートの値で処理を分岐 */
	switch (DucDfuNxtState) {
	/* AppNormal-Enumerating state */
	case DFU_ISTATE_APP_NORMAL_ENUMERATING:		/* 0x00 */
		/* 次ステートにappIDLEを設定 */
		DucNextState = DFU_OSTATE_APP_IDLE;	/* 0x00 */
		/* switch文を抜ける */
		break;
	/* AppIdle-Not-Detachable state */
	case DFU_ISTATE_APP_IDLE_NOT_DETACHABLE:	/* 0x01 */
		/* 次ステートにappIDLEを設定 */
		DucNextState = DFU_OSTATE_APP_IDLE;	/* 0x00 */
		/* switch文を抜ける */
		break;
	/* AppIdle-Detachable state */
	case DFU_ISTATE_APP_IDLE_DETACHABLE:		/* 0x02 */
		/* 次ステートにappIDLEを設定 */
		DucNextState = DFU_OSTATE_APP_IDLE;	/* 0x00 */
		/* switch文を抜ける */
		break;
	/* AppDetach-Idle state */
	case DFU_ISTATE_APP_DETACH_IDLE:		/* 0x03 */
		/* 次ステートにappDETACHを設定 */
		DucNextState = DFU_OSTATE_APP_DETACH;	/* 0x01 */
		/* switch文を抜ける */
		break;
	/* AppDetach-Reset state */
	case DFU_ISTATE_APP_DETACH_RESET:		/* 0x04 */
		/* 次ステートにappDETACHを設定 */
		DucNextState = DFU_OSTATE_APP_DETACH;	/* 0x01 */
		/* switch文を抜ける */
		break;
	/* DfuNormal-Enumerating state */
	case DFU_ISTATE_DFU_NORMAL_ENUMERATING:			/* 0x05 */
		/* 次ステートにdfuIDLEを設定 */
		DucNextState = DFU_OSTATE_DFU_IDLE;		/* 0x02 */
		/* switch文を抜ける */
		break;
	/* DfuIdle state */
	case DFU_ISTATE_DFU_IDLE:			/* 0x06 */
		/* 次ステートにdfuIDLEを設定 */
		DucNextState = DFU_OSTATE_DFU_IDLE;		/* 0x02 */
		/* switch文を抜ける */
		break;
	/* DfuFirstBk-Buffering state */
//	case DFU_ISTATE_DFU_FIRST_BK_BUFFERING:
//		/* 次ステートにdfuDNLOAD-SYNCを設定 */
//		DucNextState = DFU_OSTATE_DFU_DNLOAD_SYNC;
//		/* switch文を抜ける */
//		break;
	/* DfuWait-GetStatus1 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS1:		/* 0x08 */
		/* 次ステートにdfuDNLOAD-SYNCを設定 */
		DucNextState = DFU_OSTATE_DFU_DNLOAD_SYNC;	/* 0x03 */
		/* switch文を抜ける */
		break;
	/* DfuWait-PollTimeout state */	
	case DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT:		/* 0x09 */
		/* 次ステートにdfuDNBUSYを設定 */
		DucNextState = DFU_OSTATE_DFU_DNBUSY;	/* 0x04 */
		/* switch文を抜ける */
		break;
	/* DfuWait-GetStatus2 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS2:	/* 0x0A */
		/* 次ステートにdfuDNLOAD-SYNCを設定 */
		DucNextState = DFU_OSTATE_DFU_DNLOAD_SYNC;	/* 0x03 */
		/* switch文を抜ける */
		break;
	/* DfuDnload-Idle state */
	case DFU_ISTATE_DFU_DNLOAD_IDLE:	/* 0x0B */
		/* 次ステートにdfuDNLOAD-IDLEを設定 */
		DucNextState = DFU_OSTATE_DFU_DNLOAD_IDLE;	/* 0x05 */
		/* switch文を抜ける */
		break;
	/* DfuWriting-Buffering state */
	case DFU_ISTATE_DFU_WRITING_BUFFERING: 	/* 0x0C */
		/* 次ステートにdfuDNLOAD-SYNCを設定 */
		DucNextState = DFU_OSTATE_DFU_DNLOAD_SYNC;	/* 0x03 */
		/* switch文を抜ける */
		break;
	/* DfuLastBk-Writing state */
	case DFU_ISTATE_DFU_LAST_BK_WRITING:		/* 0x0D */
		/* 次ステートにdfuMANIFEST-SYNCを設定 */
		DucNextState = DFU_OSTATE_DFU_MANIFEST_SYNC;	/* 0x06 */
		/* switch文を抜ける */
		break;
	/* DfuManifest-Sync state */
	case DFU_ISTATE_DFU_MANIFEST_SYNC: 	/* 0x0E */
		/* 次ステートにdfuMANIFEST-SYNCを設定 */
		DucNextState = DFU_OSTATE_DFU_MANIFEST_SYNC;	/* 0x06 */
		/* switch文を抜ける */
		break;
	/* DfuManifest state */
	case DFU_ISTATE_DFU_MANIFEST:		/* 0x0F */
		/* 次ステートにdfuMANIFESTを設定 */
		DucNextState = DFU_OSTATE_DFU_MANIFEST;	/* 0x07 */
		/* switch文を抜ける */
		break;
	/* DfuManifest-Wait-Reset state */
	case DFU_ISTATE_DFU_MANIFEST_WAIT_RESET:		/* 0x10 */
		/* 次ステートにdfuMANIFEST-WAIT-RESETを設定 */
		DucNextState = DFU_OSTATE_DFU_MANIFEST_WAIT_RESET;	/* 0x08 */
		/* switch文を抜ける */
		break;
	/* DfuUplaod-Idle state */
	case DFU_ISTATE_DFU_UPLOAD_IDLE:		/* 0x11 */
		/* 次ステートにdfuUPLOAD-IDLEを設定 */
		DucNextState = DFU_OSTATE_DFU_UPLOAD_IDLE;	/* 0x09 */
		/* switch文を抜ける */
		break;
	/* DfuError state */
	case DFU_ISTATE_DFU_ERROR:		/* 0x12 */
		/* 次ステートにdfuERRORを設定 */
		DucNextState = DFU_OSTATE_DFU_ERROR;	/* 0x0A */
		/* switch文を抜ける */
		break;
	/* DfuError-Enumerating state */
	case DFU_ISTATE_DFU_ERROR_ENUMERATING:		/* 0x13 */
		/* 次ステートにdfuERRORを設定 */
		DucNextState = DFU_OSTATE_DFU_ERROR;	/* 0x0A */
		/* switch文を抜ける */
		break;
	}

	/* 次ステート値を返す */
	return DucNextState;
}


u8 GetDfuCurState(void);
void data_send_state (void) /* for get state */
{
	u8 DucCurState;

	/* DFU I/Fの現ステートの取得処理 */
	DucCurState = GetDfuCurState();
	l_pucDtSndBufDfu0G8[0] = DucCurState;
	l_usSndDataSizeDfu0G8 = GRCOMD_DFU_GETSTATE_SIZE;

	return;
}

/*
 *  処理概要（機能説明）
 *	　DFU I/Fの現ステートの取得処理。
 *
 *	　DFU I/Fの現内部ステートを取得し、外部ステート値に変換する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		u8 : 現ステート
 */
u8 GetDfuCurState(void)	/* For Get State *///  
{
	/* 現ステート */
	u8 DucCurState;

	/* DFU I/Fの現内部ステートの値で処理を分岐 */
	switch (DucDfuCurState) {
	/* AppNormal-Enumerating state */
	case DFU_ISTATE_APP_NORMAL_ENUMERATING:		/* 0x00 */
		/* 現ステートにappIDLEを設定 */
		DucCurState = DFU_OSTATE_APP_IDLE;		/* 0x00 */
		/* switch文を抜ける */
		break;
	/* AppIdle-Not-Detachable state */
	case DFU_ISTATE_APP_IDLE_NOT_DETACHABLE:	/* 0x01 */
		/* 現ステートにappIDLEを設定 */
		DucCurState = DFU_OSTATE_APP_IDLE;		/* 0x00 */
		/* switch文を抜ける */
		break;
	/* AppIdle-Detachable state */
	case DFU_ISTATE_APP_IDLE_DETACHABLE:	/* 0x02 */
		/* 現ステートにappIDLEを設定 */
		DucCurState = DFU_OSTATE_APP_IDLE;	/* 0x00 */
		/* switch文を抜ける */
		break;
	/* AppDetach-Idle state */
	case DFU_ISTATE_APP_DETACH_IDLE:	/* 0x03 */
		/* 現ステートにappDETACHを設定 */
		DucCurState = DFU_OSTATE_APP_DETACH;	/* 0x01 */
		/* switch文を抜ける */
		break;
	/* AppDetach-Reset state */
	case DFU_ISTATE_APP_DETACH_RESET:	/* 0x04 */
		/* 現ステートにappDETACHを設定 */
		DucCurState = DFU_OSTATE_APP_DETACH;	/* 0x01 */
		/* switch文を抜ける */
		break;
	/* DfuNormal-Enumerating state */
	case DFU_ISTATE_DFU_NORMAL_ENUMERATING:	/* 0x05 */
		/* 現ステートにdfuIDLEを設定 */
		DucCurState = DFU_OSTATE_DFU_IDLE;	/* 0x02 */
		/* switch文を抜ける */
		break;
	/* DfuIdle state */
	case DFU_ISTATE_DFU_IDLE:	/* 0x06 */
		/* 現ステートにdfuIDLEを設定 */
		DucCurState = DFU_OSTATE_DFU_IDLE;	/* 0x02 */
		/* switch文を抜ける */
		break;
	/* DfuFirstBk-Buffering state */
//	case DFU_ISTATE_DFU_FIRST_BK_BUFFERING:
//		/* 現ステートにdfuDNLOAD-SYNCを設定 */
//		DucCurState = DFU_OSTATE_DFU_DNLOAD_SYNC;
//		/* switch文を抜ける */
//		break;
	/* DfuWait-GetStatus1 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS1:	/* 0x08 */
		/* 現ステートにdfuDNLOAD-SYNCを設定 */
		DucCurState = DFU_OSTATE_DFU_DNLOAD_SYNC;	/* 0x03 */
		/* switch文を抜ける */
		break;
	/* DfuWait-PollTimeout state */
	case DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT:	/* 0x09 */
		/* 現ステートにdfuDNBUSYを設定 */
		DucCurState = DFU_OSTATE_DFU_DNBUSY;	/* 0x04 */
		/* switch文を抜ける */
		break;
	/* DfuWait-GetStatus2 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS2:	/* 0x0A */
		/* 現ステートにdfuDNLOAD-SYNCを設定 */
		DucCurState = DFU_OSTATE_DFU_DNLOAD_SYNC;	/* 0x03 */
		/* switch文を抜ける */
		break;
	/* DfuDnload-Idle state */
	case DFU_ISTATE_DFU_DNLOAD_IDLE:	/* 0x0B */
		/* 現ステートにdfuDNLOAD-IDLEを設定 */
		DucCurState = DFU_OSTATE_DFU_DNLOAD_IDLE;	/* 0x05 */
		/* switch文を抜ける */
		break;
	/* DfuWriting-Buffering state */
	case DFU_ISTATE_DFU_WRITING_BUFFERING:	/* 0x0C */
		/* 現ステートにdfuDNLOAD-SYNCを設定 */
		DucCurState = DFU_OSTATE_DFU_DNLOAD_SYNC;		/* 0x03 */
		/* switch文を抜ける */
		break;
	/* DfuLastBk-Writing state */
	case DFU_ISTATE_DFU_LAST_BK_WRITING:		/* 0x0D */
		/* 現ステートにdfuMANIFEST-SYNCを設定 */
		DucCurState = DFU_OSTATE_DFU_MANIFEST_SYNC;	/* 0x06 */
		/* switch文を抜ける */
		break;
	/* DfuManifest-Sync state */
	case DFU_ISTATE_DFU_MANIFEST_SYNC:	/* 0x0E */
		/* 現ステートにdfuMANIFEST-SYNCを設定 */
		DucCurState = DFU_OSTATE_DFU_MANIFEST_SYNC;	/* 0x06 */
		/* switch文を抜ける */
		break;
	/* DfuManifest state */
	case DFU_ISTATE_DFU_MANIFEST:	/* 0x0F */
		/* 現ステートにdfuMANIFESTを設定 */
		DucCurState = DFU_OSTATE_DFU_MANIFEST;	/* 0x07 */
		/* switch文を抜ける */
		break;
	/* DfuManifest-Wait-Reset state */
	case DFU_ISTATE_DFU_MANIFEST_WAIT_RESET:	/* 0x10 */
		/* 現ステートにdfuMANIFEST-WAIT-RESETを設定 */
		DucCurState = DFU_OSTATE_DFU_MANIFEST_WAIT_RESET;	/* 0x08 */
		/* switch文を抜ける */
		break;
	/* DfuUplaod-Idle state */
	case DFU_ISTATE_DFU_UPLOAD_IDLE:	/* 0x11 */
		/* 現ステートにdfuUPLOAD-IDLEを設定 */
		DucCurState = DFU_OSTATE_DFU_UPLOAD_IDLE;	/* 0x09 */
		/* switch文を抜ける */
		break;
	/* DfuError state */
	case DFU_ISTATE_DFU_ERROR:	/* 0x12 */
		/* 現ステートにdfuERRORを設定 */
		DucCurState = DFU_OSTATE_DFU_ERROR;		/* 0x0A */
		/* switch文を抜ける */
		break;
	/* DfuError-Enumerating state */
	case DFU_ISTATE_DFU_ERROR_ENUMERATING:	/* 0x13 */
		/* 現ステートにdfuERRORを設定 */
		DucCurState = DFU_OSTATE_DFU_ERROR;		/* 0x0A */
		/* switch文を抜ける */
		break;
	}

	/* 現ステート値を返す */
	return DucCurState;
}





//static void data_send_upload(void * pUsbGadgetObj, usbSetupPkt_t * setup, u16 req_length)	/* for upload *//* iPRO GenControlInRes_DFU_UPLOAD */
void data_send_upload(u16 wValue, u16 req_length)	/* for upload *//* iPRO GenControlInRes_DFU_UPLOAD */
{

	DucUsbCmdNo = CMD_NUM_DFU_UPLOAD; //2022-07-15

	/* DFU I/F現内部ステートがDfuIdleの場合？ */
	if (DucDfuCurState == DFU_ISTATE_DFU_IDLE)
	{
		#if 1 //2022-09-29
	    //ループ待ちでステータスをかえても間に合わないのでここで変える		
		//従来はProcDfuIdleStateの処理
		DucUsbCmdNo = CMD_NUM_NONE;
		/* DFU I/F現内部ステートをDfuUpload-Idleステートに設定 */
		DucDfuCurState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */
		/* DFU I/F次内部ステートをDfuUpload-Idleステートに設定 */
		DucDfuNxtState = DFU_ISTATE_DFU_UPLOAD_IDLE;/* 0x11 *//* 17 *//* DfuUplaod-Idle state *//* DFU_ISTATE_DFU_IDLEでDFU_UPLOADリクエスト受信時 */
	
		#endif

		/* uploadの開始の1回のみ*/
		/* 読み込みアドレスを開始アドレスで初期化 */
		PucReadAddr = (u8*)ROM_ALIAS_START_ADDRESS;
	}

	/* 指定サイズの読み込み */
	DusUpReqLen = req_length;	/* 2020-10-22 */

	/* アップロード用読み込みアドレスが最終アドレスを超える場合？ */
	if (PucReadAddr > (u8 *)ROM_ALIAS_END_ADDRESS)
 	{
		/* 実際のアップロードデータ長を0に設定 */
		/* 最後 送るデータなし*/
		DusUpActualLen = 0;
	}
	/* 指定サイズを読み込むことで、最終アドレスを超える場合？ */

	else if (PucReadAddr + DusUpReqLen > (u8 *)ROM_ALIAS_END_ADDRESS)
	{
		/* 実際のアップロードデータ長を残りの読み込み可能サイズに変更 */
		/* 最後の1つ前、残りのデータを送る*/
		DusUpActualLen = (u8 *)ROM_ALIAS_END_ADDRESS - PucReadAddr + 1;	/* 1追加しないと1バイト足りない*/

	}
	/* 指定サイズを読み込むことで、最終アドレスを超えない場合？ */
	else
	{
		/* 実際のアップロードデータ長を指定サイズで設定 */
		DusUpActualLen = DusUpReqLen;
	}

	if( DusUpActualLen != 0 )
	{
		l_usSndDataSizeDfu0G8 = DusUpActualLen;
		GRUSB_COMD_DFU_Set_GetEncapsulatedResponse2(l_usSndDataSizeDfu0G8, PucReadAddr);
	}
	else
	{
	/* 最後はNo dataで終了*/
		l_usSndDataSizeDfu0G8 = 0;
		GRUSB_COMD_DFU_Set_GetEncapsulatedResponse2(l_usSndDataSizeDfu0G8, l_pucDtSndBufDfu0G8);

		//ここで遷移させないと、ステータス遷移が間に合わない 
		// 0 data送信後に
		// 0xA1, 0x05, 0x00... DFU GETSTATE Requestに
		// DFU IDLEの0x02を返すのが理想だが、
		// ラインタスクのループで遷移させる前に、GETSTATE Requestが来る為
		// DFU Upload Idlingの0x09を返す事になり、シミュレータがエラーを判断
		if (DusUpActualLen < DusUpReqLen) 
		{

			DucUsbCmdNo = CMD_NUM_NONE;

			/* DFU I/F現内部ステートをDfuIdleステートに設定 */
			DucDfuCurState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

			/* DFU I/F次内部ステートをDfuIdleステートに設定 */
			DucDfuNxtState = DFU_ISTATE_DFU_IDLE;/* 0x06*/

			return;
		}

	}

	/* アップロード用読み込みアドレスを実際の読み込みサイズ分だけ進める */
	PucReadAddr += DusUpActualLen;

	return;

}


/*
 *  処理概要（機能説明）
 *	　SET_CONFIGURATIONリクエスト通知処理。
 *
 *	　DFU I/F現内部ステートに応じて紙幣鑑別処理部へのSET_CONFIGURATIONリクエストの通知を決定する。
 *
 *  入力データ
 *		なし
 *
 *  出力データ
 *		なし
 *
 *  関数値
 *		なし
 */
void SignalSET_CONFIGURATIONCmd(void)
{
	/* GSA-USBフラグをONに設定 */
//	DucGsaUsbFlag = ON;

	/* DFU I/F現内部ステートに応じて処理を分岐 */
	switch (DucDfuCurState) {
	/* AppNormal-Enumerating state */
	case DFU_ISTATE_APP_NORMAL_ENUMERATING:
	/* DfuNormal-Enumerating state */
	case DFU_ISTATE_DFU_NORMAL_ENUMERATING:
	/* DfuError-Enumerating state */
	case DFU_ISTATE_DFU_ERROR_ENUMERATING:
		/* SET_CONFIGURATIONリクエスト用USBコマンド番号の設定 */
		DucUsbCmdNo = CMD_NUM_SET_CONFIGURATION;
		/* switch文を抜ける */
		break;
	/* AppIdle-Not-Detachable state */
	case DFU_ISTATE_APP_IDLE_NOT_DETACHABLE:
	/* AppIdle-Detachable state */
	case DFU_ISTATE_APP_IDLE_DETACHABLE:
	/* AppDetach-Idle state */
	case DFU_ISTATE_APP_DETACH_IDLE:
	/* AppDetach-Reset state */
	case DFU_ISTATE_APP_DETACH_RESET:
	/* DfuIdle state */
	case DFU_ISTATE_DFU_IDLE:
	/* DfuFirstBk-Buffering state */
//	case DFU_ISTATE_DFU_FIRST_BK_BUFFERING:
	/* DfuWait-GetStatus1 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS1:
	/* DfuWait-PollTimeout state */
	case DFU_ISTATE_DFU_WAIT_POLL_TIMEOUT:
	/* DfuWait-GetStatus2 state */
	case DFU_ISTATE_DFU_WAIT_GETSTATUS2:
	/* DfuDnload-Idle state */
	case DFU_ISTATE_DFU_DNLOAD_IDLE:
	/* DfuWriting-Buffering state */
	case DFU_ISTATE_DFU_WRITING_BUFFERING:
	/* DfuLastBk-Writing state */
	case DFU_ISTATE_DFU_LAST_BK_WRITING:
	/* DfuManifest-Sync state */
	case DFU_ISTATE_DFU_MANIFEST_SYNC:
	/* DfuManifest state */
	case DFU_ISTATE_DFU_MANIFEST:
	/* DfuManifest-Wait-Reset state */
	case DFU_ISTATE_DFU_MANIFEST_WAIT_RESET:
	/* DfuUpload-Idle state */
	case DFU_ISTATE_DFU_UPLOAD_IDLE:
	/* DfuError state */
	case DFU_ISTATE_DFU_ERROR:
		/* switch文を抜ける(何も処理しない) */
		break;
	}
}


