/******************************************************************************/
/*! @addtogroup Main
    @file       active_mode_enable.c
    @brief      enable mode of main task
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



#if defined(_PROTOCOL_ENABLE_ID003)
	#include "task/cline_task/003/id003.h"
#endif


#define EXT
#include "com_ram.c"


/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/


/*********************************************************************//**
 * @brief active enable message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_enable_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case ACTIVE_ENABLE_MODE2_WAIT_REQ: //待機状態で、スタッカやPBなどの動作完了後に入口センサで紙幣を検知したので、紙幣取り込み開始したい状態
		active_enable_wait_req();
		break;
	case ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ:
		active_enable_wait_reject_req();
		break;
	case ACTIVE_ENABLE_MODE2_STACKER_HALF:
		active_enable_stacker_half();
		break;

	case ACTIVE_ENABLE_MODE2_CENTERING_HOME:
		active_enable_centering_home();
		break;
	case ACTIVE_ENABLE_MODE2_SHUTTER_OPEN:
		active_enable_shutter_open();
		break;
	case ACTIVE_ENABLE_MODE2_PB_CLOSE:
		active_enable_pb_close();
		break;

	default:
		/* system error ? */
		_main_system_error(0, 120);
		break;
	}
}


/******************************************************************************/
/*! @brief wait request (enable state)
    @return         none
    @exception      none
******************************************************************************/
void active_enable_wait_req(void) //待機状態で、スタッカやPBなどの動作完了後に入口センサで紙幣を検知したので、紙幣取り込み開始したい状態
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		//ラインからの取り込み開始待ちで、Disableが送られてきたので、強制的にActiveではないDisableにする
		_main_set_disable(); //2024-11-11
		break;
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_ACCEPT_REQ:
	case TMSG_DLINE_ACCEPT_REQ:
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
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
		}
		else if (ex_multi_job.reject)
		{ /* othrer job reject */
			_main_reject_sub(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ENABLE, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
		}
		else
		{
			#if MAG1_ENABLE
			if(ex_uba710 == 1)
			{
				_hal_i2c3_write_mag_cntl(1);	//2022-06-29
			}
			#endif

			ex_2nd_note_uba = 0; /*1枚目の開始*/
			_main_set_accept();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
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
	case TMSG_APB_EXEC_RSP:
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
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			//#if defined(UBA_RS)
			else if (!(is_rs_remain_note()))
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

	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
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
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 122);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief wait reject request procedure (at enable state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_enable_wait_reject_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_WAIT_REJECT_REQ);
		break;
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
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
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
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
	case TMSG_APB_EXEC_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			//#if defined(UBA_RS)
			else if (!(is_rs_remain_note()))
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
	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
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
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 123);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief stacker half operation procedure (at enable state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void active_enable_stacker_half(void)
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_STACKER_HALF);
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
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		ex_main_reject_flag = ex_main_msg.arg1;
		break;
	case TMSG_STACKER_HALF_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_STACKER;

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
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else if (ex_main_reject_flag)
			{ /* リジェクト要求有り */
				_main_reject_req(0, 0, ex_main_reject_flag, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
			

				bill_in = _main_bill_in();
				if (bill_in == BILL_IN_NON)
				{
					_main_set_enable();
				}
				else if (bill_in == BILL_IN_ENTRANCE)
				{
					_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_ACCEPT, 0, 0, 0);
				}
				else if (bill_in == BILL_IN_ACCEPTOR)
				{
					_main_reject_sub(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ENABLE, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
				}
				else if (bill_in == BILL_IN_STACKER)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_STACKER;
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
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 132);
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
	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
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
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 123);
		}
		break;
	}
}

void active_enable_centering_home(void)
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_CENTERING_HOME);
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
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
			else
			{
				bill_in = _main_bill_in();
				if (bill_in == BILL_IN_NON)
				{
					_main_set_enable();
				}
				else if (bill_in == BILL_IN_ENTRANCE)
				{
					_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_ACCEPT, 0, 0, 0);
				}
				else if (bill_in == BILL_IN_ACCEPTOR)
				{
					_main_reject_sub(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ENABLE, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
				}
				else if (bill_in == BILL_IN_STACKER)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_CENTERING);
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			//#if defined(UBA_RS)
			else if (!(is_rs_remain_note()))
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

	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
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

	case TMSG_APB_EXEC_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 123);
		}
		break;
	}
}

void active_enable_shutter_open(void)
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_SHUTTER_OPEN);
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
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
			else
			{
				bill_in = _main_bill_in();
				if (bill_in == BILL_IN_NON)
				{
					_main_set_enable();
				}
				else if (bill_in == BILL_IN_ENTRANCE)
				{
					_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_ACCEPT, 0, 0, 0);
				}
				else if (bill_in == BILL_IN_ACCEPTOR)
				{
					_main_reject_sub(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ENABLE, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
				}
				else if (bill_in == BILL_IN_STACKER)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			//#if defined(UBA_RS)
			else if (!(is_rs_remain_note()))
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
#endif // uBA_RTQ
		break;
	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
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

	case	TMSG_RC_DEL_RSP://step2
			switch(ex_main_msg.arg1)
			{
			case	TMSG_RC_ENC_KEY:
					/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
					break;
			case	TMSG_RC_ENC_NUM:
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					break;
			}
			break;
	case	TMSG_RC_MODE_RSP:
			/* send message to rc_task (TMSG_RC_RESET_SKIP_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_SKIP_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_RESET_SKIP_RSP:
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
	#endif		
	#endif

	case TMSG_APB_EXEC_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 123);
		}
		break;
	}
}


void active_enable_pb_close(void) //2024-06-08
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_PB_CLOSE);
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		ex_main_reject_flag = 1;
		break;
	case TMSG_APB_CLOSE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_APB;

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
			else
			{
				bill_in = _main_bill_in();
				if (bill_in == BILL_IN_NON)
				{
					_main_set_enable();
				}
				else if (bill_in == BILL_IN_ENTRANCE)
				{
					_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_ACCEPT, 0, 0, 0);
				}
				else if (bill_in == BILL_IN_ACCEPTOR)
				{
					_main_reject_sub(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ENABLE, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
				}
				else if (bill_in == BILL_IN_STACKER)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			//#if defined(UBA_RS)
			else if (!(is_rs_remain_note()))
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

	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
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

	case TMSG_APB_EXEC_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
