#ifndef	_GRP_SEM_H_
#define	_GRP_SEM_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_sem.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definition for semaphore interface									*/
/* FUNCTIONS:																*/
/*		grp_sem_create				create semaphore						*/
/*		grp_sem_delete				delete semaphore						*/
/*		grp_sem_get					get semaphore							*/
/*		grp_sem_get					get semaphore							*/
/*		grp_sem_release				release semaphore						*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_mdep/grp_mdep_sem.h												*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
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
/*  platform dependent typedef for grp_sem_t								*/
/****************************************************************************/
#include "grp_mdep_sem.h"

/****************************************************************************/
/*  platform dependent interfaces											*/
/****************************************************************************/
/****************************************************/
/* create semaphore									*/
/****************************************************/
int	grp_sem_create(								/* create semaphore */
		grp_sem_t		*ptSem,					/* [OUT] semaphore */
		const char		*pcName,				/* [IN]  semaphore name */
		int				iInitCnt);				/* [IN]  initial sem count */

/****************************************************/
/* delete semaphore									*/
/****************************************************/
int	grp_sem_delete(								/* delete semaphore */
		grp_sem_t		tSem);					/* [IN]  semaphore */

/****************************************************/
/* get semaphore									*/
/****************************************************/
int grp_sem_get(								/* get semaphore */
		grp_sem_t		tSem,					/* [IN]  semaphore */
		grp_int32_t		iTimeout);				/* [IN]  timeout(msec) */

/****************************************************/
/* release semaphore								*/
/****************************************************/
void grp_sem_release(							/* release semaphore */
		grp_sem_t		tSem);					/* [IN]  semaphore */

#endif	/* _GRP_SEM_H_ */
