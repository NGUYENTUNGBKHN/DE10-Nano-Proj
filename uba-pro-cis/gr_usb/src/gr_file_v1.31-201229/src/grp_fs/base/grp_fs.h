#ifndef	_GRP_FS_H_
#define	_GRP_FS_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for file system management								*/
/* FUNCTIONS:																*/
/*		grp_fs_lookup_dev			lookup device							*/
/*		grp_fs_lookup_buf			lookup buffer							*/
/*		grp_fs_unref_buf			unreference buffer						*/
/*		grp_fs_buf_fill_end			filling buffer end						*/
/*		grp_fs_block_buf_mod		block buffer modify						*/
/*		grp_fs_unblock_buf_mod		unblock buffer modify					*/
/*		grp_fs_wait_io				wait buffer I/O							*/
/*		grp_fs_block_fs_mod			block file system modify				*/
/*		grp_fs_unblock_fs_mod		unblock file system modify				*/
/*		grp_fs_block_file_op		block file operation					*/
/*		grp_fs_unblock_file_op		unblock file operation					*/
/*		grp_fs_block_file_op_by_id	block file operation by id				*/
/*		grp_fs_unblock_file_op_by_id unblock file operation by id			*/
/*		grp_fs_close_file			close file								*/
/*		grp_fs_lookup_file_ctl		lookup file information					*/
/*		grp_fs_change_fid			change file id							*/
/*		grp_fs_check_io_status		check I/O status						*/
/*		grp_fs_read_buf				read data in buffer						*/
/*		grp_fs_write_buf			write data in buffer					*/
/*		grp_fs_exec_dev_io			execute device I/O						*/
/*		grp_fs_get_path_comp		get a path component					*/
/*		grp_fs_set_access_time		set access time							*/
/*		grp_fs_file_open_common		common file open function				*/
/*		grp_fs_get_mount_root_attr	get mounted root attribute				*/
/*		grp_fs_check_mnt_dev		check mounted device					*/
/*		grp_fs_check_dev_busy		check device busy						*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_queue.h															*/
/*		grp_fs_mdep_if.h													*/
/*		grp_fs_if.h															*/
/*		grp_fs_cfg.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/06/14	Added file name cache function			*/
/*		T.Imashiki		2004/07/25	Changed grp_fs_lookup_dev to support	*/
/*									sub-device id and partition				*/
/*									Added grp_fs_check_dev_busy				*/
/*									Added GRP_FS_STAT_NO_UPD_ACCTIME		*/
/*									Added GRP_FS_STAT_NO_MNT_FLAG			*/
/*									Added GRP_FS_STAT_NO_CRT_ACCTIME		*/
/*		T.Imashiki		2004/12/07	Added buffer kind parameter to grp_fs_	*/
/*									exec_dev_io								*/
/*		T.Imashiki		2004/12/24	Added grp_fs_change_fid, grp_fs_block/	*/
/*									unblock_file_op_by_id, and GRP_FS_STAT	*/
/*									BUSY_FID/GRP_FS_STAT_WAIT_BUSY_FID to	*/
/*									fix bug of not complying treatment of	*/
/*									size 0 file								*/
/*		T.Imashiki		2005/02/10 	Changed return types of grp_fs_read_buf	*/
/*									and grp_fs_write_buf for 16 bit CPU		*/
/*		T.Imashiki		2005/11/25	Added grp_fs_purge_fname_cache for		*/
/*									simple purge case						*/
/*									Added grp_fs_block_file_write and		*/
/*									grp_fs_unblock_file_write, and			*/
/*									GRP_FS_FSTAT_WR_LOCK and GRP_FS_FSTAT_	*/
/*									WR_WAIT bits							*/
/*		T.Imashiki		2006/01/31	Exported grp_fs_purge_fname_cache_entry	*/
/*		T.Imashiki		2007/09/21	Added iFsWaitCnt to grp_fs_ctl			*/
/*		T.Imashiki		2011/05/23	Added ptFs parameter to grp_fs_task_	*/
/*		K.Kaneko					wait to	allow unmount not waited FS		*/
/*									Deleted iFsWaitCnt of grp_fs_ctl by		*/
/*									changing to update ptFs->iFsRef to		*/
/*									allow unmount not waited FS 			*/
/*									Added GRP_FS_OPEN_UNDER_CLOSE/			*/
/*									GRP_FS_OPEN_WAIT_CLOSE for under close	*/
/*									check to allow unmount not waited FS	*/
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
#include "grp_types.h"
#include "grp_queue.h"
#include "grp_fs_mdep_if.h"
#include "grp_fs_if.h"
#include "grp_fs_cfg.h"

/****************************************************************************/
/*  file system information													*/
/****************************************************************************/
struct grp_fs_info {							/* file system information */
	int				iDev;						/* device number */
	grp_ushort_t	usStatus;					/* status */
	short			sPathLen;					/* path length */
	grp_int32_t		iFsRef;						/* reference count */
	grp_int32_t		iFsOpen;					/* open count */
	grp_fs_op_t		*ptFsOp;					/* function table */
	grp_fs_type_tbl_t *ptFsTbl;					/* file system type table */
	grp_fs_info_t	*ptFsNest;					/* nested mount point list */
	grp_que_fld(grp_fs_info_t *, ptFsOther);	/* other mount point list */
	grp_uchar_t		aucPath[GRP_FS_MOUNT_COMP];	/* mount path component */
	grp_fs_file_t	*ptFsParent;				/* parent file */
	grp_int32_t		iDevHandle;					/* device I/O handle */
	grp_uint32_t	uiDevSize;					/* device size (blocks) */
	grp_uint32_t	uiDevOff;					/* device start block offset */
	grp_uchar_t		ucDevBlkShift;				/* device size shift */
	/* following fields are set by file system dependent routines */
	grp_uchar_t		ucFsFBlkShift;				/* file block shift count */
	grp_uchar_t		ucFsDBlkShift;				/* data block shift count */
	grp_uchar_t		ucFsCBlkShift;				/* data cluster shift count */
	grp_uint32_t	uiFsFBlkOff;				/* file block offset */
	grp_uint32_t	uiFsDBlkOff;				/* data block offset */
	grp_uint32_t	uiFsFreeBlk;				/* free block count */
	grp_uint32_t	uiFsFreeFile;				/* free file count */
	grp_uint32_t	uiFsBlkCnt;					/* total block count */
	grp_uint32_t	uiFsFileCnt;				/* total file count */
	grp_uint32_t	uiVolSerNo;					/* volume serial number */
	grp_uchar_t		aucVolName[GRP_FS_VOL_NAME_LEN];/* volume name */
	grp_ushort_t	usVolNameLen;				/* volume name length */
	grp_ushort_t	usFsSubType;				/* sub file system type */
	void			*pvFsInfo;					/* FS dependent information */
	grp_uint32_t	uiFsBusyFid;				/* busy file id */
};

/************************************************/
/* usStatus										*/
/************************************************/
#define GRP_FS_STAT_RONLY			0x0001		/* read only */
#define GRP_FS_STAT_DAY_ACCTIME		0x0002		/* day based access time */
#define GRP_FS_STAT_NO_UPD_ACCTIME	0x0004		/* no update media acc time */
#define GRP_FS_STAT_NO_MNT_FLAG		0x0008		/* no mount flag on media */
#define GRP_FS_STAT_NO_CRT_ACCTIME	0x1000		/* no media creat/access time */
#define GRP_FS_STAT_SYNC_ALL		0x0010		/* sync write always */
#define GRP_FS_STAT_SYNC_FL_CLOSE	0x0020		/* sync write on each close */
#define GRP_FS_STAT_SYNC_FS_CLOSE	0x0040		/* sync write on last close */
#define GRP_FS_STAT_MOD				0x0100		/* under modification */
#define GRP_FS_STAT_WAITMOD			0x0200		/* wait modification */
#define GRP_FS_STAT_DEV_INV			0x0400		/* invalid device */
#define GRP_FS_STAT_BUSY_FID		0x0800		/* exist busy file id */
#define GRP_FS_STAT_WAIT_BUSY_FID	0x8000		/* wait busy file id */

/****************************************************************************/
/*  open file information													*/
/****************************************************************************/

/************************************************/
/* open file table								*/
/************************************************/
struct grp_fs_file {							/* file information */
	grp_uchar_t		ucType;						/* file type */
	grp_ushort_t	usStatus;					/* status */
	grp_uint32_t	uiProtect;					/* protection */
	int				iRefCnt;					/* reference count */
	int				iDev;						/* device number */
	grp_uint32_t	uiFid;						/* file ID */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_uisize_t	uiSize;						/* file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	grp_isize_t		iSize;						/* file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	grp_int32_t		iCTime;						/* creation time */
	grp_int32_t		iMTime;						/* modify time */
	grp_int32_t		iATime;						/* access time */
	grp_uint32_t	uiAttr;						/* FS dependent attribute */
	grp_uint32_t	uiMapFBlk;					/* mapping start file block */
	grp_uint32_t	uiMapCnt;					/* mapping count */
	grp_uint32_t	*puiMap;					/* file to phisical block map */
	grp_fs_info_t	*ptFs;						/* FS information */
	void			*pvFileInfo;				/* FS depenent file info */
	grp_que_fld(grp_fs_file_t *, ptList);		/* LRU list */
	grp_que_fld(grp_fs_file_t *, ptHash);		/* hash list */
	grp_fs_file_t	**pptHashTop;				/* hash top */
};

/************************************************/
/* usStatus										*/
/************************************************/
#define GRP_FS_FSTAT_BUSY		0x0001			/* under operation */
#define GRP_FS_FSTAT_UPD_ATIME	0x0002			/* update access time */
#define GRP_FS_FSTAT_UPD_MTIME	0x0004			/* update modified time */
#define GRP_FS_FSTAT_UPD_ATTR	0x0008			/* modified attriute */
#define GRP_FS_FSTAT_UPD_BITS	0x000e			/* all update bits */
#define GRP_FS_FSTAT_ROOT		0x0010			/* root file */
#define GRP_FS_FSTAT_MOUNT		0x0020			/* mounted on it */
#define GRP_FS_FSTAT_NO_UPD_TIME 0x0040			/* no time modification */
#define GRP_FS_FSTAT_INVALID	0x0100			/* invalid data */
#define GRP_FS_FSTAT_WR_LOCK	0x0200			/* write lock */
#define GRP_FS_FSTAT_WR_WAIT	0x4000			/* waiting write */
#define GRP_FS_FSTAT_WAIT		0x8000			/* waiting */

/************************************************/
/* file hash macro								*/
/************************************************/
#define GRP_FS_FILE_HASH(ptFsCtl, uiFid)		/* file hash function */	\
	(&ptFsCtl->pptFileHash[(uiFid) & (grp_fs_param.uiFHashCnt - 1)])

/************************************************/
/* iMode parameter for grp_fs_close_file		*/
/************************************************/
#define GRP_FS_FILE_INVALID	0x0001				/* invalidate */
#define GRP_FS_FILE_UNBLOCK	0x0002				/* unblock file op */

/****************************************************************************/
/*  file handles for applications											*/
/****************************************************************************/
typedef struct grp_fs_task_ctl	grp_fs_task_ctl_t;/* task control table */
typedef struct grp_fs_fhdl {					/* file handle */
	short				sHdlId;					/* handle id */
	grp_ushort_t		usMode;					/* mode */
	grp_fs_file_t		*ptFile;				/* pointer to file info */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_uioffset_t		uiOffset;				/* offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
	grp_ioffset_t		iOffset;				/* offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
	grp_fs_task_ctl_t	*ptTask;				/* task environment */
} grp_fs_fhdl_t;

/************************************************/
/* macro to make a free list of file handles	*/
/************************************************/
#define grp_fs_fhdl_list(ptFhdl) (*((grp_fs_fhdl_t **)&(ptFhdl)->ptFile))

#ifdef	GRP_FS_FNAME_CACHE
/****************************************************************************/
/*  file name cache															*/
/****************************************************************************/
typedef struct grp_fs_fname_cache grp_fs_fname_cache_t;
struct grp_fs_fname_cache {						/* file name cache */
	int					iDev;					/* device ID */
	grp_uint32_t		uiDirFid;				/* parent file ID */
	grp_uint32_t		uiFid;					/* file ID */
	short				sNameLen;				/* name length */
	short				sNameBufLen;			/* name buffer length */
	grp_uchar_t			*pucName;				/* file name */
	grp_fs_fname_cache_t *ptAlias;				/* alias entry */
	grp_que_fld(grp_fs_fname_cache_t *, ptList);/* LRU list */
	grp_que_fld(grp_fs_fname_cache_t *, ptHash);/* hash list */
	grp_fs_fname_cache_t **pptHashTop;			/* hash top */
};
#endif	/* GRP_FS_FNAME_CACHE */

/****************************************************************************/
/*  buffer information														*/
/****************************************************************************/
typedef struct grp_fs_buf grp_fs_buf_t;			/* file buffer */
struct grp_fs_buf {								/* file buffer */
	int				iDev;						/* device number */
	int				iRefCnt;					/* reference count */
	grp_ushort_t	usStatus;					/* status */
	grp_fs_info_t	*ptFs;						/* FS information */
	grp_uint32_t	uiBlk;						/* buffer block number */
	grp_int32_t		iSize;						/* size of data in buffer */
	grp_uchar_t		*pucData;					/* data buffer */
	grp_que_fld(grp_fs_buf_t *, ptList);		/* LRU list */
	grp_que_fld(grp_fs_buf_t *, ptHash);		/* hash list */
	grp_fs_buf_t	**pptHashTop;				/* hash top */
};

#define GRP_FS_BSTAT_FBUF		0x0001			/* file buffer */
#define GRP_FS_BSTAT_DBUF		0x0002			/* data buffer */
#define GRP_FS_BSTAT_SYNC		0x0010			/* sync write */
#define GRP_FS_BSTAT_TSYNC		0x0020			/* temporary sync write */
#define GRP_FS_BSTAT_FILL		0x0100			/* under filling data */
#define GRP_FS_BSTAT_MOD		0x0200			/* under modification */
#define GRP_FS_BSTAT_DIRTY		0x0400			/* modified */
#define GRP_FS_BSTAT_WFAIL		0x0800			/* write failed */
#define GRP_FS_BSTAT_WAITMOD	0x4000			/* wait for modification */
#define GRP_FS_BSTAT_WAIT		0x8000			/* wait for buf */

/************************************************/
/* buffer hash macro							*/
/************************************************/
#define GRP_FS_BUF_HASH(ptFsCtl, uiBufBlk)		/* buf hash function */	\
	(&ptFsCtl->pptBufHash[(uiBufBlk) & (grp_fs_param.uiBHashCnt - 1)])

/************************************************/
/* buffer I/O information for grp_fs_read_buf	*/
/************************************************/
typedef struct grp_fs_bio {
	grp_fs_buf_t	*ptBuf;						/* buffer table */
	grp_uchar_t		*pucData;					/* data pointer */
	grp_uint32_t	uiSize;						/* size of data */
	grp_uint32_t	uiBlk;						/* FS block number */
} grp_fs_bio_t;

/************************************************/
/* buffer kind used by grp_fs_read_buf or		*/
/* _grp_fs_lookup_buf							*/
/************************************************/
#define GRP_FS_BUF_FILE			0x0001			/* get file buffer */
#define GRP_FS_BUF_DATA			0x0002			/* get data buffer */
#define GRP_FS_BUF_ALLOC		0x0004			/* allocate buffer(lookup) */

/************************************************/
/* iMode used for _grp_fs_wait_io				*/
/************************************************/
#define GRP_FS_BUF_WAIT_INV		0x0001			/* wait and invalidate */
#define GRP_FS_BUF_FORCE_INV	0x0002			/* force invalidate */

/****************************************************************************/
/*  task mangement table 													*/
/****************************************************************************/
struct grp_fs_task_ctl {						/* task control table */
	grp_fs_task_t	tTaskId;					/* task id */
	grp_fs_sem_t	tTaskSem;					/* semaphore */
	void			*pvWaitRsc;					/* waiting resource pointer */
	int				iOpenCnt;					/* open count */
	grp_fs_file_t	*ptCurDir;					/* current directory */
	grp_que_fld(grp_fs_task_ctl_t *, ptTaskList); /* task list */
};

#define GRP_FS_NO_TASK ((grp_fs_task_t)0xffffffff)		/* no task */
#define GRP_FS_INV_CD ((grp_fs_file_t *)0xffffffff)	/* invalidated dir */

/****************************************************************************/
/*  top level file system management table 									*/
/****************************************************************************/
typedef struct grp_fs_ctl {						/* FS control table */
	grp_fs_sem_t	tFsSem;						/* semaphore */
	int				iFsStat;					/* status */
#ifndef GRP_FS_ASYNC_UNMOUNT
	int				iFsWaitCnt;					/* wait task count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	grp_fs_info_t	*ptFsFree;					/* free FS info list */
	grp_fs_info_t	*ptFsMnt;					/* mount point list */
	grp_fs_info_t	*ptFsDefault;				/* default FS */
	grp_que_fld(grp_fs_file_t *, ptFile);		/* file LRU list */
	grp_que_fld(grp_fs_buf_t *, ptDBuf);		/* data buffer list */
	grp_que_fld(grp_fs_buf_t *, ptFBuf);		/* file buffer list */
	grp_fs_task_ctl_t *ptTask;					/* active task control table */
	grp_fs_task_ctl_t *ptTaskFree;				/* free task control table */
	grp_fs_fhdl_t	*ptFhdlTbl;					/* file handles */
	grp_fs_fhdl_t	*ptFhdlFree;				/* free file handle list */
	grp_fs_buf_t	**pptBufHash;				/* buffer hash */
	grp_fs_file_t	**pptFileHash;				/* file hash */
#ifdef	GRP_FS_FNAME_CACHE
	grp_que_fld(grp_fs_fname_cache_t *, ptFnCache);/* fname cache LRU list */
	grp_fs_fname_cache_t **pptFnHash;			/* file name hash */
#endif	/* GRP_FS_FNAME_CACHE */
	void			*pvTblMem;					/* allocated table memory */
	void			*pvBufMem;					/* allocated buffer memory */
} grp_fs_ctl_t;

/************************************************/
/* iFsStatus									*/
/************************************************/
#define GRP_FS_CSTAT_BUSY	0x0001				/* busy for mount/umount */
#define GRP_FS_CSTAT_FBWAIT	0x0100				/* wait for file buffer */
#define GRP_FS_CSTAT_DBWAIT	0x0200				/* wait for data buffer */
#define GRP_FS_CSTAT_WAIT	0x0400				/* wait for mount/umount */

/****************************************************************************/
/*  constant definitions for exported interface								*/
/****************************************************************************/
/************************************************/
/* return value of grp_fs_get_path_comp and		*/
/* path based FS dependent functions 			*/
/************************************************/
#define GRP_FS_COMP_LAST	1					/* last componet */
#define GRP_FS_COMP_MIDDLE	2					/* middle of component */

/************************************************/
/* iMode paramater of grp_fs_file_open_common	*/
/************************************************/
#define GRP_FS_OPEN_EXEC		0x0001			/* execute mode */
#define GRP_FS_OPEN_WRITE		0x0002			/* write mode */
#define GRP_FS_OPEN_READ		0x0004			/* read mode */
#define GRP_FS_OPEN_APPEND		0x0008			/* append mode */
#define GRP_FS_OPEN_PARENT		0x1000			/* open parent direcotry */
#define GRP_FS_OPEN_DIRECT_IO	0x2000			/* direct I/O */

#ifdef GRP_FS_ASYNC_UNMOUNT
#define GRP_FS_OPEN_WAIT_CLOSE	0x4000			/* wait close opertion */
#define GRP_FS_OPEN_UNDER_CLOSE	0x8000			/* under close operation */
#endif /* GRP_FS_ASYNC_UNMOUNT */

#ifdef GRP_FS_FAST_MAKE_SNAME
#define GRP_FS_MAKE_SNAME_THRESHOLD	5			/* short name threshold */
#endif /* GRP_FS_FAST_MAKE_SNAME */

/****************************************************************************/
/*  exported interaces to file system dependent routines					*/
/****************************************************************************/
int grp_fs_lookup_buf(							/* lookup buffer */
		grp_fs_info_t	*ptFs,					/* [IN]  FS information */
		grp_uint32_t	uiBlk,					/* [IN]  block number to read */
		int				iBufKind,				/* [IN]  buffer kind */
		grp_int32_t		iSize,					/* [IN]  data size */
		grp_fs_bio_t	*ptBio);				/* [OUT] buffer I/O info */
void grp_fs_unref_buf(							/* unreference buffer */
		grp_fs_bio_t	*ptBio);				/* [IN]  buffer I/O info */
void grp_fs_buf_fill_end(						/* filling buffer end */
		grp_fs_bio_t	*ptBio,					/* [IN]  buffer I/O info */
		int				iInvalidate);			/* [IN]  invalidate buffer */
void grp_fs_block_buf_mod(						/* block buffer modify */
		grp_fs_bio_t	*ptBio);				/* [IN]  buffer I/O info */
void grp_fs_unblock_buf_mod(					/* unblock buffer modify */
		grp_fs_bio_t	*ptBio,					/* [IN]  buffer I/O info */
		grp_ushort_t	usModStat);				/* [IN]  modification status */
int	grp_fs_wait_io(								/* wait buffer I/O */
		int				iDev,					/* [IN]  device number */
		int				iMode);					/* [IN]  wait mode */
void grp_fs_block_fs_mod(						/* block file system modify */
		grp_fs_info_t	*ptFs);					/* [IN]  file system info */
void grp_fs_unblock_fs_mod(						/* unblock file system modify */
		grp_fs_info_t	*ptFs);					/* [IN]  file system info */
void grp_fs_block_file_op(						/* block file operation */
		grp_fs_file_t	*ptFile);				/* [IN]  file information */
void grp_fs_unblock_file_op(					/* unblock file operation */
		grp_fs_file_t	*ptFile);				/* [IN]  file information */
grp_fs_file_t * grp_fs_block_file_op_by_id(		/* block file op by file id */
		grp_fs_info_t	*ptFs,					/* [IN]  file system info */
		grp_uint32_t	uiFid);					/* [IN]  file id to block */
void grp_fs_unblock_file_op_by_id(				/* unblock file op by id */
		grp_fs_info_t	*ptFs);					/* [IN]  file system info */
void grp_fs_block_file_write(					/* block file write */
		grp_fs_file_t	*ptFile);				/* [IN]  file information */
void grp_fs_unblock_file_write(					/* unblock file write */
		grp_fs_file_t	*ptFile);				/* [IN]  file information */
void grp_fs_close_file(							/* close file */
		grp_fs_file_t	*ptFile,				/* [IN] file info to close */
		int				iMode);					/* [IN] close mode */
int grp_fs_lookup_file_ctl(						/* lookup file information */
		grp_fs_info_t	*ptFs,					/* [IN]  FS information */
		grp_uint32_t	uiFid,					/* [IN]  file ID */
		int				iAlloc,					/* [IN]  alloc if not found */
		grp_fs_file_t	**pptFile);				/* [OUT] file control info */
void grp_fs_change_fid(							/* change file id */
		grp_fs_file_t	*ptFile,				/* [IN]  file control info */
		grp_uint32_t	uiFid);					/* [IN]  file ID */
#ifdef	GRP_FS_FNAME_CACHE
int grp_fs_lookup_fname_cache(					/* lookup file name cache */
		grp_fs_file_t	*ptDir,					/* [IN]  parent directory */
		const grp_uchar_t *pucName,				/* [IN]  file name */
		int				iPurge,					/* [IN]  purge cache entry */
		grp_fs_file_t	**pptFile);				/* [OUT] found file info */
grp_fs_fname_cache_t *grp_fs_set_fname_cache(	/* set file name cache */
		grp_fs_file_t	*ptDir,					/* [IN]  parent directory */
		const grp_uchar_t *pucName,				/* [IN]  file name */
		grp_fs_file_t	*ptFile,				/* [IN]  file information */
		grp_fs_fname_cache_t *ptAlias);			/* [IN]  alias cache entry */
void grp_fs_purge_fname_cache_by_dev(			/* purge file name cache */
		int				iDev);					/* [IN]  device ID to purge */
void grp_fs_purge_fname_cache_entry(			/* purge file name cache ent */
		grp_fs_fname_cache_t	*ptCache);		/* [IN]  file name cache ent */
void grp_fs_purge_fname_cache(					/* purge file name cache */
		grp_fs_file_t	*ptDir,					/* [IN]  parent directory */
		const grp_uchar_t *pucName);			/* [IN]  file name */
#endif	/* GRP_FS_FNAME_CACHE */
int grp_fs_check_io_status(						/* check I/O status */
		grp_fs_info_t	*ptFs,					/* [IN]  FS information */
		grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
		grp_isize_t		iCnt,					/* [IN]  device block I/O cnt */
		int				iMode);					/* [IN]  I/O mode */
grp_int32_t grp_fs_read_buf(					/* read data in buffer */
		grp_fs_info_t	*ptFs,					/* [IN]  FS information */
		grp_uint32_t	uiBlk,					/* [IN]  block number to read */
		int				iBufKind,				/* [IN]  buffer kind */
		grp_int32_t		iSize,					/* [IN]  data size */
		grp_fs_bio_t	*ptBio);				/* [OUT] buffer I/O info */
grp_int32_t grp_fs_write_buf(					/* write data in buffer */
		grp_fs_bio_t	*ptBio);				/* [IN]  buffer I/O info */
grp_int32_t grp_fs_exec_dev_io(					/* execute device I/O */
		grp_fs_info_t	*ptFs,					/* [IN]  head of buffer list */
		grp_uint32_t	uiBlk,					/* [IN]  block number */
		grp_uchar_t		*pucBuf,				/* [IN or OUT] I/O buffer */
		grp_isize_t		iCnt,					/* [IN]  device block count */
		int				iOp,					/* [IN]  operation mode */
		int				iBlkKind);				/* [IN]  I/O block kind */
int grp_fs_get_path_comp(						/* get a path component */
		const grp_uchar_t **ppucPath,			/* [IN/OUT]  path name */
		grp_uchar_t		*pucComp,				/* [OUT] component buf */
		int				iCompBufSize);			/* [IN]  component buf size */
void grp_fs_set_access_time(					/* set access time */
		grp_fs_file_t	*ptFile);				/* [IN] file information */
int grp_fs_file_open_common(					/* common file open function */
		grp_fs_info_t		*ptFs,				/* [IN]  file system info */
		grp_fs_file_t		*ptDir,				/* [IN]  search start dir */
		const grp_uchar_t	**ppucPath,			/* [IN/OUT]  path name */
		int					iMode,				/* [IN]  open mode */
		grp_uchar_t			*pucComp,			/* [IN/OUT] component buffer */
		grp_fs_file_t		**pptOpened);		/* [OUT] opened file info */
int grp_fs_get_mount_root_attr(					/* get mounted root attribute */
		grp_fs_file_t	*ptFile,				/* [IN] file mounted on */
		grp_fs_dir_ent_t *ptDirEnt);			/* [OUT] directory entry */
grp_fs_info_t * grp_fs_check_mnt_dev(			/* check mounted device */
		grp_fs_info_t	*ptFs,					/* [IN]  FS info to check */
		int				iDev);					/* [IN]  device number */
int grp_fs_check_dev_busy(						/* check device busy */
		int				iDev);					/* [IN]  device number */

/****************************************************************************/
/*  exported variables														*/
/****************************************************************************/
extern grp_fs_ctl_t	*grp_fs_ctl;				/* control block of GR-FILE */

#endif	/* _GRP_FS_H_ */
