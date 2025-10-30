/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fill.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (fill buffer)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fill				fill data								*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
/*		T.Imashiki		2007/02/20	Added type cast for 16 bit CPU support	*/
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
#include <string.h>
#include "grp_stdio.h"

/****************************************************************************/
/* FUNCTION:	grp_stdio_fill												*/
/*																			*/
/* DESCRIPTION:	Fill buffer													*/
/* INPUT:		ptFile:					file control data					*/
/* OUTPUT:		ptFile:					read data							*/
/*																			*/
/* RESULT:		-1:						read error							*/
/*				others:					in buffer count						*/
/*																			*/
/****************************************************************************/
int
grp_stdio_fill(
	FILE			*ptFile)				/* [IN/OUT]  file control data */
{
	grp_int32_t			iSize;				/* read size */

	if ((ptFile->iStatus & GRP_STDIO_READ) == 0) /* not read mode */
		return(-1);							/* return error */
	if (ptFile->iNext < ptFile->iInBuf)		/* data left in buffer */
		return(ptFile->iInBuf - ptFile->iNext);/* return in buffer size */
	ptFile->iNext = 0;						/* init next character index */
	iSize = read(ptFile->iFd, ptFile->pucBuf, ptFile->iBufSize);
											/* read data from file */
	if (iSize <= 0) {						/* error or EOF */
		ptFile->iInBuf = 0;					/* no data */
		if (iSize == 0)	{					/* EOF */
			ptFile->iStatus |= GRP_STDIO_EOF;/* set EOF flag */
			return(0);						/* return 0 */
		} else {							/* error */
			ptFile->iError = (int)iSize;	/* remember error status */
			return(-1);						/* return error */
		}
	}
	ptFile->iInBuf = iSize;					/* set size */
	return(iSize);							/* return filled size */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
