/******************************************************************************/
/*! @addtogroup Group2
    @file       feed_rc_unit.c
    @brief      
    @date       2024/05/15
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/

#if defined(UBA_RTQ)
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "sub_functions.h"
#include "motor_ctrl.h"
#include "sensor.h"
#include "sensor_ad.h"
#include "status_tbl.h"
#include "feed_rc_unit.h"
#include "if_rc.h"
#include "feed.h" //2024-08-01

#define EXT
#include "com_ram.c"

#include "jdl_conf.h"
#if defined(_PROTOCOL_ENABLE_ID003)
	#include "task/cline_task/003/id003.h"
#endif


static u8 s_feed_unit;				/* Unit RC number of stack */
static u8 s_feed_recover;			/* Step recovery */
static u8 s_feed_payout_option;		/* Payotu option  */
static u8 s_feed_search_direction;  /* Search direction */
static u8 s_feed_payout_last;		/* remain or not */

static u8 s_feed_rc_sensor_backup;

void _feed_rc_stack_1200_seq_start(void);
static void _feed_rc_stack_1200_seq(u32 flag);
static void _feed_rc_stack_12ff_seq(u32 flag);

/* TWIN */
static void _feed_rc_stack_1201_seq(u32 flag);
static void _feed_rc_stack_1202_seq(u32 flag);
static void _feed_rc_stack_1203_seq(u32 flag);
static void _feed_rc_stack_1204_seq(u32 flag);
static void _feed_rc_stack_1206_seq(u32 flag);
/* QUAD */
static void _feed_rc_stack_1211_seq(u32 flag);
static void _feed_rc_stack_1212_seq(u32 flag);
static void _feed_rc_stack_1213_seq(u32 flag);
static void _feed_rc_stack_1214_seq(u32 flag);
static void _feed_rc_stack_1215_seq(u32 flag);
static void _feed_rc_stack_1217_seq(u32 flag);
/* Cash box TWIN */
static void _feed_rc_stack_1221_seq(u32 flag);
static void _feed_rc_stack_1222_seq(u32 flag);
static void _feed_rc_stack_1223_seq(u32 flag);
static void _feed_rc_stack_1224_seq(u32 flag);
static void _feed_rc_stack_1225_seq(u32 flag);
static void _feed_rc_stack_1227_seq(u32 flag);
/* Cash box QUAD */
static void _feed_rc_stack_1231_seq(u32 flag);
static void _feed_rc_stack_1232_seq(u32 flag);
static void _feed_rc_stack_1233_seq(u32 flag);
static void _feed_rc_stack_1234_seq(u32 flag);
static void _feed_rc_stack_1235_seq(u32 flag);
static void _feed_rc_stack_1236_seq(u32 flag);
static void _feed_rc_stack_1238_seq(u32 flag);

/* STACK FORCE PAYOUT */
static void _feed_rc_fpayout_1600_seq(u32 flag); //payout以外でも使用している。Payoutというより、ドラムから背面搬送まで戻す処理、 mode_init, mode_payout,mode_stack
static void _feed_rc_fpayout_1601_seq(u32 flag);
static void _feed_rc_fpayout_1602_seq(u32 flag);
static void _feed_rc_fpayout_1603_seq(u32 flag);

/* PAYOUT */
void _feed_rc_payout_seq_proc(u32 flag); //only mode payout
static void _feed_rc_payout_1300_seq(u32 flag);
static void _feed_rc_payout_1301_seq(u32 flag);
static void _feed_rc_payout_1302_seq(u32 flag);
static void _feed_rc_payout_1303_seq(u32 flag);
static void _feed_rc_payout_1304_seq(u32 flag);
static void _feed_rc_payout_1305_seq(u32 flag);
static void _feed_rc_payout_1306_seq(u32 flag);
static void _feed_rc_payout_1307_seq(u32 flag);
static void _feed_rc_payout_1308_seq(u32 flag);
static void _feed_rc_payout_1309_seq(u32 flag);
static void _feed_rc_payout_1310_seq(u32 flag);
static void _feed_rc_payout_1311_seq(u32 flag);
static void _feed_rc_payout_1312_seq(u32 flag);
static void _feed_rc_payout_1313_seq(u32 flag);

/* PAYOUT FORCE STACK */
static void _feed_rc_fstack_1700_seq(u32 flag);
static void _feed_rc_fstack_1701_seq(u32 flag);
static void _feed_rc_fstack_1702_seq(u32 flag);
static void _feed_rc_fstack_1703_seq(u32 flag);


/*COLECCT */
static void _feed_rc_collect_1400_seq(u32 flag);
static void _feed_rc_collect_1401_seq(u32 flag);
static void _feed_rc_collect_1402_seq(u32 flag);
static void _feed_rc_collect_1403_seq(u32 flag);
static void _feed_rc_collect_1404_seq(u32 flag);
static void _feed_rc_collect_1405_seq(u32 flag);
static void _feed_rc_collect_1411_seq(u32 flag);
static void _feed_rc_collect_1412_seq(u32 flag);
static void _feed_rc_collect_1413_seq(u32 flag);
static void _feed_rc_collect_1414_seq(u32 flag);
static void _feed_rc_collect_1415_seq(u32 flag);


/* BILLBACK */
static void _feed_rc_billback_1800_seq(u32 flag);
static void _feed_rc_billback_1801_seq(u32 flag);
static void _feed_rc_billback_1802_seq(u32 flag);

/* SERACH BILL */
static void _feed_rc_search_bill_0a00_seq(u32 flag);
static void _feed_rc_search_bill_0a01_seq(u32 flag);
static void _feed_rc_search_bill_0a02_seq(u32 flag);
static void _feed_rc_search_bill_0a03_seq(u32 flag);
static void _feed_rc_search_bill_0a04_seq(u32 flag);
static void _feed_rc_search_bill_0a05_seq(u32 flag);


static void _feed_rc_payout_stop_1900_seq(u32 flag);
static void _feed_rc_payout_stop_1901_seq(u32 flag);

static void _feed_rs_payout_2300_seq(u32 flag);
static void _feed_rs_payout_2301_seq(u32 flag);
static void _feed_rs_payout_2302_seq(u32 flag);
static void _feed_rs_payout_2303_seq(u32 flag);
static void _feed_rs_payout_2304_seq(u32 flag);
static void _feed_rs_payout_2305_seq(u32 flag);
static void _feed_rs_payout_2306_seq(u32 flag);
static void _feed_rs_payout_2307_seq(u32 flag);
static void _feed_rs_payout_2308_seq(u32 flag);
static void _feed_rs_payout_2309_seq(u32 flag);
static void _feed_set_feed_rs_payout_fwd_retry(u32 alarm_code);
static void _feed_rs_payout_2320_seq(u32 flag);
static void _feed_rs_payout_2321_seq(u32 flag);
static void _feed_rs_payout_2322_seq(u32 flag);
static void _feed_set_feed_rs_payout_rev_retry(u32 alarm_code);
static void _feed_rs_payout_2330_seq(u32 flag);
static void _feed_rs_payout_2331_seq(u32 flag);
static void _feed_rs_payout_2332_seq(u32 flag);
/* RS FORCE PAYOUT */
static void _feed_rs_fpayout_2400_seq(u32 flag);
static void _feed_rs_fpayout_2401_seq(u32 flag);
static void _feed_rs_fpayout_2402_seq(u32 flag);
static void _feed_rs_fpayout_2403_seq(u32 flag);
static void _feed_rs_fpayout_2404_seq(u32 flag);
static void _feed_rs_fpayout_2405_seq(u32 flag);
static void _feed_set_feed_rs_fpayout_fwd_retry(u32 alarm_code);
static void _feed_rs_fpayout_2420_seq(u32 flag);
static void _feed_rs_fpayout_2421_seq(u32 flag);
static void _feed_rs_fpayout_2422_seq(u32 flag);


/*********************************************************************/
/**
 * @brief Set feed rc unit
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void feed_rc_set_unit(u8 unit)
{
    s_feed_unit = unit;
}

void feed_rc_set_recover(u8 recover)
{
    s_feed_recover = recover;
}

void feed_rc_set_payout_option(u8 option)
{
    s_feed_payout_option = option;
}

void feed_rc_set_search_dir(u8 dir)
{
	s_feed_search_direction = dir;
}


void feed_rc_set_sensor_bk(u8 status)
{
	s_feed_rc_sensor_backup = status;
}

//#if defined(UBA_RS)
void feed_rc_set_payout_last(u8 data)
{
	s_feed_payout_last = data;
}
//#endif

/*********************************************************************/
/**
 * @brief feed control sub function
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
static void _feed_rc_set_cancel(void)
{
	u32 msg;
	/* STOP MOTOR */
	motor_ctrl_feed_stop();

	switch (ex_feed_task_seq & 0xFF00)
	{
	case FEED_SEQ_RC_STACK:
		msg = TMSG_FEED_RC_STACK_RSP;
		break;
	case FEED_SEQ_RC_PAYOUT:
	case FEED_SEQ_RS_PAYOUT:
		msg = TMSG_FEED_RC_PAYOUT_RSP;
		break;
	case FEED_SEQ_RC_PAYOUT_STOP: /* '21-10-11 */
		msg = TMSG_FEED_RC_PAYOUT_STOP_RSP;
		break;
	case FEED_SEQ_RC_COLLECT:
		msg = TMSG_FEED_RC_COLLECT_RSP;
		break;
	case FEED_SEQ_RC_FORCE_PAYOUT:
		msg = TMSG_FEED_RC_FORCE_PAYOUT_RSP;
		break;
	case FEED_SEQ_RC_FORCE_STACK:
		msg = TMSG_FEED_RC_FORCE_STACK_RSP;
		break;
	case FEED_SEQ_RC_BILL_BACK:
		msg = TMSG_FEED_RC_BILLBACK_RSP;
		break;
	case FEED_SEQ_RC_CLEANING:
		// msg = TMSG_FEED_RC_CLEANING_RSP;
		break;
	case FEED_SEQ_RS_FORCE_PAYOUT:
		msg = TMSG_FEED_RS_FORCE_PAYOUT_RSP;
		break;
	default:
		msg = TMSG_FEED_STATUS_INFO;	
		break;
	}

	_feed_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_SUCCESS, 0, 0, 0);
	_feed_set_seq(FEED_SEQ_IDLE, 0);
}


/*********************************************************************//**
 * @brief feed control sub function
 *  all sensor status
 * @param[in]	None
 * @return 		true  : all sensor off
 * 				false : any sensor on
 **********************************************************************/
static bool is_rc_busy(void)
{
	if(ex_rc_status.sst1A.bit.busy)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void _feed_rc_stack_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	/* Common */
	case 0x00:
		_feed_rc_stack_1200_seq(flag);
		break;
	case 0xff:
		_feed_rc_stack_12ff_seq(flag);
		break;
	/* transport to RC-Twin */
	case 0x01:
		_feed_rc_stack_1201_seq(flag);
		break;
	case 0x02:
		_feed_rc_stack_1202_seq(flag);
		break;
	case 0x03:
		_feed_rc_stack_1203_seq(flag);
		break;
	case 0x04:
		_feed_rc_stack_1204_seq(flag);
		break;
	case 0x06:
		_feed_rc_stack_1206_seq(flag);
		break;
	/* transport to RC-Quad */
	case 0x11:
		_feed_rc_stack_1211_seq(flag);
		break;
	case 0x12:
		_feed_rc_stack_1212_seq(flag);
		break;
	case 0x13:
		_feed_rc_stack_1213_seq(flag);
		break;
	case 0x14:
		_feed_rc_stack_1214_seq(flag);
		break;
	case 0x15:
		_feed_rc_stack_1215_seq(flag);
		break;
	case 0x17:
		_feed_rc_stack_1217_seq(flag);
		break;
	/* transport to Cash box(RC-Twin) */
	case 0x21:
		_feed_rc_stack_1221_seq(flag);
		break;
	case 0x22:
		_feed_rc_stack_1222_seq(flag);
		break;
	case 0x23:
		_feed_rc_stack_1223_seq(flag);
		break;
	case 0x24:
		_feed_rc_stack_1224_seq(flag);
		break;
	case 0x25:
		_feed_rc_stack_1225_seq(flag);
		break;
	case 0x27:
		_feed_rc_stack_1227_seq(flag);
		break;
	/* transport to Cash box(RC-Quad) */
	case 0x31:
		_feed_rc_stack_1231_seq(flag);
		break;
	case 0x32:
		_feed_rc_stack_1232_seq(flag);
		break;
	case 0x33:
		_feed_rc_stack_1233_seq(flag);
		break;
	case 0x34:
		_feed_rc_stack_1234_seq(flag);
		break;
	case 0x35:
		_feed_rc_stack_1235_seq(flag);
		break;
	case 0x36:
		_feed_rc_stack_1236_seq(flag);
		break;
	case 0x38:
		_feed_rc_stack_1238_seq(flag);
		break;
	default: /* other	*/
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 31);
		break;
	}
}


void _feed_rc_stack_1200_seq_start(void)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, WAIT_PB_IN_OFF ))
	{
		_feed_set_seq(0x12ff, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1200_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, WAIT_PB_IN_OFF ))
	{
		_feed_set_seq(0x12ff, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}



/*********************************************************************//**
 * @brief feed control sequence 0x3000
 *  Motor APB start 
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
//UBA500はモータ起動確認を見ていなくて0x1200ではモータ起動済みとして処理している
//モータ起動を確認した方がいいので、UBA700では0x1200をモータ起動確認、0x12ffをUBA500の0x1200と同様のPB IN抜け待ち
static void _feed_rc_stack_12ff_seq(u32 flag) //UBA500は0x1200、
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_APB_IN))	// PB IN OFF
	{
        if( ex_main_emergency_flag == 0 )
		{
		    if(s_feed_recover != PAYREJECT)
			{
				set_recovery_step(RECOVERY_STEP_APB_IN);
			}
		}
		switch (s_feed_unit)
		{
		/* リサイクラへの搬送の時のみ、完了前にmainに TMSG_SUB_INTERIM を通知*/
		case RC_TWIN_DRUM1:
		case RC_TWIN_DRUM2:
			if(is_rc_rs_unit())
			{
				_feed_set_seq(0x1201, FEED_SEQ_RS_STACK_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1201, FEED_SEQ_RC_STACK_TIMEOUT);
			}
			break;
		case RC_QUAD_DRUM1:
		case RC_QUAD_DRUM2:
			if(is_rc_rs_unit())
			{
				_feed_set_seq(0x1211, FEED_SEQ_RS_STACK_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1211, FEED_SEQ_RC_STACK_TIMEOUT);
			}
			break;
		default:
			if (!(is_feed_quad_model()))
			{
				/* RC-Twin model  */
				if(is_rc_rs_unit())
				{
					_feed_set_seq(0x1221, FEED_SEQ_RS_STACK_TIMEOUT);
				}
				else
				{
					_feed_set_seq(0x1221, FEED_SEQ_RC_STACK_TIMEOUT);
				}
			}
			else
			{
				/* RC-Quad model  */
				if(is_rc_rs_unit())
				{
					_feed_set_seq(0x1231, FEED_SEQ_RS_STACK_TIMEOUT);
				}
				else
				{
					_feed_set_seq(0x1231, FEED_SEQ_RC_STACK_TIMEOUT);
				}
			}
			break;
		}
	}
}

/* TWIN */
/*********************************************************************//**
 * @brief feed control sequence 0x3002
 *  wait rc pos1 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_stack_1201_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS1_ON)
	{
        if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_TRANSPORT);
		}
		_feed_set_seq(0x1202, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x3003
 *  wait rc pos2 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_stack_1202_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS2_ON)
	{
		_feed_set_seq(0x1203, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x3004
 *  wait rc pos2 sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_stack_1203_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if( ( (RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1) && (s_feed_rc_sensor_backup == FALSE) ) ||
		((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2) && (s_feed_rc_sensor_backup == FALSE))        )
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_DRUM);
		}
        s_feed_rc_sensor_backup = TRUE;
    }
	else if(!(RC_POS2_ON))
	{
		_feed_set_seq(0x1204, FEED_SEQ_TIMEOUT);
		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE(40/PITCH)));
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x3005
 *  wait arrive feed motoe pulse
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_stack_1204_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if( ((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1) && (s_feed_rc_sensor_backup == FALSE)) ||
			 ((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2) && (s_feed_rc_sensor_backup == FALSE)) )
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_DRUM);
		}
		s_feed_rc_sensor_backup = TRUE;
	}
	else if(RC_POS2_ON)
	{
		_feed_set_seq(0x1203, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag) && !(RC_POS2_ON))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x1206, FEED_SEQ_TIMEOUT);

		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_STACK_RSP, TMSG_SUB_INTERIM, 0, 0, 0);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x3006
 *  wait feed motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_stack_1206_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || (is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0)
	||		(!(SENSOR_VALIDATION_OFF)) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if( ((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1) && (s_feed_rc_sensor_backup == FALSE)) ||
			 ((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2) && (s_feed_rc_sensor_backup == FALSE)) )
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_DRUM);
		}
		s_feed_rc_sensor_backup = TRUE;
	}
	else if(is_motor_ctrl_feed_stop() && (s_feed_rc_sensor_backup == TRUE))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/* QUAD */
static void _feed_rc_stack_1211_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS1_ON)
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_TRANSPORT);
		}
		_feed_set_seq(0x1212, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1212_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS2_ON)
	{
		_feed_set_seq(0x1213, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1213_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS3_ON)
	{
		_feed_set_seq(0x1214, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1214_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if( ((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1) && (s_feed_rc_sensor_backup == FALSE)) ||
			 ((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2) && (s_feed_rc_sensor_backup == FALSE)) )
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_DRUM);
		}
		s_feed_rc_sensor_backup = TRUE;
	}
	else if(!(RC_POS3_ON))
	{
		_feed_set_seq(0x1215, FEED_RC_SEQ_TIMEOUT);
		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE(40/PITCH)));
	}
}

static void _feed_rc_stack_1215_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if( ((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1) && (s_feed_rc_sensor_backup == FALSE)) ||
			 ((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2) && (s_feed_rc_sensor_backup == FALSE)) )
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_DRUM);
		}
		s_feed_rc_sensor_backup = TRUE;
	}
	else if(RC_POS3_ON)
	{
		_feed_set_seq(0x1214, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag) && !(RC_POS3_ON))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x1217, FEED_SEQ_TIMEOUT);
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_STACK_RSP, TMSG_SUB_INTERIM, 0, 0, 0);
	}
}

static void _feed_rc_stack_1217_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || (is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0)
	|| (!(SENSOR_VALIDATION_OFF)) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if( ((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1) && (s_feed_rc_sensor_backup == FALSE)) ||
			 ((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2) && (s_feed_rc_sensor_backup == FALSE)) )
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_DRUM);
		}
		s_feed_rc_sensor_backup = TRUE;
	}
	else if(is_motor_ctrl_feed_stop() && (s_feed_rc_sensor_backup == TRUE))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}


/* Cash box TWIN */
static void _feed_rc_stack_1221_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(RC_POS1_ON)
	{
	    if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_TRANSPORT);
		}
		_feed_set_seq(0x1222, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1222_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(RC_POS2_ON)
	{
		_feed_set_seq(0x1223, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1223_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(RC_POS3_ON)
	{
		_feed_set_seq(0x1224, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1224_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(RC_POS3_ON))
	{
		_feed_set_seq(0x1225, FEED_SEQ_TIMEOUT);
		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE(40/PITCH)));
	}
}

static void _feed_rc_stack_1225_seq(u32 flag)
{	
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS3_ON)
	{
		_feed_set_seq(0x1224, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag) && !(RC_POS3_ON))
	{
		if (s_feed_recover != PAYREJECT)
		{
			if (s_feed_recover == STACK)
			{
				set_recovery_step(RECOVERY_STEP_STACKING);
			}
			else
			{
				set_recovery_step(RECOVERY_STEP_COLLECT_STACKING);
			}
		}
		motor_ctrl_feed_stop();
		_feed_set_seq(0x1227, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_stack_1227_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || (is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0)
	|| (!(SENSOR_VALIDATION_OFF)) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if( (is_motor_ctrl_feed_stop()) && (!(is_rc_busy())) )
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/* Cash box QUAD */
/*********************************************************************//**
 * @brief feed control sequence 0x3032
 *  wait rc pos3 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_stack_1231_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS1_ON)
	{
		if(s_feed_recover != PAYREJECT)
		{
			set_recovery_step(RECOVERY_STEP_STACK_TRANSPORT);
		}
		_feed_set_seq(0x1232, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x3033
 *  wait rc pos6 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_stack_1232_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS2_ON)
	{
		_feed_set_seq(0x1233, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}

static void _feed_rc_stack_1233_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS3_ON)
	{
		_feed_set_seq(0x1234, FEED_SEQ_RC_STACK_TIMEOUT);
	}
}	

static void _feed_rc_stack_1234_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_reject(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS6_ON)
	{
		_feed_set_seq(0x1235, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_stack_1235_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(!(RC_POS6_ON))
	{
		_feed_set_seq(0x1236, FEED_SEQ_TIMEOUT);
		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE(40/PITCH)));
	}

}

static void _feed_rc_stack_1236_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS6_ON)
	{
		_feed_set_seq(0x1235, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag) && !(RC_POS6_ON))
	{
		if (s_feed_recover != PAYREJECT)
		{
			if (s_feed_recover == STACK)
			{
				set_recovery_step(RECOVERY_STEP_STACKING);
			}
			else
			{
				set_recovery_step(RECOVERY_STEP_COLLECT_STACKING);
			}
		}
		motor_ctrl_feed_stop();
		_feed_set_seq(0x1238, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_stack_1238_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if( (is_motor_ctrl_feed_stop()) && (!(is_rc_busy())) )
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/**        PAYOUT      ***/

void _feed_rc_payout_seq_proc(u32 flag) //ok  //only mode payout
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:
		_feed_rc_payout_1300_seq(flag);
		break;
	case 0x01:
		_feed_rc_payout_1301_seq(flag);
		break;
	case 0x02:
		_feed_rc_payout_1302_seq(flag);
		break;
	case 0x03:
		_feed_rc_payout_1303_seq(flag);
		break;
	case 0x04:
		_feed_rc_payout_1304_seq(flag);
		break;
	case 0x05:
		_feed_rc_payout_1305_seq(flag);
		break;
	case 0x06:
		_feed_rc_payout_1306_seq(flag);
		break;
	case 0x07:
		_feed_rc_payout_1307_seq(flag);
		break;
	case 0x08:
		_feed_rc_payout_1308_seq(flag);
		break;
	case 0x09:
		_feed_rc_payout_1309_seq(flag);
		break;
	case 0x10:
		_feed_rc_payout_1310_seq(flag);
		break;
	case 0x11:
		_feed_rc_payout_1311_seq(flag);
		break;
	case 0x12:
		_feed_rc_payout_1312_seq(flag);
		break;
	case 0x13:
		_feed_rc_payout_1313_seq(flag);
		break;
	default: /* other	*/
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 16);
		break;
	}
}

static void _feed_rc_payout_1300_seq(u32 flag) //only mode payout
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else
	{
		if(s_feed_payout_option == FEED_PAYOUT_OPTION_RETRY) /* Retry payout */
		{
			if(!(SENSOR_VALIDATION_OFF) || SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT)
			{
				if(SENSOR_CENTERING)
				{
					if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_REJECT_CENTERING))
					{
						_feed_set_seq(0x1308, FEED_SEQ_TIMEOUT);
					}
				}
				else
				{
					if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_REJECT_CENTERING_ON_TO_OFF))
					{
						_feed_set_seq(0x1307, FEED_SEQ_TIMEOUT);
					}
				}
			}
			else if(SENSOR_CENTERING)
			{
				if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_REJECT_CENTERING))
				{
					_feed_set_seq(0x1308, FEED_SEQ_TIMEOUT);
				}
			}
			else if(SENSOR_ONLY_ENTRANCE_ON)
			{
				//入り口を除く全てのセンサがOFFの場合
				// Rev 30mm
				if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START))
				{
					_feed_set_seq(0x1309, FEED_SEQ_TIMEOUT);
				}
			}
			else
			{
				if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_REJECT_CENTERING_ON_TO_OFF))
				{
					_feed_set_seq(0x1309, FEED_SEQ_TIMEOUT);
				}
			}
		}
		else  // NORMAL
		{
			if(!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
			{
				_feed_set_alarm(ALARM_CODE_CHEAT);
			}
			else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
			{
				ex_rc_internal_jam_flag = 0;
				ex_rc_internal_jam_flag_bk = 0;
				_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
			}
			else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
			{
				ex_rc_internal_jam_flag = 0;
				ex_rc_internal_jam_flag_bk = 0;
				_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
			}
			else if((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
			{
				ex_rc_internal_jam_flag = 0;
				ex_rc_internal_jam_flag_bk = 0;
				_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
			}
			else if((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
			{
				ex_rc_internal_jam_flag = 0;
				ex_rc_internal_jam_flag_bk = 0;
				_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
			}
		}
	}
}

static void _feed_rc_payout_1301_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x1302, 30);
	}
	else if(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x1302, 30);
	}
	else if(!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
	{
		_feed_set_seq(0x1302, 30);
	}
	else if(!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
	{
		_feed_set_seq(0x1302, 30);
	}
}

static void _feed_rc_payout_1302_seq(u32 flag)
{
	if (!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN))
	{	
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
	{
		_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
	{
		_feed_set_seq(0x1301, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if( (!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1)) ||
			(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2)) ||
			(!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1)) ||
			(!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2)) )
		{
			set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
			_feed_set_seq(0x1303, FEED_SEQ_TIMEOUT);
		}
	}
}

static void _feed_rc_payout_1303_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS2_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
		_feed_set_seq(0x1304, FEED_SEQ_TIMEOUT);
	}
	else if(RC_POS1_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_POS1);
		_feed_set_seq(0x1305, FEED_SEQ_TIMEOUT);
	}
	else
	{
		if( (!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1)) ||
			(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2)) ||
			(!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1)) ||
			(!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2)) )
		{
			set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
		}
		else
		{
			set_recovery_step(RECOVERY_STEP_PAYOUT_DRUM);
		}
	}
}

static void _feed_rc_payout_1304_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS1_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_POS1);
		_feed_set_seq(0x1305, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_payout_1305_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(SENSOR_EXIT)
	{
		// cheat_check(0);
		_feed_set_seq(0x1306, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_payout_1306_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if (!(SENSOR_VALIDATION_OFF))
	{
#if 1	/* '20-07-03 */
		if(ex_main_test_no == TEST_RC_AGING || ex_main_test_no == TEST_RC_AGING_FACTORY)
		{
			ex_reject_escrow = 0;
		}
		else
		{
			ex_reject_escrow = 1;
		}
#endif
		/* set recovery */
		set_recovery_step(RECOVERY_STEP_PAYOUT_ESCROW);
		motor_ctrl_feed_set_pulse(FEED_REJECT_CENTERING_ON_TO_OFF);
		_feed_set_seq(0x1307, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_payout_1307_seq(u32 flag)
{
	if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0 || 
		(is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_payout_retry(FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
			_feed_set_feed_payout_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_feed_payout_retry(FEED_SLIP);
		}
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag))
	{
		if((SENSOR_VALIDATION_OFF) && !(SENSOR_APB_IN) && (SENSOR_APB_OUT || SENSOR_EXIT))
		{
			_feed_set_feed_payout_retry(FEED_SLIP);
		}
		else
		{
			_feed_set_feed_payout_retry(FEED_SLIP);
		}
	}
	else if(!(SENSOR_VALIDATION_OFF) && SENSOR_CENTERING && !(SENSOR_APB_IN))
	{
		_feed_set_seq(0x1308, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_payout_1308_seq(u32 flag)
{
	if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || 
	(is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_payout_retry(FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
			_feed_set_feed_payout_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_feed_payout_retry(FEED_SLIP);
		}
	}
	else if(SENSOR_APB_IN)
	{
		motor_ctrl_feed_set_pulse(FEED_REJECT_CENTERING_ON_TO_OFF);
		_feed_set_seq(0x1307, FEED_SEQ_TIMEOUT);
	}
	else
	{
		if (ex_reject_escrow == 1)
		{
		 	if((SENSOR_VALIDATION_OFF))
		 	{
		 		motor_ctrl_feed_set_pulse(FEED_HANGING_POSITION_NEW);
		 		_feed_set_seq(0x1309, FEED_SEQ_TIMEOUT);
		 	}
		}
		else
		{
			if(SENSOR_ONLY_ENTRANCE_ON)
			{
				motor_ctrl_feed_set_pulse(FEED_HANGING_POSITION_SHORT_TICKET);
				_feed_set_seq(0x1309, FEED_SEQ_TIMEOUT);
			}
		}
	}
}

static void _feed_rc_payout_1309_seq(u32 flag)
{
	if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || 
		(is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_payout_retry(FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(SENSOR_ONLY_ENTRANCE_ON || SENSOR_ALL_OFF)
		{
			/* not spray mode *//* '21-10-11 */
			if((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) != ID003_OPTION_SPRAY_MODE
			&& !(is_rc_rs_unit())
			)
			{
				motor_ctrl_feed_stop();
			}
			_feed_set_seq(0x1310, FEED_SEQ_TIMEOUT);
		}
		else
		{
			if(_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
			{
				_feed_set_feed_payout_retry(FEED_TIMEOUT);
			}
			else
			{
				_feed_set_feed_payout_retry(FEED_SLIP);
			}
		}
	}
	else if(SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT)
	{
		_feed_set_feed_payout_retry(FEED_OTHER_SENSOR);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_seq(0x1308, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag) && (SENSOR_VALIDATION_OFF))
	{
		/* not spray mode *//* '21-10-11 */
		if((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) != ID003_OPTION_SPRAY_MODE
		&& !(is_rc_rs_unit()))
		{
			motor_ctrl_feed_stop();
		}
		_feed_set_seq(0x1310, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_payout_1310_seq(u32 flag)
{
	/* spray mode *//* '21-10-11 */
	if((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE
	|| is_rc_rs_unit())
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
	else
	{
		if ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || 
		(is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
		else if (IS_FEED_EVT_MOTOR_LOCK(flag))
		{
			_feed_set_feed_payout_retry(FEED_MOTOR_LOCK);
		}
		else if (IS_FEED_EVT_TIMEOUT(flag))
		{
			if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
			{
				_feed_set_feed_payout_retry(FEED_TIMEOUT);
			}
			else
			{
				_feed_set_feed_payout_retry(FEED_SLIP);
			}
		}
		else if (is_motor_ctrl_feed_stop())
		{
			_feed_set_seq(0x1311, FEED_SEQ_STOP_CONF_TIMEOUT);
		}
	}
}

static void _feed_rc_payout_1311_seq(u32 flag)
{
	if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || 
	(is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	if(SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

// リトライ処理の開始シーケンス、モータ停止確認後、紙幣位置により正転距離を設定
static void _feed_rc_payout_1312_seq(u32 flag)
{
	u16 drive_pulse = 0;

	if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || 
		(is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_payout_retry(FEED_MOTOR_LOCK);
	}
	// モータが停止しないエラ-を追加してもいいかも
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
			_feed_set_feed_payout_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_feed_payout_retry(FEED_SLIP);
		}
	}
	#if 0	//2024-10-03 UBA500では存在しているが、実際はこの条件は発生しない
			//払い出し時にpulse_countをクリアしていないので、前の情報が残ったままなのと、
			//最後に受け取り動作させた紙幣情報なので、Payoutする紙幣の情報とは異なる
			//Payout時にカウンタ更新のモードにも設定していないので、払い出す紙幣の情報も取得していない
			//その為,UBA500ではコード上存在しているが、使用していない
	else if(ex_validation.pulse_count < (LENGTH_LOWER_LIMIT - 5))
	{
		/* only short bill	*/// Feed
		if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, FEED_REJECT_RETRY1_FWD_SHORT_PULSE)) // 20mm
		{
			_feed_set_seq(0x1313, FEED_REJECT_RETRY_FWD_TIME);
		}
	}
	#endif
	else if(SENSOR_CENTERING)
	{
		if(s_feed_alarm_retry == 1)
		{
			// 1st retry
			drive_pulse = FEED_REJECT_RETRY1_FWD_LONG_PULSE;
		}
		else
		{
			drive_pulse = FEED_REJECT_RETRY2_FWD_LONG_PULSE;
		}
		// Feed->Entry
		if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, drive_pulse)) // 70mm or 90mm
		{
			_feed_set_seq(0x1313, FEED_REJECT_RETRY_FWD_TIME);
		}
	}
	else
	{
		if(s_feed_alarm_retry == 1)
		{
			// 1st retry
			drive_pulse = FEED_REJECT_RETRY1_FWD_SHORT_PULSE;
		}
		else
		{
			drive_pulse = FEED_REJECT_RETRY2_FWD_SHORT_PULSE;
		}
		if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, drive_pulse))// 20mm or 40mm
		{
			_feed_set_seq(0x1313, FEED_REJECT_RETRY_FWD_TIME);
		}
	}
}

static void _feed_rc_payout_1313_seq(u32 flag)
{
	if((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0 || 
		(is_feed_quad_model() && (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_payout_retry(FEED_MOTOR_LOCK);
	}
	// モータが停止しないエラ-を追加してもいいかも
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
			_feed_set_feed_payout_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_feed_payout_retry(FEED_SLIP);
		}
	}
	else if(!(SENSOR_CENTERING) && 
			(SENSOR_VALIDATION_OFF) &&
			!(SENSOR_APB_IN) && 
			!(SENSOR_APB_OUT))
	{
		// 紙幣がBOX内に完全に入る前には、正転動作は停止
		motor_ctrl_feed_stop();

		if( (SENSOR_ENTRANCE) && 
			(SENSOR_CENTERING) && 
			!(SENSOR_VALIDATION_OFF) && 
			(SENSOR_APB_IN) && 
			(SENSOR_APB_OUT) )
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
		else
		{
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_FEED_REJECT_RETRY, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag))
	{
		// 規定パルス正転完了
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_FEED_REJECT_RETRY, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

static void _feed_set_feed_payout_retry(u32 alarm_code) // only 0x13XX
{

	ex_reject_escrow = 0;

	switch (alarm_code)
	{
	case FEED_OTHER_SENSOR:
		alarm_code = ALARM_CODE_FEED_OTHER_SENSOR_AT;
		break;
	case FEED_SLIP:
		alarm_code = ALARM_CODE_FEED_SLIP_AT;
		break;
	case FEED_TIMEOUT:
		alarm_code = ALARM_CODE_FEED_TIMEOUT_AT;
		break;
	case FEED_MOTOR_LOCK:
		alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		break;
	default:
		alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		break;
	}

	s_feed_alarm_retry++;
	if (s_feed_alarm_retry > FEED_REJECT_RETRY_COUNT)
	{
		/* retry over */
		_feed_set_alarm(alarm_code);
	}
	else
	{
		if (SENSOR_CENTERING_HOME)
		{
			s_feed_alarm_code = alarm_code;
			motor_ctrl_feed_stop();
			_feed_set_seq(0x1312, FEED_SEQ_TIMEOUT);
		}
		else
		{
			/*  Home out */
			s_feed_alarm_code = alarm_code;
			motor_ctrl_feed_stop();
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_REJECT_RSP, TMSG_SUB_INTERIM, 0, 0, 0); /* ??? */
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
	}
}
/*  PAYOUT STACK  */

void _feed_rc_force_stack_proc(u32 flag) //ok
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:
		_feed_rc_fstack_1700_seq(flag);
		break;
	case 0x01:
		_feed_rc_fstack_1701_seq(flag);
		break;
	case 0x02:
		_feed_rc_fstack_1702_seq(flag);
		break;
	case 0x03:
		_feed_rc_fstack_1703_seq(flag);
		break;
	default: /* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 19);
		break;
	}
}

static void _feed_rc_fstack_1700_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		_feed_rc_set_cancel();
	}
	else if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
	{
		_feed_set_seq(0x1701, 2000);
	}
}

static void _feed_rc_fstack_1701_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_rc_set_cancel();
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE )
	{
		_feed_rc_set_cancel();
	}
	
	else if((SENSOR_EXIT) || (!(SENSOR_VALIDATION_OFF)) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{

	}
	else if( is_feed_quad_model()                                    &&
	         ( (ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0) &&
	         ( (ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) == 0) )
	{
		_feed_set_seq(0x1702, 400);
		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE(40/PITCH)));
	}
	else if( (!(is_feed_quad_model())) &&
	         ( ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0) ) )
	{
		_feed_set_seq(0x1702, 400);
		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE(40/PITCH)));
	}
}

static void _feed_rc_fstack_1702_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		_feed_rc_set_cancel();
	}
	else if (is_feed_quad_model() &&
			 (((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) || ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0)))
	{
		_feed_set_seq(0x1701, 2000);
	}
	else if ((!(is_feed_quad_model())) &&
			 (((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0)))
	{
		_feed_set_seq(0x1701, 2000);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x1703, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_fstack_1703_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_FORCE_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/*COLLECT */
void _feed_rc_collect_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	/* Common */
	case 0x00:
		_feed_rc_collect_1400_seq(flag);
		break;
	case 0x01:
		_feed_rc_collect_1401_seq(flag);
		break;
	case 0x02:
		_feed_rc_collect_1402_seq(flag);
		break;
	case 0x03:
		_feed_rc_collect_1403_seq(flag);
		break;
	case 0x04:
		_feed_rc_collect_1404_seq(flag);
		break;
	case 0x05:
		_feed_rc_collect_1405_seq(flag);
		break;
	case 0x11:
		_feed_rc_collect_1411_seq(flag);
		break;
	case 0x12:
		_feed_rc_collect_1412_seq(flag);
		break;
	case 0x13:
		_feed_rc_collect_1413_seq(flag);
		break;
	case 0x14:
		_feed_rc_collect_1414_seq(flag);
		break;
	case 0x15:
		_feed_rc_collect_1415_seq(flag);
		break;
	default: /* other	*/
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 31);
		break;
	}
}

	
static void _feed_rc_collect_1400_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) ||
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else
	{
		if(!(is_feed_quad_model()))
		{
			/* RC-Twin model  */
			_feed_set_seq(0x1401, FEED_RC_SEQ_TIMEOUT);
		}
		else
		{
			/* RC-Quad model  */
			_feed_set_seq(0x1411, FEED_RC_SEQ_TIMEOUT);
		}
	}
}

static void _feed_rc_collect_1401_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) ||
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		/* jam flag reset */
		ex_rc_internal_jam_flag = 0;
		ex_rc_internal_jam_flag_bk = 0;
		/* recovery step collect */
		set_recovery_step(RECOVERY_STEP_COLLECT_TRANSPORT);
		_feed_set_seq(0x1402, FEED_RC_SEQ_TIMEOUT);
	}
	else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		/* jam flag reset */
		ex_rc_internal_jam_flag = 0;
		ex_rc_internal_jam_flag_bk = 0;
		/* recovery step collect */
		set_recovery_step(RECOVERY_STEP_COLLECT_TRANSPORT);
		_feed_set_seq(0x1402, FEED_RC_SEQ_TIMEOUT);
	}
}

static void _feed_rc_collect_1402_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) ||
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x1403, 30);
	}
	else if(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x1403, 30);
	}
}

static void _feed_rc_collect_1403_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) || 
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x1402, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x1402, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
		{
			if(RC_POS3_ON)
			{
				_feed_set_seq(0x1405, FEED_RC_SEQ_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1404, FEED_RC_SEQ_TIMEOUT);
			}
		}
		else if(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
		{
			if(RC_POS3_ON)
			{
				_feed_set_seq(0x1405, FEED_RC_SEQ_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1404, FEED_RC_SEQ_TIMEOUT);
			}
		}
	}
}

static void _feed_rc_collect_1404_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) || 
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS3_ON)
	{
		_feed_set_seq(0x1405, FEED_RC_SEQ_TIMEOUT);
	}
}

static void _feed_rc_collect_1405_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) || 
		 (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(!(RC_POS3_ON) && !(is_rc_busy()))
	{
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

static void _feed_rc_collect_1411_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) || 
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		ex_rc_internal_jam_flag = 0;
		ex_rc_internal_jam_flag_bk = 0;
		set_recovery_step(RECOVERY_STEP_COLLECT_TRANSPORT);
		_feed_set_seq(0x1412, FEED_RC_SEQ_TIMEOUT);
	}
	else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		ex_rc_internal_jam_flag = 0;
		ex_rc_internal_jam_flag_bk = 0;
		set_recovery_step(RECOVERY_STEP_COLLECT_TRANSPORT);
		_feed_set_seq(0x1412, FEED_RC_SEQ_TIMEOUT);
	}
	else if((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
	{
		ex_rc_internal_jam_flag = 0;
		ex_rc_internal_jam_flag_bk = 0;
		set_recovery_step(RECOVERY_STEP_COLLECT_TRANSPORT);
		_feed_set_seq(0x1412, FEED_RC_SEQ_TIMEOUT);
	}
	else if((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
	{
		ex_rc_internal_jam_flag = 0;
		ex_rc_internal_jam_flag_bk = 0;
		set_recovery_step(RECOVERY_STEP_COLLECT_TRANSPORT);
		_feed_set_seq(0x1412, FEED_RC_SEQ_TIMEOUT);
	}
}

static void _feed_rc_collect_1412_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) ||  
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x1413, 30);
	}
	else if(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x1413, 30);
	}
	else if(!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
	{
		_feed_set_seq(0x1413, 30);
	}
	else if(!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
	{
		_feed_set_seq(0x1413, 30);
	}
}

static void _feed_rc_collect_1413_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) ||  
		(SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x1412, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x1412, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
	{
		_feed_set_seq(0x1412, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
	{
		_feed_set_seq(0x1412, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
		{
			if(RC_POS6_ON)
			{
				_feed_set_seq(0x1415, FEED_RC_SEQ_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1414, FEED_RC_SEQ_TIMEOUT);
			}
		}
		else if(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
		{
			if(RC_POS6_ON)
			{
				_feed_set_seq(0x1415, FEED_RC_SEQ_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1414, FEED_RC_SEQ_TIMEOUT);
			}
		}
		else if(!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
		{
			if(RC_POS6_ON)
			{
				_feed_set_seq(0x1415, FEED_RC_SEQ_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1414, FEED_RC_SEQ_TIMEOUT);
			}
		}
		else if(!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
		{
			if(RC_POS6_ON)
			{
				_feed_set_seq(0x1415, FEED_RC_SEQ_TIMEOUT);
			}
			else
			{
				_feed_set_seq(0x1414, FEED_RC_SEQ_TIMEOUT);
			}
		}
	}
}	

static void _feed_rc_collect_1414_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) ||  
		 (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(RC_POS6_ON)
	{
		_feed_set_seq(0x1415, FEED_RC_SEQ_TIMEOUT);
	}
}

static void _feed_rc_collect_1415_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((ex_rc_status.sst21A.byte & RC1_POS1_POS2) != 0 || (!(SENSOR_VALIDATION_OFF)) ||   
		 (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(!(RC_POS6_ON) && !(is_rc_busy()))
	{
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/* STACK FORCE PAYOYT */
void _feed_rc_fpayout_seq_proc(u32 flag) //ok //payout以外でも使用している。Payoutというより、ドラムから背面搬送まで戻す処理、 mode_init, mode_payout,mode_stack
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq3300 */
		_feed_rc_fpayout_1600_seq(flag);
		break;
	case 0x01:									/* seq3301 */
		_feed_rc_fpayout_1601_seq(flag);
		break;
	case 0x02:									/* seq3302 */
		_feed_rc_fpayout_1602_seq(flag);
		break;
	case 0x03:									/* seq3303 */
		_feed_rc_fpayout_1603_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 18);
		break;
	}
}

static void _feed_rc_fpayout_1600_seq(u32 flag) //payout以外でも使用している。Payoutというより、ドラムから背面搬送まで戻す処理、 mode_init, mode_payout,mode_stack
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		_feed_rc_set_cancel();
	}
	else if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, 0))
	{
		_feed_set_seq(0x1601, 1500);
	}
}

static void _feed_rc_fpayout_1601_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_rc_set_cancel();
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		_feed_rc_set_cancel();
	}
	else if((RC_POS1_ON) || (SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_POS1);

		_feed_set_seq(0x1602, 500);
	}
}

static void _feed_rc_fpayout_1602_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_rc_set_cancel();
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		_feed_rc_set_cancel();
	}
	else if((!RC_POS1_ON) && (!RC_POS2_ON))
	{
		motor_ctrl_feed_stop();

		_feed_set_seq(0x1603, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_fpayout_1603_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_FORCE_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/* SEARCH BILL */
void _feed_rc_search_bill_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0A00 */
		_feed_rc_search_bill_0a00_seq(flag);
		break;
	case 0x01:									/* seq0A01 */
		_feed_rc_search_bill_0a01_seq(flag);
		break;
	case 0x02:									/* seq0A02 */
		_feed_rc_search_bill_0a02_seq(flag);	//same UBA500 not use UBA500
		break;
	case 0x03:									/* seq0A03 */
		_feed_rc_search_bill_0a03_seq(flag);	//same UBA500 not use UBA500
		break;
	case 0x04:									/* seq0A04 */
		_feed_rc_search_bill_0a04_seq(flag);	//same UBA500 not use UBA500	
		break;
	case 0x05:									/* seq0A05 */
		_feed_rc_search_bill_0a05_seq(flag);	//same UBA500 not use UBA500
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 18);
		break;
	}
}

static void _feed_rc_search_bill_0a00_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if (is_motor_ctrl_feed_stop())
	{
	    if( (is_feed_quad_model() &&
	        ( (SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) ||
              (SENSOR_APB_OUT) || (SENSOR_EXIT) ||
	          ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) ||
	          ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0) ||
	          ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) ||
	          ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) ) )
	        ||
	        ((!(is_feed_quad_model())) &&
	        ( (SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) ||
              (SENSOR_APB_OUT) || (SENSOR_EXIT) ||
	          ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) ||
	          ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0) ) ) )
		{
			/* found bill */
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
		else
		{
			if( FEED_SEARCH_IN == s_feed_search_direction )
			{
				if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, 0))
				{
					motor_ctrl_feed_set_pulse( FEED_SEARCH_IN_PULSE );
					_feed_set_seq(0x0A01, FEED_SERCH_REV_TIMEOUT);
				}
			}
			else
			{
				if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, 0))
				{
					_feed_set_seq(0x0A01, FEED_SERCH_REV_TIMEOUT);
				}
			}
		}
	}
}

static void _feed_rc_search_bill_0a01_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if( (is_feed_quad_model() &&
	         ( (SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) ||
               (SENSOR_APB_OUT) || (SENSOR_EXIT) ||
	           ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) ||
	           ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0) ||
	           ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0) ||
	           ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) ) )
	        ||
	         ((!(is_feed_quad_model())) &&
	         ( (SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) ||
               (SENSOR_APB_OUT) || (SENSOR_EXIT) ||
	           ((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) != 0) ||
	           ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0) ) ) )
	{
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);

	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if( IS_FEED_EVT_TIMEOUT(flag) )
	{
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
	else if( (FEED_SEARCH_IN == s_feed_search_direction) &&
		 	 (IS_FEED_EVT_OVER_PULSE(flag))              )
	{
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

static void _feed_rc_search_bill_0a02_seq(u32 flag)
{

}

static void _feed_rc_search_bill_0a03_seq(u32 flag)
{

}

static void _feed_rc_search_bill_0a04_seq(u32 flag)
{

}

static void _feed_rc_search_bill_0a05_seq(u32 flag)
{

}

/* BILL BACK */
void _feed_rc_bill_back_seq_proc(u32 flag) //ok
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00: 								/* seq1800 */
		_feed_rc_billback_1800_seq(flag);
		break;
	case 0x01: 								/* seq1801 */
		_feed_rc_billback_1801_seq(flag);
		break;
	case 0x02: 								/* seq1802 */
		_feed_rc_billback_1802_seq(flag);
		break;
	default: 								/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);
		/* system error ? */
		_feed_system_error(0, 20);
		break;
	}
}

static void _feed_rc_billback_1800_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, 
												(u16)(CONV_PULSE(20/PITCH))))
	{
		_feed_set_seq(0x1801, 200);
	}
}

static void _feed_rc_billback_1801_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x1802, FEED_SEQ_TIMEOUT);
	}
}

static void _feed_rc_billback_1802_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_BILLBACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}


/*********************************************************************//**
 * @brief feed control interrupt procedure (search sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_rc_payout_stop_seq_proc(u32 flag) //ok	/* '21-10-11 */
{
	switch(ex_feed_task_seq & 0x00FF)
	{
	case	0x00:									/* seq1900 */
			_feed_rc_payout_stop_1900_seq(flag);
			break;
	case	0x01:									/* seq1901 */
			_feed_rc_payout_stop_1901_seq(flag);
			break;
	default:									/* other */
			_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

			/* system error ? */
			_feed_system_error(0, 20);
			break;
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x1900
 *  feed forward
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_payout_stop_1900_seq(u32 flag)
{
	motor_ctrl_feed_stop();
	_feed_set_seq(0x1901, FEED_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief feed control sequence 0x1901
 *  wait feed motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rc_payout_stop_1901_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error() && ex_rc_retry_flg != TRUE)
	{
		/* モータ停止して待機状態に戻る */
		//_feed_set_cancel();
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
	/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_STOP_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/****************  RS PAYOUT  ***************/
void _feed_rs_payout_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00: /* seq 2300 */
		_feed_rs_payout_2300_seq(flag);
		break;
	case 0x01: /* seq 2301 */
		_feed_rs_payout_2301_seq(flag);
		break;
	case 0x02: /* seq 2302 */
		_feed_rs_payout_2302_seq(flag);
		break;
	case 0x03: /* seq 2303 */
		_feed_rs_payout_2303_seq(flag);
		break;
	case 0x04: /* seq 2304 */
		_feed_rs_payout_2304_seq(flag);
		break;
	case 0x05: /* seq 2305 */
		_feed_rs_payout_2305_seq(flag);
		break;
	case 0x06: /* seq 2306 */
		_feed_rs_payout_2306_seq(flag);
		break;
	case 0x07: /* seq 2307 */
		_feed_rs_payout_2307_seq(flag);
		break;
	case 0x08: /* seq 2308 */
		_feed_rs_payout_2308_seq(flag);
		break;
	case 0x09: /* seq 2309 */
		_feed_rs_payout_2309_seq(flag);
		break;
	case 0x20: /* seq 2320 */
		_feed_rs_payout_2320_seq(flag);
		break;
	case 0x21: /* seq 2321 */
		_feed_rs_payout_2321_seq(flag);
		break;
	case 0x22: /* seq 2322 */
		_feed_rs_payout_2322_seq(flag);
		break;
	case 0x30: /* seq 2330 */
		_feed_rs_payout_2330_seq(flag);
		break;
	case 0x31: /* seq 2331 */
		_feed_rs_payout_2331_seq(flag);
		break;
	case 0x32: /* seq 2332 */
		_feed_rs_payout_2332_seq(flag);
		break;
	default: /* other	*/
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 16);
		break;
	}
}

static void _feed_rs_payout_2300_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else
	{
		if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
		else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
		{
			ex_rc_internal_jam_flag = 0;	/* '19-09-19 */
			ex_rc_internal_jam_flag_bk = 0;

			set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
			_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
		}
		else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
		{
			ex_rc_internal_jam_flag = 0;	/* '19-09-19 */
			ex_rc_internal_jam_flag_bk = 0;

			set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
			_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
		}
		else if((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
		{
			ex_rc_internal_jam_flag = 0;	/* '19-09-19 */
			ex_rc_internal_jam_flag_bk = 0;

			set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
			_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
		}
		else if((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
		{
			ex_rc_internal_jam_flag = 0;	/* '19-09-19 */
			ex_rc_internal_jam_flag_bk = 0;

			set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
			_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
		}
	}
}

static void _feed_rs_payout_2301_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x2302, 30);
	}
	else if(!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x2302, 30);
	}
	else if(!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
	{
		_feed_set_seq(0x2302, 30);
	}
	else if(!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
	{
		_feed_set_seq(0x2302, 30);
	}
}

static void _feed_rs_payout_2302_seq(u32 flag)
{
	if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if((RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
	{
		_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
	{
		_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
	{
		_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
	}
	else if((RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2))
	{
		_feed_set_seq(0x2301, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if((!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
		|| (!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
		|| (!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
		|| (!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2)))
		{
			_feed_set_seq(0x2303, FEED_SEQ_TIMEOUT);
		}
	}
}
/*********************************************************************//**
 * @brief feed control sequence 0x2303
 *  wait rc pos2 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_payout_2303_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RC_POS2_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
		_feed_set_seq(0x2304, FEED_SEQ_TIMEOUT);
	}
	else if(RC_POS1_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_POS1);
		_feed_set_seq(0x2305, FEED_SEQ_TIMEOUT);
	}
	else
	{
		if((!(RC_POSB_ON) && (s_feed_unit == RC_TWIN_DRUM1))
		|| (!(RC_POSC_ON) && (s_feed_unit == RC_TWIN_DRUM2))
		|| (!(RC_POSE_ON) && (s_feed_unit == RC_QUAD_DRUM1))
		|| (!(RC_POSF_ON) && (s_feed_unit == RC_QUAD_DRUM2)))
		{
			set_recovery_step(RECOVERY_STEP_PAYOUT_TRANSPORT);
		}
		else
		{
			set_recovery_step(RECOVERY_STEP_PAYOUT_DRUM);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2304
 *  wait rc pos1 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_payout_2304_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RC_POS1_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_POS1);
		_feed_set_seq(0x2305, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x1305
 *  wait RS POS1 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_payout_2305_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RS_POS1_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_RS_POS7);
		_feed_set_seq(0x2306, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2306
 *  wait RS POS2 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_payout_2306_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RS_POS2_ON)
	{
		set_recovery_step(RECOVERY_STEP_PAYOUT_RS_ESCROW);

		if(s_feed_payout_last == true)
		{
			_feed_set_seq(0x2307, FEED_SEQ_TIMEOUT);
		}
		else
		{
			// RE_SET_FAIL_SAFE();

			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence	0x1307
 *  wait RS POS3 sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_payout_2307_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(s_feed_payout_last == true)
		{
			_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_TIMEOUT_AT);
		}
		else
		{
			_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
		}
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		if(s_feed_payout_last == true)
		{
			_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_MOTOR_LOCK);
		}
		else
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RS_POS3_ON)
	{
		if(s_feed_payout_last == true)
		{
			motor_ctrl_feed_set_pulse((u16)(CONV_PULSE((RecycleSettingInfo.DenomiInfo[s_feed_unit - 1].BillInfo.Length)/PITCH) + CONV_PULSE(40/PITCH)));
		}
		else
		{
			motor_ctrl_feed_set_pulse((u16)(CONV_PULSE((RecycleSettingInfo.DenomiInfo[s_feed_unit - 1].BillInfo.Length)/PITCH)));
		}
		_feed_set_seq(0x2308, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2308
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2308_seq(u32 flag)
{
	if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		if(s_feed_payout_last == true)
		{
			_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_MOTOR_LOCK);
		}
		else
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
		}
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(s_feed_payout_last == true)
		{
			_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_TIMEOUT_AT);
		}
		else
		{
			_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
		}
	}
	else if( (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(!(RS_POS3_ON) && IS_FEED_EVT_OVER_PULSE(flag))
	{
		if(s_feed_payout_last == true)
		{
			motor_ctrl_feed_stop();
			_feed_set_seq(0x2309, FEED_SEQ_TIMEOUT);
		}
		else
		{
			// RE_SET_FAIL_SAFE();

			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2309
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2309_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		if(s_feed_payout_last == true)
		{
			_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_TIMEOUT_AT);
		}
		else
		{
			_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
		}
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_motor_ctrl_feed_stop())
	{
		// cheat_check(0);
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/*********************************************************************//**
 * @brief feed control sub function
 *  set payout forward
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
static void _feed_set_feed_rs_payout_fwd_retry(u32 alarm_code)
{
	s_feed_alarm_retry++;

	if(s_feed_alarm_retry > FEED_REJECT_RETRY_COUNT)
	{
		/* retry over */
		_feed_set_alarm(alarm_code);
	}
	else
	{
		s_feed_alarm_code = alarm_code;
		motor_ctrl_feed_stop();
		_feed_set_seq(0x2320, FEED_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x2320
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2320_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if( (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, (u16)(15/PITCH)))
		{
			_feed_set_seq(0x2321, FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2321
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2321_seq(u32 flag)
{
	if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x2322, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2322
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2322_seq(u32 flag)
{
	if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		if(RS_POS2_ON)
		{
			/* 正転で紙幣を戻したのにRS POS2の場合は、出金扱いで処理再開 */
			set_recovery_step(RECOVERY_STEP_PAYOUT_RS_ESCROW);

			if(s_feed_payout_last == true)
			{
				_feed_set_seq(0x2307, FEED_SEQ_TIMEOUT);
				motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START);
			}
			else
			{
				motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START);

				_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RC_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				_feed_set_seq(FEED_SEQ_IDLE, 0);
			}
		}
		else if(RS_POS1_ON)
		{
			/* 正転で紙幣を戻したのにRS POS1の場合は、出金前で処理再開 */
			set_recovery_step(RECOVERY_STEP_PAYOUT_RS_POS7);

			_feed_set_seq(0x2306, FEED_SEQ_TIMEOUT);
			motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START);
		}
		else
		{
			/* 深部まで戻ってたら紙幣搬送からやり直し */
			set_recovery_step(RECOVERY_STEP_PAYOUT_POS1);

			_feed_set_seq(0x2305, FEED_SEQ_TIMEOUT);
			motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, 
				(u16)(CONV_PULSE((RecycleSettingInfo.DenomiInfo[s_feed_unit - 1].BillInfo.Length) / PITCH) + CONV_PULSE(40/PITCH)));
		}
	}
}

/*********************************************************************//**
 * @brief feed control sub function
 *  set payout retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
static void _feed_set_feed_rs_payout_rev_retry(u32 alarm_code)
{
	s_feed_alarm_retry++;

	if(s_feed_alarm_retry > FEED_REJECT_RETRY_COUNT)
	{
		/* retry over */
		_feed_set_alarm(alarm_code);
	}
	else
	{
		s_feed_alarm_code = alarm_code;
		motor_ctrl_feed_stop();
		_feed_set_seq(0x2330, FEED_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x2330
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2330_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_motor_ctrl_feed_stop())
	{
		if(IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, (u16)(15/PITCH)))
		{
			_feed_set_seq(0x2331, FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2331
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2331_seq(u32 flag)
{
	if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if( (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x2332, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2332
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_payout_2332_seq(u32 flag)
{
	if( (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_rev_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if(is_motor_ctrl_feed_stop())
	{
		_feed_set_seq(0x2307, FEED_SEQ_TIMEOUT);
		motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, 
			(u16)(CONV_PULSE((RecycleSettingInfo.DenomiInfo[s_feed_unit - 1].BillInfo.Length) / PITCH) + CONV_PULSE(40/PITCH)));
	}
}

void _feed_rs_fpayout_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00: /* seq2400 */
		_feed_rs_fpayout_2400_seq(flag);
		break;
	case 0x01: /* seq2401 */
		_feed_rs_fpayout_2401_seq(flag);
		break;
	case 0x02: /* seq2402 */
		_feed_rs_fpayout_2402_seq(flag);
		break;
	case 0x03: /* seq2403 */
		_feed_rs_fpayout_2403_seq(flag);
		break;
	case 0x04: /* seq2404 */
		_feed_rs_fpayout_2404_seq(flag);
		break;
	case 0x05: /* seq2405 */
		_feed_rs_fpayout_2405_seq(flag);
		break;
	case 0x20: /* seq2420 */
		_feed_rs_fpayout_2420_seq(flag);
		break;
	case 0x21: /* seq2421 */
		_feed_rs_fpayout_2421_seq(flag);
		break;
	case 0x22: /* seq2422 */
		_feed_rs_fpayout_2422_seq(flag);
		break;
	default: /* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 18);
		break;
	}
}

void _feed_rs_fpayout_2400_seq_start()
{
	if(!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		_feed_rc_set_cancel();
	}
	else if(IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START))
	{
		_feed_set_seq(0x2400, FEED_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x2400
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_fpayout_2400_seq(u32 flag)// use
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, 0))
	{
		if(RS_POS3_ON)
		{
			/* RS POS3 OFF待ちから開始 */
			_feed_set_seq(0x2403, FEED_SEQ_TIMEOUT);
		}
		else if(RS_POS2_ON)
		{
			/* RS POS3 ON待ちから開始 */
			_feed_set_seq(0x2402, FEED_SEQ_TIMEOUT);
		}
		else
		{
			/* RS POS2 ON待ちから開始 */
			_feed_set_seq(0x2401, FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2401
 *  feed revers
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_fpayout_2401_seq(u32 flag)// use
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RS_POS2_ON)
	{
		_feed_set_seq(0x2402, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2402
 *  feed revers
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_fpayout_2402_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RS_POS3_ON)
	{
//		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE((RecycleSettingInfo.DenomiInfo[s_feed_unit - 1].BillInfo.Length)/FPITCH) + CONV_PULSE(40/FPITCH)));
		_feed_set_seq(0x2403, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2403
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_fpayout_2403_seq(u32 flag)
{
	if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(!(RS_POS3_ON))
	{
		motor_ctrl_feed_set_pulse((u16)(CONV_PULSE(40/PITCH)));
		_feed_set_seq(0x2404, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2403
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
static void _feed_rs_fpayout_2404_seq(u32 flag)
{
	if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(RS_POS3_ON)
	{
		_feed_set_seq(0x2404, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x2405, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2404
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_fpayout_2405_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_RS_FORCE_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}


/*********************************************************************//**
 * @brief feed control sub function
 *  set payout retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
static void _feed_set_feed_rs_fpayout_fwd_retry(u32 alarm_code)
{
	s_feed_alarm_retry++;

	if(s_feed_alarm_retry > FEED_REJECT_RETRY_COUNT)
	{
		/* retry over */
		_feed_set_alarm(alarm_code);
	}
	else
	{
		s_feed_alarm_code = alarm_code;
		motor_ctrl_feed_stop();
		_feed_set_seq(0x2420, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2420
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_fpayout_2420_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		if(IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, (u16)(CONV_PULSE(15/PITCH))))
		{
			_feed_set_seq(0x2421, FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2421
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_fpayout_2421_seq(u32 flag)
{
	if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_payout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x2422, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x2422
 *  feed to Entrance sensor ON or All sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
static void _feed_rs_fpayout_2422_seq(u32 flag)
{
	if((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		// 意図しないセンサがONした
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_rs_fpayout_fwd_retry(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_feed_rc_set_cancel();
	}
	else if(is_motor_ctrl_feed_stop())
	{
		if(RS_POS3_ON)
		{
			_feed_set_seq(0x2403, FEED_SEQ_TIMEOUT);
			motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START);
		}
		else if(RS_POS2_ON)
		{
			_feed_set_seq(0x2402, FEED_SEQ_TIMEOUT);
			motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START);
		}
		else if(RS_POS1_ON)
		{
			_feed_set_seq(0x2401, FEED_SEQ_TIMEOUT);
			motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_HUNGING_POSITION_START);
		}
		else
		{
			_feed_set_seq(0x2403, FEED_SEQ_TIMEOUT);
			motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, (u16)(CONV_PULSE(40/PITCH)));
		}
	}
}

#endif // UBA_RTQ
