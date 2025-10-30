/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_default_io.c										*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (stdin/out/err I/O)							*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
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

static FILE	_grp_stdio_ctl[2] = {
	{ -1, 0, GRP_STDIO_STDIN,  0, 0, 0, NULL },		/* stdin */
	{ -1, 0, GRP_STDIO_STDOUT, 0, 0, 0, NULL },		/* stdout/stderr */
};
FILE	*grp_stdio_stdin = &_grp_stdio_ctl[0];		/* stdin */
FILE	*grp_stdio_stdout = &_grp_stdio_ctl[1];		/* stdout */
FILE	*grp_stdio_stderr = &_grp_stdio_ctl[1];		/* stderr */

static grp_stdio_io_func_t _grp_stdio_not_support; /* not supported */

grp_stdio_io_func_t	*grp_stdio_io_stdin = _grp_stdio_not_support; /* stdin */
grp_stdio_io_func_t	*grp_stdio_io_stdout = _grp_stdio_not_support;/* stdout */

/****************************************************************************/
/* FUNCTION:	_grp_stdio_not_support										*/
/*																			*/
/* DESCRIPTION:	Not supported I/O											*/
/* INPUT:		ptFile:					file control data					*/
/*				pucBuf:					I/O buffer							*/
/*				iSize:					I/O size							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:						error								*/
/*																			*/
/****************************************************************************/
static grp_isize_t
_grp_stdio_not_support(
	FILE				*ptFile,			/* [IN]  file control data */
	grp_uchar_t			*pucBuf,			/* [IN/OUT] I/O buffer */
	grp_isize_t			iSize)				/* [IN]  I/O size */
{
	return(-1);								/* return error */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
