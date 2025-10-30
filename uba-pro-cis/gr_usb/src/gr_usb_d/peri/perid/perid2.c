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
/*      perid.c                                                 1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file composes the GR-USB/Peripheral User Interface.            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  A.Yoshida   2002/12/13  Created initial version 0.01                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
/*  Y.Sato      2003/05/15  It changes Remote Wakeup function               */
/*  K.Takagi    2003/07/16  version 0.92, modified variable names.          */
/*  Y.Sato      2003/07/24  version 0.93, add/rename API function           */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              modified following function to added new    */
/*                              function.                                   */
/*                              - GRUSB_DEV_ApInit2                         */
/*                              added new API functions.                    */
/*                              - GRUSB_DEV_ApClearStall2                   */
/*                              - GRUSB_DEV_ApClearToggle2                  */
/*                              - GRUSB_DEV_ApReqPullupRegister2            */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              Add logging routine.                        */
/*  K.Handa     2005/09/26  V 1.11                                          */
/*                              Changed disconnection flow                  */
/*  K.Handa     2006/12/27  V 1.20                                          */
/*                              deleted state check for some illegal host   */
/*  S.Tomizawa  2007/01/24  V 1.30                                          */
/*                              added function for getting MaxPacketSize    */
/*                              added function for Media Transfer Protocol  */
/*  K.Handa     2007/03/12  V 1.40                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/06/12  V 1.41                                          */
/*                              version was updated.                        */
/*  K.Handa     2008/01/21  V 1.42                                          */
/*                              version was updated.                        */
/*                                                                          */
/****************************************************************************/

/**** INCLUDE FILES *********************************************************/
#include    <string.h>
#include    "perid.h"
#include    "peri_ctl.h"
#include    "peri_hal.h"
#include    "peri_sts.h"
#include    "peri_trs.h"

/* Logging routine */
#ifdef GRUSB_PERI_DEBUG
#include    "dbg_mdl.h"
#define     GRPERI_DBG_TRACE2(m,n,x,y)   GRDBG_TRACE2(m,n,x,y)
#else
#define     GRPERI_DBG_TRACE2(m,n,x,y)
#endif

/**** INTERNAL DATA DEFINES *************************************************/
BOOLEAN              g_bMTPSupport2 = GRUSB_TRUE;        /* 2007/01/24 [1.30] Added for MTP */
#if 1  /* HID */
BOOLEAN              g_bHIDSupport2 = GRUSB_FALSE;       /* 2022/06/21 Added for HID */
#endif /* HID */

DLOCAL  GRUSB_Notice    l_pfnCbConnect2 = GRUSB_NULL;
DLOCAL  GRUSB_Notice    l_pfnCbDisconn2 = GRUSB_NULL;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
LOCAL INT _GRUSB_DEV_EndPointCheckForTransfer2( INT, GRUSB_EndPointInfo**);
LOCAL INT _GRUSB_DEV_EndPointCheck2( INT, GRUSB_EndPointInfo**);

/* 2007/01/24 [1.30] Added for getting MaxPacetSize */
/* following parameters are defineded in peri_prm.c */
EXTERN UINT8                l_ucBusSpd2;                     /* Bus Speed (0:full/1:high) from peri_prm.c */
EXTERN GRUSB_EndPointInfo   l_atEpInfo2[GRUSB_DEV_MAX_EP];   /* Endpoint Information                      */

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_Connect2                                          */
/*                                                                          */
/* DESCRIPTION: Processing when connector is connected.                     */
/*                                                                          */
/* Func Code  : 01                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_pfnCbConnect2          callback function pointer          */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_Connect2(VOID)
{
    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x01, 0, 0 );

    /* initial processing to controller */
    GRUSB_DEV_HALConnectController2();

    /* re-initialize of Control transfer's module */
    GRUSB_DEV_CtrlReInit2();

    /* re-initialize of management parameter */
    GRUSB_Prm_ReInit2();

    /* change state to IDLE */
    GRUSB_DEV_StateSuspend2(GRUSB_FALSE);
    GRUSB_DEV_StateIdle2();

    /* execute callback function */
    if (l_pfnCbConnect2 != GRUSB_NULL)
        (*l_pfnCbConnect2)();

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x01, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_Disconnect2                                       */
/*                                                                          */
/* DESCRIPTION: Processing when connector is disconnected.                  */
/*                                                                          */
/* Func Code  : 02                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_pfnCbDisconn2          callback function pointer          */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_Disconnect2(VOID)
{
    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x02, 0, 0 );

#if 0   /* V1.11 */
    /* Processing of disconnecting to controller */
    GRUSB_DEV_HALDisonnectController2();
#endif  /* V1.11 */

    /* re-initialize of Control transfer's module */
    GRUSB_DEV_CtrlReInit2();

    /* change state to IDLE */
    GRUSB_DEV_StateSuspend2(GRUSB_FALSE);
    GRUSB_DEV_StateIdle2();

    /* execute callback function */
    if (l_pfnCbDisconn2 != GRUSB_NULL)
        (*l_pfnCbDisconn2)();

#if 1   /* V1.11 */
    /* Processing of disconnecting to controller */
    GRUSB_DEV_HALDisonnectController2();
#endif  /* V1.11 */

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x02, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ControlAreaSize2                                  */
/*                                                                          */
/* DESCRIPTION: The size of a management domain is determined.              */
/*                                                                          */
/* Func Code  : 03                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               number of endpoints                     */
/*              pusQueNum           number of queue for several endpoints   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : iSize               size of management area                 */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ControlAreaSize2( INT          iEpNum,
                               UINT16*      pusQueNum)
{
    INT     iSize = 0;
    INT     i;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x03, 0, 0 );

    for (i=0; i<iEpNum; i++)
        iSize += ((pusQueNum[i]+1) * sizeof(GRUSB_TransmitInfo));

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x03, 0, END_FUNC );

    return iSize;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApInit2                                           */
/*                                                                          */
/* DESCRIPTION: Execution of initialization processing                      */
/*                                                                          */
/* Func Code  : 04                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptInitInfo          initial informations                    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       success                             */
/*              GRUSB_DEV_INVALID_PARAM invalid parameters                  */
/*              GRUSB_DEV_INIT_FAILED   error end                           */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApInit2( GRUSB_DEV_INITINFO*       ptInitInfo)
{
    INT     iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x04, 0, 0 );

    /* initialize of parameter management module */
    iRet = GRUSB_Prm_Init2( ptInitInfo);
    if(iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x04, 0x01, END_FUNC );

        return GRUSB_DEV_INVALID_PARAM;
    }

    /* initialize of status management module */
    GRUSB_DEV_StateInit2(ptInitInfo->pfnCngStateNoConfigured);

    /* initialize of control module */
    GRUSB_DEV_CtrlInit2();

    /* register callback functions of the control transfer */
    GRUSB_DEV_CtrlSetCallBack2( ptInitInfo->usReqCodeMap,
                               ptInitInfo->pfnDevRquest,
                               ptInitInfo->pfnSndCancel,
                               ptInitInfo->pfnRcvCancel);

    /* register a callback functions */
    GRUSB_DEV_HALCallbackBusReset2( ptInitInfo->pfnBusReset);                /* BusReset             */
    GRUSB_DEV_HALCallbackSuspend2( ptInitInfo->pfnSuspend);                  /* Suspend              */
    GRUSB_DEV_HALCallbackResume2( ptInitInfo->pfnResume);                    /* Resume               */
    GRUSB_DEV_HALCallbackCmpSetIf2( ptInitInfo->pfnCmpSetInterface);         /* SET_INTERFACE        */
    GRUSB_DEV_HALCallbackCmpSetConfig2( ptInitInfo->pfnCmpSetConfiguration); /* SET_CONFIGURETION    */
    GRUSB_DEV_CtrlCallbackClearFeature2( ptInitInfo->pfnClearFeature);       /* CLEAR_FEATURE        */
    GRUSB_DEV_CtrlCallbackEnRmtWkup2( ptInitInfo->pfnEnbRmtWkup);            /* RemoteWakeup         */

    /* initialize of controller */
    GRUSB_DEV_HALInit2();

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x04, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApGetDeviceState2                                 */
/*                                                                          */
/* DESCRIPTION: It is got that current device state                         */
/*                                                                          */
/* Func Code  : 05                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_STATE_IDLE        Idle(Attached/Powered) state    */
/*              GRUSB_DEV_STATE_DEFAULT     Default state                   */
/*              GRUSB_DEV_STATE_ADDRESS     Address state                   */
/*              GRUSB_DEV_STATE_CONFIGURED  Configured state                */
/*              GRUSB_DEV_STATE_SUSPEND     Suspend state                   */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
GRUSB_DEV_STATE GRUSB_DEV_ApGetDeviceState2(VOID)
{
    GRUSB_DEV_STATE     eRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x05, 0, 0 );

    /* get device state */
    eRet = GRUSB_DEV_StateGetDeviceState2();

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x05, 0, (UINT8)eRet );

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x05, 0, END_FUNC );

    return eRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApControlSend2                                    */
/*                                                                          */
/* DESCRIPTION: Sending data using the Control Transfer.                    */
/*              This function is used for only endpoint_0.                  */
/*                                                                          */
/* Func Code  : 06                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulDataSz            size of data                            */
/*              i0Len               flag to add 0-length                    */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_SIZE          Data size is invalid        */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*              GRUSB_DEV_BUSY_ENDPOINT         Endpoint is busy            */
/*              GRUSB_DEV_STALLING              Now stalling                */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApControlSend2( INT                iEpNo,
                             UINT8*             pucBuf,
                             UINT32             ulDataSz,
                             INT                i0Len,
                             VOID*              pAplInfo,
                             GRUSB_DataPushEnd  pfnFunc)
{
    INT                 iRet;
    GRUSB_EndPointInfo* ptEpInfo;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x06, iEpNo, ulDataSz );

    iRet = _GRUSB_DEV_EndPointCheck2( iEpNo, &ptEpInfo);
    /* check endpoint */
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x06, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint information */
    }

    /* get device status */
    iRet = GRUSB_DEV_StateGetDeviceState2();

#if 0   /* 1.20 */
    /* check device status */
    if (iRet != GRUSB_DEV_STATE_CONFIGURED)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x06, 0x02, END_FUNC );

        return GRUSB_DEV_NOT_CONFIGURED;            /* invalid status */
    }
#endif

    /* check endpoint type */
    if (!GRUSB_DEV_MAC_IsEpTypeCTRL(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x06, 0x03, END_FUNC );

        return GRUSB_DEV_INVALID_PROTOCOL;          /* invalid transmission type */
    }

    iRet = GRUSB_DEV_DataSetSndCtrlData2( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc);

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x06, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferSend2                                   */
/*                                                                          */
/* DESCRIPTION: Sending data using several transfer.                        */
/*                                                                          */
/* Func Code  : 07                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulDataSz            size of data                            */
/*              i0Len               flag to add 0-length                    */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_SIZE          Data size is invalid        */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*              GRUSB_DEV_BUSY_ENDPOINT         Endpoint is busy            */
/*              GRUSB_DEV_STALLING              Now stalling                */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApTransferSend2( INT               iEpNo,
                              UINT8*            pucBuf,
                              UINT32            ulDataSz,
                              BOOLEAN           i0Len,
                              VOID*             pAplInfo,
                              GRUSB_DataPushEnd pfnFunc)
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x07, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer2( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x07, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* setting to send data */
    iRet = GRUSB_DEV_DataSetSndData2( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc);

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x07, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferSendQueue2                              */
/*                                                                          */
/* DESCRIPTION: Request sending data using several transfer (only queuing). */
/*                                                                          */
/* Func Code  : 08                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulDataSz            size of data                            */
/*              i0Len               flag to add 0-length                    */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_SIZE          Data size is invalid        */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*              GRUSB_DEV_BUSY_ENDPOINT         Endpoint is busy            */
/*              GRUSB_DEV_STALLING              Now stalling                */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApTransferSendQueue2( INT                  iEpNo,
                                   UINT8*               pucBuf,
                                   UINT32               ulDataSz,
                                   BOOLEAN              i0Len,
                                   VOID*                pAplInfo,
                                   GRUSB_DataPushEnd    pfnFunc)
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x08, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer2( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x08, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* queuing to request sending data */
    iRet = GRUSB_DEV_SndDataWriteToQueue2( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc );

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x08, 0, END_FUNC );

    return iRet;

}


/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferSendStart2                              */
/*                                                                          */
/* DESCRIPTION: Start sending data in queue (not Control transfer)          */
/*                                                                          */
/* Func Code  : 09                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_SIZE          Data size is invalid        */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*              GRUSB_DEV_BUSY_ENDPOINT         Endpoint is busy            */
/*              GRUSB_DEV_STALLING              Now stalling                */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApTransferSendStart2( INT iEpNo )
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x09, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer2( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x09, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* start sending data of queuing request */
    GRUSB_DEV_HALDataWrite2( ptEpInfo );

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x09, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApAbort2                                          */
/*                                                                          */
/* DESCRIPTION: Transmission of specified endpoint is interrupted.          */
/*              The tranmission is not performed until the inside of FIFO   */
/*              is also cleared and the following data is specified.        */
/*                                                                          */
/* Func Code  : 0A                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo                   endpoint number                     */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApAbort2( INT      iEpNo)
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0A, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck2( iEpNo, &ptEpInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0A, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* check endpoint type */
    if (GRUSB_DEV_MAC_IsEpTypeIN(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0A, 0, 0x01 );
        /* abort sending */
        iRet = GRUSB_DEV_DataSndAbort2( ptEpInfo );
    }
    else if (GRUSB_DEV_MAC_IsEpTypeOUT(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0A, 0, 0x02 );
        /* abort receiving */
        iRet = GRUSB_DEV_DataRcvAbort2( ptEpInfo );
    }

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0A, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApSetStall2                                       */
/*                                                                          */
/* DESCRIPTION: The specified endpoint is set as a STALL state.             */
/*                                                                          */
/* Func Code  : 0B                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo                   endpoint number                     */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApSetStall2( INT       iEpNo)
{
    GRUSB_EndPointInfo* ptEPInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0B, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck2( iEpNo, &ptEPInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0B, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* set as a STALL state */
    GRUSB_DEV_HALSetStallState2( iEpNo, GRUSB_TRUE);

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0B, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApClearStall2                                     */
/*                                                                          */
/* DESCRIPTION: The specified endpoint is clear from a STALL state.         */
/*                                                                          */
/* Func Code  : 0C                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo                   endpoint number                     */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApClearStall2( INT         iEpNo)
{
    GRUSB_EndPointInfo* ptEPInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0C, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck2( iEpNo, &ptEPInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0C, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* clear from a Stall state */
    GRUSB_DEV_StateSetStall2( iEpNo, GRUSB_FALSE);

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0C, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApClearToggle2                                    */
/*                                                                          */
/* DESCRIPTION: The specified endpoint is clear toggle bit.                 */
/*                                                                          */
/* Func Code  : 0D                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo                   endpoint number                     */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApClearToggle2( INT        iEpNo)
{
    GRUSB_EndPointInfo* ptEPInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0D, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck2( iEpNo, &ptEPInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0D, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* clear of toggle bit */
    GRUSB_DEV_HALTogleClear2( iEpNo);

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0D, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApControlRecv2                                    */
/*                                                                          */
/* DESCRIPTION: The callback function for Control transmission reception is */
/*              registered.                                                 */
/*              This function acts only on endpoint_0.                      */
/*                                                                          */
/* Func Code  : 0E                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulDataSz            size of data                            */
/*              i0Len               flag to add 0-length                    */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApControlRecv2( INT                    iEpNo,
                             UINT8*                 pucBuf,
                             UINT32                 ulSize,
                             VOID*                  pAplInfo,
                             GRUSB_DataReceiveEnd   pfnFunc)
{
    GRUSB_EndPointInfo*     ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0E, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck2( iEpNo, &ptEpInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0E, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    if (!GRUSB_DEV_MAC_IsEpTypeCTRL(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0E, 0x02, END_FUNC );

        return GRUSB_DEV_INVALID_ENDPOINT;
    }

    /* seting data baffer to receive */
    GRUSB_DEV_BufferSetCtrlRcvData2( ptEpInfo, ulSize, pucBuf, pAplInfo, pfnFunc);

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0E, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferRecv2                                   */
/*                                                                          */
/* DESCRIPTION: Callback functions for reception other than Control Trans-  */
/*              mission are registered.                                     */
/*                                                                          */
/* Func Code  : 0F                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              i0Len               flag to add 0-length                    */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*              GRUSB_DEV_BUSY_ENDPOINT         Endpoint is busy            */
/*              GRUSB_DEV_STALLING              Now stalling                */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApTransferRecv2( INT                   iEpNo,
                              UINT8*                pucBuf,
                              UINT32                ulSize,
                              VOID*                 pAplInfo,
                              GRUSB_DataReceiveEnd  pfnFunc)
{
    GRUSB_EndPointInfo      *ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0F, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer2( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0F, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* request to receive data */
    iRet = GRUSB_DEV_BufferSetRcvData2( ptEpInfo, ulSize, pucBuf, pAplInfo, pfnFunc );

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x0F, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferRecvQueue2                              */
/*                                                                          */
/* DESCRIPTION: Callback functions for reception other than Control Trans-  */
/*              mission are registered. But request is only queuing.        */
/*                                                                          */
/* Func Code  : 10                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              i0Len               flag to add 0-length                    */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*              GRUSB_DEV_BUSY_ENDPOINT         Endpoint is busy            */
/*              GRUSB_DEV_STALLING              Now stalling                */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApTransferRecvQueue2( INT                  iEpNo,
                                   UINT8*               pucBuf,
                                   UINT32               ulSize,
                                   VOID*                pAplInfo,
                                   GRUSB_DataReceiveEnd pfnFunc)
{
    GRUSB_EndPointInfo      *ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x10, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer2( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x10, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* queuing request */
    iRet = GRUSB_DEV_RcvBufferWriteToQueue2( ptEpInfo, ulSize, pucBuf, pAplInfo, pfnFunc );

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x10, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferRecvStart2                              */
/*                                                                          */
/* DESCRIPTION: Start receiving data in queue (not Control transfer).       */
/*                                                                          */
/* Func Code  : 11                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*              GRUSB_DEV_INVALID_PROTOCOL      Transmission type is invalid*/
/*              GRUSB_DEV_NOT_CONFIGURED        Device is no CONFIGURED     */
/*                                              condition                   */
/*              GRUSB_DEV_BUSY_ENDPOINT         Endpoint is busy            */
/*              GRUSB_DEV_STALLING              Now stalling                */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApTransferRecvStart2( INT iEpNo )
{
    GRUSB_EndPointInfo      *ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x11, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer2( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x11, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* start to receive data */
    GRUSB_DEV_HALDataFIFORcvStart2( ptEpInfo );

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x11, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}


/****************************************************************************/
/* FUNCTION   : _GRUSB_DEV_EndPointCheckForTransfer2                        */
/*                                                                          */
/* DESCRIPTION: Check specified endpoint information and get it.            */
/*                                                                          */
/* Func Code  : 12                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/* OUTPUT     : pptEpInfo           endpoint information                    */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_DEV_EndPointCheckForTransfer2( INT                  iEpNo,
                                               GRUSB_EndPointInfo** pptEpInfo)
{
    INT                     iRet;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x12, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck2( iEpNo, pptEpInfo );
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x12, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* get device state */
    iRet = GRUSB_DEV_StateGetDeviceState2();
    if( iRet != GRUSB_DEV_STATE_CONFIGURED )        /* is it not CONFIGURED state? */
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x12, 0x02, END_FUNC );

        return GRUSB_DEV_NOT_CONFIGURED;            /* yes: invalid state */
    }

    /* check endpoint transmission type */
    if ((!GRUSB_DEV_MAC_IsEpTypeBLK((*pptEpInfo)->ucEpType))
     && (!GRUSB_DEV_MAC_IsEpTypeINTR((*pptEpInfo)->ucEpType))
     && (!GRUSB_DEV_MAC_IsEpTypeISO((*pptEpInfo)->ucEpType)))
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x12, 0x03, END_FUNC );

        return GRUSB_DEV_INVALID_PROTOCOL;          /* invalid transmission type */
    }

    /* get endpoint stall information */
    iRet = GRUSB_DEV_StateGetStall2( iEpNo );
    if( iRet != GRUSB_FALSE )
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x12, 0x04, END_FUNC );

        return GRUSB_DEV_STALLING;                  /* under the STALL state */
    }

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x12, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}


/****************************************************************************/
/* FUNCTION   : _GRUSB_DEV_EndPointCheck2                                   */
/*                                                                          */
/* DESCRIPTION: The specified endpoint information is acquired.             */
/*                                                                          */
/* Func Code  : 13                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/* OUTPUT     : pptEpInfo           endpoint information                    */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*              GRUSB_DEV_INVALID_ENDPOINT      Endpoint number is invalid  */
/*              GRUSB_DEV_UNDEFINED_ENDPOINT    Specified endpoint is not   */
/*                                              defined                     */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_DEV_EndPointCheck2( INT                     iEpNo,
                                    GRUSB_EndPointInfo**    pptEpInfo)
{
    GRUSB_EndPointInfo  *ptEpInfo;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x13, 0, 0 );

    /* check endpoint number */
    if (iEpNo > GRUSB_DEV_MAX_EP)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x13, 0x01, END_FUNC );

        return GRUSB_DEV_UNDEFINED_ENDPOINT;        /* invalid endpoint number */
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo);
    if (ptEpInfo == GRUSB_NULL)
    {
        GRPERI_DBG_TRACE2( GRDBG_PERID, 0x13, 0x02, END_FUNC );

        return GRUSB_DEV_INVALID_ENDPOINT;          /* invalid endpoint */
    }

    *pptEpInfo = ptEpInfo;

    GRPERI_DBG_TRACE2( GRDBG_PERID, 0x13, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApCallbackConnect2                                */
/*                                                                          */
/* DESCRIPTION: Callback function called at the time of cable connecting is */
/*              registered.                                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : pFnc                function pointer                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*--------------------------------------------------------------------------*/
/* REFER      : l_pfnCbConnect2      function pointer                       */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApCallbackConnect2( GRUSB_Notice       pFnc)
{
    l_pfnCbConnect2 = pFnc;

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApCallbackDisconnect2                              */
/*                                                                          */
/* DESCRIPTION: Callback function called at the time of cable disconnecting */
/*              is registered.                                              */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : pFnc                function pointer                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*--------------------------------------------------------------------------*/
/* REFER      : l_pfnCbConnect2      function pointer                       */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApCallbackDisconnect2( GRUSB_Notice        pFnc)
{
    l_pfnCbDisconn2 = pFnc;

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApRmtWkup2                                        */
/*                                                                          */
/* DESCRIPTION: Remote wakeup request is executed.                          */
/*                                                                          */
/* Func Code  : 14                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS               Success                     */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApRmtWkup2(VOID)
{
    GRPERI_DBG_TRACE2(GRDBG_PERID, 0x14, 0, 0);

    /* request of Remote wakeup */
    GRUSB_DEV_CtrlRmtWkup2();

    GRPERI_DBG_TRACE2(GRDBG_PERID, 0x14, 0, END_FUNC);

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApReqPullupRegister2                              */
/*                                                                          */
/* DESCRIPTION: Request to the HAL module to contorol of pullup register    */
/*                                                                          */
/* Func Code  : 14                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iReqFlag        GRUSB_TRUE  = Pullup ON when VBUS is drived */
/*                              GRUSB_FALSE = Pullup OFF                    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_ApReqPullupRegister2(INT      iReqFlag)
{
    /* request to HAL module */
    GRUSB_DEV_HALReqPullupRegister2( iReqFlag);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApGetMaxPacketSize2                               */
/*                                                                          */
/* DESCRIPTION: returns the MaxPacketSize of a indicated endpoint.          */
/*              2007/01/24 [1.30] Added                                     */
/*                                                                          */
/* Func Code  :                                                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : INT     iEpNo           Endpoint No                         */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : MaxPacketSize                                               */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
UINT16 GRUSB_DEV_ApGetMaxPacketSize2( INT iEpNo )
{
    return l_atEpInfo2[iEpNo].usMaxPktSz[l_ucBusSpd2];
}

#ifdef GRUSB_MTP_MS_VENDORCODE
/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApSetMTPSupportFlg2                               */
/*                                                                          */
/* DESCRIPTION: sets whether to process the OS feature descriptor.          */
/*              2007/01/24 [1.30] Added for MTP                             */
/* Func Code  :                                                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : BOOLEAN bFlg                                                */
/* OUTPUT     : BOOLEAN g_bMTPSupport2                                      */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_ApSetMTPSupportFlg2( BOOLEAN bFlg )
{
    g_bMTPSupport2 = bFlg;
}
#endif

#if 1  /* HID */
/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApSetHHIDSupportFlg2                               */
/*                                                                          */
/* DESCRIPTION: sets whether to process the OS feature descriptor.          */
/*              2022/06/21  Added for HID                                   */
/* Func Code  :                                                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : BOOLEAN bFlg                                                */
/* OUTPUT     : BOOLEAN g_bHIDSupport2                                      */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_ApSetHIDSupportFlg2( BOOLEAN bFlg )
{
    g_bHIDSupport2 = bFlg;
}
#endif  /* HID */

