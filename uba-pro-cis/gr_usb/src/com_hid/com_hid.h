/****************************************************************************/
/*                                                                          */
/*               Copyright(C) 2003-2019 Grape Systems, Inc.                 */
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
/*      com_hid.h                                                 1.00      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file performs Abstract Control Model of HID Class              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME       DATE        REMARKS                                         */
/*                                                                          */
/*   JCM-HQ     2022/06/21  V1.00                                           */
/*                          Created initial version                         */
/*                                                                          */
/****************************************************************************/
#ifndef     _COM_HID_H_
#define     _COM_HID_H_

/**** INCLUDE FILES *********************************************************/
#include    "com_hid_def.h"
#include    "grusbtyp.h"
#include    "perid.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* Error code */
#define GRUSB_COMD_HID_SUCCESS              (0)             /* Successfully end         */
#define GRUSB_COMD_HID_ERROR                (-1)            /* Unusual end              */
#define GRUSB_COMD_HID_DESC_ERROR           (-2)            /* Discriptor setting error */
#define GRUSB_COMD_HID_SIZE_ERROR           (-3)            /* Size error               */
#define GRUSB_COMD_HID_PRM_ERR              (-4)            /* Parameter error          */

/* Completion status */
#define GRUSB_COMD_HID_COMPLETE             GRUSB_DEV_SUCCESS       /* Complete                      */
#define GRUSB_COMD_HID_CANCEL               GRUSB_DEV_CANCELED      /* Cancel                        */
#define GRUSB_COMD_HID_TRAN_CANCEL          GRUSB_DEV_TRAN_CANCELED /* Cancel(Transmission Finished) */

/* Network connection state */
#define GRUSB_COMD_HID_DISCONNECT           (0)             /* Network Disconnection    */
#define GRUSB_COMD_HID_CONNECT              (1)             /* Network Connection       */

/* USB connection state */
#define GRUSB_COMD_HID_DISC                 (0) /* Disconnection State�iADDRESS State   */
                                            /* or Disconnection State)              */
#define GRUSB_COMD_HID_CON                  (1) /* Connection State�iCONFIGURED State�j */

typedef VOID (*GRUSB_COMD_HID_ConnStat)( INT );
typedef VOID (*GRUSB_COMD_HID_SendEncapsulatedCmd)( UINT32, UINT8* );
typedef VOID (*GRUSB_COMD_HID_Get)( UINT16 );
typedef VOID (*GRUSB_COMD_HID_Set)( UINT32, UINT8* );
typedef VOID (*GRUSB_COMD_HID_ClearCommFeature)( VOID );
typedef VOID (*GRUSB_COMD_HID_SetControlLineState)( UINT16 );
typedef VOID (*GRUSB_COMD_HID_SendBreak)( UINT16 );
typedef VOID (*GRUSB_COMD_HID_Notification)( VOID );
#ifdef GRCOMD_HID_COMP_STATUS_USE
typedef VOID (*GRUSB_COMD_HID_TrnsData)( UINT32, UINT8*, VOID*, INT );
#else
typedef VOID (*GRUSB_COMD_HID_TrnsData)( UINT32, UINT8*, VOID* );
#endif
typedef VOID (*GRUSB_COMD_HID_SetControlLineState)( UINT16 ); //2022-08-05

/* Structure of the Initial parameter of Callback functions */
typedef struct grusb_comd_initinfo_tag
{
    GRUSB_COMD_HID_ConnStat                     pfnConnStat;
    GRUSB_COMD_HID_SendEncapsulatedCmd          pfnSendEncapsulatedCmd;
    GRUSB_COMD_HID_Get                          pfnGetEncapsulatedRes;
    GRUSB_COMD_HID_Set                          pfnSetCommFeature;
    GRUSB_COMD_HID_Get                          pfnGetCommFeature;
    GRUSB_COMD_HID_ClearCommFeature             pfnClearCommFeature;
    GRUSB_COMD_HID_Set                          pfnSetLineCoding;
    GRUSB_COMD_HID_Get                          pfnGetLineCoding;
    GRUSB_COMD_HID_SetControlLineState          pfnSetControlLineState;
    GRUSB_COMD_HID_SendBreak                    pfnSendBreak;
    GRUSB_COMD_HID_Notification                 pfnNetworkConnection;
    GRUSB_COMD_HID_Notification                 pfnResponseAvailable;
    GRUSB_COMD_HID_Notification                 pfnSerialState;
    GRUSB_COMD_HID_TrnsData                     pfnSendData;
    GRUSB_COMD_HID_TrnsData                     pfnReciveData;
    GRUSB_COMD_HID_SetControlLineState          pfnHidDetach;   //2022-08-05
} GRUSB_COMD_HID_INITINFO;

//#if 0  /* Unsupported */
/**** EXTERNAL FUNCTION PROTOTYPES ******************************************/
INT GRUSB_COMD_HID_Init( GRUSB_COMD_HID_INITINFO*   ptInitInfo );
INT GRUSB_COMD_HID_SetSerialNumber( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars );
INT GRUSB_COMD_HID_Notification_NetworkConnection( UINT16 usConn );
INT GRUSB_COMD_HID_Notification_ResponseAvailable( VOID );
INT GRUSB_COMD_HID_Notification_SerialState( UINT16 usStatBmap );
INT GRUSB_COMD_HID_Set_GetEncapsulatedResponse( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_HID_Set_GetCommFeature( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_HID_Set_GetLineCoding( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_HID_SendData( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_HID_ReciveData( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_HID_Get_ConnState( void );
//#endif /* Unsupported */

INT GRUSB_COMD_HID_Init2( GRUSB_COMD_HID_INITINFO*   ptInitInfo );
INT GRUSB_COMD_HID_SetSerialNumber2( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars );
INT GRUSB_COMD_HID_SetInterface2( UINT16* pusInterface, UINT8 ucInterfaceChars );
INT GRUSB_COMD_HID_Notification_NetworkConnection2( UINT16 usConn );
INT GRUSB_COMD_HID_Notification_ResponseAvailable2( VOID );
INT GRUSB_COMD_HID_Notification2( UINT32 ulSize, UINT8* pucData);
INT GRUSB_COMD_HID_Notification_SerialState2( UINT16 usStatBmap );
INT GRUSB_COMD_HID_Set_GetEncapsulatedResponse2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_HID_Set_GetCommFeature2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_HID_Set_GetLineCoding2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_HID_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_HID_ReciveData2( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
#endif  /* _COM_HID_H_ */
