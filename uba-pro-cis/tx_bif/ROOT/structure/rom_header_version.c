/******************************************************************************/
/*! @addtogroup Main
    @file       rom_header_version.c
    @date       2018/01/25
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * rom_header_version.c
 *
 *  Created on: 2018/01/25
 *      Author: suzuki-hiroyuki
 */

/* Boot I/F Version	[0x18-0x27]	*/
#if defined(_PROTOCOL_ENABLE_ID0G8)
const unsigned char _boot_if_version[0x10]  __attribute__ ((section ("ROM_HEADR_VER"),used))  =
		"B-ID0G8 Ver0.01";
#else
const unsigned char _boot_if_version[0x10]  __attribute__ ((section ("ROM_HEADR_VER"),used))  =
		"B-ID003 Ver0.07";
#endif
