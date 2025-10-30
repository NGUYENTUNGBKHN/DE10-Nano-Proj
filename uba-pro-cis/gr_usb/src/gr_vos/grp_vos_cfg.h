/************************************************************************************************/
/*                                                                                              */
/*                           Copyright(C) 2007-2020 Grape Systems, Inc.                         */
/*                                       All Rights Reserved                                    */
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
/*      grp_vos_cfg.h                                                           2.02            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Configuration Parameters                                                                */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*   M.Kitahara     2015/01/31  2.01                                                            */
/*                              Changed the Select Function.                                    */
/*                              Changed the value of GRP_VOS_SYSTEM_MEMORY_SIZE.                */
/*                              Deleted the GRP_VOS_FLG_BASE.                                   */
/*   K.Kaneko       2020/03/24  2.02                                                            */
/*                              Deleted GRVOS_TIME_SLICE definition and GRVOS_TSK_TIME_SLICE    */
/*                              definition.                                                     */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_VOS_CFG_H_
#define _GRP_VOS_CFG_H_

/* VOS Object Count */
#define GRP_VOS_MAX_TSK                 40          /* Task                          */
#define GRP_VOS_MAX_QUE                 25/*15*/          /* Queue                         */
#define GRP_VOS_MAX_SEM                 25/*15*/          /* Semaphore                     */
#define GRP_VOS_MAX_PPL                 15          /* Partition Pool                */
#define GRP_VOS_MAX_MPL                 5           /* Memory Pool                   */
#define GRP_VOS_MAX_FLG                 15          /* Event Flag                    */

/* Timer Parameters */
//#define GRP_VOS_TICK_INTERVAL           10          /* Timer Interval (ms)           */
#define GRP_VOS_TICK_INTERVAL           1          /* Timer Interval (ms)           */

/* VOS System Memory Size (add for ThreadX) */
#define GRP_VOS_SYSTEM_MEMORY_SIZE      0x09500

/* Use or No Use */
#define GRP_VOS_USE                     0x00
#define GRP_VOS_NOUSE                   0x01

/* Select Function (if a function is supported by OS then a define is 'GRP_VOS_USE') */
#define GRP_VOS_DEL_TSK                 GRP_VOS_NOUSE
#define GRP_VOS_DEL_QUE                 GRP_VOS_NOUSE
#define GRP_VOS_DEL_SEM                 GRP_VOS_NOUSE
#define GRP_VOS_DEL_PPL                 GRP_VOS_NOUSE
#define GRP_VOS_DEL_FLG                 GRP_VOS_NOUSE

#define GRP_VOS_CRE_MPL                 GRP_VOS_NOUSE
#define GRP_VOS_DEL_MPL                 GRP_VOS_NOUSE
#define GRP_VOS_TGET_MPL                GRP_VOS_NOUSE
#define GRP_VOS_REL_MPL                 GRP_VOS_NOUSE

#endif  /* _GRP_VOS_CFG_H_ */

