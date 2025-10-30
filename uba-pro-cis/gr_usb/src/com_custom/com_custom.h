/****************************************************************************/
/*                                                                          */
/*                   Copyright(C) 2021 Grape Systems, Inc.                  */
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
/*      com_custom.h                                              1.00      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file performs Abstract Control Model of Communication Device   */
/*      Class. (Custom version with multiple COM ports.)                    */
/*      Based on GR-USB/DEVICE Communication Function Driver V1.42.         */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME        DATE        REMARKS                                        */
/*                                                                          */
/*   T.Yamaguchi 2021/09/08  V1.00                                          */
/*                           Created initial version                        */
/*                                                                          */
/****************************************************************************/
#ifndef     _COM_CUSTOM_H_
#define     _COM_CUSTOM_H_

/**** INCLUDE FILES *********************************************************/
#include    "com_custom_def.h"
#include    "grusbtyp.h"
#include    "perid.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* Error code */
#define GRUSB_COMD_CUSTOM_SUCCESS       (0)     /* Successfully end         */
#define GRUSB_COMD_CUSTOM_ERROR         (-1)    /* Unusual end              */
#define GRUSB_COMD_CUSTOM_DESC_ERROR    (-2)    /* Discriptor setting error */
#define GRUSB_COMD_CUSTOM_SIZE_ERROR    (-3)    /* Size error               */
#define GRUSB_COMD_CUSTOM_PRM_ERR       (-4)    /* Parameter error          */

/* Completion status */
#define GRUSB_COMD_CUSTOM_COMPLETE      GRUSB_DEV_SUCCESS       /* Complete                      */
#define GRUSB_COMD_CUSTOM_CANCEL        GRUSB_DEV_CANCELED      /* Cancel                        */
#define GRUSB_COMD_CUSTOM_TRAN_CANCEL   GRUSB_DEV_TRAN_CANCELED /* Cancel(Transmission Finished) */

/* Network connection state */
#define GRUSB_COMD_CUSTOM_DISCONNECT    (0)     /* Network Disconnection    */
#define GRUSB_COMD_CUSTOM_CONNECT       (1)     /* Network Connection       */

/* USB connection state */
#define GRUSB_COMD_CUSTOM_DISC          (0)     /* Disconnection State�iADDRESS State   */
                                                /* or Disconnection State)              */
#define GRUSB_COMD_CUSTOM_CON           (1)     /* Connection State�iCONFIGURED State�j */

typedef VOID (*GRUSB_COMD_CUSTOM_ConnStat)( INT );
typedef VOID (*GRUSB_COMD_CUSTOM_SendEncapsulatedCmd)( UINT32, UINT32, UINT8* );
typedef VOID (*GRUSB_COMD_CUSTOM_Get)( UINT32, UINT16 );
typedef VOID (*GRUSB_COMD_CUSTOM_Set)( UINT32, UINT32, UINT8* );
typedef VOID (*GRUSB_COMD_CUSTOM_ClearCommFeature)( UINT32 );
typedef VOID (*GRUSB_COMD_CUSTOM_SetControlLineState)( UINT32, UINT16 );
typedef VOID (*GRUSB_COMD_CUSTOM_SendBreak)( UINT32, UINT16 );
typedef VOID (*GRUSB_COMD_CUSTOM_Notification)( UINT32 );
#ifdef GRCOMD_CUSTOM_COMP_STATUS_USE
typedef VOID (*GRUSB_COMD_CUSTOM_TrnsData)( UINT32, UINT32, UINT8*, VOID*, INT );
#else
typedef VOID (*GRUSB_COMD_CUSTOM_TrnsData)( UINT32, UINT32, UINT8*, VOID* );
#endif
typedef struct {
	UINT8 LSB;
	UINT8 MSB;
} *PGRUSB_PID,GRUSB_PID;

/* Structure of the Initial parameter of Callback functions */
typedef struct grusb_comd_custom_initinfo_tag
{
    GRUSB_COMD_CUSTOM_ConnStat                     pfnConnStat;
    GRUSB_COMD_CUSTOM_SendEncapsulatedCmd          pfnSendEncapsulatedCmd;
    GRUSB_COMD_CUSTOM_Get                          pfnGetEncapsulatedRes;
    GRUSB_COMD_CUSTOM_Set                          pfnSetCommFeature;
    GRUSB_COMD_CUSTOM_Get                          pfnGetCommFeature;
    GRUSB_COMD_CUSTOM_ClearCommFeature             pfnClearCommFeature;
    GRUSB_COMD_CUSTOM_Set                          pfnSetLineCoding;
    GRUSB_COMD_CUSTOM_Get                          pfnGetLineCoding;
    GRUSB_COMD_CUSTOM_SetControlLineState          pfnSetControlLineState;
    GRUSB_COMD_CUSTOM_SendBreak                    pfnSendBreak;
    GRUSB_COMD_CUSTOM_Notification                 pfnNetworkConnection;
    GRUSB_COMD_CUSTOM_Notification                 pfnResponseAvailable;
    GRUSB_COMD_CUSTOM_Notification                 pfnSerialState;
    GRUSB_COMD_CUSTOM_TrnsData                     pfnSendData;
    GRUSB_COMD_CUSTOM_TrnsData                     pfnReciveData;
    PGRUSB_PID								       pstUsbPid;
} GRUSB_COMD_CUSTOM_INITINFO;

/**** EXTERNAL FUNCTION PROTOTYPES ******************************************/
INT GRUSB_COMD_CUSTOM_Init( GRUSB_COMD_CUSTOM_INITINFO*   ptInitInfo );
INT GRUSB_COMD_CUSTOM_SetSerialNumber( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars );
INT GRUSB_COMD_CUSTOM_Notification_NetworkConnection( UINT32 ulDcom, UINT16 usConn );
INT GRUSB_COMD_CUSTOM_Notification_ResponseAvailable( UINT32 ulDcom );
INT GRUSB_COMD_CUSTOM_Notification_SerialState( UINT32 ulDcom, UINT16 usStatBmap );
INT GRUSB_COMD_CUSTOM_Set_GetEncapsulatedResponse( UINT32 ulDcom, UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_CUSTOM_Set_GetCommFeature( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_CUSTOM_Set_GetLineCoding( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_CUSTOM_SendData( UINT32 ulDcom, UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_CUSTOM_ReciveData( UINT32 ulDcom, UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_CUSTOM_Get_ConnState( void );


INT GRUSB_COMD_CUSTOM_Init2( GRUSB_COMD_CUSTOM_INITINFO*   ptInitInfo );
INT GRUSB_COMD_CUSTOM_SetSerialNumber2( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars );
INT GRUSB_COMD_CUSTOM_Notification_NetworkConnection2( UINT16 usConn );
INT GRUSB_COMD_CUSTOM_Notification_ResponseAvailable2( VOID );
INT GRUSB_COMD_CUSTOM_Notification_SerialState2( UINT16 usStatBmap );
INT GRUSB_COMD_CUSTOM_Set_GetEncapsulatedResponse2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_CUSTOM_Set_GetCommFeature2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_CUSTOM_Set_GetLineCoding2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_CUSTOM_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_CUSTOM_ReciveData2( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
#endif  /* _COM_CUSTOM_H_ */
