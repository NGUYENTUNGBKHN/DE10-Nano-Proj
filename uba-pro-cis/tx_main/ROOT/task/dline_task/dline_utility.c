/******************************************************************************/
/*! @addtogroup Main
    @file       dline_utility.c
    @date       2018/01/24
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal.h"
#include "dline_suite.h"
#include "dline_test.h"
#include "sub_functions.h"
#include "user/icb/icb.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS ******************************/
enum ICB_FUNCTION_PHASE_NUMBER
{
	PHASE_ICB_STATUS 	= 0x01,
	PHASE_ICB_NUMBER 	= 0x02,
	PHASE_ICB_FUNCTION 	= 0x03,
};
#define CMD_ICB_ENABLE_SETTING	(0x01)
#define CMD_ICB_DISABLE_SETTING	(0xFF)
#define CMD_ICB_ENQ				(0x05)
#define CMD_ICB_NUMBER_SETTING	(0x01)
#define CMD_ICB_FUNC_INHIBIT	(0x01)
/************************** EXTERNAL VARIABLES *******************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/
extern void set_response_1data(u8 cmd);
extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
/************************** Variable declaration *****************************/

static void phase_icb_enable(void)
{
	switch (ex_front_usb.pc.mess.command)	/* 6byte目*/
	{
	case CMD_ICB_ENABLE_SETTING:	/* 0x01 0x01 */
		set_ICBenable_flag();
		_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		set_response_1data(ACK);
		break;
	case CMD_ICB_DISABLE_SETTING:	/* 0x01 0xFF */
		set_ICBdisable_flag();
		_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		set_response_1data(ACK);
		break;
	case CMD_ICB_ENQ:	/* 0x01 0x05 */
		if(check_ICBflag()) //2023-11-13 is_icb_enableはテストモードチェックがあるので使用しない
		{
			ex_usb_write_size = 6 + 1;				/* send 6+1 Byte			*/
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = 0x00;
			*(ex_usb_write_buffer + 2) = 0x07;
			*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_DATA;
			*(ex_usb_write_buffer + 6) = 0x01;
		}
		else
		{
			ex_usb_write_size = 6 + 1;				/* send 6+1 Byte			*/
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = 0x00;
			*(ex_usb_write_buffer + 2) = 0x07;
			*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_DATA;
			*(ex_usb_write_buffer + 6) = 0xff;
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
}
static void phase_icb_number(void)
{
	switch (ex_front_usb.pc.mess.command)	/* 6byte目*/
	{
	case CMD_ICB_NUMBER_SETTING:
		set_MCnumber(&ex_usb_read_buffer[6]);
		_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		set_response_1data(ACK);
		break;
	case CMD_ICB_ENQ:
		ex_usb_write_size = 6 + BAR_MC_LNG - 2;				/* send 20 Byte			*/
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)(ex_usb_write_size >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;
		memcpy( ex_usb_write_buffer + 6, &ex_ICB_gameno, BAR_MC_LNG - 2);
		break;
	default:
		set_response_1data(NAK);
		break;
	}
}
static void phase_icb_function(void)
{
	switch (ex_front_usb.pc.mess.command)	/* 6byte目*/
	{
	case CMD_ICB_FUNC_INHIBIT:	//RFIDのタグ側にInhibit情報を書き込む,UBAは未使用
		//_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_ICB_INHIBIT_REQ, 0, 0, 0, 0);
		set_response_1data(NAK);
		break;
	case CMD_ICB_ENQ:
		/* Utility Tool起動時によびだされるので、残しておく*/
		/* UBAは仕様が決まっていないので、Tag側は常にEnable状態 */ //2023-05-12
			ex_usb_write_size = 6 + 1;				/* send 6+1 Byte			*/
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = 0x00;
			*(ex_usb_write_buffer + 2) = 0x07;
			*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_DATA;
		//	*(ex_usb_write_buffer + 6) = Smrtdat.flg;
			*(ex_usb_write_buffer + 6) = 0x01;
			break;
	default:
		set_response_1data(NAK);
		break;
	}
}
void front_usb_icb_function_request(void)
{
	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_UTILITY_MODE;
	/*<<	テストモード及び各モードのフェーズ確認		>>*/
	if((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_ICB_FUNCTION)
	{
		switch (ex_front_usb.pc.mess.phase)	/* 5byte目*/
		{
		case PHASE_ICB_STATUS:
			phase_icb_enable();
			break;
		case PHASE_ICB_NUMBER:
			phase_icb_number();
			break;
		case PHASE_ICB_FUNCTION:
			phase_icb_function();
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
