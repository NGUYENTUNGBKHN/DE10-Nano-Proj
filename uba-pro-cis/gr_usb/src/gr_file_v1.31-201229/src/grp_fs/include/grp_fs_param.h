#ifndef	_GRP_FS_PARAM_H_
#define	_GRP_FS_PARAM_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_param.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		External interface parameters of file system management				*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		None																*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		K.Kaneko		2008/05/12	Moved file system management parameter	*/
/*									define from grp_fs_cfg.h				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003 Grape Systems, Inc.,  All Rights Reserved.             */
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

/****************************************************************************/
/*  multi language support undef options									*/
/****************************************************************************/
#ifdef GRP_FS_MULTI_LANGUAGE
#ifdef GRP_FS_FNAME_CACHE
#undef GRP_FS_FNAME_CACHE
#endif
#ifdef GRP_FS_FAT_CACHE_BY_GET_DIRENT
#undef GRP_FS_FAT_CACHE_BY_GET_DIRENT
#endif
#endif  /* GRP_FS_MULTI_LANGUAGE */

/****************************************************************************/
/*  file system management parameters										*/
/****************************************************************************/
#define GRP_FS_MAX_MOUNT	8					/* max mount count */
#define GRP_FS_MAX_FILE		16					/* max open files */
#define GRP_FS_MAX_FHDL		16					/* max file handle */
#define GRP_FS_FBLK_SHIFT	11					/* shift for file block size */
#define GRP_FS_DBLK_SHIFT	11					/* shift for data block size */
#define GRP_FS_FBLK_CNT		4					/* file info block count */
#define GRP_FS_DBLK_CNT		16					/* data block count */
#define GRP_FS_BLK_NHASH	64					/* block hash count */
#define GRP_FS_MAX_TASK		8					/* number of parallel tasks */
#define GRP_FS_FILE_NHASH	16					/* file hash count */
#ifdef	GRP_FS_FNAME_CACHE
#define GRP_FS_FNAME_CACHE_CNT	(GRP_FS_MAX_FILE*2)	/* fname cache count */
#define GRP_FS_FNAME_NHASH	(GRP_FS_FILE_NHASH*2)	/* fname hash count */
#endif	/* GRP_FS_FNAME_CACHE */

#define GRP_FS_MAX_PATH		256					/* max path length */
#define GRP_FS_MAX_COMP		128					/* max component length */
#define GRP_FS_DEV_NAME_LEN	16					/* max dev name length */
#define GRP_FS_TYPE_LEN		16					/* max FS type length */
#define GRP_FS_MOUNT_COMP	16					/* max mount component length */
#define GRP_FS_MOUNT_PATH	64					/* max mount path length */
#define GRP_FS_VOL_NAME_LEN	16					/* max volume name length */
#define GRP_FS_DIR_NEST		64					/* max directory nest */

#endif	/* _GRP_FS_PARAM_H_ */
