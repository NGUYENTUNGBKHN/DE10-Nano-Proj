/******************************************************************************/
/*! @addtogroup Main
    @file       display_task.c
    @brief      control status led task function
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
#include "common.h"
#include "sub_functions.h"
#include "hal.h"

#define EXT
#include "com_ram.c"
/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
u32 st_led_tm;


#if 1 //

	static u16 s_errdisp_led_ctrl_time;		//LED点滅用のインターバルカウンタ
	static u16 s_errdisp_led_ctrl_count_red;	// 実際に点滅した回数
	static u16 s_errdisp_led_ctrl_count_green;	// 実際に点滅した回数

	#define DISP_INTERVAL_MIDDLE	3
	#define DISP_ROUNDWAIT_MIDDLE	20		/* 2sec */// 点滅の次の点滅までの待ち時間
	#define DISP_DISABLE_UBA	0


	void _errdisp_set_mode(u16 mode);
	
	
	void _errdisp_blink_on_proc(void);
	
	void _errdisp_blink_last_proc(void);
	bool _is_errdisp_led_ctrl_count_over_red(u16 count);
	bool _is_errdisp_led_ctrl_count_over_green(u16 count);
	bool _is_errdisp_led_ctrl_timeup(u16 time);

	struct DISP_LED_CTRL_VAL
	{
		u16 mode;
		u8 code;		// エラーコード
		u8 red_count;
		u8 red_mode;
		u8 green_count;
		u8 green_mode;
		u16 interval;	// 1回の点滅周期
		u8 red_current_count;
		u8 green_current_count;
		u16 round_wait;
	};
	extern struct DISP_LED_CTRL_VAL s_errdisp_ctrl;
	struct DISP_LED_CTRL_VAL s_errdisp_ctrl;

	struct ERRDISP_LED_SET_TBL2
	{
		u8 code;		// エラーコード
		u8 red_count;
		u8 red_mode;
		u8 green_count;
		u8 green_mode;
		u16 interval;
		u8 rfid_index;
	};

#else


#endif

enum _ERRDISP_MODE
{
	ERRDISP_MODE_OFF = 1,
	ERRDISP_MODE_BLINK_ON,
	ERRDISP_MODE_BLINK_LAST,
	ERRDISP_MODE_IDLE,
	STATUS_LED_MODE_DOWNLOAD,
	STATUS_LED_MODE_DOWNLOAD_ING,
	STATUS_LED_MODE_DOWNLOAD_COMPLETE,

};

	const struct ERRDISP_LED_SET_TBL2 ex_errdisp_alarm_tbl[]=
	{
		{ALARM_CODE_IF_AREA,				3, STATUS_LED_BLINK, 1, STATUS_LED_ON,	DISP_INTERVAL_MIDDLE, 0} /* UBA500異なるが3回にする、UBA500は2回,2回だとBootIFエラーとIFエラーの区別がつかないので、ROM check IF-AREA error */
		,{ALARM_CODE_ROM_WRITE,				6, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE,	0} /* UBA500はダウンロード中のエラーは全て6に統一している */
		,{ALARM_CODE_DOWNLOAD,				6, STATUS_LED_BLINK, 0, STATUS_LED_ON, DISP_INTERVAL_MIDDLE,	0} /* dwonload file error ID-003のダウンロード完了後の確認でエラー*/
	};
	#define ERRDISP_LED_ALARM_TBL_MAX	(sizeof(ex_errdisp_alarm_tbl) / sizeof(struct ERRDISP_LED_SET_TBL2))

	static T_MSG_BASIC errdisp_msg;


/************************** PRIVATE FUNCTIONS *************************/
void display_task(VP_INT exinf);

void _display_msg_proc(void);
void _display_idel_proc(void);
void _display_status_led_off(void);
void _display_status_led_set_alarm(void);
void _display_status_led_set_download(void);
void _display_status_led_set_download_complete(void);
void _display_status_led_set_download_ing(void);
void _display_status_led_download_ing(void);
void _display_set_mode(u16 mode);
void _display_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _display_system_error(u8 fatal_err, u8 code);


/************************** EXTERN FUNCTIONS *************************/


/*******************************
        display_task
 *******************************/
void display_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;

	st_led_tm = 0;

	_hal_status_led(DISP_COLOR_INIT);

	while(1)
	{
		if((trcv_mbx(ID_DISPLAY_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME_DISPLAY)) == E_OK)
		{
			memcpy(&errdisp_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(errdisp_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_display_system_error(1, 3);
			}
			_display_msg_proc();
		}
		_display_idel_proc();
	}
}



void _display_msg_proc(void)
{
	switch (errdisp_msg.tmsg_code)
	{
	case TMSG_DISP_LED_OFF:
		_display_status_led_off();
		break;
	case TMSG_DISP_LED_ALARM:
		_display_status_led_set_alarm();
		break;
	case TMSG_DISP_LED_DOWNLOAD:
		_display_status_led_set_download();
		break;
	case TMSG_DISP_LED_DOWNLOAD_ING:
		_display_status_led_set_download_ing();
		break;
	case TMSG_DISP_LED_DOWNLOAD_COMPLETE:
		_display_status_led_set_download_complete();
		break;
	default:					/* other */
		/* system error ? */
		_display_system_error(0, 4);
		break;
	}
}



/*********************************************************************//**
 * @brief		status LED display off
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_off(void)
{
	memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));
	s_errdisp_ctrl.mode = ERRDISP_MODE_OFF;
	s_errdisp_led_ctrl_time = 0;			//LED点滅用のインターバルカウンタ
	s_errdisp_led_ctrl_count_red = 0;
	s_errdisp_led_ctrl_count_green = 0;

	_hal_status_led(DISP_COLOR_RED_OFF);
	_hal_status_led(DISP_COLOR_GREEN_OFF);

	_errdisp_set_mode(ERRDISP_MODE_OFF);
}




/*********************************************************************//**
 * @brief		Set status LED display
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_set_alarm(void)
{

#if 1 //2023-12-25
	u16 cnt;

	for (cnt = 0; cnt < ERRDISP_LED_ALARM_TBL_MAX; cnt++)
	{
		if (ex_errdisp_alarm_tbl[cnt].code == errdisp_msg.arg1)	// 引数としては3番目
		{
			memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));
			s_errdisp_led_ctrl_time = 0;			//LED点滅用のインターバルカウンタ
			s_errdisp_led_ctrl_count_red = 0;
			s_errdisp_led_ctrl_count_green = 0;
			s_errdisp_ctrl.mode = ERRDISP_MODE_BLINK_ON;
			s_errdisp_ctrl.code = ex_errdisp_alarm_tbl[cnt].code;
			s_errdisp_ctrl.red_count = ex_errdisp_alarm_tbl[cnt].red_count;
			s_errdisp_ctrl.red_mode = ex_errdisp_alarm_tbl[cnt].red_mode;
			s_errdisp_ctrl.green_count = ex_errdisp_alarm_tbl[cnt].green_count;
			s_errdisp_ctrl.green_mode = ex_errdisp_alarm_tbl[cnt].green_mode;
			s_errdisp_ctrl.interval = ex_errdisp_alarm_tbl[cnt].interval;
			s_errdisp_ctrl.round_wait = DISP_ROUNDWAIT_MIDDLE;

			if( STATUS_LED_ON == s_errdisp_ctrl.red_mode )
			{
				_hal_status_led(DISP_COLOR_RED); // red on keep
			}
			else
			{
				_hal_status_led(DISP_COLOR_RED_OFF);// red off start
			}

			if( STATUS_LED_ON == s_errdisp_ctrl.green_mode )
			{
				_hal_status_led(DISP_COLOR_GREEN); // green on keep
			}
			else
			{
				_hal_status_led(DISP_COLOR_GREEN_OFF); // green off start
			}

			if( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK || s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
			{
				_errdisp_set_mode(ERRDISP_MODE_BLINK_ON);
			}
			else
			{
    			_errdisp_set_mode(ERRDISP_MODE_IDLE);
			}

			break;
		}
	}
#else

#endif	
}

/*********************************************************************//**
 * @brief		Set status LED display
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_set_download(void) //get message
{
	_errdisp_set_mode(STATUS_LED_MODE_DOWNLOAD);
	st_led_tm = 0;
}


void _display_status_led_set_download_ing(void) //get message
{
	_errdisp_set_mode(STATUS_LED_MODE_DOWNLOAD_ING);
}

/*********************************************************************//**
 * @brief		Set status LED display
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_set_download_complete(void)
{
	_errdisp_set_mode(TMSG_DISP_LED_OFF);
}

/*********************************************************************//**
 * @brief		BIF download display.
 * 				Flashing alternately in red and green
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_download(void)
{
	static u32 sbk_color;
	u32 color;

	st_led_tm += TASK_WAIT_TIME_DISPLAY;

	if(st_led_tm < 800)
	{
		color = DISP_COLOR_GREEN;
	}
	else
	{
		color = DISP_COLOR_RED;
	}

	if(sbk_color != color)
	{
		sbk_color = color;
		if(color == DISP_COLOR_GREEN)
		{
			_hal_i2c3_for_led_tca9535(0, 0x08); /* Red OFF	*/
			_hal_i2c3_for_led_tca9535(1, 0x10);	/* green ON */
		}
		else
		{
			_hal_i2c3_for_led_tca9535(1, 0x08); /* Red ON	*/
			_hal_i2c3_for_led_tca9535(0, 0x10);	/* green OFF */
		}
	}
	if(st_led_tm >= 1600)
	{
		/* status LED on/off */
		st_led_tm = 0;
	}
}


/*********************************************************************//**
 * @brief		display task idel procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_idel_proc(void)
{
	switch(ex_display_task_mode)
	{
    case    ERRDISP_MODE_IDLE:
	case	ERRDISP_MODE_OFF:			/* Error LED : OFF			*/
			break;

	case	ERRDISP_MODE_BLINK_ON:		/* Error LED : Blink ON		*/// use
			_errdisp_blink_on_proc();	// use
			break;

	case	ERRDISP_MODE_BLINK_LAST:	/* Error LED : Blink Last	*/
			_errdisp_blink_last_proc();
			break;

	case 	STATUS_LED_MODE_DOWNLOAD:
			_display_status_led_download();
			break;

	case 	STATUS_LED_MODE_DOWNLOAD_ING:
			_display_status_led_download_ing();
			break;

	default:
		/* system error ? */
		_display_system_error(0, 5);
		break;
	}
}


/*********************************************************************//**
 * @brief set task mode
 * @param[in]	mode : task mode
 * @return 		None
 **********************************************************************/
void _display_set_mode(u16 mode)
{

	ex_display_task_mode = mode;

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
void _display_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_DISPLAY_TASK;
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
			_display_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_display_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _display_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_display_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_DISPLAY_TASK, (u16)code, (u16)errdisp_msg.tmsg_code, (u16)errdisp_msg.arg1, fatal_err);
}


void _display_status_led_download_ing(void)
{
	static u32 back_seq;
	u32 time;
	u32 seq;

	/* 400msec*/
	time = OSW_TIM_value() % 1600;
	if(time < 400)
	{
		seq = 0;	//ALL off
	}
	else if(time < 800)
	{
		seq = 1;	//Green on, Red off
	}
	else if(time < 1200)
	{
		seq = 2;	//Green off, Red on
	}
	else
	{
		seq = 3;	//Green on, Red on
	}

	if(back_seq != seq)
	{
		back_seq = seq;
		if(seq == 0)
		{
		//ALL off
			_hal_i2c3_for_led_tca9535(0, 0x10);	/* green OFF */
			_hal_i2c3_for_led_tca9535(0, 0x08); /* Red OFF	*/
		}
		else if(seq == 1)
		{
		//Green on, Red off
			_hal_i2c3_for_led_tca9535(1, 0x10);	/* green ON */
			_hal_i2c3_for_led_tca9535(0, 0x08); /* Red OFF	*/
		}
		else if(seq == 2)
		{
		//Green off, Red on
			_hal_i2c3_for_led_tca9535(0, 0x10);	/* green OFF */
			_hal_i2c3_for_led_tca9535(1, 0x08); /* Red ON	*/
		}
		else if(seq == 3)
		{
		//Green on, Red on
			_hal_i2c3_for_led_tca9535(1, 0x10);	/* green ON */
			_hal_i2c3_for_led_tca9535(1, 0x08); /* Red ON	*/
		}
	}

}

void _errdisp_blink_on_proc(void)
{

	if (_is_errdisp_led_ctrl_timeup(s_errdisp_ctrl.interval))	// 点滅インターバル時間が経過したか確認
	{
		// 緑確認
		if ( s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
		{
			if (_is_errdisp_led_ctrl_count_over_green(s_errdisp_ctrl.green_count))	// 点滅回数の最後か確認
			{
				_hal_status_led(DISP_COLOR_GREEN_OFF);
			}
			else
			{
				if( green_on == 0 )
				{
					_hal_status_led(DISP_COLOR_GREEN);
				}
				else
				{
					_hal_status_led(DISP_COLOR_GREEN_OFF);
					s_errdisp_led_ctrl_count_green++;
				}
			}
		}

		// 赤確認
		if ( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK )
		{
			if (_is_errdisp_led_ctrl_count_over_red(s_errdisp_ctrl.red_count))
			{
				_hal_status_led(DISP_COLOR_RED_OFF);
			}
			else
			{
				if( red_on == 0 )
				{
					_hal_status_led(DISP_COLOR_RED);
				}
				else
				{
					_hal_status_led(DISP_COLOR_RED_OFF);
					s_errdisp_led_ctrl_count_red++;
				}
			}
		}

		if(
		((_is_errdisp_led_ctrl_count_over_green(s_errdisp_ctrl.green_count) )
		|| s_errdisp_ctrl.green_mode != STATUS_LED_BLINK )
		&&
		( (_is_errdisp_led_ctrl_count_over_red(s_errdisp_ctrl.red_count))
		|| s_errdisp_ctrl.red_mode != STATUS_LED_BLINK )
		)
		{
			if( ex_display_task_mode == ERRDISP_MODE_BLINK_ON )
			{
			/* 必要以上のログがログが更新される可能性がある為、ログ保存処理はスキップ	*/
				ex_display_task_mode = ERRDISP_MODE_BLINK_LAST;/* 最後の点滅完了状態	*/
			}
		}
		s_errdisp_led_ctrl_time = 0;			//LED点滅用のインターバルカウンタ
	}
}



void _errdisp_set_mode(u16 mode)
{
	ex_display_task_mode = mode;
}

void _errdisp_blink_last_proc(void)
{

	if (_is_errdisp_led_ctrl_timeup(s_errdisp_ctrl.round_wait))
	{
		// green
		if ( s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
		{
			s_errdisp_led_ctrl_count_green = 0;
		}

		//red
		if ( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK )
		{
			s_errdisp_led_ctrl_count_red = 0;
		}
		/* 必要以上のログがログが更新される可能性がある為、ログ保存処理はスキップ	*/
		ex_display_task_mode = ERRDISP_MODE_BLINK_ON;
		s_errdisp_led_ctrl_time = 0;		//LED点滅用のインターバルカウンタ
	}
}


bool _is_errdisp_led_ctrl_timeup(u16 time)	// 緑、赤共通
{
	s_errdisp_led_ctrl_time++;			//LED点滅用のインターバルカウンタ
	return (s_errdisp_led_ctrl_time >= time) ? true : false;
}

/*********************************************************************//**
 * @brief is led control count over
 * @param[in]	None
 * @return 		None
 **********************************************************************/
bool _is_errdisp_led_ctrl_count_over_red(u16 count)
{

	return (s_errdisp_led_ctrl_count_red >= count) ? true : false;
}


/*********************************************************************//**
 * @brief is led control count over
 * @param[in]	None
 * @return 		None
 **********************************************************************/
bool _is_errdisp_led_ctrl_count_over_green(u16 count)
{

	return (s_errdisp_led_ctrl_count_green >= count) ? true : false;
}




/* EOF */
