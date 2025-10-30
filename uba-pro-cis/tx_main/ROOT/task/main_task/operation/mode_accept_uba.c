/******************************************************************************/
/*! @addtogroup Main
    @file       mode_accept.c
    @brief      accept mode of main task
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
/***************************** Include Files *********************************/
#include <string.h>
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "operation.h"
#include "sub_functions.h"
#include "sensor.h"
#include "sensor_ad.h"
#include "status_tbl.h"

#include "systemdef.h"					//2022-02-17 test
#include "cyclonev_sysmgr_reg_def.h"	//2022-02-17 test
#include "hal_gpio_reg.h"				//2022-02-17 test

#if defined(UBA_RTQ)
#include "if_rc.h"
#endif

#define EXT
#include "com_ram.c"
#include "cis_ram.c"

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/
#if defined(UBA_RTQ)
static u8 flap1_pos;
static u8 flap2_pos;
#endif // UBA_RTQ
/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/
void accept_wait_accept_req(void);
void mode_accpet_uba_stack(u8 run);
void mode_accpet_uba_start_check(void);

/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void accept_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	/*---------------------------------------------*/
	/* 紙幣取り込み前の準備 */
	case ACCEPT_MODE2_SENSOR_ACTIVE:	/* CIS完了を待つのが主 *//* 遷移時PB動作命令 */
		accept_sensor_active();			/* 動作 連続動作の収納 */
		break;

	case ACCEPT_MODE2_PB_CENTERING_HOME:	/* 既存 */// 幅寄せ専用
		 pb_centering_home();				/* 動作 幅よせ、連続動作の収納 */// 幅寄せ専用 取り込み開始直前に幅寄せがHomeになかったのでHomeに戻す
		 break;

	/*-----------------------------------------------------*/
	/* 本来はPB,搬送の順で動かすが、PBがすでにHomeの場合省略*/
	case ACCEPT_MODE2_WAIT_PB_START: 		/* 新規 *//* 遷移時、搬送動作命令 *//* ok */
		 wait_pb_start();					/* 動作 PB、連続動作の収納 */
		 break;

	case ACCEPT_MODE2_WAIT_FEED_START:		/* PB動作中,遷移時 収納Home命令 */
		 wait_feed_start();					/* 動作 PB、搬送、連続動作の収納 */// 幅寄せも固定幅も共通 搬送モータ動作待ち、メッセージ受信後、押しメカをHomeに戻す ここから連続動作の押しメカ動作が重複する
		 break;

	case ACCEPT_MODE2_FEED_CENTERING:		/* 動作 PB、搬送、連続動作の収納 *//* ここでPB完了を待たせる */
		feed_centering();					
		break;

	case ACCEPT_MODE2_CENTERING:			/* 動作 幅よせ、収納、連続動作の収納*/
		centering_exec();					/* 動作 幅よせ、収納、連続動作の収納*///ここが、1枚受け取りも連続受け取りもスタッカーのHome戻し待ちの最終場所
		break;

	/*-----------------------------------------------------*/
	/* これ以降はStacker Homeに戻っている*/
	case ACCEPT_MODE2_FEED_ESCROW:			/* common *//* 途中でシャッター締めたい *//* 途中でシャッターを締める動作は、電流問題で廃止、識別中にシャッターを締める */
		feed_escrow();						/* 動作 搬送、Vendに対するAck受信、未受信で処理分岐*/
		break;
	case ACCEPT_MODE2_WAIT_ACCEPT_REQ:		/* Escrow位置に到達 連続動作のID-003 1枚目のVendに対するAck待ち */
		accept_wait_accept_req();			/* Escrow位置に到達 連続動作のID-003 1枚目のVendに対するAck待ち */
		break;
	/*----------------------------------------------------*/
	/* これ移行は、連続動作のID-003 Vendに対するAck受信後*/
	case ACCEPT_MODE2_DISCRIMINATION:		/* common *//* シャッター動作中なので、識別完了でも待つ */
		discrimination();
		break;
	case ACCEPT_MODE2_WAIT_REQ:				/* common */
		accept_wait_req();
		break;
	case ACCEPT_MODE2_WAIT_REJECT_REQ:		/* common *//* 返却は全て、一度ここにいれる、ここでICB,幅よせ、シャッター関係を処理する*/
		accept_wait_reject_req();
		break;

#if defined(UBA_RTQ)
	case ACCEPT_MODE2_INIT_RC:
		accept_init_rc();
		break;
	case ACCEPT_MODE2_WAIT_RC_RSP:
		accept_wait_rc_rsp();
		break;
#endif // UBA_RTQ

	default:
		/* system error ? */
		_main_system_error(0, 130);
		break;
	}

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_1ST_NOTE_DONE_REQ://ここに残す
		if( ex_2nd_note_uba == 1 )
		{
			ex_2nd_note_uba = 2;
		}
		break;
	}
}


/*********************************************************************//**
 * @brief sensor active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void accept_sensor_active(void)	/* CIS完了を待つのが主 */
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
		_main_set_init();
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_SENSOR;
			#if 1 /* 2022-03-02 */
			/* 連続処理の場合Stackerは動いているので無視 */
			if ( !(ex_multi_job.busy) || ( (ex_multi_job.busy == TASK_ST_STACKER) && ex_2nd_note_uba == 1) )
			#else
			if (!(ex_multi_job.busy))
			#endif
			{ /* all job end */
				if (!(SENSOR_CENTERING_HOME))
				{
					ex_multi_job.busy |= TASK_ST_CENTERING;
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_PB_CENTERING_HOME);
					_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
				}
				else
				{
					mode_accpet_uba_start_check(); /* 取り込み開始していいかの判断*/
				}
			}
		}
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_CIS_INIT;
			#if 1 /* 2022-03-02 */
			/* 連続処理の場合Stackerは動いているので無視 */
			if ( !(ex_multi_job.busy) || ( (ex_multi_job.busy == TASK_ST_STACKER) && ex_2nd_note_uba == 1) )
			#else
			if (!(ex_multi_job.busy))
			#endif
			{ /* all job end */
				if (!(SENSOR_CENTERING_HOME))
				{
					ex_multi_job.busy |= TASK_ST_CENTERING;
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_PB_CENTERING_HOME);
					_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
				}
				else
				{
					mode_accpet_uba_start_check(); /* 取り込み開始していいかの判断*/
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_CIS_INIT;
			if (!(ex_multi_job.alarm))
			{ /* set alarm inform */
				ex_multi_job.alarm |= TASK_ST_CIS_INIT;
				ex_multi_job.code[MULTI_CIS] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_CIS] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_CIS] = ex_main_msg.arg4;
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 131);
		}
		break;

	case	TMSG_STACKER_PULL_RSP:	// 連続動作の1枚目戻し処理
			mode_accpet_uba_stack(0);
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
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 131);
		}
		break;
	}
}


/******************************************************************************/
/*! @brief feed centering procedure
    @return         none
    @exception      none
******************************************************************************/
void feed_centering(void)	/* 幅よせ位置までの、搬送待ち */
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
#endif // UBA_RTQ
		break;
	case TMSG_FEED_CENTERING_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);

			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				#if 1//#if defined(WAIT_PB_BEFORE_WID)
				/* 幅よせ動作前に、PB 終わるのを末 */
				if( (ex_multi_job.busy & TASK_ST_APB) != 0 )
				{
					/* このモードでPB動作待ち */

				}
				else
				{
					//2023-09-13
					if (ex_multi_job.alarm & TASK_ST_STACKER)
					{ /* other job alarm */
					// ステータスとしてはErrorを優先させる	/* 紙幣返却後、スタッカーエラー	Reject点滅の為　ex_multi_job.code[MULTI_STACKER]は使用できない　*/
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
					}
					else if (ex_multi_job.alarm & TASK_ST_APB)
					{ /* other job alarm */
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_APB_HOME, ex_multi_job.sequence[MULTI_APB], ex_multi_job.sensor[MULTI_APB]); // MUL_ERROR
					}
					else
					{ /* other job normal end */
						_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_CENTERING);
						ex_multi_job.busy |= TASK_ST_CENTERING;
						ex_wid_reject_uba = 0;
						/* 幅よせが上手く動作しないくてもエラーにはしない*/
						_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 1, 0, 0, 0);
					#if defined(UBA_RTQ)//#if defined(UBA_RS)
						/* Check if RS Flap was in the correct position */
						if (( is_rc_rs_unit() )							/* RS existed */
						 || ((ex_main_test_no == TEST_RC_AGING || ex_main_test_no == TEST_RC_AGING_FACTORY) && ex_rc_configuration.unit_type_bk == RS_CONNECT)) /* For test and used module RS */
						{
							if(!(RS_FLAP_IN_POS))
							{
								ex_multi_job.busy |= TASK_ST_RC;
								_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, RS_FLAP_POS_IN, 0, 0, 0);
							}
						}
					#endif
					}
				}
				#else
				_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_CENTERING);
				ex_multi_job.busy |= TASK_ST_CENTERING;
				ex_wid_reject_uba = 0;
				/* 幅よせが上手く動作しないくてもエラーにはしない*/
				_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 1, 0, 0, 0);
				#endif
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			// リジェクトの場合の全ての処理が終わるのを待つ必要がある
			ex_multi_job.busy &= ~(TASK_ST_FEED);// 起動タスクにより変更

			ex_multi_job.reject |= TASK_ST_FEED;
			ex_multi_job.code[MULTI_FEED] = ex_main_msg.arg2;
			ex_multi_job.sequence[MULTI_FEED] = ex_main_msg.arg3;
			ex_multi_job.sensor[MULTI_FEED] = ex_main_msg.arg4;

			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{ /* other job alarm */
				// ステータスとしてはErrorを優先させる	/* 紙幣返却後、スタッカーエラー	Reject点滅の為　ex_multi_job.code[MULTI_STACKER]は使用できない　*/
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else
				{ /* other job normal end */
					// 2017-10-20ここで問題となっているそもそも、Rejectのはず
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			// エラーの場合の全ての処理が終わるのを待つ必要がある
			ex_multi_job.busy &= ~(TASK_ST_FEED);// 起動タスクにより変更

			ex_multi_job.alarm |= TASK_ST_FEED;
			ex_multi_job.code[MULTI_FEED] = ex_main_msg.arg2;
			ex_multi_job.sequence[MULTI_FEED] = ex_main_msg.arg3;
			ex_multi_job.sensor[MULTI_FEED] = ex_main_msg.arg4;

			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{ /* other job alarm */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 134);
		}
		break;

	/* PBも追加 */
	case	TMSG_APB_HOME_RSP:	/*  */
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);
				#if 1//#if defined(WAIT_PB_BEFORE_WID)
				/* 幅よせ動作前に、PB 終わるのを待っている、 */
				/* 搬送が終わっていれば遷移させる */
				if( (ex_multi_job.busy & TASK_ST_FEED) != 0 )
				{
					/* このモードで搬送完了待ち */

				}
				else
				{
					//2023-09-13
					if (ex_multi_job.alarm & TASK_ST_STACKER)
					{ /* other job alarm */
					// ステータスとしてはErrorを優先させる	/* 紙幣返却後、スタッカーエラー	Reject点滅の為　ex_multi_job.code[MULTI_STACKER]は使用できない　*/
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
					}
					else
					{
						_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_CENTERING);
						ex_multi_job.busy |= TASK_ST_CENTERING;
						ex_wid_reject_uba = 0;
						/* 幅よせが上手く動作しないくてもエラーにはしない*/
						_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 1, 0, 0, 0);
					#if defined(UBA_RTQ)//#if defined(UBA_RS)
						/* Check if RS Flap was in the correct position */
						if (( is_rc_rs_unit() )							/* RS existed */
						 || ((ex_main_test_no == TEST_RC_AGING || ex_main_test_no == TEST_RC_AGING_FACTORY) && ex_rc_configuration.unit_type_bk == RS_CONNECT)) /* For test and used module RS */
						{
							if(!(RS_FLAP_IN_POS))
							{
								ex_multi_job.busy |= TASK_ST_RC;
								_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, RS_FLAP_POS_IN, 0, 0, 0);
							}
						}
					#endif
					}
				}
				#endif
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);

				ex_multi_job.alarm |= TASK_ST_APB;
				ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;

				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else
					{
					#if 1 //2023-09-13
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_APB_HOME, ex_multi_job.sequence[MULTI_APB], ex_multi_job.sensor[MULTI_APB]); // MUL_ERROR
					#else	
						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
//						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[MULTI_APB], ex_multi_job.sequence[MULTI_APB], ex_multi_job.sensor[MULTI_APB]); // MUL_ERROR
					#endif
					}
				}
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{
				/* system error ? */
				_main_system_error(0, 78);
			}
			break;

	case	TMSG_STACKER_PULL_RSP:		// 連続動作の1枚目戻し処理
	#if defined(OLD_UBA_SEQ) 
	case	TMSG_STACKER_HOME_RSP:		// 1枚受け取りのHome戻し処理
	#endif
			mode_accpet_uba_stack(0);
			break;

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
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 135);
		}
		break;
	}
}


/******************************************************************************/
/*! @brief centering exec procedure
    @return         none
    @exception      none
******************************************************************************/
void centering_exec(void) /* ここでは、収納、幅よせ完了待ち *//* Escrow位置で紙幣とStackerがぶつかるので待つ */
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
#endif // UBA_RTQ
		break;
	#if 0//#if defined(WAIT_PB_BEFORE_WID) //PB動作完了は、幅よせ前になったのでここの処理廃止 2023-09-13
	case TMSG_APB_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* 幅よせ完了を待つ、必要あり */
			ex_multi_job.busy &= ~(TASK_ST_APB);
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				//2023-09-12
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else
				{ /* all job normal end */
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_FEED_ESCROW);
					ex_multi_job.busy |= TASK_ST_FEED;
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_ESCROW_REQ, 0, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			//2023-09-13
			ex_multi_job.alarm |= TASK_ST_APB;
			ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
			ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
			ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;


			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				#if 0 //2023-09-18
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				#endif
				else
				{ /* other job normal end */
			//2023-09-13		_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_APB_HOME, ex_multi_job.sequence[MULTI_APB], ex_multi_job.sensor[MULTI_APB]); // MUL_ERROR
				}
			}
			else
			{
			#if 0 //2023-09-13
				if (!(ex_multi_job.alarm))
				{ /* set alarm inform */
					ex_multi_job.alarm |= TASK_ST_APB;
					ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
				}
			#endif
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 137);
		}
		break;
	#endif
	case TMSG_CENTERING_EXEC_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_START)
		{
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				/* Wait massage TMSG_SUB_SUCCESS or TMSG_SUB_ALARM */
			}
		#if defined(OLD_UBA_SEQ)

		#else	//消費電流の関係で収納戻しは、幅よせ中にする 2022-02-18
			#if !defined(UBA_RTQ) //2024-09-10 処理速度を考えるとここでスタッカを戻した方がいいがUBA500 RTQに合わせて、識別後にHome戻しを行う
			ex_multi_job.busy |= TASK_ST_STACKER;
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
			#endif
		#endif
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{

			ex_multi_job.busy &= ~(TASK_ST_CENTERING);
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				//2023-09-12
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else
				{ /* all job normal end */
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_FEED_ESCROW);
					ex_multi_job.busy |= TASK_ST_FEED;
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_ESCROW_REQ, 0, 0, 0, 0);
				}
			}
		}
	//#if 1//#if defined(NEW_WID) /* 2022-03-02 */
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_wid_reject_uba = 1;
		// リジェクトの場合も全てのタスクが完了するのを待つ必要がある
			ex_multi_job.busy &= ~(TASK_ST_CENTERING);
			ex_multi_job.reject |= TASK_ST_CENTERING;
			ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
			ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
			ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;

			if (!(ex_multi_job.busy))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{ /* other job alarm */
					/* この処理は保留 */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else
				{
				// 返却優先
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					//_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			}
		}
	//#endif
		else
		{
			/* system error ? */
			_main_system_error(0, 138);
		}
		break;
	/* 押しメカのHome待ちも追加 */
	case TMSG_STACKER_PULL_RSP: 	// 連続動作の1枚目戻し処理	
	case TMSG_STACKER_HOME_RSP:		// 1枚受け取りのHome戻し処理
		mode_accpet_uba_stack(1);
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

#if defined(UBA_RTQ)//#if defined(UBA_RS)
	case TMSG_RS_FLAPPER_RSP:
		if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_RC);

			if(!(ex_multi_job.busy))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				//2023-09-12
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else
				{ /* all job normal end */
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_FEED_ESCROW);
					ex_multi_job.busy |= TASK_ST_FEED;
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_ESCROW_REQ, 0, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_RC);
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 139);
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
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 139);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief feed escrow procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void feed_escrow(void) /* 途中でシャッターを締める動作は、電流問題で廃止、識別中にシャッターを締める */
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
#endif // UBA_RTQ
		break;
	case TMSG_FEED_ESCROW_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{

			//CISサンプリング終了
			stop_ad();
			ex_multi_job.busy &= ~TASK_ST_FEED;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			#if defined(UBA_RTQ)		/* '18-06-15 */
				else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
				{
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_OPERATION, _main_conv_seq(), ex_position_sensor);
				}
				else if(ex_rc_error_flag)
				{
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_OPERATION, _main_conv_seq(), ex_position_sensor);
				}
			#endif
				else
				{ /* other job normal end */
					if (is_all_reject_mode())
					{
						if(ex_collection_data.enable)
						{
							ex_collection_data.data_exist = true;
						}
					/* All Reject */
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_INHIBIT, _main_conv_seq(), ex_position_sensor);
					}
					//DataCollection用データ格納
					else if(ex_collection_data.enable)
					{
						/* 外形検知 */
						change_ad_sampling_mode(AD_MODE_MAG_OFF); //2022-06-29
						_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_DISCRIMINATION);
						_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_DATA_COLLECTION_REQ, 0, 0, 0, 0);
						ex_multi_job.busy |= TASK_ST_DISCRIMINATION;

						_pl_cis_enable_set(0);
					}
				#if defined(UBA_RTQ)
					//RTQの無鑑別は、リサイクル先を札長で決める為、識別を動かす必要がある。
					else if( (ex_dipsw1 == DIPSW1_ACCEPT_ALLACC_TEST) && (is_test_mode())) //2025-05-13a ok
					{
						_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_DISCRIMINATION);
						_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_VALIDATION_REQ, 0, 0, 0, 0);
						ex_multi_job.busy |= TASK_ST_DISCRIMINATION;

						/* 識別ありも識別ありと同様に識別完了待ちのモードで、シャッター動作完了を待つ  */
						_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_CLOSE_REQ, 0, 0, 0, 0);
						ex_multi_job.busy |= TASK_ST_SHUTTER; //どこで待たせるか保留なので、フラグ立てないでおく
					}
				#endif	
					else if (is_all_accept_mode())
					{
						/* 識別ありと識別なしでシャッタ動作完了待ちのモードを分けると複雑になるので */
						/* 識別なしの場合も、識別待ちでシャッター動作完了を待つ						*/
						_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_CLOSE_REQ, 0, 0, 0, 0);
						ex_multi_job.busy |= TASK_ST_SHUTTER; //どこで待たせるか保留なので、フラグ立てないでおく
						change_ad_sampling_mode(AD_MODE_MAG_OFF); //2022-06-29
						_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_DISCRIMINATION);
					}
					else
					{
					/* 識別 */
						if( is_test_mode() || ex_2nd_note_uba == 0  || ex_2nd_note_uba == 2 )
						{
							//2024-11-21 1coreなので温度測定が識別タスクより先にメッセージを送る
							_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_TMP, 0, 0, 0);
							ex_multi_job.busy |= TASK_ST_MGU;

							change_ad_sampling_mode(AD_MODE_MAG_OFF); //2022-06-29
							_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_DISCRIMINATION);
							_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_VALIDATION_REQ, 0, 0, 0, 0);
							ex_multi_job.busy |= TASK_ST_DISCRIMINATION;

							/* 識別ありも識別ありと同様に識別完了待ちのモードで、シャッター動作完了を待つ  */
							_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_CLOSE_REQ, 0, 0, 0, 0);
							ex_multi_job.busy |= TASK_ST_SHUTTER; //どこで待たせるか保留なので、フラグ立てないでおく
						}
						else
						{
							_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_ACCEPT_REQ); /* Vendに対するAck待ちへ*/
						}
					}
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			//CISサンプリング終了
			stop_ad();
			ex_multi_job.busy &= ~TASK_ST_FEED;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else
				{ /* other job normal end */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				ex_multi_job.reject |= TASK_ST_FEED;
				ex_multi_job.code[MULTI_FEED] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_FEED] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_FEED] = ex_main_msg.arg4;
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_FEED;
			//CISサンプリング終了
			stop_ad();
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else
				{ /* other job normal end */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				if (!(ex_multi_job.alarm))
				{ /* set alarm inform */
					ex_multi_job.alarm |= TASK_ST_FEED;
					ex_multi_job.code[MULTI_FEED] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_FEED] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_FEED] = ex_main_msg.arg4;
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{
			//CISサンプリング終了
			start_ad();
		}
		else
		{
			//CISサンプリング終了
			stop_ad();
			/* system error ? */
			_main_system_error(0, 140);
		}
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
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 141);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief discrimination procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if defined(UBA_RTQ)
void discrimination_sub_rtq(void)
{
	/* judge rc unit or cash box */
	if(ex_main_test_no == TEST_RC_AGING || ex_main_test_no == TEST_RC_AGING_FACTORY || ex_main_test_no == TEST_ACCEPT_ALLACC)
	{
		_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REQ);
		_main_send_connection_task(TMSG_CONN_ACCEPT, TMSG_SUB_SUCCESS, ex_main_msg.arg2, ex_main_msg.arg3, 0);
	#if (_DEBUG_CIS_MULTI_IMAGE==1)
		_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_IMAGE_INITIALIZE_REQ, 0, 0, 0, 0);
	#endif
		return;
	}
	else
	{
		is_recycle_denomi_check();	/* ここでは、エマージェンシーStopとRefill modeの為にコールしている */

		/* Emergency Stopの場合 */
		if( 1 == ex_main_emergency_flag )
		{
			if( OperationDenomi.unit_bk != OperationDenomi.unit_emergency )
			{
				_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_INHIBIT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				set_recovery_step(RECOVERY_STEP_EMRGENCY_TRANSPORT);
				_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_ACCEPT, TMSG_SUB_SUCCESS, ex_main_msg.arg2, ex_main_msg.arg3, 0);
			}
			return;
		}
		if(OperationDenomi.mode && (OperationDenomi.unit_bk == RC_CASH_BOX))
		{
			_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_INHIBIT, _main_conv_seq(), ex_position_sensor);
			return;
		}
		else
		{
			_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_ACCEPT, TMSG_SUB_SUCCESS, ex_main_msg.arg2, ex_main_msg.arg3, 0);
		#if (_DEBUG_CIS_MULTI_IMAGE==1)
			_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_IMAGE_INITIALIZE_REQ, 0, 0, 0, 0);
		#endif
		}
	}
}
#endif

void discrimination(void)
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
#endif // UBA_RTQ
		break;
	case TMSG_SHUTTER_CLOSE_RSP:
		/* シャッター完了待ちを識別待ちにするか検討 */
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (!(ex_multi_job.busy))
			{
				/* ここにコピーを配置 *//* 2022-03-15 */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else
			#if defined(UBA_RTQ)
				{
					ex_main_msg.arg2 = ex_multi_job.denomi;	//2024-10-21
					ex_main_msg.arg3 = ex_multi_job.direction;	//2024-10-21
					discrimination_sub_rtq();//2024-10-07
				}
			#else
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ACCEPT, TMSG_SUB_SUCCESS, ex_multi_job.denomi, ex_multi_job.direction, 0);
				}
			#endif // UBA_RTQ
			}
			else
			{


			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (!(ex_multi_job.busy))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);

				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				ex_multi_job.alarm |= TASK_ST_SHUTTER;
				ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
		}
		break;

	case TMSG_MGU_READ_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{

			_pl_cis_enable_set(0); //CISの温度計測後なので、消灯する処理 2024-05-28 温度上昇対策,引き抜きのリスクはある

			ex_multi_job.busy &= ~(TASK_ST_MGU);
			if (!(ex_multi_job.busy))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[alarm_index()]);

				}
				else
				{
			#if defined(UBA_RTQ)
					ex_main_msg.arg2 = ex_multi_job.denomi;	//2024-10-21
					ex_main_msg.arg3 = ex_multi_job.direction;	//2024-10-21

					discrimination_sub_rtq(); //2024-10-07
			#else
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ACCEPT, TMSG_SUB_SUCCESS, ex_multi_job.denomi, ex_multi_job.direction, 0);
					#if (_DEBUG_CIS_MULTI_IMAGE==1)
					_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_IMAGE_INITIALIZE_REQ, 0, 0, 0, 0);
					#endif
			#endif // UBA_RTQ
				}
			}
		}
		else
		{
			ex_multi_job.busy &= ~(TASK_ST_MGU);
			if (!(ex_multi_job.busy))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);

				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				if (!(ex_multi_job.alarm))
				{ /* set alarm inform */
					ex_multi_job.alarm |= TASK_ST_MGU;
					ex_multi_job.code[MULTI_MGU] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_MGU] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_MGU] = ex_main_msg.arg4;
				}
			}
		}
		break;
	case TMSG_VALIDATION_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_DISCRIMINATION);
			if (!(ex_multi_job.busy))
			{
				/* ここからの処理をコピー *//* 2022-03-15 */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);

				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if ((_is_main_accept_denomi((u16)ex_main_msg.arg2)) || (ex_main_msg.arg2 == BAR_INDX))
				{
			#if defined(UBA_RTQ)
					discrimination_sub_rtq();
			#else
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_ACCEPT, TMSG_SUB_SUCCESS, ex_main_msg.arg2, ex_main_msg.arg3, 0);
				#if (_DEBUG_CIS_MULTI_IMAGE==1)
					_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_IMAGE_INITIALIZE_REQ, 0, 0, 0, 0);
				#endif
			#endif 	
				}
				else
				{
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_PRECOMP, _main_conv_seq(), ex_position_sensor);

				}
			}
			else
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);

				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if ((_is_main_accept_denomi((u16)ex_main_msg.arg2)) || (ex_main_msg.arg2 == BAR_INDX))
				{
					ex_multi_job.denomi = ex_main_msg.arg2;
					ex_multi_job.direction = ex_main_msg.arg3;
				}
				else
				{
					ex_multi_job.reject = TASK_ST_DISCRIMINATION;
					ex_multi_job.code[MULTI_DISCRIMINATION] = REJECT_CODE_PRECOMP;
					ex_multi_job.sequence[MULTI_DISCRIMINATION] = _main_conv_seq();
					ex_multi_job.sensor[MULTI_DISCRIMINATION] = ex_position_sensor;
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
		{
			ex_multi_job.busy &= ~(TASK_ST_DISCRIMINATION);
			if (!(ex_multi_job.busy))
			{

				_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			else
			{

				ex_multi_job.reject = TASK_ST_DISCRIMINATION;
				ex_multi_job.code[MULTI_DISCRIMINATION] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_DISCRIMINATION] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_DISCRIMINATION] = ex_main_msg.arg4;
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 143);
		}
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
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 144);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief wait request (accept state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void accept_wait_req(void)
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
		_main_set_init();
		break;
	case TMSG_CLINE_STACK_REQ:
	case TMSG_DLINE_STACK_REQ:
		if (ex_multi_job.reject)
		{ /* othrer job reject */
			_main_reject_sub(MODE1_STACK, STACK_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
		}
	#if defined(UBA_RTQ) /* '18-05-01 */
		else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
		{
			_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_OPERATION, _main_conv_seq(), ex_position_sensor);
		}
		else if(ex_rc_error_flag)
		{
			_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_OPERATION, _main_conv_seq(), ex_position_sensor);
		}
	#endif
		else
		{
			ex_main_pause_flag = 0;
#if defined(UBA_RTQ) /* '18-05-01 */

			/* Set stack */
			set_stack_mode(ex_main_msg.arg1);
		//#if (_DEBUG_CIS_AS_A_POSITION == 1)
			/* Enable check cheat */
			_pl_cis_enable_set(1);

			dly_tsk(10); // 2024-05-28

			change_ad_sampling_mode(AD_MODE_VALIDATION_CHECK);
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_CIS_ACTIVE_REQ, 0, 0, 0, 0); // これを呼ばないと機能してない
		//#endif 
			if (ex_operating_mode != OPERATING_MODE_NORMAL)
			{
				/* judge rc unit or cash box */
				if (ex_main_test_no == TEST_RC_AGING ||
					ex_main_test_no == TEST_RC_AGING_FACTORY)
				{
					is_recycle_aging_accept();
				}
				else
				{
					is_recycle_denomi_check();
					OperationDenomi.unit = OperationDenomi.unit_bk;
				}
			}
			else
			{
				/* Get operation denomi */
				is_recycle_denomi_check();
				OperationDenomi.unit = OperationDenomi.unit_bk;
			}

			if (ex_main_msg.arg2 != 0 && OperationDenomi.mode == 0)
			{
				OperationDenomi.unit = RC_CASH_BOX;
				set_recovery_unit(RC_CASH_BOX, RC_CASH_BOX);
			}

			switch (OperationDenomi.unit)
			{
			case RC_TWIN_DRUM1:
			case RC_TWIN_DRUM2:
				/* flapper1 position check */
				if (!(is_flapper1_head_to_twin_pos()))
				{
					flap1_pos = RC_FLAP1_POS_HEAD_TO_RC; /* change postion	*/
				}
				else
				{
					flap1_pos = 0; /* don't move		*/
				}

				/* flapper2 position check */
				flap2_pos = 0; /* don't move		*/

				/* send flapper command */
				if (flap1_pos == 0 && flap2_pos == 0)
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_RC_RSP);
					_main_send_msg(ID_RC_MBX, TMSG_RC_STACK_REQ, OperationDenomi.unit, 0, ex_validation.bill_length, 0);
				}
				else
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_INIT_RC);
					_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
				}
				break;
			case RC_QUAD_DRUM1:
			case RC_QUAD_DRUM2:
				/* flapper1 position check */
				if (!(is_flapper1_head_to_box_pos()))
				{
					flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change postion	*/
				}
				else
				{
					flap1_pos = 0; /* don't move		*/
				}

				/* flapper2 position check */
				if (!(is_flapper2_head_to_quad_pos()))
				{
					flap2_pos = RC_FLAP2_POS_HEAD_TO_RC; /* change postion	*/
				}
				else
				{
					flap2_pos = 0; /* don't move		*/
				}

				/* send flapper command */
				if (flap1_pos == 0 && flap2_pos == 0)
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_RC_RSP);
					_main_send_msg(ID_RC_MBX, TMSG_RC_STACK_REQ, OperationDenomi.unit, 0, ex_validation.bill_length, 0);
				}
				else
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_INIT_RC);
					_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
				}
				break;
			default:
				/* stacker home */
				if (!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
				{
					ex_multi_job.busy |= TASK_ST_STACKER;
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
				}
				else
				{
					/* flapper1 position check */
					if (!(is_flapper1_head_to_box_pos()))
					{
						flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change postion	*/
					}
					else
					{
						flap1_pos = 0; /* don't move		*/
					}

					/* flapper2 position check */
					if (is_quad_model() && !(is_flapper2_head_to_box_pos()))
					{
						flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX; /* change postion	*/
					}
					else
					{
						flap2_pos = 0; /* don't move		*/
					}

					/* send flapper command */
					if (flap1_pos == 0 && flap2_pos == 0)
					{
						_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_RC_RSP);
						_main_send_msg(ID_RC_MBX, TMSG_RC_STACK_REQ, OperationDenomi.unit, 0, ex_validation.bill_length, 0);
					}
					else
					{
						_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_INIT_RC);
						_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
					}
				}
				break;
			}
			
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
#else	//SS
			ex_2nd_note_uba = 0; /* 連続取り込み clear */ /* 通常はここと、Reject,Errorで*/

	#if (DATA_COLLECTION_DEBUG == 1) // 2024-06-17
			_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_DATA_COLLECTION);
			ex_multi_job.busy |= TASK_ST_FEED;
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_APB_REQ, 0, 0, 0, 0);
	#else
			// 2023-09-27
			/* 暗号化有効時は単体動作*/
			if (is_security_mode())
			{
				_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_STACK_LOW);
			}
			else if (is_uba_mode())
			{
				/* DIP-SW 7 ONで強制単体モードの時は、単体モード*/
				_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_STACK_LOW);
			}
			else if (ex_is_uba_mode == 1)
			{
				/* 搬送イニシャルで単体モード判定の時は、単体モード*/
				_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_STACK_LOW);
			}
			// 2024-05-28
			else if (ex_is_cis_high == 1)
			{
				/* 搬送イニシャルで単体モード判定の時は、単体モード*/
				_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_STACK_LOW);
			}
			else
			{
				/* それ以外は並列取り込みの可能性があるので、並列処理へ*/
				_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_STACK);
			}
			// 2024-06-17
		//#if (_DEBUG_CIS_AS_A_POSITION == 1) // ここから監視処理開始
			_pl_cis_enable_set(1);

			dly_tsk(10); // 2024-05-28

			change_ad_sampling_mode(AD_MODE_VALIDATION_CHECK);
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_CIS_ACTIVE_REQ, 0, 0, 0, 0); // これを呼ばないと機能してない
		//#endif
			ex_multi_job.busy |= TASK_ST_FEED;
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_APB_REQ, 0, 0, 0, 0);
	#endif
#endif
		}
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:

		u8 data = 0;

		/* ホスト設定による返却 */
		ex_validation.reject_code = REJECT_CODE_INHIBIT;
		data = get_reject_code_icb(ex_validation.reject_code);
	#if !defined(UBA_RTQ)//RTQはカウントアップのみ
		if (is_icb_enable() && (data != 0) && (ex_validation.start == VALIDATION_STRT))
		{
			if (ex_validation.denomi == BAR_INDX)
			{
				// ticket
				ex_multi_job.busy |= TASK_ST_ICB;
				_main_send_msg(ID_ICB_MBX, TMSG_ICB_REJECT_TICKET_REQ, BAR_INDX, data, 0, 0);
			}
			else
			{
				// bill
				ex_multi_job.busy |= TASK_ST_ICB;
				_main_send_msg(ID_ICB_MBX, TMSG_ICB_REJECT_REQ, ex_validation.denomi, data, 0, 0);
			}
		}
		else if (!(SENSOR_CENTERING_HOME) || !(SENSOR_SHUTTER_OPEN))
	#else
		if (!(SENSOR_CENTERING_HOME) || !(SENSOR_SHUTTER_OPEN))
	#endif
		{
			ex_multi_job.busy = 0;
			if (!(SENSOR_CENTERING_HOME))
			{
				ex_multi_job.busy |= TASK_ST_CENTERING;
				_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
			}
			if (!(SENSOR_SHUTTER_OPEN))
			{
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			}
		}
		else
		{
			ex_multi_job.busy |= TASK_ST_FEED;
			ex_reject_retry_uba = 0;
			_main_reject_sub(MODE1_REJECT, REJECT_MODE2_FEED_REJECT, TMSG_CONN_ACCEPT, REJECT_CODE_INHIBIT, _main_conv_seq(), ex_position_sensor); // 2023-03-09
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
		}
		break;

	case TMSG_ICB_REJECT_RSP:
	case TMSG_ICB_REJECT_TICKET_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_ICB;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				if (!(SENSOR_CENTERING_HOME) || !(SENSOR_SHUTTER_OPEN))
				{
					ex_multi_job.busy = 0;
					if (!(SENSOR_CENTERING_HOME))
					{
						ex_multi_job.busy |= TASK_ST_CENTERING;
						_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
					}
					if (!(SENSOR_SHUTTER_OPEN))
					{
						ex_multi_job.busy |= TASK_ST_SHUTTER;
						_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
					}
				}
				else
				{
					ex_multi_job.busy |= TASK_ST_FEED;
					ex_reject_retry_uba = 0;
					_main_reject_sub(MODE1_REJECT, REJECT_MODE2_FEED_REJECT, TMSG_CONN_ACCEPT, REJECT_CODE_INHIBIT, _main_conv_seq(), ex_position_sensor); // 2023-03-09
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_ICB;
			if (!(ex_multi_job.busy))
			{ /* set alarm inform */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 151);
		}
		break;

	case TMSG_SHUTTER_OPEN_RSP:
		/* シャッター完了待ちを識別待ちにするか検討 */
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else
				{ /* other job normal end */
					ex_multi_job.busy |= TASK_ST_FEED;
					ex_reject_retry_uba = 0;
					_main_reject_sub(MODE1_REJECT, REJECT_MODE2_FEED_REJECT, TMSG_CONN_ACCEPT, REJECT_CODE_INHIBIT, _main_conv_seq(), ex_position_sensor); // 2023-03-09
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
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
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{																																														/* other job alarm */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			}
		}
		break;

	case TMSG_CENTERING_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_CENTERING;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else
				{ /* other job normal end */
					ex_multi_job.busy |= TASK_ST_FEED;
					ex_reject_retry_uba = 0;
					_main_reject_sub(MODE1_REJECT, REJECT_MODE2_FEED_REJECT, TMSG_CONN_ACCEPT, REJECT_CODE_INHIBIT, _main_conv_seq(), ex_position_sensor); // 2023-03-09
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_CENTERING;

			/* テスト用なので、他のタスクを待たないでいきなりエラーにする */
			ex_multi_job.alarm |= TASK_ST_CENTERING;
			ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
			ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
			ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;

			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{																																														/* other job alarm */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			}
		}
		break;

	case TMSG_SENSOR_STATUS_INFO:
		if (SENSOR_CENTERING)
		{
			_main_send_connection_task(TMSG_CONN_ACCEPT, TMSG_SUB_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER, 0, 0);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // uBA_RTQ
		break;
#if defined(UBA_RTQ)

	case	TMSG_STACKER_HOME_RSP: //2024-09-10 
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);

				/* flapper1 position check */
				if(!(is_flapper1_head_to_box_pos()))
				{
					flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX;		/* change postion	*/
				}
				else
				{
					flap1_pos = 0;								/* don't move		*/
				}

				/* flapper2 position check */
				if(!(is_flapper2_head_to_box_pos()))
				{
					flap2_pos = RC_FLAP2_POS_HEAD_TO_BOX;		/* change postion	*/
				}
				else
				{
					flap2_pos = 0;								/* don't move		*/
				}

				/* send flapper command */
				if(flap1_pos == 0 && flap2_pos == 0)
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_RC_RSP);
					_main_send_msg(ID_RC_MBX, TMSG_RC_STACK_REQ, OperationDenomi.unit, 0, ex_validation.bill_length, 0);
				}
				else
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_INIT_RC);
					_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
				}
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else
			{
				_main_system_error(0, 91);
			}
			break;


	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
		//UBA500はここでエラーにしているが、並列処理中なのでエラーにしない
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 146);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief wait reject request procedure (at accept state)
 * @param[in]	None
 * @return 		None
 * 処理の順番は
 * ラインタスクからの返却許可
 * ICB書き込み
 * 幅よせ、シャッターの確認
 * 返却処理へ遷移
 **********************************************************************/
void accept_wait_reject_req(void)
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
		_main_set_init();
		break;
	case TMSG_CLINE_REJECT_REQ:
	case TMSG_DLINE_REJECT_REQ:

		u8 data = 0;
		data = get_reject_code_icb(ex_validation.reject_code);

	#if !defined(UBA_RTQ)//RTQはカウントアップのみ
		if(is_icb_enable() && (data != 0) && (ex_validation.start == VALIDATION_STRT))
		{
			if( ex_validation.denomi == BAR_INDX )
			{
			// ticket
				ex_multi_job.busy |= TASK_ST_ICB;
				_main_send_msg(ID_ICB_MBX, TMSG_ICB_REJECT_TICKET_REQ, BAR_INDX, data,  0, 0);
			}
			else
			{
			// bill
				ex_multi_job.busy |= TASK_ST_ICB;
				_main_send_msg(ID_ICB_MBX, TMSG_ICB_REJECT_REQ, ex_validation.denomi, data, 0, 0);
			}
		}
		else if( !(SENSOR_CENTERING_HOME) || !(SENSOR_SHUTTER_OPEN) )
	#else
		if( !(SENSOR_CENTERING_HOME) || !(SENSOR_SHUTTER_OPEN) )
	#endif
		{			
			ex_multi_job.busy = 0;
			if(!(SENSOR_CENTERING_HOME))
			{	
				ex_multi_job.busy |= TASK_ST_CENTERING;
				_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
			}
			if(!(SENSOR_SHUTTER_OPEN))
			{
				ex_multi_job.busy |= TASK_ST_SHUTTER;
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			}
		}
		else
		{
			ex_multi_job.busy |= TASK_ST_FEED;
			//すでに、一度subをよんでいるので2度目の呼び出しを止める
			ex_reject_retry_uba = 0;
			_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
		}
		break;

	case TMSG_ICB_REJECT_RSP:
	case TMSG_ICB_REJECT_TICKET_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_ICB;
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else
			{
				if ( !(SENSOR_CENTERING_HOME) || !(SENSOR_SHUTTER_OPEN) )
				{			
					ex_multi_job.busy = 0;
					if(!(SENSOR_CENTERING_HOME))
					{	
						ex_multi_job.busy |= TASK_ST_CENTERING;
						_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
					}
					if(!(SENSOR_SHUTTER_OPEN))
					{
						ex_multi_job.busy |= TASK_ST_SHUTTER;
						_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
					}
				}
				else
				{
					ex_multi_job.busy |= TASK_ST_FEED;
					//すでに、一度subをよんでいるので2度目の呼び出しを止める
					ex_reject_retry_uba = 0;	
					_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_ICB;
			if (!(ex_multi_job.busy))
			{ /* set alarm inform */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 151);
		}
		break;

	case TMSG_SHUTTER_OPEN_RSP:
		/* シャッター完了待ちを識別待ちにするか検討 */
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_SHUTTER;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				#if 0	/* ex_multi_job.rejectのFeedのビットは立っているが無視 */
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				#endif
				else
				{ /* other job normal end */
					ex_multi_job.busy |= TASK_ST_FEED;
					ex_reject_retry_uba = 0;
					_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
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
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{ /* other job alarm */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			}
		}
		break;

	case TMSG_CENTERING_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~TASK_ST_CENTERING;
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else if (ex_multi_job.reject)
				{ /* othrer job reject */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
				}
				else
				{ /* other job normal end */
					ex_multi_job.busy |= TASK_ST_FEED;
					ex_reject_retry_uba = 0;
					_main_set_mode(MODE1_REJECT, REJECT_MODE2_FEED_REJECT);
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_REJECT_REQ, FEED_REJECT_OPTION_NORMAL, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~TASK_ST_CENTERING;

			/* テスト用なので、他のタスクを待たないでいきなりエラーにする */
			ex_multi_job.alarm |= TASK_ST_CENTERING;
			ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
			ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
			ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;

			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{ /* other job alarm */
					_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
			}
		}
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
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 147);
		}
		break;
	}
}


void pb_centering_home(void) //取り込み開始前のはばよせHome動作
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
			if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
			}
#if defined(UBA_RTQ)	/* '19-03-18 */
			else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
#endif
			break;

	case	TMSG_CENTERING_HOME_RSP:
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_CENTERING);// 起動タスクにより変更

				if (!(ex_multi_job.busy))
				{ /* all job end */

					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (!(is_box_set()))
					{
						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else
					{
						if (ex_multi_job.alarm & TASK_ST_APB)
						{ /* other job alarm */
							_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[MULTI_APB], ex_multi_job.sequence[MULTI_APB], ex_multi_job.sensor[MULTI_APB]); // MUL_ERROR
						}
						else
						{
							mode_accpet_uba_start_check(); /* 取り込み開始していいかの判断*/
						}
					}
				}
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

				if (!(ex_multi_job.busy))
				{ /* all job end */

					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else
					{
					// スタッカのエラーも発生しているかもしれない、2つのエラーの場合、どちらのエラーを優先させるか検討する
					// 先に発生したエラーを優先させた方がいい様な
						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
//						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[MULTI_CENTERING], ex_multi_job.sequence[MULTI_CENTERING], ex_multi_job.sensor[MULTI_CENTERING]); // MUL_ERROR
					}
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 46);
			}
			break;

	case	TMSG_STACKER_PULL_RSP:	// 連続動作の1枚目戻し処理
			mode_accpet_uba_stack(0);
			break;

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
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 47);
			}
			break;
	}
}

void wait_pb_start(void)
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
			if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
			}
#if defined(UBA_RTQ)	/* '19-03-18 */
			else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
#endif
			break;
	case	TMSG_APB_HOME_RSP:	/* Startが主の処理 */
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				/* 通常PB closeしているので、いきなりsuccessになる事はないが */
				/* いずれ、通常Homeもあり得るので、ここの処理も作成しておく 	*/
				ex_multi_job.busy &= ~(TASK_ST_APB);
				/* ここでは通常マルチはありえない */
				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (!(is_box_set()))
					{
						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else
					{
						if (ex_multi_job.alarm & TASK_ST_CENTERING)
						{ /* other job alarm */
							_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[MULTI_CENTERING], ex_multi_job.sequence[MULTI_CENTERING], ex_multi_job.sensor[MULTI_CENTERING]); // MUL_ERROR
						}
						else
						{
							ex_multi_job.busy |= TASK_ST_FEED;
							_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_FEED_START);
							_main_send_msg(ID_FEED_MBX, TMSG_FEED_CENTERING_REQ, 0, 0, 0, 0);
						}
					}
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{
				/* これが主 */
				ex_multi_job.busy |= TASK_ST_FEED;
				_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_FEED_START);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_CENTERING_REQ, 0, 0, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);

				ex_multi_job.alarm |= TASK_ST_APB;
				ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;

				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else
					{
						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
//						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[MULTI_APB], ex_multi_job.sequence[MULTI_APB], ex_multi_job.sensor[MULTI_APB]); // MUL_ERROR
					}
				}
			}
			break;

	case	TMSG_STACKER_PULL_RSP:	// 連続動作の1枚目戻し処理
			mode_accpet_uba_stack(0);
			break;

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
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 47);
			}
			break;
	}
}


void wait_feed_start(void)	/* PBは動作開始している,搬送開始待ち,収納はまだ動かしてない *//* 連続はむし */
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
			if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
			}
#if defined(UBA_RTQ)	/* '19-03-18 */
			else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
#endif
			break;

	case	TMSG_FEED_CENTERING_RSP:	/* startが主の処理、遷移時にStacker戻し */

			//PROでも使用する、連続動作の兼ね合いがあるので使用方法は検討する
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			// タスク起動前に成功はありえない
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_START)
			{
			//主の処理
			//幅寄せ開始位置までの搬送タスクが起動したので、スタッカをHomeに戻す処理開始
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else{
				// next step
				// 押しメカHome戻し
			#if defined(OLD_UBA_SEQ)		
			//消費電流の関係で収納戻しは、幅よせ中にする、取り込み開始時のHome戻しは廃止 2022-02-18
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_FEED_CENTERING);
					ex_multi_job.busy |= TASK_ST_STACKER;
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
			#else
					//消費電流の関係で収納戻しは、幅よせ中にする、取り込み開始時のHome戻しは廃止 2022-02-18
					//幅よせ位置への搬送中はPBと搬送のみ動かす
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_FEED_CENTERING);
			#endif
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
			{
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{

			// リジェクトの場合の全ての処理が終わるのを待つ必要がある
				ex_multi_job.busy &= ~(TASK_ST_FEED);// 起動タスクにより変更

				ex_multi_job.reject |= TASK_ST_FEED;
				ex_multi_job.code[MULTI_FEED] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_FEED] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_FEED] = ex_main_msg.arg4;

				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}

					else if (ex_multi_job.alarm & TASK_ST_STACKER)
					{ /* other job alarm */
					// ステータスとしてはErrorを優先させる	/* 紙幣返却後、スタッカーエラー	Reject点滅の為　ex_multi_job.code[MULTI_STACKER]は使用できない　*/
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
					}
					else
					{ /* other job normal end */
						// 2017-10-20ここで問題となっているそもそも、Rejectのはず
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
					}
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
			// エラーの場合の全ての処理が終わるのを待つ必要がある
				ex_multi_job.busy &= ~(TASK_ST_FEED);// 起動タスクにより変更

				ex_multi_job.alarm |= TASK_ST_FEED;
				ex_multi_job.code[MULTI_FEED] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_FEED] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_FEED] = ex_main_msg.arg4;

				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else if (ex_multi_job.alarm & TASK_ST_STACKER)
					{ /* other job alarm */
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
					}
					else
					{ /* other job normal end */
						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					}
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 48);
			}
			break;

	case	TMSG_APB_HOME_RSP:	/*  搬送開始が主の処理なので、エラー以外は無視 */
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_APB);

				ex_multi_job.alarm |= TASK_ST_APB;
				ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
				ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
				ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;

				if (!(ex_multi_job.busy))
				{ /* all job end */
					if (ex_main_reset_flag)
					{ /* リセット要求有り */
						_main_set_init();
					}
					else
					{
					#if 1 //2023-09-13
						_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_APB_HOME, ex_multi_job.sequence[MULTI_APB], ex_multi_job.sensor[MULTI_APB]); // MUL_ERROR
					#else	
						_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
					#endif
					}
				}
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{

			}
			break;

	case	TMSG_STACKER_PULL_RSP:		// 連続動作の1枚目戻し処理
			mode_accpet_uba_stack(0);
			break;

#if defined(UBA_RTQ) /* '18-05-01 */
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
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				if (ex_main_reset_flag)
				{ /* リセット要求有り */
					_main_set_init();
				}
				else
				{
					_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 49);
			}
			break;
	}
}

void mode_accpet_uba_stack(u8 run)
{

	if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		if( ex_main_msg.tmsg_code == TMSG_STACKER_PULL_RSP )	// 連続動作の1枚目戻し処理
		{
			set_recovery_step(RECOVERY_STEP_VEND);
			_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_VEND, 0, 0, 0);
			_main_send_connection_task(TMSG_CONN_STACK, TMSG_SUB_SUCCESS, 0, 0, 0);
		}

		ex_multi_job.busy &= ~(TASK_ST_STACKER);
		if (!(ex_multi_job.busy))
		{ /* all job end */
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (ex_multi_job.alarm)
			{ /* other job alarm */
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
			}
			else if (ex_multi_job.reject)
			{ /* othrer job reject */
				_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, ex_multi_job.code[reject_index()], ex_multi_job.sequence[reject_index()], ex_multi_job.sensor[reject_index()]);
			}
			else
			{ /* other job normal end */
				if(run == 1)
				{
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_FEED_ESCROW);
					ex_multi_job.busy |= TASK_ST_FEED;
					_main_send_msg(ID_FEED_MBX, TMSG_FEED_ESCROW_REQ, 0, 0, 0, 0);
				}
			}
		}
	}
	else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
	{
		// エラーの場合の全ての処理が終わるのを待つ必要がある
		ex_multi_job.busy &= ~(TASK_ST_STACKER);

		ex_multi_job.alarm |= TASK_ST_STACKER;
		ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
		ex_multi_job.sequence[MULTI_STACKER]  = ex_main_msg.arg3;
		ex_multi_job.sensor[MULTI_STACKER]  = ex_main_msg.arg4;

		if (!(ex_multi_job.busy))
		{ /* all job end */
			if (ex_main_reset_flag)
			{ /* リセット要求有り */
				_main_set_init();
			}
			else if (!(is_box_set()))
			{ /* box unset */
				_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{ /* other job normal end */
			// ステータスとしてはErrorを優先させる	/* 紙幣返却後、スタッカーエラー	*/
				_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_STACKER_HOME, ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]); // MUL_ERROR
			}
		}
	}
}

void mode_accpet_uba_start_check(void)
{
	u16 bill_in;

	bill_in = _main_bill_in();
	if ((bill_in == BILL_IN_NON)
	|| (bill_in == BILL_IN_ENTRANCE))
	{
		if (SENSOR_APB_HOME)/*連続動作の時に,PB Homeなのに動かしている *//* 電流問題もあるので、動かす必要ない時は動かさない*/
		{
			ex_multi_job.busy |= TASK_ST_FEED;
			_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_FEED_START);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_CENTERING_REQ, 0, 0, 0, 0);
		}
		else
		{
			ex_multi_job.busy |= TASK_ST_APB;
			_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_PB_START);
			_main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);
		}
	}
	else if (bill_in == BILL_IN_ACCEPTOR)
	{
#if defined(UBA_RTQ)
		if(ex_main_emergency_flag == 1) /* Emergency */
		{
			if (SENSOR_APB_HOME) /*連続動作の時に,PB Homeなのに動かしている */ /* 電流問題もあるので、動かす必要ない時は動かさない*/
			{
				ex_multi_job.busy |= TASK_ST_FEED;
				_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_FEED_START);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_CENTERING_REQ, 0, 0, 0, 0);
			}
			else
			{
				ex_multi_job.busy |= TASK_ST_APB;
				_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_PB_START);
				_main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);
			}
		}
		else 
		{
			_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
		}
#else
		_main_reject_sub(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_REJECT_REQ, TMSG_CONN_ACCEPT, REJECT_CODE_ACCEPTOR_STAY_PAPER, _main_conv_seq(), ex_position_sensor);
#endif // UBA_RTQ
	}
	else if (bill_in == BILL_IN_STACKER)
	{
		_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
	}
}

void accept_wait_accept_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		break;

//	case TMSG_CLINE_ENABLE_REQ:
//	case TMSG_DLINE_ENABLE_REQ:

	case TMSG_CLINE_1ST_NOTE_DONE_REQ:

		//2024-11-21 1coreなので温度測定が識別タスクより先にメッセージを送る
		_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_TMP, 0, 0, 0);
		ex_multi_job.busy |= TASK_ST_MGU;

		change_ad_sampling_mode(AD_MODE_MAG_OFF); //2022-06-29
		_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_DISCRIMINATION);
		_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_VALIDATION_REQ, 0, 0, 0, 0);
		ex_multi_job.busy |= TASK_ST_DISCRIMINATION;

		_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_TMP, 0, 0, 0);
		ex_multi_job.busy |= TASK_ST_MGU;

		/* 識別ありも識別ありと同様に識別完了待ちのモードで、シャッター動作完了を待つ  */
		_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_CLOSE_REQ, 0, 0, 0, 0);
		ex_multi_job.busy |= TASK_ST_SHUTTER; //どこで待たせるか保留なので、フラグ立てないでおく

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
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 177);
		}
		break;
	}
}

#if defined(UBA_RTQ)
void accept_init_rc()
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
		else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if(rc_busy_status())
			{
				ex_multi_job.busy |= TASK_ST_RC;
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			}
			else
			{
				if(!(rc_busy_status()) && (ex_multi_job.busy & TASK_ST_RC) != 0)
				{
					ex_multi_job.busy &= ~(TASK_ST_RC);
					_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_WAIT_RC_RSP);
#if 1
					_main_send_msg(ID_RC_MBX, TMSG_RC_STACK_REQ, OperationDenomi.unit, 0, ex_validation.bill_length, 0);
#else
					_main_send_msg(ID_RC_MBX, TMSG_RC_STACK_REQ, OperationDenomi.unit, 0, 0, 0);
#endif
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		} 
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ACCEPT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 177);
		}
		break;
	}
}

void accept_wait_rc_rsp()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_RC_STACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 90);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (rc_busy_status())
			{
			#if 1//2024-12-17
				ex_rc_retry_flg = FALSE;	//2025-02-01
				if( OperationDenomi.unit == RC_CASH_BOX)
				{
					_main_set_mode(MODE1_STACK, STACK_MODE2_RC_FEED_BOX);
				#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)	
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_BACK, ex_validation.denomi, RFID_BACK_ACCEPT, 0); //Accept時-リカバリフラグ有効 2025-07-23
				#endif
				}
				else
				{
					_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_RC_STACK);
				#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_BACK, ex_validation.denomi, RFID_BACK_ACCEPT, 0); //Accept時-リカバリフラグ有効 2025-07-23
				#endif
				}
				ex_multi_job.busy |= TASK_ST_FEED;
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, OperationDenomi.unit, 0, 0, 0);
			#else
				_main_set_mode(MODE1_STACK, STACK_MODE2_FEED_RC_STACK);
				ex_multi_job.busy |= TASK_ST_FEED;
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, OperationDenomi.unit, 0, 0, 0);
			#endif
			}
			else 
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			}
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS) 
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		} 
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_ACCEPT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 177);
		}
		break;
	}
}
#endif // UBA_RTQ

/* EOF */


