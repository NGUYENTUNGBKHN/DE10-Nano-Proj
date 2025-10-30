/*
 * memorymap.h
 *
 *  Created on: 2018/01/31
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_MEMORYMAP_H_
#define SRC_MEMORYMAP_H_


/*-----------------------------------
*			Memory MAP
*-----------------------------------*/
#define DDR_START_ADDRESS				0x00100000ul
#define DDR_END_ADDRESS					0x080FFFFFul
#define BOOT_FUNC_BASE					0x00010000ul
#define PROGRAM_START_ADRESS			0x00180000ul
#define PROGRAM_END_ADRESS				0x010FFFFDul
//#define PROGRAM_END_ADRESS				0x020FFFFDul
#define ROM_ALIAS_START_ADDRESS			0x00180000ul
#define ROM_ALIAS_END_ADDRESS			0x010FFFFFul
//#define ROM_ALIAS_END_ADDRESS				0x020FFFFFul
#define BIF_SECTION_BASE				0x00180100ul
#define BIF1_SECTION_BASE				0x00180200ul
#define BIF2_SECTION_BASE				0x00200000ul
#define IF_SECTION_BASE					0x00280000ul
#define COUNTRY_SECTION_BASE			0x00410000ul
#define MAIN_DATA_BASE	 				0x02100000ul
#define MAIN_BSS_BASE	 				0x02200000ul
#define USB_CDC_BUF_BASE        		0x03000000ul
#define SDMMC_BUF_BASE 		       		0x03040000ul
#define SDMMC_FILE_BASE		       		0x03080000ul
#define ADJDAT_ADDR_BASE 				0x03100000ul
#define ROWDAT_ADDR_BASE 				0x03200000ul
#define BILL_NOTE_IMAGE_TOP_BASE		0x03400000ul
#define FPGA_LOG_BASE	 				0x080C0000ul

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

#define bif_if_download 			FUNC((BIF_SECTION_BASE + 0x002C))	/* (002C) BIF I/F DOWNLOAD ENTRY ADDR. 		*/
#define bif_device_usb_download 	FUNC((BIF_SECTION_BASE + 0x0030))	/* (0030) BIF DEVICE USB DOWNLOAD ENTRY ADDR. 		*/
#define bif_host_usb_download 		FUNC((BIF_SECTION_BASE + 0x0034))	/* (0034) BIF HOST USB DOWNLOAD ENTRY ADDR. 		*/
#define bif_subline_usb_download 	FUNC((BIF_SECTION_BASE + 0x0038))	/* (0038) BIF SUBLINE USB DOWNLOAD ENTRY ADDR. 		*/
#define bif_if_diff_download 		FUNC((BIF_SECTION_BASE + 0x003C))	/* (003C) BIF I/F DIFFERENTIAL DOWNLOAD ENTRY ADDR.	*/

// MPUメインメモリのアドレス配置
// BOOT_ROM:0x0～0x10000
#define BOOT_ROM_BASE (0x0)
#define BOOT_ROM_SIZE (0x10000)
#define BOOT_ROM_END (BOOT_ROM_BASE + BOOT_ROM_END)

#define CIS_RAM_BASE	0x05000000
#define CIS_RAM_END	(0x05000000 + 0x06000000 + 0x01000000)

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

#if defined(_PROTOCOL_ENABLE_ID0G8)
#define QSPI_REPLACE_BUF              	0x08000000ul
#define QSPI_REPLACE_SIZE              	0x00100000ul
#endif

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

#define ADJDAT ((int *)ADJDAT_ADDR_BASE)
#define ROWDAT ((int *)ROWDAT_ADDR_BASE)
#define IMAGEDAT ((int *)BILL_NOTE_IMAGE_TOP_BASE)




#endif /* SRC_MEMORYMAP_H_ */
