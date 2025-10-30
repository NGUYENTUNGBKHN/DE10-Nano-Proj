/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2008 Grape Systems, Inc.                          */
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
/*      grp_msc_drv.c                                                           1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver.                                                 */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created new policy initial version                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Correction by member change in structure.                       */
/*                              - grp_usbdi_device_request                                      */
/*                                  pfnCallbackFunc -> pfnDvCbFunc                              */
/*                              - grp_usbdi_st_device_request                                   */
/*                                  pfnCallbackFunc -> pfnStCbFunc                              */
/*                              - grp_msc_reg                                                   */
/*                                  pfnCallback -> pfnMscEvCallback                             */
/*                              - _grp_msc_staff                                                */
/*                                  pfnCallback -> pfnMscSfCallback                             */
/*                            - Modified value type.                                            */
/*                              - grp_msc_Init                                                  */
/*                                  grp_u32 ulUse -> grp_s32 lUse                               */
/*                            - Addition of initialization to uninitalization data.             */
/*                              - l_tMscBotSeq                                                  */
/*                              - l_tMscCbiSeq                                                  */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_msc_local.h"


/***** LOCAL PARAMETER DEFINITIONS **************************************************************/
DLOCAL grp_msc_set              l_atMscSet[GRP_MSC_DEVICE_MAX];
DLOCAL grp_msc_reg              l_atMscReg[GRP_MSC_REG_MAX];

DLOCAL grp_vos_t_semaphore      *l_ptMscSem;

#ifdef GRP_MSC_USE_BOT
DLOCAL grp_msc_seq              l_tMscBotSeq = {0};
#endif /* GRP_MSC_USE_BOT */

#ifdef GRP_MSC_USE_CBI
DLOCAL grp_msc_seq              l_tMscCbiSeq = {0};
#endif /* GRP_MSC_USE_CBI */


/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/
LOCAL grp_s32 _msc_SubClassInit( void *pvRsv );
LOCAL grp_s32 _msc_ClearPipeCallback( grp_usbdi_st_device_request *ptStdReq );
LOCAL grp_s32 _msc_EventCallback( grp_cnfsft_notify *ptNotify );
#ifdef GRP_MSC_USE_VENDOR_SUBCLASS
LOCAL grp_s32 _msc_VendorCallback( grp_cnfsft_notify *ptNotify );
#endif /* GRP_MSC_USE_VENDOR_SUBCLASS */
LOCAL grp_s32 _msc_SetConfigCallback( grp_usbdi_st_device_request *ptReq );
LOCAL grp_s32 _msc_EventAttached( grp_cnfsft_notify *ptCnfsftNotify );
LOCAL grp_s32 _msc_EventDetached( grp_cnfsft_notify *ptNotify );
LOCAL grp_s32 _msc_GetMaxLunCallback( grp_usbdi_device_request *ptReq );
LOCAL grp_s32 _msc_SearchEmptySet( grp_u32 *pulIndex );
LOCAL grp_s32 _msc_SearchRelevantSet( grp_u16 usDevId, grp_u32 *pulIndex );
LOCAL grp_s32 _msc_SearchEmptyReg( grp_msc_reg *ptMscReg, grp_u32 *pulIndex );
LOCAL grp_s32 _msc_CallbackCopy( grp_msc_reg *ptMscReg );


/************************************************************************************************/
/* FUNCTION     : grp_msc_Init                                                                  */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : pvRsv                         Reserved argument.                              */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                GRP_MSC_VOS_ERROR             Virtual OS error                                */
/*                GRP_MSC_CMEM_ERROR            CMEM module error                               */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_Init( void *pvRsv )
{
grp_si                          i;
grp_si                          j;
grp_s32                         lUse    = GRP_MSC_ERROR;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x01, 0x00, 0x00);

#ifdef GRP_MSC_USE_BOT
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_BotInit( &l_tMscBotSeq, pvRsv );
        lUse = GRP_MSC_OK;
    }
#endif /* GRP_MSC_USE_BOT */

#ifdef GRP_MSC_USE_CBI
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_CbiInit( &l_tMscCbiSeq, pvRsv );
        lUse = GRP_MSC_OK;
    }
#endif /* GRP_MSC_USE_CBI */

    if( lUse != GRP_MSC_OK ){
        lStatus = GRP_MSC_ERROR;
    }

    _TRACE_MSC_DRV_(0x01, (grp_u8)lStatus, 0x01);

    /* Initialize semaphore */
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_vos_CreateSemaphore( &l_ptMscSem, GRP_MSC_SEM_NAME, 1 );
        if( lStatus != GRP_VOS_POS_RESULT ){
            lStatus = GRP_MSC_VOS_ERROR;
        }
    }

    if( lStatus == GRP_MSC_OK ){
        for(i=0;i<GRP_MSC_DEVICE_MAX;i++){
            l_atMscSet[i].ptMscReg          = GRP_USB_NULL;
            l_atMscSet[i].ucInterfaceNum    = 0;
            l_atMscSet[i].ucAlternateNum    = 0;
            l_atMscSet[i].usUsbDevId        = GRP_MSC_DEFAULT_DEVID;
            l_atMscSet[i].ulStatus          = GRP_MSC_BLANK;
            l_atMscSet[i].pucConfigDesc     = GRP_USB_NULL;
            l_atMscSet[i].pucInterfaseDesc  = GRP_USB_NULL;
            l_atMscSet[i].ptCmd             = GRP_USB_NULL;
            l_atMscSet[i].pfnProtocolSeq    = GRP_USB_NULL;
            l_atMscSet[i].usPhase           = GRP_MSC_PHASE_NON;

            l_atMscSet[i].tStdReq.pvReferenceP = GRP_USB_NULL;
            l_atMscSet[i].tDevReq.pvReferenceP = GRP_USB_NULL;
            l_atMscSet[i].tNmlReq.pvReferenceP = GRP_USB_NULL;

            for(j=0;j<GRP_MSC_PIPE_MAX;j++){
                l_atMscSet[i].atPipe[j].usUsbDevId = GRP_MSC_DEFAULT_DEVID;
            }
            /* get cmem area */
            if (grp_cmem_BlkGet( GRP_CMEM_ID_MSC_COMMAND, (void **)&l_atMscSet[i].pucCmd) != GRP_CMEM_OK) {
                _TRACE_MSC_DRV_(0x01, 0x01, _F_END);
                return GRP_MSC_CMEM_ERROR;
            }
            if (grp_cmem_BlkGet( GRP_CMEM_ID_MSC_STATUS,  (void **)&l_atMscSet[i].pucSts) != GRP_CMEM_OK) {
                _TRACE_MSC_DRV_(0x01, 0x02, _F_END);
                return GRP_MSC_CMEM_ERROR;
            }
        }

        for(i=0;i<GRP_MSC_REG_MAX;i++){
            l_atMscReg[i].ulStatus          = GRP_MSC_FREE;
            l_atMscReg[i].ucSubClass        = 0;
            l_atMscReg[i].ucProtocol        = 0;
            l_atMscReg[i].pfnMscEvCallback  = GRP_USB_NULL;
            l_atMscReg[i].pvUserRef         = 0;
            l_atMscReg[i].usVendorId        = 0;
            l_atMscReg[i].usProductId       = 0;
            l_atMscReg[i].ucMode            = 0;
        }
    }

    if( lStatus == GRP_MSC_OK ){
         lStatus = _msc_SubClassInit( pvRsv );
    }

    _TRACE_MSC_DRV_(0x01, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Register                                                              */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        :                                                                               */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_Register( grp_msc_reg *ptUsrReg )
{
grp_cnfsft_registration         tCnfsft;
grp_msc_reg                     *ptMscReg;
grp_u32                         ulIndex;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x02, 0x00, 0x00);

    lStatus = _msc_SearchEmptyReg( ptUsrReg, &ulIndex );
    if( lStatus == GRP_MSC_OK ){
        ptMscReg = &l_atMscReg[ulIndex];

        ptMscReg->ucSubClass        = ptUsrReg->ucSubClass;
        ptMscReg->ucProtocol        = ptUsrReg->ucProtocol;
        ptMscReg->pfnMscEvCallback  = ptUsrReg->pfnMscEvCallback;
        ptMscReg->pvUserRef         = ptUsrReg->pvUserRef;
        ptMscReg->usVendorId        = ptUsrReg->usVendorId;
        ptMscReg->usProductId       = ptUsrReg->usProductId;
        ptMscReg->ucMode            = ptUsrReg->ucMode;

        tCnfsft.usVendorID          = ptUsrReg->usVendorId;
        tCnfsft.usProductID         = ptUsrReg->usProductId;
        tCnfsft.ucInterfaceClass    = GRP_MSC_CLASS_CODE;
        tCnfsft.ucInterfaceSubClass = ptUsrReg->ucSubClass;
        tCnfsft.ucInterfaceProtocol = ptUsrReg->ucProtocol;
        tCnfsft.pfnEventNotification= _msc_EventCallback;
        tCnfsft.pvReference         = (void *)ptMscReg;

        if( ptMscReg->ucMode == GRP_MSC_REG_PROTOCOL ){
            tCnfsft.usLoadOption = GRP_CNFSFT_PROTOCOL_SPECIFIED;
        }
        else{ /* ptMscReg->ucMode == GRP_MSC_REG_VENDOR */
            tCnfsft.usLoadOption = GRP_CNFSFT_VENDOR_SPECIFIED;

#ifdef GRP_MSC_USE_VENDOR_SUBCLASS
            tCnfsft.pfnEventNotification = _msc_VendorCallback;
            lStatus = _msc_CallbackCopy( ptMscReg );
#endif /* GRP_MSC_USE_VENDOR_SUBCLASS */
        }

        if( lStatus == GRP_MSC_OK ){
            lStatus = grp_cnfsft_SetNotification( &tCnfsft );
        }
    }

    _TRACE_MSC_DRV_(0x02, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Open                                                                  */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_Open( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x03, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    lStatus = ptMscSet->pfnProtocolSeq->pfnOpen( ptCmd );

    _TRACE_MSC_DRV_(0x03, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Close                                                                 */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_Close( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x04, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    lStatus = ptMscSet->pfnProtocolSeq->pfnClose( ptCmd );

    _TRACE_MSC_DRV_(0x04, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_ReqCmd                                                                */
/*                                                                                              */
/* DESCRIPTION  : Issue transefr request.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_ReqCmd( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x05, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    if( ptMscSet->ulStatus == GRP_MSC_OPEN ){
        ptCmd->tStaff.ptNextCmd = GRP_USB_NULL;
        /* assume no error */
        ptCmd->tStaff.lStatus = GRP_MSC_NO_FAIL;
        ptCmd->tStaff.ulRetry = 0;

        lStatus = ptMscSet->pfnProtocolSeq->pfnCmdReq( ptCmd );
    }

    _TRACE_MSC_DRV_(0x05, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Cancel                                                                */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_Cancel( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x06, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    lStatus = ptMscSet->pfnProtocolSeq->pfnCancel( ptCmd );

    _TRACE_MSC_DRV_(0x06, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Abort                                                                 */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_Abort( grp_msc_set *ptMscSet )
{
grp_msc_cmd                     *ptNextCmd;
grp_msc_cmd                     *ptFirstCmd;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x07, 0x00, 0x00);

    ptFirstCmd = GRP_USB_NULL;

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptFirstCmd      = ptMscSet->ptCmd;
    ptMscSet->ptCmd = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* No request aborted */
    if( ptFirstCmd == GRP_USB_NULL ){
        return( GRP_MSC_OK );
    }

    ptNextCmd = ptFirstCmd;
    while( ptNextCmd != GRP_USB_NULL ){
        if( ptNextCmd->pfnCallback != GRP_USB_NULL ){
            ptNextCmd->lStatus = GRP_MSC_CANCEL;
            if( ptNextCmd != ptFirstCmd ){
                ptNextCmd->pfnCallback( ptNextCmd );
            }
        }

        ptNextCmd = ptNextCmd->tStaff.ptNextCmd;
    }

    lStatus = ptMscSet->pfnProtocolSeq->pfnAbort( ptFirstCmd );
    if( (lStatus != GRP_MSC_OK) && (ptFirstCmd->pfnCallback != GRP_USB_NULL) ){
        ptFirstCmd->pfnCallback( ptFirstCmd );
    }

    _TRACE_MSC_DRV_(0x07, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Reset                                                                 */
/*                                                                                              */
/* DESCRIPTION  : A reset of various kind is specified to the device.                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Mass storage command                            */
/*                ulMode                        Reset mode                                      */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_Reset( grp_msc_cmd *ptCmd, grp_u32 ulMode )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_bus_control           tBusCtrl;
grp_u16                         usDevId;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x08, 0x00, 0x00);

    switch( ulMode ){
    case GRP_MSC_RESET_MASS:
        /* Command block reset in case of CBI   */
        /* Mass storage reset in case of BOT    */
        ptMscSet = ptCmd->hMscHdr;
        lStatus  = ptMscSet->pfnProtocolSeq->pfnReset( ptCmd );
        break;

    case GRP_MSC_RESET_ENUM:
        lStatus = grp_msc_GetDeviceId( ptCmd->hMscHdr, &usDevId );
        if( lStatus != GRP_MSC_OK ){
            break;
        }

        lStatus = grp_usbd_GetDeviceInfo( usDevId, &tBusCtrl.tDev );
        if( lStatus != GRP_USBD_OK ){
            break;
        }

        tBusCtrl.iReq = GRP_USBD_PORT_RESET;
        lStatus = grp_usbd_BusPowerControl( &tBusCtrl );
        break;

    default:
        lStatus = GRP_MSC_NOSUPPORT;
        break;
    }

    _TRACE_MSC_DRV_(0x08, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_GetMaxLun( BOT Only )                                                 */
/*                                                                                              */
/* DESCRIPTION  : Get number of logical unit of device.                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_GetMaxLun( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_device_request        *ptDevReq;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x09, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    if( ptMscSet->ptMscReg->ucProtocol != GRP_MSC_BOT_CODE ){
        return( GRP_MSC_NOSUPPORT );
    }

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptDevReq = &ptMscSet->tDevReq;
    if( ptDevReq->pvReferenceP == GRP_USB_NULL ){
        ptDevReq->pvReferenceP = ptCmd;
    }

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* if BUSY, please retry upper layer */
    if( ptDevReq->pvReferenceP != ptCmd ){
        return( GRP_MSC_BUSY );
    }

    if( ptCmd->tStaff.ulRetry > GRP_MSC_MAX_RETRY ){
        ptCmd->tStaff.ulRetry = 0;
        /* If the device doesn't support this command, */
        /* it is assumed that it has one LUN.          */
        if( ptCmd->tStaff.lStatus == GRP_USBD_TR_STALL ){
            ptDevReq->tIrp.ulActualLength = 1;
            ptDevReq->lStatus             = GRP_USBD_TR_NO_FAIL;
            ptDevReq->pucBuffer[0]        = 0;
        }
        else{
            ptDevReq->lStatus = ptCmd->tStaff.lStatus;
        }

        _msc_GetMaxLunCallback( ptDevReq );
    }
    else{
        ptDevReq->usUsbDevId        = ptMscSet->usUsbDevId;
        ptDevReq->bmRequestType     = GRP_USBD_TYPE_DEV2HOST
                                    | GRP_USBD_TYPE_CLASS
                                    | GRP_USBD_TYPE_INTERFACE;
        ptDevReq->bRequest          = GRP_BOT_GETLUN;
        ptDevReq->wValue            = 0;
        ptDevReq->wIndex            = ptMscSet->ucInterfaceNum;
        ptDevReq->pucBuffer         = ptCmd->pucReqBuffer;
        ptDevReq->wLength           = 1;
        ptDevReq->pfnDvCbFunc       = _msc_GetMaxLunCallback;

        lStatus = grp_usbd_DeviceRequest( ptDevReq );
    }

    _TRACE_MSC_DRV_(0x09, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_GetMaxLun( BOT Only )                                                 */
/*                                                                                              */
/* DESCRIPTION  : Get number of logical unit of device.                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_GetMaxLunCancel( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_device_request        *ptDevReq;
grp_msc_cmd                     *ptDevCmd;
grp_s32                         lStatus;

    _TRACE_MSC_DRV_(0x0A, 0x00, 0x00);

    ptDevCmd = GRP_USB_NULL;
    ptMscSet = ptCmd->hMscHdr;
    if( ptMscSet->ptMscReg->ucProtocol != GRP_MSC_BOT_CODE ){
        return( GRP_MSC_NOSUPPORT );
    }

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptDevReq = &ptMscSet->tDevReq;
    ptDevCmd = ptDevReq->pvReferenceP;

    ptDevReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* Alreday completed */
    if( ptDevCmd == GRP_USB_NULL ){
        return( GRP_MSC_ERROR );
    }

    lStatus = grp_usbd_DeviceRequestCancel( ptDevReq );
    if( lStatus != GRP_USBD_OK ){
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->lStatus = GRP_MSC_ILLEGAL_FAIL;
            ptCmd->pfnCallback( ptCmd );
        }
    }

    _TRACE_MSC_DRV_(0x0A, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_GetDeviceId                                                           */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_GetDeviceId( grp_msc_hdr hMscHdr, grp_u16 *pusDevId )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x0B, 0x00, 0x00);

    *pusDevId = hMscHdr->usUsbDevId;

    _TRACE_MSC_DRV_(0x0B, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_GetSubClass                                                           */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_GetSubClass( grp_msc_hdr hMscHdr, grp_u8 *pucSubClass )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x0C, 0x00, 0x00);

    *pucSubClass = hMscHdr->ucSubClass;

    _TRACE_MSC_DRV_(0x0C, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_ClearPipe                                                             */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_ClearPipe( grp_msc_cmd *ptCmd, grp_u8 ucEpNum )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_st_device_request     *ptStdReq;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x21, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptMscSet = ptCmd->hMscHdr;
    ptStdReq = &ptMscSet->tStdReq;
    if( ptStdReq->pvReferenceP == GRP_USB_NULL ){
        ptStdReq->pvReferenceP = ptCmd;
    }

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* if BUSY, please retry upper layer */
    if( ptStdReq->pvReferenceP != ptCmd ){
        return( GRP_MSC_BUSY );
    }

    if( ucEpNum != GRP_USBD_DEFAULT_ENDPOINT ){
        switch( ptMscSet->usPhase ){
        case GRP_MSC_PHASE_COMMAND: /* NO BREAK */
        case GRP_MSC_PHASE_DOUT:
            ptStdReq->ptPipe = ptMscSet->tPipes.ptBulkOut;
            break;

        case GRP_MSC_PHASE_DIN:     /* NO BREAK */
        case GRP_MSC_PHASE_STATUS:
            ptStdReq->ptPipe = ptMscSet->tPipes.ptBulkIn;
            break;

        default:
            lStatus = GRP_MSC_ERROR;
            break;
        }

        if( lStatus == GRP_MSC_OK ){
            ptStdReq->pfnStCbFunc     = _msc_ClearPipeCallback;
            ptStdReq->pvReferenceP    = (void *)ptCmd;

            lStatus = grp_usbd_PipeActive( ptStdReq );
        }
    }
    else{ /* GRP_USBD_DEFAULT_ENDPOINT */
        ptStdReq->usUsbDevId        = ptMscSet->usUsbDevId;
        ptStdReq->ucFeatureSelector = GRP_USBD_ENDPOINT_HALT;
        ptStdReq->ucEndpoint        = GRP_USBD_DEFAULT_ENDPOINT;
        ptStdReq->pfnStCbFunc       = _msc_ClearPipeCallback;
        ptStdReq->pvReferenceP      = (void *)ptCmd;

        lStatus = grp_usbd_ClearFeature( ptStdReq );
    }

    _TRACE_MSC_DRV_(0x21, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_ClearAllPipes                                                         */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_ClearAllPipes( grp_msc_pipes *ptPipes )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x22, 0x00, 0x00);

    ptPipes->ptBulkIn  = GRP_USB_NULL;
    ptPipes->ptBulkOut = GRP_USB_NULL;
    ptPipes->ptIntrIn  = GRP_USB_NULL;

    _TRACE_MSC_DRV_(0x22, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_LinkCmd                                                               */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_LinkCmd( grp_msc_set *ptMscSet, grp_msc_cmd *ptCmd )
{
grp_msc_cmd                     *ptNextCmd;
grp_s32                         lBusy;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x23, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    lBusy = GRP_USB_FALSE;

    if( ptMscSet->ptCmd == GRP_USB_NULL ){
        ptMscSet->ptCmd = ptCmd;
    }
    else {
        lBusy     = GRP_USB_TRUE;
        ptNextCmd = ptMscSet->ptCmd;
        while( ptNextCmd->tStaff.ptNextCmd != GRP_USB_NULL ){
            ptNextCmd = ptNextCmd->tStaff.ptNextCmd;
        }
        ptNextCmd->tStaff.ptNextCmd = ptCmd;
    }

    ptCmd->tStaff.ptNextCmd = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    if( lBusy == GRP_USB_FALSE ){
        lStatus = ptMscSet->pfnProtocolSeq->pfnRun( ptCmd );
        if( lStatus != GRP_MSC_OK ){
            if( GRP_MSC_OK != grp_msc_UnlinkCmd( ptMscSet, ptCmd ) ){
                lStatus = GRP_MSC_ERROR;
            }
        }
    }

    _TRACE_MSC_DRV_(0x23, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_UnlinkCmd                                                             */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_UnlinkCmd( grp_msc_set *ptMscSet, grp_msc_cmd *ptCmd )
{
grp_msc_cmd                     *ptNextCmd;
grp_s32                         lRun;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x24, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    lRun = GRP_USB_FALSE;

    if( ptMscSet->ptCmd == ptCmd ){
        /* Complete request unlink */
        lRun = GRP_USB_TRUE;
        ptMscSet->ptCmd = ptMscSet->ptCmd->tStaff.ptNextCmd;
    }
    else if( ptMscSet->ptCmd == GRP_USB_NULL ){
        lStatus = GRP_MSC_ERROR;
    }
    else{
        /* Cancel request unlink */
        ptNextCmd = ptMscSet->ptCmd;
        while( ptNextCmd->tStaff.ptNextCmd != ptCmd ){
            if( ptNextCmd->tStaff.ptNextCmd == GRP_USB_NULL ){
                lStatus = GRP_MSC_ERROR;
                break;
            }
            if (lStatus == GRP_MSC_OK) {
                ptNextCmd = ptNextCmd->tStaff.ptNextCmd;
            }
        }
        ptNextCmd->tStaff.ptNextCmd = ptNextCmd->tStaff.ptNextCmd->tStaff.ptNextCmd;
    }

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    if( (lRun == GRP_USB_TRUE)
     && (ptMscSet->ptCmd != GRP_USB_NULL)
     && (lStatus == GRP_MSC_OK) ){
        lStatus = ptMscSet->pfnProtocolSeq->pfnRun( ptMscSet->ptCmd );
        if( lStatus != GRP_MSC_OK ){
            /* Recursive grp_msc_UnlinkCmd Recursive call */
            if( GRP_MSC_OK != grp_msc_UnlinkCmd( ptMscSet, ptMscSet->ptCmd ) ){
                lStatus = GRP_MSC_ERROR;
            }
        }
    }

    _TRACE_MSC_DRV_(0x24, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Lock_Sem                                                              */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_Lock_Sem( void )
{
grp_s32                         lStatus = GRP_MSC_OK;

    if( grp_vos_GetSemaphore( l_ptMscSem, GRP_VOS_INFINITE ) != GRP_VOS_POS_RESULT) {
        lStatus = GRP_MSC_ERROR;
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Unlock_Sem                                                            */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_Unlock_Sem( void )
{
grp_s32                         lStatus = GRP_MSC_OK;

    if( grp_vos_ReleaseSemaphore( l_ptMscSem ) != GRP_VOS_POS_RESULT ){
        lStatus = GRP_MSC_ERROR;
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_SubClassInit                                                             */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : pvRsv                         Reserved argument.                              */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_SubClassInit( void *pvRsv )
{
grp_s32                         lStatus = GRP_MSC_OK;

    pvRsv = GRP_USB_NULL;

#ifdef GRP_MSC_USE_ATAPI
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_AtapiInit( );
    }
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_Sff8070iInit( );
    }
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_ScsiInit( );
    }
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_UfiInit( );
    }
#endif /* GRP_MSC_USE_UFI */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_GetMaxLunCallback                                                        */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
LOCAL grp_s32 _msc_GetMaxLunCallback( grp_usbdi_device_request *ptDevReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_u32                         ulNextSeq = GRP_MSC_SEQ_NORMAL;
grp_s32                         lStatus   = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x41, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptCmd = ptDevReq->pvReferenceP;
    ptDevReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* Already canceled */
    if( ptCmd == GRP_USB_NULL ){
        return( GRP_MSC_OK );
    }

    ptMscSet = ptCmd->hMscHdr;
    ptCmd->ulActualLength = ptDevReq->tIrp.ulActualLength;

    switch( ptDevReq->lStatus ){
    case GRP_USBD_TR_NO_FAIL:
        ptCmd->lStatus = GRP_MSC_NO_FAIL;
        break;

    case GRP_USBD_TR_CANCEL:
        ptCmd->lStatus = GRP_MSC_CANCEL;
        break;

    case GRP_USBD_TR_TIMEOUT:
        ptCmd->lStatus = GRP_MSC_TIMEOUT;
        break;

    case GRP_USBD_TR_STALL:
        ulNextSeq = GRP_MSC_SEQ_CLRPIPE;
        break;

    default:
        ptCmd->lStatus = ptDevReq->lStatus;
        break;
    }

    if( ulNextSeq == GRP_MSC_SEQ_NORMAL ){
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }

    if( ulNextSeq == GRP_MSC_SEQ_CLRPIPE ){
        ptCmd->tStaff.pfnMscSfCallback = (grp_msc_request_callback)grp_msc_GetMaxLun;
        ptCmd->tStaff.pvParam          = ptCmd;
        ptCmd->tStaff.lStatus          = ptDevReq->lStatus;
        ptCmd->tStaff.ulRetry++;

        ptDevReq->lStatus = GRP_USBD_TR_NO_FAIL; /* For next request */

        lStatus = grp_msc_ClearPipe( ptCmd, GRP_USBD_DEFAULT_ENDPOINT );
        if( lStatus != GRP_MSC_OK ){
            ptCmd->lStatus = lStatus;
            ptCmd->pfnCallback( ptCmd );
        }
    }

    _TRACE_MSC_DRV_(0x41, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ClearPipeCallback                                                        */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_ClearPipeCallback( grp_usbdi_st_device_request *ptStdReq )
{
grp_msc_cmd                     *ptCmd;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_DRV_(0x42, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptCmd = ptStdReq->pvReferenceP;
    ptStdReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* Already canceled */
    if( ptCmd == GRP_USB_NULL )
        return( GRP_MSC_OK );

    lStatus = ptCmd->tStaff.pfnMscSfCallback( ptCmd->tStaff.pvParam );

    _TRACE_MSC_DRV_(0x42, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_EventCallback                                                            */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_EventCallback( grp_cnfsft_notify *ptNotify )
{
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x43, 0x00, 0x00);

    switch( ptNotify->usEvent ){
    case GRP_CNFSFT_DEVICE_ATTACHED:
        lStatus = _msc_EventAttached( ptNotify );
        break;

    case GRP_CNFSFT_DEVICE_DETACHED:
        lStatus = _msc_EventDetached( ptNotify );
        break;

    default:
        break;
    }

    _TRACE_MSC_DRV_(0x43, 0x00, _F_END);

    return( lStatus );
}

#ifdef GRP_MSC_USE_VENDOR_SUBCLASS
/************************************************************************************************/
/* FUNCTION     : _msc_VendorCallback                                                           */
/*                                                                                              */
/* DESCRIPTION  : If use vendor specific subclass on fsif, this function                        */
/*                callback when device attach or detach.                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_VendorCallback( grp_cnfsft_notify *ptNotify )
{
grp_usbdi_st_device_request     *ptStdReq;
grp_msc_reg                     *ptMscReg;
grp_msc_set                     *ptMscSet;
grp_usbdi_config_desc_b         *ptConfDesc;
grp_u32                         ulIndex;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x44, 0x00, 0x00);

    ptMscReg = (grp_msc_reg *)ptNotify->pvReference;
    if( ptMscReg->ucMode == GRP_MSC_REG_VENDOR ){
        if( ptNotify->usEvent == GRP_CNFSFT_DEVICE_ATTACHED ){
            /* For none cached memory for request */
            lStatus = _msc_SearchEmptySet( &ulIndex );
            if( lStatus == GRP_MSC_OK ){
                ptMscSet = &l_atMscSet[ulIndex];

                ptNotify->ucInterfaceNum = ptMscReg->ucInputIfNum;
                ptMscSet->ptMscReg       = (grp_msc_reg *)ptNotify->pvReference;
                ptNotify->pvReference    = ptMscSet;

                ptConfDesc = (grp_usbdi_config_desc_b*)ptNotify->pucConfDesc;
                ptStdReq   = &ptMscSet->tStdReq;

                ptStdReq->usUsbDevId        = ptNotify->usUsbDevId;
                ptStdReq->ucConfiguration   = ptConfDesc->bConfigurationValue;
                ptStdReq->pvDescriptor      = ptConfDesc;
                ptStdReq->ulSize            = (grp_u16)((((grp_u16)ptConfDesc->wTotalLength_Low) & (grp_u16)0x00FF)
                                            | ((grp_u16)((grp_u16)(ptConfDesc->wTotalLength_High) << 8) & (grp_u16)0xFF00));
                ptStdReq->pfnStCbFunc       = _msc_SetConfigCallback;
                ptStdReq->pvReferenceP      = ptNotify;

                lStatus = grp_usbd_SetConfiguration( ptStdReq );
            }
        }
        else{
            lStatus = _msc_EventCallback( ptNotify );
        }
    }

    _TRACE_MSC_DRV_(0x44, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_SetConfigCallback                                                        */
/*                                                                                              */
/* DESCRIPTION  : This callback for doing set configuration when using                          */
/*                vendor specific subclass on fsif.                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptReq                         set config request                              */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_SetConfigCallback( grp_usbdi_st_device_request *ptStdReq )
{
grp_cnfsft_notify               *ptNotify;
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x45, 0x00, 0x00);

    if( ptStdReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        ptNotify = (grp_cnfsft_notify *)ptStdReq->pvReferenceP;
        ptMscSet = (grp_msc_set *)ptNotify->pvReference;

        ptNotify->pvReference = (grp_msc_reg *)ptMscSet->ptMscReg;

        ptMscSet->ulStatus = GRP_MSC_BLANK;

        lStatus = _msc_EventAttached( ptNotify );
    }

    _TRACE_MSC_DRV_(0x45, 0x00, _F_END);

    return( lStatus );
}
#endif /* GRP_MSC_USE_VENDOR_SUBCLASS */

/************************************************************************************************/
/* FUNCTION     : _msc_EventAttached                                                            */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_EventAttached( grp_cnfsft_notify *ptCnfsftNotify )
{
grp_msc_reg                     *ptMscReg;
grp_msc_set                     *ptMscSet;
grp_msc_notify                  tMscNotify;
grp_u32                         ulIndex;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x46, 0x00, 0x00);

    ptMscReg = (grp_msc_reg *)ptCnfsftNotify->pvReference;

    lStatus = _msc_SearchEmptySet( &ulIndex );
    if( lStatus == GRP_MSC_OK ){
        ptMscSet = &l_atMscSet[ulIndex];

        ptMscSet->ptMscReg         = ptMscReg;
        ptMscSet->ucInterfaceNum   = ptCnfsftNotify->ucInterfaceNum;
        ptMscSet->ucAlternateNum   = 0;
        ptMscSet->usUsbDevId       = ptCnfsftNotify->usUsbDevId;
        ptMscSet->ulStatus         = GRP_MSC_CONNECT;
        ptMscSet->pucConfigDesc    = ptCnfsftNotify->pucConfDesc;
        ptMscSet->pucInterfaseDesc = ptCnfsftNotify->pucInterfaceDesc;
        ptMscSet->ptCmd            = GRP_USB_NULL;
        ptMscSet->usPhase          = GRP_MSC_PHASE_NON;

        lStatus = GRP_MSC_UNINIT_ERROR;

#ifdef GRP_MSC_USE_BOT
        if( ptMscReg->ucProtocol == GRP_MSC_BOT_CODE ){
            if( l_tMscBotSeq.ulStatus == GRP_MSC_USE ){
                ptMscSet->pfnProtocolSeq = &l_tMscBotSeq;
                lStatus = GRP_MSC_OK;
            }
        }
#endif /* GRP_MSC_USE_BOT */

#ifdef GRP_MSC_USE_CBI
        if( (ptMscReg->ucProtocol == GRP_MSC_CBI_CODE)
        || (ptMscReg->ucProtocol == GRP_MSC_CB_CODE) ){
            if( l_tMscCbiSeq.ulStatus == GRP_MSC_USE ){
                ptMscSet->pfnProtocolSeq = &l_tMscCbiSeq;
                lStatus = GRP_MSC_OK;
            }
        }
#endif /* GRP_MSC_USE_CBI */

        if( lStatus == GRP_MSC_OK ){
            ptMscSet->ucSubClass = ptMscReg->ucSubClass;

            if( ptMscReg->pfnMscEvCallback != GRP_USB_NULL ){
                tMscNotify.iEvent     = GRP_MSC_ATTACHED;
                tMscNotify.hMscHdr    = ptMscSet;
                tMscNotify.ucSubClass = ptMscSet->ucSubClass;
                tMscNotify.ucProtocol = ptMscReg->ucProtocol;
                tMscNotify.pvUserRef  = ptMscReg->pvUserRef;

                lStatus = ptMscReg->pfnMscEvCallback( &tMscNotify );
            }
        }
    }

    _TRACE_MSC_DRV_(0x46, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_EventDetached                                                            */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_EventDetached( grp_cnfsft_notify *ptNotify )
{
grp_msc_reg                     *ptMscReg;
grp_msc_set                     *ptMscSet   = GRP_USB_NULL;
grp_msc_notify                  tMscNotify;
grp_u32                         ulIndex;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x47, 0x00, 0x00);

    ptMscReg = (grp_msc_reg *)ptNotify->pvReference;

    lStatus = _msc_SearchRelevantSet( ptNotify->usUsbDevId, &ulIndex );
    if( lStatus == GRP_MSC_OK ){
        ptMscSet = &l_atMscSet[ulIndex];

        if( ptMscSet->ptMscReg != ptMscReg ){
            lStatus = GRP_MSC_ERROR;
        }
    }

    if( lStatus == GRP_MSC_OK ){
        switch( ptMscSet->ulStatus ){
        case GRP_MSC_CONNECT:       /* NO BREAK */
        case GRP_MSC_HIBERNATE:
            ptMscSet->ulStatus = GRP_MSC_BLANK;
            break;

        case GRP_MSC_OPEN:          /* NO BREAK */
        case GRP_MSC_SUSPEND:
            ptMscSet->ulStatus = GRP_MSC_DISCONNECT;
            break;

        default:
            ptMscSet->ulStatus = GRP_MSC_BLANK;
            lStatus = GRP_MSC_ERROR;
            break;
        }

        if( ptMscReg->pfnMscEvCallback != GRP_USB_NULL ){
            tMscNotify.iEvent     = GRP_MSC_DETACHED;
            tMscNotify.hMscHdr    = ptMscSet;
            tMscNotify.ucSubClass = ptMscSet->ucSubClass;
            tMscNotify.ucProtocol = ptMscReg->ucProtocol;
            tMscNotify.pvUserRef  = ptMscReg->pvUserRef;

            lStatus = ptMscReg->pfnMscEvCallback( &tMscNotify );
        }
    }

    _TRACE_MSC_DRV_(0x47, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_SearchEmptySet                                                           */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : *pulIndex                     Index number of GRP_MSC_SET                     */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_SearchEmptySet( grp_u32 *pulIndex )
{
grp_u32                         i;
grp_s32                         lStatus = GRP_MSC_RESOURCE_ERROR;

    _TRACE_MSC_DRV_(0x48, 0x00, 0x00);

    for(i=0;i<GRP_MSC_DEVICE_MAX;i++){
        if( l_atMscSet[i].ulStatus == GRP_MSC_BLANK ){
            /* Get empty */
            *pulIndex = i;
            l_atMscSet[i].ulStatus = GRP_MSC_CONNECT;
            lStatus = GRP_MSC_OK;

            break;
        }
    }

    _TRACE_MSC_DRV_(0x48, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_SearchRelevantSet                                                        */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : *pulIndex                     Index number of GRP_MSC_SET                     */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _msc_SearchRelevantSet( grp_u16 usDevId, grp_u32 *pulIndex )
{
grp_u32                         i;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x49, 0x00, 0x00);

    for(i=0;i<GRP_MSC_DEVICE_MAX;i++){
        if( l_atMscSet[i].ulStatus != GRP_MSC_BLANK ){
            if( l_atMscSet[i].usUsbDevId == usDevId ){
                _TRACE_MSC_DRV_(0x49, 0x00, 0x01);

                /* Get relevant */
                *pulIndex = i;
                lStatus   = GRP_MSC_OK;
                break;
            }
        }
    }

    _TRACE_MSC_DRV_(0x49, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_SearchEmptyReg                                                           */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : *pulIndex                     Index number of GRP_MSC_REG                     */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
LOCAL grp_s32 _msc_SearchEmptyReg( grp_msc_reg *ptMscReg, grp_u32 *pulIndex )
{
grp_u32                         i;
grp_s32                         lStatus = GRP_MSC_RESOURCE_ERROR;

    _TRACE_MSC_DRV_(0x4A, 0x00, 0x00);

    for(i=0;i<GRP_MSC_REG_MAX;i++){
        if( l_atMscReg[i].ulStatus == GRP_MSC_FREE ){
            if( lStatus == GRP_MSC_RESOURCE_ERROR ){
                /* Get empty */
                *pulIndex = i;

                l_atMscReg[i].ulStatus = GRP_MSC_USE;
                lStatus = GRP_MSC_OK;
            }
        }
        else if( l_atMscReg[i].ucMode == ptMscReg->ucMode ){
            /* Duplicate check */
            if( ptMscReg->ucMode == GRP_MSC_REG_PROTOCOL ){
                if( (l_atMscReg[i].ucSubClass == ptMscReg->ucSubClass)
                && (l_atMscReg[i].ucProtocol == ptMscReg->ucProtocol) ){
                    lStatus = GRP_MSC_ERROR;

                    break;
                }
            }
            else{ /* ptMscReg->ucMode == GRP_MSC_REG_VENDOR */
                lStatus = GRP_MSC_ERROR;        /* No support at Ver1.00 */

                break;
            }
        }
    }

    _TRACE_MSC_DRV_(0x4A, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_CallbackCopy                                                             */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptMscReg                                                                      */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
LOCAL grp_s32 _msc_CallbackCopy( grp_msc_reg *ptMscReg )
{
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_DRV_(0x4B, 0x00, 0x00);

    if( ptMscReg->pfnMscEvCallback == GRP_USB_NULL ){
        if( l_atMscReg[0].ulStatus == GRP_MSC_USE ){
            /* Copy first register callback. Maybe fsif */
            ptMscReg->pfnMscEvCallback = l_atMscReg[0].pfnMscEvCallback;
            lStatus = GRP_MSC_OK;
        }
    }

    _TRACE_MSC_DRV_(0x4B, 0x00, _F_END);

    return( lStatus );
}
