/******************************************************************************/
/*! @addtogroup Group2
	@file       currency.c
	@brief      currency process
	@date       2013/03/22
	@author     Development Dept at Tokyo
	@par        Revision
	$Id$
	@par        Copyright (C)
	2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
	@par        History
	- 2013/03/22 Development Dept at Tokyo
	  -# Initial Version
******************************************************************************/

//#include <string.h>
#include "country_custom.h"
//#include "common.h"
#include "Crncytbl.h"

#ifdef SIMURATION
#include "jcm_typedef.h"
#endif

#if defined(UBA_RTQ)
	#if 0 //2025-02-17
		/* この文字列の各スペースを使用してmodel name, countory code, ID, Versionを判別しているので、スペースを削らない事 */
		const unsigned char software_ver[64] = "UBA-700-SH2 GBR ID-003 V005-00 21FEB25                         \0";
		const u8 Ctry_software_id_uba[] =	"U(GBR5)-700-SH2-RTQ ID003-05V005-00 21FEB25";
	#else
		/* この文字列の各スペースを使用してmodel name, countory code, ID, Versionを判別しているので、スペースを削らない事 */
		const unsigned char software_ver[64] = "UBA-700-SH2 EUR ID-003 V025-00 27OCT25                         \0";
		const u8 Ctry_software_id[]		=	"U(EUR5)-700-SH2-RTQ ID003-05V025-00 27OCT25";
		const u8 Ctry_software_id_uba[]	=	"U(EUR5)-10-SH2-RTQ ID003-05V025-00 27OCT25";
	#endif
#else
	/* この文字列の各スペースを使用してmodel name, countory code, ID, Versionを判別しているので、スペースを削らない事 */
	const unsigned char software_ver[64] = "UBA-700-SS EUR ID-003 V027-00 05DEC24                          \0";
	const u8 Ctry_software_id[]		=	"U(EUR5)-700-SS ID003-05V027-00 05DEC24";
	const u8 Ctry_software_id_uba[]	=	"U(EUR5)-10-SS ID003-05V027-00 05DEC24";
#endif


//0: disable auto erase function
//1: enable auto erase function
//2: disable auto erase only for ICB
const u8 logid[]	= "1 10EUR-SS003 ";	//2023-07-04


/*--------------------------------------------------------------------------*/
/* COMMON DENOMINATION DATA													*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
const u16 dipsw_bar_inhi = 0x0000;
/*--------------------------------------------------------------------------*/
/* DipSW Inhibit table														*/
/*  現状使用禁止 : 0xFF80(DipSW1-8, DipSW2-1～8)							*/
/*--------------------------------------------------------------------------*/
const u16 dipsw_inhi[DENOMI_SIZE / 4] =
{
	0x0001,	/* 1: Denomi 1: EUR5.1		*/
	0x0002,	/* 2: Denomi 2: EUR10.1		*/
	0x0004,	/* 3: Denomi 3: EUR20.1		*/
	0x0008,	/* 4: Denomi 4: EUR50.1		*/
	0x0010,	/* 5: Denomi 5: EUR100.1	*/
	0x0020,	/* 6: Denomi 6: EUR200.1	*/
#if defined(UBA_RTQ)
	0x0040,	/* 7: Denomi 7:	EUR500.1	*/
#else
	0x0020,	/* 7: Denomi 7:	EUR500.1	*/
#endif
	0x0001,	/* 8: Denomi 8: EUR5.2		*/
	0x0002,	/* 9: Denomi 9: EUR10.2		*/
	0x0004,	/*10: Denomi10: EUR20.2		*/
	0x0008,	/*11: Denomi11: EUR50.2		*/
	0x0010,	/*12: Denomi12: EUR100.2	*/
	0x0020,	/*13: Denomi13:	EUR200.2	*/
	0,	/*14: Denomi14:				*/
	0,	/*15: Denomi15:				*/
	0,	/*16: Denomi16:				*/
	0,	/*17: Denomi17:				*/
	0,	/*18: Denomi18:				*/
	0,	/*19: Denomi19:				*/
	0,	/*20: Denomi18:				*/
	0,	/*21: Denomi19:				*/
	0,	/*22: Denomi20:				*/
};

/*--------------------------------------------------------------------------*/
/* Accept Denomi table														*/
/*--------------------------------------------------------------------------*/
/* 0 == Inhibit                */
/* LED Flash Count (Test Mode) */
const u8 accept_denomi[DENOMI_SIZE / 4] =
{
	1,	/* 1: Denomi 1: EUR5.1		*/
	2,	/* 2: Denomi 2: EUR10.1		*/
	3,	/* 3: Denomi 3: EUR20.1		*/
	4,	/* 4: Denomi 4: EUR50.1		*/
	5,	/* 5: Denomi 5: EUR100.1	*/
	6,	/* 6: Denomi 6: EUR200.1	*/
	7,	/* 7: Denomi 7:	EUR500.1	*/
	1,	/* 8: Denomi 8: EUR5.2		*/
	2,	/* 9: Denomi 9: EUR10.2		*/
	3,	/*10: Denomi10: EUR20.2		*/
	4,	/*11: Denomi11: EUR50.2		*/
	5,	/*12: Denomi12: EUR100.2	*/
	6,	/*13: Denomi13:	EUR200.2	*/
	0,	/*14: Denomi14:				*/
	0,	/*15: Denomi15:				*/
	0,	/*16: Denomi16:				*/
	0,	/*17: Denomi17:				*/
	0,	/*18: Denomi18:				*/
	0,	/*19: Denomi19:				*/
	0,	/*20: Denomi18:				*/
	0,	/*21: Denomi19:				*/
	0,	/*22: Denomi20:				*/
};
/* ######################################################################## */
/* ########################## ID-003関連 ################################## */
/* ######################################################################## */
/*--------------------------------------------------------------------------*/
/* ID003 Protocol Version													*/
/*--------------------------------------------------------------------------*/
#ifndef SIMURATION
/*										    10     	*/
/*										     |     	*/
/*								   0123456789012345	*/
#if defined(_PROTOCOL_ENABLE_ID003GD)
const u8 id003_protocol_ver[16] = "ID003GD-05     \0";
#elif defined(_PROTOCOL_ENABLE_ID003V)
const u8 id003_protocol_ver[16] = "ID003V-05      \0";
#else /* _PROTOCOL_ENABLE_ID003 */
const u8 id003_protocol_ver[16] = "ID003-05       \0";
#endif
#endif

/*--------------------------------------------------------------------------*/
/* ID003 CURRENCY ASSIGN DATA												*/
/* 0x61-0x69 --> 1st Country												*/
/* 0x71-0x79 --> 2nd Country												*/
/* When creating a one country firmware, put 0x00 for 0x71-0x79				*/
/*--------------------------------------------------------------------------*/
const u8 id003_bill_info[18][6] =
{
/*-- data1 [] --*/
	/* escrow code, country code, denomination data, inhibit(cmd) */
	{ 0x61,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi01:		*/
	,{ 0x62,        EUR_CRNCY,    0x05, 0x00,        0x00, 0x02 }	/* Denomi02:   5	*/
	,{ 0x63,        EUR_CRNCY,    0x0A, 0x00,        0x00, 0x04 }	/* Denomi03:  10	*/
	,{ 0x64,        EUR_CRNCY,    0x14, 0x00,        0x00, 0x08 }	/* Denomi04:  20	*/
	,{ 0x65,        EUR_CRNCY,    0x32, 0x00,        0x00, 0x10 }	/* Denomi05:  50	*/
	,{ 0x66,        EUR_CRNCY,    0x0A, 0x01,        0x00, 0x20 }	/* Denomi06: 100	*/
	,{ 0x67,        EUR_CRNCY,    0x14, 0x01,        0x00, 0x40 }	/* Denomi07: 200	*/
	,{ 0x68,        EUR_CRNCY,    0x32, 0x01,        0x00, 0x80 }	/* Denomi08: 500	*/
	,{ 0x00,		0x00,		  0x00,	0x00,		 0x00, 0x00 }	/* Denomi09:		*/
/*-- data2 [] --*/
	/* escrow code, country code, denomination data, inhibit(cmd) */
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi01:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi02:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi03:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi04:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi05:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi06:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi07:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi08:		*/
	,{ 0x00,        0x00,         0x00, 0x00,        0x00, 0x00 }	/* Denomi09:		*/
};


/*--------------------------------------------------------------------------*/
/* Convert base index to escrow code										*/
/*--------------------------------------------------------------------------*/
const u8 id003_index_to_code[DENOMI_SIZE / 4] =
{
	0x62,	/* 1: Denomi 1: EUR5.1		*/
	0x63,	/* 2: Denomi 2: EUR10.1		*/
	0x64,	/* 3: Denomi 3: EUR20.1		*/
	0x65,	/* 4: Denomi 4: EUR50.1		*/
	0x66,	/* 5: Denomi 5: EUR100.1	*/
	0x67,	/* 6: Denomi 6: EUR200.1	*/
	0x68,	/* 7: Denomi 7:	EUR500.1	*/
	0x62,	/* 8: Denomi 8: EUR5.2		*/
	0x63,	/* 9: Denomi 9: EUR10.2		*/
	0x64,	/*10: Denomi10: EUR20.2		*/
	0x65,	/*11: Denomi11: EUR50.2		*/
	0x66,	/*12: Denomi12: EUR100.2	*/
	0x67,	/*13: Denomi13:	EUR200.2	*/
	0x00,	/*14: Denomi14:				*/
	0x00,	/*15: Denomi15:				*/
	0x00,	/*16: Denomi16:				*/
	0x00,	/*17: Denomi17:				*/
	0x00,	/*18: Denomi18:				*/
	0x00,	/*19: Denomi19:				*/
	0x00,	/*20: Denomi18:				*/
	0x00,	/*21: Denomi19:				*/
	0x00,	/*22: Denomi20:				*/
};

/*--------------------------------------------------------------------------*/
/* ENABLE/DISABLE設定の各bitの有効/無効を示す(1:有効 / 0:無効)				*/
/*--------------------------------------------------------------------------*/
const u16 id003_inhi_mask = 0x00FE;
/* b0 : 61h    5 */
/* b1 : 62h   10 */
/* b2 : 63h   20 */
/* b3 : 64h   50 */
/* b4 : 65h  100 */
/* b5 : 66h  200 */
/* b6 : 67h  500 */
/* b7 : 68h		 */
/* b0 : 71h 	 */
/* b1 : 72h 	 */
/* b2 : 73h 	 */
/* b3 : 74h 	 */
/* b4 : 75h 	 */
/* b5 : 76h 	 */
/* b6 : 77h 	 */
/* b7 : 78h 	 */


/*******************************************************************************
* ICB Settings
*
*******************************************************************************/
const u8 smrt_id = 3; /* ID-003 */
const u8 smrt_assign = EUR_CRNCY;	/* EUR */
const u8 smrt_assign2 = 0;
const u8 smrt_assign3 = 0;
const u8 smrt_assign4 = 0;
const u8 smrt_assign5 = 0;

const u8 icb_custom_denom_assign[] =
{
	1,	/* 1: Denomi 1: EUR5.1		*/// rcTbl_dt1[XX][0] の列の数値 -1 を設定する
	2,	/* 2: Denomi 2: EUR10.1		*/
	3,	/* 3: Denomi 3: EUR20.1		*/
	4,	/* 4: Denomi 4: EUR50.1		*/
	5,	/* 5: Denomi 5: EUR100.1	*/
	6,	/* 6: Denomi 6: EUR200.1	*/
	7,	/* 7: Denomi 7:	EUR500.1	*/
	1,	/* 8: Denomi 8: EUR5.2		*/
	2,	/* 9: Denomi 9: EUR10.2		*/
	3,	/*10: Denomi10: EUR20.2		*/
	4,	/*11: Denomi11: EUR50.2		*/
	5,	/*12: Denomi12: EUR100.2	*/
	6,	/*13: Denomi13:	EUR200.2	*/
	0,	/*14: Denomi14:				*/
	0,	/*15: Denomi15:				*/
	0,	/*16: Denomi16:				*/
	0,	/*17: Denomi17:				*/
	0,	/*18: Denomi18:				*/
	0,	/*19: Denomi19:				*/
	0,	/*20: Denomi18:				*/
	0,	/*21: Denomi19:				*/
	0,	/*22: Denomi20:				*/
};

const u8 icb_custom_currency_table[20][3] =
{	//country	//base	//exponent
	{	0,		0,		0,		},	/*1:			*///rcTbl_dt1[XX][0] == 0x01
	{	0,		5,		0,		},  /*2:	EUR	  5	*///rcTbl_dt1[XX][0] == 0x02 の列の数値と合わせる 
	{	0,		10,		0,		},  /*3:	EUR	 10	*/
	{	0,		20,		0,		},  /*4:	EUR	 20	*/
	{	0,		50,		0,		},  /*5:	EUR	 50	*/
	{	0,		100,	0,		},  /*6:	EUR	100	*/
	{	0,		200,	0,		},  /*7:	EUR	200	*/
#if 0 // TUNG CHANGE 2023-10-13 (u8 limited value 255, if set 500 , process will be bug)
	{	0,		500,	0,		},  /*8:					*/
#else
	{	0,		5,		2,		},  /*8:	EUR	500			*/
#endif
	{	0,		0,		0,		},  /*9:					*/
	{	0,		0,		0,		},  /*10:					*/
	{	0,		0,		0,		},  /*11:					*/
	{	0,		0,		0,		},  /*12:					*/
	{	0,		0,		0,		},  /*13:					*/
	{	0,		0,		0,		},  /*14:					*/
	{	0,		0,		0,		},  /*15:					*/
	{	0,		0,		0,		},  /*16:					*/
	{	0,		0,		0,		},  /*17:					*/
	{	0,		0,		0,		},  /*18:					*/
	{	0,		0,		0,		},  /*19:					*/
	{	0,		0,		0,		},  /*20:					*/
};

/*===== JCM Device Log (JDL) definition =====*/
typedef struct _currency_information{
		u8 country_code;
		u8 denomi_base;
		u8 denomi_exp;
		u8 info0;
		u8 info1;
		u8 info2;
} currency_infomation;
const currency_infomation currency_info[DENOMI_SIZE] = {
/*  | Currency 	  | Denomination Code |     Detail Information      |   */
/*  |             |1st |2nd | Printed | reserve | reserve |   */
/*  |             |    |    | or Issue|         |         |   */
//		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
		{EUR_CRNCY,0x05,0x00,0x02,0x00,0x00},	/*   1 EUR	  5  */
		{EUR_CRNCY,0x0A,0x00,0x02,0x00,0x00},	/*   2 EUR	 10  */
		{EUR_CRNCY,0x14,0x00,0x02,0x00,0x00},	/*   3 EUR	 20  */
		{EUR_CRNCY,0x32,0x00,0x02,0x00,0x00},	/*   4 EUR	 50  */
		{EUR_CRNCY,0x0A,0x01,0x02,0x00,0x00},	/*   5 EUR	100  */
		{EUR_CRNCY,0x14,0x01,0x02,0x00,0x00},	/*   6 EUR	200  */
		{EUR_CRNCY,0x32,0x01,0x02,0x00,0x00},	/*   7 EUR	500  */
		{EUR_CRNCY,0x05,0x00,0x0D,0x00,0x00},	/*   8 EUR	  5  */
		{EUR_CRNCY,0x0A,0x00,0x0E,0x00,0x00},	/*   9 EUR	 10  */
		{EUR_CRNCY,0x14,0x00,0x0F,0x00,0x00},	/*  10 EUR	 20  */
		{EUR_CRNCY,0x32,0x00,0x10,0x00,0x00},	/*  11 EUR	 50  */
		{EUR_CRNCY,0x0A,0x01,0x12,0x00,0x00},	/*  12 EUR	100  */
		{EUR_CRNCY,0x14,0x01,0x12,0x00,0x00},	/*  13 EUR	200  */
		{	  0x00,0x00,0x00,0x00,0x00,0x00},	/*	  non			*/
		{	  0x00,0x00,0x00,0x00,0x00,0x00},	/*	  non			*/
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
		{	  0x00,0x00,0x00,0x00,0x00,0x00},	/*	  non			*/
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
};


/*--------------------------------------------------------------------------*/
/* length limit table	[mm]												*/
/*--------------------------------------------------------------------------*/
const u32 length_over_margin = 5;
const u32 length_less_margin = 5;
const u32 length_limits[] =
{
	120,	/* 1: Denomi 1: EUR5.1		*/
	127,	/* 2: Denomi 2: EUR10.1		*/
	133,	/* 3: Denomi 3: EUR20.1		*/
	140,	/* 4: Denomi 4: EUR50.1		*/
	147,	/* 5: Denomi 5: EUR100.1	*/
	153,	/* 6: Denomi 6: EUR200.1	*/
	160,	/* 7: Denomi 7:	EUR500.1	*/
	120,	/* 8: Denomi 8: EUR5.2		*/
	127,	/* 9: Denomi 9: EUR10.2		*/
	133,	/*10: Denomi10: EUR20.2		*/
	140,	/*11: Denomi11: EUR50.2		*/
	147,	/*12: Denomi12: EUR100.2	*/
	153,	/*13: Denomi13:	EUR200.2	*/
	0,	/*14: Denomi14:				*/
	0,	/*15: Denomi15:				*/
	0,	/*16: Denomi16:				*/
	0,	/*17: Denomi17:				*/
	0,		/*18: Denomi18:				*/
	0,		/*19: Denomi19:				*/
	0,	/*20: Denomi18:				*/
	0,	/*21: Denomi19:				*/
	0,	/*22: Denomi20:				*/

};
/*--------------------------------------------------------------------------*/
/* width limit table	[mm]												*/
/*--------------------------------------------------------------------------*/
const u32 width_over_margin = 5;
const u32 width_less_margin = 5;
const u32 width_limits[] =
{
	62,	/* 1: Denomi 1: EUR5.1		*/
	67,	/* 2: Denomi 2: EUR10.1		*/
	72,	/* 3: Denomi 3: EUR20.1		*/
	77,	/* 4: Denomi 4: EUR50.1		*/
	82,	/* 5: Denomi 5: EUR100.1	*/
	82,	/* 6: Denomi 6: EUR200.1	*/
	82,	/* 7: Denomi 7:	EUR500.1	*/
	62,	/* 8: Denomi 8: EUR5.2		*/
	67,	/* 9: Denomi 9: EUR10.2		*/
	72,	/*10: Denomi10: EUR20.2		*/
	77,	/*11: Denomi11: EUR50.2		*/
	77,	/*12: Denomi12: EUR100.2	*/
	77,	/*13: Denomi13:	EUR200.2	*/
	0,	/*14: Denomi14:				*/
	0,	/*15: Denomi15:				*/
	0,	/*16: Denomi16:				*/
	0,	/*17: Denomi17:				*/
	0,		/*18: Denomi18:				*/
	0,		/*19: Denomi19:				*/
	0,	/*20: Denomi18:				*/
	0,	/*21: Denomi19:				*/
	0,	/*22: Denomi20:				*/
};

#if 0 //2025-02-17
const float skew_limits[] =
{
	5.0f,	/* 1: Denomi 1: GBR5.6		*/
	5.0f,	/* 2: Denomi 2: CAN		*/
	5.0f,	/* 3: Denomi 3:				*/
	3.0f,	/* 4: Denomi 4:				*/
	3.0f,	/* 5: Denomi 5:				*/
	3.0f,	/* 6: Denomi 6:				*/
	3.0f,	/* 7: Denomi 7:				*/
	5.0f,	/* 8: Denomi 8:				*/
	5.0f,	/* 9: Denomi 9:				*/
	5.0f,	/*10: Denomi10:				*/
	3.0f,	/*11: Denomi11:				*/
	3.0f,	/*12: Denomi12:				*/
	3.0f,	/*13: Denomi13:				*/
	0.0f,	/*14: Denomi14:				*/
	0.0f,	/*15: Denomi15:				*/
	0.0f,	/*16: Denomi16:				*/
	0.0f,	/*17: Denomi17:				*/
	0.0f,		/*18: Denomi18:				*/
	0.0f,		/*19: Denomi19:				*/
	0.0f,		/*20: Denomi20:				*/
	0.0f,	/*21: Denomi21:				*/
	0.0f,	/*22: Denomi22:				*/

};

#else
const float skew_limits[] =
{
	5.0f,	/* 1: Denomi 1: EUR5.1		*/
	5.0f,	/* 2: Denomi 2: EUR10.1		*/
	5.0f,	/* 3: Denomi 3: EUR20.1		*/
	3.0f,	/* 4: Denomi 4: EUR50.1		*/
	3.0f,	/* 5: Denomi 5: EUR100.1	*/
	3.0f,	/* 6: Denomi 6: EUR200.1	*/
	3.0f,	/* 7: Denomi 7:	EUR500.1	*/
	5.0f,	/* 8: Denomi 8: EUR5.2		*/
	5.0f,	/* 9: Denomi 9: EUR10.2		*/
	5.0f,	/*10: Denomi10: EUR20.2		*/
	3.0f,	/*11: Denomi11: EUR50.2		*/
	3.0f,	/*12: Denomi12: EUR100.2	*/
	3.0f,	/*13: Denomi13:	EUR200.2	*/
	0.0f,	/*14: Denomi14:				*/
	0.0f,	/*15: Denomi15:				*/
	0.0f,	/*16: Denomi16:				*/
	0.0f,	/*17: Denomi17:				*/
	0.0f,		/*18: Denomi18:				*/
	0.0f,		/*19: Denomi19:				*/
	0.0f,		/*20: Denomi20:				*/
	0.0f,	/*21: Denomi21:				*/
	0.0f,	/*22: Denomi22:				*/

};
#endif
