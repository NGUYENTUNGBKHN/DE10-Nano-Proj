/******************************************************************************/
/*! @addtogroup Main
    @file       mode_stack.c
    @brief      stack mode of main task
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
#include "status_tbl.h"

#include "systemdef.h"					//2022-02-17 test
#include "cyclonev_sysmgr_reg_def.h"	//2022-02-17 test
#include "hal_gpio_reg.h"				//2022-02-17 test
#if defined(UBA_RTQ)
#include "if_rc.h"
#include "feed_rc_unit.h"
#endif // UBA_RTQ

#if defined(_PROTOCOL_ENABLE_ID003)
	#include "task/cline_task/003/id003.h"
#endif

#define EXT
#include "com_ram.c"

#include "jdl_conf.h"
#include "cyc.h"

#if defined(UBA_RTQ)
//static void stack_rc_operation();
static void stack_feed_rc_stack_sub();
static void stack_rc_feed_box_sub();

static void stack_rc_retry_operation();
static u8 flap1_pos;
static u8 flap2_pos;
void mode_stack_home_finish_rtq(void); //2025-04-01
#endif  // UBA_RTQ

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

void finish_stacker_pb_icb(void);
#if defined(UBA_RTQ)
u8	get_PBcount(void);
#endif

/************************** EXTERNAL VARIABLES *************************/
#if !defined(UBA_RTQ)
void feed_stack_low(void) //only SS, not use RTQ /*単体動作*////最終搬送完了待ち(すでに前のモードで搬送は起動している) 搬送完了後この中でPB動作開始、完了後、入口レバーOpen
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
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_FEED_APB_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		// 収納位置まで紙幣搬送完了
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_main_pause_flag = 0;

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			//2023-05-23
			else if (is_ld_mode())
			{
				/* next step */
				// LDの時はシャッターのみ動かす
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
				_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_SHUTTER_OPEN_LD_MODE);
			}
			else
			{
			// next step
				ex_multi_job.busy |= TASK_ST_APB;
				_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
			}
		}
		else if(ex_main_msg.arg1 == TMSG_SUB_EXIT_OUT)	// testmessage 最終的には削除する可能性が高い
		{
		// 紙幣がEXITを抜けた
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_PAUSE)
		{
		// ポーズ
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				ex_main_pause_flag = 1;
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_PAUSE, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_RESUME)
		{
		// ポーズ解除
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_RESUME, 0, 0, 0);

			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				// とりあえず PBはOPENのはず
				_main_reject_sub(MODE1_STACK, STACK_MODE2_WAIT_REJECT_REQ, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			ex_main_pause_flag = 0;
			/* system error ? */
			_main_system_error(0, 73);
		}
		break;

	case	TMSG_APB_CLOSE_RSP: // MUL 主の処理2 close
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);
				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
					// とりあえずPBエラーとする
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					else
					{ /* other job normal end */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}else{
					ex_multi_job.alarm |= TASK_ST_APB;
					ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{

			}
			break;

	case	TMSG_SHUTTER_OPEN_RSP:	// MUL 主の処理3
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_SHUTTER);
				_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_STACK_TOP);	// 単体動作、PB、レバー動作完了 スタッカー起動 押しメカTOP待ちへ
				ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更
				set_recovery_step(RECOVERY_STEP_STACKING);
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, SS_BILL_STACK, 0, 0, 0);/* 押し込み頂点へ失敗の場合Rejectメッセージを受信する*/
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_SHUTTER);
				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
					// とりあえずPBエラーとする
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					else
					{ /* other job normal end */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}else{
					ex_multi_job.alarm |= TASK_ST_SHUTTER;
					ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;
				}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 176);
		}
		break;

	}
}
#endif



/*********************************************************************//**
 * @brief stack message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void stack_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	//only SS, not use RTQ
	#if !defined(UBA_RTQ)
	case	STACK_MODE2_FEED_STACK:			//並列動作専用 only SS, not use RTQ
			feed_stack();					//only SS, not use RTQ //最終搬送完了待ち(すでに前のモードで搬送は起動している) ok
			break;
	#endif
			/*--------------------------------------------------------------*/
	/* use SS, RTQ */
	case	STACK_MODE2_WAIT_STACK_START:	//並列動作専用 use SS, RTQ
			wait_stack_start();				//use SS, RTQ // 押し込み開始待ち 追加した ok,
			break;
	case	STACK_MODE2_WAIT_STACK_TOP:		//単体動作と並列動作がこのシーケンスで合流する use SS RTQ
			stack_wait_stack_top();			//use SS RTQ // ここでは必ず、収納頂点待ちとPB終了,shutterを待つ ここがcheatの確認の最終 ok 
			break;
	case	STACK_MODE2_STACK_HOME:
			stack_wait_stack_home();		//use SS RTQ //単体動作と並列動作　さらにこの中で連続と分岐　// 収納戻り中(半戻し中の場合ありえる)、規定パルス以上戻したら、紙幣取り込み処理を許可する ok
			break;
	#if 0 //2023-08-10
	case	STACK_MODE2_STACK_HOME_WAIT_NOTE:	//連続保留中、押し込み前は入口ON,戻し開始時は入口OFF　連続がキャンセルされる可能性あり//連続動作用,入口センサ監視中
			stack_wait_stack_home_wait_note();	//連続保留中、押し込み前は入口ON,戻し開始時は入口OFF　連続がキャンセルされる可能性あり　//連続動作用,入口センサ監視中 ok
			break;
	#endif

	/* 並列 */
	case	STACK_MODE2_WAIT_REQ_FIRST:		//連続動作用, LINEからの許可待ち ok
			stack_wait_req_first();			//use SS RTQ //連続動作用, LINEからの許可待ち、ここからmode_acceptへ遷移する
			break;

	/* 収納リトライ */
	case	STACK_MODE2_STACK_RETRY:		//押し込みリトライ動作 ok
			stack_exec_retry();				//use SS RTQ
			break;

	/* 通常 */
	case	STACK_MODE2_WAIT_REQ:			//連続動作でない待ち処理 ok //use SS,RTQ
			stack_wait_req();				//use SS RTQ
			break;
#if !defined(UBA_RTQ)
			/* LD modeの搬送完了後のShutter open、テストモードのBoxなし */
	case	STACK_MODE2_WAIT_SHUTTER_OPEN_LD_MODE:
			stack_wait_shutter_open_ld_mode();		//only SS, not use RTQ // ok
			break;
	//#if defined(HIGH_SECURITY_MODE)
	case	STACK_MODE2_FEED_REV_REQ:		//連続動作でない BOX搬送路に紙幣が残っているかの確認 ok
			stack_feed_rev_req();			//only SS, not use RTQ	/* High security */
			break;
	case	STACK_MODE2_FEED_STACK_LOW: /* 単体動作*/
			feed_stack_low();	//only SS, not use RTQ //最終搬送完了待ち(すでに前のモードで搬送は起動している) 搬送完了後この中でPB動作開始、完了後、入口レバーOpen動作 ok
			break;
#endif

	/* 返却 */
	case	STACK_MODE2_WAIT_REJECT_REQ:	// 押し込み位置まで、紙幣を搬送できななった場合の処理、紙幣を返却させる PBは回していない
			stack_wait_reject_req();		//use SS RTQ // ok
			break;

			 /* shutter open待ち */
	case	STACK_MODE2_WAIT_REJECT_SHUTTER_OPEN:	// 押し込み位置まで、紙幣を搬送できななった場合の処理、紙幣を返却させる PBは回していない
			stack_wait_reject_shutter_open();		//use SS RTQ // ok 返却前の最終
			break;

#if (DATA_COLLECTION_DEBUG==1)
	case   STACK_MODE2_FEED_DATA_COLLECTION:
			stack_feed_data_collection();
			break;
#endif

#if defined(UBA_RTQ)
	case STACK_MODE2_FEED_RC_STACK: 	//正規の処理の最初、リサイクル庫の場合このまま処理を抜ける、回収庫の場合、通常のSSの収納動作に合流
			stack_feed_rc_stack();		//NGの場合がおそらく2種類、リサイクラへ搬送予定だった場合、STACK_MODE2_RC_RETRY_REV へ
			break;						//回収庫へ搬送予定だった場合、STACK_MODE2_RC_RETRY_FEED_BOX_REV へ

	case STACK_MODE2_RC_FEED_BOX:		//正規処理 BOXへの搬送
			stack_rc_feed_box();
			break;


/*-----------------------------------------------------------------*/
	/* 下記はリカバリ処理で基本的に回収庫へ紙幣を搬送させる */
	/* リサイクル庫への搬送失敗で、回収庫搬送前に、リサイクル庫から紙幣を出す処理 */
	/* ループ1 start */
	/*step1*/
	case STACK_MODE2_RC_RETRY_REV: //STACK_MODE2_FEED_RC_STACK か STACK_MODE2_RC_PREFEED_STACK から遷移する. 上手くいった場合 ここから抜ける
			stack_rc_retry_rev();
			break;
	/* STACK_MODE2_RC_RETRY_REV のリトライ処理 */
	case STACK_MODE2_RC_PREFEED_STACK: //STACK_MODE2_RC_RETRY_REV が上手く行かない場合、ここに来る、ここでとりあえず動かしてまた、STACK_MODE2_RC_RETRY_REV へ戻す
			stack_rc_retry_prefeed_stack();
			break;
	/* ループ1 end */

	/* step1-2*/
	/*回収庫搬送前の保護処理*/
	case STACK_MODE2_RC_RETRY_STACK_HOME://保護処理 回収庫へ搬送する為、スタッカをHomeへ、完了後、フラッパを動かす場合は STACK_MODE2_RC_RETRY_INIT_RC それ以外は主の搬送開始で STACK_MODE2_RC_RETRY_FEED_BOX
			stack_rc_retry_stack_home();
			break;
	case STACK_MODE2_RC_RETRY_INIT_RC: //保護処理 回収庫へ搬送する為のフラッパ動作、完了後は搬送開始で STACK_MODE2_RC_RETRY_FEED_BOX へ
			stack_rc_retry_init_rc();
			break;

	/* step2*/
	/*回収庫搬送処理 */
	/* ループ2 start */
	case STACK_MODE2_RC_RETRY_FEED_BOX:	//成功なら収納開始の STACK_MODE2_WAIT_STACK_START へ、失敗なら STACK_MODE2_RC_RETRY_FEED_BOX_REV 上手くいった場合 ここから抜ける(ここが、リカバリ全体の出口)
			stack_rc_retry_feed_box();
			break;
	/* STACK_MODE2_RC_RETRY_FEED_BOX のリトライ処理*/
	case STACK_MODE2_RC_RETRY_FEED_BOX_REV:	//必ず STACK_MODE2_RC_RETRY_FEED_BOX に遷移
			stack_rc_retry_feed_box_rev();
			break;
	/* ループ2 end */
/*-----------------------------------------------------------------*/

#endif

	default:
		/* system error ? */
		_main_system_error(0, 170);
		break;
	}
}



/*********************************************************************//**
 * @brief feed stack procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if !defined(UBA_RTQ)
void feed_stack(void)	//only SS, not use RTQ //並列動作専用 //並列動作専用 
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
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_FEED_APB_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
		// 収納位置まで紙幣搬送完了
			ex_main_pause_flag = 0;

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			if (is_ld_mode())
			{
				/* next step */				//2022-01-21
				//shutterが閉じたままなので、開く
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
				_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_SHUTTER_OPEN_LD_MODE);
			}
			else
			{
				// next step
				set_recovery_step(RECOVERY_STEP_STACKING);
				// 押し込み動作へ

				_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_STACK_START);
				ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更
				//#if defined(LOW_POWER_3)
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, SS_BILL_STACK, 0, 0, 0);/* 押し込み頂点へ失敗の場合Rejectメッセージを受信する*/
			}
		}
		else if(ex_main_msg.arg1 == TMSG_SUB_EXIT_OUT)	// testmessage 最終的には削除する可能性が高い
		{
		// 紙幣がEXITを抜けた
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_PAUSE)
		{
		// ポーズ
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				ex_main_pause_flag = 1;
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_PAUSE, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_RESUME)
		{
		// ポーズ解除
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_RESUME, 0, 0, 0);

			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				// とりあえず PBはOPENのはず
				_main_reject_sub(MODE1_STACK, STACK_MODE2_WAIT_REJECT_REQ, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			ex_main_pause_flag = 0;
			/* system error ? */
			_main_system_error(0, 73);
		}
		break;

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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 176);
		}
		break;
	}
}
#endif


/*********************************************************************//**
 * @brief wait request (stack state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void stack_wait_req(void) //use SS RTQ
{
	switch(ex_main_msg.tmsg_code)
	{
#if 1 //2023-11-20
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
#endif

	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
	#if 1 //2024-05-28
		if(ex_is_cis_high==1)
		{
			ex_is_cis_high = 0;
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_CIS_TEMPERATURE, _main_conv_seq(), ex_position_sensor);
			break;
		}
	#endif
		_main_set_disable();
		_main_display_denomination();
		break;

	case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
	#if 1 //2024-05-28
		if(ex_is_cis_high==1)
		{
			ex_is_cis_high = 0;
			_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_CIS_TEMPERATURE, _main_conv_seq(), ex_position_sensor);
			break;
		}
	#endif
		if (!is_ld_mode())
		{
		#if defined(UBA_RTQ)
			if(ex_main_test_no == TEST_RC_AGING 
				|| ex_main_test_no == TEST_RC_AGING_FACTORY)
			{
				if (ex_rc_configuration.unit_type_bk == RS_CONNECT)
				{
					ex_multi_job.busy |= TASK_ST_RC;
					_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, RS_FLAP_POS_OUT, 0, 0, 0);
				}
				else
				{
				#if defined(QA_TEST_EMC_EMI)
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0); //5s UBA500RTQは10s
				#else
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 1500, 0, 0);
				#endif
				}
			}
			else
			{
				_main_set_enable();
			}
		#else
			_main_set_enable();
		#endif  // UBA_RTQ
		}
		else
		{
			//TODO:
			_main_set_enable();
		}
		_main_display_denomination();
		break;

	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			_main_set_enable();
		}
	#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	//#if defined(UBA_RS)
	case TMSG_RS_FLAPPER_RSP:
		ex_multi_job.busy &= ~(TASK_ST_RC);

		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* None */
		#if defined(QA_TEST_EMC_EMI)
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0); //5s UBA500RTQは10s
		#else
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 1500, 0, 0);
		#endif
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 177);
		}
		break;
	//#endif // UBA_RS

	case TMSG_RC_STATUS_INFO:	//ok
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;

#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 177);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief wait reject request procedure (at stack state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void stack_wait_reject_req(void) //use SS RTQ
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:

	#if defined(DATA_COLLECTION)
		_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
		_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
	#else  /* !DATA_COLLECTION */
	
		#if defined(UBA_RTQ)
		if(ex_rc_error_flag)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			ex_multi_job.busy |= TASK_ST_SHUTTER;
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REJECT_SHUTTER_OPEN);	
		}
		#else
			ex_multi_job.busy |= TASK_ST_SHUTTER;
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REJECT_SHUTTER_OPEN);	
		#endif
	#endif /* !DATA_COLLECTION */
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
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 179);
		}
		break;
	}
}


void stack_wait_reject_shutter_open(void) //use SS RTQ //マルチ処理は存在しない、Lineからの返却許可も得ている、返却前の最終
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		break;
	case TMSG_SHUTTER_OPEN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{ /* other job normal end */
				_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_AFTER_ESCROW, 0, 0, 0);
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

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{ /* other job normal end */
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
		}
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
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 179);
		}
		break;
	}
}

#if !defined(UBA_RTQ)
void stack_wait_shutter_open_ld_mode(void) //only SS, not use RTQ /* LD modeの搬送完了後のShutter open、テストモードのBoxなし *//* マルチ動作は現状存在しない*/
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:
		break;
	case TMSG_SHUTTER_OPEN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{ /* other job normal end */
					_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
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
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			}
		}
		break;

	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 179);
		}
		break;
	}
}
#endif


// 押し込みモータ起動待ちその後shutter起動
void wait_stack_start(void)	//use SS, RTQ 並列動作専用 
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			ex_main_reset_flag = 1;
			break;
	case	TMSG_SENSOR_STATUS_INFO:
			break;
	case	TMSG_TIMER_TIMES_UP:
		#if defined(UBA_RTQ)	/* '19-03-18 */
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#endif
			break;
	case	TMSG_STACKER_EXEC_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_START)
			{
				//2023-10-19
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
				_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_STACK_TOP);	// 並列動作、スタッカ、レバー起動 押しメカTOP待ちへ
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			// 通常はあり得ない

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
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 75);
			}
			break;
#if defined(UBA_RTQ)	/* '18-05-01 */
	case	TMSG_RC_STATUS_INFO: //ok
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 76);
			}
			break;
	}
}

/*********************************************************************//**
 * @brief stack exec procedure 押しメカ頂点待ち、シャッタ動作完了後、PB回転
 * @param[in]	None
 * @return 		None
 **********************************************************************/
/* 今までは、収納と、PBだったが、シャッターが追加されたので、リトライ条件を変える必要がある */
/* 基本的に、収納はRejectメッセージを受信する可能性があるが、PBとシャッターはSuccessがエラーのみ */
/* PB,シャッターエラーの場合、スタッカー戻さないでそのままエラー */
void stack_wait_stack_top(void)//use SS RTQ //単体動作と並列動作がこのシーケンスで合流する// 押し込みの頂点時にすでにリトライが確定している場合は、TMSG_SUB_REJECTメッセージを受信するようにする
{

	u16 bill_in; 

	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			ex_main_reset_flag = 1;
			break;
	case	TMSG_SENSOR_STATUS_INFO:
			break;
	case	TMSG_TIMER_TIMES_UP:
		#if defined(UBA_RTQ)
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#endif
			break;
	case	TMSG_STACKER_EXEC_RSP:	// MUL 主の処理1
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS) // 収納のTop位置に到達した(FULLでもない)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

			#if 1//#ifdef _ENABLE_JDL
				jdl_accept(ex_validation.denomi);
			#endif /* _ENABLE_JDL */

				if(is_icb_enable())
				{
					ex_multi_job.busy |= TASK_ST_ICB; //2024-02-16
				#if defined(UBA_RTQ_ICB)
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI, ex_validation.denomi, 0, 0);	//通常			
				#else
					set_recovery_step(RECOVERY_STEP_ICB_ACCEPT);
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_REQ, ex_validation.denomi, 0, 0, 0);
				#endif
					break;
				}
				else
				{
					if (!(ex_multi_job.busy))
					{ /* all job end */
					// SUBの押しメカがエラーが発生した可能性があるので確認する
						// next step 2024-02-16
						finish_stacker_pb_icb();
					}// end all job
				}
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
			// 押し込みの頂点時にすでにリトライが確定している場合は、Retryメッセージを受信するようにする
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				if (!(ex_multi_job.busy))
				{ /* all job end */
				// SUBの押しメカがエラーが発生した可能性があるので確認する
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
					// とりあえずPBエラーとする
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					else if (!(is_box_set()))
					{ /* box unset */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else
					{
					// next step
						if (ex_main_reset_flag)
						{ /* リセット要求有り */
							_main_set_init();
						}
						else if (!(is_box_set()))
						{
							_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
						}
						else
						{
						// 押し込みリトライ処理へ
							ex_multi_job.busy |= TASK_ST_STACKER;
							// スタッカHomeへ
							_main_set_mode(MODE1_STACK, STACK_MODE2_STACK_RETRY);
							_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
						}
					}
				}
				else
				{
					ex_multi_job.reject |= TASK_ST_STACKER;
					ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
					// とりあえずPBエラーとする
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					else
					{ /* other job normal end */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}else{
					ex_multi_job.alarm |= TASK_ST_STACKER;
					ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{
				//
			}
			else
			{
			/* system error ? */
				_main_system_error(0, 200);
			}
			break;
	case	TMSG_APB_EXEC_RSP:	// MUL 主の処理2 Home
	case	TMSG_APB_CLOSE_RSP: // MUL 主の処理2 close
			if (ex_main_msg.arg1 == TMSG_SUB_START)
			{

			}
			else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);
				if (!(ex_multi_job.busy))
				{ /* all job end */
					finish_stacker_pb_icb();
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{

				ex_multi_job.busy &= ~(TASK_ST_APB);
				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					else
					{ /* other job normal end */
					// どちらを優先するか不明
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}
				else if ((ex_main_reset_flag) && (ex_main_pause_flag != 0))
				{
					ex_main_pause_flag = 0;
					_main_set_init();
				}
				else
				{
					ex_multi_job.alarm |= TASK_ST_APB;
					ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
				}
			}
			else
			{
				ex_main_pause_flag = 0;
				/* system error ? */
				_main_system_error(0, 71);
			}
			break;


	case	TMSG_SHUTTER_OPEN_RSP:	// MUL 主の処理3 レバー動作完了後、PB動作開始
			if (ex_main_msg.arg1 == TMSG_SUB_START)
			{

			}
			else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_SHUTTER);

				//2023-10-19 レバー動作完了したので、PB動作開始
				// PB起動
				bill_in = _main_bill_in();
				ex_multi_job.busy |= TASK_ST_APB;

			#if defined(UBA_RTQ)
				if( bill_in == BILL_IN_ENTRANCE )
				{
					ex_2nd_note_uba = 1;	/* 連続取り込み有効*/
				}
				else
				{
					ex_2nd_note_uba = 0;	/* 連続取り込み無効*/
				}

				if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)  //2024-09-30
				{
					ex_2nd_note_uba = 0;	/* 連続取り込み無効*/
					/* 回転してclose */
					_main_send_msg(ID_APB_MBX, TMSG_APB_EXEC_REQ, get_PBcount(), 0, 0, 0);
				}
				else
				{
					/* 回転してHome */
					_main_send_msg(ID_APB_MBX, TMSG_APB_EXEC_REQ, 1, 0, 0, 0);
				}
			#else
				if( bill_in == BILL_IN_ENTRANCE )
				{
					ex_2nd_note_uba = 1;	/* 連続取り込み有効*/
				}
				else
				{
					ex_2nd_note_uba = 0;	/* 連続取り込み無効*/
				}
				/* 回転してHome */
				_main_send_msg(ID_APB_MBX, TMSG_APB_EXEC_REQ, 1, 0, 0, 0);
			#endif
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{

				ex_multi_job.busy &= ~(TASK_ST_SHUTTER);
				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					else
					{ /* other job normal end */
					// どちらを優先するか不明
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}
				else if ((ex_main_reset_flag) && (ex_main_pause_flag != 0))
				{
					ex_main_pause_flag = 0;
					_main_set_init();
				}
				else
				{
					ex_multi_job.alarm |= TASK_ST_SHUTTER;
					ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;
				}
			}
			else
			{
				ex_main_pause_flag = 0;
				/* system error ? */
				_main_system_error(0, 71);
			}
			break;

	#if 1 //2024-02-16

	case 	TMSG_ICB_ACCEPT_RSP:
#if defined(UBA_RTQ_ICB)
	case 	TMSG_ICB_ACCEPT_RTQ_RSP:
#endif
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{

				ex_multi_job.busy &= ~TASK_ST_ICB;	
				if (!(ex_multi_job.busy))
				{ /* all job end */
					finish_stacker_pb_icb();
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_ICB);

				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					#if 0 //ICBエラーは優先なので
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
					//
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					#endif
					else
					{ /* other job normal end */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}
				else
				{
					ex_multi_job.alarm |= TASK_ST_ICB;
					ex_multi_job.code[MULTI_ICB] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_ICB] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_ICB] = ex_main_msg.arg4;
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 173);
			}
			break;
	#endif

#if defined(UBA_RTQ)		/* '18-05-01 */
	case	TMSG_RC_STATUS_INFO:	//ok
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 236);
			}
			break;
	}
}

/*********************************************************************//**
 * @brief stack exec procedure 押しメカHome戻し中(半戻しの場合もある)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void stack_wait_stack_home(void) //use SS RTQ //単体動作と並列動作　この中で連続と分岐　//押しメカHome戻し中(半戻しの場合もある)
{

    u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			ex_main_reset_flag = 1;
			break;
	case	TMSG_SENSOR_STATUS_INFO:

			break;
	case	TMSG_TIMER_TIMES_UP:
		#if defined(UBA_RTQ)
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#endif
			break;
	case	TMSG_STACKER_PULL_RSP:	// MUL 主の処理1
			if (ex_main_msg.arg1 == TMSG_SUB_ENABLE_NEXT) // 次の紙幣取り込み許可命令
			{
			/* ディフォルトがPB closeになった為、次の紙幣許可しずらくなった */
			/* PB Openを行う必要がでた */	
				if( ex_2nd_note_uba == 1)//2022-02-21
				{
				#if 1 //2023-08-10 入口の再確認は止める
					_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ_FIRST);
					_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_STACKING_ENTRY_ON, 0, 0, 0);	// ラインに連続取り込みを通知する為、SUCCESS以外のメッセージにした方がいい
				#else
					/* 連続動作か*/
					bill_in = _main_bill_in();
					if (bill_in == BILL_IN_ENTRANCE)
					{
						_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ_FIRST);
						_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_STACKING_ENTRY_ON, 0, 0, 0);	// ラインに連続取り込みを通知する為、SUCCESS以外のメッセージにした方がいい
					}
					else
					{
						_main_set_mode(MODE1_STACK, STACK_MODE2_STACK_HOME_WAIT_NOTE);
					}
				#endif
				}
				else
				{
				/* 連続動作でない場合、ここでHomeになるまで待つ*/

				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS) // 収納成功 TMSG_SUB_ENABLE_NEXTで次のモードに遷移している為、通常ここに入る事はあり得ない、
			// ICBのキューがリミットに近い場合はここに入る
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
			
			#if !defined(UBA_RTQ)
				/* 搬送逆転でVendか決める*/
				if( is_security_mode() )	//#if defined(HIGH_SECURITY_MODE)
				{
					/* UBA同等逆転バージョン*/
					_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_REV_REQ);
					ex_multi_job.busy |= TASK_ST_FEED;
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REV_CHECK_BILL_REQ, 0, 0, 0, 0);
				}
				else
				{

					/* Normal Operation */
					// 連続処理でLINEにVendを送信すると、その後、LINEからメッセージを受信して処理が複雑になるので、今後検討
					set_recovery_step(RECOVERY_STEP_VEND);
					_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);	/* vend position 2 */

					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (!(is_box_set()))
					{
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else
					{
					// next step
						// 連続処理は辞める
						_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
					}
				}
			#else //RTQ
				//2025-04-01
				mode_stack_home_finish_rtq();
			#endif
			}
			// MSG_SUB_ENABLE_NEXTの前にTMSG_SUB_REJECTが発生しないのなら、この処理はいらないが将来の変更を考慮していれておく
			else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
				/* 半戻しなのにHomeになってしまった */
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// 戻し動作は成功したので、モードはこのままで押し込みリトライ命令を行う
					ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更

					_main_set_mode(MODE1_STACK, STACK_MODE2_STACK_RETRY);
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, SS_BILL_STACK, 0, 0, 0);	// リトライ用の押し込み命令へ
				}
				break;
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
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 226);
			}
			break;
#if defined(UBA_RTQ)		/* '18-05-01 */
	case	TMSG_RC_STATUS_INFO: //ok
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 237);
			}
			break;
	}
}


/*********************************************************************//**
 * @brief stack exec procedure 押しメカHome戻し中,紙幣検知待ち
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0 //2023-08-10
void stack_wait_stack_home_wait_note(void)//連続保留中、押し込み前は入口ON,戻し開始時は入口OFF 連続がキャンセルされる可能性あり//連続動作用,入口センサ監視中
{
    u16 bill_in;

    switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			ex_main_reset_flag = 1;
			break;
	case	TMSG_SENSOR_STATUS_INFO:

			if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
			}
			else
			{
				bill_in = _main_bill_in();
				if (bill_in == BILL_IN_ENTRANCE)
				{
				//紙幣を検知したので、LINEに取り込み開始を要請
					_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ_FIRST);
					_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_STACKING_ENTRY_ON, 0, 0, 0);	// ラインに連続取り込みを通知する為、SUCCESS以外のメッセージにした方がいい
				}
				else if (bill_in == BILL_IN_ACCEPTOR)
				{
				/* ここで返却処理を行わないで、待機時で返却処理する */

				}
				else if (bill_in == BILL_IN_STACKER)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
				}
				else
				{
				//とくになにもしない
				}
			}
			break;
	case	TMSG_TIMER_TIMES_UP:
			break;
	case	TMSG_STACKER_PULL_RSP:	// MUL 主の処理1
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS) // 収納成功
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				/* Normal Operation */

				// 連続処理でLINEにVendを送信すると、その後、LINEからメッセージを受信して処理が複雑になるので、今後検討
				set_recovery_step(RECOVERY_STEP_VEND);
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);	/* vend position 2 */


				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// next step
				// 連続処理は辞める
					_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
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
				{ /* other job normal end */
					ex_multi_job.alarm |= TASK_ST_STACKER;
					ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ENABLE_NEXT)
			{
				// 
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 227);
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
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 238);
			}
			break;
	}
}

#endif

/*********************************************************************//**
 * @brief wait request (stack state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void stack_wait_req_first(void)	//use SS RTQ //連続動作用, 入口で紙幣を検知していて、LINEからの許可待ち
{
	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
	case	TMSG_CLINE_SET_STATUS:
			if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;

	case	TMSG_CLINE_ACCEPT_REQ:
	case	TMSG_DLINE_ACCEPT_REQ:
 	/* 次の取り込み動作へ*/	
			ex_2nd_note_uba = 1; //2023-08-10で廃止した処理が復活した場合、ここに設定しないと問題になる可能性があるので
			dly_tsk(10); //Disableが反映されるのを待つ 5msecで十分だが10にする

			_main_set_accept(); //2023-07-20
			break;

	case	TMSG_SENSOR_STATUS_INFO:
			if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			break;
	case	TMSG_TIMER_TIMES_UP:
		#if defined(UBA_RTQ)
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#endif
			break;
	case	TMSG_STACKER_PULL_RSP:	// MUL SUB
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS) // 収納成功
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				/* Normal Operation */
				// 戻しも成功したので、Vend
				// 連続処理でLINEにVendを送信すると、その後、LINEからメッセージを受信して処理が複雑になるので、今後検討
				set_recovery_step(RECOVERY_STEP_VEND);
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// next step
				// 入口に紙幣を検知していて、LINからの許可待ち状態の連続動作なので、なにもしない
					_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);	/* 2018-11-09	*/
				}
			}
			//連続取り込みキャンセル
			else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
				/* 半戻しなのにHomeになってしまった */
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// 戻し動作は成功したので、モードはこのままで押し込みリトライ命令を行う
					ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更
					_main_set_mode(MODE1_STACK, STACK_MODE2_STACK_RETRY);
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, SS_BILL_STACK, 0, 0, 0);	// リトライ用の押し込み命令へ
				}
				break;
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{ /* other job normal end */
					ex_multi_job.alarm |= TASK_ST_STACKER;
					ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ENABLE_NEXT)
			{
				// ここのくることはない
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 228);
			}
			break;
	#if defined(UBA_RTQ)		/* '18-05-01 */
	case	TMSG_RC_STATUS_INFO: //ok
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	#endif
	default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 77);
			}
			break;
	}
}

void stack_exec_retry(void)	//use SS RTQ // 押し込みリトライ動作用
{
	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			ex_main_reset_flag = 1;
			break;
	case	TMSG_SENSOR_STATUS_INFO:

			break;
	case	TMSG_TIMER_TIMES_UP:
		#if defined(UBA_RTQ)
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#endif
			break;
	case	TMSG_STACKER_EXEC_NG_PULL_RSP:	//Setp1  1度目の押し込みでNG、押しメカ戻し動作,リトライ必修
			ex_multi_job.busy &= ~(TASK_ST_STACKER);

			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// 戻し動作は成功したので、モードはこのままで押し込みリトライ命令を行う
					ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更
				#if defined(UBA_RTQ)//#if defined(NEW_STACK)
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, 0, 0, 0, 0);	// リトライ用の押し込み命令へ
				#else
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, SS_BILL_STACK, 0, 0, 0);	// リトライ用の押し込み命令へ
				#endif
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
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 229);
			}
			break;
	case	TMSG_STACKER_EXEC_RE_RSP:	//Setp2  1度目の押し込みでNG、押しメカ押し込み and 戻し動作
			ex_multi_job.busy &= ~(TASK_ST_STACKER);

			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{

			#if 1//#ifdef _ENABLE_JDL
				jdl_accept(ex_validation.denomi);
			#endif /* _ENABLE_JDL */

				if(is_icb_enable())
				{
					ex_multi_job.busy |= TASK_ST_ICB; //2024-04-01
				#if defined(UBA_RTQ_ICB)
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI, ex_validation.denomi, 0, 0);	//通常			
				#else
					set_recovery_step(RECOVERY_STEP_ICB_ACCEPT);
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_REQ, ex_validation.denomi, 0, 0, 0);
				#endif
					break;	//2024-04-01
				}
				else
				{
				//ここから
				#if !defined(UBA_RTQ) //2025-04-01
					/* 搬送逆転でVendか決める*/
					if( is_security_mode() )	//#if defined(HIGH_SECURITY_MODE)
					{
						_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_REV_REQ);
						ex_multi_job.busy |= TASK_ST_FEED;
						_main_send_msg(ID_FEED_MBX, TMSG_FEED_REV_CHECK_BILL_REQ, 0, 0, 0, 0);
					}
					else
					{

						/* Normal Operation */
						set_recovery_step(RECOVERY_STEP_VEND);
						_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);

						if (ex_main_reset_flag)
						{ /* リセット要求有り */
							_main_set_init();
						}
						else if (!(is_box_set()))
						{
							_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
						}
						else
						{	
							_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
							_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
						}
					}
				#else
					mode_stack_home_finish_rtq();
				#endif
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{

				if( ALARM_CODE_STACKER_FULL == ex_main_msg.arg2 )
				{
					//2024-04-01
					ex_multi_job.alarm |= TASK_ST_STACKER;
					ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;

				#if 1//#ifdef _ENABLE_JDL
					jdl_accept(ex_validation.denomi);
				#endif /* _ENABLE_JDL */
				
					if(is_icb_enable())
					{
						ex_multi_job.busy |= TASK_ST_ICB; //2024-04-01
					#if defined(UBA_RTQ_ICB)
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI, ex_validation.denomi, 0, 0);	//通常			
					#else	
						set_recovery_step(RECOVERY_STEP_ICB_ACCEPT);
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_REQ, ex_validation.denomi, 0, 0, 0);
					#endif
						break;	//2024-04-01
					}
					else
					{
					/* Normal Operation */
						set_recovery_step(RECOVERY_STEP_VEND);
						//ID003_MODE_VEND_FULL でラインタスク側でVendを出すので、Mainから送る必要ない(Fullの時)
						//_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);

					}
				}
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 230);
			}
			break;

	case 	TMSG_ICB_ACCEPT_RSP:	//2024-04-01
#if defined(UBA_RTQ_ICB)
	case 	TMSG_ICB_ACCEPT_RTQ_RSP:	//2025-07-01
#endif
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~TASK_ST_ICB;	
				//受け取り動作継続 OR Fullエラー
				if (ex_multi_job.alarm)
				{ /* other job alarm */
				//
				#if defined(UBA_RTQ_ICB)	
					_main_rtq_rfid();
				#endif

					set_recovery_step(RECOVERY_STEP_VEND);
					//ID003_MODE_VEND_FULL でラインタスク側でVendを出すので、Mainから送る必要ない(Fullの時)
					//_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);

					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else
					{
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
				}
				else
				{
				#if !defined(UBA_RTQ) //2025-04-01
					if( is_security_mode() )	//#if defined(HIGH_SECURITY_MODE)
					{
						_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_REV_REQ);
						ex_multi_job.busy |= TASK_ST_FEED;
						_main_send_msg(ID_FEED_MBX, TMSG_FEED_REV_CHECK_BILL_REQ, 0, 0, 0, 0);
					}
					else
					{

						/* Normal Operation */
						set_recovery_step(RECOVERY_STEP_VEND);
						_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);

						if (ex_main_reset_flag)
						{ /* リセット要求有り */
							_main_set_init();
						}
						else if (!(is_box_set()))
						{
							_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
						}
						else
						{	
							_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
							_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
						}
					}
				#else //RTQ
					mode_stack_home_finish_rtq();
				#endif
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_ICB);

				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					#if 0 //ICBエラーは優先なので
					else if (ex_multi_job.alarm)
					{ /* other job alarm */
					//
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
					}
					#endif
					else
					{ /* other job normal end */
						_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}
				else
				{
					ex_multi_job.alarm |= TASK_ST_ICB;
					ex_multi_job.code[MULTI_ICB] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_ICB] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_ICB] = ex_main_msg.arg4;
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 173);
			}
			break;

	#if defined(UBA_RTQ)		/* '18-05-01 */
	case	TMSG_RC_STATUS_INFO:	//ok
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	#endif
	default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 239);
			}
			break;
	}
}

#if !defined(UBA_RTQ) //only SS, not use RTQ
void stack_feed_rev_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			ex_main_reset_flag = 1;
			break;
	case	TMSG_SENSOR_STATUS_INFO:
			break;

	case	TMSG_FEED_REV_CHECK_BILL_RSP:

			ex_multi_job.busy &= ~(TASK_ST_FEED);
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS) // 収納成功
			{
			/* 紙幣は残っていない*/
				/* Normal Operation */
				// 連続処理でLINEにVendを送信すると、その後、LINEからメッセージを受信して処理が複雑になるので、今後検討
				set_recovery_step(RECOVERY_STEP_VEND);
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);	/* vend position 2 */

				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (!(is_box_set()))
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{
				// next step
					_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
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
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 226);
			}
			break;

	case	TMSG_TIMER_TIMES_UP:
			if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
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
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 237);
			}
			break;
	}
}
#endif


#if defined(UBA_RTQ)

void stack_feed_rc_stack()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_FEED_RC_STACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_APB + TASK_ST_SHUTTER)) == 0)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					if (!rc_warning_status())
					{
						if (!(rc_busy_status()))
						{
							stack_feed_rc_stack_sub(); //Feed success + PB done + Shutter done + RTQ not busy
						}
						else
						{
							_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
						}
					}
					else
					{
						ex_rc_retry_flg = TRUE;	//2025-02-01
						/* retry */
						stack_rc_retry_operation();
					}
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{
		/* リサイクラへの搬送時のPB */
			ex_multi_job.busy |= TASK_ST_APB;			
			if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)  //2024-09-30
			{
				_main_send_msg(ID_APB_MBX, TMSG_APB_EXEC_REQ, get_PBcount(), 0, 0, 0);
			}
			else
			{
				_main_send_msg(ID_APB_MBX, TMSG_APB_EXEC_REQ, 1, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_send_msg(ID_RC_MBX, TMSG_RC_CANCEL_REQ, 0, 0, 0, 0);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				if (ex_main_emergency_flag != 0)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_FEED_TIMEOUT_SK, ex_main_msg.arg3, ex_main_msg.arg4);
				}
				else
				{
					_main_reject_sub(MODE1_STACK, STACK_MODE2_WAIT_REJECT_REQ, TMSG_CONN_STACK, REJECT_CODE_OPERATION, _main_conv_seq(), ex_position_sensor);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);

			/* system error ? */
			_main_system_error(0, 90);
		}
		break;
	case TMSG_APB_EXEC_RSP: /* リサイクラへの搬送時のPB */
		if (ex_main_msg.arg1 == TMSG_SUB_START)
		{
			/* RC を入る場合*/
			ex_multi_job.busy |= TASK_ST_SHUTTER;
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_APB + TASK_ST_SHUTTER)) == 0)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					if (!rc_warning_status())
					{
						if (!(rc_busy_status()))
						{
							stack_feed_rc_stack_sub(); //Feed done + PB finish + Shutter done + RTQ not busy
						}
						else
						{
							_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
						}
					}
					else
					{
						ex_rc_retry_flg = TRUE;	//2025-02-01
						/* retry */
						stack_rc_retry_operation();
					}
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);

			/* system error ? */
			_main_system_error(0, 71);
		}
		break;
	case TMSG_SHUTTER_OPEN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_START)
		{
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_SHUTTER);
			if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_APB + TASK_ST_SHUTTER)) == 0)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					if (!rc_warning_status())
					{
						if (!(rc_busy_status()))
						{
							stack_feed_rc_stack_sub();  //Feed done + PB done + Shutter finish + RTQ not busy
						}
						else
						{
							_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
						}
					}
					else
					{
						ex_rc_retry_flg = TRUE;	//2025-02-01
						/* retry */
						stack_rc_retry_operation();
					}
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_SHUTTER);
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{																																						/* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]); // MUL_ERROR
				}
				else
				{	/* other job normal end */
					// どちらを優先するか不明
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_reset_flag) && (ex_main_pause_flag != 0))
			{
				ex_main_pause_flag = 0;
				_main_set_init();
			}
			else
			{
				ex_multi_job.alarm |= TASK_ST_SHUTTER;
				ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;
			}
		}
		else
		{
			ex_main_pause_flag = 0;
			/* system error ? */
			_main_system_error(0, 71);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
		//基本的にここはStack2のみ(busy解除待ち) 
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
			if (!rc_warning_status())
			{
				if (!(rc_busy_status()))
				{
					stack_feed_rc_stack_sub(); //RTQ not busy
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else
			{
				ex_rc_retry_flg = TRUE;	//2025-02-01				
				/* retry  */
				stack_rc_retry_operation();
			}
		}
		else  if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{			
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		/* Timeout for retry */
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_STATUS_INFO: //ok
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}



void stack_rc_feed_box() //新規作成 BOXへの搬送完了を待つのみ
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_FEED_RC_STACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_APB + TASK_ST_SHUTTER)) == 0)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else
				{
					if (!rc_warning_status())
					{
						if (!(rc_busy_status()))
						{
							stack_rc_feed_box_sub(); //Feed success + PB done + Shutter done + RTQ not busy
						}
						else
						{
							_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
						}
					}
					else
					{
						/* retry */
						ex_rc_retry_flg = TRUE;	//2025-02-27				
						stack_rc_retry_operation();
					}
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{
		/* このシーケンスでは、このメッセージは送られて来ない*/
		/* リサイクラへの搬送時のみおくられるので、BOX搬送では来ない */

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_send_msg(ID_RC_MBX, TMSG_RC_CANCEL_REQ, 0, 0, 0, 0);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				#if 1
				if (ex_main_emergency_flag != 0)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_FEED_TIMEOUT_SK, ex_main_msg.arg3, ex_main_msg.arg4);
				}
				else
				{
					_main_reject_sub(MODE1_STACK, STACK_MODE2_WAIT_REJECT_REQ, TMSG_CONN_STACK, REJECT_CODE_OPERATION, _main_conv_seq(), ex_position_sensor);
				}
				#else
				_main_reject_sub(MODE1_STACK, STACK_MODE2_WAIT_REJECT_REQ, TMSG_CONN_STACK, REJECT_CODE_OPERATION, _main_conv_seq(), ex_position_sensor);
				#endif 
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);

			/* system error ? */
			_main_system_error(0, 90);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
		//基本的にここはStack2のみ(busy解除待ち) 
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
			if (!rc_warning_status())
			{
				if (!(rc_busy_status()))
				{
					stack_rc_feed_box_sub(); //RTQ not busy
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else
			{
				/* retry  */
				ex_rc_retry_flg = TRUE;	//2025-02-27				
				stack_rc_retry_operation();
			}
		}
		else  if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{			
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		/* Timeout for retry */
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_STATUS_INFO: //ok
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

void stack_rc_retry_prefeed_stack()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_RC_PREFEED_STACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (rc_warning_status())
			{
				ex_rc_retry_flg = TRUE;
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & TASK_ST_RC) == 0)
				{
					ex_multi_job.busy |= TASK_ST_RC;

					/* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
					_main_set_mode(MODE1_STACK, STACK_MODE2_RC_PREFEED_STACK);
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
				{
					ex_multi_job.busy |= TASK_ST_RC;
					ex_multi_job.busy |= TASK_ST_FEED;

					_main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, OperationDenomi.unit, RC_RETRY_STACK_DIR, 0, 0);
					//背面搬送で紙幣が検知できなくなるまで、取り込み方法に回す, フラッパの状態によっては、回収庫もしくは、ドラムの方へ行く
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, OperationDenomi.unit, 0, 0, 0);

					_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_REV);
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
		/* Timeout for retry */
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

void stack_rc_retry_rev()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_RC_RETRY_BILL_DIR_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_FEED_RC_FORCE_PAYOUT_RSP:
		ex_multi_job.busy &= ~(TASK_ST_FEED);
		if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			
		}
		else
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (rc_warning_status())
			{
				ex_rc_retry_flg = TRUE; //2025-02-01
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
				{
					ex_multi_job.busy |= TASK_ST_RC;

					/* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
					_main_set_mode(MODE1_STACK, STACK_MODE2_RC_PREFEED_STACK);
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else if(!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
				{
					/* stacker home */
					if (!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
					{
						ex_multi_job.busy |= TASK_ST_STACKER;
						_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_STACK_HOME);
						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
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
						if (is_quad_model() && !(is_flapper2_head_to_box_pos()))
						{
							flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX; /* change position	*/
						}
						else
						{
							flap2_pos = 0; /* don't move		*/
						}

						/* send flapper command */
						if (flap1_pos == 0 && flap2_pos == 0)
						{
							ex_multi_job.busy |= TASK_ST_FEED;
							ex_multi_job.busy |= TASK_ST_RC;

							/* change box from rc to cash box */
							OperationDenomi.unit = RC_CASH_BOX;
							set_recovery_unit(RC_CASH_BOX, RC_CASH_BOX);

							_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX);
							_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, STACK, 0, 0);
							_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
						}
						else
						{
							_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_INIT_RC);
							_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
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
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

void stack_rc_retry_stack_home()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_STACKER_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
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
			if (is_quad_model() && !(is_flapper2_head_to_box_pos()))
			{
				flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX; /* change position	*/
			}
			else
			{
				flap2_pos = 0; /* don't move		*/
			}

			/* send flapper command */
			if (flap1_pos == 0 && flap2_pos == 0)
			{
				ex_multi_job.busy |= TASK_ST_FEED;
				ex_multi_job.busy |= TASK_ST_RC;

				/* change box from rc to cash box */
				OperationDenomi.unit = RC_CASH_BOX;
				set_recovery_unit(RC_CASH_BOX, RC_CASH_BOX);

				_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, STACK, 0, 0);
				_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
			}
			else
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_INIT_RC);
				_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			_main_system_error(0, 91);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

void stack_rc_retry_init_rc()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_RC_FLAPPER_RSP:
		if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if(rc_busy_status())
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
			}
			else
			{
				ex_multi_job.busy |= TASK_ST_FEED;
				ex_multi_job.busy |= TASK_ST_RC;

				/* change box from rc to cash box */
				OperationDenomi.unit = RC_CASH_BOX;
				set_recovery_unit(RC_CASH_BOX, RC_CASH_BOX);

				_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, STACK, 0, 0);

				_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			if (rc_busy_status())
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
			}
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

void stack_rc_retry_feed_box()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_FEED_RC_STACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_RC_FEED_BOX_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (rc_warning_status())
			{
				ex_rc_retry_flg = TRUE;	//2025-02-01				
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
				{
					ex_multi_job.busy |= TASK_ST_RC;
					ex_multi_job.busy |= TASK_ST_FEED;

					_main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, RC_CASH_BOX, RC_RETRY_PAYOUT_DIR, 0, 0);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, RC_CASH_BOX, 0, 0, 0); //回収庫への搬送中にワーニング

					_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX_REV);
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else if(!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy &= (TASK_ST_FEED + TASK_ST_RC)) == 0)
				{
					ex_multi_job.busy |= TASK_ST_STACKER; // 起動タスクにより変更
					set_recovery_step(RECOVERY_STEP_STACKING);
					_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_STACK_START);
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
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
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

void stack_rc_retry_feed_box_rev()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (rc_warning_status())
			{
				ex_rc_retry_flg = TRUE;	//2025-02-01
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
				{
					ex_multi_job.busy |= TASK_ST_FEED;
					ex_multi_job.busy |= TASK_ST_RC;

					_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, STACK, 0, 0);
					_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
			else if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);

				if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
				{
					ex_multi_job.busy |= TASK_ST_FEED;
					ex_multi_job.busy |= TASK_ST_RC;

					_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, STACK, 0, 0);
					_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
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
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			if (rc_busy_status())
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
			}
		}
		break;
	case TMSG_RC_RETRY_BILL_DIR_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_FEED_RC_FORCE_PAYOUT_RSP:
		ex_multi_job.busy &= ~(TASK_ST_FEED);
		//
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (!(is_detect_rc_twin()) || /* detect twin box */
				!(is_detect_rc_quad()))	  /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STACK, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STACK, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

static void stack_feed_rc_stack_sub() //ドラム用
{

	#if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20	//2025-02-20 エージングテスト用
//生産エージングソフトは、収納後にmode_stack状態でインターバルを行っているので
//CISの消灯をここに入れる
//通常動作ではmode_enable、mode_disableで消している
	_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
	//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
	dly_tsk(5);
	_main_set_pl_active(PL_DISABLE);
#endif
	// TODO: cheat check

	// if (!is_bookmark_mode())
	// {
	// 	set_recovery_step(RECOVERY_STEP_VEND);
	// 	_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);
	// 	// TODO: incBoxCount
	// }
	set_recovery_step(RECOVERY_STEP_VEND);
	_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);
	// TODO: incBoxCount
	_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
	_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);

	/* JDL set log*/
	jdl_rc_stack(OperationDenomi.unit, 0, ex_validation.denomi, ex_validation.bill_length);
	jdl_rc_each_count(OperationDenomi.unit, STACK);

}

static void stack_rc_feed_box_sub() //BOX用
{
	/* Not emergency */
	if (ex_main_emergency_flag == 0)
	{
		set_recovery_step(RECOVERY_STEP_STACKING);
		_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_STACK_START);
	}
	else
	{
		_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_COLLECT, 0, 0, 0);
		_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_STACK_START);

		#if defined(UBA_RTQ_ICB) //2025-03-25 2025-05-09
		if(is_icb_enable())
		{
			_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI_UNIT, OperationDenomi.unit_emergency, 0, 0); //Payout to Emergency stop
		}				
		#endif
	}	
	ex_multi_job.busy |= TASK_ST_STACKER;
	_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
}



static void stack_rc_retry_operation() //ワーニング発生中、紙幣位置のよってRTQへ通知するかの違い(RTQ側も動かすか判断)
{

	if ((ex_multi_job.busy & TASK_ST_FEED) == 0)
	{
		// ヘッド内に紙幣がある場合
		if ((((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0) &&
			 ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0) &&
			 ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) == 0) &&
			 ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) == 0)) &&
			((SENSOR_EXIT) || (SENSOR_APB_IN) || (SENSOR_APB_OUT)))
		{
			ex_multi_job.busy |= TASK_ST_FEED;

			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0); //ワーニング発生中、背面搬送まで戻したい

			if (OperationDenomi.unit != RC_CASH_BOX)
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_REV);
			}
			else
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX_REV);
			}
		}
		else
		{
			ex_multi_job.busy |= TASK_ST_FEED;
			ex_multi_job.busy |= TASK_ST_RC;

			_main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, OperationDenomi.unit, RC_RETRY_PAYOUT_DIR, 0, 0);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0); //ワーニング発生中、背面搬送までもどしたい

			if (OperationDenomi.unit != RC_CASH_BOX)
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_REV);
			}
			else
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_RC_RETRY_FEED_BOX_REV);
			}
		}
	}
	else
	{
	}
}

#endif // UBA_RTQ


void finish_stacker_pb_icb(void) //use SS RTQ
{

	if (ex_main_reset_flag)
	{ /* リセット要求有り */
		_main_set_init();
	}
	else if (ex_multi_job.alarm)
	{ /* other job alarm */
		// alarmの場合、codeのMAINのmode分岐に使用している為大切
		_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);//MUL_ERROR
	}
	else if (ex_multi_job.reject)
	{ /* othrer job reject */
		ex_multi_job.reject &= ~(TASK_ST_STACKER);
		// 押し込みリトライ処理へ
		ex_multi_job.busy |= TASK_ST_STACKER;
		// スタッカHomeへ
		_main_set_mode(MODE1_STACK, STACK_MODE2_STACK_RETRY);
		_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
		// rejectの場合、code以外特に使用していない、codeもそのまま流すだけ
	}
	else if (!(is_box_set()))
	{
		_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
	}
	else
	{ /* all job normal end */
	// next step

		ex_multi_job.busy |= TASK_ST_STACKER;
		_main_set_mode(MODE1_STACK, STACK_MODE2_STACK_HOME);
	#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		//2023-09-27
		/* 暗号化有効時は単体動作*/
		if(is_security_mode())
		{
			/* 単体モード*/
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, SS_PULL_HALF, 0, 0, 0);	/* 単体 */
		}
		else if(is_uba_mode())
		{
			/* DIP-SW 7 ONで強制単体モードの時は、単体モード*/
			/* 単体モード*/
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, SS_PULL_HALF, 0, 0, 0);	/* 単体 */
		}
		else if(ex_is_uba_mode==1)
		{
			/* 搬送イニシャルで単体モード判定の時は、単体モード*/
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, SS_PULL_HALF, 0, 0, 0);	/* 単体 */
		}
		else if(ex_2nd_note_uba == 1) //収納の時は、この条件が追加される
		{
			/* 並列処理へ */
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);	/*  並列許可  */										
		}
		else
		{
			/* 単体モード*/
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, SS_PULL_HALF, 0, 0, 0);	/* 単体 */
		}
	#else
		//常にHome戻し
		_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);
	#endif
	}//


}


#if (DATA_COLLECTION_DEBUG==1)
void stack_feed_data_collection(void)	//only Data collection
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
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_FEED_APB_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_reject_sub(MODE1_STACK, STACK_MODE2_WAIT_REJECT_REQ, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			ex_main_pause_flag = 0;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else
		{
			ex_main_pause_flag = 0;
			/* system error ? */
			_main_system_error(0, 73);
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
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 176);
		}
		break;
	}
}
#endif

#if defined(UBA_RTQ)
u8	get_PBcount(void)
{
	return ((ex_timer_task_tick % 5)+3);//random number from 3 to 7 to rotate the PB 3-7 times
}


void mode_stack_home_finish_rtq(void) //2025-04-01
{

	//RTQは搬送逆転は存在しない、将来的にPB動作を変える可能性があるので、UBA500RTQと同様にSSとは処理分ける 20250401
	/* Normal Operation */
	// 連続処理でLINEにVendを送信すると、その後、LINEからメッセージを受信して処理が複雑になるので、今後検討
	#if 0//とりあえず、今は未対応 #if defined() && defined(ID003_SPECK64)
	if((ex_cheat_apb_occurred) && ((ex_line_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION) && OperationDenomi.mode == 0)
	{
		_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_CHEAT, _main_conv_seq(), ex_position_sensor);
	}
	else
	#endif
	{
		set_recovery_step(RECOVERY_STEP_VEND);
		_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);	/* vend position 2 */
	}
	if (ex_main_reset_flag)
	{ /* リセット要求有り */
		_main_set_init();
	}
	else if(ex_rc_error_flag)
	{
		_main_alarm_sub(0, 0, TMSG_CONN_STACK, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
	}
	else if (!(is_box_set()))
	{
		_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
	}
	else
	{
	// next step
	#if 0//とりあえず、今はPB監視とCloseは行わない #if defined(ID003_SPECK64)
		if((ex_line_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
		{
			if(ex_cheat_apb_occurred && OperationDenomi.mode == 0)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STACK, ALARM_CODE_CHEAT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_PB_CLOSE);
				_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);
			}							
		}
		else
		{
			_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
		}					
	#else
		_main_set_mode(MODE1_STACK, STACK_MODE2_WAIT_REQ);
		_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
	#endif
	}
}

#endif
/* EOF */
