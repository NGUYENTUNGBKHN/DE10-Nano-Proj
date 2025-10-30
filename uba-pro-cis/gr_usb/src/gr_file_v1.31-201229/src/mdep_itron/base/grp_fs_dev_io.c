/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_dev_io.c												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Device I/O routine(empty stub)										*/
/* FUNCTIONS:																*/
/*		_grp_fs_open_dev			open device								*/
/*		_grp_fs_close_dev			close device							*/
/*		_grp_fs_read_dev			read device								*/
/*		_grp_fs_write_dev			write device							*/
/*		_grp_fs_ioctl_dev			ioctl device							*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_fs_cfg.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		K.Kaneko		2008/05/21	Created inital version 1.0				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2008 Grape Systems, Inc.,  All Rights Reserved.             */
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
#include "grp_fs_cfg.h"

#ifdef GRP_FS_PORTING_DEVICE_IO

static grp_fs_dev_open_t	_grp_fs_open_dev;	/* open device */
static grp_fs_dev_close_t	_grp_fs_close_dev;	/* close device */
static grp_fs_dev_read_t	_grp_fs_read_dev;	/* read device */
static grp_fs_dev_write_t	_grp_fs_write_dev;	/* write device */
static grp_fs_dev_ioctl_t	_grp_fs_ioctl_dev;	/* ioctl device */

grp_fs_dev_op_t		grp_fs_dev_op = {			/* device I/O */
	_grp_fs_open_dev,							/* open device */
	_grp_fs_close_dev,							/* close device */
	_grp_fs_read_dev,							/* read device */
	_grp_fs_write_dev,							/* write device */
	_grp_fs_ioctl_dev							/* ioctl device */
};

/****************************************************************************/
/* FUNCTION:	_grp_fs_open_dev											*/
/*																			*/
/* DESCRIPTION:	Open device													*/
/* INPUT:		iDev:					device number						*/
/*				iRWOpen:				read/write open						*/
/* OUTPUT:		piHandle:				I/O handle							*/
/*				puiOff:					start offset						*/
/*				puiSize:				size of device						*/
/*				piSzShift:				block size shift					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV:		bad device name/number				*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_open_dev(
	int				iDev,					/* [IN]  device number */
	int				iRWOpen,				/* [IN]  read/write open */
	grp_int32_t		*piHandle,				/* [OUT] I/O handle */
	grp_uint32_t	*puiOff,				/* [OUT] start offset */
	grp_uint32_t	*puiSize,				/* [OUT] size of device */
	int				*piSzShift)				/* [OUT] shift of size */
{

	*****************************************************
	* after confirmation, please delete this message	*
	*													*
	* device open code here								*
	*	return piHandle is open device handle.			*
	*	return puiOff is open partition offset			*
	*	return puiSize is open partition size			*
	*	return piSzShift is open device sector size		*
	*****************************************************

	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_close_dev											*/
/*																			*/
/* DESCRIPTION:	Close device												*/
/* INPUT:		iHandle,				I/O handle							*/
/*				iDev:					device number						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:						success								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_close_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev)					/* [IN]  device number */
{

	*****************************************************
	* after confirmation, please delete this message	*
	*													*
	* device close code here							*
	*****************************************************

	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_read_dev											*/
/*																			*/
/* DESCRIPTION:	Read device													*/
/* INPUT:		iHandle,				I/O handle							*/
/*				iDev:					device number						*/
/*				uiDevBlk:				device block number					*/
/*				iCnt:					I/O count							*/
/* OUTPUT:		pucBuf:					data read							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV:		bad handle							*/
/*				0 or positive:			read count							*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_grp_fs_read_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
	grp_uchar_t		*pucBuf,				/* [OUT] I/O buffer */
	grp_isize_t		iCnt)					/* [IN]  I/O count */
{

	*****************************************************
	* after confirmation, please delete this message	*
	*													*
	* device read code here								*
	*****************************************************

	return(iCnt);							/* return read count */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_write_dev											*/
/*																			*/
/* DESCRIPTION:	Write device												*/
/* INPUT:		iHandle,				I/O handle							*/
/*				iDev:					device number						*/
/*				uiDevBlk:				device block number					*/
/*				iCnt:					I/O count							*/
/* 				pucBuf:					data to write						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV:		bad handle							*/
/*				0 or positive:			written count						*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_grp_fs_write_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
	grp_uchar_t		*pucBuf,				/* [IN]  data to write */
	grp_isize_t		iCnt)					/* [IN]  I/O count */
{

	*****************************************************
	* after confirmation, please delete this message	*
	*													*
	* device write code here							*
	*****************************************************

	return(iCnt);							/* return written count */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_ioctl_dev											*/
/*																			*/
/* DESCRIPTION:	ioctl device												*/
/* INPUT:		iDev:					device number						*/
/*				uiCmd:					command								*/
/*				pvParam:				parameter (for some commands)		*/
/* OUTPUT:		pvParam					result (for some commands)			*/
/*																			*/
/* RESULT:		GRP_FS_ERR_NOT_SUPPORT:	not supported						*/
/*				GRP_FS_ERR_BAD_DEV:		invalid device number				*/
/*				GRP_FS_ERR_TOO_MANY:	too many opens						*/
/*				GRP_FS_ERR_BUSY:		device busy							*/
/*				GRP_FS_ERR_IO:			I/O error							*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_grp_fs_ioctl_dev(
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiCmd,					/* [IN]  command number */
	void			*pvParam)				/* [IN/OUT] parameter/result */
{

	*****************************************************
	* after confirmation, please delete this message	*
	*													*
	* device ioctl code here							*
	*****************************************************

	return(0);								/* return */
}

#endif	/* GRP_FS_PORTING_DEVICE_IO */
