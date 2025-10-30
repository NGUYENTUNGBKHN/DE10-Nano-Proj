/******************************************************************************/
/*! @addtogroup Main
    @file       power_task.c
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
enum _POWER_MODE
{
	POWER_MODE_INIT = 1,
	POWER_MODE_ACTIVE,
};
#define IS_POWER_EVT_VOLTAGE(x)	((x & EVT_POWER_VOLTAGE) == EVT_POWER_VOLTAGE)
#define IS_POWER_EVT_RESET(x)	((x & EVT_EXTERNAL_RESET) == EVT_EXTERNAL_RESET)
#define IS_POWER_EVT_PLL_LOCK(x)	((x & EVT_PLL_LOCK) == EVT_PLL_LOCK)
/************************** Function Prototypes ******************************/
void power_task(VP_INT exinf);
void _power_proc(FLGPTN flg);
void power_task_initialize(void);
void power_port_initialize(void);
void _power_task_set_mode(u16 mode);
void _power_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _power_system_error(u8 fatal_err, u8 code);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
static T_MSG_BASIC power_msg;

/*******************************
        power_task
 *******************************/
void power_task(VP_INT exinf)
{
	FLGPTN flag = 0;
	ER ercd;

	/* Initialize Variables */
	power_task_initialize();
	/* Initialize GPIO, enable Interrupt */
	power_port_initialize();

	while (1)
	{
		ercd = twai_flg(ID_POWER_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TMO_FEVR);
		if ((ercd == E_OK) && (flag != 0))
		{
			clr_flg(ID_POWER_FLAG, ~EVT_ALL_BIT);
			_power_proc(flag);
		}
	}
}

/******************************************************************************/
/*! @brief Power Fail TASK initialize
    @return         none
    @exception      none
******************************************************************************/
void power_task_initialize(void)
{
	_power_task_set_mode(POWER_MODE_INIT);
}

/******************************************************************************/
/*! @brief Power Fail GPIO port initialize
    @return         none
    @exception      none
******************************************************************************/
void power_port_initialize(void)
{
	init_external_reset();
	init_power_fail();
	_pl_pll_init();
}

/******************************************************************************/
/*! @brief Power Fail GPIO change event
 *  @param[in]	    flg event flag
    @return         none
    @exception      none
******************************************************************************/
void _power_proc(FLGPTN flg)
{
	FLGPTN local = flg;

	if(IS_POWER_EVT_VOLTAGE(local))
	{
		_power_task_set_mode(POWER_MODE_ACTIVE);
		_power_send_msg(ID_FRAM_MBX, TMSG_POWER_NOTICE, 0, 0, 0, 0);
	}

	if(IS_POWER_EVT_RESET(local))
	{
		_power_task_set_mode(POWER_MODE_ACTIVE);
		_power_send_msg(ID_FRAM_MBX, TMSG_RESET_NOTICE, 0, 0, 0, 0);
	}
	if(IS_POWER_EVT_PLL_LOCK(local))
	{
		_power_task_set_mode(POWER_MODE_ACTIVE);
		_power_send_msg(ID_FRAM_MBX, TMSG_PLL_LOCK, 0, 0, 0, 0);
	}

	_power_task_set_mode(POWER_MODE_INIT);
}


/******************************************************************************/
/*! @brief Chage Power Fail TASK mode
    @return         none
    @exception      none
******************************************************************************/
void _power_task_set_mode(u16 mode)
{
	ex_power_task_mode = mode;
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
void _power_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_POWER_TASK;
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
			_power_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_power_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _power_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_power_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_power_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_POWER_TASK, (u16)code, (u16)power_msg.tmsg_code, (u16)power_msg.arg1, fatal_err);
}

/* EOF */
