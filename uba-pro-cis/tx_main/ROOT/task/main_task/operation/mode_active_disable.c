/******************************************************************************/
/*! @addtogroup Main
    @file       mode_active_disable.c
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
#include "user/icb/icb.h"

#if defined(UBA_RTQ)
#include "if_rc.h"
#endif // UBA_RTQ

#define EXT
#include "com_ram.c"

#if defined(_PROTOCOL_ENABLE_ID003)
static bool ex_recive_enable_cmd_003=0;
#endif

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/


/*********************************************************************//**
 * @brief active disable message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_disable_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case ACTIVE_DISABLE_MODE2_WAIT_REJECT_REQ:
		active_disable_wait_reject_req();
		break;
	case ACTIVE_DISABLE_MODE2_STACKER_HALF:
		active_disable_stacker_half();
		break;

	case ACTIVE_DISABLE_MODE2_CENTERING_HOME:
		active_disable_centering_home();
		break;
	case ACTIVE_DISABLE_MODE2_SHUTTER_OPEN:
		active_disable_shutter_open();
		break;
	case ACTIVE_DISABLE_MODE2_FEED_REJECT:
		active_disable_feed_reject(); //ID-003のoptional function 入口検知での搬送逆転
		break;

	case ACTIVE_DISABLE_MODE2_PB_CLOSE:
		active_disable_pb_close();
		break;

	default:
		/* system error ? */
		_main_system_error(0, 110);
		break;
	}
}


/*********************************************************************//**
 * @brief wait reject request procedure (at disable state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_disable_wait_reject_req(void)
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
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ);
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		if(!(ex_multi_job.busy))
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
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else
			{
				_main_reject_req(0, 0, ex_main_msg.arg1, _main_conv_seq(), ex_position_sensor);
			}
		}
		else
		{
			if(!ex_multi_job.reject)
			{
				ex_multi_job.reject = TASK_ST_MAIN;
				ex_multi_job.code[MULTI_MAIN] = ex_main_msg.arg1;
				ex_multi_job.sequence[MULTI_MAIN] = _main_conv_seq();
				ex_multi_job.sensor[MULTI_MAIN] = ex_position_sensor;
			}
		}
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

	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			if (!(ex_multi_job.alarm))
			{ /* set alarm inform */
				ex_multi_job.alarm |= TASK_ST_SENSOR;
				ex_multi_job.code[MULTI_SENSOR] = ALARM_CODE_BOX;
				ex_multi_job.sequence[MULTI_SENSOR] = _main_conv_seq();
				ex_multi_job.sensor[MULTI_SENSOR] = ex_position_sensor;
			}
		}
		break;

#if defined(UBA_RTQ)
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
#endif

	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
	#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
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
		else if(ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
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
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
	/* 返却開始前なので無視*/
			break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
	/* 返却開始前なので無視*/
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
	#if defined(RTQ_ENABLE_RESET)
	case	TMSG_RC_REWAIT_READY_RSP: //step1 通信が復活
			if ( ex_main_msg.arg1 == TMSG_SUB_SUCCESS )
			{
			#if defined(RC_ENCRYPTION)
				// 暗号化初期化
				initialize_encryption();
				renewal_cbc_context();
				
				/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
			#else
				/* send message to rc_task (TMSG_RC_MODE_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
			#endif
			}
			else
			{
				if(ex_rc_error_flag == 0)
				{
					ex_rc_error_flag = ex_main_msg.arg2;
				}
			}
			break;
	//2024-11-18 待機時のRTQ not ready対策
	case	TMSG_RC_DEL_RSP://step2
			switch(ex_main_msg.arg1)
			{
			case	TMSG_RC_ENC_KEY://step2-1
					/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
					break;
			case	TMSG_RC_ENC_NUM://step2-2
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					break;
			}
			break;
	case	TMSG_RC_MODE_RSP://step3
			/* send message to rc_task (TMSG_RC_RESET_SKIP_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_SKIP_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_RESET_SKIP_RSP://step4
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
	#endif		
#endif
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 112);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief stacker half operation procedure (at disable state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_disable_stacker_half(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
	#if defined(UBA_RTQ)
		if(ex_main_payout_flag == 0 && ex_main_collect_flag == 0)
		{
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_STACKER_HALF);
		}
	#else
		_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_STACKER_HALF);
	#endif	
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		ex_main_reject_flag = ex_main_msg.arg1;
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
	case TMSG_STACKER_HALF_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (ex_main_reject_flag)
			{ /* リジェクト要求有り */
				_main_reject_req(0, 0, ex_main_reject_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
		#if defined(UBA_RTQ)	
			else if(ex_main_payout_flag == 1 || ex_main_collect_flag == 1) //2024-11-13
			{
				_main_set_payout_or_collect(0);
			}
		#endif
			else
			{
				_main_set_disable();
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			if (!(ex_multi_job.alarm))
			{ /* set alarm inform */
				ex_multi_job.alarm |= TASK_ST_STACKER;
				ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;
			}

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 132);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;

	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
			ex_main_payout_flag = 1;
			break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
			ex_main_collect_flag = 1;
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
	#if defined(RTQ_ENABLE_RESET)
	case 	TMSG_RC_REWAIT_READY_RSP:	//step1 通信が復活
			if ( ex_main_msg.arg1 == TMSG_SUB_SUCCESS )
			{
			#if defined(RC_ENCRYPTION)
				// 暗号化初期化
				initialize_encryption();
				renewal_cbc_context();
				
				/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
			#else
				/* send message to rc_task (TMSG_RC_MODE_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
			#endif
			}
			else
			{
				if(ex_rc_error_flag == 0)
				{
					ex_rc_error_flag = ex_main_msg.arg2;
				}
			}
			break;

	//2024-11-18 待機時のRTQ not ready対策
	case	TMSG_RC_DEL_RSP:	//step2
			switch(ex_main_msg.arg1)
			{
			case	TMSG_RC_ENC_KEY://step2-1
					/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
					break;
			case	TMSG_RC_ENC_NUM://step2-2
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					break;
			}
			break;
	case	TMSG_RC_MODE_RSP://step3
			/* send message to rc_task (TMSG_RC_RESET_SKIP_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_SKIP_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_RESET_SKIP_RSP://step4
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
	#endif
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
#endif

	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			if (!(ex_multi_job.alarm))
			{ /* set alarm inform */
				ex_multi_job.alarm |= TASK_ST_SENSOR;
				ex_multi_job.code[MULTI_SENSOR] = ALARM_CODE_BOX;
				ex_multi_job.sequence[MULTI_SENSOR] = _main_conv_seq();
				ex_multi_job.sensor[MULTI_SENSOR] = ex_position_sensor;
			}
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
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
		else if(ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
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
	case TMSG_APB_EXEC_RSP:
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
			_main_system_error(0, 123);
		}
		break;
	}
}

void active_disable_centering_home(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
	#if defined(UBA_RTQ)
		if(ex_main_payout_flag == 0 && ex_main_collect_flag == 0)
		{
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_CENTERING_HOME);
		}
	#else
		_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_CENTERING_HOME);
	#endif
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		ex_main_reject_flag = 1;
		break;
	case TMSG_CENTERING_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_CENTERING);

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (ex_main_reject_flag)
			{ /* リジェクト要求有り */
				_main_set_reject();
			}
		#if defined(UBA_RTQ)	
			else if(ex_main_payout_flag == 1 || ex_main_collect_flag == 1) //2024-11-13
			{
				_main_set_payout_or_collect(0);
			}
		#endif
			else
			{
				_main_set_disable();
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_CENTERING);

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 132);
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;

	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
			ex_main_payout_flag = 1;
			break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
			ex_main_collect_flag = 1;
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
	#if defined(RTQ_ENABLE_RESET)
	case	TMSG_RC_REWAIT_READY_RSP://step1 通信が復活
			if ( ex_main_msg.arg1 == TMSG_SUB_SUCCESS )
			{
			#if defined(RC_ENCRYPTION)
				// 暗号化初期化
				initialize_encryption();
				renewal_cbc_context();
				
				/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
			#else
				/* send message to rc_task (TMSG_RC_MODE_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
			#endif
			}
			else
			{
				if(ex_rc_error_flag == 0)
				{
					ex_rc_error_flag = ex_main_msg.arg2;
				}
			}
			break;
	//2024-11-18 待機時のRTQ not ready対策
	case	TMSG_RC_DEL_RSP://step2
			switch(ex_main_msg.arg1)
			{
			case	TMSG_RC_ENC_KEY://step2-1
					/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
					break;
			case	TMSG_RC_ENC_NUM://step2-2
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					break;
			}
			break;
	case	TMSG_RC_MODE_RSP://step3
			/* send message to rc_task (TMSG_RC_RESET_SKIP_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_SKIP_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_RESET_SKIP_RSP://step4
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
	#endif		
#endif
	#if defined(UBA_RTQ)	
		case	TMSG_LINE_CURRENT_COUNT_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
				break;
		case	TMSG_CLINE_RC_INFO_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
				break;
		case	TMSG_LINE_RC_ENABLE_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
				break;
	#endif

	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
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
		else if(ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
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
	case TMSG_APB_EXEC_RSP:
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
			_main_system_error(0, 123);
		}
		break;
	}
}

void active_disable_shutter_open(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
	#if defined(UBA_RTQ)
		if(ex_main_payout_flag == 0 && ex_main_collect_flag == 0)
		{
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_SHUTTER_OPEN);
		}
	#else		
		_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_SHUTTER_OPEN);
	#endif
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		ex_main_reject_flag = 1;
		break;
	case TMSG_SHUTTER_OPEN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (ex_main_reject_flag)
			{ /* リジェクト要求有り */
				_main_set_reject();
			}
		#if defined(UBA_RTQ)	
			else if(ex_main_payout_flag == 1 || ex_main_collect_flag == 1) //2024-11-13
			{
				_main_set_payout_or_collect(0);
			}
		#endif
			else
			{
				_main_set_disable();
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 132);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
			ex_main_payout_flag = 1;
			break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
			ex_main_collect_flag = 1;
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
	#if defined(RTQ_ENABLE_RESET)	 
	case	TMSG_RC_REWAIT_READY_RSP://step1 通信が復活
			if ( ex_main_msg.arg1 == TMSG_SUB_SUCCESS )
			{
			#if defined(RC_ENCRYPTION)
				// 暗号化初期化
				initialize_encryption();
				renewal_cbc_context();
				
				/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
			#else
				/* send message to rc_task (TMSG_RC_MODE_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
			#endif
			}
			else
			{
				if(ex_rc_error_flag == 0)
				{
					ex_rc_error_flag = ex_main_msg.arg2;
				}
			}
			break;

	//2024-11-18 待機時のRTQ not ready対策
	case	TMSG_RC_DEL_RSP://step2
			switch(ex_main_msg.arg1)
			{
			case	TMSG_RC_ENC_KEY://step2-1
					/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
					break;
			case	TMSG_RC_ENC_NUM://step2-2
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					break;
			}
			break;
	case	TMSG_RC_MODE_RSP://step3
			/* send message to rc_task (TMSG_RC_RESET_SKIP_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_SKIP_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_RESET_SKIP_RSP://step4
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
	#endif		
#endif

	#if defined(UBA_RTQ)
		case	TMSG_LINE_CURRENT_COUNT_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
				break;
		case	TMSG_CLINE_RC_INFO_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
				break;
		case	TMSG_LINE_RC_ENABLE_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
				break;
	#endif

	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
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
		else if(ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
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
	case TMSG_APB_EXEC_RSP:
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
			_main_system_error(0, 123);
		}
		break;
	}
}



void active_disable_feed_reject(void) //2024-03-18a //ID-003のoptional function 入口検知での搬送逆転
{

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;

	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
	#if defined(UBA_RTQ)
		if(ex_main_payout_flag == 0 && ex_main_collect_flag == 0)
		{
			if(ex_recive_enable_cmd_003 == 0)
			{
				/* Enable/DisableはLineタスクから定期的に送られて来るので、1回のみになるようにする */
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_FORCE_REV_REQ, 1, 0, 0, 0);	/* 停止命令 */
			}
		}
		ex_recive_enable_cmd_003 = 1;
	#else		
		if(ex_recive_enable_cmd_003 == 0)
		{
			/* Enable/DisableはLineタスクから定期的に送られて来るので、1回のみになるようにする */
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_FORCE_REV_REQ, 1, 0, 0, 0);	/* 停止命令 */
		}
		ex_recive_enable_cmd_003 = 1;
	#endif
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		ex_recive_enable_cmd_003 = 0;
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;

	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
		 ex_main_payout_flag = 1;
		 break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
		 ex_main_collect_flag = 1;
		 break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
	#if defined(RTQ_ENABLE_RESET)	 
	case	TMSG_RC_REWAIT_READY_RSP://step1 通信が復活
			if ( ex_main_msg.arg1 == TMSG_SUB_SUCCESS )
			{
			#if defined(RC_ENCRYPTION)
				// 暗号化初期化
				initialize_encryption();
				renewal_cbc_context();
				
				/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
			#else
				/* send message to rc_task (TMSG_RC_MODE_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
			#endif
			}
			else
			{
				if(ex_rc_error_flag == 0)
				{
					ex_rc_error_flag = ex_main_msg.arg2;
				}
			}
			break;

	//2024-11-18 待機時のRTQ not ready対策
	case	TMSG_RC_DEL_RSP://step2
			switch(ex_main_msg.arg1)
			{
			case	TMSG_RC_ENC_KEY://step2-1
					/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
					break;
			case	TMSG_RC_ENC_NUM://step2-2
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					break;
			}
			break;
	case	TMSG_RC_MODE_RSP://step3
			/* send message to rc_task (TMSG_RC_RESET_SKIP_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_SKIP_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_RESET_SKIP_RSP://step4
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
	#endif		
#endif
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
		else if(ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
		{
			if(is_rs_remain_note())
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_GREEN, 0, 0);
				ex_rs_payout_remain_flag = RS_NOTE_REMAIN_CONFIRM;
				// jdl_remain_note();
			}
		}
		//#endif
	#endif		
		break;

	case TMSG_FEED_FORCE_REV_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
		#if defined(UBA_RTQ)	
			else if(ex_main_payout_flag == 1 || ex_main_collect_flag == 1) //2024-11-13
			{
				_main_set_payout_or_collect(0);
			}
		#endif
			else if( ex_recive_enable_cmd_003 == 1 )
			{
			/* Enable */
				ex_recive_enable_cmd_003 = 0;
				_main_set_enable();
			}
			else
			{
			/* Disable */
				_main_set_disable();
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_recive_enable_cmd_003 = 0;

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 82);
		}
		break;


	#if defined(UBA_RTQ)
		case	TMSG_LINE_CURRENT_COUNT_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
				break;
		case	TMSG_CLINE_RC_INFO_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
				break;
		case	TMSG_LINE_RC_ENABLE_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
				break;
	#endif

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


void active_disable_pb_close(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
	#if defined(UBA_RTQ)
		if(ex_main_payout_flag == 0 && ex_main_collect_flag == 0)
		{
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_PB_CLOSE);
		}
	#else
		_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_PB_CLOSE);
	#endif
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		ex_main_reject_flag = 1;
		break;
	case TMSG_APB_CLOSE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (ex_main_reject_flag)
			{ /* リジェクト要求有り */
				_main_set_reject();
			}
		#if defined(UBA_RTQ)	
			else if(ex_main_payout_flag == 1 || ex_main_collect_flag == 1) //2024-11-13
			{
				_main_set_payout_or_collect(0);
			}
		#endif
			else
			{
				_main_set_disable();
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 132);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;

	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
			ex_main_payout_flag = 1;
			break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
			ex_main_collect_flag = 1;
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
	#if defined(RTQ_ENABLE_RESET)
	case	TMSG_RC_REWAIT_READY_RSP://step1 通信が復活
			if ( ex_main_msg.arg1 == TMSG_SUB_SUCCESS )
			{
			#if defined(RC_ENCRYPTION)
				// 暗号化初期化
				initialize_encryption();
				renewal_cbc_context();
				
				/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
			#else
				/* send message to rc_task (TMSG_RC_MODE_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
			#endif
			}
			else
			{
				if(ex_rc_error_flag == 0)
				{
					ex_rc_error_flag = ex_main_msg.arg2;
				}
			}
			break;
	//2024-11-18 待機時のRTQ not ready対策
	case	TMSG_RC_DEL_RSP://step2
			switch(ex_main_msg.arg1)
			{
			case	TMSG_RC_ENC_KEY://step2-1
					/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
					break;
			case	TMSG_RC_ENC_NUM://step2-2
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					break;
			}
			break;
	case	TMSG_RC_MODE_RSP://step3
			/* send message to rc_task (TMSG_RC_RESET_SKIP_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_SKIP_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_RESET_SKIP_RSP://step4
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
	#endif		
#endif

	#if defined(UBA_RTQ)
		case	TMSG_LINE_CURRENT_COUNT_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
				break;
		case	TMSG_CLINE_RC_INFO_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
				break;
		case	TMSG_LINE_RC_ENABLE_REQ:
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
				break;
	#endif

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
		else if(ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
		{
			if(is_rs_remain_note())
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_GREEN, 0, 0);
				ex_rs_payout_remain_flag = RS_NOTE_REMAIN_CONFIRM;
				// jdl_remain_note();
			}
		}
		//#endif
	#endif
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
			_main_system_error(0, 123);
		}
		break;
	}
}

/* EOF */
