#ifndef	_GRP_MDEP_SEM_H_
#define	_GRP_MDEP_SEM_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_mdep_sem.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Platform dependent semaphore definitions							*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		Platform dependent include files									*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
/*		T.Imashiki		2004/03/30	Added ifdef for THREADX					*/
/*		K.Kaneko		2008/05/21	Deleted #ifdef except VOS				*/
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


/****************************************************************************/
/*  basic typedefs															*/
/****************************************************************************/
#include "grp_vos.h"

typedef	GRP_VOS_tSemaphore	*grp_sem_t;			/* semaphore ID */

#endif	/* _GRP_MDEP_SEM_H_ */
