/******************************************************************************/
/*! @addtogroup Main
    @file       fusb_det_task_task.c
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
#include "hal_gpio.h"
#include "hal_bezel.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS *************************/
enum _FUSB_DET_MODE
{
	/* Not Initialize */
	FUSB_DET_MODE_UNKNOWN = 0,
	/* Not Initialize */
	FUSB_DET_MODE_DISCONNECT,
	FUSB_DET_MODE_CONNECT,
};
#define DEBOUNCE_COUNT 3

/************************** Function Prototypes ******************************/
void fusb_det_task(VP_INT exinf);
void _fusb_det_proc(FLGPTN flg);
void _fusb_idle_proc(void);
void fusb_det_task_initialize(void);
void fusb_det_port_initialize(void);
void _fusb_det_task_set_mode(u16 mode);
void _fusb_det_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _fusb_det_system_error(u8 fatal_err, u8 code);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
typedef struct _PORT_STATUS
{
	u32 change_count;
	u32 current_value;
}PORT_STATUS;
PORT_STATUS fusb_det_status;
u16 ex_fusb_det_task_mode;
static T_MSG_BASIC fusb_det_msg;

/*******************************
        fusb_det_task
 *******************************/
void fusb_det_task(VP_INT exinf)
{
	FLGPTN flag = 0;
	ER ercd;

	/* Initialize Variables */
	fusb_det_task_initialize();
	/* Initialize GPIO, enable Interrupt */
	fusb_det_port_initialize();

	while (1)
	{
		ercd = twai_flg(ID_FUSB_DET_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TASK_WAIT_TIME);
		if ((ercd == E_OK) && (flag != 0))
		{
			clr_flg(ID_FUSB_DET_FLAG, ~EVT_ALL_BIT);
			//_fusb_det_proc(flag);
		}
		_fusb_idle_proc();
	}
}

/******************************************************************************/
/*! @brief Front USB detect TASK initialize
    @return         none
    @exception      none
******************************************************************************/
void fusb_det_task_initialize(void)
{
}

/******************************************************************************/
/*! @brief Front USB detect GPIO port initialize
    @return         none
    @exception      none
******************************************************************************/
void fusb_det_port_initialize(void)
{
	//init_intr_fusb_dect();
	fusb_det_status.current_value = is_fusb_dect_on();
	fusb_det_status.change_count = 0;
}

/******************************************************************************/
/*! @brief Front USB detect GPIO change event
 *  @param[in]	    flg event flag
    @return         none
    @exception      none
******************************************************************************/
void _fusb_det_proc(FLGPTN flg)
{
	_fusb_det_send_msg(ID_DLINE_MBX, TMSG_FUSB_DECT_NOTICE, 0, 0, 0, 0);
}


/******************************************************************************/
/*! @brief Front USB task idle proc
 *  @param[in]	    flg event flag
    @return         none
    @exception      none
******************************************************************************/
void _fusb_idle_proc(void)
{
	u8 tmp;

	tmp = is_fusb_dect_on();
	if(tmp != fusb_det_status.current_value)
	{
		fusb_det_status.change_count++;
		if(fusb_det_status.change_count > DEBOUNCE_COUNT)
		{
			fusb_det_status.current_value = tmp;
			fusb_det_status.change_count = 0;
			_fusb_det_send_msg(ID_DLINE_MBX, TMSG_FUSB_DECT_NOTICE, 0, 0, 0, 0);
		}
	}
}


/******************************************************************************/
/*! @brief Chage Front USB detect TASK mode
    @return         none
    @exception      none
******************************************************************************/
void _fusb_det_task_set_mode(u16 mode)
{
	ex_fusb_det_task_mode = mode;
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
void _fusb_det_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_FUSB_DET_TASK;
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
			_fusb_det_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_fusb_det_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _fusb_det_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_fusb_det_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_FUSB_DET_TASK, (u16)code, (u16)fusb_det_msg.tmsg_code, (u16)fusb_det_msg.arg1, fatal_err);
}

/* EOF */
