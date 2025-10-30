/******************************************************************************/
/*! @addtogroup Main
    @file       bezel_task.c
    @brief      control bezel display task function
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Branch from Display Task
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "custom.h"
#include "sub_functions.h"
#include "hal_bezel.h"
#include "hal.h"

#define EXT
#include "com_ram.c"

#if defined(PRJ_IVIZION2) //プロジェクトがivizionの場合、設定ミスなのでエラーにする
	TARGET_PROJECT_IS_IVIZION2_ERROR
#endif

/************************** Function Prototypes ******************************/
void bezel_task(VP_INT exinf);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
static T_MSG_BASIC bezel_msg;

/*********************************/
/*			BEZEL CONTROL		 */
/*********************************/
/* Status LED setting infomation */
u8 ex_bezel_led_mode;

/************************** PRIVATE FUNCTIONS *************************/
void _bezel_initialize_proc(void);
void _bezel_msg_proc(void);
void _bezel_idle_proc(void);
void _bezel_set_mode(u16 mode);
void _bezel_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _bezel_system_error(u8 fatal_err, u8 code);


/************************** EXTERN FUNCTIONS *************************/


/*******************************
        bezel_task
 *******************************/
void bezel_task(VP_INT exinf)	/* UBAはベゼルはFPGA経由での制御、BootIFはFPGA制御できないので、BootIFではベゼル制御しない*/
{
	T_MSG_BASIC *tmsg_pt;

	_bezel_initialize_proc();

	while(1)
	{
		if((trcv_mbx(ID_BEZEL_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME_BEZEL)) == E_OK)
		{
			memcpy(&bezel_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(bezel_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_bezel_system_error(1, 3);
			}
			_bezel_msg_proc();
		}
		_bezel_idle_proc();
	}
}

void _bezel_initialize_proc(void)
{
	_bezel_set_mode(0);
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  bezel task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _bezel_msg_proc(void)
{
	switch (bezel_msg.tmsg_code)
	{
	default:					/* other */
		break;
	}
}



/*********************************************************************//**
 * @brief Bezel task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _bezel_idle_proc(void)
{
	switch (ex_bezel_led_mode)
	{
	default:
		break;
	}
}



void _bezel_set_mode(u16 mode)
{
	ex_bezel_led_mode =  mode;
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
void _bezel_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_BEZEL_TASK;
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
			_bezel_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_bezel_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _bezel_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	_bezel_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_TEST_RUNNING, 0, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_BEZEL_TASK, (u16)code, (u16)bezel_msg.tmsg_code, (u16)bezel_msg.arg1, fatal_err);
}


/* EOF */
