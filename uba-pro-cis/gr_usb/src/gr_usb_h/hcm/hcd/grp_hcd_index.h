/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2019 Grape Systems, Inc.                             */
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
/*      grp_hcd_index.h                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB Host controller index file                                                       */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kaneko       2019/04/26  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_HCD_INDEX_
#define _GRP_HCD_INDEX_

/* CYCLONEV host controller (upper 8bit is 0x00) */
#define GRP_HCDI_CYCLONEVH_IDX_00       0x0000
#define GRP_HCDI_CYCLONEVH_IDX_01       0x0001
#define GRP_HCDI_CYCLONEVH_IDX_02       0x0002
#define GRP_HCDI_CYCLONEVH_IDX_03       0x0003
#define GRP_HCDI_CYCLONEVH_TAG          0x0000

#define GRP_HCDI_CYCLONEVH_MAX          1

#endif /* _GRP_HCD_INDEX_ */
