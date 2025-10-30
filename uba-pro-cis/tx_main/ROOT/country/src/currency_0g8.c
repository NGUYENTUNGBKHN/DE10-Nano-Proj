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

const u16 dipsw_bar_inhi = 0x0001;

const u8 Head_iInterface_GSA[] = "1.1.2,UBA-700-SS,";

//GAT Data
const u8 romid_name[] = "ROMID:";
const u8 romid[] = "U(USA)-700-SS ID0G8-01  ";
const u8 romid_UBA[] = "U(USA)-14_24-SX ID0G8-01";
const u8 gat_new_line[] = "\n\r";
const u8 ver_name[] = "Ver & Date:";
const u8 ver[] = "V1.01-01 11MAY23";
const u8 crc16_name[] = "CRC16:";


const u8 DucGdsBuildVersion[] = "V1.01-01,";
const u8 DucGdsManufacuturingDate[] = "2025-06-13";
const u8 DucGdsFirmwareIssue[] = "U(USA)700-SS ID0G8-01,";


#if 1 //2022-11-15
const u8 smrt_id = 240; /* ID-0G8 */
const u8 smrt_assign = 0x01;	/* USA */
const u8 smrt_assign2 = 0;
const u8 smrt_assign3 = 0;
const u8 smrt_assign4 = 0;
const u8 smrt_assign5 = 0;
#endif


/*******************************************************************************
* GSA Note Table diff ID-003
*
*******************************************************************************/
const u8 GsaNumNoteDataEntries = 6;
const u8 GsaNoteTable[] = 
{
	0x01, 'U', 'S', 'D', 0x01, 0x00, 0x80, 0x00,	/*	1 Dollar	*/
	0x02, 'U', 'S', 'D', 0x05, 0x00, 0x80, 0x00,    /*	5 Dollar	*/
	0x03, 'U', 'S', 'D', 0x0A, 0x00, 0x80, 0x00,    /*	10 Dollar	*/
	0x04, 'U', 'S', 'D', 0x14, 0x00, 0x80, 0x00,    /*	20 Dollar	*/
	0x05, 'U', 'S', 'D', 0x32, 0x00, 0x80, 0x00,    /*	50 Dollar	*/
	0x06, 'U', 'S', 'D', 0x64, 0x00, 0x80, 0x00,    /*	100 Dollar	*/
};

/*--------------------------------------------------------------------------*/
/* Convert base index to escrow code	diff ID-003と異なるので注意が必要	*/
/*--------------------------------------------------------------------------*/
const u8 id0g8_index_to_code[DENOMI_SIZE / 4] =
{

	0x01,	/* 1: Denomi 1: USD1.1B		*/
	0x00,	/* 2: Denomi 2: USD2.2B		*/
	0x02,	/* 3: Denomi 3: USD5.1B		*/
	0x03,	/* 4: Denomi 4: USD10.1B	*/
	0x04,	/* 5: Denomi 5: USD20.1B	*/
	0x05,	/* 6: Denomi 6: USD50.1B	*/
	0x06,	/* 7: Denomi 7:	USD100.1B	*/
	0x02,	/* 3: Denomi 8: USD5.2		*/
	0x03,	/* 4: Denomi 9: USD10.2		*/
	0x04,	/* 5: Denomi10: USD20.2		*/
	0x05,	/* 6: Denomi11: USD50.2		*/
	0x05,	/* 6: Denomi12: USD50.2		*/
	0x06,	/* 7: Denomi13:	USD100.2	*/
	0x06,	/* 7: Denomi14:	USD100.2	*/
	0x02,	/* 8: Denomi15: USD5.3		*/
	0x03,	/* 9: Denomi16:	USD10.3		*/
	0x04,	/*10: Denomi17: USD20.3		*/
	0x05,	/*11: Denomi18:	USD50.3		*/
	0x06,	/*12: Denomi19: USD100.3	*/
	0x00,	/*13: Denomi18:				*/
	0x00,	/*13: Denomi19:				*/
	0x00,	/*13: Denomi20:				*/

};

/*--------------------------------------------------------------------------*/
/* COMMON DENOMINATION DATA													*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* DipSW Inhibit table														*/
/*  現状使用禁止 : 0xFF80(DipSW1-8, DipSW2-1～8)	same ID-003				*/
/*--------------------------------------------------------------------------*/
const u16 dipsw_inhi[DENOMI_SIZE / 4] =
{

	0x0002,	/* 1: Denomi 1: USD1.1B		*/
	0,		/* 2: Denomi 2: USD2.2B		*/
	0x0004,	/* 3: Denomi 3: USD5.1B		*/
	0x0008,	/* 4: Denomi 4: USD10.1B	*/
	0x0010,	/* 5: Denomi 5: USD20.1B	*/
	0x0020,	/* 6: Denomi 6: USD50.1B	*/
	0x0040,	/* 7: Denomi 7:	USD100.1B	*/
	0x0004,	/* 8: Denomi 8: USD5.2		*/
	0x0008,	/* 9: Denomi 9: USD10.2		*/
	0x0010,	/*10: Denomi10: USD20.2		*/
	0x0020,	/*11: Denomi11: USD50.2		*/
	0x0020,	/*12: Denomi12: USD50.2		*/
	0x0040,	/*13: Denomi13:	USD100.2	*/
	0x0040,	/*14: Denomi14:	USD100.2	*/
	0x0004,	/*15: Denomi15: USD5.3		*/
	0x0008,	/*16: Denomi16:	USD10.3		*/
	0x0010,	/*17: Denomi17: USD20.3		*/
	0x0020,	/*18: Denomi18:	USD50.3		*/
	0x0040,	/*19: Denomi19: USD100.3	*/
	0,	/*20: Denomi18:				*/
	0,	/*21: Denomi19:				*/
	0,	/*22: Denomi20:				*/

};

/*--------------------------------------------------------------------------*/
/* Accept Denomi table							same ID-003					*/
/*--------------------------------------------------------------------------*/
/* 0 == Inhibit                */
/* LED Flash Count (Test Mode) */
const u8 accept_denomi[DENOMI_SIZE / 4] =
{

	1,	/* 1: Denomi 1: USD1.1B		*/
	0,	/* 2: Denomi 2: USD2.2B		*/
	0,	/* 3: Denomi 3: USD5.1B		*/
	0,	/* 4: Denomi 4: USD10.1B	*/
	0,	/* 5: Denomi 5: USD20.1B	*/
	0,	/* 6: Denomi 6: USD50.1B	*/
	0,	/* 7: Denomi 7:	USD100.1B	*/
	3,	/* 3: Denomi 8: USD5.2		*/
	4,	/* 4: Denomi 9: USD10.2		*/
	5,	/* 5: Denomi10: USD20.2		*/
	6,	/* 6: Denomi11: USD50.2		*/
	6,	/* 6: Denomi12: USD50.2		*/
	7,	/* 7: Denomi13:	USD100.2	*/
	7,	/* 7: Denomi14:	USD100.2	*/
	3,	/* 8: Denomi15: USD5.3		*/
	4,	/* 9: Denomi16:	USD10.3		*/
	5,	/*10: Denomi17: USD20.3		*/
	6,	/*11: Denomi18:	USD50.3		*/
	7,	/*12: Denomi19: USD100.3	*/
	0,	/*13: Denomi18:				*/
	0,	/*13: Denomi19:				*/
	0,	/*13: Denomi20:				*/

};



/* ######################################################################## */
/* ##########################  ################################## */
/* ######################################################################## */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* ENABLE/DISABLE設定の各bitの有効/無効を示す(1:有効 / 0:無効)	same ID-003	*/
/*--------------------------------------------------------------------------*/
const u16 id0g8_inhi_mask = 0x007D;
/* b0 : 61h    1 */
/* b1 : 62h    2 */
/* b2 : 63h    5 */
/* b3 : 64h   10 */
/* b4 : 65h   20 */
/* b5 : 66h   50 */
/* b6 : 67h  100 */
/* b7 : 68h		 */
/* b0 : 71h 	 */
/* b1 : 72h 	 */
/* b2 : 73h 	 */
/* b3 : 74h 	 */
/* b4 : 75h 	 */
/* b5 : 76h 	 */
/* b6 : 77h 	 */
/* b7 : 78h 	 */






/* same ID-003 */
const u8 icb_custom_denom_assign[] =
{
	0,	/* 1: Denomi 1: USD1.1B		*/
	1,	/* 2: Denomi 2: USD2.2B		*/
	2,	/* 3: Denomi 3: USD5.1B		*/
	3,	/* 4: Denomi 4: USD10.1B	*/
	4,	/* 5: Denomi 5: USD20.1B	*/
	5,	/* 6: Denomi 6: USD50.1B	*/
	6,	/* 7: Denomi 7:	USD100.1B	*/
	2,	/* 3: Denomi 8: USD5.2		*/
	3,	/* 4: Denomi 9: USD10.2		*/
	4,	/* 5: Denomi10: USD20.2		*/
	5,	/* 6: Denomi11: USD50.2		*/
	5,	/* 6: Denomi12: USD50.2		*/
	6,	/* 7: Denomi13:	USD100.2	*/
	6,	/* 7: Denomi14:	USD100.2	*/
	2,	/* 8: Denomi15: USD5.3		*/
	3,	/* 9: Denomi16:	USD10.3		*/
	4,	/*10: Denomi17: USD20.3		*/
	5,	/*11: Denomi18:	USD50.3		*/
	6,	/*12: Denomi19: USD100.3	*/
	0,	/*13: Denomi18:				*/
	0,	/*13: Denomi19:				*/
	0,	/*13: Denomi20:				*/
};

/* same ID-003 */
const u8 icb_custom_currency_table[20][3] =
{	//country	//base	//exponent
	{	0,		1,		0,		},	/*1:	USA 1 Dollar	*/
	{	0,		2,		0,		},  /*2:	USA 2 Dollar	*/
	{	0,		5,		0,		},  /*3:	USA 5 Dollar	*/
	{	0,		10,		0,		},  /*4:	USA 10 Dollar	*/
	{	0,		20,		0,		},  /*5:	USA 20 Dollar	*/
	{	0,		50,		0,		},  /*6:	USA 50 Dollar	*/
	{	0,		100,	0,		},  /*7:	USA 100 Dollar	*/
	{	0,		0,		0,		},  /*8:					*/
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
/* same ID-003 */
typedef struct _currency_information{
		u8 country_code;
		u8 denomi_base;
		u8 denomi_exp;
		u8 info0;
		u8 info1;
		u8 info2;
} currency_infomation;
const currency_infomation currency_info[DENOMI_SIZE] = {
		{USA_CRNCY,0x01,0x00,0x00,0x00,0x00},	/*   1 USA    1 $  */
		{USA_CRNCY,0x02,0x00,0x00,0x00,0x00},	/*   2 USA    2 $  */
		{USA_CRNCY,0x05,0x00,0x00,0x00,0x00},	/*   3 USA    5 $  */
		{USA_CRNCY,0x0A,0x00,0x00,0x00,0x00},	/*   4 USA   10 $  */
		{USA_CRNCY,0x14,0x00,0x00,0x00,0x00},	/*   5 USA   20 $  */
		{USA_CRNCY,0x32,0x00,0x00,0x00,0x00},	/*   6 USA   50 $  */
		{USA_CRNCY,0x0A,0x01,0x00,0x00,0x00},	/*   7 USA  100 $  */
		{USA_CRNCY,0x05,0x00,0x00,0x00,0x00},	/*   8 USA    5 $  */
		{USA_CRNCY,0x0A,0x00,0x00,0x00,0x00},	/*   9 USA   10 $  */
		{USA_CRNCY,0x14,0x00,0x00,0x00,0x00},	/*  10 USA   20 $  */
		{USA_CRNCY,0x32,0x00,0x00,0x00,0x00},	/*  11 USA   50 $  */
		{USA_CRNCY,0x32,0x00,0x00,0x00,0x00},	/*  12 USA   50 $  */
		{USA_CRNCY,0x0A,0x01,0x00,0x00,0x00},	/*  13 USA  100 $  */
		{USA_CRNCY,0x0A,0x01,0x00,0x00,0x00},	/*  14 USA  100 $  */
		{USA_CRNCY,0x05,0x00,0x00,0x00,0x00},	/*  15 USA    5 $  */
		{USA_CRNCY,0x0A,0x00,0x00,0x00,0x00},	/*  16 USA   10 $  */
		{USA_CRNCY,0x14,0x00,0x00,0x00,0x00},	/*  17 USA   20 $  */
		{USA_CRNCY,0x32,0x00,0x00,0x00,0x00},	/*  18 USA   50 $  */
		{USA_CRNCY,0x0A,0x01,0x00,0x00,0x00},	/*  19 USA  100 $  */
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
		{     0x00,0x00,0x00,0x00,0x00,0x00},	/*    non	          */
};


/*--------------------------------------------------------------------------*/
/* length limit table	[mm]												*/
/*--------------------------------------------------------------------------*/
const u32 length_over_margin = 5;
const u32 length_less_margin = 6;
const u32 length_limits[] =
{
	156,	/* 1: Denomi 1: USD1.1B		*/
	156,	/* 2: Denomi 2: USD2.2B		*/
	156,	/* 3: Denomi 3: USD5.1B		*/
	156,	/* 4: Denomi 4: USD10.1B	*/
	156,	/* 5: Denomi 5: USD20.1B	*/
	156,	/* 6: Denomi 6: USD50.1B	*/
	156,	/* 7: Denomi 7:	USD100.1B	*/
	156,	/* 8: Denomi 8: USD5.2		*/
	156,	/* 9: Denomi 9: USD10.2		*/
	156,	/*10: Denomi10: USD20.2		*/
	156,	/*11: Denomi11: USD50.2		*/
	156,	/*11: Denomi12: USD50.2		*/
	156,	/*12: Denomi13:	USD100.2	*/
	156,	/*12: Denomi14:	USD100.2	*/
	156,	/*13: Denomi15: USD5.3		*/
	156,	/*14: Denomi16:	USD10.3		*/
	156,	/*15: Denomi17: USD20.3		*/
	156,	/*16: Denomi18:	USD50.3		*/
	156,	/*17: Denomi19: USD100.3	*/
	0,		/*18: Denomi18:				*/
	0,		/*19: Denomi19:				*/
	0,		/*20: Denomi20:				*/
};
/*--------------------------------------------------------------------------*/
/* width limit table	[mm]												*/
/*--------------------------------------------------------------------------*/
const u32 width_over_margin = 5;
const u32 width_less_margin = 5;
const u32 width_limits[] =
{
	66,	/* 1: Denomi 1: USD1.1B		*/
	66,	/* 2: Denomi 2: USD2.2B		*/
	66,	/* 3: Denomi 3: USD5.1B		*/
	66,	/* 4: Denomi 4: USD10.1B	*/
	66,	/* 5: Denomi 5: USD20.1B	*/
	66,	/* 6: Denomi 6: USD50.1B	*/
	66,	/* 7: Denomi 7:	USD100.1B	*/
	66,	/* 8: Denomi 8: USD5.2		*/
	66,	/* 9: Denomi 9: USD10.2		*/
	66,	/*10: Denomi10: USD20.2		*/
	66,	/*11: Denomi11: USD50.2		*/
	66,	/*11: Denomi12: USD50.2		*/
	66,	/*12: Denomi13:	USD100.2	*/
	66,	/*12: Denomi14:	USD100.2	*/
	66,	/*13: Denomi15: USD5.3		*/
	66,	/*14: Denomi16:	USD10.3		*/
	66,	/*15: Denomi17: USD20.3		*/
	66,	/*16: Denomi18:	USD50.3		*/
	66,	/*17: Denomi19: USD100.3	*/
	0,		/*18: Denomi18:				*/
	0,		/*19: Denomi19:				*/
	0,		/*20: Denomi20:				*/	
};
const float skew_limits[] =
{
	10.0f,	/* 1: Denomi 1: USD1.1B		*/
	10.0f,	/* 2: Denomi 2: USD2.2B		*/
	10.0f,	/* 3: Denomi 3: USD5.1B		*/
	10.0f,	/* 4: Denomi 4: USD10.1B	*/
	10.0f,	/* 5: Denomi 5: USD20.1B	*/
	10.0f,	/* 6: Denomi 6: USD50.1B	*/
	10.0f,	/* 7: Denomi 7:	USD100.1B	*/
	10.0f,	/* 8: Denomi 8: USD5.2		*/
	10.0f,	/* 9: Denomi 9: USD10.2		*/
	10.0f,	/*10: Denomi10: USD20.2		*/
	10.0f,	/*11: Denomi11: USD50.2		*/
	10.0f,	/*11: Denomi12: USD50.2		*/
	10.0f,	/*12: Denomi13:	USD100.2	*/
	10.0f,	/*12: Denomi14:	USD100.2	*/
	10.0f,	/*13: Denomi15: USD5.3		*/
	10.0f,	/*14: Denomi16:	USD10.3		*/
	10.0f,	/*15: Denomi17: USD20.3		*/
	10.0f,	/*16: Denomi18:	USD50.3		*/
	10.0f,	/*17: Denomi19: USD100.3	*/
	0,		/*18: Denomi18:				*/
	0,		/*19: Denomi19:				*/
	0,		/*20: Denomi20:				*/
};

