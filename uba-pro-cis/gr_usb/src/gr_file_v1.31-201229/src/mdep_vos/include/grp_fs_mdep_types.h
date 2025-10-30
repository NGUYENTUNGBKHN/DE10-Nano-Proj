#ifndef	_GRP_FS_MDEP_TYPES_H_
#define	_GRP_FS_MDEP_TYPES_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_mdep_types.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Platform dependent type definitions for file system management		*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		platform dependent include files									*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*		T.Imashiki		2004/03/30	Added ifdef for THREADX					*/
/*		K.Kaneko		2008/05/21	Deleted #ifdef except VOS				*/
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
#include "grp_types.h"

/****************************************************************************/
/*  basic typedefs															*/
/****************************************************************************/
#include "grp_vos.h"

typedef	GRP_VOS_tSemaphore	*grp_fs_sem_t;			/* semaphore ID */
typedef grp_uint32_t		grp_fs_task_t;			/* task ID */

#endif	/* _GRP_FS_MDEP_TYPES_H_ */
