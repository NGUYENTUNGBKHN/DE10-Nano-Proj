/******************************************************************************/
/*! @addtogroup Main
    @file       usb1_cb_task_task.c
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
enum _USB1_CB_MODE
{
	/* Not Initialize */
	USB1_CB_MODE_UNKNOWN = 0,
	/* Not Initialize */
	USB1_CB_MODE_DISCONNECT,
	USB1_CB_MODE_CONNECT,
};
// Check
#define IS_USB1_EVT_CONNECT(x)	((x & EVT_USB_CON) == EVT_USB_CON)
#define IS_USB1_EVT_RECV(x)		((x & EVT_USB_RCV) == EVT_USB_RCV)
#define IS_USB1_EVT_EMPTY(x)	((x & EVT_USB_EMP) == EVT_USB_EMP)

// Clear
#define CLR_USB1_EVT_CONNECT(x)	((x &= (~EVT_USB_CON)))
#define CLR_USB1_EVT_RECV(x)	((x &= (~EVT_USB_RCV)))
#define CLR_USB1_EVT_EMPTY(x)	((x &= (~EVT_USB_EMP)))


/************************** External functions *******************************/

/************************** Function Prototypes ******************************/
void usb1_cb_task(VP_INT exinf);
void _usb1_cb_proc(FLGPTN flg);
void usb1_cb_task_initialize(void);
void usb1_cb_port_initialize(void);
void _usb1_cb_con_flg_proc(void);
void _usb1_cb_recv_flg_proc(void);
void _usb1_cb_empty_flg_proc(void);
void _usb1_cb_task_set_mode(u16 mode);
void _usb1_cb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _usb1_cb_system_error(u8 fatal_err, u8 code);

/************************** Variable declaration *****************************/
static T_MSG_BASIC usb1_cb_msg;

/*******************************
        usb1_cb_task
 *******************************/
void usb1_cb_task(VP_INT exinf)
{
	FLGPTN flag = 0;
	ER ercd;

	/* Initialize Variables */
	usb1_cb_task_initialize();
	/* Initialize GPIO, enable Interrupt */
	usb1_cb_port_initialize();

	while (1)
	{
		ercd = twai_flg(ID_USB1_CB_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TMO_FEVR);
		if ((ercd == E_OK) && (flag != 0))
		{
			clr_flg(ID_USB1_CB_FLAG, ~EVT_ALL_BIT);
			_usb1_cb_proc(flag);
		}
	}
}

/******************************************************************************/
/*! @brief Front USB detect TASK initialize
    @return         none
    @exception      none
******************************************************************************/
void usb1_cb_task_initialize(void)
{
	_usb1_cb_task_set_mode(USB1_CB_MODE_UNKNOWN);
}

/******************************************************************************/
/*! @brief Front USB detect GPIO port initialize
    @return         none
    @exception      none
******************************************************************************/
void usb1_cb_port_initialize(void)
{
}

/******************************************************************************/
/*! @brief Front USB detect GPIO change event
 *  @param[in]	    flg event flag
    @return         none
    @exception      none
******************************************************************************/
void _usb1_cb_proc(FLGPTN flg)
{
	FLGPTN local = flg;

	if(IS_USB1_EVT_RECV(local))
	{
		CLR_USB1_EVT_RECV(local);
		_usb1_cb_recv_flg_proc();
	}
	if(IS_USB1_EVT_CONNECT(local))
	{
		CLR_USB1_EVT_CONNECT(local);
		_usb1_cb_con_flg_proc();
	}
	if(IS_USB1_EVT_EMPTY(local))
	{
		CLR_USB1_EVT_EMPTY(local);
		_usb1_cb_empty_flg_proc();
	}
}


/******************************************************************************/
/*! @brief USB Connect/Disconnect event
    @return         none
    @exception      none
******************************************************************************/
void _usb1_cb_con_flg_proc(void)
{
	#if defined(USB_REAR_USE)
	u32 connect = rear_is_usb1_connected();
	_usb1_cb_send_msg(ID_DLINE_MBX, TMSG_USB1CB_CALLBACK_INFO, TMSG_SUB_CONNECT, connect, 0, 0);
	#else
	u32 connect = is_usb1_connected();
	_usb1_cb_send_msg(ID_CLINE_MBX, TMSG_USB1CB_CALLBACK_INFO, TMSG_SUB_CONNECT, connect, 0, 0);
	#endif // #if defined(USB_REAR_USE)
}
/******************************************************************************/
/*! @brief USB Recv event
    @return         none
    @exception      none
******************************************************************************/
void _usb1_cb_recv_flg_proc(void)
{
	#if defined(USB_REAR_USE)
	_usb1_cb_send_msg(ID_DLINE_MBX, TMSG_USB1CB_CALLBACK_INFO, TMSG_SUB_RECEIVE, 0, 0, 0);
	#else
	_usb1_cb_send_msg(ID_CLINE_MBX, TMSG_USB1CB_CALLBACK_INFO, TMSG_SUB_RECEIVE, 0, 0, 0);
	#endif  // #if defined(USB_REAR_USE)
}
/******************************************************************************/
/*! @brief USB Send complete event
    @return         none
    @exception      none
******************************************************************************/
void _usb1_cb_empty_flg_proc(void)
{
	#if defined(USB_REAR_USE)
	_usb1_cb_send_msg(ID_DLINE_MBX, TMSG_USB1CB_CALLBACK_INFO, TMSG_SUB_EMPTY, 0, 0, 0);
	#else
	_usb1_cb_send_msg(ID_CLINE_MBX, TMSG_USB1CB_CALLBACK_INFO, TMSG_SUB_EMPTY, 0, 0, 0);
	#endif // #if defined(USB_REAR_USE)
}

/******************************************************************************/
/*! @brief Chage Front USB detect TASK mode
    @return         none
    @exception      none
******************************************************************************/
void _usb1_cb_task_set_mode(u16 mode)
{
	ex_usb1_cb_task_mode = mode;
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
void _usb1_cb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_USB1_CB_TASK;
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
			_usb1_cb_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_usb1_cb_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _usb1_cb_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_usb1_cb_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_USB1_CB_TASK, (u16)code, (u16)usb1_cb_msg.tmsg_code, (u16)usb1_cb_msg.arg1, fatal_err);
}

/* EOF */
