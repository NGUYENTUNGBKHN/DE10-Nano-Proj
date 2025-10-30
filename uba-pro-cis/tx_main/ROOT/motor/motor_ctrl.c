/******************************************************************************/
/*! @addtogroup Group2
    @file       motor_task.c
    @brief      motor task process
    @date       2018/02/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/

/***************************** Include Files *********************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "MP_Mutexes.h"
#include "custom.h"
#include "common.h"
#include "pl/pl.h"
#include "pl/pl_motor.h"
#include "motor_ctrl.h"
#include "hal_gpio.h"



#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "jsl_ram.c"

#ifdef _ENABLE_JDL
#include "jdl_conf.h"
#endif	/* _ENABLE_JDL */

static void _motor_ctrl_feed_start_clr_val(u16 speed, u16 drive_pulse);
static void _motor_ctrl_centering_start_clr_val(u16 speed, u16 drive_home_cnt);

void _motor_ctrl_apb_start_clr_val_puls(u8 pwm, u8 drive_home_cnt, u8 drive_pulse);
static void _motor_ctrl_apb_start_clr_val(u8 pwm, u16 drive_home_cnt);
static void _motor_ctrl_stacker_start_clr_val_uba(u16 drive_pulse);

static void _motor_ctrl_stacker_start_clr_val(u16 speed, u16 drive_pulse, u16 current_max);
static void _motor_ctrl_disable_intr(u8 intr_num);
static void _motor_ctrl_enable_intr(u8 intr_num);

static void _motor_ctrl_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/
extern void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _main_system_error(u8 fatal_err, u8 code);
extern void _feed_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _feed_system_error(u8 fatal_err, u8 code);
extern void _centering_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _centering_system_error(u8 fatal_err, u8 code);
extern void _apb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _apb_system_error(u8 fatal_err, u8 code);
extern void _stacker_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _stacker_system_error(u8 fatal_err, u8 code);


/************************** Private declaration *****************************/
#define PIN_FEDENC 0
#define PIN_CENTHP 1
#define PIN_PBHP   2
#define PIN_STKENC 3
#define PIN_PBENC 4

/************************** Variable declaration *****************************/

/*********************************************************************//**
 * motor max current control
 **********************************************************************/
u8 init_motor_max_current(void)
{
	u8 rtn = ALARM_CODE_OK;
	// motor
	/* Feed */
	if(_pl_set_hmot_cur_uba() != ALARM_CODE_OK)
		rtn = ALARM_CODE_PL_SPI;
	/* Stacker */
	motor_limit_stacker_table_index = 0;	/* for stacker full *//* 温度によって、変更が必要、起動時は温度未取得なので、ここで初期値設定しておく */
	if(_pl_set_smot_cur_uba(0xFF) != ALARM_CODE_OK) //起動時の1回目
		rtn = ALARM_CODE_PL_SPI;
	/* PB */
	if(motor_ctrl_pb_set_max_current(PB_MOTOR_MAX_CURRENT) != ALARM_CODE_OK)
		rtn = ALARM_CODE_PL_SPI;
	/* Shutter */
	if(motor_ctrl_shutter_set_max_current(SHUTTER_MOTOR_MAX_CURRENT) != ALARM_CODE_OK)
		rtn = ALARM_CODE_PL_SPI;
	if(motor_ctrl_centering_set_max_current(CENTERING_MOTOR_MAX_CURRENT) != ALARM_CODE_OK)
		rtn = ALARM_CODE_PL_SPI;
	return rtn;
}
/*********************************************************************//**
 * feed motor control
 **********************************************************************/
/*********************************************************************//**
 * @brief forward feed motor
 * @param[in]	motor speed
 * 				drive pulse
 * @return 		error code
 **********************************************************************/
u8 motor_ctrl_feed_fwd(u16 speed, u16 drive_pulse)
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_feed_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_feed_start_clr_val(speed, drive_pulse);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_FEED_FWD_REQ, 0, 0, 0, 0);		
	}
	else if (_ir_feed_motor_ctrl.mode == MOTOR_FWD)
	{
		_motor_ctrl_feed_start_clr_val(speed, drive_pulse);
		_pl_feed_motor_cw(speed);				/* drive feed motor counter clockwise */
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}


/*********************************************************************//**
 * @brief revers feed motor
 * @param[in]	motor speed
 * 				drive pulse
 * @return 		error code
 **********************************************************************/
u8 motor_ctrl_feed_rev(u16 speed, u16 drive_pulse)
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_feed_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_feed_start_clr_val(speed, drive_pulse);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_FEED_REV_REQ, 0, 0, 0, 0);
	}
	else if (_ir_feed_motor_ctrl.mode == MOTOR_REV)
	{
		_motor_ctrl_feed_start_clr_val(speed, drive_pulse);
		_pl_feed_motor_ccw(speed);				/* drive feed motor clockwise */
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}
/*********************************************************************//**
 * @brief clear feed motor control value
 * @param[in]	motor speed
 * 				drive pulse
 * @return 		None
 **********************************************************************/
static void _motor_ctrl_feed_start_clr_val(u16 speed, u16 drive_pulse)
{
	_motor_ctrl_disable_intr(PIN_FEDENC);	/* 搬送モータ エンコーダー割り込み禁止 */

#if 1//#if (_DEBUG_DELETE_DISABLE_IRQ==1)
	while(1)
	{
		_ir_feed_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア

		if(_ir_feed_motor_ctrl.pulse == 0) break;
	};
#else
	_ir_feed_motor_ctrl.pulse = 0;
#endif
	_ir_feed_motor_ctrl.drive_pulse = drive_pulse;
	_ir_feed_motor_ctrl.lock_count = 0;
	_ir_feed_motor_ctrl.run_time = 0;
	_ir_feed_motor_ctrl.speed = speed;
	_ir_feed_motor_ctrl.reject = 0;

	_ir_feed_motor_ctrl.over_pulse = 0;

	_motor_ctrl_enable_intr(PIN_FEDENC);	/* 搬送モータ エンコーダー割り込み許可 */
}

/*********************************************************************//**
 * @brief stop feed motor
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_feed_stop(void)
{
#if 1  /* DEBUG info */
	memcpy(&_debug_feed_motor_ctrl, &_ir_feed_motor_ctrl, sizeof(_ir_feed_motor_ctrl));
#endif /* DEBUG info */
	_motor_ctrl_disable_intr(PIN_FEDENC);	/* 搬送モータ エンコーダー割り込み禁止 */

	_ir_feed_motor_ctrl.drive_pulse = 0;
	_ir_feed_motor_ctrl.pulse = 0;

	_ir_feed_motor_ctrl.lock_count = 0;
	_ir_feed_motor_ctrl.run_time = 0;
	_ir_feed_motor_ctrl.stop_time = FEED_MOTOR_STOP_TIME;
	_ir_feed_motor_ctrl.speed = 0;
	_ir_feed_motor_ctrl.reject = 0;

	_ir_feed_motor_ctrl.over_pulse = 0;

	if(_ir_feed_motor_ctrl.mode == MOTOR_FWD)
	{
		_ir_feed_motor_ctrl.mode = MOTOR_BRAKE_FWD;
	}
	else if(_ir_feed_motor_ctrl.mode == MOTOR_REV)
	{
		_ir_feed_motor_ctrl.mode = MOTOR_BRAKE_REV;
	}
	else
	{
		_ir_feed_motor_ctrl.mode = MOTOR_BRAKE;
	}

	_motor_ctrl_enable_intr(PIN_FEDENC);	/* 搬送モータ エンコーダー割り込み許可 */

	_pl_feed_motor_stop();					/* stop feed motor */
	_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_FEED_STOP, 0, 0, 0, 0);

#ifdef _ENABLE_JDL
	jdl_move_feed(_debug_feed_motor_ctrl.run_time);
#endif	/* _ENABLE_JDL */
}

/*********************************************************************//**
 * @brief set feed drive pulse
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_feed_set_pulse(u16 drive_pulse)
{
	_motor_ctrl_disable_intr(PIN_FEDENC);	/* 搬送モータ エンコーダー割り込み禁止 */

#if 1//#if (_DEBUG_DELETE_DISABLE_IRQ==1)
	while(1)
	{
		_ir_feed_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア

		if(_ir_feed_motor_ctrl.pulse == 0) break;
	};
#else
	_ir_feed_motor_ctrl.pulse = 0;
#endif
	_ir_feed_motor_ctrl.drive_pulse = drive_pulse;
	_ir_feed_motor_ctrl.over_pulse = 0;

	clr_flg(ID_FEED_CTRL_FLAG, ~(EVT_FEED_OVER_PULSE));	 //追加
	
	_motor_ctrl_enable_intr(PIN_FEDENC);	/* 搬送モータ エンコーダー割り込み許可 */
}


/*********************************************************************//**
 * @brief feed motor status
 * @param[in]	None
 * @return 		true  : stopped
 * 				false : running
 **********************************************************************/
bool is_motor_ctrl_feed_stop(void)
{
	if (_ir_feed_motor_ctrl.mode == MOTOR_STOP)
	{
		return true;
	}
	else
	{
		return false;
	}
}




/*********************************************************************//**
 * centering motor control
 **********************************************************************/
/*********************************************************************/
void set_centering_timeout(BOOL flag, u16 timeout)
{
	ex_centor_motor_run_time = timeout;
	ex_centor_motor_run = flag;
}


u8 motor_ctrl_centering_fwd(u8 speed, u8 drive_home_cnt, u16 timeout)
{
	u8 rtn = IERR_CODE_OK;

	ex_centor_motor_run = 0;
	ex_centor_motor_run_time = timeout;

	if (_ir_centering_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_centering_start_clr_val(speed, drive_home_cnt);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_CENTERING_FWD_REQ, 0, 0, 0, 0);
	}
	else if (_ir_centering_motor_ctrl.mode == MOTOR_FWD)
	{
		_motor_ctrl_centering_start_clr_val(speed, drive_home_cnt);
		_pl_centering_motor_cw(speed);			/* centering motor clockwise */
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}

/*********************************************************************//**
 * @brief revers centering motor
 * @param[in]	motor speed
 * 				drive home count
 * @return 		error code
 **********************************************************************/
u8 motor_ctrl_centering_rev(u8 speed, u8 drive_home_cnt, u16 timeout)
{
	u8 rtn = IERR_CODE_OK;

	//ここのみ異なる
	ex_centor_motor_run = 0;
	ex_centor_motor_run_time = timeout;

	if (_ir_centering_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_centering_start_clr_val(speed, drive_home_cnt);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_CENTERING_REV_REQ, 0, 0, 0, 0);
	}
	else if (_ir_centering_motor_ctrl.mode == MOTOR_REV)
	{
		_motor_ctrl_centering_start_clr_val(speed, drive_home_cnt);
		_pl_centering_motor_ccw(speed);			/* centering motor counter clockwise */
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}


/*********************************************************************//**
 * @brief stop centering motor
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_centering_stop(void)
{
#if 1  /* DEBUG info */
//	memcpy(&_debug_centering_motor_ctrl, &_ir_centering_motor_ctrl, sizeof(_ir_centering_motor_ctrl));
#endif /* DEBUG info */

	_motor_ctrl_disable_intr(PIN_CENTHP);	/* 幅寄モータ エンコーダー割り込み許可 */

	_ir_centering_motor_ctrl.drive_home_cnt = 0;

	_ir_centering_motor_ctrl.open_cnt = 0;
	_ir_centering_motor_ctrl.close_cnt = 0;

	_ir_centering_motor_ctrl.event_time = 0;
	_ir_centering_motor_ctrl.prev_time = 0;
	_ir_centering_motor_ctrl.run_time = 0;
	_ir_centering_motor_ctrl.stop_time = CENTERING_MOTOR_STOP_TIME;
	_ir_centering_motor_ctrl.speed = 0;
	_ir_centering_motor_ctrl.mode = MOTOR_BRAKE;

	_motor_ctrl_enable_intr(PIN_CENTHP);	/* 幅寄モータ エンコーダー割り込み許可 */

	_pl_centering_motor_stop();				/* stop centering motor */
	_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_CENTERING_STOP, 0, 0, 0, 0);

#ifdef _ENABLE_JDL
	jdl_move_centering(0);
#endif	/* _ENABLE_JDL */
}

/*********************************************************************//**
 * @brief set centering max current
 * @param[in]	current_max speed
 * @return 		error code
 **********************************************************************/
u8 motor_ctrl_centering_set_max_current(u16 current_max)
{
	u8 ret = ALARM_CODE_OK;

	_ir_centering_motor_ctrl.max_current = 0xFF & current_max;

	if(ret == ALARM_CODE_OK)
	{
		ret = _pl_set_cmot_cur();
	}

	return ret;
}



/*********************************************************************//**
 * @brief centering motor status
 * @param[in]	None
 * @return 		true  : stopped
 * 				false : running
 **********************************************************************/
bool is_motor_ctrl_centering_stop(void)
{
	if (_ir_centering_motor_ctrl.mode == MOTOR_STOP)
	{
		ex_centor_motor_run = 0;
		return true;
	}
	else
	{
		return false;
	}
}


/*********************************************************************//**
 * @brief clear centering motor control value
 * @param[in]	motor speed
 * 				drive pulse
 * @return 		None
 **********************************************************************/
void _motor_ctrl_centering_start_clr_val(u16 speed, u16 drive_home_cnt)
{
	_motor_ctrl_disable_intr(PIN_CENTHP);	/* 幅寄モータ エンコーダー割り込み許可 */

	_ir_centering_motor_ctrl.drive_home_cnt = drive_home_cnt;

	_ir_centering_motor_ctrl.open_cnt = 0;
	_ir_centering_motor_ctrl.close_cnt = 0;

	_ir_centering_motor_ctrl.event_time = 0;
	_ir_centering_motor_ctrl.prev_time = 0;
	_ir_centering_motor_ctrl.run_time = 0;
	_ir_centering_motor_ctrl.stop_time = 0;
	_ir_centering_motor_ctrl.speed = speed;
	_ir_centering_motor_ctrl.status = 0;

	_motor_ctrl_enable_intr(PIN_CENTHP);	/* 幅寄モータ エンコーダー割り込み許可 */
}


/*********************************************************************//**
 * @brief forward apb motor
 * @param[in]	motor speed
 * 				drive home count
 * @return 		error code
 **********************************************************************/
u8 motor_ctrl_apb_fwd(u8 pwm, u16 drive_home_cnt)
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_apb_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_apb_start_clr_val(pwm, drive_home_cnt);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_APB_FWD_REQ, 0, 0, 0, 0); /* こののspeedはモータタスクで取得していない必要ない(1つ前の処理で、speedを保存している為) */
	}
	else if (_ir_apb_motor_ctrl.mode == MOTOR_FWD)
	{
		_motor_ctrl_apb_start_clr_val(pwm, drive_home_cnt);		
		_pl_apb_motor_cw(pwm);
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}

u8 motor_ctrl_apb_fwd_puls(u8 pwm, u8 drive_home_cnt, u8 drive_pulse) //追加、指定パルス後にイベント送信、いずれパルスなしと合わせこむ
{
	u8 rtn = IERR_CODE_OK;


	clr_flg(ID_APB_CTRL_FLAG, ~EVT_APB_OVER_PULSE);

	if (_ir_apb_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_apb_start_clr_val_puls(pwm, drive_home_cnt, drive_pulse);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_APB_FWD_REQ, 0, 0, 0, 0);/* こののspeedはモータタスクで取得していない必要ない(1つ前の処理で、speedを保存している為) */
	}
	else if (_ir_apb_motor_ctrl.mode == MOTOR_FWD)
	{
		_motor_ctrl_apb_start_clr_val_puls(pwm, drive_home_cnt, drive_pulse);
		_pl_apb_motor_cw(pwm);
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}

/*********************************************************************//**
 * @brief revers apb motor
 * @param[in]	motor speed
 * 				drive home count
 * @return 		error code
 **********************************************************************/
u8 motor_ctrl_apb_rev(u8 pwm, u16 drive_home_cnt)
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_apb_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_apb_start_clr_val(pwm, drive_home_cnt);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_APB_REV_REQ, 0, 0, 0, 0);
	}
	else if (_ir_apb_motor_ctrl.mode == MOTOR_REV)
	{
		_motor_ctrl_apb_start_clr_val(pwm, drive_home_cnt);
		_pl_apb_motor_ccw(pwm);				/* apb motor clockwise */
		pb_rev_time = APB_REV_TIME;
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}


/*********************************************************************//**
 * @brief stop apb motor
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_apb_stop(void)
{
#if 1  /* DEBUG info */
	memcpy(&_debug_apb_motor_ctrl, &_ir_apb_motor_ctrl, sizeof(_ir_apb_motor_ctrl));
#endif /* DEBUG info */

	_motor_ctrl_disable_intr(PIN_PBHP);	/* APBモータ  */
	_motor_ctrl_disable_intr(PIN_PBENC);	/* APBモータ エンコーダー割り込み許可 *///2022-01-11 


	_ir_apb_motor_ctrl.drive_home_cnt = 0;
	_ir_apb_motor_ctrl.home_cnt = 0;
	_ir_apb_motor_ctrl.event_time = 0;
	_ir_apb_motor_ctrl.prev_time = 0;
	_ir_apb_motor_ctrl.run_time = 0;
	_ir_apb_motor_ctrl.stop_time = APB_MOTOR_STOP_TIME;
//	_ir_apb_motor_ctrl.speed = 0;	/* PWM 0% */
	_ir_apb_motor_ctrl.pwm = 0;		/* PWM 0% */
	_ir_apb_motor_ctrl.mode = MOTOR_BRAKE;

	_motor_ctrl_enable_intr(PIN_PBHP);	/* APBモータ エンコーダー割り込み許可 */
	_motor_ctrl_enable_intr(PIN_PBENC);	/* APBモータ エンコーダー割り込み許可 *///2022-01-11 

	_pl_apb_motor_stop();				/* stop apb motor */
	_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_APB_STOP, 0, 0, 0, 0);

#ifdef _ENABLE_JDL
	jdl_move_apb(_debug_apb_motor_ctrl.run_time);
#endif	/* _ENABLE_JDL */
}


/*********************************************************************//**
 * @brief apb motor status
 * @param[in]	None
 * @return 		true  : stopped
 * 				false : running
 **********************************************************************/
bool is_motor_ctrl_apb_stop(void)
{
	if (_ir_apb_motor_ctrl.mode == MOTOR_STOP)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*********************************************************************//**
 * @brief clear apb motor control value
 * @param[in]	motor speed
 * 				drive pulse
 * @return 		None
 **********************************************************************/
void _motor_ctrl_apb_start_clr_val(u8 pwm, u16 drive_home_cnt)
{
	_motor_ctrl_disable_intr(PIN_PBHP);	/* APBモータ エンコーダー割り込み許可 */
	_motor_ctrl_disable_intr(PIN_PBENC);	/* APBモータ エンコーダー割り込み許可 *///2022-01-22


	/* UBA_MOTOR_SPEED_PWM, UBA_MOTOR_SPEED_FULL, UBA_MOTOR_SPEED_STOP */

	_ir_apb_motor_ctrl.drive_home_cnt = drive_home_cnt;
	_ir_apb_motor_ctrl.home_cnt = 0;
	_ir_apb_motor_ctrl.event_time = 0;
	_ir_apb_motor_ctrl.prev_time = 0;
	_ir_apb_motor_ctrl.run_time = 0;
	_ir_apb_motor_ctrl.stop_time = 0;
//	_ir_apb_motor_ctrl.speed = speed;	/*今はこれはPWMではなく、動作タイプ、PWM,100%,速度調整など*/
	_ir_apb_motor_ctrl.pwm = pwm;		/*pwmに統一*/

	_ir_apb_motor_ctrl.status = 0;

	_motor_ctrl_enable_intr(PIN_PBHP);	/* APBモータ エンコーダー割り込み許可 */
	_motor_ctrl_enable_intr(PIN_PBENC);	/* APBモータ エンコーダー割り込み許可 *///2022-01-22

}

void _motor_ctrl_apb_start_clr_val_puls(u8 pwm, u8 drive_home_cnt, u8 drive_pulse)
{
	_motor_ctrl_disable_intr(PIN_PBHP);
	_motor_ctrl_disable_intr(PIN_PBENC);	//2022-01-11

	#if 1 //2022-10-03
	_ir_apb_motor_ctrl.init_flag = 1;
	_ir_apb_motor_ctrl.init_value = drive_pulse;
	#else
	_ir_apb_motor_ctrl.pulse = 0;		// これを初期化しないと問題となる
	_ir_apb_motor_ctrl.drive_pulse = drive_pulse;
	#endif

	_ir_apb_motor_ctrl.drive_home_cnt = drive_home_cnt;
	_ir_apb_motor_ctrl.home_cnt = 0;
	_ir_apb_motor_ctrl.event_time = 0;
	_ir_apb_motor_ctrl.prev_time = 0;
	_ir_apb_motor_ctrl.run_time = 0;
	_ir_apb_motor_ctrl.stop_time = 0;
//	_ir_apb_motor_ctrl.speed = speed;	/*今はこれはPWMではなく、動作タイプ、PWM,100%,速度調整など*/
	_ir_apb_motor_ctrl.pwm = pwm;		/*pwmに統一*/

	_ir_apb_motor_ctrl.status = 0;

	_motor_ctrl_enable_intr(PIN_PBHP);
	_motor_ctrl_enable_intr(PIN_PBENC);	//2022-01-11


}
//#endif



/*********************************************************************//**
 * stacker motor control
 **********************************************************************/
u8 motor_ctrl_stacker_fwd_uba(u16 speed, u16 drive_pulse, u16 current_max) /* speed, current_maxはDA処理変更により使用してない*/
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_stacker_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_stacker_start_clr_val_uba(drive_pulse);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_STACKER_FWD_REQ, 0, 0, 0, 0);
	}
	else if (_ir_stacker_motor_ctrl.mode == MOTOR_FWD)
	{
		_motor_ctrl_stacker_start_clr_val_uba(drive_pulse);
		_pl_stacker_motor_ccw_uba();				/* drive stacker motor counter clockwise */
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}

u8 motor_ctrl_stacker_rev_uba(u16 speed, u16 drive_pulse, u16 current_max) /* speed, current_maxはDA処理変更により使用してない*/
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_stacker_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_stacker_start_clr_val_uba(drive_pulse);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_STACKER_REV_REQ, 0, 0, 0, 0);
	}
	else if (_ir_stacker_motor_ctrl.mode == MOTOR_REV)
	{
		_motor_ctrl_stacker_start_clr_val_uba(drive_pulse);
		_pl_stacker_motor_cw_uba();				/* drive stacker motor clockwise */
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}



static void _motor_ctrl_stacker_start_clr_val_uba(u16 drive_pulse)
{

	_motor_ctrl_disable_intr(PIN_STKENC);	/* 収納モータ エンコーダー割り込み禁止 */

#if 1//#if (_DEBUG_DELETE_DISABLE_IRQ==1)
	_ir_stacker_motor_ctrl.init_flag = 1;
	_ir_stacker_motor_ctrl.init_value = drive_pulse;
#else
	_ir_stacker_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア
	_ir_stacker_motor_ctrl.event_pulse = 0;
	_ir_stacker_motor_ctrl.drive_pulse = drive_pulse; 	//設定値になった場合にイベント発生 リミット値設定
#endif
	_ir_stacker_motor_ctrl.lock_count = 0;
	_ir_stacker_motor_ctrl.run_time = 0;

	//2023-07-04
	_ir_stacker_motor_ctrl.peakload_flag = 0;	//これは必ず必要 2023-07-04 OK
	_ir_stacker_motor_ctrl.full_check = 0;		//これは必ず必要 2023-07-04 OK

	_motor_ctrl_enable_intr(PIN_STKENC);	/* 収納モータ エンコーダー割り込み許可 */
}

/*********************************************************************//**
 * @brief stop stacker motor
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_stacker_stop(void)
{
	_motor_ctrl_disable_intr(PIN_STKENC);	/* 収納モータ エンコーダー割り込み禁止 */

	_ir_stacker_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア
	_ir_stacker_motor_ctrl.event_pulse = 0;
	_ir_stacker_motor_ctrl.drive_pulse = 0;
	_ir_stacker_motor_ctrl.lock_count = 0;
	_ir_stacker_motor_ctrl.run_time = 0;
	_ir_stacker_motor_ctrl.stop_time = STACKER_MOTOR_STOP_TIME;
	_ir_stacker_motor_ctrl.speed = 0;		//not use
	_ir_stacker_motor_ctrl.init_flag = 0;
	_ir_stacker_motor_ctrl.init_value = 0;
	if(_ir_stacker_motor_ctrl.mode == MOTOR_FWD)
	{
		_ir_stacker_motor_ctrl.mode = MOTOR_BRAKE_FWD;
	}
	else if(_ir_stacker_motor_ctrl.mode == MOTOR_REV)
	{
		_ir_stacker_motor_ctrl.mode = MOTOR_BRAKE_REV;
	}
	else
	{
		_ir_stacker_motor_ctrl.mode = MOTOR_BRAKE;
	}

	_motor_ctrl_enable_intr(PIN_STKENC);	/* 収納モータ エンコーダー割り込み禁止 */

	_pl_stacker_motor_stop();					/* stop stacker motor */
	_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_STACKER_STOP, 0, 0, 0, 0);

#ifdef _ENABLE_JDL
	jdl_move_stack(_ir_stacker_motor_ctrl.run_time);
#endif	/* _ENABLE_JDL */
}

/*********************************************************************//**
 * @brief set stacker drive pulse
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_stacker_set_pulse(u16 drive_pulse)
{
	_motor_ctrl_disable_intr(PIN_STKENC);	/* 収納モータ エンコーダー割り込み禁止 */

#if 1//#if (_DEBUG_DELETE_DISABLE_IRQ==1)
	_ir_stacker_motor_ctrl.init_flag = 1;
	_ir_stacker_motor_ctrl.init_value = drive_pulse;
#else
	_ir_stacker_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア
	_ir_stacker_motor_ctrl.event_pulse = 0;
	_ir_stacker_motor_ctrl.drive_pulse = drive_pulse; 	//設定値になった場合にイベント発生 リミット値設定
#endif

	_motor_ctrl_enable_intr(PIN_STKENC);	/* 収納モータ エンコーダー割り込み禁止 */
}

/*********************************************************************//**
 * @brief stacker motor status
 * @param[in]	None
 * @return 		true  : stopped
 * 				false : running
 **********************************************************************/
bool is_motor_ctrl_stacker_stop(void)
{
	if (_ir_stacker_motor_ctrl.mode == MOTOR_STOP)
	{
		return true;
	}
	else
	{
		return false;
	}
}



static void _motor_ctrl_disable_intr(u8 intr_num)
{

	ID task_id;
	ER ercd = E_OK;

	switch(intr_num)
	{
	case PIN_FEDENC:
		_pl_feed_enc_interrupt_disable();
		break;	
	case PIN_PBENC:
		_pl_pb_enc_interrupt_disable();
		break;
	case PIN_CENTHP:
		break;
	case PIN_PBHP:
		break;
	case PIN_STKENC:
		_pl_stacker_enc_interrupt_disable();
		break;
	default:
		ercd = E_OK - 1;
		break;
	}
	if (ercd != E_OK)
	{
		/* system error */
		ercd = get_tid(&task_id);

		switch (task_id)
		{
		case ID_FEED_TASK:
			_feed_system_error(1, 254);
			break;
		case ID_CENTERING_TASK:
			_centering_system_error(1, 254);
			break;
		case ID_APB_TASK:
			_apb_system_error(1, 254);
			break;
		case ID_STACKER_TASK:
			_stacker_system_error(1, 254);
			break;
		default:
			_main_system_error(1, 254);
			break;
		}
	}
}


static void _motor_ctrl_enable_intr(u8 intr_num)
{

	ID task_id;
	ER ercd = E_OK;

	switch(intr_num)
	{
	case PIN_FEDENC:
		_pl_feed_enc_interrupt_enable();
		break;
	case PIN_PBENC:
		_pl_pb_enc_interrupt_enable();
		break;
	case PIN_CENTHP:
		break;
	case PIN_PBHP:
		break;
	case PIN_STKENC:
		_pl_stacker_enc_interrupt_enable();
		break;
	default:
		ercd = E_OK - 1;
		break;
	}
	if (ercd != E_OK)
	{
		/* system error */
		ercd = get_tid(&task_id);

		switch (task_id)
		{
		case ID_FEED_TASK:
			_feed_system_error(1, 255);
			break;
		case ID_CENTERING_TASK:
			_centering_system_error(1, 255);
			break;
		case ID_APB_TASK:
			_apb_system_error(1, 255);
			break;
		case ID_STACKER_TASK:
			_stacker_system_error(1, 255);
			break;
		default:
			_main_system_error(1, 255);
			break;
		}
	}
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
void _motor_ctrl_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	ID task_id;

	get_tid(&task_id);

	switch (task_id)
	{
	case ID_FEED_TASK:
		_feed_send_msg(receiver_id, tmsg_code, arg1, arg2, arg3, arg4);
		break;
	case ID_CENTERING_TASK:
		_centering_send_msg(receiver_id, tmsg_code, arg1, arg2, arg3, arg4);
		break;
	case ID_APB_TASK:
		_apb_send_msg(receiver_id, tmsg_code, arg1, arg2, arg3, arg4);
		break;
	case ID_STACKER_TASK:
		_stacker_send_msg(receiver_id, tmsg_code, arg1, arg2, arg3, arg4);
		break;
	default:
		_main_send_msg(receiver_id, tmsg_code, arg1, arg2, arg3, arg4);
		break;
	}
}

static void _motor_ctrl_shutter_start_clr_val(u8 pwm, u16 drive_pulse)
{
	_ir_shutter_motor_ctrl.drive_pulse = drive_pulse;
	_ir_shutter_motor_ctrl.pulse = 0;
	_ir_shutter_motor_ctrl.event_pulse = 0;
	_ir_shutter_motor_ctrl.lock_count = 0;
	_ir_shutter_motor_ctrl.run_time = 0;
	_ir_shutter_motor_ctrl.pwm = pwm;
}


void motor_ctrl_shutter_stop(void)
{
#if 1  /* DEBUG info */
//	memcpy(&_debug_shutter_motor_ctrl, &_ir_shutter_motor_ctrl, sizeof(_ir_shutter_motor_ctrl));
#endif /* DEBUG info */

	_ir_shutter_motor_ctrl.drive_pulse = 0;
	_ir_shutter_motor_ctrl.pulse = 0;
	_ir_shutter_motor_ctrl.event_pulse = 0;
	_ir_shutter_motor_ctrl.lock_count = 0;
	_ir_shutter_motor_ctrl.run_time = 0;
	_ir_shutter_motor_ctrl.stop_time = SHUTTER_MOTOR_STOP_TIME;
	_ir_shutter_motor_ctrl.pwm = 0;

	_ir_shutter_motor_ctrl.mode = MOTOR_BRAKE;

	_pl_shutter_motor_stop();					/* stop feed motor */
	_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_SHUTTER_STOP, 0, 0, 0, 0);

#ifdef _ENABLE_JDL
	jdl_move_shutter(0);//2025-10-02
#endif	/* _ENABLE_JDL */
}


u8 motor_ctrl_pb_set_max_current(u16 current_max)
{
	u8 ret = ALARM_CODE_OK;

	_ir_apb_motor_ctrl.max_current = 0xFF & current_max;

	if(ret == ALARM_CODE_OK)
	{
		ret = _pl_set_pbmot_cur();
	}

	return ret;
}


u8 motor_ctrl_shutter_set_max_current(u16 current_max)
{
	u8 ret = ALARM_CODE_OK;

	_ir_shutter_motor_ctrl.max_current = 0xFF & current_max;

	if(ret == ALARM_CODE_OK)
	{
		ret = _pl_set_shutter_mot_cur();
	}

	return ret;
}


/* Shutter */
u8 motor_ctrl_shutter_fwd(u8 pwm, u8 drive_home_cnt)
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_shutter_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_shutter_start_clr_val(pwm, drive_home_cnt);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_SHUTTER_FWD_REQ, 0, 0, 0, 0);
	}
	else if (_ir_shutter_motor_ctrl.mode == MOTOR_FWD)
	{
		_motor_ctrl_shutter_start_clr_val(pwm, drive_home_cnt);
		_pl_shutter_motor_cw(pwm);
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}

u8 motor_ctrl_shutter_rev(u8 pwm, u8 drive_home_cnt)
{
	u8 rtn = IERR_CODE_OK;

	if (_ir_shutter_motor_ctrl.mode == MOTOR_STOP)
	{
		_motor_ctrl_shutter_start_clr_val(pwm, drive_home_cnt);
		_motor_ctrl_send_msg(ID_MOTOR_MBX, TMSG_MOTOR_SHUTTER_REV_REQ, 0, 0, 0, 0);
	}
	else if (_ir_shutter_motor_ctrl.mode == MOTOR_REV)
	{
		_motor_ctrl_shutter_start_clr_val(pwm, drive_home_cnt);
		_pl_shutter_motor_ccw(pwm);
	}
	else
	{
		rtn = IERR_CODE_BUSY;
	}

	return rtn;
}

bool is_motor_ctrl_shutter_stop(void)
{
	if (_ir_shutter_motor_ctrl.mode == MOTOR_STOP)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*********************************************************************//**
 * @brief set stacker ctrl value
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_stacker_set_full_check(u16 drive_pulse, u8 full_check)// 押し込み時にHomeを外れた後に設定して、押し込みパルスと時間を計測
{
	_motor_ctrl_disable_intr(PIN_STKENC);

	_ir_stacker_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア
	_ir_stacker_motor_ctrl.drive_pulse = drive_pulse; 	//設定値になった場合にイベント発生 リミット値設定
	_ir_stacker_motor_ctrl.full_check = 1;

	_motor_ctrl_enable_intr(PIN_STKENC);

}

#if defined(UBA_RTQ)		/* '21-03-01 */
/*********************************************************************//**
 * @brief set stacker ctrl value
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void motor_ctrl_stacker_set_drive_check(u16 drive_pulse)
{
	_motor_ctrl_disable_intr(PIN_STKENC);

//	既存とは異なるが新方式にするivizion2に合わせる
//	理由は、CPUコアが2つなので、
//  エンコーダ割込みと、ここの処理が同時に動く事もあり得るので
	_ir_stacker_motor_ctrl.init_flag = 1;
	_ir_stacker_motor_ctrl.init_value = drive_pulse;

	_motor_ctrl_enable_intr(PIN_STKENC);
}
#endif

/* EOF */
