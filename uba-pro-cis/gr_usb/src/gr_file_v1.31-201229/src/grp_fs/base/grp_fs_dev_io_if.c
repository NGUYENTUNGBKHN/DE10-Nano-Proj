/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_dev_io_if.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Exported device I/O interfaces										*/
/* FUNCTIONS:																*/
/*		grp_fs_open_dev				open device								*/
/*		grp_fs_close_dev			close device							*/
/*		grp_fs_read_dev				read device								*/
/*		grp_fs_write_dev			write device							*/
/*		grp_fs_ioctl_dev			I/O control device						*/
/* DEPENDENCIES:															*/
/*		grp_fs_dev_io_if.h													*/
/*		grp_fs_cfg.h														*/
/*		grp_fs.h															*/
/*		grp_mem.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2004/07/25	Created inital version 1.0				*/
/*		T.Imashiki		2007/02/20	Changed type of iRet variables for 16	*/
/*									bit CPU support							*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
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
#include "grp_fs_dev_io_if.h"
#include "grp_fs_cfg.h"
#include "grp_fs.h"
#include "grp_mem.h"

#define GRP_FS_DEV_CTL_BUF		128			/* local I/O buffer size */

/****************************************************************************/
/* FUNCTION:	_grp_fs_check_dev_no										*/
/*																			*/
/* DESCRIPTION:	Check device number											*/
/* INPUT:		iDev:					device number						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL					invalid device number				*/
/*				others					device operation pointer			*/
/*																			*/
/****************************************************************************/
static grp_fs_dev_op_t *
_grp_fs_check_dev_no(
	int				iDev)					/* [IN]  device number */
{
	int				iMajor;					/* major device number */

	iMajor = GRP_FS_DEV_MAJOR(iDev);		/* major device number */
	if (iMajor < 0 || iMajor >= grp_fs_dev_tbl_cnt)
		return(NULL);						/* invalid device number */
	return(grp_fs_dev_tbl[iMajor].ptOp);	/* return device operation ptr */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_open_dev												*/
/*																			*/
/* DESCRIPTION:	Open device													*/
/* INPUT:		iDev:					device number						*/
/*				iRWOpen:				read/write open						*/
/* OUTPUT:		ptDevIoInfo:			device I/O information				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:			I/O error							*/
/*				GRP_FS_ERR_BAD_DEV:		bad device name/number				*/
/*				GRP_FS_ERR_NOT_SUPPORT	not supported						*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_fs_open_dev(
	int				iDev,					/* [IN]  device number */
	int				iRWOpen,				/* [IN]  read/write open */
	grp_fs_dev_io_info_t *ptDevIoInfo)		/* [OUT] device I/O information */
{
	int				iRet;					/* return value */
	grp_fs_dev_op_t	*ptDevOp;				/* device operation */
	grp_fs_dev_io_info_t tDevInfo;			/* device I/O information */

	ptDevOp = _grp_fs_check_dev_no(iDev);	/* check device number */
	if (ptDevOp == NULL)					/* invalid device number */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	if (ptDevOp->pfnOpen == NULL)			/* no open function */
		return(GRP_FS_ERR_NOT_SUPPORT);		/* return error */
	iRet = ptDevOp->pfnOpen(iDev, iRWOpen, 
							&tDevInfo.iHandle, &tDevInfo.uiOff,
							&tDevInfo.uiSize, &tDevInfo.iSzShift);
											/* open device */
	if (iRet == 0) {						/* success */
		if (grp_fs_copyout(ptDevIoInfo, &tDevInfo, sizeof(tDevInfo))
					!= sizeof(tDevInfo))	/* put result failed */
			iRet = GRP_FS_ERR_BAD_PARAM;	/* set error number */
	}
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_close_dev											*/
/*																			*/
/* DESCRIPTION:	Close device												*/
/* INPUT:		iHandle,				I/O handle							*/
/* 				iDev:					device number						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:			I/O error							*/
/*				GRP_FS_ERR_NOT_SUPPORT	not supported						*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_fs_close_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev)					/* [IN]  device number */
{
	int				iRet;					/* return value */
	grp_fs_dev_op_t	*ptDevOp;				/* device operation */

	ptDevOp = _grp_fs_check_dev_no(iDev);	/* check device number */
	if (ptDevOp == NULL)					/* invalid device number */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	if (ptDevOp->pfnClose == NULL)			/* no close function */
		return(GRP_FS_ERR_NOT_SUPPORT);		/* return error */
	iRet = ptDevOp->pfnClose(iHandle, iDev);/* close device */
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_read_dev												*/
/*																			*/
/* DESCRIPTION:	Read device													*/
/*				Note: Address space conversion should be added to support	*/
/*					  multi-address-space cnfiguration						*/
/* INPUT:		iHandle,				I/O handle							*/
/* 				iDev:					device number						*/
/*				uiDevBlk:				device block number					*/
/*				iCnt:					I/O count							*/
/* OUTPUT:		pucBuf:					data read							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:			I/O error							*/
/*				GRP_FS_ERR_NOT_SUPPORT	not supported						*/
/*				0 or positive:			read block count					*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_read_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
	grp_uchar_t		*pucBuf,				/* [OUT] I/O buffer */
	grp_isize_t		iCnt)					/* [IN]  I/O count */
{
	grp_int32_t		iRet;					/* return value */
	grp_fs_dev_op_t	*ptDevOp;				/* device operation */

	ptDevOp = _grp_fs_check_dev_no(iDev);	/* check device number */
	if (ptDevOp == NULL)					/* invalid device number */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	if (ptDevOp->pfnRead == NULL)			/* no read function */
		return(GRP_FS_ERR_NOT_SUPPORT);		/* return error */
	iRet = ptDevOp->pfnRead(iHandle, iDev, uiDevBlk, pucBuf, iCnt);
											/* read device */
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_write_dev											*/
/*																			*/
/* DESCRIPTION:	Write device												*/
/*				Note: Address space conversion should be added to support	*/
/*					  multi-address-space cnfiguration						*/
/* INPUT:		iHandle,				I/O handle							*/
/* 				iDev:					device number						*/
/*				uiDevBlk:				device block number					*/
/*				iCnt:					I/O count							*/
/* 				pucBuf:					data to write						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:			I/O error							*/
/*				GRP_FS_ERR_NOT_SUPPORT	not supported						*/
/*				0 or positive:			written block count					*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_write_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
	grp_uchar_t		*pucBuf,				/* [IN]  data to write */
	grp_isize_t		iCnt)					/* [IN]  I/O count */
{
	grp_int32_t		iRet;					/* return value */
	grp_fs_dev_op_t	*ptDevOp;				/* device operation */

	ptDevOp = _grp_fs_check_dev_no(iDev);	/* check device number */
	if (ptDevOp == NULL)					/* invalid device number */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	if (ptDevOp->pfnWrite == NULL)			/* no write function */
		return(GRP_FS_ERR_NOT_SUPPORT);		/* return error */
	iRet = ptDevOp->pfnWrite(iHandle, iDev, uiDevBlk, pucBuf, iCnt);
											/* write device */
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_ioctl_dev											*/
/*																			*/
/* DESCRIPTION:	I/O control device											*/
/* INPUT:		iDev:					device number						*/
/*				uiCmd:					I/O control command					*/
/*				pvParam:				parameter							*/
/* OUTPUT:		(pvParam:				result)								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:			I/O error							*/
/*				GRP_FS_ERR_NOT_SUPPORT:	not supported						*/
/*				GRP_FS_ERR_BAD_PARAM:	invalid pvParam						*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_fs_ioctl_dev(
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiCmd,					/* [IN]  I/O control command */
	void			*pvParam)				/* [IN/OUT] parameter/result */
{
	int				iRet;					/* return value */
	grp_int32_t		iSize;					/* I/O size */
	char			*pcIoBuf;				/* I/O buffer */
	grp_fs_dev_op_t	*ptDevOp;				/* device operation */
	long			aiBuf[GRP_FS_DEV_CTL_BUF/sizeof(long)]; /* ioctl buffer */
	
	ptDevOp = _grp_fs_check_dev_no(iDev);	/* check device number */
	if (ptDevOp == NULL)					/* invalid device number */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	if (ptDevOp->pfnIoctl == NULL)			/* no ioctl operation */
		return(GRP_FS_ERR_NOT_SUPPORT);		/* return error */
	iSize = GRP_FS_DEV_CTL_SZ(uiCmd);		/* I/O size */
	if (iSize > sizeof(aiBuf)) {			/* over local buffer */
		pcIoBuf = grp_mem_alloc(iSize);		/* allocate buffer */
		if (pcIoBuf == NULL)				/* no buffer */
			return(GRP_FS_ERR_NOMEM);		/* return error */
	} else									/* within local buffer */
		pcIoBuf = (char *)aiBuf;			/* use local buffer */
	if (iSize
		&& (uiCmd & GRP_FS_DEV_WT_CTL_BIT)) { /* read type ioctl */
		if (grp_fs_copyin(pcIoBuf, pvParam, iSize) != iSize) { /* get data */
			iRet = GRP_FS_ERR_BAD_PARAM;	/* set error number */
			goto out;						/* return error */
		}
	}
	iRet = ptDevOp->pfnIoctl(iDev, uiCmd, pcIoBuf); /* ioctl device */
	if (iRet == 0 && iSize 
		&& (uiCmd & GRP_FS_DEV_RD_CTL_BIT)) { /* write type ioctl */
		if (grp_fs_copyout(pvParam, pcIoBuf, iSize) != iSize) /* put data */
			iRet = GRP_FS_ERR_BAD_PARAM;	/* set error number */
	}
out:
	if (iSize > sizeof(aiBuf))				/* over local buffer */
		grp_mem_free(pcIoBuf);				/* free buffer */
	return(iRet);							/* return */
}
