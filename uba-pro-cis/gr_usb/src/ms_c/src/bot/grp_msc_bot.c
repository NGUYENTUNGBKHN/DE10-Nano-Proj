/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2015 Grape Systems, Inc.                          */
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
/*      grp_msc_bot.c                                                           1.02            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver  Bulk Only Protocol.                             */
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
/*                              - grp_usbdi_request                                             */
/*                                  pfnCallbackFunc -> pfnNrCbFunc                              */
/*                              - grp_usbdi_device_request                                      */
/*                                  pfnCallbackFunc -> pfnDvCbFunc                              */
/*                              - grp_usbdi_st_device_request                                   */
/*                                  pfnCallbackFunc -> pfnStCbFunc                              */
/*                              - _grp_msc_staff                                                */
/*                                  pfnCallback -> pfnMscSfCallback                             */
/*   K.Kaneko       2015/12/07  V1.02                                                           */
/*                            - When the grp_msc_ClearPipe function of an error-recovery        */
/*                              processing makes an error, correct so that a request may be     */
/*                              deleted.                                                        */
/*                              - _grp_msc_BotCbwCallback                                       */
/*                              - _grp_msc_BotDataCallback                                      */
/*                              - _grp_msc_BotCswCallback                                       */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_msc_bot.h"


/***** LOCAL PARAMETER DEFINITIONS **************************************************************/
DLOCAL grp_u32          l_ulMscTag;


/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/
LOCAL grp_s32 _grp_msc_BotOpen( grp_msc_cmd *ptCmd );
LOCAL grp_s32 _grp_msc_BotClose( grp_msc_cmd *ptCmd );
LOCAL grp_s32 _grp_msc_BotCmdReq( grp_msc_cmd *ptCmd );
LOCAL grp_s32 _grp_msc_BotCancel( grp_msc_cmd *ptCmd );
LOCAL grp_s32 _grp_msc_BotAbort( grp_msc_cmd *ptCmd );
LOCAL grp_s32 _grp_msc_BotReset( grp_msc_cmd *ptCmd );
LOCAL grp_s32 _grp_msc_BotRun( grp_msc_cmd *ptCmd );
LOCAL grp_s32 _grp_msc_BotDel( grp_msc_cmd *ptCmd );

LOCAL grp_s32 _grp_msc_BotCbwCallback( grp_usbdi_request *ptNmlReq );
LOCAL grp_s32 _grp_msc_BotDataCallback( grp_usbdi_request *ptNmlReq );
LOCAL grp_s32 _grp_msc_BotCswCallback( grp_usbdi_request *ptNmlReq );
LOCAL grp_s32 _grp_msc_BotSetCbw( grp_msc_cmd *ptCmd, grp_msc_cbw *ptCbw );
LOCAL grp_s32 _grp_msc_BotCheckCsw( grp_msc_cmd *ptCmd, grp_msc_csw *ptCsw );
LOCAL grp_s32 _grp_msc_BotMakeTag( grp_u32 *NowTag );
#ifdef GRP_MSC_BOT_MS_RESET
LOCAL grp_s32 _grp_msc_BotInPipeRecovery( grp_usbdi_device_request *ptDevReq );
LOCAL grp_s32 _grp_msc_BotOutPipeRecovery( grp_usbdi_st_device_request *ptStdReq);
LOCAL grp_s32 _grp_msc_BotPipeRecoveryComp( grp_usbdi_st_device_request *ptStdReq);
#endif  /* GRP_MSC_BOT_MS_RESET */
LOCAL grp_s32 _grp_msc_BotRetryCswCb( void *pvData );


/************************************************************************************************/
/* FUNCTION     : grp_msc_BotInit                                                               */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : SeqFunc                       Bot sequence function pointer.                  */
/*                pvRsv                         Reserved argument.                              */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_BotInit( grp_msc_seq *SeqFunc, void *pvRsv )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x01, 0x00, 0x00);

    pvRsv = GRP_USB_NULL;

    l_ulMscTag = GRP_MSC_FIRST_TAG_NUM;

    if( SeqFunc->ulStatus != GRP_MSC_USE ){
        SeqFunc->pfnOpen   = _grp_msc_BotOpen;
        SeqFunc->pfnClose  = _grp_msc_BotClose;
        SeqFunc->pfnCmdReq = _grp_msc_BotCmdReq;
        SeqFunc->pfnCancel = _grp_msc_BotCancel;
        SeqFunc->pfnAbort  = _grp_msc_BotAbort;
        SeqFunc->pfnReset  = _grp_msc_BotReset;

        SeqFunc->pfnRun    = _grp_msc_BotRun;
        SeqFunc->pfnDel    = _grp_msc_BotDel;

        SeqFunc->ulStatus  = GRP_MSC_USE;
    }

    _TRACE_MSC_BOT_(0x01, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_BotOpen                                                               */
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
LOCAL grp_s32 _grp_msc_BotOpen( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_pipe_operate          tOper;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x21, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    if( ptMscSet->ulStatus != GRP_MSC_CONNECT ){
        return( lStatus );
    }

    tOper.usUsbDevId                        = ptMscSet->usUsbDevId;
    tOper.ucInterface                       = ptMscSet->ucInterfaceNum;
    tOper.ptPipe                            = ptMscSet->atPipe;
    tOper.ucPipeNumber                      = GRP_MSC_BOT_PIPES;
    tOper.atSelPipe[0].ucTransferMode       = GRP_USBD_BULK;
    tOper.atSelPipe[0].ucTransferDirection  = GRP_USBD_TX_OUT;
    tOper.atSelPipe[1].ucTransferMode       = GRP_USBD_BULK;
    tOper.atSelPipe[1].ucTransferDirection  = GRP_USBD_TX_IN;

    /* Interface open */
    lStatus = grp_usbd_InterfaceOpen( &tOper );
    if( lStatus == GRP_MSC_OK ){
        /* clear all paipes informations */
        lStatus = grp_msc_ClearAllPipes( &ptMscSet->tPipes );
    }

    if( lStatus == GRP_MSC_OK ){
        ptMscSet->tPipes.ptBulkOut = &ptMscSet->atPipe[0];
        ptMscSet->tPipes.ptBulkIn  = &ptMscSet->atPipe[1];

        /* OK */
        ptMscSet->ulStatus = GRP_MSC_OPEN;
        lStatus            = GRP_MSC_OK;
    }
    else{
        /* close pipes */
        ptMscSet->ulStatus = GRP_MSC_DISCONNECT;
        _grp_msc_BotClose( ptCmd);
    }

    _TRACE_MSC_BOT_(0x21, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_BotClose                                                              */
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
LOCAL grp_s32 _grp_msc_BotClose( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_pipe_operate          tOper;
grp_u32                         ulNowSta;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x22, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    if( (ptMscSet->ulStatus != GRP_MSC_OPEN)
     && (ptMscSet->ulStatus != GRP_MSC_DISCONNECT) ){
        return( lStatus );
    }

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    if( ptMscSet->ptCmd != GRP_USB_NULL ){
        /* unlock */                                                                    /* }    */
        if( grp_msc_Unlock_Sem() != GRP_MSC_OK) {
            return( GRP_MSC_ERROR );
        }

        return( GRP_MSC_BUSY );
    }

    ulNowSta = ptMscSet->ulStatus;
    ptMscSet->ulStatus = GRP_MSC_CLOSE;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    tOper.usUsbDevId                    = ptMscSet->usUsbDevId;
    tOper.ucInterface                   = ptMscSet->ucInterfaceNum;
    tOper.ptPipe                        = ptMscSet->atPipe;
    tOper.ucPipeNumber                  = GRP_MSC_BOT_PIPES;
    tOper.atSelPipe[0].ucTransferMode   = GRP_USBD_UNKNOWN_MODE;

    lStatus = grp_usbd_InterfaceClose( &tOper );
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_ClearAllPipes( &ptMscSet->tPipes );
        if( ulNowSta == GRP_MSC_OPEN ){
            ptMscSet->ulStatus = GRP_MSC_CONNECT;
        }
        else if( ulNowSta == GRP_MSC_DISCONNECT ){
            ptMscSet->ulStatus = GRP_MSC_BLANK;
        }
        else{
            ptMscSet->ulStatus = GRP_MSC_BLANK;
            lStatus = GRP_MSC_ERROR;
        }
    }
    else{
        ptMscSet->ulStatus = GRP_MSC_BLANK;
    }

    _TRACE_MSC_BOT_(0x22, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_BotCmdReq                                                             */
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
LOCAL grp_s32 _grp_msc_BotCmdReq( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x23, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    lStatus = grp_msc_LinkCmd( ptMscSet, ptCmd );

    _TRACE_MSC_BOT_(0x23, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotCancel                                                            */
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
LOCAL grp_s32 _grp_msc_BotCancel( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x24, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    if( (grp_msc_cmd *)ptMscSet->tDevReq.pvReferenceP == ptCmd ){
        lStatus = grp_msc_GetMaxLunCancel( ptCmd );
    }
    else{
        lStatus = grp_usbd_NormalRequestCancel( &ptMscSet->tNmlReq );
        if( lStatus == GRP_USBD_OK ){
            lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
        }
        /* If error do nothing, because already completed. */
    }

    _TRACE_MSC_BOT_(0x24, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotAbort                                                             */
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
LOCAL grp_s32 _grp_msc_BotAbort( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x25, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    lStatus = grp_usbd_NormalRequestCancel( &ptMscSet->tNmlReq );

    _TRACE_MSC_BOT_(0x25, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotReset                                                             */
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
LOCAL grp_s32 _grp_msc_BotReset( grp_msc_cmd *ptCmd )
{
#ifdef GRP_MSC_BOT_MS_RESET
grp_msc_set                     *ptMscSet;
grp_usbdi_device_request        *ptDevReq;
grp_s32                         lStatus = GRP_MSC_ERROR;

    /*--- Reset recovery ---*/
    _TRACE_MSC_BOT_(0x26, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptDevReq = &ptMscSet->tDevReq;
    if( ptDevReq->pvReferenceP == GRP_USB_NULL ){
        ptDevReq->pvReferenceP = (void *)ptCmd;
    }

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* if BUSY, please retry upper layer */
    if( (grp_msc_cmd *)ptDevReq->pvReferenceP != ptCmd ){
        return( GRP_MSC_BUSY );
    }

    /* set parameters */
    ptDevReq->usUsbDevId        = ptMscSet->usUsbDevId;
    ptDevReq->bmRequestType     = GRP_USBD_TYPE_HOST2DEV
                                | GRP_USBD_TYPE_CLASS
                                | GRP_USBD_TYPE_INTERFACE;
    ptDevReq->bRequest          = GRP_BOT_RESET;
    ptDevReq->wValue            = 0;
    ptDevReq->wIndex            = ptMscSet->ucInterfaceNum;
    ptDevReq->pucBuffer         = GRP_USB_NULL;
    ptDevReq->wLength           = 0;
    ptDevReq->pfnDvCbFunc       = _grp_msc_BotInPipeRecovery;
    /* MS reset command */
    lStatus = grp_usbd_DeviceRequest( ptDevReq );

    _TRACE_MSC_BOT_(0x26, 0x00, _F_END);

    return( lStatus );
#else   /* GRP_MSC_BOT_MS_RESET */

    ptCmd = GRP_USB_NULL;   /* Warning measures */

    return GRP_MSC_ERROR;
#endif  /* GRP_MSC_BOT_MS_RESET */
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotRun                                                               */
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
LOCAL grp_s32 _grp_msc_BotRun( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x27, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    ptCmd->ulActualLength = 0;
    ptCmd->lStatus        = GRP_MSC_NOT_PROCESS;

    ptMscSet->usPhase = GRP_MSC_PHASE_COMMAND;

    /* create CBW data */
    _grp_msc_BotSetCbw( ptCmd, (grp_msc_cbw *)ptMscSet->pucCmd );

    ptMscSet->tNmlReq.ptPipe          = ptMscSet->tPipes.ptBulkOut;
    ptMscSet->tNmlReq.pucBuffer       = ptMscSet->pucCmd;
    ptMscSet->tNmlReq.ulBufferLength  = GRP_MSC_CBW_LENGTH;
    ptMscSet->tNmlReq.lStatus         = GRP_MSC_NOT_PROCESS;
    ptMscSet->tNmlReq.pfnNrCbFunc     = _grp_msc_BotCbwCallback;
    ptMscSet->tNmlReq.pvReferenceP    = (void *)ptCmd;

    lStatus = grp_usbd_NormalRequest( &ptMscSet->tNmlReq );

    _TRACE_MSC_BOT_(0x27, 0x00, 0x00);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotDel                                                               */
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
LOCAL grp_s32 _grp_msc_BotDel( grp_msc_cmd *ptCmd )
{
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x28, 0x00, 0x00);

    ptCmd = GRP_USB_NULL;   /* Warning measures */

    _TRACE_MSC_BOT_(0x28, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotCbwCallback                                                       */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptPipes                       Pipes that hopes to clear                       */
/* OUTPUT       : *ptPipes                      Clear by NULL                                   */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _grp_msc_BotCbwCallback( grp_usbdi_request *ptNmlReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x29, 0x00, 0x00);

    ptCmd    = (grp_msc_cmd *)ptNmlReq->pvReferenceP;
    ptMscSet = ptCmd->hMscHdr;
    if( ptMscSet->ptCmd != ptCmd ){
        /* Allready canceled */
        ptCmd->lStatus = GRP_MSC_CANCEL;
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }
    else if( ptNmlReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        /* Transaction success */
        if( ptCmd->ulReqLength == 0 ){
            ptNmlReq->ulActualLength = 0;
            lStatus = _grp_msc_BotDataCallback( ptNmlReq );
        }
        else{
            if( ptCmd->ucDir == GRP_USBD_TX_IN ){
                ptMscSet->usPhase              = GRP_MSC_PHASE_DIN;
                ptMscSet->tNmlReq.iShortXferOK = GRP_USB_TRUE;

                ptNmlReq->ptPipe = ptMscSet->tPipes.ptBulkIn;
            }
            else{ /* GRP_USBD_TX_OUT */
                ptMscSet->usPhase              = GRP_MSC_PHASE_DOUT;

                ptNmlReq->ptPipe = ptMscSet->tPipes.ptBulkOut;
            }
            ptNmlReq->pucBuffer       = ptCmd->pucReqBuffer;
            ptNmlReq->ulBufferLength  = ptCmd->ulReqLength;
            ptNmlReq->lStatus         = GRP_MSC_NOT_PROCESS;
            ptNmlReq->pfnNrCbFunc     = _grp_msc_BotDataCallback;
            ptNmlReq->pvReferenceP    = (void *)ptCmd;

            lStatus = grp_usbd_NormalRequest( ptNmlReq );
            if( lStatus != GRP_USBD_OK ){
                lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );

                if( ptCmd->pfnCallback != GRP_USB_NULL ){
                    ptCmd->pfnCallback( ptCmd );
                }
            }
        }
    }
    else{
        /* Transaction error */
        switch( ptNmlReq->lStatus ){
        case GRP_USBD_TR_STALL:
            lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );

            ptCmd->tStaff.pfnMscSfCallback = (grp_msc_request_callback)ptCmd->pfnCallback;
            ptCmd->tStaff.pvParam          = ptCmd;
            ptCmd->lStatus                 = GRP_MSC_STALL;

            lStatus = grp_msc_ClearPipe( ptCmd, ptNmlReq->ptPipe->ucEndpointNumber );
            if( lStatus != GRP_MSC_OK ){
                ptNmlReq->lStatus = GRP_USBD_OTHER_FAIL;
            }
            break;

        case GRP_USBD_TR_CANCEL:
            /* It is a premise that the cancel callback occurs. */
            /* If not occurs, this case is meaningless.         */
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = GRP_MSC_CANCEL;
            }
            break;

        case GRP_USBD_TR_TIMEOUT:
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = GRP_MSC_TIMEOUT;
            }
            break;

        default:
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = ptNmlReq->lStatus;
            }
            break;
        }

        /* If STALL, retry in msc, but other error call callback. */
        if( ptNmlReq->lStatus != GRP_USBD_TR_STALL ){
            lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
            if( ptCmd->pfnCallback != GRP_USB_NULL ){
                ptCmd->pfnCallback( ptCmd );
            }
        }
    }

    _TRACE_MSC_BOT_(0x29, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotDataCallback                                                      */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptPipes                       Pipes that hopes to clear                       */
/* OUTPUT       : *ptPipes                      Clear by NULL                                   */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _grp_msc_BotDataCallback( grp_usbdi_request *ptNmlReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_u32                         ulNextSeq = GRP_MSC_SEQ_NORMAL;
grp_s32                         lStatus   = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x2A, 0x00, 0x00);

    ptCmd    = (grp_msc_cmd *)ptNmlReq->pvReferenceP;
    ptMscSet = ptCmd->hMscHdr;

    if( ptMscSet->ptCmd != ptCmd ){
        /* Allready canceled */
        ptCmd->lStatus = GRP_MSC_CANCEL;
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }
    else {
        /* Transaction status check */
        switch( ptNmlReq->lStatus ){
        case GRP_USBD_TR_NO_FAIL:
            ulNextSeq = GRP_MSC_SEQ_NORMAL;
            break;

        case GRP_USBD_TR_STALL:         /* NO BREAK */
            ptNmlReq->lStatus = GRP_MSC_STALL;  /* change a error code to notice upper layer.   */
        case GRP_USBD_TR_DATA_OVERRUN:
            ulNextSeq = GRP_MSC_SEQ_CLRPIPE;
            break;

        case GRP_USBD_TR_CANCEL:
            /* It is a premise that the cancel callback occurs. */
            /* If not occurs, this case is meaningless.         */
            ulNextSeq = GRP_MSC_SEQ_TERM;
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = GRP_MSC_CANCEL;
            }
            break;

        case GRP_USBD_TR_TIMEOUT:
            ulNextSeq = GRP_MSC_SEQ_TERM;
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = GRP_MSC_TIMEOUT;
            }
            break;

        default:
            ulNextSeq = GRP_MSC_SEQ_TERM;
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = ptNmlReq->lStatus;
            }
            break;
        }

        if( ulNextSeq == GRP_MSC_SEQ_CLRPIPE ){
            ptCmd->tStaff.pfnMscSfCallback = (grp_msc_request_callback)_grp_msc_BotDataCallback;
            ptCmd->tStaff.pvParam          = ptNmlReq;
            ptCmd->tStaff.lStatus          = ptNmlReq->lStatus;

            ptNmlReq->lStatus = GRP_USBD_TR_NO_FAIL; /* For next request */

            lStatus = grp_msc_ClearPipe( ptCmd, ptNmlReq->ptPipe->ucEndpointNumber );
            if( lStatus != GRP_MSC_OK ){
                ulNextSeq = GRP_MSC_SEQ_TERM;
                ptCmd->lStatus = GRP_MSC_ILLEGAL_FAIL;
            }
        }

        if( ulNextSeq == GRP_MSC_SEQ_NORMAL ){
            if( ptCmd->ulReqLength != 0 ){
                ptCmd->ulActualLength = ptNmlReq->ulActualLength;
            }
            ptMscSet->usPhase = GRP_MSC_PHASE_STATUS;

            ptNmlReq->ptPipe            = ptMscSet->tPipes.ptBulkIn;
            ptNmlReq->pucBuffer         = ptMscSet->pucSts;
            ptNmlReq->ulBufferLength    = GRP_MSC_CSW_LENGTH;
            ptNmlReq->iShortXferOK      = GRP_USB_FALSE;
            ptNmlReq->lStatus           = GRP_MSC_NOT_PROCESS;
            ptNmlReq->pfnNrCbFunc       = _grp_msc_BotCswCallback;
            ptNmlReq->pvReferenceP      = ptCmd;

            lStatus = grp_usbd_NormalRequest( ptNmlReq );
            if( lStatus != GRP_USBD_OK ){
                ulNextSeq = GRP_MSC_SEQ_TERM;
                ptCmd->lStatus = GRP_MSC_ILLEGAL_FAIL;
            }
        }

        if( ulNextSeq == GRP_MSC_SEQ_TERM ){
            lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
            if( ptCmd->pfnCallback != GRP_USB_NULL ){
                ptCmd->pfnCallback( ptCmd );
            }
        }
    }

    _TRACE_MSC_BOT_(0x2A, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotCswCallback                                                       */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptPipes                       Pipes that hopes to clear                       */
/* OUTPUT       : *ptPipes                      Clear by NULL                                   */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32   _grp_msc_BotCswCallback( grp_usbdi_request *ptNmlReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_msc_csw                     *ptCsw;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_BOT_(0x2B, 0x00, 0x00);

    ptCmd    = (grp_msc_cmd *)ptNmlReq->pvReferenceP;
    ptMscSet = ptCmd->hMscHdr;
    ptCsw    = (grp_msc_csw *)ptMscSet->pucSts;
    if( ptMscSet->ptCmd != ptCmd ){
        /* Allready canceled */
        ptCmd->lStatus = GRP_MSC_CANCEL;
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }
    else if( ptNmlReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        /* Transaction success */
        lStatus = _grp_msc_BotCheckCsw( ptCmd, ptCsw );
        if( lStatus == GRP_MSC_OK ){
            ptCmd->lStatus = GRP_MSC_NO_FAIL;
        }
        else if( lStatus == GRP_MSC_CHECK_CONDITION ){
            ptCmd->lStatus = GRP_MSC_CHECK_CONDITION;
        }
        else{
            /* Invalid CSW */
            ptCmd->lStatus = GRP_MSC_ILLEGAL_FAIL;
        }

        lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            if( (ptCmd->tStaff.lStatus != GRP_MSC_NO_FAIL )
             && (ptCmd->lStatus        != GRP_MSC_CHECK_CONDITION)){
                ptCmd->lStatus = ptCmd->tStaff.lStatus;
            }
            ptCmd->pfnCallback( ptCmd );
        }
    }
    else{
        /* Transaction error */
        switch( ptNmlReq->lStatus ){
        case GRP_USBD_TR_STALL:
            ptCmd->tStaff.pfnMscSfCallback = (grp_msc_request_callback)_grp_msc_BotRetryCswCb;
            ptCmd->tStaff.pvParam          = ptNmlReq;
            ptCmd->tStaff.ulRetry++;
            ptCmd->tStaff.lStatus          = GRP_MSC_STALL;

            lStatus = grp_msc_ClearPipe( ptCmd, ptNmlReq->ptPipe->ucEndpointNumber );
            if( lStatus != GRP_MSC_OK ){
                ptNmlReq->lStatus = GRP_USBD_OTHER_FAIL;
            }
            break;

        case GRP_USBD_TR_CANCEL:
            /* It is a premise that the cancel callback occurs. */
            /* If not occurs, this case is meaningless.         */
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = GRP_MSC_CANCEL;
            }
            break;

        case GRP_USBD_TR_TIMEOUT:
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = GRP_MSC_TIMEOUT;
            }
            break;

        default:
            if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                ptCmd->lStatus = ptNmlReq->lStatus;
            }
            break;
        }

        /* If STALL, retry in msc, but other error call callback. */
        if( ptNmlReq->lStatus != GRP_USBD_TR_STALL ){
            lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
            if( ptCmd->pfnCallback != GRP_USB_NULL ){
                ptCmd->pfnCallback( ptCmd );
            }
        }
    }

    _TRACE_MSC_BOT_(0x2B, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotSetCbw                                                            */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                                                                         */
/*                ptCbw                                                                         */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _grp_msc_BotSetCbw( grp_msc_cmd *ptCmd, grp_msc_cbw *ptCbw )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x2C, 0x00, 0x00);

    ptCbw->dCBWSignature          = grp_msc_swap32(GRP_MSC_CBW_SIGNATURE);
    _grp_msc_BotMakeTag ( &ptCbw->dCBWTag );
    ptCmd->tStaff.ulThisTag       = ptCbw->dCBWTag;
    ptCbw->dCBWDataTransferLength = grp_msc_swap32(ptCmd->ulReqLength);
    ptCbw->bmCBWFlags             = (grp_u8)((grp_u32)ptCmd->ucDir << 7);
    ptCbw->bmCBWLUN               = ptCmd->ucLun;
    ptCbw->bmCBWCBLength          = (grp_u8)ptCmd->ulCmdLength;
    grp_std_memcpy( ptCbw->bmCBWCB, ptCmd->aucCmdContent, GRP_MSC_CBWCB_LENGTH);

    _TRACE_MSC_BOT_(0x2C, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotCheckCsw                                                          */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                                                                         */
/*                ptCsw                                                                         */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _grp_msc_BotCheckCsw( grp_msc_cmd *ptCmd, grp_msc_csw *ptCsw )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x2D, 0x00, 0x00);

    if( ptCsw->dCSWSignature != grp_msc_swap32(GRP_MSC_CSW_SIGNATURE) ){
        lStatus = GRP_MSC_ERROR;
    }

    if( ptCsw->dCSWTag != ptCmd->tStaff.ulThisTag ){
        lStatus = GRP_MSC_ERROR;
    }

    if( lStatus == GRP_MSC_OK ){
        switch( ptCsw->bCSWStatus ){
        case GRP_CSW_COMMAND_PASSED:
            lStatus = GRP_MSC_OK;
            break;
        case GRP_CSW_COMMAND_FAILED:
            lStatus = GRP_MSC_CHECK_CONDITION;
            break;
        case GRP_CSW_PHASE_ERROR:
            lStatus = GRP_MSC_ERROR;
            break;
        }
    }

    _TRACE_MSC_BOT_(0x2D, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotMakeTag                                                           */
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
LOCAL grp_s32 _grp_msc_BotMakeTag( grp_u32 *pulNowTag )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x2E, 0x00, 0x00);

    *pulNowTag = l_ulMscTag++;

    _TRACE_MSC_BOT_(0x2E, 0x00, _F_END);

    return( lStatus );
}

#ifdef GRP_MSC_BOT_MS_RESET
/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotInPipeRecovery                                                    */
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
LOCAL grp_s32 _grp_msc_BotInPipeRecovery( grp_usbdi_device_request *ptDevReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_usbdi_st_device_request     *ptStdReq;
grp_s32                         lStatus   = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x2F, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_BOT_(0x2F, 0x01, _F_END);
        return( GRP_MSC_ERROR );
    }

    ptCmd = (grp_msc_cmd *)ptDevReq->pvReferenceP;
    ptDevReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_BOT_(0x2F, 0x02, _F_END);
        return( GRP_MSC_ERROR );
    }

    /* Already canceled */
    if( ptCmd == GRP_USB_NULL ){
        _TRACE_MSC_BOT_(0x2F, 0x03, _F_END);
        return( GRP_MSC_OK );
    }

    /* get information */
    ptMscSet = ptCmd->hMscHdr;
    ptStdReq = &ptMscSet->tStdReq;

    if( ptDevReq->lStatus != GRP_USBD_TR_NO_FAIL){
        /* set status */
        ptCmd->lStatus = ptDevReq->lStatus;
        /* notified upper layer */
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
        _TRACE_MSC_BOT_(0x2F, 0x04, _F_END);
        return GRP_MSC_OK;
    }

    /* set parameters */
    ptStdReq->usUsbDevId        = ptMscSet->usUsbDevId;
    ptStdReq->ptPipe            = ptMscSet->tPipes.ptBulkIn;
    ptStdReq->pfnStCbFunc       = _grp_msc_BotOutPipeRecovery;
    ptStdReq->pvReferenceP      = (void *)ptCmd;

    /* Activate In-pipe */
    lStatus = grp_usbd_PipeActive( ptStdReq );
    if (lStatus != GRP_USBD_OK ){
        /* set status */
        ptCmd->lStatus = ptDevReq->lStatus;
        /* notified upper layer */
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }

    _TRACE_MSC_BOT_(0x2F, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotOutPipeRecovery                                                   */
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
LOCAL grp_s32 _grp_msc_BotOutPipeRecovery( grp_usbdi_st_device_request *ptStdReq)
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_s32                         lStatus   = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x30, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_BOT_(0x30, 0x01, _F_END);
        return( GRP_MSC_ERROR );
    }

    ptCmd = (grp_msc_cmd *)ptStdReq->pvReferenceP;
    ptStdReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_BOT_(0x30, 0x02, _F_END);
        return( GRP_MSC_ERROR );
    }

    /* Already canceled */
    if( ptCmd == GRP_USB_NULL ){
        _TRACE_MSC_BOT_(0x30, 0x03, _F_END);
        return( GRP_MSC_OK );
    }

    /* get information */
    ptMscSet = ptCmd->hMscHdr;

    if( ptStdReq->lStatus != GRP_USBD_TR_NO_FAIL){
        /* set status */
        ptCmd->lStatus = ptStdReq->lStatus;
        /* notified upper layer */
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
        _TRACE_MSC_BOT_(0x30, 0x04, _F_END);
        return GRP_MSC_OK;
    }

    /* set parameters */
    ptStdReq->usUsbDevId        = ptMscSet->usUsbDevId;
    ptStdReq->ptPipe            = ptMscSet->tPipes.ptBulkOut;
    ptStdReq->pfnStCbFunc       = _grp_msc_BotPipeRecoveryComp;
    ptStdReq->pvReferenceP      = (void *)ptCmd;

    /* Activate Out-pipe */
    lStatus = grp_usbd_PipeActive( ptStdReq );
    if (lStatus != GRP_USBD_OK ){
        /* set status */
        ptCmd->lStatus = ptStdReq->lStatus;
        /* notified upper layer */
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }

    _TRACE_MSC_BOT_(0x30, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotPipeRecoveryComp                                                  */
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
LOCAL grp_s32 _grp_msc_BotPipeRecoveryComp( grp_usbdi_st_device_request *ptStdReq)
{
grp_msc_cmd                     *ptCmd;

    _TRACE_MSC_BOT_(0x31, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_BOT_(0x31, 0x01, _F_END);
        return( GRP_MSC_ERROR );
    }

    ptCmd = (grp_msc_cmd *)ptStdReq->pvReferenceP;
    ptStdReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_BOT_(0x31, 0x02, _F_END);
        return( GRP_MSC_ERROR );
    }

    /* set status */
    ptCmd->lStatus = ptStdReq->lStatus;

    /* notified upper layer */
    if( ptCmd->pfnCallback != GRP_USB_NULL ){
        ptCmd->pfnCallback( ptCmd );
    }

    _TRACE_MSC_BOT_(0x31, 0x00, _F_END);

    return GRP_MSC_OK;
}
#endif  /* GRP_MSC_BOT_MS_RESET */

/************************************************************************************************/
/* FUNCTION     : _grp_msc_BotRetryCswCb                                                        */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : pvData                        pointer                                         */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
LOCAL grp_s32 _grp_msc_BotRetryCswCb( void *pvData )
{
grp_usbdi_request               *ptNmlReq;
grp_msc_cmd                     *ptCmd;

    _TRACE_MSC_BOT_(0x32, 0x00, 0x00);

    ptNmlReq = (grp_usbdi_request *)pvData;
    ptCmd    = (grp_msc_cmd *)ptNmlReq->pvReferenceP;

    if (ptCmd->tStaff.ulRetry > 1) {
        _TRACE_MSC_BOT_(0x32, 0x01, 0x00);
        /* not recovery */
        ptCmd->lStatus    = GRP_MSC_NOT_PROCESS;
        ptNmlReq->lStatus = GRP_MSC_ILLEGAL_FAIL;
    }
    else {
        _TRACE_MSC_BOT_(0x32, 0x02, 0x00);
        /* retry CSWrequest */
        ptNmlReq->lStatus = GRP_USBD_TR_NO_FAIL;
        if( ptCmd->ulReqLength != 0 ){
            ptNmlReq->ulActualLength = ptCmd->ulActualLength;
        }
    }

    _grp_msc_BotDataCallback( ptNmlReq);

    _TRACE_MSC_BOT_(0x32, 0x00, _F_END);

    return GRP_MSC_OK;
}

#ifdef  _GRP_INTERNAL_TEST_
/************************************************************************************************/
/* FUNCTION     : grp_msc_BotSetTag                                                             */
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
grp_s32 grp_msc_BotSetTag( grp_u32 ulSetTag )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_BOT_(0x41, 0x00, 0x00);

    l_ulMscTag = ulSetTag;

    _TRACE_MSC_BOT_(0x41, 0x00, _F_END);

    return( lStatus );
}
#endif  /* _GRP_INTERNAL_TEST_ */
