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
 * @file main_task.c
 * @brief メインタスクを格納しています。
 * @date 2018.01.05
 */
/****************************************************************************/
#include <string.h>
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "operation.h"
#include "sub_functions.h"
#include "download.h"

#define EXT
#include "com_ram.c"

#undef EXTERN
#define EXTERN
#include "main_task.h"

/************************** Function Prototypes ******************************/
void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _main_system_error(u8 fatal_err, u8 code);
void operation_main(void);
void operation_main_msg_proc(void);

T_MSG_BASIC ex_main_msg;
/*******************************
        activate task
 *******************************/
void _main_start_cyc(void);
void _main_act_peripheral_task(void);
/****************************************************************/
/**
 * @brief メインタスクエントリー
 */
/****************************************************************/
void main_task(void)
{
	_main_start_cyc();
	_main_act_peripheral_task();
	operation_main();
}

/*********************************************************************//**
 * @brief operation main
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void operation_main(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	if(( (ex_operating_mode & OPERATING_MODE_IF_DOWNLOAD) == OPERATING_MODE_IF_DOWNLOAD )
	|| ( (ex_operating_mode & OPERATING_MODE_IF_DIFF_DOWNLOAD) == OPERATING_MODE_IF_DIFF_DOWNLOAD )
	){
		/* Download from MAIN (ID-003 Download)*/
		_check_all_section_crc(D_OK);
		_main_select_protocol();
		/* wakeup ID_CLINE_TASK */
		act_tsk(ID_CLINE_TASK);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_IF);
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_INITIAL_REQ, ex_operating_mode, TMSG_SUB_SUCCESS, 0, 0);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
	}
	else if(( (ex_operating_mode & OPERATING_MODE_USB_DOWNLOAD) == OPERATING_MODE_USB_DOWNLOAD )
	 || ( (ex_operating_mode & OPERATING_MODE_FILE_DOWNLOAD) == OPERATING_MODE_FILE_DOWNLOAD ))
	{
		/* Download from MAIN (Front USB Download(Tool Suite))*/
		_check_all_section_crc(D_OK);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_USB);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
	}
	else if((ex_operating_mode & OPERATING_MODE_SUBLINE_DOWNLOAD) == OPERATING_MODE_SUBLINE_DOWNLOAD )
	{
		/* Download from MAIN */
		_check_all_section_crc(D_OK);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_SUBLINE_USB);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
	}
	else
	{
		/* Download by MAIN NG Bootからジャンプ*/
		_check_all_section_crc(D_NG);
		_main_select_protocol();
		/* wakeup ID_CLINE_TASK */
		act_tsk(ID_CLINE_TASK);
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_INITIAL_REQ, ex_operating_mode, TMSG_SUB_SUCCESS, 0, 0);
		_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_IF_AREA, ex_operating_mode, 0, 0);

	}

	while(1)
	{
		ercd = trcv_mbx(ID_MAIN_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		if (ercd == E_OK)
		{
			memcpy(&ex_main_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(ex_main_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
					_main_system_error(1, 3);
			}
			operation_main_msg_proc();
		}
	}
}


/*********************************************************************//**
 * @brief main_task message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void operation_main_msg_proc(void)
{
	switch (ex_main_task_mode1)
	{
	case MODE1_DOWNLOAD:
		download_msg_proc();
		break;
	case MODE1_ALARM:
		alarm_msg_proc();
		break;
	default:
		/* system error ? */
		_main_system_error(0, 4);
		break;
	}
}
/* EOF */
