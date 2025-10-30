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
/*      grp_cyclonev_cmod.h                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV header file                               */
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
#ifndef _GRP_CYCLONEV_CMOD_H_
#define _GRP_CYCLONEV_CMOD_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_cyclonev_types.h"
#include "grp_cyclonev_cfg.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_bit_val.h"
#include "grp_cyclonev_reg.h"


/**** EXTERNAL DATA DEFINES *********************************************************************/
/* Reset retry count */
#define GRP_CYCLONEV_RETRY_COUNT        (100000)

/* Flush all the transmit FIFOs */
#define GRP_CYCLONEV_ALL_TXFIFO         (0x10)

/* Error code */
#define GRP_COMMON_OK                   GRP_TARGET_OK                                /* Success */
#define GRP_COMMON_ERROR                GRP_TARGET_ERROR                             /* Error   */

/**** EXTERNAL FUNCTION PROTOTYPES **************************************************************/
extern grp_s32 grp_cyclonev_cmod_CoreInit( grp_si iMode );
extern grp_s32 grp_cyclonev_cmod_EnableGlobalIntr( void );
extern grp_s32 grp_cyclonev_cmod_DisableGlobalIntr( void );
extern grp_s32 grp_cyclonev_cmod_ClearGlobalIntr( void );
extern grp_s32 grp_cyclonev_cmod_EnableDeviceIntr( void );
extern grp_s32 grp_cyclonev_cmod_DisableDeviceIntr( void );
extern grp_s32 grp_cyclonev_cmod_EnableHostIntr( void );
extern grp_s32 grp_cyclonev_cmod_DisableHostIntr( void );
extern grp_s32 grp_cyclonev_cmod_FlushTxFifo( grp_si iFnum );
extern grp_s32 grp_cyclonev_cmod_FlushRxFifo( void );
extern grp_s32 grp_cyclonev_cmod_SetMode( grp_si iMode );
extern grp_si  grp_cyclonev_cmod_GetMode( void );


extern grp_s32 grp_cyclonev_cmod_CoreInit2( grp_si iMode );
extern grp_s32 grp_cyclonev_cmod_EnableGlobalIntr2( void );
extern grp_s32 grp_cyclonev_cmod_DisableGlobalIntr2( void );
extern grp_s32 grp_cyclonev_cmod_ClearGlobalIntr2( void );
extern grp_s32 grp_cyclonev_cmod_EnableDeviceIntr2( void );
extern grp_s32 grp_cyclonev_cmod_DisableDeviceIntr2( void );
extern grp_s32 grp_cyclonev_cmod_EnableHostIntr2( void );
extern grp_s32 grp_cyclonev_cmod_DisableHostIntr2( void );
extern grp_s32 grp_cyclonev_cmod_FlushTxFifo2( grp_si iFnum );
extern grp_s32 grp_cyclonev_cmod_FlushRxFifo2( void );
extern grp_s32 grp_cyclonev_cmod_SetMode2( grp_si iMode );
extern grp_si  grp_cyclonev_cmod_GetMode2( void );


#endif /* _GRP_CYCLONEV_CMOD_H_ */
