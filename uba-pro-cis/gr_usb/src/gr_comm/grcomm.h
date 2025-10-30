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
/*      grcomm.h                                                  1.42      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file perfoems Abstract Control Model of Communication Device   */
/*      Class.                                                              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME       DATE        REMARKS                                         */
/*                                                                          */
/*   K.Takagi   2003/09/12  V0.01                                           */
/*                          Created initial version                         */
/*   K.Takagi   2003/10/17  V1.00M                                          */
/*                          1st Release version                             */
/*   K.Takagi   2003/11/17  V1.10M                                          */
/*                          The processing to state change is corrected     */
/*   M.Suzuki   2004/05/27  V1.11                                           */
/*                          version is updated                              */
/*   S.Tomizawa 2005/08/02  V1.20                                           */
/*                          version was updated                             */
/*   K.Takagi   2006/08/31  V1.22                                           */
/*                          version was updated                             */
/*   K.Handa    2006/09/26  V1.23                                           */
/*                          version was updated                             */
/*   K.handa    2006/12/27  V1.30                                           */
/*                          version was updated                             */
/*   K.handa    2007/06/12  V1.31                                           */
/*                          version was updated                             */
/*   M.Suzuki   2015/03/04  V1.40                                           */
/*                          Added the following function to set the serial  */
/*                          number of the user-specified.                   */
/*                          - GRUSB_COMD_SetSerialNumber                    */
/*                          - GRUSB_COMD_SetSerialNumber2                   */
/*                          Added the following error code.                 */
/*                          - GRUSB_COMD_PRM_ERR                            */
/*   M.Suzuki   2019/03/26  V1.42                                           */
/*                          Added the following completion status.          */
/*                          - GRUSB_COMD_COMPLETE                           */
/*                          - GRUSB_COMD_CANCEL                             */
/*                          - GRUSB_COMD_TRAN_CANCEL                        */
/*                          Added transfer completion status to following   */
/*                          callback function parameter.                    */
/*                          - GRUSB_COMD_TrnsData                           */
/*                                                                          */
/****************************************************************************/
#ifndef     _GRCOMM_H_
#define     _GRCOMM_H_

/**** INCLUDE FILES *********************************************************/
#include    "grusbtyp.h"
#include    "perid.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* Error code */
#define GRUSB_COMD_SUCCESS              (0)             /* Successfully end         */
#define GRUSB_COMD_ERROR                (-1)            /* Unusual end              */
#define GRUSB_COMD_DESC_ERROR           (-2)            /* Discriptor setting error */
#define GRUSB_COMD_SIZE_ERROR           (-3)            /* Size error               */
#define GRUSB_COMD_PRM_ERR              (-4)            /* Parameter error          */

/* Completion status */
#define GRUSB_COMD_COMPLETE             GRUSB_DEV_SUCCESS       /* Complete                      */
#define GRUSB_COMD_CANCEL               GRUSB_DEV_CANCELED      /* Cancel                        */
#define GRUSB_COMD_TRAN_CANCEL          GRUSB_DEV_TRAN_CANCELED /* Cancel(Transmission Finished) */

/* Network connection state */
#define GRUSB_COMD_DISCONNECT           (0)             /* Network Disconnection    */
#define GRUSB_COMD_CONNECT              (1)             /* Network Connection       */

/* USB connection state */
#define GRUSB_COMD_DISC                 (0) /* Disconnection State�iADDRESS State   */
                                            /* or Disconnection State)              */
#define GRUSB_COMD_CON                  (1) /* Connection State�iCONFIGURED State�j */

typedef VOID (*GRUSB_COMD_ConnStat)( INT );
typedef VOID (*GRUSB_COMD_SendEncapsulatedCmd)( UINT32, UINT8* );
typedef VOID (*GRUSB_COMD_Get)( UINT16 );
typedef VOID (*GRUSB_COMD_Set)( UINT32, UINT8* );
typedef VOID (*GRUSB_COMD_ClearCommFeature)( VOID );
typedef VOID (*GRUSB_COMD_SetControlLineState)( UINT16 );
typedef VOID (*GRUSB_COMD_SendBreak)( UINT16 );
typedef VOID (*GRUSB_COMD_Notification)( VOID );
#ifdef GRCOMD_COMP_STATUS_USE
typedef VOID (*GRUSB_COMD_TrnsData)( UINT32, UINT8*, VOID*, INT );
#else
typedef VOID (*GRUSB_COMD_TrnsData)( UINT32, UINT8*, VOID* );
#endif
typedef struct {
	UINT8 LSB;
	UINT8 MSB;
} *PGRUSB_PID,GRUSB_PID;

/* Structure of the Initial parameter of Callback functions */
typedef struct grusb_comd_initinfo_tag
{
    GRUSB_COMD_ConnStat                     pfnConnStat;
    GRUSB_COMD_SendEncapsulatedCmd          pfnSendEncapsulatedCmd;
    GRUSB_COMD_Get                          pfnGetEncapsulatedRes;
    GRUSB_COMD_Set                          pfnSetCommFeature;
    GRUSB_COMD_Get                          pfnGetCommFeature;
    GRUSB_COMD_ClearCommFeature             pfnClearCommFeature;
    GRUSB_COMD_Set                          pfnSetLineCoding;
    GRUSB_COMD_Get                          pfnGetLineCoding;
    GRUSB_COMD_SetControlLineState          pfnSetControlLineState;
    GRUSB_COMD_SendBreak                    pfnSendBreak;
    GRUSB_COMD_Notification                 pfnNetworkConnection;
    GRUSB_COMD_Notification                 pfnResponseAvailable;
    GRUSB_COMD_Notification                 pfnSerialState;
    GRUSB_COMD_TrnsData                     pfnSendData;
    GRUSB_COMD_TrnsData                     pfnReciveData;
    PGRUSB_PID								pstUsbPid;
} GRUSB_COMD_INITINFO;

/**** EXTERNAL FUNCTION PROTOTYPES ******************************************/
INT GRUSB_COMD_Init( GRUSB_COMD_INITINFO*   ptInitInfo );
INT GRUSB_COMD_SetSerialNumber( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars );
INT GRUSB_COMD_Notification_NetworkConnection( UINT16 usConn );
INT GRUSB_COMD_Notification_ResponseAvailable( VOID );
INT GRUSB_COMD_Notification_SerialState( UINT16 usStatBmap );
INT GRUSB_COMD_Set_GetEncapsulatedResponse( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_Set_GetCommFeature( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_Set_GetLineCoding( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_SendData( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_ReciveData( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_Get_ConnState( void );


INT GRUSB_COMD_Init2( GRUSB_COMD_INITINFO*   ptInitInfo );
INT GRUSB_COMD_SetSerialNumber2( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars );
INT GRUSB_COMD_Notification_NetworkConnection2( UINT16 usConn );
INT GRUSB_COMD_Notification_ResponseAvailable2( VOID );
INT GRUSB_COMD_Notification_SerialState2( UINT16 usStatBmap );
INT GRUSB_COMD_Set_GetEncapsulatedResponse2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_Set_GetCommFeature2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_Set_GetLineCoding2( UINT32 ulSize, UINT8* pucData );
INT GRUSB_COMD_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
INT GRUSB_COMD_ReciveData2( UINT32 ulSize, UINT8* pucData, VOID* pAplInfo );
#endif  /* _GRCOMM_H_ */
