#ifndef	_GRP_FAT_PARAM_H_
#define	_GRP_FAT_PARAM_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fat_param.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		fat file system management parameters								*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		None																*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		K.Kaneko		2008/05/12	Created inital version 1.0				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2008 Grape Systems, Inc.,  All Rights Reserved.             */
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

/****************************************************************************/
/*  parameters																*/
/****************************************************************************/
#define FAT_BLK_SHIFT	9					/* FAT boot/info block size shift */
#define FAT_MAP_CNT		4					/* cluster map count */
#define FAT_FREE_TBL	16					/* free cache count */
#define FAT_COMP_SZ		256					/* component byte size */
#define FAT_COMP_CHCNT	128					/* component character count */
#define FAT_CNT_BUF_SZ	0x8000				/* buf size for count free FAT */

#endif	/* _GRP_FAT_PARAM_H_ */
