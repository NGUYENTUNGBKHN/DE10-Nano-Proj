/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_get_cwd_lib.c										*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Get current working diretory name									*/
/* FUNCTIONS:																*/
/*		grp_fs_get_cwd				get current working diretory name		*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_fs_if.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2007/02/20	Added type casts for 16 bit CPU support	*/
/*		K.Kaneko		2007/11/1	Changed copy size in grp_fs_get_cwd		*/
/*									function								*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2010/10/15	Fixed spell miss at comment				*/
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
/****************************************************************************/
/* FUNCTION:	grp_fs_get_cwd												*/
/*																			*/
/* DESCRIPTION:	Get current working diretory name							*/
/* INPUT:		iPathBufLen:				max path buffer length			*/
/*				iSepChar:					separator character				*/
/* OUTPUT:		pucPath:					current working directory		*/
/*																			*/
/* RESULT:		GRP_FS_ERR_TOO_LONG:		not fit in path buffer			*/
/*				GRP_FS_ERR_IO:				I/O error						*/
/*				GRP_FS_ERR_NOT_FOUND:		not found current directory		*/
/*				GRP_FS_ERR_FS:				bad file system					*/
/*				GRP_FS_ERR_NOMEM:			no valid cache buffer			*/
/*				0:							success							*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_cwd(
	grp_uchar_t			*pucPath,				/* [OUT] path name */
	int					iPathBufLen,			/* [IN]  max path buf length */
	int					iSepChar)				/* [IN]  separator char */
{
	int					i;						/* loop count */
	int					iRet;					/* return value */
	int					iLen;					/* length */
	int					iFd;					/* file descriptor */
	int					iPos = iPathBufLen;		/* path position */
	int					iDev;					/* device number */
	int					iDevNext;				/* next device number */
	grp_uint32_t		uiFid;					/* file ID */
	grp_uint32_t		uiFidNext;				/* next file ID */
	grp_uchar_t			*ptName;				/* name */
	grp_fs_dir_ent_t	tEnt;					/* directory entry */
	grp_fs_mnt_info_t	tMntInfo;				/* mount information */
	grp_uchar_t			aucFName[GRP_FS_MAX_COMP]; /* file name */
	grp_uchar_t			aucLName[GRP_FS_MAX_COMP]; /* long name */
	grp_uchar_t			aucDir[(GRP_FS_DIR_NEST + 1) * 3]; /* parent dirs */

	pucPath[0] = 0;								/* init as null */
	iRet = grp_fs_get_attr((grp_uchar_t *)".", &tEnt);
												/* get info for curent dir */
	if (iRet < 0)								/* error */
		return(iRet);							/* return error */
	uiFid = tEnt.uiFid;							/* get current file ID */
	iDev = tEnt.iDev;							/* current device number */
	strcpy((char *)aucDir, "..");				/* parent directory */
	for (i = 0; i < GRP_FS_DIR_NEST; i++) {		
		iRet = grp_fs_get_attr(aucDir, &tEnt);	/* get info for parent dir */
		if (iRet < 0)							/* error */
			return(iRet);						/* return error */
		uiFidNext = tEnt.uiFid;					/* get parent file ID */
		iDevNext = tEnt.iDev;					/* parent device number */
		if (uiFidNext == uiFid && iDevNext == iDev) /* root */
			break;								/* stop search */
		iFd = grp_fs_open(aucDir, GRP_FS_O_RDONLY, 0); /* open parent */
		if (iFd < 0)							/* error */
			return(iFd);						/* return error */
		tEnt.uiStart = tEnt.uiEnd = 0;			/* init offset */
		tEnt.sNameSize = sizeof(aucFName);		/* set name buffer size */
		tEnt.pucName = aucFName;				/* set name buffer */
		ptName = aucFName;						/* set file name pointer */
		while ((iRet = grp_fs_get_dirent(iFd, &tEnt)) > 0) {
			if (tEnt.ucType == GRP_FS_FILE_LINK
				&& tEnt.uiFid == 0) {			/* FAT long name */
				strncpy((char *)aucLName, (char *)aucFName, sizeof(aucLName));
												/* save name  */
				if(sizeof(aucLName) <= tEnt.sNameSize) { /* check over buffer */
					aucLName[sizeof(aucLName) - 1] = '\0'; /* set null at buffer end */
				} else {						/* within buffer */
					aucLName[tEnt.sNameSize] = '\0'; /* null terminate */
				}
				ptName = aucLName;				/* use long name */
			} else if (tEnt.ucType == GRP_FS_FILE_DIR
					&&tEnt.uiFid == uiFid && tEnt.iDev == iDev) {/* match */
				iLen = (int)strlen((char *)ptName);	/* file name length */
				if (iPos - (iLen + 1) < 0) {	/* not fit in buffer */
					grp_fs_close(iFd);			/* close file */
					return(GRP_FS_ERR_TOO_LONG);/* return error */
				}
				iPos -= (iLen + 1);				/* get copy postion */
				strcpy((char *)&pucPath[iPos], (char *)ptName); /* set name */
				pucPath[iPos + iLen] = (grp_uchar_t)iSepChar;
												/* set path separator */
				break;
			} else {							/* not match */
				ptName = aucFName;				/* use short entry name */
			}
			tEnt.sNameSize = sizeof(aucFName);	/* set name buffer size */
			tEnt.uiStart = tEnt.uiEnd;			/* search next */
		}
		grp_fs_close(iFd);						/* close file */
		if (iRet < 0)							/* error */
			return(iRet);						/* return error */
		if (iRet == 0) 							/* not found */
			return(GRP_FS_ERR_NOT_FOUND);		/* return not found error */
		uiFid = uiFidNext;						/* next file id */
		iDev = iDevNext;						/* next device id */
		strcpy((char *)&aucDir[i * 3 + 2], "/.."); /* upper parent */
	}
	if (i >= GRP_FS_DIR_NEST)					/* over nesting */
		return(GRP_FS_ERR_NOT_FOUND);			/* return not found error */
	iRet = grp_fs_get_mnt_by_dev(iDev, &tMntInfo); /* get mount info */
	if (iRet < 0)								/* error */
		return(iRet);							/* return error */
	iLen = (int)strlen((char *)tMntInfo.aucPath);	/* get root name */
	if (iPos == iPathBufLen)					/* no path */
		iPos--;									/* space for NULL */
	if (iPos - iLen < 0)  						/* not fit */
		return(GRP_FS_ERR_TOO_LONG);			/* return error */
	memmove((char *)&pucPath[iLen], (char *)&pucPath[iPos],
				(grp_size_t)(iPathBufLen - iPos));
	memcpy((char *)pucPath, (char *)tMntInfo.aucPath,
				(grp_size_t)iLen);				/* set name */
	pucPath[iLen + (iPathBufLen - iPos) - 1] = 0; /* NULL terminate */
	return(0);									/* return success */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
