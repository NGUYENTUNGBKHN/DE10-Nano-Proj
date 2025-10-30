/******************************************************************************/
/*! @addtogroup Main
    @file       mode_adjust.c
    @brief      adjust mode of main task
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
#include "sub_functions.h"

#if defined(UBA_RTQ)
#include "if_rc.h"
#endif

#define EXT
#include "com_ram.c"
/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/


/*********************************************************************//**
 * @brief adjust message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void adjust_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case ADJUST_MODE2_DISABLE_TEMP_ADJ:
	case ADJUST_MODE2_ENABLE_TEMP_ADJ:
		adjust_temp_adj();
		break;
	default:
		/* system error ? */
		_main_system_error(0, 115);
		break;
	}
}

void adjust_temp_adj_end_sub(void)
{
	if(ex_test_finish)
	{
		_main_set_test_standby();
	}
	else if (ex_main_reset_flag)
	{ /* リセット要求有り */
		_main_set_init();
	}
	else if (ex_multi_job.alarm)
	{ /* other job alarm */

		switch (ex_main_task_mode2)
		{
		case ADJUST_MODE2_DISABLE_TEMP_ADJ:
			_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			break;
		case ADJUST_MODE2_ENABLE_TEMP_ADJ:
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			break;
		default:
			/* system error ? */
			_main_system_error(0, 116);
			break;
		}
	}
#if defined(UBA_RTQ)	
	else if (ex_main_payout_flag)
	{ /* リセット要求有り */
		switch (ex_main_task_mode2)
		{
		case ADJUST_MODE2_DISABLE_TEMP_ADJ:
			/* PAY_OUT request received */
			_main_set_payout_or_collect(1);
			break;
		case ADJUST_MODE2_ENABLE_TEMP_ADJ:
		default:
			ex_main_payout_flag = 0;
			/* system error ? */
			_main_system_error(0, 118);
			break;
		}
	}
	else if (ex_main_collect_flag)
	{ /* リセット要求有り */
		switch (ex_main_task_mode2)
		{
		case ADJUST_MODE2_DISABLE_TEMP_ADJ:
			/* Collect request received */
			_main_set_payout_or_collect(2);
			break;
		case ADJUST_MODE2_ENABLE_TEMP_ADJ:
		default:
			ex_main_collect_flag = 0;
			/* system error ? */
			_main_system_error(0, 118);
			break;
		}
	}	
#endif	
	else
	{
		switch (ex_main_task_mode2)
		{
		case ADJUST_MODE2_DISABLE_TEMP_ADJ:
			_main_set_disable();
			break;
		case ADJUST_MODE2_ENABLE_TEMP_ADJ:
			_main_set_enable();
			break;
		default:
			/* system error ? */
			_main_system_error(0, 116);
			break;
		}
		if((ex_main_msg.arg1 ==TEMP_ADJ_SUCCESS) || (ex_main_msg.arg1 ==TEMP_ADJ_OVER_RUN))
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SUCCESS, 0, 0);
		}
		else
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_FAILURE, 0, 0);
		}
	}
}
/******************************************************************************/
/*! @brief temperature adjustment (at disable/enable state)
    @return         none
    @exception      none
******************************************************************************/
void adjust_temp_adj(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_mode(MODE1_ADJUST, ADJUST_MODE2_DISABLE_TEMP_ADJ);
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		_main_set_mode(MODE1_ADJUST, ADJUST_MODE2_ENABLE_TEMP_ADJ);
		_main_display_enable();
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
		ex_main_payout_flag = 1;
		break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
		ex_main_collect_flag = 1;
		break;
#endif
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_SENSOR_TEMP_ADJ_RSP:
	#ifndef DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE
		if((ex_main_msg.arg1 ==TEMP_ADJ_SUCCESS) || (ex_main_msg.arg1 ==TEMP_ADJ_OVER_RUN))
		{
		#if defined(UBA_RTQ)
			if(ex_main_msg.arg1 ==TEMP_ADJ_SUCCESS)
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_CONDITION_REQ, 0, 0, 0, 0);
			}
		#endif
			write_fram_tmpadj();
		}
		else if((ex_main_msg.arg1 ==TEMP_ADJ_SENSOR_SHIFT) || (ex_main_msg.arg1 ==TEMP_ADJ_ERROR))
		{
			// error or cancel
		}
	#endif
		ex_multi_job.busy &= ~TASK_ST_SENSOR;
		adjust_temp_adj_end_sub();
		break;
		
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				switch (ex_main_task_mode2)
				{
				case ADJUST_MODE2_DISABLE_TEMP_ADJ:
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
					break;
				case ADJUST_MODE2_ENABLE_TEMP_ADJ:
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
					break;
				default:
					/* system error ? */
					_main_system_error(0, 116);
					break;
				}
			}
			else
			{
				switch (ex_main_task_mode2)
				{
				case ADJUST_MODE2_DISABLE_TEMP_ADJ:
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
					break;
				case ADJUST_MODE2_ENABLE_TEMP_ADJ:
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
					break;
				default:
					/* system error ? */
					_main_system_error(0, 116);
					break;
				}
			}
		}
		break;

	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		if(ex_main_task_mode2 == ADJUST_MODE2_DISABLE_TEMP_ADJ) //Disableの時のみ
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		}
		break;	
#endif

	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			//#if defined(UBA_RS)
			if (!(is_rs_remain_note()))
			{
				if(ex_rs_payout_disp_on == 0)
				{
					_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_GREEN, 0, 0);
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RS_CONTROL_LED, 0, 0, 0);				
					ex_rs_payout_disp_on = 1;
				}
			}
			//#endif 
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	//#if defined(UBA_RS)
		else if (ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
		{
			if(is_rs_remain_note())
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_GREEN, 0, 0);
				ex_rs_payout_remain_flag = RS_NOTE_REMAIN_CONFIRM;
				// jdl_remain_note();
			}
		}
	//#endif
#endif // UBA_RTQ
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			if (!(ex_multi_job.alarm))
			{ /* set alarm inform */
				ex_multi_job.alarm |= TASK_ST_LINE;
				ex_multi_job.code[MULTI_LINE] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_LINE] = _main_conv_seq();
				ex_multi_job.sensor[MULTI_LINE] = ex_position_sensor;
			}
		}
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			switch (ex_main_task_mode2)
			{
			case ADJUST_MODE2_DISABLE_TEMP_ADJ:
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				break;
				case ADJUST_MODE2_ENABLE_TEMP_ADJ:
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				break;
			default:
				/* system error ? */
				_main_system_error(0, 116);
				break;
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 117);
		}
		break;
	}
}


/* EOF */
