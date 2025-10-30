/****************************************************************************/
/*																			*/
/* FILE NAME:	fat.c														*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		FAT file system access routines										*/
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
/*		<string.h>															*/
/*		fat.h																*/
/*		grp_fs.h															*/
/*		grp_time_lib.h														*/
/*		grp_mem.h															*/
/*		fat_multi_language.h												*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2003/12/19	Deleted debug option code				*/
/*		T.Imashiki		2004/06/14	Added file name cache code				*/
/*									Added iPurge parameter for _fat_match_	*/
/*									comp									*/
/*									Optimized file creation sequence		*/
/*		T.Imashiki		2004/07/25	Added GRP_FS_NO_MNT_FLAG processing		*/
/*									Added GRP_FS_NO_UPD_ACCTIME processing	*/
/*									Added GRP_FS_NO_CRT_ACCTIME processing	*/
/*									Added init at mount to fix non init		*/
/*									sequence at create without lookup		*/
/*									Added purge data cache at mount not to	*/
/*									get data for previous device			*/
/*									Fixed read error in _fat_check_volume	*/
/*									for sector size bigger than 512			*/
/*									Added GRP_FS_FAT_CACHE_BY_GET_DIRENT	*/
/*									option for name lookup caching			*/
/*		T.Imashiki		2004/10/29	Fixed bug not updating directory entry	*/
/*									cache at close time	for GRP_FS_FAT_		*/
/*									CACHE_BY_GET_DIRENT option				*/
/*						2004/11/12	Fixed matching volume label and 0		*/
/*									length string by mistake by GRP_FS_FAT_ */
/*									CACHE_BY_GET_DIRENT option				*/
/*		T.Imashiki		2004/11/28	Fixed a bug in case of file cache block */
/*									size is greater than data offset		*/
/*		T.Imashiki		2004/11/30	Fixed bug for improperly handling error */
/*									from _fat_alloc_open_info at _fat_get_  */
/*									dirent									*/
/*									Changed free list of open information   */
/*									from mount private to global			*/
/*		T.Imashiki		2004/12/07	Changed implementation of _fat_get_free */
/*									_cache for fast FAT lookup				*/
/*									Added grp_fs_fat_cnt_buf_sz for fast	*/
/*									free FAT lookup at mount time			*/
/*									Added buffer kind parameter to grp_fs_	*/
/*									exec_dev_io for new _fat_get_free_cache	*/
/*									Added GRP_FS_FAT_NO_DIR_SIZE_INFO		*/
/*									option not to set size information		*/
/*									by _fat_get_dirent						*/
/*		T.Imashiki		2004/12/20	Fixed bug for infinite loop with a file	*/
/*									name with space							*/
/*		T.Imashiki		2004/12/24	Fixed bug of not complying treatment of */
/*									size 0 file; this bug fix needed wide-	*/
/*									range of additions and modifications:	*/
/*									<added functions>						*/
/*									  _fat_lookup_size0_file, _fat_deque_	*/
/*									  size0_list, _fat_set_1st_cluster,		*/
/*									  _fat_free_1st_cluster, _fat_update_	*/
/*									  fname_cache, _fat_update_attr			*/
/*									<added variables>						*/
/*									  grp_fs_fat_open_ctl					*/
/*									<modified functions>					*/
/*									  _fat_init_free_open_info, _fat_alloc_	*/
/*									  open_info, _fat_free_open_info, _fat_	*/
/*									  phys_cluster, _fat_get_free_cluster,	*/
/*									  _fat_count_cont_blk, _fat_io_direct,	*/
/*									  _fat_write, _fat_get_next_dir_ent,	*/
/*									  _fat_open_root, _fat_match_comp, _fat */
/*									  _close, _fat_write_directory, _fat_	*/
/*									  create_rename_file, _fat_unlink, _fat */
/*									  get_attr, _fat_truncate, _fat_get_	*/
/*									  dirent 								*/
/*		T.Imashiki		2004/12/24	Added and exported init function		*/
/*									grp_fs_fat_init							*/
/*		T.Imashiki		2005/01/05	Fixed to write 0 as a cluster number	*/
/*									for a root directory even FAT32		    */
/*									Fixed to clear size information for a	*/
/*									directory file in directory cache		*/
/*		T.Imashiki		2005/02/10 	Added type casts and changed variable   */
/*									return types of some functions for 16   */
/*									bit CPU support							*/
/*									Changed type of valid character table	*/
/*									for 16 bit CPU support					*/
/*		T.Imashiki		2005/04/18	Fixed bug in setting file name cache	*/
/*									for created short file name directory	*/
/*		T.Imashiki		2005/10/03	Fixed bug not read block in some case   */
/*									of patial block write				    */
/*		T.Imashiki		2005/11/04	Fixed dead lock by ".." access from     */
/*									multiple tasks at _fat_match_comp and	*/
/*									_fat_get_dirent							*/
/*		T.Imashiki		2005/11/18	Fixed unexpected unsigned comparison	*/
/*									by sizeof at _fat_get_next_dir_ent 		*/
/*		T.Imashiki		2005/11/25	Fixed to cleanup partially written		*/
/*									 directory entry					    */
/*									Fixed uninitialized buffer reference	*/
/*									 at _fat_write_file_internal and		*/
/*									 _fat_mount in some error case, and		*/
/*									 clear buffer pointer at _fat_write and	*/
/*									 _fat_alloc_clear_cluster for safety	*/
/*									Fixed to delete duplicated free at		*/
/*									 _fat_write_directory for error case	*/
/*									Fixed deadlock by mutiple accesses at	*/
/*									 _fat_set_1st_cluster, _fat_free_1st_	*/
/*									 cluster, _fat_get_free_cluster, and	*/
/*									 _fat_create_rename_file				*/
/*									Added write lock operation at _fat_		*/
/*									 write, _fat_create_rename_file, and	*/
/*									 _fat_truncate for deadlock elimination	*/
/*									Fixed to purge stale ".." file name		*/
/*									 cache by renaming directory			*/
/*		T.Imashiki		2006/1/31	Fixed to purge state size 0 file name	*/
/*									 cache at _fat_free_open_info			*/
/*						2006/1/31	Fixed to reset directory entry buffer	*/
/*									size not to corrupt with bad file		*/
/*									system									*/
/*		T.Imashiki		2006/9/12	Fixed to set real file cache for ".."	*/
/*									instead of ".." one, and not to set		*/
/*									file name cache for ".." in order to	*/
/*									avoid alias problem and miss-reference	*/
/*									to removed file with ".." cache			*/
/*						2006/9/12	Fixed comment for _fat_get/set_flds,	*/
/*									and _fat_set_fatn functions				*/
/*									Changed parameter type of _fat_update_	*/
/*									time from unsigned to int for strict	*/
/*									consistency								*/
/*		T.Imashiki		2006/10/18	Release cache buffer at _fat_get_free_	*/
/*									cluster and _fat_get_dirent not to		*/
/*									dead-lock in cache buffer shortage		*/
/*		T.Imashiki		2006/10/31	Added _fat_sync interface and grp_fs_	*/
/*									 sync_hint variable to write back		*/
/*									 FAT32 free cluster hint information at	*/
/*									 other than unmount						*/
/*									Added write back operation of updated	*/
/*									 file attribute in _fat_write and _fat_	*/
/*									 truncate for GRP_FS_STAT_SYNC_ALL case	*/
/*									Fixed to write back with both SYNC and	*/
/*									 TSYNC bits set at _fat_clean_unref_buf	*/
/*						2006/10/31	Fixed to support logical sector size	*/
/*									 different from physical one even in	*/
/*									 direct I/O								*/
/*									Modified to return error number from	*/
/*									 device driver instead of GRP_FS_ERR_IO	*/
/*		T.Imashiki		2006/11/10	Adjusted file size for over 2GB file	*/
/*						2006/11/17	Update execute attribute bits at rename */
/*									case in _fat_write_directory			*/
/*		T.Imashiki		2007/01/16	Fixed to add missing grp_fs_block/		*/
/*									unblock_fs_mod calls at _fat_free_		*/
/*									cluster_list							*/
/*		T.Imashiki		2007/02/20 	Added type casts and changed variable/  */
/*									parameter types of some functions for	*/
/*									16 bit CPU support						*/
/*		T.Imashiki		2007/03/23	Fixed lock loop with broken directory	*/
/*									information at _fat_get_dirent			*/
/*		T.Imashiki		2007/10/26	Added check for invalid block read 		*/
/*									request with invalid file system at		*/
/*									_fat_get_blk							*/
/*						2007/10/26	Added duplicated cluster check to		*/
/*									_fat_get_free_cluster to avoid deadlock */
/*						2007/10/26	Added check of root FAT at mount		*/
/*									without force option					*/
/*						2007/10/26	Added check 1st cluster of file at open */
/*									according to flag fat_check_cluster_at_	*/
/*									open (default=1)						*/
/*						2007/10/26	Added max offset check and interrupt	*/
/*									hook function to directory search and	*/
/*									directory size computation in case of	*/
/*									cluster loop							*/
/*		T.Imashiki		2007/11/16	Fixed miscomputation of file making		*/
/*									time information						*/
/*		T.Imashiki		2008/01/09	Cleared *pptFile in case of cluster		*/
/*									check error at _fat_match_comp		    */
/*						2008/01/09	Deleted unnecessary new line in an		*/
/*									error message of _fat_free_cluster		*/
/*		K.Kaneko		2008/01/11	Added check file system version of BPB	*/
/*		T.Imashiki		2008/01/25	Fixed overwrite of return value with	*/
/*									that of _fat_clean_unref_buf in case	*/
/*									of an error at _fat_get_free_cluster	*/
/*						2008/01/25	Changed to return GRP_FS_ERR_FS if		*/
/*									file system error is detected at		*/
/*									_fat_free_cluster_list_n				*/
/*						2008/01/25	Moved to call grp_fs_buf_fill_end		*/
/*									before _fat_free_cluster_list at		*/
/*									_fat_write for copyin error case		*/
/*		M.Toyama		2008/03/05	Include multi language support function */
/*		T.Imashiki		2008/04/04	Fixed I/O processing in _fat_get_free_  */
/*									cache to handle logical/physical sector */
/*									mismatch case							*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		K.Kaneko		2008/07/02	Added ignore msb 4 bits of FAT			*/
/*									Added read over sector length again at	*/
/*									_fat_check_volume						*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		T.Imashiki		2010/11/16	Improved next free hint in FSINFO		*/
/*		K.Kaneko					Fixed to report an error for EOC in the */
/*									 middle of FAT chain in DIRECT I/O		*/
/*									Added _fat_get_short_after_long and		*/
/*									 changed processing method to get short	*/
/*									 entry after long one to process orphan	*/
/*									 long entry more correctly				*/
/*									Fixed to correctly create a file with	*/
/*									 file name starting with E5				*/
/*									Added GRP_FS_FAT_TRY_NO_NUM_SHORT		*/
/*									 option, and not to try no number short	*/
/*									 name associated for a long name as		*/
/*									 default								*/
/*									Fixed to add purge operation before		*/
/*									 setup new file name cache not to left	*/
/*									 old one by mistake in case of previous	*/
/*									 short cache setup error				*/
/*									Changed to return volume label as it is	*/
/*									Made the use of local stack buffer in	*/
/*									 _fat_check_volume as GRP_FAT_USE_LOCAL */
/*									 _BUF option							*/
/*									Restore directory entry in case of no	*/
/*									 open cache in _fat_write_directory		*/
/*									Fixed to make no extra close in case of */
/*									 _fat_write_directory error by rename	*/
/*									Added check and roll back of directory	*/
/*									 expansion error to _fat_clear_cluster,	*/
/*									 _fat_alloc_clear_cluster, and _fat_free*/
/*									 cluster_list							*/
/*									Fixed to restore open information in	*/
/*									 case of _fat_change_dir_link error in	*/
/*									 _fat_write_directory					*/
/*									Fixed to restore to not EOF entry but	*/
/*									 free entry in case of directory write	*/
/*									 error in _fat_write_directory			*/
/*									Restored directory entry in case of		*/
/*									 update failure in _fat_set_1st_cluster	*/
/*									Restored back to free cluster in case	*/
/*									 error in _fat_get_free_cluster			*/
/*									Fixed to add open information count		*/
/*									 in _fat_init_free_open_info by			*/
/*									 grp_fs_param.sTasks not to exhaust with*/
/*									 accesses from multiple tasks			*/
/*		K.Kaneko		2011/05/23	Added to upper case name in _fat_match	*/
/*									_comp and _fat_create_rename_file		*/
/*		K.Kaneko		2011/05/25	Added set archive attribute in			*/
/*									_fat_write and _fat_create_rename_file	*/
/*		K.Kaneko		2011/05/30	Added make short name function of		*/
/*									another method							*/
/*		K.Kaneko		2011/08/11	Fixed type casts of _fat_make_short_ent	*/
/*									_name function and _fat_match_comp		*/
/*									function								*/
/*		K.Kaneko		2011/10/03	Fixed loop condition when the value of	*/
/*									next free hint is 3 in					*/
/*									_fat_get_free_cache						*/
/*		K.Kaneko		2011/10/03	Fixed judgment way of readonly mount	*/
/*									in _fat_get_free_cache					*/
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
/*		K.Kaneko		2020/12/09	Fixed the mistake of "," in the			*/
/*									_fat_check_volume function to ";".		*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2020 Grape Systems, Inc.,  All Rights Reserved.        */
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
#include "fat.h"
#include "grp_fs.h"
#include "grp_time_lib.h"
#include "grp_mem.h"
#ifdef  GRP_FS_MULTI_LANGUAGE
#include "fat_multi_language.h"
#endif  /* GRP_FS_MULTI_LANGUAGE */
#ifdef	GRP_FS_DEBUG
#include "grp_fs_trace.h"
#else
#define grp_fs_ftrace(iDev, uiClst, uiDClst, uiDoff, ptFile, pucComp, iOp)
#endif

/****************************************************************************/
/*  FAT management parameter variables										*/
/****************************************************************************/
int		grp_fat_update_access_time = 1;			/* update access time */
int		grp_fat_update_mod_time = 1;			/* update modification time */
int		grp_fat_sync_hint = 1;					/* sync FAT32 hint */
int		fat_check_cluster_at_open = 1;			/* check cluster at open */

/****************************************************************************/
/*  file system operation table for FAT										*/
/****************************************************************************/
static grp_fs_open_root_t _fat_open_root;	/* return root */
static grp_fs_mount_t	_fat_mount;			/* mount function */
static grp_fs_umount_t	_fat_unmount;		/* unmount function */
static grp_fs_close_t	_fat_close;			/* close function */
static grp_fs_read_t	_fat_read;			/* read function */
static grp_fs_write_t	_fat_write;			/* write function */
static grp_fs_create_t	_fat_create;		/* create function */
static grp_fs_unlink_t	_fat_unlink;		/* unlink function */
static grp_fs_rename_t	_fat_rename;		/* rename function */
static grp_fs_get_attr_t _fat_get_attr;		/* get attribute function */
static grp_fs_set_attr_t _fat_set_attr;		/* set attribute function */
static grp_fs_truncate_t _fat_truncate;		/* truncate function */
static grp_fs_get_dirent_t _fat_get_dirent;	/* get directory entry */
static grp_fs_match_comp_t _fat_match_comp;	/* match component */
static grp_fs_check_volume_t _fat_check_volume;/* check volume */
static grp_fs_sync_t	_fat_sync;			/* FS dependent sync data */

grp_fs_op_t		grp_fs_op_fat = {			/* file system operation table */
	_fat_open_root,							/* return root */
	_fat_mount,								/* mount function */
	_fat_unmount,							/* unmount function */
	grp_fs_file_open_common,				/* open function */
	_fat_close,								/* close function */
	_fat_read,								/* read function */
	_fat_write,								/* write function */
	_fat_create,							/* create function */
	_fat_unlink,							/* unlink function */
	_fat_rename,							/* rename function */
	_fat_get_attr,							/* get attribute */
	_fat_set_attr,							/* set attribute */
	_fat_truncate,							/* truncate function */
	_fat_get_dirent,						/* get directory entry */
	_fat_match_comp,						/* match component */
	_fat_check_volume,						/* check volume */
	_fat_sync,								/* FS dependent sync */
#ifdef  GRP_FS_MULTI_LANGUAGE
	FAT_MULTI_LANG_FUNCTIONS				/* multi language function list */
#endif  /* GRP_FS_MULTI_LANGUAGE */
};

/****************************************************************************/
/*  conversion table from pysical BPB data to internal control data			*/
/****************************************************************************/
static fat_conv_tbl_t _fat_BPB_common_conv[] = {/* BPB conv table for common */
	{  0, 3, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucJmpBoot[0]) },/* jump boot code */
	{  3, 8, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucOEMName[0]) },/* OEM name */
	{ 11, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usBytePerSec) },	/* byte/sec */
	{ 13, 1, FAT_FLD_TYPE_UC, FAT_BFLD_OFF(ucSecPerClst) },	/* sec/cluster */
	{ 14, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usRsvSecCnt), },	/* rsv sec count */
	{ 16, 1, FAT_FLD_TYPE_UC, FAT_BFLD_OFF(ucFatCnt), },	/* FAT count */
	{ 17, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usRootEntCnt) },	/* root ent count */
	{ 19, 2, FAT_FLD_TYPE_UI, FAT_BFLD_OFF(uiTotalSec) },	/* total sec 12/16*/
	{ 21, 1, FAT_FLD_TYPE_UC, FAT_BFLD_OFF(ucMedia) },		/* media type */
	{ 22, 2, FAT_FLD_TYPE_UI, FAT_BFLD_OFF(uiFatSz) },		/* FAT sec count */
	{ 24, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usSecPerTrk) },	/* sec/track */
	{ 26, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usNumHeads) },	/* head count */
	{ 28, 4, FAT_FLD_TYPE_UI, FAT_BFLD_OFF(uiHidenSec) },	/* hidden sec */
	{ 32, 4, FAT_FLD_TYPE_SUI, FAT_BFLD_OFF(uiTotalSec) },	/* total sec 32 */
	{ 510,2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usBPBSig) },		/* BPB sign */
	{ -1,  0 }
};
static fat_conv_tbl_t _fat_BPB12_16_conv[] = {/* BPB conv table for FAT12/16 */
	{ 36, 1, FAT_FLD_TYPE_UC, FAT_BFLD_OFF(ucDrvNum) },		/* drive number */
	{ 37, 1, FAT_FLD_TYPE_NA, 0 },							/* reserved */
	{ 38, 1, FAT_FLD_TYPE_UC, FAT_BFLD_OFF(ucBootSig) },	/* boot sign */
	{ 39, 4, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucVolSer[0]) },	/* volume serial */
	{ 43,11, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucVolLab[0]) },	/* volume label */
	{ 54, 8, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucFsTypeName[0]) },/* type name */
	{ -1,-1, FAT_FLD_TYPE_NA, 0 }
};
static fat_conv_tbl_t _fat_BPB32_conv[] = {	/* BPB conv table for FAT32 */
	{ 36, 4, FAT_FLD_TYPE_SUI, FAT_BFLD_OFF(uiFatSz) },		/* FAT sec count */
	{ 40, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usExtFlags) },	/* ext flags */
	{ 42, 2, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucFsVersion) },	/* FS version */
	{ 44, 4, FAT_FLD_TYPE_UI, FAT_BFLD_OFF(uiRootClst) },	/* root cluster */
	{ 48, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usFsInfo) },		/* FS info sec */
	{ 50, 2, FAT_FLD_TYPE_US, FAT_BFLD_OFF(usBackupBootSec) },/* backup boot */
	{ 52,12, FAT_FLD_TYPE_NA, 0 },							/* reserved */
	{ 64, 1, FAT_FLD_TYPE_UC, FAT_BFLD_OFF(ucDrvNum) },		/* drive number */
	{ 65, 1, FAT_FLD_TYPE_NA, 0 },							/* reserved */
	{ 66, 1, FAT_FLD_TYPE_UC, FAT_BFLD_OFF(ucBootSig) },	/* boot sign */
	{ 67, 4, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucVolSer[0]) },	/* volume serial */
	{ 71,11, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucVolLab[0]) },	/* volume label */
	{ 82, 8, FAT_FLD_TYPE_UA, FAT_BFLD_OFF(aucFsTypeName[0]) },/* type name */
	{ -1,-1, FAT_FLD_TYPE_NA, 0 }
};
static fat_conv_tbl_t _fat_fsinfo_conv[] = { /* FS info conv table */
	{   0,   4, FAT_FLD_TYPE_UI, FAT_IFLD_OFF(uiFiSig1) },	/* FS info sign 1 */
	{   4, 480, FAT_FLD_TYPE_NA, 0 },						/* reserved */
	{ 484,   4, FAT_FLD_TYPE_UI, FAT_IFLD_OFF(uiFiSig2) },	/* FS info sign 2 */
	{ 488,   4, FAT_FLD_TYPE_UI, FAT_IFLD_OFF(uiFiFreeCnt) },/* free count */
	{ 492,   4, FAT_FLD_TYPE_UI, FAT_IFLD_OFF(uiFiNextFree) },/* next free */
	{ 496,  12, FAT_FLD_TYPE_NA, 0 },						/* reserved */
	{ 508,   4, FAT_FLD_TYPE_UI, FAT_IFLD_OFF(uiFiSig3) },	/* FS info sign 3 */
	{  -1,  -1, FAT_FLD_TYPE_NA, 0 }
};

/****************************************************************************/
/*  buffer size for counting free FAT										*/
/****************************************************************************/
grp_int32_t		grp_fs_fat_cnt_buf_sz = FAT_CNT_BUF_SZ;		/* count buf size */

/****************************************************************************/
/*  bit table for checking legal short/long name character					*/
/****************************************************************************/
static	grp_uchar_t _aucFatChar[128];			/* legal characters */
static	int			_iFatCharInitDone = 0;		/* FAT char table init done */

/****************************************************************************/
/*  free list of FAT dependent file inforamtion area						*/
/****************************************************************************/
static	fat_open_info_ctl_t	*grp_fs_fat_open_ctl = NULL; /* open info control */

/****************************************************************************/
/*  hook function to interrupt directory lookup loop					    */
/****************************************************************************/
int		(*fat_interrupt_lookup)(int iDev, grp_uint32_t uiFid,
							grp_uint32_t uiOffset); /* interrupt hook */

/****************************************************************************/
/*  forward prototype declartion											*/
/****************************************************************************/
static void _fat_free_dir_ent(grp_fs_file_t *ptDir, fat_open_info_t *ptOpen);
static int	_fat_free_1st_cluster(grp_fs_file_t	*ptFile);
static int	_fat_free_cluster(grp_fs_info_t *ptFs, grp_fs_bio_t *ptBio,
							  grp_uint32_t uiCluster, grp_uint32_t *puiNext);
static int	_fat_free_cluster_list(grp_fs_file_t *ptFile, grp_uint32_t uiStart,
							  grp_uint32_t uiCnt, grp_uint32_t	uiOffClBlk);
static int	_fat_update_attr(grp_fs_file_t	*ptFile);

/****************************************************************************/
/* FUNCTION:	_fat_init_char_table										*/
/*																			*/
/* DESCRIPTION:	Initialize FAT character table								*/
/* INPUT:		None														*/
/* OUTPUT:		_aucFatChar:	legal character table						*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_init_char_table(void)
{
	int			i;									/* loop index */
	grp_uchar_t	*puc;								/* character pointer */

	/****************************************************/
	/* set valid short/long character					*/
	/****************************************************/
	for (i = 'a'; i <= 'z'; i++)					/* lowercase characters */
		FAT_SET_VALID_SLCHAR(i);					/* valid short/long char */
	for (i = 'A'; i <= 'Z'; i++)					/* lowercase characters */
		FAT_SET_VALID_SLCHAR(i);					/* valid short/long char */
	for (i = '0'; i <= '9'; i++)					/* lowercase characters */
		FAT_SET_VALID_SLCHAR(i);					/* valid short/long char */
	for (puc = (grp_uchar_t *)" .$%'-_@~`!(){}^#&"; *puc; puc++)
		FAT_SET_VALID_SLCHAR(*puc);					/* valid short/long char */

	/****************************************************/
	/* set valid long character only					*/
	/****************************************************/
	for (puc = (grp_uchar_t *)"+,;=[]"; *puc; puc++)
		FAT_SET_VALID_LCHAR(*puc);					/* valid only long */

	_iFatCharInitDone = 1;							/* initalization end */
}

/****************************************************************************/
/* FUNCTION:	_fat_init_free_open_info									*/
/*																			*/
/* DESCRIPTION:	Allocate free open information area							*/
/* INPUT:		None														*/
/* OUTPUT:		_ptFatOpenFree:	free list of open info area					*/
/*																			*/
/* RESULT:		-1:					failed to allocate						*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_init_free_open_info(void)
{
	fat_open_info_t	*ptOpen;						/* open file info */
	fat_open_info_ctl_t *ptCtl;						/* open info control */
	grp_uint32_t	uiIdx;							/* index */
	grp_uint32_t	uiCnt;							/* open file info count */

	/****************************************************/
	/* allocate FAT dependent open information and,		*/
	/* size 0 file table area							*/
	/****************************************************/
	uiCnt = grp_fs_param.uiFileCnt + grp_fs_param.sTasks;/* count to allocate */
	ptCtl = grp_mem_alloc((grp_isize_t)(sizeof(fat_open_info_t) * uiCnt
						+ sizeof(fat_open_info_ctl_t))); /* allocate area */
	if (ptCtl == NULL)								/* failed to allocate */
		return(-1);									/* return error */

	/****************************************************/
	/* initialize open inforamtion area					*/
	/****************************************************/
	ptOpen = (fat_open_info_t *)&ptCtl[1];			/* open info */
	ptCtl->ptOpenInfo = ptOpen;						/* set top of area */
	ptCtl->ptOpenFree = ptOpen;						/* set free list */
	ptCtl->ptSize0List = NULL;						/* clear size 0 list */
	for (uiIdx = 0; uiIdx < uiCnt; ptOpen++, uiIdx++) {
		ptOpen->uiUniqFid = FAT_IDX_TO_UNIQ_FID(uiIdx); /* set unique file id */
		ptOpen->iDev = -1;							/* invalid device */
		ptOpen->ptSz0Fwd = &ptOpen[1];				/* link to next */
	}
	ptOpen[-1].ptSz0Fwd = NULL;						/* end of free list */
	grp_fs_fat_open_ctl = ptCtl;					/* set control pointer */
	return(0);										/* return success */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_fat_init												*/
/*																			*/
/* DESCRIPTION:	Initialize FAT management									*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_NOMEM:	failed to allocate						*/
/*				0:					success to initialize					*/
/*																			*/
/****************************************************************************/
int
grp_fs_fat_init(void)
{
	/****************************************************/
	/* init FAT character table							*/
	/****************************************************/
	if (_iFatCharInitDone == 0) 		/* FAT char table is not initialized */
		_fat_init_char_table();			/* initialize FAT char table */

	/****************************************************/
	/* allocate and initialize FAT open info area		*/
	/****************************************************/
	if (grp_fs_fat_open_ctl == NULL) {	/* not allocated yet */
		if (_fat_init_free_open_info() < 0) /* failed to allocate */
			return(GRP_FS_ERR_NOMEM);	/* return error */
	}
	return(0);							/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_conv_data												*/
/*																			*/
/* DESCRIPTION:	Convert data 												*/
/* INPUT:		pucData: 	data to be converted							*/
/*				ptConvInfo:	conversion information							*/
/* OUTPUT:		pvSruct:	top address of a structure						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_INVALID:	invalid data							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_conv_data(
	grp_uchar_t		*pucData,			/* [IN]  data to be converted */
	fat_conv_tbl_t	*ptConvInfo,		/* [IN]  conversion information */
	void			*pvStruct)			/* [OUT] top address of a structure */
{
	grp_uchar_t		*pucSrc;			/* source data address */
	grp_uchar_t		*pucDst;			/* destination field address */
	grp_uchar_t		*pucSrcBase;		/* base of source data address */
	grp_uint32_t	uiValue;			/* int value */
	grp_uint32_t	uiPrevValue;		/* previous value */
	int				iType;				/* type */
	int				iBytes;				/* bytes */

	/****************************************************/
	/* copy data if byte type (UC, UA, NA) data			*/
	/****************************************************/
	pucSrc = pucData + ptConvInfo->sPhysOff;		/* source address */
	pucDst = (grp_uchar_t *)pvStruct + ptConvInfo->ucFldOff; /* dest addr */
	iBytes = ptConvInfo->sPhysSize;					/* conversion size */
	iType = FAT_FLD_TYPE(ptConvInfo);				/* field type */
	if (FAT_FLD_TYPE_IS_BYTE(iType)) {				/* byte type data */
		if (iType != FAT_FLD_TYPE_NA) {				/* UC or UA */
			while (iBytes-- > 0)					/* loop by size */
				*pucDst++ = *pucSrc++;				/* copy data */
		}
		return(0);									/* return success */
	}

	/****************************************************/
	/* convert to integer for int (US, UI) type data	*/
	/****************************************************/
	uiValue = 0;									/* reset value */
	pucSrcBase = pucSrc;							/* set base address */
	for (pucSrc += iBytes - 1; pucSrc >= pucSrcBase; pucSrc--)
		uiValue = (uiValue << 8) + *pucSrc;			/* convert to int */
	if (FAT_FLD_TYPE_IS_SHR(ptConvInfo)) {			/* shared field */
		if (iType == FAT_FLD_TYPE_UI)				/* grp_uint32_t type */
			uiPrevValue = *(grp_uint32_t *)pucDst;	/* get previous value */
		else										/* grp_ushort_t type */
			uiPrevValue = *(grp_ushort_t *)pucDst;	/* get previous value */
		if (uiPrevValue != 0) {						/* non 0 previous value */
			if (uiValue != 0)						/* current is non 0 too */
				return(GRP_FS_ERR_FS);				/* return error */
			else									/* current is 0 */
				return(0);							/* use previous one */
		}
	}
	if (iType == FAT_FLD_TYPE_UI)					/* grp_uint32 type */
		*(grp_uint32_t *)pucDst = uiValue;			/* set by grp_uint32_t */
	else											/* grp_ushort_t type */
		*(grp_ushort_t *)pucDst = (grp_ushort_t)uiValue;
													/* set by grp_ushort_t */
	return(0);										/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_flds												*/
/*																			*/
/* DESCRIPTION:	Convert and get field data									*/
/* INPUT:		pucData: 	data to be converted							*/
/*				ptConvInfo:	conversion information							*/
/* OUTPUT:		pvStruct:	top address of a structure						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_INVALID:	invalid data							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_get_flds(
	grp_uchar_t		*pucData,			/* [IN]  data to be converted */
	fat_conv_tbl_t	*ptConvInfo,		/* [IN]  conversion information */
	void			*pvStruct)			/* [OUT] top address of a structure */
{
	int				iRet;				/* return value */

	for ( ; ptConvInfo->sPhysOff >= 0; ptConvInfo++) {		/* loop until end */
		iRet = _fat_conv_data(pucData, ptConvInfo, pvStruct);/* conv data */
		if (iRet != 0)					/* error detected */
			return(iRet);				/* return error */
	}
	return(0);							/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_flds												*/
/*																			*/
/* DESCRIPTION:	Make write back data for the fields							*/
/* INPUT:		pvStruct:	top address of a struct to be converted			*/
/*				ptConvInfo:	conversion information							*/
/* OUTPUT:		pucData:	data area to be written back					*/
/*																			*/
/* RESULT:		none														*/
/*																			*/
/****************************************************************************/
static void
_fat_set_flds(
	grp_uchar_t		*pucData,			/* [OUT] data area to be written back */
	fat_conv_tbl_t	*ptConvInfo,		/* [IN]  conversion information */
	void			*pvStruct)			/* [IN]  struct addr to be converted */
{
	grp_uint32_t	uiVal;				/* int value */
	grp_uchar_t		*pucSrc;			/* source address */
	grp_uchar_t		*pucDst;			/* destination address */
	int				i;					/* loop count */

	for ( ; ptConvInfo->sPhysOff >= 0; ptConvInfo++) {	/* loop until end */
		pucSrc = (grp_uchar_t *)pvStruct + ptConvInfo->ucFldOff; /* src */
		pucDst = pucData + ptConvInfo->sPhysOff;		/* dst */
		switch(FAT_FLD_TYPE(ptConvInfo)) {				/* switch by type */
		case FAT_FLD_TYPE_NA:							/* no data */
			continue;									/* skip it */
		case FAT_FLD_TYPE_UC:							/* character */
		case FAT_FLD_TYPE_UA:							/* array of char */
			memcpy(pucDst, pucSrc, (grp_size_t)ptConvInfo->sPhysSize);
														/* copy data */
			continue;									/* proc next */
		case FAT_FLD_TYPE_US:							/* short value */
			uiVal = *((grp_ushort_t *)pucSrc);			/* get value */
			break;
		case FAT_FLD_TYPE_UI:							/* int32 value */
			uiVal = *((grp_uint32_t *)pucSrc);			/* get value */
			break;
		default:										/* others */
			continue;
		}
		for (i = 0; i < ptConvInfo->sPhysSize; i++) {	/* loop by byte */
			*pucDst++ = (grp_uchar_t)(uiVal & 0xff);	/* set data */
			uiVal >>= 8;								/* shift 1 byte */
		}
	}
}

/****************************************************************************/
/* FUNCTION:	_fat_count_shift											*/
/*																			*/
/* DESCRIPTION:	Compute shift count of the value							*/
/* INPUT:		uiValue:	value to compute shift value					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:				not 2's power								*/
/*				non negative:	shift count									*/
/*																			*/
/****************************************************************************/
static int
_fat_count_shift(
	grp_uint32_t	uiValue)			/* value to compute shift count */
{
	int			iShift;					/* shift count */

	if ((uiValue & (uiValue - 1)) || uiValue == 0)	/* not 2's power */
		return(-1);									/* return error */
	for (iShift = 0; (uiValue & 1) == 0; iShift++)	/* loop until bit 1 */
		uiValue >>= 1;								/* shift 1 bit */
	return(iShift);
}

/****************************************************************************/
/* FUNCTION:	_fat_time_to_canon											*/
/*																			*/
/* DESCRIPTION:	Convert time information to canonical form					*/
/* INPUT:		pucDate:			date information						*/
/*				pucTime:			time in a day information				*/
/*				iSec1:				1 sec information						*/
/* OUTPUT:		piTimeSec:			total seconds from base time			*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_time_to_canon(
	grp_int32_t		*piTimeSec,				/* [OUT] total seconds from base */
	grp_uchar_t		*pucDate,				/* [IN]  date information  */
	grp_uchar_t		*pucTime,				/* [IN]  time in a day */
	int				iSec1)					/* [IN]  1 sec */
{
	grp_int32_t		iSec;					/* second */
	grp_int32_t		iDate;					/* date */
	grp_time_tm_t	tTM;					/* time structure */

	if (pucTime)							/* time in a day exists */
		iSec = ((pucTime[0] + ((grp_int32_t)pucTime[1] << 8)) << 1) + iSec1;
	else									/* no time in a day */
		iSec = 0;							/* clear with 0 */
	tTM.ucSec = (grp_uchar_t)(iSec & 0x0003f);/* second */
	tTM.ucMin = (grp_uchar_t)((iSec >> 6) & 0x0003f);/* minute */
	tTM.ucHour = (grp_uchar_t)((iSec >> 12) & 0x0001f);/* hour */
	iDate = pucDate[0] + ((grp_int32_t)pucDate[1] << 8);/* date */
	tTM.ucDay = (grp_uchar_t)(iDate & 0x001f);/* day */
	tTM.ucMon = (grp_uchar_t)((iDate >> 5) & 0x000f);/* month */
	tTM.sYear = (short)(((iDate >> 9) & 0x007f) + 1980);/* year */
	iSec = grp_time_mktime(&tTM);			/* convert to canon */
	if (iSec != -1)							/* not error */
		*piTimeSec = iSec;					/* set time */
	else
		*piTimeSec = 0;						/* set time 0 */
}

/****************************************************************************/
/* FUNCTION:	_fat_update_time											*/
/*																			*/
/* DESCRIPTION:	Convert time information to canonical form					*/
/* INPUT:		iTimeSec:			total seconds from base time			*/
/*				ptFs:				file system information					*/
/* OUTPUT:		pucDate:			date information						*/
/*				pucTime:			time in a day information				*/
/*				pucSec:				sub-sec information						*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_update_time(
	grp_uchar_t		*pucDate,				/* [OUT] date information  */
	grp_uchar_t		*pucTime,				/* [OUT] time in a day */
	grp_uchar_t		*pucSec,				/* [OUT] sub-sec information */
	grp_int32_t		iTimeSec,				/* [IN]  total seconds from base */
	grp_fs_info_t	*ptFs)					/* [IN]  file system information */
{
	grp_ushort_t	usFatDate;				/* FAT date information */
	grp_ushort_t	usFatTime;				/* FAT time information */
	grp_time_tm_t	tTM;					/* time information */

	if (ptFs && (ptFs->usStatus & GRP_FS_STAT_NO_CRT_ACCTIME)) {
		pucDate[0] = pucDate[1] = 0;			/* no date information */
		if (pucTime) {							/* neet to set time info */
			pucTime[0] = pucTime[1] = 0;		/* no time information */
			if (pucSec)							/* need to set sub-sec */
				*pucSec = 0;					/* no sub-sec information */
		}
		return;									/* return */
	}
	grp_time_localtime(iTimeSec, &tTM);			/* convert information */
	usFatDate = (grp_ushort_t)((((tTM.sYear - 1980) << 9) & 0xfe00)/* year */
				| ((tTM.ucMon << 5) & 0x01e0)	/* month */
				| (tTM.ucDay & 0x1f));			/* day */
	pucDate[0] = (grp_uchar_t)(usFatDate & 0xff);/* low date byte */
	pucDate[1] = (grp_uchar_t)((usFatDate >> 8) & 0xff);
												/* high date byte */
	if (pucTime == NULL)						/* no time info */
		return;									/* return here */
	usFatTime = (grp_ushort_t)(((tTM.ucHour << 11) & 0xf800)/* hour */
				| ((tTM.ucMin << 5) & 0x07e0)	/* minute */
				| ((tTM.ucSec >> 1) & 0x001f));	/* sec / 2 */
	pucTime[0] = (grp_uchar_t)(usFatTime & 0xff);/* low time byte */
	pucTime[1] = (grp_uchar_t)((usFatTime >> 8) & 0xff);/* high time byte */
	if (pucSec)									/* need to set sub-sec */
		*pucSec = (grp_uchar_t)((tTM.ucSec & 1) * 100);/* set 1 sec in sub-sec */
}

/****************************************************************************/
/* FUNCTION:	_fat_clean_uref_buf											*/
/*																			*/
/* DESCRIPTION:	Release buffer with write if necessary						*/
/* INPUT:		ptBio:				buffer I/O information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ER_IO:		I/O error								*/
/*				0:					success									*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
static int
_fat_clean_unref_buf(
	grp_fs_bio_t	*ptBio)					/* [IN]  buffer I/O information */
{
	int				iRet = 0;				/* return value */
	grp_ushort_t	usModStat;				/* modification status */
	grp_fs_buf_t	*ptBuf;					/* buffer pointer */

	if (ptBio->ptBuf) {						/* buffer exists */
		ptBuf = ptBio->ptBuf;				/* buffer pointer */
		usModStat = (grp_ushort_t)ptBuf->usStatus; /* buffer status */
		if ((usModStat & GRP_FS_BSTAT_DIRTY) /* dirty buffer */
			&& (usModStat & (GRP_FS_BSTAT_SYNC|GRP_FS_BSTAT_TSYNC))) {
			/****************************************************/
			/* write back sync write data						*/
			/****************************************************/
			grp_int32_t	iWrite;				/* written size */
			iWrite = grp_fs_write_buf(ptBio);	/* write it back */
			if (iWrite == (grp_int32_t)ptBio->uiSize) { /* write success */
				iRet = 0;					/* set success return */
			} else {						/* write failed */
				grp_fs_printf("FAT: write failed on block 0x%lx(dev 0x%x)\n",
							(unsigned long)ptBio->uiBlk, ptBio->ptBuf->iDev);
				if (iWrite >= 0)			/* not error number */
					iRet = GRP_FS_ERR_IO;	/* set error number */
				else						/* error number */
					iRet = (int)iWrite;		/* set error number */
			}
		}
		grp_fs_unref_buf(ptBio);			/* release buffer */
	}
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_blk												*/
/*																			*/
/* DESCRIPTION:	Get a block in buffer										*/
/* INPUT:		ptFs:				file system information					*/
/*				uiBlk:				block number							*/
/*				iBufKind:			buffer kind								*/
/*				ptBio:				buffer I/O information					*/
/* OUTPUT:		ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		GRP_FS_ER_FS:		bad file system							*/
/*				GRP_FS_ER_IO:		I/O error								*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_get_blk(
	grp_fs_info_t	*ptFs,					/* [IN]  file system information */
	grp_uint32_t	uiBlk,					/* [IN]  block number */
	int				iBufKind,				/* [IN]  buffer kind */
	grp_fs_bio_t	*ptBio)					/* [IN/OUT] buffer I/O info */
{
	fat_BPB_t		*ptBPB;					/* BPB information */
	grp_int32_t		iSize;					/* size to read */
	grp_int32_t		iRemain;				/* remain size */
	grp_int32_t		iRead;					/* read size */
	int				iRet;					/* return value */

	if (ptBio->ptBuf != NULL) {				/* buffered data exists */
		if (ptBio->uiBlk == uiBlk && ptBio->uiSize != 0)	/* in buffer */
			return(0);						/* return success */
		iRet = _fat_clean_unref_buf(ptBio);	/* release buf */
		if (iRet != 0)						/* error occured */
				return(iRet);				/* return error */
	}
	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	if (iBufKind == GRP_FS_BUF_FILE) {		/* FAT block */
		iSize = ptBPB->uiFBlkSize;			/* size to read */
		iRemain = ptFs->uiFsDBlkOff - (uiBlk << ptBPB->iFBlkShift);
											/* remain in FAT block */
		if (iRemain <= 0)					/* no data */
			return(GRP_FS_ERR_FS);			/* return error */
		if (iRemain < iSize)				/* remain is less than block */
			iSize = iRemain;				/* use the remain size */
	} else {								/* data block */
		iSize = ptBPB->uiDBlkSize;			/* size to read */
	}
	iRead = grp_fs_read_buf(ptFs, uiBlk, iBufKind, iSize, ptBio);/* read data */
	if (iRead != iSize)						/* read error */
		return((int)((iRead < 0)? iRead: GRP_FS_ERR_FS)); /* return error */
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_fat												*/
/*																			*/
/* DESCRIPTION:	Get FAT data												*/
/* INPUT:		ptFs:	FS information										*/
/*				uiCluster:	cluster number									*/
/*				ptBio:		buffer I/O information							*/
/* OUTPUT:		puiFat:		FAT value										*/
/*																			*/
/* RESULT:		GRP_FS_ERR_INVALID:	invalid data							*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_get_fat(
	grp_fs_info_t	*ptFs,			/* [IN]  FS information */
	grp_uint32_t	uiCluster,		/* [IN]  cluster number */
	grp_fs_bio_t	*ptBio,			/* [IN]  buffer I/O information */
	grp_uint32_t	*puiFat)		/* [OUT] FAT value */
{
	grp_int32_t		iBlkOff;		/* block offset */
	fat_BPB_t		*ptBPB;			/* BPB information */
	grp_uint32_t	uiOff;			/* offset in FAT area */
	grp_uint32_t	uiBlk;			/* block number */
	grp_uint32_t	uiFatVal;		/* FAT value */
	grp_uchar_t		*pucFat;		/* FAT pointer */
	grp_uchar_t		*pucStFat;		/* start of FAT */
	grp_fs_bio_t	tFsBio;			/* buffer I/O information */
	int				iRet = 0;		/* return value */

	/****************************************************/
	/* read FAT data if not buffered					*/
	/****************************************************/
	if (ptBio == NULL) {							/* no buffer I/O info */
		ptBio = &tFsBio;							/* use local one */
		ptBio->ptBuf = NULL;						/* no buffer */
	}
	ptBPB = ptFs->pvFsInfo;							/* BPB information */
	if (uiCluster == 0 || uiCluster >= ptBPB->uiMaxClst) {
													/* bad cluster number */
		iRet = GRP_FS_ERR_FS;						/* set error number */
		goto err_out;								/* return error */
	}
	uiOff = FAT_OFF(ptBPB, uiCluster);				/* FAT offset */
	uiBlk = FAT_FBLK(ptBPB, uiOff);					/* block number */
	iRet = _fat_get_blk(ptFs, uiBlk, GRP_FS_BUF_FILE, ptBio);/* get FATs */
	if (iRet != 0)									/* error occured */
		goto err_out;								/* return error */

	/****************************************************/
	/* get FAT data										*/
	/****************************************************/
	iBlkOff = FAT_FBLK_OFF(ptBPB, uiOff);			/* block offset */
	pucStFat = &ptBio->pucData[iBlkOff];			/* start of FAT */
	if (ptBPB->iFsType != FAT_TYPE_12) {				/* FAT16/32 */
		pucFat = &pucStFat[(ptBPB->iFsType == FAT_TYPE_32)? 3: 1]; /* end FAT */
		uiFatVal = 0;								/* set initial value */
		while (pucFat >= pucStFat)					/* loop by FAT size */
			uiFatVal = (uiFatVal << 8) + *pucFat--;	/* compute value */
		if ((ptBPB->iFsType == FAT_TYPE_32)			/* FAT32 */
			&& (2 <= uiCluster))					/* cluster more than 2 */
			uiFatVal &= 0x0fffffff;					/* mask reserve 4bits */
	} else {
		if (iBlkOff + 1 < (grp_int32_t)ptBio->uiSize) {	/* in buffer */
			uiFatVal = pucStFat[0] + ((grp_uint32_t)pucStFat[1] << 8);
													/* compute value */
		} else {
			uiFatVal = pucStFat[0];					/* get low byte */
			iRet = _fat_get_blk(ptFs, uiBlk + 1, GRP_FS_BUF_FILE, ptBio);
													/* get next FATs */
			if (iRet != 0)							/* read error */
				goto err_out;						/* return error */
			uiFatVal += ((grp_uint32_t)ptBio->pucData[0] << 8);
													/* get high byte */
		}
		uiFatVal = (uiCluster & 1)? (uiFatVal >> 4): (uiFatVal & 0x0fff);
	}
	if (ptBio == &tFsBio && ptBio->ptBuf)			/* use local one */
		grp_fs_unref_buf(ptBio);					/* release buffer */
	*puiFat = uiFatVal;								/* return FAT value */
	return(0);

err_out:
	if (ptBio->ptBuf)								/* buffer exists */
		grp_fs_unref_buf(ptBio);					/* release buffer */
	*puiFat = FAT_EOF_CLST;							/* return EOF FAT */
	return(iRet);									/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_fatn												*/
/*																			*/
/* DESCRIPTION:	Set FAT data												*/
/* INPUT:		ptFs:		FS information									*/
/*				uiCluster:	cluster number									*/
/*				ptBio:		buffer I/O information							*/
/* 				puiFatVal:	new FAT value									*/
/*				iFatNum:	FAT number										*/
/* OUTPUT:		puiFatVal:	previous FAT value								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_INVALID:	invalid data							*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_set_fatn(
	grp_fs_info_t	*ptFs,			/* [IN]  FS information */
	grp_uint32_t	uiCluster,		/* [IN]  cluster number */
	grp_fs_bio_t	*ptBio,			/* [IN]  buffer I/O information */
	grp_uint32_t	*puiFatVal,		/* [IN/OUT]  FAT value */
	int				iFatNum)		/* [IN]  FAT number */
{
	grp_int32_t		iBlkOff;		/* block offset */
	fat_BPB_t		*ptBPB;			/* BPB information */
	grp_uint32_t	uiFatVal;		/* new FAT value */
	grp_uint32_t	uiPrevVal;		/* previous value */
	grp_uint32_t	uiOff;			/* offset in FAT area */
	grp_uint32_t	uiBlk;			/* FAT block */
	grp_uchar_t		*pucFat;		/* FAT pointer */
	grp_uchar_t		*pucStFat;		/* start of FAT */
	grp_uchar_t		*pucEndFat;		/* end of FAT */
	grp_ushort_t	usModStat;		/* modification status */
	int				iRet = 0;		/* return value */

	/****************************************************/
	/* read FAT data if not buffered					*/
	/****************************************************/
	ptBPB = ptFs->pvFsInfo;							/* BPB information */
	if (uiCluster == 0 || uiCluster >= ptBPB->uiMaxClst) {
													/* bad cluster number */
		iRet = GRP_FS_ERR_FS;						/* set error number */
		goto err_out;								/* return error */
	}
	uiOff = FATN_OFF(ptBPB, uiCluster, iFatNum);	/* FAT offset */
	uiBlk = FAT_FBLK(ptBPB, uiOff);					/* block number */
	iRet = _fat_get_blk(ptFs, uiBlk, GRP_FS_BUF_FILE, ptBio);/* get FATs */
	if (iRet != 0)									/* error occured */
		goto err_out;								/* return error */

	/****************************************************/
	/* get previous FAT value and set new FAT value		*/
	/****************************************************/
	iBlkOff = FAT_FBLK_OFF(ptBPB, uiOff);			/* block offset */
	pucStFat = &ptBio->pucData[iBlkOff];			/* start of FAT */
	uiFatVal = *puiFatVal;							/* get new FAT value */
	grp_fs_block_buf_mod(ptBio);					/* block modify */
	usModStat = (grp_ushort_t)((uiCluster != 1)?
					GRP_FS_BSTAT_DIRTY:
					(GRP_FS_BSTAT_DIRTY|GRP_FS_BSTAT_TSYNC));/* sync for FAT1 */
	if (ptBPB->iFsType != FAT_TYPE_12) {			/* FAT16/32 */
		pucEndFat = &pucStFat[(ptBPB->iFsType == FAT_TYPE_32)? 3: 1];
													/* end FAT */
		uiPrevVal = 0;								/* set initial value */
		for (pucFat = pucEndFat; pucFat >= pucStFat; )
			uiPrevVal = (uiPrevVal << 8) + *pucFat--;/* get previous value */
		if ((ptBPB->iFsType == FAT_TYPE_32)			/* FAT32 */
			&& (2 <= uiCluster)){					/* cluster more than 2 */
			grp_uint32_t uiWorkVal = uiPrevVal;
			uiFatVal  &= 0x0fffffff;				/* mask reserve 4 bits */
			uiWorkVal &= 0xf0000000;				/* mask fat 28 bits */
			uiFatVal  |= uiWorkVal;					/* merge reserve 4bits */
			uiPrevVal &= 0x0fffffff;				/* mask reserve 4bits */
		}
		for (pucFat = pucStFat; pucFat <= pucEndFat; ) {
			*pucFat++ = (grp_uchar_t)(uiFatVal & 0xff);/* set new value */
			uiFatVal >>= 8;							/* shift 1 byte */
		}
	} else {
		pucFat = pucStFat;							/* set pointer */
		uiFatVal = (uiCluster & 1)? (uiFatVal << 4): (uiFatVal & 0x0fff);
		if (iBlkOff + 1 < (grp_int32_t)ptBio->uiSize) {	/* in buffer */
			uiPrevVal = pucFat[0] + ((grp_uint32_t)pucFat[1] << 8);
													/* get previous value */
			uiFatVal |= (uiPrevVal & ((uiCluster & 1)? 0x000f: 0xf000));
			pucFat[0] = (grp_uchar_t)(uiFatVal & 0xff);/* set 1st byte */
			pucFat[1] = (grp_uchar_t)(uiFatVal >> 8);/* set 2nd byte */
		} else {
			uiPrevVal = *pucFat;					/* get previous value */
			if (uiCluster & 1)						/* odd cluster */
				uiFatVal |= (uiPrevVal & 0x000f);	/* add low 4 bits */
			*pucFat = (grp_uchar_t)(uiFatVal & 0xff);/* set 1st byte */
			grp_fs_unblock_buf_mod(ptBio, usModStat);/* unblock modify */
			iRet = _fat_get_blk(ptFs, uiBlk + 1, GRP_FS_BUF_FILE, ptBio);
													/* write and get next FAT */
			if (iRet != 0)							/* write or read error */
				goto err_out;						/* return error */
			grp_fs_block_buf_mod(ptBio);			/* block modify */
			pucFat = ptBio->pucData;				/* data buffer */
			uiPrevVal |= ((grp_uint32_t)*pucFat << 8);	/* get previous value */
			if ((uiCluster & 1) == 0)				/* even cluster */
				uiFatVal |= (uiPrevVal & 0xf000);	/* add upper 4 bits */
			*pucFat = (grp_uchar_t)(uiFatVal >> 8);	/* set 2nd byte */
		}
		uiPrevVal = (uiCluster & 1)? (uiPrevVal >> 4): (uiPrevVal & 0x0fff);
	}
	grp_fs_unblock_buf_mod(ptBio, usModStat);		/* unblock modify */
	*puiFatVal = uiPrevVal;							/* set previous value */
	return(0);

err_out:
	if (ptBio->ptBuf)								/* buffer exists */
		grp_fs_unref_buf(ptBio);					/* release buffer */
	return(iRet);									/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_fat												*/
/*																			*/
/* DESCRIPTION:	Set FAT data												*/
/* INPUT:		ptFs:		FS information									*/
/*				uiCluster:	cluster number									*/
/* 				puiFatVal:	new FAT value									*/
/*				ptBio:		buffer I/O information							*/
/* OUTPUT:		puiFatVal:	previous FAT value								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_INVALID:	invalid data							*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_set_fat(
	grp_fs_info_t	*ptFs,			/* [IN]  FS information */
	grp_uint32_t	uiCluster,		/* [IN]  cluster number */
	grp_fs_bio_t	*ptBio,			/* [IN]  buffer I/O information */
	grp_uint32_t	*puiFatVal)		/* [IN/OUT]  new/old FAT value */
{
	fat_BPB_t		*ptBPB;			/* BPB information */
	int				iFatNum;		/* FAT number */
	int				iRet = 0;		/* return value */
	grp_uint32_t	uiFatVal;		/* FAT value */
	grp_fs_bio_t	tFsBio;			/* buffer I/O information */

	/****************************************************/
	/* set private buffer I/O information if necessary	*/
	/****************************************************/
	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	if (ptBio == NULL) {					/* no buffer I/O info */
		ptBio = &tFsBio;					/* use local one */
		ptBio->ptBuf = NULL;				/* no buffer */
	}

	/****************************************************/
	/* get previous FAT value and set new FAT value		*/
	/****************************************************/
	if (ptBPB->iFsType == FAT_TYPE_32 && FAT_NOT_MIRRORED(ptBPB)) {/* single */
		/****************************************************/
		/* if not mirrored, set only active one				*/
		/****************************************************/
		iRet = _fat_set_fatn(ptFs, uiCluster, ptBio, puiFatVal,
						 (int)FAT_ACTIVE_NUM(ptBPB));/* update FAT */
	} else {								/* mirrored */
		/****************************************************/
		/* otherwise, update all FATs						*/
		/****************************************************/
		uiFatVal = *puiFatVal;				/* set new FAT value */
		for (iFatNum = 0; iFatNum < ptBPB->ucFatCnt; iFatNum++) {
			*puiFatVal = uiFatVal;			/* set new FAT vlaue */
			iRet = _fat_set_fatn(ptFs, uiCluster, ptBio, puiFatVal,
								 iFatNum);	/* update FAT n */
			if (iRet != 0)					/* error occured */
				break;						/* break */
		}
	}

	/****************************************************/
	/* write back updated data if necessary				*/
	/****************************************************/
	if (ptBio == &tFsBio) {					/* use local buffer */
		int  iSubRet;
		iSubRet =  _fat_clean_unref_buf(ptBio);	/* clean release buffer */
		if (iRet == 0)						/* success case */
			iRet = iSubRet;					/* set return from clean buf */
	}
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_fat_proc_fat1												*/
/*																			*/
/* DESCRIPTION:	Process FAT1 data											*/
/* INPUT:		ptFs:		FS information									*/
/*				ptBio:		buffer I/O information							*/
/*				iMode:		processing mode									*/
/*								FAT_FAT1_MOUNT:								*/
/*								  check FAT1 and clear clean shutdown bit	*/
/*								FAT_FAT1_UMOUNT:							*/
/*								  set clean shutdown bit					*/
/* OUTPUT:		pvSruct:	top address of a structure						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_INVALID:	invalid data							*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_NEED_CHECK:	need check								*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_proc_fat1(
	grp_fs_info_t	*ptFs,			/* [IN] FS information */
	grp_fs_bio_t	*ptBio,			/* [IN] buffer I/O information */
	int				iMode)			/* [IN] processing mode */
{
	fat_BPB_t		*ptBPB;			/* BPB information */
	grp_uint32_t	uiFatVal;		/* FAT1 value */
	int				iRet;			/* return value */

	ptBPB = ptFs->pvFsInfo;								/* BPB info */
	iRet = _fat_get_fat(ptFs, 1, ptBio, &uiFatVal);		/* get FAT1 */
	if (iRet != 0)										/* error */
		return(iRet);									/* return error */
	if (iMode == FAT_FAT1_MOUNT) {						/* mount mode */
		if (ptBPB->iFsType == FAT_TYPE_32) {			/* FAT 32 */
			if ((uiFatVal & FAT_FAT1_MASK32)
				!= (FAT_EOC_32 & FAT_FAT1_MASK32))		/* invalid FAT1 */
				return(GRP_FS_ERR_FS);					/* return invalid FS */
			if ((uiFatVal & FAT_FAT1_CLEAN32) == 0		/* not clean */
				|| (uiFatVal & FAT_FAT1_NOERR32) == 0)	/* I/O error */
				return(GRP_FS_ERR_NEED_CHECK);			/* return check */
			uiFatVal &= ~FAT_FAT1_CLEAN32;				/* clear clean */
		} else {										/* FAT 12/16 */
			if ((uiFatVal & FAT_FAT1_MASK16)
				!= (FAT_EOC_16 & FAT_FAT1_MASK16))		/* invalid FAT1 */
				return(GRP_FS_ERR_FS);					/* return invalid FS */
			if ((uiFatVal & FAT_FAT1_CLEAN16) == 0		/* not clean */
				|| (uiFatVal & FAT_FAT1_NOERR16) == 0)	/* I/O error */
				return(GRP_FS_ERR_NEED_CHECK);			/* return check */
			uiFatVal &= ~FAT_FAT1_CLEAN16;				/* clear clean */
		}
	} else {											/* umount mode */
		uiFatVal = (ptBPB->iFsType == FAT_TYPE_32)?
				(((ptBPB->uiStatus & FAT_STAT_IO_ERR)? 0: FAT_FAT1_NOERR32)
				 |FAT_FAT1_CLEAN32|FAT_EOC_32):			/* FAT1 for FAT32 */
				(((ptBPB->uiStatus & FAT_STAT_IO_ERR)? 0: FAT_FAT1_NOERR16)
				 |FAT_FAT1_CLEAN16|FAT_EOC_16);			/* FAT1 for FAT16 */
	}
	iRet = _fat_set_fat(ptFs, 1, ptBio, &uiFatVal);		/* set FAT1 */
	if (iRet == 0)										/* no error */
		iRet = _fat_clean_unref_buf(ptBio);				/* write back */
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_fat_alloc_open_info										*/
/*																			*/
/* DESCRIPTION:	Allocate open information									*/
/* INPUT:		iDev:				device number							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		pointer to allocated open file information structure		*/
/*																			*/
/****************************************************************************/
static fat_open_info_t *
_fat_alloc_open_info(
	int				iDev)					/* [IN]  device number */
{
	fat_open_info_ctl_t *ptCtl;				/* open info control */
	fat_open_info_t	*ptOpen;				/* open information pointer */

	ptCtl = grp_fs_fat_open_ctl;			/* open info control */
	if (ptCtl->ptOpenFree == NULL)			/* no free open info buffer */
		return(NULL);						/* return NULL */
	ptOpen = ptCtl->ptOpenFree;				/* get top entry */
	ptCtl->ptOpenFree = ptOpen->ptSz0Fwd;	/* unlink from free list */
	ptOpen->iDev = iDev;					/* set device number */
	ptOpen->ptSz0Fwd = ptOpen->ptSz0Bwd = NULL; /* clear size 0 list */
#ifdef	GRP_FS_FNAME_CACHE
	ptOpen->ptFnCache = NULL;				/* no file name cache */
#endif	/* GRP_FS_FNAME_CACHE */
	return(ptOpen);							/* return allocated */
}

/****************************************************************************/
/* FUNCTION:	_fat_free_open_info											*/
/*																			*/
/* DESCRIPTION:	Free open information										*/
/* INPUT:		ptOpen:				open information area to free			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_free_open_info(
	fat_open_info_t	*ptOpen)				/* [IN]  open infortion area */
{
	fat_open_info_ctl_t *ptCtl;				/* open info control */

#ifdef	GRP_FS_FNAME_CACHE
	grp_fs_fname_cache_t *ptCache;			/* file name cache */
	ptCache = ptOpen->ptFnCache;			/* file name cache */
	if (ptCache								/* exists file name cache */
		&& ptCache->uiFid == ptOpen->uiUniqFid)	/* assocated with this */
		grp_fs_purge_fname_cache_entry(ptCache);/* purge file name cache */
	ptOpen->ptFnCache = NULL;				/* clear association */
#endif	/* GRP_FS_FNAME_CACHE */
	ptCtl = grp_fs_fat_open_ctl;			/* open info control */
	ptOpen->iDev = -1;						/* invalidate device */
	ptOpen->ptSz0Fwd = ptCtl->ptOpenFree;	/* insert to free list */
	ptCtl->ptOpenFree = ptOpen;				/* chain it */
}

#ifdef	GRP_FS_FNAME_CACHE
/****************************************************************************/
/* FUNCTION:	_fat_update_fname_cache										*/
/*																			*/
/* DESCRIPTION:	Update file name cache with new file id						*/
/* INPUT:		ptFile:				file information						*/
/*				uiFid:				new file id								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void 
_fat_update_fname_cache(
	grp_fs_file_t	*ptFile,				/* [IN]  file information */
	grp_uint32_t	uiFid)					/* [IN]  new file id */
{
	fat_open_info_t	*ptOpen;				/* open information */
	grp_fs_fname_cache_t *ptCache;			/* file name cache */

	ptOpen = ptFile->pvFileInfo;			/* open info */
	ptCache = ptOpen->ptFnCache;			/* file name cache */
	if (ptCache								/* exist file name cache */
		&& ptCache->iDev == ptFile->iDev	/* match device */
		&& ptCache->uiFid == ptFile->uiFid) {/* match file id */
		ptCache->uiFid = uiFid;				/* change file id */
		if (ptCache->ptAlias)				/* exist alias */
			ptCache->ptAlias->uiFid = uiFid;/* change file id */
	}
}
#else	/* GRP_FS_FNAME_CACHE */
#define _fat_update_fname_cache(ptFile, uiFid)
#endif	/* GRP_FS_FNAME_CACHE */

/****************************************************************************/
/* FUNCTION:	_fat_lookup_size0_file										*/
/*																			*/
/* DESCRIPTION:	Lookup opened size 0 file, and return unique file id for	*/
/*				it.  If not found, allocate new open info if necessary.		*/
/* INPUT:		iDev:				device number							*/
/*				uiDirFid:			directory file id						*/
/*				uiEnd:				end offset of the entry					*/
/* OUTPUT:		pptOpen:			newly allocated open info				*/
/*																			*/
/* RESULT:		FAT_EOF_CLST:		not found or failed to allocate			*/
/*				others:				unique file id							*/
/*																			*/
/****************************************************************************/
static grp_uint32_t
_fat_lookup_size0_file(
	int				iDev,					/* [IN]  device number */
	grp_uint32_t	uiDirFid,				/* [IN]  directory file id */
	grp_uint32_t	uiDirEnd,				/* [IN]  start offset */
	fat_open_info_t	**pptOpen)				/* [OUT] allocated open info */
{
	fat_open_info_ctl_t *ptCtl;				/* open info control */
	fat_open_info_t	*ptOpen;				/* open information */

	ptCtl = grp_fs_fat_open_ctl;			/* open info control */
	for (ptOpen = ptCtl->ptSize0List; ptOpen; ptOpen = ptOpen->ptSz0Fwd) {
		if (ptOpen->iDev == iDev			/* match device */
			&& ptOpen->uiDirFid == uiDirFid	/* match directory */
			&& ptOpen->uiDirEnd == uiDirEnd) { /* match end offset */
			if (pptOpen)					/* return allocated */
				*pptOpen = NULL;			/* not allocated */
			return(ptOpen->uiUniqFid);		/* return found id */
		}
	}
	if (pptOpen == NULL)					/* just lookup without allocate */
		return(FAT_EOF_CLST);				/* return not found */
	ptOpen = _fat_alloc_open_info(iDev);	/* allocate free open info */
	*pptOpen = ptOpen;						/* set allocated info */
	if (ptOpen == NULL)						/* failed to allocate */
		return(FAT_EOF_CLST);				/* return failed to allocate */
	ptOpen->uiDirFid = uiDirFid;			/* set directory file id */
	ptOpen->uiDirEnd = uiDirEnd;			/* set end offset */
	grp_enque_shead(&ptCtl->ptSize0List, ptOpen, ptSz0);
											/* chain to size 0 list */
	return(ptOpen->uiUniqFid);				/* return allcoated uniq id */
}

/****************************************************************************/
/* FUNCTION:	_fat_deque_size0_list										*/
/*																			*/
/* DESCRIPTION:	Deque from size 0 list										*/
/* INPUT:		ptOpen:				open file information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_deque_size0_list(
	fat_open_info_t	*ptOpen)				/* [IN]  open information */
{
	fat_open_info_ctl_t *ptCtl;				/* open info control */

	ptCtl = grp_fs_fat_open_ctl;			/* open info control */
	grp_deque_sent(&ptCtl->ptSize0List, ptOpen, ptSz0);
											/* deque from size 0 list */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_1st_cluster										*/
/*																			*/
/* DESCRIPTION:	Set first cluster of the file								*/
/* INPUT:		ptFile:				file information						*/
/*				uiClst:				cluster assigned						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_set_1st_cluster(
	grp_fs_file_t	*ptFile,				/* [IN]  file information */
	grp_uint32_t	uiClst)					/* [IN]  cluster assigned */
{
	fat_open_info_t	*ptOpen;				/* open file information */
	grp_fs_file_t	*ptDir;					/* directory file information */
	int				iRet;					/* return value */

	ptOpen = ptFile->pvFileInfo;			/* open information */
	_fat_update_fname_cache(ptFile, uiClst);/* update file name cache */
	_fat_deque_size0_list(ptOpen);			/* deque from size0 list */
	grp_fs_change_fid(ptFile, uiClst);		/* change file id */
	grp_fs_get_current_time(&ptFile->iMTime);/* set modify time */
	ptFile->iATime = ptFile->iMTime;		/* set access time */
	ptFile->usStatus |= GRP_FS_FSTAT_UPD_BITS;/* set update bits */
	grp_fs_unblock_file_op(ptFile);			/* unblock to avoid deadlock */
	ptDir = grp_fs_block_file_op_by_id(ptFile->ptFs, ptOpen->uiDirFid);
											/* block file op on the directory */
	grp_fs_block_file_op(ptFile);			/* block file operation again */
	iRet = _fat_update_attr(ptFile);		/* update file attribute */
	if (ptDir == NULL)						/* no opened directory info */
		grp_fs_unblock_file_op_by_id(ptFile->ptFs); /* unblock by id */
	else									/* exist opened directory info */
		grp_fs_close_file(ptDir, GRP_FS_FILE_UNBLOCK);
											/* unblock by opened file */
	if (iRet < 0) {							/* failed to update attribute */
		(void)_fat_free_1st_cluster(ptFile);/* free 1st cluster */
	}
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	_fat_free_1st_cluster										*/
/*																			*/
/* DESCRIPTION:	Free first cluster of the file								*/
/* INPUT:		ptFile:				file information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				GRP_FS_ERR_TOO_MANY: too many open files					*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_free_1st_cluster(
	grp_fs_file_t	*ptFile)				/* [IN]  file information */
{
	fat_open_info_t	*ptOpen;				/* open file information */
	grp_fs_file_t	*ptDir;					/* directory file information */
	grp_uint32_t	uiFid;					/* unique file id */
	fat_open_info_ctl_t *ptCtl;				/* open info control */
	int				iRet;					/* return value */

	ptCtl = grp_fs_fat_open_ctl;			/* open info control */
	ptOpen = ptFile->pvFileInfo;			/* open information */
	uiFid = ptOpen->uiUniqFid;				/* unique file id */
	_fat_update_fname_cache(ptFile, uiFid);	/* update file name cache */
	grp_enque_shead(&ptCtl->ptSize0List, ptOpen, ptSz0);
											/* chain to size 0 list */
	grp_fs_change_fid(ptFile, uiFid);		/* change file id */
	grp_fs_get_current_time(&ptFile->iMTime);/* set modify time */
	ptFile->iATime = ptFile->iMTime;		/* set access time */
	ptFile->usStatus |= GRP_FS_FSTAT_UPD_BITS;/* set update bits */
	grp_fs_unblock_file_op(ptFile);			/* unblock to avoid deadlock */
	ptDir = grp_fs_block_file_op_by_id(ptFile->ptFs, ptOpen->uiDirFid);
											/* block file op on the directory */
	grp_fs_block_file_op(ptFile);			/* block file operation again */
	iRet = _fat_update_attr(ptFile);		/* update file attribute */
	if (ptDir == NULL)						/* no opened directory info */
		grp_fs_unblock_file_op_by_id(ptFile->ptFs); /* unblock by id */
	else									/* exist opened directory info */
		grp_fs_close_file(ptDir, GRP_FS_FILE_UNBLOCK);
											/* unblock by opened file */
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_cluster_map										*/
/*																			*/
/* DESCRIPTION:	Get mapping for file cluster offset to physical one			*/
/* INPUT:		ptFile:				file information						*/
/*				uiOffClBlk:			file offset cluster block number		*/
/* OUTPUT:		ptFile->puiMap		cluster map								*/
/*				ptFile->uiMapFblk	mapping start file block				*/
/*				ptFile->uiMapCnt	mapping count							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_get_cluster_map(
	grp_fs_file_t	*ptFile,			/* [IN/OUT]  file information */
	grp_uint32_t	uiOffClBlk)			/* [IN]  file offset block number */
{
	grp_uint32_t	uiClst;					/* data cluster number */
	grp_uint32_t	uiOff;					/* file cluster offset */
	int				iMap;					/* map count */
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	int				iRet = 0;				/* return value */
	grp_fs_bio_t	tBio;					/* buffer I/O information */

	/****************************************************/
	/* If closer mapping exists, search from it.		*/
	/* Otherwise, search from top						*/
	/****************************************************/
	if (ptFile->uiMapCnt
		&& ptFile->uiMapFBlk + ptFile->uiMapCnt <= uiOffClBlk) {
		uiOff = ptFile->uiMapFBlk + ptFile->uiMapCnt - 1;/* get offset */
		uiClst = ptFile->puiMap[ptFile->uiMapCnt - 1];	/* get cluster num */
	} else {								/* no map */
		uiOff = 0;							/* 0 offset */
		uiClst = ptFile->uiFid;				/* start cluster */
	}

	/****************************************************/
	/* get FAT one by one until found or EOF			*/
	/****************************************************/
	tBio.ptBuf = NULL;						/* no buffer */
	for ( ; uiOff != uiOffClBlk && uiClst != FAT_EOF_CLST; uiOff++) {
		iRet = _fat_get_fat(ptFs, uiClst, &tBio, &uiClst);	/* get next */
		if (iRet != 0)						/* error occured */
			goto err_out;					/* return error */
		if (uiClst >= ptBPB->uiEOF) {		/* end of file */
			uiClst = FAT_EOF_CLST;			/* EOF cluster value */
		} else if (uiClst < 2 || uiClst >= ptBPB->uiMaxClst) {/* bad */
			iRet = GRP_FS_ERR_FS;			/* invalid file system */
			goto err_out;					/* return error */
		} else if (uiClst == ptBPB->uiBadC) {	/* bad cluster */
			iRet = GRP_FS_ERR_IO;			/* set I/O error */
			goto err_out;					/* error return */
		}
	}

	/****************************************************/
	/* set mapping table								*/
	/****************************************************/
	ptFile->puiMap[0] = uiClst;				/* set mapping table */
	ptFile->uiMapFBlk = uiOff;				/* set mapping start offset */
	for (iMap = 1; iMap < FAT_MAP_CNT && uiClst != FAT_EOF_CLST; iMap++) {
		if (_fat_get_fat(ptFs, uiClst, &tBio, &uiClst))/* get next failed */
			break;							/* set mapping end */
		if (uiClst >= ptBPB->uiEOF) {		/* end of file */
			uiClst = FAT_EOF_CLST;			/* EOF FAT value */
		} else if (uiClst < 2				/* bad value */
				  || uiClst >= ptBPB->uiMaxClst	/* bad value */
				  || uiClst == ptBPB->uiBadC) {	/* bad cluster */
			break;
		}
		ptFile->puiMap[iMap] = uiClst;		/* set mapping */
	}
	ptFile->uiMapCnt = iMap;				/* set map count */
	if (tBio.ptBuf)							/* buffer exists */
		grp_fs_unref_buf(&tBio);			/* release buffer */
	return(0);								/* return success */

err_out:
	ptFile->uiMapCnt = 0;					/* clear map count */
	ptFile->uiMapFBlk = 0;					/* clear mapping start offset */
	if (tBio.ptBuf)							/* buffer exists */
		grp_fs_unref_buf(&tBio);			/* release buffer */
	return(iRet);							/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_phys_cluster											*/
/*																			*/
/* DESCRIPTION:	Convert file cluster offset to physical cluster number		*/
/* INPUT:		ptFile:				file information						*/
/*				uiOffClBlk:			file offset cluster block number		*/
/* OUTPUT:		puiDataClst:		physical cluster number					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_phys_cluster(
	grp_fs_file_t	*ptFile,				/* [IN]  file information */
	grp_uint32_t	uiOffClBlk,				/* [IN]  file offset block number */
	grp_uint32_t	*puiDataClst)			/* [OUT] phyical cluster number */
{
	int				iRet;					/* return value */

	/****************************************************/
	/* for size 0 file id, return EOF					*/
	/****************************************************/
	if (FAT_IS_SIZE0_FID(ptFile->uiFid)) {	/* size 0 file id */
		*puiDataClst = FAT_EOF_CLST;		/* set EOF */
		return(0);							/* return */
	}

	/****************************************************/
	/* for 0th block, return file cluster number		*/
	/****************************************************/
	if (uiOffClBlk == 0) {					/* 0th cluster */
		*puiDataClst = ptFile->uiFid;		/* data cluster = file number */
		return(0);							/* return success */
	}

	/****************************************************/
	/* if not in cluster map, get the map for the block */
	/****************************************************/
	*puiDataClst = FAT_EOF_CLST;			/* init with EOF */
	if (ptFile->uiMapFBlk > uiOffClBlk
		|| uiOffClBlk >= ptFile->uiMapFBlk + ptFile->uiMapCnt) {
		iRet = _fat_get_cluster_map(ptFile, uiOffClBlk);	/* get mapping */
		if (iRet != 0)						/* error detected */
			return(iRet);					/* return error */
	}

	/****************************************************/
	/* get physical data cluster map					*/
	/****************************************************/
	if (uiOffClBlk >= ptFile->uiMapFBlk + ptFile->uiMapCnt) {/* out of range */
		*puiDataClst = FAT_EOF_CLST;		/* EOF */
	} else {								/* in mapping */
		*puiDataClst = ptFile->puiMap[uiOffClBlk - ptFile->uiMapFBlk];
	}

	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_phys_blk												*/
/*																			*/
/* DESCRIPTION:	Convert offset block number to physical block number		*/
/* INPUT:		ptFile:				file information						*/
/*				uiOffBlk:			file offset cluster block number		*/
/* OUTPUT:		puiDataBlk:			physical block number					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_phys_blk(
	grp_fs_file_t	*ptFile,				/* [IN]  file information */
	grp_uint32_t	uiOffBlk,				/* [IN]  file offset block number */
	grp_uint32_t	*puiDataBlk)			/* [OUT] phyical block number */
{
	int				iRet;					/* return value */
	grp_uint32_t	uiOffClBlk;				/* offset cluster block number */
	grp_uint32_t	uiClst;					/* cluster number */
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* file system info */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */

	/****************************************************/
	/* check FAT12/16 root directory as a special case	*/
	/****************************************************/
	if (ptFile->uiFid == 0) {				/* root directory */
		*puiDataBlk = (uiOffBlk < ptBPB->uiDBlkStart)?	/* within root */
			uiOffBlk: FAT_EOF_BLK;			/* set block number */
		return(0);							/* return EOF */
	}

	/****************************************************/
	/* get physical cluster number						*/
	/****************************************************/
	uiOffClBlk = FAT_DBLK_CLST(ptBPB, uiOffBlk);/* offset cluster block */
	iRet = _fat_phys_cluster(ptFile, uiOffClBlk, &uiClst);
											/* get physical cluster number */
	if (iRet != 0) {						/* error */
		*puiDataBlk = FAT_EOF_BLK;			/* EOF */
		return(iRet);						/* return error */
	}
	if (uiClst == FAT_EOF_CLST) {			/* EOF */
		*puiDataBlk = FAT_EOF_BLK;			/* EOF */
		return(0);							/* return success */
	}
	uiOffBlk = FAT_DBLK_CLST_OFF(ptBPB, uiOffBlk); /* offset in cluster */
	*puiDataBlk = FAT_PHYS_CLST_DBLK(ptBPB, uiClst, uiOffBlk);
											/* set physical block number */
	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_dir_size											*/
/*																			*/
/* DESCRIPTION:	Count directory size										*/
/* INPUT:		ptDir:				directory file information				*/
/* OUTPUT:		piSize:				size information						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_get_dir_size(
	grp_fs_file_t	*ptDir,					/* [IN]  directory file */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_uisize_t	*puiSize)				/* [OUT] size information */
#else  /* GRP_FS_ENABLE_OVER_2G */
	grp_isize_t		*piSize)				/* [OUT] size information */
#endif /* GRP_FS_ENABLE_OVER_2G */
{
	grp_uint32_t	uiOffBlk;				/* offset cluster block number */
	grp_uint32_t	uiDataBlk;				/* data block */
	grp_uint32_t	uiOffset;				/* offset */
	grp_uint32_t	uiClstSize;				/* cluster size */
	grp_fs_info_t	*ptFs;					/* file system information */
	fat_BPB_t		*ptBPB;					/* BPB information */
	int				iRet;					/* return value */
	grp_uint32_t	uiMaxClst;				/* max cluster number */

	ptFs = ptDir->ptFs;						/* file system information */
	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	if (ptDir->uiFid == 0) {				/* root directory of FAT12/16 */
#ifdef GRP_FS_ENABLE_OVER_2G
		*puiSize = (ptBPB->uiDBlkStart << ptFs->ucFsDBlkShift); /* set size */
#else  /* GRP_FS_ENABLE_OVER_2G */
		*piSize = (ptBPB->uiDBlkStart << ptFs->ucFsDBlkShift); /* set size */
#endif /* GRP_FS_ENABLE_OVER_2G */
		return(0);							/* return success */
	}
	uiMaxClst = FAT_CLST(ptBPB, (grp_uint32_t)0x80000000);
											/* max cluster number */
	if (uiMaxClst > ptBPB->uiBadC)			/* over bad cluster number */
		uiMaxClst = ptBPB->uiBadC;			/* adjust max cluster number */
	uiClstSize = (1 << ptFs->ucFsCBlkShift);/* cluster size */
	uiOffset = 0;							/* offset */
	for (uiOffBlk = 0;
		 (iRet = _fat_phys_cluster(ptDir, uiOffBlk, &uiDataBlk)) == 0
		 && uiDataBlk != FAT_EOF_CLST; uiOffBlk++) { /* count cluster blocks */
		uiOffset += uiClstSize;				/* update offset */
		if (uiOffBlk >= uiMaxClst)			/* over max offset */
			return(GRP_FS_ERR_FS);			/* return error */
		if (fat_interrupt_lookup &&			/* defined interrupt hook */
		    (iRet = fat_interrupt_lookup(ptDir->iDev, ptDir->uiFid,
								uiOffset)) != 0)	/* interrupted */
			return(iRet);					/* return error */
	}
#ifdef GRP_FS_ENABLE_OVER_2G
	*puiSize = uiOffset;					/* set size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	*piSize = uiOffset;						/* set size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	return(iRet);							/* return status */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_free_cache											*/
/*																			*/
/* DESCRIPTION:	Get free cluster cache										*/
/* INPUT:		ptFs:				file system information					*/
/* OUTPUT:		ptBPB->iFreeTblCnt:	free cache count						*/
/*				ptBPB->auiFreeTbl:	free cache								*/
/*				ptBPB->iFreeGetIdx:	free get index							*/
/*				ptBPB->iFreePutIdx:	free put index 							*/
/*				ptBPB->uiNextFree:  next free hint							*/
/*				ptFs->uiFsFreeBlk:  free count								*/
/*				ptFs->uiFsFreeFile: free count								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				0:					success									*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
static int
_fat_get_free_cache(
	grp_fs_info_t	*ptFs)				/* [IN]  FS information */
{
	grp_uchar_t		*pucBp;				/* buffer pointer */
	grp_uchar_t		*pucEndp;			/* end buffer pointer */
	fat_BPB_t		*ptBPB;				/* BPB information */
	grp_uint32_t	uiSchCnt;			/* search count */
	grp_uint32_t	uiCluster;			/* cluster number */
	grp_uint32_t	uiFat;				/* FAT value */
	grp_uint32_t	uiPrev = 0;			/* previous byte value */
	grp_uint32_t	uiFreeCnt;			/* free count */
	grp_uint32_t	uiMaxFreeCnt;		/* max free count */
	grp_uint32_t	uiMaxClst;			/* max cluster */
	grp_uint32_t	uiFBlk;				/* block number */
	grp_uint32_t	uiOff;				/* offset in a block */
	grp_uint32_t	uiNextSec;			/* next sector to read */
	int				iBytePos;			/* byte position */
	grp_int32_t		iSec;				/* sector count to I/O */
	grp_int32_t		iIoCnt;				/* I/O done sector count */
	grp_uchar_t		*pucBuf = NULL;		/* I/O buffer */
	grp_uint32_t	uiEndSec = 0;		/* end sector */
	grp_int32_t		iBufSec = 0;		/* buffer sector count */
	int				iRet = 0;			/* return value */
	grp_fs_bio_t	tBio;				/* buffer I/O information */

	/****************************************************/
	/* if free block count is less than or equal 0, and	*/
	/* full free check has been made, do nothing		*/
	/****************************************************/
	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	if (ptFs->uiFsFreeBlk <= 0				/* no free block left */
		&& (ptBPB->uiStatus & FAT_STAT_FREE_CHKED)) /* fully checked */
		return(0);							/* do nothing */

	/****************************************************/
	/* if mount mode is GRP_FS_STAT_RONLY and succeed   */
	/* in reading of FSINFO then return                 */
	/****************************************************/
	if( (ptFs->usStatus & GRP_FS_STAT_RONLY) &&	 /* read only mount */
		(FAT_TYPE_32 == ptBPB->iFsType) &&		 /* FAT32 file system */
		(FAT_FI_NOHINT != ptFs->uiFsFreeBlk) &&	 /* free block info valid */
		(FAT_FI_NOHINT != ptBPB->uiNextFree) ){	 /* next free info valid */
		return(0);								 /* do nothing */
	}

	/****************************************************/
	/* allocate special I/O buffer in case of no hint	*/
	/* at mount if possible								*/
	/****************************************************/
	uiMaxClst = ptBPB->uiMaxClst;			/* max cluster number */
	if (ptBPB->uiNextFree == FAT_FI_NOHINT) {/* no hint at mount */
		pucBuf = grp_mem_alloc(grp_fs_fat_cnt_buf_sz); /* allocate I/O buffer */
		iBufSec = (grp_int32_t)(grp_fs_fat_cnt_buf_sz >> ptBPB->iByteSecShift);
											/* sectors in the buffer */
		uiOff = FAT_OFF(ptBPB, uiMaxClst);	/* end offset of FAT */
		uiEndSec = FAT_NSEC(ptBPB, uiOff);	/* end sector of FAT */
	}

	/****************************************************/
	/* setup inital values								*/
	/****************************************************/
	uiCluster = (ptBPB->uiNextFree == FAT_FI_NOHINT)? 2: ptBPB->uiNextFree;
	if (uiCluster < 2 || uiCluster >= uiMaxClst) /* invalid value */
		uiCluster = 2;						/* set to 2 */
	if (ptBPB->iFsType == FAT_TYPE_12)		/* FAT12 */
		uiCluster &= ~1;					/* start from even cluster */
	uiOff = FAT_OFF(ptBPB, uiCluster);		/* offset of start cluster FAT */
	uiFBlk = FAT_FBLK(ptBPB, uiOff);		/* block number of start FAT */
	uiOff = FAT_FBLK_OFF(ptBPB, uiOff);		/* offset in the block */
	uiNextSec = (uiFBlk << (ptBPB->iFBlkShift - ptBPB->iByteSecShift));
											/* sector of next block */
	tBio.ptBuf = NULL;						/* init cache buffer */
	if (_fat_get_blk(ptFs, uiFBlk, GRP_FS_BUF_FILE, &tBio) == 0) {
											/* exist cache */
		pucBp = &tBio.pucData[uiOff];		/* set FAT data pointer */
		pucEndp = &tBio.pucData[tBio.uiSize]; /* end of data */
		uiFBlk++;							/* set for next block */
		uiOff = 0;							/* reset offset for next */
	} else {								/* no in block I/O buffer */
		pucBp = pucEndp = NULL;				/* clear FAT data pointer */
	}
	ptBPB->iFreeTblCnt = 0;					/* reset free table count */
	ptBPB->iFreeGetIdx = 0;					/* reset free get index */
	uiFreeCnt = 0;							/* clear free count */
	uiMaxFreeCnt = (ptFs->uiFsFreeBlk == FAT_FI_NOHINT)?
				ptBPB->uiMaxClst: FAT_FREE_TBL;		/* set max found count */
	iBytePos = 0;							/* byte position for FAT12 */

	/****************************************************/
	/* search free FATs, and set them in cache			*/
	/****************************************************/
	for (uiSchCnt = 2; uiSchCnt < uiMaxClst; ) {
		if (pucBp >= pucEndp) {				/* end of buffer */
			if (tBio.ptBuf)					/* cache buffer exists */
				grp_fs_unref_buf(&tBio);	/* release cached buffer */
			if (pucBuf) {					/* exist special I/O buffer */
				iSec = (grp_int32_t)(uiEndSec - uiNextSec);
											/* # of logical setors to end */
				if (iSec > iBufSec)			/* over buffer */
					iSec = iBufSec;			/* adjust to buffer size */
				iSec <<= (ptBPB->iByteSecShift - (int)ptFs->ucDevBlkShift);
											/* convert to device sectors */
				iIoCnt = grp_fs_exec_dev_io(ptFs, uiFBlk, pucBuf, iSec,
										GRP_FS_IO_READ, GRP_FS_BUF_FILE);
				if (iIoCnt != iSec) {		/* read error */
					if (iIoCnt >= 0)		/* not error number */
						iRet = GRP_FS_ERR_IO; /* set I/O error */
					else					/* error number */
						iRet = (int)iIoCnt;	/* use it as return value */
					goto out;				/* return error */
				}
				iIoCnt >>= (ptBPB->iByteSecShift - (int)ptFs->ucDevBlkShift);
											/* convert to logical sectors */
				pucBp = &pucBuf[uiOff];		/* set buffer pointer */
				pucEndp = &pucBuf[iIoCnt << ptBPB->iByteSecShift];
											/* set end pointer */
				uiFBlk += (iIoCnt >> 
							(ptBPB->iFBlkShift - ptBPB->iByteSecShift));
											/* advance to next block */
				uiNextSec += iIoCnt;		/* next I/O sector */
			} else {						/* no special buffer */
				iRet = _fat_get_blk(ptFs, uiFBlk, GRP_FS_BUF_FILE, &tBio);
											/* get next block */
				if (iRet < 0)				/* error or no data */
						goto out;			/* return error */
				pucBp = &tBio.pucData[uiOff];/* set FAT data pointer */
				pucEndp = &tBio.pucData[tBio.uiSize]; /* set end pointer */
				uiFBlk++;					/* set for next block */
			}
			uiOff = 0;						/* reset offset for next */
		}
		switch (ptBPB->iFsType) {			/* file system type */
		case FAT_TYPE_12:					/* FAT12 */
			if (iBytePos == 0) {			/* 1st byte */
				uiPrev = *pucBp++;			/* set high 8 bits  */
				iBytePos = 1;				/* advance byte position */
				continue;					/* get next byte */
			} else if (iBytePos == 1) {		/* 2nd byte */
				uiFat = uiPrev; 			/* set low 8 bits */
				uiFat |= (((uiPrev = *pucBp++) << 8) & 0x0f00);
											/* set high 4 bits */
				iBytePos = 2;				/* advance byte position */
			} else {						/* 3rd byte */
				uiFat = (uiPrev >> 4);		/* set low 4 bits */
				uiFat |= (*pucBp++ << 4);	/* set high 8 bits */
				iBytePos = 0;				/* reset to 1st byte position */
			}
			break;
		case FAT_TYPE_16:					/* FAT16 */
			uiFat = (((grp_uint32_t)pucBp[1] << 8) | pucBp[0]);
											/* get 16 bits FAT */
			pucBp += 2;						/* advance to next */
			break;
		default:							/* FAT32 */
			uiFat = (((grp_uint32_t)pucBp[3] << 24)
					 | ((grp_uint32_t)pucBp[2] << 16)
					 | ((grp_uint32_t)pucBp[1] << 8)
					 | pucBp[0]);			/* get 32 bits FAT */
			uiFat &= 0x0fffffff;			/* mask reserve 4bits */
			pucBp += 4;						/* advance to next */
			break;
		}
		if (uiFat == 0) {							/* found free */
			if (uiFreeCnt < FAT_FREE_TBL)			/* table not full */
				ptBPB->auiFreeTbl[ptBPB->iFreeTblCnt++] = uiCluster;
													/* set it */
			if (++uiFreeCnt >= uiMaxFreeCnt)		/* got max free */
				break;								/* stop search */
		}
		uiSchCnt++;									/* increment search count */
		uiCluster++;								/* increment cluster */
		if (uiCluster >= uiMaxClst && uiSchCnt < uiMaxClst) {/* over max */
			uiCluster = 2;							/* lap to 2 */
			pucBp = pucEndp = NULL;					/* reset pointer */
			uiOff = FAT_OFF(ptBPB, uiCluster);		/* offset of FAT */
			uiFBlk = FAT_FBLK(ptBPB, uiOff);		/* block number of FAT */
			uiOff = FAT_FBLK_OFF(ptBPB, uiOff);		/* offset in the block */
			uiNextSec = (uiFBlk << (ptBPB->iFBlkShift - ptBPB->iByteSecShift));
													/* sector of next block */
			iBytePos = 0;							/* rest byte position */
		}
	}
	ptBPB->iFreePutIdx = (ptBPB->iFreeTblCnt < FAT_FREE_TBL)?
				ptBPB->iFreeTblCnt: 0;				/* set free put index */

	/****************************************************/
	/* if full check has been made, set count info		*/
	/****************************************************/
	if (uiSchCnt >= uiMaxClst) {					/* fully searched */
		ptBPB->uiStatus |= FAT_STAT_FREE_CHKED;		/* set checked bit */
		ptFs->uiFsFreeBlk = uiFreeCnt;				/* set free count */
		ptFs->uiFsFreeFile = uiFreeCnt;				/* set free count */
	}
	ptBPB->uiNextFree = (ptBPB->iFreeTblCnt)?
		ptBPB->auiFreeTbl[ptBPB->iFreeGetIdx]: 2;
													/* set next free hint */
	if (ptBPB->uiNextFree >= uiMaxClst)				/* over max */
		ptBPB->uiNextFree = 2;						/* lap to 2 */
out:
	if (pucBuf)										/* allocate buffer */
		grp_mem_free(pucBuf);						/* free buffer */
	if (tBio.ptBuf)									/* buffer exists */
		grp_fs_unref_buf(&tBio);					/* release buffer */
	return(iRet);									/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_link_new_cluster										*/
/*																			*/
/* DESCRIPTION:	Link a new cluster											*/
/* INPUT:		ptFs:				file system information					*/
/*				uiPrev:				previous cluster number 				*/
/* 				uiNew:				new cluster number to link				*/
/*				ptBio:				buffer I/O information					*/
/*				iFatNum:			FAT number								*/
/* OUTPUT:		ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		invalid file system						*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_link_new_cluster(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_uint32_t	uiPrev,				/* [IN]  previous cluster */
	grp_uint32_t	uiNew,				/* [IN]  new cluster number to link */
	grp_fs_bio_t	*ptBio,				/* [IN/OUT] buffer I/O information */
	int				iFatNum)			/* [IN]  FAT number */
{
	fat_BPB_t		*ptBPB;				/* BPB information */
	grp_uint32_t	uiFat;				/* FAT value */
	grp_uint32_t	uiPrevFat;			/* FAT value of previous cluster */
	int				iRet; 				/* return value */

	/****************************************************/
	/* set EOC mark to allocated one					*/
	/****************************************************/
	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	uiFat = ptBPB->uiEOC;					/* EOC */
	iRet = _fat_set_fatn(ptFs, uiNew, ptBio, &uiFat, iFatNum);
											/* set EOC */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
	if (uiFat != 0) {						/* no free one */
		grp_fs_printf("FAT: not free FAT%d 0x%lx(dev 0x%x)\n",
						iFatNum, (unsigned long)uiNew, ptFs->iDev);
		iRet = GRP_FS_ERR_FS;			/* set error number */
		goto restore_free_out;			/* return with restore */
	}

	/****************************************************/
	/* link to previous cluster							*/
	/****************************************************/
	if (uiPrev) {						/* previous cluster exists */
		uiPrevFat = uiNew;				/* new cluster block */
		iRet = _fat_set_fatn(ptFs, uiPrev, ptBio, &uiPrevFat, iFatNum);
										/* set link */
		if (iRet != 0)					/* error detected */
			goto restore_free_out;		/* return with restore */
		if (uiPrevFat != ptBPB->uiEOC) {/* not EOC */
			grp_fs_printf("FAT: not EOC FAT%d 0x%lx(dev 0x%x)\n",
							iFatNum, (unsigned long)uiPrev, ptFs->iDev);
			iRet = GRP_FS_ERR_FS;		/* set error number */
			goto restore_prev_out;		/* return with restore */
		}
	}
	return(0);

restore_prev_out:
	if (_fat_set_fat(ptFs, uiPrev, ptBio, &uiPrevFat))
		grp_fs_printf("FAT: restore FAT 0x%lx failed(value 0x%lx, dev 0x%x)\n",
					(unsigned long)uiPrev, (unsigned long)uiPrevFat,
					ptFs->iDev);
restore_free_out:
	if (_fat_set_fat(ptFs, uiNew, ptBio, &uiFat))
		grp_fs_printf("FAT: restore FAT 0x%lx failed(value 0x%lx, dev 0x%x)\n",
					(unsigned long)uiNew, (unsigned long)uiFat, ptFs->iDev);
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_fat_get_free_cluster										*/
/*																			*/
/* DESCRIPTION:	Get free contiguous clusters for the offset					*/
/* INPUT:		ptFs:				file system information					*/
/* 				ptFile:				file information						*/
/*				uiOffClBlk:			offset cluster block number				*/
/*				iCnt:				allocate count							*/
/*				uiDirFid			file ID of parent directory				*/
/* OUTPUT:		puiFree:			free cluster number						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				positive:			allocated count							*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_get_free_cluster(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiOffClBlk,			/* [IN]  offset block number  */
	grp_int32_t		iCnt,				/* [IN]  allocate count */
	grp_uint32_t	uiDirFid,			/* [IN]  file ID of parent directory */
	grp_uint32_t	*puiFree)			/* [OUT] free cluster number */
{
	fat_BPB_t		*ptBPB;				/* BPB information */
	grp_uint32_t	uiPrev = 0;			/* previous cluster */
	grp_uint32_t	uiFree = 0;			/* free cluster */
	int				iRet = 0;			/* return value */
	int				iFatNum;			/* FAT number */
	int				iFatCnt;			/* FAT count */
	grp_int32_t		iAlloc = 0;			/* allocated count */
	grp_uint32_t	uiFid = 0;			/* cluster number of this file */
	grp_fs_bio_t	tBio;				/* buffer I/O information */

	/****************************************************/
	/* get physical cluster number of previous one		*/
	/****************************************************/
	*puiFree = FAT_EOF_CLST;				/* set default return value */
	if (uiOffClBlk != 0) {					/* previous cluster exists */
		iRet = _fat_phys_cluster(ptFile, uiOffClBlk - 1, &uiPrev);
											/* get prev */
		if (iRet != 0)						/* error occured */
			return((grp_int32_t)iRet);		/* return error */
		if (uiPrev == FAT_EOF_CLST)			/* EOF */
			return(GRP_FS_ERR_FS);			/* return error */
	} else if (ptFile 						/* file exists */
				&& !FAT_IS_SIZE0_FID(ptFile->uiFid)) {	/* 1st blk allocated */
		/****************************************************/
		/* since 1st cluster is already allocated, advance	*/
		/* to next cluster									*/
		/****************************************************/
		*puiFree = uiPrev = ptFile->uiFid;	/* set cluster number */
		iAlloc++;							/* increment for 1st cluster */
		uiOffClBlk++;						/* skip to next */
		if (iCnt <= 1)						/* only alloc 1 cluster */
			return(iAlloc);					/* return here */
	}

	/****************************************************/
	/* get cluster number of my own and prarent			*/
	/****************************************************/
	if (ptFile) {							/* exist file */
		fat_open_info_t	*ptOpen = ptFile->pvFileInfo; /* open information */
		uiFid = ptFile->uiFid;				/* file ID */
		uiDirFid = ptOpen->uiDirFid;		/* cluseter number of parent */
	}

	/****************************************************/
	/* block modification								*/
	/****************************************************/
	tBio.ptBuf = NULL;						/* no buffer */
	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	grp_fs_block_fs_mod(ptFs);				/* block modification */

	/****************************************************/
	/* get free caches, and set cluster link			*/
	/****************************************************/
	for ( ; iAlloc < iCnt; iAlloc++) {
		/****************************************************/
		/* fill free cache, if empty						*/
		/****************************************************/
		uiFree = 0;							/* no cluster to free */
		if (ptBPB->iFreeTblCnt <= 0) { 		/* no free in cache */
			iRet = _fat_get_free_cache(ptFs);/* get free cache */
			if (iRet < 0)					/* error occured */
				break;						/* break */
		}

		/****************************************************/
		/* check free										*/
		/****************************************************/
		if (ptBPB->iFreeTblCnt <= 0) {		/* no free */
			iRet = GRP_FS_ERR_NO_SPACE;		/* return no space error */
			break;							/* break */
		}

		/****************************************************/
		/* get free cache									*/
		/****************************************************/
		uiFree = ptBPB->auiFreeTbl[ptBPB->iFreeGetIdx];/* get cache */
		if (iAlloc == 0)					/* 1st cluster */
			*puiFree = uiFree;				/* set it in *puiFree */
		else if (uiFree != uiPrev + 1)		/* not contiguous */
			break;							/* break */
		ptFs->uiFsFreeBlk--;				/* decrement free count */
		ptFs->uiFsFreeFile = ptFs->uiFsFreeBlk;/* free file */
		ptBPB->iFreeTblCnt--;				/* decrement free cache count */
		if (++(ptBPB->iFreeGetIdx) >= FAT_FREE_TBL)	/* over get index */
			ptBPB->iFreeGetIdx = 0;			/* lap around get index */
		if(ptBPB->iFreeTblCnt > 0) {		/* free in cache */
			ptBPB->uiNextFree =				/* set free from cache */
				ptBPB->auiFreeTbl[ptBPB->iFreeGetIdx];
		} else if((ptBPB->uiNextFree + 1) >= ptBPB->uiMaxClst) {
			ptBPB->uiNextFree = 2;			/* no free in cache & over max */
		}
		if (uiFree == uiFid || uiFree == uiDirFid) { /* duplicated */
			uiFree = 0;						/* no cluster to free */
			iRet = GRP_FS_ERR_FS;			/* invalid file system */
			break;							/* stop here */
		}

		/****************************************************/
		/* link free cluster								*/
		/****************************************************/
		if (ptBPB->iFsType == FAT_TYPE_32
			&& FAT_NOT_MIRRORED(ptBPB)) {	/* single FAT */
			iFatNum = FAT_ACTIVE_NUM(ptBPB);/* active FAT */
			iFatCnt = 1;					/* FAT count */
		} else {							/* mirrored FAT */
			iFatNum = 0;					/* start from 0 */
			iFatCnt = ptBPB->ucFatCnt;		/* FAT count */
		}
		while (iFatCnt-- > 0) {
			iRet = _fat_link_new_cluster(ptFs, uiPrev, uiFree, &tBio,
									iFatNum++);	/* link free */
			if (iRet < 0)					/* error */
				goto out;					/* return error */
		}
		if (tBio.ptBuf) {					/* buffer exists */
			iRet = _fat_clean_unref_buf(&tBio);	/* clean it */
			if (iRet < 0)					/* error */
				goto out;					/* return error */
		}

		/****************************************************/
		/* set cluster mapping cache						*/
		/****************************************************/
		if (ptFile) {						/* file exists */
			if (FAT_IS_SIZE0_FID(ptFile->uiFid)) {	/* size 0 file id */
				grp_fs_unblock_fs_mod(ptFs);/* unblock to avoid deadlock */
				iRet = _fat_set_1st_cluster(ptFile, uiFree);
											/* set 1st cluster */
				grp_fs_block_fs_mod(ptFs);	/* block modification again */
				if (iRet < 0)	 			/* error */
					goto out;				/* return error */
			} else {
				if (ptFile->uiMapFBlk + ptFile->uiMapCnt != uiOffClBlk
					|| ptFile->uiMapCnt >= FAT_MAP_CNT) { /* isolate or over */
					ptFile->uiMapCnt = 0;	/* clear map count */
					ptFile->uiMapFBlk = uiOffClBlk; /* set start block */
				}
				ptFile->puiMap[ptFile->uiMapCnt++] = uiFree; /* set it */
			}
		}
		uiPrev = uiFree;					/* set current to previous */
		uiOffClBlk++;						/* next cluster */
	}
out:
	if (tBio.ptBuf)	{						/* buffer exists */
		int	iCleanRet = _fat_clean_unref_buf(&tBio); /* clean it */
		if (iRet == 0)						/* no error upto now */
			iRet = iCleanRet;				/* use return value of clean buf */
	}
	grp_fs_unblock_fs_mod(ptFs);			/* unblock modification */
	if (iRet != 0 && uiFree != 0) {			/* need to free with error */
		if (ptFile == NULL					/* no file information */
			|| FAT_IS_SIZE0_FID(ptFile->uiFid)) {	/* size 0 file id */
			grp_uint32_t	uiNext;			/* next cluster */
			(void)_fat_free_cluster(ptFs, NULL, uiFree, &uiNext);
		} else {							/* exist file information */
			(void)_fat_free_cluster_list(ptFile, uiFree, 1, uiOffClBlk);
											/* free file cluster list */
		}
	}
	if (iAlloc == 0) {						/* not allocated */
		*puiFree = FAT_EOF_CLST;			/* set FAT_EOF_CLST */
	}
	return((iAlloc == 0)? (grp_int32_t)iRet: iAlloc);
											/* return error or alloc count */
}

/****************************************************************************/
/* FUNCTION:	_fat_invalidate_buf_cache									*/
/*																			*/
/* DESCRIPTION:	Invalidate buffer cache										*/
/* INPUT:		ptFs:				file system information					*/
/*				uiCluster:			cluster number to invalidate			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_invalidate_buf_cache(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_uint32_t	uiCluster)			/* [IN]  cluster number to invalidate */
{
	grp_int32_t		iBlkCnt;			/* block count */
	grp_uint32_t	uiBlk;				/* block number */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo; /* BPB information */
	grp_fs_bio_t	tFsBio;				/* buffer I/O information */
	int				iRet;				/* return value */

	uiBlk = FAT_PHYS_CLST_DBLK(ptBPB, uiCluster, 0);
											/* data block number */
	iBlkCnt = ((grp_int32_t)1 << ptBPB->iDBlkClstShift);
											/* block count in cluster */
	while (iBlkCnt-- > 0) {					/* loop for all blocks in cluster */
		iRet = grp_fs_lookup_buf(ptFs, uiBlk++, GRP_FS_BUF_DATA,
							(grp_int32_t)ptBPB->uiDBlkSize, &tFsBio);
											/* allocate a block */
		if (iRet == 0) {					/* found cache */
			tFsBio.ptBuf->iDev = -1;		/* clear dev association */
			grp_fs_unref_buf(&tFsBio);		/* free/invalidate buffer */
		}
	}
}

/****************************************************************************/
/* FUNCTION:	_fat_free_cluster											*/
/*																			*/
/* DESCRIPTION:	Free a cluster												*/
/* INPUT:		ptFs:				file system information					*/
/*				ptBio:				buffer I/O information					*/
/*				uiCluster:			cluster number to free					*/
/* OUTPUT:		puiNext:			next cluster to free (previous value)	*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_free_cluster(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_fs_bio_t	*ptBio,				/* [IN]  buffer I/O information */
	grp_uint32_t	uiCluster,			/* [IN]  cluster number to free */
	grp_uint32_t	*puiNext)			/* [OUT] next cluster to free */
{
	fat_BPB_t		*ptBPB;				/* BPB information */
	grp_uint32_t	uiFat;				/* FAT value */
	int				iRet;				/* return value */

	/****************************************************/
	/* free a cluster									*/
	/****************************************************/
	if (ptBio == NULL)						/* no buffer I/O information */
		grp_fs_block_fs_mod(ptFs);			/* block modification */
	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	uiFat = 0;								/* free FAT value */
	iRet = _fat_set_fat(ptFs, uiCluster, ptBio, &uiFat);	/* set free */
	*puiNext = FAT_EOF_CLST;				/* default next cluster */
	if (iRet == 0) {
		if (ptBPB->iFreeTblCnt < FAT_FREE_TBL) { /* free cache is not full */
			ptBPB->iFreeTblCnt++;				/* increment free count */
			ptBPB->auiFreeTbl[ptBPB->iFreePutIdx++] = uiCluster;/* set cache */
			if (ptBPB->iFreePutIdx >= FAT_FREE_TBL)	/* over put index */
				ptBPB->iFreePutIdx = 0;		/* lap around put index */
		}
		ptFs->uiFsFreeBlk++;				/* increment free count */
		ptFs->uiFsFreeFile = ptFs->uiFsFreeBlk;/* free file */
		*puiNext = uiFat;					/* set next cluster */
	} else {
		grp_fs_printf("FAT: failed to free FAT 0x%lx(dev 0x%x)\n",
					(unsigned long)uiCluster, ptFs->iDev);
	}
	if (ptBio == NULL)						/* no buffer I/O information */
		grp_fs_unblock_fs_mod(ptFs);		/* unblock modification */
	return(iRet);							/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_free_cluster_list_n									*/
/*																			*/
/* DESCRIPTION:	Free cluster list in FATn									*/
/* INPUT:		ptFile:				file information						*/
/*				uiStart:			start cluster number					*/
/*				uiCnt:				cluster count to free					*/
/*				uiPrev:				previous cluster						*/
/*				iRemain:			remaining FAT							*/
/*				ptBio:				buffer I/O information					*/
/*				iFatNum:			FAT number								*/
/* OUTPUT:		ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_free_cluster_list_n(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiStart,			/* [IN]  start cluster number */
	grp_uint32_t	uiCnt,				/* [IN]  cluster count to free */
	grp_uint32_t	uiPrev,				/* [IN]  previous cluster */
	int				iRemain,			/* [IN]  remaining FAT */
	grp_fs_bio_t	*ptBio,				/* [IN/OUT] buffer I/O information */
	int				iFatNum)			/* [IN]  FAT number */
{
	grp_uint32_t	uiCluster;			/* cluster number */
	grp_uint32_t	uiNext = 0;			/* next number */
	grp_fs_info_t	*ptFs = ptFile->ptFs;/* file system information */
	fat_BPB_t		*ptBPB;				/* BPB information */
	int				iFreeRet = 0;		/* return value of cluster free */
	int				iRet = 0;			/* return value */

	/****************************************************/
	/* set EOC to previous cluster if specified			*/
	/****************************************************/
	ptBPB = ptFs->pvFsInfo;				/* BPB information */
	if (uiPrev) {
		uiNext = ptBPB->uiEOC;			/* EOC mark */
		iRet = _fat_set_fatn(ptFs, uiPrev, ptBio, &uiNext, iFatNum);
										/* set EOC */
		if (iRet != 0) 					/* error occured */
			grp_fs_printf("FAT: cannot set EOC for FAT%d 0x%lx(blk 0x%lx)(dev 0x%x)\n",
					iFatNum, (unsigned long)ptFile->uiFid,
					(unsigned long)uiPrev, ptFs->iDev);
	}

	/****************************************************/
	/* free cluster list								*/
	/****************************************************/
	uiCluster = uiPrev;					/* set previous */
	uiNext = uiStart;					/* start to free */
	while (uiNext < ptBPB->uiEOF && uiNext >= 2) {	/* while not EOF */
		uiCluster = uiNext;				/* set current cluster */
		uiNext = 0;						/* free FAT value */
		iFreeRet = _fat_set_fatn(ptFs, uiCluster, ptBio, &uiNext, iFatNum);
										/* set free */
		if (iFreeRet != 0) {			/* error */
			grp_fs_printf("FAT: failed to free FAT%d 0x%lx\n(dev 0x%x)\n",
						iFatNum, (unsigned long)uiCluster, ptFs->iDev);
			break;						/* break */
		}
		if (iRemain == 0) {				/* final FAT */
			/****************************************************/
			/* set the cluster in free cache list 				*/
			/****************************************************/
			if (ptBPB->iFreeTblCnt < FAT_FREE_TBL) { /* free cache not full */
				ptBPB->iFreeTblCnt++;	/* increment free count */
				ptBPB->auiFreeTbl[ptBPB->iFreePutIdx++] = uiCluster;
										/* set cache */
				if (ptBPB->iFreePutIdx >= FAT_FREE_TBL)	/* over put index */
					ptBPB->iFreePutIdx = 0;	/* lap around put index */
			}
			ptFs->uiFsFreeBlk++;		/* increment free count */
			ptFs->uiFsFreeFile = ptFs->uiFsFreeBlk;/* free file */

			/****************************************************/
			/* invalidate cache buffer							*/
			/****************************************************/
			_fat_invalidate_buf_cache(ptFs, uiCluster);
										/* invalidate buffer cache */
		}
		if (uiCnt && --uiCnt == 0)		/* requested count becomes 0 */
			break;						/* stop free */
	}
	if (iFreeRet != 0)					/* free error */
		return((iRet != 0)? iRet: iFreeRet); /* return error */
	if (uiCnt != 0 || uiNext != ptBPB->uiEOC) { /* in the middle or not EOC */
		grp_fs_printf("FAT: FAT%d invalid next cluster 0x%lx at 0x%lx(dev 0x%x)\n",
					iFatNum, (unsigned long)uiNext, (unsigned long)uiCluster,
					ptFs->iDev);		/* output error */
		iRet = GRP_FS_ERR_FS;			/* set  error */
	}
	return(iRet);						/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_free_cluster_list										*/
/*																			*/
/* DESCRIPTION:	Free cluster list											*/
/* INPUT:		ptFile:				file information						*/
/*				uiStart:			start cluster number					*/
/*				uiCnt:				cluster count to free					*/
/*				uiOffClBlk:			offset block number of start cluster	*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_free_cluster_list(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiStart,			/* [IN]  start cluster number */
	grp_uint32_t	uiCnt,				/* [IN]  cluster count to free */
	grp_uint32_t	uiOffClBlk)			/* [IN]  offset block number of start */
{
	int				iFatNum;			/* FAT number */
	int				iFatCnt;			/* FAT count */
	grp_uint32_t	uiPrev = 0;			/* previous cluster */
	grp_fs_info_t	*ptFs = ptFile->ptFs;/* file system information */
	fat_BPB_t		*ptBPB;				/* BPB information */
	int				iFreeRet;			/* return value of free cluster */
	int				iCleanRet;			/* return value of clean buffer */
	int				iRet = 0;			/* return value */
	grp_fs_bio_t	tBio;				/* buffer I/O information */

	/****************************************************/
	/* set EOC to previous cluster if specified			*/
	/****************************************************/
	ptBPB = ptFs->pvFsInfo;				/* BPB information */
	tBio.ptBuf = NULL;					/* no buffer */
	if (uiOffClBlk) {					/* not first cluster block */
		iRet = _fat_phys_cluster(ptFile, uiOffClBlk - 1, &uiPrev);
										/* get prev */
		if (iRet != 0 && uiPrev == FAT_EOF_CLST)/* EOF */
			uiPrev = 0;					/* clear previous */
	}

	/****************************************************/
	/* free cluster list								*/
	/****************************************************/
	if (ptBPB->iFsType == FAT_TYPE_32
			&& FAT_NOT_MIRRORED(ptBPB)) {	/* single FAT */
		iFatNum = FAT_ACTIVE_NUM(ptBPB);/* active FAT */
		iFatCnt = 1;					/* FAT count */
	} else {							/* mirrored FAT */
		iFatNum = 0;					/* start from 0 */
		iFatCnt = ptBPB->ucFatCnt;		/* FAT count */
	}
	grp_fs_block_fs_mod(ptFs);			/* block modification */
	while (iFatCnt-- > 0) {
		iFreeRet = _fat_free_cluster_list_n(ptFile, uiStart, uiCnt,
							uiPrev, iFatCnt, &tBio, iFatNum++);
		if (iRet == 0 && iFreeRet)		/* error detected */
			iRet = iFreeRet;			/* set error */
	}
	grp_fs_unblock_fs_mod(ptFs);		/* unblock modification */

	/****************************************************/
	/* erase file cluster map							*/
	/*		Erase cluster map for the roll back	of		*/
	/*		directory expanstion in case of error. 		*/
	/*	Note:											*/
	/*		No erasing cluster map may be necessary		*/
	/*		for regular files with file size info.		*/
	/****************************************************/
	if (uiOffClBlk <= ptFile->uiMapFBlk) { /* less than map */
		ptFile->uiMapFBlk = uiOffClBlk;	/* set to free postion */
		ptFile->uiMapCnt = 0;			/* clear map count */
	} else if (uiOffClBlk < ptFile->uiMapFBlk + ptFile->uiMapCnt) {
		ptFile->uiMapCnt = uiOffClBlk - ptFile->uiMapFBlk;
										/* adjust map count */
	}

	/****************************************************/
	/* write back if necessary							*/
	/****************************************************/
	iCleanRet = _fat_clean_unref_buf(&tBio);/* clean buffer */
	if (iRet == 0)						/* operation succeeded */
		iRet = iCleanRet;				/* use return from clean buffer */
	return(iRet);						/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_check_cluster											*/
/*																			*/
/* DESCRIPTION:	check the cluster is allocated							    */
/* INPUT:		ptFs:		FS information									*/
/*				uiClst:		cluster number									*/
/*				ptBio:		buffer I/O information							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int _fat_check_cluster(
	grp_fs_info_t	*ptFs,			/* [IN] FS information */
	grp_uint32_t	uiClst,			/* [IN] cluster number */
	grp_fs_bio_t	*ptBio)			/* [IN] buffer I/O information */
{
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;	/* BPB information */
	grp_uint32_t	uiFatVal;			/* FAT value */
	int				iRet;				/* return value */

	if (uiClst < 2) {					/* less than regular number */
		if (uiClst != 0 || ptBPB->iFsType == FAT_TYPE_32) /* not 12/16 root */
			return(GRP_FS_ERR_FS);		/* return error */
		return(0);						/* return success */
	} else if (uiClst >= ptBPB->uiMaxClst)	/* over max cluster number */
			return(GRP_FS_ERR_FS);		/* return error */
	iRet = _fat_get_fat(ptFs, uiClst, ptBio, &uiFatVal);/* get FAT value */
	if (iRet != 0)						/* I/O error */
		return(iRet);					/* return error */
	if (uiFatVal < 2 ||					/* invalid next cluster */
		(uiFatVal >= ptBPB->uiMaxClst && uiFatVal < ptBPB->uiBadC))
		return(GRP_FS_ERR_FS);			/* return error */
	return(0);							/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_mount													*/
/*																			*/
/* DESCRIPTION:	Mount FAT file system										*/
/* INPUT:		ptFs:		FS information									*/
/*				iMode:		mount mode										*/
/* OUTPUT:		ptFs->pvFsInfo:	FAT dependent info (BPB)					*/
/*				ptFs->uiFsFreeBlk: free block count							*/
/*				ptFs->uiFsFreeFile: free file count							*/
/*				ptFs->uiFsBlkCnt: total block count 						*/
/*				ptFs->uiFsFileCnt: total file count 						*/
/*				ptFs->ucFsFBlkShift: FAT block shift count					*/
/*				ptFs->ucFsDBlkShift: data block shift count					*/
/*				ptFs->ucFsCBlkShift: cluster block shift count				*/
/*				ptFs->uiFsFBlkOff:   FAT block offset						*/
/*				ptFs->uiFsDBlkOff:	 data block offset						*/
/*				ptFs->uiVolSerNo: volume serial number						*/
/*				ptFs->aucVolName: volume name								*/
/*				ptFs->usFsSubType:	sub file system type					*/
/*				ptFs->usVolNameLen: volume name length						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FS:		invalid file system 					*/
/*				GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_NOMEM:	no more memory to allocate FS info		*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_mount(
	grp_fs_info_t	*ptFs,			/* [IN/OUT]  FS information */
	int				iMode)			/* [IN]  mount mode */
{
	fat_BPB_t		tBPB;			/* BPB information */
	fat_fs_info_t 	tHint;			/* free hint */
	grp_int32_t		iRead;			/* read bytes */
	grp_uint32_t	uiFat12_16_Sz;	/* FAT12/16 size */
	grp_fs_bio_t	tFsBio;			/* buffer I/O information */
	fat_conv_tbl_t	*ptConvTbl;		/* conversion table */
	grp_uint32_t	uiMaxFat;		/* max FAT cluster number */
	grp_uint32_t	uiDataSec;		/* start of data sector */
	int				iLen;			/* volume name length */
	grp_int32_t		iRootSecs;		/* sectors in root */
	int				iSecClstShift;	/* sectors in clusters */
	int				iSecDBlkShift;	/* sectors in data block */
	grp_uint32_t	uiOff;			/* offset */
	int				iRet = 0;		/* return value */

	/****************************************************/
	/* init FAT character table							*/
	/****************************************************/
	if (_iFatCharInitDone == 0) 		/* FAT char table is not initialized */
		_fat_init_char_table();			/* initialize FAT char table */

	/****************************************************/
	/* allocate and initialize FAT open info area		*/
	/****************************************************/
	if (grp_fs_fat_open_ctl == NULL) {	/* not allocated yet */
		if (_fat_init_free_open_info() < 0) /* failed to allocate */
			return(GRP_FS_ERR_NOMEM);	/* return error */
	}

	/****************************************************/
	/* read boot parameter block(BPB)					*/
	/****************************************************/
	grp_fs_wait_io(ptFs->iDev, GRP_FS_BUF_FORCE_INV);	/* invalidate cache */
	memset(&tBPB, 0, sizeof(tBPB));						/* clear BPB info */
	if (iMode & GRP_FS_RONLY)							/* read only */
		tBPB.uiStatus |= FAT_STAT_RONLY;				/* set read only bit */
	if (iMode & GRP_FS_SYNC_ALL)						/* sync write always */
		tBPB.uiStatus |= FAT_STAT_SYNC_ALL;				/* set sync bit */
	tBPB.iFBlkShift = grp_fs_param.ucFBlkShift;			/* default blk shift */
	tBPB.uiFBlkSize  = ((grp_uint32_t)1 << tBPB.iFBlkShift);
														/* default blk size */
	ptFs->ucFsFBlkShift = (grp_uchar_t)tBPB.iFBlkShift;	/* set shift for BPB */
	ptFs->uiFsFBlkOff = 0;								/* set block offset */
	tFsBio.ptBuf = NULL;								/* clear buffer */
	iRead = grp_fs_read_buf(ptFs, 0, GRP_FS_BUF_FILE,
							(grp_int32_t)tBPB.uiFBlkSize, &tFsBio);
														/* read BPB */
	if (iRead < FAT_BLK_SZ) {							/* read error */
		iRet = (int)((iRead < 0)? iRead: GRP_FS_ERR_FS);/* set error value */
		goto err_ret;									/* return error */
	}

	/****************************************************/
	/* convert common part of BPB to canonical one		*/
	/****************************************************/
	if (_fat_get_flds(tFsBio.pucData,
					_fat_BPB_common_conv, &tBPB) < 0) {	/* get fields */
		iRet = GRP_FS_ERR_FS;							/* get error value */
		goto err_ret;									/* return error */
	}
	uiFat12_16_Sz = tBPB.uiFatSz;						/* FAT12/16 FAT size */
	if (tBPB.uiFatSz == 0)								/* FAT32 type */
		_fat_conv_data(tFsBio.pucData, &_fat_BPB32_conv[0], &tBPB);
	if (tBPB.usBPBSig != FAT_BPB_SIG
		|| (iSecClstShift = _fat_count_shift(
										(grp_uint32_t)tBPB.ucSecPerClst)) < 0
		|| (tBPB.iByteSecShift = _fat_count_shift(
										(grp_uint32_t)tBPB.usBytePerSec)) <= 0
		|| tBPB.iByteSecShift > grp_fs_param.ucFBlkShift
		|| tBPB.iByteSecShift > grp_fs_param.ucDBlkShift
		|| tBPB.ucFatCnt < 1
		|| tBPB.uiFatSz < 1) {							/* invalid BPB */
		iRet = GRP_FS_ERR_FS;							/* set error value */
		goto err_ret;									/* return error */
	}
	tBPB.iClstShift = iSecClstShift + tBPB.iByteSecShift;/* cluster shift */
	tBPB.uiClstSize = ((grp_uint32_t)1 << tBPB.iClstShift);	/* cluster size */
														/* set by 32 info */
	iRootSecs = FAT_NSEC(&tBPB,
			(grp_int32_t)tBPB.usRootEntCnt * sizeof(fat_dir_t));
														/* # of secs for root */
	uiDataSec = tBPB.usRsvSecCnt						/* data start = rsv */
					+ tBPB.ucFatCnt * tBPB.uiFatSz;		/*		+ FAT area	*/
	if (uiDataSec + iRootSecs >= tBPB.uiTotalSec) {		/* no data sectors */
		iRet = GRP_FS_ERR_FS;							/* set error value */
		goto err_ret;									/* return error */
	}
	tBPB.uiMaxClst = ((tBPB.uiTotalSec - (uiDataSec + iRootSecs))
						>> iSecClstShift) + 2;	/* set max cluster */
	tBPB.uiFatStart = ((grp_uint32_t)tBPB.usRsvSecCnt << tBPB.iByteSecShift);
														/* set FAT start */
	ptFs->uiFsFreeBlk = FAT_FI_NOHINT;					/* no hint */
	tBPB.uiFreeHint = FAT_FI_NOHINT;					/* no hint */
	tBPB.uiNextFree = FAT_FI_NOHINT;					/* no hint */
	uiMaxFat = (tBPB.uiFatSz << tBPB.iByteSecShift);	/* FAT byte count */
	if (FAT_TYPE_IS_12(&tBPB)) {						/* FAT 12 */
		tBPB.iFsType = FAT_TYPE_12;						/* set FAT type */
		ptFs->usFsSubType = 12;							/* set FAT type */
		uiMaxFat = (uiMaxFat * 3) / 2;					/* se max FAT count */
		tBPB.uiEOC = FAT_EOC_12;						/* end of cluster */
		tBPB.uiEOF = FAT_EOF_12;						/* EOF shreshould */
		tBPB.uiBadC = FAT_BADC_12;						/* bad cluster */
	} else if (FAT_TYPE_IS_16(&tBPB)) {					/* FAT 16 */
		tBPB.iFsType = FAT_TYPE_16;						/* set FAT type */
		ptFs->usFsSubType = 16;							/* set FAT type */
		uiMaxFat = uiMaxFat / 2;						/* se max FAT count */
		tBPB.uiEOC = FAT_EOC_16;						/* end of cluster */
		tBPB.uiEOF = FAT_EOF_16;						/* EOF shreshould */
		tBPB.uiBadC = FAT_BADC_16;						/* bad cluster */
	} else {											/* FAT 32 */
		tBPB.iFsType = FAT_TYPE_32;						/* set FAT type */
		ptFs->usFsSubType = 32;							/* set FAT type */
		uiMaxFat = uiMaxFat / 4;						/* se max FAT count */
		tBPB.uiEOC = FAT_EOC_32;						/* end of cluster */
		tBPB.uiEOF = FAT_EOF_32;						/* EOF shreshould */
		tBPB.uiBadC = FAT_BADC_32;						/* bad cluster */
	}
	if (uiFat12_16_Sz == 0 && tBPB.iFsType != FAT_TYPE_32) {/* bad 32 type */
		iRet = GRP_FS_ERR_FS;							/* set error value */
		goto err_ret;									/* return error */
	}
	if (uiMaxFat < tBPB.uiMaxClst)						/* smaller FAT count */
		tBPB.uiMaxClst = uiMaxFat;						/* set max cluster */
	if (tBPB.iByteSecShift < ptFs->ucDevBlkShift) {		/* dev blk is bigger */
		iRet = GRP_FS_ERR_FS;							/* set error value */
		goto err_ret;									/* return error */
	}
	ptFs->uiFsBlkCnt = tBPB.uiMaxClst - 2;				/* total block count */
	ptFs->uiFsFileCnt = tBPB.uiMaxClst - 2;				/* total file count */

	/****************************************************/
	/* find max possible data block size, and set		*/
	/* block information to grp_fs_info					*/
	/****************************************************/
	for (iSecDBlkShift = 0; iSecDBlkShift < iSecClstShift; iSecDBlkShift++) {
		if (iSecDBlkShift + tBPB.iByteSecShift >= grp_fs_param.ucDBlkShift)
			break;
		if (iRootSecs & 1)								/* odd */
			break;
		iRootSecs >>= 1;								/* shift 1 bit */
	}
	tBPB.iDBlkShift = tBPB.iByteSecShift + iSecDBlkShift;/* data blk shift */
	tBPB.uiDBlkSize = ((grp_uint32_t)1 << tBPB.iDBlkShift);	/* data blk size */
	tBPB.uiDBlkStart = iRootSecs;						/* data blk start */
	tBPB.iDBlkClstShift = tBPB.iClstShift - tBPB.iDBlkShift;/* clst-blk shift */
	ptFs->ucFsDBlkShift = (grp_uchar_t)tBPB.iDBlkShift;	/* set data blk shift */
	ptFs->uiFsDBlkOff = (uiDataSec << tBPB.iByteSecShift);/* set data offset */
	ptFs->ucFsCBlkShift = (grp_uchar_t)tBPB.iClstShift;	/* set cluster shift */
	if (ptFs->uiFsDBlkOff < tBPB.uiFBlkSize) {			/* lap to data */
		tFsBio.uiSize = ptFs->uiFsDBlkOff;				/* reduce size */
		tFsBio.ptBuf->iSize = (grp_int32_t)ptFs->uiFsDBlkOff;/* reduce size */
	}

	/****************************************************/
	/* get FAT12/16 and FAT32 dependent BPB information	*/
	/****************************************************/
	ptConvTbl = (tBPB.iFsType == FAT_TYPE_32)?
				&_fat_BPB32_conv[1]: _fat_BPB12_16_conv;/* select conv table */
	if (_fat_get_flds(tFsBio.pucData, ptConvTbl, &tBPB) < 0) { /* get failed */
		iRet = GRP_FS_ERR_FS;							/* set error value */
		goto err_ret;									/* return error */
	}
	iLen = (sizeof(tBPB.aucVolLab) < sizeof(ptFs->aucVolName))?
		sizeof(tBPB.aucVolLab): sizeof(ptFs->aucVolName);/* volume name len */
	memcpy(ptFs->aucVolName, tBPB.aucVolLab, (grp_size_t)iLen);
														/* set volume name */
	if (iLen < sizeof(ptFs->aucVolName))				/* area is bigger */
		ptFs->aucVolName[iLen] = 0;						/* NULL terminate */
	ptFs->usVolNameLen = (grp_ushort_t)iLen;			/* set name length */
	memcpy(&ptFs->uiVolSerNo, tBPB.aucVolSer, sizeof(ptFs->uiVolSerNo));
														/* set serial number */

	/****************************************************/
	/* set FAT32 dependent information					*/
	/****************************************************/
	ptFs->pvFsInfo = &tBPB;								/* temporary set tBPB */
	if (tBPB.iFsType == FAT_TYPE_32) {
		if (FAT_NOT_MIRRORED(&tBPB))					/* FAT not mirrored */
			tBPB.uiFatStart += ((FAT_ACTIVE_NUM(&tBPB) * tBPB.uiFatSz)
								<< tBPB.iByteSecShift);	/* adjust for active */
		if (!(FAT_SUPPORT_FSVER_CHK(&tBPB))) {			/* check support FS version */
			iRet = GRP_FS_ERR_FS;						/* set error value */
			goto err_ret;								/* return error */
		}
		/****************************************************/
		/* get free hint information						*/
		/****************************************************/
		uiOff = ((grp_uint32_t)tBPB.usFsInfo << tBPB.iByteSecShift);
														/* offet hint info */
		iRet = _fat_get_blk(ptFs, FAT_FBLK(&tBPB, uiOff), GRP_FS_BUF_FILE,
							&tFsBio);					/* hint info */
		if (iRet != 0)									/* read error */
			goto err_ret;								/* return error */
		uiOff = FAT_FBLK_OFF(&tBPB, uiOff);				/* block offset */
		if (tFsBio.uiSize < uiOff + FAT_BLK_SZ) {		/* read error */
			iRet = GRP_FS_ERR_FS;						/* set error value */
			goto err_ret;								/* return error */
		}
		if (_fat_get_flds(&tFsBio.pucData[uiOff], _fat_fsinfo_conv, &tHint) == 0
			&& tHint.uiFiSig1 == FAT_FI_SIG1
			&& tHint.uiFiSig2 == FAT_FI_SIG2
			&& tHint.uiFiSig3 == FAT_FI_SIG3) {			/* valid data */
			if (tHint.uiFiFreeCnt <= tBPB.uiMaxClst - 2) {
				ptFs->uiFsFreeBlk = tHint.uiFiFreeCnt;	/* set free count */
				tBPB.uiFreeHint = tHint.uiFiFreeCnt;	/* set free hint */
			}
			if (tHint.uiFiNextFree < tBPB.uiMaxClst
				&& tHint.uiFiNextFree >= 2)
				tBPB.uiNextFree = tHint.uiFiNextFree;	/* set next free */
		}
	}

	/****************************************************/
	/* check root cluster								*/
	/****************************************************/
	if ((iMode & GRP_FS_FORCE_MOUNT) == 0 &&			/* not force mount */
		(iRet = _fat_check_cluster(ptFs, tBPB.uiRootClst, &tFsBio)) < 0)
		goto err_ret;									/* return  error */

	/****************************************************/
	/* check FAT1 and clear clean shutdown flag			*/
	/****************************************************/
	if (tBPB.iFsType != FAT_TYPE_12) {					/* 16/32 type */
		if ((iMode & (GRP_FS_RONLY|GRP_FS_FORCE_MOUNT|GRP_FS_NO_MNT_FLAG)) == 0)
		{												/* regular mount */
			iRet = _fat_proc_fat1(ptFs, &tFsBio, FAT_FAT1_MOUNT);/* proc FAT1 */
			if (iRet != 0)								/* error occured */
				goto err_ret;							/* return error */
		} else if (iMode & GRP_FS_FORCE_MOUNT) {		/* force mount */
			ptFs->uiFsFreeBlk = FAT_FI_NOHINT;			/* no free hint */
			tBPB.uiNextFree = FAT_FI_NOHINT;			/* no next hint */
		}
	}
	if (tFsBio.ptBuf)									/* buffer exists */
		grp_fs_unref_buf(&tFsBio);						/* release buffer */

	/****************************************************/
	/* get free cache, and set free count				*/
	/****************************************************/
	if ((iRet = _fat_get_free_cache(ptFs)) != 0)		/* get free cache */
		goto err_ret;
	ptFs->uiFsFreeFile = ptFs->uiFsFreeBlk;				/* set free file */

	/****************************************************/
	/* allocate FS information area for BPB and open	*/
	/* file information, and copy BPB and setup open	*/
	/* file free list									*/
	/****************************************************/
	ptFs->pvFsInfo = grp_mem_alloc(sizeof(tBPB));		/* allocate area */
	if (ptFs->pvFsInfo == NULL) {						/* no area */
		iRet = GRP_FS_ERR_NOMEM;						/* set error value */
		goto err_ret;									/* return error */
	}
	memcpy(ptFs->pvFsInfo, &tBPB, sizeof(tBPB));		/* copy BPB */
	ptFs->usStatus |= GRP_FS_STAT_DAY_ACCTIME;			/* set day acc time */
	return(0);											/* return success */

err_ret:
	grp_fs_unref_buf(&tFsBio);							/* release FS buffer */
	grp_fs_wait_io(ptFs->iDev, GRP_FS_BUF_FORCE_INV);	/* invalidate cache */
	return(iRet);										/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_sync													*/
/*																			*/
/* DESCRIPTION:	write back FAT dependent cached data						*/
/* INPUT:		ptFs:		file system information							*/
/*				iMode:		sync mode										*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_sync(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	int				iMode)				/* [IN]  sync mode */
{
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;	/* BPB information */
	grp_uint32_t	uiOff;				/* offset */
	int				iRet;				/* return value */
	fat_fs_info_t	tHint;				/* hint info */
	grp_fs_bio_t	tFsBio;				/* buffer I/O data */

	/****************************************************/
	/* check necessity for write back hint information	*/
	/****************************************************/
	if (ptBPB->iFsType != FAT_TYPE_32 ||		/* not FAT 32 */
		(ptFs->usStatus & GRP_FS_STAT_RONLY) ||	/* read only */
		ptBPB->uiFreeHint == ptFs->uiFsFreeBlk || /* no update */
		!(grp_fat_sync_hint || (iMode & GRP_FS_SYNC_HINT)))
		return(0);								/* return success */

	/****************************************************/
	/* write back hint information						*/
	/****************************************************/
	tHint.uiFiSig1 = FAT_FI_SIG1;				/* signature 1 */
	tHint.uiFiSig2 = FAT_FI_SIG2;				/* signature 2 */
	tHint.uiFiSig3 = FAT_FI_SIG3;				/* signature 2 */
	tHint.uiFiFreeCnt = ptFs->uiFsFreeBlk;		/* set free count */
	tHint.uiFiNextFree = ptBPB->uiNextFree;		/* set next free hint */
	uiOff = ((grp_uint32_t)ptBPB->usFsInfo << ptBPB->iByteSecShift);
												/* offet hint info */
	tFsBio.ptBuf = NULL;						/* clear buffer info */
	iRet = _fat_get_blk(ptFs, FAT_FBLK(ptBPB, uiOff), GRP_FS_BUF_FILE,
					&tFsBio);					/* hint information */
	uiOff = FAT_FBLK_OFF(ptBPB, uiOff);			/* block offset */
	if (iRet == 0) {							/* read success */
		int		iUnrefRet;						/* unref return */
		if (tFsBio.uiSize < uiOff + FAT_BLK_SZ) { /* bad read size */
			iRet = GRP_FS_ERR_FS;				/* bad FS */
		} else {
			grp_fs_block_buf_mod(&tFsBio);		/* block modify */
			_fat_set_flds(&tFsBio.pucData[uiOff],
						_fat_fsinfo_conv, &tHint); /* set hint information */
			grp_fs_unblock_buf_mod(&tFsBio,
						GRP_FS_BSTAT_DIRTY|GRP_FS_BSTAT_TSYNC);
												/* unblock modify */
		}
		iUnrefRet = _fat_clean_unref_buf(&tFsBio);	/* write back */
		if (iRet == 0) {						/* no error upto now */
			if (iUnrefRet == 0)					/* success to write back */
				ptBPB->uiFreeHint = ptFs->uiFsFreeBlk;	/* update hint */
			else								/* failed to write back */
				iRet = iUnrefRet;				/* set error */
		}
	}
	return(iRet);								/* return */
}

/****************************************************************************/
/* FUNCTION:	_fat_unmount												*/
/*																			*/
/* DESCRIPTION:	Unmount FAT file system										*/
/* INPUT:		ptFs:		file system information							*/
/*				iMode:		unmount mode									*/
/* OUTPUT:		ptFs->pvFsInfo: cleared to NULL if seccuess					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_unmount(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	int				iMode)				/* [IN]  unmount mode */
{
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;	/* BPB information */
	int				iRet = 0;				/* return value */
	grp_fs_bio_t	tFsBio;				/* buffer I/O data */

	/****************************************************/
	/* set clean shutdown flag							*/
	/****************************************************/
	if ((iMode & GRP_FS_REVOKE_MOUNT) == 0			/* not revoke */
		&& (ptFs->usStatus & GRP_FS_STAT_RONLY) == 0) {	/* not read only */
		if (ptBPB->iFsType == FAT_TYPE_32)			/* FAT 32 */
			iRet = _fat_sync(ptFs, GRP_FS_SYNC_HINT);/* write back hint info */
		tFsBio.ptBuf = NULL;						/* clear buffer info */
		if (iRet == 0 && (ptFs->usStatus & GRP_FS_STAT_NO_MNT_FLAG) == 0
			&& ptBPB->iFsType != FAT_TYPE_12)		/* FAT16/32 */
			iRet = _fat_proc_fat1(ptFs, &tFsBio, FAT_FAT1_UMOUNT);
													/* proc FAT1 */
		if (tFsBio.ptBuf) {							/* buffer still exists */
			int	iUnrefRet = _fat_clean_unref_buf(&tFsBio);	/* write back */
			if (iRet == 0)							/* no error upto now */
				iRet = iUnrefRet;					/* set unref return */
		}
		if (iRet != 0 && (iMode & GRP_FS_FORCE_UMOUNT) == 0)/* no force error */
			return(iRet);							/* return error */
	}
	grp_mem_free(ptBPB);							/* free BPB info area */
	ptFs->pvFsInfo = NULL;							/* clear FsInfo */
	return(0);
}

#ifdef	GRP_FS_FAT_DIRECT_IO
/****************************************************************************/
/* FUNCTION:	_fat_count_cont_blk											*/
/*																			*/
/* DESCRIPTION:	Count contiguous block										*/
/* INPUT:		ptFile:				file information						*/
/*				uiFsBlk:			file offset block number				*/
/*				iSize:				I/O size								*/
/*				iOp:				operation mode							*/
/*										GRP_FS_IO_READ:  read operation		*/
/*										GRP_FS_IO_WRITE: write operation	*/
/* OUTPUT:		puiDBlk:			data block number						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				positive			contiguous block count					*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_count_cont_blk(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiFsBlk,			/* [IN]  FS block number */
	grp_isize_t		iSize,				/* [IN]  I/O size */
	int				iOp,				/* [IN]  operation mode */
	grp_uint32_t	*puiDBlk)			/* [OUT] data block number */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	grp_uint32_t	uiClstCnt;				/* cluster count */
	grp_uint32_t	uiDBlk = 0;				/* data block number */
	grp_uint32_t	uiFsEndBlk;				/* end block */
	grp_uint32_t	uiBlkCnt;				/* block count */
	grp_int32_t		iContBlk;				/* continuous block count */
	int				iRet;					/* return value */
	grp_fs_bio_t	tFsBio;					/* buffer I/O info */

	if (FAT_IS_SIZE0_FID(ptFile->uiFid)) {	/* size 0 file */
		*puiDBlk = FAT_EOF_BLK;				/* EOF */
		return(0);							/* no cluster */
	}
	if ((iOp & GRP_FS_IO_WRITE) == 0) {		/* read operation */
#ifdef GRP_FS_ENABLE_OVER_2G
		uiBlkCnt = FAT_DBLK(ptBPB, ptFile->uiSize);/* block count */
#else  /* GRP_FS_ENABLE_OVER_2G */
		uiBlkCnt = FAT_DBLK(ptBPB, ptFile->iSize);/* block count */
#endif /* GRP_FS_ENABLE_OVER_2G */
	} else {								/* write operation */
#ifdef GRP_FS_ENABLE_OVER_2G
		uiClstCnt = FAT_NCLST(ptBPB, ptFile->uiSize);/* cluster count */
#else  /* GRP_FS_ENABLE_OVER_2G */
		uiClstCnt = FAT_NCLST(ptBPB, ptFile->iSize);/* cluster count */
#endif /* GRP_FS_ENABLE_OVER_2G */
		if (uiClstCnt == 0)					/* 0 cluster */
			uiClstCnt++;					/* increment it */
		uiBlkCnt = FAT_CLST_DBLK(ptBPB, uiClstCnt);	/* block count */
	}
	uiFsEndBlk = uiFsBlk + FAT_DBLK(ptBPB, iSize);	/* end block */
	if (uiFsEndBlk > uiBlkCnt)				/* real end is smaller */
		uiFsEndBlk = uiBlkCnt;				/* adjust to real end */
	for (iContBlk = 0; uiFsBlk < uiFsEndBlk; uiFsBlk++, iContBlk++) {
		if (iContBlk == 0 || FAT_DBLK_CLST_OFF(ptBPB, uiFsBlk) == 0) {
			iRet = _fat_phys_blk(ptFile, uiFsBlk, &uiDBlk);
											/* get blk number */
			if (iRet < 0) {					/* error occured */
				if (iContBlk > 0)			/* contiguous blocks exist */
					break;					/* break here */
				return((grp_int32_t)iRet);	/* return error */
			}
			if (iContBlk == 0) {			/* start block */
				if (FAT_EOF_BLK == uiDBlk)	/* EOF */
					return(0);				/* return EOF */
				else						/* not EOF */
					*puiDBlk = uiDBlk;		/* set block number */
			}
			else if (uiDBlk != *puiDBlk + iContBlk) /* not contiguous */
				break;						/* break search */
		} else								/* block inside cluster */
			uiDBlk++;						/* increment block number */
		iRet = grp_fs_lookup_buf(ptFs, uiDBlk, GRP_FS_BUF_DATA,
							(grp_int32_t)ptBPB->uiDBlkSize, &tFsBio);
											/* lookup buffer */
		if (iRet == 0) {					/* found cache */
			if ((iOp & GRP_FS_IO_WRITE) == 0) {	/* read operation */
				grp_fs_unref_buf(&tFsBio);	/* unreference buffer */
				break;						/* break here */
			} else {						/* write operation */
				tFsBio.ptBuf->iDev = -1;	/* clear association */
				grp_fs_unref_buf(&tFsBio);	/* invalidate cache */
			}
		}
	}
	return(iContBlk);						/* return contiguous block count */
}

/****************************************************************************/
/* FUNCTION:	_fat_alloc_cont_blk											*/
/*																			*/
/* DESCRIPTION:	Allocate contiguous block									*/
/* INPUT:		ptFile:				file information						*/
/*				uiFsBlk:			file offset block number				*/
/*				iSize:				I/O size								*/
/* OUTPUT:		puiDBlk:			data block number						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				positive			contiguous block count					*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_alloc_cont_blk(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiFsBlk,			/* [IN]  FS block number */
	grp_isize_t		iSize,				/* [IN]  I/O size */
	grp_uint32_t	*puiDBlk)			/* [OUT] data block number */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	grp_uint32_t	uiClst;					/* cluster number */
	grp_uint32_t	uiClstCnt;				/* cluster count */
	grp_uint32_t	uiDClst;				/* data cluster number */
	grp_uint32_t	uiDBlk = 0;				/* data block number */
	grp_uint32_t	uiFsEndBlk;				/* end block */
	grp_int32_t		iContBlk;				/* continuous block count */
	grp_int32_t		iAlloc;					/* allocated cluster count */
	grp_fs_bio_t	tFsBio;					/* buffer I/O info */

	/****************************************************/
	/* allocate new free clusters						*/
	/****************************************************/
	uiClst = FAT_DBLK_CLST(ptBPB, uiFsBlk);	/* cluster number */
	uiFsEndBlk = uiFsBlk + FAT_DBLK(ptBPB, iSize);	/* end block */
	uiClstCnt = FAT_DBLK_NCLST(ptBPB, uiFsEndBlk);	/* cluster count */
	iAlloc = _fat_get_free_cluster(ptFs, ptFile, uiClst,
							(grp_int32_t)(uiClstCnt - uiClst), 0,
							&uiDClst);		/* allocate contiguous clusters */
	if (iAlloc < 0)							/* error */
		return(iAlloc);						/* return error */
	uiDBlk = FAT_PHYS_CLST_DBLK(ptBPB, uiDClst, 0);	/* data block number */
	iContBlk = FAT_CLST_DBLK(ptBPB, iAlloc);/* block count */
	if (uiFsBlk + iContBlk < uiFsEndBlk)	/* contiguous block is smaller */
		uiFsEndBlk = uiFsBlk + iContBlk;	/* set end block */
	else									/* end block is smaller */
		iContBlk = uiFsEndBlk -  uiFsBlk;	/* set contiguous block count */
	*puiDBlk = uiDBlk;						/* set data block number */
	for ( ; uiFsBlk < uiFsEndBlk; uiDBlk++, uiFsBlk++) {
		if (grp_fs_lookup_buf(ptFs, uiDBlk, GRP_FS_BUF_DATA,
							(grp_int32_t)ptBPB->uiDBlkSize, &tFsBio) == 0) {
											/* found cache */
			tFsBio.ptBuf->iDev = -1;		/* clear association */
			grp_fs_unref_buf(&tFsBio);		/* invalidate cache */
		}
	}
	return(iContBlk);						/* return contiguous block count */
}

/****************************************************************************/
/* FUNCTION:	_fat_io_direct												*/
/*																			*/
/* DESCRIPTION:	I/O FAT file directly to/from application buffer			*/
/* INPUT:		ptFile:				file information						*/
/*				uiFsBlk:			file offset block number				*/
/* 				pucBuf:				I/O data/buffer							*/
/*				iSize:				I/O size								*/
/*				iOp:				I/O mode								*/
/*										GRP_FS_IO_READ:  read operation		*/
/*										GRP_FS_IO_WRITE: write operation	*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				0 or positive		size written or read					*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_io_direct(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiFsBlk,			/* [IN]  FS block number */
	grp_uchar_t		*pucBuf,			/* [IN/OUT]  I/O data/buffer */
#ifdef GRP_FS_ENABLE_OVER_2G
	grp_uisize_t	uiSize,				/* [IN]  I/O size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	grp_isize_t		iSize,				/* [IN]  I/O size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	int				iOp)				/* [IN]  I/O mode */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	grp_uint32_t	uiClstCnt;				/* cluster count */
	grp_uint32_t	uiClst;					/* cluster offset number */
	grp_uint32_t	uiRealCnt;				/* real cluster count */
	grp_uint32_t	uiDClst;				/* data cluster number */
	grp_uint32_t	uiDBlk;					/* data block number */
	grp_int32_t		iContBlk;				/* continuous block count */
#ifdef GRP_FS_ENABLE_OVER_2G
	int				iDevBlkShift;			/*0xffff8 device block shift */
#else  /* GRP_FS_ENABLE_OVER_2G */
	int				iDevBlkShift;			/* device block shift */
#endif /* GRP_FS_ENABLE_OVER_2G */
	grp_int32_t		iIoCnt;					/* I/O count */

#ifdef GRP_FS_ENABLE_OVER_2G
	uiClstCnt = FAT_NCLST(ptBPB, ptFile->uiSize);/* cluster count */
#else  /* GRP_FS_ENABLE_OVER_2G */
	uiClstCnt = FAT_NCLST(ptBPB, ptFile->iSize);/* cluster count */
#endif /* GRP_FS_ENABLE_OVER_2G */
	uiClst = FAT_DBLK_CLST(ptBPB, uiFsBlk);	/* start cluster */
	iContBlk = (uiClst < uiClstCnt)?
#ifdef GRP_FS_ENABLE_OVER_2G
		_fat_count_cont_blk(ptFile, uiFsBlk, uiSize, iOp, &uiDBlk):
											/* count contiguous blks */
		_fat_alloc_cont_blk(ptFile, uiFsBlk, uiSize, &uiDBlk);
											/* alloc contiguous blks */
#else  /* GRP_FS_ENABLE_OVER_2G */
		_fat_count_cont_blk(ptFile, uiFsBlk, iSize, iOp, &uiDBlk):
											/* count contiguous blks */
		_fat_alloc_cont_blk(ptFile, uiFsBlk, iSize, &uiDBlk);
											/* alloc contiguous blks */
#endif /* GRP_FS_ENABLE_OVER_2G */
	if (iContBlk <= 0)						/* error or in cache */
		return(iContBlk);					/* return */
	iDevBlkShift = (ptBPB->iDBlkShift - (int)ptFs->ucDevBlkShift);
											/* device block shift count */
	iIoCnt = grp_fs_exec_dev_io(ptFs, uiDBlk, pucBuf,
							(iContBlk << iDevBlkShift), iOp, GRP_FS_BUF_DATA);
											/* execute I/O */
	if (iOp & GRP_FS_IO_WRITE) {			/* write operation */
		grp_fs_get_current_time(&ptFile->iMTime);	/* set modify time */
		ptFile->iATime = ptFile->iMTime;			/* set access time */
		ptFile->usStatus |= GRP_FS_FSTAT_UPD_BITS;	/* set update flags */
#ifdef GRP_FS_ENABLE_OVER_2G
		uiSize = (uiFsBlk << ptBPB->iDBlkShift)
				 + ((iIoCnt > 0)? (iIoCnt << (int)ptFs->ucDevBlkShift): 0);
													/* end offset */
		if (uiSize > ptFile->uiSize)				/* over file end */
			ptFile->uiSize = uiSize;				/* set file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
		iSize = (uiFsBlk << ptBPB->iDBlkShift)
				+ ((iIoCnt > 0)? (iIoCnt << (int)ptFs->ucDevBlkShift): 0);
													/* end offset */
		if (iSize > ptFile->iSize)					/* over file end */
			ptFile->iSize = iSize;					/* set file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
		if (uiClst >= uiClstCnt && iIoCnt != (iContBlk << iDevBlkShift)) {
			/****************************************************/
			/* partial success: free clusters 					*/
			/****************************************************/
			uiClstCnt = FAT_DBLK_NCLST(ptBPB, uiFsBlk + iContBlk);
													/* allocated clusters */
#ifdef GRP_FS_ENABLE_OVER_2G
			uiRealCnt = FAT_NCLST(ptBPB, uiSize);	/* real cluster count */
#else  /* GRP_FS_ENABLE_OVER_2G */
			uiRealCnt = FAT_NCLST(ptBPB, iSize);	/* real cluster count */
#endif /* GRP_FS_ENABLE_OVER_2G */
			if (uiRealCnt < uiClstCnt) {			/* real one is smaller */
				uiDClst = FAT_PHYS_DBLK_CLST(ptBPB, uiDBlk)
							+ (uiRealCnt - uiClst);	/* to cluster */
				(void)_fat_free_cluster_list(ptFile, uiDClst,
						uiClstCnt - uiRealCnt, uiRealCnt);/* free clusters */
				if (uiRealCnt == 0)					/* size 0 file */
					(void)_fat_free_1st_cluster(ptFile); /* free 1st cluster */
			}
		}
	}
	if (iIoCnt >= 0)								/* data written */
		return((iIoCnt << (int)ptFs->ucDevBlkShift)); /* return byte count */
	return(iIoCnt);									/* return error */
}
#endif

/****************************************************************************/
/* FUNCTION:	_fat_read													*/
/*																			*/
/* DESCRIPTION:	read FAT file												*/
/* INPUT:		ptFile:				file information						*/
/*				uiFsBlk:			file offset block number				*/
/*				uiBlkOff:			FS block offset							*/
/*				iSize:				read size								*/
/*				iMode:				I/O mode(GRP_FS_OPEN_DIRECT_IO)			*/
/* OUTPUT:		pucBuf:				read data								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_BAD_PARAM: invalid area specified by pucBuf		*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0 or positive:		size read								*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_read(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiFsBlk,			/* [IN]  FS block number */
	grp_uint32_t	uiBlkOff,			/* [IN]  FS block offset */
	grp_uchar_t		*pucBuf,			/* [OUT] read buffer */
	grp_isize_t		iSize,				/* [IN]  read size */
	int				iMode)				/* [IN]  I/O mode */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;			/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;		/* BPB information */
	grp_uint32_t	uiDataBlk;						/* data block number */
	grp_uint32_t	uiFendBlk;						/* file end cluster */
	grp_int32_t		iDataSz;						/* data size to read */
	grp_int32_t		iRead;							/* read size */
	int				iRet;							/* return value */
	grp_fs_bio_t	tFsBio;							/* buffer I/O info */

	/****************************************************/
	/* check size and offset 							*/
	/****************************************************/
	grp_fs_set_access_time(ptFile);					/* set access time */
	if (iSize <= 0)									/* 0 size request */
		return(0);									/* return 0 */
	iDataSz = (grp_int32_t)ptBPB->uiDBlkSize;		/* full block size */
	if (ptFile->ucType == GRP_FS_FILE_FILE) {		/* regular file */
		/****************************************************/
		/* check EOF by file size							*/
		/****************************************************/
#ifdef GRP_FS_ENABLE_OVER_2G
		uiFendBlk = FAT_DBLK(ptBPB, ptFile->uiSize); /* file end block number */
#else  /* GRP_FS_ENABLE_OVER_2G */
		uiFendBlk = FAT_DBLK(ptBPB, ptFile->iSize);	/* file end block number */
#endif /* GRP_FS_ENABLE_OVER_2G */
		if (uiFendBlk < uiFsBlk)					/* over file end */
			return(0);								/* return EOF */
		if (uiFendBlk == uiFsBlk) {					/* end file cluster */
#ifdef GRP_FS_ENABLE_OVER_2G
			iDataSz = (grp_int32_t)(ptFile->uiSize & (iDataSz - 1));
													/* adjust to end */
#else  /* GRP_FS_ENABLE_OVER_2G */
			iDataSz = (grp_int32_t)(ptFile->iSize & (iDataSz - 1));
													/* adjust to end */
#endif /* GRP_FS_ENABLE_OVER_2G */
			if ((grp_int32_t)uiBlkOff >= iDataSz)	/* over file end */
				return(0);							/* return EOF */
		}
	}

#ifdef	GRP_FS_FAT_DIRECT_IO
	/****************************************************/
	/* do direct I/O if specified and possible			*/
	/****************************************************/
	if ((iMode & GRP_FS_OPEN_DIRECT_IO)				/* direct I/O mode */
		&& (uiBlkOff == 0
			&& iDataSz == (grp_int32_t)ptBPB->uiDBlkSize
			&& iSize >= iDataSz)) {					/* direct condition match */
		iRead = _fat_io_direct(ptFile, uiFsBlk, pucBuf, iSize, GRP_FS_IO_READ);
													/* do direct I/O */
		if (iRead != 0)								/* success or error */
			return(iRead);							/* return */
	}
#endif

	/****************************************************/
	/* get physical data block number					*/
	/****************************************************/
	iRet = _fat_phys_blk(ptFile, uiFsBlk, &uiDataBlk);
	if (iRet < 0)									/* error occured */
		return((grp_int32_t)iRet);					/* return error */
	if (uiDataBlk == FAT_EOF_BLK){					/* EOF */
		if (ptFile->ucType == GRP_FS_FILE_FILE)		/* regular file */
			return(GRP_FS_ERR_FS);					/* error */
		else
			return(0);								/* return EOF */
	}

	/****************************************************/
	/* get cluster data in buffer						*/
	/****************************************************/
	iRead = grp_fs_read_buf(ptFs, uiDataBlk, GRP_FS_BUF_DATA,
						(grp_int32_t)ptBPB->uiDBlkSize,  &tFsBio);
													/* read data */
	if (iRead < 0)									/* error */
		return(iRead);								/* return error */

	/****************************************************/
	/* copy data to buffer								*/
	/****************************************************/
	if (iRead < iDataSz)							/* read size < iDataSz */
		iDataSz = iRead;								/* adjust it */
	iDataSz -= uiBlkOff;							/* subtract offset */
	if (iDataSz <= 0) {								/* actual data is smaller */
		iRead = 0;									/* EOF */
	} else {										/* data exists */
		if (iDataSz > iSize)						/* requested is smaller */
			iDataSz = iSize;						/* use requested */
		iRead = grp_fs_copyout(pucBuf, tFsBio.pucData + uiBlkOff, iDataSz);	
													/* copy out data */
	}
	grp_fs_unref_buf(&tFsBio);						/* release buffer */
	return(iRead);
}

/****************************************************************************/
/* FUNCTION:	_fat_write													*/
/*																			*/
/* DESCRIPTION:	Write FAT file												*/
/* INPUT:		ptFile:				file information						*/
/*				uiFsBlk:			file offset block number				*/
/*				uiBlkOff:			FS block offset							*/
/* 				pucBuf:				write data								*/
/*				iSize:				write size								*/
/*				iMode:				I/O mode(GRP_FS_OPEN_DIRECT_IO)			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_BAD_OFF:	bad offset								*/
/*				GRP_FS_ERR_BAD_PARAM: invalid area specified by pucBuf		*/
/*				GRP_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0 or positive		size written							*/
/*				others				error number set by device driver		*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_write(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiFsBlk,			/* [IN]  FS block number */
	grp_uint32_t	uiBlkOff,			/* [IN]  FS block offset */
	grp_uchar_t		*pucBuf,			/* [IN]  write data */
	grp_isize_t		iSize,				/* [IN]  write size */
	int				iMode)				/* [IN]  I/O mode */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;			/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;		/* BPB information */
	grp_uint32_t	uiDataBlk;						/* data block number */
	grp_uint32_t	uiDataClst = 0;					/* data cluster number */
	grp_uint32_t	uiFendBlk;						/* file end block number */
	grp_uint32_t	uiFendOff;						/* file end block offset */
	grp_int32_t		iDataSz;						/* data size to write */
	grp_int32_t		iInitSz = 0;					/* size to initailzed */
	grp_int32_t		iWrite;							/* buffer/written size */
	int				iRet;							/* return value */
	grp_fs_bio_t	tFsBio;							/* buffer I/O info */

	/****************************************************/
	/* check write access to directory					*/
	/****************************************************/
	if ((ptFile->ucType == GRP_FS_FILE_DIR)			/* directory file */
		|| (ptFs->usStatus & GRP_FS_STAT_RONLY))	/* read only file system */
		return(GRP_FS_ERR_PERMIT);					/* permission denied */

	/****************************************************/
	/* check size and offset							*/
	/****************************************************/
	if (iSize <= 0)									/* 0 size request */
		return(0);									/* return 0 */
	grp_fs_block_file_write(ptFile);				/* block write */
	grp_fs_set_access_time(ptFile);					/* set access time */
	iDataSz = (grp_int32_t)ptBPB->uiDBlkSize;		/* full block size */
#ifdef GRP_FS_ENABLE_OVER_2G
	uiFendBlk = FAT_DBLK(ptBPB, ptFile->uiSize);	/* file end block number */
	uiFendOff = FAT_DBLK_OFF(ptBPB, ptFile->uiSize); /* file end block offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
	uiFendBlk = FAT_DBLK(ptBPB, ptFile->iSize);		/* file end block number */
	uiFendOff = FAT_DBLK_OFF(ptBPB, ptFile->iSize);	/* file end block offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
	if (uiFendBlk < uiFsBlk
		|| (uiFendBlk == uiFsBlk && uiBlkOff > uiFendOff)) { /* over file end */
		iWrite = GRP_FS_ERR_BAD_OFF;				/* bad offset */
		goto err_out2;								/* return error */
	}

#ifdef	GRP_FS_FAT_DIRECT_IO
	/****************************************************/
	/* do direct I/O if specified and possible			*/
	/****************************************************/
	if ((iMode & GRP_FS_OPEN_DIRECT_IO)				/* direct I/O mode */
		&& (uiBlkOff == 0 && iSize >= iDataSz)) {	/* possible request */
		iWrite = _fat_io_direct(ptFile, uiFsBlk, pucBuf, iSize,
								GRP_FS_IO_WRITE);	/* do direct I/O */
#ifdef GRP_FS_UPDATE_ARCHIVE
		/****************************************************/
		/* added archive attribute							*/
		/****************************************************/
		ptFile->uiAttr |= FAT_ATTR_ARCHIVE;			/* set archive */
		ptFile->usStatus |= GRP_FS_FSTAT_UPD_ATTR;	/* set update flag */
#endif /* GRP_FS_UPDATE_ARCHIVE */
		if ((ptFile->usStatus & GRP_FS_FSTAT_UPD_BITS) && /* updated */
			(ptFs->usStatus & GRP_FS_STAT_SYNC_ALL))/* sync all */
			(void)_fat_update_attr(ptFile);			/* update file attribute */
		grp_fs_unblock_file_write(ptFile);			/* unblock write */

		if(!iWrite) {								/* 0 byte write */
			iRet = _fat_phys_blk(ptFile, uiFsBlk, &uiDataBlk);
			if (uiDataBlk == FAT_EOF_BLK){			/* EOF */
				if (ptFile->ucType == GRP_FS_FILE_FILE)		/* regular file */
					iWrite = GRP_FS_ERR_FS;			/* error */
			}
		}
		
		return(iWrite);								/* return */
	}
#endif

	/****************************************************/
	/* convert to physical data block number			*/
	/****************************************************/
	if (uiFendBlk > uiFsBlk							/* less than end block */
#ifdef GRP_FS_ENABLE_OVER_2G
		|| FAT_CLST_OFF(ptBPB, ptFile->uiSize) != 0	/* not cluster alignment */
#else  /* GRP_FS_ENABLE_OVER_2G */
		|| FAT_CLST_OFF(ptBPB, ptFile->iSize) != 0	/* not cluster alignment */
#endif /* GRP_FS_ENABLE_OVER_2G */
		|| (!FAT_IS_SIZE0_FID(ptFile->uiFid)		/* not size 0 file id */
			 && uiFendBlk == 0)) {					/* size 0 block allocated */
		/****************************************************/
		/* get physical data block number					*/
		/****************************************************/
		iRet = _fat_phys_blk(ptFile, uiFsBlk, &uiDataBlk);/* get blk number */
		if (iRet < 0) {								/* error occured */
			iWrite = (grp_int32_t)iRet;				/* set error */
			goto err_out2;							/* return error */
		}
		if (uiDataBlk == FAT_EOF_BLK) {				/* EOF */
			iWrite = GRP_FS_ERR_FS;					/* bad file system */
			goto err_out2;							/* return error */
		}
	} else {										/* new cluster */
		/****************************************************/
		/* allocate new free cluster						*/
		/****************************************************/
		grp_int32_t iCnt = _fat_get_free_cluster(ptFs, ptFile,
						 FAT_DBLK_CLST(ptBPB, uiFendBlk), 1, 0,
									&uiDataClst);	/* allocate */
		grp_fs_get_current_time(&ptFile->iMTime);	/* set modify time */
		ptFile->iATime = ptFile->iMTime;			/* set access time */
		ptFile->usStatus |= (GRP_FS_FSTAT_UPD_ATIME|GRP_FS_FSTAT_UPD_MTIME);
													/* set update flags */
		if (iCnt < 0) {								/* error occured */
			iWrite = iCnt;							/* set error */
			goto err_out2;							/* return error */
		}
		uiDataBlk = FAT_PHYS_CLST_DBLK(ptBPB, uiDataClst, 0);
													/* convert to block */
	}

	/****************************************************/
	/* get cluster data in buffer or clear 0			*/
	/****************************************************/
	tFsBio.ptBuf = NULL;							/* clear buffer */
	if (uiBlkOff != 0
		|| (iSize < iDataSz  
#ifdef GRP_FS_ENABLE_OVER_2G
		    && (uiFendBlk > uiFsBlk
				|| (grp_uint32_t)iSize < uiFendOff))) {/* partial write */
#else  /* GRP_FS_ENABLE_OVER_2G */
		    && (uiFendBlk > uiFsBlk || iSize < uiFendOff))) {/* partial write */
#endif /* GRP_FS_ENABLE_OVER_2G */
		/****************************************************/
		/* read cluster data in buffer						*/
		/****************************************************/
		iWrite = grp_fs_read_buf(ptFs, uiDataBlk, GRP_FS_BUF_DATA,
						 (grp_int32_t)ptBPB->uiDBlkSize, &tFsBio);
													/* read data */
		if (iWrite < 0)								/* error */
			goto err_out;							/* return error */
	} else {										/* new block */
		/****************************************************/
		/* allocate and clear new block						*/
		/****************************************************/
		iRet = grp_fs_lookup_buf(ptFs, uiDataBlk,
							GRP_FS_BUF_DATA|GRP_FS_BUF_ALLOC,
							(grp_int32_t)ptBPB->uiDBlkSize, &tFsBio);
													/* allocate a block */
		if (iRet != 0 && iRet != GRP_FS_ERR_NOT_FOUND) { /* error occured */
			iWrite = (grp_int32_t)iRet;				/* set error number */
			goto err_out;							/* return error */
		}
		iWrite = (grp_int32_t)tFsBio.uiSize;		/* set size */
		iInitSz = iWrite;							/* size to init */
	}

	/****************************************************/
	/* copy data to buffer								*/
	/****************************************************/
	if (iWrite < iDataSz)							/* buf size < iDataSz */
		iDataSz = iWrite;							/* adjust it */
	iDataSz -= uiBlkOff;							/* subtract offset */
	if (iDataSz <= 0) {								/* actual data is smaller */
		iWrite = 0;									/* EOF */
	} else {
		if (iDataSz > iSize)						/* requested is smaller */
			iDataSz = iSize;						/* use requested */
		grp_fs_block_buf_mod(&tFsBio);				/* block modification */
		iWrite = grp_fs_copyin(tFsBio.pucData + uiBlkOff, pucBuf, iDataSz);
													/* get and write to buf */
		if (iWrite >= 0 && iInitSz > iWrite)		/* need to initialize */
			memset(&tFsBio.pucData[iWrite], 0, (grp_size_t)(iInitSz - iWrite));
													/* clear rest of area */
		grp_fs_unblock_buf_mod(&tFsBio, GRP_FS_BSTAT_DIRTY);/* unblock modify */
		grp_fs_get_current_time(&ptFile->iMTime);	/* set modify time */

#ifdef GRP_FS_UPDATE_ARCHIVE
		/****************************************************/
		/* added archive attribute							*/
		/****************************************************/
		ptFile->uiAttr |= FAT_ATTR_ARCHIVE;			/* set archive */
		ptFile->usStatus |= GRP_FS_FSTAT_UPD_ATTR;	/* set update flag */
#endif /* GRP_FS_UPDATE_ARCHIVE */

		ptFile->iATime = ptFile->iMTime;			/* set access time */
		ptFile->usStatus |= GRP_FS_FSTAT_UPD_BITS;	/* set update flags */
		if (iWrite >= 0) {							/* copy succeeded */
			uiBlkOff += iWrite;						/* new offset */
			if (uiFendBlk == uiFsBlk && uiBlkOff > uiFendOff) /* grow */
#ifdef GRP_FS_ENABLE_OVER_2G
				ptFile->uiSize = (uiFendBlk << ptFs->ucFsDBlkShift) + uiBlkOff;
#else  /* GRP_FS_ENABLE_OVER_2G */
				ptFile->iSize = (uiFendBlk << ptFs->ucFsDBlkShift) + uiBlkOff;
#endif /* GRP_FS_ENABLE_OVER_2G */
		} else										/* copy failed */
			goto err_out;							/* return error */
	}
	grp_fs_buf_fill_end(&tFsBio, 0);				/* end fill */
	iRet = _fat_clean_unref_buf(&tFsBio);			/* write back */
	if (iRet < 0)									/* write back failed */
		iWrite = (grp_int32_t)iRet;					/* set error number */
	if ((ptFile->usStatus & GRP_FS_FSTAT_UPD_BITS) && /* updated */
		(ptFs->usStatus & GRP_FS_STAT_SYNC_ALL))	/* sync all */
		(void)_fat_update_attr(ptFile);				/* update file attribute */
	grp_fs_unblock_file_write(ptFile);				/* unblock write */
	return(iWrite);									/* return result */

err_out:
	grp_fs_buf_fill_end(&tFsBio, 1);				/* end fill with error */
#ifdef GRP_FS_ENABLE_OVER_2G
	if (uiFendBlk == uiFsBlk && FAT_CLST_OFF(ptBPB, ptFile->uiSize) == 0) {
#else  /* GRP_FS_ENABLE_OVER_2G */
	if (uiFendBlk == uiFsBlk && FAT_CLST_OFF(ptBPB, ptFile->iSize) == 0) {
#endif /* GRP_FS_ENABLE_OVER_2G */
		(void)_fat_free_cluster_list(ptFile, uiDataClst, 1,
				 FAT_DBLK_CLST(ptBPB, uiFendBlk));	/* free it */
		if (uiFendBlk == 0)							/* size 0 file */
			(void)_fat_free_1st_cluster(ptFile);	/* free 1st cluster */
	}
	(void)_fat_clean_unref_buf(&tFsBio);			/* release buffer */
err_out2:
	if ((ptFile->usStatus & GRP_FS_FSTAT_UPD_BITS) && /* updated */
		(ptFs->usStatus & GRP_FS_STAT_SYNC_ALL))	/* sync all */
		(void)_fat_update_attr(ptFile);				/* update file attribute */
	grp_fs_unblock_file_write(ptFile);				/* unblock write */
	return(iWrite);									/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_conv_char												*/
/*																			*/
/* DESCRIPTION:	Convert character for short name entry						*/
/* INPUT:		pucSrc:		source string									*/
/*				pucDstEnd:	end of destination string						*/
/* OUTPUT:		pucDst:		converted string								*/
/*				piFnType:	file name type									*/
/*																			*/
/* RESULT:		positive:	character byte count							*/
/*				0:			end of string									*/
/*				-1:			invalid character								*/
/*																			*/
/****************************************************************************/
static int
_fat_conv_char(
	const grp_uchar_t	*pucSrc,				/* [IN]  src string pointer */
	grp_uchar_t			*pucDst,				/* [OUT] dest string pointer */
	grp_uchar_t			*pucDstEnd,				/* [IN]  end of dest string */
	int					*piFnType)				/* [OUT] file name type */
{
	int					iChVal;					/* character value */
	int					iChByte;				/* character count */

	iChByte = grp_fs_char_cnt(pucSrc);			/* character count */
	if (iChByte == 1) {							/* 1 byte character */
		iChVal = *pucSrc;						/* get character value */
		if (iChVal >= 'a' && iChVal <= 'z')	{	/* lower case letter */
			*pucDst = (grp_uchar_t)(iChVal - 'a' + 'A');
			*piFnType |= FAT_FNAME_LOWER;		/* include lower case */
		} else if (iChVal < 0x80) {				/* in 7 bit ascii range */
			if (!FAT_IS_VALID_SNAME(iChVal)) {	/* not valid short name */
				*pucDst = '_';					/* convert to '_' */
				*piFnType |= FAT_FNAME_LONG;	/* include non short char */
			} else {							/* valid short name */
				*pucDst = (grp_uchar_t)iChVal;	/* copy it */
			}
		} else {								/* not in 7 bit ascii range */
			*pucDst = (grp_uchar_t)iChVal;		/* copy it */
		}
	} else if (iChByte > 1) {					/* multibyte character */
		if (pucDst + iChByte > pucDstEnd) {		/* overflow */
			memset(pucDst, '_', (grp_size_t)(pucDstEnd - pucDst));
												/* convert to '_' */
			*piFnType |= FAT_FNAME_LONG;		/* include non short char */
		} else
			memcpy(pucDst, pucSrc, (grp_size_t)iChByte); /* copy it */
	}
	return(iChByte);							/* return byte count */
}

/****************************************************************************/
/* FUNCTION:	_fat_make_short_cmp_name									*/
/*																			*/
/* DESCRIPTION:	Make uppercase short name for comparison					*/
/* INPUT:		pucSrc:		source string									*/
/* OUTPUT:		pucDst:		converted string								*/
/*																			*/
/* RESULT:		0:							uppercase short name			*/
/*				FAT_FNAME_LOWER bit set:	include lower case char			*/
/*				FAT_FNAME_LONG  bit set:	include non short char			*/
/*																			*/
/****************************************************************************/
static int
_fat_make_short_cmp_name(
	const grp_uchar_t	*pucSrc,				/* [IN]  string to convert */
	grp_uchar_t			*pucDst)				/* [OUT] converted string */
{
	grp_uchar_t			*pucDstStart = pucDst;	/* start of destination */
	grp_uchar_t			*pucDstEnd;				/* end of destination */
	const grp_uchar_t	*pucDot;				/* dot find pointer */
	const grp_uchar_t	*pucFirstDot;			/* 1st dot position */
	int					iLen;					/* character length */
	int					iRet = 0;				/* return value */

	/****************************************************/
	/* check leading '.'								*/
	/****************************************************/
	if (*pucSrc == '.') {						/* exist leading '.' */
		if (pucSrc[1] == 0						/* "." */
			|| (pucSrc[1] == '.' && pucSrc[2] == 0)) { /* ".." */
			*pucDst++ = '.';					/* set dot */
			if (pucSrc[1])						/* ".." case */
				*pucDst++ = '.';				/* set dot */
			*pucDst++ = 0;						/* null terminate */
			return(0);							/* return 0 */
		}
		goto long_name;							/* long name */
	}

	/****************************************************/
	/* make base name part								*/
	/****************************************************/
	pucDstEnd = &pucDst[FAT_BASE_NAME_LEN];		/* end of destination */
	while (*pucSrc && *pucSrc != '.' && pucDst < pucDstEnd) {
		if (*pucSrc == ' ')						/* space */
			goto long_name;						/* long name */
		iLen = _fat_conv_char(pucSrc, pucDst, pucDstEnd, &iRet); /* convert */
		pucSrc += iLen;							/* advance to next */
		pucDst += iLen;							/* advance to next */
	}

	/****************************************************/
	/* lookup last '.'									*/
	/****************************************************/
	if ((iRet & FAT_FNAME_LONG)					/* long name */
		|| (*pucSrc != '.' && *pucSrc))			/* long base name */
		goto long_name;							/* long name */
	pucDot = pucFirstDot = pucSrc;				/* remember current point */
	while (*pucSrc) {							/* lookup until end */
		if (*pucSrc == '.') {
			pucDot = pucSrc++;					/* remember it */
			continue;							/* advance to next */
		}
		pucSrc += grp_fs_char_cnt(pucSrc);		/* advance character count */
	}
	if (pucDot != pucFirstDot)					/* last dot is not 1st */
		goto long_name;							/* long name */

	/****************************************************/
	/* make suffix part									*/
	/****************************************************/
	if (*pucDot == '.') {						/* dot found */
		*pucDst++ = '.';						/* set separator */
		pucDstEnd = &pucDst[FAT_SUFFIX_NAME_LEN];/* end of destination */
		pucSrc = pucDot + 1;					/* set start point */
		while (*pucSrc && pucDst < pucDstEnd) {
			if (*pucSrc == ' ') 				/* space */
				goto long_name;					/* long name */
			iLen = _fat_conv_char(pucSrc, pucDst, pucDstEnd, &iRet); /* conv */
			pucSrc += iLen;						/* advance to next */
			pucDst += iLen;						/* advance to next */
		}
	}
	if ((iRet & FAT_FNAME_LONG) || *pucSrc)		/* long name */
		goto long_name;							/* long name */
	*pucDst = 0;								/* NULL terminate */
	return(iRet);								/* return conv flag */

long_name:										/* non short name */
	*pucDstStart = 0;							/* no short name */
	return(FAT_FNAME_LONG);						/* return long name type */
}

/****************************************************************************/
/* FUNCTION:	_fat_make_short_ent_name									*/
/*																			*/
/* DESCRIPTION:	Make short entry name										*/
/*				Note: Source string should be guaranteed not to inlclude	*/
/*					  invalid character sequence so that _fat_conv_char		*/
/*					  always return positive value.							*/
/* INPUT:		pucSrc:		source string									*/
/* OUTPUT:		pucDst:		converted string								*/
/*																			*/
/* RESULT:		0:							uppercase short name			*/
/*				FAT_FNAME_LOWER bit set:	include lower case char			*/
/*				FAT_FNAME_LONG  bit set:	include non short char			*/
/*																			*/
/****************************************************************************/
static int
_fat_make_short_ent_name(
	const grp_uchar_t	*pucSrc,				/* [IN]  string to convert */
	grp_uchar_t			*pucDst)				/* [OUT] converted string */
{
	grp_uchar_t			*pucDstEnd;				/* end of destination */
	const grp_uchar_t	*pucDot;				/* dot find pointer */
	const grp_uchar_t	*pucLastDot;			/* last dot position */
	int					iLen;					/* character length */
	int					iRet = 0;				/* return value */

	/****************************************************/
	/* skip leading '.'									*/
	/****************************************************/
	for ( ; *pucSrc == '.'; pucSrc++)			/* loop while '.' char */
		iRet = FAT_FNAME_LONG;					/* set non short name bit */

	/****************************************************/
	/* lookup last '.'									*/
	/****************************************************/
	pucLastDot = NULL;							/* init last dot position */
	pucDot = pucSrc;							/* remember current point */
	while (*pucDot) {							/* lookup until end */
		if (*pucDot == '.') {
			if (pucLastDot) {					/* "." 2nd since then */
				iRet = FAT_FNAME_LONG;			/* set non short name bit */
			}
			pucLastDot = pucDot++;				/* remember it */
			continue;							/* advance to next */
		}
		pucDot += grp_fs_char_cnt(pucDot);		/* advance character count */
	}
	if (pucLastDot && ((pucLastDot - pucSrc) > FAT_BASE_NAME_LEN)) {
												/* base name over */
		iRet = FAT_FNAME_LONG;					/* set non short name bit */
	}

	/****************************************************/
	/* make base name part								*/
	/****************************************************/
	pucDstEnd = &pucDst[FAT_BASE_NAME_LEN];		/* end of destination */
	if (pucLastDot == NULL) {					/* not found "." */
		pucDstEnd = &pucDst[FAT_BASE_NAME_LEN];	/* end of destination */
	} else {
		pucDstEnd = &pucDst[FAT_BASE_NAME_LEN];	/* end of destination */
		if (pucDstEnd > pucLastDot) {
			pucDstEnd = (grp_uchar_t *)pucLastDot; /* set base part end */
		}
	}
	while (*pucSrc && pucDst < pucDstEnd) {
		if ((*pucSrc == ' ')					/* space */
			|| (*pucSrc == '.')) {				/* "." */
			if (pucLastDot && (pucSrc == pucLastDot)) {
				break;							/* last "." */
			}
			pucSrc++;							/* skip char */
			iRet = FAT_FNAME_LONG;				/* set non short name bit */
			continue;							/* skip space */
		}
		iLen = _fat_conv_char(pucSrc, pucDst, pucDstEnd, &iRet); /* convert */
		pucSrc += iLen;							/* advance to next */
		pucDst += iLen;							/* advance to next */
	}
	if (pucDst > pucDstEnd)						/* over */
		pucDst = pucDstEnd;						/* reset to end */

	/****************************************************/
	/* make suffix part									*/
	/****************************************************/
	pucDot = pucLastDot;						/* set last dot pointer */
	if (pucDot && (*pucDot == '.')) {			/* dot found */
		*pucDst++ = '.';						/* set separator */
		pucDstEnd = &pucDst[FAT_SUFFIX_NAME_LEN];/* end of destination */
		pucSrc = pucDot + 1;					/* set start point */
		while (*pucSrc && pucDst < pucDstEnd) {
			if (*pucSrc == ' ') {				/* space */
				pucSrc++;						/* skip char */
				iRet = FAT_FNAME_LONG;			/* set non short name bit */
				continue;						/* skip space */
			}
			iLen = _fat_conv_char(pucSrc, pucDst, pucDstEnd, &iRet); /* conv */
			pucSrc += iLen;						/* advance to next */
			pucDst += iLen;						/* advance to next */
		}
		if (pucDst > pucDstEnd)					/* over end */
			pucDst = pucDstEnd;					/* reset to end */
	}
	*pucDst = 0;								/* NULL terminate */
	if (*pucSrc)								/* data remains */
		iRet = FAT_FNAME_LONG;					/* set non short name bit */
	return(iRet);								/* return conv flag */
}

/****************************************************************************/
/* FUNCTION:	_fat_make_long_name											*/
/*																			*/
/* DESCRIPTION:	Make long name with truncating trailing space or '.'		*/
/* INPUT:		pucStr:		source string									*/
/* OUTPUT:		pucStr:		truncated string								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_NAME:	include bad character				*/
/*				positive value: character count in pucStr (not byte count)	*/
/*																			*/
/****************************************************************************/
static int
_fat_make_long_name(
	grp_uchar_t	*pucStr)					/* [IN]  source string */
{
	int			iCharCnt = 0;				/* total character count */
	int			iCharByte;					/* character count */
	int			iChVal;						/* character value */
	grp_uchar_t	*pucTrimPoint = NULL;		/* trim point */

	/****************************************************/
	/* count characters									*/
	/****************************************************/
	while (*pucStr) {						/* loop until end of string */
		iCharByte = grp_fs_char_cnt(pucStr); /* get character count */
		if (iCharByte == 1) {				/* 1 byte character */
			iChVal = *pucStr;				/* get character value */
			if (iChVal < 0x80				/* 7 bit ascii range */
				&& !FAT_IS_VALID_LNAME(iChVal)) /* not valid long name */
				return(GRP_FS_ERR_BAD_NAME); /* return bad name error */
			if (iChVal == ' ' || iChVal == '.') { /* ' ' or '.' */
				if (pucTrimPoint == NULL)	/* trim point is not set */
					pucTrimPoint = pucStr;	/* remember it */
			} else
				pucTrimPoint = NULL;		/* reset trim point */
		} else if (iCharByte > 1) {			/* multibyte */
			pucTrimPoint = NULL;
		} else {							/* invalid character */
			return(GRP_FS_ERR_BAD_NAME);	/* return bad name error */
		}
		pucStr += iCharByte;				/* advance to next */
		iCharCnt++;							/* increment count */
	}

	/****************************************************/
	/* truncate trailing ' ' or '.'						*/
	/****************************************************/
	if (pucTrimPoint) {						/* trim point exists */
		iCharCnt -= (int)(pucStr - pucTrimPoint); /* decrement char count */
		*pucTrimPoint = 0;					/* trim it */
	}
	if (iCharCnt == 0)						/* 0 length name */
		return(GRP_FS_ERR_BAD_NAME);		/* return bad name error */
	return(iCharCnt);						/* return character count */
}

/****************************************************************************/
/* FUNCTION:	_fat_copy_short_file_name									*/
/*																			*/
/* DESCRIPTION:	Copy short file name until end of source or space			*/
/* INPUT:		pucDst:		destination buffer								*/
/*				pucSrc:		source string									*/
/*				iSrcLen:	source length									*/
/*				ucDelim:	delimiter character								*/
/* OUTPUT:		pucDst:		copied string									*/
/*																			*/
/* RESULT:		length of copied											*/
/*																			*/
/****************************************************************************/
static int
_fat_copy_short_file_name(
	grp_uchar_t		*pucDst,				/* [OUT] destination buffer */
	grp_uchar_t		*pucSrc,				/* [IN]  source string */
	int				iSrcSize,				/* [IN]  source length */
	grp_uchar_t		ucDelim)				/* [IN]  delimiter */
{
	grp_uchar_t		*pucSrcStart = pucSrc;	/* start of source */
	grp_uchar_t		*pucSrcEnd;				/* end of source */
	int				iCharByte;				/* character byte */
	int				iLen;					/* copy length */

	pucSrcEnd = &pucSrc[iSrcSize];			/* end of source */
	while (pucSrc < pucSrcEnd && *pucSrc && *pucSrc != ucDelim) {
		iCharByte = grp_fs_char_cnt(pucSrc); /* get character count */
		if (iCharByte <= 0					/* invalid character */
			|| iCharByte + pucSrc > pucSrcEnd)	/* out of source */
			break;
		pucSrc += iCharByte;				/* advance to next */
	}
	iLen = (int)(pucSrc - pucSrcStart);		/* copy length */
	memcpy(pucDst, pucSrcStart, (grp_size_t)iLen); /* copy it */
	return(iLen);							/* return copy length */
}

/****************************************************************************/
/* FUNCTION:	_fat_copy_long_name											*/
/*																			*/
/* DESCRIPTION:	Copy long name to directory entry information				*/
/* INPUT:		ptLDir:				long directory information				*/
/* 				ptDirEnt:			directory entry information				*/
/*				piUsed:				used length in name buffer				*/
/* OUTPUT:		ptDirEnt:			directory entry information				*/
/*				piUsed:				used length in name buffer				*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_copy_long_name(
	fat_long_dir_t	*ptLDir,				/* [IN]  long directory entry */
	grp_fs_dir_ent_t *ptDirEnt,				/* [IN/OUT]  directory info */
	int				*piUsed)				/* [IN/OUT]  used name size */
{
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	grp_uchar_t		*pucSrc;				/* source pointer */
	grp_uchar_t		*pucSrcEnd;				/* end of source */
	grp_uchar_t		*pucDst;				/* destination pointer */
	grp_uint32_t	uiCode;					/* character code */
	int				iCharByte;				/* byte count of character */
	int				iLen;					/* length of name */
	int				iSize;					/* copy size */
	struct name_info {						/* name component information */
		grp_uchar_t	*pucComp;				/* start of component */
		int			iCompLen;				/* component length */
	} tNameInfo[3];							/* name component information */
	struct name_info *ptNameInfo = tNameInfo;/* name component information */
	grp_uchar_t		aucOEM[FAT_LNAME_CNT * 4 + 1]; /* OEM long name */

	/****************************************************/
	/*	set name component information					*/
	/****************************************************/
	pucSrc = ptLDir->aucName1;							/* name 1 */
	pucSrcEnd = &pucSrc[sizeof(ptLDir->aucName1)];		/* name 1 */
	tNameInfo[0].pucComp = ptLDir->aucName2;			/* name 2 */
	tNameInfo[0].iCompLen = sizeof(ptLDir->aucName2);	/* name 2 */
	tNameInfo[1].pucComp = ptLDir->aucName3;			/* name 3 */
	tNameInfo[1].iCompLen = sizeof(ptLDir->aucName3);	/* name 3 */
	tNameInfo[2].pucComp = NULL;						/* end of component */
	tNameInfo[2].iCompLen = sizeof(ptLDir->aucName3);	/* name 3 */

	/****************************************************/
	/*	convert to OEM character sequence				*/
	/****************************************************/
	pucDst = aucOEM;								/* destination */
	while (pucSrc) {
		uiCode = ((grp_uint32_t)pucSrc[1] << 8) + pucSrc[0]; /* get unicode */
		if (uiCode == 0)							/* null */
			break;									/* break */
		iCharByte = grp_fs_unicode_to_char(pucDst, uiCode); /* convert it */
		if (iCharByte > 0) {						/* legal one */
			pucDst += iCharByte;					/* advance it */
		} else {									/* error case */
			if (uiCode >= 0x100) {					/* 2 bytes */
				*pucDst++ = (grp_uchar_t)(uiCode & 0xff);/* copy low byte */
				*pucDst++ = (grp_uchar_t)((uiCode >> 8) & 0xff);
													/* copy high byte */
			} else									/* 1 byte */
				*pucDst++ = (grp_uchar_t)uiCode;	/* copy byte */
		}
		pucSrc += 2;								/* advance source */
		if (pucSrc >= pucSrcEnd) {					/* end of component */
			pucSrc = ptNameInfo->pucComp;			/* next component */
			pucSrcEnd = &pucSrc[ptNameInfo->iCompLen]; /* end of component */
			ptNameInfo++;							/* advance to next comp */
		}
	}
	*pucDst = 0;									/* null terminate */
	iLen = pucDst - aucOEM;							/* length */

	/****************************************************/
	/*	copy to directory entry name buffer				*/
	/****************************************************/
	if (ptDirEnt->sNameSize - 1 > iLen) {			/* fit in name buffer */
		iSize = ptDirEnt->sNameSize -1 - iLen;		/* copy length */
		if (iSize > *piUsed)						/* used is smaller */
			iSize = *piUsed;						/* copy only used */
		memmove(&ptDirEnt->pucName[iLen], ptDirEnt->pucName, iSize);
													/* make room for current */
		memcpy(ptDirEnt->pucName, aucOEM, (grp_size_t)iLen); /* copy current */
		*piUsed = iSize + iLen;						/* new used length */
	} else {										/* not fit in buffer */
		*piUsed = ptDirEnt->sNameSize - 1;			/* max used length */
		memcpy(ptDirEnt->pucName, aucOEM, (grp_size_t)*piUsed);	/* copy data */
	}
#else  /* GRP_FS_MINIMIZE_LEVEL < 2 */
	*piUsed = 0;
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
}
/****************************************************************************/
/* FUNCTION:	_fat_comp_checksum											*/
/*																			*/
/* DESCRIPTION:	Computer checksum of short file								*/
/* INPUT:		pucName:	short name										*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		checksum value												*/
/*																			*/
/****************************************************************************/
static int
_fat_comp_checksum(
	grp_uchar_t		*pucName)				/* short name */
{
	grp_uchar_t		ucSum = 0;					/* checksum */
	grp_uchar_t		*pucNameEnd;			/* end of name */

	pucNameEnd = &pucName[FAT_BASE_NAME_LEN + FAT_SUFFIX_NAME_LEN];
	while (pucName < pucNameEnd)			/* loop until end of name */
		ucSum = (grp_uchar_t)
				(((ucSum & 1)? 0x80: 0) + (ucSum >> 1) + *pucName++);
	return((int)ucSum);						/* return check sum */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_next_fat_dir_ent									*/
/*																			*/
/* DESCRIPTION:	Get next FAT directory entry								*/
/* INPUT:		ptDir:				directory information					*/
/*				uiOff:				file offset								*/
/*				ptBio:				buffer I/O information					*/
/* OUTPUT:		ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					EOF										*/
/*				positive:			size of component						*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_get_next_fat_dir_ent(
	grp_fs_file_t	*ptDir,				/* [IN] directory file */
	grp_uint32_t	uiOff,				/* [IN] file offset */
	grp_fs_bio_t	*ptBio)				/* [IN/OUT] buffer I/O information */
{
	fat_BPB_t		*ptBPB;					/* BPB information */
	grp_uint32_t	uiOffBlk;				/* offset block number */
	grp_uint32_t	uiDataBlk;				/* data block number */
	grp_uint32_t	uiBlkOff;				/* block offet */
	grp_int32_t		iRemain;				/* remain size */
	int				iRet;					/* return value */

	ptBPB = ptDir->ptFs->pvFsInfo;				/* BPB information */
	uiBlkOff = FAT_DBLK_OFF(ptBPB, uiOff);		/* block offset */
	iRemain = (uiBlkOff == 0)? 0: 				/* next cluster */
			  ptBio->uiSize - uiBlkOff;			/* remain in current */
	if (iRemain < (int)sizeof(fat_dir_t)) {		/* not in buffer */
		/****************************************************/
		/* check max offset									*/
		/****************************************************/
		if (uiOff >= 0x80000000)			/* over max offset */
			return(GRP_FS_ERR_FS);			/* return error */
		if (FAT_CLST(ptBPB, uiOff) >= ptBPB->uiBadC) /* over max cluster */
			return(GRP_FS_ERR_FS);			/* return error */

		/****************************************************/
		/* check interrupt request							*/
		/****************************************************/
		if (fat_interrupt_lookup &&				/* defined interrupt hook */
		    (iRet = fat_interrupt_lookup(ptDir->iDev, ptDir->uiFid,
								uiOff)) != 0)	/* interrupted */
			return(iRet);						/* return error */

		/****************************************************/
		/* convert to physical data cluster number			*/
		/****************************************************/
		uiOffBlk = FAT_DBLK(ptBPB, uiOff);		/* offset block number */
		iRet = _fat_phys_blk(ptDir, uiOffBlk, &uiDataBlk);/* get phys blk no */
		if (iRet < 0)							/* error occured */
			return((grp_int32_t)iRet);			/* return error */
		if (uiDataBlk == FAT_EOF_BLK)			/* EOF */
			return(0);							/* return EOF */

		/****************************************************/
		/* get cluster data in buffer						*/
		/****************************************************/
		iRet = _fat_get_blk(ptDir->ptFs, uiDataBlk,
						 GRP_FS_BUF_DATA, ptBio);/* get data */
		if (iRet < 0)							/* error */
			return((grp_int32_t)iRet);			/* return error */
		iRemain = ptBio->uiSize - uiBlkOff;		/* remain size */
		if (iRemain <= 0)						/* less than 0 */
			return(0);							/* EOF */
	}
	return(iRemain);							/* return size */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_long_dir											*/
/*																			*/
/* DESCRIPTION:	Get long name directory entry								*/
/* INPUT:		ptDir:				directory information					*/
/*				ptFatDirEnt:		FAT directory entry						*/
/*				ptBio:				buffer I/O information					*/
/* OUTPUT:		ptDirEnt:			directory entry information				*/
/*				ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				positive:			size of directory entry					*/
/*																			*/
/****************************************************************************/
static int
_fat_get_long_dir(
	grp_fs_file_t	*ptDir,				/* [IN]  directory file */
	fat_dir_t		*ptFatDir,			/* [IN]  FAT directory entry */
	grp_fs_bio_t	*ptBio,				/* [IN/OUT] buffer I/O information */
	grp_fs_dir_ent_t *ptDirEnt)			/* [IN/OUT] directory entry info */
{
	grp_uint32_t	uiOff;				/* offset in buffer */
	fat_long_dir_t	*ptLDir;			/* long FAT directory */
	fat_BPB_t		*ptBPB;				/* BPB information */
	grp_int32_t		iSize;				/* next entry data in buffer */
	int				iUsed = 0;			/* used length in name buffer */
	int				iOrder;				/* order number */

	/****************************************************/
	/*	get long name									*/
	/****************************************************/
	ptLDir = (fat_long_dir_t *)ptFatDir;	/* long FAT directory entry */
	if ((ptLDir->ucOrder & FAT_DIR_LAST_LONG) == 0)	/* no last flag */
		return(GRP_FS_ERR_FS);				/* return bad file system */
	_fat_copy_long_name(ptLDir, ptDirEnt, &iUsed);	/* copy long name */

	/****************************************************/
	/*	copy until end of long name						*/
	/****************************************************/
	ptBPB = ptDir->ptFs->pvFsInfo;			/* BPB information */
	uiOff = ptDirEnt->uiStart + sizeof(fat_long_dir_t);	/* start offset */
	iOrder = (ptLDir->ucOrder & ~FAT_DIR_LAST_LONG);	/* order number */
	while (iOrder > 1) {					/* until order becomes 1 */
		iSize = _fat_get_next_fat_dir_ent(ptDir, uiOff, ptBio);
											/* get next entry in buffer */
		if (iSize < 0)						/* error occured */
			return((int)iSize);				/* return it */
		if (iSize < sizeof(fat_long_dir_t))	/* less than entry size */
			return(GRP_FS_ERR_FS);			/* return bad file system */
		ptLDir = (fat_long_dir_t *)
			&ptBio->pucData[FAT_DBLK_OFF(ptBPB, uiOff)];/* set pointer */
		if ((ptLDir->ucAttr & FAT_ATTR_TYPE_MASK) != FAT_ATTR_LONG
			|| ptLDir->ucOrder != iOrder - 1)	/* invalid entry */
			return(GRP_FS_ERR_FS);			/* return bad file system */
		_fat_copy_long_name(ptLDir, ptDirEnt, &iUsed);	/* copy long name */
		iOrder--;							/* decrement order */
		uiOff += sizeof(fat_long_dir_t);	/* advance to next */
	}

	/****************************************************/
	/*	setup directory entry information				*/
	/****************************************************/
	ptDirEnt->iDev = ptDir->iDev;			/* device number */
	ptDirEnt->uiFid = 0;					/* no Fid */
	ptDirEnt->pucName[iUsed] = 0;			/* NULL terminate */
	ptDirEnt->sNameSize = (short)iUsed;		/* set name length */
	ptDirEnt->ucType = GRP_FS_FILE_LINK;	/* long name */
	ptDirEnt->uiProtect = 0;				/* no read/write/execute */
#ifdef GRP_FS_ENABLE_OVER_2G
	ptDirEnt->uiSize = 0;					/* no length information */
#else  /* GRP_FS_ENABLE_OVER_2G */
	ptDirEnt->iSize = 0;					/* no length information */
#endif /* GRP_FS_ENABLE_OVER_2G */
	ptDirEnt->iCTime = 0;					/* no creation time info */
	ptDirEnt->iMTime = 0;					/* no modification time info */
	ptDirEnt->iATime = 0;					/* no access time info */
	ptDirEnt->uiMisc = ptLDir->ucChkSum;	/* check sum */
	ptDirEnt->uiAttr = FAT_ATTR_LONG;		/* long name */
	ptDirEnt->uiEnd = uiOff;				/* end offset */
	return((int)(uiOff - ptDirEnt->uiStart));/* return entry size */
}

/****************************************************************************/
/* FUNCTION:	_fat_conv_dir_info											*/
/*																			*/
/* DESCRIPTION:	Convert directory information								*/
/* INPUT:		ptBPB:				PBP information							*/
/*				ptSDir:				FAT shoft directory information			*/
/* OUTPUT:		ptDirEnt:			directory entry information				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_FS		bad file system							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_conv_dir_info(
	fat_BPB_t		*ptBPB,				/* BPB information */
	fat_dir_t		 *ptSDir,			/* [IN]  FAT short directory */
	grp_fs_dir_ent_t *ptDirEnt)			/* [OUT] directory entry info */
{
	grp_uchar_t		*pucSrc;			/* source name pointer */
	grp_uchar_t		*pucDst;			/* destination name pointer */
	grp_uchar_t		*pucSuffix;			/* suffix pointer */
	int				iLen;				/* copy length */
	grp_uint32_t	uiFid;				/* file id */
	grp_uchar_t		aucSName[FAT_BASE_NAME_LEN]; /* base name buffer */

	/****************************************************/
	/* convert short name entry to canonical data		*/
	/****************************************************/
	if ((ptSDir->ucAttr & FAT_ATTR_DIR)
		&& (ptSDir->ucAttr & FAT_ATTR_VOLID))	/* invalid attribute */
		return(GRP_FS_ERR_FS);					/* bad file system */
	uiFid = ((grp_uint32_t)ptSDir->aucClstHigh[1] << 24)
				+ ((grp_uint32_t)ptSDir->aucClstHigh[0] << 16)
				+ ((grp_uint32_t)ptSDir->aucClstLow[1] << 8)
				+ ptSDir->aucClstLow[0];		/* set file ID */
	if (uiFid >= ptBPB->uiMaxClst || uiFid == 1 || FAT_IS_SIZE0_FID(uiFid))
		return(GRP_FS_ERR_FS);					/* return error */
	ptDirEnt->uiFid = uiFid;					/* set file id */ 
	ptDirEnt->uiAttr = ptSDir->ucAttr;			/* set attribute */
	if (ptSDir->ucAttr & FAT_ATTR_VOLID) {		/* volume ID */
		ptDirEnt->ucType = GRP_FS_FILE_OTHER;	/* other type file */
		ptDirEnt->uiProtect = 0;				/* no access */
	} else {
		ptDirEnt->uiProtect = GRP_FS_PROT_RALL;	/* set read flag */
		if ((ptSDir->ucAttr & FAT_ATTR_RONLY) == 0) /* not read only */
			ptDirEnt->uiProtect |= GRP_FS_PROT_WALL;/* set write flag */
		if (ptSDir->ucAttr & FAT_ATTR_DIR) {	/* directory file */
			ptDirEnt->ucType = GRP_FS_FILE_DIR;	/* set directory type */
			ptDirEnt->uiProtect |= GRP_FS_PROT_XALL; /* set search flag */
		} else {								/* regular file */
			ptDirEnt->ucType = GRP_FS_FILE_FILE;/* set regular file type */
			pucSuffix = &ptSDir->aucName[FAT_BASE_NAME_LEN]; /* suffix */
			if (FAT_EXEC_FILE_SUFFIX(pucSuffix))/* executable file */
				ptDirEnt->uiProtect |= GRP_FS_PROT_XALL; /* set exec flag */
		}
		if (ptSDir->ucAttr & FAT_ATTR_HIDDEN)	/* hidden file */
			ptDirEnt->uiProtect &= ~(GRP_FS_PROT_RWXG|GRP_FS_PROT_RWXO);
												/* clear other/group bits */
	}
#ifdef GRP_FS_ENABLE_OVER_2G
	ptDirEnt->uiSize = ptSDir->aucFileSize[0]
					 + ((grp_int32_t)ptSDir->aucFileSize[1] << 8)
					 + ((grp_int32_t)ptSDir->aucFileSize[2] << 16)
					 + ((grp_int32_t)ptSDir->aucFileSize[3] << 24);
												/* set size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	ptDirEnt->iSize = ptSDir->aucFileSize[0]
					 + ((grp_int32_t)ptSDir->aucFileSize[1] << 8)
					 + ((grp_int32_t)ptSDir->aucFileSize[2] << 16)
					 + ((grp_int32_t)ptSDir->aucFileSize[3] << 24);
												/* set size */
	if (ptDirEnt->iSize < 0)					/* over 2G */
		ptDirEnt->iSize = 0x7fffffff;			/* adjust to 2G - 1 */
#endif /* GRP_FS_ENABLE_OVER_2G */

	/****************************************************/
	/* make special processing for volume label			*/
	/****************************************************/
	if ((ptSDir->ucAttr & FAT_ATTR_VOLID) != 0) { /* volume id */
		pucDst = ptDirEnt->pucName;				/* name buffer pointer */
		memcpy(pucDst, ptSDir->aucName, FAT_SNAME_LEN); /* copy volume label */
		pucDst[FAT_SNAME_LEN] = 0;				/* NULL terminate */
		ptDirEnt->sNameSize = FAT_SNAME_LEN;	/* set name length */
		goto set_time;							/* set time information */
	}

	/****************************************************/
	/* set base name part								*/
	/****************************************************/
	if (ptSDir->aucName[0] == FAT_DIR_E5) {		/* 0xe5 escape */
		memcpy(aucSName, ptSDir->aucName, FAT_BASE_NAME_LEN); /* copy it */
		aucSName[0] = 0xe5;						/* change to 0xe5 */
		pucSrc = aucSName;						/* use aucSName */
	} else {									/* not escape */
		pucSrc = ptSDir->aucName;				/* use directly */
	}
	pucDst = ptDirEnt->pucName;					/* name buffer pointer */
	iLen = _fat_copy_short_file_name(pucDst, pucSrc,
						 FAT_BASE_NAME_LEN, ' '); /* copy name */

	/****************************************************/
	/* set suffix name part								*/
	/****************************************************/
	pucDst[iLen++] = '.';						/* set '.' */
	pucDst += iLen;								/* advance to end */
	iLen = _fat_copy_short_file_name(pucDst,
					&ptSDir->aucName[FAT_BASE_NAME_LEN],
					FAT_SUFFIX_NAME_LEN, ' ');	/* copy name */
	if (iLen == 0)								/* no suffix */
		iLen = -1;								/* erase '.' */
	ptDirEnt->sNameSize = (short)((int)(pucDst - ptDirEnt->pucName) + iLen);
												/* name size */
	pucDst[iLen] = 0;							/* NULL terminate */

	/****************************************************/
	/* set time information 							*/
	/****************************************************/
set_time:
	_fat_time_to_canon(&ptDirEnt->iCTime, ptSDir->aucCDate,
					 ptSDir->aucCTime, ptSDir->ucCTime10ms / 100);
												/* set creation time */
	_fat_time_to_canon(&ptDirEnt->iMTime, ptSDir->aucMDate,
					 ptSDir->aucMTime, 0);	/* set modification time */
	_fat_time_to_canon(&ptDirEnt->iATime, ptSDir->aucADate, NULL, 0);
												/* set access time */
	ptDirEnt->uiMisc = _fat_comp_checksum(ptSDir->aucName);
												/* set checksum */
	return(0);									/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_next_dir_ent										*/
/*																			*/
/* DESCRIPTION:	Get next directory entry									*/
/*				Note: Assuming sNameSize is big enough to hold short name,	*/
/*					  no limit check is made in copying short name.			*/
/* INPUT:		ptDir:				directory information					*/
/*				ptBio:				buffer I/O information					*/
/*				ptDirEnt->pucName:	name buffer								*/
/*				ptDirEnt->sNameSize: name buffer size						*/
/*				ptDirEnt->uiStart:	start to search							*/
/* OUTPUT:		ptDirEnt:			directory entry information				*/
/*				ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				0:					EOF										*/
/*				positive:			size of directory entry					*/
/*																			*/
/****************************************************************************/
static int
_fat_get_next_dir_ent(
	grp_fs_file_t	*ptDir,				/* [IN] directory file */
	grp_fs_bio_t	*ptBio,				/* [IN/OUT] buffer I/O information */
	grp_fs_dir_ent_t *ptDirEnt)			/* [IN/OUT] directory entry info */
{
	grp_uint32_t	uiOff;				/* offset in buffer */
	fat_dir_t		*ptSDir;			/* short FAT directory */
	fat_BPB_t		*ptBPB;				/* BPB information */
	grp_int32_t		iSize;				/* next entry data in buffer */
	int				iRet;				/* return value */

	/****************************************************/
	/* check get non free entry							*/
	/****************************************************/
	ptBPB = ptDir->ptFs->pvFsInfo;			/* BPB information */
	for (uiOff = ptDirEnt->uiStart;			/* set start offset */
		 (iSize = _fat_get_next_fat_dir_ent(ptDir, uiOff, ptBio))
		  >= (int)sizeof(fat_dir_t);		/* get next entry in buffer */
		uiOff += sizeof(fat_dir_t)) {		/* advance to next */
		/****************************************************/
		/* check free and EOF								*/
		/****************************************************/
		ptSDir = (fat_dir_t *)
			&ptBio->pucData[FAT_DBLK_OFF(ptBPB, uiOff)];/* set pointer */
		if (ptSDir->aucName[0] == FAT_DIR_FREE)		/* free entry */
			continue;								/* advance to next */
		if (ptSDir->aucName[0] == FAT_DIR_EOF)		/* end of file */
			break;									/* get end */

		/****************************************************/
		/* process long name entry							*/
		/****************************************************/
		if ((ptSDir->ucAttr & FAT_ATTR_TYPE_MASK) == FAT_ATTR_LONG) {
			ptDirEnt->uiStart = uiOff;				/* set start offset */
			return(_fat_get_long_dir(ptDir, ptSDir, ptBio, ptDirEnt));
		}

		/****************************************************/
		/* convert short name entry to canonical data		*/
		/****************************************************/
		iRet = _fat_conv_dir_info(ptBPB, ptSDir, ptDirEnt);/* convert dir info */
		if (iRet != 0)								/* error detected */
			return(iRet);							/* return error */
		ptDirEnt->iDev = ptDir->iDev;				/* device number */
		ptDirEnt->uiStart = uiOff;					/* set start */
		ptDirEnt->uiEnd = uiOff + sizeof(fat_dir_t);/* set end offset */
		return(sizeof(fat_dir_t));
	}

	/****************************************************/
	/* end of entries									*/
	/****************************************************/
	ptDirEnt->uiEnd = uiOff;				/* set end offset */
	if (iSize < 0)							/* error detected */
		return((int)iSize);					/* return error */
	if (iSize != 0 && iSize < sizeof(fat_dir_t)) /* bad directory file */
		return(GRP_FS_ERR_FS);				/* return error */
	return(0);								/* return EOF */
}

/****************************************************************************/
/* FUNCTION:	_fat_open_root												*/
/*																			*/
/* DESCRIPTION:	Open root dirctory											*/
/* INPUT:		ptFs:				file system information					*/
/* OUTPUT:		pptRoot:			opened root file information			*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_TOO_MANY: too many open files					*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_open_root(
	grp_fs_info_t		*ptFs,				/* [IN]  file system information */
	grp_fs_file_t		**pptRoot)			/* [IN/OUT] opened root file */
{
	grp_fs_file_t		*ptFile = NULL;		/* opened file information */
	fat_open_info_t		*ptOpen;			/* open information */
	fat_BPB_t			*ptBPB;				/* BPB information */
	int					iRet;				/* return value */

	ptBPB = ptFs->pvFsInfo;					/* BPB information */
	iRet = grp_fs_lookup_file_ctl(ptFs, ptBPB->uiRootClst, 1, pptRoot);
											/* lookup/allocate root file info */
	if (iRet != 0) {						/* not found cache */
		if (*pptRoot == NULL)				/* not allocated */
			return(GRP_FS_ERR_TOO_MANY);	/* too many open files */
		ptFile = *pptRoot;					/* set file */
		ptOpen = _fat_alloc_open_info(ptFs->iDev); /* alloc open information */
		if (ptOpen == NULL) {
			ptFile->usStatus |= GRP_FS_FSTAT_INVALID;/* invalid file data */
			grp_fs_close_file(ptFile, GRP_FS_FILE_INVALID|GRP_FS_FILE_UNBLOCK);
											/* close and free it */
			*pptRoot = NULL;				/* clear return file */
			return(GRP_FS_ERR_TOO_MANY);	/* too many open files */
		}
		ptFile->usStatus |= GRP_FS_FSTAT_ROOT;/* root file */
#ifdef GRP_FS_ENABLE_OVER_2G
		ptFile->uiSize = 0;					/* 0 size for director */
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptFile->iSize = 0;					/* 0 size for director */
#endif /* GRP_FS_ENABLE_OVER_2G */
		ptFile->iCTime = ptBPB->iRootCTime;/* set creation time */
		ptFile->iMTime = ptBPB->iRootMTime;/* set modification time */
		ptFile->iATime = ptBPB->iRootATime;/* set access time */
		ptFile->uiAttr = FAT_ATTR_DIR;		/* set attribute information */
		ptFile->ucType = GRP_FS_FILE_DIR;	/* directory */
		ptFile->uiProtect = GRP_FS_PROT_RWXA;/* permit all access */
		ptFile->puiMap = ptOpen->uiMap;		/* set mapping table */
		ptOpen->uiDirFid = 0;				/* no parent directory */
		ptOpen->uiDirBlk = 0;				/* no block */
		ptOpen->uiDirStart = 0;				/* no offset */
		ptOpen->uiDirEnd = 0;				/* no offset */
		ptFile->pvFileInfo = ptOpen;		/* set FS dependent info */
	}
	return(0);
}

/****************************************************************************/
/* FUNCTION:	_fat_lookup_dotdot											*/
/*																			*/
/* DESCRIPTION:	Lookup real directory entry of ".." 						*/
/* INPUT:		ptFs:		FS information									*/
/*				ptDirEnt:	directory entry information						*/
/* OUTPUT:		ptDirEnt:	directory entry information						*/
/*				puiDirFid:	file id of parent directory						*/
/* 				puiDirBlk:	directory block number							*/
/*				piStart:	offset of the found entry						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_TOO_LONG: too long file name						*/
/*				GRP_FS_ERR_NOT_FOUND:file not found							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_lookup_dotdot(
	grp_fs_info_t	*ptFs,				/* [IN]  FS information */
	grp_fs_dir_ent_t *ptDirEnt,			/* [IN/OUT] directory entry info */
	grp_uint32_t	*puiDirFid,			/* [OUT] file id of parent directory */
	grp_uint32_t	*puiDirBlk,			/* [OUT] directory block number */
	grp_int32_t		*piStart)			/* [OUT] offset of found entry */
{
	fat_dir_t		*ptSDir;			/* short directory entry */
	fat_BPB_t		*ptBPB;				/* BPB information */
	grp_uint32_t	uiFid;				/* file id */
	grp_uint32_t	uiTargetFid;		/* target file id */
	grp_uint32_t	uiParentFid;		/* parent file id */
	grp_uint32_t	uiOff;				/* offset */
	grp_uint32_t	uiBlkOff;			/* offset in a block */
	grp_uint32_t	uiClstOff;			/* next cluster start offset */
	grp_uint32_t	uiClst;				/* cluster number */
	grp_uint32_t	uiBlk;				/* block number */
	int				iLong;				/* in long entry */
	int				iRet;				/* return value */
	grp_fs_bio_t	tBio;				/* file buffer information */

	/****************************************************/
	/* init data										*/
	/****************************************************/
	ptBPB = ptFs->pvFsInfo;				/* BPB information */
	uiTargetFid = ptDirEnt->uiFid;		/* file id */
	tBio.ptBuf = NULL;					/* no buffer */

	/****************************************************/
	/* get ".." entry of the directory file				*/
	/****************************************************/
	uiBlk = FAT_PHYS_CLST_DBLK(ptBPB, uiTargetFid, 0);
										/* phys block of the file */
	iRet = _fat_get_blk(ptFs, uiBlk, GRP_FS_BUF_DATA,
						&tBio);			/* get 1st block */
	if (iRet != 0)						/* failed to get */
		return(iRet);					/* return error */
	if (tBio.uiSize < sizeof(fat_dir_t) * 2) /* invalid size */
		goto fs_err_out;				/* return error */
	ptSDir = (fat_dir_t *)&tBio.pucData[sizeof(fat_dir_t)];
										/* ".." entry */
	if (memcmp((char *)ptSDir->aucName, "..         ", sizeof(ptSDir->aucName)))
		goto fs_err_out;				/* return error */
	uiParentFid = ((((grp_uint32_t)ptSDir->aucClstHigh[1]) << 24) |
				(((grp_uint32_t)ptSDir->aucClstHigh[0]) << 16) |
				(((grp_uint32_t)ptSDir->aucClstLow[1]) << 8) |
				((grp_uint32_t)ptSDir->aucClstLow[0])); /* file id of parent */
	if (uiParentFid == 0)				/* root directory */
		uiParentFid = ptBPB->uiRootClst;/* root cluster */
	if (uiParentFid >= ptBPB->uiMaxClst 
		|| uiParentFid == 1 
		|| FAT_IS_SIZE0_FID(uiParentFid)) /* invalid file id */
		goto fs_err_out;				/* return error */
	*puiDirFid = uiParentFid;			/* set parent file id */

	/****************************************************/
	/* find real entry for the directory file			*/
	/****************************************************/
	iLong = 0;							/* not in long entry */
	uiClst = uiParentFid;				/* cluster number */
	uiClstOff = ptBPB->uiClstSize;		/* next cluster offset */
	uiBlkOff = tBio.uiSize;				/* init block offset as no remain */
	for (uiOff = 0; ;
		 uiOff += sizeof(fat_dir_t), uiBlkOff += sizeof(fat_dir_t)) {
		 if (uiBlkOff >= tBio.uiSize) {	/* not in buffer */
			uiBlk = FAT_DBLK(ptBPB, uiOff); /* offset block number */
			if (uiParentFid == 0) {		/* root of FAT12/16 */
				if (uiBlk >= ptBPB->uiDBlkStart) /* EOF */
					goto fs_err_out;	/* return error */
			} else {					/* not FAT12/16 root */
				if (uiOff >= uiClstOff) {	/* over cluster */
					iRet = _fat_get_fat(ptFs, uiClst, NULL, &uiClst);
										/* get next cluster */
					if (iRet != 0)		/* failed to get */
						goto err_out;	/* return error */
					if (uiClst < 2 || uiClst >= ptBPB->uiMaxClst)
						goto fs_err_out;/* return error */
					uiClstOff += ptBPB->uiClstSize; /* update to next */
				}
				uiBlk = FAT_PHYS_CLST_DBLK(ptBPB, uiClst, 
										FAT_DBLK_CLST_OFF(ptBPB, uiBlk));
										/* next block number */
			}
			*puiDirBlk = uiBlk;			/* set directory block number */
			iRet = _fat_get_blk(ptFs, uiBlk, GRP_FS_BUF_DATA, &tBio);
										/* get the block */
			if (iRet != 0)				/* failed to get */
				goto err_out;			/* return error */
			if ((tBio.uiSize & (sizeof(fat_dir_t) - 1)) != 0
				|| tBio.uiSize == 0)	/* invalid size */
				goto fs_err_out;		/* return error */
			uiBlkOff = 0;				/* reset block offset */
		}
		ptSDir = (fat_dir_t *)&tBio.pucData[uiBlkOff]; /* directory entry */
		if (ptSDir->aucName[0] == FAT_DIR_FREE)	/* free entry */
			continue;					/* search next */
		if (ptSDir->aucName[1] == FAT_DIR_EOF) /*  end of directory */
			goto fs_err_out;			/* return error */
		if ((ptSDir->ucAttr & FAT_ATTR_TYPE_MASK) == FAT_ATTR_LONG) {
			/****************************************************/
			/* long directory entry								*/
			/****************************************************/
			if (iLong == 0)				/* start of long entry */
				*piStart = (grp_int32_t)uiOff;	/* remember start */
			iLong = 1;					/* in long entry */
			continue;					/* lookup next */
		}
		if (ptSDir->ucAttr & FAT_ATTR_DIR) { /* directory */
			uiFid = ((((grp_uint32_t)ptSDir->aucClstHigh[1]) << 24) |
					(((grp_uint32_t)ptSDir->aucClstHigh[0]) << 16) |
					(((grp_uint32_t)ptSDir->aucClstLow[1]) << 8) |
					((grp_uint32_t)ptSDir->aucClstLow[0])); /* file id */
			if (uiFid == uiTargetFid) {	/* found */
				if (iLong == 0)			/* no long file */
					*piStart = (grp_int32_t)uiOff;	/* remember start */
				iRet = _fat_conv_dir_info(ptBPB, ptSDir, ptDirEnt);
										/* convert directory info */
				if (iRet != 0)			/* error */
					goto err_out;		/* return error */
				ptDirEnt->iDev = ptFs->iDev;/* device number */
				ptDirEnt->uiStart = uiOff;	/* start offset */
				ptDirEnt->uiEnd = uiOff + sizeof(fat_dir_t); /* end offset */
				break;					/* stop search */
			}
		}
		iLong = 0;						/* not in long entry */
		/****************************************************/
		/* check max offset									*/
		/****************************************************/
		if( (0x80000000 <= uiOff) ||
			(ptBPB->uiBadC <= FAT_CLST(ptBPB, uiOff)) ){
			goto fs_err_out;
		}
		/****************************************************/
		/* check interrupt request							*/
		/****************************************************/
		if(fat_interrupt_lookup &&
			(0 != (iRet = fat_interrupt_lookup(
						ptFs->iDev, uiParentFid, uiOff))) ){
			goto err_out;
		}
	}
	_fat_clean_unref_buf(&tBio);		/* release buffer */
	return(0);							/* return success */

fs_err_out:
	iRet = GRP_FS_ERR_FS;				/* invalid file system */
err_out:
	_fat_clean_unref_buf(&tBio);		/* release buffer */
	return(iRet);						/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_short_after_long									*/
/*																			*/
/* DESCRIPTION: Get short entry after long entry							*/
/* INPUT:		ptDir:				directory information					*/
/*				ptDirEnt->uiStart:	start of long entry						*/
/*				ptDirEnt->uiEnd:	start of short entry					*/
/*				ptDirEnt->uiMisc:	checksum								*/
/* OUTPUT:		ptDirEnt:			found directory entry information 		*/
/*				ptBio:				buffer I/O information					*/
/*																			*/
/* RESULT:		positive:			entry size								*/
/*				-1:					failed to get							*/
/*																			*/
/****************************************************************************/
static int
_fat_get_short_after_long(
	grp_fs_file_t		*ptDir, 			/* [IN] directory file */
	grp_fs_bio_t		*ptBio, 			/* [IN/OUT] buffer I/O info */
	grp_fs_dir_ent_t	*ptDirEnt) 			/* [IN/OUT] directory entry info */
{
	fat_dir_t			*ptSDir;			/* short FAT directory */
	fat_BPB_t			*ptBPB; 			/* BPB information */
	grp_uint32_t		uiOff;				/* short name start */
	grp_uint32_t		uiLongStart;		/* long name start */
	grp_uint32_t		uiCheckSum;			/* checksum */
	grp_int32_t			iSize;				/* got size */
	int					iRet;				/* return value */

	ptBPB = ptDir->ptFs->pvFsInfo;			/* BPB information */
	uiLongStart = ptDirEnt->uiStart;		/* remember start offset */ 
	uiOff = ptDirEnt->uiEnd;				/* next entry offset */
	uiCheckSum = ptDirEnt->uiMisc;			/* remember checksum */
	iSize = _fat_get_next_fat_dir_ent(ptDir, uiOff, ptBio);
											/* get next directory entry */
	if (iSize < (int)sizeof(fat_dir_t))		/* no short entry */
		return(-1);							/* return error */
	ptSDir = (fat_dir_t *)
		&ptBio->pucData[FAT_DBLK_OFF(ptBPB, uiOff)];/* set pointer */
	if (ptSDir->aucName[0] == FAT_DIR_FREE 	/* free entry */
		|| ptSDir->aucName[0] == FAT_DIR_EOF) /* end of file */
		return(-1);							/* return error */
	if ((ptSDir->ucAttr & FAT_ATTR_VOLID) != 0)	/* volume id */
		return(-1);							/* return error */
	iRet = _fat_conv_dir_info(ptBPB, ptSDir, ptDirEnt);
											/* convert directory info */
	if (iRet < 0)							/* failed to convert */
		return(-1);							/* return error */
	if (ptDirEnt->uiMisc != uiCheckSum)		/* checksum mismatch */
		return(-1);							/* return error */
	ptDirEnt->iDev = ptDir->iDev;			/* device number */
	ptDirEnt->uiStart = uiLongStart;		/* set start */
	ptDirEnt->uiEnd = uiOff + sizeof(fat_dir_t);/* set end offset */
	return((int)(ptDirEnt->uiEnd - ptDirEnt->uiStart));	/* return entry size */
}

/****************************************************************************/
/* FUNCTION:	_fat_match_comp												*/
/*																			*/
/* DESCRIPTION:	Open matched component in the directory						*/
/* INPUT:		ptDir:		directory file information						*/
/*				pucComp:	component name to match							*/
/*				iPurge:		purge file name cache							*/
/*				iNeed:		needed space size								*/
/* OUTPUT:		pptFile:	opened file information							*/
/*				puiOff:		offset of needed space							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_TOO_LONG: too long file name						*/
/*				GRP_FS_ERR_NOT_FOUND:file not found							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				GRP_FS_ERR_BAD_NAME: bad file name							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_match_comp(
	grp_fs_file_t	*ptDir,				/* [IN] directory file */
	const grp_uchar_t *pucComp,			/* [IN] component name */
	int				iPurge,				/* [IN] purge file name cache */
	grp_fs_file_t	**pptFile,			/* [OUT] opened file information */
	int				iNeed,				/* [IN] size of needed space */
	grp_uint32_t	*puiOff)			/* [OUT] offset of needed space */
{
	grp_fs_file_t	*ptFile;			/* file information */
	grp_fs_dir_ent_t tDirEnt;			/* directory entry information */
	grp_fs_bio_t	tBio;				/* buffer I/O information */
	fat_BPB_t		*ptBPB;				/* BPB information */
	fat_open_info_t	*ptOpen = NULL;		/* open information */
	grp_uint32_t	uiDirBlk = 0;		/* block number of directory */
	grp_uint32_t	uiPrevEnd = 0;		/* previous end */
	int				iRet;				/* return value */
	int				iFnType;			/* file name type */
	int				iDotDot;			/* ".." file */
#ifdef	GRP_FS_FNAME_CACHE
	grp_fs_fname_cache_t *ptCache = NULL; /* file name cache */
#endif	/* GRP_FS_FNAME_CACHE */
	grp_uchar_t		aucComp[FAT_COMP_SZ];/* component buffer */
	grp_uchar_t		aucSName[FAT_SNAME_BUF_SZ]; /* short name buffer */
	grp_uchar_t		aucUpName[FAT_COMP_SZ];	/* upper case name buffer */

	/****************************************************/
	/* init FAT character table							*/
	/****************************************************/
	if (_iFatCharInitDone == 0) 		/* FAT char table is not initialized */
		_fat_init_char_table();			/* initialize FAT char table */

	/****************************************************/
	/* check "."										*/
	/****************************************************/
	if (strcmp((char *)pucComp, ".") == 0) { /* is "." */
		if (pptFile)					/* need open file */
			*pptFile = ptDir;			/* return the directory */
		return(0);						/* return success */
	}

	/****************************************************/
	/* make short name to compare						*/
	/****************************************************/
	iFnType = _fat_make_short_cmp_name(pucComp, aucSName);
										/* make short name to compare */

#if(GRP_FS_MINIMIZE_LEVEL > 1)
	if (iFnType > 0) {
		iRet = GRP_FS_ERR_BAD_NAME; 			/* bad file name */
		goto err_out;							/* return error */
	}
#endif /* GRP_FS_MINIMIZE_LEVEL > 1 */

	/****************************************************/
	/* init for lookup									*/
	/****************************************************/
	if (pptFile)						/* need open file */
		*pptFile = NULL;				/* clear for init */
	tBio.ptBuf = NULL;					/* no buffer */
	tBio.uiSize = 0;					/* no data */
	ptBPB = ptDir->ptFs->pvFsInfo;		/* BPB information */

	/****************************************************/
	/* convert upper case name							*/
	/****************************************************/
	grp_fs_to_upper(aucUpName, pucComp); /* covert to upper case */
	pucComp = aucUpName;				/* replace with upper case one */

#ifdef	GRP_FS_FNAME_CACHE
	/****************************************************/
	/* lookup file name cache							*/
	/****************************************************/
	iRet = grp_fs_lookup_fname_cache(ptDir, pucComp, iPurge, pptFile);
	if (iRet == GRP_FS_ERR_NOT_FOUND && iFnType == FAT_FNAME_LOWER)
		iRet = grp_fs_lookup_fname_cache(ptDir, aucSName, iPurge, pptFile);
	if (iRet == 0) {					/* found */
		if (pptFile == NULL)			/* not to allocate new one */
			return(0);					/* return 0 */
		ptFile = *pptFile;				/* file info */
		goto found_cache;				/* goto found_cache */
	}
#endif	/* GRP_FS_FNAME_CACHE */

#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
	/****************************************************/
	/* lookup FAT directory cache						*/
	/****************************************************/
	if (ptBPB->uiDirFid == ptDir->uiFid) {
		if ((iFnType != 0				/* long name */
			 && ptBPB->aucLCacheName[0]
			 && strcmp((char *)pucComp, (char *)ptBPB->aucLCacheName) == 0)
			|| ((iFnType & FAT_FNAME_LONG) == 0 /* has short name */
				&& ptBPB->aucSCacheName[0]
			    && strcmp((char *)aucSName, (char *)ptBPB->aucSCacheName) == 0))
		{
			uiDirBlk = ptBPB->uiDirBlk;		/* set block number */
			tDirEnt = ptBPB->tDirCache;		/* get cache info */
			tDirEnt.pucName = aucComp;		/* set file name buffer */
			if (ptBPB->aucSCacheName[0]) {	/* exist short cache */
				strcpy((char *)aucComp, (char *)ptBPB->aucSCacheName);
				tDirEnt.uiStart = ptBPB->uiDirStart; /* set entry start */
				goto found_dir_ent_cache;	/* goto found_dir_ent_cache */
			} else {						/* not exist */
				if (_fat_get_short_after_long(ptDir, &tBio, &tDirEnt) > 0) {
											/* success to get short entry */
					uiDirBlk = tBio.uiBlk;	/* get cluster num of the entry */
					_fat_clean_unref_buf(&tBio); /* release buffer */
					goto found_dir_ent_cache;/* goto found_dir_ent_cache */
				}
			}
		}
	}
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */

	/****************************************************/
	/* lookup component in the directory				*/
	/****************************************************/
	tDirEnt.pucName = aucComp;			/* set file name buffer */
	tDirEnt.sNameSize = FAT_COMP_SZ;	/* set name buffer size */
	tDirEnt.uiStart = 0;				/* start offset is 0 */
	tDirEnt.uiEnd = 0;					/* end offset is 0 */
	while ((iRet = _fat_get_next_dir_ent(ptDir, &tBio, &tDirEnt)) > 0) {
		if (iNeed
			&& (int)(tDirEnt.uiStart - uiPrevEnd) >= iNeed) { /* enough free */
			*puiOff = uiPrevEnd;		/* remember it */
			iNeed = 0;					/* reset need search */
		}
		if (tDirEnt.ucType == GRP_FS_FILE_LINK) {	/* long name */
			int iMatch = (iFnType != 0	/* not short file name */
					&& grp_fs_cmp_fname((const grp_uchar_t *)tDirEnt.pucName,
										(const grp_uchar_t *)pucComp) == 0);
					iRet = _fat_get_short_after_long(ptDir, &tBio, &tDirEnt);
										/* get short entry after long */
			if (iRet < 0)				/* failed to get short entry */
				goto check_next;		/* check next after long again */
			if (iMatch)					/* match long entry */
				break;					/* stop search */
		}

		/****************************************************/
		/* check short entry								*/
		/****************************************************/
		if (tDirEnt.ucType != GRP_FS_FILE_OTHER) {	/* short name */
			if ((iFnType & FAT_FNAME_LONG) == 0) {	/* short or lowercase */
				if (strcmp((char *)tDirEnt.pucName, (char *)aucSName) == 0)
					break;				/* stop search */
			}
		}

	check_next:
		tDirEnt.sNameSize = FAT_COMP_SZ;/* set size again */
		tDirEnt.uiStart = tDirEnt.uiEnd;/* set next */
		uiPrevEnd = tDirEnt.uiEnd;		/* remember previous end */
	}

	if (iRet == 0) {					/* EOF */
		iRet = GRP_FS_ERR_NOT_FOUND;	/* not found */
		if (iNeed)						/* needed space is not found yet */
			*puiOff = uiPrevEnd;		/* set end of last entry */
	}
	if (tBio.ptBuf)						/* buffer exists */
		uiDirBlk = tBio.uiBlk;			/* get cluster number of the entry */
	_fat_clean_unref_buf(&tBio);		/* release buffer */
	if (iRet < 0) {						/* error detected */
		return(iRet);					/* return error */
	}

	/****************************************************/
	/* setup new file information for found component	*/
	/****************************************************/
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
found_dir_ent_cache:
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
	if (pptFile == NULL)				/* not to allocate new one */
		return(0);						/* return 0 */
	iDotDot = (strcmp((char *)pucComp, "..") == 0); /* is ".." */
	if (tDirEnt.uiFid == 0) {			/* root or size 0 file */
		if (!iDotDot					/* not  ".." */
			&& tDirEnt.ucType == GRP_FS_FILE_FILE) {/* regular file */
			/****************************************************/
			/* size 0 file; allocate uniq file id				*/
			/****************************************************/
			tDirEnt.uiFid = _fat_lookup_size0_file(ptDir->iDev, ptDir->uiFid,
								tDirEnt.uiEnd, &ptOpen);
										/* get unique file id for it */
			if (tDirEnt.uiFid == FAT_EOF_CLST)	/* failed to allocate */
				return(GRP_FS_ERR_TOO_MANY); /* return error */
		} else if (ptBPB->uiRootClst)	/* root has cluster */
			tDirEnt.uiFid = ptBPB->uiRootClst;	/* use the number */
	}
	if (ptDir->uiFid == tDirEnt.uiFid) {/* same to parent directory */
		*pptFile = ptDir;				/* set the file */
		return(0);						/* return success */
	}
	if (iDotDot)						/* ".." file */
		grp_fs_unblock_file_op(ptDir);	/* temporary unblock file operation */
	if (ptBPB->uiRootClst == tDirEnt.uiFid) /* root directory */
		iRet = _fat_open_root(ptDir->ptFs, pptFile); /* open root */
	else
		iRet = grp_fs_lookup_file_ctl(ptDir->ptFs, tDirEnt.uiFid, 1, pptFile);
										/* get target file */
	if (iDotDot)						/* ".." file */
		grp_fs_block_file_op(ptDir);	/* block file operation again */
	ptFile = *pptFile;					/* set file */
	if (iRet != 0) {					/* cache not found */
		grp_uint32_t	uiDirFid;		/* file id of parent directory */
		if (ptFile == NULL) {			/* not allocated */
			iRet = GRP_FS_ERR_TOO_MANY;	/* too many open files */
			goto err_out;				/* return error */
		}
		if (fat_check_cluster_at_open && /* check cluster at open */
			!FAT_IS_SIZE0_FID(ptFile->uiFid)) { /* not size 0 file */
			iRet = _fat_check_cluster(ptDir->ptFs, ptFile->uiFid, NULL);
										/* check cluster */
			if (iRet != 0)				/* error */
				goto close_file;		/* return error */
		}
		uiDirFid = ptDir->uiFid;		/* file id of parent directory */
		if (iDotDot) {					/* ".." file */
			grp_int32_t		iStart = 0;	/* start of directory entry */
			iRet = _fat_lookup_dotdot(ptDir->ptFs, &tDirEnt, &uiDirFid, 
										&uiDirBlk, &iStart);
										/* find real name of ".." */
			if (iRet != 0)				/* failed to find */
				goto close_file;		/* return error */
			tDirEnt.uiStart = (grp_uint32_t)iStart;
										/* set start */
		}
		if (ptOpen == NULL)				/* no open information allocated */
			ptOpen = _fat_alloc_open_info(ptDir->iDev); /* alloc open info */
		if (ptOpen == NULL) {			/* failed to allocate */
			ptFile->usStatus |= GRP_FS_FSTAT_INVALID;/* invalid file data */
			grp_fs_close_file(ptFile, GRP_FS_FILE_INVALID|GRP_FS_FILE_UNBLOCK);
										/* close and free it */
			iRet = GRP_FS_ERR_TOO_MANY; /* too many open files */
			goto err_out;				/* return error */
		}
		if (tDirEnt.uiFid == ptBPB->uiRootClst) /* root file */
			ptFile->usStatus |= GRP_FS_FSTAT_ROOT;/* set root flag */
#ifdef GRP_FS_ENABLE_OVER_2G
		ptFile->uiSize = tDirEnt.uiSize;	/* copy size information */
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptFile->iSize = tDirEnt.iSize;		/* copy size information */
#endif /* GRP_FS_ENABLE_OVER_2G */
		ptFile->iCTime = tDirEnt.iCTime;	/* set creation time */
		ptFile->iMTime = tDirEnt.iMTime;	/* set modification time */
		ptFile->iATime = tDirEnt.iATime;	/* set access time */
		ptFile->uiAttr = tDirEnt.uiAttr;	/* set attribute information */
		ptFile->ucType = tDirEnt.ucType;	/* set type information */
		ptFile->uiProtect = tDirEnt.uiProtect; /* set protection information */
		ptFile->puiMap = ptOpen->uiMap;		/* set mapping table */
		ptOpen->uiDirFid = uiDirFid;		/* remember parent directory id */
		ptOpen->uiDirBlk = uiDirBlk;		/* remember cluster number */
		ptOpen->uiDirStart = tDirEnt.uiStart;/* remember start offset */
		ptOpen->uiDirEnd = tDirEnt.uiEnd;	/* remember end offset */
		ptFile->pvFileInfo = ptOpen;		/* set FS dependent info */
	}
#ifdef	GRP_FS_FNAME_CACHE
	/****************************************************/
	/* setup file name cache							*/
	/****************************************************/
	if (iPurge == 0 && !iDotDot) {			/* not purge cache and not ".." */
		ptOpen = ptFile->pvFileInfo;		/* open info */
		if (ptOpen->ptFnCache) {			/* exist file name cache */
			grp_fs_purge_fname_cache_entry(ptOpen->ptFnCache);
											/* purge old cache */
			ptOpen->ptFnCache = NULL;		/* reset cache pointer */
		}
		if (iFnType != 0) {					/* not short name */
			ptCache = grp_fs_set_fname_cache(ptDir, pucComp, ptFile, NULL);
			if (ptCache)					/* cache set */
				ptOpen->ptFnCache = ptCache;/* set file name cache info */
		}
		ptCache = grp_fs_set_fname_cache(ptDir, tDirEnt.pucName, ptFile,
								ptCache);	/* set file name cache */
		if (ptCache)						/* cache set */
			ptOpen->ptFnCache = ptCache;	/* set file name cache info */
	}

found_cache:
#endif	/* GRP_FS_FNAME_CACHE */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
	if (iPurge) {							/* purge cache */
		ptBPB->uiDirFid = 0;				/* clear directory file id */
		ptBPB->aucSCacheName[0] = 0;		/* clear short name */
		ptBPB->aucLCacheName[0] = 0;		/* clear long name */
		ptBPB->uiDirBlk = 0;				/* clear block */
		ptBPB->uiDirStart = 0;				/* clear start */
		memset(&ptBPB->tDirCache, 0, sizeof(ptBPB->tDirCache));
											/* cleaer cache */
	}
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
	if (ptDir->uiFid != ptFile->uiFid		/* not my self */
		&& strcmp((char *)pucComp, "..") != 0) {	/* not parent directory */
		if ((ptDir->uiProtect & GRP_FS_PROT_WUSR) == 0) /* no write directory */
			ptFile->usStatus |= GRP_FS_FSTAT_NO_UPD_TIME;/* disable time mod */
		else
			ptFile->usStatus &= ~GRP_FS_FSTAT_NO_UPD_TIME;/* enable time mod */
	}
	grp_fs_close_file(ptDir, GRP_FS_FILE_UNBLOCK); /* close parent directory */
	return(0);								/* return success */

close_file:
	ptFile->usStatus |= GRP_FS_FSTAT_INVALID;/* invalid file data */
	grp_fs_close_file(ptFile, GRP_FS_FILE_INVALID|GRP_FS_FILE_UNBLOCK);
err_out:
	if (pptFile)							/* need to file */
		*pptFile = NULL;					/* clear pointer */
	if (ptOpen)	{							/* allocated open info */
		_fat_deque_size0_list(ptOpen);		/* free from size0 list */
		_fat_free_open_info(ptOpen);		/* free open information */
	}
	return(iRet);							/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_update_attr											*/
/*																			*/
/* DESCRIPTION:	Update file attributes										*/
/* INPUT:		ptFile:				file information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_update_attr(
	grp_fs_file_t	*ptFile)			/* [IN]  file information */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	fat_open_info_t	*ptOpen = ptFile->pvFileInfo; /* open information */
	grp_uint32_t	uiOff;					/* block offset */
	grp_uint32_t	uiFid;					/* file id */
	grp_int32_t		iRead;					/* read count */
	fat_dir_t		*ptFatDir;				/* FAT directory entry */
	int				iRet = 0;				/* return value */
	int				iCleanRet;				/* return of _fat_clean_unref_buf */
	grp_fs_bio_t	tBio;					/* buffer I/O info */

	uiFid = FAT_IS_SIZE0_FID(ptFile->uiFid)? 0: ptFile->uiFid;	/* file id */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
	if (ptOpen->uiDirFid == ptBPB->uiDirFid
		&& ptOpen->uiDirEnd == ptBPB->tDirCache.uiEnd) {/* chached one */
		ptBPB->tDirCache.uiFid = uiFid;				/* update cache */
#ifdef GRP_FS_ENABLE_OVER_2G
		ptBPB->tDirCache.uiSize = ptFile->uiSize;	/* update cache */
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptBPB->tDirCache.iSize = ptFile->iSize;		/* update cache */
#endif /* GRP_FS_ENABLE_OVER_2G */
		ptBPB->tDirCache.iCTime = ptFile->iCTime;	/* update cache */
		ptBPB->tDirCache.iMTime = ptFile->iMTime;	/* update cache */
		ptBPB->tDirCache.iATime = ptFile->iATime;	/* update cache */
		ptBPB->tDirCache.uiAttr = ptFile->uiAttr;	/* update cache */
		ptBPB->tDirCache.uiProtect = ptFile->uiProtect;
													/* update cache */
	}
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */

	iRead = grp_fs_read_buf(ptFs, ptOpen->uiDirBlk, GRP_FS_BUF_DATA,
				(grp_int32_t)ptBPB->uiDBlkSize, &tBio);	/* read block */
	if (iRead < 0) {						/* I/O error */
		return((int)iRead);					/* return error */
	} else if (iRead != (grp_int32_t)ptBPB->uiDBlkSize) { /* I/O error */
		iRet = GRP_FS_ERR_IO;				/* I/O error */
		goto out;
	}
	uiOff = ptOpen->uiDirEnd - sizeof(fat_dir_t); /* offset in dir */
	uiOff = FAT_DBLK_OFF(ptBPB, uiOff);		/* block offset */
	ptFatDir = (fat_dir_t *)&tBio.pucData[uiOff]; /* directory entry */
	grp_fs_block_buf_mod(&tBio);			/* block modify */
	if (ptFile->usStatus & GRP_FS_FSTAT_UPD_ATTR) { /* modified attr */
		ptFatDir->aucClstLow[0] = (grp_uchar_t)(uiFid & 0xff);
		ptFatDir->aucClstLow[1] = (grp_uchar_t)((uiFid >> 8) & 0xff);
		ptFatDir->aucClstHigh[0] = (grp_uchar_t)((uiFid >> 16) & 0xff);
		ptFatDir->aucClstHigh[1] = (grp_uchar_t)((uiFid >> 24) & 0xff);
#ifdef GRP_FS_ENABLE_OVER_2G
		ptFatDir->aucFileSize[0] = (grp_uchar_t)(ptFile->uiSize & 0xff);
		ptFatDir->aucFileSize[1] = (grp_uchar_t)((ptFile->uiSize >> 8) & 0xff);
		ptFatDir->aucFileSize[2] = (grp_uchar_t)((ptFile->uiSize >> 16) & 0xff);
		ptFatDir->aucFileSize[3] = (grp_uchar_t)((ptFile->uiSize >> 24) & 0xff);
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptFatDir->aucFileSize[0] = (grp_uchar_t)(ptFile->iSize & 0xff);
		ptFatDir->aucFileSize[1] = (grp_uchar_t)((ptFile->iSize >> 8) & 0xff);
		ptFatDir->aucFileSize[2] = (grp_uchar_t)((ptFile->iSize >> 16) & 0xff);
		ptFatDir->aucFileSize[3] = (grp_uchar_t)((ptFile->iSize >> 24) & 0xff);
#endif /* GRP_FS_ENABLE_OVER_2G */
		ptFatDir->ucAttr = (grp_uchar_t)ptFile->uiAttr;
											/* set attribute */
		if ((ptFs->usStatus & GRP_FS_STAT_NO_CRT_ACCTIME) == 0)
			_fat_update_time(ptFatDir->aucCDate, ptFatDir->aucCTime,
						 &ptFatDir->ucCTime10ms, ptFile->iCTime, ptFs);
	}
	_fat_update_time(ptFatDir->aucMDate, ptFatDir->aucMTime,
				 NULL, ptFile->iMTime, NULL);/* update mod time */
	_fat_update_time(ptFatDir->aucADate, NULL,
				 NULL, ptFile->iATime, ptFs);/* update access time */
	grp_fs_unblock_buf_mod(&tBio, GRP_FS_BSTAT_DIRTY);/* unblock modify */

out:
	iCleanRet = _fat_clean_unref_buf(&tBio);/* write back */
	if (iRet == 0) {						/* no error */
		iRet = iCleanRet;					/* use ret from clean_buf */
		if (iCleanRet == 0)					/* write success */
			ptFile->usStatus &= ~GRP_FS_FSTAT_UPD_BITS; /* clear flags */
	}
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	_fat_close													*/
/*																			*/
/* DESCRIPTION:	Close a FAT file											*/
/* INPUT:		ptFile:				file information						*/
/*				iMode:				close mode								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_close(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	int				iMode)				/* [IN]  close mode */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	grp_uint32_t	iMask;					/* status mask */
	int				iRet = 0;				/* return value */

	/****************************************************/
	/* update time information if necessary				*/
	/****************************************************/
	iMask = GRP_FS_FSTAT_UPD_ATTR;					/* set UPD_ATTR mask */
	if ((ptFile->usStatus & GRP_FS_FSTAT_NO_UPD_TIME) == 0) { /* can time mod */
		if (grp_fat_update_access_time				/* update access time */
			&& (ptFs->usStatus & GRP_FS_STAT_NO_UPD_ACCTIME) == 0)
			iMask |= GRP_FS_FSTAT_UPD_ATIME;		/* set UPD_ATIME mask */
		if (grp_fat_update_mod_time)				/* update mod time */
			iMask |= GRP_FS_FSTAT_UPD_MTIME;		/* set UPD_MTIME mask */
	}
	if (ptFile->uiFid == ptBPB->uiRootClst) {		/* root directory */
		ptBPB->iRootMTime = ptFile->iMTime;			/* copy mod time */
		ptBPB->iRootATime = ptFile->iATime;			/* copy access time */
		ptFile->usStatus &= ~GRP_FS_FSTAT_UPD_BITS;	/* reset update bits */
													/* clear flags */
	} else if ((ptFs->usStatus & GRP_FS_STAT_RONLY) == 0 /* not read only */
		&& (ptFile->usStatus & GRP_FS_FSTAT_INVALID) == 0 /* invalid file */
		&& (ptFile->usStatus & iMask)				/* need to update */
		&& ptFile->pvFileInfo) {					/* with open info */
		iRet = _fat_update_attr(ptFile);			/* update attribute info */
	}

	/****************************************************/
	/* release open information if necessary			*/
	/****************************************************/
	if (iMode == GRP_FS_CLOSE_RELEASE) {			/* release open info */
		if (ptFile->pvFileInfo) {					/* need to release */
			fat_open_info_t *ptOpen = ptFile->pvFileInfo; /* file info */
			if (FAT_IS_SIZE0_FID(ptFile->uiFid))	/* size 0 file */
				_fat_deque_size0_list(ptOpen);		/* deque from size 0 list */
			_fat_free_open_info(ptOpen);			/* release open info */
			ptFile->pvFileInfo = NULL;				/* no data */
			ptFile->puiMap = NULL;					/* no map */
			ptFile->uiMapCnt = 0;					/* no map */
		}
	}
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_fat_make_unique_sname										*/
/*																			*/
/* DESCRIPTION:	Make unique short name										*/
/* INPUT:		pucSProto:		proto short name							*/
/*				iNum:			number to make the name unique				*/
/* OUTPUT:		pucSname:		candidate of unique short name				*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_make_unique_sname(
	grp_uchar_t		*pucSName,			/* [OUT] short name */
	grp_uchar_t		*pucSProto,			/* [IN]  proto short name */
	int				iNum)				/* [IN]  number to make unique name */
{
	grp_uchar_t		*pucSp;				/* short name pointer */
	int				iCharByte;			/* character byte count */
	int				iGenLen;			/* length of number name part */
	int				iLen = 0;			/* length */
	int				iFill;				/* fill size */
	grp_uchar_t		aucGen[8];			/* generated number name */
	int	sprintf(char *pcBuf, const char *pcFormat, ...);

	/****************************************************/
	/* generate number name part						*/
	/****************************************************/
	iGenLen = sprintf((char *)aucGen, "~%d", iNum);
										/* genarate number name part */

	/****************************************************/
	/* lookup fit position in base part					*/
	/****************************************************/
	for (pucSp = pucSProto; *pucSp && *pucSp != '.'; pucSp += iCharByte) {
		iCharByte = grp_fs_char_cnt(pucSp);	/* get next character bytes */
		if (iLen + iCharByte + iGenLen > FAT_BASE_NAME_LEN) /* over base */
			break;							/* break */
		iLen += iCharByte;					/* advance to next */
	}

	/****************************************************/
	/* make base part									*/
	/****************************************************/
	memcpy(pucSName, pucSProto, (grp_size_t)iLen); /* copy base part */
#ifdef	GRP_FS_FAT_TRY_NO_NUM_SHORT
	if (*pucSp && *pucSp != '.') {			/* trimmed before end of base */
		iFill = FAT_BASE_NAME_LEN - iLen - iGenLen; /* fill length */
		if (iFill != 0)						/* need to fill */
			memset(&pucSName[iLen], '_', (grp_size_t)iFill);/* fill '_' */
	} else									/* copied whole base part */
#endif	/* GRP_FS_FAT_TRY_NO_NUM_SHORT */
		iFill = 0;							/* no fill '_' */
	memcpy(&pucSName[iLen + iFill], aucGen, (grp_size_t)iGenLen);
											/* copy gen part */

	/****************************************************/
	/* skip to '.' or end of string						*/
	/****************************************************/
	for ( ; *pucSp && *pucSp != '.'; pucSp += iCharByte)
		iCharByte = grp_fs_char_cnt(pucSp);	/* get next character bytes */

	/****************************************************/
	/* copy suffix part									*/
	/****************************************************/
	strcpy((char *)&pucSName[iLen + iFill + iGenLen], (char *)pucSp);
}

/****************************************************************************/
/* FUNCTION:	_fat_clear_cluster											*/
/*																			*/
/* DESCRIPTION:	Clear cluster block											*/
/* INPUT:		ptFs:				file information						*/
/*				uiDataClst:			cluster number							*/
/*				ptBio:				buffer I/O information					*/
/* OUTPUT:		puiDataBlk:			top data block number of the cluster	*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				positive:			size of data block						*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_clear_cluster(
	grp_fs_info_t	*ptFs,					/* [IN]  file system information */
	grp_uint32_t	uiDataClst,				/* [IN]  cluster number */
	grp_fs_bio_t	*ptBio,					/* [OUT] buffer I/O information */
	grp_uint32_t	*puiDataBlk)			/* [OUT] data block number */
{
	grp_int32_t		iBlkClstOff;			/* block cluster offset */
	grp_uint32_t	uiDataBlk;				/* data block number */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	int				iRet;					/* return value */

	/****************************************************/
	/* allocate and clear new block						*/
	/****************************************************/
	ptBio->ptBuf = NULL;						/* no buffer */
	uiDataBlk = FAT_PHYS_CLST_DBLK(ptBPB, uiDataClst, 0); /* data block */
	for (iBlkClstOff = ((grp_int32_t)1 << ptBPB->iDBlkClstShift) - 1;
									iBlkClstOff >= 0; iBlkClstOff--) {
		if (ptBio->ptBuf) {						/* release previous buffer */
			grp_fs_buf_fill_end(ptBio, 0);		/* end fill */
			iRet = _fat_clean_unref_buf(ptBio);	/* release buffer */
			if (iRet < 0)						/* failed to write back */
				return((grp_int32_t)iRet);		/* return error */
		}
		iRet = grp_fs_lookup_buf(ptFs, uiDataBlk + iBlkClstOff,
						GRP_FS_BUF_DATA|GRP_FS_BUF_ALLOC,
							(grp_int32_t)ptBPB->uiDBlkSize, ptBio);
												/* allocate a block */
		if (iRet != 0 && iRet != GRP_FS_ERR_NOT_FOUND) /* error occured */
			return((grp_int32_t)iRet);			/* return error */
		grp_fs_block_buf_mod(ptBio);			/* block modification */
		memset(ptBio->pucData, 0, (grp_size_t)ptBio->uiSize);/* clear buffer */
		grp_fs_unblock_buf_mod(ptBio, GRP_FS_BSTAT_DIRTY);/* unblock modify */
	}
	*puiDataBlk = uiDataBlk;					/* allocated data block */
	return((grp_int32_t)ptBPB->uiDBlkSize);		/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_alloc_clear_cluster									*/
/*																			*/
/* DESCRIPTION:	Allocate and clear cluster block							*/
/* INPUT:		ptFile:				file information						*/
/*				uiOffBlk:			offset block number 					*/
/*				ptBio:				buffer I/O information					*/
/* OUTPUT:		puiDataBlk:			allocated data block number				*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				positive			allocate data block size				*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_alloc_clear_cluster(
	grp_fs_file_t	*ptFile,				/* [IN]  file information */
	grp_uint32_t	uiOffBlk,				/* [IN]  offset block number */
	grp_fs_bio_t	*ptBio,					/* [IN/OUT]  buffer I/O info */
	grp_uint32_t	*puiDataBlk)			/* [OUT] allocated data block */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;	/* file system information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	grp_uint32_t	uiDataClst;				/* allocated cluster number */
	grp_int32_t		iCnt;					/* free cluster count */
	int				iRet;					/* return value */

	/****************************************************/
	/* allocate a new free cluster						*/
	/****************************************************/
	ptBio->ptBuf = NULL;						/* clear buffer */
	*puiDataBlk = FAT_EOF_BLK;					/* init default return block */
	if (ptFile->uiFid == 0)						/* fix size root directory */
			return(GRP_FS_ERR_NO_SPACE);		/* return no space */
	iCnt = _fat_get_free_cluster(ptFs, ptFile, FAT_DBLK_CLST(ptBPB, uiOffBlk),
								1, 0, &uiDataClst);/* allocate cluster */
	if (iCnt < 0)								/* error occured */
		return(iCnt);							/* return error */

	/****************************************************/
	/* allocate and clear new block						*/
	/****************************************************/
	iRet = _fat_clear_cluster(ptFs, uiDataClst, ptBio, puiDataBlk);
	if (iRet < 0) {								/* failed to clear buffer */
		_fat_free_cluster_list(ptFile, uiDataClst, 1, 
								FAT_DBLK_CLST(ptBPB, uiOffBlk));
												/* free it */
	}
	return(iRet);								/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_write_file_internal									*/
/*																			*/
/* DESCRIPTION:	Write a FAT file											*/
/* INPUT:		ptFile:				file information						*/
/*				uiOff:				file offset								*/
/* 				pucBuf:				write data								*/
/*				iSize:				write size								*/
/*				pfnCopy:			data copy function						*/
/* OUTPUT:		puiBlk:				block number of last write block		*/
/*				piSuccess:			success write size					    */
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0 or positive:		size written							*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_write_file_internal(
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiOff,				/* [IN]  offset */
	grp_uchar_t		*pucBuf,			/* [IN]  write data */
	grp_isize_t		iSize,				/* [IN]  write size */
	fat_copy_func_t	pfnCopy,			/* [IN]  data copy function */
	grp_uint32_t	*puiBlk,			/* [OUT] block number of last write */
	grp_isize_t		*piSuccess)			/* [OUT] success write size */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;		/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;	/* BPB information */
	grp_uint32_t	uiBlk;						/* offset block number */
	grp_uint32_t	uiBlkOff;					/* block offset */
	grp_uint32_t	uiDataBlk;					/* data block number */
	grp_int32_t		iDataSz;					/* data size to write */
	grp_int32_t		iWrite;						/* buffer/written size */
	int				iRet;						/* return value */
	grp_isize_t		iTotal = 0;					/* byte already written */
	grp_fs_bio_t	tFsBio;						/* buffer I/O info */

	/****************************************************/
	/* set update time									*/
	/****************************************************/
	grp_fs_get_current_time(&ptFile->iMTime);	/* set modify time */
	ptFile->iATime = ptFile->iMTime;			/* set access time */
	ptFile->usStatus |= (GRP_FS_FSTAT_UPD_ATIME|GRP_FS_FSTAT_UPD_MTIME);
												/* set update flags */

	/****************************************************/
	/* write per cluster								*/
	/****************************************************/
	tFsBio.ptBuf = NULL;						/* clear buffer */
	while (iTotal < iSize ) {
		/****************************************************/
		/* convert to physical cluster						*/
		/****************************************************/
		uiBlk = FAT_DBLK(ptBPB, uiOff);			/* block number */
		iRet = _fat_phys_blk(ptFile, uiBlk, &uiDataBlk);/* conv to phys */
		if (iRet < 0)							/* error detected */
			goto err_out_without_unref;			/* return error */
		if (uiDataBlk == FAT_EOF_BLK) {			/* EOF */
			/****************************************************/
			/* allocate and clear new cluster					*/
			/****************************************************/
			iWrite = _fat_alloc_clear_cluster(ptFile, uiBlk,
						&tFsBio, &uiDataBlk);	/* allocate a new cluster */
		} else {
			/****************************************************/
			/* read cluster data in buffer						*/
			/****************************************************/
			iWrite = grp_fs_read_buf(ptFs, uiDataBlk, GRP_FS_BUF_DATA,
							(grp_int32_t)ptBPB->uiDBlkSize, &tFsBio);
												/* read data */
		}
		if (iWrite <= 0) {						/* error */
			iRet = (int)iWrite;					/* set error number */
			if (iRet == 0)
				iRet = GRP_FS_ERR_NO_SPACE;		/* no space */
			goto err_out;						/* return error */
		}

		/****************************************************/
		/* copy data to buffer								*/
		/****************************************************/
		uiBlkOff = FAT_DBLK_OFF(ptBPB, uiOff);	/* block offset */
		iDataSz = iWrite - uiBlkOff;			/* data in buffer */
		if (iDataSz <= 0) {						/* actual data is smaller */
			iRet = GRP_FS_ERR_FS;				/* invalid FS */
			goto err_out;						/* return error */
		}
		if (iDataSz > iSize - iTotal)			/* write remained is smaller */
			iDataSz = iSize - iTotal;			/* use remained */
		grp_fs_block_buf_mod(&tFsBio);			/* block modification */
		iWrite = pfnCopy(tFsBio.pucData + uiBlkOff, pucBuf, iDataSz);
												/* copy data */
		grp_fs_unblock_buf_mod(&tFsBio, GRP_FS_BSTAT_DIRTY);/* unblock modify */
		if (iWrite != iDataSz) {
			iRet = GRP_FS_ERR_IO;				/* I/O error */
			goto err_out;						/* return error */
		}
		grp_fs_get_current_time(&ptFile->iMTime);/* set modify time */
		ptFile->iATime = ptFile->iMTime;		/* set access time */
		ptFile->usStatus |= (GRP_FS_FSTAT_UPD_ATIME|GRP_FS_FSTAT_UPD_MTIME);
												/* set update flags */
		uiOff += iDataSz;						/* advance next */
		iTotal += iDataSz;						/* advance next */
		pucBuf += iDataSz;						/* advance next */
		*puiBlk = tFsBio.uiBlk;					/* set block number */
		grp_fs_buf_fill_end(&tFsBio, 0);		/* end fill */
		iRet = _fat_clean_unref_buf(&tFsBio);	/* write back */
		if (iRet < 0)							/* error occured */
			goto err_out_without_unref;			/* return error */
	}
	*piSuccess = iTotal;						/* set success write size */
	return(iTotal);								/* return written size */

err_out:
	grp_fs_buf_fill_end(&tFsBio, 1);			/* end fill with error */
	(void)_fat_clean_unref_buf(&tFsBio);		/* release buffer */
err_out_without_unref:
	*piSuccess = iTotal;						/* set success write size */
	return((grp_int32_t)iRet);					/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_memcpy													*/
/*																			*/
/* DESCRIPTION:	Memory copy routine used for _fat_write_file_internal		*/
/* INPUT:		pucSrc:				source buffer							*/
/*				iSize:				copy size								*/
/* OUTPUT:		pucDst:				copied data								*/
/*																			*/
/* RESULT:		same as iSize												*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_memcpy(
	grp_uchar_t		*pucDst,				/* [IN] destination buffer */
	grp_uchar_t		*pucSrc,				/* [IN] source buffer */
	grp_int32_t		iSize)					/* [IN] copy size */
{
	memcpy(pucDst, pucSrc, (grp_size_t)iSize); /* copy data */
	return(iSize);							/* return copied size */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_short_dir_entry									*/
/*																			*/
/* DESCRIPTION:	Set short directory information								*/
/* INPUT:		pucSName:	short name										*/
/*				uiClst:		cluster number									*/
/*				uiAttr:		FAT dependent attribute information				*/
/*				iCTime:		create time										*/
/*				iMTime:		modification time								*/
/*				iATime:		access time										*/
/*				iSize:		file size										*/
/*				ptFs:		file system information							*/
/* OUTPUT:		ptSDir:		short directory entry							*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_set_short_dir_entry(
	fat_dir_t		*ptSDir,				/* [OUT] short directory entry */
	grp_uchar_t		*pucSName,				/* [IN]  short name */
	grp_uint32_t	uiClst,					/* [IN]  cluster number */
	grp_uint32_t	uiAttr,					/* [IN]  file attribute */
	grp_int32_t		iCTime,					/* [IN]  creation time */
	grp_int32_t		iMTime,					/* [IN]  modification time */
	grp_int32_t		iATime,					/* [IN]  access time */
	grp_isize_t		iSize,					/* [IN]  size of file */
	grp_fs_info_t	*ptFs)					/* [IN]  file system information */
{
	grp_uchar_t		*pucSrc;				/* source name */
	grp_uchar_t		*pucDst;				/* destination name buffer */
	int				iLen;					/* copy length */

	/****************************************************/
	/* set cluster and size information					*/
	/****************************************************/
	ptSDir->aucClstLow[0] = (grp_uchar_t)(uiClst & 0xff);
	ptSDir->aucClstLow[1] = (grp_uchar_t)((uiClst >> 8) & 0xff);
	ptSDir->aucClstHigh[0] = (grp_uchar_t)((uiClst >> 16) & 0xff);
	ptSDir->aucClstHigh[1] = (grp_uchar_t)((uiClst >> 24) & 0xff);
	ptSDir->aucFileSize[0] = (grp_uchar_t)(iSize & 0xff);
	ptSDir->aucFileSize[1] = (grp_uchar_t)((iSize >> 8) & 0xff);
	ptSDir->aucFileSize[2] = (grp_uchar_t)((iSize >> 16) & 0xff);
	ptSDir->aucFileSize[3] = (grp_uchar_t)((iSize >> 24) & 0xff);

	/****************************************************/
	/* set short name									*/
	/****************************************************/
	pucDst = ptSDir->aucName;						/* destination */
	pucSrc = pucSName;								/* source name */
	memset(pucDst, ' ', FAT_SNAME_LEN);				/* fill ' ' */
	if (*pucSrc == '.') {							/* "." or ".." */
		memcpy(pucDst, pucSrc, strlen((char *)pucSrc));	/* just copy it */
	} else {										/* other file */
		iLen = _fat_copy_short_file_name(pucDst, pucSrc,
					FAT_BASE_NAME_LEN, '.');		/* copy base part */
		if (*pucDst == 0xe5)						/* E5 char */
			*pucDst = FAT_DIR_E5;					/* replace it */
		pucSrc += iLen;								/* advance to suffix */
		if (*pucSrc++ == '.') {						/* suffix exists */
			pucDst += FAT_BASE_NAME_LEN;			/* set to suffix part */
			iLen = _fat_copy_short_file_name(pucDst,
						pucSrc, FAT_SUFFIX_NAME_LEN, 0);/* copy suffix */
		}
	}

	/****************************************************/
	/* set time information								*/
	/****************************************************/
	_fat_update_time(ptSDir->aucCDate, ptSDir->aucCTime,
					 &ptSDir->ucCTime10ms, iCTime, ptFs);/* set creation time */
	_fat_update_time(ptSDir->aucMDate, ptSDir->aucMTime,
					 NULL, iMTime, NULL);			/* set mod time */
	_fat_update_time(ptSDir->aucADate, NULL,
					 NULL, iATime, ptFs);			/* set access time */

	/****************************************************/
	/* set attribute and reserved						*/
	/****************************************************/
	ptSDir->ucAttr = (grp_uchar_t)uiAttr;			/* set attribute */
	ptSDir->ucRsv = 0;								/* clear reserved */
}

/****************************************************************************/
/* FUNCTION:	_fat_create_directory										*/
/*																			*/
/* DESCRIPTION:	Create a directory											*/
/* INPUT:		ptDir:		parent directory								*/
/*				uiFileClst:	file cluster number								*/
/*				iCTime:		creation time									*/
/*				uiAttr:		file attribute									*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_create_directory(
	grp_fs_file_t	*ptDir,				/* [IN]  parent directory */
	grp_uint32_t	uiFileClst,			/* [IN]  file cluster number */
	grp_int32_t		iCTime,				/* [IN]  creation time */
	grp_uint32_t	uiAttr)				/* [IN]  attribute */
{
	fat_dir_t		*ptSDir;			/* directory entry */
	grp_uint32_t	uiDirBlk;			/* directory block */
	int				iRet;				/* return value */
	grp_fs_info_t	*ptFs = ptDir->ptFs;/* file system information */
	grp_fs_bio_t	tFsBio;				/* buffer I/O information */

	/****************************************************/
	/* allocate and clear new block						*/
	/****************************************************/
	iRet = (int)_fat_clear_cluster(ptFs, uiFileClst, &tFsBio, &uiDirBlk);
												/* alloc and clear block */
	if (iRet < 0)								/* error occured */
		return(iRet);							/* return error */

	/****************************************************/
	/* set "." and ".." information						*/
	/****************************************************/
	ptSDir = (fat_dir_t *)tFsBio.pucData;		/* set top of buffer */
	grp_fs_block_buf_mod(&tFsBio);				/* block modification */
	_fat_set_short_dir_entry(&ptSDir[0], (grp_uchar_t *)".",
				uiFileClst, uiAttr,
				iCTime, iCTime, iCTime, 0, ptFs);/* set "." */
	_fat_set_short_dir_entry(&ptSDir[1], (grp_uchar_t *)"..",
				((ptDir->usStatus & GRP_FS_FSTAT_ROOT)? 0: ptDir->uiFid),
				 ptDir->uiAttr,
				iCTime, iCTime, iCTime, 0, ptFs);/* set ".." */
	grp_fs_unblock_buf_mod(&tFsBio, GRP_FS_BSTAT_DIRTY);/* unblock modify */
	grp_fs_buf_fill_end(&tFsBio, 0);			/* fill end */
	iRet = _fat_clean_unref_buf(&tFsBio);		/* write back */
	return(iRet);								/* return return cdoe */
}

/****************************************************************************/
/* FUNCTION:	_fat_change_dir_link										*/
/*																			*/
/* DESCRIPTION:	Change parent directory link								*/
/* INPUT:		ptFs:		file system information							*/
/*				uiFileClst:	file cluster number								*/
/*				uiDirClst:	cluster number of the parent directory			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_change_dir_link(
	grp_fs_info_t	*ptFs,				/* [IN]  file system information */
	grp_uint32_t	uiFileClst,			/* [IN]  file cluster number */
	grp_uint32_t	uiDirClst)			/* [IN]  cluster number of directory */
{
	fat_dir_t		*ptSDir;				/* directory entry */
	int				iRet;					/* return value */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;/* BPB information */
	grp_fs_bio_t	tFsBio;					/* buffer I/O information */

	/****************************************************/
	/* get 1st block of the directory in buffer			*/
	/****************************************************/
	tFsBio.ptBuf = NULL;						/* init buffer */
	iRet = _fat_get_blk(ptFs, FAT_PHYS_CLST_DBLK(ptBPB, uiFileClst, 0),
							GRP_FS_BUF_DATA, &tFsBio);/* get data block */
	if (iRet != 0) 								/* error detected */
		return(iRet);							/* return error */

	/****************************************************/
	/* change ".." information							*/
	/****************************************************/
	ptSDir = &((fat_dir_t *)tFsBio.pucData)[1];	/* second entry */
	grp_fs_block_buf_mod(&tFsBio);				/* block modification */
	if (strncmp((char *)ptSDir->aucName, "..", 2) != 0
		|| ptSDir->aucName[2] != ' ')			/* not ".." */ {
		grp_fs_unblock_buf_mod(&tFsBio, 0);		/* unblock modification */
		_fat_clean_unref_buf(&tFsBio);			/* release buffer */
		return(GRP_FS_ERR_FS);					/* return bad FS error */
	}
	ptSDir->aucClstLow[0] = (grp_uchar_t)(uiDirClst & 0xff);
	ptSDir->aucClstLow[1] = (grp_uchar_t)((uiDirClst >> 8) & 0xff);
	ptSDir->aucClstHigh[0] = (grp_uchar_t)((uiDirClst >> 16) & 0xff);
	ptSDir->aucClstHigh[1] = (grp_uchar_t)((uiDirClst >> 24) & 0xff);
	grp_fs_unblock_buf_mod(&tFsBio, GRP_FS_BSTAT_DIRTY);/* unblock modify */
	iRet = _fat_clean_unref_buf(&tFsBio);		/* write back */
	return(iRet);								/* return return value */
}

/****************************************************************************/
/* FUNCTION:	_fat_write_directory										*/
/*																			*/
/* DESCRIPTION:	Write a directory entry										*/
/* INPUT:		ptDir:		parent directory								*/
/*				uiOffset:	offset of the directory entry					*/
/*				iSize:		size of the directory entry						*/
/*				uiFileClst:	cluster number of created file					*/
/*				uiType:		type of the file								*/
/*				uiProtect:	protection										*/
/*				uiAttr:		FAT dependent attribute information				*/
/*				pucSName:	short name										*/
/*				pucLName:	long name										*/
/*				pptFile:	original file to be renamed	or NULL				*/
/* OUTPUT:		pptFile:	created file information						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GF_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_BAD_NAME		bad file name 							*/
/*				GRP_FS_ERR_BAD_PARAM: invalid parameter						*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_write_directory(
	grp_fs_file_t	*ptDir,				/* [IN]  directory file */
	grp_uint32_t	uiOffset,			/* [IN]  offset of the entry */
	int				iSize,				/* [IN]  size of entry */
	grp_uint32_t	uiFileClst,			/* [IN]  cluster number of file */
	grp_uint32_t	uiType,				/* [IN]  type of file */
	grp_uint32_t	uiProtect,			/* [IN]  protection */
	grp_uint32_t	uiAttr,				/* [IN]  FAT dependent attribute */
	grp_uchar_t		*pucSName,			/* [IN]  short name */
	grp_uchar_t		*pucLName,			/* [IN]  long name */
	grp_fs_file_t	**pptFile)			/* [IN/OUT] created file */
{
	fat_dir_t		*ptSDir;			/* short directory entry */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	fat_long_dir_t	*ptLDir;			/* long directory */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
	grp_int32_t		iCTime;				/* creation time */
	grp_int32_t		iMTime;				/* modify time */
	grp_int32_t		iATime;				/* access time */
	grp_int32_t		iFSize;				/* file size */
	int				iCheckSum;			/* checksum */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	int				iCharCnt;			/* character count */
	int				iCharByte;			/* character byte count */
	grp_uint32_t	uiCode;				/* unicode */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
	grp_uint32_t	uiDirBlk;			/* directory entry block number */
	grp_uint32_t	uiOrgDirFid;		/* original directory's fid */
	grp_int32_t		iWrite;				/* written size */
	int				iOrder = 0;			/* order count */
	int				iRet;				/* return value */
	fat_open_info_t	*ptOpen;			/* open file information */
	grp_fs_file_t	*ptFile;			/* file information */
	grp_fs_file_t	*ptOrg;				/* original file */
	grp_fs_info_t	*ptFs = ptDir->ptFs;/* file system information */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	grp_uchar_t		*pucSrc;			/* source char pointer */
	grp_uchar_t		*pucDst;			/* destination char pointer */
	grp_uchar_t		*pucUnSrc;			/* unicode source pointer */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
	grp_isize_t		iSuccess = 0;		/* success write size */
	grp_uint32_t	uiOrgDirBlk = 0;	/* original directory entry block */
	grp_uint32_t	uiOrgDirStart = 0;	/* original start offset */
	grp_uint32_t	uiOrgDirEnd = 0;	/* original end offset */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	fat_dir_t		tFatDir[FAT_MAX_DENT]; /* directory entry buffer */
#else	/* GRP_FS_MINIMIZE_LEVEL < 2 */
	fat_dir_t		tFatDir[1];			   /* directory entry buffer */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	grp_uchar_t		aucUnicode[FAT_LNAME_CNT * 2]; /* uicode buffer */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

	/****************************************************/
	/* in creating a directory file, setup inside first	*/
	/****************************************************/
	ptOrg = *pptFile;								/* get orginal file */
	*pptFile = NULL;								/* clear it */
	if (ptOrg == NULL) {							/* no original file */
		grp_fs_get_current_time(&iCTime);			/* get current time */
		iMTime = iATime = iCTime;					/* copy to each time */
		iFSize = 0;									/* 0 file size */
		if (uiType == GRP_FS_FILE_DIR) {			/* create directory */
			iRet = _fat_create_directory(ptDir, uiFileClst, iCTime, uiAttr);
													/* create directory file */
			if (iRet < 0)							/* error occured */
				goto err_out;						/* return error */
		}
	} else {										/* original file exists */
		iCTime = ptOrg->iCTime;						/* creation time */
		iMTime = ptOrg->iMTime;						/* modify time */
		iATime = ptOrg->iATime;						/* access time */
#ifdef GRP_FS_ENABLE_OVER_2G
		iFSize = ptOrg->uiSize;						/* set file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
		iFSize = ptOrg->iSize;						/* set file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	}

	/****************************************************/
	/* set short entry									*/
	/****************************************************/
#if(GRP_FS_MINIMIZE_LEVEL > 1)
	if (iSize != sizeof(fat_dir_t)) {
		iRet = GRP_FS_ERR_BAD_PARAM;
		goto err_out;
	}
#endif /* GRP_FS_MINIMIZE_LEVEL > 1 */
	ptSDir = &tFatDir[(iSize / sizeof(fat_dir_t)) - 1];	/* short dir entry */
	_fat_set_short_dir_entry(ptSDir, pucSName, 
							(FAT_IS_SIZE0_FID(uiFileClst)? 0: uiFileClst),
							uiAttr, iCTime, iMTime, iATime,
							iFSize, ptFs);			/* set short directory */
	iCheckSum = _fat_comp_checksum(ptSDir->aucName);/* compute checksum */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
	/****************************************************/
	/* set long name entry								*/
	/****************************************************/
	ptLDir = (fat_long_dir_t *)&ptSDir[-1];			/* long directory */
	pucSrc = pucLName;								/* long name */
	for ( ; ptLDir >= (fat_long_dir_t *)tFatDir; ptLDir--) {
		pucDst = aucUnicode;						/* unicode buffer  */
		for (iCharCnt = 0; iCharCnt < FAT_LNAME_CNT; iCharCnt++) {
			iCharByte = grp_fs_char_to_unicode(pucSrc, &uiCode); /* unicode */
			if (iCharByte <= 0)						/* end of string */
				break;								/* break */
			*pucDst++ = (grp_uchar_t)(uiCode & 0xff);/* low byte */
			*pucDst++ = (grp_uchar_t)((uiCode >> 8) & 0xff);/* high byte */
			pucSrc += iCharByte;					/* advance next */
		}
		if (iCharCnt++ < FAT_LNAME_CNT) {			/* buffer remains */
			*pucDst++ = 0;							/* set null */
			*pucDst++ = 0;							/* set null */
		}
		while (iCharCnt++ < FAT_LNAME_CNT) {		/* buffer remains */
			*pucDst++ = 0xff;						/* set end mark */
			*pucDst++ = 0xff;						/* set end mark */
		}
		pucUnSrc = aucUnicode;						/* unicode buffer */
		memcpy(ptLDir->aucName1, pucUnSrc, sizeof(ptLDir->aucName1));
		pucUnSrc += sizeof(ptLDir->aucName1);		/* advance */
		memcpy(ptLDir->aucName2, pucUnSrc, sizeof(ptLDir->aucName2));
		pucUnSrc += sizeof(ptLDir->aucName2);		/* advance */
		memcpy(ptLDir->aucName3, pucUnSrc, sizeof(ptLDir->aucName3));
		ptLDir->ucAttr = FAT_ATTR_LONG;				/* long file */
		ptLDir->ucChkSum = (grp_uchar_t)iCheckSum;	/* set checksum */
		ptLDir->ucOrder = (grp_uchar_t)(++iOrder);	/* set order number */
		if (ptLDir == (fat_long_dir_t *)tFatDir)	/* last entry */
			ptLDir->ucOrder |= FAT_DIR_LAST_LONG;	/* set last bit */
		ptLDir->ucType = 0;							/* file type 0 */
		ptLDir->aucClstLow[0] = 0;					/* clear reserved */
		ptLDir->aucClstLow[1] = 0;					/* clear reserved */
	}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

	/****************************************************/
	/* write directory entry							*/
	/****************************************************/
	iWrite = _fat_write_file_internal(ptDir, uiOffset,
						(grp_uchar_t *)tFatDir, (grp_isize_t)iSize,
						_fat_memcpy, &uiDirBlk, &iSuccess);	/* write entries */
	if (iWrite != (grp_int32_t)iSize) {				/* write failed */
		if (iWrite >= 0)							/* not error value */
			iRet = GRP_FS_ERR_FS;					/* invalid FS */
		else										/* error value */
			iRet = (int)iWrite;						/* set error number */
		goto err_out;								/* return error */
	}

	/****************************************************/
	/* create file information							*/
	/****************************************************/
	if (ptOrg == NULL) {							/* no orignal file */
		iRet = grp_fs_lookup_file_ctl(ptFs, uiFileClst, 1, pptFile);
													/* get target file */
		ptFile = *pptFile;							/* set file */
		if (iRet != 0 && ptFile == NULL) {			/* not allocated */
			iRet = GRP_FS_ERR_TOO_MANY;				/* too many open files */
			goto err_out;							/* return error */
		}
		ptOpen = ptFile->pvFileInfo;				/* open information */
		if (ptOpen == NULL) {						/* no information */
			if (FAT_IS_SIZE0_FID(uiFileClst)) {		/* size 0 file */
				ptOpen = FAT_SIZE0_OPEN_INFO(uiFileClst); /* get open info */
			} else {								/* need to alloc */
				ptOpen = _fat_alloc_open_info(ptDir->iDev);/* alloc open info */
				if (ptOpen == NULL) {				/* not allocated */
					*pptFile = NULL;				/* clear file info */
					ptFile->usStatus |= GRP_FS_FSTAT_INVALID;
													/* invalid file data */
					grp_fs_close_file(ptFile,
								GRP_FS_FILE_INVALID|GRP_FS_FILE_UNBLOCK);
													/* close and free it */
					iRet = GRP_FS_ERR_TOO_MANY;		/* too many open files */
					goto err_out;					/* return error */
				}
			}
		}
#ifdef GRP_FS_ENABLE_OVER_2G
		ptFile->uiSize = 0;							/* clear file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptFile->iSize = 0;							/* clear file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
		ptFile->iCTime = iCTime;					/* set creation time */
		ptFile->iMTime = iMTime;					/* set modification time */
		ptFile->iATime = iATime;					/* set access time */
		ptFile->uiAttr = uiAttr;					/* set attribute info */
		ptFile->ucType = (grp_uchar_t)uiType;		/* set type information */
		ptFile->uiProtect = uiProtect;				/* set protection info */
		ptFile->puiMap = ptOpen->uiMap;				/* set mapping table */
		ptFile->pvFileInfo = ptOpen;				/* set FS dependent info */
	} else {
		*pptFile = ptFile = ptOrg;					/* set original file */
		ptOpen = ptOrg->pvFileInfo;					/* open information */
		uiOrgDirBlk = ptOpen->uiDirBlk;				/* save original dir blk */
		uiOrgDirStart = ptOpen->uiDirStart;			/* save original start */
		uiOrgDirEnd = ptOpen->uiDirEnd;				/* save original end */
	}
	if (ptFile->ucType == GRP_FS_FILE_FILE) {		/* regular file */
		grp_uchar_t *pucSuffix = &ptSDir->aucName[FAT_BASE_NAME_LEN];
		ptFile->uiProtect &= ~GRP_FS_PROT_XALL;		/* clear execute bit */
		if (FAT_EXEC_FILE_SUFFIX(pucSuffix)) {		/* executable file */
			if (uiAttr & FAT_ATTR_HIDDEN)			/* hidden file */
				ptFile->uiProtect |= GRP_FS_PROT_XUSR; /* set exec bit */
			else									/* not hidden file */
				ptFile->uiProtect |= GRP_FS_PROT_XALL; /* set exec bit */
		}
	}
	uiOrgDirFid = ptOpen->uiDirFid;					/* orginal fid */
	ptOpen->uiDirFid = ptDir->uiFid;				/* set parent dir id */
	ptOpen->uiDirBlk = uiDirBlk;					/* set block number */
	ptOpen->uiDirStart = uiOffset;					/* set start offset */
	ptOpen->uiDirEnd = uiOffset + iSize;			/* rset end offset */
	if (ptOrg && 									/* rename */
		uiType == GRP_FS_FILE_DIR &&				/* directory file */
		uiOrgDirFid != ptDir->uiFid) {				/* different parent */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
		fat_BPB_t		*ptBPB = ptFs->pvFsInfo;	/* BPB information */
		if (ptBPB->uiDirFid == ptOrg->uiFid			/* match directory */
		    && strcmp((char *)ptBPB->aucSCacheName, "..") == 0) /* ".." */
				ptBPB->aucSCacheName[0] = 0;		/* clear short name cache */
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
#ifdef	GRP_FS_FNAME_CACHE
		grp_fs_purge_fname_cache(ptOrg, (const grp_uchar_t *)"..");
													/* purge ".." */
#endif	/* GRP_FS_FNAME_CACHE */
		iRet = _fat_change_dir_link(ptFs, uiFileClst, ptDir->uiFid);
													/* change directory link */
		if (iRet != 0) {							/* change link failed */
			_fat_free_dir_ent(ptDir, ptOpen);		/* free directory entry */
			ptOpen->uiDirFid = uiOrgDirFid;			/* restore parent id */
			ptOpen->uiDirBlk = uiOrgDirBlk;			/* restore block number */
			ptOpen->uiDirStart = uiOrgDirStart;		/* restore start offset */
			ptOpen->uiDirEnd = uiOrgDirEnd;			/* restore end offset */
		}
		return(iRet);								/* return return value */
	} else
		return(0);									/* return success */

err_out:
	if (iSuccess) {									/* write success */
		grp_uchar_t		*pucEnd;					/* end pointer */
		memset(tFatDir, 0, (grp_size_t)iSuccess);	/* clear data */
		pucEnd = ((grp_uchar_t *)tFatDir) + iSuccess; /* end pointer */
		for (ptSDir = tFatDir; ptSDir < (fat_dir_t *)pucEnd; ptSDir++) {
			ptSDir->aucName[0] = FAT_DIR_FREE;		/* set free entry */
		}
		_fat_write_file_internal(ptDir, uiOffset,
						(grp_uchar_t *)tFatDir, iSuccess,
						_fat_memcpy, &uiDirBlk, &iSuccess);	/* clear entries */
	}
	return(iRet);									/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_create_rename_file										*/
/*																			*/
/* DESCRIPTION:	Create or rename a FAT file or directory					*/
/*				Note:	ptDir is locked when called, and 					*/
/*						created/ptOrgFile and ptOrgDir are locked, 			*/
/*						but ptDir is closed in success return.				*/
/*						In case of error, ptDir is closed, but ptOrgFile	*/
/*						and ptOrgDir are unchanged.							*/
/* INPUT:		ptFs:		file system information							*/
/*				ptDir:		parent directory								*/
/*				pucComp:	component name									*/
/*				uiType:		type of file									*/
/*				uiProtect:	protection										*/
/*				uiAttr:		FS dependent attribute							*/
/*				ptOrgFile:	original file to be renamed						*/
/*				ptOrgDir:	parent directory of the original file			*/
/* OUTPUT:		pptFile:	created file information						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GF_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_BAD_NAME: bad file name							*/
/*				GRP_FS_ERR_TOO_LONG: too long file name						*/
/*				GRP_FS_ERR_BAD_TYPE: bad type								*/
/*				GRP_FS_ERR_EXIST:	already exist							*/
/*				GRP_FS_ERR_TOO_MANY: too many open files					*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_create_rename_file(
	grp_fs_info_t		*ptFs,				/* [IN]  file system information */
	grp_fs_file_t		*ptDir,				/* [IN]  parent directory */
	grp_uchar_t			*pucComp,			/* [IN]  component name */
	grp_uint32_t		uiType,				/* [IN]  file type */
	grp_uint32_t		uiProtect,			/* [IN]  protection */
	grp_uint32_t		uiAttr,				/* [IN]  FS dependent attribute */
	grp_fs_file_t		*ptOrgFile,			/* [IN]  original file */
	grp_fs_file_t		*ptOrgDir,			/* [IN]  original directory */
	grp_fs_file_t		**pptFile)			/* [OUT] created file information */
{
	grp_fs_file_t		*ptFile = NULL;		/* file information */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	int					iCharCnt;			/* character count */
	int					iSLSize;			/* short + long size */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
	int					iFnType;			/* file name type */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	int					iNum;				/* number to make unique name */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
	int					iRet;				/* return value */
	int					iNeed;				/* need entry size */
	int					iBlockDirWrite = 0;	/* block directory write */
#ifdef GRP_FS_FAST_MAKE_SNAME
	int					iFastMakeFlg = 0;	/* fast maked sname flag */
#endif /* GRP_FS_FAST_MAKE_SNAME */
	grp_uint32_t		uiOffset;			/* offset of available entry */
	grp_uint32_t		uiNewClst = FAT_EOF_CLST;	/* free new cluster */
	fat_open_info_t		*ptOpen = NULL;		/* open information */
#ifdef	GRP_FS_FNAME_CACHE
	grp_fs_fname_cache_t *ptCache = NULL;	/* file name cache */
#endif	/* GRP_FS_FNAME_CACHE */
	grp_uchar_t			aucSProto[FAT_SNAME_BUF_SZ];/* short proto name */
	grp_uchar_t			aucSName[FAT_SNAME_BUF_SZ]; /* short name buffer */

	/****************************************************/
	/* init FAT character table							*/
	/****************************************************/
	if (_iFatCharInitDone == 0) 		/* FAT char table is not initialized */
		_fat_init_char_table();			/* initialize FAT char table */

	/****************************************************/
	/* check invalid device								*/
	/****************************************************/
	if (grp_fs_check_io_status(ptFs, 0, 0, GRP_FS_IO_REQ) != 0) {
		iRet = GRP_FS_ERR_IO;					/* I/O error */
		goto err_out;							/* return error */
	}

	/****************************************************/
	/* check "." and ".." file							*/
	/****************************************************/
	if (strcmp((char *)pucComp, ".") == 0
		|| strcmp((char *)pucComp, "..") == 0) {/* "." or ".." file */
		iRet = GRP_FS_ERR_EXIST;				/* file exist error */
		goto err_out;							/* return error */
	}

	/****************************************************/
	/* make long and short name prototype				*/
	/****************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	iCharCnt = _fat_make_long_name(pucComp);	/* check and make long name */
	if (iCharCnt < 0) {							/* error detected */
		iRet = iCharCnt;						/* set error number */
		goto err_out;							/* return error */
	}
	if (iCharCnt > FAT_COMP_CHCNT) {			/* too long component */
		iRet = GRP_FS_ERR_TOO_LONG;				/* too long file name */
		goto err_out;							/* return error */
	}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
	iFnType = _fat_make_short_ent_name(pucComp, aucSProto);/* make proto name */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	strcpy((char *)aucSName, (char *)aucSProto);/* copy to short name */
	iSLSize = ((iCharCnt + FAT_LNAME_CNT - 1)/FAT_LNAME_CNT + 1)
				* (int)sizeof(fat_dir_t);			/* short + long size */
	iNeed = (iFnType != 0)? iSLSize: (int)sizeof(fat_dir_t);/* need size */
#else  /* GRP_FS_MINIMIZE_LEVEL < 2 */
	if (iFnType > 0) {
		iRet = GRP_FS_ERR_BAD_NAME; 			/* bad file name */
		goto err_out;							/* return error */
	}
	
	strcpy((char *)aucSName, (char *)aucSProto);/* copy to short name */
	iNeed = (int)sizeof(fat_dir_t); 			/* need size */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

	/****************************************************/
	/* lock directory file								*/
	/****************************************************/
	grp_fs_block_file_write(ptDir);				/* block directory write */
	iBlockDirWrite = 1;							/* blocked write */

	/****************************************************/
	/* lookup file in the parent directroy				*/
	/****************************************************/
	ptDir->iRefCnt++;							/* increment ref not to free */
	iRet = _fat_match_comp(ptDir, pucComp, 0, &ptFile, iNeed, &uiOffset);
												/* lookup file */
	if (iRet == 0) {							/* found the file */
		iRet = GRP_FS_ERR_EXIST;				/* file exist error */
		grp_fs_unblock_file_write(ptDir);		/* unblock file write */
		iBlockDirWrite = 0;						/* unblocked write */
		grp_fs_close_file(ptDir, 0);			/* unreference directory */
		if (ptOrgFile == NULL) {				/* not rename */
			*pptFile = ptFile;					/* return found file */
			return(iRet);						/* return error */
		}
		ptDir = NULL;							/* no directory file */
		goto err_out;							/* return error */
	}
	ptDir->iRefCnt--;							/* decrement extra reference */
	if (iRet != GRP_FS_ERR_NOT_FOUND)			/* other than not found */
		goto err_out;							/* return error */
	if ((iFnType & FAT_FNAME_LONG) == 0)		/* not long name */
		goto create_file;						/* create file */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
	/****************************************************/
	/* make unique short name							*/
	/****************************************************/
	for (iNum = 1; ; iNum++) {					/* until found unique name */
#ifdef GRP_FS_FAST_MAKE_SNAME
		if ((iFastMakeFlg == 0)
			&& (iNum > GRP_FS_MAKE_SNAME_THRESHOLD)) {
			/****************************************************/
			/* generate proto short name						*/
			/****************************************************/
			if (0 == grp_fs_make_sname_another_method(pucComp, aucSProto)) {
				iNum = 1;						/* reset number */
			}
			iFastMakeFlg = 1;					/* remake proto sname */
		}
#endif /* GRP_FS_FAST_MAKE_SNAME */

		/****************************************************/
		/* lookup the generated file name					*/
		/****************************************************/
#ifndef	GRP_FS_FAT_TRY_NO_NUM_SHORT
		_fat_make_unique_sname(aucSName, aucSProto, iNum);/* make unique name */
#endif	/* GRP_FS_FAT_TRY_NO_NUM_SHORT */
		ptDir->iRefCnt++;						/* increment ref not to free */
		iRet = _fat_match_comp(ptDir, aucSName, 0, &ptFile,
								iNeed, &uiOffset);/* lookup file */
		if (iRet != 0)							/* error or not found */
			break;								/* break processing */

		/****************************************************/
		/* conflict: generate another name					*/
		/****************************************************/
		grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* close found file */
#ifdef	GRP_FS_FAT_TRY_NO_NUM_SHORT
		_fat_make_unique_sname(aucSName, aucSProto, iNum);/* make unique name */
#endif	/* GRP_FS_FAT_TRY_NO_NUM_SHORT */
		grp_fs_block_file_op(ptDir);			/* block file operation */
	}
	ptDir->iRefCnt--;							/* decrement reference */
	ptFile = NULL;								/* no file */
	if (iRet != GRP_FS_ERR_NOT_FOUND)			/* error detected */
		goto err_out;							/* return error */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

	/****************************************************/
	/* get free cluster if necessary					*/
	/****************************************************/
create_file:
	if (ptOrgFile) {							/* rename original exists */
		if (ptDir == ptOrgFile) {				/* rename parent */
			iRet = GRP_FS_ERR_BUSY;				/* busy error */
			goto err_out;						/* return error */
		}
		if (ptDir != ptOrgDir) {				/* rename different directory */
			/****************************************************/
			/* lock original directory							*/
			/* 	Since deadlock may occur at moving to parent	*/
			/*	or at simultaneous cross rename case,			*/
			/* 	temporaryly release new directory lock 			*/
			/*	to keep lock order by from parent to file and	*/
			/* 	by from smaller file id to bigger				*/
			/****************************************************/
			grp_uint32_t uiParent;				/* parenet of new directory */

			uiParent = ((fat_open_info_t *)(ptDir->pvFileInfo))->uiDirFid;
			if (uiParent == ptOrgDir->uiFid		/* orginal is parent of new */
				|| ptDir->uiFid > ptOrgDir->uiFid) { /* orginal id is smaller */
				grp_fs_unblock_file_op(ptDir);	/* unblock to avoid deadlock */
				grp_fs_block_file_op(ptOrgDir);	/* lock original directory */
				grp_fs_block_file_op(ptDir);	/* lock new directory again */
			} else {
				grp_fs_block_file_op(ptOrgDir);	/* lock original directory */
			}
		}
		grp_fs_block_file_op(ptOrgFile);		/* lock original file */
		if (ptOrgFile->iRefCnt != 1) {			/* someone is referencing */
			iRet = GRP_FS_ERR_BUSY;				/* busy error */
			grp_fs_unblock_file_op(ptOrgFile);	/* unlock original file */
			if (ptDir != ptOrgDir) 				/* different directory */
				grp_fs_unblock_file_op(ptOrgDir);/* unlock original directory */
			goto err_out;						/* return error */
		}
		uiNewClst = ptOrgFile->uiFid;			/* use the cluster */
		ptFile = ptOrgFile;						/* use original file */
	} else {									/* create new file */
		if (uiType == GRP_FS_FILE_DIR) {		/* create directory */
			iRet = (int)_fat_get_free_cluster(ptDir->ptFs, NULL, 0, 1,
							ptDir->uiFid, &uiNewClst); /* get free cluster */
			if (iRet < 0)						/* error occured */
				goto err_out;					/* return error */
		} else {								/* create file */
			uiNewClst = _fat_lookup_size0_file(ptDir->iDev, ptDir->uiFid,
								uiOffset + iNeed, &ptOpen);
												/* get uniq file id */
			if (uiNewClst == FAT_EOF_CLST) {	/* failed to allocate */
				iRet = GRP_FS_ERR_TOO_MANY;		/* set error */
				goto err_out;					/* return error */
			}
			if (ptOpen == NULL) {				/* no open info allocated */
				iRet = GRP_FS_ERR_BUSY;			/* already exists */
				goto err_out;					/* return error */
			}
		}
	}

#ifdef GRP_FS_UPDATE_ARCHIVE
	/****************************************************/
	/* added archive attribute							*/
	/****************************************************/
		uiAttr |= FAT_ATTR_ARCHIVE;				/* set archive */
#endif /* GRP_FS_UPDATE_ARCHIVE */

	/****************************************************/
	/* write directory information						*/
	/****************************************************/
	iRet = _fat_write_directory(ptDir, uiOffset, iNeed, uiNewClst,
						uiType, uiProtect, uiAttr, aucSName,
						pucComp, &ptFile);		/* write directory */
	if (iRet != 0) {							/* error occured */
		if (ptOrgFile) {						/* original file exists */
			grp_fs_unblock_file_op(ptOrgFile);	/* unlock original file */
			if (ptOrgDir != ptDir)				/* org dir != dst dir */
				grp_fs_unblock_file_op(ptOrgDir);/* unlock original directory */
			ptFile = NULL;						/* reset file pointer */
		}
		goto err_out;							/* return error */
	}

#ifdef GRP_FS_UPDATE_ARCHIVE
	/****************************************************/
	/* added archive attribute							*/
	/****************************************************/
	if (ptFile) {								/* valid file infomation */
		ptFile->uiAttr |= FAT_ATTR_ARCHIVE;		/* set archive */
		ptFile->usStatus |= GRP_FS_FSTAT_UPD_ATTR;
	}
#endif /* GRP_FS_UPDATE_ARCHIVE */

#ifdef	GRP_FS_FNAME_CACHE
	/****************************************************/
	/* set file name cache								*/
	/****************************************************/
	ptOpen = ptFile->pvFileInfo;				/* open info */
	if (iFnType != 0) {							/* not short name */
		grp_fs_to_upper(pucComp, pucComp);		/* covert to upper case */
		ptCache = grp_fs_set_fname_cache(ptDir, pucComp, ptFile, NULL);
		if (ptCache)							/* cache set */
			ptOpen->ptFnCache = ptCache;		/* set file name cache info */
	}
	ptCache = grp_fs_set_fname_cache(ptDir, aucSName, ptFile, ptCache);
	if (ptCache)								/* cache set */
		ptOpen->ptFnCache = ptCache;			/* set file name cache info */
#endif	/* GRP_FS_FNAME_CACHE */

	/****************************************************/
	/* release directory and created file, and return	*/
	/****************************************************/
	grp_fs_ftrace(ptDir->iDev, ptFile->uiFid, ptDir->uiFid,
					((uiOffset + iNeed) << 16) + uiOffset, ptFile, pucComp,
					(ptOrgFile)? GRP_FS_FT_RNF: GRP_FS_FT_CRF);
												/* trace */
	grp_fs_unblock_file_write(ptDir);			/* unblock directory write */
	grp_fs_close_file(ptDir, GRP_FS_FILE_UNBLOCK);/* close directory */
	*pptFile = ptFile;							/* set opened file */
	return(0);									/* return success */

err_out:
	*pptFile = NULL;							/* clear file info */
	if (uiNewClst != FAT_EOF_CLST && ptOrgFile == NULL) {
		if (!FAT_IS_SIZE0_FID(uiNewClst)) {		/* allocate cluster */
			_fat_free_cluster(ptFs, NULL, uiNewClst, &uiNewClst); /* free it */
		} else if (ptOpen) {					/* allocate open info */
			_fat_deque_size0_list(ptOpen);		/* deque from size 0 list */
			_fat_free_open_info(ptOpen);		/* free it */
		}
	}
	if (iBlockDirWrite)							/* blocked file write */
		grp_fs_unblock_file_write(ptDir);		/* unblock directory write */
	if (ptFile)									/* file opened */
		grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* close it */
	if (ptDir)									/* directory opened */
		grp_fs_close_file(ptDir, GRP_FS_FILE_UNBLOCK);/* close it */
	return(iRet);								/* return error */
}

/****************************************************************************/
/* FUNCTION:	_fat_create													*/
/*																			*/
/* DESCRIPTION:	Create a FAT file or directory								*/
/* INPUT:		ptFs:		file system information							*/
/*				ptDir:		search base directory							*/
/*				pucPath:	path name										*/
/*				uiType:		type of file									*/
/*				uiProtect:	protection										*/
/*				uiAttr:		FS dependent attribute							*/
/*				pucComp:	component buffer								*/
/* OUTPUT:		pptFile:	created file information						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GF_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_BAD_NAME: bad file name							*/
/*				GRP_FS_ERR_TOO_LONG: too long file name						*/
/*				GRP_FS_ERR_BAD_TYPE: bad type								*/
/*				GRP_FS_ERR_NOT_FOUND: not found parent directory 			*/
/*				GRP_FS_COMP_MIDDLE:	search not complete due to cross FS		*/
/*				GRP_FS_ERR_EXIST:	already exist							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_create(
	grp_fs_info_t		*ptFs,				/* [IN]  file system information */
	grp_fs_file_t		*ptDir,				/* [IN]  search base directory */
	const grp_uchar_t	**ppucPath,			/* [IN]  path name */
	grp_uint32_t		uiType,				/* [IN]  file type */
	grp_uint32_t		uiProtect,			/* [IN]  protection */
	grp_uint32_t		uiAttr,				/* [IN]  FS dependent attribute */
	grp_uchar_t			*pucComp,			/* [IN]  component buffer */
	grp_fs_file_t		**pptFile)			/* [OUT] created file information */
{
	int					iRet;				/* return value */
	grp_fs_file_t		*ptDstDir = NULL;	/* parent directory */

	/****************************************************/
	/* check type and attributes						*/
	/****************************************************/
	if (uiType != GRP_FS_FILE_FILE && uiType != GRP_FS_FILE_DIR)
		return(GRP_FS_ERR_BAD_TYPE);		/* bad type info */
	uiAttr &= (FAT_ATTR_HIDDEN|FAT_ATTR_SYSTEM|FAT_ATTR_ARCHIVE);
	uiProtect |= GRP_FS_PROT_RALL;			/* set read bit */
	if ((uiProtect & GRP_FS_PROT_WUSR) == 0) { /* not writable */
		uiAttr |= FAT_ATTR_RONLY;			/* set read only bit */
		uiProtect &= ~GRP_FS_PROT_WALL;		/* clear write bit */
	} else
		uiProtect |= GRP_FS_PROT_WALL;		/* set write bit */
	if (uiType == GRP_FS_FILE_DIR) {		/* directory */
		uiAttr |= FAT_ATTR_DIR;				/* set derectory bit */
		uiProtect |= GRP_FS_PROT_XALL;		/* set search bit */
	} else
		uiProtect &= ~GRP_FS_PROT_XALL;		/* reset exec bit */
	if (uiAttr & FAT_ATTR_HIDDEN)			/* hidden file */
		uiProtect &= ~(GRP_FS_PROT_RWXG|GRP_FS_PROT_RWXO);
											/* clear group/other bits */

	/****************************************************/
	/* lookup parent directory							*/
	/****************************************************/
	iRet = grp_fs_file_open_common(ptFs, ptDir, ppucPath,
						GRP_FS_OPEN_WRITE|GRP_FS_OPEN_PARENT,
						pucComp, &ptDstDir);/* lookup directory */
	if (iRet != 0) {						/* error or cross FS */
		*pptFile = ptDstDir;				/* return cross FS file info */
		return(iRet);						/* return return value */
	}

	/****************************************************/
	/* create a file under the directory				*/
	/****************************************************/
	return(_fat_create_rename_file(ptFs, ptDstDir, pucComp,
					uiType, uiProtect, uiAttr, NULL, NULL, pptFile));
}

/****************************************************************************/
/* FUNCTION:	_fat_check_dir_empty										*/
/*																			*/
/* DESCRIPTION:	Check the directory file is empty							*/
/* INPUT:		ptDir:				directory file information				*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					the directory is empty					*/
/*				GRP_FS_ERR_BUSY:	not empty								*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*																			*/
/****************************************************************************/
static int
_fat_check_dir_empty(
	grp_fs_file_t	*ptDir)				/* [IN]  directory information */
{
	grp_fs_dir_ent_t tDirEnt;			/* directory entry information */
	grp_fs_bio_t	tBio;				/* buffer I/O information */
	int				iRet;				/* return value */
	grp_uchar_t		aucComp[FAT_COMP_SZ];/* component buffer */

	/****************************************************/
	/* lookup entries in the directory					*/
	/****************************************************/
	tDirEnt.pucName = aucComp;			/* set file name buffer */
	tDirEnt.sNameSize = FAT_COMP_SZ;	/* set name buffer size */
	tDirEnt.uiStart = 0;				/* start offset is 0 */
	tDirEnt.uiEnd = 0;					/* end offset is 0 */
	tBio.ptBuf = NULL;					/* no buffer */
	tBio.uiSize = 0;					/* no data */
	while ((iRet = _fat_get_next_dir_ent(ptDir, &tBio, &tDirEnt)) > 0) {
		if (tDirEnt.ucType == GRP_FS_FILE_LINK		/* long name */
			|| strcmp((char *)tDirEnt.pucName, ".") == 0	/* "." */
			|| strcmp((char *)tDirEnt.pucName, "..") == 0) {/* ".." */
			tDirEnt.sNameSize = FAT_COMP_SZ;		/* set name buffer size */
			tDirEnt.uiStart = tDirEnt.uiEnd;		/* advance to next */
			continue;								/* continue search */
		}
		iRet = GRP_FS_ERR_BUSY;						/* found short entry */
		break;										/* stop search */
	}
	_fat_clean_unref_buf(&tBio);		/* release buffer */
	return((iRet == 0)? 0: iRet);		/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_make_free_dir_ent										*/
/*																			*/
/* DESCRIPTION:	Set free entry mark on directory entries.					*/
/*				This routine is used as a memory copy routine for			*/
/*				_fat_write_file_internal, and modifies 1st byte of each		*/
/*				directory entry.											*/
/* INPUT:		pucSrc:				source buffer							*/
/*				iSize:				copy size								*/
/* OUTPUT:		pucDst:				copied data								*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		invalid copy size						*/
/*				others:				same as iSize							*/
/*																			*/
/****************************************************************************/
static grp_int32_t
_fat_make_free_dir_ent(
	grp_uchar_t		*pucDst,				/* [IN] destination buffer */
	grp_uchar_t		*pucSrc,				/* [IN] source buffer */
	grp_int32_t		iSize)					/* [IN] copy size */
{
	fat_dir_t		*ptFatDir;				/* FAT directory entry */
	fat_dir_t		*ptEndDir;				/* end of FAT directory entry */

	if (iSize % sizeof(fat_dir_t) != 0)		/* bad size */
		return(GRP_FS_ERR_IO);				/* return error */
	ptFatDir = (fat_dir_t *) pucDst;		/* destination FAT dir entry */
	ptEndDir = (fat_dir_t *)(&pucDst[iSize]);/* end of FAT dir entry */
	grp_fs_ftrace(0, 0, 0, 0, NULL, ptEndDir[-1].aucName, GRP_FS_FT_FRE);
	while (ptFatDir < ptEndDir)
		ptFatDir++->aucName[0] = FAT_DIR_FREE;/* free entry */
	return(iSize);							/* return size */
}

/****************************************************************************/
/* FUNCTION:	_fat_free_dir_ent											*/
/*																			*/
/* DESCRIPTION:	Free a directory entry										*/
/* INPUT:		ptDir:				directory file information				*/
/*				ptOpen:				Open file information					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_fat_free_dir_ent(
	grp_fs_file_t	*ptDir,					/* [IN]  directory information */
	fat_open_info_t	*ptOpen)				/* [IN]  open file information */
{
	grp_int32_t		iWrite;					/* written size */
	grp_int32_t		iSize;					/* size of directory entry */
	grp_uint32_t	uiBlk;					/* last directory block */
	grp_isize_t		iSuccess;				/* success write size */

	iSize =	ptOpen->uiDirEnd - ptOpen->uiDirStart;	/* size of entries */
	iWrite = _fat_write_file_internal(ptDir,
					ptOpen->uiDirStart, NULL, iSize,
					_fat_make_free_dir_ent,
					&uiBlk, &iSuccess);		/* free directory entries */
	if (iWrite != iSize)
		grp_fs_printf("FAT: free dir entry 0x%lx(0x%lx-0x%lx) failed (dev 0x%x)\n",
					(unsigned long)ptDir->uiFid,
					(unsigned long)ptOpen->uiDirStart,
					(unsigned long)ptOpen->uiDirEnd, ptDir->iDev);
}

/****************************************************************************/
/* FUNCTION:	_fat_unlink													*/
/*																			*/
/* DESCRIPTION:	Unlink a FAT file or directory								*/
/* INPUT:		ptFs:		file system information							*/
/*				ptDir:		search base directory							*/
/*				pucPath:	path name										*/
/*				pucComp:	component buffer								*/
/* OUTPUT:		pucComp:	component data									*/
/*				pptFile:	file information for cross FS					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_BAD_NAME: bad file name							*/
/*				GRP_FS_ERR_TOO_LONG: too long file name						*/
/*				GRP_FS_ERR_NOT_FOUND: not found parent directory			*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				GRP_FS_COMP_MIDDLE:	search not complete due to cross FS		*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_unlink(
	grp_fs_info_t		*ptFs,				/* [IN]  file system information */
	grp_fs_file_t		*ptDir,				/* [IN]  search base directory */
	const grp_uchar_t	**ppucPath,			/* [IN]  path name */
	grp_uchar_t			*pucComp,			/* [IN/OUT] component buffer */
	grp_fs_file_t		**pptFile)			/* [OUT] file information */
{
	int					iRet;				/* return value */
	grp_fs_file_t		*ptFile;			/* file to unlink */
	grp_fs_file_t		*ptDstDir = NULL;	/* parent directory */

	/****************************************************/
	/* lookup parent directory							*/
	/****************************************************/
	iRet = grp_fs_file_open_common(ptFs, ptDir, ppucPath,
						GRP_FS_OPEN_WRITE|GRP_FS_OPEN_PARENT,
						pucComp, &ptDstDir);/* lookup directory */
	if (iRet != 0) {						/* error or cross FS */
		*pptFile = ptDstDir;				/* return cross FS information */
		return(iRet);						/* return return value */
	}
	ptDstDir->iRefCnt++;					/* to protect not to release */

	/****************************************************/
	/* lookup file to unlink in the parent directroy	*/
	/* Note: ptDstDir is unlocked with reference count	*/
	/*		 decremented if target file is found		*/
	/****************************************************/
	iRet = _fat_match_comp(ptDstDir, pucComp, 1, &ptFile, 0, NULL);
											/* lookup file */
	if (iRet != 0) {						/* error detected */
		ptDstDir->iRefCnt--;				/* decrement extra reference */
		grp_fs_close_file(ptDstDir, GRP_FS_FILE_UNBLOCK); /* close it */
		return(iRet);						/* return error */
	}

	/****************************************************/
	/* check target file to unlink						*/
	/****************************************************/
	if (grp_fs_check_io_status(ptFs, 0, 0, GRP_FS_IO_REQ) != 0)
											/* device invalidated */
		iRet = GRP_FS_ERR_IO;				/* I/O error */
	else if ((strcmp((char *)pucComp, ".") == 0
		|| strcmp((char *)pucComp, "..") == 0)
		|| ptFile->uiFid == ptDstDir->uiFid)
		iRet = GRP_FS_ERR_BUSY;				/* return busy error */
	else if (ptFile->ucType == GRP_FS_FILE_DIR) /* directory file */
		iRet = _fat_check_dir_empty(ptFile); /* check directory is empty */
	if (iRet != 0) {
		grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);	/* close file */
		grp_fs_close_file(ptDstDir, 0);		/* close it */
		return(iRet);						/* return error */
	}

	/****************************************************/
	/* block file operation for the parent directory	*/
	/* Note: to avoid deadlock, lock for file is 		*/
	/*		 released before blocking direcotry,		*/
	/*		 and then, get the lock again.				*/
	/****************************************************/
	grp_fs_unblock_file_op(ptFile);			/* temporarily unblock */
	grp_fs_block_file_op(ptDstDir);			/* block file op for directory */
	grp_fs_block_file_op(ptFile);			/* block file op for file */

	/****************************************************/
	/* if the file has other references, return busy	*/
	/* error.											*/
	/****************************************************/
	if (ptFile->iRefCnt != 1) {				/* other references */
		grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);	/* close file */
		grp_fs_close_file(ptDstDir, GRP_FS_FILE_UNBLOCK); /* close directory */
		return(GRP_FS_ERR_BUSY);			/* return busy error */
	}

	/****************************************************/
	/* free directory entry								*/
	/****************************************************/
	_fat_free_dir_ent(ptDstDir, ptFile->pvFileInfo); /* free directory entry */

	/****************************************************/
	/* free clusters of the file						*/
	/****************************************************/
	if (!FAT_IS_SIZE0_FID(ptFile->uiFid))			/* not size 0 */
		(void)_fat_free_cluster_list(ptFile, ptFile->uiFid, 0, 0);

	/****************************************************/
	/* release directory and file							*/
	/****************************************************/
	grp_fs_ftrace(ptFile->iDev, ptFile->uiFid,
				((fat_open_info_t *)(ptFile->pvFileInfo))->uiDirFid,
				((fat_open_info_t *)(ptFile->pvFileInfo))->uiDirStart
				+ (((fat_open_info_t *)(ptFile->pvFileInfo))->uiDirEnd << 16),
				ptFile, pucComp, GRP_FS_FT_RMF);	/* trace */
	grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK|GRP_FS_FILE_INVALID);
	grp_fs_close_file(ptDstDir, GRP_FS_FILE_UNBLOCK); /* close directory */

	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_rename													*/
/*																			*/
/* DESCRIPTION:	Rename a FAT file or directory								*/
/* INPUT:		ptFs:		file system information							*/
/*				ptOldDir:	old parent directory							*/
/*				pucOld:		old component name 								*/
/*				ptNewDir:	new parent directory							*/
/*				pucNew:		new compnent name								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_BAD_NAME: bad file name							*/
/*				GRP_FS_ERR_TOO_LONG: too long file name						*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_rename(
	grp_fs_info_t		*ptFs,				/* [IN]  file system information */
	grp_fs_file_t		*ptOldDir,			/* [IN]  old parent directory */
	grp_uchar_t			*pucOld,			/* [IN]  old path name */
	grp_fs_file_t		*ptNewDir,			/* [IN]  new parent directory */
	grp_uchar_t			*pucNew)			/* [IN]  new path name */
{
	int					iRet;				/* return value */
	grp_fs_file_t		*ptOldFile;			/* file to unlink */
	grp_fs_file_t		*ptFile;			/* new file */
	fat_open_info_t		tOpenInfo;			/* open information */

	/****************************************************/
	/* lookup file to unlink in the old directroy		*/
	/* Note: ptOldDir is unlocked with reference count	*/
	/*		decremented if target file is found			*/
	/****************************************************/
	grp_fs_block_file_op(ptOldDir);			/* lock old parent directory */
	ptOldDir->iRefCnt++;					/* to protect not to release */
	iRet = _fat_match_comp(ptOldDir, pucOld, 1, &ptOldFile, 0, NULL);
											/* lookup file */
	if (iRet != 0) {						/* error detected */
		grp_fs_close_file(ptOldDir, GRP_FS_FILE_UNBLOCK);
											/* release extra ref */
		return(iRet);						/* return error */
	}

	/****************************************************/
	/* protect ".", ".." and referenced files from		*/
	/* rename											*/
	/****************************************************/
	if ((strcmp((char *)pucOld, ".") == 0 || strcmp((char *)pucOld, "..") == 0)
		|| ptOldFile->uiFid == ptOldDir->uiFid
		|| ptOldFile->iRefCnt != 1) {
		grp_fs_close_file(ptOldFile, GRP_FS_FILE_UNBLOCK); /* close file */
		return(GRP_FS_ERR_BUSY);			/* return busy error */
	}

	/****************************************************/
	/* release lock for old file not to deadlock		*/
	/****************************************************/
	grp_fs_unblock_file_op(ptOldFile);		/* unblock file operation */

	/****************************************************/
	/* link the old file to new one						*/
	/* Note: ptOldFile(ptFile) and ptOldDir are locked,	*/
	/*		 and ptNewDir is dereferenced and unblocked */
	/*		 in success return.							*/
	/****************************************************/
	tOpenInfo = *((fat_open_info_t *)ptOldFile->pvFileInfo);
											/* save open information */
	grp_fs_block_file_op(ptNewDir);			/* lock new directory */
	ptNewDir->iRefCnt++;					/* increment to keep reference */
	iRet = _fat_create_rename_file(
					ptFs, ptNewDir, pucNew,
					(grp_uint32_t)ptOldFile->ucType, ptOldFile->uiProtect,
					ptOldFile->uiAttr, ptOldFile, ptOldDir,
					&ptFile);				/* link the old to new one */
	if (iRet != 0) {						/* error occured */
		grp_fs_close_file(ptOldFile, 0);	/* close file */
		return(iRet);						/* return error */
	} else {								/* success */
		grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);/* close original file */
	}

	/****************************************************/
	/* free old directory entry							*/
	/****************************************************/
	_fat_free_dir_ent(ptOldDir, &tOpenInfo);	/* free directory entry */

	/****************************************************/
	/* release old parent directory						*/
	/****************************************************/
	grp_fs_unblock_file_op(ptOldDir);		/* unlock old parent directory */

	return(0);								/* return success */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_attr												*/
/*																			*/
/* DESCRIPTION:	Get file attribute											*/
/* INPUT:		ptFile:		file information								*/
/* OUTPUT:		ptAttr:		file attribute information without name			*/
/*																			*/
/* RESULT:		0:					success									*/
/*				GRP_FS_ERR_IO		I/O error								*/
/*				GRP_FS_ERR_FS		bad file system							*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*																			*/
/****************************************************************************/
static int
_fat_get_attr(
	grp_fs_file_t		*ptFile,			/* [IN]  file information */
	grp_fs_dir_ent_t	*ptAttr)			/* [OUT] file attribute info */
{
	int					iRet = 0;			/* return value */
	fat_open_info_t		*ptOpen;			/* open information */

	ptAttr->iDev = ptFile->iDev;			/* set device number */
	ptAttr->uiFid = FAT_IS_SIZE0_FID(ptFile->uiFid)?
					0: ptFile->uiFid;		/* set file id */
	ptAttr->ucType = ptFile->ucType;		/* set file type */
	ptAttr->uiProtect = ptFile->uiProtect;	/* set file protection */
#ifdef GRP_FS_ENABLE_OVER_2G
	if (ptFile->ucType == GRP_FS_FILE_DIR)	/* directory file */
		iRet = _fat_get_dir_size(ptFile, &ptAttr->uiSize); /* get actual size */
	else									/* not directory file */
		ptAttr->uiSize = ptFile->uiSize;	/* set file size */
#else  /* GRP_FS_ENABLE_OVER_2G */
	if (ptFile->ucType == GRP_FS_FILE_DIR)	/* directory file */
		iRet = _fat_get_dir_size(ptFile, &ptAttr->iSize); /* get actual size */
	else									/* not directory file */
		ptAttr->iSize = ptFile->iSize;		/* set file size */
#endif /* GRP_FS_ENABLE_OVER_2G */
	ptAttr->iCTime = ptFile->iCTime;		/* set file creation time */
	ptAttr->iMTime = ptFile->iMTime;		/* set file modify time */
	ptAttr->iATime = ptFile->iATime;		/* set file access time */
	ptAttr->uiAttr = ptFile->uiAttr;		/* set file attribute */
	ptAttr->uiMisc = 0;						/* misc info (reserved) */
	ptOpen = ptFile->pvFileInfo;			/* open info */
	ptAttr->uiStart = ptOpen->uiDirStart;	/* directory start offset */
	ptAttr->uiEnd = ptOpen->uiDirEnd; 		/* directory end offset */
	return(iRet);							/* return */
}

/****************************************************************************/
/* FUNCTION:	_fat_set_attr												*/
/*																			*/
/* DESCRIPTION:	Set file attribute											*/
/* INPUT:		ptFile:		file information								*/
/*				ptAttr:		file attribute information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_set_attr(
	grp_fs_file_t		*ptFile,			/* [IN]  file information */
	grp_fs_dir_ent_t	*ptAttr)			/* [IN]  file attribute info */
{
	grp_uint32_t		uiAttr;				/* attribute */
	grp_uint32_t		uiProtect;			/* protection */

	uiAttr = ptAttr->uiAttr;				/* attribute */
	uiAttr &= (FAT_ATTR_HIDDEN|FAT_ATTR_SYSTEM|FAT_ATTR_ARCHIVE);
	uiProtect = GRP_FS_PROT_RALL;			/* set read bit */
	if (ptAttr->uiProtect & GRP_FS_PROT_WUSR) /* writeable mode */
		uiProtect |= GRP_FS_PROT_WALL;		/* set write bit */
	else
		uiAttr |= FAT_ATTR_RONLY;			/* set read only bit */
	if (ptFile->uiProtect & GRP_FS_PROT_XUSR) /* exec bit in original */
		uiProtect |= GRP_FS_PROT_XALL;		/* set exec bit */
	if (uiAttr & FAT_ATTR_HIDDEN)			/* hidden file */
		uiProtect &= ~(GRP_FS_PROT_RWXG|GRP_FS_PROT_RWXO);
											/* clear group and other bits */
	ptFile->uiProtect = uiProtect;			/* set protection */
	if (ptFile->ucType == GRP_FS_FILE_DIR)	/* directory */
		uiAttr |= FAT_ATTR_DIR;				/* set derectory bit */
	ptFile->uiAttr = uiAttr;				/* set attribute */
	ptFile->iCTime = ptAttr->iCTime;		/* creation time */
	ptFile->iMTime = ptAttr->iMTime;		/* modify time */
	ptFile->iATime = ptAttr->iATime;		/* access time */
	ptFile->usStatus |= GRP_FS_FSTAT_UPD_BITS; /* set update flags */
	return(0);
}

/****************************************************************************/
/* FUNCTION:	_fat_truncate												*/
/*																			*/
/* DESCRIPTION:	Truncate a FAT file											*/
/* INPUT:		ptFile:				file information						*/
/*				uiFBlk:				file block number						*/
/*				uiBlkOff:			FS block offset							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_PERMIT:	permission denied						*/
/*				GRP_FS_ERR_BAD_OFF:	bad offset								*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					success									*/
/*																			*/
/****************************************************************************/
static int
_fat_truncate(							/* truncate a file */
	grp_fs_file_t	*ptFile,			/* [IN]  file information */
	grp_uint32_t	uiFsBlk,			/* [IN]  FS block number */
	grp_uint32_t	uiBlkOff)			/* [IN]  FS block offset */
{
	grp_fs_info_t	*ptFs = ptFile->ptFs;			/* FS information */
	fat_BPB_t		*ptBPB = ptFs->pvFsInfo;		/* BPB information */
	grp_uint32_t	uiDataClst;						/* data cluster number */
	grp_uint32_t	uiFendBlk;						/* file end block number */
	grp_uint32_t	uiFendOff;						/* file end block offset */
	grp_uint32_t	uiNClst;						/* # of req clusters */
	grp_uint32_t	uiNFClst;						/* # of file clusters */
	int				iRet = 0;						/* return value */

	/****************************************************/
	/* check write access to directory					*/
	/****************************************************/
	if ((ptFile->ucType == GRP_FS_FILE_DIR)			/* directory file */
		|| (ptFs->usStatus & GRP_FS_STAT_RONLY))	/* read only file system */
		return(GRP_FS_ERR_PERMIT);					/* permission denied */

	/****************************************************/
	/* check size and offset 							*/
	/****************************************************/
	grp_fs_block_file_write(ptFile);				/* block write */
	grp_fs_set_access_time(ptFile);					/* set access time */
#ifdef GRP_FS_ENABLE_OVER_2G
	uiFendBlk = FAT_DBLK(ptBPB, ptFile->uiSize);	/* file end block */
	uiFendOff = FAT_DBLK_OFF(ptBPB, ptFile->uiSize); /* file end offset */
#else  /* GRP_FS_ENABLE_OVER_2G */
	uiFendBlk = FAT_DBLK(ptBPB, ptFile->iSize);		/* file end block */
	uiFendOff = FAT_DBLK_OFF(ptBPB, ptFile->iSize);	/* file end offset */
#endif /* GRP_FS_ENABLE_OVER_2G */
	if (uiFendBlk < uiFsBlk
		|| (uiFendBlk == uiFsBlk && uiBlkOff > uiFendOff)) { /* over file end */
		iRet = GRP_FS_ERR_BAD_OFF;					/* bad offset */
		goto out;									/* return error */
	}

	/****************************************************/
	/* release cluster if necessary						*/
	/****************************************************/
	uiNClst = FAT_DBLK_CLST(ptBPB, uiFsBlk);		/* # of req clusters */
	if (uiBlkOff || FAT_DBLK_CLST_OFF(ptBPB, uiFsBlk)) /* partial cluster */
		uiNClst++;									/* increment count */
#ifdef GRP_FS_ENABLE_OVER_2G
	uiNFClst = FAT_NCLST(ptBPB, ptFile->uiSize);	/* file cluster count */
#else  /* GRP_FS_ENABLE_OVER_2G */
	uiNFClst = FAT_NCLST(ptBPB, ptFile->iSize);		/* file cluster count */
#endif /* GRP_FS_ENABLE_OVER_2G */
	if (uiNClst != uiNFClst) {						/* need to release */
		/****************************************************/
		/* get physical data cluster number					*/
		/****************************************************/
		iRet = _fat_phys_cluster(ptFile, uiNClst, &uiDataClst);
		if (iRet < 0)								/* error occured */
			goto out;								/* return error */
		if (uiDataClst == FAT_EOF_CLST) {			/* EOF */
			iRet = GRP_FS_ERR_FS;					/* bad file system */
			goto out;								/* return error */
		}

		/****************************************************/
		/* free clusters (update size even in error return)	*/
		/****************************************************/
		iRet = _fat_free_cluster_list(ptFile, uiDataClst,
					uiNFClst - uiNClst, uiNClst);	/* free clusters */
	}

	/****************************************************/
	/* update file size information if necessary		*/
	/****************************************************/
	if (uiFendBlk != uiFsBlk || uiBlkOff != uiFendOff) { /* size changed */
#ifdef GRP_FS_ENABLE_OVER_2G
		ptFile->uiSize = (uiFsBlk << ptFs->ucFsDBlkShift) + uiBlkOff;
		if (ptFile->uiSize == 0) {					/* become 0 */
#else  /* GRP_FS_ENABLE_OVER_2G */
		ptFile->iSize = (uiFsBlk << ptFs->ucFsDBlkShift) + uiBlkOff;
		if (ptFile->iSize == 0) {					/* become 0 */
#endif /* GRP_FS_ENABLE_OVER_2G */
			iRet = _fat_free_1st_cluster(ptFile);	/* clear 1st cluster */
		} else {									/* not size 0 */
			grp_fs_get_current_time(&ptFile->iMTime);/* set modify time */
			ptFile->iATime = ptFile->iMTime;		/* set access time */
			ptFile->usStatus |= GRP_FS_FSTAT_UPD_BITS;/* set update flags */
			if (ptFs->usStatus & GRP_FS_STAT_SYNC_ALL) /* sync all */
				iRet = _fat_update_attr(ptFile);	/* update file attribute */
		}
	}
out:
	grp_fs_unblock_file_write(ptFile);				/* unblock write */
	return(iRet);									/* return result */
}

/****************************************************************************/
/* FUNCTION:	_fat_get_dirent												*/
/*																			*/
/* DESCRIPTION:	Get directory entry											*/
/* INPUT:		ptDir:				directory information					*/
/*				ptDirEnt->uiStart:	start offset							*/
/*				ptDirEnt->sNameSize: name buffer							*/
/* OUTPUT:		ptDirEnt:			directory entry							*/
/*																			*/
/* RESULT:		GRP_FS_ERR_IO:		device I/O error						*/
/*				GRP_FS_ERR_FS:		bad file system							*/
/*				GRP_FS_ERR_BAD_DIR:	bad direcotry file						*/
/*				GRP_FS_ERR_BAD_PARAM: invalid parameter						*/
/*				GRP_FS_ERR_NOMEM:	no valid buffer							*/
/*				0:					EOF										*/
/*				positive:			directory entry size					*/
/*																			*/
/****************************************************************************/
static int
_fat_get_dirent(
	grp_fs_file_t	*ptDir,					/* [IN]  directory information */
	grp_fs_dir_ent_t *ptDirEnt)				/* [IN/OUT] directory entry */
{
	int				iRet;					/* return value */
	int				iLookupRet = 0;			/* lookup return */
	grp_fs_file_t	*ptFile;				/* file information */
	grp_fs_bio_t	tBio;					/* buffer I/O information */
	fat_BPB_t		*ptBPB = ptDir->ptFs->pvFsInfo;	/* BPB information */
	int				iDotDot;				/* ".." file */
#if(GRP_FS_MINIMIZE_LEVEL > 1)
	short			sNameSize;				/* backup sNameSize */
#endif /* GRP_FS_MINIMIZE_LEVEL > 1 */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
	grp_uint32_t	uiStart = ptDirEnt->uiStart;	/* start offset */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
#else  /* GRP_FS_MINIMIZE_LEVEL < 2 */
	grp_uint32_t	uiEntSize = 0;			/* directory entry size */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
	grp_uint32_t	uiDirBlk;				/* directory block number */
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */

	if (ptDir->ucType != GRP_FS_FILE_DIR)	/* not directory */
		return(GRP_FS_ERR_BAD_DIR);			/* return error */
	if (ptDirEnt->uiStart % sizeof(fat_dir_t)) /* invalid offset */
		return(GRP_FS_ERR_BAD_PARAM);
	tBio.ptBuf = NULL;						/* no buffer */
	tBio.uiSize = 0;						/* no data */
#if(GRP_FS_MINIMIZE_LEVEL > 1)
	sNameSize = ptDirEnt->sNameSize;		/* backup sNameSize */
#endif /* GRP_FS_MINIMIZE_LEVEL > 1 */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
	tBio.pucData = NULL;					/* no data */
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	iRet = _fat_get_next_dir_ent(ptDir, &tBio, ptDirEnt);
#else  /* GRP_FS_MINIMIZE_LEVEL < 2 */
	while ((iRet = _fat_get_next_dir_ent(ptDir, &tBio, ptDirEnt)) > 0) {
		if (ptDirEnt->ucType == GRP_FS_FILE_LINK) { /* long name */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
			uiEntSize = (grp_uint32_t)iRet;			/* reset directory entry size */
#endif /* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
			ptDirEnt->uiStart = ptDirEnt->uiEnd;	/* skip long directory entry */
			ptDirEnt->sNameSize = sNameSize;		/* re-store sNameSize */
		}
		else {
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
			uiEntSize += (grp_uint32_t)iRet;			/* add directory entry size */
			if (uiStart != ptDirEnt->uiStart) {
				uiStart = ptDirEnt->uiEnd - uiEntSize;	/* calculate start offset */
				ptDirEnt->uiStart = uiStart;			/* re-store start offset */
			}
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
			break;
		}
	}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
	uiDirBlk = (tBio.pucData)? tBio.uiBlk: 0; /* set block */
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
	if (tBio.ptBuf)							/* buffer exists */
		_fat_clean_unref_buf(&tBio);		/* release buffer */
	if (iRet > 0) {							/* got entry */
		/****************************************************/
		/* check file cache to get up-to-date information	*/
		/****************************************************/
		if (ptDirEnt->ucType != GRP_FS_FILE_FILE	/* not regular file */
			&& ptDirEnt->ucType != GRP_FS_FILE_DIR) {/* not directory */
			ptFile = NULL;					/* no file cache info */
		} else if (ptDir->uiFid == ptDirEnt->uiFid	/* equal to parent */
					&& ptDirEnt->ucType == GRP_FS_FILE_DIR) { /* directory */
			ptFile = ptDir;					/* use parent cache info */
		} else  {							/* directory or file */
			grp_uint32_t uiFid = ptDirEnt->uiFid;	/* file id */
			iDotDot = (strcmp((char *)ptDirEnt->pucName, "..") == 0); /* ".." */
			if (uiFid == 0) {				/* root or size 0 file */
				if (!iDotDot				/* not  ".." */
					&& ptDirEnt->ucType == GRP_FS_FILE_FILE) {	 /* file */
					/****************************************************/
					/* size 0 file;										*/
					/*   lookup unique file id for it if already opened	*/
					/****************************************************/
					uiFid = _fat_lookup_size0_file(ptDir->iDev, ptDir->uiFid,
										ptDirEnt->uiEnd, NULL);
												/* get uniq file id for it */
					if (uiFid == FAT_EOF_CLST)	/* not found */
						goto no_opened_file;	/* return error */
				} else if (ptBPB->uiRootClst)	/* root has cluster */
					uiFid = ptBPB->uiRootClst;	/* use the number */
			}
			if (uiFid == ptDir->uiFid) {		/* same as parent */
				ptFile = ptDir;					/* use parent cache info */
				goto check_cache;				/* check file cache */
			}
			if (iDotDot)						/* ".." file */
				grp_fs_unblock_file_op(ptDir);	/* temporary unblock file op */
#ifdef	GRP_FS_FAT_NO_DIR_SIZE_INFO
			iLookupRet = grp_fs_lookup_file_ctl(ptDir->ptFs, uiFid,
								0, &ptFile);/* lookup target file cache */
			if (iDotDot)						/* ".." file */
				grp_fs_block_file_op(ptDir);	/* block file op again */
#else	/* GRP_FS_FAT_NO_DIR_SIZE_INFO */
			iLookupRet = grp_fs_lookup_file_ctl(ptDir->ptFs, uiFid,
								(ptDirEnt->ucType == GRP_FS_FILE_DIR),
								&ptFile);	/* lookup target file cache */
			if (iDotDot)						/* ".." file */
				grp_fs_block_file_op(ptDir);	/* block file op again */
			if (iLookupRet != 0 && ptFile) {
				fat_open_info_t *ptOpen;	/* open info */
				ptFile->ucType = GRP_FS_FILE_DIR;/* directory file */
				ptOpen = _fat_alloc_open_info(ptDir->iDev);/* alloc open info */
				if (ptOpen == NULL) {		/* still no info */
					iRet = GRP_FS_ERR_TOO_MANY;/* too many open file error */
					ptFile->usStatus |= GRP_FS_FSTAT_INVALID;
											/* invalid file data */
					grp_fs_close_file(ptFile,
							GRP_FS_FILE_INVALID|GRP_FS_FILE_UNBLOCK);
					ptFile = NULL;			/* clear file info pointer */
				} else {					/* allocate open information */
					ptFile->pvFileInfo = ptOpen; /* set open information */
					ptFile->puiMap = ptOpen->uiMap; /* set map */
				}
			}
#endif	/* GRP_FS_FAT_NO_DIR_SIZE_INFO */
		}
check_cache:
		if (ptFile) {						/* cache exists */
			if (ptFile->usStatus & GRP_FS_FSTAT_MOUNT) { /* mounted on */
				grp_uint32_t uiStart = ptDirEnt->uiStart;/* save start */
				grp_uint32_t uiEnd = ptDirEnt->uiEnd;	/* save end */
				int	iAttrRet = grp_fs_get_mount_root_attr(ptFile, ptDirEnt);
												/* get attr of mounted root */
				grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK); /* release it */
				if (iAttrRet != 0)				/* error in get attr */
					iRet = iAttrRet;			/* set return value */
				ptDirEnt->uiStart = uiStart;	/* set back start */
				ptDirEnt->uiEnd = uiEnd;		/* set back end */
				goto out;						/* goto out */
			}
#ifndef	GRP_FS_FAT_NO_DIR_SIZE_INFO
			if (ptFile->ucType == GRP_FS_FILE_DIR) { /* directory file */
#ifdef GRP_FS_ENABLE_OVER_2G
				int iDirRet = _fat_get_dir_size(ptFile, &ptDirEnt->uiSize);
												/* get actual size */
#else  /* GRP_FS_ENABLE_OVER_2G */
				int iDirRet = _fat_get_dir_size(ptFile, &ptDirEnt->iSize);
												/* get actual size */
#endif /* GRP_FS_ENABLE_OVER_2G */
				if (iDirRet != 0)				/* error to get size */
					iRet = iDirRet;				/* set error */
			} else								/* not directory file */
#endif	/* GRP_FS_FAT_NO_DIR_SIZE_INFO */
#ifdef GRP_FS_ENABLE_OVER_2G
				ptDirEnt->uiSize = ptFile->uiSize;/* copy size information */
#else  /* GRP_FS_ENABLE_OVER_2G */
				ptDirEnt->iSize = ptFile->iSize;/* copy size information */
#endif /* GRP_FS_ENABLE_OVER_2G */
			if (iLookupRet != 0) {				/* temporary allocated file */
				ptFile->usStatus |= GRP_FS_FSTAT_INVALID;/* invalid file data */
				grp_fs_close_file(ptFile,
							GRP_FS_FILE_INVALID|GRP_FS_FILE_UNBLOCK);
			} else {							/* real file information */
				ptDirEnt->uiProtect = ptFile->uiProtect;/* copy protect info */
				ptDirEnt->iCTime = ptFile->iCTime;	/* copy create time info */
				ptDirEnt->iMTime = ptFile->iMTime;	/* copy modify time info */
				ptDirEnt->iATime = ptFile->iATime;	/* copy access time info */
				ptDirEnt->uiAttr = ptFile->uiAttr;	/* copy attribute info */
				if (ptFile != ptDir)			/* not parent */
					grp_fs_close_file(ptFile, GRP_FS_FILE_UNBLOCK);/* release */
			}
		}

no_opened_file: ;
#ifdef	GRP_FS_FAT_CACHE_BY_GET_DIRENT
		if ((ptBPB->uiDirFid == ptDir->uiFid
			&& ptBPB->tDirCache.uiEnd == uiStart)	/* lookup next one */
			|| uiStart == 0) {						/* lookup new directory */
			switch(ptDirEnt->ucType) {				/* file type */
			case GRP_FS_FILE_FILE:					/* regular file */
			case GRP_FS_FILE_DIR:					/* directory */
				if (uiStart != 0
					&& ptBPB->tDirCache.ucType == GRP_FS_FILE_LINK) {
					if (ptBPB->tDirCache.uiMisc != ptDirEnt->uiMisc)
						goto out;					/* not match checksum */
				} else {							/* not long previous */
					ptBPB->uiDirStart = ptDirEnt->uiStart; /* update start */
					ptBPB->aucLCacheName[0] = 0;	/* clear long name */
				}
				strcpy((char *)ptBPB->aucSCacheName, (char *)ptDirEnt->pucName);
				break;
			case GRP_FS_FILE_LINK:					/* long file name */
				ptBPB->uiDirStart = ptDirEnt->uiStart; /* update start */
				strcpy((char *)ptBPB->aucLCacheName, (char *)ptDirEnt->pucName);
				ptBPB->aucSCacheName[0] = 0;		/* clear short name */
				break;
			default:								/* others */
				ptBPB->aucSCacheName[0] = 0;		/* clear short name */
				ptBPB->aucLCacheName[0] = 0;		/* clear long name */
				break;
			}
			ptBPB->uiDirFid = ptDir->uiFid;			/* set directory file id */
			ptBPB->uiDirBlk = uiDirBlk;				/* set block */
			ptBPB->tDirCache = *ptDirEnt;			/* copy entry info */
			if (ptDirEnt->ucType == GRP_FS_FILE_DIR)/* directory */
#ifdef GRP_FS_ENABLE_OVER_2G
				ptBPB->tDirCache.uiSize = 0;		/* clear size info */
#else  /* GRP_FS_ENABLE_OVER_2G */
				ptBPB->tDirCache.iSize = 0;			/* clear size info */
#endif /* GRP_FS_ENABLE_OVER_2G */
		}
#endif	/* GRP_FS_FAT_CACHE_BY_GET_DIRENT */
	}
out:
	return(iRet);
}

/****************************************************************************/
/* FUNCTION:	_fat_check_volume											*/
/*																			*/
/* DESCRIPTION:	Check volume												*/
/* INPUT:		iDev:				device number							*/
/*				ptFs:				file system information					*/
/* OUTPUT:		pucVolName:			volume name								*/
/*				puiVolSerNo:		volume serial number					*/
/*																			*/
/* RESULT:		GRP_FS_ERR_BAD_DEV:	bad device number						*/
/*				GRP_FS_ERR_IO:		I/O error								*/
/*				GRP_FS_ERR_NOMEM:	cannot allocate I/O buffer				*/
/*				FAT_VOL_LAB_LEN:	length of volume label					*/
/*																			*/
/****************************************************************************/
static int
_fat_check_volume(
	int				iDev,					/* [IN]  device number */
	grp_fs_info_t	*ptFs,					/* [IN]  file system information */
	grp_uchar_t		*pucVolName,			/* [OUT] volume name */
	grp_uint32_t	*puiVolSerNo)			/* [OUT] serial number */
{
	int				iMajor;					/* major device number */
	grp_fs_dev_tbl_t *ptDev;				/* device table */
	grp_fs_dev_op_t	*ptDevOp;				/* device operation */
	grp_int32_t		iDevHdl;				/* device handle */
	grp_uint32_t	uiDevOff;				/* start offset */
	grp_uint32_t	uiDevSize;				/* device size */
	int				iBlkShift;				/* block shift count */
	grp_int32_t		iRead;					/* read size */
	grp_uchar_t		*pucVolLab;				/* volume label */
	grp_uchar_t		*pucVolSer;				/* volume serial number */
	int				iRet;					/* return value */
	grp_int32_t		iIoSize;				/* I/O size */
	grp_uchar_t		*pucBuf;				/* buffer pointer */
#ifdef	GRP_FAT_USE_LOCAL_BUF
	grp_uchar_t		aucBuf[FAT_BLK_SZ];		/* I/O buffer */
#endif	/* GRP_FAT_USE_LOCAL_BUF */

	/****************************************************/
	/* check device number and block number				*/
	/****************************************************/
	iMajor = GRP_FS_DEV_MAJOR(iDev);			/* major device number */
	ptDev = &grp_fs_dev_tbl[iMajor];			/* device table entry */
	if ((iMajor < 0 || iMajor >= grp_fs_dev_tbl_cnt) /* bad devince number */
		|| (ptDevOp = ptDev->ptOp) == NULL) 	/* no operation */
		return(GRP_FS_ERR_BAD_DEV);				/* return error */

	/****************************************************/
	/* open device if necessary							*/
	/****************************************************/
	if (ptFs == NULL) {							/* not opened */
		iRet = ptDev->ptOp->pfnOpen(iDev, 0,
							&iDevHdl, &uiDevOff,
							&uiDevSize, &iBlkShift);/* open device */
		if (iRet < 0)							/* open error */
			return(iRet);						/* return error */
	} else {									/* already opened */
		iRet = ptDev->ptOp->pfnOpen(iDev, 0,
							&iDevHdl, &uiDevOff,
							&uiDevSize, &iBlkShift);/* open device */
		if (iRet < 0)							/* open error */
			return(iRet);						/* return error */
		ptDevOp->pfnClose(iDevHdl, iDev);		/* close device */
		if ( (uiDevOff  != ptFs->uiDevOff ) ||	/* check parameter */
			 (uiDevSize != ptFs->uiDevSize) ||
			 (iBlkShift != (int)ptFs->ucDevBlkShift) )
			return(GRP_FS_ERR_BAD_DEV);			/* return error */
		iDevHdl = ptFs->iDevHandle;				/* device handle */
		uiDevOff = ptFs->uiDevOff;				/* start offset */
		iBlkShift = (int)ptFs->ucDevBlkShift;	/* block shift */
	}

	/****************************************************/
	/* allocate buffer if necessay						*/
	/****************************************************/
	iIoSize = ((grp_int32_t)1 << iBlkShift);	/* sector size */
#ifdef	GRP_FAT_USE_LOCAL_BUF
	if (iIoSize > sizeof(aucBuf)) {				/* bigger than local buffer */
		pucBuf = grp_mem_alloc(iIoSize);		/* allocate buffer */
		if (pucBuf == NULL)	{					/* failed to allocate */
			iRet = GRP_FS_ERR_NOMEM;			/* set error number */
			goto out;							/* return error */
		}
	} else {									/* fit in local buffer */
		pucBuf = aucBuf;						/* use local buffer */
		iIoSize = sizeof(aucBuf);				/* set I/O size */
	}
#else	/* GRP_FAT_USE_LOCAL_BUF */
	if (iIoSize < FAT_BLK_SZ) {					/* less than FAT block size */
		iIoSize = FAT_BLK_SZ;					/* use FAT block size */
	}
	pucBuf = grp_mem_alloc(iIoSize);			/* allocate buffer */
	if (pucBuf == NULL)	{						/* failed to allocate */
		iRet = GRP_FS_ERR_NOMEM;				/* set error number */
		goto out;								/* return error */
	}
#endif	/* GRP_FAT_USE_LOCAL_BUF */
	iIoSize >>= iBlkShift;						/* make I/O size to sectors */

	/****************************************************/
	/* read data into buffer,							*/
	/* and close device if necessary					*/
	/****************************************************/
	iRead = ptDevOp->pfnRead(iDevHdl, iDev, uiDevOff, pucBuf, iIoSize);
												/* read data */
	if (iRead != iIoSize) {						/* I/O error */
		if (iRead >= 0)							/* not error number */
			iRet = GRP_FS_ERR_IO;				/* set error number */
		else									/* error number */
			iRet = (int)iRead;					/* use it as return value */
		goto out_with_buf;						/* return error */
	}

	/****************************************************/
	/* check volum label and serial number				*/
	/****************************************************/
	if (pucBuf[FAT_12_16_SZ] == 0 && pucBuf[FAT_12_16_SZ + 1] == 0) {
		pucVolLab = pucBuf + FAT_32_VOL_LAB;	/* volume label */
		pucVolSer = pucBuf + FAT_32_VOL_SER;	/* volume serial number */
	} else {
		pucVolLab = pucBuf + FAT_12_16_VOL_LAB;	/* volume label */
		pucVolSer = pucBuf + FAT_12_16_VOL_SER;	/* volume serial number */
	}
	memcpy((char *)pucVolName, (char *)pucVolLab, FAT_VOL_LAB_LEN);
												/* copy volume name */
	if (GRP_FS_VOL_NAME_LEN > FAT_VOL_LAB_LEN)	/* area is bigger */
		pucVolName[FAT_VOL_LAB_LEN] = 0;		/* null terminate */
	memcpy((char *)puiVolSerNo, (char *)pucVolSer, sizeof(grp_uint32_t));
												/* copy serial number */
	iRet = FAT_VOL_LAB_LEN;						/* set return value */

out_with_buf:
#ifdef	GRP_FAT_USE_LOCAL_BUF
	if (pucBuf != aucBuf)						/* buffer allocated */
#endif	/* GRP_FAT_USE_LOCAL_BUF */
		grp_mem_free(pucBuf);					/* frre buffer */
out:
	if (ptFs == NULL)
		ptDevOp->pfnClose(iDevHdl, iDev);		/* close device */
	return(iRet);								/* return */
}

#ifdef  GRP_FS_MULTI_LANGUAGE
/* multi language support */
#include "fat_multi_language.c"
#endif  /* GRP_FS_MULTI_LANGUAGE */

#ifdef	GRP_FS_DEBUG
#include "fat_debug.c"
#endif	/* GRP_FS_DEBUG */
