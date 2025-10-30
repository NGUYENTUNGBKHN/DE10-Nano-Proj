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
/*      peri_trs.h                                              1.42        */
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
/*  Y.Sato      2003/07/24  version 0.93, add API function                  */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              version is updated.                         */
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
#ifndef __PERI_TRS_H__
#define __PERI_TRS_H__

INT     GRUSB_DEV_DataSetSndCtrlData( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                        BOOLEAN, VOID*, GRUSB_DataPushEnd );
INT     GRUSB_DEV_BufferSetRcvCtrlData( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                        VOID*, GRUSB_DataReceiveEnd );
INT     GRUSB_DEV_DataSetSndData( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                    BOOLEAN, VOID*, GRUSB_DataPushEnd );
INT     GRUSB_DEV_SndDataWriteToQueue( GRUSB_EndPointInfo *, UINT32 , UINT8 *,
                                        BOOLEAN , VOID *, GRUSB_DataPushEnd );
INT     GRUSB_DEV_DataSndAbort( GRUSB_EndPointInfo* );
INT     GRUSB_DEV_DataRcvAbort( GRUSB_EndPointInfo* );
INT     GRUSB_DEV_BufferSetCtrlRcvData( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                        VOID*, GRUSB_DataReceiveEnd );
INT     GRUSB_DEV_BufferSetRcvData( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                    VOID*, GRUSB_DataReceiveEnd );
INT     GRUSB_DEV_RcvBufferWriteToQueue( GRUSB_EndPointInfo *, UINT32 , UINT8 *,
                                         VOID *, GRUSB_DataReceiveEnd );


INT     GRUSB_DEV_DataSetSndCtrlData2( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                        BOOLEAN, VOID*, GRUSB_DataPushEnd );
INT     GRUSB_DEV_BufferSetRcvCtrlData2( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                        VOID*, GRUSB_DataReceiveEnd );
INT     GRUSB_DEV_DataSetSndData2( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                    BOOLEAN, VOID*, GRUSB_DataPushEnd );
INT     GRUSB_DEV_SndDataWriteToQueue2( GRUSB_EndPointInfo *, UINT32 , UINT8 *,
                                        BOOLEAN , VOID *, GRUSB_DataPushEnd );
INT     GRUSB_DEV_DataSndAbort2( GRUSB_EndPointInfo* );
INT     GRUSB_DEV_DataRcvAbort2( GRUSB_EndPointInfo* );
INT     GRUSB_DEV_BufferSetCtrlRcvData2( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                        VOID*, GRUSB_DataReceiveEnd );
INT     GRUSB_DEV_BufferSetRcvData2( GRUSB_EndPointInfo*, UINT32, UINT8*,
                                    VOID*, GRUSB_DataReceiveEnd );
INT     GRUSB_DEV_RcvBufferWriteToQueue2( GRUSB_EndPointInfo *, UINT32 , UINT8 *,
                                         VOID *, GRUSB_DataReceiveEnd );

#endif  /* __PERI_TRS_H__ */
