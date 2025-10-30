/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2020 Grape Systems, Inc.                            */
/*                                     All Rights Reserved.                                     */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      grp_usbc_reinit.h                                                       1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB re-initialization related functions                                              */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Kato         2020/01/10  V1.00                                                           */
/*                            - Created first version                                           */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_USBC_REINIT_H_
#define _GRP_USBC_REINIT_H_

#include "grp_std_types.h"

#define REINIT_STATUS_OK (0)
#define REINIT_STATUS_NOT_INITIALIZED (-1)
#define REINTT_STATUS_INIT_ERROR (-2)
#define REINIT_STATUS_ILLEGAL_PARAM (-3)
#define REINIT_STATUS_CONNECTED (-4)

EXTERN grp_si _not_initialized;

grp_s32 grp_reinit_HostReInit(void);

/* functions to initialize DLOCAL variables before reinitialization */
extern void _grp_dlocal_init_usbd_c(void); /* grp_usbc.c */
#endif /*_GRP_USBC_REINIT_H_*/
