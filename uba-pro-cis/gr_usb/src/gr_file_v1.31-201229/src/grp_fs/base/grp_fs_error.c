/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_error.c												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Error message handling for GRP_FS									*/
/* FUNCTIONS:																*/
/*		grp_fs_err					convert error number to message string	*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_fs_if.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*									Fixed spell miss at _grp_fs_err_msg		*/
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
#include <string.h>
#include "grp_fs_sysdef.h"
#include "grp_fs_if.h"

#if(GRP_FS_MINIMIZE_LEVEL < 1)
static char	*_grp_fs_err_msg[] = {					/* error message table */
	"\"I/O error\"",								/* I/O error */
	"\"invalid file handle\"",						/* invalid file handle */
	"\"invalid file system\"",						/* invalid file system */
	"\"no memory\"",								/* no memory */
	"\"need to check file system\"",				/* need to check FS */
	"\"bad device name/number\"",					/* bad device name/number */
	"\"permission denied\"",						/* permission denied */
	"\"bad FS name\"",								/* bad FS name */
	"\"too many open/mount\"",						/* too many open/mount */
	"\"already used\"",								/* already used */
	"\"still used or under proc\"",					/* still used */
	"\"too long path name\"",						/* too long name */
	"\"file not found\"",							/* file no found */
	"\"bad mode\"",									/* bad mode */
	"\"should be closed\"",							/* should be closed */
	"\"bad offset\"",								/* bad offset */
	"\"no space\"",									/* no space left */
	"\"bad name\"",									/* bad name */
	"\"bad directory\"",							/* bad directory */
	"\"bad file type information\"",				/* bad file type */
	"\"cross file system\"",						/* cross file system */
	"\"bad parameter\"",							/* bad parameter */
	"\"too big block size\"",						/* too big block size */
	"\"semaphore error\"",							/* semaphore error */
	"\"not supported\"",							/* not supported */
	NULL
};

extern int sprintf(char *pcBuf, const char *pcFormat, ...);

/****************************************************************************/
/* FUNCTION:	grp_fs_err													*/
/*																			*/
/* DESCRIPTION:	Convert error number to error message string				*/
/*				If pcMsgBuf is not NULL, the related error message is		*/
/*				stored into pcMsgBuf.  Otherwise, static error message		*/
/*				area is used to return error message.						*/
/* INPUT:		iErrNo:				error number							*/
/* OUTPUT:		pcMsgBuf:			error message							*/
/*																			*/
/* RESULT:		pointer to error message									*/
/*																			*/
/****************************************************************************/
char *grp_fs_err(
	int				iErrNo,							/* [IN]  error number */
	char			*pcMsgBuf)						/* [OUT] message buffer */
{
	char			*pcRetMsg;						/* message to return */
	static char		 acOtherMsg[32];				/* static mssage buffer */

	if (iErrNo >= GRP_FS_ERR_NOT_SUPPORT && iErrNo <= GRP_FS_ERR_IO) {
		iErrNo = -(iErrNo - GRP_FS_ERR_IO);			/* get error msg index */
		pcRetMsg = _grp_fs_err_msg[iErrNo];			/* message to return */
		if (pcMsgBuf != NULL) {						/* message buffer exists */
			strcpy(pcMsgBuf, pcRetMsg);				/* copy it */
			pcRetMsg = pcMsgBuf;					/* change to msg buffer */
		}
		return(pcRetMsg);							/* return message */
	}
	pcRetMsg = (pcMsgBuf)? pcMsgBuf: acOtherMsg;	/* set message buffer */
	if (iErrNo >= 0)								/* 0 or positive value */
		sprintf(pcRetMsg, "%d", iErrNo);			/* convert to decimal */
	else											/* negative value */
		sprintf(pcRetMsg, "-0x%x", -iErrNo);		/* convert to hex */
	return(pcRetMsg);								/* return message */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
