/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_io_disk_part.c										*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		I/O disk partition information										*/
/* FUNCTIONS:																*/
/*		grp_fs_read_part			read partition information				*/
/*		grp_fs_write_part			write partiontion information			*/
/* DEPENDENCIES:															*/
/*		grp_fs_disk_part.h													*/
/*		grp_fs_if.h															*/
/*		grp_fs_cfg.h														*/
/*		grp_fs.h															*/
/*		grp_fat_format.h													*/
/*		<string.h>															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2004/07/25	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10 	Added type casts for 16 bit CPU support	*/
/*		T.Imashiki		2007/02/20	Added type casts and changed type of	*/
/*									some variables for 16 bit CPU support	*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2010 Grape Systems, Inc.,  All Rights Reserved.        */
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
#include "grp_fs_disk_part.h"
#include "grp_fs_if.h"
#include "grp_fs_cfg.h"
#include "grp_fs.h"
#include "grp_fat_format.h"
#include <string.h>

#if(GRP_FS_MINIMIZE_LEVEL < 1)
grp_fs_default_part_type_t grp_fs_default_part_type = {
	GRP_FS_PART_FAT12,							/* for FAT12 */
	GRP_FS_PART_FAT16_L32,						/* for FAT16 < 32MB */
	GRP_FS_PART_FAT16_H32,						/* for FAT16 >= 32MB */
	GRP_FS_PART_FAT32_LBA						/* for FAT32 */
};												/* default partition type */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_read_part											*/
/*																			*/
/* DESCRIPTION:	read partition table										*/
/* INPUT:		pcDev:				device name								*/
/* OUTPUT:		ptPart:				partition table							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV	bad device name							*/
/*				GRP_FS_ERR_NOMEM: 	cannot allocate buffer					*/
/* 				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_FS:		partition less							*/
/*				GRP_FS_ERR_NOT_FOUND: not found partition table				*/
/*				0:					success to read							*/
/*																			*/
/****************************************************************************/
int
grp_fs_read_part(
	const char			*pcDev,					/* [IN]  device name */
	grp_fs_dk_part_t	*ptPart)				/* [OUT] partition table */
{
	int					iDev;					/* device number */
	int					iMajor;					/* major device number */
	grp_int32_t			iHandle;				/* I/O handle */
	int					iRet;					/* return value */
	grp_int32_t			iRead;					/* read count */
	grp_fs_dev_op_t		*ptDevOp;				/* device operation */
	grp_uint32_t		uiStartSec;				/* start sector */
	grp_uint32_t		uiTotalSec;				/* total sectors */
	int					iSecShift;				/* sector shift count */
	grp_uchar_t			*pucBuf = NULL;			/* I/O buffer */

	/****************************************************/
	/* lookup device									*/
	/****************************************************/
	iDev = grp_fs_lookup_dev(pcDev);			/* lookup device */
	if (iDev < 0)								/* not found */
		return(iDev);							/* return error */
	if (GRP_FS_DEV_PART(iDev) != GRP_FS_DEV_RAW_PART) /* not raw partition */
		return(GRP_FS_ERR_BAD_DEV);				/* return error */
	iMajor = GRP_FS_DEV_MAJOR(iDev);			/* major device number */
	ptDevOp = grp_fs_dev_tbl[iMajor].ptOp;		/* device operation */

	/****************************************************/
	/* get open device									*/
	/****************************************************/
	iRet = ptDevOp->pfnOpen(iDev, 0, &iHandle,
							&uiStartSec, &uiTotalSec,
							&iSecShift);		/* open device */
	if (iRet != 0)								/* open failed */
		return(iRet);							/* return error */

	/****************************************************/
	/* allocate I/O buffer								*/
	/****************************************************/
	pucBuf = grp_mem_alloc((grp_int32_t)1 << iSecShift);
												/* allocate buffer */
	if (pucBuf == NULL) {						/* failed to allocate */
		iRet = GRP_FS_ERR_NOMEM;				/* set error number */
		goto out;								/* return error */
	}

	/****************************************************/
	/* read 1st sector and check partition				*/
	/****************************************************/
	iRead = ptDevOp->pfnRead(iHandle, iDev,
					uiStartSec, pucBuf, 1);		/* read 1st sector */
	if (iRead != 1) {							/* I/O error */
		iRet = GRP_FS_ERR_IO;					/* set error number */
		goto out;								/* return error */
	}
	iRet = grp_fs_get_part(pucBuf, ptPart);		/* get partition info */
	iRet = (iRet == GRP_FS_PART_VALID)? 0:		/* success to get */
			(iRet == GRP_FS_PART_LESS)? GRP_FS_ERR_FS: /* no partition */
			GRP_FS_ERR_NOT_FOUND;				/* not found partition tableI */

	/****************************************************/
	/* return result									*/
	/****************************************************/
out:
	if (pucBuf)									/* buffer allocated */
		grp_mem_free(pucBuf);					/* free it */
	ptDevOp->pfnClose(iHandle, iDev);			/* close device */
	return(iRet);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/****************************************************************************/
/* FUNCTION:	grp_fs_write_part											*/
/*																			*/
/* DESCRIPTION:	write partition table										*/
/* INPUT:		pcDev:				device name								*/
/*				iAuto:				automatically make 1 partition			*/
/*				ptPart:				partition table							*/
/* OUTPUT:		ptPart:				partition table							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV:	bad device name							*/
/*				GPR_FS_ERR_BUSY:	device is mounted						*/
/*				GRP_FS_ERR_NOMEM: 	not enough buffer						*/
/* 				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_FS:		bad partition information 				*/
/*				0:					success to read							*/
/*																			*/
/****************************************************************************/
int
grp_fs_write_part(
	const char			*pcDev,					/* [IN]  device name */
	int					iAuto,					/* [IN]  make 1 partition */
	grp_fs_dk_part_t	*ptPart)				/* [OUT] partition table */
{
	int					iDev;					/* device number */
	int					iMajor;					/* major device number */
	grp_int32_t			iHandle;				/* I/O handle */
	int					iRet;					/* return value */
	grp_int32_t			iWrite;					/* write count */
	grp_fs_dev_op_t		*ptDevOp;				/* device operation */
	grp_fs_media_info_t	tMedia;					/* media information */
	grp_fs_dk_chs_t		*ptStart;				/* start CHS */
	grp_fs_dk_chs_t		*ptEnd;					/* end CHS */
	grp_uint32_t		uiSec;					/* sector */
	grp_uint32_t		uiHead;					/* head */
	grp_uint32_t		uiCyl;					/* cylinder */
	grp_uchar_t			*pucBuf = NULL;			/* I/O buffer */
	grp_fat_format_param_t tReq;				/* format request param */
	grp_fat_format_param_t tRes;				/* format result param */

	/****************************************************/
	/* lookup device									*/
	/****************************************************/
	iDev = grp_fs_lookup_dev(pcDev);			/* lookup device */
	if (iDev < 0)								/* not found */
		return(iDev);							/* return error */
	if (GRP_FS_DEV_PART(iDev) != GRP_FS_DEV_RAW_PART) /* not raw partition */
		return(GRP_FS_ERR_BAD_DEV);				/* return error */
	if (grp_fs_check_dev_busy(iDev))			/* under mounted */
		return(GRP_FS_ERR_BUSY);				/* return busy */
	iMajor = GRP_FS_DEV_MAJOR(iDev);			/* major device number */
	ptDevOp = grp_fs_dev_tbl[iMajor].ptOp;		/* device operation */

	/****************************************************/
	/* get open device									*/
	/****************************************************/
	memset(&tMedia, 0, sizeof(tMedia));			/* clear media info */
	iRet = ptDevOp->pfnOpen(iDev, 1, &iHandle,
							&tMedia.uiStartSec, &tMedia.uiTotalSec,
							&tMedia.iSecShift);	/* open device */
	if (iRet != 0)								/* open failed */
		return(iRet);							/* return error */

	/****************************************************/
	/* allocate I/O buffer								*/
	/****************************************************/
	pucBuf = grp_mem_alloc((grp_int32_t)1 << tMedia.iSecShift);
												/* allocate buffer */
	if (pucBuf == NULL) {						/* failed to allocate */
		iRet = GRP_FS_ERR_NOMEM;				/* set error number */
		goto out;								/* return error */
	}

	/****************************************************/
	/* get media information if possible for auto set	*/
	/* 													*/
	/* Try to get media information by device dependent	*/
	/* ioctl first, and then use default value if not	*/
	/* obtained.										*/
	/****************************************************/
	if (iAuto == 0)								/* not auto */
		goto set_data;							/* goto set_data */
	if (ptDevOp->pfnIoctl) {					/* exists ioctl operation */
		iRet = ptDevOp->pfnIoctl(iDev, GRP_FS_DEV_CTL_GET_MEDIA, &tMedia);
												/* get media information */
		if (iRet < 0 && iRet != GRP_FS_ERR_NOT_SUPPORT)/* failed to get */
			goto out;							/* return error */
	}
	if (tMedia.uiTotalSec == 0) {				/* no total sector info */
		iRet = GRP_FS_ERR_BAD_PARAM;			/* invaid parameter */
		goto out;								/* return error */
	}
	if (tMedia.usTrkSec == 0) {					/* no sector/track info */
		tMedia.usTrkSec = GRP_FS_PART_TRK_SEC;	/* set default sector/track */
		tMedia.usHead = GRP_FS_PART_HEAD;		/* set default head */
	}

	/****************************************************/
	/* calculate start/end sector; skip 1st track for	*/
	/* start of patition								*/
	/****************************************************/
	memset(ptPart, 0, sizeof(*ptPart) * GRP_FS_PART_CNT);
												/* clear partion table */
	if (tMedia.uiTotalSec <= (grp_uint32_t)tMedia.usTrkSec) {
		iRet = GRP_FS_ERR_BAD_PARAM;			/* invalid parameter */
		goto out;
	}
	ptPart->ucActive = GRP_FS_PART_ACT;			/* active partition */
	ptPart->uiStartSec = tMedia.usTrkSec;		/* 2nd track */
	ptPart->uiSecCnt = tMedia.uiTotalSec - ptPart->uiStartSec;
												/* set sector count */
	ptStart = &ptPart->tStartCHS;				/* start */
	ptStart->ucSec = 1;							/* start sector */
	if (tMedia.usHead <= 1) {					/* less than 1 head */
		ptStart->ucHead = 0;					/* start head */
		ptStart->usCyl = 1;						/* start cylinder */
	} else {
		ptStart->ucHead = 1;					/* start head */
		ptStart->usCyl = 0;						/* start cylinder */
	}
	uiSec = tMedia.uiTotalSec - 1;				/* end sector */
	ptEnd = &ptPart->tEndCHS;					/* start */
	uiCyl = uiSec / ((grp_uint32_t)tMedia.usTrkSec * tMedia.usHead);
												/* cylinder */
	ptEnd->usCyl = (grp_int16_t)((uiCyl > GRP_FS_PART_MAX_CYL)?
					GRP_FS_PART_MAX_CYL: uiCyl);/* set cylinder */
	uiSec -= (uiCyl * (grp_uint32_t)tMedia.usTrkSec * tMedia.usHead);
												/* remain sectors */
	uiHead = uiSec / tMedia.usTrkSec;			/* head */
	ptEnd->ucHead = (grp_uchar_t)((uiHead > GRP_FS_PART_MAX_HEAD)?
					GRP_FS_PART_MAX_HEAD: uiHead);/* set head */
	uiSec = uiSec - (grp_uint32_t)ptEnd->ucHead * tMedia.usTrkSec + 1;
												/* sector */
	ptEnd->ucSec = (grp_uchar_t) ((uiSec > GRP_FS_PART_MAX_SEC)?
					GRP_FS_PART_MAX_SEC: uiSec);/* set sector */

	/****************************************************/
	/* get FAT type and set partition type				*/
	/****************************************************/
	memset((void *)&tReq, 0, sizeof(tReq));		/* clear request param */
	iRet = grp_fat_find_type(tMedia.uiTotalSec, 0, tMedia.iSecShift,
							&tReq, &tRes);		/* find FAT type */
	if (iRet < 0)								/* error */
		goto out;								/* return error */
	switch(tRes.ucFatType) {
	case GRP_FAT_TYPE_12:						/* FAT12 */
		ptPart->ucPartType = grp_fs_default_part_type.ucPartType12;
		break;
	case GRP_FAT_TYPE_16:						/* FAT16 */
		ptPart->ucPartType = (grp_uchar_t)
			((tMedia.uiTotalSec << tMedia.iSecShift < 0x2000000)?
			grp_fs_default_part_type.ucPartType16Small:	/* <  32MB */
			grp_fs_default_part_type.ucPartType16Big);	/* >= 32MB */
		break;
	case GRP_FAT_TYPE_32:						/* FAT32 */
		ptPart->ucPartType = grp_fs_default_part_type.ucPartType32;
		break;
	}											/* set partition type */

	/****************************************************/
	/* set partition information						*/
	/****************************************************/
set_data:
	memset(pucBuf, 0, (grp_size_t)((grp_int32_t)1 << tMedia.iSecShift));
												/* clear buffer */
	iRet = grp_fs_set_part(pucBuf, ptPart);		/* set partition */
	if (iRet != 0)								/* faield to set */
		goto out;								/* return error */

	/****************************************************/
	/* write partition information						*/
	/****************************************************/
	iWrite = ptDevOp->pfnWrite(iHandle, iDev,
					tMedia.uiStartSec, pucBuf, 1); /* write 1st sector */
	iRet = (iWrite == 1)? 0: GRP_FS_ERR_IO;		/* set error number */

	/****************************************************/
	/* return result									*/
	/****************************************************/
out:
	if (pucBuf)									/* buffer allocated */
		grp_mem_free(pucBuf);					/* free it */
	ptDevOp->pfnClose(iHandle, iDev);			/* close device */
	return(iRet);								/* return */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
