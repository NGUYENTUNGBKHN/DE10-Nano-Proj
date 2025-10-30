#ifndef	_GRP_FS_DEV_IO_RAM_H_
#define	_GRP_FS_DEV_IO_RAM_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_dev_io_ram.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for RAM disk											*/
/* FUNCTIONS:																*/
/*		grp_fs_dev_io_ram_init		init RAM disk							*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/12/14	Created inital version 1.0				*/
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

#include "grp_types.h"

/****************************************************************************/
/* parameters																*/
/****************************************************************************/
#define GRP_FS_DEV_RAM_NDISK	4			/* number of disks */
#define GRP_FS_DEV_RAM_SECSFT	9			/* sector shift */

/****************************************************************************/
/* typedef for ram disk information											*/
/****************************************************************************/
typedef struct grp_fs_dev_ram_info {
	grp_uchar_t			*pucStart;			/* start address */
	grp_uint32_t		uiSecCount;			/* sector count */
} grp_fs_dev_ram_info_t;

/****************************************************************************/
/* exported interface														*/
/****************************************************************************/
int	grp_fs_dev_io_ram_init(					/* init RAM disk */
		int				iDiskNo,			/* RAM disk number */
		grp_uchar_t		*pucStart,			/* start address */
		grp_uint32_t	uiSecCount);		/* sector count */

#endif	/* _GRP_FS_DEV_IO_RAM_H_ */
