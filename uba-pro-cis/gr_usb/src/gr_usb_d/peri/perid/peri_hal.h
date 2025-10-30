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
/*      peri_hal.h                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      Hardware Abstraction Layer Module for peripheral                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  A.Yoshida   2002/12/16  Created initial version 0.00                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
/*  K.Takagi    2003/07/16  version 0.92, modified variable names.          */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              added following functions.                  */
/*                              - GRUSB_DEV_HALCallbackCmpSetIf             */
/*                              - GRUSB_DEV_HALCallbackCmpSetConfig         */
/*                              - GRUSB_DEV_HALReqPullupRegister            */
/*                              - GRUSB_DEV_HALCallbackCmpSetIf2            */
/*                              - GRUSB_DEV_HALCallbackCmpSetConfig2        */
/*                              - GRUSB_DEV_HALReqPullupRegister2           */
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
#ifndef     _PERI_HAL_
#define     _PERI_HAL_

#include    "grusbtyp.h"
#include    "peri_prm.h"

VOID    GRUSB_DEV_HALInit(VOID);
VOID    GRUSB_DEV_HALConnectController(VOID);
VOID    GRUSB_DEV_HALDisonnectController(VOID);
VOID    GRUSB_DEV_HALDataWrite( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALDataFIFORcvStart( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALCtrlSnd( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALCtrlRcv( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALSetStallState( INT, INT );
VOID    GRUSB_DEV_HALSetAddress( UINT16 );
VOID    GRUSB_DEV_HALCallbackBusReset( GRUSB_Notice );
VOID    GRUSB_DEV_HALCallbackSuspend( GRUSB_Notice );
VOID    GRUSB_DEV_HALCallbackResume( GRUSB_Notice );
VOID    GRUSB_DEV_HALCallbackCtrTransferCancel( GRUSB_CtrTransferCancel );
VOID    GRUSB_DEV_HALCallbackCtrReceiveCancel( GRUSB_CtrReceiveCancel );
VOID    GRUSB_DEV_HALTogleClear( INT );
VOID    GRUSB_DEV_HALRmtWkup(VOID);
VOID    GRUSB_DEV_HALSndAbort( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALRcvAbort( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALCallbackCmpSetIf( GRUSB_Notice);
VOID    GRUSB_DEV_HALCallbackCmpSetConfig( GRUSB_Notice);
VOID    GRUSB_DEV_HALReqPullupRegister( INT );
VOID    GRUSB_DEV_HALSetTestMode( UINT16 );


VOID    GRUSB_DEV_HALInit2(VOID);
VOID    GRUSB_DEV_HALConnectController2(VOID);
VOID    GRUSB_DEV_HALDisonnectController2(VOID);
VOID    GRUSB_DEV_HALDataWrite2( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALDataFIFORcvStart2( GRUSB_EndPointInfo* );
//VOID    GRUSB_DEV_HALCtrlSnd2( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALCtrlRcv2( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALSetStallState2( INT, INT );
VOID    GRUSB_DEV_HALSetAddress2( UINT16 );
VOID    GRUSB_DEV_HALCallbackBusReset2( GRUSB_Notice );
VOID    GRUSB_DEV_HALCallbackSuspend2( GRUSB_Notice );
VOID    GRUSB_DEV_HALCallbackResume2( GRUSB_Notice );
VOID    GRUSB_DEV_HALCallbackCtrTransferCancel2( GRUSB_CtrTransferCancel );
VOID    GRUSB_DEV_HALCallbackCtrReceiveCancel2( GRUSB_CtrReceiveCancel );
VOID    GRUSB_DEV_HALTogleClear2( INT );
VOID    GRUSB_DEV_HALRmtWkup2(VOID);
VOID    GRUSB_DEV_HALSndAbort2( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALRcvAbort2( GRUSB_EndPointInfo* );
VOID    GRUSB_DEV_HALCallbackCmpSetIf2( GRUSB_Notice);
VOID    GRUSB_DEV_HALCallbackCmpSetConfig2( GRUSB_Notice);
VOID    GRUSB_DEV_HALReqPullupRegister2( INT );
VOID    GRUSB_DEV_HALSetTestMode2( UINT16 );

#endif      /* _PERI_HAL_ */
