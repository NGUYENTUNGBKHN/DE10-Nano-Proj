/******************************************************************************/
/*! @addtogroup Main
    @file       uart01_cb_task_task.c
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
enum _UART01_CB_MODE
{
	/* Not Initialize */
	UART01_CB_MODE_UNKNOWN = 0,
	/* Not Initialize */
	UART01_CB_MODE_DISCONNECT,
	UART01_CB_MODE_CONNECT,
};


// Check
#define IS_UART_EVT_RECV(x)		((x & EVT_UART_RCV) == EVT_UART_RCV)
#define IS_UART_EVT_EMPTY(x)	((x & EVT_UART_EMP) == EVT_UART_EMP)
#define IS_UART_EVT_ERROR(x)		((x & EVT_UART_ERR) == EVT_UART_ERR)

// Local flag Clear
#define CLR_UART_EVT_RECV(x)	((x &= (~EVT_UART_RCV)))
#define CLR_UART_EVT_EMPTY(x)	((x &= (~EVT_UART_EMP)))
#define CLR_UART_EVT_ERROR(x)	((x &= (~EVT_UART_ERR)))
/************************** External functions *******************************/

/************************** Function Prototypes ******************************/
void uart01_cb_task(VP_INT exinf);
void uart01_cb_task_initialize(void);
void uart01_cb_port_initialize(void);
void _uart01_cb_proc(FLGPTN flg);
void _uart01_cb_task_set_mode(u16 mode);
void _uart01_cb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _uart01_cb_system_error(u8 fatal_err, u8 code);
void _uart01_cb_recv_flg_proc(void);
void _uart01_cb_empty_flg_proc(void);

/************************** Variable declaration *****************************/
static T_MSG_BASIC uart01_cb_msg;

/*******************************
        uart01_cb_task
 *******************************/
void uart01_cb_task(VP_INT exinf)
{
	FLGPTN flag = 0;
	ER ercd;

	/* Initialize Variables */
	uart01_cb_task_initialize();
	/* Initialize GPIO, enable Interrupt */
	uart01_cb_port_initialize();

	while (1)
	{
		ercd = twai_flg(ID_UART01_CB_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TMO_FEVR);
		if ((ercd == E_OK) && (flag != 0))
		{
			_uart01_cb_proc(flag);
		}
	}
}

/******************************************************************************/
/*! @brief Front USB detect TASK initialize
    @return         none
    @exception      none
******************************************************************************/
void uart01_cb_task_initialize(void)
{
	_uart01_cb_task_set_mode(UART01_CB_MODE_CONNECT);
}

/******************************************************************************/
/*! @brief Front USB detect GPIO port initialize
    @return         none
    @exception      none
******************************************************************************/
void uart01_cb_port_initialize(void)
{
}

/******************************************************************************/
/*! @brief UART Recv event
    @return         none
    @exception      none
******************************************************************************/
void _uart01_cb_recv_flg_proc(void)
{
	_uart01_cb_send_msg(ID_CLINE_MBX, TMSG_UART01CB_CALLBACK_INFO, TMSG_SUB_RECEIVE, 0, 0, 0);
}
/******************************************************************************/
/*! @brief UART Send complete event
    @return         none
    @exception      none
******************************************************************************/
void _uart01_cb_empty_flg_proc(void)
{
	_uart01_cb_send_msg(ID_CLINE_MBX, TMSG_UART01CB_CALLBACK_INFO, TMSG_SUB_EMPTY, 0, 0, 0);
}
/******************************************************************************/
/*! @brief UART communication error event
    @return         none
    @exception      none
******************************************************************************/
void _uart01_cb_error_flg_proc(void)
{
	_uart01_cb_send_msg(ID_CLINE_MBX, TMSG_UART01CB_CALLBACK_INFO, TMSG_SUB_ALARM, 0, 0, 0);
}

/******************************************************************************/
/*! @brief UART01 event proc
 *  @param[in]	    flg event flag
    @return         none
    @exception      none
******************************************************************************/
void _uart01_cb_proc(FLGPTN flg)
{
	FLGPTN local = flg;

	if (IS_UART_EVT_RECV(local))
	{
		CLR_UART_EVT_RECV(local);
		_uart01_cb_recv_flg_proc();
	}
	else if(IS_UART_EVT_EMPTY(local))
	{
		CLR_UART_EVT_EMPTY(local);
		_uart01_cb_empty_flg_proc();
	}
	else if(IS_UART_EVT_ERROR(local))
	{
		CLR_UART_EVT_ERROR(local);
		_uart01_cb_error_flg_proc();
	}
}

/******************************************************************************/
/*! @brief Chage Front USB detect TASK mode
    @return         none
    @exception      none
******************************************************************************/
void _uart01_cb_task_set_mode(u16 mode)
{
	ex_uart01_cb_task_mode = mode;
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
void _uart01_cb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_UART01_CB_TASK;
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
			_uart01_cb_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_uart01_cb_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _uart01_cb_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_uart01_cb_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_UART01_CB_TASK, (u16)code, (u16)uart01_cb_msg.tmsg_code, (u16)uart01_cb_msg.arg1, fatal_err);
}

/* EOF */
