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
/*      grp_usbd.c                                                              1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# USB driver                                                                 */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Processing concernign Cast is corrected.                        */
/*                              - grp_usbd_ClearFeature                                         */
/*                              - grp_usbd_DeviceRequest                                        */
/*                              - grp_usbd_GetConfigDescriptor                                  */
/*                              - grp_usbd_GetDeviceDescriptor                                  */
/*                              - grp_usbd_GetStatus                                            */
/*                              - grp_usbd_GetStringDescriptor                                  */
/*                              - grp_usbd_PipeActive                                           */
/*                              - grp_usbd_PipeHalt                                             */
/*                              - grp_usbd_SetAddress                                           */
/*                              - grp_usbd_SetConfiguration                                     */
/*                              - grp_usbd_SetFeature                                           */
/*                              - grp_usbd_SetInterface                                         */
/*                              - grp_usbd_UnSetConfiguration                                   */
/*                              - grp_usbd_SetConfigDescriptor                                  */
/*                              - grp_usbd_SetDeviceDescriptor                                  */
/*                              - grp_usbd_SetStringDescriptor                                  */
/*                              - grp_usbd_SynchFrame                                           */
/*                            - Correction by member change in structure.                       */
/*                              - grp_usbdi_request                                             */
/*                                  pfnCallbackFunc -> pfnNrCbFunc                              */
/*                              - grp_usbdi_device_request                                      */
/*                                  pfnCallbackFunc -> pfnDvCbFunc                              */
/*                              - grp_usbdi_st_device_request                                   */
/*                                  pfnCallbackFunc -> pfnStCbFunc                              */
/*   K.Takagi       2013/04/19  V1.04                                                           */
/*                            - Corrected to process of the change state by the Suspend/Resume. */
/*                              - USBD_MACRO_CONNECT_CHECK                                      */
/*                              - grp_usbd_DisconnectDevice                                     */
/*                              - grp_usbd_NotifyEvent                                          */
/*                              - _grp_usbd_PortControl                                         */
/*                            - Warning removal of compile.                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Fixed bug where STALL is returned when PORT_RESUME is executed  */
/*                              for external HUB.                                               */
/*                              - _grp_usbd_PortControl                                         */
/*                            - Fixed incorrect return value.                                   */
/*                              - grp_usbd_SetInterface                                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_ctr.h"
#include "grp_usbc.h"
#include "grp_usbd_local.h"
#include "grp_hcd.h"
#include "grp_usbc_dbg.h"

#include "grp_usbc_reinit.h" /* reinit */

#ifdef GRP_USB_HOST_USE_HUB_DRIVER
#include "grp_hubd.h"
#endif  /* GRP_USB_HOST_USE_HUB_DRIVER */


/***** LOCAL PARAMETER DEFINITIONS **************************************************************/
DLOCAL grp_usbd_cb l_tUSBD_CB;

static grp_u16                  l_usDevTag = GRP_USBD_DEVID_TAG_MIN; /* reinit */

/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/
/* Callback function prototype declaration */
LOCAL void _grp_usbd_CmpDeviceRequest(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpGetStatus(grp_hcdi_tr_request *);
/*LOCAL void _grp_usbd_CmpSynchframe(grp_hcdi_tr_request *);*/                                      /* V1.04    */
LOCAL void _grp_usbd_CmpNormalRequest(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpPipeActive(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpPipeHalt(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpSetaddress(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpSetconfiguration(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpSetinterface(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpStdDeviceRequest(grp_hcdi_tr_request *);
LOCAL void _grp_usbd_CmpUnsetconfiguration(grp_hcdi_tr_request *);
#ifdef GRP_USB_HOST_USE_ISOCHRONOUS                                                                 /* V1.04 {  */
LOCAL void _grp_usbd_CmpSynchFrame( grp_hcdi_tr_request *);
#endif /* GRP_USB_HOST_USE_ISOCHRONOUS */                                                           /* V1.04 }  */

/* Internal function prototype declaration */
LOCAL grp_s32   _grp_usbd_Add0DefaultpipeOpen(void);
LOCAL grp_s32   _grp_usbd_Add0DefaultpipeClose(void);
LOCAL grp_s32   _grp_usbd_AllDeleteUrb(grp_usbdi_pipe *);
LOCAL grp_u32   _grp_usbd_CalcBandwidth(grp_u8,grp_usbdi_ep_desc *);
LOCAL grp_u32   _grp_usbd_CalcIfBandwidth(grp_u8,grp_u8,grp_u8,grp_usbdi_config_desc *);
/*LOCAL grp_s32   _grp_usbd_CloseAllPipes(grp_u8);*/                                                /* V1.04    */
LOCAL void      _grp_usbd_CloseInterfaceAllPipes(grp_usbdi_pipe_operate *,grp_usbdi_if_desc *);
LOCAL grp_s32   _grp_usbd_DefaultpipeOpen(grp_u8,grp_u8);
LOCAL grp_s32   _grp_usbd_DefaultpipeClose(grp_u8);
LOCAL grp_u8    _grp_usbd_GetEndpointInterval(grp_si,grp_usbdi_ep_desc *);
LOCAL grp_s32   _grp_usbd_GetFirstEpDescptr(grp_usbdi_config_desc *,grp_usbdi_if_desc *,grp_usbdi_ep_desc **,void **);
LOCAL grp_s32   _grp_usbd_GetIfDescptr(grp_usbdi_config_desc *,grp_u8,grp_u8,grp_usbdi_if_desc **);
LOCAL grp_s32   _grp_usbd_GetNextEpDescptr(grp_usbdi_config_desc *,grp_usbdi_ep_desc *,grp_usbdi_ep_desc **,void **);
LOCAL grp_s32   _grp_usbd_GlobalBusControl(grp_usbdi_bus_control *);
LOCAL grp_s32   _grp_usbd_IFBandwidthCheck(grp_u8,grp_usbdi_descriptor_info *);
LOCAL grp_s32   _grp_usbd_InternalDeviceRequest(grp_usbdi_st_device_request *);
LOCAL grp_s32   _grp_usbd_InternalPipeCheck(grp_usbdi_pipe *);
LOCAL grp_s32   _grp_usbd_OpenInterfaceAllPipes(grp_usbdi_pipe_operate *,grp_usbdi_if_desc *);
LOCAL grp_s32   _grp_usbd_PortControl(grp_usbdi_bus_control *);
LOCAL grp_s32   _grp_usbd_SearchDevId(grp_u16 *);
LOCAL grp_si    _grp_usbd_SearchHcidx(grp_u16);
LOCAL grp_s32   _grp_usbd_ReleaseDevId(grp_u16);
LOCAL grp_s32   _grp_usbd_InitResource(void);
LOCAL void      _grp_usbd_InitControlBlock(void);
LOCAL grp_s32   _grp_usbd_InitHcControlBlock(void);
LOCAL grp_s32   _grp_usbd_SearchUSB20Hub(grp_u8,grp_u8 *,grp_u8 *);
LOCAL grp_s32   _grp_usbd_GetDevId(grp_u8,grp_u16*);
LOCAL grp_s32   _grp_usbd_GetAddress(grp_u16,grp_u8 *);
LOCAL grp_s32   _grp_usbd_CheckPipeSelect(grp_usbdi_pipe_operate *, grp_usbdi_ep_desc *ptEpDesPtr);

/* External function declaration */
//EXTERN grp_s32 GRP_HCD_Init(grp_hcdi_system_init *);


/* Internal error checking macro */

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING

/************************************************************************************************/
/* MACRO      : _ID2AD                                                                          */
/*                                                                                              */
/* DESCRIPTION: Macro for get devide address by device id                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : id                              address number                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Device address                                                                  */
/*                                                                                              */
/************************************************************************************************/
#define     _ID2AD(id)   ((grp_u8)(id & GRP_USBD_DEVID_ADDR_MASK))

/************************************************************************************************/
/* MACRO      : USBD_MACRO_ADRS_CHECK                                                           */
/*                                                                                              */
/* DESCRIPTION: Macro for checking address number                                               */
/*              returns from function with error code at error                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ad                              address number                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*                                                                                              */
/************************************************************************************************/
#define     USBD_MACRO_ADRS_CHECK(ad)\
{\
    if( (ad) >= GRP_USBD_HOST_MAX_DEVICE ){\
        return GRP_USBD_INVALIED_ADR;\
    }\
    if( ( (ad) != GRP_USBD_DEFAULT_ADDRESS )\
    && ( l_tUSBD_CB.ausDevIdTable[ad] == GRP_USBD_ADDRESS_NO_ASIGNED) ){\
        return GRP_USBD_INVALIED_ADR;\
    }\
}

/************************************************************************************************/
/* MACRO      : USBD_MACRO_CONNECT_CHECK                                                        */
/*                                                                                              */
/* DESCRIPTION: Macro for checking connection                                                   */
/*              returns from function with error code at error                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ad                              address number                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_BUS_SUSPEND            USB bus suspend state                           */
/*              GRP_USBD_HOST_HALT              Host controller HALT state                      */
/*                                                                                              */
/************************************************************************************************/
#define     USBD_MACRO_CONNECT_CHECK(ad) \
{\
    if( l_tUSBD_CB.atDevTable[(ad)].ptHcIndex->ucHcState != GRP_USBD_HC_ACTIVE ){\
        if( l_tUSBD_CB.atDevTable[(ad)].ptHcIndex->ucHcState == GRP_USBD_HC_SUSPEND ){\
            return GRP_USBD_BUS_SUSPEND;\
        }\
        else {\
            return GRP_USBD_HOST_HALT;\
        }\
    }\
    if( (l_tUSBD_CB.atDevTable[(ad)].iPortState != GRP_USBD_STATE_CONNECT)\
     && (l_tUSBD_CB.atDevTable[(ad)].iPortState != GRP_USBD_STATE_OPEN) ){\
        if (l_tUSBD_CB.atDevTable[(ad)].iPortState == GRP_USBD_STATE_SUSPEND ){\
            return GRP_USBD_BUS_SUSPEND;\
        }\
        else if( l_tUSBD_CB.atDevTable[(ad)].iPortState == GRP_USBD_STATE_HIBERNATE ){\
            return GRP_USBD_BUS_SUSPEND;/*return GRP_USBD_STATE_HIBERNATE;*/\
        }\
        else {\
            return GRP_USBD_HOST_HALT;\
        }\
    }\
}

/************************************************************************************************/
/* MACRO      : USBD_MACRO_PIPE_HALT_CHECK                                                      */
/*                                                                                              */
/* DESCRIPTION: Macro for checking pipe HALT                                                    */
/*              returns with error code from function at HALT                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pipe                            pipe handler                                    */
/*              err                             error code                                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : err                             error code                                      */
/*                                                                                              */
/************************************************************************************************/
#define     USBD_MACRO_PIPE_HALT_CHECK(pipe,err) \
{\
    if( (pipe)->iStatus == GRP_USBD_PIPE_HALT ){\
        return (err);\
    }\
}

/************************************************************************************************/
/* MACRO      : USBD_MACRO_PIPE_CHECK                                                           */
/*                                                                                              */
/* DESCRIPTION: pipe check macro                                                                */
/*              returns from function with error code at error                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pipe                            pipe handler                                    */
/*              err                             error code                                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : err                             error code                                      */
/*                                                                                              */
/************************************************************************************************/
#define     USBD_MACRO_PIPE_CHECK(pipe,err) \
{\
    if( GRP_USBD_OK != _grp_usbd_InternalPipeCheck((pipe)) ){\
        return (err);\
    }\
}

/************************************************************************************************/
/* MACRO      : USBD_MACRO_BANDWIDTH_CHECK                                                      */
/*                                                                                              */
/* DESCRIPTION: Macro checking bandwidth                                                        */
/*              returns from function with error code on error                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ad                              address number                                  */
/*              desc                            device descriptor                               */
/*              bd                              current bandwidth                               */
/*              err                             error code                                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : err                             error code                                      */
/*                                                                                              */
/************************************************************************************************/
#define     USBD_MACRO_BANDWIDTH_CHECK(ad,desc,bd,err) \
{\
    if( GRP_USBD_HC_MAXBANDWIDTH < _grp_usbd_CalcBandwidth((ad),(desc)) + (bd) ){\
        return (err);\
    }\
}

#endif  /* GRP_USB_HOST_NO_PARAM_CHECKING */

/* GR-USBD library functions */

/************************************************************************************************/
/* FUNCTION   : grp_usbd_Init                                                                   */
/*                                                                                              */
/* DESCRIPTION: USB driver initialization                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptInit                          initialize data                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INIT_ERROR             Porting module error                            */
/*              GRP_USBD_HOST_INIT_ERROR        HCD initialization error                        */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_Init(grp_usbdi_system_init *ptInit)
{
grp_s32                         lStatus;

    ptInit = GRP_USB_NULL;  /* Warning measures */

    /* Initialize internal resource(Semaphore) */
    if(_not_initialized) { /* reinit */
        lStatus = _grp_usbd_InitResource();
        if( lStatus != GRP_USBD_OK ){
            /* error */
            return GRP_USBD_INIT_ERROR;
        }
    } /* reinit */

    /* Initialize USB driver control block */
    _grp_usbd_InitControlBlock();

    /* Initialize host controller control block */
    lStatus = _grp_usbd_InitHcControlBlock();
    if( lStatus != GRP_USBD_OK ){
        /* error */
        return GRP_USBD_HOST_INIT_ERROR;
    }

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_IdToAddress                                                            */
/*                                                                                              */
/* DESCRIPTION: Get device address by device identifier                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*              pucAddr                         Area for getting the device address             */
/* OUTPUT     : *pucAddr                        Device address                                  */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_IdToAddress( grp_u16 usDevId, grp_u8 *pucAddr)
{
grp_s32                         lRetStatus = GRP_USBD_OK;
grp_u16                         usTag;

    *pucAddr = (grp_u8)(usDevId & GRP_USBD_DEVID_ADDR_MASK);
    usTag = (grp_u16)(usDevId >> 8);

    if( (l_tUSBD_CB.ausDevIdTable[(*pucAddr)] != usDevId) && (usTag != 0) ) {
        lRetStatus = GRP_USBD_ILLEGAL_ERROR;
        *pucAddr = 0;
    }

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_BusPowerControl                                                        */
/*                                                                                              */
/* DESCRIPTION: USB bus power and port control                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           port control structure                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_INVALIED_PMTR          Invalid parameter                               */
/*              GRP_USBD_HOST_HALT              Host controller is halted                       */
/*              USBDI_NO_FUCNTION               Host controller driver doesn't have this        */
/*                                              function                                        */
/*              GRP_USBD_ILLEGAL_ERROR          Other error                                     */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_BusPowerControl( grp_usbdi_bus_control *ptReq)
{
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x01, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->tDev.usUsbDevId));
#endif

    switch( ptReq->iReq ){
    case GRP_USBD_GLOBAL_SUSPEND:                       /* following down   */
    case GRP_USBD_GLOBAL_RESUME:
        /* Global Suspend */
        lStatus = _grp_usbd_GlobalBusControl(ptReq);
        break;

    case GRP_USBD_PORT_SUSPEND:                         /* following down   */
    case GRP_USBD_PORT_RESUME:                          /* following down   */
    case GRP_USBD_PORT_RESET:
        /* Port Suspend */
        lStatus = _grp_usbd_PortControl(ptReq);
        break;

    default:
        lStatus = GRP_USBD_INVALIED_PMTR;
        break;
    }

    _TRACE_USBC_USBD_(0x01, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_ClearFeature                                                           */
/*                                                                                              */
/* DESCRIPTION: clearing                                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*               (Member used)                                                                  */
/*              usUsbDevId                      Device identifier                               */
/*              ucFeatureSelector               Feature selector                                */
/*              ucEndpoint                      endpoint number                                 */
/*              pfnCallbackFunc                 Callback function after execution               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_ClearFeature( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;
grp_u8                          ucSelector = GRP_USBD_TYPE_DEVICE;

    _TRACE_USBC_USBD_(0x02, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif /* GRP_USB_HOST_NO_PARAM_CHECKING */

    /* Setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* Create request */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x02, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Set setup data */
    if( ptReq->ucFeatureSelector == GRP_USBD_ENDPOINT_HALT ){
        ucSelector = GRP_USBD_TYPE_ENDPOINT;

        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)  = ptReq->ucEndpoint;     /* wIndex */
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH) = 0;
    }
    else {
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)  = 0;
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH) = 0;
    }

    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (grp_u8)((GRP_USBD_TYPE_HOST2DEV
                                                                        | GRP_USBD_TYPE_STANDARD
                                                                        | ucSelector));
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_CLEAR_FEATURE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = (grp_u8)ptReq->ucFeatureSelector;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;

    ptIrpPtr->ulBufferLength                                          = 0;
    ptIrpPtr->ulXferinfo                                              = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc = _grp_usbd_CmpStdDeviceRequest;

    /* Make an internal device request */
    _TRACE_USBC_USBD_(0x02, 0x00, F_END);

    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_ConnectDevice                                                          */
/*                                                                                              */
/* DESCRIPTION: Notify USB device connection                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iSpeed                          Transfer speed                                  */
/*                                              GRP_USBD_FULL_SPEED : full speed                */
/*                                              GRP_USBD_LOW_SPEED  : low speed                 */
/*                                              GRP_USBD_HIGH_SPEED : high speed                */
/*              usHcIndexNum                    Host controller index number                    */
/*              ptDevInfo                       Device information                              */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_ConnectDevice( grp_si iSpeed, grp_usbdi_device_info *ptDevInfo)
{
grp_si                          iHcNum;
grp_s32                         lStatus;
grp_s32                         lRetStatus;

    /* Connect the device to temporary area(address 0 connection) */
    _TRACE_USBC_USBD_(0x03, 0x00, 0x00);

    /* search host controller data */
    iHcNum = _grp_usbd_SearchHcidx(ptDevInfo->usHcIndexNum);

    /* Set Host controller index */
    if( iHcNum >= 0 ){
        l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].ptHcIndex = &l_tUSBD_CB.atHcData[iHcNum];
    }
    else {
        _TRACE_USBC_USBD_(0x03, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* set host controller information */
    /* if USB device is connected,host controller is active. */
    l_tUSBD_CB.atHcData[iHcNum].ucHcState = GRP_USBD_HC_ACTIVE;

    /* set port information */
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iSpeed                   = iSpeed;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDeviceInfo.usUsbDevId   = GRP_USBD_DEFAULT_DEVID;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDeviceInfo.ucHubAddr    = ptDevInfo->ucHubAddr;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDeviceInfo.ucPortNum    = ptDevInfo->ucPortNum;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDeviceInfo.ucPortInfo   = ptDevInfo->ucPortInfo;

    /* Open the default pipe */
    lStatus = _grp_usbd_Add0DefaultpipeOpen();
    if( lStatus == GRP_USBD_OK ) {
        /* port connect */
        l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iPortState = GRP_USBD_STATE_CONNECT;
        lRetStatus = GRP_USBD_OK;
    }
    else {
        _TRACE_USBC_USBD_(0x03, 0x01, 0x01);
        lRetStatus = GRP_USBD_ILLEGAL_ERROR;
    }

    _TRACE_USBC_USBD_(0x03, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_DeviceRequest                                                          */
/*                                                                                              */
/* DESCRIPTION: Notify USB device connection                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Communication request                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Address error                                   */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend state                           */
/*              GRP_USBD_HOST_HALT              Host controller HALT state                      */
/*              GRP_USBD_ILLEGAL_ERROR          function error                                  */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_DeviceRequest( grp_usbdi_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x04, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* Only OUT is that transaction of 0 lengths. */
    if( (ptReq->wLength == 0 ) && (ptReq->bmRequestType & GRP_USBD_TYPE_DEV2HOST) ) {
        _TRACE_USBC_USBD_(0x04, 0x01, F_END);
/*        return GRP_USBD_INVALIED_PMTR; reserved for the future */
    }

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if (lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x04, 0x02, F_END);
        return lStatus;
    }

    /* Get Default Pipe */
    ptPipe = &l_tUSBD_CB.atDevTable[ucAddr].tDefaultPipe;

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* Create send packet */

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x04, 0x03, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = ptReq->bmRequestType;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = ptReq->bRequest;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = (grp_u8)((ptReq->wValue & 0x00FF));
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = (grp_u8)((ptReq->wValue & 0xFF00) >> 8);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = (grp_u8)((ptReq->wIndex & 0x00FF));
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = (grp_u8)((ptReq->wIndex & 0xFF00) >> 8);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = (grp_u8)((ptReq->wLength & 0x00FF));
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = (grp_u8)((ptReq->wLength & 0xFF00) >> 8);

    /* other data */
    ptIrpPtr->ptEndpoint     = &ptPipe->tEndpoint;
    ptIrpPtr->pucBufferPtr   = ptReq->pucBuffer;
    ptIrpPtr->ulBufferLength = ptReq->wLength;
    ptIrpPtr->ulXferinfo     = (grp_u32)((ptReq->bmRequestType & GRP_USBD_TYPE_DEV2HOST)
                                ? GRP_USBD_TX_IN : GRP_USBD_TX_OUT);
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpDeviceRequest;
    ptIrpPtr->iRefCon        = 0;
    ptIrpPtr->pvUrbPtr       = (void *)ptReq;

    /* shortXfer = OK */
    ptIrpPtr->iShortXferOK   = GRP_USB_TRUE;

    /* set host controller index number */
    ptIrpPtr->tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

    /* Make a transfer request to Host Controller driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcTrRun(ptIrpPtr);
    /* error code check */
    if( lStatus != GRP_HCDI_OK ){
        _TRACE_USBC_USBD_(0x04, 0x01, 0x01);

        /* Release setup data buffer to usb communication buffer */
        grp_cmem_BlkRel(ptIrpPtr->pucSetupPtr);
    }

    _TRACE_USBC_USBD_(0x04, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_DeviceRequestCancel                                                    */
/*                                                                                              */
/* DESCRIPTION: Device request cancellation                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Communication request                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Address error                                   */
/*              GRP_USBD_ALREADY_XFER           Start/end transfer                              */
/*              GRP_USBD_ILLEGAL_ERROR          function error                                  */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_DeviceRequestCancel( grp_usbdi_device_request *ptReq)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x05, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x05, 0x01, F_END);
        return lStatus;
    }

    /* Make a cancel request to lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcTrDel(&ptReq->tIrp);
    /* Error code check */
    if( lStatus != GRP_USBD_ILLEGAL_ERROR ){
        /* Release setup data buffer to usb communication buffer */
        grp_cmem_BlkRel(ptReq->tIrp.pucSetupPtr);
    }

    _TRACE_USBC_USBD_(0x05, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_DisconnectDevice                                                       */
/*                                                                                              */
/* DESCRIPTION: Device disconnection notification                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*                                              (Address is not set in case of 0 Disconnects    */
/*                                              the device)                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Address error                                   */
/*              GRP_USBD_ILLEGAL_ERROR          function error                                  */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_DisconnectDevice( grp_u16 usDevId)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x06, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x06, 0x01, F_END);
        return lStatus;
    }

    /* Address is not set at at the device with address 0 */
    /* Must close the default pipe as well */
    if( ucAddr == GRP_USBD_DEFAULT_ADDRESS ){
        _TRACE_USBC_USBD_(0x06, 0x01, 0x00);
        /* Close the default pipe */
        lStatus = _grp_usbd_Add0DefaultpipeClose();
    }
    else {
        /* Physical device was deleted */
        _TRACE_USBC_USBD_(0x06, 0x02, 0x00);

        /* Close the defaule pipe */
        lStatus = _grp_usbd_DefaultpipeClose(ucAddr);
        if( lStatus == GRP_USBD_OK ){
            _TRACE_USBC_USBD_(0x06, 0x03, 0x00);

            /* Set the state for commnunication disable */
/*          if( l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_CONNECT ){*/             /* V1.04 {  */
            if( (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_CONNECT )
             || (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_HIBERNATE )){           /* V1.04 }  */
                l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_BLANK;
                /* Release Address */
                _grp_usbd_ReleaseDevId(usDevId);

                _TRACE_USBC_USBD_(0x06, 0x04, 0x00);
            }
            else if( (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_OPEN )
                  || (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_SUSPEND )){
                l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_DISCONNECT;
            }
        }
    }

    _TRACE_USBC_USBD_(0x06, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetConfigDescriptor                                                    */
/*                                                                                              */
/* DESCRIPTION: Get configuration descriptor                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device                   */
/*                                              request                                         */
/*              (Member used)                                                                   */
/*              address                         Device address                                  */
/*              configuration                   Configuration number                            */
/*              ulSize                          Get discriptor size                             */
/*              descriptor                      Head address in the area to store discriptor    */
/*                                              information                                     */
/*              pfnCallbackFunc                 Callback function at the end of execution       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_GetConfigDescriptor( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x07, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x07, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_DEV2HOST
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_GET_DESCRIPTOR;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = (grp_u8)((ptReq->ucConfiguration) & 0xF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = GRP_USBD_DESC_CONFIGURATION;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = (grp_u8)(ptReq->ulSize & 0x00FF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = (grp_u8)((ptReq->ulSize & 0xFF00) >> 8);

    /* other data */
    ptIrpPtr->pucBufferPtr   = (grp_u8 *)ptReq->pvDescriptor;
    ptIrpPtr->ulBufferLength = ptReq->ulSize;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_IN;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpStdDeviceRequest;

    _TRACE_USBC_USBD_(0x07, 0x00, F_END);

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetDeviceDescriptor                                                    */
/*                                                                                              */
/* DESCRIPTION: Get device descriptor                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device                   */
/*                                              request                                         */
/*              (Member used)                                                                   */
/*              address                         Device address                                  */
/*              descriptor                      Head address in the area to store discriptor    */
/*                                              information                                     */
/*              pfnCallbackFunc                 Callback function at the end of execution       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_GetDeviceDescriptor( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x08, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* Create request */
    
    /* setup data */
    
    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x08, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_DEV2HOST
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_GET_DESCRIPTOR;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = GRP_USBD_DESC_DEVICE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = GRP_USBD_DEVICE_DESC_SIZE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;

    /* other data */
    ptIrpPtr->pucBufferPtr   = (grp_u8 *)ptReq->pvDescriptor;
    ptIrpPtr->ulBufferLength = GRP_USBD_DEVICE_DESC_SIZE;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_IN;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpStdDeviceRequest;

    _TRACE_USBC_USBD_(0x08, 0x00, F_END);

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetDeviceInfo                                                          */
/*                                                                                              */
/* DESCRIPTION: Getting Device Information                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/* OUTPUT     : ptDevInfo                       Device information                              */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Address error                                   */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_GetDeviceInfo( grp_u16 usDevId, grp_usbdi_device_info *ptDevInfo)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x09, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x09, 0x01, F_END);
        return lStatus;
    }
    
    /* copy device information */
    ptDevInfo->usUsbDevId   = l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.usUsbDevId;
    ptDevInfo->ucHubAddr    = l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.ucHubAddr;
    ptDevInfo->ucPortNum    = l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.ucPortNum;
    ptDevInfo->ucPortInfo   = l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.ucPortInfo;
    ptDevInfo->usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;
    
    _TRACE_USBC_USBD_(0x09, 0x00, F_END);

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetStatus                                                              */
/*                                                                                              */
/* DESCRIPTION: Getting Status                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device request           */
/*              (Member used)                                                                   */
/*              usUsbDevId                      Device address                                  */
/*              bmRequestRecipient              Characteristics of request                      */
/*              ucInterface                     Interface number (if use)                       */
/*              ucEndpoint                      Endpoint number (if use)                        */
/*              pfnCallbackFunc                 Callback function at the end of execution       */
/* OUTPUT     : ptReq                           Structure for standard device request           */
/*              (Output member)                                                                 */
/*              retStatus                       Status information                              */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_GetStatus( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x0a, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* Create request */

    /* setup data */
    
    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x0a, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Get Status buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucBufferPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x0a, 0x02, F_END);
        /* Release setup data buffer to usb communication buffer */
        grp_cmem_BlkRel(ptIrpPtr->pucSetupPtr);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE)   = (grp_u8)(GRP_USBD_TYPE_DEV2HOST
                                                                        |          GRP_USBD_TYPE_STANDARD
                                                                        |          (ptReq->bmRequestRecipient & 0x03));
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)        = GRP_USBD_GET_STATUS;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)      = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)     = 0;

    if( ptReq->bmRequestRecipient == GRP_USBD_INTERFACE ){
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)  = ptReq->ucInterface;
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH) = 0;
    }
    else if( ptReq->bmRequestRecipient == GRP_USBD_ENDPOINT ){
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)  = ptReq->ucEndpoint;
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH) = 0;
    }
    else {
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)  = 0;
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH) = 0;
    }

    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)     = GRP_USBD_DREQ_GET_STAT_DATALEN;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)    = 0;
    
    /* other data */
    ptIrpPtr->ulBufferLength = GRP_USBD_DREQ_GET_STAT_DATALEN;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_IN;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpGetStatus;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x0a, 0x03, F_END);
        /* Release setup data buffer to usb communication buffer */
        grp_cmem_BlkRel(ptIrpPtr->pucBufferPtr);
    }

    _TRACE_USBC_USBD_(0x0a, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetStringDescriptor                                                    */
/*                                                                                              */
/* DESCRIPTION: Get string descriptor                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device request           */
/*              (Member used)                                                                   */
/*              usUsbDevId                      Device address                                  */
/*              ucIndex                         Descriptor index                                */
/*              pvDescriptor                    Head address in the area to store string        */
/*                                              descriptor                                      */
/*              usLangID                        Langedgh ID                                     */
/*              pfnCallbackFunc                 Callback function at the end of execution       */
/*              ulSize                          String descriptor area size                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_GetStringDescriptor( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x0b, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* Create request */
    /* setup data */
    
    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x0b, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }
    
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_DEV2HOST
                                                                      |  GRP_USBD_TYPE_STANDARD
                                                                      |  GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_GET_DESCRIPTOR;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = ptReq->ucIndex;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = GRP_USBD_DESC_STRING;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = (grp_u8)(ptReq->usLangID & 0x00FF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = (grp_u8)((ptReq->usLangID & 0xFF00) >> 8);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = (grp_u8)(ptReq->ulSize & 0x00FF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = (grp_u8)((ptReq->ulSize & 0xFF00) >> 8);

    /* other data */
    ptIrpPtr->pucBufferPtr   = (grp_u8 *)ptReq->pvDescriptor;
    ptIrpPtr->ulBufferLength = ptReq->ulSize;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_IN;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpStdDeviceRequest;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x0b, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_InterfaceOpen                                                          */
/*                                                                                              */
/* DESCRIPTION: This function opens all pipes belonging to the specified interface              */
/*              If other interface has been opened, close the interface and open the specified  */
/*              interface                                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptOper                          Structure for operation of pipes                */
/*              (Member used)                                                                   */
/*                  usUsbDevId                  Device Id to be set                             */
/*                  interface                   Interface to be set                             */
/*                  pipe                        Head address of pipe handler of pipe            */
/*                                              corresponding to the first endpoint pertaining  */
/*                                              to interface                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_INVALIED_IF            Invalid device number                           */
/*              GRP_USBD_NO_BANDWIDTH           Not enough communication bandwidth in order to  */
/*                                              switch interface                                */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_InterfaceOpen( grp_usbdi_pipe_operate *ptOper)
{
grp_usbdi_if_desc               *ptOpenifDesPtr;
grp_usbdi_descriptor_info       tDesc;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x0c, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptOper->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x0c, 0x01, F_END);
        return lStatus;
    }

    /* Configuration */
    if( l_tUSBD_CB.atDevTable[ucAddr].ulConfNum == 0 ){
        _TRACE_USBC_USBD_(0x0c, 0x02, F_END);
        return GRP_USBD_CONFIGURATION_ERROR;
    }

    /* Interface */
    if( (l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucState != GRP_USBD_STATE_BLANK)
     && (l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucState != GRP_USBD_STATE_CONNECT) ){
        _TRACE_USBC_USBD_(0x0c, 0x04, F_END);
        return GRP_USBD_CANNOT_OPERATE;
    }

    /* Get Configuration Descriptor */
    lStatus = grp_cnfsft_GetDescFromUsbDevId( ptOper->usUsbDevId,
                                              (void **)GRP_USB_NULL,
                                              &ptOper->pvDescriptor );
    if( lStatus != GRP_CNFSFT_OK ){
        _TRACE_USBC_USBD_(0x0c, 0x05, F_END);
        return lStatus;
    }

    /* Check bandwidth */
    tDesc.ucInterface = ptOper->ucInterface;
    tDesc.ucAlternate = l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucAltNum;
    tDesc.pvDesc      = ptOper->pvDescriptor;
    tDesc.ulSize      = 
          (grp_u16)(((grp_usbdi_config_desc_b *)ptOper->pvDescriptor)->wTotalLength_Low)
        | (grp_u16)((((grp_usbdi_config_desc_b *)ptOper->pvDescriptor)->wTotalLength_High) << 8);

    lStatus = _grp_usbd_IFBandwidthCheck( ucAddr, &tDesc );
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x0c, 0x06, F_END);
        return GRP_USBD_NO_BANDWIDTH;
    }

    /* Get the specified interface descriptor (open) */
    lStatus = _grp_usbd_GetIfDescptr( ptOper->pvDescriptor,
                                      ptOper->ucInterface,
                                      l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucAltNum,
                                      &ptOpenifDesPtr);
    if( lStatus == GRP_USBD_OK ){
        if( GRP_USBD_OK != grp_usbd_LockOpen( ) ){
            _TRACE_USBC_USBD_(0x0c, 0x07, F_END);
            return GRP_USBD_OS_RELATED_ERROR;
        }

        /* Device state */
        if( (l_tUSBD_CB.atDevTable[ucAddr].iPortState != GRP_USBD_STATE_OPEN)
         && (l_tUSBD_CB.atDevTable[ucAddr].iPortState != GRP_USBD_STATE_CONNECT) ){
            _TRACE_USBC_USBD_(0x0c, 0x02, 0x02);
            lStatus = GRP_USBD_CANNOT_OPERATE;
        }

        /* Open all pipes */
        if( lStatus == GRP_USBD_OK ){
            lStatus = _grp_usbd_OpenInterfaceAllPipes(ptOper,ptOpenifDesPtr);
            if( lStatus != GRP_USBD_OK ){
                _TRACE_USBC_USBD_(0x0c, 0x03, 0x03);
                _grp_usbd_CloseInterfaceAllPipes(ptOper,ptOpenifDesPtr);
            }
            else {
                l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_OPEN;
                l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucState = GRP_USBD_STATE_OPEN;
            }
        }

        if( GRP_USBD_OK != grp_usbd_UnlockOpen( ) ){
            _TRACE_USBC_USBD_(0x0c, 0x09, F_END);
            return GRP_USBD_OS_RELATED_ERROR;
        }
    }

    _TRACE_USBC_USBD_(0x0c, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_InterfaceClose                                                         */
/*                                                                                              */
/* DESCRIPTION: This function closes all pipes belonging to the specified interface             */
/*              If other interface has been not opened, do not at all.                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptOper                          Structure for operation of pipes                */
/*              (Member used)                                                                   */
/*                  usUsbDevId                  Device Id to be set                             */
/*                  interface                   Interface to be set                             */
/*                  pipe                        Head address of pipe handler of pipe            */
/*                                              corresponding to the first endpoint pertaining  */
/*                                              to interface                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_INVALIED_IF            Invalid interface number                        */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_InterfaceClose( grp_usbdi_pipe_operate *ptOper)
{
grp_usbdi_if_desc               *ptCloseifDesPtr = GRP_USB_NULL;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x0d, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptOper->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x0d, 0x01, F_END);
        return lStatus;
    }

    /* Configuration */
    if( l_tUSBD_CB.atDevTable[ucAddr].ulConfNum == 0 ){
        _TRACE_USBC_USBD_(0x0d, 0x02, F_END);
        return GRP_USBD_CONFIGURATION_ERROR;
    }

    /* Get Configuration Descriptor */
    lStatus = grp_cnfsft_GetDescFromUsbDevId( ptOper->usUsbDevId,
                                              (void **)GRP_USB_NULL,
                                              &ptOper->pvDescriptor );
    if( lStatus == GRP_CNFSFT_OK ){
        lStatus = _grp_usbd_GetIfDescptr( ptOper->pvDescriptor,
                                          ptOper->ucInterface,
                                          l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucAltNum,
                                          &ptCloseifDesPtr);
    }
    else {
        lStatus = GRP_USBD_ILLEGAL_ERROR;
    }

    if( lStatus != GRP_USBD_OK ){
        ptCloseifDesPtr = GRP_USB_NULL;
        lStatus         = GRP_USBD_OK;
    }

    if( GRP_USBD_OK != grp_usbd_LockOpen( ) ){
        _TRACE_USBC_USBD_(0x0d, 0x04, F_END);
        return GRP_USBD_OS_RELATED_ERROR;
    }

    /* Device state */
    if( (l_tUSBD_CB.atDevTable[ucAddr].iPortState != GRP_USBD_STATE_OPEN)
     && (l_tUSBD_CB.atDevTable[ucAddr].iPortState != GRP_USBD_STATE_DISCONNECT) ){
        _TRACE_USBC_USBD_(0x0d, 0x02, 0x02);
        lStatus = GRP_USBD_CANNOT_OPERATE;
    }

    /* If other alternate setting number has been registered already, close the alternate setting interface */
    if( lStatus == GRP_USBD_OK ){
        /* Interface */
        if( l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucState != GRP_USBD_STATE_OPEN ){
            l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucState = GRP_USBD_STATE_CONNECT;
        }
        else if( l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucState != GRP_USBD_STATE_DISCONNECT ){
            l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptOper->ucInterface].ucState = GRP_USBD_STATE_BLANK;
        }
        else {
            if( GRP_USBD_OK != grp_usbd_UnlockOpen() ){
                _TRACE_USBC_USBD_(0x0d, 0x03, F_END);
                return GRP_USBD_OS_RELATED_ERROR;
            }
            _TRACE_USBC_USBD_(0x0d, 0x04, F_END);
            return GRP_USBD_CANNOT_OPERATE;
        }

        _grp_usbd_CloseInterfaceAllPipes(ptOper,ptCloseifDesPtr);
    }

    if( GRP_USBD_OK != grp_usbd_UnlockOpen( ) ){
        _TRACE_USBC_USBD_(0x0d, 0x05, F_END);
        return GRP_USBD_OS_RELATED_ERROR;
    }

    _TRACE_USBC_USBD_(0x0d, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_NormalRequest                                                          */
/*                                                                                              */
/* DESCRIPTION: USB transfer request                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Communication request                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalid pipe information                        */
/*              GRP_USBD_PIPE_HALT_ERROR        Pipe HALT                                       */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_NormalRequest( grp_usbdi_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x0e, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->ptPipe->usUsbDevId));
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptReq->ptPipe,GRP_USBD_INVALIED_PH);
    /* Pipe HALT check */
    USBD_MACRO_PIPE_HALT_CHECK(ptReq->ptPipe,GRP_USBD_PIPE_HALT_ERROR);
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->ptPipe->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x0e, 0x01, F_END);
        return lStatus;
    }

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* Generate send packet */
    ptIrpPtr->ptEndpoint     = &ptReq->ptPipe->tEndpoint;
    ptIrpPtr->pucBufferPtr   = ptReq->pucBuffer;
    ptIrpPtr->ulBufferLength = ptReq->ulBufferLength;
    ptIrpPtr->iShortXferOK   = ptReq->iShortXferOK;
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpNormalRequest;
    ptIrpPtr->iRefCon        = 0;
    ptIrpPtr->pvUrbPtr       = (void *)ptReq;

    /* set host controller index number */
    ptIrpPtr->tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

#ifdef GRP_USB_HOST_USE_CTRL_NML_PIPE
    /* Copy the buffer for SETUP phase if it is the control transfer pipe */
    /* Set the transfer direction */
    if( ptReq->ptPipe->iTransferMode == GRP_USBD_CONTROL ){
        ptIrpPtr->pucSetupPtr = ptReq->pucSetup;
        ptIrpPtr->ulXferinfo  = (ptReq->ucTransferDirection == GRP_USBD_TX_OUT) ? GRP_USBD_TX_OUT : GRP_USBD_TX_IN;
    }
#endif  /* GRP_USB_HOST_USE_CTRL_NML_PIPE */

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
    /* Set start frame and control flag in case of isochronous transfer pipe */
    if( ptReq->ptPipe->iTransferMode == GRP_USBD_ISOCHRONOUS ){
        /* Transfer execution flag ON ? */
        if (ptReq->iIsoImmediatelyFlag == GRP_USB_TRUE) {
            ptIrpPtr->iImmediate = GRP_USB_TRUE;
        }
        else {
            ptIrpPtr->iImmediate = GRP_USB_FALSE;
            ptIrpPtr->usFrameNo = (grp_u16)ptReq->ulIsoStartFrame;
        }

        /* set parameters */
        ptIrpPtr->ulErrorCnt    = 0;
        ptIrpPtr->ptIsoStatus   = ptReq->ptIsoStatus;
        ptIrpPtr->ulIsoNum      = ptReq->ulIsoStatusNum;
        ptIrpPtr->ulIsoIndexM   = 0;
        ptIrpPtr->ulIsoIndexC   = 0;

        switch (ptReq->ulIsoBufferMode) {
        case GRP_USBD_ISO_BUF_WHOLE_MODE:       /* NO BRAEK */
        case GRP_USBD_ISO_BUF_ADDRESS_MODE:     /* NO BRAEK */
        case GRP_USBD_ISO_BUF_NATURAL_MODE:     /* NO BRAEK */
        case GRP_USBD_ISO_BUF_ASSIGN_MODE:      /* NO BRAEK */
        case GRP_USBD_ISO_BUF_COUNT_MODE:       /* NO BRAEK */
        case GRP_USBD_ISO_BUF_PACKED_MODE:
            ptIrpPtr->ulIsoMode = ptReq->ulIsoBufferMode;
            break;

        default:
            ptIrpPtr->ulIsoMode = GRP_USBD_ISO_BUF_WHOLE_MODE;
            break;
        }

        /* Make a transfer request to Host controller driver */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcItrRun(ptIrpPtr);
    }
    else
#endif
    {
        /* Make a transfer request to Host controller driver */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcTrRun(ptIrpPtr);
    }


    _TRACE_USBC_USBD_(0x0e, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_NormalRequestCancel                                                    */
/*                                                                                              */
/* DESCRIPTION: USB transfer cancel request                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Communication Cancel request                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalid pipe information                        */
/*              GRP_USBD_ALREADY_XFER           Transfer started and end                        */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_NormalRequestCancel( grp_usbdi_request *ptReq)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x0f, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptReq->ptPipe,GRP_USBD_INVALIED_PH);
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->ptPipe->usUsbDevId, &ucAddr);
    if(lStatus != GRP_USBD_OK) {
        _TRACE_USBC_USBD_(0x0f, 0x01, F_END);
        return lStatus;
    }

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
    /* Make a request for deleting request to lower driver */
    if( ptReq->ptPipe->iTransferMode == GRP_USBD_ISOCHRONOUS ){
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcItrDel(&ptReq->tIrp);
    }
    else
#endif
    {
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcTrDel(&ptReq->tIrp);
    }

    _TRACE_USBC_USBD_(0x0f, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_NotifyEvent                                                            */
/*                                                                                              */
/* DESCRIPTION: Notify host controller and hub event                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usHcIndexNum                    Host controller index number                    */
/*              usDevId                         Device identifier                               */
/*              iEvent                          Host controller and hub event                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_NotifyEvent( grp_u16 usHcIndexNum, grp_u16 usDevId, grp_si iEvent)
{
grp_si                          iHcNum;
grp_si                          i;
grp_s32                         lStatus;
grp_u16                         usPortEvent;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x10, 0x00, 0x00);

    /* search host controller data */
    iHcNum = _grp_usbd_SearchHcidx(usHcIndexNum);
    if( iHcNum < 0 ){
        _TRACE_USBC_USBD_(0x10, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x10, 0x01, F_END);
        return lStatus;
    }

    /* Change status */
    switch( iEvent ){
    case GRP_USBD_HC_HALT_EVENT:
        usPortEvent                                 = GRP_CNFSFT_DEVICE_DISABLED;
        l_tUSBD_CB.atHcData[iHcNum].ucHcState       = GRP_USBD_HC_HALTED;
        break;

    case GRP_USBD_HC_RESUME_EVENT:
        usPortEvent                                 = GRP_CNFSFT_DEVICE_ENABLED;
        l_tUSBD_CB.atHcData[iHcNum].ucHcState       = GRP_USBD_HC_ACTIVE;
        break;

    case GRP_USBD_PORT_HALT_EVENT:
        usPortEvent                                 = GRP_CNFSFT_DEVICE_DISABLED;
        l_tUSBD_CB.atDevTable[ucAddr].iPortState    = GRP_USBD_STATE_HALTED;
        break;

    case GRP_USBD_PORT_RESUME_EVENT:
        usPortEvent                                 = GRP_CNFSFT_DEVICE_ENABLED;
/*      l_tUSBD_CB.atDevTable[ucAddr].iPortState    = GRP_USBD_STATE_CONNECT;*/                     /* V1.04 {  */
        if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_HIBERNATE) {
            l_tUSBD_CB.atDevTable[ucAddr].iPortState  = GRP_USBD_STATE_CONNECT;
        }
        else if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_SUSPEND) {
            l_tUSBD_CB.atDevTable[ucAddr].iPortState  = GRP_USBD_STATE_OPEN;
        }                                                                                           /* V1.04 }  */
        break;

    default:
        return GRP_USBD_ILLEGAL_ERROR;              /* DIRECT RETURN */
    }

    /* Notify Configuring software */
    if( (iEvent == GRP_USBD_HC_HALT_EVENT) || (iEvent == GRP_USBD_HC_RESUME_EVENT) ){
        for( i = GRP_USBD_DEVICE_BASE_ADDRESS;i < GRP_USBD_HOST_MAX_DEVICE;i++ ){
            /* Check address table */
            if( l_tUSBD_CB.ausDevIdTable[i] != GRP_USBD_DEVID_NO_ASIGNED ){
                /* check host controller index number */
                if( l_tUSBD_CB.atDevTable[i].ptHcIndex->usHcIndexNum == usHcIndexNum ){
                    /* Notify event all devices. Right? ymd */
                    _grp_usbd_GetDevId( (grp_u8)i, &usDevId );
                    grp_cnfsft_ConfigSoftware( usPortEvent, usDevId );
                }
            }
        }
    }
    else {
        grp_cnfsft_ConfigSoftware( usPortEvent, usDevId );
    }

    _TRACE_USBC_USBD_(0x10, 0x00, F_END);

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SearchEpDescriptor                                                     */
/*                                                                                              */
/* DESCRIPTION: Get endpoint descriptor by analyzing configuration descriptor                   */
/*              (This function will work for 16 interfaces and 16 alternatives)                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDesInfo                       Descriptor analysis information structure       */
/*              ptEpDesPtr                      Head address in the area where endpoint         */
/*                                              descriptor to be got is stored                  */
/*                                                                                              */
/*              (About descriptor analysis information structure)                               */
/*              interface                       Interface number                                */
/*              ucAlternate                     Alternative set number                          */
/*              ucEndpoint                      endpoint number                                 */
/*                                              (0x1 - 0xf:OUT 0x81 - 0x8f:IN)                  */
/*              desc                            Configuration descriptor for analysis           */
/*              ulSize                          Configuration Descriptor Size                   */
/* OUTPUT     : ptEpDesPtr                      endpoint descriptor                             */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Detection complete                              */
/*              GRP_USBD_INVALIED_IF            Invalid interface number                        */
/*              GRP_USBD_INVALIED_EP            Set invalid endpoint number                     */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SearchEpDescriptor( grp_usbdi_descriptor_info *ptDesInfo, grp_usbdi_ep_desc *ptEpDesPtr)
{
grp_usbdi_if_desc               *ptIfDesPtr;
grp_usbdi_ep_desc               *ptEpDes2Ptr;
grp_usbdi_ep_desc               *ptNextEpPtr;
grp_s32                         lStatus;
grp_s32                         lRetStatus = GRP_USBD_INVALIED_EP;
grp_u8                          ucEp = 0;

    _TRACE_USBC_USBD_(0x11, 0x00, 0x00);
    /* Search interface descriptor */
    lStatus = _grp_usbd_GetIfDescptr( (grp_usbdi_config_desc *)ptDesInfo->pvDesc,
                                       ptDesInfo->ucInterface,
                                       ptDesInfo->ucAlternate,
                                       &ptIfDesPtr);
    if( lStatus == GRP_USBD_OK ){
        /* Search first endpoint descriptor */
        lStatus = _grp_usbd_GetFirstEpDescptr( (grp_usbdi_config_desc *)ptDesInfo->pvDesc,
                                                ptIfDesPtr,
                                                &ptEpDes2Ptr,
                                                (void **)&ptNextEpPtr );
        if( lStatus == GRP_USBD_OK ){
            /* check endpoint number */
            if( ptEpDes2Ptr->bEndpointAddress == ptDesInfo->ucEndpoint ){
                /* Copy endpoint descriptor */
                grp_std_memcpy((void *)ptEpDesPtr,(void *)ptEpDes2Ptr,sizeof(grp_usbdi_ep_desc));
                lRetStatus = GRP_USBD_OK;
            }
            else {
                for( ucEp = 1; ucEp < ptIfDesPtr->bNumEndpoints; ucEp++ ){
                    /* Get next endpoint descriptor */
                    lStatus = _grp_usbd_GetNextEpDescptr( (grp_usbdi_config_desc *)ptDesInfo->pvDesc,
                                                           ptNextEpPtr,
                                                           &ptEpDes2Ptr,
                                                           (void **)&ptNextEpPtr );
                    if ( lStatus == GRP_USBD_OK ){
                        /* check endpoint number */
                        if( ptEpDes2Ptr->bEndpointAddress == ptDesInfo->ucEndpoint ){
                            /* Copy endpoint descriptor */
                            grp_std_memcpy((void *)ptEpDesPtr,(void *)ptEpDes2Ptr,sizeof(grp_usbdi_ep_desc));
                            lRetStatus = GRP_USBD_OK;
                            break;
                        }
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }
    else {
        lRetStatus = GRP_USBD_INVALIED_IF;
    }

    _TRACE_USBC_USBD_(0x11, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SearchIfDescriptor                                                     */
/*                                                                                              */
/* DESCRIPTION: Get after anlyzing interface descriptor among configuration descriptor          */
/*              (This function can handle 16 interfaces and 16 alternatives)                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDesInfo                       Descriptor analysis information structure       */
/*              ptIfDesPtr                      Head address in the area to store interface     */
/*                                              descriptor                                      */
/*                                                                                              */
/*              (About descriptor analysis information structure)                               */
/*              interface                       Interface number                                */
/*              ucAlternate                     Alternative set number                          */
/*              desc                            Configuration descriptor for analysis           */
/*              ulSize                          Configuration Descriptor Size                   */
/* OUTPUT     : ptIfDesPtr                      Interface descriptor                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Detection complete                              */
/*              GRP_USBD_INVALIED_IF            Invalid interface number                        */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SearchIfDescriptor( grp_usbdi_descriptor_info *ptDesInfo, grp_usbdi_if_desc *ptIfDesPtr)
{
grp_usbdi_if_desc               *ptIfDes2Ptr;
grp_s32                         lStatus;
grp_s32                         lRetStatus = GRP_USBD_OK;

    _TRACE_USBC_USBD_(0x12, 0x00, 0x00);

    /* search interface descriptor */
    lStatus = _grp_usbd_GetIfDescptr( (grp_usbdi_config_desc *)ptDesInfo->pvDesc,
                                       ptDesInfo->ucInterface,
                                       ptDesInfo->ucAlternate,
                                       &ptIfDes2Ptr);
    if( lStatus == GRP_USBD_OK ){
        grp_std_memcpy((void *)ptIfDesPtr,(void *)ptIfDes2Ptr,GRP_USBD_INTERFACE_DESC_SIZE);
    }
    else {
        lRetStatus = GRP_USBD_INVALIED_IF;
    }

    _TRACE_USBC_USBD_(0x12, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_PipeAbort                                                              */
/*                                                                                              */
/* DESCRIPTION: Abort pipe                                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPipe                          Pipe handler of the pipe to be aborted          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_PipeAbort( grp_usbdi_pipe *ptPipe)
{
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x13, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptPipe,GRP_USBD_INVALIED_PH);
#endif

    _TRACE_USBC_USBD_(0x13, 0x00, F_END);

    /* Delete URB */
    lStatus = _grp_usbd_AllDeleteUrb(ptPipe);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_PipeActive                                                             */
/*                                                                                              */
/* DESCRIPTION: Activate Pipe                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              ptPipe                          Pipe handler of the pipe to be activated        */
/*              pfnCallbackFunc                 Excecute when pipe is activated                 */
/*                                              Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_PIPE_ACTIVE_FAIL       Failure to activate pipe                        */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_PipeActive( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x14, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->ptPipe->usUsbDevId));
    /* Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->ptPipe->usUsbDevId));
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptReq->ptPipe,GRP_USBD_INVALIED_PH);
#endif

    /* Copy Device Identifier */
    ptReq->usUsbDevId = ptReq->ptPipe->usUsbDevId;

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x14, 0x01, F_END);
        return lStatus;
    }

    /* Make an activate request to the lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpActive(&ptReq->ptPipe->tEndpoint);
    if( lStatus != GRP_HCDI_OK ){
        _TRACE_USBC_USBD_(0x14, 0x02, F_END);
        return GRP_USBD_PIPE_ACTIVE_FAIL;
    }

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x14, 0x03, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_ENDPOINT);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_CLEAR_FEATURE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = GRP_USBD_ENDPOINT_HALT;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = (grp_u8)(ptReq->ptPipe->ucEndpointNumber
                                                                               & GRP_USBD_ED_BEPADDRESS_MASK);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;
    if ( ptReq->ptPipe->ucTransferDirection == GRP_USBD_TX_IN ){
        /* Set IN direction bit. */
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW) |= (grp_u8)GRP_USBD_ED_BEPADDRESS_PID_BIT;
    }

    /* other data */
    ptIrpPtr->ulBufferLength = 0;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpPipeActive;

    _TRACE_USBC_USBD_(0x14, 0x00, F_END);

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_PipeCheck                                                              */
/*                                                                                              */
/* DESCRIPTION: Pipe handler status check                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPipe                          Pipe handler                                    */
/*              pStatus                         Head address of the area to store the current   */
/*                                              pipe status                                     */
/* OUTPUT     : pStatus                         Pipe status                                     */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_PipeCheck( grp_usbdi_pipe *ptPipe, grp_si *piStatus)
{
    _TRACE_USBC_USBD_(0x15, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptPipe,GRP_USBD_INVALIED_PH);
#endif

    *piStatus = ptPipe->iStatus;

    _TRACE_USBC_USBD_(0x15, 0x00, F_END);

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_PipeClose                                                              */
/*                                                                                              */
/* DESCRIPTION: Close communication pipe                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPipe                          Pipe handler                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_PipeClose( grp_usbdi_pipe *ptPipe)
{
grp_s32                         lStatus;
grp_s32                         lRetStatus = GRP_USBD_OK;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x16, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptPipe,GRP_USBD_INVALIED_PH);
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptPipe->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x16, 0x01, F_END);
        return lStatus;
    }

    /* Check already closed. */
    if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[ptPipe->ucTransferDirection][ptPipe->ucEndpointNumber]
     == (grp_usbdi_pipe *)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x16, 0x02, F_END);
        lRetStatus = GRP_USBD_ALREADY_OPERATED;
    }

    /* Make a request to lower host controoler driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpClose(&ptPipe->tEndpoint);
    /* Status check */
    if( lStatus == GRP_HCDI_OK ){
        /* Process after Success */

        /* Detach pipe information */
        l_tUSBD_CB.atDevTable[ucAddr].patPipe[ptPipe->ucTransferDirection][ptPipe->ucEndpointNumber] = (grp_usbdi_pipe *)GRP_USB_NULL;

        if( lRetStatus != GRP_USBD_ALREADY_OPERATED ){
            /* bandwidth release */
            l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulBandWidth -= ptPipe->ulBandWidth;

            if( l_tUSBD_CB.atDevTable[ucAddr].ulOpenPipeCount > 0 ){
                l_tUSBD_CB.atDevTable[ucAddr].ulOpenPipeCount -= 1;

                if( l_tUSBD_CB.atDevTable[ucAddr].ulOpenPipeCount == 0 ){
                    if( l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_OPEN ){
                        l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_CONNECT;
                    }
                    else if( l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_DISCONNECT ){
                        l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_BLANK;

                        /* Release address */
                        _grp_usbd_ReleaseDevId(ptPipe->usUsbDevId);
                    }
                    else {
                        lRetStatus = GRP_USBD_ILLEGAL_ERROR;
                        _TRACE_USBC_USBD_(0x16, 0x03, F_END);
                    }
                }
            }
        }
    }
    else {
        lRetStatus = GRP_USBD_ILLEGAL_ERROR;
    }

    _TRACE_USBC_USBD_(0x16, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_PipeHalt                                                               */
/*                                                                                              */
/* DESCRIPTION: Halt the pipe                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              ptPipe                          Pipe handler to make the pipe Halt              */
/*              pfnCallbackFunc                 Excecute when pipe Halt Callback function       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_PIPE_HALT_FAIL         Failure to pipe Halt                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_PipeHalt( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x17, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->ptPipe->usUsbDevId));
    /* Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->ptPipe->usUsbDevId));
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptReq->ptPipe,GRP_USBD_INVALIED_PH);
#endif

    /* Copy Device Identifier */
    ptReq->usUsbDevId = ptReq->ptPipe->usUsbDevId;

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x17, 0x01, F_END);
        return lStatus;
    }

    /* Make a pipe HALT request to the lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpHalt(&ptReq->ptPipe->tEndpoint);
    if( lStatus != GRP_HCDI_OK ){
        _TRACE_USBC_USBD_(0x17, 0x02, F_END);
        return GRP_USBD_PIPE_HALT_FAIL;
    }

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x17, 0x03, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }
    
    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_ENDPOINT);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_SET_FEATURE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = GRP_USBD_ENDPOINT_HALT;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = (grp_u8)(ptReq->ptPipe->ucEndpointNumber
                                                                               & GRP_USBD_ED_BEPADDRESS_MASK);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;
    if ( ptReq->ptPipe->ucTransferDirection == GRP_USBD_TX_IN ){
        /* Set IN direction bit. */
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW) |= (grp_u8)GRP_USBD_ED_BEPADDRESS_PID_BIT;
    }

    /* other data */
    ptIrpPtr->ulBufferLength = 0;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpPipeHalt;

    _TRACE_USBC_USBD_(0x17, 0x00, F_END);

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_PipeOpen                                                               */
/*                                                                                              */
/* DESCRIPTION: Open the communication pipe                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier( address + tag )              */
/*              ptEpDesPtr                      Endpoint descriptor information corresponding   */
/*                                              pipe to open                                    */
/*              ptPipe                          Head address in the area to store pipe handler  */
/*                                              to open                                         */
/* OUTPUT     : ptPipe                          Pipe handler                                    */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Device invalid address                          */
/*              GRP_USBD_NO_BANDWIDTH           Not enough commnunication                       */
/*                                              bandwidth                                       */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_PipeOpen( grp_u16 usDevId, grp_usbdi_ep_desc *ptEpDesPtr, grp_usbdi_pipe *ptPipe)
{
grp_hcdi_endpoint               *ptEpPtr;
grp_s32                         lStatus;
grp_u16                         usWMaxPacketSize = 0;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x18, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Check communication bandwidth */
    USBD_MACRO_BANDWIDTH_CHECK(_ID2AD(usDevId),ptEpDesPtr,l_tUSBD_CB.atDevTable[_ID2AD(usDevId)].ptHcIndex->ulBandWidth,GRP_USBD_NO_BANDWIDTH);
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x18, 0x01, F_END);
        return lStatus;
    }

    /* setup pointer */
    ptEpPtr = &ptPipe->tEndpoint;

    /* Set endpoint information from endpoint descriptor */
    ptEpPtr->ucAddress = ucAddr;
    ptEpPtr->ucEpNum   = (grp_u8)(ptEpDesPtr->bEndpointAddress & GRP_USBD_ED_BEPADDRESS_MASK);
    ptEpPtr->ucTxDir   = (grp_u8)(((ptEpDesPtr->bEndpointAddress & GRP_USBD_ED_BEPADDRESS_PID_BIT) == GRP_USBD_ED_BEPADDRESS_PID_BIT)
                        ? GRP_USBD_TX_IN : GRP_USBD_TX_OUT);
    ptEpPtr->ucTxMode  = (grp_u8)(ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK);

    /* Check already opened. */
    if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[ptEpPtr->ucTxDir][ptEpPtr->ucEpNum]
     != (grp_usbdi_pipe *)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x18, 0x02, F_END);
        return GRP_USBD_ALREADY_OPERATED;
    }

    /* Set the polling interval */
    ptEpPtr->ucTxInterval = _grp_usbd_GetEndpointInterval(l_tUSBD_CB.atDevTable[ucAddr].iSpeed,ptEpDesPtr);

    usWMaxPacketSize = (grp_u16)((grp_u16)(((grp_usbdi_ep_desc_b *)ptEpDesPtr)->wMaxPacketSize_Low)
                      |          (grp_u16)((((grp_usbdi_ep_desc_b *)ptEpDesPtr)->wMaxPacketSize_High) << 8));

    if( l_tUSBD_CB.atDevTable[ucAddr].iSpeed == GRP_USBD_HIGH_SPEED ){
        if( (ptEpPtr->ucTxMode == GRP_USBD_CONTROL) || (ptEpPtr->ucTxMode == GRP_USBD_BULK) ){
            /* Control or bulk transfer */
            /* Set Nak Rate */
            ptEpPtr->ucNakRate = ptEpDesPtr->bInterval;
        }
        else {
            /* Interrupt or isochronous transfer */

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
            /* The beginning microframe specification has not been supported yet now. */
            ptEpPtr->ucIsoStartMicroFrm = 0;
#endif /* GRP_USB_HOST_USE_ISOCHRONOUS */

            /* Set Microframe per Packet rate */
            if( (usWMaxPacketSize & GRP_USBD_ED_WMAXPKTSIZ_ADD_MASK) == GRP_USBD_ED_WMAXPKTSIZ_ADDTR_1 ){
                ptEpPtr->ucMicroPerPkt = GRP_USBD_1_ADD_TX_PER_FRAME;
            }
            else if( (usWMaxPacketSize & GRP_USBD_ED_WMAXPKTSIZ_ADD_MASK) == GRP_USBD_ED_WMAXPKTSIZ_ADDTR_2 ){
                ptEpPtr->ucMicroPerPkt = GRP_USBD_2_ADD_TX_PER_FRAME;
            }
            else {/* if( (usWMaxPacketSize & GRP_USBD_ED_WMAXPKTSIZ_ADD_MASK) == GRP_USBD_ED_WMAXPKTSIZ_ADDTR_0) */
                ptEpPtr->ucMicroPerPkt = GRP_USBD_NO_ADD_TX_PER_FRAME;
            }
        }
    }

    ptEpPtr->usMaxPacketSize     = (grp_u16)(usWMaxPacketSize & GRP_USBD_ED_WMAXPKTSIZ_MPS_MASK);
    ptEpPtr->ucTxSpeed           = (grp_u8)(l_tUSBD_CB.atDevTable[ucAddr].iSpeed);
    ptEpPtr->tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

    lStatus = _grp_usbd_SearchUSB20Hub(ucAddr,&ptEpPtr->ucHubAddress,&ptEpPtr->ucPortNum);
    if( lStatus != GRP_USBD_OK ){
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Make a request for endpoint open to lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpOpen(ptEpPtr);
    if( lStatus == GRP_HCDI_OK ){
        /* Afterward process of Success */

        /* Update pipe information */
        ptPipe->usUsbDevId          = usDevId;
        ptPipe->ucEndpointNumber    = ptEpPtr->ucEpNum;
        ptPipe->iTransferMode       = ptEpPtr->ucTxMode;
        ptPipe->ucTransferDirection = ptEpPtr->ucTxDir;
        ptPipe->ulInterval          = ptEpPtr->ucTxInterval;
        ptPipe->usMaxPacketSize     = ptEpPtr->usMaxPacketSize;
        ptPipe->iStatus             = GRP_USBD_PIPE_ACTIVE;             /* Pipe is active at Open */

        /* Register the pipe information into device management table */
        l_tUSBD_CB.atDevTable[ucAddr].patPipe[ptEpPtr->ucTxDir][ptEpPtr->ucEpNum] = ptPipe;

        if( l_tUSBD_CB.atDevTable[ucAddr].ulOpenPipeCount == 0 ){
            if( l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_CONNECT ){
                l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_OPEN;
            }
            else {
                lStatus = GRP_USBD_ILLEGAL_ERROR;
                _TRACE_USBC_USBD_(0x18, 0x02, 0x01);
            }
        }

        if( lStatus == GRP_USBD_OK ){
            l_tUSBD_CB.atDevTable[ucAddr].ulOpenPipeCount += 1;

            /* Allocate bandwidth for this pipe */
            ptPipe->ulBandWidth = _grp_usbd_CalcBandwidth(ucAddr,ptEpDesPtr);
            l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulBandWidth += ptPipe->ulBandWidth;
        }
    }
    else {
        lStatus = GRP_USBD_ILLEGAL_ERROR;
    }

    _TRACE_USBC_USBD_(0x18, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_PipeReset                                                              */
/*                                                                                              */
/* DESCRIPTION: Reset the pipe                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              ptPipe                          Pipe handler of pipe to reset                   */
/*              pfnCallbackFunc                 Callback function to be executed at pipe reset  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_PIPE_ACTIVE_FAIL       Failure to activate pipe                        */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_PipeReset( grp_usbdi_st_device_request *ptReq)
{
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x19, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptReq->ptPipe,GRP_USBD_INVALIED_PH);
#endif

    /* Delete all URB */
    lStatus = _grp_usbd_AllDeleteUrb(ptReq->ptPipe);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x19, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    _TRACE_USBC_USBD_(0x19, 0x00, F_END);

    /* Activate pipe */
    lStatus = grp_usbd_PipeActive(ptReq);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SearchDescriptor                                                       */
/*                                                                                              */
/* DESCRIPTION: This function finds the specified descriptor                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : *ptCnfDesPtr                    Pointer to all of configuration descriptor      */
/*              ucBDescriptorType               Descriptor type                                 */
/*              *pvStartDesPtr                  Pointer to a descriptor which starts reference  */
/* OUTPUT     : **ppvSearchDesPtr               Pointer to the found specified descriptor       */
/*                                              (if return value isn't GRP_USBD_OK, this value  */
/*                                              is invalid)                                     */
/*              **ppvNext2Ptr                   Pointer to the descriptor next to the found     */
/*                                              descriptor                                      */
/*                                              (if return value isn't GRP_USBD_OK, this value  */
/*                                              is invalid)                                     */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_NO_DESCRITOR           There isn't specified descriptor                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SearchDescriptor( grp_usbdi_config_desc *ptCnfDesPtr, grp_u8 ucBDescriptorType, void *pvStartDesPtr, void **ppvSearchDesPtr, void **ppvNext2Ptr )
{
void                            *pvTmpDesPtr;
grp_s32                         lRetStatus = GRP_USBD_NO_DESCRITOR;
grp_u32                         ulWTotalLength;

    _TRACE_USBC_USBD_(0x1a, 0x00, 0x00);

    /* set total length */
    ulWTotalLength = (grp_u32)((grp_u16)(( (grp_usbdi_config_desc_b *)ptCnfDesPtr)->wTotalLength_Low)
                             | (grp_u16)((((grp_usbdi_config_desc_b *)ptCnfDesPtr)->wTotalLength_High) << 8));

    /* Go to next descriptor */
    for( pvTmpDesPtr = pvStartDesPtr;
         (grp_u32)((grp_u32)pvTmpDesPtr - (grp_u32)ptCnfDesPtr) < ulWTotalLength;
         pvTmpDesPtr = (void *)((grp_u32)pvTmpDesPtr + (grp_u32)*(grp_u8 *)(pvTmpDesPtr) ) ){

        /* Is this a specified descriptor ? */
        if( *(grp_u8 *)((grp_u32)pvTmpDesPtr + 1) == ucBDescriptorType ){
            /* Get descriptor pointer */
            *ppvSearchDesPtr = pvTmpDesPtr;
            *ppvNext2Ptr     = (void *)((grp_u32)pvTmpDesPtr + (grp_u32)(*(grp_u8 *)pvTmpDesPtr));
            lRetStatus       = GRP_USBD_OK;
            break;
        }
    }

    _TRACE_USBC_USBD_(0x1a, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SetAddress                                                             */
/*                                                                                              */
/* DESCRIPTION: Allocate the address to the device                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              ucEp0size                       Maximum transfer size of endpoint 0             */
/*              address                         Area to store the allocated address             */
/*              pfnCallbackFunc                 Callback function to be executed after address  */
/*                                              allocation or transfer                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_TOO_MANY_DEVICE        Exceed maximum number of connected devices      */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SetAddress( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;
grp_u16                         usDevId;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x1b, 0x00, 0x00);

    /* Search the free device identifier ( address + tag ) */
    lStatus = _grp_usbd_SearchDevId(&usDevId);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x1b, 0x01, F_END);
        return lStatus;
    }

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x1b, 0x02, F_END);
        return lStatus;
    }

    /* Set 0 as defalut address */
    ptReq->usUsbDevId = GRP_USBD_DEFAULT_DEVID;

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x1b, 0x02, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_SET_ADDRESS;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = ucAddr;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;

    /* other data */
    ptIrpPtr->ulBufferLength = 0;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpSetaddress;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x1b, 0x01, 0x00);
        /* Release address */
        _grp_usbd_ReleaseDevId(usDevId);
    }

    _TRACE_USBC_USBD_(0x1b, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SetConfiguration                                                       */
/*                                                                                              */
/* DESCRIPTION: Set device configuration                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              address                         Device address to be configured                 */
/*              configuration                   Configuration number to set                     */
/*              pvDescriptor                    Configuration descriptor to set                 */
/*              pfnCallbackFunc                 Callback function to be executed                */
/*                                              after configuration setting                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_CONFIGURATION_ERROR    Configuration error                             */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SetConfiguration( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_usbdi_ep_desc               *ptEpDesPtr;
void                            *pvStartDesPtr;
void                            *pvNextDesPtr   = GRP_USB_NULL;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x1c, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x1c, 0x01, F_END);
        return lStatus;
    }

    /* Check the bandwidth of each endpoint */
    for( pvStartDesPtr = (void *)ptReq->pvDescriptor; ; pvStartDesPtr = pvNextDesPtr ){
        /* Get endpoint descriptor pointer */
        lStatus = grp_usbd_SearchDescriptor((grp_usbdi_config_desc *)ptReq->pvDescriptor,
                                            GRP_USBD_DESC_ENDPOINT,
                                            pvStartDesPtr,
                                            (void **)&ptEpDesPtr,
                                            &pvNextDesPtr);
        if( lStatus == GRP_USBD_OK ){
            /* Check the bandwidth */
            if( (GRP_USBD_HC_MAXBANDWIDTH - l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulBandWidth) < _grp_usbd_CalcBandwidth(ucAddr,ptEpDesPtr) ){
                _TRACE_USBC_USBD_(0x1c, 0x02, F_END);
                /* Bandwidth not enough error */
                return GRP_USBD_CONFIGURATION_ERROR;
            }
        }
        else {
            break;
        }
    }

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x1c, 0x03, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_SET_CONFIGURATION;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = ptReq->ucConfiguration;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;

    /* other data */
    ptIrpPtr->ulBufferLength = 0;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpSetconfiguration;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x1c, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SetFeature                                                             */
/*                                                                                              */
/* DESCRIPTION: Feature selector setting                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*               (Member used)                                                                  */
/*              usUsbDevId                      Device identifier to be set                     */
/*              ucFeatureSelector               Feature selector                                */
/*              ucEndpoint                      endpoint number                                 */
/*              pfnCallbackFunc                 Callback function after execution               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SetFeature( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;
grp_u8                          ucSelector = GRP_USBD_TYPE_DEVICE;

    _TRACE_USBC_USBD_(0x1d, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x1d, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    if( ptReq->ucFeatureSelector == GRP_USBD_ENDPOINT_HALT ){
        ucSelector = GRP_USBD_TYPE_ENDPOINT;
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)  = ptReq->ucEndpoint;
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH) = 0;
    }
    else {
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)  = 0;
        *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH) = 0;
    }

    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE)   = (grp_u8)(GRP_USBD_TYPE_HOST2DEV
                                                                         | GRP_USBD_TYPE_STANDARD
                                                                         | ucSelector);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)        = GRP_USBD_SET_FEATURE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)      = ptReq->ucFeatureSelector;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)     = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)     = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)    = 0;

    /* other data */
    ptIrpPtr->ulBufferLength = 0;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpStdDeviceRequest;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x1d, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SetInterface                                                           */
/*                                                                                              */
/* DESCRIPTION: This function sets interface                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device request           */
/*              (Member used)                                                                   */
/*              usUsbDevId                      Device identifier to be set                     */
/*              ucInterface                     Interface to be set                             */
/*              ptPipe                          Head address of pipe handler of pipe            */
/*                                              corresponding to the first endpoint pertaining  */
/*                                              to interface                                    */
/*              ucPipeNumber                    Number of endpoints pertaining to interface     */
/*              pvDescriptor                    Configuration descriptor                        */
/*                                              configured so that interface to be set is       */
/*                                              pertained                                       */
/*              ulSize                          Size of configuration descriptor                */
/*              pfnCallbackFunc                 Callback function after execution               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_NO_BANDWIDTH           Not enough bandwidth                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SetInterface( grp_usbdi_st_device_request *ptReq)
{
grp_usbdi_pipe_operate          tOper;
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;
grp_si                          iIfStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x1e, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x1e, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Exclusive control when pipe opened */
    if( GRP_USBD_OK != grp_usbd_LockOpen() ){
        _TRACE_USBC_USBD_(0x1e, 0x02, F_END);
        return GRP_USBD_OS_RELATED_ERROR;
    }

    iIfStatus = grp_usbd_GetInterfaceStatus( ptReq->usUsbDevId, ptReq->ucInterface );
    if( iIfStatus == GRP_USBD_STATE_BLANK ){
        iIfStatus = grp_usbd_GetDevicePortStatus( ptReq->usUsbDevId );
        if( iIfStatus != GRP_USBD_STATE_BLANK ){
            tOper.usUsbDevId   = ptReq->usUsbDevId;
            tOper.ucInterface  = ptReq->ucInterface;
            tOper.ucPipeNumber = 0; /* Only check the existence of the interface ( NOT pipes ). MUST be 0. */
            tOper.ptPipe       = GRP_USB_NULL;
            if( GRP_USBD_OK != grp_usbd_InterfaceCheck( &tOper, 0 ) ){
                if( GRP_USBD_OK != grp_usbd_UnlockOpen() ){
                    _TRACE_USBC_USBD_(0x1e, 0x03, F_END);
                    return GRP_USBD_OS_RELATED_ERROR;
                }

                _TRACE_USBC_USBD_(0x1e, 0x04, F_END);
                return GRP_USBD_CANNOT_OPERATE;
            }
        }
        else{
            if( GRP_USBD_OK != grp_usbd_UnlockOpen() ){
                _TRACE_USBC_USBD_(0x1e, 0x05, F_END);
                return GRP_USBD_OS_RELATED_ERROR;
            }
        }
    }
    else if( iIfStatus != GRP_USBD_STATE_CONNECT ){
        if( GRP_USBD_OK != grp_usbd_UnlockOpen() ){
            _TRACE_USBC_USBD_(0x1e, 0x06, F_END);
            return GRP_USBD_OS_RELATED_ERROR;
        }

        _TRACE_USBC_USBD_(0x1e, 0x07, F_END);
        return GRP_USBD_CANNOT_OPERATE;
    }

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        if( GRP_USBD_OK != grp_usbd_UnlockOpen() ){
            _TRACE_USBC_USBD_(0x1e, 0x08, F_END);
            return GRP_USBD_OS_RELATED_ERROR;
        }

        _TRACE_USBC_USBD_(0x1e, 0x09, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_INTERFACE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_REQ_SET_INTERFACE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = ptReq->ucAlternate;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = ptReq->ucInterface;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;

    /* other data */
    ptIrpPtr->ulBufferLength = 0;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpSetinterface;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);
    if( lStatus != GRP_USBD_OK ){
        if( GRP_USBD_OK != grp_usbd_UnlockOpen() ){
            _TRACE_USBC_USBD_(0x1e, 0x0A, F_END);
            return GRP_USBD_OS_RELATED_ERROR;
        }

        _TRACE_USBC_USBD_(0x1e, 0x08, 0x01);
    }

    _TRACE_USBC_USBD_(0x1e, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_UnSetConfiguration                                                     */
/*                                                                                              */
/* DESCRIPTION: Clear device configuration                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device request           */
/*              (Member used)                                                                   */
/*              address                         Device address to clear configuration           */
/*              pfnCallbackFunc                 Callback function executed after clearing the   */
/*                                              configuration                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_UnSetConfiguration( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x1f, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x1f, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_SET_CONFIGURATION;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;

    /* other data */
    ptIrpPtr->ulBufferLength = 0;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpUnsetconfiguration;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x1f, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_CheckFreeAddress                                                       */
/*                                                                                              */
/* DESCRIPTION: Check Free address.                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          No address.                                     */
/*              GRP_USBD_OS_RELATED_ERROR       OS related error                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_CheckFreeAddress(void)
{
grp_si                          i;
grp_s32                         lRetStatus = GRP_USBD_ILLEGAL_ERROR;

    _TRACE_USBC_USBD_(0x20, 0x00, 0x00);

    /* Exclusive control when address released */
    if( GRP_USBD_OK != grp_usbd_LockAddress() ){
        return GRP_USBD_OS_RELATED_ERROR;
    }

    /* Search address table */
    for( i = GRP_USBD_DEVICE_BASE_ADDRESS;i < GRP_USBD_HOST_MAX_DEVICE;i++ ){
        if( l_tUSBD_CB.ausDevIdTable[i] == GRP_USBD_DEVID_NO_ASIGNED ){
            lRetStatus = GRP_USBD_OK;
            break;
        }
    }

    /* Cancel exclusive control */
    grp_usbd_UnlockAddress();

    _TRACE_USBC_USBD_(0x20, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_Pipe0Abort                                                             */
/*                                                                                              */
/* DESCRIPTION: Abort pipe0                                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_Pipe0Abort( grp_u16 usDevId)
{
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x21, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
    /* Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(usDevId));
#endif

    _TRACE_USBC_USBD_(0x21, 0x00, 0x01);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x21, 0x01, F_END);
        return lStatus;
    }

    /* Select default pipe */
    ptPipe = &l_tUSBD_CB.atDevTable[ucAddr].tDefaultPipe;

    /* Make a request to the lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpFlash(&ptPipe->tEndpoint);
    if( lStatus != GRP_HCDI_OK ){
        lStatus = GRP_USBD_ILLEGAL_ERROR;
    }

    _TRACE_USBC_USBD_(0x21, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_StDeviceRequestCancel                                                  */
/*                                                                                              */
/* DESCRIPTION: Device request cancellation                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Communication request                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Address error                                   */
/*              GRP_USBD_ALREADY_XFER           Start/end transfer                              */
/*              GRP_USBD_ILLEGAL_ERROR          function error                                  */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_StDeviceRequestCancel( grp_usbdi_st_device_request *ptReq)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_( 0x22, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x22, 0x01, F_END);
        return lStatus;
    }

    /* Make a cancel request to lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcTrDel(&ptReq->tIrp);
    /* Error code check */
    if( lStatus == GRP_HCDI_OK ){
        /* Release setup data buffer to usb communication buffer */
        /* When the cancellation succeeds, the callback is not occured. */
        grp_cmem_BlkRel(ptReq->tIrp.pucSetupPtr);
    }

    _TRACE_USBC_USBD_(0x22, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetDevicePortStatus                                                    */
/*                                                                                              */
/* DESCRIPTION: Get device portstatus.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                             Device identifier                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : PortStatus                                                                      */
/*                                                                                              */
/************************************************************************************************/
grp_si grp_usbd_GetDevicePortStatus( grp_u16 usDevId)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_( 0x23, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        return GRP_USBD_STATE_UNKNOWN;
    }

    _TRACE_USBC_USBD_(0x23, 0x00, F_END);

    return (l_tUSBD_CB.atDevTable[ucAddr].iPortState);
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetInterfaceStatus                                                     */
/*                                                                                              */
/* DESCRIPTION: Get device portstatus.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                             Device identifier                           */
/*              ucIfNum                             Interface number                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : InterfaceStatus                                                                 */
/*                                                                                              */
/************************************************************************************************/
grp_si grp_usbd_GetInterfaceStatus( grp_u16 usDevId, grp_u8 ucIfNum )
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_( 0x24, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        return GRP_USBD_STATE_UNKNOWN;
    }

    _TRACE_USBC_USBD_(0x24, 0x00, F_END);

    return ((grp_si)l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ucIfNum].ucState);

} /* _grp_usbd_GetInterfaceStatus */

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SetConfigDescriptor                                                    */
/*                                                                                              */
/* DESCRIPTION: Configuration descriptor setting                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              ucConfiguration                 Configuration number                            */
/*              pvDescriptor                    Address of the area where descriptor            */
/*                                              information to be set is stored                 */
/*              ulSize                          Setting descriptor size                         */
/*              pfnCallbackFunc                 Callback function at the end of execution       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SetConfigDescriptor( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x25, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x25, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_SET_DESCRIPTOR;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = (grp_u8)(ptReq->ucConfiguration & 0xF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = GRP_USBD_DESC_CONFIGURATION;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = (grp_u8)(ptReq->ulSize & 0x00FF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = (grp_u8)((ptReq->ulSize & 0xFF00) >> 8);

    /* other data */
    ptIrpPtr->pucBufferPtr   = (grp_u8 *)ptReq->pvDescriptor;
    ptIrpPtr->ulBufferLength = ptReq->ulSize;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpStdDeviceRequest;;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x25, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SetDeviceDescriptor                                                    */
/*                                                                                              */
/* DESCRIPTION: Device descriptor setting                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              pvDescriptor                    Address of the area where descriptor            */
/*                                              information to be set is stored                 */
/*              pfnCallbackFunc                 Callback function at the end of execution       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SetDeviceDescriptor( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x26, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x26, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_SET_DESCRIPTOR;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = GRP_USBD_DESC_DEVICE;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = (grp_u8)(ptReq->ulSize & 0x00FF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = (grp_u8)((ptReq->ulSize & 0xFF00) >> 8);

    /* other data */
    ptIrpPtr->pucBufferPtr   = (grp_u8 *)ptReq->pvDescriptor;
    ptIrpPtr->ulBufferLength = ptReq->ulSize;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpStdDeviceRequest;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x26, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SetStringDescriptor                                                    */
/*                                                                                              */
/* DESCRIPTION: String Descriptor setting                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device request           */
/*              (Member used)                                                                   */
/*              ucIndex                         Descriptor index                                */
/*              pvDescriptor                    Address of the area where descriptor            */
/*                                              information to be set is stored                 */
/*              pfnCallbackFunc                 Callback function at the end of                 */
/*                                              execution                                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SetStringDescriptor( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x27, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet(GRP_CMEM_ID_USBD,(void **)&ptIrpPtr->pucSetupPtr);
    if( lStatus != GRP_CMEM_OK ){
        _TRACE_USBC_USBD_(0x27, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_HOST2DEV
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_DEVICE);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_SET_DESCRIPTOR;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = ptReq->ucIndex;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = GRP_USBD_DESC_STRING;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = (grp_u8)(ptReq->usLangID & 0x00FF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = (grp_u8)((ptReq->usLangID & 0xFF00) >> 8);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = (grp_u8)(ptReq->ulSize & 0x00FF);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = (grp_u8)((ptReq->ulSize & 0xFF00) >> 8);

    /* other data */
    ptIrpPtr->pucBufferPtr   = (grp_u8 *)ptReq->pvDescriptor;
    ptIrpPtr->ulBufferLength = ptReq->ulSize;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_OUT;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpStdDeviceRequest;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x27, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetIsochronousInfo                                                     */
/*                                                                                              */
/* DESCRIPTION: Buffer length and number of Status for 1ms frame in isochronous transfer.       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPipe                          Pipe handle for isochronous                     */
/*              pulBufferLen                    Address for getting buffer length               */
/*              pulStatusSz                     Address for getting status size                 */
/*              pucInterval                     Address for getting interval                    */
/* OUTPUT     : pulBufferLen                    Buffer Length in 1ms frame                      */
/*              pulStatusSz                     Status size in 1ms frame                        */
/*              pucInterval                     Number of interval in 1ms frame                 */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_GetIsochronousInfo( grp_usbdi_pipe *ptPipe, grp_u32 *pulBufferLen, grp_u32 *pulStatusSz, grp_u8 *pucInterval)
{
grp_u32                         ulNum;
grp_u32                         ulAlignMax;
grp_s32                         lStatus     = GRP_USBD_OK;
grp_u8                          ucFrame;

    _TRACE_USBC_USBD_(0x28, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptPipe->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptPipe->usUsbDevId));
#endif

    _TRACE_USBC_USBD_(0x28, 0x00, 0x01);

    if ((pulBufferLen == GRP_USB_NULL) || (pulStatusSz == GRP_USB_NULL)) {
        return GRP_USBD_ILLEGAL_ERROR;
    }

    ucFrame = (grp_u8)ptPipe->ulInterval;

    switch (ucFrame) {
    case GRP_USBD_INTERVAL_125us:
        ulNum = 8;
        ucFrame = GRP_USBD_INTERVAL_1ms;
        break;

    case GRP_USBD_INTERVAL_250us:
        ulNum = 4;
        ucFrame = GRP_USBD_INTERVAL_1ms;
        break;

    case GRP_USBD_INTERVAL_500us:
        ulNum = 2;
        ucFrame = GRP_USBD_INTERVAL_1ms;
        break;

    default:
        ulNum = 1;
        break;
    }

    ulAlignMax = ptPipe->usMaxPacketSize;
    if (ulAlignMax & 3) {
        ulAlignMax = (ulAlignMax & 0x7fc) + 4;  /* 4 byte align adjust */
    }
    *pulBufferLen = ulAlignMax * (ptPipe->tEndpoint.ucMicroPerPkt + 1) * ulNum;
    *pulStatusSz  = ulNum;

    if (pucInterval != GRP_USB_NULL) {
        *pucInterval = ucFrame;
    }

    _TRACE_USBC_USBD_(0x28, 0x00, F_END);

    return lStatus;

} /* GRUSB_USBD_Get_Isochronous_Info */

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
/************************************************************************************************/
/* FUNCTION   : grp_usbd_SynchFrame                                                             */
/*                                                                                              */
/* DESCRIPTION: This function is used though synchronizes the frame                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Structure for standard device request           */
/*              (Member used)                                                                   */
/*              usUsbDevId                      Device identifier to be set                     */
/*              ptPipe                          Pipe handler of pipe to synch                   */
/*              pfnCallbackFunc                 Callback function after execution               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_NO_BANDWIDTH           Not enough bandwidth                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SynchFrame( grp_usbdi_st_device_request *ptReq)
{
grp_hcdi_tr_request             *ptIrpPtr;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x29, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(ptReq->usUsbDevId));
    /* Device Connect check */
    USBD_MACRO_CONNECT_CHECK(_ID2AD(ptReq->usUsbDevId));
#endif

    /* setup pointer */
    ptIrpPtr = &ptReq->tIrp;

    /* setup data */

    /* Get setup buffer for usb comminucation buffer */
    lStatus = grp_cmem_BlkGet( GRP_CMEM_ID_USBD, (void **)&ptIrpPtr->pucSetupPtr);
    if (lStatus != GRP_CMEM_OK) {
        _TRACE_USBC_USBD_(0x29, 0x01, F_END);
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Create request */
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BMREQUESTTYPE) = (GRP_USBD_TYPE_DEV2HOST
                                                                       | GRP_USBD_TYPE_STANDARD
                                                                       | GRP_USBD_TYPE_ENDPOINT);
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_BREQUEST)      = GRP_USBD_REQ_SYNCH_FRAME;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW)    = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WVALUE_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_LOW)    = ptReq->ptPipe->ucEndpointNumber;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WINDEX_HIGH)   = 0;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_LOW)   = GRP_USBD_DREQ_SYNC_FRM_DATALEN;
    *(grp_u8 *)(ptIrpPtr->pucSetupPtr + GRP_USBD_SETUP_WLENGTH_HIGH)  = 0;

    /* other data */
    ptIrpPtr->ulBufferLength = GRP_USBD_DREQ_SYNC_FRM_DATALEN;
    ptIrpPtr->ulXferinfo     = GRP_USBD_TX_IN;

    /* Set callback function */
    ptIrpPtr->pfnCompFunc    = _grp_usbd_CmpSynchFrame;

    /* Make an internal device request */
    lStatus = _grp_usbd_InternalDeviceRequest(ptReq);

    _TRACE_USBC_USBD_(0x29, 0x00, F_END);
    return lStatus;
}
#endif  /* GRP_USB_HOST_USE_ISOCHRONOUS */

/************************************************************************************************/
/* FUNCTION   : grp_usbd_FrameWidthControl                                                      */
/*                                                                                              */
/* DESCRIPTION: Get and set framewidth                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*              iControl                        Framewidth control                              */
/*                                              0:Get 1:Set                                     */
/*              plFrameLength                   Get:Head pointer to obtain framewidth in the    */
/*                                                  area                                        */
/*                                              Set:Head pointer (-1 to +1) of the area where   */
/*                                                  framewidth                                  */
/* OUTPUT     : plFrameLength                   Get only:Framewidth                             */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_CLIANT        Invalid Master Client                           */
/*              GRP_USBD_INVALIED_PMTR          Parameter invalid                               */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend state                           */
/*              GRP_USBD_HOST_HALT              Host controller HALT state                      */
/*              GRP_USBD_NO_FUNCTION            No function                                     */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_FrameWidthControl( grp_u16 usDevId, grp_si iControl, grp_s32 *plFrameLength)
{
grp_hcdi_frame_control          tFrameReq;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x2a, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x2a, 0x01, F_END);
        return lStatus;
    }

    /* Master client check */
    if( ucAddr != l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucMasterCliantAddr ){
        _TRACE_USBC_USBD_(0x2a, 0x02, F_END);
        return GRP_USBD_INVALIED_CLIANT;
    }

    /* Parameter check */
    if( ((iControl != GRP_USBD_GET_FRAME_WIDTH) && (iControl != GRP_USBD_SET_FRAME_WIDTH))
     || ((iControl == GRP_USBD_SET_FRAME_WIDTH) && ((*plFrameLength < -1) || (*plFrameLength > 1))) ){
        /* parameter error */
        _TRACE_USBC_USBD_(0x2a, 0x03, F_END);
        return GRP_USBD_INVALIED_PMTR;
    }

    /* set host controller index number */
    tFrameReq.tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

    /* Get and set framewidth from lower driver */
    if( iControl == GRP_USBD_GET_FRAME_WIDTH ){
        /* Get framewidth */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcGetFrame(&tFrameReq);
        if( lStatus == GRP_HCDI_OK ){
            *plFrameLength = tFrameReq.lFrameWidth;
        }
    }
    else {
        tFrameReq.lFrameWidth = *plFrameLength;

        /* Set framewidth */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcSetFrame(&tFrameReq);
    }

    _TRACE_USBC_USBD_(0x2a, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_FrameNumberControl                                                     */
/*                                                                                              */
/* DESCRIPTION: Get and set frame number                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*              iControl                        Framewidth control                              */
/*                                              0:Get 1:Set                                     */
/*              plFrameNumber                   Get:Area head pointer to get                    */
/*                                                  frame number                                */
/*                                              Set:Area head pointer where set frame number    */
/*                                                  is stored                                   */
/* OUTPUT     : plFrameNumber                   Get only:Frame number                           */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_CLIANT        Invalid Master Client                           */
/*              GRP_USBD_INVALIED_PMTR          Parameter invalid                               */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend state                           */
/*              GRP_USBD_HOST_HALT              Host controller HALT state                      */
/*              GRP_USBD_NO_FUNCTION            No function                                     */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_FrameNumberControl( grp_u16 usDevId, grp_si  iControl, grp_s32 *plFrameNumber)
{
grp_hcdi_frame_control          tFrameReq;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x2b, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Device Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x2b, 0x01, F_END);
        return lStatus;
    }

    /* Check master client */
    if( (iControl == GRP_USBD_SET_FRAME_NUM)
     && (ucAddr   != l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucMasterCliantAddr) ){
        _TRACE_USBC_USBD_(0x2b, 0x02, F_END);
        return GRP_USBD_INVALIED_CLIANT;
    }

    /* Parameter check */
    if( (iControl != GRP_USBD_GET_FRAME_NUM) && (iControl != GRP_USBD_SET_FRAME_NUM) ){
        _TRACE_USBC_USBD_(0x2b, 0x03, F_END);
        return GRP_USBD_INVALIED_PMTR;
    }

    /* set host controller index number */
    tFrameReq.tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

    /* Get and set frame number to lower driver */
    if( iControl == GRP_USBD_GET_FRAME_NUM ){
        /* Get frame number */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcGetSof(&tFrameReq);
        if( lStatus == GRP_HCDI_OK ){
            *plFrameNumber = tFrameReq.lFrameNum;
        }
    }
    else {
        tFrameReq.lFrameNum = ((grp_u32)*plFrameNumber & GRP_USBD_FRAMENUMBER_MASK);
        /* Set framewidth */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcSetSof(&tFrameReq);
    }

    _TRACE_USBC_USBD_(0x2b, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_GetMasterClient                                                        */
/*                                                                                              */
/* DESCRIPTION: Get master client right                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_UNAVAILABLE            Master client unavailable                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_GetMasterClient( grp_u16 usDevId)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x2c, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        /* error */
        _TRACE_USBC_USBD_(0x2c, 0x01, F_END);
        return lStatus;
    }

    /* Master client right check */
    if( l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucMasterCliantAddr != GRP_USBD_NO_CLIANT_ADDRESS ){
        lStatus = GRP_USBD_UNAVAILABLE;
    }
    else {
        l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucMasterCliantAddr = ucAddr;
    }

    _TRACE_USBC_USBD_(0x2c, 0x00, F_END);

    return lStatus;
}
/************************************************************************************************/
/* FUNCTION   : grp_usbd_ReleaseMasterClient                                                    */
/*                                                                                              */
/* DESCRIPTION: Release master client                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier to release client right       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_CANNOT_RELEASE         Release failure for the master                  */
/*                                              client right                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_ReleaseMasterClient( grp_u16 usDevId)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x2d, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Address check */
    USBD_MACRO_ADRS_CHECK(_ID2AD(usDevId));
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x2d, 0x01, F_END);
        return lStatus;
    }

    if( ucAddr != l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucMasterCliantAddr ){
        _TRACE_USBC_USBD_(0x2d, 0x02, F_END);
        return GRP_USBD_CANNOT_RELEASE;
    }

    /* Release the master client right */
    l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucMasterCliantAddr = GRP_USBD_NO_CLIANT_ADDRESS;

    _TRACE_USBC_USBD_(0x2d, 0x00, F_END);

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_InterfaceCheck                                                         */
/*                                                                                              */
/* DESCRIPTION: Checked all pipes belonging to the specified interface and alternate.           */
/*              This function do NOT open any interface, only check.                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptOper                          Structure for operation of pipes                */
/*              (Member used)                                                                   */
/*                  usUsbDevId                  Device Id to be set                             */
/*                  ucInterface                 Interface to be set                             */
/*                  ucPipeNumber                Number of area of pipes information             */
/*                  ptPipe                      Head address of pipe handler of pipe            */
/*                                              corresponding to the first endpoint pertaining  */
/*                                              to interface                                    */
/* ONPUT      : ptOper                          Structure for operation of pipes                */
/*              (Member used)                                                                   */
/*                  ucPipeNumber                Actual Number of pipes belong to the interface  */
/*                  ptPipe                      Pipe information                                */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_INVALIED_IF            Invalid device number                           */
/*              GRP_USBD_NO_BANDWIDTH           Not enough communication bandwidth in order to  */
/*                                              switch interface                                */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_InterfaceCheck( grp_usbdi_pipe_operate *ptOper, grp_u8 ucAlternate )
{
grp_usbdi_if_desc               *ptCheckifDesPtr;
grp_usbdi_ep_desc               *ptEpDesPtr;
grp_usbdi_ep_desc               *ptNextEpPtr;
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;
grp_s32                         lCheck;
grp_u8                          ucAddr;
grp_u8                          ucPipeCnt   = 0;

    _TRACE_USBC_USBD_(0x2e, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptOper->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x2e, 0x01, F_END);
        return lStatus;
    }

    /* Configuration */
    if( l_tUSBD_CB.atDevTable[ucAddr].ulConfNum == 0 ){
        _TRACE_USBC_USBD_(0x2e, 0x02, F_END);
        return GRP_USBD_CONFIGURATION_ERROR;
    }

    /* Get Configuration Descriptor */
    lStatus = grp_cnfsft_GetDescFromUsbDevId( ptOper->usUsbDevId,
                                              (void **)GRP_USB_NULL,
                                              &ptOper->pvDescriptor );
    if( lStatus != GRP_CNFSFT_OK ){
        _TRACE_USBC_USBD_(0x2e, 0x05, F_END);
        return lStatus;
    }

    /* Get the specified interface descriptor */
    lStatus = _grp_usbd_GetIfDescptr( ptOper->pvDescriptor,
                                      ptOper->ucInterface,
                                      ucAlternate,
                                      &ptCheckifDesPtr);

    if( lStatus == GRP_USBD_OK ){
        if( ptOper->ucPipeNumber == 0 )
            ptOper->ucPipeNumber = ptCheckifDesPtr->bNumEndpoints;
        else{
            if( GRP_USBD_OK != grp_usbd_LockOpen( ) ){
                _TRACE_USBC_USBD_(0x2e, 0x07, F_END);
                return GRP_USBD_OS_RELATED_ERROR;
            }

            /* Get the first endpoint descriptor */
            lCheck = _grp_usbd_GetFirstEpDescptr( (grp_usbdi_config_desc *)ptOper->pvDescriptor,
                                                  ptCheckifDesPtr,
                                                  &ptEpDesPtr,
                                                  (void **)&ptNextEpPtr );

            /* Check all of pipes belonging to the interface */
            for( ucPipeCnt = 0; ((lCheck == GRP_USBD_OK) && (ptCheckifDesPtr->bNumEndpoints)); ucPipeCnt++ ){
                /* Set information */
                if( (ucPipeCnt < ptOper->ucPipeNumber) && (ptOper->ptPipe != GRP_USB_NULL) ){
                    ptPipe = &ptOper->ptPipe[ucPipeCnt];

                    ptPipe->usUsbDevId          = ptOper->usUsbDevId;
                    ptPipe->usMaxPacketSize     = (grp_u16)((grp_u16)(((grp_usbdi_ep_desc_b *)ptEpDesPtr)->wMaxPacketSize_Low)
                                                        | (grp_u16)((((grp_usbdi_ep_desc_b *)ptEpDesPtr)->wMaxPacketSize_High) << 8));
                    ptPipe->ucEndpointNumber    = ptEpDesPtr->bEndpointAddress;
                    ptPipe->ucBelongInterface   = ptOper->ucInterface;
                    ptPipe->ucBelongAlternate   = ucAlternate;
                    ptPipe->iTransferMode       = (grp_si)ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK;
                    ptPipe->ucTransferDirection = (grp_u8)(((ptEpDesPtr->bEndpointAddress & GRP_USBD_ED_BEPADDRESS_PID_BIT)
                                                           == GRP_USBD_ED_BEPADDRESS_PID_BIT) ? GRP_USBD_TX_IN : GRP_USBD_TX_OUT);
                    ptPipe->ulInterval          = _grp_usbd_GetEndpointInterval( l_tUSBD_CB.atDevTable[ucAddr].iSpeed, ptEpDesPtr );
                    ptPipe->ulBandWidth         = _grp_usbd_CalcBandwidth( ucAddr, ptEpDesPtr );
                }

                /* Get next endpoint descriptor */
                lCheck = _grp_usbd_GetNextEpDescptr( (grp_usbdi_config_desc *)ptOper->pvDescriptor,
                                                     ptNextEpPtr,
                                                     &ptEpDesPtr,
                                                     (void **)&ptNextEpPtr );
            }

            ptOper->ucPipeNumber = ucPipeCnt;

            if( GRP_USBD_OK != grp_usbd_UnlockOpen( ) ){
                _TRACE_USBC_USBD_(0x2e, 0x09, F_END);
                return GRP_USBD_OS_RELATED_ERROR;
            }
        }
    }

    _TRACE_USBC_USBD_(0x2e, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_SearchIfClass                                                          */
/*                                                                                              */
/* DESCRIPTION: Get interface that corresponds to the class after anlyzing among configuration  */
/*               descriptor. (This function can work in 16 interfaces and 16 alternatives)      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDesInfo                       Descriptor analysis information structure       */
/*                                              (About descriptor analysis information structure)*/
/*                pvDesc                        Configuration descriptor for analysis           */
/*              ucInterfaceClass                Class that hope to acquire                      */
/*              ucInterfaceSubClass             Sub class that hope to acquire                  */
/*              ucInterfaceProtocol             Protocol that hope to acquire                   */
/*              ptIfDesPtr                      Head address in the area to store interface     */
/*                                               descriptor.                                    */
/* OUTPUT     : ptIfDesPtr                      Interface descriptor                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Detection complete                              */
/*              GRP_USBD_INVALIED_IF            Invalid interface number                        */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_SearchIfClass(grp_usbdi_descriptor_info *ptDesInfo, grp_u8 ucIfClass, grp_u8 ucIfSubClass, grp_u8 ucIfProtocol, grp_usbdi_if_desc **pptIfDesPtr)
{
void                            *pvStartDesPtr;
void                            *pvNextDesPtr;
grp_s32                         lStatus = GRP_USBD_OK;

    _TRACE_USBC_USBD_(0x2f, 0x00, 0x00);

    /* Check the bandwidth of each endpoint */
    for( pvStartDesPtr = ptDesInfo->pvDesc; lStatus == GRP_USBD_OK; pvStartDesPtr = pvNextDesPtr ){
        lStatus = grp_usbd_SearchDescriptor((grp_usbdi_config_desc *)ptDesInfo->pvDesc,
                                             GRP_USBD_DESC_INTERFACE,
                                             pvStartDesPtr,
                                             (void **)pptIfDesPtr,
                                             &pvNextDesPtr);
        if( lStatus == GRP_USBD_OK ){
            if( (((*pptIfDesPtr)->bInterfaceClass == ucIfClass) || (ucIfClass == 0xff))
             && (((*pptIfDesPtr)->bInterfaceSubClass == ucIfSubClass) || (ucIfSubClass == 0xff))
             && (((*pptIfDesPtr)->bInterfaceProtocol == ucIfProtocol) || (ucIfProtocol == 0xff)) ){
                /* exit FOR loop */
                break;
            }
        }
    }

    _TRACE_USBC_USBD_(0x2f, 0x00, F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_LockOpen                                                               */
/*                                                                                              */
/* DESCRIPTION:  Start exclusive control.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_LockOpen(void)
{
    if ( grp_vos_GetSemaphore( l_tUSBD_CB.ptSem[GRP_USBD_OPENPIPE_SEM],GRP_VOS_INFINITE)
     == GRP_VOS_POS_RESULT) {
        /* ok */
        return GRP_USBD_OK;
    }
    else {
        /* ng */
        return GRP_USBD_ILLEGAL_ERROR;
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_UnlockOpen                                                             */
/*                                                                                              */
/* DESCRIPTION:  Start exclusive control.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iSemNum                 Semaphore number                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_UnlockOpen(void)
{
    if ( grp_vos_ReleaseSemaphore( l_tUSBD_CB.ptSem[GRP_USBD_OPENPIPE_SEM])
     == GRP_VOS_POS_RESULT) {
        /* ok */
        return GRP_USBD_OK;
    }
    else {
        /* ng */
        return GRP_USBD_ILLEGAL_ERROR;
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_LockAddress                                                            */
/*                                                                                              */
/* DESCRIPTION:  Start exclusive control.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_LockAddress(void)
{
    if ( grp_vos_GetSemaphore( l_tUSBD_CB.ptSem[GRP_USBD_ADDRESS_SEM],GRP_VOS_INFINITE)
     == GRP_VOS_POS_RESULT) {
        /* ok */
        return GRP_USBD_OK;
    }
    else {
        /* ng */
        return GRP_USBD_ILLEGAL_ERROR;
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_UnlockAddress                                                          */
/*                                                                                              */
/* DESCRIPTION:  Start exclusive control.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iSemNum                         Semaphore number                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_UnlockAddress(void)
{
    if ( grp_vos_ReleaseSemaphore( l_tUSBD_CB.ptSem[GRP_USBD_ADDRESS_SEM])
     == GRP_VOS_POS_RESULT) {
        /* ok */
        return GRP_USBD_OK;
    }
    else {
        /* ng */
        return GRP_USBD_ILLEGAL_ERROR;
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_LockEnumeration                                                        */
/*                                                                                              */
/* DESCRIPTION:  Start exclusive control.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_LockEnumeration(void)
{
    if ( grp_vos_GetSemaphore( l_tUSBD_CB.ptSem[GRP_USBD_DEFAULTPIPE_SEM],GRP_VOS_INFINITE)
     == GRP_VOS_POS_RESULT) {
        /* ok */
        return GRP_USBD_OK;
    }
    else {
        /* ng */
        return GRP_USBD_ILLEGAL_ERROR;
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_usbd_UnlockEnumeration                                                      */
/*                                                                                              */
/* DESCRIPTION:  Start exclusive control.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbd_UnlockEnumeration(void)
{
    if ( grp_vos_ReleaseSemaphore( l_tUSBD_CB.ptSem[GRP_USBD_DEFAULTPIPE_SEM])
     == GRP_VOS_POS_RESULT) {
        /* ok */
        return GRP_USBD_OK;
    }
    else {
        /* ng */
        return GRP_USBD_ILLEGAL_ERROR;
    }
}

/************************************************************************************************/
/*                                     Callback functions                                       */
/************************************************************************************************/

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpDeviceRequest                                                      */
/*                                                                                              */
/* DESCRIPTION: Device request callback function                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpDeviceRequest( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_device_request        *ptDevReq;

    _TRACE_USBC_USBD_(0x40, 0x00, 0x00);

    /* Get URB */
    ptDevReq = (grp_usbdi_device_request *)(ptIrpReq->pvUrbPtr);

    /* Set status */
    ptDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    if( (grp_u32)ptDevReq->pfnDvCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x40, 0x01, 0x00);

        /* Read upper callback function from URB */
        ptDevReq->pfnDvCbFunc( ptDevReq );
    }

    _TRACE_USBC_USBD_(0x40, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpGetStatus                                                          */
/*                                                                                              */
/* DESCRIPTION: Get Status request callback function                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpGetStatus( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;

    _TRACE_USBC_USBD_(0x41, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Set status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);
    /* Release status data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucBufferPtr);

    /* Copy Status data */
    ptStDevReq->aucRetStatus[0] = *ptIrpReq->pucBufferPtr;
    ptStDevReq->aucRetStatus[1] = *(ptIrpReq->pucBufferPtr + 1);

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x41, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc( ptStDevReq );
    }


    _TRACE_USBC_USBD_(0x41, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpNormalRequest                                                      */
/*                                                                                              */
/* DESCRIPTION: Communication requestcallback function                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpNormalRequest( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_request               *ptReq;

    _TRACE_USBC_USBD_(0x42, 0x00, 0x00);

    /* Get URB */
    ptReq = (grp_usbdi_request *)(ptIrpReq->pvUrbPtr);

    /* Set status */
    ptReq->lStatus = ptIrpReq->lStatus;

    /* Set send complete value */
    ptReq->ulActualLength = ptIrpReq->ulActualLength;

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
    /* Store the number of errors at isochronous transfer <Ver 2.02a> */
    if( ptReq->ptPipe->iTransferMode == GRP_USBD_ISOCHRONOUS ){
        ptReq->ulIsoErrorCount = ptIrpReq->ulErrorCnt;

        /* pipe halted when pertinent error occured */
        if( ptIrpReq->lStatus == GRP_USBD_TR_HC_HALTED ){
            ptReq->ptPipe->iStatus = GRP_USBD_PIPE_HALT;
        }
    }
    else
#endif
    {
        /* if error occured, pipe halted */
        if( (ptIrpReq->lStatus != GRP_USBD_TR_NO_FAIL)
         && (ptIrpReq->lStatus != GRP_USBD_TR_CANCEL)
         && (ptIrpReq->lStatus != GRP_USBD_TR_DATA_UNDERRUN)
         && (ptIrpReq->ptEndpoint->ucTxMode != GRP_USBD_CONTROL) ){
            /* set HALT status */
            ptReq->ptPipe->iStatus = GRP_USBD_PIPE_HALT;
        }
    }

    if( (grp_u32)ptReq->pfnNrCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x42, 0x01, 0x00);

        /* Read upper callback function from URB */
        ptReq->pfnNrCbFunc( ptReq );
    }

    _TRACE_USBC_USBD_(0x42, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpPipeActive                                                         */
/*                                                                                              */
/* DESCRIPTION: PIPE activate request callback function                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpPipeActive( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;

    _TRACE_USBC_USBD_(0x43, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Change pipe status when transfer was successful */
    if( ptIrpReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        /* set ACTIVE status */
        ptStDevReq->ptPipe->iStatus = GRP_USBD_PIPE_ACTIVE;
    }

    /* Set the status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x43, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc(ptStDevReq);
    }

    _TRACE_USBC_USBD_(0x43, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpPipeHalt                                                           */
/*                                                                                              */
/* DESCRIPTION: PIPE HALT request callback function                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpPipeHalt( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;

    _TRACE_USBC_USBD_(0x44, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Set the pipe status if transfer is successful */
    if( ptIrpReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        /* set HALT status */
        ptStDevReq->ptPipe->iStatus = GRP_USBD_PIPE_HALT;
    }

    /* Set the status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x44, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc(ptStDevReq);
    }

    _TRACE_USBC_USBD_(0x44, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpSetaddress                                                         */
/*                                                                                              */
/* DESCRIPTION: Set Address Request callback function                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpSetaddress( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;
grp_usbdi_device_info           tTmpDevInfo;
grp_usbd_host_data              *ptTmpHcIndex;
grp_si                          iTmpSpeed;
grp_s32                         lStatus = 0;
grp_si                          i       = 0;
grp_si                          j       = 0;
grp_u16                         usDevId = 0;
grp_u8                          ucAddr  = 0;

    _TRACE_USBC_USBD_(0x45, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Set the status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* get address number and device identifier (address number is the wValue of setupdata) */
    ucAddr  = *(grp_u8 *)(ptIrpReq->pucSetupPtr + GRP_USBD_SETUP_WVALUE_LOW);
    usDevId = l_tUSBD_CB.ausDevIdTable[ucAddr];
    
    /* Switch the default pipe if transfer is successful */
    if( ptIrpReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        _TRACE_USBC_USBD_(0x45, 0x02, 0x00);

        /* save address 0 port state */
        iTmpSpeed              = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iSpeed;
        tTmpDevInfo.ucHubAddr  = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDeviceInfo.ucHubAddr;
        tTmpDevInfo.ucPortNum  = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDeviceInfo.ucPortNum;
        tTmpDevInfo.ucPortInfo = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDeviceInfo.ucPortInfo;
        ptTmpHcIndex           = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].ptHcIndex;

        /* move to disconnect state */
        l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iPortState = GRP_USBD_STATE_BLANK;

        /* Close the default pipe of address 0 */
        lStatus = _grp_usbd_Add0DefaultpipeClose();
        if( lStatus != GRP_USBD_OK ){
            _TRACE_USBC_USBD_(0x45, 0x03, 0x00);

            /* Release address */
            _grp_usbd_ReleaseDevId(usDevId);

            ptStDevReq->lStatus = GRP_USBD_OTHER_FAIL;
        }
        else {
            _TRACE_USBC_USBD_(0x45, 0x04, 0x00);

            /* Determine the port state */
            l_tUSBD_CB.atDevTable[ucAddr].iSpeed                 = iTmpSpeed;
            l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.usUsbDevId = usDevId;
            l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.ucHubAddr  = tTmpDevInfo.ucHubAddr;
            l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.ucPortNum  = tTmpDevInfo.ucPortNum;
            l_tUSBD_CB.atDevTable[ucAddr].tDeviceInfo.ucPortInfo = tTmpDevInfo.ucPortInfo;
            l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex              = ptTmpHcIndex;
            l_tUSBD_CB.atDevTable[ucAddr].ulOpenPipeCount        = 0;

            /* Open the default pipe newly allocated */
            lStatus = _grp_usbd_DefaultpipeOpen( ucAddr, ptStDevReq->ucEp0size );
            /* Error if default pipe does not open */
            if( lStatus != GRP_USBD_OK ){
                _TRACE_USBC_USBD_(0x45, 0x05, 0x00);

                /* Release address */
                _grp_usbd_ReleaseDevId(usDevId);

                ptStDevReq->lStatus = GRP_USBD_OTHER_FAIL;
            }
            else {
                _TRACE_USBC_USBD_(0x45, 0x06, 0x00);

                /* Initialization of device pipe management area and endpoint management area */
                for( i = 0; i < GRP_USBD_MAX_PIPE; i++ ){
                    for( j = 0; j < 2; j++ ){ /* IN and OUT */
                        l_tUSBD_CB.atDevTable[ucAddr].patPipe[j][i] = (grp_usbdi_pipe *)GRP_USB_NULL;
                    }
                }

                /* Initialization of interface management area */
                for( i = 0; i < GRP_USBD_MAX_IF; i++ ){
                    l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[i].ucState = GRP_USBD_STATE_BLANK;
                    l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[i].ucAltNum = 0;
                }

                /* Move to physically connected state(Now in address state) */
                l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_CONNECT;
            }

            /* address is valid */
            ptStDevReq->usUsbDevId = usDevId;
        }
    }
    else {
        _TRACE_USBC_USBD_(0x45, 0x07, 0x00);
        /* Release address */
        _grp_usbd_ReleaseDevId(usDevId);
    }

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x45, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc(ptStDevReq);
    }

    _TRACE_USBC_USBD_(0x45, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpSetconfiguration                                                   */
/*                                                                                              */
/* DESCRIPTION: Set Configuration Request callback function                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpSetconfiguration( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;
grp_si                          i = 0;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x46, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Get Device Address */
    ucAddr = (grp_u8)(ptStDevReq->usUsbDevId & GRP_USBD_DEVID_ADDR_MASK);

    /* Store the current configuration */
    if( ptIrpReq->lStatus == GRP_USBD_TR_NO_FAIL ){

        l_tUSBD_CB.atDevTable[ucAddr].ulConfNum = ptStDevReq->ucConfiguration;

        /* Initialize the interface information confitured */
        for( i = 0; i < GRP_USBD_MAX_IF; i++ ){
            l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[i].ucState = GRP_USBD_STATE_BLANK;
            l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[i].ucAltNum = 0;
        }
    }

    /* Set the status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x46, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc(ptStDevReq);
    }

    _TRACE_USBC_USBD_(0x46, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpSetinterface                                                       */
/*                                                                                              */
/* DESCRIPTION: Set Interface Request callback function                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpSetinterface( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x47, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Get Device Address */
    ucAddr = GRP_USBD_DEVID2ADDR( ptStDevReq->usUsbDevId );

    /* Set the status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Store the current interface */
    if( ptIrpReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        _TRACE_USBC_USBD_(0x47, 0x00, 0x01);

        /* Register the alnernate number */
        l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptStDevReq->ucInterface].ucAltNum = ptStDevReq->ucAlternate;
    }

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    /* Release open pipe semaphore */
    grp_usbd_UnlockOpen( );

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        /* Call callback function */
        ptStDevReq->pfnStCbFunc(ptStDevReq);
    }

    _TRACE_USBC_USBD_(0x47, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpStdDeviceRequest                                                   */
/*                                                                                              */
/* DESCRIPTION: Standard device request callback function                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpStdDeviceRequest( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;

    _TRACE_USBC_USBD_(0x48, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Set status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x48, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc( ptStDevReq );
    }

    _TRACE_USBC_USBD_(0x48, 0x00, F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpUnsetconfiguration                                                 */
/*                                                                                              */
/* DESCRIPTION: Devide configuration clear request callback function                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpUnsetconfiguration( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;
grp_si                          i;
grp_si                          j;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x49, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Get Device Address */
    ucAddr = (grp_u8)(ptStDevReq->usUsbDevId & GRP_USBD_DEVID_ADDR_MASK);

    /* Move to unconfigured status */
    if( ptIrpReq->lStatus == GRP_USBD_TR_NO_FAIL ){

        l_tUSBD_CB.atDevTable[ucAddr].ulConfNum = 0;

        /* Close all the pipe for the interfaces which have been open */
        for( i = 0; i < GRP_USBD_MAX_PIPE; i++ ){
            for( j = 0; j < 2; j++ ){
                if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[j][i] != (grp_usbdi_pipe *)GRP_USB_NULL ){
                    grp_usbd_PipeClose(l_tUSBD_CB.atDevTable[ucAddr].patPipe[j][i]);
                }
            }
        }
    }

    /* Set status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);

    if( (grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL ){
        _TRACE_USBC_USBD_(0x49, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc( ptStDevReq );
    }

    _TRACE_USBC_USBD_(0x49, 0x01, F_END);
}

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CmpSynchFrame                                                         */
/*                                                                                              */
/* DESCRIPTION: Synch Frame request callback function                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptIrpReq                        request                                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CmpSynchFrame( grp_hcdi_tr_request *ptIrpReq)
{
grp_usbdi_st_device_request     *ptStDevReq;

    _TRACE_USBC_USBD_(0x4a, 0x00, 0x00);

    /* Get URB */
    ptStDevReq = (grp_usbdi_st_device_request *)(ptIrpReq->pvUrbPtr);

    /* Set status */
    ptStDevReq->lStatus        = ptIrpReq->lStatus;
    /* Set send complete value */
    ptStDevReq->ulActualLength = ptIrpReq->ulActualLength;

    /* Copy Frame data */
    ptStDevReq->aucRetStatus[0] = *ptIrpReq->pucBufferPtr;
    ptStDevReq->aucRetStatus[1] = *(ptIrpReq->pucBufferPtr + 1);

    /* Release setup data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucSetupPtr);
    /* Release status data buffer to usb communication buffer */
    grp_cmem_BlkRel(ptIrpReq->pucBufferPtr);

    if((grp_u32)ptStDevReq->pfnStCbFunc != (grp_u32)GRP_USB_NULL) {
        _TRACE_USBC_USBD_(0x4a, 0x01, 0x00);

        /* Call callback function */
        ptStDevReq->pfnStCbFunc( ptStDevReq);
    }

    _TRACE_USBC_USBD_(0x4a, 0x00, F_END);
}
#endif /* GRP_USB_HOST_USE_ISOCHRONOUS */

/************************************************************************************************/
/*                                       Internal functions                                     */
/************************************************************************************************/

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_Add0DefaultpipeOpen                                                   */
/*                                                                                              */
/* DESCRIPTION: Address 0 default pipe open                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK             Success                                                 */
/*              GRP_USBD_ILLEGAL_ERROR  Illegal function                                        */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_Add0DefaultpipeOpen(void)
{
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x50, 0x00, 0x00);

    /* Select default pipe */
    ptPipe = &l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe;

    /* Set default pipe information */
    ptPipe->tEndpoint.ucTxSpeed = (grp_u8)(l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iSpeed);


    /* set max packet size */
    if( l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iSpeed == GRP_USBD_FULL_SPEED ){
        ptPipe->tEndpoint.usMaxPacketSize  = GRP_USBD_FULL_SPEED_CTRL_MPS;
    }
    else if( l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iSpeed == GRP_USBD_LOW_SPEED ){
        ptPipe->tEndpoint.usMaxPacketSize  = GRP_USBD_LOW_SPEED_CTRL_MPS;
    }
    else {/* if(l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].iSpeed == GRP_USBD_HIGH_SPEED) */
        ptPipe->tEndpoint.usMaxPacketSize  = GRP_USBD_HIGH_SPEED_CTRL_MPS;
    }

    /* set port status */
    lStatus = _grp_usbd_SearchUSB20Hub(GRP_USBD_DEFAULT_ADDRESS,&ptPipe->tEndpoint.ucHubAddress,&ptPipe->tEndpoint.ucPortNum);
    if( lStatus != GRP_USBD_OK ){
        return GRP_USBD_ILLEGAL_ERROR;
    }

    ptPipe->tEndpoint.tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].ptHcIndex->usHcIndexNum;

    /* Make endpoint open request to lower driver */
    lStatus = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].ptHcIndex->tHcdiFunc.pfnHcEpOpen( &ptPipe->tEndpoint );
    if( lStatus == GRP_HCDI_OK ){
        /* Process after Success */

        /* Update pipe information */
        ptPipe->usMaxPacketSize = ptPipe->tEndpoint.usMaxPacketSize;
        
        /* default pipe is activated */
        ptPipe->iStatus = GRP_USBD_PIPE_ACTIVE;
    }

    _TRACE_USBC_USBD_(0x50, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_Add0DefaultpipeClose                                                  */
/*                                                                                              */
/* DESCRIPTION: Address 0 default pipe close                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_Add0DefaultpipeClose(void)
{
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x51, 0x00, 0x00);

    /* Select default pipe */
    ptPipe = &l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe;

    /* Make a request to lower host controller driver */
    lStatus = l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].ptHcIndex->tHcdiFunc.pfnHcEpClose(&ptPipe->tEndpoint);
    /* Status check */
    if( lStatus == GRP_HCDI_OK )
    {
        /* default pipe is halted */
        ptPipe->iStatus = GRP_USBD_PIPE_HALT;
    }

    _TRACE_USBC_USBD_(0x51, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_AllDeleteUrb                                                          */
/*                                                                                              */
/* DESCRIPTION: Delete all the transfer request from pipe handler                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPipe                          Pipe handler to delete all                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalide pipe information                       */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_AllDeleteUrb( grp_usbdi_pipe *ptPipe)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x52, 0x00, 0x00);

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    /* Pipe check */
    USBD_MACRO_PIPE_CHECK(ptPipe,GRP_USBD_INVALIED_PH);
#endif

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptPipe->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x52, 0x01, F_END);
        return lStatus;
    }

    /* Make a request to the lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpFlash(&ptPipe->tEndpoint);

    _TRACE_USBC_USBD_(0x52, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CalcBandwidth                                                         */
/*                                                                                              */
/* DESCRIPTION: Bandwidth calculation                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Device address                                  */
/*              ptEpDesPtr                      endpoint descriptor to secure the bandwidth     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Bandwidth of the specified endpoint                                             */
/*              (Calculate by ns)                                                               */
/*                                                                                              */
/************************************************************************************************/
/* CAUTION    : This function is made based on Chapter 5.11.3 of Universal Serial Bus           */
/*              Specification Recision 2.0.                                                     */
/*----------------------------------------------------------------------------------------------*/
DLOCAL const grp_u16 usPoByte[4]            = {45,9,13,13};
DLOCAL const grp_u32 ulHighShakeByte[4]     = {91652, 63323, 91652, 91652};
DLOCAL const grp_u32 ulFullShakeByte[2][4]  = {{910700,726800,910700,910700},
                                               {910700,626500,910700,910700}};
DLOCAL const grp_u32 ulLowShakeByte[4]      = {6406000,0,0,6410700};
DLOCAL const grp_u32 ulLsData[2]            = {67667,66700};
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_u32 _grp_usbd_CalcBandwidth( grp_u8 ucAddr, grp_usbdi_ep_desc *ptEpDesPtr)
{
grp_u32                         ulBandWidth = 0;
grp_u16                         usTransferByte;
grp_u8                          ucTransferMode;
grp_u8                          ucTransferDir;

    _TRACE_USBC_USBD_(0x53, 0x00, 0x00);

    ucTransferMode = (grp_u8)(ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK);
    ucTransferDir  = (grp_u8)(((ptEpDesPtr->bEndpointAddress & GRP_USBD_ED_BEPADDRESS_PID_BIT) == GRP_USBD_ED_BEPADDRESS_PID_BIT)
                              ? GRP_USBD_TX_OUT : GRP_USBD_TX_IN);

    /* Transfer byte calculation */
    /* Calculation differs depending on endian */
    usTransferByte = (grp_u16)(usPoByte[ucTransferMode] + (grp_u16)((grp_u16)(((grp_usbdi_ep_desc_b *)ptEpDesPtr)->wMaxPacketSize_Low)
                                                        | (grp_u16)((((grp_usbdi_ep_desc_b *)ptEpDesPtr)->wMaxPacketSize_High) << 8)));

    if( ((ucTransferMode == GRP_USBD_ISOCHRONOUS) || (ucTransferMode == GRP_USBD_INTERRUPT))
     && (l_tUSBD_CB.atDevTable[ucAddr].iSpeed == GRP_USBD_HIGH_SPEED) ){
        usTransferByte *= (grp_u16)(((((grp_usbdi_ep_desc_b *)ptEpDesPtr)->wMaxPacketSize_High & 0x18) >> 3) + 1);
    }

    /* Calculate in isochronous and interrupt modes */
    if( (ucTransferMode == GRP_USBD_ISOCHRONOUS)
     || (ucTransferMode == GRP_USBD_INTERRUPT) ){
        /* Calculation differs at full speed,low speed and high speed */
        if( l_tUSBD_CB.atDevTable[ucAddr].iSpeed == GRP_USBD_FULL_SPEED ){
            ulBandWidth = (grp_u32)(ulFullShakeByte[ucTransferDir][ucTransferMode]
                                 + ((8354) * (grp_u32)(((grp_u32)usTransferByte * 7 * 8) / 6))
                                 + l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulHostDelay * 100);
        }
        else if( l_tUSBD_CB.atDevTable[ucAddr].iSpeed == GRP_USBD_LOW_SPEED ){
            ulBandWidth = (grp_u32)(ulLowShakeByte[ucTransferMode]
                                 + (2 * l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulLsSetup * 100)
                                 + (ulLsData[ucTransferDir] * (grp_u32)(((grp_u32)usTransferByte * 7 * 8) / 6))
                                 + l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulHostDelay * 100);
        }
        else {/* if(l_tUSBD_CB.atDevTable[ulAddr].iSpeed == GRP_USBD_HIGH_SPEED) */
            ulBandWidth = (grp_u32)(ulHighShakeByte[ucTransferMode]
                                 + ((208) * (grp_u32)(((grp_u32)usTransferByte * 7 * 8) / 6))
                                 + l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulHostDelay * 100);
        }
    }

    _TRACE_USBC_USBD_(0x53, 0x00, F_END);

    return (ulBandWidth / 100);
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CalcIfBandwidth                                                       */
/*                                                                                              */
/* DESCRIPTION: Bandwidth calculation for the specified interface                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                      Address number                                      */
/*              ucIf                        Interface number                                    */
/*              ucAlt                       Alternate setting number                            */
/*              *ptCnfDesPtr                Pointer to all of configuration descriptor          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : BandWidth                                                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_u32 _grp_usbd_CalcIfBandwidth( grp_u8 ucAddr, grp_u8 ucIf, grp_u8 ucAlt, grp_usbdi_config_desc *ptCnfDesPtr)
{
grp_usbdi_ep_desc               *ptEpDesPtr;
grp_usbdi_ep_desc               *ptNextEpPtr;
grp_usbdi_if_desc               *ptIfDesPtr;
grp_s32                         lStatus;
grp_u32                         ulBandWidth = 0;
grp_u8                          ucEp        = 0;

    _TRACE_USBC_USBD_(0x54, 0x00, 0x00);

    /* search the interface descriptor */
    lStatus = _grp_usbd_GetIfDescptr(ptCnfDesPtr, ucIf, ucAlt, &ptIfDesPtr);
    if( lStatus == GRP_USBD_OK ){
        /* Get the first endpoint descriptor */
        lStatus = _grp_usbd_GetFirstEpDescptr( ptCnfDesPtr,
                                               ptIfDesPtr,
                                               &ptEpDesPtr,
                                               (void **)&ptNextEpPtr );
        if( lStatus == GRP_USBD_OK ){
            /* Calculate the bandwidth */
            ulBandWidth += _grp_usbd_CalcBandwidth(ucAddr,ptEpDesPtr);

            /* Get the next endpoint descriptor */
            for( ucEp = 1; ucEp < ptIfDesPtr->bNumEndpoints; ucEp++ ){
                /* Get next endpoint descriptor */
                lStatus = _grp_usbd_GetNextEpDescptr( ptCnfDesPtr,
                                                      ptNextEpPtr,
                                                      &ptEpDesPtr,
                                                      (void **)&ptNextEpPtr );
                if ( lStatus == GRP_USBD_OK ){
                    /* Calculate the bandwidth */
                    ulBandWidth += _grp_usbd_CalcBandwidth(ucAddr,ptEpDesPtr);
                }
                else {
                    break;
                }
            }
        }
    }

    _TRACE_USBC_USBD_(0x54, 0x00, F_END);

    return ulBandWidth;
}

#if(0)                                                                                              /* V1.04 {  */
/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CloseAllPipes                                                         */
/*                                                                                              */
/* DESCRIPTION: This function closies all pipes                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Device address                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_CloseAllPipes( grp_u8 ucAddr )
{
grp_s32                         lEp         = 0;
grp_s32                         lStatus     = 0;
grp_s32                         lRetStatus  = GRP_USBD_OK;

    _TRACE_USBC_USBD_(0x55, 0x00, 0x00);

    /* Close all out transaction pipe */
    for( lEp = 1; lEp < GRP_USBD_MAX_PIPE; lEp++ ){
        if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_OUT][lEp] != (grp_usbdi_pipe *)GRP_USB_NULL ){
            /* Close the pipe */
            lStatus = grp_usbd_PipeClose( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_OUT][lEp] );
            if( lStatus != GRP_USBD_OK ){
                lRetStatus = GRP_USBD_ILLEGAL_ERROR;
            }
        }
    }

    /* Close all in transaction pipe */
    for( lEp = 1; lEp < GRP_USBD_MAX_PIPE; lEp++ ){
        if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_IN][lEp] != (grp_usbdi_pipe *)GRP_USB_NULL ){
            /* Close the pipe */
            lStatus = grp_usbd_PipeClose( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_IN][lEp] );
            if( lStatus != GRP_USBD_OK ){
                lRetStatus = GRP_USBD_ILLEGAL_ERROR;
            }
        }
    }

    _TRACE_USBC_USBD_(0x55, 0x00, F_END);

    return lRetStatus;
}
#endif                                                                                              /* V1.04 }  */

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CloseInterfaceAllPipes                                                */
/*                                                                                              */
/* DESCRIPTION: This function opens all pipes belonging to the specifeid interface              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : *ptOper                         Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              descriptor                      Configuration descriptor configured  so that    */
/*                                              interface to be set is pertained                */
/*              *ptIfDesPtr                     Pointer to interface descriptor                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_CloseInterfaceAllPipes( grp_usbdi_pipe_operate *ptOper, grp_usbdi_if_desc *ptIfDesPtr)
{
grp_usbdi_ep_desc               *ptNextEpPtr;
grp_usbdi_ep_desc               *ptEpDesPtr;
grp_s32                         lStatus;
grp_u8                          ucAddr;
grp_u8                          ucPipeCnt;
grp_u8                          ucEpNum = 0;
grp_u8                          ucEpDir = 0;

    _TRACE_USBC_USBD_(0x56, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptOper->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x56, 0x01, F_END);
        return;
    }

    if( ptIfDesPtr != GRP_USB_NULL ){
        /* Get the first endpoint descriptor */
        lStatus = _grp_usbd_GetFirstEpDescptr( (grp_usbdi_config_desc *)ptOper->pvDescriptor,
                                               ptIfDesPtr,
                                               &ptEpDesPtr,
                                               (void **)&ptNextEpPtr );

        /* Close all of pipes belonging to the interface */
        for( ucPipeCnt = 0;
             ((lStatus == GRP_USBD_OK) && (ucPipeCnt < ptIfDesPtr->bNumEndpoints));
             ucPipeCnt++ ){

            ucEpNum = (grp_u8)(ptEpDesPtr->bEndpointAddress & GRP_USBD_ED_BEPADDRESS_MASK);
            ucEpDir = (grp_u8)(((ptEpDesPtr->bEndpointAddress & GRP_USBD_ED_BEPADDRESS_PID_BIT) == GRP_USBD_ED_BEPADDRESS_PID_BIT)
                               ? GRP_USBD_TX_IN : GRP_USBD_TX_OUT);

            /* Close the pipe */
            if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[ucEpDir][ucEpNum] != (grp_usbdi_pipe *)GRP_USB_NULL ){
                lStatus = grp_usbd_PipeClose( l_tUSBD_CB.atDevTable[ucAddr].patPipe[ucEpDir][ucEpNum] );
            }

            /* Get next endpoint descriptor */
            if( lStatus == GRP_USBD_OK ){
                lStatus = _grp_usbd_GetNextEpDescptr( (grp_usbdi_config_desc *)ptOper->pvDescriptor,
                                                       ptNextEpPtr,
                                                       &ptEpDesPtr,
                                                       (void **)&ptNextEpPtr );
            }
        }
    }
    else {
        /* Close all of pipes belonging to the interface */
        for( ucEpNum = 1;
             ((lStatus == GRP_USBD_OK) && (ucEpNum < GRP_USBD_MAX_PIPE));
             ucEpNum++ ){
            /* Close the pipe */
            if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_OUT][ucEpNum] != (grp_usbdi_pipe *)GRP_USB_NULL ){
                if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_OUT][ucEpNum]->ucBelongInterface
                 == ptOper->ucInterface ){
                    lStatus = grp_usbd_PipeClose( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_OUT][ucEpNum] );
                }
            }
        }

        for( ucEpNum = 1;
             ((lStatus == GRP_USBD_OK) && (ucEpNum < GRP_USBD_MAX_PIPE));
             ucEpNum++ ){
            /* Close the pipe */
            if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_IN][ucEpNum] != (grp_usbdi_pipe *)GRP_USB_NULL ){
                if( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_IN][ucEpNum]->ucBelongInterface
                 == ptOper->ucInterface ){
                    lStatus = grp_usbd_PipeClose( l_tUSBD_CB.atDevTable[ucAddr].patPipe[GRP_USBD_TX_IN][ucEpNum] );
                }
            }
        }
    }

    _TRACE_USBC_USBD_(0x56, 0x00, F_END);

    return;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_DefaultpipeOpen                                                       */
/*                                                                                              */
/* DESCRIPTION: default pipe open                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Device address                                  */
/*              ucEp0Size                       Endpoint 0 max packet size                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_DefaultpipeOpen( grp_u8 ucAddr, grp_u8 ucEp0Size)
{
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x57, 0x00, 0x00);

    /* Select default pipe */
    ptPipe = &l_tUSBD_CB.atDevTable[ucAddr].tDefaultPipe;

    /* Set default pipe information */
    ptPipe->tEndpoint.ucAddress           = ucAddr;
    ptPipe->tEndpoint.ucEpNum             = GRP_USBD_DEFAULT_ENDPOINT;
    ptPipe->tEndpoint.ucTxMode            = GRP_USBD_CONTROL;
    ptPipe->tEndpoint.usMaxPacketSize     = ucEp0Size;
    ptPipe->tEndpoint.ucTxSpeed           = (grp_u8)l_tUSBD_CB.atDevTable[ucAddr].iSpeed;
    ptPipe->tEndpoint.tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

    lStatus = _grp_usbd_SearchUSB20Hub(ucAddr,&ptPipe->tEndpoint.ucHubAddress,&ptPipe->tEndpoint.ucPortNum);
    if( lStatus != GRP_USBD_OK ){
        return GRP_USBD_ILLEGAL_ERROR;
    }

    /* Make endpoint open request to lower driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpOpen( &ptPipe->tEndpoint );
    if( lStatus == GRP_HCDI_OK ){
        /* Process after Success */

        /* Update pipe information */
        ptPipe->usUsbDevId          = l_tUSBD_CB.ausDevIdTable[ucAddr];
        ptPipe->ucEndpointNumber    = GRP_USBD_DEFAULT_ENDPOINT;
        ptPipe->iTransferMode       = GRP_USBD_CONTROL;
        ptPipe->ucTransferDirection = GRP_USBD_TX_INOUT; /* Not used */
        ptPipe->ulInterval          = 0;
        ptPipe->usMaxPacketSize     = (grp_u16)ucEp0Size;
        ptPipe->iStatus             = GRP_USBD_PIPE_ACTIVE;   /* Pipe is actived at open */
    }

    _TRACE_USBC_USBD_(0x57, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_DefaultpipeClose                                                      */
/*                                                                                              */
/* DESCRIPTION: default pipe close                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Device address                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_DefaultpipeClose( grp_u8 ucAddr)
{
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;

    _TRACE_USBC_USBD_(0x58, 0x00, 0x00);

    /* Select default pipe */
    ptPipe = &l_tUSBD_CB.atDevTable[ucAddr].tDefaultPipe;

    /* Make a request to lower host controller driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcEpClose(&ptPipe->tEndpoint);

    _TRACE_USBC_USBD_(0x58, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_GetEndpointInterval                                                   */
/*                                                                                              */
/* DESCRIPTION: Interval calculation                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : speed                           Device speed                                    */
/*              ptEpDesPtr                      endpoint descriptor                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Endpoint interval                                                               */
/*                                                                                              */
/************************************************************************************************/
/*----------------------------------------------------------------------------------------------*/
DLOCAL const grp_u8 ucHighInterval[] = {GRP_USBD_NO_INTERVAL,
                                        GRP_USBD_INTERVAL_125us, GRP_USBD_INTERVAL_250us, GRP_USBD_INTERVAL_500us, GRP_USBD_INTERVAL_1ms ,
                                        GRP_USBD_INTERVAL_2ms  , GRP_USBD_INTERVAL_4ms  , GRP_USBD_INTERVAL_8ms  , GRP_USBD_INTERVAL_16ms,
                                        GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms,
                                        GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms   };
DLOCAL const grp_u8 ucFullInterval[] = {GRP_USBD_NO_INTERVAL,
                                        GRP_USBD_INTERVAL_1ms  , GRP_USBD_INTERVAL_2ms  , GRP_USBD_INTERVAL_4ms  , GRP_USBD_INTERVAL_8ms ,
                                        GRP_USBD_INTERVAL_16ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms,
                                        GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms,
                                        GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms , GRP_USBD_INTERVAL_32ms   };
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_u8 _grp_usbd_GetEndpointInterval( grp_si iSpeed, grp_usbdi_ep_desc *ptEpDesPtr)
{
grp_u8                          ucInterval  = GRP_USBD_NO_INTERVAL;

    _TRACE_USBC_USBD_(0x59, 0x00, 0x00);

    if( iSpeed == GRP_USBD_HIGH_SPEED ){
        /* High speed */
        if( ((ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK) == GRP_USBD_ED_BMATTR_ISOCRONOUS)
         || ((ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK) == GRP_USBD_ED_BMATTR_INTERRUPT) ){
            if( ptEpDesPtr->bInterval <= 16 ){
                /* Isochronous or interrupt transfer */
                ucInterval = ucHighInterval[ptEpDesPtr->bInterval];
            }
        }
    }
    else {
        /* Full or low speed */
        if( (ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK) == GRP_USBD_ED_BMATTR_ISOCRONOUS ){
            if( ptEpDesPtr->bInterval <= 16 ){
                /* Isochronous transfer */
                ucInterval = ucFullInterval[ptEpDesPtr->bInterval];
            }
        }
        else if( (ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK) == GRP_USBD_ED_BMATTR_INTERRUPT ){
            /* Interrupt transfer */
            if( ptEpDesPtr->bInterval < 2 ){
                ucInterval = GRP_USBD_INTERVAL_1ms;
            }
            else if( ptEpDesPtr->bInterval < 4 ){
                ucInterval = GRP_USBD_INTERVAL_2ms;
            }
            else if( ptEpDesPtr->bInterval < 8 ){
                ucInterval = GRP_USBD_INTERVAL_4ms;
            }
            else if( ptEpDesPtr->bInterval < 16 ){
                ucInterval = GRP_USBD_INTERVAL_8ms;
            }
            else if( ptEpDesPtr->bInterval < 32 ){
                ucInterval = GRP_USBD_INTERVAL_16ms;
            }
            else {
                ucInterval = GRP_USBD_INTERVAL_32ms;
            }
        }
    }

    _TRACE_USBC_USBD_(0x59, 0x00, F_END);

    return ucInterval;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_GetFirstEpDescptr                                                     */
/*                                                                                              */
/* DESCRIPTION: This function finds the first endpoint descriptor which belongs to the          */
/*              specified interface descriptor                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : *ptCnfDesPtr                    Pointer to all of configuration descriptor      */
/*              *ptIfDesPtr                     Pointer to a interface descriptor               */
/* OUTPUT     : **ptEpDesPtr                    Pointer to the endpoint descriptor              */
/*                                              (if return value isn't GRP_USBD_OK, this value  */
/*                                              is invalid)                                     */
/*              **ppvNext2Ptr                   Pointer to the descriptor next to the found     */
/*                                              endpoint descriptor                             */
/*                                              (if return value isn't GRP_USBD_OK, this value  */
/*                                              is invalid)                                     */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_NO_DESCRITOR           There is not endpoint descriptor                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_GetFirstEpDescptr( grp_usbdi_config_desc *ptCnfDesPtr, grp_usbdi_if_desc *ptIfDesPtr, grp_usbdi_ep_desc **ptEpDesPtr, void **ppvNext2Ptr)
{
grp_usbdi_ep_desc               *ptStartEpPtr;
grp_s32                         lRetStatus  = GRP_USBD_NO_DESCRITOR;

    _TRACE_USBC_USBD_(0x5a, 0x00, 0x00);

    /* Check if this is a standard intarface descriptor */
    if( ptIfDesPtr->bDescriptor == GRP_USBD_DESC_INTERFACE ){
        /* set start pointer */
        ptStartEpPtr = (grp_usbdi_ep_desc *)((grp_u32)ptIfDesPtr + (grp_u32)ptIfDesPtr->bLength);

        /* search endpoint descriptor */
        lRetStatus = _grp_usbd_GetNextEpDescptr(ptCnfDesPtr,ptStartEpPtr,ptEpDesPtr,ppvNext2Ptr);
    }

    _TRACE_USBC_USBD_(0x5a, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_GetIfDescptr                                                          */
/*                                                                                              */
/* DESCRIPTION: This function finds the specified interface descriptor which belongs the        */
/*              specified configuration descriptor                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : *ptCnfDesPtr                    Pointer to all of configuration descriptor      */
/*              ucIf                            Interface number                                */
/*              ucAlt                           Alternate setting number Pointer to the found   */
/* OUTPUT     : **ptIf2DesPtr                   interface descriptor                            */
/*                                              (if return value isn't GRP_USBD_OK, this value  */
/*                                              is invalid)                                     */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_NO_DESCRITOR           There is not endpoint descriptor                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_GetIfDescptr(grp_usbdi_config_desc *ptCnfDesPtr, grp_u8 ucIf, grp_u8 ucAlt, grp_usbdi_if_desc **ptIfDes2Ptr )
{
grp_usbdi_if_desc               *ptIfDesPtr;
void                            *pvStartDesPtr;
void                            *pvNextDesPtr   = GRP_USB_NULL;
grp_s32                         lStatus;
grp_s32                         lRetStatus = GRP_USBD_NO_DESCRITOR;

    _TRACE_USBC_USBD_(0x5b, 0x00, 0x00);

    for( pvStartDesPtr = (void *)ptCnfDesPtr; ; pvStartDesPtr = pvNextDesPtr ){
        /* Get interface descriptor pointer */
        lStatus = grp_usbd_SearchDescriptor(ptCnfDesPtr,
                                           GRP_USBD_DESC_INTERFACE,
                                           pvStartDesPtr,
                                           (void **)&ptIfDesPtr,
                                           &pvNextDesPtr);
        if( lStatus == GRP_USBD_OK ){
            /* check interface and alternate number */
            if( (ptIfDesPtr->bInterfaceNumber == ucIf) && (ptIfDesPtr->bAlternateSetting == ucAlt) ){
                /* set interface descriptor pointer */
                *ptIfDes2Ptr    = ptIfDesPtr;
                lRetStatus      = GRP_USBD_OK;
                break;
            }
        }
        else {
            break;
        }
    }

    _TRACE_USBC_USBD_(0x5b, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_GetNextEpDescptr                                                      */
/*                                                                                              */
/* DESCRIPTION: This function finds the endpoint descriptor next to the specified endpoint      */
/*              descriptor                                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : *ptCnfDesPtr                    Pointer to all of configuration descriptor      */
/*              *ptEpDesPtr                     Pointer to a endpoint descriptor The found      */
/* OUTPUT     : *ptEpDes2Ptr                    endpoint descriptor                             */
/*                                              (if return value isn't GRP_USBD_OK, this value  */
/*                                              is invalid)                                     */
/*              **ppvNext2Ptr                   Pointer to the descriptor next to the found     */
/*                                              endpoint descriptor                             */
/*                                              (if return value isn't GRP_USBD_OK, this value  */
/*                                              is invalid)                                     */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Normal end                                      */
/*              GRP_USBD_INVALIED_EP            Invalid device endpoint                         */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_GetNextEpDescptr( grp_usbdi_config_desc *ptCnfDesPtr, grp_usbdi_ep_desc *ptEpDesPtr, grp_usbdi_ep_desc **ptEpDes2Ptr, void **ppvNext2Ptr )
{
grp_usbdi_ep_desc               *ptTmpEpDesPtr;
grp_u32                         ulWTotalLength;
grp_s32                         lRetStatus = GRP_USBD_NO_DESCRITOR;

    _TRACE_USBC_USBD_(0x5c, 0x00, 0x00);

    /* set total length */
    ulWTotalLength = (grp_u32)((grp_u16)(( (grp_usbdi_config_desc_b *)ptCnfDesPtr)->wTotalLength_Low)
                             | (grp_u16)((((grp_usbdi_config_desc_b *)ptCnfDesPtr)->wTotalLength_High) << 8));
                                           
    /* Go to next descriptor */
    for( ptTmpEpDesPtr = ptEpDesPtr;
        (grp_u32)((grp_u32)ptTmpEpDesPtr - (grp_u32)ptCnfDesPtr) < ulWTotalLength;
         ptTmpEpDesPtr = (grp_usbdi_ep_desc *)((grp_u32)ptTmpEpDesPtr + (grp_u32)ptTmpEpDesPtr->bLength) ){

        /* Is this a standard endpoint descriptor ? */
        if( ptTmpEpDesPtr->bDescriptor == GRP_USBD_DESC_ENDPOINT ){
            /* Get first standard endpoint descriptor */
            *ptEpDes2Ptr = ptTmpEpDesPtr;
            *ppvNext2Ptr   = (void *)((grp_u32)ptTmpEpDesPtr + (grp_u32)ptTmpEpDesPtr->bLength);
            lRetStatus   = GRP_USBD_OK;
            break;
        }

        /* Is this a standard interface descriptor ? */
        else if( ptTmpEpDesPtr->bDescriptor == GRP_USBD_DESC_INTERFACE ){
            /* There is not endpoint descriptor */
            lRetStatus = GRP_USBD_NO_DESCRITOR;
            break;
        }
    }
    
    _TRACE_USBC_USBD_(0x5c, 0x00, F_END);

    return lRetStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_GlobalBusControl                                                      */
/*                                                                                              */
/* DESCRIPTION: USB bus power control                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           port control structure                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_HOST_HALT              Host controller is halted                       */
/*              USBDI_NO_FUCNTION               Host controller driver doesn't                  */
/*                                              have this function                              */
/*              GRP_USBD_ILLEGAL_ERROR          Other error                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_GlobalBusControl( grp_usbdi_bus_control *ptReq)
{
grp_hcdi_hc_hdr                 tHcHdr;
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x5d, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->tDev.usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x5d, 0x01, F_END);
        return lStatus;
    }

    /* set host controller handler */
    tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

    if( ptReq->iReq == GRP_USBD_GLOBAL_SUSPEND ){
        /* Global Suspend */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcGlobalSuspend(&tHcHdr);
    }
    else {
        /* Global Resume */
        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcGlobalResume(&tHcHdr);
    }

    /* Status check */
    if( lStatus == GRP_HCDI_OK ){
        /* State change */
        if( ptReq->iReq == GRP_USBD_GLOBAL_SUSPEND ){
            l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucHcState = GRP_USBD_HC_SUSPEND;

#ifdef GRUSB_COMMON_USE_OTG
            /* for otg */
            if( ptReq->tDev.ucPortInfo == GRP_USBD_USB_20_OTG ){
                /* notify otg driver */
                GROTGD_Suspend_Notice();
            }
#endif /* GRUSB_COMMON_USE_OTG */
        }
        else {
            l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ucHcState = GRP_USBD_HC_ACTIVE;
        }
    }

    _TRACE_USBC_USBD_(0x5d, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_IFBandwidthCheck                                                      */
/*                                                                                              */
/* DESCRIPTION: Bandwidth check of selected interface                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Address number                                  */
/*              ptDesc                          Descriptor analysys info.                       */
/*                                              structure                                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_NO_BANDWIDTH           Not enough bandwidth                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_IFBandwidthCheck( grp_u8 ucAddr, grp_usbdi_descriptor_info *ptDesc)
{
grp_u32                         ulOldBandWidth  = 0;
grp_u32                         ulBandWidth     = 0;
grp_s32                         lStatus         = GRP_USBD_OK;

    _TRACE_USBC_USBD_(0x5e, 0x00, 0x00);

    /* Calculate the bandwidth of previous interface alternatively set */
    if( l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptDesc->ucInterface].ucState != GRP_USBD_STATE_BLANK ){
        ulOldBandWidth = _grp_usbd_CalcIfBandwidth(
                                ucAddr,
                                ptDesc->ucInterface,
                                l_tUSBD_CB.atDevTable[ucAddr].atIfInfo[ptDesc->ucInterface].ucAltNum,
                                (grp_usbdi_config_desc *)ptDesc->pvDesc );
    }

    /* Calculate the bandwidth of endpoint for specified interface alternatively set */
    ulBandWidth = _grp_usbd_CalcIfBandwidth( ucAddr,
                                             ptDesc->ucInterface,
                                             ptDesc->ucAlternate,
                                             (grp_usbdi_config_desc *)ptDesc->pvDesc );

    /* Bandwidth currently in use - if previously used bandwidth + total used bandwidth */
    /* > 90%  then NG                                                                   */
    ulBandWidth += l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->ulBandWidth - ulOldBandWidth;
    if( ulBandWidth > GRP_USBD_HC_MAXBANDWIDTH ){
        lStatus = GRP_USBD_NO_BANDWIDTH;
    }

    _TRACE_USBC_USBD_(0x5e, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_InternalDeviceRequest                                                 */
/*                                                                                              */
/* DESCRIPTION: device request(for internal process)                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Transfer request                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_ADR           Invalid address                                 */
/*              GRP_USBD_BUS_SUSPEND            USB bus suspend                                 */
/*              GRP_USBD_HOST_HALT              Host controller HALT                            */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_InternalDeviceRequest( grp_usbdi_st_device_request *ptReq)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x5f, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x5f, 0x01, F_END);
        return lStatus;
    }

    /* Create send packet */
    ptReq->tIrp.ptEndpoint = &l_tUSBD_CB.atDevTable[ucAddr].tDefaultPipe.tEndpoint;
    ptReq->tIrp.iRefCon    = 0;
    ptReq->tIrp.pvUrbPtr   = (void *)ptReq;

    /* Set shortXfer as OK */
    ptReq->tIrp.iShortXferOK = GRP_USB_TRUE;

    /* Set host controller index number */
    ptReq->tIrp.tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

    _TRACE_USBC_USBD_(0x5f, 0x01, 0x00);

    /* Transfer request to host controller driver */
    lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcTrRun(&ptReq->tIrp);

    _TRACE_USBC_USBD_(0x5f, 0x02, 0x00);

    /* Error code check */
    if( lStatus != GRP_HCDI_OK ){
        _TRACE_USBC_USBD_(0x5f, 0x03, 0x00);

        /* Release setup data buffer to usb communication buffer */
        grp_cmem_BlkRel(ptReq->tIrp.pucSetupPtr);

        _TRACE_USBC_USBD_(0x5f, 0x04, 0x00);
    }

    _TRACE_USBC_USBD_(0x5f, 0x00, F_END);

    return lStatus;
}

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
/************************************************************************************************/
/* FUNCTION   : _grp_usbd_InternalPipeCheck                                                     */
/*                                                                                              */
/* DESCRIPTION: Pipe handler status check                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPipe                          Pipe handler to check                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INVALIED_PH            Invalid pipe information                        */
/*              GRP_USBD_ILLEGAL_ERROR          Illegal function                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_InternalPipeCheck( grp_usbdi_pipe *ptPipe)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x60, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptPipe->usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x60, 0x01, F_END);
        return lStatus;
    }

    /* Check the address range */
    if( ucAddr >= GRP_USBD_HOST_MAX_DEVICE ){
        _TRACE_USBC_USBD_(0x60, 0x02, F_END);
        return GRP_USBD_INVALIED_PH;
    }

    /* Default pipe check */
    if( ptPipe->ucEndpointNumber == GRP_USBD_DEFAULT_ENDPOINT ){
        _TRACE_USBC_USBD_(0x60, 0x03, F_END);
        return GRP_USBD_OK;
    }

    /* Check the relationship */
    if( (grp_u32)l_tUSBD_CB.atDevTable[ucAddr].patPipe[ptPipe->ucTransferDirection][ptPipe->ucEndpointNumber]
     != (grp_u32)ptPipe ){
        _TRACE_USBC_USBD_(0x60, 0x04, F_END);
        return GRP_USBD_INVALIED_PH;
    }

    _TRACE_USBC_USBD_(0x60, 0x00, F_END);

    return GRP_USBD_OK;
}
#endif  /* GRP_USB_HOST_NO_PARAM_CHECKING */

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_OpenInterfaceAllPipes                                                 */
/*                                                                                              */
/* DESCRIPTION: This function opens all pipes belonging to the specifeid interface              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : *ptReqPtr                       Standard device request structure               */
/*                                                                                              */
/*              (Member used)                                                                   */
/*              pipe                            Head address of pipe handler of pipe            */
/*                                              corresponding to the first endpoint pertaining  */
/*                                              to interface                                    */
/*              descriptor                      Configuration descriptor configured so that     */
/*                                              interface to be set is pertained                */
/*                                                                                              */
/*              *ptIfDesPtr                     Pointer to interface descriptor                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Successful end                                  */
/*              GRP_USBD_INVALIED_PMTR          Invalid parameter                               */
/*              GRP_USBD_NO_BANDWIDTH           Not enough communication bandwidth in order to  */
/*                                              switch interface                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_OpenInterfaceAllPipes( grp_usbdi_pipe_operate *ptOper, grp_usbdi_if_desc *ptIfDesPtr)
{
grp_usbdi_ep_desc               *ptNextEpPtr;
grp_usbdi_ep_desc               *ptEpDesPtr;
grp_usbdi_pipe                  *ptPipe;
grp_s32                         lStatus;
grp_si                          i;
grp_u8                          ucPipeMax;
grp_u8                          ucPipeCnt   = 0;

    _TRACE_USBC_USBD_(0x61, 0x00, 0x00);

    if( ptIfDesPtr->bNumEndpoints == 0 ){ /* Interface without any endpoint */
        /* In this case, it is judged that Interface Descriptor has no Endpoint Descriptor, */
        /* and function ends normally.                                                      */
        return GRP_USBD_OK;
    }

    /* initialize of the internal flag */
    for (i=0; i<GRP_USBD_SELECT_PIPE; i++) {
        ptOper->atSelPipe[i].ucInternalFlag = GRP_USB_FALSE;
    }

    /* Get the first endpoint descriptor */
    lStatus = _grp_usbd_GetFirstEpDescptr( (grp_usbdi_config_desc *)ptOper->pvDescriptor,
                                            ptIfDesPtr,
                                            &ptEpDesPtr,
                                            (void **)&ptNextEpPtr );

    /* Open all of pipes belonging to the interface */
    ucPipeMax = ptOper->ucPipeNumber;
    for( ucPipeCnt = 0;((lStatus == GRP_USBD_OK) && (ucPipeCnt < ucPipeMax)); ){
        /* check informations */
        lStatus = _grp_usbd_CheckPipeSelect( ptOper, ptEpDesPtr);
        if( lStatus != GRP_USBD_ILLEGAL_ERROR ){
            /* Open the pipe */
            ptPipe = (grp_usbdi_pipe *)(ptOper->ptPipe + lStatus);

            lStatus = grp_usbd_PipeOpen( ptOper->usUsbDevId, ptEpDesPtr, ptPipe );
            if( lStatus == GRP_USBD_OK ){
                ptPipe->ucBelongInterface = ptOper->ucInterface;
                ptPipe->ucBelongAlternate = ptIfDesPtr->bAlternateSetting;
                ucPipeCnt++;
                if( ucPipeCnt >= ucPipeMax ){
                    break;
                }
            }
        }

        /* Get next endpoint descriptor */
        lStatus = _grp_usbd_GetNextEpDescptr( (grp_usbdi_config_desc *)ptOper->pvDescriptor,
                                               ptNextEpPtr,
                                               &ptEpDesPtr,
                                               (void **)&ptNextEpPtr );
    }
    ptOper->ucPipeNumber = ucPipeCnt;

    /* Whether all pipes were opened is checked. */
    for (i=0; i<ucPipeMax; i++) {
        if (ptOper->atSelPipe[i].ucInternalFlag == GRP_USB_FALSE) {
            /* not open pipe */
            lStatus = GRP_USBD_NO_DESCRITOR;
            break;
        }
    }

    _TRACE_USBC_USBD_(0x61, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_PortControl                                                           */
/*                                                                                              */
/* DESCRIPTION: USB port power control                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           port control structure                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_HOST_HALT              Host controller is halted                       */
/*              USBDI_NO_FUCNTION               Host controller driver doesn't have this        */
/*                                              function                                        */
/*              GRP_USBD_INVALIED_PMTR          Invalid parameter                               */
/*              GRP_USBD_ILLEGAL_ERROR          Other error                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_PortControl( grp_usbdi_bus_control *ptReq)
{
grp_hcdi_port_control           tPortReq;
grp_s32                         lStatus;
grp_u8                          ucAddr;
#ifdef GRP_USB_HOST_USE_HUB_DRIVER
grp_u16                         usUsbDevId;
grp_hubd_hub_req                tHubReq;
#endif  /* GRP_USB_HOST_USE_HUB_DRIVER */

    _TRACE_USBC_USBD_(0x62, 0x00, 0x00);

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(ptReq->tDev.usUsbDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x62, 0x01, F_END);
        return lStatus;
    }

    /* Roothub or Hub */
    if( ptReq->tDev.ucHubAddr == GRP_USBD_ROOTHUB_ADDRESS ){
        /* Make a request for HC driver */
        tPortReq.tPortHdr.ucPort              = ptReq->tDev.ucPortNum;
        tPortReq.tPortHdr.tHcHdr.usHcIndexNum = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->usHcIndexNum;

        if( ptReq->iReq == GRP_USBD_PORT_SUSPEND ){
            tPortReq.iPortReq = GRP_USBD_PORT_SUSPEND_CONTROL;
        }
        else if( ptReq->iReq == GRP_USBD_PORT_RESUME ){
            tPortReq.iPortReq = GRP_USBD_PORT_RESUME_CONTROL;
        }
        else {
            tPortReq.iPortReq = GRP_USBD_PORT_RESET_CONTROL;
        }

        lStatus = l_tUSBD_CB.atDevTable[ucAddr].ptHcIndex->tHcdiFunc.pfnHcPortControl(&tPortReq);
        /* Status check */
        if( lStatus == GRP_HCDI_OK ){
            /* State Change */
            if( ptReq->iReq == GRP_USBD_PORT_SUSPEND ){

/*              l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_SUSPEND;*/                /* V1.04 {  */
                if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_OPEN) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_SUSPEND;
                }
                else if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_CONNECT) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_HIBERNATE;
                }                                                                                   /* V1.04 }  */

#ifdef GRUSB_COMMON_USE_OTG
                /* for otg */
                if( ptReq->tDev.ucPortInfo == GRP_USBD_USB_20_OTG ){
                    /* notify otg driver */
                    GROTGD_Suspend_Notice();
                }
#endif /* GRUSB_COMMON_USE_OTG */
            }
            else if( ptReq->iReq == GRP_USBD_PORT_RESUME ){
/*              l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_CONNECT;*/                /* V1.04 {  */
                if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_SUSPEND) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_OPEN;
                }
                else if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_HIBERNATE) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_CONNECT;
                }                                                                                   /* V1.04 }  */
            }
        }
    }

#ifdef GRP_USB_HOST_USE_HUB_DRIVER
    else {
        /* Make a request for Hub driver */
        if (_grp_usbd_GetDevId( ptReq->tDev.ucHubAddr, &usUsbDevId) != GRP_USBD_OK) {
            return GRP_USBD_ILLEGAL_ERROR;
        }
        tHubReq.usUsbDevId   = usUsbDevId;
        tHubReq.ucPortNumber = ptReq->tDev.ucPortNum;

        switch (ptReq->iReq) {
        case GRP_USBD_PORT_SUSPEND:
            tHubReq.usFeature = GRP_HUBD_FS_PS;

            lStatus = grp_hubd_SetPortFeature( &tHubReq);
            if (lStatus == GRP_HUBD_OK ){
                /* State Change */
/*              l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_SUSPEND;*/                /* V1.04 {  */
                if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_OPEN) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_SUSPEND;
                }
                else if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_CONNECT) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_HIBERNATE;
                }                                                                                   /* V1.04 }  */
            }
            break;

        case GRP_USBD_PORT_RESUME:
            tHubReq.usFeature = GRP_HUBD_FS_PS;

            lStatus = grp_hubd_ClearPortFeature( &tHubReq);
            if (lStatus == GRP_HUBD_OK ){
                /* State Change */
/*              l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_CONNECT;*/                /* V1.04 {  */
                if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_SUSPEND) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_OPEN;
                }
                else if (l_tUSBD_CB.atDevTable[ucAddr].iPortState == GRP_USBD_STATE_HIBERNATE) {
                    l_tUSBD_CB.atDevTable[ucAddr].iPortState = GRP_USBD_STATE_CONNECT;
                }                                                                                   /* V1.04 }  */
            }
            break;

        case GRP_USBD_PORT_RESET:
            lStatus = lStatus = grp_hubd_ReEnumeration( &tHubReq);
            break;

        default:
            lStatus = GRP_USBD_INVALIED_PMTR;
            break;
        }
    }
#endif /* GRP_USB_HOST_USE_HUB_DRIVER */

    _TRACE_USBC_USBD_(0x62, 0x00, F_END);

    return lStatus;

}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_SearchHcidx                                                           */
/*                                                                                              */
/* DESCRIPTION: Search host controller index                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usHcIndexNum                    Host controller index number                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : more than 0                     Host controller index                           */
/*              -1                              No host controller index                        */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_si _grp_usbd_SearchHcidx( grp_u16 usHcIndexNum)
{
grp_si                          iHcNum = -1;     /* error */
grp_si                          i;

    _TRACE_USBC_USBD_(0x63, 0x00, 0x00);

    /* Search all table */
    for( i = 0;i < GRP_USBC_HOST_QTY_CONTROLLER;i++ ){
        if( l_tUSBD_CB.atHcData[i].usHcIndexNum == usHcIndexNum ){
            /* set hc index */
            iHcNum = i;
            break;
        }
    }

    _TRACE_USBC_USBD_(0x63, 0x00, F_END);

    return iHcNum;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_InitResource                                                          */
/*                                                                                              */
/* DESCRIPTION: Initialize internal resource                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_INIT_ERROR             porting module error                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_InitResource(void)
{
grp_s32                         lStatus;
grp_s8                          acName1[] = "sUsbdA";
grp_s8                          acName2[] = "sUsbdD";
grp_s8                          acName3[] = "sUsbdO";

    /* Initialize address table semaphore */
    lStatus = grp_vos_CreateSemaphore(&l_tUSBD_CB.ptSem[GRP_USBD_ADDRESS_SEM],(grp_u8 *)&acName1[0],1);
    if( lStatus != GRP_VOS_POS_RESULT ){
        return GRP_USBD_INIT_ERROR;
    }

    /* Initialize default pipe semaphore */
    lStatus = grp_vos_CreateSemaphore(&l_tUSBD_CB.ptSem[GRP_USBD_DEFAULTPIPE_SEM],(grp_u8 *)&acName2[0],1);
    if( lStatus != GRP_VOS_POS_RESULT ){
        return GRP_USBD_INIT_ERROR;
    }

    /* Initialize open pipe semaphore */
    lStatus = grp_vos_CreateSemaphore(&l_tUSBD_CB.ptSem[GRP_USBD_OPENPIPE_SEM],(grp_u8 *)&acName3[0],1);
    if( lStatus != GRP_VOS_POS_RESULT ){
        return GRP_USBD_INIT_ERROR;
    }

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_InitControlBlock                                                      */
/*                                                                                              */
/* DESCRIPTION: Initialize internal resource                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_usbd_InitControlBlock(void)
{
grp_si                          i;

    /* Initialize device id table and device table */
    for( i = 0;i < GRP_USBD_HOST_MAX_DEVICE;i++ ){
        l_tUSBD_CB.atDevTable[i].iPortState = GRP_USBD_STATE_BLANK;
        l_tUSBD_CB.ausDevIdTable[i]         = GRP_USBD_DEVID_NO_ASIGNED;
    }
    l_tUSBD_CB.ausDevIdTable[0]             = GRP_USBD_DEFAULT_DEVID;

    /* Initialize default pipe area */
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.usUsbDevId          = GRP_USBD_DEFAULT_DEVID;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.ucEndpointNumber    = GRP_USBD_DEFAULT_ENDPOINT;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.iTransferMode       = GRP_USBD_CONTROL;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.ucTransferDirection = GRP_USBD_TX_INOUT;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.ulInterval          = 0;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.iStatus             = GRP_USBD_PIPE_HALT;

    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.tEndpoint.ucAddress = GRP_USBD_DEFAULT_ADDRESS;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.tEndpoint.ucEpNum   = GRP_USBD_DEFAULT_ENDPOINT;
    l_tUSBD_CB.atDevTable[GRP_USBD_DEFAULT_ADDRESS].tDefaultPipe.tEndpoint.ucTxMode  = GRP_USBD_CONTROL;

    return;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_InitHcControlBlock                                                    */
/*                                                                                              */
/* DESCRIPTION: Initialize host controller control block                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_HOST_INIT_ERROR        Host controller driver Initialization error     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_InitHcControlBlock(void)
{
grp_hcdi_system_init            atHcdInit[GRP_USBC_HOST_QTY_CONTROLLER];
grp_si                          i;
grp_s32                         lStatus;

    /* Initialize host controller control block */
    for( i = 0;i < GRP_USBC_HOST_QTY_CONTROLLER;i++ ){
        atHcdInit[i].ptHcIoFunc    = &l_tUSBD_CB.atHcData[i].tHcdiFunc;
    }

    /* Initialize host controller driver */
    lStatus = grp_hcm_Init(&atHcdInit[0]);
    if( lStatus == GRP_HCDI_OK ){
        for( i = 0;i < GRP_USBC_HOST_QTY_CONTROLLER;i++ ){
            if( atHcdInit[i].iStatus == GRP_HCDI_OK ){
                /* Initialize host controller index data */
                l_tUSBD_CB.atHcData[i].ulBandWidth        = 0;
                l_tUSBD_CB.atHcData[i].ucMasterCliantAddr = GRP_USBD_NO_CLIANT_ADDRESS;
                l_tUSBD_CB.atHcData[i].ucHcState          = GRP_USBD_HC_ACTIVE;
                l_tUSBD_CB.atHcData[i].usHcIndexNum       = atHcdInit[i].usHcIndexNum;
                l_tUSBD_CB.atHcData[i].ulHostDelay        = atHcdInit[i].ulHostDelay;
                l_tUSBD_CB.atHcData[i].ulLsSetup          = atHcdInit[i].ulLsSetup;
            }
            else {
                l_tUSBD_CB.atHcData[i].ucHcState = GRP_USBD_HC_HALTED;
            }
        }
    }
    else {
        /* All Host controller is halt state */
        for( i = 0;i < GRP_USBC_HOST_QTY_CONTROLLER;i++ ){
            l_tUSBD_CB.atHcData[i].ucHcState = GRP_USBD_HC_HALTED;
        }

        lStatus = GRP_USBD_HOST_INIT_ERROR;
    }

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_SearchUSB20Hub                                                        */
/*                                                                                              */
/* DESCRIPTION: Search USB2.0 Hub                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Device Address                                  */
/* OUTPUT     : ucHubAddr                       USB2.0 Hub Address                              */
/*                                              (0 is No USB2.0 Hub)                            */
/*              ucPortNum                       USB2.0 Port Number                              */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_SearchUSB20Hub( grp_u8 ucAddr, grp_u8 *pucHubAddr, grp_u8 *pucPortNum)
{
grp_usbdi_device_info           tDevInfo;
grp_s32                         lStatus         = GRP_USBD_ILLEGAL_ERROR;
grp_u32                         ulNum           = GRP_USBD_HOST_MAX_DEVICE;
grp_u16                         usSearchDevId;
grp_u8                          ucSearchAddr    = ucAddr;

    _TRACE_USBC_USBD_(0x64, 0x00, 0x00);

    /* Search Device Address */
    while( ulNum-- ){
        /* Get device identifier */
        lStatus = _grp_usbd_GetDevId(ucSearchAddr, &usSearchDevId);
        if( lStatus != GRP_USBD_OK ){
            _TRACE_USBC_USBD_(0x64, 0x01, F_END);
            return lStatus;
        }

        /* Get Device Information */
        lStatus = grp_usbd_GetDeviceInfo(usSearchDevId,&tDevInfo);
        if( lStatus != GRP_USBD_OK ){
            _TRACE_USBC_USBD_(0x64, 0x02, F_END);
            return GRP_USBD_ILLEGAL_ERROR;
        }

        /* Check USB2.0 Hub */
        if( tDevInfo.ucPortInfo == GRP_USBD_USB_20 ){
            *pucHubAddr = tDevInfo.ucHubAddr;
            *pucPortNum = tDevInfo.ucPortNum;
            break;
        }

        /* check Roothub */
        if( tDevInfo.ucHubAddr == GRP_USBD_ROOTHUB_ADDRESS ){
            /* No USB2.0 Hub */
            *pucHubAddr = 0;
            *pucPortNum = 0;
            break;
        }

        /* continue search */
        ucSearchAddr = tDevInfo.ucHubAddr;
    }

    _TRACE_USBC_USBD_(0x64, 0x00, F_END);

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_SearchDevId                                                           */
/*                                                                                              */
/* DESCRIPTION: Search valid address number                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : More than 1                     Valid address number                            */
/*              GRP_USBD_NO_ADDRESS             cannot obtain address number                    */
/*              GRP_USBD_OS_RELATED_ERROR       OS related error                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_SearchDevId( grp_u16 *usDevId)
{
grp_u16                         usAddr;
/*static grp_u16                  l_usDevTag = GRP_USBD_DEVID_TAG_MIN;*//* reinit */

    _TRACE_USBC_USBD_(0x65, 0x00, 0x00);

    if( usDevId == GRP_USB_NULL ){
        return GRP_USBD_INVALIED_PMTR;
    }

    /* Exclusive control when address released */
    if( GRP_USBD_OK != grp_usbd_LockAddress() ){
        _TRACE_USBC_USBD_(0x65, 0x01, F_END);
        return GRP_USBD_OS_RELATED_ERROR;
    }

    /* Tag increment */
    l_usDevTag++;
    if( l_usDevTag > GRP_USBD_DEVID_TAG_MAX ){
        l_usDevTag = GRP_USBD_DEVID_TAG_MIN;
    }

    /* Search address table */
    for( usAddr = GRP_USBD_DEVICE_BASE_ADDRESS; usAddr < GRP_USBD_HOST_MAX_DEVICE; usAddr++ ){
        if( l_tUSBD_CB.ausDevIdTable[usAddr] == GRP_USBD_DEVID_NO_ASIGNED ){

            *usDevId = (grp_u16)(usAddr | (l_usDevTag << 8));

            /* Asigned device identifier */
            l_tUSBD_CB.ausDevIdTable[usAddr] = *usDevId;

            break;
        }
    }

    /* Cancel exclusive control */
    grp_usbd_UnlockAddress();

    /* Return error if there is no more free device table available */
    if( usAddr >= GRP_USBD_HOST_MAX_DEVICE ){
        return GRP_USBD_TOO_MANY_DEVICE;
    }

    _TRACE_USBC_USBD_(0x65, 0x00, F_END);

    return GRP_USBD_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_ReleaseDevId                                                          */
/*                                                                                              */
/* DESCRIPTION: Release valid address number                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Valid address number                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_OS_RELATED_ERROR       OS related error                                */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_ReleaseDevId( grp_u16 usDevId)
{
grp_s32                         lStatus;
grp_u8                          ucAddr;

    _TRACE_USBC_USBD_(0x66, 0x00, 0x00);

    /* Exclusive control when address released */
    if( GRP_USBD_OK != grp_usbd_LockAddress() ){
        _TRACE_USBC_USBD_(0x66, 0x01, F_END);
        return GRP_USBD_OS_RELATED_ERROR;
    }

    /* Get Device Address */
    lStatus = _grp_usbd_GetAddress(usDevId, &ucAddr);
    if( lStatus != GRP_USBD_OK ){
        _TRACE_USBC_USBD_(0x66, 0x02, 0x01);
    }
    else if( ucAddr != 0 ){
        /* Unconfig device */
        l_tUSBD_CB.atDevTable[ucAddr].ulConfNum = 0;

        /* Release address table */
        l_tUSBD_CB.ausDevIdTable[ucAddr] = GRP_USBD_DEVID_NO_ASIGNED;
    }

    /* Cancel exclusive control */
    grp_usbd_UnlockAddress();

    _TRACE_USBC_USBD_(0x66, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_GetAddress                                                            */
/*                                                                                              */
/* DESCRIPTION: Get device address by device identifier                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usDevId                         Device identifier                               */
/*              ucAddr                          Area for getting the device address             */
/* OUTPUT     : *ucAddr                         Device address                                  */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_GetAddress( grp_u16 usDevId, grp_u8 *pucAddr )
{
grp_s32     lStatus  = GRP_USBD_OK;
grp_u16     usTag;

    _TRACE_USBC_USBD_(0x67, 0x00, 0x00);

    *pucAddr = (grp_u8)(usDevId & GRP_USBD_DEVID_ADDR_MASK);
    usTag   = (grp_u16)(usDevId >> 8);

    if( (l_tUSBD_CB.ausDevIdTable[(*pucAddr)] != usDevId) && (usTag != 0) ){
        lStatus = GRP_USBD_ILLEGAL_ERROR;
        *pucAddr = 0;
    }

    _TRACE_USBC_USBD_(0x67, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_GetDevId                                                              */
/*                                                                                              */
/* DESCRIPTION: Get device identifier by device address                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucAddr                          Device address                                  */
/*              usDevId                         Area for getting the device identifier          */
/* OUTPUT     : *usDevId                        Device identifier                               */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_GetDevId( grp_u8 ucAddr, grp_u16 *pusDevId )
{
grp_s32     lStatus  = GRP_USBD_OK;
grp_u16     usTag;

    _TRACE_USBC_USBD_(0x68, 0x00, 0x00);

    *pusDevId   = l_tUSBD_CB.ausDevIdTable[ucAddr];
    usTag       = (grp_u16)(*pusDevId >> 8);

    if( (ucAddr != (grp_u8)((*pusDevId) & GRP_USBD_DEVID_ADDR_MASK))
     && (usTag != 0) ){
        lStatus = GRP_USBD_ILLEGAL_ERROR;
        *pusDevId = 0;
    }

    _TRACE_USBC_USBD_(0x68, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_usbd_CheckPipeSelect                                                       */
/*                                                                                              */
/* DESCRIPTION: Get device identifier by device address                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptOper                          Pipe operation structure                        */
/*              *ptEpDesPtr                     Pointer to a endpoint descriptor                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : integer number                  Success                                         */
/*              GRP_USBD_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_usbd_CheckPipeSelect( grp_usbdi_pipe_operate *ptOper, grp_usbdi_ep_desc *ptEpDesPtr )
{
grp_usbdi_pipe_select           *ptPipeSel  = ptOper->atSelPipe;
grp_s32                         lStatus     = GRP_USBD_ILLEGAL_ERROR;
grp_si                          i;
grp_u8                          ucTransferMode;
grp_u8                          ucTransferDir;


    _TRACE_USBC_USBD_(0x69, 0x00, 0x00);

    ucTransferMode = (grp_u8)(ptEpDesPtr->bmAttributes & GRP_USBD_ED_BMATTR_MASK);
    ucTransferDir  = (grp_u8)(((ptEpDesPtr->bEndpointAddress & GRP_USBD_ED_BEPADDRESS_PID_BIT) == GRP_USBD_ED_BEPADDRESS_PID_BIT)
                              ? GRP_USBD_TX_IN : GRP_USBD_TX_OUT);

    /* search information */
    for (i=0; i<GRP_USBD_SELECT_PIPE; i++, ptPipeSel++) {
        if (ptPipeSel->ucInternalFlag == GRP_USB_TRUE) {
            /* already used */
            continue;
        }
        /* check Endpoint descriptor */
        if ((ptPipeSel->ucTransferMode      == ucTransferMode)
         && (ptPipeSel->ucTransferDirection == ucTransferDir)) {
            /* success */
            lStatus = i;
            /* set flag */
            ptPipeSel->ucInternalFlag = GRP_USB_TRUE;
            break;
        }
    }

    _TRACE_USBC_USBD_(0x69, 0x00, F_END);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_dlocal_init_usbd_c                                                         */
/*                                                                                              */
/* DESCRIPTION: reinitialize DLOCAL variables for reinit                                        */
/*              to took care of USB re-initialization                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void _grp_dlocal_init_usbd_c()
{
    l_usDevTag = GRP_USBD_DEVID_TAG_MIN;
}
