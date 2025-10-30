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
/*      grp_cnfsft.c                                                            1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# configuring software module                                                */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2006/08/04  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/06  V1.01                                                           */
/*                            - The check processing of the descriptor was corrected.           */
/*                              - _grp_cnfsft_SearchDevice                                      */
/*                              - _grp_cnfsft_NormalSearch                                      */
/*                            - Processing concernign Cast is corrected.                        */
/*                              - grp_cnfsft_Init                                               */
/*                              - _grp_cnfsft_DevMngInit                                        */
/*                            - Correction by member change in structure.                       */
/*                              - grp_usbdi_device_request                                      */
/*                                  pfnCallbackFunc -> pfnDvCbFunc                              */
/*                              - grp_usbdi_st_device_request                                   */
/*                                  pfnCallbackFunc -> pfnStCbFunc                              */
/*                            - Deleted unnecessary parameter.                                  */
/*                              - _grp_cnfsft_GetOsStringDesc                                   */
/*                              - _grp_cnfsft_ChkOsStringDesc                                   */
/*                              - _grp_cnfsft_GetMtpFeatureDesc                                 */
/*                              - _grp_cnfsft_ChkMtpFeatureDesc                                 */
/*                              - _grp_cnfsft_MtpSearch                                         */
/*   K.Takagi       2009/04/02  V1.02                                                           */
/*                            - Correspondence to trouble that detects connection of HUB twice. */
/*                              - _grp_cnfsft_SearchDevice                                      */
/*                              - _grp_cnfsft_NormalSearch                                      */
/*                              - _grp_cnfsft_CheckDevDesc                                      */
/*   K.Takagi       2013/07/02  V1.05                                                           */
/*                            - GRP_USB_USE_MTP is added as conditions for compail warning      */
/*                              removal                                                         */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                            - Added upper limit check of management information counter to    */
/*                              the following functions.                                        */
/*                              - _grp_cnfsft_InvalidSearch                                     */
/*                              - _grp_cnfsft_CheckDevDesc                                      */
/*                              - _grp_cnfsft_CheckRegInfo                                      */
/*                              - _grp_cnfsft_MtpSearch                                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include    "grp_usbc.h"
#include    "grp_cnfsft.h"
#include    "grp_cnfsft_local.h"
#include    "grp_usbc_dbg.h"

/**** INTERNAL DATA DEFINES *********************************************************************/

/* Structure of Internal informations                                                           */
DLOCAL _grp_cnfsft_internal_info    l_tInternalInfo;
/* Structure of the Device informations                                                         */
DLOCAL _grp_cnfsft_dev_info         l_atDevInfo[GRP_USB_HOST_DEVICE_MAX];
/* Structure of Registration informations                                                       */
DLOCAL grp_cnfsft_registration      l_atRegInfo[GRP_USBD_HOST_MAX_REGISTER];


/* Flag that shows inialization */
DLOCAL grp_si   l_iCnfSft_Init = GRP_USB_FALSE;

/************************************************************************************************/
/* MACRO for exclusive control                                                                  */
/*                                                                                              */
#define GRP_CNFSFT_LOCK_OK              GRP_VOS_POS_RESULT

#define GRP_CNFSFT_INFO_LOCK()          (grp_vos_GetSemaphore( l_tInternalInfo.ptCnfInfoSem, GRP_VOS_INFINITE))
#define GRP_CNFSFT_INFO_UNLOCK()        (grp_vos_ReleaseSemaphore( l_tInternalInfo.ptCnfInfoSem))

#define GRP_CNFSFT_ENUM_LOCK()          (grp_vos_GetSemaphore( l_tInternalInfo.ptCnfEnmSem, GRP_VOS_INFINITE))
#define GRP_CNFSFT_ENUM_UNLOCK()        (grp_vos_ReleaseSemaphore( l_tInternalInfo.ptCnfEnmSem))
/*                                                                                              */
/************************************************************************************************/


/************************************************************************************************/
/* MACRO for making 16bit data from 8bit data                                                   */
/*                                                                                              */
#define _GRP_CNFSFT_CRE_UINT16( ucUpData, ucLowData)                                            \
    (grp_u16)((((grp_u16)ucLowData) & (grp_u16)0x00FF) | ((grp_u16)((grp_u16)(ucUpData) << 8) & (grp_u16)0xFF00))
/*                                                                                              */
/************************************************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_s32   _grp_cnfsft_DevMngInit(void);
LOCAL grp_s32   _grp_cnfsft_CmpChgConf( grp_usbdi_st_device_request *ptStDevReq);
LOCAL grp_s32   _grp_cnfsft_DevMngGetBuf( _grp_cnfsft_dev_info **pptDevInfo, grp_u16 usDevID);
LOCAL grp_s32   _grp_cnfsft_RegDataSet( grp_cnfsft_registration *ptInput);
LOCAL void      _grp_cnfsft_Disconnect( grp_u16 usEvent, grp_u16 usDevID);
LOCAL void      _grp_cnfsft_Connect( grp_u16 usDevID);
LOCAL grp_s32   _grp_cnfsft_CmpGetDeviceDesc( grp_usbdi_st_device_request *ptStDevReq);
#ifdef GRP_USB_USE_MTP
LOCAL grp_s32   _grp_cnfsft_CmpDeviceReq( grp_usbdi_device_request *ptDevReq);
#endif /* GRP_USB_USE_MTP */
LOCAL grp_s32   _grp_cnfsft_GetStringDesc( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo, grp_u8 ucIndex);
LOCAL grp_s32   _grp_cnfsft_GetLangidPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo);
LOCAL grp_s32   _grp_cnfsft_GetManufacturePhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo);
LOCAL grp_s32   _grp_cnfsft_GetProductPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo);
LOCAL grp_s32   _grp_cnfsft_GetConfigPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo);
LOCAL grp_s32   _grp_cnfsft_GetAllConfigPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo);
#ifdef GRP_USB_USE_MTP
LOCAL grp_s32   _grp_cnfsft_GetOsStringPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo);
LOCAL grp_s32   _grp_cnfsft_GetMsDescPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo);
LOCAL grp_s32   _grp_cnfsft_ChkDev( _grp_cnfsft_dev_info *ptDevInfo);
#endif /* GRP_USB_USE_MTP */
LOCAL grp_s32   _grp_cnfsft_GetDeviceInformation( grp_u16 usDevID, grp_s32 lReqStat);
LOCAL grp_s32   _grp_cnfsft_GetConfigDesc( grp_u16, _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usCnfDescSize);
LOCAL grp_s32   _grp_cnfsft_SearchDevice( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID);
#ifdef GRP_USB_USE_INVALIDE_SPECIFIED
LOCAL grp_s32   _grp_cnfsft_InvalidSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID);
#endif /* GRP_USB_USE_INVALIDE_SPECIFIED */
LOCAL grp_s32   _grp_cnfsft_NormalSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID);
LOCAL grp_s32   _grp_cnfsft_IadDevSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID);
LOCAL grp_s32   _grp_cnfsft_CheckDevDesc( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID, grp_si iFlag);  /* V1.02  */
LOCAL grp_s32   _grp_cnfsft_CheckDescriptor( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID);
LOCAL grp_s32   _grp_cnfsft_CheckRegInfo( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID, grp_u8 *pucIfDesc, grp_u8 ucNumIf);
LOCAL grp_s32   _grp_cnfsft_SearchNextIForIAD( grp_u8 **ppucDesc, grp_u16 *pusRemainLength);
LOCAL grp_s32   _grp_cnfsft_Unregistered( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID, grp_u16 usEvent);
LOCAL grp_s32   _grp_cnfsft_SetConfiguration( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID);
LOCAL grp_s32   _grp_cnfsft_CmpSetConfiguration( grp_usbdi_st_device_request *ptStDevReq);
LOCAL grp_s32   _grp_cnfsft_CallbackFunc( _grp_cnfsft_dev_info *ptDevInfo);
#ifdef GRP_USB_USE_MTP
LOCAL grp_s32   _grp_cnfsft_GetOsStringDesc( grp_u16 usDevID);
LOCAL grp_s32   _grp_cnfsft_ChkOsStringDesc(void);
LOCAL grp_s32   _grp_cnfsft_GetMtpFeatureDesc( grp_u16 usDevID, grp_u8 ucVendorCode);
LOCAL grp_s32   _grp_cnfsft_ChkMtpFeatureDesc(void);
LOCAL grp_s32   _grp_cnfsft_MtpSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID);
#endif /* GRP_USB_USE_MTP */


/************************************************************************************************/
/* FUNCTION   : grp_cnfsft_Init                                                                 */
/*                                                                                              */
/* DESCRIPTION: Initialize thid modules.                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cnfsft_Init(void)
{
grp_s32                         lStat;

    _TRACE_USBC_CNFSFT_(0x01, 0x00, 0x00);

    if (l_iCnfSft_Init != GRP_USB_FALSE) {
        _TRACE_USBC_CNFSFT_(0x01, 0x01, F_END);
        /* already initialized */
        return GRP_CNFSFT_OK;
    }

    /* set initialized flag */
    l_iCnfSft_Init = GRP_USB_TRUE;

    /* 0 clear */
    grp_std_memset( &l_tInternalInfo, 0, sizeof(_grp_cnfsft_internal_info));
    grp_std_memset( &l_atRegInfo[0],  0, sizeof(l_atRegInfo));

    /* Initialize of Device management area */
    lStat = _grp_cnfsft_DevMngInit();
    if (lStat != GRP_CNFSFT_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* Create semaphore */
    lStat = grp_vos_CreateSemaphore( &l_tInternalInfo.ptCnfInfoSem, (grp_u8 *)"sCnf_I", 1);
    if (lStat != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* Create semaphore */
    lStat = grp_vos_CreateSemaphore( &l_tInternalInfo.ptCnfEnmSem, (grp_u8 *)"sCnf_E", 1);
    if (lStat != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

#ifdef GRP_USB_USE_MTP
    /* Create memory area for the String Descriptor */
    lStat = grp_cmem_BlkGet( GRP_CMEM_ID_CNF_STR, (void **)&l_tInternalInfo.ptString);
    if (lStat != GRP_CMEM_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* Create memory area for the MS Descriptor */
    lStat = grp_cmem_BlkGet( GRP_CMEM_ID_CNF_MSD, (void **)&l_tInternalInfo.pucMsDesc);
    if (lStat != GRP_CMEM_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }
#endif  /* GRP_USB_USE_MTP */

    _TRACE_USBC_CNFSFT_(0x01, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_DevMngInit                                                          */
/*                                                                                              */
/* DESCRIPTION: Initialize of Device management area.                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_DevMngInit(void)
{
grp_si                          i;
grp_si                          j;
grp_s32                         lStat;

    _TRACE_USBC_CNFSFT_(0x02, 0x00, 0x00);

    /* 0 claer */
    grp_std_memset( l_atDevInfo, 0, sizeof(l_atDevInfo));

    for (i=0; i<GRP_USB_HOST_DEVICE_MAX; i++) {
        /* Create memory area for the Device Descriptor */
        lStat = grp_cmem_BlkGet( GRP_CMEM_ID_CNF_DEV, (void **)&l_atDevInfo[i].pucDevDesc);
        if (lStat != GRP_CMEM_OK) {
            /* error */
            _TRACE_USBC_CNFSFT_(0x02, 0x01, F_END);
            return GRP_CNFSFT_ERROR;
        }

        /* Create memory area for the Configuration Descriptor */
        lStat = grp_cmem_BlkGet( GRP_CMEM_ID_CNF_CONF, (void **)&l_atDevInfo[i].pucConfDesc);
        if (lStat != GRP_CMEM_OK) {
            /* error */
            _TRACE_USBC_CNFSFT_(0x02, 0x02, F_END);
            return GRP_CNFSFT_ERROR;
        }

        for (j=0; j<GRP_USBD_HOST_MAX_CLASS; j++) {
            /* set initialized valuse */
            l_atDevInfo[i].atIndex[j].iIndex = GRP_CNFSFT_NO_ASSIGNED;
        }
    }

    _TRACE_USBC_CNFSFT_(0x02, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cnfsft_GetDescFromUsbDevId                                                  */
/*                                                                                              */
/* DESCRIPTION: Get Device and Configuration Descriptor.                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/* OUTPUT     : ppvDevDesc                      Pointer of Device descriptor area               */
/*              ppvConfDesc                     Pointer of Configration descriptor area         */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cnfsft_GetDescFromUsbDevId( grp_u16 usDevID, void **ppvDevDesc, void **ppvConfDesc)
{
_grp_cnfsft_dev_info            *ptDevInfo;
grp_s32                         lStat   = GRP_CNFSFT_ERROR;

    _TRACE_USBC_CNFSFT_(0x03, 0x00, 0x00);

    /* Get the management area */
    _grp_cnfsft_DevMngGetBuf( &ptDevInfo, usDevID);
    if (ptDevInfo != GRP_USB_NULL) {
        _TRACE_USBC_CNFSFT_(0x03, 0x01, 0x00);

        if (ppvDevDesc != GRP_USB_NULL) {
            *ppvDevDesc = (void *)ptDevInfo->pucDevDesc;
        }
        if (ppvConfDesc != GRP_USB_NULL) {
            *ppvConfDesc = (void *)ptDevInfo->pucConfDesc;
        }

        lStat = GRP_CNFSFT_OK;
    }

    _TRACE_USBC_CNFSFT_(0x03, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : grp_cnfsft_ChangeConfiguration                                                  */
/*                                                                                              */
/* DESCRIPTION: Requesting to change configuration at specified number.                         */
/*              But this function is only requested the UNSET_CONFIG.                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*              ucConfigIdx                     Specified configuration descriptor index number */
/*                                              - "0xFF" is special value. if it is used then   */
/*                                               invalid search is stopped and normal search is */
/*                                               start from index 0.                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   UNSET_CONFIG request is Success                 */
/*              GRP_CNFSFT_ERROR                UNSET_CONFIG request is Error (disconnected)    */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cnfsft_ChangeConfiguration( grp_u16 usDevID, grp_u8 ucConfigIdx)
{
grp_usbdi_st_device_request     *ptStDevReq = &l_tInternalInfo.tStDevReq;
_grp_cnfsft_dev_info            *ptDevInfo  = GRP_USB_NULL;
grp_s32                         lStat       = GRP_CNFSFT_OK;

    _TRACE_USBC_CNFSFT_(0x04, 0x00, 0x00);

    /* lock */
    if (GRP_CNFSFT_ENUM_LOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* Get the management area */
    _grp_cnfsft_DevMngGetBuf( &ptDevInfo, usDevID);
    if (!ptDevInfo) {
        _TRACE_USBC_CNFSFT_(0x04, 0x01, F_END);
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
            return GRP_CNFSFT_ERROR;
        }
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* check Configuration index number */
    if (ucConfigIdx == GRP_CNFSFT_SPECIAL_NUMBER) {
        ptDevInfo->ucTmpFlag = GRP_USB_FALSE;
        ucConfigIdx          = 0;
    }

    ptDevInfo->ucCnfIdx = ucConfigIdx;

    /* set parameters */
    ptStDevReq->usUsbDevId      = usDevID;
    ptStDevReq->ucConfiguration = ptDevInfo->ucCnfIdx;
    ptStDevReq->ulSize          = GRP_USBD_CONFIG_DESC_SIZE;
    ptStDevReq->pvDescriptor    = ptDevInfo->pucConfDesc;
    ptStDevReq->pfnStCbFunc     = _grp_cnfsft_CmpChgConf;

    /* Get Device Descriptor */
    lStat = grp_usbd_GetConfigDescriptor( ptStDevReq);
    if (lStat != GRP_USBD_OK) {
        _TRACE_USBC_CNFSFT_(0x04, 0x02, F_END);
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
            return GRP_CNFSFT_ERROR;
        }
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x04, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CmpChgConf                                                          */
/*                                                                                              */
/* DESCRIPTION: Get Device Descriptor request callback function                                 */
/*              It keeps processing this function even in case of the transfer error.           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptStDevReq                      standard device request structure               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_CmpChgConf( grp_usbdi_st_device_request *ptStDevReq)
{
_grp_cnfsft_dev_info            *ptDevInfo  = GRP_USB_NULL;
grp_s32                         lStat       = GRP_CNFSFT_OK;
grp_u16                         usDevID     = ptStDevReq->usUsbDevId;
grp_u8                          ucCnfIdx;
grp_u8                          ucTmpFlag;
grp_u16                         usLangId;

    _TRACE_USBC_CNFSFT_(0x05, 0x00, 0x00);

    /* Get the management area */
    _grp_cnfsft_DevMngGetBuf( &ptDevInfo, usDevID);
    if (!ptDevInfo) {
        _TRACE_USBC_CNFSFT_(0x05, 0x01, F_END);
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
            return GRP_CNFSFT_ERROR;
        }
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* save values */
    ucCnfIdx    = ptDevInfo->ucCnfIdx;
    ucTmpFlag   = ptDevInfo->ucTmpFlag;
    usLangId    = ptDevInfo->usLangId;

    /* Disconnected process */
    _grp_cnfsft_Disconnect( GRP_CNFSFT_DEVICE_DETACHED, usDevID);

    /* Set parameters to the management area */
    ptDevInfo->ucEnumStat   = GRP_CNFSFT_GET_ALLCONFIG_PHASE;
    ptDevInfo->ucConnStat   = GRP_CNFSFT_CONNECTED_STAT;
    ptDevInfo->ucMngCnt     = 0;
    ptDevInfo->ucUnRegFlag  = GRP_USB_FALSE;
    ptDevInfo->ucCnfIdx     = ucCnfIdx;                                 /* load the saved value */
    ptDevInfo->ucTmpFlag    = ucTmpFlag;                                /* load the saved value */
    ptDevInfo->usLangId     = usLangId;                                 /* load the saved value */

    /* Get Configuration Descriptor (header only) */
    lStat = _grp_cnfsft_GetAllConfigPhase( usDevID, ptDevInfo);
    if (lStat != GRP_CNFSFT_OK) {
        _TRACE_USBC_CNFSFT_(0x05, 0x02, F_END);
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
            return GRP_CNFSFT_ERROR;
        }
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x05, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : grp_cnfsft_ConfigSoftware                                                       */
/*                                                                                              */
/* DESCRIPTION: Execute processing for several events.                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usEvent                         Specified event from HC                         */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_cnfsft_ConfigSoftware( grp_u16 usEvent, grp_u16 usDevID)
{
    _TRACE_USBC_CNFSFT_(0x06, 0x00, 0x00);

    /* check event */
    switch (usEvent) {

    case GRP_CNFSFT_DEVICE_ATTACHED:
        /* Connected device */
        _grp_cnfsft_Connect( usDevID);
        break;

    case GRP_CNFSFT_DEVICE_DETACHED:
        /* Disconnected device */
        _grp_cnfsft_Disconnect( usEvent, usDevID);
        break;

    default:
        /* error case */
        break;
    }

    _TRACE_USBC_CNFSFT_(0x06, 0x00, F_END);

    return;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_DevMngGetBuf                                                        */
/*                                                                                              */
/* DESCRIPTION: Get buffer of the manegement area.                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/* OUTPUT     : pptDevInfo                      Structure of the Device informations            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_DevMngGetBuf( _grp_cnfsft_dev_info **pptDevInfo, grp_u16 usDevID)
{
grp_si                          iIndex;

    _TRACE_USBC_CNFSFT_(0x07, 0x00, 0x00);

    /* get the device address from the device ID */
    iIndex = GRP_USBD_DEVID2ADDR( usDevID );
    iIndex--;       /*  */
    if ((iIndex < 0) || (GRP_USB_HOST_DEVICE_MAX <= iIndex)) {
        /* error */
        *pptDevInfo = GRP_USB_NULL;

        _TRACE_USBC_CNFSFT_(0x07, 0x01, F_END);
        return GRP_CNFSFT_ERROR;
    }
    else {
        /* success */
        *pptDevInfo = &l_atDevInfo[iIndex];

        _TRACE_USBC_CNFSFT_(0x07, 0x00, F_END);
        return GRP_CNFSFT_OK;
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_cnfsft_SetNotification                                                      */
/*                                                                                              */
/* DESCRIPTION: Registration of device informations.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptInput                         Structure of Registration informations          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_MULTIPLE_SETTING     Registration by same information < not error >  */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cnfsft_SetNotification( grp_cnfsft_registration *ptInput)
{
grp_s32                         lStat   = GRP_CNFSFT_ERROR;

    _TRACE_USBC_CNFSFT_(0x08, 0x00, 0x00);

    lStat = _grp_cnfsft_RegDataSet( ptInput);

    _TRACE_USBC_CNFSFT_(0x08, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_RegDataSet                                                          */
/*                                                                                              */
/* DESCRIPTION: Registration of device informations.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptInput                         Structure of Registration informations          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_MULTIPLE_SETTING     Registration by same information < not error >  */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_RegDataSet( grp_cnfsft_registration *ptInput)
{
grp_cnfsft_registration         *ptRegInfo  = GRP_USB_NULL;
grp_s32                         lStat       = GRP_CNFSFT_OK;
grp_si                          i;

    _TRACE_USBC_CNFSFT_(0x09, 0x00, 0x00);

#ifndef GRP_USB_USE_INVALIDE_SPECIFIED
    if (ptInput->usLoadOption == GRP_CNFSFT_INVALID_SPECIFIED) {
        /* invalide parameter */
        return GRP_CNFSFT_ERROR;
    }
#endif /* GRP_USB_USE_INVALIDE_SPECIFIED */

    /* lock */
    if (GRP_CNFSFT_INFO_LOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* check registered number */
    if (l_tInternalInfo.usRegCnt >= GRP_USBD_HOST_MAX_REGISTER) {
        /* error */
        lStat = GRP_CNFSFT_ERROR;
    }
    else {
        for (i=0; i<l_tInternalInfo.usRegCnt; i++) {
            /* check registered data */
            if (grp_std_memcmp( &l_atRegInfo[i], ptInput, sizeof(grp_cnfsft_registration)) == 0) {
                /* correspond to information */
                lStat = GRP_CNFSFT_MULTIPLE_SETTING;
                break;
            }
        }

        if (lStat == GRP_CNFSFT_OK) {
            /* get free area */
            ptRegInfo = &l_atRegInfo[l_tInternalInfo.usRegCnt];

            /* register informations */
            ptRegInfo->usVendorID           = ptInput->usVendorID;
            ptRegInfo->usProductID          = ptInput->usProductID;
            ptRegInfo->ucInterfaceClass     = ptInput->ucInterfaceClass;
            ptRegInfo->ucInterfaceSubClass  = ptInput->ucInterfaceSubClass;
            ptRegInfo->ucInterfaceProtocol  = ptInput->ucInterfaceProtocol;
            ptRegInfo->ucDeviceClass        = ptInput->ucDeviceClass;
            ptRegInfo->usLoadOption         = ptInput->usLoadOption;
            ptRegInfo->pfnEventNotification = ptInput->pfnEventNotification;
            ptRegInfo->pvReference          = ptInput->pvReference;

            /* renew the registered number */
            l_tInternalInfo.usRegCnt++;
        }
    }

    /* unlock */
    if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x09, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_Disconnect                                                          */
/*                                                                                              */
/* DESCRIPTION: Device disconnect process.                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usEvent                         event                                           */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_cnfsft_Disconnect( grp_u16 usEvent, grp_u16 usDevID)
{
_grp_cnfsft_dev_info            *ptDevInfo  = GRP_USB_NULL;
grp_cnfsft_registration         *ptRegInfo  = GRP_USB_NULL;
grp_si                          i;

    _TRACE_USBC_CNFSFT_(0x0A, 0x00, 0x00);

    /* Get the management area */
    _grp_cnfsft_DevMngGetBuf( &ptDevInfo, usDevID);
    if (!ptDevInfo) {
        _TRACE_USBC_CNFSFT_(0x0A, 0x01, F_END);
        /* error */
        return;
    }

    /* lock */
    if (GRP_CNFSFT_INFO_LOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return;
    }

    if (ptDevInfo->ucUnRegFlag == GRP_USB_TRUE) {
        /* No Registration Device's process */
        _grp_cnfsft_Unregistered( ptDevInfo, usDevID, GRP_CNFSFT_DEVICE_DETACHED);
    }
    else {
        for (i=0; i<GRP_USBD_HOST_MAX_CLASS; i++) {
            /* check assigned */
            if (ptDevInfo->atIndex[i].iIndex != GRP_CNFSFT_NO_ASSIGNED) {

                ptRegInfo = &l_atRegInfo[ptDevInfo->atIndex[i].iIndex];

                /* set parameters */
                ptDevInfo->atIndex[i].tNotify.usEvent = usEvent;

                if ((ptRegInfo->pfnEventNotification)
                 && (ptDevInfo->atIndex[i].iNotifyFlag == GRP_CNFSFT_NOTIFIED_UPPER)) {
                    /* unlock */
                    if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                        /* error */
                        return;
                    }

                    /* called callback function */
                    (*ptRegInfo->pfnEventNotification)( &ptDevInfo->atIndex[i].tNotify);

                    /* lock */
                    if (GRP_CNFSFT_INFO_LOCK() != GRP_CNFSFT_LOCK_OK) {
                        /* error */
                        return;
                    }
                }

                /* clear */
                ptDevInfo->atIndex[i].iIndex        = GRP_CNFSFT_NO_ASSIGNED;
                ptDevInfo->atIndex[i].iNotifyFlag   = GRP_CNFSFT_NOT_NOTIFY;
            }
        }
    }

    /* clear informations */
    ptDevInfo->ucEnumStat   = GRP_CNFSFT_DEFAULT_PHASE;
    ptDevInfo->ucConnStat   = GRP_CNFSFT_DISCONNECT_STAT;
    ptDevInfo->ucCnfIdx     = 0;
    ptDevInfo->ucTmpFlag    = GRP_USB_FALSE;
    ptDevInfo->usLangId     = 0;

    /* unlock */
    if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return;
    }

    _TRACE_USBC_CNFSFT_(0x0A, 0x00, F_END);

    return;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_Connect                                                             */
/*                                                                                              */
/* DESCRIPTION: Device connect process.                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_cnfsft_Connect( grp_u16 usDevID)
{
_grp_cnfsft_dev_info            *ptDevInfo  = GRP_USB_NULL;
grp_usbdi_st_device_request     *ptDevReq   = GRP_USB_NULL;
grp_s32                         lStat       = GRP_CNFSFT_OK;

    _TRACE_USBC_CNFSFT_(0x0B, 0x00, 0x00);

    /* lock */
    if (GRP_CNFSFT_ENUM_LOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return ;
    }

    /* Get the management area */
    _grp_cnfsft_DevMngGetBuf( &ptDevInfo, usDevID);
    if (!ptDevInfo) {
        _TRACE_USBC_CNFSFT_(0x0B, 0x01, F_END);
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
            return;
        }
        /* error */
        return;
    }

    /* Set parameters to the management area */
#ifdef GRP_CNFSFT_START_STRING
    ptDevInfo->ucEnumStat   = GRP_CNFSFT_GET_LANGID_PHASE;
#else
    ptDevInfo->ucEnumStat   = GRP_CNFSFT_GET_CONFIG_PHASE;
#endif
    ptDevInfo->ucConnStat   = GRP_CNFSFT_CONNECTED_STAT;
    ptDevInfo->ucCnfIdx     = 0;
    ptDevInfo->ucMngCnt     = 0;
    ptDevInfo->ucUnRegFlag  = GRP_USB_FALSE;
    ptDevInfo->ucTmpFlag    = GRP_USB_FALSE;
    ptDevInfo->usLangId     = 0;

    /* Create Standard Device Request Structure */
    ptDevReq = &l_tInternalInfo.tStDevReq;

    ptDevReq->usUsbDevId    = usDevID;
    ptDevReq->pvDescriptor  = ptDevInfo->pucDevDesc;
    ptDevReq->pfnStCbFunc   = _grp_cnfsft_CmpGetDeviceDesc;

    /* Get Device Descriptor */
    lStat = grp_usbd_GetDeviceDescriptor( ptDevReq);
    if (lStat != GRP_USBD_OK) {
        _TRACE_USBC_CNFSFT_(0x0B, 0x02, F_END);
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
            return;
        }
    }

    _TRACE_USBC_CNFSFT_(0x0B, 0x00, F_END);

    return;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CmpGetDeviceDesc                                                    */
/*                                                                                              */
/* DESCRIPTION: Get Device Descriptor request callback function                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptStDevReq                      standard device request structure               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_CmpGetDeviceDesc( grp_usbdi_st_device_request *ptStDevReq)
{
grp_s32                         lStat       = GRP_CNFSFT_OK;

    _TRACE_USBC_CNFSFT_(0x0C, 0x00, 0x00);

    lStat = _grp_cnfsft_GetDeviceInformation( ptStDevReq->usUsbDevId, ptStDevReq->lStatus);

    _TRACE_USBC_CNFSFT_(0x0C, 0x00, F_END);

    return lStat;
}

#ifdef GRP_USB_USE_MTP
/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CmpDeviceReq                                                        */
/*                                                                                              */
/* DESCRIPTION: Device request callback function                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevReq                        device request structure                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_CmpDeviceReq( grp_usbdi_device_request *ptDevReq)
{
grp_s32                         lStat       = GRP_CNFSFT_OK;

    _TRACE_USBC_CNFSFT_(0x0D, 0x00, 0x00);

    lStat = _grp_cnfsft_GetDeviceInformation( ptDevReq->usUsbDevId, ptDevReq->lStatus);

    _TRACE_USBC_CNFSFT_(0x0D, 0x00, F_END);

    return lStat;
}
#endif  /* GRP_USB_USE_MTP */

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetStringDesc                                                       */
/*                                                                                              */
/* DESCRIPTION: Get STRING Descriptor                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/*              ucIndex                         index number                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetStringDesc( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo, grp_u8 ucIndex)
{
grp_usbdi_st_device_request     *ptStDevReq = &l_tInternalInfo.tStDevReq;
grp_u16                         usStrLen    = GRP_CNFSFT_STRING_SIZE;

    /* check request length */
    if (usStrLen > GRP_CMEM_BSIZE_CNF_CONF) {
        usStrLen = GRP_CMEM_BSIZE_CNF_CONF;
    }

    /* set parameters */
    ptStDevReq->usUsbDevId      = usDevID;
    ptStDevReq->ucIndex         = ucIndex;
    ptStDevReq->usLangID        = ptDevInfo->usLangId;
    ptStDevReq->ulSize          = usStrLen;
    ptStDevReq->pvDescriptor    = ptDevInfo->pucConfDesc;           /* used the ConfDesc area   */
    ptStDevReq->pfnStCbFunc     = _grp_cnfsft_CmpGetDeviceDesc;

    if (grp_usbd_GetStringDescriptor( ptStDevReq) != GRP_USBD_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }
    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetLangidPhase                                                      */
/*                                                                                              */
/* DESCRIPTION: Get STRING Descriptor (LANGIDs)                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetLangidPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo)
{
grp_u8                          ucIdx1 = *(ptDevInfo->pucDevDesc+0x0E); /* 0x0E : iManufacture  */
grp_u8                          ucIdx2 = *(ptDevInfo->pucDevDesc+0x0F); /* 0x0F : iProduct      */
grp_u8                          ucIdx3 = *(ptDevInfo->pucDevDesc+0x10); /* 0x10 : iSerialNumber */
grp_s32                         lStat;

    /* change status */
    if ( (ucIdx1 + ucIdx2 + ucIdx3) != 0) {
        ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_MANUFACT_PHASE;
    }
    else {
        ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_CONFIG_PHASE;
        return GRP_CNFSFT_EX_ERROR;
    }
    /* check conditions */
    lStat = _grp_cnfsft_CustomisedFunction( GRP_CNFSFT_CHK_LANGID_PHASE, ptDevInfo);
    if (lStat != GRP_CNFSFT_OK) {
        /* exit this function */
        return lStat;
    }
    /* Get String Descriptor (LANGIDs) */
    if (_grp_cnfsft_GetStringDesc( usDevID, ptDevInfo, GRP_CNFSFT_STR_LANGID) != GRP_CNFSFT_OK) {
        return GRP_CNFSFT_ERROR;
    }
    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetManufacturePhase                                                 */
/*                                                                                              */
/* DESCRIPTION: Get STRING Descriptor (Manufacture data)                                        */
/*              If the condtion is not suitable then the GetString is not executed.             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*              GRP_CNFSFT_EX_ERROR             Extra error (go on a next process)              */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetManufacturePhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo)
{
grp_u8                          ucIndex = *(ptDevInfo->pucDevDesc+0x0E); /* 0x0E : iManufacture */
grp_s32                         lStat;

    if (ucIndex == 0) {                                                     /* No string data   */
        /* change status */
        ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_PRODUCT_PHASE;
        return GRP_CNFSFT_EX_ERROR;
    }
    /* change status */
    ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_PRODUCT_PHASE;
    /* check conditions */
    lStat = _grp_cnfsft_CustomisedFunction( GRP_CNFSFT_CHK_MANUFACT_PHASE, ptDevInfo);
    if (lStat != GRP_CNFSFT_OK) {
        /* exit this function */
        return lStat;
    }
    /* check LANGID */
    if (ptDevInfo->usLangId == 0) {
        /* change status */
        ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_CONFIG_PHASE;
        return GRP_CNFSFT_EX_ERROR;
    }
    /* Get STRING Descriptor (Manufacture data) */
    if (_grp_cnfsft_GetStringDesc( usDevID, ptDevInfo, ucIndex) != GRP_CNFSFT_OK) {
        return GRP_CNFSFT_ERROR;
    }
    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetProductPhase                                                     */
/*                                                                                              */
/* DESCRIPTION: Get STRING Descriptor (Product data)                                            */
/*              If the condtion is not suitable then the GetString is not executed.             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*              GRP_CNFSFT_EX_ERROR             Extra error (go on a next process)              */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetProductPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo)
{
grp_u8                          ucIndex = *(ptDevInfo->pucDevDesc+0x0F); /* 0x0F : iProduct     */
grp_s32                         lStat;

    /* change status */
    ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_CONFIG_PHASE;
    if (ucIndex == 0) {                                                     /* No string data   */
        return GRP_CNFSFT_EX_ERROR;
    }
    /* check conditions */
    lStat = _grp_cnfsft_CustomisedFunction( GRP_CNFSFT_CHK_PRODUCT_PHASE, ptDevInfo);
    if (lStat != GRP_CNFSFT_OK) {
        /* exit this function */
        return lStat;
    }
    /* check LANGID */
    if (ptDevInfo->usLangId == 0) {
        /* change status */
        ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_CONFIG_PHASE;
        return GRP_CNFSFT_EX_ERROR;
    }
    /* Get STRING Descriptor (Product data) */
    if (_grp_cnfsft_GetStringDesc( usDevID, ptDevInfo, ucIndex) != GRP_CNFSFT_OK) {
        return GRP_CNFSFT_ERROR;
    }
    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetConfigPhase                                                      */
/*                                                                                              */
/* DESCRIPTION: Get CONFIGURATIOM Descriptor (top)                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetConfigPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo)
{
grp_s32                         lStat;

    /* change status */
    ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_ALLCONFIG_PHASE;
    /* check conditions */
    lStat = _grp_cnfsft_CustomisedFunction( GRP_CNFSFT_CHK_CONFIG_PHASE, ptDevInfo);
    if (lStat != GRP_CNFSFT_OK) {
        /* exit this function */
        return lStat;
    }
    /* Get Configuration Descriptor (header only) */
    if (_grp_cnfsft_GetConfigDesc( usDevID, ptDevInfo, (grp_u16)GRP_USBD_CONFIG_DESC_SIZE)
        != GRP_CNFSFT_OK) {
        return GRP_CNFSFT_ERROR;
    }
    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetAllConfigPhase                                                   */
/*                                                                                              */
/* DESCRIPTION: Get CONFIGURATIOM Descriptor (all data)                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetAllConfigPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo)
{
grp_usbdi_config_desc_b         *ptConfDesc = (grp_usbdi_config_desc_b *)ptDevInfo->pucConfDesc;
grp_u16                         usConfSize  = 0;

    /* change status */
#ifdef GRP_USB_USE_MTP
    ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_OS_STRING_PHASE;
#else
    ptDevInfo->ucEnumStat = GRP_CNFSFT_NOMAL_DEVICE;
#endif /* GRP_USB_USE_MTP */

    /* Calculate Configuration Descriptor Size */
    usConfSize = _GRP_CNFSFT_CRE_UINT16( ptConfDesc->wTotalLength_High, ptConfDesc->wTotalLength_Low);
    /* Get Configuration Descriptor (all data) */
    if (_grp_cnfsft_GetConfigDesc( usDevID, ptDevInfo, usConfSize) != GRP_CNFSFT_OK) {
        return GRP_CNFSFT_ERROR;
    }
    return GRP_CNFSFT_OK;
}

#ifdef GRP_USB_USE_MTP
/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetOsStringPhase                                                    */
/*                                                                                              */
/* DESCRIPTION: Get OsString Descriptor (for MTP)                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetOsStringPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo)
{
    /* change status */
    ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_MS_DESC_PHASE;
    /* Get OS String descriptor */
    if (_grp_cnfsft_GetOsStringDesc( usDevID) != GRP_CNFSFT_OK) {
        return GRP_CNFSFT_ERROR;
    }
    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetMsDescPhase                                                      */
/*                                                                                              */
/* DESCRIPTION: Get MS Descriptor (for MTP)                                                     */
/*              If the condtion is not suitable then the GetMsDesc is not executed.             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevID                         Device identifier                               */
/*              ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetMsDescPhase( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo)
{
    /* check descriptor informations */
    if (_grp_cnfsft_ChkOsStringDesc() != GRP_CNFSFT_OK) {
        /* change status */
        ptDevInfo->ucEnumStat = GRP_CNFSFT_NOMAL_DEVICE;
        /* set local event flag */
        return GRP_CNFSFT_FOUND_DEVICE;
    }
    else {
        /* change status */
        ptDevInfo->ucEnumStat = GRP_CNFSFT_MTP_DEVICE;
        /* Get MTP Feature descriptor */
        if (_grp_cnfsft_GetMtpFeatureDesc( usDevID, 
                                    l_tInternalInfo.ptString->bMS_VendorCode) != GRP_CNFSFT_OK) {
            return GRP_CNFSFT_ERROR;
        }
    }
    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_ChkDev                                                              */
/*                                                                                              */
/* DESCRIPTION: Check port status                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Structure of the Device informations            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_FOUND_DEVICE         Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_ChkDev( _grp_cnfsft_dev_info *ptDevInfo)
{
    /* check port states */
    if (ptDevInfo->ucConnStat != GRP_CNFSFT_DISCONNECT_STAT) {
        /* change status */
        ptDevInfo->ucEnumStat = GRP_CNFSFT_NOMAL_DEVICE;
        /* set local event flag */
        return GRP_CNFSFT_FOUND_DEVICE;
    }
    return GRP_CNFSFT_ERROR;
}
#endif /* GRP_USB_USE_MTP */

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetDeviceInformation                                                */
/*                                                                                              */
/* DESCRIPTION: Get Device information (configuration descriptor)                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*              lReqStat                        Previous requests status                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetDeviceInformation( grp_u16 usDevID, grp_s32 lReqStat)
{
_grp_cnfsft_dev_info            *ptDevInfo  = GRP_USB_NULL;
grp_s32                         lStat       = GRP_CNFSFT_EX_ERROR;

    _TRACE_USBC_CNFSFT_(0x0E, 0x00, 0x00);

    /* Get the management area */
    _grp_cnfsft_DevMngGetBuf( &ptDevInfo, usDevID);

    /* check transfer status and getted area */
    if (!ptDevInfo) {
        _TRACE_USBC_CNFSFT_(0x0E, 0x01, F_END);
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
        }
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    while (lStat == GRP_CNFSFT_EX_ERROR) {
        lStat = GRP_CNFSFT_ERROR;
        switch (ptDevInfo->ucEnumStat) {
        case GRP_CNFSFT_GET_LANGID_PHASE: /*----------------------------------------------------*/
            if (lReqStat == GRP_USBD_TR_NO_FAIL) {
                lStat = _grp_cnfsft_GetLangidPhase( usDevID, ptDevInfo);
            }
            break;

        case GRP_CNFSFT_GET_MANUFACT_PHASE: /*--------------------------------------------------*/
            lStat = _grp_cnfsft_GetManufacturePhase( usDevID, ptDevInfo);
            break;

        case GRP_CNFSFT_GET_PRODUCT_PHASE: /*---------------------------------------------------*/
            lStat = _grp_cnfsft_GetProductPhase( usDevID, ptDevInfo);
            break;

        case GRP_CNFSFT_GET_CONFIG_PHASE: /*----------------------------------------------------*/
            lStat = _grp_cnfsft_GetConfigPhase( usDevID, ptDevInfo);
            break;

        case GRP_CNFSFT_GET_ALLCONFIG_PHASE: /*-------------------------------------------------*/
            if (lReqStat == GRP_USBD_TR_NO_FAIL) {
                lStat = _grp_cnfsft_GetAllConfigPhase( usDevID, ptDevInfo);
            }
            break;

        case GRP_CNFSFT_NOMAL_DEVICE: /*--------------------------------------------------------*/
            if (lReqStat == GRP_USBD_TR_NO_FAIL) {
                /* set local event flag */
                lStat = GRP_CNFSFT_FOUND_DEVICE;
            }
            break;

#ifdef GRP_USB_USE_MTP
        case GRP_CNFSFT_GET_OS_STRING_PHASE: /*-------------------------------------------------*/
            if (lReqStat == GRP_USBD_TR_NO_FAIL) {
                lStat = _grp_cnfsft_GetOsStringPhase( usDevID, ptDevInfo);
            }
            break;

        case GRP_CNFSFT_GET_MS_DESC_PHASE: /*---------------------------------------------------*/
            if (lReqStat == GRP_USBD_TR_NO_FAIL) {
                lStat = _grp_cnfsft_GetMsDescPhase( usDevID, ptDevInfo);
            }
            else {
                lStat = _grp_cnfsft_ChkDev(ptDevInfo);
            }
            break;

        case GRP_CNFSFT_MTP_DEVICE: /*----------------------------------------------------------*/
            if (lReqStat == GRP_USBD_TR_NO_FAIL) {
                /* check descriptor informations */
                if (_grp_cnfsft_ChkMtpFeatureDesc() != GRP_CNFSFT_OK) {
                    /* change status */
                    ptDevInfo->ucEnumStat = GRP_CNFSFT_NOMAL_DEVICE;
                }
                /* set local event flag */
                lStat = GRP_CNFSFT_FOUND_DEVICE;
            }
            else {
                lStat = _grp_cnfsft_ChkDev(ptDevInfo);
            }
            break;
#endif /* GRP_USB_USE_MTP */

        default: /*----------------------------------------------------------------------------*/
            /* illegal error */
            lStat = GRP_CNFSFT_ERROR;
            break;
        }
    }

    /* finished a process of enumeration? */
    if (lStat == GRP_CNFSFT_FOUND_DEVICE) {
        /* Search Device */
        if (_grp_cnfsft_SearchDevice( ptDevInfo, usDevID) != GRP_CNFSFT_OK) {
            /* change status */
            ptDevInfo->ucEnumStat = GRP_CNFSFT_CONNENCT_PHASE;
            /* unlock */
            if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                /* error */
            }
            /* error end */
            return GRP_CNFSFT_ERROR;
        }
    }
    else if (lStat == GRP_CNFSFT_ERROR) {
        /* change status */
        ptDevInfo->ucEnumStat = GRP_CNFSFT_CONNENCT_PHASE;
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
        }
        /* error end */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x0E, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetConfigDesc                                                       */
/*                                                                                              */
/* DESCRIPTION: Executed GET_DESCRIPTOR                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usCnfDescSize                   Reqested size                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetConfigDesc( grp_u16 usDevID, _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usCnfDescSize)
{
grp_usbdi_st_device_request     *ptStDevReq     = &l_tInternalInfo.tStDevReq;
grp_s32                         lStat           = 0;

    _TRACE_USBC_CNFSFT_(0x0F, 0x00, 0x00);

    /* parameter check */
    if (usCnfDescSize > GRP_CMEM_BSIZE_CNF_CONF) {
        /* parameter error */
        return GRP_CNFSFT_ERROR;
    }

    /* set parameters */
    ptStDevReq->usUsbDevId      = usDevID;
    ptStDevReq->ucConfiguration = ptDevInfo->ucCnfIdx;
    ptStDevReq->ulSize          = usCnfDescSize;
    ptStDevReq->pvDescriptor    = ptDevInfo->pucConfDesc;
    ptStDevReq->pfnStCbFunc     = _grp_cnfsft_CmpGetDeviceDesc;

    lStat = grp_usbd_GetConfigDescriptor( ptStDevReq);
    if (lStat != GRP_USBD_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x0F, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_SearchDevice                                                        */
/*                                                                                              */
/* DESCRIPTION: Search Device                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_SearchDevice( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID)
{
grp_usbdi_dev_desc_b            *ptDevDesc  = GRP_USB_NULL;
grp_s32                         lStat       = GRP_CNFSFT_OK;

    _TRACE_USBC_CNFSFT_(0x10, 0x00, 0x00);

    /* lock */
    if (GRP_CNFSFT_INFO_LOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    /* check port states */
    if (ptDevInfo->ucConnStat != GRP_CNFSFT_DISCONNECT_STAT) {

#ifdef GRP_USB_USE_INVALIDE_SPECIFIED
        if (ptDevInfo->ucTmpFlag == GRP_USB_TRUE) {
            lStat = _grp_cnfsft_InvalidSearch( ptDevInfo, usDevID);
            if (lStat == GRP_CNFSFT_OK) {
                _TRACE_USBC_CNFSFT_(0x10, 0x01, 0x00);
                /* unlock */
                if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                    /* error */
                }
                /* called callback function */
                _grp_cnfsft_CallbackFunc( ptDevInfo);
                _TRACE_USBC_CNFSFT_(0x10, 0x00, F_END);
                /* unlock */
                if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                    /* error */
                    return GRP_CNFSFT_ERROR;
                }
                /* process end */
                return GRP_CNFSFT_OK;
            }
        }
#endif /* GRP_USB_USE_INVALIDE_SPECIFIED */

        lStat = _grp_cnfsft_CheckDevDesc( ptDevInfo, usDevID, GRP_CNFSFT_VENDOR_SPECIFIED);         /* V1.02 */
        if (lStat == GRP_CNFSFT_VENDOR) {
            _TRACE_USBC_CNFSFT_(0x10, 0x02, 0x00);
            /* unlock */
            if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                /* error */
            }
            /* called callback function */
            _grp_cnfsft_CallbackFunc( ptDevInfo);
            _TRACE_USBC_CNFSFT_(0x10, 0x00, F_END);
            /* unlock */
            if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                /* error */
                return GRP_CNFSFT_ERROR;
            }
            /* process end */
            return GRP_CNFSFT_OK;
        }

#ifdef GRP_USB_USE_MTP
        /* check Enumeration status */
        if (ptDevInfo->ucEnumStat == GRP_CNFSFT_MTP_DEVICE) {
            /* MTP device */
            lStat = _grp_cnfsft_MtpSearch( ptDevInfo, usDevID);
        }
        else
#endif
        {
            ptDevDesc = (grp_usbdi_dev_desc_b *)ptDevInfo->pucDevDesc;

            /* check IAD descriptor */
            if ((ptDevDesc->bDeviceClass    == GRP_CNFSFT_EF)
             && (ptDevDesc->bDeviceSubClass == GRP_CNFSFT_CMN_CLASS)
             && (ptDevDesc->bDeviceProtocol == GRP_CNFSFT_IAD)) {
                /* Composite device */
                lStat = _grp_cnfsft_IadDevSearch( ptDevInfo, usDevID);
            }
            else {
                /* Normal Device */
                lStat = _grp_cnfsft_NormalSearch( ptDevInfo, usDevID);
            }
        }
    }

    /* unlock */
    if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x10, 0x00, F_END);

    return lStat;
}

#ifdef GRP_USB_USE_INVALIDE_SPECIFIED
/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_InvalidSearch                                                       */
/*                                                                                              */
/* DESCRIPTION: Check user specified condition on Normal device                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK           Success                                                 */
/*              GRP_CNFSFT_ERROR        Error                                                   */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_InvalidSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID)
{
grp_cnfsft_notify               *ptNotify   = GRP_USB_NULL;
grp_s32                         lStat       = GRP_CNFSFT_ERROR;
grp_si                          i;

    /* Search GRP_CNFSFT_INVALID_SPECIFIED option */
    for (i=0; i<l_tInternalInfo.usRegCnt; i++) {
        if (l_atRegInfo[i].usLoadOption == GRP_CNFSFT_INVALID_SPECIFIED) {
            lStat = GRP_CNFSFT_OK;
            break;
        }
    }
    
    /* check of the upper limit */
    if (ptDevInfo->ucMngCnt >= GRP_USBD_HOST_MAX_CLASS) {
        lStat = GRP_CNFSFT_ERROR;
    }
    
    if (lStat == GRP_CNFSFT_OK) {
        /* save informations and notice to upper layar */
        ptDevInfo->atIndex[ptDevInfo->ucMngCnt].iIndex = i;
        /* set parameters */
        ptNotify = &ptDevInfo->atIndex[ptDevInfo->ucMngCnt].tNotify;
        ptNotify->usEvent           = GRP_CNFSFT_DEVICE_ATTACHED;
        ptNotify->usUsbDevId        = usDevID;
        ptNotify->usConfigIdx       = ptDevInfo->ucCnfIdx;
        ptNotify->ucInterfaceNum    = 0;
        ptNotify->ucTotalIfNum      = 0;
        ptNotify->pucInterfaceDesc  = GRP_USB_NULL;
        ptNotify->pucDevDesc        = ptDevInfo->pucDevDesc;
        ptNotify->pucConfDesc       = ptDevInfo->pucConfDesc;
        ptNotify->pvReference       = l_atRegInfo[i].pvReference;
        /* update counter */
        ptDevInfo->ucMngCnt++;
    }

    return lStat;
}
#endif /* GRP_USB_USE_INVALIDE_SPECIFIED */

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_NormalSearch                                                        */
/*                                                                                              */
/* DESCRIPTION: Search Device in Normal Descriptor                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK           Success                                                 */
/*              GRP_CNFSFT_ERROR        Error                                                   */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_NormalSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID)
{
grp_usbdi_dev_desc_b            *ptDevDesc;
grp_s32                         lRet    = GRP_CNFSFT_NOMATCH;
grp_s32                         lStat   = GRP_CNFSFT_ERROR;

    _TRACE_USBC_CNFSFT_(0x11, 0x00, 0x00);

    ptDevDesc = (grp_usbdi_dev_desc_b *)ptDevInfo->pucDevDesc;

#ifdef GRP_USB_USE_INVALIDE_SPECIFIED
    if (ptDevInfo->ucTmpFlag == GRP_USB_TRUE) {
        lStat = _grp_cnfsft_InvalidSearch( ptDevInfo, usDevID);
        if (lStat == GRP_CNFSFT_OK) {
            _TRACE_USBC_CNFSFT_(0x11, 0x01, 0x00);
            /* unlock */
            if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                /* error */
            }
            /* called callback function */
            _grp_cnfsft_CallbackFunc( ptDevInfo);
            _TRACE_USBC_CNFSFT_(0x11, 0x00, F_END);
            /* process end */
            return GRP_CNFSFT_OK;
        }
    }
#endif /* GRP_USB_USE_INVALIDE_SPECIFIED */

    /* --- search Vendor specific class or Device class code --- */
    lRet = _grp_cnfsft_CheckDevDesc( ptDevInfo, usDevID, GRP_CNFSFT_DEVCLASS_SPECIFIED);            /* V1.02 */
    if (lRet != GRP_CNFSFT_DEVCLASS) {
        /* --- search Interface descriptor --- */
        lRet = _grp_cnfsft_CheckDescriptor( ptDevInfo, usDevID);
    }

    if (lRet != GRP_CNFSFT_NOMATCH) {
        /* request SET_CONFIGURATION */
        lStat = _grp_cnfsft_SetConfiguration( ptDevInfo, usDevID);
    }
    else {
        if ((ptDevInfo->ucConnStat == GRP_CNFSFT_CONNECTED_STAT)
         && ((ptDevInfo->ucCnfIdx+1) < ptDevDesc->bNumConfiguration)) {
            /* renew configuration number */
            ptDevInfo->ucCnfIdx++;
            /* Get the next configurations information */
            /* change status */
            ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_ALLCONFIG_PHASE;
            /* Get Configuration Descriptor (header only) */
            lStat = _grp_cnfsft_GetConfigDesc( usDevID, ptDevInfo, (grp_u16)GRP_USBD_CONFIG_DESC_SIZE);
        }
        else {
            /* not corresponded */
            ptDevInfo->ucCnfIdx = 0;
            /* Notify to the application */
            lStat = _grp_cnfsft_Unregistered( ptDevInfo, usDevID, GRP_CNFSFT_DEVICE_ATTACHED);
        }
    }

    _TRACE_USBC_CNFSFT_(0x11, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_IadDevSearch                                                        */
/*                                                                                              */
/* DESCRIPTION: Search Device have IAD Descriptor                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK           Success                                                 */
/*              GRP_CNFSFT_ERROR        Error                                                   */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_IadDevSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID)
{
grp_usbdi_dev_desc_b            *ptDevDesc  = (grp_usbdi_dev_desc_b *)ptDevInfo->pucDevDesc;
grp_s32                         lStat       = GRP_CNFSFT_NOMATCH;

    _TRACE_USBC_CNFSFT_(0x12, 0x00, 0x00);

    /* check descriptor */
    lStat = _grp_cnfsft_CheckDescriptor( ptDevInfo, usDevID);

    if (lStat != GRP_CNFSFT_NOMATCH) {
        /* request SET_CONFIGURATION */
        lStat = _grp_cnfsft_SetConfiguration( ptDevInfo, usDevID);
    }
    else {
        if ((ptDevInfo->ucConnStat == GRP_CNFSFT_CONNECTED_STAT)
         && ((ptDevInfo->ucCnfIdx+1) < ptDevDesc->bNumConfiguration)) {
            /* renew configuration number */
            ptDevInfo->ucCnfIdx++;
            /* Get the next configurations information */
            /* change status */
            ptDevInfo->ucEnumStat = GRP_CNFSFT_GET_ALLCONFIG_PHASE;
            /* Get Configuration Descriptor (header only) */
            lStat = _grp_cnfsft_GetConfigDesc( usDevID, ptDevInfo, (grp_u16)GRP_USBD_CONFIG_DESC_SIZE);
        }
        else {
            /* not corresponded */
            ptDevInfo->ucCnfIdx = 0;
            /* Notify to the application */
            lStat = _grp_cnfsft_Unregistered( ptDevInfo, usDevID, GRP_CNFSFT_DEVICE_ATTACHED);
        }
    }

    _TRACE_USBC_CNFSFT_(0x12, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CheckDevDesc                                                        */
/*                                                                                              */
/* DESCRIPTION: Check device descriptor on the Normal device                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_VENDOR               Corresponded Vendor specific                    */
/*              GRP_CNFSFT_DEVCLASS             Corresponded DevClass specific                  */
/*              GRP_CNFSFT_NOMATCH              No match                                        */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_CheckDevDesc( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID, grp_si iFlag) /* V1.02 */
{
grp_usbdi_dev_desc_b            *ptDevDesc;
grp_usbdi_config_desc_b         *ptConfDesc;
grp_cnfsft_notify               *ptNotify   = GRP_USB_NULL;
grp_cnfsft_registration         *ptRegInfo  = GRP_USB_NULL;
grp_si                          i;
grp_s32                         lStat       = GRP_CNFSFT_NOMATCH;
grp_u16                         usVendorID;
grp_u16                         usProductID;
grp_u16                         usRemainLength;
grp_u8                          *pucTmpDesc = GRP_USB_NULL;

    _TRACE_USBC_CNFSFT_(0x13, 0x00, 0x00);
    
    /* check of the upper limit */
    if (ptDevInfo->ucMngCnt >= GRP_USBD_HOST_MAX_CLASS) {
        _TRACE_USBC_CNFSFT_(0x13, 0x01, F_END);
        return GRP_CNFSFT_NOMATCH;
    }

    ptDevDesc  = (grp_usbdi_dev_desc_b *)ptDevInfo->pucDevDesc;
    ptConfDesc = (grp_usbdi_config_desc_b *)ptDevInfo->pucConfDesc;

    /* set IDs */
    usVendorID  = _GRP_CNFSFT_CRE_UINT16( ptDevDesc->idVendor_High,  ptDevDesc->idVendor_Low);
    usProductID = _GRP_CNFSFT_CRE_UINT16( ptDevDesc->idProduct_High, ptDevDesc->idProduct_Low);

    /* Check Vendor ID and Product ID */
    for (i=0; i<l_tInternalInfo.usRegCnt; i++) {

        ptRegInfo = &l_atRegInfo[i];

        /* --- check VendorID and ProductID --- */
        if ((ptRegInfo->usLoadOption == GRP_CNFSFT_VENDOR_SPECIFIED)                    /* V1.02 */
         && (iFlag                   == GRP_CNFSFT_VENDOR_SPECIFIED)) {                 /* V1.02 */
            if ((usVendorID  == ptRegInfo->usVendorID)
             && (usProductID == ptRegInfo->usProductID)) {
                /* corresponded device */
                lStat = GRP_CNFSFT_VENDOR;
                /* set parameters */
                ptNotify = &ptDevInfo->atIndex[ptDevInfo->ucMngCnt].tNotify;
                ptNotify->ucInterfaceNum    = 0;
                ptNotify->ucTotalIfNum      = 0;
                ptNotify->pucInterfaceDesc  = GRP_USB_NULL;
            }
        }

        /* --- check Device class code --- */
        else if ((ptRegInfo->usLoadOption == GRP_CNFSFT_DEVCLASS_SPECIFIED)             /* V1.02 */
              && (iFlag                   == GRP_CNFSFT_DEVCLASS_SPECIFIED)) {          /* V1.02 */
            if (ptDevDesc->bDeviceClass == ptRegInfo->ucDeviceClass) {
                /* corresponded device */
                lStat = GRP_CNFSFT_DEVCLASS;
                /* set parameters */
                ptNotify = &ptDevInfo->atIndex[ptDevInfo->ucMngCnt].tNotify;
                ptNotify->ucInterfaceNum    = 0;
                ptNotify->ucTotalIfNum      = ptConfDesc->bNumInterface;
                /*--- search next interface descriptor ---*/
                pucTmpDesc      = ptDevInfo->pucConfDesc;
                usRemainLength  = _GRP_CNFSFT_CRE_UINT16( ptConfDesc->wTotalLength_High,
                                                          ptConfDesc->wTotalLength_Low);
                _grp_cnfsft_SearchNextIForIAD( &pucTmpDesc, &usRemainLength);
                /*--- end ---*/
                ptNotify->pucInterfaceDesc  = pucTmpDesc;
            }
        }

        if (lStat != GRP_CNFSFT_NOMATCH) {
            /* save informations and notice to upper layar */
            ptDevInfo->atIndex[ptDevInfo->ucMngCnt].iIndex = i;
            /* set parameters */
/*          ptNotify = &ptDevInfo->atIndex[ptDevInfo->ucMngCnt].tNotify;*/
                                                            /* this parameter is already set    */
            ptNotify->usEvent           = GRP_CNFSFT_DEVICE_ATTACHED;
            ptNotify->usUsbDevId        = usDevID;
            ptNotify->usConfigIdx       = ptDevInfo->ucCnfIdx;
/*          ptNotify->ucInterfaceNum    = 0;            */  /* this parameter is already set    */
/*          ptNotify->ucTotalIfNum      = 0;            */  /* this parameter is already set    */
/*          ptNotify->pucInterfaceDesc  = GRP_USB_NULL; */  /* this parameter is already set    */
            ptNotify->pucDevDesc        = ptDevInfo->pucDevDesc;
            ptNotify->pucConfDesc       = ptDevInfo->pucConfDesc;
            ptNotify->pvReference       = l_atRegInfo[i].pvReference;
            /* update counter */
            ptDevInfo->ucMngCnt++;
            /* exit FOR loop */
            break;
        }
    }

    _TRACE_USBC_CNFSFT_(0x13, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CheckDescriptor                                                     */
/*                                                                                              */
/* DESCRIPTION: Check descriptor on the IAD device                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK           Success                                                 */
/*              GRP_CNFSFT_ERROR        Error                                                   */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_CheckDescriptor( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID)
{
grp_usbdi_config_desc_b         *ptConfDesc = (grp_usbdi_config_desc_b*)ptDevInfo->pucConfDesc;
grp_usbdi_iad_desc              *ptIadDesc;
grp_si                          iIfCnt;
grp_si                          i;
grp_u16                         usRemainLength;
grp_u8                          *pucTmpDesc;

    _TRACE_USBC_CNFSFT_(0x14, 0x00, 0x00);

    /* get the total length of data returned this configuration */
    pucTmpDesc      = (grp_u8 *)ptConfDesc;
    usRemainLength  = _GRP_CNFSFT_CRE_UINT16( ptConfDesc->wTotalLength_High,
                                              ptConfDesc->wTotalLength_Low);
    /* search next interface descriptor or IAD */
    if (_grp_cnfsft_SearchNextIForIAD( &pucTmpDesc, &usRemainLength) != GRP_CNFSFT_OK) {
        _TRACE_USBC_CNFSFT_(0x14, 0x01, F_END);
        /* no Interface of IAD descriptor */
        return GRP_CNFSFT_NOMATCH;
    }

    for (iIfCnt=0; iIfCnt<ptConfDesc->bNumInterface; iIfCnt++) {
        /* IAD? */
        if (((_grp_cnfsft_wrap_desc *)pucTmpDesc)->bDescriptor == GRP_USBD_DESC_INTERFACE_ASS) {
            ptIadDesc = (grp_usbdi_iad_desc *)pucTmpDesc;
            /* check register information */
            _grp_cnfsft_CheckRegInfo( 
                                ptDevInfo, usDevID,         /* Device information and ID        */
                                (pucTmpDesc + *pucTmpDesc), /* pointer for interface descriptor */
                                ptIadDesc->bInterfaceCount);/* Number of contiguous interface   */
            /* renew the next pointer and remaind size */
            for (i=0; i<=ptIadDesc->bInterfaceCount; i++) {
                if (_grp_cnfsft_SearchNextIForIAD( &pucTmpDesc, &usRemainLength) != GRP_CNFSFT_OK) {
                    /* exit FOR loop */
                    break;
                }
            }
        }
        /* Interface Descriptor? */
        else if (((_grp_cnfsft_wrap_desc *)pucTmpDesc)->bDescriptor == GRP_USBD_DESC_INTERFACE) {
            /* check register information */
            _grp_cnfsft_CheckRegInfo( 
                                ptDevInfo, usDevID,         /* Device information and ID        */
                                pucTmpDesc,                 /* pointer for interface descriptor */
                                1);                         /* Number of contiguous interface   */
            /* renew the next pointer and remaind size */
            _grp_cnfsft_SearchNextIForIAD( &pucTmpDesc, &usRemainLength);
        }

        /* check remain data */
        if (usRemainLength == 0) {
            /* end of the descriptor */
            break;
        }
    }

    _TRACE_USBC_CNFSFT_(0x14, 0x00, F_END);

    if (ptDevInfo->ucMngCnt) {
        return GRP_CNFSFT_MATCH;
    }
    else {
        return GRP_CNFSFT_NOMATCH;
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CheckRegInfo                                                        */
/*                                                                                              */
/* DESCRIPTION: Check registed information                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_MATCH        Corresponded device                                     */
/*              GRP_CNFSFT_NOMATCH      not corresponded                                        */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _grp_cnfsft_CheckRegInfo( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID, grp_u8 *pucIfDesc, grp_u8 ucNumIf)
{
grp_usbdi_dev_desc_b            *ptDevDesc  = (grp_usbdi_dev_desc_b *)ptDevInfo->pucDevDesc;
grp_cnfsft_notify               *ptNotify   = GRP_USB_NULL;
grp_cnfsft_registration         *ptRegInfo  = GRP_USB_NULL;
grp_usbdi_if_desc               *ptIfDesc   = (grp_usbdi_if_desc *)pucIfDesc;
grp_si                          iIndex;
grp_s32                         lStat       = GRP_CNFSFT_NOMATCH;

    _TRACE_USBC_CNFSFT_(0x15, 0x00, 0x00);
    
    /* check of the upper limit */
    if (ptDevInfo->ucMngCnt >= GRP_USBD_HOST_MAX_CLASS) {
        _TRACE_USBC_CNFSFT_(0x15, 0x01, F_END);
        return GRP_CNFSFT_NOMATCH;
    }

    for (iIndex=0; iIndex<l_tInternalInfo.usRegCnt; iIndex++) {
        /* set regiseterd information */
        ptRegInfo = &l_atRegInfo[iIndex];

        switch (ptRegInfo->usLoadOption) {
        case GRP_CNFSFT_DEVCLASS_SPECIFIED:
            if (ptRegInfo->ucDeviceClass == ptDevDesc->bDeviceClass) {
                /* corresponded device */
                lStat = GRP_CNFSFT_MATCH;
            }
            break;

        case GRP_CNFSFT_INFCLASS_SPECIFIED:
            if (ptRegInfo->ucInterfaceClass == ptIfDesc->bInterfaceClass) {
                /* corresponded device */
                lStat = GRP_CNFSFT_MATCH;
            }
            break;

        case GRP_CNFSFT_SUBCLASS_SPECIFIED:
            if ((ptRegInfo->ucInterfaceClass    == ptIfDesc->bInterfaceClass)
             && (ptRegInfo->ucInterfaceSubClass == ptIfDesc->bInterfaceSubClass)) {
                /* corresponded device */
                lStat = GRP_CNFSFT_MATCH;
            }
            break;

        case GRP_CNFSFT_PROTOCOL_SPECIFIED:
            if ((ptRegInfo->ucInterfaceClass    == ptIfDesc->bInterfaceClass)
             && (ptRegInfo->ucInterfaceSubClass == ptIfDesc->bInterfaceSubClass)
             && (ptRegInfo->ucInterfaceProtocol == ptIfDesc->bInterfaceProtocol)) {
                /* corresponded device */
                lStat = GRP_CNFSFT_MATCH;
            }
            break;

        default:
            /* illegal error */
            break;
        }

        if (lStat != GRP_CNFSFT_NOMATCH) {
            /* save informations and notice to upper layar */
            ptDevInfo->atIndex[ptDevInfo->ucMngCnt].iIndex = iIndex;
            /* set parameters */
            ptNotify = &ptDevInfo->atIndex[ptDevInfo->ucMngCnt].tNotify;
            ptNotify->usEvent           = GRP_CNFSFT_DEVICE_ATTACHED;
            ptNotify->usUsbDevId        = usDevID;
            ptNotify->usConfigIdx       = ptDevInfo->ucCnfIdx;
            ptNotify->ucInterfaceNum    = ptIfDesc->bInterfaceNumber;
            ptNotify->ucTotalIfNum      = ucNumIf;
            ptNotify->pucInterfaceDesc  = (grp_u8 *)ptIfDesc;
            ptNotify->pucDevDesc        = ptDevInfo->pucDevDesc;
            ptNotify->pucConfDesc       = ptDevInfo->pucConfDesc;
            ptNotify->pvReference       = l_atRegInfo[iIndex].pvReference;
            /* update counter */
            ptDevInfo->ucMngCnt++;
            /* exit FOR loop */
            break;
        }
    }

    _TRACE_USBC_CNFSFT_(0x15, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_SearchNextIForIAD                                                   */
/*                                                                                              */
/* DESCRIPTION: This function is searched next Interface descriptor or IAD. But, if pointer of  */
/*              the descriptor is Interface of IAD then it is ignored.                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usCnfDescSize                   Reqested size                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _grp_cnfsft_SearchNextIForIAD( grp_u8 **ppucDesc, grp_u16 *pusRemainLength)
{
grp_s32                         lStat       = GRP_CNFSFT_ERROR;
grp_u16                         usCount     = 0;
_grp_cnfsft_wrap_desc           *ptTmpDesc  = (_grp_cnfsft_wrap_desc *)*ppucDesc;

    _TRACE_USBC_CNFSFT_(0x16, 0x00, 0x00);

    while (1) {
        /* get next descriptor */
        usCount   += ptTmpDesc->bLength;
        ptTmpDesc  = (_grp_cnfsft_wrap_desc *)((grp_u8 *)ptTmpDesc + ptTmpDesc->bLength);

        if (usCount >= *pusRemainLength) {
            /* set remain data length */
            *pusRemainLength = 0;
            *ppucDesc         = (grp_u8 *)ptTmpDesc;
            /* exit WHILE loop */
            break;
        }

        /* cgeck descriptor type */
        if (((ptTmpDesc->bDescriptor == GRP_USBD_DESC_INTERFACE)
          && (ptTmpDesc->bOffset3    == 0))                 /* skip alternate setting   */
         || (ptTmpDesc->bDescriptor == GRP_USBD_DESC_INTERFACE_ASS)) {
            /* find next interface or IAD descriptor */
            lStat = GRP_CNFSFT_OK;
            /* set remain data length */
            *pusRemainLength -= usCount;
            *ppucDesc         = (grp_u8 *)ptTmpDesc;
            /* exit WHILE loop */
            break;
        }
    }

    _TRACE_USBC_CNFSFT_(0x16, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_Unregistered                                                        */
/*                                                                                              */
/* DESCRIPTION: No Registration Device in Normal Descriptor                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/*              usEvent                         Connect Event                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_Unregistered( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID, grp_u16 usEvent)
{
grp_cnfsft_notify               tNotify;
grp_s32                         lStat   = GRP_CNFSFT_OK;
grp_u32                         ulAnomCode;

    _TRACE_USBC_CNFSFT_(0x17, 0x00, 0x00);

    /* set parameters */
    switch (usEvent) {
    case GRP_CNFSFT_DEVICE_ATTACHED:
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
        }
        ptDevInfo->ucUnRegFlag = GRP_USB_TRUE;
        ulAnomCode             = GRP_ANOM_UNKNOWN_DEV_CON;
        break;

    case GRP_CNFSFT_DEVICE_DETACHED:
        ptDevInfo->ucUnRegFlag = GRP_USB_FALSE;
        ulAnomCode             = GRP_ANOM_UNKNOWN_DEV_DISCON;
        break;

    default:
        /* illegal error */
        _TRACE_USBC_CNFSFT_(0x17, 0x01, F_END);
        /* DIRECT RETURN */
        return GRP_CNFSFT_ERROR;
    }

    tNotify.usEvent     = usEvent;
    tNotify.usUsbDevId  = usDevID;
    tNotify.pucDevDesc  = ptDevInfo->pucDevDesc;
    tNotify.pucConfDesc = ptDevInfo->pucConfDesc;

    /* Notify to the application */
    _INFO_CNFSFT_(ulAnomCode, &tNotify);

    _TRACE_USBC_CNFSFT_(0x17, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_SetConfiguration                                                    */
/*                                                                                              */
/* DESCRIPTION: Request SET_CONFIGURATION                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_SetConfiguration( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID)
{
grp_usbdi_st_device_request     *ptStDevReq = &l_tInternalInfo.tStDevReq;
grp_usbdi_config_desc_b         *ptConfDesc = (grp_usbdi_config_desc_b *)ptDevInfo->pucConfDesc;
grp_s32                         lStat       = GRP_CNFSFT_OK;

    _TRACE_USBC_CNFSFT_(0x18, 0x00, 0x00);

    /* set parameters */
    ptStDevReq->usUsbDevId      = usDevID;
    ptStDevReq->ucConfiguration = ptConfDesc->bConfigurationValue;
    ptStDevReq->pvDescriptor    = (void *)ptDevInfo->pucConfDesc;
    ptStDevReq->pfnStCbFunc     = _grp_cnfsft_CmpSetConfiguration;

    lStat = grp_usbd_SetConfiguration( ptStDevReq);
    if (lStat == GRP_USBD_OK) {
        /* unlock */
        if (GRP_CNFSFT_ENUM_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
            /* error */
        }
    }
    else {
        /* error */
        lStat = GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x18, 0x00, F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CmpSetConfiguration                                                 */
/*                                                                                              */
/* DESCRIPTION: SET_CONFIGURATION callback function                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptStDevReq                      standard device request structure               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_CmpSetConfiguration( grp_usbdi_st_device_request *ptStDevReq)
{
_grp_cnfsft_dev_info            *ptDevInfo  = GRP_USB_NULL;

    _TRACE_USBC_CNFSFT_(0x19, 0x00, 0x00);

    if (ptStDevReq->lStatus == GRP_USBD_TR_NO_FAIL) {
        /* --- success --- */
        /* Get the management area */
        _grp_cnfsft_DevMngGetBuf( &ptDevInfo, ptStDevReq->usUsbDevId);
        if (!ptDevInfo) {
            /* error */
        }
        else {
            /* lock */
            if (GRP_CNFSFT_INFO_LOCK() != GRP_CNFSFT_LOCK_OK) {
                /* error */
                return GRP_CNFSFT_ERROR;
            }

            /*  */
            _grp_cnfsft_CallbackFunc( ptDevInfo);

            /* unlock */
            if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                /* error */
                return GRP_CNFSFT_ERROR;
            }
        }
    }
    else {
        /* --- error --- */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x19, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CallbackFunc                                                        */
/*                                                                                              */
/* DESCRIPTION: Search Device in Normal Descriptor                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_CallbackFunc( _grp_cnfsft_dev_info *ptDevInfo)
{
grp_cnfsft_registration         *ptRegInfo  = GRP_USB_NULL;
grp_si                          i;

    _TRACE_USBC_CNFSFT_(0x1A, 0x00, 0x00);

    /* check port states */
    if (ptDevInfo->ucConnStat != GRP_CNFSFT_DISCONNECT_STAT) {

        for (i=0; i<ptDevInfo->ucMngCnt; i++) {
            /* set pointer */
            ptRegInfo = &l_atRegInfo[ptDevInfo->atIndex[i].iIndex];

            ptDevInfo->atIndex[i].iNotifyFlag = GRP_CNFSFT_NOTIFIED_UPPER;

            if (ptRegInfo->pfnEventNotification) {
                /* unlock */
                if (GRP_CNFSFT_INFO_UNLOCK() != GRP_CNFSFT_LOCK_OK) {
                    /* error */
                    return GRP_CNFSFT_ERROR;
                }

                /* called callback function */
                (*ptRegInfo->pfnEventNotification)( &ptDevInfo->atIndex[i].tNotify);

                /* lock */
                if (GRP_CNFSFT_INFO_LOCK() != GRP_CNFSFT_LOCK_OK) {
                    /* error */
                    return GRP_CNFSFT_ERROR;
                }
            }
        }
    }

    _TRACE_USBC_CNFSFT_(0x1A, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

#ifdef GRP_USB_USE_MTP
/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetOsStringDesc                                                     */
/*                                                                                              */
/* DESCRIPTION: Executed GET_DESCRIPTOR to get the MS OS String descriptor for MTP              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usCnfDescSize                   Reqested size                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetOsStringDesc( grp_u16 usDevID)
{
grp_usbdi_st_device_request     *ptStDevReq     = &l_tInternalInfo.tStDevReq;
grp_s32                         lStat           = 0;

    _TRACE_USBC_CNFSFT_(0x1B, 0x00, 0x00);

    /* set parameters */
    ptStDevReq->usUsbDevId      = usDevID;
    ptStDevReq->pvDescriptor    = (void *)l_tInternalInfo.ptString;
    ptStDevReq->ulSize          = 0x00000012;           /* MS OS String descriptor size         */
    ptStDevReq->ucIndex         = 0xEE;                 /* MS OS String descriptor ID           */
    ptStDevReq->usLangID        = 0x0000;               /* MS OS String descriptor Language ID  */
    ptStDevReq->pfnStCbFunc     = _grp_cnfsft_CmpGetDeviceDesc;

    /* Get string descriptor */
    lStat = grp_usbd_GetStringDescriptor( ptStDevReq);
    if (lStat != GRP_USBD_OK) {
        _TRACE_USBC_CNFSFT_(0x1B, 0x01, F_END);
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x1B, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_ChkOsStringDesc                                                     */
/*                                                                                              */
/* DESCRIPTION: Check the MS OS String descriptor                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_ChkOsStringDesc(void)
{
_grp_cnfsft_os_string           *ptString   = l_tInternalInfo.ptString;
grp_si                          i;
const grp_u8                    aucSig[]    = {'M',0x00,'S',0x00,'F',0x00,'T',0x00,'1',0x00,'0',0x00,'0',0x00,}; 

    _TRACE_USBC_CNFSFT_(0x1C, 0x00, 0x00);

    if ((ptString->bLength != 0x12) || (ptString->bDescriptorType != 0x03)) {
        _TRACE_USBC_CNFSFT_(0x1C, 0x01, F_END);
        /* invalid informations */
        return GRP_CNFSFT_ERROR;
    }

    for (i=0; i<sizeof(aucSig); i++) {
        if (ptString->qwSignature[i] != aucSig[i]) {
            _TRACE_USBC_CNFSFT_(0x1C, 0x02, F_END);
            /* invalid informations */
            return GRP_CNFSFT_ERROR;
        }
    }

    _TRACE_USBC_CNFSFT_(0x1C, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_GetMtpFeatureDesc                                                   */
/*                                                                                              */
/* DESCRIPTION: Executed GET_DESCRIPTOR to get the MS OS String descriptor for MTP              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*              ucVendorCode                    Vendor code                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_GetMtpFeatureDesc( grp_u16 usDevID, grp_u8 ucVendorCode)
{
grp_usbdi_device_request        *ptDevReq   = &l_tInternalInfo.tDevReq;
grp_s32                         lStat       = 0;

    _TRACE_USBC_CNFSFT_(0x1D, 0x00, 0x00);

    /* set parameters */
    ptDevReq->usUsbDevId    = usDevID;
    ptDevReq->bmRequestType = GRP_USBD_TYPE_DEV2HOST | GRP_USBD_TYPE_VENDOR | GRP_USBD_TYPE_DEVICE;
    ptDevReq->bRequest      = ucVendorCode;
    ptDevReq->wValue        = 0x0000;                   /* UB:Interface number, LB:Page number  */
    ptDevReq->wIndex        = 0x0004;                   /* Feature index                        */
    ptDevReq->wLength       = GRP_CMEM_BSIZE_CNF_MSD;   /* Data length                          */
    ptDevReq->pucBuffer     = l_tInternalInfo.pucMsDesc;
    /* set callback function */
    ptDevReq->pfnDvCbFunc   = _grp_cnfsft_CmpDeviceReq;

    /* Get string descriptor */
    lStat = grp_usbd_DeviceRequest( ptDevReq);
    if (lStat != GRP_USBD_OK) {
        _TRACE_USBC_CNFSFT_(0x1D, 0x01, F_END);
        /* error */
        return GRP_CNFSFT_ERROR;
    }

    _TRACE_USBC_CNFSFT_(0x1D, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_ChkMtpFeatureDesc                                                   */
/*                                                                                              */
/* DESCRIPTION: Check the MS OS String descriptor                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_ChkMtpFeatureDesc(void)
{
grp_si                          i;
grp_u8                          *pucData    = l_tInternalInfo.pucMsDesc;
const grp_u8                    aucSig[]    = {'M', 'T', 'P'};

    _TRACE_USBC_CNFSFT_(0x1E, 0x00, 0x00);

    if ((pucData[6] != 0x04) || (pucData[7] != 0x00)) {
        _TRACE_USBC_CNFSFT_(0x1E, 0x01, F_END);
        /* invalid information */
        return GRP_CNFSFT_ERROR;
    }

    for (i=0; i<sizeof(aucSig); i++) {
        _TRACE_USBC_CNFSFT_(0x1E, 0x02, F_END);
        if (pucData[18+i] != aucSig[i]) {
            /* invalid information */
            return GRP_CNFSFT_ERROR;
        }
    }

    _TRACE_USBC_CNFSFT_(0x1E, 0x00, F_END);

    return GRP_CNFSFT_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_MtpSearch                                                           */
/*                                                                                              */
/* DESCRIPTION: Search MTP Device                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevInfo                       Device informations                             */
/*              usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK           Success                                                 */
/*              GRP_CNFSFT_ERROR        Error                                                   */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cnfsft_MtpSearch( _grp_cnfsft_dev_info *ptDevInfo, grp_u16 usDevID)
{
grp_cnfsft_notify               *ptNotify;
grp_usbdi_config_desc_b         *ptConfDesc;
grp_si                          i;
grp_s32                         lRet        = GRP_CNFSFT_NOMATCH;
grp_s32                         lStat       = GRP_CNFSFT_ERROR;
grp_u16                         usRemainLength;
grp_u8                          *pucTmpDesc = GRP_USB_NULL;

    _TRACE_USBC_CNFSFT_(0x1F, 0x00, 0x00);

    ptConfDesc = (grp_usbdi_config_desc_b *)ptDevInfo->pucConfDesc;

    for (i=0; i<l_tInternalInfo.usRegCnt; i++) {
        /* check of the upper limit */
        if (ptDevInfo->ucMngCnt >= GRP_USBD_HOST_MAX_CLASS) {
            _TRACE_USBC_CNFSFT_(0x1F, 0x01, 0x00);
            lRet = GRP_CNFSFT_NOMATCH;
            break;
        }
        
        if(l_atRegInfo[i].usLoadOption == GRP_CNFSFT_MTP_DEV_SPECIFIED) {
            /* corresponded device */
            lRet = GRP_CNFSFT_MATCH;
            /* save informations and notice to upper layar */
            ptDevInfo->atIndex[ptDevInfo->ucMngCnt].iIndex = i;
            /* set parameters */
            ptNotify = &ptDevInfo->atIndex[ptDevInfo->ucMngCnt].tNotify;
            ptNotify->usEvent           = GRP_CNFSFT_DEVICE_ATTACHED;
            ptNotify->usUsbDevId        = usDevID;
            ptNotify->usConfigIdx       = ptDevInfo->ucCnfIdx;
            ptNotify->ucInterfaceNum    = 0;
            ptNotify->ucTotalIfNum      = ptConfDesc->bNumInterface;
            /*--- search next interface descriptor ---*/
            pucTmpDesc      = ptDevInfo->pucConfDesc;
            usRemainLength  = _GRP_CNFSFT_CRE_UINT16( ptConfDesc->wTotalLength_High,
                                                      ptConfDesc->wTotalLength_Low);
            _grp_cnfsft_SearchNextIForIAD( &pucTmpDesc, &usRemainLength);
            /*--- end ---*/
            ptNotify->pucInterfaceDesc  = pucTmpDesc;
            ptNotify->pucDevDesc        = ptDevInfo->pucDevDesc;
            ptNotify->pucConfDesc       = ptDevInfo->pucConfDesc;
            ptNotify->pvReference       = l_atRegInfo[i].pvReference;

            /* update counter */
            ptDevInfo->ucMngCnt++;
            break;
        }
    }

    if (lRet == GRP_CNFSFT_MATCH) {
        /* request SET_CONFIGURATION */
        lStat = _grp_cnfsft_SetConfiguration( ptDevInfo, usDevID);
    }
    else {
        /* not corresponded */
        ptDevInfo->ucCnfIdx = 0;
        /* Notify to the application */
        lStat = _grp_cnfsft_Unregistered( ptDevInfo, usDevID, GRP_CNFSFT_DEVICE_ATTACHED);
    }

    _TRACE_USBC_CNFSFT_(0x1F, 0x00, F_END);

    return lStat;
}
#endif  /* GRP_USB_USE_MTP */
