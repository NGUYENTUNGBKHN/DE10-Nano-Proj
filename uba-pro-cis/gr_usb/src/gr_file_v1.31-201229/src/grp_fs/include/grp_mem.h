#ifndef	_GRP_MEM_H_
#define	_GRP_MEM_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_mem.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Header file for memory management library							*/
/* FUNCTIONS:																*/
/*		grp_mem_alloc				allocate memory							*/
/*		grp_mem_free				free memory								*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10	Changed type of iSize parameter of		*/
/*									grp_mem_alloc from grp_int32 to			*/
/*									grp_isize_t								*/
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
/*  exported interface														*/
/****************************************************************************/
void *grp_mem_alloc(							/* allocate memory */
		grp_isize_t		iSize);					/* [IN]  size to allocate */
void grp_mem_free(								/* free memory */
		void			*pvMem);				/* [IN]  memory to free */

#endif	/* _GRP_MEM_H_ */
