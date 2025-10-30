/******************************************************************************/
/*! @addtogroup Group2
    @file       rc_download.c
    @brief      
    @date       2024/05/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/

#if defined(UBA_RTQ)
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "hal.h"
#include "hal_uart0.h"
#include "rc_task.h"

#include "rc_operation.h"
#include "if_rc.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#ifdef _ENABLE_JDL
#include "jdl_conf.h"
#endif /* _ENABLE_JDL */


/*********************************************************************//**
 * @brief download start command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_start_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DL_START_REQ;
	_set_head_status();								/* sst1 - sst32	*/

}

/*********************************************************************//**
 * @brief download data command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_data_command(void) //ok
{
	u32 temp;

	pol_time = 20;			/* set polling timer 20msec */

	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x09;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DL_DATA_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	/* exchange endian */
	temp = swap32(ex_rc_dl_offset);
	memcpy((u8 *)&_rc_tx_buff.data[0], (u8 *)&temp, 4);

	/* 通常パケット送信の場合 */
#if 0 //2025-02-02 UBA500に合わせて廃止したいが、現状 0xC8側を使用すると動かないので、暫定でこっちを使用する
	//UBA700RTQは、長浜のCS生産や、JCM-Eのでもソフトなど初期はこっちを使用していた。
	if((u8 *)(Rom2str + ex_rc_dl_offset + 0x50) < (u8 *)Rom2end)
	{
		_rc_tx_buff.length		= 0x5A;					/* length		*/
		memcpy((u8 *)&_rc_tx_buff.data[4], (u8 *)(Rom2str + ex_rc_dl_offset), 0x50);
	}
#else
	//UBA500は昔からこっちを使用していたようだ。
	if((u8 *)(Rom2str + ex_rc_dl_offset + 0xC8) < (u8 *)Rom2end)
	{
		_rc_tx_buff.length		= 0xD2;					/* length		*/
		memcpy((u8 *)&_rc_tx_buff.data[4], (u8 *)(Rom2str + ex_rc_dl_offset), 0xC8);
	}
#endif
	/* 最終パケット送信の場合 */
	else
	{
		_rc_tx_buff.length		= (u8)(Rom2end - (Rom2str + ex_rc_dl_offset) + 0x0A);            /* Lenght      */
		memcpy((u8 *)&_rc_tx_buff.data[4], (u8 *)(Rom2str + ex_rc_dl_offset), (u8)(Rom2end - (Rom2str + ex_rc_dl_offset)));
	}

	uart_send_rc(&_rc_tx_buff);
}

/*********************************************************************//**
 * @brief download check command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_check_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DL_CHECK_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief download end command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_end_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DL_END_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	pol_time = 100;			/* set polling timer 100msec */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_start_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_START_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_DL_START_RSP) */

	Rom2str = (u8 *)RC_DOWNLOAD_DATA_START;		/* 開始アドレス */
	Rom2end = (u8 *)RC_DOWNLOAD_DATA_END;		/* 終了アドレス */

	ex_rc_dl_offset = 0;						/* オフセット	*/

	ex_rc_download_ready = 0;

}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_data_response(void)
{
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_check_response(void)
{
	u8 count;
	u8 *ptr;
	int ret;

	memcpy((u8 *)&RecycleSoftInfo.FlashCheckSum[0], (u8 *)&_rc_rx_buff.data[0], 2);

#if 1
	ret = memcmp((u8 *)RC_DOWNLOAD_DATA_SUM, (u8 *)&RecycleSoftInfo.FlashCheckSum[0], 2);
#else
	ret = 0;
#endif 

	if(ret != 0)
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_CHECK_RSP, TMSG_SUB_ALARM, 0, 0, 0);		/* To main_task(TMSG_RC_DL_CHECK_RSP) */
	}
	else
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_CHECK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_DL_CHECK_RSP) */
	}

}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_dl_end_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_END_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_DL_END_RSP) */
}

#endif // UBA_RTQ

