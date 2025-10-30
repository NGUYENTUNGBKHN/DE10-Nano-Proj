/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_time_lib.c												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Time conversion library												*/
/* FUNCTIONS:																*/
/*		grp_time_mktime				convert grp_time_tm_t to seconds		*/
/*		grp_time_localtime			convert seconds to grp_time_tm_t		*/
/*		grp_time_comp_days			compute days from 00/01/01				*/
/*		grp_time_set_base_year		set base year							*/
/*		grp_time_set_time_diff		set time difference from GMT			*/
/*		grp_time_get_config			get time configuration					*/
/* DEPENDENCIES:															*/
/*		grp_time_lib.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10  Added type cast for 16 bit CPU			*/
/*		T.Imashiki		2007/02/20  Added type cast for 16 bit CPU			*/
/*		T.Imashiki		2007/11/16	Fixed miscomputation of February day in	*/
/*									leap year, and that of day of the week  */
/*									due to cast-miss in the computation at	*/
/*									grp_fs_localtime						*/
/*									Fixed spell miss of leap year			*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2008 Grape Systems, Inc.,  All Rights Reserved.        */
/*                                                                          */
/* This software is furnished under a license, and may be used only in      */
/* accordance with the terms of such license and with the inclusion of the  */
/* above copyright notice.  No title to and the ownership of the software   */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* non-infringement of any intellectual property rights and the performance */
/* of this computer program, and specifically disclaims any responsibility  */
/* for any damages, special or consequential, connected  with the use of    */
/* this program.                                                            */
/****************************************************************************/

#include "grp_fs_sysdef.h"
#include "grp_time_lib.h"

#define GRP_TIME_HOUR_SEC	((grp_int32_t)60 * 60)	/* seconds in a day */
#define GRP_TIME_DAY_SEC	(24 * GRP_TIME_HOUR_SEC)/* seconds in an hour */
#define	GRP_TIME_0TH_WDAY	6					/* day of the week 0/1/1 */
#define GRP_TIME_OUT_RANGE	70					/* year span */

static grp_uchar_t	_ucDaysPerMonth[12] = {		/* days per month */
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
static short		_sDaysMonYear[12] = {		/* total days in a year */
	0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 
};

grp_int32_t		grp_time_base = GRP_TIME_BASE_YEAR; /* base year */
grp_int32_t		grp_time_base_days = -1;			/* days from 0 */
grp_int32_t		grp_time_diff_from_GMT = GRP_TIME_DIFF; /* diff from GMT */

/****************************************************************************/
/* FUNCTION:	grp_time_comp_days											*/
/*																			*/
/* DESCRIPTION:	Compute days from year 0									*/
/* INPUT:		iYear:				year									*/
/*				uiMonth:			month									*/
/*				uiDay:				day of month							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		days from 0 year											*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_time_comp_days(
	int				iYear,					/* [IN] year */
	grp_uint_t		uiMonth,				/* [IN] month */
	grp_uint_t		uiDay)					/* [IN] day of month */
{
	grp_int32_t		iDays;					/* days */
	int				iLeap;					/* leap year */

	/****************************************************/
	/* check value										*/
	/****************************************************/
	if (uiMonth == 0 || uiMonth > 12 || uiDay == 0) /* invalid month or day */
		return(-1);							/* error */

	/****************************************************/
	/* compute total days from 0th year					*/
	/****************************************************/
	iLeap = ((iYear % 4) == 0 && ((iYear % 100) != 0 || (iYear % 400) == 0));
											/* leap year */
	iDays = (uiMonth == 2)? ((iLeap)? 29: 28):/* days for February */
			_ucDaysPerMonth[uiMonth - 1];	/* days for others */
	if ((grp_int32_t)uiDay > iDays)			/* invalid day */
		return(-1);							/* error */
	iDays = ((grp_int32_t)iYear * 365) + ((iYear + 3) / 4) 
			- ((iYear + 99) / 100)
			+ ((iYear + 399) / 400);		/* to the year */
	iDays += _sDaysMonYear[uiMonth - 1];	/* to the month */
	iDays += uiDay - 1;						/* to the day */
	if (iLeap && uiMonth > 2)				/* leap year */
		iDays++;							/* add 1 day */
	return(iDays);							/* return total days */
}

/****************************************************************************/
/* FUNCTION:	_grp_time_init												*/
/*																			*/
/* DESCRIPTION:	Initialize _base_wday										*/
/* INPUT:		None														*/
/* OUTPUT:		grp_time_base_days: 	total days for the base				*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_time_init(void)
{
	grp_time_base_days = grp_time_comp_days(grp_time_base, 1, 1);
										/* set total days */
}

/****************************************************************************/
/* FUNCTION:	grp_time_set_base_year										*/
/*																			*/
/* DESCRIPTION:	Set base year												*/
/* INPUT:		iBaseYear:				base year							*/
/* OUTPUT:		grp_time_base:			base year							*/
/*				grp_time_base_days:		total days for base from 0th year	*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_time_set_base_year(
	int				iBaseYear)			/* [IN]  base year */
{
	grp_time_base = iBaseYear;			/* set base year */
	grp_time_base_days = -1;			/* reset base days */
	_grp_time_init();					/* initialize */
}

/****************************************************************************/
/* FUNCTION:	grp_time_set_time_diff										*/
/*																			*/
/* DESCRIPTION:	Set time difference from GMT								*/
/* INPUT:		iTimeDiff:				time difference from GMT (sec)		*/
/* OUTPUT:		grp_time_diff_from_GMT: time difference from GMT			*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_time_set_time_diff(
	grp_int32_t		iTimeDiff)			/* [IN]  time difference */
{
	grp_time_diff_from_GMT = iTimeDiff % GRP_TIME_DAY_SEC; /* set time diff */
}

/****************************************************************************/
/* FUNCTION:	grp_time_get_config											*/
/*																			*/
/* DESCRIPTION:	Get time configuration ( base time and time difference )	*/
/* INPUT:		None														*/
/* OUTPUT:		piBaseYear:				base year							*/
/*				piTimeDiff:				time difference						*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_time_get_config(
	int				*piBaseYear,		/* [IN]  base year */
	grp_int32_t		*piTimeDiff)		/* [IN]  time difference */
{
	*piBaseYear = (int)grp_time_base;		/* get base year */
	*piTimeDiff = grp_time_diff_from_GMT;	/* get time difference from GMT */
}

/****************************************************************************/
/* FUNCTION:	grp_time_mktime												*/
/*																			*/
/* DESCRIPTION:	Convert grp_time_tm_t information to seconds from base		*/
/* INPUT:		ptTM:			time information							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		total seconds since base year								*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_time_mktime(
	grp_time_tm_t	*ptTM)				/* [IN]  grp_time_tm_t information */
{
	grp_int32_t		iDays;				/* total days */
	grp_int32_t		iSecs;				/* seconds */
	grp_int32_t		iSecsGMT;			/* seconds for GMT */

	if (grp_time_base_days < 0)			/* base is not set */
		_grp_time_init();				/* initialize */
	if (ptTM->sYear < grp_time_base - GRP_TIME_OUT_RANGE
		|| ptTM->sYear > grp_time_base + GRP_TIME_OUT_RANGE)
		return(-1);						/* year out of range */
	if (ptTM->ucSec >= 60				/* invalid sec */
		|| ptTM->ucMin >= 60			/* invalid min */
		|| ptTM->ucHour >= 24)			/* invalid hour */
		return(-1);						/* error */
	iDays = grp_time_comp_days(ptTM->sYear, ptTM->ucMon, ptTM->ucDay);
										/* total days */
	if (iDays == -1)					/* error detected */
		return(-1);						/* error */
	iDays -= grp_time_base_days;		/* days from base */ 
	iSecs = iDays * GRP_TIME_DAY_SEC
			+ ptTM->ucHour * GRP_TIME_HOUR_SEC
			+ (grp_int32_t)ptTM->ucMin * 60
			+ ptTM->ucSec;				/* total seconds */
	iSecsGMT = iSecs - grp_time_diff_from_GMT;	/* seconds for GMT */
	if ((ptTM->sYear >= grp_time_base && iSecsGMT < -grp_time_diff_from_GMT) 
		|| (ptTM->sYear < grp_time_base 
			&& iSecsGMT >= -grp_time_diff_from_GMT))
		return(-1);						/* return error */
	return(iSecsGMT);					/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_time_localtime											*/
/*																			*/
/* DESCRIPTION:	Convert total seconds from base to grp_time_tm_t			*/
/* INPUT:		iTime:			seconds from base							*/
/* OUTPUT:		ptTM:			time information							*/
/*																			*/
/* RESULT:		0: 			success											*/
/*				-1:			error											*/
/*																			*/
/****************************************************************************/
int
grp_time_localtime(
	grp_int32_t			iTime,			/* [IN]   seconds from base */
	grp_time_tm_t		*ptTM)			/* [OUT]  grp_tm_t information */
{
	grp_int32_t		iDays;				/* total days */
	grp_int32_t		iDaysOfYear;		/* total days of the year */
	grp_int32_t		iSec;				/* sec */
	grp_uint32_t	uiTimeLoc;			/* local seconds */
	int				iLeap;				/* leap year */
	int				iDayYear;			/* days in year */
	int				iMon;				/* month */
	int				iYear;				/* year */

	/****************************************************/
	/* compute total days from 0th year					*/
	/****************************************************/
	if (iTime >= -grp_time_diff_from_GMT) {		/* positive computation */
		uiTimeLoc = iTime + grp_time_diff_from_GMT; /* GMT seconds */
		iDays = uiTimeLoc / GRP_TIME_DAY_SEC; /* total days */
		iSec = uiTimeLoc - (iDays * GRP_TIME_DAY_SEC); /* sec in day */
	} else {										/* negative computation */
		uiTimeLoc = -(iTime + grp_time_diff_from_GMT); /* GMT seconds */
		iDays = (uiTimeLoc + (GRP_TIME_DAY_SEC - 1)) /  GRP_TIME_DAY_SEC;
		iSec = uiTimeLoc - (iDays * GRP_TIME_DAY_SEC);/* sec in day */
		iDays = -iDays;								/* negate day */
		iSec = -iSec;								/* negate sec */
	}
	if (grp_time_base_days < 0)			/* base is not set */
		_grp_time_init();				/* initialize */
	iDays += grp_time_base_days;		/* add base days */
	if (iDays < 0)						/* negative days */
		return(-1);						/* error */
	ptTM->ucWday = (grp_uchar_t)((iDays + GRP_TIME_0TH_WDAY) % 7);
										/* day of the week */

	/****************************************************/
	/* compute year										*/
	/****************************************************/
	iYear = (iDays / 365);				/* estimate year */
	for ( ; iDays < (iDaysOfYear = grp_time_comp_days(iYear, 1, 1)); iYear--);
	ptTM->sYear = (short)iYear;			/* set year */

	/****************************************************/
	/* compute month									*/
	/****************************************************/
	iDays -= iDaysOfYear;				/* days in the year */
	iLeap = ((ptTM->sYear % 4) == 0 
			&& ((ptTM->sYear % 100) != 0
				|| (ptTM->sYear % 400) == 0));	/* leap year */
	for (iMon = 0; iMon < 12; iMon++) {		/* lookup month */
		iDayYear = _sDaysMonYear[iMon];		/* days in year */
		if (iLeap && iMon >= 2)				/* leap year */
			iDayYear++;						/* increment it */
		if (iDayYear > iDays)				/* found month */
			break;							/* break */
	}
	ptTM->ucMon = (grp_uchar_t)iMon;		/* set month */

	/****************************************************/
	/* compute day/hour/min/sec							*/
	/****************************************************/
	iDays -= _sDaysMonYear[iMon - 1];		/* day in month */
	if (iLeap && iMon > 2)					/* leap year */
		iDays--;							/* decrement */
	ptTM->ucDay = (grp_uchar_t)(iDays + 1);	/* set day */
	ptTM->ucHour = (grp_uchar_t)(iSec / GRP_TIME_HOUR_SEC);	/* hour */
	iSec -= ptTM->ucHour * GRP_TIME_HOUR_SEC; /* sec in hour */
	ptTM->ucMin = (grp_uchar_t)(iSec / 60);	/* set minute */
	ptTM->ucSec = (grp_uchar_t)(iSec % 60);	/* set sec */
	return(0);
}
