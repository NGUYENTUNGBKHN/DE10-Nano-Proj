/****************************************************************************/
/*                                                                          */
/*              Copyright(C) 2002-2008 Grape Systems, Inc.                  */
/*                        All Rights Reserved                               */
/*                                                                          */
/* This software is furnished under a license and may be used and copied    */
/* only in accordance with the terms of such license and with the inclusion */
/* of the above copyright notice. No title to and ownership of the software */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* the performance of this computer program, and specifically disclaims any */
/* responsibility for any damages, special or consequential, connected with */
/* the use of this program.                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILE NAME                                                    VERSION     */
/*                                                                          */
/*      peri_ctl.c                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      Processing to the device request of control transmission            */
/*      is performed.                                                       */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME       DATE        REMARKS                                         */
/*                                                                          */
/*  Y.Sano      2002/12/12  Created initial version 0.00                    */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              modified following functions                */
/*                              - _RcvClearFeature                          */
/*                              - _RcvGetDescriptor                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              Add logging routine.                        */
/*  K.Handa     2005/03/18  V 1.11                                          */
/*                              below descriptors supported.                */
/*                              - Device Qualifier Descriptor               */
/*                              - Other Speed Configuration Descriptor      */
/*  K.Handa     2005/03/18  V 1.12                                          */
/*                              for override maxpower                       */
/*  K.Handa     2005/04/08  V 1.13                                          */
/*                              for supporting test mode                    */
/*  K.Handa     2005/11/10  V 1.14                                          */
/*                              bugfix for no process on non-OTG            */
/*  K.Handa     2006/07/27  V 1.16                                          */
/*                              modified alignment in stack variable        */
/*  K.Handa     2006/12/27  V 1.20                                          */
/*                              version was updated.                        */
/*  S.Tomizawa  2007/01/24  V 1.30                                          */
/*                              to support MTP,                             */
/*                              - added some parameters                     */
/*                              - modified code that process a received     */
/*                                vender request                            */
/*                              - added a function for sending OS descriptor*/
/*  K.Handa     2007/03/12  V 1.40                                          */
/*                              added selector for fixed full-speed.        */
/*  K.Handa     2007/06/12  V 1.41                                          */
/*                              bmAttributes update timing was changed.     */
/*  K.Handa     2008/01/21  V 1.42                                          */
/*                              change of the condition for 0-len addition  */
/*                              on reply to device request.                 */
/****************************************************************************/

/**** INCLUDE FILES *********************************************************/
#include    <string.h>
#include    "perid.h"
#include    "peri_hal.h"
#include    "peri_sts.h"
#include    "peri_trs.h"
#include    "peri_ctl.h"

#ifdef GRUSB_COMMON_USE_OTG
#include    "otgd.h"
#endif

/* Logging routine */
#ifdef GRUSB_CTL_DEBUG
#include    "dbg_mdl.h"
#define     GRCTL_DBG_TRACE2(m,n,x,y)    GRDBG_TRACE2(m,n,x,y)
#else
#define     GRCTL_DBG_TRACE2(m,n,x,y)
#endif

EXTERN  UINT8   l_ucBusSpd2; /* bus speed (0:full/1:high) from peri_prm.c    */

/**** INTERNAL DATA DEFINES *************************************************/
typedef enum {
    TRSSTS_Idle = 0,                                                /* Idle */
    TRSSTS_Trs                                                  /* Sending  */
} TRANS_STATUS2;                                 /* Transmission state flag  */

DLOCAL  TRANS_STATUS2           l_eCtrSts2 = TRSSTS_Idle;
DLOCAL  UINT8                   l_ucCfgValue2;           /* Set_Config flag  */
DLOCAL  UINT8                   l_aucDeviceState2[2];    /* Device State     */
DLOCAL  UINT8                   l_ucNumOfInterface2;     /* Interface number */
DLOCAL  UINT16                  l_usCodeMap2;            /* notice to receive*/
DLOCAL  UINT8                   l_ucEnbRmWkup2 = 0;      /*  */
DLOCAL  GRUSB_DeviceRequest     l_pfnDevReqInd2;
/* Callback functions */
DLOCAL  GRUSB_NoticeEndPoint    l_pfnSndCnclFunc2;
DLOCAL  GRUSB_NoticeEndPoint    l_pfnRcvCnclFunc2;
DLOCAL  GRUSB_NoticeEndPoint16  l_pfnCbClearFeature2;
/* Remote Wakeup functional effective notice function   */
DLOCAL  GRUSB_Notice16          l_pfnCbEnRmtWkup2;

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
EXTERN BOOLEAN g_bMTPSupport2;
DLOCAL const union {
    UINT8    uacData[0x28];
    UINT32   ulDummy;       /* for 32bits alignment */
} l_uMTPFeatureDesc2 = {
    0x28, 0x00, 0x00, 0x00,                             /* dwLength              */
    0x00, 0x01,                                         /* bcdVersion            */
    0x04, 0x00,                                         /* wIndex                */
    0x01,                                               /* bCount                */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,           /* RESERVED              */
    0x00,                                               /* bFirstInterfaceNumber */
    0x01,                                               /* bInterfaceCount       */
    0x4D, 0x54, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,     /* compatibleID          */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* subCompatibleID       */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00                  /* RESERVED              */
};
#endif

#if 1  /* HID */
EXTERN BOOLEAN g_bHIDSupport2;
#endif /* HID */


/**** INTERNAL PROTOTYPES ***************************************************/
LOCAL   VOID    _RcvGetStatus2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvClearFeature2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetFeature2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetAddress2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvGetDescriptor2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetDescriptor2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvGetConfiguration2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetConfiguration2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvGetInterface2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetInterface2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSynchFrame2( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvAplDeviceRequest2( DEVICE_REQUEST_INFO *pucDeviceRequest );
LOCAL   VOID    _setSndData2( GRUSB_EndPointInfo* ptEPInfo,UINT16 usLength,INT i0Len,UINT8 *pucResData );    /* 1.42 */
LOCAL   VOID    _CtrlCallRcvCancelFunc2( VOID );
LOCAL   VOID    _CtrlCallSndCancelFunc2( VOID );
/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
LOCAL   VOID    _RcvGetMTPFeatureDescriptor2( DEVICE_REQUEST_INFO* );
#endif

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlInit2                                         */
/*                                                                          */
/* DESCRIPTION: Initialization in the Control transmission Module           */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 01                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : l_pfnDevReqInd2      function of DeviceRequest reception    */
/*              l_pfnSndCnclFunc2    function of transmitting Cancel        */
/*              l_pfnRcvCnclFunc2    function of receiving Cancel           */
/*              l_pfnCbClearFeature2 function of CreateFeature reception    */
/*              l_pfnCbEnRmtWkup2    function of RemoteWakeup effective     */
/*              l_usCodeMap2         code map                               */
/*              l_ucCfgValue2        configuration value                    */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlInit2(VOID)
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x01, 0, 0 );

    /* initialize of parameters */
    l_pfnDevReqInd2      = GRUSB_NULL;
    l_pfnSndCnclFunc2    = GRUSB_NULL;
    l_pfnRcvCnclFunc2    = GRUSB_NULL;
    l_pfnCbClearFeature2 = GRUSB_NULL;
    l_pfnCbEnRmtWkup2    = GRUSB_NULL;
    l_usCodeMap2         = 0;
    l_ucCfgValue2        = 0;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x01, 0, END_FUNC );
}
/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlReInit2                                       */
/*                                                                          */
/* DESCRIPTION: Re-initialization in the Control transmission Module        */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 14                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : l_ucCfgValue2        configuration valu                     */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlReInit2(VOID)
{
    UINT8*                  pucDesc;        /* pointer of descriptor        */  /* 1.41 */
    UINT16                  usCnfgSz;       /* ConfigurationDescriptor Size */  /* 1.41 */
    GRUSB_DEV_CONFIG_DESC*  ptCnfgDesc;     /* ConfigurationDescriptor      */  /* 1.41 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x14, 0, 0 );

    l_ucCfgValue2 = 0;

    /* Configuration descriptor information acquisition processing */           /* 1.41 */
    pucDesc = GRUSB_Prm_GetConfigurationDescriptor2(l_ucCfgValue2, &usCnfgSz);    /* 1.41 */
    pucDesc = GRUSB_Prm_GetConfigurationDescriptorOverride2(pucDesc, usCnfgSz);  /* 1.41 */
    ptCnfgDesc = (GRUSB_DEV_CONFIG_DESC*)pucDesc;                               /* 1.41 */
                                                                                /* 1.41 */
    /* Initialize Device State */                                               /* 1.41 */
    l_aucDeviceState2[0] = 0x0;                                                  /* 1.41 */
    l_aucDeviceState2[1] = 0x0;                                                  /* 1.41 */
    if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_SELF_POWER)                     /* 1.41 */
    {                                                                           /* 1.41 */
        l_aucDeviceState2[0] |= 0x01;                                            /* 1.41 */
    }                                                                           /* 1.41 */
                                                                                /* 1.41 */
    /* Initialize Remote Wakeup */                                              /* 1.41 */
    if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_RM_WAKEUP)                      /* 1.41 */
    {                                                                           /* 1.41 */
        l_ucEnbRmWkup2 |= 0x02;                                                  /* 1.41 */
    }                                                                           /* 1.41 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x14, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlSetCallBack2                                  */
/*                                                                          */
/* DESCRIPTION: CallBack functiona are registered.                          */
/*                                                                          */
/* Func Code  : 02                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : usReqCodeMap        code map                                */
/*              pfnFunc             function of DeviceRequest reception     */
/*              pfnSndCnclFunc      function of transmitting Cancel         */
/*              pfnRcvCnclFunc      function of receiving Cancel            */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : l_pfnDevReqInd2      function of DeviceRequest reception    */
/*              l_pfnSndCnclFunc2    function of transmitting Cancel        */
/*              l_pfnRcvCnclFunc2    function of receiving Cancel           */
/*              l_usCodeMap2         code map                               */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlSetCallBack2( UINT16                  usReqCodeMap,
                                GRUSB_DeviceRequest     pfnFunc,
                                GRUSB_NoticeEndPoint    pfnSndCnclFunc,
                                GRUSB_NoticeEndPoint    pfnRcvCnclFunc)
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x02, 0, 0 );

    /* set parameters */
    l_pfnDevReqInd2   = pfnFunc;
    l_pfnSndCnclFunc2 = pfnSndCnclFunc;
    l_pfnRcvCnclFunc2 = pfnRcvCnclFunc;
    l_usCodeMap2      = usReqCodeMap;

    /* set callback function to part of HAL */
    GRUSB_DEV_HALCallbackCtrTransferCancel2( _CtrlCallSndCancelFunc2);
    GRUSB_DEV_HALCallbackCtrReceiveCancel2( _CtrlCallRcvCancelFunc2);

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x02, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlRcvReq2                                       */
/*                                                                          */
/* DESCRIPTION: DeviceRequest which received is analyzed.                   */
/*                                                                          */
/* Func Code  : 03                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo        received DeviceRequest                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_usCodeMap2         code map                               */
/*              l_pfnDevReqInd2      function of DeviceRequest reception    */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlRcvReq2( DEVICE_REQUEST_INFO*     ptDevReqInfo)
{
    UINT16      usCallBack;
    UINT8       ucRequestType;
    UINT8       ucRequest;
#ifdef GRUSB_CTL_DEBUG
    UINT16      usValue;
    UINT16      usIndex;
    UINT16      usLength;
#endif

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x03, 0, 0 );

    ucRequestType = ptDevReqInfo->ucRequestType;
    ucRequest     = ptDevReqInfo->ucRequest;
#ifdef GRUSB_CTL_DEBUG
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;
#endif

    l_eCtrSts2 = TRSSTS_Idle;

    if ((ucRequestType & GRUSB_DEV_REQUEST_TYPE) == GRUSB_DEV_STANDARD_TYPE)
    {
        usCallBack = (UINT16)(l_usCodeMap2 & (0x01 << ((INT)(ucRequest))));
        if (usCallBack != 0x00)         /* The request notified to a higher rank */
        {
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x03, 0x01, END_FUNC );

            /* Notice call-back of a device request */
            _RcvAplDeviceRequest2( ptDevReqInfo);
            return;
        }

        l_eCtrSts2 = TRSSTS_Trs;

        switch (ucRequest)           /*--- DeviceRequest judging ---*/
        {
        case GRUSB_DEV_GET_STATUS:              /* GET_STATUS       */
            _RcvGetStatus2( ptDevReqInfo);
            break;

        case GRUSB_DEV_CLEAR_FEATURE:           /* CLEAR_FEATUR     */
            _RcvClearFeature2( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_FEATURE:             /* SET_FEATURE      */
            _RcvSetFeature2( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_ADDRESS:             /* SET_ADDRESS      */
            _RcvSetAddress2( ptDevReqInfo);
            break;

        case GRUSB_DEV_GET_DESCRIPTOR:          /* GET_DESCRIPTOR   */
            _RcvGetDescriptor2( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_DESCRIPTOR:          /* SET_DESCRIPTOR   */
            _RcvSetDescriptor2( ptDevReqInfo);
            break;

        case GRUSB_DEV_GET_CONFIGURATION:       /* GET_CONFIGURATION */
            _RcvGetConfiguration2( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_CONFIGURATION:       /* SET_CONFIGURATION */
            _RcvSetConfiguration2( ptDevReqInfo);
            break;

        case GRUSB_DEV_GET_INTERFACE:           /* GET_INTERFACE    */
            _RcvGetInterface2( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_INTERFACE:           /* SET_INTERFACE    */
            _RcvSetInterface2( ptDevReqInfo);
            break;

        case GRUSB_DEV_SYNCH_FRAME:             /* SYNCH_FRAME      */
            _RcvSynchFrame2( ptDevReqInfo);
            break;

        default:                                /* except the above */
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x03, 0x01, 0 );

            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
            break;
        }
    }
    else
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x03, 0x02, 0 );

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
        if( g_bMTPSupport2 )
        {
            if( ( ucRequestType == 0xC0 ) && ( ucRequest == GRUSB_MTP_MS_VENDORCODE ) )
            {
                _RcvGetMTPFeatureDescriptor2( ptDevReqInfo );
            }
            else
            {
                _RcvAplDeviceRequest2( ptDevReqInfo);
            }
        }
        else
        {
            _RcvAplDeviceRequest2( ptDevReqInfo);
        }
#else
        _RcvAplDeviceRequest2( ptDevReqInfo);
#endif
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x03, 0x02, END_FUNC );

    return;
}

/****************************************************************************/
/* FUNCTION   : _CtrlCallSndCancelFunc2                                     */
/*                                                                          */
/* DESCRIPTION: The higher rank Layer function which notifies               */
/*              Transmission Cancel is called.                              */
/*                                                                          */
/* Func Code  : 04                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_pfnSndCnclFunc2    function of transmitting Cancel        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _CtrlCallSndCancelFunc2(VOID)
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x04, 0, 0 );

    if ((l_pfnSndCnclFunc2 != GRUSB_NULL) && (l_eCtrSts2 == TRSSTS_Idle))
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x04, 0x01, 0 );

        (*l_pfnSndCnclFunc2)( GRUSB_DEV_EP0);
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x04, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _CtrlCallRcvCancelFunc2                                     */
/*                                                                          */
/* DESCRIPTION: The higher rank Layer function which notifies               */
/*              Reception Cancel is called.                                 */
/*                                                                          */
/* Func Code  : 05                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_pfnRcvCnclFunc2    function of receiving Cancel            */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL   VOID    _CtrlCallRcvCancelFunc2( VOID )
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x05, 0, 0 );

    if ((l_pfnRcvCnclFunc2 != GRUSB_NULL) && (l_eCtrSts2 == TRSSTS_Idle))
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x05, 0x01, 0 );

        (*l_pfnRcvCnclFunc2)( GRUSB_DEV_EP0);
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x05, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvGetStatus2                                              */
/*                                                                          */
/* DESCRIPTION: Processing at the time of GET_STATUS reception is performed.*/
/*                                                                          */
/* Func Code  : 06                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : *pucDeviceRequest   received DeviceRequest                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvGetStatus2( DEVICE_REQUEST_INFO*      ptDevReqInfo)
{
    INT                     iCrtState;          /* current state                    */
    INT                     iEpNo;              /* endpoint number                  */
    GRUSB_EndPointInfo*     ptEpInfo;           /* pointer of endpoint information  */
    UINT8                   ucRequestType;      /* bmRequestType                    */
#ifdef GRUSB_CTL_DEBUG
    UINT8                   ucRequest;          /* bRequest                         */
    UINT16                  usValue;            /* wValue                           */
#endif
    UINT16                  usIndex;            /* wIndex                           */
    UINT16                  usLength;           /* wLength                          */
#if 0                                                                                   /* 1.16 */
    DLOCAL      UINT8       aucResDt[2];        /* responce data                    */
#else                                                                                   /* 1.16 */
    DLOCAL      UINT16      ausResDt[1];        /* responce data                    */  /* 1.16 */
    DLOCAL      UINT8*      aucResDt = (UINT8*)&ausResDt[0];                            /* 1.16 */
#endif                                                                                  /* 1.16 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
#endif
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    aucResDt[0] = 0x00;
    aucResDt[1] = 0x00;
    iEpNo       = usIndex & 0x000F;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, usLength, iEpNo );

    /* get device state */
    iCrtState = GRUSB_DEV_ApGetDeviceState2();

    if (ucRequestType == GRUSB_DEVICE_STATUS)               /* GET_DEVICE_STATUS */
    {
        if (iCrtState <= GRUSB_DEV_STATE_DEFAULT)       /* is device state CONFIG? */
        {
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, 0x01, END_FUNC );

            /* set up to Stall state */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }

        /* set data of device state */
        aucResDt[0] = l_aucDeviceState2[0];
        aucResDt[1] = l_aucDeviceState2[1];
    }
    else if (ucRequestType == GRUSB_INTERFACE_STATUS)       /* GET_INTERFACE_STATUS */
    {
        /* check interface number */
        if (usIndex >= l_ucNumOfInterface2)
        {
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, 0x02, END_FUNC );

            /* set up to Stall state */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }
    }
    else if (ucRequestType == GRUSB_ENDPOINT_STATUS)        /* GET_ENDPOINT_STATUS */
    {
        if ((iCrtState == GRUSB_DEV_STATE_CONFIGURED)   /* is device state CONFIG?  */
         && (iEpNo <= GRUSB_DEV_MAX_EP))                /* valid endpoint?          */
        {
            if (GRUSB_DEV_StateGetStall2( iEpNo) != GRUSB_FALSE)
            {                                   /* is an applicable end point STALL? */
                aucResDt[0] = 0x01;
            }
        }
        else if ((iCrtState < GRUSB_DEV_STATE_CONFIGURED)   /* is device state befor CONFIG? */
              && (iEpNo == GRUSB_DEV_EP0))                  /* is specified endpoint 0? */
        {
            if (GRUSB_DEV_StateGetStall2( iEpNo) != GRUSB_FALSE)
            {                                   /* is an applicable end point STALL? */
                aucResDt[0] = 0x01;
            }
        }
        else
        {
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, 0x03, END_FUNC );

            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }
    }
    else
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, 0x04, END_FUNC );

        /* set up to STALL status */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0);

    if (ptEpInfo == GRUSB_NULL)                         /* invalide endpoint */
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, 0x05, END_FUNC );

        /* set up to STALL status */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    _setSndData2( ptEpInfo, usLength, GRUSB_FALSE, aucResDt);    /* 1.42 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x06, 0, END_FUNC );
}


/****************************************************************************/
/* FUNCTION   : _RcvClearFeature2                                           */
/*                                                                          */
/* DESCRIPTION: Processing at the time of CLEAR_FEATURE reception           */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 07                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvClearFeature2( DEVICE_REQUEST_INFO*       ptDevReqInfo)
{
    INT                 iCrtState;          /* current state    */
    INT                 iEpNo;              /* endpoint number  */
    UINT8               ucRequestType;      /* bmRequestType    */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;          /* bRequest         */
#endif
    UINT16              usValue;            /* wValue           */
    UINT16              usIndex;            /* wIndex           */
#ifdef GRUSB_CTL_DEBUG
    UINT16              usLength;           /* wLength          */
#endif

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x07, 0, 0 );

    /* get device status */
    iCrtState     = GRUSB_DEV_ApGetDeviceState2();
    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
#ifdef GRUSB_CTL_DEBUG
    usLength      = ptDevReqInfo->usLength;
#endif
    iEpNo         = (UINT8)(usIndex & 0x0F);

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x07, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x07, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x07, usLength, iEpNo );

    if (ucRequestType == GRUSB_DEVICE_STATUS)       /* DEVICE_CLEAR_FEATURE */
    {
        if (usValue == GRUSB_DEV_DEVICE_REMOTE_WAKEUP)
        {                                       /* release RemoteWakeup functions */
            if (l_ucEnbRmWkup2)
                l_aucDeviceState2[0] &= ~0x02;   /* clear RemoteWakeup bit */

            if (l_pfnCbEnRmtWkup2 != GRUSB_NULL)
                /* call functions if it is registered */
                (*l_pfnCbEnRmtWkup2)(GRUSB_DEV_RmtWkup_DISABLE);
        }
        else if (usValue == GRUSB_DEV_TEST_MODE)
        {                                       /* release TestMode functions */
        }
        else
        {
            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        }
    }
    else if ((ucRequestType == GRUSB_ENDPOINT_STATUS)   /* ENDPOINT_CLEAR_FEATURE   */
          && (iEpNo < GRUSB_DEV_MAX_EP))                /* valid endpoint?          */
    {
        if ((iCrtState != GRUSB_DEV_STATE_CONFIGURED)   /* device state is not CONFIG   */
         || (iEpNo != GRUSB_DEV_EP0))                   /* specified endpoint is not 0  */
        {
            /* toggle clear */
            GRUSB_DEV_HALTogleClear2(iEpNo);
            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState2( iEpNo, GRUSB_FALSE );

            if (l_pfnCbClearFeature2 != GRUSB_NULL)
                l_pfnCbClearFeature2( iEpNo, usValue );
        }
        else
        {
            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        }
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x07, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetFeature2                                             */
/*                                                                          */
/* DESCRIPTION: Processing at the time of SET_FEATURE reception             */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 08                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : *ptDevReqInfo   received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvSetFeature2( DEVICE_REQUEST_INFO*     ptDevReqInfo)
{
    INT                 iCrtState;          /* current state    */
    INT                 iEpNo;              /* endpoint number  */
    UINT8               ucRequestType;      /* bmRequestType    */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;          /* bRequest         */
#endif
    UINT16              usValue;            /* wValue           */
    UINT16              usIndex;            /* wIndex           */
    UINT16              usLength;           /* wLength          */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x08, 0, 0 );

    /* get device state */
    iCrtState     = GRUSB_DEV_ApGetDeviceState2();
    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;
    iEpNo         = (UINT8)(usIndex & 0x0F);

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x08, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x08, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x08, usLength, iEpNo );

    if (usLength != 0)                      /* is length 0? */
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x08, 0x01, END_FUNC );

        /* no -> set up to STALL status */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    if (ucRequestType == GRUSB_DEVICE_STATUS)           /* DEVICE_SET_FEATURE */
    {
        if (usValue == GRUSB_DEV_DEVICE_REMOTE_WAKEUP)  /* effective RemoteWakeup function */
        {
            if (l_ucEnbRmWkup2)
                l_aucDeviceState2[0] |= l_ucEnbRmWkup2;

            if (l_pfnCbEnRmtWkup2 != GRUSB_NULL)
            {
                /* call functions if it is registered */
                (*l_pfnCbEnRmtWkup2)(GRUSB_DEV_RmtWkup_ENABLE);
            }
        }
        else if (usValue == GRUSB_DEV_TEST_MODE)
        {
            if( l_ucBusSpd2 == GRUSB_PRM_BUS_HS )
            {
                /* HiSpeed */
                usIndex = (usIndex >> 8) & 0x00ff;
                GRUSB_DEV_HALSetTestMode2( usIndex );
            }
            else
            {
                /* FullSpeed */
                /* set up to STALL function */
                GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
            }
        }
        else
        {
#ifdef  GRUSB_COMMON_USE_OTG
            /* notifies to OTG module */
            if ((GROTGD_Set_Feature_Notice( usValue)) != GRUSB_TRUE)
            {
                /* set up to STALL function */
                GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
            }
#else   /* GRUSB_COMMON_USE_OTG */ /* V 1.14 */
            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
#endif  /* GRUSB_COMMON_USE_OTG */ /* V 1.14 */
        }
    }
    else if (ucRequestType == GRUSB_INTERFACE_STATUS)
    {                                       /* INTERFACE_SET_FEATURE */
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
    }
    else if ((ucRequestType == GRUSB_ENDPOINT_STATUS)   /* ENDPOINT_SET_FEATURE */
          && (usValue == GRUSB_DEV_ENDPOINT_HALT)       /* halt of endpoint     */
          && (iEpNo < GRUSB_DEV_MAX_EP))                /* valid endpoint?      */
    {
        if ((iCrtState == GRUSB_DEV_STATE_CONFIGURED)   /* device state is CONFIG?  */
         || (iEpNo == GRUSB_DEV_EP0))                   /* specified endpoint is 0? */
        {
            /* set up to STALL function */
            GRUSB_DEV_StateSetStall2( iEpNo, GRUSB_TRUE);
        }
        else
        {
            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        }
    }
    else
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x08, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetAddress2                                             */
/*                                                                          */
/* DESCRIPTION: Processing at the time of SET_ADDRESS reception             */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 09                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvSetAddress2( DEVICE_REQUEST_INFO*     ptDevReqInfo)
{
    UINT8               ucRequestType;      /* bmRequestType    */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;          /* bRequest         */
#endif
    UINT16              usValue;            /* wValue           */
    UINT16              usIndex;            /* wIndex           */
    UINT16              usLength;           /* wLength          */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, usLength, 0 );

    if ((ucRequestType != GRUSB_DEVICE_STATUS)
     || (usIndex != 0)                          /* wIndex must be 0           */
     || (usLength != 0)                         /* wLength must be 0          */
     || (usValue > GRUSB_DEV_MAX_ADDR))         /* wValue must be 127 or less */
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
    }
    else
    {
        /* An address is set up. */
        GRUSB_DEV_HALSetAddress2( usValue);

        /* Device State change */
        GRUSB_DEV_StateSetAddress2( (INT)usValue);
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvGetDescriptor2                                          */
/*                                                                          */
/* DESCRIPTION: Processing at the time of GET_DESCRIPTOR reception          */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0A                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvGetDescriptor2( DEVICE_REQUEST_INFO*      ptDevReqInfo)
{
    UINT16              usActLength;        /* actual lenght                    */
    UINT8*              pucDesc;            /* pointer of descriptor            */
    GRUSB_EndPointInfo  *ptEpInfo;          /* pointer of endpoint information  */
    UINT8               ucRequestType;      /* bmRequestType                    */
    UINT8               ucRequest;          /* bRequest                         */
    UINT16              usValue;            /* wValue                           */
    UINT16              usIndex;            /* wIndex                           */
    UINT16              usLength;           /* wLength                          */
    UINT8               ucDescTyp;          /* descriptor type                  */
    UINT8               ucDescIdx;          /* descriptor index                 */
    INT                 i0Len;              /* 0-Length Addition                */  /* 1.42 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    ucDescTyp     = (UINT8)((usValue>>8) & 0xff);
    ucDescIdx     = (UINT8)((usValue>>0) & 0xff);
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, usLength, 0 );

    if ((ucRequestType != GRUSB_DEVICE_STATUS)
     && (ucRequestType != GRUSB_INTERFACE_STATUS))
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    switch(ucDescTyp)
    {
    case GRUSB_DEV_DEVICE:                      /* GET_DEVICE_DESCRIPTOR */
        pucDesc = GRUSB_Prm_GetDeviceDescriptor2( &usActLength);
        break;

    case GRUSB_DEV_CONFIGURATION:               /* GET_CONFIGURATION_DESCRIPTOR */
        pucDesc = GRUSB_Prm_GetConfigurationDescriptor2( ucDescIdx, &usActLength);
        pucDesc = GRUSB_Prm_GetConfigurationDescriptorOverride2(pucDesc, usActLength);
        break;

    case GRUSB_DEV_STRING:                      /* GET_STRING_DESCRIPTOR */
        pucDesc = GRUSB_Prm_GetStringDescriptor2( ucDescIdx, &usActLength);
        break;

    case GRUSB_DEV_INTERFACE:                   /* GET_INTERFACE_DESCRIPTOR */
    case GRUSB_DEV_ENDPOINT:                    /* GET_ENDPOINT_DESCRIPTOR */
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;

    case GRUSB_DEV_DEVICE_QUALIFIER:            /* GET_DEVICE_QUALIFIER_DESCRIPTOR */
#ifndef GRUSB_DEV_FIXED_FS                                                  /* 1.40 */
        pucDesc = GRUSB_Prm_GetDeviceQualifierDescriptor2( &usActLength);
        break;
#else                                                                       /* 1.40 */
        /* set up to STALL function */                                      /* 1.40 */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );            /* 1.40 */
        return;                                                             /* 1.40 */
#endif                                                                      /* 1.40 */

    case GRUSB_DEV_OTHER_SPEED_CONFIGURATION:   /* GET_OTHER_SPEED_CONFIGURATION_DESCRIPTOR */
#ifndef GRUSB_DEV_FIXED_FS                                                  /* 1.40 */
        pucDesc = GRUSB_Prm_GetOtherSpeedConfigurationDescriptor2( ucDescIdx, &usActLength);
        break;
#else                                                                       /* 1.40 */
        /* set up to STALL function */                                      /* 1.40 */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );            /* 1.40 */
        return;                                                             /* 1.40 */
#endif                                                                      /* 1.40 */
#if 1  /* HID */
    case GRUSB_DEV_REPORT:
        if (g_bHIDSupport2)
        {
            pucDesc = GRUSB_Prm_GetReportDescriptor2( ucDescIdx, &usActLength);
        }
        else
        {
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0x5, END_FUNC );

            if (l_pfnDevReqInd2 != GRUSB_NULL)
                (*l_pfnDevReqInd2)( ptDevReqInfo->ucRequestType, ucRequest, usValue, usIndex, usLength );
            return;
        }
        break;
#endif /* HID */

    default:                                    /* except the above */
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0x2, END_FUNC );

        if (l_pfnDevReqInd2 != GRUSB_NULL)
            (*l_pfnDevReqInd2)( ptDevReqInfo->ucRequestType, ucRequest, usValue, usIndex, usLength );
        return;
    }

    /* check got pointer */
    if (pucDesc == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }
    else
    {
        if (usLength > usActLength)
        {                                                       /* 1.42 */
            usLength = usActLength;
            i0Len    = GRUSB_TRUE;                              /* 1.42 */
        }                                                       /* 1.42 */
        else                                                    /* 1.42 */
        {                                                       /* 1.42 */
            i0Len    = GRUSB_FALSE;                             /* 1.42 */
        }                                                       /* 1.42 */
    }

    /* End point information acquisition */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0 );

    if (ptEpInfo == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0x04, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* data which transmits is set to cyclic buffer. */
    _setSndData2( ptEpInfo, usLength, i0Len, pucDesc);           /* 1.42 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetDescriptor2                                          */
/*                                                                          */
/* DESCRIPTION: Processing at the time of SET_DESCRIPTOR reception          */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0B                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvSetDescriptor2( DEVICE_REQUEST_INFO*      ptDevReqInfo)
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0B, 0, 0 );

    /* set up to STALL function */
    GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0B, 0, END_FUNC );

    return;
}

/****************************************************************************/
/* FUNCTION   : _RcvGetConfiguration2                                       */
/*                                                                          */
/* DESCRIPTION: Processing at the time of GET_CONFIGURATION reception       */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0C                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvGetConfiguration2( DEVICE_REQUEST_INFO*       ptDevReqInfo)
{
    INT                 iCrtState;      /* current state                    */
    GRUSB_EndPointInfo  *ptEpInfo;      /* pointer of endpoint information  */
    UINT8               ucRequestType;  /* bmRequestType                    */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;      /* bRequest                         */
    UINT16              usValue;        /* wValue                           */
    UINT16              usIndex;        /* wIndex                           */
#endif
    UINT16              usLength;       /* wLength                          */
#if 0                                                                           /* 1.16 */
    DLOCAL UINT8        aucResDt[1];    /* responce data                    */
#else                                                                           /* 1.16 */
    DLOCAL UINT16       ausResDt[1];    /* responce data                    */  /* 1.16 */
    DLOCAL UINT8*       aucResDt = (UINT8*)&ausResDt[0];                        /* 1.16 */
#endif                                                                          /* 1.16 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
#endif
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, usLength, 0 );

    if ((ucRequestType != GRUSB_DEVICE_STATUS)
     || (usLength != 1))                    /* is length 1? */
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* get device state */
    iCrtState = GRUSB_DEV_StateGetDeviceState2();

    switch(iCrtState)
    {
    case GRUSB_DEV_STATE_DEFAULT:       /* DEFAULT state */
    case GRUSB_DEV_STATE_ADDRESS:       /* ADDRESS state */
        l_ucCfgValue2 = 0x00;
        break;

    case GRUSB_DEV_STATE_CONFIGURED:    /* CONFIGURED state */
        l_ucCfgValue2 = 0x01;
        break;

    default:
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, 0x02, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0);

    if (ptEpInfo == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* data which transmits is set to cyclic buffer. */
    aucResDt[0] = l_ucCfgValue2;
    _setSndData2( ptEpInfo, usLength, GRUSB_FALSE, aucResDt);    /* 1.42 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0C, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetConfiguration2                                       */
/*                                                                          */
/* DESCRIPTION: Processing at the time of SET_CONFIGURATION reception       */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0D                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvSetConfiguration2( DEVICE_REQUEST_INFO*       ptDevReqInfo)
{
    UINT8*                  pucDesc;            /* pointer of descriptor        */
    UINT16                  usCnfgSz;           /* ConfigurationDescriptor Size */
    GRUSB_DEV_CONFIG_DESC*  ptCnfgDesc;         /* ConfigurationDescriptor      */
    INT                     iCrtState;          /* current state                */
    UINT8                   ucRequestType;      /* bmRequestType                */
#ifdef GRUSB_CTL_DEBUG
    UINT8                   ucRequest;          /* bRequest                     */
#endif
    UINT16                  usValue;            /* wValue                       */
#ifdef GRUSB_CTL_DEBUG
    UINT16                  usIndex;            /* wIndex                       */
    UINT16                  usLength;           /* wLength                      */
#endif

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
#ifdef GRUSB_CTL_DEBUG
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;
#endif

#if 0   /* 1.41 */
    l_aucDeviceState2[0] = 0x0;
    l_aucDeviceState2[1] = 0x0;
#endif  /* 1.41 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, usLength, 0 );

    if (ucRequestType != GRUSB_DEVICE_STATUS)
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* get device state */
    iCrtState = GRUSB_DEV_ApGetDeviceState2();

    switch(iCrtState)
    {
    case GRUSB_DEV_STATE_DEFAULT:       /* DEFAULT state */
        if (usValue == 0x01)            /* after SET_CONFIGURATION? */
        {
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, 0x02, END_FUNC );

            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
            return;
        }
        break;

    case GRUSB_DEV_STATE_ADDRESS:       /* ADDRESS state */
    case GRUSB_DEV_STATE_CONFIGURED:    /* CONFIGURED state */

        l_ucCfgValue2 = (UINT8)usValue;

        GRUSB_DEV_StateSetConfigure2( (INT)usValue);

        if (usValue != 0)
        {
            /* Configuration descriptor information acquisition processing */
            pucDesc = GRUSB_Prm_GetConfigurationDescriptor2( (UINT8)(usValue-1), &usCnfgSz);
            pucDesc = GRUSB_Prm_GetConfigurationDescriptorOverride2(pucDesc, usCnfgSz);
            ptCnfgDesc = (GRUSB_DEV_CONFIG_DESC*)pucDesc;

            if (ptCnfgDesc == GRUSB_NULL)
            {
                GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, 0x02, END_FUNC );

                /* set up to STALL function */
                GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
                return;
            }

            if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_SELF_POWER)
                                                            /* self power */
            {
                l_aucDeviceState2[0] |= 0x01;
            }
            else                                                                /* 1.41 */
            {                                                                   /* 1.41 */
                l_aucDeviceState2[0] &= ~0x01;                                   /* 1.41 */
            }                                                                   /* 1.41 */
            if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_RM_WAKEUP)
                                                            /* Remote Wakeup */
            {
                l_ucEnbRmWkup2 |= 0x02;
            }
            else                                                                /* 1.41 */
            {                                                                   /* 1.41 */
                l_ucEnbRmWkup2 &= ~0x02;                                         /* 1.41 */
            }                                                                   /* 1.41 */

            l_ucNumOfInterface2 = ptCnfgDesc->uc_bNumInterfaces;
        }

        break;

    default:                            /* except the above */
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0D, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvGetInterface2                                           */
/*                                                                          */
/* DESCRIPTION: Processing at the time of GET_INTERFACE reception           */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0E                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvGetInterface2( DEVICE_REQUEST_INFO*       ptDevReqInfo )
{
    INT                 iCrtState;          /* current state                    */
    INT                 iStat;              /* return value                     */
    GRUSB_EndPointInfo  *ptEpInfo;          /* pointer of endpoint information  */
    UINT8               ucRequestType;      /* bmRequestType                    */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;          /* bRequest                         */
    UINT16              usValue;            /* wValue                           */
#endif
    UINT16              usIndex;            /* wIndex                           */
    UINT16              usLength;           /* wLength                          */
    UINT8               ucInterface;        /* Interface number                 */
#if 0                                                                               /* 1.16 */
    DLOCAL UINT8        aucResDt[1];        /* responce data                    */
#else                                                                               /* 1.16 */
    DLOCAL UINT16       ausResDt[1];        /* responce data                    */  /* 1.16 */
    DLOCAL UINT8*       aucResDt = (UINT8*)&ausResDt[0];                            /* 1.16 */
#endif                                                                              /* 1.16 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
#endif
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, usLength, 0 );

    if ((ucRequestType != GRUSB_INTERFACE_STATUS)
     || (usLength != 1))                        /* is length 1? */
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* get device state */
    iCrtState = GRUSB_DEV_ApGetDeviceState2();

    aucResDt[0] = 0x00;

    switch( iCrtState )
    {
    case GRUSB_DEV_STATE_CONFIGURED:        /* CONFIGURED state */
        /* An alternative setting value is substituted for ucSndData. */
        iStat = GRUSB_Prm_GetInterface2( usIndex, &ucInterface);
        if (iStat != GRUSB_TRUE)
        {
            GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, 0x02, END_FUNC );

            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }
        aucResDt[0] = ucInterface;
        break;

    case GRUSB_DEV_STATE_DEFAULT:           /* DEFAULT state */
    case GRUSB_DEV_STATE_ADDRESS:           /* ADDRESS state */
    default:
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0);

    if (ptEpInfo == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, 0x04, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* data which transmits is set to cyclic buffer. */
    _setSndData2( ptEpInfo, usLength, GRUSB_FALSE, aucResDt);    /* 1.42 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0E, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetInterface2                                           */
/*                                                                          */
/* DESCRIPTION: Processing at the time of GET_INTERFACE reception           */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0F                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvSetInterface2( DEVICE_REQUEST_INFO*       ptDevReqInfo)
{
    INT                 iCrtState;          /* current state    */
    INT                 iStat;              /* return value     */
    UINT8               ucRequestType;      /* bmRequestType    */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;          /* bRequest         */
#endif
    UINT16              usValue;            /* wValue           */
    UINT16              usIndex;            /* wIndex           */
    UINT16              usLength;           /* wLength          */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0F, 0, 0 );

    /* get device state */
    iCrtState     = GRUSB_DEV_ApGetDeviceState2();
    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0F, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0F, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0F, usLength, 0 );

    if ((ucRequestType != GRUSB_INTERFACE_STATUS)
     || (usLength != 0)                             /* is length 0?             */
     || (iCrtState != GRUSB_DEV_STATE_CONFIGURED))  /* except CONFIGURED state  */
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0F, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* Alternative setting value registration */
    iStat = GRUSB_Prm_SetInterface2( usIndex, usValue);
    if (iStat != GRUSB_TRUE)
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0F, 0x02, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0F, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSynchFrame2                                             */
/*                                                                          */
/* DESCRIPTION: Processing at the time of SYNCH_FRAME reception             */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 10                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvSynchFrame2( DEVICE_REQUEST_INFO*     ptDevReqInfo)
{
    UINT8               iEpNo;
    UINT8               ucRequestType;      /* bmRequestType */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;          /* bRequest */
    UINT16              usValue;            /* wValue */
#endif
    UINT16              usIndex;            /* wIndex */
#ifdef GRUSB_CTL_DEBUG
    UINT16              usLength;           /* wLength */
#endif

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x10, 0, 0 );

    /* get device state */
    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
#endif
    usIndex       = ptDevReqInfo->usIndex;
#ifdef GRUSB_CTL_DEBUG
    usLength      = ptDevReqInfo->usLength;
#endif
    iEpNo         = (UINT8)(usIndex & 0x0F);

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x10, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x10, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x10, usLength, iEpNo );

    if ((ucRequestType != GRUSB_ENDPOINT_STATUS)
     || (iEpNo > GRUSB_DEV_MAX_EP))
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE);
    }
    else
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( (INT)usIndex, GRUSB_TRUE);
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x10, 0, END_FUNC );

    return;
}

/****************************************************************************/
/* FUNCTION   : _RcvAplDeviceRequest2                                       */
/*                                                                          */
/* DESCRIPTION: The higher rank Layer function which notifies a device      */
/*              request is called.                                          */
/*                                                                          */
/* Func Code  : 11                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvAplDeviceRequest2( DEVICE_REQUEST_INFO*       ptDevReqInfo)
{
    UINT8           ucRequestType;      /* bmRequestType    */
    UINT8           ucRequest;          /* bRequest         */
    UINT16          usValue;            /* wValue           */
    UINT16          usIndex;            /* wIndex           */
    UINT16          usLength;           /* wLength          */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x11, 0, 0 );

    /* get device state */
    ucRequestType = ptDevReqInfo->ucRequestType;
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x11, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x11, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x11, usLength, 0 );

    if (l_pfnDevReqInd2 != GRUSB_NULL)
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x11, 0x01, 0 );

        (*l_pfnDevReqInd2)( ucRequestType, ucRequest, usValue, usIndex, usLength);
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x11, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlCallbackClearFeature2                         */
/*                                                                          */
/* DESCRIPTION: The callback function called when ClearFeature is detected  */
/*              is registered.                                              */
/* Func Code  : 12                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : pFunc           pointer of Callback function                */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlCallbackClearFeature2( GRUSB_NoticeEndPoint16     pFunc)
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x12, 0, 0 );

    l_pfnCbClearFeature2 = pFunc;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x12, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _setSndData2                                                 */
/*                                                                          */
/* DESCRIPTION: Data which transmits is registered into Queue.              */
/*                                                                          */
/* Func Code  : 13                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usLength        send data length                            */
/*              pucResData      send data                                   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _setSndData2( GRUSB_EndPointInfo*       ptEpInfo,
                  UINT16                    usLength,
                  INT                       i0Len,              /* 1.42 */
                  UINT8*                    pucResData)
{
    INT                     iWrtPnt;
    GRUSB_SndDataInfo       *ptSndDtInfo;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x13, 0, 0 );

    /* data which transmits is set to cyclic buffer. */
    iWrtPnt = GRLIB_Cyclic_CheckWrite2( &(ptEpInfo->tCycBufInfo) );
    if ( iWrtPnt == GRLIB_NONBUFFER )
    {
        GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x13, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* set parameters */
    ptEpInfo->ucEpType    = EPTYPE_IN | EPTYPE_CONTROL;
    ptSndDtInfo           = &(ptEpInfo->ptTrnsInfo[iWrtPnt].tSndDtInfo);
    ptSndDtInfo->ulDataSz = usLength;
    ptSndDtInfo->pucBuf   = pucResData;
    ptSndDtInfo->i0Len    = i0Len;                              /* 1.42 */
    ptSndDtInfo->pAplInfo = GRUSB_NULL;
    ptSndDtInfo->pfnFunc  = GRUSB_NULL;

    GRLIB_Cyclic_IncWrite2( &(ptEpInfo->tCycBufInfo));

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x13, 0, END_FUNC );
}

/********************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlCallbackEnRmtWkup2                                */
/*                                                                              */
/* DESCRIPTION: The callback function called when a remote Wakeup function      */
/*              becomes effective is registered.                                */
/*                                                                              */
/* Func Code  : 15                                                              */
/*------------------------------------------------------------------------------*/
/* INPUT      : pFunc           pointer of Callback function                    */
/* OUTPUT     : none                                                            */
/*                                                                              */
/* RESULTS    : none                                                            */
/*                                                                              */
/*------------------------------------------------------------------------------*/
/* REFER      : none                                                            */
/* MODIFY     : none                                                            */
/*                                                                              */
/********************************************************************************/
VOID GRUSB_DEV_CtrlCallbackEnRmtWkup2(GRUSB_Notice16         pFunc)
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x15, 0, 0 );

    l_pfnCbEnRmtWkup2 = pFunc;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x15, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlRmtWkup2                                      */
/*                                                                          */
/* DESCRIPTION: A Remote Wakeup is required.                                */
/*                                                                          */
/* Func Code  : 16                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlRmtWkup2(VOID)
{
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x16, 0, 0 );

    /* Execution of a remote Wakeup */
    GRUSB_DEV_HALRmtWkup2();

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x16, 0, END_FUNC );
}

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
/****************************************************************************/
/* FUNCTION   : _RcvGetMTPFeatureDescriptor2                                */
/*              2007/01/24 [1.30] Added for MTP                             */
/*                                                                          */
/* DESCRIPTION: send OS feature Descriptor data to USB-HOST                 */
/*                                                                          */
/* Func Code  :                                                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : DEVICE_REQUEST_INFO* ptDevReqInfo                           */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvGetMTPFeatureDescriptor2( DEVICE_REQUEST_INFO* ptDevReqInfo )
{
    UINT16              usLength;
    GRUSB_EndPointInfo  *ptEpInfo;
    INT                 i0Len;              /* 0-Length Addition            */  /* 1.42 */

    usLength = ptDevReqInfo->usLength;
    if(usLength > sizeof(l_uMTPFeatureDesc2))
    {
        usLength = sizeof(l_uMTPFeatureDesc2);
        i0Len    = GRUSB_TRUE;                              /* 1.42 */
    }
    else                                                    /* 1.42 */
    {                                                       /* 1.42 */
        i0Len    = GRUSB_FALSE;                             /* 1.42 */
    }                                                       /* 1.42 */

    /* End point information acquisition */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0 );

    /* data which transmits is set to cyclic buffer. */
    _setSndData2( ptEpInfo, usLength, i0Len, (UINT8*)&l_uMTPFeatureDesc2.uacData[0] );    /* 1.42 */
}
#endif

//#if !defined(_ORG_GR_SRC)
#if 0
/****************************************************************************/
/* FUNCTION   : _RcvHIDSetReport2                                             */
/*              2022/06/01 [] Added for HID                                 */
/*                                                                          */
/* DESCRIPTION: Processing at the time of SET_IDLE reception                */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0A                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvHIDSetReport2( DEVICE_REQUEST_INFO* ptDevReqInfo )
{
    GRUSB_EndPointInfo  *ptEpInfo;          /* pointer of endpoint information  */
    UINT8               ucRequestType;      /* bmRequestType    */
    UINT8               ucRequest;          /* bRequest         */
    UINT16              usValue;            /* wValue           */
    UINT16              usIndex;            /* wIndex           */
    UINT16              usLength;           /* wLength          */
    DLOCAL   UINT16     ausResDt[1];        /* responce data                    */  /* 1.16 */
    DLOCAL   UINT8*     aucResDt = (UINT8*)&ausResDt[0];                            /* 1.16 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, usLength, 0 );

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvHIDSetIdle2                                             */
/*              2022/06/01 [] Added for HID                                 */
/*                                                                          */
/* DESCRIPTION: Processing at the time of SET_IDLE reception                */
/*              is performed.                                               */
/*                                                                          */
/* Func Code  : 0A                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptDevReqInfo    received DeviceRequest                      */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _RcvHIDSetIdle2( DEVICE_REQUEST_INFO* ptDevReqInfo )
{
    GRUSB_EndPointInfo  *ptEpInfo;          /* pointer of endpoint information  */
    UINT8               ucRequestType;      /* bmRequestType    */
    UINT8               ucRequest;          /* bRequest         */
    UINT16              usValue;            /* wValue           */
    UINT16              usIndex;            /* wIndex           */
    UINT16              usLength;           /* wLength          */
    DLOCAL   UINT16     ausResDt[1];        /* responce data                    */  /* 1.16 */
    DLOCAL   UINT8*     aucResDt = (UINT8*)&ausResDt[0];                            /* 1.16 */

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, usValue, usIndex );
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, usLength, 0 );

#if 1
    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x0A, 0, END_FUNC );
#else
    if ((ucRequestType != GRUSB_DEVICE_STATUS)
     || (usIndex != 0)                          /* wIndex must be 0           */
     || (usLength != 0)                         /* wLength must be 0          */
     || (usValue > GRUSB_DEV_MAX_ADDR))         /* wValue must be 127 or less */
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
    }
    else
    {
        /* An address is set up. */
        GRUSB_DEV_HALSetAddress2( usValue);

        /* Device State change */
        GRUSB_DEV_StateSetAddress2( (INT)usValue);
    }

    GRCTL_DBG_TRACE2( GRDBG_PERI_CTL, 0x09, 0, END_FUNC );
#endif
}
#endif /* Changed for ID-0G8 */
