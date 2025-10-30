/******************************************************************************/
/*! @addtogroup Main
    @file       dline_jdl.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_jdl.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "cyc.h"
#include "motor_ctrl.h"
#include "dline_jdl.h"
#include "hal.h"
#include "../jdl/include/jdl.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"
/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/
static u8 phase_jdl_start(u8 cmd);
static u8 phase_jdl_data(u8 cmd);
static u8 phase_jdl_end(u8 cmd);
static u8 phase_jdl_clear(u8 cmd);

/************************** External functions *******************************/
extern void set_response_1data(u8 cmd);

/******************************************************************************/
/*! @brief JDL Request Function.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void front_usb_jdl_request(void)
{
	u8 response = 0;

	if ((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_JDL_REQUEST)
	{
		switch (ex_front_usb.pc.mess.phase)
		{
		case USB_JDL_PHASE_START:
			response = phase_jdl_start(ex_front_usb.pc.mess.command);
			break;
		case USB_JDL_PHASE_DATA:
			response = phase_jdl_data(ex_front_usb.pc.mess.command);
			break;
		case USB_JDL_PHASE_END:
			response = phase_jdl_end(ex_front_usb.pc.mess.command);
			break;
		case USB_JDL_PHASE_CLEAR:
			response = phase_jdl_clear(ex_front_usb.pc.mess.command);
			break;
		default:
			response = USB_JDL_RES_NAK;
			break;
		}
	}
	else
	{
		response = USB_JDL_RES_NAK;
	}

	if (response != 0)
	{
		set_response_1data(response);
	}
}

/******************************************************************************/
/*! @brief Read JDL.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_jdl_start(u8 cmd)
{
	u8 response = 0;
	u8 rtn;
	u32 data_length;
	u32 temp;

	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_JDL;

	switch(cmd)
	{
	case USB_JDL_HOST_CMD_REQ:
		rtn = jdl_get_sta(&data_length);
		if (rtn == JDL_E_OK)
		{
			response = USB_JDL_RES_OK;

			ex_usb_write_size = (FUSB_HEADER_SIZE + sizeof(data_length));

			temp = _JDL_SWAP_32(data_length);

			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = USB_JDL_RES_OK;
			memcpy(ex_usb_write_buffer + FUSB_HEADER_SIZE, &temp, sizeof(temp));
#if 0  /* yamazaki DEBUG */
			send_response();
#endif /* yamazaki DEBUG */
		}
		else
		{
			response = USB_JDL_RES_NAK;
		}
		break;
	default:
		response = USB_JDL_RES_NAK;
		break;

	}

	return(response);
}


/******************************************************************************/
/*! @brief Read JDL.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_jdl_data(u8 cmd)
{
	u8 response = 0;
	u8 rtn;
	u32 data_length;
	u32 temp;
	static u32 s_jdl_offset;

	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_JDL;

	switch(cmd)
	{
	case USB_JDL_HOST_CMD_REQ:
		memcpy((u8 *)&temp, (ex_usb_read_buffer + FUSB_HEADER_SIZE), USB_JDL_ADDR_SIZE);
		s_jdl_offset = _JDL_SWAP_32(temp);
#if defined(UBA_RTQ)
		rc_fram_log.read_offset = _JDL_SWAP_32(temp);
#endif // UBA_RTQ

		rtn = jdl_req_data(s_jdl_offset, USB_JDL_PACKT_SIZE);
		if(rtn == JDL_E_OK)
		{
#if defined(UBA_RTQ)
			rc_fram_log.wait_flg = FALSE;
#endif // UBA_RTQ
			response = USB_JDL_RES_ACK;
		}
#if defined(UBA_RTQ)
		else if (rtn == JDL_E_BUSY)
		{
			rc_fram_log.wait_flg = TRUE;
			response = USB_JDL_RES_ACK;
		}
#endif // UBA_RTQ
		else
		{
			response = USB_JDL_RES_NAK;
		}
		break;
	case USB_JDL_HOST_CMD_ENQ:
#if defined(UBA_RTQ)
		if(rc_fram_log.wait_flg == FALSE)
		{
			rtn = jdl_get_data((u8 *)&ex_usb_write_buffer[FUSB_HEADER_SIZE + USB_JDL_ADDR_SIZE], USB_JDL_PACKT_SIZE, s_jdl_offset, &data_length);
			if ((rtn == JDL_E_OK) && (data_length != 0))
			{
				response = USB_JDL_RES_OK;
				
				ex_usb_write_size = (FUSB_HEADER_SIZE + USB_JDL_ADDR_SIZE + data_length);
				
				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = USB_JDL_RES_OK;
				temp = _JDL_SWAP_32(s_jdl_offset);
				memcpy((ex_usb_write_buffer + FUSB_HEADER_SIZE), (u8 *)&temp, USB_JDL_ADDR_SIZE);
#if 0  /* yamazaki DEBUG */
				send_response();
#endif /* yamazaki DEBUG */
			}
		}
		else if(rc_fram_log.wait_flg == TRUE)
		{
			response = USB_JDL_RES_BUSY;
		}
		else
		{
			response = USB_JDL_RES_NAK;
		}
#else
		rtn = jdl_get_data((u8 *)&ex_usb_write_buffer[FUSB_HEADER_SIZE + USB_JDL_ADDR_SIZE], USB_JDL_PACKT_SIZE, s_jdl_offset, &data_length);
		if ((rtn == JDL_E_OK) && (data_length != 0))
		{
			response = USB_JDL_RES_OK;

			ex_usb_write_size = (FUSB_HEADER_SIZE + USB_JDL_ADDR_SIZE + data_length);

			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = USB_JDL_RES_OK;
			temp = _JDL_SWAP_32(s_jdl_offset);
			memcpy((ex_usb_write_buffer + FUSB_HEADER_SIZE), (u8 *)&temp, USB_JDL_ADDR_SIZE);
		}
		else
		{
			response = USB_JDL_RES_NAK;
		}
#endif // UBA_RTQ
		break;
	default:
		response = USB_JDL_RES_NAK;
		break;

	}

	return(response);
}


/******************************************************************************/
/*! @brief Clear JDL.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_jdl_end(u8 cmd)
{
	u8 response = 0;
	u8 rtn;

	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_JDL;

	switch(cmd)
	{
	case USB_JDL_HOST_CMD_REQ:
		rtn = jdl_get_end();
		if (rtn == JDL_E_OK)
		{
			response = USB_JDL_RES_OK;
			ex_usb_write_size = FUSB_HEADER_SIZE;

			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = USB_JDL_RES_ACK;
#if 0  /* yamazaki DEBUG */
			send_response();
#endif /* yamazaki DEBUG */
		}
		else
		{
			response = USB_JDL_RES_NAK;
		}
		break;
	default:
		response = USB_JDL_RES_NAK;
		break;
	}

	return(response);
}


/******************************************************************************/
/*! @brief Clear JDL.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_jdl_clear(u8 cmd)
{
	u8	response = 0;
	u8	rtn;
	u16 category;

	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_JDL;

	switch(cmd)
	{
	case USB_JDL_HOST_CMD_REQ:
		category = (u16)(ex_usb_read_buffer[(FUSB_HEADER_SIZE)] << 8);
		category += (u16)(ex_usb_read_buffer[(FUSB_HEADER_SIZE + 1)]);

		rtn = jdl_category_clear(category);
		if(rtn == JDL_E_OK)
		{
			response = USB_JDL_RES_OK;
		}
		else
		{
			response = USB_JDL_RES_NAK;
		}
		ex_usb_write_size = FUSB_HEADER_SIZE;

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = USB_JDL_RES_ACK;
		break;
	default:
		response = USB_JDL_RES_NAK;
		break;
	}

	return(response);
}


/*---	End of file	---*/

