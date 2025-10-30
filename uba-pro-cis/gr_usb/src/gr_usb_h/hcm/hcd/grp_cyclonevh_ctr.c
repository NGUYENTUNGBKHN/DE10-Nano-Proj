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
/*      grp_cyclonevh_ctr.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      CYCLONEV Host Controller Driver control module                                          */
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
DLOCAL grp_vos_t_task                   *l_ptCbTask;

DLOCAL grp_vos_t_flag                   *l_ptCyclonevhFlg;
/* Event */
#define GRP_CYCLONEVH_HCD_NTC               (1)
#define GRP_CYCLONEVH_HCD_NTC_CLR           (0)

/* Flag */
#define GRP_CYCLONEVH_HCD_1ST_REQ           ((grp_u8)1)
#define GRP_CYCLONEVH_HCD_CNT_REQ           ((grp_u8)2)

/* Event information block */
typedef struct  grp_cyclonevh_tr_ntc_tg {
    grp_u8                              ucPipeNum;
    grp_u8                              ucPad[3];
    grp_u16                             usIntrFactor;
    grp_u16                             usPktData;
} grp_cyclonevh_tr_ntc;

/* local data */
DLOCAL grp_cyclonevh_tr_ntc             l_atEvnt[GRP_CYCLONEVH_MAX_TR_NUM] = {0};
/* pointer */
DLOCAL grp_u8                           l_ucEvntRdPtr = 0;
DLOCAL grp_u8                           l_ucEvntWrPtr = 0;

/*** INTERNAL FUNCTION PROTOTYPES ***************************************************************/
LOCAL grp_s32   _grp_cyclonevh_CtrSetupData(grp_hcdi_tr_request *ptTrReq);
LOCAL grp_s32   _grp_cyclonevh_CtrDataIn(grp_hcdi_tr_request *ptTrReq, grp_cyclonevh_mng_info *ptMngInfo);
LOCAL grp_s32   _grp_cyclonevh_CtrDataOut(grp_hcdi_tr_request *ptTrReq, grp_cyclonevh_mng_info *ptMngInfo);
LOCAL grp_s32   _grp_cyclonevh_CtrBulk(grp_hcdi_tr_request *ptTrReq, grp_u8 ucFlag);
LOCAL grp_s32   _grp_cyclonevh_CtrRdDone(grp_hcdi_tr_request *ptTrReq, grp_cyclonevh_mng_info *ptMngInfo, grp_u16 usPktData);
LOCAL grp_s32   _grp_cyclonevh_CtrGetCmpPipe(grp_u16 *pusIntrFactor, grp_u16 *pusPktData, grp_u8 *pucPipeNum);
LOCAL void      _grp_cyclonevh_Dumy_Task(grp_u32 dumy);
LOCAL grp_s32   _grp_cyclonevh_CtrCb_Task(grp_u32 dumy);
LOCAL grp_s32   _grp_cyclonevh_CtrReqCancel(grp_u8 ucPipeNum);
LOCAL grp_s32   _grp_cyclonevh_CtrDisableChannel(grp_u8 ucChIdx);
LOCAL grp_s32   _grp_cyclonevh_CtrCmpControl(grp_u16 usIntrFactor, grp_u16 usPktData, grp_u8 ucPipeNum);
LOCAL grp_s32   _grp_cyclonevh_CtrCmpOtherXfer(grp_u16 usIntrFactor, grp_u16 usPktData, grp_u8 ucPipeNum);
LOCAL grp_s32   _grp_cyclonevh_CtrCmpErrChk(grp_u16 usIntrFactor);


/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_CtrInit                                                           */
/*                                                                                              */
/* DESCRIPTION: Initialize this modules.                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_CtrInit(void)
{
    _TRACE_CTRL_(0x01, 0x00, 0x00);

    /* Create A event flag. for File System Interface   */
    if (GRP_VOS_POS_RESULT != grp_vos_CreateFlag(&l_ptCyclonevhFlg,
                                                 (grp_u8 *)"fCycvH")) {
        _TRACE_CTRL_(0x01, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* Creation and activation of Interrupt Process Task */
    if (GRP_VOS_POS_RESULT != grp_vos_CreateTask(&l_ptCbTask,
                                                 (grp_u8 *)"tCycvHCb",
                                                 _grp_cyclonevh_Dumy_Task,
                                                 GRP_CYCLONEVH_CBTSK_STK,
                                                 GRP_CYCLONEVH_CBTSK_PRI,
                                                 GRP_VOS_READY,
                                                 0)) {
        _TRACE_CTRL_(0x01, 0x02, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    _TRACE_CTRL_(0x01, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_CtrTrRun                                                          */
/*                                                                                              */
/* DESCRIPTION: Execute transfer.                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_CtrTrRun(grp_hcdi_tr_request *ptTrReq)
{
grp_s32                         lStatus;

    _TRACE_CTRL_(0x02, 0x00, 0x00);

    /* set initial value */
    ptTrReq->lStatus        = GRP_USBD_TR_NOT_PROCESS;

    switch (ptTrReq->ptEndpoint->ucTxMode) {
    case GRP_USBD_CONTROL:
        lStatus = _grp_cyclonevh_CtrSetupData(ptTrReq);
        break;
    case GRP_USBD_BULK:
        lStatus = _grp_cyclonevh_CtrBulk(ptTrReq, GRP_CYCLONEVH_HCD_1ST_REQ);
        break;
    case GRP_USBD_INTERRUPT:
        lStatus = GRP_HCDI_ERROR;
        break;
    case GRP_USBD_ISOCHRONOUS:
        lStatus = GRP_HCDI_ERROR;
        break;
    default:
        lStatus = GRP_HCDI_ERROR;
        break;
    }

    _TRACE_CTRL_(0x02, 0x00, _F_END_);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrSetupData                                                     */
/*                                                                                              */
/* DESCRIPTION: Execute transfer.                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrSetupData(grp_hcdi_tr_request *ptTrReq)
{
grp_cyclonevh_mng_info           *ptMngInfo = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;
grp_u32                         ulRegData;

    _TRACE_CTRL_(0x03, 0x00, 0x00);

    /* check port enable */
    if ((CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HPRT) & CYCLONEVH_VPRTENA_ENA) != CYCLONEVH_VPRTENA_ENA) {
        ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_GINTSTS);
        _TRACE_CTRL_(0x03, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* set transfer size */
    ulRegData = (CYCLONEVH_VPID_SETUP | 0x00080008);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCTSIZ(GRP_CYCLONEVH_CTRL_SETUP), ulRegData);

    ptMngInfo->ulDMABufSize = 8;

    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCDMA(GRP_CYCLONEVH_CTRL_SETUP), (grp_u32)ptTrReq->pucSetupPtr);

    /* set next information */
    if (ptTrReq->ulBufferLength == 0) {
        _TRACE_CTRL_(0x03, 0x01, 0x00);
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_DATA_0_IN;
    }
    else {
        if (ptTrReq->ulXferinfo == GRP_USBD_TX_IN) {
            _TRACE_CTRL_(0x03, 0x02, 0x00);
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_SETUP_IN;
        }
        else {
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_SETUP_OUT;
        }
    }

    /* reset togle bit */
    CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HCTSIZ(GRP_CYCLONEVH_CTRL_DATAIN),  CYCLONEVH_VPID_DATA1);
    CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HCTSIZ(GRP_CYCLONEVH_CTRL_DATAOUT), CYCLONEVH_VPID_DATA1);

    /* save next PID */
    ptMngInfo->ucDPID       = GRP_CYCLONEVH_PID_DATA1;
    /* save data length */
    ptMngInfo->ulTmpBufLen  = 0;

    /* Unmasked interrupt */
    ulRegData = (CYCLONEVH_B01_DATATGLERRMSK | CYCLONEVH_B01_FRMOVRUNMSK | CYCLONEVH_B01_BBLERRMSK
               | CYCLONEVH_B01_XACTERRMSK    | CYCLONEVH_B01_STALLMSK    | CYCLONEVH_B01_XFERCOMPLMSK);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINTMSK(GRP_CYCLONEVH_CTRL_SETUP), ulRegData);

    /* set characteristics */
    ulRegData = (ptMngInfo->ulHcChar | CYCLONEVH_B01_CHENA);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(GRP_CYCLONEVH_CTRL_SETUP), ulRegData);

    _TRACE_CTRL_(0x03, 0x00, _F_END_);

    return GRP_HCDI_OK;
}
/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrDataIn                                                        */
/*                                                                                              */
/* DESCRIPTION: Execute CONTROL IN transfer.                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrDataIn(grp_hcdi_tr_request *ptTrReq, grp_cyclonevh_mng_info *ptMngInfo)
{
grp_u32                         ulRegData   = 0;
grp_u32                         ulXferLen   = 0;
grp_u32                         ulDataCnt   = 0;

    _TRACE_CTRL_(0x06, 0x00, 0x00);

    /* check port enable */
    if ((CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HPRT) & CYCLONEVH_VPRTENA_ENA) != CYCLONEVH_VPRTENA_ENA) {
        ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_GINTSTS);
        _TRACE_CTRL_(0x06, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* check data size */
    ulXferLen = ptTrReq->ulBufferLength - ptMngInfo->ulTmpBufLen;
    if (ulXferLen > ptTrReq->ptEndpoint->usMaxPacketSize) {
        ulXferLen = ptTrReq->ptEndpoint->usMaxPacketSize;
    }

    /* set transfer size */
    if (ulXferLen) {
        ulDataCnt = (ulXferLen + ptTrReq->ptEndpoint->usMaxPacketSize - 1) / ptTrReq->ptEndpoint->usMaxPacketSize;
        _TRACE_CTRL_(0x06, 0x01, 0x00);
    }
    else {
        ulDataCnt = 1;
    }

    ptMngInfo->ulDMABufSize = ulDataCnt * ptTrReq->ptEndpoint->usMaxPacketSize;

    /* create channel-x transfer size register data */
    ulRegData  = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCTSIZ(GRP_CYCLONEVH_CTRL_DATAIN)) & CYCLONEVH_VPID_DATA1;
    ulRegData |= (ulDataCnt << 19);   /* 19bit shift */
    ulRegData |= ulDataCnt * ptTrReq->ptEndpoint->usMaxPacketSize;
    /* set data */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCTSIZ(GRP_CYCLONEVH_CTRL_DATAIN), ulRegData);

    /* set DMA buffer address */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCDMA(GRP_CYCLONEVH_CTRL_DATAIN), (grp_u32)(ptTrReq->pucBufferPtr + ptMngInfo->ulTmpBufLen));

    /* Unmasked interrupt */
    ulRegData = (CYCLONEVH_B01_DATATGLERRMSK | CYCLONEVH_B01_FRMOVRUNMSK | CYCLONEVH_B01_BBLERRMSK
               | CYCLONEVH_B01_XACTERRMSK    | CYCLONEVH_B01_STALLMSK    | CYCLONEVH_B01_XFERCOMPLMSK);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINTMSK(GRP_CYCLONEVH_CTRL_DATAIN), ulRegData);

    /* set characteristics */
    ulRegData = (ptMngInfo->ulHcChar | CYCLONEVH_VEPDIR_IN);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(GRP_CYCLONEVH_CTRL_DATAIN), (ulRegData | CYCLONEVH_B01_CHENA));

    _TRACE_CTRL_(0x06, 0x00, _F_END_);

    return GRP_HCDI_OK;
}
/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrDataOut                                                       */
/*                                                                                              */
/* DESCRIPTION: Execute CONTROL OUT transfer.                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrDataOut(grp_hcdi_tr_request *ptTrReq, grp_cyclonevh_mng_info *ptMngInfo)
{
grp_u32                         ulRegData   = 0;
grp_u32                         ulXferLen   = 0;
grp_u32                         ulDataCnt   = 0;

    _TRACE_CTRL_(0x07, 0x00, 0x00);

    /* check port enable */
    if ((CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HPRT) & CYCLONEVH_VPRTENA_ENA) != CYCLONEVH_VPRTENA_ENA) {
        ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_GINTSTS);
        _TRACE_CTRL_(0x07, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* check data size */
    ulXferLen = ptTrReq->ulBufferLength - ptMngInfo->ulTmpBufLen;
    if (ulXferLen > ptTrReq->ptEndpoint->usMaxPacketSize) {
        ulXferLen = ptTrReq->ptEndpoint->usMaxPacketSize;
        _TRACE_CTRL_(0x07, 0x01, 0x00);
    }

    /* set transfer size */
    if (ulXferLen) {
        ulDataCnt = (ulXferLen + ptTrReq->ptEndpoint->usMaxPacketSize - 1) / ptTrReq->ptEndpoint->usMaxPacketSize;
        _TRACE_CTRL_(0x07, 0x02, 0x00);
    }
    else {
        ulDataCnt = 1;
    }

    ptMngInfo->ulDMABufSize = ulXferLen;

    /* create channel-x transfer size register data */
    ulRegData  = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCTSIZ(GRP_CYCLONEVH_CTRL_DATAOUT)) & CYCLONEVH_VPID_DATA1;
    ulRegData |= (ulDataCnt << 19);   /* 19bit shift */
    ulRegData |= ulXferLen;
    /* set data */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCTSIZ(GRP_CYCLONEVH_CTRL_DATAOUT), ulRegData);

    /* Unmasked interrupt */
    ulRegData = (CYCLONEVH_B01_DATATGLERRMSK | CYCLONEVH_B01_FRMOVRUNMSK | CYCLONEVH_B01_BBLERRMSK
               | CYCLONEVH_B01_XACTERRMSK    | CYCLONEVH_B01_STALLMSK    | CYCLONEVH_B01_XFERCOMPLMSK);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINTMSK(GRP_CYCLONEVH_CTRL_DATAOUT), ulRegData);

    /* set DMA buffer address */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCDMA(GRP_CYCLONEVH_CTRL_DATAOUT), (grp_u32)(ptTrReq->pucBufferPtr + ptMngInfo->ulTmpBufLen));

    /* save data length */
    ptMngInfo->ulTmpBufLen  += ulXferLen;

    /* set characteristics */
    ulRegData = (ptMngInfo->ulHcChar | CYCLONEVH_VEPDIR_OUT);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(GRP_CYCLONEVH_CTRL_DATAOUT), (ulRegData | CYCLONEVH_VCHENA_READY));

    _TRACE_CTRL_(0x07, 0x00, _F_END_);

    return GRP_HCDI_OK;
}
/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrBulk                                                          */
/*                                                                                              */
/* DESCRIPTION: Execute BULK transfer.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrBulk(grp_hcdi_tr_request *ptTrReq, grp_u8 ucFlag)
{
grp_cyclonevh_mng_info          *ptMngInfo  = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;
grp_u32                         ulRegData   = 0;
grp_u32                         ulXferLen   = 0;
grp_u32                         ulDataCnt   = 0;
grp_si                          i;

    _TRACE_CTRL_(0x08, 0x00, 0x00);

    /* check channel enable/disable */
    for (i = 0; i < GRP_CYCLONEVH_MAX_CHK_CH_ENA; i++) {
        ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCCHAR(ptMngInfo->ucChIdx));
        if (!(ulRegData & (CYCLONEVH_B01_CHDIS | CYCLONEVH_B01_CHENA))) {
            break;
        }
    }
    if (i >= GRP_CYCLONEVH_MAX_CHK_CH_ENA) {
        /* count over error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }
    ulRegData = 0;

    /* check port enable */
    if ((CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HPRT) & CYCLONEVH_VPRTENA_ENA) != CYCLONEVH_VPRTENA_ENA) {
        ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_GINTSTS);
        _TRACE_CTRL_(0x08, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    if (ucFlag == GRP_CYCLONEVH_HCD_1ST_REQ) {
        /* save data length */
        ptMngInfo->ulTmpBufLen  = 0;
    }

    /* check data size */
    ulXferLen = ptTrReq->ulBufferLength - ptMngInfo->ulTmpBufLen;
    if (ulXferLen > ptTrReq->ptEndpoint->usMaxPacketSize) {
        ulXferLen = ptTrReq->ptEndpoint->usMaxPacketSize;
        _TRACE_CTRL_(0x08, 0x01, 0x00);
    }

    /* set transfer size */
    if (ulXferLen) {
        ulDataCnt = (ulXferLen + ptTrReq->ptEndpoint->usMaxPacketSize - 1) / ptTrReq->ptEndpoint->usMaxPacketSize;
        _TRACE_CTRL_(0x08, 0x02, 0x00);
    }
    else {
        ulDataCnt = 1;
    }

    /* create channel-x transfer size register data */
    if (ptMngInfo->ucDPID == GRP_CYCLONEVH_PID_DATA1) {
        ulRegData = CYCLONEVH_VPID_DATA1;
        _TRACE_CTRL_(0x08, 0x03, 0x00);
    }
    ulRegData |= (ulDataCnt << 19);   /* 19bit shift */
    if (ptTrReq->ptEndpoint->ucTxDir == GRP_USBD_TX_OUT) {
        ptMngInfo->ulDMABufSize = ulXferLen;
        _TRACE_CTRL_(0x08, 0x04, 0x00);
    }
    else {
        ptMngInfo->ulDMABufSize = ulDataCnt * ptTrReq->ptEndpoint->usMaxPacketSize;
    }
    ulRegData |= ptMngInfo->ulDMABufSize;

    /* set data */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCTSIZ(ptMngInfo->ucChIdx), ulRegData);

    /* set DMA buffer address */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCDMA(ptMngInfo->ucChIdx), (grp_u32)(ptTrReq->pucBufferPtr + ptMngInfo->ulTmpBufLen));

    if (ptTrReq->ptEndpoint->ucTxDir == GRP_USBD_TX_OUT) {
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_DATA_OUT;
        /* save data length */
        ptMngInfo->ulTmpBufLen  += ulXferLen;
        _TRACE_CTRL_(0x08, 0x05, 0x00);
    }
    else {
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_DATA_IN;
    }
    /* Unmasked interrupt */
    ulRegData = (CYCLONEVH_B01_DATATGLERRMSK | CYCLONEVH_B01_FRMOVRUNMSK | CYCLONEVH_B01_BBLERRMSK
               | CYCLONEVH_B01_XACTERRMSK    | CYCLONEVH_B01_STALLMSK    | CYCLONEVH_B01_XFERCOMPLMSK);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINTMSK(ptMngInfo->ucChIdx), ulRegData);

    /* set characteristics */
    ulRegData = ptMngInfo->ulHcChar;
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(ptMngInfo->ucChIdx), (ulRegData | CYCLONEVH_VCHENA_READY));

    _TRACE_CTRL_(0x08, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrRdDone                                                        */
/*                                                                                              */
/* DESCRIPTION: Read data done.                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTrReq                         Transfer Request Information                    */
/*              ptMngInfo                       Pipe information                                */
/*              usPktData                       Packet data                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrRdDone(grp_hcdi_tr_request *ptTrReq, grp_cyclonevh_mng_info *ptMngInfo, grp_u16 usPktData)
{
grp_u32                         ulXferLen   = 0;
grp_u32                         ulReqLen    = 0;

    _TRACE_CTRL_(0x09, 0x00, 0x00);

    ulXferLen = usPktData;

    /* check data size */
    ulReqLen = ptTrReq->ulBufferLength - ptMngInfo->ulTmpBufLen;
    if (ulReqLen > ptTrReq->ptEndpoint->usMaxPacketSize) {
        ulReqLen = ptTrReq->ptEndpoint->usMaxPacketSize;
        _TRACE_CTRL_(0x09, 0x01, 0x00);
    }

    /* save data length */
    ptMngInfo->ulTmpBufLen  += ulXferLen;
    ptTrReq->ulActualLength += ulXferLen;

    /* data end? */
    if (ptMngInfo->ulTmpBufLen == ptTrReq->ulBufferLength) {
        /* set end mark */
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_END;
        _TRACE_CTRL_(0x09, 0x02, 0x00);
    }
    if (ulXferLen != ulReqLen) {                                                /* short packet */
        /* set end mark */
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_END;
        _TRACE_CTRL_(0x09, 0x03, 0x00);
        if (ptTrReq->iShortXferOK != GRP_USB_TRUE) {
            /* underrun error */
            ptTrReq->lStatus = GRP_USBD_TR_DATA_UNDERRUN;
            _TRACE_CTRL_(0x09, 0x04, 0x00);
        }
    }

    _TRACE_CTRL_(0x09, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_Interrupt                                                         */
/*                                                                                              */
/* DESCRIPTION: Host controller interrupt process rourtine.                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_cyclonevh_Interrupt(void)
{
grp_u32                         ulRegData;
grp_s32                         lStatus;
grp_u16                         usIntrFactor;
grp_u16                         usPktData;
grp_u16                         usPipeBitIndex;
grp_si                          i;
grp_cyclonevh_mng_info         *ptMngInfo;
grp_hcdi_tr_request             *ptTrReq;

    _TRACE_CTRL_(0x0A, 0x00, 0x00);

    /* read OTG_HS_HAINT */
    ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HAINT);
    usPipeBitIndex  = (grp_u16)ulRegData;

    for (i=0; i<GRP_CYCLONEVH_MAX_PIPE_NUM; i++) {
        if (usPipeBitIndex == 0) {
            break;                                                      /* exit FOR loop    */
        }
        if (usPipeBitIndex & CYCLONEVH_VHAINT_CH(0)) {
            /* read OTG_HS_HCINTx */
            ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCINT(i));
            /* clear interrupt factor */
            CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCINT(i), ulRegData);
            /* set data */
            usIntrFactor = (grp_u16)ulRegData;

            ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCINTMSK(i)) & CYCLONEVH_B01_AHBERRMSK;
            usIntrFactor |= (grp_u16)ulRegData;

            /* get TrRequest */
            if ( i <= GRP_CYCLONEVH_CTRL_PIPE ) {
                lStatus = grp_cyclonevh_MngGetPipeFromNum(GRP_CYCLONEVH_CTRL_SETUP, &ptTrReq);
                _TRACE_CTRL_(0x0A, 0x01, 0x00);
            }
            else {
                lStatus = grp_cyclonevh_MngGetPipeFromNum(i, &ptTrReq);
                _TRACE_CTRL_(0x0A, 0x02, 0x00);
            }
            if ((lStatus == GRP_HCDI_OK) && (ptTrReq != GRP_USB_NULL)) {
                /* get pipe information */
                ptMngInfo = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;
                _TRACE_CTRL_(0x0A, 0x03, 0x00);
                if ( ptMngInfo != GRP_USB_NULL) {
                    usPktData = (grp_u16)(ptMngInfo->ulDMABufSize - (CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCTSIZ(i)) & CYCLONEVH_B19_XFERSIZE));
                    /* notify to the task */
                    grp_cyclonevh_CtrCmpPipe(usIntrFactor, usPktData, i);
                }
            }
        }

        usPipeBitIndex = usPipeBitIndex >> 1;
    }

    _TRACE_CTRL_(0x0A, 0x00, _F_END_);

    return;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_CtrCmpPipe                                                        */
/*                                                                                              */
/* DESCRIPTION: Nortify interrupt information to the callback task                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usIntrFactor                    Interrupt factor                                */
/*              ucPipeNum                       Pipe number                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_CtrCmpPipe(grp_u16 usIntrFactor, grp_u16 usPktData, grp_u8 ucPipeNum)
{
    _TRACE_CTRL_(0x0B, 0x00, 0x00);

    /* set information */
    l_atEvnt[l_ucEvntWrPtr].usIntrFactor = usIntrFactor;
    l_atEvnt[l_ucEvntWrPtr].ucPipeNum    = ucPipeNum;
    l_atEvnt[l_ucEvntWrPtr].usPktData    = usPktData;

    /* renew the write pointer */
    l_ucEvntWrPtr = (l_ucEvntWrPtr + 1) % GRP_CYCLONEVH_MAX_TR_NUM;

    /* check read pointer */
    if (l_ucEvntWrPtr == l_ucEvntRdPtr) {
        /* over flow the notification list */
        _TRACE_CTRL_(0x0B, 0x01, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    if (GRP_VOS_POS_RESULT  != grp_vos_SetFlag(l_ptCyclonevhFlg, GRP_CYCLONEVH_HCD_NTC)) {
        /* illegal error */
        _TRACE_CTRL_(0x0B, 0x02, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    _TRACE_CTRL_(0x0B, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrGetCmpPipe                                                    */
/*                                                                                              */
/* DESCRIPTION: Get information from interupt handler.                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : pusIntrFactor                   Interrupt factor                                */
/*              pucPipeNum                      Pipe number                                     */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrGetCmpPipe(grp_u16 *pusIntrFactor, grp_u16 *pusPktData, grp_u8 *pucPipeNum)
{
grp_s32                         lStatus = GRP_HCDI_ERROR;

    _TRACE_CTRL_(0x0C, 0x00, 0x00);

    if (l_ucEvntRdPtr != l_ucEvntWrPtr) {
        /* exist event */
        *pusIntrFactor = l_atEvnt[l_ucEvntRdPtr].usIntrFactor;
        *pucPipeNum    = l_atEvnt[l_ucEvntRdPtr].ucPipeNum;
        *pusPktData    = l_atEvnt[l_ucEvntRdPtr].usPktData;
        /* clear */
        l_atEvnt[l_ucEvntRdPtr].usIntrFactor = 0;
        l_atEvnt[l_ucEvntRdPtr].ucPipeNum    = 0;
        l_atEvnt[l_ucEvntRdPtr].usPktData    = 0;
        /* renew the read pointer */
        l_ucEvntRdPtr = (l_ucEvntRdPtr + 1) % GRP_CYCLONEVH_MAX_TR_NUM;
        /* success */
        lStatus = GRP_HCDI_OK;
    }

    _TRACE_CTRL_(0x0C, 0x00, _F_END_);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_Dumy_Task                                                        */
/*                                                                                              */
/* DESCRIPTION: Dumy function                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : dumy                            dumy parameter                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_ERROR                  Error (System error)                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_cyclonevh_Dumy_Task(grp_u32 dumy)
{
    _TRACE_CTRL_(0x0D, 0x00, 0x00);

    _grp_cyclonevh_CtrCb_Task(dumy);

    _TRACE_CTRL_(0x0D, 0x00, _F_END_);
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrCb_Task                                                       */
/*                                                                                              */
/* DESCRIPTION: Interrupt process task                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : dumy                            dumy parameter                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_ERROR                  Error (System error)                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrCb_Task(grp_u32 dumy)
{
grp_u32                          ulEvent;
grp_u16                          usIntrFactor;
grp_u16                          usPktData;
grp_u8                           ucPipeNum;

    _TRACE_CTRL_(0x10, 0x00, 0x00);

    for (;;) {
        /* wait event */
        if (GRP_VOS_POS_RESULT == grp_vos_WaitFlag(l_ptCyclonevhFlg, GRP_CYCLONEVH_HCD_NTC, &ulEvent, GRP_VOS_INFINITE)) {
            /* clear */
            if (GRP_VOS_POS_RESULT == grp_vos_ClearFlag(l_ptCyclonevhFlg, GRP_CYCLONEVH_HCD_NTC_CLR)) {
                for (;;) {
                    _TRACE_CTRL_(0x10, 0x01, 0x00);

                    if (GRP_HCDI_OK == _grp_cyclonevh_CtrGetCmpPipe(&usIntrFactor, &usPktData, &ucPipeNum)) {

                        GRP_CYCLONEVH_LOCK();

                        /* check pipe */
                        if (ucPipeNum & GRP_CYCLONEVH_CANCEL_FLAG) {
                            _TRACE_CTRL_(0x10, 0x02, 0x00);
                            _grp_cyclonevh_CtrReqCancel((ucPipeNum & ~GRP_CYCLONEVH_CANCEL_FLAG));
                        }
                        else if (ucPipeNum <= GRP_CYCLONEVH_CTRL_PIPE) {
                            _TRACE_CTRL_(0x10, 0x03, 0x00);
                            _grp_cyclonevh_CtrCmpControl(usIntrFactor, usPktData, ucPipeNum);
                        }
                        else {
                            _TRACE_CTRL_(0x10, 0x04, 0x00);
                            _grp_cyclonevh_CtrCmpOtherXfer(usIntrFactor, usPktData, ucPipeNum);
                        }

                        GRP_CYCLONEVH_UNLOCK();
                    }
                    else {
                        break;                                  /* exit FOR loop : not error    */
                    }
                }
            }
        }
    }

    _TRACE_CTRL_(0x10, 0x00, _F_END_);

    return GRP_HCDI_ERROR;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrReqCancel                                                     */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucPipeNum                       Pipe number                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrReqCancel(grp_u8 ucPipeNum)
{
    _TRACE_CTRL_(0x11, 0x00, 0x00);

    if (ucPipeNum <= GRP_CYCLONEVH_CTRL_PIPE) {
        _TRACE_CTRL_(0x11, 0x01, 0x00);
        _grp_cyclonevh_CtrCmpControl(0, 0, ucPipeNum);
                                /*    ^^^^^ dumy data   */
    }
    else {
        _TRACE_CTRL_(0x11, 0x02, 0x00);
        _grp_cyclonevh_CtrCmpOtherXfer(0, 0, ucPipeNum);
                                /*      ^^^^^ dumy data */
    }

    _TRACE_CTRL_(0x11, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrDisableChannel                                                */
/*                                                                                              */
/* DESCRIPTION: Channel deactivation process                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucChIdx                         Channel number                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrDisableChannel(grp_u8 ucChIdx)
{
grp_u32                         ulRegData   = 0;

    ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCCHAR(ucChIdx));
    ulRegData |= (CYCLONEVH_B01_CHENA | CYCLONEVH_B01_CHDIS);
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(ucChIdx), ulRegData);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrCmpControl                                                    */
/*                                                                                              */
/* DESCRIPTION: Complete processing of control transfer                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usIntrFactor                    Interrupt factor                                */
/*              ucPipeNum                       Pipe number                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrCmpControl(grp_u16 usIntrFactor, grp_u16 usPktData, grp_u8 ucPipeNum)
{
grp_cyclonevh_mng_info           *ptMngInfo;
grp_hcdi_tr_request             *ptTrReq;
grp_s32                         lStatus;
grp_u32                         ulRegData   = 0;

    _TRACE_CTRL_(0x12, 0x00, 0x00);

    /* get TrRequest */
    lStatus = grp_cyclonevh_MngGetPipeFromNum(GRP_CYCLONEVH_CTRL_SETUP, &ptTrReq);
    if ((lStatus != GRP_HCDI_OK) || (ptTrReq == GRP_USB_NULL)) {
        _TRACE_CTRL_(0x12, 0x01, _F_END_);
        /* no exist request */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }
    /* get pipe information */
    ptMngInfo = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;

    /*--- CANCEL? ---*/
    if (ptTrReq->lStatus == GRP_USBD_TR_CANCEL) {
        _TRACE_CTRL_(0x12, 0x02, _F_END_);
        /* call back : success */
        _grp_cyclonevh_CtrDisableChannel(ucPipeNum);
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /*--- ERROR? ---*/
    lStatus = _grp_cyclonevh_CtrCmpErrChk(usIntrFactor);
    if (lStatus == GRP_USBD_TR_NOT_PROCESS) {                           /* NAK,NYET */
        _TRACE_CTRL_(0x12, 0x03, 0x00);
        /* NAK retry */
        if (ucPipeNum == GRP_CYCLONEVH_CTRL_DATAOUT) {                  /* control DATA OUT */
            _TRACE_CTRL_(0x12, 0x04, 0x00);
            _grp_cyclonevh_CtrDisableChannel(ucPipeNum);
            /* rewind buffer pointer */
            if (ptMngInfo->ucStatus != GRP_CYCLONEVH_PS_END) {          /* not END */
                _TRACE_CTRL_(0x12, 0x05, 0x00);
                ptMngInfo->ulTmpBufLen = ptTrReq->ulActualLength;
            }
            /* retry out transfer */
            if (_grp_cyclonevh_CtrDataOut(ptTrReq, ptMngInfo) != GRP_HCDI_OK) {
                _TRACE_CTRL_(0x12, 0x06, 0x00);                         /* error */
                ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_END;
                /* set status */
                ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
                /* call back : error */
                grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
            }
        }
        else {                                                          /* control SETUP/DATA IN */
            _TRACE_CTRL_(0x12, 0x07, 0x00);
            ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCCHAR(ucPipeNum));
            ulRegData &= (~CYCLONEVH_B01_CHDIS);
            CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(ucPipeNum), (ulRegData | CYCLONEVH_VCHENA_READY));
        }
        _TRACE_CTRL_(0x12, 0x03, _F_END_);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }
    else if (lStatus != GRP_USBD_TR_NO_FAIL) {
        _TRACE_CTRL_(0x12, 0x17, 0x00);
        /* call back : error */
        ptTrReq->lStatus = lStatus;
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        _TRACE_CTRL_(0x12, 0x02, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }
    else if ((ucPipeNum == GRP_CYCLONEVH_CTRL_DATAOUT)                  /* control DATA OUT */
          && (ptMngInfo->ucStatus != GRP_CYCLONEVH_PS_END)) {           /* data stage DATA OUT(not last) */
        /* renew actual length */
        ptTrReq->ulActualLength = ptMngInfo->ulTmpBufLen;
        _TRACE_CTRL_(0x12, 0x08, 0x00);
    }

    /* read data done */
    if (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_DATA_IN) {              /* control DATA IN */
        _TRACE_CTRL_(0x12, 0x09, 0x00);
        /* read data done */
        /* When IN transfer is completed, "GRP_CYCLONEVH_PS_END" is set in "ptMngInfo->ucStatus". */
        _grp_cyclonevh_CtrRdDone(ptTrReq, ptMngInfo, usPktData);        /* read data */
        if (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_END) {              /* last IN data */
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_DATA_0_OUT;          /* next STATUS stage is 0 byte OUT */
            _TRACE_CTRL_(0x12, 0x0A, 0x00);
        }
    }

    /* check status */
    if (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_END) {                  /* end */
        _TRACE_CTRL_(0x12, 0x18, 0x00);
        /* set status */
        if (ptTrReq->lStatus != GRP_USBD_TR_DATA_UNDERRUN) {            /* success */
            ptTrReq->lStatus = GRP_USBD_TR_NO_FAIL;
            _TRACE_CTRL_(0x12, 0x0B, 0x00);
        }
        /* call back : success */
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);

        _TRACE_CTRL_(0x12, 0x04, _F_END_);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /* check status */
    if (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_END_0_IN) {             /* DATA IN no data */
        _TRACE_CTRL_(0x12, 0x19, 0x00);
        /* set status */
        if (ptTrReq->lStatus != GRP_USBD_TR_DATA_UNDERRUN) {            /* success */
            ptTrReq->lStatus = GRP_USBD_TR_NO_FAIL;
            _TRACE_CTRL_(0x12, 0x0C, 0x00);
        }
        /* read data done */
        /* When IN transfer is completed, "GRP_CYCLONEVH_PS_END" is set in "ptMngInfo->ucStatus". */
        /* Since there is no data, "GRP_CYCLONEVH_PS_END" is set to "ptMngInfo->ucStatus". */
        _grp_cyclonevh_CtrRdDone(ptTrReq, ptMngInfo, usPktData);        /* read data */
        _TRACE_CTRL_(0x12, 0x1A, 0x00);
        /* call back : success */
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);

        _TRACE_CTRL_(0x12, 0x05, _F_END_);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /* check status */
    switch (ptMngInfo->ucStatus) {
    case GRP_CYCLONEVH_PS_SETUP_OUT:                                    /* SETUP with out data */
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_DATA_OUT;                /* NO BREAK */
        _TRACE_CTRL_(0x12, 0x0D, 0x00);
    case GRP_CYCLONEVH_PS_DATA_OUT:                                     /* DATA OUT */
        if (_grp_cyclonevh_CtrDataOut(ptTrReq, ptMngInfo) != GRP_HCDI_OK) {
            _TRACE_CTRL_(0x12, 0x0E, 0x00);
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_ERROR;
            /* set status */
            ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
            /* call back : error */
            grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        }
        else if (ptMngInfo->ulTmpBufLen == ptTrReq->ulBufferLength) {   /* DATA OUT(last) */
            _TRACE_CTRL_(0x12, 0x0F, 0x00);
            /* change next status */
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_DATA_0_IN;           /* next STATUS stage is 0 byte IN */
        }
        /* If there is OUT data, GRP_CYCLONEVH_PS_DATA_OUT remains. */
        break;

    case GRP_CYCLONEVH_PS_SETUP_IN:                                     /* SETUP with in data */
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_DATA_IN;                 /* NO BREAK */
        _TRACE_CTRL_(0x12, 0x10, 0x00);
    case GRP_CYCLONEVH_PS_DATA_IN:                                      /* DATA IN */
        _TRACE_CTRL_(0x12, 0x11, 0x00);
        /* request next data */
        if (_grp_cyclonevh_CtrDataIn(ptTrReq, ptMngInfo) != GRP_HCDI_OK) {
            _TRACE_CTRL_(0x12, 0x12, 0x00);
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_ERROR;
            /* set status */
            ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
            /* call back : error */
            grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        }
        /* not status changed */
        break;

    case GRP_CYCLONEVH_PS_DATA_0_OUT:                                   /* DATA OUT with no data */
        _TRACE_CTRL_(0x12, 0x13, 0x00);
        /* There is no data by setting the same value. */
        ptMngInfo->ulTmpBufLen = ptTrReq->ulBufferLength;
        if (_grp_cyclonevh_CtrDataOut(ptTrReq, ptMngInfo) != GRP_HCDI_OK) {
            _TRACE_CTRL_(0x12, 0x1B, 0x00);
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_ERROR;
            /* set status */
            ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
            /* call back : error */
            grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        }
        else {                                                          /* success */
            _TRACE_CTRL_(0x12, 0x14, 0x00);
            /* got to the next stage */
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_END;
        }
        break;

    case GRP_CYCLONEVH_PS_DATA_0_IN:                                    /* SETUP with no data */
        _TRACE_CTRL_(0x12, 0x15, 0x00);
        if (_grp_cyclonevh_CtrDataIn(ptTrReq, ptMngInfo) != GRP_HCDI_OK) {
            _TRACE_CTRL_(0x12, 0x1C, 0x00);
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_ERROR;
            /* set status */
            ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
            /* call back : error */
            grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        }
        else {                                                          /* success */
            _TRACE_CTRL_(0x12, 0x16, 0x00);
            /* got to the next stage */
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_END_0_IN;
        }
        break;

    default:
        /* error */
        _TRACE_CTRL_(0x12, 0x06, _F_END_);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    _TRACE_CTRL_(0x12, 0x00, _F_END_);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrCmpOtherXfer                                                  */
/*                                                                                              */
/* DESCRIPTION: Complete processing of bulk or interrupt or isochronous transfer                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usIntrFactor                    Interrupt factor                                */
/*              ucPipeNum                       Pipe number                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrCmpOtherXfer(grp_u16 usIntrFactor, grp_u16 usPktData, grp_u8 ucPipeNum)
{
grp_cyclonevh_mng_info          *ptMngInfo;
grp_hcdi_tr_request             *ptTrReq;
grp_s32                         lStatus;
grp_u32                         ulRegData;

    _TRACE_CTRL_(0x13, 0x00, 0x00);

    /* get TrRequest */
    lStatus = grp_cyclonevh_MngGetPipeFromNum(ucPipeNum, &ptTrReq);
    if ((lStatus != GRP_HCDI_OK) || (ptTrReq == GRP_USB_NULL)) {
        _TRACE_CTRL_(0x13, 0x01, _F_END_);
        /* no exist request */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }
    /* get pipe information */
    ptMngInfo = (grp_cyclonevh_mng_info *)ptTrReq->ptEndpoint->pvHcdworkPtr;

    /* check PID information */
    ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCTSIZ(ptMngInfo->ucChIdx));
    if (ulRegData & CYCLONEVH_VPID_DATA1) {
        _TRACE_CTRL_(0x13, 0x01, 0x00);
        ptMngInfo->ucDPID = GRP_CYCLONEVH_PID_DATA1;
    }
    else {
        _TRACE_CTRL_(0x13, 0x02, 0x00);
        ptMngInfo->ucDPID = GRP_CYCLONEVH_PID_DATA0;
    }

    /*--- CANCEL? ---*/
    if (ptTrReq->lStatus == GRP_USBD_TR_CANCEL) {
        _TRACE_CTRL_(0x13, 0x02, _F_END_);
        /* call back : success */
        _grp_cyclonevh_CtrDisableChannel(ucPipeNum);
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /*--- ERROR? ---*/
    lStatus = _grp_cyclonevh_CtrCmpErrChk(usIntrFactor);
    if (lStatus == GRP_USBD_TR_NOT_PROCESS) {                           /* NAK,NYET */
        _TRACE_CTRL_(0x13, 0x03, 0x00);
        /* NAK retry */
        if (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_DATA_OUT) {         /* BULK OUT retry */
            _TRACE_CTRL_(0x13, 0x04, 0x00);
            _grp_cyclonevh_CtrDisableChannel(ucPipeNum);                /* channel disable */
            /* rewind buffer pointer */
            ptMngInfo->ulTmpBufLen = ptTrReq->ulActualLength;
            /* retry out transfer */
            if (_grp_cyclonevh_CtrBulk(ptTrReq, GRP_CYCLONEVH_HCD_CNT_REQ) != GRP_HCDI_OK) {
                _TRACE_CTRL_(0x13, 0x05, 0x00);                         /* retry issue error */
                ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_ERROR;
                /* set status */
                ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
                /* call back : error */
                grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
            }
        }
        else {                                                          /* BULK IN retry */
            _TRACE_CTRL_(0x13, 0x06, 0x00);
            ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HCCHAR(ucPipeNum));
            ulRegData &= (~CYCLONEVH_B01_CHDIS);
            CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCCHAR(ucPipeNum), (ulRegData | CYCLONEVH_VCHENA_READY));
        }
        _TRACE_CTRL_(0x13, 0x03, _F_END_);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }
    else if (lStatus != GRP_USBD_TR_NO_FAIL) {                          /* other error */
        _TRACE_CTRL_(0x13, 0x09, 0x00);
        /* clear toggle information */
        ptMngInfo->ucDPID = GRP_CYCLONEVH_PID_DATA0;
        /* call back : error */
        ptTrReq->lStatus = lStatus;
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        _TRACE_CTRL_(0x13, 0x04, _F_END_);
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    if (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_DATA_OUT) {             /* BULK OUT success */
        _TRACE_CTRL_(0x13, 0x07, 0x00);
        /* renew actual length */
        ptTrReq->ulActualLength = ptMngInfo->ulTmpBufLen;
        if (ptTrReq->ulActualLength == ptTrReq->ulBufferLength) {
            ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_END;                 /* goto completion processing */
        }
    }
    else {                                                              /* BULK IN success */
        _TRACE_CTRL_(0x13, 0x0A, 0x00);
        /* receive data done */
        /* When IN transfer is completed, "GRP_CYCLONEVH_PS_END" is set in "ptMngInfo->ucStatus". */
        _grp_cyclonevh_CtrRdDone(ptTrReq, ptMngInfo, usPktData);        /* read data */
    }

    /* check status */
    if (ptMngInfo->ucStatus == GRP_CYCLONEVH_PS_END) {                  /* completion processing */
        _TRACE_CTRL_(0x13, 0x0B, 0x00);
        /* set status */
        if (ptTrReq->lStatus != GRP_USBD_TR_DATA_UNDERRUN) {            /* success */
            ptTrReq->lStatus = GRP_USBD_TR_NO_FAIL;
            _TRACE_CTRL_(0x13, 0x0C, 0x00);
        }
        /* call back : success */
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
        _TRACE_CTRL_(0x13, 0x05, _F_END_);
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /* continue request */
    /* The next request issuance process when the IN transfer has not ended. */
    if (_grp_cyclonevh_CtrBulk(ptTrReq, GRP_CYCLONEVH_HCD_CNT_REQ) != GRP_HCDI_OK) {
        _TRACE_CTRL_(0x13, 0x08, 0x00);
        ptMngInfo->ucStatus = GRP_CYCLONEVH_PS_ERROR;
        if (ptTrReq->lStatus != GRP_USBD_TR_DATA_UNDERRUN) {            /* success */
            /* set status */
            ptTrReq->lStatus = GRP_USBD_TR_TX_FAIL;
            _TRACE_CTRL_(0x13, 0x0D, 0x00);
        }
        /* call back : error */
        grp_cyclonevh_CB_Process(ptTrReq, ucPipeNum);
    }

    _TRACE_CTRL_(0x13, 0x00, _F_END_);

    return GRP_HCDI_OK;
}


/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_CtrCmpErrChk                                                     */
/*                                                                                              */
/* DESCRIPTION: check error status                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usIntrFactor                    Interrupt factor                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_TR_NO_FAIL             TR success                                      */
/*              GRP_USBD_TR_TIMEOUT             TR timeout                                      */
/*              GRP_USBD_TR_DATA_OVERRUN        TR receive data overrun                         */
/*              GRP_USBD_TR_STALL               STALL end                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_CtrCmpErrChk(grp_u16 usIntrFactor)
{
grp_s32                             lStatus = GRP_USBD_TR_NO_FAIL;

    _TRACE_CTRL_(0x14, 0x00, 0x00);

    if (usIntrFactor & CYCLONEVH_B01_DATATGLERR) {
        lStatus = GRP_USBD_TR_HC_DATATOGGLE_ERROR;
    }
    else if (usIntrFactor & CYCLONEVH_B01_FRMOVRUN) {
        lStatus = GRP_USBD_OTHER_FAIL;
    }
    else if (usIntrFactor & CYCLONEVH_B01_BBLERR) {
        lStatus = GRP_USBD_TR_BABBLE_DETECTED;
    }
    else if (usIntrFactor & CYCLONEVH_B01_XACTERR) {
        lStatus = GRP_USBD_TR_TX_FAIL;
    }
    else if (usIntrFactor & CYCLONEVH_B01_STALL) {
        lStatus = GRP_USBD_TR_STALL;
    }
    else if (usIntrFactor & CYCLONEVH_B01_AHBERR) {
        lStatus = GRP_USBD_OTHER_FAIL;
    }
    else if (usIntrFactor & CYCLONEVH_B01_NAK) {
        lStatus = GRP_USBD_TR_NOT_PROCESS;
    }
    else if (usIntrFactor & CYCLONEVH_B01_NYET) {
        lStatus = GRP_USBD_TR_NOT_PROCESS;
    }

    _TRACE_CTRL_(0x14, 0x00, _F_END_);

    return lStatus;
}
