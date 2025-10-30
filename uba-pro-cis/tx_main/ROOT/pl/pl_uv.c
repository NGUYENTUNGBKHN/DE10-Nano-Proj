/******************************************************************************/
/*! @addtogroup Main
    @file       pl_mag.c
    @date       2022/04/07
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2022 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * pl_uv.c
 * UVセンサードライバファイル。
 *  Created on: 2022/04/07
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
//#include "custom.h"

#include "pl_gpio.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#include "pl_uv.h"

/************************** Private Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/*********************************************************************//**
 * @brief		check uv sensor
 * @return 		1: Normal
 * @return		0: Abnormal
 **********************************************************************/
u32 _pl_uv0_check(void)
{
	UV_CHK_UNION uv_chk;

	uv_chk.LWORD = FPGA_REG.UV_CHK.LWORD;

	return uv_chk.BIT.CHK0;
}

/*********************************************************************//**
 * @brief		check uv sensor
 * @return 		1: Normal
 * @return		0: Abnormal
 **********************************************************************/
u32 _pl_uv1_check(void)
{

	UV_CHK_UNION uv_chk;

	uv_chk.LWORD = FPGA_REG.UV_CHK.LWORD;

	return uv_chk.BIT.CHK1;
}
/* EOF */
