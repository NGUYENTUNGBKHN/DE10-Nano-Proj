/****************************************************************************/
/*                                                                          */
/*               Copyright(C) 2008 Grape Systems, Inc.                      */
/*                            All Rights Reserved                           */
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
/*      comm_wrap.h                                               1.00      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file perfoems Vendor Device Class.                             */
/*                                                                          */
/* FANCTIONS:                                                               */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME        DATE       REMARKS                                         */
/*                                                                          */
/*   K.Jinnouchi 2017/11/06 V1.00                                           */
/*                          Created initial version                         */
/*                                                                          */
/****************************************************************************/
#ifndef     _COMM_WRAP_H_
#define     _COMM_WRAP_H_

/**** INCLUDE FILES *********************************************************/
#include    "grusbtyp.h"
#include    "perid.h"
#include    "grcomm.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* Structure of the Initial parameter of Callback functions */
typedef struct grusb_comd_wrap_initinfo_tag
{
    GRUSB_COMD_TrnsData                     pfnRecvCommand;
    GRUSB_COMD_TrnsData                     pfnSendStatus;
    GRUSB_COMD_TrnsData                     pfnRecvData;
    GRUSB_COMD_TrnsData                     pfnSendData;
    GRUSB_COMD_Notification                 pfnSuspend;
    GRUSB_COMD_Notification                 pfnResume;
} GRUSB_COMD_WRAP_INITINFO;

/* 0 length packet option (TRUE:add, FALSE:not add) */
#define GRUSB_ADD_0LENPKT   GRUSB_TRUE

/**** EXTERNAL FUNCTION PROTOTYPES ******************************************/
//#ifdef __cplusplus
//extern "C" {
//#endif
VOID GRUSB_COMD_WRAP_Init( GRUSB_COMD_WRAP_INITINFO* ptInitInfo );
INT GRUSB_COMD_WRAP_RecvCommand( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_SendStatus( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_RecvData( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_SendData( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_RecvAbort( VOID );
INT GRUSB_COMD_WRAP_SendAbort( VOID );
//INT GRUSB_VNDD_AbortRecvData( VOID );
//INT GRUSB_VNDD_AbortSendData( VOID );
//INT GRUSB_VNDD_PcutWaitComplete( VOID );
//#ifdef __cplusplus
//}
//#endif


VOID GRUSB_COMD_WRAP_Init2( GRUSB_COMD_WRAP_INITINFO* ptInitInfo );
INT GRUSB_COMD_WRAP_RecvCommand2( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_SendStatus2( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_RecvData2( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_SendData2( UINT32 iSize, UINT8* pcuData, VOID* pInfo );
INT GRUSB_COMD_WRAP_RecvAbort2( VOID );
INT GRUSB_COMD_WRAP_SendAbort2( VOID );

#endif  /* _COMM_WRAP_H_ */
