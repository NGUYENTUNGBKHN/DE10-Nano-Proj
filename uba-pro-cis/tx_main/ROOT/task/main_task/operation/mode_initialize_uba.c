/******************************************************************************/
/*! @addtogroup Main
    @file       mode_initialize.c
    @brief      initialize mode of main task
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
#include "pl/pl_cis.h"
#include "sub_functions.h"
#include "status_tbl.h"
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
#if defined(UBA_RTQ)
static u8 s_recovery_unit;
static u8 s_recovery_type;
#endif // UBA_RTQ
/************************** PRIVATE FUNCTIONS *************************/
void init_sub_select_mode(void);
u8 initial_position_uba(void);
/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/


/*********************************************************************//**
 * @brief initialize message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void initialize_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case	INIT_MODE2_SENSOR_ACTIVE:
			init_sensor_active();	/* common *//* ok */
			break;

	case INIT_MODE2_ICB:
			init_icb();
			break;
	case INIT_MODE2_CIS_INITIALIZE:
			init_cis_init();
			break;

	// イニシャル動作開始前の動作
	case	INIT_MODE2_INITIAL_POSITION:
			init_initial_position();	/* 上記の処理を全てこの1つにまとめた方がいいのでは？ */ 
			break;
	// 通常のイニシャル動作
	case	INIT_MODE2_STACKER:
			init_stacker();// 収納イニシャル　/* common *//* ok オリジナル追加*/
			break;
	case	INIT_MODE2_FEED:
			init_feed();// 搬送イニシャル /* common *//* ok */
			break;
	case	INIT_MODE2_APB:
			init_apb();	// 1回転させHomeへ /* ok 500完全移植 */
			break;
	case	INIT_MODE2_SHUTTER:
			init_shutter();	// PBの後にシャッターを行い、その後、幅よせ
			break;
	case	INIT_MODE2_CENTERING:
			init_centering();			/* ok 500完全移植 */
			break;

	case	INIT_MODE2_WAIT_REQ:
			init_wait_req();// LINEからのイニシャル完了許可待ち(イニシャル動作の最後) /* common *//* ok オリジナル追加*/
			break;
	// イニシャル前、イニシャル中に紙幣を検知した為、リカバリ処理
	case	INIT_MODE2_WAIT_REMAIN_REQ:
			init_wait_remain_req();	// 紙幣がある為、LINEからの返却命令 or 収納命令待ち　//common 500も移植 ok
			break;
	case	INIT_MODE2_FORCE_FEED_STACK:
			init_force_feed_stack();// LINEからの収納命令の為、紙幣搬送 common 500も移植 OK
			break;
	case	INIT_MODE2_FORCE_STACK:
			init_force_stack();// LINEからの収納命令の為、紙幣搬送 only 完全移植　ok
			break;
	case	INIT_MODE2_FEED_REJECT:
			init_feed_reject();// 返却処理 common 移植　ok
			break;
	case	INIT_MODE2_REJECT_APB_HOME:
			init_reject_apb_home(); // 返却の為、PBをHomeに戻す、その後紙幣返却
			break;
	case	INIT_MODE2_NOTE_STAY:
			init_note_stay();	// 返却紙幣の取り除き待ち
			break;
	case INIT_MODE2_WAIT_RESET_REQ:
			init_wait_reset_req();	// 返却紙幣の取り除き完了 or 紙幣収納の完了の為、LINEからのイニシャル許可待ち
			break;
#if defined(UBA_RTQ)
	case INIT_MODE2_RC:
		init_rc();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_DRUM_GAP_ADJ:
		init_rc_wait_recovery_drum_gap_adj();
		break;
	case INIT_MODE2_RC_WAIT_LAST_FEED:	//case2 通常のイニシャルから呼び出される、このあと通常のイニシャルスタッカーへ戻る
		init_rc_wait_last_feed();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_BACK: //case1-1
		init_rc_wait_recovery_back();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_STACK_HOME://case1-1
		init_rc_wait_recovery_stack_home();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_INIT_RC://case1-1
		init_rc_wait_recovery_init_rc();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_BACK_BOX://case1-1
		init_rc_wait_recovery_back_box();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM:	//この次、INIT_MODE2_RC_WAIT_RECOVERY_BACK　へ合流
	case INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM_FOR_COLLECT: //この次　INIT_MODE2_RC_WAIT_RECOVERY_PAYDRUM_BOX__PAYDRUM_BOX
		init_rc_wait_recovery_force_stack_drum();
		break;
	#if 0
	case INIT_MODE2_RC_WAIT_RECOVERY_FRONT_BACK_BOX__FRONT:
		init_rc_wait_recovery_front_back_box__front();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_FRONT_BACK_BOX__INIT_RC1:
		init_rc_wait_recovery_front_back_box__init_rc1();
		break;
	#endif
	case INIT_MODE2_RC_WAIT_RECOVERY_PAYDRUM_BOX__PAYDRUM_BOX: //この次 INIT_MODE2_RC_WAIT_RECOVERY_STACK へ
		init_rc_wait_recovery_paydrum_box__paydrum_box();
		break;

	#if 0
	case INIT_MODE2_RC_WAIT_RECOVERY_BOX_SEARCH_BOX__BOX_SEARCH:
		init_rc_wait_recovery_box_search_box__box_search();
		break;

	case INIT_MODE2_RC_WAIT_RECOVERY_BOX_SEARCH_BOX__BOX:
		init_rc_wait_recovery_box_search_box__box();
		break;
	#endif

	case INIT_MODE2_RC_WAIT_RECOVERY_STACK://case1-1
		init_rc_wait_recovery_stack();
		break;
	case INIT_MODE2_RC_WAIT_RECOVERY_BILL_BACK: //この後　INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM
	case INIT_MODE2_RC_WAIT_RECOVERY_BILL_BACK_FOR_COLLECT: //この後　INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM_FOR_COLLECT
		init_rc_wait_recovery_bill_back();
		break;
#endif // UBA_RTQ

	default:
		/* system error ? */
		_main_system_error(0, 200);
		break;
	}
}


/*********************************************************************//**
 * @brief sensor active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_sensor_active(void)	/* common *//* ok */
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
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
#if defined(UBA_RTQ)
			if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_twin()) ||    /* detect twin box */
					!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_jam_check()) 	|| 				/* detect jam */
					!(is_detect_rc_internal_jam())	|| 				/* detect jam */
					!(is_detect_rc_internal_jam_initial()))			/* detect jam */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ALARM_CODE_RC_TRANSPORT, _main_conv_seq(), ex_position_sensor);
			}
		//#if defined(UBA_RS)
			else if (is_rs_mode_remain_note_check())			/* detect bill in RS unit */
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_FEED_OTHER_SENSOR_AT, _main_conv_seq(), ex_position_sensor);
			}
		//#endif
			else
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_READ_MAINTE_SERIALNO_REQ, 0, 0, 0, 0);
			}
#else
			ex_multi_job.busy &= ~TASK_ST_SENSOR;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				//ivizon2はこのタイミングでRFIDの確認をしているが、ubaはイニシャルの最後で行う
				//理由はID-003のイニシャル中のICB有効無効設定コマンドがある為、
				//このタイミングではICB有効無効が確定しない場合がある為
				ex_multi_job.busy |= TASK_ST_CIS_INIT;
				_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_BILL_IN, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_CIS_INITIALIZE);

			}	
#endif 
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 151);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_READ_MAINTE_SERIALNO_RSP:

		if(!(is_rc_serial_no_check()) || ex_rc_exchanged_unit_powerup == 1) //2024-12-11
		{
			/* RC unit were swapped */
			ex_rc_exchanged_unit_powerup = 0;
			ex_rc_exchanged_unit = 1;
		}
		else
		{
			ex_rc_exchanged_unit = 0;
		}

		_main_send_msg(ID_RC_MBX, TMSG_RC_READ_EDITION_REQ, 1, 0, 0, 0);
		break;
	case TMSG_RC_READ_EDITION_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_GET_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_GET_RECYCLE_SETTING_RSP:
		ex_multi_job.busy &= ~TASK_ST_SENSOR;
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
		else if (!(is_detect_rc_twin()) || /* detect twin*/
				!(is_detect_rc_quad()))		/* detect quad */
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			if (!(ex_multi_job.busy))
			{ /* all job end */
				//ivizon2はこのタイミングでRFIDの確認をしているが、ubaはイニシャルの最後で行う
				//理由はID-003のイニシャル中のICB有効無効設定コマンドがある為、
				//このタイミングではICB有効無効が確定しない場合がある為
				ex_multi_job.busy |= TASK_ST_CIS_INIT;
				_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_BILL_IN, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_CIS_INITIALIZE);
			}
		}
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
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
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
#endif // uBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 201);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief feed init procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_feed(void) /* common *//* ok */
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	case TMSG_FEED_INITIAL_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);

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
				// PBイニシャルへ
			#if defined(UBA_RTQ)
				_main_set_mode(MODE1_INIT, INIT_MODE2_RC);
				_main_send_msg(ID_RC_MBX, TMSG_RC_SET_MOTOR_SPEED_REQ, 0, 0, 0, 0);
			#else
				ex_multi_job.busy |= TASK_ST_APB;
				_main_set_mode(MODE1_INIT, INIT_MODE2_APB);
				_main_send_msg(ID_APB_MBX, TMSG_APB_INITIAL_REQ, 0, 0, 0, 0);			
			#endif // UBA_RTQ		
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);

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
				_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REMAIN_REQ);
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_INIT_REJECT_REQUEST, ex_main_msg.arg2, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);

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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 204);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
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
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
#endif // UBA_RTQ
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				if (!(is_detect_rc_twin()) || 
					!(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 205);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief stacker init procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_stacker(void) /* common */
{
	u8 denomi = 0;

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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif
		break;
	case TMSG_STACKER_EXEC_RSP:
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
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				/* 2022-01-17 */
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);
			}
		}
		else if(ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
		// 押し込みの頂点時にすでにリトライが確定している場合は、Retryメッセージを受信するようにする
			ex_multi_job.busy &= ~(TASK_ST_STACKER);

		// SUBの押しメカがエラーが発生した可能性があるので確認する
			if(ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if(!(is_box_set()))
			{ /* box unset */
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
			// next step
			// 押し込みリトライ処理へ
				ex_multi_job.busy |= TASK_ST_STACKER;
				// スタッカHomeへ
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_INTERIM)
		{
			/* system error ? */
			_main_system_error(0, 19);
		}
		break;

	case	TMSG_STACKER_PULL_RSP:	// MUL 主の処理1
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS) // 収納成功
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// next step
					_main_set_mode(MODE1_INIT, INIT_MODE2_FEED);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_INITIAL_REQ, 0, 0, 0, 0);
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
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ENABLE_NEXT)
			{
				// 
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 226);
			}
			break;

	case	TMSG_STACKER_EXEC_NG_PULL_RSP:	//Setp1  1度目の押し込みでNG、押しメカ戻し動作
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// 戻し動作は成功したので、モードはこのままで押し込みリトライ命令を行う
					ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, 0, 0, 0, 0);	// リトライ用の押し込み命令へ
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 229);
			}
			break;
	case	TMSG_STACKER_EXEC_RE_RSP:	//Setp2  1度目の押し込みでNG、押しメカ戻し動作
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					_main_set_mode(MODE1_INIT, INIT_MODE2_FEED);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_INITIAL_REQ, 0, 0, 0, 0);
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
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 230);
			}
			break;
#if defined(UBA_RTQ)		/* '19-03-07 */
	case 	TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_RC_STATUS_INFO:
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
				}
			}
			break;
#endif
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 20);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief centering init procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_centering(void) /* ok 500完全移植 */
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
			//UBA500とは異なる、UBA500の場合 RTQ外れを監視して外れている場合 いきなりエラーに飛ばしている
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif // UBA_RTQ
		break;
	case TMSG_CENTERING_EXEC_RSP:
		ex_multi_job.busy &= ~(TASK_ST_CENTERING);
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
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				//UBA500の場合は、幅よせ、PB,ICB
				//UBA500の場合は、				幅よせ、PB,ICB
				//ivizionに近づける場合は、		 幅よせ、で終了
			#if defined(UBA_RTQ)
				if(ex_rc_error_flag)
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
				}
				else if(!(is_detect_rc_twin()) || 
					!(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				#if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20
				//紙幣ありエージングで、RFDI通信も行う為、強制的に遷移
					_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_RESET_REQ, RFID_RUN, 0, 0, 0);
				#else
					if (is_ld_mode() || is_test_mode() || !(is_icb_enable()))
					{
						_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);
					}
					else
					{
						_main_set_mode(MODE1_INIT, INIT_MODE2_ICB);
						ex_multi_job.busy |= TASK_ST_ICB;
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_INITIAL_REQ, 0, 0, 0, 0);
					}
					_main_send_msg(ID_RC_MBX, TMSG_RC_WU_RESET_REQ, 0, 0, 0, 0);
				#endif
				}
			#else	//SS
				if (is_ld_mode() || is_test_mode())
				{
					_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);
				}
				else
				{					
				/* ICB確認だけではない*/
				/* リカバリ処理もある(電源OFF,通信エラー後Reset,動作中Reset)*/
				/* ICB書き込み処理まで進み書き込む処理をFRAMにバックアップ以降 通信エラー、リセット、電源OFF*/
				/* Stackコマンド受信～FRAMバックアップ前のリカバリはこの次に行っている */
					_main_set_mode(MODE1_INIT, INIT_MODE2_ICB);
					ex_multi_job.busy |= TASK_ST_ICB;
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_INITIAL_REQ, 0, 0, 0, 0);
				}
			#endif // UBA_RTQ
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 206);
		}
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ)
	//UBA500と異なるが、PBイニシャルで待たせているので、ここには必要ない
	//case TMSG_RC_RESET_RSP:
	//	_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
	//	break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
	#if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20
	case TMSG_RC_RFID_RESET_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);
			_main_send_msg(ID_RC_MBX, TMSG_RC_WU_RESET_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{

		}
		break;
	#endif
#endif // UBA_RTQ
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 207);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief apb init procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_apb(void) /* ok 500完全移植 */ //ここでRTQ側のイニシャル完了を待つ
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		//UBA500は TIMER_ID_RC_CHECKを入れているが必要ないので廃止
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
		// ここでは、RTQのイニシャル完了も待っているので大切 UBA500と若干変えている
			if(	
				(
				!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad())
				)
				&& 
				ex_rc_error_flag == 0
				)
			{
				ex_rc_error_flag = ALARM_CODE_RC_REMOVED;
			}
			else
			{
				if (!(ex_multi_job.busy) && !(rc_initial_status()))
				{
					if(ex_rc_error_flag)
					{
						_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
					}
					else if(!(is_detect_rc_twin()) || 
						!(is_detect_rc_quad()))
					{
						_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
					}
					//UBA500RTQと若干変える、この条件を追加 2025-08-19
					else if(ex_rc_status.sst1A.bit.error == 1)
					{
						//RTQ側でエラーが発生しても通知はタスクメッセージ
						//RTQ側がイニシャル中かは rc_initial_status() で直接参照している
						//その為、
						//エラー発生後、エラー発生のタスクメッセージ受信前に、このタイマイベントが発生した場合、
						//次のシーケンスに遷移してしまう。
						//対策としてここでエラーを見て、エラーの場合、遷移させない

						//すでにエラー発生しているのでエラーメッセージ受信を待つ
						// TIMER_ID_RC_STATUS を送信し続ける
					}
					else 
					{
						//2025-08-20 UBAと変えている、この処理がなかったので改善した
						if (ex_multi_job.alarm)
						{ /* other job alarm *//* PBエラーが発生しているので */
							_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
						}
						else
						{
						//PB RTQイニシャル完了なので次へ
							ex_multi_job.busy |= TASK_ST_SHUTTER;
							_main_set_mode(MODE1_INIT, INIT_MODE2_SHUTTER);
							_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
						}
					}
				}
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_APB_INITIAL_RSP:
	case TMSG_APB_EXEC_RSP:
	case TMSG_APB_HOME_RSP:

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
			else
			{
			//UBA500の場合は次に幅よせ,、PB->幅よせ　 iVIZIONの場合はPBなしの幅よせEnd
			// 幅よせあり、幅よせイニシャルへ			
			#if defined(UBA_RTQ)
				if(ex_rc_error_flag)
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
				}
				else if(!(is_detect_rc_twin()) || 
					!(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					if(ex_rc_status.sst1A.bit.error == 1) //2025-08-19
					{
					//すでにエラー発生しているのでエラーメッセージ受信を待つ
						break;
					}

					if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)  //2024-09-30
					{
					//RTQは暗号化有効時のみPB close //このシーケンスをキープ
						ex_multi_job.busy |= TASK_ST_APB;
						_main_set_mode(MODE1_INIT, INIT_MODE2_APB);
						_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);//2025-01-18 保護処理 このシーケンスでこのタイマを使用してRTQ側のイニシャル完了を待つ必要があるので念のため			
					}
					else
					{
						if (!(ex_multi_job.busy) && !(rc_initial_status()))
						{
							ex_multi_job.busy |= TASK_ST_SHUTTER;
							_main_set_mode(MODE1_INIT, INIT_MODE2_SHUTTER);
							_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
						}
					}
				}
			#else
				#if (DATA_COLLECTION_DEBUG!=1)	//2022-04-18 PB closeを追加
				ex_multi_job.busy |= TASK_ST_APB;
				_main_set_mode(MODE1_INIT, INIT_MODE2_APB);
				_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);			

				#else
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_set_mode(MODE1_INIT, INIT_MODE2_SHUTTER);
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
				#endif
			#endif  // UBA_RTQ
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			 //2025-08-20 UBAと変えている、改善した
			//ドラムのイニシャル中にHead側からいきなりエラーにするのはよくないとこ事(安高 曰く)
			//実際に試したが、ドラムが変な挙動をしていたので、いきなりエラーにはしないで改善させる
			ex_multi_job.busy &= ~(TASK_ST_APB);
		#if defined(UBA_RTQ)
			if(ex_rc_error_flag)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				if(ex_rc_status.sst1A.bit.error == 1) //2025-08-19
				{
				//すでにエラー発生しているのでエラーメッセージ受信を待つ
					break;
				}
				else if(ex_test_finish)
				{
					_main_set_test_standby();
				}
				else if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					if (!(rc_initial_status()))
					{
					//RTQ側は完了しているのでエラーへ
						_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
					else
					{
						ex_multi_job.alarm |= TASK_ST_APB;
						ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
						ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
						ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
					}
				}
			}
		#else	//SS
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		#endif
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 208);
		}
		break;
	//2022-04-18
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
			else
			{
			//UBA500の場合は次に幅よせ,、PB->幅よせ　 iVIZIONの場合はPBなしの幅よせEnd
			// 幅よせあり、幅よせイニシャルへ
			#if defined(UBA_RTQ)
				if(ex_rc_error_flag)
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
				}
				else if(!(is_detect_rc_twin()) || 
					!(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					if (!(ex_multi_job.busy) && !(rc_initial_status()))
					{
						if(ex_rc_status.sst1A.bit.error == 1) //2025-08-19
						{
							//すでにエラー発生しているのでエラーメッセージ受信を待つ
						}
						else
						{
							ex_multi_job.busy |= TASK_ST_SHUTTER;
							_main_set_mode(MODE1_INIT, INIT_MODE2_SHUTTER);
							_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
						}
					}
				}
			#else
				/* 先にシャッター */
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_set_mode(MODE1_INIT, INIT_MODE2_SHUTTER);
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
			#endif // UBA_RTQ
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			//2025-08-20
			ex_multi_job.busy &= ~(TASK_ST_APB);

		#if defined(UBA_RTQ)
			if(ex_rc_error_flag)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				if(ex_rc_status.sst1A.bit.error == 1) //2025-08-19
				{
				//すでにエラー発生しているのでエラーメッセージ受信を待つ
					break;
				}
				else if(ex_test_finish)
				{
					_main_set_test_standby();
				}
				else if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					if (!(rc_initial_status()))
					{
					//RTQ側は完了しているのでエラーへ
						_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
					else
					{
						ex_multi_job.alarm |= TASK_ST_APB;
						ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
						ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
						ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
					}
				}
			}
		#else	//SS
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		#endif

		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 208);
		}
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_RESET_RSP:
		//UBA500はコールしているが必要ないので廃止 _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
#endif // UBA_RTQ
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}



void init_initial_position(void) //2022-02-14
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
#endif
	case TMSG_APB_EXEC_RSP:
		break;
	/*-------------------------------------------------*/	
	/* 幅よせ start  init_centering_homeから移植*/
	case TMSG_CENTERING_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_CENTERING);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				if(!(initial_position_uba()))//2023-03-24
				{
					init_sub_select_mode();
				}
				else
				{
					/* モード継続 */
				}
			}
		}
		else if(ex_main_msg.arg1 == TMSG_SUB_START)
		{

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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 15);
		}
		break;
	/* 幅よせ end */
	/*-------------------------------------------------*/	
	/*-------------------------------------------------*/	

	case TMSG_STACKER_HOME_RSP:
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
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				if(!(initial_position_uba()))//2023-03-24
				{
					init_sub_select_mode();
				}
				else
				{
					/* モード継続 */
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 17);
		}
		break;
	/* stacker end */
	/*-------------------------------------------------*/	
	/*-------------------------------------------------*/	
	/* pb start   から移植*/

	case TMSG_APB_HOME_RSP:

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
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else
			{
				if(!(initial_position_uba()))//2023-03-24
				{
					init_sub_select_mode();
				}
				else
				{
					/* モード継続 */
				}
			}
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
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0,210);
		}
		break;
	/* pb end */
	/*-------------------------------------------------*/	
	/*-------------------------------------------------*/	
	/* shutter start   から移植*/

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
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else
			{
				if(!(initial_position_uba()))//2023-03-24
				{
					init_sub_select_mode();
				}
				else
				{
					/* モード継続 */
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;

			/* テスト用なので、他のタスクを待たないでいきなりエラーにする */
			ex_multi_job.alarm |= TASK_ST_SHUTTER;
			ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
			ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
			ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;

			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			}
		}
		break;

	/* shutter end */
	/*-------------------------------------------------*/	

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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0,211);
		}
		break;
	}
}





/*********************************************************************//**
 * @brief wait enable/disable request procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_wait_req(void)
{
	u16 bill_in;
	
	switch(ex_main_msg.tmsg_code)
	{
#if 1 //2023-11-20
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
		ex_force_stack_retry = 0;	//2023-11-27

	#if defined(UBA_RTQ)		/* '19-10-01 */
		ex_rc_exchanged_unit = 0;
	#endif

		bill_in = _main_bill_remain();
		if ((bill_in == BILL_IN_NON) || (bill_in == BILL_IN_ENTRANCE))
		{
#if defined(UBA_RTQ)
			/* Set RTQ state IDLE */
			_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_IDLE, 0, 0, 0);
			ex_rc_collect_sw = 0;
#endif // UBA_RTQ			
			// dummy set disable mode for first adjustment
			_main_set_mode(MODE1_DISABLE, DISABLE_MODE2_WAIT_REQ);
			_main_set_adjustment();
		}
		else
		{
			_main_set_disable();
		}
		break;
	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
		ex_force_stack_retry = 0;	//2023-11-27
	#if defined(UBA_RTQ)		/* '19-10-01 */
		ex_rc_exchanged_unit = 0;
	#endif
		
		bill_in = _main_bill_remain();
		if ((bill_in == BILL_IN_NON) || (bill_in == BILL_IN_ENTRANCE))
		{
#if defined(UBA_RTQ)
			/* Set RTQ state IDLE */
			_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_IDLE, 0, 0, 0);
			ex_rc_collect_sw = 0;
#endif // UBA_RTQ
			// dummy set enable mode for first adjustment
			_main_set_mode(MODE1_ENABLE, DISABLE_MODE2_WAIT_REQ);
			_main_set_adjustment();
		}
		else
		{
			_main_set_enable();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
#endif
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 212);
		}
		break;
	}
}
#if defined(UBA_RTQ)
static void mode_initialize_uba_recovery_type_proc()
{
	if (s_recovery_type == GAP_ADJ_DRUM)
	{
		// s_recovery_unit = recovery_unit;
		// ドラムの紙幣間隔調整
		ex_multi_job.busy |= TASK_ST_RC;

		_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_GAP_ADJ_REQ, s_recovery_unit, 0, 0, 0);
		_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_DRUM_GAP_ADJ);
	}
	else if (s_recovery_type == BILLBACK_FEEDBOX) /* BILL is going to transport to BOX */
	{
		// s_recovery_unit = recovery_unit;

		// 紙幣戻し→BOXに搬送
		ex_multi_job.busy |= TASK_ST_RC;
		ex_multi_job.busy |= TASK_ST_FEED;

		_main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, s_recovery_unit, RC_RETRY_PAYOUT_DIR, 0, 0);
		_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);

		_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK);
	}
	else if (s_recovery_type == BILLBACK_PAYDRUM_FEED_BOX_FOR_STACK) /* BILL is going to transport to DRUM/STACK */
	{
		// s_recovery_unit = recovery_unit;

		// 紙幣戻し→ドラム払出し→BOXに搬送
		ex_multi_job.busy |= TASK_ST_RC;
		ex_multi_job.busy |= TASK_ST_FEED;

		if (s_recovery_unit == RC_TWIN_DRUM1 || s_recovery_unit == RC_QUAD_DRUM1)
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_BILLBACK2_REQ, s_recovery_unit, 0, 0, 0);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_BILLBACK_REQ, s_recovery_unit, 0, 0, 0); //20mm戻し
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BILL_BACK);
		}
		else
		{
#if 1 // POLYMOR
			_main_send_msg(ID_RC_MBX, TMSG_RC_FORCE_STACK_DRUM_REQ, s_recovery_unit, 0, 0, 0);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, s_recovery_unit, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM);
#else
			_main_send_msg(ID_RC_MBX, TMSG_RC_BILLBACK_DRUM_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK);
#endif
		}
	}
	else if (s_recovery_type == BILLBACK_PAYDRUM_FEED_BOX_FOR_PAYOUT)
	{
//		s_recovery_unit = recovery_unit;

		// 紙幣戻し→ドラム払出し→BOXに搬送
		ex_multi_job.busy |= TASK_ST_RC;
		ex_multi_job.busy |= TASK_ST_FEED;

#if 1 // POLYMOR
		_main_send_msg(ID_RC_MBX, TMSG_RC_FORCE_STACK_DRUM_REQ, s_recovery_unit, 0, 0, 0);
		_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, s_recovery_unit, 0, 0, 0);
		_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM);
#else
		_main_send_msg(ID_RC_MBX, TMSG_RC_BILLBACK_DRUM_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
		_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
		_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK);
#endif
	}
	else if (s_recovery_type == FEED_BOX)
	{
		// s_recovery_unit = recovery_unit;

		// BOXに搬送
		if (!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
		{
			ex_multi_job.busy |= TASK_ST_STACKER;

			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK_HOME);
		}
		else
		{
			u8 flap1_pos;
			u8 flap2_pos;

			switch (s_recovery_unit)
			{
			case RC_CASH_BOX:
				/* flappr1 position check */
				if (!(is_flapper1_head_to_box_pos()))
				{
					flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
				}
				else
				{
					flap1_pos = 0; /* don't move		*/
				}
				/* flapper2 position check */
				if (is_quad_model() && !(is_flapper2_head_to_box_pos()))
				{
					flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX; /* change position	*/
				}
				else
				{
					flap2_pos = 0; /* don't move		*/
				}
				break;
			case RC_TWIN_DRUM1:
			case RC_TWIN_DRUM2:
				/* flappr1 position check */
				if (!(is_flapper1_twin_to_box_pos()))
				{
					flap1_pos = RC_FLAP1_POS_RC_TO_BOX; /* change position	*/
				}
				else
				{
					flap1_pos = 0; /* don't move		*/
				}
				/* flapper2 position check */
				if (is_quad_model() && !(is_flapper2_head_to_box_pos()))
				{
					flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX; /* change position	*/
				}
				else
				{
					flap2_pos = 0; /* don't move		*/
				}
				break;
			case RC_QUAD_DRUM1:
			case RC_QUAD_DRUM2:
				/* flappr1 position check */
				flap1_pos = 0; /* don't move		*/
				/* flapper2 position check */
				if (is_quad_model() && !(is_flapper2_quad_to_box_pos()))
				{
					flap2_pos = RC_FLAP2_POS_RC_TO_BOX; /* change position	*/
				}
				else
				{
					flap2_pos = 0; /* don't move		*/
				}
				break;
			}

			/* send flapper command */
			if (flap1_pos == 0 && flap2_pos == 0)
			{
				ex_multi_job.busy |= TASK_ST_RC;

				_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, s_recovery_unit, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK_BOX);
			}
			else
			{
				_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_INIT_RC);
				_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
			}
		}
	}
	else if (s_recovery_type == PAYDRUM_FEED_BOX)
	{
		// s_recovery_unit = recovery_unit;

		if (s_recovery_unit == RC_TWIN_DRUM1 || s_recovery_unit == RC_QUAD_DRUM1)
		{
			ex_multi_job.busy |= TASK_ST_RC;
			ex_multi_job.busy |= TASK_ST_FEED;
			_main_send_msg(ID_RC_MBX, TMSG_RC_BILLBACK2_REQ, s_recovery_unit, 0, 0, 0);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_BILLBACK_REQ, s_recovery_unit, 0, 0, 0); //20mm戻し
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BILL_BACK_FOR_COLLECT);
		}
		else
		{
#if 1		// POLYMOR
			//  紙幣戻し→ドラム払出し→BOXに搬送
			ex_multi_job.busy |= TASK_ST_RC;
			ex_multi_job.busy |= TASK_ST_FEED;
			_main_send_msg(ID_RC_MBX, TMSG_RC_FORCE_STACK_DRUM_REQ, s_recovery_unit, 0, 0, 0);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, s_recovery_unit, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM_FOR_COLLECT);
#else
			// ドラム払出し→BOXに搬送
			ex_multi_job.busy |= TASK_ST_RC;
			_main_send_msg(ID_RC_MBX, TMSG_RC_FEEDBOX_DRUM_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_PAYDRUM_BOX__PAYDRUM_BOX);
#endif
		}
	}
	// else if (s_recovery_type == BOX_SEARCH_FEED_BOX)
	// {
	// 	s_recovery_unit = recovery_unit;

	// 	// BOX内の紙幣有無確認 有の場合BOXに搬送する 無の場合終了
	// 	ex_multi_job.busy |= TASK_ST_RC;

	// 	_main_send_msg(ID_RC_MBX, TMSG_RC_SEARCH_REQ, 0, 0, 0, 0);

	// 	_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BOX_SEARCH_BOX__BOX_SEARCH);
	// }
	else if (s_recovery_type == BILL_CHEAT)
	{
		_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_CHEAT, _main_conv_seq(), ex_position_sensor);
	}
}
#endif // uBA_RTQ

/*********************************************************************//**
 * @brief wait remain request procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_wait_remain_req(void)
{
	u16 bill_in;
	
	switch(ex_main_msg.tmsg_code)
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
	case TMSG_CLINE_STACK_REQ:
	case TMSG_DLINE_STACK_REQ:

		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
#if defined(UBA_RTQ)
		else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
		}
#endif
		// 特殊な状況 ID-008などの
		else if((!(SENSOR_APB_HOME)) && ((SENSOR_CENTERING) /* || !(SENSOR_VALIDATION_OFF) */ || (SENSOR_APB_IN)))
		{
		// PBがHomeでない && ( 幅寄せがON || 識別ｾﾝｻがON || PB INセンサがON	)
		// LINEから収納命令を受けていて、PBがHomeにない状況で、PB機構より前に紙幣があるので、PBをHomeにする
			ex_multi_job.busy |= TASK_ST_APB;
			_main_set_mode(MODE1_INIT, INIT_MODE2_INITIAL_POSITION);
			_main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);	// use
		}
		//2022-02-14 ここにシャッターopenの処理を追加するかも
		else if(!(SENSOR_SHUTTER_OPEN))
		{
			ex_multi_job.busy |= TASK_ST_SHUTTER;
			_main_set_mode(MODE1_INIT, INIT_MODE2_INITIAL_POSITION);
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
		}
		else
		{
		// 搬送、収納命令を受けているので、搬送して収納する
			ex_main_pause_flag = 0;
			//2023-11-27 
			//待機時に90mm程度の札でEXITのみ検知させた状態の場合、Stacker JAMとなる
			//この時に、Resetコマンドでイニシャルした場合、搬送路逆転でEXITセンサで紙幣を見つけるが、
			//待機時に紙幣を突っ込んでEXITのみで紙幣を検知させた状態だと、スタッカは半押し状態なので、
			//押しメカの裏側に紙幣が入り込んだ状態となっている。
			//紙によっては、Stacker Home戻しで、噛み込んだ状態でもセンサ的にはStacker Homeとなり、物理的にはHomeに戻っていない状態となる。
			//この状態で、いくら収納側に搬送しても、押しメカ裏側に紙幣が搬送されるだけで、収納エラーにはならないが、紙幣は搬送路に残った状態となる
			//再度、紙幣チェックの為、搬送路を逆転すると再度EXITで紙幣を検知するが、スタッカは同様に、センサ的にはStacker Homeとなり、物理的にはHomeに戻っていない状態
			//の為、再度強制搬送、強制収納でも収納できない結果となり、イニシャル動作を永遠に続ける
			//解決する為には、紙幣探しをEXITより手間まで戻して、紙幣を押しメカの裏側から取り出す必要がある
			//紙幣戻し時の引き抜きのリスクがあるので、リトライ3回で強制的にエラーにする
			if(ex_force_stack_retry < 3)
			{
				ex_force_stack_retry++;	
				_main_set_mode(MODE1_INIT, INIT_MODE2_FORCE_FEED_STACK);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_FORCE_STACK_REQ, 0, 0, 0, 0);
			}
			else
			{
			// id003側が TMSG_CONN_STACK で待っているので、
			//	_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_STACKER_TIMEOUT, _main_conv_seq(), ex_position_sensor);
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_FEED_TIMEOUT_SK, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;

	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		// 返却命令
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
#if defined(UBA_RTQ)
		else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
		}
#endif
		else
		{
			bill_in = _main_bill_remain();

			if( (bill_in == BILL_IN_NON) || (bill_in == BILL_IN_ENTRANCE) )
			{
			// 紙幣がない為、紙幣取り除き待ち処理へ
			//	_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_INTERIM, 0, 0, 0);
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_INIT_REJECT_HANGING, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_NOTE_STAY);
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
			}
			else
			{
				if(!(SENSOR_SHUTTER_OPEN))
				{
					ex_multi_job.busy |= TASK_ST_SHUTTER;
					_main_set_mode(MODE1_INIT, INIT_MODE2_INITIAL_POSITION);
					_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
				}
				else if(!(SENSOR_APB_HOME)
				&& ((SENSOR_APB_OUT) || (SENSOR_EXIT))
				){
				// 特殊な状況
				// PBがHomeにない状況で、PB以降に紙幣があり、返却命令をうけたのでPBをHomeに戻す
				// PBがHomeにない && ( PB OUT ON || EXIT ON )
					_main_set_mode(MODE1_INIT, INIT_MODE2_REJECT_APB_HOME);
					_main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);	// use
				}
				else
				{
				#if defined(UBA_RTQ)//#if defined(UBA_RS)
					_main_set_mode(MODE1_INIT, INIT_MODE2_FEED_REJECT);

					if (is_rc_rs_unit())
					{
						if (RS_POS1_ON)
						{
							if (SENSOR_APB_IN || SENSOR_APB_OUT)
							{
								// 返却処理
								_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_INITIAL, 0, 1, 0);
							}
							else
							{
								// 返却処理
								_main_send_msg(ID_FEED_MBX, TMSG_FEED_RS_FORCE_PAYOUT_REQ, 0, 0, 0, 0);
							}
						}
						else if (RS_POS2_ON || RS_POS3_ON)
						{
							// 返却処理
							_main_send_msg(ID_FEED_MBX, TMSG_FEED_RS_FORCE_PAYOUT_REQ, 0, 0, 0, 0);
						}
						else
						{
							// 返却処理
							_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_INITIAL, 0, 1, 0);
						}
					}
					else
					{
						// 返却処理
						_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_INITIAL, 0, 1, 0);
					}
				#else
				// 返却処理 SS
					_main_set_mode(MODE1_INIT, INIT_MODE2_FEED_REJECT);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_INITIAL, 0, 1, 0); //2023-11-01
				#endif
				}
			}
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_CLINE_RC_RECOVERY_REQ:
		s_recovery_type = ex_main_msg.arg2;
		s_recovery_unit = ex_main_msg.arg1;
		if(SENSOR_SHUTTER_OPEN)
		{
			ex_multi_job.busy |= TASK_ST_SHUTTER;
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_CLOSE_REQ, 0, 0, 0, 0);
		}
		else
		{
			mode_initialize_uba_recovery_type_proc();
		}
		break;
	case TMSG_SHUTTER_CLOSE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			mode_initialize_uba_recovery_type_proc();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			// _main_alarm_sub(0, 0, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 85);
		}
		break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 213);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief force feed stack procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_force_feed_stack(void)
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
		if (ex_main_pause_flag == 0)
		{
			ex_main_reset_flag = 1;
		}
		else
		{
			_main_set_init();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif		
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
#endif
	case TMSG_FEED_FORCE_STACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_main_pause_flag = 0;
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
			#if defined(UBA_RTQ)		/* '19-12-23 */
				if((((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0))
				|| ((((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) || ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0)) && (ex_rc_status.sst1A.bit.quad))
				|| (RS_POS1_ON && is_rc_rs_unit()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					// 通常の収納動作と同じ
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_BACK_REQ, 0, 0, 0, 0);
				}
			#else
				// 通常の収納動作と同じ
				_main_set_mode(MODE1_INIT, INIT_MODE2_FORCE_STACK);
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0); /* 押し込み頂点へ失敗の場合Rejectメッセージを受信する*/
			#endif
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_PAUSE)
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				ex_main_pause_flag = 1;
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_PAUSE, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_RESUME)
		{
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_RESUME, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_main_pause_flag = 0;
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			ex_main_pause_flag = 0;
			/* system error ? */
			_main_system_error(0, 214);
		}
		break;
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 215);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief apb init procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_reject_apb_home(void)	/* '15-03-30  */
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
	#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif		
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
#endif
	case TMSG_APB_EXEC_RSP:
		break;
	case TMSG_APB_HOME_RSP:
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
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else
			{
				_main_set_mode(MODE1_REJECT, REJECT_MODE2_SENSOR_ACTIVE);
				_main_set_sensor_active(1);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0,216);
		}
		break;
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0,217);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief feed reject procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_feed_reject(void)
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
#endif
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	case TMSG_FEED_REJECT_RSP:
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
			//	_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_INTERIM, 0, 0, 0);
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_INIT_REJECT_HANGING, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_NOTE_STAY);
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/* このにも処理が必要なような*//* 2020-01-14a */
			ex_multi_job.busy |= TASK_ST_CENTERING;
			_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 218);
		}
		break;
	case TMSG_APB_EXEC_RSP:
		break;

	/* 2020-01-14a */
	case	TMSG_CENTERING_HOME_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_CENTERING);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					_main_set_mode(MODE1_INIT, INIT_MODE2_FEED_REJECT);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_INITIAL, 0, 1, 0);	//2023-11-01
				}
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_START)
			{

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
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 15);
			}
			break;
#if defined(UBA_RTQ)//#if defined(UBA_RS)
	case TMSG_FEED_RS_FORCE_PAYOUT_RSP:
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
				// 返却処理が完了したので、紙幣取り除き待ち処理へ
				_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_INTERIM, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_NOTE_STAY);
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/* このにも処理が必要なような*/ /* 2020-01-14a */
			ex_multi_job.busy |= TASK_ST_CENTERING;
			_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 219);
		}
		break;
#endif // UBA_RS
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 219);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_note_stay(void)
{
	u16 bill_in;
	
	switch(ex_main_msg.tmsg_code)
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_SENSOR_STATUS_INFO:
		bill_in = _main_bill_remain();
		if (bill_in == BILL_IN_ACCEPTOR)
		{
			_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REMAIN_REQ);
			_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_INIT_REJECT_REQUEST, ex_main_msg.arg2, 0, 0);
		}
		else if (bill_in == BILL_IN_STACKER)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
		}
		else if (bill_in == BILL_IN_NON)
		{
		//	_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_SUCCESS, 0, 0, 0);
			_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_INIT_REJECT_REMOVE, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_RESET_REQ);
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
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
#endif
	case TMSG_APB_EXEC_RSP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 220);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_wait_reset_req(void)
{
	switch(ex_main_msg.tmsg_code)
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
#endif
	case TMSG_APB_EXEC_RSP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 221);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief initilize select mode (init sub function)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_sub_select_mode(void) /* ポジション関係が全てHome位置に動作完了 (PB間に紙幣がある場合以外) */
{
	u16 bill_remain;
#if defined(UBA_RTQ)	/* '19-04-12 */
	u8 sts;
	u8 unit;
#endif
	
	bill_remain = _main_bill_remain();

	if (bill_remain == BILL_IN_NON)
	{
	//残留紙幣がない場合
		if (is_ld_mode())
		{
		// 搬送のイニシャル動作へ
			_main_set_mode(MODE1_INIT, INIT_MODE2_FEED);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_INITIAL_REQ, 0, 0, 0, 0);
		}
		else
		{
			// 搬送を少し動かしで窓以外を検知させる
			_feed_bill_over_window_rev();

			bill_remain = _main_bill_remain();

			if (bill_remain == BILL_IN_NON)
			{
			#if !defined(UBA_RTQ)
				/* 戻し動作を行ったことにより、紙幣がBOXの押しメカに噛み込む位置にある為、正転させる*/
				_feed_bill_over_window_fwd();

				bill_remain = _main_bill_remain();

				if (bill_remain == BILL_IN_NON)
				{
				// 紙幣なし
					_main_set_mode(MODE1_INIT, INIT_MODE2_STACKER);
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);/* 押し込み頂点へ失敗の場合Rejectメッセージを受信する*/
				}
				else
				{
				// 紙幣有り、次のシーケンスでLINEからの、返却 or 収納命令を待つ
					_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REMAIN_REQ);
					_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_REMAIN, bill_remain, 0, 0);
				}
			#else
				_feed_bill_over_window_fwd();
				bill_remain = _main_bill_remain();

				if (bill_remain == BILL_IN_NON)
				{
					// 紙幣なし
					/* 戻し動作を行ったことにより、紙幣がBOXの押しメカに噛み込む位置にある為、正転させる */
					_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_LAST_FEED);
					_main_send_msg(ID_RC_MBX, TMSG_RC_LAST_FEED_CASHBOX_REQ, 0, 0, 0, 0);
				}
				else
				{
					// 紙幣有り、次のシーケンスでLINEからの、返却 or 収納命令を待つ
					_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REMAIN_REQ);
					_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_REMAIN, bill_remain, 0, 0);
				}
			#endif
			}
			else
			{

			// 紙幣有り、次のシーケンスでLINEからの、返却 or 収納命令を待つ
				_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REMAIN_REQ);
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_REMAIN, bill_remain, 0, 0);
			}
		}
	}
	#if defined(UBA_RTQ)
	else if (bill_remain == BILL_IN_ENTRANCE)
	{
//        _main_set_mode(MODE1_INIT, INIT_MODE2_NOTE_STAY);
//        _main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
		//残留紙幣がある場合、次のシーケンスでLINEからの、返却 or 収納命令を待つ
		_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REMAIN_REQ);
        get_rc_recovery_status(&unit, &sts, 0);
		_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_REMAIN, bill_remain, unit, sts);
	}
	#endif
	else
	{
		//残留紙幣がある場合、次のシーケンスでLINEからの、返却 or 収納命令を待つ
		_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REMAIN_REQ);
	#if !defined(UBA_RTQ)
		_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_REMAIN, bill_remain, 0, 0);
	#else
        get_rc_recovery_status(&unit, &sts, 0);
		_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_REMAIN, bill_remain, unit, sts);
	#endif
	}

}

/*********************************************************************//**
 * @brief force stack procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
// 残留紙幣がある為、強制収納
void init_force_stack(void)
{

	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ://TMSG_LINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			ex_main_reset_flag = 1;
			break;
	case	TMSG_SENSOR_STATUS_INFO:
	case	TMSG_SENSOR_ACTIVE_RSP:
			break;
	case	TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
#endif
			break;
	case	TMSG_STACKER_EXEC_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					//_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0); //2025-02-08 注意
					_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_END_INIT_STACK, 0, 0, 0);
					_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_RESET_REQ);
				}
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_REJECT)// ここはリジェクト受けではないか
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				// スタッカHomeへ
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
			} 
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM) // BOX OPENがあるので必要
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_INTERIM)
			{
				/* system error ? */
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{
				//
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 213);
			}
			break;

	case	TMSG_STACKER_EXEC_NG_PULL_RSP:	//Setp1  1度目の押し込みでNG、押しメカ戻し動作
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// 戻し動作は成功したので、モードはこのままで押し込みリトライ
				//	ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, 0, 0, 0, 0);	// リトライ用の押し込み命令へ
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
//				_main_system_error(0, 229);
			}
			break;


	case	TMSG_STACKER_EXEC_RE_RSP:	//Setp2  1度目の押し込みでNG、押しメカ押し込み and 戻し動作
			ex_multi_job.busy &= ~(TASK_ST_STACKER);

			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				//_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);  //2025-02-08 注意
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_END_INIT_STACK, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_RESET_REQ);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else
			{
				/* system error ? */
//				_main_system_error(0, 230);
			}
			break;


	case	TMSG_FEED_INITIAL_RSP:
            break;
#if defined(UBA_RTQ)		/* '19-03-07 */
	case	TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_RC_STATUS_INFO:
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
				}
			}
			break;
#endif
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
				  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
			{
				/* system error ? */
				_main_system_error(0, 201);
			}
			break;
	}
}


void init_shutter(void)
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
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	case TMSG_SHUTTER_INITIAL_OPEN_RSP:
		ex_multi_job.busy &= ~(TASK_ST_SHUTTER);

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
			//UBA500の場合は次に幅よせ,、PB->幅よせ　 iVIZIONの場合はPBなしの幅よせEnd
			// 幅よせあり、幅よせイニシャルへ
			#if defined(UBA_RTQ)
				if(ex_rc_error_flag)
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
				}
				else if (!(is_detect_rc_twin()) || 
					!(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					ex_multi_job.busy |= TASK_ST_CENTERING;
					_main_set_mode(MODE1_INIT, INIT_MODE2_CENTERING);
					_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 0, 0, 0, 0);
				}
			#else
				ex_multi_job.busy |= TASK_ST_CENTERING;
				_main_set_mode(MODE1_INIT, INIT_MODE2_CENTERING);
				_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 0, 0, 0, 0);
			#endif // UBA_RTQ	
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 208);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
//				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
#endif // UBA_RTQ
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}

u8 initial_position_uba(void)
{

	if(!(SENSOR_SHUTTER_OPEN))
	{
		ex_multi_job.busy |= TASK_ST_SHUTTER;
		_main_set_mode(MODE1_INIT, INIT_MODE2_INITIAL_POSITION);
		_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
	}
	else if (!(SENSOR_CENTERING_HOME))
	{
	// 幅寄せがHomeにない場合、Homeに戻す
		ex_multi_job.busy |= TASK_ST_CENTERING;
		_main_set_mode(MODE1_INIT, INIT_MODE2_INITIAL_POSITION);
		_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
	}
	else if( !(SENSOR_APB_HOME) && !(SENSOR_APB_IN) && !(SENSOR_APB_OUT) ) 
	{
	// PBがHomeにない, PB間に紙幣がないと判断しPBを回す
	// PB間に紙幣がないと判断しPBを回す
		ex_multi_job.busy |= TASK_ST_APB;
		_main_set_mode(MODE1_INIT, INIT_MODE2_INITIAL_POSITION);
		_main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);
	}
	else if (!(SENSOR_STACKER_HOME) && (!(is_ld_mode())))
	{
	// 押しメカがHomeにない場合// パワーリカバリを考慮する必要あり// 少し押し込んでから戻す
		ex_multi_job.busy |= TASK_ST_STACKER;
		_main_set_mode(MODE1_INIT, INIT_MODE2_INITIAL_POSITION);
		_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, ex_recovery_info.back_fwd_pulse, 0, 0, 0);
	}
	else
	{
		return(0);
	}

	return(1);	/* 動作あり */

}


/*********************************************************************//**
 * @brief read ICB procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_icb(void)
{
	u8 denomi = 0;

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
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	case TMSG_ICB_INITIAL_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_ICB;

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
		#if defined(UBA_RTQ)
			else if(ex_rc_error_flag)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
		#endif
			else
			{
			#if defined(UBA_RTQ)
				#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER) //202-07-23
				if(Smrtdat_fram_bk_power.mode != 0)
				{
				// 1:通常
					if(Smrtdat_fram_bk_power.mode == RFID_BACK_ACCEPT)
					{
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI, Smrtdat_fram_bk_power.unit, 0, 0); //Collect時-リカバリフラグ有						
					}
				// 2:collect
				// 3:payout
					else if(Smrtdat_fram_bk_power.mode == RFID_BACK_COLECT || Smrtdat_fram_bk_power.mode == RFID_BACK_PAYOUT )
					{
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI_UNIT, Smrtdat_fram_bk_power.unit, 0, 0); //Collect時-リカバリフラグ有
					}
					else
					{
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_BACK, 0, 0, 0);	//Framバックアップをクリア	Smrtdat_fram_bk.unit					
					}
				}
				else
				{
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_BACK, 0, 0, 0);	//Framバックアップをクリア	Smrtdat_fram_bk.unit				
				}

				memset((u8 *)&Smrtdat_fram_bk_power, 0, sizeof(Smrtdat_fram_bk_power));
				_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);

				#else
				_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);
				#endif
			#else
				if (is_icb_enable() && is_icb_recovery_info_stack(&denomi))
				{
					/* Stackコマンド受信～FRAMバックアップ前のリカバリ */
					set_recovery_step(RECOVERY_STEP_ICB_ACCEPT);
					ex_multi_job.busy |= TASK_ST_ICB;
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_REQ, denomi, 0, 0, 0);
				}
				else
				{
					_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);
				}
			#endif	
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{

			ex_multi_job.busy &= ~TASK_ST_ICB;
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
				if(is_icb_enable())
				{
					if (!(is_box_set()))
					{
						_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else
					{
						_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}
				else
				{
					if(ex_main_msg.arg2 == ALARM_CODE_RFID_ICB_SETTING)
					{
						_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
					else
					{
						_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);
					}
				}
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 202);
		}
		break;
	//2023-11-09
	case TMSG_ICB_ACCEPT_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_ICB;

			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
		#if defined(UBA_RTQ)
			else if(ex_rc_error_flag)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
		#endif
			else
			{
				_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_ICB;

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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 202);
		}
		break;
	case TMSG_FEED_INITIAL_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
#endif // UBA_RTQ
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 203);
		}
		break;
	}
}

void init_cis_init(void)
{
	switch (ex_main_msg.tmsg_code)
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
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~TASK_ST_CIS_INIT;

			//#if (_DEBUG_CIS_AS_A_POSITION==1)	//ここから監視処理開始 2024-02-17
				change_ad_sampling_mode(AD_MODE_VALIDATION_CHECK);
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_CIS_ACTIVE_REQ, 0, 0, 0, 0); //これを呼ばないと機能してない
				dly_tsk(10);
			//#endif
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else if(!(initial_position_uba()))//2023-03-24
				{
					
					init_sub_select_mode();
				}
				else
				{
					//INIT_MODE2_INITIAL_POSITIONへ　/* モード継続 */


				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~TASK_ST_CIS_INIT;
				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else
					{
						_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 74);
			}
		}
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM )
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			  && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 201);
		}
		break;
	}
}


#if defined(UBA_RTQ)
void init_rc()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_RC_SET_MOTOR_SPEED_RSP:
		//UBA500はSuccess以外もあるが、実際はSuccess以外はRCタスクから送られてこない
		/* JDL set log*/
		jdl_rc_set_speed();
#if defined(RC_ENCRYPTION)
		if(ex_init_add_enc_flg == 1)
		{
			// 暗号化初期化
			initialize_encryption();
			renewal_cbc_context();

			/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
		}
		else
		{
			/* APB init */
			ex_multi_job.busy |= TASK_ST_APB;
			_main_set_mode(MODE1_INIT, INIT_MODE2_APB);
			_main_send_msg(ID_APB_MBX, TMSG_APB_INITIAL_REQ, 0, 0, 0, 0);
			/* RTQ reset  */
			_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_REQ, INITIAL_NORAML, 0, 0, 0);
		}
#else
		/* APB init */
		ex_multi_job.busy |= TASK_ST_APB;
		_main_set_mode(MODE1_INIT, INIT_MODE2_APB);
		_main_send_msg(ID_APB_MBX, TMSG_APB_INITIAL_REQ, 0, 0, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);//2025-01-18 保護処理 次のシーケンスでこのタイマを使用してRTQ側のイニシャル完了を待つ必要があるので念のため
		/* RTQ reset  */
		_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_REQ, INITIAL_NORAML, 0, 0, 0);
#endif 		
		break;
#if defined(RC_ENCRYPTION)
	case TMSG_RC_DEL_RSP:
		switch (ex_main_msg.arg1)
		{
		case TMSG_RC_ENC_KEY:
			/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
			break;
		case TMSG_RC_ENC_NUM:
			ex_init_add_enc_flg = 0;

			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				ex_multi_job.busy |= TASK_ST_APB;

				_main_set_mode(MODE1_INIT, INIT_MODE2_APB);
				_main_send_msg(ID_RC_MBX, TMSG_RC_RESET_REQ, INITIAL_NORAML, 0, 0, 0);
				_main_send_msg(ID_APB_MBX, TMSG_APB_INITIAL_REQ, 0, 0, 0, 0);
			}
			break;
		}
		break;
#endif // RC_ENCRYPTION
	case TMSG_RC_RESET_RSP:
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
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
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}

void init_rc_wait_recovery_drum_gap_adj()
{
	switch (ex_main_msg.tmsg_code)
	{

		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
		case TMSG_RC_DRUM_GAP_ADJ_RSP:
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			break;
		case TMSG_TIMER_TIMES_UP:
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
			else if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
			{
				if(!(rc_busy_status()))
				{
					ex_multi_job.busy &= ~(TASK_ST_RC);

					if((ex_multi_job.busy & TASK_ST_RC) == 0)
					{
						//TODO: check again
						//_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0); //2025-02-08 注意
						_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_END_INIT_STACK, 0, 0, 0);
						_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_RESET_REQ);
					}
					else
					{
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
				}
			}
			break;
		case TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		case TMSG_RC_STATUS_INFO:
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
		default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}

void init_rc_wait_recovery_back()
{
	switch (ex_main_msg.tmsg_code)
	{

		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
		case TMSG_RC_RETRY_BILL_DIR_RSP:
		case TMSG_RC_BILLBACK_DRUM_PAYOUT_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				/* リセット要求有り */
				if(ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				if(!(is_detect_rc_twin()) || 
					!(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
			else if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
			{
				if(!(rc_busy_status()))
				{
					ex_multi_job.busy &= ~(TASK_ST_RC);

					if((ex_multi_job.busy & (TASK_ST_RC + TASK_ST_FEED)) == 0)
					{
						/* スイッチバック位置への搬送完了 */
						/* 紙無しリカバリ時に出金方向へ駆動させるためリカバリフラグ変更 */
						/* Vend Valid送信有無のため出金と入金でフラグ分ける。（回収では */
						/* 当関数未使用) */
						if( (ex_recovery_info.step >= RECOVERY_STEP_SWITCHBACK_TRANSPORT) ||
						    (ex_recovery_info.step == RECOVERY_STEP_NON)         )
						{
							set_recovery_step(RECOVERY_STEP_SWITCHBACK_TRANSPORT);
						}
						else
						{
						    set_recovery_unit( RC_CASH_BOX, ex_recovery_info.unit );
							set_recovery_step( RECOVERY_STEP_STACK_TRANSPORT );
						}
						/* stacker home */
						if(!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
						{
							ex_multi_job.busy |= TASK_ST_STACKER;
							_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK_HOME);
							_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
						}
						else
						{
							u8 flap1_pos;
							u8 flap2_pos;

							/* flappr1 position check */
							if(!(is_flapper1_head_to_box_pos()))
							{
								flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX;		/* change position	*/
							}
							else
							{
								flap1_pos = 0;								/* don't move		*/
							}
							/* flapper2 position check */
							if(is_quad_model() && !(is_flapper2_head_to_box_pos()))
							{
								flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX;		/* change position	*/
							}
							else
							{
								flap2_pos = 0;								/* don't move		*/
							}

							/* send flapper command *//*22-FEB-24*/
							if(flap1_pos == 0 && flap2_pos == 0)
							{
								if (ex_rc_status.sst1A.bit.error)
								{
									_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
								}
								else
								{
									ex_multi_job.busy |= TASK_ST_RC;

									_main_set_mode( MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK_BOX );
									_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, 0, 0, 0, 0);
								}
							}
							else
							{
								_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_INIT_RC );
								_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
							}
						}
					}
					else
					{
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
				}
			}
			break;
		case TMSG_FEED_RC_FORCE_PAYOUT_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_FEED);
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_FEED);
				/* リセット要求有り */
				if(ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			break;

	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;

		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		case TMSG_RC_STATUS_INFO:
			if (ex_main_msg.arg1 == TMSG_SUB_ALARM && 
				ex_rc_error_flag == 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
		default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
			{
				/* system error ? */
				_main_system_error(0, 209);
			}
		break;
	}
}

void init_rc_wait_recovery_stack_home()
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
		case TMSG_TIMER_TIMES_UP:
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
			break;
		case TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;

		case TMSG_STACKER_HOME_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
					u8 flap1_pos;
					u8 flap2_pos;

					switch (s_recovery_unit)
					{
					case RC_CASH_BOX:
						/* flappr1 position check */
						if (!(is_flapper1_head_to_box_pos()))
						{
							flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
						}
						else
						{
							flap1_pos = 0; /* don't move		*/
						}
						/* flapper2 position check */
						if (is_quad_model() && !(is_flapper2_head_to_box_pos()))
						{
							flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX; /* change position	*/
						}
						else
						{
							flap2_pos = 0; /* don't move		*/
						}
						break;
					case RC_TWIN_DRUM1:
					case RC_TWIN_DRUM2:
						/* flappr1 position check */
						if (!(is_flapper1_twin_to_box_pos()))
						{
							flap1_pos = RC_FLAP1_POS_RC_TO_BOX; /* change position	*/
						}
						else
						{
							flap1_pos = 0; /* don't move		*/
						}
						/* flapper2 position check */
						if (is_quad_model() && !(is_flapper2_head_to_box_pos()))
						{
							flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX; /* change position	*/
						}
						else
						{
							flap2_pos = 0; /* don't move		*/
						}
						break;
					case RC_QUAD_DRUM1:
					case RC_QUAD_DRUM2:
						/* flappr1 position check */
						flap1_pos = 0; /* don't move		*/
						/* flapper2 position check */
						if (is_quad_model() && !(is_flapper2_quad_to_box_pos()))
						{
							flap2_pos = RC_FLAP2_POS_RC_TO_BOX; /* change position	*/
						}
						else
						{
							flap2_pos = 0; /* don't move		*/
						}
						break;
					}

					/* send flapper command */
					if (flap1_pos == 0 && flap2_pos == 0)
					{
						ex_multi_job.busy |= TASK_ST_RC;

						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK_BOX);
						_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, s_recovery_unit, 0, 0, 0);
					}
					else
					{
						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_INIT_RC);
						_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
					}
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 17);
			}
			break;
		default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}

void init_rc_wait_recovery_init_rc()
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
		case TMSG_RC_FLAPPER_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				/* リセット要求有り */
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 90);
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
			else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
			{
				if (!(rc_busy_status()))
				{
					ex_multi_job.busy |= TASK_ST_RC;

					_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK_BOX);
					_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, 0, 0, 0, 0);
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
				}
			}
			break;
		// case TMSG_SENSOR_STATUS_INFO:
		// 	break;
		case TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
		case TMSG_RC_STATUS_INFO:
			if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		case TMSG_RC_RECYCLE_SETTING_RSP:
	#if 1 // #ifdef _ENABLE_JDL
			jdl_rc_set_rc_setting();
	#endif /* _ENABLE_JDL */
			break;
		default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 91);
			}
			break;
	}
}

void init_rc_wait_recovery_back_box()
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
		case TMSG_FEED_RC_FORCE_STACK_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			//step2
				ex_multi_job.busy &= ~(TASK_ST_FEED);
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_FEED);
								/* リセット要求有り */
				if(ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			break;
		case TMSG_RC_FEED_BOX_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			//step1
				ex_multi_job.busy |= TASK_ST_FEED;

				_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, RC_CASH_BOX, 0, 0, 0);

				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
							/* リセット要求有り */
				if(ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			break;
		case TMSG_RC_STATUS_INFO:
			if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
			else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
			{
				if (!(rc_busy_status()))
				{
				//step3
					ex_multi_job.busy &= ~(TASK_ST_RC);

					if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
					{
						ex_multi_job.busy |= TASK_ST_STACKER;
						/* Cash Boxへの搬送完了 */
						set_recovery_unit(RC_CASH_BOX, ex_recovery_info.unit);
						if ((ex_recovery_info.step >= RECOVERY_STEP_COLLECT_DRUM) ||
							(ex_recovery_info.step == RECOVERY_STEP_NON))
						{
							set_recovery_step(RECOVERY_STEP_COLLECT_STACKING);
						}
						else
						{
							set_recovery_step(RECOVERY_STEP_STACKING);
						}
						// #if defined(RC_STACKER_NEW)		/* '21-03-01 */
						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_BACK_REQ, 0, 0, 0, 0);
						// #else
						//						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
						//						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
						// #endif
					}
					else
					{
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			break;
		case TMSG_STACKER_BACK_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			//step4 end
				set_recovery_step(RECOVERY_STEP_NON); /* '24-04-02 */

				// スタッカのイニシャル動作へ
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
				}
			}
			break;
		case TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1//#ifdef _ENABLE_JDL
            jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
			break;
		default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 91);
			}
			break;
	}
}

/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_rc_wait_recovery_force_stack_drum(void)
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
		// case TMSG_SENSOR_STATUS_INFO:
		// 	break;
		case TMSG_RC_FORCE_STACK_DRUM_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			//step1
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				/* リセット要求有り */
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			break;
		case TMSG_FEED_RC_FORCE_STACK_RSP:
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			//
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			//step1
				ex_multi_job.busy &= ~(TASK_ST_FEED);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_FEED);
				/* リセット要求有り */
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
			else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
			{
				if (!(rc_busy_status()))
				{
					ex_multi_job.busy &= ~(TASK_ST_RC);

					if ((ex_multi_job.busy & (TASK_ST_RC + TASK_ST_FEED)) == 0)
					{
					//step2
						if (INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM == ex_main_task_mode2)
						{
							// ドラム払出し→BOXに搬送
							ex_multi_job.busy |= TASK_ST_RC;
							ex_multi_job.busy |= TASK_ST_FEED;

							_main_send_msg(ID_RC_MBX, TMSG_RC_BILLBACK_DRUM_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
							_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
							_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK);
						}
						else
						{
							// ドラム払出し→BOXに搬送
							ex_multi_job.busy |= TASK_ST_RC;
							_main_send_msg(ID_RC_MBX, TMSG_RC_FEEDBOX_DRUM_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
							_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_PAYDRUM_BOX__PAYDRUM_BOX);
						}
					}
					else
					{
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
				}
			}
			break;
		case TMSG_CLINE_RC_INFO_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		case TMSG_RC_STATUS_INFO:
			if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
		case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1 // #ifdef _ENABLE_JDL
			jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
			break;
		default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				/* リセット要求有り */
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 204);
			}
			break;
	}
}

/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void init_rc_wait_recovery_front_back_box__front(void)
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_RETRY_BILL_DIR_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* リセット要求有り */
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		break;
	case TMSG_FEED_RC_FORCE_STACK_RSP:
		ex_multi_job.busy &= ~(TASK_ST_FEED);
		//
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			/* リセット要求有り */
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & (TASK_ST_RC + TASK_ST_FEED)) == 0)
				{
					u8 flap1_pos;
					u8 flap2_pos;

					if ((s_recovery_unit == RC_TWIN_DRUM1) || (s_recovery_unit == RC_TWIN_DRUM2))
					{
						/* flappr1 position check */
						if (!(is_flapper1_head_to_twin_pos()))
						{
							flap1_pos = RC_FLAP1_POS_HEAD_TO_RC; /* change position	*/
						}
						else
						{
							flap1_pos = 0; /* don't move		*/
						}
						/* flapper2 position check */
						flap2_pos = 0; /* don't move		*/
					}
					else
					{
						/* flappr1 position check */
						if (!(is_flapper1_head_to_box_pos()))
						{
							flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
						}
						else
						{
							flap1_pos = 0; /* don't move		*/
						}
						/* flapper2 position check */
						if (is_quad_model() && !(is_flapper2_head_to_quad_pos()))
						{
							flap2_pos = RC_FLAP2_POS_HEAD_TO_RC; /* change position	*/
						}
						else
						{
							flap2_pos = 0; /* don't move		*/
						}
					}

					/* send flapper command */
					if (flap1_pos == 0 && flap2_pos == 0)
					{
						ex_multi_job.busy |= TASK_ST_FEED;
						ex_multi_job.busy |= TASK_ST_RC;

						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK);
						_main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, s_recovery_unit, RC_RETRY_PAYOUT_DIR, 0, 0);
						_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
					}
					else
					{
						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_FRONT_BACK_BOX__INIT_RC1);
						_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
				}
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			}
		}
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1 // #ifdef _ENABLE_JDL
		jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			/* リセット要求有り */
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 204);
		}
		break;
	}
}
#endif

void init_rc_wait_last_feed()
{
	switch (ex_main_msg.tmsg_code)
	{
	#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	#endif

	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if(!(rc_busy_status()))
			{
			//step2
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_BACK_REQ, 0, 0, 0, 0);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RC_LAST_FEED_CASHBOX_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		//step1
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
	case TMSG_STACKER_BACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		//step3
			_main_set_mode(MODE1_INIT, INIT_MODE2_STACKER);
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
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
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}

void init_rc_wait_recovery_bill_back()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & TASK_ST_RC) == 0)
				{
					if (ex_main_task_mode2 == INIT_MODE2_RC_WAIT_RECOVERY_BILL_BACK)
					{
						// 紙幣戻し→ドラム払出し→BOXに搬送
						ex_multi_job.busy |= TASK_ST_RC;
						ex_multi_job.busy |= TASK_ST_FEED;

#if 1 // POLYMOR
						_main_send_msg(ID_RC_MBX, TMSG_RC_FORCE_STACK_DRUM_REQ, s_recovery_unit, 0, 0, 0);
						_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, s_recovery_unit, 0, 0, 0);
						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM);
#else
						_main_send_msg(ID_RC_MBX, TMSG_RC_BILLBACK_DRUM_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
						_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK);
#endif
					}
					else
					{
#if 1 // POLYMOR
	  					//  紙幣戻し→ドラム払出し→BOXに搬送
						ex_multi_job.busy |= TASK_ST_RC;
						ex_multi_job.busy |= TASK_ST_FEED;
						_main_send_msg(ID_RC_MBX, TMSG_RC_FORCE_STACK_DRUM_REQ, s_recovery_unit, 0, 0, 0);
						_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, s_recovery_unit, 0, 0, 0);
						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_FORCE_STACK_DRUM_FOR_COLLECT);
#else
						// ドラム払出し→BOXに搬送
						ex_multi_job.busy |= TASK_ST_RC;
						_main_send_msg(ID_RC_MBX, TMSG_RC_FEEDBOX_DRUM_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_PAYDRUM_BOX__PAYDRUM_BOX);
#endif
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
				}
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			}
		}
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;

	case TMSG_FEED_RC_BILLBACK_RSP:
		ex_multi_job.busy &= ~(TASK_ST_FEED);

		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			ex_multi_job.busy &= ~(TASK_ST_FEED);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}

void init_rc_wait_recovery_stack()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_STACKER_EXEC_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				//_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0); //2025-02-08 注意
				_main_send_connection_task(TMSG_CONN_RESET, TMSG_SUB_END_INIT_STACK, 0, 0, 0);
				_main_set_mode(MODE1_INIT, INIT_MODE2_WAIT_RESET_REQ);
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
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1 // #ifdef _ENABLE_JDL
		jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
		{
			/* system error ? */
			_main_system_error(0, 209);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void init_rc_wait_recovery_box_search_box__box_search(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_RC_SEARCH_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & TASK_ST_RC) == 0)
				{
					// Twinモデル
					if (!is_quad_model())
					{
						if (RC_POSC_ON)
						{
							ex_multi_job.busy |= TASK_ST_RC;

							_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, s_recovery_unit, 0, 0, 0);

							_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BOX_SEARCH_BOX__BOX);
						}
						else
						{
							set_recovery_step(RECOVERY_STEP_STACKING);

							ex_multi_job.busy |= TASK_ST_STACKER;

							// #if defined(RC_STACKER_NEW)		/* '21-03-01 */
							_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_BACK_REQ, 0, 0, 0, 0);
							// #else
							//								_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
							//								_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
							// #endif
						}
					}
					// Quadモデル
					else
					{
						if (RC_POSF_ON)
						{
							ex_multi_job.busy |= TASK_ST_RC;

							_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, s_recovery_unit, 0, 0, 0);

							_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BOX_SEARCH_BOX__BOX);
						}
						else
						{
							set_recovery_step(RECOVERY_STEP_STACKING);

							ex_multi_job.busy |= TASK_ST_STACKER;

							// #if defined(RC_STACKER_NEW)		/* '21-03-01 */
							_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_BACK_REQ, 0, 0, 0, 0);
							// #else
							//								_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
							//								_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
							// #endif
						}
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			}
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_STACKER_BACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			// スタッカのイニシャル動作へ
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
		// #endif
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1 // #ifdef _ENABLE_JDL
		jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 91);
		}
		break;
	}
}
#endif


/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void init_rc_wait_recovery_box_search_box__box(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & TASK_ST_RC) == 0)
				{
					ex_multi_job.busy |= TASK_ST_STACKER;

					set_recovery_step(RECOVERY_STEP_STACKING);

					// #if defined(RC_STACKER_NEW)		/* '21-03-01 */
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_BACK_REQ, 0, 0, 0, 0);
					// #else
					//						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
					//						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
					// #endif
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
				}
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			}
		}
		break;
	case TMSG_FEED_RC_FORCE_STACK_RSP:
		ex_multi_job.busy &= ~(TASK_ST_FEED);

		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
		// #if defined(RC_STACKER_NEW)		/* '21-03-01 */
	case TMSG_STACKER_BACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			// スタッカのイニシャル動作へ
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
		// #endif
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1 // #ifdef _ENABLE_JDL
		jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			/* リセット要求有り */
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 204);
		}
		break;
	}
}
#endif

/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_rc_wait_recovery_paydrum_box__paydrum_box(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_RC_FEEDBOX_DRUM_PAYOUT_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		//step1
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* リセット要求有り */
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & TASK_ST_RC) == 0)
				{
				//step2
					/* Cash Boxへの搬送完了。*/
					set_recovery_step(RECOVERY_STEP_COLLECT_STACKING);

					ex_multi_job.busy |= TASK_ST_STACKER;

					// #if defined(RC_STACKER_NEW)		/* '21-03-01 */
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_BACK_REQ, 0, 0, 0, 0);
					// #else
					//						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
					//						_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
					// #endif
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			}
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_STACKER_BACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		//step3 end
			// スタッカのイニシャル動作へ
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_STACK);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1 // #ifdef _ENABLE_JDL
		jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 91);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief note stay procedure (at init state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void init_rc_wait_recovery_front_back_box__init_rc1(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_RC_FLAPPER_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* リセット要求有り */
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 90);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_RESET, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (!(rc_busy_status()))
			{
				ex_multi_job.busy |= TASK_ST_FEED;
				ex_multi_job.busy |= TASK_ST_RC;

				_main_set_mode(MODE1_INIT, INIT_MODE2_RC_WAIT_RECOVERY_BACK);
				_main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, s_recovery_unit, RC_RETRY_PAYOUT_DIR, 0, 0);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, s_recovery_unit, 0, 0, 0);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
			}
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_CLINE_RC_INFO_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_RESET, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_RECYCLE_SETTING_RSP:
#if 1 // #ifdef _ENABLE_JDL
		jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			if (ex_main_reset_flag)
			{
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 91);
		}
		break;
	}
}
#endif


#endif 

/* EOF */

