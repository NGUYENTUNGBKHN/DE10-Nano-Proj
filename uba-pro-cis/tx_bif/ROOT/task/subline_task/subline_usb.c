/******************************************************************************/
/*! @addtogroup Main
    @file       subline_usb.c
    @date       2020/01/29
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
// オペレーションUSB（Subline)の最上位ファイル。受信コマンドのプロトコル解析を行う。

/***************************** Include Files *********************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"

#include "hal_operation_usb.h"
#include "subline_usb.h"
#include "subline_download.h"
#include "subline_suite.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS ******************************/

/************************** Function Prototypes ******************************/
u8 subline_receive_data(void);
void subline_send_data(void);
static void _operation_usb_suite_command(void);
static void _operation_usb_original_command(void);
static void _operation_usb_command_analyze(void);
/************************** External functions *******************************/
extern void _subline_set_mode(u16 mode);
extern void _subline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);

/************************** Variable declaration *****************************/

/*********************************************************************//**
 * @brief		Main ROM update
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		TRUE Succeeded
 * @retval 		FALSE Failure
 **********************************************************************/
void subline_suite(void)
{
	u8 ret;

	ret = subline_receive_data();
	if(ret == SUBLINE_RECEIVE_COMPLETE)
	{
		_operation_usb_command_analyze();
	}

	/* Send data */
	subline_send_data();
}
/******************************************************************************/
/*! @brief Operation USB Command analyze process
    @return         none
    @exception      none
******************************************************************************/
static void _operation_usb_command_analyze(void)
{
	if(ex_operation_usb_read_buffer[0] != CMD_SUITE_TYPE_ID)
	{
		_operation_usb_original_command();
	}
	else
	{
		_operation_usb_suite_command();
	}
	switch(ex_operation_usb.status)
	{
	case SUBLINE_ANALYZE:
		break;
	case SUBLINE_ANALYZE_SUITE:
		switch(ex_operation_usb.pc.mode)
		{
		case	OUSB_SUITE_CUR_MODE:
				subline_usb_suite_cur_mode();
				break;
		case	OUSB_SUITE_MODE_LIST:
				subline_usb_suite_mode_list();
				break;
		case	OUSB_SUITE_CHANG_MODE:
				subline_usb_suite_chang_mode();
				break;
		case	OUSB_SUITE_PRODUCT_ID:
				subline_usb_suite_product_id();
				break;
		case	OUSB_SUITE_BOOT_ROM_VERSION:
				subline_usb_suite_boot_rom_version();
				break;
		case	OUSB_SUITE_EX_ROM_VERSION:
				subline_usb_suite_ex_rom_version();
				break;
		case	OUSB_SUITE_EX_ROM_CRC16:
				subline_usb_suite_ex_rom_crc16();
				break;
		case	OUSB_SUITE_SERIAL_NUMBER:
				subline_usb_suite_serial_number();
				break;
		case	OUSB_SUITE_ROM_STATUS:
				subline_usb_suite_rom_status();
				break;
		case	OUSB_SUITE_PROTOCOL_ID:
				subline_usb_suite_protocol_ID();
				break;
		case	OUSB_SUITE_MAIN_SOURCE_VERSION:
				subline_usb_suite_ex_main_version();
				break;
		default:
				break;
		}
		break;
	default:
		break;
	}
}

/******************************************************************************/
/*! @brief Front usb original command
    @return         none
    @exception      none
******************************************************************************/
static void _operation_usb_original_command(void)
{
	if((ex_operation_usb_read_buffer[0] == 0xD0)
	 && (ex_operation_usb_read_buffer[1] == 0x00))
	{
		*(ex_operation_usb_write_buffer + 0) = 0xE0;
		*(ex_operation_usb_write_buffer + 1) = 0x01;
		*(ex_operation_usb_write_buffer + 2) = 0x00;
		ex_operation_usb_write_size = 3;				/* send 3 Byte			*/
		_subline_set_mode(SUBLINE_MODE_DOWNLOAD_WAIT_RSP);
    	_subline_send_msg(ID_MAIN_MBX, TMSG_SUBLINE_DOWNLOAD_REQ, 0, 0, 0, 0);
	}
}

/******************************************************************************/
/*! @brief Front usb suite command
    @return         none
    @exception      none
******************************************************************************/
static void _operation_usb_suite_command(void)
{
/*<<	Get length of message	>>*/
	ex_operation_usb.pc.mess.length = ex_operation_usb_read_buffer[1];
/*<<	Get mode ID				>>*/
	ex_operation_usb.pc.mess.modeID = ex_operation_usb_read_buffer[2];
/*<<	check each Mode command 			>>*/
	switch(ex_operation_usb.pc.mess.modeID)
	{
	case	OUSB_SUITE_CUR_MODE:
	case	OUSB_SUITE_MODE_LIST:
	case	OUSB_SUITE_PRODUCT_ID:
	case	OUSB_SUITE_BOOT_ROM_VERSION:
	case	OUSB_SUITE_EX_ROM_VERSION:
	case	OUSB_SUITE_EX_ROM_CRC16:
	case	OUSB_SUITE_SERIAL_NUMBER:
	case	OUSB_SUITE_ROM_STATUS:
	case	OUSB_SUITE_PROTOCOL_ID:
	case	OUSB_SUITE_MAIN_SOURCE_VERSION:
		ex_operation_usb.pc.mode = ex_operation_usb.pc.mess.modeID;
		ex_operation_usb.status = SUBLINE_ANALYZE_SUITE;
		break;
	case	OUSB_SUITE_CHANG_MODE:
	/*<<	Get phase number		>>*/
		ex_operation_usb.pc.mess.phase = ex_operation_usb_read_buffer[3];
		ex_operation_usb.pc.mode = ex_operation_usb.pc.mess.modeID;
		ex_operation_usb.status = SUBLINE_ANALYZE_SUITE;
		break;
	default:
		break;
	}
}


u8 subline_receive_data(void)
{
	s32 numAvailByte;
	u32 read_byte = 0;
	u32 r_size = 0;

	numAvailByte = OperationUsbGetAvailableChar();

	if(numAvailByte > 0)
	{
		if (numAvailByte > USB_BUFFER_SIZE)
		{
			read_byte = USB_BUFFER_SIZE;
		}
		else
		{
			read_byte = numAvailByte;
		}
		OSW_ISR_disable( OSW_INT_USB0_IRQ );
		/* Block receiving the buffer. */
		r_size = OperationUsbGetRecvData(
				&ex_operation_usb_read_buffer[ex_subline_pkt.offset_r],
				read_byte);
		OSW_ISR_enable( OSW_INT_USB0_IRQ );

		if(!ex_subline_pkt.offset_r)		/* 1st packet? */
		{
			/* Check header */
			if(ex_operation_usb_read_buffer[0] == CMD_SUITE_TYPE_ID)
			{
				/* length : 1byte */
				ex_subline_pkt.total_len = ex_operation_usb_read_buffer[1];
			}
			else
			{
				// Download
				if(ex_operation_usb_read_buffer[0] == CMD_ID_DNLOAD_DATA)
				{
					if(read_byte >= 3)
					{
						ex_subline_pkt.total_len = 3 + (ex_operation_usb_read_buffer[1] << 8) + ex_operation_usb_read_buffer[2];
					}
				}
				else
				{
					if(read_byte >= 2)
					{
						ex_subline_pkt.total_len = 2 + ex_operation_usb_read_buffer[1];
					}
				}
			}
			ex_operation_usb_read_size = r_size;
			ex_subline_pkt.offset_r = r_size;
		}
		else
		{
			ex_operation_usb_read_size += r_size;
			ex_subline_pkt.offset_r += r_size;
		}

		if(ex_subline_pkt.offset_r == ex_subline_pkt.total_len)
		{
			ex_subline_pkt.offset_r = 0;
			return SUBLINE_RECEIVE_COMPLETE;
		}
		ex_subline_pkt.cnt ++;
		return SUBLINE_RECEIVE_OK;			/* waiting next packet */
	}
	else
	{
		if(ex_subline_pkt.cnt)
		{
			ex_subline_pkt.cnt ++;
			if(ex_subline_pkt.cnt > 10)	/* timeout */
			{
				ex_operation_usb_read_size = 0;
				ex_subline_pkt.offset_r = 0;
				ex_subline_pkt.cnt = 0;
			}
		}
		return SUBLINE_NO_DATA;
	}
}

void subline_send_data(void)
{
	if(ex_operation_usb_write_size)
	{
		OperationUsbReqSendData();
	}
}
