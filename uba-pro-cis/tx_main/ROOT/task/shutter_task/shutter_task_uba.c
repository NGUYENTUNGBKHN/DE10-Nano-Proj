/******************************************************************************/
/*! @addtogroup Main
    @file       shutter_task.c
    @brief      shutter task function
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



/************************** PRIVATE DEFINITIONS *************************/
enum _SHUTTER_SEQ
{
	SHUTTER_SEQ_IDLE			= 0xA000,
	SHUTTER_SEQ_FORCE_QUIT		= 0xA001,
	SHUTTER_SEQ_INITIAL_OPEN	= 0xA100,
	SHUTTER_SEQ_OPEN			= 0xA200,
	SHUTTER_SEQ_CLOSE			= 0xA300,
	SHUTTER_SEQ_FREERUN			= 0xAA00,

};

#define SHUTTER_SEQ_TIMEOUT			2000	/* 2sec */
#define SHUTTER_FREERUN_CHECK_TIME	1000	/* 1sec */
#define SHUTTER_RETRY_COUNT			5

#define IS_SHUTTER_EVT_SENSOR(x)			((x & EVT_SHUTTER_SENSOR) == EVT_SHUTTER_SENSOR)// これがなくても上手く動いている
//#define IS_SHUTTER_EVT_MOTOR_STOP(x)		((x & EVT_SHUTTER_MOTOR_STOP) == EVT_SHUTTER_MOTOR_STOP)
#define IS_SHUTTER_EVT_TIMEOUT(x)			((x & EVT_SHUTTER_TIMEOUT) == EVT_SHUTTER_TIMEOUT)

/************************** PRIVATE VARIABLES *************************/
static T_MSG_BASIC shutter_msg;
static u16 s_shutter_task_wait_seq;
static u8 s_shutter_alarm_code;
static u8 s_shutter_alarm_retry = 0;
static u8 s_shutter_freerun_dir;

/************************** PRIVATE FUNCTIONS *************************/
void _shutter_initialize_proc(void);
void _shutter_idle_msg_proc(void);
void _shutter_idel_proc(void);
void _shutter_busy_msg_proc(void);
void _shutter_busy_proc(void);


/* SHUTTER initialize sequence */
void _shutter_initial_open_seq_proc(u32 flag);
void _shutter_initial_open_A100_seq_start(void);
void _shutter_initial_open_A100_seq(u32 flag);
void _shutter_initial_open_A104_seq(u32 flag);
void _shutter_initial_open_A106_seq(u32 flag);
void _shutter_initial_open_A108_seq(u32 flag);
void _shutter_set_initial_open_retry(u32 alarm_code);

/* SHUTTER open sequence */
void _shutter_open_seq_proc(u32 flag);
void _shutter_open_A200_seq_start(void);
void _shutter_open_A200_seq(u32 flag);
void _shutter_open_A204_seq(u32 flag);
void _shutter_open_A206_seq(u32 flag);
void _shutter_set_open_retry(u32 alarm_code);

/* SHUTTER close sequence */
void _shutter_close_seq_proc(u32 flag);
void _shutter_close_A300_seq_start(void);
void _shutter_close_A300_seq(u32 flag);
void _shutter_close_A304_seq(u32 flag);
void _shutter_close_A306_seq(u32 flag);
void _shutter_set_close_retry(u32 alarm_code);

/* SHUTTER free run sequence */ 
void _shutter_freerun_seq_proc(u32 flag);
void _shutter_freerun_AA00_seq(u32 flag);
void _shutter_freerun_AA04_seq(u32 flag);
void _shutter_freerun_AA06_seq(u32 flag);
void _shutter_set_freerun_retry(u32 alarm_code);

/* SHUTTER sub functions */
void _shutter_set_seq(u16 seq, u16 time_out);
void _shutter_set_waiting_seq(u32 seq);

void _shutter_set_alarm(u32 alarm_code);
void _shutter_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _shutter_system_error(u8 fatal_err, u8 code);

/************************** EXTERN VARIABLES *************************/

/************************** EXTERN FUNCTIONS *************************/


/*********************************************************************//**
 * @ SHUTTER task
 * @param[in]	extension information
 * @return 		None
 **********************************************************************/
void shutter_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_shutter_initialize_proc();

	while(1)
	{
		if (ex_shutter_task_seq == SHUTTER_SEQ_IDLE)
		{
		/* idle */
			ercd = trcv_mbx(ID_SHUTTER_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
			if (ercd == E_OK)
			{
				memcpy(&shutter_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(shutter_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_shutter_system_error(1, 1);
				}
				_shutter_idel_proc();
				_shutter_idle_msg_proc();

			}
			else
			{
				_shutter_idel_proc();
			}
		}
		else
		{
		/* busy */
			_shutter_busy_proc();
			ercd = prcv_mbx(ID_SHUTTER_MBX, (T_MSG **)&tmsg_pt);
			if (ercd == E_OK)
			{
				memcpy(&shutter_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(shutter_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_shutter_system_error(1, 2);
				}
				_shutter_busy_msg_proc();
			}
		}
	}
}


/*********************************************************************//**
 * @brief initialize SHUTTER task
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _shutter_initialize_proc(void)
{
	ex_shutter_task_seq = SHUTTER_SEQ_IDLE;
	s_shutter_task_wait_seq	= SHUTTER_SEQ_IDLE;

	clr_flg(ID_SHUTTER_CTRL_FLAG, ~EVT_ALL_BIT);
}

/*********************************************************************//**
 * @brief SHUTTER task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _shutter_idel_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;

	ercd = pol_flg(ID_SHUTTER_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag);
	if (ercd != E_OK)
	{
		flag = 0;
	}
}

/*********************************************************************//**
 * @brief MBX message procedure
 *  shutter task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _shutter_idle_msg_proc(void)
{

	switch (shutter_msg.tmsg_code)
	{
	case TMSG_SHUTTER_INITIAL_OPEN_REQ:					/* INITIAL message */
		s_shutter_alarm_code = 0;
		s_shutter_alarm_retry = 0;
		_shutter_set_seq(SHUTTER_SEQ_INITIAL_OPEN, SHUTTER_SEQ_TIMEOUT);
		_shutter_initial_open_A100_seq_start();
		break;

	case TMSG_SHUTTER_OPEN_REQ:
		s_shutter_alarm_code = 0;
		s_shutter_alarm_retry = 0;
		_shutter_set_seq(SHUTTER_SEQ_OPEN, SHUTTER_SEQ_TIMEOUT);
		_shutter_open_A200_seq_start();
		break;

	case TMSG_SHUTTER_CLOSE_REQ:					/* CLOSE message */
		s_shutter_alarm_code = 0;
		s_shutter_alarm_retry = 0;
		_shutter_set_seq(SHUTTER_SEQ_CLOSE, SHUTTER_SEQ_TIMEOUT);
		_shutter_close_A300_seq_start();
		break;

	case TMSG_SHUTTER_FREERUN_REQ:				/* FREERUN message */
		s_shutter_alarm_code = 0;
		s_shutter_freerun_dir = shutter_msg.arg1;
		_shutter_set_seq(SHUTTER_SEQ_FREERUN, SHUTTER_SEQ_TIMEOUT);
		break;
	default:					/* other */
		/* system error ? */
		_shutter_system_error(0, 5);
		break;
	}
}


void _shutter_idle_3001_seq(u32 flag)
{
	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
		_shutter_set_alarm(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (is_motor_ctrl_shutter_stop())
	{
		if (s_shutter_task_wait_seq == SHUTTER_SEQ_IDLE)
		{
			_shutter_set_seq(s_shutter_task_wait_seq, 0);
		}
		else if (s_shutter_task_wait_seq == SHUTTER_SEQ_FREERUN)
		{
			_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_FREERUN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
		}
		else
		{
			_shutter_set_seq(s_shutter_task_wait_seq, SHUTTER_SEQ_TIMEOUT);
		}
		s_shutter_task_wait_seq = SHUTTER_SEQ_IDLE;
	}
}

void _shutter_idle_seq_proc(u32 flag)
{
	switch (ex_shutter_task_seq & 0x00FF)
	{
	case 0x01:
		_shutter_idle_3001_seq(flag);
		break;
	default:
		_shutter_set_alarm(ALARM_CODE_SHUTTER_FORCED_QUIT);

		/* system error ? */
		_shutter_system_error(0, 8);
		break;
	}
}


/*********************************************************************//**
 * @brief shutter task busy procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _shutter_busy_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;

	ercd = twai_flg(ID_SHUTTER_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TASK_WAIT_TIME);
	if (ercd != E_OK)
	{
		flag = 0;
	}

	switch (ex_shutter_task_seq & 0xFF00)
	{
	case SHUTTER_SEQ_IDLE:
		_shutter_idle_seq_proc(flag);
		break;
	case SHUTTER_SEQ_INITIAL_OPEN:
		_shutter_initial_open_seq_proc(flag);
		break;
	case SHUTTER_SEQ_OPEN:
		_shutter_open_seq_proc(flag);
		break;
	case SHUTTER_SEQ_CLOSE:
		_shutter_close_seq_proc(flag);
		break;
	case SHUTTER_SEQ_FREERUN:
		_shutter_freerun_seq_proc(flag);
		break;
	default:
		_shutter_set_alarm(ALARM_CODE_SHUTTER_FORCED_QUIT);

		/* system error ? */
		_shutter_system_error(0, 6);
		break;
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  shutter task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _shutter_busy_msg_proc(void)
{

	switch (shutter_msg.tmsg_code)
	{
	case TMSG_SHUTTER_INITIAL_OPEN_REQ:					/* INITIAL message */
		s_shutter_alarm_code = 0;
		s_shutter_alarm_retry = 0;
		_shutter_set_waiting_seq(SHUTTER_SEQ_INITIAL_OPEN);
		break;

	case TMSG_SHUTTER_OPEN_REQ:
		s_shutter_alarm_code = 0;
		s_shutter_alarm_retry = 0;
		_shutter_set_waiting_seq(SHUTTER_SEQ_OPEN);
		break;
	case TMSG_SHUTTER_CLOSE_REQ:					/* CLOSE message */
		s_shutter_alarm_code = 0;
		s_shutter_alarm_retry = 0;
		_shutter_set_waiting_seq(SHUTTER_SEQ_CLOSE);
		break;
	case TMSG_SHUTTER_FREERUN_REQ:				/* FREERUN message */
		s_shutter_alarm_code = 0;
		if (shutter_msg.arg1 == MOTOR_STOP)
		{
			_shutter_set_waiting_seq(SHUTTER_SEQ_FREERUN);
		}
		break;
	default:					/* other */
		_shutter_set_alarm(ALARM_CODE_SHUTTER_FORCED_QUIT);

		/* system error ? */
		_shutter_system_error(0, 7);
		break;
	}
}



/*********************************************************************//**
 * shutter open
 **********************************************************************/
/*********************************************************************//**
 * @brief shutter control interrupt procedure (open sequence)
 * @param[in]	shutter motor event flag
 * @return 		None
 **********************************************************************/
void _shutter_open_seq_proc(u32 flag)
{
	switch (ex_shutter_task_seq & 0x00FF)
	{
	case 0x00:									/* seq3100 */
		_shutter_open_A200_seq(flag);
		break;

	case 0x04:
		_shutter_open_A204_seq(flag);
		break;

	case 0x06:
		_shutter_open_A206_seq(flag);
		break;

	default:
		_shutter_set_alarm(ALARM_CODE_SHUTTER_FORCED_QUIT);

		/* system error ? */
		_shutter_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief shutter control sequence
 *  wait motor start
 * @param[in]	shutter motor event flag
 * @return 		None
 **********************************************************************/
void _shutter_open_A200_seq_start(void)
{

	if (SENSOR_SHUTTER_OPEN)
	{
		_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_OPEN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
	}
	else if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
	{
		_shutter_set_seq(0xA204, SHUTTER_SEQ_TIMEOUT);
	}

}

void _shutter_open_A200_seq(u32 flag)
{

	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_open_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (SENSOR_SHUTTER_OPEN)
	{
		_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_OPEN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
	}
	else if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
	{
		_shutter_set_seq(0xA204, SHUTTER_SEQ_TIMEOUT);
	}

}


/*********************************************************************//**
 * @brief shutter control sequence
 *  wait home interrupt
 * @param[in]	shutter motor event flag
 * @return 		None
 **********************************************************************/
void _shutter_open_A204_seq(u32 flag)
{
	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_open_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (SENSOR_SHUTTER_OPEN)
	{
		motor_ctrl_shutter_stop();
		_shutter_set_seq(0xA206, SHUTTER_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief shutter control sequence
 *  wait motor stop
 * @param[in]	shutter motor event flag
 * @return 		None
 **********************************************************************/
void _shutter_open_A206_seq(u32 flag)
{
	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
		_shutter_set_open_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (is_motor_ctrl_shutter_stop())
	{
	/* no error */
		if (SENSOR_SHUTTER_OPEN)
		{
			_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_OPEN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
		}
		else
		{
			_shutter_set_open_retry(ALARM_CODE_SHUTTER_HOME_STOP);
		}
	}
}

/*********************************************************************//**
 * @brief shutter control sub function
 *  set home retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _shutter_set_open_retry(u32 alarm_code)
{
	s_shutter_alarm_retry++;
	if (s_shutter_alarm_retry >= SHUTTER_RETRY_COUNT)
	{
	/* retry over */
		_shutter_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_shutter_stop()))
		{
			motor_ctrl_shutter_stop();
		}
		_shutter_set_seq(0xA200, SHUTTER_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * shutter exec
 **********************************************************************/
/*********************************************************************//**
 * @brief shutter control interrupt procedure (exec sequence)
 * @param[in]	shutter motor event flag
 * @return 		None
 **********************************************************************/
void _shutter_close_seq_proc(u32 flag)
{

	switch (ex_shutter_task_seq & 0x00FF)
	{
	case 0x00:
		_shutter_close_A300_seq(flag);
		break;

	case 0x04:
		_shutter_close_A304_seq(flag);
		break;

	case 0x06:
		_shutter_close_A306_seq(flag);
		break;

	default:									/* other */
		_shutter_set_alarm(ALARM_CODE_SHUTTER_FORCED_QUIT);

		/* system error ? */
		_shutter_system_error(0, 10);
		break;
	}
}


void _shutter_close_A300_seq_start(void)
{

	if (!(SENSOR_SHUTTER_OPEN))
	{
		_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_CLOSE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
	}
	else if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
	{
		_shutter_set_seq(0xA304, SHUTTER_SEQ_TIMEOUT);
	}

}

void _shutter_close_A300_seq(u32 flag)
{
	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_close_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (!(SENSOR_SHUTTER_OPEN))
	{
		_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_CLOSE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
	}
	else if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
	{
		_shutter_set_seq(0xA304, SHUTTER_SEQ_TIMEOUT);
	}
}


void _shutter_close_A304_seq(u32 flag)
{

	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_close_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (!(SENSOR_SHUTTER_OPEN))
	{
		motor_ctrl_shutter_stop();
		_shutter_set_seq(0xA306, SHUTTER_SEQ_TIMEOUT);
	}
}

void _shutter_close_A306_seq(u32 flag)
{
	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
		_shutter_set_close_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (is_motor_ctrl_shutter_stop())
	{
	/* no error */
		if (!(SENSOR_SHUTTER_OPEN))
		{
			_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_CLOSE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
		}
		else
		{
			_shutter_set_close_retry(ALARM_CODE_SHUTTER_HOME_STOP);
		}
	}
}

/*********************************************************************//**
 * @brief shutter control sub function
 *  set exec retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _shutter_set_close_retry(u32 alarm_code)
{
	s_shutter_alarm_retry++;

	if (s_shutter_alarm_retry >= SHUTTER_RETRY_COUNT)
	{
	/* retry over */
		_shutter_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_shutter_stop()))
		{
			motor_ctrl_shutter_stop();
		}
		_shutter_set_seq(0xA300, SHUTTER_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * shutter init
 **********************************************************************/
/*********************************************************************//**
 * @brief shutter control interrupt procedure (initialize sequence)
 * @param[in]	shutter motor event flag
 * @return 		None
 **********************************************************************/
void _shutter_initial_open_seq_proc(u32 flag)
{
	switch (ex_shutter_task_seq & 0x00FF)
	{
	case 0x00:
		_shutter_initial_open_A100_seq(flag);
		break;
	case 0x04:
		_shutter_initial_open_A104_seq(flag);
		break;
	case 0x06:
		_shutter_initial_open_A106_seq(flag);
		break;
	case 0x08:
		_shutter_initial_open_A108_seq(flag);
		break;

	default:									/* other */
		_shutter_set_alarm(ALARM_CODE_SHUTTER_FORCED_QUIT);

		/* system error ? */
		_shutter_system_error(0, 16);

		break;
	}
}


/*********************************************************************//**
 * @brief shutter control sequence 0x3300
 *  wait motor start
 * @param[in]	shutter motor event flag
 * @return 		None
 **********************************************************************/
void _shutter_initial_open_A100_seq_start(void)
{
	if (SENSOR_SHUTTER_OPEN)
	{
		if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
		{
			ex_dline_testmode.test_start = 1;
			ex_dline_testmode.time_count = 0;
			/* close待ちへ */
			_shutter_set_seq(0xA104, SHUTTER_SEQ_TIMEOUT);
		}
	}
	else if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
	{
		/* open待ち */
		_shutter_set_seq(0xA106, SHUTTER_SEQ_TIMEOUT);
	}
}

void _shutter_initial_open_A100_seq(u32 flag)
{
	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_initial_open_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (SENSOR_SHUTTER_OPEN)
	{
		if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
		{
			ex_dline_testmode.test_start = 1;
			ex_dline_testmode.time_count = 0;

			/* close待ちへ */
			_shutter_set_seq(0xA104, SHUTTER_SEQ_TIMEOUT);
		}
	}
	else if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
	{
		/* open待ち */
		_shutter_set_seq(0xA106, SHUTTER_SEQ_TIMEOUT);
	}

}

void _shutter_initial_open_A104_seq(u32 flag)/* close待ち */
{

	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_initial_open_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (!(SENSOR_SHUTTER_OPEN))
	{
	/* close */
		_shutter_set_seq(0xA106, SHUTTER_SEQ_TIMEOUT);
	}
}


void _shutter_initial_open_A106_seq(u32 flag)/* open待ち */
{

	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_initial_open_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if ( SENSOR_SHUTTER_OPEN )
	{
		if( ex_dline_testmode.test_start == 1 )
		{
			ex_dline_testmode.time1 = ex_dline_testmode.time_count;	//2018-11-28 時間を保存
			ex_dline_testmode.test_start = 0;
			ex_dline_testmode.test_result = TEST_RESULT_OK;
		}

	/* open */
		motor_ctrl_shutter_stop();
		_shutter_set_seq(0xA108, SHUTTER_SEQ_TIMEOUT);
	}
}

void _shutter_initial_open_A108_seq(u32 flag) /* 停止待ち */
{
	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
		_shutter_set_initial_open_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (is_motor_ctrl_shutter_stop())
	{
	/* no error */
		ex_shutter_count++;
		if (SENSOR_SHUTTER_OPEN)
		{
			_shutter_send_msg(ID_MAIN_MBX, TMSG_SHUTTER_INITIAL_OPEN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
		}
		else
		{
			_shutter_set_initial_open_retry(ALARM_CODE_SHUTTER_HOME_STOP);
		}
	}
}

void _shutter_set_initial_open_retry(u32 alarm_code)
{
	s_shutter_alarm_retry++;

	if (s_shutter_alarm_retry >= SHUTTER_RETRY_COUNT)
	{
	/* retry over */
		_shutter_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_shutter_stop()))
		{
			motor_ctrl_shutter_stop();
		}
		_shutter_set_seq(0xA100, SHUTTER_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * shutter free run
 **********************************************************************/
void _shutter_freerun_seq_proc(u32 flag) //現状使用してないはず
{

	switch (ex_shutter_task_seq & 0x00FF)
	{
	case 0x00:
		_shutter_freerun_AA00_seq(flag);
		break;

	case 0x04:
		_shutter_freerun_AA04_seq(flag);
		break;

	case 0x06:
		_shutter_freerun_AA06_seq(flag);
		break;

	default:									/* other */
		_shutter_set_alarm(ALARM_CODE_SHUTTER_FORCED_QUIT);

		/* system error ? */
		_shutter_system_error(0, 10);
		break;
	}
}


void _shutter_freerun_AA00_seq(u32 flag)
{

	if (s_shutter_freerun_dir == MOTOR_FWD)
	{
		if (IERR_CODE_OK == motor_ctrl_shutter_fwd(MOTOR_MAX_SPEED, 1))
		{
			/* とりあえず6に飛ばす */
			_shutter_set_seq(0xAA06, SHUTTER_SEQ_TIMEOUT);
		}
	}
	else if (s_shutter_freerun_dir == MOTOR_REV)
	{
		if (IERR_CODE_OK == motor_ctrl_shutter_rev(MOTOR_MAX_SPEED, 1))
		{
			/* とりあえず6に飛ばす */
			_shutter_set_seq(0xAA06, SHUTTER_SEQ_TIMEOUT);
		}
	}
	else
	{
		_shutter_set_seq(SHUTTER_SEQ_IDLE, 0);
		/* system error ? */
		_shutter_system_error(0, 12);
	}
}


void _shutter_freerun_AA04_seq(u32 flag)
{


	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_freerun_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (!(SENSOR_SHUTTER_OPEN))
	{
		motor_ctrl_shutter_stop();
		_shutter_set_seq(0xAA06, SHUTTER_SEQ_TIMEOUT);
	}
}

void _shutter_freerun_AA06_seq(u32 flag)
{

	if (IS_SHUTTER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_shutter_set_freerun_retry(ALARM_CODE_SHUTTER_TIMEOUT);
	}
	else if (!(SENSOR_SHUTTER_OPEN))
	{
		motor_ctrl_shutter_stop();
		_shutter_set_seq(0xAA06, SHUTTER_SEQ_TIMEOUT);
	}
	
}

/*********************************************************************//**
 * @brief shutter control sub function
 *  set exec retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _shutter_set_freerun_retry(u32 alarm_code)
{
	s_shutter_alarm_retry++;

	if (s_shutter_alarm_retry >= SHUTTER_RETRY_COUNT)
	{
	/* retry over */
		_shutter_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_shutter_stop()))
		{
			motor_ctrl_shutter_stop();
		}
		_shutter_set_seq(0xA300, SHUTTER_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief shutter control sub function
 *  set shutter sequence
 * @param[in]	sequence no.
 * 				time out
 * @return 		None
 **********************************************************************/
void _shutter_set_seq(u16 seq, u16 time_out)
{
	ex_shutter_task_seq = seq;
	_ir_shutter_ctrl_time_out = time_out;
	clr_flg(ID_SHUTTER_CTRL_FLAG, ~EVT_ALL_BIT);

#if 1//#ifdef _ENABLE_JDL
	jdl_add_trace(ID_SHUTTER_TASK, ((ex_shutter_task_seq >> 8) & 0xFF), (ex_shutter_task_seq & 0xFF), s_shutter_alarm_code, s_shutter_alarm_retry, 0);
#endif /* _ENABLE_JDL */

}


/*********************************************************************//**
 * @brief shutter control sub function
 *  set shutter waiteing sequence
 * @param[in]	sequence no.
 * @return 		None
 **********************************************************************/
void _shutter_set_waiting_seq(u32 seq)
{
	if (ex_shutter_task_seq == SHUTTER_SEQ_FORCE_QUIT)
	{
		s_shutter_task_wait_seq = seq;
	}
	else
	{
		s_shutter_task_wait_seq = seq;

		motor_ctrl_shutter_stop();
		_shutter_set_seq(SHUTTER_SEQ_FORCE_QUIT, SHUTTER_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief shutter control sub function
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _shutter_set_alarm(u32 alarm_code)
{
	u32 msg;
	u32 sensor;

	motor_ctrl_shutter_stop();

	s_shutter_alarm_code = alarm_code;

	switch (ex_shutter_task_seq & 0xFF00)
	{
	case SHUTTER_SEQ_INITIAL_OPEN:
		msg = TMSG_SHUTTER_INITIAL_OPEN_RSP;
		break;

	case SHUTTER_SEQ_OPEN:
		msg = TMSG_SHUTTER_OPEN_RSP;
		break;
	case SHUTTER_SEQ_CLOSE:
		msg = TMSG_SHUTTER_CLOSE_RSP;
		break;
	case SHUTTER_SEQ_FREERUN:
		msg = TMSG_SHUTTER_FREERUN_RSP;
		break;
	default:
		msg = TMSG_SHUTTER_STATUS_INFO;
		break;
	}

	sensor = ex_position_sensor;

	_shutter_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_ALARM, s_shutter_alarm_code, ex_shutter_task_seq, sensor);
	_shutter_set_seq(SHUTTER_SEQ_FORCE_QUIT, SHUTTER_SEQ_TIMEOUT);
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
void _shutter_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_SHUTTER_TASK;
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
			_shutter_system_error(1, 3);
		}
	}
	else
	{
		/* system error */
		_shutter_system_error(1, 4);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _shutter_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR

	//_shutter_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_BLINK, 1, 0, 0, 0);

#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_shutter_send_msg(ID_ERRDISP_MBX, TMSG_ERRDISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	// fatal_err == 1
	_debug_system_error(ID_SHUTTER_TASK, (u16)code, (u16)shutter_msg.tmsg_code, (u16)shutter_msg.arg1, fatal_err);
}

/* EOF */
