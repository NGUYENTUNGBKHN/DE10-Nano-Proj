/******************************************************************************/
/*! @addtogroup Main
    @file       program_crc.c
    @date       2018/01/26
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * program_crc.c
 *
 *  Created on: 2018/01/26
 *      Author: suzuki-hiroyuki
 */

/************************** EXTERNAL FUNCTIONS *************************/

/***********************************************************************/

/*********************************************/
/*-	Program CRC Section	                    -*/
/*-	CRC calc [0x00180000 to 0x010FFFFD]     -*/
/*-	use aligned(2) to assign last 2bytes    -*/
/*********************************************/
const unsigned char __attribute__ ((aligned(2),used)) ex_program_crc2[2] = { 0x00, 0x00 };
const unsigned char __attribute__ ((aligned(2),used)) ex_program_crc[2] = { 0x00, 0x00 };
