
/******************************************************************************/
/*! @addtogroup Main
    @file       timer_task.c
    @brief      timer task process
    @date       2018/01/24
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/
void timer_task(VP_INT exinf);

/************************** Variable declaration *****************************/

/************************** PRIVATE DEFINITIONS *************************/
enum _TIMER_MODE
{
	TIMER_MODE_IDLE = 1,
};


/************************** PRIVATE VARIABLES *************************/
static T_MSG_BASIC timer_msg;
static u32 timer_id[MAX_TIMER];
static u32 timer_set_task[MAX_TIMER];
static u32 timer_cnt[MAX_TIMER];
static u32 timer_tick_backup;
static u32 timer_elapse_time;

/************************** PRIVATE FUNCTIONS *************************/
void _timer_rcv_msg_proc(void);
void _timer_init(void);
void _timer_set(void);
void _timer_cancel(void);
void _timer_idle_proc(void);
void _timer_elapse_time_counter(void);
void _timer_set_mode(u16 mode);
void _timer_send_timeup_msg(u32 timer_id, u32 task_id);
void _timer_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _timer_system_error(u8 fatal_err, u8 code);

/************************** EXTERN FUNCTIONS *************************/


/*********************************************************************//**
 * @brief process of receive message
 * @param[in]	exinf : extension information
 * @return 		None
 **********************************************************************/
void timer_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u16 count=0;

	_timer_init();								// イニシャル処理
	while (1)
	{
		_timer_idle_proc();						// アイドリング処理
		ercd = trcv_mbx(ID_TIMER_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME_TIMER);
		if(ercd == E_OK)
		{
			memcpy(&timer_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(timer_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_timer_system_error(1, 3);
			}
			_timer_rcv_msg_proc();		/* receive message procedure */
		}
		_timer_elapse_time_counter();

	#if defined(UBA_RTQ_ICB)
		if(ex_rtq_rfid_data == 1)
		{
			if(5000 < TASK_WAIT_TIME_TIMER * count)
			{
				count = 0;
				_timer_send_msg(ID_MAIN_MBX, TMSG_TIMER_RFID_RTQ, 0, 0, 0, 0);
			}
			else
			{
				count++;	
			}	
		}
		else
		{
			count = 0;	
		}
	#endif

	}
}


/*********************************************************************//**
 * @brief process of receive message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _timer_rcv_msg_proc(void)
{
	switch(timer_msg.tmsg_code)
	{
	case TMSG_TIMER_SET_TIMER:			/* set timer */
		_timer_set();
		break;
	case TMSG_TIMER_CANCEL_TIMER:		/* cancel timer */
		_timer_cancel();
		break;
	default:				/* other */
		/* system error ? */
		_timer_system_error(0, 4);
		break;
	}
}


/*********************************************************************//**
 * @brief initialize timer task
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _timer_init(void)
{
	memset((void*)&timer_id[0], 0, sizeof(timer_id));
	memset((void*)&timer_set_task[0], 0, sizeof(timer_set_task));
	memset((void*)&timer_cnt[0], 0, sizeof(timer_cnt));
	timer_tick_backup = ex_timer_task_tick;
	timer_elapse_time = 0;

	_timer_set_mode(TIMER_MODE_IDLE);
}


/*********************************************************************//**
 * @brief set timer
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _timer_set(void)
{
	int		ii;
	for (ii=0; ii<MAX_TIMER; ii++)
	{
		if ((timer_id[ii] == timer_msg.arg1)
		 && (timer_set_task[ii] == timer_msg.sender_id))
		{
			timer_cnt[ii] = timer_msg.arg2;
			return;
		}
	}
	for(ii=0;ii<MAX_TIMER;ii++)
	{
		if(timer_id[ii] == 0)
		{
			timer_id[ii] = timer_msg.arg1;
			timer_set_task[ii] = timer_msg.sender_id;
			timer_cnt[ii] = timer_msg.arg2;
			return;
		}
	}
}


/*********************************************************************//**
 * @brief cancel timer
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _timer_cancel(void)
{
	int ii;

	for (ii = 0; ii < MAX_TIMER; ii++)
	{
		if (timer_id[ii] == timer_msg.arg1)
		{
			timer_id[ii] = 0;
			timer_set_task[ii] = 0;
			timer_cnt[ii] = 0;
			break;
		}
	}
}


/*********************************************************************//**
 * @brief process of idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _timer_idle_proc(void)
{
	u32	ii;

	for (ii=0; ii<MAX_TIMER; ii++)
	{
		if ((timer_id[ii] != 0) && (timer_cnt[ii] > 0))
		{
			if (timer_cnt[ii] > timer_elapse_time)
			{
				timer_cnt[ii] -= timer_elapse_time;
			}
			else
			{
				timer_cnt[ii] = 0;
			}
			/*
			timer_cnt[ii]--;
			*/
			if(timer_cnt[ii] == 0)
			{
				_timer_send_timeup_msg((timer_id[ii]), (timer_set_task[ii]));
				timer_id[ii] = 0;
			}
		}
	}
}


/*********************************************************************//**
 * @brief Elapse time counter
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _timer_elapse_time_counter(void)
{
	if (ex_timer_task_tick < timer_tick_backup)
	{
		timer_elapse_time = (((0x00010000 - timer_tick_backup) + ex_timer_task_tick) & 0x0000FFFF);
	}
	else
	{
		timer_elapse_time = ((ex_timer_task_tick - timer_tick_backup) & 0x0000FFFF);
	}
	timer_tick_backup = ex_timer_task_tick;
}


/*********************************************************************//**
 * @brief set task mode
 * @param[in]	mode : task mode
 * @return 		None
 **********************************************************************/
void _timer_set_mode(u16 mode)
{
	ex_timer_task_mode = mode;
}


/*********************************************************************//**
 * @brief send TIMA_TIMEUP message
 * @param[in]	timer_id : _TIMER_ID
 * 				task_id  : Destination Task-ID
 * @return 		None
 **********************************************************************/
void _timer_send_timeup_msg(u32 timer_id, u32 task_id)
{
	s32 mbx_id = 0;

	switch(task_id)
	{
	case ID_MAIN_TASK:			/* main task */
		mbx_id = ID_MAIN_MBX;
		break;
	case ID_CLINE_TASK:			/* cline task */
		mbx_id = ID_CLINE_MBX;
		break;
	case ID_DLINE_TASK:			/* dline task */
		mbx_id = ID_DLINE_MBX;
		break;
	default:				/* other */
		/* system error ? */
		_timer_system_error(0, 5);
		break;
	}

	_timer_send_msg(mbx_id, TMSG_TIMER_TIMES_UP, timer_id, 0, 0, 0);
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
void _timer_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;
	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_TIMER_TASK;
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
			_timer_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_timer_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _timer_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_timer_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_timer_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_TIMER_TASK, (u16)code, (u16)timer_msg.tmsg_code, (u16)timer_msg.arg1, fatal_err);
}


/* EOF */
