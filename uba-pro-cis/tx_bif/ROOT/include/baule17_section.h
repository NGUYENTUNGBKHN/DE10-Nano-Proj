/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
#ifndef BAULE17_SECTION_H
#define BAULE17_SECTION_H


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


#endif		// BAULE17_SECTION_H
