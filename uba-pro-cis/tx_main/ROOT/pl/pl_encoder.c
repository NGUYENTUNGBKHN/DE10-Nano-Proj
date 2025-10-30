/******************************************************************************/
/*! @addtogroup Main
    @file       pl_encoder.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * pl_encoder.c
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
//#include "custom.h"

#include "pl_cis.h"
#include "pl_gpio.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Private Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/


/*********************************************************************//**
 * @brief		Set encoder sensor LED (feed encoder)
 * @param[in]	set
 * @return 		None
 **********************************************************************/
void _pl_sen_feed_encoder_LED_set(int set)
{
	SNS_CTL_UNION sns_ctl;

	sns_ctl.LWORD = FPGA_REG.SNS_CTL.LWORD;
	if(set)
	{
		// FEED & ADPENC LED ON
		sns_ctl.BIT.ENC_ON = 1;
	}
	else
	{
		// FEED & ADPENC LED OFF
		sns_ctl.BIT.ENC_ON = 0;
	}
	FPGA_REG.SNS_CTL.LWORD = sns_ctl.LWORD;
}
/*********************************************************************//**
 * @brief		Get encoder count
 * @param[in]	set
 * @return 		None
 **********************************************************************/
u32 _pl_cis_encoder_count(void)
{
	ENC_CNT_UNION enc_cnt;

	enc_cnt.LWORD = FPGA_REG.ENC_CNT.LWORD;

	return enc_cnt.BIT.CNT;
}
/* EOF */
