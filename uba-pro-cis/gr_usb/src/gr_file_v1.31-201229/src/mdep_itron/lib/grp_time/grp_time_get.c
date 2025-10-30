/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_time_get.c												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Platform dependent get time routine(empty stub)						*/
/* FUNCTIONS:																*/
/*		grp_time_get				get time from 1970/1/1 in seconds		*/
/* DEPENDENCIES:															*/
/*		grp_time.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/01	Created inital version 1.0				*/
/*		K.Kaneko		2008/05/21	Deleted Platform dependent code			*/
/*									Added processing image					*/
/*									Added include grp_fs_sysdef.h			*/
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
#include "grp_time.h"

/****************************************************************************/
/* FUNCTION:	grp_time_get												*/
/*																			*/
/* DESCRIPTION:	Get current time (in seconds from base date)				*/
/*				default base date is 1970/1/1 GMT							*/
/* INPUT:		None														*/
/* OUTPUT:		piTime:						current time in seconds			*/
/*																			*/
/* RESULT:		-1:							error							*/
/*				others:						current time value in seconds	*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_time_get(
	grp_int32_t		*piTime)				/* [OUT] time in seconds */
{

	*********************************************************************
	* after confirmation, please delete this message					*
	*																	*
	* time get code here												*
	*  The following cord is a processing image							*
	*  Note: rtc_ctl_t is RTC mapping structure							*
	*********************************************************************
	*	rtc_ctl_t		*ptRtc = RTC_ADDR;	/* RTC register address */	*
	*	grp_time_tm_t	tTime;				/* time info */				*
	*																	*
	*	do {															*
	*		tTime.sYear  = ptRtc->ucYear;	/* read RTC Year value */	*
	*		tTime.ucMon  = ptRtc->ucMon;	/* read RTC Month value */	*
	*		tTime.ucDay  = ptRtc->ucDay;	/* read RTC Day value */	*
	*		tTime.ucHour = ptRtc->ucHour;	/* read RTC Hour value */	*
	*		tTime.ucMin  = ptRtc->ucMin;	/* read RTC Minute value */	*
	*		tTime.ucSec  = ptRtc->ucSec;	/* read RTC Second value */	*
	*	} while ( check carry up flag );								*
	*																	*
	*	if (tTime.sYear >= 70)				/* >= 70 */					*
	*		tTime.sYear += 1900;			/* assume 1900 base */		*
	*	else								/* < 70 */					*
	*		tTime.sYear += 2000;			/* assume 2000 base */		*
	*	iTime = grp_time_mktime(&tTime);	/* current time */			*
	*	if (piTime)							/* non null arg */			*
	*		*piTime = iTime;				/* set time value */		*
	*																	*
	*	return(iTime);						/* return time */			*
	*********************************************************************

	*piTime = -1;

	return(-1);								/* return time(error value) */
}
