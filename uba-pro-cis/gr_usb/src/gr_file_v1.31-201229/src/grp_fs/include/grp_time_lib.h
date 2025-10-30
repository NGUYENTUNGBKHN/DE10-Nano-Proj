#ifndef	_GRP_TIME_LIB_H_
#define	_GRP_TIME_LIB_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_time_lib.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Header file for time conversion library								*/
/* FUNCTIONS:																*/
/*		grp_time_mktime				convert grp_time_tm_t to seconds		*/
/*		grp_time_localtime			convert seconds to grp_time_tm_t		*/
/*		grp_time_comp_days			compute days from 00/01/01				*/
/*		grp_time_set_base_year		set base year							*/
/*		grp_time_set_time_diff		set time difference from GMT			*/
/*		grp_time_get_config			get time configuration					*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003 Grape Systems, Inc.,  All Rights Reserved.             */
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

#include "grp_types.h"
/****************************************************************************/
/*  parameters																*/
/****************************************************************************/
#define GRP_TIME_BASE_YEAR	1970			/* base year */
#define GRP_TIME_DIFF		(9 * 60 * 60)	/* JST */

/****************************************************************************/
/*  time structure															*/
/****************************************************************************/
typedef struct grp_time_tm {
	grp_uchar_t		ucSec;					/* seconds (0 - 59) */
	grp_uchar_t		ucMin;					/* minutes (0 - 59) */
	grp_uchar_t		ucHour;					/* hour (0 - 23) */
	grp_uchar_t		ucDay;					/* day (1 - 31) */
	grp_uchar_t		ucMon;					/* month (1 - 12) */
	grp_uchar_t		ucWday;					/* day of the week (0 - 6) */
	short			sYear;					/* year */
} grp_time_tm_t;

/****************************************************************************/
/*  exported interfaces														*/
/****************************************************************************/
grp_int32_t	grp_time_mktime(			/* convert grp_time_tm_t to seconds */
		grp_time_tm_t		*ptTM);		/* [IN]  grp_tm_t information */
int	 grp_time_localtime(				/* convert seconds to grp_time_tm_t */
		grp_int32_t			iTime,		/* [IN]  seconds from base */
		grp_time_tm_t		*ptTM);		/* [OUT] grp_time_tm_t information */
grp_int32_t grp_time_comp_days(			/* compute days from 00/01/01 */
		int				iYear,			/* [IN] year */
		grp_uint_t		uiMonth,		/* [IN] month */
		grp_uint_t		uiDay);			/* [IN] day of month */
void grp_time_set_base_year(			/* set base year */
		int				iBaseYear);		/* [IN]  base year */
void grp_time_set_time_diff(			/* set time difference from GMT */
		grp_int32_t		iTimeDiff);		/* [IN]  time difference (sec) */
void grp_time_get_config(				/* get time configuration */
		int				*piBaseYear,	/* [IN]  base year */
		grp_int32_t		*piTimeDiff);	/* [IN]  time difference */

/****************************************************************************/
/*  exported variables														*/
/****************************************************************************/
extern	grp_int32_t	grp_time_base;		/* base year */
extern	grp_int32_t	grp_time_base_days;	/* days from 0 */
extern	grp_int32_t	grp_time_diff_from_GMT; /* diff from GMT */

#endif	/* _GRP_TIME_LIB_H_ */
