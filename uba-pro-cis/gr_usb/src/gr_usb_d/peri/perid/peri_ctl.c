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
#define     GRCTL_DBG_TRACE(m,n,x,y)    GRDBG_TRACE(m,n,x,y)
#else
#define     GRCTL_DBG_TRACE(m,n,x,y)
#endif

EXTERN  UINT8   l_ucBusSpd; /* bus speed (0:full/1:high) from peri_prm.c    */

/**** INTERNAL DATA DEFINES *************************************************/
typedef enum {
    TRSSTS_Idle = 0,                                                /* Idle */
    TRSSTS_Trs                                                  /* Sending  */
} TRANS_STATUS;                                 /* Transmission state flag  */

DLOCAL  TRANS_STATUS            l_eCtrSts = TRSSTS_Idle;
DLOCAL  UINT8                   l_ucCfgValue;           /* Set_Config flag  */
DLOCAL  UINT8                   l_aucDeviceState[2];    /* Device State     */
DLOCAL  UINT8                   l_ucNumOfInterface;     /* Interface number */
DLOCAL  UINT16                  l_usCodeMap;            /* notice to receive*/
DLOCAL  UINT8                   l_ucEnbRmWkup = 0;      /*  */
DLOCAL  GRUSB_DeviceRequest     l_pfnDevReqInd;
/* Callback functions */
DLOCAL  GRUSB_NoticeEndPoint    l_pfnSndCnclFunc;
DLOCAL  GRUSB_NoticeEndPoint    l_pfnRcvCnclFunc;
DLOCAL  GRUSB_NoticeEndPoint16  l_pfnCbClearFeature;
/* Remote Wakeup functional effective notice function   */
DLOCAL  GRUSB_Notice16          l_pfnCbEnRmtWkup;

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
EXTERN BOOLEAN g_bMTPSupport;
DLOCAL const union {
    UINT8    uacData[0x28];
    UINT32   ulDummy;       /* for 32bits alignment */
} l_uMTPFeatureDesc = {
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

/**** INTERNAL PROTOTYPES ***************************************************/
LOCAL   VOID    _RcvGetStatus( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvClearFeature( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetFeature( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetAddress( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvGetDescriptor( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetDescriptor( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvGetConfiguration( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetConfiguration( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvGetInterface( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSetInterface( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvSynchFrame( DEVICE_REQUEST_INFO* );
LOCAL   VOID    _RcvAplDeviceRequest( DEVICE_REQUEST_INFO *pucDeviceRequest );
LOCAL   VOID    _setSndData( GRUSB_EndPointInfo* ptEPInfo,UINT16 usLength,INT i0Len,UINT8 *pucResData );    /* 1.42 */
LOCAL   VOID    _CtrlCallRcvCancelFunc( VOID );
LOCAL   VOID    _CtrlCallSndCancelFunc( VOID );
/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
LOCAL   VOID    _RcvGetMTPFeatureDescriptor( DEVICE_REQUEST_INFO* );
#endif

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlInit                                          */
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
/* MODIFY     : l_pfnDevReqInd      function of DeviceRequest reception     */
/*              l_pfnSndCnclFunc    function of transmitting Cancel         */
/*              l_pfnRcvCnclFunc    function of receiving Cancel            */
/*              l_pfnCbClearFeature function of CreateFeature reception     */
/*              l_pfnCbEnRmtWkup    function of RemoteWakeup effective      */
/*              l_usCodeMap         code map                                */
/*              l_ucCfgValue        configuration value                     */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlInit(VOID)
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x01, 0, 0 );

    /* initialize of parameters */
    l_pfnDevReqInd      = GRUSB_NULL;
    l_pfnSndCnclFunc    = GRUSB_NULL;
    l_pfnRcvCnclFunc    = GRUSB_NULL;
    l_pfnCbClearFeature = GRUSB_NULL;
    l_pfnCbEnRmtWkup    = GRUSB_NULL;
    l_usCodeMap         = 0;
    l_ucCfgValue        = 0;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x01, 0, END_FUNC );
}
/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlReInit                                        */
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
/* MODIFY     : l_ucCfgValue        configuration valu                      */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlReInit(VOID)
{
    UINT8*                  pucDesc;        /* pointer of descriptor        */  /* 1.41 */
    UINT16                  usCnfgSz;       /* ConfigurationDescriptor Size */  /* 1.41 */
    GRUSB_DEV_CONFIG_DESC*  ptCnfgDesc;     /* ConfigurationDescriptor      */  /* 1.41 */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x14, 0, 0 );

    l_ucCfgValue = 0;

    /* Configuration descriptor information acquisition processing */           /* 1.41 */
    pucDesc = GRUSB_Prm_GetConfigurationDescriptor(l_ucCfgValue, &usCnfgSz);    /* 1.41 */
    pucDesc = GRUSB_Prm_GetConfigurationDescriptorOverride(pucDesc, usCnfgSz);  /* 1.41 */
    ptCnfgDesc = (GRUSB_DEV_CONFIG_DESC*)pucDesc;                               /* 1.41 */
                                                                                /* 1.41 */
    /* Initialize Device State */                                               /* 1.41 */
    l_aucDeviceState[0] = 0x0;                                                  /* 1.41 */
    l_aucDeviceState[1] = 0x0;                                                  /* 1.41 */
    if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_SELF_POWER)                     /* 1.41 */
    {                                                                           /* 1.41 */
        l_aucDeviceState[0] |= 0x01;                                            /* 1.41 */
    }                                                                           /* 1.41 */
                                                                                /* 1.41 */
    /* Initialize Remote Wakeup */                                              /* 1.41 */
    if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_RM_WAKEUP)                      /* 1.41 */
    {                                                                           /* 1.41 */
        l_ucEnbRmWkup |= 0x02;                                                  /* 1.41 */
    }                                                                           /* 1.41 */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x14, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlSetCallBack                                   */
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
/* MODIFY     : l_pfnDevReqInd      function of DeviceRequest reception     */
/*              l_pfnSndCnclFunc    function of transmitting Cancel         */
/*              l_pfnRcvCnclFunc    function of receiving Cancel            */
/*              l_usCodeMap         code map                                */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlSetCallBack( UINT16                  usReqCodeMap,
                                GRUSB_DeviceRequest     pfnFunc,
                                GRUSB_NoticeEndPoint    pfnSndCnclFunc,
                                GRUSB_NoticeEndPoint    pfnRcvCnclFunc)
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x02, 0, 0 );

    /* set parameters */
    l_pfnDevReqInd   = pfnFunc;
    l_pfnSndCnclFunc = pfnSndCnclFunc;
    l_pfnRcvCnclFunc = pfnRcvCnclFunc;
    l_usCodeMap      = usReqCodeMap;

    /* set callback function to part of HAL */
    GRUSB_DEV_HALCallbackCtrTransferCancel( _CtrlCallSndCancelFunc);
    GRUSB_DEV_HALCallbackCtrReceiveCancel( _CtrlCallRcvCancelFunc);

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x02, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlRcvReq                                        */
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
/* REFER      : l_usCodeMap         code map                                */
/*              l_pfnDevReqInd      function of DeviceRequest reception     */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_CtrlRcvReq( DEVICE_REQUEST_INFO*     ptDevReqInfo)
{
    UINT16      usCallBack;
    UINT8       ucRequestType;
    UINT8       ucRequest;
#ifdef GRUSB_CTL_DEBUG
    UINT16      usValue;
    UINT16      usIndex;
    UINT16      usLength;
#endif

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x03, 0, 0 );

    ucRequestType = ptDevReqInfo->ucRequestType;
    ucRequest     = ptDevReqInfo->ucRequest;
#ifdef GRUSB_CTL_DEBUG
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;
#endif

    l_eCtrSts = TRSSTS_Idle;

    if ((ucRequestType & GRUSB_DEV_REQUEST_TYPE) == GRUSB_DEV_STANDARD_TYPE)
    {
        usCallBack = (UINT16)(l_usCodeMap & (0x01 << ((INT)(ucRequest))));
        if (usCallBack != 0x00)         /* The request notified to a higher rank */
        {
            GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x03, 0x01, END_FUNC );

            /* Notice call-back of a device request */
            _RcvAplDeviceRequest( ptDevReqInfo);
            return;
        }

        l_eCtrSts = TRSSTS_Trs;

        switch (ucRequest)           /*--- DeviceRequest judging ---*/
        {
        case GRUSB_DEV_GET_STATUS:              /* GET_STATUS       */
            _RcvGetStatus( ptDevReqInfo);
            break;

        case GRUSB_DEV_CLEAR_FEATURE:           /* CLEAR_FEATUR     */
            _RcvClearFeature( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_FEATURE:             /* SET_FEATURE      */
            _RcvSetFeature( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_ADDRESS:             /* SET_ADDRESS      */
            _RcvSetAddress( ptDevReqInfo);
            break;

        case GRUSB_DEV_GET_DESCRIPTOR:          /* GET_DESCRIPTOR   */
            _RcvGetDescriptor( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_DESCRIPTOR:          /* SET_DESCRIPTOR   */
            _RcvSetDescriptor( ptDevReqInfo);
            break;

        case GRUSB_DEV_GET_CONFIGURATION:       /* GET_CONFIGURATION */
            _RcvGetConfiguration( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_CONFIGURATION:       /* SET_CONFIGURATION */
            _RcvSetConfiguration( ptDevReqInfo);
            break;

        case GRUSB_DEV_GET_INTERFACE:           /* GET_INTERFACE    */
            _RcvGetInterface( ptDevReqInfo);
            break;

        case GRUSB_DEV_SET_INTERFACE:           /* SET_INTERFACE    */
            _RcvSetInterface( ptDevReqInfo);
            break;

        case GRUSB_DEV_SYNCH_FRAME:             /* SYNCH_FRAME      */
            _RcvSynchFrame( ptDevReqInfo);
            break;

        default:                                /* except the above */
            GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x03, 0x01, 0 );

            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
            break;
        }
    }
    else
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x03, 0x02, 0 );

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
        if( g_bMTPSupport )
        {
            if( ( ucRequestType == 0xC0 ) && ( ucRequest == GRUSB_MTP_MS_VENDORCODE ) )
            {
                _RcvGetMTPFeatureDescriptor( ptDevReqInfo );
            }
            else
            {
                _RcvAplDeviceRequest( ptDevReqInfo);
            }
        }
        else
        {
            _RcvAplDeviceRequest( ptDevReqInfo);
        }
#else
        _RcvAplDeviceRequest( ptDevReqInfo);
#endif
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x03, 0x02, END_FUNC );

    return;
}

/****************************************************************************/
/* FUNCTION   : _CtrlCallSndCancelFunc                                      */
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
/* REFER      : l_pfnSndCnclFunc    function of transmitting Cancel         */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _CtrlCallSndCancelFunc(VOID)
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x04, 0, 0 );

    if ((l_pfnSndCnclFunc != GRUSB_NULL) && (l_eCtrSts == TRSSTS_Idle))
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x04, 0x01, 0 );

        (*l_pfnSndCnclFunc)( GRUSB_DEV_EP0);
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x04, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _CtrlCallRcvCancelFunc                                      */
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
/* REFER      : l_pfnRcvCnclFunc    function of receiving Cancel            */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL   VOID    _CtrlCallRcvCancelFunc( VOID )
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x05, 0, 0 );

    if ((l_pfnRcvCnclFunc != GRUSB_NULL) && (l_eCtrSts == TRSSTS_Idle))
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x05, 0x01, 0 );

        (*l_pfnRcvCnclFunc)( GRUSB_DEV_EP0);
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x05, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvGetStatus                                               */
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
LOCAL VOID _RcvGetStatus( DEVICE_REQUEST_INFO*      ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, 0, 0 );

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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, usLength, iEpNo );

    /* get device state */
    iCrtState = GRUSB_DEV_ApGetDeviceState();

    if (ucRequestType == GRUSB_DEVICE_STATUS)               /* GET_DEVICE_STATUS */
    {
        if (iCrtState <= GRUSB_DEV_STATE_DEFAULT)       /* is device state CONFIG? */
        {
            GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, 0x01, END_FUNC );

            /* set up to Stall state */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }

        /* set data of device state */
        aucResDt[0] = l_aucDeviceState[0];
        aucResDt[1] = l_aucDeviceState[1];
    }
    else if (ucRequestType == GRUSB_INTERFACE_STATUS)       /* GET_INTERFACE_STATUS */
    {
        /* check interface number */
        if (usIndex >= l_ucNumOfInterface)
        {
            GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, 0x02, END_FUNC );

            /* set up to Stall state */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }
    }
    else if (ucRequestType == GRUSB_ENDPOINT_STATUS)        /* GET_ENDPOINT_STATUS */
    {
        if ((iCrtState == GRUSB_DEV_STATE_CONFIGURED)   /* is device state CONFIG?  */
         && (iEpNo <= GRUSB_DEV_MAX_EP))                /* valid endpoint?          */
        {
            if (GRUSB_DEV_StateGetStall( iEpNo) != GRUSB_FALSE)
            {                                   /* is an applicable end point STALL? */
                aucResDt[0] = 0x01;
            }
        }
        else if ((iCrtState < GRUSB_DEV_STATE_CONFIGURED)   /* is device state befor CONFIG? */
              && (iEpNo == GRUSB_DEV_EP0))                  /* is specified endpoint 0? */
        {
            if (GRUSB_DEV_StateGetStall( iEpNo) != GRUSB_FALSE)
            {                                   /* is an applicable end point STALL? */
                aucResDt[0] = 0x01;
            }
        }
        else
        {
            GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, 0x03, END_FUNC );

            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }
    }
    else
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, 0x04, END_FUNC );

        /* set up to STALL status */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo( GRUSB_DEV_EP0);

    if (ptEpInfo == GRUSB_NULL)                         /* invalide endpoint */
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, 0x05, END_FUNC );

        /* set up to STALL status */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    _setSndData( ptEpInfo, usLength, GRUSB_FALSE, aucResDt);    /* 1.42 */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x06, 0, END_FUNC );
}


/****************************************************************************/
/* FUNCTION   : _RcvClearFeature                                            */
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
LOCAL VOID _RcvClearFeature( DEVICE_REQUEST_INFO*       ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x07, 0, 0 );

    /* get device status */
    iCrtState     = GRUSB_DEV_ApGetDeviceState();
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x07, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x07, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x07, usLength, iEpNo );

    if (ucRequestType == GRUSB_DEVICE_STATUS)       /* DEVICE_CLEAR_FEATURE */
    {
        if (usValue == GRUSB_DEV_DEVICE_REMOTE_WAKEUP)
        {                                       /* release RemoteWakeup functions */
            if (l_ucEnbRmWkup)
                l_aucDeviceState[0] &= ~0x02;   /* clear RemoteWakeup bit */

            if (l_pfnCbEnRmtWkup != GRUSB_NULL)
                /* call functions if it is registered */
                (*l_pfnCbEnRmtWkup)(GRUSB_DEV_RmtWkup_DISABLE);
        }
        else if (usValue == GRUSB_DEV_TEST_MODE)
        {                                       /* release TestMode functions */
        }
        else
        {
            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        }
    }
    else if ((ucRequestType == GRUSB_ENDPOINT_STATUS)   /* ENDPOINT_CLEAR_FEATURE   */
          && (iEpNo < GRUSB_DEV_MAX_EP))                /* valid endpoint?          */
    {
        if ((iCrtState != GRUSB_DEV_STATE_CONFIGURED)   /* device state is not CONFIG   */
         || (iEpNo != GRUSB_DEV_EP0))                   /* specified endpoint is not 0  */
        {
            /* toggle clear */
            GRUSB_DEV_HALTogleClear(iEpNo);
            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState( iEpNo, GRUSB_FALSE );

            if (l_pfnCbClearFeature != GRUSB_NULL)
                l_pfnCbClearFeature( iEpNo, usValue );
        }
        else
        {
            /* set up to STALL status */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        }
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x07, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetFeature                                              */
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
LOCAL VOID _RcvSetFeature( DEVICE_REQUEST_INFO*     ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x08, 0, 0 );

    /* get device state */
    iCrtState     = GRUSB_DEV_ApGetDeviceState();
    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;
    iEpNo         = (UINT8)(usIndex & 0x0F);

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x08, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x08, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x08, usLength, iEpNo );

    if (usLength != 0)                      /* is length 0? */
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x08, 0x01, END_FUNC );

        /* no -> set up to STALL status */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    if (ucRequestType == GRUSB_DEVICE_STATUS)           /* DEVICE_SET_FEATURE */
    {
        if (usValue == GRUSB_DEV_DEVICE_REMOTE_WAKEUP)  /* effective RemoteWakeup function */
        {
            if (l_ucEnbRmWkup)
                l_aucDeviceState[0] |= l_ucEnbRmWkup;

            if (l_pfnCbEnRmtWkup != GRUSB_NULL)
            {
                /* call functions if it is registered */
                (*l_pfnCbEnRmtWkup)(GRUSB_DEV_RmtWkup_ENABLE);
            }
        }
        else if (usValue == GRUSB_DEV_TEST_MODE)
        {
            if( l_ucBusSpd == GRUSB_PRM_BUS_HS )
            {
                /* HiSpeed */
                usIndex = (usIndex >> 8) & 0x00ff;
                GRUSB_DEV_HALSetTestMode( usIndex );
            }
            else
            {
                /* FullSpeed */
                /* set up to STALL function */
                GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
            }
        }
        else
        {
#ifdef  GRUSB_COMMON_USE_OTG
            /* notifies to OTG module */
            if ((GROTGD_Set_Feature_Notice( usValue)) != GRUSB_TRUE)
            {
                /* set up to STALL function */
                GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
            }
#else   /* GRUSB_COMMON_USE_OTG */ /* V 1.14 */
            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
#endif  /* GRUSB_COMMON_USE_OTG */ /* V 1.14 */
        }
    }
    else if (ucRequestType == GRUSB_INTERFACE_STATUS)
    {                                       /* INTERFACE_SET_FEATURE */
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
    }
    else if ((ucRequestType == GRUSB_ENDPOINT_STATUS)   /* ENDPOINT_SET_FEATURE */
          && (usValue == GRUSB_DEV_ENDPOINT_HALT)       /* halt of endpoint     */
          && (iEpNo < GRUSB_DEV_MAX_EP))                /* valid endpoint?      */
    {
        if ((iCrtState == GRUSB_DEV_STATE_CONFIGURED)   /* device state is CONFIG?  */
         || (iEpNo == GRUSB_DEV_EP0))                   /* specified endpoint is 0? */
        {
            /* set up to STALL function */
            GRUSB_DEV_StateSetStall( iEpNo, GRUSB_TRUE);
        }
        else
        {
            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        }
    }
    else
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x08, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetAddress                                              */
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
LOCAL VOID _RcvSetAddress( DEVICE_REQUEST_INFO*     ptDevReqInfo)
{
    UINT8               ucRequestType;      /* bmRequestType    */
#ifdef GRUSB_CTL_DEBUG
    UINT8               ucRequest;          /* bRequest         */
#endif
    UINT16              usValue;            /* wValue           */
    UINT16              usIndex;            /* wIndex           */
    UINT16              usLength;           /* wLength          */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x09, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x09, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x09, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x09, usLength, 0 );

    if ((ucRequestType != GRUSB_DEVICE_STATUS)
     || (usIndex != 0)                          /* wIndex must be 0           */
     || (usLength != 0)                         /* wLength must be 0          */
     || (usValue > GRUSB_DEV_MAX_ADDR))         /* wValue must be 127 or less */
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
    }
    else
    {
        /* An address is set up. */
        GRUSB_DEV_HALSetAddress( usValue);

        /* Device State change */
        GRUSB_DEV_StateSetAddress( (INT)usValue);
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x09, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvGetDescriptor                                           */
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
LOCAL VOID _RcvGetDescriptor( DEVICE_REQUEST_INFO*      ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    ucDescTyp     = (UINT8)((usValue>>8) & 0xff);
    ucDescIdx     = (UINT8)((usValue>>0) & 0xff);
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, usLength, 0 );

    if ((ucRequestType != GRUSB_DEVICE_STATUS)
     && (ucRequestType != GRUSB_INTERFACE_STATUS))
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    switch(ucDescTyp)
    {
    case GRUSB_DEV_DEVICE:                      /* GET_DEVICE_DESCRIPTOR */
        pucDesc = GRUSB_Prm_GetDeviceDescriptor( &usActLength);
        break;

    case GRUSB_DEV_CONFIGURATION:               /* GET_CONFIGURATION_DESCRIPTOR */
        pucDesc = GRUSB_Prm_GetConfigurationDescriptor( ucDescIdx, &usActLength);
        pucDesc = GRUSB_Prm_GetConfigurationDescriptorOverride(pucDesc, usActLength);
        break;

    case GRUSB_DEV_STRING:                      /* GET_STRING_DESCRIPTOR */
        pucDesc = GRUSB_Prm_GetStringDescriptor( ucDescIdx, &usActLength);
        break;

    case GRUSB_DEV_INTERFACE:                   /* GET_INTERFACE_DESCRIPTOR */
    case GRUSB_DEV_ENDPOINT:                    /* GET_ENDPOINT_DESCRIPTOR */
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;

    case GRUSB_DEV_DEVICE_QUALIFIER:            /* GET_DEVICE_QUALIFIER_DESCRIPTOR */
#ifndef GRUSB_DEV_FIXED_FS                                                  /* 1.40 */
        pucDesc = GRUSB_Prm_GetDeviceQualifierDescriptor( &usActLength);
        break;
#else                                                                       /* 1.40 */
        /* set up to STALL function */                                      /* 1.40 */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );            /* 1.40 */
        return;                                                             /* 1.40 */
#endif                                                                      /* 1.40 */

    case GRUSB_DEV_OTHER_SPEED_CONFIGURATION:   /* GET_OTHER_SPEED_CONFIGURATION_DESCRIPTOR */
#ifndef GRUSB_DEV_FIXED_FS                                                  /* 1.40 */
        pucDesc = GRUSB_Prm_GetOtherSpeedConfigurationDescriptor( ucDescIdx, &usActLength);
        break;
#else                                                                       /* 1.40 */
        /* set up to STALL function */                                      /* 1.40 */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );            /* 1.40 */
        return;                                                             /* 1.40 */
#endif                                                                      /* 1.40 */

    default:                                    /* except the above */
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, 0x2, END_FUNC );

        if (l_pfnDevReqInd != GRUSB_NULL)
            (*l_pfnDevReqInd)( ptDevReqInfo->ucRequestType, ucRequest, usValue, usIndex, usLength );
        return;
    }

    /* check got pointer */
    if (pucDesc == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
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
    ptEpInfo = GRUSB_Prm_GetEndPointInfo( GRUSB_DEV_EP0 );

    if (ptEpInfo == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, 0x04, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* data which transmits is set to cyclic buffer. */
    _setSndData( ptEpInfo, usLength, i0Len, pucDesc);           /* 1.42 */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0A, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetDescriptor                                           */
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
LOCAL VOID _RcvSetDescriptor( DEVICE_REQUEST_INFO*      ptDevReqInfo)
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0B, 0, 0 );

    /* set up to STALL function */
    GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0B, 0, END_FUNC );

    return;
}

/****************************************************************************/
/* FUNCTION   : _RcvGetConfiguration                                        */
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
LOCAL VOID _RcvGetConfiguration( DEVICE_REQUEST_INFO*       ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
#endif
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, usLength, 0 );

    if ((ucRequestType != GRUSB_DEVICE_STATUS)
     || (usLength != 1))                    /* is length 1? */
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* get device state */
    iCrtState = GRUSB_DEV_StateGetDeviceState();

    switch(iCrtState)
    {
    case GRUSB_DEV_STATE_DEFAULT:       /* DEFAULT state */
    case GRUSB_DEV_STATE_ADDRESS:       /* ADDRESS state */
        l_ucCfgValue = 0x00;
        break;

    case GRUSB_DEV_STATE_CONFIGURED:    /* CONFIGURED state */
        l_ucCfgValue = 0x01;
        break;

    default:
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, 0x02, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo( GRUSB_DEV_EP0);

    if (ptEpInfo == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* data which transmits is set to cyclic buffer. */
    aucResDt[0] = l_ucCfgValue;
    _setSndData( ptEpInfo, usLength, GRUSB_FALSE, aucResDt);    /* 1.42 */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0C, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetConfiguration                                        */
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
LOCAL VOID _RcvSetConfiguration( DEVICE_REQUEST_INFO*       ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, 0, 0 );

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
    l_aucDeviceState[0] = 0x0;
    l_aucDeviceState[1] = 0x0;
#endif  /* 1.41 */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, usLength, 0 );

    if (ucRequestType != GRUSB_DEVICE_STATUS)
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* get device state */
    iCrtState = GRUSB_DEV_ApGetDeviceState();

    switch(iCrtState)
    {
    case GRUSB_DEV_STATE_DEFAULT:       /* DEFAULT state */
        if (usValue == 0x01)            /* after SET_CONFIGURATION? */
        {
            GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, 0x02, END_FUNC );

            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
            return;
        }
        break;

    case GRUSB_DEV_STATE_ADDRESS:       /* ADDRESS state */
    case GRUSB_DEV_STATE_CONFIGURED:    /* CONFIGURED state */

        l_ucCfgValue = (UINT8)usValue;

        GRUSB_DEV_StateSetConfigure( (INT)usValue);

        if (usValue != 0)
        {
            /* Configuration descriptor information acquisition processing */
            pucDesc = GRUSB_Prm_GetConfigurationDescriptor( (UINT8)(usValue-1), &usCnfgSz);
            pucDesc = GRUSB_Prm_GetConfigurationDescriptorOverride(pucDesc, usCnfgSz);
            ptCnfgDesc = (GRUSB_DEV_CONFIG_DESC*)pucDesc;

            if (ptCnfgDesc == GRUSB_NULL)
            {
                GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, 0x02, END_FUNC );

                /* set up to STALL function */
                GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
                return;
            }

            if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_SELF_POWER)
                                                            /* self power */
            {
                l_aucDeviceState[0] |= 0x01;
            }
            else                                                                /* 1.41 */
            {                                                                   /* 1.41 */
                l_aucDeviceState[0] &= ~0x01;                                   /* 1.41 */
            }                                                                   /* 1.41 */
            if (ptCnfgDesc->uc_bmAttributes & GRUSB_DEV_RM_WAKEUP)
                                                            /* Remote Wakeup */
            {
                l_ucEnbRmWkup |= 0x02;
            }
            else                                                                /* 1.41 */
            {                                                                   /* 1.41 */
                l_ucEnbRmWkup &= ~0x02;                                         /* 1.41 */
            }                                                                   /* 1.41 */

            l_ucNumOfInterface = ptCnfgDesc->uc_bNumInterfaces;
        }

        break;

    default:                            /* except the above */
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0D, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvGetInterface                                            */
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
LOCAL VOID _RcvGetInterface( DEVICE_REQUEST_INFO*       ptDevReqInfo )
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, 0, 0 );

    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
#endif
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, usLength, 0 );

    if ((ucRequestType != GRUSB_INTERFACE_STATUS)
     || (usLength != 1))                        /* is length 1? */
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
        return;
    }

    /* get device state */
    iCrtState = GRUSB_DEV_ApGetDeviceState();

    aucResDt[0] = 0x00;

    switch( iCrtState )
    {
    case GRUSB_DEV_STATE_CONFIGURED:        /* CONFIGURED state */
        /* An alternative setting value is substituted for ucSndData. */
        iStat = GRUSB_Prm_GetInterface( usIndex, &ucInterface);
        if (iStat != GRUSB_TRUE)
        {
            GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, 0x02, END_FUNC );

            /* set up to STALL function */
            GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
            return;
        }
        aucResDt[0] = ucInterface;
        break;

    case GRUSB_DEV_STATE_DEFAULT:           /* DEFAULT state */
    case GRUSB_DEV_STATE_ADDRESS:           /* ADDRESS state */
    default:
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, 0x03, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo( GRUSB_DEV_EP0);

    if (ptEpInfo == GRUSB_NULL)
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, 0x04, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* data which transmits is set to cyclic buffer. */
    _setSndData( ptEpInfo, usLength, GRUSB_FALSE, aucResDt);    /* 1.42 */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0E, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSetInterface                                            */
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
LOCAL VOID _RcvSetInterface( DEVICE_REQUEST_INFO*       ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0F, 0, 0 );

    /* get device state */
    iCrtState     = GRUSB_DEV_ApGetDeviceState();
    ucRequestType = (UINT8)(ptDevReqInfo->ucRequestType & GRUSB_REQUEST_RECIPIENT);
#ifdef GRUSB_CTL_DEBUG
    ucRequest     = ptDevReqInfo->ucRequest;
#endif
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0F, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0F, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0F, usLength, 0 );

    if ((ucRequestType != GRUSB_INTERFACE_STATUS)
     || (usLength != 0)                             /* is length 0?             */
     || (iCrtState != GRUSB_DEV_STATE_CONFIGURED))  /* except CONFIGURED state  */
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0F, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    /* Alternative setting value registration */
    iStat = GRUSB_Prm_SetInterface( usIndex, usValue);
    if (iStat != GRUSB_TRUE)
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0F, 0x02, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return;
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x0F, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _RcvSynchFrame                                              */
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
LOCAL VOID _RcvSynchFrame( DEVICE_REQUEST_INFO*     ptDevReqInfo)
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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x10, 0, 0 );

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

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x10, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x10, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x10, usLength, iEpNo );

    if ((ucRequestType != GRUSB_ENDPOINT_STATUS)
     || (iEpNo > GRUSB_DEV_MAX_EP))
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
    }
    else
    {
        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( (INT)usIndex, GRUSB_TRUE);
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x10, 0, END_FUNC );

    return;
}

/****************************************************************************/
/* FUNCTION   : _RcvAplDeviceRequest                                        */
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
LOCAL VOID _RcvAplDeviceRequest( DEVICE_REQUEST_INFO*       ptDevReqInfo)
{
    UINT8           ucRequestType;      /* bmRequestType    */
    UINT8           ucRequest;          /* bRequest         */
    UINT16          usValue;            /* wValue           */
    UINT16          usIndex;            /* wIndex           */
    UINT16          usLength;           /* wLength          */

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x11, 0, 0 );

    /* get device state */
    ucRequestType = ptDevReqInfo->ucRequestType;
    ucRequest     = ptDevReqInfo->ucRequest;
    usValue       = ptDevReqInfo->usValue;
    usIndex       = ptDevReqInfo->usIndex;
    usLength      = ptDevReqInfo->usLength;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x11, ucRequestType, ucRequest );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x11, usValue, usIndex );
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x11, usLength, 0 );

    if (l_pfnDevReqInd != GRUSB_NULL)
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x11, 0x01, 0 );

        (*l_pfnDevReqInd)( ucRequestType, ucRequest, usValue, usIndex, usLength);
    }

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x11, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlCallbackClearFeature                          */
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
VOID GRUSB_DEV_CtrlCallbackClearFeature( GRUSB_NoticeEndPoint16     pFunc)
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x12, 0, 0 );

    l_pfnCbClearFeature = pFunc;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x12, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _setSndData                                                 */
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
VOID _setSndData( GRUSB_EndPointInfo*       ptEpInfo,
                  UINT16                    usLength,
                  INT                       i0Len,              /* 1.42 */
                  UINT8*                    pucResData)
{
    INT                     iWrtPnt;
    GRUSB_SndDataInfo       *ptSndDtInfo;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x13, 0, 0 );

    /* data which transmits is set to cyclic buffer. */
    iWrtPnt = GRLIB_Cyclic_CheckWrite( &(ptEpInfo->tCycBufInfo) );
    if ( iWrtPnt == GRLIB_NONBUFFER )
    {
        GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x13, 0x01, END_FUNC );

        /* set up to STALL function */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE );
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

    GRLIB_Cyclic_IncWrite( &(ptEpInfo->tCycBufInfo));

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x13, 0, END_FUNC );
}

/********************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlCallbackEnRmtWkup                                 */
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
VOID GRUSB_DEV_CtrlCallbackEnRmtWkup(GRUSB_Notice16         pFunc)
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x15, 0, 0 );

    l_pfnCbEnRmtWkup = pFunc;

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x15, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_CtrlRmtWkup                                       */
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
VOID GRUSB_DEV_CtrlRmtWkup(VOID)
{
    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x16, 0, 0 );

    /* Execution of a remote Wakeup */
    GRUSB_DEV_HALRmtWkup();

    GRCTL_DBG_TRACE( GRDBG_PERI_CTL, 0x16, 0, END_FUNC );
}

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
/****************************************************************************/
/* FUNCTION   : _RcvGetMTPFeatureDescriptor                                 */
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
LOCAL VOID _RcvGetMTPFeatureDescriptor( DEVICE_REQUEST_INFO* ptDevReqInfo )
{
    UINT16              usLength;
    GRUSB_EndPointInfo  *ptEpInfo;
    INT                 i0Len;              /* 0-Length Addition            */  /* 1.42 */

    usLength = ptDevReqInfo->usLength;
    if(usLength > sizeof(l_uMTPFeatureDesc))
    {
        usLength = sizeof(l_uMTPFeatureDesc);
        i0Len    = GRUSB_TRUE;                              /* 1.42 */
    }
    else                                                    /* 1.42 */
    {                                                       /* 1.42 */
        i0Len    = GRUSB_FALSE;                             /* 1.42 */
    }                                                       /* 1.42 */

    /* End point information acquisition */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo( GRUSB_DEV_EP0 );

    /* data which transmits is set to cyclic buffer. */
    _setSndData( ptEpInfo, usLength, i0Len, (UINT8*)&l_uMTPFeatureDesc.uacData[0] );    /* 1.42 */
}
#endif
