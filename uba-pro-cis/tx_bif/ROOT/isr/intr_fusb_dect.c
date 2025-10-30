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
 * @file int_system_timer.c
 * @brief
 * @date 2018.01.05 Created.
 */
/****************************************************************************/
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal_gpio.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/**** VARIABLE DEFINES ******************************************************/

/*********************************************************************//**
 * @brief		Front USB Connect detect proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _intr_fusb_dect(void *arg)
{
	iset_flg(ID_FUSB_DET_FLAG, EVT_FUSB_DECT_INTR);
	clear_intr_fusb_dect();
}
/* End of File */
