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
 * @file cyclonev_wdt_reg.h
 * @brief ウォッチドッグタイマレジスタの定義
 * @date 2013.1.22 Created.
 */
/****************************************************************************/
#define L4WD0_BASE_ADDRESS			(0xFFD02000)
#define L4WD1_BASE_ADDRESS			(0xFFD03000)

/* stat*/
enum
{
    RESET_EVENT_L4WD0RST = 0x00004000,	/* L4 Watchdog 0 Warm Reset */
    RESET_EVENT_L4WD1RST = 0x00008000,	/* L4 Watchdog 1 Warm Reset */
};

/****************************************************************/
/**
 * @struct WDT_REG_STRUCTURE
 * @brief ウォッチドッグタイマレジスタ
 */
/****************************************************************/
struct WDT_REG_STRUCTURE
{
#if 1
	union
	{
		u32 _LONG;
		struct
		{
			u32 WDT_EN: 1;
			u32 RMOD: 1;
			u32 RESERVED: 30;
		} BIT;
	} WDT_CR;


	union
	{
		u32 _LONG;
		struct
		{
			u32 TOP: 4;
			u32 TOP_INIT: 4;
		} BIT;

	} WDT_TORR;


	union
	{
		u32 _LONG;
		struct
		{
			u32 WDT_CCVR;
		} BIT;
	} WDT_CCVR;


	union
	{
		u32 _LONG;
		struct
		{
			u32 WDT_CRR: 8;
			u32 RESERVED: 24;
		} BIT;
	} WDT_CRR;


	union
	{
		u32 _LONG;
		struct
		{
			u32 WDT_STAT: 1;
			u32 RESERVED: 31;
		} BIT;
	} WDT_STAT;


	union
	{
		u32 _LONG;
		struct
		{
			u32 WDT_EOI: 1;
			u32 RESERVED: 31;
		} BIT;
	} WDT_EOI;

	u8 RESERVED1[208];							/* 予備 */

	u32 CP_WDT_USER_TOP_MAX;
	u32 CP_WDT_USER_TOP_INIT_MAX;
	u32 CP_WDT_TOP_RST;
	u32 CP_WDT_CNT_RST;

	union
	{
		u32 _LONG;
		struct
		{
			u32 CP_WDT_ALWAYES_EN: 1;
			u32 CP_WDT_DFLT_RMOD: 1;
			u32 CP_WDT_DUAL_TOP: 1;
			u32 CP_WDT_HC_RMOD: 1;
			u32 CP_WDT_HC_RPL: 1;
			u32 CP_WDT_HC_TOP: 1;
			u32 CP_WDT_USE_FIX_TOP: 1;
			u32 CP_WDT_PAUSE: 1;
			u32 CP_WDT_APB_DATA_WIDTH: 2;
			u32 CP_WDT_DFLT_RPL: 3;
			u32 RESERVED1: 3;
			u32 CP_WDT_DFLT_TOP:  4;
			u32 CP_WDT_DFLT_TOP_INIT: 4;
			u32 CP_WDT_CNT_WIDTH: 5;
			u32 RESERVED2: 3;
		} BIT;
	} WDT_COMP_PARAM_1;

	u32 WDT_COMP_VERSION;

	u32 WDT_COMP_TYPE;
#else
	union
	{
		s32 _LONG;
		struct
		{
			s32 WDT_EN: 1;
			s32 RMOD: 1;
			s32 RESERVED: 30;
		} BIT;
	} WDT_CR;


	union
	{
		s32 _LONG;
		struct
		{
			s32 TOP: 4;
			s32 TOP_INIT: 4;
		} BIT;
		
	} WDT_TORR;
	
	
	union
	{
		s32 _LONG;
		struct
		{
			u32 WDT_CCVR;
		} BIT;
	} WDT_CCVR;

	
	union
	{
		s32 _LONG;
		struct
		{
			s32 WDT_CRR: 8;
			s32 RESERVED: 24;
		} BIT;
	} WDT_CRR;
	
	
	union
	{
		s32 _LONG;
		struct
		{
			s32 WDT_STAT: 1;
			s32 RESERVED: 31;
		} BIT;
	} WDT_STAT;
	
	
	union
	{
		s32 _LONG;
		struct
		{
			s32 WDT_EOI: 1;
			s32 RESERVED: 31;
		} BIT;
	} WDT_EOI;

	u8 RESERVED1[208];							/* 予備 */

	u32 CP_WDT_USER_TOP_MAX;
	u32 CP_WDT_USER_TOP_INIT_MAX;
	u32 CP_WDT_TOP_RST;
	s32 CP_WDT_CNT_RST;
	
	union
	{
		s32 _LONG;
		struct
		{
			s32 CP_WDT_ALWAYES_EN: 1;
			s32 CP_WDT_DFLT_RMOD: 1;
			s32 CP_WDT_DUAL_TOP: 1;
			s32 CP_WDT_HC_RMOD: 1;
			s32 CP_WDT_HC_RPL: 1;
			s32 CP_WDT_HC_TOP: 1;
			s32 CP_WDT_USE_FIX_TOP: 1;
			s32 CP_WDT_PAUSE: 1;
			s32 CP_WDT_APB_DATA_WIDTH: 2;
			s32 CP_WDT_DFLT_RPL: 3;
			s32 RESERVED1: 3;
			s32 CP_WDT_DFLT_TOP:  4;
			s32 CP_WDT_DFLT_TOP_INIT: 4;
			s32 CP_WDT_CNT_WIDTH: 5;
			s32 RESERVED2: 3;
		} BIT;
	} WDT_COMP_PARAM_1;
	
	u32 WDT_COMP_VERSION;
	
	u32 WDT_COMP_TYPE;
#endif
};


#define WD0_REG		(*(volatile struct WDT_REG_STRUCTURE*)L4WD0_BASE_ADDRESS)
#define WD1_REG		(*(volatile struct WDT_REG_STRUCTURE*)L4WD1_BASE_ADDRESS)


