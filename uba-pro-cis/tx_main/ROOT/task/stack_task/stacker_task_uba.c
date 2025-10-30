/******************************************************************************/
/*! @addtogroup Group2
    @file       stacker_task.c
    @brief      stacker task process
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
#include "sensor.h"
#include "status_tbl.h"
#include "pl/pl_evrec.h"

#include "systemdef.h"					//2022-02-17 test
#include "cyclonev_sysmgr_reg_def.h"	//2022-02-17 test
#include "hal_gpio_reg.h"				//2022-02-17 test

#if defined(UBA_RTQ)
#include "task/rc_task/rc_task.h"
#endif

#define EXT
#include "com_ram.c"


extern const MOTOR_LIMIT_STACKER_TABLE motor_limit_stacker_table[STACKER_AD_NUMBER];	/*2022-01-31 */

// motor_ctrl_stacker_fwd_uba_uba_uba_ubaの引数の意味合いが変わった
// UBAの時は、第3引数は使用していなかったが、本来の意味は1で、Home out後のパルスカウントを有効として
// パルスイベントを発生させるようにしていたが、Home outをスタッカータスクで検出して
// シーケンス内で、再度有効パルスを設定するようにした為、第3引数を使用しなくなった、
// CISでは第3引数は電流DA設定値となるため、同じ様に使用する


#define SENSOR_STACKER_HOME		SENSOR_PUSHER_HOME


/************************** PRIVATE DEFINITIONS *************************/
enum _STACKER_SEQ
{
	STACKER_SEQ_IDLE		= 0x1000,
	STACKER_SEQ_FORCE_QUIT	= 0x1001,
	STACKER_SEQ_HOME		= 0x1100,
	STACKER_SEQ_EXEC		= 0x1200,
	STACKER_SEQ_HALF		= 0x1300,
	STACKER_SEQ_PULL		= 0x1400,
	STACKER_SEQ_EXEC_NG_PULL	= 0x1600,// 1度目の押し込みNG 戻し動作
	STACKER_SEQ_EXEC_RETRY  = 0x1500,
#if defined(UBA_RTQ)	/* '21-03-01 */
	STACKER_SEQ_BACK		= 0x1700,
#endif
	STACKER_SEQ_FREERUN		= 0x1F00,
};


#define STACKER_SEQ_TIMEOUT						3000	/*time*/// use	/* 3sec *///シーケンスタイムアウト
#define STACKER_FREERUN_CHECK_TIME				1000	/*time*/// use	/* 1sec */
#define STACKER_SEQ_HOME_RECOVER_LAST_FEED		700	 	/*time*/	// use	700msec押し込み失敗でHome戻しの時すでにHomeの場合、Home検知が壊れている場合がある為、多めに回す
#define STACKER_FULL_TIME						350		/*time*//* use 350msec	Full検知用の時間リミット Normal Box *//* time */

#define STACKER_MOTOR_GEAR_LIMIT				660		//720×0.916 /*pulse*//* 720 pulse エラーの処理の切り分け用 これ以下はロックかタイムアウト、以上ギアエラー	*/
#define STACKER_HALF_POSITION_PULSE				36		//40×0.916 /*pulse*/// use	/* */// 半押し位置へのパルス数

#if defined(UBA_RTQ)
	#define STACKER_ALLOW_NEXT_PULSE		183		//200×0.916 /*pulse*/// RTQでは意味をなしていないので、同じにする
	#define STACKER_HOME_PULSE_RTQ			183		//200×0.916 /*pulse*/// RTQではHome戻しのHome検知後の念押しはパスルを使用、SSは時間
	#define STACKER_SEQ_HOME_LAST_FEED		350		// use	/* 200msec設定したいが起動までに150msec有している場合があるため200+150=350msecとする
	#define STACKER_HOMEOUT_PULSE			549		//600×0.916/*pulse*/// use	/* ハード的不良で常にHomeとなる時,700パルスでEUR20はだいたい抜ける、EUR500の場合800パルスは必要*/
	#define STACKER_HALF_POSITION_PULSE_RTQ	320		//350×0.916
#else
	#define STACKER_ALLOW_NEXT_PULSE		91		//100×0.916 /*pulse*/
	#define STACKER_SEQ_HOME_LAST_FEED		100		/*time*/// use	100msec /* UBA10,iPRO同等100msec */// スタッカをHomeに戻す時、Homeを検知してからの戻し時間
	#define STACKER_HOMEOUT_PULSE			641		//700×0.916/*pulse*/// use	/* ハード的不良で常にHomeとなる時,700パルスでEUR20はだいたい抜ける、EUR500の場合800パルスは必要*/
	#define STACKER_PULL_HALF_PULSE			715		//780×0.916 /*pulse*//* 頂点からの戻しで半押し位置になるはずの位置 */
	#define STACKER_BILL_IN_BOX				523		//570×0.916 /*pulse*//* 紙幣はBOX内に押し込めたはず*/
#endif

#define IS_STACKER_EVT_DRIVE_PULSE_OVER(x)	((x & EVT_STACKER_DRIVE_PULSE_OVER) == EVT_STACKER_DRIVE_PULSE_OVER)

#define IS_STACKER_EVT_TIMEOUT(x)			((x & EVT_STACKER_TIMEOUT) == EVT_STACKER_TIMEOUT)
#define IS_STACKER_EVT_MOTOR_LOCK(x)		((x & EVT_STACKER_MOTOR_LOCK) == EVT_STACKER_MOTOR_LOCK)


/************************** PRIVATE VARIABLES *************************/
static T_MSG_BASIC stacker_msg;
static u16 s_stacker_task_wait_seq;//use

static u8 s_stacker_freerun_dir;// use モータテストモードで使用

/************************** PRIVATE FUNCTIONS *************************/
void _stacker_initialize_proc(void);
void _stacker_idel_proc(void);
void _stacker_busy_proc(void);
void _stacker_msg_proc(void);
void _stacker_busy_msg_proc(void);//連続動作用に作成


/* Stacker idle (Stop Motor) sequence */
void _stacker_idle_seq_proc(u32 flag);
void _stacker_idle_1001_seq(u32 flag);

/* Stacker home sequence */// リトライ処理も共通 	// Reject処理は存在させない
void _stacker_home_seq_proc(u32 flag);//use
void _stacker_home_1100_seq(u32 flag);// ok use
void _stacker_home_1102_seq(u32 flag);// ok use
void _stacker_home_1104_seq(u32 flag);// ok use
#if defined(UBA_RTQ)	/* '21-03-01 */
void _stacker_home_1105_seq(u32 flag);
#endif
void _stacker_home_1106_seq(u32 flag);// ok use

void _stacker_home_1110_seq(u32 flag);// ok use
void _stacker_home_1116_seq(u32 flag);// ok use
void _stacker_home_1118_seq(u32 flag);// ok use

/* Stacker exec sequence */// リトライなし
void _stacker_exec_seq_proc(u32 flag); //use

void _stacker_exec_1200_seq_start(void); //use
void _stacker_exec_1200_seq(u32 flag); //use
void _stacker_exec_1204_seq(u32 flag); //use
void _stacker_exec_1206_seq(u32 flag); //use
void _stacker_exec_1208_seq(u32 flag); //use
void _stacker_exec_1210_seq(u32 flag); //use
void _stacker_exec_1212_seq(u32 flag); //use
#if defined(UBA_RTQ)	/* '21-03-01 */
void _stacker_exec_1213_seq(u32 flag); //use
#endif
void _stacker_exec_1214_seq(u32 flag); //use

void _stacker_set_retry_request(u32 alarm_code);	//use 押し込みエラーなのでMANIにReject(リトライ要望)を通知する処理


/* Stacker half sequence */
void _stacker_half_seq_proc(u32 flag);//use
void _stacker_half_1300_seq(u32 flag);//use
void _stacker_half_1302_seq(u32 flag);//use
void _stacker_half_1304_seq(u32 flag);//use
void _stacker_half_1306_seq(u32 flag);//use
void _stacker_half_1310_seq(u32 flag);//use
#if defined(UBA_RTQ)		/* '21-03-01 */
void _stacker_half_1311_seq(u32 flag);//use
#endif
void _stacker_half_1312_seq(u32 flag);//use

void _stacker_set_half_retry(u32 alarm_code);	//半押しエラー リトライ用


// 1回目押し込みOK時の戻し動作
void _stacker_pull_seq_proc(u32 flag);
void _stacker_exec_1400_seq_start(void); //use
void _stacker_exec_1400_seq(u32 flag); //use
void _stacker_exec_1409_seq(u32 flag); //use Home戻し
void _stacker_exec_140A_seq(u32 flag); //use
void _stacker_exec_140C_seq(u32 flag); //use
#if defined(UBA_RTQ)	/* '21-03-01 */
void _stacker_exec_140D_seq(u32 flag); //use
#endif
void _stacker_exec_140E_seq(u32 flag); //use
void _stacker_exec_1420_seq(u32 flag); //use
void _stacker_exec_1422_seq(u32 flag); //use
void _stacker_exec_1424_seq(u32 flag); //use
#if defined(UBA_RTQ)	/* '21-03-01 */
void _stacker_exec_1425_seq(u32 flag); //use
#endif
void _stacker_exec_1426_seq(u32 flag); //use
#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
void _stacker_exec_1430_seq(u32 flag); //use 半戻し
void _stacker_exec_1432_seq(u32 flag); //use
#endif

/* Stacker exec Retry sequence */
void _stacker_exec_retry_seq_proc(u32 flag); //use
void _stacker_exec_1500_seq(u32 flag); //use
void _stacker_exec_1504_seq(u32 flag); //use
void _stacker_exec_1506_seq(u32 flag); //use
void _stacker_exec_1508_seq(u32 flag); //use
void _stacker_exec_150A_seq(u32 flag); //use
void _stacker_exec_150C_seq(u32 flag); //use
#if defined(UBA_RTQ)	/* '21-03-01 */
void _stacker_exec_150D_seq(u32 flag); //use
#endif
void _stacker_exec_150E_seq(u32 flag); //use
void _stacker_exec_1520_seq(u32 flag); //use
void _stacker_exec_1522_seq(u32 flag); //use
void _stacker_exec_1524_seq(u32 flag); //use
#if defined(UBA_RTQ)	/* '21-03-01 */
void _stacker_exec_1525_seq(u32 flag); //use
#endif
void _stacker_exec_1526_seq(u32 flag); //use

// 押し込み動作のエラーリトライ用
void _stacker_check_to_push_count(u32 alarm_code);	//ok 押し込みエラー リトライ用 0x15


// 1回目押し込みNG時の戻し動作
void _stacker_exec_ng_pull_seq_proc(u32 flag);
void _stacker_exec_1600_seq(u32 flag);
void _stacker_exec_1609_seq(u32 flag);
void _stacker_exec_160A_seq(u32 flag);
void _stacker_exec_160C_seq(u32 flag);
#if defined(UBA_RTQ)
void _stacker_exec_160B_seq(u32 flag);// ok
#endif // UBA_RTQ

#if defined(UBA_RTQ)	/* '21-03-01 */
/* Stacker back sequence */
void _stacker_back_seq_proc(u32 flag);
void _stacker_back_1700_seq(u32 flag);
void _stacker_back_1702_seq(u32 flag);
void _stacker_back_1704_seq(u32 flag);
#endif

static u8 s_stacker_alarm_code;//use

#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
static u8  s_stacker_pull_half;//use
static u32 stacker_msg_arg1_back;
#endif

#define STACKER_HALF_RETRY_COUNT		3		//半押しリトライ
#define STACKER_EXEC_RETRY_PUSH_COUNT	2		//押し込みのエラー
static u8 s_stacker_retry;	// スタッカHome戻しのリトライ 0x11系,半押しのリトライカウンタ 0x13系 押しリトライカウンタ 0x15系 

// 戻り動作のエラーリトライ用
void _stacker_check_to_home_count(u32 alarm_code);	//ok Home戻しエラー リトライ用 //ok 0x11,0x12,0x13,0x14,0x15,0x16
#define STACKER_HOME_RETRY_COUNT		3		//Homeリトライ	0x11,0x13
#define STACKER_EXEC_RETRY_HOME_COUNT	2		//Homeリトライ	0x12,0x14,0x15,0x16
static u8 s_stacker_alarm_home_retry;			//戻りリトライカウンタ //ok 0x11,0x12,0x13,0x14,0x15,0x16

/*---------------------------------------------------------------------------*/


/* Stacker motor free run sequence */
void _stacker_freerun_seq_proc(u32 flag);
void _stacker_freerun_1f00_seq(u32 flag);
void _stacker_freerun_1f01_seq(u32 flag);

/* Stacker sub functions */
void _stacker_set_seq(u16 seq, u16 time_out);
void _stacker_select_seq(u32 seq);

#if defined(UBA_RTQ)	/* '18-05-01 */
void _stacker_set_cancel(void);
#endif

void _stacker_set_alarm(u32 alarm_code);
void _stacker_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _stacker_system_error(u8 fatal_err, u8 code);

/************************** EXTERN FUNCTIONS *************************/


/*********************************************************************//**
 * @ stacker task
 * @param[in]	extension information
 * @return 		None
 **********************************************************************/
void stacker_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	
	_stacker_initialize_proc();

	while(1)
	{
		if (ex_stacker_task_seq == STACKER_SEQ_IDLE)
		{
		/* idle */
			ercd = trcv_mbx(ID_STACKER_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
			if (ercd == E_OK)
			{
				memcpy(&stacker_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(stacker_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_stacker_system_error(1, 3);
				}
				_stacker_idel_proc();
				_stacker_msg_proc();
			}
			else
			{
				_stacker_idel_proc();
			}
		}
		else
		{
		/* busy */
			_stacker_busy_proc();
			ercd = prcv_mbx(ID_STACKER_MBX, (T_MSG **)&tmsg_pt);
			if (ercd == E_OK)
			{
				memcpy(&stacker_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(stacker_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_stacker_system_error(1, 4);
				}
#if 1//連続動作用に変更
				_stacker_busy_msg_proc();
#else
				_stacker_msg_proc();
#endif
			}
		}
	}
}


/*********************************************************************//**
 * @brief initialize stacker task
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _stacker_initialize_proc(void)
{

//	ex_cheat_occurred = 0;	/* 識別のポジション化に対応してないので無効 */

	ex_stacker_task_seq = STACKER_SEQ_IDLE;
	s_stacker_task_wait_seq = STACKER_SEQ_IDLE;
	
	clr_flg(ID_STACKER_CTRL_FLAG, ~EVT_ALL_BIT);
}


/*********************************************************************//**
 * @brief stacker task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _stacker_idel_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;


	ercd = pol_flg(ID_STACKER_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag);
	if (ercd != E_OK)
	{
		flag = 0;
	}
}


/*********************************************************************//**
 * @brief stacker task busy procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _stacker_busy_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;
	
	ercd = twai_flg(ID_STACKER_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TASK_WAIT_TIME);
	if (ercd != E_OK)
	{
		flag = 0;
	}
	
	switch (ex_stacker_task_seq & 0xFF00)
	{
	case STACKER_SEQ_IDLE:
		_stacker_idle_seq_proc(flag);
		break;
	case STACKER_SEQ_HOME:
		_stacker_home_seq_proc(flag);	// ok
		break;
	case STACKER_SEQ_EXEC://12XX
		_stacker_exec_seq_proc(flag);
		break;

	case STACKER_SEQ_HALF://13XX
		_stacker_half_seq_proc(flag);	// ok
		break;

	case STACKER_SEQ_PULL://14XX
		_stacker_pull_seq_proc(flag);
		break;

	case STACKER_SEQ_EXEC_RETRY://15XX
		_stacker_exec_retry_seq_proc(flag);
		break;

	case STACKER_SEQ_EXEC_NG_PULL:	//16XX // 1度目の押し込みNG 戻し動作
		_stacker_exec_ng_pull_seq_proc(flag);
		break;

#if defined(UBA_RTQ)	/* '21-03-01 */
	case STACKER_SEQ_BACK:
		_stacker_back_seq_proc(flag);
		break;
#endif

	case STACKER_SEQ_FREERUN:
		_stacker_freerun_seq_proc(flag);
		break;
	default:
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 6);
		break;
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  stacker task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _stacker_msg_proc(void)
{
	switch (stacker_msg.tmsg_code)
	{
	case TMSG_STACKER_HOME_REQ:						/* HOME message */
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;

	#if !defined(UBA_RTQ)//#if defined(NEW_STACK)	/* 2020-01-20 */
		_stacker_select_seq(STACKER_SEQ_HOME);
	#else
		if( stacker_msg.arg1 != 0 )
		{
		/* イニシャル動作からのメッセージ and 押しメカが押し込み中だった	*/
		/* 状況により、押しメカがHomeにない場合押し込みを行う				*/
			_stacker_select_seq(0x1110);
		}
		else
		{
		/* 通常動作	*/
			_stacker_select_seq(STACKER_SEQ_HOME);
		}
	#endif

		break;
	case TMSG_STACKER_EXEC_REQ:						/* EXEC message */
		// この処理はリトライ動作は行わない
#ifdef _DEBUG_STACKER_MOTOR_PULSE		/* '13-12-03 */
		_debug_stacker_motor_time = 0;
		_debug_stacker_motor_pulse = 0;
		_debug_stacker_motor_unhome_pulse = 0;
		_debug_stacker_motor_home_pulse = 0;
		_debug_stacker_motor_pulse_time = (u8 *)ex_bar.bar_buffer;
		memset(_debug_stacker_motor_pulse_time, 0, sizeof(ex_bar.bar_buffer));

#endif /* _DEBUG_STACKER_MOTOR_PULSE */
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;

#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg.arg1 != 0 )
		{
			stacker_msg_arg1_back = stacker_msg.arg1;
		}
		else
		{
			stacker_msg_arg1_back = 0;
		}
#endif

		_stacker_select_seq(STACKER_SEQ_EXEC);
		if( ex_stacker_task_seq == STACKER_SEQ_EXEC )
		{
			_stacker_exec_1200_seq_start();
		}
		break;

	case TMSG_STACKER_HALF_REQ:						/* HALF message */
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;
		_stacker_select_seq(STACKER_SEQ_HALF);
		break;

	case TMSG_STACKER_PULL_REQ:		// 押し込みOKの戻し動作
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;
#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg.arg1 != 0 )
		{
			stacker_msg_arg1_back = stacker_msg.arg1;
		}
		else
		{
			stacker_msg_arg1_back = 0;
		}
#endif

		_stacker_select_seq(STACKER_SEQ_PULL);

		if( ex_stacker_task_seq == STACKER_SEQ_PULL )
		{
			_stacker_exec_1400_seq_start();
		}
		break;

	case TMSG_STACKER_EXEC_NG_PULL_REQ:// 押し込みNGの戻し動作
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;
		_stacker_select_seq(STACKER_SEQ_EXEC_NG_PULL);
		break;

	case TMSG_STACKER_EXEC_RE_REQ:
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;

#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg.arg1 != 0 )
		{
			stacker_msg_arg1_back = stacker_msg.arg1;
		}
		else
		{
			stacker_msg_arg1_back = 0;
		}
#endif

		_stacker_select_seq(STACKER_SEQ_EXEC_RETRY);
		break;

#if defined(UBA_RTQ)	/* '21-03-01 */
	case TMSG_STACKER_BACK_REQ:
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		_stacker_select_seq(STACKER_SEQ_BACK);
		break;
#endif

	case TMSG_STACKER_FREERUN_REQ:					/* FREERUN message */
		s_stacker_alarm_code = 0;
		if (stacker_msg.arg1 == MOTOR_STOP)
		{
			_stacker_select_seq(STACKER_SEQ_FREERUN);
		}
		else
		{
			s_stacker_freerun_dir = stacker_msg.arg1;
			_stacker_select_seq(STACKER_SEQ_FREERUN);
		}
		break;
	default:					/* other */
		if (ex_stacker_task_seq != STACKER_SEQ_IDLE)
		{
			_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		}
		
		/* system error ? */
		_stacker_system_error(0, 7);
		break;
	}
}

void _stacker_busy_msg_proc(void)
{
	switch (stacker_msg.tmsg_code)
	{
	case TMSG_STACKER_HOME_REQ:						/* HOME message */
	// 連続動作の為処理を変える
		switch (ex_stacker_task_seq & 0xFF00)
		{
			case STACKER_SEQ_PULL://14XX
			// Home戻り動作中はモードを変えない

				break;
			default:
				s_stacker_alarm_code = 0;
				s_stacker_retry = 0;
				s_stacker_alarm_home_retry = 0;
				_stacker_select_seq(STACKER_SEQ_HOME);
				break;
		}
		break;
	case TMSG_STACKER_EXEC_REQ:						/* EXEC message */
		// この処理はリトライ動作は行わない
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;

#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg.arg1 != 0 )
		{
			stacker_msg_arg1_back = stacker_msg.arg1;
		}
		else
		{
			stacker_msg_arg1_back = 0;
		}
#endif

		_stacker_select_seq(STACKER_SEQ_EXEC);
		break;
	case TMSG_STACKER_HALF_REQ:						/* HALF message */
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;
		_stacker_select_seq(STACKER_SEQ_HALF);
		break;

	case TMSG_STACKER_PULL_REQ:			// 押し込みOKの戻し動作
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;

#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg.arg1 != 0 )
		{
			stacker_msg_arg1_back = stacker_msg.arg1;
		}
		else
		{
			stacker_msg_arg1_back = 0;
		}
#endif
		_stacker_select_seq(STACKER_SEQ_PULL);
		break;


	case TMSG_STACKER_EXEC_NG_PULL_REQ:	// 押し込みNGの戻し動作
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;
		_stacker_select_seq(STACKER_SEQ_EXEC_NG_PULL);
		break;

	case TMSG_STACKER_EXEC_RE_REQ:
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		s_stacker_alarm_home_retry = 0;
#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg.arg1 != 0 )
		{
			stacker_msg_arg1_back = stacker_msg.arg1;
		}
		else
		{
			stacker_msg_arg1_back = 0;
		}
#endif
		_stacker_select_seq(STACKER_SEQ_EXEC_RETRY);
		break;

#if defined(UBA_RTQ)	/* '21-03-01 */
	case TMSG_STACKER_BACK_REQ:
		s_stacker_alarm_code = 0;
		s_stacker_retry = 0;
		_stacker_select_seq(STACKER_SEQ_BACK);
		break;
#endif

	case TMSG_STACKER_FREERUN_REQ:					/* FREERUN message */
		s_stacker_alarm_code = 0;
		if (stacker_msg.arg1 == MOTOR_STOP)
		{
			_stacker_select_seq(STACKER_SEQ_FREERUN);
		}
		else
		{
			s_stacker_freerun_dir = stacker_msg.arg1;
			_stacker_select_seq(STACKER_SEQ_FREERUN);
		}
		break;
	default:					/* other */
		if (ex_stacker_task_seq != STACKER_SEQ_IDLE)
		{
			_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		}
		
		/* system error ? */
		_stacker_system_error(0, 7);
		break;
	}
}



/*********************************************************************//**
 * idle
 **********************************************************************/
/*********************************************************************//**
 * @brief stacker control interrupt procedure (idle sequence)
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_idle_seq_proc(u32 flag)
{
	switch (ex_stacker_task_seq & 0x00FF)
	{
	case 0x01:
		_stacker_idle_1001_seq(flag);
		break;
	default:
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1001
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_idle_1001_seq(u32 flag)
{
	if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
#if defined(_DEBUG_STACKER_MOTOR_PULSE)
		_debug_stacker_motor_flag = 0;	//計測終了
#endif
		if (s_stacker_task_wait_seq == STACKER_SEQ_IDLE)
		{
		/* No waiting process */
			_stacker_set_seq(s_stacker_task_wait_seq, 0);
		}
		else if (s_stacker_task_wait_seq == STACKER_SEQ_FREERUN)
		{
		/* Wait motor stop for Freerun */
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_FREERUN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
		/* Set waiting process */
			_stacker_set_seq(s_stacker_task_wait_seq, STACKER_SEQ_TIMEOUT);
		}
		s_stacker_task_wait_seq = STACKER_SEQ_IDLE;
	}
}


/*********************************************************************//**
 * staker home
 **********************************************************************/
/*********************************************************************//**
 * @brief stacker control interrupt procedure (home sequence)
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_home_seq_proc(u32 flag)	// Reject処理は存在させない
{
	switch(ex_stacker_task_seq & 0x00FF)
	{
	case 0x00:									/* seq1100 */
		_stacker_home_1100_seq(flag);
		break;
	case 0x02:									/* seq1102 */
		_stacker_home_1102_seq(flag);
		break;
	case 0x04:									/* seq1104 */
		_stacker_home_1104_seq(flag);
		break;
#if defined(UBA_RTQ)	/* '21-03-01 */
	case 0x05:									/* seq1105 */
		_stacker_home_1105_seq(flag);
		break;
#endif
	case 0x06:									/* seq1106 */
		_stacker_home_1106_seq(flag);
		break;

	/* パワーリカバリ時の押し込み動作からFWD	*/
	case 0x10:									/* seq1110 */
		_stacker_home_1110_seq(flag);
		break;
	case 0x16:									/* seq1112 */
		_stacker_home_1116_seq(flag);
		break;
	case 0x18:									/* seq1114 */
		_stacker_home_1118_seq(flag);
		break;





	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);

		/* system error ? */
		_stacker_system_error(0, 10);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1100
 *  wait motor start
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_home_1100_seq(u32 flag)
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if(is_ld_mode())
	{
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
	else
	{
		if (!(is_box_set()))
		{
			_stacker_set_alarm(ALARM_CODE_BOX);
		}
		else if (IS_STACKER_EVT_TIMEOUT(flag))
		{
		/* time out */
			_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
		}
		else if (is_motor_ctrl_stacker_stop())
		{

			//ex_u32_free_buffer1[1] = ex_recovery_info.step;

			if (SENSOR_STACKER_HOME)
			{
				//  すでにHomeにあるので、かるく戻す処理へ
				//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_HOME_PULSE_RTQ, UBA_STACKER_AD_REV))
				if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_HOME_PULSE_RTQ, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
				{
#if defined(UBA_RTQ)
					ex_dline_testmode.test_start = 1;
					ex_dline_testmode.time_count = 0;
#endif // UBA_RTQ
					_stacker_set_seq(0x1105, STACKER_SEQ_TIMEOUT);
				}
			}
			else
			{
			// Homeにない場合、次のシーケンスで
				//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
				if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
				{
#if defined(UBA_RTQ)
					ex_dline_testmode.test_start = 1;
					ex_dline_testmode.time_count = 0;
#endif // UBA_RTQ
					_stacker_set_seq(0x1102, STACKER_SEQ_TIMEOUT);
				}
			}
		}
		else if(is_rc_error())
		{
			/* モータ停止して待機状態に戻る */
			_stacker_set_cancel();
		}
	}
#else
	u16 temp = 0;

	if(is_ld_mode())
	{
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
	else
	{
		if (!(is_box_set()))
		{
			_stacker_set_alarm(ALARM_CODE_BOX);
		}
		else if (IS_STACKER_EVT_TIMEOUT(flag))
		{
		/* time out */
			_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
		}
		else if (is_motor_ctrl_stacker_stop())
		{
			//ex_u32_free_buffer1[1] = ex_recovery_info.step;

			if (SENSOR_STACKER_HOME)
			{
				//  すでにHomeにあるので、かるく戻す処理へ	
				if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
				{
					_stacker_set_seq(0x1104, STACKER_SEQ_HOME_LAST_FEED);
				}
			}
			else
			{
				if(
					( ex_recovery_info.step == RECOVERY_STEP_VEND ) || 
					( ex_recovery_info.step == RECOVERY_STEP_STACKING ) ||
					( ex_recovery_info.step == RECOVERY_STEP_STACKING_BILL_IN_BOX )
				)
				{

					if( STACKER_BILL_IN_BOX <= ex_recovery_info.back_fwd_pulse )
					{
					/* 十分押し込めているので戻し*/
						if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
						{
							_stacker_set_seq(0x1102, STACKER_SEQ_TIMEOUT);
						}
					}
					else
					{
					/* 押し込みが足りていないので押し込み */
						temp = STACKER_BILL_IN_BOX - ex_recovery_info.back_fwd_pulse;

						if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, (temp), motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))	// 現在位置からのカウントでイベント発生
						{
						// 押し込み開始
							_stacker_set_seq(0x1116, STACKER_SEQ_TIMEOUT);
						}
					}
				}
				else
				{
				/* 通常時なのでHome戻し*/
					if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
					{
						//2023-03-02 500の場合は0x14xxで動かしていたが、今回はこっちでとりあえず対応
						ex_dline_testmode.test_start = 1;
						ex_dline_testmode.time_count = 0;

						_stacker_set_seq(0x1102, STACKER_SEQ_TIMEOUT);
					}
				}
			}
		}
	}
#endif
}



/*********************************************************************//**
 * @brief stacker control sequence 0x1101
 *  move home position (revers)
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカ逆転中
// 押しメカHome検知待ち
 **********************************************************************/
void _stacker_home_1102_seq(u32 flag)
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
//      時間指定にするか、パルス指定にするか未定
// Home検知後のだめおし
		_stacker_set_seq(0x1105, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);	//ALARM_CODE_STACKER_TIMEOUT
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);		//ALARM_CODE_STACKER_GEAR
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
		if( ex_dline_testmode.test_start == 1 )
		{
			ex_dline_testmode.time2 = ex_dline_testmode.time_count;			/* 時間を保存	*/
			ex_dline_testmode.test_start = 0;
			ex_dline_testmode.test_result = TEST_RESULT_OK;
		}

//      時間指定にするか、パルス指定にするか未定
// Home検知後のだめおし
		_stacker_set_seq(0x1104, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1224
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_home_1104_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1106, STACKER_SEQ_TIMEOUT);	
	}
#if defined(UBA_RTQ)		/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence 0x1105
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_home_1105_seq(u32 flag)// ok
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
#if defined(UBA_RTQ)
		if( ex_dline_testmode.test_start == 1 )
		{			/* 時間を保存	*/
			ex_dline_testmode.time2 = ex_dline_testmode.time_count;	/* 時間を保存	*/
			ex_dline_testmode.test_start = 0;
			if (ex_dline_testmode.time1 != 0)
			{
				ex_dline_testmode.test_result = TEST_RESULT_OK;
			}
		}
#endif // UBA_RTQ
		_stacker_set_seq(0x1106, STACKER_SEQ_TIMEOUT);	
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif


/*********************************************************************//**
 * @brief stacker control sequence 0x1226
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押し完了
// モータ停止待ち
 **********************************************************************/
void _stacker_home_1106_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
	// Home戻し成功
		if(SENSOR_STACKER_HOME)
		{
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
			_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// NG 2024-05-08
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
	// Home戻し成功
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
#endif
}


void _stacker_home_1110_seq(u32 flag)	//not use
{

	u16 temp = 0;


	if(is_ld_mode())
	{
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_HOME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
	else
	{
		if (!(is_box_set()))
		{
			_stacker_set_alarm(ALARM_CODE_BOX);
		}
		else if (IS_STACKER_EVT_TIMEOUT(flag))
		{
		/* time out */
			_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
		}
		else if (is_motor_ctrl_stacker_stop())
		{
			if (SENSOR_STACKER_HOME)
			{
				_stacker_set_seq(0x1100, STACKER_SEQ_TIMEOUT);
			}
			else
			{
	#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
				if( STACKER_BILL_IN_BOX <= ex_recovery_info.back_fwd_pulse )
				{
				/* 十分押し込めているので戻し*/
					_stacker_set_seq(0x1100, STACKER_SEQ_TIMEOUT);
				}
				else
				{
				/* 押し込みが足りていないので押し込み */
					temp = STACKER_BILL_IN_BOX - ex_recovery_info.back_fwd_pulse;

					if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, (temp), motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))	// 現在位置からのカウントでイベント発生				
					{
					// 押し込み開始
						_stacker_set_seq(0x1116, STACKER_SEQ_TIMEOUT);
					}
				}
	#else
				if(STACKER_FULL_DA < ex_recovery_info.back_fwd_pulse)
				{
					temp = 3;
				}
				else
				{
				//2025-06-30	temp = STACKER_FULL_DA - ex_recovery_info.back_fwd_pulse;
					temp = 3;
				}
				//if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, (temp), 0))	// 現在位置からのカウントでイベント発生
				if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, (temp), motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))	// 現在位置からのカウントでイベント発生
				{
				// 押し込み開始
		// Homeを外れてから、正式にFullのカウントを行う
					_ir_stacker_motor_ctrl.event_pulse_up = 0;
					_stacker_set_seq(0x1116, STACKER_SEQ_TIMEOUT);
				}
	#endif
			}
		}
#if defined(UBA_RTQ)		/* '19-03-25 */
		else if(is_rc_error())
		{
			/* モータ停止して待機状態に戻る */
			_stacker_set_cancel();
		}
#endif
	}
}

void _stacker_home_1116_seq(u32 flag)//ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* エラーにせずに戻し動作へ	*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1118, STACKER_SEQ_TIMEOUT);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* エラーにせずに戻し動作へ	*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1118, STACKER_SEQ_TIMEOUT);
	}
	else if((SENSOR_STACKER_HOME))
	{
	// なぜかHomeにあるので
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1118, STACKER_SEQ_TIMEOUT);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// 規定パルスは押し込めた
		//next step
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1118, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}



void _stacker_home_1118_seq(u32 flag)
{

	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */// リトライ可能であれば、押しメカ戻し処理へ
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// next step
		_stacker_set_seq(0x1100, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}

/*********************************************************************//**
 * staker exec
 **********************************************************************/
/*********************************************************************//**
 * @brief stacker control interrupt procedure (exec sequence)
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_exec_seq_proc(u32 flag)
{
// リトライ処理は行わない、1発エラー
	switch(ex_stacker_task_seq & 0x00FF)
	{
	case 0x00:									/* seq1200 */
		_stacker_exec_1200_seq(flag);
		break;
	case 0x04:									/* seq1204 */
		_stacker_exec_1204_seq(flag);
		break;
	case 0x06:									/* seq1206 */
		_stacker_exec_1206_seq(flag);
		break;
	case 0x08:									/* seq1208 */
		_stacker_exec_1208_seq(flag);			// stacker top
		break;
	case 0x10:									/* seq1210 */// 押し込み開始前にStakerがHome外の時
		_stacker_exec_1210_seq(flag);			// pull befor push
		break;
	case 0x12:									/* seq1212 */
		_stacker_exec_1212_seq(flag);			// pull
		break;
#if defined(UBA_RTQ)	/* '21-03-01 */
	case 0x13:									/* seq1213 */
		_stacker_exec_1213_seq(flag);			// pull
		break;
#endif
	case 0x14:									/* seq1214 */
		_stacker_exec_1214_seq(flag);			// pull
		break;

	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 11);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1200
 *  wait motor start
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカ位置確認
**********************************************************************/
void _stacker_exec_1200_seq_start(void)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	// 通常はあり得ないのでこの処理はコメントアウトしておく
	else if (!(SENSOR_STACKER_HOME))
	{
	// Not Home
	// Homeへ戻す処理へ
		//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		{
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1210, STACKER_SEQ_TIMEOUT);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
	else
	{
		//if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, 0))		// 現在位置からのカウントでイベント発生
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))		// 現在位置からのカウントでイベント発生
		{
		//モータ起動を通知
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
		// 押し込み開始


			ex_dline_testmode.time_count = 0;
			ex_dline_testmode.test_start = 1;


		// Homeを外れてから、正式にFullのカウントを行う
			_ir_stacker_motor_ctrl.event_pulse_up = 0;
			ex_recovery_info.back_fwd_pulse = 0;

#if defined(_DEBUG_STACKER_MOTOR_PULSE)
			// Homeを外れた位置から押し込みパルスを指定
			_debug_stacker_motor_flag = 1;	//計測有効
#endif
			// この前のシーケンスのmotor_ctrl_stacker_fwd_uba_uba_uba_ubaの第3引数が1に設定されているので、エンコーダ割り込みの関数で、Home外となった時に、パルス計測のカウンタを再設定しているので、
			// ここで設定する必要なし
			motor_ctrl_stacker_set_full_check(STACKER_TOP_PULSE, 1);//USA 998パルス、EUR1119パスル Fullチェック有効
			_stacker_set_seq(0x1206, STACKER_SEQ_TIMEOUT);
		}
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	// 通常はあり得ないのでこの処理はコメントアウトしておく
	else if (!(SENSOR_STACKER_HOME))
	{
	// Not Home
	// Homeへ戻す処理へ
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		{
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1210, STACKER_SEQ_TIMEOUT);
		}
	}
	else
	{
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, STACKER_HOMEOUT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))		// 現在位置からのカウントでイベント発生
		{
		//モータ起動を通知
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
		// 押し込み開始

			ex_dline_testmode.time_count = 0;
			ex_dline_testmode.test_start = 1;

			_stacker_set_seq(0x1204, STACKER_SEQ_TIMEOUT);
		}
	}
#endif
}


void _stacker_exec_1200_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// 起動処理なので1発でエラーとする

		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	// 通常はあり得ないのでこの処理はコメントアウトしておく
	else if (!(SENSOR_STACKER_HOME))
	{
	// Not Home
	// Homeへ戻す処理へ
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		{
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1210, STACKER_SEQ_TIMEOUT);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
	else
	{
		// 第3引数が1なので、Home外になった後で、イベントパルスが再設定される
		//if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, 0))		// 現在位置からのカウントでイベント発生
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))		// 現在位置からのカウントでイベント発生
		{
		//モータ起動を通知
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
		// 押し込み開始

			// ex_dline_testmode.time_count = 0;
			// ex_dline_testmode.test_start = 1;


		// Homeを外れてから、正式にFullのカウントを行う
			_ir_stacker_motor_ctrl.event_pulse_up = 0;
			// ex_recovery_info.back_fwd_pulse = 0;

#if defined(_DEBUG_STACKER_MOTOR_PULSE)
			// Homeを外れた位置から押し込みパルスを指定
			_debug_stacker_motor_flag = 1;	//計測有効
#endif
			// この前のシーケンスのmotor_ctrl_stacker_fwd_uba_uba_uba_ubaの第3引数が1に設定されているので、エンコーダ割り込みの関数で、Home外となった時に、パルス計測のカウンタを再設定しているので、
			// ここで設定する必要なし
			motor_ctrl_stacker_set_full_check(STACKER_TOP_PULSE, 1);//USA 998パルス、EUR1119パスル Fullチェック有効
			_stacker_set_seq(0x1206, STACKER_SEQ_TIMEOUT);
		}
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// 起動処理なので1発でエラーとする

		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	// 通常はあり得ないのでこの処理はコメントアウトしておく
	else if (!(SENSOR_STACKER_HOME))
	{
	// Not Home
	// Homeへ戻す処理へ
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		{
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1210, STACKER_SEQ_TIMEOUT);
		}
	}
	else
	{
		// 第3引数が1なので、Home外になった後で、イベントパルスが再設定される
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, STACKER_HOMEOUT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))		// 現在位置からのカウントでイベント発生
		{
		//モータ起動を通知
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);
		// 押し込み開始

			ex_dline_testmode.time_count = 0;
			ex_dline_testmode.test_start = 1;


			_stacker_set_seq(0x1204, STACKER_SEQ_TIMEOUT);
		}
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1201
 *  deviate home position
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカHome外れ待ち
 **********************************************************************/
void _stacker_exec_1204_seq(u32 flag)//ok
{

	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */// リトライ可能であれば、押しメカ戻し処理へ
		_stacker_set_retry_request(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse < STACKER_MOTOR_GEAR_LIMIT)
		{
		/* time out */
			_stacker_set_retry_request(ALARM_CODE_STACKER_TIMEOUT);
		}
		else
		{
		/* gear */
			_stacker_set_retry_request(ALARM_CODE_STACKER_GEAR);
		}
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		/* time out */
		_stacker_set_retry_request(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// Homeを外れてから、正式にFullのカウントを行う
		_ir_stacker_motor_ctrl.event_pulse_up = 0;

#if defined(_DEBUG_STACKER_MOTOR_PULSE)
		// Homeを外れた位置から押し込みパルスを指定
		_debug_stacker_motor_flag = 1;	//計測有効
#endif
		// この前のシーケンスのmotor_ctrl_stacker_fwd_uba_uba_uba_ubaの第3引数が1に設定されているので、エンコーダ割り込みの関数で、Home外となった時に、パルス計測のカウンタを再設定しているので、
		// ここで設定する必要なし
		motor_ctrl_stacker_set_full_check(STACKER_TOP_PULSE, 1);//USA 998パルス、EUR1119パスル Fullチェック有効
		_stacker_set_seq(0x1206, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1202
 *  push stacker
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカ頂点待ち
 **********************************************************************/
void _stacker_exec_1206_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_set_retry_request(ALARM_CODE_STACKER_FULL);
		}
		else
		{
		/* motor lock */
			_stacker_set_retry_request(ALARM_CODE_STACKER_MOTOR_LOCK);
		}
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_set_retry_request(ALARM_CODE_STACKER_FULL);
		}
		else
		{
			/* time out */
			_stacker_set_retry_request(ALARM_CODE_STACKER_TIMEOUT);
		}

	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	// 識別センサがONしているのでCheat
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// 規定パルスは押し込めた
		//next step
#if defined(_DEBUG_STACKER_MOTOR_PULSE)
		_debug_stacker_motor_flag = 0;	//計測終了
#endif
		motor_ctrl_stacker_stop();


		if( ex_dline_testmode.test_start == 1 )
		{			/* 時間を保存	*/
			ex_dline_testmode.time1 = ex_dline_testmode.time_count;	/* 時間を保存	*/
			ex_dline_testmode.test_start = 0;
		}


		_stacker_set_seq(0x1208, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
		if( stacker_msg_arg1_back == SS_BILL_STACK)
		{
			if( _ir_stacker_motor_ctrl.event_pulse_up == STACKER_BILL_IN_BOX )
			{
				set_recovery_step(RECOVERY_STEP_STACKING_BILL_IN_BOX);
			}
		}
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_set_retry_request(ALARM_CODE_STACKER_FULL);
		}
		else
		{
		/* motor lock */
			_stacker_set_retry_request(ALARM_CODE_STACKER_MOTOR_LOCK);
		}
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if( stacker_msg_arg1_back == SS_BILL_STACK)
		{
			if( _ir_stacker_motor_ctrl.event_pulse_up == STACKER_BILL_IN_BOX )
			{
				set_recovery_step(RECOVERY_STEP_STACKING_BILL_IN_BOX);
			}
		}
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_set_retry_request(ALARM_CODE_STACKER_FULL);
		}
		else
		{
			/* time out */
			_stacker_set_retry_request(ALARM_CODE_STACKER_TIMEOUT);
		}

	}
	else if((SENSOR_STACKER_HOME))
	{
	// なぜかHomeにあるのでエラー
		_stacker_set_retry_request(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// 規定パルスは押し込めた
		//next step
		if( stacker_msg_arg1_back == SS_BILL_STACK)
		{
			set_recovery_step(RECOVERY_STEP_STACKING_BILL_IN_BOX);
		}
	#if defined(_DEBUG_STACKER_MOTOR_PULSE)
		_debug_stacker_motor_flag = 0;	//計測終了
	#endif
		motor_ctrl_stacker_stop();

		ex_stacker_count++;
		if( ex_dline_testmode.test_start == 1 )
		{			/* 時間を保存	*/
			ex_dline_testmode.time1 = ex_dline_testmode.time_count;	/* 時間を保存	*/
			ex_dline_testmode.test_start = 0;
		}
		_stacker_set_seq(0x1208, STACKER_SEQ_TIMEOUT);
	}

#endif
}


/*********************************************************************//**
 *
 *
 *
 *
// 	ここでFullか判断する
// 押しメカ頂点待ち後、モータ停止待ち
 **********************************************************************/
void _stacker_exec_1208_seq(u32 flag)
{

	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */// リトライ可能であれば、押しメカ戻し処理へ
		_stacker_set_retry_request(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse < STACKER_MOTOR_GEAR_LIMIT)
		{
		/* time out */
			_stacker_set_retry_request(ALARM_CODE_STACKER_TIMEOUT);
		}
		else
		{
		/* gear */
			_stacker_set_retry_request(ALARM_CODE_STACKER_GEAR);
		}
	}
	else if (SENSOR_STACKER_HOME)
	{
	//保留
//		_stacker_set_seq(0x1212, STACKER_SEQ_TIMEOUT);
		_stacker_set_retry_request(ALARM_CODE_STACKER_GEAR);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// next step
	// Full確認 頂点手前150パルスでの経過時間リミット
		if( _ir_stacker_motor_ctrl.peakload_time  >= STACKER_FULL_TIME )
		{
			_stacker_set_retry_request(ALARM_CODE_STACKER_FULL);
		}
		else
		{
			// 押し込み成功
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 押し込み前のHome戻し
 **********************************************************************/
void _stacker_exec_1210_seq(u32 flag)//ok use
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (SENSOR_STACKER_HOME)
	{
		// next step
		// Home検知後のだめおし
		_stacker_set_seq(0x1213, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
//      時間指定にするか、パルス指定にするか未定
		_stacker_set_seq(0x1212, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 押し込み前のHome戻し
 **********************************************************************/
void _stacker_exec_1212_seq(u32 flag)// ok use
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1214, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}

#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence 0x1213
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// Last Homeち
 **********************************************************************/
void _stacker_exec_1213_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		motor_ctrl_stacker_stop();	// モータ停止
		_stacker_set_seq(0x1214, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif


/*********************************************************************//**
 * @brief stacker control sequence
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 押し込み前のHome戻し
// モータ停止待ち
 **********************************************************************/
void _stacker_exec_1214_seq(u32 flag)//ok use
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認	// 押し込み開始処理へ
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);	// リトライ処理へ
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認	// 押し込み開始処理へ
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);	// リトライ処理へ
	}
#endif
}

/*********************************************************************//**
 * staker half
 **********************************************************************/
/*********************************************************************//**
 * @brief stacker control interrupt procedure (half sequence)
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_half_seq_proc(u32 flag)
{
	switch(ex_stacker_task_seq & 0x00FF)
	{
	case 0x00:									/* seq1300 */
		_stacker_half_1300_seq(flag);			// モータ起動待ち
		break;
	case 0x02:									/* seq1302 */
		_stacker_half_1302_seq(flag);			// HOME外れ待ち
		break;
	case 0x04:									/* seq1304 */
		_stacker_half_1304_seq(flag);			// 指定パルス押し込み待ち
		break;
	case 0x06:									/* seq1306 */
		_stacker_half_1306_seq(flag);			// モータ停止待ち
		break;

	case 0x10:
		_stacker_half_1310_seq(flag);			// Home戻し中
		break;
#if defined(UBA_RTQ)		/* '21-03-01 */
	case 0x11:
		_stacker_half_1311_seq(flag);			// Last Home戻し中
		break;
#endif
	case 0x12:
		_stacker_half_1312_seq(flag);			// モータ停止待ち
		break;
	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 12);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1300
 *  wait motor start
 * @param[in]	stacker motor event flag
 * @return 		None
// 半押し 押し込み処理
**********************************************************************/
void _stacker_half_1300_seq(u32 flag)//use
{
#if defined(UBA_RTQ) 		/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		_stacker_set_half_retry(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// not home
		//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1310, STACKER_SEQ_TIMEOUT);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
	else
	{
		// home
		//if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, STACKER_HALF_POSITION_PULSE_RTQ, 0))	// 現在位置からのカウントでイベント発生
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, STACKER_HALF_POSITION_PULSE_RTQ, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))	// 現在位置からのカウントでイベント発生
		{
			_ir_stacker_motor_ctrl.event_pulse_up = 0;
			ex_recovery_info.back_fwd_pulse = 0;
			_stacker_set_seq(0x1304, STACKER_SEQ_TIMEOUT);
		}
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		_stacker_set_half_retry(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// not home
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
			_stacker_set_seq(0x1310, STACKER_SEQ_TIMEOUT);
		}
	}
	else
	{
	// home
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, STACKER_HOMEOUT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))	// 現在位置からのカウントでイベント発生
		{
			_stacker_set_seq(0x1302, STACKER_SEQ_TIMEOUT);
		}
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1302
 *  pusher half push position
 * @param[in]	stacker motor event flag
 * @return 		None
// FWD方向 Home外待ち
 **********************************************************************/
void _stacker_half_1302_seq(u32 flag)// use
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_set_half_retry(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_set_half_retry(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	/* time out */
		_stacker_set_half_retry(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// next step
	// パルス指定
	// 前のシーケンスでmotor_ctrl_stacker_fwd_uba_uba_uba_ubaの第3引数が1なのでここでのパルス指定は必要ないかも
		motor_ctrl_stacker_set_full_check(STACKER_HALF_POSITION_PULSE, 0);
		_stacker_set_seq(0x1304, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


// 指定パルス押し込み完了待ち
void _stacker_half_1304_seq(u32 flag)//use
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_set_half_retry(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_set_half_retry(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	//next step
	// 規定パルスは押し込めた
		motor_ctrl_stacker_stop();	// モータ停止
	#if defined(UBA_RTQ)
		dly_tsk(200); //30msecで問題ないが、RTAのポール周期が100msecになる場合を考慮
	#endif
		_stacker_set_seq(0x1306, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1306
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// モータ停止待ち
 **********************************************************************/
void _stacker_half_1306_seq(u32 flag)// use
{
#if defined(UBA_RTQ)		/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
	// モータがとまらなかったので、エラーとする
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (!(SENSOR_STACKER_HOME))
		{
		// 成功
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_HALF_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
		// Home
			_stacker_set_half_retry(ALARM_CODE_STACKER_HOME);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
	// モータがとまらなかったので、エラーとする
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (!(SENSOR_STACKER_HOME))
		{
		// 成功
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_HALF_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
		// Home
			_stacker_set_half_retry(ALARM_CODE_STACKER_HOME);
		}
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1310
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// ホーム戻し
 **********************************************************************/
void _stacker_half_1310_seq(u32 flag)
{
#if defined(UBA_RTQ)		/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
		// Home検知後のだめおし
		_stacker_set_seq(0x1311, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);	//ALARM_CODE_STACKER_TIMEOUT
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);	//ALARM_CODE_STACKER_GEAR
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
//      時間指定にするか、パルス指定にするか未定
// Home検知後のだめおし
		_stacker_set_seq(0x1312, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}


#if defined(UBA_RTQ)		/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence 0x1311
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// Last Homeち
 **********************************************************************/
void _stacker_half_1311_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		motor_ctrl_stacker_stop();	// モータ停止
		_stacker_set_seq(0x1312, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif


/*********************************************************************//**
 * @brief stacker control sequence 0x1224
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_half_1312_seq(u32 flag)// ok
{
#if defined(UBA_RTQ)		/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// next step
		_stacker_set_half_retry(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
		_stacker_set_half_retry(ALARM_CODE_STACKER_TIMEOUT);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sub function
 *  set half retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _stacker_set_half_retry(u32 alarm_code)// use
{
	s_stacker_retry++;

	if (s_stacker_retry >= STACKER_HALF_RETRY_COUNT)
	{
	/* retry over */
		_stacker_set_alarm(alarm_code);
	}
	else
	{
		if (!(is_motor_ctrl_stacker_stop()))
		{
			motor_ctrl_stacker_stop();
		}
		_stacker_set_seq(0x1300, STACKER_SEQ_TIMEOUT);
	}
}



void _stacker_pull_seq_proc(u32 flag)	// 0x14XX
{
	switch(ex_stacker_task_seq & 0x00FF)
	{

	// 1回目の押し込み成功後 Fullでもない
	case 0x00:
		_stacker_exec_1400_seq(flag);		// 頂点からの戻し動作開始待ち
		break;
	case 0x09:
		_stacker_exec_1409_seq(flag);		// 規定パルス戻し待ち、連続取り込みの為
		break;
	case 0x0A:
		_stacker_exec_140A_seq(flag);		// 押しメカ Home待ち
		break;
	case 0x0C:
		_stacker_exec_140C_seq(flag);		// 規定時間待ち
		break;
#if defined(UBA_RTQ)	/* '21-03-01 */
	case 0x0D:
		_stacker_exec_140D_seq(flag);		// 規定時間待ち
		break;
#endif
	case 0x0E:
		_stacker_exec_140E_seq(flag);		// モータ停止待ち
		break;
	case 0x20:
		_stacker_exec_1420_seq(flag);		// 押しメカ Home retry
		break;
	case 0x22:
		_stacker_exec_1422_seq(flag);		// Home retry
		break;
	case 0x24:
		_stacker_exec_1424_seq(flag);		// Home retry
		break;
#if defined(UBA_RTQ)	/* '21-03-01 */
	case 0x25:
		_stacker_exec_1425_seq(flag);		// Home retry
		break;
#endif
	case 0x26:
		_stacker_exec_1426_seq(flag);		// Home retry
		break;

#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
	case 0x30:
		_stacker_exec_1430_seq(flag);		// 押しメカ 半戻し待ち
		break;
	case 0x32:
		_stacker_exec_1432_seq(flag);		// 押しメカ 停止待ち
		break;
#endif
	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 11);
		break;
	}
}


/*********************************************************************//**
 *
// 押しメカ頂点状態,押し込みが成功した状態、Fullでもない
 **********************************************************************/
void _stacker_exec_1400_seq_start(void)
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (SENSOR_STACKER_HOME)
	{
	// 通常はあり得ない
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
	else
	{
	// next step 押しメカ戻し動作
	#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg_arg1_back == SS_PULL_HALF )
		{
			//2023-09-27
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_PULL_HALF_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	//
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;

				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;

				_stacker_set_seq(0x1430, STACKER_SEQ_TIMEOUT);	/* 半戻し*/
			}
		}
		else
		{
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_ALLOW_NEXT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;
				_stacker_set_seq(0x1409, STACKER_SEQ_TIMEOUT); //Home戻し
			}
		}
	#else
		//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_ALLOW_NEXT_PULSE, UBA_STACKER_AD_REV))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_ALLOW_NEXT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		{
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			// ex_dline_testmode.test_start = 1;
			// ex_dline_testmode.time_count = 0;
			#if defined(UBA_RTQ)
			ex_dline_testmode.test_start = 1;
			ex_dline_testmode.time_count = 0;
			#endif // UBA_RTQ
			_stacker_set_seq(0x1409, STACKER_SEQ_TIMEOUT);
		}
	#endif
	}
}

void _stacker_exec_1400_seq(u32 flag)
{

	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);

	}
	else if (SENSOR_STACKER_HOME)
	{
	// 通常はあり得ない
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
	else
	{
	#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
		if( stacker_msg_arg1_back == SS_PULL_HALF )
		{
			//2023-09-27
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_PULL_HALF_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	//
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;

				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;

				_stacker_set_seq(0x1430, STACKER_SEQ_TIMEOUT);	/* 半戻し*/
			}
		}
		else
		{
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_ALLOW_NEXT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				ex_dline_testmode.test_start = 1;
				ex_dline_testmode.time_count = 0;
				_stacker_set_seq(0x1409, STACKER_SEQ_TIMEOUT);	/* Home戻し */
			}
		}
	#else
		//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_ALLOW_NEXT_PULSE, UBA_STACKER_AD_REV))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_ALLOW_NEXT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))	// ずっと戻し続ける 一定戻し量でイベントを発生させる
		{
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			// ex_dline_testmode.test_start = 1;
			// ex_dline_testmode.time_count = 0;
			#if defined(UBA_RTQ)
			ex_dline_testmode.test_start = 1;
			ex_dline_testmode.time_count = 0;
			#endif // UBA_RTQ
			_stacker_set_seq(0x1409, STACKER_SEQ_TIMEOUT);
		}
	#endif
	}
}

void _stacker_exec_1409_seq(u32 flag)//ok Home戻し中、連続有効モード
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);

	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// next step
	// 規定パルス以上もどせたので、次の取り込みを許可する
		
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_PULL_RSP, TMSG_SUB_ENABLE_NEXT, 0, 0, 0);
		_stacker_set_seq(0x140A, STACKER_SEQ_TIMEOUT);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// NG
	// 予定より早くHomeになった
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence
 *  move home position
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込み成功後
// 押しメカHome戻し動作中
// 押しメカHome待ち
 **********************************************************************/
void _stacker_exec_140A_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);

	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
		// if( ex_dline_testmode.test_start == 1 )
		// {
		// 	ex_dline_testmode.time2 = ex_dline_testmode.time_count;			/* 時間を保存	*/
		// 	ex_dline_testmode.test_start = 0;
		// 	ex_dline_testmode.test_result = TEST_RESULT_OK;
		// }
		_stacker_set_seq(0x140D, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);

	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
		if( ex_dline_testmode.test_start == 1 )
		{
			ex_dline_testmode.time2 = ex_dline_testmode.time_count;			/* 時間を保存	*/
			ex_dline_testmode.test_start = 0;
			ex_dline_testmode.test_result = TEST_RESULT_OK;
		}
		_stacker_set_seq(0x140C, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1224
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込み成功後
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_exec_140C_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x140E, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence 0x140D
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 規定パルス到達待ち
 **********************************************************************/
void _stacker_exec_140D_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x140E, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif

/*********************************************************************//**
 * @brief stacker control sequence 0x1226
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込み成功後
// home検出後の最後の念押し完了
// モータ停止待ち
 **********************************************************************/
void _stacker_exec_140E_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
		if(SENSOR_STACKER_HOME)
		{
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_PULL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
			_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// NG 2024-05-08
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{

	// モータ停止を確認
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_PULL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1220
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込みでエラーとなった為、押しメカをHomeに戻す処理、FULL含む
// モータ停止確認後、モータ起動待ち
// Home戻りのリトライ開始関数
 **********************************************************************/
void _stacker_exec_1420_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (SENSOR_STACKER_HOME)
		{
		//ここに入ったときの処理は、別途検討する
		// すでにHomeなので、HomeへのラストFeed
			//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				_stacker_set_seq(0x1425, STACKER_SEQ_TIMEOUT);
				motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
			}
		}
		//else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
		else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
		// 通常はここ
		// 押しメカの位置はまちまちなので、マージンを持って設定する
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1422, STACKER_SEQ_TIMEOUT);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (SENSOR_STACKER_HOME)
		{
		//ここに入ったときの処理は、別途検討する
		// すでにHomeなので、HomeへのラストFeed
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				_stacker_set_seq(0x1424, STACKER_SEQ_HOME_LAST_FEED);
			}
		}
		else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
		// 通常はここ
		// 押しメカの位置はまちまちなので、マージンを持って設定する
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1422, STACKER_SEQ_TIMEOUT);
		}
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1222
 *  move home position
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// 押しメカHome戻し動作中
// 押しメカHome待ち
 **********************************************************************/
void _stacker_exec_1422_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
// 時間指定にするか、パルス指定にするか未定
// Home検知後のだめおし
		_stacker_set_seq(0x1425, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
// 時間指定にするか、パルス指定にするか未定
// Home検知後のだめおし
		_stacker_set_seq(0x1424, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1424
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_exec_1424_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1426, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence 0x1425
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 規定パルス到達待ち
 **********************************************************************/
void _stacker_exec_1425_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1426, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif

/*********************************************************************//**
 * @brief stacker control sequence 0x1426
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押し完了
// モータ停止待ち
 **********************************************************************/
void _stacker_exec_1426_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		// モータ停止を確認
		// 最初の押し込みに成功 戻し動作には成功した
		if(SENSOR_STACKER_HOME)
		{
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_PULL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
			_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// NG 2024-05-08
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		// モータ停止を確認
		// 最初の押し込みに成功 戻し動作には成功した
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_PULL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
#endif
}



#if !defined(UBA_RTQ)//#if defined(NEW_STACK)
/* 一定パルスの戻し待ち *//* 半戻し *//* 単体動作*/
void _stacker_exec_1430_seq(u32 flag)//ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_set_alarm(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);

	}
	else if (SENSOR_STACKER_HOME)
	{

	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// next step
	// 半戻し完了
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1432, STACKER_SEQ_TIMEOUT);

	}
}


void _stacker_exec_1432_seq(u32 flag)//ok
{

	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{

	// モータ停止を確認
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_PULL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
}



#endif



/*********************************************************************//**
 * Stacker motor free run
 **********************************************************************/
/*********************************************************************//**
 * @brief stacker control interrupt procedure (free run sequence)
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_freerun_seq_proc(u32 flag)	// DIP-SW test mode
{
	switch(ex_stacker_task_seq & 0x00FF)
	{
	case 0x00:									/* seq1F00 */
		_stacker_freerun_1f00_seq(flag);
		break;
	case 0x01:									/* seq1F01 */
		_stacker_freerun_1f01_seq(flag);
		break;
	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 13);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1F00
 *  wait motor start
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_freerun_1f00_seq(u32 flag)
{

	if (!(is_box_set()))
	{
		if (s_stacker_freerun_dir == MOTOR_FWD)
		{
			if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[0].limit1_1st))		// 現在位置からのカウントでイベント発生
			{
				_stacker_set_seq(0x1f01, STACKER_FREERUN_CHECK_TIME);
			}
		}
		else if (s_stacker_freerun_dir == MOTOR_REV)
		{
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[0].limit2_1st))
			{
				_stacker_set_seq(0x1f01, STACKER_FREERUN_CHECK_TIME);
			}
		}
		else
		{
			_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
			
			/* system error ? */
			_stacker_system_error(0, 14);
		}
	}
	else
	{
		_stacker_set_alarm(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1F01
 *  free run
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_freerun_1f01_seq(u32 flag)
{
	if (!(is_box_set()))
	{
	#if 1
		if (IS_STACKER_EVT_MOTOR_LOCK(flag))
		{
		/* motor lock */
			_stacker_set_alarm(ALARM_CODE_STACKER_MOTOR_LOCK);
		}
		else if (IS_STACKER_EVT_TIMEOUT(flag))
		{
			if (s_stacker_freerun_dir == MOTOR_FWD)
			{
				if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[0].limit1_1st))		// 現在位置からのカウントでイベント発生
				{
					_stacker_set_seq(0x1f01, STACKER_FREERUN_CHECK_TIME);
				}
				else
				{
					_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);

					/* system error ? */
					_stacker_system_error(0, 15);
				}
			}
			else if (s_stacker_freerun_dir == MOTOR_REV)
			{
				if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[0].limit2_1st))
				{
					_stacker_set_seq(0x1f01, STACKER_FREERUN_CHECK_TIME);
				}
				else
				{
					_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);

					/* system error ? */
					_stacker_system_error(0, 16);
				}
			}
			else
			{
				_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);

				/* system error ? */
				_stacker_system_error(0, 17);
			}
		}
	#else
		if (IS_STACKER_EVT_MOTOR_LOCK(flag))
		{
		/* motor lock */
			_stacker_set_alarm(ALARM_CODE_STACKER_MOTOR_LOCK);
		}
		else if (IS_STACKER_EVT_TIMEOUT(flag))
		{
			_stacker_set_seq( 0x1f01, STACKER_FREERUN_CHECK_TIME );
		}
	#endif	
	}
	else
	{
		_stacker_set_alarm(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
}


void _stacker_exec_retry_seq_proc(u32 flag)
{
	switch(ex_stacker_task_seq & 0x00FF)
	{
	case 0x00:
		_stacker_exec_1500_seq(flag);
		break;
	case 0x04:
		_stacker_exec_1504_seq(flag);
		break;
	case 0x06:
		_stacker_exec_1506_seq(flag);		// 規定パルス押し込めてない場合、ここでエラーへ遷移する
		break;
	case 0x08:
		_stacker_exec_1508_seq(flag);		// 押しメカ頂点後の押しメカ停止待ち、ここでFull判定を行う ここでモータ逆転起動
		break;
	case 0x0A:
		_stacker_exec_150A_seq(flag);		// 押し込み成功時の戻し動作
		break;
	case 0x0C:
		_stacker_exec_150C_seq(flag);		// 押し込み成功時の戻し動作
		break;
#if defined(UBA_RTQ)		/* '21-03-01 */
	case 0x0D:
		_stacker_exec_150D_seq(flag);
		break;
#endif
	case 0x0E:
		_stacker_exec_150E_seq(flag);	// 成功のみ、Fullの場合0x1508で0x1520へ遷移している
		break;

	// リトライ、エラー確定時に戻し動作
	case 0x20:
		_stacker_exec_1520_seq(flag);	// リトライ確定の戻し動作 ここでモータ逆転起動
		break;
	case 0x22:
		_stacker_exec_1522_seq(flag);
		break;
	case 0x24:
		_stacker_exec_1524_seq(flag);
		break;
#if defined(UBA_RTQ)		/* '21-03-01 */
	case 0x25:
		_stacker_exec_1525_seq(flag);
		break;
#endif
	case 0x26:
		_stacker_exec_1526_seq(flag);
		break;

	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 11);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1200
 *  wait motor start
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカ位置確認
**********************************************************************/
void _stacker_exec_1500_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_push_count(ALARM_CODE_STACKER_TIMEOUT);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	// 通常はあり得ないのでこの処理はコメントアウトしておく
	else if (!(SENSOR_STACKER_HOME))
	{
	// Not Home	// Homeへ戻す処理へ
		_stacker_set_seq(0x1520, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
	else
	{
		//if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, 0))	// 現在位置からのカウントでイベント発生
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))	// 現在位置からのカウントでイベント発生
		{
		//モータ起動を通知
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);

			// Homeを外れてから、正式にFullのカウントを行う
			_ir_stacker_motor_ctrl.event_pulse_up = 0;
			ex_recovery_info.back_fwd_pulse = 0;

#if defined(_DEBUG_STACKER_MOTOR_PULSE)
			// Homeを外れた位置から押し込みパルスを指定
			_debug_stacker_motor_flag = 1;	//計測有効
#endif
			// この前のシーケンスのmotor_ctrl_stacker_fwd_uba_uba_uba_ubaの第3引数が1に設定されているので、エンコーダ割り込みの関数で、Home外となった時に、パルス計測のカウンタを再設定しているので、
			// ここで設定する必要なし
			
			motor_ctrl_stacker_set_full_check(STACKER_TOP_PULSE, 1);//USA 998パルス、EUR1119パスル Fullチェック有効
			_stacker_set_seq(0x1506, STACKER_SEQ_TIMEOUT);
		}
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_push_count(ALARM_CODE_STACKER_TIMEOUT);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	// 通常はあり得ないのでこの処理はコメントアウトしておく
	else if (!(SENSOR_STACKER_HOME))
	{
	// Not Home	// Homeへ戻す処理へ
		_stacker_set_seq(0x1520, STACKER_SEQ_TIMEOUT);
	}
	else
	{
		if (IERR_CODE_OK == motor_ctrl_stacker_fwd_uba(MOTOR_MAX_SPEED, STACKER_HOMEOUT_PULSE, motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st))	// 現在位置からのカウントでイベント発生
		{
		//モータ起動を通知
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RSP, TMSG_SUB_START, 0, 0, 0);

		// 押し込み開始
			_stacker_set_seq(0x1504, STACKER_SEQ_TIMEOUT);
		}
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1201
 *  deviate home position
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカHome外れ待ち
 **********************************************************************/
void _stacker_exec_1504_seq(u32 flag)//ok
{

	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */// リトライ可能であれば、押しメカ戻し処理へ
		_stacker_check_to_push_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse < STACKER_MOTOR_GEAR_LIMIT)
		{
		/* time out */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_TIMEOUT);
		}
		else
		{
		/* gear */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_GEAR);
		}
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	// 押し込み開始時に識別ｾﾝｻがONした場合 Cheat
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		/* time out */
		_stacker_check_to_push_count(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (!(SENSOR_STACKER_HOME))
	{



	// Homeを外れてから、正式にFullのカウントを行う
		_ir_stacker_motor_ctrl.event_pulse_up = 0;

		// Homeを外れた位置から押し込みパルスを指定
		// 前のシーケンスでmotor_ctrl_stacker_fwd_uba_uba_uba_ubaの第3引数が1なのでここでのパルス指定は必要ないかも
		motor_ctrl_stacker_set_full_check(STACKER_TOP_PULSE, 1);
		_stacker_set_seq(0x1506, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1202
 *  push stacker
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカ頂点待ち
 **********************************************************************/
void _stacker_exec_1506_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_FULL);
		}
		else
		{
		/* motor lock */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_MOTOR_LOCK);
		}
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_FULL);
		}
		else
		{
			/* time out */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_TIMEOUT);
		}

	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	// 識別センサがONしているのでCheat
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// 規定パルスは押し込めた
		//next step
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1508, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
		if( stacker_msg_arg1_back == SS_BILL_STACK)
		{
			if( _ir_stacker_motor_ctrl.event_pulse_up == STACKER_BILL_IN_BOX )
			{
				set_recovery_step(RECOVERY_STEP_STACKING_BILL_IN_BOX);
			}
		}
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_FULL);
		}
		else
		{
		/* motor lock */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_MOTOR_LOCK);
		}
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if( stacker_msg_arg1_back == SS_BILL_STACK)
		{
			if( _ir_stacker_motor_ctrl.event_pulse_up == STACKER_BILL_IN_BOX )
			{
				set_recovery_step(RECOVERY_STEP_STACKING_BILL_IN_BOX);
			}
		}
		if (_ir_stacker_motor_ctrl.pulse >= STACKER_FULL_DA)
		{
		/* Full */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_FULL);
		}
		else
		{
			/* time out */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_TIMEOUT);
		}

	}
	else if((SENSOR_STACKER_HOME))
	{
	// なぜかHomeにあるのでエラー
		_stacker_check_to_push_count(ALARM_CODE_STACKER_HOME);

	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	// 識別センサがONしているのでCheat
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// 規定パルスは押し込めた
	// 規定パルスは押し込めた
		//next step
		if( stacker_msg_arg1_back == SS_BILL_STACK)
		{
			set_recovery_step(RECOVERY_STEP_STACKING_BILL_IN_BOX);
		}
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1508, STACKER_SEQ_TIMEOUT);
	}

#endif
}


/*********************************************************************//**
 *
 *
 *
 *
// 押しメカ頂点待ち後、モータ停止待ち
 **********************************************************************/
void _stacker_exec_1508_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */// リトライ可能であれば、押しメカ戻し処理へ
		_stacker_check_to_push_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		if (_ir_stacker_motor_ctrl.pulse < STACKER_MOTOR_GEAR_LIMIT)
		{
		/* time out */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_TIMEOUT);
		}
		else
		{
		/* gear */
			_stacker_check_to_push_count(ALARM_CODE_STACKER_GEAR);
		}
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	// 押し込み開始時に識別ｾﾝｻがONした場合 Cheat
		_stacker_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (SENSOR_STACKER_HOME)
	{
		_stacker_check_to_push_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// next step
	// Full確認 頂点手前150パルスでの経過時間リミット
		if( _ir_stacker_motor_ctrl.peakload_time  >= STACKER_FULL_TIME )
		{
		// Full 戻し動作へ
			_stacker_check_to_push_count(ALARM_CODE_STACKER_FULL);
		}
		else
		{
		// OK
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
	//			押し込みのリトライ動作なので、押し切った位置でのメッセージは必要ない結果のみ通知すればいい
				_stacker_set_seq(0x150A, STACKER_SEQ_TIMEOUT);
			}
		}
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1222
 *  move home position
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込み成功後
// 押しメカHome戻し動作中
// 押しメカHome待ち
 **********************************************************************/
void _stacker_exec_150A_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);

	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
//      時間指定にするか、パルス指定にするか未定
		_stacker_set_seq(0x150D, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);

	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);

	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
//      時間指定にするか、パルス指定にするか未定
		_stacker_set_seq(0x150C, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1224
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込み成功後
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_exec_150C_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x150E, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence 0x150D
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込み成功後
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_exec_150D_seq(u32 flag)// ok
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x150E, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif


/*********************************************************************//**
 * @brief stacker control sequence 0x1226
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込み成功後
// home検出後の最後の念押し完了
// モータ停止待ち
// FULLを含まない、全ての成功のみ
 **********************************************************************/
void _stacker_exec_150E_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
		if(SENSOR_STACKER_HOME)
		{
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
			_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// NG 2024-05-08
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_RE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sub function
 *  set exec revers
 * @param[in]	None
 * @return 		None
// 押し込み1回目のみ
// 押し込み動作時になんらかのエラーがあった、
// 取り合えずモータ停止命令
 **********************************************************************/
void _stacker_check_to_push_count(u32 alarm_code)//ok 0x15
{

	//2018-12-03
	s_stacker_alarm_code = alarm_code;

	s_stacker_retry++;

	if (!(is_motor_ctrl_stacker_stop()))
	{
		motor_ctrl_stacker_stop();
	}

	// 押し込みエラーなので戻し動作へ
	_stacker_set_seq(0x1520, STACKER_SEQ_TIMEOUT);
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1220
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込みでエラーとなった為、押しメカをHomeに戻す処理、FULL含む
// モータ停止確認後、モータ起動待ち
// Home戻りのリトライ開始関数
 **********************************************************************/
void _stacker_exec_1520_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (SENSOR_STACKER_HOME)
		{
		//ここに入ったときの処理は、別途検討する
		// すでにHomeなので、HomeへのラストFeed
			//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				_stacker_set_seq(0x1525, STACKER_SEQ_TIMEOUT);
				motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
			}
		}
		//else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
		else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
		// 通常はここ
		// 押しメカの位置はまちまちなので、マージンを持って設定する
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1522, STACKER_SEQ_TIMEOUT);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (SENSOR_STACKER_HOME)
		{
		//ここに入ったときの処理は、別途検討する
		// すでにHomeなので、HomeへのラストFeed
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				_stacker_set_seq(0x1524, STACKER_SEQ_HOME_RECOVER_LAST_FEED);
			}
		}
		else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
		// 通常はここ
		// 押しメカの位置はまちまちなので、マージンを持って設定する
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1522, STACKER_SEQ_TIMEOUT);
		}
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1222
 *  move home position
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// 押しメカHome戻し動作中
// 押しメカHome待ち
 **********************************************************************/
void _stacker_exec_1522_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
// 時間指定にするか、パルス指定にするか未定
// Home検知後のだめおし
		_stacker_set_seq(0x1525, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
// 時間指定にするか、パルス指定にするか未定
// Home検知後のだめおし
		_stacker_set_seq(0x1524, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1224
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_exec_1524_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1526, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence 0x1225
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_exec_1525_seq(u32 flag)// ok
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1526, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif


/*********************************************************************//**
 * @brief stacker control sequence 0x1226
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押し完了
// モータ停止待ち
 **********************************************************************/
void _stacker_exec_1526_seq(u32 flag)//ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// NG 2024-05-08
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
	// 最初の押し込みに失敗したが、戻し動作には成功した
	//エラー回数を確認 ここでは、押し込みのエラーカウントのみ確認
		if (s_stacker_retry > STACKER_EXEC_RETRY_PUSH_COUNT ||
			s_stacker_alarm_home_retry > STACKER_EXEC_RETRY_HOME_COUNT )
		{
		// エラー処理へ遷移
			_stacker_set_alarm(s_stacker_alarm_code);
		}
		else
		{
		// 押し込みリトライ処理へ
			_stacker_set_seq(0x1500, STACKER_SEQ_TIMEOUT);
		}
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


// 1回目の押し込み失敗後、
// Home戻し動作でエラーとなった
// リトライ回数をオーバーしているか確認する
void _stacker_check_to_home_count(u32 alarm_code)//ok 0x11,0x12,0x13,0x14,0x15,0x16
{

	s_stacker_alarm_home_retry++;

	s_stacker_alarm_code = alarm_code;

	switch(ex_stacker_task_seq & 0xFF00) //2024-06-17
	{
		case STACKER_SEQ_EXEC: //0x12XX
		case STACKER_SEQ_PULL: //0x14XX
		case STACKER_SEQ_EXEC_RETRY: //0x15XX
		case STACKER_SEQ_EXEC_NG_PULL: //0x16XX
			//2
			if (s_stacker_alarm_home_retry > STACKER_EXEC_RETRY_HOME_COUNT)
			{
			/* retry over */
				_stacker_set_alarm(alarm_code);
				return;
			}
			break;
		case STACKER_SEQ_HOME: 	//0x11XX
		case STACKER_SEQ_HALF:	//0x13XX
			if (s_stacker_alarm_home_retry >= STACKER_HOME_RETRY_COUNT)
			{
			/* retry over */
				_stacker_set_alarm(alarm_code);
			}
			//3
			break;
	}


	if (s_stacker_alarm_home_retry > STACKER_EXEC_RETRY_HOME_COUNT)
	{
	/* retry over */
		_stacker_set_alarm(alarm_code);
	}
	else
	{

		if (!(is_motor_ctrl_stacker_stop()))
		{
			motor_ctrl_stacker_stop();
		}

		switch(ex_stacker_task_seq & 0xFF00)
		{

			case STACKER_SEQ_HOME: 	//0x11XX
				_stacker_set_seq(0x1100, STACKER_SEQ_TIMEOUT); // 1100 リトライ処理も通常シーケンスと同じ
				break;

			case STACKER_SEQ_EXEC:
				_stacker_set_seq(0x1200, STACKER_SEQ_TIMEOUT);
				break;

			case STACKER_SEQ_HALF:	//0x13XX
				_stacker_set_seq(0x1300, STACKER_SEQ_TIMEOUT); // 1300 リトライ処理も通常シーケンスと同じ
				break;

			case STACKER_SEQ_PULL:
				_stacker_set_seq(0x1420, STACKER_SEQ_TIMEOUT);
				break;

			case STACKER_SEQ_EXEC_RETRY:
				_stacker_set_seq(0x1520, STACKER_SEQ_TIMEOUT);
				break;

			case STACKER_SEQ_EXEC_NG_PULL:
				_stacker_set_seq(0x1600, STACKER_SEQ_TIMEOUT);
				break;
		}
	}
}



void _stacker_exec_ng_pull_seq_proc(u32 flag)	// 0x16XX
{

	switch(ex_stacker_task_seq & 0x00FF)
	{

	// 1回目の押し込みNG後
	case 0x00:
		_stacker_exec_1600_seq(flag);		// 頂点からの戻し動作開始待ち
		break;
	case 0x09:
		_stacker_exec_1609_seq(flag);		// 規定パルス戻し待ち、連続取り込みの為
		break;
	case 0x0A:
		_stacker_exec_160A_seq(flag);		// 押しメカ Home待ち
		break;
#if defined(UBA_RTQ)	/* '21-03-01 */
	case 0x0B:
		_stacker_exec_160B_seq(flag);
		break;
#endif
	case 0x0C:
		_stacker_exec_160C_seq(flag);		// 規定時間待ち
		break;
	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 11);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1220
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 1回目の押し込みでエラーとなった為、押しメカをHomeに戻す処理、FULL含む
// モータ停止確認後、モータ起動待ち
// Home戻りのリトライ開始関数
 **********************************************************************/
void _stacker_exec_1600_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (SENSOR_STACKER_HOME)
		{
		//ここに入ったときの処理は、別途検討する
		// すでにHomeなので、HomeへのラストFeed
			//if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				_stacker_set_seq(0x160B, STACKER_SEQ_TIMEOUT);
				motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
			}
		}
		//else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, UBA_STACKER_AD_REV))
		else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
		// 通常はここ
		// 押しメカの位置はまちまちなので、マージンを持って設定する
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1609, STACKER_SEQ_TIMEOUT);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		if (SENSOR_STACKER_HOME)
		{
		//ここに入ったときの処理は、別途検討する
		// すでにHomeなので、HomeへのラストFeed
			if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
			{
				_ir_stacker_motor_ctrl.event_pulse_down = 0;
				_stacker_set_seq(0x160A, STACKER_SEQ_HOME_RECOVER_LAST_FEED);
			}
		}
		else if (IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, 0, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
		// 通常はここ
		// 押しメカの位置はまちまちなので、マージンを持って設定する
			_ir_stacker_motor_ctrl.event_pulse_down = 0;
			_stacker_set_seq(0x1609, STACKER_SEQ_TIMEOUT);
		}
	}
#endif
}


/*********************************************************************//**
 * @brief stacker control sequence
 *  move home position
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// 押しメカHome戻し動作中
// 押しメカHome待ち
 **********************************************************************/
void _stacker_exec_1609_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
	// Home検知後のだめおし
		_stacker_set_seq(0x160B, STACKER_SEQ_TIMEOUT);
		motor_ctrl_stacker_set_drive_check(STACKER_HOME_PULSE_RTQ);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		//規定数以上ギアが回っているが、Homeに到達していない
		/* gear or home sensor abnormal */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_GEAR);
	}
	else if (SENSOR_STACKER_HOME)
	{
	// next step
	// Home検知後のだめおし
		_stacker_set_seq(0x160A, STACKER_SEQ_HOME_LAST_FEED);
	}
#endif
}

/*********************************************************************//**
 * @brief stacker control sequence
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押しtime out
 **********************************************************************/
void _stacker_exec_160A_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x160C, STACKER_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)	/* '19-03-25 */
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#endif
}


#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * @brief stacker control sequence
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// 規定パルス到達待ち
 **********************************************************************/
void _stacker_exec_160B_seq(u32 flag)// ok
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
	// next step
	/* 最後の念押し完了 モータ停止*/
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x160C, STACKER_SEQ_TIMEOUT);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif

/*********************************************************************//**
 * @brief stacker control sequence
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// リトライ時
// home検出後の最後の念押し完了
// モータ停止待ち
 **********************************************************************/
void _stacker_exec_160C_seq(u32 flag)//ok
{
#if defined(UBA_RTQ)	/* '21-03-01 */
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
	// 最初の押し込みに失敗したが、戻し動作には成功した
	// 押し込みリトライをMAINから通知してもらう
		if(SENSOR_STACKER_HOME)
		{
			_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_NG_PULL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_stacker_set_seq(STACKER_SEQ_IDLE, 0);
		}
		else
		{
			_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
#else
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (!(SENSOR_STACKER_HOME))
	{
	// NG 2024-05-08
		_stacker_check_to_home_count(ALARM_CODE_STACKER_HOME);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
	/* time out */// エラーとする not retry
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
	// モータ停止を確認
	// 最初の押し込みに失敗したが、戻し動作には成功した
	// 押し込みリトライをMAINから通知してもらう
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_EXEC_NG_PULL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
#endif
}

#if defined(UBA_RTQ)	/* '21-03-01 */
/*********************************************************************//**
 * staker back
 **********************************************************************/
/*********************************************************************//**
 * @brief stacker control interrupt procedure (back sequence)
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_back_seq_proc(u32 flag)
{
	switch(ex_stacker_task_seq & 0x00FF)
	{
	case 0x00:									/* seq1700 */
		_stacker_back_1700_seq(flag);			// モータ起動待ち
		break;
	case 0x02:									/* seq1702 */
		_stacker_back_1702_seq(flag);			// 指定パルス戻し待ち
		break;
	case 0x04:									/* seq1704 */
		_stacker_back_1704_seq(flag);			// モータ停止待ち
		break;
	default:									/* other */
		_stacker_set_alarm(ALARM_CODE_STACKER_FORCED_QUIT);
		
		/* system error ? */
		_stacker_system_error(0, 12);
		break;
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1700
 *  wait motor start
 * @param[in]	stacker motor event flag
 * @return 		None
 **********************************************************************/
void _stacker_back_1700_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(is_motor_ctrl_stacker_stop())
	{
		//if(IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_HOME_PULSE_RTQ, UBA_STACKER_AD_REV))
		if(IERR_CODE_OK == motor_ctrl_stacker_rev_uba(MOTOR_MAX_SPEED, STACKER_HOME_PULSE_RTQ, motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st))
		{
			_stacker_set_seq(0x1702, STACKER_SEQ_TIMEOUT);
		}
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}


/*********************************************************************//**
 * @brief stacker control sequence 0x1702
 *  move home position (revers)
 * @param[in]	stacker motor event flag
 * @return 		None
// 押しメカ逆転中
 **********************************************************************/
void _stacker_back_1702_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_STACKER_EVT_MOTOR_LOCK(flag))
	{
		_stacker_set_alarm(ALARM_CODE_STACKER_MOTOR_LOCK);
	}
	else if (IS_STACKER_EVT_TIMEOUT(flag))
	{
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if(IS_STACKER_EVT_DRIVE_PULSE_OVER(flag))
	{
		motor_ctrl_stacker_stop();
		_stacker_set_seq(0x1704, STACKER_SEQ_TIMEOUT);	
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}

/*********************************************************************//**
 * @brief stacker control sequence 0x1704
 *  wait motor stop
 * @param[in]	stacker motor event flag
 * @return 		None
// モータ停止待ち
 **********************************************************************/
void _stacker_back_1704_seq(u32 flag)
{
	if(!(is_box_set()))
	{
		_stacker_set_alarm(ALARM_CODE_BOX);
	}
	else if(IS_STACKER_EVT_TIMEOUT(flag))
	{
		_stacker_set_alarm(ALARM_CODE_STACKER_TIMEOUT);
	}
	else if (is_motor_ctrl_stacker_stop())
	{
		_stacker_send_msg(ID_MAIN_MBX, TMSG_STACKER_BACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_stacker_set_seq(STACKER_SEQ_IDLE, 0);
	}
	else if(is_rc_error())
	{
		/* モータ停止して待機状態に戻る */
		_stacker_set_cancel();
	}
}
#endif

/*********************************************************************//**
 * @brief stacker control sub function
 *  set stacker sequence
 * @param[in]	sequence no.
 * 				time out
 * @return 		None
 **********************************************************************/
void _stacker_set_seq(u16 seq, u16 time_out)
{

#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
	switch (ex_stacker_task_seq & 0xFF00)
	{
	case STACKER_SEQ_PULL://14XX
		save_stack_fram_log(seq);
		break;
	default:
		break;		
	}
#endif

	ex_stacker_task_seq = seq;
	_ir_stacker_ctrl_time_out = time_out;
	clr_flg(ID_STACKER_CTRL_FLAG, ~EVT_ALL_BIT);

#if 1//#ifdef _ENABLE_JDL
	jdl_add_trace(ID_STACKER_TASK, ((ex_stacker_task_seq >> 8) & 0xFF), (ex_stacker_task_seq & 0xFF), s_stacker_alarm_code, s_stacker_retry, s_stacker_alarm_home_retry);
#endif /* _ENABLE_JDL */

	#if defined(UBA_RTQ_AZ_LOG) //2025-06-11
	if(ex_fram_log_enable == 1)  //テストモード開始後1
	{
		ex_fram_uba[16] = ex_fram_uba[14];
		ex_fram_uba[17] = ex_fram_uba[15];


		ex_fram_uba[14] = ex_fram_uba[12];
		ex_fram_uba[15] = ex_fram_uba[13];

		ex_fram_uba[12] = (u8)((seq>>8) & 0xff);
		ex_fram_uba[13] = (u8)(seq & 0xff);
		_hal_write_fram_debug_log_uba();
	}
	#endif
}


/*********************************************************************//**
 * @brief stacker control sub function
 *  set stacker waiteing sequence
 * @param[in]	sequence no.
 * @return 		None
 **********************************************************************/
void _stacker_select_seq(u32 seq)
{
	if (ex_stacker_task_seq == STACKER_SEQ_IDLE)
	{
		_stacker_set_seq(seq, STACKER_SEQ_TIMEOUT);
	}
	else if (ex_stacker_task_seq == STACKER_SEQ_FORCE_QUIT)
	{
		s_stacker_task_wait_seq = seq;
	}
	else
	{
		s_stacker_task_wait_seq = seq;
		
		motor_ctrl_stacker_stop();
		_stacker_set_seq(STACKER_SEQ_FORCE_QUIT, STACKER_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief stacker control sub function
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _stacker_set_alarm(u32 alarm_code)
{
	u32 msg;
	u32 sensor;
	
	s_stacker_alarm_code = alarm_code;
	
	motor_ctrl_stacker_stop();
	
	switch (ex_stacker_task_seq & 0xFF00)
	{
	case STACKER_SEQ_HOME:	// 0x11XX
		msg = TMSG_STACKER_HOME_RSP;
		break;
	case STACKER_SEQ_EXEC:	// 0x12XX
		msg = TMSG_STACKER_EXEC_RSP;
		break;
	case STACKER_SEQ_HALF:	// 0x13XX
		msg = TMSG_STACKER_HALF_RSP;
		break;
	case STACKER_SEQ_PULL:	// 0x14XX
		msg = TMSG_STACKER_PULL_RSP;
		break;
	case STACKER_SEQ_EXEC_RETRY:	// 0x15XX
		msg = TMSG_STACKER_EXEC_RE_RSP;
		break;
	case STACKER_SEQ_EXEC_NG_PULL:	// 0x16XX
		msg = TMSG_STACKER_EXEC_NG_PULL_RSP;
		break;
#if defined(UBA_RTQ)	/* '21-03-01 */
	case STACKER_SEQ_BACK:
		msg = TMSG_STACKER_BACK_RSP;
		break;
#endif
	case STACKER_SEQ_FREERUN:	// 0x1FXX
		msg = TMSG_STACKER_FREERUN_RSP;
		break;
	default:
		msg = TMSG_STACKER_STATUS_INFO;
		break;
	}
	
	sensor = ex_position_sensor;

	_stacker_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_ALARM, s_stacker_alarm_code, ex_stacker_task_seq, sensor);
	_stacker_set_seq(STACKER_SEQ_FORCE_QUIT, STACKER_SEQ_TIMEOUT);
}


#if defined(UBA_RTQ)	/* '19-03-25 */

/*********************************************************************//**
 * @brief stacker control sub function
 *  cancel response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _stacker_set_cancel(void)
{
	u32 msg;

	motor_ctrl_stacker_stop();
	
	switch(ex_stacker_task_seq & 0xFF00)
	{
	case	STACKER_SEQ_HOME:	// 0x11XX
			msg = TMSG_STACKER_HOME_RSP;
			break;
	case	STACKER_SEQ_EXEC:	// 0x12XX
			msg = TMSG_STACKER_EXEC_RSP;
			break;
	case	STACKER_SEQ_HALF:	// 0x13XX
			msg = TMSG_STACKER_HALF_RSP;
			break;
	case	STACKER_SEQ_PULL:	// 0x14XX
			msg = TMSG_STACKER_PULL_RSP;
			break;
	case	STACKER_SEQ_EXEC_RETRY:	// 0x15XX
			msg = TMSG_STACKER_EXEC_RE_RSP;
			break;
	case	STACKER_SEQ_EXEC_NG_PULL:	// 0x16XX
			msg = TMSG_STACKER_EXEC_NG_PULL_RSP;
			break;
#if defined(UBA_RTQ)	/* '21-03-01 */
	case	STACKER_SEQ_BACK:
			msg = TMSG_STACKER_BACK_RSP;
			break;
#endif
	case	STACKER_SEQ_FREERUN:	// 0x1FXX
			msg = TMSG_STACKER_FREERUN_RSP;
			break;
	default:
			msg = TMSG_STACKER_STATUS_INFO;
			break;
	}

	_stacker_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_SUCCESS, 0, 0, 0);
	_stacker_set_seq(STACKER_SEQ_IDLE, 0);
}
#endif


/*********************************************************************//**
// MAINタスクへのRejectメッセージ
// １回目の押し込みが失敗した事を通知する為、MAINに収納動作のリトライが必要な事をRejectで通知する
 **********************************************************************/
void _stacker_set_retry_request(u32 alarm_code)	// 現状 0x12XXのみ
{
	u32 msg;
	u32 sensor;
	
	s_stacker_alarm_code = alarm_code;
	
	motor_ctrl_stacker_stop();
	
	switch (ex_stacker_task_seq & 0xFF00)
	{

	case STACKER_SEQ_EXEC:	/* 0x1200 */
	#if defined(_DEBUG_STACKER_MOTOR_PULSE)
		_debug_stacker_motor_flag = 0;	//計測終了
	#endif
		msg = TMSG_STACKER_EXEC_RSP;
		break;
	default:
		msg = TMSG_STACKER_STATUS_INFO;
		break;
	}

	sensor = ex_position_sensor;

	_stacker_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_REJECT, s_stacker_alarm_code, ex_stacker_task_seq, sensor);
	_stacker_set_seq(STACKER_SEQ_FORCE_QUIT, STACKER_SEQ_TIMEOUT);
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
void _stacker_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;
	
	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_STACKER_TASK;
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
			_stacker_system_error(1, 1);
		}
	}
	else
	{
		/* system error ? */
		_stacker_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _stacker_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR

//	_stacker_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_BLINK, 10, 0, 0, 0);

#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{

		_stacker_send_msg(ID_ERRDISP_MBX, TMSG_ERRDISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	// fatal_err == 1の場合、致命的なエラーなのでエラーコードは出力しない、LED表示のみ通知で無限待ち
	_debug_system_error(ID_STACKER_TASK, (u16)code, (u16)stacker_msg.tmsg_code, (u16)stacker_msg.arg1, fatal_err);
}

