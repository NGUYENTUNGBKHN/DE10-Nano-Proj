#ifndef	_FAT_FORMAT_DEF_H_
#define	_FAT_FORMAT_DEF_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	fat_format_def.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Private definitions for FAT format function							*/
/* FUNCTIONS:																*/
/*																			*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		fat.h																*/
/*		grp_fat_format.h													*/
/*		grp_fs_cfg.h														*/
/*																			*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2004/07/25	Created initial version 1.0				*/
/*		T.Imashiki		2005/02/10 	Added type casts to macros for 16 bit	*/
/*									CPU support								*/
/*		T.Imashiki		2010/11/16	Adjusted max/min cluster count para-	*/
/*		K.Kaneko					 meters to set gap only min side		*/
/*									Added iLogToDevShift field to fat_io_	*/
/*									 buf_t to support logical sector		*/
/*									Changed FAT_XX_NEED_SEC macros to call	*/
/*									 _fat_need_sec							*/
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

#include "grp_types.h"
#include "fat.h"
#include "grp_fat_format.h"
#include "grp_fs_cfg.h"

/****************************************************************************/
/* various constant definitions											    */
/****************************************************************************/
#define	FAT_SEC_SHIFT		9					/* default sector shift count */
#define	FAT_MIN_SEC_SHIFT	9					/* min sector shift count */
#define FAT_MAX_SEC_SHIFT	15					/* max sector shift count */
#define FAT_JMP_OPCODE		0xeb				/* jump instruction */
#define FAT_NOP_OPCODE		0x90				/* nop opcode */
#define FAT_JMP_12_16_DISP	(62 - 2)			/* FAT12/16 jump displacement */
#define FAT_JMP_32_DISP		(90 - 2)			/* FAT32 jump displacement */
#define FAT_2HD_HEADS		2					/* heads for 2HD */
#define FAT_2HD_TRK_SECS	18					/* sector/track for 2HD */
#define FAT_2HD_CYLS		80					/* cylinder for 2HD */
#define FAT_2HD_SECS		(FAT_2HD_CYLS * FAT_2HD_TRK_SECS * FAT_2HD_HEADS)
#define FAT_2DD_TRK_SECS	9					/* sector/track for 2HD */
#define FAT_2DD_HEADS		2					/* heads for 2DD */
#define FAT_2DD_CYLS		80					/* cylinder for 2HD */
#define FAT_2DD_SECS		(FAT_2DD_CYLS * FAT_2DD_TRK_SECS * FAT_2DD_HEADS)
#define FAT_12_END_CLST		0x00000ff6			/* FAT12 end cluster count */
#define FAT_16_END_CLST		0x0000fff6			/* FAT16 end cluster count */
#define FAT_32_END_CLST		0x0ffffff6			/* FAT32 end cluster count */
#define FAT_12_MIN_CLST		2					/* min FAT12 cluster */
#define FAT_12_MAX_CLST		(FAT_12_END_CLST)
#define FAT_16_MIN_CLST		(FAT_12_END_CLST + 1 + grp_fat_format_cfg.uiSafetyGap)
#define FAT_16_MAX_CLST		(FAT_16_END_CLST)
#define FAT_32_MIN_CLST		(FAT_16_END_CLST + 1 + grp_fat_format_cfg.uiSafetyGap)
#define FAT_32_MAX_CLST		FAT_32_END_CLST		/* max FAT32 cluster */
#define FAT_BPB_SIG_OFF		510					/* BPB signature offset */
#define FAT_BPB_SIG1		0x55				/* BPB signature 1 */
#define FAT_BPB_SIG2		0xaa				/* BPB signature 2 */

/****************************************************************************/
/* various macros														    */
/****************************************************************************/
#define FAT_SEC_SZ(iSecShift)					/* sector size */			\
	((grp_int32_t)1 << (iSecShift))
#define FAT_B2SEC(iSize, iSecShift)				/* number of sectors */		\
	(((iSize) + FAT_SEC_SZ(iSecShift) - 1) >> (iSecShift))
#define FAT_RND_UP(x,y)		(((x)+(y)-1)/(y))	/* divide with round up */
#define FAT_FAT12_SEC(uiClstCnt, iSecShift)		/* sectors for FAT12 area */\
	FAT_B2SEC(((uiClstCnt) * 3 + 1) / 2, iSecShift)
#define FAT_FAT16_SEC(uiClstCnt, iSecShift)		/* sectors for FAT16 area */\
	FAT_B2SEC((uiClstCnt) * 2, iSecShift)
#define FAT_FAT32_SEC(uiClstCnt, iSecShift)		/* sectors for FAT32 area */\
	FAT_B2SEC((uiClstCnt) * 4, iSecShift)
#define FAT_DIR_SEC(uiDirCnt, iSecShift)		/* sectors for directory */	\
	FAT_B2SEC((uiDirCnt) * sizeof(fat_dir_t), iSecShift)
#define FAT_12_NEED_SEC(ptParam, uiClstCnt, iSecShift) /* FAT12 need sectors */\
	_fat_need_sec((ptParam), uiClstCnt, 									\
	    FAT_FAT12_SEC(uiClstCnt, iSecShift) * GRP_FAT_AREA_CNT +			\
		FAT_DIR_SEC((ptParam)->uiRootCnt, iSecShift))
#define FAT_16_NEED_SEC(ptParam, uiClstCnt, iSecShift) /* FAT16 need sectors */\
	_fat_need_sec((ptParam), uiClstCnt, 									\
	    FAT_FAT16_SEC(uiClstCnt, iSecShift) * GRP_FAT_AREA_CNT +			\
		FAT_DIR_SEC((ptParam)->uiRootCnt, iSecShift))
#define FAT_32_NEED_SEC(ptParam, uiClstCnt, iSecShift) /* FAT32 need sectors */\
	_fat_need_sec((ptParam), uiClstCnt, 									\
	    FAT_FAT32_SEC(uiClstCnt, iSecShift) * GRP_FAT_AREA_CNT)
#define FAT_GET_CNT_PARAM(ptRes, ptReq, ptDef, iSecShift) { /* count param */ \
	if ((ptReq)->uiClstSec) {					/* sector/cluster specified */\
		(ptRes)->uiClstSec = (ptReq)->uiClstSec;/* use it */				\
	} else {									/* not specified */			\
		(ptRes)->uiClstSec = 1;					/* set 1 */					\
	}																		\
	(ptRes)->uiRootCnt = ((ptReq)->uiRootCnt)?								\
		(ptReq)->uiRootCnt: (ptDef)->uiRootCnt;	/* root entry count */		\
	(ptRes)->uiRsvSecCnt = ((ptReq)->uiRsvSecCnt)?							\
		(ptReq)->uiRsvSecCnt: (ptDef)->uiRsvSecCnt;	/* rsv sector count */	\
}
#define FAT_SET_INT16(aucFld, uiVal) {			/* set 16 bit int value */	\
	grp_uint32_t	_uiVal = (uiVal);			/* use local for speed-up */\
	(aucFld)[0] = (grp_uchar_t)(_uiVal & 0xff);	/* lower byte */			\
	(aucFld)[1] = (grp_uchar_t)((_uiVal >> 8) & 0xff);/* higher byte */		\
}
#define FAT_SET_INT32(aucFld, uiVal) {			/* set 16 bit int value */	\
	grp_uint32_t	_uiVal = (uiVal);			/* use local for speed-up */\
	(aucFld)[0] = (grp_uchar_t)(_uiVal & 0xff);	/* lowest byte */			\
	(aucFld)[1] = (grp_uchar_t)((_uiVal >> 8) & 0xff);/* 2nd lower byte */	\
	(aucFld)[2] = (grp_uchar_t)((_uiVal >> 16) & 0xff);/* 3rd lower byte */	\
	(aucFld)[3] = (grp_uchar_t)((_uiVal >> 24) & 0xff);/* highest byte */	\
}

/****************************************************************************/
/* I/O buffer control block												    */
/****************************************************************************/
typedef struct fat_io_buf {						/* I/O buffer */
	grp_uint32_t		uiSec;					/* sector number */
	grp_int32_t			iBufSize;				/* buffer size */
	grp_uint32_t		uiSecCnt;				/* sector count */
	grp_uchar_t			*pucBuf;				/* buffer */
	grp_int32_t			iHandle;				/* I/O handle */
	int					iDev;					/* device number */
	int					iLogToDevShift;			/* logical to device shift */
	grp_fs_dev_op_t		*ptDevOp;				/* device operation */
	grp_fs_media_info_t	*ptMedia;				/* media information */
} fat_io_buf_t;

/****************************************************************************/
/* type definition for BPB in disk											*/
/****************************************************************************/
typedef struct fat_dk_BPB_head {				/* head part of BPB */
	grp_uchar_t			aucJumpInst[3];			/* jump instruction */
	grp_uchar_t			aucOEMName[8];			/* OEM name */
	grp_uchar_t			aucSecByte[2];			/* byte/sector */
	grp_uchar_t			ucClstSec;				/* sector/cluster */
	grp_uchar_t			aucRsvSec[2];			/* reserved sectors */
	grp_uchar_t			ucFatCnt;				/* number of FATs */
	grp_uchar_t			aucRootCnt[2];			/* root entry count */
	grp_uchar_t			aucTotalSec16[2];		/* sector count(< 0x10000) */
	grp_uchar_t			ucMedia;				/* media type */
	grp_uchar_t			aucFatSec16[2];			/* FAT12/16 sector count */
	grp_uchar_t			aucTrkSec[2];			/* sector/track */
	grp_uchar_t			aucHead[2];				/* heads */
	grp_uchar_t			aucHiddenSec[4];		/* hidden sectors */
	grp_uchar_t			aucTotalSec32[4];		/* sector count(>= 0x10000) */
} fat_dk_BPB_head_t;

typedef struct fat_dk_BPB_32 {					/* FAT32 specific part of BPB */
	grp_uchar_t			aucFatSec32[4];			/* FAT32 sector count */
	grp_uchar_t			aucExtFlags[2];			/* extension flags */
	grp_uchar_t			aucFsVersion[2];		/* file system version */
	grp_uchar_t			aucRootClst[4];			/* root cluster number */
	grp_uchar_t			aucFsInfo[2];			/* sector num of FSINFO */
	grp_uchar_t			aucBackupBootSec[2];	/* sector num of backup BPB */
	grp_uchar_t			aucRsv[12];				/* reserved */
} fat_dk_BPB_32_t;

typedef struct fat_dk_BPB_tail {				/* tail part of BPB */
	grp_uchar_t			ucDrvNum;				/* drive number */
	grp_uchar_t			ucRsv;					/* reserved */
	grp_uchar_t			ucBootSig;				/* boot signature */
	grp_uchar_t			aucVolSer[4];			/* volume serial number */
	grp_uchar_t			aucVolLab[11];			/* volume label */
	grp_uchar_t			aucFsTypeName[8];		/* FAT type name */
} fat_dk_BPB_tail_t;

/****************************************************************************/
/* type definition for FSINFO in disk										*/
/****************************************************************************/
typedef struct fat_dk_fsinfo {					/* FSINFO structure */
	grp_uchar_t			aucFiSig1[4];			/* signature 0x41615252 */
	grp_uchar_t			aucRsv1[480];			/* reserved */
	grp_uchar_t			aucFiSig2[4];			/* signature 0x61417272 */
	grp_uchar_t			aucFiFreeCnt[4];		/* free cluster count */
	grp_uchar_t			aucFiNextFree[4];		/* next free cluster */
	grp_uchar_t			aucRsv2[12];			/* reserved */
	grp_uchar_t			aucFiSig3[4];			/* sigunature 0xaa550000 */
} fat_dk_fsinfo_t;

#endif	/* _FAT_FORMAT_DEF_H_ */
