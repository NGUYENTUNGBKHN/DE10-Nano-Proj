/******************************************************************************/
/*! @addtogroup Main
    @file       mode_enable.c
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
 * @brief enable message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void enable_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case ENABLE_MODE2_WAIT_BILL_IN:
		enable_wait_bill_in(); //ok rtq
		break;
	case ENABLE_MODE2_WAIT_REQ:
		enable_wait_req();
		break;
	case ENABLE_MODE2_WAIT_REJECT_REQ:
		enable_wait_reject_req();
		break;
	default:
		/* system error ? */
		_main_system_error(0, 120);
		break;
	}
}


/******************************************************************************/
/*! @brief enable wait bill in procedure
    @return         none
    @exception      none
******************************************************************************/
void enable_wait_bill_in(void)
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
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		//RFIDはdisable側でもカバーする
		_main_set_disable();
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		//テストモードはRFID関係ない
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
		//BOX外れなので、RFID書き込みできないのでエラーにそのまましていい
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
	#if defined(UBA_RTQ)
		else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
		}
		else if(ex_rc_error_flag != 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
		}
	#endif
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
				_main_set_active_enable_uba(ACTIVE_ENABLE_MODE2_CENTERING_HOME);
				break;
			}
			else if(!(SENSOR_SHUTTER_OPEN))
			{
				_main_set_active_enable_uba(ACTIVE_ENABLE_MODE2_SHUTTER_OPEN);
				break;
			}
		#if (DATA_COLLECTION_DEBUG!=1)
			#if defined(UBA_RTQ) //2024-09-30
			else if( (SENSOR_APB_HOME) && ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION) )
			#else	
			else if(SENSOR_APB_HOME) //2024-06-08
			#endif
			{
				_main_set_active_enable_uba(ACTIVE_ENABLE_MODE2_PB_CLOSE);
				break;
			}
		
		#endif
			bill_in = _main_bill_in();
			if (bill_in == BILL_IN_ENTRANCE)
			{
				if ((!(ex_collection_data.enable && ex_collection_data.data_exist))
				){
					_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_ACCEPT, 0, 0, 0);
				}
				else
				{
					_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
				}
			}
			else if (bill_in == BILL_IN_ACCEPTOR)
			{
				_main_reject_sub(MODE1_ENABLE, ENABLE_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ENABLE, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_main_msg.arg2);
			}
#if (DEBUG_POLYMER_BELT_ACCEPT==1)
			else if ((bill_in == BILL_IN_STACKER) && ( SENSOR_APB_OUT ))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
			}
#else
			else if (bill_in == BILL_IN_STACKER)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
			}
#endif
			//イニシャル動作完了後の半押しをテスト的にここに入れる 2022-02-16
			else
			{
				if (!(is_ld_mode()))
				{
					if (!(is_box_set()))
					{
						_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else if(SENSOR_STACKER_HOME)
					{
					#if defined(UBA_RTQ)
						#if 1 //2025-03-10
						ex_rtq_rfid_write_disable = 2; //半押しシーケンス開始
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_STACK_HALF, WAIT_TIME_STACK_HALF, 0, 0);
						#else	
						_main_send_msg(ID_RC_MBX, TMSG_RC_POLL_CHANGE_REQ, 1, 0, 0, 0);	// change polling time in RC_TASK
						// ex_multi_job.busy |= TASK_ST_RC;
						_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_TWIN_TRANSPORT_POS, 0, 0); // SET sensor position of RC -> transport position 
						#endif
					#else
						_main_set_active_enable_uba(ACTIVE_ENABLE_MODE2_STACKER_HALF);
					#endif // UBA_RTQ
					}
				}
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
			if (bill_in == BILL_IN_NON)
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
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
			if(ex_rtq_rfid_write_disable == 1)
			{
				//今は処理できないので、再度このメッセージを送ってもらうように、タイマを設定
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
				break;
			}
		#endif
			if (!(is_box_set()))
			{
			//2024-07-16 test	_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			if (ex_main_test_no != TEST_RC_AGING && 
				ex_main_test_no != TEST_RC_AGING_FACTORY)
			{
				if(!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}	
			}
			else
			{
				if(SENSOR_SHUTTER_OPEN)  //  if shutter have not yet opened, not payout
				{
					is_recycle_aging_payout();
				}
			}
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
		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //2025-07-04
		case TMSG_RC_RFID_WRITE_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_rtq_rfid_write_disable = 0;
				
				//ここからは、RFID書き込み中に受信したメッセージの処理
			#if 1
				if( ex_main_reset_flag == 1)
				{
					_main_set_init();
					break;
				}
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
			#endif
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rtq_rfid_write_disable = 0;
			//	_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ALARM_CODE_RC_RFID, _main_conv_seq(), ex_position_sensor);
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_RC_RFID, _main_conv_seq(), ex_position_sensor);
			}
			break;
		#endif
	#endif
	

	case TMSG_APB_EXEC_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SENSOR_RSP:
		ex_rtq_rfid_write_disable = 0;
		_main_set_active_enable_uba(ACTIVE_ENABLE_MODE2_STACKER_HALF);
		break;
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
		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID) //ok
		if(ex_rtq_rfid_write_disable == 1)
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		}
		#endif

		if(ex_operating_mode == OPERATING_MODE_NORMAL)
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
		}
		else
		{
			/* RC-Quad model */
			if(is_quad_model())
			{
				// 紙幣あり
				if(!(is_rc_twin_d1_empty()) || !(is_rc_twin_d2_empty()) || !(is_rc_quad_d1_empty()) || !(is_rc_quad_d2_empty()))
				{
					_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_COLLECT, 0, 0, 0);
					_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
				}
				// 紙幣なし
				else
				{
					_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
					ex_rc_collect_sw = 0;
				}
			}
			else
			{
				// 紙幣あり
				if(!(is_rc_twin_d1_empty()) || !(is_rc_twin_d2_empty()))
				{
					_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_COLLECT, 0, 0, 0);
					_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
				}
				else
				{
					_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
					ex_rc_collect_sw = 0;
				}
			}
		}
		break;

	#if defined(UBA_RTQ)	//2024-11-18 待機時のRTQ not ready対策
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

#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 121);
		}
		break;
	}
}


/******************************************************************************/
/*! @brief wait request (enable state)
    @return         none
    @exception      none
******************************************************************************/
void enable_wait_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_disable();
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_CLINE_ACCEPT_REQ:
	case TMSG_DLINE_ACCEPT_REQ:
		ex_2nd_note_uba = 0; /*1枚目の開始*/
		_main_set_accept();	//通常時
		break;

	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:
		//2024-11-13 廃止で様子見 //2025-01-20 復活させる test modeで使用いている 札ありエージングモードで使用
		if(is_test_mode())
		{
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else if (!(is_detect_rc_twin()) || 
					!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				if ((OperationDenomi.unit == RC_TWIN_DRUM1 && is_rc_twin_d1_empty()) || 
					(OperationDenomi.unit == RC_TWIN_DRUM2 && is_rc_twin_d2_empty()) || 
					(OperationDenomi.unit == RC_QUAD_DRUM1 && is_rc_quad_d1_empty()) || 
					(OperationDenomi.unit == RC_QUAD_DRUM2 && is_rc_quad_d2_empty()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					_main_set_payout_or_collect(1);
				}
			}
		}	
		break;
	//2024-10-07
	case	TMSG_CLINE_COLLECT_REQ:
	case	TMSG_DLINE_COLLECT_REQ:
		//2024-11-13 廃止で様子見 2024-12-11 復活させる test modeで使用いている, test modeのみ enable_wait_bill_in() からジャンプして来る
		if(is_test_mode())
		{
			if(ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if(!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_ENABLE, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				if(is_rc_twin_d1_empty() && is_rc_twin_d2_empty() && is_rc_quad_d1_empty() && is_rc_quad_d2_empty())
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ENABLE, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					_main_set_payout_or_collect(2);
				}
			}
		}
		break;
#endif // UBA_RTQ

	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
#if defined(UBA_RTQ)
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
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
	case TMSG_APB_EXEC_RSP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ) 
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
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
#endif // UBA_RTQ
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
void enable_wait_reject_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		_main_set_disable();
		_main_set_mode(MODE1_DISABLE, DISABLE_MODE2_WAIT_REJECT_REQ);
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			ex_test_finish = 1;
		}
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		_main_reject_req(0, 0, ex_main_msg.arg1, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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


/* EOF */
