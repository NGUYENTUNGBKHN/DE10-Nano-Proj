/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_cfg.c												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Configuration definitions of file system management					*/
/* FUNCTIONS:																*/
/*		Platform dependent I/O routines										*/
/*		File System dependent routines										*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_fs.h															*/
/*		grp_fs_cfg.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/06/14	Added config value for file name cache	*/
/*		T.Imashiki		2004/07/25	Added grp_fs_dev_tbl_cnt variable		*/
/*		K.Kaneko		2008/05/21	Deleted TEST_FS, T_KERNEL, M32R #ifdef	*/
/*									block									*/
/*									Added mdep device switch table			*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		K.Kaneko		2008/06/03	Divide device switch table to mdep_xxx/ */
/*									base/grp_fs_dev_sw_tbl.c				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2008 Grape Systems, Inc.,  All Rights Reserved.        */
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
#include "grp_types.h"
#include "grp_fs.h"
#include "grp_fs_cfg.h"

/****************************************************************************/
/*  file system management parameter										*/
/****************************************************************************/
grp_fs_param_t	grp_fs_param = {		/* file system management parameter */
	GRP_FS_MAX_MOUNT,					/* mount table count */
	GRP_FS_MAX_TASK,					/* task table count */
	GRP_FS_FBLK_SHIFT,					/* file block shift count */
	GRP_FS_DBLK_SHIFT,					/* data block shift count */
	GRP_FS_FBLK_CNT,					/* file block count */
	GRP_FS_DBLK_CNT,					/* data block count */
	GRP_FS_BLK_NHASH,					/* buffer hash count */
	GRP_FS_MAX_FILE,					/* open file count */
	GRP_FS_FILE_NHASH,					/* file hash count */
	GRP_FS_MAX_FHDL,					/* file handle count */
#ifdef	GRP_FS_FNAME_CACHE
	GRP_FS_FNAME_CACHE_CNT,				/* file name cache count */
	GRP_FS_FNAME_NHASH,					/* file name hash count */
#endif	/* GRP_FS_FNAME_CACHE */
};

/****************************************************************************/
/*  file system switch table												*/
/****************************************************************************/
extern	grp_fs_op_t			grp_fs_op_fat;		/* FAT file system */

grp_fs_type_tbl_t	grp_fs_type_tbl[] = {		/* file system type table */
	{ "fat",	&grp_fs_op_fat },				/* FAT file system */
	{ NULL,		NULL }
};
