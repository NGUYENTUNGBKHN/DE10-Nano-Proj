/******************************************************************************/
/*! @addtogroup Main
    @file       dline_task.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal.h"
#include "hal_gpio.h"
#include "hal_bezel.h"
#include "hal_cyclonev_rstmgr.h"
#include "sub_functions.h"

#ifdef  FLASH_QSPI_MODE
#include "alt_qspi.h"
#endif

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

#include "dline_download.h"
#include "dline_suite.h"

/************************** PRIVATE DEFINITIONS *************************/
#define IS_OTG_EVT_DOWNLOAD_READY(x)	((x & EVT_OTG_DOWNLOAD_READY) == EVT_OTG_DOWNLOAD_READY)

/************************** EXTERNAL VARIABLES *************************/

/************************** EXTERNAL Functions *************************/
extern void front_usb_suite_initial(void);
extern INT8 write_rom_from_file(void);
/************************** Function Prototypes ******************************/
void dline_task(VP_INT exinf);
void _dline_initialize(void);
void _dline_clear_buffer(void);
void _dline_rcv_msg_proc(void);
void _dline_idle_proc(void);

void _dline_msg_proc_timesup(void);
void _dline_msg_proc_timesup_fusb_bounce(void);
void _dline_msg_proc_timesup_data_wait(void);

void _dline_msg_proc_callback(void);
void _dline_msg_proc_callback_connect(void);
void _dline_msg_proc_callback_empty(void);
void _dline_msg_proc_callback_receive(void);

#if defined(USB_REAR_USE)
void _dline_msg_proc_callback2(void);
void _dline_msg_proc_callback_connect2(void);
void _dline_msg_proc_callback_empty2(void);
void _dline_msg_proc_callback_receive2(void);
#endif // USB_REAR_USE

void _dline_msg_proc_otg_notice(void);
void _dline_msg_proc_otg_notice_download_ready(void);
void _dline_msg_proc_otg_notice_connect(void);

void _dline_msg_proc_download_rsp(void);

void _dline_set_con_state(u16 state);
void _dline_set_mode(u16 mode);
void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _dline_system_error(u8 fatal_err, u8 code);

/************************** Variable declaration *****************************/
static T_MSG_BASIC dline_msg;

u16 ex_dline_con_state;

/*******************************
        dline_task
 *******************************/
void dline_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_dline_initialize();

	while (1)
	{
		ercd = trcv_mbx(ID_DLINE_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME_DLINE);
		if (ercd == E_OK)
		{
			memcpy(&dline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			memset(tmsg_pt, 0, sizeof(T_MSG_BASIC));
			if ((rel_mpf(dline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_dline_system_error(1, 4);
			}
			_dline_rcv_msg_proc();
		}
		else
		{
			_dline_idle_proc();
		}
	}
}

/******************************************************************************/
/*! @brief Front USB device initialize
    @return         none
    @exception      none
******************************************************************************/
void _dline_initialize(void)
{
#ifdef  FLASH_QSPI_MODE
	bool stat = false;
	qspi_software_reset();
	RstmgrQspiReset();
	/* FLASHメモリドライバ設定 cv5hwlib */
	stat = alt_qspi_init();
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}

	/* disable mode bit */
	stat = alt_qspi_mode_bit_disable();
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}
	stat = alt_qspi_mode_bit_config_set(0x00);
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}

	stat = alt_qspi_enable();
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}
	stat = alt_qspi_sram_partition_set(0x40);
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}
#else
	/* FLASHメモリドライバ設定 JSL */
	QSPI_Flash_init(NULL);
	memset((void *)&flash_param, 0, sizeof(flash_param));
	QSPI_Flash_open(&hQFlash, &flash_param, &size);
#endif
	/* Wait Download Request Mode */
	_dline_set_mode(DLINE_MODE_NORMAL);

	memset((void *)&ex_front_usb, 0, sizeof(ex_front_usb));
	/* debug */
	ex_front_usb.pc.mess.serviceID = 0x03;
	ex_front_usb.status = DLINE_ANALYZE;
	front_usb_suite_initial();

	_dline_clear_buffer();
}
void _dline_msg_proc_fusb_dect_notice(void)
{
	_dline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_FUSB_BOUNCE, WAIT_TIME_FUSB_BOUNCE, 0, 0);
}
void _dline_msg_proc_timesup(void)
{
	switch(dline_msg.arg1)
	{
	case TIMER_ID_FUSB_BOUNCE:
		 _dline_msg_proc_timesup_fusb_bounce();
		break;
	case TIMER_ID_DATA_WAIT:
		 _dline_msg_proc_timesup_data_wait();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 5);
		break;
	}
}


void _dline_msg_proc_timesup_fusb_bounce(void)
{
	u8 fusb_dect = 0;
	fusb_dect =is_fusb_dect_on();
#if 1
	// TODO:
#elif 1
	if(fusb_dect)
	{
		FrontUSBConnect();
	}
	else
	{
		RearUSBConnect();
	}
#else
	switch (ex_dline_con_state)
	{
	case DLINE_CON_STATE_DISCONNECTED:
	case DLINE_CON_STATE_REAR_CONNECTED:
		if(fusb_dect)
		{
			USBDisconnect();
			FrontUSBConnect();
			_dline_set_con_state(DLINE_CON_STATE_FRONT_CONNECTED);
		}
		break;
	case DLINE_CON_STATE_FRONT_CONNECTED:
		if(!fusb_dect)
		{
			USBDisconnect();
			RearUSBConnect();
			_dline_set_con_state(DLINE_CON_STATE_DISCONNECTED);
		}
		break;
	default:
		/* system error ? */
		_dline_system_error(0, 5);
		break;
	}
#endif
}

void _dline_clear_buffer(void)
{
	ex_usb_write_size = 0;
	ex_usb_read_size = 0;
	memset((void *)&ex_dline_pkt, 0, sizeof(ex_dline_pkt));
}
void _dline_msg_proc_timesup_data_wait(void)
{
	_dline_clear_buffer();
}
void _dline_idle_proc(void)
{

	switch (ex_dline_task_mode)
	{
	case DLINE_MODE_NORMAL:
		break;
	case DLINE_MODE_DOWNLOAD:
		break;
	case DLINE_MODE_DOWNLOAD_ERROR:
		break;
	case DLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR:
		break;
	case DLINE_MODE_DOWNLOAD_WAIT_RESET:
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_SOFT_RESET_REQ, 0, 0, 0, 0);
		_dline_set_mode(DLINE_MODE_NORMAL);
		break;
	case DLINE_MODE_HOST_DOWNLOAD:
		break;
	case DLINE_MODE_HOST_DOWNLOAD_ERROR:
		break;
	case DLINE_MODE_HOST_DOWNLOAD_ILLEGAL_FILE_ERROR:
		break;
	case DLINE_MODE_HOST_DOWNLOAD_WAIT_PULL_OUT:
		if(is_host_usb_pull_out())
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_SOFT_RESET_REQ, 0, 0, 0, 0);
			_dline_set_mode(DLINE_MODE_HOST_DOWNLOAD_WAIT_RESET);
		}
		break;
	case DLINE_MODE_HOST_DOWNLOAD_WAIT_RESET:
		break;
	default:
		break;
	}
}

/*********************************************************************//**
 * @brief process of receive message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc(void)
{
	switch (dline_msg.tmsg_code)
	{
#if defined(USB_REAR_USE)
	case TMSG_USB1CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_FUSB_DECT_NOTICE:
		_dline_msg_proc_fusb_dect_notice();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_DOWNLOAD_RSP:
		_dline_msg_proc_download_rsp();
		break;
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
/*********************************************************************//**
 * @brief process of USB0 callback notice message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_msg_proc_callback(void)
{
	switch (dline_msg.arg1)
	{
	case TMSG_SUB_CONNECT:
		_dline_msg_proc_callback_connect();
		break;
	case TMSG_SUB_RECEIVE:
		_dline_msg_proc_callback_receive();
		break;
	case TMSG_SUB_EMPTY:
		_dline_msg_proc_callback_empty();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_connect(void)
{
	switch (ex_dline_con_state)
	{
	case DLINE_CON_STATE_REAR_CONNECTED:
	case DLINE_CON_STATE_FRONT_CONNECTED:
		if(!dline_msg.arg2)
		{
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_DISCONNECTED);
		}
		break;
	case DLINE_CON_STATE_DISCONNECTED:
		if(dline_msg.arg2)
		{
#if 1
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_FRONT_CONNECTED);
#else
			if(!fusb_dect)
			{
				_dline_set_con_state(DLINE_CON_STATE_FRONT_CONNECTED);
			}
			else
			{
				_dline_set_con_state(DLINE_CON_STATE_REAR_CONNECTED);
			}
#endif
		}
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_empty(void)
{
	ex_usb_write_busy = 0;
}
void _dline_msg_proc_callback_receive(void)
{
	switch (ex_dline_task_mode)
	{
	case DLINE_MODE_DOWNLOAD:
		dline_download();
		break;
	case DLINE_MODE_NORMAL:
	case DLINE_MODE_DOWNLOAD_ERROR:
	case DLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR:
	case DLINE_MODE_DOWNLOAD_WAIT_RESET:
	default:
		dline_suite();
		break;
	}
}
#if defined(USB_REAR_USE)
/*********************************************************************//**
 * @brief process of USB0 callback notice message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_msg_proc_callback2(void)
{
	switch (dline_msg.arg1)
	{
	case TMSG_SUB_CONNECT:
		_dline_msg_proc_callback_connect2();
		break;
	case TMSG_SUB_RECEIVE:
		_dline_msg_proc_callback_receive2();
		break;
	case TMSG_SUB_EMPTY:
		_dline_msg_proc_callback_empty2();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_connect2(void)
{
	switch (ex_dline_con_state)
	{
	case DLINE_CON_STATE_REAR_CONNECTED:
	case DLINE_CON_STATE_FRONT_CONNECTED:
		if(!dline_msg.arg2)
		{
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_DISCONNECTED);
		}
		break;
	case DLINE_CON_STATE_DISCONNECTED:
		if(dline_msg.arg2)
		{
#if 1
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_FRONT_CONNECTED);
#else
			if(!fusb_dect)
			{
				_dline_set_con_state(DLINE_CON_STATE_FRONT_CONNECTED);
			}
			else
			{
				_dline_set_con_state(DLINE_CON_STATE_REAR_CONNECTED);
			}
#endif
		}
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_empty2(void)
{
	ex_rear_usb_write_busy = 0;
}
void _dline_msg_proc_callback_receive2(void)
{
	switch (ex_dline_task_mode)
	{
	case DLINE_MODE_DOWNLOAD:
		dline_download2();
		break;
	case DLINE_MODE_NORMAL:
	case DLINE_MODE_DOWNLOAD_ERROR:
	case DLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR:
	case DLINE_MODE_DOWNLOAD_WAIT_RESET:
	default:
		dline_suite2();
		break;
	}
}
#endif // USB_REAR_USE
void _dline_msg_proc_otg_notice(void)
{
	switch (dline_msg.arg1)
	{
	case TMSG_SUB_DOWNLOAD:
		_dline_msg_proc_otg_notice_download_ready();
		break;
	case TMSG_SUB_CONNECT:
		_dline_msg_proc_otg_notice_connect();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_otg_notice_download_ready(void)
{
	switch (ex_dline_task_mode)
	{
	case DLINE_MODE_NORMAL:
		/* download ready */
		_dline_set_mode(DLINE_MODE_HOST_DOWNLOAD_WAIT_RSP);
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_DOWNLOAD_REQ, 0, 0, 0, 0);
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 9);
		break;
	}
}

void _dline_msg_proc_otg_notice_connect(void)
{
	if(!dline_msg.arg2)
	{
		/* wait reset */
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_SOFT_RESET_REQ, 0, 0, 0, 0);
		_dline_set_mode(DLINE_MODE_HOST_DOWNLOAD_WAIT_RESET);
	}
}


void _dline_msg_proc_download_rsp(void)
{
	switch (ex_dline_task_mode)
	{
	case DLINE_MODE_DOWNLOAD_WAIT_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			// Start USB Download
			_dline_set_mode(DLINE_MODE_DOWNLOAD);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			// Download request refuse
			_dline_set_mode(DLINE_MODE_NORMAL);
		}
		else
		{
			/* system error ? */
			_dline_system_error(0, 5);
		}
		break;
	case DLINE_MODE_HOST_DOWNLOAD_WAIT_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			// Start USB Download
			_dline_set_mode(DLINE_MODE_HOST_DOWNLOAD);
			// write flash
			if(write_rom_from_file())
			{
		    	_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_DOWNLOAD_COMPLETE_REQ, 0, 0, 0, 0);
				_dline_set_mode(DLINE_MODE_HOST_DOWNLOAD_WAIT_PULL_OUT);
			}
			else
			{
				_dline_set_mode(DLINE_MODE_HOST_DOWNLOAD_ERROR);
			}
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			// Download request refuse
			_dline_set_mode(DLINE_MODE_NORMAL);
		}
		else
		{
			/* system error ? */
			_dline_system_error(0, 5);
		}
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 9);
		break;
	}
}
void _dline_set_con_state(u16 state)
{
	ex_dline_con_state = state;
}

void _dline_set_mode(u16 mode)
{
	ex_dline_task_mode = mode;
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
void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_DLINE_TASK;
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
			_dline_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_dline_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _dline_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_dline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_DLINE_TASK, (u16)code, (u16)dline_msg.tmsg_code, (u16)dline_msg.arg1, fatal_err);
}

/* EOF */
