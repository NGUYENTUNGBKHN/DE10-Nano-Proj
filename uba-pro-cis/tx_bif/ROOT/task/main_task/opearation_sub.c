/******************************************************************************/
/*! @addtogroup Main********************************/
/*! @addtogroup Group2
    @file       operation_sub.c
    @brief      operation sub process
    @date       2018/03/05
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "sub_functions.h"
#include "hal.h"

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/
void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _main_system_error(u8 fatal_err, u8 code);
/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/
void _main_display_init(void);

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/
extern u8 ex_main_task_mode1;
extern u8 ex_main_task_mode2;

/*********************************************************************//**
 * @brief set initialize
 * @param[in]	type : initialize type
 * @return 		None
 **********************************************************************/
void _main_set_init(void)
{
	ex_abnormal_code    = 0;

	_main_display_init();
}



/*********************************************************************//**
 * @brief set display powerup
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_powerup(void)
{
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
#if defined(_PROTOCOL_ENABLE_ID064GD)
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
#endif
}


/*********************************************************************//**
 * @brief set display initialize
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_init(void)
{
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
#if defined(_PROTOCOL_ENABLE_ID064GD)
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
#endif
}

/*********************************************************************//**
 * @brief set display pause
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u16 _main_conv_seq(void)
{
	u16 rtn = (0xF000|(ex_main_task_mode1 << 8)|(ex_main_task_mode2));
	return rtn;
}

/*********************************************************************//**
 * @brief reject sub function
 * @param[in]	msg  : respons(send) TMSG
 * 				arg1 : argument 1
 * 				arg2 : argument 2
 * 				arg3 : argument 3
 * 				arg4 : argument 4
 * @return 		None
 **********************************************************************/
void _main_send_connection_task(u32 msg, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	_main_send_msg(ID_CLINE_MBX, (msg|TMSG_TCODE_MAIN), arg1, arg2, arg3, arg4);
	_main_send_msg(ID_DLINE_MBX, (msg|TMSG_TCODE_MAIN), arg1, arg2, arg3, arg4);
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
void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MAIN_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_MAIN_TASK;
		t_msg->mpf_id = ID_MBX_MAIN_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_main_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_main_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _main_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_MAIN_TASK, (u16)code, (u16)ex_main_msg.tmsg_code, (u16)ex_main_msg.arg1, fatal_err);
}



void _main_set_mode(u8 mode1, u8 mode2)
{
	ex_main_task_mode1 = mode1;
	ex_main_task_mode2 = mode2;
}
/* EOF */
