/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fwrite.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (write file)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fwrite			write data								*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10	Fixed return type of grp_stdio_fwrite	*/
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
/* FUNCTION:	grp_stdio_fwrite											*/
/*																			*/
/* DESCRIPTION:	Write data to file											*/
/* INPUT:		pvPtr:					write data							*/
/*				iSize:					element size						*/
/*				iCnt:					element count						*/
/*				ptFile:					file control data					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						write error							*/
/*				others:					write count							*/
/*																			*/
/****************************************************************************/
grp_isize_t
grp_stdio_fwrite(
	void			*pvPtr,					/* [IN]  write data */
	grp_isize_t		iSize,					/* [IN]  element size */
	grp_isize_t		iCnt,					/* [IN]  element count */
	FILE			*ptFile)				/* [IN]  file control data */
{
	grp_isize_t		iWriteSize;				/* size to write */
	grp_isize_t		iWriteCnt;				/* total written byte count */
	grp_isize_t		iBufCnt;				/* copy count to buffer */
	grp_uchar_t		*pucPtr = pvPtr;		/* buffer pointer */

	iWriteSize = iSize * iCnt;				/* size to write */
	if (ptFile->iStatus & GRP_STDIO_STDOUT) { /* stdout */
		iWriteCnt = grp_stdio_io_stdout(ptFile, pucPtr, iWriteSize);
		if (iWriteCnt < 0)					/* read error */
			return(-1);						/* return errro */
		return(iWriteSize / iSize);			/* return element count */
	}
	if ((ptFile->iStatus & GRP_STDIO_WRITE) == 0) { /* not read */
		if ((ptFile->iStatus & GRP_STDIO_PLUS) == 0)	/* not plus mode */
			return(-1);						/* return error */
		if (grp_stdio_fflush(ptFile) < 0)	/* flush I/O failed */
			return(-1);						/* return error */
		ptFile->iStatus &= ~GRP_STDIO_READ; /* clear read mode */
		ptFile->iStatus |= GRP_STDIO_WRITE; /* set write mode */
	}
	for (iWriteCnt = 0; iWriteCnt < iWriteSize; iWriteCnt += iBufCnt) {
		if (ptFile->iNext >= ptFile->iBufSize) { /* buffer full */
			if (grp_stdio_fflush(ptFile) < 0) /* flush buffer failed */
				return(-1);					/* return error */
		}
		iBufCnt = ptFile->iBufSize - ptFile->iNext; /* data size */
		if (iBufCnt > iWriteSize - iWriteCnt)/* in-buffer size is bigger */
			iBufCnt = iWriteSize - iWriteCnt;/* adust to size to write */
		memcpy(&ptFile->pucBuf[ptFile->iNext], pucPtr, (grp_size_t)iBufCnt);
											/* copy data */
		ptFile->iNext += iBufCnt;			/* increment buffer index */
		pucPtr += iBufCnt;					/* increment buffer pointer */
	}
	return(iWriteCnt / iSize);				/* return element count */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
