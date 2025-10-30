/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2013 Grape Systems, Inc.                             */
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
/*      grp_sys_mem.h                                                           1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      This file has declared the constant value used by the GR_USB module and the memory      */
/*      management module. It is a user porting file.                                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   M.Suzuki       2013/01/31  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_SYS_MEM_H_
#define _GRP_SYS_MEM_H_


/**** INTERNAL DATA DEFINES *********************************************************************/
#define GRP_CMEM_ID_GU_MAX              0x0C                    /* Max value on memory pool ID  */

/* GR_USB use memory pool ID */
#define GRP_CMEM_ID_USBD                0x80000001
#define GRP_CMEM_ID_CNF_STR             0x80000002
#define GRP_CMEM_ID_CNF_MSD             0x80000003
#define GRP_CMEM_ID_CNF_DEV             0x80000004
#define GRP_CMEM_ID_CNF_CONF            0x80000005
#define GRP_CMEM_ID_HUBD                0x80000006
#define GRP_CMEM_ID_MSC_COMMAND         0x80000007
#define GRP_CMEM_ID_MSC_STATUS          0x80000008
#define GRP_CMEM_ID_FSIF                0x80000009
#define GRP_CMEM_ID_FSIF_MAX_LUN        0x8000000A
#define GRP_CMEM_ID_FSIF_NC_BUF         0x8000000B
#define GRP_CMEM_ID_CYCLNVH_DEV_DESC    0x8000000C

/* GR_USB use memory block size */
#define GRP_CMEM_BSIZE_USBD             16
#define GRP_CMEM_BSIZE_CNF_STR          16
#define GRP_CMEM_BSIZE_CNF_MSD          64
#define GRP_CMEM_BSIZE_CNF_DEV          20
#define GRP_CMEM_BSIZE_CNF_CONF         200
#define GRP_CMEM_BSIZE_HUBD             116
#define GRP_CMEM_BSIZE_MSC_COMMAND      32
#define GRP_CMEM_BSIZE_MSC_STATUS       16 
#define GRP_CMEM_BSIZE_FSIF             36
#define GRP_CMEM_BSIZE_FSIF_MAX_LUN     4  
#define GRP_CMEM_BSIZE_FSIF_NC_BUF      512/*2048*/
#define GRP_CMEM_BSIZE_CYCLNVH_DEV_DESC 20

/* GR_USB use memory block number */
#define GRP_CMEM_BNUM_USBD              4/*50*/
#define GRP_CMEM_BNUM_CNF_STR           1
#define GRP_CMEM_BNUM_CNF_MSD           1
#define GRP_CMEM_BNUM_CNF_DEV           GRP_USB_HOST_DEVICE_MAX
#define GRP_CMEM_BNUM_CNF_CONF          GRP_USB_HOST_DEVICE_MAX
#define GRP_CMEM_BNUM_HUBD              0/*10*/
#define GRP_CMEM_BNUM_MSC_COMMAND       GRP_USB_HOST_DEVICE_MAX/*GRP_USB_HOST_DEVICE_MAX*/
#define GRP_CMEM_BNUM_MSC_STATUS        GRP_USB_HOST_DEVICE_MAX/*GRP_USB_HOST_DEVICE_MAX*/
#define GRP_CMEM_BNUM_FSIF              5/*5*/
#define GRP_CMEM_BNUM_FSIF_MAX_LUN      1/*1*/
#define GRP_CMEM_BNUM_FSIF_NC_BUF       1/*1*/
#define GRP_CMEM_BNUM_CYCLNVH_DEV_DESC  1

#define GRP_CMEM_BNUM_GU_ALL              GRP_CMEM_BNUM_USBD            \
                                        + GRP_CMEM_BNUM_CNF_STR         \
                                        + GRP_CMEM_BNUM_CNF_MSD         \
                                        + GRP_CMEM_BNUM_CNF_DEV         \
                                        + GRP_CMEM_BNUM_CNF_CONF        \
                                        + GRP_CMEM_BNUM_HUBD            \
                                        + GRP_CMEM_BNUM_MSC_COMMAND     \
                                        + GRP_CMEM_BNUM_MSC_STATUS      \
                                        + GRP_CMEM_BNUM_FSIF            \
                                        + GRP_CMEM_BNUM_FSIF_MAX_LUN    \
                                        + GRP_CMEM_BNUM_FSIF_NC_BUF     \
                                        + GRP_CMEM_BNUM_CYCLNVH_DEV_DESC

/* GR_USB use boundary size */
#define GRP_CMEM_BOUNDARY               0

#endif  /* _GRP_SYS_MEM_H_ */
