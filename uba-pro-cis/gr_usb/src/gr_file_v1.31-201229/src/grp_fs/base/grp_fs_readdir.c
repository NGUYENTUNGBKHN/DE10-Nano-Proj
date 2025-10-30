/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_readdir.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Conversion libary for opendir/closedir/readdir interfaces			*/
/* FUNCTIONS:																*/
/*		grp_fs_opendir				open direcotry							*/
/*		grp_fs_closedir				close diretory							*/
/*		grp_fs_readdir				get next directory entry				*/
/* DEPENDENCIES:															*/
/*		grp_fs_if.h															*/
/*		grp_fs_readdir.h													*/
/*		grp_mem.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2004/04/06	Created inital version 1.0				*/
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
#include "grp_fs_if.h"
#include "grp_fs_readdir.h"
#include "grp_mem.h"

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_opendir												*/
/*																			*/
/* DESCRIPTION:	Open directroy for subsequent readdir calls					*/
/* INPUT:		pucPath:			file name								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:				error									*/
/*				others:				directory file handle					*/
/*																			*/
/****************************************************************************/
DIR *
grp_fs_opendir(
	const grp_uchar_t	*pucPath)				/* [IN]  file name */
{
	int					iFd;					/* file descriptor */
	int					iRet;					/* return value */
	grp_fs_dir_ent_t	tAttr;					/* attribute */
	DIR					*ptDirHdl;				/* directory handle */

	iRet = grp_fs_get_attr(pucPath, &tAttr);	/* get attribute */
	if (iRet != 0 || tAttr.ucType != GRP_FS_FILE_DIR)
		return(NULL);							/* not found or not dir */
	iFd = grp_fs_open(pucPath, GRP_FS_O_RDONLY, 0); /* open directory */
	if (iFd < 0)								/* open error */
		return(NULL);							/* return error */
	ptDirHdl = (DIR *) grp_mem_alloc(sizeof(DIR)); /* allocate file handle */
	if (ptDirHdl == NULL) {						/* allocate error */
		grp_fs_close(iFd);						/* close file */
		return(NULL);							/* return error */
	}
	ptDirHdl->tDirent.uiStart = ptDirHdl->tDirent.uiEnd = 0;
												/* initialize offset */
	ptDirHdl->iDirFd = iFd;						/* set file handle */
	return(ptDirHdl);							/* return directory handle */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_closedir												*/
/*																			*/
/* DESCRIPTION:	close directory opened by grp_fs_opendir					*/
/* INPUT:		ptDirHdl:			directory file handle					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_closedir(
	DIR					*ptDirHdl)				/* [IN] directory file handle */
{
	int					iRet;					/* return value */

	iRet = grp_fs_close(ptDirHdl->iDirFd);		/* close file */
	grp_mem_free(ptDirHdl);						/* free area */
	return(iRet);								/* return */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_readdir												*/
/*																			*/
/* DESCRIPTION:	get next directory entry									*/
/* INPUT:		ptDirHdl:			directory file handle					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:				error									*/
/*				others:				directory entry information				*/
/*																			*/
/****************************************************************************/
struct dirent *
grp_fs_readdir(
	DIR					*ptDirHdl)				/* [IN] directory file handle */
{
	int					iRet;					/* return value */
	grp_fs_dir_ent_t	*ptInt = &ptDirHdl->tDirent;	/* internal info */
	struct dirent		*ptExt = &ptDirHdl->tExtDirent;	/* external info */

	ptInt->uiStart = ptInt->uiEnd;				/* search next entry */
	ptInt->pucName = (grp_uchar_t *)ptExt->d_name;	/* set name buffer */
												/* set name buffer */
	ptInt->sNameSize = (short)sizeof(ptExt->d_name);/* set name buffer size */
	iRet = grp_fs_get_dirent(ptDirHdl->iDirFd, ptInt);	/* get dir entry */
	if (iRet <= 0)								/* error */
		return(NULL);							/* return error or EOF */
	ptExt->d_reclen = ptInt->sNameSize;			/* set name size */
	ptExt->d_ino = ptInt->uiFid;				/* file number */
	ptExt->d_dev = ptInt->iDev;					/* device number */
#ifdef GRP_FS_ENABLE_OVER_2G
	ptExt->d_size = ptInt->uiSize;				/* file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	ptExt->d_size = ptInt->iSize;				/* file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	switch(ptInt->ucType) {						/* file type */
	case GRP_FS_FILE_FILE:						/* regular file */
		ptExt->d_type = DT_REG;					/* reguar file */
		break;									/* break */
	case GRP_FS_FILE_DIR:						/* directory */
		ptExt->d_type = DT_DIR;					/* directory */
		break;									/* break */
	case GRP_FS_FILE_LINK:						/* link */
		ptExt->d_type = DT_LNK;					/* link file */
		break;									/* break */
	default:									/* unknown */
		ptExt->d_type = DT_UNKNOWN;				/* unknown */
		break;									/* break */
	}
	return(ptExt);								/* return found entry */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
