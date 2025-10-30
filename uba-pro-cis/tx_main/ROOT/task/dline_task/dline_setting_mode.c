/******************************************************************************/
/*! @addtogroup Main
    @file       dline_setting_mode.c
    @date       2020/02/25
    @author     yuji-kenta
    @par        Revision
    @par        Copyright (C)
    2020 Japan CashMachine Co., Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_setting_mode.c
 *
 *  Created on: 2020/02/25
 *      Author: yuji-kenta
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "dline_suite.h"
#include "dline_test.h"
#include "dline_utility.h"
#include "dline_adjustment_mode.h"
#include "dline_setting_mode.h"
#include "hal.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

/************************** EXTERN VARIABLES *************************/

/************************** EXTERN FUNCTIONS *************************/
extern void set_response_1data(u8 cmd);
extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/

/******************************************************************************/
/*! @brief Get/Set stacker full threshold Setting
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static	u8	phase_temperature(u8 cmd)
{
	u8	response = 0;

	//u16 back_data = 0;

	//ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_UTILITY_MODE;
	switch(cmd)
	{
	case CMD_READ:
		response = RES_DATA;

		ex_usb_write_size = FUSB_HEADER_SIZE + sizeof(Temperature);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;
		memcpy(&ex_usb_write_buffer[6], &ex_temperature,  sizeof(Temperature));
		break;
	case CMD_WRITE:
		response = RES_ACK;
		memcpy(&ex_temperature, &ex_usb_read_buffer[6],  sizeof(Temperature));
		break;
	default:
		response = RES_NAK;
		break;
	}

	return(response);
}

static	u8	phase_version_set(u8 cmd)
{
	u8	response = 0;
	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_UTILITY_MODE;
	switch(cmd)
	{
	case UTILITY_CMD_GET_VER:	/* 0x80 0x01 0x05 */
		response = RES_DATA;

		ex_usb_write_size = FUSB_HEADER_SIZE + sizeof(u8);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		if(is_legacy_mode_enable())
		{
			*(ex_usb_write_buffer + 6) = 1;
		}
		else
		{
			*(ex_usb_write_buffer + 6) = 0;
		}
		break;
	case UTILITY_CMD_VER_UBA700:	/* 0x75 0x01 0x00 */
	case UTILITY_CMD_VER_UBA:		/* 0x75 0x01 0x01 */
		response = RES_ACK;
		if(ex_usb_read_buffer[5] == UTILITY_CMD_VER_UBA)
		{
			ex_mode2_setting.legacy_mode = 0xAA;
		}
		else
		{
			ex_mode2_setting.legacy_mode = 0x55;
		}
		ex_mode2_setting.legacy_mode_chk = ~(ex_mode2_setting.legacy_mode);
		_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_MODE_SETTING, 0, 0, 0);
		break;
	default:
		response = RES_NAK;
		break;
	}
	return(response);
}
/******************************************************************************/
/*! @brief Setting Request Function.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void front_usb_setting_request(void)
{
	u8 response = 0;

	if((u8)ex_front_usb.pc.mess.modeID == (u8)UTILITY_MODE_IF_SETTINGS)
	{
		switch(ex_front_usb.pc.mess.phase)
		{
		case PHASE_SETTING_MOTOR_CURRENT:
			response = NAK;
			break;
		case PHASE_SETTING_STACKER_FULL_THRESHOLD:
			response = NAK;
			break;
		case PHASE_SETTING_TEMPERATURE:
			response = phase_temperature(ex_front_usb.pc.mess.command);
			break;
		case PHASE_SETTING_SPEED_MODE:
			response = NAK;
			break;
		case PHASE_SETTING_SSSU_MODE:
			response = NAK;
			break;
		}
	}
	else if((u8)ex_front_usb.pc.mess.modeID == (u8)UTILITY_MODE_HW_SETTINGS)
	{
		switch(ex_front_usb.pc.mess.phase)
		{
		case HW_PHASE_SETTING_VERSION_SET:
			response = phase_version_set(ex_front_usb.pc.mess.command);
			break;
		default:
			response = NAK;
			break;
		}
	}
	else
	{
		response = NAK;
	}

	if(response != 0)
	{
		set_response_1data(response);
	}
}


/*---	End of file	---*/
