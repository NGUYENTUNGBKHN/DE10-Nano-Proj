/******************************************************************************/
/*! @addtogroup Group2
    @file       id0g8.c
    @brief      id0g8 process
    @date       2013/03/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2013/03/22 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#include <string.h>
#include "kernel.h"
#include "itron.h"
#include "common.h"
#include "custom.h"
#include "id0g8.h"
#include "memorymap.h"
#include "sub_functions.h"
#include "download.h"
#include "com_dfu_def.h"
#include "com_dfu_id0g8.h"

#include "IF_ID0G8.def"	//2022-08-01


#define EXT
#include "com_ram.c"

/************************** PRIVATE DEFINITIONS *************************/
/************************** PRIVATE VARIABLES *************************/
//ID0G8_CMD_BUFFER _id0g8_rx_buff;
//ID0G8_CMD_BUFFER _id0g8_tx_buff;
//2022-08-01	static u8 l_aulID0G8_rx_buff[COM_DFU_BUF_SIZE];
u8 l_aulID0G8_rx_buff[COM_DFU_BUF_SIZE];

static u8 l_aulID0G8_tx_buff[COM_DFU_BUF_SIZE];
static u16 l_ausID0G8_tx_size;
static u8 s_abnomal_code; /**< @brief Abnormal data */


/************************** PRIVATE FUNCTIONS *************************/
static void _init_variable( void );
void _id0g8_initial_msg_proc(void);
void _id0g8_msg_proc(void);

void _id0g8_cmd_proc(void);

/* Request */
//2022-08-01 void _id0g8_getstatus_cmd_proc(void);
//2022-08-01 void _id0g8_clrstatus_cmd_proc(void);
//2022-08-01 void _id0g8_getstate_cmd_proc(void);
//2022-08-01 void _id0g8_abort_cmd_proc(void);

/* Operation command */
void _id0g8_download_start_cmd_proc(void);
void _id0g8_download_end_confirm_cmd_proc(void);
void _id0g8_download_baudrate_cmd_proc(void);
void _id0g8_diff_download_start_cmd_proc(void);
void _id0g8_download_data_cmd_proc(void);
void _id0g8_ack_proc(void);

/* Response */
void _id0g8_send_host_ack(void);
void _id0g8_send_host_invalid(void);

/* MAIN status message */
void _id0g8_status_info_msg_proc(void);
/* UART1 callback message */
void _id0g8_callback_msg_proc(void);
void _id0g8_callback_receive(void);
void _id0g8_callback_empty(void);

void _id0g8_write_flash_rsp_msg_proc(void);
void _id0g8_download_calc_crc_rsp_msg_proc(void);
void _id0g8_download_rsp_msg_proc(void);


/************************** EXTERN VARIABLES *************************/
T_MSG_BASIC cline_msg;

/************************** EXTERN FUNCTIONS *************************/
extern void _cline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _cline_system_error(u8 fatal_err, u8 code);


#if 1//#if defined(ID0G8_BOOTIF)
extern void Fid0g8_download_init(void);
extern void fid0g8_download_main(void);
#endif

/*******************************
        line_task
 *******************************/
u8 id0g8_main(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 rtn;


	ex_usb_dfu_disconect_0g8 = 0;
	_id0g8_initial_msg_proc();

	#if 1//#if defined(ID0G8_BOOTIF)
	Fid0g8_download_init();		//2022-08-01
	DuwTotalDnloadBytes = 0;	//2022-08-01
	#endif

	while (1)
	{

		if(DuwTotalDnloadBytes == 0)	//2022-08-01
		{
			ercd = trcv_mbx(ID_CLINE_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		}
		else
		{
			ercd = trcv_mbx(ID_CLINE_MBX, (T_MSG **)&tmsg_pt, 1);
		}

		if (ercd == E_OK)
		{
		/* Receiving task message */
			memcpy(&cline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(cline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_cline_system_error(1, 3);
			}
 			_id0g8_msg_proc();
		}

		fid0g8_download_main();	//2022-08-01

#if 0
		// if download start flag set, TX disabled and UART not busy
		if( (ex_cline_download_control.is_end_sent)
		 && (UART_is_txd_active()  == 0) )
		{
	    	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SOFT_RESET_REQ, 0, 0, 0, 0);
			ex_cline_status_tbl.line_task_mode = ID0G8_MODE_QUIT;
		}
		if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_QUIT)
		{
			break;
		}
#endif
	}
	return 0;
}


#if 0
/*--------------------------*/
/*	Set abnormal status		*/
/*--------------------------*/
static void id_abn_set( u16 ercd)
{
	s_abnomal_code = ercd;
}
#endif
void _id0g8_initial_msg_proc(void)
{
#if 1
    INT     iStat;

    /* 受信データバッファ初期化 */
	memset(l_aulID0G8_rx_buff, 0x00, COM_DFU_BUF_SIZE);
    /* 送信データバッファ初期化 */
	memset(l_aulID0G8_tx_buff, 0x00, COM_DFU_BUF_SIZE);
    /* 送信データサイズ初期化 */
	l_ausID0G8_tx_size = 0;

	/* --------------------------------------------------------------------- */
	/* Start id-0G8 communication                                            */
	/* --------------------------------------------------------------------- */
	/* Initialize USB1 (DFU) */
    /* H/W init */
	iStat = com_dfu_0g8_Init2(COM_DFU_BUF_SIZE, l_aulID0G8_rx_buff, COM_DFU_BUF_SIZE, l_aulID0G8_tx_buff);
	/* wakeup ID_USB1_CB_TASK */
	act_tsk(ID_USB1_CB_TASK);
#else
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 wait_init = 1;
	/* clear abnormal code */
	s_abnomal_code = 0;

	/* Wait TMSG_CLINE_INITIAL_REQ */
	while (wait_init)
	{
		ercd = rcv_mbx(ID_CLINE_MBX, (T_MSG **)&tmsg_pt);
		if (ercd == E_OK)
		{
			memcpy(&cline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			memset(tmsg_pt, 0, sizeof(T_MSG_BASIC));
			if ((rel_mpf(cline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_cline_system_error(1, 4);
			}
			if ((cline_msg.tmsg_code == TMSG_CLINE_INITIAL_REQ))
			{
				if ((cline_msg.arg2 != TMSG_SUB_SUCCESS) && (cline_msg.arg2 != TMSG_SUB_ALARM))
				{
					/* system error ? */
					_cline_system_error(0, 5);
				}
				else
				{
					wait_init = 0;
					if (cline_msg.arg1 & OPERATING_MODE_IF_DOWNLOAD)
					{
					    /* ダウンロード関係変数の初期化 */
					    _init_variable();
					    /* ダウンロード開始要求フラグ     */
						ex_cline_download_control.is_start_received = DL_START_NORMAL;
					    /* Download Mode */
						ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD;
					}
					else if(cline_msg.arg1 & OPERATING_MODE_IF_DIFF_DOWNLOAD)
					{
					    /* ダウンロード関係変数の初期化 */
					    _init_variable();
					    /* ダウンロード開始要求フラグ     */
						ex_cline_download_control.is_start_received = DL_START_DIFFERENTIAL;
					    /* Download Mode */
						ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD;

					}
					else
					{
					    /* ダウンロード関係変数の初期化 */
					    _init_variable();
					    /* ダウンロード開始要求フラグ     */
						ex_cline_download_control.is_start_received = DL_START_NOT;
					/* Wait Command Mode */
						ex_cline_status_tbl.line_task_mode = ID0G8_MODE_WAIT_REQ;
						id_abn_set(ALARM_CODE_IF_AREA);
					}
				}
			}
		}
	}
#endif
}


#if 0
/*------------------------------------------------------*/
/*  変数の初期化処理                                                                                         */
/*  <引数> なし                                                                                                   */
/*  <返値> なし                                                                                                   */
/*------------------------------------------------------*/
void static _init_variable( void )
{
	// 変数初期化
	memset(&_id0g8_rx_buff, 0, sizeof(_id0g8_rx_buff));
	memset(&_id0g8_tx_buff, 0, sizeof(_id0g8_tx_buff));
	memset(&ex_cline_download_control, 0, sizeof(ex_cline_download_control));
    init_download_variable();
    ex_cline_download_control.baudrate = 9600;

    return;
}
#endif
/*********************************************************************//**
 * @brief TMSG
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_msg_proc(void)
{
	switch (cline_msg.tmsg_code)
	{
	case TMSG_USB1CB_CALLBACK_INFO:
		_id0g8_callback_msg_proc();
		break;
	case TMSG_CLINE_STATUS_INFO:
		_id0g8_status_info_msg_proc();
		break;
	case TMSG_CLINE_DOWNLOAD_RSP:
		_id0g8_download_rsp_msg_proc();
		break;
	case TMSG_CLINE_WRITE_FLASH_RSP:
		_id0g8_write_flash_rsp_msg_proc();
		break;
	case TMSG_CLINE_DOWNLOAD_CALC_CRC_RSP:
		_id0g8_download_calc_crc_rsp_msg_proc();
		break;
	default:					/* other */
		break;
	}
}

/*********************************************************************//**
 * @brief Get Status request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0	//2022-08-01 これらのコマンドは、com_dfu_id08g.cファイルの中で、個別にUSBドライバ側からコールバックで呼ばれて対応するので、
			//この処理が走るので、Downloadデータ受信の時のみ

void _id0g8_getstatus_cmd_proc(void)
{

}


/*********************************************************************//**
 * @brief Clear Status request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_clrstatus_cmd_proc(void)
{
}

/*********************************************************************//**
 * @brief Get State request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_getstate_cmd_proc(void)
{
}



/*********************************************************************//**
 * @brief Abort receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_abort_cmd_proc(void)
{
}


/*********************************************************************//**
 * @brief response status (ID0G8_STS_DOWNLOAD_END)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stsreq_download_error(void)
{
	_id0g8_tx_buff.length = 4 + 4;
	_id0g8_tx_buff.cmd = ID0G8_STS_DOWNLOAD_END;
	_id0g8_tx_buff.data[0] = ID0G8_DOWNLOAD_FAILURE;
	_id0g8_tx_buff.data[1] = (u8)(ex_cline_download_control.file_crc >> 8);
	_id0g8_tx_buff.data[2] = (u8)(ex_cline_download_control.file_crc & 0xFF);

	uart_send_id0g8(&_id0g8_tx_buff);
}
/*********************************************************************//**
 * @brief response status (ID0G8_STS_DOWNLOAD_END)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stsreq_download_end(void)
{
	_id0g8_tx_buff.length = 4 + 4;
	_id0g8_tx_buff.cmd = ID0G8_STS_DOWNLOAD_END;
	_id0g8_tx_buff.data[0] = ID0G8_DOWNLOAD_SUCCESS;
	_id0g8_tx_buff.data[1] = (u8)(ex_cline_download_control.file_crc >> 8);
	_id0g8_tx_buff.data[2] = (u8)(ex_cline_download_control.file_crc & 0xFF);

	uart_send_id0g8(&_id0g8_tx_buff);

    /* wait */
    while(UART_is_txd_active())
    {
    	OSW_TSK_sleep(1);
    };
	ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD_WAIT_RESET;
	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SOFT_RESET_REQ, 0, 0, 0, 0);
}
/*********************************************************************//**
 * @brief response status (ID0G8_STS_DOWNLOAD)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stsreq_download(void)
{
	if(ex_cline_download_control.busy)
	{
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_STS_DOWNLOAD;
		_id0g8_tx_buff.data[0] = ID0G8_DOWNLOAD_STS_BUSY;
	}
	else if(ex_cline_download_control.is_start_received == DL_START_NORMAL)
	{
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_STS_DOWNLOAD;
		_id0g8_tx_buff.data[0] = ID0G8_DOWNLOAD_STS_READY;
	}
	else if(ex_cline_download_control.is_start_received == DL_START_DIFFERENTIAL)
	{
		_id0g8_tx_buff.length = 6 + 4;
		_id0g8_tx_buff.cmd = ID0G8_STS_DOWNLOAD;
		_id0g8_tx_buff.data[0] = ID0G8_DOWNLOAD_STS_READY_DIFFERENTIAL;
		_id0g8_tx_buff.data[1] = (u8)((s_dl_file_offset & 0xFF)            );
		_id0g8_tx_buff.data[2] = (u8)((s_dl_file_offset & 0xFF00)     >> 8 );
		_id0g8_tx_buff.data[3] = (u8)((s_dl_file_offset & 0xFF0000)   >> 16);
		_id0g8_tx_buff.data[4] = (u8)((s_dl_file_offset & 0xFF000000) >> 24);
	}
	else
	{
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_STS_DOWNLOAD;
		_id0g8_tx_buff.data[0] = ID0G8_DOWNLOAD_STS_READY;
	}

	uart_send_id0g8(&_id0g8_tx_buff);
}
#endif

/*********************************************************************//**
 * @brief UART1 callback message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_callback_msg_proc(void)
{
	switch (cline_msg.arg1)
	{
	case TMSG_SUB_RECEIVE:
		_id0g8_callback_receive();
		break;
	case TMSG_SUB_EMPTY:
		_id0g8_callback_empty();
		break;
	default:					/* other */
		break;
	}
}
/*********************************************************************//**
 * @brief UART1 callback TXD message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_callback_empty(void)
{
}
/*********************************************************************//**
 * @brief UART1 callback RXD message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_callback_receive(void)
{
	#if 0
	u32 rcvSize;
	rcvSize = com_dfu_0g8_listen();
	if (rcvSize != 0)
	{
	/* Receiving command */
		_id0g8_cmd_proc();
	}
	#endif
}



/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
void _id0g8_cmd_proc(void)
{
// これらのコマンドは、com_dfu_id08g.cファイルの中で、個別にUSBドライバ側からコールバックで呼ばれて対応するので、	
#if 0 //2022-08-01	
//2022-08-01	DucUsbCmdNo = CMD_NUM_DFU_DNLOAD_END;
	switch (l_aulID0G8_rx_buff[0])
	{
	case GRUSB_DFU_DETACH:
		break;
	case GRUSB_DFU_DNLOAD:
		break;
	case GRUSB_DFU_UPLOAD:
		break;
	case GRUSB_DFU_GETSTATUS:
		_id0g8_getstatus_cmd_proc();
		break;
	case GRUSB_DFU_CLRSTATUS:
		_id0g8_clrstatus_cmd_proc();
		break;
	case GRUSB_DFU_GETSTATE:
		_id0g8_getstate_cmd_proc();
		break;
	case GRUSB_DFU_ABORT:
		_id0g8_abort_cmd_proc();
		break;
	default:
		break;
	}
#endif	
}

/*********************************************************************//**
 * @brief Status infomation message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_status_info_msg_proc(void)
{
}
#if 0
/*********************************************************************//**
 * @brief Download Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_download_start_cmd_proc(void)
{
    /* ダウンロード関係変数の初期化 */
    _init_variable();
    /* ダウンロード開始要求フラグ     */
	ex_cline_download_control.is_start_received = DL_START_NORMAL;


    /* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(ACK) */
    _id0g8_send_host_ack();

    if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_WAIT_REQ)
    {
    	ex_cline_status_tbl.line_task_mode = ID0G8_MODE_WAIT_DOWNLOAD_START_RSP;
    	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DOWNLOAD_REQ, 0, 0, 0, 0);
    }
}
/*********************************************************************//**
 * @brief Download Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_diff_download_start_cmd_proc(void)
{
    /* ダウンロード関係変数の初期化 */
    _init_variable();
    /* ダウンロード開始要求フラグ     */
	ex_cline_download_control.is_start_received = DL_START_DIFFERENTIAL;

    /* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(ACK) */
    _id0g8_send_host_ack();

    if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_WAIT_REQ)
    {
    	ex_cline_status_tbl.line_task_mode = ID0G8_MODE_WAIT_DOWNLOAD_START_RSP;
    	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DOWNLOAD_REQ, 0, 0, 0, 0);
    }
}
/*********************************************************************//**
 * @brief Download Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_download_end_confirm_cmd_proc(void)
{
    if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DOWNLOAD)
    {
		/*********************/
		/* BUSY状態にする                       */
		/*********************/
		ex_cline_download_control.busy = TRUE;
		ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD_CALC_CRC;

		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DOWNLOAD_CALC_CRC_REQ, 0, 0, 0, 0);
	    /* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(ACK) */
	    _id0g8_send_host_ack();
    }
    else
    {
		_id0g8_send_host_invalid();
    }
}
void _id0g8_get_baudrate_command(void)
{
	switch( ex_cline_download_control.baudrate )
	{
	case 9600:
		/* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(9600bps) */
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_CMD_BAUDRATE_REQUEST;
		_id0g8_tx_buff.data[0] = 0x01;
		uart_send_id0g8(&_id0g8_tx_buff);
		break;
	case 19200:
		/* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(19200bps) */
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_CMD_BAUDRATE_REQUEST;
		_id0g8_tx_buff.data[0] = 0x02;
		uart_send_id0g8(&_id0g8_tx_buff);
		break;
	case 38400:
		/* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(38400bps) */
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_CMD_BAUDRATE_REQUEST;
		_id0g8_tx_buff.data[0] = 0x03;
		uart_send_id0g8(&_id0g8_tx_buff);
		break;
	case 57600:
		/* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(57600bps) */
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_CMD_BAUDRATE_REQUEST;
		_id0g8_tx_buff.data[0] = 0x04;
		uart_send_id0g8(&_id0g8_tx_buff);
		break;
	case 115200:
		/* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(115200bps) */
		_id0g8_tx_buff.length = 2 + 4;
		_id0g8_tx_buff.cmd = ID0G8_CMD_BAUDRATE_REQUEST;
		_id0g8_tx_buff.data[0] = 0x05;
		uart_send_id0g8(&_id0g8_tx_buff);
		break;
	default:
		break;
	}
}
void _id0g8_set_baudrate_command(void)
{
    /* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(ACK) */
    _id0g8_send_host_ack();

    /* wait */
    while(UART_is_txd_active())
    {
    	OSW_TSK_sleep(1);
    };

	switch( _id0g8_rx_buff.data[1] )
	{
	/* 通信ﾚｰﾄ : 9600bps   */
	case 0x01:
		ex_cline_download_control.baudrate = 9600;
		break;
	/* 通信ﾚｰﾄ : 19200bps  */
	case 0x02:
		ex_cline_download_control.baudrate = 19200;
		break;
	/* 通信ﾚｰﾄ : 38400bps  */
	case 0x03:
		ex_cline_download_control.baudrate = 38400;
		break;
	/* 通信ﾚｰﾄ : 57600bps  */
	case 0x04:
		ex_cline_download_control.baudrate = 57600;
		break;
	/* 通信ﾚｰﾄ : 115200bps */
	case 0x05:
		ex_cline_download_control.baudrate = 115200;
		break;
	/* 通信ﾚｰﾄ : 9600bps   */
	default:
		ex_cline_download_control.baudrate = 9600;
		break;
	}
	UART_change_baudrate(ex_cline_download_control.baudrate);
}
/*********************************************************************//**
 * @brief Download Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_download_baudrate_cmd_proc(void)
{
    if( _id0g8_rx_buff.data[0] == 0x00 )
    {
        _id0g8_get_baudrate_command();
    }
    /* Setコマンドの場合 */
    else if( _id0g8_rx_buff.data[0] == 0x01 )
    {
        _id0g8_set_baudrate_command();
    }
}
/*********************************************************************//**
 * @brief Download Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_download_data_cmd_proc(void)
{
    if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DOWNLOAD)
    {
        /************************/
        /* Downlaodデータコピー */
        /************************/
        download_copy( &(_id0g8_rx_buff.data[4]), _id0g8_rx_buff.length-9 );

        /*********************/
        /* Downlaodチェック */
        /*********************/
        if(RET_OK == download_data_check())
    	{
            /*********************/
            /* BUSY状態にする                       */
            /*********************/
            ex_cline_download_control.busy = TRUE;
        	ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD_BUSY;

        	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_WRITE_FLASH_REQ, 0, 0, 0, 0);
    	    /* ID0G8ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞ作成(ACK) */
    	    _id0g8_send_host_ack();
    	}
    	else
    	{
    		_id0g8_send_host_invalid();
    	}
    }
    else
    {
		_id0g8_send_host_invalid();
    }
}

/*********************************************************************//**
 * @brief Receive ACK
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_ack_proc(void)
{
}
/*********************************************************************//**
 * @brief Send ACK
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_send_host_ack(void)
{
	_id0g8_tx_buff.length = 1 + 4;
	_id0g8_tx_buff.cmd = ID0G8_CMD_ACK;
	uart_send_id0g8(&_id0g8_tx_buff);
}


/*********************************************************************//**
 * @brief Send Invalid
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_send_host_invalid(void)
{
	_id0g8_tx_buff.length = 1 + 4;
	_id0g8_tx_buff.cmd = ID0G8_CMD_INVALID;
	uart_send_id0g8(&_id0g8_tx_buff);
}
#endif

/*********************************************************************//**
 * @brief Download response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_write_flash_rsp_msg_proc(void)
{
#if 0
    if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DOWNLOAD_BUSY)
    {
        ex_cline_download_control.busy = FALSE;
    	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
    	{
    		// End Write Flash
    		ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD;
    	}
    	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
    	{
    		id_abn_set(ALARM_CODE_DOWNLOAD);
    		// Download request refuse
    		ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD_ERROR;
    	}
    	else
    	{
    		/* system error ? */
    		_cline_system_error(0, 10);
    	}
    }
	else
	{
		/* system error ? */
		_cline_system_error(0, 10);
	}
#endif
}

/*********************************************************************//**
 * @brief Download response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_download_calc_crc_rsp_msg_proc(void)
{
#if 0
    if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DOWNLOAD_CALC_CRC)
    {
        /* Ready状態にする        */
        ex_cline_download_control.busy = FALSE;
    	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
    	{
            /* ダウンロード正常完了状態 */
        	s_dl_end_status = D_DL_COMPLENTION;
            /* ダウンロード正常完了状態 */
        	ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD_COMPLETE;
    	}
    	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
    	{
    		id_abn_set(ALARM_CODE_DOWNLOAD);
            /* ダウンロード失敗状態 */
        	s_dl_end_status = D_DL_FAILURE;
            /* ダウンロード失敗状態 */
        	ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD_COMPLETE_ERROR;
    	}
    	else
    	{
    		/* system error ? */
    		_cline_system_error(0, 10);
    	}
    }
	else
	{
		/* system error ? */
		_cline_system_error(0, 10);
	}
#endif
}

/*********************************************************************//**
 * @brief Download response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_download_rsp_msg_proc(void)
{
#if 0
    if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_WAIT_DOWNLOAD_START_RSP)
    {
    	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
    	{
    		// Start I/F Download
    		ex_cline_status_tbl.line_task_mode = ID0G8_MODE_DOWNLOAD;
    	}
    	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
    	{
    		// Download request refuse
    		ex_cline_status_tbl.line_task_mode = ID0G8_MODE_QUIT;
    	}
    	else
    	{
    		/* system error ? */
    		_cline_system_error(0, 10);
    	}
    }
	else
	{
		/* system error ? */
		_cline_system_error(0, 10);
	}
#endif
}

/* EOF */
