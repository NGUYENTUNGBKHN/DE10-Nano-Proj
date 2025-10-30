#ifndef	_GRP_FS_DEV_IO_IF_H_
#define	_GRP_FS_DEV_IO_IF_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_dev_io_if.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for exported device I/O interfaces						*/
/* FUNCTIONS:																*/
/*		grp_fs_open_dev				open device								*/
/*		grp_fs_close_dev			close device							*/
/*		grp_fs_read_dev				read device								*/
/*		grp_fs_write_dev			write device							*/
/*		grp_fs_ioctl_dev			I/O control device						*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2004/07/25	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10	Added type casts for 16 bit CPU support	*/
/*									Fixed return type of grp_fs_read_dev	*/
/*		K.Kaneko		2011/05/31	Added define GRP_FS_DEV_CTL_GET_WRITE	*/
/*									_PROTECT								*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2011 Grape Systems, Inc.,  All Rights Reserved.        */
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

#include "grp_types.h"

/****************************************************************************/
/*  macros for device number 												*/
/****************************************************************************/
#define GRP_FS_DEV_MAJOR(iDev)					/* major device number */	\
	(((iDev) >> 8) & 0xff)
#define GRP_FS_DEV_MINOR(iDev)					/* major device number */	\
	((iDev) & 0xff)
#define GRP_FS_DEV_NO(iMajor, iMinor)			/* make devince number */	\
	((((iMajor) & 0xff) << 8) | ((iMinor) & 0xff))
#define GRP_FS_DEV_PART(iDev)					/* partition number */		\
	(((iDev) >> 4) & 0x0f)
#define GRP_FS_DEV_RAW_PART		0x0f			/* raw device(no partition) */
#define GRP_FS_DEV_SUBID(iDev)					/* sub-device ID */			\
	((iDev) & 0x0f)
#define GRP_FS_DEV_MK_MINOR(iSubId, iPart)		/* make minor device ID */	\
	((((iPart) << 4) & 0xf0) | ((iSubId) & 0x0f))
#define GRP_FS_DEV_MATCH_SUB(iDev1, iDev2)		/* match sub device */		\
	(((iDev1) & 0xff0f) == ((iDev2) & 0xff0f))

/****************************************************************************/
/*  device I/O information													*/
/****************************************************************************/
typedef struct grp_fs_dev_io_info {
		grp_int32_t		iHandle;				/* I/O handle */
		grp_uint32_t	uiOff;					/* start offset */
		grp_uint32_t	uiSize;					/* size of device */
		int				iSzShift;				/* shift count for size unit */
} grp_fs_dev_io_info_t;

/****************************************************************************/
/*  exported interfaces														*/
/****************************************************************************/
int grp_fs_open_dev(							/* open device */
		int				iDev,					/* [IN]  device number */
		int				iRWOpen,				/* [IN]  read/write open */
		grp_fs_dev_io_info_t *ptDevIoInfo);		/* [OUT] device I/O info */

int grp_fs_close_dev(							/* close device */
		grp_int32_t		iHandle,				/* [IN]  I/O handle */
		int				iDev);					/* [IN]  device number */

grp_int32_t grp_fs_read_dev(					/* read device */
		grp_int32_t		iHandle,				/* [IN]  I/O handle */
		int				iDev,					/* [IN]  device number */
		grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
		grp_uchar_t		*pucBuf,				/* [OUT] I/O buffer */
		grp_isize_t		iCnt);					/* [IN]  I/O count */

grp_int32_t grp_fs_write_dev(					/* write device */
		grp_int32_t		iHandle,				/* [IN]  I/O handle */
		int				iDev,					/* [IN]  device number */
		grp_uint32_t	uiDevBlk,				/* [IN]  device block number */
		grp_uchar_t		*pucBuf,				/* [IN]  I/O buffer */
		grp_isize_t		iCnt);					/* [IN]  I/O count */

int grp_fs_ioctl_dev(							/* I/O control device */
		int				iDev,					/* [IN]  device number */
		grp_uint32_t	uiCmd,					/* [IN]  command number */
		void			*pvParam);				/* [IN/OUT] parameter */

/************************************************/
/* ioctl commands 								*/
/************************************************/
#define GRP_FS_DEV_RD_CTL_BIT	0x80000000	/* read type control bit */
#define GRP_FS_DEV_WT_CTL_BIT	0x40000000	/* write type control bit */
#define GRP_FS_DEV_RW_CTL_BIT	(GRP_FS_DEV_RD_CTL_BIT|GRP_FS_DEV_WT_CTL_BIT)

#define GRP_FS_DEV_RD_CTL(num, param_type)	/* read type ioctl */		\
	(GRP_FS_DEV_RD_CTL_BIT | ((grp_uint32_t)(num) << 16) | sizeof(param_type))
#define GRP_FS_DEV_WT_CTL(num, param_type)	/* write type ioctl */		\
	(GRP_FS_DEV_WT_CTL_BIT | ((grp_uint32_t)(num) << 16) | sizeof(param_type))
#define GRP_FS_DEV_RW_CTL(num, param_type)	/* read/write type ioctl */	\
	(GRP_FS_DEV_RW_CTL_BIT | ((grp_uint32_t)(num) << 16) | sizeof(param_type))
#define GRP_FS_DEV_NP_CTL(num)				/* no param ioctl */		\
	((grp_uint32_t)(num) << 16)
#define GRP_FS_DEV_CTL_SZ(cmd)				/* ioctl I/O size */		\
	((cmd) & 0xffff)

#define GRP_FS_DEV_CTL_GET_MEDIA			/* get media information */	\
		GRP_FS_DEV_RD_CTL(1, grp_fs_media_info_t)
#define GRP_FS_DEV_CTL_EJECT				/* control media eject */	\
		GRP_FS_DEV_WT_CTL(2, int)
#define GRP_FS_DEV_CTL_FORMAT				/* make physical formating */\
		GRP_FS_DEV_NP_CTL(3)
#define GRP_FS_DEV_CTL_GET_WRITE_PROTECT	/* get write protect */	\
		GRP_FS_DEV_RD_CTL(4, int)

/************************************************/
/* ioctl parameters 							*/
/************************************************/
/************************************************/
/* GRP_FS_DEV_CTL_GET_MEDIA (get media info)	*/
/************************************************/
typedef struct grp_fs_media_info {				/* media information */
	grp_uint32_t			uiStartSec;			/* start sector offset */
	grp_uint32_t			uiTotalSec;			/* sector count */
	grp_uint16_t			usTrkSec;			/* sector/track */
	grp_uint16_t			usHead;				/* heads */
	int						iSecShift;			/* shift val for sector size */
	grp_uchar_t				ucMediaType;		/* media type */
} grp_fs_media_info_t;

/* ucMediaType */
#define GRP_FS_MEDIA_REMOVABLE	0xf0			/* removable media */
#define GRP_FS_MEDIA_FIXED		0xf8			/* fixed media */
#define GRP_FS_MEDIA_2DD		0xf9			/* 2DD floppy */
#define GRP_FS_MEDIA_2HD		0xf0			/* 2HD floppy */

/************************************************/
/* GRP_FS_DEV_CTL_EJECT(eject control) (int)	*/
/************************************************/
#define GRP_FS_DEV_LOCK_MEDIA	0				/* lock media */
#define GRP_FS_DEV_UNLOCK_MEDIA	1				/* unlock media */
#define GRP_FS_DEV_EJECT_MEDIA	2				/* eject media */

#endif	/* _GRP_FS_DEV_IO_IF_H_ */
