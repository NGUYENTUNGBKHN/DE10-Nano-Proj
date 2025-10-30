/******************************************************************************/
/*! @addtogroup Group2
    @file       centering_task.c
    @brief      centering task process
    @date       2012/09/20
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2012/09/20 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "sub_functions.h"
#include "motor_ctrl.h"


#include "systemdef.h"					//2022-01-12 test
#include "cyclonev_sysmgr_reg_def.h"	//2022-01-12 test
#include "hal_gpio_reg.h"				//2022-01-12 test


#define EXT
#include "com_ram.c"

#include "jdl_conf.h"

/************************** PRIVATE DEFINITIONS *************************/
enum _CENTERING_SEQ
{
	CENTERING_SEQ_IDLE			= 0x2000,
	CENTERING_SEQ_FORCE_QUIT	= 0x2001,
	CENTERING_SEQ_HOME			= 0x2100,
	CENTERING_SEQ_EXEC			= 0x2200,
	CENTERING_SEQ_CLOSE			= 0x2300,
	CENTERING_SEQ_HOME_OUT_RETRY	= 0x2400,
	CENTERING_SEQ_FREERUN		= 0x2F00,
};


/* 通常動作では1回の幅寄せ動作は300ms程度 */
#define CENTERING_SEQ_TIMEOUT			800		/* 800ms	*/
//#define CENTERING_SEQ_TIMEOUT			600		/* 600ms	*/

#define CENTERING_RETRY_COUNT			3
#define CENTERING_REV_TIME				50		/* 50msec 	*//* 通常のもとからのリトライ用 200-> 50 *//* 2021-07-08 */

//2022-01-11	#define CENTERING_SEQ_STABLE_TIMEOUT	20		/* 20ms		*//* HomeになってからHome位置が安定するまでのFWD時間*/
#define CENTERING_SEQ_STABLE_TIMEOUT	5		/* 20ms		*//* HomeになってからHome位置が安定するまでのFWD時間*//* 2022-02-15 700対応*/

#define CENTERING_SEQ_HOME_OUT_TIMEOUT	600		/* 600ms	*//* Home Out になるまでの待ち時間	*/
//#define CENTERING_SEQ_HOME_OUT_TIMEOUT	280		/* 280ms	*//* Home Out になるまでの待ち時間	*/

#define CENTERING_SEQ_HOME_IN_TIMEOUT	600		/* 600ms	*//* Home になるまでの待ち時間		*/
//#define CENTERING_SEQ_HOME_IN_TIMEOUT	300		/* 300ms	*//* Home になるまでの待ち時間		*/

#define CENTERING_FWD_RETRY_TIME		110		/* 110msec 	*//* Home outしない時のリトライ用*/
#define CENTERING_REV_RETRY_TIME		20		/* 20msec 	*//* Home outしない時のリトライ用*/

//#define IS_CENTERING_EVT_SENSOR(x)		((x & EVT_CENTERING_SENSOR) == EVT_CENTERING_SENSOR)
#define IS_CENTERING_EVT_HOME_INTR(x)	((x & EVT_CENTERING_HOME_INTR) == EVT_CENTERING_HOME_INTR)
//#define IS_CENTERING_EVT_MOTOR_STOP(x)	((x & EVT_CENTERING_MOTOR_STOP) == EVT_CENTERING_MOTOR_STOP)
#define IS_CENTERING_EVT_TIMEOUT(x)		((x & EVT_CENTERING_TIMEOUT) == EVT_CENTERING_TIMEOUT)

#define IS_CENTERING_EVT_HOME_OUT_INTR(x)	((x & EVT_CENTERING_HOME_OUT_INTR) == EVT_CENTERING_HOME_OUT_INTR)	/* 40ms間 Home outの場合のみ通知される */
#define IS_CENTERING_EVT_RUN_TIMEOUT(x)		((x & EVT_CENTERING_RUN_TIMEOUT) == EVT_CENTERING_RUN_TIMEOUT)
#define IS_CENTERING_EVT_HOME_OUT_INTR_10MSEC(x)	((x & EVT_CENTERING_HOME_OUT_INTR_10MSEC) == EVT_CENTERING_HOME_OUT_INTR_10MSEC)	/* 10ms間 Home outの場合のみ通知される *//* 2021-05-27 *//* 幅よせclose専用 */

/************************** PRIVATE VARIABLES *************************/
static T_MSG_BASIC centering_msg;
static u16 s_centering_task_wait_seq;
static u8 s_centering_alarm_code;
static u8 s_centering_alarm_retry;
static u8 s_centering_alarm_disable;



/************************** PRIVATE FUNCTIONS *************************/
void _centering_initialize_proc(void);
void _centering_idle_msg_proc(void);
void _centering_idel_proc(void);
void _centering_busy_msg_proc(void);
void _centering_busy_proc(void);

/* Centering idle sequence */
void _centering_idle_seq_proc(u32 flag);
void _centering_idle_2001_seq(u32 flag);

/* Centering home sequence */
void _centering_home_seq_proc(u32 flag);
void _centering_home_2100_seq(u32 flag);
void _centering_home_2101_seq(u32 flag);
void _centering_home_2104_seq(u32 flag);
void _centering_home_2106_seq(u32 flag);
void _centering_set_home_retry(u32 alarm_code);
void _centering_home_2110_seq(u32 flag);
void _centering_home_2111_seq(u32 flag);

void _centering_home_2120_seq(u32 flag);	/* 2021-07-08 */
void _centering_home_2122_seq(u32 flag);
void _centering_home_2124_seq(u32 flag);
void _centering_home_2126_seq(u32 flag);
void _centering_home_2128_seq(u32 flag);
void _centering_home_212A_seq(u32 flag);


/* Centering exec sequence */
// Homeを抜けない場合、リトライしない
void _centering_exec_seq_proc(u32 flag);//ok
void _centering_exec_2200_seq_start(void);//ok
void _centering_exec_2200_seq(u32 flag);//ok
void _centering_exec_2202_seq(u32 flag);//ok
void _centering_exec_2204_seq(u32 flag);//ok
void _centering_exec_2206_seq(u32 flag);//ok
void _centering_exec_2208_seq(u32 flag);//ok
void _centering_exec_220A_seq(u32 flag);//ok
void _centering_set_exec_retry(u32 alarm_code);//ok
void _centering_exec_2210_seq(u32 flag);//ok
void _centering_exec_2212_seq(u32 flag);//ok

void _centering_exec_2220_seq(u32 flag);// 0x2400と同じ
void _centering_exec_2224_seq(u32 flag);//
void _centering_exec_2226_seq(u32 flag);//
void _centering_exec_2228_seq(u32 flag);//
void _centering_exec_222A_seq(u32 flag);//


/* Centering close sequence */
void _centering_close_seq_proc(u32 flag);
void _centering_close_2300_seq(u32 flag);
void _centering_close_2302_seq(u32 flag);
void _centering_close_2304_seq(u32 flag);
void _centering_close_230A_seq(u32 flag);
void _centering_set_close_retry(u32 alarm_code);

void _centering_home_out_retry_seq_proc(u32 flag);//ok
void _centering_home_out_retry_2400_seq(u32 flag);//ok
void _centering_home_out_retry_2404_seq(u32 flag);//ok
void _centering_home_out_retry_2406_seq(u32 flag);//ok
void _centering_home_out_retry_2408_seq(u32 flag);//ok
void _centering_home_out_retry_240A_seq(u32 flag);//ok


/* Centering motor free run sequence */
void _centering_freerun_seq_proc(u32 flag);
void _centering_freerun_2f00_seq(u32 flag);
void _centering_freerun_2f02_seq(u32 flag);
void _centering_freerun_2f04_seq(u32 flag);
void _centering_freerun_2f06_seq(u32 flag);
void _centering_freerun_2f08_seq(u32 flag);
void _centering_freerun_2f0A_seq(u32 flag);
void _centering_set_exec_retry_freerun(u32 alarm_code);//ok
void _centering_freerun_2f10_seq(u32 flag);
void _centering_freerun_2f12_seq(u32 flag);

/* Centering sub functions */
void _centering_set_seq(u16 seq, u16 time_out);
void _centering_set_waiting_seq(u32 seq);

void _centering_set_alarm(u32 alarm_code);
void _centering_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _centering_system_error(u8 fatal_err, u8 code);

/************************** EXTERN VARIABLES *************************/

/************************** EXTERN FUNCTIONS *************************/


/*********************************************************************//**
 * @ centering task
 * @param[in]	extension information
 * @return 		None
 **********************************************************************/
void centering_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	
	_centering_initialize_proc();
	
	while(1)
	{
		if (ex_centering_task_seq == CENTERING_SEQ_IDLE)
		{
		/* idle */
			ercd = trcv_mbx(ID_CENTERING_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
			if (ercd == E_OK)
			{
				memcpy(&centering_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(centering_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_centering_system_error(1, 3);
				}
				_centering_idel_proc();
				_centering_idle_msg_proc();
			}
			else
			{
				_centering_idel_proc();
			}
		}
		else
		{
		/* busy */
			_centering_busy_proc();
			ercd = prcv_mbx(ID_CENTERING_MBX, (T_MSG **)&tmsg_pt);
			if (ercd == E_OK)
			{
				memcpy(&centering_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(centering_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_centering_system_error(1, 4);
				}
				_centering_busy_msg_proc();
			}
		}
	}
}


/*********************************************************************//**
 * @brief initialize centering task
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _centering_initialize_proc(void)
{
	ex_centering_task_seq = CENTERING_SEQ_IDLE;
	s_centering_task_wait_seq = CENTERING_SEQ_IDLE;
	
	clr_flg(ID_CENTERING_CTRL_FLAG, ~EVT_ALL_BIT);
}


/*********************************************************************//**
 * @brief centering task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _centering_idel_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;
	
	ercd = pol_flg(ID_CENTERING_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag);
	if (ercd != E_OK)
	{
		flag = 0;
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  centering task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _centering_idle_msg_proc(void)
{
	switch (centering_msg.tmsg_code)
	{
	case TMSG_CENTERING_HOME_REQ:					/* HOME message */

		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_seq(CENTERING_SEQ_HOME, CENTERING_SEQ_TIMEOUT);
		break;
	case TMSG_CENTERING_EXEC_REQ:					/* EXEC message */
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = centering_msg.arg1;
		_centering_set_seq(CENTERING_SEQ_EXEC, CENTERING_SEQ_TIMEOUT);
		_centering_exec_2200_seq_start();//ok
		break;

	case TMSG_CENTERING_CLOSE_REQ:					/* CLOSE message */

		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_seq(CENTERING_SEQ_CLOSE, CENTERING_SEQ_TIMEOUT);
		break;

	case TMSG_CENTERING_HOME_OUT_RETRY_REQ:
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_seq(CENTERING_SEQ_HOME_OUT_RETRY, CENTERING_SEQ_TIMEOUT);
		break;

	case TMSG_CENTERING_FREERUN_REQ:				/* FREERUN message *//* use test mode */
		// only one way
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_seq(CENTERING_SEQ_FREERUN, CENTERING_SEQ_TIMEOUT);
		break;
	default:					/* other */
		/* system error ? */
		_centering_system_error(0, 5);
		break;
	}
}


/*********************************************************************//**
 * @brief centering task busy procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _centering_busy_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;
	
	ercd = twai_flg(ID_CENTERING_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TASK_WAIT_TIME);
	if (ercd != E_OK)
	{
		flag = 0;
	}
	
	switch (ex_centering_task_seq & 0xFF00)
	{
	case CENTERING_SEQ_IDLE://20XX use
		_centering_idle_seq_proc(flag);
		break;
	case CENTERING_SEQ_HOME://21XX use
		_centering_home_seq_proc(flag);
		break;
	case CENTERING_SEQ_EXEC://22XX use
		_centering_exec_seq_proc(flag);
		break;
	case CENTERING_SEQ_CLOSE://23XX
		_centering_close_seq_proc(flag);
		break;
	case CENTERING_SEQ_HOME_OUT_RETRY://24XX
		_centering_home_out_retry_seq_proc(flag);
		break;
	case CENTERING_SEQ_FREERUN:
		_centering_freerun_seq_proc(flag);
		break;
	default:
		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 6);
		break;
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  centering task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _centering_busy_msg_proc(void)
{
	switch (centering_msg.tmsg_code)
	{
	case TMSG_CENTERING_HOME_REQ:					/* HOME message */
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_waiting_seq(CENTERING_SEQ_HOME);
		break;
	case TMSG_CENTERING_EXEC_REQ:					/* EXEC message */
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = centering_msg.arg1;
		_centering_set_waiting_seq(CENTERING_SEQ_EXEC);
		break;
	case TMSG_CENTERING_CLOSE_REQ:					/* CLOSE message */
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_waiting_seq(CENTERING_SEQ_CLOSE);
		break;
	case TMSG_CENTERING_HOME_OUT_RETRY_REQ:
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_waiting_seq(CENTERING_SEQ_HOME_OUT_RETRY);
		break;
	case TMSG_CENTERING_FREERUN_REQ:				/* FREERUN message */
		s_centering_alarm_code = 0;
		s_centering_alarm_retry = 0;
		s_centering_alarm_disable = 0;
		_centering_set_waiting_seq(CENTERING_SEQ_FREERUN);
		break;
	default:					/* other */
		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 7);
		break;
	}
}


/*********************************************************************//**
 * idle
 **********************************************************************/
/*********************************************************************//**
 * @brief centering control interrupt procedure (idle sequence)
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_idle_seq_proc(u32 flag)
{
	switch (ex_centering_task_seq & 0x00FF)
	{
	case 0x01:
		_centering_idle_2001_seq(flag);
		break;
	default:
		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 8);
		break;
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2001
 *  wait motor stop
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_idle_2001_seq(u32 flag)
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_alarm(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (s_centering_task_wait_seq == CENTERING_SEQ_IDLE)
		{
			_centering_set_seq(s_centering_task_wait_seq, 0);
		}
		else if (s_centering_task_wait_seq == CENTERING_SEQ_FREERUN)
		{
			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_FREERUN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else
		{
			_centering_set_seq(s_centering_task_wait_seq, CENTERING_SEQ_TIMEOUT);
		}
		s_centering_task_wait_seq = CENTERING_SEQ_IDLE;
	}
}


/*********************************************************************//**
 * centering home
 **********************************************************************/
/*********************************************************************//**
 * @brief centering control interrupt procedure (home sequence)
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_home_seq_proc(u32 flag)
{
	switch (ex_centering_task_seq & 0x00FF)
	{
	case 0x00:									/* seq2100 */
		_centering_home_2100_seq(flag);
		break;
	case 0x01:									/* seq2101 */
		_centering_home_2101_seq(flag);
		break;
	case 0x04:									/* seq2104 */
		_centering_home_2104_seq(flag);
		break;
	case 0x06:									/* seq2106 */
		_centering_home_2106_seq(flag);
		break;
	/* home retry */
	case 0x10:									/* seq2110 */
		_centering_home_2110_seq(flag);
		break;
	case 0x11:									/* seq2111 */
		_centering_home_2111_seq(flag);
		break;

	/* 手動などで、幅よせをいじられると、欠歯の関係せいが崩れHomeの区間が短くなる */
	/* Home検知後20msec経過後に停止では、Homeを外れる場合がある*/
	/* 上記の問題より、リトライ動作では、一度Homeにいれて、欠歯の関係性を戻し、*/
	/* 2回目のHome検知で停止させる												*/
	/* 2021-07-08 *//* リトライ動作は通常と動作を変える*//* Homeに入っても1回目は無視 *//* 欠歯の関係で一度Homeにいれた後でないとHome位置のマージンが少なくて、Homeからすぐ外れるから */
	case 0x20:									/* seq2110 */
		_centering_home_2120_seq(flag);
		break;
	case 0x22:									/* seq2111 */
		_centering_home_2122_seq(flag);
		break;
	case 0x24:									/* seq2111 */
		_centering_home_2124_seq(flag);
		break;
	case 0x26:									/* seq2111 */
		_centering_home_2126_seq(flag);
		break;
	case 0x28:									/* seq2111 */
		_centering_home_2128_seq(flag);
		break;
	case 0x2A:									/* seq2111 */
		_centering_home_212A_seq(flag);
		break;


	default:

		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2100
 *  wait motor start
 * @param[in]	centering motor event flag
 * @return 		None
// 幅よせHome戻し動作、モータ起動待ち
 **********************************************************************/
void _centering_home_2100_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (SENSOR_CENTERING_HOME)
		{

			ex_dline_testmode.test_result = TEST_RESULT_OK;

			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, 0))
		{
			// MAINに起動開始を通知
			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_RSP, TMSG_SUB_START, 0, 0, 0);
			_centering_set_seq(0x2101, CENTERING_SEQ_HOME_IN_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2101
 *  wait home interrupt
 * @param[in]	centering motor event flag
 * @return 		None
// 幅よせHomeへ戻し動作中
 **********************************************************************/
void _centering_home_2101_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
		set_centering_timeout(1, CENTERING_SEQ_STABLE_TIMEOUT);
		_centering_set_seq(0x2104, CENTERING_SEQ_TIMEOUT);
	}
}


/* Home後 10msec待ち */
void _centering_home_2104_seq(u32 flag)//ok also use test moede
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2106, CENTERING_SEQ_TIMEOUT);
	}
}

void _centering_home_2106_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
	/* no error */
		if (SENSOR_CENTERING_HOME)
		{

			ex_dline_testmode.test_result = TEST_RESULT_OK;

			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else
		{
			_centering_set_home_retry(ALARM_CODE_CENTERING_HOME_STOP);
		}
	}
}

/*********************************************************************//**
 * @brief centering control sub function
 *  set home retry
 * @param[in]	alarm code
 * @return 		None
 //幅よせHome戻し処理、リトライ確認
 **********************************************************************/
void _centering_set_home_retry(u32 alarm_code)//ok
{
	s_centering_alarm_retry++;
	if (s_centering_alarm_retry >= CENTERING_RETRY_COUNT)
	{
	/* retry over */
		_centering_set_alarm(alarm_code);
	}
	else
	{
		s_centering_alarm_code = alarm_code;
		
		if (!(is_motor_ctrl_centering_stop()))
		{
			motor_ctrl_centering_stop();
		}
		_centering_set_seq(0x2110, CENTERING_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2110
 *  wait motor start
 * @param[in]	centering motor event flag
 * @return 		None
// 幅よせHome戻し、リトライ処理モータ起動待ち
 **********************************************************************/
void _centering_home_2110_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (SENSOR_CENTERING_HOME)
		{
			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else if (IERR_CODE_OK == motor_ctrl_centering_rev(MOTOR_MAX_SPEED, 0, 0))
		{
			_centering_set_seq(0x2111, CENTERING_REV_TIME); /* 200msec -> 50msec*/
		}
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2111
 *  centering revers
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_home_2111_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
//2021-07-08 		_centering_set_seq(0x2100, CENTERING_SEQ_TIMEOUT);
		_centering_set_seq(0x2120, CENTERING_SEQ_TIMEOUT);

	}
}


void _centering_home_2120_seq(u32 flag)//ok
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		//取り合えず、閉じて、その後開いて、再度閉じる
		//合計2回Homeに入れる
		if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_IN_TIMEOUT))
		{
			if (s_centering_alarm_retry == 0)
			{
			//リトライ時ではなく、最初の1回のみメッセージ送信
				_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_RSP, TMSG_SUB_START, 0, 0, 0);
			}
			_centering_set_seq(0x2122, CENTERING_SEQ_TIMEOUT);
		}
	}
}

/* 1回 Homeに入れる */
void _centering_home_2122_seq(u32 flag)//ok also use test moede
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
		/* ok Home outへ*/
		set_centering_timeout(1, CENTERING_SEQ_HOME_OUT_TIMEOUT);
		_centering_set_seq(0x2124, CENTERING_SEQ_TIMEOUT);
	}
}

/* Home OUT待ち*/
/* 条件 20msec以内のHome検知は無視 */
void _centering_home_2124_seq(u32 flag)
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
	/* エラー処理 シーケンスこのまま*/
	/* 40msec以下の瞬間Home INは無視してもう一度 シーケンスはこのまま */

	}
	else if (IS_CENTERING_EVT_HOME_OUT_INTR(flag))
	{
	/* 主の処理 Home out */
		set_centering_timeout(1, CENTERING_SEQ_HOME_IN_TIMEOUT);
		_centering_set_seq(0x2126, CENTERING_SEQ_TIMEOUT);
	}
}

void _centering_home_2126_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
		set_centering_timeout(1, CENTERING_SEQ_STABLE_TIMEOUT);
		_centering_set_seq(0x2128, CENTERING_SEQ_TIMEOUT);
	}
}

/* Home後 10msec待ち */
void _centering_home_2128_seq(u32 flag)//ok also use test moede
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x212A, CENTERING_SEQ_TIMEOUT);
	}
}

void _centering_home_212A_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_home_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
	/* no error */
		if (SENSOR_CENTERING_HOME)
		{
			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else
		{
			_centering_set_home_retry(ALARM_CODE_CENTERING_HOME_STOP);
		}
	}
}


/*********************************************************************//**
 * centering exec
 **********************************************************************/
/*********************************************************************//**
 * @brief centering control interrupt procedure (exec sequence)
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_exec_seq_proc(u32 flag)
{
	switch(ex_centering_task_seq & 0x00FF)
	{
	case 0x00:
		_centering_exec_2200_seq(flag);
		break;
	case 0x02:
		_centering_exec_2202_seq(flag);
		break;
	case 0x04:
		_centering_exec_2204_seq(flag);
		break;
	case 0x06:
		_centering_exec_2206_seq(flag);
		break;
	case 0x08:
		_centering_exec_2208_seq(flag);
		break;
	case 0x0A:
		_centering_exec_220A_seq(flag);
		break;
	/* exec retry */
	case 0x10:									/* seq2210 */
		_centering_exec_2210_seq(flag);
		break;
	case 0x12:									/* seq2212 */
		_centering_exec_2212_seq(flag);
		break;

	case 0x20:
		_centering_exec_2220_seq(flag);
		break;
	case 0x24:
		_centering_exec_2224_seq(flag);
		break;
	case 0x26:
		_centering_exec_2226_seq(flag);
		break;
	case 0x28:
		_centering_exec_2228_seq(flag);
		break;
	case 0x2A:
		_centering_exec_222A_seq(flag);
		break;

	default:									/* other */
		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 10);
		break;
	}
}

/*********************************************************************//**
 * @brief centering control sequence 0x2200
 *  wait motor start
 * @param[in]	centering motor event flag
 * @return 		None
 // 幅よせモータ正転起動待ち
 **********************************************************************/
void _centering_exec_2200_seq_start(void)//ok
{

	if (is_motor_ctrl_centering_stop())
	{
#if (DATA_COLLECTION_DEBUG==1)
		if(ex_collection_data.enable)
		{
			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else
		{
			if (SENSOR_CENTERING_HOME)
			{
			//すでにHomeの場合は、1回 Homeに入れる
				if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_OUT_TIMEOUT))
				{
					if (s_centering_alarm_retry == 0)
					{
					//リトライ時ではなく、最初の1回のみメッセージ送信
						_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
					}
					_centering_set_seq(0x2204, CENTERING_SEQ_TIMEOUT);
				}
			}
			else
			{
			//現在Homeでない場合、
			//取り合えず、閉じて、その後開いて、再度閉じる
			//合計2回Homeに入れる

				if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_IN_TIMEOUT))
				{
					if (s_centering_alarm_retry == 0)
					{
					//リトライ時ではなく、最初の1回のみメッセージ送信
						_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
					}
					_centering_set_seq(0x2202, CENTERING_SEQ_TIMEOUT);
				}
			}
		}
#else

		if (SENSOR_CENTERING_HOME)
		{
		//すでにHomeの場合は、1回 Homeに入れる
			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_OUT_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_centering_set_seq(0x2204, CENTERING_SEQ_TIMEOUT);
			}
		}
		else
		{
		//現在Homeでない場合、
		//取り合えず、閉じて、その後開いて、再度閉じる
		//合計2回Homeに入れる

			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_IN_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_centering_set_seq(0x2202, CENTERING_SEQ_TIMEOUT);
			}
		}
	#endif	
	}
}

/*********************************************************************//**
 * @brief centering control sequence 0x2200
 *  wait motor start
 * @param[in]	centering motor event flag
 * @return 		None
 // 幅よせモータ正転起動待ち
 **********************************************************************/
void _centering_exec_2200_seq(u32 flag)//ok
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);


	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (SENSOR_CENTERING_HOME)
		{
		//すでにHomeの場合は、1回 Homeに入れる
			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_OUT_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_centering_set_seq(0x2204, CENTERING_SEQ_TIMEOUT);
			}
		}
		else
		{
		//現在Homeでない場合、
		//取り合えず、閉じて、その後開いて、再度閉じる
		//合計2回Homeに入れる

			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_IN_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_centering_set_seq(0x2202, CENTERING_SEQ_TIMEOUT);
			}
		}
	}
}

/* 動かす前にすでにHome outなので1回 Homeに入れる */
/* Home 戻し待ち*/
void _centering_exec_2202_seq(u32 flag)//ok also use test moede
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
		/* ok Home outへ*/
		set_centering_timeout(1, CENTERING_SEQ_HOME_OUT_TIMEOUT);
		_centering_set_seq(0x2204, CENTERING_SEQ_TIMEOUT);
	}
}

/* Home OUT待ち*/
/* 条件 20msec以内のHome検知は無視 */
void _centering_exec_2204_seq(u32 flag)
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);


	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);


	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
	/* エラー処理 シーケンスこのまま*/
	/* 40msec以下の瞬間Home INは無視してもう一度 シーケンスはこのまま */

	}
//	else if(!SENSOR_CENTERING_HOME)
	else if (IS_CENTERING_EVT_HOME_OUT_INTR(flag))
	{
	/* 主の処理 Home out */
		set_centering_timeout(1, CENTERING_SEQ_HOME_IN_TIMEOUT);
		_centering_set_seq(0x2206, CENTERING_SEQ_TIMEOUT);
	}
}


/* Home Out中、Home 待ち*/
void _centering_exec_2206_seq(u32 flag)
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{

		/* ok Home outへ*/
		set_centering_timeout(1, CENTERING_SEQ_STABLE_TIMEOUT);
		_centering_set_seq(0x2208, CENTERING_SEQ_TIMEOUT);
	}
}

/* Home後 10msec待ち */
void _centering_exec_2208_seq(u32 flag)//ok also use test moede
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x220A, CENTERING_SEQ_TIMEOUT);
	}
}


/* モータ停止待ち */
void _centering_exec_220A_seq(u32 flag)//ok also use test moede
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
	/* no error */
		if (SENSOR_CENTERING_HOME)
		{
			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else
		{
		// Homeに入ってないのでリトライする
			_centering_set_exec_retry(ALARM_CODE_CENTERING_HOME_STOP);
		}
	}
}


/*********************************************************************//**
 * @brief centering control sub function
 *  set exec retry
 * @param[in]	alarm code
 * @return 		None
// リトライ回数確認処理
 **********************************************************************/
void _centering_set_exec_retry(u32 alarm_code)//ok
{
	s_centering_alarm_retry++;
	if (s_centering_alarm_retry >= CENTERING_RETRY_COUNT)
	{
	/* retry over */
		_centering_set_alarm(alarm_code);
	}
	else
	{
		s_centering_alarm_code = alarm_code;
		
		if (!(is_motor_ctrl_centering_stop()))
		{
			motor_ctrl_centering_stop();
		}

		if (SENSOR_CENTERING_HOME)
		{
		/* 幅よせがHomeかどうかでリトライ処理を変える*/
			if(s_centering_alarm_disable == 1)
			{
			/* リトライ動作なし*//* 内部でRejectメッセージ*/
				_centering_set_alarm(alarm_code);
			}
			else
			{
			/* リトライ処理へ*/
				_centering_set_seq(0x2220, CENTERING_SEQ_TIMEOUT);
			}
		}
		else
		{
		/* Homeに入らないのでリトライ処理 */
			_centering_set_seq(0x2210, CENTERING_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2210
 *  wait motor start
 * @param[in]	centering motor event flag
 * @return 		None
//幅よせをHomeに戻した時に、Homeに戻らなかった場合、リトライ処理を行う必要がある
//ローラのかみ合わせの関係で一度、一定時間逆転させる必要がある
//モータ停止確認後の逆転動作
 **********************************************************************/
void _centering_exec_2210_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IERR_CODE_OK == motor_ctrl_centering_rev(MOTOR_MAX_SPEED, 0, CENTERING_REV_TIME))  /* 200msec -> 50msec*/
	{
	//
		_centering_set_seq(0x2212, CENTERING_SEQ_TIMEOUT); //600ms
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2212
 *  centering revers
 * @param[in]	centering motor event flag
 * @return 		None
// リトライ処理の為、ローラかみ合わせの為の一定時間モータ逆転中
  **********************************************************************/
void _centering_exec_2212_seq(u32 flag)// ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* 本来の処理とちがうが、リトライさせる */
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2200, CENTERING_SEQ_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2200, CENTERING_SEQ_TIMEOUT);
	}
}



/* Home outを抜けない問題なので、特殊処理 逆転動作から*/
void _centering_exec_2220_seq(u32 flag)//ok
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_alarm(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else
	{
	/* 無条件で逆転から */
	/* Home outを抜けない問題なので、特殊処理 逆転動作から*/
		if (IERR_CODE_OK == motor_ctrl_centering_rev(MOTOR_MAX_SPEED, 0, CENTERING_REV_RETRY_TIME))/* 20msec 	*/
		{
			_centering_set_seq(0x2224, CENTERING_SEQ_TIMEOUT);
		}
	}
}

/* 逆転中*/
void _centering_exec_2224_seq(u32 flag)// ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* 本来の処理とちがうが、リトライさせる */
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2226, CENTERING_SEQ_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2226, CENTERING_SEQ_TIMEOUT);
	}
}

void _centering_exec_2226_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_alarm(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_FWD_RETRY_TIME))
		{
			_centering_set_seq(0x2228, CENTERING_SEQ_TIMEOUT);
		}
	}
}


void _centering_exec_2228_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		motor_ctrl_centering_stop();
		_centering_set_seq(0x222A, CENTERING_SEQ_TIMEOUT);
	}
//	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x222A, CENTERING_SEQ_TIMEOUT);
	}
}

void _centering_exec_222A_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_seq(0x2200, CENTERING_SEQ_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
	/* no error */
		_centering_set_seq(0x2200, CENTERING_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * centering close
 **********************************************************************/
/*********************************************************************//**
 * @brief centering control interrupt procedure (close sequence)
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_close_seq_proc(u32 flag)
{
	switch(ex_centering_task_seq & 0x00FF)
	{
	case 0x00:									/* seq2300 */
		_centering_close_2300_seq(flag);
		break;
	case 0x02:									/* seq2302 */
		_centering_close_2302_seq(flag);
		break;
	case 0x04:									/* seq2304 */
		_centering_close_2304_seq(flag);
		break;
	case 0x0A:
		_centering_close_230A_seq(flag);
		break;
	/* exec retry */
	default:									/* other */
		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 11);
		break;
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2300
 *  wait motor start
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_close_2300_seq(u32 flag)
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_close_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (SENSOR_CENTERING_HOME)
		{
		//すでにHomeの場合は、1回 Homeに入れる
			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_OUT_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_CLOSE_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_centering_set_seq(0x2304, CENTERING_SEQ_TIMEOUT);
			}
		}
		else
		{
		//現在Homeでない場合、
		//取り合えず、閉じて、その後開いて、再度閉じる
		//合計2回Homeに入れる

			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_IN_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_CLOSE_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_centering_set_seq(0x2302, CENTERING_SEQ_TIMEOUT);
			}
		}
	}
}




/*********************************************************************//**
 * @brief centering control sequence 0x2302
 *  wait motor stop
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_close_2302_seq(u32 flag)	/* Home待ち */
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_close_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_close_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
		/* ok Home outへ*/
		set_centering_timeout(1, CENTERING_SEQ_HOME_OUT_TIMEOUT);
		_centering_set_seq(0x2304, CENTERING_SEQ_TIMEOUT);
	}
}


/* Home OUT待ち*/
/* 条件 20msec以内のHome検知は無視 */
void _centering_close_2304_seq(u32 flag)
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_close_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		_centering_set_close_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
	/* エラー処理 シーケンスこのまま*/
	/* 40msec以下の瞬間Home INは無視してもう一度 シーケンスはこのまま */

	}
	//2021-05-27 else if (IS_CENTERING_EVT_HOME_OUT_INTR(flag))
	else if (IS_CENTERING_EVT_HOME_OUT_INTR_10MSEC(flag))	/* 2021-05-27 */
	{
	/* 主の処理 Home out */
	/* Home closeなので停止処理	*/
		motor_ctrl_centering_stop();
		_centering_set_seq(0x230A, CENTERING_SEQ_TIMEOUT);

	}
}


/* モータ停止待ち */
void _centering_close_230A_seq(u32 flag)//ok also use test moede
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_close_retry(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
	/* no error */
		if (SENSOR_CENTERING_HOME)
		{
		/* Homeなので、リトライ*/
			_centering_set_close_retry(ALARM_CODE_CENTERING_HOME_STOP);
		}
		else
		{
		/* Home外なので終了	*/

			ex_dline_testmode.test_result = TEST_RESULT_OK;

			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_CLOSE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
	}
}


/*********************************************************************//**
 * @brief centering control sub function
 *  set exec retry
 * @param[in]	alarm code
 * @return 		None
// リトライ回数確認処理
 **********************************************************************/
void _centering_set_close_retry(u32 alarm_code)//ok
{
	s_centering_alarm_retry++;
	if (s_centering_alarm_retry >= CENTERING_RETRY_COUNT)
	{
	/* retry over */
		_centering_set_alarm(alarm_code);
	}
	else
	{
		s_centering_alarm_code = alarm_code;
		
		if (!(is_motor_ctrl_centering_stop()))
		{
			motor_ctrl_centering_stop();
		}

		_centering_set_seq(0x2300, CENTERING_SEQ_TIMEOUT);

	}
}


/*********************************************************************//**
 * Centering motor free run
 **********************************************************************/
/*********************************************************************//**
 * @brief centering control interrupt procedure (free run sequence)
 * @param[in]	centering motor event flag
 * @return 		None
 **********************************************************************/
void _centering_freerun_seq_proc(u32 flag)		// 生産用のテストモード
{
	switch(ex_centering_task_seq & 0x00FF)
	{
	case 0x00:									/* seq2F00 */
		_centering_freerun_2f00_seq(flag);
		break;
	case 0x02:									/* seq2F02 */
		_centering_freerun_2f02_seq(flag);
		break;
	case 0x04:									/* seq2F04 */
		_centering_freerun_2f04_seq(flag);
		break;
	case 0x06:									/* seq2F06 */
		_centering_freerun_2f06_seq(flag);
		break;
	case 0x08:									/* seq2F08 */
		_centering_freerun_2f08_seq(flag);
		break;
	case 0x0A:									/* seq2F0A */
		_centering_freerun_2f0A_seq(flag);
		break;

	case 0x10:									/* seq2F10 */
		_centering_freerun_2f10_seq(flag);
		break;
	case 0x12:									/* seq2F12 */
		_centering_freerun_2f12_seq(flag);
		break;

	default:									/* other */
		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 12);
		break;
	}
}


void _centering_freerun_2f00_seq(u32 flag)
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (SENSOR_CENTERING_HOME)
		{
		//すでにHomeの場合は、1回 Homeに入れる
		#if 1
			_hal_status_led(DISP_COLOR_GREEN);
			_hal_status_led(DISP_COLOR_RED_OFF);
		#endif

			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_OUT_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
				}

				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;	//2018-11-28
				_centering_set_seq(0x2f04, CENTERING_SEQ_TIMEOUT);
			}
		}
		else
		{
		//現在Homeでない場合、
		//取り合えず、閉じて、その後開いて、再度閉じる
		//合計2回Homeに入れる

		#if 1
			_hal_status_led(DISP_COLOR_GREEN_OFF);
			_hal_status_led(DISP_COLOR_RED_OFF);
		#endif

			if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_SEQ_HOME_IN_TIMEOUT))
			{
				if (s_centering_alarm_retry == 0)
				{
				//リトライ時ではなく、最初の1回のみメッセージ送信
					_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
				}
				_centering_set_seq(0x2f02, CENTERING_SEQ_TIMEOUT);
			}
		}
	}
}


/* 動かす前にすでにHome outなので1回 Homeに入れる */
/* Home 戻し待ち*/
void _centering_freerun_2f02_seq(u32 flag)//ok also use test moede
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
		#if 1
		_hal_status_led(DISP_COLOR_GREEN);
		#endif

		ex_dline_testmode.test_start = 1;
		ex_dline_testmode.time_count = 0;	//2018-11-28
		set_centering_timeout(1, CENTERING_SEQ_HOME_OUT_TIMEOUT);
		_centering_set_seq(0x2f04, CENTERING_SEQ_TIMEOUT);
	}
}

/* Home OUT待ち*/
/* 条件 20msec以内のHome検知は無視 */
void _centering_freerun_2f04_seq(u32 flag)
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
	/* エラー処理 シーケンスこのまま*/
	/* 40msec以下の瞬間Home INは無視してもう一度 シーケンスはこのまま */

	}
//	else if(!SENSOR_CENTERING_HOME)
	else if (IS_CENTERING_EVT_HOME_OUT_INTR(flag))
	{
	/* 主の処理 Home out */
		#if 1
		_hal_status_led(DISP_COLOR_GREEN_OFF);
		#endif

		set_centering_timeout(1, CENTERING_SEQ_HOME_IN_TIMEOUT);
		_centering_set_seq(0x2f06, CENTERING_SEQ_TIMEOUT);
	}
}


/* Home Out中、Home 待ち*/
void _centering_freerun_2f06_seq(u32 flag)
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	{
		#if 1
		_hal_status_led(DISP_COLOR_GREEN);
		#endif

		if( ex_dline_testmode.test_start == 1 )
		{
			ex_dline_testmode.time1 = ex_dline_testmode.time_count;	//2018-11-28 時間を保存
			ex_dline_testmode.test_result = TEST_RESULT_OK;
		}
		set_centering_timeout(1, CENTERING_SEQ_STABLE_TIMEOUT);
		_centering_set_seq(0x2f08, CENTERING_SEQ_TIMEOUT);
	}
}

/* Home後 10msec待ち */
void _centering_freerun_2f08_seq(u32 flag)//ok also use test moede
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2f0A, CENTERING_SEQ_TIMEOUT);
	}
}

/* モータ停止待ち */
void _centering_freerun_2f0A_seq(u32 flag)//ok also use test moede
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
	/* no error */
		if (SENSOR_CENTERING_HOME)
		{

			ex_centering_count++;
		#if 1
			_hal_status_led(DISP_COLOR_GREEN);
		#endif
			_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_FREERUN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_centering_set_seq(CENTERING_SEQ_IDLE, 0);
		}
		else
		{
		// Homeに入ってないのでリトライする

		#if 1
			_hal_status_led(DISP_COLOR_GREEN_OFF);
		#endif

			_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_HOME_STOP);
		}
	}
}



/*********************************************************************//**
 * @brief centering control sub function
 *  set exec retry
 * @param[in]	alarm code
 * @return 		None
// リトライ回数確認処理
 **********************************************************************/
void _centering_set_exec_retry_freerun(u32 alarm_code)//ok
{
	s_centering_alarm_retry++;
	if (s_centering_alarm_retry >= CENTERING_RETRY_COUNT)
	{
	/* retry over */
		_centering_set_alarm(alarm_code);
	}
	else
	{
		s_centering_alarm_code = alarm_code;
		
		if (!(is_motor_ctrl_centering_stop()))
		{
			motor_ctrl_centering_stop();
		}
		_centering_set_seq(0x2f10, CENTERING_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief centering control sequence 0x2210
 *  wait motor start
 * @param[in]	centering motor event flag
 * @return 		None
//幅よせをHomeに戻した時に、Homeに戻らなかった場合、リトライ処理を行う必要がある
//ローラのかみ合わせの関係で一度、一定時間逆転させる必要がある
//モータ停止確認後の逆転動作
 **********************************************************************/
void _centering_freerun_2f10_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_exec_retry_freerun(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (IERR_CODE_OK == motor_ctrl_centering_rev(MOTOR_MAX_SPEED, 0, CENTERING_REV_TIME))  /* 200msec -> 50msec*/
	{
	//
		_centering_set_seq(0x2f12, CENTERING_SEQ_TIMEOUT); //600ms
	}
}


/*********************************************************************//**
 * @brief centering control sequence 0x2212
 *  centering revers
 * @param[in]	centering motor event flag
 * @return 		None
// リトライ処理の為、ローラかみ合わせの為の一定時間モータ逆転中
  **********************************************************************/
void _centering_freerun_2f12_seq(u32 flag)// ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* 本来の処理とちがうが、リトライさせる */
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2f00, CENTERING_SEQ_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2f00, CENTERING_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief centering control sub function
 *  set centering sequence
 * @param[in]	sequence no.
 * 				time out
 * @return 		None
 **********************************************************************/
void _centering_set_seq(u16 seq, u16 time_out)
{
	ex_centering_task_seq = seq;
	_ir_centering_ctrl_time_out = time_out;
	clr_flg(ID_CENTERING_CTRL_FLAG, ~EVT_ALL_BIT);
	
#if 1//#ifdef _ENABLE_JDL
	jdl_add_trace(ID_CENTERING_TASK, ((ex_centering_task_seq >> 8) & 0xFF), (ex_centering_task_seq & 0xFF), s_centering_alarm_code, s_centering_alarm_retry, 0);
#endif /* _ENABLE_JDL */

}


/*********************************************************************//**
 * @brief centering control sub function
 *  set centering waiteing sequence
 * @param[in]	sequence no.
 * @return 		None
 **********************************************************************/
void _centering_set_waiting_seq(u32 seq)
{
	if (ex_centering_task_seq == CENTERING_SEQ_FORCE_QUIT)
	{
		s_centering_task_wait_seq = seq;
	}
	else
	{
		s_centering_task_wait_seq = seq;
		
		motor_ctrl_centering_stop();
		_centering_set_seq(CENTERING_SEQ_FORCE_QUIT, CENTERING_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief centering control sub function
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _centering_set_alarm(u32 alarm_code)
{
	u32 msg;
	u32 sensor;
	
	motor_ctrl_centering_stop();
	
	s_centering_alarm_code = alarm_code;
	
	switch (ex_centering_task_seq & 0xFF00)
	{
	case CENTERING_SEQ_HOME:
		msg = TMSG_CENTERING_HOME_RSP;
		break;
	case CENTERING_SEQ_EXEC:
		msg = TMSG_CENTERING_EXEC_RSP;
		break;
	case CENTERING_SEQ_CLOSE:
		msg = TMSG_CENTERING_CLOSE_RSP;
		break;
	case CENTERING_SEQ_HOME_OUT_RETRY:
		msg = TMSG_CENTERING_HOME_OUT_RETRY_RSP;
		break;
	case CENTERING_SEQ_FREERUN:
		msg = TMSG_CENTERING_FREERUN_RSP;
		break;
	default:
		msg = TMSG_CENTERING_STATUS_INFO;
		break;
	}
	
	sensor = ex_position_sensor;

	/* Error message */
	_centering_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_ALARM, s_centering_alarm_code, ex_centering_task_seq, sensor);
	_centering_set_seq(CENTERING_SEQ_FORCE_QUIT, CENTERING_SEQ_TIMEOUT);
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
void _centering_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;
	
	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_CENTERING_TASK;
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
			_centering_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_centering_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _centering_system_error(u8 fatal_err, u8 code)
{

	if (fatal_err)
	{
//		_centering_send_msg(ID_ERRDISP_MBX, TMSG_ERRDISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}

	// fatal_err == 1の場合、致命的なエラーなのでエラーコードは出力しない、LED表示のみ通知で無限待ち
	_debug_system_error(ID_CENTERING_TASK, (u16)code, (u16)centering_msg.tmsg_code, (u16)centering_msg.arg1, fatal_err);
}


void _centering_home_out_retry_seq_proc(u32 flag)
{
	switch(ex_centering_task_seq & 0x00FF)
	{
	case 0x00:
		_centering_home_out_retry_2400_seq(flag);
		break;
	case 0x04:
		_centering_home_out_retry_2404_seq(flag);
		break;
	case 0x06:
		_centering_home_out_retry_2406_seq(flag);
		break;
	case 0x08:
		_centering_home_out_retry_2408_seq(flag);
		break;
	case 0x0A:
		_centering_home_out_retry_240A_seq(flag);
		break;

	default:									/* other */
		_centering_set_alarm(ALARM_CODE_CENTERING_FORCED_QUIT);
		
		/* system error ? */
		_centering_system_error(0, 10);
		break;
	}
}

/* Home outを抜けない問題なので、特殊処理 逆転動作から*/
void _centering_home_out_retry_2400_seq(u32 flag)//ok
{

	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		_centering_set_alarm(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else
	{
	/* 無条件で逆転から */
	/* Home outを抜けない問題なので、特殊処理 逆転動作から*/
		if (IERR_CODE_OK == motor_ctrl_centering_rev(MOTOR_MAX_SPEED, 0, CENTERING_REV_RETRY_TIME))/* 20msec 	*/
		{
			_centering_set_seq(0x2404, CENTERING_SEQ_TIMEOUT);
		}
	}
}

/* 逆転中*/
void _centering_home_out_retry_2404_seq(u32 flag)// ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* 本来の処理とちがうが、リトライさせる */
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2406, CENTERING_SEQ_TIMEOUT);
	}
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x2406, CENTERING_SEQ_TIMEOUT);
	}
}

void _centering_home_out_retry_2406_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_set_alarm(ALARM_CODE_CENTERING_TIMEOUT);
	}
	else if (is_motor_ctrl_centering_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_centering_fwd(MOTOR_MAX_SPEED, 1, CENTERING_FWD_RETRY_TIME))
		{
			_centering_set_seq(0x2408, CENTERING_SEQ_TIMEOUT);
		}
	}
}


void _centering_home_out_retry_2408_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
	/* time out */
		motor_ctrl_centering_stop();
		_centering_set_seq(0x240A, CENTERING_SEQ_TIMEOUT);
	}
//	else if (IS_CENTERING_EVT_HOME_INTR(flag))
	else if (IS_CENTERING_EVT_RUN_TIMEOUT(flag))
	{
		motor_ctrl_centering_stop();
		_centering_set_seq(0x240A, CENTERING_SEQ_TIMEOUT);
	}
}

void _centering_home_out_retry_240A_seq(u32 flag)//ok
{
	if (IS_CENTERING_EVT_TIMEOUT(flag))
	{
		_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_OUT_RETRY_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_centering_set_seq(CENTERING_SEQ_IDLE, 0);
	}
	else if (is_motor_ctrl_centering_stop())
	{
	/* no error */
		_centering_send_msg(ID_MAIN_MBX, TMSG_CENTERING_HOME_OUT_RETRY_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_centering_set_seq(CENTERING_SEQ_IDLE, 0);
	}
}





