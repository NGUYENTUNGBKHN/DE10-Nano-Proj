/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio_fopen.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library (open file)									*/
/* FUNCTIONS:																*/
/*		grp_stdio_fopen				open file								*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		<stdlib.h>															*/
/*		grp_stdio.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/10	Created inital version 1.0				*/
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
#include <string.h>
#include <stdlib.h>
#include "grp_stdio.h"

/****************************************************************************/
/* FUNCTION:	grp_stdio_fopen												*/
/*																			*/
/* DESCRIPTION:	Open file													*/
/* INPUT:		pcPath:					file path name						*/
/*				pcMode:					open mode							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:					open error							*/
/*				others:					opened file control data			*/
/*																			*/
/****************************************************************************/
FILE *
grp_stdio_fopen(
	const char			*pcPath,			/* [IN]  path name */
	const char			*pcMode)			/* [IN]  open mode */
{
	int					iFd;				/* file descriptor */
	int					iMode;				/* open mode */
	int					iStatus;			/* status */
	grp_uchar_t			*pucAlloc;			/* allocated area */
	FILE				*ptFile;			/* file control data */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_uioffset_t		uiOff;				/* offset */
#endif /* GRP_FS_ENABLE_OVER_2G */

	switch(pcMode[0]) {
	case 'r':								/* read mode */
		iStatus = GRP_STDIO_READ;			/* read mode */
		if (pcMode[1] == '+') {				/* plus mode */
			iStatus |= GRP_STDIO_PLUS;		/* plus mode */
			iMode = O_RDWR;					/* read write mode */
		} else								/* not plus mode */
			iMode = O_RDONLY;				/* read only */
		break;
	case 'w':								/* write mode */
		iStatus = GRP_STDIO_WRITE;			/* write mode */
		if (pcMode[1] == '+') {				/* plus mode */
			iStatus |= GRP_STDIO_PLUS;		/* plus mode */
			iMode = O_RDWR;					/* read write mode */
		} else								/* not plus mode */
			iMode = O_WRONLY;				/* write only */
		iMode |= (O_CREAT|O_TRUNC);			/* create and truncate */
		break;
	case 'a':								/* append mode */
		iStatus = GRP_STDIO_WRITE;			/* write mode */
		if (pcMode[1] == '+') {				/* plus mode */
			iStatus |= GRP_STDIO_PLUS;		/* plus mode */
			iMode = O_RDWR;					/* read write mode */
		} else								/* not plus mode */
			iMode = (O_WRONLY|O_APPEND);	/* write only and append mode */
		iMode |= O_CREAT;					/* create */
		break;
	default:
		return(NULL);						/* error */
	}
	pucAlloc = malloc(sizeof(FILE) + GRP_STDIO_BUF);
											/* allocate buffer */
	if (pucAlloc == NULL)					/* not allocated */
		return(NULL);						/* return error */
	if ((iFd = open(pcPath, iMode, GRP_STDIO_CRT_PROT)) < 0) {
		free(pucAlloc);						/* free memory */
		return(NULL);						/* return error */
	}
	if (pcMode[0] == 'a')					/* append mode */
#ifdef GRP_FS_ENABLE_OVER_2G
		lseek4G(iFd, 0, SEEK_END, &uiOff);	/* seek to end */
#else  /* GRP_FS_ENABLE_OVER_2G */
		lseek(iFd, 0, SEEK_END);			/* seek to end */
#endif /* GRP_FS_ENABLE_OVER_2G */
	memset(pucAlloc, 0, sizeof(FILE) + GRP_STDIO_BUF); /* clear area */
	ptFile = (FILE *) pucAlloc;				/* set file pointer */
	ptFile->iFd = iFd;						/* set file descriptor */
	ptFile->iStatus = iStatus;				/* set status */
	ptFile->pucBuf = pucAlloc + sizeof(FILE);/* set buffer */
	ptFile->iBufSize = GRP_STDIO_BUF;		/* set buffer size */
	return(ptFile);							/* return file control pointer */
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
