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
/*                              - GRUSB_DEV_ApInit                          */
/*                              added new API functions.                    */
/*                              - GRUSB_DEV_ApClearStall                    */
/*                              - GRUSB_DEV_ApClearToggle                   */
/*                              - GRUSB_DEV_ApReqPullupRegister             */
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
#define     GRPERI_DBG_TRACE(m,n,x,y)   GRDBG_TRACE(m,n,x,y)
#else
#define     GRPERI_DBG_TRACE(m,n,x,y)
#endif

/**** INTERNAL DATA DEFINES *************************************************/
BOOLEAN              g_bMTPSupport = GRUSB_TRUE;        /* 2007/01/24 [1.30] Added for MTP */

DLOCAL  GRUSB_Notice    l_pfnCbConnect = GRUSB_NULL;
DLOCAL  GRUSB_Notice    l_pfnCbDisconn = GRUSB_NULL;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
LOCAL INT _GRUSB_DEV_EndPointCheckForTransfer( INT, GRUSB_EndPointInfo**);
LOCAL INT _GRUSB_DEV_EndPointCheck( INT, GRUSB_EndPointInfo**);

/* 2007/01/24 [1.30] Added for getting MaxPacetSize */
/* following parameters are defineded in peri_prm.c */
EXTERN UINT8                l_ucBusSpd;                     /* Bus Speed (0:full/1:high) from peri_prm.c */
EXTERN GRUSB_EndPointInfo   l_atEpInfo[GRUSB_DEV_MAX_EP];   /* Endpoint Information                      */

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_Connect                                           */
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
/* REFER      : l_pfnCbConnect          callback function pointer           */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_Connect(VOID)
{
    GRPERI_DBG_TRACE( GRDBG_PERID, 0x01, 0, 0 );

    /* initial processing to controller */
    GRUSB_DEV_HALConnectController();

    /* re-initialize of Control transfer's module */
    GRUSB_DEV_CtrlReInit();

    /* re-initialize of management parameter */
    GRUSB_Prm_ReInit();

    /* change state to IDLE */
    GRUSB_DEV_StateSuspend(GRUSB_FALSE);
    GRUSB_DEV_StateIdle();

    /* execute callback function */
    if (l_pfnCbConnect != GRUSB_NULL)
        (*l_pfnCbConnect)();

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x01, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_Disconnect                                        */
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
/* REFER      : l_pfnCbDisconn          callback function pointer           */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_Disconnect(VOID)
{
    GRPERI_DBG_TRACE( GRDBG_PERID, 0x02, 0, 0 );

#if 0   /* V1.11 */
    /* Processing of disconnecting to controller */
    GRUSB_DEV_HALDisonnectController();
#endif  /* V1.11 */

    /* re-initialize of Control transfer's module */
    GRUSB_DEV_CtrlReInit();

    /* change state to IDLE */
    GRUSB_DEV_StateSuspend(GRUSB_FALSE);
    GRUSB_DEV_StateIdle();

    /* execute callback function */
    if (l_pfnCbDisconn != GRUSB_NULL)
        (*l_pfnCbDisconn)();

#if 1   /* V1.11 */
    /* Processing of disconnecting to controller */
    GRUSB_DEV_HALDisonnectController();
#endif  /* V1.11 */

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x02, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ControlAreaSize                                   */
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
INT GRUSB_DEV_ControlAreaSize( INT          iEpNum,
                               UINT16*      pusQueNum)
{
    INT     iSize = 0;
    INT     i;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x03, 0, 0 );

    for (i=0; i<iEpNum; i++)
        iSize += ((pusQueNum[i]+1) * sizeof(GRUSB_TransmitInfo));

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x03, 0, END_FUNC );

    return iSize;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApInit                                            */
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
INT GRUSB_DEV_ApInit( GRUSB_DEV_INITINFO*       ptInitInfo)
{
    INT     iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x04, 0, 0 );

    /* initialize of parameter management module */
    iRet = GRUSB_Prm_Init( ptInitInfo);
    if(iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x04, 0x01, END_FUNC );

        return GRUSB_DEV_INVALID_PARAM;
    }

    /* initialize of status management module */
    GRUSB_DEV_StateInit(ptInitInfo->pfnCngStateNoConfigured);

    /* initialize of control module */
    GRUSB_DEV_CtrlInit();

    /* register callback functions of the control transfer */
    GRUSB_DEV_CtrlSetCallBack( ptInitInfo->usReqCodeMap,
                               ptInitInfo->pfnDevRquest,
                               ptInitInfo->pfnSndCancel,
                               ptInitInfo->pfnRcvCancel);

    /* register a callback functions */
    GRUSB_DEV_HALCallbackBusReset( ptInitInfo->pfnBusReset);                /* BusReset             */
    GRUSB_DEV_HALCallbackSuspend( ptInitInfo->pfnSuspend);                  /* Suspend              */
    GRUSB_DEV_HALCallbackResume( ptInitInfo->pfnResume);                    /* Resume               */
    GRUSB_DEV_HALCallbackCmpSetIf( ptInitInfo->pfnCmpSetInterface);         /* SET_INTERFACE        */
    GRUSB_DEV_HALCallbackCmpSetConfig( ptInitInfo->pfnCmpSetConfiguration); /* SET_CONFIGURETION    */
    GRUSB_DEV_CtrlCallbackClearFeature( ptInitInfo->pfnClearFeature);       /* CLEAR_FEATURE        */
    GRUSB_DEV_CtrlCallbackEnRmtWkup( ptInitInfo->pfnEnbRmtWkup);            /* RemoteWakeup         */

    /* initialize of controller */
    GRUSB_DEV_HALInit();

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x04, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApGetDeviceState                                  */
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
GRUSB_DEV_STATE GRUSB_DEV_ApGetDeviceState(VOID)
{
    GRUSB_DEV_STATE     eRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x05, 0, 0 );

    /* get device state */
    eRet = GRUSB_DEV_StateGetDeviceState();

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x05, 0, (UINT8)eRet );

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x05, 0, END_FUNC );

    return eRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApControlSend                                     */
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
INT GRUSB_DEV_ApControlSend( INT                iEpNo,
                             UINT8*             pucBuf,
                             UINT32             ulDataSz,
                             INT                i0Len,
                             VOID*              pAplInfo,
                             GRUSB_DataPushEnd  pfnFunc)
{
    INT                 iRet;
    GRUSB_EndPointInfo* ptEpInfo;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x06, iEpNo, ulDataSz );

    iRet = _GRUSB_DEV_EndPointCheck( iEpNo, &ptEpInfo);
    /* check endpoint */
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x06, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint information */
    }

    /* get device status */
    iRet = GRUSB_DEV_StateGetDeviceState();

#if 0   /* 1.20 */
    /* check device status */
    if (iRet != GRUSB_DEV_STATE_CONFIGURED)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x06, 0x02, END_FUNC );

        return GRUSB_DEV_NOT_CONFIGURED;            /* invalid status */
    }
#endif

    /* check endpoint type */
    if (!GRUSB_DEV_MAC_IsEpTypeCTRL(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x06, 0x03, END_FUNC );

        return GRUSB_DEV_INVALID_PROTOCOL;          /* invalid transmission type */
    }

    iRet = GRUSB_DEV_DataSetSndCtrlData( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc);

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x06, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferSend                                    */
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
INT GRUSB_DEV_ApTransferSend( INT               iEpNo,
                              UINT8*            pucBuf,
                              UINT32            ulDataSz,
                              BOOLEAN           i0Len,
                              VOID*             pAplInfo,
                              GRUSB_DataPushEnd pfnFunc)
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x07, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x07, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* setting to send data */
    iRet = GRUSB_DEV_DataSetSndData( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc);

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x07, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferSendQueue                               */
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
INT GRUSB_DEV_ApTransferSendQueue( INT                  iEpNo,
                                   UINT8*               pucBuf,
                                   UINT32               ulDataSz,
                                   BOOLEAN              i0Len,
                                   VOID*                pAplInfo,
                                   GRUSB_DataPushEnd    pfnFunc)
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x08, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x08, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* queuing to request sending data */
    iRet = GRUSB_DEV_SndDataWriteToQueue( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc );

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x08, 0, END_FUNC );

    return iRet;

}


/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferSendStart                               */
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
INT GRUSB_DEV_ApTransferSendStart( INT iEpNo )
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x09, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x09, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* start sending data of queuing request */
    GRUSB_DEV_HALDataWrite( ptEpInfo );

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x09, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApAbort                                           */
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
INT GRUSB_DEV_ApAbort( INT      iEpNo)
{
    GRUSB_EndPointInfo* ptEpInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0A, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck( iEpNo, &ptEpInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0A, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* check endpoint type */
    if (GRUSB_DEV_MAC_IsEpTypeIN(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0A, 0, 0x01 );
        /* abort sending */
        iRet = GRUSB_DEV_DataSndAbort( ptEpInfo );
    }
    else if (GRUSB_DEV_MAC_IsEpTypeOUT(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0A, 0, 0x02 );
        /* abort receiving */
        iRet = GRUSB_DEV_DataRcvAbort( ptEpInfo );
    }

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0A, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApSetStall                                        */
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
INT GRUSB_DEV_ApSetStall( INT       iEpNo)
{
    GRUSB_EndPointInfo* ptEPInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0B, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck( iEpNo, &ptEPInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0B, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* set as a STALL state */
    GRUSB_DEV_HALSetStallState( iEpNo, GRUSB_TRUE);

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0B, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApClearStall                                      */
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
INT GRUSB_DEV_ApClearStall( INT         iEpNo)
{
    GRUSB_EndPointInfo* ptEPInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0C, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck( iEpNo, &ptEPInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0C, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* clear from a Stall state */
    GRUSB_DEV_StateSetStall( iEpNo, GRUSB_FALSE);

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0C, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApClearToggle                                     */
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
INT GRUSB_DEV_ApClearToggle( INT        iEpNo)
{
    GRUSB_EndPointInfo* ptEPInfo;
    INT                 iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0D, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck( iEpNo, &ptEPInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0D, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* clear of toggle bit */
    GRUSB_DEV_HALTogleClear( iEpNo);

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0D, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApControlRecv                                     */
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
INT GRUSB_DEV_ApControlRecv( INT                    iEpNo,
                             UINT8*                 pucBuf,
                             UINT32                 ulSize,
                             VOID*                  pAplInfo,
                             GRUSB_DataReceiveEnd   pfnFunc)
{
    GRUSB_EndPointInfo*     ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0E, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck( iEpNo, &ptEpInfo);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0E, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    if (!GRUSB_DEV_MAC_IsEpTypeCTRL(ptEpInfo->ucEpType))
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0E, 0x02, END_FUNC );

        return GRUSB_DEV_INVALID_ENDPOINT;
    }

    /* seting data baffer to receive */
    GRUSB_DEV_BufferSetCtrlRcvData( ptEpInfo, ulSize, pucBuf, pAplInfo, pfnFunc);

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0E, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferRecv                                    */
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
INT GRUSB_DEV_ApTransferRecv( INT                   iEpNo,
                              UINT8*                pucBuf,
                              UINT32                ulSize,
                              VOID*                 pAplInfo,
                              GRUSB_DataReceiveEnd  pfnFunc)
{
    GRUSB_EndPointInfo      *ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0F, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x0F, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* request to receive data */
    iRet = GRUSB_DEV_BufferSetRcvData( ptEpInfo, ulSize, pucBuf, pAplInfo, pfnFunc );

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x0F, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferRecvQueue                               */
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
INT GRUSB_DEV_ApTransferRecvQueue( INT                  iEpNo,
                                   UINT8*               pucBuf,
                                   UINT32               ulSize,
                                   VOID*                pAplInfo,
                                   GRUSB_DataReceiveEnd pfnFunc)
{
    GRUSB_EndPointInfo      *ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x10, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x10, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* queuing request */
    iRet = GRUSB_DEV_RcvBufferWriteToQueue( ptEpInfo, ulSize, pucBuf, pAplInfo, pfnFunc );

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x10, 0, END_FUNC );

    return iRet;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApTransferRecvStart                               */
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
INT GRUSB_DEV_ApTransferRecvStart( INT iEpNo )
{
    GRUSB_EndPointInfo      *ptEpInfo;
    INT                     iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x11, 0, 0 );

    iRet = _GRUSB_DEV_EndPointCheckForTransfer( iEpNo, &ptEpInfo );
    /* check return value */
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x11, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* start to receive data */
    GRUSB_DEV_HALDataFIFORcvStart( ptEpInfo );

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x11, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}


/****************************************************************************/
/* FUNCTION   : _GRUSB_DEV_EndPointCheckForTransfer                         */
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
LOCAL INT _GRUSB_DEV_EndPointCheckForTransfer( INT                  iEpNo,
                                               GRUSB_EndPointInfo** pptEpInfo)
{
    INT                     iRet;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x12, 0, 0 );

    /* check endpoint */
    iRet = _GRUSB_DEV_EndPointCheck( iEpNo, pptEpInfo );
    if( iRet != GRUSB_DEV_SUCCESS )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x12, 0x01, END_FUNC );

        return iRet;                                /* invalid endpoint */
    }

    /* get device state */
    iRet = GRUSB_DEV_StateGetDeviceState();
    if( iRet != GRUSB_DEV_STATE_CONFIGURED )        /* is it not CONFIGURED state? */
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x12, 0x02, END_FUNC );

        return GRUSB_DEV_NOT_CONFIGURED;            /* yes: invalid state */
    }

    /* check endpoint transmission type */
    if ((!GRUSB_DEV_MAC_IsEpTypeBLK((*pptEpInfo)->ucEpType))
     && (!GRUSB_DEV_MAC_IsEpTypeINTR((*pptEpInfo)->ucEpType))
     && (!GRUSB_DEV_MAC_IsEpTypeISO((*pptEpInfo)->ucEpType)))
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x12, 0x03, END_FUNC );

        return GRUSB_DEV_INVALID_PROTOCOL;          /* invalid transmission type */
    }

    /* get endpoint stall information */
    iRet = GRUSB_DEV_StateGetStall( iEpNo );
    if( iRet != GRUSB_FALSE )
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x12, 0x04, END_FUNC );

        return GRUSB_DEV_STALLING;                  /* under the STALL state */
    }

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x12, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}


/****************************************************************************/
/* FUNCTION   : _GRUSB_DEV_EndPointCheck                                    */
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
LOCAL INT _GRUSB_DEV_EndPointCheck( INT                     iEpNo,
                                    GRUSB_EndPointInfo**    pptEpInfo)
{
    GRUSB_EndPointInfo  *ptEpInfo;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x13, 0, 0 );

    /* check endpoint number */
    if (iEpNo > GRUSB_DEV_MAX_EP)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x13, 0x01, END_FUNC );

        return GRUSB_DEV_UNDEFINED_ENDPOINT;        /* invalid endpoint number */
    }

    /* get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo( iEpNo);
    if (ptEpInfo == GRUSB_NULL)
    {
        GRPERI_DBG_TRACE( GRDBG_PERID, 0x13, 0x02, END_FUNC );

        return GRUSB_DEV_INVALID_ENDPOINT;          /* invalid endpoint */
    }

    *pptEpInfo = ptEpInfo;

    GRPERI_DBG_TRACE( GRDBG_PERID, 0x13, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApCallbackConnect                                 */
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
/* REFER      : l_pfnCbConnect      function pointer                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApCallbackConnect( GRUSB_Notice       pFnc)
{
    l_pfnCbConnect = pFnc;

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApCallbackDisconnect                              */
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
/* REFER      : l_pfnCbConnect      function pointer                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_ApCallbackDisconnect( GRUSB_Notice        pFnc)
{
    l_pfnCbDisconn = pFnc;

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApRmtWkup                                         */
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
INT GRUSB_DEV_ApRmtWkup(VOID)
{
    GRPERI_DBG_TRACE(GRDBG_PERID, 0x14, 0, 0);

    /* request of Remote wakeup */
    GRUSB_DEV_CtrlRmtWkup();

    GRPERI_DBG_TRACE(GRDBG_PERID, 0x14, 0, END_FUNC);

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApReqPullupRegister                               */
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
VOID GRUSB_DEV_ApReqPullupRegister(INT      iReqFlag)
{
    /* request to HAL module */
    GRUSB_DEV_HALReqPullupRegister( iReqFlag);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApGetMaxPacketSize                                */
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
UINT16 GRUSB_DEV_ApGetMaxPacketSize( INT iEpNo )
{
    return l_atEpInfo[iEpNo].usMaxPktSz[l_ucBusSpd];
}

#ifdef GRUSB_MTP_MS_VENDORCODE
/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_ApSetMTPSupportFlg                                */
/*                                                                          */
/* DESCRIPTION: sets whether to process the OS feature descriptor.          */
/*              2007/01/24 [1.30] Added for MTP                             */
/* Func Code  :                                                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : BOOLEAN bFlg                                                */
/* OUTPUT     : BOOLEAN g_bMTPSupport                                       */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_DEV_ApSetMTPSupportFlg( BOOLEAN bFlg )
{
    g_bMTPSupport = bFlg;
}
#endif

