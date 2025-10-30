/******************************************************************************/
/*! @addtogroup Main
    @file       otg_task.c
    @brief      control status led task function
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "js_oswapi.h"
#include "common.h"
#include "perid.h"
#include "peri_prm.h"
#include "peri_sts.h"
#include "peri_hal.h"
#include "hal_usb.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/
LOCAL void OTG_Mode_Change(void);
void _otg_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _otg_system_error(u8 fatal_err, u8 code);

/************************** External functions *******************************/
EXTERN void usb_host_dev_test_init(void);
/************************** Variable declaration *****************************/
static T_MSG_BASIC otg_msg;

/* Device or Host */
#define GRP_MODE_HOST			(0)
#define GRP_MODE_DEVICE			(1)

/************************** EXTERN FUNCTIONS *************************/

/*******************************
        otg_task
 *******************************/
void otg_task(VP_INT exinf)
{
	/* モード切替テスト */
	OTG_Mode_Change();
}

/************************************************************************************************/
/* FUNCTION   : OTG_Mode_Change	　　　　　　	                                                        */
/*                                                                                              */
/* DESCRIPTION: モード切り替えタスク　　　　　　　　					                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none									                                        */
/*                                                                                              */
/************************************************************************************************/
void OTG_Mode_Change(void)
{
	/* モード切り替え処理初期化 */
	reset_usb0();

	usb_host_dev_test_init();
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
void _otg_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
	{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_OTG_TASK;
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
			_otg_system_error(1, 1);
	}
	}
	else
	{
		/* system error */
		_otg_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _otg_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_otg_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_OTG_TASK, (u16)code, (u16)otg_msg.tmsg_code, (u16)otg_msg.arg1, fatal_err);
}


/* EOF */
