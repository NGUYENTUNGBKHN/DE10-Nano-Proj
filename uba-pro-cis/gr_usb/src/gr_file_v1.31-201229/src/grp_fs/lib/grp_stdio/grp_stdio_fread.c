/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fread.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (read file)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fread				read data								*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10	Fixed return type of grp_stdio_fread	*/
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
/* FUNCTION:	grp_stdio_fread												*/
/*																			*/
/* DESCRIPTION:	Read data from file											*/
/* INPUT:		iSize:					element size						*/
/*				iCnt:					element count						*/
/*				ptFile:					file control data					*/
/* OUTPUT:		pvPtr:					read data							*/
/*																			*/
/* RESULT:		-1:						read error							*/
/*				others:					read count							*/
/*																			*/
/****************************************************************************/
grp_isize_t
grp_stdio_fread(
	void			*pvPtr,					/* [OUT] read buffer */
	grp_isize_t		iSize,					/* [IN]  element size */
	grp_isize_t		iCnt,					/* [IN]  element count */
	FILE			*ptFile)				/* [IN]  file control data */
{
	grp_isize_t		iReadSize;				/* size to read */
	grp_isize_t		iReadCnt;				/* total read byte count */
	grp_isize_t		iBufCnt;				/* copy count from buffer */
	grp_uchar_t		*pucPtr = pvPtr;		/* buffer pointer */

	iReadSize = iSize * iCnt;				/* size to read */
	if (ptFile->iStatus & GRP_STDIO_STDIN) { /* stdin */
		iReadCnt = grp_stdio_io_stdin(ptFile, pucPtr, iReadSize);
		if (iReadCnt < 0)					/* read error */
			return(-1);						/* return errro */
		return(iReadSize / iSize);			/* return element count */
	}
	if ((ptFile->iStatus & GRP_STDIO_READ) == 0) { /* not read */
		if ((ptFile->iStatus & GRP_STDIO_PLUS) == 0)	/* not plus mode */
			return(-1);						/* return error */
		if (grp_stdio_fflush(ptFile) < 0)	/* flush I/O failed */
			return(-1);						/* return error */
		ptFile->iStatus &= ~GRP_STDIO_WRITE; /* clear write mode */
		ptFile->iStatus |= GRP_STDIO_READ; /* set read mode */
	}
	for (iReadCnt = 0; iReadCnt < iReadSize; iReadCnt += iBufCnt) {
		if (ptFile->iInBuf <= ptFile->iNext) { /* no data in buffer */
			if (grp_stdio_fill(ptFile) < 0) /* fill buffer failed */
				return(-1);					   /* return error */
		}
		if (ptFile->iInBuf <= ptFile->iNext)/* still no data */
			break;							/* break */
		iBufCnt = ptFile->iInBuf - ptFile->iNext; /* data size */
		if (iBufCnt > iReadSize - iReadCnt)	/* in-buffer size is bigger */
			iBufCnt = iReadSize - iReadCnt;	/* adust to size to read */
		memcpy(pucPtr, &ptFile->pucBuf[ptFile->iNext], (grp_size_t)iBufCnt);
											/* copy data */
		ptFile->iNext += iBufCnt;			/* increment buffer index */
		pucPtr += iBufCnt;					/* increment buffer pointer */
	}
	return(iReadCnt / iSize);				/* return element count */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
