 /******************************************************************************/
/*! @addtogroup Main
    @file       rom_header_entrytbl.c
    @date       2018/01/25
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * rom_header_entrytbl.c
 *
 *  Created on: 2018/01/25
 *      Author: suzuki-hiroyuki
 */
// entry function address
/************************** External functions *******************************/

extern void Reset_Handler(void);
extern void __cs3_reset(void);
extern void Reset_Vector(void);

/* external ROM i/f jump table [0x28-0x40]	*/
void (* const _boot_if_entry[])(void)  __attribute__ ((section ("ROM_HEADER_ENTTBL"),used))  =
{
#if 0
		Reset_Vector,
#elif 1
		__cs3_reset,              /* (0028) EXTERNAL ROM START ADDR. 		*/
#else
		ex_boot_if_start,			/* (0028) EXTERNAL ROM START ADDR. 		*/
#endif
};

