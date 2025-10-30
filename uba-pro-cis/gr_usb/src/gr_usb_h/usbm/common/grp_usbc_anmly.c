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
/*      grp_usbc_anmly.c                                                        1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# anomaly function                                                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created first version x.00  based on the 1.00b                  */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "FILE NAME" and "DESCRIPTION" of the file header.  */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_usbc.h"
#include "grp_usbc_anmly.h"
#include "grp_std_tools.h"

/**** INTERNAL DATA DEFINES *********************************************************************/
LOCAL grp_usbc_anomaly_info  l_tAnomalyInfo;


/************************************************************************************************/
/* FUNCTION   : grp_usbc_AnomalyInit                                                            */
/*                                                                                              */
/* DESCRIPTION: GR-USB Anomaly module Initialize function                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_ANOM_OK                     Success                                         */
/*              GRP_ANOM_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbc_AnomalyInit(void)
{
grp_s32                         lStatus = GRP_ANOM_OK;

    /* initialize this module */
    l_tAnomalyInfo.tRequest.pfnAnomaly_func  = GRP_USB_NULL;
    l_tAnomalyInfo.tRequest.pRefer           = GRP_USB_NULL;
    l_tAnomalyInfo.tRequest.ulNoticeLevel    = 0;
    /* this signature indicate to pass the initialize routine */
    grp_std_strncpy( l_tAnomalyInfo.aucSignature, GRP_ANOM_SIGNATURE, sizeof(GRP_ANOM_SIGNATURE));

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbc_AnomalySetCallback                                                     */
/*                                                                                              */
/* DESCRIPTION: GR-USB Anomaly module Set Callback function                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Anomaly request for setting callback            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_ANOM_OK                     Success                                         */
/*              GRP_ANOM_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbc_AnomalySetCallback(grp_usbc_anomaly_request *ptReq)
{
grp_s32                         lStatus;

    lStatus = GRP_ANOM_ILLEGAL_ERROR;

    /* check signature */
    if (grp_std_strncmp( l_tAnomalyInfo.aucSignature, GRP_ANOM_SIGNATURE, sizeof(GRP_ANOM_SIGNATURE)) == 0) {
        if (ptReq != GRP_USB_NULL) {
            if (l_tAnomalyInfo.tRequest.pfnAnomaly_func == GRP_USB_NULL) {
                /* set parameteres */
                l_tAnomalyInfo.tRequest.pfnAnomaly_func = ptReq->pfnAnomaly_func;
                l_tAnomalyInfo.tRequest.ulNoticeLevel   = ptReq->ulNoticeLevel;
                l_tAnomalyInfo.tRequest.pRefer          = ptReq->pRefer;

                lStatus = GRP_ANOM_OK;
            }
        }
    }

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbc_AnomalySetNoticeLevel                                                  */
/*                                                                                              */
/* DESCRIPTION: GR-USB Anomaly module Set Notice Level function                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Anomaly request for setting level               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_ANOM_OK                     Success                                         */
/*              GRP_ANOM_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbc_AnomalySetNoticeLevel( grp_u32 ulNoticeLevel)
{
grp_s32                         lStatus;

    lStatus = GRP_ANOM_ILLEGAL_ERROR;

    /* check signature */
    if (grp_std_strncmp( l_tAnomalyInfo.aucSignature, GRP_ANOM_SIGNATURE, sizeof(GRP_ANOM_SIGNATURE)) == 0) {
        /* set level */
        l_tAnomalyInfo.tRequest.ulNoticeLevel = ulNoticeLevel;

        lStatus = GRP_ANOM_OK;
    }

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbc_AnomalyNotifyCallback                                                  */
/*                                                                                              */
/* DESCRIPTION: GR-USB Anomaly module Initialize function                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulLevel                         Anomaly level for shown the degree              */
/*              ulModule                        Module that sends anomaly ulCode                */
/*              ulCode                          Anomaly ulCode in which kind of anomaly         */
/*              pvParam                         Pointer for extended (reserved)                 */
/*                                                                                              */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_ANOM_OK                     Success                                         */
/*              GRP_ANOM_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbc_AnomalyNotifyCallback( grp_u32 ulLevel, grp_u32 ulModule, grp_u32 ulCode, void* pvParam)
{
grp_usbc_anomaly_arise          tArise;
grp_s32                         lCbStatus;
grp_s32                         lStatus;

    lStatus = GRP_ANOM_ILLEGAL_ERROR;

    /* check signature */
    if (grp_std_strncmp( l_tAnomalyInfo.aucSignature, GRP_ANOM_SIGNATURE, sizeof(GRP_ANOM_SIGNATURE)) == 0) {
        /* null check */
        if (l_tAnomalyInfo.tRequest.pfnAnomaly_func != GRP_USB_NULL) {
            /* level check */
            if (l_tAnomalyInfo.tRequest.ulNoticeLevel & ulLevel) {
                /* set parameters */
                tArise.ulModule  = ulModule;                        /* Caller module number     */
                tArise.pFunction = GRP_USB_NULL;                    /* Caller function pointer  */
                tArise.ulLevel   = ulLevel;                         /* Anomaly level            */
                tArise.pParam    = pvParam;                         /* Other parameter          */
                tArise.pRefer    = l_tAnomalyInfo.tRequest.pRefer;  /* User pointer             */
                /* Notification to the application layer */
                lCbStatus = (*l_tAnomalyInfo.tRequest.pfnAnomaly_func)( ulCode, &tArise);
            }
            lStatus = GRP_ANOM_OK;
        }
    }

    return lStatus;
}
