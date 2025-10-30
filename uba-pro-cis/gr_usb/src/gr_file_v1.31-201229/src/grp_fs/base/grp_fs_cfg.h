#ifndef	_GRP_FS_CFG_H_
#define	_GRP_FS_CFG_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_cfg.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for file system configuration							*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_fs_if.h															*/
/*		grp_fs_dev_io.h														*/
/*		grp_fs_multi_language_cfg.h											*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/06/14	Added definitions for file name cache	*/
/*		T.Imashiki		2004/07/25	Added device ioctl function				*/
/*									Moved device number macros to grp_fs	*/
/*									_dev_io_if.h							*/
/*									Changed default value for caching		*/
/*									parameters GRP_FS_FBLK_SHIFT, GRP_FS_	*/
/*									DBLK_SHIFT, GRP_FS_DBLK_CNT				*/
/*									Deleted GRP_FS_MAX_DEV and added grp_fs	*/
/*									_dev_tbl_cnt instead					*/
/*		T.Imashiki		2005/02/10	Changed return type of grp_fs_read_t	*/
/*									and grp_fs_write_t for 16 bit CPU		*/
/*		T.Imashiki		2006/10/31	Added FS dependent sync function to		*/
/*									grp_fs_op								*/
/*		M.Toyama		2008/03/05	Added FS dependent multi language		*/
/*									support function.						*/
/*		K.Kaneko		2008/05/12	Moved file system management parameter	*/
/*									define to grp_fs_param.h				*/
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
#include "grp_fs_if.h"
#include "grp_fs_dev_io_if.h"

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
/*  file system management parameter table									*/
/****************************************************************************/
typedef struct grp_fs_param {
	short				sMounts;				/* mount table count */
	short				sTasks;					/* task control table count */
	grp_uchar_t			ucFBlkShift;			/* file block shift count */
	grp_uchar_t			ucDBlkShift;			/* data block shift count */
	grp_uint32_t		uiFBlkCnt;				/* file block count */
	grp_uint32_t		uiDBlkCnt;				/* data block count */
	grp_uint32_t		uiBHashCnt;				/* buffer hash count */
	grp_uint32_t		uiFileCnt;				/* file count */
	grp_uint32_t		uiFHashCnt;				/* file hash count */
	grp_uint32_t		uiFhdlCnt;				/* file handle count */
#ifdef	GRP_FS_FNAME_CACHE
	grp_uint32_t		uiFnameCacheCnt;		/* file name cache count */
	grp_uint32_t		uiFnameHashCnt;			/* file name hash count */
#endif	/* GRP_FS_FNAME_CACHE */
	grp_uint32_t		uiFBlkSize;				/* file block size (computed) */
	grp_uint32_t		uiDBlkSize;				/* data block size (computed) */
} grp_fs_param_t;

/****************************************************************************/
/*  device table structure													*/
/****************************************************************************/
/************************************************/
/* typedefs for device operation function		*/
/************************************************/
typedef int grp_fs_dev_open_t(					/* device open function */
					int				iDev,		/* [IN]  device number */
					int				iRWOpen,	/* [IN]  read/write open */
					grp_int32_t		*piHandle,	/* [OUT] I/O handle */
					grp_uint32_t	*puiOff,	/* [OUT] start offset */
					grp_uint32_t	*puiSize,	/* [OUT] size of device */
					int				*piSzShift);/* [OUT] shift of size */
typedef int grp_fs_dev_close_t(					/* device close function */
					grp_int32_t		iHandle,	/* [IN]  I/O handle */
					int				iDev);		/* [IN]  device number */
typedef grp_int32_t grp_fs_dev_read_t(			/* device read function */
					grp_int32_t		iHandle,	/* [IN]  I/O handle */
					int				iDev,		/* [IN]  device number */
					grp_uint32_t	uiDevBlk,	/* [IN]  device block number */
					grp_uchar_t		*pucBuf,	/* [OUT] I/O buffer */
					grp_isize_t		iCnt);		/* [IN]  I/O count */
typedef grp_int32_t grp_fs_dev_write_t(			/* device write function */
					grp_int32_t		iHandle,	/* [IN]  I/O handle */
					int				iDev,		/* [IN]  device number */
					grp_uint32_t	uiDevBlk,	/* [IN]  device block number */
					grp_uchar_t		*pucBuf,	/* [IN]  I/O buffer */
					grp_isize_t		iCnt);		/* [IN]  I/O count */
typedef int grp_fs_dev_ioctl_t(					/* device I/O control */
					int				iDev,		/* [IN]  device number */
					grp_uint32_t	uiCmd,		/* [IN]  command number */
					void			*pvParam);	/* [IN/OUT] parameter */

/************************************************/
/* type definition for device operation			*/
/************************************************/
typedef struct grp_fs_dev_op {					/* device operation info */
	grp_fs_dev_open_t		*pfnOpen;			/* open function */
	grp_fs_dev_close_t		*pfnClose;			/* close function */
	grp_fs_dev_read_t		*pfnRead;			/* read function */
	grp_fs_dev_write_t		*pfnWrite;			/* write function */
	grp_fs_dev_ioctl_t		*pfnIoctl;			/* device I/O control */
} grp_fs_dev_op_t;

/************************************************/
/* type definition for device switch			*/
/************************************************/
typedef struct grp_fs_dev_tbl {					/* device table */
	const char				*pcDevName;			/* device name */
	grp_fs_dev_op_t			*ptOp;				/* device operation */
} grp_fs_dev_tbl_t;

/****************************************************************************/
/*  file system type table structure										*/
/****************************************************************************/
typedef struct grp_fs_info	grp_fs_info_t;		/* file system information */
typedef struct grp_fs_file	grp_fs_file_t;		/* open file information */

/************************************************/
/* typedefs for FS operation function			*/
/************************************************/
typedef int grp_fs_open_root_t(					/* return root */
					grp_fs_info_t	*ptFs,		/* [IN]  file system info */
					grp_fs_file_t	**pptFile);	/* [OUT] root file */
typedef int grp_fs_mount_t(						/* mount file system */
					grp_fs_info_t	*ptFs,		/* [IN/OUT] file system info */
					int				iMode);		/* [IN]  mount mode */
typedef int grp_fs_umount_t(					/* unmont file system */
					grp_fs_info_t	*ptFs,		/* [IN/OUT] file system info */
					int				iMode);		/* [IN]  mount mode */
typedef int grp_fs_open_t(					/* open file */
					grp_fs_info_t	*ptFs,		/* [IN]  file system info */
					grp_fs_file_t	*ptDir,		/* [IN]  base directory */
					const grp_uchar_t **ppucPath,/* [IN/OUT] path name */
					int				iMode,		/* [IN]  open mode */
					grp_uchar_t		*pucComp,	/* [IN]  component buffer */
					grp_fs_file_t	**pptFile);	/* [IN/OUT] file information */
typedef int grp_fs_close_t(						/* close file */
					grp_fs_file_t	*ptFile,	/* [IN]  file information */
					int				iMode);		/* [IN]  close mode */
typedef grp_int32_t grp_fs_read_t(				/* read file */
					grp_fs_file_t	*ptFile,	/* [IN]  file information */
					grp_uint32_t	uiFsBlk,	/* [IN]  FS block number */
					grp_uint32_t	uiBlkOff,	/* [IN]  block offset */
					grp_uchar_t		*pucBuf,	/* [OUT] I/O buffer */
					grp_isize_t		iSize,		/* [IN]  I/O count */
					int				iMode);		/* [IN]  I/O mode */
typedef grp_int32_t grp_fs_write_t(				/* write file */
					grp_fs_file_t	*ptFile,	/* [IN]  file information */
					grp_uint32_t	uiFsBlk,	/* [IN]  FS block number */
					grp_uint32_t	uiBlkOff,	/* [IN]  block offset */
					grp_uchar_t		*pucBuf,	/* [IN]  I/O buffer */
					grp_isize_t		iSize,		/* [IN]  I/O count */
					int				iMode);		/* [IN]  I/O mode */
typedef int grp_fs_create_t(					/* create file */
					grp_fs_info_t	*ptFs,		/* [IN]  file system info */
					grp_fs_file_t	*ptDir,		/* [IN]  base directory */
					const grp_uchar_t **ppucPath,/* [IN/OUT] file name */
					grp_uint32_t	uiType,		/* [IN]  type of file */
					grp_uint32_t	uiProtect,	/* [IN]  protection */
					grp_uint32_t	uiAttr,		/* [IN]  FS dependent attr */
					grp_uchar_t		*pucComp,	/* [IN/OUT]  component buffer */
					grp_fs_file_t	**pptFile);	/* [OUT] file information */
typedef int grp_fs_unlink_t(					/* unlink file */
					grp_fs_info_t	*ptFs,		/* [IN]  file system info */
					grp_fs_file_t	*ptDir,		/* [IN]  base directory */
					const grp_uchar_t **ppucPath,/* [IN] path name */
					grp_uchar_t		*pucComp,	/* [IN/OUT]  component buffer */
					grp_fs_file_t	**pptFile);	/* [OUT] cross FS information */
typedef int grp_fs_rename_t(					/* rename file */
					grp_fs_info_t	*ptFs,		/* [IN]  file system info */
					grp_fs_file_t	*ptOldDir,	/* [IN]  base dir for org */
					grp_uchar_t		*pucOld,	/* [IN]  old file name */
					grp_fs_file_t	*ptNewDir,	/* [IN]  base dir for new */
					grp_uchar_t		*pucNew);	/* [IN]  new file name */
typedef int grp_fs_get_attr_t(					/* get file attribute */
					grp_fs_file_t	*ptFile,	/* [IN]  file information */
					grp_fs_dir_ent_t *ptDirEnt); /* [OUT] attr info */
typedef int grp_fs_set_attr_t(					/* set file attribute */
					grp_fs_file_t	*ptFile,	/* [IN]  file information */
					grp_fs_dir_ent_t *ptDirEnt); /* [IN] attr info */
typedef int grp_fs_truncate_t(					/* truncate file */
					grp_fs_file_t	*ptFile,	/* [IN]  file information */
					grp_uint32_t	uiFsBlk,	/* [IN]  FS block number */
					grp_uint32_t	uiBlkOff);	/* [IN]  block offset */
typedef int grp_fs_get_dirent_t(				/* get directory entry */
					grp_fs_file_t	*ptDir,		/* [IN]  directory info */
					grp_fs_dir_ent_t *ptDirEnt); /* [IN/OUT] directory entry */
typedef int grp_fs_match_comp_t(				/* match component */
					grp_fs_file_t	*ptDir,		/* [IN]  directory info */
					const grp_uchar_t *pucComp,	/* [IN]  component name */
					int				iPurge,		/* [IN]  purge fname cache */
					grp_fs_file_t	**ptFile,	/* [OUT] file opened */
					int				iNeed,		/* [IN]  need dirent size */
					grp_uint32_t	*puiOff);	/* [OUT] found offset */
typedef int grp_fs_check_volume_t(				/* check volume */
					int				iDev,		/* [IN]  device number */
					grp_fs_info_t	*ptFs,		/* [IN]  file system info */
					grp_uchar_t		*pucVolName,/* [OUT] volume name */
					grp_uint32_t	*puiVolSerNo);/* [OUT] serial number */
typedef int grp_fs_sync_t(						/* FS dependent sync */
					grp_fs_info_t	*ptFs,		/* [IN]  file system info */
					int				iMode);		/* [IN]  sync mode */

#ifdef GRP_FS_MULTI_LANGUAGE
#include "grp_fs_multi_language_cfg.h"
#endif  /* GRP_FS_MULTI_LANGUAGE */

/************************************************/
/* special unmount iMode flag for revoke		*/
/************************************************/
#define	GRP_FS_REVOKE_MOUNT			0x8000		/* revoke mount */

/************************************************/
/* close mode (iMode)							*/
/************************************************/
#define GRP_FS_CLOSE_RELEASE		0x0000		/* compretely close */
#define GRP_FS_CLOSE_CACHE			0x0001		/* leave cache */
#define GRP_FS_CLOSE_UPDATE			0x0002		/* only update attribute */

/************************************************/
/* type definition for FS operation				*/
/************************************************/
typedef struct grp_fs_op {						/* FS operation info */
	grp_fs_open_root_t	*pfnOpenRoot;			/* return root */
	grp_fs_mount_t		*pfnMount;				/* mount function */
	grp_fs_umount_t		*pfnUmount;				/* unmount function */
	grp_fs_open_t		*pfnOpen;				/* open function */
	grp_fs_close_t		*pfnClose;				/* close function */
	grp_fs_read_t		*pfnRead;				/* read function */
	grp_fs_write_t		*pfnWrite;				/* write function */
	grp_fs_create_t		*pfnCreate;				/* create function */
	grp_fs_unlink_t		*pfnUnlink;				/* unlink function */
	grp_fs_rename_t		*pfnRename;				/* rename function */
	grp_fs_get_attr_t	*pfnGetAttr;			/* get attribute function */
	grp_fs_set_attr_t	*pfnSetAttr;			/* set attribute function */
	grp_fs_truncate_t	*pfnTruncate;			/* truncate function */
	grp_fs_get_dirent_t	*pfnGetDirEnt;			/* get directory entry */
	grp_fs_match_comp_t	*pfnMatchComp;			/* match component */
	grp_fs_check_volume_t *pfnCheckVolume;		/* check volume */
	grp_fs_sync_t		*pfnSync;				/* FS dependent sync */
#ifdef GRP_FS_MULTI_LANGUAGE
	GRP_FS_MULTI_LANG_FUNCTIONS					/* multi language function list */
#endif  /* GRP_FS_MULTI_LANGUAGE */
} grp_fs_op_t;

/************************************************/
/* type definition for file system switch		*/
/************************************************/
typedef struct grp_fs_type_tbl {				/* file system type table */
	const char			*pcFsName;				/* file system type name */
	grp_fs_op_t			*ptFsOp;				/* operation information */
} grp_fs_type_tbl_t;

extern	int		grp_fs_not_supported(void);		/* not supported function */

/****************************************************************************/
/*  configuration tables													*/
/****************************************************************************/
extern	grp_fs_param_t		grp_fs_param;		/* file system parameter */
extern	grp_fs_dev_tbl_t	grp_fs_dev_tbl[];	/* device switch table */
extern	grp_fs_type_tbl_t	grp_fs_type_tbl[];	/* file system switch table */
extern	int					grp_fs_dev_tbl_cnt;	/* device table count */

#endif	/* _GRP_FS_CFG_H_ */
