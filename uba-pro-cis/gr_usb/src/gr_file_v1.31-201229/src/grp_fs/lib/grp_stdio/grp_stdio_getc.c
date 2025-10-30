/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_get.c												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (get character)								*/
/* FUNCTIONS:																*/
/*		grp_stdio_getc				get character							*/
/* DEPENDENCIES:															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
/*		T.Imashiki		2007/02/20	Added type casts for 16 bit CPU support	*/
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
/* FUNCTION:	grp_stdio_getc												*/
/*																			*/
/* DESCRIPTION:	get character												*/
/* INPUT:		ptFile:					file control data					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						I/O error							*/
/*				others:					got character						*/
/*																			*/
/****************************************************************************/
int
grp_stdio_getc(
	FILE			*ptFile)				/* [IN]  file control data */
{
	grp_uchar_t		uc;						/* character data */

	if (ptFile->iStatus & GRP_STDIO_STDIN) { /* stdin */
		if (grp_stdio_io_stdin(ptFile, &uc, 1) <= 0)/* read failed */
			return(-1);						/* return erorr */
		return((int)uc);					/* return character data */
	}
	if ((ptFile->iStatus & GRP_STDIO_READ) == 0) { /* not read */
		if ((ptFile->iStatus & GRP_STDIO_PLUS) == 0)	/* not plus mode */
			return(-1);						/* return error */
		if (grp_stdio_fflush(ptFile) < 0)	/* flush I/O failed */
			return(-1);						/* return error */
		ptFile->iStatus &= ~GRP_STDIO_WRITE; /* clear write mode */
		ptFile->iStatus |= GRP_STDIO_READ; /* set read mode */
	}
	if (ptFile->iNext >= ptFile->iInBuf) {	/* no data in buffer */
		if (grp_stdio_fill(ptFile) < 0)		/* fill buffer failed */
			return(-1);						/* return error */
	}
	if (ptFile->iNext >= ptFile->iInBuf) 	/* still no data in buffer */
		return(-1);							/* EOF */
	return((int)ptFile->pucBuf[ptFile->iNext++]);/* return got character */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
