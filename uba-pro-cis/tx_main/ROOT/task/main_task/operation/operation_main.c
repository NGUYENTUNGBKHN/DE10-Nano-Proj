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
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "operation.h"
#include "sensor.h"

#define EXT
#include "com_ram.c"


/************************** Function Prototypes ******************************/
/************************** Variable declaration *****************************/
extern T_MSG_BASIC ex_main_msg;
/*********************************************************************//**
 * @brief operation main
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void operation_main(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

#if defined(UBA_TEST)	/* 2021-12-17 */
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_STANDBY, 0, 0, 0);
//	do{
		OSW_TSK_sleep(3000);
//	}while(1);


#endif

#if defined(UBA_RTQ)
	/* Timer check status RC */
	_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
#endif // UBA_RTQ

	_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_READ_REQ, FRAM_ALL, 0, 0, 0);
	_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_FRAM_READ);
	while(1)
	{
		ercd = trcv_mbx(ID_MAIN_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		if(ercd == E_OK)
		{
			memcpy(&ex_main_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(ex_main_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_main_system_error(1, 44);
			}
			operation_main_msg_proc();
		}

		if(ex_100msec_motor == 0) //2024-11-21 1core
		{
			_pl_gpio_set_enmt_uba(0);
			dly_tsk(2);
			_pl_gpio_set_enmt_uba(1);
			ex_100msec_motor = 30;	/*3s*/
		}
	}
}


/*********************************************************************//**
 * @brief main_task message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void operation_main_msg_proc(void)
{
	switch (ex_main_task_mode1)
	{
	case MODE1_POWERUP:
		powerup_msg_proc();
		break;
	case MODE1_INIT:
		initialize_msg_proc();
		break;
	case MODE1_ADJUST:
		adjust_msg_proc();
		break;
	case MODE1_DISABLE:
		disable_msg_proc();
		break;
	case MODE1_ENABLE:
		enable_msg_proc();
		break;
	case MODE1_ACTIVE_DISABLE:
		active_disable_msg_proc();
		break;
	case MODE1_ACTIVE_ENABLE:
		active_enable_msg_proc();
		break;
	case MODE1_ACCEPT:
		accept_msg_proc();
		break;
	case MODE1_STACK:
		stack_msg_proc();
		break;
	case MODE1_REJECT:
		reject_msg_proc();
		break;
	case MODE1_REJECT_STANDBY:
		reject_standby_msg_proc();
		break;
	case MODE1_ALARM:
		alarm_msg_proc();
		break;
	case MODE1_ACTIVE_ALARM:
		active_alarm_msg_proc();
		break;
	case MODE1_TEST_STANDBY:
		test_standby_msg_proc();
		break;
	case MODE1_TEST_ACTIVE:
		test_active_msg_proc();
		break;
#if defined(UBA_RTQ)
	case MODE1_COLLECT:
		collect_msg_proc();
		break;
	case MODE1_PAYOUT:
		payout_msg_proc();
		break;
#endif // UBA_RTQ
	default:
		/* system error ? */
		_main_system_error(0, 45);
		break;
	}
}
/* EOF */
