#ifndef	_GRP_FS_PROC_EVENT_H_
#define	_GRP_FS_PROC_EVENT_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_proc_event.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Process insert/eject event											*/
/* FUNCTIONS:																*/
/*		grp_fs_proc_event			process insert/eject event				*/
/* DEPENDENCIES:															*/
/*		None																*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/09/01	Created inital version 1.0				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003 Grape Systems, Inc.,  All Rights Reserved.             */
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
/* paremeter definition														*/
/****************************************************************************/
#define GRP_FS_SAVE_BUF_SIZE	0x10000		/* save buffer size */

/****************************************************************************/
/* event type definition													*/
/****************************************************************************/
#define GRP_FS_EVENT_INSERT		0			/* insert event */
#define GRP_FS_EVENT_EJECT		1			/* eject event */
#define GRP_FS_EVENT_UNMOUNT	2			/* unmount request event */

/****************************************************************************/
/* device mount definition table											*/
/****************************************************************************/
typedef struct grp_fs_mnt_def {				/* mount definition */
	const char			*pcDev;				/* device name */
	const char			*pcMntPoint;		/* mount point */
	int					iMntMode;			/* mount mode */
} grp_fs_mnt_def_t;

/****************************************************************************/
/* exported interface														*/
/****************************************************************************/
int grp_fs_proc_event(						/* process insert/eject event */
		const char			*pcDev,			/* [IN]  devince name */
		int					iEvent);		/* [IN]  event type */

#endif	/* _GRP_FS_PROC_EVENT_H_ */
