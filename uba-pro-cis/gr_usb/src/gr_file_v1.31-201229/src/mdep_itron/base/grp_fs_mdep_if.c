/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_mdep_if.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Platform dependent routines for file system management for itron	*/
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
/*		<stdio.h>															*/
/*		<string.h>															*/
/*		grp_types.h															*/
/*		grp_fs_if.h															*/
/*		grp_fs_mdep_if.h													*/
/*		grp_char_conv.h														*/
/*		grp_sem.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10 	Changed return type of grp_fs_copyin	*/
/*									and grp_fs_copyout for 16 bit CPU		*/
/*		T.Imashiki		2005/11/05	Fixed function comment of grp_fs_char_	*/
/*									to_unicode and grp_fs_unicode_to_char	*/
/*									Fixed return value of grp_fs_get_		*/
/*									current_time							*/
/*		K.Kaneko		2008/05/21	Deleted processing M32R in grp_fs_printf*/
/*									function								*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2011/05/23	Added to upper case name function		*/
/*		K.Kaneko		2011/05/30	Added make short name function of		*/
/*									another method							*/
/*		K.Kaneko		2011/08/11	Fixed return type of grp_fs_cmp_fname	*/
/*									function								*/
/*		K.Kaneko		2020/12/09	Fixed a warning that there is no		*/
/*									prototype declaration when				*/
/*									GRP_FS_MINIMIZE_LEVEL is set to 2.		*/
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

#include <stdio.h>
#include <string.h>
#include "grp_fs_sysdef.h"
#include "grp_types.h"
#include "grp_fs_if.h"
#include "grp_fs_mdep_if.h"
#include "grp_char_conv.h"
#include "grp_sem.h"

/****************************************************************************/
/* FUNCTION:	grp_fs_create_sem											*/
/*																			*/
/* DESCRIPTION:	Create semaphore											*/
/* INPUT:		pcName:				semaphore name							*/
/*				iInstance:			instance number							*/
/* OUTPUT:		ptSem:				created semaphore						*/
/*																			*/
/* RESULT:		GRP_FS_ERR_EXIST:		already exists						*/
/*				0:						success								*/
/*																			*/
/****************************************************************************/
int
grp_fs_create_sem(
	grp_fs_sem_t	*ptSem,					/* [OUT] semaphore */
	const char		*pcName,				/* [IN]  semaphore name */
	int				iInstance,				/* [IN]  instance number */
	int				iInitCnt)				/* [IN]  initial sem count */
{
	/****************************************************/
	/* create semphore 									*/
	/****************************************************/
	return((grp_sem_create(ptSem, "", iInitCnt) == 0)? 0: GRP_FS_ERR_EXIST);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_sem												*/
/*																			*/
/* DESCRIPTION:	Get semaphore												*/
/* INPUT:		tSem:				get semaphore							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				-1:					error									*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_sem(
	grp_fs_sem_t	tSem)					/* [IN]  semaphore */
{
	/****************************************************/
	/* get semphore here								*/
	/****************************************************/
	return((wai_sem(tSem) == 0)? 0: -1);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_release_sem											*/
/*																			*/
/* DESCRIPTION:	Release semaphore											*/
/* INPUT:		tSem:				release semaphore						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_release_sem(
	grp_fs_sem_t	tSem)					/* [IN]  semaphore */
{
	/****************************************************/
	/* release semphore									*/
	/****************************************************/
	sig_sem(tSem);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_taskid											*/
/*																			*/
/* DESCRIPTION:	Get task id													*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		task id														*/
/*																			*/
/****************************************************************************/
grp_fs_task_t
grp_fs_get_taskid(void)
{
	ID	iTaskID;

	/****************************************************/
	/* get task id here									*/
	/****************************************************/
	(void)get_tid(&iTaskID);
	return(iTaskID);
}

/****************************************************************************/
/* FUNCTION:	grp_fs_copyin												*/
/*																			*/
/* DESCRIPTION:	Copy data from user											*/
/* INPUT:		pvSrc:					source data							*/
/*				iSize:					size to copy						*/
/* OUTPUT:		pvDst:					copied data							*/
/*																			*/
/* RESULT:		copied size													*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_copyin(
	void			*pvDst,					/* [OUT] destination */
	void			*pvSrc,					/* [IN]  source */
	grp_isize_t		iSize)					/* [IN]  size to copy */
{
	memcpy(pvDst, pvSrc, iSize);			/* copy data */
	return((grp_int32_t)iSize);				/* return size */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_copyout												*/
/*																			*/
/* DESCRIPTION:	Copy data to user											*/
/* INPUT:		pvSrc:					source data							*/
/*				iSize:					size to copy						*/
/* OUTPUT:		pvDst:					copied data							*/
/*																			*/
/* RESULT:		copied size													*/
/*																			*/
/****************************************************************************/
grp_int32_t
grp_fs_copyout(
	void			*pvDst,					/* [OUT] destination */
	void			*pvSrc,					/* [IN]  source */
	grp_isize_t		iSize)					/* [IN]  size to copy */
{
	memcpy(pvDst, pvSrc, iSize);			/* copy data */
	return((grp_int32_t)iSize);				/* return size */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_str												*/
/*																			*/
/* DESCRIPTION:	Get string from user										*/
/* INPUT:		pucSrc:					source data							*/
/*				iSize:					size of destination area			*/
/* OUTPUT:		pucDst:					copied string						*/
/*																			*/
/* RESULT:		0:						success								*/
/*				GRP_FS_ERR_BAD_PARAM:	bad string parameter				*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_str(
	grp_uchar_t		*pucDst,				/* [OUT] destination */
	const grp_uchar_t *pucSrc,				/* [IN]  source */
	grp_isize_t		iSize)					/* [IN]  size of destination */
{
	if (pucSrc == NULL)						/* null */
		return(GRP_FS_ERR_BAD_PARAM);		/* bad parameter */
	strncpy((char *)pucDst, (char *)pucSrc, iSize - 1);	/* copy data */
	pucDst[iSize - 1] = 0;					/* NULL terminante */
	return(0);								/* return success */
}
	
/****************************************************************************/
/* FUNCTION:	grp_fs_get_current_time										*/
/*																			*/
/* DESCRIPTION:	Get current time											*/
/* INPUT:		None														*/
/* OUTPUT:		piTime:				current time							*/
/*																			*/
/* RESULT:		0:					success									*/
/*				-1:					error									*/
/*																			*/
/****************************************************************************/
int
grp_fs_get_current_time(
	grp_int32_t		*piTime)				/* [OUT] current time */
{
	/****************************************************/
	/* get current time 								*/
	/****************************************************/
	return((grp_time_get(piTime) == -1)? -1: 0); /* get current time */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_char_cnt												*/
/*																			*/
/* DESCRIPTION:	Return byte count of next character							*/
/* INPUT:		pucStr:				character string						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:					bad sequence							*/
/*				0 or positive:		byte count of next character			*/
/*																			*/
/****************************************************************************/
int
grp_fs_char_cnt(
	const grp_uchar_t	*pucStr)			/* [IN]  string */
{
	/****************************************************/
	/* count character bytes code here	(XXX)			*/
	/****************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 2)
	return(grp_char_sjis_cnt(pucStr));		/* return Shift JIS count */
#else  /* GRP_FS_MINIMIZE_LEVEL < 2 */
	grp_int32_t 	iCh1;								/* 1st character value */
	grp_int32_t 	iCh2;								/* 2nd character value */
	grp_uint32_t	uiVal;								/* 2 characters value */
	
	iCh1 = *pucStr++;									/* get character value */
	if (iCh1 == 0)										/* end of string */
		return(0);										/* 0 byte */
	else if (iCh1 < 0x80)								/* 7 bit ascii */
		return(1);										/* 1 byte */
	else if ((iCh1 >= 0xa1) && (iCh1 < 0xe0))			/* JIS0201 */
		return(1);										/* 1 byte */
	else if ((iCh1 >= 0x81) && (iCh1 < 0xa0)) { 		/* Shift JIS 1st byte (1) */
		iCh2 = *pucStr; 								/* get 2nd byte */
		uiVal = (iCh1 << 8) + iCh2; 					/* 2 chars value */
		if ((uiVal >= 0x8240) && (uiVal < 0x8480)) 
			return(2);									/* 2 bytes */
		if ((iCh2 >= 0x40) && (iCh2 < 0x7f))			/* Shift JIS 2nd byte (1) */
			return(2);
		else if ((iCh2 >= 0x80) && (iCh2 < 0xfd))		/* Shift JIS 2nd byte (2) */
			return(2);
		else											/* bad seq */
			return(-1); 								/* return error */
	} else if ((iCh1 >= 0xe0) && (iCh1 < 0xeb)) {		/* Shift JIS 1st byte (2) */
		iCh2 = *pucStr; 								/* get 2nd byte */
		uiVal = (iCh1 << 8) + iCh2; 					/* 2 chars value */
		if ((iCh2 >= 0x40) && (iCh2 < 0x7f))			/* Shift JIS 2nd byte (1) */
			return(2);
		else if ((iCh2 >= 0x80) && (iCh2 < 0xfd))		/* Shift JIS 2nd byte (2) */
			return(2);
		else											/* bad seq */
			return(-1); 								/* return error */
	} else if ((iCh1 >= 0xf0) && (iCh1 < 0xfa)) {		/* user defined */
		iCh2 = *pucStr; 								/* get 2nd byte */
		if (((iCh2 >= 0x40) && (iCh2 < 0x7f)) 
		  || ((iCh2 >= 0x80) && (iCh2 < 0xfd))) 		/* 2nd byte */
			return(2);									/* 2 bytes */
	}
	return(-1); 										/* bad seq */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
}

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************************************/
/* FUNCTION:	grp_fs_char_to_unicode										*/
/*																			*/
/* DESCRIPTION:	Convert next character to unicode							*/
/* INPUT:		pucStr:				character string						*/
/* OUTPUT:		puiCode:			unicode data							*/
/*																			*/
/* RESULT:		-1:					bad sequence							*/
/*				0 or positive:		byte count of next character			*/
/*																			*/
/****************************************************************************/
int
grp_fs_char_to_unicode(
	const grp_uchar_t	*pucStr,			/* [IN]  Shift JIS string */
	grp_uint32_t		*puiCode)			/* [OUT] unicode */
{
	/****************************************************/
	/* convert character to unicode here (XXX)			*/
	/****************************************************/
	return(grp_char_sjis_to_unicode(pucStr, puiCode));/* Shift JIS to unicode */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************************************/
/* FUNCTION:	grp_fs_unicode_to_char										*/
/*																			*/
/* DESCRIPTION:	Convert unicode to Shift JIS character						*/
/* INPUT:		uiCode:				unicode data							*/
/* OUTPUT:		pucDst:				SJIS string								*/
/*																			*/
/* RESULT:		-1:					bad sequence							*/
/*				0 or positive:		byte count of next character			*/
/*																			*/
/****************************************************************************/
int
grp_fs_unicode_to_char(
	grp_uchar_t		*pucDst,				/* [OUT] Shift JIS string */
	grp_uint32_t	uiCode)					/* [IN]  unicode */
{
	/****************************************************/
	/* convert unicode to character here (XXX)			*/
	/****************************************************/
	return(grp_char_unicode_to_sjis(pucDst, uiCode)); /* unicode to Shift JIS */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

/****************************************************************************/
/* FUNCTION:	grp_fs_to_upper												*/
/*																			*/
/* DESCRIPTION:	Convert to upper case name									*/
/* INPUT:		pucOrgName:		original name								*/
/* OUTPUT:		pucUpName:		upper case name								*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_fs_to_upper(
	grp_uchar_t			*pucUpName,			/* [OUT]  upper case name */
	const grp_uchar_t	*pucOrgName)		/* [IN]   original name */
{
	/****************************************************/
	/* convert to upper case name here 					*/
	/****************************************************/
	grp_char_to_upper(pucUpName, pucOrgName); /* to upper case name */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_cmp_fname											*/
/*																			*/
/* DESCRIPTION:	Compare file name with case insenstive character match		*/
/* INPUT:		pucName1:	file name 1									    */
/*				pucName2:	file name 2 (represented by upper case)			*/
/*																			*/
/* RESULT:		0:			match											*/
/*				-1:			miss match										*/
/*																			*/
/****************************************************************************/
int
grp_fs_cmp_fname(
	const grp_uchar_t	*pucName1,			/* [IN]  file name 1 */
	const grp_uchar_t	*pucName2)			/* [IN]  file name 2 */
{
	/****************************************************/
	/* compare file name here 							*/
	/****************************************************/
	return(grp_char_cmp_fname(pucName1, pucName2)); /* compare file name */
}

#ifdef GRP_FS_FAST_MAKE_SNAME
/****************************************************************************/
/* FUNCTION:	grp_fs_make_sname_another_method							*/
/*																			*/
/* DESCRIPTION:	Generate short name from long name							*/
/*				Note:	pucSName is 9bytes buffer.							*/
/* INPUT:		pucSName:	short name										*/
/* OUTPUT:		pucSName:	generate short name								*/
/*																			*/
/* RESULT:		0:			generate OK										*/
/*				-1:			generate NG										*/
/*																			*/
/****************************************************************************/
int
grp_fs_make_sname_another_method(
	const grp_uchar_t	*pucLName,			/* [IN] long name */
	grp_uchar_t			*pucSName)			/* [OUT] short name */
{
	grp_uchar_t			*pucSrc;			/* short name */
	grp_uint32_t		uiTotal = 0;		/* compute total value */
	grp_uint32_t		uiValue;			/* long name to value */
	int					i;					/* loop counter */
	
	/****************************************************/
	/* parameter check									*/
	/****************************************************/
	if (pucSName == NULL) {					/* short name pouinter is null */
		return -1;							/* generate error */
	}
	
	/****************************************************/
	/* compute total value from short name				*/
	/****************************************************/
	pucSrc = (grp_uchar_t *)pucLName;		/* long name top pointer */
	while (*pucSrc) {						/* until */
		uiValue = 0;
		for (i = 0;*pucSrc && (i < sizeof(grp_uint32_t));i++, pucSrc++) {
			uiValue |= ((*pucSrc) << (i * 8));
		}
		uiTotal += uiValue;
	}
	sprintf((char *)pucSName, "%08X", uiTotal);
											/* make hash short name */
	return(0);								/* return OK */
}
#endif /* GRP_FS_FAST_MAKE_SNAME */
/****************************************************************************/
/* FUNCTION:	grp_fs_printf												*/
/*																			*/
/* DESCRIPTION:	Output error message										*/
/* INPUT:		pcMsg:					message format						*/
/*				...						message parameters					*/
/* OUTPUT:		output message												*/
/*																			*/
/* RESULT:		length of message											*/
/*																			*/
/****************************************************************************/
int
grp_fs_printf(
	const char		*pcMsg,					/* [IN]  format */
	...)									/* [IN]  variable parameters */
{
	int				iLen;					/* output length */
	char			buf[512];				/* message buffer */
	va_list			vap;					/* argument list */
	int				i;						/* loop index */
	extern	int		cons_putchar(int);		/* put character */

	/****************************************************/
	/* output message									*/
	/****************************************************/
	va_start(vap, pcMsg);					/* set argument list */
	iLen = vsprintf(buf, pcMsg, vap);		/* make message */
	for (i = 0; i < iLen; i++) {
		if (buf[i] == '\n') {				/* new line */
			
			*****************************************************
			* after confirmation, please delete this message	*
			*													*
			* GR-FILE call cons_putchar function to output		*
			* character.										*
			* cons_putchar function is porting function.		*
			* please implement cons_putchar function.			*
			*****************************************************
			
			cons_putchar((int)'\r');		/* carriage return */
			cons_putchar((int)'\n');		/* new line */
		} else								/* others */
			cons_putchar((int)(buf[i] & 0xff));/* output it */
	}
	va_end(vap);							/* end argument list */
	return(iLen);							/* return length */
}
