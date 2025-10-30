/******************************************************************************/
/*! @addtogroup Main
    @file       subline_suite.c
    @brief      jcm usb suite header file
    @date       2018/03/19
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/19 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

/*<<	Suite command		>>*/
#define	CMD_SUITE_TYPE_ID				0xAA
#define	RES_SUITE_TYPE_ID				0xBB
#define	OUSB_SUITE_CUR_MODE				0x00
#define	OUSB_SUITE_MODE_LIST			0x01
#define	OUSB_SUITE_CHANG_MODE			0x02
#define	OUSB_SUITE_PRODUCT_ID			0x03
#define	OUSB_SUITE_BOOT_ROM_VERSION		0x04
#define	OUSB_SUITE_ROM_STATUS			0x05
#define	OUSB_SUITE_SERIAL_NUMBER		0x06
#define	OUSB_SUITE_EX_ROM_VERSION		0x07
#define	OUSB_SUITE_EX_ROM_CRC16			0x08
#define	OUSB_SUITE_PROTOCOL_ID			0x09
#define	OUSB_SUITE_MAIN_SOURCE_VERSION	0x0A

/*<<	Suite Mode ID				>>*/
#define	OUSB_SUITE_PHASE_DOWNLOAD		0x00

/*<<	JCM Service mode Item		>>*/
#define	SUITE_ITEM_DOWNLOAD				0x00
#define	SUITE_ITEM_ACCLOAD				0x01
#define	SUITE_ITEM_ADJUSTMENT			0x02
/*										0x03	*/
/*										0x04	*/
/*										0x05	*/
/*										0x06	*/
#define	SUITE_ITEM_TESTMODE				0x07
#define	SUITE_ITEM_UTILITY				0x08
#define	SUITE_ITEM_EVENTLOG				0x0A
#define SUITE_ITEM_CONDITION			0x0B
#define SUITE_ITEM_COMPLEATE			0xff

#define	BIT_DOWNLOAD_SERVICE			0x0001
#define	BIT_ACCLOAD_SERVICE				0x0002
#define	BIT_ADJUSTMENT_SERVICE			0x0004
/*#define	BIT_MONITOR_SERVICE			0x0008	*/
/*#define	BIT_COLLECTION_SERVICE		0x0010	*/
/*#define								0x0020	*/
/*#define								0x0040	*/
#define	BIT_TESTMODE_SERVICE			0x0080
#define	BIT_UTILITY_SERVICE				0x0100
#define	BIT_EVENT_LOG_SERVICE			0x0800

#define	BIT_SUITE_ITEM_LIST_DEBUG		( BIT_DOWNLOAD_SERVICE )
#define	BIT_SUITE_ITEM_LIST				( BIT_DOWNLOAD_SERVICE )

extern void subline_usb_suite_initial(void);
extern void subline_usb_suite_cur_mode(void);
extern void subline_usb_suite_mode_list(void);
extern void subline_usb_suite_chang_mode(void);
extern void subline_usb_suite_product_id(void);
extern void subline_usb_suite_boot_rom_version(void);
extern void subline_usb_suite_ex_rom_version(void);
extern void subline_usb_suite_ex_rom_crc16(void);
extern void subline_usb_suite_serial_number(void);
extern void subline_usb_suite_rom_status(void);
extern void subline_usb_suite_protocol_ID(void);
extern void subline_usb_suite_ex_main_version(void);

/* EOF */
