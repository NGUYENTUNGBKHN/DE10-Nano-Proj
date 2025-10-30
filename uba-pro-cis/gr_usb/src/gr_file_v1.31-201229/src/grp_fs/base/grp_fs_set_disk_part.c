/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_set_disk_part.c										*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Set disk partition information										*/
/* FUNCTIONS:																*/
/*		grp_fs_set_part				set partition information				*/
/* DEPENDENCIES:															*/
/*		grp_fs_disk_part.h													*/
/*		grp_fs_if.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2004/07/25	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10 	Added type casts for 16 bit CPU support	*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2008 Grape Systems, Inc.,  All Rights Reserved.        */
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
#include "grp_fs_disk_part.h"
#include "grp_fs_if.h"

#define CONV_UINT32(auc, uiVal)							/* convert to uint32 */\
{																			   \
	grp_uint32_t	_uiVal = (grp_uint32_t)(uiVal);		/* unsigned int val */ \
	(auc)[0] = (grp_uchar_t)(_uiVal & 0xff);			/* 1st byte */		\
	(auc)[1] = (grp_uchar_t)((_uiVal >> 8) & 0xff);		/* 2nd byte */		\
	(auc)[2] = (grp_uchar_t)((_uiVal >> 16) & 0xff);	/* 3rd byte */		\
	(auc)[3] = (grp_uchar_t)((_uiVal >> 24) & 0xff);	/* 4th byte */		\
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_set_chs												*/
/*																			*/
/* DESCRIPTION:	Set CHS information 										*/
/* INPUT:		ptCHS				decoded CHS information					*/
/* OUTPUT:		pucCHS				encoded CHS information					*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_set_chs(
	grp_uchar_t			*pucCHS,					/* [OUT] encoded CHS */
	grp_fs_dk_chs_t		*ptCHS)						/* [IN]  decodede CHS */
{
	pucCHS[0] = ptCHS->ucHead;						/* head */
	pucCHS[1] = (grp_uchar_t)((ptCHS->ucSec & 0x3f)
				 | ((ptCHS->usCyl & 0x0300) >> 2)); /* sector/track */
													/* high 2bits of cylinder */
	pucCHS[2] = (grp_uchar_t)(ptCHS->usCyl & 0xff);	/* low 8bits of cyliner */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_set_part												*/
/*																			*/
/* DESCRIPTION:	Get partition table											*/
/* INPUT:		ptPartInfo:			partition information					*/
/* OUTPUT:		pucSecData:			1st sector data							*/
/*																			*/
/* RESULT:		0:					success to set							*/
/*				GRP_FS_ERR_BAD_PARAM: bad parameter							*/
/*																			*/
/****************************************************************************/
int grp_fs_set_part(
	grp_uchar_t			*pucSecData,				/* [OUT] sector data */
	grp_fs_dk_part_t	*ptPartInfo)				/* [IN]  partition info */
{
	grp_fs_dk_part_dk_t	*ptDkPart;					/* disk partition info */
	grp_fs_dk_part_tbl_t *ptPartTbl;				/* partition table */
	grp_fs_dk_part_t	*ptPart;					/* partition info */
	int					i;							/* loop count */
	int					iActiveCnt;					/* active count */
	grp_fs_dk_chs_t		*ptStart;					/* decoded start CHS */
	grp_fs_dk_chs_t		*ptEnd;					/* decoded start CHS */

	/********************************************/
	/* check partition information				*/
	/********************************************/
	iActiveCnt = 0;									/* active count */
	ptPart = ptPartInfo;							/* partition info */
	for (i = 0; i < GRP_FS_PART_CNT; i++, ptPart++) {
		if (ptPart->ucActive && iActiveCnt++ > 0)	/* active already exists */
			return(GRP_FS_ERR_BAD_PARAM);			/* invalid parameter */
		if (ptPart->ucPartType == GRP_FS_PART_NULL)	/* NULL partition */
			continue;								/* skip check */
		ptStart = &ptPart->tStartCHS;				/* start CHS */
		ptEnd = &ptPart->tEndCHS;					/* start CHS */
		if (((ptStart->ucSec & 0xc0) || (ptEnd->ucSec & 0xc0))
			|| ((ptStart->usCyl & ~0x3ff) || (ptEnd->usCyl & ~0x3ff))
			|| (ptStart->ucSec | (ptStart->ucHead << 6) 
				| ((grp_uint32_t)ptStart->usCyl << 14))
				> (ptEnd->ucSec | (ptEnd->ucHead << 6) 
					| ((grp_uint32_t)ptEnd->usCyl << 14)))
			return(GRP_FS_ERR_BAD_PARAM);			/* invalid parameter */
	}

	/********************************************/
	/* convert partition information			*/
	/********************************************/
	ptPartTbl = (grp_fs_dk_part_tbl_t *)&pucSecData[GRP_FS_PART_ST_OFF];
													/* partition table */
	ptDkPart = ptPartTbl->atPart;					/* partition info */
	ptPart = ptPartInfo;							/* partition info */
	for (i = 0; i < GRP_FS_PART_CNT; i++) {			/* set all partition info */
		ptDkPart->ucActive = ptPart->ucActive;		/* set active info */
		ptDkPart->ucPartType = ptPart->ucPartType;	/* set active info */
		_grp_fs_set_chs(ptDkPart->aucStartCHS, &ptPart->tStartCHS);
													/* set start CHS */
		_grp_fs_set_chs(ptDkPart->aucEndCHS, &ptPart->tEndCHS);
													/* set end CHS */
		CONV_UINT32(ptDkPart->aucStartSec, ptPart->uiStartSec);
													/* set start sector */
		CONV_UINT32(ptDkPart->aucSecCnt, ptPart->uiSecCnt);
													/* set sector count */
		ptDkPart++;									/* next partition */
		ptPart++;									/* next partition */
	}
	ptPartTbl->aucSign[0] = GRP_FS_PART_SIGN0;		/* set signature 0 */
	ptPartTbl->aucSign[1] = GRP_FS_PART_SIGN1;		/* set signature 1 */
	return(0);										/* return valid */
}
