/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fflush.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (flush file)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fflush			flush buffer							*/
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
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2016 Grape Systems, Inc.,  All Rights Reserved.        */
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
/* FUNCTION:	grp_stdio_fflush											*/
/*																			*/
/* DESCRIPTION:	flush buffer												*/
/* INPUT:		ptFile:					file control data					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						I/O error							*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_stdio_fflush(
	FILE			*ptFile)				/* [IN]  file control data */
{
#ifdef GRP_FS_ENABLE_OVER_2G
    int					ret = 0;
#endif /* GRP_FS_ENABLE_OVER_2G */
	grp_int32_t			iSize;				/* read size */

	if (ptFile->iStatus & GRP_STDIO_READ) {	/* read mode */
		/****************************************************/
		/* adjust seek pointer and purge buffer cache		*/
		/****************************************************/
		if ((ptFile->iStatus & GRP_STDIO_STDIN)	== 0) { /* not standard input */
#ifdef GRP_FS_ENABLE_OVER_2G
			grp_uioffset_t	uiOff;			/* offset */
			ret = lseek4G(ptFile->iFd, (ptFile->iInBuf - ptFile->iNext),
							SEEK_CUR|SEEK_MINUS, &uiOff);
											/* adujst seek pointer */
			if (ret < 0) {					/* seek error */
				ptFile->iError = (int)ret;	/* remember error */
#else  /* GRP_FS_ENABLE_OVER_2G */
			grp_ioffset_t	iOff;			/* offset */
			iOff = lseek(ptFile->iFd, ptFile->iNext - ptFile->iInBuf, 
								SEEK_CUR);	/* adujst seek pointer */
			if (iOff < 0) {					/* seek error */
				ptFile->iError = (int)iOff;	/* remember error */
#endif /* GRP_FS_ENABLE_OVER_2G */
				return(-1);					/* seek error */
			}
		}
		ptFile->iInBuf = 0;					/* reset buffer */
		ptFile->iNext = 0;					/* reset index */
		return(0);							/* return success */
	}
	if (ptFile->iStatus & GRP_STDIO_WRITE) {/* write mode */
		if (ptFile->iNext != 0) {			/* data in buffer */
			iSize = write(ptFile->iFd, ptFile->pucBuf, ptFile->iNext);
											/* write data in buffer */
			if (iSize != ptFile->iNext) {	/* write failed */
				if (iSize < 0) {			/* write error */
					ptFile->iError = iSize;	/* set error */
				} else {
					/****************************************************/
					/* copy remaining data								*/
					/****************************************************/
					grp_uchar_t *pucSrc = &ptFile->pucBuf[iSize];/* source */
					grp_uchar_t *pucDst = ptFile->pucBuf;	/* destination */
					iSize = ptFile->iNext - iSize;			/* copy size */
					ptFile->iNext = iSize;					/* set next index */
					while (iSize-- > 0)						/* copy loop */
						*pucDst++ = *pucSrc++;				/* copy data */
				}
				return(-1);
			}
			ptFile->iNext = 0;				/* reset next index */
		}
		return(0);							/* return success */
	}
	if (ptFile->iStatus & GRP_STDIO_STDIN)	/* standard input */
		return(grp_stdio_io_stdin(ptFile, NULL, 0));	/* flush stdin */
	if (ptFile->iStatus & GRP_STDIO_STDOUT)	/* standard input */
		return(grp_stdio_io_stdout(ptFile, NULL, 0));	/* flush stdout */
	return(-1);								/* invalid mode */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
