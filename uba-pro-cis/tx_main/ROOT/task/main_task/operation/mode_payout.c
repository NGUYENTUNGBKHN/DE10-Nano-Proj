/******************************************************************************/
/*! @addtogroup Group2
    @file       mode_collect.c
    @brief
    @date       2023/12/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/
#if defined(UBA_RTQ)
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

#include "systemdef.h"               //2022-02-17 test
#include "cyclonev_sysmgr_reg_def.h" //2022-02-17 test
#include "hal_gpio_reg.h"            //2022-02-17 test
#include "cyc.h"

#if defined(UBA_RTQ)
#include "if_rc.h"
#include "feed_rc_unit.h"
#endif

#if defined(_PROTOCOL_ENABLE_ID003)
	#include "task/cline_task/003/id003.h"
#endif

#define EXT
#include "com_ram.c"
#include "cis_ram.c"

#define RC_ENCRYPTION
/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

static u8 flap1_pos;
static u8 flap2_pos;

static u8 payout_remain;

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/*********************************************************************/ 
/**
* @brief payout message procedure
* @param[in]	None
* @return 		None
**********************************************************************/
void payout_msg_proc(void)
{
    switch (ex_main_task_mode2)
    {
    case PAYOUT_MODE2_SENSOR_ACTIVE:
        payout_sensor_active(); //ok 払い出し開始の1枚目の1回目のみ入る
        break;
    case PAYOUT_MODE2_INIT_TRANSPORT:
        payout_init_transport(); //ok 払い出しのフラッパ動作開始前に幅よせなどをHomeにしたい場合、リトライ時も使用
        break;
    case PAYOUT_MODE2_INIT_RC:
        payout_init_rc();   //ok 払い出しのたびに常にここに入る1回目、2回目全て、リカバリ成功で次の払い出し時も
        break;
    case PAYOUT_MODE2_WAIT_RC_RSP:
        payout_wait_rc_rsp(); //ok FeedとRCタスクにメッセージ送信済み、RCタスクのメッセージ受信待ち、Feedがサクセスする事はない
        break;
    case PAYOUT_MODE2_FEED_RC_PAYOUT: 
        payout_feed_rc_payout(); //ok 0x0805 紙幣ハンギング位置までの払い出し待ち
        break;
    case PAYOUT_MODE2_NOTE_STAY:
        payout_note_stay(); //ok  0x0807 Pay ValidのAck受信までこの中
        break;
    case PAYOUT_MODE2_FEED_REJECT_STOP_WAIT_WID:
        payout_reject_stop_wait_wid(); //ok  0x0806
        break;
    case PAYOUT_MODE2_WAIT_REQ:
        payout_wait_req(); //ok 払い出し成功時or払い出し失敗でリカバリで収納成功時
        break;

    /* PAYOUT RETRY */
    case PAYOUT_MODE2_WAIT_STACK_START:
        payout_wait_stack_start(); //ok 押し込みの為スタッカタスクの起動待ち
        break;
    case PAYOUT_MODE2_WAIT_STACK_TOP:
        payout_wait_stack_top(); //ok
        break;
    case PAYOUT_MODE2_STACK_HOME:
        payout_wait_stack_home(); //ok
        break;
    case PAYOUT_MODE2_STACK_RETRY:
        payout_exec_retry(); //ok
        break;
    case PAYOUT_MODE2_DUMMY_FEED: //テストモードエージング時のみ、払い出し紙幣ハンギング状態でのRTQ搬送空動作
        payout_dummy_feed(); //ok
        break;
    case PAYOUT_MODE2_RC_PREFEED_STACK:
        payout_rc_retry_prefeed_stack(); //ok　RTQからのメッセージ待ち
        break;
    case PAYOUT_MODE2_RC_RETRY_FWD: //FeedとRCタスクにメッセージ送信済み、背面搬送に紙幣がかからなくなるまで取り込み方向に回す、元のドラムに戻している
        payout_rc_retry_fwd(); //ok
        break;
    case PAYOUT_MODE2_RC_RETRY_REV://FeedとRCタスクにメッセージ送信済み、背面搬送のポジションセンサ1を紙幣が完全に通過するまで、払い出し方法に回す
        payout_rc_retry_rev(); //ok
        break;
    case PAYOUT_MODE2_RC_RETRY_STACK_HOME: //Payout時にエラーとなった場合とPayout時にエラーとなった後搬送リトライ後の2種類あるので紙幣位置まちまち
										   //どちらの場合でも次は回収庫への搬送のはず
        payout_rc_retry_stack_home(); //ok
        break;
    case PAYOUT_MODE2_RC_RETRY_INIT_RC:
        payout_rc_retry_init_rc();	//ok Payout失敗時、紙幣をBOXへ搬送する前にフラッパの位置を整える、この次はBOXへ搬送し、押し込み動作
        break;
    case PAYOUT_MODE2_RC_RETRY_FEED_BOX: //通常の収納と同様、BOXまでの搬送完了待ち FeedとRTQタスク待ち
        payout_rc_retry_feed_box(); //ok
        break;
#if 1//#if defined(ID003_SPECK64)
	case PAYOUT_MODE2_RC_WAIT_PB_CLOSE:
		payout_wait_pb_close();
		break;
#endif
    default:
        /* system error ? */
        _main_system_error(0, 88);
        break;
    }
}

void payout_sensor_active() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        _main_set_init();
        break;
    case TMSG_SENSOR_ACTIVE_RSP:
        ex_multi_job.busy &= ~(TASK_ST_SENSOR);

        //TODO: add cheat check


        if (!(SENSOR_CENTERING_HOME) || !(SENSOR_APB_HOME) || (!SENSOR_SHUTTER_OPEN))
        {
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_TRANSPORT);

            if (!(SENSOR_CENTERING_HOME))
            {
                ex_multi_job.busy |= TASK_ST_CENTERING;
                _main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
            }
            if (!(SENSOR_APB_HOME))
            {
                ex_multi_job.busy |= TASK_ST_APB;
                _main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);
            }
            if (!(SENSOR_SHUTTER_OPEN))
            {
                ex_multi_job.busy |= TASK_ST_SHUTTER;
                _main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
            }
        }
        else
        {
            if (!(is_detect_rc_twin()) || 
                !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
				ex_rc_retry_flg = FALSE; //2024-11-12 new

                switch (OperationDenomi.unit)
                {
                case RC_TWIN_DRUM1:
                case RC_TWIN_DRUM2:
                    /* flappr1 position check */
                    if (!(is_flapper1_head_to_twin_pos()))
                    {
                        flap1_pos = RC_FLAP1_POS_HEAD_TO_RC; /* change position	*/
                    }
                    else
                    {
                        flap1_pos = 0; /* don't move		*/
                    }

                    /* flappr2 position check */
                    flap2_pos = 0; /* don't move		*/

                    /* send flapper command */
                    if (flap1_pos == 0 && flap2_pos == 0)
                    {

                    #if defined(_PROTOCOL_ENABLE_ID003)
						/* 出金紙幣のエスクロコード取得 */
						id003_set_escrow_for_payout( RecycleSettingInfo.DenomiInfo[OperationDenomi.unit-1].PayoutCode );
                    #endif
                        OperationDenomi.pre_feed = is_pre_feed_check();
                        _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);

                        payout_remain = is_payout_remain_check();
                        if(is_rc_rs_unit())
                        {
                            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                        }
                        else
                        {
                            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                        }
                    #if defined(RC_ENCRYPTION)
						_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                    #else
                        _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                    #endif
                    }
                    else
                    {
                        _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                        _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    }
                    break;
                case RC_QUAD_DRUM1:
                case RC_QUAD_DRUM2:
                    /* flappr1 position check */
                    if (!(is_flapper1_head_to_box_pos()))
                    {
                        flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
                    }
                    else
                    {
                        flap1_pos = 0; /* don't move		*/
                    }

                    /* flappr2 position check */
                    if (!(is_flapper2_head_to_quad_pos()))
                    {
                        flap2_pos = RC_FLAP2_POS_HEAD_TO_RC; /* change position	*/
                    }
                    else
                    {
                        flap2_pos = 0; /* don't move		*/
                    }

                    /* send flapper command */
                    if (flap1_pos == 0 && flap2_pos == 0)
                    {

                    #if defined(_PROTOCOL_ENABLE_ID003)
						/* 出金紙幣のエスクロコード取得 */
						id003_set_escrow_for_payout( RecycleSettingInfo.DenomiInfo[OperationDenomi.unit-1].PayoutCode );
                    #endif
                        OperationDenomi.pre_feed = is_pre_feed_check();
                        _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);
                        payout_remain = is_payout_remain_check();
                        if(is_rc_rs_unit())
                        {
                            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                        }
                        else
                        {
                            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                        }
                    #if defined(RC_ENCRYPTION)
						_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                    #else
                        _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                    #endif
                    }
                    else
                    {
                        _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                        _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    }
                    break;
                default:
                    /* system error ? */
                    _main_system_error(0, 89);
                    break;
                }
            }
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
        break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
        break;
    default:
        if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
        {
            /* system error ? */
            _main_system_error(0, 89);
        }
        break;
    }
}

void payout_init_transport() //払い出しのフラッパ動作開始前に幅よせなどをHomeにしたい場合、リトライ時も使用
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_CENTERING_HOME_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy &= ~(TASK_ST_CENTERING);

            if (!(ex_multi_job.busy))
            {
                if (ex_main_reset_flag)
                {
                    _main_set_init();
                }
                else
                {
                    if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
                    {
                        _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
                    }
                    else
                    {
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);

                        switch (OperationDenomi.unit)
                        {
                        case RC_TWIN_DRUM1:
                        case RC_TWIN_DRUM2:
                            /* flappr1 position check */
                            if (!(is_flapper1_head_to_twin_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_HEAD_TO_RC; /* change position	*/
                            }
                            else
                            {
                                flap1_pos = 0; /* don't move		*/
                            }

                            /* flappr2 position check */
                            flap2_pos = 0; /* don't move		*/

                            /* send flapper command */
                            if (flap1_pos == 0 && flap2_pos == 0)
                            {
                                OperationDenomi.pre_feed = is_pre_feed_check();
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);

                                payout_remain = is_payout_remain_check();
                                if(is_rc_rs_unit())
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                                }
                                else
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                                }
                            #if defined(RC_ENCRYPTION)
								_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                            #else
                                _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            #endif
                            }
                            else
                            {
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                            }
                            break;
                        case RC_QUAD_DRUM1:
                        case RC_QUAD_DRUM2:
                            /* flappr1 position check */
                            if (!(is_flapper1_head_to_box_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
                            }
                            else
                            {
                                flap1_pos = 0; /* don't move		*/
                            }

                            /* flappr2 position check */
                            if (!(is_flapper2_head_to_quad_pos()))
                            {
                                flap2_pos = RC_FLAP2_POS_HEAD_TO_RC; /* change position	*/
                            }
                            else
                            {
                                flap2_pos = 0; /* don't move		*/
                            }

                            /* send flapper command */
                            if (flap1_pos == 0 && flap2_pos == 0)
                            {
                                OperationDenomi.pre_feed = is_pre_feed_check();
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);
                                payout_remain = is_payout_remain_check();
                                if(is_rc_rs_unit())
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                                }
                                else
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                                }
                            #if defined(RC_ENCRYPTION)
								_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                            #else
                                _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            #endif
                            }
                            else
                            {
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                            }
                            break;
                        default:
                            /* system error ? */
                            _main_system_error(0, 89);
                            break;
                        }
                    }
                }
            }
            else
            {
                ex_multi_job.reject = TASK_ST_CENTERING;
                ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
                ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
                ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_START)
        {
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_CENTERING);

            if (!(ex_multi_job.busy))
            {
                /* リセット要求有り */
                if (ex_main_reset_flag)
                {
                    _main_set_init();
                }
                else
                {
                    _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
                }
            }
            else
            {
                ex_multi_job.alarm = TASK_ST_CENTERING;
                ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
                ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
                ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 90);
        }
        break;
    case TMSG_APB_HOME_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy &= ~(TASK_ST_APB);

            if (!(ex_multi_job.busy))
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
                        _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
                    }
                    else
                    {
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);

                        switch (OperationDenomi.unit)
                        {
                        case RC_TWIN_DRUM1:
                        case RC_TWIN_DRUM2:
                            /* flappr1 position check */
                            if (!(is_flapper1_head_to_twin_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_HEAD_TO_RC; /* change position	*/
                            }
                            else
                            {
                                flap1_pos = 0; /* don't move		*/
                            }

                            /* flappr2 position check */
                            flap2_pos = 0; /* don't move		*/

                            /* send flapper command */
                            if (flap1_pos == 0 && flap2_pos == 0)
                            {
                                OperationDenomi.pre_feed = is_pre_feed_check();
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);
                                payout_remain = is_payout_remain_check();
                                if(is_rc_rs_unit())
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                                }
                                else
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                                }
                            #if defined(RC_ENCRYPTION)
								_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                            #else
                                _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            #endif
                            }
                            else
                            {
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                            }
                            break;
                        case RC_QUAD_DRUM1:
                        case RC_QUAD_DRUM2:
                            /* flappr1 position check */
                            if (!(is_flapper1_head_to_box_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
                            }
                            else
                            {
                                flap1_pos = 0; /* don't move		*/
                            }

                            /* flappr2 position check */
                            if (!(is_flapper2_head_to_quad_pos()))
                            {
                                flap2_pos = RC_FLAP2_POS_HEAD_TO_RC; /* change position	*/
                            }
                            else
                            {
                                flap2_pos = 0; /* don't move		*/
                            }

                            /* send flapper command */
                            if (flap1_pos == 0 && flap2_pos == 0)
                            {
                                OperationDenomi.pre_feed = is_pre_feed_check();
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);
                                payout_remain = is_payout_remain_check();
                                if(is_rc_rs_unit())
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                                }
                                else
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                                }
                            #if defined(RC_ENCRYPTION)
								_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                            #else
                                _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            #endif
                            }
                            else
                            {
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                            }
                            break;
                        default:
                            /* system error ? */
                            _main_system_error(0, 89);
                            break;
                        }
                    }
                }
            }
            else
            {
                ex_multi_job.reject = TASK_ST_APB;
                ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
                ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
                ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_APB);

            if (!(ex_multi_job.busy))
            {
                /* リセット要求有り */
                if (ex_main_reset_flag)
                {
                    _main_set_init();
                }
                else
                {
                    _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
                }
            }
            else
            {
                ex_multi_job.alarm = TASK_ST_APB;
                ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
                ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
                ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 90);
        }
        break;
    case TMSG_SHUTTER_OPEN_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
             ex_multi_job.busy &= ~(TASK_ST_SHUTTER);

            if (!(ex_multi_job.busy))
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
                        _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
                    }
                    else
                    {
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);

                        switch (OperationDenomi.unit)
                        {
                        case RC_TWIN_DRUM1:
                        case RC_TWIN_DRUM2:
                            /* flappr1 position check */
                            if (!(is_flapper1_head_to_twin_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_HEAD_TO_RC; /* change position	*/
                            }
                            else
                            {
                                flap1_pos = 0; /* don't move		*/
                            }

                            /* flappr2 position check */
                            flap2_pos = 0; /* don't move		*/

                            /* send flapper command */
                            if (flap1_pos == 0 && flap2_pos == 0)
                            {
                                OperationDenomi.pre_feed = is_pre_feed_check();
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);
                                payout_remain = is_payout_remain_check();
                                if(is_rc_rs_unit())
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                                }
                                else
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                                }
                            #if defined(RC_ENCRYPTION)
								_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                            #else
                                _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            #endif
                            }
                            else
                            {
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                            }
                            break;
                        case RC_QUAD_DRUM1:
                        case RC_QUAD_DRUM2:
                            /* flappr1 position check */
                            if (!(is_flapper1_head_to_box_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
                            }
                            else
                            {
                                flap1_pos = 0; /* don't move		*/
                            }

                            /* flappr2 position check */
                            if (!(is_flapper2_head_to_quad_pos()))
                            {
                                flap2_pos = RC_FLAP2_POS_HEAD_TO_RC; /* change position	*/
                            }
                            else
                            {
                                flap2_pos = 0; /* don't move		*/
                            }

                            /* send flapper command */
                            if (flap1_pos == 0 && flap2_pos == 0)
                            {
                                OperationDenomi.pre_feed = is_pre_feed_check();
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);
                                payout_remain = is_payout_remain_check();
                                if(is_rc_rs_unit())
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                                }
                                else
                                {
                                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                                }
                            #if defined(RC_ENCRYPTION)
								_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                            #else
                                _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            #endif
                            }
                            else
                            {
                                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);
                                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                            }
                            break;
                        default:
                            /* system error ? */
                            _main_system_error(0, 89);
                            break;
                        }
                    }
                }
            }
            else
            {
                ex_multi_job.reject = TASK_ST_SHUTTER;
                ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
                ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
                ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_SHUTTER);

            if (!(ex_multi_job.busy))
            {
                /* リセット要求有り */
                if (ex_main_reset_flag)
                {
                    _main_set_init();
                }
                else
                {
                    _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
                }
            }
            else
            {
                ex_multi_job.alarm = TASK_ST_SHUTTER;
                ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
                ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
                ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;
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
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_init_rc() //払い出しのたびに常にここに入る1回目、2回目全て、リカバリ成功で次の払い出し時も
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
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
                ex_multi_job.busy |= TASK_ST_RC;
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
            }
            else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                if (!(rc_busy_status()) && (ex_multi_job.busy & TASK_ST_RC) != 0)
                {
                    ex_multi_job.busy &= ~(TASK_ST_RC);
                    OperationDenomi.pre_feed = is_pre_feed_check();
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_RC_RSP);

                    payout_remain = is_payout_remain_check();
                    if(is_rc_rs_unit())
                    {
                        _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, payout_remain, 0);
                    }
                    else
                    {
                        _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0);
                    }
                #if defined(RC_ENCRYPTION)
					_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0);
                #else
                    _main_send_msg(ID_RC_MBX, TMSG_RC_PAYOUT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                #endif
                }
                else
                {
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
                }
            }
        }
        else if(ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if(rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_wait_rc_rsp()
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_RC_PAYOUT_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy &= ~(TASK_ST_RC);
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_FEED_RC_PAYOUT);
			_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_TMP, 0, 0, 0);	//2025-02-14
		}
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 90);
        }
        break;
    /* 2025/03/12 　feed payout response alarm */
    case TMSG_FEED_RC_PAYOUT_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {

        }
        else if (ex_main_msg.arg1 == TMSG_SUB_FEED_REJECT_RETRY)
        {

        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if (rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_feed_rc_payout()  //ok 0x0805 紙幣ハンギング位置までの払い出し待ち
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        if(ex_main_msg.arg1 == RESET_TYPE_WAIT_PAYVALID_ACK
        || ex_main_msg.arg1 == RESET_TYPE_WAIT_PAYSTAY_POLL)	/* '22-02-15 */
        {
            motor_ctrl_feed_stop();

            _main_set_init();
        }
        else
        {
            ex_main_reset_flag = 1;
        }
        break;
    case TMSG_FEED_RC_PAYOUT_RSP: //step1
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
			//2025-10-22 RSの場合、ポーリング停止している場合、このシーケンスでモータ回りっぱなしになるので
			//このシーケンスにとどまり続ける場合、一定時間経過でモータ停止
			//UBA500の場合、フェールセーフが動作して停止するようだ。
			if (is_rc_rs_unit())
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_SEQ, 1000, 0, 0);	//10s
			}

			if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else if(ex_rc_error_flag != 0)
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
			#if defined(QA_TEST_AZ) || defined(QA_TEST_EMC_EMI)
				//動作インターバル優先なので、無視
				
			#else
				if ((ex_temperature.cis_a >= 60)
				|| (ex_temperature.cis_b >= 60)) //2025-02-14
			   {
				   OSW_TSK_sleep(5000);
			   }
			#endif
				if (ex_main_test_no == TEST_RC_AGING || 
                    ex_main_test_no == TEST_RC_AGING_FACTORY) /* TESTING */
                {
                    if (ex_rc_aging_seq == 0)
                    {
                        _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED1, MOTOR_FWD, 0, 0); //step1-1
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    }
                    else if (ex_rc_aging_seq == 4)
                    {
                        _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED2, MOTOR_FWD, 0, 0); //step1-1
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    }
                    else
                    {
					#if 1//#if defined(ID003_SPECK64)
						#if defined(QA_TEST_EMC_EMI) //EMC_EMIはループバックの確認必要
						if(ex_loopback_error != 0) //2025-04-09
						{
						//通常ではあり得ないエラーにしておく
							_main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_RC_EXCHAGED, _main_conv_seq(), ex_position_sensor);
							return;
						}
						#endif
						if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
						{
							_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_WAIT_PB_CLOSE);
							_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);
						}
						else
						{
							_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
							_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
						}
					#else
						_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
                    #endif
						/* JDL set log*/
                        jdl_rc_payout(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
						jdl_rc_each_count(OperationDenomi.unit, PAYOUT);
                    }
                }
                else            /* NORMAL */
                {
                    if (rc_warning_status())
                    {
                       ex_rc_retry_flg = TRUE; //RTQがエラーだろうと無視して動作させる

                        ex_multi_job.busy |= TASK_ST_RC;

                        /* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
                        _main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
                        _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_PREFEED_STACK); //払い出しは成功したが、RTQはワーニング状態
                        _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETURN_PAYOUT_NOTE, 0, 0, 0);
                    }
                    else
                    {
						/* PAYSTAYは1回ポーリングを待つ */
						_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_PAYSTAY, 0, 0, 0);
                    }
                }
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_FEED_REJECT_RETRY)
        {
            ex_multi_job.busy |= TASK_ST_CENTERING;
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_FEED_REJECT_STOP_WAIT_WID);
            _main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 0, 0, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {

            if ((!rc_busy_status()) &&                                                                                              /* NOT busy */
            (ex_main_msg.arg2 == ALARM_CODE_FEED_TIMEOUT_SK) &&                                                                     /* TIMEOUT */
            ((ex_recovery_info.step == RECOVERY_STEP_PAYOUT_TRANSPORT) || (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_POS1)))    /* RECOVERY step */
            {
                if ((OperationDenomi.unit == RC_QUAD_DRUM1 || OperationDenomi.unit == RC_QUAD_DRUM2) && 
                    is_quad_model() && !(RC_POS3_ON))
                {

                    set_recovery_step(RECOVERY_STEP_SWITCHBACK_TRANSPORT);
                    set_recovery_unit(RC_CASH_BOX, ex_recovery_info.unit);
                    _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETURN_PAYOUT_NOTE, 0, 0, 0);

                    ex_rc_retry_flg = TRUE; //RTQがエラーだろうと無視して動作させる

                    /* stacker home */
                    if (!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
                    {
                        ex_multi_job.busy |= TASK_ST_STACKER;
                        _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_STACK_HOME);
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
                            ex_multi_job.busy |= TASK_ST_RC;

                            /* change box from rc to cash box */
                            OperationDenomi.unit_retry = OperationDenomi.unit;
                            OperationDenomi.unit = RC_CASH_BOX;

                            _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0); //step2-1
                        }
                        else
                        {
                        //Payout失敗時、紙幣をBOXへ搬送する前にフラッパの位置を整える、この次はBOXへ搬送し、押し込み動作 
						    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_INIT_RC);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                        }
                    }
                }
                else
                {
                    ex_rc_retry_flg = TRUE; //RTQがエラーだろうと無視して動作させる

                    ex_multi_job.busy |= TASK_ST_RC;
                    ex_multi_job.busy |= TASK_ST_FEED;

                    _main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, OperationDenomi.unit, RC_RETRY_STACK_DIR, 0, 0);
					//リトライとしての正転、次のシーケンスで逆転させる
					//背面搬送で紙幣が検知できなくなるまで、取り込み方法に回す,元のドラムの方に戻している
					//背面搬送で紙幣が検知できなくなるまで、取り込み方法に回す,イニシャル時と、Payout時に意味合いが異なる可能性あり フラッパ動作させてない場合は、おそらくドラムの方へ行く
                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, OperationDenomi.unit, 0, 0, 0);
                    _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETURN_PAYOUT_NOTE, 0, 0, 0);

                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_FWD);
                }
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 90);
        }
        break;
    case TMSG_CLINE_PAYSTAY_REQ: //step2    //2024-11-06 合わせこみ
        /* PAYSTAYステータス1回送信済み */
        if(((ex_cline_status_tbl.option & ID003_OPTION_CHANGE_PAYVALID_TIMING) == ID003_OPTION_CHANGE_PAYVALID_TIMING)
        || ((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE)
        || (is_rc_rs_unit())
        )		/* '21-10-11 */
        {
            /* CHANGE PAY VALID TIMINGの時はPAYVALID送信 */
			set_recovery_step(RECOVERY_STEP_PAYOUT_VALID);
			_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_PAYVALID, 0, 0, 0);
        }
        else
        {
			/* 通常の時は紙幣取り除き待ちへ */
			_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_NOTE_STAY);
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);

			//2025-02-14
			_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
			//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
			dly_tsk(5);
			_main_set_pl_active(PL_DISABLE);
 
		}
        break;
	#if 1
	case	TMSG_CLINE_PAYVALID_REQ: //step3   //2024-11-06 合わせこみ
			//spray modeかPay validタイミングを変えている場合のみ
			/* spray mode *//* '21-10-11 */
            if ((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE
                || (is_rc_rs_unit())
            )
            {
			// spray mode の場合は、次の払い出しの準備
			#if 1//#if defined(ID003_SPECK64)
				if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
				{
					_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_WAIT_PB_CLOSE);
                	_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);	
				}
				else
				{
					_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
				}                
            #else
				/* 紙幣無しの時は次に */
				_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
            #endif
                if(OperationDenomi.remain == 0)
                {
                    /* 2025/03/10 On Spray mode, the motor will continuously move in reverse. 
                    if order remain bill is 0, motor will be stopped. */
					//UBA500もメッセージは送信しているが、Main側での何も処理を受けていない
                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_STOP_REQ, 0, 0, 0, 0);
                }
			}
			else
			{
				if((ex_cline_status_tbl.option & ID003_OPTION_CHANGE_PAYVALID_TIMING) == ID003_OPTION_CHANGE_PAYVALID_TIMING)
				{
				//通常これ以外あり得ない	
					_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_NOTE_STAY);
					_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);

					//2025-02-14
					_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
					//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
					dly_tsk(5);
					_main_set_pl_active(PL_DISABLE);
 
				}
			}
			break;
	#endif
	// TMSG_RC_FEED_BOX_REQ
    case TMSG_RC_FEED_BOX_RSP://エラーの時のリカバリ step2-1  //2024-11-06 合わせこみ
        if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
		//RTQ側はモータ動作開始している
            ex_multi_job.busy |= TASK_ST_FEED;
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_FEED_BOX);
            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, PAYREJECT, 0, 0);
        }
        else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
	//TMSG_RC_FEED_REQ
    case TMSG_RC_FEED_RSP: //このシーケンス内のテストモードエージング時のみ //step1-1
	//払い出し紙幣をハンギング状態で、エージング動作としてRTQの搬送動作命令済み
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            if (ex_main_test_no == TEST_STANDBY)
            {
                _main_set_test_standby();
            }
            else
            {
                if (ex_rc_error_flag != 0)
                {
                    _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
                }
                else
                {
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_DUMMY_FEED);
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 100, 0, 0);
                }
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            /* RC-Twin/Quad error */
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 125);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            if (!(is_detect_rc_twin()) || 
                !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
		else if(ex_main_msg.arg1 == TIMER_ID_SEQ)	//2025-10-22
		{
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_STOP_REQ, 0, 0, 0, 0); //このシーケンスにとどまり続ける場合
		}
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_note_stay() //ok　Pay ValidのAck受信までこの中
{
	u16 bill_in;
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        _main_set_init();
        break;
	case TMSG_SENSOR_STATUS_INFO:	//step1
	//2025-03-26 case TMSG_SENSOR_CIS_ACTIVE_RSP:  //set_led_payout内でセンサアクティブとCISアクティブしている、このにCISを入れると、
	//呼び出しがループするので、廃止
		bill_in = _main_bill_in();
		if (bill_in == BILL_IN_NON)
		{
			set_led_payout();
		}
		else if(bill_in == BILL_IN_STACKER)
		{
	//	UBA500とは変えて、この処理を無効にする紙幣ハンギング時、CISはOFFなのでUBA500と異なる
	//  ハンギング状態でHead開くと、識別センサ以外では、PB IN, OUTが幅よせセンサより先にOFFになり、
	//	結果、UBA500とは異なり、Stacker JAMとなる為、この処理を無効にした		
	//2025-05-15		_main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
		}
		break;
	case TMSG_SENSOR_ACTIVE_RSP:	//2025-02-14 step2
		ex_multi_job.busy &= ~(TASK_ST_SENSOR);

		bill_in = _main_bill_in();
        if (bill_in == BILL_IN_NON)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else
            {
                if(((ex_cline_status_tbl.option & ID003_OPTION_CHANGE_PAYVALID_TIMING) == ID003_OPTION_CHANGE_PAYVALID_TIMING)
                || ((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE)
				|| is_rc_rs_unit()	
				)		/* '21-10-11 */
                {
                    /* 紙幣無しの時は次に */
                    if(OperationDenomi.remain == 0)
                    {
                        _main_send_msg(ID_FEED_MBX, TMSG_ENTRY_BACK_REQ, 0, 0, 0, 0);
                    }
                    else
                    {
					#if 1//#if defined(ID003_SPECK64)
						if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
						{
							_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_WAIT_PB_CLOSE);
							_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);
						}
						else
						{
							_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
							_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
						}                        
					#else
						_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
					#endif
					}
                }
                else
                {
					/* 通常の場合は紙幣取除きでPAYVALID送信 */
					set_recovery_step(RECOVERY_STEP_PAYOUT_VALID);
					_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_PAYVALID, 0, 0, 0);
                }
                jdl_rc_payout(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
                jdl_rc_each_count(OperationDenomi.unit, PAYOUT);
            }   
        }
        else if(bill_in == BILL_IN_STACKER)
        {
		//2025-05-15 こっちは、一度、BILL_IN_NON 後なのでチェック残す
			_main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
        }
        break;
	#if 1
	case	TMSG_CLINE_PAYVALID_REQ: //2024-11-06 合わせこみ //Pay validに対してAck受信した step3
            if(((ex_cline_status_tbl.option & ID003_OPTION_CHANGE_PAYVALID_TIMING) == ID003_OPTION_CHANGE_PAYVALID_TIMING)
			|| ((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE))		/* '21-10-11 */
			{
				/* None */
				/* このに入る事はあり得ない、*/
			}
			else
			{
				if(OperationDenomi.remain == 0)
				{
					_main_send_msg(ID_FEED_MBX, TMSG_ENTRY_BACK_REQ, 0, 0, 0, 0);
				}
				else
				{
				#if 1//#if defined(ID003_SPECK64)
					if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
					{
						_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_WAIT_PB_CLOSE);
				   		_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);
					}
					else
					{
						_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
						_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
					}				    
                #else
					_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
                #endif
				}
			}
			break;
	#endif
	case	TMSG_ENTRY_BACK_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
			#if 1//#if defined(ID003_SPECK64)
				if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
				{
					_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_WAIT_PB_CLOSE);
                	_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);
				}
				else
				{
					_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
					_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
				}
                
			#else
				_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
			#endif
			}
			else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				/* RC-Twin/Quad error */
				_main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 125);
			}
			break;

    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
        break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
        break;
    case TMSG_CLINE_EMERGENCY_REQ:
        ex_main_emergency_flag = 1;
        _main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
        _main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_ACCEPT, 0, 0, 0);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_reject_stop_wait_wid() //ok
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
        break;
    case TMSG_CENTERING_EXEC_RSP:
        ex_multi_job.busy &= ~(TASK_ST_CENTERING); // 起動タスクにより変更

        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            // next step 幅よせHome完了 */
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_FEED_RC_PAYOUT);
            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_PAYOUT_REQ, OperationDenomi.unit, FEED_PAYOUT_OPTION_RETRY, 0, 0);
		}
        else if (ex_main_msg.arg1 == TMSG_SUB_START_TASK)
        {
            // 幅寄せタスクが動作開始
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.alarm |= TASK_ST_CENTERING;
            ex_multi_job.code[MULTI_CENTERING] = ex_main_msg.arg2;
            ex_multi_job.sequence[MULTI_CENTERING] = ex_main_msg.arg3;
            ex_multi_job.sensor[MULTI_CENTERING] = ex_main_msg.arg4;
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
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
    default:
        if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
        {
            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_wait_req() //ok 払い出し成功時or払い出し失敗でリカバリで収納成功時
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        _main_set_init();
        break;
    case TMSG_CLINE_DISABLE_REQ:
	case TMSG_DLINE_DISABLE_REQ:
		if(is_rc_rs_unit())
		{
			/* Set timer */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RS_CONTROL_LED, WAIT_TIME_CHECK_REMAIN, 0, 0);			// LED
		}
		_main_set_disable();
		break;
    case TMSG_CLINE_ENABLE_REQ:
	case TMSG_DLINE_ENABLE_REQ:
        if (ex_main_test_no == TEST_RC_AGING || 
            ex_main_test_no == TEST_RC_AGING_FACTORY)
        {
			//2025-05-07
			_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
			//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
			dly_tsk(5);
			_main_set_pl_active(PL_DISABLE);	

            if(ex_rc_configuration.unit_type_bk == RS_CONNECT)
            {
                ex_multi_job.busy |= TASK_ST_RC;
				_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, RS_FLAP_POS_OUT, 0, 0, 0);
            }
            else 
            {
                if (ex_rc_aging_seq == 4 || 
                    ex_rc_aging_seq == 0)
                {
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 200, 0, 0);
                }
                else
                {
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 200, 0, 0);
                }
            }
		}
		else
		{
			if(is_rc_rs_unit())
			{
					/* Set timer */
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RS_CONTROL_LED, WAIT_TIME_CHECK_REMAIN, 0, 0);			// LED
			}
			_main_set_enable();
		}
		break;

    case TMSG_CLINE_PAYOUT_REQ:
	case TMSG_DLINE_PAYOUT_REQ:

        // リトライ動作時にOperationDenomi.unitが変更されるため、ここで元に戻す
        if(OperationDenomi.unit == RC_CASH_BOX && ex_rc_retry_flg == true)
        {
            OperationDenomi.unit = OperationDenomi.unit_retry;
        }
		ex_rc_retry_flg = FALSE; //2024-11-12 new

        if ((OperationDenomi.unit == RC_TWIN_DRUM1 && is_rc_twin_d1_empty()) ||
            (OperationDenomi.unit == RC_TWIN_DRUM2 && is_rc_twin_d2_empty()) ||
            (OperationDenomi.unit == RC_QUAD_DRUM1 && is_rc_quad_d1_empty()) ||
            (OperationDenomi.unit == RC_QUAD_DRUM2 && is_rc_quad_d2_empty()))
        {
            _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
            _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);
        }
        else
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);

            if (!(SENSOR_CENTERING_HOME) || 
                !(SENSOR_APB_HOME))
            {
                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_TRANSPORT);

                if (!(SENSOR_CENTERING_HOME))
                {
                    ex_multi_job.busy |= TASK_ST_CENTERING;
                    _main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
                }
                if (!(SENSOR_APB_HOME))
                {
                    ex_multi_job.busy |= TASK_ST_APB;
                    _main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);
                }
            }
            else
            {
                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_INIT_RC);

                switch (OperationDenomi.unit)
                {
                case RC_TWIN_DRUM1:
                case RC_TWIN_DRUM2:
                    /* flappr1 position check */
                    if (!(is_flapper1_head_to_twin_pos()))
                    {
                        flap1_pos = RC_FLAP1_POS_HEAD_TO_RC; /* change position	*/
                    }
                    else
                    {
                        flap1_pos = 0; /* don't move		*/
                    }

                    /* flappr2 position check */
                    flap2_pos = 0; /* don't move		*/

                    /* send flapper command */
                    _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    break;
                case RC_QUAD_DRUM1:
                case RC_QUAD_DRUM2:
                    /* flappr1 position check */
                    if (!(is_flapper1_head_to_box_pos()))
                    {
                        flap1_pos = RC_FLAP1_POS_HEAD_TO_BOX; /* change position	*/
                    }
                    else
                    {
                        flap1_pos = 0; /* don't move		*/
                    }

                    /* flappr2 position check */
                    if (!(is_flapper2_head_to_quad_pos()))
                    {
                        flap2_pos = RC_FLAP2_POS_HEAD_TO_RC; /* change position	*/
                    }
                    else
                    {
                        flap2_pos = 0; /* don't move		*/
                    }

                    /* send flapper command */
                    _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    break;
                default:
                    /* system error ? */
                    _main_system_error(0, 89);
                    break;
                }
            }
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
        {
            if (ex_main_test_no == TEST_RC_AGING || 
                ex_main_test_no == TEST_RC_AGING_FACTORY)
            {
                _main_set_enable();
            }
        }
        break;
//#if defined(UBA_RS)
    case TMSG_RS_FLAPPER_RSP:
        ex_multi_job.busy &= ~(TASK_ST_RC);

        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            if (ex_rc_aging_seq == 4 || ex_rc_aging_seq == 0)
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 200, 0, 0);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 200, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 89);
        }
        break;
//#endif // uBA_RS
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
    default:
        if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
        {
            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_wait_stack_start() //ok 押し込みの為スタッカタスクの起動待ち
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_STACKER_EXEC_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_START)
        {
            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else
            {
                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_STACK_TOP);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 75);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_wait_stack_top() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        _main_set_init();
        break;
    case TMSG_STACKER_EXEC_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);

            // SUBの押しメカがエラーが発生した可能性があるので確認する
            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                if (ex_main_reset_flag)
                { /* リセット要求有り */
                    _main_set_init();
                }
                else if (!(is_box_set()))
                {
                    _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
                }
                else
                {
               #if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20
				//紙ありエージングで押し込み時にRFID通信を行う
				//書き込みの場合、RTQ側のLEDが紫に点滅、書き込みにする
				//読み込みの場合、RTQ側のLEDは点滅しないようだ
					rc_rfid_write_test();
					_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_WRITE_REQ, RFID_RUN, 256, 8, 0);	
			 	//	_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_READ_REQ, RFID_RUN, 0, 8, 0);
				#else
                    ex_multi_job.busy |= TASK_ST_STACKER;
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_STACK_HOME);
                    _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);
                #endif
                }
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
        {
            // 押し込みの頂点時にすでにリトライが確定している場合は、Retryメッセージを受信するようにする
            ex_multi_job.busy &= ~(TASK_ST_STACKER);

            // SUBの押しメカがエラーが発生した可能性があるので確認する
            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else if (!(is_box_set()))
            { /* box unset */
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                if (ex_main_reset_flag)
                { /* リセット要求有り */
                    _main_set_init();
                }
                else if (!(is_box_set()))
                {
                    _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
                }
                else
                {
                    // 押し込みリトライ処理へ
                    ex_multi_job.busy |= TASK_ST_STACKER;
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_STACK_RETRY);
                    _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
                }
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(!(is_detect_rc_twin()) ||    /* detect twin box */
				!(is_detect_rc_quad()))		 /* detect quad box */
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
		}
        break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
        break;

#if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20
    //case TMSG_RC_RFID_READ_RSP:
    case TMSG_RC_RFID_WRITE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy |= TASK_ST_STACKER;
			_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_STACK_HOME);
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_wait_stack_home() //OK
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        _main_set_init();
        break;
    case TMSG_STACKER_PULL_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_ENABLE_NEXT) // 次の紙幣取り込み許可命令
        {
            /* None */
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS) // 収納成功
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);
			ex_rc_internal_jam_flag = 0;

            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                if (ex_main_test_no == TEST_RC_AGING || 
                    ex_main_test_no == TEST_RC_AGING_FACTORY)
                {
                //    _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_PAYVALID, 0, 0, 0);
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);

                    // RCのリトライ動作時
                    if (ex_rc_retry_flg == TRUE) //エージング時なので必要ないかも
                    {
                        _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETRY_REQUEST, 0, 0, 0);
                    }
                    else
                    {
                        _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);
                    }
                    /* JDL set log */
                    jdl_rc_payout(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
                    jdl_rc_each_count(OperationDenomi.unit, PAYOUT);
                }
                else
                {
					//step1
                    _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETURN_PAYOUT_COLLECTED, 0, 0, 0);//名前がよくないが、Payout紙幣の払い出し失敗で収納成功
                }
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 226);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
			{
				ex_rc_error_flag = ex_main_msg.arg2;
			}
			break;

	case	TMSG_LINE_PAYOUT_RETURNED_REQ: //2024-11-06 合わせこみ //名前がよくないが、Payout紙幣の払い出し失敗で収納成功
			//step2
			//安高さんからのアドバイスID-003モードではいらない可能性が高い、テストモードでは必要かも _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_PAYVALID, 0, 0, 0);

			_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
			if(++ex_rc_retry_count > 2)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TRANSPORT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETRY_REQUEST, 0, 0, 0);
			}

            #if 1//#ifdef _ENABLE_JDL
            jdl_rc_payout(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
			jdl_rc_each_count(OperationDenomi.unit_retry, PAYREJECT);
            #endif /* _ENABLE_JDL */
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_exec_retry() //ok
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
    case TMSG_STACKER_EXEC_NG_PULL_RSP: // Setp1  1度目の押し込みでNG、押しメカ戻し動作
        ex_multi_job.busy &= ~(TASK_ST_STACKER);

        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                // 戻し動作は成功したので、モードはこのままで押し込みリトライ命令を行う
                ex_multi_job.busy |= TASK_ST_STACKER;                                 // 起動タスクにより変更
                _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, 0, 0, 0, 0); // リトライ用の押し込み命令へ
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 229);
        }
        break;
    case TMSG_STACKER_EXEC_RE_RSP: // Setp2  1度目の押し込みでNG、押しメカ戻し動作
        ex_multi_job.busy &= ~(TASK_ST_STACKER);

        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            if (ex_main_reset_flag)
            { /* リセット要求有り */
                _main_set_init();
            }
            else if (ex_rc_error_flag)
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
			//step1
                _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETURN_PAYOUT_COLLECTED, 0, 0, 0); //名前がよくないが、Payout紙幣の払い出し失敗で収納成功
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 230);
        }
        break;

	case	TMSG_LINE_PAYOUT_RETURNED_REQ: //2024-11-06 合わせこみ //名前がよくないが、Payout紙幣の払い出し失敗で収納成功
			//step2
			// RCのリトライ動作時
			_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
			if(++ex_rc_retry_count > 2)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TRANSPORT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_RETRY_REQUEST, 0, 0, 0);
			}

        #if 1//#ifdef _ENABLE_JDL
            jdl_rc_payout(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
			jdl_rc_each_count(OperationDenomi.unit_retry, PAYREJECT);
        #endif /* _ENABLE_JDL */
			break;

    case TMSG_RC_STATUS_INFO:
        if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
        {
            ex_rc_error_flag = ex_main_msg.arg2;
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
 * @brief Dummy feed : actives when aging mode and aging factory 
 *                      mode is on (payout state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void payout_dummy_feed() //ok //テストモードエージング時のみ、払い出し紙幣ハンギング状態でのRTQ搬送空動作
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        _main_set_init();
        break;
    case TMSG_RC_FEED_RSP: //step2
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
		//RTQの搬送停止命令成功
            if (ex_main_test_no == TEST_STANDBY)
            {
                _main_set_test_standby();
            }
            else
            {
			//空での収納動作
                ex_multi_job.busy |= TASK_ST_STACKER; //起動タスクにより変更
                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_STACK_START);
                _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
				#if defined(UBA_RTQ_ICB) //2025-03-25 2025-05-09
				if(is_icb_enable())
				{
					_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI_UNIT, OperationDenomi.unit_retry, 0, 0); //Payout失敗での回収庫
				}
				#endif
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            /* RC-Twin/Quad error */
            _main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 125);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK) //step1
        {
		//RTQ側の搬送を100msec動作させたので停止
            if (ex_rc_aging_seq == 0)
            {
                _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED1, MOTOR_STOP, 0, 0);
            }
            else if (ex_rc_aging_seq == 4)
            {
                _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED2, MOTOR_STOP, 0, 0);
            }
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
 * @brief rc retry (payout state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void payout_rc_retry_prefeed_stack() //ok RTQからのメッセージ待ち
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
		//step1
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            /* RC-Twin/Quad error */
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if (rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
		//step2
            if (rc_warning_status())
            {
                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & TASK_ST_RC) == 0)
                {
                    ex_multi_job.busy |= TASK_ST_RC;

                    /* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
                    _main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_PREFEED_STACK); //シーケンスは遷移していない
                }
                else
                {
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
                }
            }
            else if (!(rc_busy_status()))
            {
                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & TASK_ST_RC) == 0)
                {
                    ex_multi_job.busy |= TASK_ST_RC;
                    ex_multi_job.busy |= TASK_ST_FEED;

                    _main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, OperationDenomi.unit, RC_RETRY_STACK_DIR, 0, 0);
					//リトライとしての正転、次のシーケンスで逆転させる
					//背面搬送で紙幣が検知できなくなるまで、取り込み方法に回す,元のドラムの方に戻している
					//背面搬送で紙幣が検知できなくなるまで、取り込み方法に回す,イニシャル時と、Payout時に意味合いが異なる可能性あり フラッパ動作させてない場合は、おそらくドラムの方へ行く
                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_STACK_REQ, OperationDenomi.unit, 0, 0, 0);

                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_FWD);
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
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_rc_retry_fwd() //ok FeedとRCタスクにメッセージ送信済み、 背面搬送に紙幣がかからなくなるまで取り込み方向に回す、元のドラムの方に戻している
//ここではFeedタスクとRCへのメッセージ送信完了の2つを待つ
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
        if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
            if (rc_warning_status())
            {

                ex_multi_job.busy &= ~(TASK_ST_RC);
				//FeedとRTQの両方の完了待ち step2
                if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
                {
                    ex_multi_job.busy |= TASK_ST_RC;
                    ex_multi_job.busy |= TASK_ST_FEED;

                    _main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, OperationDenomi.unit, RC_RETRY_PAYOUT_DIR, 0, 0);
					//背面搬送のポジションセンサ1を紙幣が完全に通過するまで、払い出し方法に回す
                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0); //ワーニング発生中、payout失敗で元のドラムに戻し中にワーニング発生

                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_REV);
                }
                else
                {
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
                }
            }
            else if (!(rc_busy_status()))
            {
                ex_multi_job.busy &= ~(TASK_ST_RC);
				//FeedとRTQの両方の完了待ち step2
                if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
                {
                    ex_multi_job.busy |= TASK_ST_RC;
                    ex_multi_job.busy |= TASK_ST_FEED;

                    _main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, OperationDenomi.unit, RC_RETRY_PAYOUT_DIR, 0, 0);
					//背面搬送のポジションセンサ1を紙幣が完全に通過するまで、払い出し方法に回す
                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, OperationDenomi.unit, 0, 0, 0); //ワーニング未発生、payout失敗で元のドラム方向に戻し成功

                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_REV);
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
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
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
		//step1
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_FEED_RC_FORCE_STACK_RSP: //背面搬送に紙幣がかからなくなるまで取り込み方向に回す、元のドラム方向回す 
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
		//Feed完了 step1 背面搬送に紙幣がかからなくなるまで取り込み方向に回す、元のドラム方向回す 
            ex_multi_job.busy &= ~(TASK_ST_FEED);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_FEED);
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_rc_retry_rev() //ok 背面搬送のポジションセンサ1を紙幣が完全に通過するまで、払い出し方法に回す
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
		//FeedとRTQの完了待ち、成功時は紙幣はHead付近まで戻っている
            if (rc_warning_status())
            {

                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
                {
                    ex_multi_job.busy |= TASK_ST_RC;

                    /* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
                    _main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_PREFEED_STACK);
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
                    /* stacker home */
                    if (!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
                    {
                        ex_multi_job.busy |= TASK_ST_STACKER;
                        _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_STACK_HOME);
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
                            OperationDenomi.unit_retry = OperationDenomi.unit;
                            OperationDenomi.unit = RC_CASH_BOX;

                            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_FEED_BOX);
                            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, COLLECT, 0, 0);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
                        }
                        else
                        {
						//Payout失敗時、紙幣をBOXへ搬送する前にフラッパの位置を整える、この次はBOXへ搬送し、押し込み動作	
                            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_INIT_RC);
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
            if (rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
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
		//step1
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_FEED_RC_FORCE_PAYOUT_RSP:
        ex_multi_job.busy &= ~(TASK_ST_FEED);
        //
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
		//step1 背面搬送のポジションセンサ1を紙幣が完全に通過するまで、払い出し方法に回す
            ex_multi_job.busy &= ~(TASK_ST_FEED);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_FEED);
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_rc_retry_stack_home() //ok //Payout時にエラーとなった場合とPayout時にエラーとなった後搬送リトライ後の2種類あるので紙幣位置まちまち
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
        else if(ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if(rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
            }
        }
        break;
    case TMSG_STACKER_HOME_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
		//step1	
            ex_multi_job.busy &= ~(TASK_ST_STACKER);

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
                ex_multi_job.busy |= TASK_ST_RC;

                /* change box from rc to cash box */
                OperationDenomi.unit_retry = OperationDenomi.unit;
                OperationDenomi.unit = RC_CASH_BOX;

                _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
            }
            else
            {
			//Payout失敗時、紙幣をBOXへ搬送する前にフラッパの位置を整える、この次はBOXへ搬送し、押し込み動作	
                _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_INIT_RC);
                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else
        {
            _main_system_error(0, 91);
        }
        break;
    case TMSG_RC_FEED_BOX_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
		//step2 回収庫への搬送の為、RTQへの命令完了
            ex_multi_job.busy |= TASK_ST_FEED;
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_FEED_BOX);
            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, PAYREJECT, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_rc_retry_init_rc() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_RC_FLAPPER_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
        else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
            if (rc_busy_status())
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
            }
            else
            {
                ex_multi_job.busy |= TASK_ST_RC;

                /* change box from rc to cash box */
                OperationDenomi.unit_retry = OperationDenomi.unit;
                OperationDenomi.unit = RC_CASH_BOX;

                _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, RC_CASH_BOX, 0, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if (rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
            }
        }
        break;
    case TMSG_RC_FEED_BOX_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy |= TASK_ST_FEED;
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_FEED_BOX);
            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_STACK_REQ, RC_CASH_BOX, PAYREJECT, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void payout_rc_retry_feed_box() //ok //通常の収納と同様、BOXまでの搬送完了待ち FeedとRTQタスク待ち
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
		//step1	
            ex_multi_job.busy &= ~(TASK_ST_FEED);
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_FEED);
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_RC_FEED_BOX_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
		//step1
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
		//step2
            if (rc_warning_status())
            {

                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & (TASK_ST_FEED + TASK_ST_RC)) == 0)
                {
                    ex_multi_job.busy |= TASK_ST_RC;
                    ex_multi_job.busy |= TASK_ST_FEED;

                    _main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, RC_CASH_BOX, RC_RETRY_PAYOUT_DIR, 0, 0);
					//背面搬送のポジションセンサ1を紙幣が完全に通過するまで、払い出し方法に回す
                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_FORCE_PAYOUT_REQ, RC_CASH_BOX, 0, 0, 0); //BOXへの搬送で、ワーニング発生中

                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_RC_RETRY_REV);
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
                    set_recovery_step(RECOVERY_STEP_COLLECT_STACKING);
                    ex_multi_job.busy |= TASK_ST_STACKER; // 起動タスクにより変更
                    _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_STACK_START);
                    _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
					#if defined(UBA_RTQ_ICB) //2025-03-25
					if(is_icb_enable())
					{
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI_UNIT, OperationDenomi.unit_retry, 0, 0);  //Payout失敗での回収庫
					}
					#endif
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
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
            }
        }
        break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

#if 1//#if defined(ID003_SPECK64)
void payout_wait_pb_close(void)
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        ex_main_reset_flag = 1;
        break;
        /* wait for pb to close */
    case TMSG_APB_CLOSE_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            _main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_WAIT_REQ);
            _main_send_connection_task(TMSG_CONN_PAYOUT, TMSG_SUB_SUCCESS, 0, 0, 0);

            jdl_rc_payout(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
            jdl_rc_each_count(OperationDenomi.unit, PAYOUT);

        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;

    case TMSG_TIMER_TIMES_UP:
		if(ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			if(rc_busy_status())
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_PAYOUT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_PAYOUT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
    case TMSG_RC_STATUS_INFO:
        if (ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
        {
            ex_rc_error_flag = ex_main_msg.arg2;
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
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_PAYOUT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
        {
            _main_system_error(0, 91);
        }
        break;
    }
}
#endif

#endif // UBA_RTQ
// end
