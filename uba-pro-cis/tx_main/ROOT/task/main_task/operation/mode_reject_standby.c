/******************************************************************************/
/*! @addtogroup Main
    @file       mode_reject.c
    @brief      reject mode of main task
    @date       2019/09/26
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2019/09/26 Development Dept at Tokyo
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
#include "sub_functions.h"

#define EXT
#include "com_ram.c"

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/
/*********************************************************************//**
 * @brief reject standby message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reject_standby_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case REJECT_STANDBY_MODE2_NOTE_STAY:
		reject_standby_note_stay(); //ok rtq
		break;
	case REJECT_STANDBY_MODE2_CONFIRM_NOTE_STAY:
		reject_standby_confirm_note_stay(); //ok rtq
		break;
	case REJECT_STANDBY_MODE2_WAIT_REJECT_REQ:
		reject_standby_wait_reject_req(); //ok rtq
		break;
	default:
		/* system error ? */
		_main_system_error(0, 164);
		break;
	}
}


/*********************************************************************//**
 * @brief note reject standby stay procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reject_standby_note_stay(void) //ok rtq
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
#endif // UBA_RTQ
		break;
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			bill_in = _main_stay_bill_check();	//2024-02-14
			if (bill_in == BILL_IN_NON)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_NOTE_STAY_WAIT, 10, 0, 0);	/* 100ms*//* 2022-02-28 暫定で減らす */
				_main_set_mode(MODE1_REJECT_STANDBY, REJECT_STANDBY_MODE2_CONFIRM_NOTE_STAY);
			}
			else if (bill_in == BILL_IN_ACCEPTOR)
			{
				_main_reject_sub(MODE1_REJECT_STANDBY, REJECT_STANDBY_MODE2_WAIT_REJECT_REQ, TMSG_CONN_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
			}
			else if (bill_in == BILL_IN_STACKER)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
			}
		}
		break;
	case TMSG_FEED_STATUS_INFO:
		break;
	case TMSG_APB_EXEC_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            if (!(is_detect_rc_twin()) || /* detect twin box */
                !(is_detect_rc_quad()))   /* detect quad box */
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_REJECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_REJECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
            }
        }
        break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 165);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief note reject standby confirm stay procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reject_standby_confirm_note_stay(void) //ok rtq
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_NOTE_STAY_WAIT, 0, 0, 0);
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_NOTE_STAY_WAIT)
		{
			if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
		#if defined(UBA_RTQ)
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_REJECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
		#endif
			else
			{
				bill_in = _main_stay_bill_check();	//2024-02-14
				if (bill_in == BILL_IN_NON)
				{
					_main_set_reject_note_removed_wait_sensor_active();
				}
				else if (bill_in == BILL_IN_ACCEPTOR)
				{
					_main_reject_sub(MODE1_REJECT_STANDBY, REJECT_STANDBY_MODE2_WAIT_REJECT_REQ, TMSG_CONN_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
				}
				else if (bill_in == BILL_IN_STACKER)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
				}
			}
		}
#if defined(UBA_RTQ)
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            if (!(is_detect_rc_twin()) || /* detect twin box */
                !(is_detect_rc_quad()))   /* detect quad box */
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_REJECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
#endif // uBA_RTQ
		break;
	case TMSG_SENSOR_STATUS_INFO:

		bill_in = _main_stay_bill_check(); //2024-02-14

		if (bill_in == BILL_IN_NON)
		{
		}
		else if (bill_in == BILL_IN_ACCEPTOR)
		{
			_main_reject_sub(MODE1_REJECT_STANDBY, REJECT_STANDBY_MODE2_WAIT_REJECT_REQ, TMSG_CONN_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
		}
		else if (bill_in == BILL_IN_STACKER)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
		}
		else if (bill_in == BILL_IN_ENTRANCE)
		{
			#if defined(_PROTOCOL_ENABLE_ID0G8)
			_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_INTERIM, 0, 0, 0);	// イニシャルポーズ用
			#endif

			_main_set_mode(MODE1_REJECT_STANDBY, REJECT_STANDBY_MODE2_NOTE_STAY);
		}
		break;
	case TMSG_FEED_STATUS_INFO:
		break;
	case TMSG_APB_EXEC_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
			ex_rc_error_flag = ex_main_msg.arg2;
        }
        break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 165);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief wait reject request procedure (at reject_standby_ state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reject_standby_wait_reject_req(void) //ok rtq
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		_main_reject_req(0, 0, ex_main_msg.arg1, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif
		break;

#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
			ex_rc_error_flag = ex_main_msg.arg2;
        }
        break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 167);
		}
		break;
	}
}

/* EOF */
