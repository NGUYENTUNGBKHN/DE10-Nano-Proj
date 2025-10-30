#ifndef	_GRP_FAT_FORMAT_H_
#define	_GRP_FAT_FORMAT_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fat_format.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Exported format function interface for FAT file system				*/
/* FUNCTIONS:																*/
/*		grp_fat_format				format media							*/
/*		grp_fat_find_type			find FAT type							*/
/*																			*/
/* DEPENDENCIES:															*/
/*		grp_fs_dev_io_if.h													*/
/*																			*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2004/07/25	Created initial version 1.0				*/
/*		T.Imashiki		2004/11/28	Changed counts GRP_FAT_12_ROOT_CNT and	*/
/*									GRP_FAT_16_ROOT_CNT						*/
/*		T.Imashiki		2010/11/16	Added GRP_FAT_BOOT_SEC_CNT for SDHC		*/
/*		K.Kaneko					Changed GRP_FAT_SAFETY_GAP parameter	*/
/*									 to match with bit increasing point of	*/
/*									 cluster count like 0x1001 for FAT12->	*/
/*									 FAT16.									*/
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

#include "grp_fs_dev_io_if.h"

/****************************************************************************/
/* default parameters													    */
/****************************************************************************/
#define GRP_FAT_512_CLST_LIMIT	(4200*2)		/* 512 cluster for < 4.1MB */
#define GRP_FAT_1K_CLST_LIMIT	(0x4000*2)		/* 1K  cluster for < 16MB  */
#define GRP_FAT_2K_CLST_LIMIT	(0x20000*2)		/* 2K  cluster for < 128MB */
#define GRP_FAT_4K_CLST_LIMIT	(0x40000*2)		/* 4K  cluster for < 256MB */
#define GRP_FAT_8K_CLST_LIMIT	0				/* 8K  cluster for max */
#define GRP_FAT_16K_CLST_LIMIT	0				/* 16K cluster not recommend */
#define GRP_FAT_32K_CLST_LIMIT	0				/* 32K cluster not recommend */
#define GRP_FAT_SAFETY_GAP		10				/* gap between FAT types */
#define GRP_FAT_PART_OFF		1				/* partition offset(track) */
#define GRP_FAT_OEM_NAME		"GR-FILE "		/* OEM name */
#define GRP_FAT_12_ROOT_CNT		256				/* FAT12 root entry count */
#define GRP_FAT_16_ROOT_CNT		512				/* FAT16 root entry count */
#define GRP_FAT_32_ROOT_CNT		512				/* FAT32 root entry count */
#define GRP_FAT_12_RSV_SEC		1				/* FAT12 reserved secs */
#define GRP_FAT_16_RSV_SEC		1				/* FAT16 reserved secs */
#define GRP_FAT_32_RSV_SEC		32				/* FAT32 reserved secs */
#define GRP_FAT_12_TRK_SECS		18				/* FAT12 default sector/track */
#define GRP_FAT_16_TRK_SECS		32				/* FAT16 default sector/track */
#define GRP_FAT_32_TRK_SECS		32				/* FAT32 default sector/track */
#define GRP_FAT_12_HEADS		2				/* FAT12 default heads */
#define GRP_FAT_16_HEADS		128				/* FAT16 default heads */
#define GRP_FAT_32_HEADS		128				/* FAT32 default heads */
#define GRP_FAT_AREA_CNT		2				/* FAT count */
#define GRP_FAT_ROOT_CLST		2				/* root cluster number */
#define GRP_FAT_FSINFO_SEC		1				/* sector num of FSINFO */
#define GRP_FAT_BACKUP_BPB		6				/* sector num of backup BPB */
#define GRP_FAT_BOOT_SEC_CNT	3				/* boot sector count */
#define GRP_FAT_IO_BUF_SZ		4096			/* format I/O buffer size */

/****************************************************************************/
/* format parameters													 	*/
/****************************************************************************/
typedef struct grp_fat_format_param_t {			/* format parameter */
	grp_uchar_t			ucFatType;				/* [IN/OUT] FAT type */
	grp_uchar_t			aucVolLab[11];			/* [IN/OUT] volume label */
	grp_uint32_t		uiClstSec;				/* [IN/OUT] sector/cluster */
	grp_uint32_t		uiRootCnt;				/* [IN/OUT] root entry count */
	grp_uint32_t		uiRsvSecCnt;			/* [IN/OUT] rsv sector count */
	grp_uint32_t		uiAlign;				/* [IN]    cluster area align */
	grp_uint32_t		uiOption;				/* [IN]     format option */
	grp_uint32_t		uiClst;					/* [OUT]    cluster count */
	grp_uint32_t		uiFatSec;				/* [OUT]    FAT sector count */
	grp_uchar_t			aucVolSer[4];			/* [OUT]    volume serial No. */
	grp_uint32_t		uiNotUsed;				/* [OUT]    not used sector */
	grp_uint32_t		uiAdjust;				/* [OUT]    rsv/hidden adjust */
} grp_fat_format_param_t;

/* ucFatType - FAT type */
#define GRP_FAT_TYPE_12			1				/* FAT 12 */
#define GRP_FAT_TYPE_16			2				/* FAT 16 */
#define GRP_FAT_TYPE_32			3				/* FAT 32 */

/* uiOption - format option */
#define GRP_FAT_NO_CRT_ACC_TIME	0x00000001		/* no create/access time */
#define GRP_FAT_ADJ_BY_START	0x00000002		/* adjust by start offset */

/****************************************************************************/
/* default format configuration parameters								    */
/****************************************************************************/
typedef struct grp_fat_default {				/* default cfg for each type */
	grp_uint32_t		uiRootCnt;				/* root entry count */
	grp_uint32_t		uiRsvSecCnt;			/* reserved sector count */
	grp_uint16_t		usTrkSec;				/* sector/track */
	grp_uint16_t		usHead;					/* heads */
} grp_fat_default_t;

typedef struct grp_fat_format_cfg {				/* format configuration data */
	const grp_uchar_t	aucOEMName[8];			/* OEM name */
	grp_uint32_t		uiSafetyGap;			/* safety gap between types */
	grp_int32_t			iBufSize;				/* I/O buffer size */
} grp_fat_format_cfg_t;

/****************************************************************************/
/* exported interfaces														*/
/****************************************************************************/
int grp_fat_format(								/* format media */
		const char				*pcDev,			/* [IN]  device name */
		grp_fat_format_param_t	*ptParam,		/* [IN/OUT]  format parameter */
		grp_fs_media_info_t		*ptMedia);		/* [IN/OUT] media information */

int grp_fat_find_type(							/* find FAT type */
		grp_uint32_t			uiTotalSec,		/* [IN]  total sector */
		grp_uint32_t			uiOffset,		/* [IN]  start offset */
		int						iSecShift,		/* [IN]  sector shift */
		grp_fat_format_param_t	*ptReq,			/* [IN]  request format param */
		grp_fat_format_param_t	*ptRes);		/* [OUT] result format param */

/****************************************************************************/
/* exported variables														*/
/****************************************************************************/
extern grp_fat_format_cfg_t	grp_fat_format_cfg;	/* format configuration param */
extern grp_fat_default_t	grp_fat_default[];	/* default each format param */
extern grp_uint32_t			grp_fat_cluster_limit_tbl[];
												/* cluster size limit table */

#endif	/* _GRP_FAT_FORMAT_H_ */
