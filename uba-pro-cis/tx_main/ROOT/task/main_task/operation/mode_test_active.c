/******************************************************************************/
/*! @addtogroup Main
    @file       mode_test_active.c
    @brief      test mode during IDLE status
    @date       2018/03/05
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/05 Development Dept at Tokyo
      -# Initial Version
*****************************************************************************/
#include <string.h>
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "cpu_api.h"
#include "custom.h"
#include "common.h"
#include "hal.h"
#include "operation.h"
#include "sensor.h"
#include "sub_functions.h"
#include "pl/pl_cis.h"
#if defined(UBA_RTQ)
#include "if_rc.h"
#endif // UBA_RTQ

/* 2023-05-17 耐久用*/
#include "systemdef.h"
#include "cyclonev_sysmgr_reg_def.h"
#include "hal_gpio_reg.h"

#define EXT
#include "com_ram.c"


/************************** PRIVATE VARIABLES *************************/
/************************** EXTERNAL VARIABLES *************************/

/*********************************************************************//**
 * @brief test active message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_active_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case TEST_ACTIVE_MODE2_SENSOR_ACTIVE:
		test_sensor_active();
		break;
	case TEST_ACTIVE_MODE2_FEED_MOTOR_FWD:
	case TEST_ACTIVE_MODE2_FEED_MOTOR_REV:
		test_feed_motor();
		break;
	case TEST_ACTIVE_MODE2_STACKER_MOTOR_FWD:
		test_stacker_motor();
		break;
	case TEST_ACTIVE_MODE2_STACKER:
		test_stacker();
		break;
	case TEST_ACTIVE_MODE2_STACKER_HOME:
		test_stacker_home();
		break;

	case TEST_ACTIVE_MODE2_STACKER_RETRY: //2024-04-09
		test_stacker_retry();
		break;

	case TEST_ACTIVE_MODE2_APB:
		test_apb();
		break;
	case TEST_ACTIVE_MODE2_CENTERING_OPEN_UBA:
		test_centering_open_uba(); 		//UBA500は test_centering();
		break;
	case TEST_ACTIVE_MODE2_CENTERING_CLOSE_UBA:
		test_centering_close_uba();		//UBA500は test_centering_close();
		break;
	case TEST_ACTIVE_MODE2_SENSOR:
		test_sensor();
		break;
	case TEST_ACTIVE_MODE2_AGING_SENSOR_ACTIVE:
		test_aging_sensor_active();
		break;
	case TEST_ACTIVE_MODE2_AGING_CIS_INITIALIZE:
		test_aging_cis_init();
		break;
	case TEST_ACTIVE_MODE2_AGING_INIT:
		test_aging_sensor_init();
		break;
	case TEST_ACTIVE_MODE2_AGING_APB_OPEN:
		test_aging_apb_open_uba();//PB動作完了、次取り込み動作
		break;
	case TEST_ACTIVE_MODE2_AGING_FEED_CENTERING:
		test_aging_feed_centering_uba();
		break;
	case TEST_ACTIVE_MODE2_AGING_CENTERING:
		test_aging_centering_uba();
		break;
	case TEST_ACTIVE_MODE2_AGING_FEED_ESCROW:
		test_aging_feed_escrow_uba();//escrow位置への搬送完了待ち、次shutter close
		break;
	case TEST_ACTIVE_MODE2_AGING_SHUTTER_CLOSE:
		test_aging_shutter_close_uba();//shutter close待ち、次box搬送
		break;
	case TEST_ACTIVE_MODE2_AGING_FEED_APB:
		test_aging_feed_apb_uba();//box搬送待ち、次PB動作
		break;
	case TEST_ACTIVE_MODE2_AGING_APB_CLOSE:
		test_aging_apb_close_uba();//PB動作完了、次shutter open
		break;
	case TEST_ACTIVE_MODE2_AGING_SHUTTER_OPEN:
		test_aging_shutter_open_uba();//shutter open待ち、次収納 top
		break;
	case TEST_ACTIVE_MODE2_AGING_STACKER:
		test_aging_stack_exec_uba();//収納 top待ち、次収納 home
		break;
	case TEST_ACTIVE_MODE2_AGING_STACKER_HOME:
		test_aging_stack_exec_home_uba();//収納 Home待ち、次待機
		break;
	case TEST_ACTIVE_MODE2_SHUTTER:
		test_shutter_motor();
		break;
	case TEST_ACTIVE_MODE2_SHUTTER_CLOSE:
		test_shutter_motor_close();
		break;
	case TEST_ACTIVE_MODE2_RFID_UBA:
		test_rfid_uba();
		break;
#if defined(UBA_RTQ)
	case TEST_ACTIVE_MODE2_COMMUNICATION:
		test_rc_communication();
		break;
	case TEST_ACTIVE_MODE2_DIPSW:
		test_rc_dipsw();
		break;
	case TEST_ACTIVE_MODE2_SW_LED:
		test_rc_sw_led();
		break;
	case TEST_ACTIVE_MODE2_TWIN_FEED_FWD:
	case TEST_ACTIVE_MODE2_TWIN_FEED_REV:
	case TEST_ACTIVE_MODE2_QUAD_FEED_FWD:
	case TEST_ACTIVE_MODE2_QUAD_FEED_REV:
		test_rc_feed();
		break;
	case TEST_ACTIVE_MODE2_TWIN_FLAP:
	case TEST_ACTIVE_MODE2_QUAD_FLAP:
		test_rc_flap();
		break;	
	case TEST_ACTIVE_MODE2_TWIN_POS_SEN1:
	case TEST_ACTIVE_MODE2_TWIN_POS_SEN2:
	case TEST_ACTIVE_MODE2_QUAD_POS_SEN1:
	case TEST_ACTIVE_MODE2_QUAD_POS_SEN2:
		test_rc_sensor();
		break;	
	case TEST_ACTIVE_MODE2_TWIN_SOL:
	case TEST_ACTIVE_MODE2_QUAD_SOL:
		test_rc_sol();
		break;	
	case TEST_ACTIVE_MODE2_TWIN_DRUM1:
	case TEST_ACTIVE_MODE2_TWIN_DRUM2:
	case TEST_ACTIVE_MODE2_QUAD_DRUM1:
	case TEST_ACTIVE_MODE2_QUAD_DRUM2:
		test_rc_drum();
		break;
	case TEST_ACTIVE_MODE2_WRITE_SERIALNO:
	case TEST_ACTIVE_MODE2_READ_SERIALNO:
		test_rc_serial_no();
		break;
	case TEST_ACTIVE_MODE2_START_SENS_ADJ:
	case TEST_ACTIVE_MODE2_READ_SENS_ADJ_DATA:
		test_rc_sens_adj();
		break;
	case TEST_ACTIVE_MODE2_TWIN_FLAP_USB:
	case TEST_ACTIVE_MODE2_QUAD_FLAP_USB:
		test_rc_flap_usb();
		break;
	case TEST_ACTIVE_MODE2_DRUM1_TAPE_POS_ADJ:
	case TEST_ACTIVE_MODE2_DRUM2_TAPE_POS_ADJ:
		test_rc_drum_tape_pos_adj();
		break;
	case TEST_ACTIVE_MODE2_FRAM_CHECK:
		test_rc_fram_check();
		break;
	case TEST_ACTIVE_MODE2_SENS_ADJ_WRITE_FRAM:
		test_rc_sens_adj_write_fram();
		break;
	case TEST_ACTIVE_MODE2_SENS_ADJ_READ_FRAM:
		test_rc_sens_adj_read_fram();
		break;
	case TEST_ACTIVE_MODE2_PERFORM_TEST_WRITE_FRAM:
		test_rc_perform_test_write_fram();
		break;
	case TEST_ACTIVE_MODE2_PERFORM_TEST_READ_FRAM:
		test_rc_perform_test_read_fram();
		break;
	case TEST_ACTIVE_MODE2_WRITE_EDITIONNO:
	case TEST_ACTIVE_MODE2_READ_EDITIONNO:
		test_rc_edition_no();
		break;
	//#if defined(RC_BOARD_GREEN)
	case TEST_ACTIVE_MODE2_RC_WAIT_SENSOR_ADJ:
		test_rc_wait_sensor_adj();
		break;
	//#endif //
	//#if defined(UBA_RTQ_ICB)
	case TEST_ACTIVE_MODE2_RC_RFID:
		test_rc_rfid();
		break;
	//#endif
	//#if defined(UBA_RS)
	case TEST_ACTIVE_MODE2_RC_RS_FLAP:
		test_rc_rs_flap();
		break;
	case TEST_ACTIVE_MODE2_RC_RS_FLAP_USB:
		test_rc_rs_flap_usb();
		break;
	case TEST_ACTIVE_MODE2_RC_RS_SEN:
		test_rc_sensor();
		break;
	//endif
#endif // UBA_RTQ

	default:
		/* system error ? */
		_main_system_error(0, 60);
		break;
	}
}


void test_sensor_active_multi_job_sub(void)
{
	if(ex_test_finish)
	{
		_main_set_test_standby();
	}
	else
	{
		switch (ex_main_test_no)
		{
		case TEST_AGING:
		//case TEST_AGING_LD:
			if (!_is_main_position_all_off())
			{
				_main_set_test_standby();
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER, 0, 0, 0);
				_main_set_mode(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_AGING_INIT_WAIT);
			}
			else
			{
				ex_multi_job.busy |= TASK_ST_FEED;
				_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_INIT);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_INITIAL_REQ, 0, 0, 0, 0);
			}
			break;
		case TEST_ACCEPT:
		case TEST_ACCEPT_ALLACC:
		case TEST_ACCEPT_LD:
		case TEST_ACCEPT_LD_ALLACC:
		case TEST_REJECT:
		case TEST_REJECT_LD:
		case TEST_REJECT_CENT_OPEN: //2024-12-01 生産CIS初期流動に使用
			_main_set_init();
			break;
		case TEST_FEED_MOTOR_FWD:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_FEED_MOTOR_FWD);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_FREERUN_REQ, MOTOR_FWD, 0, 0, 0);
			break;
		case TEST_FEED_MOTOR_REV:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_FEED_MOTOR_REV);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_FREERUN_REQ, MOTOR_REV, 0, 0, 0);
			break;
		case TEST_STACKER_MOTOR_FWD:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_STACKER_MOTOR_FWD);
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_FREERUN_REQ, MOTOR_FWD, 0, 0, 0);
			break;
		case TEST_STACKER:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_STACKER);
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
			break;
		case TEST_STACKER_HOME:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_STACKER_HOME);
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
			break;
		case TEST_APB:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_APB);
			ex_main_test_no = TEST_APB;
			_main_send_msg(ID_APB_MBX, TMSG_APB_EXEC_REQ, 1, 0, 0, 0); /* 回転してHome */
			break;
		case TEST_APB_CLOSE:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_APB);
			ex_main_test_no = TEST_APB_CLOSE;
			_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0); /* 回転してclose */
			break;

		case TEST_CENTERING:	//DIP-1-5
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_CENTERING_OPEN_UBA);
			_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_FREERUN_REQ, 0, 0, 0, 0);
			break;
		case TEST_CENTERING_CLOSE:	//DIP-1-3-5
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_CENTERING_CLOSE_UBA);
			_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_CLOSE_REQ, 0, 0, 0, 0);
			break;
		//#endif
		case TEST_SENSOR:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SENSOR);
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP, 50, 0, 0); //500ms 2025-09-12
			break;
		/* DIP-SWでのテストモードはここから動作開始 */
		case TEST_SHUTTER:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SHUTTER);
			//_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_FREERUN_REQ, MOTOR_FWD, 0, 0, 0);
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
			break;
		case TEST_SHUTTER_CLOSE:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SHUTTER_CLOSE);
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_CLOSE_REQ, 0, 0, 0, 0);
			break;
		case TEST_RFID_UBA:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RFID_UBA);
			_main_send_msg(ID_RFID_MBX, TMSG_RFID_RESET_REQ, 0, 0, 0, 0); //2023-03-16
			break;
#if defined(UBA_RTQ)
		case TEST_RC1:	/* DIP 1, 2, 5 */
		case TEST_RC1_USB:
			{
				mode_test_active_rc1_proc();
			}
			break;
		case TEST_RC2:	/* DIP 1, 2, 6 */
		case TEST_RC2_USB:
			{
				mode_test_active_rc2_proc();
			}
			break;
		case TEST_RC3_USB:
			{
				mode_test_active_rc3_proc();
			}
			break;
		case TEST_RC_AGING_FACTORY:
			{
				OperationDenomi.unit = RC_TWIN_DRUM1;
				OperationDenomi.count = 1;
				OperationDenomi.remain = 0;
				_main_set_init();
			}
			break;
		case TEST_RC_AGING:
			{
				OperationDenomi.unit = RC_TWIN_DRUM1;
				OperationDenomi.count = 1;
				OperationDenomi.remain = 0;
				_main_set_init();
			}
			break;
	//#if defined(RC_BOARD_GREEN)
		case TEST_RS_USB:
			{
				mode_test_active_rs_sh3_usb_proc();
			}
			break;
	//#endif 
	//#if defined(UBA_RTQ_ICB)
		case TEST_RS:
			{
				mode_test_active_rs_sh3_proc();
			}
			break;
	//#endif
#endif // UBA_RTQ
		default:
			/* system error ? */
			_main_system_error(0, 61);
			break;
		}
	}
}
/*********************************************************************//**
 * @brief sensor active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_sensor_active(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		test_sensor_active_multi_job_sub();
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 62);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief feed motor test
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_feed_motor(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		_main_send_msg(ID_FEED_MBX, TMSG_FEED_FREERUN_REQ, MOTOR_STOP, 0, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_FEED_FREERUN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_set_test_standby();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 63);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief stack motor test
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_stacker_motor(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_FREERUN_REQ, MOTOR_STOP, 0, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_STACKER_FREERUN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_set_test_standby();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 63);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief stacker test
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_stacker(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_STACKER_EXEC_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				// wait next test run
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
			{
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			//2024-04-09
			else if(ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
			// 押し込みの頂点時にすでにリトライが確定している場合は、Retryメッセージを受信するようにする
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				if (!(is_box_set()))
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
						_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_STACKER_RETRY);
						_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);
					}
				}
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{
				/* system error ? */
				_main_system_error(0, 67);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 68);
		}
		break;
	}
}
/*********************************************************************//**
 * @brief stacker home test
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_stacker_home(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_STACKER_HOME_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				// wait next test run
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{
				/* system error ? */
				_main_system_error(0, 67);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 68);
		}
		break;
	}
}

#if 1 //2024-04-09
void test_stacker_retry(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
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
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 229);
			}
			break;
	case	TMSG_STACKER_EXEC_RE_RSP:	//Setp2  1度目の押し込みでNG、押しメカ押し込み and 戻し動作

			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				// wait next test run
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~(TASK_ST_STACKER);
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 230);
			}
			break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 68);
		}
		break;
	}
}
#endif


void test_centering_open_uba(void) //UBA500は test_centering();
{

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_main_test_no = TEST_STANDBY;
		//ex_test_finish = 1;	//こっちでもいいようだ
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)		/* '19-03-18 */
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif
		break;
	case TMSG_CENTERING_FREERUN_RSP:
		if (ex_main_test_no == TEST_STANDBY)
		//if (ex_test_finish) //こっちでもいいようだ
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_dline_testmode.test_result = TEST_RESULT_OK;
				_main_set_test_wait();

				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_dline_testmode.test_result = TEST_RESULT_NG;

				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{
				/* system error ? */
				_main_system_error(0, 67);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{

			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 68);
		}
		break;
	}
}
/*********************************************************************//**
 * @brief centering test
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_centering_close_uba(void)
{

	u8 dip = 0;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		/* 生産初期流動でも使用しているので、処理を変えない。*/
		ex_main_test_no = TEST_STANDBY;
		_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 100, 0, 0);
		}
#if defined(UBA_RTQ)		/* '19-03-18 */
		else if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif
		break;

	case	TMSG_CENTERING_HOME_RSP:
		/* 生産初期流動でも使用しているので、処理を変えない。*/
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if (ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 100, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_START)
		{
		// 幅寄せタスクが動作開始

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_dline_testmode.test_result = TEST_RESULT_NG;

			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;

	case TMSG_CENTERING_CLOSE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* DIP-SWが変更されるまで、幅よせは閉じたまま */
			ex_dline_testmode.test_result = TEST_RESULT_OK;
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 100, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_dline_testmode.test_result = TEST_RESULT_NG;

			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 135);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{

			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 136);
		}
		break;
	}
}





/*********************************************************************//**
 * @brief apb test
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_apb(void)
{

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_main_test_no = TEST_STANDBY;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
	#if defined(UBA_RTQ)		/* '19-03-18 */
		if(ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
	#endif
		break;
	case TMSG_APB_EXEC_RSP:
	case TMSG_APB_HOME_RSP:
	case TMSG_APB_CLOSE_RSP:
		if (ex_main_test_no == TEST_STANDBY)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_dline_testmode.test_result = TEST_RESULT_OK;
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_dline_testmode.test_result = TEST_RESULT_NG;
				_main_set_test_wait();
//				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);

			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{

			}
			else
			{
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 132);
		}
		break;
	}
}

void test_sensor_uba(void)
{

	u8 dipsw;
	u8 red = 0;
	u8 green = 0;

	dipsw = ex_dipsw1; //ok

	switch(dipsw)
	{
	case 0x41:
	// DIP-1,7
	// 入口ONで緑点灯
	// 幅よせポジションONで赤点灯
		if(SENSOR_ENTRANCE)
		{
			green = 1;	//on
		}
		else
		{
			green = 0;	//off
		}

		if(SENSOR_CENTERING)
		{
			red = 1;
		}
		else
		{
			red = 0;
		}
		_hal_i2c3_for_led_tca9535(red, 0x08); //red
		_hal_i2c3_for_led_tca9535(green, 0x10); //green

		break;

	case 0x42:
	// DIP-2,7
	// PB IN ONで緑点灯
	// PB OUT ONで赤点灯
		if(SENSOR_APB_IN)
		{
			green = 1;
		}
		else
		{
			green = 0;
		}

		if(SENSOR_APB_OUT)
		{
			red = 1;
		}
		else
		{
			red = 0;
		}
		_hal_i2c3_for_led_tca9535(red, 0x08); //red
		_hal_i2c3_for_led_tca9535(green, 0x10); //green

		break;

	case 0x44:
	// DIP-3,7
	// PB Unit Homeで緑点灯
	// 幅寄せUnit Homeで赤点灯
		if(SENSOR_APB_HOME)
		{
			green = 1;
		}
		else
		{
			green = 0;
		}

		if(SENSOR_CENTERING_HOME)
		{
			red = 1;
		}
		else
		{
			red = 0;
		}
		_hal_i2c3_for_led_tca9535(red, 0x08); //red
		_hal_i2c3_for_led_tca9535(green, 0x10); //green
		break;

	#if 0 //製品仕様書に記載していないので、未対応にする
	case 0x48:
	// DIP-4,7
	// 搬送モータエンコーダで緑点灯
	// Stackerモータエンコーダで赤点灯
		break;
	#endif

	case 0x50:
	// DIP-5,7
	// 押しメカHomeで緑点灯
	// BOXありで赤点灯
		if(SENSOR_STACKER_HOME)
		{
			green = 1;
		}
		else
		{
			green = 0;
		}

		if(SENSOR_BOX)
		{
			red = 1;
		}
		else
		{
			red = 0;
		}
		_hal_i2c3_for_led_tca9535(red, 0x08); //red
		_hal_i2c3_for_led_tca9535(green, 0x10); //green
		break;

	case 0x60:
	// DIP-6,7
	// EXITで緑点灯
		if(SENSOR_EXIT)
		{
			green = 1;
		}
		else
		{
			green = 0;
		}
		red = 0;

		_hal_i2c3_for_led_tca9535(red, 0x08); //red
		_hal_i2c3_for_led_tca9535(green, 0x10); //green
		break;
	}
}

/******************************************************************************/
/*! @brief sensor test procedure (only wait after sensor active)
    @return         none
    @exception      none
******************************************************************************/
void test_sensor(void)
{
	u8 dipsw;
	u8 sensor = 0;



	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		_main_set_test_standby();
		break;
	case TMSG_SENSOR_STATUS_INFO:
		test_sensor_uba();
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_TEMP) //2025-09-12
		{
			test_sensor_uba();
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP, 50, 0, 0); //500ms 2025-09-12			
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief aging feed init
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_aging_sensor_init(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_FEED_INITIAL_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~TASK_ST_FEED;
				_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_SENSOR_ACTIVE);
				_main_set_sensor_active(1);
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
				ex_multi_job.busy &= ~TASK_ST_FEED;
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, ex_main_msg.arg2, 0, 0, 0);

				// wait next test run
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~TASK_ST_FEED;
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 74);
			}
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 75);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief aging cis init
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_aging_cis_init(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
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
				if (!is_ld_mode())
				{
				#if MAG1_ENABLE
					_hal_i2c3_write_mag_cntl(1);		//2022-05-26
				#endif
					_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_APB_OPEN);
					_main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);
				}
				else
				{
				#if MAG1_ENABLE	
					_hal_i2c3_write_mag_cntl(1);		//2022-05-26
				#endif
					_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_APB_OPEN);
					_main_send_msg(ID_APB_MBX, TMSG_APB_HOME_REQ, 0, 0, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				ex_multi_job.busy &= ~TASK_ST_CIS_INIT;
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 74);
			}
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 75);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief sensor active (payout state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_aging_sensor_active(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{			
		#if (_DEBUG_EMI_IMAGE_CHECK==1) || (_DEBUG_EMI_MAG_CHECK==1)
			_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_AGING, 0, 0, 0);
		#else
			_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_BILL_IN, 0, 0, 0);
		#endif
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_CIS_INITIALIZE);
		}
		break;

	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 76);
		}
		break;
	}
}

/******************************************************************************/
/*! @brief feed centering procedure
    @return         none
    @exception      none
******************************************************************************/
void test_aging_feed_centering_uba(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_FEED_AGING_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_FEED);
				/* next step */
				_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_CENTERING);
				ex_multi_job.busy |= TASK_ST_CENTERING;
				_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_EXEC_REQ, 0, 0, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_INTERIM)
			{

			}
			else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, ex_main_msg.arg2, 0, 0, 0);

				// wait next test run
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_START)
			{

			}

		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 78);
		}
		break;
	}
}

/******************************************************************************/
/*! @brief centering exec procedure
    @return         none
    @exception      none
******************************************************************************/
void test_aging_centering_uba(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_CENTERING_EXEC_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_START)
		{


		}
		else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_CENTERING);

			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				ex_multi_job.busy |= TASK_ST_FEED;
				_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_FEED_ESCROW);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_AGING_REQ, FEED_AGING_ESCROW, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_CENTERING);
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 81);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 82);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief feed escrow procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_aging_feed_escrow_uba(void)//escrow位置への搬送完了待ち、次shutter close
{
	#if (_DEBUG_EMI_MAG_CHECK==1)//2023-09-28
	u8 rerult;	//2023-09-28
	#endif

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_FEED_AGING_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
// EMI用
#if defined(_ENABLE_MAG_AGING_EMI)
				if (is_mag_noise_adove_limit())
				{
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_MAG, _main_conv_seq(), ex_position_sensor);
				}
				else
#endif
				{
					#if MAG1_ENABLE
					_hal_i2c3_write_mag_cntl(0);		//2022-05-26

					#if (_DEBUG_EMI_MAG_CHECK==1)//2023-09-28
					rerult = check_mag_aging();

					if(rerult == 0)
					{
						_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_MAG, ex_main_msg.arg3, ex_main_msg.arg4);
						return;
					}
					#endif

				#if (_DEBUG_EMI_IMAGE_CHECK==1)
					if(emi_image_check_black_line() != RET_OK)
					{
						_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_CHEAT, ex_main_msg.arg3, ex_main_msg.arg4);
						return;
					}
				#endif


					#endif
					_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_SHUTTER_CLOSE);
					_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_CLOSE_REQ, 0, 0, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, ex_main_msg.arg2, 0, 0, 0);

				// wait next test run
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{
				/* system error ? */
				_main_system_error(0, 83);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 84);
		}
		break;
	}
}


void  test_aging_shutter_close_uba(void)//shutter close待ち、次box搬送
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_SHUTTER_CLOSE_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_TMP, 0, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{
				/* system error ? */
				_main_system_error(0, 85);
			}
		}
		break;

	case TMSG_MGU_READ_RSP:
		ex_multi_job.busy &= ~(TASK_ST_MGU);
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{	
			ex_multi_job.busy |= TASK_ST_FEED;
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_FEED_APB);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_AGING_REQ, FEED_AGING_APB, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 88);
		}
		break;

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
 * @brief feed apb procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void  test_aging_feed_apb_uba(void)//box搬送待ち、次PB動作
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_FEED_AGING_RSP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_multi_job.busy &= ~(TASK_ST_FEED);
				_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_APB_CLOSE);
				_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 0, 0, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)
			{
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, ex_main_msg.arg2, 0, 0, 0);

				// wait next test run
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if (ex_main_msg.arg1 != TMSG_SUB_START)
			{
				/* system error ? */
				_main_system_error(0, 85);
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
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
 * @brief apb exec procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_aging_apb_close_uba(void)//PB動作完了、次shutter open
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_APB_CLOSE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_SHUTTER_OPEN);		/* 2021-12-03 */
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				if (!(ex_multi_job.alarm))
				{ /* set alarm inform */
					ex_multi_job.alarm |= TASK_ST_APB;
					ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
				}
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 88);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 89);
		}
		break;
	}
}

void test_aging_apb_open_uba(void)//PB動作完了、次shutter open
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_APB_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_FEED_CENTERING);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_AGING_REQ, FEED_AGING_CENTERING, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_APB);
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				if (!(ex_multi_job.alarm))
				{ /* set alarm inform */
					ex_multi_job.alarm |= TASK_ST_APB;
					ex_multi_job.code[MULTI_APB] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_APB] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_APB] = ex_main_msg.arg4;
				}
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 88);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 89);
		}
		break;
	}
}


void test_aging_shutter_open_uba(void)//shutter open待ち、次収納 top
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_SHUTTER_OPEN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_SHUTTER);
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				if(!is_box_set())
				{
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{ /* all job normal end */
					_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_STACKER);
					ex_multi_job.busy |= TASK_ST_STACKER;
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_REQ, 0, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_SHUTTER);
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				if (!(ex_multi_job.alarm))
				{ /* set alarm inform */
					ex_multi_job.alarm |= TASK_ST_SHUTTER;
					ex_multi_job.code[MULTI_SHUTTER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_SHUTTER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_SHUTTER] = ex_main_msg.arg4;
				}
			}
		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 88);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 89);
		}
		break;
	}
}

void test_aging_stack_exec_uba(void)	//収納 top待ち、次収納 home
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_STACKER_EXEC_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				if(!is_box_set())
				{
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
				}
				else
				{ /* all job normal end */
					ex_multi_job.busy |= TASK_ST_STACKER;

					// スタッカHomeへ
					_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_STACKER_HOME);		/* 2021-12-03 */
					_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_PULL_REQ, 0, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			if (!(ex_multi_job.busy))
			{ /* all job end */
				if (ex_multi_job.alarm)
				{ /* other job alarm */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_multi_job.code[alarm_index()], ex_multi_job.sequence[alarm_index()], ex_multi_job.sensor[alarm_index()]);
				}
				else
				{ /* other job normal end */
					_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
				}
			}
			else
			{
				if (!(ex_multi_job.alarm))
				{ /* set alarm inform */
					ex_multi_job.alarm |= TASK_ST_STACKER;
					ex_multi_job.code[MULTI_STACKER] = ex_main_msg.arg2;
					ex_multi_job.sequence[MULTI_STACKER] = ex_main_msg.arg3;
					ex_multi_job.sensor[MULTI_STACKER] = ex_main_msg.arg4;
				}
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_REJECT)	//2022-10-04
		{
			// リジェクトの場合の全ての処理が終わるのを待つ必要がある
			// エージングは単体動作なので、収納戻りからのリトライ処理を行う
			//2023-07-31
			// 押し込みリトライ処理へ
			ex_multi_job.busy |= TASK_ST_STACKER;
			// スタッカHomeへ
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_STACKER_HOME);
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_NG_PULL_REQ, 0, 0, 0, 0);

		}
		else if(ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{

		}
		else
		{
			/* system error ? */
			//いろいろ受信するので無視 2022-10-19
			//_main_system_error(0, 88);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 89);
		}
		break;
	}
}



void test_aging_stack_exec_home_uba(void)	//収納 Home待ち、次待機
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_STACKER_PULL_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				// wait next test run
				#if (UBA_RFID_AGING == 1)
				/* RFID通信するかも */
				_main_send_msg(ID_RFID_MBX, TMSG_RFID_RESET_REQ, 0, 0, 0, 0);

				#else
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, _main_get_aging_time(), 0, 0);
				#endif

				#if 1 //2023-07-22
				//2023-05-17 耐久カウンタ更新
				Gpio_out( GPIO_28, 0 );
				dly_tsk(100);
				Gpio_out( GPIO_28, 1 );
				#endif

			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM) //2023-07-03
		{
			ex_multi_job.busy &= ~(TASK_ST_STACKER);
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4); //2023-07-03
		}
		else if(ex_main_msg.arg1 == TMSG_SUB_INTERIM)
		{

		}
		else
		{
			/* system error ? */
			//いろいろ受信するので無視 2022-10-19
			//_main_system_error(0, 88);
		}
		break;
#if 1	//2023-07-31 押し込みエラーのリトライ中
	case TMSG_STACKER_EXEC_NG_PULL_RSP:	// 戻し動作
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* Normal Operation */
			if (!(is_box_set()))
			{
				_main_alarm_sub( MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
			// next step
				ex_multi_job.busy |= TASK_ST_STACKER;//起動タスクにより変更
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_EXEC_RE_REQ, 0, 0, 0, 0);	// リトライ用の押し込み命令へ
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub( MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 234);
		}
		break;

	case TMSG_STACKER_EXEC_RE_RSP:	//Setp2  1度目の押し込みでNG、押しメカ押し込み and 戻し動作

		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* Normal Operation */
			if (!(is_box_set()))
			{
				_main_alarm_sub( MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
			// next step
				// wait next test run
				#if (UBA_RFID_AGING == 1)
				/* RFID通信するかも */
				_main_send_msg(ID_RFID_MBX, TMSG_RFID_RESET_REQ, 0, 0, 0, 0);

				#else
				_main_set_test_wait();
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, _main_get_aging_time(), 0, 0);
				#endif

				#if 1 //2023-07-22
				//2023-05-17 耐久カウンタ更新
				Gpio_out( GPIO_28, 0 );
				dly_tsk(100);
				Gpio_out( GPIO_28, 1 );
				#endif
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub( MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 234);
		}
		break;
#endif
	
	#if (UBA_RFID_AGING == 1)
	case TMSG_RFID_RESET_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_set_test_wait();
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, _main_get_aging_time(), 0, 0);
		}
		else
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_RFID_UNIT_MAIN, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	#endif
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 89);
		}
		break;
	}
}

void test_shutter_motor(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		ex_test_finish = 1;
		//_main_send_msg(ID_FEED_MBX, TMSG_FEED_FREERUN_REQ, MOTOR_STOP, 0, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_SHUTTER_INITIAL_OPEN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_dline_testmode.test_result = TEST_RESULT_OK;
			_main_set_test_wait();
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 300, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(ex_test_finish == 1)
			{
				_main_set_test_standby();
			}
			else
			{
				ex_dline_testmode.test_result = TEST_RESULT_NG;
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_system_error(0, 63);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}

void test_shutter_motor_close(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		//ex_test_finish = 1;
		_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_SHUTTER_INITIAL_OPEN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_set_test_standby();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			//2023-12-21
			ex_dline_testmode.test_result = TEST_RESULT_NG;
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			_main_set_test_standby();
		}
		break;
	case TMSG_SHUTTER_CLOSE_RSP:	//TEST_SHUTTER_CLOSE
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_dline_testmode.test_result = TEST_RESULT_OK;
			if(ex_test_finish == 1)
			{
				/* 開いてからテストモード解除 */
				_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
				//_main_set_test_standby();
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(ex_test_finish == 1)
			{
				_main_set_test_standby();
			}
			else
			{
				ex_dline_testmode.test_result = TEST_RESULT_NG;
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_START)
		{

		}
		else if (ex_main_msg.arg1 != TMSG_SUB_START)
		{
			/* system error ? */
			_main_set_test_standby();
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}


void test_rfid_uba(void) //2023-03-16
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
	//DIP-SW 8 OFFのテストモード解除には対応しない
	//ex_dline_testmode.test_no がすでに、解除されているため
	//RFIDタスクからのレスポンスがMAINタスクではなくICBタスクで受信するように
	//切り替わってしまっている為、正式に対応するには、RFIDタスクでのメッセージ送信先に
	//工夫が必要、
	//このテストモードのDIP-SW 8 ONのモード解除が大切ではないので対応しない
	//	ex_test_finish = 1;
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	case TMSG_RFID_RESET_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_hal_i2c3_for_led_tca9535(1, 0x10);	/* green on*/
			_hal_i2c3_for_led_tca9535(0, 0x08);	/* red off */
			dly_tsk(3000);
			_main_send_msg(ID_RFID_MBX, TMSG_RFID_RESET_REQ, 0, 0, 0, 0);
		}
		else
		{
		#if 0 //2023-10-03, エラーにしないでリトライの方が便利がいいかもしれないのでこっちにする
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_RFID_UNIT_MAIN, ex_main_msg.arg3, ex_main_msg.arg4);
		#else
			_hal_i2c3_for_led_tca9535(0, 0x10);	/* green off*/
			_hal_i2c3_for_led_tca9535(1, 0x08);	/* red on */
			dly_tsk(3000);
			_main_send_msg(ID_RFID_MBX, TMSG_RFID_RESET_REQ, 0, 0, 0, 0);
		#endif

		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}



#if defined(UBA_RTQ)
void mode_test_active_rc1_proc()
{
	u8 rx_test_type;
	if (ex_main_test_no == TEST_RC1)
	{
		rx_test_type = ex_rc_dip_sw;
	}
	else if (ex_main_test_no == TEST_RC1_USB)
	{
		rx_test_type = ex_rc_test_type;
	}
	else
	{
		_main_system_error(0, 123);
	}

	switch(rx_test_type)
	{
		case DIPSWRC_COMMUNICATION:
			// change mode
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_COMMUNICATION);
			/* set first sequence */
			ex_rc_aging_seq = RC_STATE_IDLE;
			// rc task 
			_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, ex_rc_aging_seq, 0, 0, 0);

			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 50, 0, 0);
			break;
		case DIPSWRC_DIPSW:
			// change mode
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_DIPSW);
			/* timer 100 */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
			break;
		case DIPSWRC_SW_LED:
			// change mode
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SW_LED);

			/* set first sequence */
			ex_rc_aging_seq = COLOR_NONE;

			//2025-05-08
			
			if(is_rc_rs_unit())
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, ex_rc_aging_seq, 0, 0, RC_RS_UNIT);
			}
			else
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, ex_rc_aging_seq, 0, 0, 0);
			}		
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
			break;
		default:
			// error
			_main_set_test_standby();
			break;
	}
}

/*********************************************************************//**
 * @brief communication test(RC-Twin/Quad)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_rc_communication(void)
{
	static int count = 0;
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		/* test finish */
		ex_test_finish = 1;
	
		_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_IDLE, 0, 0, 0);
		break;
	case TMSG_TIMER_TIMES_UP:
		if(ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				if(++ex_rc_aging_seq > RC_STATE_REJECT)
				{
					ex_rc_aging_seq = RC_STATE_IDLE;
				}

				if(ex_rc_aging_seq == RC_STATE_DEPOSIT_BEFORE_VEND
				|| ex_rc_aging_seq == RC_STATE_DESPENSE_BEFORE_VEND
				|| ex_rc_aging_seq == RC_STATE_COLLECTION)
				{
					_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_RC_OFF, 0, 0, 0);
				}
				else
				{
					_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_RC_GREEN, 0, 0, 0);
				}

			
				_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, 0, 0, 0, 0);

			
				/* wait 500msec */
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 50, 0, 0);
			}
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* 03/04/2025 add default case */
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}

void test_rc_dipsw()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		/* test finish */
		ex_test_finish = 1;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{

				_main_send_msg(ID_RC_MBX, TMSG_RC_GET_DIPSW_REQ, 0, 0, 0, 0);

				switch (ex_rc_dip_sw)
				{
				case 0x55:
					/* 	Red = OFF, Green = ON */
					_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_RC_GREEN, 0, 0, 0);
					break;
				case 0xAA:
					/* 	Red = ON, Green = OFF */
					_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_RC_RED, 0, 0, 0);
					break;
				case 0x00:
					/* 	Red = ON, Green = ON  */
					_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_RC_ON, 0, 0, 0);
					break;
				default:
					/* 	Red = OFF, Green = OFF  */
					_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_RC_OFF, 0, 0, 0);
					break;
				}
				/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
				/* wait 100msec */
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
			}
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* 03/04/2025 add default case */
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}

void test_rc_sw_led()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		/* test finish */
		ex_test_finish = 1;
	
		_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			/* wait 200msec */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		if (ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (ex_rc_collect_sw)
			{
				if (++ex_rc_aging_seq > COLOR_WHITE)
				{
					ex_rc_aging_seq = COLOR_NONE;
				
					_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, ex_rc_aging_seq, 0, 0);
				}
				else
				{
				
					_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, ex_rc_aging_seq, 0, 0);
				}
				ex_rc_collect_sw = 0;
			}
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	default:
		/* 03/04/2025 add default case */
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 64);
		}
		break;
	}
}

void mode_test_active_rc2_proc()
{
	u8 rx_test_type;
	if (ex_main_test_no == TEST_RC2)	/* DIP 1, 2, 6 */
	{
		rx_test_type = ex_rc_dip_sw;
	}
	else if (ex_main_test_no == TEST_RC2_USB)
	{
		rx_test_type = ex_rc_test_type;
	}
	else
	{
		_main_system_error(0, 124);
	}

	switch(rx_test_type)
	{
		/* TWIN Test */
		case DIPSWRC_TWIN_FEED_FWD:			/* feed forward */
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_FEED_FWD);
			ex_rc_test_status = RC_BUSY_STATUS;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED1, MOTOR_FWD, 0, 0);
			break;
		case DIPSWRC_TWIN_FEED_REV:			/* feed reverse */
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_FEED_REV);
			ex_rc_test_status = RC_BUSY_STATUS;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED1, MOTOR_REV, 0, 0);
			break;
		case DIPSWRC_TWIN_FLAP:				/* flap motor */
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_FLAP);
			/* set first sequence*/
			ex_rc_aging_seq = RC_FLAP1_POS_HEAD_TO_RC;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, ex_rc_aging_seq, 0, 0, 0);
			break;
		case DIPSWRC_TWIN_SEN1:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_POS_SEN1);
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_TWIN_TRANSPORT_POS, 0, 0);
			break;
		case DIPSWRC_TWIN_SEN2:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_POS_SEN2);
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_TWIN_DRUM_POS, 0, 0);
			break;
		case DIPSWRC_TWIN_SOL:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_SOL);

			ex_rc_test_status = RC_BUSY_STATUS;

			/* set first sequence */
			ex_rc_aging_seq = RC_SOL_ON;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_SOL_REQ, RC_TWIN, ex_rc_aging_seq, 0, 0);
			break;
		case DIPSWRC_TWIN_DRUM1:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_DRUM1);
			ex_rc_test_status = RC_BUSY_STATUS;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_TWIN_DRUM1, RC_DRUM_START, 0, 0);
			break;
		case DIPSWRC_TWIN_DRUM2:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_DRUM2);	
			ex_rc_test_status = RC_BUSY_STATUS;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_TWIN_DRUM2, RC_DRUM_START, 0, 0);
			break;
		/* QUAD Test */
		case DIPSWRC_QUAD_FEED_FWD:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_FEED_FWD);
			ex_rc_test_status = RC_BUSY_STATUS;	
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED2, MOTOR_FWD, 0, 0);
			break;	
		case DIPSWRC_QUAD_FEED_REV:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_FEED_REV);
			ex_rc_test_status = RC_BUSY_STATUS;	
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED2, MOTOR_REV, 0, 0);
			break;	
		case DIPSWRC_QUAD_FLAP:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_FLAP);	
			/* set first sequence*/
			ex_rc_aging_seq = RC_FLAP2_POS_RC_TO_BOX;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, 0, ex_rc_aging_seq, 0, 0);
			break;		
		case DIPSWRC_QUAD_SEN1:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_POS_SEN1);	
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_QUAD_TRANSPORT_POS, 0, 0);
			break;		
		case DIPSWRC_QUAD_SEN2:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_POS_SEN2);	
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_QUAD_DRUM_POS, 0, 0);
			break;		
		case DIPSWRC_QUAD_SOL:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_SOL);	
			/* set first sequence */
			ex_rc_aging_seq = RC_SOL_ON;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_SOL_REQ, RC_QUAD, ex_rc_aging_seq, 0, 0);
			break;		
		case DIPSWRC_QUAD_DRUM1:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_DRUM1);	
			ex_rc_test_status = RC_BUSY_STATUS;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_QUAD_DRUM1, RC_DRUM_START, 0, 0);
			break;		
		case DIPSWRC_QUAD_DRUM2:
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_DRUM2);	
			ex_rc_test_status = RC_BUSY_STATUS;
		
			_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_QUAD_DRUM2, RC_DRUM_START, 0, 0);
			break;		
		default:
			break;
	}
}

void test_rc_feed(void)
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_DLINE_TEST_FINISH_REQ:
			/* test finish */
			ex_test_finish = 1;
			switch(ex_main_task_mode2)
			{
			case TEST_ACTIVE_MODE2_TWIN_FEED_FWD:
			case TEST_ACTIVE_MODE2_TWIN_FEED_REV:
			
				_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED1, MOTOR_STOP, 0, 0);
				break;
			case TEST_ACTIVE_MODE2_QUAD_FEED_FWD:
			case TEST_ACTIVE_MODE2_QUAD_FEED_REV:
			
				_main_send_msg(ID_RC_MBX, TMSG_RC_FEED_REQ, RC_FEED2, MOTOR_STOP, 0, 0);
				break;
			default:
				/* system error ? */
				_main_system_error(0, 125);
				break;
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					switch (ex_main_task_mode2)
					{
					case TEST_ACTIVE_MODE2_TWIN_FEED_FWD:
					case TEST_ACTIVE_MODE2_TWIN_FEED_REV:
					
						_main_send_msg(ID_RC_MBX, TMSG_RC_GET_MOTOR_SPEED_REQ, RC_MOT_FEED1, 0, 0, 0);
						break;
					case TEST_ACTIVE_MODE2_QUAD_FEED_FWD:
					case TEST_ACTIVE_MODE2_QUAD_FEED_REV:
					
						_main_send_msg(ID_RC_MBX, TMSG_RC_GET_MOTOR_SPEED_REQ, RC_MOT_FEED2, 0, 0, 0);
						break;
					default:
						/* error */
						_main_system_error(0, 64);
						break;
					}
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
				}
			}
			break;
		case TMSG_RC_GET_MOTOR_SPEED_RSP:
			// ex_multi_job.busy &= ~TASK_ST_RC;
			if(ex_rc_test_status == RC_BUSY_STATUS)
			{
				ex_rc_test_status = RC_STANDBY_STATUS;
			}

			if(ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			break;
		case TMSG_RC_FEED_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				// alarm
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_position_sensor);
			}
			else
			{
				// error
				_main_system_error(0, 64);
			}
			break;
		case TMSG_RC_STATUS_INFO:
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		default:
			/* 03/04/2025 add default case */
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
			&& (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 64);
			}
			break;
	}
}

void test_rc_flap(void)
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_DLINE_TEST_FINISH_REQ:
			/* test finish */
			ex_test_finish = 1;
			/* immediately change state */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
			break;
		case TMSG_TIMER_TIMES_UP:
			if(ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					ex_rc_aging_seq++;
					
					switch(ex_main_task_mode2)
					{
						case TEST_ACTIVE_MODE2_TWIN_FLAP:
							if (ex_rc_aging_seq > RC_FLAP1_POS_HEAD_TO_BOX)
							{
								ex_rc_aging_seq = RC_FLAP1_POS_HEAD_TO_RC;
							}
							_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, ex_rc_aging_seq, 0, 0, 0);
							break;
						case TEST_ACTIVE_MODE2_QUAD_FLAP:
							if (ex_rc_aging_seq > RC_FLAP2_POS_HEAD_TO_BOX)
							{
								ex_rc_aging_seq = RC_FLAP2_POS_RC_TO_BOX;
							}
							_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, 0, ex_rc_aging_seq, 0, 0);
							break;
						default:
							/* error */
							_main_system_error(0, 64);
							break;
					}
				}
			}
			break;
		case TMSG_RC_FLAPPER_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				// alarm
				/* RC-Twin/Quad error */
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				// error
				/* system error ? */
				_main_system_error(0, 64);
			}

			break;
		case TMSG_RC_STATUS_INFO:
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		default:
			/* 03/04/2025 add default case */
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
			&& (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 64);
			}
			break;
	}
}

void test_rc_sensor(void)
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_DLINE_TEST_FINISH_REQ:
			/* test finish */
			ex_test_finish = 1;
			/* led off*/
			_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
			/* stop drum operation */
			switch(ex_main_task_mode2)
			{
				case TEST_ACTIVE_MODE2_TWIN_POS_SEN1:
					_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_OFF, RC_TWIN_TRANSPORT_POS, 0, 0);					/* To rc_task(TMSG_RC_SENSOR_REQ)		*/
					break;
				case TEST_ACTIVE_MODE2_TWIN_POS_SEN2:
					_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_OFF, RC_TWIN_DRUM_POS, 0, 0);					/* To rc_task(TMSG_RC_SENSOR_REQ)		*/
					break;
				case TEST_ACTIVE_MODE2_QUAD_POS_SEN1:
					_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_OFF, RC_QUAD_TRANSPORT_POS, 0, 0);					/* To rc_task(TMSG_RC_SENSOR_REQ)		*/
					break;
				case TEST_ACTIVE_MODE2_QUAD_POS_SEN2:
					_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_OFF, RC_QUAD_DRUM_POS, 0, 0);					/* To rc_task(TMSG_RC_SENSOR_REQ)		*/
					break;
				//#if defined(UBA_RS)
				case TEST_ACTIVE_MODE2_RC_RS_SEN:		
					_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_OFF, RC_RS_TRANSPORT_POS, 0, 0);				/* To rc_task(TMSG_RC_SENSOR_REQ)		*/						
					break;
				//#endif
				default:
					/* error */
					_main_system_error(0, 64);
					break;
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if(ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					switch(ex_main_task_mode2)
					{
						case TEST_ACTIVE_MODE2_TWIN_POS_SEN1:
							if (ex_rc_status.sst21A.bit.pos_sen1 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_SYAN, 0, 0);
							}
							else if (ex_rc_status.sst21A.bit.pos_sen2 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_BLUE, 0, 0);
							}
							else if (ex_rc_status.sst21A.bit.pos_sen3 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_WHITE, 0, 0);
							}
							else
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
							}
							break;
						case TEST_ACTIVE_MODE2_TWIN_POS_SEN2:
							if (ex_rc_status.sst31A.bit.pos_senA == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_MAGENTA, 0, 0);
							}
							else if (ex_rc_status.sst31A.bit.pos_senB == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_YELLOW, 0, 0);
							}
							else if (ex_rc_status.sst31A.bit.pos_senC == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_RED, 0, 0);
							}
							else
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
							}
							break;
						case TEST_ACTIVE_MODE2_QUAD_POS_SEN1:
							if (ex_rc_status.sst22A.bit.pos_sen4 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_SYAN, 0, 0);
							}
							else if (ex_rc_status.sst22A.bit.pos_sen5 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_BLUE, 0, 0);
							}
							else if (ex_rc_status.sst22A.bit.pos_sen6 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_WHITE, 0, 0);
							}
							else
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
							}
							break;
						case TEST_ACTIVE_MODE2_QUAD_POS_SEN2:
							if (ex_rc_status.sst32A.bit.pos_senD == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_MAGENTA, 0, 0);
							}
							else if (ex_rc_status.sst32A.bit.pos_senE == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_YELLOW, 0, 0);
							}
							else if (ex_rc_status.sst32A.bit.pos_senF == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_RED, 0, 0);
							}
							else
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
							}
							break;
					//#if defined(UBA_RS)
						case TEST_ACTIVE_MODE2_RC_RS_SEN:
							if (ex_rc_status.sst4A.bit.pos_sen1 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_MAGENTA, 0, 0);
							}
							else if (ex_rc_status.sst4A.bit.pos_sen2 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_BLUE, 0, 0);
							}
							else if (ex_rc_status.sst4A.bit.pos_sen3 == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_RED, 0, 0);
							}
							else if (ex_rc_status.sst4A.bit.pos_senR == 1)
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_WHITE, 0, 0);
							}
							else
							{
								_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
							}
							break;
					//#endif // uBA_RS
						default:
							/* error */
							_main_system_error(0, 64);
							break;
					}
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
				}
			}
			break;
		case TMSG_RC_SENSOR_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				// alarm
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				// error
				_main_system_error(0, 64);
			}
			break;
		case TMSG_RC_STATUS_INFO:
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		default:
			/* 03/04/2025 add default case */
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
			&& (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 64);
			}
			break;
	}
}

void test_rc_sol(void)
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_DLINE_TEST_FINISH_REQ:
			/* test finish */
			ex_test_finish = 1;
			switch(ex_main_task_mode2)
			{
				case TEST_ACTIVE_MODE2_TWIN_SOL:
					/* send message to rc_task (TMSG_RC_SOL_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_SOL_REQ, RC_TWIN, RC_SOL_OFF, 0, 0);
					break;
				case TEST_ACTIVE_MODE2_QUAD_SOL:
					/* send message to rc_task (TMSG_RC_SOL_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_SOL_REQ, RC_QUAD, RC_SOL_OFF, 0, 0);
					break;
				default:
					/* error */
					_main_system_error(0, 64);
					break;
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if(ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					if (ex_rc_aging_seq == RC_SOL_ON)
					{
						ex_rc_aging_seq = RC_SOL_OFF;
					}
					else
					{
						ex_rc_aging_seq = RC_SOL_ON;
					}
					switch(ex_main_task_mode2)
					{
						case TEST_ACTIVE_MODE2_TWIN_SOL:
							_main_send_msg(ID_RC_MBX, TMSG_RC_SOL_REQ, RC_TWIN, ex_rc_aging_seq, 0, 0);
							break;
						case TEST_ACTIVE_MODE2_QUAD_SOL:
							_main_send_msg(ID_RC_MBX, TMSG_RC_SOL_REQ, RC_QUAD, ex_rc_aging_seq, 0, 0);
							break;
						default:
							/* error */
							_main_system_error(0, 64);
							break;
					}
					//_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
				}
			}
			break;
		case TMSG_RC_SOL_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					ex_rc_test_status = RC_STANDBY_STATUS;

					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 100, 0, 0);
				}
				
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				// alarm
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				/* error */
				_main_system_error(0, 64);
			}
			break;
		case TMSG_RC_STATUS_INFO:
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		default:
			/* 03/04/2025 add default case */
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
			&& (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 64);
			}
			break;
	}
}

void test_rc_drum(void)
{
	switch (ex_main_msg.tmsg_code)
	{
		case TMSG_DLINE_TEST_FINISH_REQ:
			/* test finish */
			ex_test_finish = 1;
			switch(ex_main_task_mode2)
			{
				case TEST_ACTIVE_MODE2_TWIN_DRUM1:
					/* send message to rc_task (TMSG_RC_DRUM_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_TWIN_DRUM1, RC_DRUM_STOP, 0, 0);
					break;
				case TEST_ACTIVE_MODE2_TWIN_DRUM2:
					/* send message to rc_task (TMSG_RC_DRUM_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_TWIN_DRUM2, RC_DRUM_STOP, 0, 0);
					break;
				case TEST_ACTIVE_MODE2_QUAD_DRUM1:
					/* send message to rc_task (TMSG_RC_DRUM_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_QUAD_DRUM1, RC_DRUM_STOP, 0, 0);
					break;
				case TEST_ACTIVE_MODE2_QUAD_DRUM2:
					/* send message to rc_task (TMSG_RC_DRUM_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_REQ, RC_QUAD_DRUM2, RC_DRUM_STOP, 0, 0);
					break;
				default:
					/* error */
					_main_system_error(0, 64);
					break;
			}
			break;
		case TMSG_TIMER_TIMES_UP:
			if(ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					if(ex_rc_status.sst1A.bit.busy == 1)
					{
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
					}
					else
					{
						switch (ex_main_task_mode2)
						{
						case TEST_ACTIVE_MODE2_TWIN_DRUM1:
							_main_send_msg(ID_RC_MBX, TMSG_RC_GET_MOTOR_SPEED_REQ, RC_MOT_TWIN_DRUM1, 0, 0, 0);
							break;
						case TEST_ACTIVE_MODE2_TWIN_DRUM2:
							_main_send_msg(ID_RC_MBX, TMSG_RC_GET_MOTOR_SPEED_REQ, RC_MOT_TWIN_DRUM2, 0, 0, 0);
							break;
						case TEST_ACTIVE_MODE2_QUAD_DRUM1:
							_main_send_msg(ID_RC_MBX, TMSG_RC_GET_MOTOR_SPEED_REQ, RC_MOT_QUAD_DRUM1, 0, 0, 0);
							break;
						case TEST_ACTIVE_MODE2_QUAD_DRUM2:
							_main_send_msg(ID_RC_MBX, TMSG_RC_GET_MOTOR_SPEED_REQ, RC_MOT_QUAD_DRUM2, 0, 0, 0);
							break;
						default:
							/* error */
							_main_system_error(0, 64);
							break;
						}
					}
					
					//_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
				}
			}
			break;
		case TMSG_RC_DRUM_RSP:
			if(ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				if (ex_test_finish)
				{
					_main_set_test_standby();
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
				}
			}
			else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
			{
				// alarm
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			else
			{
				// error
				_main_system_error(0, 125);
			}
			break;
		case TMSG_RC_GET_MOTOR_SPEED_RSP:
			// ex_multi_job.busy &= ~TASK_ST_RC;
			if(ex_rc_test_status == RC_BUSY_STATUS)
			{
				ex_rc_test_status = RC_STANDBY_STATUS;
			}
			break;
		case TMSG_RC_STATUS_INFO:
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			break;
		case TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
		default:
			/* 03/04/2025 add default case */
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
			&& (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 64);
			}
			break;
	}
}

void mode_test_active_rc3_proc()
{

	switch(ex_rc_test_type)
	{
	case DIPSWRC_WRITE_SERIAL_NO:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_WRITE_SERIALNO);

		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_WRITE_SERIALNO_REQ, 
						ex_rc_test_maintenance, 0, 0, 0);
		break;
	case DIPSWRC_READ_SERIAL_NO:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_READ_SERIALNO);

		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_READ_SERIALNO_REQ, 
						ex_rc_test_maintenance, 0, 0, 0);

		break;
	case DIPSWRC_START_SENS_ADJ:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_START_SENS_ADJ);

		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_START_SENS_ADJ_REQ, 
						ex_rc_test_maintenance, 0, 0, 0);
		break;
	case DIPSWRC_GET_SENS_ADJ_DATA:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_READ_SENS_ADJ_DATA);

		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_READ_SENS_ADJ_DATA_REQ, 0, 0, 0, 0);
		break;
	case DIPSWRC_TWIN_FLAP_USB:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_TWIN_FLAP_USB);

		ex_rc_test_status = RC_BUSY_STATUS;

		/* send message to ra_task (TMSG_RC_FLAPPER_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, 0, 0, 1, 0);
		break;
	case DIPSWRC_QUAD_FLAP_USB:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_QUAD_FLAP_USB);

		ex_rc_test_status = RC_BUSY_STATUS;

		/* send message to ra_task (TMSG_RC_FLAPPER_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_FLAPPER_REQ, 0, 0, 2, 0);
		break;
	case DIPSWRC_DRUM1_TAPE_POS_ADJ:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_DRUM1_TAPE_POS_ADJ);

		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_TAPE_POS_ADJ_REQ, 1, 0, 0, 0);
		break;
	case DIPSWRC_DRUM2_TAPE_POS_ADJ:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_DRUM2_TAPE_POS_ADJ);

		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_DRUM_TAPE_POS_ADJ_REQ, 2, 0, 0, 0);
		break;
	case DIPSWRC_FRAM_CHECK:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_FRAM_CHECK);

		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_FRAM_CHECK_REQ, 0, 0, 0, 0);
		break;
	case DIPSWRC_SENS_ADJ_WRITE_FRAM:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SENS_ADJ_WRITE_FRAM);

		/* send message to rc_task (TMSG_RC_SENS_ADJ_WRITE_FRAM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_SENS_ADJ_WRITE_FRAM_REQ, ex_rc_test_maintenance, 0, 0, 0);
		break;
	case DIPSWRC_SENS_ADJ_READ_FRAM:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SENS_ADJ_READ_FRAM);

		/* send message to rc_task (TMSG_RC_SENS_ADJ_READ_FRAM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_SENS_ADJ_READ_FRAM_REQ, ex_rc_test_maintenance, 0, 0, 0);
		break;
	case DIPSWRC_PERFORM_TEST_WRITE_FRAM:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_PERFORM_TEST_WRITE_FRAM);

		/* send message to rc_task (TMSG_RC_PERFORM_TEST_WRITE_FRAM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_PERFORM_TEST_WRITE_FRAM_REQ, 0, 0, 0, 0);
		break;
	case DIPSWRC_PERFORM_TEST_READ_FRAM:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_PERFORM_TEST_READ_FRAM);

		/* send message to rc_task (TMSG_RC_PERFORM_TEST_READ_FRAM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_PERFORM_TEST_READ_FRAM_REQ, 0, 0, 0, 0);
		break;
	case DIPSWRC_WRITE_EDITION_NO:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_WRITE_EDITIONNO);
		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_WRITE_EDITION_REQ, ex_rc_test_maintenance, 0, 0, 0);
		break;
	case DIPSWRC_READ_EDITION_NO:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_READ_EDITIONNO);
		/* send message to rc_task (TMSG_RC_DRUM_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_READ_EDITION_REQ, ex_rc_test_maintenance, 0, 0, 0);
		break;
	default:
		/* system error ? */
		_main_system_error(0, 123);
		break;
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_serial_no(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_WRITE_SERIALNO_RSP:
		_main_set_test_standby();
		break;
	case TMSG_RC_READ_SERIALNO_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{

			read_serailno_data.read_end &= ~(READ_RTQ_EXEC);
			read_serailno_data.read_end |= READ_RTQ_END;

			_main_set_test_standby();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			/* wait 5sec */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0);
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
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_sens_adj(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_START_SENS_ADJ_RSP:
		/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
		/* wait 100msec */
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
		break;
	case TMSG_RC_READ_SENS_ADJ_DATA_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if (ex_sens_adj_end == ADJ_READ_EXEC)
			{
				ex_sens_adj_end = ADJ_READ_END;
				_main_set_test_standby();
			}
			else
			{
				/* system error ? */
				_main_system_error(0, 125);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			ex_sens_adj_end = ADJ_ERR;
			_main_set_test_standby();
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			else
			{
				if (ex_rc_status.sst1A.bit.busy == 1)
				{
					/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
					/* wait 500mssec */
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 50, 0, 0);
				}
				else
				{
					switch (ex_main_task_mode2)
					{
					case TEST_ACTIVE_MODE2_START_SENS_ADJ:
						if (ex_sens_adj_end == ADJ_EXEC)
						{
							ex_sens_adj_end = ADJ_WRITE_END;
						}
						else
						{
							_main_system_error(0, 125);
						}
						_main_set_test_standby();
						break;
					case TEST_ACTIVE_MODE2_READ_SENS_ADJ_DATA:
						/* system error ? */
						_main_system_error(0, 125);
						break;
					default:
						/* system error ? */
						_main_system_error(0, 125);
						break;
					}
				}
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
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_flap_usb(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		/* test finish */
		ex_main_test_no = TEST_STANDBY;

		/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
		/* wait 100msec */
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			else
			{
				if (ex_rc_status.sst1A.bit.busy == 1)
				{
					/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
					/* wait 5sec */
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 5, 0, 0);
				}
				else
				{
					ex_rc_flap_test_time_send = ex_rc_flap_test_time;
					ex_rc_test_status = RC_STANDBY_STATUS;
					ex_rc_flap_test_flg = 0;

					_main_set_test_standby();
				}
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RC_FLAPPER_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_rc_flap_test_time = 0;
			ex_rc_flap_test_flg = 1;
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			/* wait 5msec */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 5, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_drum_tape_pos_adj(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_DRUM_TAPE_POS_ADJ_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* wait 100msec */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 100, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			else
			{
				if (ex_rc_status.sst1A.bit.busy == 1)
				{
					/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
					/* wait 500msec */
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0);
				}
				else
				{
					ex_test_end_flg = 1;
					_main_set_test_standby();
				}
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_fram_check(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_FRAM_CHECK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_test_end_flg = 1;
			_main_set_test_standby();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_sens_adj_write_fram(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_SENS_ADJ_WRITE_FRAM_RSP:
		if (ex_sens_adj_fram_end == ADJ_EXEC)
		{
			ex_sens_adj_fram_end = ADJ_WRITE_END;

			ex_test_end_flg = 1;
			_main_set_test_standby();
		}
		else
		{
			ex_sens_adj_fram_end = ADJ_ERR;
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_sens_adj_read_fram(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_SENS_ADJ_READ_FRAM_RSP:
		if (ex_sens_adj_fram_end == ADJ_READ_EXEC)
		{
			ex_sens_adj_fram_end = ADJ_READ_END;

			ex_test_end_flg = 1;
			_main_set_test_standby();
		}
		else
		{
			ex_sens_adj_fram_end = ADJ_ERR;
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_perform_test_write_fram(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_PERFORM_TEST_WRITE_FRAM_RSP:
		if (ex_perform_test_fram_end == PTEST_EXEC)
		{
			ex_perform_test_fram_end = PTEST_WRITE_END;

			ex_test_end_flg = 1;
			_main_set_test_standby();
		}
		else
		{
			ex_perform_test_fram_end = PTEST_ERR;
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_perform_test_read_fram(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_PERFORM_TEST_READ_FRAM_RSP:
		if (ex_perform_test_fram_end == PTEST_READ_EXEC)
		{
			ex_perform_test_fram_end = PTEST_READ_END;

			ex_test_end_flg = 1;
			_main_set_test_standby();
		}
		else
		{
			ex_perform_test_fram_end = PTEST_ERR;
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************/ 
/**
* @brief drum test(RC-Twin/Quad)
* @param[in]	None
* @return 		None
**********************************************************************/
void test_rc_edition_no(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_WRITE_EDITION_RSP:
		_main_set_test_standby();
		break;
	case TMSG_RC_READ_EDITION_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			read_editionno_data.read_end &= ~(READ_RTQ_EXEC);
			read_editionno_data.read_end |= READ_RTQ_END;
			_main_set_test_standby();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_RC_WRITE_SERIALNO_RSP:
		_main_set_test_standby();
		break;
	case TMSG_RC_READ_SERIALNO_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS) //2024-10-17 おそらく使用していないはず
		{
		//	read_editionno_data.read_end &= ~(READ_RTQ_END);
		//	read_editionno_data.read_end |= READ_RTQ_END;
			_main_set_test_standby();
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			/* wait 5sec */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0);
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
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

//#if defined(RC_BOARD_GREEN)
void mode_test_active_rs_sh3_usb_proc()
{
	switch(ex_rc_test_type)
	{
//#if defined(UBA_RS)
	case DIPSWRC_RS_FLAP:
		/* RSユニット無し, 旧基板はエラーにする */
		if (ex_rc_configuration.unit_type == RS_NOT_CONNECT || ex_rc_configuration.board_type == RC_OLD_BOARD)
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_RC_TRANSPORT, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_RS_FLAP_USB);

			ex_rc_test_status = RC_BUSY_STATUS;

			/* set first sequence */
			ex_rc_aging_seq = RS_FLAP_POS_OUT;

			/* Step1 出金位置に移動する */
			/* send message to ra_task (TMSG_RS_FLAPPER_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, ex_rc_aging_seq, 0, 0, 0);
		}
		break;
	case DIPSWRC_RS_SEN:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_RS_SEN);

		/* send message to rc_task (TMSG_RC_SENSOR_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_RS_TRANSPORT_POS, 0, 0);
//#endif
	case DIPSWRC_GET_POS_ONOFF:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_WAIT_SENSOR_ADJ);
		_main_send_msg(ID_RC_MBX, TMSG_RC_GET_POS_ONOFF_REQ, 0, 0, 0, 0);
		break;
	case DIPSWRC_SET_POS_DA:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_WAIT_SENSOR_ADJ);
		_main_send_msg(ID_RC_MBX, TMSG_RC_SET_POS_DA_REQ, 0, 0, 0, 0);
		break;
	case DIPSWRC_SET_POS_GAIN:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_WAIT_SENSOR_ADJ);
		_main_send_msg(ID_RC_MBX, TMSG_RC_GET_POS_GAIN_REQ, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void test_rc_wait_sensor_adj()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_GET_POS_ONOFF_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_RC_GET_POS_DA_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 100, 0, 0);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_RC_GET_POS_GAIN_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 100, 0, 0);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			else
			{
				if (ex_rc_status.sst1A.bit.busy == 1)
				{
					/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
					/* wait 500msec */
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 20, 0, 0);
				}
				else
				{
					ex_test_end_flg = 0;
					_main_set_test_standby();
				}
			}
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}
//#endif // RC_BOARD_GREEN


#if defined(UBA_RTQ) //#if defined(UBA_RTQ_ICB)
void mode_test_active_rs_sh3_proc()
{
	u8 rx_test_type;

	if (ex_main_test_no == TEST_RS)
	{
		rx_test_type = ex_rc_dip_sw;
	}
	else
	{
		/* system error ? */
		_main_system_error(0, 123);
	}
	switch(rx_test_type)
	{
	//#if defined(UBA_RS)
	case DIPSWRC_RS_FLAP:
		/* RSユニット無し, 旧基板はエラーにする */
		if (ex_rc_configuration.unit_type == RS_NOT_CONNECT || ex_rc_configuration.board_type == RC_OLD_BOARD)
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_RC_TRANSPORT, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_RS_FLAP);

			ex_rc_test_status = RC_BUSY_STATUS;

			/* set first sequence */
			ex_rc_aging_seq = RS_FLAP_POS_IN;

			/* send message to ra_task (TMSG_RS_FLAPPER_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, ex_rc_aging_seq, 0, 0, 0);
		}
		break;
	case DIPSWRC_RS_SEN:
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_RS_SEN);

		/* send message to rc_task (TMSG_RC_SENSOR_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_SENSOR_REQ, RC_SENSOR_ON, RC_RS_TRANSPORT_POS, 0, 0);
		break;
	//#endif
	case DIPSWRC_RC_RFID:  //生産で使用
		_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_RC_RFID);
		_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_TEST_REQ, RFID_TEST_START, 0, 0, 0);
		break;
	default:
		break;
	}
}

void test_rc_rfid()  //生産で使用
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:	//2025-01-25
		/* test finish */
		ex_main_test_no = TEST_STANDBY;

		_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_TEST_REQ, RFID_TEST_END, 0, 0, 0);
		_main_set_test_standby();	
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_RFID_TEST_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{

		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);	
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
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}
#endif // UBA_RTQ


void test_rc_rs_flap()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		/* test finish */
		ex_main_test_no = TEST_STANDBY;

		/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
		/* wait 100msec */
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			else
			{
				if (ex_rc_status.sst1A.bit.busy == 1)
				{
					/* まだ動作中の場合はさらに5sec延長 */
					/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
					/* wait 5sec */
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0);
				}
				else
				{
					switch (ex_rc_aging_seq)
					{
					case RS_FLAP_POS_IN:
						ex_rc_aging_seq = RS_FLAP_POS_OUT;
						break;
					case RS_FLAP_POS_OUT:
						ex_rc_aging_seq = RS_FLAP_POS_IN;
						break;
					default:
						/* system error ? */
						_main_system_error(0, 125);
						break;
					}

					_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, ex_rc_aging_seq, 0, 0, 0);
				}
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RS_FLAPPER_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			/* wait 5sec */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 500, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RS error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

void test_rc_rs_flap_usb()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		/* test finish */
		ex_main_test_no = TEST_STANDBY;

		/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
		/* wait 100msec */
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if (ex_main_test_no == TEST_STANDBY)
			{
				_main_set_test_standby();
			}
			else
			{
				if (ex_rc_status.sst1A.bit.busy == 1)
				{
					/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
					/* wait 5sec */
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 5, 0, 0);
				}
				else
				{
					switch (ex_rc_aging_seq)
					{
					case RS_FLAP_POS_OUT:
						if (!(RS_FLAP_OUT_POS))
						{
							_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ALARM_CODE_RC_TIMEOUT, _main_conv_seq(), ex_position_sensor);
						}
						else
						{
							/* Step2 入金位置へ移動する(時間計測する) */
							ex_rc_aging_seq = RS_FLAP_POS_IN;
							ex_rc_flap_test_time = 0;
							ex_rc_flap_test_flg = 1;

							_main_send_msg(ID_RC_MBX, TMSG_RS_FLAPPER_REQ, ex_rc_aging_seq, 0, 0, 0);
						}
						break;
					case RS_FLAP_POS_IN:
						/* Step3 計測時間を保存して終了 */
						ex_rc_flap_test_time_send = ex_rc_flap_test_time;
						ex_rc_test_status = RC_STANDBY_STATUS;
						ex_rc_flap_test_flg = 0;

						_main_set_test_standby();
						break;
					default:
						/* system error ? */
						_main_system_error(0, 125);
						break;
					}
				}
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RS_FLAPPER_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* send message to timer_task (TMSG_TIMER_SET_TIMER) */
			/* wait 5msec */
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 5, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* RC-Twin/Quad error */
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		/* RC-Twin/Quad error */
		_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}


#endif // UBA_RTQ

/* EOF */
