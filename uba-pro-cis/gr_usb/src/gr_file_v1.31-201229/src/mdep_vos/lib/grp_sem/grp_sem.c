/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_sem.c													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Semaphore interfaces for VOS										*/
/* FUNCTIONS:																*/
/*		grp_sem_create				create semaphore						*/
/*		grp_sem_delete				delete semaphore						*/
/*		grp_sem_get					get semaphore							*/
/*		grp_sem_get					get semaphore							*/
/*		grp_sem_release				release semaphore						*/
/* DEPENDENCIES:															*/
/*		grp_sem.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/12/07	Fixed bug no cast for pcName parameter	*/
/*									of tx_semaphore_create					*/
/*									Fixed spell miss of THREADX ifdef		*/
/*		T.Imashiki		2005/03/24	Added id initialization at THREADX		*/
/*									semaphore create for undocumented		*/
/*									feature of THREADX						*/
/*		T.Imashiki		2006/07/04	Fixed to change ifdef name from			*/
/*									T_KERNEL to GRP_VOS in grp_sem_release	*/
/*		K.Kaneko		2008/05/21	Deleted processing except VOS			*/
/*									Added include grp_fs_sysdef.h			*/
/*		K.Kaneko		2010/11/16	Added a type cast to the initial count	*/
/*									parameter of GRP_VOS_CreateSemaphore	*/
/*									called from grp_sem_create				*/
/*						2010/11/16	Added a type cast to the timeout		*/
/*									parameter of GRP_VOS_GetSemaphore		*/
/*									called from grp_sem_get					*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2010 Grape Systems, Inc.,  All Rights Reserved.        */
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
#include "grp_sem.h"

/****************************************************************************/
/* FUNCTION:	grp_sem_create												*/
/*																			*/
/* DESCRIPTION:	Create semaphore											*/
/* INPUT:		pcName:				semaphore name							*/
/*				iInstance:			instance number							*/
/* OUTPUT:		ptSem:				created semaphore						*/
/*																			*/
/* RESULT:		-1:					failed to create semaphore				*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_sem_create(
	grp_sem_t		*ptSem,					/* [OUT] semaphore */
	const char		*pcName,				/* [IN]  semaphore name */
	int				iInitCnt)				/* [IN]  initial sem count */
{
	if (GRP_VOS_CreateSemaphore(ptSem, (grp_uchar_t *)pcName, (UINT32)iInitCnt) != 0)
		return(-1);							/* create error */
	return(0);								/* success */
}

/****************************************************************************/
/* FUNCTION:	grp_sem_delete												*/
/*																			*/
/* DESCRIPTION:	Delete semaphore											*/
/* INPUT:		tSem:				semaphore								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:					failed to delete semaphore				*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_sem_delete(
	grp_sem_t		tSem)					/* [IN]  semaphore */
{
	if (GRP_VOS_DeleteSemaphore(tSem) != 0)	/* delete error */
		return(-1);							/* return error */
	return(0);								/* success */
}

/****************************************************************************/
/* FUNCTION:	grp_sem_get													*/
/*																			*/
/* DESCRIPTION:	Get semaphore												*/
/*				Note: timeout may not be supported in some imprementation	*/
/* INPUT:		tSem:				get semaphore							*/
/*				iTimeout:			timeout in msec(negative: wait forever)	*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:					error									*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_sem_get(
	grp_sem_t		tSem,					/* [IN]  semaphore */
	grp_int32_t		iTimeout)				/* [IN]  timeout (msec) */
{
	grp_uint32_t	uiTimeout;

	uiTimeout = (iTimeout < 0)? GRP_VOS_INFINITE: iTimeout;
	if (GRP_VOS_GetSemaphore(tSem, (UINT32)uiTimeout) != 0)
		return(-1);							/* return semaphore error */
	return(0);
}

/****************************************************************************/
/* FUNCTION:	grp_sem_release												*/
/*																			*/
/* DESCRIPTION:	Release semaphore											*/
/* INPUT:		tSem:				release semaphore						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_sem_release(
	grp_sem_t		tSem)					/* [IN]  semaphore */
{
	GRP_VOS_ReleaseSemaphore(tSem);			/* release semaphore */
}
