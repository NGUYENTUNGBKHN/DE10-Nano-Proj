/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2018 Grape Systems, Inc.                          */
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
/*      grp_usbc_dbg.h                                                          1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# debug module header file                                                   */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  0.01                                                            */
/*                              Created first version x.00  based on the 1.00                   */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_USBC_DBG_H_
#define _GRP_USBC_DBG_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_usbc.h"

/**** INTERNAL DATA DEFINES *********************************************************************/
#ifdef  GRP_USBC_DBG_MDL
    /* if comment this define do not logging in usbd    */
    #define GRP_USBD_DEBUG_LOGGING

    /* Number of stored trace */
    #define GRP_DBG_MAX_TRACE               1024                    /* Trace Buffer number      */

    /* Function */
    EXTERN void GRDBG_DebugTrace( grp_u8, grp_u8, grp_u8, grp_u8 );
    #define GRDBG_TRACE(x,y,m,n)       GRDBG_DebugTrace((grp_u8)x,(grp_u8)y,(grp_u8)m,(grp_u8)n)

#else   /* GRP_USBC_DBG_MDL */
    /* Function */
    #define GRDBG_TRACE(x,y,m,n)

#endif  /* GRP_USBC_DBG_MDL */


/**** Module Code list **************************************************************************/

#define GRP_DBG_HCM_BASE                GRP_HCM_MDL_ID              /* HCD,USBCTR          0x10 */
#define GRP_DBG_USBC_BASE               GRP_USBC_MDL_ID             /* USBD,CNFSFT         0x20 */
#define GRP_DBG_HUBD_BASE               GRP_HUB_MDL_ID              /* HUB                 0x30 */
#define GRP_DBG_HID_BASE                GRP_HID_MDL_ID              /* HID                 0x40 */
#define GRP_DBG_MSC_BASE                GRP_MSC_MDL_ID              /* MSC                 0x50 */
#define GRP_DBG_FSIF_BASE               GRP_FSIF_MDL_ID             /* FSIF                0x60 */
#define GRP_DBG_SICD_BASE               GRP_SICD_MDL_ID             /* SICD,PTP,MTP,Pict   0x70 */
#define GRP_DBG_CDC_BASE                GRP_CDC_MDL_ID              /* CDC                 0x80 */
#define GRP_DBG_PCD_BASE                GRP_PDC_MDL_ID              /* PRINTER             0x90 */
#define GRP_DBG_AUDIO_BASE              GRP_AUDIO_MDL_ID            /* AUDIO               0xA0 */
#define GRP_DBG_VIDEO_BASE              GRP_VIDEO_MDL_ID            /* VIDEO               0xB0 */
#define GRP_DBG_CWL_BASE                GRP_CWL_MDL_ID              /* Certified WUSB      0xC0 */


/**** Sequence Code list ************************************************************************/

#define END_FUNC                        (0xFFFFFFFF)
#define STR_FUNC                        (0x00000000)
#define STED_FUNC                       (0x80000000)
#define SEQ_FUNC                        (0x00000001)


/**** Only for USBM *****************************************************************************/

#define GRP_DBG_USBC_USBD               (GRP_DBG_USBC_BASE+2)
#define GRP_DBG_USBC_CNFSFT             (GRP_DBG_USBC_BASE+3)
#define GRP_DBG_USBC_CMEM               (GRP_DBG_USBC_BASE+4)


#ifdef  GRP_USBD_DEBUG_LOGGING
    #define _TRACE_USBC_USBD_(n,x,y)        GRDBG_TRACE(GRP_DBG_USBC_USBD,n,x,y)
    #define _TRACE_USBC_CNFSFT_(n,x,y)      GRDBG_TRACE(GRP_DBG_USBC_CNFSFT,n,x,y)
    #define _TRACE_USBC_CMEM_(n,x,y)        GRDBG_TRACE(GRP_DBG_USBC_CMEM,n,x,y)

    #define F_END                           END_FUNC

#else   /* GRP_USBD_DEBUG_LOGGING */
    #define _TRACE_USBC_USBD_(n,x,y)
    #define _TRACE_USBC_CNFSFT_(n,x,y)
    #define _TRACE_USBC_CMEM_(n,x,y)

    #define F_END

#endif  /* GRP_USBD_DEBUG_LOGGING */

#endif /* _GRP_USBC_DBG_H_ */
