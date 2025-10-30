/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_conv.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Convert standard file I/O interface to GRP_FS interface				*/
/* FUNCTIONS:																*/
/*		open						open file								*/
/*		creat						create file								*/
/*		close						close file								*/
/*		read						read data								*/
/*		write						write data								*/
/*		lseek						seek									*/
/*		unlink						unlink file								*/
/*		chdir						change directory						*/
/*		mkdir						make directory							*/
/*		rmdir						remove directory						*/
/*		rename						rename file								*/
/*		rename						rename file								*/
/*		stat						get file attribute						*/
/*		chmod 						change file mode						*/
/*		utimes						change access/mod time					*/
/*		sync						write back modification					*/
/*		malloc						allocate memory							*/
/*		free						free memory								*/
/*		grp_fs_chmod				change file protect mode				*/
/*		grp_fs_utimes				change access/mod time					*/
/*		grp_fs_stat					get file attribute						*/
/* DEPENDENCIES:															*/
/*		grp_fs_if.h															*/
/*		grp_mem.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
/*									Added definition lseek4G				*/
/*									Added definition grp_fs_stat_t			*/
/*									Modified definition stat				*/
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
#ifndef	_GRP_FS_CONV_H_
#define	_GRP_FS_CONV_H_

/****************************************************************************/
/* definitions for file system I/O											*/
/****************************************************************************/
#include "grp_fs_sysdef.h"
#include "grp_fs_if.h"
#include "grp_mem.h"

#if(GRP_FS_MINIMIZE_LEVEL < 1)
#define open(pcFile, iFlags, uiProt)			/* open file */			\
	grp_fs_open((const grp_uchar_t *)(pcFile), iFlags, uiProt)
#define creat(pcFile, uiProt)					/* create file */		\
	open(pcFile, O_CREAT|O_TRUNC|O_WRONLY, uiProt)
#define close(iFd)								/* close file */		\
	grp_fs_close(iFd)
#define read(iFd, pcBuf, iSize)					/* read data */			\
	grp_fs_read(iFd, (grp_uchar_t *)(pcBuf), iSize)
#define write(iFd, pcBuf, iSize)				/* write data */		\
	grp_fs_write(iFd, (grp_uchar_t *)(pcBuf), iSize)
#define lseek(iFd, iOff, iMode)					/* seek */				\
	grp_fs_lseek(iFd, iOff, iMode)
#ifdef GRP_FS_ENABLE_OVER_2G
#define lseek4G(iFd, iOff, iMode, puiResultOffset)	/* seek */			\
	grp_fs_lseek4G(iFd, iOff, iMode, puiResultOffset)
#endif /* GRP_FS_ENABLE_OVER_2G */
#define unlink(pcFile)							/* unlink file */		\
	grp_fs_unlink((const grp_uchar_t *)(pcFile))
#define chdir(pcDir)							/* change directory */	\
	grp_fs_chdir((const grp_uchar_t *)(pcDir))
#define mkdir(pcDir, uiProt)					/* make directory */	\
	grp_fs_create((const grp_uchar_t *)(pcDir), GRP_FS_FILE_DIR, uiProt, 0)
#define rmdir(pcDir)							/* remove directory */	\
	grp_fs_unlink((const grp_uchar_t *)(pcDir))
#define rename(pcOld, pcNew)					/* rename file */		\
	grp_fs_rename((const grp_uchar_t *)(pcOld), (const grp_uchar_t *)pcNew)
#define ftruncate(iFd, iOff)					/* truncate file */		\
	grp_fs_truncate(iFd, iOff)
#ifdef GRP_FS_ENABLE_OVER_2G
#define stat4G(pcPath, ptStat)					/* get file attribute */ \
	grp_fs_stat((const grp_uchar_t *)(pcPath), ptStat)
#else  /* GRP_FS_ENABLE_OVER_2G */
#define stat(pcPath, ptStat)					/* get file attribute */ \
	grp_fs_stat((const grp_uchar_t *)(pcPath), ptStat)
#endif /* GRP_FS_ENABLE_OVER_2G */
#define chmod(pcPath, uiProt) 					/* change file mode */	\
	grp_fs_chmod((const grp_uchar_t *)(pcPath), uiProt)
#define utimes(pcPath, ptTimes)					/* change access/mod time */ \
	grp_fs_utimes((const grp_uchar_t *)(pcPath), ptTimes)
#define sync()									/* write back modification */ \
	grp_fs_sync(0)
#define getcwd(pcBuf, iSize)					/* get current directory */	\
	((grp_fs_get_cwd((grp_uchar_t *)pcBuf, iSize, '/') != 0)? NULL: (pcBuf))
#ifdef	GRP_MEM
#define malloc(iSize)							/* allocate memory */	\
	grp_mem_alloc(iSize)
#define free(pvMem)								/* free memory */		\
	grp_mem_free(pvMem)
#endif	/* GRP_MEM */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/* open flag */
#ifdef	O_RDONLY								/* open flags defined */
#undef	O_RDONLY								/* undef O_RDONLY */
#undef	O_WRONLY								/* undef O_WRONLY */
#undef	O_RDWR									/* undef O_RDWR */
#undef	O_NONBLOCK								/* undef O_NONBLOCK */
#undef	O_APPEND								/* undef O_APPEND */
#undef	O_CREAT									/* undef O_CREAT */
#undef	O_TRUNC									/* undef O_TRUNC */
#undef	O_EXCL									/* undef O_EXCL */
#undef	O_SHLOCK								/* undef O_SHLOCK */
#undef	O_EXLOCK								/* undef O_EXLOCK */
#endif
#define O_RDONLY	GRP_FS_O_RDONLY				/* read only */
#define O_WRONLY	GRP_FS_O_WRONLY				/* write only */
#define O_RDWR		GRP_FS_O_RDWR				/* read write */
#define O_APPEND	GRP_FS_O_APPEND				/* append mode */
#define O_CREAT		GRP_FS_O_CREAT				/* create */
#define O_TRUNC		GRP_FS_O_TRUNC				/* truncate */
#define O_EXCL		GRP_FS_O_EXCL				/* exclusively create */

/* open mode */
#ifdef	S_IRUSR									/* file mode defined */
#undef	S_IRWXU									/* undef S_IRWXU */
#undef	S_IRUSR									/* undef S_IRUSR */
#undef	S_IWUSR									/* undef S_IWUSR */
#undef	S_IXUSR									/* undef S_IXUSR */
#undef	S_IRWXG									/* undef S_IRWXG */
#undef	S_IRGRP									/* undef S_IRGRP */
#undef	S_IWGRP									/* undef S_IWGRP */
#undef	S_IXGRP									/* undef S_IXGRP */
#undef	S_IRWXO									/* undef S_IRWXO */
#undef	S_IROTH									/* undef S_IROTH */
#undef	S_IWOTH									/* undef S_IWOTH */
#undef	S_IXOTH									/* undef S_IXOTH */
#undef	S_ISUID									/* undef S_ISUID */
#undef	S_ISGID									/* undef S_ISGID */
#endif
#ifdef	S_IFMT
#undef	S_IFMT									/* undef S_IFMT */
#undef	S_IFIFO									/* undef S_IFIFO */
#undef	S_IFCHR									/* undef S_IFCHR */
#undef	S_IFBLK									/* undef S_IFBLK */
#undef	S_IFREG									/* undef S_IFREG */
#undef	S_IFLNK									/* undef S_IFLNK */
#undef	S_IFSOCK								/* undef S_ISOCK */
#undef	S_ISVTX									/* undef S_ISVTX */
#undef	S_ISDIR									/* undef S_ISDIR */
#undef	S_ISCHR									/* undef S_ISCHR */
#undef	S_ISBLK									/* undef S_ISBLK */
#undef	S_ISREG									/* undef S_ISREG */
#undef	S_ISFIFO								/* undef S_ISFIFO */
#undef	S_ISLNK									/* undef S_ISLNK */
#undef	S_ISSOCK								/* undef S_ISSOCK */
#endif

#define	S_IRWXU		GRP_FS_PROT_RWXU			/* permit all to owner */
#define	S_IRUSR		GRP_FS_PROT_RUSR			/* owner readable */
#define	S_IWUSR		GRP_FS_PROT_WUSR			/* owner writable */
#define	S_IXUSR		GRP_FS_PROT_XUSR			/* owner executable */
#define	S_IRWXG		GRP_FS_PROT_RWXG			/* permit all to group */
#define	S_IRGRP		GRP_FS_PROT_RGRP			/* group readable */
#define	S_IWGRP		GRP_FS_PROT_WGRP			/* group writable */
#define	S_IXGRP		GRP_FS_PROT_XGRP			/* group executable */
#define	S_IRWXO		GRP_FS_PROT_RWXO			/* permit all to others */
#define	S_IROTH		GRP_FS_PROT_ROTH			/* others readable */
#define	S_IWOTH		GRP_FS_PROT_WOTH			/* others writable */
#define	S_IXOTH		GRP_FS_PROT_XOTH			/* others executable */

#define S_IFMT		0170000						/* type of file */
#define S_IFIFO		0010000						/* fifo file */
#define S_IFCHR		0020000						/* character device */
#define S_IFDIR		0040000						/* directory */
#define S_IFBLK		0060000						/* block device */
#define S_IFREG		0100000						/* regular file */
#define S_IFLNK		0120000						/* symbolic link */
#define S_IFSOCK	0140000						/* socket */

#define S_ISFIFO(iMode)	(((iMode) & S_IFMT) == S_IFIFO)	/* is fifo file */
#define S_ISCHR(iMode)	(((iMode) & S_IFMT) == S_IFCHR)	/* is char dev */
#define S_ISDIR(iMode)	(((iMode) & S_IFMT) == S_IFDIR)	/* is directory */
#define S_ISBLK(iMode)	(((iMode) & S_IFMT) == S_IFBLK)	/* is block dev */
#define S_ISREG(iMode)	(((iMode) & S_IFMT) == S_IFREG)	/* is regular file */
#define S_ISLNK(iMode)	(((iMode) & S_IFMT) == S_IFLNK)	/* is link file */
#define S_ISSOCK(iMode)	(((iMode) & S_IFMT) == S_IFSOCK)/* is socket file */

#ifdef GRP_FS_ENABLE_OVER_2G
#define GRP_FS_STAT_COMMON_MEMBER				/* stat common member */	\
	int				st_dev;						/* device number */			\
	grp_uint32_t	st_ino;						/* file number */			\
	grp_uint32_t	st_mode;					/* protection */			\
	int				st_nlink;					/* number of links */		\
	int				st_uid;						/* user id */				\
	int				st_gid;						/* group id */				\
	int				st_rdev;					/* real device */			\
	grp_int32_t		st_atime;					/* access time */			\
	grp_int32_t		st_mtime;					/* modification time */		\
	grp_int32_t		st_ctime;					/* create time */

typedef struct grp_fs_stat {					/* file attribute */
	GRP_FS_STAT_COMMON_MEMBER					/* stat common member */
	grp_uint32_t	st_size;					/* file size */
} grp_fs_stat_t;

#else  /* GRP_FS_ENABLE_OVER_2G */
#endif /* GRP_FS_ENABLE_OVER_2G */

#ifndef	_SYS_STAT_H
#ifdef GRP_FS_ENABLE_OVER_2G
struct stat {									/* file attribute */
	GRP_FS_STAT_COMMON_MEMBER					/* stat common member */
	grp_int32_t		st_size;					/* file size */
};
#else  /* GRP_FS_ENABLE_OVER_2G */
struct stat {									/* file attribute */
	int				st_dev;						/* device number */
	grp_uint32_t	st_ino;						/* file number */
	grp_uint32_t	st_mode;					/* protection */
	int				st_nlink;					/* number of links */
	int				st_uid;						/* user id */
	int				st_gid;						/* group id */
	int				st_rdev;					/* real device */
	grp_int32_t		st_atime;					/* access time */
	grp_int32_t		st_mtime;					/* modification time */
	grp_int32_t		st_ctime;					/* create time */
	grp_int32_t		st_size;					/* file size */
};
#endif /* GRP_FS_ENABLE_OVER_2G */
#endif	/* _SYS_STAT_H */
#ifndef	_SYS_TIME_H
#ifndef	_STRUCT_TIMEVAL
#ifndef	_WINSOCKAPI_
struct timeval {								/* time */
	long			tv_sec;						/* seconds */
	long			tv_usec;					/* microseconds */
};
#endif	/* _WINSOCKAPI_ */
#endif	/* _STRUCT_TIMEVAL */
#endif	/* _SYS_TYME_H */

/* seek mode */
#ifdef	SEEK_SET								/* seek mode defined */
#undef	SEEK_SET								/* undef SEEK_SET */
#undef	SEEK_CUR								/* undef SEEK_CUR */
#undef	SEEK_END								/* undef SEEK_END */
#endif
#define	SEEK_SET	GRP_FS_SEEK_SET				/* set absolute offset */
#define	SEEK_CUR	GRP_FS_SEEK_CUR				/* set relative to current */
#define	SEEK_END	GRP_FS_SEEK_END				/* set relative to end */
#ifdef GRP_FS_ENABLE_OVER_2G
/*  only lseek4G  */
#define	SEEK_MINUS	GRP_FS_SEEK_MINUS			/* move seek position back */
#endif /* GRP_FS_ENABLE_OVER_2G */

/****************************************************************************/
/* exported library 														*/
/****************************************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_chmod(								/* change file protect mode */
		const grp_uchar_t	*pucPath,			/* [IN] file name */
		grp_uint32_t		uiProtect);			/* [IN] file prorection */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
int grp_fs_utimes(								/* change access/mod time */
		const grp_uchar_t	*pucPath,			/* [IN] file name */
		struct timeval		*ptTimes);			/* [IN] access/mod times */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
#ifdef GRP_FS_ENABLE_OVER_2G
int grp_fs_stat(								/* get file attribute */
		const grp_uchar_t	*pucPath,			/* [IN]  file name */
		grp_fs_stat_t		*ptStat);			/* [OUT] file attribute */
int stat(										/* get file attribute */
		const grp_uchar_t	*pucPath,			/* [IN]  file name */
		struct stat			*ptStat);			/* [OUT] file attribute */
#else  /* GRP_FS_ENABLE_OVER_2G */
int grp_fs_stat(								/* get file attribute */
		const grp_uchar_t	*pucPath,			/* [IN]  file name */
		struct stat			*ptStat);			/* [OUT] file attribute */
#endif /* GRP_FS_ENABLE_OVER_2G */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
#endif	/* _GRP_FS_CONV_H_ */
