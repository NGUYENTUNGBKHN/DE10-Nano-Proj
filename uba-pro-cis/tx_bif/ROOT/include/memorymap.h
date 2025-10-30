/*
 * memorymap.h
 *
 *  Created on: 2018/01/31
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_MEMORYMAP_H_
#define SRC_MEMORYMAP_H_

//#include "typedefine.h"

/*-----------------------------------
*			Memory MAP
*-----------------------------------*/
#define DDR_START_ADDRESS				0x00100000ul
#define DDR_END_ADDRESS					0x080FFFFFul
#define BOOT_FUNC_BASE					0x00100000ul
#define PROGRAM_START_ADRESS			0x00180000ul
#define PROGRAM_END_ADRESS				0x020FFFFDul
#define ROM_ALIAS_START_ADDRESS			0x00180000ul
#define ROM_ALIAS_END_ADDRESS			0x010FFFFFul
#define MAIN_END_ADDRESS	 			0x010FFFFFul
#define BIF_SECTION_BASE				0x00180100ul
#define BIF1_SECTION_BASE				0x00180200ul
#define BIF2_SECTION_BASE				0x00200000ul
#define IF_SECTION_BASE					0x00280000ul
#define MAIN_DATA_BASE	 				0x02100000ul
#define MAIN_BSS_BASE	 				0x02200000ul
#define USB_CDC_BUF_BASE        		0x03000000ul
#define DOWNLOAD_VERIFY_BUF             0x04000000ul
#define DOWNLOAD_IMAGE_BUF              0x06000000ul
#define QSPI_REPLACE_BUF              	0x08000000ul
#define QSPI_REPLACE_SIZE              	0x00100000ul

#define _DOWNLOAD_FILE_SIZE				(0x00f80000ul)
#define _WRITE_DATA_BUF_SIZE            (0x00080000ul)
#define _READ_DATA_BUF_SIZE             (_DOWNLOAD_FILE_SIZE)
#define WRITE_BUF_ADDR                  (0x03000000ul)
#define READ_BUF_ADDR                   (0x03800000ul)
#if 0
#define BIF_SECTION_BASE				0x00030000ul
#define BIF1_SECTION_BASE				0x00030200ul
#define BIF2_SECTION_BASE				0x000A0100ul
#define IF_SECTION_BASE					0x00110000ul
#define COUNTRY_SECTION_BASE			0x00410000ul
#define USB_CDC_BUF_BASE        		0x01800000ul
#define USB_CDC_BUF_END           		0x018FFFFFul
#define DOWNLOAD_DATA_BUF_BASE          0x01900000ul
#define DOWNLOAD_DATA_BUF_END           0x019FFFFFul
#define USB_RX_BUF                      0x01A00000ul
#define USB_TX_BUF                      0x01A10000ul
#define DOWNLOAD_RX_BUF                 0x01A20000ul
#define USB_FILE_BUF 	                0x01A40000ul
#define BIF_TLB_ADDR					0x04000000ul
#define Boot_ver		(*(u8 (*)[4])0x0013FFF0)
#endif

/*-------------------------------*/
/* section header*               */
/*-------------------------------*/
#define D_ENTRY_TBL_OFFSET_CRC32      0x08
#define D_ENTRY_TBL_OFFSET_CRC16      0x0C
#define D_ENTRY_TBL_OFFSET_SECTION_NO 0x0E
#define D_ENTRY_TBL_OFFSET_STARTADR   0x10
#define D_ENTRY_TBL_OFFSET_ENDADR     0x14
#define D_ENTRY_TBL_OFFSET_SEC_NAME   0x1C

/*------------------------------------*/
/*			BOOT entry define		  */
/*------------------------------------*/
#define _bootif_start FUNC((BIF_SECTION_BASE + 0x0028))	/*	BOOT I/F START ADDR.			*/
#define _if_start     FUNC((IF_SECTION_BASE + 0x0028))	/*	I/F START ADDR.			*/

// JTAGデバッグ用
#define ICE_MODE	(((*(u8 *)ROM_ALIAS_END_ADDRESS) == 0x00) && ((*(u8 *)(ROM_ALIAS_END_ADDRESS - 1)) == 0x00) ? 1 : 0 )
//#define ICE_MODE	((*(u32 *)(ADR_QSPI_FROM_END - 3) == 0x00000000) && (*(u32 *)(ADR_QSPI_FROM_END - 7) == 0xFFFFFFFF) ? 1 : 0 )


// MPUメインメモリのアドレス配置
// BOOT_ROM:0x0～0x10000
#define BOOT_ROM_BASE (0x0)
#define BOOT_ROM_SIZE (0x10000)
#define BOOT_ROM_END (BOOT_ROM_BASE + BOOT_ROM_END)

// 未使用エリア:0x10000～0x100000

// SDRAM:0x100000～0x80000000
#define SDRAM_BASE (0x100000)
#define SDRAM_SIZE (0x80000000 - 0x100000)
#define PROGRAM_BASE SDRAM_BASE
#define PROGRAM_SIZE (0x100000)
#define TEMPLATE_BASE (PROGRAM_BASE + PROGRAM_SIZE)
#define TEMPLATE_SIZE (0x800000)
#define IMAGEDATA_BASE (TEMPLATE_BASE + TEMPLATE_SIZE)
#define IMAGEDATA_SIZE (0x6F600000)
#define DATA_BASE (IMAGEDATA_BASE + IMAGEDATA_SIZE)
#define DATA_SIZE (0x10000000)
#define SDRAM_END (SDRAM_BASE + SDRAM_SIZE)

// 未使用エリア:0x80000000～0xC0000000
#define NO_MEM_BASE (0x80000000)
#define NO_MEM_SIZE (0x40000000)
#define NO_MEM_END (NO_MEM_BASE + NO_MEM_SIZE)

// 内蔵モジュールエリア:0xC0000000～0x100000000
#define PERIPHERAL_BASE (0xC0000000)
#define PERIPHERAL_SIZE (0x40000000)
#define OC_RAM_BASE (0xFFFF0000)
#define OC_RAM_SIZE (0x10000)
#define PERIPHERAL_END (PERIPHERAL_BASE + PERIPHERAL_SIZE)
#endif /* SRC_MEMORYMAP_H_ */
