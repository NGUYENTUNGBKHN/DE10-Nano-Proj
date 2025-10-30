/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fat_format.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Format FAT file system												*/
/* FUNCTIONS:																*/
/*		grp_fat_format				format media							*/
/*		grp_fat_find_type			get FAT type							*/
/*																			*/
/* DEPENDENCIES:															*/
/*		fat.h																*/
/*		fat_format_def.h													*/
/*		grp_fat_format.h													*/
/*		grp_time.h															*/
/*		grp_fs_mdep_if.h													*/
/*		fat_format_def.h													*/
/*		grp_fs.h															*/
/*		<string.h>															*/
/*																			*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2004/07/25	Created initial version 1.0				*/
/*		T.Imashiki		2004/10/29	Fixed bug returning 0 instead of FAT	*/
/*									type number by grp_fat_format.			*/
/*		T.Imashiki		2004/11/12	Fixed clearing over to reserved area	*/
/*									in setting backup boot sector number.	*/
/*		T.Imashiki		2005/02/10 	Added type casts for 16 bit CPU support	*/
/*		T.Imashiki		2007/02/20 	Deleted redundant include of fat_format	*/
/*									_def.h									*/
/*		T.Imashiki		2007/02/20 	Added type casts and changed variable/  */
/*									parameter types of some functions for	*/
/*									16 bit CPU support						*/
/*		T.Imashiki		2007/06/01	Fixed bug setting bad EOC data for		*/
/*									FAT32 root directory					*/
/*		T.Imashiki		2007/06/11	Fixed incorrect bad character check in  */
/*									_fat_conv_vol_label					    */
/*		T.Imashiki		2007/09/18	Fixed bug setting incorrect EOC data	*/
/*									for FAT32 FAT0						    */
/*		K.kaneko		2008/05/12	Changed if volume label is NULL terminal*/
/*									then fill volume label in space in		*/
/*									_fat_conv_vol_label						*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		T.Imashiki		2010/11/16	Changed boot sector count of FAT32 to 3	*/
/*		K.Kaneko					and copy FSINFO to backup area for SDHC */
/*									Made strict character check for volume	*/
/*									label									*/
/*									Added a layer to get more optimal FAT	*/
/*									 sector count value for grp_fat_find	*/
/*									 _type									*/
/*									Supported logical sector format			*/
/*									Added _fat_need_sec and check of		*/
/*									 request FAT type at _fat_check_fat_type*/
/*									 to make consistent FAT type by			*/
/*									 increasing total sector count with		*/
/*									 default parameter						*/
/*									Fixed overflow in computing FAT sector	*/
/*									 count of FAT32 with near 4G sectors	*/
/*									 at _fat_check_fat_type					*/
/*		K.Kaneko		2010/11/16	Fixed logical sector format in partition*/
/*		K.Kaneko		2012/06/13	Fixed bug returning uiClst value in		*/
/*									grp_fat_find_type						*/
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
#include "fat.h"
#include "fat_format_def.h"
#include "grp_fat_format.h"
#include "grp_time.h"
#include "grp_fs_mdep_if.h"
#include "grp_fs.h"
#include <string.h>

/****************************************************************************/
/* format configuration parameters                                          */
/****************************************************************************/
/************************************************/
/* format config parameters						*/
/************************************************/
grp_fat_format_cfg_t	grp_fat_format_cfg = {	/* format config parameter */
	GRP_FAT_OEM_NAME,							/* format OEM name */
	GRP_FAT_SAFETY_GAP,							/* safety gap between types */
	GRP_FAT_IO_BUF_SZ,							/* I/O buffer size */
};

/************************************************/
/* format default parameters for each FAT type	*/
/************************************************/
grp_fat_default_t grp_fat_default[3] = {		/* default parameters */
	{ GRP_FAT_12_ROOT_CNT,						/* root entry count */
	  GRP_FAT_12_RSV_SEC,						/* reserved sector count */
	  GRP_FAT_12_TRK_SECS,						/* sector/track */
	  GRP_FAT_12_HEADS,							/* head count */
	},											/* default for FAT12 */
	{ GRP_FAT_16_ROOT_CNT,						/* root entry count */
	  GRP_FAT_16_RSV_SEC,						/* reserved sector count */
	  GRP_FAT_16_TRK_SECS,						/* sector/track */
	  GRP_FAT_16_HEADS,							/* head count */
	},											/* default for FAT16 */
	{ GRP_FAT_32_ROOT_CNT,						/* root entry count */
	  GRP_FAT_32_RSV_SEC,						/* reserved sector count */
	  GRP_FAT_32_TRK_SECS,						/* sector/track */
	  GRP_FAT_32_HEADS,							/* head count */
	},											/* default for FAT16 */
};

/************************************************/
/* cluster size limit table						*/
/************************************************/
grp_uint32_t grp_fat_cluster_limit_tbl[FAT_MAX_SEC_SHIFT-FAT_MIN_SEC_SHIFT] =
{
	GRP_FAT_512_CLST_LIMIT,						/* limit for 512 cluster */
	GRP_FAT_1K_CLST_LIMIT,						/* limit for 1K cluster */
	GRP_FAT_2K_CLST_LIMIT,						/* limit for 2K cluster */
	GRP_FAT_4K_CLST_LIMIT,						/* limit for 4K cluster */
	GRP_FAT_8K_CLST_LIMIT,						/* limit for 8K cluster */
	GRP_FAT_16K_CLST_LIMIT,						/* limit for 16K cluster */
};

#define GRP_FAT_MAX_FIND_TYPE_TRY	5			/* max find type try count */

/****************************************************************************/
/* FUNCTION:	_fat_get_clst_shift_limit									*/
/*																			*/
/* DESCRIPTION:	Select sector per cluster									*/
/* INPUT:		uiTotalSec:		total sector count							*/
/*				iSecShift:		shift count of sector size					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		sector per cluster											*/
/*																			*/
/****************************************************************************/
static int
_fat_get_clst_shift_limit(
	grp_uint32_t			uiTotalSec,			/* [IN]  total sector count */
	int						iSecShift)			/* [IN]  sector shift */
{
	int						iShift;				/* shift size */
	grp_uint32_t			uiSec;				/* 512 sector count */
	grp_uint32_t			uiLimit;			/* limit sector count */

	/****************************************************/
	/* convert to 512 sector count						*/
	/****************************************************/
	uiSec = (uiTotalSec << (iSecShift - FAT_MIN_SEC_SHIFT));
												/* 512 sector count */
	if (iSecShift > FAT_MIN_SEC_SHIFT
		&& (uiSec >> (iSecShift - FAT_MIN_SEC_SHIFT)) < uiTotalSec) {
												/* overflow */
		uiSec = 0xffffffff;						/* adjust to max sec */
	}

	/****************************************************/
	/* get limit shift with limit table 				*/
	/****************************************************/
	for (iShift = iSecShift; iShift < FAT_MAX_SEC_SHIFT; iShift++) {
		uiLimit = grp_fat_cluster_limit_tbl[iShift - FAT_MIN_SEC_SHIFT];
		if (uiLimit == 0 || uiLimit > uiSec)	/* found limit shift */
			break;								/* break */
	}
	return(iShift);								/* sector shift */
}

/****************************************************************************/
/* FUNCTION:	_fat_need_sec												*/
/*																			*/
/* DESCRIPTION:	Return total need sector count								*/
/* INPUT:		ptParam:		format parameter							*/
/*				uiClstCnt:		cluster count								*/
/*				uiFatRootSec:	FAT and root area sector count				*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		total need sector count										*/
/*																			*/
/****************************************************************************/
static grp_uint32_t
_fat_need_sec(
	grp_fat_format_param_t	*ptParam,			/* [IN]  format param */
	grp_uint32_t			uiClstCnt,			/* [IN]  cluster count */
	grp_uint32_t			uiFatRootSec)		/* [IN]  FAT and root sectors */
{
	grp_uint32_t			uiSec;				/* total need sector count */
	grp_uint32_t			uiStart;			/* cluster start offset */
	grp_uint32_t			uiAlign;			/* alignment */
	grp_uint32_t			uiRemain;			/* alignment remain count */

	uiStart = ptParam->uiRsvSecCnt + uiFatRootSec; /* start offset */
	uiSec = uiStart + (uiClstCnt - 2) * ptParam->uiClstSec;
												/* compute total sectors */
	if (ptParam->uiOption & GRP_FAT_ADJ_BY_START) /* adjust by start */
		uiStart += ptParam->uiNotUsed;			/* add start offset */
	uiAlign = (ptParam->uiAlign == 0)? ptParam->uiClstSec: ptParam->uiAlign;
												/* alignment */
	uiRemain = uiStart % uiAlign;				/* alignement remain */
	if (uiRemain != 0)							/* not aligned */
		uiSec += (uiAlign - uiRemain);			/* add alignment remain */
	return(uiSec);								/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_check_fat_type											*/
/*																			*/
/* DESCRIPTION:	Check FAT type by size										*/
/* INPUT:		uiTotalSec:		total sector count							*/
/*				iSecShift:		shift count of sector size					*/
/*				ptReq:			request format parameter					*/
/* OUTPUT:		ptRes:			result format parameter						*/
/*																			*/
/* RESULT:		0:				bad size									*/
/*				GRP_FAT_TYPE_12	FAT12										*/
/*				GRP_FAT_TYPE_16	FAT16										*/
/*				GRP_FAT_TYPE_32	FAT32										*/
/*																			*/
/****************************************************************************/
static int
_fat_check_fat_type(
	grp_uint32_t			uiTotalSec,			/* [IN]  total sector count */
	int						iSecShift,			/* [IN]  sector shift */
	grp_fat_format_param_t	*ptReq,				/* [IN]  format request param */
	grp_fat_format_param_t	*ptRes)				/* [OUT] format result param */
{
	grp_uint32_t			uiLimit;			/* limit sector count */
	grp_uint32_t			uiSec;				/* sector count */
	grp_uint32_t			uiDiv;				/* divider */
	grp_int32_t				iAdj;				/* adjust count */
	grp_fat_default_t		*ptFatCfg;			/* format configuration */

	/****************************************************/
	/* initialize result parameter						*/
	/****************************************************/
	ptRes->ucFatType = 0;						/* invalid type */
	ptRes->uiClst = 0;							/* cluster count */
	ptRes->uiFatSec = 0;						/* FAT sector count */
	ptRes->uiClstSec = ptReq->uiClstSec;		/* set sector/cluster */

	/****************************************************/
	/* check FAT12 first								*/
	/****************************************************/
	ptFatCfg = &grp_fat_default[GRP_FAT_TYPE_12 - 1]; /* default FAT12 param */
	FAT_GET_CNT_PARAM(ptRes, ptReq, ptFatCfg, iSecShift);
												/* get count parameter */
	uiLimit = FAT_12_NEED_SEC(ptRes, FAT_12_MIN_CLST, iSecShift);
												/* min FAT12 sectors */
	if (uiTotalSec <= uiLimit)					/* less than minimum */
		return(0);								/* return bad */
	uiLimit = FAT_12_NEED_SEC(ptRes, FAT_12_MAX_CLST, iSecShift);
												/* max FAT12 sectors */
	if (uiTotalSec <= uiLimit) {				/* within FAT12 size */
		/****************************************************/
		/* compute FAT area size first by considering		*/
		/* 	<FAT Sector Count> = 3 X + Y (0 <= Y < 3)		*/
		/****************************************************/
		grp_uint32_t	uiFatSec3;				/* FAT sector count / 3 */
		grp_uint32_t	uiFatSecRem3;			/* FAT sector count % 3 */
		grp_uint32_t	uiDiv3;					/* divisor for X */
		uiSec = uiTotalSec + ptRes->uiClstSec * 2
				- (FAT_DIR_SEC(ptRes->uiRootCnt, iSecShift)
					+ ptRes->uiRsvSecCnt);		/* area for FAT and data */
		uiDiv3 = (FAT_SEC_SZ(iSecShift) * 2) * ptRes->uiClstSec
							 + 3 * GRP_FAT_AREA_CNT;/* divisor for X */
		uiFatSec3 = uiSec / uiDiv3;				/* compute X value */
		uiDiv = uiDiv3 / 3;						/* divisor for Y */
		uiFatSecRem3 = FAT_RND_UP(uiSec - uiDiv3 * uiFatSec3, uiDiv);
												/* compute Y */
		if (uiFatSecRem3 > 3)					/* affected by round */
			uiFatSecRem3 = 3;					/* adjust it */
		ptRes->uiFatSec = uiFatSec3 * 3 + uiFatSecRem3;	/* FAT sector count */
		ptRes->uiClst = (uiSec - ptRes->uiFatSec * GRP_FAT_AREA_CNT)
					 / ptRes->uiClstSec;		/* cluster count */
		if (FAT_FAT12_SEC(ptRes->uiClst, iSecShift) < ptRes->uiFatSec) {
												/* not effective */
			ptRes->uiFatSec--;					/* decrement FAT count */
			ptRes->uiClst = ((FAT_SEC_SZ(iSecShift) * ptRes->uiFatSec) * 2) / 3;
												/* adjust cluster count */
		}
		ptRes->uiClst -= 2;						/* Adjust cluster count */
		return((int)(ptRes->ucFatType = GRP_FAT_TYPE_12));/* return FAT12 */
	}

	/****************************************************/
	/* check FAT16										*/
	/****************************************************/
	ptFatCfg = &grp_fat_default[GRP_FAT_TYPE_16 - 1]; /* default FAT16 param */
	FAT_GET_CNT_PARAM(ptRes, ptReq, ptFatCfg, iSecShift);
												/* get count parameter */
	uiLimit = FAT_16_NEED_SEC(ptRes, FAT_16_MIN_CLST, iSecShift);
												/* min FAT16 sectors */
	if (uiTotalSec < uiLimit					/* not reach FAT16 */
	    || ptReq->ucFatType == GRP_FAT_TYPE_12) { /* request FAT12 */
		ptFatCfg = &grp_fat_default[GRP_FAT_TYPE_12 - 1];
												/* default FAT12 param */
		FAT_GET_CNT_PARAM(ptRes, ptReq, ptFatCfg, iSecShift);
												/* get count param */
		ptRes->uiFatSec = FAT_FAT12_SEC(FAT_12_MAX_CLST, iSecShift);
												/* FAT sector count */
		ptRes->uiClst = FAT_12_MAX_CLST - 2;	/* cluster count */
		return((int)(ptRes->ucFatType = GRP_FAT_TYPE_12));/* return FAT12 */
	}
	uiLimit = FAT_16_NEED_SEC(ptRes, FAT_16_MAX_CLST, iSecShift);
												/* max FAT16 sectors */
	if (uiTotalSec <= uiLimit) {				/* within FAT16 size */
		uiSec = uiTotalSec + ptRes->uiClstSec * 2
				- (FAT_DIR_SEC(ptRes->uiRootCnt, iSecShift)
					+ ptRes->uiRsvSecCnt);		/* area for FAT and data */
		uiDiv = (FAT_SEC_SZ(iSecShift) / 2) * ptRes->uiClstSec
				+ GRP_FAT_AREA_CNT;				/* sector divisor */
		ptRes->uiFatSec = FAT_RND_UP(uiSec, uiDiv);	/* FAT sector count */
		ptRes->uiClst = (uiSec - ptRes->uiFatSec * GRP_FAT_AREA_CNT)
					 / ptRes->uiClstSec;		/* cluster count */
		if (FAT_FAT16_SEC(ptRes->uiClst, iSecShift) < ptRes->uiFatSec) {
												/* not effective */
			ptRes->uiFatSec--;					/* decrement FAT count */
			ptRes->uiClst = (FAT_SEC_SZ(iSecShift) * ptRes->uiFatSec) / 2;
												/* adjust cluster count */
		}
		ptRes->uiClst -= 2;						/* Adjust cluster count */
		return((int)(ptRes->ucFatType = GRP_FAT_TYPE_16));/* return FAT16 */
	}

	/****************************************************/
	/* check FAT32										*/
	/****************************************************/
	ptFatCfg = &grp_fat_default[GRP_FAT_TYPE_32 - 1]; /* default FAT32 param */
	FAT_GET_CNT_PARAM(ptRes, ptReq, ptFatCfg, iSecShift);
												/* get count parameter */
	uiLimit = FAT_32_NEED_SEC(ptRes, FAT_32_MIN_CLST, iSecShift);
												/* min FAT32 sectors */
	if (uiTotalSec < uiLimit					/* not reach FAT32 */
	    || ptReq->ucFatType == GRP_FAT_TYPE_16) { /* request FAT16 */
		ptFatCfg = &grp_fat_default[GRP_FAT_TYPE_16 - 1];
												/* default FAT16 param */
		FAT_GET_CNT_PARAM(ptRes, ptReq, ptFatCfg, iSecShift);
												/* get count param */
		ptRes->uiFatSec = FAT_FAT16_SEC(FAT_16_MAX_CLST, iSecShift);
												/* FAT sector count */
		ptRes->uiClst = FAT_16_MAX_CLST - 2;	/* cluster count */
		return((int)(ptRes->ucFatType = GRP_FAT_TYPE_16));/* return FAT16 */
	}
	iAdj = (grp_int32_t)ptRes->uiClstSec * 2 - (grp_int32_t)ptRes->uiRsvSecCnt;
	uiSec = uiTotalSec + iAdj;					/* area for FAT and data */
	uiDiv = (FAT_SEC_SZ(iSecShift) / 4) * ptRes->uiClstSec + GRP_FAT_AREA_CNT;
	if (uiSec < uiTotalSec && iAdj > 0) {		/* over flow */
		ptRes->uiFatSec = uiTotalSec / uiDiv;	/* FAT sector count */
		ptRes->uiFatSec += FAT_RND_UP((uiTotalSec % uiDiv) + iAdj, uiDiv);
												/* adjust with adjust count */
	} else {									/* not over flow */
		ptRes->uiFatSec = uiSec / uiDiv;		/* FAT sector count */
		if (uiSec % uiDiv != 0) {				/* not multiple of uiDiv */
			ptRes->uiFatSec++;					/* add one */
		}
	}
	ptRes->uiClst = (uiSec - ptRes->uiFatSec * GRP_FAT_AREA_CNT)
					/ ptRes->uiClstSec;			/* cluster count */
	if (FAT_FAT32_SEC(ptRes->uiClst, iSecShift) < ptRes->uiFatSec) {
												/* not effective */
		ptRes->uiFatSec--;						/* decrement FAT count */
		ptRes->uiClst = (FAT_SEC_SZ(iSecShift) * ptRes->uiFatSec) / 4;
												/* adjust cluster count */
	}
	if (ptRes->uiClst > FAT_32_MAX_CLST) {		/* over max FAT32 */
		ptRes->uiFatSec = FAT_FAT32_SEC(FAT_32_MAX_CLST, iSecShift);
												/* FAT sector count */
		ptRes->uiClst = FAT_32_MAX_CLST;		/* cluster count */
	}
	ptRes->uiClst -= 2;							/* Adjust cluster count */
	if (ptRes->uiRsvSecCnt < (GRP_FAT_BACKUP_BPB + GRP_FAT_BOOT_SEC_CNT)) {
												/* invalid rsv sector count */
		return(0);								/* return invalid */
	}
	return((int)(ptRes->ucFatType = GRP_FAT_TYPE_32));/* return FAT32 */
}

/****************************************************************************/
/* FUNCTION:	_fat_adjust_boudary											*/
/*																			*/
/* DESCRIPTION:	Adjust cluster area boundary								*/
/* INPUT:		ptParam:		format parameter							*/
/*				uiTotalSec:		total sector count							*/
/*				uiOffset:		start offset								*/
/*				iSecShift:		sector shift								*/
/* OUTPUT:		ptParam:		adjusted format parameter					*/
/*																			*/
/* RESULT:		0:				success										*/
/*				GRP_FS_ERR_BAD_PARAM: not enough cluster					*/
/*																			*/
/****************************************************************************/
static int
_fat_adjust_boundary(
	grp_fat_format_param_t	*ptParam,			/* [IN]  format parameter */
	grp_uint32_t			uiTotalSec,			/* [IN]  total sector */
	grp_uint32_t			uiOffset,			/* [IN]  start offset */
	int						iSecShift)			/* [IN]  sector shift */
{
	grp_uint32_t			uiClstOff;			/* cluster area offset */
	grp_uint32_t			uiClstSec;			/* sector/cluster */
	grp_uint32_t			uiRemain;			/* remain sector */
	grp_uint32_t			uiAdjust;			/* adjust count */
	grp_uint32_t			uiAlign;			/* alignment */
	grp_uint32_t			uiClstCnt;			/* cluster count */

	uiClstSec = ptParam->uiClstSec;				/* sector/cluster */
	uiClstOff = ptParam->uiRsvSecCnt + ptParam->uiFatSec * GRP_FAT_AREA_CNT;
												/* reserved area + FAT area */
	switch(ptParam->ucFatType) {				/* FAT type */
	case GRP_FAT_TYPE_12:						/* FAT12 */
	case GRP_FAT_TYPE_16:						/* FAT16 */
		uiClstOff += FAT_DIR_SEC(ptParam->uiRootCnt, iSecShift);
												/* add root directory area */
		break;
	}
	ptParam->uiNotUsed = uiTotalSec 
						- (uiClstOff + ptParam->uiClst * uiClstSec);
												/* not used sector */
	uiAlign = ptParam->uiAlign;					/* alignment */
	if (uiAlign == 0)							/* default alignment */
		uiAlign = uiClstSec;					/* use sector/cluster */
	if (ptParam->uiOption & GRP_FAT_ADJ_BY_START) /* adjust by start */
		uiClstOff += uiOffset;					/* add start offset */
	uiRemain = (uiClstOff % uiAlign);			/* miss-align count */
	if (uiRemain) {								/* miss-aligned */
		uiAdjust = uiAlign - uiRemain;			/* adjust count */
		ptParam->uiAdjust += uiAdjust;			/* set adjust count */
		if (ptParam->uiNotUsed < uiAdjust) {	/* adjust is bigger */
			uiClstCnt = FAT_RND_UP(uiAdjust - ptParam->uiNotUsed, uiClstSec);
			if (ptParam->uiClst < uiClstCnt)	/* not enough cluster */
				return(GRP_FS_ERR_BAD_PARAM);	/* return error */
			ptParam->uiClst -= uiClstCnt;		/* adjust clsuter count */
			ptParam->uiNotUsed += uiClstCnt * uiClstSec; /* adjust not used */
		}
		if ((ptParam->uiOption & GRP_FAT_ADJ_BY_START) == 0) {
												/* adjust rsv sectors */
			ptParam->uiNotUsed -= uiAdjust;		/* adjust not used */
			ptParam->uiRsvSecCnt += uiAdjust;	/* adjust rsv sector count */
		}
	}
	return(0);
}

/****************************************************************************/
/* FUNCTION:	_fat_init_buf												*/
/*																			*/
/* DESCRIPTION:	initialize I/O buffer										*/
/* INPUT:		iHandle:		device I/O handle							*/
/*				iDev:			device number								*/
/*				iLogToDevShift:	logical to device sector shift				*/
/*				pucBuf:			I/O buffer									*/
/*				iBufSize:		I/O buffer size								*/
/*				ptMedia:		media information							*/
/* OUTPUT:		ptBio:			buffer I/O control data						*/
/*																			*/
/* RESULT:		0:				  success									*/
/*				GRP_FS_ERR_BAD_PARAM: invalid buffer size					*/
/*				GRP_FS_ERR_NOMEM: not enough buffer size					*/
/*																			*/
/****************************************************************************/
static int
_fat_init_buf(
	grp_int32_t				iHandle,			/* [IN]  I/O handle */
	int						iDev,				/* [IN]  device number */
	int						iLogToDevShift,		/* [IN]  logical to dev shift */
	grp_uchar_t				*pucBuf,			/* [IN]  I/O buffer */
	grp_int32_t				iBufSize,			/* [IN]  I/O buffer size */
	grp_fs_media_info_t		*ptMedia,			/* [IN]  media information */
	fat_io_buf_t			*ptBio)				/* [OUT] buffer I/O control */
{
	int						iMajor;				/* major device number */
	int						iSecShift;			/* sector shift count */

	iSecShift = ptMedia->iSecShift;				/* sector shift count */
	if (iBufSize & (iBufSize - 1)) 				/* not power of 2 */
		return(GRP_FS_ERR_BAD_PARAM);			/* return error */
	if (iBufSize < ((grp_int32_t)1 << iSecShift))/* less than sector size */
		return(GRP_FS_ERR_NOMEM);				/* return error */
	ptBio->iBufSize = iBufSize;					/* set buffer size */
	ptBio->uiSecCnt = (grp_uint32_t)(iBufSize >> iSecShift);
												/* buffer sector count */
	ptBio->pucBuf = pucBuf;						/* set buffer */
	ptBio->iDev = iDev;							/* set device number */
	ptBio->iHandle = iHandle;					/* set I/O handle */
	ptBio->iLogToDevShift = iLogToDevShift;		/* set logical to dev shift */
	iMajor = GRP_FS_DEV_MAJOR(iDev);			/* major device number */
	ptBio->ptDevOp = grp_fs_dev_tbl[iMajor].ptOp;/* device operation */
	memset(ptBio->pucBuf, 0, (grp_size_t)iBufSize); /* clear buffer */
	ptBio->uiSec = 0;							/* sector 0 */
	ptBio->ptMedia = ptMedia;					/* set media information */
	return(0);									/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_buf												*/
/*																			*/
/* DESCRIPTION:	set I/O buffer												*/
/* INPUT:		ptBio:			buffer I/O control data						*/
/*				uiSec:			sector number to set						*/
/*				iFillZero:		fill 0 to the specified sector				*/
/* OUTPUT:		ptBio:			buffer I/O control data						*/
/*				ppucData:		buffer pointer								*/
/*																			*/
/* RESULT:		0:				success										*/
/*				GRP_FS_ERR_IO:	I/O error								    */
/*																			*/
/****************************************************************************/
static int
_fat_set_buf(
	fat_io_buf_t			*ptBio,				/* [IN/OUT] buf I/O control */
	grp_uint32_t			uiSec,				/* [IN]  sector number */
	int						iFillZero,			/* [IN]  fill 0 to the sector */
	grp_uchar_t				**ppucData)			/* [OUT] buffer pointer */
{
	grp_fs_media_info_t		*ptMedia;			/* media information */
	grp_int32_t				iRet;				/* return value */
	int						iSecShift;			/* sector shift */
	int						iLogToDevShift;		/* logical to device shift */
	grp_uint32_t			uiSecCnt;			/* sector count */
	grp_uint32_t			uiDevSecCnt;		/* device sector count */
	grp_uint32_t			uiStart;			/* start sector number */

	ptMedia = ptBio->ptMedia;					/* media information */
	iSecShift = ptMedia->iSecShift;				/* sector shift count */
	uiSecCnt = ptBio->uiSecCnt;					/* sector count */
	uiStart = ptBio->uiSec;						/* start sector */
	if (uiStart <= uiSec && uiSec < uiStart + uiSecCnt) /* in current buffer */
		goto out;								/* goto out */
	iLogToDevShift = ptBio->iLogToDevShift;		/* logical to device shift */
	uiDevSecCnt = (uiSecCnt << iLogToDevShift);	/* device sector count */
	iRet = ptBio->ptDevOp->pfnWrite(ptBio->iHandle, ptBio->iDev,
				((ptBio->uiSec << iLogToDevShift) + ptMedia->uiStartSec),
				ptBio->pucBuf, (grp_isize_t)uiDevSecCnt); /* write data */
	if (iRet != (grp_int32_t)uiDevSecCnt)		/* write failed */
		return(GRP_FS_ERR_IO);					/* return error */
	memset(ptBio->pucBuf, 0, ptBio->iBufSize);	/* clear buffer */
	uiStart = (uiSec / uiSecCnt) * uiSecCnt;	/* start sector number */
	if (iFillZero) {							/* fill zero to the sector */
		for (ptBio->uiSec += uiSecCnt; ptBio->uiSec < uiStart;
			 ptBio->uiSec += uiSecCnt) {		/* loop until target sector */
			iRet = ptBio->ptDevOp->pfnWrite(ptBio->iHandle, ptBio->iDev,
					((ptBio->uiSec << iLogToDevShift) + ptMedia->uiStartSec),
					ptBio->pucBuf, (grp_isize_t)uiDevSecCnt);	/* fill 0 */
			if (iRet != (grp_int32_t)uiDevSecCnt)	/* write failed */
				return(GRP_FS_ERR_IO);			/* return error */
		}
	}
	ptBio->uiSec = uiStart;						/* set start sector */
out:
	*ppucData = ptBio->pucBuf + ((uiSec - uiStart) << iSecShift);
	return(0);									/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_flush_buf												*/
/*																			*/
/* DESCRIPTION:	flush I/O buffer											*/
/* INPUT:		ptBio:			buffer I/O control data						*/
/*				pucEnd			end of data									*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:				success										*/
/*				GRP_FS_ERR_IO:	I/O error								    */
/*																			*/
/****************************************************************************/
static int
_fat_flush_buf(
	fat_io_buf_t			*ptBio,				/* [IN] buf I/O control */
	grp_uchar_t				*pucEnd)			/* [IN] end of data */
{
	grp_fs_media_info_t		*ptMedia;			/* media information */
	grp_int32_t				iRet;				/* return value */
	int						iSecShift;			/* sector shift */
	int						iLogToDevShift;		/* logical to device shift */
	grp_uint32_t			uiSecCnt;			/* sector count */
	grp_uint32_t			uiDevSecCnt;		/* device sector count */
	grp_uint32_t			uiStart;			/* start sector number */
	grp_int32_t				iSize;				/* size */

	iSize = pucEnd - ptBio->pucBuf;				/* size of data */
	if (iSize <= 0)								/* no data */
			return(0);							/* return success */
	ptMedia = ptBio->ptMedia;					/* media information */
	iSecShift = ptMedia->iSecShift;				/* sector shift count */
	uiSecCnt = ptBio->uiSecCnt;					/* sector count */
	uiStart = ptBio->uiSec;						/* start sector */
	if (uiStart + uiSecCnt > ptMedia->uiTotalSec) { /* over end */
		uiSecCnt = (grp_uint32_t)FAT_B2SEC(iSize, iSecShift);
												/* compute sector count */
	}
	iLogToDevShift = ptBio->iLogToDevShift;		/* logical to device shift */
	uiDevSecCnt = (uiSecCnt << iLogToDevShift);	/* device sector count */
	iRet = ptBio->ptDevOp->pfnWrite(ptBio->iHandle, ptBio->iDev,
					((ptBio->uiSec << iLogToDevShift) + ptMedia->uiStartSec),
					ptBio->pucBuf, (grp_isize_t)uiDevSecCnt);/* write buffer */
	return((iRet == (grp_int32_t)uiDevSecCnt)? 0: GRP_FS_ERR_IO);/* return */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_BPB												*/
/*																			*/
/* DESCRIPTION:	set BPB information											*/
/* INPUT:		ptParam:		format parameter							*/
/*				ptMedia:		media information							*/
/* OUTPUT:		pucBPB:			BPB data									*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_set_BPB(
	grp_uchar_t				*pucBPB,			/* [OUT] BPB data */
	grp_fat_format_param_t	*ptParam,			/* [IN]  format parameter */
	grp_fs_media_info_t		*ptMedia)			/* [IN]  media information */
{
	fat_dk_BPB_head_t		*ptBPBhp;			/* BPB head */
	fat_dk_BPB_32_t			*ptBPB32p;			/* FAT32 specific BPB data */
	fat_dk_BPB_tail_t		*ptBPBtp;			/* BPB tail */
	int						iFatType;			/* FAT type */
	grp_uint32_t			uiTotalSec;			/* total sector count */
	grp_uchar_t				*pucBPBSig;			/* BPB signature */
	const char				*pcFatTypeName;		/* FAT type name */

	/****************************************************/
	/* set head part of BPB								*/
	/****************************************************/
	iFatType = (int)ptParam->ucFatType;			/* FAT type */
	ptBPBhp = (fat_dk_BPB_head_t *)pucBPB;		/* BPB head */
	ptBPBhp->aucJumpInst[0] = FAT_JMP_OPCODE;	/* jump operation code */
	ptBPBhp->aucJumpInst[1] = (grp_uchar_t)
							((ptParam->ucFatType == GRP_FAT_TYPE_32)?
							FAT_JMP_32_DISP:	/* FAT32 jump displacement */
							FAT_JMP_12_16_DISP);/* FAT12/16 jump displacement */
	ptBPBhp->aucJumpInst[2] = FAT_NOP_OPCODE;	/* nop opcode */
	memcpy((char *)ptBPBhp->aucOEMName, (char *)grp_fat_format_cfg.aucOEMName,
			sizeof(ptBPBhp->aucOEMName));		/* set OEM name */
	FAT_SET_INT16(ptBPBhp->aucSecByte, (1 << ptMedia->iSecShift));
												/* set sector size */
	ptBPBhp->ucClstSec = (grp_uchar_t)ptParam->uiClstSec;  /* sector/cluster */
	FAT_SET_INT16(ptBPBhp->aucRsvSec, ptParam->uiRsvSecCnt);/* rsv sectors */
	ptBPBhp->ucFatCnt = GRP_FAT_AREA_CNT;		/* FAT area count */
	if (iFatType != GRP_FAT_TYPE_32) {			/* FAT12/16 */
		FAT_SET_INT16(ptBPBhp->aucRootCnt, ptParam->uiRootCnt);/* root count */
		FAT_SET_INT16(ptBPBhp->aucFatSec16, ptParam->uiFatSec);/* FAT sectors */
	}
	uiTotalSec = ptMedia->uiTotalSec - ptParam->uiNotUsed;
												/* total sector count */
	if (uiTotalSec < 0x10000) {					/* less than 16 bits */
		FAT_SET_INT16(ptBPBhp->aucTotalSec16, uiTotalSec);
	} else {									/* over 16 bits */
		FAT_SET_INT32(ptBPBhp->aucTotalSec32, uiTotalSec);
	}
	ptBPBhp->ucMedia = ptMedia->ucMediaType;	/* set media type */
	FAT_SET_INT16(ptBPBhp->aucTrkSec, ptMedia->usTrkSec);	/* sector/track */
	FAT_SET_INT16(ptBPBhp->aucHead, ptMedia->usHead);		/* heads */
	FAT_SET_INT32(ptBPBhp->aucHiddenSec, ptMedia->uiStartSec);/* hidden sec */

	/****************************************************/
	/* set FAT32 specific part of BPB					*/
	/****************************************************/
	if (iFatType == GRP_FAT_TYPE_32) {			/* FAT32 */
		ptBPB32p = (fat_dk_BPB_32_t *) &ptBPBhp[1];/* FAT32 specific part */
		FAT_SET_INT32(ptBPB32p->aucFatSec32, ptParam->uiFatSec);
												/* set FAT sector count */
		FAT_SET_INT32(ptBPB32p->aucRootClst, GRP_FAT_ROOT_CLST);
												/* root cluster */
		FAT_SET_INT16(ptBPB32p->aucFsInfo, GRP_FAT_FSINFO_SEC);
												/* FSINFO sector */
		FAT_SET_INT16(ptBPB32p->aucBackupBootSec, GRP_FAT_BACKUP_BPB);
												/* backup BPB sector */
		ptBPBtp = (fat_dk_BPB_tail_t *) &ptBPB32p[1];/* tail part of BPB */
	} else
		ptBPBtp = (fat_dk_BPB_tail_t *) &ptBPBhp[1]; /* tail part of BPB */

	/****************************************************/
	/* set tail part of BPB								*/
	/****************************************************/
	ptBPBtp->ucDrvNum = (grp_uchar_t)(((ptMedia->iSecShift == FAT_SEC_SHIFT)
			&& (ptMedia->uiTotalSec == FAT_2HD_SECS
				|| ptMedia->uiTotalSec == FAT_2DD_SECS))?
					 FAT_DRV_FD: FAT_DRV_HD);	/* set drive number */
	ptBPBtp->ucBootSig = FAT_BOOT_SIG;			/* boot signature */
	memcpy((char *)ptBPBtp->aucVolLab, (char *)ptParam->aucVolLab,
			sizeof(ptBPBtp->aucVolLab));		/* set volume label */
	memcpy((char *)ptBPBtp->aucVolSer, (char *)ptParam->aucVolSer,
			sizeof(ptBPBtp->aucVolSer));		/* set volume serial number */
	pcFatTypeName = (iFatType == GRP_FAT_TYPE_12)? FAT_TYPE_NAME_12:
					(iFatType == GRP_FAT_TYPE_16)? FAT_TYPE_NAME_16:
					FAT_TYPE_NAME_32;			/* FAT type name */
	memcpy((char *)ptBPBtp->aucFsTypeName, pcFatTypeName,
			sizeof(ptBPBtp->aucFsTypeName));	/* set FAT type name */
	pucBPBSig = &pucBPB[FAT_BPB_SIG_OFF];		/* BPB signature */
	*pucBPBSig++ = FAT_BPB_SIG1;				/* BPB signature 1 */
	*pucBPBSig = FAT_BPB_SIG2;					/* BPB signature 2 */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_root_cluster_count									*/
/*																			*/
/* DESCRIPTION:	get root cluster count										*/
/* INPUT:		ptParam:		format parameter							*/
/*				iSecShift:		sector shift count							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		root cluster count											*/
/*																			*/
/****************************************************************************/
static grp_uint32_t
_fat_get_root_cluster_count(
	grp_fat_format_param_t	*ptParam,			/* [IN]  format parameter */
	int						iSecShift)			/* [IN]  sector shift count */
{
	grp_uint32_t			uiRootClstCnt;		/* root cluster count */
	int						iClstShift = 0;		/* cluser sihft */

	/****************************************************/
	/* computer cluster shift count						*/
	/****************************************************/
	while (ptParam->uiClstSec != ((grp_uint32_t)1 << iClstShift))
			iClstShift++;						/* increment count */
	iClstShift += iSecShift;					/* add sector shift */

	/****************************************************/
	/* computer root cluster count						*/
	/****************************************************/
	uiRootClstCnt = FAT_DIR_SEC(ptParam->uiRootCnt, iClstShift);
	return(uiRootClstCnt);						/* return root cluster count */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_fsinfo												*/
/*																			*/
/* DESCRIPTION:	set FSINFO for FAT32										*/
/* INPUT:		ptParam:		format parameter							*/
/*				iSecShift:		sector shift count							*/
/* OUTPUT:		pucFsInfo:		FSINFO data									*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_set_fsinfo(
	grp_uchar_t				*pucFsInfo,			/* [OUT] FSINFO data */
	grp_fat_format_param_t	*ptParam,			/* [IN]  format parameter */
	int						iSecShift)			/* [IN]  sector shift count */
{
	fat_dk_fsinfo_t			*ptFsInfo;			/* FSINFO structure */
	grp_uint32_t			uiRootClstCnt;		/* root cluster count */

	/****************************************************/
	/* computer root cluster count						*/
	/****************************************************/
	uiRootClstCnt = _fat_get_root_cluster_count(ptParam, iSecShift);

	/****************************************************/
	/* set FSINFO										*/
	/****************************************************/
	ptFsInfo = (fat_dk_fsinfo_t *)pucFsInfo;	/* FSINFO structure */
	FAT_SET_INT32(ptFsInfo->aucFiFreeCnt,
			ptParam->uiClst - uiRootClstCnt);	/* set free count */
	FAT_SET_INT32(ptFsInfo->aucFiNextFree,
							uiRootClstCnt + 2);	/* next free cluster */
	FAT_SET_INT32(ptFsInfo->aucFiSig1, FAT_FI_SIG1); /* set signature 1 */
	FAT_SET_INT32(ptFsInfo->aucFiSig2, FAT_FI_SIG2); /* set signature 2 */
	FAT_SET_INT32(ptFsInfo->aucFiSig3, FAT_FI_SIG3); /* set signature 3 */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_fat												*/
/*																			*/
/* DESCRIPTION:	set FAT	data												*/
/* INPUT:		ptBio:			buffer I/O control data						*/
/*				uiSec;			current sector number						*/
/*				ptParam:		format parameter							*/
/*				ptMedia:		media information							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:				success										*/
/*				GRP_FS_ERR_IO:	I/O error								    */
/*																			*/
/****************************************************************************/
static int
_fat_set_fat(
	fat_io_buf_t			*ptBio,				/* [IN]  buf I/O control */
	grp_uint32_t			uiSec,				/* [IN]  current sector */
	grp_fat_format_param_t	*ptParam,			/* [IN]  format parameter */
	grp_fs_media_info_t		*ptMedia)			/* [IN]  media information */
{
	grp_uchar_t				*pucFat;			/* FAT area pointer */
	grp_uint32_t			uiClst;				/* cluster number */
	grp_uint32_t			uiEndClst;			/* end cluster in a sector */
	grp_uint32_t			uiRootClstCnt;		/* root cluster count */
	grp_uint32_t			uiFatPerSec;		/* FAT per sector */
	int						iSecShift;			/* sector shift count */
	int						iRet;				/* return value */

	/****************************************************/
	/* move to FAT area									*/
	/****************************************************/
	iRet = _fat_set_buf(ptBio, uiSec, 1, &pucFat);/* move to FAT area */
	if (iRet < 0)								/* I/O error */
		return(iRet);							/* return error */

	/****************************************************/
	/* set FAT0 and FAT1								*/
	/****************************************************/
	*pucFat++ = ptMedia->ucMediaType;			/* set media type for FAT0 */
	switch(ptParam->ucFatType) {				/* FAT type */
	case GRP_FAT_TYPE_12:						/* FAT12 */
		*pucFat++ = 0xff;						/* high FAT0/low EOC for FAT1 */
		*pucFat++ = 0xff;						/* EOC for FAT1 */
		return(0);								/* return success */
	case GRP_FAT_TYPE_16:						/* FAT16 */
		*pucFat++ = 0xff;						/* high byte of FAT0 */
		*pucFat++ = 0xff;						/* low byte of EOC for FAT1 */
		*pucFat++ = 0xff;						/* high byte of EOC for FAT1 */
		return(0);								/* return success */
	default:									/* FAT32 */
		*pucFat++ = 0xff;						/* 2nd byte of FAT0 */
		*pucFat++ = 0xff;						/* 3rd byte of FAT0 */
		*pucFat++ = 0x0f;						/* 4th byte of FAT0 */
		*pucFat++ = 0xff;						/* 1st byte of FAT1 */
		*pucFat++ = 0xff;						/* 2nd byte of FAT1 */
		*pucFat++ = 0xff;						/* 3rd byte of FAT1 */
		*pucFat++ = 0x0f;						/* 4th byte of FAT1 */
		break;
	}

	/****************************************************/
	/* set FAT for root directory						*/
	/****************************************************/
	iSecShift = ptMedia->iSecShift;				/* sector shift */
	uiRootClstCnt = _fat_get_root_cluster_count(ptParam, iSecShift);
												/* root cluster count */
	uiFatPerSec = FAT_SEC_SZ(iSecShift)/4;		/* FAT per sctor */
	uiClst = 2;									/* current cluster number */
	uiEndClst = uiFatPerSec;					/* end cluster in a sector */
	while (--uiRootClstCnt > 0) {				/* loop by root cluster */
		FAT_SET_INT32(pucFat, ++uiClst);		/* set FAT */
		pucFat += 4;							/* advance pointer */
		if (uiClst >= uiEndClst) {				/* end of sector */
			iRet = _fat_set_buf(ptBio, ++uiSec, 1, &pucFat);
												/* move to next sector */
			if (iRet < 0)						/* I/O error */
				return(iRet);					/* return error */
			uiEndClst += uiFatPerSec;			/* set end cluster */
		}
	}
	*pucFat++ = 0xff;							/* EOF cluster */
	*pucFat++ = 0xff;							/* EOF cluster */
	*pucFat++ = 0xff;							/* EOF cluster */
	*pucFat++ = 0x0f;							/* EOF cluster */
	return(0);									/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_root												*/
/*																			*/
/* DESCRIPTION:	set root directory											*/
/* INPUT:		ptBio:			buffer I/O control data						*/
/*				uiSec;			current sector number						*/
/*				ptParam:		format parameter							*/
/*				ptMedia:		media information							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:				success										*/
/*				GRP_FS_ERR_IO:	I/O error								    */
/*																			*/
/****************************************************************************/
static int
_fat_set_root(
	fat_io_buf_t			*ptBio,				/* [IN]  buf I/O control */
	grp_uint32_t			uiSec,				/* [IN]  current sector */
	grp_fat_format_param_t	*ptParam,			/* [IN]  format parameter */
	grp_fs_media_info_t		*ptMedia)			/* [IN]  media information */
{
	grp_uchar_t				*pucDir;			/* directory area pointer */
	fat_dir_t				*ptDir;				/* directory pointer */
	grp_int32_t				iCurTime;			/* current time */
	grp_time_tm_t			tTM;				/* time information */
	grp_ushort_t			usFatDate;			/* FAT date information */
	grp_ushort_t			usFatTime;			/* FAT time information */
	int						iRet;				/* return value */
	int						iSecShift;			/* sector shift */
	grp_uint32_t			uiRootClstCnt;		/* root cluster count */
	grp_uint32_t			uiSecCnt;			/* sector count */

	/****************************************************/
	/* move to root directory area						*/
	/****************************************************/
	iRet = _fat_set_buf(ptBio, uiSec, 1, &pucDir);/* move to root area */
	if (iRet < 0)								/* I/O error */
		return(iRet);							/* return error */

	/****************************************************/
	/* make volume name entry							*/
	/****************************************************/
	if (memcmp((char *)ptParam->aucVolLab, FAT_LAB_NONAME,
			sizeof(ptParam->aucVolLab)) != 0) {	/* exist volume name */
		ptDir = (fat_dir_t *)pucDir;			/* set directory pointer */
		memcpy((char *)ptDir->aucName, (char *)ptParam->aucVolLab,
				sizeof(ptDir->aucName));		/* copy volume name */
		ptDir->ucAttr = FAT_ATTR_VOLID;			/* volume id */
		grp_fs_get_current_time(&iCurTime);		/* current time */
		grp_time_localtime(iCurTime, &tTM);		/* convert information */
		usFatDate = (grp_ushort_t)((((tTM.sYear - 1980) << 9) & 0xfe00)/* year */
				| ((tTM.ucMon << 5) & 0x01e0)	/* month */
				| (tTM.ucDay & 0x1f));			/* day */
		ptDir->aucMDate[0] = (grp_uchar_t)(usFatDate & 0xff);/* low date byte */
		ptDir->aucMDate[1] = (grp_uchar_t)((usFatDate >> 8) & 0xff);
												/* high date byte */
		usFatTime = (grp_ushort_t)(((tTM.ucHour << 11) & 0xf800)/* hour */
				| ((tTM.ucMin << 5) & 0x07e0)	/* minute */
				| ((tTM.ucSec >> 1) & 0x001f));	/* sec / 2 */
		ptDir->aucMTime[0] = (grp_uchar_t)(usFatTime & 0xff);
		ptDir->aucMTime[1] = (grp_uchar_t)((usFatTime >> 8) & 0xff);
		if ((ptParam->uiOption & GRP_FAT_NO_CRT_ACC_TIME) == 0) {
			ptDir->aucCDate[0] = ptDir->aucMDate[0];
			ptDir->aucCDate[1] = ptDir->aucMDate[1];
			ptDir->aucCTime[0] = ptDir->aucMTime[0];
			ptDir->aucCTime[1] = ptDir->aucMTime[1];
			ptDir->ucCTime10ms = (grp_uchar_t)((usFatTime & 1) * 100);
			ptDir->aucADate[0] = ptDir->aucMDate[0];
			ptDir->aucADate[1] = ptDir->aucMDate[1];
		}
	}

	/****************************************************/
	/* clear until end of directory						*/
	/****************************************************/
	iSecShift = ptMedia->iSecShift;				/* sector shift */
	if (ptParam->ucFatType == GRP_FAT_TYPE_32) {/* FAT32 */
		uiRootClstCnt = _fat_get_root_cluster_count(ptParam, iSecShift);
												/* root cluster count */
		uiSecCnt = uiRootClstCnt * ptParam->uiClstSec; /* sector count */
	} else {									/* FAT12/16 */
		uiSecCnt = FAT_DIR_SEC(ptParam->uiRootCnt, iSecShift);
												/* get sector count for root */
	}
	while (--uiSecCnt > 0) {
		iRet = _fat_set_buf(ptBio, ++uiSec, 1, &pucDir);/* move to next sec */
		if (iRet < 0)							/* I/O error */
			return(iRet);						/* return error */
	}

	/****************************************************/
	/* flush end remaining data in buffer				*/
	/****************************************************/
	iRet = _fat_flush_buf(ptBio, pucDir + ((grp_int32_t)1 << iSecShift));
	return(iRet);								/* return */
}

/****************************************************************************/
/* FUNCTION:	_fat_format_media											*/
/*																			*/
/* DESCRIPTION:	Format media												*/
/* INPUT:		iHandle:		device I/O handle							*/
/*				iDev:			device number								*/
/*				iLogToDevShift:	logical to device sector shift				*/
/*				pucBuf:			I/O buffer									*/
/*				iBufSize:		I/O buffer size								*/
/*				ptParam:		format parameter							*/
/*				ptMedia:		media information							*/
/* OUTPUT:		none														*/
/*																			*/
/* RESULT:		0:				success										*/
/*				GRP_FS_ERR_IO	I/O error									*/
/*				GRP_FS_ERR_BAD_PARAM invalid parameter						*/
/*				GRP_FS_ERR_NOMEM not enough I/O buffer						*/
/*																			*/
/****************************************************************************/
static int
_fat_format_media(
	grp_int32_t				iHandle,			/* [IN]  I/O handle */
	int						iDev,				/* [IN]  device number */
	int						iLogToDevShift,		/* [IN]  logical to dev shift */
	grp_uchar_t				*pucBuf,			/* [IN]  I/O buffer */
	grp_int32_t				iBufSize,			/* [IN]  I/O buffer size */
	grp_fat_format_param_t	*ptParam,			/* [IN]  format parameter */
	grp_fs_media_info_t		*ptMedia)			/* [IN]  media information */
{
	int						iRet;				/* return value */
	int						iFatType;			/* FAT type */
	int						i;					/* loop index */
	grp_uchar_t				*pucPtr;			/* data pointer */
	grp_uint32_t			uiSec;				/* sector */
	fat_io_buf_t			tBio;				/* buffer I/O control data */

	/****************************************************/
	/* initialize I/O buffer							*/
	/****************************************************/
	iRet = _fat_init_buf(iHandle, iDev, iLogToDevShift, pucBuf, iBufSize,
						ptMedia, &tBio);		/* init I/O buffer */
	if (iRet < 0)								/* buf init error */
		return(iRet);							/* return error */

	/****************************************************/
	/* set BPB data										*/
	/****************************************************/
	_fat_set_BPB(tBio.pucBuf, ptParam, ptMedia);/* set BPB information */

	/****************************************************/
	/* set FSINFO and backup BPB for FAT32				*/
	/****************************************************/
	iFatType = (int)ptParam->ucFatType;			/* FAT type */
	if (iFatType == GRP_FAT_TYPE_32) {
		iRet = _fat_set_buf(&tBio, GRP_FAT_FSINFO_SEC, 1, &pucPtr);
												/* move to FSINFO area */
		if (iRet < 0)							/* I/O error */
			return(iRet);						/* return error */
		_fat_set_fsinfo(pucPtr, ptParam, ptMedia->iSecShift);
												/* set FSINFO */
		iRet = _fat_set_buf(&tBio, GRP_FAT_FSINFO_SEC+1, 1, &pucPtr);
												/* move to reserved boot area */
		if (iRet < 0)							/* I/O error */
			return(iRet);						/* return error */
		FAT_SET_INT16(&pucPtr[FAT_BPB_SIG_OFF], FAT_BPB_SIG);
												/* set rsv boot signature */
		iRet = _fat_set_buf(&tBio, GRP_FAT_BACKUP_BPB, 1, &pucPtr);
												/* move to backup BPB area */
		if (iRet < 0)							/* I/O error */
			return(iRet);						/* return error */
		_fat_set_BPB(pucPtr, ptParam, ptMedia);	/* set BPB information */
		iRet = _fat_set_buf(&tBio, GRP_FAT_BACKUP_BPB+1, 1, &pucPtr);
												/* move to copy of FSINFO */
		if (iRet < 0)							/* I/O error */
			return(iRet);						/* return error */
		_fat_set_fsinfo(pucPtr, ptParam, ptMedia->iSecShift);
												/* set FSINFO */
		iRet = _fat_set_buf(&tBio, GRP_FAT_BACKUP_BPB+2, 1, &pucPtr);
												/* move to copy of rsv boot */
		if (iRet < 0)							/* I/O error */
			return(iRet);						/* return error */
		FAT_SET_INT16(&pucPtr[FAT_BPB_SIG_OFF], FAT_BPB_SIG);
												/* set rsv boot signature */
	}

	/****************************************************/
	/* set FAT area										*/
	/****************************************************/
	uiSec = ptParam->uiRsvSecCnt;				/* FAT start sector */
	for (i = 0; i < GRP_FAT_AREA_CNT; i++) {
		iRet = _fat_set_fat(&tBio, uiSec, ptParam, ptMedia); /* set FAT info */
		if (iRet < 0)							/* failed to set */
			return(iRet);						/* return error */
		uiSec += ptParam->uiFatSec;				/* move to next FAT area */
	}

	/****************************************************/
	/* set root directory								*/
	/****************************************************/
	iRet = _fat_set_root(&tBio, uiSec, ptParam, ptMedia);
	return(iRet);								/* return */
}

/****************************************************************************/
/* FUNCTION:	_grp_fat_find_type_sub										*/
/*																			*/
/* DESCRIPTION:	find FAT type (internal subroutine)							*/
/*		 (Note) FAT sector count may not be optimal due to boudary adjust	*/
/* INPUT:		uiTotalSec:				total sector count					*/
/*				uiOffset:				start offset						*/
/*				iSecShift:				sector shift count					*/
/*				ptReq:					request format parameter			*/
/* OUTPUT:		ptRes:					result format parameter				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_PARAM: 	bad parameter						*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
static int
_grp_fat_find_type_sub(
	grp_uint32_t			uiTotalSec,			/* [IN]  total sector */
	grp_uint32_t			uiOffset,			/* [IN]  start offset */
	int						iSecShift,			/* [IN]  sector shift */
	grp_fat_format_param_t	*ptReq,				/* [IN]  request format param */
	grp_fat_format_param_t	*ptRes)				/* [OUT] result format param */
{
	int						iShift;				/* shift count */
	int						iLimitShift;		/* limit shift count */
	int						iFound = 0;			/* found shift */

	/****************************************************/
	/* check FAT type									*/
	/****************************************************/
	*ptRes = *ptReq;							/* copy request parameter */
	if (iSecShift < FAT_MIN_SEC_SHIFT || iSecShift > FAT_MAX_SEC_SHIFT)
		return(GRP_FS_ERR_BAD_PARAM);			/* return error */
	ptRes->uiNotUsed = uiOffset;				/* for _fat_check_fat_type */
	if (ptReq->uiClstSec) {						/* specified sector/cluster */
		/****************************************************/
		/* check by specified sector/cluster				*/
		/****************************************************/
		if ((ptReq->uiClstSec & (ptReq->uiClstSec - 1))
			|| (ptReq->uiClstSec << iSecShift) 
				> ((grp_uint32_t)1 << FAT_MAX_SEC_SHIFT))
			return(GRP_FS_ERR_BAD_PARAM);		/* return error */
		_fat_check_fat_type(uiTotalSec, iSecShift, ptReq, ptRes);
		if (ptRes->ucFatType == 0 ||
			(ptReq->ucFatType && ptReq->ucFatType != ptRes->ucFatType))
			return(GRP_FS_ERR_BAD_PARAM);		/* return error */
	} else {									/* no sector/cluster info */
		iShift = iSecShift;						/* shift count */
		iLimitShift = _fat_get_clst_shift_limit(uiTotalSec, iSecShift);
												/* get limit shift count */
		for (ptReq->uiClstSec = 1; iShift <= FAT_MAX_SEC_SHIFT;
				 ptReq->uiClstSec <<= 1, iShift++) { /* lookup sector shift */
			_fat_check_fat_type(uiTotalSec, iSecShift, ptReq, ptRes);
												/* check FAT type */
			if (ptRes->ucFatType == 0)			/* no valid type found */
				continue;						/* continue */
			if (ptReq->ucFatType && ptReq->ucFatType > ptRes->ucFatType)
				break;							/* stop here */
			if (ptReq->ucFatType == 0 || ptReq->ucFatType == ptRes->ucFatType) {
				/****************************************************/
				/* found appropriate sector/cluster					*/
				/****************************************************/
				if (iLimitShift >= iShift		/* more appropriate */
					|| iFound == 0)				/* not found yet */
					iFound = iShift;			/* remember shift count */
			}
		}
		if (iFound == 0)						/* not found */
			return(GRP_FS_ERR_BAD_PARAM);		/* return error */
		ptReq->uiClstSec = ((grp_uint32_t)1 << (iFound - iSecShift));
												/* set sec/cluster */
		_fat_check_fat_type(uiTotalSec, iSecShift,
							ptReq, ptRes); 		/* check FAT type again */
	}

	/****************************************************/
	/* adjust cluster boundary							*/
	/****************************************************/
	ptRes->uiAlign = ptReq->uiAlign;			/* copy alignment info */
	ptRes->uiOption = ptReq->uiOption;			/* copy format option */
	ptRes->uiAdjust = (ptReq->uiRsvSecCnt != 0)?
		ptRes->uiRsvSecCnt - ptReq->uiRsvSecCnt: 0; /* set adjust value */
	return(_fat_adjust_boundary(ptRes, uiTotalSec, uiOffset, iSecShift));
												/* adjust boundary */
}

/****************************************************************************/
/* FUNCTION:	grp_fat_find_type											*/
/*																			*/
/* DESCRIPTION:	find FAT type												*/
/* INPUT:		uiTotalSec:				total sector count					*/
/*				uiOffset:				start offset						*/
/*				iSecShift:				sector shift count					*/
/*				ptReq:					request format parameter			*/
/* OUTPUT:		ptRes:					result format parameter				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_PARAM: 	bad parameter						*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_fat_find_type(
	grp_uint32_t			uiTotalSec,			/* [IN]  total sector */
	grp_uint32_t			uiOffset,			/* [IN]  start offset */
	int						iSecShift,			/* [IN]  sector shift */
	grp_fat_format_param_t	*ptReq,				/* [IN]  request format param */
	grp_fat_format_param_t	*ptRes)				/* [OUT] result format param */
{
	int						iRet;				/* return value */
	int						iTry;				/* try count */
	grp_uint32_t			uiAdjust;			/* adjust size */
	grp_uint32_t			uiTotalAdjust = 0;	/* total adjust size */
	grp_fat_format_param_t	tReq;				/* copy of request */

	/****************************************************/
	/* Since _grp_fat_find_type_sub may not return		*/
	/* optimal FAT sector count value in case of		*/
	/* boundary adjust case, loop to call _grp_fat_		*/
	/* find_type_sub until no adjust is made			*/
	/****************************************************/
	tReq = *ptReq;								/* copy request */
	iTry = GRP_FAT_MAX_FIND_TYPE_TRY;			/* set max try count */
	while ((iRet = _grp_fat_find_type_sub(uiTotalSec, uiOffset, iSecShift,
							&tReq, ptRes)) == 0	/* find type */
			&& ptRes->uiAdjust != 0) {			/* need adjust */ 
			if (--iTry <= 0)					/* try count over */
				return(GRP_FS_ERR_BAD_PARAM);	/* return error */
			uiAdjust = ptRes->uiAdjust;			/* adjust size */
			if (tReq.uiOption & GRP_FAT_ADJ_BY_START) { /* adjust by start */
				uiOffset += uiAdjust;			/* adjust start offset */
				uiTotalSec -= uiAdjust;			/* adjust total size */
			} else {
				tReq.uiRsvSecCnt = ptRes->uiRsvSecCnt;	/* adjust reserved */
			}
			tReq.ucFatType = ptRes->ucFatType;	/* set FAT type */
			uiTotalAdjust += uiAdjust;			/* accumulate adjust */
	}
	if (uiTotalAdjust != 0) {					/* need to adjust */
		ptRes->uiAdjust = uiTotalAdjust;		/* set adjust value */
		if (tReq.uiOption & GRP_FAT_ADJ_BY_START) /* adjust by start */
			ptRes->uiNotUsed += uiTotalAdjust;	/* add adjust to not used */
	}
	return(iRet);								/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_conv_vol_label											*/
/*																			*/
/* DESCRIPTION:	select FAT type												*/
/* INPUT:		pucLable		volume label								*/
/*				iSize			label size									*/
/* OUTPUT:		pucLable		volume label								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_PARAM:	bad label							*/
/*																			*/
/****************************************************************************/
static int
_fat_conv_vol_label(
	grp_uchar_t				*pucLabel,			/* [IN/OUT]  volume label */
	int						iSize)				/* [IN]  label size */
{
	grp_uchar_t				*pucChar;			/* character pointer */
	grp_uchar_t				*pucEnd;			/* end pointer */
	int						iChar;				/* character */
	const char				*pc;				/* character pointer */
	int						iByte;				/* character bytes */

	pucEnd = &pucLabel[iSize];					/* end pointer */
	for (pucChar = pucLabel; pucChar < pucEnd && *pucChar != '\0'; 
												pucChar += iByte) {
		iByte =grp_fs_char_cnt(pucChar);		/* get character count */
		if (iByte < 0)	{						/* invalid char sequence */
			return(GRP_FS_ERR_BAD_PARAM);		/* return error */
		}
		if (iByte != 1) {						/* multi-byte char */
			continue;							/* check next char */
		}
		iChar = (int) *pucChar;					/* character */
		if (iChar >= 0x80
			|| (iChar >= '0' && iChar <= '9')
			|| (iChar >= 'A' && iChar <= 'Z')) {
			continue;							/* do nothing */
		}
		if (iChar >= 'a' && iChar <= 'z') {
			*pucChar = (grp_uchar_t)(iChar - 'a' + 'A'); /* convert to upper */
			continue;
		}
		for (pc = " .$%'-_@~`!(){}^#&"; *pc && *pc != (char)iChar; pc++);
		if (*pc == 0) {							/* not found */
			return(GRP_FS_ERR_BAD_PARAM);		/* return error */
		}
	}
	for ( ; pucChar < pucEnd; pucChar++) {		/* until end of volume label */
			*pucChar = ' ';						/* fill space */
	}
	return(0);									/* return success */
}

/****************************************************************************/
/* FUNCTION:	grp_fat_format												*/
/*																			*/
/* DESCRIPTION:	Format media with FAT										*/
/* INPUT:		pcDev:			device name									*/
/*				ptParam:		format parameter							*/
/*				ptMedia:		media parameter								*/
/* OUTPUT:		ptParam:		format parameter							*/
/*				ptMedia:		media parameter								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV		bad device name						*/
/*				GRP_FS_ERR_BAD_PARAM: 	bad parameter						*/
/* 				GRP_FS_ERR_IO:			I/O error							*/
/*				GRP_FS_ERR_FS:			bad partition information 			*/
/*				GRP_FS_ERR_BUSY:		device is mounted					*/
/*				GRP_FS_ERR_NOMEM:		cannot allocate I/O buffer			*/
/*				GRP_FAT_TYPE_12:		FAT12								*/
/*				GRP_FAT_TYPE_16:		FAT16								*/
/*				GRP_FAT_TYPE_32:		FAT32								*/
/*																			*/
/****************************************************************************/
int
grp_fat_format(
	const char				*pcDev,				/* [IN]  device name */
	grp_fat_format_param_t	*ptParam,			/* [IN/OUT]  format parameter */
	grp_fs_media_info_t		*ptMedia)			/* [IN/OUT] media information */
{
	int						iDev;				/* device number */
	int						iMajor;				/* major device number */
	grp_fs_dev_op_t			*ptDevOp;			/* device operation */
	int						iRet;				/* return value */
	grp_int32_t				iHandle;			/* file handle */
	grp_fat_default_t		*ptFatCfg;			/* FAT configuration */
	grp_int32_t				iCurTime;			/* current time */
	grp_int32_t				iBufSize;			/* I/O buffer size */
	int						iLogToDevShift = 0;	/* logical to device shift */
	grp_uchar_t				*pucBuf = NULL;		/* I/O buffer */
	grp_fs_media_info_t		tMedia;				/* media information */
	grp_fat_format_param_t	tReq;				/* request format param */
	grp_fat_format_param_t	tRes;				/* result format param */

	/****************************************************/
	/* lookup device									*/
	/****************************************************/
	memset(&tMedia, 0, sizeof(tMedia));			/* clear media information */
	memset(&tRes, 0, sizeof(tRes));				/* clear result format param */
	iDev = grp_fs_lookup_dev(pcDev);			/* lookup device */
	if (iDev < 0) {								/* not found */
		iRet = iDev;							/* return value */
		goto out_without_close;					/* return error */
	}
	iMajor = GRP_FS_DEV_MAJOR(iDev);			/* major device number */
	ptDevOp = grp_fs_dev_tbl[iMajor].ptOp;		/* device operation */
	if (grp_fs_check_dev_busy(iDev)) {			/* device is mounted */
		iRet = GRP_FS_ERR_BUSY;					/* return value */
		goto out_without_close;					/* return error */
	}

	/****************************************************/
	/* get media information							*/
	/****************************************************/
	iRet = ptDevOp->pfnOpen(iDev, 1, &iHandle,
							&tMedia.uiStartSec, &tMedia.uiTotalSec,
							&tMedia.iSecShift);
	if (iRet != 0)								/* open failed */
		goto out_without_close;					/* return error */

	if (ptDevOp->pfnIoctl) {					/* exists ioctl operation */
		iRet = ptDevOp->pfnIoctl(iDev, GRP_FS_DEV_CTL_GET_MEDIA, &tMedia);
												/* get media information */
		if (iRet < 0 && iRet != GRP_FS_ERR_NOT_SUPPORT)/* failed to get */
			goto out;							/* return error */
	}
	if (ptMedia) {								/* media info specified */
		if (ptMedia->uiTotalSec) {				/* specified total secotors */
			if (tMedia.uiTotalSec
				&& tMedia.uiTotalSec < ptMedia->uiTotalSec) {
				iRet = GRP_FS_ERR_BAD_PARAM;	/* bad parameter */
				goto out;						/* return error */
			}
			tMedia.uiTotalSec = ptMedia->uiTotalSec; /* use it */
		}
		if (ptMedia->usTrkSec) {				/* specified sector/track */
			tMedia.usTrkSec = ptMedia->usTrkSec;/* use track and */
			tMedia.usHead = ptMedia->usHead;	/* head information */
		}
		if (ptMedia->iSecShift) {				/* specified sector shift */
			iLogToDevShift = ptMedia->iSecShift - tMedia.iSecShift;
												/* logical to device shift */
			if (iLogToDevShift < 0) {			/* bad logical shift */
				iRet = GRP_FS_ERR_BAD_PARAM;	/* set return value */
				goto out;						/* return error */
			}
			tMedia.iSecShift = ptMedia->iSecShift; /* use it */
			if (ptMedia->uiTotalSec == 0) {		/* not specified total sector */
				tMedia.uiTotalSec >>= iLogToDevShift; /* convert to logical */
			}
		}
		if (ptMedia->ucMediaType)				/* specified media type */
			tMedia.ucMediaType = ptMedia->ucMediaType; /* use it */
	}
	if (tMedia.iSecShift < FAT_MIN_SEC_SHIFT
		|| tMedia.iSecShift > FAT_MAX_SEC_SHIFT
		|| tMedia.uiTotalSec == 0) {			/* invalid media info */
		iRet = GRP_FS_ERR_BAD_PARAM;			/* set return value */
		goto out;								/* return error */
	}

	/****************************************************/
	/* allocate I/O buffer								*/
	/****************************************************/
	iBufSize = ((grp_int32_t)1 << tMedia.iSecShift);/* sector buffer size */
	if (iBufSize < grp_fat_format_cfg.iBufSize)	/* use bigger buffer */
		iBufSize = grp_fat_format_cfg.iBufSize;	/* use it */
	pucBuf = grp_mem_alloc(iBufSize);			/* allocate buffer */
	if (pucBuf == NULL) {						/* failed to allocate */
		iRet = GRP_FS_ERR_NOMEM;				/* set return value */
		goto out;								/* return error */
	}

	/****************************************************/
	/* get format parameter								*/
	/****************************************************/
	if (ptParam)								/* specified format parameter */
		tReq = *ptParam;						/* get it */
	else										/* not specified */
		memset(&tReq, 0, sizeof(tReq));			/* clear parameter */

	/****************************************************/
	/* check FAT type									*/
	/****************************************************/
	iRet = grp_fat_find_type(tMedia.uiTotalSec, tMedia.uiStartSec,
							 tMedia.iSecShift, &tReq, &tRes);
												/* find FAT type */
	if (iRet < 0)								/* error */
		goto out;								/* return error */

	/****************************************************/
	/* set volume and missing media inforamtion			*/
	/****************************************************/
	memcpy(tRes.aucVolLab,
		   (tReq.aucVolLab[0])? tReq.aucVolLab: (grp_uchar_t *)FAT_LAB_NONAME,
		   sizeof(tRes.aucVolLab));				/* set volume label */
	iRet = _fat_conv_vol_label(tRes.aucVolLab, sizeof(tRes.aucVolLab));
												/* convert volume label */
	if (iRet < 0)								/* convert error */
		goto out;								/* return error */
	grp_fs_get_current_time(&iCurTime);			/* current time */
	FAT_SET_INT32(tRes.aucVolSer, iCurTime);	/* set volume serial number */
	ptFatCfg = &grp_fat_default[(int)tRes.ucFatType - 1];
												/* FAT configuration */
	if (tRes.ucFatType == GRP_FAT_TYPE_12		/* FAT12 type */
		&& tMedia.iSecShift == FAT_SEC_SHIFT) {	/* regular sector shift */
		switch(tMedia.uiTotalSec) {				/* check total sectors */
		case FAT_2HD_SECS:						/* 2HD sectors */
			if (tMedia.ucMediaType == 0)		/* no media type information */
				tMedia.ucMediaType = GRP_FS_MEDIA_2HD;	/* set 2HD type */
			if (tMedia.usTrkSec == 0) {					/* no track info */
				tMedia.usTrkSec = FAT_2HD_TRK_SECS;		/* set track info */
				tMedia.usHead = FAT_2HD_HEADS;			/* set head info */
			}
			break;
		case FAT_2DD_SECS:						/* 2DD sectors */
			if (tMedia.ucMediaType == 0)		/* no media type information */
				tMedia.ucMediaType = GRP_FS_MEDIA_2DD; /* set 2DD type */
			if (tMedia.usTrkSec == 0) {					/* no track info */
				tMedia.usTrkSec = FAT_2DD_TRK_SECS;		/* set track info */
				tMedia.usHead = FAT_2DD_HEADS;			/* set head info */
			}
			break;
		}
	}
	if (tMedia.usTrkSec == 0) {					/* no sector/track info */
		tMedia.usTrkSec = ptFatCfg->usTrkSec;	/* use default track and */
		tMedia.usHead = ptFatCfg->usHead;		/* head information */
	}
	if (tMedia.ucMediaType == 0)				/* no media type information */
		tMedia.ucMediaType = GRP_FS_MEDIA_FIXED;/* set default media type */

	/****************************************************/
	/* format media										*/
	/****************************************************/
	iRet = _fat_format_media(iHandle, iDev, iLogToDevShift, pucBuf, iBufSize,
							&tRes, &tMedia);	/* format media */
	/****************************************************/
	/* return result									*/
	/****************************************************/
out:
	ptDevOp->pfnClose(iHandle, iDev);			/* close device */
	if (pucBuf)									/* allocated buffer */
		grp_mem_free(pucBuf);					/* free buffer */

out_without_close:
	if (ptMedia)								/* specified media param */
		*ptMedia = tMedia;						/* return media parameter */
	if (ptParam)								/* specified format param */
		*ptParam = tRes;						/* return format param */
	if (iRet == 0)								/* no error */
		iRet = (int) tRes.ucFatType;			/* FAT type as return value */
	return(iRet);								/* return */
}
