/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fclose.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (close file)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fclose			close file								*/
/* DEPENDENCIES:															*/
/*		<stdlib.h>															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
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
#include <stdlib.h>
#include "grp_stdio.h"

/****************************************************************************/
/* FUNCTION:	grp_stdio_fclose											*/
/*																			*/
/* DESCRIPTION:	Close file													*/
/* INPUT:		ptFile:					file control data					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:					open error							*/
/*				others:					opened file control data			*/
/*																			*/
/****************************************************************************/
int
grp_stdio_fclose(
	FILE			*ptFile)				/* [IN]  file control data */
{
	int				iRet_fflush;			/* return value of fflush */
	int				iRet;					/* return value */

	iRet_fflush = grp_stdio_fflush(ptFile);	/* flush buffer */
	if (ptFile->iStatus & (GRP_STDIO_STDIN|GRP_STDIO_STDOUT))
		return(iRet_fflush);				/* return */
	iRet = close(ptFile->iFd);				/* close file */
	free(ptFile);							/* free allocated area */
	if (iRet_fflush != 0 || iRet != 0)		/* error detected */
		return(-1);							/* return error */
	return(0);								/* return success */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
