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
/*      peri_trs.c                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This module manages data transfer for peripheral.                   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  Y.Sano      2002/12/13  Created initial version 0.01                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
/*  K.Takagi    2003/07/16  version 0.92, modified variable names.          */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              Add logging routine.                        */
/*  K.Handa     2006/12/27  V 1.20                                          */
/*                              version was updated.                        */
/*  S.Tomizawa  2007/01/24  V 1.30                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/03/12  V 1.40                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/06/12  V 1.41                                          */
/*                              version was updated.                        */
/*  K.Handa     2008/01/21  V 1.42                                          */
/*                              version was updated.                        */
/*                                                                          */
/****************************************************************************/

/**** INCLUDE FILES *********************************************************/
#include    "perid.h"
#include    "peri_hal.h"
#include    "peri_trs.h"
#include    "peri_sts.h"

/* Logging routine */
#ifdef GRUSB_TRS_DEBUG
#include    "dbg_mdl.h"
#define     GRTRS_DBG_TRACE(m,n,x,y)    GRDBG_TRACE(m,n,x,y)
#else
#define     GRTRS_DBG_TRACE(m,n,x,y)
#endif

/**** INTERNAL DEFINES ******************************************************/
/* Endpoint information */
#define GRUSB_TRS_NON_BUSY  0
#define GRUSB_TRS_BUSY      1

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
LOCAL INT _GRUSB_DEV_ReqSnd2Q( GRUSB_EndPointInfo*, UINT32, UINT8*,
                               BOOLEAN , VOID *, GRUSB_DataPushEnd);
LOCAL INT _GRUSB_DEV_ReqRcvBf2Q( GRUSB_EndPointInfo*, UINT32 , UINT8*,
                                 VOID*, GRUSB_DataReceiveEnd);

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_DataSetSndCtrlData                                */
/*                                                                          */
/* DESCRIPTION: Sending data to use Control transfer.                       */
/*                                                                          */
/* Func Code  : 01                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo            endpoint information                    */
/*              ulDataSz            size of data                            */
/*              pucBuf              pointer of data buffer                  */
/*              i0Len               flag to add 0-length                    */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_DataSetSndCtrlData( GRUSB_EndPointInfo*   ptEpInfo,
                                  UINT32                ulDataSz,
                                  UINT8*                pucBuf,
                                  BOOLEAN               i0Len,
                                  VOID*                 pAplInfo,
                                  GRUSB_DataPushEnd     pfnFunc)
{
    INT     iRet;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x01, 0, 0 );

    iRet = _GRUSB_DEV_ReqSnd2Q( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc);

    if (iRet != GRUSB_DEV_SUCCESS)                  /* if return value is not success */
    {
        GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x01, 0x01, END_FUNC );

        /* set STALL state */
        GRUSB_DEV_HALSetStallState( GRUSB_DEV_EP0, GRUSB_TRUE);
        return iRet;
    }

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x01, 0, END_FUNC );

    return  GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_BufferSetRcvCtrlData                              */
/*                                                                          */
/* DESCRIPTION: Receiving data to us Control transfer.                      */
/*                                                                          */
/* Func Code  : 02                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo            endpoint information                    */
/*              ulDataSz            size of data                            */
/*              pucBuf              pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/*              pfnFunc             callback function pointer               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT endpoint is busy                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_BufferSetRcvCtrlData( GRUSB_EndPointInfo*     ptEpInfo,
                                    UINT32                  ulDataSz,
                                    UINT8*                  pucBuf,
                                    VOID*                   pAplInfo,
                                    GRUSB_DataReceiveEnd    pfnFunc)
{
    INT                 iWrtP;
    GRUSB_TransmitInfo  *ptTrnsInfo;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x02, 0, 0 );

    /* check of writting point */
    iWrtP = GRLIB_Cyclic_CheckWrite( &ptEpInfo->tCycBufInfo);
    if (iWrtP < 0)                                  /* no point */
    {
        GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x02, 0x01, END_FUNC );

        return GRUSB_DEV_BUSY_ENDPOINT;             /* endpoint is busy */
    }

    ptTrnsInfo = (ptEpInfo->ptTrnsInfo + iWrtP);

    ptTrnsInfo->tRcvDtInfo.ulRcvBufferSz = ulDataSz;
    ptTrnsInfo->tRcvDtInfo.pucBuf        = pucBuf;
    ptTrnsInfo->tRcvDtInfo.pAplInfo      = pAplInfo;
    ptTrnsInfo->tRcvDtInfo.pfnFunc       = pfnFunc;

    GRLIB_Cyclic_IncWrite( &ptEpInfo->tCycBufInfo); /* renew a writting pointer */

    /* receiving request to HAL module */
    GRUSB_DEV_HALCtrlRcv( ptEpInfo);

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x02, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_DataSetSndData                                    */
/*                                                                          */
/* DESCRIPTION: Set sending data                                            */
/*                                                                          */
/* Func Code  : 03                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usDataSz        size of data                                */
/*              pucBuf          pointer of data buffer                      */
/*              i0Len           flag to add 0-length                        */
/*              pAplInfo        pointer of Application's information        */
/*              pfnFunc         callback function pointer                   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_DataSetSndData( GRUSB_EndPointInfo*       ptEpInfo,
                              UINT32                    ulDataSz,
                              UINT8*                    pucBuf,
                              BOOLEAN                   i0Len,
                              VOID*                     pAplInfo,
                              GRUSB_DataPushEnd         pfnFunc)
{
    INT     iRet;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x03, 0, 0 );

    iRet = _GRUSB_DEV_ReqSnd2Q( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x03, 0x01, END_FUNC );

        return iRet;
    }

    GRUSB_DEV_HALDataWrite( ptEpInfo );

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x03, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_DataSndAbort                                      */
/*                                                                          */
/* DESCRIPTION: Abort sending transmission                                  */
/*                                                                          */
/* Func Code  : 04                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_DataSndAbort( GRUSB_EndPointInfo*     ptEpInfo)
{
    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x04, 0, 0 );

    /* abort request */
    GRUSB_DEV_HALSndAbort( ptEpInfo );

    ptEpInfo->uInfo.tSndInfo.iFlag = GRUSB_TRS_NON_BUSY;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x04, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_DataRcvAbort                                      */
/*                                                                          */
/* DESCRIPTION: Abort receiving request                                     */
/*                                                                          */
/* Func Code  : 09                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpointe information                       */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_DataRcvAbort( GRUSB_EndPointInfo*     ptEpInfo)
{
    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x09, 0, 0 );

    /* abort request */
    GRUSB_DEV_HALRcvAbort( ptEpInfo);

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x09, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_BufferSetCtrlRcvData                              */
/*                                                                          */
/* DESCRIPTION: The buffer to receive data at the time of contorol trans-   */
/*              mission is setting.                                         */
/*                                                                          */
/* Func Code  : 05                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usDataSz        size of data                                */
/*              pucBuf          pointer of data buffer                      */
/*              pAplInfo        pointer of Application's information        */
/*              pfnFunc         calback function pointer                    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_BufferSetCtrlRcvData( GRUSB_EndPointInfo*     ptEpInfo,
                                    UINT32                  ulDataSz,
                                    UINT8*                  pucBuf,
                                    VOID*                   pAplInfo,
                                    GRUSB_DataReceiveEnd    pfnFunc)
{
    INT     iRet;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x05, 0, 0 );

    iRet = _GRUSB_DEV_ReqRcvBf2Q( ptEpInfo, ulDataSz, pucBuf, pAplInfo, pfnFunc);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x05, 0x01, END_FUNC );

        return iRet;
    }

    /* receive request to HAL module */
    GRUSB_DEV_HALCtrlRcv( ptEpInfo);

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x05, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_BufferSetRcvData                                  */
/*                                                                          */
/* DESCRIPTION: The buffer which receives receiving data is set up.         */
/*                                                                          */
/* Func Code  : 06                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usDataSz        size of data                                */
/*              pucBuf          pointer of data buffer                      */
/*              pAplInfo        pointer of Application's information        */
/*              pfnFunc         calback function pointer                    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_BufferSetRcvData( GRUSB_EndPointInfo*     ptEpInfo,
                                UINT32                  ulDataSz,
                                UINT8*                  pucBuf,
                                VOID*                   pAplInfo,
                                GRUSB_DataReceiveEnd    pfnFunc)
{
    INT     iRet;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x06, 0, 0 );

    iRet = _GRUSB_DEV_ReqRcvBf2Q( ptEpInfo, ulDataSz, pucBuf, pAplInfo, pfnFunc);
    if (iRet != GRUSB_DEV_SUCCESS)
    {
        GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x06, 0x01, END_FUNC );

        return iRet;
    }

    /* start operation */
    GRUSB_DEV_HALDataFIFORcvStart( ptEpInfo);

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x06, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_DEV_SndDataWriteToQueue                               */
/*                                                                          */
/* DESCRIPTION: The data to transmit is set up.                             */
/*                                                                          */
/* Func Code  : 0A                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usDataSz        size of data                                */
/*              pucBuf          pointer of data buffer                      */
/*              i0Len           flag to add 0-length                        */
/*              pAplInfo        pointer of Application's information        */
/*              pfnFunc         callback function pointer                   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT     GRUSB_DEV_SndDataWriteToQueue( GRUSB_EndPointInfo*  ptEpInfo,
                                       UINT32               ulDataSz,
                                       UINT8*               pucBuf,
                                       BOOLEAN              i0Len,
                                       VOID*                pAplInfo,
                                       GRUSB_DataPushEnd    pfnFunc )
{
    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x0A, 0, 0 );

    return( _GRUSB_DEV_ReqSnd2Q( ptEpInfo, ulDataSz, pucBuf, i0Len, pAplInfo, pfnFunc ));
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_DEV_ReqSnd2Q                                         */
/*                                                                          */
/* DESCRIPTION: The data to transmit is set up.                             */
/*                                                                          */
/* Func Code  : 07                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usDataSz        size of data                                */
/*              pucBuf          pointer of data buffer                      */
/*              i0Len           flag to add 0-length                        */
/*              pAplInfo        pointer of Application's information        */
/*              pfnFunc         callback function pointer                   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_DEV_ReqSnd2Q( GRUSB_EndPointInfo*      ptEpInfo,
                               UINT32                   ulDataSz,
                               UINT8*                   pucBuf,
                               BOOLEAN                  i0Len,
                               VOID*                    pAplInfo,
                               GRUSB_DataPushEnd        pfnFunc)
{
    INT                 iWrtP;
    GRUSB_SndDataInfo*  ptSndDtInfo;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x07, 0, 0 );

    /* check cyclic buffer information to write */
    iWrtP = GRLIB_Cyclic_CheckWrite( &ptEpInfo->tCycBufInfo );
    if (iWrtP == GRLIB_NONBUFFER)
    {
        GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x07, 0x01, END_FUNC );

        return GRUSB_DEV_BUSY_ENDPOINT;             /* under endpoint use */
    }

    ptSndDtInfo = &(ptEpInfo->ptTrnsInfo[iWrtP].tSndDtInfo);

    ptSndDtInfo->ulDataSz = ulDataSz;
    ptSndDtInfo->pucBuf   = pucBuf;
    ptSndDtInfo->i0Len    = i0Len;
    ptSndDtInfo->pAplInfo = pAplInfo;
    ptSndDtInfo->pfnFunc  = pfnFunc;

    GRLIB_Cyclic_IncWrite( &ptEpInfo->tCycBufInfo); /* renew a writting pointer */

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x07, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : RcvBufferWriteToQueue                                       */
/*                                                                          */
/* DESCRIPTION: The buffer which receives receiving data is set up.         */
/*                                                                          */
/* Func Code  : 0B                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usDataSz        size of data                                */
/*              pucBuf          pointer of data buffer                      */
/*              pAplInfo        pointer of Application's information        */
/*              pfnFunc         calback function pointer                    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_DEV_RcvBufferWriteToQueue( GRUSB_EndPointInfo*    ptEpInfo,
                                     UINT32                 ulDataSz,
                                     UINT8*                 pucBuf,
                                     VOID*                  pAplInfo,
                                     GRUSB_DataReceiveEnd   pfnFunc )
{
    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x0B, 0, 0 );

    return(_GRUSB_DEV_ReqRcvBf2Q( ptEpInfo, ulDataSz, pucBuf, pAplInfo, pfnFunc ));
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_DEV_ReqRcvBf2Q                                       */
/*                                                                          */
/* DESCRIPTION: The buffer which receives receiving data is set up.         */
/*                                                                          */
/* Func Code  : 08                                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo        endpoint information                        */
/*              usDataSz        size of data                                */
/*              pucBuf          pointer of data buffer                      */
/*              pAplInfo        pointer of Application's information        */
/*              pfnFunc         calback function pointer                    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_BUSY_ENDPOINT Endpoint is busy                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_DEV_ReqRcvBf2Q( GRUSB_EndPointInfo*        ptEpInfo,
                                 UINT32                     ulDataSz,
                                 UINT8*                     pucBuf,
                                 VOID*                      pAplInfo,
                                 GRUSB_DataReceiveEnd       pfnFunc)
{
    INT                 iWrtP;
    GRUSB_TransmitInfo* ptTrnsInfo;

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x08, 0, 0 );

    /* check cyclic byffer information to write */
    iWrtP = GRLIB_Cyclic_CheckWrite( &ptEpInfo->tCycBufInfo);
    if (iWrtP < 0)
    {
        GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x08, 0x01, END_FUNC );

        return GRUSB_DEV_BUSY_ENDPOINT;             /* under endpoint use */
    }

    ptTrnsInfo = (ptEpInfo->ptTrnsInfo + iWrtP);

    ptTrnsInfo->tRcvDtInfo.ulRcvBufferSz = ulDataSz;
    ptTrnsInfo->tRcvDtInfo.pucBuf        = pucBuf;
    ptTrnsInfo->tRcvDtInfo.pAplInfo      = pAplInfo;
    ptTrnsInfo->tRcvDtInfo.pfnFunc       = pfnFunc;

    GRLIB_Cyclic_IncWrite( &ptEpInfo->tCycBufInfo); /* renew a writting pointer */

    GRTRS_DBG_TRACE( GRDBG_PERI_TRS, 0x08, 0, END_FUNC );

    return GRUSB_DEV_SUCCESS;
}
