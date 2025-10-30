/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_get_disk_part.c										*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Disk partition handling routine										*/
/* FUNCTIONS:																*/
/*		grp_fs_get_part				get partition information				*/
/* DEPENDENCIES:															*/
/*		grp_fs_disk_part.h													*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/06/14	Changed partition less check by BootSig	*/
/*									to jump instruction in BPB				*/
/*		T.Imashiki		2004/07/25	changed file name to grp_fs_get_disk	*/
/*									_part.c									*/
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

#define CONV_UINT(auc)								/* convert to int */	\
	(((grp_uint32_t)(auc)[3] << 24)											\
	 | ((grp_uint32_t)(auc)[2] << 16)										\
	 | ((grp_uint32_t)(auc)[1] << 8)										\
	 | auc[0])

/****************************************************************************/
/* FUNCTION:	_grp_fs_get_chs												*/
/*																			*/
/* DESCRIPTION:	Get CHS information 										*/
/* INPUT:		pucCHS				encoded CHS information					*/
/* OUTPUT:		ptCHS				decoded CHS information					*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_fs_get_chs(
	grp_fs_dk_chs_t		*ptCHS,						/* [OUT] decodede CHS */
	grp_uchar_t			*pucCHS)					/* [IN]  encoded CHS */
{
	ptCHS->ucHead = pucCHS[0];						/* head */
	ptCHS->ucSec = (grp_uchar_t)(pucCHS[1] & 0x3f);	/* sector */
	ptCHS->usCyl = (grp_ushort_t)(((pucCHS[1] & 0xc0) << 2) | pucCHS[2]);
													/* cylinder */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_get_part												*/
/*																			*/
/* DESCRIPTION:	Get partition table											*/
/* INPUT:		pucSecData:			1st sector data							*/
/* OUTPUT:		ptPartInfo:			partition information					*/
/*																			*/
/* RESULT:		GRP_FS_PART_VAILD:	valid partition information				*/
/*				GRP_FS_PART_LESS:	partition less							*/
/*				GRP_FS_PART_INVALID: invalid partition information			*/
/*																			*/
/****************************************************************************/
int grp_fs_get_part(
	grp_uchar_t			*pucSecData,				/* [IN]  sector data */
	grp_fs_dk_part_t	*ptPartInfo)				/* [OUT] partition info */
{
	grp_fs_dk_part_dk_t	*ptDkPart;					/* disk partition info */
	grp_fs_dk_part_tbl_t *ptPartTbl;				/* partition table */
	int					i;							/* loop count */

	/********************************************/
	/* check partition signature				*/
	/********************************************/
	ptPartTbl = (grp_fs_dk_part_tbl_t *)&pucSecData[GRP_FS_PART_ST_OFF];
													/* partition table info */
	if (ptPartTbl->aucSign[0] != GRP_FS_PART_SIGN0
		|| ptPartTbl->aucSign[1] != GRP_FS_PART_SIGN1)/* sign missmatch */
		return(GRP_FS_PART_INVALID);				/* return invalid  */

	/********************************************/
	/* check partition less BPB					*/
	/********************************************/
	if (GRP_FS_PART_IS_BPB(pucSecData))			/* 1st sector is BPB */
		return(GRP_FS_PART_LESS);				/* return partition less */

	/********************************************/
	/* convert partition information			*/
	/********************************************/
	ptDkPart = ptPartTbl->atPart;					/* partition info */
	for (i = 0; i < GRP_FS_PART_CNT; i++) {			/* set all partition info */
		ptPartInfo->ucActive = ptDkPart->ucActive;	/* active info */
		ptPartInfo->ucPartType = ptDkPart->ucPartType;/* partition type */
		_grp_fs_get_chs(&ptPartInfo->tStartCHS, ptDkPart->aucStartCHS);
													/* get start CHS */
		_grp_fs_get_chs(&ptPartInfo->tEndCHS, ptDkPart->aucEndCHS);
													/* get end CHS */
		ptPartInfo->uiStartSec =  CONV_UINT(ptDkPart->aucStartSec);
													/* get start sector */
		ptPartInfo->uiSecCnt = CONV_UINT(ptDkPart->aucSecCnt);
													/* get sector count */
		ptDkPart++;									/* next partition */
		ptPartInfo++;								/* next partition */
	}
	return(GRP_FS_PART_VALID);						/* return valid */
}
