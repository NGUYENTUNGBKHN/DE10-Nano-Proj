/******************************************************************************/
/*! @addtogroup Main
    @file       bezel_task.c
    @brief      control bezel display task function
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Branch from Display Task
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "hal_bezel.h"
#include "hal.h"

#define EXT
#include "com_ram.c"
/************************** Function Prototypes ******************************/
void bezel_task(VP_INT exinf);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
static T_MSG_BASIC bezel_msg;


struct _BEZEL_LED_INFO ex_bezel_led;

#define BEZEL_LED_MODE_OFF				0x10	/* LED制御 OFF */// use
#define BEZEL_LED_MODE_ON				0x11	/* LED制御 ON */// use
#define BEZEL_LED_MODE_BLINK			0x12	// 新規作成 点滅      // use

/************************** PRIVATE FUNCTIONS *************************/
void _bezel_msg_proc(void);
void _bezel_LED_off(void);
void _bezel_LED_on(void);

void _bezel_idle_proc(void);

void _bezel_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _bezel_system_error(u8 fatal_err, u8 code);
void _bezel_blink_on_proc(void);
void _bezel_LED_blink(void);

/************************** EXTERN FUNCTIONS *************************/
// display_msg -> bezel_msg

void _bezel_initialize_proc(void)
{
	memset(&ex_bezel_led, 0, sizeof(ex_bezel_led));
	ex_bezel_led.mode = BEZEL_LED_MODE_OFF;
	ex_bezel_led.bezel_led_tm = 0;
}

/*******************************
        bezel_task
 *******************************/
void bezel_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;

	_bezel_initialize_proc();

	while(1)
	{
		if((trcv_mbx(ID_BEZEL_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME_BEZEL)) == E_OK)
		{
			memcpy(&bezel_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(bezel_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_bezel_system_error(1, 3);
			}
			_bezel_msg_proc();
		}
		_bezel_idle_proc();
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  bezel task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _bezel_msg_proc(void)
{
	switch (bezel_msg.tmsg_code)
	{
	case TMSG_DISP_BEZEL_LED_OFF:
		_bezel_LED_off();
		break;
	case TMSG_DISP_BEZEL_LED_ON:
	case TMSG_DISP_BEZEL_DEMO:
		_bezel_LED_on();
		break;

	case TMSG_DISP_BEZEL_BLINK:
		_bezel_LED_blink();
		break;		

	case TMSG_DISP_BEZEL_TEST_RUNNING:	//タスク暴走
		_bezel_LED_blink();
		break;
	default:					/* other */
		/* system error ? */
		_bezel_system_error(0, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief		bezel LED off
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _bezel_LED_off(void)
{
	_pl_bezel_led_uba(0);
	ex_bezel_led.mode = BEZEL_LED_MODE_OFF;
	ex_bezel_led.count = 0;
	ex_bezel_led.wait_time = 0;
	ex_bezel_led.s_bezel_count = 0;			// 実際に点滅した回数
}

/*********************************************************************//**
 * @brief		bezel LED bezel on
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _bezel_LED_on(void)
{
	_pl_bezel_led_uba(1);
	ex_bezel_led.mode = BEZEL_LED_MODE_ON;
	ex_bezel_led.count = 0;
	ex_bezel_led.wait_time = 0;
	ex_bezel_led.s_bezel_count = 0;			// 実際に点滅した回数
}

/*********************************************************************//**
 * @brief Bezel task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _bezel_idle_proc(void)
{
	// ここで点滅回数、周期を調整する
	if( ex_bezel_led.mode == BEZEL_LED_MODE_BLINK )
	{
		_bezel_blink_on_proc();	
	}
}

void _bezel_blink_on_proc(void)
{

	ex_bezel_led.bezel_led_tm++;

	if(ex_bezel_led.bezel_led_tm >= ex_bezel_led.wait_time)	// 点滅インターバル時間が経過したか確認 	// 初期値は50
	{
		if( ex_bezel_led.count <= ex_bezel_led.s_bezel_count  && ( ex_bezel_led.count != 0 ) )	// 点滅回数の最後だったか確認
		//  点滅リクエスト回数 <=  実際に点滅した回数  && 点滅リクエスト回数
		{
		// 点灯の最後
			_pl_bezel_led_uba(0);	// LED OFF
			ex_bezel_led.wait_time = ex_bezel_led.on_off_time;
			ex_bezel_led.s_bezel_count = 0;			// 実際に点滅した回数
		}
		else
		{
			if( ex_bezel_led.bezel_on == 0 )
			{
				_pl_bezel_led_uba(1);		// LED ON
				ex_bezel_led.wait_time = ex_bezel_led.on_time;
				ex_bezel_led.s_bezel_count++;			// 実際に点滅した回数
			}
			else
			{
				_pl_bezel_led_uba(0);	// LED OFF
				ex_bezel_led.wait_time = ex_bezel_led.off_time;
			}
		}
	}
}


void _bezel_LED_blink(void)
{

//	bezel_msg.tmsg_code		種類 モード(点灯、消灯、点滅)
//	bezel_msg.arg1			// 回数
//	bezel_msg.arg2			// ON時間
//	bezel_msg.arg3			// OFF時間
//	bezel_msg.arg4			// インターバル

	ex_bezel_led.mode = BEZEL_LED_MODE_BLINK;
	ex_bezel_led.bezel_led_tm = 0;

	if( bezel_msg.arg1 == 0 )	// 点滅回数
	{
	// 通常0点滅はあり得ない、取りあえず、点灯もしくは、消灯にする
		ex_bezel_led.count = bezel_msg.arg1;	// 永遠に点滅
	}
	else
	{
		ex_bezel_led.count = bezel_msg.arg1;	// 引数分点滅
	}

	if( bezel_msg.arg2 == 0 )	// on time
	{
		ex_bezel_led.on_time = 3;	// default設定値
	}
	else
	{
		ex_bezel_led.on_time = bezel_msg.arg2;
	}

	if( bezel_msg.arg3 == 0 )	// off time
	{
		ex_bezel_led.off_time = 3;	// default設定値
	}
	else
	{
		ex_bezel_led.off_time = bezel_msg.arg3;
	}

	if( bezel_msg.arg4 == 0 )	// on off インターバル
	{
		ex_bezel_led.on_off_time = 30;	// default設定値
	}
	else
	{
		ex_bezel_led.on_off_time = bezel_msg.arg4;
	}

	ex_bezel_led.wait_time = 0;

	_pl_bezel_led_uba(0);		// LED OFF

	ex_bezel_led.s_bezel_count = 0;			// 実際に点滅した回数

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
void _bezel_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_BEZEL_TASK;
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
			_bezel_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_bezel_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _bezel_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR


#else  /* _DEBUG_SYSTEM_ERROR */
	//if (fatal_err)
	//{
	//	_bezel_send_msg(ID_BEZEL_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	//}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_BEZEL_TASK, (u16)code, (u16)bezel_msg.tmsg_code, (u16)bezel_msg.arg1, fatal_err);
}


/* EOF */
