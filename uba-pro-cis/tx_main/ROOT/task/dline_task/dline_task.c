/******************************************************************************/
/*! @addtogroup Main
    @file       dline_task.c
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
#include "hal_gpio.h"
#include "hal_bezel.h"
#include "sub_functions.h"
#include "jdl_conf.h"

#if defined(UBA_RTQ)
#include "if_rc.h"
#endif // UBA_RTQ

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

#include "dline_test.h"
#include "dline_suite.h"

/************************** PRIVATE DEFINITIONS *************************/
#define IS_OTG_EVT_DOWNLOAD_READY(x)	((x & EVT_OTG_DOWNLOAD_READY) == EVT_OTG_DOWNLOAD_READY)

enum _DLINE_CON_STATE
{
	/* Not connected(rear enable) */
	DLINE_CON_STATE_DISCONNECTED = 0,
	/* Rear USB Connected. Work as a Composite Device(CDC*2) */
	DLINE_CON_STATE_REAR_CONNECTED,
	/* Front USB Connected. Work as OTG Host(Storage) & Device(CDC) */
	DLINE_CON_STATE_FRONT_CONNECTED,
};
/************************** EXTERNAL VARIABLES *************************/

/************************** EXTERNAL FUNCTIONS *************************/
extern void dline_send_data(void);
extern void set_response_1data(u8 cmd);
extern void front_usb_suite_initial(void);

/************************** Function Prototypes ******************************/
void dline_task(VP_INT exinf);
void _dline_initialize(void);
void _dline_rcv_msg_proc(void);
void _dline_clear_buffer(void);
void _dline_msg_proc_timesup(void);

void _dline_msg_proc_timesup_data_wait(void);

void _dline_msg_proc_callback(void);
void _dline_msg_proc_callback_connect(void);
void _dline_msg_proc_callback_empty(void);
void _dline_msg_proc_callback_receive(void);
#if defined(USB_REAR_USE)
void _dline_msg_proc_callback2(void);
void _dline_msg_proc_callback_connect2(void);
void _dline_msg_proc_callback_empty2(void);
void _dline_msg_proc_callback_receive2(void);
#endif
void _dline_msg_proc_otg_notice(void);
void _dline_msg_proc_otg_notice_download_ready(void);
void _dline_msg_proc_otg_notice_connect(void);

void _dline_msg_proc_status(void);
void _dline_check_ogt_evt(void);

void _dline_set_con_state(u16 state);
void _dline_set_mode(u16 mode);
void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _dline_system_error(u8 fatal_err, u8 code);

/************************** Variable declaration *****************************/
static T_MSG_BASIC dline_msg;

u16 ex_dline_con_state;


void _dline_msg_proc_test_sw_notice(void);
void _dline_idel_proc(void)
{

	switch(ex_dline_task_mode)
	{
	case DLINE_MODE_TEST_STANDBY:

	case DLINE_MODE_TEST_ERROR:
	case DLINE_MODE_ATEST_ERROR:
	case DLINE_MODE_TEST_EXEC:

	case DLINE_MODE_ATEST_ENABLE:
		_dline_msg_proc_test_sw_notice();	/* check DIP-SW */
		break;

	case DLINE_MODE_NORMAL:
	case DLINE_MODE_ATEST_INITIAL:
	case DLINE_MODE_ATEST_ACCEPT:
	case DLINE_MODE_ATEST_STACK:
	case DLINE_MODE_ATEST_REJECT:
	case DLINE_MODE_PTEST_PAYOUT:
	case DLINE_MODE_PTEST_COLLECT:
	default:
		/* system error ? */
		break;
	}

}

/*******************************
        dline_task
 *******************************/
void dline_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_dline_initialize();

#if (_DEBUG_TEST_MODE_AUTO_RESET==1)
	if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
	{
		ex_dline_testmode.action = TEST_USB_CONTROL;
		ex_dline_testmode.test_no = TEST_ACCEPT;
		_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
	}
#elif (_DEBUG_TEST_MODE_AUTO_ICB_TEST==1)
	if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
	{
		ex_dline_testmode.action = TEST_USB_CONTROL;
		ex_dline_testmode.test_no = TEST_AGING;
		_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
	}
#elif (_DEBUG_TEST_MODE_AUTO_AGING_TEST==1)
	if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
	{
		ex_dline_testmode.action = TEST_USB_CONTROL;
		ex_dline_testmode.test_no = TEST_AGING;
		_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
	}
#endif

	while (1)
	{
		ercd = trcv_mbx(ID_DLINE_MBX, (T_MSG **)&tmsg_pt, 200);
		if (ercd == E_OK)
		{
			memcpy(&dline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(dline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_dline_system_error(1, 4);
			}
			_dline_rcv_msg_proc();
		}
		_dline_idel_proc();
		_dline_check_ogt_evt();
	}
}

/******************************************************************************/
/*! @brief Front USB device initialize
    @return         none
    @exception      none
******************************************************************************/
void _dline_initialize(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 wait_init = 1;

	/* Wait TMSG_DLINE_INITIAL_REQ */
	while (wait_init)
	{
		ercd = rcv_mbx(ID_DLINE_MBX, (T_MSG **)&tmsg_pt);
		if (ercd == E_OK)
		{
			memcpy(&dline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(dline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_dline_system_error(1, 3);
			}
			if (dline_msg.tmsg_code == TMSG_DLINE_INITIAL_REQ)
			{
				if (dline_msg.arg1 & OPERATING_MODE_TEST)
				{
				/* Performance Test Mode */
					_dline_set_mode(DLINE_MODE_TEST_STANDBY);
				}
				else
				{
				/* Normal Operation(I/F) Mode */
					_dline_set_mode(DLINE_MODE_NORMAL);
				}
				wait_init = 0;
			}
		}
	}

	memset((void *)&ex_front_usb, 0, sizeof(ex_front_usb));
	/* debug */
	ex_front_usb.pc.mess.serviceID = 0x03;
	ex_front_usb.status = DLINE_ANALYZE;
	front_usb_suite_initial();

	ex_usb_read_size = 0;
	_dline_clear_buffer();

	ex_dline_testmode.action = TEST_NON_ACTION;
	ex_dline_testmode.test_no = 0;
	ex_monitor_info.data_exist = false;



#if 1//#if (FIX_FRONT_USB_USE!=1)
	if(is_fusb_dect_on())
	{
		FrontUSBConnect();
	}
	else
	{
		RearUSBConnect();
	}
#endif
	act_tsk(ID_OTG_TASK);

	if(ex_dline_task_mode!=DLINE_MODE_NORMAL)
	{
		act_tsk(ID_SUBLINE_TASK);
	}
}

#if defined(UBA_RTQ)
void _dline_msg_proc_fusb_dect_notice(void)
{
//	_dline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_FUSB_BOUNCE, WAIT_TIME_FUSB_BOUNCE, 0, 0);
}
#endif // uBA_RTQ

void _dline_msg_proc_test_sw_notice(void)
{
	if((ex_dline_testmode.action == TEST_SW_CONTROL)
	 && (ex_dline_testmode.test_no != TEST_STANDBY))
	{
		if (ex_dipsw1 & DIPSW1_PERFORMANCE_TEST) //ok
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			reset_dline_sw_test();
		}
	}
	else if((ex_dline_testmode.action == TEST_NON_ACTION)
	 && (ex_dline_testmode.test_no == TEST_STANDBY))
	{
		select_dline_sw_test();
	}
}
void _dline_msg_proc_timesup(void)
{
	switch(dline_msg.arg1)
	{
	case TIMER_ID_DATA_WAIT:
		 _dline_msg_proc_timesup_data_wait();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 5);
		break;
	}
}

void _dline_clear_buffer(void)
{
	ex_usb_write_size = 0;
	ex_usb_read_size = 0;
	memset((void *)&ex_dline_pkt, 0, sizeof(ex_dline_pkt));
}
void _dline_msg_proc_timesup_data_wait(void)
{
	_dline_clear_buffer();
}

void _dline_msg_proc_status(void)
{
	if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
#if 1
		_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
#else
		_dline_set_mode(DLINE_MODE_TEST_STANDBY);
#endif
	}
	else if (dline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_dline_set_mode(DLINE_MODE_ATEST_ERROR);
	}
}


/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_nomal(void)
{
	switch (dline_msg.tmsg_code)
	{
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_test_standby(void)
{
	switch (dline_msg.tmsg_code)
	{
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_ENABLE_RSP:
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_REJECT_RSP:
	case TMSG_DLINE_STACK_RSP:
	case TMSG_DLINE_TEST_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_DLINE_TEST_DIPSW_RSP:
	#if 1 //2025-09-04 ここでテストモードへ遷移させるのがUBA500に近い
		ex_dline_testmode.action = TEST_SW_CONTROL;
		_dline_set_mode(DLINE_MODE_TEST_EXEC);		// その他のテストモードへ
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
	#endif
		break;
#endif 
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_test_exec(void)
{
	switch (dline_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_dline_set_mode(DLINE_MODE_TEST_STANDBY);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_TEST_ERROR);
		}
		break;
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_ENABLE_RSP:
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_REJECT_RSP:
	case TMSG_DLINE_STACK_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_atest_initial(void)
{
	switch (dline_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/*  */
		}
		else if (dline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (dline_msg.arg1 == TMSG_SUB_REJECT)
		{
			/*  */
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else
		{
			/* system error ? */
			_dline_system_error(0, 20);
		}
		break;
	case TMSG_DLINE_REJECT_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (dline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_REJECT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else if (dline_msg.arg1 == TMSG_SUB_REMAIN)
		{
			/* for ID-0G8  */
		}
		else
		{
			/* system error ? */
			_dline_system_error(0, 20);
		}
		break;
	case TMSG_DLINE_RESET_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
			_dline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_DEMO, 0, 0, 0, 0);
		}
		else if ((dline_msg.arg1 == TMSG_SUB_REJECT)
			   || (dline_msg.arg1 == TMSG_SUB_REMAIN))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_REJECT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else
		{
			/* system error ? */
			_dline_system_error(1, 1);
		}
		break;
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_STACK_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_atest_enable(void)
{
	u8 remain = 0;

	switch (dline_msg.tmsg_code)
	{
	case TMSG_DLINE_ENABLE_RSP:
		if (dline_msg.arg1 == TMSG_SUB_ACCEPT)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ACCEPT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ACCEPT_REQ, 0, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_dline_set_mode(DLINE_MODE_ATEST_REJECT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_REJECT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
#if defined(UBA_RTQ) /* Aging mode use */
		else if (dline_msg.arg1 == TMSG_SUB_PAYOUT)
		{
			_dline_set_mode(DLINE_MODE_PTEST_PAYOUT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_PAYOUT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_COLLECT)
		{
			/* RC-Quad model */
			if(ex_rc_status.sst1A.bit.quad)
			{
				if (!is_rc_twin_d1_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM1;
					remain = 1;
				}
				else if (!is_rc_twin_d2_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM2;
					remain = 1;
				}
				else if (!is_rc_quad_d1_empty())
				{
					OperationDenomi.unit = RC_QUAD_DRUM1;
					remain = 1;
				}
				else if (!is_rc_quad_d2_empty())
				{
					OperationDenomi.unit = RC_QUAD_DRUM2;
					remain = 1;
				}
				else
				{
					remain = 0;					
				}

				if(remain)
				{
					_dline_set_mode(DLINE_MODE_PTEST_COLLECT);
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_COLLECT_REQ, 0, 0, 0, 0);
					_dline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
				}
				else
				{
					_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
					_dline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_LED_ON, 0, 0, 0, 0);
				}
			}
			/* RC-Twin model */
			else
			{
				if (!is_rc_twin_d1_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM1;
					remain = 1;
				}
				else if (!is_rc_twin_d2_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM2;
					remain = 1;
				}
				else
				{
					remain = 0;					
				}

				if(remain)
				{
					_dline_set_mode(DLINE_MODE_PTEST_COLLECT);
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_COLLECT_REQ, 0, 0, 0, 0);
					_dline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
				}
				else
				{
					_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
					_dline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0, 0, 0, 0);
				}
			}
		}
#endif
		else
		{
			/* system error ? */
			_dline_system_error(1, 4);
		}
		break;
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_REJECT_RSP:
	case TMSG_DLINE_STACK_RSP:
	case TMSG_DLINE_TEST_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_atest_accept(void)
{
	switch (dline_msg.tmsg_code)
	{
#if 1 /* 2022-03-02 *//*  *//* 連続動作暫定対応 */
	/* mode_stackeからmode_acceptの搬送起動ではなく、一時mode_enableにいれる為、*/
	/* この処理がないと、ACTIVE_ENABLE_MODE2_WAIT_REQでlineからのメッセージ待ちで待ち続ける為 */
	case TMSG_DLINE_ENABLE_RSP:
		if (dline_msg.arg1 == TMSG_SUB_ACCEPT)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ACCEPT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ACCEPT_REQ, 0, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_dline_set_mode(DLINE_MODE_ATEST_REJECT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_REJECT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else
		{
			/* system error ? */
			_dline_system_error(1, 4);
		}
		break;
#endif
	case TMSG_DLINE_ACCEPT_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_dline_set_mode(DLINE_MODE_ATEST_STACK);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_STACK_REQ, 0, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_dline_set_mode(DLINE_MODE_ATEST_REJECT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_REJECT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else
		{
			/* system error ? */
			_dline_system_error(0, 26);
		}
		break;
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_REJECT_RSP:
	case TMSG_DLINE_STACK_RSP:
	case TMSG_DLINE_TEST_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_atest_stack(void)
{
	switch (dline_msg.tmsg_code)
	{
	case TMSG_DLINE_STACK_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_dline_set_mode(DLINE_MODE_ATEST_REJECT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_REJECT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_STACKING_ENTRY_ON)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ACCEPT);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ACCEPT_REQ, 0, 0, 0, 0);
		}	
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else if ((dline_msg.arg1 != TMSG_SUB_PAUSE)
			  && (dline_msg.arg1 != TMSG_SUB_RESUME)
			  && (dline_msg.arg1 != TMSG_SUB_VEND))
		{
			/* system error ? */
			_dline_system_error(0, 29);
		}
		break;
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_ENABLE_RSP:
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_REJECT_RSP:
	case TMSG_DLINE_TEST_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
/*********************************************************************//**
 * @brief process of receive message sub function
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc_mode_atest_reject(void)
{
	switch (dline_msg.tmsg_code)
	{
	case TMSG_DLINE_REJECT_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (dline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_REJECT_REQ, dline_msg.arg2, 0, 0, 0);
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else if (dline_msg.arg1 == TMSG_SUB_REMAIN)
		{
			/* for ID-0G8  */
		}
		else
		{
			/* system error ? */
			_dline_system_error(1, 4);
		}
		break;
#if defined(USB_REAR_USE)
	case TMSG_USB2CB_CALLBACK_INFO:
		_dline_msg_proc_callback2();
		break;
#endif // USB_REAR_USE
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_ENABLE_RSP:
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_STACK_RSP:
	case TMSG_DLINE_TEST_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}

#if defined(UBA_RTQ)
void _dline_rcv_msg_proc_mode_atest_payout(void)
{
	switch (dline_msg.tmsg_code)
	{
	case TMSG_DLINE_PAYOUT_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if (--OperationDenomi.remain == 0)
			{
				_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
				_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
			}
			else
			{
				_dline_set_mode(DLINE_MODE_PTEST_PAYOUT);
				_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_PAYOUT_REQ, dline_msg.arg2, 0, 0, 0);
			}
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		break;
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_FUSB_DECT_NOTICE:
		_dline_msg_proc_fusb_dect_notice();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_ENABLE_RSP:
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_REJECT_RSP:
	case TMSG_DLINE_TEST_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}

void _dline_rcv_msg_proc_mode_atest_collect(void)
{
	u8 remain = 0;
	switch (dline_msg.tmsg_code)
	{
	case TMSG_DLINE_COLLECT_RSP:
		if (dline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* RC-Quad model */
			if(ex_rc_status.sst1A.bit.quad)
			{
				if (!is_rc_twin_d1_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM1;
					remain = 1;
				}
				else if (!is_rc_twin_d2_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM2;
					remain = 1;
				}
				else if (!is_rc_quad_d1_empty())
				{
					OperationDenomi.unit = RC_QUAD_DRUM1;
					remain = 1;
				}
				else if (!is_rc_quad_d2_empty())
				{
					OperationDenomi.unit = RC_QUAD_DRUM2;
					remain = 1;
				}
				else
				{
					remain = 0;
				}
				if (remain)
				{
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_COLLECT_REQ, 0, 0, 0, 0);
				}
				else
				{
					_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
					_dline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0, 0, 0, 0);
				}
			}
			else
			{
				if (!is_rc_twin_d1_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM1;
					remain = 1;
				}
				else if (!is_rc_twin_d2_empty())
				{
					OperationDenomi.unit = RC_TWIN_DRUM2;
					remain = 1;
				}
				else
				{
					remain = 0;
				}
				if (remain)
				{
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_COLLECT_REQ, 0, 0, 0, 0);
				}
				else
				{
					_dline_set_mode(DLINE_MODE_ATEST_ENABLE);
					_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ENABLE_REQ, 0, 0, 0, 0);
					_dline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0, 0, 0, 0);
				}
			}
		}
		else if (dline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_dline_set_mode(DLINE_MODE_ATEST_ERROR);
		}
		else if (dline_msg.arg1 != TMSG_SUB_COLLECTED)
		{
			/* system error ? */
			_dline_system_error(0, 26);
		}
		break;
	case TMSG_USB0CB_CALLBACK_INFO:
		_dline_msg_proc_callback();
		break;
	case TMSG_FUSB_DECT_NOTICE:
		_dline_msg_proc_fusb_dect_notice();
		break;
	case TMSG_OTG_NOTICE:
		_dline_msg_proc_otg_notice();
		break;
	case TMSG_TIMER_TIMES_UP:
		 _dline_msg_proc_timesup();
		 break;
	case TMSG_DLINE_STATUS_INFO:
		 _dline_msg_proc_status();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	case TMSG_DLINE_RESET_RSP:
	case TMSG_DLINE_ENABLE_RSP:
	case TMSG_DLINE_ACCEPT_RSP:
	case TMSG_DLINE_REJECT_RSP:
	case TMSG_DLINE_TEST_RSP:
	default:				/* other */
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
#endif // UBA_RTQ
/*********************************************************************//**
 * @brief process of receive message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_rcv_msg_proc(void)
{
	switch(ex_dline_task_mode)
	{
	case DLINE_MODE_NORMAL:
		_dline_rcv_msg_proc_mode_nomal();
		break;
	case DLINE_MODE_TEST_STANDBY:

	case DLINE_MODE_TEST_ERROR:
	case DLINE_MODE_ATEST_ERROR:
		_dline_rcv_msg_proc_mode_test_standby();	/* DIP-SW対応 */
		break;
	case DLINE_MODE_TEST_EXEC:
		_dline_rcv_msg_proc_mode_test_exec();		/* DIP-SW対応 */
		break;
	case DLINE_MODE_ATEST_INITIAL:
		_dline_rcv_msg_proc_mode_atest_initial();
		break;
	case DLINE_MODE_ATEST_ENABLE:
		_dline_rcv_msg_proc_mode_atest_enable();	/* DIP-SW対応 */
		break;
	case DLINE_MODE_ATEST_ACCEPT:
		_dline_rcv_msg_proc_mode_atest_accept();
		break;
	case DLINE_MODE_ATEST_STACK:
		_dline_rcv_msg_proc_mode_atest_stack();
		break;
	case DLINE_MODE_ATEST_REJECT:
		_dline_rcv_msg_proc_mode_atest_reject();
		break;
#if defined(UBA_RTQ)
	case DLINE_MODE_PTEST_PAYOUT:
		_dline_rcv_msg_proc_mode_atest_payout();
		break;
	case DLINE_MODE_PTEST_COLLECT:
		_dline_rcv_msg_proc_mode_atest_collect();
		break;
#else
	case DLINE_MODE_PTEST_PAYOUT:
	case DLINE_MODE_PTEST_COLLECT:

#endif // UBA_RTQ
	default:
		/* system error ? */
		_dline_system_error(1, 1);
		break;
	}
}
/*********************************************************************//**
 * @brief process of USB0 callback notice message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_msg_proc_callback(void)
{
	switch (dline_msg.arg1)
	{
	case TMSG_SUB_CONNECT:
		_dline_msg_proc_callback_connect();
		break;
	case TMSG_SUB_RECEIVE:
		_dline_msg_proc_callback_receive();
		break;
	case TMSG_SUB_EMPTY:
		_dline_msg_proc_callback_empty();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_connect(void)
{
	switch (ex_dline_con_state)
	{
	case DLINE_CON_STATE_REAR_CONNECTED:
	case DLINE_CON_STATE_FRONT_CONNECTED:
		if(!dline_msg.arg2)
		{
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_DISCONNECTED);
		}
		break;
	case DLINE_CON_STATE_DISCONNECTED:
		if(dline_msg.arg2)
		{
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_FRONT_CONNECTED);
		}
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_empty(void)
{
	if(ex_collection_data.data_exist == DATA_REQEST)
	{
		ex_collection_data.data_exist = DATA_NONE;
	}
	/* usb download */
	if(ex_suite_item.curent_service == SUITE_ITEM_DOWNLOAD)
	{
		while (((ex_main_task_mode1 == MODE1_INIT) && (ex_main_task_mode2 != INIT_MODE2_WAIT_REQ))
		|| (ex_main_task_mode1 == MODE1_ACCEPT)
		|| (ex_main_task_mode1 == MODE1_STACK)
		|| (ex_main_task_mode1 == MODE1_REJECT)
	#if defined(UBA_RTQ)
		|| (ex_main_task_mode1 == MODE1_PAYOUT)
	#endif
		){
			dly_tsk(100);
		}
		while(ex_usb_write_busy)
		{
			dly_tsk(1);
		}
		UINT32 isr_stat = OSW_ISR_global_disable();
		terminate_main_sys();

		// GOTO BIF
		bif_device_usb_download_smp();
		OSW_ISR_global_restore( isr_stat );
	}
}
extern void dline_suite(void);
void _dline_msg_proc_callback_receive(void)
{
	dline_suite();
}

#if defined(USB_REAR_USE)
/*********************************************************************//**
 * @brief process of USB0 callback notice message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _dline_msg_proc_callback2(void)
{
	switch (dline_msg.arg1)
	{
	case TMSG_SUB_CONNECT:
		_dline_msg_proc_callback_connect2();
		break;
	case TMSG_SUB_RECEIVE:
		_dline_msg_proc_callback_receive2();
		break;
	case TMSG_SUB_EMPTY:
		_dline_msg_proc_callback_empty2();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_connect2(void)
{
	switch (ex_dline_con_state)
	{
	case DLINE_CON_STATE_REAR_CONNECTED:
	case DLINE_CON_STATE_FRONT_CONNECTED:
		if(!dline_msg.arg2)
		{
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_DISCONNECTED);
		}
		break;
	case DLINE_CON_STATE_DISCONNECTED:
		if(dline_msg.arg2)
		{
			_dline_clear_buffer();
			_dline_set_con_state(DLINE_CON_STATE_FRONT_CONNECTED);
		}
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}
void _dline_msg_proc_callback_empty2(void)
{
	if(ex_collection_data.data_exist == DATA_REQEST)
	{
		ex_collection_data.data_exist = DATA_NONE;
	}
	/* usb download */
	if(ex_suite_item.curent_service == SUITE_ITEM_DOWNLOAD)
	{
		while (((ex_main_task_mode1 == MODE1_INIT) && (ex_main_task_mode2 != INIT_MODE2_WAIT_REQ))
		|| (ex_main_task_mode1 == MODE1_ACCEPT)
		|| (ex_main_task_mode1 == MODE1_STACK)
		|| (ex_main_task_mode1 == MODE1_REJECT)
	#if defined(UBA_RTQ)
		|| (ex_main_task_mode1 == MODE1_PAYOUT)
	#endif
		){
			dly_tsk(100);
		}
		while(ex_usb_write_busy)
		{
			dly_tsk(1);
		}
		UINT32 isr_stat = OSW_ISR_global_disable();
		terminate_main_sys();

		// GOTO BIF
		bif_device_usb_download_smp();
		OSW_ISR_global_restore( isr_stat );
	}
}
extern void dline_suite2(void);
void _dline_msg_proc_callback_receive2(void)
{
	dline_suite2();
}
#endif 

void _dline_msg_proc_otg_notice(void)
{
	switch (dline_msg.arg1)
	{
	case TMSG_SUB_DOWNLOAD:
		_dline_msg_proc_otg_notice_download_ready();
		break;
	case TMSG_SUB_CONNECT:
		_dline_msg_proc_otg_notice_connect();
		break;
	default:
		/* system error ? */
		_dline_system_error(1, 4);
		break;
	}
}

void _dline_check_ogt_evt(void)
{
	FLGPTN flag = 0;
	ER ercd;
	/* usb download */
	ercd = pol_flg(ID_OTG_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag);
	if ((ercd == E_OK) && (flag != 0))
	{
		clr_flg(ID_OTG_CTRL_FLAG, ~EVT_ALL_BIT);
		if (IS_OTG_EVT_DOWNLOAD_READY(flag))
		{
			while (((ex_main_task_mode1 == MODE1_INIT) && (ex_main_task_mode2 != INIT_MODE2_WAIT_REQ))
			|| (ex_main_task_mode1 == MODE1_ACCEPT)
			|| (ex_main_task_mode1 == MODE1_STACK)
			|| (ex_main_task_mode1 == MODE1_REJECT)
		#if defined(UBA_RTQ)
			|| (ex_main_task_mode1 == MODE1_PAYOUT)
		#endif
			){
				dly_tsk(100);
			}
			UINT32 isr_stat = OSW_ISR_global_disable();
			terminate_main_sys();
			// GOTO BIF
			bif_host_usb_download_smp();
			OSW_ISR_global_restore( isr_stat );
		}
	}
	/* usb download */
	if(ex_suite_item.curent_service == SUITE_ITEM_DOWNLOAD)
	{
		while (((ex_main_task_mode1 == MODE1_INIT) && (ex_main_task_mode2 != INIT_MODE2_WAIT_REQ))
		|| (ex_main_task_mode1 == MODE1_ACCEPT)
		|| (ex_main_task_mode1 == MODE1_STACK)
		|| (ex_main_task_mode1 == MODE1_REJECT)
	#if defined(UBA_RTQ)
		|| (ex_main_task_mode1 == MODE1_PAYOUT)
	#endif
		){
			dly_tsk(100);
		}
		while(ex_usb_write_busy)
		{
			dly_tsk(1);
		}
		UINT32 isr_stat = OSW_ISR_global_disable();
		terminate_main_sys();

		// GOTO BIF
		bif_device_usb_download_smp();
		OSW_ISR_global_restore( isr_stat );
	}
}
void _dline_msg_proc_otg_notice_download_ready(void)
{
	while (((ex_main_task_mode1 == MODE1_INIT) && (ex_main_task_mode2 != INIT_MODE2_WAIT_REQ))
	|| (ex_main_task_mode1 == MODE1_ACCEPT)
	|| (ex_main_task_mode1 == MODE1_STACK)
	|| (ex_main_task_mode1 == MODE1_REJECT)
#if defined(UBA_RTQ)
	|| (ex_main_task_mode1 == MODE1_PAYOUT)
#endif
	){
		dly_tsk(100);
	}
	UINT32 isr_stat = OSW_ISR_global_disable();
	terminate_main_sys();

	// GOTO BIF
	bif_host_usb_download_smp();
	OSW_ISR_global_restore( isr_stat );
}

void _dline_msg_proc_otg_notice_connect(void)
{
	if(!dline_msg.arg2)
	{
		/* check download file */
	}
}

void _dline_set_con_state(u16 state)
{
	ex_dline_con_state = state;
}

void _dline_set_mode(u16 mode)
{
	ex_dline_task_mode = mode;

#ifdef _ENABLE_JDL
    jdl_add_trace(ID_DLINE_TASK, ((ex_dline_task_mode >> 8) & 0xFF), (ex_dline_task_mode & 0xFF), ex_dline_testmode.action, ex_dline_testmode.option, ex_dline_testmode.test_no);
#endif /* _ENABLE_JDL */
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
void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_DLINE_TASK;
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
			_dline_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_dline_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _dline_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_dline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_dline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */
	_debug_system_error(ID_DLINE_TASK, (u16)code, (u16)dline_msg.tmsg_code, (u16)dline_msg.arg1, fatal_err);
}

/* EOF */
