/******************************************************************************/
/*! @addtogroup Main
    @file       motor_task.c
    @brief      motor task process
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
/*
 * motor_task.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal.h"
#include "hal_bezel.h"
#include "pl/pl_motor.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

#include "jdl_conf.h"

//#include "test_file.h"


/************************** Function Prototypes ******************************/
void motor_task(VP_INT exinf);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/************************** PRIVATE DEFINITIONS *************************/
enum _MOTOR_TASK_MODE
{
	MOTOR_TASK_MODE_READY = 1,
};


enum _MOTOR_NUM
{
	MOTOR_NUM_FEED = 1,
	MOTOR_NUM_STACKER,
	MOTOR_NUM_CENTERING,
	MOTOR_NUM_SHUTTER,
	MOTOR_NUM_APB,
};
#define MOTOR_MAX_NUMBER	MOTOR_NUM_APB

#define MOTOR_DISABLE_TIME	100				/* 1sec : 100msec * 10msec */

/************************** PRIVATE VARIABLES *************************/
static T_MSG_BASIC motor_msg;

static u8 s_motor_feed_state;
static u8 s_motor_stacker_state;
static u8 s_motor_centering_state;
static u8 s_motor_apb_state;
static u8 s_motor_shutter_state;
static u8 s_motor_start_waiting[MOTOR_MAX_NUMBER];


/************************** PRIVATE FUNCTIONS *************************/
void _motor_initialize_proc(void);
void _motor_msg_proc(void);
void _motor_idel_proc(void);

void _motor_stop_proc(void);
void _motor_start_proc(void);

void _motor_start_feed(u8 dir);
void _motor_start_stacker(u8 dir);
void _motor_start_centering(u8 dir);
void _motor_start_apb(u8 dir);
//現状UBA-CISでは使用しない、RCでは使用するかも
void _motor_start_shutter(u8 dir);

/* 今回は1 = 0.0625度 */
const MOTOR_LIMIT_STACKER_TABLE motor_limit_stacker_table[STACKER_AD_NUMBER] =
#if defined(UBA_RTQ)
	{
	/*  temp	Fwd 1st		Fwd 2nd		Rev 1st(0.6A)*/
	{-21,   	150, 85, 	100 }	/* -20	*//* 0	*/		//0x0974, 0x0555, 0x064E
	,{-16,		150, 85, 	100 }	/* -20～-15	*//* 1	*/	//0x0974, 0x0555, 0x064E
	,{-11,		150, 85, 	100 }	/* -15～-10	*//* 2	*/	//0x0974, 0x0555, 0x064E
	,{-6,		150, 85, 	100 }	/* -10～-5	*//* 3	*/	//0x0974, 0x0555, 0x064E
	,{-1,		150, 85, 	100 }	/* -5～0	*//* 4	*/	//0x0974, 0x0555, 0x064E
	,{4,		150, 85, 	100 }	/*  0～5	*//* 5	*/	//0x0974, 0x0555, 0x064E
	,{9,		150, 85, 	100 }	/*  5～10	*//* 6	*/	//0x0974, 0x0555, 0x064E
	,{14,		150, 85, 	100 }	/* 10～15	*//* 7	*/	//0x0974, 0x0555, 0x064E
	,{19,		146, 80, 	100 }	/* 15～20	*//* 8	*/	//0x092A, 0x050B, 0x064E
	,{24,		142, 74, 	100 }	/* 20～25	*//* 9	*/	//0x08EC, 0x04B4, 0x064E
	,{29,		138, 70, 	100 }	/* 25～30	*//* 10 */	//0x08AE, 0x046A, 0x064E
	,{34,		133, 64, 	100 }	/* 30～35	*//* 11 */	//0x0863, 0x0413, 0x064E
	,{39,		129, 60, 	100 }	/* 35～40	*//* 12 */	//0x0825, 0x03C8, 0x064E
	,{44,		125, 54, 	100 }	/* 40～45	*//* 13 */	//0x07DB, 0x0371, 0x064E
	,{49,		120, 50, 	100 }	/* 45～50	*//* 14 */	//0x0790, 0x0327, 0x064E
	,{54,		115, 44, 	100 }	/* 50～55	*//* 15 */	//0x0739, 0x02D0, 0x064E
	,{59,		110, 40, 	100 }	/* 55～60	*//* 16 */	//0x06EF, 0x0285, 0x064E
	,{64,		105, 34, 	100 }	/* 60～65	*//* 17 */	//0x0698, 0x022F, 0x064E
	,{69,		105, 34, 	100 }	/* 65～75	*//* 18 */	//0x0698, 0x022F, 0x064E
	,{74,		105, 34, 	100 }	/* 70～75	*//* 19 */	//0x0698, 0x022F, 0x064E
	,{79,		105, 34, 	100 }	/* 75～80	*//* 20 */	//0x0698, 0x022F, 0x064E
	,{125,		105, 34, 	100 }	/* 80	*//* 21 */		//0x0698, 0x022F, 0x064E 
	};

#else
	//SS
	//SSは押し込み初動は電圧可変で、温度による違いはなし
	//その為、Fwd1は使用していない、代わりに smot_dac_table_uba_1st_0 を使用している
	//Fw2は使用している
	{
	/*  temp	Fwd 1st(not use)		Fwd 2nd		Rev 1st(0.6A)*/
	{-21,   	167, 118, 	101 }	/* -20	*//* 0	*/
	,{-16,		167, 118, 	101 }	/* -20～-15	*//* 1	*/
	,{-11,		167, 118, 	101 }	/* -15～-10	*//* 2	*/
	,{-6,		167, 118, 	101 }	/* -10～-5	*//* 3	*/
	,{-1,		167, 118, 	101 }	/* -5～0	*//* 4	*/
	,{4,		167, 118, 	101 }	/*  0～5	*//* 5	*/
	,{9,		167, 118, 	101 }	/*  5～10	*//* 6	*/
	,{14,		167, 118, 	101 }	/* 10～15	*//* 7	*/
	,{19,		162, 112, 	101 }	/* 15～20	*//* 8	*/
	,{24,		157, 108, 	101 }	/* 20～25	*//* 9	*/
	,{29,		152, 102, 	101 }	/* 25～30	*//* 10 */
	,{34,		148,  97, 	101 }	/* 30～35	*//* 11 */
	,{39,		142,  92, 	101 }	/* 35～40	*//* 12 */
	,{44,		138,  87, 	101 }	/* 40～45	*//* 13 */
	,{49,		132,  82, 	101 }	/* 45～50	*//* 14 */
	,{54,		128,  77, 	101 }	/* 50～55	*//* 15 */
	,{59,		122,  72, 	101 }	/* 55～60	*//* 16 */
	,{64,		118,  67, 	101 }	/* 60～65	*//* 17 */
	,{69,		118,  67, 	101 }	/* 65～75	*//* 18 */
	,{74,		118,  67, 	101 }	/* 70～75	*//* 19 */
	,{79,		118,  67, 	101 }	/* 75～80	*//* 20 */
	,{125,		118,  67, 	101 }	/* 80	*//* 21 */
	};
#endif


void _motor_add_next_start(u8 motor_no);
void _motor_shift_next_start(void);
void _motor_remove_next_start(u8 motor_no);

void _motor_set_mode(u16 mode);
void _motor_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _motor_system_error(u8 fatal_err, u8 code);

/************************** EXTERN FUNCTIONS *************************/

/*******************************
        motor_task
 *******************************/
void motor_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u16 time_out;

	_motor_initialize_proc();				/* motor task initialize */

	while(1)
	{
		_motor_idel_proc();

		if ((s_motor_feed_state == MOTOR_STATE_STOP)
		 && (s_motor_centering_state == MOTOR_STATE_STOP)
		 && (s_motor_shutter_state == MOTOR_STATE_STOP)
		 && (s_motor_apb_state == MOTOR_STATE_STOP))
		{

			time_out = 10;
		}
		else
		{
			time_out = 1;
		}

		ercd = trcv_mbx(ID_MOTOR_MBX, (T_MSG **)&tmsg_pt, time_out);
		if(ercd == E_OK)
		{
			memcpy(&motor_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(motor_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_motor_system_error(1, 3);
			}
			_motor_msg_proc();
		}
	}
}


void _motor_initialize_proc(void)
{
	s_motor_feed_state = MOTOR_STATE_STOP;
	s_motor_centering_state = MOTOR_STATE_STOP;
	s_motor_apb_state = MOTOR_STATE_STOP;
	s_motor_shutter_state = MOTOR_STATE_STOP;

	memset(s_motor_start_waiting, 0, sizeof(s_motor_start_waiting));
	_motor_set_mode(MOTOR_TASK_MODE_READY);
}


void _motor_set_limit_start(UB type, UB dir )
{

	u16 motor_limit;

	switch (type)
	{
		case MOTOR_NUM_FEED:
			_pl_set_hmot_cur_uba();
			break;
		
		case MOTOR_NUM_APB:
			_pl_set_pbmot_cur();
			break;

		case MOTOR_NUM_STACKER:
			// 収納は時間ではなく、パルス数到達でDAを切り替えるので
			_pl_set_smot_cur_uba(dir);

			//動作開始時はFWDもREVは通常DAポート使用
			_pl_change_staker_da_uba(0);	/* 2022-01-31 */
			break;

		case MOTOR_NUM_CENTERING:
			_pl_set_cmot_cur();
			break;

		case MOTOR_NUM_SHUTTER:
			_pl_set_shutter_mot_cur();
			break;

		default:					/* other */
			break;
	}
}


/******************************************************************************/
/*! @brief MBX message procedure
    @return         none
    @exception      none
******************************************************************************/
void _motor_msg_proc(void)
{
	switch (motor_msg.tmsg_code)
	{
	case TMSG_MOTOR_FEED_FWD_REQ:		/* feed motor forward message */
		if (_ir_feed_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_feed_state = MOTOR_STATE_FWD_WAIT;
			_motor_add_next_start(MOTOR_NUM_FEED);
		}
		break;
	case TMSG_MOTOR_FEED_REV_REQ:		/* feed motor revers message */
		if (_ir_feed_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_feed_state = MOTOR_STATE_REV_WAIT;
			_motor_add_next_start(MOTOR_NUM_FEED);
		}
		break;
	case TMSG_MOTOR_FEED_STOP:			/* feed motor stop message */
		//s_motor_feed_state = MOTOR_STATE_BRAKE;
		break;
	case TMSG_MOTOR_STACKER_FWD_REQ:		/* stacker motor forward message */
		if (_ir_stacker_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_stacker_state = MOTOR_STATE_FWD_WAIT;
			_motor_add_next_start(MOTOR_NUM_STACKER);
		}
		break;
	case TMSG_MOTOR_STACKER_REV_REQ:		/* stacker motor revers message */
		if (_ir_stacker_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_stacker_state = MOTOR_STATE_REV_WAIT;
			_motor_add_next_start(MOTOR_NUM_STACKER);
		}
		break;
	case TMSG_MOTOR_STACKER_STOP:			/* stacker motor stop message */
		//s_motor_stacker_state = MOTOR_STATE_BRAKE;
		break;
	case TMSG_MOTOR_CENTERING_FWD_REQ:	/* centering motor forward message */
		if (_ir_centering_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_centering_state = MOTOR_STATE_FWD_WAIT;
			_motor_add_next_start(MOTOR_NUM_CENTERING);
		}
		break;
	case TMSG_MOTOR_CENTERING_REV_REQ:	/* centering motor revers message */
		if (_ir_centering_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_centering_state = MOTOR_STATE_REV_WAIT;
			_motor_add_next_start(MOTOR_NUM_CENTERING);
		}
		break;
	case TMSG_MOTOR_CENTERING_STOP:		/* centering motor stop message */
		//s_motor_centering_state = MOTOR_STATE_BRAKE;
		break;
	case TMSG_MOTOR_APB_FWD_REQ:		/* APB motor forward message */
		if (_ir_apb_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_apb_state = MOTOR_STATE_FWD_WAIT;
			_motor_add_next_start(MOTOR_NUM_APB);
		}
		break;
	case TMSG_MOTOR_APB_REV_REQ:		/* APB motor revers message */
		if (_ir_apb_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_apb_state = MOTOR_STATE_REV_WAIT;
			_motor_add_next_start(MOTOR_NUM_APB);
		}
		break;
	case TMSG_MOTOR_APB_STOP:			/* APB motor stop message */
		//s_motor_apb_state = MOTOR_STATE_BRAKE;
		break;
	case TMSG_MOTOR_SHUTTER_FWD_REQ:		/* shutter motor forward message */
		if (_ir_shutter_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_shutter_state = MOTOR_STATE_FWD_WAIT;
			_motor_add_next_start(MOTOR_NUM_SHUTTER);
		}
		break;
	case TMSG_MOTOR_SHUTTER_REV_REQ:		/* shutter motor revers message */
		if (_ir_shutter_motor_ctrl.mode == MOTOR_STOP)
		{
			s_motor_shutter_state = MOTOR_STATE_REV_WAIT;
			_motor_add_next_start(MOTOR_NUM_SHUTTER);
		}
		break;
	case TMSG_MOTOR_SHUTTER_STOP:			/* shutter motor stop message */
		//s_motor_stacker_state = MOTOR_STATE_BRAKE;
		break;
	default:					/* other */
		break;
	}
}


/******************************************************************************/
/*! @brief motor task idel procedure
    @return         none
    @exception      none
******************************************************************************/
void _motor_idel_proc(void)
{
	/* Check motor stop */
	_motor_stop_proc();

	/* Check motor start */
	_motor_start_proc();

}


void _motor_stop_proc(void)
{
	/* feed motor */
	if (((_ir_feed_motor_ctrl.mode == MOTOR_BRAKE) && (_ir_feed_motor_ctrl.stop_time == 0))
	 || ((_ir_feed_motor_ctrl.mode == MOTOR_BRAKE_FWD) && (_ir_feed_motor_ctrl.stop_time == 0))
	 || ((_ir_feed_motor_ctrl.mode == MOTOR_BRAKE_REV) && (_ir_feed_motor_ctrl.stop_time == 0))
	) {
		s_motor_feed_state = MOTOR_STATE_STOP;
		_ir_feed_motor_ctrl.mode = MOTOR_STOP;
		set_flg(ID_FEED_CTRL_FLAG, EVT_FEED_MOTOR_STOP);

	}
	/* stacker motor */
	if (((_ir_stacker_motor_ctrl.mode == MOTOR_BRAKE) && (_ir_stacker_motor_ctrl.stop_time == 0))
	 || ((_ir_stacker_motor_ctrl.mode == MOTOR_BRAKE_FWD) && (_ir_stacker_motor_ctrl.stop_time == 0))
	 || ((_ir_stacker_motor_ctrl.mode == MOTOR_BRAKE_REV) && (_ir_stacker_motor_ctrl.stop_time == 0))
	) {
		s_motor_stacker_state = MOTOR_STATE_STOP;
		_ir_stacker_motor_ctrl.mode = MOTOR_STOP;
		set_flg(ID_STACKER_CTRL_FLAG, EVT_STACKER_MOTOR_STOP);
	}

	/* centering motor */
	if ((_ir_centering_motor_ctrl.mode == MOTOR_BRAKE) && (_ir_centering_motor_ctrl.stop_time == 0))
	{
		s_motor_centering_state = MOTOR_STATE_STOP;
		_ir_centering_motor_ctrl.mode = MOTOR_STOP;
		set_flg(ID_CENTERING_CTRL_FLAG, EVT_CENTERING_MOTOR_STOP);
	}

	/* APB motor */
	if ((_ir_apb_motor_ctrl.mode == MOTOR_BRAKE) && (_ir_apb_motor_ctrl.stop_time == 0))
	{
		s_motor_apb_state = MOTOR_STATE_STOP;
		_ir_apb_motor_ctrl.mode = MOTOR_STOP;
		set_flg(ID_APB_CTRL_FLAG, EVT_APB_MOTOR_STOP);
	}
	if ((_ir_shutter_motor_ctrl.mode == MOTOR_BRAKE) && (_ir_shutter_motor_ctrl.stop_time == 0))
	{
		s_motor_shutter_state = MOTOR_STATE_STOP;
		_ir_shutter_motor_ctrl.mode = MOTOR_STOP;
		set_flg(ID_SHUTTER_CTRL_FLAG, EVT_SHUTTER_MOTOR_STOP);
	}

}


void _motor_start_proc(void)
{
	if (s_motor_start_waiting[0] != 0)
	{
		if (_ir_motor_disable_time == 0)
		{
		/* Motor �����N���֎~���Ԃ�0�̏ꍇ */
			switch (s_motor_start_waiting[0])
			{
			case MOTOR_NUM_FEED:
				if (_ir_feed_motor_ctrl.stop_time == 0)
				{
				/* Feed Motor ��~���Ԃ�0�̏ꍇ */
					if (s_motor_feed_state == MOTOR_STATE_FWD_WAIT)
					{
						s_motor_feed_state = MOTOR_STATE_FWD;
						_motor_start_feed(s_motor_feed_state);
					}
					else if (s_motor_feed_state == MOTOR_STATE_REV_WAIT)
					{
						s_motor_feed_state = MOTOR_STATE_REV;
						_motor_start_feed(s_motor_feed_state);
					}
					_motor_shift_next_start();
				}
				break;
			case MOTOR_NUM_STACKER:
				if (_ir_stacker_motor_ctrl.stop_time == 0)
				{
				/* Stacker Motor ��~���Ԃ�0�̏ꍇ */
					if (s_motor_stacker_state == MOTOR_STATE_FWD_WAIT)
					{
						s_motor_stacker_state = MOTOR_STATE_FWD;
						_motor_start_stacker(s_motor_stacker_state);
					}
					else if (s_motor_stacker_state == MOTOR_STATE_REV_WAIT)
					{
						s_motor_stacker_state = MOTOR_STATE_REV;
						_motor_start_stacker(s_motor_stacker_state);
					}
					_motor_shift_next_start();
				}
				break;
			case MOTOR_NUM_CENTERING:
				if (_ir_centering_motor_ctrl.stop_time == 0)
				{
				/* Centering Motor ��~���Ԃ�0�̏ꍇ */
					if (s_motor_centering_state == MOTOR_STATE_FWD_WAIT)
					{
						s_motor_centering_state = MOTOR_STATE_FWD;
						_motor_start_centering(s_motor_centering_state);
					}
					else if (s_motor_centering_state == MOTOR_STATE_REV_WAIT)
					{
						s_motor_centering_state = MOTOR_STATE_REV;
						_motor_start_centering(s_motor_centering_state);
					}
					_motor_shift_next_start();
				}
				break;
			case MOTOR_NUM_APB:
				if (_ir_apb_motor_ctrl.stop_time == 0)
				{
				/* Centering Motor ��~���Ԃ�0�̏ꍇ */
					if (s_motor_apb_state == MOTOR_STATE_FWD_WAIT)
					{
						s_motor_apb_state = MOTOR_STATE_FWD;
						_motor_start_apb(s_motor_apb_state);
					}
					else if (s_motor_apb_state == MOTOR_STATE_REV_WAIT)
					{
						s_motor_apb_state = MOTOR_STATE_REV;
						_motor_start_apb(s_motor_apb_state);
					}
					_motor_shift_next_start();
				}
				break;

			case MOTOR_NUM_SHUTTER:
				if (_ir_shutter_motor_ctrl.stop_time == 0)
				{

					if (s_motor_shutter_state == MOTOR_STATE_FWD_WAIT)
					{
						s_motor_shutter_state = MOTOR_STATE_FWD;
						_motor_start_shutter(s_motor_shutter_state);
					}
					else if (s_motor_shutter_state == MOTOR_STATE_REV_WAIT)
					{
						s_motor_shutter_state = MOTOR_STATE_REV;
						_motor_start_shutter(s_motor_shutter_state);
					}
					_motor_shift_next_start();
				}
				break;

			default:
				_motor_shift_next_start();
				break;
			}
		}
	}
}


void _motor_start_feed(u8 dir)
{

	_motor_set_limit_start(MOTOR_NUM_FEED,dir);

	_ir_motor_disable_time = NEXT_MOTOR_START_WAIT_TIME;
	if (dir == MOTOR_STATE_FWD)
	{
		_ir_feed_motor_ctrl.mode = MOTOR_FWD;
		_pl_feed_motor_cw(_ir_feed_motor_ctrl.speed);
	}
	else
	{
		_ir_feed_motor_ctrl.mode = MOTOR_REV;
		_pl_feed_motor_ccw(_ir_feed_motor_ctrl.speed);
	}
}


void _motor_start_stacker(u8 dir)
{

	_motor_set_limit_start(MOTOR_NUM_STACKER,dir);

	_ir_motor_disable_time = STACKER_NEXT_MOTOR_START_WAIT_TIME;

	if (dir == MOTOR_STATE_FWD)
	{
		_ir_stacker_motor_ctrl.mode = MOTOR_FWD;
		_pl_stacker_motor_ccw_uba();
	}
	else
	{
		_ir_stacker_motor_ctrl.mode = MOTOR_REV;
		_pl_stacker_motor_cw_uba();
	}
}


void _motor_start_centering(u8 dir)
{

	_motor_set_limit_start(MOTOR_NUM_CENTERING,dir);

	_ir_motor_disable_time = NEXT_MOTOR_START_WAIT_TIME;
	if (dir == MOTOR_STATE_FWD)
	{
		_ir_centering_motor_ctrl.mode = MOTOR_FWD;
		_pl_centering_motor_cw(_ir_centering_motor_ctrl.speed);	/* centering motor clockwise */
	}
	else
	{
		_ir_centering_motor_ctrl.mode = MOTOR_REV;
		_pl_centering_motor_ccw(_ir_centering_motor_ctrl.speed);	/* centering motor counter clockwise */
	}
}


void _motor_start_apb(u8 dir)
{

	_motor_set_limit_start(MOTOR_NUM_APB,dir);

	_ir_motor_disable_time = NEXT_MOTOR_START_WAIT_TIME;
	if (dir == MOTOR_STATE_FWD)
	{
		_ir_apb_motor_ctrl.mode = MOTOR_FWD;
		_pl_apb_motor_cw(_ir_apb_motor_ctrl.pwm);
	}
	else
	{
		_ir_apb_motor_ctrl.mode = MOTOR_REV;
		_pl_apb_motor_ccw(_ir_apb_motor_ctrl.pwm);
	}
	pb_rev_time = APB_REV_TIME;
}


void _motor_start_shutter(u8 dir)
{

	_motor_set_limit_start(MOTOR_NUM_SHUTTER,dir);

	_ir_motor_disable_time = NEXT_MOTOR_START_WAIT_TIME;
	if (dir == MOTOR_STATE_FWD)
	{
		_ir_shutter_motor_ctrl.mode = MOTOR_FWD;
		_pl_shutter_motor_ccw(_ir_shutter_motor_ctrl.pwm);
	}
	else
	{
		_ir_shutter_motor_ctrl.mode = MOTOR_REV;
		_pl_shutter_motor_cw(_ir_shutter_motor_ctrl.pwm);
	}
}


void _motor_add_next_start(u8 motor_no)
{
	u8 cnt;

	if ((motor_no > 0) && (motor_no <= MOTOR_MAX_NUMBER))
	{
		_motor_remove_next_start(motor_no);
		for (cnt = 0; cnt < MOTOR_MAX_NUMBER; cnt++)
		{
			if (s_motor_start_waiting[cnt] == 0)
			{
				s_motor_start_waiting[cnt] = motor_no;
				break;
			}
		}
	}
}


void _motor_shift_next_start(void)
{
	u8 cnt;

	for (cnt = 0; cnt < (MOTOR_MAX_NUMBER - 1); cnt++)
	{
		s_motor_start_waiting[cnt] = s_motor_start_waiting[(cnt + 1)];
	}
	s_motor_start_waiting[cnt] = 0;
}


void _motor_remove_next_start(u8 motor_no)
{
	u8 cnt;
	u8 shift;

	for (cnt = 0; cnt < MOTOR_MAX_NUMBER; cnt++)
	{
		if (s_motor_start_waiting[cnt] == motor_no)
		{
			for (shift = 0; (cnt + shift) < (MOTOR_MAX_NUMBER - 1); shift++)
			{
				s_motor_start_waiting[(cnt+shift)] = s_motor_start_waiting[(cnt+shift+1)];
			}
			s_motor_start_waiting[(cnt + shift)] = 0;
			break;
		}
	}
}


/*********************************************************************//**
 * @brief set task mode
 * @param[in]	mode : task mode
 * @return 		None
 **********************************************************************/
void _motor_set_mode(u16 mode)
{
	ex_motor_task_mode = mode;

#if (_ENABLE_JDL==1)
    jdl_add_trace(ID_MOTOR_TASK, ((ex_motor_task_mode >> 8) & 0xFF), (ex_motor_task_mode & 0xFF), 0, 0, 0);
#endif /* _ENABLE_JDL */
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
void _motor_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_MOTOR_TASK;
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
			_motor_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_motor_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _motor_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_motor_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_motor_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_MOTOR_TASK, (u16)code, (u16)motor_msg.tmsg_code, (u16)motor_msg.arg1, fatal_err);
}


/* EOF */
