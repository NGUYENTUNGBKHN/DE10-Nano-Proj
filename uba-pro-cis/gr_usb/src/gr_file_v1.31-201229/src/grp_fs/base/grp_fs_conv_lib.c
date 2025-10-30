/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_conv_lib.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Conversion libary for standard interface							*/
/* FUNCTIONS:																*/
/*		grp_fs_chmod				change file protect mode				*/
/*		grp_fs_utimes				change access/mod time					*/
/*		grp_fs_stat					get file attribute						*/
/* DEPENDENCIES:															*/
/*		grp_fs_conv.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
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
#include "grp_fs_conv.h"

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_chmod												*/
/*																			*/
/* DESCRIPTION:	Change protection mode										*/
/* INPUT:		pucPath:			file name								*/
/*				uiProtect:			protection mode							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_chmod(
	const grp_uchar_t	*pucPath,				/* [IN] file name */
	grp_uint32_t		uiProtect)				/* [IN] file prorection */
{
	int					iRet;					/* return value */
	grp_fs_dir_ent_t	tAttr;					/* attribute */

	iRet = grp_fs_get_attr(pucPath, &tAttr);	/* get attribute */
	if (iRet != 0)								/* error */
		return(iRet);							/* return error */
	tAttr.uiProtect = uiProtect;				/* set protection value */
	return(grp_fs_set_attr(pucPath, &tAttr));	/* set attribute */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_utimes												*/
/*																			*/
/* DESCRIPTION:	Change access and modification time							*/
/* INPUT:		pucPath:			file name								*/
/*				ptTimes:			access and modification time			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_utimes(
	const grp_uchar_t	*pucPath,				/* [IN] file name */
	struct timeval		*ptTimes)				/* [IN] access/mod times */
{
	int					iRet;					/* return value */
	grp_fs_dir_ent_t	tAttr;					/* attribute */

	iRet = grp_fs_get_attr(pucPath, &tAttr);	/* get attribute */
	if (iRet != 0)								/* error */
		return(iRet);							/* return error */
	tAttr.iATime = ptTimes[0].tv_sec;			/* set access time */
	tAttr.iMTime = ptTimes[1].tv_sec;			/* set modification time */
	return(grp_fs_set_attr(pucPath, &tAttr));	/* set attribute */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_stat													*/
/*																			*/
/* DESCRIPTION:	Get file attribute											*/
/* INPUT:		pucPath:			file name								*/
/* OUTPUT:		ptStat:				file attribute							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_stat(
	const grp_uchar_t	*pucPath,				/* [IN]  file name */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_fs_stat_t		*ptStat)				/* [OUT] file attribute */
#else  /* GRP_FS_ENABLE_OVER_2G */
	struct stat			*ptStat)				/* [OUT] file attribute */
#endif /* GRP_FS_ENABLE_OVER_2G */
{
	int					iRet;					/* return value */
	grp_fs_dir_ent_t	tAttr;					/* attribute */

	iRet = grp_fs_get_attr(pucPath, &tAttr);	/* get attribute */
	if (iRet != 0)								/* error */
		return(iRet);							/* return error */
	ptStat->st_dev = tAttr.iDev;				/* set device number */
	ptStat->st_rdev = tAttr.iDev;				/* set device number */
	ptStat->st_ino = tAttr.uiFid;				/* file number */
	ptStat->st_mode = tAttr.uiProtect;			/* protect mode */
	switch(tAttr.ucType) {						/* file type */
	case GRP_FS_FILE_FILE:						/* regular file */
		ptStat->st_mode |= S_IFREG;				/* set regular file bit */
		break;									/* break */
	case GRP_FS_FILE_DIR:						/* directory */
		ptStat->st_mode |= S_IFDIR;				/* set directory bit */
		break;									/* break */
	case GRP_FS_FILE_LINK:						/* link */
		ptStat->st_mode |= S_IFLNK;				/* set link bit */
		break;									/* break */
	}
	ptStat->st_nlink = 1;						/* number of links */
	ptStat->st_uid = 0;							/* clear uid */
	ptStat->st_gid = 0;							/* clear gid */
	ptStat->st_atime = tAttr.iATime;			/* access time */
	ptStat->st_mtime = tAttr.iMTime;			/* modification time */
	ptStat->st_ctime = tAttr.iCTime;			/* creation time */
#ifdef GRP_FS_ENABLE_OVER_2G
	ptStat->st_size = tAttr.uiSize;				/* set file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	ptStat->st_size = tAttr.iSize;				/* set file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	return(0);									/* return 0 */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
#ifdef GRP_FS_ENABLE_OVER_2G
/****************************************************************************/
/* FUNCTION:	stat														*/
/*																			*/
/* DESCRIPTION:	Get file attribute											*/
/* INPUT:		pucPath:			file name								*/
/* OUTPUT:		ptStat:				file attribute							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
stat(
	const grp_uchar_t	*pucPath,				/* [IN]  file name */
	struct stat			*ptStat)				/* [OUT] file attribute */
{
	int					iRet;					/* return value */
	grp_fs_stat_t		tGrpStat;				/* file attribute */
	
	iRet = grp_fs_stat(pucPath, &tGrpStat);		/* Get file attribute */
	if (iRet == 0) {
		ptStat->st_dev   = tGrpStat.st_dev;
		ptStat->st_ino   = tGrpStat.st_ino;
		ptStat->st_mode  = tGrpStat.st_mode;
		ptStat->st_nlink = tGrpStat.st_nlink;
		ptStat->st_uid   = tGrpStat.st_uid;
		ptStat->st_gid   = tGrpStat.st_gid;
		ptStat->st_rdev  = tGrpStat.st_rdev;
		ptStat->st_atime = tGrpStat.st_atime;
		ptStat->st_mtime = tGrpStat.st_mtime;
		ptStat->st_ctime = tGrpStat.st_ctime;
		ptStat->st_size  = (grp_int32_t)((tGrpStat.st_size > 0x7fffffff) ?
										 0x7fffffff: tGrpStat.st_size);
	}

	return(iRet);								/* return */
}
#endif /* GRP_FS_ENABLE_OVER_2G */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
