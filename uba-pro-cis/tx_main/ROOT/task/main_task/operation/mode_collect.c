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

#if defined(UBA_RTQ)
#include "if_rc.h"
#include "systemdef.h"                  //2022-02-17 test
#include "cyclonev_sysmgr_reg_def.h"    //2022-02-17 test
#include "hal_gpio_reg.h"               //2022-02-17 test
#include "feed_rc_unit.h"
#endif

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/
static u8 flap1_pos;
static u8 flap2_pos;

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/*********************************************************************/ /**
* @brief payout message procedure
* @param[in]	None
* @return 		None
**********************************************************************/
void collect_msg_proc(void)
{
    switch (ex_main_task_mode2)
    {
    case COLLECT_MODE2_SENSOR_ACTIVE:
        collect_sensor_active(); //ok
        break;
    case COLLECT_MODE2_INIT_TRANSPORT:
        collect_init_transport(); //ok
        break;
    case COLLECT_MODE2_INIT_RC:
        collect_init_rc(); //ok
        break;
    case COLLECT_MODE2_WAIT_RC_RSP:
        collect_wait_rc_rsp(); //ok
        break;
    case COLLECT_MODE2_WAIT_STACK_START:
        collect_wait_stack_start(); //ok
        break;
    case COLLECT_MODE2_WAIT_STACK_TOP:
        collect_wait_stack_top(); //ok
        break;
    case COLLECT_MODE2_STACK_HOME:
        collect_wait_stack_home(); //ok
        break;
    case COLLECT_MODE2_STACK_RETRY:
        collect_exec_retry(); //ok
        break;
    case COLLECT_MODE2_WAIT_REQ:
        collect_wait_req(); //ok
        break;
    /* Collect retry */
    case COLLECT_MODE2_RC_PREFEED_STACK:
        collect_rc_retry_prefeed_stack(); //ok
        break;
    case COLLECT_MODE2_RC_RETRY_FWD:
        collect_rc_retry_fwd(); //ok
        break;
    case COLLECT_MODE2_RC_RETRY_STACK_HOME:
        collect_rc_retry_stack_home(); //ok
        break;
    case COLLECT_MODE2_RC_RETRY_INIT_RC:
        collect_rc_retry_init_rc(); //ok
        break;
    case COLLECT_MODE2_RC_RETRY_FEED_BOX:
        collect_rc_retry_feed_box(); //ok
        break;
    default:
        /* system error ? */
        _main_system_error(0, 88);
        break;
    }
}

void collect_sensor_active()	//ok
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
    case TMSG_SENSOR_ACTIVE_RSP:
        /* set recovery step */
        set_recovery_step(RECOVERY_STEP_COLLECT_DRUM);
	#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)
		_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_BACK, OperationDenomi.unit, RFID_BACK_COLECT, 0); //Collect時-リカバリフラグ有効 2025-07-23
	#endif

        ex_multi_job.busy &= ~(TASK_ST_SENSOR);

        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
			ex_rc_retry_flg = FALSE; //2025-02-01
            if (!(is_box_set()))        /* CHECK BOX */
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
            else if (!(is_detect_rc_twin()) ||  /* CHECK DRUM */
                    !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                if (!(SENSOR_STACKER_HOME) && !(is_ld_mode()))
                {
                    ex_multi_job.busy |= TASK_ST_STACKER;
                    _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_INIT_TRANSPORT);
                    _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
                }
                else
                {
                    /* SET timeout */
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);

                    switch (OperationDenomi.unit)
                    {
                    case RC_TWIN_DRUM1:
                    case RC_TWIN_DRUM2:
                        /* flapper1 position check */
                        if (!(is_flapper1_twin_to_box_pos()))
                        {
                            flap1_pos = RC_FLAP1_POS_RC_TO_BOX; /* change postion	*/
                        }
                        else
                        {
                            flap1_pos = 0; /* don't move		*/
                        }

                        /* flapper2 position check */
                        if (is_quad_model())
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
                            OperationDenomi.pre_feed = is_pre_feed_check();

                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_RC_RSP);
                            _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_COLLECT_REQ, OperationDenomi.unit, 0, 0, 0);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_COLLECT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                        }
                        else
                        {
                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_INIT_RC);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                        }
                        break;
                    case RC_QUAD_DRUM1:
                    case RC_QUAD_DRUM2:
                        /* flapper1 position check */
                        flap1_pos = 0; /* don't move		*/

                        /* flapper2 position check */
                        if (!(is_flapper2_quad_to_box_pos()))
                        {
                            flap2_pos = RC_FLAP2_POS_RC_TO_BOX; /* change postion	*/
                        }
                        else
                        {
                            flap2_pos = 0; /* don't move		*/
                        }

                        /* send flapper command */
                        if (flap1_pos == 0 && flap2_pos == 0)
                        {
                            OperationDenomi.pre_feed = is_pre_feed_check();

                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_RC_RSP);
                           _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_COLLECT_REQ, OperationDenomi.unit, 0, 0, 0);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_COLLECT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                        }
                        else
                        {
                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_INIT_RC);
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
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    default:
        if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
        {
            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && ((ex_main_msg.tmsg_code & TMSG_TCODE_MASK) != TMSG_TCODE_CLINE))
        {
            /* system error ? */
            _main_system_error(0, 201);
        }
        break;
    }
}

void collect_init_transport() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_STACKER_HOME_RSP:
        ex_multi_job.busy &= ~(TASK_ST_STACKER);

        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
            else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);

                switch (OperationDenomi.unit)
                {
                case RC_TWIN_DRUM1:
                case RC_TWIN_DRUM2:
                    /* flapper1 position check */
                    if (!(is_flapper1_twin_to_box_pos()))
                    {
                        flap1_pos = RC_FLAP1_POS_RC_TO_BOX; /* change postion	*/
                    }
                    else
                    {
                        flap1_pos = 0; /* don't move		*/
                    }

                    /* flapper2 position check */
                    if (is_quad_model())
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
                        OperationDenomi.pre_feed = is_pre_feed_check();

                        _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_RC_RSP);
                        _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_COLLECT_REQ, OperationDenomi.unit, 0, 0, 0);
                        _main_send_msg(ID_RC_MBX, TMSG_RC_COLLECT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    }
                    else
                    {
                        _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_INIT_RC);
                        _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    }
                    break;
                case RC_QUAD_DRUM1:
                case RC_QUAD_DRUM2:
                    /* flapper1 position check */
                    flap1_pos = 0; /* don't move		*/

                    /* flapper2 position check */
                    if (!(is_flapper2_quad_to_box_pos()))
                    {
                        flap2_pos = RC_FLAP2_POS_RC_TO_BOX; /* change postion	*/
                    }
                    else
                    {
                        flap2_pos = 0; /* don't move		*/
                    }

                    /* send flapper command */
                    if (flap1_pos == 0 && flap2_pos == 0)
                    {
                        OperationDenomi.pre_feed = is_pre_feed_check();
                        
                        _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_RC_RSP);
                        // ex_multi_job.busy |= (TASK_ST_FEED);
                        _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_COLLECT_REQ, OperationDenomi.unit, 0, 0, 0);
                        _main_send_msg(ID_RC_MBX, TMSG_RC_COLLECT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                        _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
                    }
                    else
                    {
                        _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_INIT_RC);
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
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 17);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_init_rc() //ok
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
            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
            else if (!(is_detect_rc_twin()) || 
                    !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                if (!(rc_busy_status()) && (ex_multi_job.busy & TASK_ST_RC) != 0)
                {
                    OperationDenomi.pre_feed = is_pre_feed_check();
                    ex_multi_job.busy &= ~(TASK_ST_RC);

                    _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_RC_RSP);
                    _main_send_msg(ID_FEED_MBX, TMSG_FEED_RC_COLLECT_REQ, OperationDenomi.unit, 0, 0, 0);
                    _main_send_msg(ID_RC_MBX, TMSG_RC_COLLECT_REQ, OperationDenomi.unit, OperationDenomi.pre_feed, 0, 0);
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);
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
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
			}
		}
        else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_wait_rc_rsp() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_RC_COLLECT_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0); 
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
            if (rc_warning_status())
            {
				ex_rc_retry_flg = TRUE; //2025-02-01
                ex_multi_job.busy |= TASK_ST_RC;

                /* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
                _main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
                _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_PREFEED_STACK);
            }
            else if (rc_busy_status())
            {
                ex_multi_job.busy |= TASK_ST_RC;
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
            }
            else if (ex_rc_status.sst32B.bit.collect_empty != 0)
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
				if (!(rc_busy_status()) && 
					((ex_multi_job.busy & TASK_ST_RC) != 0))
				{
					ex_multi_job.busy &= ~(TASK_ST_RC);

					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_CHECK, 0, 0, 0);

					/* Set recovery step */
					set_recovery_step(RECOVERY_STEP_COLLECT_STACKING);

					_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_STACK_START);

					ex_multi_job.busy |= TASK_ST_STACKER;
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
				#if defined(UBA_RTQ_ICB) //2025-03-25
					if(is_icb_enable())
					{
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI_UNIT, OperationDenomi.unit, 0, 0); //Collect時
					}
				#endif
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
				}
			}
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if (rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
			if(!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_wait_stack_start()
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
#if 0//#if defined(ID003_SPECK64) //UBA500もソース上は存在するが、PB Close処理が存在しない
	case    TMSG_APB_CLOSE_RSP:
	        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
	        {
	            ex_multi_job.busy &= ~(TASK_ST_APB);
	        }
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{

			}
	        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
	        {
	            ex_multi_job.busy &= ~(TASK_ST_APB);
	            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
	        }
	        break;
#endif
    case TMSG_STACKER_EXEC_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_START)
        {
        #if 0 //2025-08-01 QA評価指摘なのでUBA500と同じだが変更する
              //押し込み開始途中でのResetは半押しで不十分
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
        #endif
            if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
            else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_STACK_TOP);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 75);
        }
        break;
	case	TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_wait_stack_top() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;

#if 0//#if defined(ID003_SPECK64) //UBA500もソース上は存在するが、PB Close処理が存在しない
	case    TMSG_APB_CLOSE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);

			if (!ex_multi_job.busy)
			{
				if (ex_main_reset_flag)
				{
					_main_set_init();
				}
				else if(ex_multi_job.reject & TASK_ST_STACKER)
				{
					// SUBの押しメカがエラーが発生した可能性があるので確認する
					if (!(is_box_set()))
					{ /* box unset */
						_main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else
					{
						// 押し込みリトライ処理へ
						ex_multi_job.busy |= TASK_ST_STACKER;
						_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_STACK_RETRY);
						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
					}
				}
				else if (ex_multi_job.alarm & TASK_ST_STACKER)
				{
					_main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_multi_job.code[MULTI_STACKER], ex_multi_job.sequence[MULTI_STACKER], ex_multi_job.sensor[MULTI_STACKER]);
				}
				else
				{
					if (!(is_box_set()))
					{
						_main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
					}
					else if(ex_rc_error_flag != 0)
					{
						_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
					}
					else if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
					{
						_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
					}
					else
					{
						ex_multi_job.busy |= TASK_ST_STACKER;
						_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_STACK_HOME);
						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);
					}
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			_main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
#endif
    case TMSG_STACKER_EXEC_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);

            if (!ex_multi_job.busy)
            {
                if (ex_main_reset_flag)
                {
                    _main_set_init();
                }
                else if (!(is_box_set()))
                {
                    _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
                }
				else if(ex_rc_error_flag != 0)
				{
					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
				}
                else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
                {
                    _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
                }
                else
                {
                    ex_multi_job.busy |= TASK_ST_STACKER;
                    _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_STACK_HOME);
                    _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);
                }
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);
            ex_multi_job.reject |= TASK_ST_STACKER;
            ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
            ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
            ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;

            if (!ex_multi_job.busy)
            {
                if (ex_main_reset_flag)
                {
                    _main_set_init();
                }
                else if (!(is_box_set()))
                { /* box unset */
                    _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
                }
                else
                {
                    ex_multi_job.busy |= TASK_ST_STACKER;
                    _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_STACK_RETRY);
                    _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
                }
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);
            ex_multi_job.alarm |= TASK_ST_STACKER;
            ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
            ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
            ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;

            if (!ex_multi_job.busy)
            {
                if (ex_main_reset_flag)
                {
                    _main_set_init();
                }
                else
                {
                    _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
                }
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_START)
        {
        }
        else
        {
            _main_system_error(0, 200);
        }
        break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_wait_stack_home() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case	TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
    case TMSG_STACKER_PULL_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_ENABLE_NEXT)
        {
            /* None */
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);

            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
            else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_REQ);
                _main_send_connection_task(TMSG_CONN_COLLECT, TMSG_SUB_SUCCESS, 0, 0, 0);
                /* JDL set log*/
                jdl_rc_collect(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
				jdl_rc_each_count(OperationDenomi.unit, COLLECT);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);

            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 226);
        }
        break;
	case	TMSG_RC_STATUS_INFO:
			if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_exec_retry() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
    case TMSG_STACKER_EXEC_NG_PULL_RSP:
        ex_multi_job.busy &= ~(TASK_ST_STACKER);
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
            else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                ex_multi_job.busy |= TASK_ST_STACKER;
                _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, 0, 0, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 229);
        }
        break;
    case TMSG_STACKER_EXEC_RE_RSP:
        ex_multi_job.busy &= ~(TASK_ST_STACKER);

        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else if (ex_rc_error_flag)
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
            }
            else if (!(is_box_set()))
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
            }
			//上とUBA500も条件が被って、こっちにはいる事はない、正しこっちの方が他のシーケンスと同じで統一されてる
			else if(ex_rc_error_flag != 0)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
            else if (!(is_detect_rc_twin()) || 
                    !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
				_main_send_connection_task(TMSG_CONN_COLLECT, TMSG_SUB_COLLECTED, 0, 0, 0); //UBA500に合わせて復活

				_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_COLLECT, TMSG_SUB_SUCCESS, 0, 0, 0);

				/* JDL set log*/
				jdl_rc_collect(OperationDenomi.unit, 0, OperationDenomi.pre_feed);
				jdl_rc_each_count(OperationDenomi.unit, COLLECT);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            if (ex_main_reset_flag)
            {
                _main_set_init();
            }
            else
            {
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
            }
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 230);
        }
        break;
    case TMSG_RC_SW_COLLECT_RSP:
        _main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
        ex_rc_collect_sw = 0;
        break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_wait_req() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
        _main_set_init();
        break;
case	TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
    case TMSG_CLINE_DISABLE_REQ:
    case TMSG_DLINE_DISABLE_REQ:
        _main_set_disable();
        break;
    case TMSG_CLINE_ENABLE_REQ:
    case TMSG_DLINE_ENABLE_REQ:
        _main_set_enable();
        break;
    case TMSG_CLINE_COLLECT_REQ:
    case TMSG_DLINE_COLLECT_REQ:
        if (is_rc_twin_d1_empty() && 
            is_rc_twin_d2_empty() && 
            is_rc_quad_d1_empty() && 
            is_rc_quad_d2_empty())
        {
            _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);
        }
        else
        {
            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_SENSOR_ACTIVE);
            _main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_ACTIVE_REQ, 0, 0, 0, 0);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
        {
            if (!(is_detect_rc_twin()) || /* detect twin box */
                !(is_detect_rc_quad()))   /* detect quad box */
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
        }
        break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM && ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_rc_retry_prefeed_stack() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_RC_PREFEED_STACK_RSP:
        if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            /* RC-Twin/Quad error */
            _main_alarm_sub(0, 0, MODE1_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
		if(ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
        else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
            if(rc_warning_status())
            {
				ex_rc_retry_flg = TRUE; //2025-02-01
                ex_multi_job.busy |= TASK_ST_RC;

                if ((ex_multi_job.busy & TASK_ST_RC) == 0)
                {
                    /* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
                    _main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
                    _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_PREFEED_STACK);
                }
                else
                {
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
                }
            }
            else if (ex_rc_error_flag != 0)
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
            }
            else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else if (!(rc_busy_status()))
            {
                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & TASK_ST_RC) == 0)
                {
					ex_rc_retry_flg = TRUE; //2025-02-01
                    ex_multi_job.busy |= TASK_ST_RC;

                    _main_send_msg(ID_RC_MBX, TMSG_RC_RETRY_BILL_DIR_REQ, OperationDenomi.unit, RC_RETRY_STACK_DIR, 0, 0);

                    _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_FWD);
                }
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
            }
        }
		else if(ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			if(rc_busy_status())
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
			}
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
    case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_rc_retry_fwd() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
            if (rc_warning_status())
            {
				ex_rc_retry_flg = TRUE; //2025-02-01
                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & TASK_ST_RC) == 0)
                {
                    if (!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
                    {
                        ex_multi_job.busy |= TASK_ST_STACKER;
                        _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_STACK_HOME);
                        _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
                    }
                    else
                    {
                        switch (OperationDenomi.unit)
                        {
                        case RC_TWIN_DRUM1:
                        case RC_TWIN_DRUM2:
                            /* flapper1 position check */
                            if (!(is_flapper1_twin_to_box_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_RC_TO_BOX; /* change postion	*/
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

                            break;
                        case RC_QUAD_DRUM1:
                        case RC_QUAD_DRUM2:
                            /* flapper1 position check */
                            flap1_pos = 0; /* don't move		*/

                            /* flapper2 position check */
                            if (!(is_flapper2_quad_to_box_pos()))
                            {
                                flap2_pos = RC_FLAP2_POS_RC_TO_BOX; /* change postion	*/
                            }
                            else
                            {
                                flap2_pos = 0; /* don't move		*/
                            }
                            break;
                        default:
                            /* system error ? */
                            _main_system_error(0, 91);
                            break;
                        }

                        /* send flapper command */
                        if (flap1_pos == 0 && flap2_pos == 0)
                        {
                            ex_multi_job.busy |= TASK_ST_RC;

                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_FEED_BOX);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, OperationDenomi.unit, 0, 0, 0);
                        }
                        else
                        {
                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_INIT_RC);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                        }
                    }
                }
                else
                {
                    _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
                }
            }
            else if (ex_rc_error_flag != 0)
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
            }
            else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_COLLECT, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
            }
            else if (!(rc_busy_status()))
            {
                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & TASK_ST_RC) == 0)
                {
                    if (!(is_ld_mode()) && !(SENSOR_STACKER_HOME))
                    {
                        ex_multi_job.busy |= TASK_ST_STACKER;
                        _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_STACK_HOME);
                        _main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
                    }
                    else
                    {
                        switch (OperationDenomi.unit)
                        {
                        case RC_TWIN_DRUM1:
                        case RC_TWIN_DRUM2:
                            /* flapper1 position check */
                            if (!(is_flapper1_twin_to_box_pos()))
                            {
                                flap1_pos = RC_FLAP1_POS_RC_TO_BOX; /* change postion	*/
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

                            break;
                        case RC_QUAD_DRUM1:
                        case RC_QUAD_DRUM2:
                            /* flapper1 position check */
                            flap1_pos = 0; /* don't move		*/

                            /* flapper2 position check */
                            if (!(is_flapper2_quad_to_box_pos()))
                            {
                                flap2_pos = RC_FLAP2_POS_RC_TO_BOX; /* change postion	*/
                            }
                            else
                            {
                                flap2_pos = 0; /* don't move		*/
                            }
                            break;
                        default:
                            /* system error ? */
                            _main_system_error(0, 91);
                            break;
                        }

                        /* send flapper command */
                        if (flap1_pos == 0 && flap2_pos == 0)
                        {
                            ex_multi_job.busy |= TASK_ST_RC;

                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_FEED_BOX);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, OperationDenomi.unit, 0, 0, 0);
                        }
                        else
                        {
                            _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_INIT_RC);
                            _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
                        }
                    }
                }
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
            }
        }
		else if(ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			if(rc_busy_status())
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
			}
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
    case TMSG_RC_RETRY_BILL_DIR_RSP:
        if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_rc_retry_stack_home() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
	case TMSG_TIMER_TIMES_UP:
		if(ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			if(rc_busy_status())
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
			}
		}
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
    case TMSG_STACKER_HOME_RSP:
        if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);

            switch (OperationDenomi.unit)
            {
            case RC_TWIN_DRUM1:
            case RC_TWIN_DRUM2:
                /* flapper1 position check */
                if (!(is_flapper1_twin_to_box_pos()))
                {
                    flap1_pos = RC_FLAP1_POS_RC_TO_BOX; /* change postion	*/
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

                break;
            case RC_QUAD_DRUM1:
            case RC_QUAD_DRUM2:
                /* flapper1 position check */
                flap1_pos = 0; /* don't move		*/

                /* flapper2 position check */
                if (!(is_flapper2_quad_to_box_pos()))
                {
                    flap2_pos = RC_FLAP2_POS_RC_TO_BOX; /* change postion	*/
                }
                else
                {
                    flap2_pos = 0; /* don't move		*/
                }
                break;
            default:
                /* system error ? */
                _main_system_error(0, 91);
                break;
            }

            /* send flapper command */
            if (flap1_pos == 0 && flap2_pos == 0)
            {
                ex_multi_job.busy |= TASK_ST_RC;

                _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_FEED_BOX);
                _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, OperationDenomi.unit, 0, 0, 0);
            }
            else
            {
                _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_INIT_RC);
                _main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, flap1_pos, flap2_pos, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            ex_multi_job.busy &= ~(TASK_ST_STACKER);
            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else
        {
            _main_system_error(0, 91);
        }
        break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_rc_retry_init_rc() //ok
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
            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        else
        {
            /* system error ? */
            _main_system_error(0, 90);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if(ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
            if (rc_busy_status())
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_FLAP_CHECK, 0, 0);
            }
            else
            {
                ex_multi_job.busy |= TASK_ST_RC;

                _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_RETRY_FEED_BOX);
                _main_send_msg(ID_RC_MBX, TMSG_RC_FEED_BOX_REQ, OperationDenomi.unit, 0, 0, 0);
            }
        }
        else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if (rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
            }
            else
            {
                _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
            }
        }
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
        break;
	case TMSG_RC_STATUS_INFO:
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

void collect_rc_retry_feed_box() //ok
{
    switch (ex_main_msg.tmsg_code)
    {
    case TMSG_CLINE_RESET_REQ:
    case TMSG_DLINE_RESET_REQ:
		ex_main_reset_flag = 1;
        break;
	case TMSG_FEED_RC_STACK_RSP:
		if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
		}
		else if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_FEED);
			_main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
    case TMSG_RC_FEED_BOX_RSP:
        if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
        {
            _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
        }
        else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
        {
            _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
        }
        break;
    case TMSG_TIMER_TIMES_UP:
        if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
        {
            if(rc_warning_status())
            {
				ex_rc_retry_flg = TRUE; //2025-02-01
                ex_multi_job.busy &= ~(TASK_ST_RC);

                if ((ex_multi_job.busy & TASK_ST_RC) == 0)
                {
                    ex_multi_job.busy |= TASK_ST_RC;

                    /* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
                    _main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
                    _main_set_mode(MODE1_COLLECT, COLLECT_MODE2_RC_PREFEED_STACK);
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
					set_recovery_step(RECOVERY_STEP_COLLECT_STACKING);
					ex_multi_job.busy |= TASK_ST_STACKER; //起動タスクにより変更
					_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_WAIT_STACK_START);
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);

				#if defined(UBA_RTQ_ICB) //2025-03-25
					if(is_icb_enable())
					{
						_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_DENOMI_UNIT, OperationDenomi.unit, 0, 0); //Collect時
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
        else if(ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
        {
            if (rc_busy_status())
            {
                _main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
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
	case TMSG_RC_STATUS_INFO: //UBA500にはないが、いれた
		if(ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_COLLECT, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
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
                _main_alarm_sub(0, 0, TMSG_CONN_COLLECT, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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

#endif // UBA_RTQ
