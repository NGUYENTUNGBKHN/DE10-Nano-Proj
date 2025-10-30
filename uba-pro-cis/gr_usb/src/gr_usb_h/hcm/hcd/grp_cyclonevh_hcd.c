/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2019 Grape Systems, Inc.                             */
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
/*      grp_cyclonevh_hcd.c                                                     1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      CYCLONEV Host Controller Driver function                                                */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kaneko       2019/04/26  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                              (based on GR-USB/HOST# for STM32Fx(OTG_HS) V1.01)               */
/*                                                                                              */
/************************************************************************************************/

/*** INCLUDE FILES ******************************************************************************/
#include "grp_cyclonevh.h"

#include "grp_usbc_reinit.h" /* reinit */

/*** INTERNAL DATA DEFINES **********************************************************************/
#define GRP_CYCLONEVH_HOST_DELAY            (500)
#define GRP_CYCLONEVH_LS_SETUP              (200)
#define GRP_CYCLONEVH_DIS_RETRY_MAX         (100)

/*--- Definition of exclusive access control ---*/
grp_vos_t_semaphore                     *g_ptListLock;

/*** INTERNAL FUNCTION PROTOTYPES ***************************************************************/
/* Controller control functions */
LOCAL grp_s32 _grp_cyclonevh_Reset(grp_hcdi_hc_hdr *ptHcd);
LOCAL grp_s32 _grp_cyclonevh_GlobalSuspend(grp_hcdi_hc_hdr *ptHcd);
LOCAL grp_s32 _grp_cyclonevh_GlobalResume(grp_hcdi_hc_hdr *ptHcd);
LOCAL grp_s32 _grp_cyclonevh_SetSof(grp_hcdi_frame_control *ptFrmCtrl);
LOCAL grp_s32 _grp_cyclonevh_GetSof(grp_hcdi_frame_control *ptFrmCtrl);
LOCAL grp_s32 _grp_cyclonevh_SetFrame(grp_hcdi_frame_control *ptFrmCtrl);
LOCAL grp_s32 _grp_cyclonevh_GetFrame(grp_hcdi_frame_control *ptFrmCtrl);
/* Endpoint control functions */
LOCAL grp_s32 _grp_cyclonevh_EpOpen(grp_hcdi_endpoint *ptHcdiEp);
LOCAL grp_s32 _grp_cyclonevh_EpClose(grp_hcdi_endpoint *ptHcdiEp);
LOCAL grp_s32 _grp_cyclonevh_EpHalt(grp_hcdi_endpoint *ptHcdiEp);
LOCAL grp_s32 _grp_cyclonevh_EpActive(grp_hcdi_endpoint *ptHcdiEp);
LOCAL grp_s32 _grp_cyclonevh_EpFlash(grp_hcdi_endpoint*ptHcdiEp);
/* Communication functions */
LOCAL grp_s32 _grp_cyclonevh_TrRun(grp_hcdi_tr_request *ptTrReq);
LOCAL grp_s32 _grp_cyclonevh_TrDel(grp_hcdi_tr_request *ptTrReq);
LOCAL grp_s32 _grp_cyclonevh_ItrRun(grp_hcdi_tr_request *ptTrReq);
LOCAL grp_s32 _grp_cyclonevh_ItrDel(grp_hcdi_tr_request *ptTrReq);
/* Port Control Function */
LOCAL grp_s32 _grp_cyclonevh_PortControl( grp_hcdi_port_control *ptPtCtrl);

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_Init                                                              */
/*                                                                                              */
/* DESCRIPTION: Initialization to Host Controller Driver.                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcdInfo                       System initialize structure                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_Init(grp_hcdi_system_init *ptHcdInfo)
{
grp_hcdi_request                *ptFunc     = ptHcdInfo->ptHcIoFunc;

    _TRACE_HCDI_(0x00, 0x00, 0x00);

    /* Register HCD I/O functions to GR-USBD */
    /* Controller control functions */
    ptFunc->pfnHcReset         = _grp_cyclonevh_Reset;              /* none                                     */
    ptFunc->pfnHcSetSof        = _grp_cyclonevh_SetSof;             /* grp_usbd_FrameNumberControl()            */
    ptFunc->pfnHcGetSof        = _grp_cyclonevh_GetSof;             /* grp_usbd_FrameNumberControl()            */
    ptFunc->pfnHcSetFrame      = _grp_cyclonevh_SetFrame;           /* grp_usbd_FrameWidthControl()             */
    ptFunc->pfnHcGetFrame      = _grp_cyclonevh_GetFrame;           /* grp_usbd_FrameWidthControl()             */
    ptFunc->pfnHcGlobalSuspend = _grp_cyclonevh_GlobalSuspend;      /* grp_usbd_BusPowerControl()               */
    ptFunc->pfnHcGlobalResume  = _grp_cyclonevh_GlobalResume;       /* grp_usbd_BusPowerControl()               */
    /* Endpoint control functions */
    ptFunc->pfnHcEpOpen        = _grp_cyclonevh_EpOpen;             /* grp_usbd_PipeOpen()                      */
    ptFunc->pfnHcEpClose       = _grp_cyclonevh_EpClose;            /* grp_usbd_PipeClose()                     */
    ptFunc->pfnHcEpHalt        = _grp_cyclonevh_EpHalt;             /* grp_usbd_PipeHalt()                      */
    ptFunc->pfnHcEpActive      = _grp_cyclonevh_EpActive;           /* grp_usbd_PipeActive()                    */
    ptFunc->pfnHcEpFlash       = _grp_cyclonevh_EpFlash;            /* grp_usbd_PipeAbort/Reset()               */
    /* Communication functions */
    ptFunc->pfnHcTrRun         = _grp_cyclonevh_TrRun;              /* grp_usbd_Device/NormalRequest()          */
    ptFunc->pfnHcTrDel         = _grp_cyclonevh_TrDel;              /* grp_usbd_Device/NormalRequestCancel()    */
    ptFunc->pfnHcItrRun        = _grp_cyclonevh_ItrRun;             /* [ISO] grp_usbd_NormalRequest()           */
    ptFunc->pfnHcItrDel        = _grp_cyclonevh_ItrDel;             /* [ISO] grp_usbd_NormalRequestCancel()     */
    /* Port Control Function */
    ptFunc->pfnHcPortControl   = _grp_cyclonevh_PortControl;        /* grp_usbd_BusPowerControl()               */

    ptHcdInfo->usHcIndexNum = GRP_HCDI_CYCLONEVH_IDX_00;
    ptHcdInfo->iStatus      = GRP_HCDI_ERROR;

    /*--- Create semaphore ---*/
    if(_not_initialized){
        if (GRP_VOS_POS_RESULT  != grp_vos_CreateSemaphore(&g_ptListLock,
                                                           (grp_u8*)"sCycvH",
                                                           1)) {
            _TRACE_HCDI_(0x00, 0x01, END_FUNC);
            /* set error status */
            return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
        }
    } /* reinit */

    /*--- Initialize of modules ---*/
    /* managment module */
    if (GRP_HCDI_OK != grp_cyclonevh_MngInit()) {
        _TRACE_HCDI_(0x00, 0x03, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    if(_not_initialized){
            /* Controller module */
            if (GRP_HCDI_OK != grp_cyclonevh_CtrInit()) {
                _TRACE_HCDI_(0x00, 0x04, END_FUNC);
                /* error */
                return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
            }

            /* Root Hub module */
            if (GRP_HCDI_OK != grp_cyclonevh_RhubInit()) {
                _TRACE_HCDI_(0x00, 0x05, END_FUNC);
                /* error */
                return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
            }
    } /* reinit */

    ptHcdInfo->ulHostDelay = GRP_CYCLONEVH_HOST_DELAY;
    ptHcdInfo->ulLsSetup   = GRP_CYCLONEVH_LS_SETUP;
    ptHcdInfo->iStatus     = GRP_HCDI_OK;

    _TRACE_HCDI_(0x00, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_DisableAllChannel                                                 */
/*                                                                                              */
/* DESCRIPTION: Disable all channel's.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_DisableAllChannel(void)
{
grp_s32                         lStatus=GRP_HCDI_OK;
grp_u32                         ulData;
grp_si                          i;
grp_si                          j;

    _TRACE_HCDI_(0x14, 0x00, 0x00);
    
    /* Halt all channels to put them into a known state. */
    for (i=0; i<GRP_CYCLONEVH_MAX_PIPE_NUM; i++) {
        ulData  = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCCHAR(i));
        ulData &= ~(CYCLONEVH_B01_CHENA | CYCLONEVH_B01_CHDIS | CYCLONEVH_B01_EPDIR);
        ulData |= (CYCLONEVH_B01_CHENA | CYCLONEVH_B01_CHDIS);
        CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(i), ulData);
        for (j=0; j<GRP_CYCLONEVH_DIS_RETRY_MAX; j++) {
            ulData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCINT(i));
            CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINT(i), ulData);
            if (ulData & CYCLONEVH_B01_CHHLTD) {
                break;                                                  /* exit FOR loop    */
            }
        }
        if (j >= GRP_CYCLONEVH_DIS_RETRY_MAX) {
            lStatus = GRP_HCDI_ERROR;
        }
        for (j=0; j<GRP_CYCLONEVH_DIS_RETRY_MAX; j++) {
            ulData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCCHAR(i));
            if (!(ulData & (CYCLONEVH_B01_CHENA | CYCLONEVH_B01_CHDIS))) {
                break;                                                  /* exit FOR loop    */
            }
        }
        if (j >= GRP_CYCLONEVH_DIS_RETRY_MAX) {
            lStatus = GRP_HCDI_ERROR;
        }
    }

    _TRACE_HCDI_(0x14, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_HcInit                                                            */
/*                                                                                              */
/* DESCRIPTION: Initialization of CYCLONEV Host Controller.                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_HcInit(void)
{
grp_s32                         lStatus;
grp_u32                         ulData;

    _TRACE_HCDI_(0x01, 0x00, 0x00);
    
    /* Initialize Host Configuration register */
    CYCLONEV_R32_CR(CYCLONEV_A32_OTG_HCFG, CYCLONEVH_B02_FSLSPCLKSEL);
    CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HCFG, CYCLONEVH_VFSLSPCLKSEL_48MHZ);
#if (GRP_CYCLONEVH_SPEED_MODE == 0)
    CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HCFG, CYCLONEVH_B01_FSLSSUPP);
#else
    CYCLONEV_R32_CR(CYCLONEV_A32_OTG_HCFG, CYCLONEVH_B01_FSLSSUPP);
#endif

    /* RxFIFO size */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_GRXFSIZ, GRP_CYCLONEVH_GRXFSIZ);

    /* Non-periodic TxFIFO size */
    ulData  = (GRP_CYCLONEVH_GNPTXFSIZ << 16);
    ulData |= GRP_CYCLONEVH_GRXFSIZ;
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_GNPTXFSIZ, ulData);

    /* Periodic TxFIFO size */
    ulData  = (GRP_CYCLONEVH_HPTXFSIZ << 16);
    ulData |= (GRP_CYCLONEVH_GRXFSIZ + GRP_CYCLONEVH_GNPTXFSIZ);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HPTXFSIZ, ulData);

    /* Flush TxFIFO */
    lStatus = grp_cyclonev_cmod_FlushTxFifo(GRP_CYCLONEV_ALL_TXFIFO);
    if (GRP_COMMON_OK != lStatus) {
        _TRACE_HCDI_(0x01, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }
    
    /* Flush RxFIFO */
    lStatus = grp_cyclonev_cmod_FlushRxFifo();
    if (GRP_COMMON_OK != lStatus) {
        _TRACE_HCDI_(0x01, 0x02, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* Disable all channels interrupts */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_HAINTMSK, 0x00000000 );
    
    /* Halt all channels to put them into a known state. */
    grp_cyclonevh_DisableAllChannel();

    /* Port power on */
    CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HPRT, CYCLONEVH_B01_PRTPWR);

    _TRACE_HCDI_(0x01, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_Reset                                                            */
/*                                                                                              */
/* DESCRIPTION: Reset of HC and HCD.                                                            */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcd                           HC Handler Information                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_NO_FUNCTION            no function                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_Reset(grp_hcdi_hc_hdr *ptHcd)
{
    _TRACE_HCDI_(0x02, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x02, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_SetSof                                                           */
/*                                                                                              */
/* DESCRIPTION: Specified Frame Number is set HC.                                               */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFrmCtrl                       Frame control structure                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_NO_FUNCTION            no function                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_SetSof( grp_hcdi_frame_control *ptFrmCtrl )
{
    _TRACE_HCDI_(0x03, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x03, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_GetSof                                                           */
/*                                                                                              */
/* DESCRIPTION: Current Frame Number is read from HC.                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFrmCtrl                       Frame control structure                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_GetSof(grp_hcdi_frame_control *ptFrmCtrl)
{
    _TRACE_HCDI_(0x04, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x04, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_SetFrame                                                         */
/*                                                                                              */
/* DESCRIPTION: Specified Frame Width is set HC.                                                */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFrmCtrl                       Frame control structure                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_NO_FUNCTION            no function                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_SetFrame( grp_hcdi_frame_control *ptFrmCtrl )
{
    _TRACE_HCDI_(0x05, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x05, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_GetFrame                                                         */
/*                                                                                              */
/* DESCRIPTION: Current Frame Width is read from HC.                                            */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFrmCtrl                       Frame control structure                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_NO_FUNCTION            no function                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_GetFrame(grp_hcdi_frame_control *ptFrmCtrl)
{
    _TRACE_HCDI_(0x06, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x06, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_GlobalSuspend                                                    */
/*                                                                                              */
/* DESCRIPTION: HC is set in the state of a global suspend.                                     */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcd                           HC Handler Information                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_GlobalSuspend(grp_hcdi_hc_hdr *ptHcd)
{
    _TRACE_HCDI_(0x07, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x07, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_GlobalResume                                                     */
/*                                                                                              */
/* DESCRIPTION: HC is set in the state of a global resume.                                      */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcd                           HC Handler Information                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_GlobalResume(grp_hcdi_hc_hdr *ptHcd)
{
    _TRACE_HCDI_(0x08, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x08, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_EpOpen                                                           */
/*                                                                                              */
/* DESCRIPTION: Specified Endpoint is opened.                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcdiEp                        HCDI Endpoint Informations                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_EpOpen(grp_hcdi_endpoint *ptHcdiEp)
{
grp_cyclonevh_mng_info           *ptMngInfo  = GRP_USB_NULL;
grp_s32                         lStatus     = GRP_HCDI_OK;

    _TRACE_HCDI_(0x09, 0x00, 0x00);

    GRP_CYCLONEVH_LOCK();

    ptHcdiEp->pvHcdworkPtr = GRP_USB_NULL;

    /* get pipe information area */
    lStatus = grp_cyclonevh_MngGetPipe(ptHcdiEp, &ptMngInfo);
    if (lStatus == GRP_HCDI_OK) {
        _TRACE_HCDI_(0x09, 0x01, 0x00);
        /* set pipe number */
        ptHcdiEp->pvHcdworkPtr = (void *)ptMngInfo;
    }

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x09, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_EpClose                                                          */
/*                                                                                              */
/* DESCRIPTION: Specified Endpoint is closed.                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcdiEp                        HCDI Endpoint Informations                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_EpClose(grp_hcdi_endpoint *ptHcdiEp)
{
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptHcdiEp->pvHcdworkPtr;
grp_hcdi_tr_request             *ptTrReq;
grp_s32                         lStatus     = GRP_HCDI_OK;
grp_si                          iFlag       = GRP_USB_FALSE;
grp_si                          i;

    _TRACE_HCDI_(0x0A, 0x00, 0x00);

    /* NULL check */
    if (ptMngInfo == GRP_USB_NULL) {
        /* already closed */
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    if (grp_cyclonevh_MngGetPipeFromNum(ptMngInfo->ucChIdx, &ptTrReq) != GRP_HCDI_OK) {
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    GRP_CYCLONEVH_LOCK();

    /* cancell all request */
    for (i=0; i<GRP_CYCLONEVH_MAX_TR_NUM; i++) {
        if (ptTrReq == GRP_USB_NULL) {
            _TRACE_HCDI_(0x0A, 0x01, 0x00);
            /* all finished */
            break;                                                      /* exit FOR loop    */
        }
        if (ptTrReq->ptEndpoint == ptHcdiEp) {
            lStatus = grp_cyclonevh_MngCheckRequest(ptTrReq);
            if (lStatus != GRP_HCDI_ERROR) {
                _TRACE_HCDI_(0x0A, 0x02, 0x00);
                /* set flag */
                iFlag = GRP_USB_TRUE;
                /* set cancel status */
                ptTrReq->lStatus = GRP_USBD_TR_CANCEL;
                if (lStatus == 0) {
                    _TRACE_HCDI_(0x0A, 0x03, 0x00);
                }
            }
        }
        /* set next point */
        ptTrReq = (grp_hcdi_tr_request *)ptTrReq->pvHcdworkPtr;
    }

    if (iFlag == GRP_USB_TRUE) {
        _TRACE_HCDI_(0x0A, 0x04, 0x00);

        /* disable interrupt */
        grp_target_StopIntr(GRP_CYCLONEV_MODE_HOST);

        GRP_CYCLONEVH_UNLOCK();

        /* notify to the task */
        lStatus = grp_cyclonevh_CtrCmpPipe(0, 0, (ptMngInfo->ucChIdx | GRP_CYCLONEVH_CANCEL_FLAG));
                                      /*    ^^^^ dumy parameters  */

        GRP_CYCLONEVH_LOCK();

        /* enable interrupt */
        grp_target_StartIntr(GRP_CYCLONEV_MODE_HOST);
    }

    lStatus = grp_cyclonevh_MngReleasePipe(ptHcdiEp);

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x0A, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_EpHalt                                                           */
/*                                                                                              */
/* DESCRIPTION: The Endpoint is halted.                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcdiEp                        Endpoint Information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_EpHalt(grp_hcdi_endpoint *ptHcdiEp)
{
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptHcdiEp->pvHcdworkPtr;

    /* NULL check */
    if (ptMngInfo == GRP_USB_NULL) {
        _TRACE_HCDI_(0x0B, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    GRP_CYCLONEVH_LOCK();

    /* set halt status to the pipe */
    ptMngInfo->ucStatus |= GRP_CYCLONEVH_PS_PIPE_HALT;

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x0B, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_EpActive                                                         */
/*                                                                                              */
/* DESCRIPTION: The Endpoint is activated.                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcdiEp                        Endpoint Information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_EpActive(grp_hcdi_endpoint *ptHcdiEp)
{
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptHcdiEp->pvHcdworkPtr;

    _TRACE_HCDI_(0x0C, 0x00, 0x00);

    /* NULL check */
    if (ptMngInfo == GRP_USB_NULL) {
        _TRACE_HCDI_(0x0C, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    GRP_CYCLONEVH_LOCK();

    /* clear halt status */
    ptMngInfo->ucStatus &= ~GRP_CYCLONEVH_PS_PIPE_HALT;

    /* clear toggle information */
    ptMngInfo->ucDPID = GRP_CYCLONEVH_PID_DATA0;

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x0C, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_EpFlash                                                          */
/*                                                                                              */
/* DESCRIPTION: The communication data of Endpoint is deleted.                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcdiEp                        Endpoint Information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_EpFlash(grp_hcdi_endpoint *ptHcdiEp)
{
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptHcdiEp->pvHcdworkPtr;
grp_s32                         lStatus     = GRP_HCDI_OK;

    _TRACE_HCDI_(0x0D, 0x00, 0x00);

    /* NULL check */
    if (ptMngInfo == GRP_USB_NULL) {
        _TRACE_HCDI_(0x0D, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    GRP_CYCLONEVH_LOCK();

    /* set cancel flag to all request */
    lStatus = grp_cyclonevh_MngCancelAllReq(ptMngInfo->ucChIdx, ptHcdiEp);

    if (lStatus == GRP_HCDI_OK) {
        _TRACE_HCDI_(0x0D, 0x01, 0x00);

        /* disable interrupt */
        grp_target_StopIntr(GRP_CYCLONEV_MODE_HOST);

        /* notify to the task */
        lStatus = grp_cyclonevh_CtrCmpPipe(0, 0, (ptMngInfo->ucChIdx | GRP_CYCLONEVH_CANCEL_FLAG));
                                      /*    ^^^^ dumy parameters  */

        /* enable interrupt */
        grp_target_StartIntr(GRP_CYCLONEV_MODE_HOST);
    }
    else {
        _TRACE_HCDI_(0x0D, 0x02, 0x00);
        /* no request specified pipe */
        lStatus = GRP_USBD_ALREADY_XFER;
    }

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x0D, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_TrRun                                                            */
/*                                                                                              */
/* DESCRIPTION: The transfer request other than Isochronous Transfer are accepted.              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_TrRun(grp_hcdi_tr_request *ptTrReq)
{
grp_s32                         lStatus = GRP_HCDI_OK;

    _TRACE_HCDI_(0x0E, 0x00, 0x00);

    GRP_CYCLONEVH_LOCK();

    /* clear actual length */
    ptTrReq->ulActualLength = 0;
    ptTrReq->lStatus        = GRP_USBD_TR_NOT_PROCESS;

    lStatus = grp_cyclonevh_MngSetRequest(ptTrReq);
    if (lStatus == GRP_HCDI_OK) {
        _TRACE_HCDI_(0x0E, 0x01, 0x00);
        /* 1st request */
        lStatus = grp_cyclonevh_CtrTrRun(ptTrReq);
        if (lStatus != GRP_HCDI_OK) {
            /* deleted from the list */
            grp_cyclonevh_MngDelRequest(ptTrReq);
        }
    }
    else if (lStatus != GRP_HCDI_ERROR) {
        _TRACE_HCDI_(0x0E, 0x02, 0x00);
        /* not 1st request */
        lStatus = GRP_HCDI_OK;
    }
/*  else {                                                                  */
/*      /- error -/                                                         */
/*  }                                                                       */

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x0E, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_TrDel                                                            */
/*                                                                                              */
/* DESCRIPTION: The transfer request other than Isochronous Transfer are deleted.               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_TrDel(grp_hcdi_tr_request *ptTrReq)
{
grp_hcdi_endpoint               *ptHcdiEp   = (grp_hcdi_endpoint *)ptTrReq->ptEndpoint;
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptHcdiEp->pvHcdworkPtr;
grp_s32                         lStatus     = GRP_HCDI_OK;

    _TRACE_HCDI_(0x0F, 0x00, 0x00);

    /* NULL check */
    if (ptMngInfo == GRP_USB_NULL) {
        _TRACE_HCDI_(0x0F, 0x01, END_FUNC);
        /* already closed */
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    GRP_CYCLONEVH_LOCK();

    lStatus = grp_cyclonevh_MngCheckRequest(ptTrReq);
    if (lStatus != GRP_HCDI_ERROR) {
        _TRACE_HCDI_(0x0F, 0x01, 0x00);
        /* set cancel status */
        ptTrReq->lStatus = GRP_USBD_TR_CANCEL;
        if (lStatus == 0) {
            _TRACE_HCDI_(0x0F, 0x02, 0x00);
        }

        /* disable interrupt */
        grp_target_StopIntr(GRP_CYCLONEV_MODE_HOST);

        /* notify to the task */
        lStatus = grp_cyclonevh_CtrCmpPipe(0, 0, (ptMngInfo->ucChIdx | GRP_CYCLONEVH_CANCEL_FLAG));
                                      /*    ^^^^ dumy parameters  */

        /* enable interrupt */
        grp_target_StartIntr(GRP_CYCLONEV_MODE_HOST);
    }
    else {
        /* request is already completed */
        lStatus = GRP_USBD_ALREADY_XFER;
    }

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x0F, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_ItrRun                                                           */
/*                                                                                              */
/* DESCRIPTION: The Isochronous Transfer request are accepted.                                  */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_NO_FUNCTION            no function                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_ItrRun(grp_hcdi_tr_request *ptTrReq)
{
    _TRACE_HCDI_(0x10, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x10, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_ItrDel                                                           */
/*                                                                                              */
/* DESCRIPTION: The Isochronous Transfer request are deleted.                                   */
/*              !!! NO SUPPORT !!!                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_NO_FUNCTION            no function                                     */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_ItrDel(grp_hcdi_tr_request *ptTrReq)
{
    _TRACE_HCDI_(0x11, 0x00, 0x00);

    /* No support */

    _TRACE_HCDI_(0x11, 0x00, END_FUNC);

    return GRP_HCDI_NO_FUNCTION;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_PortControl                                                      */
/*                                                                                              */
/* DESCRIPTION: USB Port is controlled.                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPtCtrl                        HCDI Port Control Information                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_INVALIED_PMTR          Parameter error                                 */
/*              GRP_HCDI_ERROR                  Other errors                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_PortControl(grp_hcdi_port_control* ptPtCtrl)
{
grp_s32                         lStatus = GRP_HCDI_OK;

    _TRACE_HCDI_(0x12, 0x00, 0x00);

    GRP_CYCLONEVH_LOCK();

    switch (ptPtCtrl->iPortReq) {
    case GRP_USBD_PORT_SUSPEND_CONTROL:
        _TRACE_HCDI_(0x12, 0x01, 0x00);
        lStatus = GRP_HCDI_NO_FUNCTION;
        break;

    case GRP_USBD_PORT_RESUME_CONTROL:
        _TRACE_HCDI_(0x12, 0x02, 0x00);
        lStatus = GRP_HCDI_NO_FUNCTION;
        break;

    case GRP_USBD_PORT_RESET_CONTROL:
        _TRACE_HCDI_(0x12, 0x03, 0x00);
        GRP_CYCLONEVH_UNLOCK();
        lStatus = grp_cyclonevh_RhubEnumeration(ptPtCtrl->tPortHdr.ucPort);
        GRP_CYCLONEVH_LOCK();
        break;

    default:
        _TRACE_HCDI_(0x12, 0x04, 0x00);
        lStatus = GRP_HCDI_INVALIED_PMTR;
        break;
    }

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_HCDI_(0x12, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_SetVbusPower                                                      */
/*                                                                                              */
/* DESCRIPTION: Perform power control of VBUS.                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : lPower                          Power control flag.                             */
/*                                               GRP_CYCLONEVH_VBUS_POWER_OFF: VBUS power off   */
/*                                               GRP_CYCLONEVH_VBUS_POWER_ON: VBUS power ON     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_INVALIED_PMTR          Parameter error                                 */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_SetVbusPower(grp_s32 lPower)
{
grp_s32                         lStatus = GRP_HCDI_OK;

    _TRACE_HCDI_(0x14, 0x00, 0x00);

    if (GRP_CYCLONEVH_VBUS_POWER_OFF == lPower) {
        _TRACE_HCDI_(0x14, 0x01, 0x00);
        /* Port power off */
        CYCLONEV_R32_CR(CYCLONEV_A32_OTG_HPRT, CYCLONEVH_B01_PRTPWR);
    }
    else if (GRP_CYCLONEVH_VBUS_POWER_ON == lPower){
        _TRACE_HCDI_(0x14, 0x02, 0x00);
        /* Port power on */
        CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HPRT, CYCLONEVH_B01_PRTPWR);
    }
    else {
        _TRACE_HCDI_(0x14, 0x03, 0x00);
        lStatus = GRP_HCDI_INVALIED_PMTR;
    }

    _TRACE_HCDI_(0x14, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_CB_Process                                                        */
/*                                                                                              */
/* DESCRIPTION: Callback function and next transfers start if it exist.                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_CB_Process(grp_hcdi_tr_request *ptTrReq, grp_u8 ucIdx)
{
grp_hcdi_tr_request             *ptNextTrReq;
grp_si                          i;
grp_cyclonevh_mng_info          *ptMngInfo;

    _TRACE_HCDI_(0x13, 0x00, 0x00);

    if (ucIdx <= GRP_CYCLONEVH_CTRL_PIPE) {
        /* Control transfer */
        /* Mask interrupts on all channels used in control transfer. */
        for (i = 0; i <= ucIdx; i++) {
            /* i number Mask all interrupts on the channel. */
            CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINTMSK(i), 0);
        }
    }
    else {
        /* ucIdx number Mask all interrupts on the channel. */
        CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINTMSK(ucIdx), 0);
    }

    /* Null check */
    if (ptTrReq == GRP_USB_NULL) {
        /* success : no request */
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /* get pipe information */
    ptMngInfo = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;

    /* check data overrun */
    if (ptMngInfo->ulTmpBufLen > ptTrReq->ulBufferLength) {
        if (ptTrReq->lStatus == GRP_USBD_TR_NO_FAIL) {
            /* set overrun */
            ptTrReq->lStatus = GRP_USBD_TR_DATA_OVERRUN;
            _TRACE_CTRL_(0x13, 0x03, 0x00);
        }
    }

    /* deleted from the list */
    if (grp_cyclonevh_MngReleaseRequest(ptTrReq, &ptNextTrReq) != GRP_HCDI_OK) {
        _TRACE_HCDI_(0x13, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* check next requesut */
    if (ptNextTrReq) {
        _TRACE_HCDI_(0x13, 0x01, 0x00);
        /* next transfer */
        grp_cyclonevh_CtrTrRun(ptNextTrReq);
    }

    GRP_CYCLONEVH_UNLOCK();

    /* notified upper layer */
    for (i=0; i<GRP_CYCLONEVH_MAX_TR_NUM; i++) {
        if (grp_cyclonevh_MngGetRequest(&ptTrReq) != GRP_HCDI_OK) {
            _TRACE_HCDI_(0x13, 0x02, 0x00);
            /* no request */
            break;                                                      /* exit FOR loop    */
        }

        /* nortified to the upper layer */
        if (ptTrReq->pfnCompFunc) {
            ptTrReq->pfnCompFunc(ptTrReq);
        }
    }

    GRP_CYCLONEVH_LOCK();

    _TRACE_HCDI_(0x13, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

