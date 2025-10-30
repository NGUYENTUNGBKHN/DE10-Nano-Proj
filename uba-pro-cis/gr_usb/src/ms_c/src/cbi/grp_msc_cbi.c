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
/*      grp_msc_bot.h                                                           1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Control/Bulk/Interrupt Transport.                */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                              Created new policy initial version                              */
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
/*                                                                                              */
/************************************************************************************************/

/***** INCLUDE FILES ****************************************************************************/
#include    "grp_msc_cbi.h"


/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/
LOCAL grp_s32   _grp_msc_CbiOpen( grp_msc_cmd *ptCmd );
LOCAL grp_s32   _grp_msc_CbiClose( grp_msc_cmd *ptCmd );
LOCAL grp_s32   _grp_msc_CbiCmdReq( grp_msc_cmd *ptCmd );
LOCAL grp_s32   _grp_msc_CbiCancel( grp_msc_cmd *ptCmd );
LOCAL grp_s32   _grp_msc_CbiAbort( grp_msc_cmd *ptCmd );
LOCAL grp_s32   _grp_msc_CbiReset( grp_msc_cmd *ptCmd );
LOCAL grp_s32   _grp_msc_CbiRun( grp_msc_cmd *ptCmd );
LOCAL grp_s32   _grp_msc_CbiDel( grp_msc_cmd *ptCmd );

LOCAL grp_s32   _grp_msc_CbiCmdCallback( grp_usbdi_device_request *ptDevReq );
LOCAL grp_s32   _grp_msc_CbiDataCallback( grp_usbdi_request *ptNmlReq );
LOCAL grp_s32   _grp_msc_CbiStsCallback( grp_usbdi_request *ptNmlReq );
LOCAL grp_s32   _grp_msc_CbiCheckSts( grp_msc_cmd *ptCmd, grp_msc_idb *ptIdb );
#ifdef GRP_MSC_CBI_MS_RESET
LOCAL grp_s32 _grp_msc_CbiInPipeRecovery( grp_usbdi_st_device_request *ptStdReq);
LOCAL grp_s32 _grp_msc_CbiOutPipeRecovery( grp_usbdi_st_device_request *ptStdReq);
LOCAL grp_s32 _grp_msc_CbiPipeRecoveryComp( grp_usbdi_st_device_request *ptStdReq);
#endif  /* GRP_MSC_CBI_MS_RESET */
LOCAL grp_s32 _grp_msc_CbiRetryStsCb( void *pvData );
LOCAL grp_s32 _grp_msc_CbStallError( void *pvData );


/************************************************************************************************/
/* FUNCTION     : grp_msc_CbiInit                                                               */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : SeqFunc                       Cbi sequence function pointer.                  */
/*                pvRsv                         Reserved argument.                              */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
grp_s32 grp_msc_CbiInit( grp_msc_seq *SeqFunc, void *pvRsv )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_CBI_(0x01, 0x00, 0x00);

    pvRsv = GRP_USB_NULL;

    if( SeqFunc->ulStatus != GRP_MSC_USE ){
        SeqFunc->pfnOpen   = _grp_msc_CbiOpen;
        SeqFunc->pfnClose  = _grp_msc_CbiClose;
        SeqFunc->pfnCmdReq = _grp_msc_CbiCmdReq;
        SeqFunc->pfnCancel = _grp_msc_CbiCancel;
        SeqFunc->pfnAbort  = _grp_msc_CbiAbort;
        SeqFunc->pfnReset  = _grp_msc_CbiReset;

        SeqFunc->pfnRun    = _grp_msc_CbiRun;
        SeqFunc->pfnDel    = _grp_msc_CbiDel;

        SeqFunc->ulStatus  = GRP_MSC_USE;
    }

    _TRACE_MSC_CBI_(0x01, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_CbiOpen                                                               */
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
LOCAL grp_s32 _grp_msc_CbiOpen( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_pipe_operate          tOper;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x21, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    if( ptMscSet->ulStatus != GRP_MSC_CONNECT ){
        return( lStatus );
    }

    tOper.usUsbDevId                        = ptMscSet->usUsbDevId;
    tOper.ucInterface                       = ptMscSet->ucInterfaceNum;
    tOper.ptPipe                            = ptMscSet->atPipe;
    tOper.ptPipe                            = ptMscSet->atPipe;
    tOper.ucPipeNumber                      = GRP_MSC_CBI_PIPES;
    tOper.atSelPipe[0].ucTransferMode       = GRP_USBD_BULK;
    tOper.atSelPipe[0].ucTransferDirection  = GRP_USBD_TX_OUT;
    tOper.atSelPipe[1].ucTransferMode       = GRP_USBD_BULK;
    tOper.atSelPipe[1].ucTransferDirection  = GRP_USBD_TX_IN;
    tOper.atSelPipe[2].ucTransferMode       = GRP_USBD_INTERRUPT;
    tOper.atSelPipe[2].ucTransferDirection  = GRP_USBD_TX_IN;

    /* Interface open */
    lStatus = grp_usbd_InterfaceOpen( &tOper );
    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_ClearAllPipes( &ptMscSet->tPipes );
    }

    if( lStatus == GRP_MSC_OK ){
        ptMscSet->tPipes.ptBulkOut = &ptMscSet->atPipe[0];
        ptMscSet->tPipes.ptBulkIn  = &ptMscSet->atPipe[1];
        ptMscSet->tPipes.ptIntrIn  = &ptMscSet->atPipe[2];

        /* OK */
        ptMscSet->ulStatus = GRP_MSC_OPEN;
        lStatus            = GRP_MSC_OK;
    }
    else{
        /* Colse pipes */
        ptMscSet->ulStatus = GRP_MSC_DISCONNECT;
        _grp_msc_CbiClose( ptCmd);
    }

    _TRACE_MSC_CBI_(0x21, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_CbiClose                                                              */
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
LOCAL grp_s32 _grp_msc_CbiClose( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_pipe_operate          tOper;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x22, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;
    if( (ptMscSet->ulStatus != GRP_MSC_OPEN)
     && (ptMscSet->ulStatus != GRP_MSC_DISCONNECT) ){
        return( lStatus );
    }

    tOper.usUsbDevId                    = ptMscSet->usUsbDevId;
    tOper.ucInterface                   = ptMscSet->ucInterfaceNum;
    tOper.ptPipe                        = ptMscSet->atPipe;
    tOper.ptPipe                        = ptMscSet->atPipe;
    tOper.ucPipeNumber                  = GRP_MSC_CBI_PIPES;
    tOper.atSelPipe[0].ucTransferMode   = GRP_USBD_UNKNOWN_MODE;

    lStatus = grp_usbd_InterfaceClose( &tOper );

    if( lStatus == GRP_MSC_OK ){
        lStatus = grp_msc_ClearAllPipes( &ptMscSet->tPipes );
        if( ptMscSet->ulStatus == GRP_MSC_OPEN ){
            ptMscSet->ulStatus = GRP_MSC_CONNECT;
        }
        else if( ptMscSet->ulStatus == GRP_MSC_DISCONNECT ){
            ptMscSet->ulStatus = GRP_MSC_BLANK;
        }
        else{
            lStatus = GRP_MSC_ERROR;
        }
    }

    _TRACE_MSC_CBI_(0x22, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_CbiCmdReq                                                             */
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
LOCAL grp_s32 _grp_msc_CbiCmdReq( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x23, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    lStatus = grp_msc_LinkCmd( ptMscSet, ptCmd );

    _TRACE_MSC_CBI_(0x23, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiCancel                                                            */
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
LOCAL grp_s32 _grp_msc_CbiCancel( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x24, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    /* Cancel to the Control transfer */
    grp_usbd_DeviceRequestCancel( &ptMscSet->tDevReq );
    /* Cancel to the Bulk of Interrupt transfer */
    grp_usbd_NormalRequestCancel( &ptMscSet->tNmlReq );

    lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );

    _TRACE_MSC_CBI_(0x24, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiAbort                                                             */
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
LOCAL grp_s32 _grp_msc_CbiAbort( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x25, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    lStatus = grp_usbd_NormalRequestCancel( &ptMscSet->tNmlReq );

    _TRACE_MSC_CBI_(0x25, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiReset                                                             */
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
LOCAL grp_s32 _grp_msc_CbiReset( grp_msc_cmd *ptCmd )
{
#ifdef GRP_MSC_CBI_MS_RESET
grp_msc_set                     *ptMscSet;
grp_usbdi_st_device_request     *ptStdReq;
grp_s32                         lStatus   = GRP_MSC_OK;

    _TRACE_MSC_CBI_(0x26, 0x00, 0x00);

    /* get information */
    ptMscSet = ptCmd->hMscHdr;
    ptStdReq = &ptMscSet->tStdReq;

    /* set parameters */
    ptStdReq->usUsbDevId        = ptMscSet->usUsbDevId;
    ptStdReq->ptPipe            = ptMscSet->tPipes.ptIntrIn;
    ptStdReq->pfnStCbFunc       = _grp_msc_CbiInPipeRecovery;
    ptStdReq->pvReferenceP      = (void *)ptCmd;

    /* Activate In-pipe */
    lStatus = grp_usbd_PipeActive( ptStdReq );

    _TRACE_MSC_CBI_(0x26, 0x00, _F_END);

    return( lStatus );

#else   /* GRP_MSC_CBI_MS_RESET */

    ptCmd = GRP_USB_NULL;   /* Warnign measures */

    return GRP_MSC_ERROR;

#endif  /* GRP_MSC_CBI_MS_RESET */
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiRun                                                               */
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
LOCAL grp_s32 _grp_msc_CbiRun( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptMscSet;
grp_usbdi_device_request        *ptReq;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_CBI_(0x27, 0x00, 0x00);

    ptMscSet = ptCmd->hMscHdr;

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    ptReq = &ptMscSet->tDevReq;
    if( ptReq->pvReferenceP == GRP_USB_NULL ){
        ptReq->pvReferenceP = (void *)ptCmd;
    }

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* if BUSY, please retry upper layer */
    if( (grp_msc_cmd *)ptReq->pvReferenceP != ptCmd ){
        return( GRP_MSC_BUSY );
    }

    ptCmd->ulActualLength = 0;
    ptCmd->lStatus        = GRP_MSC_NOT_PROCESS;

    ptMscSet->usPhase = GRP_MSC_PHASE_COMMAND;

    /* create command data */
    grp_std_memcpy( ptMscSet->pucCmd, ptCmd->aucCmdContent, GRP_MSC_CMD_LENGTH);

    ptMscSet->tDevReq.bmRequestType     = GRP_USBD_TYPE_CLASS
                                        | GRP_USBD_TYPE_INTERFACE;      /* Request type         */
    ptMscSet->tDevReq.bRequest          = GRUSB_CBI_REQ_COMMAND;        /* Request              */
    ptMscSet->tDevReq.wValue            = 0;                            /* Value                */
    ptMscSet->tDevReq.wIndex            = ptMscSet->ucInterfaceNum;     /* Interface number     */
    ptMscSet->tDevReq.wLength           = (grp_u8)ptCmd->ulCmdLength;   /* Command length       */
    ptMscSet->tDevReq.pucBuffer         = ptMscSet->pucCmd;             /* Command content      */
    ptMscSet->tDevReq.pfnDvCbFunc       = _grp_msc_CbiCmdCallback;      /* Callback function    */
    ptMscSet->tDevReq.lStatus           = GRP_MSC_NOT_PROCESS;          /* Transfer result      */
    ptMscSet->tDevReq.pvReferenceP      = (void *)ptCmd;                /* Pointer for user     */
    ptMscSet->tDevReq.usUsbDevId        = ptMscSet->usUsbDevId;         /* Device ID            */

    lStatus = grp_usbd_DeviceRequest( &ptMscSet->tDevReq );

    _TRACE_MSC_CBI_(0x27, 0x00, 0x00);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiDel                                                               */
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
LOCAL grp_s32 _grp_msc_CbiDel( grp_msc_cmd *ptCmd )
{
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x28, 0x00, 0x00);

    ptCmd = GRP_USB_NULL;   /* Warnign measures */

    _TRACE_MSC_CBI_(0x28, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiCmdCallback                                                       */
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
LOCAL grp_s32 _grp_msc_CbiCmdCallback( grp_usbdi_device_request *ptDevReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_usbdi_request               *ptNmlReq;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x29, 0x00, 0x00);

    ptCmd     = (grp_msc_cmd *)ptDevReq->pvReferenceP;
    ptMscSet  = ptCmd->hMscHdr;

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }
    /* set NULL pointer to the next request */
    ptDevReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        return( GRP_MSC_ERROR );
    }

    /* Operate data phase */
    if( ptMscSet->ptCmd != ptCmd ){
        /* Allready canceled */
        ptCmd->lStatus = GRP_MSC_CANCEL;
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }
    else if( ptDevReq->lStatus == GRP_USBD_TR_NO_FAIL ){
        ptNmlReq = &ptMscSet->tNmlReq;
        ptNmlReq->pvReferenceP = (void *)ptCmd;

        /* Transaction success */
        if( ptCmd->ulReqLength == 0 ){
            ptNmlReq->ulActualLength = 0;
            ptNmlReq->lStatus        = GRP_USBD_TR_NO_FAIL;

            lStatus = _grp_msc_CbiDataCallback( ptNmlReq );
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
            ptNmlReq->pfnNrCbFunc     = _grp_msc_CbiDataCallback;

            lStatus = grp_usbd_NormalRequest( ptNmlReq );
            if( lStatus != GRP_USBD_OK ){
                lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
                ptCmd->lStatus = GRP_MSC_ILLEGAL_FAIL;
                if( ptCmd->pfnCallback != GRP_USB_NULL ){
                    ptCmd->pfnCallback( ptCmd );
                }
            }
        }
    }
    else{
        /* Transaction error */
        switch( ptDevReq->lStatus ){
        case GRP_USBD_TR_STALL:
            ptNmlReq = &ptMscSet->tNmlReq;

            ptCmd->tStaff.pfnMscSfCallback = _grp_msc_CbStallError;
            ptCmd->tStaff.pvParam          = ptNmlReq;
            ptCmd->tStaff.lStatus          = ptNmlReq->lStatus;

            ptNmlReq->lStatus = GRP_USBD_TR_NO_FAIL; /* For next request */
            ptNmlReq->pvReferenceP = (void *)ptCmd;

            lStatus = grp_msc_ClearPipe( ptCmd, GRP_USBD_DEFAULT_ENDPOINT );
            break;

        case GRP_USBD_TR_CANCEL:
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
                ptCmd->lStatus = ptDevReq->lStatus;
            }
            break;
        }

        if( ptDevReq->lStatus != GRP_USBD_TR_STALL ){
            lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
            if( ptCmd->pfnCallback != GRP_USB_NULL ){
                ptCmd->pfnCallback( ptCmd );
            }
        }
    }

    _TRACE_MSC_CBI_(0x29, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiDataCallback                                                      */
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
LOCAL grp_s32 _grp_msc_CbiDataCallback( grp_usbdi_request *ptNmlReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_u32                         ulNextSeq = GRP_MSC_SEQ_NORMAL;
grp_s32                         lStatus   = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x2A, 0x00, 0x00);

    ptCmd    = (grp_msc_cmd *)ptNmlReq->pvReferenceP;
    ptMscSet = ptCmd->hMscHdr;
    if( ptMscSet->ptCmd != ptCmd ){
        /* Allready canceled */
        ptCmd->lStatus = GRP_MSC_CANCEL;
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }
    else{
        /* Transaction status check */
        switch( ptNmlReq->lStatus ){
        case GRP_USBD_TR_NO_FAIL:
            ulNextSeq = GRP_MSC_SEQ_NORMAL;
            break;

        case GRP_USBD_TR_STALL:         /* NO BREAK */
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
            if( ptMscSet->ptMscReg->ucProtocol == GRP_MSC_CBI_CODE ){
                ptCmd->tStaff.pfnMscSfCallback = (grp_msc_request_callback)_grp_msc_CbiDataCallback;
            }
            else {
                ptCmd->tStaff.pfnMscSfCallback = _grp_msc_CbStallError;
            }
            ptCmd->tStaff.pvParam     = ptNmlReq;
            ptCmd->tStaff.lStatus     = ptNmlReq->lStatus;

            ptNmlReq->lStatus = GRP_USBD_TR_NO_FAIL; /* For next request */

            lStatus = grp_msc_ClearPipe( ptCmd, ptNmlReq->ptPipe->ucEndpointNumber );
        }

        if( ulNextSeq == GRP_MSC_SEQ_NORMAL ){
            if( ptCmd->ulReqLength != 0 ){
                /* Transaction success */
                ptCmd->ulActualLength = ptNmlReq->ulActualLength;
            }
            if( ptMscSet->ptMscReg->ucProtocol == GRP_MSC_CBI_CODE ){
                /* for CBI protocol */
                ptMscSet->usPhase              = GRP_MSC_PHASE_STATUS;
                ptMscSet->tNmlReq.iShortXferOK = GRP_USB_FALSE;

                ptNmlReq->ptPipe          = ptMscSet->tPipes.ptIntrIn;
                ptNmlReq->pucBuffer       = ptMscSet->pucSts;
                ptNmlReq->ulBufferLength  = GRP_MSC_IDB_LENGTH;
                ptNmlReq->lStatus         = GRP_MSC_NOT_PROCESS;
                ptNmlReq->pfnNrCbFunc     = _grp_msc_CbiStsCallback;
                ptNmlReq->pvReferenceP    = (void *)ptCmd;

                lStatus = grp_usbd_NormalRequest( ptNmlReq );
                if( lStatus != GRP_USBD_OK ){
                    ulNextSeq = GRP_MSC_SEQ_TERM;
                    if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                        ptCmd->lStatus = GRP_MSC_ILLEGAL_FAIL;
                    }
                }
            }
            else{ /* GRP_MSC_CBI_CODE */
                /* for CB protocol */
                ulNextSeq         = GRP_MSC_SEQ_TERM;
                ptMscSet->usPhase = GRP_MSC_PHASE_NON;
                if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
                    ptCmd->lStatus = ptNmlReq->lStatus;     /* GRP_USBD_TR_NO_FAIL  */
                }
            }
        }

        if( ulNextSeq == GRP_MSC_SEQ_TERM ){
            lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
            if( ptCmd->pfnCallback != GRP_USB_NULL ){
                ptCmd->pfnCallback( ptCmd );
            }
        }
    }

    _TRACE_MSC_CBI_(0x2A, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiStsCallback                                                       */
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
LOCAL grp_s32 _grp_msc_CbiStsCallback( grp_usbdi_request *ptNmlReq )
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_s32                         lCmdSts;
grp_s32                         lStatus = GRP_MSC_ERROR;

    _TRACE_MSC_CBI_(0x2B, 0x00, 0x00);

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
        /* Transaction success, check Interrupt Data Block for status */
        lStatus = _grp_msc_CbiCheckSts( ptCmd, (grp_msc_idb *)ptMscSet->pucSts );
        if( lStatus == GRP_MSC_OK ){
            lCmdSts = GRP_MSC_NO_FAIL;
        }
        else if( lStatus == GRP_MSC_CHECK_CONDITION ){
            lCmdSts = GRP_MSC_CHECK_CONDITION;
        }
        else{
            /* Invalid CSW */
            lCmdSts = GRP_MSC_ILLEGAL_FAIL;
        }
        if( ptCmd->lStatus == GRP_MSC_NOT_PROCESS ){
            ptCmd->lStatus = lCmdSts;
        }

        lStatus = grp_msc_UnlinkCmd( ptMscSet, ptCmd );
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            if( ptCmd->tStaff.lStatus != GRP_MSC_NO_FAIL ){
                ptCmd->lStatus = ptCmd->tStaff.lStatus;
            }
            ptCmd->pfnCallback( ptCmd );
        }
    }
    else{
        /* Transaction error */
        switch( ptNmlReq->lStatus ){
        case GRP_USBD_TR_STALL:
            ptCmd->tStaff.pfnMscSfCallback = (grp_msc_request_callback)_grp_msc_CbiRetryStsCb;
            ptCmd->tStaff.pvParam          = ptNmlReq;
            ptCmd->tStaff.ulRetry++;

            lStatus = grp_msc_ClearPipe( ptCmd, ptNmlReq->ptPipe->ucEndpointNumber );
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

    _TRACE_MSC_CBI_(0x2B, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiCheckSts                                                          */
/*                                                                                              */
/* DESCRIPTION  : Check Interrupt Data Block for status                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Command structure                               */
/*                ptIdb                         Interrupt Data Block                            */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Failed                                          */
/*                                                                                              */
/* EFFECT      : none                                                                           */
/************************************************************************************************/
LOCAL grp_s32 _grp_msc_CbiCheckSts( grp_msc_cmd *ptCmd, grp_msc_idb *ptIdb )
{
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_CBI_(0x2C, 0x00, 0x00);

    ptCmd = GRP_USB_NULL;   /* Warnign measures */

    if( ptIdb->bType != GRP_MSC_OK ){
        lStatus = GRP_MSC_ERROR;
    }

    if( ptIdb->bValue != GRP_MSC_OK ){
        lStatus = GRP_MSC_ERROR;
    }

    _TRACE_MSC_CBI_(0x2C, 0x00, _F_END);

    return( lStatus );
}

#ifdef GRP_MSC_CBI_MS_RESET
/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiInPipeRecovery                                                    */
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
LOCAL grp_s32 _grp_msc_CbiInPipeRecovery( grp_usbdi_st_device_request *ptStdReq)
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_s32                         lStatus   = GRP_MSC_OK;

    _TRACE_MSC_CBI_(0x2D, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_CBI_(0x2D, 0x01, _F_END);
        return( GRP_MSC_ERROR );
    }

    ptCmd = (grp_msc_cmd *)ptStdReq->pvReferenceP;
    ptStdReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_CBI_(0x2D, 0x02, _F_END);
        return( GRP_MSC_ERROR );
    }

    /* Already canceled */
    if( ptCmd == GRP_USB_NULL ){
        _TRACE_MSC_CBI_(0x2D, 0x03, _F_END);
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
        _TRACE_MSC_CBI_(0x2D, 0x04, _F_END);
        return GRP_MSC_OK;
    }

    /* set parameters */
    ptStdReq->usUsbDevId        = ptMscSet->usUsbDevId;
    ptStdReq->ptPipe            = ptMscSet->tPipes.ptBulkIn;
    ptStdReq->pfnStCbFunc       = _grp_msc_CbiOutPipeRecovery;
    ptStdReq->pvReferenceP      = (void *)ptCmd;

    /* Activate In-pipe */
    lStatus = grp_usbd_PipeActive( ptStdReq );
    if (lStatus != GRP_USBD_OK ){
        /* set status */
        ptCmd->lStatus = ptStdReq->lStatus;
        /* notified upper layer */
        if( ptCmd->pfnCallback != GRP_USB_NULL ){
            ptCmd->pfnCallback( ptCmd );
        }
    }

    _TRACE_MSC_CBI_(0x2D, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiOutPipeRecovery                                                   */
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
LOCAL grp_s32 _grp_msc_CbiOutPipeRecovery( grp_usbdi_st_device_request *ptStdReq)
{
grp_msc_set                     *ptMscSet;
grp_msc_cmd                     *ptCmd;
grp_s32                         lStatus   = GRP_MSC_OK;

    _TRACE_MSC_CBI_(0x2E, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_CBI_(0x2E, 0x01, _F_END);
        return( GRP_MSC_ERROR );
    }

    ptCmd = (grp_msc_cmd *)ptStdReq->pvReferenceP;
    ptStdReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_CBI_(0x2E, 0x02, _F_END);
        return( GRP_MSC_ERROR );
    }

    /* Already canceled */
    if( ptCmd == GRP_USB_NULL ){
        _TRACE_MSC_CBI_(0x2E, 0x03, _F_END);
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
        _TRACE_MSC_CBI_(0x2E, 0x04, _F_END);
        return GRP_MSC_OK;
    }

    /* set parameters */
    ptStdReq->usUsbDevId        = ptMscSet->usUsbDevId;
    ptStdReq->ptPipe            = ptMscSet->tPipes.ptBulkOut;
    ptStdReq->pfnStCbFunc       = _grp_msc_CbiPipeRecoveryComp;
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

    _TRACE_MSC_CBI_(0x2E, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiPipeRecoveryComp                                                  */
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
LOCAL grp_s32 _grp_msc_CbiPipeRecoveryComp( grp_usbdi_st_device_request *ptStdReq)
{
grp_msc_cmd                     *ptCmd;

    _TRACE_MSC_CBI_(0x2F, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if( grp_msc_Lock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_CBI_(0x2F, 0x01, _F_END);
        return( GRP_MSC_ERROR );
    }

    ptCmd = (grp_msc_cmd *)ptStdReq->pvReferenceP;
    ptStdReq->pvReferenceP = GRP_USB_NULL;

    /* unlock */                                                                        /* }    */
    if( grp_msc_Unlock_Sem() != GRP_MSC_OK ){
        _TRACE_MSC_CBI_(0x2F, 0x02, _F_END);
        return( GRP_MSC_ERROR );
    }

    /* set status */
    ptCmd->lStatus = ptStdReq->lStatus;

    /* notified upper layer */
    if( ptCmd->pfnCallback != GRP_USB_NULL ){
        ptCmd->pfnCallback( ptCmd );
    }

    _TRACE_MSC_CBI_(0x2F, 0x00, _F_END);

    return GRP_MSC_OK;
}
#endif  /* GRP_MSC_CBI_MS_RESET */

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbiRetryStsCb                                                        */
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
LOCAL grp_s32 _grp_msc_CbiRetryStsCb( void *pvData )
{
grp_usbdi_request               *ptNmlReq;
grp_msc_cmd                     *ptCmd;

    _TRACE_MSC_CBI_(0x30, 0x00, 0x00);

    ptNmlReq = (grp_usbdi_request *)pvData;
    ptCmd    = (grp_msc_cmd *)ptNmlReq->pvReferenceP;

    if (ptCmd->tStaff.ulRetry > 1) {
        _TRACE_MSC_CBI_(0x30, 0x01, 0x00);
        /* not recovery */
        ptCmd->lStatus    = GRP_MSC_NOT_PROCESS;
        ptNmlReq->lStatus = GRP_MSC_ILLEGAL_FAIL;
    }
    else {
        _TRACE_MSC_CBI_(0x30, 0x02, 0x00);
        /* retry CSWrequest */
        ptNmlReq->lStatus = GRP_USBD_TR_NO_FAIL;
    }

    _grp_msc_CbiDataCallback( ptNmlReq);

    _TRACE_MSC_CBI_(0x30, 0x00, _F_END);

    return GRP_MSC_OK;
}

/************************************************************************************************/
/* FUNCTION     : _grp_msc_CbStallError                                                         */
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
LOCAL grp_s32 _grp_msc_CbStallError( void *pvData )
{
grp_usbdi_request               *ptNmlReq;
grp_msc_cmd                     *ptCmd;
grp_msc_set                     *ptMscSet;

    _TRACE_MSC_CBI_(0x31, 0x00, 0x00);

    ptNmlReq = (grp_usbdi_request *)pvData;
    ptCmd    = (grp_msc_cmd *)ptNmlReq->pvReferenceP;
    ptMscSet = ptCmd->hMscHdr;

    /* for CB protocol */
    ptMscSet->usPhase = GRP_MSC_PHASE_NON;
    ptCmd->lStatus    = GRP_MSC_CHECK_CONDITION;

    grp_msc_UnlinkCmd( ptMscSet, ptCmd );
    if( ptCmd->pfnCallback != GRP_USB_NULL ){
        ptCmd->pfnCallback( ptCmd );
    }

    _TRACE_MSC_CBI_(0x31, 0x00, _F_END);

    return GRP_MSC_OK;
}

