/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_gets.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (get line)										*/
/* FUNCTIONS:																*/
/*		grp_stdio_fgets				get line								*/
/* DEPENDENCIES:															*/
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
#include "grp_stdio.h"

/****************************************************************************/
/* FUNCTION:	grp_stdio_fgets												*/
/*																			*/
/* DESCRIPTION:	Get a line													*/
/* INPUT:		iSize:					size of line buffer					*/
/*				ptFile:					file control data					*/
/* OUTPUT:		pcStr:					line data							*/
/*																			*/
/* RESULT:		string:					I/O error							*/
/*				others:					got character						*/
/*																			*/
/****************************************************************************/
char *
grp_stdio_fgets(
	char			*pcStr,					/* [OUT] line buffer */
	int				iSize,					/* [IN]  size of line buffer */
	FILE			*ptFile)				/* [IN]  file control data */
{
	char			*pc;					/* buffer pointer */
	char			*pcEnd;					/* end pointer */
	int				iChar;					/* character */

	pcEnd = &pcStr[iSize - 1];				/* end of buffer */
	for (pc = pcStr; pc < pcEnd; pc++) {	/* loop until end of buffer */
		iChar = getc(ptFile);				/* get character */
		if (iChar == '\n') {				/* end of line */
			*pc++ = (grp_uchar_t)iChar;		/* set in buffer */
			break;							/* break */
		} else if (iChar < 0) {				/* EOF or error */
			if (ferror(ptFile))				/* error */
				return(NULL);				/* return NULL */
			break;
		}
		*pc = (grp_uchar_t)iChar;			/* set charecter */
	}
	if (pc == pcStr)						/* EOF */
		return(NULL);						/* return NULL */
	*pc = 0;								/* null terminate */
	return(pcStr);							/* return top pointer */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
