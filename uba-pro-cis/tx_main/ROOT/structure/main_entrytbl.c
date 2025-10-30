/******************************************************************************/
/*! @addtogroup Main
    @file       bif_entrytbl.c
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
extern void __cs3_reset(void);
/***********************************************************************/

// Address :0x00180128
/*------ external ROM i/f define -----*/
void (* const bif_entry[])(void) __attribute__ ((section ("MAIN_ENTTBL"),used))  =
{
		/* (0028) EXTERNAL ROM START ADDR. 		*/
		__cs3_reset
};
