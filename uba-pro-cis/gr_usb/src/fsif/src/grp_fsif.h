/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2008 Grape Systems, Inc.                          */
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
/*      grp_fsif.h                                                              1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      This file composes the File System Interface between a file system                      */
/*      and GR-USB/HOST# Mass storage class modules.                                            */
/*                                                                                              */
/* FUNCTIONS:                                                                                   */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2006/12/28  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Correction by member change in structure.                       */
/*                              - grp_fsif_init_prm                                             */
/*                                  pfnEventNotification -> pfnFsifNotification                 */
/*                            - Modified value type.                                            */
/*                              - grp_s32 -> grp_u32                                            */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRUSB_FSIF_H_
#define _GRUSB_FSIF_H_


/**** INCLUDE FILES *****************************************************************************/
#include "grp_std_types.h"
#include "grp_mdl_id.h"

/* Error Code */
#define GRP_FSIF_OK                     (0)                 /* Success                          */

#define _ERROR_BASE_                    GRP_RET_ERROR_BASE(GRP_FSIF_MDL_ID)

#define GRP_FSIF_NG                     (_ERROR_BASE_-1)    /* error                            */
#define GRP_FSIF_VOS_ERROR              (_ERROR_BASE_-2)    /* Virtual Operating System error   */
#define GRP_FSIF_CMEM_ERROR             (_ERROR_BASE_-3)    /* CMEM module error                */
#define GRP_FSIF_ILLEAGAL_ERROR         (_ERROR_BASE_-4)    /* illegal error                    */
#define GRP_FSIF_TMOUT                  (_ERROR_BASE_-5)    /* Time out was occured             */
#define GRP_FSIF_PARAM_ERROR            (_ERROR_BASE_-6)    /* parameter error                  */
#define GRP_FSIF_QUEINFO_ERROR          (_ERROR_BASE_-7)    /* queue information was error      */
#define GRP_FSIF_ILLEAGAL_STATE         (_ERROR_BASE_-8)    /* Illegal status                   */
#define GRP_FSIF_BUSY                   (_ERROR_BASE_-9)    /* Busy status                      */
#define GRP_FSIF_CHECK_CONDITION        (_ERROR_BASE_-10)   /* need Status                      */
#define GRP_FSIF_NO_CHECK_CONDITION     (_ERROR_BASE_-11)   /* not need Status                  */

#define GRP_FSIF_ATTACHED_MEDIA         ((grp_u32)0x80000000)   /* attached media               */
#define GRP_FSIF_DETACHED_MEDIA         ((grp_u32)0)            /* detached media               */

/* Reset mode */
#define GRP_FSIF_MSC_RESET              1
#define GRP_FSIF_RE_ENUMERATION         2


typedef struct grp_fsif_init_prm_tag
{
    void                        (*pfnFsifNotification)( grp_u32, void*, grp_u8);

} grp_fsif_init_prm;


typedef struct grp_fsif_media_info_tag
{
    grp_u32                     ulSectorSize;
    grp_u32                     ulSectorNum;
    grp_u8                      ucPeriDevType;
    grp_u8                      ucLun;
    grp_u16                     usUsbDevId;

} grp_fsif_media_info;

/**** FUNCTION PROTOTYPES ***********************************************************************/
EXTERN grp_s32 grp_fsif_Init( grp_fsif_init_prm*);
EXTERN grp_s32 grp_fsif_WriteSector( void*, grp_u32, grp_u32, grp_u8*);
EXTERN grp_s32 grp_fsif_ReadSector(  void*, grp_u32, grp_u32, grp_u8*);
EXTERN grp_s32 grp_fsif_GetMediaInfo( void*, grp_fsif_media_info*);
EXTERN grp_s32 grp_fsif_Reset( void*, grp_u32);
EXTERN grp_s32 grp_fsif_GetNonCacheBuffer( grp_u8**);
EXTERN grp_s32 grp_fsif_RelNonCacheBuffer( grp_u8*);

#endif  /* _GRUSB_FSIF_H_ */
