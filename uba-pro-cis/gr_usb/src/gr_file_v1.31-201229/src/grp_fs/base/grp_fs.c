/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs.c													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		File system management routines										*/
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
/*		<string.h>															*/
/*		grp_fs.h															*/
/*		grp_mem.h															*/
/*		grp_time_lib.h														*/
/*		grp_fs_trace.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/06/14	Added file name cache function			*/
/*									Optimized file create sequence			*/
/*		T.Imashiki		2004/07/25	Changed grp_fs_lookup_dev to support	*/
/*									sub-device id and partition				*/
/*									Added grp_fs_check_dev_busy and			*/
/*									_grp_fs_check_mnt_dev_busy				*/
/*									Added lock/unlock media at mount/umount */
/*									Added GRP_FS_NO_UPD_ACCTIME processing	*/
/*									Added GRP_FS_NO_MNT_FLAG processing		*/
/*									Changed constant GRP_FS_MAX_DEV to a	*/
/*									variable grp_fs_dev_tbl_cnt				*/
/*									Added GRP_FS_NO_CRT_ACCTIME processing	*/
/*		T.Imashiki		2004/11/30	Fixed bug not checking invalid current	*/
/*									directory at grp_fs_chdir				*/
/*		T.Imashiki		2004/12/07	Added buffer kind parameter to grp_fs_	*/
/*									exec_dev_io								*/
/*		T.Imashiki		2004/12/24	Added grp_fs_change_fid and grp_fs_		*/
/*									block/unblock_file_op_by_id, and		*/
/*									modified grp_fs_block_file_op to fix	*/
/*									bug of not complying treatment of size */
/*									0 file									*/
/*		T.Imashiki		2005/02/10	Changed return types of grp_fs_read,	*/
/*									grp_fs_write, grp_fs_get_error,			*/
/*									grp_fs_read_buf and grp_fs_write_buf	*/
/*									for 16 bit CPU support					*/
/*									Added type casts and changed types of	*/
/*									some variables for 16 bit CPU support	*/
/*		T.Imashiki		2005/06/21	Fixed bug not properly handling file	*/
/*									handle in _grp_fs_invalidate_fhdl		*/
/*									invalidated by force unmount			*/
/*		T.Imashiki		2005/11/04	Fixed dead lock by ".." access from     */
/*									multiple tasks at grp_fs_lookup_fname_	*/
/*									cache 								    */
/*									Changed close operation to wait instead */
/*									of returning BUSY when the file is busy */
/*		T.Imashiki		2005/11/25	Changed to always clear buffer pointer  */
/*									in error case at grp_fs_lookup_buf for	*/
/*									safety									*/
/*									Added grp_fs_purge_fname_cache for		*/
/*									simple purge case						*/
/*									Added grp_fs_block_file_write and		*/
/*									grp_fs_unblock_file_write				*/
/*									Changed to wait BUSY at _grp_fs_close_	*/
/*									fhdl									*/
/*		T.Imashiki		2005/12/16	Fixed to reject O_TRUNC and write mode	*/
/*									open with O_CREAT without write			*/
/*									permission								*/
/*		T.Imashiki		2006/1/31	Exported grp_fs_purge_fname_cache_entry	*/
/*		T.Imashiki		2006/2/17	Fixed _grp_fs_return_mnt_info to return */
/*									correct device partition name			*/
/*		T.Imashiki		2006/2/23	Fixed variable name mistake for write	*/
/*									mode open fix with O_CREAT				*/
/*		T.Imashiki		2006/8/10	Fixed misupdate iFsRef by grp_fs_open	*/
/*									with O_TRUNC for cross mount point path */
/*		T.Imashiki		2006/10/31	Added FS dependent sync function call	*/
/*									 to cache write back, and make/merged	*/
/*									 the cache write back processing to		*/
/*									 _grp_fs_sync_fs&_grp_fs_sync_dep_info	*/
/*									Fixed to write back with both SYNC and	*/
/*									 TSYNC bits set at grp_fs_unref_buf		*/
/*						2006/10/31	Modified to match both '/' and '\' as	*/
/*									 root regardless of mounted as '/' or	*/
/*									 '\'									*/
/*									Modified to return error number from	*/
/*									 device driver instead of GRP_FS_ERR_IO	*/
/*		T.Imashiki		2006/11/10	Set file size limit to 2GB at write		*/
/*		T.Imashiki		2006/11/29	Check with timezone consideration for	*/
/*									date only access time update			*/
/*		T.Imashiki		2007/02/20	Added type casts for 16 bit CPU support	*/
/*		T.Imashiki		2007/09/21	Block unmount operation by iFsWaitCnt	*/
/*						2007/09/21	Deleted incorrect _grp_fs_wakeup_wait_	*/
/*									mount call at grp_fs_sync				*/
/*		K.Kaneko		2008/01/11	Change buffer size of aucComp in		*/
/*									grp_fs_get_dirent function				*/
/*      M.Toyama        2008/03/05  Include multi language support function,*/
/*									and added grp_fs_get_dirent_common()	*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		T.Imashiki		2010/11/16	Fixed to release task environment in	*/
/*		K.Kaneko					 grp_fs_close							*/
/*									Changed to use iMode parameter in		*/
/*									 _grp_fs_check_fhdl_common				*/
/*									Deleted an unused variable ptTask in	*/
/*									 grp_fs_get_dirent_common				*/
/*									Fixed to return GRP_FS_ERR_PERMIT error */
/*									 for grp_fs_set_attr call to read-only	*/
/*									 file system							*/
/*									Added grp_fs_check_io_status call to	*/
/*									 grp_fs_truncate						*/
/*									Fixed to set ptBuf before call to grp_	*/
/*									 fs_unblock_buf_mod from grp_fs_unref_	*/
/*									 buf (Note: Since grp_fs_unref_buf is	*/
/*									 not called with GRP_FS_BSTAT_MOD		*/
/*									 status, call to grp_fs_unblock_buf_mod	*/
/*									 will not happen, and this bug fix is	*/
/*									 not critical)							*/
/*									Added to return mount option flags not	*/
/*									 returned by _grp_fs_return_mnt_info	*/
/*									Added open count check to reduce file	*/
/*									 handle check loop in _grp_fs_task_free	*/
/*									 _env_by_task							*/
/*									Made recovery processing in case of		*/
/*									 detection of internal error in grp_fs_	*/
/*									 close_file								*/
/*									Re-checked buffer list again in case	*/
/*									 of write failure considering parallel	*/
/*									 I/O during waiting write in _grp_fs_	*/
/*									 wait_buf_io							*/
/*									Changed I/O error message in grp_fs_	*/
/*									 write_buf and grp_fs_exec_dev_io		*/
/*									Fixed to execute file system dependent	*/
/*									 sync function of sub-layer file system	*/
/*									 even if the upper layer does not have	*/
/*									 sync function in _grp_fs_sync_dep_info	*/
/*									Added null check to grp_fs_lseek		*/
/*									 function								*/
/*		T.Imashiki		2011/05/23	Added ptFs parameter to grp_fs_task_	*/
/*		K.Kaneko					wait to	allow unmount not waited FS		*/
/*									Changed to update ptFs->iFsRef instead 	*/
/*									of ptFsCtl->iFsWaitCnt during wait in	*/
/*									order to allow unmount not waited FS	*/
/*									Added under close check at _grp_fs_		*/
/*									_close_fhdl to allow unmount not waited	*/
/*									FS										*/
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
/*									Added finction grp_fs_lseek4G			*/
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

#include <string.h>
#include "grp_fs_sysdef.h"
#include "grp_fs.h"
#include "grp_mem.h"
#include "grp_time_lib.h"
#include "grp_fs_trace.h"

grp_fs_ctl_t	*grp_fs_ctl = NULL;			/* file system control data */
grp_fs_inform_io_err_func_t *grp_fs_inform_io_err = NULL;
											/* inform func for I/O error */

/****************************************************************************/
/* forward prototype declartions											*/
/****************************************************************************/
static	int		_grp_fs_close_fhdl(grp_fs_fhdl_t *ptFhdl);
static	int		_grp_fs_sync_buf_io(
						grp_fs_buf_t **pptBufHead, int iDev, int iMode);
static	int		_grp_fs_sync_fs(grp_fs_info_t *ptFs, int iMode);

/****************************************************************************/
/* FUNCTION:	_grp_fs_revoke_mount										*/
/*																			*/
/* DESCRIPTION:	Revoke mounted file systems									*/
/* INPUT:		ptFs:				file system information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_revoke_mount(
	grp_fs_info_t	*ptFs)				/* [IN]  file system information */
{
	for ( ; ptFs; ptFs = ptFs->ptFsOtherFwd) {	/* FSs mounted on the FS */
		if (ptFs->ptFsNest)						/* nested mount exists */
			_grp_fs_revoke_mount(ptFs->ptFsNest);/* revoke nested mount */
		ptFs->ptFsOp->pfnUmount(ptFs, (int)GRP_FS_REVOKE_MOUNT);
												/* revoke mount */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_init													*/
/*																			*/
/* DESCRIPTION:	Allocate and initialize file system management tables		*/
/* INPUT:		grp_fs_param:	FS parameters								*/
/* OUTPUT:		various FS management table									*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_PARAM: bad initialization parameter			*/
/*				GRP_FS_ERR_NOMEM:	no memory								*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_init(void)
{
	grp_fs_ctl_t	*ptFsCtl;		/* file system control block */
	grp_fs_param_t	*ptFsParam;		/* FS management parameter */
	grp_uint32_t	uiSize;			/* size to allocate */
	grp_uint32_t	uiBlk;			/* buffer count */
	grp_uint32_t	uiMntSz;		/* mount table size */
	grp_uint32_t	uiTaskSz;		/* task table size */
	grp_uint32_t	uiBlkTblSz;		/* buffer table size */
	grp_uint32_t	uiBHashSz;		/* buffer hash size */
	grp_uint32_t	uiFileSz;		/* file table size */
	grp_uint32_t	uiFhdlSz;		/* file handle size */
	grp_uint32_t	uiFHashSz;		/* file hash size */
#ifdef	GRP_FS_FNAME_CACHE
	grp_uint32_t	uiFnCacheSz;	/* file name cache size */
	grp_uint32_t	uiFnHashSz;		/* file name hash size */
	grp_fs_fname_cache_t *ptFnCache;/* file name cache pointer */
#endif	/* GRP_FS_FNAME_CACHE */
	grp_uchar_t		*pucTblArea;	/* table area */
	grp_uchar_t		*pucBufArea;	/* buffer area */
	grp_uchar_t		*pucDataBuf;	/* buffer pointer */
	grp_fs_info_t	*ptFs;			/* FS info pointer */
	grp_fs_task_ctl_t *ptTask;		/* task control pointer */
	grp_fs_buf_t	*ptBuf;			/* buffer table pointer */
	grp_fs_file_t	*ptFile;		/* file table pointer */
	grp_fs_fhdl_t	*ptFhdl;		/* file handle pointer */
	grp_uint32_t	uiCnt;			/* loop index */

	/****************************************************/
	/* I/O buffer trace for debug						*/
	/****************************************************/
	grp_fs_trace_init();								/* I/O buffer trace */

	/****************************************************/
	/* in case of reinitialize, free previous one		*/
	/****************************************************/
	ptFsCtl = grp_fs_ctl;								/* FS control block */
	if (ptFsCtl) {										/* already allocated */
		if (ptFsCtl->ptFsMnt)							/* mounted FSs exist */
			_grp_fs_revoke_mount(ptFsCtl->ptFsMnt);		/* revoke mount */
		grp_mem_free(ptFsCtl->pvBufMem);				/* free buffer mem */
		ptFsCtl->pvBufMem = NULL;						/* clear it */
		grp_mem_free(ptFsCtl);							/* free table mem */
		grp_fs_ctl = NULL;								/* no control table */
	}

	/****************************************************/
	/* check parameter									*/
	/****************************************************/
	ptFsParam = &grp_fs_param;							/* FS parameter */
	if ((ptFsParam->uiBHashCnt & (grp_fs_param.uiBHashCnt - 1)) != 0
		|| (ptFsParam->uiFHashCnt & (grp_fs_param.uiFHashCnt - 1)) != 0
		|| ptFsParam->sMounts <= 0
		|| ptFsParam->sTasks <= 0
		|| ptFsParam->ucFBlkShift == 0
		|| ptFsParam->ucDBlkShift == 0
		|| ptFsParam->uiFBlkCnt == 0
		|| ptFsParam->uiDBlkCnt == 0
		|| ptFsParam->uiFileCnt == 0
		|| ptFsParam->uiFhdlCnt == 0
		|| ptFsParam->uiBHashCnt == 0
		|| ptFsParam->uiFHashCnt == 0)
		return(GRP_FS_ERR_BAD_PARAM);					/* invalid param */

	/****************************************************/
	/* allocate area									*/
	/****************************************************/
	uiBlk = ptFsParam->uiFBlkCnt + ptFsParam->uiDBlkCnt;/* buffer count */
	uiMntSz = (grp_uint32_t)ptFsParam->sMounts * sizeof(grp_fs_info_t);
														/* mount tbl size */
	uiTaskSz = (grp_uint32_t)ptFsParam->sTasks * sizeof(grp_fs_task_ctl_t);
														/* task tbl size */
	uiBlkTblSz = uiBlk * sizeof(grp_fs_buf_t);			/* buf tbl size */
	uiFhdlSz = ptFsParam->uiFhdlCnt * sizeof(grp_fs_fhdl_t);/* file hdl size */
	uiFileSz = ptFsParam->uiFileCnt * sizeof(grp_fs_file_t);/* file info size */
	uiBHashSz = ptFsParam->uiBHashCnt * sizeof(grp_fs_buf_t *);/* buf hash sz */
	uiFHashSz = ptFsParam->uiFHashCnt * sizeof(grp_fs_file_t *);/* file hash  */
	ptFsParam->uiFBlkSize = ((grp_uint32_t)1 << ptFsParam->ucFBlkShift);
														/* file blk size */
	ptFsParam->uiDBlkSize = ((grp_uint32_t)1 << ptFsParam->ucDBlkShift);
														/* data blk size */
	uiSize = sizeof(grp_fs_ctl_t) + uiMntSz + uiTaskSz + uiBlkTblSz + uiFhdlSz
			+ uiFileSz + uiBHashSz + uiFHashSz;			/* total size */
#ifdef	GRP_FS_FNAME_CACHE
	uiFnCacheSz = ptFsParam->uiFnameCacheCnt * sizeof(grp_fs_fname_cache_t);
	uiFnHashSz = ptFsParam->uiFnameHashCnt * sizeof(grp_fs_fname_cache_t *);
	uiSize += uiFnCacheSz + uiFnHashSz;					/* add fname cache */
#endif	/* GRP_FS_FNAME_CACHE */
	pucTblArea = grp_mem_alloc((grp_isize_t)uiSize);	/* alloc table */
	pucBufArea = grp_mem_alloc((grp_isize_t)
								(ptFsParam->uiFBlkSize * ptFsParam->uiFBlkCnt +
								 ptFsParam->uiDBlkSize * ptFsParam->uiDBlkCnt));
														/* alloc buf */
	grp_fs_ctl = (grp_fs_ctl_t *)pucTblArea;			/* table area */
	if (pucTblArea == NULL || pucBufArea == NULL)		/* not allocated */
		goto err_ret;
	memset(pucTblArea, 0, uiSize);						/* clear table area */
	ptFsCtl = grp_fs_ctl;								/* FS control block */
	pucTblArea += sizeof(grp_fs_ctl_t);					/* advance to next */
	ptFsCtl->pvBufMem = pucBufArea;						/* buffer area */

	/****************************************************/
	/* set FS info free list							*/
	/****************************************************/
	ptFsCtl->ptFsFree = (grp_fs_info_t *)pucTblArea;	/* set free mount */
	ptFs = ptFsCtl->ptFsFree;							/* set pointer */
	for (uiCnt = 0; (int)uiCnt < ptFsParam->sMounts; uiCnt++, ptFs++)
		ptFs->ptFsOtherFwd = &ptFs[1];					/* link free list */
	ptFs[-1].ptFsOtherFwd = NULL;						/* clear tail */
	pucTblArea += uiMntSz;								/* advance to next */

	/****************************************************/
	/* set task control table							*/
	/****************************************************/
	ptFsCtl->ptTaskFree = (grp_fs_task_ctl_t *)pucTblArea;/* set task table */
	ptTask = ptFsCtl->ptTaskFree;						/* set pointer */
	for (uiCnt = 0; (int)uiCnt < ptFsParam->sTasks; uiCnt++, ptTask++) {
		ptTask->tTaskId = GRP_FS_NO_TASK;				/* no task */
		ptTask->ptTaskListFwd = &ptTask[1];				/* link to next */
		if (grp_fs_create_sem(&ptTask->tTaskSem,		/* create semaphore */
								"FS_TK", (int)uiCnt, 0)) /* failed */
			goto err_ret;								/* error return */
	}
	ptTask[-1].ptTaskListFwd = NULL;					/* clear tail */
	pucTblArea += uiTaskSz;								/* advance to next */

	/****************************************************/
	/* set buffer list									*/
	/****************************************************/
	ptBuf = (grp_fs_buf_t *)pucTblArea;					/* set pointer */
	pucDataBuf = pucBufArea;							/* data buf pointer */
	for (uiCnt = 0; uiCnt < ptFsParam->uiFBlkCnt; uiCnt++, ptBuf++) {
		ptBuf->iDev = -1;								/* no dev */
		ptBuf->usStatus = GRP_FS_BSTAT_FBUF;			/* file buffer */
		ptBuf->pucData = pucDataBuf;					/* set data buffer */
		pucDataBuf += ptFsParam->uiFBlkSize;			/* advance to next */
		grp_enque_tail(ptFsCtl, ptFBuf, ptBuf, ptList);	/* chain it */
	}
	for (uiCnt = 0; uiCnt < ptFsParam->uiDBlkCnt; uiCnt++, ptBuf++) {
		ptBuf->iDev = -1;								/* no dev */
		ptBuf->usStatus = GRP_FS_BSTAT_DBUF;			/* data buffer */
		ptBuf->pucData = pucDataBuf;					/* set data buffer */
		pucDataBuf += ptFsParam->uiDBlkSize;			/* advance to next */
		grp_enque_tail(ptFsCtl, ptDBuf, ptBuf, ptList);	/* chain it */
	}
	pucTblArea += uiBlkTblSz;							/* advance to next */

	/****************************************************/
	/* set file handles									*/
	/****************************************************/
	ptFhdl = (grp_fs_fhdl_t *)pucTblArea;				/* set pointer */
	ptFsCtl->ptFhdlTbl = ptFhdl;						/* set file hdl table */
	ptFsCtl->ptFhdlFree = ptFhdl;						/* set free list */
	for (uiCnt = 0; uiCnt < ptFsParam->uiFhdlCnt; uiCnt++, ptFhdl++) {
		ptFhdl->sHdlId = (short)uiCnt;					/* set handle id */
		ptFhdl->ptTask = NULL;							/* not assoc task */
		grp_fs_fhdl_list(ptFhdl) = &ptFhdl[1];			/* chain free list */
	}
	grp_fs_fhdl_list(&ptFhdl[-1]) = NULL;				/* clear tail */
	pucTblArea += uiFhdlSz;								/* advance to next */

	/****************************************************/
	/* set file list									*/
	/****************************************************/
	ptFile = (grp_fs_file_t *)pucTblArea;				/* set pointer */
	for (uiCnt = 0; uiCnt < ptFsParam->uiFileCnt; uiCnt++, ptFile++) {
		ptFile->iDev = -1;								/* no assoc dev */
		grp_enque_tail(ptFsCtl, ptFile, ptFile, ptList); /* chain it */
	}
	pucTblArea += uiFileSz;								/* advance to next */

	/****************************************************/
	/* set hash list									*/
	/****************************************************/
	ptFsCtl->pptBufHash = (grp_fs_buf_t **)pucTblArea;	/* set buf hash */
	pucTblArea += uiBHashSz;							/* advance to next */
	ptFsCtl->pptFileHash = (grp_fs_file_t **)pucTblArea; /* set file hash */
	pucTblArea += uiFHashSz;							/* advance to next */

#ifdef	GRP_FS_FNAME_CACHE
	/****************************************************/
	/* initialize file name cache						*/
	/****************************************************/
	ptFnCache = (grp_fs_fname_cache_t *)pucTblArea;		/* set pointer */
	for (uiCnt = 0; uiCnt < ptFsParam->uiFnameCacheCnt; uiCnt++, ptFnCache++) {
		ptFnCache->iDev = -1;							/* no assoc dev */
		grp_enque_tail(ptFsCtl, ptFnCache, ptFnCache, ptList); /* chain it */
	}
	pucTblArea += uiFnCacheSz;							/* advance to next */
	ptFsCtl->pptFnHash = (grp_fs_fname_cache_t **)pucTblArea;/* set hash */
	pucTblArea += uiFnHashSz;							/* advance to next */
#endif	/* GRP_FS_FNAME_CACHE */

	/****************************************************/
	/* create samaphore									*/
	/****************************************************/
	if (grp_fs_create_sem(&ptFsCtl->tFsSem, "FS_CTL", 0, 1))/* create failed */
		goto err_ret;									/* error return */
	return(0);

err_ret:
	if (pucBufArea) 									/* buffer allocated */
		grp_mem_free(pucBufArea);						/* free it */
	if (grp_fs_ctl) {
		grp_mem_free(grp_fs_ctl);						/* free it */
		grp_fs_ctl = NULL;								/* reset pointer */
	}
	return(GRP_FS_ERR_NOMEM);							/* no memory */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_task_lookup_env_by_id								*/
/*																			*/
/* DESCRIPTION:	Lookup task environment by task id							*/
/* INPUT:		tTaskId:			task id									*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL				not found								*/
/*				others				found task environment					*/
/*																			*/
/****************************************************************************/
static grp_fs_task_ctl_t *
_grp_fs_task_lookup_env_by_id(
	grp_fs_task_t		tTaskId)				/* task id */
{
	grp_fs_task_ctl_t	*ptTask;				/* task pointer */
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;	/* FS control data */

	/****************************************************/
	/* check active task								*/
	/****************************************************/
	for (ptTask = ptFsCtl->ptTask; ptTask; ptTask = ptTask->ptTaskListFwd) {
		if (ptTask->tTaskId == tTaskId) 			/* found */
			return(ptTask);
	}
	return(NULL);
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_task_lookup_env										*/
/*																			*/
/* DESCRIPTION:	Lookup task environment										*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL				not found								*/
/*				others				found task environment					*/
/*																			*/
/****************************************************************************/
static grp_fs_task_ctl_t *
_grp_fs_task_lookup_env(void)
{
	return(_grp_fs_task_lookup_env_by_id(grp_fs_get_taskid()));
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_task_invaidate_cd									*/
/*																			*/
/* DESCRIPTION:	invalidate current directory for the device					*/
/* INPUT:		iDev:			device number								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_task_invalidate_cd(
	int			iDev)						/* [IN]  device number */
{
	grp_fs_task_ctl_t	*ptTask;				/* task pointer */
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;	/* FS control data */

	for (ptTask = ptFsCtl->ptTask; ptTask; ptTask = ptTask->ptTaskListFwd) {
		if (ptTask->ptCurDir == NULL
			|| ptTask->ptCurDir == GRP_FS_INV_CD)/* no current directory */
			continue;							/* search next */
		if (ptTask->ptCurDir->iDev != iDev)		/* not match */
			continue;							/* search next */
		grp_fs_close_file(ptTask->ptCurDir, GRP_FS_FILE_INVALID);
												/* close file */
		ptTask->ptCurDir = GRP_FS_INV_CD;		/* bad current directory */
	}
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_task_get_env										*/
/*																			*/
/* DESCRIPTION:	Get task environment										*/
/* INPUT:		None														*/
/* OUTPUT:		pptFsTask			task environment						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_TOO_MANY	too many tasks							*/
/*				0					success									*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_task_get_env(
	grp_fs_task_ctl_t	**pptFsTask)		/* [OUT] task environment */
{
	grp_fs_task_ctl_t	*ptTask;				/* task pointer */
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;	/* FS control data */

	/****************************************************/
	/* check active task								*/
	/****************************************************/
	ptTask = _grp_fs_task_lookup_env();				/* lookup task env */
	if (ptTask) {									/* found */
		/****************************************************/
		/* rechain to top									*/
		/****************************************************/
		if (ptTask != ptFsCtl->ptTask) {
			grp_deque_sent(&ptFsCtl->ptTask, ptTask, ptTaskList);
			grp_enque_shead(&ptFsCtl->ptTask, ptTask, ptTaskList);
		}
		*pptFsTask = ptTask;					/* set task */
		return(0);								/* return success */
	}

	/****************************************************/
	/* get free task environment						*/
	/****************************************************/
	if ((ptTask = ptFsCtl->ptTaskFree) == NULL) {	/* no free entry */
		*pptFsTask = NULL;							/* no task env */
		return(GRP_FS_ERR_TOO_MANY);				/* return error */
	}
	ptFsCtl->ptTaskFree = ptTask->ptTaskListFwd;	/* remove from free list */
	ptTask->tTaskId = grp_fs_get_taskid();			/* set task id */
	ptTask->pvWaitRsc = NULL;						/* no wait */
	ptTask->iOpenCnt = 0;							/* no opens */
	ptTask->ptCurDir = NULL;						/* no current directory */
	grp_enque_shead(&ptFsCtl->ptTask, ptTask, ptTaskList);/* active list */
	*pptFsTask = ptTask;							/* set task */
	return(0);										/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_task_free_env_by_task								*/
/*																			*/
/* DESCRIPTION:	Free task environment										*/
/* INPUT:		ptTask:		task environment to free						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_BUSY:	file busy error							*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_task_free_env_by_task(
	grp_fs_task_ctl_t	*ptTask)			/* [IN] task env to release */
{
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;/* FS control data */
	int					iHdl;				/* file hundle number */
	int					iHdlCnt;			/* file hundle count */
	grp_fs_fhdl_t		*ptFhdl;			/* file handle */
	int					iRet = 0;			/* return value */

	/****************************************************/
	/* find task environment if not specified			*/
	/****************************************************/
	if (ptTask == NULL) {					/* task is not specified */
		ptTask = _grp_fs_task_lookup_env();	/* lookup task env */
		if (ptTask == NULL)					/* not found */
			return(0);						/* do nothing */
	}

	/****************************************************/
	/* close current directory							*/
	/****************************************************/
	if (ptTask->ptCurDir) {					/* current directory exists */
		if (ptTask->ptCurDir != GRP_FS_INV_CD) {	/* valid diretory */
			if (ptTask->ptCurDir->usStatus & GRP_FS_FSTAT_BUSY) {
				iRet = GRP_FS_ERR_BUSY;
			} else {
				grp_fs_close_file(ptTask->ptCurDir, 0);	/* close it */
				ptTask->ptCurDir = NULL;	/* clear it */
			}
		} else
			ptTask->ptCurDir = NULL;		/* clear it */
	 }

	/****************************************************/
	/* close open files									*/
	/****************************************************/
	if (ptTask->iOpenCnt) {
		iHdlCnt = (int)grp_fs_param.uiFhdlCnt; /* file handle count */
		ptFhdl = ptFsCtl->ptFhdlTbl;		/* file handle table */
		for (iHdl = 0; iHdl < iHdlCnt && ptTask->iOpenCnt; iHdl++, ptFhdl++) {
			if (ptFhdl->ptTask == ptTask) {	/* match */
				if (_grp_fs_close_fhdl(ptFhdl) != 0)/* close file handle */
					iRet = GRP_FS_ERR_BUSY;	/* busy error */
			}
		}
		if (iRet == 0)						/* success */
			ptTask->iOpenCnt = 0;			/* clear open count for safty */
	}
	if (iRet != 0)							/* error */
		return(iRet);						/* return error */

	/****************************************************/
	/* free task table									*/
	/****************************************************/
	ptTask->tTaskId = GRP_FS_NO_TASK;		/* no task assoc */
	grp_deque_sent(&ptFsCtl->ptTask, ptTask, ptTaskList);/* deque it */
	ptTask->ptTaskListFwd = ptFsCtl->ptTaskFree;/* insert to free list */
	ptFsCtl->ptTaskFree = ptTask;			/* chain to free list */
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_task_rel_env										*/
/*																			*/
/* DESCRIPTION:	Release task environment									*/
/*				If no state is left, free it								*/
/* INPUT:		ptTask:		task environment to release						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_task_rel_env(
	grp_fs_task_ctl_t	*ptTask)			/* [IN] task env to release */
{
	if (ptTask->iOpenCnt || ptTask->ptCurDir) /* state remains */
		return;								/* keep envirnment */
	_grp_fs_task_free_env_by_task(ptTask);	/* free task environment */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_task_wait											*/
/*																			*/
/* DESCRIPTION:	Wait for a resource											*/
/* INPUT:		pvWaitPoint			wait point								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_task_wait(
#ifdef GRP_FS_ASYNC_UNMOUNT
	grp_fs_info_t	*ptFs,						/* [IN] FS information */
	void			*pvWaitPoint)				/* [IN] wait point */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	void			*pvWaitPoint)				/* [IN] wait point */
#endif /* GRP_FS_ASYNC_UNMOUNT */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	grp_fs_task_ctl_t *ptTask;					/* task environment */
	int				iAllocTask = 0;				/* allocate task */

	ptTask = _grp_fs_task_lookup_env();			/* lookup task env */
	if (ptTask == NULL) {						/* no task environment */
		if (_grp_fs_task_get_env(&ptTask) != 0)	{ /* get task environment */
			grp_fs_printf("GRP_FS: no task environment(%lu)\n",
							(unsigned long)grp_fs_get_taskid());
			return;								/* return */
		}
		iAllocTask = 1;							/* allocated task environment */
	}
	ptTask->pvWaitRsc = pvWaitPoint;			/* set wait point */
#ifdef GRP_FS_ASYNC_UNMOUNT
	if (ptFs != NULL) {							/* exist target FS */
		ptFs->iFsRef++;							/* increment reference count */
	}
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt++;						/* increment wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release FS management */
	grp_fs_get_sem(ptTask->tTaskSem);			/* wait for wakeup */
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* lock FS management */
#ifdef GRP_FS_ASYNC_UNMOUNT
	if (ptFs != NULL) {							/* exist target FS */
		ptFs->iFsRef--;							/* decrement reference count */
	}
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt--;						/* decrement wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	if (iAllocTask)								/* task allocated */
		_grp_fs_task_rel_env(ptTask);			/* release task environment */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_task_wakeup											*/
/*																			*/
/* DESCRIPTION:	Wakeup a waiting task for a resource						*/
/* INPUT:		pvWaitPoint			wait point								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_task_wakeup(
	void			*pvWaitPoint)				/* [IN] wait point */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	grp_fs_task_ctl_t *ptTask;					/* task environment */

	for (ptTask = ptFsCtl->ptTask; ptTask; ptTask = ptTask->ptTaskListFwd) {
		if (ptTask->pvWaitRsc == pvWaitPoint) {	/* match wait point */
			ptTask->pvWaitRsc = NULL;			/* reset wait point */
			grp_fs_release_sem(ptTask->tTaskSem);/* wakeup task */
		}
	}
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_task_free_env										*/
/*																			*/
/* DESCRIPTION:	free my task environment									*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:						success								*/
/*				GRP_FS_ERR_BUSY:		file busy							*/
/*																			*/
/****************************************************************************/
int
grp_fs_task_free_env(void)
{
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;	/* FS control data */
	int					iRet;					/* return value */

	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	iRet = _grp_fs_task_free_env_by_task(NULL);	/* free my task environment */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);								/* return */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_task_free_env_by_id									*/
/*																			*/
/* DESCRIPTION:	free specified task environment								*/
/* INPUT:		tTaskId:				task id								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0						success								*/
/*				GRP_FS_ERR_NOT_FOUND	task not found						*/
/*				GRP_FS_ERR_BUSY			file busy							*/
/*																			*/
/****************************************************************************/
int
grp_fs_task_free_env_by_id(
	grp_fs_task_t		tTaskId)				/* task id */
{
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_task_ctl_t *ptTask;					/* task environment */
	int					iRet;					/* return value */

	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	ptTask = _grp_fs_task_lookup_env_by_id(tTaskId);	/* lookup task */
	if (ptTask == NULL)							/* not found */
		iRet = GRP_FS_ERR_NOT_FOUND;			/* set error */
	else
		iRet = _grp_fs_task_free_env_by_task(ptTask);/* free task environment */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);								/* return status */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_task_free_all_env									*/
/*																			*/
/* DESCRIPTION:	free all task environment									*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0						success								*/
/*				GRP_FS_ERR_BUSY			file busy							*/
/*																			*/
/****************************************************************************/
int
grp_fs_task_free_all_env(void)
{
	grp_fs_task_ctl_t	*ptTask;				/* task pointer */
	grp_fs_task_ctl_t	*ptTaskNext;			/* next task pointer */
	int					iRet = 0;				/* return value */
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;	/* FS control data */

	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	for (ptTask = ptFsCtl->ptTask; ptTask; ptTask = ptTaskNext) {
		ptTaskNext = ptTask->ptTaskListFwd;		/* set next */
		if (_grp_fs_task_free_env_by_task(ptTask) != 0)
												/* free task environment */
			iRet = GRP_FS_ERR_BUSY;				/* file busy */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);								/* return */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/****************************************************************************/
/* FUNCTION:	_grp_fs_wakeup_buf_wait										*/
/*																			*/
/* DESCRIPTION:	Wakeup task waiting for buffer								*/
/* INPUT:		ptBuf:					buffer information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_wakeup_buf_wait(
	grp_fs_buf_t		*ptBuf)				/* [IN]  buffer information */
{
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;/* FS control infomration */

	if (ptBuf->usStatus & GRP_FS_BSTAT_WAIT) {/* someone is waiting */
		ptBuf->usStatus &= ~GRP_FS_BSTAT_WAIT;/* reset wait bit */
		grp_fs_btrace(ptBuf, GRP_FS_BT_WKF); /* trace(wake_fill) */
		grp_fs_task_wakeup((void *)ptBuf);	/* wake up task */
	}
	if ((ptBuf->usStatus & GRP_FS_BSTAT_FBUF)/* release file buffer */
		&& (ptFsCtl->iFsStat & GRP_FS_CSTAT_FBWAIT)) {	/* waited file buffer */
		ptFsCtl->iFsStat &= ~GRP_FS_CSTAT_FBWAIT;		/* reset wait bit */
		grp_fs_btrace(ptBuf, GRP_FS_BT_WKB); /* trace(wakeup_buf) */
		grp_fs_task_wakeup((void *)&ptFsCtl->ptFBufBwd); /* wake up task */
	}
	if ((ptBuf->usStatus & GRP_FS_BSTAT_DBUF)/* release data buffer */
		&& (ptFsCtl->iFsStat & GRP_FS_CSTAT_DBWAIT)) {	/* waited data buffer */
		ptFsCtl->iFsStat &= ~GRP_FS_CSTAT_DBWAIT;		/* reset wait bit */
		grp_fs_btrace(ptBuf, GRP_FS_BT_WKB); /* trace(wakeup_buf) */
		grp_fs_task_wakeup((void *)&ptFsCtl->ptDBufBwd); /* wake up task */
	}
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_write_buf_int										*/
/*																			*/
/* DESCRIPTION:	Write buffer												*/
/* INPUT:		ptBuf:					buffer information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:			I/O error							*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_write_buf_int(
	grp_fs_buf_t		*ptBuf)				/* [IN]  buffer information */
{
	int					iRet;				/* return value */
	grp_int32_t			iWrite;				/* written size */
	grp_fs_bio_t		tBio;				/* buffer I/O information */

	ptBuf->iRefCnt++;						/* get reference */
	ptBuf->usStatus |= GRP_FS_BSTAT_FILL;	/* set buffer busy */
	tBio.ptBuf = ptBuf;						/* set buffer */
	tBio.uiSize = ptBuf->iSize;				/* set size */
	tBio.uiBlk = ptBuf->uiBlk;				/* set block number */
	tBio.pucData = ptBuf->pucData;			 /* set data buffer */
	iWrite = grp_fs_write_buf(&tBio);		/* write buffer */
	ptBuf->usStatus &= ~GRP_FS_BSTAT_FILL;	/* reset buffer busy */
	ptBuf->iRefCnt--;						/* decrement reference */
	iRet = (iWrite != ptBuf->iSize)? GRP_FS_ERR_IO: 0; /* return value */
	_grp_fs_wakeup_buf_wait(ptBuf);			/* wakeup task waiting for buffer */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_lookup_buf											*/
/*																			*/
/* DESCRIPTION:	Lookup associated buffer									*/
/*				Note: buffer is returned with reference count incremented	*/
/* INPUT:		ptFs:					file system information				*/
/*				uiFsBlk:				block number						*/
/*				iBufKind:				lookup kind							*/
/*											GF_FS_BUF_FILE: file buffer		*/
/*											GRP_FS_BUF_DATA: data buffer	*/
/*											GRP_FS_BUF_ALLOC: allocate buf	*/
/*				iSize:					size of data						*/
/* OUTPUT:		ptBio:					buffer I/O information				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_NOT_FOUND:	cache not found						*/
/*				GRP_FS_ERR_FS:			bad file system						*/
/*				GRP_FS_ERR_NOMEM:		no valid buffer						*/
/*				GRP_FS_ERR_TOO_BIG:		too big block size					*/
/*				0:						cache found							*/
/*																			*/
/****************************************************************************/
int
grp_fs_lookup_buf(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_uint32_t	uiBlk,				/* [IN]  block number to read */
	int				iBufKind,			/* [IN]  buffer kind */
	grp_int32_t		iSize,				/* [IN]  size of data */
	grp_fs_bio_t	*ptBio)				/* [OUT] buffer I/O information */
{
	grp_fs_buf_t	*ptBuf;				/* buffer pointer */
	grp_fs_buf_t	**pptBhash;			/* buffer hash */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;/* FS control infomration */
	grp_fs_buf_t	**pptBufList;		/* pointer to buffer list */
	grp_uint32_t	uiBlkOff;			/* block offset */
	grp_uint_t		uiShift;			/* block shift count */
	int				iBStatKind;			/* block status kind */
	int				iDev;				/* device number */
	int				iRet;				/* return vlaue */
	grp_int32_t		iValidCnt;			/* valid buffer count */

	/****************************************************/
	/* set block shift count, offset, and status		*/
	/****************************************************/
	ptBio->ptBuf = NULL;						/* no buffer */
	if (iBufKind & GRP_FS_BUF_FILE) {			/* file type buffer */
		uiShift = ptFs->ucFsFBlkShift;			/* block shift */
		uiBlkOff = ptFs->uiFsFBlkOff;			/* block offset */
		iBStatKind = GRP_FS_BSTAT_FBUF;			/* buffer kind */
		if (uiShift > grp_fs_param.ucFBlkShift)
			return(GRP_FS_ERR_TOO_BIG);			/* too big block size */
	} else {									/* data type buffer */
		uiShift = ptFs->ucFsDBlkShift;			/* block shift */
		uiBlkOff = ptFs->uiFsDBlkOff;			/* block offset */
		iBStatKind = GRP_FS_BSTAT_DBUF;			/* buffer kind */
		if (uiShift > grp_fs_param.ucDBlkShift)
			return(GRP_FS_ERR_TOO_BIG);			/* too big block size */
	}
	if (uiShift < ptFs->ucDevBlkShift			/* device block is bigger */
		|| (uiBlkOff & (((grp_uint32_t)1 << ptFs->ucDevBlkShift) - 1)))
												/* bad offset */
		return(GRP_FS_ERR_FS);					/* set error number */

	/****************************************************/
	/* look up buffer cache								*/
	/****************************************************/
	pptBhash = GRP_FS_BUF_HASH(ptFsCtl, uiBlk);	/* get hash top */
	iDev = ptFs->iDev;							/* device number */
again:
	iRet = 0;									/* set return value */
	for (ptBuf = *pptBhash; ptBuf; ptBuf = ptBuf->ptHashFwd) {
		if (ptBuf->uiBlk == uiBlk
			&& ptBuf->iDev == iDev
			&& (ptBuf->usStatus & iBStatKind)
			&& ptBuf->ptFs == ptFs) {			/* match */
			if (ptBuf->usStatus & GRP_FS_BSTAT_FILL) {/* under filling */
				ptBuf->usStatus |= GRP_FS_BSTAT_WAIT;/* set wait bit */
				grp_fs_btrace(ptBuf, GRP_FS_BT_WTF); /* trace(wait_buf) */
#ifdef GRP_FS_ASYNC_UNMOUNT
				grp_fs_task_wait(ptFs, (void *)ptBuf); /* wait for filling */
#else  /* GRP_FS_ASYNC_UNMOUNT */
				grp_fs_task_wait((void *)ptBuf);	/* wait for filling */
#endif /* GRP_FS_ASYNC_UNMOUNT */
				grp_fs_btraceX(iDev, uiBlk, iBStatKind, GRP_FS_BT_RCF);
													/* trace(check_again) */
				goto again;							/* check again */
			}
			grp_fs_btrace(ptBuf, GRP_FS_BT_FND);	/* trace(found_buf) */
			goto found;								/* found cache */
		}
	}
	iRet = GRP_FS_ERR_NOT_FOUND;				/* not found */
	if ((iBufKind & GRP_FS_BUF_ALLOC) == 0) {	/* not to allocate */
		ptBio->ptBuf = NULL;					/* clear buffer pointer */
		ptBio->pucData = NULL;					/* clear data pointer */
		ptBio->uiSize = 0; 						/* clear size */
		ptBio->uiBlk = 0;						/* clear block number */
		return(iRet);							/* return not found */
	}

	/****************************************************/
	/* no cache: get free buffer						*/
	/****************************************************/
	pptBufList = (iBufKind & GRP_FS_BUF_FILE)?	/* set tail of buf list */
				&ptFsCtl->ptFBufBwd: 			/* file buffer list */
				&ptFsCtl->ptDBufBwd;			/* data buffer list */
	iValidCnt = 0;								/* valid buffer count */
	for (ptBuf = *pptBufList; ptBuf; ptBuf = ptBuf->ptListBwd) {
		if (ptBuf->usStatus & GRP_FS_BSTAT_WFAIL) /* write failed buffer */
			continue;							/* skip it */
		iValidCnt++;							/* increment valid count */
		if (ptBuf->iRefCnt <= 0) 				/* no reference */
			break;								/* found free */
	}
	if (ptBuf == NULL) {						/* no free buffer */
		if (iValidCnt == 0)						/* no valid buffer */
			return(GRP_FS_ERR_NOMEM);			/* no free resource */
		ptFsCtl->iFsStat |= (iBufKind & GRP_FS_BUF_FILE)?
			GRP_FS_CSTAT_FBWAIT: GRP_FS_CSTAT_DBWAIT;/* set buffer wait bit */
		grp_fs_btraceX(iDev, uiBlk, iBStatKind, GRP_FS_BT_WTB);
												/* trace(wait_buf) */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(ptFs, (void *)pptBufList);	/* wait for free buffer */
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)pptBufList);	/* wait for free buffer */
#endif /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_btraceX(iDev, uiBlk, iBStatKind, GRP_FS_BT_RCB);
												/* trace(check_again) */
		goto again;								/* lookup again */
	}

	/****************************************************/
	/* if buffer is still dirty, write it back			*/
	/****************************************************/
	grp_fs_btrace(ptBuf, GRP_FS_BT_ALC);		/* trace(alloc_buf) */
	if ((ptBuf->usStatus & GRP_FS_BSTAT_DIRTY) && ptBuf->iDev >= 0) {
		_grp_fs_write_buf_int(ptBuf);			/* write back */
		grp_fs_btrace(ptBuf, GRP_FS_BT_RCW);	/* trace(check_again) */
		goto again;								/* search again */
	}

	/****************************************************/
	/* set device and block number information			*/
	/****************************************************/
	ptBuf->iRefCnt = 0;							/* init reference count */
	ptBuf->iDev = iDev;							/* set device number */
	ptBuf->uiBlk = uiBlk;						/* set block number */
	ptBuf->ptFs = ptFs;							/* set FS */
	ptBuf->usStatus &= (GRP_FS_BSTAT_FBUF|GRP_FS_BSTAT_DBUF);/* clean flags */
	ptBuf->usStatus |= GRP_FS_BSTAT_FILL;		/* under filling */
	if (ptFs->usStatus & GRP_FS_STAT_SYNC_ALL)	/* sync write always */
		ptBuf->usStatus |= GRP_FS_BSTAT_SYNC;	/* set sync write bit */
	grp_fs_btrace(ptBuf, GRP_FS_BT_INT);		/* trace(init_buf) */

	/****************************************************/
	/* rechain hash and LRU list						*/
	/****************************************************/
found:
	if (ptBuf->usStatus & GRP_FS_BSTAT_FBUF) {	/* file buffer */
		grp_deque_ent(ptFsCtl, ptFBuf, ptBuf, ptList); /* deque it */
		grp_enque_head(ptFsCtl, ptFBuf, ptBuf, ptList);/* enque file top */
	} else {									/* data buffer */
		grp_deque_ent(ptFsCtl, ptDBuf, ptBuf, ptList); /* deque it */
		grp_enque_head(ptFsCtl, ptDBuf, ptBuf, ptList);/* enque data top */
	}
	if (ptBuf->pptHashTop) {					/* in hash list */
		grp_deque_sent(ptBuf->pptHashTop, ptBuf, ptHash);	/* deque */
	}
	ptBuf->pptHashTop = pptBhash;				/* set hash top */
	grp_enque_shead(pptBhash, ptBuf, ptHash);	/* enque top  */
	ptBuf->iRefCnt++;							/* increment reference */
	ptBuf->iSize = iSize;						/* set size */

	/****************************************************/
	/* set buffer information to return					*/
	/****************************************************/
	ptBio->ptBuf = ptBuf;						/* buffer pointer */
	ptBio->pucData = ptBuf->pucData;			/* set data pointer */
	ptBio->uiSize = iSize;						/* data size */
	ptBio->uiBlk = uiBlk;						/* FS block number */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_invalidate_buf										*/
/*																			*/
/* DESCRIPTION:	Invalidate buffer											*/
/* INPUT:		ptBuf:				FS buffer 								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_invalidate_buf(
	grp_fs_buf_t		*ptBuf)				/* [IN]  FS buffer to invalidate */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */

	/****************************************************/
	/* reset device information							*/
	/****************************************************/
	grp_fs_btrace(ptBuf, GRP_FS_BT_INV);	/* trace(invalidate) */
	_grp_fs_wakeup_buf_wait(ptBuf);			/* wakeup task waiting for buffer */
	ptBuf->iDev = -1;						/* reset device informaition */
	ptBuf->usStatus &= (GRP_FS_BSTAT_FBUF|GRP_FS_BSTAT_DBUF);/* clean flags */

	/****************************************************/
	/* remove from hash list							*/
	/****************************************************/
	if (ptBuf->pptHashTop) {				/* in hash list */
		grp_deque_sent(ptBuf->pptHashTop, ptBuf, ptHash);/* deque */
		ptBuf->pptHashTop = NULL;			/* not in hash list */
	}

	/****************************************************/
	/* rechain to tail of LRU list						*/
	/****************************************************/
	if (ptBuf->usStatus & GRP_FS_BSTAT_FBUF) {/* file buffer */
		grp_deque_ent(ptFsCtl, ptFBuf, ptBuf, ptList);/* deque it */
		grp_enque_tail(ptFsCtl, ptFBuf, ptBuf, ptList);/* enque file tail */
	} else {								/* data buffer */
		grp_deque_ent(ptFsCtl, ptDBuf, ptBuf, ptList);	/* deque it */
		grp_enque_tail(ptFsCtl, ptDBuf, ptBuf, ptList);/* enque data tail */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unref_buf											*/
/*																			*/
/* DESCRIPTION:	Unreference buffer											*/
/* INPUT:		ptBio:				buffer I/O information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_unref_buf(
	grp_fs_bio_t	*ptBio)					/* [IN]  buffer I/O information */
{
	grp_fs_buf_t	*ptBuf;					/* FS buffer */
	grp_ushort_t	usStat;					/* buffer status */

	if ((ptBuf = ptBio->ptBuf) == NULL)		/* no buffer */
		return;								/* just return */
	grp_fs_btrace(ptBuf, GRP_FS_BT_URB);	/* trace(unref_buf) */
	usStat = (grp_ushort_t)ptBuf->usStatus; /* status */
	if ((usStat & GRP_FS_BSTAT_DIRTY)		/* dirty */
		&& (usStat & (GRP_FS_BSTAT_SYNC|GRP_FS_BSTAT_TSYNC)) /* sync mode */
		&& (usStat & GRP_FS_BSTAT_WFAIL) == 0 /* no write failed */
		&& ptBuf->iRefCnt == 1) {			/* last reference */
		if (ptBuf->iDev >= 0)				/* valid device */
			grp_fs_write_buf(ptBio);		/* write buffer */
		else
			ptBuf->usStatus &= ~(GRP_FS_BSTAT_DIRTY|GRP_FS_BSTAT_TSYNC);
											/* clear dirty flag */
	}
	ptBio->ptBuf = NULL;					/* clear buffer pointer */
	ptBio->uiSize = 0;						/* no data */
	if (--(ptBuf->iRefCnt) > 0)				/* still has reference */
		return;								/* just return */
	if (ptBuf->iRefCnt < 0)
		grp_fs_printf("GRP_FS: negative buf ref(dev:0x%x blk:0x%lx ref:%d)\n",
					ptBuf->iDev, (unsigned long)ptBuf->uiBlk, ptBuf->iRefCnt);
	ptBuf->iRefCnt = 0;						/* set to 0 for safety */
	_grp_fs_wakeup_buf_wait(ptBuf);			/* wakeup task waiting for buffer */
	if (ptBuf->usStatus & GRP_FS_BSTAT_MOD) { /* under modification */
		ptBio->ptBuf = ptBuf;				/* set buffer */
		grp_fs_unblock_buf_mod(ptBio, 0);	/* end modification */
		ptBio->ptBuf = NULL;				/* clear buffer pointer */
	}
	if (ptBuf->iDev < 0)					/* invalid data */
		_grp_fs_invalidate_buf(ptBuf);		/* invalidate buffer */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_buf_fill_end											*/
/*																			*/
/* DESCRIPTION:	End buffer fill												*/
/* INPUT:		ptBio					buffer I/O information				*/
/*				iInvalidate				need to invalidate					*/
/* OUTPUT:		ptBio->puBuf->usStatus	cleared GRP_FS_BSTAT_FILL flag		*/
/*				ptBio->ptBuf->iDev		-1 in case of invalidating buffer	*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_buf_fill_end(
	grp_fs_bio_t	*ptBio,				/* [IN]  buffer I/O information */
	int				iInvalidate)		/* [IN]  invalidate buffer */
{
	grp_fs_buf_t	*ptBuf = ptBio->ptBuf;		/* buffer pointer */

	if (ptBuf == NULL 							/* no buffer */
		|| (ptBuf->usStatus & GRP_FS_BSTAT_FILL) == 0) /* not fill mode */
		return;									/* do nothing */
	ptBuf->usStatus &= ~GRP_FS_BSTAT_FILL;		/* clear fill flag */
	grp_fs_btrace(ptBuf, GRP_FS_BT_ENF);		/* trace(end_fill) */
	if (iInvalidate)							/* need to invalidate */
		ptBuf->iDev = -1;						/* no assoc data */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_block_buf_mod										*/
/*																			*/
/* DESCRIPTION:	Block buffer modification or write							*/
/* INPUT:		ptBio				buffer I/O information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_block_buf_mod(
	grp_fs_bio_t	*ptBio)				/* [IN]  buffer I/O information */
{
	grp_fs_buf_t	*ptBuf = ptBio->ptBuf;		/* buffer pointer */

	while (ptBuf->usStatus & GRP_FS_BSTAT_MOD) { /* under modification */
		ptBuf->usStatus |= GRP_FS_BSTAT_WAITMOD; /* wait for modification */
		grp_fs_btrace(ptBuf, GRP_FS_BT_WTM);	/* trace(block_mod) */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(ptBuf->ptFs, (void *)ptBuf); /* wait for end of mod */
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)ptBuf);		/* wait for end of mod */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	}
	ptBuf->usStatus |= GRP_FS_BSTAT_MOD;		/* under modification */
	grp_fs_btrace(ptBuf, GRP_FS_BT_BMD);		/* trace(block_mod) */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unblock_buf_mod										*/
/*																			*/
/* DESCRIPTION:	Unblock buffer modification or write						*/
/* INPUT:		ptBio				buffer I/O information					*/
/*				usModStat			modification status						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_unblock_buf_mod(
	grp_fs_bio_t	*ptBio,				/* [IN]  buffer I/O information */
	grp_ushort_t	usModStat)			/* [IN]  modification status */
{
	grp_fs_buf_t	*ptBuf = ptBio->ptBuf;		/* buffer pointer */

	ptBuf->usStatus |= (usModStat & (GRP_FS_BSTAT_DIRTY|GRP_FS_BSTAT_TSYNC));
												/* set modification status */
	ptBuf->usStatus &= ~GRP_FS_BSTAT_MOD;		/* reset under modify bit */
	grp_fs_btrace(ptBuf, GRP_FS_BT_UBM);		/* trace(unblock_mod) */
	if (ptBuf->usStatus & GRP_FS_BSTAT_WAITMOD) {/* waiting for modification */
		ptBuf->usStatus &= ~GRP_FS_BSTAT_WAITMOD;/* reset wait mod flag */
		grp_fs_btrace(ptBuf, GRP_FS_BT_WKM);	/* trace(wakeup_mod) */
		grp_fs_task_wakeup((void *)ptBuf);		/* wakeup waiting task */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_block_fs_mod											*/
/*																			*/
/* DESCRIPTION:	Block file system modification or write						*/
/* INPUT:		ptFs:				file system information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_block_fs_mod(
	grp_fs_info_t	*ptFs)				/* [IN]  file system information */
{
	while (ptFs->usStatus & GRP_FS_STAT_MOD) {		/* under modification */
		ptFs->usStatus |= GRP_FS_STAT_WAITMOD;		/* wait for modification */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(ptFs, (void *)ptFs);		/* wait for end of mod */
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)ptFs);				/* wait for end of mod */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	}
	ptFs->usStatus |= GRP_FS_STAT_MOD;				/* under modification */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unblock_fs_mod										*/
/*																			*/
/* DESCRIPTION:	Unblock file system modification or write					*/
/* INPUT:		ptFs:				file system information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_unblock_fs_mod(
	grp_fs_info_t	*ptFs)				/* [IN]  file system information */
{
	ptFs->usStatus &= ~GRP_FS_STAT_MOD;			/* reset under modify bit */
	if (ptFs->usStatus & GRP_FS_STAT_WAITMOD) {	/* waiting for modification */
		ptFs->usStatus &= ~GRP_FS_STAT_WAITMOD;	/* reset wait mod flag */
		grp_fs_task_wakeup((void *)ptFs);		/* wakeup waiting task */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_block_file_op										*/
/*																			*/
/* DESCRIPTION:	Block file operation										*/
/* INPUT:		ptFile:				file information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_block_file_op(
	grp_fs_file_t	*ptFile)				/* [IN]  file information */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* file system information */
	
again:
	while (ptFile->usStatus & GRP_FS_FSTAT_BUSY) {/* under operation */
		ptFile->usStatus |= GRP_FS_FSTAT_WAIT;	/* wait for operation */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(ptFs, (void *)ptFile);	/* wait for end of op */
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)ptFile);		/* wait for end of op */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	}
	if ((ptFs->usStatus & GRP_FS_STAT_BUSY_FID) /* exist busy file id */
		&& ptFs->uiFsBusyFid == ptFile->uiFid) {/* match busy file id */
		ptFs->usStatus |= GRP_FS_STAT_WAIT_BUSY_FID; /* wait for operation */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(ptFs, (void *)&ptFs->uiFsBusyFid);
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)&ptFs->uiFsBusyFid);/* wait for end of op */
#endif /* GRP_FS_ASYNC_UNMOUNT */
		goto again;								/* check again */
	}
	ptFile->usStatus |= GRP_FS_FSTAT_BUSY;		/* under operation */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unblock_file_op										*/
/*																			*/
/* DESCRIPTION:	Unblock file operation										*/
/* INPUT:		ptFile:				file information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_unblock_file_op(
	grp_fs_file_t	*ptFile)				/* [IN]  file information */
{
	ptFile->usStatus &= ~GRP_FS_FSTAT_BUSY;		/* reset operation flag */
	if (ptFile->usStatus & GRP_FS_FSTAT_WAIT) {	/* waiting for operation */
		ptFile->usStatus &= ~GRP_FS_FSTAT_WAIT;	/* reset wait flag */
		grp_fs_task_wakeup((void *)ptFile);		/* wakeup waiting task */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_block_file_op_by_id									*/
/*																			*/
/* DESCRIPTION:	Block file operation by id									*/
/* INPUT:		ptFs:				file system information					*/
/*				uiFid:				file id to block						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:				no opened file; blocked by id only		*/
/*									(unblock should be done with			*/
/*									 grp_fs_unblock_file_op_by_id)			*/
/*				others:				opened file information blocked			*/
/*									(unblock should be done with			*/
/*									 grp_fs_close_file)						*/
/*																			*/
/****************************************************************************/
grp_fs_file_t *
grp_fs_block_file_op_by_id(
	grp_fs_info_t	*ptFs,				/* [IN]  file system information */
	grp_uint32_t	uiFid)				/* [IN]  file id to block */
{
	grp_fs_file_t	*ptFile;			/* file information */

again:
	if (grp_fs_lookup_file_ctl(ptFs, uiFid, 0, &ptFile) == 0) /* found opened */
		return(ptFile);						/* returne opened one */
	if (ptFs->usStatus & GRP_FS_STAT_BUSY_FID) { /* exist busy file id */
		ptFs->usStatus |= GRP_FS_STAT_WAIT_BUSY_FID; /* wait for operation */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(ptFs, (void *)&ptFs->uiFsBusyFid);
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)&ptFs->uiFsBusyFid);/* wait for end of op */
#endif /* GRP_FS_ASYNC_UNMOUNT */
		goto again;							/* try again */
	}
	ptFs->uiFsBusyFid = uiFid;				/* set busy file id */
	ptFs->usStatus |= GRP_FS_STAT_BUSY_FID;	/* under operation */
	return(NULL);							/* no opened one */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unblock_file_op_by_id								*/
/*																			*/
/* DESCRIPTION:	Unblock file operation by id								*/
/* INPUT:		ptFs:				file system information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_unblock_file_op_by_id(
	grp_fs_info_t	*ptFs)				/* [IN]  file system information */
{
	ptFs->usStatus &= ~GRP_FS_STAT_BUSY_FID;		/* reset operation flag */
	if (ptFs->usStatus & GRP_FS_STAT_WAIT_BUSY_FID) { /* waiting for op */
		ptFs->usStatus &= ~GRP_FS_STAT_WAIT_BUSY_FID; /* reset wait flag */
		grp_fs_task_wakeup((void *)&ptFs->uiFsBusyFid);/* wakeup waiting */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_block_file_write										*/
/*																			*/
/* DESCRIPTION:	Block file write											*/
/* INPUT:		ptFile:				file information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_block_file_write(
	grp_fs_file_t	*ptFile)				/* [IN]  file information */
{
	while (ptFile->usStatus & GRP_FS_FSTAT_WR_LOCK) {/* under write */
		ptFile->usStatus |= GRP_FS_FSTAT_WR_WAIT;	/* wait for write */
		grp_fs_unblock_file_op(ptFile);				/* unblock file operation */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(ptFile->ptFs, (void *)&ptFile->usStatus);
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)&ptFile->usStatus);/* wait for end of write */
#endif /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_block_file_op(ptFile);				/* block file operation */
	}
	ptFile->usStatus |= GRP_FS_FSTAT_WR_LOCK;		/* under write */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unblock_file_write									*/
/*																			*/
/* DESCRIPTION:	Unblock file write											*/
/* INPUT:		ptFile:				file information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_unblock_file_write(
	grp_fs_file_t	*ptFile)				/* [IN]  file information */
{
	ptFile->usStatus &= ~GRP_FS_FSTAT_WR_LOCK;		/* reset write flag */
	if (ptFile->usStatus & GRP_FS_FSTAT_WR_WAIT) {	/* waiting for write */
		ptFile->usStatus &= ~GRP_FS_FSTAT_WR_WAIT;	/* reset wait flag */
		grp_fs_task_wakeup((void *)&ptFile->usStatus);/* wakeup waiting task */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_close_file											*/
/*																			*/
/* DESCRIPTION:	close a file												*/
/* INPUT:		ptFile:		file information to close						*/
/*				iMode:		close mode										*/
/*								GRP_FS_FILE_INVALID: invalidate cache		*/
/*								GRP_FS_FILE_UNBLOCK: unblock operation		*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_close_file(
	grp_fs_file_t	*ptFile,			/* [IN] file info to close */
	int				iMode)				/* [IN] close mode */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;/* FS control data */
	grp_fs_info_t	*ptFs;				/* file system information */

	if (--(ptFile->iRefCnt) > 0) {		/* still has reference */
		if (iMode & GRP_FS_FILE_UNBLOCK) /* need to unblock */
			grp_fs_unblock_file_op(ptFile);/* unblock file operation */
		return;							/* return */
	}
	ptFs = ptFile->ptFs;				/* file system information */
	if (ptFile->iRefCnt < 0)
		grp_fs_printf("GRP_FS: negative file ref dev:0x%x fid:0x%lx st:0x%x ref:%d\n",
						ptFile->iDev, (unsigned long)ptFile->uiFid,
						ptFile->usStatus, ptFile->iRefCnt);
	ptFile->iRefCnt = 0;				/* set 0 for safety */
	if ((iMode & GRP_FS_FILE_UNBLOCK) == 0) { /* file operation is not blocked */
		if (ptFile->usStatus & GRP_FS_FSTAT_BUSY) {	/* still blocked */
			grp_fs_printf("GRP_FS: file still busy dev:0x%x fid:0x%lx st:0x%x\n",
						ptFile->iDev, (unsigned long)ptFile->uiFid,
						ptFile->usStatus);
		} else {						/* not blocked */
			grp_fs_block_file_op(ptFile); /* block file operation */
		}
	} else if ((ptFile->usStatus & GRP_FS_FSTAT_BUSY) == 0) { /* not blocked */
		grp_fs_printf("GRP_FS: file not blocked dev:0x%x fid:0x%lx st:0x%x\n",
						ptFile->iDev, (unsigned long)ptFile->uiFid,
						ptFile->usStatus);
		grp_fs_block_file_op(ptFile);	/* block file operation */
	}
	ptFs->ptFsOp->pfnClose(ptFile, (iMode & GRP_FS_FILE_INVALID)?
							GRP_FS_CLOSE_RELEASE: GRP_FS_CLOSE_CACHE);
										/* close file */
	if (iMode & GRP_FS_FILE_INVALID) {	/* need to invalidate */
		ptFile->iDev = -1;				/* invalidate device number */
		if (ptFile->pptHashTop) {		/* in hash */
			grp_deque_sent(ptFile->pptHashTop, ptFile, ptHash);	/* deque */
			ptFile->pptHashTop = NULL;	/* no in hash */
		}
		grp_deque_ent(ptFsCtl, ptFile, ptFile, ptList);	/* deque LRU list */
		grp_enque_tail(ptFsCtl, ptFile, ptFile, ptList); /* enque tail of LRU */
	}
	grp_fs_unblock_file_op(ptFile);		/* unblock file operation */
	if (ptFs->usStatus & (GRP_FS_STAT_SYNC_FL_CLOSE|GRP_FS_STAT_SYNC_ALL))
										/* sync on each close or all */
		(void)_grp_fs_sync_fs(ptFs, 0);	/* sync data */
	if (--(ptFs->iFsOpen) <= 0)	{		/* final close on the file system */
		if (ptFs->iFsOpen < 0)			/* negative open value */
			grp_fs_printf("GRP_FS: negative FS open dev:0x%x open:%ld\n",
						ptFs->iDev, (long)ptFs->iFsOpen);
		ptFs->iFsOpen = 0;				/* set 0 for safety */
		if (ptFs->usStatus & GRP_FS_STAT_SYNC_FS_CLOSE)/* sync on final close */
			(void)_grp_fs_sync_fs(ptFs, 0);	/* sync data */
	}
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_sync_file											*/
/*																			*/
/* DESCRIPTION:	Update file information to device							*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		I/O error								*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_sync_file(void)
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_file_t	*ptFile;				/* file info pointer */
	int				iRet = 0;				/* return value */
	int				iClose;					/* return value of close */

	for (ptFile = ptFsCtl->ptFileFwd; ptFile; ptFile = ptFile->ptListFwd) {
		if (ptFile->iDev < 0					/* no device */
			|| (ptFile->ptFs->usStatus & GRP_FS_STAT_RONLY) /* read only */
			|| (ptFile->usStatus & GRP_FS_FSTAT_UPD_BITS) == 0) /* need update */
			continue;							/* search next */
		grp_fs_block_file_op(ptFile);			/* lock file */
		if (ptFile->usStatus & GRP_FS_FSTAT_UPD_BITS) {/* still need to update */
			iClose = ptFile->ptFs->ptFsOp->pfnClose(ptFile, GRP_FS_CLOSE_UPDATE);
												/* update information */
			if (iRet == 0)						/* no error upto now */
				iRet = iClose;					/* set the return from close */
		}
		grp_fs_unblock_file_op(ptFile);			/* release lock */
	}
	return(iRet);								/* return update status */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_invalidate_file										*/
/*																			*/
/* DESCRIPTION:	Invalidate file information for the device					*/
/* INPUT:		iDev:				file information to close				*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BUSY:	still has reference						*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_invalidate_file(
	int				iDev)				/* [IN] device number */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_file_t	*ptFile;				/* file info pointer */
	grp_fs_file_t	*ptFileNext;			/* next file info pointer */
	grp_fs_file_t	*ptFileTail;			/* tail file */

	ptFileTail = ptFsCtl->ptFileBwd;			/* tail of file info */
	for (ptFile = ptFsCtl->ptFileFwd; ptFile; ptFile = ptFileNext) {
		/****************************************************/
		/* lookup matching cache							*/
		/****************************************************/
		ptFileNext = ptFile->ptListFwd;			/* next file */
		if (ptFile->iDev == iDev) {				/* match */
			if (ptFile->iRefCnt > 0)			/* still has reference */
				return(GRP_FS_ERR_BUSY);		/* return busy */

			/****************************************************/
			/* completely close it								*/
			/* Since file link may be changed while close,		*/
			/* get next link again after close.					*/
			/****************************************************/
			grp_fs_block_file_op(ptFile);		/* block file operation */
			ptFile->ptFs->ptFsOp->pfnClose(ptFile, GRP_FS_CLOSE_RELEASE);
												/* completely close */
			grp_fs_unblock_file_op(ptFile);		/* unblock file operation */
			ptFileNext = ptFile->ptListFwd;		/* get next file again */
			ptFile->iDev = -1;					/* invalidate */
			if (ptFile->pptHashTop) {			/* in hash */
				grp_deque_sent(ptFile->pptHashTop, ptFile, ptHash);/* deque */
			}
			ptFile->pptHashTop = NULL;			/* clear hash */
			grp_deque_ent(ptFsCtl, ptFile, ptFile, ptList);	/* deque LRU */
			grp_enque_tail(ptFsCtl, ptFile, ptFile, ptList); /* enque LRU tail */
		}
		if (ptFile == ptFileTail)				/* original tail file */
			break;								/* end processing */
	}
	return(0);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_lookup_file_ctl										*/
/*																			*/
/* DESCRIPTION:	Lookup file control information								*/
/*				Note: file control information is returned with reference	*/
/*					  count incremented and GRP_FS_FSTAT_BUSY flag set.		*/
/* INPUT:		ptFs:			file system information						*/
/*				uiFid:			file ID										*/
/*				iAlloc:			allocate if not found						*/
/* OUTPUT:		pptFile:		file control information found or allocated	*/
/*																			*/
/* RESULT:		-1:				cache not found								*/
/*				0:				cache found									*/
/*																			*/
/****************************************************************************/
int
grp_fs_lookup_file_ctl(
	grp_fs_info_t	*ptFs,				/* [IN]  file system information */
	grp_uint32_t	uiFid,				/* [IN]  file ID */
	int				iAlloc,				/* [IN]  allocate if not found */
	grp_fs_file_t	**pptFile)			/* [OUT] file control info */
{
	int				iDev = ptFs->iDev;	/* device number */
	grp_fs_file_t	*ptFile;			/* file info */
	grp_fs_file_t	**pptFHash;			/* file hash */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;/* FS control data */
	int				iRet;				/* return value */

	/****************************************************/
	/* lookup active cache								*/
	/****************************************************/
	pptFHash = GRP_FS_FILE_HASH(ptFsCtl, uiFid);
	iRet = 0;									/* init return value */
again:
	for (ptFile = *pptFHash; ptFile; ptFile = ptFile->ptHashFwd) {
		if (ptFile->iDev == iDev && ptFile->uiFid == uiFid) { /* found */
			if (ptFile->usStatus & GRP_FS_FSTAT_BUSY) {/* under operation */
				ptFile->usStatus |= GRP_FS_FSTAT_WAIT;	/* wait for operation */
#ifdef GRP_FS_ASYNC_UNMOUNT
				grp_fs_task_wait(ptFs, (void *)ptFile);	/* wait for end of op */
#else  /* GRP_FS_ASYNC_UNMOUNT */
				grp_fs_task_wait((void *)ptFile);		/* wait for end of op */
#endif /* GRP_FS_ASYNC_UNMOUNT */
				goto again;								/* lookup again */
			}
			if ((ptFs->usStatus & GRP_FS_STAT_BUSY_FID)	/* exist busy file */
				&& ptFs->uiFsBusyFid == ptFile->uiFid) {/* match it */
				ptFs->usStatus |= GRP_FS_STAT_WAIT_BUSY_FID;/* wait for op */
#ifdef GRP_FS_ASYNC_UNMOUNT
				grp_fs_task_wait(ptFs, (void *)&ptFs->uiFsBusyFid);
#else  /* GRP_FS_ASYNC_UNMOUNT */
				grp_fs_task_wait((void *)&ptFs->uiFsBusyFid);/* wait for end */
#endif /* GRP_FS_ASYNC_UNMOUNT */
				goto again;								/* lookup again */
			}
			goto found;							/* return it */
		}
	}
	iRet = -1;									/* set iRet value not found */
	if (iAlloc == 0) {							/* not allocate */
		*pptFile = NULL;						/* clear pointer */
		return(iRet);							/* return not found */
	}

	/****************************************************/
	/* get from free list								*/
	/****************************************************/
	for (ptFile = ptFsCtl->ptFileBwd; ptFile; ptFile = ptFile->ptListBwd) {
		if (ptFile->iRefCnt <= 0
			&& (ptFile->usStatus & GRP_FS_FSTAT_BUSY) == 0)	/* found free */
			break;								/* break */
	}
	if (ptFile == NULL) {						/* not found */
		*pptFile = NULL;						/* clear pointer */
		return(iRet);							/* return not found */
	}

	/****************************************************/
	/* If still assoicated with some file system, clean	*/
	/* it.  To avoid race condition for force free		*/
	/* process waiting for the file while close, set	*/
	/* reference count non zero, before call close.		*/
	/****************************************************/
	if (ptFile->iDev >= 0) {					/* still associated */
		ptFile->iRefCnt = 1;					/* temporary get reference */
		grp_fs_block_file_op(ptFile);			/* block file operation */
		ptFile->ptFs->ptFsOp->pfnClose(ptFile, GRP_FS_CLOSE_RELEASE);
												/* completely close */
		grp_fs_unblock_file_op(ptFile);			/* block file operation */
	}

	/****************************************************/
	/* set information and update hash and LRU list		*/
	/****************************************************/
	ptFile->usStatus = 0;						/* set busy flag */
	ptFile->iDev = iDev;						/* set device number */
	ptFile->uiFid = uiFid;						/* set file number */
	ptFile->iRefCnt = 0;						/* init reference count */
	ptFile->uiMapCnt = 0;						/* no mapping */
	ptFile->uiMapFBlk = 0;						/* init map start block */
	ptFile->pvFileInfo = NULL;					/* no FS depend information */
	ptFile->puiMap = NULL;						/* no mapping table */
	ptFile->ptFs = ptFs;						/* set file system info */

found:
	ptFile->usStatus |= GRP_FS_FSTAT_BUSY;		/* set busy flag */
	if (ptFile->pptHashTop) {
		grp_deque_sent(ptFile->pptHashTop, ptFile, ptHash);	/* deque hash */
	}
	ptFile->pptHashTop = pptFHash;				/* set hash top */
	grp_enque_shead(pptFHash, ptFile, ptHash);	/* enque hash head */
	grp_deque_ent(ptFsCtl, ptFile, ptFile, ptList);	/* deque LRU */
	grp_enque_head(ptFsCtl, ptFile, ptFile, ptList); /* enque LRU head */
	if (ptFile->iRefCnt++ == 0)					/* increment reference */
		ptFs->iFsOpen++;						/* increment open count */
	*pptFile = ptFile;							/* set pointer */
	return(iRet);								/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_change_fid											*/
/*																			*/
/* DESCRIPTION:	Change file id												*/
/* INPUT:		ptFile:			file control information					*/
/*				uiFid:			file ID										*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_change_fid(
	grp_fs_file_t	*ptFile,			/* [IN]  file control info */
	grp_uint32_t	uiFid)				/* [IN]  file ID */
{
	grp_fs_file_t	**pptFHash;					/* file hash */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	
	pptFHash = GRP_FS_FILE_HASH(ptFsCtl, uiFid);/* hash top */
	if (ptFile->pptHashTop) {					/* previous hash exists */
		grp_deque_sent(ptFile->pptHashTop, ptFile, ptHash);	/* deque hash */
	}
	ptFile->uiFid = uiFid;						/* set new file id */
	ptFile->pptHashTop = pptFHash;				/* set hash top */
	grp_enque_shead(pptFHash, ptFile, ptHash);	/* enque hash head */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_check_io_status										*/
/*																			*/
/* DESCRIPTION:	Check I/O status											*/
/* INPUT:		ptFs:				file system information					*/
/*				uiDevBlk:			device block number						*/
/*				iCnt:				device I/O block count					*/
/*				iMode:				I/O mode								*/
/*										GRP_FS_IO_READ:  read operation		*/
/*										GRP_FS_IO_WRITE: write operation	*/
/*										GRP_FS_IO_REQ:   I/O request check	*/
/*										GRP_FS_IO_OP_ERR: I/O op error		*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		not ready or abort						*/
/*				0					ready or retry							*/
/*																			*/
/****************************************************************************/
int
grp_fs_check_io_status(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_uint32_t	uiDevBlk,			/* [IN]  device block number */
	grp_isize_t		iCnt,				/* [IN]  device I/O block count */
	int				iMode)				/* [IN]  I/O mode */
{
	int				iRet;				/* return value */

	if ((ptFs->usStatus & GRP_FS_STAT_DEV_INV) == 0	/* valid status */
		&& (iMode & GRP_FS_IO_OP_ERR) == 0)	 /* not I/O operation error */
		return(0);						/* return ready */
	iRet = GRP_FS_ERR_IO;				/* not ready or abort */
	if (grp_fs_inform_io_err) {			/* inform function exists */
		/****************************************************/
		/* inform I/O error and retry if possible			*/
		/****************************************************/
		while ((iRet = grp_fs_inform_io_err(ptFs->iDev, uiDevBlk,
													iCnt, iMode)) == 0
			&& (ptFs->usStatus & GRP_FS_STAT_DEV_INV));	/* invalidated */
	}
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_read_buf												*/
/*																			*/
/* DESCRIPTION:	Read a data block from device to buffer						*/
/*				Note: buffer is returned with reference count incremented	*/
/* INPUT:		ptFs:				file system information					*/
/*				uiFsBlk:			block number							*/
/*				iBufKind:			GF_FS_BUF_FILE: file buffer				*/
/*									GRP_FS_BUF_DATA: data buffer			*/
/*				iSize:				size of data to read					*/
/* OUTPUT:		ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_BAD_DEV	bad device								*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0 or positive:		data count								*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_read_buf(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_uint32_t	uiBlk,				/* [IN]  block number to read */
	int				iBufKind,			/* [IN]  buffer kind */
	grp_int32_t		iSize,				/* [IN]  size of data */
	grp_fs_bio_t	*ptBio)				/* [OUT] buffer I/O information */
{
	grp_fs_buf_t	*ptBuf;				/* buffer pointer */
	grp_fs_ctl_t	*ptFsCtl;			/* FS control infomration */
	int				iMajor;				/* major device number */
	grp_fs_dev_op_t	*ptDevOp;			/* device operation */
	grp_fs_dev_tbl_t *ptDev;			/* device table entry */
	grp_int32_t		iCnt;				/* count to read */
	grp_int32_t		iRead;				/* read count */
	int				iShift;				/* shift count */
	grp_uint32_t	uiBlkOff;			/* block offset */
	grp_uint32_t	uiDevBlk;			/* device block number */
	int				iRet;				/* return vlaue */

	/****************************************************/
	/* check cache data									*/
	/****************************************************/
	iBufKind |= GRP_FS_BUF_ALLOC;				/* set allocate bit */
	iRet = grp_fs_lookup_buf(ptFs, uiBlk, iBufKind, iSize, ptBio);
												/* lookup buffer */
	if (iRet == 0)								/* found cache */
		return((grp_int32_t)ptBio->uiSize);		/* return size */
	if (iRet != GRP_FS_ERR_NOT_FOUND)			/* other than not found */
		return((grp_int32_t)iRet);				/* return the error */
	ptBuf = ptBio->ptBuf;						/* get FS buffer */

	/****************************************************/
	/* check device number and block number				*/
	/****************************************************/
	iMajor = GRP_FS_DEV_MAJOR(ptBuf->iDev);		/* major device number */
	ptDev = &grp_fs_dev_tbl[iMajor];			/* device table entry */
	if ((iMajor < 0 || iMajor >= grp_fs_dev_tbl_cnt) /* bad devince number */
		|| (ptDevOp = ptDev->ptOp) == NULL) {	/* no operation */
		iRet = GRP_FS_ERR_BAD_DEV;				/* set error number */
		goto err_ret;							/* error return */
	}
	if (iBufKind & GRP_FS_BUF_FILE) {			/* file type buffer */
		iShift = ptFs->ucFsFBlkShift;			/* block shift */
		uiBlkOff = ptFs->uiFsFBlkOff;			/* block offset */
	} else {									/* data type buffer */
		iShift = ptFs->ucFsDBlkShift;			/* block shift */
		uiBlkOff = ptFs->uiFsDBlkOff;			/* block offset */
	}
	iShift -= ptFs->ucDevBlkShift;				/* shift count */
	uiDevBlk = (ptBuf->uiBlk << iShift)
			+ (uiBlkOff >> ptFs->ucDevBlkShift);/* device block number */
	iCnt = ((iSize 
			+ ((grp_uint32_t)1<<ptFs->ucDevBlkShift)-1) >> ptFs->ucDevBlkShift);
	if (ptFs->uiDevSize && ptFs->uiDevSize <= uiDevBlk) { /* too big */
		iRet = 0;								/* no data */
		goto err_ret;							/* return */
	}
	uiDevBlk += ptFs->uiDevOff;					/* add offset */
	if (grp_fs_check_io_status(ptFs, uiDevBlk, iCnt, GRP_FS_IO_READ) != 0) {
		iRet = GRP_FS_ERR_IO;					/* I/O error */
		goto err_ret;							/* return error */
	}

	/****************************************************/
	/* read data into buffer							*/
	/****************************************************/
again:
	ptFsCtl = grp_fs_ctl;						/* FS control infomration */
	grp_fs_btrace(ptBuf, GRP_FS_BT_STR);		/* trace(start_read) */
#ifdef GRP_FS_ASYNC_UNMOUNT
	ptFs->iFsRef++;								/* increment reference count */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt++;						/* increment wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	iRead = ptDevOp->pfnRead(ptFs->iDevHandle, ptBuf->iDev, uiDevBlk,
						ptBuf->pucData, iCnt);	/* read data */
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
#ifdef GRP_FS_ASYNC_UNMOUNT
	ptFs->iFsRef--;								/* decrement reference count */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt--;						/* decrement wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	if (iRead != iCnt) {						/* I/O error */
		if (grp_fs_check_io_status(ptFs, uiDevBlk, iCnt,
									GRP_FS_IO_READ|GRP_FS_IO_OP_ERR) == 0) {
			grp_fs_btrace(ptBuf, GRP_FS_BT_ERR);/* trace(error_read) */
			goto again;							/* try again */
		}
		if (iRead >= 0)							/* not error number */
			iRet = GRP_FS_ERR_IO;				/* return I/O error */
		else									/* error number */
			iRet = (int)iRead;					/* return it */
		goto err_ret;							/* error return */
	}
	ptBuf->usStatus &= ~GRP_FS_BSTAT_FILL;		/* reset fill flag */
	grp_fs_btrace(ptBuf, GRP_FS_BT_ENR);		/* trace(end_read) */
	return((grp_int32_t)ptBio->uiSize);			/* return size */

err_ret:
	ptBuf->usStatus &= ~GRP_FS_BSTAT_FILL;		/* reset fill flag */
	grp_fs_btrace(ptBuf, GRP_FS_BT_ERR);		/* trace(error_read) */
	ptBuf->iDev = -1;							/* no associated data */
	grp_fs_unref_buf(ptBio);					/* unreference buffer */
	ptBio->uiSize = 0;							/* clear data size */
	return((grp_int32_t)iRet);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_write_buf											*/
/*																			*/
/* DESCRIPTION:	Write a data block from buffer to device					*/
/*				Note: ptBio->ptBuf->usStatus & GRP_FS_BSTAT_DIRTY should	*/
/*					  be set before call.  If not set, do nothing.			*/
/* INPUT:		ptBio				buffer I/O information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_BAD_DEV	bad device								*/
/*				0 or positive:		data count								*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_write_buf(
	grp_fs_bio_t	*ptBio)				/* [IN]  buffer I/O information */
{
	grp_fs_buf_t	*ptBuf;				/* buffer pointer */
	grp_fs_ctl_t	*ptFsCtl;			/* FS control infomration */
	int				iMajor;				/* major device number */
	grp_fs_dev_op_t	*ptDevOp;			/* device operation */
	grp_fs_dev_tbl_t *ptDev;			/* device table entry */
	grp_int32_t		iCnt;				/* count to write */
	grp_int32_t		iWrite = GRP_FS_ERR_IO;	/* written count */
	int				iShift;				/* shift count */
	grp_uint32_t	uiBlkOff;			/* block offset */
	grp_uint32_t	uiDevBlk;			/* block number */
	grp_fs_info_t	*ptFs;				/* FS information */

	/****************************************************/
	/* check device number								*/
	/****************************************************/
	ptBuf = ptBio->ptBuf;						/* get FS buffer */
	iMajor = GRP_FS_DEV_MAJOR(ptBuf->iDev);		/* major device number */
	ptDev = &grp_fs_dev_tbl[iMajor];			/* device table entry */
	if ((iMajor < 0 || iMajor >= grp_fs_dev_tbl_cnt) /* bad device number */
		|| (ptDevOp = ptDev->ptOp) == NULL)		/* no operation */
		return(GRP_FS_ERR_BAD_DEV);				/* bad device number */
	ptFs = ptBuf->ptFs;							/* FS information */
	if (ptBuf->usStatus & GRP_FS_BSTAT_FBUF) {	/* file type buffer */
		iShift = ptFs->ucFsFBlkShift;			/* block shift */
		uiBlkOff = ptFs->uiFsFBlkOff;			/* block offset */
	} else {									/* data type buffer */
		iShift = ptFs->ucFsDBlkShift;			/* block shift */
		uiBlkOff = ptFs->uiFsDBlkOff;			/* block offset */
	}
	iShift -= ptFs->ucDevBlkShift;				/* shift count */
	uiDevBlk = (ptBuf->uiBlk << iShift)
			+ (uiBlkOff >> ptFs->ucDevBlkShift);/* device block number */
	iCnt = ((ptBio->uiSize + ((grp_uint32_t)1 << ptFs->ucDevBlkShift) - 1)
					>> ptFs->ucDevBlkShift);	/* count to write */
	if (ptFs->uiDevSize && ptFs->uiDevSize <= uiDevBlk) { /* too big */
		uiDevBlk += ptFs->uiDevOff;				/* add offset */
		goto write_error;						/* return */
	}
	uiDevBlk += ptFs->uiDevOff;					/* add offset */
	if (grp_fs_check_io_status(ptFs, uiDevBlk, iCnt, GRP_FS_IO_WRITE) != 0)
		goto write_error;						/* return error */

	/****************************************************/
	/* write data to device								*/
	/****************************************************/
again:
	ptFsCtl = grp_fs_ctl;						/* FS control infomration */
	grp_fs_block_buf_mod(ptBio);				/* start write */
	if ((ptBuf->usStatus & GRP_FS_BSTAT_DIRTY) == 0) { /* written by others */
		grp_fs_btrace(ptBuf, GRP_FS_BT_OTW);	/* trace(write_other) */
		grp_fs_unblock_buf_mod(ptBio, 0);		/* end write */
		return((grp_int32_t)ptBio->uiSize);		/* return success */
	}
	grp_fs_btrace(ptBuf, GRP_FS_BT_STW);		/* trace(start_write) */
#ifdef GRP_FS_ASYNC_UNMOUNT
	ptFs->iFsRef++;								/* increment reference count */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt++;						/* increment wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	iWrite = ptDevOp->pfnWrite(ptFs->iDevHandle, ptBuf->iDev, uiDevBlk,
						ptBuf->pucData, iCnt);	/* write */
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
#ifdef GRP_FS_ASYNC_UNMOUNT
	ptFs->iFsRef--;								/* decrement reference count */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt--;						/* decrement wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	grp_fs_unblock_buf_mod(ptBio, 0);			/* end write */
	if (iWrite != iCnt) {							/* I/O error */
		if (grp_fs_check_io_status(ptFs, uiDevBlk, iCnt,
									GRP_FS_IO_WRITE|GRP_FS_IO_OP_ERR) == 0) {
			grp_fs_btrace(ptBuf, GRP_FS_BT_ERW);/* trace(error_write) */
			goto again;							/* try again */
		}
		goto write_error;
	}
	ptBuf->usStatus &=
				~(GRP_FS_BSTAT_DIRTY|GRP_FS_BSTAT_WFAIL|GRP_FS_BSTAT_TSYNC);
												/* clear dirty and fail flag */
	grp_fs_btrace(ptBuf, GRP_FS_BT_ENW);		/* trace(end_write) */
	return((grp_int32_t)ptBio->uiSize);			/* return size */

write_error:
	grp_fs_printf("GRP_FS: write %s block(0x%lx) failed(dev:0x%x blk:0x%lx blk_shift:%d cnt:%ld)\n",
					((ptBuf->usStatus & GRP_FS_BSTAT_FBUF)? "file": "data"),
					(unsigned long)ptBuf->uiBlk, ptBuf->iDev,
					(unsigned long)uiDevBlk,
					ptFs->ucDevBlkShift, (long)iCnt);
	ptBuf->usStatus |= GRP_FS_BSTAT_WFAIL;		/* set write fail bit */
	grp_fs_btrace(ptBuf, GRP_FS_BT_ERW);		/* trace(err_write) */
	if (iWrite >= 0)							/* not error number */
		iWrite = GRP_FS_ERR_IO;					/* set error number */
	return(iWrite);								/* return I/O error */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_exec_dev_io											*/
/*																			*/
/* DESCRIPTION:	execute device I/O											*/
/* INPUT:		ptFs:				file system information					*/
/*				uiBlk:				block number							*/
/*				pucBuf:				I/O buffer (iOp == GRP_FS_IO_WRITE)		*/
/*				iCnt:				device block(sector) count				*/
/*				iOp:				operation mode							*/
/*										GRP_FS_IO_READ:  read operation		*/
/*										GRP_FS_IO_WRITE: write operation	*/
/*				iBlkKind:				I/O block kind						*/
/*										GRP_FS_BUF_FILE: file block			*/
/*										GRP_FS_BUF_DATA: data block			*/
/* OUTPUT:		pucBuf:				I/O buffer (iOp == GRP_FS_IO_READ)		*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_BAD_DEV	bad device								*/
/*				0 or positive:		data count								*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_exec_dev_io(
	grp_fs_info_t	*ptFs,					/* [IN]  head of buffer list */
	grp_uint32_t	uiBlk,					/* [IN]  block number */
	grp_uchar_t		*pucBuf,				/* [IN or OUT] I/O buffer */
	grp_isize_t		iCnt,					/* [IN]  device block count */
	int				iOp,					/* [IN]  operation mode */
	int				iBlkKind)				/* [IN]  I/O block kind */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control infomration */
	int				iMajor;					/* major device number */
	grp_fs_dev_tbl_t *ptDev;				/* device table entry */
	int				iDevBlkShift;			/* device block shift */
	grp_int32_t		iIoCnt;					/* I/O count */
	grp_uint32_t	uiDevBlk;				/* device block */

	iMajor = GRP_FS_DEV_MAJOR(ptFs->iDev);	/* major device number */
	ptDev = &grp_fs_dev_tbl[iMajor];		/* device table entry */
	if ((iMajor < 0 || iMajor >= grp_fs_dev_tbl_cnt)/* bad device number */
		|| ptDev->ptOp == NULL)				/* no operation */
		return(GRP_FS_ERR_BAD_DEV);			/* return eror */
	if (iBlkKind == GRP_FS_BUF_FILE) {		/* file block */
		iDevBlkShift = (int)(ptFs->ucFsFBlkShift - ptFs->ucDevBlkShift);
											/* device block shift count */
		uiDevBlk = ptFs->uiDevOff
				+ (ptFs->uiFsFBlkOff >> ptFs->ucDevBlkShift)
				+ (uiBlk << iDevBlkShift);	/* device block offset */
	} else {								/* data block */
		iDevBlkShift = (int)(ptFs->ucFsDBlkShift - ptFs->ucDevBlkShift);
											/* device block shift count */
		uiDevBlk = ptFs->uiDevOff
				+ (ptFs->uiFsDBlkOff >> ptFs->ucDevBlkShift)
				+ (uiBlk << iDevBlkShift);	/* device block offset */
	}
	if (grp_fs_check_io_status(ptFs, uiDevBlk, iCnt, iOp) != 0) {
		iIoCnt = GRP_FS_ERR_IO;				/* set error number */
		goto io_error;						/* return error */
	}

again:
	grp_fs_iotrace(ptFs->iDev,
		uiDevBlk, (uiDevBlk << ptFs->ucDevBlkShift), iCnt, pucBuf,
		(iOp & GRP_FS_IO_WRITE)? GRP_FS_IT_WFS: GRP_FS_IT_RFS);
											/* trace (start_io) */
#ifdef GRP_FS_ASYNC_UNMOUNT
	ptFs->iFsRef++;							/* increment reference count */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt++;					/* increment wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	if (iOp & GRP_FS_IO_WRITE) {			/* write opertion */
		iIoCnt = ptDev->ptOp->pfnWrite(ptFs->iDevHandle, ptFs->iDev,
						uiDevBlk, pucBuf, iCnt);	/* write */
	} else {								/* read operation */
		iIoCnt = ptDev->ptOp->pfnRead(ptFs->iDevHandle, ptFs->iDev,
						uiDevBlk, pucBuf, iCnt);	/* read */
	}
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
#ifdef GRP_FS_ASYNC_UNMOUNT
	ptFs->iFsRef--;							/* decrement reference count */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	ptFsCtl->iFsWaitCnt--;					/* decrement wait count */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	grp_fs_iotrace(ptFs->iDev,
		uiDevBlk, (uiDevBlk << ptFs->ucDevBlkShift), iIoCnt, pucBuf,
		(iOp & GRP_FS_IO_WRITE)? GRP_FS_IT_WFE: GRP_FS_IT_RFE);
											/* trace (end_io) */
	if (iIoCnt < 0) {							/* I/O error */
		if (grp_fs_check_io_status(ptFs, uiDevBlk, iCnt,
									iOp|GRP_FS_IO_OP_ERR) == 0)
			goto again;						/* try again */
		goto io_error;						/* error return */
	}
	return(iIoCnt);							/* return I/O count */

io_error:
	grp_fs_printf("GRP_FS: %s failed(dev:0x%x blk:0x%lx blk_shift:%d cnt:%ld)\n",
					((iOp & GRP_FS_IO_WRITE)? "write": "read"),
					ptFs->iDev, (unsigned long)uiDevBlk, ptFs->ucDevBlkShift,
					(long)iCnt);
	return(iIoCnt);							/* return I/O error */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_wait_buf_io											*/
/*																			*/
/* DESCRIPTION:	Wait for buffer I/O											*/
/* INPUT:		pptBufHead:			head of buffer list						*/
/*				iDev:				device number							*/
/*				iMode:				wait mode								*/
/*									GRP_FS_BUF_WAIT_INV: wait and invalidate */
/*									GRP_FS_BUF_FORCE_INV: force invalidate	*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_wait_buf_io(
	grp_fs_buf_t	**pptBufHead,			/* [IN]  head of buffer list */
	int				iDev,					/* [IN]  device number */
	int				iMode)					/* [IN]  wait mode */
{
	grp_fs_buf_t	*ptBuf;					/* buffer pointer */
	grp_fs_buf_t	*ptBufNext;				/* next buffer pointer */

again:
	for (ptBuf = *pptBufHead; ptBuf; ptBuf = ptBufNext) {
		ptBufNext = ptBuf->ptListFwd;		/* next buffer */
		if (ptBuf->iDev == iDev) {			/* buffer for the device */
			if (ptBuf->iRefCnt > 0) {		/* still referencing */
				ptBuf->usStatus |= GRP_FS_BSTAT_WAIT;/* set wait bit */
#ifdef GRP_FS_ASYNC_UNMOUNT
				grp_fs_task_wait(ptBuf->ptFs, (void *)ptBuf);
#else  /* GRP_FS_ASYNC_UNMOUNT */
				grp_fs_task_wait((void *)ptBuf); /* wait for end of referece */
#endif /* GRP_FS_ASYNC_UNMOUNT */
				goto again;					/* check again */
			}
			if ((ptBuf->usStatus & GRP_FS_BSTAT_DIRTY) /* dirty buffer */
				&& ((ptBuf->usStatus & GRP_FS_BSTAT_WFAIL) == 0 /* no fail */
					|| (iMode & GRP_FS_BUF_FORCE_INV) == 0)) { /* not force */
				if (_grp_fs_write_buf_int(ptBuf) != 0) { /* write failed */
					if (iMode & GRP_FS_BUF_FORCE_INV) {
						_grp_fs_invalidate_buf(ptBuf);/* invalidate buffer */
						goto again;			/* check again */
					}
					return(GRP_FS_ERR_IO);	/* return error */
				}
				goto again;					/* check again */
			}
			if (iMode & (GRP_FS_BUF_WAIT_INV|GRP_FS_BUF_FORCE_INV)) /* need inv */
				_grp_fs_invalidate_buf(ptBuf);/* invalidate buffer */
		}
	}
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_sync_buf_io											*/
/*																			*/
/* DESCRIPTION:	Update modified buffer write back to device					*/
/* INPUT:		pptBufHead:			head of buffer list						*/
/*				iDev:				device to update						*/
/*				iMode:				sync mode								*/
/*									GRP_FS_SYNC_FAILED: sync failed data	*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_sync_buf_io(
	grp_fs_buf_t	**pptBufHead,			/* [IN]  head of buffer list */
	int				iDev,					/* [IN]  device to update */
	int				iMode)					/* [IN]  sync mode */
{
	grp_fs_buf_t	*ptBuf;					/* buffer pointer */

	for (ptBuf = *pptBufHead; ptBuf; ptBuf = ptBuf->ptListFwd) {
		if (ptBuf->iDev < 0					/* no device */
			|| (iDev >= 0 && ptBuf->iDev != iDev)) /* device not match */
			continue;						/* advance to next */
		if ((ptBuf->usStatus & GRP_FS_BSTAT_DIRTY) /* dirty buffer */
			&& ((ptBuf->usStatus & GRP_FS_BSTAT_WFAIL) == 0 /* no fail */
				|| (iMode & GRP_FS_SYNC_FAILED))) { /* need to write */
			if (_grp_fs_write_buf_int(ptBuf) != 0) /* write failed */
				return(GRP_FS_ERR_IO);		/* return error */
		}
	}
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_wait_io												*/
/*																			*/
/* DESCRIPTION:	Wait for device I/O											*/
/* INPUT:		iDev:				device number							*/
/*				iMode:				wait mode								*/
/*									GRP_FS_BUF_WAIT_INV: wait and invalidate */
/*									GRP_FS_BUF_FORCE_INV: force invalidate	*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*																			*/
/****************************************************************************/
int
grp_fs_wait_io(
	int				iDev,				/* [IN]  device number */
	int				iMode)				/* [IN]  wait mode */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;/* FS control data */
	int				iRet;				/* return value */

	iRet = _grp_fs_wait_buf_io(&ptFsCtl->ptFBufFwd, iDev, iMode);
										/* wait for file I/O */
	if (iRet != 0)						/* error occured */
		return(iRet);					/* return error */
	return(_grp_fs_wait_buf_io(&ptFsCtl->ptDBufFwd, iDev, iMode));
										/* wait for data I/O */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_close_fhdl											*/
/*																			*/
/* DESCRIPTION:	Close file handle											*/
/* INPUT:		ptFhdl:				file handle 							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_BUSY:	file busy error							*/
/*																			*/
/****************************************************************************/
#ifdef GRP_FS_ASYNC_UNMOUNT
static int
_grp_fs_close_fhdl(
	grp_fs_fhdl_t	*ptFhdl)			/* [IN]  file handle */
{
	grp_fs_file_t	*ptFile;				/* file information */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	int				iNeedWakeup;			/* need wakeup */

	if (ptFhdl->usMode & GRP_FS_OPEN_UNDER_CLOSE) {	  /* under close */
		while (ptFhdl->usMode & GRP_FS_OPEN_UNDER_CLOSE) {
			ptFhdl->usMode |= GRP_FS_OPEN_WAIT_CLOSE; /* wait close mode */
			ptFile = ptFhdl->ptFile;		/* file information */
			grp_fs_task_wait(((ptFile == NULL)? NULL: ptFile->ptFs),
							(void *)ptFhdl);/* wait close */
		}
		return(0);							/* return success */
	}

	ptFile = ptFhdl->ptFile;				/* file information */
#ifdef  GRP_FS_RET_BUSY_AT_CLOSE
	if (ptFile != NULL
		&& (ptFile->usStatus & GRP_FS_FSTAT_BUSY)) { /* file busy */
		return(GRP_FS_ERR_BUSY);			/* return busy error */
	}
#endif  /* GRP_FS_RET_BUSY_AT_CLOSE */
	ptFhdl->usMode |= GRP_FS_OPEN_UNDER_CLOSE;	/* under close */
	if (ptFile != NULL) {					/* file opened */
		ptFhdl->ptFile = NULL;				/* clear file info */
#ifndef GRP_FS_RET_BUSY_AT_CLOSE
		while (ptFile->usStatus & GRP_FS_FSTAT_BUSY) {	/* under operation */
			ptFile->usStatus |= GRP_FS_FSTAT_WAIT;		/* wait for operation */
			grp_fs_task_wait(ptFile->ptFs, (void *)ptFile);
														/* wait for end of op */
		}
#endif	/* GRP_FS_RET_BUSY_AT_CLOSE */
		grp_fs_close_file(ptFile, 0);		/* close file */
	}
	iNeedWakeup = ((ptFhdl->usMode & GRP_FS_OPEN_WAIT_CLOSE) != 0);
											/* need to wakeup or not */
	ptFhdl->ptTask->iOpenCnt--;				/* decrement open count */
	ptFhdl->ptTask = NULL;					/* clear task env */
	ptFhdl->usMode = 0;						/* not open */
	grp_fs_fhdl_list(ptFhdl) = ptFsCtl->ptFhdlFree;	/* ins to free */
	ptFsCtl->ptFhdlFree = ptFhdl;			/* link to free */
	if (iNeedWakeup) {						/* need to wakeup */
		grp_fs_task_wakeup((void *)ptFhdl);	/* wakeup waiting task */
	}
	return(0);								/* return success */
}

#else	/* GRP_FS_ASYNC_UNMOUNT */

static int
_grp_fs_close_fhdl(
	grp_fs_fhdl_t	*ptFhdl)			/* [IN]  file handle */
{
	grp_fs_file_t	*ptFile;				/* file information */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */

	if (ptFhdl->ptFile)	{					/* file opened */
		ptFile = ptFhdl->ptFile;			/* file information */
#ifdef	GRP_FS_RET_BUSY_AT_CLOSE
		if (ptFile->usStatus & GRP_FS_FSTAT_BUSY) /* file busy */
			return(GRP_FS_ERR_BUSY);		/* return busy error */
#else	/* GRP_FS_RET_BUSY_AT_CLOSE */
		while (ptFile->usStatus & GRP_FS_FSTAT_BUSY) {	/* under operation */
				ptFile->usStatus |= GRP_FS_FSTAT_WAIT;	/* wait for operation */
				grp_fs_task_wait((void *)ptFile);		/* wait for end of op */
		}
#endif	/* GRP_FS_RET_BUSY_AT_CLOSE */
		grp_fs_close_file(ptFile, 0);		/* close file */
	}
	ptFhdl->ptTask->iOpenCnt--;				/* decrement open count */
	ptFhdl->ptTask = NULL;					/* clear task env */
	ptFhdl->usMode = 0;						/* not open */
	grp_fs_fhdl_list(ptFhdl) = ptFsCtl->ptFhdlFree;	/* ins to free */
	ptFsCtl->ptFhdlFree = ptFhdl;			/* link to free */
	return(0);								/* return success */
}
#endif /* GRP_FS_ASYNC_UNMOUNT */

/****************************************************************************/
/* FUNCTION:	_grp_fs_invalidate_fhdl										*/
/*																			*/
/* DESCRIPTION:	Invalidate file handles for the device						*/
/* INPUT:		iDev:				device number to invalidate				*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_invalidate_fhdl(
	int			iDev)					/* [IN]  device number to invalidate */
{
	grp_fs_fhdl_t	*ptFhdl;					/* file handle */
	grp_fs_fhdl_t	*ptFhdlEnd;					/* end of file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
#ifdef GRP_FS_ASYNC_UNMOUNT
	grp_fs_file_t	*ptFile;					/* file information */
#endif	/* GRP_FS_ASYNC_UNMOUNT */

	ptFhdl = ptFsCtl->ptFhdlTbl;				/* set file handle pointer */
	ptFhdlEnd  = &ptFhdl[grp_fs_param.uiFhdlCnt];/* end of file handle */
	for ( ; ptFhdl < ptFhdlEnd; ptFhdl++) {		/* loop for all handles */
		if (ptFhdl->usMode == 0)				/* not allocated */
			continue;
		if (ptFhdl->ptFile == NULL)				/* invalidated */
			continue;
		if (ptFhdl->ptFile->iDev != iDev)		/* not for the device */
			continue;
#ifdef GRP_FS_ASYNC_UNMOUNT
		ptFile = ptFhdl->ptFile;				/* file information */
		ptFhdl->ptFile = NULL;					/* clear file information */
		grp_fs_close_file(ptFile, GRP_FS_FILE_INVALID);
												/* invalidate file */
#else	/* GRP_FS_ASYNC_UNMOUNT */

		grp_fs_close_file(ptFhdl->ptFile, GRP_FS_FILE_INVALID);
												/* invalidate file */
		ptFhdl->ptFile = NULL;					/* close it */
#endif	/* GRP_FS_ASYNC_UNMOUNT */
	}
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_check_fhdl											*/
/*																			*/
/* DESCRIPTION:	Read data from file											*/
/* INPUT:		iFhdl:				file handle number						*/
/*				iMode:				open mode								*/
/* OUTPUT:		pptFhdl:			file handle								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_ERR_PERMIT	no read permission						*/
/*				GRP_FS_ERR_SHOULD_CLOSE: need to close the file				*/
/*				0 or positive:		data count								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_check_fhdl(
	int				iFhdl,				/* [IN]  file hundle number */
	int				iMode,				/* [IN]  open mode */
	grp_fs_fhdl_t	**pptFhdl)			/* [OUT] file handle */
{
	grp_fs_fhdl_t	*ptFhdl;					/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */

	*pptFhdl = NULL;							/* clear file handle */
	if (iFhdl < 0 || iFhdl >= (int)grp_fs_param.uiFhdlCnt)
												/* invalid handle number */
		return(GRP_FS_ERR_FHDL);				/* return error */
	ptFhdl = &ptFsCtl->ptFhdlTbl[iFhdl];		/* set file handle pointer */
#ifndef	GRP_FS_SHARE_OPEN
	if (ptFhdl->usMode == 0						/* not opened */
		|| ptFhdl->ptTask == NULL
		|| ptFhdl->ptTask->tTaskId != grp_fs_get_taskid())/* not my task */
#else	/* GRP_FS_SHARE_OPEN */
	if (ptFhdl->usMode == 0	|| ptFhdl->ptTask == NULL)	/* not opened */
#endif	/* GRP_FS_SHARE_OPEN */
		return(GRP_FS_ERR_FHDL);				/* return error */
	if ((ptFhdl->usMode & iMode) != iMode) 		/* not correct permission */
		return(GRP_FS_ERR_PERMIT);				/* return error */
	if (ptFhdl->ptFile == NULL && iMode)		/* no file */
		return(GRP_FS_ERR_SHOULD_CLOSE);		/* should close */
	*pptFhdl = ptFhdl;							/* set file handle */
	return(0);									/* return success */
}

#ifdef	GRP_FS_FNAME_CACHE
/****************************************************************************/
/* FUNCTION:	grp_fs_comp_fname_hash										*/
/*																			*/
/* DESCRIPTION:	Compute file name hash										*/
/* INPUT:		ptDir:				parent directory						*/
/*				pucName:			file name								*/
/* OUTPUT:		psNameLen:			name length								*/
/*																			*/
/* RESULT:		hash value													*/
/*																			*/
/****************************************************************************/
static grp_int32_t
grp_fs_comp_fname_hash(
	grp_fs_file_t	*ptDir,				/* [IN]  parent directory */
	const grp_uchar_t *pucName,			/* [IN]  file name */
	short			*psNameLen)			/* [OUT] name length */
{
	const grp_uchar_t *puc = pucName;	/* char pointer */
	grp_int32_t		iVal;				/* hash value */

	iVal = (((grp_int32_t)ptDir->iDev << 4) ^ ptDir->uiFid);/* init hash val */
	while (*puc)										/* loop to end of str */
		iVal = ((iVal << 1) ^ (*puc++ - '0'));			/* compute hash */
	*psNameLen = (short)(puc - pucName);				/* set name length */
	return(iVal & (grp_fs_param.uiFnameHashCnt - 1));	/* return hash value */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_purge_fname_cache_entry								*/
/*																			*/
/* DESCRIPTION:	purge file name cache entry									*/
/* INPUT:		ptCache:			file name cache entry					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_purge_fname_cache_entry(
	grp_fs_fname_cache_t	*ptCache)		/* [IN]  file name cache entry */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_fname_cache_t *ptAlias;			/* alias file name cache entry */

	if (ptCache->pptHashTop) {				/* on hash */
		grp_deque_sent(ptCache->pptHashTop, ptCache, ptHash);
		ptCache->pptHashTop = NULL;			/* remove from hash list */
	}
	ptCache->iDev = -1;						/* no associated device */
	grp_deque_ent(ptFsCtl, ptFnCache, ptCache, ptList);	/* deque from LRU */
	grp_enque_tail(ptFsCtl, ptFnCache, ptCache, ptList);/* enque LRU tail */
	ptAlias = ptCache->ptAlias;				/* alias entry */
	ptCache->ptAlias = NULL;				/* reset alias */
	if (ptAlias) {							/* exist alias */
		ptAlias->ptAlias = NULL;			/* reset alias */
		grp_fs_purge_fname_cache_entry(ptAlias); /* purge alias entry */
	}
}

/****************************************************************************/
/* FUNCTION:	grp_fs_lookup_fname_cache_entry								*/
/*																			*/
/* DESCRIPTION:	Lookup file name cache entry								*/
/* INPUT:		ptDir:				parent directory						*/
/*				pucName:			file name								*/
/*				iAlloc:				allocate cache if not found				*/
/* OUTPUT:		pptFnCache:			found cache entry						*/
/*																			*/
/* RESULT:		0:					found cache entry						*/
/*				GRP_FS_ERR_NOT_FOUND not found								*/
/*				GRP_FS_ERR_NOMEM	no memory to allocate					*/
/*																			*/
/****************************************************************************/
static int
grp_fs_lookup_fname_cache_entry(
	grp_fs_file_t	*ptDir,					/* [IN]  parent directory */
	const grp_uchar_t *pucName,				/* [IN]  file name */
	int				iAlloc,					/* [IN]  allocate if not found */
	grp_fs_fname_cache_t **pptFnCache)		/* [OUT] fname cache entry */
{
	grp_fs_fname_cache_t *ptCache;			/* file name cache entry */
	grp_fs_fname_cache_t **pptHashTop;		/* hash top */
	grp_fs_fname_cache_t *ptAlias;			/* alias file name cache entry */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	int				iDev = ptDir->iDev;		/* parent device number */
	grp_uint32_t	uiFid = ptDir->uiFid;	/* parent file id */
	grp_int32_t		iHashVal;				/* hash value */
	short			sNameLen;				/* name length */

	/****************************************************/
	/* lookup hash 										*/
	/****************************************************/
	iHashVal = grp_fs_comp_fname_hash(ptDir, pucName, &sNameLen);
	pptHashTop = &ptFsCtl->pptFnHash[iHashVal];		/* hash top */
	for (ptCache = *pptHashTop; ptCache; ptCache = ptCache->ptHashFwd) {
		if (ptCache->iDev == iDev
			&& ptCache->uiDirFid == uiFid
			&& ptCache->sNameLen == sNameLen
			&& strcmp((char *)ptCache->pucName, (char *)pucName) == 0) {
				/****************************************************/
				/* found entry;										*/
				/*    rechain to top of hash/LRU list and return	*/
				/****************************************************/
				ptAlias = ptCache->ptAlias;	/* alias entry */
				if (ptAlias) {				/* exist alias */
					grp_deque_ent(ptFsCtl, ptFnCache, ptAlias, ptList);
					grp_enque_head(ptFsCtl, ptFnCache, ptAlias, ptList);
				}
				grp_deque_sent(pptHashTop, ptCache, ptHash);
				grp_enque_shead(pptHashTop, ptCache, ptHash);
				grp_deque_ent(ptFsCtl, ptFnCache, ptCache, ptList);
				grp_enque_head(ptFsCtl, ptFnCache, ptCache, ptList);
				*pptFnCache = ptCache;		/* set found one */
				return(0);					/* return found */
		}
	}

	/****************************************************/
	/* cache not found;									*/
	/*     if iAlloc is specified, get an entry from 	*/
	/*   tail of the LRU list, and return 				*/
	/****************************************************/
	if (!iAlloc) {							/* not allocate */
		*pptFnCache = NULL;					/* no entry */
		return(GRP_FS_ERR_NOT_FOUND);		/* return not found */
	}
	ptCache = ptFsCtl->ptFnCacheBwd;		/* tail of LRU list */
	if (ptCache->pptHashTop) {				/* on hash */
		grp_deque_sent(ptCache->pptHashTop, ptCache, ptHash);
		ptCache->pptHashTop = NULL;			/* remove from hash list */
	}
	ptAlias = ptCache->ptAlias;				/* alias entry */
	ptCache->ptAlias = NULL;				/* reset alias */
	if (ptAlias) {							/* exist alias */
		ptAlias->ptAlias = NULL;			/* reset alias */
		grp_fs_purge_fname_cache_entry(ptAlias); /* purge alias entry */
	}
	if (ptCache->pucName					/* allocated name area */
		&& ptCache->sNameBufLen < sNameLen + 1) { /* but not enough size */
		grp_mem_free(ptCache->pucName);		/* free name area */
		ptCache->pucName = NULL;			/* reset name area pointer */
	}
	if (ptCache->pucName == NULL) {			/* no name area */
		ptCache->pucName = (grp_uchar_t *)
							grp_mem_alloc((grp_isize_t)sNameLen + 1);
		if (ptCache->pucName == NULL) {		/* no name area */
			*pptFnCache = NULL;				/* no entry */
			return(GRP_FS_ERR_NOMEM);		/* return error */
		}
		ptCache->sNameBufLen = (short)(sNameLen + 1);/* set buffer length */
	}
	strcpy((char *)ptCache->pucName, (char *)pucName);	/* copy name */
	ptCache->sNameLen = sNameLen;			/* set name length */
	ptCache->iDev = iDev;					/* set dev ID */
	ptCache->uiDirFid = uiFid;				/* set parent file ID */
	grp_enque_shead(pptHashTop, ptCache, ptHash);		/* enque hash list */
	ptCache->pptHashTop = pptHashTop;					/* set hash top */
	grp_deque_ent(ptFsCtl, ptFnCache, ptCache, ptList);	/* deque from LRU */
	grp_enque_head(ptFsCtl, ptFnCache, ptCache, ptList);/* enque LRU top */
	*pptFnCache = ptCache;					/* set allocated entry */
	return(GRP_FS_ERR_NOT_FOUND);			/* return not found */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_lookup_fname_cache									*/
/*																			*/
/* DESCRIPTION:	Lookup file name cache										*/
/* INPUT:		ptDir:				parent directory						*/
/*				pucName:			file name								*/
/*				iPurge:				purge cache if found					*/
/* OUTPUT:		pptFile:			found file information					*/
/*																			*/
/* RESULT:		0:					found cache entry						*/
/*				GRP_FS_ERR_NOT_FOUND not found								*/
/*																			*/
/****************************************************************************/
int
grp_fs_lookup_fname_cache(
	grp_fs_file_t	*ptDir,					/* [IN]  parent directory */
	const grp_uchar_t *pucName,				/* [IN]  file name */
	int				iPurge,					/* [IN]  purge cache entry */
	grp_fs_file_t	**pptFile)				/* [OUT] found file information */
{
	grp_fs_fname_cache_t *ptCache;			/* file name cache entry */
	int				iRet;					/* return value */
	int				iDotDot;				/* ".." file */

	if (pptFile)							/* need to return file info */
		*pptFile = NULL;					/* set default info */
	iRet = grp_fs_lookup_fname_cache_entry(ptDir, pucName, 0, &ptCache);
											/* lookup cache entry */
	if (iRet)								/* not found */
		return(GRP_FS_ERR_NOT_FOUND);		/* return not found */
	if (pptFile == NULL)					/* not need to return file info */
		return(0);							/* return found */

	iDotDot = (strcmp((char *)pucName, "..") == 0); /* ".." file */
	if (iDotDot)							/* ".." file */
		grp_fs_unblock_file_op(ptDir);		/* temporary unblock file op */
	iRet = grp_fs_lookup_file_ctl(ptDir->ptFs, ptCache->uiFid, 0, pptFile);
											/* lookup file control */
	if (iDotDot)							/* ".." file */
		grp_fs_block_file_op(ptDir);		/* block file op again */
	if (iRet) {								/* no cached file control */
		grp_fs_purge_fname_cache_entry(ptCache); /* purge cache entry */
		return(GRP_FS_ERR_NOT_FOUND);		/* return not found */
	}
	if (iPurge)								/* need to purge entry */
		grp_fs_purge_fname_cache_entry(ptCache); /* purge cache entry */
	return(0);								/* return found */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_set_fname_cache										*/
/*																			*/
/* DESCRIPTION:	Set file name cache											*/
/* INPUT:		ptDir:				parent directory						*/
/*				pucName:			file name								*/
/* 				ptFile:				file information						*/
/*				ptAlias:			alias cache entry						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:				cannot set file name cache				*/
/*				others:				file name cache entry					*/
/*																			*/
/****************************************************************************/
grp_fs_fname_cache_t *
grp_fs_set_fname_cache(
	grp_fs_file_t	*ptDir,					/* [IN]  parent directory */
	const grp_uchar_t *pucName,				/* [IN]  file name */
	grp_fs_file_t	*ptFile,				/* [IN]  file information */
	grp_fs_fname_cache_t *ptAlias)			/* [IN]  alias cache entry */
{
	grp_fs_fname_cache_t *ptCache;			/* file name cache entry */
	int				iRet;					/* return value */

	iRet = grp_fs_lookup_fname_cache_entry(ptDir, pucName, 1, &ptCache);
											/* lookup cache entry */
	if (iRet == 0)							/* found cache entry */
		return(ptCache);					/* do nothing */
	if (iRet != GRP_FS_ERR_NOT_FOUND)		/* other than not found */
		return(NULL);						/* return NULL */
	ptCache->uiFid = ptFile->uiFid;			/* set file ID */
	if (ptCache->ptAlias) { 				/* exist old alias */
		ptCache->ptAlias->ptAlias = NULL;	/* reset alias link */
		grp_fs_purge_fname_cache_entry(ptCache->ptAlias);
											/* purge alias */
	}
	ptCache->ptAlias = ptAlias;				/* set alias link */
	if (ptAlias)							/* exist alias */
		ptAlias->ptAlias = ptCache;			/* set back alias link */
	return(ptCache);						/* return success */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_purge_fname_cache_by_dev								*/
/*																			*/
/* DESCRIPTION:	Purge file name cache by device								*/
/* INPUT:		iDev:				device ID to purge						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_purge_fname_cache_by_dev(
	int				iDev)					/* [IN]  device ID to purge */
{
	grp_fs_fname_cache_t *ptCache;			/* file name cache entry */
	grp_fs_fname_cache_t *ptLast;			/* last file name cache entry */
	grp_fs_fname_cache_t *ptNext;			/* next cache entry */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */

	ptLast = ptFsCtl->ptFnCacheBwd;			/* last entry */
	ptNext = ptFsCtl->ptFnCacheFwd;			/* first entry */
	do {
		ptCache = ptNext;					/* current */
		ptNext = ptCache->ptListFwd;		/* next entry */
		if (ptCache->pptHashTop				/* on hash */
			&& (iDev < 0 || ptCache->iDev == iDev)) {/* match device ID */
			ptCache->ptAlias = NULL;				/* purge this entry only */
			grp_fs_purge_fname_cache_entry(ptCache);/* purge cache entry */
		}
	} while (ptCache != ptLast);			/* until last entry */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_purge_fname_cache									*/
/*																			*/
/* DESCRIPTION:	Purge file name cache										*/
/* INPUT:		ptDir:				parent directory						*/
/*				pucName:			file name								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void grp_fs_purge_fname_cache(
	grp_fs_file_t	*ptDir,					/* [IN]  parent directory */
	const grp_uchar_t *pucName)				/* [IN]  file name */
{
	grp_fs_fname_cache_t *ptCache;			/* file name cache entry */

	if (grp_fs_lookup_fname_cache_entry(ptDir, pucName, 0, &ptCache) == 0)
		grp_fs_purge_fname_cache_entry(ptCache); /* purge cache entry */
}

#endif	/* GRP_FS_FNAME_CACHE */

/****************************************************************************/
/* FUNCTION:	grp_fs_lookup_dev											*/
/*																			*/
/* DESCRIPTION:	Lookup device												*/
/* INPUT:		pcDev:				device name								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV	bad device name							*/
/*				GRP_FS_ERR_BAD_PARAM: bad parameter							*/
/*				others:				device number							*/
/*																			*/
/****************************************************************************/
int
grp_fs_lookup_dev(
	const char		*pcDev)					/* [IN] device name */
{
	int				iMajor = 0;				/* major dev number */
	int				iSubId = 0;				/* sub device Id */
	int				iPart = 0;				/* partition number */
	int				iMinor;					/* minor dev number */
	int				iLen;					/* device name length */
	int				iRet;					/* return value */
	grp_fs_dev_tbl_t *ptDev;				/* device table pointer */
	const char		*pcMp;					/* minor dev name pointer */
	char			acDevName[GRP_FS_DEV_NAME_LEN]; /*device name */

	/****************************************************/
	/* get user space string to local area				*/
	/****************************************************/
	iRet = grp_fs_get_str((grp_uchar_t *)acDevName,
						(const grp_uchar_t *)pcDev, sizeof(acDevName));
											/* get string to local area */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pcDev = acDevName;						/* switch to local device name */

	/****************************************************/
	/* lookup device table								*/
	/****************************************************/
	for (ptDev = grp_fs_dev_tbl; ptDev->pcDevName; ptDev++, iMajor++) {
		/****************************************************/
		/* check device name part							*/
		/****************************************************/
		iLen = (int)strlen(ptDev->pcDevName);/* device name length */
		if (strncmp(ptDev->pcDevName, pcDev,
					(grp_size_t)iLen) != 0)	/* not match */
			continue;

		/****************************************************/
		/* get sub device ID part							*/
		/****************************************************/
		for (pcMp = &pcDev[iLen]; *pcMp >= '0' && *pcMp <= '9'; pcMp++)
			iSubId = (iSubId * 10) + *pcMp - '0';

		/****************************************************/
		/* get partition number								*/
		/****************************************************/
		if (*pcMp >= 'a' && *pcMp <= 'o') {	/* partition information */
			iPart = *pcMp++ - 'a';			/* partition number */
		} else if (*pcMp == '*') {			/* raw partition */
			iPart = GRP_FS_DEV_RAW_PART;	/* raw partition number */
			pcMp++;							/* advance character */
		}
		if (*pcMp)							/* not match */
			continue;						/* search next */
		iMinor = GRP_FS_DEV_MK_MINOR(iSubId, iPart);
											/* make minor device number */
		return(GRP_FS_DEV_NO(iMajor, iMinor)); /* return device number */
	}
	return(GRP_FS_ERR_BAD_DEV);				/* return error */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_lookup_fs_type										*/
/*																			*/
/* DESCRIPTION:	Lookup file system type										*/
/* INPUT:		pcFsName:			file system name						*/
/* OUTPUT:		pptFsTbl:			file system table entry					*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_BAD_PARAM: bad parameter							*/
/*				GRP_FS_ERR_BAD_FSNAME: bad file system type name			*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_lookup_fs_type(
	const char			*pcFsName,			/* [IN]  file system type name */
	grp_fs_type_tbl_t	**pptFsTbl)			/* [OUT] FS table entry */
{
	grp_fs_type_tbl_t	*ptFsType;			/* FS type table pointer */
	int					iRet;				/* return value */
	char				acFsName[GRP_FS_TYPE_LEN]; /* FS type name */

	/****************************************************/
	/* get user space string to local area				*/
	/****************************************************/
	*pptFsTbl = NULL;						/* set default value */
	iRet = grp_fs_get_str((grp_uchar_t *)acFsName,
						(const grp_uchar_t *)pcFsName, sizeof(acFsName));
											/* get string to local area */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pcFsName = acFsName;					/* switch to local name */

	/****************************************************/
	/* lookup FS table									*/
	/****************************************************/
	for (ptFsType = grp_fs_type_tbl; ptFsType->pcFsName; ptFsType++) {
		if (strcmp(pcFsName, ptFsType->pcFsName) == 0)	/* match name */ {
			*pptFsTbl = ptFsType;			/* set found entry */
			return(0);						/* return the entry */
		}
	}
	return(GRP_FS_ERR_BAD_FSNAME);			/* return error */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_set_access_time										*/
/*																			*/
/* DESCRIPTION:	Set access time												*/
/* INPUT:		ptFile->iATime:	previous access time						*/
/* OUTPUT:		ptFile->iATime:	new access time								*/
/*				ptFile->usStatus: GRP_FS_FSTAT_UPD_ATIME flag if updated	*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_set_access_time(
	grp_fs_file_t	*ptFile)				/* [IN] file information */
{
	grp_int32_t		iTime;					/* current time */

	grp_fs_get_current_time(&iTime);		/* get current time */
	if ((ptFile->ptFs->usStatus & GRP_FS_STAT_DAY_ACCTIME) == 0
		|| ((iTime + grp_time_diff_from_GMT) / ((grp_int32_t)24 * 60 * 60) 
			!= (ptFile->iATime + grp_time_diff_from_GMT)
											/ ((grp_int32_t)24 * 60 * 60)))
		ptFile->usStatus |= GRP_FS_FSTAT_UPD_ATIME;	/* set update flag */
	ptFile->iATime = iTime;						/* set new time */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_path_comp										*/
/*																			*/
/* DESCRIPTION:	Get path component											*/
/* INPUT:		ppucPath:			path name								*/
/*				iCompBufSize:		component buffer size					*/
/* OUTPUT:		pucComp:			path component							*/
/*				ppucPath:			start of next component					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_TOO_LONG	too long path name						*/
/*				GRP_FS_ERR_BAD_NAME	include bad character					*/
/*				0:					no component							*/
/*				GRP_FS_COMP_LAST	last component							*/
/*				GRP_FS_COMP_MIDDLE	middle of path							*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_path_comp(
	const grp_uchar_t **ppucPath,			/* [IN/OUT]  path name */
	grp_uchar_t		*pucComp,				/* [OUT] component buffer */
	int				iCompBufSize)			/* [IN]  component buffer size */
{
	const grp_uchar_t *pucPath = *ppucPath;	/* path name */
	int				iCharByte;				/* character byte count */
	int				iLen;					/* length of component */

	/****************************************************/
	/* skip leading '/' or '\\' or ' ' 					*/
	/****************************************************/
	for ( ;(*pucPath == '/' || *pucPath == '\\' || *pucPath == ' '); pucPath++);
	*ppucPath = pucPath;					/* set next component pointer */
	if (*pucPath == 0) 						/* no component */
		return(0);							/* return 0 */

	/****************************************************/
	/* copy component									*/
	/****************************************************/
	while (*pucPath && *pucPath != '/' && *pucPath != '\\') {
		iCharByte = grp_fs_char_cnt(pucPath);/* get character count */
		if (iCharByte <= 0)					/* invalid character sequence */
			return(GRP_FS_ERR_BAD_NAME);	/* return error */
		pucPath += iCharByte;				/* advance to net */
	}
	iLen = (int)(pucPath - *ppucPath);		/* length of component */
	if (iLen > iCompBufSize - 1)			/* too long component */
		return(GRP_FS_ERR_TOO_LONG);		/* too long component */
	memcpy(pucComp, *ppucPath, (grp_size_t)iLen); /* copy component */
	pucComp[iLen] = 0;						/* NULL terminate */

	/****************************************************/
	/* skip trailing '/' or '\\' or ' ' 				*/
	/****************************************************/
	for ( ;(*pucPath == '/' || *pucPath == '\\' || *pucPath == ' '); pucPath++);
	*ppucPath = pucPath;					/* set next component pointer */
	return((*pucPath)? GRP_FS_COMP_MIDDLE: GRP_FS_COMP_LAST);/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_check_mnt_dev										*/
/*																			*/
/* DESCRIPTION:	Check mount device is already mounted or not				*/
/* INPUT:		ptFs:				FS information to check					*/
/*				iDev:				device number to check					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:				not mounted								*/
/*				others:				mounted FS info							*/
/*																			*/
/****************************************************************************/
grp_fs_info_t *
grp_fs_check_mnt_dev(
	grp_fs_info_t	*ptFs,					/* [IN]  FS information to check */
	int				iDev)					/* [IN]  device number to check */
{
	grp_fs_info_t	*ptNestFs;				/* nested FS info */

	for ( ; ptFs; ptFs = ptFs->ptFsOtherFwd) {
		if (ptFs->iDev == iDev)				/* device number matched */
			return(ptFs);					/* return it */
		if (ptFs->ptFsNest) {				/* nested mount */
			ptNestFs = grp_fs_check_mnt_dev(ptFs->ptFsNest, iDev);
			if (ptNestFs != NULL)			/* found in nested mount */
				return(ptNestFs);			/* return it */
		}
	}
	return(NULL);							/* not mounted */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_lookup_mount										*/
/*																			*/
/* DESCRIPTION:	Lookup mount point											*/
/* INPUT:		ptFsBase:			base point								*/
/*				ppucPath:			path name								*/
/* OUTPUT:		ppucPath:			remaing path							*/
/*				pptFsIns:			insert point FS info					*/
/*																			*/
/* RESULT:		NULL:				not found								*/
/*				others:				found grp_fs_info_t						*/
/*																			*/
/****************************************************************************/
static grp_fs_info_t *
_grp_fs_lookup_mount(
	grp_fs_info_t		*ptFsBase,			/* [IN]  base point */
	const grp_uchar_t	**ppucPath,			/* [IN/OUT] path name */
	grp_fs_info_t		**pptFsIns)			/* [OUT] insert point */
{
	const grp_uchar_t	*pucPath = *ppucPath;	/* path name */
	grp_fs_info_t		*ptFs;					/* FS info pointer */
	grp_fs_info_t		*ptFsPrev = NULL;		/* previous FS info pointer */
	grp_fs_info_t		*ptFsFound = NULL;		/* found FS info */
	int					iLen;					/* length of path */
	int					iCmpLen;				/* compare length */

	/****************************************************/
	/* lookup matched FS info							*/
	/****************************************************/
	iLen = (int)strlen((char *)pucPath);		/* path length */
	for (ptFs = ptFsBase; ptFs; ptFs = ptFs->ptFsOtherFwd) {
		iCmpLen = ptFs->sPathLen;				/* compare length */
		if (iCmpLen > iLen)						/* less length */
			break;								/* search end */
		if (((pucPath[0] == '/' || pucPath[0] == '\\') &&
			 (ptFs->aucPath[0] == '/' || ptFs->aucPath[0] == '\\') &&
			  iCmpLen == 1) ||
			(strncmp((char *)ptFs->aucPath, (char *)pucPath,
						(grp_size_t)iCmpLen) == 0
			 && (ptFs->ptFsParent == NULL
				|| pucPath[iCmpLen] == 0
				|| pucPath[iCmpLen] == '/'
				|| pucPath[iCmpLen] == '\\'))) {
			ptFsFound = ptFs;					/* set found */
			pucPath += iCmpLen;					/* advance */
			while (*pucPath == '/' || *pucPath == '\\')
				pucPath++;						/* skip separator */
			break;
		}
		ptFsPrev = ptFs;						/* previous FS info */
	}
	*ppucPath = pucPath;						/* set path */
	if (pptFsIns)								/* need insert point */
		*pptFsIns = ptFsPrev;					/* set insert point */
	return(ptFsFound);							/* return found FS info */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_find_nest_mount										*/
/*																			*/
/* DESCRIPTION:	Find nested mount point										*/
/*				Note: FS info is returned with iFsRef field incremented		*/
/* INPUT:		ptFile:				nested mount point						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		mounted file system											*/
/*																			*/
/****************************************************************************/
static grp_fs_info_t *
_grp_fs_find_nest_mount(
	grp_fs_file_t		*ptFile)			/* [IN]  mount point */
{
	grp_fs_info_t		*ptFs;				/* FS info pointer */

	for (ptFs = ptFile->ptFs->ptFsNest; ptFs; ptFs = ptFs->ptFsOtherFwd) {
		if (ptFs->ptFsParent == ptFile) {	/* match mount point */
			ptFs->iFsRef++;					/* increment reference */
			break;							/* break */
		}
	}
	return(ptFs);							/* return found */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_wait_mount											*/
/*																			*/
/* DESCRIPTION:	Wait for mount/umount operation								*/
/* INPUT:		iNoWait:			no wait									*/
/* OUTPUT:		GRP_FS_CSTAT_BUSY bit is set in grp_fs_ctl->iFsStatus for	*/
/*				no wait case.												*/
/*																			*/
/* RESULT:		0:					no mount/umount operation in progress	*/
/*				GRP_FS_ERR_BUSY:	mount/umount in progress (no wait case)	*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_wait_mount(
	int				iNoWait)					/* [IN]  no wait */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */

	while (ptFsCtl->iFsStat & GRP_FS_CSTAT_BUSY) {/* mount/umount in progress */
		if (iNoWait)							/* no wait */
			return(GRP_FS_ERR_BUSY);			/* return with busy */
		ptFsCtl->iFsStat |= GRP_FS_CSTAT_WAIT;	/* set wait flag */
#ifdef GRP_FS_ASYNC_UNMOUNT
		grp_fs_task_wait(NULL, (void *)ptFsCtl);/* wait for mount/umount */
#else  /* GRP_FS_ASYNC_UNMOUNT */
		grp_fs_task_wait((void *)ptFsCtl);		/* wait for mount/umount */
#endif /* GRP_FS_ASYNC_UNMOUNT */
	}
	if (iNoWait)
		ptFsCtl->iFsStat |= GRP_FS_CSTAT_BUSY;	/* set busy flag */
	return(0);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_wakup_wait_mount										*/
/*																			*/
/* DESCRIPTION:	Wakeup task waiting for mount/umount operation				*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_wakeup_wait_mount(void)
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */

	ptFsCtl->iFsStat &= ~GRP_FS_CSTAT_BUSY;		/* reset busy flag */
	if (ptFsCtl->iFsStat & GRP_FS_CSTAT_WAIT) {	/* wait for mount/umount */
		ptFsCtl->iFsStat &= ~GRP_FS_CSTAT_WAIT;	/* reset wait bit */
		grp_fs_task_wakeup((void *)ptFsCtl);	/* wake up task */
	}
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_check_mnt_dev_busy									*/
/*																			*/
/* DESCRIPTION:	Check device is busy as mounted device						*/
/* INPUT:		ptFs:				FS information to check					*/
/*				iDev:				device number to check					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					not busy								*/
/*				1:					busy									*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_check_mnt_dev_busy(
	grp_fs_info_t	*ptFs,					/* [IN]  FS information to check */
	int				iDev)					/* [IN]  device number to check */
{
	int				iFound;					/* found device */

	for ( ; ptFs; ptFs = ptFs->ptFsOtherFwd) {
		if (ptFs->iDev == iDev				/* device number matched */
			|| (GRP_FS_DEV_MATCH_SUB(ptFs->iDev, iDev)
				&& (GRP_FS_DEV_PART(ptFs->iDev) == GRP_FS_DEV_RAW_PART
					|| GRP_FS_DEV_PART(iDev) == GRP_FS_DEV_RAW_PART)))
			return(1);						/* return busy */
		if (ptFs->ptFsNest) {				/* nested mount */
			iFound = _grp_fs_check_mnt_dev_busy(ptFs->ptFsNest, iDev);
			if (iFound)						/* found in nested mount */
				return(iFound);				/* return busy */
		}
	}
	return(0);								/* return not busy */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_check_dev_busy										*/
/*																			*/
/* DESCRIPTION:	Check device is busy										*/
/* INPUT:		iDev:				device number to check					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					not busy								*/
/*				1:					busy									*/
/*																			*/
/****************************************************************************/
int
grp_fs_check_dev_busy(
	int				iDev)					/* [IN]  device number to check */
{
	int				iRet;					/* return value */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */

	/****************************************************/
	/* lock mount/umount								*/
	/****************************************************/
	if (ptFsCtl == NULL)					/* no control information */
		return(0);							/* return not busy */
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	if (_grp_fs_wait_mount(1) != 0) {		/* mount/umount in progress */
		iRet = 1;							/* return busy */
		goto out;							/* goto out */
	}

	/****************************************************/
	/* check device busy								*/
	/****************************************************/
	iRet = _grp_fs_check_mnt_dev_busy(ptFsCtl->ptFsMnt, iDev);

	/****************************************************/
	/* unlock mount										*/
	/****************************************************/
	_grp_fs_wakeup_wait_mount();			/* wakeup wait mount */
out:
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_read													*/
/*																			*/
/* DESCRIPTION:	Read data from file											*/
/* INPUT:		iFhdl:				file handle number						*/
/*				iSize:				size to read							*/
/* OUTPUT:		pucBuf:				data read								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_ERR_PERMIT	no read permission						*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_SHOULD_CLOSE: need to close the file				*/
/*				0 or positive:		data count								*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
grp_isize_t
grp_fs_read(
	int				iFhdl,				/* [IN]  file handle number */
	grp_uchar_t		*pucBuf,			/* [OUT]  buffer to fill data */
	grp_isize_t		iSize)				/* [IN]  size to read */
{
	int				iBlkShift;					/* FS block shift */
	grp_int32_t		iBlkSize;					/* FS block offset mask */
	grp_uint32_t	uiFsBlk;					/* FS block number */
	grp_uint32_t	uiBlkOff;					/* block offset */
	grp_isize_t		iTotal;						/* total read count */
	grp_int32_t		iRead;						/* read size */
	grp_fs_file_t	*ptFile;					/* file pointer */
	grp_fs_info_t	*ptFs;						/* file system information */
	grp_fs_fhdl_t	*ptFhdl;					/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	int				iRet;						/* return value */

	/****************************************************/
	/* check file handle								*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	iRet = _grp_fs_check_fhdl(iFhdl, GRP_FS_OPEN_READ, &ptFhdl);
												/* check handle */
	if (iRet != 0)								/* error detected */
		goto err_ret;							/* return error */

	/****************************************************/
	/* read data										*/
	/****************************************************/
	ptFile = ptFhdl->ptFile;					/* get file pointer */
	ptFs = ptFile->ptFs;						/* file system info */
	iBlkShift = ptFs->ucFsDBlkShift;			/* FS data block shift */
	iBlkSize = ((grp_int32_t)1 << iBlkShift);	/* FS block offset mask */
	for (iTotal = 0; iTotal < iSize; iTotal += iRead) {
#ifdef GRP_FS_ENABLE_OVER_2G
		uiFsBlk = (ptFhdl->uiOffset >> iBlkShift); 	/* FS block number */
		uiBlkOff = (ptFhdl->uiOffset & (iBlkSize - 1));/* FS block offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
		uiFsBlk = (ptFhdl->iOffset >> iBlkShift); 	/* FS block number */
		uiBlkOff = (ptFhdl->iOffset & (iBlkSize - 1));/* FS block offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
		grp_fs_block_file_op(ptFile);			/* block file operation */
		iRead = ptFs->ptFsOp->pfnRead(ptFile, uiFsBlk, uiBlkOff,
									pucBuf, iSize - iTotal,
									(ptFhdl->usMode & GRP_FS_OPEN_DIRECT_IO));
												 /* read data */
		grp_fs_unblock_file_op(ptFile);			/* unblock file operation */
		if (iRead <= 0)  {						/* EOF or read error */
			if (iRead == 0)						/* EOF */
				break;							/* read end */
			iRet = (int)iRead;					/* set error number */
			goto err_ret;						/* return error */
		}
		pucBuf += iRead;						/* advance buffer pointer */
#ifdef GRP_FS_ENABLE_OVER_2G
		ptFhdl->uiOffset += iRead;				/* advance offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptFhdl->iOffset += iRead;				/* advance offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iTotal);

err_ret:
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return((grp_isize_t)iRet);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_write												*/
/*																			*/
/* DESCRIPTION:	Write data to file											*/
/* INPUT:		iFhdl:				file handle number						*/
/* 				pucBuf:				data to write							*/
/*				iSize:				size to write							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				GRP_FS_ERR_SHOULD_CLOSE: need to close the file				*/
/*				0 or positive:		data count								*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
grp_isize_t
grp_fs_write(
	int				iFhdl,				/* [IN]  file handle number */
	grp_uchar_t		*pucBuf,			/* [IN]  data to write */
	grp_isize_t		iSize)				/* [IN]  size to write */
{
	int				iBlkShift;					/* FS block shift */
	grp_int32_t		iBlkSize;					/* FS block offset mask */
	grp_uint32_t	uiFsBlk;					/* FS block number */
	grp_uint32_t	uiBlkOff;					/* block offset */
	grp_isize_t		iTotal;						/* total write count */
	grp_int32_t		iWrite;						/* written size */
	grp_fs_file_t	*ptFile;					/* file pointer */
	grp_fs_info_t	*ptFs;						/* file system information */
	grp_fs_fhdl_t	*ptFhdl;					/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	int				iRet;						/* return value */

	/****************************************************/
	/* check file handle								*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	iRet = _grp_fs_check_fhdl(iFhdl, GRP_FS_OPEN_WRITE, &ptFhdl);/* check hdl */
	if (iRet != 0)								/* error detected */
		goto err_ret;							/* return error */

	/****************************************************/
	/* write data										*/
	/****************************************************/
	ptFile = ptFhdl->ptFile;					/* get file pointer */
	ptFs = ptFile->ptFs;						/* file system info */
	if (grp_fs_check_io_status(ptFs, 0, 0, GRP_FS_IO_REQ) != 0) {
		iRet = GRP_FS_ERR_IO;					/* I/O error */
		goto err_ret;
	}
	iBlkShift = ptFs->ucFsDBlkShift;			/* FS block shift */
	iBlkSize = ((grp_int32_t)1 << iBlkShift);	/* FS block offset mask */
#ifdef GRP_FS_ENABLE_OVER_2G
	if (ptFhdl->usMode & GRP_FS_OPEN_APPEND)	/* append mode */
		ptFhdl->uiOffset = ptFile->uiSize;		/* set to end of file */
	if(iSize > 0 && ((ptFhdl->uiOffset + iSize) < ptFhdl->uiOffset) ){ /* over 4G */
		iSize = 0xffffffff - ptFhdl->uiOffset;	/* adjust size within 4G */
	}
#else  /* GRP_FS_ENABLE_OVER_2G */
	if (ptFhdl->usMode & GRP_FS_OPEN_APPEND)	/* append mode */
		ptFhdl->iOffset = ptFile->iSize;		/* set to end of file */
	if (iSize > 0 && ptFhdl->iOffset + iSize < 0) /* over 2G */
		iSize = 0x7fffffff - ptFhdl->iOffset;	/* adjust size within 2G */
#endif /* GRP_FS_ENABLE_OVER_2G */
	for (iTotal = 0; iTotal < iSize; iTotal += iWrite) {
#ifdef GRP_FS_ENABLE_OVER_2G
		uiFsBlk = (ptFhdl->uiOffset >> iBlkShift); 	/* FS block number */
		uiBlkOff = (ptFhdl->uiOffset & (iBlkSize - 1));/* FS block offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
		uiFsBlk = (ptFhdl->iOffset >> iBlkShift); 	/* FS block number */
		uiBlkOff = (ptFhdl->iOffset & (iBlkSize - 1));/* FS block offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
		grp_fs_block_file_op(ptFile);			/* block file operation */
		iWrite = ptFs->ptFsOp->pfnWrite(ptFile, uiFsBlk, uiBlkOff,
								pucBuf, iSize - iTotal,
								(int)(ptFhdl->usMode & GRP_FS_OPEN_DIRECT_IO));
												/* write data */
		grp_fs_unblock_file_op(ptFile);			/* unblock file operation */
		if (iWrite <= 0)  {						/* write error */
			if ((iWrite == GRP_FS_ERR_NO_SPACE && iTotal)
				|| iWrite == 0)					/* no space */
				break;							/* end of write */
			iRet = (int)iWrite;					/* set error number */
			goto err_ret;						/* return error */
		}
		pucBuf += iWrite;						/* advance buffer pointer */
#ifdef GRP_FS_ENABLE_OVER_2G
		ptFhdl->uiOffset += iWrite;				/* advance offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptFhdl->iOffset += iWrite;				/* advance offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iTotal);

err_ret:
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return((grp_isize_t)iRet);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_lseek												*/
/*																			*/
/* DESCRIPTION:	Change current I/O position of opened file					*/
/* INPUT:		iFhdl:				file handle number						*/
/* 				iOffset:			offset to seek							*/
/*				iMode:				seek mode								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_ERR_BAD_PARAM: bad iMode parameter					*/
/*				GRP_FS_ERR_SHOULD_CLOSE: need to close the file				*/
/*				0 or positive:		current seek position					*/
/*																			*/
/****************************************************************************/
grp_ioffset_t
grp_fs_lseek(
	int				iFhdl,				/* [IN]  file handle number */
	grp_ioffset_t	iOffset,			/* [IN]  offset to seek */
	int				iMode)				/* [IN]  seek mode */
{
	grp_fs_fhdl_t	*ptFhdl;					/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	grp_ioffset_t	iRet;						/* return value */

	/****************************************************/
	/* check file handle								*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	iRet = _grp_fs_check_fhdl(iFhdl, 0, &ptFhdl);/* check hdl */
	if (iRet == 0) {							/* valid file handle */
		if (NULL == ptFhdl->ptFile) {			/* no file */
			iRet = GRP_FS_ERR_SHOULD_CLOSE;		/* should close */
			goto err_out;						/* return error */
		}
		switch(iMode) {
		case GRP_FS_SEEK_SET:					/* absolute */
			break;								/* use as it is */
		case GRP_FS_SEEK_CUR:					/* relative to current */
#ifdef GRP_FS_ENABLE_OVER_2G
			if (ptFhdl->usMode & GRP_FS_OPEN_APPEND)	/* append mode */
				ptFhdl->uiOffset = ptFhdl->ptFile->uiSize;/* adjust offset */
			iOffset = ptFhdl->uiOffset + iOffset;/* add current */
#else  /* GRP_FS_ENABLE_OVER_2G */
			if (ptFhdl->usMode & GRP_FS_OPEN_APPEND)	/* append mode */
				ptFhdl->iOffset = ptFhdl->ptFile->iSize;/* adjust offset */
			iOffset = ptFhdl->iOffset + iOffset;/* add current */
#endif /* GRP_FS_ENABLE_OVER_2G */
			break;
		case GRP_FS_SEEK_END:					/* realtive to file end */
#ifdef GRP_FS_ENABLE_OVER_2G
			iOffset = ptFhdl->ptFile->uiSize + iOffset; /* add file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
			iOffset = ptFhdl->ptFile->iSize + iOffset; /* add file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
			break;
		default:			 					/* invalide iMode */
			iRet = GRP_FS_ERR_BAD_PARAM;		/* invalid param */
			break;
		}
		if (iOffset < 0)						/* negative offset */
			iOffset = 0;						/* reset to 0 */
		if (iRet == 0) {						/* no error */
#ifdef GRP_FS_ENABLE_OVER_2G
			ptFhdl->uiOffset = iOffset;			/* set file offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
			ptFhdl->iOffset = iOffset;			/* set file offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
			iRet = iOffset;						/* set return value */
		}
	}
err_out:
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);								/* return result */
}

#ifdef GRP_FS_ENABLE_OVER_2G
/****************************************************************************/
/* FUNCTION:	grp_fs_lseek4G												*/
/*																			*/
/* DESCRIPTION:	Change current I/O position of opened file					*/
/* INPUT:		iFhdl:				file handle number						*/
/* 				uiOffset:			offset to seek							*/
/*				iMode:				seek mode								*/
/* OUTPUT:		puiResultOffset		current seek position					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_ERR_BAD_PARAM: bad iMode parameter					*/
/*				0 :					success to change position				*/
/*																			*/
/****************************************************************************/
int
grp_fs_lseek4G(
	int				iFhdl,				/* [IN]  file handle number */
	grp_uioffset_t	uiOffset,			/* [IN]  offset to seek */
	int				iMode,				/* [IN]  seek mode */
	grp_uioffset_t	*puiResultOffset )	/* [IN]  current seek position*/
{
	grp_fs_fhdl_t	*ptFhdl;					/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	int  			iRet;						/* return value */
	int				iSkMode;					/* seek mode */

	/****************************************************/
	/* check file handle								*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	iRet = _grp_fs_check_fhdl(iFhdl, 0, &ptFhdl); /* check hdl */
	if (iRet == 0) {							/* valid file handle */
		iSkMode = (iMode & 0xFF);

		switch(iSkMode) {
		case GRP_FS_SEEK_SET:					/* absolute */
			if ((iMode & GRP_FS_SEEK_MINUS) &&
				(0 != uiOffset)) {				/* back */
				iRet = GRP_FS_ERR_BAD_OFF;		/* under flow */
			}
			break;								/* use as it is */
		case GRP_FS_SEEK_CUR:					/* relative to current */
			if (ptFhdl->usMode & GRP_FS_OPEN_APPEND)	/* append mode */
				ptFhdl->uiOffset = ptFhdl->ptFile->uiSize;/* adjust offset */
			if (iMode & GRP_FS_SEEK_MINUS){		/* back */
				uiOffset = (grp_uioffset_t)(ptFhdl->uiOffset) - uiOffset;
												/* add current */
			} else {							/* forward */
				uiOffset = (grp_uioffset_t)(ptFhdl->uiOffset) + uiOffset;
												/* add current */
			}
			if ((iMode & GRP_FS_SEEK_MINUS) &&
				(uiOffset > (ptFhdl->uiOffset))) {
				iRet = GRP_FS_ERR_BAD_OFF;		/* under flow */
			} else if (((iMode & GRP_FS_SEEK_MINUS) == 0) &&
					   (uiOffset < (ptFhdl->uiOffset))) {
				iRet = GRP_FS_ERR_BAD_OFF;		/* over flow */
			}
			break;
		case GRP_FS_SEEK_END:					/* realtive to file end */
			if  (iMode & GRP_FS_SEEK_MINUS){	/* back */
				uiOffset = ptFhdl->ptFile->uiSize - uiOffset;  /* add file size */
			} else {							/* forward */
				uiOffset = ptFhdl->ptFile->uiSize + uiOffset;  /* add file size */
			}
			if ((iMode & GRP_FS_SEEK_MINUS) &&
				(uiOffset > (ptFhdl->ptFile->uiSize))) {
				iRet = GRP_FS_ERR_BAD_OFF;		/* under flow */
			} else if (((iMode & GRP_FS_SEEK_MINUS) == 0) &&
					   (uiOffset < (ptFhdl->ptFile->uiSize))) {
				iRet = GRP_FS_ERR_BAD_OFF;		/* over flow */
			}
			break;
		default:			 					/* invalide iMode */
			iRet = GRP_FS_ERR_BAD_PARAM;		/* invalid param */
			break;
		}
		
		if (iRet == 0) {						/* no error */
			ptFhdl->uiOffset = uiOffset;		/* set file offset */
			*puiResultOffset = uiOffset;		/* set file offset */
		}
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);								/* return result */
}
#endif /* GRP_FS_ENABLE_OVER_2G */

/****************************************************************************/
/* FUNCTION:	_grp_fs_open_internal										*/
/*																			*/
/* DESCRIPTION:	Open a file	 (internal function)							*/
/* INPUT:		pptFs:				file system information					*/
/*				ptDir:				directory information					*/
/*				ppucPath:			file name to open						*/
/*				iMode:				open mode								*/
/*				pucComp:			component buffer						*/
/* OUTPUT:		ppucPath:			path remained							*/
/*				pucComp:			last component							*/
/*				pptFile:			opened file								*/
/*				pptFs:				file system information					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_NOT_FOUND not found								*/
/*				GRP_FS_ERR_TOO_MANY	too many files/tasks					*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0 or positive value  file handle							*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_open_internal(
	grp_fs_info_t	**pptFs,				/* [IN] file system info */
	grp_fs_file_t	*ptDir,					/* [IN] directory information */
	const grp_uchar_t **ppucPath,			/* [IN/OUT] path name */
	int				iMode,					/* [IN]  open mode */
	grp_uchar_t		*pucComp,				/* [IN/OUT] component buffer */
	grp_fs_file_t	**pptFile)				/* [OUT] opened file */
{
	int				iRet = 0;				/* return value */
	grp_fs_info_t	*ptFs = *pptFs;			/* file system information */

	/****************************************************/
	/* open file										*/
	/****************************************************/
	while (ptFs != NULL
		  && (iRet = ptFs->ptFsOp->pfnOpen(ptFs, ptDir, ppucPath,
						iMode, pucComp, pptFile)) == GRP_FS_COMP_MIDDLE) {
		ptFs->iFsRef--;						/* decrement reference */
		if ((*pptFile)->usStatus & GRP_FS_FSTAT_ROOT) { /* at root */
			ptDir = ptFs->ptFsParent;		/* go up to parent */
			ptFs = ptDir->ptFs;				/* change file system */
			ptFs->iFsRef++;					/* increment reference */
		} else {							/* mount point */
			ptFs = _grp_fs_find_nest_mount(*pptFile);/* go down to nested */
			ptDir = NULL;					/* root of the file system */
		}
		grp_fs_close_file(*pptFile, GRP_FS_FILE_UNBLOCK); /* release file */
	}
	if (ptFs == NULL)						/* not FS info */
		iRet = GRP_FS_ERR_FS;				/* set error */
	*pptFs = ptFs;							/* set current FS info */
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_mount												*/
/*																			*/
/* DESCRIPTION:	Mount a file system											*/
/* INPUT:		pcDevName:			device name to mount					*/
/*				pucMP:				mount point								*/
/*				pcFsType:			file system type						*/
/*				iMode:				mount mode								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/* 				GRP_FS_ERR_BAD_DEV	bad device name							*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_TOO_MANY	too many mounts							*/
/*				GRP_FS_ERR_EXIST	already mounted							*/
/*				GRP_FS_ERR_BUSY		mount/umount in progress				*/
/*				GRP_FS_ERR_TOO_LONG	too long mount point name				*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0 					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_mount(
	const char		*pcDevName,				/* [IN]  device name */
	const grp_uchar_t *pucMp,				/* [IN]  mount point */
	const char		*pcFsType,				/* [IN]  file system type */
	int				iMode)					/* [IN]  mount mode */
{
	int				iDev;					/* device number */
	int				iMajor;					/* major device number */
	grp_fs_type_tbl_t *ptFsTbl;				/* FS type table */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptFs = NULL;			/* FS info */
	grp_fs_info_t	*ptMntFs;				/* mounted FS info */
	grp_fs_info_t	*ptFsIns = NULL;		/* insert point FS info */
	grp_fs_info_t	*ptNextFs;				/* next FS */
	int				iRet;					/* return value */
	int				iShift;					/* shift count */
	grp_uchar_t		aucComp[GRP_FS_MAX_COMP];/* component name */
	grp_uchar_t		aucPath[GRP_FS_MOUNT_PATH];/* path name */

	/****************************************************/
	/* get mount path name to local area				*/
	/****************************************************/
	iRet = grp_fs_get_str(aucPath, pucMp, sizeof(aucPath));	/* get path */
	if (iRet != 0)								/* error detected */
		return(iRet);							/* return error */
	pucMp = aucPath;							/* switch to local name */

	/****************************************************/
	/* lookup device and file system type name			*/
	/****************************************************/
	if ((iDev = grp_fs_lookup_dev(pcDevName)) < 0)	/* bad device */
		return(iDev);							/* return error */
	iRet = _grp_fs_lookup_fs_type(pcFsType, &ptFsTbl);/* lookup FS type */
	if (iRet != 0)								/* error detected */
		return(iRet);							/* return error */

	/****************************************************/
	/* check mount/umount in progress					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	if ((iRet = _grp_fs_wait_mount(1)) != 0)	/* mount/umount in progress */
		goto err_out_busy;						/* return error */

	/****************************************************/
	/* check free FS information						*/
	/****************************************************/
	if (ptFsCtl->ptFsFree == NULL) {			/* no free FS info */
		iRet = GRP_FS_ERR_TOO_MANY;				/* too many */
		goto err_out;							/* return error */
	}
	ptFs = ptFsCtl->ptFsFree;					/* get free FS info */
	ptFsCtl->ptFsFree = ptFs->ptFsOtherFwd;		/* remove from free list */
	ptFs->iDev = iDev;							/* set device number */
	if (iMode & GRP_FS_RONLY) {					/* read only mount */
		iMode &= (GRP_FS_RONLY | GRP_FS_FORCE_MOUNT); /* reset flags */
	}
	ptFs->usStatus = (grp_ushort_t)((iMode & GRP_FS_RONLY)?
						GRP_FS_STAT_RONLY: 0);	/* set status */
	if (iMode & GRP_FS_SYNC_ALL)				/* sync write always */
		ptFs->usStatus |= GRP_FS_STAT_SYNC_ALL;	/* set sync write bit */
	else if (iMode & GRP_FS_SYNC_FL_CLOSE)		/* sync write on each close */
		ptFs->usStatus |= GRP_FS_STAT_SYNC_FL_CLOSE;/* set sync close bit */
	else if (iMode & GRP_FS_SYNC_FS_CLOSE)		/* sync write on last close */
		ptFs->usStatus |= GRP_FS_STAT_SYNC_FS_CLOSE;/* set sync close bit */
	if (iMode & GRP_FS_NO_UPD_ACCTIME)			/* no update media acc time */
		ptFs->usStatus |= GRP_FS_STAT_NO_UPD_ACCTIME; /* set no update bit */
	if (iMode & GRP_FS_NO_MNT_FLAG)				/* no mount flag on media */
		ptFs->usStatus |= GRP_FS_STAT_NO_MNT_FLAG;	/* set no mount flag bit */
	if (iMode & GRP_FS_NO_CRT_ACCTIME) {		/* no media creat/access time */
		ptFs->usStatus |= 
				(GRP_FS_STAT_NO_CRT_ACCTIME|GRP_FS_STAT_NO_UPD_ACCTIME);
												/* set no time bit */
	}
	ptFs->ptFsParent = NULL;					/* no parent file */
	ptFs->iFsRef = 0;							/* reference count */
	ptFs->iFsOpen = 0;							/* open count */
	ptFs->ptFsTbl = ptFsTbl;					/* file type table */
	ptFs->ptFsOp = ptFsTbl->ptFsOp;				/* set FS operation */
	ptFs->ptFsNest = NULL;						/* clear nest link */

	/****************************************************/
	/* check the same device already mounted			*/
	/****************************************************/
	if (_grp_fs_check_mnt_dev_busy(ptFsCtl->ptFsMnt, iDev)) {
		iRet = GRP_FS_ERR_EXIST;				/* already mounted error */
		goto err_out;							/* return error */
	}

	/****************************************************/
	/* check the same mount point already used			*/
	/****************************************************/
	ptMntFs = NULL;								/* no mount point */
	ptNextFs = ptFsCtl->ptFsMnt;				/* lookup from root */
	while (*pucMp) {							/* until not end of path */
		ptNextFs = _grp_fs_lookup_mount(ptNextFs, &pucMp, &ptFsIns);
												/* search nested mount */
		if (ptNextFs == NULL)					/* no same mount */
			break;								/* search end */
		ptMntFs = ptNextFs;						/* search next level */
		ptNextFs = ptMntFs->ptFsNest;			/* go inside */
	}
	if (*pucMp == 0) {							/* already mounted */
		iRet = GRP_FS_ERR_EXIST;				/* set error number */
		goto err_out;							/* return error */
	}
	ptFs->sPathLen = (short)strlen((char *)pucMp);/* path length */
	if (ptFs->sPathLen >= GRP_FS_MOUNT_COMP) {	/* too long mount path */
		iRet = GRP_FS_ERR_TOO_LONG;				/* set error number */
		goto err_out;							/* return error */
	}
	strcpy((char *)ptFs->aucPath, (char *)pucMp);/* copy mount path */

	/****************************************************/
	/* open mount point if necessary					*/
	/****************************************************/
	if (ptMntFs != NULL) {						/* mount non root FS */
		/****************************************************/
		/* open mounted point								*/
		/****************************************************/
		ptMntFs->iFsRef++;						/* increment reference */
		iRet = _grp_fs_open_internal(&ptMntFs, NULL, &pucMp, GRP_FS_OPEN_READ,
					aucComp, &ptFs->ptFsParent); /* open file */
		if (ptMntFs != NULL)					/* mount not NULL */
			ptMntFs->iFsRef--;					/* decrement reference */
		if (iRet != 0) 							/* error in open */
			goto err_out;						/* return error */
		grp_fs_unblock_file_op(ptFs->ptFsParent);/* release lock */
		if (ptFs->ptFsParent->ucType != GRP_FS_FILE_DIR) {	/* not directory */
			iRet = GRP_FS_ERR_BAD_DIR;			/* not directory error */
			goto err_out;						/* return error */
		}
		if (ptFs->ptFsParent->iRefCnt != 1) {	/* have other reference */
			iRet = GRP_FS_ERR_BUSY;				/* busy error */
			goto err_out;						/* return error */
		}
	} else {									/* mount as root */
		/****************************************************/
		/* check path is "/" or "\" or "XXX:"				*/
		/****************************************************/
		if (ptFs->sPathLen <= 0					/* null path */
			|| (((pucMp[0] != '/' && pucMp[0] != '\\') || pucMp[1])
				&& pucMp[ptFs->sPathLen - 1] != ':')) {
			iRet = GRP_FS_ERR_BAD_PARAM;		/* bad parameter */
			goto err_out;						/* return error */
		}
	}

	/****************************************************/
	/* open mount device								*/
	/****************************************************/
	iMajor = GRP_FS_DEV_MAJOR(iDev);			/* major device number */
	iRet = grp_fs_dev_tbl[iMajor].ptOp->pfnOpen(iDev,
				(iMode & GRP_FS_RONLY) == 0,
				&ptFs->iDevHandle,
				&ptFs->uiDevOff,
				&ptFs->uiDevSize, &iShift);		/* open device */
	if (iRet != 0)								/* error in open */
		goto err_out;							/* return error */
	ptFs->ucDevBlkShift = (grp_uchar_t)iShift;	/* set device shift */

	/****************************************************/
	/* enque FS information in mount list				*/
	/****************************************************/
	ptFs->pvFsInfo = NULL;						/* no FS dependent info */
	iRet = ptFs->ptFsOp->pfnMount(ptFs, iMode);	/* mount file system */
	if (iRet != 0) {							/* error in FS mount */
		grp_fs_dev_tbl[iMajor].ptOp->pfnClose(ptFs->iDevHandle, iDev);
												/* close device */
		goto err_out;							/* return error */
	}
	if (ptFs->ptFsParent)						/* parent exists */
		ptFs->ptFsParent->usStatus |= GRP_FS_FSTAT_MOUNT; /* set mount flag */

	/****************************************************/
	/* enque FS information in mount list				*/
	/****************************************************/
	if (ptMntFs != NULL) {						/* mount non root FS */
		if (ptFsIns == NULL) {					/* insert head */
			grp_enque_shead(&ptMntFs->ptFsNest, ptFs, ptFsOther);
		} else {								/* insert next */
			grp_enque_snext(&ptMntFs->ptFsNest, ptFsIns, ptFs, ptFsOther)
		}
	} else {									/* mount root FS */
		if (ptFsIns == NULL) {					/* insert head */
			grp_enque_shead(&ptFsCtl->ptFsMnt, ptFs, ptFsOther);
		} else {								/* insert next */
			grp_enque_snext(&ptFsCtl->ptFsMnt, ptFsIns, ptFs, ptFsOther)
		}
	}
	if (ptFsCtl->ptFsDefault == NULL) 			/* no default FS */
		ptFsCtl->ptFsDefault = ptFs;			/* set it as default */

	/****************************************************/
	/* lock media if supported							*/
	/****************************************************/
	if (grp_fs_dev_tbl[iMajor].ptOp->pfnIoctl) { /* exist ioctl function */
		int		iCtl = GRP_FS_DEV_LOCK_MEDIA;	/* lock media */
		(void)grp_fs_dev_tbl[iMajor].ptOp->pfnIoctl(iDev, GRP_FS_DEV_CTL_EJECT,
										 &iCtl);/* lock media */
	}

	_grp_fs_wakeup_wait_mount();				/* wakeup wait mount */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(0);									/* return success */

err_out:
	_grp_fs_wakeup_wait_mount();				/* wakeup wait mount */
err_out_busy:
	if (ptFs) {									/* FS info extracted */
		if (ptFs->ptFsParent)					/* parent opened */
			grp_fs_close_file(ptFs->ptFsParent, 0);/* close parent file */
		ptFs->usStatus = 0;						/* clear status */
		ptFs->ptFsOtherFwd = ptFsCtl->ptFsFree; /* insert to free list */
		ptFsCtl->ptFsFree = ptFs;				/* chain it to free list */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unmount												*/
/*																			*/
/* DESCRIPTION:	Unmount a file system										*/
/* INPUT:		pcDevName:			device name to mount					*/
/*				iMode:				unmount mode							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/* 				GRP_FS_ERR_BAD_DEV	bad device name							*/
/*				GRP_FS_ERR_BUSY		mount/umount in progress				*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0 					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_unmount(
	const char		*pcDevName,				/* [IN]  device name */
	int				iMode)					/* [IN]  mount mode */
{

	int				iDev;					/* device number */
	int				iMajor;					/* major device number */
	int				iWaitMode;				/* buffer I/O wait mode */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptFs;					/* FS info */
	grp_fs_info_t	*ptFsMnt;				/* mount point FS info */
	int				iRet = 0;				/* return value */

	/****************************************************/
	/* lookup device name								*/
	/****************************************************/
	if ((iDev = grp_fs_lookup_dev(pcDevName)) < 0)	/* bad device */
		return(iDev);							/* return error */

	/****************************************************/
	/* check mount/umount in progress					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	if ((iRet = _grp_fs_wait_mount(1)) != 0)	/* mount/umount in progress */
		goto err_out_busy;						/* return error */

	/****************************************************/
	/* lookup FS info mounting the device				*/
	/****************************************************/
	ptFs = grp_fs_check_mnt_dev(ptFsCtl->ptFsMnt, iDev);
	if (ptFs == NULL) {							/* no such device mounted */
		iRet = GRP_FS_ERR_BAD_DEV;				/* bad device */
		goto err_out;							/* return error */
	}
#ifdef GRP_FS_ASYNC_UNMOUNT
	if (ptFs->iFsRef > 0) {						/* accessing now */
#else  /* GRP_FS_ASYNC_UNMOUNT */
	if (ptFsCtl->iFsWaitCnt != 0 ||				/* someone waiting in GR-FILE */
		ptFs->iFsRef > 0) {						/* accessing now */
#endif /* GRP_FS_ASYNC_UNMOUNT */
		iRet = GRP_FS_ERR_BUSY;					/* busy error */
		goto err_out;							/* return error */
	}

	/****************************************************/
	/* if force unmount, forcibly invalidate open files	*/
	/****************************************************/
	if (iMode & GRP_FS_FORCE_UMOUNT) {
		_grp_fs_invalidate_fhdl(iDev);			/* invalidate file handles */
		_grp_fs_task_invalidate_cd(iDev);		/* invalidate current dirs */
	}

	/****************************************************/
	/* check open files and invalidate file cache		*/
	/****************************************************/
	if ((iRet = _grp_fs_invalidate_file(iDev)) != 0) /* still opened */
		goto err_out;

#ifdef	GRP_FS_FNAME_CACHE
	/****************************************************/
	/* invalidate file name cache						*/
	/****************************************************/
	grp_fs_purge_fname_cache_by_dev(iDev);		/* invalidate file name cache */
#endif	/* GRP_FS_FNAME_CACHE */

	/****************************************************/
	/* wait for device I/O, and invalidate buffer cache	*/
	/****************************************************/
	iWaitMode = (iMode & GRP_FS_FORCE_UMOUNT)?
			GRP_FS_BUF_FORCE_INV: GRP_FS_BUF_WAIT_INV;
	iRet = grp_fs_wait_io(iDev, iWaitMode);		/* wait for device I/O */
	if (iRet != 0)								/* error occurred */
		goto err_out;

	/****************************************************/
	/* unmount file system								*/
	/****************************************************/
	iMode &= ~GRP_FS_REVOKE_MOUNT;				/* reset revoke flag */
	iRet = ptFs->ptFsOp->pfnUmount(ptFs, iMode);/* unmount file system */
	if (iRet != 0)								/* error in unmount */
		goto err_out;							/* return error */

	/****************************************************/
	/* wait and invalidate buffer I/O again for I/O		*/
	/* made by FS dependent umount operation			*/
	/****************************************************/
	(void) grp_fs_wait_io(iDev, GRP_FS_BUF_FORCE_INV);/* wait I/O */

	/****************************************************/
	/* close  device									*/
	/****************************************************/
	iMajor = GRP_FS_DEV_MAJOR(iDev);			/* major device number */
	grp_fs_dev_tbl[iMajor].ptOp->pfnClose(ptFs->iDevHandle, iDev);
												/* close device */

	/****************************************************/
	/* close mount point and chain FS info to free list	*/
	/****************************************************/
	if (ptFs->ptFsParent) {						/* parent FS exists */
		ptFs->ptFsParent->usStatus &= ~GRP_FS_FSTAT_MOUNT; /* reset mount flag */
		ptFsMnt = ptFs->ptFsParent->ptFs;		/* parent FS info */
		grp_fs_close_file(ptFs->ptFsParent, 0);	/* close parent file */
		grp_deque_sent(&ptFsMnt->ptFsNest, ptFs, ptFsOther);/* deque from it */
	} else {
		grp_deque_sent(&ptFsCtl->ptFsMnt, ptFs, ptFsOther);/* deque from root */
	}
	if (ptFsCtl->ptFsDefault == ptFs)			/* default FS */
		ptFsCtl->ptFsDefault = NULL;			/* clear it */
	ptFs->usStatus = 0;							/* clear status */
	ptFs->ptFsOtherFwd = ptFsCtl->ptFsFree;		/* insert in free list */
	ptFsCtl->ptFsFree = ptFs;					/* chain to free list */

	/****************************************************/
	/* unlock media if supported						*/
	/****************************************************/
	if (grp_fs_dev_tbl[iMajor].ptOp->pfnIoctl) { /* exist ioctl function */
		int		iCtl = GRP_FS_DEV_UNLOCK_MEDIA;	/* unlock media */
		(void)grp_fs_dev_tbl[iMajor].ptOp->pfnIoctl(iDev, GRP_FS_DEV_CTL_EJECT,
										 &iCtl);/* unlock media */
	}

err_out:
	_grp_fs_wakeup_wait_mount();				/* wakeup wait mount */
err_out_busy:
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_lookup_base_fs										*/
/*																			*/
/* DESCRIPTION:	Lookup base file system										*/
/* INPUT:		ppucPath:			file path name							*/
/*				iMode:				open mode								*/
/*				pptTask:			task environment						*/
/* OUTPUT:		pptFs:				base file system						*/
/*				pptDir:				current directory						*/
/*				ppucPath:			remaining path							*/
/*				pptTask:			task environment						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_TOO_MANY	too many tasks							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				0 or positive value  file handle							*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_lookup_base_fs(
	const grp_uchar_t **ppucPath,				/* [IN/OUT] path name */
	int				iMode,						/* [IN]     open mode */
	grp_fs_task_ctl_t **pptTask,				/* [IN/OUT] task environment */
	grp_fs_info_t	**pptFs,					/* [OUT]  target file system */
	grp_fs_file_t	**pptDir)					/* [OUT]  current directory */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;		/* FS control data */
	grp_fs_info_t	*ptFs;						/* file system information */
	grp_fs_file_t	*ptDir = NULL;				/* directory file */
	grp_fs_task_ctl_t *ptTask;					/* task information */
	int				iRet;						/* return value */

	/****************************************************/
	/* init return information							*/
	/****************************************************/
	*pptFs = NULL;								/* no FS info */
	*pptDir = NULL;								/* no directory */
	if (*ppucPath == NULL)						/* null path */
		return(GRP_FS_ERR_BAD_PARAM);			/* bad parameter */

	/****************************************************/
	/* get task environment	if necessary				*/
	/****************************************************/
	if (*pptTask == NULL) {
		iRet = _grp_fs_task_get_env(pptTask);	/* get task environment */
		if (iRet != 0)							/* error occured */
			return(iRet);						/* return error */
	}

	/****************************************************/
	/* check absolute path or not						*/
	/****************************************************/
	ptFs = _grp_fs_lookup_mount(ptFsCtl->ptFsMnt, ppucPath, NULL);
	if (ptFs == NULL) {							/* not absolute path */
		const grp_uchar_t *pucPath = *ppucPath;	/* path name */
		int		iRoot;							/* is path for root */
		ptTask = *pptTask;						/* task information */
		if (ptTask->ptCurDir == GRP_FS_INV_CD)	/* invalidated directory */
			return(GRP_FS_ERR_NOT_FOUND);		/* return not found error */
		iRoot = (pucPath[0] == '/' || pucPath[0] == '\\'); /* path for root */
		if (ptTask->ptCurDir == NULL) {			/* no current directory */
			if (ptFsCtl->ptFsDefault == NULL)	/* no default file system */
				return(GRP_FS_ERR_NOT_FOUND);	/* return not found error */
			ptFs = ptFsCtl->ptFsDefault;		/* use default FS */
			ptDir = NULL;						/* use root of the FS */
			if (iRoot)							/* path for root */
				*ppucPath = &pucPath[1];		/* advance path pointer */
		} else {								/* has current directory */
			ptDir = ptTask->ptCurDir;			/* use current directory */
			ptFs = ptDir->ptFs;					/* set FS pointer */
			if (iRoot) {						/* path for root */
				ptDir = NULL;					/* use root of the FS */
				*ppucPath = &pucPath[1];		/* advance path pointer */
			}
		}
	}
	if ((ptFs->usStatus & GRP_FS_STAT_RONLY)	/* read only file system */
		&& (iMode & GRP_FS_OPEN_WRITE))			/* write open request */
		return(GRP_FS_ERR_PERMIT);				/* permission denied error */
	*pptFs = ptFs;								/* return FS */
	*pptDir = ptDir;							/* return current directory */
	return(0);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_file_open_common										*/
/*																			*/
/* DESCRIPTION:	Common routine for open file								*/
/*				Note: Opened file is returned with file operation blocked.	*/
/* INPUT:		ptFs:		file system information							*/
/*				ptDir:		search start directory							*/
/*				ppucPath:	path name										*/
/*				iMode:		open mode										*/
/* OUTPUT:		pptOpened:	opened file information							*/
/*				ppucPath:	remaining path									*/
/*				pucComp:	last component 									*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_BAD_NAME: bad file name 							*/
/*				GRP_FS_ERR_NOT_FOUND:file not found							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				GRP_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_COMP_MIDDLE:	search not complete due to cross FS		*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_file_open_common(
	grp_fs_info_t		*ptFs,				/* [IN]  file system information */
	grp_fs_file_t		*ptDir,				/* [IN]  search start directory */
	const grp_uchar_t	**ppucPath,			/* [IN/OUT]  path name */
	int					iMode,				/* [IN]  open mode */
	grp_uchar_t			*pucComp,			/* [OUT] last component */
	grp_fs_file_t		**pptOpened)		/* [OUT] opened file information */
{
	grp_fs_file_t		*ptFile = NULL;		/* opened file information */
	const grp_uchar_t	*pucPrev;			/* previous component pointer */
	int					iRet;				/* return value */
	int					iMatch;				/* return value of match */

	/****************************************************/
	/* get current directory file						*/
	/****************************************************/
	if (ptDir == NULL) {						/* no current directory */
		iRet = ptFs->ptFsOp->pfnOpenRoot(ptFs, &ptDir);
												/* get root derectory */
		if (iRet != 0)							/* error occured */
			goto out;							/* return error */
	} else {									/* current directory exists */
		if (ptDir->ucType != GRP_FS_FILE_DIR) {	 /* not directory */
			iRet = GRP_FS_ERR_BAD_DIR;			/* not directory error */
			goto out;
		}
		grp_fs_block_file_op(ptDir);			/* block file operation */
		ptDir->iRefCnt++;						/* increment reference */
	}
	ptFile = ptDir;								/* set ptFile for NULL path */

	/****************************************************/
	/* open specified file								*/
	/****************************************************/
	for (pucPrev = *ppucPath;
		 (iRet = grp_fs_get_path_comp(ppucPath, pucComp, GRP_FS_MAX_COMP)) > 0;
		 pucPrev = *ppucPath) {
		/****************************************************/
		/* for parent request, stop at last component		*/
		/****************************************************/
		if ((iMode & GRP_FS_OPEN_PARENT) && (iRet == GRP_FS_COMP_LAST))
			break;								/* stop stop search */

		/****************************************************/
		/* check special cases								*/
		/****************************************************/
		if (strcmp((char *)pucComp, ".") == 0)	/* dot */
			continue;							/* current directory */
		if (strcmp((char *)pucComp, "..") == 0
			&& (ptDir->usStatus & GRP_FS_FSTAT_ROOT)) { /* ".." at root */
			if (ptFs->ptFsParent == NULL)		/* no parent file system */
				continue;						/* ignore it */
			*ppucPath = pucPrev;				/* roll back to ".." */
			iRet = GRP_FS_COMP_MIDDLE;			/* middle of component */
			break;								/* stop search */
		}
		grp_fs_set_access_time(ptDir);			/* set access time */
		iMatch = ptFs->ptFsOp->pfnMatchComp(ptDir, pucComp, 0, &ptFile, 0,
											NULL);/* match component */
		if (iMatch != 0) {						/* error */
			iRet = iMatch;						/* set error number */
			break;								/* stop search */
		}
		if (ptFile->usStatus & GRP_FS_FSTAT_MOUNT) { /* at mount point */
			iRet = GRP_FS_COMP_MIDDLE;			/* middle of component */
			break;								/* stop search */
		}
		if (iRet == GRP_FS_COMP_LAST)			/* last component */
			break;
		ptDir = ptFile;							/* advance to next component */
		if (ptDir->ucType != GRP_FS_FILE_DIR) {	/* not directory */
			iRet = GRP_FS_ERR_NOT_FOUND;		/* not found */
			break;								/* stop search */
		}
		if ((ptDir->uiProtect & GRP_FS_PROT_XUSR) == 0) { /* no searchable */
			iRet = GRP_FS_ERR_PERMIT;			/* permission denied */
			break;								/* stop search */
		}
	}
	if (iRet == 0 || iRet == GRP_FS_COMP_LAST) { /* found file */
		/****************************************************/
		/* check permission									*/
		/****************************************************/
		if (((iMode & GRP_FS_OPEN_WRITE)
				&& ((ptFile->uiProtect & GRP_FS_PROT_WUSR) == 0
					|| (ptFs->usStatus & GRP_FS_STAT_RONLY)
					|| (ptFile->ucType == GRP_FS_FILE_DIR
						&& (iMode & GRP_FS_OPEN_PARENT) == 0)
					|| ((ptFile->usStatus & GRP_FS_FSTAT_NO_UPD_TIME)
						&& (iMode & GRP_FS_OPEN_PARENT) == 0)))
			|| ((iMode & GRP_FS_OPEN_READ)
				&& (ptFile->uiProtect & GRP_FS_PROT_RUSR) == 0)
			|| ((iMode & GRP_FS_OPEN_EXEC)
				&& (ptFile->uiProtect & GRP_FS_PROT_XUSR) == 0)) {
			iRet = GRP_FS_ERR_PERMIT;			/* permission denied */
		} else if (iRet == 0) {					/* null name */
			if (iMode & GRP_FS_OPEN_PARENT)		/* rename/unlink root/current */
				iRet = GRP_FS_ERR_BUSY;			/* return busy error */
		} else									/* end of component */
			iRet = 0;							/* success */
		if (iRet != 0) {						/* error */
			grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* close file */
			ptFile = NULL;						/* no file */
		}
	} else if (iRet < 0) {						/* error */
		ptFile = NULL;							/* no opened file */
		grp_fs_close_file(ptDir, GRP_FS_FILE_UNBLOCK); /* close directory */
	}
out:
	*pptOpened = ptFile;						/* set opened file */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_create_internal										*/
/*																			*/
/* DESCRIPTION:	Create a file (internal function)							*/
/* INPUT:		pucPath:			file name to open						*/
/*				iMode:				open mode								*/
/*				uiProtect:			create protection						*/
/*				pucComp:			component buffer						*/
/*				pptTask:			task control information				*/
/* OUTPUT:		pptFile:			created file							*/
/*				pptTask:			task control information				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_NOT_FOUND not found								*/
/*				GRP_FS_ERR_TOO_MANY	too many files/tasks					*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				GRP_FS_ERR_EXIST	already exist							*/
/*				0 or positive value  file handle							*/
/*																			*/
/****************************************************************************/
int
_grp_fs_create_internal(
	const grp_uchar_t *pucPath,				/* [IN]  file name to open */
	grp_uint32_t	uiType,					/* [IN]  type of file */
	grp_uint32_t	uiProtect,				/* [IN]  file protection */
	grp_uint32_t	uiAttr,					/* [IN]  FS dependent attribute */
	grp_uchar_t		*pucComp,				/* [IN]  component buffer */
	grp_fs_task_ctl_t **pptTask,			/* [IN/OUT] task control info */
	grp_fs_file_t	**pptFile)				/* [OUT] created file */
{
	grp_fs_file_t	*ptFile = NULL;			/* file pointer */
	grp_fs_file_t	*ptDir;					/* directory file pointer */
	grp_fs_info_t	*ptFs;					/* FS info */
	int				iRet = 0;				/* return value */

	/****************************************************/
	/* get base file system information					*/
	/****************************************************/
	iRet = _grp_fs_lookup_base_fs(&pucPath, GRP_FS_OPEN_WRITE, pptTask,
								&ptFs, &ptDir);
	if (iRet != 0)							/* error occured */
		return(iRet);						/* return error */
	ptFs->iFsRef++;							/* increment reference */

	/****************************************************/
	/* create file										*/
	/****************************************************/
	while (ptFs != NULL
		  && (iRet = ptFs->ptFsOp->pfnCreate(ptFs, ptDir, &pucPath, uiType,
				uiProtect, uiAttr, pucComp, &ptFile)) == GRP_FS_COMP_MIDDLE) {
		ptFs->iFsRef--;						/* decrement reference */
		if (ptFile->usStatus & GRP_FS_FSTAT_ROOT) { /* at root */
			ptDir = ptFs->ptFsParent;		/* go up to parent */
			ptFs = ptDir->ptFs;				/* change file system */
			ptFs->iFsRef++;					/* increment reference */
		} else {							/* mount point */
			ptFs = _grp_fs_find_nest_mount(ptFile);/* go down to nested mount */
			ptDir = NULL;					/* root of the file system */
		}
		grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* release file */
	}
	if (ptFs == NULL)						/* no FS info */
		iRet = GRP_FS_ERR_FS;				/* set error number */
	if (iRet == 0) {						/* create success */
		if (pptFile)						/* need file opened */
			*pptFile = ptFile;				/* set it */
		else								/* just create */
			grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);/* close file */
	} else if (iRet == GRP_FS_ERR_EXIST) {	/* exist file */
		if (ptFile) {						/* opened file */
			if (pptFile) {					/* need file opened */
				*pptFile = ptFile;			/* set it */
			} else {						/* need not file opened */
				grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);/* close file */
			}
		}
	}
	if (ptFs)								/* FS info exists */
		ptFs->iFsRef--;						/* decrement reference */
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_open													*/
/*																			*/
/* DESCRIPTION:	Open a file													*/
/* INPUT:		pucPath:			file name to open						*/
/*				iMode:				open mode								*/
/*				uiProtect:			create protection						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_NOT_FOUND not found								*/
/*				GRP_FS_ERR_TOO_MANY	too many files/tasks					*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_EXIST	file exists (GRP_FS_O_EXCL)				*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0 or positive value  file handle							*/
/*																			*/
/****************************************************************************/
int
grp_fs_open(
	const grp_uchar_t *pucPath,				/* [IN]  file name to open */
	int				iMode,					/* [IN]  open mode */
	grp_uint32_t	uiProtect)				/* [IN]  create protection */
{
	grp_fs_file_t	*ptDir;					/* directory file pointer */
	grp_fs_fhdl_t	*ptFhdl;				/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_file_t	*ptFile = NULL;			/* file information */
	grp_fs_info_t	*ptFs = NULL;			/* FS info */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task environment */
	const grp_uchar_t *pucOpenPath;			/* open path name */
	int				iOpenMode;				/* internal open mode */
	int				iRet;					/* return value */
	grp_uchar_t		aucComp[GRP_FS_MAX_COMP];/* component name */
	grp_uchar_t		aucPath[GRP_FS_MAX_PATH];/* path name */

	/****************************************************/
	/* get path name to local area						*/
	/****************************************************/
	iRet = grp_fs_get_str(aucPath, pucPath, sizeof(aucPath));	/* get path */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pucPath = aucPath;						/* switch to local name */

	/****************************************************/
	/* check open mode									*/
	/****************************************************/
	switch(iMode & GRP_FS_O_ACCMODE) {		/* access mode part */
	case GRP_FS_O_RDONLY:					/* read only */
		iOpenMode = GRP_FS_OPEN_READ;		/* read only */
		break;
	case GRP_FS_O_WRONLY:					/* write only */
		iOpenMode = GRP_FS_OPEN_WRITE;		/* write only */
		break;
	case GRP_FS_O_RDWR:						/* read and write */
		iOpenMode = (GRP_FS_OPEN_READ|GRP_FS_OPEN_WRITE); /* read and write */
		break;
	default:								/* others */
		return(GRP_FS_ERR_BAD_MODE);		/* return error */
	}
	if (iMode & GRP_FS_O_APPEND)			/* append mode */
		iOpenMode |= GRP_FS_OPEN_APPEND;	/* set append mode */
	if (iMode & GRP_FS_O_DIRECT_IO)			/* direct I/O mode */
		iOpenMode |= GRP_FS_OPEN_DIRECT_IO;	/* set direct I/O mode */
	if (iMode & GRP_FS_O_TRUNC)				/* truncate mode */
		iOpenMode |= GRP_FS_OPEN_WRITE;		/* write mode */
	uiProtect &= GRP_FS_PROT_RWXA;			/* mask protection bits */

	/****************************************************/
	/* wait for mount/umount operation					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait for mount/umount */

	/****************************************************/
	/* check free file handle							*/
	/****************************************************/
	if ((ptFhdl = ptFsCtl->ptFhdlFree) == NULL)	{ /* no free file handle */
		iRet = GRP_FS_ERR_TOO_MANY;			/* set error number */
		goto err_out;						/* return error */
	}
	ptFsCtl->ptFhdlFree = grp_fs_fhdl_list(ptFhdl);/* remove from free list */

	/****************************************************/
	/* get base file system information					*/
	/****************************************************/
	pucOpenPath = pucPath;					/* set path name */
	iRet = _grp_fs_lookup_base_fs(&pucOpenPath, iOpenMode, &ptTask,
								&ptFs, &ptDir);	/* get base file system */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	ptFs->iFsRef++;							/* increment reference */

	/****************************************************/
	/* open file										*/
	/****************************************************/
	if (iMode & GRP_FS_O_CREAT) {			/* create file */
		iRet = _grp_fs_create_internal(pucPath, GRP_FS_FILE_FILE, uiProtect,
										0, aucComp, &ptTask, &ptFile);
		if (iRet == 0) {					/* successfully created */
			iMode &= ~GRP_FS_O_TRUNC;		/* reset truncate flag */
		} else if (iRet == GRP_FS_ERR_EXIST) {	/* file exists */
			if (ptFile == NULL)				/* no file opened */
				goto err_out;				/* return error */
			if (((iOpenMode & GRP_FS_OPEN_WRITE)
					&& ((ptFile->uiProtect & GRP_FS_PROT_WUSR) == 0
						|| (ptFs->usStatus & GRP_FS_STAT_RONLY)
						|| (ptFile->ucType == GRP_FS_FILE_DIR
							&& (iOpenMode & GRP_FS_OPEN_PARENT) == 0)
						|| ((ptFile->usStatus & GRP_FS_FSTAT_NO_UPD_TIME)
							&& (iOpenMode & GRP_FS_OPEN_PARENT) == 0)))
				|| ((iOpenMode & GRP_FS_OPEN_READ)
					&& (ptFile->uiProtect & GRP_FS_PROT_RUSR) == 0)
				|| ((iOpenMode & GRP_FS_OPEN_EXEC)
					&& (ptFile->uiProtect & GRP_FS_PROT_XUSR) == 0)) {
				grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);
				iRet = GRP_FS_ERR_PERMIT;	/* permission denied */
				goto err_out;				/* return error */
			}
			if (iMode & GRP_FS_O_EXCL) {	/* exlusive create */
				grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);
											/* close the file */
				goto err_out;				/* return error */
			}
			iRet = 0;						/* reset error */
		}
	} else {								/* not create file */
		iRet = _grp_fs_open_internal(&ptFs, ptDir, &pucOpenPath, iOpenMode,
					aucComp, &ptFile);		 /* open file */
	}
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	if (iMode & GRP_FS_O_TRUNC) {			/* need to truncate */
		grp_fs_info_t *ptTgtFs = ptFile->ptFs; /* get FS information again */
		if (grp_fs_check_io_status(ptTgtFs, 0, 0, GRP_FS_IO_REQ) != 0)
			iRet = GRP_FS_ERR_IO;			/* I/O error */
		else
			iRet = ptTgtFs->ptFsOp->pfnTruncate(ptFile, 0, 0);
											/* truncate file */
		if (iRet != 0) {					/* truncate failed */
			grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);/* close the file */
			goto err_out;					/* return error */
		}
	}
	grp_fs_set_access_time(ptFile);			/* set access time */
	grp_fs_unblock_file_op(ptFile);			/* unblock file operation */
	ptFhdl->ptFile = ptFile;				/* file information */
#ifdef GRP_FS_ENABLE_OVER_2G
	ptFhdl->uiOffset = (iMode & GRP_FS_O_APPEND)? ptFile->uiSize: 0;
											/* set offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
	ptFhdl->iOffset = (iMode & GRP_FS_O_APPEND)? ptFile->iSize: 0;
											/* set offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
	ptFhdl->ptTask = ptTask;				/* set task environment */
	ptFhdl->usMode = (grp_ushort_t)iOpenMode;/* set mode */
	ptTask->iOpenCnt++;						/* increment open count */
	ptFs->iFsRef--;							/* decrement reference */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return((int)ptFhdl->sHdlId);			/* return handle Id */

err_out:
	if (ptTask)								/* task environment exist */
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
	if (ptFs)								/* file system referenced */
		ptFs->iFsRef--;						/* decrement reference */
	if (ptFhdl) {							/* allocated file handle */
		grp_fs_fhdl_list(ptFhdl) = ptFsCtl->ptFhdlFree;/* insert to free */
		ptFsCtl->ptFhdlFree = ptFhdl;		/* chain to free list */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_create												*/
/*																			*/
/* DESCRIPTION:	Create a file												*/
/* INPUT:		pucPath:			file name to create						*/
/*				uiType:				type of file							*/
/*				uiProtect:			file protection							*/
/*				uiAttr:				file system dependent attribute			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_NOT_FOUND directory not found					*/
/*				GRP_FS_ERR_EXIST	already exist							*/
/*				GRP_FS_ERR_TOO_MANY	too many files/tasks					*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_BAD_TYPE	bad file type							*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_create(
	const grp_uchar_t *pucPath,				/* [IN]  file name to open */
	grp_uint32_t	uiType,					/* [IN]  type of file */
	grp_uint32_t	uiProtect,				/* [IN]  file protection */
	grp_uint32_t	uiAttr)					/* [IN]  FS dependent attribute */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	int				iRet;					/* return value */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task environment */
	grp_uchar_t		aucComp[GRP_FS_MAX_COMP];/* component name */
	grp_uchar_t		aucPath[GRP_FS_MAX_PATH];/* path name */

	/****************************************************/
	/* get path name to local area						*/
	/****************************************************/
	iRet = grp_fs_get_str(aucPath, pucPath, sizeof(aucPath));	/* get path */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pucPath = aucPath;						/* switch to local name */

	/****************************************************/
	/* check type value									*/
	/****************************************************/
	if (uiType < GRP_FS_FILE_FILE || uiType > GRP_FS_FILE_OTHER)
		return(GRP_FS_ERR_BAD_TYPE);		/* bad type error */

	/****************************************************/
	/* wait for mount/umount operation					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait for mount/umount */

	/****************************************************/
	/* create file										*/
	/****************************************************/
	uiProtect &= GRP_FS_PROT_RWXA;			/* mask protection bits */
	iRet = _grp_fs_create_internal(pucPath, uiType, uiProtect,
										uiAttr, aucComp, &ptTask, NULL);
	if (ptTask)								/* task environment exits */
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_chdir												*/
/*																			*/
/* DESCRIPTION:	Change directory											*/
/* INPUT:		pucPath:			directory name							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOT_FOUND not found								*/
/*				GRP_FS_ERR_TOO_MANY	too many files/tasks					*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_chdir(
	const grp_uchar_t *pucPath)				/* [IN]  directory name */
{
	grp_fs_file_t	*ptDir;					/* directory file pointer */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptFs = NULL;			/* FS info */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task environment */
	grp_fs_file_t	*ptFile;				/* file information */
	int				iRet;					/* return value */
	grp_uchar_t		aucComp[GRP_FS_MAX_COMP];/* component name */
	grp_uchar_t		aucPath[GRP_FS_MAX_PATH];/* path name */

	/****************************************************/
	/* get path name to local area						*/
	/****************************************************/
	if (pucPath == NULL) {					/* invalidate current directory */
		aucPath[0] = 0;						/* set NULL path */
	} else {
		iRet = grp_fs_get_str(aucPath, pucPath, sizeof(aucPath));/* get path */
		if (iRet != 0)						/* error detected */
			return(iRet);					/* return error */
	}
	pucPath = aucPath;						/* switch to local name */

	/****************************************************/
	/* wait for mount/umount operation					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait for mount/umount */

	/****************************************************/
	/* if pucPath == "", close current diretory			*/
	/****************************************************/
	if (*pucPath == 0) {					/* close current directory */
		iRet = _grp_fs_task_get_env(&ptTask);/* get task environment */
		if (iRet != 0)						/* error occured */
			goto err_out;					/* return error */
		if (ptTask->ptCurDir == GRP_FS_INV_CD) /* invalid current directory */
			ptTask->ptCurDir = NULL;		/* reset pointer */
		if (ptTask->ptCurDir) { 			/* current directory exists */
			ptFile = ptTask->ptCurDir;		/* current directory */
			ptFs = ptFile->ptFs;			/* file system information */
			ptFs->iFsRef++;					/* increment reference */
			grp_fs_block_file_op(ptFile);	/* lock current directory */
			grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);
											/* close current directory */
			ptFs->iFsRef--;					/* decrement reference */
			ptTask->ptCurDir = NULL;		/* clear current directory */
		}
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
		goto out;							/* return */
	}

	/****************************************************/
	/* get base file system information					*/
	/****************************************************/
	iRet = _grp_fs_lookup_base_fs(&pucPath, 0, &ptTask,
								&ptFs, &ptDir);	/* get base file system */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	ptFs->iFsRef++;							/* increment reference */

	/****************************************************/
	/* open file										*/
	/****************************************************/
	iRet = _grp_fs_open_internal(&ptFs, ptDir, &pucPath, 0,
					aucComp, &ptFile);		/* open file */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	if (ptFile->ucType != GRP_FS_FILE_DIR) { /* not directory */
		iRet = GRP_FS_ERR_BAD_DIR;			/* not directory error */
		goto err_out_with_close;			/* return error */
	}
	if ((ptFile->uiProtect & GRP_FS_PROT_XUSR) == 0) { /* not searchable */
		iRet = GRP_FS_ERR_PERMIT;			/* no permitted */
		goto err_out_with_close;			/* return error */
	}
	grp_fs_set_access_time(ptFile);			/* set access time */
	grp_fs_unblock_file_op(ptFile);			/* unblock file operation */
	if (ptTask->ptCurDir
		&& ptTask->ptCurDir != GRP_FS_INV_CD) {/* current directory exists */
		grp_fs_block_file_op(ptTask->ptCurDir); /* lock current directory */
		grp_fs_close_file(ptTask->ptCurDir, GRP_FS_FILE_UNBLOCK);
											/* close current directory */
	}
	ptTask->ptCurDir = ptFile;				/* set new current directory */
	ptFs->iFsRef--;							/* decrement reference */
out:
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(0);								/* return success */

err_out_with_close:
	grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);/* close file */
err_out:
	if (ptTask)								/* task environment exist */
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
	if (ptFs)								/* file system referenced */
		ptFs->iFsRef--;						/* decrement reference */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_unlink												*/
/*																			*/
/* DESCRIPTION:	Unlink a file												*/
/* INPUT:		pucPath:			file name to unlink						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_BUSY		file busy								*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_unlink(
	const grp_uchar_t *pucPath)				/* [IN]  file name to open */
{
	grp_fs_file_t	*ptDir;					/* directory file pointer */
	grp_fs_info_t	*ptFs;					/* FS info */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task environment */
	grp_fs_file_t	*ptFile;				/* file information */
	int				iRet = 0;				/* return value */
	grp_uchar_t		aucComp[GRP_FS_MAX_COMP];/* component name */
	grp_uchar_t		aucPath[GRP_FS_MAX_PATH];/* path name */

	/****************************************************/
	/* get path name to local area						*/
	/****************************************************/
	iRet = grp_fs_get_str(aucPath, pucPath, sizeof(aucPath));	/* get path */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pucPath = aucPath;						/* switch to local name */

	/****************************************************/
	/* wait for mount/umount operation					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait for mount/umount */

	/****************************************************/
	/* get base file system information					*/
	/****************************************************/
	iRet = _grp_fs_lookup_base_fs(&pucPath, GRP_FS_OPEN_WRITE, &ptTask,
								&ptFs, &ptDir);	/* get base file system */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	ptFs->iFsRef++;							/* increment reference */

	/****************************************************/
	/* unlink file										*/
	/****************************************************/
	while (ptFs != NULL
			&& (iRet = ptFs->ptFsOp->pfnUnlink(ptFs, ptDir, &pucPath,
								aucComp, &ptFile)) == GRP_FS_COMP_MIDDLE) {
		ptFs->iFsRef--;						/* decrement reference */
		if (ptFile->usStatus & GRP_FS_FSTAT_ROOT) { /* at root */
			ptDir = ptFs->ptFsParent;		/* go up to parent */
			ptFs = ptDir->ptFs;				/* change file system */
			ptFs->iFsRef++;					/* increment reference */
		} else {							/* mount point */
			ptFs = _grp_fs_find_nest_mount(ptFile);/* go down to nested mount */
			ptDir = NULL;					/* root of the file system */
		}
		grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* release file */
	}
	if (ptFs == NULL)						/* no FS info */
		iRet = GRP_FS_ERR_FS;				/* set error number */
	else
		ptFs->iFsRef--;						/* decrement reference */

err_out:
	if (ptTask)								/* task environment exists */
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_rename												*/
/*																			*/
/* DESCRIPTION:	Rename a file												*/
/* INPUT:		pucOld:				original file name to rename			*/
/*				pucNew:				new file name							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_EXIST	already exist							*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_XFS		cross file system						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_rename(
	const grp_uchar_t *pucOld,				/* [IN]  old file name */
	const grp_uchar_t *pucNew)				/* [IN]  new file name */
{
	grp_fs_file_t	*ptOldStDir;			/* old search start directory */
	grp_fs_file_t	*ptNewStDir;			/* new search start directory */
	grp_fs_file_t	*ptOldDir = NULL;		/* old file directory */
	grp_fs_file_t	*ptNewDir = NULL;		/* new file directory */
	grp_fs_info_t	*ptOldFs = NULL;		/* FS info for old file */
	grp_fs_info_t	*ptNewFs = NULL;		/* FS info for new file */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task environment */
	int				iRet = 0;				/* return value */
	grp_uchar_t		aucOldComp[GRP_FS_MAX_COMP];/* old component name */
	grp_uchar_t		aucNewComp[GRP_FS_MAX_COMP];/* new component name */
	grp_uchar_t		aucOldPath[GRP_FS_MAX_PATH];/* old path name */
	grp_uchar_t		aucNewPath[GRP_FS_MAX_PATH];/* new path name */

	/****************************************************/
	/* get path name to local area						*/
	/****************************************************/
	iRet = grp_fs_get_str(aucOldPath, pucOld, sizeof(aucOldPath));/* get path */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pucOld = aucOldPath;					/* switch to local name */
	iRet = grp_fs_get_str(aucNewPath, pucNew, sizeof(aucNewPath));/* get path */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pucNew = aucNewPath;					/* switch to local name */

	/****************************************************/
	/* wait for mount/umount operation					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait for mount/umount */

	/****************************************************/
	/* get base file system information for old name	*/
	/****************************************************/
	iRet = _grp_fs_lookup_base_fs(&pucOld, 0, &ptTask, &ptOldFs, &ptOldStDir);
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	ptOldFs->iFsRef++;						/* increment reference */

	/****************************************************/
	/* get parent directory of old file					*/
	/****************************************************/
	iRet = _grp_fs_open_internal(&ptOldFs, ptOldStDir, &pucOld,
					GRP_FS_OPEN_WRITE|GRP_FS_OPEN_PARENT,
					aucOldComp, &ptOldDir); /* open old directory */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	grp_fs_unblock_file_op(ptOldDir);		/* release lock to avoid deadlock */

	/****************************************************/
	/* get base file system information for new name	*/
	/****************************************************/
	iRet = _grp_fs_lookup_base_fs(&pucNew, 0, &ptTask, &ptNewFs, &ptNewStDir);
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	ptNewFs->iFsRef++;						/* increment reference */

	/****************************************************/
	/* get parent directory of new file					*/
	/****************************************************/
	iRet = _grp_fs_open_internal(&ptNewFs, ptNewStDir, &pucNew,
					GRP_FS_OPEN_WRITE|GRP_FS_OPEN_PARENT,
					aucNewComp, &ptNewDir); /* open new directory */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	grp_fs_unblock_file_op(ptNewDir);		/* release lock to avoid deadlock */

	/****************************************************/
	/* check cross file system rename					*/
	/****************************************************/
	if (ptOldFs != ptNewFs) {				/* cross file system */
		iRet = GRP_FS_ERR_XFS;				/* cross file system error */
		goto err_out;
	}

	/****************************************************/
	/* rename file										*/
	/****************************************************/
	iRet = ptOldFs->ptFsOp->pfnRename(ptOldFs, ptOldDir, aucOldComp,
							ptNewDir, aucNewComp);	/* rename file */
err_out:
	if (ptOldDir)							/* old directory */
		grp_fs_close_file(ptOldDir, 0);		/* close it */
	if (ptNewDir)							/* old directory */
		grp_fs_close_file(ptNewDir, 0);		/* close it */
	if (ptOldFs)							/* referenced old FS */
		ptOldFs->iFsRef--;					/* decrement reference */
	if (ptNewFs)							/* referenced new FS */
		ptNewFs->iFsRef--;					/* decrement reference */
	if (ptTask)								/* task environment exists */
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_attr												*/
/*																			*/
/* DESCRIPTION:	Get file attribute											*/
/* INPUT:		pucPath:			file name to unlink						*/
/* OUTPUT:		ptAttr:				file attribute information				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOT_FOUND file not found							*/
/*				GRP_FS_ERR_TOO_LONG	too long file name						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: bad path name/attr parameter			*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_attr(
	const grp_uchar_t *pucPath,				/* [IN]  file name to open */
	grp_fs_dir_ent_t *ptAttr)				/* [OUT] file attribute info */
{
	grp_fs_file_t	*ptDir;					/* directory file pointer */
	grp_fs_info_t	*ptFs;					/* FS info */
	grp_fs_file_t	*ptFile;				/* file information */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task environment */
	int				iRet = 0;				/* return value */
	grp_fs_dir_ent_t tAttr;					/* attribute information */
	grp_uchar_t		aucComp[GRP_FS_MAX_COMP];/* component name */
	grp_uchar_t		aucPath[GRP_FS_MAX_PATH];/* path name */

	/****************************************************/
	/* get path name to local area						*/
	/****************************************************/
	iRet = grp_fs_get_str(aucPath, pucPath, sizeof(aucPath));	/* get path */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pucPath = aucPath;						/* switch to local name */

	/****************************************************/
	/* wait for mount/umount operation					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait for mount/umount */

	/****************************************************/
	/* get base file system information					*/
	/****************************************************/
	iRet = _grp_fs_lookup_base_fs(&pucPath, 0, &ptTask, &ptFs, &ptDir);
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	ptFs->iFsRef++;							/* increment reference */

	/****************************************************/
	/* get file attribute								*/
	/****************************************************/
	iRet = _grp_fs_open_internal(&ptFs, ptDir, &pucPath, 0,
					aucComp, &ptFile);		/* open file */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	tAttr.pucName = NULL;					/* no name buffer */
	tAttr.sNameSize = 0;					/* no name buffer */
	iRet = ptFs->ptFsOp->pfnGetAttr(ptFile, &tAttr);/* get attr */
	grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* release file */
	if (iRet == 0) {
		iRet = (int)grp_fs_copyout(ptAttr, &tAttr, sizeof(tAttr));
											/* copy attr */
		iRet = (iRet != sizeof(tAttr))? GRP_FS_ERR_BAD_PARAM: 0;
											/* set return value */
	}
err_out:
	if (ptTask)								/* task environment exits */
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
	if (ptFs)								/* FS info exists */
		ptFs->iFsRef--;						/* decrement reference */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_set_attr												*/
/*																			*/
/* DESCRIPTION:	Set file attribute											*/
/* INPUT:		pucPath:			file name to unlink						*/
/* 				ptAttr:				file attribute information				*/
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
grp_fs_set_attr(
	const grp_uchar_t *pucPath,				/* [IN]  file name to open */
	grp_fs_dir_ent_t *ptAttr)				/* [IN]  file attribute info */
{
	grp_fs_file_t	*ptDir;					/* directory file pointer */
	grp_fs_info_t	*ptFs;					/* FS info */
	grp_fs_file_t	*ptFile;				/* file information */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task environment */
	int				iRet;					/* return value */
	grp_fs_dir_ent_t tAttr;					/* attribute information */
	grp_uchar_t		aucComp[GRP_FS_MAX_COMP];/* component name */
	grp_uchar_t		aucPath[GRP_FS_MAX_PATH];/* path name */

	/****************************************************/
	/* get path name to local area						*/
	/****************************************************/
	iRet = grp_fs_get_str(aucPath, pucPath, sizeof(aucPath));	/* get path */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	pucPath = aucPath;						/* switch to local name */

	/****************************************************/
	/* get attribute information						*/
	/****************************************************/
	iRet = (int)grp_fs_copyin(&tAttr, ptAttr, sizeof(tAttr));
											/* get attr info */
	if (iRet != sizeof(tAttr))				/* copy failed */
		return(GRP_FS_ERR_BAD_PARAM);		/* bad parameter error */
	tAttr.uiProtect &= GRP_FS_PROT_RWXA;	/* mask protection bits */

	/****************************************************/
	/* wait for mount/umount operation					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait for mount/umount */

	/****************************************************/
	/* get base file system information					*/
	/****************************************************/
	iRet = _grp_fs_lookup_base_fs(&pucPath, 0, &ptTask, &ptFs, &ptDir);
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	ptFs->iFsRef++;							/* increment reference */

	/****************************************************/
	/* check read-only file system						*/
	/****************************************************/
	if (ptFs->usStatus & GRP_FS_STAT_RONLY) { /* read-only file system */
		iRet = GRP_FS_ERR_PERMIT;			/* protection error */
		goto err_out;						/* return error */
	}

	/****************************************************/
	/* set file attribute								*/
	/****************************************************/
	iRet = _grp_fs_open_internal(&ptFs, ptDir, &pucPath, 0,
						 aucComp, &ptFile);	/* open file */
	if (iRet != 0)							/* error occured */
		goto err_out;						/* return error */
	if (grp_fs_check_io_status(ptFs, 0, 0, GRP_FS_IO_REQ) != 0)
		iRet = GRP_FS_ERR_IO;				/* I/O error */
	else if (ptFile->usStatus & GRP_FS_FSTAT_NO_UPD_TIME) { /* no time mod */
		iRet = GRP_FS_ERR_PERMIT;			/* permission denied */
	} else {								/* allowd attr mod */
		tAttr.pucName = NULL;				/* no name buffer */
		tAttr.sNameSize = 0;				/* no name buffer */
		iRet = ptFs->ptFsOp->pfnSetAttr(ptFile, &tAttr);/* set attr */
	}
	grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* release file */

err_out:
	if (ptTask)								/* task environment exits */
		_grp_fs_task_rel_env(ptTask);		/* release task environment */
	if (ptFs)								/* FS info exists */
		ptFs->iFsRef--;						/* decrement reference */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_truncate												*/
/*																			*/
/* DESCRIPTION:	Truncate file												*/
/* INPUT:		iFhdl:				file handle of the file					*/
/*				uiOffset:			truncate offset							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_IO			I/O error								*/
/*				GRP_FS_ERR_PERMIT	no write permission						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				GRP_FS_ERR_BAD_OFF	bad offset								*/
/*				GRP_FS_ERR_SHOULD_CLOSE: need to close the file				*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_truncate(
	int				iFhdl,					/* [IN]  file handle to close */
	grp_uint32_t	uiOffset)				/* [IN] attribute information */
{
	grp_fs_fhdl_t	*ptFhdl;				/* file handle */
	grp_fs_file_t	*ptFile;				/* file information */
	grp_fs_info_t	*ptFs;					/* FS information */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	int				iRet;					/* return value */

	/****************************************************/
	/* truncate file									*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	iRet = _grp_fs_check_fhdl(iFhdl, GRP_FS_OPEN_WRITE, &ptFhdl);
											/* check handle */
	if (iRet == 0) {						/* valid handle */
		ptFile = ptFhdl->ptFile;			/* file information */
		ptFs = ptFile->ptFs;				/* FS information */
		if (grp_fs_check_io_status(ptFs, 0, 0, GRP_FS_IO_REQ) != 0) {
			iRet = GRP_FS_ERR_IO;			/* I/O error */
			goto out;						/* return error */
		}
		grp_fs_block_file_op(ptFile);		/* block file operation */
		iRet = ptFs->ptFsOp->pfnTruncate(ptFile,
				(uiOffset >> ptFs->ucFsDBlkShift),
				(uiOffset & (((grp_uint32_t)1 << ptFs->ucFsDBlkShift) - 1)));
											/* truncate file */
		grp_fs_unblock_file_op(ptFile);		/* unblock file operation */
	}
out:
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_close												*/
/*																			*/
/* DESCRIPTION:	close a file												*/
/* INPUT:		iFhdl:				file handle to close					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL		bad file handle							*/
/*				GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_NOMEM	no valid buffer							*/
/*				0					success									*/
/*																			*/
/****************************************************************************/
int
grp_fs_close(
	int				iFhdl)					/* [IN]  file handle to close */
{
	grp_fs_fhdl_t	*ptFhdl;				/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_task_ctl_t *ptTask;				/* task control information */
	int				iRet;					/* return value */

	/****************************************************/
	/* close file handle								*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	iRet = _grp_fs_check_fhdl(iFhdl, 0, &ptFhdl);/* check handle */
	if (iRet == 0) 								/* valid handle */
		iRet = _grp_fs_close_fhdl(ptFhdl);		/* close file hundle */
	ptTask = _grp_fs_task_lookup_env();			/* get task ctrl block */
	if (ptTask)									/* task environment exits */
		_grp_fs_task_rel_env(ptTask);			/* release task environment */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_check_fhdl_common									*/
/*																			*/
/* DESCRIPTION: Read data from file 										*/
/* INPUT:		iFhdl:				file handle number						*/
/*				iMode:				open mode								*/
/* OUTPUT:		pptFhdl:			file handle 							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL 	bad file handle 						*/
/*				GRP_FS_ERR_PERMIT	no read permission						*/
/*				0 or positive:		data count								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_check_fhdl_common(
	int 			iFhdl,				/* [IN]  file hundle number */
	int 			iMode,				/* [IN]  open mode */
	grp_fs_file_t	**pptDir,			/* [OUT] file information */
	grp_fs_info_t	**pptDefFs) 		/* [OUT] default fs information */
{
	grp_fs_fhdl_t	*ptFhdl;				/* file handle */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_task_ctl_t *ptTask = NULL;		/* task control information */
	int 			iRet = 0;				/* return value */
	*pptDefFs = NULL;						/* default FS information */
	*pptDir = NULL;

	if (iFhdl == -1) {							/* use current directory */
		iRet = _grp_fs_task_get_env(&ptTask);	/* get task environment */
		if (iRet != 0)							/* error occured */
			goto out;					 		/* exit function */
		if (ptTask->ptCurDir == GRP_FS_INV_CD) { /* invalidated directory */
			iRet = GRP_FS_ERR_BAD_DIR; 			/* return error */
			goto out;					 		/* exit function */
		}
		if (ptTask->ptCurDir == NULL) { 		/* no current directory */
			if (ptFsCtl->ptFsDefault == NULL) { /* no default FS */
				iRet = GRP_FS_ERR_BAD_DIR;	 	/* return error */
				goto out;				 		/* exit function */
			}
			*pptDefFs = ptFsCtl->ptFsDefault;	/* default FS */
			iRet = (*pptDefFs)->ptFsOp->pfnOpenRoot(*pptDefFs, pptDir);
												/* get root */
			if (iRet != 0)						/* error occured */
				goto out;				 		/* exit function */
			grp_fs_unblock_file_op(*pptDir);	/* release lock */
		} else {								/* current directory exists */
			*pptDir = ptTask->ptCurDir; 		/* use current directory */
		}
	} else {									/* use opened file */
		iRet = _grp_fs_check_fhdl(iFhdl, iMode, &ptFhdl);
												/* check handle */
		if (iRet != 0)							/* invalid handle */
			goto out;				 			/* exit function */
		*pptDir = ptFhdl->ptFile;				/* use opened file */
	}
out:
	if (ptTask)									/* task environment exits */
		_grp_fs_task_rel_env(ptTask);			/* release task environment */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_dirent_common									*/
/*																			*/
/* DESCRIPTION: Get a directory entry										*/
/* INPUT:		iFhdl:				file handle 							*/
/*									(-1: current directory) 				*/
/*				ptDirEnt->uiStart:	start offset							*/
/*				ptDirEnt->sNameSize: name buffer							*/
/*				usRawFlag:			raw flag								*/
/* OUTPUT:		ptDirEnt:			directory entry 						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL 	bad file handle 						*/
/*				GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_BAD_DIR	bad directory							*/
/*				GRP_FS_ERR_BAD_PARAM invalid parameter						*/
/*				GRP_FS_ERR_FS		invalid file system 					*/
/*				GRP_FS_ERR_NOMEM	no valid buffer 						*/
/*				0					EOF 									*/
/*				positive			directory entry size					*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_dirent_common(
	int 			iFhdl,					/* [IN]  file handle to close */
	grp_fs_dir_ent_t *ptDirEnt, 			/* [IN/OUT] directory entry */
	grp_ushort_t	usRawFlag)				/* [IN] raw flag */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptDefFs = NULL;		/* default FS information */
	grp_fs_file_t	*ptDir = NULL;			/* directory information */
	grp_uchar_t 	*pucNameDst;			/* name destination */
	int 			iDstSize;				/* destination size */
	int 			iRet = 0;				/* return value */
	grp_fs_dir_ent_t tDirEnt;				/* directory entry */
	grp_uchar_t 	aucComp[GRP_FS_MAX_COMP];/* component name */
	int 			aucCompNullSize;		/* component name null size */

	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
		
	iRet = _grp_fs_check_fhdl_common(iFhdl, GRP_FS_OPEN_READ, &ptDir, &ptDefFs);
												/* check handle */
	if (iRet != 0)								/* invalid handle */
		goto err_out;							/* return error */

	if (ptDir->ucType != GRP_FS_FILE_DIR) { 	/* not directory */
		iRet = GRP_FS_ERR_BAD_DIR;				/* set error number */
		goto err_out;							/* return error */
	}
	if (ptDirEnt == NULL)						/* null check */
		goto err_out;

	iRet = (int)grp_fs_copyin(&tDirEnt, ptDirEnt, sizeof(tDirEnt));
												/* get dirent information */
	if (tDirEnt.pucName == NULL)				/* null check */
		goto err_out;

	if (iRet != sizeof(tDirEnt))				/* copy failed */
		goto err_out;

	pucNameDst = tDirEnt.pucName;				/* name destination */
	iDstSize = tDirEnt.sNameSize;				/* destination size */
	tDirEnt.pucName = aucComp;					/* set name buffer */
	tDirEnt.sNameSize = sizeof(aucComp);		/* set max component size */
	grp_fs_block_file_op(ptDir);				/* block file operation */
	if(usRawFlag == 0)
	{
		iRet = ptDir->ptFs->ptFsOp->pfnGetDirEnt(ptDir, &tDirEnt);
	}
	else
	{
#ifdef GRP_FS_MULTI_LANGUAGE
		iRet = GET_DIRENT_MULTI_LANG_FUNCTION(ptDir, &tDirEnt);	/* multi language function */
#endif 	/* GRP_FS_MULTI_LANGUAGE */
	}
	if (ptDefFs) {								/* got default root */
		grp_fs_close_file(ptDir, GRP_FS_FILE_UNBLOCK); /* close it */
		ptDir = NULL;							/* clear directory info */
	} else {									/* not default root */
		grp_fs_unblock_file_op(ptDir);			/* unblock file operation */
	}
	if (iRet < 0)								/* error occured */
		goto err_out;							/* error return */
	if (iRet > 0) { 							/* directory entry exists */
		if(usRawFlag == 0)
			aucCompNullSize = 1;				/* null size 1 */
		else
			aucCompNullSize = 2;				/* null siez 2 */

		if(iDstSize != 0)
		{
			if (iDstSize < (tDirEnt.sNameSize + aucCompNullSize))	/* destination is smaller */
				tDirEnt.sNameSize = (short)(iDstSize - aucCompNullSize);/* set new size */
			iRet = (int)grp_fs_copyout(pucNameDst, aucComp, (grp_isize_t)(tDirEnt.sNameSize + aucCompNullSize));
													/* copy out name */
			if (iRet != tDirEnt.sNameSize + aucCompNullSize)	/* error occured */
				goto err_out;						/* return error */
		}
		else
		{
			tDirEnt.sNameSize = 0;
		}
		tDirEnt.pucName = pucNameDst;			/* set original destination */
		iRet = (int)grp_fs_copyout(ptDirEnt, &tDirEnt, sizeof(tDirEnt));
												/* copy back dirent info */
		if (iRet != sizeof(tDirEnt))			/* copy failed */
			goto err_out;
		iRet = (int)(tDirEnt.uiEnd - tDirEnt.uiStart); /* set return value */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);								/* return return value */

err_out:
	if (ptDefFs && ptDir)						/* got default root */
		grp_fs_close_file(ptDir, 0);			/* close it */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	if (iRet >= 0)								/* error in copy */
		iRet = GRP_FS_ERR_BAD_PARAM;			/* bad param error */
	return(iRet);								/* return error */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_dir_ent											*/
/*																			*/
/* DESCRIPTION: Get a directory entry										*/
/* INPUT:		iFhdl:				file handle 							*/
/*									(-1: current directory) 				*/
/*				ptDirEnt->uiStart:	start offset							*/
/*				ptDirEnt->sNameSize: name buffer							*/
/* OUTPUT:		ptDirEnt:			directory entry 						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FHDL 	bad file handle 						*/
/*				GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_BAD_DIR	bad directory							*/
/*				GRP_FS_ERR_BAD_PARAM invalid parameter						*/
/*				GRP_FS_ERR_FS		invalid file system 					*/
/*				GRP_FS_ERR_NOMEM	no valid buffer 						*/
/*				0					EOF 									*/
/*				positive			directory entry size					*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_dirent(
	int 			iFhdl,					/* [IN]  file handle to close */
	grp_fs_dir_ent_t *ptDirEnt) 			/* [IN/OUT] directory entry */
{
	return(grp_fs_get_dirent_common(iFhdl, ptDirEnt, 0));	/* get directory entry */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_mount_root_attr									*/
/*																			*/
/* DESCRIPTION:	Get attribute of mounted root								*/
/* INPUT:		ptFile:				file mounted on							*/
/* OUTPUT:		ptDirEnt:			directory entry							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_BAD_DIR	bad directory							*/
/*				GRP_FS_ERR_BAD_PARAM invalid parameter						*/
/*				GRP_FS_ERR_FS		invalid file system						*/
/*				positive			directory entry size					*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_mount_root_attr(
	grp_fs_file_t	*ptFile,				/* [IN] file mounted on */
	grp_fs_dir_ent_t *ptDirEnt)				/* [OUT] directory entry */
{
	grp_fs_info_t	*ptFs;					/* file system inforamtion */
	grp_fs_file_t	*ptRoot;				/* root file */
	int				iRet;					/* return value */

	ptFs = _grp_fs_find_nest_mount(ptFile);	/* find nestd mount point */
	if (ptFs == NULL)						/* no such file system */
		return(GRP_FS_ERR_FS);				/* return erorr */
	iRet = ptFs->ptFsOp->pfnOpenRoot(ptFs, &ptRoot);
											/* get root derectory */
	if (iRet == 0) {						/* success to get root */
		iRet = ptFs->ptFsOp->pfnGetAttr(ptRoot, ptDirEnt);/* get attr */
		grp_fs_close_file(ptRoot, GRP_FS_FILE_UNBLOCK); /* release root */
	}
	ptFs->iFsRef--;							/* decrement reference */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_sync_fs												*/
/*																			*/
/* DESCRIPTION:	Synchronize specified file system information				*/
/* INPUT:		ptFs:				file system information					*/
/*				iMode:				sync mode								*/
/*									GRP_FS_SYNC_FAILED: sync failed data	*/
/*									GRP_FS_SYNC_HINT:   sync hint data		*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_sync_fs(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	int				iMode)				/* [IN]  wait mode */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;/* FS control data */
	int				iRet;				/* return value */
	int				iStepRet;			/* return value of each step */

	iRet = _grp_fs_sync_buf_io(&ptFsCtl->ptFBufFwd, ptFs->iDev, iMode);
										/* write back file info cache */
	iStepRet = _grp_fs_sync_buf_io(&ptFsCtl->ptDBufFwd, ptFs->iDev, iMode);
										/* write back file data cache */
	if (iRet == 0)						/* no error upto now */
		iRet = iStepRet;				/* set steop return value */
	if (ptFs->ptFsOp->pfnSync) {		/* exists FS depedent sync */
		iStepRet = ptFs->ptFsOp->pfnSync(ptFs, iMode); /* make FS dep sync */
		if (iRet == 0)					/* no error upto now */
			iRet = iStepRet;			/* set steop return value */
	}
	return(iRet);						/* return result */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_sync_dep_info										*/
/*																			*/
/* DESCRIPTION:	Synchronize FS depenedent information						*/
/* INPUT:		ptFs:				file system information					*/
/*				iMode:				sync mode								*/
/*									GRP_FS_SYNC_FAILED: sync failed data	*/
/*									GRP_FS_SYNC_HINT:   sync hint data		*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_sync_dep_info(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	int				iMode)				/* [IN]  wait mode */
{
	int				iRet = 0;			/* return value */
	int				iFsRet;				/* return value FS sync */

	for ( ; ptFs; ptFs = ptFs->ptFsOtherFwd) {	/* for all same level FS */
		iFsRet = (ptFs->ptFsOp->pfnSync != NULL)?
				ptFs->ptFsOp->pfnSync(ptFs, iMode): 0;
										/* make FS dep sync */
		if (iRet == 0)					/* no error upto now */
			iRet = iFsRet;				/* set FS dependent return */
		if (ptFs->ptFsNest) {			/* nested mount */
			iFsRet = _grp_fs_sync_dep_info(ptFs->ptFsNest, iMode);
										/* sync nested FS */
			if (iRet == 0)				/* no error upto now */
				iRet = iFsRet;			/* set FS dependent return */
		}
	}
	return(iRet);						/* return result */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_sync													*/
/*																			*/
/* DESCRIPTION:	Update all modifications to devices							*/
/* INPUT:		iMode:				sync mode								*/
/*									GRP_FS_SYNC_FAILED: sync failed data	*/
/*									GRP_FS_SYNC_HINT:   sync hint data		*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*																			*/
/****************************************************************************/
int
grp_fs_sync(
	int				iMode)				/* [IN]  wait mode */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;/* FS control data */
	int				iRet;				/* return value */
	int				iStepRet;			/* step return value */

	grp_fs_get_sem(ptFsCtl->tFsSem);	/* get semaphore */
	iRet = _grp_fs_sync_file();			/* sync updated file */
	iStepRet = _grp_fs_sync_buf_io(&ptFsCtl->ptFBufFwd, -1, iMode);
										/* sync file type data I/O */
	if (iRet == 0)						/* no error upto now */
		iRet = iStepRet;				/* set step return value */
	iStepRet = _grp_fs_sync_buf_io(&ptFsCtl->ptDBufFwd, -1, iMode);
										/* sync data type data I/O */
	if (iRet == 0)						/* no error upto now */
		iRet = iStepRet;				/* set step return value */
	(void)_grp_fs_wait_mount(0);		/* wait mount/unmount in progress */
	iStepRet = _grp_fs_sync_dep_info(ptFsCtl->ptFsMnt, iMode);
										/* sync FS dependent information */
	if (iRet == 0)						/* no error upto now */
		iRet = iStepRet;				/* set step return value */
	grp_fs_release_sem(ptFsCtl->tFsSem); /* release semaphore */
	return(iRet);						/* return sync result */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_return_mnt_info										*/
/*																			*/
/* DESCRIPTION:	Return mounted file system information						*/
/* INPUT:		ptFs:				file system information					*/
/* OUTPUT:		ptMntInfo:			mounted file system information			*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_BAD_PARAM: bad ptMntInfo paremeter				*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_return_mnt_info(
	grp_fs_info_t		*ptFs,			/* [IN]  file system information */
	grp_fs_mnt_info_t	*ptMntInfo)		/* [OUT] mounted FS information */
{
	int					iCopied;				/* copied length */
	int					iPart;					/* partition */
	grp_fs_mnt_info_t	tMntInfo;				/* mounted FS information */
	int	sprintf(char *pcBuf, const char *pcFormat, ...);

	tMntInfo.iDev = ptFs->iDev;					/* device number */
	tMntInfo.iParentDev = (ptFs->ptFsParent)?
		ptFs->ptFsParent->iDev: -1;				/* parent device number */
	iPart = GRP_FS_DEV_PART(ptFs->iDev);		/* partition number */
	sprintf(tMntInfo.acDevName, "%s%d%c",
			grp_fs_dev_tbl[GRP_FS_DEV_MAJOR(ptFs->iDev)].pcDevName,
			GRP_FS_DEV_SUBID(ptFs->iDev),	
			((iPart == GRP_FS_DEV_RAW_PART)? '*': ('a' +  iPart)));
												/* set device name */
	strcpy(tMntInfo.acFsType, ptFs->ptFsTbl->pcFsName); /* file system type */
	strcpy((char *)tMntInfo.aucPath, (char *)ptFs->aucPath);
												/* mount path component */
	tMntInfo.uiStatus = 0;						/* clear status bit */
	if (ptFs->usStatus & GRP_FS_STAT_RONLY)		/* read only */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_RONLY;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_DAY_ACCTIME)/* day base access time */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_DAY_ACCTIME;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_NO_UPD_ACCTIME)/* no upd media acc time */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_NO_UPD_ACCTIME;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_NO_MNT_FLAG)/* no mount flag on media */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_NO_MNT_FLAG;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_NO_CRT_ACCTIME)/* no media crt/acc time */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_NO_CRT_ACCTIME;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_DEV_INV)	/* device invalidated */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_DEV_INV;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_SYNC_ALL)	/* sync write always */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_SYNC_ALL;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_SYNC_FL_CLOSE)/* sync on each close */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_SYNC_FL_CLOSE;/* set the bit */
	if (ptFs->usStatus & GRP_FS_STAT_SYNC_FS_CLOSE)/* sync on last close */
		tMntInfo.uiStatus |= GRP_FS_MSTAT_SYNC_FS_CLOSE;/* set the bit */
	tMntInfo.usVolNameLen = ptFs->usVolNameLen;	/* volume name length */
	memcpy(tMntInfo.aucVolName, ptFs->aucVolName,
					sizeof(tMntInfo.aucVolName));/* copy volume name */
	tMntInfo.uiVolSerNo = ptFs->uiVolSerNo;		/* volume serial number */
	tMntInfo.uiFsBlkSize = ((grp_uint32_t)1 << ptFs->ucFsCBlkShift);
												/* block size */
	tMntInfo.uiFsBlkCnt = ptFs->uiFsBlkCnt;		/* total block count */
	tMntInfo.uiFsFileCnt = ptFs->uiFsFileCnt;	/* total file count */
	tMntInfo.uiFsFreeBlk = ptFs->uiFsFreeBlk;	/* free block count */
	tMntInfo.uiFsFreeFile = ptFs->uiFsFreeFile;	/* free file count */
	tMntInfo.uiFBufSize = ((grp_uint32_t)1 << ptFs->ucFsFBlkShift);
												/* file buffer size */
	tMntInfo.uiDBufSize = ((grp_uint32_t)1 << ptFs->ucFsDBlkShift);
												/* data buffer size */
	tMntInfo.uiClusterSize = ((grp_uint32_t)1 << ptFs->ucFsCBlkShift);
												/* file cluseter size */
	tMntInfo.uiFBufOff = ptFs->uiFsFBlkOff;		/* file buffer offset */
	tMntInfo.uiDBufOff = ptFs->uiFsDBlkOff;		/* data buffer offset */
	tMntInfo.uiDevOff = (ptFs->uiDevOff << ptFs->ucDevBlkShift);
												/* device offset */
	tMntInfo.usFsSubType = ptFs->usFsSubType;	/* sub file system type */
	iCopied = (int)grp_fs_copyout(ptMntInfo, &tMntInfo, sizeof(tMntInfo));
												 /* copy out information */
	return((iCopied != sizeof(tMntInfo))? GRP_FS_ERR_BAD_PARAM: 0);
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_return_all_mnt_info									*/
/*																			*/
/* DESCRIPTION:	Check mount device is already mounted or not				*/
/* INPUT:		ptFs:				FS information to check					*/
/* 				iMaxCnt:			max information count					*/
/* OUTPUT:		ptMntInfo:			mounted file system information			*/
/*																			*/
/* RESULT:		0 or positive:		returned file system information count	*/
/*				GRP_FS_ERR_BAD_PARAM: bad paremeter							*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_return_all_mnt_info(
	grp_fs_info_t		*ptFs,			/* [IN]  FS information to check */
	int					iMaxCnt,			/* [IN]  max information count */
	grp_fs_mnt_info_t	*ptMntInfo)		/* [OUT] mounted file system info */
{
	int				iTotalCnt = 0;		/* total count */
	int				iSubCnt;			/* sub count */

	for ( ; ptFs && iTotalCnt < iMaxCnt; ptFs = ptFs->ptFsOtherFwd) {
		if (_grp_fs_return_mnt_info(ptFs, ptMntInfo++) != 0) /* copy failed */
			return(GRP_FS_ERR_BAD_PARAM);
		iTotalCnt++;					/* increment count */
		if (ptFs->ptFsNest) {			/* nested mount */
			iSubCnt = _grp_fs_return_all_mnt_info(ptFs->ptFsNest,
									iMaxCnt - iTotalCnt, ptMntInfo);
										/* return nested ones */
			if (iSubCnt < 0)			/* error detected */
				return(iSubCnt);		/* return error */
			iTotalCnt += iSubCnt;		/* increment count */
			ptMntInfo += iSubCnt;		/* advance pointer */
		}
	}
	return(iTotalCnt);					/* return copied count */
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_get_mnt												*/
/*																			*/
/* DESCRIPTION:	Get all mounted file system information						*/
/* INPUT:		iMaxCnt:			max information count					*/
/* OUTPUT:		ptMntInfo:			mounted file system information			*/
/*																			*/
/* RESULT:		0 or positive:		returned file system information count	*/
/*				GRP_FS_ERR_BAD_PARAM: bad paremeter							*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_mnt(
	int					iMaxCnt,		/* [IN]  max information count */
	grp_fs_mnt_info_t	*ptMntInfo)		/* [OUT] mounted file system info */
{
	int					iRet;				/* return value */
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;/* FS control data */

	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	(void)_grp_fs_wait_mount(0);			/* wait mount/umount */
	iRet = _grp_fs_return_all_mnt_info(ptFsCtl->ptFsMnt, iMaxCnt, ptMntInfo);
											/* return all mount info */
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	return(iRet);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_get_mnt_by_dev										*/
/*																			*/
/* DESCRIPTION:	Get mounted file system information by device number		*/
/* INPUT:		iDev:				device number							*/
/* OUTPUT:		ptMntInfo:			mounted file system information			*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_BAD_PARAM: bad paremeter							*/
/*				GRP_FS_ERR_BAD_DEV:	bad device number						*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_mnt_by_dev(
	int 				iDev,				/* [IN]  device number */
	grp_fs_mnt_info_t	*ptMntInfo)			/* [OUT] mounted file system info */
{
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptFs;					/* FS info */
	int				iRet = 0;				/* return value */

	/****************************************************/
	/* check mount/umount in progress					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	(void)_grp_fs_wait_mount(0);				/* wait mount/umount */

	/****************************************************/
	/* lookup FS info mounting the device				*/
	/****************************************************/
	ptFs = grp_fs_check_mnt_dev(ptFsCtl->ptFsMnt, iDev);
	if (ptFs == NULL) {							/* no such device mounted */
		iRet = GRP_FS_ERR_BAD_DEV;				/* bad device */
	} else {									/* found FS */
		iRet =_grp_fs_return_mnt_info(ptFs, ptMntInfo); /* copy info */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/****************************************************************************/
/* FUNCTION:	grp_fs_get_mnt_by_name										*/
/*																			*/
/* DESCRIPTION:	Get mounted file system information by device name			*/
/* INPUT:		pcDevName:			device name								*/
/* OUTPUT:		ptMntInfo:			mounted file system information			*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_BAD_PARAM: bad paremeter							*/
/*				GRP_FS_ERR_BAD_DEV:	bad device name							*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_mnt_by_name(
	const char			*pcDevName,			/* [IN]  device name */
	grp_fs_mnt_info_t	*ptMntInfo)			/* [OUT] mounted file system info */
{
	int				iDev;					/* device number */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptFs;					/* FS info */
	int				iRet = 0;				/* return value */

	/****************************************************/
	/* lookup device name								*/
	/****************************************************/
	if ((iDev = grp_fs_lookup_dev(pcDevName)) < 0)	/* bad device */
		return(iDev);							/* return error */

	/****************************************************/
	/* check mount/umount in progress					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	(void)_grp_fs_wait_mount(0);				/* wait mount/umount */

	/****************************************************/
	/* lookup FS info mounting the device				*/
	/****************************************************/
	ptFs = grp_fs_check_mnt_dev(ptFsCtl->ptFsMnt, iDev);
	if (ptFs == NULL) {							/* no such device mounted */
		iRet = GRP_FS_ERR_BAD_DEV;				/* bad device */
	} else {									/* found FS */
		iRet =_grp_fs_return_mnt_info(ptFs, ptMntInfo); /* copy info */
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_invalidate_fs_dev									*/
/*																			*/
/* DESCRIPTION:	Make FS device status invalid								*/
/* INPUT:		pcDevName:			device name 							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_BAD_DEV:	bad device name							*/
/*																			*/
/****************************************************************************/
int
grp_fs_invalidate_fs_dev(
	const char		*pcDevName)			/* [IN]  device name */
{
	int				iDev;					/* device number */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptFs;					/* FS info */
	int				iRet = 0;				/* return value */

	/****************************************************/
	/* lookup device name								*/
	/****************************************************/
	if ((iDev = grp_fs_lookup_dev(pcDevName)) < 0)	/* bad device */
		return(iDev);							/* return error */

	/****************************************************/
	/* check mount/umount in progress					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	(void)_grp_fs_wait_mount(0);				/* wait mount/umount */

	/****************************************************/
	/* lookup FS info mounting the device, and set		*/
	/* invalid flag										*/
	/****************************************************/
	ptFs = grp_fs_check_mnt_dev(ptFsCtl->ptFsMnt, iDev);
	if (ptFs) 									/* no such device mounted */
		ptFs->usStatus |= GRP_FS_STAT_DEV_INV;	/* set invalid bit */
	else
		iRet = GRP_FS_ERR_BAD_DEV;				/* bad device */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/****************************************************************************/
/* FUNCTION:	_grp_fs_copyout_volume_info									*/
/*																			*/
/* DESCRIPTION:	Get volume information										*/
/* INPUT:		pucSrcVolName:		source volume name						*/
/*				iSrcVolNameLen:		source volume name length				*/
/*				uiSrcVolSerNo:		source volume serial number				*/
/*				piDstVolNameLen:	max desitination volume name length		*/
/* OUTPUT:		pucVolName:			volume name								*/
/*				piDstVolNameLen:	volume name length						*/
/*				puiVolSerNo:		volume serial number					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_PARAM: bad parameter							*/
/*				others:				iSrcVolNameLen							*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_copyout_volume_info(
	grp_uchar_t		*pucSrcVolName,		/* [IN]  source volume name */
	int				iSrcVolNameLen,		/* [IN]  source volume name length */
	grp_uint32_t	uiSrcVolSerNo,		/* [IN]  source volume serial number */
	grp_uchar_t		*pucDstVolName,		/* [OUT] dst volume name */
	int				*piDstVolNameLen,	/* [IN/OUT] dst volume name length */
	grp_uint32_t	*puiDstVolSerNo)	/* [OUT] dst volume serial number */
{
	int				iDstVolNameLen;		/* dst volume name length */
	int				iCopyoutLen;		/* copyout length */

	if (iSrcVolNameLen < 0)				/* no volume info */
		return(iSrcVolNameLen);			/* return error */
	if (pucDstVolName && piDstVolNameLen) {	/* copyout volume name */
		if (grp_fs_copyin((char *)&iDstVolNameLen, piDstVolNameLen,
							sizeof(iDstVolNameLen)) != sizeof(iDstVolNameLen))
			goto err_ret;				/* return error */
		iCopyoutLen = (iSrcVolNameLen < GRP_FS_VOL_NAME_LEN
						&& iSrcVolNameLen < iDstVolNameLen)? iSrcVolNameLen + 1:
						iSrcVolNameLen;
		if (iCopyoutLen > iDstVolNameLen
			|| grp_fs_copyout((char *)pucDstVolName, (char *)pucSrcVolName,
								 (grp_isize_t)iCopyoutLen) != iCopyoutLen
			|| grp_fs_copyout((char *)piDstVolNameLen, (char *)&iSrcVolNameLen,
							 sizeof(iSrcVolNameLen)) != sizeof(iSrcVolNameLen))
			goto err_ret;				/* return error */
	}
	if (puiDstVolSerNo) {				/* copyout volume serial number */
		if (grp_fs_copyout((char *)puiDstVolSerNo, (char *)&uiSrcVolSerNo,
							sizeof(uiSrcVolSerNo)) != sizeof(uiSrcVolSerNo))
			goto err_ret;				/* return error */
	}
	return(iSrcVolNameLen);				/* return volume name length */

err_ret:
	return(GRP_FS_ERR_BAD_PARAM);		/* return error */
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_check_fs_dev											*/
/*																			*/
/* DESCRIPTION:	Check FS device status 										*/
/* INPUT:		pcDevName:			device name 							*/
/*				piVolNameLen:		max volume name length					*/
/* OUTPUT:		pucVolName:			volume name								*/
/*				piVolNameLen:		volume name length						*/
/*				puiVolSerNo:		volume serial number					*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_NEED_CHECK: need to check file system			*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_BAD_DEV:	bad device name							*/
/*				GRP_FS_BAD_PARAM:	bad parameter							*/
/*																			*/
/****************************************************************************/
int
grp_fs_check_fs_dev(
	const char		*pcDevName,			/* [IN]  device name */
	grp_uchar_t		*pucVolName,		/* [OUT] volume name length */
	int				*piVolNameLen,		/* [IN/OUT] volume name length */
	grp_uint32_t	*puiVolSerNo)		/* [OUT] volume serial number */
{
	int				iDev;					/* device number */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_info_t	*ptFs;					/* FS info */
	int				iRet = 0;				/* return value */
	grp_uchar_t		aucVolName[GRP_FS_VOL_NAME_LEN];	/* volume name */
	grp_uint32_t	uiVolSerNo;				/* serial number */

	/****************************************************/
	/* lookup device name								*/
	/****************************************************/
	if ((iDev = grp_fs_lookup_dev(pcDevName)) < 0)	/* bad device */
		return(iDev);							/* return error */

	/****************************************************/
	/* check mount/umount in progress					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	(void)_grp_fs_wait_mount(0);				/* wait mount/umount */

	/****************************************************/
	/* lookup FS info mounting the device, and			*/
	/* check volume										*/
	/****************************************************/
	ptFs = grp_fs_check_mnt_dev(ptFsCtl->ptFsMnt, iDev);
	if (ptFs) { 								/* no such device mounted */
		ptFs->iFsRef++;							/* increment reference */
		iRet = ptFs->ptFsOp->pfnCheckVolume(iDev, ptFs,
							aucVolName, &uiVolSerNo);/* check volume label */
		iRet = _grp_fs_copyout_volume_info(aucVolName, iRet, uiVolSerNo,
									pucVolName, piVolNameLen, puiVolSerNo);
												/* copy out volume info */
		if (iRet >= 0
			&& (int)ptFs->usVolNameLen == iRet
			&& strncmp((char *)ptFs->aucVolName, (char *)aucVolName,
						(grp_size_t)iRet) == 0
			&& ptFs->uiVolSerNo == uiVolSerNo) {/* match volume */
			ptFs->usStatus &= ~GRP_FS_STAT_DEV_INV; /* reset invalid bit */
			iRet = _grp_fs_sync_fs(ptFs, GRP_FS_SYNC_FAILED);
												/* sync data */
		} else {
			ptFs->usStatus |= GRP_FS_STAT_DEV_INV; /* set invalid bit */
			if (iRet >= 0)						/* no error number is set */
				iRet = GRP_FS_ERR_NEED_CHECK;	/* set error number */
		}
		ptFs->iFsRef--;							/* decrement reference */
	} else
		iRet = GRP_FS_ERR_BAD_DEV;				/* bad device */
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return(iRet);								/* return result */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_check_volume											*/
/*																			*/
/* DESCRIPTION:	Check volume name											*/
/* INPUT:		pcDevName:			device name 							*/
/*				pcFsType:			file system type						*/
/*				piVolNameLen:		max volume name length					*/
/* OUTPUT:		pucVolName:			volume name								*/
/*				piVolNameLen:		volume name length						*/
/*				puiVolSerNo:		volume serial number					*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_BAD_DEV:	bad device name							*/
/*				GRP_FS_ERR_BAD_FSNAME: bad file system type name			*/
/*				GRP_FS_BAD_PARAM:	bad parameter							*/
/*																			*/
/****************************************************************************/
int
grp_fs_check_volume(
	const char		*pcDevName,			/* [IN]  device name */
	const char		*pcFsType,			/* [IN]  file system type */
	grp_uchar_t		*pucVolName,		/* [OUT] volume name length */
	int				*piVolNameLen,		/* [IN/OUT]  volume name length */
	grp_uint32_t	*puiVolSerNo)		/* [OUT] volume serial number */
{
	int				iDev;					/* device number */
	grp_fs_ctl_t	*ptFsCtl = grp_fs_ctl;	/* FS control data */
	grp_fs_type_tbl_t *ptFsTbl;				/* FS type table */
	int				iRet = 0;				/* return value */
	grp_uchar_t		aucVolName[GRP_FS_VOL_NAME_LEN];	/* volume name */
	grp_uint32_t	uiVolSerNo;				/* serial number */

	/****************************************************/
	/* lookup device name								*/
	/****************************************************/
	if ((iDev = grp_fs_lookup_dev(pcDevName)) < 0)	/* bad device */
		return(iDev);							/* return error */

	/****************************************************/
	/* lookup file sytsem type name						*/
	/****************************************************/
	iRet = _grp_fs_lookup_fs_type(pcFsType, &ptFsTbl);/* lookup FS type */
	if (iRet != 0)								/* error detected */
		return(iRet);							/* return error */

	/****************************************************/
	/* check mount/umount in progress					*/
	/****************************************************/
	grp_fs_get_sem(ptFsCtl->tFsSem);			/* get semaphore */
	(void)_grp_fs_wait_mount(0);				/* wait mount/umount */

	/****************************************************/
	/* get volume information							*/
	/****************************************************/
	iRet = ptFsTbl->ptFsOp->pfnCheckVolume(iDev, NULL,
							aucVolName, &uiVolSerNo);/* check volume label */
	iRet = _grp_fs_copyout_volume_info(aucVolName, iRet, uiVolSerNo,
							pucVolName, piVolNameLen, puiVolSerNo);
	grp_fs_release_sem(ptFsCtl->tFsSem);		/* release semaphore */
	return((iRet >= 0)? 0: iRet);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/****************************************************************************/
/* FUNCTION:	_grp_fs_get_err_buf_data									*/
/*																			*/
/* DESCRIPTION:	Get error buffer data										*/
/* INPUT:		pptBufHead:			head of buffer list						*/
/*				iMode:				operation mode							*/
/*				iDev:				target device							*/
/*				uiBlk:				target block							*/
/*				uiSize:				size of buffer							*/
/* OUTPUT:		pucBuf:				error data								*/
/*				puiNeed:			need buffer size						*/
/*																			*/
/* RESULT:		copied size													*/
/*																			*/
/****************************************************************************/
static grp_uint32_t
_grp_fs_get_err_buf_data(
	grp_fs_buf_t		**pptBufHead,		/* [IN]  head of buffer list */
	int					iMode,				/* [IN]  opertion mode */
	int					iDev,				/* [IN]  target device */
	grp_uint32_t		uiBlk,				/* [IN]  target block */
	grp_uchar_t			*pucBuf,			/* [OUT] output buffer */
	grp_uint32_t		uiSize,				/* [IN]  buffer size */
	grp_uint32_t		*puiNeed)			/* [OUT] need buffer size */
{
	grp_fs_buf_t		*ptBuf;				/* buffer information */
	grp_fs_buf_t		*ptBufNext;			/* next buffer */
	grp_uint32_t		uiCopied = 0;		/* copied size */
	grp_uint32_t		uiDataSize;			/* data size */
	grp_uint32_t		uiProcSize;			/* processed size */
	grp_fs_err_binfo_t	tErrInfo;			/* error information */

	for (ptBuf = *pptBufHead; ptBuf; ptBuf = ptBufNext) {
		ptBufNext = ptBuf->ptListFwd;			/* next buffer */
		if (ptBuf->iRefCnt != 0)				/* stil referenced */
			continue;							/* skip it */
		if ((ptBuf->usStatus & GRP_FS_BSTAT_WFAIL) == 0			/* no error */
			&& ((iMode & GRP_FS_GE_DIRTY) == 0
				|| (ptBuf->usStatus & GRP_FS_BSTAT_DIRTY) == 0))/* not dirty */
			continue;							/* skip it */
		if (iDev >= 0 && iDev != ptBuf->iDev)	/* dev missmatch */
			continue;							/* skip it */
		if (uiBlk != GRP_FS_GE_BLK_ANY
			&& uiBlk != ptBuf->uiBlk)			/* block missmatch */
			continue;							/* skip it */
		uiDataSize = sizeof(grp_fs_err_binfo_t); /* error info */
		if (iMode & GRP_FS_GE_CONTENT)			/* need data */
			uiDataSize += ptBuf->iSize;			/* add data size */
		if (uiCopied + uiDataSize <= uiSize) {	/* fit in buffer */
			tErrInfo.iDev = ptBuf->iDev;		/* set device number */
			tErrInfo.uiBlk = ptBuf->uiBlk;		/* set block number */
			tErrInfo.uiSize = ptBuf->iSize;		/* set size */
			if (ptBuf->usStatus & GRP_FS_BSTAT_FBUF) {	/* file type */
				tErrInfo.ucBufType = GRP_FS_EI_BUF_FILE; /* file type */
				tErrInfo.ucBlkShift = ptBuf->ptFs->ucFsFBlkShift;/* blk shift */
				tErrInfo.uiBlkOff = ptBuf->ptFs->uiFsFBlkOff;/* blk offset */
			} else {									/* data type */
				tErrInfo.ucBufType = GRP_FS_EI_BUF_DATA; /* data type */
				tErrInfo.ucBlkShift = ptBuf->ptFs->ucFsDBlkShift;/* blk shift */
				tErrInfo.uiBlkOff = ptBuf->ptFs->uiFsDBlkOff;/* blk offset */
			}
			uiProcSize = grp_fs_copyout(pucBuf, &tErrInfo, sizeof(tErrInfo));
			if (iMode & GRP_FS_GE_CONTENT)		/* need data */
				uiProcSize += grp_fs_copyout(pucBuf + sizeof(tErrInfo),
									 ptBuf->pucData, ptBuf->iSize);
			if (uiProcSize == uiDataSize) {		/* copy success */
				pucBuf += uiDataSize;			/* advance pointer */
				uiCopied += uiDataSize;			/* increment copied size */
				if (iMode & GRP_FS_GE_RELEASE)	/* release buffer */
					_grp_fs_invalidate_buf(ptBuf);/* invalidate buffer */
			}
		}
		*puiNeed += uiDataSize;					/* increment need size */
	}
	return(uiCopied);							/* return copied size */
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_get_error											*/
/*																			*/
/* DESCRIPTION:	Get write error data in I/O buffer  						*/
/*		If ((iMode & GRP_FS_GE_FBUF) != 0), error data in file buffer is	*/
/*		returned.  If ((iMode & GRP_FS_GE_DBUF) != 0), error data in data	*/
/*		buffer is returned.													*/
/*		If ((iMode & GRP_FS_GE_CONTENT) == 0), only matched grp_fs_err_binfo_t*/
/*		structures are returned. If ((iMode & GRP_FS_GE_CONTENT) != 0),		*/
/*		matched grp_fs_err_binfo_t and data pairs are returned.				*/
/*		If ((iMode & GRP_FS_GE_RELEASE) != 0), I/O data buffer is released.	*/
/*		Note that if GRP_FS_GE_RELEASE is set, GRP_FS_GE_CONTNET is assumed	*/
/*		to be set even not set.	If both GRP_FS_GE_FBUF and GRP_FS_GE_DBUF	*/
/*		are not set, GRP_FS_ERR_PARAM is returned.							*/
/*																			*/
/* INPUT:		iMode:				operation mode							*/
/*				iDev:				target device							*/
/*				uiBlk:				target block							*/
/*				uiSize:				buffer size								*/
/* OUTPUT:		pucBuf:				error data								*/
/*				puiNeed:			need buffer size						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_PARAM: invalid iMode							*/
/*				others:				size of data returned					*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_get_error(
	int					iMode,				/* [IN]  opertion mode */
	int					iDev,				/* [IN]  target device */
	grp_uint32_t		uiBlk,				/* [IN]  target block */
	grp_uchar_t			*pucBuf,			/* [OUT] output buffer */
	grp_uint32_t		uiSize,				/* [IN]  buffer size */
	grp_uint32_t		*puiNeed)			/* [OUT] need buffer size */
{
	grp_fs_ctl_t		*ptFsCtl = grp_fs_ctl;/* FS control data */
	grp_uint32_t		uiCopied = 0;		/* copied data size */
	grp_uint32_t		uiNeed = 0;			/* need size */

	if ((iMode & (GRP_FS_GE_FBUF|GRP_FS_GE_DBUF)) == 0) /* no buffer specified */
		return(GRP_FS_ERR_BAD_PARAM);		/* bad parameter */
	if (iMode & GRP_FS_GE_RELEASE)			/* release buffer */
		iMode |= GRP_FS_GE_CONTENT;			/* set GRP_FS_GE_CONTENT bit */
	grp_fs_get_sem(ptFsCtl->tFsSem);		/* get semaphore */
	if (iMode & GRP_FS_GE_FBUF) {			/* get file type buffer */
		uiCopied = _grp_fs_get_err_buf_data(&ptFsCtl->ptFBufFwd, iMode,
										iDev, uiBlk, pucBuf, uiSize, &uiNeed);
		pucBuf += uiCopied;					/* advance buffer pointer */
		uiSize -= uiCopied;					/* decrement buffer size */
	}
	if (iMode & GRP_FS_GE_DBUF) {			/* get data type buffer */
		uiCopied += _grp_fs_get_err_buf_data(&ptFsCtl->ptDBufFwd, iMode,
										iDev, uiBlk, pucBuf, uiSize, &uiNeed);
	}
	grp_fs_release_sem(ptFsCtl->tFsSem);	/* release semaphore */
	if (grp_fs_copyout((char *)puiNeed, (char *)&uiNeed, sizeof(uiNeed))
						!= sizeof(uiNeed))
		return(GRP_FS_ERR_BAD_PARAM);		/* return error */
	return((grp_int32_t)uiCopied);			/* return copied size */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/****************************************************************************/
/* FUNCTION:	grp_fs_not_supported										*/
/*																			*/
/* DESCRIPTION:	Return GRP_FS_ERR_NOT_SUPPORT								*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_NOT_SUPPORT:	not supported operation				*/
/*																			*/
/****************************************************************************/
int
grp_fs_not_supported(void)
{
	return(GRP_FS_ERR_NOT_SUPPORT);				/* return not supported */
}

#ifdef GRP_FS_MULTI_LANGUAGE
/* multi language support */
#include "grp_fs_multi_language.c"
#endif 	/* GRP_FS_MULTI_LANGUAGE */

#ifdef	GRP_FS_DEBUG
#include "grp_fs_debug.c"
#ifdef	GRP_FS_TRACE
#include "grp_fs_trace.c"
#endif	/* GRP_FS_TRACE */
#endif	/* GRP_FS_DEBUG */
