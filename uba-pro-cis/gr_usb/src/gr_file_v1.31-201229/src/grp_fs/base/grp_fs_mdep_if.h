#ifndef	_GRP_FS_MDEP_IF_H_
#define	_GRP_FS_MDEP_IF_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_mdep_if.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Platform depedent interface or definitions for file system			*/
/*		management															*/
/* FUNCTIONS:																*/
/*		grp_fs_create_sem			create semaphore						*/
/*		grp_fs_get_sem				get semaphore							*/
/*		grp_fs_release_sem			release semaphore						*/
/*		grp_fs_get_taskid			get task id								*/
/*		grp_fs_copyin				copy data from user						*/
/*		grp_fs_copyout				copy data to user						*/
/*		grp_fs_get_str				get string from user					*/
/*		grp_fs_get_current_time		get current time						*/
/*		grp_fs_char_cnt				byte count of next char					*/
/*		grp_fs_char_to_unicode		convert 1 char to unicode				*/
/*		grp_fs_unicode_to_char		convert unicode to char					*/
/*		grp_fs_printf				print error message						*/
/*		grp_mem_alloc				allocate memory							*/
/*		grp_mem_free				free memory								*/
/* DEPENDENCIES:															*/
/*		<stdarg.h>															*/
/*		grp_types.h															*/
/*		grp_mdep/grp_fs_mdep_types.h										*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10 	Changed return type of grp_fs_copyin	*/
/*									and grp_fs_copyout for 16 bit CPU		*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2011/05/23	Added to upper case name function		*/
/*		K.Kaneko		2011/05/30	Added make short name function of		*/
/*									another method							*/
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

#include "grp_fs_sysdef.h"
#include <stdarg.h>
#include "grp_types.h"
#include "grp_fs_mdep_types.h"

/****************************************************************************/
/*  platform dependent interfaces											*/
/****************************************************************************/
/****************************************************/
/* create semaphore									*/
/****************************************************/
int	grp_fs_create_sem(							/* create semaphore */
		grp_fs_sem_t	*ptSem,					/* [OUT] semaphore */
		const char		*pcName,				/* [IN]  semaphore name */
		int				iInstance,				/* [IN]  instance number */
		int				iInitCnt);				/* [IN]  initial sem count */

#define GRP_FS_SEM_NAME_LEN		8				/* max semaphore name length */

/****************************************************/
/* get semaphore									*/
/****************************************************/
int grp_fs_get_sem(								/* get semaphore */
		grp_fs_sem_t	tSem);					/* [IN]  semaphore */

/****************************************************/
/* release semaphore								*/
/****************************************************/
void grp_fs_release_sem(						/* release semaphore */
		grp_fs_sem_t	tSem);					/* [IN]  semaphore */

/****************************************************/
/* get task id										*/
/****************************************************/
grp_fs_task_t grp_fs_get_taskid(void);			/* get task id */

/****************************************************/
/* copy data from user								*/
/****************************************************/
grp_int32_t grp_fs_copyin(						/* copy data from user */
		void			*pvDst,					/* [OUT] destination */
		void			*pvSrc,					/* [IN]  source */
		grp_isize_t		iSize);					/* [IN]  size to copy */

/****************************************************/
/* copy data to user								*/
/****************************************************/
grp_int32_t grp_fs_copyout(						/* copy data to user */
		void			*pvDst,					/* [OUT] destination */
		void			*pvSrc,					/* [IN]  source */
		grp_isize_t		iSize);					/* [IN]  size to copy */

/****************************************************/
/* copy string from user							*/
/****************************************************/
int grp_fs_get_str(								/* get string from user */
		grp_uchar_t		*pucDst,				/* [OUT] destination */
		const grp_uchar_t *pucSrc,				/* [IN]  source */
		int				iSize);					/* [IN]  size of destination */

/****************************************************/
/* get current time 								*/
/****************************************************/
int grp_fs_get_current_time(					/* get current time */
		grp_int32_t		*piTime);				/* [OUT] current time */

/****************************************************/
/* return byte count of next character 				*/
/****************************************************/
int grp_fs_char_cnt(							/* byte count of next char */
		const grp_uchar_t *pucStr);				/* [IN]  string */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************/
/* convert next charater to unicode					*/
/*	(byte count as return value)					*/
/****************************************************/
int grp_fs_char_to_unicode(						/* convert 1 char to unicode */
		const grp_uchar_t *pucStr,				/* [IN]  string */
		grp_uint32_t	*puiCode);				/* [OUT] unicode */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
		
#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************/
/* convert unicode to character						*/
/*	(byte count as return value)					*/
/****************************************************/
int grp_fs_unicode_to_char(						/* convert unicode to char */
		grp_uchar_t		*pucDst,				/* [OUT] string */
		grp_uint32_t	uiCode);				/* [IN]  unicode */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

/****************************************************/
/* convert to upper case name						*/
/****************************************************/
void grp_fs_to_upper(							/* convert to upper case name */
	grp_uchar_t			*pucUpName,				/* [OUT]  upper case name */
	const grp_uchar_t	*pucOrgName);			/* [IN]   original name */

/****************************************************/
/* compare file name								*/
/****************************************************/
int grp_fs_cmp_fname(							/* compare file name */
	const grp_uchar_t	*pucName1,				/* [IN]  file name 1 */
	const grp_uchar_t	*pucName2);				/* [IN]  file name 2 */

#ifdef GRP_FS_FAST_MAKE_SNAME
/****************************************************/
/* generate short name								*/
/****************************************************/
int grp_fs_make_sname_another_method(			/* generate short name */
	const grp_uchar_t	*pucLName,				/* [IN] long name */
	grp_uchar_t			*pucSName);				/* [OUT] short name */
#endif /* GRP_FS_FAST_MAKE_SNAME */

/****************************************************/
/* print error message								*/
/****************************************************/
int grp_fs_printf(								/* print error message */
		const char		*pcMsg,					/* [IN]  format */
		...);									/* [IN]  variable parameters */

/****************************************************/
/* alloc memory										*/
/****************************************************/
void *grp_mem_alloc(							/* allocate memory */
		grp_isize_t		iSize);					/* [IN]  size to allocate */

/****************************************************/
/* free memory										*/
/****************************************************/
void grp_mem_free(								/* free memory */
		void			*pvMem);				/* [IN]  memory addr to free */

/****************************************************/
/*  inform function for device I/O error			*/
/****************************************************/
typedef int grp_fs_inform_io_err_func_t(
	int				iDev,						/* [IN]  device number */
	grp_uint32_t	uiDevBlk,					/* [IN]  device block number */
	grp_isize_t		iCnt,						/* [IN]  device block I/O cnt */
	int				iMode);						/* [IN]  I/O mode */

/************************************************/
/* iMode										*/
/************************************************/
#define GRP_FS_IO_READ		0x0001				/* read operation */
#define GRP_FS_IO_WRITE		0x0002				/* write operation */
#define GRP_FS_IO_REQ		0x0004				/* check for I/O request */
#define GRP_FS_IO_OP_ERR	0x0010				/* I/O operation error */

extern grp_fs_inform_io_err_func_t *grp_fs_inform_io_err;
												/* inform func for I/O err */

#endif	/* _GRP_FS_MDEP_IF_H_ */
