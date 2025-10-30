/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_char_sjis_conv.c										*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Routines for converting Shift JIS characters						*/
/* FUNCTIONS:																*/
/*		grp_char_sjis_cnt			count byte count of SJIS character		*/
/*		grp_char_sjis_to_unicode	convert SJIS to unicode					*/
/*		grp_char_unicode_to_sjis	convert unicode to SJIS					*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_char_conv.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10	Changed variable types for 16 bit CPU	*/
/*		T.Imashiki		2005/07/01	Fixed bug miscalculating user defiled	*/
/*									area code in grp_char_unicode_sjis		*/
/*		T.Imashiki		2007/02/20	Changed type of iIdx parameter of		*/
/*									_StoU_Conv for 16 bit CPU support		*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		M.Suzuki		2010/10/15	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
/*		K.Kaneko		2011/05/23	Added to upper case name function and	*/
/*									file name compare function				*/
/*		K.Kaneko		2011/10/03	Fixed file which does inclusion in		*/
/*									GRP_FS_MINIMIZE_LEVEL use				*/
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
#include "grp_char_conv.h"

#if(GRP_FS_MINIMIZE_LEVEL < 2)
#ifndef	NULL
#define	NULL	((void *)0)
#endif

typedef struct conv_tbl {					/* conversion table */
	const grp_ushort_t		usOrg;			/* original value */
	const grp_ushort_t		usConv;			/* converted value */
} conv_tbl_t;
typedef struct adj_tbl {					/* index adjust table */
	const grp_ushort_t		usStart;		/* start value */
	const grp_ushort_t		usGap;			/* gap count */
	const grp_ushort_t		usAdj;			/* adjust count */
} adj_tbl_t;

#include "grp_char_sjis_tbl.h"				/* conversion table */
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************************************/
/* FUNCTION:	_conv_code													*/
/*																			*/
/* DESCRIPTION:	Convert code with conversion table							*/
/* INPUT:		uiCode:				code to convert							*/
/*				ptConvTbl:			conversion table						*/
/*				pucContCnt:			contiguous count table					*/
/*				iStartLow:			start low								*/
/*				iStartHigh:			start high								*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					bad sequence							*/
/*				positive:			converted code							*/
/*																			*/
/****************************************************************************/
static grp_ushort_t
_conv_code(
	grp_uint32_t	uiCode,					/* code to convert */
	const conv_tbl_t *ptConvTbl,			/* conversion table */
	const grp_uchar_t *pucContCnt,			/* contiguous count table */
	grp_int32_t		iStartLow,				/* start low */
	grp_int32_t		iStartEnd)				/* start end */
{
	grp_int32_t		iLow;					/* low summary index */
	grp_int32_t		iHigh;					/* high summary index */
	grp_int32_t		iIdx;					/* summary index */
	grp_uint32_t	uiEnd;					/* end value */
	const conv_tbl_t *ptConv;				/* summary table pointer */
	
	iLow = iStartLow;						/* low summary index */
	iHigh = iStartEnd - 1;					/* high summary index */
	while (iLow <= iHigh) {					/* do binary search */
		iIdx = (iLow + iHigh) / 2;			/* summary index */
		ptConv = &ptConvTbl[iIdx];			/* conversion */
		uiEnd = ptConv->usOrg + ((pucContCnt == NULL)? 1: pucContCnt[iIdx]);
											/* end value */
		if (ptConv->usOrg > uiCode) {		/* smaller */
			iHigh = iIdx - 1;				/* search smaller summary */
		} else if (uiEnd <= uiCode) {		/* bigger */
			iLow = iIdx + 1;				/* search bigger summary */
		} else {							/* found summary */
			if (ptConv->usConv == 0)		/* bad code */
				return(0);					/* return error */
			return((grp_ushort_t)(ptConv->usConv + uiCode - ptConv->usOrg));
											/* return converted value */
		}
	}
	return(0);								/* return error */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************************************/
/* FUNCTION:	_StoU_Conv													*/
/*																			*/
/* DESCRIPTION:	Convert shift JIS to unicode								*/
/* INPUT:		uiVal:				SJIS value								*/
/*				pusTbl:				conversion table						*/
/*				ptAdjTbl:			index adjust table						*/
/*				iIdx:				index									*/
/* OUTPUT:		puiCode:			unicode									*/
/*																			*/
/* RESULT:		-1:					bad sequence							*/
/*				2:					converted 2 bytes						*/
/*																			*/
/****************************************************************************/
static int
_StoU_Conv(
	grp_uint32_t	uiVal,					/* [IN]  code to convert */
	const grp_ushort_t *pusTbl,				/* [IN]  conversion table */
	const adj_tbl_t	*ptAdjTbl,				/* [IN]  index adjust table */
	grp_int32_t		iIdx,					/* [IN]  index */
	grp_uint32_t	*puiCode)				/* [OUT] converted code */
{
	const adj_tbl_t	*ptAdj;					/* adjust table */
	grp_uint32_t	uiEnd;					/* end value */
	grp_uint32_t	uiConvCode;				/* converted code */

	*puiCode = uiVal;						/* set default valule */
	for (ptAdj = ptAdjTbl; ptAdj->usGap; ptAdj++) {
		if (ptAdj->usStart > uiVal)			/* adjust end */
			break;
		uiEnd = (grp_uint32_t)ptAdj->usStart + ptAdj->usGap;/* end value */
		if (uiVal < uiEnd)					/* in no conv range */
			return(-1);						/* return error */
		iIdx -= ptAdj->usAdj;				/* adjust index */
	}
	uiConvCode = pusTbl[iIdx];				/* converted value */
	if (uiConvCode == 0)					/* bad value */
		return(-1);							/* return error */
	*puiCode = uiConvCode;					/* set converted code */
	return(2);								/* return 2 */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************************************/
/* FUNCTION:	grp_char_sjis_cnt											*/
/*																			*/
/* DESCRIPTION:	Return byte count of next Shift JIS character				*/
/* INPUT:		pucStr:				character string						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:					bad sequence							*/
/*				0 or positive:		byte count of next character			*/
/*																			*/
/****************************************************************************/
int
grp_char_sjis_cnt(
	const grp_uchar_t	*pucStr)			/* [IN]  string */
{
	grp_uint32_t	uiCode;					/* unicode */

	return(grp_char_sjis_to_unicode(pucStr, &uiCode)); /* return byte count */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************************************/
/* FUNCTION:	grp_char_sjis_to_unicode									*/
/*																			*/
/* DESCRIPTION:	Convert next Shift JIS character to unicode					*/
/* INPUT:		pucStr:				character string						*/
/*				puiCode:			unicode data							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:					bad sequence							*/
/*				0 or positive:		byte count of next character			*/
/*																			*/
/****************************************************************************/
int
grp_char_sjis_to_unicode(
	const grp_uchar_t	*pucStr,			/* [IN]  Shift JIS string */
	grp_uint32_t		*puiCode)			/* [OUT] unicode */
{
	grp_int32_t		iCh1;					/* 1st character value */
	grp_int32_t		iCh2;					/* 2nd character value */
	grp_int32_t		iIdx;					/* conv table index */
	grp_uint32_t	uiVal;					/* 2 characters value */
	const grp_ushort_t *pusTbl;				/* SJIS to unicode table */
	const adj_tbl_t	*ptAdj;					/* adjust table */
	
	iCh1 = *pucStr++;						/* get character value */
	if (iCh1 == 0) {						/* end of string */
		*puiCode = 0;						/* 0 */
		return(0);							/* 0 byte */
	} else if (iCh1 < 0x80) {				/* 7 bit ascii */
		*puiCode = iCh1;					/* use it as it is */
		return(1);							/* 1 byte */
	} else if (iCh1 >= 0xa1 && iCh1 < 0xe0) {/* JIS0201 */
		*puiCode = iCh1 + 0xfec0;			/* convert to unicode */
		return(1);							/* 1 byte */
	} else if (iCh1 >= 0x81 && iCh1 < 0xa0) { /* Shift JIS 1st byte (1) */
		iCh2 = *pucStr;						/* get 2nd byte */
		uiVal = (iCh1 << 8) + iCh2;			/* 2 chars value */
		if (uiVal >= SPEC_RANGE_START && uiVal < SPEC_RANGE_END) { 
			*puiCode = _conv_code(uiVal, _atStoU_CvTbl, _aucStoU_Cnt, 0,
								sizeof(_atStoU_CvTbl)/sizeof(conv_tbl_t));
											/* special processing */
			if (*puiCode == 0)				/* bad seq */
				return(-1);					/* return error */
			else							/* success conversion */
				return(2);					/* 2 bytes */
		}
		if (iCh2 >= 0x40 && iCh2 < 0x7f) {	/* Shift JIS 2nd byte (1) */
			iIdx = (iCh1 - 0x81) * (0x7f - 0x40) + iCh2 - 0x40; /* index */
			pusTbl = _ausStoU_Tbl11;		/* conversion table */
			ptAdj = _atStoU_Adj11;			/* adjust table */
		} else if (iCh2 >= 0x80 && iCh2 < 0xfd) { /* Shift JIS 2nd byte (2) */
			iIdx = (iCh1 - 0x81) * (0xfd - 0x80) + iCh2 - 0x80; /* index */
			pusTbl = _ausStoU_Tbl12;		/* conversion table */
			ptAdj = _atStoU_Adj12;			/* adjust table */
		} else 								/* bad seq */
			return(-1);						/* return error */
		return(_StoU_Conv(uiVal, pusTbl, ptAdj, iIdx, puiCode));
											/* convert code */
	} else if (iCh1 >= 0xe0 && iCh1 < 0xeb) {/* Shift JIS 1st byte (2) */
		iCh2 = *pucStr;						/* get 2nd byte */
		uiVal = (iCh1 << 8) + iCh2;			/* 2 chars value */
		if (iCh2 >= 0x40 && iCh2 < 0x7f) {	/* Shift JIS 2nd byte (1) */
			iIdx = (iCh1 - 0xe0) * (0x7f - 0x40) + iCh2 - 0x40; /* index */
			pusTbl = _ausStoU_Tbl21;		/* conversion table */
			ptAdj = _atStoU_Adj21;			/* adjust table */
		} else if (iCh2 >= 0x80 && iCh2 < 0xfd) { /* Shift JIS 2nd byte (2) */
			iIdx = (iCh1 - 0xe0) * (0xfd - 0x80) + iCh2 - 0x80; /* index */
			pusTbl = _ausStoU_Tbl22;		/* conversion table */
			ptAdj = _atStoU_Adj22;		/* adjust table */
		} else 								/* bad seq */
			return(-1);						/* return error */
		return(_StoU_Conv(uiVal, pusTbl, ptAdj, iIdx, puiCode));
											/* convert code */
	} else if (iCh1 >= 0xf0 && iCh1 < 0xfa) {	/* user defined */
		iCh2 = *pucStr;						/* get 2nd byte */
		if ((iCh2 >= 0x40 && iCh2 < 0x7f)
			|| (iCh2 >= 0x80 && iCh2 < 0xfd)) { /* 2nd byte */
			*puiCode = 0xe000 + (iCh1 - 0xf0) * 0xbc
					+ ((iCh2 < 0x80)? iCh2 - 0x40: iCh2 - 0x41); /* conv */
			return(2);						/* 2 bytes */
		}
	}
	return(-1);								/* bad seq */
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

#if(GRP_FS_MINIMIZE_LEVEL < 2)
/****************************************************************************/
/* FUNCTION:	grp_char_unicode_to_sjis									*/
/*																			*/
/* DESCRIPTION:	Convert unicode to Shift JIS character						*/
/* INPUT:		pucStr:				character string						*/
/*				puiCode:			unicode data							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:					bad sequence							*/
/*				0 or positive:		byte count of next character			*/
/*																			*/
/****************************************************************************/
int
grp_char_unicode_to_sjis(
	grp_uchar_t		*pucDst,				/* [OUT] Shift JIS string */
	grp_uint32_t	uiCode)					/* [IN]  unicode */
{
	grp_int32_t		iCh1;					/* 1st byte */
	grp_int32_t		iCh2;					/* 2nd byte */
	grp_int32_t		iLow;					/* low table index */
	int				iIndex;					/* index */
	int				iCnt;					/* range count */

	if (uiCode < 0x80) {					/* 7 bit ascii */
		if (uiCode == 0)					/* null */
			return(0);						/* 0 byte */
		*pucDst++ = (grp_uchar_t)uiCode;	/* use it as it is */
		return(1);							/* 1 byte */
	} else if (uiCode >= 0xe000 && uiCode < 0xe758) {	/* user defined */
		uiCode -= 0xe000;					/* offset from 0xe000 */
		iCh1 = uiCode / 0xbc;				/* page */
		iCh2 = uiCode % 0xbc;				/* page offset */
		*pucDst++ = (grp_uchar_t)(iCh1 + 0xf0);/* convert to 1st byte Shift JIS */
		*pucDst++ = (grp_uchar_t)((iCh2 < 0x3f)? iCh2 + 0x40: iCh2 + 0x41);
											/* conv to 2nd */
		return(2);							/* 2 byte */
	}
	iCh1 = (uiCode >> 8);					/* high byte character */
	iIndex = _aucUtoSIndex[iCh1];			/* UtoS index */
	if (iIndex == 0)						/* bad value */
		return(-1);							/* return error */
	iIndex--;								/* decrement index */
	iLow = _ausUtoS_Low[iIndex];			/* low table index */
	iCnt = _aucUtoS_Range[iIndex];			/* range count */
	if (iLow >= SINGLE_INDEX) {				/* single range index */
		iLow -= SINGLE_INDEX;				/* adjust */
		uiCode = _conv_code(uiCode, _atUtoS_CvTbl1, NULL, iLow, iLow + iCnt);
	} else {								/* non single range index */
		uiCode = _conv_code(uiCode, _atUtoS_CvTbl, _aucUtoS_Cnt,
							iLow, iLow + iCnt);
	}
	if (uiCode == 0)						/* bad seq */
		return(-1);							/* return error */
	*pucDst = (grp_uchar_t)(uiCode >> 8);	/* 1st byte */
	if (*pucDst == 0) {						/* 1 byte */
		*pucDst = (grp_uchar_t)(uiCode & 0xff);/* 2nd byte */
		return(1);							/* 1 byte */
	} else {
		*++pucDst = (grp_uchar_t)(uiCode & 0xff);/* 2nd byte */
		return(2);								/* 2 bytes */
	}
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 2 */

/****************************************************************************/
/* FUNCTION:	grp_char_to_upper											*/
/*																			*/
/* DESCRIPTION:	Convert to upper case name									*/
/* INPUT:		pucOrgName:		original name								*/
/* OUTPUT:		pucUpName:		upper case name								*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_char_to_upper(
	grp_uchar_t			*pucUpName,			/* [OUT]  upper case name */
	const grp_uchar_t	*pucOrgName)		/* [IN]   original name */
{
	int					iCh;				/* character */

	while ((iCh = (int)*pucOrgName++) != 0) { /* until end of name */
		if ('a' <= iCh && iCh <= 'z') {		/* lower case */
			*pucUpName++ = (grp_uchar_t)(iCh - 'a' + 'A');
											/* convert to upper */
			continue;						/* process next char */
		}
		*pucUpName++ = (grp_uchar_t)iCh;	/* copy char */
		if ((iCh >= 0x81 && iCh < 0xa0)		/* SJIS area 1 */
		    || (iCh >= 0xe0 && iCh < 0xeb)	/* SJIS area 2 */
		    || (iCh >= 0xf0 && iCh < 0xfd)) { /* user defined */
			iCh = (int)*pucOrgName++;		/* get 2nd byte */
			if (iCh == 0)					/* NULL */
				break;						/* stop here */
			*pucUpName++ = (grp_uchar_t)iCh; /* copy char */
		}
	}
	*pucUpName = '\0';						/* NULL terminate */
}

/****************************************************************************/
/* FUNCTION:	grp_char_cmp_fname											*/
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
grp_char_cmp_fname(
	const grp_uchar_t	*pucName1,			/* [IN]  file name 1 */
	const grp_uchar_t	*pucName2)			/* [IN]  file name 2 */
{
	int					iCh;				/* character */

	while ((iCh = (int)*pucName1++) != 0) { /* until end of name */
		if ('a' <= iCh && iCh <= 'z') {		/* lower case */
			iCh = iCh - 'a' + 'A';			/* convert to upper */
		}
		if (iCh != (int)*pucName2++)		/* miss match */
			return(-1);						/* return miss match */
		if ((iCh >= 0x81 && iCh < 0xa0)		/* SJIS area 1 */
		    || (iCh >= 0xe0 && iCh < 0xeb)	/* SJIS area 2 */
		    || (iCh >= 0xf0 && iCh < 0xfd)) { /* user defined */
			iCh = (int)*pucName1++;			/* get 2nd byte */
			if (iCh != (int)*pucName2++)	/* miss match */
				return(-1);					/* return miss match */
			if (iCh == 0)					/* NULL */
				return(0);					/* return match */
		}
	}
	if (*pucName2 != '\0')					/* not NULL */
		return(-1);							/* return miss match */
	return(0);								/* return match */
}
