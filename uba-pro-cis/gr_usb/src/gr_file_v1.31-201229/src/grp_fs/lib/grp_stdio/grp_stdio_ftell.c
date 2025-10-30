/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_ftell.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (get current seek pointer)						*/
/* FUNCTIONS:																*/
/*		grp_stdio_ftell				get current pointer						*/
/* DEPENDENCIES:															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
/*		T.Imashiki		2007/02/20	Fixed return value handling of lseek	*/
/*									for 16 bit CPU support					*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
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
#if(GRP_FS_MINIMIZE_LEVEL < 1)
#include "grp_stdio.h"

/****************************************************************************/
/* FUNCTION:	grp_stdio_ftell												*/
/*																			*/
/* DESCRIPTION:	Get current seek pointer									*/
/* INPUT:		ptFile:					file control data					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						I/O error							*/
/*				others:					current seek pointer				*/
/*																			*/
/****************************************************************************/
long
grp_stdio_ftell(							/* seek file */
	FILE				*ptFile)			/* [IN]  file control data */
{
	grp_ioffset_t		iOff;				/* current offset */

	if (ptFile->iStatus & (GRP_STDIO_STDIN|GRP_STDIO_STDOUT)) /* stdin/out */
		return(0);							/* return 0 */
	iOff = lseek(ptFile->iFd, 0, SEEK_CUR);	/* get current pointer */
	if (iOff < 0) {							/* seek failed */
		ptFile->iError = (int)iOff;			/* set error number */
		return(-1);							/* return error */
	}
	if (ptFile->iStatus & GRP_STDIO_WRITE)	/* write mode */
		return(iOff + ptFile->iNext);		/* return seek pointer */
	if (ptFile->iStatus & GRP_STDIO_READ)	/* read mode */
		return(iOff + ptFile->iNext - ptFile->iInBuf); /* return seek pointer */
	return(-1);								/* bad mode */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
