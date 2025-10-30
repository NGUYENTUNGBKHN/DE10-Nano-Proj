/******************************************************************************/
/*! @addtogroup Main
    @file       mode_alarm.c
    @brief      alarm mode of main task
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
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "operation.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
/************************** PRIVATE DEFINITIONS ***********************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/
void alarm_wait_req(void);

/************************** EXTERN FUNCTIONS **************************/

/************************** EXTERNAL VARIABLES *************************/
extern u8 ex_main_task_mode1;
extern u8 ex_main_task_mode2;

/**********************************************************************
 * @brief alarm message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case ALARM_MODE2_WAIT_REQ:
		alarm_wait_req();
		break;
	default:
		/* system error ? */
		_main_system_error(0, 102);
		break;
	}
}


/*********************************************************************//**
 * @brief wait reset request
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void alarm_wait_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_DOWNLOAD_RSP, 0, 0, 0, 0);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_IF);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
		break;
	case TMSG_DLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_DLINE_MBX, TMSG_DLINE_DOWNLOAD_RSP, 0, 0, 0, 0);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_USB);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
		break;
	case TMSG_SUBLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_SUBLINE_MBX, TMSG_SUBLINE_DOWNLOAD_RSP, 0, 0, 0, 0);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_SUBLINE_USB);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
		break;
	case TMSG_TIMER_TIMES_UP:
		break;
	default:
		if ((ex_main_msg.tmsg_code & TMSG_TCODE_CLINE) == TMSG_TCODE_CLINE)
		{
			/* Line TaskからのReset以外のMessageは待機 */
		}
		else if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 != TMSG_SUB_ALARM))
		{
			/* system error ? */
			_main_system_error(0, 102);
		}
		break;
	}
}

/* EOF */

