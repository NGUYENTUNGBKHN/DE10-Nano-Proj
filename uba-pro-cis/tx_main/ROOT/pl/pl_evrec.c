/******************************************************************************/
/*! @addtogroup Main
    @file       pl_evrec.c
    @date       2021/07/14
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * pl_evrec.c
 * エンコーダーON/OFF制御ドライバファイル。
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Private Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/


extern const void * Image$$FPGA_LOG$$Base;
/*********************************************************************//**
 * @brief		Evrec initialize
 * @param[in]	set
 * @return 		None
 **********************************************************************/
void _pl_evrec_init(void)
{
	//debug
	unsigned int size;
	void *addr;
	addr = &Image$$FPGA_LOG$$Base;
	size = FPGA_LOG_SIZE;
	/* clear FPGA_LOG section */
	memset( addr, 0xFF, size);
	// イベント記録 下限アドレス
	FPGA_REG.EVREC_LL.LWORD = (u32)(((u32)addr)/8);
	// イベント記録 上限アドレス
	FPGA_REG.EVREC_UL.LWORD = (u32)((((u32)addr) + size)/8);
	// イベント記録 アドレス
	FPGA_REG.EVREC_DST.BIT.DST = (u32)(((u32)addr)/8);
	// イベント記録設定
	FPGA_REG.EVREC_S0.LWORD = 0x00000000;

	ex_fpga_event_log_address = FPGA_REG.EVREC_DST.BIT.DST * 8;
}
/*********************************************************************//**
 * @brief		Start Event Record
 * @param[in]	u8 cyc :1 cyclic
 * @return 		None
 **********************************************************************/
void _pl_evrec_start(u8 cyc)
{
	EVREC_S0_UNION evrec_s0;

	evrec_s0.LWORD = 0;
	// イベント記録設定
	evrec_s0.BIT.EN = 1;
	evrec_s0.BIT.CYC = cyc;

	FPGA_REG.EVREC_S0.LWORD = evrec_s0.LWORD;
}
/*********************************************************************//**
 * @brief		Stop Event Record
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_evrec_stop(void)
{
	ex_fpga_event_log_address = FPGA_REG.EVREC_DST.BIT.DST * 8;
	// イベント記録設定
	FPGA_REG.EVREC_S0.LWORD = 0x00000000;
}
/* EOF */
