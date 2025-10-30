/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2018 Grape Systems, Inc.                          */
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
/*      grp_usbc_types.h                                                        1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# common type conversion definition file                                     */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "FILE NAME" and "DESCRIPTION" of the file header.  */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_USBC_TYPES_H_
#define _GRP_USBC_TYPES_H_

#include "grp_std_types.h"

#undef  GRP_USB_TRUE
#undef  GRP_USB_FALSE
#undef  GRP_USB_NULL

#define GRP_USB_TRUE        1
#define GRP_USB_FALSE       0
#define GRP_USB_NULL        0

#define GRP_USBC_OVER_UC    0x100
#define GRP_USBC_OVER_US    0x10000

#define GRP_USBC_MAX_UC     0xff
#define GRP_USBC_MAX_US     0xffff
#define GRP_USBC_MAX_UL     0xffffffff

#define GRP_USBC_OVER_SC    0x80
#define GRP_USBC_OVER_SS    0x8000
#define GRP_USBC_OVER_SL    0x80000000

#endif /* _GRP_USBC_TYPES_H_ */
