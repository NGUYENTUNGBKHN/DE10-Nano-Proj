/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_dev_io_ram.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Device I/O routine for RAM disk										*/
/* FUNCTIONS:																*/
/*		grp_fs_dev_io_ram_init		init RAM disk							*/
/*		_grp_fs_ram_open_dev		open device								*/
/*		_grp_fs_ram_close_dev		close device							*/
/*		_grp_fs_ram_read_dev		read device								*/
/*		_grp_fs_ram_write_dev		write device							*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_fs_cfg.h														*/
/*		grp_fs_dev_io_ram.h													*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/12/14	Created inital version 1.0				*/
/*		T.Imashiki		2004/07/25	Added sub-device id and partition		*/
/*									handling								*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
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

#include <string.h>
#include "grp_fs_sysdef.h"
#include "grp_fs_cfg.h"
#include "grp_fs_dev_io_ram.h"

#ifdef	GRP_FS_RAM_DISK

static grp_fs_dev_open_t	_grp_fs_ram_open_dev;/* open device */
static grp_fs_dev_close_t	_grp_fs_ram_close_dev;/* close device */
static grp_fs_dev_read_t	_grp_fs_ram_read_dev;/* read device */
static grp_fs_dev_write_t	_grp_fs_ram_write_dev;/* write device */

grp_fs_dev_op_t		grp_fs_dev_op_ram = {		/* device I/O for RAM disk */
	_grp_fs_ram_open_dev,						/* open device */
	_grp_fs_ram_close_dev,						/* close device */
	_grp_fs_ram_read_dev,						/* read device */
	_grp_fs_ram_write_dev						/* write device */
};

grp_fs_dev_ram_info_t grp_fs_dev_ram_info[GRP_FS_DEV_RAM_NDISK];
												/* RAM disk info table */

/****************************************************************************/
/* FUNCTION:	grp_fs_dev_io_ram_init										*/
/*																			*/
/* DESCRIPTION:	Initialize a RAM disk										*/
/* INPUT:		iDiskNo:				RAM disk number						*/
/*				pucStart:				start address						*/
/*				iSecCount:				sector count						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV:		bad RAM disk number					*/
/*				GRP_FS_ERR_BUSY:		already initialized					*/
/*				GRP_FS_ERR_BAD_PARAM:	bad sector count parameter			*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_fs_dev_io_ram_init(
	int				iDiskNo,				/* [IN]  RAM disk number */
	grp_uchar_t		*pucStart,				/* [IN]  start address */
	grp_uint32_t	uiSecCount)				/* [IN]  sector count */
{
	grp_fs_dev_ram_info_t	*ptRam;			/* RAM disk info table */

	if (iDiskNo < 0 || iDiskNo >= GRP_FS_DEV_RAM_NDISK)
		return(GRP_FS_ERR_BAD_DEV);			/* bad RAM disk number */
	if (uiSecCount <= 0)					/* bad sector count */
		return(GRP_FS_ERR_BAD_PARAM);		/* bad sector count parameter */
	ptRam = &grp_fs_dev_ram_info[iDiskNo];	/* RAM disk info table */
	if (ptRam->uiSecCount != 0)				/* already initialized */
		return(GRP_FS_ERR_BUSY);			/* return busy error */
	ptRam->pucStart = pucStart;				/* set start address */
	ptRam->uiSecCount = uiSecCount;			/* set sector count */
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_ram_open_dev										*/
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
_grp_fs_ram_open_dev(
	int				iDev,					/* [IN]  device number */
	int				iRWOpen,				/* [IN]  read/write open */
	grp_int32_t		*piHandle,				/* [OUT] I/O handle */
	grp_uint32_t	*puiOff,				/* [OUT] start offset */
	grp_uint32_t	*puiSize,				/* [OUT] size of device */
	int				*piSzShift)				/* [OUT] shift of size */
{
	int				iMajor;					/* major device number */
	int				iSubId;					/* sub device ID */
	int				iPart;					/* partition number */
	grp_fs_dev_ram_info_t	*ptRam;			/* RAM disk info table */

	iMajor = GRP_FS_DEV_MAJOR(iDev);		/* major device */
	iSubId = GRP_FS_DEV_SUBID(iDev);		/* sub device ID */
	iPart = GRP_FS_DEV_PART(iDev);			/* partition number */
	if (iSubId < 0 || iSubId >= GRP_FS_DEV_RAM_NDISK || iPart != 0)
											/* invalid device no */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	ptRam = &grp_fs_dev_ram_info[iSubId];	/* RAM disk info table */
	if (ptRam->uiSecCount == 0)				/* not initialized */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	*piSzShift = GRP_FS_DEV_RAM_SECSFT;		/* set sector shift count */
	*puiSize = ptRam->uiSecCount;			/* set sector count */
	*puiOff = 0;							/* set offset */
	*piHandle = iSubId;						/* set handle */
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_ram_close_dev										*/
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
_grp_fs_ram_close_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev)					/* [IN]  device number */
{
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_ram_read_dev										*/
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
_grp_fs_ram_read_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
	grp_uchar_t		*pucBuf,				/* [OUT] I/O buffer */
	grp_isize_t		iCnt)					/* [IN]  I/O count */
{
	grp_fs_dev_ram_info_t	*ptRam;			/* RAM disk info table */

	if (iHandle < 0 || iHandle >= GRP_FS_DEV_RAM_NDISK) /* invalid device no */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	ptRam = &grp_fs_dev_ram_info[iHandle];	/* RAM disk info table */
	if (ptRam->uiSecCount == 0)				/* not initialized */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	memcpy((char *)pucBuf,
			(char *)ptRam->pucStart + (uiDevBlk << GRP_FS_DEV_RAM_SECSFT),
			(iCnt << GRP_FS_DEV_RAM_SECSFT));/* get data */
	return(iCnt);							/* return read count */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_ram_write_dev										*/
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
_grp_fs_ram_write_dev(
	grp_int32_t		iHandle,				/* [IN]  I/O handle */
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
	grp_uchar_t		*pucBuf,				/* [IN]  data to write */
	grp_isize_t		iCnt)					/* [IN]  I/O count */
{
	grp_fs_dev_ram_info_t	*ptRam;			/* RAM disk info table */

	if (iHandle < 0 || iHandle >= GRP_FS_DEV_RAM_NDISK) /* invalid device no */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	ptRam = &grp_fs_dev_ram_info[iHandle];	/* RAM disk info table */
	if (ptRam->uiSecCount == 0)				/* not initialized */
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	memcpy((char *)ptRam->pucStart + (uiDevBlk << GRP_FS_DEV_RAM_SECSFT),
			(char *)pucBuf,
			(iCnt << GRP_FS_DEV_RAM_SECSFT));/* write data */
	return(iCnt);							/* return written count */
}

#endif	/* GRP_FS_RAM_DISK */
