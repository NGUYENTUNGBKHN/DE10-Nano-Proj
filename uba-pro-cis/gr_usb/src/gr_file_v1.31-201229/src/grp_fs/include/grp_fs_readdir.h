#ifndef	_GRP_FS_READDIR_H_
#define	_GRP_FS_READDIR_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_readdir.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Conversion libary for opendir/closedir/readdir interfaces			*/
/* FUNCTIONS:																*/
/*		grp_fs_opendir				open direcotry							*/
/*		grp_fs_closedir				close diretory							*/
/*		grp_fs_readdir				get next directory entry				*/
/* DEPENDENCIES:															*/
/*		grp_fs_if.h															*/
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

#define NAME_MAX		GRP_FS_MAX_PATH			/* path name max length */

/****************************************************************************/
/* exported directory entry information										*/
/****************************************************************************/
struct dirent {
	int					d_dev;					/* device number */
	int					d_type;					/* file type */
	grp_uint32_t		d_ino;					/* file number */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_uint32_t		d_size;					/* file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	grp_int32_t			d_size;					/* file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	int					d_reclen;				/* file name size */
	char				d_name[NAME_MAX];		/* file name buffer */
};

/* d_type */
#define DT_UNKNOWN		0						/* unknown */
#define DT_REG			1						/* regular file */
#define DT_DIR			2						/* directory */
#define DT_LNK			3						/* link file */

/****************************************************************************/
/* directory file handle													*/
/****************************************************************************/
typedef struct _DIR {
	int					iDirFd;					/* file descriptor */
	struct dirent		tExtDirent;				/* exported directroy info */
	grp_fs_dir_ent_t	tDirent;				/* internal directory info */
} DIR;

/****************************************************************************/
/* exported interfaces														*/
/****************************************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
#define opendir(pcPath)		grp_fs_opendir((const grp_uchar_t *)(pcPath))
#define closedir(ptDirHdl)	grp_fs_closedir(ptDirHdl)
#define readdir(ptDirHdl)	grp_fs_readdir(ptDirHdl)
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
DIR * grp_fs_opendir(							/* open directory */
	const grp_uchar_t	*pucPath);				/* [IN]  file name */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_closedir(								/* close directory */
	DIR					*ptDirHdl);				/* [IN] directory file handle */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
struct dirent *grp_fs_readdir(					/* get next directory entry */
	DIR					*ptDirHdl);				/* [IN] directory file handle */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#endif	/* _GRP_FS_READDIR_H_ */
