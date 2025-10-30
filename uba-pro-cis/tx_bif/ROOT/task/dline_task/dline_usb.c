/******************************************************************************/
/*! @addtogroup Main
    @file       dline_usb.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
// メンテナンスUSBの最上位ファイル。受信コマンドのプロトコル解析を行う。

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal_usb.h"

#include "dline_download.h"
#include "dline_suite.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS ******************************/

/************************** Function Prototypes ******************************/
static void front_usb_suite_command(void);
static void front_usb_original_command(void);
void front_usb_command_analyze(void);

/************************** External functions *******************************/
extern void dline_send_data(void);
extern u8 _dline_receive_data(void);
#if defined(USB_REAR_USE)
extern void dline_send_data2(void);
extern u8 _dline_receive_data2(void);
#endif 
extern void _dline_set_mode(u16 mode);
extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);

/************************** Variable declaration *****************************/

/************************** EXTERNAL VARIABLES *******************************/


/*********************************************************************//**
 * @brief		Main ROM update
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		TRUE Succeeded
 * @retval 		FALSE Failure
 **********************************************************************/
void dline_suite(void)
{
	u8 ret;

	ret = _dline_receive_data();
	if(ret == DLINE_RECEIVE_COMPLETE)
	{
		ex_usb_read_size = ex_rear_usb_read_size;
		_dline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_DATA_WAIT, 0, 0, 0);
		front_usb_command_analyze();
	}
	else if(ret == DLINE_RECEIVE_OK)
	{
		_dline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DATA_WAIT, WAIT_TIME_DATA_WAIT, 0, 0);
	}

	/* Send data */
	dline_send_data();
}
#if defined(USB_REAR_USE)
/*********************************************************************//**
 * @brief		Main ROM update
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		TRUE Succeeded
 * @retval 		FALSE Failure
 **********************************************************************/
void dline_suite2(void)
{
	u8 ret;

	ret = _dline_receive_data2();
	if(ret == DLINE_RECEIVE_COMPLETE)
	{
		memcpy(ex_usb_read_buffer, ex_rear_usb_read_buffer, ex_rear_usb_read_size);
		_dline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_DATA_WAIT, 0, 0, 0);
		front_usb_command_analyze();
	}
	else if(ret == DLINE_RECEIVE_OK)
	{
		_dline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DATA_WAIT, WAIT_TIME_DATA_WAIT, 0, 0);
	}
	ex_rear_usb_write_size = ex_usb_write_size;
	memcpy(ex_rear_usb_write_buffer, ex_usb_write_buffer, ex_usb_write_size);
	/* Send data */
	dline_send_data2();
}
#endif // USB_REAR_USE
/******************************************************************************/
/*! @brief Front USB Command analyze process
    @return         none
    @exception      none
******************************************************************************/
void front_usb_command_analyze(void)
{
	if(ex_usb_read_buffer[0] != CMD_SUITE_TYPE_ID)
	{
		front_usb_original_command();
	}
	else
	{
		front_usb_suite_command();
	}
	switch(ex_front_usb.status)
	{
	case DLINE_ANALYZE:
		switch(ex_front_usb.pc.mode)
		{
		default:
				break;
		}
		break;
	case DLINE_ANALYZE_SUITE:
		switch(ex_front_usb.pc.mode)
		{
		case	FUSB_SUITE_CUR_MODE:
				front_usb_suite_cur_mode();
				break;
		case	FUSB_SUITE_MODE_LIST:
				front_usb_suite_mode_list();
				break;
		case	FUSB_SUITE_CHANG_MODE:
				front_usb_suite_chang_mode();
				break;
		case	FUSB_SUITE_PRODUCT_ID:
				front_usb_suite_product_id();
				break;
		case	FUSB_SUITE_BOOT_ROM_VERSION:
				front_usb_suite_boot_rom_version();
				break;
		case	FUSB_SUITE_EX_ROM_VERSION:
				front_usb_suite_ex_rom_version();
				break;
		case	FUSB_SUITE_EX_ROM_CRC16:
				front_usb_suite_ex_rom_crc16();
				break;
		case	FUSB_SUITE_SERIAL_NUMBER:
				front_usb_suite_serial_number();
				break;
		case	FUSB_SUITE_ROM_STATUS:
				front_usb_suite_rom_status();
				break;
		case	FUSB_SUITE_PROTOCOL_ID:
				front_usb_suite_protocol_ID();
				break;
		case	FUSB_SUITE_MAIN_SOURCE_VERSION:
				front_usb_suite_ex_main_version();
				break;
		default:
				break;
		}
		break;
	default:
		break;
	}
}

u8 _dline_receive_data(void)
{
	//s32 numBytesToRead;
	s32 numAvailByte;
	u32 read_byte = 0;
	u32 r_size = 0;

	numAvailByte = UsbGetAvailableChar();

	if(numAvailByte > 0)
	{
		if(!ex_dline_pkt.offset_r)		/* 1st packet? */
		{
			if (numAvailByte > USB_BUFFER_SIZE)
			{
				read_byte = USB_BUFFER_SIZE;
			}
			else
			{
				read_byte = numAvailByte;
			}
			/////////////////////////////////////////////////////////////////
			OSW_ISR_disable( OSW_INT_USB0_IRQ );
			/* Block receiving the buffer. */
			r_size = UsbGetRecvData(
					&ex_usb_read_buffer[ex_dline_pkt.offset_r],
					read_byte);
			OSW_ISR_enable( OSW_INT_USB0_IRQ );
			/////////////////////////////////////////////////////////////////
			/* Check header */
			if(ex_usb_read_buffer[0] == CMD_SUITE_TYPE_ID)
			{
				/* length : 1byte */
				ex_dline_pkt.total_len = ex_usb_read_buffer[1];
			}
			else if((ex_usb_read_buffer[0] == CMD_ID_STATUS)
			|| (ex_usb_read_buffer[0] == CMD_ID_DNLOAD_START)
			|| (ex_usb_read_buffer[0] == CMD_ID_DNLOAD_DATA)
			|| (ex_usb_read_buffer[0] == CMD_ID_DNLOAD_END_CONF)
			|| (ex_usb_read_buffer[0] == CMD_ID_VERSION)
			|| (ex_usb_read_buffer[0] == CMD_ID_RESET)
			){
				// Download
				if(ex_usb_read_buffer[0] == CMD_ID_DNLOAD_DATA)
				{
					if(read_byte >= 3)
					{
						ex_dline_pkt.total_len = 3 + (ex_usb_read_buffer[1] << 8) + ex_usb_read_buffer[2];
					}
				}
				else
				{
					if(read_byte >= 2)
					{
						ex_dline_pkt.total_len = 2 + ex_usb_read_buffer[1];
					}
				}
			}
			ex_usb_read_size = r_size;
			ex_dline_pkt.offset_r = r_size;
		}
		else
		{
			if(numAvailByte > ex_dline_pkt.offset_r + ex_dline_pkt.total_len)
			{
				read_byte = ex_dline_pkt.total_len - ex_dline_pkt.offset_r;
			}
			else if (numAvailByte > USB_BUFFER_SIZE)
			{
				read_byte = USB_BUFFER_SIZE;
			}
			else
			{
				read_byte = numAvailByte;
			}
			/////////////////////////////////////////////////////////////////
			OSW_ISR_disable( OSW_INT_USB0_IRQ );
			/* Block receiving the buffer. */
			r_size = UsbGetRecvData(
					&ex_usb_read_buffer[ex_dline_pkt.offset_r],
					read_byte);
			OSW_ISR_enable( OSW_INT_USB0_IRQ );
			/////////////////////////////////////////////////////////////////
			ex_usb_read_size += r_size;
			ex_dline_pkt.offset_r += r_size;
		}

		if(ex_dline_pkt.offset_r == ex_dline_pkt.total_len)
		{
			memset((void *)&ex_dline_pkt, 0, sizeof(ex_dline_pkt));
			return DLINE_RECEIVE_COMPLETE;
		}
		else if(ex_dline_pkt.offset_r > ex_dline_pkt.total_len)
		{
			// format error
			memset((void *)&ex_dline_pkt, 0, sizeof(ex_dline_pkt));
			return DLINE_NO_DATA;
		}
		else
		{
			ex_dline_pkt.cnt ++;
			return DLINE_RECEIVE_OK;			/* waiting next packet */
		}
	}
	return DLINE_NO_DATA;
}

#if defined(USB_REAR_USE)
u8 _dline_receive_data2(void)
{
	//s32 numBytesToRead;
	s32 numAvailByte;
	u32 read_byte = 0;
	u32 r_size = 0;

	numAvailByte = Rear_UsbGetAvailableChar();

	if(numAvailByte > 0)
	{
		if(!ex_dline_pkt.offset_r)		/* 1st packet? */
		{
			if (numAvailByte > USB_BUFFER_SIZE)
			{
				read_byte = USB_BUFFER_SIZE;
			}
			else
			{
				read_byte = numAvailByte;
			}
			/////////////////////////////////////////////////////////////////
			OSW_ISR_disable( OSW_INT_USB1_IRQ );
			/* Block receiving the buffer. */
			r_size = Rear_UsbGetRecvData(
					&ex_rear_usb_read_buffer[ex_dline_pkt.offset_r],
					read_byte);
			OSW_ISR_enable( OSW_INT_USB1_IRQ );
			/////////////////////////////////////////////////////////////////
			/* Check header */
			if(ex_rear_usb_read_buffer[0] == CMD_SUITE_TYPE_ID)
			{
				/* length : 1byte */
				ex_dline_pkt.total_len = ex_rear_usb_read_buffer[1];
			}
			else if((ex_rear_usb_read_buffer[0] == CMD_ID_STATUS)
			|| (ex_rear_usb_read_buffer[0] == CMD_ID_DNLOAD_START)
			|| (ex_rear_usb_read_buffer[0] == CMD_ID_DNLOAD_DATA)
			|| (ex_rear_usb_read_buffer[0] == CMD_ID_DNLOAD_END_CONF)
			|| (ex_rear_usb_read_buffer[0] == CMD_ID_VERSION)
			|| (ex_rear_usb_read_buffer[0] == CMD_ID_RESET)
			){
				// Download
				if(ex_rear_usb_read_buffer[0] == CMD_ID_DNLOAD_DATA)
				{
					if(read_byte >= 3)
					{
						ex_dline_pkt.total_len = 3 + (ex_rear_usb_read_buffer[1] << 8) + ex_rear_usb_read_buffer[2];
					}
				}
				else
				{
					if(read_byte >= 2)
					{
						ex_dline_pkt.total_len = 2 + ex_rear_usb_read_buffer[1];
					}
				}
			}
			ex_rear_usb_read_size = r_size;
			ex_dline_pkt.offset_r = r_size;
		}
		else
		{
			if(numAvailByte > ex_dline_pkt.offset_r + ex_dline_pkt.total_len)
			{
				read_byte = ex_dline_pkt.total_len - ex_dline_pkt.offset_r;
			}
			else if (numAvailByte > USB_BUFFER_SIZE)
			{
				read_byte = USB_BUFFER_SIZE;
			}
			else
			{
				read_byte = numAvailByte;
			}
			/////////////////////////////////////////////////////////////////
			OSW_ISR_disable( OSW_INT_USB1_IRQ );
			/* Block receiving the buffer. */
			r_size = Rear_UsbGetRecvData(
					&ex_rear_usb_read_buffer[ex_dline_pkt.offset_r],
					read_byte);
			OSW_ISR_enable( OSW_INT_USB1_IRQ );
			/////////////////////////////////////////////////////////////////
			ex_rear_usb_read_size += r_size;
			ex_dline_pkt.offset_r += r_size;
		}

		if(ex_dline_pkt.offset_r == ex_dline_pkt.total_len)
		{
			memset((void *)&ex_dline_pkt, 0, sizeof(ex_dline_pkt));
			return DLINE_RECEIVE_COMPLETE;
		}
		else if(ex_dline_pkt.offset_r > ex_dline_pkt.total_len)
		{
			// format error
			memset((void *)&ex_dline_pkt, 0, sizeof(ex_dline_pkt));
			return DLINE_NO_DATA;
		}
		else
		{
			ex_dline_pkt.cnt ++;
			return DLINE_RECEIVE_OK;			/* waiting next packet */
		}
	}
	return DLINE_NO_DATA;
}
#endif // USB_REAR_USE


/******************************************************************************/
/*! @brief Front usb original command
    @return         none
    @exception      none
******************************************************************************/
static void front_usb_original_command(void)
{
	if((ex_usb_read_buffer[0] == 0xD0)
	 && (ex_usb_read_buffer[1] == 0x00))
	{
		*(ex_usb_write_buffer + 0) = 0xE0;
		*(ex_usb_write_buffer + 1) = 0x01;
		*(ex_usb_write_buffer + 2) = 0x00;
		ex_usb_write_size = 3;				/* send 3 Byte			*/
		_dline_set_mode(DLINE_MODE_DOWNLOAD_WAIT_RSP);
    	_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_DOWNLOAD_REQ, 0, 0, 0, 0);
		//2023-12-22 USBでのダウンロード受信の入口 Tool開いただけで来るはず
	}
}

/******************************************************************************/
/*! @brief Front usb suite command
    @return         none
    @exception      none
******************************************************************************/
static void front_usb_suite_command(void)
{
/*<<	Get length of message	>>*/
	ex_front_usb.pc.mess.length = ex_usb_read_buffer[1];
/*<<	Get mode ID				>>*/
	ex_front_usb.pc.mess.modeID = ex_usb_read_buffer[2];
/*<<	check each Mode command 			>>*/
	switch(ex_front_usb.pc.mess.modeID)
	{
	case	FUSB_SUITE_CUR_MODE:
	case	FUSB_SUITE_MODE_LIST:
	case	FUSB_SUITE_PRODUCT_ID:
	case	FUSB_SUITE_BOOT_ROM_VERSION:
	case	FUSB_SUITE_EX_ROM_VERSION:
	case	FUSB_SUITE_EX_ROM_CRC16:
	case	FUSB_SUITE_SERIAL_NUMBER:
	case	FUSB_SUITE_ROM_STATUS:
	case	FUSB_SUITE_PROTOCOL_ID:
	case	FUSB_SUITE_MAIN_SOURCE_VERSION:
		ex_front_usb.pc.mode = ex_front_usb.pc.mess.modeID;
		ex_front_usb.status = DLINE_ANALYZE_SUITE;
		break;
	case	FUSB_SUITE_CHANG_MODE:
	/*<<	Get phase number		>>*/
		ex_front_usb.pc.mess.phase = ex_usb_read_buffer[3];
		ex_front_usb.pc.mode = ex_front_usb.pc.mess.modeID;
		ex_front_usb.status = DLINE_ANALYZE_SUITE;
		break;
	default:
		break;
	}
}



void dline_send_data(void)
{
	if(ex_usb_write_size)
	{
		UsbReqSendData();
	}
}

#if defined(USB_REAR_USE)
void dline_send_data2(void)
{
	if(ex_rear_usb_write_size)
	{
		Rear_UsbReqSendData();
	}
}
#endif // USB_REAR_USE
/******************************************************************************/
/*! @brief Initialize USB command sequence.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void usb_command_init(u8 command)
{
	ex_usb_command.command = command;
	ex_usb_command.enq_count = 0;
	ex_usb_command.seq_sent = 0;
	ex_usb_command.seq_received = 0;
};

/******************************************************************************/
/*! @brief send one command message
    @return         none
    @exception      none
******************************************************************************/
void set_response_1data(u8 cmd)
{
	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 1) = 0x00;
	*(ex_usb_write_buffer + 2) = 0x06;
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
	*(ex_usb_write_buffer + 5) = cmd;		/* set illegal status	*/
	ex_usb_write_size = 6;				/* send 6 Byte			*/

	usb_command_init(cmd);
}



/* EOF */
