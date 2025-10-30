#ifndef	_GRP_STDIO_H_
#define	_GRP_STDIO_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_stdio.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Standard I/O library												*/
/* FUNCTIONS:																*/
/*		grp_stdio_fopen				open file								*/
/*		grp_stdio_fclose			close file								*/
/*		grp_stdio_fill				fill data								*/
/*		grp_stdio_getc				get character							*/
/*		grp_stdio_putc				put character							*/
/*		grp_stdio_fgets				get line								*/
/*		grp_stdio_fread				read data								*/
/*		grp_stdio_fwrite			write data								*/
/*		grp_stdio_fflush			flush buffer							*/
/*		grp_stdio_fseek				seek file								*/
/*		grp_stdio_ftell				get current pointer						*/
/*		grp_stdio_fprintf			output data with format					*/
/*		grp_stdio_vfprintf			output data with format					*/
/*		fgetc						get character							*/
/*		fputc						put character							*/
/*		ungetc						unget char								*/
/*		fopen						open file								*/
/*		fclose						close file								*/
/*		fread						read file								*/
/*		fwrite						write file								*/
/*		fflush						flush buffer							*/
/*		fseek						seek file								*/
/*		ftell						get current pointer						*/
/*		rewind						rewind									*/
/*		getc						get character							*/
/*		getchar						get char from stdin						*/
/*		putc						put character							*/
/*		putchar						put char to stdout						*/
/*		fgets						get string								*/
/*		fprintf						output with format						*/
/*		vfprintf					output with format						*/
/*		fileno						return file descriptor					*/
/*		ferror						return error							*/
/*		feof 						EOF										*/
/*		clearerr					clear error								*/
/* DEPENDENCIES:															*/
/*		grp_fs_conv.h				if defined(GRP_FS)						*/
/*		grp_mem.h"					if defined(GRP_FS)						*/
/*		<sys/types.h>				if !defined(GRP_FS)						*/
/*		<sys/stat.h>				if !defined(GRP_FS)						*/
/*		<stdlib.h>					if !defined(GRP_FS)						*/
/*		<fcntl.h>					if !defined(GRP_FS)						*/
/*		<unistd.h>					if !defined(GRP_FS) && !defined(WIN32)	*/
/*		<io.h>						if !defined(GRP_FS) && defined(WIN32)	*/
/*		grp_types.h					if !defined(GRP_FS)						*/
/*		<stdarg.h>															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2016/03/17	Supported compile option				*/
/*									GRP_FS_ENABLE_OVER_2G					*/
/*									Added function fseek4G and ftell4G		*/
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
#ifdef	GRP_FS
#include "grp_fs_conv.h"
#include "grp_mem.h"
#ifdef	WIN32
#undef	WIN32
#endif	/* WIN32 */
#else	/* GRP_FS */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#ifndef	WIN32
#include <unistd.h>
#else	/* WIN32 */
#include <io.h>
#endif	/* WIN32 */
#include "grp_types.h"
#endif	/* GRP_FS */
#include <stdarg.h>

#if(GRP_FS_MINIMIZE_LEVEL < 1)
#define _STDIO_H_
#define _STDIO_H
#define __STDIO_H

/****************************************************************************/
/* parameters																*/
/****************************************************************************/
#define GRP_STDIO_BUF		1024			/* I/O buffer size */
#define GRP_STDIO_PBUF		512				/* printf I/O buffer */
#ifndef	WIN32
#define GRP_STDIO_CRT_PROT	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) /* create mode */
#else
#define GRP_STDIO_CRT_PROT	(S_IREAD|S_IWRITE) /* create mode */
#endif

/****************************************************************************/
/* standard defines															*/
/****************************************************************************/
#ifndef	NULL
#define NULL			((void *)0)			/* null pointer */
#endif
#define EOF				-1					/* end of file */
#ifdef	WIN32
#define	SEEK_SET		0					/* absolute seek offset */
#define SEEK_CUR		1					/* relative to current offset */
#define SEEK_END		2					/* relative to file end */
#endif

/****************************************************************************/
/* control structure														*/
/****************************************************************************/
typedef struct grp_stdio_ctl {
	int					iFd;				/* file decriptor */
	int					iError;				/* last error value */
	int					iStatus;			/* status */
	grp_int32_t			iInBuf;				/* in buffer size */
	grp_int32_t			iNext;				/* next index */
	grp_int32_t			iBufSize;			/* buffer size */
	grp_uchar_t			*pucBuf;			/* buffer pointer */
} grp_stdio_ctl_t;

/* iStatus */
#define	GRP_STDIO_READ		0x0001			/* read mode */
#define GRP_STDIO_WRITE		0x0002			/* write mode */
#define GRP_STDIO_STDIN		0x0004			/* standard input */
#define GRP_STDIO_STDOUT	0x0008			/* standard output */
#define GRP_STDIO_PLUS		0x0010			/* plus mode */
#define GRP_STDIO_EOF		0x0100			/* EOF */

typedef grp_stdio_ctl_t		FILE;			/* standard type definition */

/****************************************************************************/
/* default I/O function type defition for stdin/out/err						*/
/****************************************************************************/
typedef grp_isize_t grp_stdio_io_func_t(
		FILE				*ptFile,			/* [IN]  file control data */
		grp_uchar_t			*pucBuf,			/* [IN/OUT] I/O buffer */
		grp_isize_t			iSize);				/* [IN]  I/O size */

/****************************************************************************/
/* exported interfaces														*/
/****************************************************************************/
FILE *grp_stdio_fopen(							/* open file */
		const char			*pcPath,			/* [IN]  path name */
		const char			*pcMode);			/* [IN]  open mode */

int	grp_stdio_fclose(							/* close file */
		FILE				*ptFile);			/* [IN]  file control data */

int grp_stdio_fill(								/* fill data */
		FILE				*ptFile);			/* [IN/OUT] file control data */

int grp_stdio_getc(								/* get character */
		FILE				*ptFile);			/* [IN/OUT] file control data */

int grp_stdio_putc(								/* put character */
		int					iChar,				/* [IN]  character to put */
		FILE				*ptFile);			/* [OUT] file control data */

char *grp_stdio_fgets(							/* get line */
		char				*pcStr,				/* [OUT] line buffer */
		int					iSize,				/* [IN]  line buffer size */
		FILE				*ptFile);			/* [IN]  file control data */

grp_isize_t	grp_stdio_fread(					/* read data */
		void				*pvPtr,				/* [OUT] read buffer */
		grp_isize_t			iSize,				/* [IN]  element size */
		grp_isize_t			iCnt,				/* [IN]  element count */
		FILE				*ptFile);			/* [IN]  file control data */

grp_isize_t	grp_stdio_fwrite(					/* write data */
		void				*pvPtr,				/* [IN]  write buffer */
		grp_isize_t			iSize,				/* [IN]  element size */
		grp_isize_t			iCnt,				/* [IN]  element count */
		FILE				*ptFile);			/* [IN]  file control data */

int	grp_stdio_fflush(							/* flush buffer */
		FILE				*ptFile);			/* [IN]  flush buffer */

int grp_stdio_fseek(							/* seek file */
		FILE				*ptFile,			/* [IN]   file control data */
		long				iOffset,			/* [IN]   offset */
		int					iMode);				/* [IN]   seek mode */

#ifdef GRP_FS_ENABLE_OVER_2G
int grp_stdio_fseek4G(							/* seek file */
		FILE				*ptFile,			/* [IN]   file control data */
		unsigned long		uiOffset,			/* [IN]   offset */
		int					iMode);				/* [IN]   seek mode */

int grp_stdio_ftell4G(							/* get current pointer */
		FILE				*ptFile,			/* [IN]   file control data */
		grp_uioffset_t		*puiOffset);		/* [OUT] current pointer */
#endif /* GRP_FS_ENABLE_OVER_2G */

long grp_stdio_ftell(							/* get current pointer */
		FILE				*ptFile);			/* [IN]   file control data */

int grp_stdio_fprintf(							/* output data with format */
		FILE			*ptFile,				/* [IN]  file control data */
		const char		*pcFormat,				/* [IN]  format data */
		...);									/* [IN]  parameter */

int grp_stdio_vfprintf(							/* output data with format */
		FILE			*ptFile,				/* [IN]  file control data */
		const char		*pcFormat,				/* [IN]  format data */
		va_list			vap);					/* [IN]  parameter */

#define	fgetc(ptFile)							/* get character */		\
		((((ptFile)->iStatus & GRP_STDIO_READ) &&						\
		 (ptFile)->iInBuf > (ptFile)->iNext)?							\
			(int)((ptFile)->pucBuf[(ptFile)->iNext++]):					\
			grp_stdio_getc(ptFile))
#define fputc(c, ptFile)						/* put character */ 	\
		((((ptFile)->iStatus & GRP_STDIO_WRITE) &&						\
		 (ptFile)->iNext < (ptFile)->iBufSize)?							\
			(int)((ptFile)->pucBuf[(ptFile)->iNext++] = (c)):			\
			grp_stdio_putc(c, ptFile))
#define ungetc(c, ptFile)						/* unget char */		\
		((((ptFile)->iStatus & GRP_STDIO_READ) == 0)? -1:				\
		 ((ptFile)->iNext == 0 || (ptFile)->pucBuf == NULL || (c) < 0)? -1: \
		 ((ptFile)->pucBuf[--((ptFile)->iNext)] = (c)))

#define	fopen(pcPath, pcMode)					/* open file */			\
	grp_stdio_fopen(pcPath, pcMode)
#define fclose(ptFile)							/* close file */		\
	grp_stdio_fclose(ptFile)
#define fread(pvPtr, iSize, iCnt, ptFile)		/* read file */			\
	grp_stdio_fread(pvPtr, iSize, iCnt, ptFile)
#define fwrite(pvPtr, iSize, iCnt, ptFile)		/* write file */		\
	grp_stdio_fwrite(pvPtr, iSize, iCnt, ptFile)
#define fflush(ptFile)	grp_stdio_fflush(ptFile) /* flush buffer */
#define fseek(ptFile, iOffset, iMode)			/* seek file */			\
	grp_stdio_fseek(ptFile, iOffset, iMode)
#define ftell(ptFile)	grp_stdio_ftell(ptFile)	/* get current pointer */
#ifdef GRP_FS_ENABLE_OVER_2G
#define fseek4G(ptFile, uiOffset, iMode)		/* seek file */			\
	grp_stdio_fseek4G(ptFile, uiOffset, iMode)
#define ftell4G(ptFile,puiOffset)				/* get current pointer */ \
	grp_stdio_ftell4G(ptFile,puiOffset)
#endif /* GRP_FS_ENABLE_OVER_2G */
#define rewind(ptFile)							/* rewind */			\
	grp_stdio_fseek(ptFile, 0, SEEK_SET)
#define getc(ptFile)	fgetc(ptFile)			/* get character */
#define getchar()		fgetc(grp_stdio_stdin)	/* get char from stdin */
#define putc(c, ptFile)	fputc(c, ptFile)		/* put character */
#define putchar(c)		fputc(c, grp_stdio_stdout) /* put char to stdout */
#define fgets(pcStr, iSize, ptFile)				/* get string */		\
	grp_stdio_fgets(pcStr, iSize, ptFile)
#define fprintf			grp_stdio_fprintf		/* output with format */
#define vfprintf(ptFile, pcFormat, vap)			/* output with format */ \
	grp_stdio_vfprintf(ptFile, pcFormat, vap)
#define fileno(ptFile)	((ptFile)->iFd)			/* return file descriptor */
#define ferror(ptFile)	((ptFile)->iError)		/* return error */
#define feof(ptFile)	((ptFile)->iStatus & GRP_STDIO_EOF) /* EOF */
#define clearerr(ptFile) 						/* clear error */		\
	((ptFile)->iStatus &= ~GRP_STDIO_EOF, (ptFile)->iError = 0)

/****************************************************************************/
/* imported interfaces														*/
/****************************************************************************/
int sprintf(char *pcBuf, const char *pcFormat, ...);/* write buf with format */
int snprintf(char *pcBuf, grp_uisize_t uiSize, const char *pcFormat, ...);
												/* write buf with format */
int vsnprintf(char *pcBuf, grp_uisize_t uiSize, const char *pcFormat, 
			va_list vap);
												/* write buf with format */

/****************************************************************************/
/* exported variables														*/
/****************************************************************************/
#define stdin	grp_stdio_stdin					/* standard input */
#define stdout	grp_stdio_stdout				/* standard output */
#define stderr	grp_stdio_stderr				/* standard error */

extern FILE		*grp_stdio_stdin;				/* standard input */
extern FILE		*grp_stdio_stdout;				/* standard output */
extern FILE		*grp_stdio_stderr;				/* standard error */

extern grp_stdio_io_func_t	*grp_stdio_io_stdin; /* I/O function for stdin */
extern grp_stdio_io_func_t	*grp_stdio_io_stdout;/* I/O function for stdout */
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */

#endif	/* _GRP_STDIO_H_ */
