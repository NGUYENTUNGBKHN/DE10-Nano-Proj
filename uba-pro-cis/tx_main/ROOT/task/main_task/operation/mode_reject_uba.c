/******************************************************************************/
/*! @addtogroup Main
    @file       mode_reject.c
    @brief      reject mode of main task
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

#if defined(_PROTOCOL_ENABLE_ID003)
	#include "task/cline_task/003/id003.h"
#endif
#if defined(UBA_RTQ)
#include "if_rc.h"
#endif // UBA_RTQ

#define EXT
#include "com_ram.c"

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/

/*********************************************************************//**
 * @brief reject message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reject_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case REJECT_MODE2_SENSOR_ACTIVE:	// 0x0701 ポジションセンサがアクティブでない状態からの、返却の為、
		reject_sensor_active();	//ok rtq		// mode_enable, mode_disableからのみ		  ポジションアクティブ後に紙幣返却する ok
		break;

	case REJECT_MODE2_FEED_REJECT:		// 0x0703 返却完了待ち(返却動作中)
		feed_reject();	//ok rtq				// 0x0703 返却成功で、ICBなしでStacker homeでない場合 note stayへ
		break;

	/* 返却成功 */
	case REJECT_MODE2_STACKER_HALF_PB_CLOSE:		// 0x0705 mode_rejectからのみ use 返却成功後の半押し
		stacker_half_pb_close(); //ok rtq					// 0x0705 成功で、ICBなしの場合、 note stayへ
		break;

//	case REJECT_MODE2_NOTE_STAY:		// 0x0706 紙幣返却完了状態
//		note_stay();					// 0x0706 紙幣返却完了状態
//		break;

	/* 返却紙幣取り除き後 */
	case REJECT_MODE2_NOTE_REMOVED_WAIT_SENSOR_ACTIVE:	/* standbyから紙幣が取り除かれて、幅よせ動作を行いたい時に呼び出される */
		reject_note_removed_wait_sensor_active(); //ok rtq
		break;


	case REJECT_MODE2_WAIT_WID:			// 返却紙幣が取り除かれた後、幅よせ動作中(幅よせエラーでの返却など)
		reject_wait_wid();	//ok rtq			// 幅よせ1回動作
		break;

	case REJECT_MODE2_WAIT_REQ:			// 0x0707 LINEからのメッセージ待ち
		reject_wait_req();	//ok rtq			// 0x0707 LINEからのメッセージ待ち ok
		break;

	default:
		/* system error ? */
		_main_system_error(0, 150);
		break;
	}
}


/*********************************************************************//**
 * @brief sensor active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reject_sensor_active(void) //ok rtq
{
	switch (ex_main_msg.tmsg_code)
	{
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
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0); //1回目のReject動作
		}
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)	/* '19-03-18 */
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_STATUS_INFO:
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
#endif
#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 151);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief feed reject procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void feed_reject(void) //ok rtq
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
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)	/* '19-03-18 */
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		#if 0 //2025-03-10 //2025-05-12 廃止
			////2025-05-12  紙幣受け取り時の背面搬送で失敗した場合の返却の時、紙幣ハンギングしない
			//その時に、Reject7Bを通知するが、その後のIDLINGステータスになるまで、3s程度時間がかかる
			//理由は下記の処理で半押し動作などを完了後に返却完了としてラインタスクに通知している為、
			//返却直後に半押し処理をすると、場合によっては、TMSG_RC_SENSOR_REQ を連続でRTQ側に通知する可能性がある。
			//その場合、RTQ側が白点灯になる可能性がある。
			//この問題は待機時の半押し動作で発生した問題で、念の為にこちらも変更していた。
			//TMSG_SENSOR_STATUS_INFOのイベントのたびに、TMSG_RC_SENSOR_REQ を送信するのは危険

			//その為、半押し開始を3s程度経過後に行う様にしていた。
			//ただ、弊害としてIDLING通知が遅くなっていた。
		else if(ex_main_msg.arg1 == TIMER_ID_STACK_HALF)
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_POLL_CHANGE_REQ, 1, 0, 0, 0);	// change polling time in RC_TASK
			_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_TWIN_TRANSPORT_POS, 0, 0); // SET sensor position of RC -> transport position 
		}
		#endif
	#endif
		break;
	case TMSG_FEED_REJECT_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_reject_retry_uba = 0;
			ex_multi_job.busy &= ~(TASK_ST_FEED);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_multi_job_alarm_backup.alarm)
			{ /* other job alarm */
				memcpy(&ex_multi_job, &ex_multi_job_alarm_backup, sizeof(MULTI_JOB));
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
	#if defined(UBA_RTQ)
			else if (!(is_ld_mode()) && (SENSOR_STACKER_HOME)) 
			{
				#if 0 //2025-03-10 //2025-05-12 廃止
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_STACK_HALF, WAIT_TIME_STACK_HALF, 0, 0);
				#else
				_main_send_msg(ID_RC_MBX, TMSG_RC_POLL_CHANGE_REQ, 1, 0, 0, 0);	// change polling time in RC_TASK
				_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_TWIN_TRANSPORT_POS, 0, 0); // SET sensor position of RC -> transport position 
				#endif
			}
			else if( (SENSOR_APB_HOME) && ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION) )  //2024-09-30
			{
				ex_multi_job.busy |= TASK_ST_APB;
				_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
				_main_set_mode(MODE1_REJECT, REJECT_MODE2_STACKER_HALF_PB_CLOSE);
			}
	#else
			else if ( (!(is_ld_mode()) && (SENSOR_STACKER_HOME)) 
		#if (DATA_COLLECTION_DEBUG!=1)
			||
			(SENSOR_APB_HOME)
		#endif
			 ) //2024-06-08
			{
				if(!(is_ld_mode()) && (SENSOR_STACKER_HOME))
				{
					ex_multi_job.busy |= TASK_ST_STACKER;
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HALF_REQ, 0, 0, 0, 0);
				}
				else
				{
					ex_multi_job.busy |= TASK_ST_APB;
					_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
				}
				_main_set_mode(MODE1_REJECT, REJECT_MODE2_STACKER_HALF_PB_CLOSE);
			}
	#endif
			else
			{
				_main_set_reject_standby_note_stay();
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
				#if defined(_PROTOCOL_ENABLE_ID0G8)
				_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_REMAIN, 0, 0, 0);
				#endif /*  */
			}
		}

		else if (ex_main_msg.arg1 == TMSG_SUB_FEED_REJECT_RETRY)	// 今回新たに対応 2018-12-25
		{
		/* 紙幣は逆転、正転後 */
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_reject_retry_uba++;
			ex_multi_job.busy |= TASK_ST_CENTERING;

			if(ex_wid_reject_uba == 1)
			{
				_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_OUT_RETRY_REQ, 0, 0, 0, 0);
			}
			else
			{
				_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 1, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{
		// 返却動作中に幅よせがclose為、幅よせopen処理へ
		/* 紙幣は逆転後 */
			ex_reject_retry_uba++;
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_multi_job.busy |= TASK_ST_CENTERING;
			_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 1, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 152);
		}
		break;

	case TMSG_CENTERING_EXEC_RSP:

			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_CENTERING);// 起動タスクにより変更

				// next step 幅よせHome完了 */
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_RETRY, ex_reject_retry_uba, 0, 0);
			}
			//#if defined(NEW_SEN)
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_CENTERING);// 起動タスクにより変更

				ex_multi_job.busy &= ~(TASK_ST_CENTERING);
				ex_multi_job.reject |= TASK_ST_CENTERING;
				ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;

				//_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_RETRY, ex_reject_retry_uba, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{
			// 幅寄せタスクが動作開始

			}
			break;

	//#if defined(NEW_SEN)
	case TMSG_CENTERING_HOME_OUT_RETRY_RSP:

			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_CENTERING);// 起動タスクにより変更

				// next step 幅よせHome完了 */
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_RETRY, ex_reject_retry_uba, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{
			// 幅寄せタスクが動作開始

			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_CENTERING);// 起動タスクにより変更

				ex_multi_job.alarm |= TASK_ST_CENTERING;
				ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;

				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			break;

	case TMSG_APB_EXEC_RSP:
		break;
		
#if defined(UBA_RTQ)
	case TMSG_RC_STATUS_INFO:
		 if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		 {
		 ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
	case TMSG_RC_SENSOR_RSP:
		if (!(is_ld_mode()) && (SENSOR_STACKER_HOME)) 
		{
			ex_multi_job.busy |= TASK_ST_STACKER;
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HALF_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_REJECT, REJECT_MODE2_STACKER_HALF_PB_CLOSE);
		}
		else if( (SENSOR_APB_HOME) && ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION) )  //2024-09-30
		{
			ex_multi_job.busy |= TASK_ST_APB;
			_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
			_main_set_mode(MODE1_REJECT, REJECT_MODE2_STACKER_HALF_PB_CLOSE);
		}
		else
		{
			_main_set_reject_standby_note_stay();
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
		}
		break;
#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 153);
		}
		break;
	}
}


void stacker_half_pb_close(void) //ok rtq
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)	/* '19-03-18 */
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif
		break;
	case TMSG_STACKER_HALF_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
		#if defined(UBA_RTQ)
				if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)  //2024-09-30
				{
					if (SENSOR_APB_HOME)
					{
						ex_multi_job.busy |= TASK_ST_APB;
						_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
					}
					else
					{
						_main_set_reject_standby_note_stay();
						_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
					}
				}
				else
				{
					_main_set_reject_standby_note_stay();
					_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
				}
		#else
				if (SENSOR_APB_HOME)
				{
					ex_multi_job.busy |= TASK_ST_APB;
					_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
				}
				else
				{
					_main_set_reject_standby_note_stay();
					_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
				}
		#endif
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 84);
		}
		break;
	//2024-06-07
	case	TMSG_APB_CLOSE_RSP: // MUL 主の処理2 close
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);
				if(!(is_ld_mode()) && (SENSOR_STACKER_HOME))
				{
				//先に収納、PBの順なのでここの入る事はない	
					ex_multi_job.busy |= TASK_ST_STACKER;
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HALF_REQ, 0, 0, 0, 0);
				}
				else
				{
					_main_set_reject_standby_note_stay();
					_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{

			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			break;

#if defined(UBA_RTQ)		/* '18-05-01 */
	case	TMSG_RC_STATUS_INFO:
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
#endif
#if defined(UBA_RTQ)		/* '19-03-07 */
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
#endif
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 85);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief sensor active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
//void reject_centering_close_sensor_active(void)
void reject_note_removed_wait_sensor_active(void) //ok rtq
{
	switch (ex_main_msg.tmsg_code)
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
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
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
				if(ex_wid_reject_uba == 1)
				{
					ex_wid_reject_uba = 0;
					ex_multi_job.busy |= TASK_ST_CENTERING;
					_main_set_mode(MODE1_REJECT, REJECT_MODE2_WAIT_WID);
					_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 0, 0, 0, 0);
				}
				else if(SENSOR_CENTERING_HOME)
				{
				//返却紙幣が取り除かれたと確定した処理へ
				//UBA500RTQはここで、搬送逆転を行っていた。
				//UBA500は入口ローラが左右に広がる構造立ったため、
				//紙幣を引き抜くと、入口ローラが左右に広がったままになる可能性がある
				//その場合、次に紙幣取り込み動作を行う場合にメカ的によくないという事で
				//このに搬送逆転動作がディーボルト対応でRTQには追加されていた。
				//UBA700ではローラは左右に動かないので、このでの搬送逆転処理は廃止する
					_main_set_mode(MODE1_REJECT, REJECT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_SUCCESS, 0, 0, 0);
				}
				else
				{
					ex_multi_job.busy |= TASK_ST_CENTERING;
					_main_set_mode(MODE1_REJECT, REJECT_MODE2_WAIT_WID);
					_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 0, 0, 0, 0);
				}
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 151);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
#endif // UBA_RTQ
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
			_main_system_error(0, 151);
		}
		break;
	}
}


void reject_wait_wid(void) //ok rtq	// 2種類のいずれか、Home確認か幅よせ1回動作
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)	/* '19-03-18 */
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif
		break;

	case	TMSG_CENTERING_EXEC_RSP:	/* 紙幣取り除かれ後、返却時のリトライ動作ありの場合 幅よせ動作1度動作中 */
			if (ex_main_msg.arg1 == TMSG_SUB_START)
			{
				//
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~TASK_ST_CENTERING;
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
				// next step 紙幣が取り除かれた
				//返却紙幣が取り除かれたと確定した処理へ
				//UBA500RTQはここで、搬送逆転を行っていた。
				//UBA500は入口ローラが左右に広がる構造立ったため、
				//紙幣を引き抜くと、入口ローラが左右に広がったままになる可能性がある
				//その場合、次に紙幣取り込み動作を行う場合にメカ的によくないという事で
				//このに搬送逆転動作がディーボルト対応でRTQには追加されていた。
				//UBA700ではローラは左右に動かないので、このでの搬送逆転処理は廃止する
					_main_set_mode(MODE1_REJECT, REJECT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_SUCCESS, 0, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_CENTERING);
				ex_multi_job.alarm |= TASK_ST_CENTERING;
				ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 53);
			}
			break;

	case TMSG_SENSOR_STATUS_INFO: /* 紙幣取り除かれ後の、幅よせ動作中のセンサ変化 */
		//UBA500では処理があるが、紙幣取り除き後の幅よせ動作中なので、モード遷移させないほうがいいので、削除 */
		break;
#if 0
	case TMSG_APB_EXEC_RSP:
		break;
#endif
#if defined(UBA_RTQ)	/* '18-05-01 */
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
#endif
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 86);
		}
		break;
	}
}




/*********************************************************************//**
 * @brief wait request (reject state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reject_wait_req(void) //ok rtq
{
	switch(ex_main_msg.tmsg_code)
	{
	#if 1 //2023-11-20
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	#endif
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
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		clear_ex_multi_job();	//2022-02-21
		_main_set_disable();
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		clear_ex_multi_job();	//2022-02-21
		_main_set_enable();
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)	/* '19-03-18 */
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif
		break;
#if defined(UBA_RTQ)	/* '18-05-01 */
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		 _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		 ex_rc_collect_sw = 0;
		 break;
#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_REJECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 177);
		}
		break;
	}
}

/* EOF */
