#ifndef	_FAT_H_
#define	_FAT_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	fat.h														*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for FAT I/O												*/
/* FUNCTIONS:																*/
/*		_fat_open_root				return root								*/
/*		_fat_mount					mount function							*/
/*		_fat_unmount				unmount function						*/
/*		_fat_close					close function							*/
/*		_fat_read					read function							*/
/*		_fat_write					write function							*/
/*		_fat_create					create function							*/
/*		_fat_unlink					unlink function							*/
/*		_fat_rename					rename function							*/
/*		_fat_get_attr				get attribute function					*/
/*		_fat_set_attr				set attribute function					*/
/*		_fat_truncate				truncate function						*/
/*		_fat_get_dirent				get directory entry						*/
/*		_fat_match_comp				match component							*/
/*		_fat_check_volume			check volume							*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/07/25	Added FAT_MEDIA_2HD/2DD macros			*/
/*									Added GRP_FS_FAT_CACHE_BY_GET_DIRENT	*/
/*									option for name lookup caching			*/
/*		T.Imashiki		2004/11/30	Deleted FAT_MAX_OPEN and ptOpenFree		*/
/*									field of fat_BPB_t						*/
/*		T.Imashiki		2004/12/07	Added FAT_CNT_BUF_SZ for fast counting	*/
/*									of free FATs at mount					*/
/*		T.Imashiki		2004/12/24	Added fat_open_info_ctl structure,		*/
/*									some fields to fat_open_info structure,	*/
/*									some macros for treating size 0 files,	*/
/*									and modified FAT_EOF_CLST constant to	*/
/*									fix bug of not complying treatment of 	*/
/*									size 0 file								*/
/*		T.Imashiki		2005/02/10 	Added type casts for 16 bit CPU support	*/
/*									Changed macros to check valid character */
/*									for 16 bit CPU support					*/
/*		T.Imashiki		2006/10/31	Added uiFreeHint to fat_BPB				*/
/*		T.Imashiki		2007/10/26	Added fat_interrupt_lookup				*/
/*						2007/10/26	Fixed FAT_CLST macro used in new fix	*/
/*		K.Kaneko		2008/01/11	Added macro to check file system version*/
/*									of BPB									*/
/*		K.Kaneko		2008/05/12	Moved parameters to grp_fat_param.h		*/
/*									(FAT_BLK_SHIFT,FAT_MAP_CNT,FAT_FREE_TBL,*/
/*									 FAT_COMP_SZ,FAT_COMP_CHCNT)			*/
/*		K.Kaneko		2010/11/16	Fixed spell miss at comment				*/
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
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

#include "grp_fs_sysdef.h"
#include "grp_types.h"
#include "grp_fs_if.h"
#include "grp_fs.h"
#include "grp_fat_param.h"

/****************************************************************************/
/*  parameters																*/
/****************************************************************************/
#define FAT_BLK_SZ		(1 << FAT_BLK_SHIFT)/* FAT boot/info block size */
#define FAT_MAX_DENT	((FAT_COMP_CHCNT + FAT_LNAME_CNT - 1)/FAT_LNAME_CNT + 1)
											/* max directory entry count */

/****************************************************************************/
/*  Offset/size definition for BPB											*/
/****************************************************************************/
#define FAT_12_16_SZ		22				/* FAT12/16 FAT size offset */
#define FAT_32_VOL_SER		67				/* volume id offset of (FAT 32) */
#define FAT_32_VOL_LAB		71				/* volume label offset (FAT 32) */
#define FAT_12_16_VOL_SER	39				/* volume id offset (FAT12/16) */
#define FAT_12_16_VOL_LAB	43				/* volume label offset (FAT12/16) */
#define FAT_VOL_LAB_LEN		11				/* volume label length */

/****************************************************************************/
/*  FAT dependent open file information										*/
/****************************************************************************/
typedef struct fat_open_info fat_open_info_t;/* FAT dependent open info */
struct fat_open_info {						/* FAT dependent open info */
	int				iDev;					/* device number */
	grp_uint32_t	uiUniqFid;				/* unique file ID */
	grp_uint32_t	uiDirFid;				/* directory ID */
	grp_uint32_t	uiDirBlk;				/* block number in the directory */
	grp_uint32_t	uiDirStart;				/* start offset in the directory */
	grp_uint32_t	uiDirEnd;				/* end offset in the directory */
	grp_uint32_t	uiMap[FAT_MAP_CNT];		/* cluster mapping table */
	grp_que_fld(fat_open_info_t *, ptSz0);	/* size 0 table/free list */
#ifdef	GRP_FS_FNAME_CACHE
	grp_fs_fname_cache_t *ptFnCache;		/* file name cache */
#endif	/* GRP_FS_FNAME_CACHE */
};

typedef struct fat_open_info_ctl {			/* open info control table */
	fat_open_info_t	*ptOpenInfo;			/* top of whole open info table */
	fat_open_info_t	*ptOpenFree;			/* free list */
	fat_open_info_t	*ptSize0List;			/* size 0 list */
} fat_open_info_ctl_t;

/****************************************************************************/
/*  defines for name length													*/
/****************************************************************************/
#define FAT_BASE_NAME_LEN	8				/* base name part length */
#define FAT_SUFFIX_NAME_LEN	3				/* suffix name part length */
#define FAT_SNAME_LEN		11				/* short name length */
#define FAT_SNAME_BUF_SZ	13				/* short name buffer size */
#define FAT_LNAME_CNT		13				/* long name count */

/****************************************************************************/
/*  logical information of Boot Parameter Block(BPB)						*/
/*	Note: This is not a physical layout of BPB, but is for canonical data	*/
/*		  converted from phisical FAT12/16/32 BPB							*/
/****************************************************************************/
typedef struct fat_BPB {
	grp_uchar_t		aucJmpBoot[3];			/* jump instruction for boot code */
	grp_uchar_t		aucOEMName[8];			/* OEM name */
	grp_uchar_t		ucFatCnt;				/* number of FATs */
	grp_uchar_t		ucMedia;				/* media type */
	grp_uchar_t		aucFsVersion[2];		/* file system version */
	grp_uchar_t		ucSecPerClst;			/* sectors per cluster */
	grp_ushort_t	usBytePerSec;			/* bytes per sector */
	grp_ushort_t	usRsvSecCnt;			/* reserved sector count */
	grp_ushort_t	usRootEntCnt;			/* directory entries in root */
	grp_ushort_t	usSecPerTrk;			/* sector per track */
	grp_ushort_t	usNumHeads;				/* number of heads */
	grp_ushort_t	usExtFlags;				/* extension flags */
	grp_ushort_t	usFsInfo;				/* sector number of FSINFO */
	grp_ushort_t	usBackupBootSec;		/* sector number of backup BPB */
	grp_uint32_t	uiFatSz;				/* FAT sector count */
	grp_uint32_t	uiTotalSec;				/* total number of sectors */
	grp_uint32_t	uiHidenSec;				/* number of hidden sectors */
	grp_uint32_t	uiRootClst;				/* cluster number of root */
	grp_uchar_t		ucDrvNum;				/* drive number */
	grp_uchar_t		ucBootSig;				/* boot signature 0x29 */
	grp_uchar_t		aucVolSer[4];			/* volume serial number */
	grp_uchar_t		aucVolLab[11];			/* volume label */
	grp_uchar_t		aucFsTypeName[8];		/* file system type name */
	grp_ushort_t	usBPBSig;				/* BPB signature 0xaa55 */
	/* following fields are set by computation */
	grp_uint32_t	uiStatus;				/* status */
	grp_uint32_t	uiMaxClst;				/* max cluster */
	grp_uint32_t	uiNextFree;				/* hint of next free cluster */
	grp_uint32_t	uiFreeHint;				/* free clusters stored in hint */
	grp_uint32_t	uiEOC;					/* EOC mark */
	grp_uint32_t	uiEOF;					/* EOF theshould */
	grp_uint32_t	uiBadC;					/* bad cluster mark */
	grp_int32_t		iRootCTime;				/* creation time of root */
	grp_int32_t		iRootMTime;				/* modification time of root */
	grp_int32_t		iRootATime;				/* access time of root */
	int				iFsType;				/* file system type */
	int				iClstShift;				/* cluster shift */
	int				iDBlkShift;				/* data block shift */
	int				iFBlkShift;				/* FAT block shift */
	int				iDBlkClstShift;			/* data block/cluster shift */
	grp_uint32_t	uiClstSize;				/* cluster size */
	grp_uint32_t	uiDBlkSize;				/* data block size */
	grp_uint32_t	uiFBlkSize;				/* FAT  block size */
	grp_uint32_t	uiDBlkStart;			/* start block of cluster 2 */
	grp_uint32_t	uiFatStart;				/* start offset of FAT */
	int				iByteSecShift;			/* byte/sector shift */
	int				iFreeTblCnt;			/* free cache table count */
	int				iFreeGetIdx;			/* free get index */
	int				iFreePutIdx;			/* free put index */
	grp_uint32_t	auiFreeTbl[FAT_FREE_TBL];/* free cache table */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
	grp_uint32_t	uiDirFid;				/* directory file id */
	grp_fs_dir_ent_t tDirCache;				/* directory cache */
	grp_uint32_t	uiDirBlk;				/* block number */
	grp_uint32_t	uiDirStart;				/* start offset */
	grp_uchar_t		aucSCacheName[FAT_SNAME_BUF_SZ];/* cached short file name */
	grp_uchar_t		aucLCacheName[FAT_COMP_SZ]; /* cached long file name */
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
} fat_BPB_t;

/************************************************/
/* ucMedia: valid values are  0xf0, 0xf8 - 0xff	*/
/************************************************/
#define	FAT_MEDIA_FIXED		0xf8			/* fixed media */
#define FAT_MEDIA_REMOVABLE	0xf0			/* removable media */
#define FAT_MEDIA_2HD		0xf0			/* 2HD */
#define FAT_MEDIA_2DD		0xf9			/* 2DD */

/************************************************/
/* usExtFlags									*/
/************************************************/
#define FAT_ACTIVE_NUM(ptBPB)				/* active mirrored FAT number */ \
	((ptBPB)->usExtFlags & 0x0f)
#define FAT_NOT_MIRRORED(ptBPB)				/* not mirrored */				\
	((ptBPB)->usExtFlags & 0x80)

/************************************************/
/* aucFsVersion									*/
/************************************************/
#define FAT_SUPPORT_FSVER_CHK(ptBPB)		/* check file system version */ \
	( ((ptBPB)->aucFsVersion[0] == 0) && ((ptBPB)->aucFsVersion[1] == 0) )

/************************************************/
/* ucDrvNum										*/
/************************************************/
#define FAT_DRV_FD			0x00			/* floppy */
#define FAT_DRV_HD			0x80			/* hard disk */

/************************************************/
/* ucBootSig									*/
/************************************************/
#define FAT_BOOT_SIG		0x29			/* boot signature */

/************************************************/
/* ucVolLab										*/
/************************************************/
#define FAT_LAB_NONAME		"NO NAME    "	/* no volume label */

/************************************************/
/* ucFsTypeName									*/
/************************************************/
#define FAT_TYPE_NAME_12	"FAT12   "		/* FAT12 */
#define FAT_TYPE_NAME_16	"FAT16   "		/* FAT16 */
#define FAT_TYPE_NAME_32	"FAT32   "		/* FAT32 */

/************************************************/
/* usBPBSig										*/
/************************************************/
#define FAT_BPB_SIG			0xaa55			/* BPB signature */

/************************************************/
/* iFsType										*/
/************************************************/
#define FAT_TYPE_12			1				/* FAT12 */
#define FAT_TYPE_16			2				/* FAT16 */
#define FAT_TYPE_32			3				/* FAT32 */

#define FAT_TYPE_IS_12(ptBPB)				/* is FAT 12 */					\
	((ptBPB)->uiMaxClst < 4087)
#define FAT_TYPE_IS_16(ptBPB)				/* is FAT 16 */					\
	((ptBPB)->uiMaxClst >= 4087 && (ptBPB)->uiMaxClst < 65527)
#define FAT_TYPE_IS_32(ptBPB)				/* is FAT 32 */					\
	((ptBPB)->uiMaxClst >= 65527)

/************************************************/
/* uiStatus										*/
/************************************************/
#define FAT_STAT_RONLY		0x00000001		/* read only */
#define FAT_STAT_SYNC_ALL	0x00000002		/* sync write always */
#define FAT_STAT_IO_ERR		0x00000004		/* I/O error occured */
#define FAT_STAT_FREE_CHKED	0x00000008		/* fully checked free block */
#define FAT_STAT_MOD		0x00010000		/* under modification */
#define FAT_STAT_WAIT_MOD	0x00020000		/* wait modification */

/****************************************************************************/
/*  FS information block of FAT file system									*/
/****************************************************************************/
/************************************************/
/* in-memory FS info structure					*/
/************************************************/
typedef struct fat_fs_info {
	grp_uint32_t	uiFiSig1;			/* FS info signature 0x41615252 */
	grp_uint32_t	uiFiSig2;			/* FS info signature 0x61417272 */
	grp_uint32_t	uiFiFreeCnt;		/* free cluster count */
	grp_uint32_t	uiFiNextFree;		/* hint of next free cluster */
	grp_uint32_t	uiFiSig3;			/* FS info signature 0xaa550000 */
} fat_fs_info_t;

#define FAT_FI_SIG1		0x41615252		/* FS info signature 1 */
#define FAT_FI_SIG2		0x61417272		/* FS info signature 2 */
#define FAT_FI_SIG3		0xaa550000		/* FS info signature 3 */

/************************************************/
/* special value in uiFiFreeCnt, uiFiNextFree	*/
/************************************************/
#define FAT_FI_NOHINT	0xffffffff		/* no cluster hint */

/****************************************************************************/
/*  type definition for conversion table from physical to internal form		*/
/****************************************************************************/
typedef struct fat_conv_tbl {
	short			sPhysOff;				/* physical offset */
	short			sPhysSize;				/* physical size */
	grp_uchar_t		ucFldType;				/* logical field type */
	grp_uchar_t		ucFldOff;				/* logical field offset */
} fat_conv_tbl_t;

/************************************************/
/* ucFldOff										*/
/************************************************/
#define FAT_FLD_OFF(type, fld_name)			/* field offset of the type */	\
	((grp_int32_t)(((char *)(&((type *)0)->fld_name))-(char *)0))
#define FAT_BFLD_OFF(fld_name)				/* BPB field offset */		\
	((grp_uchar_t)FAT_FLD_OFF(fat_BPB_t, fld_name))
#define FAT_IFLD_OFF(fld_name)				/* FS information field offset */ \
	((grp_uchar_t)FAT_FLD_OFF(fat_fs_info_t, fld_name))

/************************************************/
/* ucFldType									*/
/************************************************/
#define FAT_FLD_TYPE_NA		0x00			/* discard data */
#define FAT_FLD_TYPE_UC		0x01			/* unsigned char type */
#define FAT_FLD_TYPE_UA		0x02			/* array of unsigned char type */
#define FAT_FLD_TYPE_US		0x03			/* unsigned short type */
#define FAT_FLD_TYPE_UI		0x04			/* unsigned int(32) type */
#define FAT_FLD_TYPE_SHR	0x10			/* shared field by FAT32/12/16 */
#define FAT_FLD_TYPE_SUI	(FAT_FLD_TYPE_UI|FAT_FLD_TYPE_SHR)

#define FAT_FLD_TYPE(ptConv)				/* field basic type value */	\
	((ptConv)->ucFldType & 0x0f)
#define FAT_FLD_TYPE_IS_SHR(ptConv)			/* shared field */				\
	((ptConv)->ucFldType & 0x10)
#define FAT_FLD_TYPE_IS_BYTE(iType)			/* byte type field */		\
	((iType) <= FAT_FLD_TYPE_UA)

/****************************************************************************/
/*  type definitions for data copy function									*/
/****************************************************************************/
typedef	grp_int32_t	(*fat_copy_func_t)(
	grp_uchar_t		*pucDst,				/* destination buffer */
	grp_uchar_t		*pucSrc,				/* source buffer */
	grp_int32_t		iSize);					/* size to modify */

/****************************************************************************/
/*  type definitions for directory access									*/
/****************************************************************************/
typedef struct fat_dir {					/* short name diectrory entry */
	grp_uchar_t			aucName[11];		/* short file name */
	grp_uchar_t			ucAttr;				/* attribute */
	grp_uchar_t			ucRsv;				/* reserved for NT */
	grp_uchar_t			ucCTime10ms;		/* creation time (0-2 sec/10msec) */
	grp_uchar_t			aucCTime[2];		/* creation time (2 sec) */
	grp_uchar_t			aucCDate[2];		/* creation date */
	grp_uchar_t			aucADate[2];		/* last access date */
	grp_uchar_t			aucClstHigh[2];		/* high short word of cluster */
	grp_uchar_t			aucMTime[2];		/* last write time (2 sec) */
	grp_uchar_t			aucMDate[2];		/* last write date */
	grp_uchar_t			aucClstLow[2];		/* low short word of cluster */
	grp_uchar_t			aucFileSize[4];		/* file size */
} fat_dir_t;

typedef struct fat_long_dir {				/* long name directory entry */
	grp_uchar_t			ucOrder;			/* order of long name entry */
	grp_uchar_t			aucName1[10];		/* 0-4 chars of the file name */
	grp_uchar_t			ucAttr;				/* attribute (FAT_ATTR_LONG) */
	grp_uchar_t			ucType;				/* file type (0) */
	grp_uchar_t			ucChkSum;			/* check sum */
	grp_uchar_t			aucName2[12];		/* 5-10 chars of the file name */
	grp_uchar_t			aucClstLow[2];		/* low short word of cluster 0 */
	grp_uchar_t			aucName3[4];		/* 11-12 chars of the file name */
} fat_long_dir_t;

/************************************************/
/* aucName[0]									*/
/************************************************/
#define FAT_DIR_FREE		0xe5			/* free directory entry */
#define FAT_DIR_EOF			0x00			/* end of directory entry */
#define FAT_DIR_E5			0x05			/* alternate value for 0xe5 */

/************************************************/
/* ucAttr										*/
/************************************************/
#define FAT_ATTR_RONLY		0x01			/* read only */
#define FAT_ATTR_HIDDEN		0x02			/* hidden file */
#define FAT_ATTR_SYSTEM		0x04			/* system file */
#define FAT_ATTR_VOLID		0x08			/* volume ID */
#define FAT_ATTR_DIR		0x10			/* directory */
#define FAT_ATTR_ARCHIVE	0x20			/* to be backed up */
#define FAT_ATTR_LONG		0x0f			/* long name */
#define FAT_ATTR_TYPE_MASK	0x3f			/* type mask */

/************************************************/
/* ucOrder										*/
/************************************************/
#define FAT_DIR_LAST_LONG	0x40			/* last long entry */

/************************************************/
/* macros for checking legal characters			*/
/************************************************/
#define FAT_VALID_SNAME_CHAR	0x01		/* valid short name char */
#define FAT_VALID_LNAME_CHAR	0x02		/* valid long name char */

#define FAT_IS_VALID_SNAME(uc)				/* valid short name char */		\
	(_aucFatChar[uc] & FAT_VALID_SNAME_CHAR)
#define FAT_IS_VALID_LNAME(uc)				/* valid long name char */		\
	(_aucFatChar[uc] & FAT_VALID_LNAME_CHAR)
#define FAT_SET_VALID_SCHAR(uc)				/* set valid short char bit */ 	\
	(_aucFatChar[uc] |= FAT_VALID_SNAME_CHAR)
#define FAT_SET_VALID_LCHAR(uc)				/* set valid long char bit */ 	\
	(_aucFatChar[uc] |= FAT_VALID_LNAME_CHAR)
#define FAT_SET_VALID_SLCHAR(uc)			/* set valid short/long char */	\
	(_aucFatChar[uc] |= (FAT_VALID_SNAME_CHAR|FAT_VALID_LNAME_CHAR))

/************************************************/
/* macro to check executable file suffix		*/
/************************************************/
#define FAT_EXEC_FILE_SUFFIX(pucS)			/* executable file suffix */	\
	(((pucS)[0] == 'E' && (pucS)[1] == 'X' && (pucS)[2] == 'E')				\
	|| ((pucS)[0] == 'C' && (pucS)[1] == 'O' && (pucS)[2] == 'M')			\
	|| ((pucS)[0] == 'D' && (pucS)[1] == 'L' && (pucS)[2] == 'L')			\
	|| ((pucS)[0] == 'B' && (pucS)[1] == 'A' && (pucS)[2] == 'T'))

/************************************************/
/* file name type								*/
/************************************************/
#define FAT_FNAME_LOWER			0x01		/* include lower case char */
#define FAT_FNAME_LONG			0x02		/* include non short char */

/****************************************************************************/
/*  FAT cluster values														*/
/****************************************************************************/
#define FAT_EOC_12			0x0fff			/* end of cluster mark for FAT12 */
#define FAT_EOC_16			0xffff			/* end of cluster mark for FAT16 */
#define FAT_EOC_32			0x0fffffff		/* end of cluster mark for FAT32 */
#define FAT_EOF_12			0x0ff8			/* EOF threshould for FAT12 */
#define FAT_EOF_16			0xfff8			/* EOF threshould for FAT16 */
#define FAT_EOF_32			0x0ffffff8		/* EOF threshould for FAT32 */
#define FAT_BADC_12			0x0ff7			/* bad cluster mark for FAT12 */
#define FAT_BADC_16			0xfff7			/* bad cluster mark for FAT12 */
#define FAT_BADC_32			0x0ffffff7		/* bad cluster mark for FAT12 */

#define FAT_EOF_CLST		0x7fffffff		/* EOF cluster number */
#define FAT_EOF_BLK			0xffffffff		/* EOF block number */

#define FAT_SIZE0_CLST		0x80000000		/* pseudo cluster bit for size 0 */
#define FAT_IS_SIZE0_FID(uiFid)				/* is size 0 file */			\
		((uiFid) & FAT_SIZE0_CLST)
#define FAT_IDX_TO_UNIQ_FID(uiIdx)			/* open index to uniq file ID */\
		((uiIdx) | FAT_SIZE0_CLST)
#define FAT_SIZE0_OPEN_INFO(uiFid)			/* open info of size 0 file */	\
		(&grp_fs_fat_open_ctl->ptOpenInfo[(uiFid) & ~FAT_SIZE0_CLST])

#define FAT_FAT0(ptBPB)						/* FAT[0] value */				\
	((((ptBPB)->iFsType == FAT_TYPE_32)? (FAT_EOC_32 & ~0xff): 				\
	  ((ptBPB)->iFsType == FAT_TYPE_16)? (FAT_EOC_16 & ~0xff): 				\
	  (FAT_EOC_12 & ~0xff))													\
	 + (ptBPB)->ucMedia)
#define FAT_FAT1_MASK16		0x3fff			/* FAT1 mask for FAT16 */
#define FAT_FAT1_MASK32		0x03ffffff		/* FAT1 mask for FAT32 */
#define FAT_FAT1_CLEAN16	0x8000			/* cleanly shutdown for FAT16 */
#define FAT_FAT1_CLEAN32	0x08000000		/* cleanly shutdown for FAT32 */
#define FAT_FAT1_NOERR16	0x4000			/* no hard error for FAT16 */
#define FAT_FAT1_NOERR32	0x04000000		/* no hard error for FAT32 */

/************************************************/
/* iMode parameter of _fat_proc_fat1 			*/
/************************************************/
#define FAT_FAT1_MOUNT		1				/* mount time processing */
#define FAT_FAT1_UMOUNT		2				/* umount time processing */

/****************************************************************************/
/*  macros for accessing FAT and data										*/
/****************************************************************************/
#define FAT_NSEC(ptBPB, iBytes)				/* number of sectors */			\
	(((iBytes) + (ptBPB)->usBytePerSec - 1) >> (ptBPB)->iByteSecShift)
#define FAT_DBLK(ptBPB, uiOff)				/* data block number */			\
	((uiOff) >> (ptBPB)->iDBlkShift)
#define FAT_DBLK_OFF(ptBPB, uiOff)			/* data block offset */			\
	((uiOff) & ((ptBPB)->uiDBlkSize - 1))
#define FAT_FBLK(ptBPB, uiOff)				/* FAT block number */			\
	((uiOff) >> (ptBPB)->iFBlkShift)
#define FAT_FBLK_OFF(ptBPB, uiOff)			/* FAT block offset */			\
	((uiOff) & ((ptBPB)->uiFBlkSize - 1))
#ifdef GRP_FS_ENABLE_OVER_2G
#define FAT_NCLST(ptBPB, uiOff)				/* cluster number */			\
	(((uiOff) & ((ptBPB)->uiClstSize - 1))?	/* have surplus? */				\
	(((uiOff) >> (ptBPB)->iClstShift)+1):	/* round out*/					\
	 ((uiOff) >> (ptBPB)->iClstShift))
#else  /* GRP_FS_ENABLE_OVER_2G */
#define FAT_NCLST(ptBPB, uiOff)				/* cluster count */				\
	(((uiOff) + (ptBPB)->uiClstSize - 1) >> (ptBPB)->iClstShift)
#endif /* GRP_FS_ENABLE_OVER_2G */
#define FAT_CLST(ptBPB, uiOff)				/* cluster number */			\
	((uiOff) >> ((ptBPB)->iClstShift))
#define FAT_CLST_OFF(ptBPB, uiOff)			/* cluster offset */			\
	((uiOff) & ((ptBPB)->uiClstSize - 1))
#define FAT_CLST_DBLK(ptBPB, uiClst)		/* cluster to block number */	\
	((uiClst) << (ptBPB)->iDBlkClstShift)
#define FAT_DBLK_CLST(ptBPB, uiBlk)			/* block number to cluster */	\
	((uiBlk) >> (ptBPB)->iDBlkClstShift)
#define FAT_DBLK_NCLST(ptBPB, uiBlk)		/* block to # of clusters */	\
	FAT_DBLK_CLST(ptBPB,													\
				(uiBlk) + ((grp_uint32_t)1 << (ptBPB)->iDBlkClstShift) - 1)
#define FAT_DBLK_CLST_OFF(ptBPB, uiBlk)		/* block cluster offset */		\
	((uiBlk) & (((grp_uint32_t)1 << (ptBPB)->iDBlkClstShift) - 1))
#define FAT_PHYS_CLST_DBLK(ptBPB, iClst, uiOff)	/* phys cluster to block  */\
	((((iClst) - 2) <<  (ptBPB)->iDBlkClstShift) 							\
	+ (ptBPB)->uiDBlkStart + uiOff)
#define FAT_PHYS_DBLK_CLST(ptBPB, uiDBlk)	/* phys data block to cluster */\
	((((uiDBlk) - (ptBPB)->uiDBlkStart) >> ptBPB->iDBlkClstShift) + 2)

#define FAT_OFF(ptBPB, iClst)				/* FAT offset */				\
	(((ptBPB->iFsType == FAT_TYPE_32)? ((iClst) * 4):	/* FAT32 */			\
	 (ptBPB->iFsType == FAT_TYPE_16)? ((iClst) * 2):	/* FAT16 */			\
	 ((iClst) + (iClst / 2)))							/* FAT12 */			\
	+ (ptBPB)->uiFatStart)
#define FATN_OFF(ptBPB, iClst, iFatNum)		/* FATn offset */				\
	(((ptBPB->iFsType == FAT_TYPE_32)? ((iClst) * 4):	/* FAT32 */			\
	 (ptBPB->iFsType == FAT_TYPE_16)? ((iClst) * 2):	/* FAT16 */			\
	 ((iClst) + (iClst / 2)))							/* FAT12 */			\
	+ (((ptBPB)->usRsvSecCnt + (iFatNum) * (ptBPB)->uiFatSz)				\
		<< (ptBPB)->iByteSecShift))

/****************************************************************************/
/*  exported variables													    */
/****************************************************************************/
extern int	(*fat_interrupt_lookup)(int iDev, grp_uint32_t uiFid,
								grp_uint32_t uiOffset); /* interrupt hook */

#endif	/* _FAT_H_ */
