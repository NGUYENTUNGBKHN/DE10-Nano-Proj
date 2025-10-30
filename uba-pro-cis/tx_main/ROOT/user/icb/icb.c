/******************************************************************************/
/*! @addtogroup Group1
    @file       icb.c
    @brief      validate a icb ticket
    @date       2018/02/26
    @author     T.Yokoyama
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "custom.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "icb_ram.c"

#include "icb.h"


/************************** PRIVATE FUNCTIONS *************************/
static void	icb_set_manufacture_info(void);
static void	set_box_size_to_ICB(void);
/************************** PRIVATE DEFINITIONS *************************/
/*******************************************************************************
* Firmware Identification Strings
*
*******************************************************************************/
extern const u8 software_ver[];	/*	max 15char	*/
// _icb_blk_infoのaddressがblockを保持していたので
// addressに変更
#if 0//
const struct _icb_blk_info	send_blk_info[] =
{
	 {   0, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[0]}		/*	 1:Denomi1-2						*/
	,{   4, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[2]}		/*	 2:Denomi3-4 						*/
	,{   8, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[4]}		/*	 3:Denomi5-6						*/
	,{  12, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[6]}		/*	 4:Denomi7-8						*/
	,{  16, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[8]}		/*	 5:Denomi9-10						*/
	,{  20, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[10]}		/*	 6:Denomi11-12						*/
	,{  24, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[12]}		/*	 7:Denomi13-14						*/
	,{  28, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[14]}		/*	 8:Denomi15-16						*/
	,{  32, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[16]}		/*	 9:(Denomi17-18)					*/
	,{  36, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[18]}		/*	10:(Denomi19-20)					*/
	,{  40, (2 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi_reserved}	/*	11:Insert bill info					*/
	,{  48, (5 * SIZE_OF_1BLK), (u8 *)&Smrtdat.cinfo[0]}		/*	12:Last Ticket number				*/
	,{  68, (2 * SIZE_OF_1BLK), (u8 *)&Smrtdat.err[0]}			/*	13:error counter1,2					*/
	,{  76, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.err[8]}			/*	14:error counter3					*/
	,{  80, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.err[12]}			/*	15:(error counter4)					*/
	,{  84, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.err[16]}			/*	16:(error counter5)					*/
	,{  88, (5 * SIZE_OF_1BLK), (u8 *)&Smrtdat.gameno[0]}		/*	17:M/C number						*/
	,{ 108, (5 * SIZE_OF_1BLK), (u8 *)&Smrtdat.boxno[0]}		/*	18:box number						*/
	,{ 128, (2 * SIZE_OF_1BLK), (u8 *)&Smrtdat.ver[0]}			/*	19:software version					*/
	,{ 136, (5 * SIZE_OF_1BLK), (u8 *)&Smrtdat.rw_ver[0]}		/*	20:RFID-R/W firmware version		*/
	,{ 156, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.restim}			/*	21:Box remove time					*/
	,{ 160, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.settim}			/*	22:Box setting	time				*/
	,{ 164, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.initim}			/*	23:initial	time					*/
	,{ 168, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.flg}				/*	24:flag, currency assign, id, sum	*/
	,{ 172, (4 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.rej[0][0]}		/*	25:reject counter					*/
	,{ 188, (6 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.crncy[0]}		/*	26:currency assign					*/
	,{ 212, (2 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.ticket_rej[0]}	/*	27:reject counter for Ticket		*/
	,{ 220, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.model[0]}		/*	28:Model							*/
	,{ 224, (2 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.serial[0]}		/*	29:Serial number reserved(1) & sum	*/
	,{ 228, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.serial[4]}		/*	30:Serial number Low(2),reserved(1) & sum	*/
																/*	   Low2Byteは、29の下位2Byteを送信する		*/
																/*	sumのみを更新する場合は"30"を使用	*/
																/*	初期登録時については"30"は必要ない	*/
};
#else 
	#if !defined(UBA_RTQ)
	//8byte
	//(_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
	//(_RFID_BACK_DATA28==1)		
	const struct _icb_blk_info	send_blk_info[] =
	{
		{   0, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[0]}		/*	 1:Denomi1-2						*/
		,{   8, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[4]}		/*	 2:Denomi5-6						*/
		,{  16, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[8]}		/*	 3:Denomi9-10						*/
		,{  24, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[12]}		/*	 4:Denomi13-14						*/
		,{  32, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi[16]}		/*	 5:(Denomi17-18)					*/
		,{  40, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.denomi_reserved}	/*	 6:Insert bill info					*/
		,{  48, (3 * SIZE_OF_1BLK), (u8 *)&Smrtdat.cinfo[0]}		/*	 7:Last Ticket number				*/
		,{  72, (2 * SIZE_OF_1BLK), (u8 *)&Smrtdat.err[4]}			/*	 8:error counter3					*/
		,{  88, (3 * SIZE_OF_1BLK), (u8 *)&Smrtdat.gameno[0]}		/*	 9:M/C number						*/
		,{ 112, (3 * SIZE_OF_1BLK), (u8 *)&Smrtdat.boxno[4]}		/*	10:M/C number						*/
		,{ 136, (3 * SIZE_OF_1BLK), (u8 *)&Smrtdat.rw_ver[0]}		/*	11:RFID-R/W firmware version		*/
		,{ 160, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat.settim}			/*	12:Box setting	time				*/
		,{ 168, (3 * SIZE_OF_1BLK), (u8 *)&Smrtdat.flg}				/*	13:flag, currency assign, id, sum	*/
		,{ 192, (2 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.crncy[4]}		/*	14:currency assign					*/
		,{ 208, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.assign2}		/*	15:currency assign					*/
		,{ 216, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.ticket_rej[4]}	/*	16:currency assign					*/
		,{ 224, (1 * SIZE_OF_1BLK), (u8 *)&Smrtdat2.serial[0]}		/*	17:currency assign					*/
	};
	#endif
#endif

/*******************************************************************************
* ICB
*
*******************************************************************************/
const u8 Smrt_Denomi_tbl[][2]={
	{  0,    0},	/* 0  該当無し  		*/
	{  1, 0x82},	/* 1              0.01  */
	{  2, 0x82},	/* 2              0.02  */
	{ 25, 0x83},	/* 3              0.025 */
	{  5, 0x82},	/* 4              0.05  */
	{  1, 0x81},	/* 5              0.1   */
	{  2, 0x81},	/* 6              0.2   */
	{ 25, 0x82},	/* 7              0.25  */
	{  5, 0x81},	/* 8              0.5   */
	{  1, 0x00},	/* 9              1     */
	{  2, 0x00},	/* 10             2     */
	{  3, 0x00},	/* 11             3     */
	{  5, 0x00},	/* 12             5     */
	{  1, 0x01},	/* 13            10     */
	{  2, 0x01},	/* 14            20     */
	{ 25, 0x00},	/* 15            25     */
	{  3, 0x01},	/* 16            30     */
	{  5, 0x01},	/* 17            50     */
	{  1, 0x02},	/* 18           100     */
	{  2, 0x02},	/* 19           200     */
	{ 25, 0x01},	/* 20           250     */
	{  5, 0x02},	/* 21           500     */
	{  1, 0x03},	/* 22         1,000     */
	{  2, 0x03},	/* 23         2,000     */
	{ 25, 0x02},	/* 24         2,500     */
	{  5, 0x03},	/* 25         5,000     */
	{  1, 0x04},	/* 26        10,000     */
	{  2, 0x04},	/* 27        20,000     */
	{ 25, 0x03},	/* 28        25,000     */
	{  5, 0x04},	/* 29        50,000     */
	{  1, 0x05},	/* 30       100,000     */
	{  2, 0x05},	/* 31       200,000     */
	{ 25, 0x04},	/* 32       250,000     */
	{  5, 0x05},	/* 33       500,000     */
	{  1, 0x06},	/* 34     1,000,000     */
	{  2, 0x06},	/* 35     2,000,000     */
	{ 25, 0x05},	/* 36     2,500,000     */
	{  5, 0x06},	/* 37     5,000,000     */
	{  1, 0x07},	/* 38    10,000,000     */
	{  2, 0x07},	/* 39    20,000,000     */
	{ 25, 0x06},	/* 40    25,000,000     */
	{  5, 0x07},	/* 41    50,000,000     */
	{  1, 0x08},	/* 42   100,000,000     */
	{  2, 0x08},	/* 43   200,000,000     */
	{ 25, 0x07},	/* 44   250,000,000     */
	{  5, 0x08},	/* 45   500,000,000     */
	{  1, 0x09},	/* 46 1,000,000,000     */
	{  0,    0},	/* 47 該当無し  		*/
};


/*******************************************************************************
* ICB Settings
*
*******************************************************************************/
extern const u8 smrt_id;
extern const u8 smrt_assign;
extern const u8 smrt_assign2;
extern const u8 smrt_assign3;
extern const u8 smrt_assign4;
extern const u8 smrt_assign5;

#define TIMER_RTC_UNIX_SECONDS_OF_LEAP_YEAR		366*24*60*60
#define TIMER_RTC_UNIX_SECONDS_OF_NOR_YEAR		365*24*60*60
#define TIMER_TIC_UNIX_SECONDS_OF_4_YEAR		(TIMER_RTC_UNIX_SECONDS_OF_LEAP_YEAR + TIMER_RTC_UNIX_SECONDS_OF_NOR_YEAR * 3)
#define TIMER_RTC_UNIX_SECONDS_OF_DAY			24*60*60
#define TIMER_RTC_UNIX_SECONDS_OF_HOUR			60*60
#define TIMER_RTC_UNIX_SECONDS_OF_MIN			60
#define TIMER_RTC_UNIX_ORIGINAL					1980
static int dayOfMonth[2][12] = { { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
								 { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } };

/*  X,X,2,X,4,X,6,7,8,9,10,X,X,13,14,15 */
static const u8 rejtbl[16] = {0,0,1,0,2,0,3,4,5,6, 7,0,0, 8, 9,10};
/*static const u8 ticket_rejtbl[16] = {0,1,2,0,3,4,0,0,5,6,7,8,0,0,0,0};	*/
/*static const u8 ticket_rejtbl[16] = {0,1,2,0,3,4,0,2,5,6,7,8,2,0,0,0};	*/
			/*	ICBのメモリはないため
					BAR_LIMIT_LENGTH_DIFFを#2に新規対応
					BAR_LIMIT_WIDE_NARROWを#2に新規対応
			に変更
			*/
/*  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 */
/*  X,1,2,X,4,4,X,X,5,6, 7, 8,XX,XX,XX,XX */
static const u8 ticket_rejtbl[16] = {0,1,2,0,3,4,0,0,5,6,7,8,0,0,0,0};
			/* '12-12-18
					BAR_SHの返却を#3に追加（新規）
					BAR_STの返却をBAR_SPと共通の#4に変更
			*/


#if 0 //2024-06-18 not use
/********************************************************************************/
/*              閏年チェック・テーブル                                          */
/********************************************************************************/
static const u8	monday[2][12] =
{ /*  1  2  3  4  5  6  7  8  9 10 11 12月							*/
	{31,29,31,30,31,30,31,31,30,31,30,31},	/* 閏年					*/
	{31,28,31,30,31,30,31,31,30,31,30,31}	/* 閏年以外				*/
};

/********************************************************************************/
/*              元旦（１／１）日数オフセットテーブル                            */
/*              ２０００年～２０９９年                                          */
/********************************************************************************/
static const u32	year_tbl[100] =
	{   0L,		/*** 2000:366	*/
	  366L,		/*   2001:365	*/
	  731L,		/*   2002:365	*/
	 1096L,		/*   2003:365	*/
	 1461L,		/*** 2004:366	*/
	 1827L,		/*   2005:365	*/
	 2192L,		/*   2006:365	*/
	 2557L,		/*   2007:265	*/
	 2922L,		/*** 2008:366	*/
	 3288L,		/*   2009:365	*/
	 3653L,		/*   2010:365	*/
	 4018L,		/*   2011:365	*/
	 4383L,		/*** 2012:366	*/
	 4749L,		/*   2013:365	*/
	 5114L,		/*   2014:365	*/
	 5479L,		/*   2015:365	*/
	 5844L,		/*** 2016:366	*/
	 6210L,		/*   2017:365	*/
	 6575L,		/*   2018:365	*/
	 6940L,		/*   2019:365	*/
	 7305L,		/*** 2020:366	*/
	 7671L,		/*   2021:365	*/
	 8036L,		/*   2022:365	*/
	 8401L,		/*   2023:365	*/
	 8766L,		/*** 2024:366	*/
	 9132L,		/*   2025:365	*/
	 9497L,		/*   2026:365	*/
	 9862L,		/*   2027:365	*/
	10227L,		/*** 2028:366	*/
	10593L,		/*   2029:365	*/
	10958L,		/*   2030:365	*/
	11323L,		/*   2031:365	*/
	11388L,		/*** 2032:366	*/
	12054L,		/*   2033:365	*/
	12419L,		/*   2034:365	*/
	12784L,		/*   2035:365	*/
	13149L,		/*** 2036:366	*/
	13515L,		/*   2037:365	*/
	13880L,		/*   2038:365	*/
	14245L,		/*   2039:365	*/
	14610L,		/*** 2040:366	*/
	14976L,		/*   2041:365	*/
	15341L,		/*   2042:365	*/
	15706L,		/*   2043:365	*/
	16071L,		/*** 2044:366	*/
	16437L,		/*   2045:365	*/
	16802L,		/*   2046:365	*/
	17167L,		/*   2047:365	*/
	17532L,		/*** 2048:366	*/
	17898L,		/*   2049:365	*/
	18263L,		/*   2050:365	*/
	18628L,		/*   2051:365	*/
	18993L,		/*** 2052:366	*/
	19359L,		/*   2053:365	*/
	19724L,		/*   2054:365	*/
	20089L,		/*   2055:365	*/
	20454L,		/*** 2056:366	*/
	20820L,		/*   2057:365	*/
	21185L,		/*   2058:365	*/
	21550L,		/*   2059:365	*/
	21915L,		/*** 2060:366	*/
	22281L,		/*   2061:365	*/
	22646L,		/*   2062:365	*/
	23011L,		/*   2063:365	*/
	23376L,		/*** 2064:366	*/
	23742L,		/*   2065:365	*/
	24107L,		/*   2066:365	*/
	24472L,		/*   2067:365	*/
	24837L,		/*** 2068:366	*/
	25203L,		/*   2069:365	*/
	25568L,		/*   2070:365	*/
	25933L,		/*   2071:365	*/
	26298L,		/*** 2072:366	*/
	26664L,		/*   2073:365	*/
	27029L,		/*   2074:365	*/
	27394L,		/*   2075:365	*/
	27759L,		/*** 2076:366	*/
	28125L,		/*   2077:365	*/
	28490L,		/*   2078:365	*/
	28855L,		/*   2079:365	*/
	29220L,		/*** 2080:366	*/
	29586L,		/*   2081:365	*/
	29951L,		/*   2082:365	*/
	30316L,		/*   2083:365	*/
	30681L,		/*** 2084:366	*/
	31047L,		/*   2085:365	*/
	31412L,		/*   2086:365	*/
	31777L,		/*   2087:365	*/
	32142L,		/*** 2088:366	*/
	32508L,		/*   2089:365	*/
	32873L,		/*   2090:365	*/
	33238L,		/*   2091:365	*/
	33603L,		/*** 2092:366	*/
	33969L,		/*   2093:365	*/
	34334L,		/*   2094:365	*/
	34699L,		/*   2095:365	*/
	35064L,		/*** 2096:366	*/
	35430L,		/*   2097:365	*/
	35795L,		/*   2098:365	*/
	36160L };	/*   2099:365	*/


/********************************************************************************/
/*      PRGM-NAME   : uru_check                                                 */
/*      FUNCTION    :  閏年のチェック                                           */
/*      CALLING-SEQ                                                             */
/*          input   : year        年データ                                      */
/*          output  : info        結果                                          */
/*                    0 : 閏年である                                            */
/*                    1 : 閏年でない                                            */
/********************************************************************************/
static u32	uru_check(u32 year)
{
	if( ((year%100) == 0) && ((year%400) != 0))
		return 1;
	if( ((year%4) != 0))
		return 1;
	return 0;
}
#endif

static u8 TimerRTCUnixCheckYearLeap(u32 year)
{
	if ((year) % 4)
	{
		return 0;
	}
	return 1;
}

static u32 TimerRTCCvtSecondsToMonth(u32 *time, u8 leapYear)
{
	u32 id = 0;
	u32 tmp_time = *time;
	while (tmp_time > (dayOfMonth[leapYear][id] * (TIMER_RTC_UNIX_SECONDS_OF_DAY)))
	{
		tmp_time -= (dayOfMonth[leapYear][id] * (TIMER_RTC_UNIX_SECONDS_OF_DAY));
		id++;
	}
	*time = tmp_time;
	return (id + 1);
}

static u32 TimerRTCCvtMonthToDays(u32 month, u32 leapYear)
{
	u32 days = 0;

	for (u32 i = 0; i < month - 1; i++)
	{
		days += dayOfMonth[leapYear][i];
	}

	return days;
}

RTC_INFO Convert32bitToTimeUnix(u32 time_32bit)
{
	RTC_INFO time_unix;
	u32 leapYear = 0;
	u32 time_redudent;

	time_unix.year = ((time_32bit / (TIMER_TIC_UNIX_SECONDS_OF_4_YEAR)) * 4) +
						(time_32bit % (TIMER_TIC_UNIX_SECONDS_OF_4_YEAR)) / (TIMER_RTC_UNIX_SECONDS_OF_NOR_YEAR)
						+TIMER_RTC_UNIX_ORIGINAL;
	if (TimerRTCUnixCheckYearLeap(time_unix.year))
	{
		leapYear = 1;
	}
	else
	{
		leapYear = 0;
	}
	time_redudent = ((time_32bit % (TIMER_TIC_UNIX_SECONDS_OF_4_YEAR)) % (TIMER_RTC_UNIX_SECONDS_OF_NOR_YEAR));
	time_unix.month = TimerRTCCvtSecondsToMonth(&time_redudent, leapYear);

	time_unix.day = (time_redudent / (TIMER_RTC_UNIX_SECONDS_OF_DAY)) + 1;
	time_redudent -= (time_unix.day - 1) * (TIMER_RTC_UNIX_SECONDS_OF_DAY);
	time_unix.hour = time_redudent / (TIMER_RTC_UNIX_SECONDS_OF_HOUR);
	time_redudent -= time_unix.hour * (TIMER_RTC_UNIX_SECONDS_OF_HOUR);
	time_unix.minute = time_redudent / (TIMER_RTC_UNIX_SECONDS_OF_MIN);
	time_redudent -= time_unix.minute * (TIMER_RTC_UNIX_SECONDS_OF_MIN);
	time_unix.second = time_redudent;
	return time_unix;
}

u32 ConvertTimeUnixTo32bit(RTC_INFO time_unix)
{
	u32 time_32bit;

	u32 day;
	u32 leapYear;

	if (TimerRTCUnixCheckYearLeap(time_unix.year))
	{
		leapYear = 1;
	}
	else
	{
		leapYear = 0;
	}

	time_32bit = 0;

	day = ( (((time_unix.year - TIMER_RTC_UNIX_ORIGINAL) /4) *(366 + 365*3)) ) +(((time_unix.year - TIMER_RTC_UNIX_ORIGINAL) % 4) * 365) +
				TimerRTCCvtMonthToDays(time_unix.month, leapYear) + time_unix.day - 1;
	time_32bit = day * (TIMER_RTC_UNIX_SECONDS_OF_DAY)+
			time_unix.hour * (TIMER_RTC_UNIX_SECONDS_OF_HOUR)+
			time_unix.minute *(TIMER_RTC_UNIX_SECONDS_OF_MIN)	+
			time_unix.second;

	return time_32bit;
}


#if 0 //2024-06-18 not use
/****************************************************************/
/**
 * @brief シリアル時間の作成 処理
 * @param[in] RTC_INFO	:	日付
 * @param[out] none
 * @return none
 */
/****************************************************************/
u32		make_serial_time(RTC_INFO *_date)
{
	u16	uru;
	u16	ii;
	u16	idx;
	u32	time;
/***/
	time = 0L;
	if (_date->month > 1)								/* １月以外 */
	{
		uru = uru_check(_date->year);					/* 閏年情報確定 */
		for (ii=0; ii<(_date->month-1); ii++)
		{
			time += (u32)monday[uru][ii];				/* 前月までの日数加算 */
		}
	}
	time += (u32)(_date->day - 1);						/* 当月の日数加算 */
	idx = _date->year - 2000;
	time += year_tbl[idx];								/* 年オフセット日数加算 */
/*	R/W Toolのシステムが1980からのため
	1980から2000年までの日数を加算
*/
	time += ((366+365+365+365) * ((2000 - 1980) / 4));
/**/
	time *= 86400L;										/* 24H * 60M * 60S */
/*#*/
	time += (((u32)_date->hour) * 3600L);				/* 60M * 60S */
	time += (((u32)_date->minute) * 60L);				/* 60S */
	time += (u32)(_date->second);
/*#*/
	return time;
}


int date_check(RTC_INFO *_date)
{
	u32 uru;
	u32 info;
/***/
	if ((_date->year < 1980)
		|| (_date->year > 2099)							/* 年データ異常			*/
		|| (_date->month < 1)
		|| (_date->month > 12)							/* 月データ異常			*/
		|| (_date->day < 1))							/* 日データ異常			*/
	{
		info = 1;
	}
	else
	{
		uru = uru_check(_date->year);
		if ((_date->day > monday[uru][_date->month-1])	/* 日データ異常			*/
			|| (_date->hour > 23)						/* 時データ異常			*/
			|| (_date->minute > 59))					/* 分データ異常			*/
		{
			info = 1;
		}
		else
		{
			info = 0;									/* 日付データ正常		*/
		}
	}
	return info;
};
#endif

void set_box_flag(enum _ICB_FLAG flg_value) //use SS and RTQ
{
	Smrtdat.flg = flg_value;
}
bool is_box_flag_no_data(void) //use SS and RTQ
{
	if(Smrtdat.flg==ICB_NO_DATA)
	{
		return true;
	}
	return false;
}
bool is_box_flag_installed(void) //use SS and RTQ
{
	if(Smrtdat.flg==ICB_INSTALLED)
	{
		return true;
	}
	return false;
}

#if !defined(UBA_RTQ)
bool is_box_flag_data_exist(void)
{
	if(Smrtdat.flg==ICB_DATA_EXIST)
	{
		return true;
	}
	return false;
}
#endif

bool is_box_flag_read(void)	//use SS and RTQ
{
	if(Smrtdat.flg==ICB_DATA_HAS_BEEN_READ)
	{
		return true;
	}
	return false;
}
bool is_box_flag_initial_request(void) //use SS and RTQ
{
	if(Smrtdat.flg==ICB_AUTO_INITIAL_REQUEST)
	{
		return true;
	}
	return false;
}
bool is_box_flag_inhibit(void) //use SS and RTQ 使用するか未定だが残す
{
	if(Smrtdat.flg==ICB_SYSTEM_INHIBIT)
	{
		return true;
	}
	return false;
}
/*--------------------------------------------------------------
* memory fill for 8bit memory
*
*---------------------------------------------------------------
*	引数	:u8 data	(fill up data)
*			 u8 *ptr	(destnation address)
*			 u32 size	(memory size)
*---------------------------------------------------------------
*	戻値	:non
*-------------------------------------------------------------*/
void  fill_memo( u8 data, u8 *ptr, u32 size)
{
	u32	ii;

	for(ii = 0; ii < size; ii++)
	{
		*ptr = data;
		ptr++;
	}
}


/*--------------------------------------------------------------
* memory fill for 16bit memory
*
*---------------------------------------------------------------
*	引数	:u16 data	(fill up data)
*			 u16 *ptr	(destnation address)
*			 u32 size	(memory size)
*---------------------------------------------------------------
*	戻値	:non
*-------------------------------------------------------------*/
void  fill_memo16( u16 data, u16 *ptr, u32 size)
{
	u32	ii;

	for(ii = 0; ii < size; ii++)
	{
		*ptr = data;
		ptr++;
	}
}


/*--------------------------------------------------------------
* memory fill for 8bit memory
*
*---------------------------------------------------------------
*	引数	:u32 data	(fill up data)
*			 u32 *ptr	(destnation address)
*			 u32 size	(memory size)
*---------------------------------------------------------------
*	戻値	:non
*-------------------------------------------------------------*/
void  fill_memo32( u32 data, u32 *ptr, u32 size)
{
	u32	ii;

	for(ii = 0; ii < size; ii++)
	{
		*ptr = data;
		ptr++;
	}
}


/*--------------------------------------------------------------
* memory copy
*
*---------------------------------------------------------------
*	引数	:u16 *dptr	(destnation address)
*			 u16 *sptr	(source address)
*			 u16 size	(memory size)
*---------------------------------------------------------------
*	戻値	:non
*-------------------------------------------------------------*/
void memo_copy( u8 *dptr, u8 *sptr, u16 size )
{
	u16	ii;

	for(ii = 0; ii < size; ii++)
	{
		*dptr = *sptr;
		dptr++;
		sptr++;
	}
}

extern const u8 icb_custom_currency_table[20][3];
static void icb_set_custom_currency_assign(void) //use SS and RTQ
{
	int	denomination_count = 0;
	int denomi_table_count = 0;
	int	base = 0;
	int exponent = 0;

	Smrtdat2.assign2 = smrt_assign2;
	Smrtdat2.assign3 = smrt_assign3;
	Smrtdat2.assign4 = smrt_assign4;
	Smrtdat2.assign5 = smrt_assign5;

	for(denomination_count = 0; denomination_count < 20; denomination_count++)
	{
		base = icb_custom_currency_table[denomination_count][1];
		exponent = icb_custom_currency_table[denomination_count][2];

		if(base == 0)
		{
			continue;
		}

		//convert the current denomination's base and exponent
		while((base / 10) != 0)
		{
			if((base % 10) != 0)
			{
				break;
			}
			else
			{
				base = base / 10;
				exponent = (exponent & 0x80) ? (exponent - 1) : (exponent + 1);
			}
		}

		//match the base and exponent with the amounts in the icb denomination table
		for(denomi_table_count = 1; denomi_table_count < (sizeof(Smrt_Denomi_tbl) / 2); denomi_table_count++)
		{
			if((Smrt_Denomi_tbl[denomi_table_count][0] == base) && (Smrt_Denomi_tbl[denomi_table_count][1] == exponent))
			{
				break;
			}
		}

		if(denomi_table_count != 0) //if the current base and exponent are found in the icb denomination table
		{
			Smrtdat2.crncy[denomination_count] = (u8)((icb_custom_currency_table[denomination_count][0] << 6) | (denomi_table_count & 0x3f));
		}
	}
}

/*----------------------------------------------------------------------
* 	RTC機能から日時の取得処理
*
*	Carry ISRは使用していないがマニュアル通りにDisableに設定する
*	RTCが動作していない場合は、Timerの設定がないと判断し、"0"取得とする
*-----------------------------------------------------------------------
*	引数	:non
*-----------------------------------------------------------------------
*	戻値	:構造体アドレス
*---------------------------------------------------------------------*/
#if !defined(UBA_RTQ)
RTC_INFO 	get_date_from_RTC(void)
{
	return(ex_rtc_clock);
}
#endif

/*------------------------------------------------------------------*/
/*		ESCROW位置からの返却コード別返却回数更新 '06-01-20			*/
/*------------------------------------------------------------------*/
#if !defined(UBA_RTQ)
void icb_update_reject_counter(int code)
{
	u8	id;
	u8	rejcd;
	u16	work;

	if((rejcd = rejtbl[(code & 0x0f)]) == 0)
	{
		return;		/* 返却コード2,4,6,7,8,9,10,13,14,15のみ更新する。 */
	}

	id = (--rejcd) / 2;
						/*	返却ｺｰﾄﾞ  2(1) &  4(2)  -> index 0	*/
						/*  返却ｺｰﾄﾞ  6(3) &  7(4)  -> index 1	*/
						/*  返却ｺｰﾄﾞ  8(5) &  9(6)  -> index 2	*/
						/*  返却ｺｰﾄﾞ 10(7) & 13(8)  -> index 3	*/
						/*  返却ｺｰﾄﾞ 14(9) & 15(10) -> index 4	*/
/*< 返却回数更新 >*/
	if( (rejcd & 0x01) == 0 ){
	/* 返却ｺｰﾄﾞ 2,6,8,10,14時、上位１２ビット使用	*/
		work = (((u16)Smrtdat2.rej[id][0] << 4) | (((u16)Smrtdat2.rej[id][1] >> 4) & 0x0f));
		if( work < 0xfff )	work++;
		work = (work);
		Smrtdat2.rej[id][0] = (u8)(work >> 4);
 		Smrtdat2.rej[id][1] = (Smrtdat2.rej[id][1] & 0x0f) | (u8)((work << 4) & 0xf0);
	}
	else{
	/* 返却ｺｰﾄﾞ 4,7,9,13,15時、下位１２ビット使用	*/
		work = ((u16)Smrtdat2.rej[id][2] | ((u16)(Smrtdat2.rej[id][1] & 0x0f) << 8));
		if( work < 0xfff )	work++;
		work = (work);
		Smrtdat2.rej[id][1] = (Smrtdat2.rej[id][1] & 0xf0) | (u8)((work >> 8) & 0x0f);
 		Smrtdat2.rej[id][2] = (u8)(work & 0xff);
	}
	/*	'16-08-29 Add Box size*/
	set_box_size_to_ICB();
	/**/
	Smrtdat2.sum2 = (u8)culc_BLK2_checksum();			/* チェックサム更新 */
}
#endif

/*--------------------------------------*/
/*		Update Error counter			*/
/*--------------------------------------*/
#if !defined(UBA_RTQ)
int icb_update_error_counter(int code)
{
	int	num;

	if(code < 20)
	{
		if(Smrtdat.err[code] < 0xff)
		{
			Smrtdat.err[code] += 1;
			if(is_box_flag_installed()){
				set_box_flag(ICB_DATA_EXIST);				/* 集計データあり */
			}
			Smrtdat.sum = (u8)culc_BLK1_checksum();
		}
	}

#if 1//#if (_RFID_BACK_DATA28==1) //(_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
	if(code < 4)
	{
		num = BLK_ERR_NUMBER1;
	}
	else
	{
		num = BLK_ERR_NUMBER2;
	}
#else
	if(code < 8)
	{
		num = BLK_ERR12_NUMBER;
	}
	else if(code < 12)
	{
		num = BLK_ERR3_NUMBER;
	}
	else if(code < 16)
	{
		num = BLK_ERR4_NUMBER;
	}
	else
	{
		num = BLK_ERR5_NUMBER;
	}

#endif
/*<--->*/
	return(num);
}
#endif


/*--------------------------------------------------------------
* Add Cash Box Size			'16-08-29
*
*	CheckSUM2更新時にBOXサイズを書き込む
*	（追加仕様のため、運用機に対して書き込むタイミングが更新時
*     しかないため
*---------------------------------------------------------------
*	引数	:
*---------------------------------------------------------------
*	戻値	:
*-------------------------------------------------------------*/
static void	set_box_size_to_ICB(void) //use SS and RTQ
{
	Smrtdat2.box_size = 0;
}


/*------------------------------------------------------------------------------
* Set to send buffer and data BLK number of information
*
*-------------------------------------------------------------------------------
*	引数	:
*-------------------------------------------------------------------------------
*	戻値	:
*-----------------------------------------------------------------------------*/
#if !defined(UBA_RTQ)
void icb_send_buffer_initial_info(void)
{
	u8	ii;

	for(ii = ICB_SUB_INITIAL_WRITE_MCNo; ii < ICB_SUB_INITIAL_WRITE_END; ii++)
	{
		/*	get BLK number and Set Write data to Buffer	*/
		(void)set_ICBrecovery_data(icb_BLKnumber(ii));
	}
}

/*--------------------------------------------------------------
* ICBイニシャル設定時のBLK番号の取得処理
*
*---------------------------------------------------------------
*	引数	:
*---------------------------------------------------------------
*	戻値	:BLK番号
*-------------------------------------------------------------*/
static int icb_BLKnumber(u8 req)
{
	int	num;

	switch(req)
	{
	case	ICB_SUB_INITIAL_WRITE_MCNo:
				num = BLK_MC_NUMBER;
				break;
	case	ICB_SUB_INITIAL_WRITE_VER:
				num = BLK_VER_NUMBER;
				break;
	case	ICB_SUB_INITIAL_WRITE_INFO:
				num = BLK_SUM_NUMBER;
				break;
#if 1//#if (_RFID_BACK_DATA28==1) //(_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
	case	ICB_SUB_INITIAL_WRITE_CRENCY1:
				num = BLK_CRENCY_NUMBER1;
				break;
	case	ICB_SUB_INITIAL_WRITE_CRENCY2:
				num = BLK_CRENCY_NUMBER2;
				break;
#else
	case	ICB_SUB_INITIAL_WRITE_CRENCY:
				num = BLK_CRENCY_NUMBER;
				break;
#endif
	case	ICB_SUB_INITIAL_WRITE_MODEL:
				num = BLK_MODEL_NUMBER;		/*	model	*/
				break;
	case	ICB_SUB_INITIAL_WRITE_SERIAL:
				num = BLK_SERIAL_NUMBER;	/*	Serial# + sum	*/
				break;
	default:
				num = BLK_DUMMY1_NUMBER;
				break;
	}
/*<--->*/
	return(num);
}

/*--------------------------------------------------------------
* Update of Total Insert counter
*
*	紙幣の受け取りのみ更新する(メモリ上のみ)
*---------------------------------------------------------------
*	引数	:
*---------------------------------------------------------------
*	戻値	: non
*-------------------------------------------------------------*/
void	icb_totalInsert_counter(void)
{
	u16	data;

	data = SWAP_ENDIAN16(Smrtdat.totalin);
	if(data < 0xffff)
	{
		data += 1;
		Smrtdat.totalin = SWAP_ENDIAN16((u16)data);
		if(is_box_flag_installed()){
			set_box_flag(ICB_DATA_EXIST);				/* 集計データあり */
		}
		Smrtdat.sum = (u8)culc_BLK1_checksum();
	}
}

/*--------------------------------------*/
/*		Update denomi counter			*/
/*--------------------------------------*/
int icb_update_denomi_counter(u8 ex_denomi)
{
	u16	data;
	int	num;
	int	denom;

	extern const u8 icb_custom_denom_assign[];

	denom = icb_custom_denom_assign[ex_denomi] + 1;

	data = SWAP_ENDIAN16(Smrtdat.denomi[denom - 1]);

	if(data < 0xffff)
	{
		data += 1;

		Smrtdat.denomi[denom - 1] = SWAP_ENDIAN16((u16)data);

		if(is_box_flag_installed()){
			set_box_flag(ICB_DATA_EXIST);				/* 集計データあり */
		}
		Smrtdat.sum = (u8)culc_BLK1_checksum();
	}
#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) // TUNG CHANGE 2023-10-13
	num = ((denom - 1) / 4) + 1;
#else
	num = (denom + 1) / 2;
#endif
/*<--->*/
	return(num);
}

/*--------------------------------------*/
/*		Update ticket counter			*/
/*--------------------------------------*/
int icb_update_ticket_counter(void)
{
	u16	data;
	int	num;

	data = SWAP_ENDIAN16(Smrtdat.coupon);
	if(data < 0xffff)
	{
		data += 1;
		Smrtdat.coupon = SWAP_ENDIAN16((u16)data);
		if(is_box_flag_installed()){
			set_box_flag(ICB_DATA_EXIST);				/* 集計データあり */
		}
		Smrtdat.sum = (u8)culc_BLK1_checksum();
	}
	num = BLK_INSERT_BILL_NUMBER;

/*<--->*/
	return(num);
}

/*--------------------------------------*/
/*		Update total counter			*/
/*--------------------------------------*/
int icb_update_total_counter(void)
{
	u16	data;

	data = SWAP_ENDIAN16(Smrtdat.total);
	if(data < 0xffff)
	{
		data += 1;
		Smrtdat.total = SWAP_ENDIAN16((u16)data);
		if(is_box_flag_installed()){
			set_box_flag(ICB_DATA_EXIST);				/* 集計データあり */
		}
		Smrtdat.sum = (u8)culc_BLK1_checksum();
	}
/*<--->*/
	return(BLK_INSERT_BILL_NUMBER);
}

/*--------------------------------------*/
/*		Update Ticket number			*/
/*--------------------------------------*/
int icb_update_ticket_number(void)
{
	int	ii;

	if(ex_BAR_length[0] >= sizeof(Smrtdat.cinfo))
	{
		ii = sizeof(Smrtdat.cinfo);
	}
	else
	{
		ii = ex_BAR_length[0];
	}
	memo_copy(Smrtdat.cinfo, ICBBarcode, ii);
	for(; ii < sizeof(Smrtdat.cinfo); ii++)
	{
		Smrtdat.cinfo[ii] = ' ';
	}
	if(is_box_flag_installed()){
		set_box_flag(ICB_DATA_EXIST);				/* 集計データあり */
	}
	Smrtdat.sum = (u8)culc_BLK1_checksum();
/*<--->*/
	return(BLK_LAST_TICKET_NUMBER);
}
#endif

int icb_machine_number_is_valid(void) //use SS and RTQ
{
	int count = 0;

	//check for all blanks
	for(count = 0; count < 14; count++)
	{
		if(ex_ICB_gameno[count] != ' ')
		{
			break;
		}

		if(count == 13)
		{
			return FALSE;
		}
	}

	return TRUE;
}


/*<<	check ICB Setting mode 		>>*/
int check_ICBflag(void) //use SS and RTQ
{
#if ICB_SUB_FUNCTION == ICB_SELECT_BY_TICKET
	int	result;

	if((ICBTicket[0] == 'I')			/*	ICB_Ticket[0] ----> ICBTicket[0] '06-02-06 */
	&& (ICBTicket[1] == 'T')			/*	ICB_Ticket[1] ----> ICBTicket[1] '06-02-06 */
	&& (ICBTicket[2] == 'E')			/*	ICB_Ticket[2] ----> ICBTicket[2] '06-02-06 */
	&& (ICBTicket[3] == 'n')			/*	ICB_Ticket[3] ----> ICBTicket[3] '06-02-06 */
	&& (ICBTicket[4] == 'a')			/*	ICB_Ticket[4] ----> ICBTicket[4] '06-02-06 */
	&& (ICBTicket[5] == 'b')			/*	ICB_Ticket[5] ----> ICBTicket[5] '06-02-06 */
	&& (ICBTicket[6] == 'l')			/*	ICB_Ticket[6] ----> ICBTicket[6] '06-02-06 */
	&& (ICBTicket[7] == 'e')			/*	ICB_Ticket[7] ----> ICBTicket[7] '06-02-06 */
	){
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}
/*<--->*/
	return(result);
#else
	return(TRUE);
#endif
}

/*------------------------------------------------------------------
* Verification process of recovery data		(N100-180-01 '12-07-01)
*
* 有効データが無い条件：
*	Recovery dataポインタと送信ポインタが一致している場合
*	Recovery dataのブロック番号が一致していない。又はない
*	Recovery dataポインタと送信ポインタが範囲以外を示している
*-------------------------------------------------------------------
*	引数	:
*-------------------------------------------------------------------
*	戻値	:TRUE				 	:Non recovery data
*		 FALSE					:valid recovery data
*-----------------------------------------------------------------*/
#if !defined(UBA_RTQ)
BOOL icb_check_recovery_data(void)
{

	int num;

	if(ex_ICBsend_num == ex_ICBsave_num)
	{
	/*	non recovery data	*/
		return(TRUE);
	}
	if(is_ICBsend_flag_broken())
	{
		return(TRUE);
	}
/*<<	copy recovery data		>>*/
	num = (int)ex_ICBrecovery[ex_ICBsend_num].BLK;
	/*	Checksumは、古い場合があるため再計算処理を入れる	*/
	if(num == BLK_SUM_NUMBER
	|| num == BLK_SUM2_NUMBER
	){
		if(num == BLK_SUM_NUMBER)
		{
			Smrtdat.sum = (u8)culc_BLK1_checksum();
		}
		else
		{
			Smrtdat2.sum2 = (u8)culc_BLK2_checksum();	/* チェックサム更新 */
		}
		memo_copy(&ex_ICBrecovery[ex_ICBsave_num].data[0]
					, send_blk_info[num - 1].buffer
					, send_blk_info[num - 1].size
					);
	}

	memo_copy(send_blk_info[num - 1].buffer
			, &ex_ICBrecovery[ex_ICBsend_num].data[0]
			, send_blk_info[num - 1].size
			);
	return(FALSE);
}

static bool	is_ICBsend_flag_broken(void)
{
	if((u8)ex_ICBrecovery[ex_ICBsend_num].BLK != (u8)(~ex_ICBrecovery[ex_ICBsend_num]._BLK)
	|| ex_ICBrecovery[ex_ICBsend_num].BLK == NULL	/*	data broken	*/
	|| ex_ICBrecovery[ex_ICBsend_num]._BLK == NULL	/*	data broken	*/
	|| ex_ICBsend_num >= ICB_MAX_BACK_COUNTER		/*	data broken	*/
	|| ex_ICBsave_num >= ICB_MAX_BACK_COUNTER		/*	data broken	*/
	/*	broken recovery data	*/
	){
		return(true);
	}
	return(false);
}

void	renewal_ICBsend_flag(void)
{

/*	recovery flag clear of send buffer	*/
	ex_ICBrecovery[ex_ICBsend_num]._BLK = NULL;
/*	check next send buffer	*/
	ex_ICBsend_num += 1;
	if(ex_ICBsend_num >= ICB_MAX_BACK_COUNTER)
	{
		/* Box初期化の時、最大送信回数は30なので、ex_ICBsave_num = 30となる、上記の条件は			*/
		/* 0スタートのバッファex_ICBrecovery[]の保護の為に、ex_ICBsave_num == 30の時に0にしている	*/
		/* ただし、_icb_initial_0134_seq の時、この関数を抜けた抜けた所で							*/
		/* 送信完了かの、比較のif(ex_ICBsend_num == ex_ICBsave_num) を行っているので、 				*/
		/* この中でクリアしてしまうと、送信完了にならない											*/
		/* このでのクリアを止めるとリスクがあるかもしれないので、_icb_initial_0134_seqでの送信完了条件を変更した*/
		ex_ICBsend_num = 0;
	}
}
#endif

/* return version 'V' address in software_ver */
u8 icb_get_version_address(void) //use SS and RTQ
{
	int	ii;

	//"ID-"を見つけて、その後のVを見つける方が確実
	for(ii = 0; ii < 64; ii++)
	{
		if(software_ver[ii] == 'I' && software_ver[ii+1] == 'D' && software_ver[ii+2] == '-')
		{
			ii = ii + 3;
			break;
		}
	}

	for(; ii < 64; ii++)
	{
		if(software_ver[ii] == 'V')
		{
			break;
		}
	}

	return ii;
}
/*<<		Registration of the software information of this unit   		>>*/
void icb_set_initial_info(void) //use SS and RTQ
{

	int	ii;

	ii = icb_get_version_address();

	memo_copy( Smrtdat.gameno, ex_ICB_gameno, 20);
	memo_copy( Smrtdat.ver, (u8 *)&software_ver[ii], 8);
 
 	Smrtdat.assign = smrt_assign;
	Smrtdat.id = smrt_id;
	set_box_flag(ICB_INSTALLED);
	Smrtdat.sum = (u8)culc_BLK1_checksum();
	memo_copy(ex_ICB_boxno, Smrtdat.boxno, 20);
	icb_set_custom_currency_assign();	/* Currency Assign Table Set */
	icb_set_manufacture_info();			/* Set Model & Serial#	*/
	/*	'16-08-29 Add Box size*/
	set_box_size_to_ICB();
	/**/
	Smrtdat2.sum2 = (u8)culc_BLK2_checksum();	/* チェックサム更新 */

}


int is_icb_checksum_error(void) //use SS and RTQ
{

	int result;
	result = culc_BLK1_checksum();
	/*<<	Check BLK1 sum 		>>*/
#if defined(UBA_RTQ)
	if((u8)(result & 0xff) != Smrtdat.sum)
#else
	if((u8)(result & 0xff) != Smrtdat.sum
	&& (ex_Info_ICBrecovery & BIT_INITIAL_DATA_WRITING) == 0	/* ｲﾆｼｬﾙﾃﾞｰﾀ書き込み中のHard Reset対策	 */
	)
#endif
	{
	#if !defined(UBA_RTQ)
		ex_icb_state |= BIT_ICB_DATA_SUM_ERROR;	/*	ICB	Data error			*/
	#endif
		return(ALARM_CODE_RFID_ICB_DATA);
	}
	result = culc_BLK2_checksum();
	/*<<	Check BLK2 sum 		>>*/
#if defined(UBA_RTQ)
	if((u8)(result & 0xff) != Smrtdat2.sum2)
#else
	if((u8)(result & 0xff) != Smrtdat2.sum2
	&& (ex_Info_ICBrecovery & BIT_INITIAL_DATA_WRITING) == 0	/* ｲﾆｼｬﾙﾃﾞｰﾀ書き込み中のHard Reset対策	 */
	)
#endif
	{
	#if !defined(UBA_RTQ)
		ex_icb_state |= BIT_ICB_DATA_SUM_ERROR;	/*	ICB	Data error			*/
	#endif
		return(ALARM_CODE_RFID_ICB_DATA);
	}
#if !defined(UBA_RTQ)	
	ex_icb_state &= ~BIT_ICB_DATA_SUM_ERROR;	/*	ICB	Data error			*/
#endif

	return 0;
}
int	culc_BLK1_checksum(void) //use SS and RTQ
{
	int	ii;
	int	sum;
	u8	*ptr;

	sum = 0;
	ptr = (u8 *)Smrtdat.denomi;
	for(ii = 0; ii < sizeof(Smrtdat.denomi); ii++)
	{
		sum += *(ptr + ii);
	}
	sum += (u8)(Smrtdat.denomi_reserved >> 8);
	sum += (u8)(Smrtdat.denomi_reserved & 0xff);
	sum += (u8)(Smrtdat.total >> 8);
	sum += (u8)(Smrtdat.total & 0xff);
	sum += (u8)(Smrtdat.coupon >> 8);
	sum += (u8)(Smrtdat.coupon & 0xff);
	sum += (u8)(Smrtdat.totalin >> 8);
	sum += (u8)(Smrtdat.totalin & 0xff);
	for(ii = 0; ii < sizeof(Smrtdat.cinfo); ii++)
	{
		sum += Smrtdat.cinfo[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat.err); ii++)
	{
		sum += Smrtdat.err[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat.gameno); ii++)
	{
		sum += Smrtdat.gameno[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat.boxno); ii++)
	{
		sum += Smrtdat.boxno[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat.ver); ii++)
	{
		sum += Smrtdat.ver[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat.rw_ver); ii++)
	{
		sum += Smrtdat.rw_ver[ii];
	}
	ptr = (u8 *)&Smrtdat.restim;
	sum += *ptr;
	sum += *(ptr + 1);
	sum += *(ptr + 2);
	sum += *(ptr + 3);

	ptr = (u8 *)&Smrtdat.settim;
	sum += *ptr;
	sum += *(ptr + 1);
	sum += *(ptr + 2);
	sum += *(ptr + 3);

	ptr = (u8 *)&Smrtdat.initim;
	sum += *ptr;
	sum += *(ptr + 1);
	sum += *(ptr + 2);
	sum += *(ptr + 3);

	sum += Smrtdat.flg;
	sum += Smrtdat.assign;
	sum += Smrtdat.id;

/*<--->*/
	return(sum ^ 0xff);
}

static int	culc_BLK2_checksum(void) //use SS and RTQ
{
	int	ii;
	int	sum;
	u8	*ptr;

	sum = 0;
	ptr = (u8 *)Smrtdat2.rej[0];
	for(ii = 0; ii < sizeof(Smrtdat2.rej); ii++)
	{
		sum += *(ptr + ii);
	}
	sum += Smrtdat2.rej_dummy;
	for(ii = 0; ii < sizeof(Smrtdat2.crncy); ii++)
	{
		sum += Smrtdat2.crncy[ii];
	}
	sum += Smrtdat2.assign2;
	sum += Smrtdat2.assign3;
	sum += Smrtdat2.assign4;
	sum += Smrtdat2.assign5;
	for(ii = 0; ii < sizeof(Smrtdat2.ticket_rej); ii++)
	{
		sum += Smrtdat2.ticket_rej[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat2.model); ii++)
	{
		sum += Smrtdat2.model[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat2.serial); ii++)
	{
		sum += Smrtdat2.serial[ii];
	}
	/*	'16-08-29 Add Box size*/
	//sum += Smrtdat2.reserved0;
	sum += Smrtdat2.box_size;
/*<--->*/
	return(sum ^ 0xff);
}

int icb_check_machineNo(void) //use SS and RTQ
{
	int	result = 0;
	int	ii;

	for(ii = 0; ii < 20; ii++)
	{
		if((Smrtdat.gameno[ii] != ' ') || (ex_ICB_gameno[ii] != ' '))
		{
			/* ICB側のマシン番号 != Head側のマシン番号 */
			if(Smrtdat.gameno[ii] != ex_ICB_gameno[ii])
			{
				result = ALARM_CODE_RFID_ICB_NUMBER_MISMATCH;	/* 別のゲーム機にセットされていたＢＯＸです */
				break;
			}
		}
	}
	return(result);
}

/*--------------------------------------------------------------
* set write data info
*	copy BLK number for recovery
*	make checksum data
*---------------------------------------------------------------
*	引数	:
*---------------------------------------------------------------
*	戻値	: TRUE		:currect
*			  FALSE		:buffer over
*-------------------------------------------------------------*/
#if !defined(UBA_RTQ)
int set_ICBrecovery_data(int num)
{
	int	result = FALSE;

/*<<	set recovery data		>>*/
	//ex_ICBrecovery[ex_ICBsave_num].BLK に送るべきデータの番号がex_ICBsave_num 順に格納
	//send_blk_info[num - 1].buffer			送るべきデータ
	//send_blk_info[num - 1].size			送るべきデータサイズ
	if(ex_ICBsave_num < ICB_MAX_BACK_COUNTER)
	{
		ex_ICBrecovery[ex_ICBsave_num].BLK = (u16)num;
		ex_ICBrecovery[ex_ICBsave_num]._BLK = ~(ex_ICBrecovery[ex_ICBsave_num].BLK);
		memo_copy(&ex_ICBrecovery[ex_ICBsave_num].data[0]
					, send_blk_info[num - 1].buffer
					, send_blk_info[num - 1].size
					);
	/*	Update buffer pointer	*/
		ex_ICBsave_num += 1;
		if(ex_ICBsave_num > ICB_MAX_BACK_COUNTER)
		/* バッファex_ICBrecovery[]は0～29なので、保護処理としてここでex_ICBsave_num == 30の場合*/
		/* ex_ICBsave_num = 0とするのは正しい*/
		/* 正送る予定のデータの最大数は30なので、ex_ICBsave_num == 30になる事はICB初期化なのであり得る*/
		/* ここでex_ICBsave_num = 0でも結果的には上手く動いていたが、本来の意味と異なるので*/
		/* クリア条件を変更する*/
		{
			ex_ICBsave_num = 0;
		}
		result = TRUE;
	}
/*<--->*/
	return(result);
}
#endif

/*----------------------------------------------------------*/
/*		ゲーム機ＮＯ．ＢＯＸＮＯ．チェックサム計算			*/
/*----------------------------------------------------------*/
u8 icb_savnosum(void) //use SS and RTQ
{
	int	ii, sum;

	sum = 0;
	for(ii = 0; ii < sizeof(ex_ICB_gameno); ii++)
	{
		sum += ex_ICB_gameno[ii];
	}

	for(ii = 0; ii < sizeof(ex_ICB_boxno); ii++)
	{
		sum += ex_ICB_boxno[ii];
	}

	sum += (u8)(Savinitim & 0xff);
	sum += (u8)((Savinitim >> 8) & 0xff);
	sum += (u8)((Savinitim >> 16) & 0xff);
	sum += (u8)((Savinitim >> 24) & 0xff);

	return((u8)(~sum));

}

void set_ICBdisable_flag(void) //use SS and RTQ
{
	icb_clear_recovery_flag(ON);
	fill_memo( 0, ex_ICB_gameno, sizeof(ex_ICB_gameno));	/* マシンＮｏ．Clear */
	fill_memo( 0, ex_ICB_boxno, sizeof(ex_ICB_boxno));		/* カセットＢＯＸのＮＯ．Clear */
	fill_memo( 0, ICBTicket, sizeof(ICBTicket) );			/* '06-02-06 */
	ex_icb_Savsum = 0;										/* CheckSum Clear */
	/*	ICB Function Flag Clear	*/
	ex_ICB_systemInhiStaus = INHIBIT_ICB;
}

void set_ICBenable_flag(void) //use SS and RTQ
{
	icb_clear_recovery_flag(ON);
	ICBTicket[0] = 'I';		/*	ICB_Ticket[0] ----> ICBTicket[0] '06-02-06 */
	ICBTicket[1] = 'T';		/*	ICB_Ticket[1] ----> ICBTicket[1] '06-02-06 */
	ICBTicket[2] = 'E';		/*	ICB_Ticket[2] ----> ICBTicket[2] '06-02-06 */
	ICBTicket[3] = 'n';		/*	ICB_Ticket[3] ----> ICBTicket[3] '06-02-06 */
	ICBTicket[4] = 'a';		/*	ICB_Ticket[4] ----> ICBTicket[4] '06-02-06 */
	ICBTicket[5] = 'b';		/*	ICB_Ticket[5] ----> ICBTicket[5] '06-02-06 */
	ICBTicket[6] = 'l';		/*	ICB_Ticket[6] ----> ICBTicket[6] '06-02-06 */
	ICBTicket[7] = 'e';		/*	ICB_Ticket[7] ----> ICBTicket[7] '06-02-06 */

	Check_Savgameno();

	/*	ICB Function Flag Clear	*/
	ex_ICB_systemInhiStaus = NULL;
	ex_icb_Savsum = icb_savnosum();							/* re-culclate of CheckSUM	*/
}


void set_MCnumber(u8 *number) //use SS and RTQ
{
	icb_clear_recovery_flag(ON);
	fill_memo( ' ', ex_ICB_gameno, sizeof(ex_ICB_gameno));		/* '01-10-12 */
	memo_copy( ex_ICB_gameno, number, BAR_MC_LNG - 2);		/* '01-10-24 */
	ex_icb_Savsum = icb_savnosum();
}

/*--------------------------------------------------------------
* clear ICB recovery flag
*	When the version of software differs at the time of power
*	supply on it initializes it.
*---------------------------------------------------------------
*	引数	: ON : ソフトウェア更新時 or ICB機能有効・無効設定時
*			  OFF: 通常PowerUP
*---------------------------------------------------------------
*	戻値	:
*-------------------------------------------------------------*/
void icb_clear_recovery_flag(int mode) //use SS
{
#if !defined(UBA_RTQ)
	int	ii;

	ex_ICBsend_num = 0;
	ex_ICBsave_num = 0;
	for(ii = 0; ii < ICB_MAX_BACK_COUNTER; ii++)
	{
		ex_ICBrecovery[ii].BLK = NULL;	/*	data clear	*/
	}
	/*	ソフトウェア更新時は初期化する	(N100-180-01 '12-07-01)*/
	if(mode == ON)
	{
		/* ｲﾆｼｬﾙﾃﾞｰﾀ書き込み中のHard Reset対策 */
		ex_Info_ICBrecovery = 0;
	}
#endif	
}

/*----------------------------------------------------------------------
* モデル名及びシリアル番号の登録				(N100-180-01 '12-07-01)
*
*	モデル名は、iVIZION-100は"iVZ"固定とする。
*	モデル名は、iVIZION-200は"iVZ"とし、JAC要望あれば変更する。
*	Serial#は、工場出荷時のに登録した12桁のASCIIコードを
*	10進に変換し6文字に変換する
*-----------------------------------------------------------------------
*	引数	:
*-----------------------------------------------------------------------
*	戻値	:
*---------------------------------------------------------------------*/
static void	icb_set_manufacture_info(void)	//use SS and RTQ
{
	int	ii;
	u8	num;

	/*	Model Nameの保存	*/
	Smrtdat2.model[0] = 'U';
	Smrtdat2.model[1] = 'B';
	Smrtdat2.model[2] = 'A';

	//2020-05-05 Added SS / SU installation information to ICB memory
	if((ex_system & BIT_SU_UNIT) == BIT_SU_UNIT)
	{
    	Smrtdat2.model[3] = ((ICB_FORMAT_REVISION & ~ICB_STYLE_BIT) | ICB_SU_STYLE);
    }
    else
    {
    	Smrtdat2.model[3] = ((ICB_FORMAT_REVISION & ~ICB_STYLE_BIT) | ICB_SS_STYLE);
    }
	/*	Serial#の保存		*/
	for(ii = 0; ii < 12; ii += 2)
	{
		num = ((ex_adjustment_data.factory_info.serial_no[ii] & 0x0f) << 4);	/*	ASCII文字を数値にし、上位に配置	*/
		num |= ex_adjustment_data.factory_info.serial_no[ii+1] & 0x0f;			/*	ASCII文字を数値にし、下位に配置	*/
		Smrtdat2.serial[ii/2] = num;
	}
}

/*------------------------------------------------------------------*/
/*		ESCROW位置からの返却コード別返却回数更新 '06-01-20			*/
/*------------------------------------------------------------------*/
/*----------------------------------------------------------------------
* Ticket識別の返却回数の登録					(N100-180-01 '12-07-01)
*
*	ESCROW位置からの返却時のみ登録とする（最大255回）
*-----------------------------------------------------------------------
*	引数	:
*-----------------------------------------------------------------------
*	戻値	:
*---------------------------------------------------------------------*/
#if !defined(UBA_RTQ)
void icb_update_ticket_reject_counter(int code)
{
	u8	id;

	if((id = ticket_rejtbl[(code & 0x0f)]) != 0)
	{
		/* 返却コード1,2,4,5,8,9,10,11のみ更新する。 */
		if( Smrtdat2.ticket_rej[id - 1] < 0xff )
		{
			Smrtdat2.ticket_rej[id - 1] += 1;
		}
	}

	/*	'16-08-29 Add Box size*/
	set_box_size_to_ICB();
	/**/
	Smrtdat2.sum2 = (u8)culc_BLK2_checksum();			/* チェックサム更新 */
}
#endif

/*----------------------------------------------------------------------------------
* Erase all of the ICB data
*
*	1.Initialization of all area
*	2.Initialization of the recovery flag
*	3.Setting of initial information(Version, M/C#, Box#, Currency, Model, Serial)
*-----------------------------------------------------------------------------------
*	引数	:
*-----------------------------------------------------------------------------------
*	戻値	:
*---------------------------------------------------------------------------------*/
#if !defined(UBA_RTQ)
void	icb_set_initial_all_data(void) //not use RTQ
{
	int	ii;
	u16	num;

	if((ex_icb_state & (BIT_ICB_DATA_ALL_CLEAR | BIT_ICB_DATA_ALL_CLEAR_NEXT)) == BIT_ICB_DATA_ALL_CLEAR)
	{
		/*	Set 1st BLK#	*/
		num = BLK_DENOMI1_NUMBER;
		/*	on Next packet	*/
		ex_icb_state |= BIT_ICB_DATA_ALL_CLEAR_NEXT;
	}
	else
	{
		/*	BLK number of continuede	*/
		num = ex_Wcnt;
	}
	/*	Set to send buffer and data BLK number */
	for(ii = 0; ii < ICB_MAX_BACK_COUNTER; ii++)
	{
		(void)set_ICBrecovery_data(num);
		num += 1;
		if(num > BLK_SUM2_NUMBER)
		{
			/*	End	*/
			ex_icb_state &= ~(BIT_ICB_DATA_ALL_CLEAR | BIT_ICB_DATA_ALL_CLEAR_NEXT);
			break;
		}
	}
	/*	BLK number of continuede */
	ex_Wcnt = num;
}
#endif


/*------------------------------------------*/
/*	ゲーム機機ＮＯ．内容ﾁｪｯｸ				*/
/*	ﾃﾞｰﾀが壊れていれば0ｸﾘｱ					*/
/*------------------------------------------*/
void Check_Savgameno(void) //use SS and RTQ
{
	UB ii;

	for(ii = 0; ii < sizeof(ex_ICB_gameno); ii++){		/* マシンＮｏが適切な範囲の値であるかチェック	*//* 2013-04-10	*/
		if( (ex_ICB_gameno[ii] == ' ')										/* ASCI "スペース" */
			|| (( '0' <= ex_ICB_gameno[ii]) && ( ex_ICB_gameno[ii] <= '9'))	/* ASCI 0～9	*/
			|| (( 'A' <= ex_ICB_gameno[ii]) && ( ex_ICB_gameno[ii] <= 'Z'))	/* ASCI A～Z	*/
			|| (( 'a' <= ex_ICB_gameno[ii]) && ( ex_ICB_gameno[ii] <= 'z'))	/* ASCI a～z	*/
		){

		}
		else{												/* マシンＮｏ.が壊れている */
			memset( (u8 *)(ex_ICB_gameno), 0, (u16)(sizeof(ex_ICB_gameno)));	/* マシンＮｏ．Clear */	
			break;
		}
	}
//	Savsum = savnosum();									/* sum部分が壊れている場合もある為更新		*/
}


#if 0//#if defined(UBA_RTQ)
void update_checksum2()
{
	Smrtdat2.sum2 = (u8)culc_BLK2_checksum();			/* チェックサム更新 */
}
#endif 


#if defined(UBA_RTQ)
void icb_update_denomi_counter_rtq(u8 type, u8 ex_denomi)
{
	u16	data;
	int	denom;

	if(type == 0)
	{
	//通常時
		extern const u8 icb_custom_denom_assign[];
		denom = icb_custom_denom_assign[ex_denomi] + 1;
		data = SWAP_ENDIAN16(Smrtdat_fram.denomi[denom - 1]);
		if(data < 0xffff)
		{
			data += 1;
			Smrtdat_fram.denomi[denom - 1] = SWAP_ENDIAN16((u16)data);
			Smrtdat_fram.sum = (u8)culc_BLK1_checksum_rtq();
		}
	}
	else
	{
	//Collect時
		data = SWAP_ENDIAN16(Smrtdat_fram.denomi[ex_denomi - 1]);
		if(data < 0xffff)
		{
			data += 1;
			Smrtdat_fram.denomi[ex_denomi - 1] = SWAP_ENDIAN16((u16)data);
			Smrtdat_fram.sum = (u8)culc_BLK1_checksum_rtq();
		}		
	}
}

int	culc_BLK1_checksum_rtq(void)
{
	int	ii;
	int	sum;
	u8	*ptr;

	sum = 0;
	ptr = (u8 *)Smrtdat_fram.denomi;
	for(ii = 0; ii < sizeof(Smrtdat_fram.denomi); ii++)
	{
		sum += *(ptr + ii);
	}
	sum += (u8)(Smrtdat_fram.denomi_reserved >> 8);
	sum += (u8)(Smrtdat_fram.denomi_reserved & 0xff);
	sum += (u8)(Smrtdat_fram.total >> 8);
	sum += (u8)(Smrtdat_fram.total & 0xff);
	sum += (u8)(Smrtdat_fram.coupon >> 8);
	sum += (u8)(Smrtdat_fram.coupon & 0xff);
	sum += (u8)(Smrtdat_fram.totalin >> 8);
	sum += (u8)(Smrtdat_fram.totalin & 0xff);
	for(ii = 0; ii < sizeof(Smrtdat_fram.cinfo); ii++)
	{
		sum += Smrtdat_fram.cinfo[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat_fram.err); ii++)
	{
		sum += Smrtdat_fram.err[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat_fram.gameno); ii++)
	{
		sum += Smrtdat_fram.gameno[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat_fram.boxno); ii++)
	{
		sum += Smrtdat_fram.boxno[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat_fram.ver); ii++)
	{
		sum += Smrtdat_fram.ver[ii];
	}
	for(ii = 0; ii < sizeof(Smrtdat_fram.rw_ver); ii++)
	{
		sum += Smrtdat_fram.rw_ver[ii];
	}
	ptr = (u8 *)&Smrtdat_fram.restim;
	sum += *ptr;
	sum += *(ptr + 1);
	sum += *(ptr + 2);
	sum += *(ptr + 3);

	ptr = (u8 *)&Smrtdat_fram.settim;
	sum += *ptr;
	sum += *(ptr + 1);
	sum += *(ptr + 2);
	sum += *(ptr + 3);

	ptr = (u8 *)&Smrtdat_fram.initim;
	sum += *ptr;
	sum += *(ptr + 1);
	sum += *(ptr + 2);
	sum += *(ptr + 3);

	sum += Smrtdat_fram.flg;
	sum += Smrtdat_fram.assign;
	sum += Smrtdat_fram.id;

/*<--->*/
	return(sum ^ 0xff);
}

#endif


/* End of file */
