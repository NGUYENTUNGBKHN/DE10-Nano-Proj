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
/*      grp_cmem.h                                                              1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      This file has described the structure of user use, error declaration, numerical         */
/*      declaration of user use, and function declaration.                                      */
/*      (for flat memory model (boundary support))                                              */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   M.Suzuki       2013/01/31  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CMEM_
#define _GRP_CMEM_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_usbc_types.h"
#include "grp_usbc.h"
#include "grp_sys_mem.h"
#include "grp_usr_mem.h"

/**** INTERNAL DATA DEFINES *********************************************************************/
#if(0)
/* user memory */
#define     GRP_CMEM_MEMAREA_SIZE       5120                    /* USER CUSTOMIZE DEFINITIONS   */
#else
/* for debug */
#define GRP_CMEM_MEMAREA_SIZE   (   GRP_CMEM_BSIZE_TEST             * GRP_CMEM_BNUM_TEST        \
                                +   GRP_CMEM_BSIZE_REQ              * GRP_CMEM_BNUM_REQ         \
                                +   GRP_CMEM_BSIZE_BIG              * GRP_CMEM_BNUM_BIG         \
                                +   GRP_CMEM_BSIZE_USBD             * GRP_CMEM_BNUM_USBD        \
                                +   GRP_CMEM_BSIZE_CNF_STR          * GRP_CMEM_BNUM_CNF_STR     \
                                +   GRP_CMEM_BSIZE_CNF_MSD          * GRP_CMEM_BNUM_CNF_MSD     \
                                +   GRP_CMEM_BSIZE_CNF_DEV          * GRP_CMEM_BNUM_CNF_DEV     \
                                +   GRP_CMEM_BSIZE_CNF_CONF         * GRP_CMEM_BNUM_CNF_CONF    \
                                +   GRP_CMEM_BSIZE_HUBD             * GRP_CMEM_BNUM_HUBD        \
                                +   GRP_CMEM_BSIZE_MSC_COMMAND      * GRP_CMEM_BNUM_MSC_COMMAND \
                                +   GRP_CMEM_BSIZE_MSC_STATUS       * GRP_CMEM_BNUM_MSC_STATUS  \
                                +   GRP_CMEM_BSIZE_FSIF             * GRP_CMEM_BNUM_FSIF        \
                                +   GRP_CMEM_BSIZE_FSIF_MAX_LUN     * GRP_CMEM_BNUM_FSIF_MAX_LUN\
                                +   GRP_CMEM_BSIZE_FSIF_NC_BUF      * GRP_CMEM_BNUM_FSIF_NC_BUF \
                                +   GRP_CMEM_BSIZE_CYCLNVH_DEV_DESC * GRP_CMEM_BNUM_CYCLNVH_DEV_DESC\
                                )
#endif

/* Boundary arrange */
#define     GRP_CMEM_BND_ENABLE         GRP_USB_FALSE

/************************************************************************************************/
/* Stract                                                                                       */
/* Range of memory pool */
typedef struct {
    void        *pvStartAd;                                             /* Start address        */
    grp_u32     ulSize;                                                 /* Size of memory area  */
} grp_cmem_range;

/* Portiong Format */
typedef struct {
    grp_u32     ulID;               /* Memory pool ID                                           */
    grp_u32     ulBlkCnt;           /* Number of blocks in a fixed length memory pool           */
    grp_u32     ulBlkSz;            /* Size of blocks in a fixed length memory pool             */
    grp_u32     ulBnd;              /* Size of boundarys(0:no used, other:boundary size)        */
    grp_u32     ulArea;             /* The identifier on the area which generates a memory pool */
} grp_cmem_pf;

/************************************************************************************************/
/* Error Code                                                                                                           */
#define GRP_CMEM_OK                     0                       /* Normal termination                                   */
#define GRP_CMEM_ERROR_BASE             (grp_s32)-1             /* CMEM error base                                      */
#define GRP_CMEM_ER_VAL                 (GRP_CMEM_ERROR_BASE-1) /* An unjust value exists in a user definition value    */
#define GRP_CMEM_ER_INIT                (GRP_CMEM_ERROR_BASE-2) /* Internal infomation generation went wrong            */
#define GRP_CMEM_ER_ID                  (GRP_CMEM_ERROR_BASE-3) /* The memory pool ID is out of range                   */
#define GRP_CMEM_ER_NOEXS               (GRP_CMEM_ERROR_BASE-4) /* The memory pool is not generated                     */
#define GRP_CMEM_ER_NOBLK               (GRP_CMEM_ERROR_BASE-5) /* There is no empty block                              */
#define GRP_CMEM_ER_BLK                 (GRP_CMEM_ERROR_BASE-6) /* The pointer of memory block is out of range          */
#define GRP_CMEM_ER_OS                  (GRP_CMEM_ERROR_BASE-7) /* OS error                                             */

/************************************************************************************************/
/* Definition value                                                                             */
/* Index value of memory area */
#define     GRP_USB_SYSTEM_AREA         0               /* The memory area which GR_USB uses    */

/************************************************************************************************/
/* Prototype declaration                                                                        */
grp_s32 grp_cmem_PoolInit( void );
grp_s32 grp_cmem_BlkGet( grp_u32, void** );
grp_s32 grp_cmem_BlkRel( void* );
void*   grp_cmem_GetLogical( void* );
void*   grp_cmem_GetPhysical( void* );

#endif  /* _GRP_CMEM_ */
