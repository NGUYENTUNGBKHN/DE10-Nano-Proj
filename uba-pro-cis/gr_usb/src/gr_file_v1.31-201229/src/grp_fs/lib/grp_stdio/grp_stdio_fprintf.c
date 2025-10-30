/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fprintf.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (write data to file with format)				*/
/* FUNCTIONS:																*/
/*		grp_stdio_fprintf			output data with format					*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		<stdarg.h>															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		T.Imashiki		2010/11/16	Added vsnprintf error check				*/
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
#include <stdarg.h>
#include "grp_stdio.h"
#ifdef	WIN32
extern int	_vsnprintf(char *pcBuf, size_t iSize, const char *pcFormat, 
						va_list vap);
#define vsnprintf	_vsnprintf
#else	/* WIN32 */
#ifdef	NO_VSNPRINTF
extern int	vsprintf(char *pcBuf, const char *pcFormat, va_list vap);
#endif	/* NO_VSNPRINTF */
#endif	/* WIN32 */

/****************************************************************************/
/* FUNCTION:	grp_stdio_fprintf											*/
/*																			*/
/* DESCRIPTION:	Write data to file with format								*/
/* INPUT:		ptFile:					file control data					*/
/*				pcFormat:				format data							*/
/*				...						parameters							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						output error						*/
/*				others:					number of bytes written				*/
/*																			*/
/****************************************************************************/
int
grp_stdio_fprintf(
	FILE			*ptFile,				/* [IN]  file control data */
	const char		*pcFormat,				/* [IN]  format data */
	...)									/* [IN]  parameter */
{
	int				iLen;					/* I/O length */
	va_list			vap;					/* parameter list */
	grp_uchar_t		aucBuf[GRP_STDIO_PBUF];	/* I/O buffer */
	
	va_start(vap, pcFormat);				/* get parameter list */
#ifdef	NO_VSNPRINTF
	iLen = vsprintf((char *)aucBuf, pcFormat, vap);
#else
	iLen = vsnprintf((char *)aucBuf, sizeof(aucBuf), pcFormat, vap);
#endif
											/* make message */
	if (iLen < 0 || iLen >= sizeof(aucBuf))	/* error or buffer full */
		return(-1);							/* error */
	return(grp_stdio_fwrite(aucBuf, 1, iLen, ptFile)); /* write data */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
