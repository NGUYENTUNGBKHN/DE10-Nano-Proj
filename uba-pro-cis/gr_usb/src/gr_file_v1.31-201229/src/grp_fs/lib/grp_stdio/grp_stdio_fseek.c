/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fseek.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (seek file)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fseek				seek file								*/
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
/* FUNCTION:	grp_stdio_fseek												*/
/*																			*/
/* DESCRIPTION:	Seek file													*/
/* INPUT:		ptFile:					file control data					*/
/*				iOffset:				file offset							*/
/*				iMode:					seek mode							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						I/O error							*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_stdio_fseek(							/* seek file */
	FILE				*ptFile,			/* [IN]  file control data */
	long				iOffset,			/* [IN]  offset */
	int					iMode)				/* [IN]  seek mode */
{
	grp_ioffset_t		iNewOff;			/* new offset */

	if (grp_stdio_fflush(ptFile) < 0)		/* flush buffer failed */
		return(-1);							/* return error */
	ptFile->iStatus &= ~GRP_STDIO_EOF;		/* clear EOF flag */
	if ((ptFile->iStatus & (GRP_STDIO_STDIN|GRP_STDIO_STDOUT)) == 0) {
		iNewOff = lseek(ptFile->iFd, iOffset, iMode); /* seek file */
		if (iNewOff < 0) {					/* seek failed */
			ptFile->iError = (int)iNewOff;	/* set error number */
			return(-1);						/* return error */
		}
	}
	return(0);								/* return success */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
