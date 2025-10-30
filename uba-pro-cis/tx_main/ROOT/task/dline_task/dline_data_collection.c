/******************************************************************************/
/*! @addtogroup Main
    @file       dline_data_collection.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_data_collection.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/******************************************************************************/
/*! @brief Front USB Suite make product id command process
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "sensor.h"
#include "struct.h"
#include "dline_test.h"
#include "dline_data_collection.h"
#include "../../pl/pl_cis.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "usb_ram.c"
#include "cis_ram.c"

/************************** Variable declaration *****************************/
/************************** EXTERNAL VARIABLES *******************************/
extern u16 ex_dline_task_mode;
extern struct _testmode ex_dline_testmode;
/************************** Function Prototypes ******************************/

/************************** External functions *******************************/
extern void _dline_set_mode(u16 mode);
extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void set_response_1data(u8 cmd);

/************************** PRIVATE FUNCTIONS *************************/
void send_data_collection_header(void);
u8 send_data_collection_data(void);
void send_data_collection_status(void);

void send_data_collection_force_read(void);

/******************************************************************************/
/*! @brief Set Data Collection Header Data to send buffer.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
    @histry
v0.01:
v0.02: remove date area, and add change_led_light
******************************************************************************/
void send_data_collection_header(void)
{
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.last%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif

	/* set send length */
	ex_usb_write_size = ALL_INFO_SIZE;
	ex_usb_pbuf = (u8 *)&pbill_data->version;
	if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
	{
		ex_dline_testmode.action = TEST_USB_CONTROL;
		ex_dline_testmode.test_no = TEST_ACCEPT_LD_ALLACC;
		_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
	}
	ex_collection_data.enable = true;
	ex_collection_data.data_result = DATA_SUCCESS;

	ex_usb_buf_change = 1;
	parameter_set(pbill_data);
}
/******************************************************************************/
/*! @brief Set Data Collection Header Data to send buffer. not initillize.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
    @histry
v0.01:
******************************************************************************/
void send_data_collection_information(void)
{
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.last%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif
	/* set send length */
	ex_usb_write_size = ALL_INFO_SIZE;
	ex_usb_pbuf = (u8 *)&pbill_data->version;

	ex_usb_buf_change = 1;
	parameter_set(pbill_data);
}
/******************************************************************************/
/*! @brief Set Data Collection Data to send buffer.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
u8 send_data_collection_data(void)
{
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.last%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif




	ex_usb_write_size = RESULT_INFO_SIZE + pbill_data->Blockbytesize * pbill_data->block_count;//sizeof(SENSOR_DATA);

	ex_usb_pbuf = (u8 *)&pbill_data->proc_num;

	ex_usb_buf_change = 1;
	ex_collection_data.data_exist = DATA_REQEST;
	return 0;
}

/******************************************************************************/
/*! @brief Set Data Collection Status to send buffer.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void send_data_collection_status(void)
{
	/* set send length */
	ex_usb_write_size = 6;

	*ex_usb_write_buffer = FUSB_SERVICE_ID_DATA_COLLECTION;
	*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
	*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;

	if(ex_collection_data.enable
	&& ex_collection_data.data_exist == DATA_EXIST)
	{
		*(ex_usb_write_buffer + 5) = 1;
	}
	else
	{
		*(ex_usb_write_buffer + 5) = 0;
	}
}
void send_data_collection_force_read(void)
{/*<<	外形検知に失敗したデータも取得可能	>>*/
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data;
	int index;
	if(ex_usb_read_buffer[6] > BILL_NOTE_IMAGE_MAX_COUNT - 1)
	{
		// parameter error : set last buffer

		index = ex_cis_image_control.last;
	}
	else if(ex_cis_image_control.last < ex_usb_read_buffer[6])
	{
		index = (BILL_NOTE_IMAGE_MAX_COUNT) - abs(ex_cis_image_control.last-ex_usb_read_buffer[6]);
	}
	else
	{
		index = ex_cis_image_control.last-ex_usb_read_buffer[6];
	}
	pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[index%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif

	ex_usb_write_size = FILE_SIZE;
	ex_usb_pbuf = (u8 *)pbill_data;

	ex_usb_buf_change = 1;
	ex_collection_data.data_exist = DATA_REQEST;
}

void front_usb_data_collection_request()
{
	/*<<	紙幣データモード及び各モードのフェーズ確認		>>*/
	if((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_DATA_COLLECTION_REQUEST)
	{
		switch(ex_front_usb.pc.mess.phase)
		{
		case PHASE_DATA_COLLECTION:
			switch(ex_front_usb.pc.mess.command)
			{
			case 0:
				 send_data_collection_header();
				break;
			default:



				if(ex_collection_data.data_exist == DATA_EXIST)
				{
					send_data_collection_data();
				}
				else
				{
					set_response_1data(NAK);
				}

				break;
			}
			break;
		case PHASE_DATA_COLLECTION_STATUS:
			switch(ex_front_usb.pc.mess.command)
			{
			case 0:
				send_data_collection_status();
				break;
			case 1:
				send_data_collection_force_read();
				break;
			case 2:
				send_data_collection_information();
				break;
			default:
				set_response_1data(NAK);
				break;
			}
			break;
		default:
			/*<<	clear command waiting flag	>>*/
//			ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
			set_response_1data(NAK);
			break;
		}
	}
	else
	{
		set_response_1data(NAK);
	}
}
