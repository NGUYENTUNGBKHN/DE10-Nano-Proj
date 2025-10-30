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
 * @file sysmgr_setup.c
 * @brief SYSMGRセットアップ関数
 * @date 2018.06.08 Created
 */
/****************************************************************************/

#include "systemdef.h"
#include "cyclonev_sysmgr_reg_def.h"
#include "hal_gpio_reg.h"
#include "js_oswapi.h"
#include "js_io.h"
#include "js_gpio.h"
#include "sub_functions.h"

/****************************************************************/
/**
 * @brief SYSMGRレジスタセットアップ
 */
/****************************************************************/
void setup_sysmgr(void)
{
	// GPIO60:SPIM0_SS0
	IOREG32(SYSMGR_BASE,SYS_GENERALIO12) = (UINT32)0x0;
}

/*******************************
    UART1<-->GPIO
 *******************************/
void uart1_halt(u8 set)
{
	if(set)
	{
		// FUNCTION:UART1-TXD→GPIO64
		IOREG32(SYSMGR_BASE,SYS_GENERALIO16) = (UINT32)0x0;
		if( Gpio_mode(GPIO_64, GPIO_MODE_OUTPUT) == FALSE ){
			osw_printf( "Gpio_mode() - Error\n" );
			/* system error */
			program_error();
			return;
		};
		Gpio_out(GPIO_64,1);
	}
	else
	{
#if 0
		Gpio_out(GPIO_64,0);
		// FUNCTION:UART1-RXD
		IOREG32(SYSMGR_BASE,SYS_GENERALIO15) = (UINT32)0x2;
#endif
		// FUNCTION:UART1-TXD
		IOREG32(SYSMGR_BASE,SYS_GENERALIO16) = (UINT32)0x2;
	}
};

/* End of file */

