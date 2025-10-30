/******************************************************************************/
/*! @addtogroup Main
    @file       dline_bill_data.c
    @date       2019/08/29
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2019 Japan CashMachine Co., Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_test.c
 *
 *  Created on: 2019/08/29
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
#include "dline_suite.h"
#include "dline_test.h"
#include "dline_bill_data.h"
#include "hal.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "usb_ram.c"
#include "cis_ram.c"

/************************** PRIVATE DEFINITIONS ******************************/

/************************** Function Prototypes ******************************/
/************************** External functions *******************************/

extern void _dline_set_mode(u16 mode);
//extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void set_response_1data(u8 cmd);

/************************** Variable declaration *****************************/
extern struct _testmode ex_dline_testmode;

/************************** EXTERNAL VARIABLES *******************************/
//extern u16 ex_dline_task_mode;

/************************** Function Prototypes ******************************/
void send_accept_status_value(void);
void send_accept_status_poll(void);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/******************************************************************************/
/*! @brief send validation result
    @par            Refer
    - 参照するグローバル変数
		ex_usb_write_buffer
		ex_validation
		ex_monitor_info
    @return         none
    @exception      none
******************************************************************************/
void send_accept_status_value(void)
{
#if DEBUG_VALIDATION_RESULT
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif
	u8 *p_data = NULL;

	ex_usb_write_size = 6 + sizeof(BV_MEMORY) + ALL_INFO_SIZE;

	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 1) = (u8)(ex_usb_write_size >> 8);
	*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
	*(ex_usb_write_buffer + 5) = RES_DATA;

	p_data = (u8*)&ex_validation;	// set send data point
	memcpy(ex_usb_write_buffer + 6, p_data, sizeof(BV_MEMORY));
	p_data = (u8*)pbill_data;	// set send data point
	memcpy(ex_usb_write_buffer + 6 + sizeof(BV_MEMORY), p_data, ALL_INFO_SIZE);

	ex_monitor_info.data_exist = false;
#else
	set_response_1data(NAK);
#endif
};

/******************************************************************************/
/*! @brief send validation data exist
    @par            Refer
    - 参照するグローバル変数
		ex_usb_write_buffer
		ex_monitor_info
    @return         none
    @exception      none
******************************************************************************/
void send_accept_status_poll(void)
{
#if DEBUG_VALIDATION_RESULT
	ex_usb_write_size = 6;
	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;

	if(ex_monitor_info.data_exist)
	{
		*(ex_usb_write_buffer + 5) = RES_DATA;
	}
	else
	{
		*(ex_usb_write_buffer + 5) = RES_BUSY;
	}
	*(ex_usb_write_buffer + 1) = (u8)(ex_usb_write_size >> 8);
	*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
#else
	set_response_1data(NAK);
#endif
}

/******************************************************************************/
/*! @brief Test mode request from PC
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void front_usb_bill_data_request( void )
{
	//u8 response = 0;
	u8 reset = 0;

/*<<	テストモード及び各モードのフェーズ確認		>>*/
	if((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_WAVE_DATA_REQUEST)
	{
		switch (ex_front_usb.pc.mess.phase)
		{
		case PHASE_ACCEPT_STATUS:
			switch(ex_front_usb.pc.mess.command)
			{
			case CMD_RUN:
				set_response_1data(ACK);
				break;
			case CMD_ENQ:
				send_accept_status_value();
				break;
			case CMD_POLL:
				send_accept_status_poll();
				break;
			default:
				break;
			}
			break;
		default:
			/*<<	clear command waiting flag	>>*/
			ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
			/*	*/
			set_response_1data(NAK);
			break;
		}
	}
	else
	{
		set_response_1data(NAK);
	}
	if(reset)
	{
		reset = 1;
		ex_dline_testmode.action = TEST_NON_ACTION;
		ex_dline_testmode.test_no = TEST_STANDBY;
		_dline_set_mode(DLINE_MODE_TEST_STANDBY);
	}
}


/* EOF */
