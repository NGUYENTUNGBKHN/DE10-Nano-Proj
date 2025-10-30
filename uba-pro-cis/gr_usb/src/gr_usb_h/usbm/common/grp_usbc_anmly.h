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
/*      grp_usbc_anmly.h                                                        1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# anomaly function header file                                               */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created first version based on the 1.00b                        */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2014/03/07  V1.06                                                           */
/*                            - Deleted the unnecessary space of the following definition name  */
/*                              - GRP_ANOM_LS_DEV_ DISCON -> GRP_ANOM_LS_DEV_DISCON             */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_USBC_ANMLY_H_
#define _GRP_USBC_ANMLY_H_

/**** INCLUDE FILES *****************************************************************************/
/* Driver header files */
#include "grp_usbc.h"
#include "grp_usbc_dbg.h"

/**** INTERNAL DATA DEFINES *********************************************************************/

/************************************************************************************************/
/* Error Code                                                                                   */
#define GRP_ANOM_OK                     0
#define GRP_ANOM_ILLEGAL_ERROR          -1

/************************************************************************************************/
/* Anomaly Level Code                                                                           */
#define GRP_ANOM_FATAL_LEVEL            0x80000000  /* Fatal level: need to reboot the system   */
#define GRP_ANOM_ERROR_LEVEL            0x40000000  /* Error lever: need to user's trigger      */
#define GRP_ANOM_WARN_LEVEL             0x20000000  /* Warning level: like a infomation         */
#define GRP_ANOM_INFO_LEVEL             0x10000000  /* Information level: just a comment        */

#define GRP_ANOM_ALL_NOTICE             (GRP_ANOM_FATAL_LEVEL | GRP_ANOM_ERROR_LEVEL            \
                                         | GRP_ANOM_WARN_LEVEL | GRP_ANOM_INFO_LEVEL)
#define GRP_ANOM_WARN_NOTICE            (GRP_ANOM_FATAL_LEVEL | GRP_ANOM_ERROR_LEVEL            \
                                         | GRP_ANOM_WARN_LEVEL)
#define GRP_ANOM_ERROR_NOTICE           (GRP_ANOM_FATAL_LEVEL | GRP_ANOM_ERROR_LEVEL)
#define GRP_ANOM_FATAL_NOTICE           (GRP_ANOM_FATAL_LEVEL)

/************************************************************************************************/
/* Anomaly Code                                                                                 */
/*--- Fatal level ---*/
#define GRP_ANOM_SYSTEM_ERROR           ((grp_u32)(0x01 | GRP_ANOM_FATAL_LEVEL))
#define GRP_ANOM_ILEEGAL_ERROR          ((grp_u32)(0x02 | GRP_ANOM_FATAL_LEVEL))
#define GRP_ANOM_UNRECOVERABLE_ERROR    ((grp_u32)(0x03 | GRP_ANOM_FATAL_LEVEL))
/*--- Error level ---*/
#define GRP_ANOM_OVER_CURRENT           ((grp_u32)(0x01 | GRP_ANOM_ERROR_LEVEL))
/*--- Warning level ---*/
#define GRP_ANOM_TOO_MANY_DEVICE        ((grp_u32)(0x01 | GRP_ANOM_WARN_LEVEL))
#define GRP_ANOM_LS_DEV_CON             ((grp_u32)(0x02 | GRP_ANOM_WARN_LEVEL))
#define GRP_ANOM_LS_DEV_DISCON          ((grp_u32)(0x03 | GRP_ANOM_WARN_LEVEL))
/*--- infomation level ---*/
#define GRP_ANOM_HUB_CON                ((grp_u32)(0x01 | GRP_ANOM_INFO_LEVEL))
#define GRP_ANOM_HUB_DISCON             ((grp_u32)(0x02 | GRP_ANOM_INFO_LEVEL))
#define GRP_ANOM_UNKNOWN_DEV_CON        ((grp_u32)(0x03 | GRP_ANOM_INFO_LEVEL))
#define GRP_ANOM_UNKNOWN_DEV_DISCON     ((grp_u32)(0x04 | GRP_ANOM_INFO_LEVEL))

/************************************************************************************************/
/* Signature                                                                                    */
#define GRP_ANOM_SIGNATURE              "GRUSB ANOMALY MODULE"


/* Anomaly response structure */
typedef struct _grp_usbc_anomaly_arise grp_usbc_anomaly_arise;
struct _grp_usbc_anomaly_arise
{
    grp_u32                 ulModule;                               /* Caller module number     */
    void                    *pFunction;                             /* Caller function pointer  */
    grp_u32                 ulLevel;                                /* Anomaly level            */
    void                    *pParam;                                /* Other parameter          */
    void                    *pRefer;                                /* User pointer             */
};

/* Anomaly request structure */
typedef struct _grp_usbc_anomaly_request grp_usbc_anomaly_request;
struct _grp_usbc_anomaly_request
{
    grp_s32                 (*pfnAnomaly_func)( grp_u32, grp_usbc_anomaly_arise* );
                                                                    /* Callback function        */
    grp_u32                 ulNoticeLevel;                          /* NoticeLevel              */
    void                    *pRefer;                                /* User reference pointer   */
};

/* Anomaly information structure */
typedef struct _grp_usbc_anomaly_info grp_usbc_anomaly_info;
struct _grp_usbc_anomaly_info
{
    grp_u8                      aucSignature[32];
    grp_usbc_anomaly_request    tRequest;
};

/* Nortification structure of the over-current */
typedef struct _grp_usbc_anomaly_ovc
{
    grp_u32                     ulRefer;
    grp_u16                     usHubDevId;
    grp_u8                      ucIndex;
    grp_u8                      ucPortNum;
} grp_usbc_anomaly_ovc;


/* Function prototype */
grp_s32 grp_usbc_AnomalyInit(void);
grp_s32 grp_usbc_AnomalySetCallback( grp_usbc_anomaly_request *ptReq);
grp_s32 grp_usbc_AnomalySetNoticeLevel( grp_u32 ulNoticeLevel);
grp_s32 grp_usbc_AnomalyNotifyCallback( grp_u32 ulLevel, grp_u32 ulModule, grp_u32 ulCode, void* pvParam);


/************************************************************************************************/
/*                                                                                              */
#define GRP_USBC_ANOMALY
/*-----------------------------------------------------------------------------------------------*/
#ifdef GRP_USBC_ANOMALY

#define GRP_ANOM_FATAL_(x,y,z)      grp_usbc_AnomalyNotifyCallback( GRP_ANOM_FATAL_LEVEL, x, y, z)
#define GRP_ANOM_ERROR_(x,y,z)      grp_usbc_AnomalyNotifyCallback( GRP_ANOM_ERROR_LEVEL, x, y, z)
#define GRP_ANOM_WARN_( x,y,z)      grp_usbc_AnomalyNotifyCallback( GRP_ANOM_WARN_LEVEL,  x, y, z)
#define GRP_ANOM_INFO_( x,y,z)      grp_usbc_AnomalyNotifyCallback( GRP_ANOM_INFO_LEVEL,  x, y, z)

#else   /* GRP_USBC_ANOMALY */

#define GRP_ANOM_FATAL_(x,y,z)
#define GRP_ANOM_ERROR_(x,y,z)
#define GRP_ANOM_WARN_( x,y,z)
#define GRP_ANOM_INFO_( x,y,z)

#endif  /* GRP_USBC_ANOMALY */

/*                                                                                              */
/************************************************************************************************/

/**** Only for USBM *****************************************************************************/
/*                                                                                              */
#define GRP_USBM_ANOMALY
/*-----------------------------------------------------------------------------------------------*/
#ifdef GRP_USBM_ANOMALY

#define _FATAL_USBD_(y,z)           GRP_ANOM_FATAL_(GRP_DBG_USBC_USBD,y,z)
#define _ERROR_USBD_(y,z)           GRP_ANOM_ERROR_(GRP_DBG_USBC_USBD,y,z)
#define _WARN_USBD_( y,z)           GRP_ANOM_WARN_( GRP_DBG_USBC_USBD,y,z)
#define _INFO_USBD_( y,z)           GRP_ANOM_INFO_( GRP_DBG_USBC_USBD,y,z)
#define _FATAL_CNFSFT_(y,z)         GRP_ANOM_FATAL_(GRP_DBG_USBC_CNFSFT,y,z)
#define _ERROR_CNFSFT_(y,z)         GRP_ANOM_ERROR_(GRP_DBG_USBC_CNFSFT,y,z)
#define _WARN_CNFSFT_( y,z)         GRP_ANOM_WARN_( GRP_DBG_USBC_CNFSFT,y,z)
#define _INFO_CNFSFT_( y,z)         GRP_ANOM_INFO_( GRP_DBG_USBC_CNFSFT,y,z)

#else   /* GRP_USBM_ANOMALY */

#define _FATAL_USBD_(y,z)
#define _ERROR_USBD_(y,z)
#define _WARN_USBD_( y,z)
#define _INFO_USBD_( y,z)
#define _FATAL_CNFSFT_(y,z)
#define _ERROR_CNFSFT_(y,z)
#define _WARN_CNFSFT_( y,z)
#define _INFO_CNFSFT_( y,z)

#endif  /* GRP_USBM_ANOMALY */
/*                                                                                              */
/************************************************************************************************/

#endif /* _GRP_USBC_ANMLY_H_ */
