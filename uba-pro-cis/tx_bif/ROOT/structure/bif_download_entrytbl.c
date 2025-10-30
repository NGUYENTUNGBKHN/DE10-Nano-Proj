/******************************************************************************/
/*! @addtogroup Main
    @file       bif_download_entrytbl.c
    @date       2018/01/25
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * bif_entrytbl.c
 *
 *  Created on: 2018/01/25
 *      Author: suzuki-hiroyuki
 */
#include "common.h"

/************************** EXTERNAL FUNCTIONS *************************/
/***********************************************************************/
extern void __bif_if_download_entry(void);
extern void __bif_if_diff_download_entry(void);
extern void __bif_download_entry(void);
extern void __bif_host_download_entry(void);
extern void __bif_subline_usb_download_entry(void);
/*------ external ROM i/f define -----*/
// Address :0x0018012C
void (* const bif_download_entry[])(void)    __attribute__ ((section ("BIF_DOWNLOAD_ENTRY"),used))  =
{
	/* (002C) BIF I/F DOWNLOAD ENTRY ADDR. 		*/
	__bif_if_download_entry,
	/* (0030) BIF DEVICE USB DOWNLOAD ENTRY ADDR. 		*/
	__bif_download_entry,
	/* (0034) BIF HOST USB DOWNLOAD ENTRY ADDR. 		*/
	__bif_host_download_entry,
	/* (0038) BIF SUBLINE USB DOWNLOAD ENTRY ADDR. 		*/
	__bif_subline_usb_download_entry,
	/* (003C) BIF I/F DIFFERENTIAL DOWNLOAD ENTRY ADDR.	*/
	__bif_if_diff_download_entry,
};
