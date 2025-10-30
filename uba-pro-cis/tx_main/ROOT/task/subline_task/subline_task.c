/******************************************************************************/
/*! @addtogroup Main
    @file       subline_task.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
// "PCからUSB経由でテストモード、強制ダウンロード、JCMToolSuiteコマンドを送受信する。コマンドをメインに通知する。"

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal.h"
#include "hal_bezel.h"
#include "sub_functions.h"
#include "js_usb_test.h"
#include "subline_download.h"
#include "subline_suite.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS *************************/
/************************** Function Prototypes ******************************/
void subline_initialize(void);
void _subline_set_mode(u16 mode);
void _subline_msg_proc(void);
void subline_task(VP_INT exinf);
void _subline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _subline_system_error(u8 fatal_err, u8 code);
void subline_task(VP_INT exinf);
void subline_suite(void);
void subline_download(void);
void _subline_rcv_data(void);
void _subline_idle_proc(void);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
static T_MSG_BASIC subline_msg;

/*******************************
        subline_task
 *******************************/
void subline_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	subline_initialize();
	/* USB Initialize */
	usb_test_init();

	while (1)
	{
		subline_usb_suite_initial();
		/* Receive data */
		_subline_rcv_data();

		ercd = trcv_mbx(ID_SUBLINE_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		if (ercd == E_OK)
		{
			memcpy(&subline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(subline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_subline_system_error(1, 4);
			}
			_subline_msg_proc();
		}
		else
		{
			_subline_idle_proc();
		}
	}
}

/******************************************************************************/
/*! @brief Front USB device initialize
    @return         none
    @exception      none
******************************************************************************/
void subline_initialize(void)
{
	/* Wait Download Request Mode */
	_subline_set_mode(SUBLINE_MODE_NORMAL);

	memset((void *)&ex_operation_usb, 0, sizeof(ex_operation_usb));
	/* debug */
	ex_operation_usb.pc.mess.serviceID = 0x03;
	ex_operation_usb.status = SUBLINE_ANALYZE;

	ex_operation_usb_write_size = 0;
	ex_operation_usb_read_size = 0;
	memset((void *)&ex_subline_pkt, 0, sizeof(ex_subline_pkt));
}

void _subline_rcv_data(void)
{
	switch (ex_subline_task_mode)
	{
	case SUBLINE_MODE_DOWNLOAD:
		subline_download();
		break;
	case SUBLINE_MODE_NORMAL:
	case SUBLINE_MODE_DOWNLOAD_ERROR:
	case SUBLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR:
	case SUBLINE_MODE_DOWNLOAD_WAIT_RESET:
	default:
		subline_suite();
		break;
	}
}
void _subline_idle_proc(void)
{
	switch (ex_subline_task_mode)
	{
	case SUBLINE_MODE_NORMAL:
		break;
	case SUBLINE_MODE_DOWNLOAD:
		break;
	case SUBLINE_MODE_DOWNLOAD_ERROR:
		break;
	case SUBLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR:
		break;
	case SUBLINE_MODE_DOWNLOAD_WAIT_RESET:
		_subline_send_msg(ID_MAIN_MBX, TMSG_SUBLINE_SOFT_RESET_REQ, 0, 0, 0, 0);
		_subline_set_mode(SUBLINE_MODE_NORMAL);
		break;
	default:
		break;
	}
}
void _subline_msg_proc(void)
{
	switch (ex_subline_task_mode)
	{
	case SUBLINE_MODE_DOWNLOAD_WAIT_RSP:
		switch (subline_msg.tmsg_code)
		{
		case TMSG_SUBLINE_DOWNLOAD_RSP:
	    	if (subline_msg.arg1 == TMSG_SUB_SUCCESS)
	    	{
	    		// Start USB Download
				_subline_set_mode(SUBLINE_MODE_DOWNLOAD);
	    	}
	    	else if (subline_msg.arg1 == TMSG_SUB_ALARM)
	    	{
	    		// Download request refuse
				_subline_set_mode(SUBLINE_MODE_NORMAL);
	    	}
	    	else
	    	{
	    		/* system error ? */
				_subline_system_error(0, 5);
	    	}
			break;
		default:
			/* system error ? */
			_subline_system_error(0, 5);
			break;
		}
		break;
	case SUBLINE_MODE_NORMAL:
	case SUBLINE_MODE_DOWNLOAD:
	case SUBLINE_MODE_DOWNLOAD_ERROR:
	case SUBLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR:
	case SUBLINE_MODE_DOWNLOAD_WAIT_RESET:
	default:
		switch (subline_msg.tmsg_code)
		{
		default:
			/* system error ? */
			_subline_system_error(0, 5);
			break;
		}
		break;
	}
}

void _subline_set_mode(u16 mode)
{
	ex_subline_task_mode = mode;
}


/*********************************************************************//**
 * @brief send task message
 * @param[in]	receiver task id
 * 				task message code
 * 				argument 1
 * 				argument 2
 * 				argument 3
 * 				argument 4
 * @return 		None
 **********************************************************************/
void _subline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_SUBLINE_TASK;
		t_msg->mpf_id = ID_MBX_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_subline_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_subline_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _subline_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_subline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_subline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_SUBLINE_TASK, (u16)code, (u16)subline_msg.tmsg_code, (u16)subline_msg.arg1, fatal_err);
}

/* EOF */
