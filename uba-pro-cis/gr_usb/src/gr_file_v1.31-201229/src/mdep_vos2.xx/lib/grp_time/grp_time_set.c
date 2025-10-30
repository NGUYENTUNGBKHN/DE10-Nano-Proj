/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_time_set.c												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Platform dependent set time routine									*/
/* FUNCTIONS:																*/
/*		grp_time_set				set time from 1970/1/1 in seconds		*/
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
/* FUNCTION:	grp_time_set												*/
/*																			*/
/* DESCRIPTION:	Set current time											*/
/* INPUT:		ptTM:						current time					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:							error							*/
/*				0:							success							*/
/*																			*/
/****************************************************************************/
int
grp_time_set(
	grp_time_tm_t		*ptTM)				/* [IN]  current time */
{
#if 0
	*********************************************************************
	* after confirmation, please delete this message					*
	*																	*
	* time set code here (XXX)											*
	*  The following cord is a processing image							*
	*  Note: rtc_ctl_t is RTC mapping structure							*
	*********************************************************************
	*	rtc_ctl_t		*ptRtc = RTC_ADDR;	/* RTC register address */	*
	*	grp_int32_t		iTime;				/* time in seconds */		*
	*	int				iYear;				/* year value */			*
	*																	*
	*	if ((iTime = grp_time_mktime(ptTM)) == -1)						*
	*		return(-1);						/* bad ptTM info */			*
	*																	*
	*	grp_time_localtime(iTime, ptTM);	/* convert to day of week */*
	*	iYear = (int)(ptTM->sYear % 100);	/* year value */			*
	*																	*
	*	/*******************************/								*
	*	/* reset RTC & clear start bit */								*
	*	/*******************************/								*
	*																	*
	*	ptRtc->ucYear = iYear;				/* year */					*
	*	ptRtc->ucMon  = ptTM->ucMon;		/* month */					*
	*	ptRtc->ucWDay = ptTM->ucWday;		/* day of week */			*
	*	ptRtc->ucDay  = ptTM->ucDay;		/* day */					*
	*	ptRtc->ucHour = ptTM->ucHour;		/* hour */					*
	*	ptRtc->ucMin  = ptTM->ucMin;		/* minute */				*
	*	ptRtc->ucSec  = ptTM->ucSec;		/* second */				*
	*																	*
	*	/*******************************/								*
	*	/* start RTC                   */								*
	*	/*******************************/								*
	*																	*
	*	return(0);							/* return success */		*
	*********************************************************************

	return(-1);								/* return success */
#else
	return 0;
#endif
}
