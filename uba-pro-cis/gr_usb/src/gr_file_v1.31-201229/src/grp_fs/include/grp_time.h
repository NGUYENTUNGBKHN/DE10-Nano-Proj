#ifndef	_GRP_TIME_H_
#define	_GRP_TIME_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_time.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definition for accessing/setting current time						*/
/* FUNCTIONS:																*/
/*		grp_time_get				get time from 1970/1/1 in seconds		*/
/*		grp_time_set				set time from 1970/1/1 in seconds		*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_time_lib.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/02/26	Created inital version 1.0				*/
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
#include "grp_time_lib.h"

/****************************************************************************/
/* exported interfaces														*/
/****************************************************************************/
grp_int32_t grp_time_get(				/* get time from 1970/1/1 in seconds */
		grp_int32_t		*piTime);		/* [OUT] time in seconds */
int grp_time_set(						/* set time from 1970/1/1 in seconds */
		grp_time_tm_t	*ptTM);			/* [IN]  current time */

#endif	/* _GRP_TIME_H_ */
