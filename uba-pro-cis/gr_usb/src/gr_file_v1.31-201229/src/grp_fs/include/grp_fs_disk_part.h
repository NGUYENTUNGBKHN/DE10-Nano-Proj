#ifndef	_GRP_FS_DISK_PART_H_
#define	_GRP_FS_DISK_PART_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_disk_part.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for disk partition handling								*/
/* FUNCTIONS:																*/
/*		grp_fs_get_part				get partition information				*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/06/14	Changed constants for partition less	*/
/*									check by BootSig to a macro checking	*/
/*									jump instruction in BPB					*/
/*		T.Imashiki		2004/07/25	Added grp_fs_set/read/write_part		*/
/*						2004/07/25	Added grp_fs_default_part_type			*/
/*		T.Imashiki		2006/02/23	Changed GRP_FS_PART_IS_BPB macro to		*/
/*									accept BPB like information at MBR		*/
/*									with both FAT12/16 and FAT32 size 0		*/
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

#include "grp_types.h"

/****************************************************************************/
/*  basic constants and parameters for partition handling					*/
/****************************************************************************/
#define GRP_FS_PART_SEC_SZ			512			/* partition sector size */
#define GRP_FS_PART_CNT				4			/* partition count */
#define GRP_FS_PART_ST_OFF			0x1be		/* partition start offset */
#define GRP_FS_PART_TRK_SEC			32			/* default sector/track */
#define GRP_FS_PART_HEAD			128			/* default head count */
#define GRP_FS_PART_MAX_CYL			0x03ff		/* max cylinder */
#define GRP_FS_PART_MAX_HEAD		0xff		/* max head */
#define GRP_FS_PART_MAX_SEC			0x3f		/* max sector in a track */

/****************************************************************************/
/*  macro to check 1st sector is BPB or not 								*/
/****************************************************************************/
#define GRP_FS_PART_BPB_JMP_CODE	0			/* BPB jump instruction code */
#define GRP_FS_PART_BPB_FAT16_SZ	22			/* FAT size for FAT12/16 */
#define GRP_FS_PART_BPB_FAT32_SZ	36			/* FAT size for FAT32 */
#define GRP_FS_PART_IS_BPB(pucBPB)				/* BPB or not */			\
	(((pucBPB)[GRP_FS_PART_BPB_JMP_CODE] == 0xeb	 	/* 1 byte jmp */	\
		|| (pucBPB)[GRP_FS_PART_BPB_JMP_CODE] == 0xe9)	/* 2 byte jmp */	\
	&& (((pucBPB)[GRP_FS_PART_BPB_FAT16_SZ] != 0		/* FAT16 != 0 */	\
			|| (pucBPB)[GRP_FS_PART_BPB_FAT16_SZ+1] != 0)/* FAT16 != 0 */	\
		|| ((pucBPB)[GRP_FS_PART_BPB_FAT32_SZ] != 0		 /* FAT32 != 0 */	\
			|| (pucBPB)[GRP_FS_PART_BPB_FAT32_SZ+1] != 0 /* FAT32 != 0 */	\
			|| (pucBPB)[GRP_FS_PART_BPB_FAT32_SZ+2] != 0 /* FAT32 != 0 */	\
			|| (pucBPB)[GRP_FS_PART_BPB_FAT32_SZ+3] != 0))) /* FAT32 != 0 */

/****************************************************************************/
/*  a disk partition information in disk									*/
/****************************************************************************/
typedef struct grp_fs_dk_part_dk {
	grp_uchar_t				ucActive;			/* active partition */
	grp_uchar_t				aucStartCHS[3];		/* start CHS information */
	grp_uchar_t				ucPartType;			/* partition type */
	grp_uchar_t				aucEndCHS[3];		/* end CHS information */
	grp_uchar_t				aucStartSec[4];		/* start sector */
	grp_uchar_t				aucSecCnt[4];		/* sector count */
} grp_fs_dk_part_dk_t;

/* ucActive */
#define GRP_FS_PART_ACT			0x80			/* active partition */
#define GRP_FS_PART_NACT		0x00			/* not active */

/* ucPartType */
#define GRP_FS_PART_NULL		0x00			/* null partition */
#define GRP_FS_PART_FAT12		0x01			/* FAT12 */
#define GRP_FS_PART_FAT16_L32	0x04			/* FAT16 (< 32MB) */
#define GRP_FS_PART_EXT			0x05			/* extended partition */
#define GRP_FS_PART_FAT16_H32	0x06			/* FAT16 (>= 32MB) */
#define GRP_FS_PART_NTFS		0x07			/* NTFS */
#define GRP_FS_PART_FAT32_CHS	0x0b			/* FAT32 CHS type */
#define GRP_FS_PART_FAT32_LBA	0x0c			/* FAT32 LBA type */
#define GRP_FS_PART_FAT16_LBA	0x0e			/* FAT16 LBA type */
#define GRP_FS_PART_EXT_LBA		0x0f			/* extended partition(LBA) */
#define GRP_FS_PART_FAT32_HCHS	0x1b			/* hidden FAT32 CHS type */
#define GRP_FS_PART_FAT32_HLBA	0x1c			/* hidden FAT32 LBA type */
#define GRP_FS_PART_FAT16_HLBA	0x1e			/* hiddne FAT16 LBA type */
#define GRP_FS_PART_LINUX_SW	0x82			/* LINUX swap or Solaris */
#define GRP_FS_PART_LINUX		0x83			/* LINUX */
#define GRP_FS_PART_LINUX_EXT	0x85			/* LINUX extension */
#define GRP_FS_PART_FREE_BSD	0xa5			/* free BSD */

/****************************************************************************/
/*  disk partition table information in disk								*/
/****************************************************************************/
typedef struct grp_fs_dk_part_tbl {
	grp_fs_dk_part_dk_t		atPart[GRP_FS_PART_CNT]; /* partition info */
	grp_uchar_t				aucSign[2];			/* signature */
} grp_fs_dk_part_tbl_t;

/* ucSign */
#define GRP_FS_PART_SIGN0		0x55			/* sign 0 */
#define GRP_FS_PART_SIGN1		0xaa			/* sign 1 */

/****************************************************************************/
/*  disk partition table information in memory								*/
/****************************************************************************/
typedef struct grp_fs_dk_chs {					/* CHS information */
	grp_uchar_t				ucHead;				/* head */
	grp_uchar_t				ucSec;				/* sector */
	grp_ushort_t			usCyl;				/* cylinder */
} grp_fs_dk_chs_t;

typedef struct grp_fs_dk_part {
	grp_uchar_t				ucActive;			/* active partition */
	grp_uchar_t				ucPartType;			/* partition type */
	grp_fs_dk_chs_t			tStartCHS;			/* start CHS */
	grp_fs_dk_chs_t			tEndCHS;			/* end CHS */
	grp_uint32_t			uiStartSec;			/* start sector */
	grp_uint32_t			uiSecCnt;			/* sector count */
} grp_fs_dk_part_t;

/****************************************************************************/
/*  default partition type table											*/
/****************************************************************************/
typedef struct grp_fs_default_part_type {
	grp_uchar_t				ucPartType12;		/* for FAT12 */
	grp_uchar_t				ucPartType16Small;	/* for FAT16 < 32MB */
	grp_uchar_t				ucPartType16Big;	/* for FAT16 >= 32MB */
	grp_uchar_t				ucPartType32;		/* for FAT32 */
} grp_fs_default_part_type_t;

/****************************************************************************/
/*  exported interface														*/
/****************************************************************************/
int grp_fs_get_part(							/* get partition information */
		grp_uchar_t			*pucSecData,		/* [IN] sector data */
		grp_fs_dk_part_t	*ptPartInfo);		/* [OUT] partition info */

/* return vlaue */
#define GRP_FS_PART_VALID	0					/* valid partition info */
#define GRP_FS_PART_LESS	1					/* partition-less volume */
#define GRP_FS_PART_INVALID	-1					/* invalid partion info */

int grp_fs_set_part(							/* get partition information */
		grp_uchar_t			*pucSecData,		/* [OUT] sector data */
		grp_fs_dk_part_t	*ptPartInfo);		/* [IN] partition info */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_read_part(							/* read partition data */
		const char			*pcDev,				/* [IN]  device name */
		grp_fs_dk_part_t	*ptPart);			/* [OUT] partition table */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_write_part(							/* write partition data */
		const char			*pcDev,				/* [IN]  device name */
		int					iAuto,				/* [IN]  auto with 1 part */
		grp_fs_dk_part_t	*ptPart);			/* [IN/OUT] partition table */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/****************************************************************************/
/*  exported variables														*/
/****************************************************************************/
extern grp_fs_default_part_type_t	grp_fs_default_part_type;
												/* default partition type */

#endif	/* _GRP_FS_DISK_PART_H_ */
