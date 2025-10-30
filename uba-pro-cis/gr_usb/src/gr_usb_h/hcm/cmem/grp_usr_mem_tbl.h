/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2010 Grape Systems, Inc.                            */
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
/*      grp_usr_mem_tbl.h                                                       1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      This file sets up the memory table which user used.                                     */
/*      (for sample program)                                                                    */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2010/08/02  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_USR_MEM_TBL_H_
#define _GRP_USR_MEM_TBL_H_

/**** INTERNAL DATA DEFINES *********************************************************************/

/* User memory pool characteristic */
grp_cmem_pf     l_atCmemPf[ GRP_CMEM_ID_MAX ] = {
/*  ID                      Memory Block number         Memory block size           Boundary size               Memory area ID  */
  { GRP_CMEM_ID_TEST,       GRP_CMEM_BNUM_TEST,         GRP_CMEM_BSIZE_TEST,        GRP_CMEM_BND_TEST,          GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_REQ,        GRP_CMEM_BNUM_REQ,          GRP_CMEM_BSIZE_REQ,         GRP_CMEM_BND_REQ,           GRP_USB_SYSTEM_AREA },
  { GRP_CMEM_ID_BIG,        GRP_CMEM_BNUM_BIG,          GRP_CMEM_BSIZE_BIG,         GRP_CMEM_BND_BIG,           GRP_USB_SYSTEM_AREA }
};

#endif  /* _GRP_USR_MEM_TBL_H_ */
