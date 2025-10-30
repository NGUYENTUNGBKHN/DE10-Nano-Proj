/******************************************************************************/
/*! @addtogroup Main
    @file       dline_suite.c
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
#include "hal.h"
#include "dline_suite.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"

/************************** PRIVATE DEFINITIONS ******************************/


/************************** EXTERNAL VARIABLES *******************************/
extern const u8 ex_main_MA_ver[16];
extern const u8 software_ver[];

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/*****************************************************************************/
/*! @brief initialize jcm suite variables
    @par            Refer
    - 参照するグローバル変数 none
    @par            Modify
    - 変更するグローバル変数 ex_suite_item
    @return         none
    @exception      none
******************************************************************************/
void front_usb_suite_initial(void)
{
	ex_suite_item.curent_service = SUITE_ITEM_DOWNLOAD;
	ex_suite_item.service_list = BIT_SUITE_ITEM_LIST;
	#if defined(USB_REAR_USE)
	ex_suite_item_rear.curent_service = SUITE_ITEM_DOWNLOAD;
	ex_suite_item_rear.service_list = BIT_SUITE_ITEM_LIST;
	#endif 

	if(is_test_mode())		/* テストモード */
	{
		if((ex_dipsw1 & 0x7F) != 0)	/* DipSW1 8以外がON	*///ok
		{
			ex_suite_item.service_list = ex_suite_item.service_list ^ BIT_ADJUSTMENT_SERVICE;	/* 調整ツール非表示				*/
			ex_suite_item.service_list = ex_suite_item.service_list ^ BIT_TESTMODE_SERVICE;		/* Performance Testツール非表示 */
			#if defined(USB_REAR_USE)
			ex_suite_item_rear.service_list = ex_suite_item_rear.service_list ^ BIT_ADJUSTMENT_SERVICE;	/* 調整ツール非表示				*/
			ex_suite_item_rear.service_list = ex_suite_item_rear.service_list ^ BIT_TESTMODE_SERVICE;		/* Performance Testツール非表示 */
			ex_suite_item_rear.service_list = ex_suite_item_rear.service_list ^ BIT_UTILITY_SERVICE;		/* Performance Testツール非表示 */
			#endif // USB_REAR_USE
		}
	}
	else							/* DipSW1 All OFF	*/
	{
		ex_suite_item.service_list = ex_suite_item.service_list ^ BIT_ADJUSTMENT_SERVICE;	/* 調整ツール非表示				*/
		ex_suite_item.service_list = ex_suite_item.service_list ^ BIT_TESTMODE_SERVICE;		/* Performance Testツール非表示 */
		#if defined(USB_REAR_USE)
		ex_suite_item_rear.service_list = ex_suite_item_rear.service_list ^ BIT_ADJUSTMENT_SERVICE;	/* 調整ツール非表示				*/
		ex_suite_item_rear.service_list = ex_suite_item_rear.service_list ^ BIT_TESTMODE_SERVICE;		/* Performance Testツール非表示 */
		ex_suite_item_rear.service_list = ex_suite_item_rear.service_list ^ BIT_UTILITY_SERVICE;		/* Performance Testツール非表示 */
		#endif // USB_REAR_USE
	}
	ex_suite_item.curent_service = 6;
}

/******************************************************************************/
/*! @brief Front USB Suite make product id command process
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_product_id(void)
{
	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 0x05;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_PRODUCT_ID;
	*(ex_usb_write_buffer + 3) = (u8)(JCM_PRODUCT_ID >> 8);
	*(ex_usb_write_buffer + 4) = (u8)(JCM_PRODUCT_ID);
	ex_usb_write_size = 5;				/* send 5 Byte			*/
}

/******************************************************************************/
/*! @brief Front USB Suite make external rom CRC16 command process
    @par            Refer
    - 参照するグローバル変数 ex_self_check
    - 参照するグローバル変数 ex_CRC16_lo
    - 参照するグローバル変数 ex_CRC16_hi
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_ex_rom_crc16(void)
{
	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 5;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_EX_ROM_CRC16;
	*(ex_usb_write_buffer + 3) = (u8)(ex_rom_crc & 0xff);
	*(ex_usb_write_buffer + 4) = (u8)((ex_rom_crc >> 8) & 0xff);
	ex_usb_write_size = 5;				/* send  15Byte			*/
}

/******************************************************************************/
/*! @brief Front USB Suite make boot rom version command process
    @par            Refer
    - 参照するグローバル変数 boot_ver
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_boot_rom_version(void)
{
	u8	*boot;

	//boot = (u8 *)0x0013FFF0;/* __section_begin("_BOOT_VER") */
	boot = (u8 *)0x00140540;
	boot += 6;

	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 0x03 + 4;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_BOOT_ROM_VERSION;
	*(ex_usb_write_buffer + 3) = 'B';
	*(ex_usb_write_buffer + 4) = *boot;
	*(ex_usb_write_buffer + 5) = *(boot + 1);
	*(ex_usb_write_buffer + 6) = *(boot + 2);
	ex_usb_write_size = 7;				/* send 5 Byte			*/
}

/******************************************************************************/
/*! @brief Front USB Suite make external rom version command process
    @par            Refer
    - 参照するグローバル変数 software_ver
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_ex_rom_version(void)
{
	int	length = 3;
	int	ii = 0;

	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_EX_ROM_VERSION;
	while(software_ver[ii] != '\0')			/*	'10-06-23	*/
	{
		*(ex_usb_write_buffer + length) = software_ver[ii];
		ii += 1;
		length += 1;
	}
	*(ex_usb_write_buffer + 1) = (u8)length;
	ex_usb_write_size = (u8)length;				/* send  Byte			*/
}

/******************************************************************************/
/*! @brief Front USB Suite make external Main source version command process
    @par            Refer
    - 参照するグローバル変数 ex_main_MA_ver
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_ex_main_version(void)
{
	int	length = 3;
	int	ii = 0;

	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_MAIN_SOURCE_VERSION;
	for (ii = 0; ii < 16; ii++)
	{
		*(ex_usb_write_buffer + length) = ex_main_MA_ver[ii];
		length += 1;
	}
	*(ex_usb_write_buffer + 1) = (u8)length;
	ex_usb_write_size = (u8)length;				/* send  Byte			*/
}

/******************************************************************************/
/*! @brief Front USB Suite make product serial number command process
    @par            Refer
    - 参照するグローバル変数 ex_manufacture_info
    - 参照するグローバル変数 ex_adjustment_info
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_serial_number(void)
{
	u8	*ptr;
	int	ii;

	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 15;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_SERIAL_NUMBER;

	ptr = &ex_adjustment_data.maintenance_info.serial_no[0];

	for(ii = 3; ii < 12 + 3; ii++)
	{
		*(ex_usb_write_buffer + ii) = *ptr;
		ptr += 1;
	}
	ex_usb_write_size = 15;				/* send  15Byte			*/
}



/******************************************************************************/
/*! @brief Front USB Suite make set current mode command process
    @par            Refer
    - 参照するグローバル変数 ex_system
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_cur_mode(void)
{
	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 0x04;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_CUR_MODE;
	*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
	ex_usb_write_size = 4;
}


/******************************************************************************/
/*! @brief Front USB Suite make mode list command process
    @par            Refer
    - 参照するグローバル変数 ex_suite_item
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_mode_list(void)
{
	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 0x05;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_MODE_LIST;
	*(ex_usb_write_buffer + 3) = (u8)(ex_suite_item.service_list & 0xff);
	*(ex_usb_write_buffer + 4) = (u8)(ex_suite_item.service_list >> 8);
	ex_usb_write_size = 5;
}

#if defined(USB_REAR_USE)
/******************************************************************************/
/*! @brief Front USB Suite make mode list command process
    @par            Refer
    - 参照するグローバル変数 ex_suite_item
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_mode_list2(void)
{
	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 0x05;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_MODE_LIST;
	*(ex_usb_write_buffer + 3) = (u8)(ex_suite_item_rear.service_list & 0xff);
	*(ex_usb_write_buffer + 4) = (u8)(ex_suite_item_rear.service_list >> 8);
	ex_usb_write_size = 5;
}
#endif // USB_REAR_USE

/******************************************************************************/
/*! @brief Front USB Suite make rom status command process
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_rom_status(void)
{
	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 0x04;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_ROM_STATUS;
	*(ex_usb_write_buffer + 3) = 0x03; /* Status OK */
	ex_usb_write_size = 0x04;			/* send  4Byte			*/
}

/******************************************************************************/
/*! @brief Front USB Suite make protocol ID command process
    @par            Modify
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_protocol_ID(void)
{
#if defined(_PROTOCOL_ENABLE_ID003)
	ex_smrt_id = 3;
#elif defined(_PROTOCOL_ENABLE_ID0G8)
	ex_smrt_id = 240;
#else
	ex_smrt_id = 3;
#endif

	*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
	*(ex_usb_write_buffer + 1) = 0x04;
	*(ex_usb_write_buffer + 2) = FUSB_SUITE_PROTOCOL_ID;
	*(ex_usb_write_buffer + 3) = ex_smrt_id;
	ex_usb_write_size = 0x04;				/* send  4Byte			*/
}


/******************************************************************************/
/*! @brief Front USB Suite make mode list command & change current mode process
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @par            Modify
    - 参照するグローバル変数 ex_suite_item
    - 変更するグローバル変数 ex_usb_write_buffer[]
    - 変更するグローバル変数 ex_usb_write_size
    @return         none
    @exception      none
******************************************************************************/
void	front_usb_suite_chang_mode(void)
{
	switch(ex_front_usb.pc.mess.phase)
	{
	case SUITE_ITEM_DOWNLOAD:
	/*<		check enable Item 	>*/
		if((ex_suite_item.service_list & BIT_DOWNLOAD_SERVICE) != 0)
		{
			ex_suite_item.curent_service = SUITE_ITEM_DOWNLOAD;
		}
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
		ex_usb_write_size = 4;
		break;
	case SUITE_ITEM_ACCLOAD:
	/*<		check enable Item 	>*/
		if((ex_suite_item.service_list & BIT_ACCLOAD_SERVICE) != 0)
		{
			ex_suite_item.curent_service = SUITE_ITEM_ACCLOAD;
		}
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
		ex_usb_write_size = 4;
		ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_ACCLOAD;
		break;
	case SUITE_ITEM_ADJUSTMENT:
	/*<		check enable Item 	>*/
		if((ex_suite_item.service_list & BIT_ADJUSTMENT_SERVICE) != 0)
		{
			ex_suite_item.curent_service = SUITE_ITEM_ADJUSTMENT;
		}
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
		ex_usb_write_size = 4;
		ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_TESTMODE;
		break;
	case SUITE_ITEM_TESTMODE:
	/*<		check enable Item 	>*/
		if((ex_suite_item.service_list & BIT_TESTMODE_SERVICE) != 0)
		{
			ex_suite_item.curent_service = SUITE_ITEM_TESTMODE;
		}
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
		ex_usb_write_size = 4;
		ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_TESTMODE;
		break;
	case SUITE_ITEM_UTILITY:
	/*<		check enable Item 	>*/
		if((ex_suite_item.service_list & BIT_UTILITY_SERVICE) != 0)
		{
			ex_suite_item.curent_service = SUITE_ITEM_UTILITY;
		}
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
		ex_usb_write_size = 4;
		ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_UTILITY_MODE;
		break;

		case SUITE_ITEM_RECYCLER:	//0x09 //2025-04-07 //既存のTool suteやセンサ調整Toolで使用している, UBA500 RCで使用している
		/*<		check enable Item 	>*/
			if((ex_suite_item.service_list & BIT_RECYCLER_SENSOR) != 0)
			{
				ex_suite_item.curent_service = SUITE_ITEM_RECYCLER;
			}
			*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
			*(ex_usb_write_buffer + 1) = 0x04;
			*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
			*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
			ex_usb_write_size = 4;
			//ex_front_usb.pc.mess.serviceID	//UBA500も設定はしていない
			break;
	
		case SUITE_ITEM_EVENTLOG:
	/*<		check enable Item 	>*/
		if((ex_suite_item.service_list & SUITE_ITEM_EVENTLOG) != 0)
		{
			ex_suite_item.curent_service = SUITE_ITEM_EVENTLOG;
		}
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
		ex_usb_write_size = 4;
		ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_EVENTLOG;
		break;
	case SUITE_ITEM_CONDITION:
	/*<		check enable Item 	>*/
		if((ex_suite_item.service_list & SUITE_ITEM_CONDITION) != 0)
		{
			ex_suite_item.curent_service = SUITE_ITEM_CONDITION;
		}
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = ex_suite_item.curent_service;
		ex_usb_write_size = 4;
		ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_CONDITION;
		break;
	/*<		check enable Item 	>*/
	case SUITE_ITEM_COMPLEATE:
		*ex_usb_write_buffer = RES_SUITE_TYPE_ID;
		*(ex_usb_write_buffer + 1) = 0x04;
		*(ex_usb_write_buffer + 2) = FUSB_SUITE_CHANG_MODE;
		*(ex_usb_write_buffer + 3) = SUITE_ITEM_COMPLEATE;
		ex_usb_write_size = 4;
		if((ex_suite_item.service_list & BIT_TESTMODE_SERVICE) != 0)
		{
			ex_front_usb.pc.mess.serviceID = 0x03;
		}
		else
		{
			ex_front_usb.pc.mess.serviceID = 0;
		}
		break;
	default:
		break;
	}
}


/* EOF */
