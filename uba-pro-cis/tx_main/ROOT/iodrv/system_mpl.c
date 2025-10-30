/****************************************************************************/
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
 * @file system_mpl.c
 * @brief システムメモリプール関数
 * @date 2018.06.06 Created.
 */
/****************************************************************************/
#include <kernel.h>		// kernel.h内でEXTERN宣言を操作するので、EXTERNの前でincludeする, 18/09/14
#define EXTERN extern

#include <stdint.h>
#include "system_mpl.h"
#include "kernel_inc.h"
#include "kernel_config.h"

/****************************************************************/
/**
 * @brief システムメモリプールよりメモリを取得
 */
/****************************************************************/
void* get_system_mpl(uint32_t size)
{
	ER err;
	void* p;
	
	err = get_mpl(ID_SYSTEM_MPL,  size , (VP*)&p);
	if (err != E_OK)
	{
		p = NULL;
	}
	
	return p;
}

/****************************************************************/
/**
 * @brief システムメモリプールを解放
 */
/****************************************************************/
void release_system_mpl(void* p)
{
	ER err;
	
	if (system_mpl(p)) {		// デバッグ用, 18/08/16
		err = rel_mpl(ID_SYSTEM_MPL, p);
		if (err != E_OK) {
			err = E_OK;
		}
	}
}


/* End of file */

