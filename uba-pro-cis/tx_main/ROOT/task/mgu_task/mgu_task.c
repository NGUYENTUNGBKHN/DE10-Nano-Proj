/******************************************************************************/
/*! @addtogroup Main
    @file       mgu_task.c
    @brief      control mgu task function
    @date       2021/04/19
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/04/19 Development Dept at Tokyo
      -# Initial Version
      -# Branch from Display Task
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "hal.h"
#include "hal_i2c_tmp.h"
#include "hal_i2c_eeprom.h"
#include "pl/pl_cis.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"
/************************** Function Prototypes ******************************/
void mgu_task(VP_INT exinf);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
static T_MSG_BASIC mgu_msg;

TMP_SENSOR_AD ex_tmp[8];
/************************** PRIVATE DEFINITIONS *************************/
enum _MGU_MODE
{
	MGU_MODE_IDLE = 0,
	MGU_MODE_READ,
	MGU_MODE_WRITE,
};

// 20220113 CISの温度取得を無視
// 20220131 温度取得を無視
// 20220214 CISAとSMOTのアドレス重複の為DISABLE
#define _DEBUG_TMP_DISABLE 0
#define _DEBUG_TMP_DISABLE_OUT 0
#define _DEBUG_TMP_DISABLE_CISA 0
#define _DEBUG_TMP_DISABLE_CISB 0
#define _DEBUG_TMP_DISABLE_SMOT 0
#define _DEBUG_TMP_DISABLE_HMOT 0

/************************** PRIVATE FUNCTIONS *************************/
void _mgu_initialize_proc(void);
void _mgu_msg_proc(void);
void _mgu_idle_proc(void);
void _mgu_set_mode(u16 mode);

void _mgu_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _mgu_system_error(u8 fatal_err, u8 code);


/************************** EXTERN FUNCTIONS *************************/

/*********************************************************************//**
 * @brief read temperature IC(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 mgu_read_tmp(void)
{
	u8 err = SUCCESS;

#if (_DEBUG_TMP_DISABLE==1)
	ex_temperature.cis_a = ex_tmp[0].CEL;
	ex_temperature.cis_b = ex_tmp[1].CEL;
	ex_temperature.outer = ex_tmp[2].CEL;
#else
	static u8 s_cnt = 0;

	switch(s_cnt++)
	{
	case 0:
#if (_DEBUG_TMP_DISABLE_OUT==1)
#else
		if(_hal_tmp_read_out(&ex_tmp[2].BIN,&ex_tmp[2].CEL) != E_OK)
		{
			err = ERROR;
		}
		ex_temperature.outer = ex_tmp[2].CEL;
#endif
		break;
	default:
		s_cnt = 0;
		break;
	}

#endif

	return(err);
}
/*********************************************************************//**
 * @brief read temperature IC
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8  mgu_force_read_tmp(void)
{
	u8 err = SUCCESS;

	if(ex_cis == 0)//2025-02-13
	{
		//CIS電源ONになってないので無効
		err = ERROR;		
		return(err);
	}

	for(int retry = 0; retry < 5; retry++)
	{
		err = SUCCESS;
		// cis-a, cis-bの電源はON時のみ読み取り可能
#if (_DEBUG_TMP_DISABLE_CISA==1)
#else
		if(_hal_tmp_read_cisa(&ex_tmp[0].BIN,&ex_tmp[0].CEL) != E_OK)
		{
			err = ERROR;
		}
#endif
#if (_DEBUG_TMP_DISABLE_CISB==1)
#else
		if(_hal_tmp_read_cisb(&ex_tmp[1].BIN,&ex_tmp[1].CEL) != E_OK)
		{
			err = ERROR;
		}
#endif
#if (_DEBUG_TMP_DISABLE_OUT==1)
#else
		if(_hal_tmp_read_out(&ex_tmp[2].BIN,&ex_tmp[2].CEL) != E_OK)
		{
			err = ERROR;
		}
#endif
		if(err==SUCCESS)
		{
			break;
		}
		//ここの処理は、CISの電源付け忘れの保護処理ではない。
		//CISを付けたはずなのに勝手に消えている場合の保護処理

		_pl_cis_enable_set(0);
		OSW_TSK_sleep(100);
		_pl_cis_enable_set(1);
		OSW_TSK_sleep(100);
	}

	ex_temperature.cis_a = ex_tmp[0].CEL;
	ex_temperature.cis_b = ex_tmp[1].CEL;
	ex_temperature.outer = ex_tmp[2].CEL;

	ex_multi_job.busy &= ~(TASK_ST_MGU); //2024-11-21 1coer対応

	return(err);
}
/*********************************************************************//**
 * @brief read Log(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 mgu_read_log(void)
{
	u8 err = SUCCESS;
	u32 start,time;

	start = OSW_TIM_value();
	// eeprom　現状使用していない、iviozn2の回路流用で実装されている様だ
	//　動くか試した事なし
	if(_hal_read_eeprom(&ex_eeprom_data[0], 0, EEPROM_MAX_SIZE) != E_OK)
	{
		err = ERROR;
	}
	time = OSW_TIM_value() - start;

	return(err);
}
/*********************************************************************//**
 * @brief write Log(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 mgu_write_log(void)
{
	u8 err = SUCCESS;
#if defined(EEPROM_TEST) //ここの処理はもともとなかった
	if(_hal_write_eeprom_test() != E_OK)	//テストなので10バイト固定
	{
		err = ERROR;
	}	
#endif

	return(err);
}
/*******************************
        mgu_task
 *******************************/
void mgu_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;

	_mgu_initialize_proc();

	while(1)
	{
		if((trcv_mbx(ID_MGU_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME_MGU)) == E_OK)
		{
			memcpy(&mgu_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(mgu_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_mgu_system_error(1, 3);
			}
			_mgu_msg_proc();
		}
		else
		{
			_mgu_idle_proc();
		}
	}
}


void _mgu_initialize_proc(void)
{
}


/*********************************************************************//**
 * @brief process of MGU read message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _mgu_read_proc(void)
{
	_mgu_set_mode(MGU_MODE_READ);
	switch (mgu_msg.arg1)
	{
	case MGU_TMP:
		if(mgu_force_read_tmp() == SUCCESS)
		{
			_mgu_send_msg(ID_MAIN_MBX, TMSG_MGU_READ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_mgu_send_msg(ID_MAIN_MBX, TMSG_MGU_READ_RSP, TMSG_SUB_ALARM, ALARM_CODE_TMP_I2C, 0, 0);
		}
		break;
	case MGU_LOG:	//現状起動時のみ
		if(mgu_read_log() == SUCCESS)
		{
			_mgu_send_msg(ID_MAIN_MBX, TMSG_MGU_READ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_mgu_send_msg(ID_MAIN_MBX, TMSG_MGU_READ_RSP, TMSG_SUB_ALARM, ALARM_CODE_I2C, 0, 0);
		}
		break;
	default:
		break;
	}
	_mgu_set_mode(MGU_MODE_IDLE);
}

/*********************************************************************//**
 * @brief process of MGU write message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _mgu_write_proc(void)
{
	_mgu_set_mode(MGU_MODE_WRITE);
	switch (mgu_msg.arg1)
	{
	case MGU_TMP:
		/* Nothing to write data */
		_mgu_send_msg(ID_MAIN_MBX, TMSG_MGU_WRITE_RSP, TMSG_SUB_ALARM, ALARM_CODE_I2C, 0, 0);
		break;
	case MGU_LOG: //現状 Writeは存在しない
	#if defined(EEPROM_TEST)
		mgu_write_log(); //関数は呼び出していたな、もともと中身はなかった
	#endif
		_mgu_send_msg(ID_MAIN_MBX, TMSG_MGU_WRITE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		break;
	default:
		break;
	}
	_mgu_set_mode(MGU_MODE_IDLE);
}
/*********************************************************************//**
 * @brief MBX Idling procedure
 *  bezel task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _mgu_idle_proc(void)
{
	mgu_read_tmp();
}
/*********************************************************************//**
 * @brief MBX message procedure
 *  bezel task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _mgu_msg_proc(void)
{
	switch (mgu_msg.tmsg_code)
	{
	case TMSG_MGU_READ_REQ:		/* read mgu */
		_mgu_read_proc();
		break;
	case TMSG_MGU_WRITE_REQ:		/* write mgu */
		_mgu_write_proc();
		break;
	default:					/* other */
		/* system error ? */
		_mgu_system_error(0, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief set task mode
 * @param[in]	mode : task mode
 * @return 		None
 **********************************************************************/
void _mgu_set_mode(u16 mode)
{
	ex_mgu_task_mode = mode;
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
void _mgu_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_MGU_TASK;
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
			_mgu_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_mgu_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _mgu_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	_mgu_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_TEST_RUNNING, 0, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	//if (fatal_err)
	//{
	//	_mgu_send_msg(ID_MGU_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	//}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_MGU_TASK, (u16)code, (u16)mgu_msg.tmsg_code, (u16)mgu_msg.arg1, fatal_err);
}


/* EOF */
