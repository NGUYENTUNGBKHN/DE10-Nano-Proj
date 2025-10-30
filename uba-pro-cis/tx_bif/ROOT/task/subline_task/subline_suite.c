/******************************************************************************/
/*! @addtogroup Main
    @file       subline_suite.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "js_usb_test.h"
#include "subline_download.h"
#include "subline_suite.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS ******************************/
/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/************************** EXTERNAL VARIABLES *******************************/
extern const unsigned char _boot_if_version[16];
extern const u8 software_ver[];

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/
extern void _subline_set_mode(u16 mode);
extern void _subline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);

/************************** Variable declaration *****************************/

/*****************************************************************************/
/*! @brief initialize jcm suite variables
    @par            Refer
    - 参照するグローバル変数 ex_system
    @par            Modify
    - 変更するグローバル変数 ex_subline_suite_item
    @return         none
    @exception      none
******************************************************************************/
void subline_usb_suite_initial(void)
{
	ex_operation_usb.status = SUBLINE_ANALYZE;
	ex_subline_suite_item.curent_service = SUITE_ITEM_DOWNLOAD;
	ex_subline_suite_item.service_list = BIT_SUITE_ITEM_LIST;
	ex_subline_suite_item.curent_service = SUITE_ITEM_DOWNLOAD;
}

/******************************************************************************/
/*! @brief Operation USB Suite make product id command process
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_product_id(void)
{
	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 1) = 0x05;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_PRODUCT_ID;
	*(ex_operation_usb_write_buffer + 3) = (u8)(JCM_PRODUCT_ID >> 8);
	*(ex_operation_usb_write_buffer + 4) = (u8)(JCM_PRODUCT_ID);
	ex_operation_usb_write_size = 5;				/* send 5 Byte			*/
}

/******************************************************************************/
/*! @brief Operation USB Suite make external rom CRC16 command process
    @par            Refer
    - 参照するグローバル変数 ex_self_check
    - 参照するグローバル変数 ex_CRC16_lo
    - 参照するグローバル変数 ex_CRC16_hi
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_ex_rom_crc16(void)
{
	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 1) = 5;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_EX_ROM_CRC16;
	*(ex_operation_usb_write_buffer + 3) = (u8)(ex_rom_crc & 0xff);
	*(ex_operation_usb_write_buffer + 4) = (u8)((ex_rom_crc >> 8) & 0xff);
	ex_operation_usb_write_size = 5;				/* send  15Byte			*/
}

/******************************************************************************/
/*! @brief Operation USB Suite make boot rom version command process
    @par            Refer
    - 参照するグローバル変数 boot_ver
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_boot_rom_version(void)
{
	u8	*boot;

	boot = (u8 *)0x0013FFF0;/* __section_begin("_BOOT_VER") */
	boot += 6;

	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 1) = 0x03 + 4;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_BOOT_ROM_VERSION;
	*(ex_operation_usb_write_buffer + 3) = 'B';
	*(ex_operation_usb_write_buffer + 4) = *boot;
	*(ex_operation_usb_write_buffer + 5) = *(boot + 1);
	*(ex_operation_usb_write_buffer + 6) = *(boot + 2);
	ex_operation_usb_write_size = 7;				/* send 5 Byte			*/
}


/******************************************************************************/
/*! @brief Operation USB Suite make external rom version command process
    @par            Refer
    - 参照するグローバル変数 software_ver
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_ex_rom_version(void)
{
	// not supported
}

/******************************************************************************/
/*! @brief Operation USB Suite make set current mode command process
    @par            Refer
    - 参照するグローバル変数 ex_system
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_cur_mode(void)
{
	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 1) = 0x04;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_CUR_MODE;
	*(ex_operation_usb_write_buffer + 3) = ex_subline_suite_item.curent_service;
	ex_operation_usb_write_size = 4;
}


/******************************************************************************/
/*! @brief Operation USB Suite make mode list command process
    @par            Refer
    - 参照するグローバル変数 ex_subline_suite_item
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_mode_list(void)
{
	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 1) = 0x05;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_MODE_LIST;
	*(ex_operation_usb_write_buffer + 3) = (u8)(ex_subline_suite_item.service_list & 0xff);
	*(ex_operation_usb_write_buffer + 4) = (u8)(ex_subline_suite_item.service_list >> 8);
	ex_operation_usb_write_size = 5;
}


/******************************************************************************/
/*! @brief Operation USB Suite make mode list command & change current mode process
    @par            Refer
    - 参照するグローバル変数 ex_operation_usb
    @par            Modify
    - 参照するグローバル変数 ex_subline_suite_item
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_chang_mode(void)
{
	switch(ex_operation_usb.pc.mess.phase)
	{
	case SUITE_ITEM_DOWNLOAD:
	/*<		check enable Item 	>*/
		if((ex_subline_suite_item.service_list & BIT_DOWNLOAD_SERVICE) != 0)
		{
			ex_subline_suite_item.curent_service = SUITE_ITEM_DOWNLOAD;
		}
		*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_operation_usb_write_buffer + 1) = 0x04;
		*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_CHANG_MODE;
		*(ex_operation_usb_write_buffer + 3) = ex_subline_suite_item.curent_service;
		ex_operation_usb_write_size = 4;
		break;
	/*<		check enable Item 	>*/
	case SUITE_ITEM_COMPLEATE:
		*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_operation_usb_write_buffer + 1) = 0x04;
		*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_CHANG_MODE;
		*(ex_operation_usb_write_buffer + 3) = SUITE_ITEM_COMPLEATE;
		ex_operation_usb_write_size = 4;
		ex_operation_usb.pc.mess.serviceID = 0;
		_subline_set_mode(SUBLINE_MODE_DOWNLOAD_WAIT_RESET);
		break;
	default:
		break;
	}
}

/******************************************************************************/
/*! @brief Operation USB Suite make product serial number command process
    @par            Refer
    - 参照するグローバル変数 ex_manufacture_info
    - 参照するグローバル変数 ex_adjustment_info
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_serial_number(void)
{
	// not supported
}


/******************************************************************************/
/*! @brief Operation USB Suite make rom status command process
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_rom_status(void)
{
	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 1) = 0x04;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_ROM_STATUS;
	*(ex_operation_usb_write_buffer + 3) = 0x01; /* Status NG 	*/
	ex_operation_usb_write_size = 0x04;				/* send  4Byte			*/
}


/******************************************************************************/
/*! @brief Operation USB Suite make protocol ID command process
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_protocol_ID(void)
{
#if defined(_PROTOCOL_ENABLE_ID003)
	ex_smrt_id = 3;
#elif defined(_PROTOCOL_ENABLE_ID0G8)
	ex_smrt_id = 240;
#else
	ex_smrt_id = 3;
#endif

	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 1) = 0x04;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_PROTOCOL_ID;
	*(ex_operation_usb_write_buffer + 3) = ex_smrt_id;
	ex_operation_usb_write_size = 0x04;				/* send  4Byte			*/
}

/******************************************************************************/
/*! @brief Operation USB Suite make external Main source version command process
    @par            Refer
    - 参照するグローバル変数 ex_main_MA_ver
    @par            Modify
    - 変更するグローバル変数 ex_operation_usb_write_buffer[]
    - 変更するグローバル変数 ex_operation_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	subline_usb_suite_ex_main_version(void)
{
	int	length = 3;
	int	ii = 0;

	*ex_operation_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_operation_usb_write_buffer + 2) = OUSB_SUITE_MAIN_SOURCE_VERSION;
	for (ii = 0; ii < 16; ii++)
	{
		*(ex_operation_usb_write_buffer + length) = _boot_if_version[ii];
		length += 1;
	}
	*(ex_operation_usb_write_buffer + 1) = (u8)length;
	ex_operation_usb_write_size = (u8)length;				/* send  Byte			*/
}


/* EOF */
