/******************************************************************************/
/*! @addtogroup Main
    @file       cline_task.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * cline_task.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/************************** PRIVATE DEFINITIONS *************************/

/************************** Function Prototypes ******************************/
void cline_task(VP_INT exinf);
void _cline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _cline_system_error(u8 fatal_err, u8 code);

/************************** External functions *******************************/
#if defined(_PROTOCOL_ENABLE_ID003)
extern u8 id003_main(void);
extern void interface_DeInit_id003(void);
#elif defined(_PROTOCOL_ENABLE_ID0G8)
extern u8 id0g8_main(void);
//extern void interface_DeInit_id0G8(void);
#else
extern u8 id003_main(void);
extern void interface_DeInit_id003(void);
#endif /* _PROTOCOL_ENABLE_ID003 */
/************************** Variable declaration *****************************/
extern T_MSG_BASIC cline_msg;


/*******************************
        cline_task
 *******************************/
void cline_task(VP_INT exinf)
{
	switch (ex_cline_status_tbl.protocol_select)
	{
#if defined(_PROTOCOL_ENABLE_ID003)
	case PROTOCOL_SELECT_ID003:
		id003_main();
		break;
#elif defined(_PROTOCOL_ENABLE_ID0G8)
	case PROTOCOL_SELECT_ID0G8:
		id0g8_main();
		break;
#else
	default:
		id003_main();
		break;
#endif /* _PROTOCOL_ENABLE_ID003 */
	}
}



/*********************************************************************//**
 * @brief		Disable interface for Download
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void interface_DeInit(void)
{
	switch(ex_cline_status_tbl.protocol_select)
	{
#if defined(_PROTOCOL_ENABLE_ID003)
	case PROTOCOL_SELECT_ID003:
		interface_DeInit_id003();
		break;
#elif defined(_PROTOCOL_ENABLE_ID0G8)
	case PROTOCOL_SELECT_ID0G8:
		//interface_DeInit_id0g8();
		break;
#else
	default:
		interface_DeInit_id003();
		break;
#endif
	}
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
void _cline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_CLINE_TASK;
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
			_cline_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_cline_system_error(1, 2);
	}
}

/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _cline_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_CLINE_TASK, (u16)code, (u16)cline_msg.tmsg_code, (u16)cline_msg.arg1, fatal_err);
}
/* EOF */
