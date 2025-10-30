#ifndef	_GRP_FS_IF_H_
#define	_GRP_FS_IF_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_if.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for exported interface of file system management		*/
/*		to user																*/
/* FUNCTIONS:																*/
/*		grp_fs_mount				mount a file system						*/
/*		grp_fs_unmount				unmount a file system					*/
/*		grp_fs_open					open file								*/
/*		grp_fs_close				close file								*/
/*		grp_fs_read					read file								*/
/*		grp_fs_write				write file								*/
/*		grp_fs_lseek				set file I/O position					*/
/*		grp_fs_create				create file								*/
/*		grp_fs_unlink				unlink file								*/
/*		grp_fs_rename				rename file								*/
/*		grp_fs_get_attr				get file attribute 						*/
/*		grp_fs_set_attr				set file attribute						*/
/*		grp_fs_truncate				truncate a file							*/
/*		grp_fs_get_dirent			get direcotry entry information			*/
/*		grp_fs_chdir				change directory						*/
/*		grp_fs_get_cwd				get current working directory			*/
/*		grp_fs_sync					write back modification					*/
/*		grp_fs_get_mnt				get all file system information			*/
/*		grp_fs_get_mnt_by_dev		get FS information by dev number		*/
/*		grp_fs_get_mnt_by_name		get FS information by dev name			*/
/*		grp_fs_invalidate_fs_dev	invalidate FS device status				*/
/*		grp_fs_check_fs_dev			check FS device status					*/
/*		grp_fs_check_volume			check volume name						*/
/*		grp_fs_get_error			get write error data					*/
/*		grp_fs_task_free_env		free my environment						*/
/*		grp_fs_task_free_env_by_id	free task enviroment by task id			*/
/*		grp_fs_task_free_all_env	free all environment					*/
/*		grp_fs_init					init/reinit management					*/
/*		grp_fs_err					convert error no to msg					*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_fs_param.h														*/
/*		grp_fs_mdep_types.h													*/
/*		grp_fs_multi_language_if.h											*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2003/12/19	Deleted backspace miss-inserted at 		*/
/*									making release file						*/
/*		T.Imashiki		2004/07/25	Added GRP_FS_NO_UPD_ACCTIME				*/
/*									Added GRP_FS_NO_MNT_FLAG				*/
/*									Added GRP_FS_NO_CRT_ACCTIME				*/
/*		T.Imashiki		2005/02/10	Changed return types of grp_fs_read,	*/
/*									grp_fs_write and grp_fs_get_error for	*/
/*									16 bit CPU support						*/
/*		T.Imashiki		2006/10/31	Added GRP_FS_SYNC_HINT option to		*/
/*									grp_fs_sync call						*/
/*		M.Toyama		2008/03/05	Include multi language support function	*/
/*									call									*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		T.Imashiki		2010/11/16	Added mount option flags not returned	*/
/*		K.Kaneko					in grp_fs_mnt_info_t					*/
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
/*									Added definition GRP_FS_SEEK_MINUS		*/
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
#include "grp_types.h"
#include "grp_fs_param.h"
#include "grp_fs_mdep_types.h"

/****************************************************************************/
/*  error numbers															*/
/****************************************************************************/
#define GRP_FS_ERR(num)		-(('F'<<8)|(num))	/* FS error number */
#define GRP_FS_ERR_IO			GRP_FS_ERR(1)	/* I/O error */
#define GRP_FS_ERR_FHDL			GRP_FS_ERR(2)	/* invalid file handle */
#define GRP_FS_ERR_FS			GRP_FS_ERR(3)	/* invalid file system */
#define GRP_FS_ERR_NOMEM		GRP_FS_ERR(4)	/* no memory */
#define GRP_FS_ERR_NEED_CHECK	GRP_FS_ERR(5)	/* need to check file system */
#define GRP_FS_ERR_BAD_DEV		GRP_FS_ERR(6)	/* bad device name/number */
#define GRP_FS_ERR_PERMIT		GRP_FS_ERR(7)	/* permission denied */
#define GRP_FS_ERR_BAD_FSNAME	GRP_FS_ERR(8)	/* bad FS name */
#define GRP_FS_ERR_TOO_MANY		GRP_FS_ERR(9)	/* too many open/mount */
#define GRP_FS_ERR_EXIST		GRP_FS_ERR(10)	/* already used */
#define GRP_FS_ERR_BUSY			GRP_FS_ERR(11)	/* still used or under proc */
#define GRP_FS_ERR_TOO_LONG		GRP_FS_ERR(12)	/* too long path name */
#define GRP_FS_ERR_NOT_FOUND	GRP_FS_ERR(13)	/* file not found */
#define GRP_FS_ERR_BAD_MODE		GRP_FS_ERR(14)	/* bad mode */
#define GRP_FS_ERR_SHOULD_CLOSE	GRP_FS_ERR(15)	/* should be closed */
#define GRP_FS_ERR_BAD_OFF		GRP_FS_ERR(16)	/* bad offset */
#define GRP_FS_ERR_NO_SPACE		GRP_FS_ERR(17)	/* no space */
#define GRP_FS_ERR_BAD_NAME		GRP_FS_ERR(18)	/* bad name */
#define GRP_FS_ERR_BAD_DIR		GRP_FS_ERR(19)	/* bad directory */
#define GRP_FS_ERR_BAD_TYPE		GRP_FS_ERR(20)	/* bad file type information */
#define GRP_FS_ERR_XFS			GRP_FS_ERR(21)	/* cross file system */
#define GRP_FS_ERR_BAD_PARAM	GRP_FS_ERR(22)	/* bad parameter */
#define GRP_FS_ERR_TOO_BIG		GRP_FS_ERR(23)	/* too big block size */
#define GRP_FS_ERR_SEM			GRP_FS_ERR(24)	/* semaphore error */
#define GRP_FS_ERR_NOT_SUPPORT	GRP_FS_ERR(25)	/* not supported */

/****************************************************************************/
/*  file directory entry information										*/
/****************************************************************************/
typedef struct grp_fs_dir_ent {
	int				iDev;						/* device number */
	grp_uint32_t	uiFid;						/* file ID */
	grp_uchar_t		*pucName;					/* file name */
	short			sNameSize;					/* name size */
	grp_uchar_t		ucType;						/* file type */
	grp_uint32_t	uiProtect;					/* protection */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_uisize_t	uiSize;						/* file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	grp_isize_t		iSize;						/* file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	grp_int32_t		iCTime;						/* creation time */
	grp_int32_t		iMTime;						/* modify time */
	grp_int32_t		iATime;						/* access time */
	grp_uint32_t	uiAttr;						/* FS dependent attribute */
	grp_uint32_t	uiMisc;						/* FS dependent misc info */
	grp_uint32_t	uiStart;					/* start offset in directory */
	grp_uint32_t	uiEnd;						/* end offset in directory */
} grp_fs_dir_ent_t;

/************************************************/
/* ucType										*/
/************************************************/
#define GRP_FS_FILE_FILE		0x01			/* regular file */
#define GRP_FS_FILE_DIR			0x02			/* directory */
#define GRP_FS_FILE_LINK		0x03			/* link */
#define GRP_FS_FILE_OTHER		0x04			/* others */

/************************************************/
/* uiProtect									*/
/************************************************/
#define GRP_FS_PROT_RUSR		0400			/* owner readable */
#define GRP_FS_PROT_WUSR		0200			/* owner writalble */
#define GRP_FS_PROT_XUSR		0100			/* owner executable */ 
#define	GRP_FS_PROT_RWXU		0700			/* permit all to owner */
#define GRP_FS_PROT_RGRP		0040			/* group readable */
#define GRP_FS_PROT_WGRP		0020			/* group writable */
#define GRP_FS_PROT_XGRP		0010			/* group executable */
#define	GRP_FS_PROT_RWXG		0070			/* permit all to group */
#define GRP_FS_PROT_ROTH		0004			/* others readable */
#define GRP_FS_PROT_WOTH		0002			/* others writable */
#define GRP_FS_PROT_XOTH		0001			/* others executable */
#define	GRP_FS_PROT_RWXO		0007			/* permit all to others */
#define GRP_FS_PROT_RALL		0444			/* readable to all */
#define GRP_FS_PROT_WALL		0222			/* writeable to all */
#define GRP_FS_PROT_XALL		0111			/* executable to all */
#define GRP_FS_PROT_RWXA		0777			/* permit all */

/****************************************************************************/
/*  mount FS information 													*/
/*	(used by grp_fs_get_mnt, grp_fs_get_mnt_by_dev, grp_fs_get_mnt_by_name)	*/
/****************************************************************************/
typedef struct grp_fs_mnt_info {
	int				iDev;							/* device number */
	int				iParentDev;						/* parent device number */
	grp_uint32_t	uiStatus;						/* status */
	char			acDevName[GRP_FS_DEV_NAME_LEN];	/* device name */
	grp_uchar_t		aucPath[GRP_FS_MOUNT_COMP];		/* mount path component */
	char			acFsType[GRP_FS_TYPE_LEN];		/* FS type name */
	grp_ushort_t	usFsSubType;					/* sub file system type */
	grp_ushort_t	usVolNameLen;					/* volume name length */
	grp_uchar_t		aucVolName[GRP_FS_VOL_NAME_LEN]; /* volume name */
	grp_uint32_t	uiVolSerNo;						/* serial number */
	grp_uint32_t	uiFsBlkSize;					/* FS block size */
	grp_uint32_t	uiFsBlkCnt;						/* total block count */
	grp_uint32_t	uiFsFileCnt;					/* total file count */
	grp_uint32_t	uiFsFreeBlk;					/* free block count */
	grp_uint32_t	uiFsFreeFile;					/* free file count */
	grp_uint32_t	uiFBufSize;						/* file buffer size */
	grp_uint32_t	uiDBufSize;						/* data buffer size */
	grp_uint32_t	uiClusterSize;					/* file cluster size */
	grp_uint32_t	uiFBufOff;						/* file buffer offset */
	grp_uint32_t	uiDBufOff;						/* data buffer offset */
	grp_uint32_t	uiDevOff;						/* device offset */
} grp_fs_mnt_info_t;

/************************************************/
/* uiStatus										*/
/************************************************/
#define GRP_FS_MSTAT_RONLY			0x0001			/* read only */
#define GRP_FS_MSTAT_DAY_ACCTIME	0x0002			/* day based access time */
#define GRP_FS_MSTAT_NO_UPD_ACCTIME	0x0004			/* no upd media acc time */
#define GRP_FS_MSTAT_NO_MNT_FLAG	0x0008			/* no mount flag on media */
#define GRP_FS_MSTAT_NO_CRT_ACCTIME	0x1000			/* no media creat/acc time */
#define GRP_FS_MSTAT_DEV_INV		0x0400			/* invalid device */
#define GRP_FS_MSTAT_SYNC_ALL		0x0010			/* sync write always */
#define GRP_FS_MSTAT_SYNC_FL_CLOSE	0x0020			/* sync on each close */
#define GRP_FS_MSTAT_SYNC_FS_CLOSE	0x0040			/* sync on last close */

/****************************************************************************/
/*  error data information (used by grp_fs_get_error)						*/
/****************************************************************************/
typedef struct grp_fs_err_binfo {
	int				iDev;							/* device number */
	grp_uint32_t	uiBlk;							/* buffer block number */
	grp_uchar_t		ucBufType;						/* buffer type */
	grp_uchar_t		ucBlkShift;						/* block size shift */
	grp_uint32_t	uiBlkOff;						/* block offset */
	grp_uint32_t	uiSize;							/* data size */
} grp_fs_err_binfo_t;

/************************************************/
/* iType										*/
/************************************************/
#define GRP_FS_EI_BUF_FILE		1					/* file buffer */
#define GRP_FS_EI_BUF_DATA		2					/* data buffer */

/****************************************************************************/
/*  exported interfaces														*/
/****************************************************************************/
/************************************************/
/* mount a file system							*/
/************************************************/
int grp_fs_mount(								/* mount a file system */
		const char		*pcDevName,				/* [IN]  device name */
		const grp_uchar_t *pucMP,				/* [IN]  mount point */
		const char		*pcFsType,				/* [IN]  file system type */
		int				iMode);					/* [IN]  mount mode */

/* iMode */
#define GRP_FS_RONLY				0x0001		/* read only */
#define GRP_FS_FORCE_MOUNT			0x0002		/* force mount */
#define GRP_FS_NO_UPD_ACCTIME		0x0004		/* no update media acc time */
#define GRP_FS_NO_MNT_FLAG			0x0008		/* no mount flag on media */
#define GRP_FS_NO_CRT_ACCTIME		0x1000		/* no media creat/access time */
#define GRP_FS_SYNC_ALL				0x0010		/* sync write always */
#define GRP_FS_SYNC_FL_CLOSE		0x0020		/* sync write on each close */
#define GRP_FS_SYNC_FS_CLOSE		0x0040		/* sync write on last close */

/************************************************/
/* unmount a file system						*/
/************************************************/
int grp_fs_unmount(								/* unmount a file system */
		const char		*pcDevName,				/* [IN]  device name */
		int				iMode);					/* [IN]  mount mode */

/* iMode */
#define GRP_FS_FORCE_UMOUNT			0x0001		/* force umount */

/************************************************/
/* open file  (file handle is returned)			*/
/************************************************/
int grp_fs_open(								/* open file */
		const grp_uchar_t *pucPath,				/* [IN]  file name to open */
		int				iMode,					/* [IN]  open mode */
		grp_uint32_t	uiProtect);				/* [IN]  create protection */

/* iMode */
#define GRP_FS_O_RDONLY			0x0000			/* read only */
#define GRP_FS_O_WRONLY			0x0001			/* write only */
#define GRP_FS_O_RDWR			0x0002			/* read write */
#define GRP_FS_O_ACCMODE		0x0003			/* access mode mask */
#define GRP_FS_O_APPEND			0x0008			/* append mode */
#define GRP_FS_O_CREAT			0x0200			/* create if not exist */
#define GRP_FS_O_TRUNC			0x0400			/* truncate file */
#define GRP_FS_O_EXCL			0x0800			/* exclusively create */
#define GRP_FS_O_DIRECT_IO		0x8000			/* direct I/O */

/* uiProtect - use uiProect value for grp_fs_dir_ent_t */

/************************************************/
/* close file									*/
/************************************************/
int grp_fs_close(								/* close file */
		int				iFhdl);					/* [IN]  file handle to close */

/************************************************/
/* read file ( read size is returned )			*/
/************************************************/
grp_isize_t grp_fs_read(						/* read file */
		int				iFhdl,					/* [IN]  file handle */
		grp_uchar_t		*pucBuf,				/* [OUT]  buffer to fill data */
		grp_isize_t		iSize);					/* [IN]  size to read */

/************************************************/
/* write file ( written size is returned )		*/
/************************************************/
grp_isize_t grp_fs_write(						/* write file */
		int				iFhdl,					/* [IN]  file handle */
		grp_uchar_t		*pucBuf,				/* [IN]  data to write */
		grp_isize_t		iSize);					/* [IN]  size to write */

/************************************************/
/* set file I/O position						*/
/************************************************/
grp_ioffset_t grp_fs_lseek(						/* set file I/O position */
		int				iFhdl,					/* [IN]  file handle */
		grp_ioffset_t	iOffset,				/* [IN]  offset */
		int				iMode);					/* [IN]  seek mode */

#ifdef GRP_FS_ENABLE_OVER_2G
int grp_fs_lseek4G(
		int				iFhdl,					/* [IN]  file handle number */
		grp_uioffset_t	uiOffset,				/* [IN]  offset to seek */
		int				iMode,					/* [IN]  seek mode */
		grp_uioffset_t	*puiResultOffset );		/* [OUT]  current seek position*/
#endif /* GRP_FS_ENABLE_OVER_2G */

/* iMode */
#define	GRP_FS_SEEK_SET	0						/* absolute offset */
#define GRP_FS_SEEK_CUR	1						/* relative to current */
#define GRP_FS_SEEK_END	2						/* relative to end */

#ifdef GRP_FS_ENABLE_OVER_2G
/*  only lseek4G  */
#define GRP_FS_SEEK_MINUS	0x0100				/* move seek position back */
#endif /* GRP_FS_ENABLE_OVER_2G */

/************************************************/
/* create file									*/
/************************************************/
int grp_fs_create(								/* create */
		const grp_uchar_t *pucPath,				/* [IN]  file name to create */
		grp_uint32_t	uiType,					/* [IN]  file type */
		grp_uint32_t	uiProtect,				/* [IN]  file protection */
		grp_uint32_t	uiAttr);				/* [IN]  FS dependent attr */

/* uiType    - use ucType value for grp_fs_dir_ent_t */
/* uiProtect - use uiProect value for grp_fs_dir_ent_t */
/* uiAttr    - file system dependent value */

/************************************************/
/* unlink file									*/
/************************************************/
int grp_fs_unlink(								/* unlink file */
		const grp_uchar_t *pucPath);			/* [IN]  file name to unlink */

/************************************************/
/* rename file									*/
/************************************************/
int grp_fs_rename(								/* rename file */
		const grp_uchar_t *pucOld,				/* [IN]  old file name */
		const grp_uchar_t *pucNew);				/* [IN]  new file name */

/************************************************/
/* get file attribute (no name is returned) 	*/
/************************************************/
int grp_fs_get_attr(							/* get file attribute */
		const grp_uchar_t *pucPath,				/* [IN]  file name */
		grp_fs_dir_ent_t *ptAttr);				/* [OUT] attribute info */

/************************************************/
/* set file attribute 							*/
/************************************************/
int grp_fs_set_attr(							/* set file attribute */
		const grp_uchar_t *pucPath,				/* [IN]  file name */
		grp_fs_dir_ent_t *ptAttr);				/* [IN] attribute info */

/************************************************/
/* truncate file								*/
/************************************************/
int grp_fs_truncate(							/* truncate a file */
		int				iFhdl,					/* [IN]  file handle */
		grp_uint32_t	uiOffset);				/* [IN]  truncate length */

/************************************************/
/* get directory entry information				*/
/************************************************/
int grp_fs_get_dirent(							/* get direcotry entry info */
		int				iFhdl,					/* [IN]  file handle */
		grp_fs_dir_ent_t *ptDirEnt);			/* [IN/OUT] directory entry */

/************************************************/
/* change directory								*/
/************************************************/
int grp_fs_chdir(								/* change directory */
		const grp_uchar_t *pucPath);			/* [IN]  directory name */

/************************************************/
/* get current working directory				*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_get_cwd(								/* get current working dir */
		grp_uchar_t			*pucPath,			/* [OUT] path name */
		int					iPathBufLen,		/* [IN]  max path buf length */
		int					iSepChar);			/* [IN]  separator char */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/************************************************/
/* update all modifications to devices			*/
/************************************************/
int grp_fs_sync(								/* write back modification */
		int				iMode);					/* [IN]  wait mode */

/* iMode */
#define GRP_FS_SYNC_FAILED	0x0001				/* try to sync write failed */
#define GRP_FS_SYNC_HINT	0x0002				/* sync FS hint information */

#ifdef  GRP_FS_MULTI_LANGUAGE
/************************************************/
/* multi language interfaces					*/
/************************************************/
#ifndef  GRP_FS_FNAME_CACHE
#ifndef  GRP_FS_FAT_CACHE_BY_GET_DIRENT
#include "grp_fs_multi_language_if.h"
#endif  /* GRP_FS_FNAME_CACHE */
#endif  /* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
#endif  /* GRP_FS_MULTI_LANGUAGE */

/************************************************/
/* get mounted file system information			*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_get_mnt(								/* get all file system info */
		int					iMaxCnt,			/* [IN]  max info count */
		grp_fs_mnt_info_t	*ptMntInfo);		/* [OUT] file system info */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_get_mnt_by_dev(						/* get FS info by dev number */
		int					iDev,				/* [IN]  */
		grp_fs_mnt_info_t	*ptMntInfo);		/* [OUT] file system info */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

int grp_fs_get_mnt_by_name(						/* get FS info by dev name */
		const char			*pcDevName,			/* [IN]  device name */
		grp_fs_mnt_info_t	*ptMntInfo);		/* [OUT] file system info */

/************************************************/
/* invalidate FS device status					*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_invalidate_fs_dev(					/* invalidate FS dev status */
		const char			*pcDevName);		/* [IN]  device name */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/************************************************/
/* check FS device status						*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_check_fs_dev(						/* check FS device status */
		const char			*pcDevName,			/* [IN]  device name */
		grp_uchar_t			*pucVolName,		/* [OUT] volume name length */
		int					*piVolNameLen,		/* [IN/OUT] volume name len */
		grp_uint32_t		*puiVolSerNo);		/* [OUT] volume serial number */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/************************************************/
/* convert device name to device number			*/
/************************************************/
int grp_fs_lookup_dev(							/* lookup device */
		const char		*pcDev);				/* [IN] device name */

/************************************************/
/* check volume name							*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_check_volume(						/* check volume name */
		const char			*pcDevName,			/* [IN]  device name */
		const char			*pcFsType,			/* [IN]  file system type */
		grp_uchar_t			*pucVolName,		/* [OUT] volume name length */
		int					*piVolNameLen,		/* [IN/OUT] volume name len */
		grp_uint32_t		*puiVolSerNo);		/* [OUT] volume serial number */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/************************************************/
/* get write error data							*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
grp_int32_t grp_fs_get_error(					/* get write error data */
		int					iMode,				/* [IN]  operation mode */
		int					iDev,				/* [IN]  target device */
		grp_uint32_t		uiBlk,				/* [IN]  target block number */
		grp_uchar_t			*pucBuf,			/* [OUT] output buffer */
		grp_uint32_t		uiSize,				/* [IN]  buffer size */
		grp_uint32_t		*puiNeed);			/* [OUT] need buffer size */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/* iMode */
#define GRP_FS_GE_CONTENT	0x0001				/* get content */
#define GRP_FS_GE_RELEASE	0x0002				/* release buffer */
#define GRP_FS_GE_DIRTY		0x0004				/* get dirty buffer */
#define GRP_FS_GE_FBUF		0x0010				/* get error data in file buf */
#define GRP_FS_GE_DBUF		0x0020				/* get error data in data buf */ 
/* iDev */
#define GRP_FS_GE_DEV_ANY		-1					/* any device */

/* uiBlk */
#define GRP_FS_GE_BLK_ANY		0xffffffff			/* any block */

/************************************************/
/* free my task environment for file access		*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_task_free_env(void);					/* free my environment */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/************************************************/
/* free my task environment for file access		*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_task_free_env_by_id(					/* get task environment by id */
		grp_fs_task_t	tTaskId);				/* free task environment */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/************************************************/
/* free all task environment for file access	*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_task_free_all_env(void);				/* free all environment */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/************************************************/
/* initialize/reinitailize file system 			*/
/* management									*/
/************************************************/
int grp_fs_init(void);							/* init/reinit management */

/************************************************/
/* convert error number to error message		*/
/************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
char *grp_fs_err(								/* convert error no to msg */
		int				iErrNo,					/* [IN]  error number */
		char			*pcMsgBuf);				/* [OUT] message buffer */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
#endif	/* _GRP_FS_IF_H_ */
