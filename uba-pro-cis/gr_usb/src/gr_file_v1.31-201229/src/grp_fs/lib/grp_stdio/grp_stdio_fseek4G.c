/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fseek4G.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (seek file)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fseek4G			seek file								*/
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
/* FUNCTION:	grp_stdio_fseek4G											*/
/*																			*/
/* DESCRIPTION:	Seek file													*/
/* INPUT:		ptFile:					file control data					*/
/*				uiOffset:				file offset							*/
/*				iMode:					seek mode							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						I/O error							*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_stdio_fseek4G(							/* seek file */
	FILE				*ptFile,			/* [IN]  file control data */
	unsigned long		uiOffset,			/* [IN]  offset */
	int					iMode)				/* [IN]  seek mode */
{
	grp_uioffset_t		uiNewOff;			/* new offset */
	int					iRet;				/* return value */

	if (grp_stdio_fflush(ptFile) < 0) {		/* flush buffer failed */
		return(-1);							/* return error */
	}
	ptFile->iStatus &= ~GRP_STDIO_EOF;		/* clear EOF flag */
	if ((ptFile->iStatus & (GRP_STDIO_STDIN|GRP_STDIO_STDOUT)) == 0) {
		iRet = lseek4G(ptFile->iFd, uiOffset, iMode, &uiNewOff); /* seek file */
		if (iRet < 0) {						/* seek failed */
			ptFile->iError = (int)iRet;		/* set error number */
			return(-1);						/* return error */
		}
	}
	return(0);								/* return success */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
#endif