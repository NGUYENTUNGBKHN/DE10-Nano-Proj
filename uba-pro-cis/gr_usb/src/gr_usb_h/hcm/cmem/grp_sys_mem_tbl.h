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
/*      grp_sys_mem_tbl.h                                                       1.00            */
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
#ifndef _GRP_SYS_MEM_TBL_H_
#define _GRP_SYS_MEM_TBL_H_


/**** INTERNAL DATA DEFINES *********************************************************************/

/* GR_USB use memory pool characteristic */
grp_cmem_pf     l_atCmemPfGu[ GRP_CMEM_ID_GU_MAX ] = {
/*  ID                              Memory Block number             Memory block size               Boundary size       Memory area ID  */
  { GRP_CMEM_ID_USBD,               GRP_CMEM_BNUM_USBD,             GRP_CMEM_BSIZE_USBD,            GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_CNF_STR,            GRP_CMEM_BNUM_CNF_STR,          GRP_CMEM_BSIZE_CNF_STR,         GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_CNF_MSD,            GRP_CMEM_BNUM_CNF_MSD,          GRP_CMEM_BSIZE_CNF_MSD,         GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_CNF_DEV,            GRP_CMEM_BNUM_CNF_DEV,          GRP_CMEM_BSIZE_CNF_DEV,         GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_CNF_CONF,           GRP_CMEM_BNUM_CNF_CONF,         GRP_CMEM_BSIZE_CNF_CONF,        GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_HUBD,               GRP_CMEM_BNUM_HUBD,             GRP_CMEM_BSIZE_HUBD,            GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_MSC_COMMAND,        GRP_CMEM_BNUM_MSC_COMMAND,      GRP_CMEM_BSIZE_MSC_COMMAND,     GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_MSC_STATUS,         GRP_CMEM_BNUM_MSC_STATUS,       GRP_CMEM_BSIZE_MSC_STATUS,      GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_FSIF,               GRP_CMEM_BNUM_FSIF,             GRP_CMEM_BSIZE_FSIF,            GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_FSIF_MAX_LUN,       GRP_CMEM_BNUM_FSIF_MAX_LUN,     GRP_CMEM_BSIZE_FSIF_MAX_LUN,    GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_FSIF_NC_BUF,        GRP_CMEM_BNUM_FSIF_NC_BUF,      GRP_CMEM_BSIZE_FSIF_NC_BUF,     GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_CYCLNVH_DEV_DESC,   GRP_CMEM_BNUM_CYCLNVH_DEV_DESC, GRP_CMEM_BSIZE_CYCLNVH_DEV_DESC,GRP_CMEM_BOUNDARY,  GRP_USB_SYSTEM_AREA },
};

#endif  /* _GRP_SYS_MEM_TBL_H_ */
