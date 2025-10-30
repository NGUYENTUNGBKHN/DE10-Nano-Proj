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
/****************************************************************************/
/**
 * @file baule17_gpio_def.h
 * @brief BAU-LE17 SYSTEMレジスタ定義
 * @date 2018.03.05 Created
 */
/****************************************************************************/
#if !defined(__CYCLONE_V_RSTMGR_REG_DEF_H_INCLUDED__)
#define __CYCLONE_V_RSTMGR_REG_DEF_H_INCLUDED__

#include "reg_cyclone5.h"

#define PERMODRST_REG_ADDRESS  		(0xFFD05014) /* Peripheral Module Reset Register */
#define BRGMODRST_REG_ADDRESS  		(0xFFD0501C) /* Bridge Module Reset Register */
#define MISCMODRST_REG_ADDRESS  	(0xFFD05020) /* Miscellaneous Module Reset Register */

/****************************************************************/
/**
 * @brief PERMODRST
 */
/****************************************************************/
union PERMODRST_REG_UNION
{
	u32 _DWORD;
	struct {
		u32 emac0:1;
		u32 emac1:1;
		u32 usb0:1;
		u32 usb1:1;
		u32 nand:1;
		u32 qspi:1;
		u32 l4wd0:1;
		u32 l4wd1:1;
		u32 osc1timer0:1;
		u32 osc1timer1:1;
		u32 sptimer0:1;
		u32 sptimer1:1;
		u32 i2c0:1;
		u32 i2c1:1;
		u32 i2c2:1;
		u32 i2c3:1;
		u32 uart0:1;
		u32 uart1:1;
		u32 spim0:1;
		u32 spim1:1;
		u32 spis0:1;
		u32 spis1:1;
		u32 sdmmc:1;
		u32 can0:1;
		u32 can1:1;
		u32 gpio0:1;
		u32 gpio1:1;
		u32 gpio2:1;
		u32 dma:1;
		u32 sdr:1;
		u32 :2;
	} BIT;
};

#define PERMODRST_REG  (*(volatile union PERMODRST_REG_UNION*)(PERMODRST_REG_ADDRESS))

/****************************************************************/
/**
 * @brief BRGMODRST
 */
/****************************************************************/
typedef union _BRGMODRST_REG_UNION
{
	u32 lword;
	struct {
		u32 hps2fpga:1;
		u32 lwhps2fpga:1;
		u32 fpga2hps:1;
		u32 :29;
	} bit;
} BRGMODRST_REG_UNION;

#define BRGMODRST_REG  (*(volatile BRGMODRST_REG_UNION*)(BRGMODRST_REG_ADDRESS))

/****************************************************************/
/**
 * @brief MISCMODRST
 */
/****************************************************************/
typedef union _MISCMODRST_REG_UNION
{
	u32 lword;
	struct {
		u32 rom:1;
		u32 ocram:1;
		u32 sysmgr:1;
		u32 sysmgrcold:1;
		u32 fpgamgr:1;
		u32 acpidmap:1;
		u32 s2f:1;
		u32 s2fcold:1;
		u32 nrstpin:1;
		u32 timestampcold:1;
		u32 clkmgrcold:1;
		u32 scanmgr:1;
		u32 frzctrlcold:1;
		u32 sysdbg:1;
		u32 dbg:1;
		u32 tapcold:1;
		u32 sdrcold:1;
		u32 :15;
	} bit;
} MISCMODRST_REG_UNION;

#define MISCMODRST_REG  (*(volatile MISCMODRST_REG_UNION*)(MISCMODRST_REG_ADDRESS))

#endif /* __CYCLONE_V_RSTMGR_REG_DEF_H_INCLUDED__ */

/* End of file */

