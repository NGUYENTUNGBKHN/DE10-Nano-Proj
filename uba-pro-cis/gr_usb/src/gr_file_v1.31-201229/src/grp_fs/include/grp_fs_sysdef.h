#ifndef	_GRP_FS_SYSDEF_H_
#define	_GRP_FS_SYSDEF_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_sysdef.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		GR-FILE system define												*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		None																*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		K.Kaneko		2008/05/21	Created inital version 1.0				*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2010/11/16	Added GRP_FS_FAT_TRY_NO_NUM_SHORT option*/
/*		K.Kaneko		2011/05/23	Added GRP_FS_ASYNC_UNMOUNT and			*/
/*									GRP_FS_UPDATE_ARCHIVE and				*/
/*									GRP_FS_FAST_MAKE_SNAME options			*/
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2008-2016 Grape Systems, Inc.,  All Rights Reserved.        */
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

#define	GRP_FS								/* essential option */

/*	#define	GRP_FS_SHARE_OPEN				*/	/* task file handle sharing */
#define	GRP_FS_FAT_DIRECT_IO				/* use direct I/O function */
/*	#define	GRP_FS_FNAME_CACHE				*/	/* use filename cache */
/*	#define	GRP_FS_FAT_CACHE_BY_GET_DIRENT	*/	/* use last dirent info to filename cache */
/*	#define	GRP_FS_FAT_NO_DIR_SIZE_INFO		*/	/* no calculate directory size */
/*	#define	GRP_FS_RAM_DISK					*/	/* use RAM DISK sample driver */
/*	#define	GRP_FS_TRACE					*/	/* use trace function in processing */
#define	GRP_MEM								/* use GR-FILE memory control function */
#define	NON_POSIX							/* non posix environment */
/*	#define	WIN32							*/	/* use Windows test environment */
#define	GRP_FS_ENABLE_OVER_2G				/* enable larger file size than 2G-1 */

#define GRP_FS_MINIMIZE_LEVEL		0		/* GR-FILE minimize level */
											/*	0: not minimize */
											/*	1: POSIX,C STDIO delete */
											/*	2: LFN not support */
/*	#define	GRP_FS_FAT_TRY_NO_NUM_SHORT		*/	/* not to try no number short name */
/*	#define	GRP_FS_ASYNC_UNMOUNT			*/	/* unmount with the other volume in IO */
/*	#define	GRP_FS_UPDATE_ARCHIVE			*/	/* update archive attribute */
/*	#define	GRP_FS_FAST_MAKE_SNAME			*/	/* fast make short name */

/*	#define	GRP_FS_RET_BUSY_AT_CLOSE		*/	/* busy is error at file close */
#define	GRP_STDIO							/* use GRP-STDIO library */
#define	NO_VSNPRINTF						/* not use vsnprintf function */
#define	GR_USB								/* use FSIF driver */
/*	#define GRP_FS_PORTING_DEVICE_IO		*/	/* use porting device I/O routines */


#endif	/* _GRP_FS_SYSDEF_H_ */
