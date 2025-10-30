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
/*      grp_cyclonev_tg.h                                                       1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV target function header file               */
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
#ifndef _GRP_CYCLONEV_TG_H_
#define _GRP_CYCLONEV_TG_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_cyclonev_types.h"
#include "grp_cyclonev_cfg.h"


/**** EXTERNAL DATA DEFINES *********************************************************************/
/* Error code */
#define GRP_TARGET_OK               (0)                                              /* Success */
#define GRP_TARGET_ERROR            (-1)                                             /* Error   */

#define GRP_USBD_BUFFER_ADDRESS_ERROR(iEpNo)    grp_target_BufferAddressError(iEpNo)
#define GRP_USBD_BUFFER_ADDRESS_ERROR2(iEpNo)    grp_target_BufferAddressError2(iEpNo)
//#define GRP_USBD_BUFFER_ADDRESS_ERROR(iEpNo)
//#define GRP_USBD_BUFFER_ADDRESS_ERROR2(iEpNo)


/**** EXTERNAL FUNCTION PROTOTYPES **************************************************************/
extern grp_s32 grp_target_HwInit( grp_si iMode );
extern grp_s32 grp_target_StartIntr( grp_si iMode );
extern grp_s32 grp_target_StopIntr( grp_si iMode );
extern void    grp_target_DelayMS( grp_u32 ulMsec );
extern void    grp_target_OtgIsr(  grp_u32 icciar, void * context  );
extern void    grp_target_BufferAddressError( grp_si iEpNo );
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #if (GRP_CYCLONEV_USE_DCACHE_TYPE != GRP_CYCLONEV_DCACHE_INVALID)
extern grp_s32 grp_target_DataCacheInvalidate( grp_ui uiAddress, grp_ui uiLength );
 #endif
#endif


extern grp_s32 grp_target_HwInit2( grp_si iMode );
extern grp_s32 grp_target_StartIntr2( grp_si iMode );
extern grp_s32 grp_target_StopIntr2( grp_si iMode );
extern void    grp_target_DelayMS2( grp_u32 ulMsec );
extern void    grp_target_OtgIsr2(  grp_u32 icciar, void * context  );
extern void    grp_target_BufferAddressError2( grp_si iEpNo );
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #if (GRP_CYCLONEV_USE_DCACHE_TYPE != GRP_CYCLONEV_DCACHE_INVALID)
extern grp_s32 grp_target_DataCacheInvalidate2( grp_ui uiAddress, grp_ui uiLength );
 #endif
#endif


#endif  /* _GRP_CYCLONEV_TG_H_ */
