/******************************************************************************/
/*! @addtogroup Main
    @file       usb0_cb_task_task.c
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
#include "hal_bezel.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS *************************/
enum _USB0_CB_MODE
{
	/* Not Initialize */
	USB0_CB_MODE_UNKNOWN = 0,
	/* Not Initialize */
	USB0_CB_MODE_DISCONNECT,
	USB0_CB_MODE_CONNECT,
};
// Check
#define IS_USB0_EVT_CONNECT(x)	((x & EVT_USB_CON) == EVT_USB_CON)
#define IS_USB0_EVT_RECV(x)		((x & EVT_USB_RCV) == EVT_USB_RCV)
#define IS_USB0_EVT_EMPTY(x)	((x & EVT_USB_EMP) == EVT_USB_EMP)

// Clear
#define CLR_USB0_EVT_CONNECT(x)	((x &= (~EVT_USB_CON)))
#define CLR_USB0_EVT_RECV(x)	((x &= (~EVT_USB_RCV)))
#define CLR_USB0_EVT_EMPTY(x)	((x &= (~EVT_USB_EMP)))

/************************** External functions *******************************/

/************************** Function Prototypes ******************************/
void usb0_cb_task(VP_INT exinf);
void _usb0_cb_proc(FLGPTN flg);
void usb0_cb_task_initialize(void);
void usb0_cb_port_initialize(void);
void _usb0_cb_con_flg_proc(void);
void _usb0_cb_recv_flg_proc(void);
void _usb0_cb_empty_flg_proc(void);
void _usb0_cb_task_set_mode(u16 mode);
void _usb0_cb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _usb0_cb_system_error(u8 fatal_err, u8 code);

/************************** Variable declaration *****************************/
static T_MSG_BASIC usb0_cb_msg;

/*******************************
        usb0_cb_task
 *******************************/
void usb0_cb_task(VP_INT exinf)
{
	FLGPTN flag = 0;
	ER ercd;

	/* Initialize Variables */
	usb0_cb_task_initialize();
	/* Initialize GPIO, enable Interrupt */
	usb0_cb_port_initialize();

	while (1)
	{
		ercd = twai_flg(ID_USB0_CB_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TMO_FEVR);
		if ((ercd == E_OK) && (flag != 0))
		{
			clr_flg(ID_USB0_CB_FLAG, ~EVT_ALL_BIT);
			_usb0_cb_proc(flag);
		}
	}
}

/******************************************************************************/
/*! @brief Front USB detect TASK initialize
    @return         none
    @exception      none
******************************************************************************/
void usb0_cb_task_initialize(void)
{
}

/******************************************************************************/
/*! @brief Front USB detect GPIO port initialize
    @return         none
    @exception      none
******************************************************************************/
void usb0_cb_port_initialize(void)
{
	// TODO:implement
}

/******************************************************************************/
/*! @brief USB0 event proc
 *  @param[in]	    flg event flag
    @return         none
    @exception      none
******************************************************************************/
void _usb0_cb_proc(FLGPTN flg)
{
	FLGPTN local = flg;

	if(IS_USB0_EVT_RECV(local))
	{
		CLR_USB0_EVT_RECV(local);
		_usb0_cb_recv_flg_proc();
	}
	if(IS_USB0_EVT_CONNECT(local))
	{
		CLR_USB0_EVT_CONNECT(local);
		_usb0_cb_con_flg_proc();
	}
	if(IS_USB0_EVT_EMPTY(local))
	{
		CLR_USB0_EVT_EMPTY(local);
		_usb0_cb_empty_flg_proc();
	}
}

/******************************************************************************/
/*! @brief USB Connect/Disconnect event
    @return         none
    @exception      none
******************************************************************************/
void _usb0_cb_con_flg_proc(void)
{
	u32 connect = is_usb0_connected();
	_usb0_cb_send_msg(ID_DLINE_MBX, TMSG_USB0CB_CALLBACK_INFO, TMSG_SUB_CONNECT, connect, 0, 0);
}
/******************************************************************************/
/*! @brief USB Recv event
    @return         none
    @exception      none
******************************************************************************/
void _usb0_cb_recv_flg_proc(void)
{
	_usb0_cb_send_msg(ID_DLINE_MBX, TMSG_USB0CB_CALLBACK_INFO, TMSG_SUB_RECEIVE, 0, 0, 0);
}
/******************************************************************************/
/*! @brief USB Send complete event
    @return         none
    @exception      none
******************************************************************************/
void _usb0_cb_empty_flg_proc(void)
{
	_usb0_cb_send_msg(ID_DLINE_MBX, TMSG_USB0CB_CALLBACK_INFO, TMSG_SUB_EMPTY, 0, 0, 0);
}

/******************************************************************************/
/*! @brief Chage USB0 Callback TASK mode
    @return         none
    @exception      none
******************************************************************************/
void _usb0_cb_task_set_mode(u16 mode)
{
	ex_usb0_cb_task_mode = mode;
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
void _usb0_cb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_USB0_CB_TASK;
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
			_usb0_cb_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_usb0_cb_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _usb0_cb_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_usb0_cb_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_usb0_cb_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_USB0_CB_TASK, (u16)code, (u16)usb0_cb_msg.tmsg_code, (u16)usb0_cb_msg.arg1, fatal_err);
}

/* EOF */
