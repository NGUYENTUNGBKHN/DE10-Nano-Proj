/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_ftell4G.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (get current seek pointer)						*/
/* FUNCTIONS:																*/
/*		grp_stdio_ftell4G			get current pointer						*/
/* DEPENDENCIES:															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		K.Kaneko		2016/03/16	Created inital version 1.0				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2016 Grape Systems, Inc.,  All Rights Reserved.             */
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
#ifdef GRP_FS_ENABLE_OVER_2G
#if(GRP_FS_MINIMIZE_LEVEL < 1)
#include "grp_stdio.h"

/****************************************************************************/
/* FUNCTION:	grp_stdio_ftell4G											*/
/*																			*/
/* DESCRIPTION:	Get current seek pointer									*/
/* INPUT:		ptFile:					file control data					*/
/* OUTPUT:		puiOffset:				current seek pointer				*/
/*																			*/
/* RESULT:		-1:						I/O error							*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_stdio_ftell4G(							/* seek file */
	FILE				*ptFile,			/* [IN]  file control data */
	grp_uioffset_t		*puiOffset)			/* [OUT] file control data */
{
	grp_uioffset_t		uiOff;				/* current offset */
	int					iRet;				/* return value */

	if (ptFile->iStatus & (GRP_STDIO_STDIN|GRP_STDIO_STDOUT)){ /* stdin/out */
		puiOffset = 0;
		return(0);							/* return success */
	}
	iRet = lseek4G(ptFile->iFd, 0, SEEK_CUR, &uiOff); /* get current pointer */
	if (iRet < 0) {							/* seek failed */
		ptFile->iError = (int)iRet;			/* set error number */
		return(-1);							/* return error */
	}
	if (ptFile->iStatus & GRP_STDIO_WRITE){	/* write mode */
		*puiOffset = (uiOff + ptFile->iNext); /* set seek pointer */
		return(0);							/* return success */
	}
	if (ptFile->iStatus & GRP_STDIO_READ){	/* read mode */
		*puiOffset = (uiOff + ptFile->iNext - ptFile->iInBuf);
											/* set seek pointer */
		return(0);							/* return success */
	}
	return(-1);								/* bad mode */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
#endif