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
#ifndef SDMMC_DRV_H
#define SDMMC_DRV_H

//#include <stdbool.h>


typedef enum {
	SD_INIT = 0,
	SD_EJECT,
	SD_INSERT,
	SD_MOUNT,
	SD_OPEN,
	SD_WRITE,
	SD_UMOUNT,
} SD_MODE_T;


bool sdmmc_init(void);
void sdmmc_term(void);
bool sdmmc_get_detect(void);		// カード検知


#endif
