/******************************************************************************/
/*! @addtogroup Main
    @file       mode_test_idle.c
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
#include "custom.h"
#include "common.h"
#include "operation.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"



/************************** EXTERNAL VARIABLES *************************/

/*********************************************************************//**
 * @brief test standby message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_standby_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case TEST_STANDBY_MODE2_SENSOR_INIT:
		test_init();
		break;
	case TEST_STANDBY_MODE2_MAG_WRITE:
		test_mag_write();
		break;
	case TEST_STANDBY_MODE2_STANDBY:
		test_standby();
		break;
	case TEST_STANDBY_MODE2_AGING_INIT_WAIT:
		test_aging_sensor_init_wait();
		break;
	case TEST_STANDBY_MODE2_WAIT:
		test_wait();
		break;
	case TEST_STANDBY_MODE2_ALARM:
		test_alarm();
		break;
	case TEST_STANDBY_MODE2_DIPSW:
		test_dipsw();
		break;
	default:
		/* system error ? */
		_main_system_error(0, 50);
		break;
	}
}

/*********************************************************************//**
 * @brief test sensor init procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_init(void)
{

	u16 bill_remain;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_SENSOR_INIT_RSP:
		bill_remain = _main_bill_remain();

		_main_send_connection_task(TMSG_CONN_INITIAL, ex_operating_mode, TMSG_SUB_SUCCESS, bill_remain, 0);
		_main_set_test_standby();
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_send_connection_task(TMSG_CONN_INITIAL, ex_operating_mode, TMSG_SUB_ALARM, ex_main_msg.arg2, 0);
			_main_set_test_standby();
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 51);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief test standby
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_standby(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		_main_display_test_standby();
		break;
	case TMSG_DLINE_TEST_REQ:
		switch (ex_main_msg.arg1)
		{
		case TEST_DIPSW1:
			_main_set_mode(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_DIPSW);
			ex_main_test_no = ex_main_msg.arg1;
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, 200, 0, 0); //2024-03-28 UBA500に合わせる
			break;
		// active test
		case TEST_AGING:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			_main_set_test_active();
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
			break;
		//case TEST_AGING_LD:
		//	ex_main_test_no = ex_main_msg.arg1;
		//	set_test_ld_mode(1);
		//	_main_set_test_active();
		//	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
		//	break;
		case TEST_ACCEPT://2021-07-13 yuji add validation
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			set_test_allacc_mode(0);
			_main_set_test_active();
			break;
		case TEST_ACCEPT_ALLACC:

			/*2022-01-19 */
			_pl_cis_enable_set(1);
			dly_tsk(100);
		
			ex_main_test_no = ex_main_msg.arg1;
		#if defined(UBA_RTQ)
			ex_rc_test_type = ex_main_msg.arg2;
			ex_rc_test_maintenance = ex_main_msg.arg3;
		#endif

			set_test_ld_mode(0);
			set_test_allacc_mode(1);
			_main_set_test_active();
			break;
		case TEST_ACCEPT_LD://2021-07-13 yuji add validation
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(1);
			set_test_allacc_mode(0);
			_main_set_test_active();
			break;
		case TEST_ACCEPT_LD_ALLACC:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(1);
			set_test_allacc_mode(1);
			_main_set_test_active();
			break;
		case TEST_REJECT:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			set_test_allrej_mode(1);
			_main_set_test_active();
			break;
		case TEST_REJECT_CENT_OPEN:	//2024-12-01 生産CIS初期流動に使用
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			set_test_allrej_mode(1);
			_main_set_test_active();
			break;
		case TEST_REJECT_LD:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(1);
			set_test_allrej_mode(1);
			_main_set_test_active();
			break;
		case TEST_SENSOR:
			/* ad start */
			ex_main_test_no = ex_main_msg.arg1;
			_main_set_test_active();
			break;
		case TEST_STACKER:
		case TEST_STACKER_HOME:
			if(!SENSOR_PUSHER_HOME)
			{
				ex_main_test_no = TEST_STACKER_HOME;
				_main_set_test_active();
			}
			else
			{
				ex_main_test_no = TEST_STACKER;
				_main_set_test_active();
			}
			break;
		case TEST_FEED_MOTOR_FWD:
		case TEST_FEED_MOTOR_REV:
		case TEST_STACKER_MOTOR_FWD:
		case TEST_CENTERING:
		case TEST_CENTERING_CLOSE:	
		case TEST_APB:
		case TEST_APB_CLOSE:
		case TEST_SHUTTER:
		case TEST_SHUTTER_CLOSE:	
		case TEST_RFID_UBA:
	
			ex_main_test_no = ex_main_msg.arg1;
			_main_set_test_active();
			break;
#if defined(UBA_RTQ)
		case TEST_RC1:	/* DIP 1, 2, 5 */
		case TEST_RC2:	/* DIP 1, 2, 6 */
		case TEST_RC1_USB:
		case TEST_RC2_USB:
		case TEST_RC3_USB:
		case TEST_RC_AGING_FACTORY:
		case TEST_RC_AGING:
	//#if defined(RC_BOARD_GREEN)
		case TEST_RS_USB:
	//#endif // RC_BOARD_GREEN
	//#if defined(UBA_RTQ_ICB)
		case TEST_RS:
	//#endif // UBA_RTQ_ICB
			ex_main_test_no = ex_main_msg.arg1;
			ex_rc_test_type = ex_main_msg.arg2;
			ex_rc_test_maintenance = ex_main_msg.arg3;

			if (ex_main_test_no == TEST_RC_AGING_FACTORY || 
				ex_main_test_no == TEST_RC_AGING)
			{
				set_test_allacc_mode(1);
			}
		
			_main_set_test_active();
			break;
#endif // UBA_RTQ
		default:
			break;
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_DLINE_TEST_DIPSW_REQ: //DIP-SW 1,2,5,8か1,2,6,8の状態の場合定期的に受信する
		_main_send_msg(ID_RC_MBX, TMSG_RC_GET_DIPSW_REQ, 0, 0, 0, 0);
		break;
	case TMSG_RC_GET_DIPSW_RSP:
		_main_send_connection_task(TMSG_DLINE_TEST_DIPSW_RSP, 0, 0, 0, 0);
		break;
#endif // UBA_RTQ

	case TMSG_SENSOR_TEMP_ADJ_RSP:
#ifndef DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE
		//2024-01-11 おそらく使用していない。
		if((ex_main_msg.arg1 ==TEMP_ADJ_SUCCESS) || (ex_main_msg.arg1 ==TEMP_ADJ_OVER_RUN))
		{
			write_fram_tmpadj();
		}
		else if((ex_main_msg.arg1 ==TEMP_ADJ_SENSOR_SHIFT) || (ex_main_msg.arg1 ==TEMP_ADJ_ERROR))
		{
			// error or cancel
		}
#endif
		_main_set_sensor_active(0);
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_STANDBY_RSP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	//#if UBA_RFID デバッグ用
	case TMSG_RFID_RESET_RSP:
		// Front USBのコマンドからのRFID読み書きテスト
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_hal_i2c3_for_led_tca9535(1, 0x10);	/* green on*/
		}
		else
		{
			_hal_i2c3_for_led_tca9535(1, 0x08);	/* red on */
		}
		break;
	//#endif
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		break;
#endif 

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 52);
		}
		break;
	}
}


/******************************************************************************/
/*! @brief MAG WRIE test procedure
    @return         none
    @exception      none
******************************************************************************/
void test_mag_write(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		_main_set_test_standby();
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_COMMON_STATUS) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	}
}
/******************************************************************************/
/*! @brief DIPSW test procedure
    @return         none
    @exception      none
******************************************************************************/
void test_dipsw_uba(void);
void test_dipsw(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		_main_set_test_standby();
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_TEMP_ADJ)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
		}
		else if(ex_main_msg.arg1 == TIMER_ID_DIPSW_READ)
		{
			test_dipsw_uba(); //2024-03-28
		}
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_COMMON_STATUS) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_ALARM, TMSG_CONN_TEST, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		break;
	}
}
/*********************************************************************//**
 * @brief aging feed init wait
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_aging_sensor_init_wait(void)
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
		if(ex_test_finish)
		{
			_main_set_test_standby();
		}
		else
		{
			if (_is_main_position_all_off())
			{
				ex_main_test_no = TEST_AGING;
				_main_set_test_active();
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
			_main_system_error(0, 54);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief test aging wait
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_wait(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_AGING_WAIT, 0, 0, 0);
		_main_set_test_standby();
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_DLINE_TEST_REQ:
		switch (ex_main_msg.arg1)
		{
		case TEST_DIPSW1:
			_main_set_mode(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_DIPSW);
			ex_main_test_no = ex_main_msg.arg1;
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, 200, 0, 0); //2024-03-28 UBA500に合わせる
			break;
		// active test
		case TEST_ACCEPT:
		case TEST_AGING:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			_main_set_test_active();
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
			break;
		case TEST_ACCEPT_LD:
		//case TEST_AGING_LD:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(1);
			_main_set_test_active();
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
			break;
		case TEST_ACCEPT_ALLACC:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			set_test_allacc_mode(1);
			_main_set_test_active();
			break;
		case TEST_ACCEPT_LD_ALLACC:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(1);
			set_test_allacc_mode(1);
			_main_set_test_active();
			break;
		case TEST_REJECT:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			set_test_allrej_mode(1);
			_main_set_test_active();
			break;
		case TEST_REJECT_LD:
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(1);
			set_test_allrej_mode(1);
			_main_set_test_active();
			break;
		case TEST_REJECT_CENT_OPEN: //2024-12-01 生産CIS初期流動に使用
			ex_main_test_no = ex_main_msg.arg1;
			set_test_ld_mode(0);
			set_test_allrej_mode(1);
			_main_set_test_active();
			break;
		case TEST_SENSOR:
			/* ad start */
			ex_main_test_no = ex_main_msg.arg1;
			_main_set_test_active();
			break;
		case TEST_STACKER:
		case TEST_STACKER_HOME:
			if(!SENSOR_PUSHER_HOME)
			{
				ex_main_test_no = TEST_STACKER_HOME;
				_main_set_test_active();
			}
			else
			{
				ex_main_test_no = TEST_STACKER;
				_main_set_test_active();
			}
			break;
		case TEST_FEED_MOTOR_FWD:
		case TEST_FEED_MOTOR_REV:
		case TEST_STACKER_MOTOR_FWD:
		case TEST_APB:
		case TEST_CENTERING:
		case TEST_CENTERING_CLOSE:	
		case TEST_APB_CLOSE:
		case TEST_SHUTTER:
		case TEST_SHUTTER_CLOSE:
		case TEST_RFID_UBA:
	
			ex_main_test_no = ex_main_msg.arg1;
			_main_set_test_active();
			break;
		default:
			break;
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_AGING_WAIT)
		{
			if(ex_test_finish)
			{
				_main_set_test_standby();
			}
			else
			{
				switch (ex_main_test_no)
				{
				case TEST_STACKER:
				case TEST_STACKER_HOME:
					if(!SENSOR_PUSHER_HOME)
					{
						ex_main_test_no = TEST_STACKER_HOME;
						_main_set_test_active();
					}
					else
					{
						ex_main_test_no = TEST_STACKER;
						_main_set_test_active();
					}
					break;

				case TEST_APB:
					ex_main_test_no = TEST_APB;
					_main_set_test_active();
					break;
				case TEST_APB_CLOSE:
					ex_main_test_no = TEST_APB_CLOSE;
					_main_set_test_active();
					break;
				/* DIP-SWでの定期的な動作場合、ここで再設定して起動させる */
				case TEST_SHUTTER:
				#if 1 /* 2022-01-18 */
					_main_set_test_active();
				#else
					_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SHUTTER);
					_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_INITIAL_OPEN_REQ, 0, 0, 0, 0);
				#endif
					break;
				case TEST_CENTERING:
				#if 1 /* 2022-01-18 */
					_main_set_test_active();
				#else
					_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_CENTERING_OPEN_UBA);
					_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_FREERUN_REQ, 0, 0, 0, 0);
				#endif
					break;
				//closeは生産の閉じ切りテストで使用するので、閉じっぱなしの方がいいはず、
				//case TEST_CENTERING_CLOSE:
					//_main_set_mode(MODE1_TEST, TEST_MODE2_CENTERING_CLOSE);
				//	_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_CENTERING_CLOSE_UBA);
				//	_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_CLOSE_REQ, 0, 0, 0, 0);
				//	break;

				case TEST_AGING:
				//case TEST_AGING_LD:
					if (!_is_main_position_all_off())
					{
						if (!(ex_multi_job.reject) && !(_is_main_position_all_off()))
						{
							ex_multi_job.reject = TASK_ST_MAIN;
							_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER, 0, 0, 0);
						}
						_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_AGING_WAIT, 10, 0, 0);
					}
					else
					{
						_main_set_test_active();
						_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_AGING_SENSOR_ACTIVE);
						_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
					}
					break;
				default:
					_main_set_test_standby();
					break;
				}
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
			_main_system_error(0, 55);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief test alarm
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void test_alarm(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_TEST_FINISH_REQ:
		_main_set_test_standby();
		break;
	case TMSG_DLINE_TEST_REQ:
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 56);
		}
		break;
	}
}

void test_dipsw_uba(void)
{

	u8 dipsw=0;
	// iPROは DIP-SW 8 OFFの状態で
	// 2,4,6 OFFで緑点灯（1,3,5,7 ON）
	// 1,3,5,7 OFFで赤点灯(2,4,6 ON)
	dipsw = ex_dipsw1; //ok

	if (dipsw == 0x55)	// 2,4,6 OFFで緑点灯（1,3,5,7 ON）
	{
		_hal_i2c3_for_led_tca9535(1, 0x10); //green
	}
	else if (dipsw == 0x2A) // 1,3,5,7 OFFで赤点灯(2,4,6 ON)
	{
		_hal_i2c3_for_led_tca9535(1, 0x08); //red
	}
	else
	{
	// その他はLED消灯
		_hal_i2c3_for_led_tca9535(0, 0x10); //green
		_hal_i2c3_for_led_tca9535(0, 0x08); //red
	}
	_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, 200, 0, 0); //2024-03-28 UBA500に合わせる
}

void test_set_timer_init() //今は使用していないが、ICBの30分書き込み用
{
	if (is_icb_enable())
	{
		if ((ex_rfid_setTime_cnt >= 600) && (ex_rfid_flag_setTimeInit_done))
		{
			if (!is_box_set())
			{
				// error
				//_main_system_error(0, 55);
				ex_rfid_flag_setTimeInit_done = 0;
			}
			else
			{
				ex_rfid_flag_setTimeInit_done = false;
				// set_recovery_step(RECOVERY_STEP_ICB_ACCEPT);
			#if !defined(UBA_RTQ) //RTQはカウントアップのみ
				_main_send_msg(ID_ICB_MBX, TMSG_ICB_SET_TIME_REQ, 0, 0, 0, 0);
			#endif
			}

			ex_rfid_setTime_cnt = 0;
		}
	}
}

/* EOF */
