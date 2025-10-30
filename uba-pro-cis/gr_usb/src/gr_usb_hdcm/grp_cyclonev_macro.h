/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2015 Grape Systems, Inc.                        */
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
/*      grp_cyclonev_macro.h                                                    1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV register access macro file                */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Kobayashi    2014/12/24  V0.90                                                           */
/*                            - Created beta release version.                                   */
/*   T.Kobayashi    2015/01/21  V1.00                                                           */
/*                            - 1st release version.                                            */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CYCLONEV_MACRO_H_
#define _GRP_CYCLONEV_MACRO_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_cyclonev_types.h"


/************************************************************************************************/
/*  Register Access Macro                                                                       */
/************************************************************************************************/

/************************************************************************************************/
/* MACRO      : CYCLONEV_R32_RD                                                                 */
/*                                                                                              */
/* DESCRIPTION: Read 32 bit data from register                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulAddr                  Register address                                        */
/* OUTPUT     : none                                                                            */
/* RESULTS    :                         Read value                                              */
/************************************************************************************************/
#define CYCLONEV_R32_RD( ulAddr )        (*((volatile grp_u32*)( ulAddr )))

/************************************************************************************************/
/* MACRO      : CYCLONEV_R32_WR                                                                 */
/*                                                                                              */
/* DESCRIPTION: Write 32 bit data to register                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulAddr                  Register address                                        */
/*              ulVal                   Write value                                             */
/* OUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/************************************************************************************************/
#define CYCLONEV_R32_WR( ulAddr, ulVal ) (*((volatile grp_u32*)( ulAddr )) = (grp_u32)( ulVal ))

/************************************************************************************************/
/* MACRO      : CYCLONEV_R32_ST                                                                 */
/*                                                                                              */
/* DESCRIPTION: Set 32 bit data to register                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulAddr                  Register address                                        */
/*              ulVal                   Set value                                               */
/* OUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/************************************************************************************************/
#define CYCLONEV_R32_ST( ulAddr, ulVal )                                                        \
    (*((volatile grp_u32*)( ulAddr )) = (*((volatile grp_u32*)( ulAddr )) | (grp_u32)( ulVal )))

/************************************************************************************************/
/* MACRO      : CYCLONEV_R32_CR                                                                 */
/*                                                                                              */
/* DESCRIPTION: Clear 32 bit data from register                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulAddr                  Register address                                        */
/*              ulVal                   Clear value                                             */
/* OUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/************************************************************************************************/
#define CYCLONEV_R32_CR( ulAddr, ulVal )                                                        \
    (*((volatile grp_u32*)( ulAddr )) = (*((volatile grp_u32*)( ulAddr )) & (~((grp_u32)( ulVal )))))


#endif /* _GRP_CYCLONEV_MACRO_H_ */
