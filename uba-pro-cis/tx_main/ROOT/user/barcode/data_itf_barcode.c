/*******************************************************************************
* Project: CIS Bill Acceptor
* File: data_itf_barcode.h
* Contents: itf barcode tables
*
*
*******************************************************************************/

#define EXT
#include "tem_global.c"

const int itf_barcode_character_lookup[11] =
{
	'0',						/* 000 */
	'1', 						/* 001 */
	'2', 						/* 002 */
	'3', 						/* 003 */
	'4', 						/* 004 */
	'5', 						/* 005 */
	'6', 						/* 006 */
	'7', 						/* 007 */
	'8', 						/* 008 */
	'9', 						/* 009 */
	ITF_BARCODE_UNKNOWN_DIGIT, 	/* 010 */
};
