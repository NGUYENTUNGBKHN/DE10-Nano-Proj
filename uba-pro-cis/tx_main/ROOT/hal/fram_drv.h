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
 * @file cyclonev_fram_drv.h
 * @brief FRAMドライバ ヘッダファイル
 * @date 2019.03.05 Created.
 */
/****************************************************************************/
#ifndef FRAM_DRV_H
#define FRAM_DRV_H

#include "jcm_typedef.h"
#include "hal_gpio_reg.h"		// GPIO_60を定義, 18/09/13

enum {
	FRAM_CH = 0,		// FRAMのSPIチャネル番号
};

/****************************************************************/
/*						エラーコード							*/
/****************************************************************/
typedef enum {
	FRAM_DRV_SUCCESS = (0),
	FRAM_DRV_WRITE_ERROR = (-1),
	FRAM_DRV_READ_ERROR = (-2),
	FRAM_DRV_WREN_ERROR = (-3),
	FRAM_DRV_OPEN_ERROR = (-4),
	FRAM_DRV_INIT_ERROR = (-5),
} FRAM_DRV_ER;

#define FRAM_CS			(GPIO_60)			/* FRAM CS */

#define MRAM_SIZE_DOWNLOADMODE		1

#define MRAM_DOWNLOADMODE_TOP		1023


/****************************************************************/
/*						関数定義								*/
/****************************************************************/
FRAM_DRV_ER fram_drv_init(void);		// SPIドライバ初期化, 18/12/04
FRAM_DRV_ER fram_drv_open(s32 ch);
FRAM_DRV_ER fram_drv_read(u32 address, u8* pread_buff, u32 len);
FRAM_DRV_ER fram_drv_write(u32 address, u8* pwrite_buff, u32 len);
FRAM_DRV_ER fram_drv_read_status(u8 *pdata);
FRAM_DRV_ER fram_drv_write_status(u8 data);
FRAM_DRV_ER fram_drv_write_enable(void);
void fram_drv_close(void);

#endif /* FRAM_DRV_H */
