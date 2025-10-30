/******************************************************************************/
/*! @addtogroup Main********************************/
/*! @addtogroup Group2
    @file       operation_sub.c
    @brief      operation sub process
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
#include "sensor.h"
#include "sensor_ad.h"
#include "pl/pl.h"
#include "pl/pl_cis.h"
#include "pl/pl_gpio.h"
#include "pl/pl_encoder.h"
#include "pl/pl_motor.h"
#include "cyc.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "cis_ram.c"

#include "jdl_conf.h"
#if defined(UBA_RTQ)
#include "task/cline_task/003/id003.h"
#endif

#if defined(UBA_RTQ)
#include "if_rc.h"
#include "status_tbl.h"
#endif // UBA_RTQ



/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/
void _main_temp_adjust_enable(void);
void _main_temp_adjust_disable(void);
/************************** EXTERN FUNCTIONS *************************/
extern u32 is_cis_pga_max(void);
#if defined(_PROTOCOL_ENABLE_ID003)
extern u8  _is_id003_escrow_data(u16 denomi_code);
#endif
extern int check_pl_state(void);

#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
extern void save_phase_fram_log(void);
extern void save_debug_fram_log(int num);
#endif

#if defined(UBA_RTQ)
void set_rtq_jam(u32 code, u8 ex_main_emergency_flag_bk);

extern u16 _main_judge_bill_remain_normal_model(void);
extern u16 _main_judge_bill_remain_rs_model(void);
extern u16 _main_judge_bill_in_normal_model(void);
extern u16 _main_judge_bill_in_rs_model(void);
extern u16 _main_stay_bill_check_rs_model(void);
extern u16 _main_stay_bill_check_normal_model(void);

#endif
/************************** EXTERNAL VARIABLES *************************/


/*******************************
     position sensoer D/A
 *******************************/
void _main_set_position_da(void)
{
	set_position_da();
}

/*******************************
     position sensoer GAIN
 *******************************/
void _main_set_position_gain(void)
{
	if (0 != set_position_ga())
	{
		/* system error */
		_main_system_error(1, 255);
	}
}
/*********************************************************************//**
 * @brief set power up serch bill
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_set_powerup_search(void)
{
	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);
	/* FEED LED ON */
	_pl_sen_feed_encoder_LED_set(1);
	_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_SENSOR_ACTIVE);
	/* Set sensor mode : active */
	_main_set_sensor_active(1);
	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
}
/*********************************************************************//**
 * @brief set initialize
 * @param[in]	type : initialize type
 * @return 		None
 **********************************************************************/
void _main_set_init(void)
{
#if defined(UBA_RTQ)
	if(ex_rc_status.sst1A.bit.error)
	{
		_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);
	}

	ex_main_payout_flag = 0;
	ex_main_collect_flag = 0;
	ex_rc_error_flag = 0;
	ex_rc_error_status = 0;
	ex_rc_warning_status = 0;
	ex_rc_collect_sw = 0;
	ex_rc_exchanged_unit = 0;
	ex_main_emergency_flag = 0;
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
	ex_rtq_rfid_write_disable = 0;
	#endif
	ex_rc_timeout_error = 0;	//2025-07-22
#endif // UBA_RTQ
	
	ex_main_reset_flag = 0;
	ex_main_reject_flag = 0;
	
	#if defined(_PROTOCOL_ENABLE_ID003)//#if defined(UBA_ID003_ENC)
	if(ex_main_secret_number_change_flag==1)
	{
		update_random_seed(*(unsigned int *)&ex_adjustment_data.maintenance_info.serial_no[8]); //2025-03-28
	}
	#endif
	
	clear_ex_multi_job();
	memset(&ex_multi_job_alarm_backup, 0, sizeof(ex_multi_job_alarm_backup));

	if (!(is_box_set()))
	{
		_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_BOX_INIT, _main_conv_seq(), ex_position_sensor);
	}
	#if defined(UBA_RTQ) //2025-06-20
	else if(ex_rc_timeout == 1)
	{
		ex_rc_timeout_error = 1; //2025-07-22
		_main_alarm_sub(0, 0, TMSG_CONN_RESET, ALARM_CODE_RC_COM, _main_conv_seq(), ex_position_sensor);
	}
	#endif
	else
	{
	#ifdef _ENABLE_JDL
		jdl_dev_reset();
	#endif	/* _ENABLE_JDL */

	//#if (_DEBUG_CIS_AS_A_POSITION==1) //2024-02-15 イニシャル前に、CIS監視停止 本来は監視したいが、AD関係なので
		_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
		//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
		dly_tsk(5);
	//#endif
	#if defined(UBA_RTQ)
		ex_multi_job.busy = 0;
	#endif // UBA_RTQ
		/* variables initialize */
		ex_abnormal_code    = 0;
		/* Set FPGA mode : active */
		_main_set_pl_active(PL_ENABLE);
	
		_pl_cis_enable_set(1);
	
		/* FEED LED ON */
		_pl_sen_feed_encoder_LED_set(1);
		_main_set_mode(MODE1_INIT, INIT_MODE2_SENSOR_ACTIVE);
		/* Set sensor mode : active */
		_main_set_sensor_active(1);

		_main_display_init();
		// Temp Adjustment Timer Stop
		_main_temp_adjust_disable();

		#if defined(UBA_RTQ)
		_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_INITIALIZE, 0, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);//2025-01-18	
		#endif // UBA_RTQ

		change_ad_sampling_mode(AD_MODE_MAG_OFF); //2022-06-29
	}
}



/*********************************************************************//**
 * @brief set enable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_temp_adjust_enable(void)
{
	_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_TEMP_ADJ, WAIT_TIME_TEMP_ADJ_SKIP, 0, 0);
}
/*********************************************************************//**
 * @brief set disable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_temp_adjust_disable(void)
{
	_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_TEMP_ADJ, 0, 0, 0);
}

/******************************************************************************/
/*! @brief set enable
    @return         none
    @exception      none
******************************************************************************/
void _main_set_disable(void)
{
	if (!(is_box_set()))
	{
		_main_alarm_sub(0, 0, TMSG_CONN_DISABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
	}
	else
	{
	//#if (_DEBUG_CIS_AS_A_POSITION==1) //2024-02-15 待機状態に移行するので、CIS監視停止
		_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
		//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
		dly_tsk(5);
	//#endif
		/* Set FPGA mode : standby */
		_main_set_pl_active(PL_DISABLE);
		/* Set sensor mode : standby */
		_main_set_sensor_active(0);
		_main_set_mode(MODE1_DISABLE, DISABLE_MODE2_WAIT_REQ);
		clear_ex_multi_job();
		_main_display_disable();
		_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
		// Temp Adjustment Timer Start
		_main_temp_adjust_enable();
#if defined(UBA_RTQ)	
		ex_rc_collect_sw = 0;//通常のクリア処理
		_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_IDLE, 0, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);//2025-01-18
#endif // UBA_RTQ
	}
}

/*********************************************************************//**
 * @brief set enable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_set_enable(void)
{
	if (!(is_box_set()))
	{
		_main_alarm_sub(0, 0, TMSG_CONN_ENABLE, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
	}
	else
	{
	//#if (_DEBUG_CIS_AS_A_POSITION==1) //2024-02-15 待機状態に移行するので、CIS監視停止
		_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
		//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
		dly_tsk(5);
	//#endif
		/* Set FPGA mode : standby */
		_main_set_pl_active(PL_DISABLE);
		/* Set sensor mode : standby */
		_main_set_sensor_active(0);
		_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_BILL_IN);
		clear_ex_multi_job();

		_main_display_enable();
		_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
		// Temp Adjustment Timer Start
		_main_temp_adjust_enable();
#if defined(UBA_RTQ)	
		ex_rc_collect_sw = 0;	
		_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_IDLE, 0, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0); //2025-01-18
#endif // UBA_RTQ
	}
}


/*********************************************************************//**
 * @brief set accept
 * @param[in]	type : initialize type
 * @return 		None
 **********************************************************************/
void _main_set_accept(void)
{
	if (!(is_box_set()))
	{
		_main_alarm_sub(0, 0, TMSG_CONN_ACCEPT, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
	}
	else
	{
		/* Set FPGA mode : active */
		_main_set_pl_active(PL_ENABLE);

		_pl_cis_enable_set(1);

		/* FEED LED ON */
		_pl_sen_feed_encoder_LED_set(1);
		_main_set_mode(MODE1_ACCEPT, ACCEPT_MODE2_SENSOR_ACTIVE);
		/* Set sensor mode : active */
		_main_set_sensor_active(1);
		ex_multi_job.busy |= TASK_ST_CIS_INIT;
		_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_BILL_IN, 0, 0, 0);
		// Temp Adjustment Timer Stop
		_main_temp_adjust_disable();

		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
#if defined(UBA_RTQ)
		/* 状態通知コマンド(VEND VALID前) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_DEPOSIT_BEFORE_VEND, 0, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0); //2025-01-18
#endif // UBA_RTQ
#ifdef _ENABLE_JDL
		jdl_insert();
#endif /* _ENABLE_JDL */
	}
}

/*********************************************************************//**
 * @brief set reject
 * @param[in]	type : initialize type
 * @return 		None
 **********************************************************************/
void _main_set_reject(void)
{
	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);
	/* FEED LED ON */
	_pl_sen_feed_encoder_LED_set(1);
	_main_set_mode(MODE1_REJECT, REJECT_MODE2_SENSOR_ACTIVE);
	clear_ex_multi_job();
	/* Set sensor mode : active */
	_main_set_sensor_active(1);
	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();

	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
#if defined(UBA_RTQ)	
	_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0); //2025-01-18
#endif // UBA_RTQ

}

#if defined(UBA_RTQ)
void set_led_payout(void)	//2025-02-14
{
		/* Set FPGA mode : active */
		_main_set_pl_active(PL_ENABLE);

	//--only payout start
		_pl_cis_enable_set(1);
	
		//#if (_DEBUG_CIS_AS_A_POSITION == 1)
		/* Set for check cheat payout */
		dly_tsk(10); // 2024-05-28
		change_ad_sampling_mode(AD_MODE_VALIDATION_CHECK);
		_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_CIS_ACTIVE_REQ, 0, 0, 0, 0); // これを呼ばないと機能してない
		//#endif
		//--only payout end

		/* FEED LED ON */
		_pl_sen_feed_encoder_LED_set(1);
		/* Set sensor mode : active */
		_main_set_sensor_active(1);
		// Temp Adjustment Timer Stop
		_main_temp_adjust_disable();


	//	_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_SENSOR_ACTIVE);
	//	_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_DESPENSE_BEFORE_VEND, 0, 0, 0);
	//	clear_ex_multi_job();
	//	ex_rc_retry_count = 0;
	//	ex_main_payout_flag = 0;

}


#if defined(UBA_RTQ)
u32 get_TMSG_CONNECTION(void); //2024-11-13
#endif
//void _main_set_payout_or_collect(void) //2024-11-13
void _main_set_payout_or_collect(u8 type) //2024-11-13
{

	u32 data;
	if (!(is_box_set()))
	{
		data = get_TMSG_CONNECTION();
		_main_alarm_sub(0, 0, data, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
	}
	//2024-11-13
	else if ((OperationDenomi.unit == RC_TWIN_DRUM1 && is_rc_twin_d1_empty()) || 
		(OperationDenomi.unit == RC_TWIN_DRUM2 && is_rc_twin_d2_empty()) || 
		(OperationDenomi.unit == RC_QUAD_DRUM1 && is_rc_quad_d1_empty()) || 
		(OperationDenomi.unit == RC_QUAD_DRUM2 && is_rc_quad_d2_empty()))
	{
		data = get_TMSG_CONNECTION();
		_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, data, ALARM_CODE_RC_EMPTY, _main_conv_seq(), ex_position_sensor);

	}
	else if( (ex_main_collect_flag == 1 && type == 0) || type == 2)
	{
	//collect
		/* Set FPGA mode : active */
		_main_set_pl_active(PL_ENABLE);

		/* FEED LED ON */
		_pl_sen_feed_encoder_LED_set(1);
		/* Set sensor mode : active */
		_main_set_sensor_active(1);
		// Temp Adjustment Timer Stop
		_main_temp_adjust_disable();


		_main_set_mode(MODE1_COLLECT, COLLECT_MODE2_SENSOR_ACTIVE);
		_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_COLLECTION, 0, 0, 0);

		ex_main_collect_flag = 0;
	}
	else if( (ex_main_payout_flag == 1 && type == 0) || type == 1)
	{
	//pay out
		/* Set FPGA mode : active */
		_main_set_pl_active(PL_ENABLE);

	//--only payout start
		_pl_cis_enable_set(1);
	
		//#if (_DEBUG_CIS_AS_A_POSITION == 1)
		/* Set for check cheat payout */
		dly_tsk(10); // 2024-05-28
		change_ad_sampling_mode(AD_MODE_VALIDATION_CHECK);
		_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_CIS_ACTIVE_REQ, 0, 0, 0, 0); // これを呼ばないと機能してない
		//#endif
		//--only payout end

		/* FEED LED ON */
		_pl_sen_feed_encoder_LED_set(1);
		/* Set sensor mode : active */
		_main_set_sensor_active(1);
		// Temp Adjustment Timer Stop
		_main_temp_adjust_disable();


		_main_set_mode(MODE1_PAYOUT, PAYOUT_MODE2_SENSOR_ACTIVE);
		_main_send_msg(ID_RC_MBX, TMSG_RC_STATE_REQ, RC_STATE_DESPENSE_BEFORE_VEND, 0, 0, 0);
		clear_ex_multi_job();
		ex_rc_retry_count = 0;
		ex_main_payout_flag = 0;
		ex_rs_payout_disp_on = 0;
	}
}

#endif // UBA_RTQ

void _main_set_reject_note_removed_wait_sensor_active(void)
{
	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);
	/* FEED LED ON */
	_pl_sen_feed_encoder_LED_set(1);
	_main_set_mode(MODE1_REJECT, REJECT_MODE2_NOTE_REMOVED_WAIT_SENSOR_ACTIVE);
	/* Set sensor mode : active */
	_main_set_sensor_active(1);
	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
}

/*********************************************************************//**
 * @brief set reject note stay
 * @param[in]	type : initialize type
 * @return 		None
 **********************************************************************/
void _main_set_reject_standby_note_stay(void)
{
	#if 0 //2024-02-15 紙幣保持中は監視しないと、CISのみで紙幣を検知している場合にJAM検知できなくなる可能性がある
	/* Set FPGA mode : standby */
	_main_set_pl_active(PL_DISABLE);
	#else //2024-06-09 a 温度問題があるのでCISはやはりOFFにする必要がある
	_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
	//CISの消灯は、下記 _main_set_pl_active(PL_DISABLE);で行っている
	dly_tsk(5);
	_main_set_pl_active(PL_DISABLE);
	#endif


	/* Set sensor mode : standby */
	_main_set_sensor_active(0);

	#if defined(_PROTOCOL_ENABLE_ID0G8)
	_main_send_connection_task(TMSG_CONN_REJECT, TMSG_SUB_INTERIM, 0, 0, 0);// イニシャルポーズ用
	#endif

	_main_set_mode(MODE1_REJECT_STANDBY, REJECT_STANDBY_MODE2_NOTE_STAY);
	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
}

u8 is_uv_led_check_uba(void)
{
//実際のUVの設定をしないと、ハード的に接続できているか判断できない
//暫定で使用するにはリスクがあるので廃止
//#if DEBUG_POINT_UV_CHECK
//#if POINT_UV1_ENABLE
	if(FPGA_REG.UV_CHK.BIT.CHK0 != 1)
	{
		/* Error */
		return true;
	}
//#endif
//#if POINT_UV2_ENABLE
	if(FPGA_REG.UV_CHK.BIT.CHK1 != 1)
	{
		/* Error */
		return true;
	}
//#endif
//#endif
	return false;
}

void _main_set_active_enable_uba(u8 mode)
{
	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);
	/* FEED LED ON */
	_pl_sen_feed_encoder_LED_set(1);
	/* Set sensor mode : active */
	_main_set_sensor_active(1);

	switch (mode)
	{
		/* centering */
		case ACTIVE_ENABLE_MODE2_CENTERING_HOME:
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_CENTERING_HOME);
			ex_multi_job.busy |= TASK_ST_CENTERING;
			_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
			break;
		case ACTIVE_ENABLE_MODE2_SHUTTER_OPEN:
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_SHUTTER_OPEN);
			ex_multi_job.busy |= TASK_ST_SHUTTER;
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			break;

		case ACTIVE_ENABLE_MODE2_STACKER_HALF:	/* 2022-02-16 */
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_STACKER_HALF);
			ex_multi_job.busy |= TASK_ST_STACKER;
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HALF_REQ, 0, 0, 0, 0);
			break;

		case ACTIVE_ENABLE_MODE2_PB_CLOSE:
			_main_set_mode(MODE1_ACTIVE_ENABLE, ACTIVE_ENABLE_MODE2_PB_CLOSE);
			ex_multi_job.busy |= TASK_ST_APB;
			_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
			break;


		default:
			/* system error */
			_main_system_error(1, 105);
			break;
	}

	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
}

void _main_set_active_disable_uba(u8 mode)
{
	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);
	/* FEED LED ON */
	_pl_sen_feed_encoder_LED_set(1);
	/* Set sensor mode : active */
	_main_set_sensor_active(1);

	switch (mode)
	{
		/* centering */
		case ACTIVE_DISABLE_MODE2_CENTERING_HOME:
			_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_CENTERING_HOME);
			ex_multi_job.busy |= TASK_ST_CENTERING;
			_main_send_msg(ID_CENTERING_MBX, TMSG_CENTERING_HOME_REQ, 0, 0, 0, 0);
			break;
		case ACTIVE_DISABLE_MODE2_SHUTTER_OPEN:
			_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_SHUTTER_OPEN);
			ex_multi_job.busy |= TASK_ST_SHUTTER;
			_main_send_msg(ID_SHUTTER_MBX, TMSG_SHUTTER_OPEN_REQ, 0, 0, 0, 0);
			break;

		case ACTIVE_DISABLE_MODE2_FEED_REJECT:	//2024-03-18a
			_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_FEED_REJECT);
			ex_multi_job.busy |= TASK_ST_FEED;
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_FORCE_REV_REQ, MOTOR_REV, 0, 0, 0);
			break;

		case ACTIVE_DISABLE_MODE2_STACKER_HALF:	//2024-06-04
			_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_STACKER_HALF);
			_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HALF_REQ, 0, 0, 0, 0);
			ex_multi_job.busy |= TASK_ST_STACKER;
			break;

		case ACTIVE_DISABLE_MODE2_PB_CLOSE:
			_main_set_mode(MODE1_ACTIVE_DISABLE, ACTIVE_DISABLE_MODE2_PB_CLOSE);
			ex_multi_job.busy |= TASK_ST_APB;
			_main_send_msg(ID_APB_MBX, TMSG_APB_CLOSE_REQ, 1, 0, 0, 0);
			break;

		default:
			/* system error */
			_main_system_error(1, 105);
			break;
	}

	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
}

#if !defined(UBA_RTQ)
bool is_uba_mode(void)
{
	#if 1	//2023-04-26
//		return false;	/* 連続取り込みモード */
	if(!(ex_dipsw1 & 0x40)) //DIP-SW 7 OFF
	{
	/* 自動判別モード*/
		if(ex_is_uba_mode == 1)
		{
			return true;	/* 単体取り込みモード Low mode*/
		}
		else
		{
			return false;	/* 連続取り込みモード High mode */		
		}	
	}
	else
	{
	/* 強制Low mode*/
		return true;	/* 単体取り込みモード Low mode*/
	}


	#else

	/* UBA500とは設定は逆*/
    /* 連続取り込みかの判断	*/
	if(!(ex_dipsw1 & 0x40)) //DIP-SW 7 OFF,
//	if(ex_dipsw1 & 0x20); //DIP-SW 6 ON,
	{
	/* Parallel Mode 7 OFF*/
		return false;
	}
	else
	{
	/* UBA_Mode 7 ON*/
		return true;
	}
	#endif
}

#endif

/*********************************************************************//**
 * @brief set adjustment
 * @param[in]	type : initialize type
 * @return 		None
 **********************************************************************/
void _main_set_adjustment(void)
{

	//#if (_DEBUG_CIS_AS_A_POSITION==1) //2024-02-15 最終的にはいれるが、これは最終的は保護処理ハンギング紙幣取り除きなどにまいれる
	_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
	dly_tsk(5);
	//#endif

	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);

	_pl_cis_enable_set(1);

	if (ex_main_task_mode1 == MODE1_DISABLE)
	{
		_main_set_mode(MODE1_ADJUST, ADJUST_MODE2_DISABLE_TEMP_ADJ);
	}
	else if (ex_main_task_mode1 == MODE1_ENABLE)
	{
		_main_set_mode(MODE1_ADJUST, ADJUST_MODE2_ENABLE_TEMP_ADJ);
	}
	else
	{
		/* 暫定エラー 1 */
		_main_system_error(0, 100);
	}
	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
	_main_display_adj();
	/* Set sensor mode : adjust */
	_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_TEMP_ADJ_REQ, 0, 0, 0, 0);
}
/*********************************************************************//**
 * @brief set sensor active
 * @param[in]	1 : active
 * 				0 : standby
 * @return 		None
 **********************************************************************/
void _main_set_sensor_active(u8 active)
{
	/* Active要求の場合 */
	if (active)
	{
		//change_ad_sampling_mode(AD_MODE_WATCH);
		/* SensorをActiveに設定 */
		_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_ACTIVE_REQ, 0, 0, 0, 0);
	}
	else
	{
	//	change_ad_sampling_mode(AD_MODE_ON_OFF);
		/* SensorをStandbyに設定 */
		_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STANDBY_REQ, 0, 0, 0, 0);
	}
}


/*********************************************************************//**
 * @brief set FPGA active
 * @param[in]	1 : active
 * 				0 : standby
 * @return 		None
 **********************************************************************/
void _main_set_pl_active(u8 active)
{
	osw_printf( "_main_set_pl_active (%d) start\n", active );
	/* Active要求の場合 */
	if (active == PL_ENABLE)
	{
		if(get_pl_state() == 0)
		{
		/* FPGAをActiveに設定 */
			if(! enable_pl(1) )
			{
				/* 暫定エラー 1 */
				_main_system_error(1, 101);
				return;
			}
			/* Check FPGA User Mode */
			if(!check_pl_state())
			{
				/* 暫定エラー 1 */
				_main_system_error(1, 101);
				return;
			}
			/* PL Reset */
			/* PL Gpio 初期化 */
			_pl_gpio_init();
			/* CIS PowerUP */
			//_pl_cis_enable_set(1);//ONにしたあとに時間をおいてCIS初期化をおこなうこと
			/* Feed Motor 初期化 */
			_pl_motor_init();
			/* FEED LED ON */
			_pl_sen_feed_encoder_LED_set(1);
			/* PL ADC PowerUP */
			/* PL ADC 初期化 */
		}
	}
	else
	{
		if(get_pl_state())
    	{
			_pl_cis_enable_set(0);

			/* FEED LED OFF */
			_pl_sen_feed_encoder_LED_set(0);
			/* PL ADC PowerDown */
			/* Feed Motor 初期化 */
			_pl_motor_final();
			/* FPGAをStandbyに設定 */
			enable_pl(0);
    	}
	}
	osw_printf( "_main_set_pl_active (%d) end\n\n", active );
}

/*********************************************************************//**
 * @brief set standby
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_set_test_standby(void)
{
	ex_abnormal_code = 0;
	ex_main_test_no = TEST_STANDBY;
	set_test_ld_mode(0);
	set_test_allacc_mode(0);
	set_test_allrej_mode(0);

	clear_ex_multi_job();
	/* Set FPGA mode : standby */
	_main_set_pl_active(PL_DISABLE);
	/* Set sensor mode : standby */
	_main_set_sensor_active(0);
	/* Test mode standby OK */
	_main_display_test_standby();
	_main_set_mode(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_STANDBY);
	// Temp Adjustment Timer Start
	_main_temp_adjust_enable();
}

/*********************************************************************//**
 * @brief set active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_set_test_active(void)
{
	ex_test_finish = 0;

#if (_DEBUG_FPGA_FRAM==1)
	ex_fram_log_enable = 1;  //テストモード開始後1
#endif

#if defined(UBA_RTQ_AZ_LOG) 
	ex_fram_log_enable = 1;  //テストモード開始後1
#endif

	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);

	_pl_cis_enable_set(1);

	/* FEED LED ON */
	_pl_sen_feed_encoder_LED_set(1);
	_main_set_mode(MODE1_TEST_ACTIVE, TEST_ACTIVE_MODE2_SENSOR_ACTIVE);
	/* Set sensor mode : active */
	_main_set_sensor_active(1);

	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);

	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
}

/*********************************************************************//**
 * @brief set standby
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_set_test_wait(void)
{
	/* Set FPGA mode : standby */
	_main_set_pl_active(PL_DISABLE);
	/* Set sensor mode : standby */
	_main_set_sensor_active(0);
	clear_ex_multi_job();
	/* Test mode standby OK */
	_main_display_test_standby();
	_main_set_mode(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_WAIT);
}


void _main_set_mode_alarm_back(u8 mode1, u8 mode2)	//UBA_RFID
{
	ex_main_task_mode1_alarm_back = mode1;
	ex_main_task_mode2_alarm_back = mode2;
}

void _main_alarm_sub(u32 mode1, u32 mode2, u32 rsp_msg, u32 code, u32 seq, u32 sensor)
{
	u8 only_bill_on_cis = 0;
#if defined(UBA_RTQ)
	u8 ex_main_emergency_flag_bk = 0;
#endif
	//2024-06-09 Acceptor JAMのみ特殊処理
	if( (SENSOR_ALL_OFF_WITHOUT_CIS) && (ex_position_sensor & POSI_VALIDATION) ) //CISのみで紙幣検知でのエラー
	{
		only_bill_on_cis = 1;
	}

	#if defined(UBA_RTQ)
	if(is_rc_rs_unit())
	{
		/* alram reset remain flag */
		ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
	}
	
	if((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE 
	|| (is_rc_rs_unit()) )
	{
		motor_ctrl_feed_stop();
	}
	#endif

	#if defined(_PROTOCOL_ENABLE_ID003)	//#if defined(UBA_ID003_ENC)
	update_random_seed(0);
	#endif

	ex_abnormal_code = code;
	ex_2nd_note_uba = 0; /*エラー時のクリア*/
	ex_force_stack_retry = 0; //2023-11-27

	#ifdef _ENABLE_JDL
    jdl_error(code, seq, mode1, mode2, sensor);
	#endif  /* _ENABLE_JDL */

	//2023-11-01
	_initialize_position_pulse();
	_initialize_feed_motor_pulse();

#if defined(UBA_RTQ)
	ex_rc_retry_flg = FALSE; //2025-02-01
	_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
	ex_rc_collect_sw = 0;
	ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;

	ex_main_payout_flag = 0;
	ex_main_collect_flag = 0;
	ex_rc_exchanged_unit = 0;
	//2025-01-26

	ex_main_emergency_flag_bk = ex_main_emergency_flag;
	ex_main_emergency_flag = 0;
	/* box open中のbox open検出 */
	ex_rc_detect_next_box_open = 0;
	/* Refill mode終了 *//* '20-02-14 */
	OperationDenomi.mode = 0;
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
	ex_rtq_rfid_write_disable = 0;
	memset((u8 *)&Smrtdat_fram_bk_power, 0, sizeof(Smrtdat_fram_bk_power)); //エラーにより リカバリクリア
	#endif
#endif // UBA_RTQ

	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);

//#if (UBA_RFID==1)
#if !defined(UBA_RTQ) //RTQがエラーの時はRFIDを書き込むができないので、リスクがあるのでエラーはカウントしない
	u32 error_code_icb = get_icb_error_code((u8)code);
	if( (error_code_icb != 0) && is_icb_enable() && (ex_main_task_mode1 != MODE1_ACTIVE_ALARM) )
	{
		ex_icb_alarm_backup.mode1 = mode1;
		ex_icb_alarm_backup.mode2 = mode2;
		ex_icb_alarm_backup.rsp_msg = rsp_msg;
		ex_icb_alarm_backup.code = code;
		ex_icb_alarm_backup.seq = seq;
		ex_icb_alarm_backup.sensor = sensor;
		// go to icb error write
		ex_multi_job.busy |= TASK_ST_ICB;
		_main_send_msg(ID_ICB_MBX, TMSG_ICB_ERROR_CODE_REQ, error_code_icb, 0, 0, 0);
		_main_set_mode(MODE1_ACTIVE_ALARM, ACTIVE_ALARM_MODE2_ICB_ERROR_RSP);
		return;
	}
#endif	
//#endif

	//2023-07-22
	memset(&ex_multi_job_alarm_backup, 0, sizeof(ex_multi_job_alarm_backup));

	//2024-06-09 正監視を行うと温度上昇が続いて4h経過で常温環境下でも65度以上になる
	//本来 Acceptor JAMなどはCISでの紙幣検知もエラーの条件になっているので、CISで監視がのぞましいが、
	//(CISのみで紙幣を検知している場合にJAM解除できない)
	//温度の問題があるので、エラー確定後は、CIS OFF
	//CISのみで紙幣検知の場合、Failure AFにする
	_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
	//CISの消灯は、下記_main_set_pl_active(PL_DISABLE);で行っている
	dly_tsk(5);
	_main_set_pl_active(PL_DISABLE);

	/* Set sensor mode : standby */
	_main_set_sensor_active(0);

	if ((mode1 == 0) && (mode2 == 0))
	{
		switch (code)
		{
		case ALARM_CODE_FRAM:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_FRAM);
			break;
		case ALARM_CODE_MAG:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_MAG);
			break;
		case ALARM_CODE_I2C:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_I2C);
			break;
		case ALARM_CODE_TMP_I2C:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_TMP_I2C);
			break;
		case ALARM_CODE_SPI:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_SPI);
			break;
		case ALARM_CODE_PL_SPI:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_PL_SPI);
			break;
		case ALARM_CODE_CISA_OFF:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CISA_OFF);
			break;
		case ALARM_CODE_CISB_OFF:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CISB_OFF);
			break;
		case ALARM_CODE_UV:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_UV);
			break;
		case ALARM_CODE_CIS_ENCODER:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CIS_ENCODER);
			break;

		case ALARM_CODE_STACKER_FULL:			// STACKER FULL
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_STACKER_FULL);
			break;
		case ALARM_CODE_STACKER_MOTOR_LOCK:		// Failure A2 押しメカエラー
		case ALARM_CODE_STACKER_GEAR:			// Failure A2 押しメカエラー
		case ALARM_CODE_STACKER_TIMEOUT:		// Failure A2 押しメカエラー
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_STACKER_FAIL);
			break;
		case ALARM_CODE_STACKER_HOME:			// Failure A2 押しメカエラー
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_PUSHER_HOME);
			break;

		case ALARM_CODE_FEED_OTHER_SENSOR_SK:	// JAM IN STACKER
		case ALARM_CODE_FEED_SLIP_SK:			// JAM IN STACKER
		case ALARM_CODE_FEED_TIMEOUT_SK:		// JAM IN STACKER
		case ALARM_CODE_FEED_MOTOR_LOCK_SK:		//2024-02-13
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_STACKER_JAM);
			break;
		case ALARM_CODE_FEED_LOST_BILL:		// JAM IN STACKER
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_LOST_BILL);
			break;

		case ALARM_CODE_FEED_OTHER_SENSOR_AT:	// JAM IN ACCEPTOR
		case ALARM_CODE_FEED_SLIP_AT:			// JAM IN ACCEPTOR
		case ALARM_CODE_FEED_TIMEOUT_AT:		// JAM IN ACCEPTOR
		case ALARM_CODE_FEED_MOTOR_LOCK_AT:		// JAM IN ACCEPTOR
			//2024-06-09 エラー中などCISセンサを消灯する必要があるので、CISでの紙幣検知情報をクリアする必要がある
			//CISのみで札を検知している場合は、Acceptor JAMと自動復帰を繰り返す事になるので、
			//Acceptor JAM以外の自動復帰しないエラーにする必要がある
			if(only_bill_on_cis == 1)
			{
			//自動復帰無効
				only_bill_on_cis = 2;
				_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_ACCEPTOR_JAM_CIS);
			}
			else
			{
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_ACCEPTOR_JAM);
			}
			break;

		case ALARM_CODE_FEED_MOTOR_SPEED_LOW:	// Failure A5 搬送スピードエラー
		case ALARM_CODE_FEED_MOTOR_SPEED_HIGH:	// Failure A5 搬送スピードエラー
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_FEED_SPEED);
			break;
		case ALARM_CODE_FEED_MOTOR_LOCK:		// Failure A6 搬送エラー
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_FEED_FAIL);
			break;
		case ALARM_CODE_APB_TIMEOUT:			// Failure A9 PBエラー,  Failure AF
		case ALARM_CODE_APB_HOME:				// Failure A9 PBエラー,  Failure AF
		case ALARM_CODE_APB_HOME_STOP:			// Failure A9 PBエラー,  Failure AF
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_APB_FAIL);
			break;

		case ALARM_CODE_BOX:				// BOX OPEN

		#if !defined(UBA_RTQ)//UBA_RFID
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_BOX);
		#else
			if(ex_rc_status.sst1A.bit.busy == 1 || ex_rc_status.sst1A.bit.initial == 1)
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_CANCEL_REQ, 0, 0, 0, 0);
			}

			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_BOX);
		#endif
			break;
		case ALARM_CODE_BOX_INIT:				// BOX OPEN
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CONFIRM_BOX);
			break;
		case ALARM_CODE_CHEAT:				// CHEAT
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CHEAT);
			break;

		case ALARM_CODE_CENTERING_TIMEOUT:		// Failure C1 幅寄せエラー,  Failure AF
		case ALARM_CODE_CENTERING_HOME_STOP:	// Failure C1 幅寄せエラー,  Failure AF
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CENTERING_FAIL);
			break;

		case ALARM_CODE_SHUTTER_TIMEOUT:
		case ALARM_CODE_SHUTTER_HOME_STOP:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_SHUTTER_FAIL);
			break;

		case ALARM_CODE_RFID_UNIT_MAIN:					/* ICB有効でRFIDユニットがない場合 */
		case ALARM_CODE_RFID_ICB_SETTING: 			/* ICB無効でICBユニットがある場合 */
		case ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN: 		/* ICB読み込み、書込み失敗 */
		case ALARM_CODE_RFID_ICB_DATA: 				/* ICBチェックサムデータ異常 */
		case ALARM_CODE_RFID_ICB_NUMBER_MISMATCH:	/* ICB別のゲーム機にセットされていたBOXです */
		case ALARM_CODE_RFID_ICB_NOT_INITIALIZE: 	/* ICB集計データセーブ済みのBOXです */
		case ALARM_CODE_ICB_FORCED_QUIT:		//0x6E/* ICBタスクのシーケンス異常						*/
		case ALARM_CODE_RFID_ICB_MC_INVALID:		//マシン番号未設定
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
		case ALARM_CODE_RC_RFID:			
	#endif
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_RFID);
			break;

		case ALARM_CODE_FEED_FORCED_QUIT:
		case ALARM_CODE_STACKER_FORCED_QUIT:
		case ALARM_CODE_APB_FORCED_QUIT:
		case ALARM_CODE_CENTERING_FORCED_QUIT:
		case ALARM_CODE_SHUTTER_FORCED_QUIT:
			/* 暫定エラー 2 (Software Reset) */
			_main_system_error(1, 104);
			break;


#if defined(UBA_RTQ)		/* '18-05-01 */
		case ALARM_CODE_RC_ERROR:
		case ALARM_CODE_RC_ROM: //UBA500もコードはあるが使用していない可能性あり
		case ALARM_CODE_RC_REMOVED:
		case ALARM_CODE_RC_COM:
		case ALARM_CODE_RC_DWERR:
		case ALARM_CODE_RC_POS:
		case ALARM_CODE_RC_TRANSPORT:
		case ALARM_CODE_RC_TIMEOUT:
		case ALARM_CODE_RC_DENOMINATION:
		case ALARM_CODE_RC_EMPTY:
		case ALARM_CODE_RC_DOUBLE:
		case ALARM_CODE_RC_FULL:
		case ALARM_CODE_RC_EXCHAGED:
			/* 暫定エラー 2  */
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_RC_ERROR);
			break;
		case ALARM_CODE_RC_FORCED_QUIT:			        	/* RC forced quit */
			/* 暫定エラー 2 (Software Reset) */
			_main_system_error(1, 106);
			break;
#endif

	//2024-05-28
		case ALARM_CODE_CIS_TEMPERATURE:
			_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CIS_TEMPERATURE);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_CONFIRM_TEMP, WAIT_TIME_DATA_WAIT, 0, 0);
			break;

		case ALARM_CODE_EXTERNAL_RESET:
		case ALARM_CODE_PLL_LOCK:
		case ALARM_CODE_POWER_OFF:
		default:
			/* system error */
			_main_system_error(1, 105);
			break;
		}
	#if defined(UBA_RTQ) //2025-01-26
		set_rtq_jam(code, ex_main_emergency_flag_bk);
	#endif //end RTQ 2025-01-26
	}
	else
	{
	//((mode1 != 0) || (mode2 != 0)
	#if defined(UBA_RTQ) //2025-01-26
		if(ex_rc_status.sst1A.bit.error == 0 && is_detect_rc_twin() && is_detect_rc_quad())
		{		
			set_rtq_jam(code, ex_main_emergency_flag_bk);
			_main_set_mode_alarm_back(mode1, mode2);
		}
		else
		{
			if(!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				/* 	払出し中, 回収中のRCユニット取り外しはエラー復帰時に紙幣チェックする */
				if(rsp_msg == TMSG_CONN_PAYOUT || rsp_msg == TMSG_CONN_COLLECT || rsp_msg == TMSG_CONN_STACK || rsp_msg == TMSG_CONN_RESET)
				{
					ex_cline_status_tbl.ex_rc_after_jam = 1;
				}

				_main_send_msg(ID_RC_MBX, TMSG_RC_ERROR_CLEAR_REQ, 0, 0, 0, 0);
				code = ALARM_CODE_RC_REMOVED;
				_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_CONFIRM_RC_UNIT);
			}
			else
			{
				ex_pre_feed_after_jam = 1;
				code = ALARM_CODE_RC_ERROR;
				_main_set_mode_alarm_back(MODE1_ALARM, ALARM_MODE2_RC_ERROR);
			}
		}
	#else		
		_main_set_mode_alarm_back(mode1, mode2);
	#endif	
	}
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, code, 0, 0, 0);
	// 2020-04-24:エラー時のイニシャル復帰の為、 
	// RBA-40ではTMSG_SENSOR_RESET_DA_REQを送っていたがRBA-40CではTMSG_SENSOR_STATUS_REQを送る
	_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
	//2024-06-09
	if(only_bill_on_cis == 2)
	{
	//ホストに対する情報のみAcceptor JAMではなく、Failure AFにする
		_main_send_connection_task(rsp_msg, TMSG_SUB_ALARM, ALARM_CODE_FEED_CIS_AT, 0, 0);
	}
	else
	{
	#if defined(UBA_RTQ)
		//通常時のみ使用と明確にしたいので、UBA500より条件を追加
		if( ( (RECOVERY_STEP_PAYOUT_ESCROW == ex_recovery_info.step) ||
			(RECOVERY_STEP_PAYOUT_VALID  == ex_recovery_info.step) )
		&&	
		( ex_main_task_mode1 == MODE1_PAYOUT)
		)
		{
		//1 通常動作時のみ使用に統一、パワーリカバリ時は使用しない その為	MODE1_PAYOUT を条件に追加
		//2 UBA500と異なりメッセージの引数の順番などを変えた
		//理由、UBA500のようにTMSG_SUB_ALARMとTMSG_SUB_PAYVALID_ERRORを明確に分ける方が明確だが、
		//ラインタスクで常にTMSG_SUB_PAYVALID_ERROR をケアしないと、ステータスが遷移しなくなる
		//TMSG_SUB_ALARMにしておけば、TMSG_SUB_ALARMをケアすればPay Validだせなくなっても最低エラーには遷移できる
		//UBA500 TMSG_SUB_PAYVALID_ERROR, code, 0, 0
		//UBA700 TMSG_SUB_ALARM, TMSG_SUB_PAYVALID_ERROR, code, 0,

		//UBA500リカバリ時に入って来るが意味がない
		//通常のpayout動作では入ってこない、たまたま、payout動作でエラー監視していないだけで、将来入ってくる可能性大
		//条件だけを見ると、リカバリ用に見えるが実際UBA500RTQはこれはリカバリでは使用していない。
		//ラインタスクがリカバリ時は、パワーアップのイニシャル関係になっていて、この	TMSG_CONN_PAYOUT を受けるケース文に
		//パワーアップのイニシャル関係のcase文が存在しない。
		//UBA500RTQが
		//リカバリのエラーでPayValidを出せるいる理由は
		//mode_alarmの alarm_box();  で　MSG_SENSOR_LOW_CLOCK_RSP　を受信して
		// if ((ex_main_msg.tmsg_code & TMSG_TCODE_LINE) == TMSG_TCODE_LINE) となって
		//  _id003_status_info_mode_def(void) で ID003_MODE_PAYVALID_ERROR
			_main_send_connection_task(rsp_msg, TMSG_SUB_ALARM, code, TMSG_SUB_PAYVALID_ERROR, 0);
		}
		else
		{
		//通常のpayout動作で入ってくる。ただしpayout完了後のDisableで入ってくる
			_main_send_connection_task(rsp_msg, TMSG_SUB_ALARM, code, 0, 0);
		}
	#else
		_main_send_connection_task(rsp_msg, TMSG_SUB_ALARM, code, 0, 0);
	#endif
	}

#if defined(UBA_RTQ)
	//UBA500はこっちだが、別の方がいいと思う _main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 100, 0, 0);
	_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 100, 0, 0); //時間だけは同じにする //2025-01-18
#endif
	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();

	//UBAは最初1箇所に集める
	_main_set_mode( MODE1_ALARM, ALARM_MODE2_WAIT_SENSOR_ACTIVE );	/* 2018-11-13	*///UBAは最初1箇所に集める
}




/*********************************************************************//**
 * @brief reject sub function
 * @param[in]	mode1   : set mode1
 * 				mode2   : set mode2
 * 				rsp_msg : respons TMSG
 * 				code    : alarm code
 * 				seq     : sequence no
 * 				sensor  : status of sensor
 * @return 		None
 **********************************************************************/
void _main_reject_sub(u32 mode1, u32 mode2, u32 rsp_msg, u32 code, u32 seq, u32 sensor)
{


	ex_validation.reject_code = code;
#ifdef _ENABLE_JDL
	if(ex_validation.reject_code == REJECT_CODE_PRECOMP)
	{
	    jdl_reject(ex_validation.reject_code, ex_validation.start, JDL_ACC_DINFO_UNKNOWN_IDX, seq, mode1, mode2, sensor);
	}
	else
	{
	    jdl_reject(ex_validation.reject_code, ex_validation.start, ex_validation.denomi, seq, mode1, mode2, sensor);
	}
#endif	/* _ENABLE_JDL */

	/* Set FPGA mode : active */
	_main_set_pl_active(PL_ENABLE);

//2024-02-16 監視追加
#if 1//#if (_DEBUG_CIS_AS_A_POSITION==1)	//ここから監視処理開始
//ここで消さなくても、待機で消えている事を確認済み
	_pl_cis_enable_set(1); //2024-02-26
	dly_tsk(10);
	change_ad_sampling_mode(AD_MODE_VALIDATION_CHECK);
	_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_CIS_ACTIVE_REQ, 0, 0, 0, 0); //これを呼ばないと機能してない
#else
	_pl_cis_enable_set(0);
#endif
	/* FEED LED ON */
	_pl_sen_feed_encoder_LED_set(1);
	_main_set_mode(mode1, mode2);

	 //2023-07-22
	//返却後アラーム用 clear_ex_mult_jobでクリアされるので、バックアップ
	memcpy(&ex_multi_job_alarm_backup, &ex_multi_job, sizeof(MULTI_JOB));
	ex_2nd_note_uba = 0; /* Reject時のクリア*/


	clear_ex_multi_job();
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, code, 0, 0, 0);
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
	_main_send_connection_task(rsp_msg, TMSG_SUB_REJECT, code, 0, 0);
	// Temp Adjustment Timer Stop
	_main_temp_adjust_disable();
}


/*********************************************************************//**
 * @brief reject sub function
 * @param[in]	mode1   : set mode1
 * 				mode2   : set mode2
 * 				code    : alarm code
 * 				seq     : sequence no
 * 				sensor  : status of sensor
 * @return 		None
 **********************************************************************/
void _main_reject_req(u32 mode1, u32 mode2, u32 code, u32 seq, u32 sensor)
{
	_main_set_mode(MODE1_REJECT, REJECT_MODE2_SENSOR_ACTIVE);
	_main_set_sensor_active(1);
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_REJECT, code, 0, 0, 0);
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
}

/*********************************************************************//**
 * @brief set display initialize
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_init(void) //イニシャルの1箇所
{
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
}


/*********************************************************************//**
 * @brief set display adjustment
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_adj(void)
{
//2023-07-18	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0); 
}


/*********************************************************************//**
 * @brief set display disable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_disable(void) //mode_disableの1箇所
{
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
}


/*********************************************************************//**
 * @brief set display enable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_enable(void) //mode_enable遷移の1箇所
{
	if(is_test_mode() && (ex_validation.start == VALIDATION_STRT))
	{
	// keep 

	}
	else
	{
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
	}
#if defined(UBA_RTQ)
	if (OperationDenomi.mode)
	{
		/* LED BEZEL BLINK */
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_BLINK, 0, 3, 3, 3);
	}
	else
	{
		/* LED BEZEL ON */
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0, 0, 0, 0);
	}
#else
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0, 0, 0, 0);
#endif

}

/*********************************************************************//**
 * @brief set display when in powerup 
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_powerup()
{
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_OFF, 0, 0, 0, 0);
}

/*********************************************************************//**
 * @brief set display pause
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_test_standby(void)
{
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_STANDBY, 0, 0, 0);
	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
}


/*********************************************************************//**
 * @brief set display denomination
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_display_denomination(void)
{
	extern const u8 accept_denomi[];
	
	/* LED display for denomi code(Test mode only) */
	if (is_test_mode())
	{
		if((ex_main_test_no == TEST_ACCEPT)
		 ||(ex_main_test_no == TEST_ACCEPT_LD)
		)
		{
			if (ex_validation.denomi == BAR_INDX)
			{
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DENOMI, 0x10, 0, 0, 0);
			}
			else
			{
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DENOMI, accept_denomi[ex_validation.denomi], 0, 0, 0);
			}
		}
	}
}




/*********************************************************************//**
 * @brief set display pause
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u16 _main_conv_seq(void)
{
	u16 rtn = (0xF000|(ex_main_task_mode1 << 8)|(ex_main_task_mode2));
	return rtn;
}


/*********************************************************************//**
 * @brief Get agint wait time
 * @param[in]	None
 * @return 		aging_time : aging time
 **********************************************************************/
u16 _main_get_aging_time(void)
{
	u16 aging_time;
	u8 dipsw = 0;

	//製品規格の場合、インターバルを固定にした特殊ソフトを使用した
	//製品規格の安全評価ソフトは、CISやモータの為、インターバル30s
	//製品規格のEMI.EMS評価ソフトは、インターバル5s
	dipsw = ex_dipsw1;
	
	 dipsw &= 0x60;

	if(dipsw == 0)
	{
		aging_time = 3000;
	}
	else if(dipsw == 0x20)	// DIP-SW 6 ON
	{
		//大阪EMIチェック用5s
		aging_time = 1500;
		//aging_time = 500;
	}
	else if(dipsw == 0x40)	// DIP-SW 7 ON
	{
		aging_time = 200;
	}
	else
	{
		aging_time = 1000;
	}

	return aging_time;
}

#if defined(UBA_RTQ) //#if defined(UBA_RS)
/*********************************************************************//**
 * @brief check bill remain for normal case of RC-TQ
 * @param[in]	None
 * @return 		Bill Remain No
 **********************************************************************/
u16	_main_judge_bill_remain_normal_model(void) //mode_power, mode_initialize //ok //識別センサはUBA500と異なり条件がある為、若干異なる ok
{
	u16 rtn;

	// センサー1,2,3,A,B,C
	if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
	{
		if(!(SENSOR_VALIDATION_OFF))
		{
			rtn = BILL_IN_ACCEPTOR;
			return rtn;
		}
	}
	if((SENSOR_CENTERING))
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if ((SENSOR_APB_IN) || (SENSOR_APB_OUT) )
	{
	    /* 出金時は識別センサがAcceptorとStackerの境目 */
	    /* 入金時は出口センサ */
	    /* スイッチバック時は入金方向へ駆動しているが出金紙幣と同じ扱い */
        if( //2025-05-24 (ex_recovery_info.step != RECOVERY_STEP_PAYOUT_ESCROW)       &&
            (ex_recovery_info.step != RECOVERY_STEP_PAYOUT_POS1  )       &&
            (ex_recovery_info.step != RECOVERY_STEP_SWITCHBACK_TRANSPORT) )
        {
			rtn = BILL_IN_ACCEPTOR;
		}
		else
		{
			rtn = BILL_IN_STACKER;
		}
	}
	else if((RS_POS2_ON || RS_POS3_ON) && (is_rc_rs_unit()) )
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if((RS_POS1_ON) && (is_rc_rs_unit()) )
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	else if( ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0) )
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	// センサー4.5.6,D,E,F
	else if((((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) || ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0))
	&&		(ex_rc_status.sst1A.bit.quad))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	else if ( SENSOR_EXIT )
	{
		rtn = BILL_IN_STACKER;
	}
	else if ((SENSOR_ENTRANCE))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}

	return(rtn);
}


/*********************************************************************//**
 * @brief check bill remain for rs case of RC-TQ
 * @param[in]	None
 * @return 		Bill Remain No
 **********************************************************************/
u16	_main_judge_bill_remain_rs_model(void) //mode_power, mode_initialize //ok
{
	u16 rtn;

//	if((SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF))
//	{
//		rtn = BILL_IN_ACCEPTOR;
//	}
//	else if(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW || ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
	if(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW || ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
	{
		if((SENSOR_ENTRANCE))
		{
			rtn = BILL_IN_ENTRANCE;
		}
//		else if((SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
//		{
//			rtn = BILL_IN_ACCEPTOR;
//		}
		else if(RS_POS2_ON || RS_POS3_ON)
		{
			rtn = BILL_IN_ACCEPTOR;
		}
		else
		{
			rtn = BILL_IN_NON;
		}
	}
//	else if ((SENSOR_APB_IN) || (SENSOR_APB_OUT) )
//	{
//	    /* 出金時にAPB_IN,, APB_OUTがONする事はあり得ない */
//	    /* スイッチバック時は入金方向へ駆動しているが出金紙幣と同じ扱い */
//		if(ex_recovery_info.step != RECOVERY_STEP_SWITCHBACK_TRANSPORT)
//      {
//			rtn = BILL_IN_ACCEPTOR;
//		}
//		else
//		{
//			rtn = BILL_IN_STACKER;
//		}
//	}
	else if(RS_POS2_ON || RS_POS3_ON)
	{
			rtn = BILL_IN_ACCEPTOR;
	}
	else if(RS_POS1_ON)
	{
		rtn = BILL_IN_STACKER;
	}
	else if(((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	// センサー4.5.6,D,E,F
	else if((((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) || ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0)) && (ex_rc_status.sst1A.bit.quad))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
//	else if (SENSOR_EXIT)
//	{
//		rtn = BILL_IN_STACKER;
//	}
	else if ((SENSOR_ENTRANCE))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}
	return(rtn);
}
#endif // UBA_RS


/*********************************************************************//**
 * @brief check bill remain
 * @param[in]	None
 * @return 		Bill Remain No
 **********************************************************************/
u16 _main_bill_remain(void) //mode_power, mode_initialize //ok RTQは _main_bill_in も同じ
{
	u16 rtn;
#if defined(UBA_RTQ) //#if defined(UBA_RS)
	if(is_rc_rs_unit())
	{
		if((ex_recovery_info.step == RECOVERY_STEP_PAYOUT_DRUM)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_TRANSPORT)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_POS1)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_POS7)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
		|| (ex_recovery_info.step == RECOVERY_STEP_SWITCHBACK_TRANSPORT))
		{
			rtn = _main_judge_bill_remain_rs_model();
		}
		else
		{
			rtn = _main_judge_bill_remain_normal_model();
		}
	}
	else
	{
		rtn = _main_judge_bill_remain_normal_model();
	}
#else
	//SS
	//2024-02-27
	if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
	{
		if(!(SENSOR_VALIDATION_OFF))
		{
			rtn = BILL_IN_ACCEPTOR;
			return rtn;
		}
	}

	if ((SENSOR_CENTERING) /* || !(SENSOR_VALIDATION_OFF) */ || (SENSOR_APB_IN) || (SENSOR_APB_OUT) )
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if ( SENSOR_EXIT )
	{
		rtn = BILL_IN_STACKER;
	}
	else if ((SENSOR_ENTRANCE))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}
#endif // uBA_RS
	return rtn;
}



#if defined(UBA_RTQ) //#if defined(UBA_RS)
u16 _main_judge_bill_in_rs_model(void)
{
	u16 rtn;

//	if((SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF))
//	{
//		rtn = BILL_IN_ACCEPTOR;
//	}
//	else if(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW || ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
	if(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW || ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
	{
		if((SENSOR_ENTRANCE))
		{
			rtn = BILL_IN_ENTRANCE;
		}
//		else if((SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
//		{
//			rtn = BILL_IN_ACCEPTOR;
//		}
		else if(RS_POS2_ON || RS_POS3_ON)
		{
			rtn = BILL_IN_ACCEPTOR;
		}
		else
		{
			rtn = BILL_IN_NON;
		}
	}
//	else if ((SENSOR_APB_IN) || (SENSOR_APB_OUT) )
//	{
//	    /* 出金時にAPB_IN,, APB_OUTがONする事はあり得ない */
//	    /* スイッチバック時は入金方向へ駆動しているが出金紙幣と同じ扱い */
//		if(ex_recovery_info.step != RECOVERY_STEP_SWITCHBACK_TRANSPORT)
//      {
//			rtn = BILL_IN_ACCEPTOR;
//		}
//		else
//		{
//			rtn = BILL_IN_STACKER;
//		}
//	}
	else if(RS_POS2_ON || RS_POS3_ON)
	{
			rtn = BILL_IN_ACCEPTOR;
	}
	else if(RS_POS1_ON)
	{
		rtn = BILL_IN_STACKER;
	}
	else if(((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	// センサー4.5.6,D,E,F
	else if((((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) || ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0)) && (ex_rc_status.sst1A.bit.quad))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
//	else if (SENSOR_EXIT)
//	{
//		rtn = BILL_IN_STACKER;
//	}
	else if ((SENSOR_ENTRANCE))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}
	return(rtn);
}

u16 _main_judge_bill_in_normal_model(void)
{
	u16 rtn;

	// センサー1,2,3,A,B,C
	if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
	{
		if(!(SENSOR_VALIDATION_OFF))
		{
			rtn = BILL_IN_ACCEPTOR;
			return rtn;
		}
	}
	if((SENSOR_CENTERING))
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if ((SENSOR_APB_IN) || (SENSOR_APB_OUT) )
	{
	    /* 出金時は識別センサがAcceptorとStackerの境目 */
	    /* 入金時は出口センサ */
	    /* スイッチバック時は入金方向へ駆動しているが出金紙幣と同じ扱い */
        if( //2025-05-24 (ex_recovery_info.step != RECOVERY_STEP_PAYOUT_ESCROW)       &&
            (ex_recovery_info.step != RECOVERY_STEP_PAYOUT_POS1  )       &&
            (ex_recovery_info.step != RECOVERY_STEP_SWITCHBACK_TRANSPORT) )
        {
			rtn = BILL_IN_ACCEPTOR;
		}
		else
		{
			rtn = BILL_IN_STACKER;
		}
	}
	else if((RS_POS2_ON || RS_POS3_ON) && (is_rc_rs_unit()) )
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if((RS_POS1_ON) && (is_rc_rs_unit()) )
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	else if( ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0) )
	{
#if 1//#ifdef _ENABLE_JDL
		jdl_add_trace(ID_MAIN_TASK, 0xF1, ex_rc_status.sst21A.byte, ex_rc_status.sst22A.byte, ex_rc_status.sst31A.byte, ex_rc_status.sst32A.byte);
#endif /* _ENABLE_JDL */

		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	// センサー4.5.6,D,E,F
	else if((((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) || ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0))
	&&		(ex_rc_status.sst1A.bit.quad))
	{
#if 1//#ifdef _ENABLE_JDL
		jdl_add_trace(ID_MAIN_TASK, 0xF2, ex_rc_status.sst21A.byte, ex_rc_status.sst22A.byte, ex_rc_status.sst31A.byte, ex_rc_status.sst32A.byte);
#endif /* _ENABLE_JDL */

		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	else if ( SENSOR_EXIT )
	{
		rtn = BILL_IN_STACKER;
	}
	else if ((SENSOR_ENTRANCE))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}

	return(rtn);
}


u16 _main_stay_bill_check_rs_model(void) ////mode_reject
{
	u16 rtn;

//	if((SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF))
//	{
//		rtn = BILL_IN_ACCEPTOR;
//	}
//	else if(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW || ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
	if(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW || ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
	{
		if((SENSOR_ENTRANCE))
		{
			rtn = BILL_IN_ENTRANCE;
		}
//		else if((SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
//		{
//			rtn = BILL_IN_ACCEPTOR;
//		}
		else if(RS_POS2_ON || RS_POS3_ON)
		{
			rtn = BILL_IN_ACCEPTOR;
		}
		else
		{
			rtn = BILL_IN_NON;
		}
	}
//	else if ((SENSOR_APB_IN) || (SENSOR_APB_OUT) )
//	{
//	    /* 出金時にAPB_IN,, APB_OUTがONする事はあり得ない */
//	    /* スイッチバック時は入金方向へ駆動しているが出金紙幣と同じ扱い */
//		if(ex_recovery_info.step != RECOVERY_STEP_SWITCHBACK_TRANSPORT)
//        {
//			rtn = BILL_IN_ACCEPTOR;
//		}
//		else
//		{
//			rtn = BILL_IN_STACKER;
//		}
//	}
	else if(RS_POS2_ON || RS_POS3_ON)
	{
			rtn = BILL_IN_ACCEPTOR;
	}
	else if(RS_POS1_ON)
	{
		rtn = BILL_IN_STACKER;
	}
	else if(((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	// センサー4.5.6,D,E,F
	else if((((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) || ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0)) && (ex_rc_status.sst1A.bit.quad))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
//	else if (SENSOR_EXIT)
//	{
//		rtn = BILL_IN_STACKER;
//	}
	else if ((SENSOR_ENTRANCE))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}
	return(rtn);
}

/*********************************************************************//**
 * @brief check bill stay for normal case of RC-TQ
 * @param[in]	None
 * @return 		Bill Remain No
 **********************************************************************/
u16 _main_stay_bill_check_normal_model(void) //mode_reject
{
	u16 rtn;

	// センサー1,2,3,A,B,C
	if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
	{
		if(!(SENSOR_VALIDATION_OFF))
		{
			rtn = BILL_IN_ACCEPTOR;
			return rtn;
		}
	}

	if ((SENSOR_APB_IN) || (SENSOR_APB_OUT) )
	{
	    /* 出金時は識別センサがAcceptorとStackerの境目 */
	    /* 入金時は出口センサ */
	    /* スイッチバック時は入金方向へ駆動しているが出金紙幣と同じ扱い */
        if( //2025-05-24 (ex_recovery_info.step != RECOVERY_STEP_PAYOUT_ESCROW)       &&
            (ex_recovery_info.step != RECOVERY_STEP_PAYOUT_POS1  )       &&
            (ex_recovery_info.step != RECOVERY_STEP_SWITCHBACK_TRANSPORT) )
        {
			rtn = BILL_IN_ACCEPTOR;
		}
		else
		{
			rtn = BILL_IN_STACKER;
		}
	}
	else if((RS_POS2_ON || RS_POS3_ON) && (is_rc_rs_unit()) )
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if((RS_POS1_ON) && (is_rc_rs_unit()) )
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	else if( ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0) )
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	// センサー4.5.6,D,E,F
	else if((((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) || ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0))
	&&		(ex_rc_status.sst1A.bit.quad))
	{
		rtn = BILL_IN_STACKER;//BILL_IN_RC;
	}
	else if ( SENSOR_EXIT )
	{
		rtn = BILL_IN_STACKER;
	}
	else if ((SENSOR_ENTRANCE)||(SENSOR_CENTERING))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}

	return rtn;
}

#endif // UBA_RS


/*********************************************************************//**
 * @brief check bill in
 * @param[in]	None
 * @return 		Bill Remain No
 **********************************************************************/
u16 _main_bill_in(void) //ok RTQは _main_bill_remain も同じ
{
	u16 rtn;

#if defined(UBA_RTQ)
	if(is_rc_rs_unit())
	{
		if((ex_recovery_info.step == RECOVERY_STEP_PAYOUT_DRUM)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_TRANSPORT)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_POS1)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_POS7)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
		|| (ex_recovery_info.step == RECOVERY_STEP_SWITCHBACK_TRANSPORT))
		{
			rtn = _main_judge_bill_in_rs_model();
		}
		else
		{
			rtn = _main_judge_bill_in_normal_model();
		}
	}
	else
	{
		rtn = _main_judge_bill_in_normal_model();
	}
#else
	//2024-02-27
	if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
	{
		if(!(SENSOR_VALIDATION_OFF))
		{
			rtn = BILL_IN_ACCEPTOR;
			return rtn;
		}
	}

	if ((SENSOR_CENTERING) || (SENSOR_APB_IN) )
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if ( SENSOR_EXIT )
	{
		rtn = BILL_IN_STACKER;
	}
	else if ( SENSOR_APB_OUT )
	{
	/* PB OUTのみ、Acceptor JAMか Stacker JAMか切り分けるのが難しい為 その他のセンサを含めて判断	*/
	/* メカの構造上、Headを空けた瞬間、PB OUTのみ検知してしまう,どちらが適切が難しい				*/
		rtn = BILL_IN_ACCEPTOR;
	}
	else if ((SENSOR_ENTRANCE))
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}

#endif // UBA_RS
	return rtn;
}

u16 _main_stay_bill_check(void)	//mode_reject //ok RTQは _main_bill_remain と _main_bill_in と同じ
{
	u16 rtn;
#if defined(UBA_RTQ)
	if(is_rc_rs_unit())
	{
		if((ex_recovery_info.step == RECOVERY_STEP_PAYOUT_DRUM)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_TRANSPORT)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_POS1)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_POS7)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW)
		|| (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
		|| (ex_recovery_info.step == RECOVERY_STEP_SWITCHBACK_TRANSPORT))
		{
			rtn = _main_stay_bill_check_rs_model();
		}
		else
		{
			rtn = _main_stay_bill_check_normal_model();
		}
	}
	else
	{
		rtn = _main_stay_bill_check_normal_model();
	}
#else
	//2024-02-27
	if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
	{
		if(!(SENSOR_VALIDATION_OFF))
		{
			rtn = BILL_IN_ACCEPTOR;
			return rtn;
		}
	}

	if ( /*!(SENSOR_VALIDATION_OFF) || */(SENSOR_APB_IN) )
	{
		rtn = BILL_IN_ACCEPTOR;
	}
	else if ( SENSOR_EXIT )
	{
		rtn = BILL_IN_STACKER;
	}
	else if ( SENSOR_APB_OUT )
	{
	/* PB OUTのみ、Acceptor JAMか Stacker JAMか切り分けるのが難しい為 その他のセンサを含めて判断	*/
	/* メカの構造上、Headを空けた瞬間、PB OUTのみ検知してしまう,どちらが適切が難しい				*/
		rtn = BILL_IN_ACCEPTOR;
	}
	else if ( (SENSOR_ENTRANCE) || (SENSOR_CENTERING) )
	{
		rtn = BILL_IN_ENTRANCE;
	}
	else
	{
		rtn = BILL_IN_NON;
	}
#endif // UBA_RS
	return rtn;

}

/*********************************************************************//**
 * @brief is position sensor all off
 * @param[in]	None
 * @return 		true  : all off
 * 				false : not all off
 **********************************************************************/
bool _is_main_position_all_off(void)
{
	if (!(SENSOR_ENTRANCE) && !(SENSOR_CENTERING) && !(SENSOR_APB_IN)
	 && !(SENSOR_APB_OUT) && !(SENSOR_EXIT))
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*********************************************************************//**
 * @brief is accept denomi
 * @param[in]	denomi_code : denomination code
 * @return 		true  : normal sensor state
 * 				false : abnormal sensor state
 **********************************************************************/
bool _is_main_accept_denomi(u16 denomi_code)
{
	extern const u8 accept_denomi[];

	if (denomi_code >= DENOMI)
	{
		return false;
	}

	if (accept_denomi[denomi_code] != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}



/*********************************************************************//**
 * @brief convert denomi index to denomi code
 * @param[in]	denomi_index : denomination index
 * @return 		denomi_code  :
 *              0 : not denomi
 **********************************************************************/
u32 convert_index_to_denomi(u16 denomi_code)
{
	extern const u8 accept_denomi[];

	if(denomi_code < DENOMI_SIZE)
	{
		return accept_denomi[denomi_code];
	}
	else
	{
		return 0;
	}
}


void _main_set_mode(u8 mode1, u8 mode2)
{

	#if defined(UBA_RTQ_AZ_LOG)	//2023-09-05
	if(ex_fram_log_enable == 1)  //テストモード開始後1
	{
		ex_fram_uba[2] = ex_fram_uba[0];
		ex_fram_uba[3] = ex_fram_uba[1];
	}
	#endif

	ex_main_task_mode1 = mode1;
	ex_main_task_mode2 = mode2;
	ex_main_task_mode = (u16)((ex_main_task_mode1 << 8) |ex_main_task_mode2);

#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
	save_phase_fram_log();
#endif

	#if defined(UBA_RTQ_AZ_LOG)	//2023-09-05
	if(ex_fram_log_enable == 1)  //テストモード開始後1
	{
		ex_fram_uba[0] = ex_main_task_mode1;
		ex_fram_uba[1] = ex_main_task_mode2;

		_hal_write_fram_debug_log_uba();
	}
	#endif

#ifdef _ENABLE_JDL
	//if(ex_main_task_mode1 != MODE1_POWERUP)
	if(ex_start_jdl == 1)
	{
    	jdl_add_trace(ID_MAIN_TASK, (ex_main_task_mode1), (ex_main_task_mode2), ex_abnormal_code, (ex_validation.reject_code & 0xFF),
		( ( (ex_uba710 << 4) & 0xF0 ) | ( ex_is_uba_mode & 0x0F ) )		);
	}
#endif /* _ENABLE_JDL */

	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
	if(!
		(
		(
		(ex_main_task_mode1 == MODE1_ENABLE) &&
		(ex_main_task_mode2 == ENABLE_MODE2_WAIT_BILL_IN)
		)
		||
		(
		(ex_main_task_mode1 == MODE1_DISABLE) &&
		(ex_main_task_mode2 == DISABLE_MODE2_WAIT_REQ)
		)
		)
	)
	{
		ex_rtq_rfid_write_disable = 0;
	}

	#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)
	if(
		(ex_main_task_mode1 == MODE1_PAYOUT) &&
		(ex_main_task_mode2 == PAYOUT_MODE2_WAIT_RC_RSP)
	)
	{
		_main_send_msg(ID_ICB_MBX, TMSG_ICB_ACCEPT_RTQ_REQ, RFID_BACK, OperationDenomi.unit, RFID_BACK_PAYOUT, 0); //Payout時-リカバリフラグ有効 2025-07-23
	}
	#endif //end RFID_RECOVER

#if defined(UBA_LOG)	//2025-07-30
	ex_mode_log[ex_mode_log_index] = mode1;
	ex_mode_log[ex_mode_log_index+1] = mode2;
	ex_mode_log_index = ex_mode_log_index + 2;
	ex_mode_log_index = ex_mode_log_index% (EX_MODE_LOG-2);
#endif


	#endif

}


/*********************************************************************//**
 * @brief reject sub function
 * @param[in]	msg  : respons(send) TMSG
 * 				arg1 : argument 1
 * 				arg2 : argument 2
 * 				arg3 : argument 3
 * 				arg4 : argument 4
 * @return 		None
 **********************************************************************/
void _main_send_connection_task(u32 msg, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	if (!(is_test_mode())
	 || (msg == TMSG_CONN_INITIAL))
	{
		_main_send_msg(ID_CLINE_MBX, (msg|TMSG_TCODE_MAIN), arg1, arg2, arg3, arg4);
	}
	if ((is_test_mode())
	 || (msg == TMSG_CONN_INITIAL))
	{
		_main_send_msg(ID_DLINE_MBX, (msg|TMSG_TCODE_MAIN), arg1, arg2, arg3, arg4);
	}
}

u8 alarm_index(void)
{
	u8 count;
	u32 result = 0;
	u32 check_bit = 0x00000001;

	if(ex_multi_job.alarm == 0)
	{
		return(MULTI_END-1);
	}


	for(count = 0; count < MULTI_END; count++)
	{
		check_bit = 0x00000001 << count;
		result = ex_multi_job.alarm & check_bit;
		if( result == ex_multi_job.alarm)
		{
			return(count);
		}
	}

	/* 複数エラーがある場合*//* タスク番号が小さい順 */
	for(count = 0; count < MULTI_END; count++)
	{
		if( 0 != ex_multi_job.code[count])
		{
			return(count);
		}
	}


	return(MULTI_END-1);

}

u8 reject_index(void)
{
	u8 count;
	u32 result = 0;
	u32 check_bit = 0x00000001;

	if(ex_multi_job.reject == 0)
	{
		return(MULTI_END-1);
	}


	for(count = 0; count < MULTI_END; count++)
	{
		check_bit = 0x00000001 << count;
		result = ex_multi_job.reject & check_bit;
		if( result == ex_multi_job.reject)
		{
			return(count);
		}
	}

	/* 複数エラーがある場合*//* タスク番号が小さい順 */
	for(count = 0; count < MULTI_END; count++)
	{
		if( 0 != ex_multi_job.code[count])
		{
			return(count);
		}
	}


	return(MULTI_END-1);

}

void clear_ex_multi_job(void)
{
	memset(&ex_multi_job, 0, sizeof(ex_multi_job));
}

u8 denomi_to_escrow_code(u32 denomi)
{
#if defined(_PROTOCOL_ENABLE_ID003)
	return _is_id003_escrow_data(denomi);
#else
#endif
}


u32 is_cleaning_required(void)  //現状 特殊 Toolでのみ使用
{
	if(is_position_sensor_da_and_gain_max())
	{
		return 1;
	}
	if(is_cis_pga_max())
	{
		return 1;
	}
	return 0;
}

u32 is_temperature_warn(void) //2024-05-28
{
	//搬送モーターが閾値温度以上だとワーニングにする。
	//識別時に待機を追加
	//UBAは搬送モータ温度ICない
	ex_100msec_wait = 3;	//2024-11-21 1core
	do{
		OSW_TSK_sleep(10);
	}while( ((ex_multi_job.busy & TASK_ST_MGU) != 0) && ex_100msec_wait != 0);

	//59度以上 無限待ち
	//58度以上 5s待ち
	//56度以上 2s待ち
	ex_is_cis_high=0;
	#if 1
	if ((ex_temperature.cis_a >= 63)
	 || (ex_temperature.cis_b >= 63))
	{
		#if !defined(QA_TEST_SAFE)
		ex_is_cis_high=1;
		#endif
		OSW_TSK_sleep(5000);
	}
	else if ((ex_temperature.cis_a >= 60)
	 || (ex_temperature.cis_b >= 60))
	{
		OSW_TSK_sleep(5000);
	}
	else if ((ex_temperature.cis_a >= 56)
	 || (ex_temperature.cis_b >= 56))
	{
	//	OSW_TSK_sleep(2000);
	}
	#else //テスト用
	if ((ex_temperature.cis_a >= 45)
	 || (ex_temperature.cis_b >= 45))
	{
		ex_is_cis_high=1;
		OSW_TSK_sleep(5000);
	}
	else if ((ex_temperature.cis_a >= 40)
	 || (ex_temperature.cis_b >= 40))
	{
		OSW_TSK_sleep(5000);
	}
	else if ((ex_temperature.cis_a >= 38)
	 || (ex_temperature.cis_b >= 38))
	{
		OSW_TSK_sleep(2000);
	}
	#endif
}
u32 is_temperature_alarm_set(void) //2024-05-28
{
	//上下CISのどちらかが閾値温度以上だとアラームにする
	if ((ex_temperature.cis_a >= 63)
	 || (ex_temperature.cis_b >= 63))
	{
		return 1;
	}
	return 0;
}
u32 is_temperature_alarm_clear(void) //2024-05-28
{
	//上下CIS両方が閾値温度以下だとアラーム解除する
	//搬送モーターが閾値温度以下だとアラーム解除する
	if ((ex_temperature.cis_a <= 61)
	 && (ex_temperature.cis_b <= 61)
	)
	{
		return 1;
	}
	return 0;
}

#if defined(UBA_RTQ)
u32 get_TMSG_CONNECTION(void) //2024-11-13
{
	u32 data;
	switch (ex_main_task_mode1)
	{
	case MODE1_POWERUP:
		data = TMSG_CONN_NOTICE;
		break;
	case MODE1_INIT:
		data = TMSG_CONN_INITIAL;
		break;
	case MODE1_ADJUST:
		data = TMSG_CONN_DISABLE;
		break;
	case MODE1_DISABLE:
		data = TMSG_CONN_DISABLE;
		break;
	case MODE1_ENABLE:
		data = TMSG_CONN_ENABLE;
		break;
	case MODE1_ACTIVE_DISABLE:
		data = TMSG_CONN_DISABLE;
		break;
	case MODE1_ACTIVE_ENABLE:
		data = TMSG_CONN_ENABLE;
		break;
	case MODE1_ACCEPT:
		data = TMSG_CONN_ACCEPT;
		break;
	case MODE1_STACK:
		data = TMSG_CONN_STACK;
		break;
	case MODE1_REJECT:
		data = TMSG_CONN_REJECT;
		break;
	case MODE1_REJECT_STANDBY:
		data = TMSG_CONN_REJECT;
		break;
	case MODE1_ALARM:
		data = TMSG_CONN_NOTICE;
		break;
	case MODE1_ACTIVE_ALARM:
		data = TMSG_CONN_NOTICE;
		break;
	case MODE1_TEST_STANDBY:
		data = TMSG_CONN_NOTICE;
		break;
	case MODE1_TEST_ACTIVE:
		data = TMSG_CONN_NOTICE;
		break;
	case MODE1_COLLECT:
		data = TMSG_CONN_COLLECT;
		break;
	case MODE1_PAYOUT:
		data = TMSG_CONN_RC_PAYOUT;
		break;
	default:
		/* system error ? */
		data = TMSG_CONN_NOTICE;
		break;	
	}
	return(data);
}

void set_rtq_jam(u32 code, u8 ex_main_emergency_flag_bk)
{

	switch(code)
	{
	case	ALARM_CODE_FEED_MOTOR_SPEED_LOW:       // Failure A5 搬送スピードエラー
	case	ALARM_CODE_FEED_MOTOR_SPEED_HIGH:      // Failure A5 搬送スピードエラー
	case	ALARM_CODE_FEED_MOTOR_LOCK:            // Failure A6 搬送エラー
	//case	ALARM_CODE_ENTRY_MOTOR_LOCK:           // Failure A6 搬送エラー
	case	ALARM_CODE_FEED_OTHER_SENSOR_AT:       // JAM IN ACCEPTOR
	case	ALARM_CODE_FEED_SLIP_AT:               // JAM IN ACCEPTOR
	case	ALARM_CODE_FEED_TIMEOUT_AT:            // JAM IN ACCEPTOR
	case	ALARM_CODE_FEED_MOTOR_LOCK_AT:         // JAM IN ACCEPTOR
	case	ALARM_CODE_APB_HOME:                   // Failure A9 PBエラー, UBA Failure AF
	case	ALARM_CODE_APB_TIMEOUT:                // Failure A9 PBエラー, UBA Failure AF
	case	ALARM_CODE_APB_HOME_STOP:              // Failure A9 PBエラー, UBA Failure AF
	case	ALARM_CODE_CENTERING_TIMEOUT:          // Failure C1 幅寄せエラー, UBA Failure AF
	case	ALARM_CODE_CENTERING_HOME_STOP:        // Failure C1 幅寄せエラー, UBA Failure AF
	ex_cline_status_tbl.ex_rc_after_jam = 0;
			break;
	case	ALARM_CODE_BOX:                        // BOX OPEN
	case	ALARM_CODE_BOX_INIT:                   // BOX OPEN
			if(is_quad_model())
			{
				if( (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0 &&
					(ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) == 0 &&
					(ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0 &&
					(ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) == 0 &&
					!(SENSOR_APB_IN)									 &&
					!(SENSOR_APB_OUT)									 &&
					!(SENSOR_EXIT))
				{
					ex_cline_status_tbl.ex_rc_after_jam = 0;
				}
				else
				{
					ex_cline_status_tbl.ex_rc_after_jam = 1;
				}
			}
			else
			{
				if( (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0 &&
					(ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0 &&
					!(SENSOR_APB_IN)									 &&
					!(SENSOR_APB_OUT)									 &&
					!(SENSOR_EXIT))
				{
					ex_cline_status_tbl.ex_rc_after_jam = 0;
				}
				else
				{
					ex_cline_status_tbl.ex_rc_after_jam = 1;
				}
			}
			break;
	default: /*22-FEB-24*/
			if((ex_main_emergency_flag_bk == 1) && (code == ALARM_CODE_FEED_TIMEOUT_SK))
			{
				if (ex_rc_status.sst1A.bit.error)
				{
					ex_cline_status_tbl.ex_rc_after_jam = 1;
				}
				else
				{
					ex_cline_status_tbl.ex_rc_after_jam = 0;
				}
			}
			else
			{
				ex_cline_status_tbl.ex_rc_after_jam = 1;
			}
			break;
	}

	if(is_rc_rs_unit())
	{
		_main_send_msg(ID_RC_MBX, TMSG_RS_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
	}

	_main_send_msg(ID_RC_MBX, TMSG_RC_CANCEL_REQ, 0, 0, 0, 0);

}

#endif // UBA_RTQ



/* EOF */
