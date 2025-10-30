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
 * @file int_ihr_proc.c
 * @brief ハードウェア割り込みの関数を格納しています。
 * @date 2018.01.05
 */
/****************************************************************************/
#include "systemdef.h"
#include "kernel_config.h"
#include "isr_entry_proc.h"


struct {
	uint32_t note;
	uint32_t cis;
	uint32_t sps;
	uint32_t abort;
} isr_data;

void isr_entry_init(void)
{
	memset(&isr_data, 0, sizeof(isr_data));
}

/* End of file */

