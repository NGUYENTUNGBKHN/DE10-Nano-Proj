/******************************************************************************/
/*! @addtogroup Main
    @file       mode_alarm.c
    @brief      alarm mode of main task
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

#define EXT
#include "com_ram.c"

/************************** PRIVATE DEFINITIONS ***********************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS **************************/

/************************** EXTERNAL VARIABLES *************************/



/**********************************************************************
 * @brief alarm message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case ALARM_MODE2_WAIT_REQ: /* common*//* ok */
		alarm_wait_req();
		break;
	case ALARM_MODE2_STACKER_FULL: /* common*//* ok */
		alarm_stacker_full();
		break;
	case ALARM_MODE2_STACKER_JAM: /* common*//* ok */
		alarm_stacker_jam();
		break;
	case ALARM_MODE2_STACKER_FAIL: /* common*//* ok */
		alarm_stacker_fail();
		break;
	case ALARM_MODE2_LOST_BILL: /* common*//* ok */
		alarm_lost_bill();
		break;
	case ALARM_MODE2_FEED_SPEED: /* common*//* ok */
		alarm_feed_speed();
		break;
	case ALARM_MODE2_FEED_FAIL: /* common*//* ok */
		alarm_feed_fail();
		break;
	case ALARM_MODE2_ACCEPTOR_JAM: /* common*//* ok */
		alarm_acceptor_jam();
		break;
	case ALARM_MODE2_CONFIRM_AT_JAM:  /* common*//* ok */
		alarm_confirm_at_jam();
		break;
	case ALARM_MODE2_ACCEPTOR_JAM_CIS: /* common*//* ok */
		alarm_acceptor_cis_jam();
		break;
	case ALARM_MODE2_APB_FAIL: /* common*//* ok */
		alarm_apb_fail();
		break;
	case ALARM_MODE2_BOX: /* common*//* ok */
		alarm_box();
		break;
	case ALARM_MODE2_CONFIRM_BOX: /* common*//* ok */
		alarm_confirm_box();
		break;
	case ALARM_MODE2_CHEAT: /* common*//* ok */
		alarm_cheat();
		break;
	case ALARM_MODE2_CENTERING_FAIL: /* common*//* ok */
		alarm_centering_fail();
		break;

	case 	ALARM_MODE2_SHUTTER_FAIL: /* common*//* ok *///2023-01-18a
			alarm_shutter_fail();
			break;

	case	ALARM_MODE2_PUSHER_HOME:	// 0F70	Failure A2 押しメカエラー  /* only */
			alarm_pusher_home();		// 0F70	Failure A2 押しメカエラー
			break;
	case	ALARM_MODE2_WAIT_SENSOR_ACTIVE:	// ポジションセンサのDA再設定待ちに使用 /* only*/
			alarm_wait_sensor_active();
			break;
	
	case	ALARM_MODE2_RFID:				/* ICBエラーの場合 */
			alarm_rfid();
			break;

	case 	ALARM_MODE2_TMP_I2C:
			alarm_cpu_board_wait_reset();
			break;
#if defined(UBA_RTQ)
	case ALARM_MODE2_RC_ERROR:
		alarm_rc_error();
		break;
	case ALARM_MODE2_CONFIRM_RC_ERROR:	//Resetコマンド待ち + RTQ外れるかの監視中
		alarm_confirm_rc_error();
		break;
	case ALARM_MODE2_CONFIRM_RC_UNIT: //RTQが外れているエラー
		alarm_confirm_rc_unit();
		break;
#endif // UBA_RTQ	

	case 	ALARM_MODE2_FRAM:
	case 	ALARM_MODE2_MAG:
	case 	ALARM_MODE2_UV:
	case 	ALARM_MODE2_I2C:
	case 	ALARM_MODE2_SPI:
	case 	ALARM_MODE2_PL_SPI:
	case 	ALARM_MODE2_CISA_OFF:
	case 	ALARM_MODE2_CISB_OFF:
	case 	ALARM_MODE2_CIS_ENCODER:
			alarm_cpu_board();
			break;

	//2024-05-28
	case 	ALARM_MODE2_CIS_TEMPERATURE:
			alarm_cis_temperature();
			break;
	

	default:
		/* system error ? */
		_main_system_error(0, 230);
		break;
	}
}


/*********************************************************************//**
 * @brief wait reset request
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_wait_req(void)
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
	case TMSG_SENSOR_STATUS_INFO:
		bill_in = _main_bill_in();
		if (bill_in == BILL_IN_ACCEPTOR)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_FEED_OTHER_SENSOR_AT, _main_conv_seq(), ex_main_msg.arg2);
		}
		else if (bill_in == BILL_IN_STACKER)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
		}
		break;
	/* 2023-11-28 */
	//case TMSG_LINE_SET_STATUS:
	case TMSG_CLINE_SET_STATUS:
			if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			else if( ex_main_msg.arg1 == TMSG_SUB_WAIT )
			{
	    		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_LINE_WAIT, WAIT_TIME_LINE_REQ, 0, 0);
			}
			break;

	case TMSG_TIMER_TIMES_UP:
		/* 2023-11-28 */
			if( ex_main_msg.arg1 == TIMER_ID_LINE_WAIT )
			{
				/* LINEタスクへエラー復帰を再通知 */
			    _main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
		#if defined(UBA_RTQ)		/* '19-03-18 */
			else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#endif
			break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			/* Line TaskからのReset以外のMessageは待機 */
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 231);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief alarm stacker full
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_stacker_full(void)
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
		if (!(is_box_set()))
		{
			//これがいるか不明 _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RECOVER_WAIT, 0, 0, 0);
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
    /* Fixed EXIT sensor JAM(LD-Mode) */
    case TMSG_DLINE_ENABLE_REQ:
        break;
	case TMSG_APB_EXEC_RSP:
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 232);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief alarm stacker jam
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_stacker_jam(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 234);
		}
		break;
	}
}
/*********************************************************************//**
 * @brief alarm stacker fail
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_stacker_fail(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief alarm lost bill
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_lost_bill(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 235);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief alarm feed speed
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_feed_speed(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 236);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief alarm feed fail
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_feed_fail(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 237);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief alarm acceptor jam
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_acceptor_jam(void)
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
	case TMSG_SENSOR_STATUS_INFO:
	//2023-11-20 Box open 優先
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
		}
		else
		{
			bill_in = _main_bill_in();
			if (bill_in == BILL_IN_NON)
			{
				_main_set_mode(MODE1_ALARM, ALARM_MODE2_CONFIRM_AT_JAM);
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RECOVER_WAIT, WAIT_TIME_RECOVER, 0, 0);
			}
			else if (bill_in == BILL_IN_STACKER)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 238);
		}
		break;
	}
}





/*********************************************************************//**
 * @brief confirm acceptor non paper
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_confirm_at_jam(void)
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
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
		}
		else
		{
			bill_in = _main_bill_in();
			if ((bill_in == BILL_IN_ENTRANCE)
			 || (bill_in == BILL_IN_ACCEPTOR))
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RECOVER_WAIT, 0, 0, 0);
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0); //これをUBA500は使用している
				_main_set_mode(MODE1_ALARM, ALARM_MODE2_ACCEPTOR_JAM);
			}
			else if (bill_in == BILL_IN_STACKER)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_main_msg.arg2);
			}
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RECOVER_WAIT)
		{
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 239);
		}
		break;
	}
}


/* Acceptor JAMにすると自動復帰させないといけない*/
/* CISは消灯させないと、温度が上がり続ける */
/* CISのみで紙幣検知の場合ホストに対してはFailure AFにする */
void alarm_acceptor_cis_jam(void) //2024-06-09 CISのみで紙幣検知でのJAM
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
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			&& (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 238);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief alarm apb fail
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_apb_fail(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 241);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief alarm box set
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_box(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		if (is_box_set())
		{
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_CONFIRM_BOX);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RECOVER_WAIT, WAIT_TIME_RECOVER, 0, 0);
		#if defined(UBA_RTQ)
			/* box open中のbox open検出 */
			ex_rc_detect_next_box_open = 0;
		#endif
		}
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if((!(is_detect_rc_twin()) || !(is_detect_rc_quad())) && ex_rc_detect_next_box_open == 0)
			{
				/* RCエラー中のRecycker OpenはRCのエラーをクリアしておく*/
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STATUS, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);

				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 1;
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 114);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief confirm box
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_confirm_box(void)
{
	u16 bill_in;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RECOVER_WAIT, 0, 0, 0);
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_BOX);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RECOVER_WAIT)
		{
		#if defined(UBA_RTQ)
			if((!(is_detect_rc_twin()) || !(is_detect_rc_quad())) && ex_rc_detect_next_box_open == 0)
			{
				/* RCエラー中のRecycker OpenはRCのエラーをクリアしておく*/
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STATUS, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);

				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 1;
			}
			else
			{
				_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
		#else
			bill_in = _main_bill_in();
			//if (bill_in == BILL_IN_NON)
			if ( bill_in == BILL_IN_NON || bill_in == BILL_IN_ENTRANCE )
			{
				_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			//else if ((bill_in == BILL_IN_ENTRANCE)
			//	  || (bill_in == BILL_IN_ACCEPTOR))
			else if ( bill_in == BILL_IN_ACCEPTOR )
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_FEED_OTHER_SENSOR_AT, _main_conv_seq(), ex_position_sensor);
			}
			else if (bill_in == BILL_IN_STACKER)
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_FEED_OTHER_SENSOR_SK, _main_conv_seq(), ex_position_sensor);
			}
		#endif
		}
#if defined(UBA_RTQ)
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 115);
		}
		break;
	}
}




/*********************************************************************//**
 * @brief alarm cheat
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_cheat(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				/* RCエラー中のRecycker OpenはRCのエラーをクリアしておく*/
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);

				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STATUS, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}

			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif // UBA_RTQ
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 242);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief alarm centering fail
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_centering_fail(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 243);
		}
		break;
	}
}


void alarm_shutter_fail(void)
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
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 243);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief alarm pusher home
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_pusher_home(void)
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
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
	case	TMSG_DLINE_TEST_FINISH_REQ:
			if (is_test_mode())
			{
				_main_set_test_standby();
			}
			break;
	case	TMSG_SENSOR_ACTIVE_RSP:
	case	TMSG_SENSOR_STATUS_INFO:

			if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
			}
			break;
	case	TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)	/* '19-03-18 */
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
				{
					/* RCエラー中のRecycker OpenはRCのエラーをクリアしておく*/
					_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);

					_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STATUS, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
				}

				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
#endif
			break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
			if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
			{
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
			}
			else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
			{
				/* system error ? */
				_main_system_error(0, 110);
			}
			break;
	}
}

/*********************************************************************//**
 * @brief alarm wait sensor active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_wait_sensor_active(void)	// ポジションセンサのDA再設定待ちに使用
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
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
	case	TMSG_SENSOR_ACTIVE_RSP:
	case	TMSG_SENSOR_STATUS_INFO:

			//_main_set_sensor_active(0); CIS版はすでにactive解除されているので、ここで呼び出す必要なし
			// 本来はここでセンサ変化した場合メッセージ受信するのだが、
			// 各エラーモードへの遷移前なので判断できない
			// センサタスクにセンサステータスを投げて各エラーモードでセンサステータスを受信させる
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
		#if !defined(UBA_RTQ)//UBA_RFID
			_main_set_mode( ex_main_task_mode1_alarm_back, ex_main_task_mode2_alarm_back );
		#else
			if(is_rc_error_check())
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_DETAIL_REQ, 0, 0, 0, 0);
				_main_set_mode( ex_main_task_mode1_alarm_back, ex_main_task_mode2_alarm_back );
			}
			else
			{
				_main_set_mode( ex_main_task_mode1_alarm_back, ex_main_task_mode2_alarm_back );
			}
		#endif
			break;
	case	TMSG_TIMER_TIMES_UP:
		#if defined(UBA_RTQ)
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#endif
			break;
	case	TMSG_DLINE_TEST_FINISH_REQ:
			if (is_test_mode())
			{
				_main_set_test_standby();
			}
			break;
	case	TMSG_STACKER_HOME_RSP:
			break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
			if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
			{
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
			}
			else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
			{
				/* system error ? */
				_main_system_error(0, 176);
			}
			break;
	}
}



void alarm_rfid(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_RESET_REQ:
	case	TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
	case TMSG_DLINE_TEST_REQ:
	case	TMSG_DLINE_TEST_FINISH_REQ:
			if (is_test_mode())
			{
				_main_set_test_standby();
			}
			break;
	case	TMSG_SENSOR_ACTIVE_RSP:
	case	TMSG_SENSOR_STATUS_INFO:

			if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
			}
			break;
	case	TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)	/* '19-03-18 */
			if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
	#endif
			break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
			if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
			{
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
			}
			else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
			{
				/* system error ? */
				_main_system_error(0, 117);
			}
			break;
	}
}



void alarm_cpu_board(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		/* keep error status. need POR for recovery */
		_main_alarm_sub(0, 0, TMSG_CONN_RESET, ex_abnormal_code, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_CLINE_SET_STATUS:
		break;
	case TMSG_DLINE_TEST_REQ:
	case TMSG_DLINE_TEST_FINISH_REQ:
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
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 114);
		}
		break;
	}
}

void alarm_cpu_board_wait_reset(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SET_STATUS:
		break;
	case TMSG_DLINE_TEST_REQ:
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 114);
		}
		break;
	}
}


void alarm_cis_temperature(void) //2024-05-28
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init(); //2024-12-08
		break;
	case TMSG_CLINE_SET_STATUS:
		break;
	case TMSG_DLINE_TEST_REQ:
	case TMSG_DLINE_TEST_FINISH_REQ:
		if (is_test_mode())
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		if (!is_box_set())
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_CONFIRM_TEMP)
		{
			/* Set FPGA mode : active */
			_main_set_pl_active(PL_ENABLE);
		
			_pl_cis_enable_set(1);
		
			/* I2C通信エラー対策 wait 50ms after CIS ON/OFF */
			OSW_TSK_sleep(50);
			_main_set_mode(MODE1_ACTIVE_ALARM, ACTIVE_ALARM_MODE2_TMP_READ);
			ex_multi_job.busy |= TASK_ST_MGU;
			_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_TMP, 0, 0, 0);
		}
		break;
#if defined(UBA_RTQ)
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
#endif
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 114);
		}
		break;
	}

}

#if defined(UBA_RTQ)
void alarm_rc_error()
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
			if(ex_rc_timeout == 0 && ex_rc_timeout_error == 1) //2025-06-20	 //2025-07-22
			{
				ex_rc_timeout_error = 0; //2025-07-22
				_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			else if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{

				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);

				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STATUS, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}

			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case	TMSG_RC_ERROR_DETAIL_RSP:
			switch(ex_rc_error_code)
			{
			/* '24-09-10 */
			case	ALARM_CODE_TWIN_DRUM1_JAM_IN:
					if(!(is_rc_twin_d1_empty))
					{
						ex_rc_internal_jam_flag = RC_TWIN_DRUM1;
						ex_rc_internal_jam_flag_bk = RC_TWIN_DRUM1;
					}
					break;
			case	ALARM_CODE_TWIN_DRUM2_JAM_IN:
					if(!(is_rc_twin_d2_empty))
					{
						ex_rc_internal_jam_flag = RC_TWIN_DRUM2;
						ex_rc_internal_jam_flag_bk = RC_TWIN_DRUM2;
					}
					break;
			case	ALARM_CODE_QUAD_DRUM1_JAM_IN:
					if(!(is_rc_quad_d1_empty))
					{
						ex_rc_internal_jam_flag = RC_QUAD_DRUM1;
						ex_rc_internal_jam_flag_bk = RC_QUAD_DRUM1;
					}
					break;
			case	ALARM_CODE_QUAD_DRUM2_JAM_IN:
					if(!(is_rc_quad_d2_empty))
					{
						ex_rc_internal_jam_flag = RC_QUAD_DRUM2;
						ex_rc_internal_jam_flag_bk = RC_QUAD_DRUM2;
					}
					break;
			default:
					break;
			}
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_ERROR);
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 114);
		}
		break;
	}
}

void alarm_confirm_rc_error()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	//2025-05-13
	//case	TMSG_SENSOR_ACTIVE_RSP:
	case	TMSG_SENSOR_STATUS_INFO:
			if(!(is_box_set()))
			{
				/* RCエラー中のBox OpenはRCのエラーをクリアしておく*/
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);

				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
			}
			break;

	case TMSG_TIMER_TIMES_UP:
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if(ex_rc_timeout == 0 && ex_rc_timeout_error == 1) //2025-06-20	 //2025-07-22
			{
				ex_rc_timeout_error = 0; //2025-07-22
				_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			else if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);

				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT, TMSG_CONN_STATUS, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
			}

			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case	TMSG_RC_STATUS_INFO:
			break;

	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 114);
		}
		break;
	}
}

void alarm_confirm_rc_unit()
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_CLINE_RESET_REQ:
		case TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;
	#if defined(UBA_RTQ)	//UBA_MUST 新規作成だから、ここにはいるか確認
	case	TMSG_SENSOR_STATUS_INFO:
			if(!(is_box_set()) && ex_rc_detect_next_box_open == 0)
			{
				/* RCエラー中のBox OpenはRCのエラーをクリアしておく*/
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);

				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);

				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 1;
			}
			break;
	#endif
		case TMSG_TIMER_TIMES_UP:
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RECOVER_WAIT, 0, 0, 0);

			if (is_detect_rc_twin() && is_detect_rc_quad())
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RECOVER_WAIT, 20, 0, 0);
				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 0;
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RECOVER_WAIT)
		{
			if(!(is_box_set()) && ex_rc_detect_next_box_open == 0)
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);

				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ALARM_CODE_BOX, _main_conv_seq(), ex_main_msg.arg2);
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 1;
			}
			else
			{
				_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case	TMSG_RC_ERROR_DETAIL_RSP:
			break;
	case	TMSG_RC_STATUS_INFO:
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case	TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ex_abnormal_code, 0, 0);
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS) && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 114);
		}
		break;
	}
}

#endif // UBA_RTQ

/* EOF */

