/******************************************************************************/
/*! @addtogroup Main
    @file       dline_authentication.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_authentication.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "cyc.h"
#include "motor_ctrl.h"
#include "dline_jdl.h"
#include "hal.h"
#include "../task/cline_task/003/id003_authentication.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

#include "dline_authentication.h"

enum	CONDITION_EEPROM_CMD_NUMBER
{
	CMD_START	= 0x01,
};

enum	CONDITION_RES_NUMBER
{
	RES_OK		= 0x00,
	RES_END		= 0xFF,
};

enum TESTMODE_CMD_NUMBER
{
	CMD_NONE				= 0x00,
	CMD_READ				= 0x01,
	CMD_WRITE				= 0x02,
};

enum TESTMODE_RES_NUMBER
{
	RES_DATA	= 0x00,
	RES_NG		= 0x80,
	RES_BUSY	= 0x01,
	RES_ACK		= 0x06,
	RES_NAK		= 0x15,
};
static	u8	phase_authentication_read(void);
static	u8	phase_authentication_write(void);
/************************** External functions *******************************/
extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void set_response_1data(u8 cmd);
/******************************************************************************/
/*! @brief Adjustment Request Function.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void front_usb_authentication_request(void)
{
	u8 response = 0;

	if((ex_dipsw1 == DIPSW1_AUTHENTICATION_MODE) //ok
	 && (ex_front_usb.pc.mess.phase == 0x11))
	{
		switch(ex_front_usb.pc.mess.command)
		{
		case	CMD_READ:
				response = phase_authentication_read();
				break;
		case	CMD_WRITE:
				response = phase_authentication_write();
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

/******************************************************************************/
/*! @brief Get Authentication Data.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static	u8	phase_authentication_read(void)
{
	u8	response = 0;
#if 0
	int index;
	ex_usb_write_size = (FUSB_HEADER_SIZE + AUTHENTICATION_CUSTOMER_KEY_SIZE + AUTHENTICATION_NUMBER_KEY_SIZE);
	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
	*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
	*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
	*(ex_usb_write_buffer + 5) = RES_DATA;

	for(index = 0; index < AUTHENTICATION_CUSTOMER_KEY_SIZE; index ++)
	{
		*(ex_usb_write_buffer + 6 + index) = ex_Authentication.customerKEY[index];
	}
	for(index = 0; index < AUTHENTICATION_NUMBER_KEY_SIZE; index ++)
	{
		*(ex_usb_write_buffer + 6 + AUTHENTICATION_CUSTOMER_KEY_SIZE + index) = ex_Authentication.numberKEY[index];
	}
#else
	response = RES_NAK;
#endif

	return response;
}

/*****************************************************************************/
/*! @brief Set Authentication Data.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static	u8	phase_authentication_write(void)
{
	u8	response = RES_ACK;
	int index;

#if 0
	if (ex_front_usb.pc.mess.length == (FUSB_HEADER_SIZE + AUTHENTICATION_CUSTOMER_KEY_SIZE + AUTHENTICATION_NUMBER_KEY_SIZE))
	{
		for(index = 0; index < AUTHENTICATION_CUSTOMER_KEY_SIZE; index ++)
		{
			ex_Authentication.customerKEY[index] = *(ex_usb_read_buffer + 6 + index);
		}
		for(index = 0; index < AUTHENTICATION_NUMBER_KEY_SIZE; index ++)
		{
			ex_Authentication.numberKEY[index] = *(ex_usb_read_buffer + 6 + AUTHENTICATION_CUSTOMER_KEY_SIZE + index);
		}
		/*	write flag	*/
		ex_Authentication.functionStatus = NORMAL_SETTING_AUTHENTICATION_NUMBER;
		_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_IF_AUTHENTICATION, 0, 0, 0);
	}
	else
	{
		response = RES_NAK;
	}
#else
	response = RES_NAK;
#endif

	return(response);
}
/*--- End of File ---*/
