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
/*      grp_cyclonevh_mng.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      CYCLONEV Host Controller Driver management module                                       */
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

/*** INTERNAL DATA DEFINES **********************************************************************/
DLOCAL grp_cyclonevh_mng_info           l_atMngInfo[GRP_CYCLONEVH_MAX_INFO_NUM]   = {0};
DLOCAL grp_cyclonevh_pipe_info          l_atPipeInfo[GRP_CYCLONEVH_MAX_PIPE_NUM]  = {0};
DLOCAL grp_hcdi_tr_request              *l_ptDoneList                             = GRP_USB_NULL;

/*** INTERNAL FUNCTION PROTOTYPES ***************************************************************/

LOCAL grp_u32 _grp_cyclonevh_MngMakeChInfo(grp_hcdi_endpoint *ptHcdiEp);

LOCAL grp_u32 _grp_cyclonevh_MngMakeChInfo(grp_hcdi_endpoint *ptHcdiEp)
{
grp_u32                         ulData = 0;

    ulData |= (ptHcdiEp->ucAddress << 22);
    ulData |= (ptHcdiEp->ucTxMode  << 18);
    ulData |= (ptHcdiEp->ucTxSpeed == GRP_USBD_LOW_SPEED) ? CYCLONEVH_B01_LSPDDEV : 0x00000000;
    if (ptHcdiEp->ucTxMode != GRP_USBD_CONTROL) {
        ulData |= (ptHcdiEp->ucTxDir   << 15);
    }
    ulData |= (ptHcdiEp->ucEpNum   << 11);
    ulData |= (ptHcdiEp->usMaxPacketSize);
    ulData |= CYCLONEVH_VEC_TR1;

    return ulData;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngInit                                                           */
/*                                                                                              */
/* DESCRIPTION: Initialize this modules.                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngInit(void)
{
grp_si                          i;

    _TRACE_MGR_(0x01, 0x00, 0x00);

    grp_std_memset(&l_atMngInfo,  0, sizeof(l_atMngInfo));
    grp_std_memset(&l_atPipeInfo, 0, sizeof(l_atPipeInfo));

    /* intialize parameters */
    for (i=0; i<GRP_CYCLONEVH_MAX_INFO_NUM; i++) {
        l_atMngInfo[i].ucStatus     = GRP_CYCLONEVH_PS_NO_USE;
    }

    /* set pipe informations */
    l_atPipeInfo[0].ucTxMode        = GRP_CYCLONEVH_PIPE0_TYPE;
    l_atPipeInfo[0].ucTxDir         = GRP_CYCLONEVH_PIPE0_DIR;
    l_atPipeInfo[1].ucTxMode        = GRP_CYCLONEVH_PIPE1_TYPE;
    l_atPipeInfo[1].ucTxDir         = GRP_CYCLONEVH_PIPE1_DIR;
    l_atPipeInfo[2].ucTxMode        = GRP_CYCLONEVH_PIPE2_TYPE;
    l_atPipeInfo[2].ucTxDir         = GRP_CYCLONEVH_PIPE2_DIR;
    l_atPipeInfo[3].ucTxMode        = GRP_CYCLONEVH_PIPE3_TYPE;
    l_atPipeInfo[3].ucTxDir         = GRP_CYCLONEVH_PIPE3_DIR;
    l_atPipeInfo[4].ucTxMode        = GRP_CYCLONEVH_PIPE4_TYPE;
    l_atPipeInfo[4].ucTxDir         = GRP_CYCLONEVH_PIPE4_DIR;
    l_atPipeInfo[5].ucTxMode        = GRP_CYCLONEVH_PIPE5_TYPE;
    l_atPipeInfo[5].ucTxDir         = GRP_CYCLONEVH_PIPE5_DIR;
    l_atPipeInfo[6].ucTxMode        = GRP_CYCLONEVH_PIPE6_TYPE;
    l_atPipeInfo[6].ucTxDir         = GRP_CYCLONEVH_PIPE6_DIR;
    l_atPipeInfo[7].ucTxMode        = GRP_CYCLONEVH_PIPE7_TYPE;
    l_atPipeInfo[7].ucTxDir         = GRP_CYCLONEVH_PIPE7_DIR;
    l_atPipeInfo[8].ucTxMode        = GRP_CYCLONEVH_PIPE8_TYPE;
    l_atPipeInfo[8].ucTxDir         = GRP_CYCLONEVH_PIPE8_DIR;
    l_atPipeInfo[9].ucTxMode        = GRP_CYCLONEVH_PIPE9_TYPE;
    l_atPipeInfo[9].ucTxDir         = GRP_CYCLONEVH_PIPE9_DIR;
    l_atPipeInfo[10].ucTxMode       = GRP_CYCLONEVH_PIPE10_TYPE;
    l_atPipeInfo[10].ucTxDir        = GRP_CYCLONEVH_PIPE10_DIR;
    l_atPipeInfo[11].ucTxMode       = GRP_CYCLONEVH_PIPE11_TYPE;
    l_atPipeInfo[11].ucTxDir        = GRP_CYCLONEVH_PIPE11_DIR;
    l_atPipeInfo[12].ucTxMode       = GRP_CYCLONEVH_PIPE12_TYPE;
    l_atPipeInfo[12].ucTxDir        = GRP_CYCLONEVH_PIPE12_DIR;
    l_atPipeInfo[13].ucTxMode       = GRP_CYCLONEVH_PIPE13_TYPE;
    l_atPipeInfo[13].ucTxDir        = GRP_CYCLONEVH_PIPE13_DIR;
    l_atPipeInfo[14].ucTxMode       = GRP_CYCLONEVH_PIPE14_TYPE;
    l_atPipeInfo[14].ucTxDir        = GRP_CYCLONEVH_PIPE14_DIR;
    l_atPipeInfo[15].ucTxMode       = GRP_CYCLONEVH_PIPE15_TYPE;
    l_atPipeInfo[15].ucTxDir        = GRP_CYCLONEVH_PIPE15_DIR;

    _TRACE_MGR_(0x01, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngGetPipe                                                        */
/*                                                                                              */
/* DESCRIPTION: Get pipe information area from transfer type                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucTxMode                        Transfer type                                   */
/* OUTPUT     : pptPipeInfo                     Pipe information                                */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngGetPipe(grp_hcdi_endpoint *ptHcdiEp, grp_cyclonevh_mng_info **pptMngInfo)
{
grp_si                          i;
grp_cyclonevh_pipe_info        *ptPipeInfo = GRP_USB_NULL;

    _TRACE_MGR_(0x02, 0x00, 0x00);

    for (i=0; i<GRP_CYCLONEVH_MAX_INFO_NUM; i++) {
        if (l_atMngInfo[i].ucStatus == GRP_CYCLONEVH_PS_NO_USE) {
            l_atMngInfo[i].ucStatus = GRP_CYCLONEVH_PS_USE;
            *pptMngInfo             = &l_atMngInfo[i];
            break;                                              /* Success and exit FOR loop    */
        }
    }
    if (i == GRP_CYCLONEVH_MAX_INFO_NUM) {
        _TRACE_MGR_(0x02, 0x01, _F_END_);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    if (ptHcdiEp->ucTxMode == GRP_USBD_CONTROL) {
        /* set management information */
        (*pptMngInfo)->ucChIdx      = GRP_CYCLONEVH_PIPE0;
        (*pptMngInfo)->ucDPID       = GRP_CYCLONEVH_PID_DATA0;
        (*pptMngInfo)->ulTmpBufLen  = 0;
        (*pptMngInfo)->ulHcChar     = _grp_cyclonevh_MngMakeChInfo(ptHcdiEp);
        /* set channels interrupt mask */
        CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HAINTMSK, (CYCLONEVH_VHAINT_CH(0) | CYCLONEVH_VHAINT_CH(1) | CYCLONEVH_VHAINT_CH(2)));
        _TRACE_MGR_(0x02, 0x02, _F_END_);
        /* success */
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /* case not Controrl transfer */

    for (i=3; i<GRP_CYCLONEVH_MAX_PIPE_NUM; i++) {
        /* search same conditions */
        if ((l_atPipeInfo[i].ucTxMode == ptHcdiEp->ucTxMode)
         && (l_atPipeInfo[i].ucTxDir  == ptHcdiEp->ucTxDir)) {
            /* don't use? */
            if (l_atPipeInfo[i].ucStatus == GRP_CYCLONEVH_PS_NO_USE) {
                l_atPipeInfo[i].ucStatus = GRP_CYCLONEVH_PS_USE;
                ptPipeInfo               = &l_atPipeInfo[i];
                /* set channels interrupt mask */
                CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HAINTMSK, (1 << i));
                break;                                          /* success and exit FOR loop    */
            }
            else if (ptPipeInfo == GRP_USB_NULL) {
                ptPipeInfo = &l_atPipeInfo[i];
            }
            else if (ptPipeInfo->ucCount > l_atPipeInfo[i].ucCount) {
                ptPipeInfo = &l_atPipeInfo[i];
            }
        }
    }
    if (i == GRP_CYCLONEVH_MAX_PIPE_NUM) {
        _TRACE_MGR_(0x02, 0x03, _F_END_);
        /* error */
        (*pptMngInfo)->ucStatus = GRP_CYCLONEVH_PS_NO_USE;
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* renew pipe information */
    ptPipeInfo->ucCount++;
    /* set management information */
    (*pptMngInfo)->ucChIdx      = i;
    (*pptMngInfo)->ucDPID       = GRP_CYCLONEVH_PID_DATA0;
    (*pptMngInfo)->ulTmpBufLen  = 0;
    (*pptMngInfo)->ulHcChar     = _grp_cyclonevh_MngMakeChInfo(ptHcdiEp);

    _TRACE_MGR_(0x02, 0x00, _F_END_);

    /* success */
    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngReleasePipe                                                    */
/*                                                                                              */
/* DESCRIPTION: Release pipe information area.                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucTxMode                        Transfer type                                   */
/*              ptMngInfo                       Pipe information                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngReleasePipe(grp_hcdi_endpoint *ptHcdiEp)
{
grp_cyclonevh_mng_info         *ptMngInfo = (grp_cyclonevh_mng_info *)ptHcdiEp->pvHcdworkPtr;

    _TRACE_MGR_(0x03, 0x00, 0x00);

    if (ptHcdiEp->ucTxMode == GRP_USBD_CONTROL) {
        _TRACE_MGR_(0x03, 0x01, 0x00);
        /* clear channels interrupt mask */
        CYCLONEV_R32_CR(CYCLONEV_A32_OTG_HAINTMSK, (CYCLONEVH_VHAINT_CH(0) | CYCLONEVH_VHAINT_CH(1) | CYCLONEVH_VHAINT_CH(2)));
    } else {
        _TRACE_MGR_(0x03, 0x02, 0x00);
        /* case not Controrl transfer */

        /* renew pipe information */
        l_atPipeInfo[ptMngInfo->ucChIdx].ucCount--;
        if (l_atPipeInfo[ptMngInfo->ucChIdx].ucCount == 0) {
            _TRACE_MGR_(0x03, 0x03, 0x00);
            /* set NO_USE information */
            l_atPipeInfo[ptMngInfo->ucChIdx].ucStatus = GRP_CYCLONEVH_PS_NO_USE;
            /* clear channels interrupt mask */
            CYCLONEV_R32_CR(CYCLONEV_A32_OTG_HAINTMSK, (1 << ptMngInfo->ucChIdx));
        }
    }

    /* clear informations */
    ptMngInfo->ucStatus     = GRP_CYCLONEVH_PS_NO_USE;
    ptMngInfo->ucChIdx      = 0;
    ptMngInfo->ucDPID       = GRP_CYCLONEVH_PID_DATA0;
    ptMngInfo->ulTmpBufLen  = 0;
    ptMngInfo->ulHcChar     = 0;

    _TRACE_MGR_(0x03, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngSetRequest                                                     */
/*                                                                                              */
/* DESCRIPTION: Set transfer request structure.                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              other                           Success but not first request                   */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngSetRequest(grp_hcdi_tr_request *ptTrReq)
{
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;
grp_cyclonevh_pipe_info        *ptPipeInfo = &l_atPipeInfo[ptMngInfo->ucChIdx];
grp_hcdi_tr_request             *ptTmpTrReq;
grp_si                          i;
grp_s32                         lStatus     = GRP_HCDI_ERROR;

    _TRACE_MGR_(0x04, 0x00, 0x00);

    if (ptPipeInfo->ptTrReq == GRP_USB_NULL) {
        _TRACE_MGR_(0x04, 0x01, 0x00);
        /* success */
        ptPipeInfo->ptTrReq   = ptTrReq;
        ptTrReq->pvHcdworkPtr = GRP_USB_NULL;
        lStatus = GRP_HCDI_OK;
    }
    else {
        ptTmpTrReq = ptPipeInfo->ptTrReq;
        for (i=1; i<GRP_CYCLONEVH_MAX_TR_NUM; i++) {
            if (ptTmpTrReq->pvHcdworkPtr == GRP_USB_NULL) {
                _TRACE_MGR_(0x04, 0x02, 0x00);
                /* success */
                ptTmpTrReq->pvHcdworkPtr = (void *)ptTrReq;
                ptTrReq->pvHcdworkPtr    = GRP_USB_NULL;
                lStatus = i;
                break;                                                  /* exit FOR loop    */
            }
            ptTmpTrReq = (grp_hcdi_tr_request *)ptTmpTrReq->pvHcdworkPtr;
        }
    }

    _TRACE_MGR_(0x04, 0x00, _F_END_);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngGetPipeFromNum                                                 */
/*                                                                                              */
/* DESCRIPTION: Get pipe information area from pipe number                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucPipeNum                       Pipe number                                     */
/* OUTPUT     : pptPipeInfo                     Pipe information                                */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngGetPipeFromNum(grp_u8 ucPipeNum, grp_hcdi_tr_request **pptTrReq)
{
    _TRACE_MGR_(0x05, 0x00, 0x00);

    /* check pipe number */
    if (ucPipeNum >= GRP_CYCLONEVH_MAX_PIPE_NUM) {
        _TRACE_MGR_(0x05, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    *pptTrReq = l_atPipeInfo[ucPipeNum].ptTrReq;

    _TRACE_MGR_(0x05, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngGetRequest                                                     */
/*                                                                                              */
/* DESCRIPTION: Get transfer request information                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : pptTrReq                        Transfer Request Information                    */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngGetRequest(grp_hcdi_tr_request **pptTrReq)
{
    _TRACE_MGR_(0x06, 0x00, 0x00);

    /* check pipe number */
    if (l_ptDoneList == GRP_USB_NULL) {
        _TRACE_MGR_(0x06, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    *pptTrReq    = l_ptDoneList;
    l_ptDoneList = ((grp_hcdi_tr_request *)*pptTrReq)->pvHcdworkPtr;

    _TRACE_MGR_(0x06, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngReleaseRequest                                                 */
/*                                                                                              */
/* DESCRIPTION: Release head transfer request and return next transfer information if it exist  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : pptNextReq                      Next transfer Request Information               */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngReleaseRequest(grp_hcdi_tr_request *ptTrReq, grp_hcdi_tr_request **pptNextReq)
{
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;
grp_cyclonevh_pipe_info        *ptPipeInfo = &l_atPipeInfo[ptMngInfo->ucChIdx];
grp_hcdi_tr_request             *ptTmpTrReq = GRP_USB_NULL;
grp_si                          i;
grp_s32                         lStatus     = GRP_HCDI_ERROR;
grp_si                          *piTmpList  = (grp_si *)&ptPipeInfo->ptTrReq;
grp_si                          *piDoneList = (grp_si *)&l_ptDoneList;

    _TRACE_MGR_(0x07, 0x00, 0x00);

    /* initialize */
    *pptNextReq = GRP_USB_NULL;

    if (ptPipeInfo->ptTrReq != GRP_USB_NULL) {
        ptTmpTrReq = ptPipeInfo->ptTrReq;
        for (i=0; i<GRP_CYCLONEVH_MAX_TR_NUM; i++) {
            if (ptTmpTrReq->lStatus != GRP_USBD_TR_NOT_PROCESS) {
                _TRACE_MGR_(0x07, 0x01, 0x00);
                /* success */
                lStatus     = GRP_HCDI_OK;
                /* set done list */
                *piDoneList = (grp_si)ptTmpTrReq;
                piDoneList  = (grp_si *)&ptTmpTrReq->pvHcdworkPtr;
                /* set next pointer to the previous list poiter */
                *piTmpList  = (grp_si)ptTmpTrReq->pvHcdworkPtr;
                /* set next pointer */
                ptTmpTrReq  = ptTmpTrReq->pvHcdworkPtr;
                *piDoneList = GRP_USB_NULL;
                if (ptTmpTrReq == GRP_USB_NULL) {
                    /* not found */
                    break;                                              /* exit FOR loop    */
                }
            }
            else {
                _TRACE_MGR_(0x07, 0x02, 0x00);
                if (*pptNextReq == GRP_USB_NULL) {
                    *pptNextReq = ptTmpTrReq;
                }
                /* change previous list pointer */
                piTmpList   = (grp_si *)&ptTmpTrReq->pvHcdworkPtr;
                /* set next pointer */
                ptTmpTrReq = ptTmpTrReq->pvHcdworkPtr;
                if (ptTmpTrReq == GRP_USB_NULL) {
                    /* not found */
                    break;                                              /* exit FOR loop    */
                }
            }
        }
    }

    _TRACE_MGR_(0x07, 0x00, _F_END_);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngDelRequest                                                     */
/*                                                                                              */
/* DESCRIPTION: Delete transfer request                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngDelRequest(grp_hcdi_tr_request *ptTrReq)
{
grp_cyclonevh_mng_info         *ptMngInfo;
grp_cyclonevh_pipe_info        *ptPipeInfo;
grp_s32                         lStatus     = GRP_HCDI_ERROR;

    _TRACE_MGR_(0x08, 0x00, 0x00);

    if (ptTrReq != GRP_USB_NULL) {
        ptMngInfo  = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;
        if (ptMngInfo != GRP_USB_NULL) {
            ptPipeInfo = &l_atPipeInfo[ptMngInfo->ucChIdx];
            if (ptPipeInfo->ptTrReq != GRP_USB_NULL) {
                _TRACE_MGR_(0x08, 0x01, 0x00);
                ptPipeInfo->ptTrReq = GRP_USB_NULL;
               /* success */
               lStatus     = GRP_HCDI_OK;
            }
        }
    }

    _TRACE_MGR_(0x08, 0x00, _F_END_);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngCheckRequest                                                   */
/*                                                                                              */
/* DESCRIPTION: check request on the list                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : other                           Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngCheckRequest(grp_hcdi_tr_request *ptTrReq)
{
grp_cyclonevh_mng_info         *ptMngInfo  = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;
grp_cyclonevh_pipe_info        *ptPipeInfo = &l_atPipeInfo[ptMngInfo->ucChIdx];
grp_hcdi_tr_request             *ptTmpTrReq;
grp_si                          i;
grp_s32                         lStatus     = GRP_HCDI_ERROR;

    _TRACE_MGR_(0x09, 0x00, 0x00);

    if (ptPipeInfo->ptTrReq != GRP_USB_NULL) {
        ptTmpTrReq = ptPipeInfo->ptTrReq;
        for (i=0; i<GRP_CYCLONEVH_MAX_TR_NUM; i++) {
            if (ptTmpTrReq == ptTrReq) {
                _TRACE_MGR_(0x09, 0x01, 0x00);
                lStatus = i;
                break;                                                  /* exit FOR loop    */
            }
            else {
                ptTmpTrReq = ptTmpTrReq->pvHcdworkPtr;
                if (ptTmpTrReq == GRP_USB_NULL) {
                    _TRACE_MGR_(0x09, 0x02, 0x00);
                    /* not found */
                    break;                                              /* exit FOR loop    */
                }
            }
        }
    }

    _TRACE_MGR_(0x09, 0x00, _F_END_);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngCancelAllReq                                                   */
/*                                                                                              */
/* DESCRIPTION: All request are canceled.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucIdx                           Index of pipe number                            */
/* OUTPUT     : ptHcdiEp                        Endpoint Information                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngCancelAllReq(grp_u8 ucIdx, grp_hcdi_endpoint *ptHcdiEp)
{
grp_cyclonevh_pipe_info        *ptPipeInfo = &l_atPipeInfo[ucIdx];
grp_hcdi_tr_request             *ptTmpTrReq;
grp_si                          i;
grp_s32                         lStatus     = GRP_HCDI_ERROR;

    _TRACE_MGR_(0x0A, 0x00, 0x00);

    if (ptPipeInfo->ptTrReq != GRP_USB_NULL) {
        ptTmpTrReq = ptPipeInfo->ptTrReq;
    }
    else {
        _TRACE_MGR_(0x0A, 0x01, _F_END_);
        /* process was end */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    for (i=0; i<GRP_CYCLONEVH_MAX_TR_NUM; i++) {
        if (ptTmpTrReq == GRP_USB_NULL) {
            _TRACE_MGR_(0x0A, 0x01, 0x00);
            /* request was end */
            break;                                                      /* exit FOR loop    */
        }
        /* check informations */
        if (ptTmpTrReq->ptEndpoint  == ptHcdiEp) {
            _TRACE_MGR_(0x0A, 0x02, 0x00);
            /* set status to the cancel */
            ptTmpTrReq->lStatus = GRP_USBD_TR_CANCEL;
            /* set status */
            lStatus = GRP_HCDI_OK;
        }
        /* next TrReq pointer is set */
        ptTmpTrReq = (grp_hcdi_tr_request *)ptTmpTrReq->pvHcdworkPtr;
    }

    _TRACE_MGR_(0x0A, 0x00, _F_END_);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_MngNotCmpReqAbort                                                 */
/*                                                                                              */
/* DESCRIPTION: Check if the request completed, abort.                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_MngNotCmpReqAbort(void)
{
grp_si                          i;
grp_s32                         lStatus;
grp_cyclonevh_mng_info         *ptMngInfo;
grp_hcdi_tr_request             *ptTrReq;

    _TRACE_MGR_(0x0B, 0x00, 0x00);

    GRP_CYCLONEVH_LOCK();

    for (i=0; i<GRP_CYCLONEVH_MAX_INFO_NUM; i++) {
        ptMngInfo = &l_atMngInfo[i];
        /* Check Not Complete TrRequest */
        if ((ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_SETUP_OUT)
            || (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_SETUP_IN)
            || (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_DATA_OUT)
            || (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_DATA_IN)
            || (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_DATA_0_OUT)
            || (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_DATA_0_IN)) {
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_ERROR;
            /* get TrRequest */
            lStatus = grp_cyclonevh_MngGetPipeFromNum(ptMngInfo->ucChIdx, &ptTrReq);
            if ((lStatus == GRP_HCDI_OK) && (ptTrReq != GRP_USB_NULL)) {
                _TRACE_MGR_(0x0B, 0x01, 0x00);
                /* set status */
                ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
                /* call back : error */
                grp_cyclonevh_CB_Process(ptTrReq, ptMngInfo->ucChIdx);
            }
        }
    }

    GRP_CYCLONEVH_UNLOCK();

    _TRACE_MGR_(0x0B, 0x00, _F_END_);

    /* success */
    return GRP_HCDI_OK;
}

