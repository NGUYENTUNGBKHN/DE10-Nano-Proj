/******************************************************************************/
/*! @addtogroup Main
    @file       dipsw_task_task.c
    @date       2021/04/23
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
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

#include "hal_dipsw.h"
/************************** PRIVATE DEFINITIONS *************************/
enum _DIPSW_MODE
{
	DIPSW_MODE_INIT = 1,
	DIPSW_MODE_ACTIVE,
};
/************************** External functions *******************************/

/************************** Function Prototypes ******************************/
void dipsw_task(VP_INT exinf);
void dipsw_task_initialize(void);
void dipsw_port_initialize(void);
void _dipsw_idle_msg_proc(void);
void _dipsw_idle_proc(void);
void _dipsw_task_set_mode(u16 mode);
void _dipsw_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _dipsw_system_error(u8 fatal_err, u8 code);
void _dipsw_chg_flg_proc(void);

/************************** Variable declaration *****************************/
static T_MSG_BASIC dipsw_msg;

/*******************************
        dipsw_task
 *******************************/
void dipsw_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	/* Initialize GPIO, enable Interrupt */
	dipsw_port_initialize();
	/* Initialize Variables */
	dipsw_task_initialize();

	while (1)
	{
		ercd = trcv_mbx(ID_DIPSW_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME_DIP);
		if (ercd == E_OK)
		{
			memcpy(&dipsw_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(dipsw_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_dipsw_system_error(1, 3);
			}
			_dipsw_idle_msg_proc();
		}
		_dipsw_idle_proc();
	}
}

/******************************************************************************/
/*! @brief Dipsw TASK initialize
    @return         none
    @exception      none
******************************************************************************/
void dipsw_task_initialize(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_dipsw_task_set_mode(DIPSW_MODE_INIT);
	ercd = rcv_mbx(ID_DIPSW_MBX, (T_MSG **)&tmsg_pt);
	if (ercd == E_OK)
	{
		memcpy(&dipsw_msg, tmsg_pt, sizeof(T_MSG_BASIC));
		if ((rel_mpf(dipsw_msg.mpf_id, tmsg_pt)) != E_OK)
		{
			/* system error */
			_dipsw_system_error(1, 4);
		}
		if (dipsw_msg.tmsg_code == TMSG_DIPSW_INIT_REQ)
		{
			_dipsw_send_msg(ID_MAIN_MBX, TMSG_DIPSW_INIT_RSP, TMSG_SUB_SUCCESS, ex_dipsw1, 0, 0);
		}
	}
	_dipsw_task_set_mode(DIPSW_MODE_ACTIVE);
}

/******************************************************************************/
/*! @brief Dipsw GPIO port initialize
    @return         none
    @exception      none
******************************************************************************/
void dipsw_port_initialize(void)
{
	u8 l_dipsw;

	_hal_read_dipsw1(&l_dipsw);

	if(ex_dipsw1 != l_dipsw)
	{
		ex_dipsw1 = l_dipsw;
	}
}

/******************************************************************************/
/*! @brief Dipsw change event
    @return         none
    @exception      none
******************************************************************************/
void _dipsw_chg_flg_proc(void)
{
	_dipsw_send_msg(ID_DLINE_MBX, TMSG_DIPSW_INFO, TMSG_SUB_SUCCESS, 0, 0, 0);
}

/******************************************************************************/
/*! @brief DIPSW msg proc
 *  @param[in]	    none
    @return         none
    @exception      none
******************************************************************************/
void _dipsw_idle_msg_proc(void)
{
	switch (dipsw_msg.tmsg_code)
	{
	case TMSG_DIPSW_INIT_REQ:
		_dipsw_send_msg(ID_MAIN_MBX, TMSG_DIPSW_STATUS_INFO, TMSG_SUB_SUCCESS, ex_dipsw1, ex_dipsw2, 0);
		break;
	case TMSG_DIPSW_STATUS_REQ:
		_dipsw_send_msg(ID_MAIN_MBX, TMSG_DIPSW_STATUS_INFO, TMSG_SUB_SUCCESS, ex_dipsw1, ex_dipsw2, 0);
		break;
	default:
		/* system error ? */
		_dipsw_system_error(0, 5);
		break;
	}
}

/******************************************************************************/
/*! @brief DIPSW Idle mode proc
 *  @param[in]	    none
    @return         none
    @exception      none
******************************************************************************/
void _dipsw_idle_proc(void)
{
	u8 l_dipsw;
	_hal_read_dipsw1(&l_dipsw);

	if(ex_dipsw1 != l_dipsw)
	{
		ex_dipsw1 = l_dipsw;
		_dipsw_send_msg(ID_MAIN_MBX, TMSG_DIPSW_INFO, TMSG_SUB_SUCCESS, ex_dipsw1, ex_dipsw2, 0);
	}
};
/******************************************************************************/
/*! @brief Chage Front USB detect TASK mode
    @return         none
    @exception      none
******************************************************************************/
void _dipsw_task_set_mode(u16 mode)
{
	ex_dipsw_task_mode = mode;
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
void _dipsw_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_DIPSW_TASK;
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
			_dipsw_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_dipsw_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _dipsw_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_dipsw_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_DIPSW_TASK, (u16)code, (u16)dipsw_msg.tmsg_code, (u16)dipsw_msg.arg1, fatal_err);
}

/* EOF */
