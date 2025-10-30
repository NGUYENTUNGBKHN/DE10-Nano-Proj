/******************************************************************************/
/*! @addtogroup Main
    @file       dline_fpga.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_fpga.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "motor_ctrl.h"
#include "dline_suite.h"
#include "dline_test.h"
#include "dline_adjustment_mode.h"
#include "dline_fpga.h"
#include "hal.h"

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS ******************************/

/************************** Function Prototypes ******************************/
/************************** External functions *******************************/
extern void set_response_1data(u8 cmd);

/************************** Variable declaration *****************************/

/************************** EXTERNAL VARIABLES *******************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/*==============================================================================*/
/* DataCollection send data																		*/
/*==============================================================================*/
void data_collection_send( u32 usb_write_size )
{
	UINT32 numBytesToWrite = 0;

	if(usb_write_size)
	{
		numBytesToWrite = usb_write_size;
		/* Write bytes into the TX FIFO */
		/* Transmit the data */
		GRUSB_COMD_SendData(numBytesToWrite,
					   (UINT8 *)(ex_usb_pbuf),
					   0);
	}
	return;
}
extern const void * Image$$FPGA_LOG$$Base;
static u8 phase_fpga_log(void)
{
	//debug
	unsigned int size;
	void *addr;
	addr = &Image$$FPGA_LOG$$Base;
	size = FPGA_LOG_SIZE;
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_READ:
		ex_usb_pbuf = addr;
		data_collection_send(size);
		ex_usb_pbuf = 0;
		break;
	case CMD_ADDRESS:
		ex_usb_write_size = 6 + 4;				/* send 10 Byte			*/
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x06;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = ex_front_usb.pc.mess.command;		/* set illegal status	*/
		memcpy( ex_usb_write_buffer + 6, &ex_fpga_event_log_address, 4);
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_fpga_version(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_READ:
		ex_usb_write_size = 6 + 4;				/* send 10 Byte			*/
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x06;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;
		memcpy( ex_usb_write_buffer + 6, &ex_fpga_version, 4);
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
/******************************************************************************/
/*! @brief Test mode request from PC
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void front_usb_fpga_event_log_request( void )
{
/*<<	テストモード及び各モードのフェーズ確認		>>*/
	if((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_FPGA_EVENT_LOG_REQUEST)
	{
		switch (ex_front_usb.pc.mess.phase)
		{
		case PHASE_FPGA_EVENT_LOG:
			phase_fpga_log();
			break;
		case PHASE_FPGA_VERSION:
			phase_fpga_version();
			break;
		default:
			set_response_1data(NAK);
			break;
		}
	}
	else
	{
		set_response_1data(NAK);
	}
}

/* EOF */
