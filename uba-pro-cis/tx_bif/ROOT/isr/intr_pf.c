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
 * @file intr_pf.c
 * @brief
 * @date 2021.06.21 Created.
 */
/****************************************************************************/
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/**** VARIABLE DEFINES ******************************************************/

/*********************************************************************//**
 * @brief		Power fail detect proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _intr_pf(void *arg)
{
	iset_flg(ID_FRAM_CTRL_FLAG, EVT_FRAM_VOLTAGE);
	GpioIsr_clear(__HAL_PF);
}

/*********************************************************************//**
 * @brief		Initialize Power fail detect proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_pf(void)
{
	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	gpio_iprm.attr = GPIO_ATTR_LOW_EDGE;
	gpio_iprm.cb_isr_func = _intr_pf;
	gpio_iprm.cb_isr_arg = NULL;
	GpioIsr_open(__HAL_PF, &gpio_iprm);

	GpioIsr_enable(__HAL_PF);
}
/* End of File */
