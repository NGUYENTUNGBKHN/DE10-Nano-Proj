/******************************************************************************/
/*! @addtogroup Main********************************/
/*! @addtogroup Group2
    @file       operation_main_usb_power.c
    @brief      operation main process
    @date       2018/03/05
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/
#include <string.h>
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "operation.h"
#include "sub_functions.h"
#include "status_tbl.h"
#include "hal.h"
#ifdef _ENABLE_JDL
#include "jdl.h"
#endif	/* _ENABLE_JDL */

#define EXT
#include "com_ram.c"


/************************** Function Prototypes ******************************/
void _main_start_cyc(void);
/************************** Variable declaration *****************************/
extern T_MSG_BASIC ex_main_msg;


/*********************************************************************//**
 * @brief USB給電時のmain
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void usb_operation_main(void)
{
#ifdef _ENABLE_JDL
	u8 dummy[8];
#endif
	T_MSG_BASIC *tmsg_pt;
	ER ercd;


#if 1 //2024-02-08
	while(1)
	{
	/* 後ろの電源が接続されるのを待つ */
 		dly_tsk(1000);
		if((!__hal_5v_dect_read()))
		{
			_soft_reset();
		}
	}
#else
	_kernel_synch_cache();
	_main_start_cyc();
	// usb power only
	{
		act_tsk(ID_BEZEL_TASK);
		act_tsk(ID_DIPSW_TASK);
		act_tsk(ID_DLINE_TASK);
		act_tsk(ID_TIMER_TASK);
		act_tsk(ID_DISPLAY_TASK);
		act_tsk(ID_SUBLINE_TASK);
		//act_tsk(ID_OTG_TASK);
		act_tsk(ID_USB0_CB_TASK);
		act_tsk(ID_FRAM_TASK);

		act_tsk(ID_WDT_TASK);

	}
	_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_READ_REQ, FRAM_ALL, 0, 0, 0);
	while(1)
	{
		ercd = trcv_mbx(ID_MAIN_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		if(ercd == E_OK)
		{
			memcpy(&ex_main_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(ex_main_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_main_system_error(1, 44);
			}
			break;
		}
	}
#ifdef _ENABLE_JDL
	// DONE:ログ初期化ルーチン
	memset(dummy,0,8);
	jdl_init(0);
	jdl_powerup();
	jdl_set_jdl_time(&dummy[0]);
#endif	/* _ENABLE_JDL */
	init_status_table();
	set_test_mode();
	_main_send_msg(ID_DLINE_MBX, (TMSG_CONN_INITIAL|TMSG_TCODE_MAIN), ex_operating_mode, 0, 0, 0);
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_TEST_STANDBY, 0, 0, 0);
	while(1)
	{
		ercd = trcv_mbx(ID_MAIN_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		if(ercd == E_OK)
		{
			memcpy(&ex_main_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(ex_main_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_main_system_error(1, 44);
			}
			// TODO:download and log read
		}
		if((!__hal_5v_dect_read()))
		{
			// Download Startコマンド未受診でUSB POWER から24Vに切り替えが行われた
			// 再起動し、MAINに復帰（暫定）

			_soft_reset();

		}
	};
#endif

}
/* EOF */
