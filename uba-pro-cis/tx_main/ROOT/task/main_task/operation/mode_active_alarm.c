/******************************************************************************/
/*! @addtogroup Main
    @file       mode_active_alarm.c
    @brief      disable mode of main task
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

#define EXT
#include "com_ram.c"

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/
void active_alarm_icb_wait_error_rsp(void);
void active_alarm_tmp_wait_read(void);

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/


/*********************************************************************//**
 * @brief active disable message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_alarm_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case ACTIVE_ALARM_MODE2_ICB_ERROR_RSP:
		active_alarm_icb_wait_error_rsp();
		break;
	case ACTIVE_ALARM_MODE2_TMP_READ: //2024-05-28
		active_alarm_tmp_wait_read();
		break;
	
	default:
		/* system error ? */
		_main_system_error(0, 110);
		break;
	}
}


/*********************************************************************//**
 * @brief wait icb request procedure (at alarm state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_alarm_icb_wait_error_rsp(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_SET_STATUS:
		break;
	case TMSG_ICB_ERROR_CODE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			
			ex_multi_job.busy &= ~(TASK_ST_ICB);
			_main_alarm_sub(
					ex_icb_alarm_backup.mode1,
					ex_icb_alarm_backup.mode2,
					ex_icb_alarm_backup.rsp_msg,
					ex_icb_alarm_backup.code,
					ex_icb_alarm_backup.seq,
					ex_icb_alarm_backup.sensor);

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
		
			ex_multi_job.busy &= ~(TASK_ST_ICB);
			_main_alarm_sub(
					ex_icb_alarm_backup.mode1,
					ex_icb_alarm_backup.mode2,
					ex_icb_alarm_backup.rsp_msg,
					ex_icb_alarm_backup.code,
					ex_icb_alarm_backup.seq,
					ex_icb_alarm_backup.sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 181);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 111);
		}
		break;
	}
}

void active_alarm_tmp_wait_read(void) //2024-05-28
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_SET_STATUS:
		break;
	case TMSG_MGU_READ_RSP:

		_pl_cis_enable_set(0);
		ex_multi_job.busy &= ~(TASK_ST_MGU);

		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(is_temperature_alarm_clear())
			{
				if (is_test_mode())
				{
					switch (ex_main_test_no)
					{
					case TEST_AGING:
					//case TEST_AGING_LD:
						_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
						break;

					case TEST_ACCEPT:
					case TEST_ACCEPT_LD:
					case TEST_ACCEPT_ALLACC:
					case TEST_ACCEPT_LD_ALLACC:
					case TEST_REJECT:
					case TEST_REJECT_LD:
						_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
						break;

					default:
						_main_set_test_standby();
						break;
					}
				}
				else
				{
					//2024-12-08 CISの温度は下がった Reset受信時はResetした方がいい
					if( ex_main_reset_flag == 1)
					{
						_main_set_init();
					}
					else
					{
						/* Set FPGA mode : disable */
						_main_set_pl_active(PL_DISABLE);
						_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
					}
				}
			}
			else
			{
				//2024-12-08 CISの温度が高いので、基本エラー継続だが、Reset受信時はResetした方がいい
				if( ex_main_reset_flag == 1)
				{
					_main_set_init();
				}
				else
				{
					/* Set FPGA mode : standby */
					_main_set_pl_active(PL_DISABLE);
					/* Set sensor mode : standby */
					_main_set_sensor_active(0);
					_main_set_mode(MODE1_ALARM, ALARM_MODE2_CIS_TEMPERATURE);
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_CONFIRM_TEMP, WAIT_TIME_DATA_WAIT, 0, 0);
				}
			}
		}
		else
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_TMP_I2C, _main_conv_seq(), ex_position_sensor);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 111);
		}
		break;
	}
}



/* EOF */
