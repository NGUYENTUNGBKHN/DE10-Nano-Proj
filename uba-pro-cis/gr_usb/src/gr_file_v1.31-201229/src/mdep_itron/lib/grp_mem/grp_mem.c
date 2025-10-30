/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_mem.c													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Get/Free memory for itron											*/
/* FUNCTIONS:																*/
/*		grp_mem_alloc				allocate memory							*/
/*		grp_mem_free				free memory								*/
/* DEPENDENCIES:															*/
/*		grp_mem.h															*/
/*		Platform dependent includes											*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10	Changed type of iAllocSize of			*/
/*									grp_mem_alloc from grp_int32_t to		*/
/*									grp_isize_t								*/
/*		T.Imashiki		2007/02/20	Fixed to change get_mpl to pget_mpl		*/
/*									used in grp_mem_alloc implementation	*/
/*									for ITRON								*/
/*		K.Kaneko		2008/05/21	Deleted processing except ITRON			*/
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
#include "grp_mem.h"

#ifdef	NON_POSIX
#include <itron.h>
#include <kernel.h>
#include "grp_itron_id.h"						/* ID definitions */
ID		grp_mem_pool_id = GRP_MEM_POOL_ID;		/* memory pool id */
#else	/* NON_POSIX */
#include <stdlib.h>
#endif	/* NON_POSIX */

/****************************************************************************/
/* FUNCTION:	grp_mem_alloc												*/
/*																			*/
/* DESCRIPTION:	Allocate memory												*/
/* INPUT:		iAllocSize:			allocate size							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:				failed to allocate memory				*/
/*				others:				address of allocated memory				*/
/*																			*/
/****************************************************************************/
void *
grp_mem_alloc(
	grp_isize_t		iAllocSize)				/* [IN]  allocate size */
{
#ifdef	NON_POSIX
	VP pvAddr;
	return((pget_mpl(grp_mem_pool_id, iAllocSize, &pvAddr) == 0)? pvAddr: NULL);
#else	/* NON_POSIX */
	return(malloc(iAllocSize));				/* return memory */
#endif	/* NON_POSIX */
}

/****************************************************************************/
/* FUNCTION:	grp_mem_free												*/
/*																			*/
/* DESCRIPTION:	Free memory													*/
/* INPUT:		pvAddr:				address of memory to free				*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_mem_free(
	void			*pvAddr)				/* [IN]  memory to free */
{
#ifdef	NON_POSIX
	rel_mpl(grp_mem_pool_id, pvAddr);		/* free memory */
#else	/* NON_POSIX */
	free(pvAddr);							/* free memory */
#endif	/* NON_POSIX */
}
