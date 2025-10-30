/******************************************************************************/
/*! @addtogroup Main
    @file       mode_disable.c
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
extern bool _cline_check_opt_func(void);

/************************** EXTERNAL VARIABLES *************************/


/*********************************************************************//**
 * @brief disable message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void disable_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case DISABLE_MODE2_WAIT_REQ:
		disable_wait_req(); //ok rtq
		break;
	case DISABLE_MODE2_WAIT_REJECT_REQ:
		disable_wait_reject_req();
		break;
	default:
		/* system error ? */
		_main_system_error(0, 110);
		break;
	}
}


/*********************************************************************//**
 * @brief wait enable request procedure (at disable state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void disable_wait_req(void)
{
	u16 bill_in;
	
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
		if(ex_rtq_rfid_write_disable == 1)
		{
			ex_main_reset_flag = 1;
		}
		else
		{
			_main_set_init();
		}	
	#else
		_main_set_init();
	#endif
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		//RFIDはenable側でもカバーする
		_main_set_enable();
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_disable();
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
		if(ex_rtq_rfid_write_disable == 1)
		{
			ex_main_payout_flag = 1;
		}
		else
		{
			_main_set_payout_or_collect(1);
		}
	#else
		_main_set_payout_or_collect(1);
	#endif
		break;
	case TMSG_CLINE_COLLECT_REQ:
	case TMSG_DLINE_COLLECT_REQ:
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
		if(ex_rtq_rfid_write_disable == 1)
		{
			ex_main_collect_flag = 1;
		}
		else
		{
			_main_set_payout_or_collect(2);
		}
	#else
		_main_set_payout_or_collect(2);
	#endif
		break;
#endif 
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
		//BOX外れなので、RFID書き込みできないのでエラーにそのまましていい
			_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
			if(ex_rtq_rfid_write_disable == 1)
			{
			//RFID書き込み中はキャンセル、書き込み完了後にセンサメッセージを送信 ok
				break;
			}
		#endif
			if(!(SENSOR_CENTERING_HOME))
			{
				_main_set_active_disable_uba(ACTIVE_DISABLE_MODE2_CENTERING_HOME);
				break;
			}
			else if(!(SENSOR_SHUTTER_OPEN))
			{
				_main_set_active_disable_uba(ACTIVE_DISABLE_MODE2_SHUTTER_OPEN);
				break;
			}
		#if (DATA_COLLECTION_DEBUG!=1)	
			#if defined(UBA_RTQ) //2024-09-30
			else if( (SENSOR_APB_HOME) && ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION) )
			#else	
			if(SENSOR_APB_HOME)
			#endif
			{
				_main_set_active_disable_uba(ACTIVE_DISABLE_MODE2_PB_CLOSE);
				break;
			}
		#endif

			bill_in = _main_bill_in();
			if (bill_in == BILL_IN_ACCEPTOR)
			{
				_main_reject_sub(MODE1_DISABLE, DISABLE_MODE2_WAIT_REJECT_REQ, TMSG_CONN_DISABLE, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_main_msg.arg2);
			}
			else if (bill_in == BILL_IN_STACKER)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
			}
		#if defined(_PROTOCOL_ENABLE_ID003) && !defined(UBA_RTQ)
			else if (bill_in == BILL_IN_ENTRANCE)
			{
				bool result = _cline_check_opt_func();
				if (result)
				{
					_main_set_active_disable_uba(ACTIVE_DISABLE_MODE2_FEED_REJECT); //2024-03-18a
				}
			}
		#endif
			//2024-06-04
			else if(SENSOR_STACKER_HOME && !(is_ld_mode()))
			{
			#if defined(UBA_RTQ)
				#if 1 //2025-03-10
				ex_rtq_rfid_write_disable = 2; //半押しシーケンス開始
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_STACK_HALF, WAIT_TIME_STACK_HALF, 0, 0);
				#else
				_main_send_msg(ID_RC_MBX, TMSG_RC_POLL_CHANGE_REQ, 1, 0, 0, 0);	// change polling time in RC_TASK
				_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_TWIN_TRANSPORT_POS, 0, 0); // SET sensor position of RC -> transport position 
				#endif
			#else
				_main_set_active_disable_uba(ACTIVE_DISABLE_MODE2_STACKER_HALF);
			#endif
			}			
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
			if(ex_rtq_rfid_write_disable == 1)
			{
			//今は処理できないので、再度このメッセージを送ってもらうように、タイマを設定
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
				break;
			}
		#endif
			bill_in = _main_bill_in();
			if ((bill_in == BILL_IN_NON) || (bill_in == BILL_IN_ENTRANCE))
			{
				_main_set_adjustment();
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
			}
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_STACK_HALF) //2025-03-10
		{
		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
			if(ex_rtq_rfid_write_disable == 1)
			{
			//今は処理できないので、再度このメッセージを送ってもらうように、タイマを設定
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_STACK_HALF, WAIT_TIME_STACK_HALF, 0, 0);
				break;
			}
		#endif
			_main_send_msg(ID_RC_MBX, TMSG_RC_POLL_CHANGE_REQ, 1, 0, 0, 0);	// change polling time in RC_TASK
			_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_TWIN_TRANSPORT_POS, 0, 0); // SET sensor position of RC -> transport position 
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
			if(ex_rtq_rfid_write_disable == 1)
			{
				//今は処理できないので、再度このメッセージを送ってもらうように、タイマを設定
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
				break;
			}
		#endif
			if(!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_DISABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
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
		else if(ex_main_msg.arg1 == TIMER_ID_RC_RESET_SKIP)
		{
			if(!(rc_initial_status()))
			{
				if(ex_rc_rewait_rdy_flg == REWAIT_RDY_WAIT)
				{
					switch(ex_rc_rewait_rdy_exec_command)
					{
						case	TMSG_CLINE_PAYOUT_REQ:
						case	TMSG_DLINE_PAYOUT_REQ:
								ex_rc_retry_count = 0;
								_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_SENSOR_ACTIVE);

								ex_multi_job.busy |= TASK_ST_SENSOR;
								_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_ACTIVE_REQ, 0, 0, 0, 0);

								_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_DESPENSE_BEFORE_VEND, 0, 0, 0);
								break;
						case	TMSG_CLINE_COLLECT_REQ:
						case	TMSG_DLINE_COLLECT_REQ:
								_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_SENSOR_ACTIVE);

								ex_multi_job.busy |= TASK_ST_SENSOR;
								_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_ACTIVE_REQ, 0, 0, 0, 0);

								_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_COLLECTION, 0, 0, 0);
								break;
						
					}
				}
				ex_rc_rewait_rdy_flg = REWAIT_RDY_OFF;

				ex_rc_rewait_rdy_exec_command = 0;
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_RESET_SKIP_ERROR, 0, 0, 0);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_RESET_SKIP, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			}
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_RESET_SKIP_ERROR)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_RESET_SKIP, 0, 0, 0);
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
	//#if defined(UBA_RS)
		else if(ex_main_msg.arg1 == TIMER_ID_RS_CONTROL_LED)
		{
			//センサLEDはRFIDの関係なくそのまま送信OK
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
#if defined(UBA_RTQ_ICB)
	case TMSG_TIMER_RFID_RTQ:
		if(ex_rtq_rfid_write_disable == 0) //動作中、半押しシーケンス開始中は無効
		{
			_main_rtq_rfid();
		}
		break;
	//#if defined(NEW_RFID) //2025-07-04
	case TMSG_RC_RFID_WRITE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_rtq_rfid_write_disable = 0;			
			//ここからは、RFID書き込み中に受信したメッセージの処理
			if( ex_main_reset_flag == 1)
			{
				_main_set_init();
				break;
			}
			//
		#if defined(UBA_RTQ)	
			else if(ex_main_payout_flag == 1 || ex_main_collect_flag == 1) //2024-11-13
			{
				_main_set_payout_or_collect(0);
				break;
			}
		#endif
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_rtq_rfid_write_disable = 0;
		//	_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ALARM_CODE_RC_RFID, _main_conv_seq(), ex_position_sensor);
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_RC_RFID, _main_conv_seq(), ex_position_sensor);
		}
		break;
	//#endif
#endif

	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		#if defined(UBA_RTQ_ICB) //#if defined(NEW_RFID) //ok
		if(ex_rtq_rfid_write_disable == 1)
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		}
		#endif

		if (RecycleSettingInfo.key)
		{
			/* RC-Quad model */
			if (is_quad_model())
			{
				// 紙幣あり
				if (!(is_rc_twin_d1_empty()) || !(is_rc_twin_d2_empty()) || !(is_rc_quad_d1_empty()) || !(is_rc_quad_d2_empty()))
				{
					_main_send_connection_task(TMSG_CONN_DISABLE, TMSG_SUB_COLLECT, 0, 0, 0);
					_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
				}
				else
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);
				}
			}
			else
			{
				if (!(is_rc_twin_d1_empty()) || !(is_rc_twin_d2_empty()))
				{
					_main_send_connection_task(TMSG_CONN_DISABLE, TMSG_SUB_COLLECT, 0, 0, 0);
					_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
				}
				else
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_DISABLE, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);
				}
			}
		}
		else
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
		}
		break;
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
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
	case TMSG_RC_SENSOR_RSP: //2024-07-17
		ex_rtq_rfid_write_disable = 0;
		_main_set_active_disable_uba(ACTIVE_DISABLE_MODE2_STACKER_HALF);
		break;

	#if defined(RTQ_ENABLE_RESET) //2024-11-18 待機時のRTQ not ready対策
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
#endif // UBA_RTQ

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

/*********************************************************************//**
 * @brief wait reject request procedure (at disable state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void disable_wait_reject_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		_main_set_enable();
		_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REJECT_REQ);
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
	//主
		_main_reject_req(0, 0, ex_main_msg.arg1, _main_conv_seq(), ex_position_sensor);
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case	TMSG_CLINE_PAYOUT_REQ:
	case	TMSG_DLINE_PAYOUT_REQ:
		//返却開始待ちなので、無視
			break;
	case	TMSG_CLINE_COLLECT_REQ:
	case	TMSG_DLINE_COLLECT_REQ:
		//返却開始待ちなので、無視
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
#endif		
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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

	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
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

#if defined(UBA_RTQ)	
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	//case	TMSG_CLINE_RC_INFO_REQ:
	//		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
	//		break;
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
			_main_system_error(0, 112);
		}
		break;
	}
}

/* EOF */
