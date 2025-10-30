/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_dev_sw_tbl.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Device switch table definitions of file system management			*/
/* FUNCTIONS:																*/
/*		Platform dependent I/O routines definition table					*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_fs.h															*/
/*		grp_fs_cfg.h														*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		K.Kaneko		2008/06/03	Create initial version 1.0				*/
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

#include "grp_fs_sysdef.h"
#include "grp_types.h"
#include "grp_fs.h"
#include "grp_fs_cfg.h"

/****************************************************************************/
/*  device switch table														*/
/****************************************************************************/
#ifdef	GRP_FS_PORTING_DEVICE_IO
extern	grp_fs_dev_op_t		grp_fs_dev_op;		/* I/O for mdep */
#endif
#ifdef	GR_USB
extern	grp_fs_dev_op_t		grp_fs_dev_op_grusb;/* I/O for fsif */
#endif
#ifdef	GRP_FS_RAM_DISK
extern	grp_fs_dev_op_t		grp_fs_dev_op_ram;	/* I/O for RAM disk */
#endif

grp_fs_dev_tbl_t	grp_fs_dev_tbl[] = {		/* device switch table */
#ifdef	GRP_FS_PORTING_DEVICE_IO
	{ "dev",	&grp_fs_dev_op },				/* mdep */
#endif
#ifdef	GR_USB
	{ "usb",	&grp_fs_dev_op_grusb }, 		/* fsif */
#endif
#ifdef	GRP_FS_RAM_DISK
	{ "mem",	&grp_fs_dev_op_ram },			/* RAM disk */
#endif
	{ NULL,		NULL }
};

int		grp_fs_dev_tbl_cnt = sizeof(grp_fs_dev_tbl)/sizeof(grp_fs_dev_tbl_t)-1;
