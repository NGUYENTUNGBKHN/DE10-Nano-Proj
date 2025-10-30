/******************************************************************************/
/*! @addtogroup Main
    @file       mode_download.c
    @brief      powerup mode of main task
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
#include "download.h"

#define EXT
#include "com_ram.c"
/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/
void download_wait_req(void);
void download_if(void);
void download_usb(void);
void download_subline_usb(void);
/************************** EXTERNAL VARIABLES *************************/


/*********************************************************************//**
 * @brief download message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void download_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case DOWNLOAD_MODE2_WAIT_REQ:
		download_wait_req();
		break;
	case DOWNLOAD_MODE2_IF:
		download_if();
		break;
	case DOWNLOAD_MODE2_USB:
		download_usb();
		break;
	case DOWNLOAD_MODE2_SUBLINE_USB:
		download_subline_usb();
		break;
	default:
		/* system error ? */
		_main_system_error(0, 5);
		break;
	}
}

/*********************************************************************//**
 * @brief wait download request procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void download_wait_req(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_DOWNLOAD_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_IF);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
		break;
	case TMSG_DLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_DLINE_MBX, TMSG_DLINE_DOWNLOAD_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_USB);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
		break;
	case TMSG_SUBLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_SUBLINE_MBX, TMSG_SUBLINE_DOWNLOAD_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_main_set_mode(MODE1_DOWNLOAD, DOWNLOAD_MODE2_SUBLINE_USB);
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
		break;
	default:
		/* system error ? */
		_main_system_error(0, 7);
		break;
	}
}

/*********************************************************************//**
 * @brief wait Host I/F download complete procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void download_if(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_SOFT_RESET_REQ:
		_soft_reset();
		break;
	case TMSG_CLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_DOWNLOAD_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		break;
	case TMSG_DLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_DLINE_MBX, TMSG_DLINE_DOWNLOAD_RSP, TMSG_SUB_ALARM, 0, 0, 0);
		break;
	case TMSG_SUBLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_SUBLINE_MBX, TMSG_SUBLINE_DOWNLOAD_RSP, TMSG_SUB_ALARM, 0, 0, 0);
		break;
	case TMSG_CLINE_WRITE_FLASH_REQ:
		#if defined(_PROTOCOL_ENABLE_ID0G8)
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_WRITE_FLASH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		#else
	    _download_write();
	    if(get_dl_end_status() == D_DL_FAILURE)
	    {
			_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_WRITE_FLASH_RSP, TMSG_SUB_ALARM, ALARM_CODE_ROM_WRITE, 0, 0);
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_ROM_WRITE, ex_operating_mode, 0, 0);
	    }
	    else
	    {
			_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_WRITE_FLASH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
	    }
		#endif
		break;
	case TMSG_CLINE_DOWNLOAD_CALC_CRC_REQ:
		if(download_calc_crc())
		{
			_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_DOWNLOAD_CALC_CRC_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_DOWNLOAD_CALC_CRC_RSP, TMSG_SUB_ALARM, ALARM_CODE_DOWNLOAD, 0, 0);
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_WAIT_REQ);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_DOWNLOAD, ex_operating_mode, 0, 0);
		}
		break;
	case TMSG_DLINE_SOFT_RESET_REQ:
	default:
		/* system error ? */
		_main_system_error(0, 7);
		break;
	}
}
/*********************************************************************//**
 * @brief wait Host/Device USB Device download complete procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void download_usb(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DLINE_SOFT_RESET_REQ:
		_soft_reset();
		break;
	case TMSG_DLINE_DOWNLOAD_COMPLETE_REQ:
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD_COMPLETE, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0xF, 0, 0, 0);
		break;
	case TMSG_CLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_DOWNLOAD_RSP, TMSG_SUB_ALARM, 0, 0, 0);
		break;
	case TMSG_DLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_DLINE_MBX, TMSG_DLINE_DOWNLOAD_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		break;
	case TMSG_SUBLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_SUBLINE_MBX, TMSG_SUBLINE_DOWNLOAD_RSP, TMSG_SUB_ALARM, 0, 0, 0);
		break;
	case TMSG_SUBLINE_DOWNLOAD_COMPLETE_REQ:
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD_COMPLETE, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0xF, 0, 0, 0);
		break;
	case TMSG_CLINE_WRITE_FLASH_REQ:
	case TMSG_CLINE_DOWNLOAD_CALC_CRC_REQ:
	case TMSG_CLINE_SOFT_RESET_REQ:
	default:
		/* system error ? */
		_main_system_error(0, 7);
		break;
	}
}
/*********************************************************************//**
 * @brief wait Subline USB Device download complete procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void download_subline_usb(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_SUBLINE_SOFT_RESET_REQ:
		_soft_reset();
		break;
	case TMSG_SUBLINE_DOWNLOAD_COMPLETE_REQ:
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_DOWNLOAD_COMPLETE, 0, 0, 0, 0);
		_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0xF, 0, 0, 0);
		break;
	case TMSG_SUBLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_SUBLINE_MBX, TMSG_SUBLINE_DOWNLOAD_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		break;
	case TMSG_CLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_CLINE_MBX, TMSG_CLINE_DOWNLOAD_RSP, TMSG_SUB_ALARM, 0, 0, 0);
		break;
	case TMSG_DLINE_DOWNLOAD_REQ:
		_main_send_msg(ID_DLINE_MBX, TMSG_DLINE_DOWNLOAD_RSP, TMSG_SUB_ALARM, 0, 0, 0);
		break;
	case TMSG_CLINE_WRITE_FLASH_REQ:
	case TMSG_CLINE_DOWNLOAD_CALC_CRC_REQ:
	case TMSG_CLINE_SOFT_RESET_REQ:
	default:
		/* system error ? */
		_main_system_error(0, 7);
		break;
	}
}
/* EOF */

