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
/*      grp_cyclonevh_rhb.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      CYCLONEV Host Controller Driver RHUB module                                             */
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
DLOCAL grp_u32                          l_ulEnableFlag = GRP_USB_FALSE;

/* OS resource */
DLOCAL grp_vos_t_task                   *l_ptCyclonevhTask;
DLOCAL grp_vos_t_queue                  *l_ptCmpQueue;
#define CYCLONEVH_RH_CMP_EVENT         (1)
#define CYCLONEVH_RH_CMP_WAIT_TIME     (5000)      /* 5SEC */

/* Device descriptor information */
DLOCAL grp_usbdi_dev_desc               *l_ptDevDesc;

/* Structure of standard request */
DLOCAL grp_usbdi_st_device_request      l_tStDevReq;

/* port status */
DLOCAL grp_u8                           l_ucPortStatus;
DLOCAL grp_u16                          l_usDevId;
#define CYCLONEVH_RH_DISC              ((grp_u8)0x00)
#define CYCLONEVH_RH_CONN              ((grp_u8)0x01)
#define CYCLONEVH_RH_SUSPEND           ((grp_u8)0x10)

#define CYCLONEVH_RH_WAIT_TIME         (100)

#define CYCLONEVH_RH_RETRY_COUNT       (3)

/*** INTERNAL FUNCTION PROTOTYPES ***************************************************************/
LOCAL void    _grp_cyclonevh_RhubTask(grp_u32 dumy);
LOCAL grp_s32 _grp_cyclonevh_RhubChkFactor(void);
LOCAL grp_s32 _grp_cyclonevh_RhubConnect(grp_u8 ucPortNum);
LOCAL grp_s32 _grp_cyclonevh_RhubPortReset(grp_u8 ucPortNum);
LOCAL grp_s32 _grp_cyclonevh_RhubRequestCmp(grp_usbdi_st_device_request *ptStDevReq);
LOCAL grp_s32 _grp_cyclonevh_RhubDisconnect(grp_u8 ucPortNum);


/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_RhEnable                                                          */
/*                                                                                              */
/* DESCRIPTION: Set permission flag                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulFlag                          GRP_USB_TRUE                                    */
/*                                               The processing of the RHUB task is permitted.  */
/*                                              GRP_USB_FALSE                                   */
/*                                               The processing of the RHUB task is not         */
/*                                               permitted.                                     */
/* OUTPUT     : None                                                                            */
/*                                                                                              */
/* RESULTS    : None                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_cyclonevh_RhEnable(grp_u32 ulFlag)
{
    l_ulEnableFlag = ulFlag;

    _TRACE_RHB_(0x01, 0x00, END_FUNC);
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_RhubInit                                                          */
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
grp_s32 grp_cyclonevh_RhubInit(void)
{
    _TRACE_RHB_(0x02, 0x00, 0x00);

    /* initialize parameters */
    l_ucPortStatus = CYCLONEVH_RH_DISC;

    /* create cmem area */
    if (GRP_CMEM_OK != grp_cmem_BlkGet(GRP_CMEM_ID_CYCLNVH_DEV_DESC, (void **)&l_ptDevDesc)) {
        _TRACE_RHB_(0x02, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* Create event flag */
    if (GRP_VOS_POS_RESULT != grp_vos_CreateQueue(&l_ptCmpQueue,
                                                  (grp_u8 *)"qRhCmp",
                                                  4,
                                                  10)) {
        _TRACE_RHB_(0x02, 0x02, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* Create root hub task */
    if (GRP_VOS_POS_RESULT != grp_vos_CreateTask(&l_ptCyclonevhTask,
                                                 (grp_u8 *)"tCycvH",
                                                 _grp_cyclonevh_RhubTask,
                                                 GRP_CYCLONEVH_RHTSK_STK,
                                                 GRP_CYCLONEVH_RHTSK_PRI,
                                                 GRP_VOS_READY,
                                                 0)) {
        _TRACE_RHB_(0x02, 0x03, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    _TRACE_RHB_(0x02, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_RhubTask                                                         */
/*                                                                                              */
/* DESCRIPTION: Processing to changing the port.                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : dumy                            Dumy parameter                                  */
/* OUTPUT     : None                                                                            */
/*                                                                                              */
/* RESULTS    : None                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_cyclonevh_RhubTask(grp_u32 dumy)
{
    _TRACE_RHB_(0x03, 0x00, 0x00);

    for (;;) {
        grp_vos_DelayTask(GRP_CYCLONEVH_RH_CHECK_TIME);

        if (l_ulEnableFlag == GRP_USB_TRUE) {
            /* check interrupt factor */
            _grp_cyclonevh_RhubChkFactor();
        }
    }

    _TRACE_RHB_(0x03, 0x00, END_FUNC);

    return;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_RhubChkFactor                                                    */
/*                                                                                              */
/* DESCRIPTION: Check interrupt factors.                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : None                                                                            */
/* OUTPUT     : None                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_RhubChkFactor(void)
{
grp_s32                         ulData;

    _TRACE_RHB_(0x04, 0x00, 0x00);

    ulData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HPRT);
    /* clear interrupt factor */
    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HPRT,
                   (ulData & (CYCLONEVH_B04_PRTTSTCTL | CYCLONEVH_B01_PRTPWR | CYCLONEVH_B01_PRTRST
                            | CYCLONEVH_B01_PRTSUSP   | CYCLONEVH_B01_PRTRES | CYCLONEVH_B01_PRTOVRCURRCHNG
                            | CYCLONEVH_B01_PRTENCHNG | CYCLONEVH_B01_PRTCONNDET)));

    if ((ulData & (CYCLONEVH_B01_PRTOVRCURRCHNG | CYCLONEVH_B01_PRTOVRCURRACT)) == CYCLONEVH_B01_PRTOVRCURRCHNG) {
        /* port power on */
        CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HPRT, CYCLONEVH_B01_PRTPWR);
    }

    if ((ulData & (CYCLONEVH_B01_PRTCONNDET | CYCLONEVH_B01_PRTCONNSTS)) == (CYCLONEVH_B01_PRTCONNDET | CYCLONEVH_B01_PRTCONNSTS)) {
        _TRACE_RHB_(0x04, 0x01, 0x00);
        /* Connect */
        _grp_cyclonevh_RhubConnect(0);
    }
    if (((ulData & CYCLONEVH_B01_PRTCONNSTS) == 0) && (l_ucPortStatus == CYCLONEVH_RH_CONN)) {
        _TRACE_RHB_(0x04, 0x02, 0x00);
        /* clear interrupt factor */
        ulData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_GINTSTS);
        CYCLONEV_R32_WR(CYCLONEV_A32_OTG_GINTSTS, CYCLONEVG_B01_DISCONNINT);
        /* Check not complete request */
        grp_cyclonevh_MngNotCmpReqAbort();
        grp_vos_DelayTask(10);
        /* Disconnect */
        _grp_cyclonevh_RhubDisconnect(0);
    }

    _TRACE_RHB_(0x04, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_RhubConnect                                                      */
/*                                                                                              */
/* DESCRIPTION: Connect procession.                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucPortNum                       Host controller port number                     */
/* OUTPUT     : None                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_RhubConnect(grp_u8 ucPortNum)
{
grp_usbdi_device_info           tDeviceInfo;
grp_si                          iCnt;
grp_si                          iSpeed;
grp_u32                         ulQueData;

    _TRACE_RHB_(0x05, 0x00, 0x00);

    /* check port status */
    if (l_ucPortStatus & CYCLONEVH_RH_CONN) {
        _TRACE_RHB_(0x05, 0x01, 0x00);
        /* disconnecting processing is executed once. */
        _grp_cyclonevh_RhubDisconnect(ucPortNum);
    }

    /* set port status */
    l_ucPortStatus = CYCLONEVH_RH_CONN;

    /* connect wait */
    grp_vos_DelayTask(CYCLONEVH_RH_WAIT_TIME);

    iSpeed = _grp_cyclonevh_RhubPortReset(ucPortNum);
    if (iSpeed == GRP_USBD_UNKNOWN_SPEED) {
        _TRACE_RHB_(0x05, 0x01, END_FUNC);
        /* reset port status */
        l_ucPortStatus = CYCLONEVH_RH_DISC;
        /* error end */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }
    else if (iSpeed == GRP_USBD_LOW_SPEED) {
        /* Notify to the application */
        _WARN_RHB_( GRP_ANOM_LS_DEV_CON, GRP_USB_NULL );
        _TRACE_RHB_(0x05, 0x08, 0x00);
    }

    /* Notify connection to USBD */
    tDeviceInfo.ucHubAddr    = GRP_USBD_ROOTHUB_ADDRESS;    /* Hub Address (Root Hub Address:0) */
    tDeviceInfo.ucPortNum    = ucPortNum;                   /* Port Number                      */
    tDeviceInfo.ucPortInfo   = GRP_USBD_USB_20;             /* USB Version                      */
    tDeviceInfo.usHcIndexNum = GRP_HCDI_CYCLONEVH_TAG;      /* Host controller index number     */

    if (grp_usbd_ConnectDevice(iSpeed, &tDeviceInfo) != GRP_USBD_OK ) {
        _TRACE_RHB_(0x05, 0x02, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* clear queue event */
    while (grp_vos_ReceiveQueue(l_ptCmpQueue, (void *)&ulQueData, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);
    _TRACE_RHB_(0x05, 0x02, 0x00);

    /* create parameters */
    l_tStDevReq.usUsbDevId   = GRP_USBD_DEFAULT_DEVID;
    l_tStDevReq.pvDescriptor = (void *)l_ptDevDesc;
    l_tStDevReq.pfnStCbFunc  = _grp_cyclonevh_RhubRequestCmp;
    l_tStDevReq.pvReferenceP = GRP_USB_NULL;

    /*--- GET_DESCRIPTOR ---*/
    for (iCnt=0; iCnt<CYCLONEVH_RH_RETRY_COUNT; iCnt++) {
        /* It enumeration ends at the time of a low rank driver error. */
        if (grp_usbd_GetDeviceDescriptor(&l_tStDevReq) != GRP_USBD_OK) {
            _TRACE_RHB_(0x05, 0x03, END_FUNC);
            /* error */
            return GRP_HCDI_ERROR;                                      /* DIRECT RETURN    */
        }
        /* Wait for communication complete */
        _TRACE_RHB_(0x05, 0x03, 0x00);
        if (grp_vos_ReceiveQueue(l_ptCmpQueue, (void *)&ulQueData, CYCLONEVH_RH_CMP_WAIT_TIME)
                                                                        != GRP_VOS_POS_RESULT) {
            _TRACE_RHB_(0x05, 0x04, END_FUNC);
            /* error */
            return GRP_HCDI_ERROR;                                      /* DIRECT RETURN    */
        }
        if (l_tStDevReq.lStatus == GRP_USBD_TR_NO_FAIL) {
            if ((l_ptDevDesc->bLength     >= GRP_USBD_DEVICE_DESC_SIZE)
             && (l_ptDevDesc->bDescriptor == GRP_USBD_DESC_DEVICE)) {
                /* success */
                _TRACE_RHB_(0x05, 0x04, 0x00);
                iCnt = 0;
                break;                                                  /* exit FOR loop    */
            }
        }
        /* wait just a moment */
        grp_vos_DelayTask(CYCLONEVH_RH_WAIT_TIME);
        /* reset the port */
        _grp_cyclonevh_RhubPortReset(ucPortNum);
    }

    /* Error check */
    if (iCnt != 0) {
        _TRACE_RHB_(0x05, 0x04, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* wait just a moment */
    grp_vos_DelayTask(CYCLONEVH_RH_WAIT_TIME);

    /* create parameter */
    l_tStDevReq.ucEp0size   = l_ptDevDesc->bMaxPacketSize;
    l_tStDevReq.pfnStCbFunc = _grp_cyclonevh_RhubRequestCmp;

    /*--- SET_ADDRESS ---*/
    for (iCnt=0; iCnt<CYCLONEVH_RH_RETRY_COUNT; iCnt++) {
        /* It enumeration ends at the time of a low rank driver error. */
        if (grp_usbd_SetAddress(&l_tStDevReq) != GRP_USBD_OK) {
            _TRACE_RHB_(0x05, 0x05, END_FUNC);
            /* error */
            return GRP_HCDI_ERROR;                                      /* DIRECT RETURN    */
        }

        _TRACE_RHB_(0x05, 0x05, 0x00);
        /* Wait for communication complete */
        if (grp_vos_ReceiveQueue(l_ptCmpQueue, (void *)&ulQueData, CYCLONEVH_RH_CMP_WAIT_TIME)
                                                                        != GRP_VOS_POS_RESULT) {
            _TRACE_RHB_(0x05, 0x06, END_FUNC);
            /* error */
            return GRP_HCDI_ERROR;                                      /* DIRECT RETURN    */
        }

        if (l_tStDevReq.lStatus == GRP_USBD_TR_NO_FAIL) {
            /* success */
            _TRACE_RHB_(0x05, 0x06, 0x00);
            iCnt = 0;
            break;                                                      /* exit FOR loop    */
        }

        /* Put wait of 100msec for retry */
        grp_vos_DelayTask(CYCLONEVH_RH_WAIT_TIME);
    }

    /* Error check */
    if (iCnt != 0) {
        _TRACE_RHB_(0x05, 0x07, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* save DeviceID */
    l_usDevId = l_tStDevReq.usUsbDevId;

    _TRACE_RHB_(0x05, 0x07, 0x00);

    /* Call configuration software */
    grp_cnfsft_ConfigSoftware(GRP_CNFSFT_DEVICE_ATTACHED, l_tStDevReq.usUsbDevId);

    _TRACE_RHB_(0x05, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_RhubPortReset                                                    */
/*                                                                                              */
/* DESCRIPTION: Execute specified port reset.                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucPortNum                       Host controller port number                     */
/* OUTPUT     : None                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_HIGH_SPEED             high speed device attached                      */
/*              GRP_USBD_LOW_SPEED              low speed device attached                       */
/*              GRP_USBD_FULL_SPEED             full speed attached                             */
/*              GRP_USBD_UNKNOWN_SPEED          error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_RhubPortReset(grp_u8 ucPortNum)
{
grp_s32                         lStatus = GRP_USBD_UNKNOWN_SPEED;
grp_u32                         ulRegData;

    _TRACE_RHB_(0x06, 0x00, 0x00);

    /* set port reset */
    CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HPRT, CYCLONEVH_B01_PRTRST);
    grp_vos_DelayTask(10);
    CYCLONEV_R32_CR(CYCLONEV_A32_OTG_HPRT, CYCLONEVH_B01_PRTRST);
    grp_vos_DelayTask(100);

    /* check HPRT register */
    ulRegData = CYCLONEV_R32_RD(CYCLONEV_A32_OTG_HPRT);
    /* check port speed */
    if ((ulRegData & CYCLONEVH_B02_PRTSPD) == CYCLONEVH_VPRTSPD_HS) {
        _TRACE_RHB_(0x06, 0x01, 0x00);
        lStatus = GRP_USBD_HIGH_SPEED;
    }
    else if ((ulRegData & CYCLONEVH_B02_PRTSPD) == CYCLONEVH_VPRTSPD_FS) {
        _TRACE_RHB_(0x06, 0x02, 0x00);
        lStatus = GRP_USBD_FULL_SPEED;
    }
    else if ((ulRegData & CYCLONEVH_B02_PRTSPD) == CYCLONEVH_VPRTSPD_LS) {
        _TRACE_RHB_(0x06, 0x03, 0x00);
        lStatus = GRP_USBD_LOW_SPEED;
    }
    else {
        _TRACE_RHB_(0x06, 0x04, 0x00);
        lStatus = GRP_USBD_UNKNOWN_SPEED;
    }

    CYCLONEV_R32_WR(CYCLONEV_A32_OTG_HCFG, CYCLONEVH_VFSLSPCLKSEL_48MHZ);
#if (GRP_CYCLONEVH_SPEED_MODE == 0)
    CYCLONEV_R32_ST(CYCLONEV_A32_OTG_HCFG, CYCLONEVH_B01_FSLSSUPP);
#else
    CYCLONEV_R32_CR(CYCLONEV_A32_OTG_HCFG, CYCLONEVH_B01_FSLSSUPP);
#endif

    _TRACE_RHB_(0x06, 0x00, END_FUNC);

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_RhubRequestCmp                                                   */
/*                                                                                              */
/* DESCRIPTION: Standard device request complete function.                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptStDevReq                      Standard device request structure               */
/* OUTPUT     : None                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_RhubRequestCmp(grp_usbdi_st_device_request *ptStDevReq)
{
grp_u32                         ulQueData = CYCLONEVH_RH_CMP_EVENT;

    _TRACE_RHB_(0x07, 0x00, 0x00);

    /* notified to the RHUB task */
    if (grp_vos_SendQueue(l_ptCmpQueue, (void *)&ulQueData, GRP_VOS_INFINITE) != GRP_VOS_POS_RESULT) {
        _TRACE_RHB_(0x07, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    _TRACE_RHB_(0x07, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevh_RhubDisconnect                                                   */
/*                                                                                              */
/* DESCRIPTION: Disconnect procession.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucPortNum                       Host controller port number                     */
/* OUTPUT     : None                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonevh_RhubDisconnect(grp_u8 ucPortNum)
{
    _TRACE_RHB_(0x08, 0x00, 0x00);

    /* check port status */
    if (l_ucPortStatus == CYCLONEVH_RH_DISC) {
        _TRACE_RHB_(0x08, 0x01, END_FUNC);
        /* no process */
        return GRP_HCDI_OK;                                             /* DIRECT RETURN    */
    }

    /* change port status */
    l_ucPortStatus = CYCLONEVH_RH_DISC;

    /* Notify disconnection to USBD */
    grp_usbd_DisconnectDevice(l_usDevId);

    /* If port address is not set(0), program ends in error at enumeration complete. */
    /* No configuration software related handlingis done.                            */
    if (l_usDevId != 0) {
        _TRACE_RHB_(0x08, 0x01, 0x00);
        /* Call configuration software */
        grp_cnfsft_ConfigSoftware(GRP_CNFSFT_DEVICE_DETACHED, l_usDevId);
    }

    /* Disable all channel's */
    grp_cyclonevh_DisableAllChannel();

    /* Flush TxFIFO and RxFIFO */
    grp_cyclonev_cmod_FlushTxFifo(GRP_CYCLONEV_ALL_TXFIFO);
    grp_cyclonev_cmod_FlushRxFifo();

    /* Clear Device ID */
    l_usDevId = 0;

    _TRACE_RHB_(0x08, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_RhubPortPowerOn                                                   */
/*                                                                                              */
/* DESCRIPTION: Power on to the specified port.                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulIndex                         Host controller index number                    */
/*                                              (this controller is not used parameter)         */
/*              ucPortNum                       Host controller port number                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_RhubPortPowerOn(grp_u32 ulIndex, grp_u8 ucPortNum)
{
    _TRACE_RHB_(0x09, 0x00, 0x00);

    /* No support */

    if (ucPortNum != 0) {
        _TRACE_RHB_(0x09, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    _TRACE_RHB_(0x09, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_RhubPortPowerOff                                                  */
/*                                                                                              */
/* DESCRIPTION: Power off to the specified port.                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulIndex                         Host controller index number                    */
/*                                              (this controller is not used parameter)         */
/*              ucPortNum                       Host controller port number                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_RhubPortPowerOff(grp_u32 ulIndex, grp_u8 ucPortNum)
{
    _TRACE_RHB_(0x0A, 0x00, 0x00);

    /* No support */

    if (ucPortNum != 0) {
        _TRACE_RHB_(0x0A, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    _TRACE_RHB_(0x0A, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevh_RhubEnumeration                                                   */
/*                                                                                              */
/* DESCRIPTION: Reset sequence with Enumeration processing.                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucPortNum                       Host controller port number(only 0)             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonevh_RhubEnumeration(grp_u8 ucPortNum)
{
    _TRACE_RHB_(0x0B, 0x00, 0x00);

    if (ucPortNum != 0) {
        _TRACE_RHB_(0x0B, 0x01, END_FUNC);
        /* error */
        return GRP_HCDI_ERROR;                                          /* DIRECT RETURN    */
    }

    /* exclusive access control for the HUB */
    if (grp_usbd_LockEnumeration() != GRP_USBD_OK) {
        _TRACE_RHB_(0x0B, 0x01, 0x00);
        /* illegal error */
    }

    /* Connection */
    _grp_cyclonevh_RhubConnect(ucPortNum);

    /* exclusive access control for the Root HUB */
    if (grp_usbd_UnlockEnumeration() != GRP_USBD_OK) {
        _TRACE_RHB_(0x0B, 0x02, 0x00);
        /* illegal error */
    }

    _TRACE_RHB_(0x0B, 0x00, END_FUNC);

    return GRP_HCDI_OK;
}
