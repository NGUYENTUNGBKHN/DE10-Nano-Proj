/******************************************************************************/
/*! @addtogroup Main
    @file       apb_task.c
    @brief      apb task function
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "sub_functions.h"
#include "motor_ctrl.h"

#include "systemdef.h"					//2022-02-18 test
#include "cyclonev_sysmgr_reg_def.h"	//2022-02-18 test
#include "hal_gpio_reg.h"				//2022-02-18 test


#define EXT
#include "com_ram.c"

#include "jdl_conf.h"


/* UBAPROでは別ファイルで定義していた*/
static u32 s_apb_rotate_count;			//use PBを回転させる回数
static u32 s_apb_rotate_home_count;

/*
PB 1回転 25パルス
PB起動は100%
20パルス経過後に50％にする
Home検出後にモータ停止
*/



/************************** PRIVATE DEFINITIONS *************************/
enum _APB_SEQ
{
	APB_SEQ_IDLE		= 0x3000,
	APB_SEQ_FORCE_QUIT	= 0x3001,
	APB_SEQ_HOME		= 0x3100,
	APB_SEQ_EXEC		= 0x3200,
	APB_SEQ_INITIAL		= 0x3300,
	APB_SEQ_CLOSE		= 0x3400,
	APB_SEQ_FREERUN		= 0x3F00,

};

#define APB_SEQ_TIMEOUT			2000	/* 2sec */
#define APB_FREERUN_CHECK_TIME	1000	/* 1sec */
#define APB_RETRY_COUNT			5
#define APB_PULSE_COUNT			15		// 15パルス後に搬送スピードを変更する

#define APB_PULSE_COUNT_CLOSE	18		// 5パルス後にcloseの為、停止 8NG, 14NG, 10NG, 12NG メカ要望で18puls

#define APB_PWM_100				100		//
#define APB_PWM_81				80		// 15パルス後にPWM81%
#define APB_PWM_60				60		//

//静電気評価でPB Home誤検知が頻発してエラーになる為、割込み検知を配置、ポート検知にする
//#define IS_APB_EVT_HOME_INTR(x)			((x & EVT_APB_HOME_INTR) == EVT_APB_HOME_INTR)
//#define IS_APB_EVT_MOTOR_STOP(x)		((x & EVT_APB_MOTOR_STOP) == EVT_APB_MOTOR_STOP)
//#define IS_APB_EVT_SENSOR(x)			((x & EVT_APB_SENSOR) == EVT_APB_SENSOR)
#define IS_APB_EVT_TIMEOUT(x)			((x & EVT_APB_TIMEOUT) == EVT_APB_TIMEOUT)
#define IS_APB_EVT_REV_TIME(x)			((x & EVT_APB_REV_TIME) == EVT_APB_REV_TIME)
#define IS_APB_EVT_OVER_PULSE(x)		((x & EVT_APB_OVER_PULSE) == EVT_APB_OVER_PULSE)	// 追加

/************************** PRIVATE VARIABLES *************************/
static T_MSG_BASIC apb_msg;
static u16 s_apb_task_wait_seq;
static u8 s_apb_alarm_code;
static u8 s_apb_alarm_retry = 0;
static u8 s_apb_freerun_dir;

/************************** PRIVATE FUNCTIONS *************************/
void _apb_initialize_proc(void);
void _apb_idle_msg_proc(void);
void _apb_idel_proc(void);
void _apb_busy_msg_proc(void);
void _apb_busy_proc(void);

/* APB idle sequence */
void _apb_idle_seq_proc(u32 flag);
void _apb_idle_3001_seq(u32 flag);

/* APB home sequence */
void _apb_home_seq_proc(u32 flag);
void _apb_home_3100_seq_start(void);
void _apb_home_3100_seq(u32 flag);
void _apb_home_3101_seq(u32 flag);
void _apb_home_3102_seq(u32 flag);
void _apb_home_3104_seq(u32 flag);
void _apb_home_3106_seq(u32 flag);
void _apb_set_home_retry(u32 alarm_code);

/* APB exec sequence */// PB CLOSE PB OPEN共通
void _apb_exec_seq_proc(u32 flag);
void _apb_exec_3200_seq_start(void);
void _apb_exec_3200_seq(u32 flag);
void _apb_exec_3201_seq(u32 flag);
void _apb_exec_3202_seq(u32 flag);
void _apb_exec_3204_seq(u32 flag);
void _apb_exec_3206_seq(u32 flag);
void _apb_exec_3208_seq(u32 flag);// wait rev timeout
void _apb_set_exec_retry(u32 alarm_code);

/* APB motor free run sequence */
void _apb_freerun_seq_proc(u32 flag);
void _apb_freerun_3f00_seq(u32 flag);
void _apb_freerun_3f01_seq(u32 flag);

/* APB initialize sequence */
void _apb_initial_seq_proc(u32 flag);
void _apb_initial_3300_seq_start(void);
void _apb_initial_3300_seq(u32 flag);
void _apb_initial_3301_seq(u32 flag);
void _apb_initial_3302_seq(u32 flag);
void _apb_initial_3304_seq(u32 flag);
void _apb_initial_3306_seq(u32 flag);
void _apb_set_initial_retry(u32 alarm_code);


void _apb_close_seq_proc(u32 flag);
void _apb_close_3400_seq_start(void);
void _apb_close_3400_seq(u32 flag);
void _apb_close_3401_seq(u32 flag);
void _apb_close_3402_seq(u32 flag);
void _apb_close_3406_seq(u32 flag);
void _apb_close_3408_seq(u32 flag);// wait rev timeout
void _apb_set_close_retry(u32 alarm_code);

/* APB sub functions */
void _apb_set_seq(u16 seq, u16 time_out);
void _apb_set_waiting_seq(u32 seq);

void _apb_set_alarm(u32 alarm_code);
void _apb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _apb_system_error(u8 fatal_err, u8 code);

/************************** EXTERN VARIABLES *************************/

/************************** EXTERN FUNCTIONS *************************/

/*********************************************************************//**
 * @ APB task
 * @param[in]	extension information
 * @return 		None
 **********************************************************************/
void apb_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_apb_initialize_proc();

	while(1)
	{
		if (ex_apb_task_seq == APB_SEQ_IDLE)
		{
		/* idle */
			ercd = trcv_mbx(ID_APB_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
			if (ercd == E_OK)
			{
				memcpy(&apb_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(apb_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_apb_system_error(1, 1);
				}
				_apb_idel_proc();
				_apb_idle_msg_proc();

			}
			else
			{
				_apb_idel_proc();
			}
		}
		else
		{
		/* busy */
			_apb_busy_proc();
			ercd = prcv_mbx(ID_APB_MBX, (T_MSG **)&tmsg_pt);
			if (ercd == E_OK)
			{
				memcpy(&apb_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(apb_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_apb_system_error(1, 2);
				}
				_apb_busy_msg_proc();
			}
		}
	}
}


/*********************************************************************//**
 * @brief initialize APB task
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _apb_initialize_proc(void)
{
	ex_apb_task_seq = APB_SEQ_IDLE;
	s_apb_task_wait_seq	= APB_SEQ_IDLE;

	clr_flg(ID_APB_CTRL_FLAG, ~EVT_ALL_BIT);
}

/*********************************************************************//**
 * @brief APB task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _apb_idel_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;

	ercd = pol_flg(ID_APB_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag);
	if (ercd != E_OK)
	{
		flag = 0;
	}
}

/*********************************************************************//**
 * @brief MBX message procedure
 *  apb task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _apb_idle_msg_proc(void)
{

	switch (apb_msg.tmsg_code)
	{
	case TMSG_APB_HOME_REQ:					/* HOME message *//* use */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;
		_apb_set_seq(APB_SEQ_HOME, APB_SEQ_TIMEOUT);
		_apb_home_3100_seq_start();
		break;
	case TMSG_APB_EXEC_REQ:					/* EXEC message *//* 回転してHome */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;					//リトライカウンタ
		s_apb_rotate_count = apb_msg.arg1;		// PBの回転数
		if( s_apb_rotate_count == 0)			// リクエストのPBの回転数
		{
			s_apb_rotate_count = 1;
		}
		_apb_set_seq(APB_SEQ_EXEC, APB_SEQ_TIMEOUT);
		_apb_exec_3200_seq_start();
		break;

	case TMSG_APB_CLOSE_REQ:					/* CLOSE message *//* 2箇所のみ使用 */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;
		s_apb_rotate_count = apb_msg.arg1;		// PBの回転数
		if( s_apb_rotate_count == 0)			// リクエストのPBの回転数
		{
			s_apb_rotate_count = 1;
		}

		_apb_set_seq(APB_SEQ_CLOSE, APB_SEQ_TIMEOUT);
		_apb_close_3400_seq_start();
		break;
	case TMSG_APB_INITIAL_REQ:					/* INITIAL message */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;
		_apb_set_seq(APB_SEQ_INITIAL, APB_SEQ_TIMEOUT);
		_apb_initial_3300_seq_start();
		break;
	case TMSG_APB_FREERUN_REQ:				/* FREERUN message */
		s_apb_alarm_code = 0;
		s_apb_freerun_dir = apb_msg.arg1;
		_apb_set_seq(APB_SEQ_FREERUN, APB_SEQ_TIMEOUT);
		break;
	default:					/* other */
		/* system error ? */
		_apb_system_error(0, 5);
		break;
	}
}


/*********************************************************************//**
 * @brief apb task busy procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _apb_busy_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;

	ercd = twai_flg(ID_APB_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TASK_WAIT_TIME);
	if (ercd != E_OK)
	{
		flag = 0;
	}

	switch (ex_apb_task_seq & 0xFF00)
	{
	case APB_SEQ_IDLE:
		_apb_idle_seq_proc(flag);
		break;
	case APB_SEQ_HOME:	//
		_apb_home_seq_proc(flag);
		break;
	case APB_SEQ_EXEC:	// テストモードでも使用している
		_apb_exec_seq_proc(flag);
		break;
	case APB_SEQ_INITIAL:	//
		_apb_initial_seq_proc(flag);
		break;

	case APB_SEQ_CLOSE:	//
		_apb_close_seq_proc(flag);
		break;


	case APB_SEQ_FREERUN:
		_apb_freerun_seq_proc(flag);
		break;
	default:
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 6);
		break;
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  apb task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _apb_busy_msg_proc(void)
{

	switch (apb_msg.tmsg_code)
	{
	case TMSG_APB_HOME_REQ:					/* HOME message */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;
		_apb_set_waiting_seq(APB_SEQ_HOME);
		break;
	case TMSG_APB_EXEC_REQ:					/* EXEC message *//* 回転してHome */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;
		s_apb_rotate_count = apb_msg.arg1;		// リクエストのPBの回転数
		if( s_apb_rotate_count == 0)			// リクエストのPBの回転数
		{
			s_apb_rotate_count = 1;
		}
		_apb_set_waiting_seq(APB_SEQ_EXEC);
		break;
	case TMSG_APB_CLOSE_REQ:					/* CLOSE message *//* 2箇所のみ使用 */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;
		_apb_set_waiting_seq(APB_SEQ_CLOSE);
		break;
	case TMSG_APB_INITIAL_REQ:					/* INITIAL message */
		s_apb_alarm_code = 0;
		s_apb_alarm_retry = 0;
		_apb_set_waiting_seq(APB_SEQ_INITIAL);
		break;
	case TMSG_APB_FREERUN_REQ:				/* FREERUN message */
		s_apb_alarm_code = 0;
		if (apb_msg.arg1 == MOTOR_STOP)
		{
			_apb_set_waiting_seq(APB_SEQ_FREERUN);
		}
		break;
	default:					/* other */
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 7);
		break;
	}
}


/*********************************************************************//**
 * idle
 **********************************************************************/
/*********************************************************************//**
 * @brief apb control interrupt procedure (idle sequence)
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_idle_seq_proc(u32 flag)
{
	switch (ex_apb_task_seq & 0x00FF)
	{
	case 0x01:
		_apb_idle_3001_seq(flag);
		break;
	default:
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 8);
		break;
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3001
 *  wait motor stop
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_idle_3001_seq(u32 flag)
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
		_apb_set_alarm(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (s_apb_task_wait_seq == APB_SEQ_IDLE)
		{
			_apb_set_seq(s_apb_task_wait_seq, 0);
		}
		else if (s_apb_task_wait_seq == APB_SEQ_FREERUN)
		{
			_apb_send_msg(ID_MAIN_MBX, TMSG_APB_FREERUN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_apb_set_seq(APB_SEQ_IDLE, 0);
		}
		else
		{
			_apb_set_seq(s_apb_task_wait_seq, APB_SEQ_TIMEOUT);
		}
		s_apb_task_wait_seq = APB_SEQ_IDLE;
	}
}


/*********************************************************************//**
 * apb home
 **********************************************************************/
/*********************************************************************//**
 * @brief apb control interrupt procedure (home sequence)
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_home_seq_proc(u32 flag)	// PB動作が確立できてからソフトを変更する
{
	switch (ex_apb_task_seq & 0x00FF)
	{
	case 0x00:									/* seq3100 */
		_apb_home_3100_seq(flag);
		break;
	case 0x01:									/* seq3101 */
		_apb_home_3101_seq(flag);
		break;
	case 0x02:									/* seq3102 */
		_apb_home_3102_seq(flag);
		break;

	case 0x04:
		_apb_home_3104_seq(flag);
		break;

	case 0x06:
		_apb_home_3106_seq(flag);
		break;


	default:
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3100
 *  wait motor start
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_home_3100_seq_start(void)
{

	u8 home_cnt;

	//home_cnt = 1;	/* Homeにするには、最初の1回転はPWM100% */

	if (is_motor_ctrl_apb_stop())
	{
		if (SENSOR_APB_HOME)
		{
			_apb_send_msg(ID_MAIN_MBX, TMSG_APB_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_apb_set_seq(APB_SEQ_IDLE, 0);
		}
		/* 2022-01-21 */
		/* メカからいきなり、PWMで1回転させなくて、停止でいいということなので */
		if (s_apb_alarm_retry == 0)
		{
			/* 初回 */
			_ir_apb_motor_ctrl.pwm = APB_PWM_81;
			if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(APB_PWM_81, home_cnt, APB_PULSE_COUNT))
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_HOME_RSP, TMSG_SUB_START, 0, 0, 0);
				_apb_set_seq(0x3104, APB_SEQ_TIMEOUT);
			}
		}
		else
		{
			#if 1 //2022-10-31a 調査の為廃止
			if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))
			{
				if (s_apb_alarm_retry == 0)
				{
					_apb_send_msg(ID_MAIN_MBX, TMSG_APB_HOME_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_apb_set_seq(0x3101, APB_SEQ_TIMEOUT);
			}
			#endif
		}
	}
}


void _apb_home_3100_seq(u32 flag)
{

	u8 home_cnt;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_home_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (SENSOR_APB_HOME)
		{
			_apb_send_msg(ID_MAIN_MBX, TMSG_APB_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_apb_set_seq(APB_SEQ_IDLE, 0);
		}

		if (s_apb_alarm_retry == 0)
		{
		/* メカからいきなり、PWMで1回転させなくて、停止でいいということなので *//* 2022-01-21 */
		/* 初回 */
			_ir_apb_motor_ctrl.pwm = APB_PWM_81;
			if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(APB_PWM_81, home_cnt, APB_PULSE_COUNT))
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_HOME_RSP, TMSG_SUB_START, 0, 0, 0);
				_apb_set_seq(0x3104, APB_SEQ_TIMEOUT);
			}
		}
		else
		{			
			if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))
			{
				if (s_apb_alarm_retry == 0)
				{
					_apb_send_msg(ID_MAIN_MBX, TMSG_APB_HOME_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_apb_set_seq(0x3101, APB_SEQ_TIMEOUT);
			}
		}
	}
}


void _apb_home_3101_seq(u32 flag)
{

	u8 home_cnt;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_home_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
	// 最終1回転開始
		if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
		{
			_apb_set_seq(0x3102, APB_SEQ_TIMEOUT);
		}
		else
		{
			_apb_set_home_retry(ALARM_CODE_APB_TIMEOUT);
		}
	}
}


void _apb_home_3102_seq(u32 flag) //モータ起動後のパルス待ち
{

	u8 home_cnt;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_home_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (IS_APB_EVT_OVER_PULSE(flag))
	{
	// 規定パルス回転したので、PWMを弱める
	// 現状、エンコーダ割り込みで直接変える 50%
	//	home_cnt = 1;
		_ir_apb_motor_ctrl.pwm = APB_PWM_81;
		motor_ctrl_apb_fwd( APB_PWM_81, home_cnt );		/* 81%ﾃﾞｭｰﾃｨ */
		_apb_set_seq(0x3104, APB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3101
 *  wait home interrupt
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_home_3104_seq(u32 flag)
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_home_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
		motor_ctrl_apb_stop();
		_apb_set_seq(0x3106, APB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3101
 *  wait motor stop
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_home_3106_seq(u32 flag)
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
		_apb_set_home_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
	/* no error */
		if (SENSOR_APB_HOME)
		{
			_apb_send_msg(ID_MAIN_MBX, TMSG_APB_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_apb_set_seq(APB_SEQ_IDLE, 0);
		}
		else
		{
			_apb_set_home_retry(ALARM_CODE_APB_HOME_STOP);
		}
	}
}

/*********************************************************************//**
 * @brief apb control sub function
 *  set home retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _apb_set_home_retry(u32 alarm_code)
{
	s_apb_alarm_retry++;
	if (s_apb_alarm_retry >= APB_RETRY_COUNT)
	{
	/* retry over */
		_apb_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_apb_stop()))
		{
			motor_ctrl_apb_stop();
		}
		_apb_set_seq(0x3100, APB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * apb exec
 **********************************************************************/
void _apb_exec_320A_seq(u32 flag)//複数回転用 Home out待ち
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (!(SENSOR_APB_HOME))
	{
		_apb_set_seq(0x3201, APB_SEQ_TIMEOUT);
	}
}

void _apb_exec_320B_seq(u32 flag)//1回転用 Home out待ち
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (!(SENSOR_APB_HOME))
	{
		_apb_set_seq(0x3202, APB_SEQ_TIMEOUT);
	}
	#if 0
	//ここは、Home Out待ちで、ここで通常Home Out後のHomeで停止させる為の規定パスル到達イベントが発生する事はない
	//本来はこの次のシーケンスでこのイベントが発生するはず。
	//ここでこのイベントは発生した場合、cheatとしてみてもいいが、リスクもあるのでいれない
	else if (IS_APB_EVT_OVER_PULSE(flag)) //2024-06-06
	{
		_apb_set_alarm(ALARM_CODE_CHEAT);
	}
	#endif
}

/*********************************************************************//**
 * @brief apb control interrupt procedure (exec sequence)
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_exec_seq_proc(u32 flag)
{
	switch (ex_apb_task_seq & 0x00FF)
	{
	case 0x00:									/* seq3200 */
		_apb_exec_3200_seq(flag);
		break;
	case 0x0A:
		_apb_exec_320A_seq(flag);				// 複数回転Home out待ち 
		break;

	case 0x0B:
		_apb_exec_320B_seq(flag);				// 1回転Home out待ち 
		break;
	// PB open処理
	case 0x01:
		_apb_exec_3201_seq(flag);
		break;

	case 0x02:
		_apb_exec_3202_seq(flag);				// 規定パルス待ち
		break;
	case 0x04:
		_apb_exec_3204_seq(flag);				// Home待ち
		break;

	case 0x06:
		_apb_exec_3206_seq(flag);
		break;

	case 0x08:
		_apb_exec_3208_seq(flag);				// Reverse 
		break;

	default:									/* other */
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 10);
		break;
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3200
 *  wait motor start
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_exec_3200_seq_start(void)
{

	u8 home_cnt=0;

	if (is_motor_ctrl_apb_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
		{
			if (s_apb_alarm_retry == 0)
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
			}

			if (SENSOR_APB_HOME)
			{
				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;

				if(s_apb_rotate_count == 1)
				{
					ex_pb_encoder_count = 0;
					_apb_set_seq(0x320B, APB_SEQ_TIMEOUT);
				}
				else
				{
					_apb_set_seq(0x320A, APB_SEQ_TIMEOUT);
				}
			}
			else
			{
				if(s_apb_rotate_count <= 1)
				{
					s_apb_rotate_count++;
				}
				_apb_set_seq(0x3201, APB_SEQ_TIMEOUT);
			}	
		}
	}
}

void _apb_exec_3200_seq(u32 flag)
{

	u8 home_cnt = 0;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
		{
			if (s_apb_alarm_retry == 0)
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
			}

			if (SENSOR_APB_HOME)
			{
				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;

				if(s_apb_rotate_count == 1)
				{
					ex_pb_encoder_count = 0;
					_apb_set_seq(0x320B, APB_SEQ_TIMEOUT);				
				}
				else
				{
					_apb_set_seq(0x320A, APB_SEQ_TIMEOUT);
				}
			}
			else
			{
				if(s_apb_rotate_count <= 1)
				{
					s_apb_rotate_count++;
				}
				_apb_set_seq(0x3201, APB_SEQ_TIMEOUT);
			}	
		}
	}
}

void _apb_exec_3201_seq(u32 flag) //複数回転用 Home待ち
{

	u8 home_cnt;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
		if( s_apb_rotate_count > s_apb_rotate_home_count + 1)
		{
			s_apb_rotate_home_count++;
		}
		if( s_apb_rotate_count <= s_apb_rotate_home_count + 1)
		{
		// 最終1回転開始
			if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
			{
				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;

				ex_pb_encoder_count = 0;
				_apb_set_seq(0x320B, APB_SEQ_TIMEOUT);
			}
			else
			{
				_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
			}
		}
		else
		{
			_apb_set_seq(0x320A, APB_SEQ_TIMEOUT);
		}
	}
}


void _apb_exec_3202_seq(u32 flag) //モータ起動後のパルス待ち
{

	u8 home_cnt;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (IS_APB_EVT_OVER_PULSE(flag))
	{
	// 規定パルス回転したので、PWMを弱める
	// 現状、エンコーダ割り込みで直接変える 50%
		//home_cnt = 1;

		if( s_apb_alarm_retry == 0 )
		{
			_ir_apb_motor_ctrl.pwm = APB_PWM_81;
			motor_ctrl_apb_fwd( APB_PWM_81, home_cnt );			/* 81%ﾃﾞｭｰﾃｨ */
		}
		else
		{
			_ir_apb_motor_ctrl.pwm = APB_PWM_60;
			motor_ctrl_apb_fwd( APB_PWM_60, home_cnt );		/* 60%ﾃﾞｭｰﾃｨ */
		}

		_apb_set_seq(0x3204, APB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief apb control sequence
 *  wait home interrupt
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_exec_3204_seq(u32 flag)
{

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
		motor_ctrl_apb_stop();

		if( ex_dline_testmode.test_start == 1 )
		{
			ex_apb_count++;
			ex_dline_testmode.time1 = ex_dline_testmode.time_count;	//2018-11-28 時間を保存
			ex_dline_testmode.test_start = 0;
		}
		//2024-06-06 ここで1回転のパスル数チェックした方がいい
		//Cheatの確認のタイミングがUBA500と異なる理由は、UBA700ではHome Outを厳密に見るシーケンスになった為Home outシーケンスを追加した
		//Home OutのタイミングとPWM変更用のエンコーダパスルイベント発生のタイミングが厳密になった為、UBA500の様な1回転完了後のパスル数を確認する必要がなくなった
		//また、シーケンス的にPWM変更の為のエンコーダイベントが必修のなっているため、PB Home->Home Out -> Homeとなっても動作完了させずにリトライを続ける
		//上記のように、Home Outのタイミングとエンコーダのタイミングが一致しないと、動作し続けて最終的にエラーとなるので
		//いままでの対策は必要ない、どうしてもいれるとしたら、ここにいれるぐらい
	#if 1	
		if(ex_pb_encoder_count < 10 || ex_pb_encoder_count > PB_ENCODER_LIMIT)
		{
			_apb_set_alarm(ALARM_CODE_CHEAT);
		}
		else
		{
			_apb_set_seq(0x3206, APB_SEQ_TIMEOUT);
		}
	#else
		_apb_set_seq(0x3206, APB_SEQ_TIMEOUT);
	#endif
	}
}


/*********************************************************************//**
 * @brief apb control sequence
 *  wait motor stop
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_exec_3206_seq(u32 flag) // PB OPEN処理 モータ停止待ち
{

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_exec_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (!(SENSOR_APB_HOME))
		{
			if( ex_main_test_no == TEST_APB )
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_EXEC_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				_apb_set_seq(APB_SEQ_IDLE, 0);
			}
			else
			{
				_apb_set_exec_retry(ALARM_CODE_APB_HOME_STOP);
			}
		}
		else
		{
		// PB Home 動作完了
			/* リトライの時は、逆転を入れる	*/
			if( s_apb_alarm_retry == 0 )
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_EXEC_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				_apb_set_seq(APB_SEQ_IDLE, 0);
			}
			else
			{
			// only retry case, run reverse
				_ir_apb_motor_ctrl.pwm = APB_PWM_100;
				motor_ctrl_apb_rev( APB_PWM_100, 0 );		/* 100%ﾃﾞｭｰﾃｨ */
				_apb_set_seq(0x3208, APB_SEQ_TIMEOUT);
			}
		}
	}
}


void _apb_exec_3208_seq(u32 flag)// wait reverse timeout
{
	if (IS_APB_EVT_REV_TIME(flag))
	{
		s_apb_alarm_retry = 0;
		motor_ctrl_apb_stop();
		_apb_set_seq(0x3206, APB_SEQ_TIMEOUT);
	}
	else if (IS_APB_EVT_TIMEOUT(flag))
	{
		s_apb_alarm_retry = 0;
		motor_ctrl_apb_stop();
		_apb_set_seq(0x3206, APB_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief apb control sub function
 *  set exec retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _apb_set_exec_retry(u32 alarm_code)
{
	s_apb_alarm_retry++;
	s_apb_rotate_home_count = 0;

	if (s_apb_alarm_retry >= APB_RETRY_COUNT)
	{
	/* retry over */
		_apb_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_apb_stop()))
		{
			motor_ctrl_apb_stop();
		}
		_apb_set_seq(0x3200, APB_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * apb init
 **********************************************************************/
/*********************************************************************//**
 * @brief apb control interrupt procedure (initialize sequence)
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_initial_330A_seq(u32 flag)//複数回転用 Home out待ち
{

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (!(SENSOR_APB_HOME))
	{
		_apb_set_seq(0x3301, APB_SEQ_TIMEOUT);
	}
}

void _apb_initial_330B_seq(u32 flag)//1回転用 Home out待ち
{

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (!(SENSOR_APB_HOME))
	{
		_apb_set_seq(0x3302, APB_SEQ_TIMEOUT);
	}
}


void _apb_initial_seq_proc(u32 flag)	// PB動作が確立できてからソフトを変更する
{
	switch (ex_apb_task_seq & 0x00FF)
	{
	case 0x00:									/* seq3300 */
		_apb_initial_3300_seq(flag);
		break;

	case 0x0A:
		_apb_initial_330A_seq(flag);
		break;

	case 0x01:									/* seq3301 */
		_apb_initial_3301_seq(flag);
		break;


	case 0x0B:
		_apb_initial_330B_seq(flag);
		break;

	case 0x02:									/* seq3302 */
		_apb_initial_3302_seq(flag);
		break;
	case 0x04:
		_apb_initial_3304_seq(flag);
		break;
	case 0x06:
		_apb_initial_3306_seq(flag);
		break;


	default:									/* other */
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 16);

		break;
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3300
 *  wait motor start
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_initial_3300_seq_start(void)
{

	u8 home_cnt=0;

	if (is_motor_ctrl_apb_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
		{
			if (s_apb_alarm_retry == 0)
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_INITIAL_RSP, TMSG_SUB_START, 0, 0, 0);
			}

			if (SENSOR_APB_HOME)
			{
				if(s_apb_rotate_count == 1)
				{
					_apb_set_seq(0x330B, APB_SEQ_TIMEOUT);
				}
				else
				{
					_apb_set_seq(0x330A, APB_SEQ_TIMEOUT);
				}
			}
			else
			{
				if(s_apb_rotate_count <= 1)
				{
					s_apb_rotate_count++;
				}
				_apb_set_seq(0x3301, APB_SEQ_TIMEOUT);
			}	
		}
	}
}


void _apb_initial_3300_seq(u32 flag)
{

	u8 home_cnt=0;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
		{
			if (s_apb_alarm_retry == 0)
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_INITIAL_RSP, TMSG_SUB_START, 0, 0, 0);
			}

			if (SENSOR_APB_HOME)
			{
				if(s_apb_rotate_count == 1)
				{
					_apb_set_seq(0x330B, APB_SEQ_TIMEOUT);
				}
				else
				{
					_apb_set_seq(0x330A, APB_SEQ_TIMEOUT);
				}
			}
			else
			{
				if(s_apb_rotate_count <= 1)
				{
					s_apb_rotate_count++;
				}
				_apb_set_seq(0x3301, APB_SEQ_TIMEOUT);
			}	
		}
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3301
 *  wait home interrupt
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_initial_3301_seq(u32 flag)//複数回転用 Home待ち
{
	u8 home_cnt = 0;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
		if( s_apb_rotate_count > s_apb_rotate_home_count + 1)
		{
			s_apb_rotate_home_count++;
		}
		if( s_apb_rotate_count <= s_apb_rotate_home_count + 1)
		{
		// 最終1回転開始
			if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
			{
				ex_dline_testmode.test_start = 1;


				ex_pb_encoder_count = 0;

				_apb_set_seq(0x3302, APB_SEQ_TIMEOUT);
			}
			else
			{
				_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
			}
		}
		else
		{
			_apb_set_seq(0x330A, APB_SEQ_TIMEOUT);
		}
	}
}

/*********************************************************************//**
 * @brief apb control sequence 0x3302
 *  wait motor stop
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_initial_3302_seq(u32 flag)
{
	u8 home_cnt;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (IS_APB_EVT_OVER_PULSE(flag))
	{
	// 規定パルス回転したので、PWMを弱める
	// 現状、エンコーダ割り込みで直接変える 
	//	home_cnt = 1;
		_ir_apb_motor_ctrl.pwm = APB_PWM_81;
		motor_ctrl_apb_fwd( APB_PWM_81, home_cnt );		/* 81%ﾃﾞｭｰﾃｨ */
		_apb_set_seq(0x3304, APB_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief apb control sequence 0x3304
 *  wait motor start
 * @param[in]	apb motor event flag
 * @return 		No
 **********************************************************************/
void _apb_initial_3304_seq(u32 flag)
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
		motor_ctrl_apb_stop();
		_apb_set_seq(0x3306, APB_SEQ_TIMEOUT);
	}
}


void _apb_initial_3306_seq(u32 flag)
{

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_initial_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (!(SENSOR_APB_HOME))
		{
			if( ex_main_test_no == TEST_APB )
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_INITIAL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				_apb_set_seq(APB_SEQ_IDLE, 0);
			}
			else
			{
				_apb_set_initial_retry(ALARM_CODE_APB_HOME_STOP);
			}
		}
		else
		{
		// PB Home 動作完了
			if( ex_main_test_no == TEST_APB )
			{
			// テスト動作はここで回り続ける
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_INITIAL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				_apb_set_seq(APB_SEQ_IDLE, 0);
			}
			else
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_INITIAL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				_apb_set_seq(APB_SEQ_IDLE, 0);
			}
		}
	}
}


/*********************************************************************//**
 * @brief apb control sub function
 *  set initial retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _apb_set_initial_retry(u32 alarm_code)
{
	s_apb_alarm_retry++;
	s_apb_rotate_home_count = 0;

	if (s_apb_alarm_retry >= APB_RETRY_COUNT)
	{
	/* retry over */
		_apb_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_apb_stop()))
		{
			motor_ctrl_apb_stop();
		}
		_apb_set_seq(0x3300, APB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * APB motor free run
 **********************************************************************/
/*********************************************************************//**
 * @brief apb control interrupt procedure (free run sequence)
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_freerun_seq_proc(u32 flag)
{
	switch (ex_apb_task_seq & 0x00FF)
	{
	case 0x00:									/* seq3F00 */
		_apb_freerun_3f00_seq(flag);
		break;
	case 0x01:									/* seq3F01 */
		_apb_freerun_3f01_seq(flag);
		break;
	default:									/* other */
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 11);
		break;
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3F00
 *  wait motor start
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_freerun_3f00_seq(u32 flag)
{
	if (s_apb_freerun_dir == MOTOR_FWD)
	{
		if (IERR_CODE_OK == motor_ctrl_apb_fwd(MOTOR_MAX_SPEED, 0))
		{
			_apb_set_seq(0x3f01, APB_FREERUN_CHECK_TIME);
		}
	}
	else if (s_apb_freerun_dir == MOTOR_REV)
	{
		if (IERR_CODE_OK == motor_ctrl_apb_rev(MOTOR_MAX_SPEED, 0))
		{
			_apb_set_seq(0x3f01, APB_FREERUN_CHECK_TIME);
		}
	}
	else
	{
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 12);
	}
}


/*********************************************************************//**
 * @brief apb control sequence 0x3F01
 *  free run
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_freerun_3f01_seq(u32 flag)
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
		if (_ir_apb_motor_ctrl.home_cnt == 0)
		{
			_apb_set_alarm(ALARM_CODE_APB_HOME);
		}
		else if (s_apb_freerun_dir == MOTOR_FWD)
		{
			if (IERR_CODE_OK == motor_ctrl_apb_fwd(MOTOR_MAX_SPEED, 0))
			{
				_apb_set_seq(0x3f01, APB_FREERUN_CHECK_TIME);
			}
			else
			{
				_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

				/* system error ? */
				_apb_system_error(0, 13);
			}
		}
		else if (s_apb_freerun_dir == MOTOR_REV)
		{
			if (IERR_CODE_OK == motor_ctrl_apb_rev(MOTOR_MAX_SPEED, 0))
			{
				_apb_set_seq(0x3f01, APB_FREERUN_CHECK_TIME);
			}
			else
			{
				_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

				/* system error ? */
				_apb_system_error(0, 14);
			}
		}
		else
		{
			_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

			/* system error ? */
			_apb_system_error(0, 15);
		}
	}
}


/*********************************************************************//**
 * @brief apb control sub function
 *  set apb sequence
 * @param[in]	sequence no.
 * 				time out
 * @return 		None
 **********************************************************************/
void _apb_set_seq(u16 seq, u16 time_out)
{
	ex_apb_task_seq = seq;
	clr_flg(ID_APB_CTRL_FLAG, ~EVT_ALL_BIT);

#if 1//#ifdef _ENABLE_JDL
	jdl_add_trace(ID_APB_TASK, ((ex_apb_task_seq >> 8) & 0xFF), (ex_apb_task_seq & 0xFF), s_apb_alarm_code, s_apb_alarm_retry, 0);
#endif /* _ENABLE_JDL */

}


/*********************************************************************//**
 * @brief apb control sub function
 *  set apb waiteing sequence
 * @param[in]	sequence no.
 * @return 		None
 **********************************************************************/
void _apb_set_waiting_seq(u32 seq)
{
	if (ex_apb_task_seq == APB_SEQ_FORCE_QUIT)
	{
		s_apb_task_wait_seq = seq;
	}
	else
	{
		s_apb_task_wait_seq = seq;

		motor_ctrl_apb_stop();
		_apb_set_seq(APB_SEQ_FORCE_QUIT, APB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief apb control sub function
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _apb_set_alarm(u32 alarm_code)
{
	u32 msg;
	u32 sensor;

	motor_ctrl_apb_stop();

	s_apb_alarm_code = alarm_code;

	switch (ex_apb_task_seq & 0xFF00)
	{
	case APB_SEQ_HOME:
		msg = TMSG_APB_HOME_RSP;
		break;
	case APB_SEQ_EXEC:
		msg = TMSG_APB_EXEC_RSP;
		break;
	case APB_SEQ_FREERUN:
		msg = TMSG_APB_FREERUN_RSP;
		break;
	case APB_SEQ_INITIAL:
		msg = TMSG_APB_INITIAL_RSP;
		break;
	case APB_SEQ_CLOSE:
		msg = TMSG_APB_CLOSE_RSP;
		break;	
	default:
		msg = TMSG_APB_STATUS_INFO;
		break;
	}

	sensor = ex_position_sensor;

	_apb_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_ALARM, s_apb_alarm_code, ex_apb_task_seq, sensor);
	_apb_set_seq(APB_SEQ_FORCE_QUIT, APB_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief send task message
 * @param[in]	receiver task id
 * 				task message code
 * 				argument 1
 * 				argument 2
 * 				argument 3
 * 				argument 4
 * @return 		None
 **********************************************************************/
void _apb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_APB_TASK;
		t_msg->mpf_id = ID_MBX_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_apb_system_error(1, 3);
		}
	}
	else
	{
		/* system error */
		_apb_system_error(1, 4);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _apb_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR

	//_apb_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_BLINK, 1, 0, 0, 0);

#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_apb_send_msg(ID_ERRDISP_MBX, TMSG_ERRDISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	// fatal_err == 1の場合、致命的なエラーなのでエラーコードは出力しない、LED表示のみ通知で無限待ち
	_debug_system_error(ID_APB_TASK, (u16)code, (u16)apb_msg.tmsg_code, (u16)apb_msg.arg1, fatal_err);
}

void _apb_close_340A_seq(u32 flag)//複数回転用 Home out待ち
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_close_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (!(SENSOR_APB_HOME))
	{
		/* 1度目のHomeなので無条件*/
		_apb_set_seq(0x3401, APB_SEQ_TIMEOUT);
	}
}

void _apb_close_340C_seq(u32 flag)  /* 動作開始時がHomeでないHome 待ち*/
{
	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_close_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
		/* 1度目のHomeなので無条件*//* 動作開始時がHomeと合流 */
		ex_pb_encoder_count = 0;
		_apb_set_seq(0x340A, APB_SEQ_TIMEOUT);
	}
}

/* PB close */
void _apb_close_seq_proc(u32 flag)
{
	switch (ex_apb_task_seq & 0x00FF)
	{
	case 0x00:									/* seq3200 */
		_apb_close_3400_seq(flag);
		break;
	case 0x0A:
		_apb_close_340A_seq(flag);  //複数回転用 Home out待ち
		break;

	case 0x0C:
		_apb_close_340C_seq(flag);	/* Home待ち */
		break;
	// PB open処理
	case 0x01:
		_apb_close_3401_seq(flag);
		break;

	case 0x02:
		_apb_close_3402_seq(flag);				// 規定パルス待ち
		break;

	case 0x06:
		_apb_close_3406_seq(flag);
		break;
	default:									/* other */
		_apb_set_alarm(ALARM_CODE_APB_FORCED_QUIT);

		/* system error ? */
		_apb_system_error(0, 10);
		break;
	}
}


/*********************************************************************//**
 * @brief apb control sequence
 *  wait motor start
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_close_3400_seq_start(void)
{

	u8 home_cnt=0;

	if (is_motor_ctrl_apb_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
		{
			if (s_apb_alarm_retry == 0)
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_CLOSE_RSP, TMSG_SUB_START, 0, 0, 0);
			}

			if (SENSOR_APB_HOME)
			{
				/* closeの場合は同じでもいいかも、*/
				if(s_apb_rotate_count == 1)
				{
					ex_pb_encoder_count = 0;
					_apb_set_seq(0x340A, APB_SEQ_TIMEOUT);//
				}
				else
				{
					ex_pb_encoder_count = 0;
					_apb_set_seq(0x340A, APB_SEQ_TIMEOUT);//
				}
			}
			else
			{
				if(s_apb_rotate_count <= 1)
				{
					s_apb_rotate_count++;
				}
				_apb_set_seq(0x340C, APB_SEQ_TIMEOUT); //位置がさだかでないので、設定より+1
			}	
		}
	}
}

void _apb_close_3400_seq(u32 flag)
{

	u8 home_cnt=0;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_close_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT))	// XXパルス後にイベント発生
		{
			if (s_apb_alarm_retry == 0)
			{
				_apb_send_msg(ID_MAIN_MBX, TMSG_APB_CLOSE_RSP, TMSG_SUB_START, 0, 0, 0);
			}

			if (SENSOR_APB_HOME)
			{
				/* closeの場合は同じでもいいかも、*/
				if(s_apb_rotate_count == 1)
				{
					ex_pb_encoder_count = 0;
					_apb_set_seq(0x340A, APB_SEQ_TIMEOUT);//
				}
				else
				{
					ex_pb_encoder_count = 0;
					_apb_set_seq(0x340A, APB_SEQ_TIMEOUT);//
				}
			}
			else
			{
				if(s_apb_rotate_count <= 1)
				{
					s_apb_rotate_count++;
				}
				_apb_set_seq(0x340C, APB_SEQ_TIMEOUT); //位置がさだかでないので、設定より+1
			}	
		}
	}
}

void _apb_close_3401_seq(u32 flag)
{

	u8 home_cnt = 0;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_close_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (SENSOR_APB_HOME)
	{
		if( s_apb_rotate_count > s_apb_rotate_home_count + 1)
		{
			s_apb_rotate_home_count++;
		}
		if( s_apb_rotate_count <= s_apb_rotate_home_count + 1)
		{

			//2024-06-07
			if(ex_pb_encoder_count < 10 || ex_pb_encoder_count > PB_ENCODER_LIMIT)
			{
				_apb_set_alarm(ALARM_CODE_CHEAT);
			}
			else
			{

			// 最終半回転開始
				if (IERR_CODE_OK == motor_ctrl_apb_fwd_puls(MOTOR_MAX_SPEED, home_cnt, APB_PULSE_COUNT_CLOSE))	// XXパルス後にイベント発生、停止
				{
					ex_pb_encoder_count = 0;
					/* 通常のPB Homeの処理の場合、この次のシーケンスで、規定パルス到達でPWMを変えるが */
					/* PB Closeの場合、規定パルスで、停止させてPB Closeにする */
					_apb_set_seq(0x3402, APB_SEQ_TIMEOUT);
				}
				else
				{
					_apb_set_close_retry(ALARM_CODE_APB_TIMEOUT);
				}
			}	
		}
		else
		{
			ex_pb_encoder_count = 0;
			_apb_set_seq(0x340A, APB_SEQ_TIMEOUT);			
		}
	}
}


void _apb_close_3402_seq(u32 flag) //Closeでの停止の為、パルス待ち
{

	u8 home_cnt;

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_close_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (IS_APB_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_apb_stop();
		_apb_set_seq(0x3406, APB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief apb control sequence
 *  wait motor stop
 * @param[in]	apb motor event flag
 * @return 		None
 **********************************************************************/
void _apb_close_3406_seq(u32 flag) // PB OPEN処理 モータ停止待ち
{

	if (IS_APB_EVT_TIMEOUT(flag))
	{
	/* time out */
		_apb_set_close_retry(ALARM_CODE_APB_TIMEOUT);
	}
	else if (is_motor_ctrl_apb_stop())
	{
		if (!(SENSOR_APB_HOME))
		{
			_apb_send_msg(ID_MAIN_MBX, TMSG_APB_CLOSE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_apb_set_seq(APB_SEQ_IDLE, 0);
		}
		else
		{
			_apb_set_close_retry(ALARM_CODE_APB_HOME_STOP);
		}
	}
}

void _apb_set_close_retry(u32 alarm_code)
{
	s_apb_alarm_retry++;
	s_apb_rotate_home_count = 0;

	if (s_apb_alarm_retry >= APB_RETRY_COUNT)
	{
	/* retry over */
		_apb_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_apb_stop()))
		{
			motor_ctrl_apb_stop();
		}
		_apb_set_seq(0x3400, APB_SEQ_TIMEOUT);
	}
}





/* EOF */
