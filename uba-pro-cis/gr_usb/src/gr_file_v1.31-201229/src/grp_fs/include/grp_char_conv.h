#ifndef	_GRP_CHAR_CONV_H_
#define	_GRP_CHAR_CONV_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_char_conv.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for character conversion routines						*/
/* FUNCTIONS:																*/
/*		grp_char_sjis_cnt			byte count of SJIS char					*/
/*		grp_char_sjis_to_unicode	convert SJIS to unicode					*/
/*		grp_char_unicode_to_sjis	convert unicode to SJIS					*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2011/05/23	Added to upper case name function and	*/
/*									file name compare function				*/
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
#include "grp_types.h"

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************/
/* return byte count of next Shift JIS  character	*/
/****************************************************/
int grp_char_sjis_cnt(							/* byte count of SJIS char */
		const grp_uchar_t	*pucStr);			/* [IN]  SJIS string */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************/
/* convert next Shift JIS charater to unicode		*/
/*	(byte count as return value)					*/
/****************************************************/
int grp_char_sjis_to_unicode(					/* convert 1 SJIS to unicode */
		const grp_uchar_t	*pucStr,			/* [IN]  SJIS string */
		grp_uint32_t		*puiCode);			/* [OUT] unicode */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */
		
#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************/
/* convert unicode to Shift JIS character			*/
/*	(byte count as return value)					*/
/****************************************************/
int grp_char_unicode_to_sjis(					/* convert unicode to SJIS */
		grp_uchar_t			*pucDst,			/* [OUT] SJIS string */
		grp_uint32_t		uiCode);			/* [IN]  unicode */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

/****************************************************/
/* convert to upper case name						*/
/****************************************************/
void grp_char_to_upper(							/* convert to upper case name */
	grp_uchar_t			*pucUpName,				/* [OUT]  upper case name */
	const grp_uchar_t	*pucOrgName);			/* [IN]   original name */

/****************************************************/
/* compare file name								*/
/****************************************************/
int grp_char_cmp_fname(							/* compare file name */
	const grp_uchar_t	*pucName1,				/* [IN]  file name 1 */
	const grp_uchar_t	*pucName2);				/* [IN]  file name 2 */

#endif	/* _GRP_CHAR_CONV_H_ */
