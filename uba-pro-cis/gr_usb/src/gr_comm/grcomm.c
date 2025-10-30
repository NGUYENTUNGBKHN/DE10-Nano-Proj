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
/*      grcomm.c                                                  1.42      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file performs Abstract Control Model of Communication Device   */
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
/*                          modified following functions                    */
/*                          -_GRUSB_COMD_CbDisConn                          */
/*                          -_GRUSB_COMD_CbConn                             */
/*   M.Suzuki   2004/05/27  V1.11                                           */
/*                          version is updated                              */
/*   S.Tomizawa 2005/08/02  V1.20                                           */
/*                          follow new perid(support USB 2.0 Hi-Speed)      */
/*   K.Handa    2005/12/28  V1.21                                           */
/*                          added endpoint cancel on disconnect             */
/*   K.Takagi   2006/08/31  V1.22                                           */
/*                          The composition of the end point was changed.   */
/*                          So following functions were modified.           */
/*                          - _GRUSB_COMD_Setting                           */
/*                          - _GRUSB_COMD_GetCnfgDescFS                     */
/*                          - GRUSB_COMD_Notification_NetworkConnection     */
/*                          - GRUSB_COMD_Notification_ResponseAvailable     */
/*                          - GRUSB_COMD_Notification_SerialState           */
/*   K.handa    2006/09/26  V1.23                                           */
/*                          Modified Hi-Speed descriptor endpoint defines   */
/*                          - _GRUSB_COMD_GetCnfgDescHS                     */
/*   K.handa    2006/12/27  V1.30                                           */
/*                          Modified Big Endian Problem.                    */
/*                          - GRUSB_COMD_Notification_NetworkConnection     */
/*                          - GRUSB_COMD_Notification_ResponseAvailable     */
/*                          - GRUSB_COMD_Notification_SerialState           */
/*   K.handa    2007/06/12  V1.31                                           */
/*                          Re-Modified Big Endian Problem.                 */
/*                          - GRUSB_COMD_Notification_NetworkConnection     */
/*                          - GRUSB_COMD_Notification_ResponseAvailable     */
/*                          - GRUSB_COMD_Notification_SerialState           */
/*   M.Suzuki   2010/11/10  V1.32                                           */
/*                          Fixed Notification data defines.                */
/*                          - GRUSB_COMD_Notification_NetworkConnection     */
/*                          - GRUSB_COMD_Notification_ResponseAvailable     */
/*                          - GRUSB_COMD_Notification_SerialState           */
/*   M.Suzuki   2015/03/04  V1.40                                           */
/*                          Added the following function to set the serial  */
/*                          number of the user-specified.                   */
/*                          - GRUSB_COMD_SetSerialNumber                    */
/*                          Modified the following compile options name.    */
/*                          - (before) GRUSB_DEV_NOT_HS                     */
/*                            (after)  GRUSB_DEV_FIXED_FS                   */
/*                          Modified the parameters setup processing of     */
/*                          the following functions.                        */
/*                          - GRUSB_COMD_Notification_NetworkConnection     */
/*                          - GRUSB_COMD_Notification_ResponseAvailable     */
/*                          - GRUSB_COMD_Notification_SerialState           */
/*   M.Suzuki   2017/11/24  V1.41                                           */
/*                          Modified the macro definition for debug log.    */
/*                          - GRCOMD_LOG                                    */
/*   M.Suzuki   2019/03/26  V1.42                                           */
/*                          Added transfer complete status to parameters of */
/*                          send / receive completion callback function to  */
/*                          notify to upper layer.                          */
/*                                                                          */
/****************************************************************************/
#include    "comm_def.h"
#include    "comm_cnf.h"
#include    "grcomm.h"

#include "grusbtyp.h"

#ifdef GRCOMD_DEBUG
#include    "dbg_mdl.h"
#define GRCOMD_LOG(m,n,x,y)  GRDBG_TRACE(m,n,x,y)
#else
#define GRCOMD_LOG(m,n,x,y)
#define GRDBG_COMD                      (0x00)
#define END_FUNC                        (0x00)
#endif

/**** INTERNAL DATA DEFINES *************************************************/
/* Descriptor Information */
GRCOMD_ABSTRACT_DESC*       l_ptAbstructDesc;
GRCOMD_CALL_DESC*           l_ptCallManageDesc;

GRCOMD_NOTIFICATION                 l_tNetworkConnection __attribute__ ((section (".uncache"), aligned (4), zero_init));                   /* V1.32 */
GRCOMD_NOTIFICATION                 l_tResponseAvailable __attribute__ ((section (".uncache"), aligned (4), zero_init));                   /* V1.32 */
GRCOMD_NOTIFICATION_SERIAL_STATE    l_tSerialState __attribute__ ((section (".uncache"), aligned (4), zero_init));                         /* V1.32 */

DLOCAL INT16    l_usRespSz;
DLOCAL UINT32   l_aulCntlArea[GRCOMD_CTRLAREASIZE/GRCOMD_BOUNDERY];
DLOCAL UINT8    l_aucCommDt[GRCOMD_MAX_DT_SZ];
DLOCAL INT      l_iUsbStat;
DLOCAL UINT8*   l_pucStrDesc = GRUSB_NULL;

/* APPLICATION CALLBACK FUNCTION PROTOTYPE */
LOCAL GRUSB_COMD_INITINFO   l_tInitInfo;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
#if 1
LOCAL INT                       _GRUSB_COMD_Setting( PGRUSB_PID );
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescFS( PGRUSB_PID );
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescHS( PGRUSB_PID );
#else
LOCAL INT                       _GRUSB_COMD_Setting( VOID );
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescFS( VOID );
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescHS( VOID );
#endif
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_GetCnfgDescFS( VOID );
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_GetCnfgDescHS( VOID );

LOCAL VOID _GRUSB_COMD_CbNtwrkConn( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_CbRespAvab( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_CbSrlStat( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_CbSndData( INT, UINT8*, UINT32, VOID*, INT );
LOCAL INT  _GRUSB_COMD_CbRcvData( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_CbDevRequest( UINT8, UINT8, UINT16, UINT16, UINT16 );
LOCAL INT  _GRUSB_COMD_CbSetCommFeature( INT, UINT8*, UINT32, VOID*, INT );
LOCAL INT  _GRUSB_COMD_CbSndEncpslCmnd( INT, UINT8*, UINT32, VOID*, INT );
LOCAL INT  _GRUSB_COMD_CbSetLineCoding( INT, UINT8*, UINT32, VOID*, INT );

LOCAL VOID _GRUSB_COMD_CbConn( VOID );
LOCAL VOID _GRUSB_COMD_CbDisConn( VOID );

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_Init                                             */
/*                                                                          */
/* DESCRIPTION: Initialization of Communication Class Driver.               */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptInitInfo          Initial setting information structure   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_Init( GRUSB_COMD_INITINFO*   ptInitInfo )
{
    INT iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x00, 0x00, 0x00 );

    /* Initialization of an internal variable */
    l_usRespSz = 0;
    l_iUsbStat = GRUSB_COMD_DISC;

    /* Initialization of a Callback Function */
    l_tInitInfo.pfnConnStat                         = GRUSB_NULL;
    l_tInitInfo.pfnSendEncapsulatedCmd              = GRUSB_NULL;
    l_tInitInfo.pfnGetEncapsulatedRes               = GRUSB_NULL;
    l_tInitInfo.pfnSetCommFeature                   = GRUSB_NULL;
    l_tInitInfo.pfnGetCommFeature                   = GRUSB_NULL;
    l_tInitInfo.pfnClearCommFeature                 = GRUSB_NULL;
    l_tInitInfo.pfnSetLineCoding                    = GRUSB_NULL;
    l_tInitInfo.pfnGetLineCoding                    = GRUSB_NULL;
    l_tInitInfo.pfnSetControlLineState              = GRUSB_NULL;
    l_tInitInfo.pfnSendBreak                        = GRUSB_NULL;
    l_tInitInfo.pfnNetworkConnection                = GRUSB_NULL;
    l_tInitInfo.pfnResponseAvailable                = GRUSB_NULL;
    l_tInitInfo.pfnSerialState                      = GRUSB_NULL;
    l_tInitInfo.pfnSendData                         = GRUSB_NULL;
    l_tInitInfo.pfnReciveData                       = GRUSB_NULL;

    /* set of Callback Functions */
    if( ptInitInfo->pfnConnStat )
        l_tInitInfo.pfnConnStat = ptInitInfo->pfnConnStat;
    if( ptInitInfo->pfnSendEncapsulatedCmd )
        l_tInitInfo.pfnSendEncapsulatedCmd = ptInitInfo->pfnSendEncapsulatedCmd;
    if( ptInitInfo->pfnGetEncapsulatedRes )
        l_tInitInfo.pfnGetEncapsulatedRes = ptInitInfo->pfnGetEncapsulatedRes;
    if( ptInitInfo->pfnSetCommFeature )
        l_tInitInfo.pfnSetCommFeature = ptInitInfo->pfnSetCommFeature;
    if( ptInitInfo->pfnGetCommFeature )
        l_tInitInfo.pfnGetCommFeature = ptInitInfo->pfnGetCommFeature;
    if( ptInitInfo->pfnClearCommFeature )
        l_tInitInfo.pfnClearCommFeature = ptInitInfo->pfnClearCommFeature;
    if( ptInitInfo->pfnSetLineCoding )
        l_tInitInfo.pfnSetLineCoding = ptInitInfo->pfnSetLineCoding;
    if( ptInitInfo->pfnGetLineCoding )
        l_tInitInfo.pfnGetLineCoding = ptInitInfo->pfnGetLineCoding;
    if( ptInitInfo->pfnSetControlLineState )
        l_tInitInfo.pfnSetControlLineState = ptInitInfo->pfnSetControlLineState;
    if( ptInitInfo->pfnSendBreak )
        l_tInitInfo.pfnSendBreak = ptInitInfo->pfnSendBreak;
    if( ptInitInfo->pfnNetworkConnection )
        l_tInitInfo.pfnNetworkConnection = ptInitInfo->pfnNetworkConnection;
    if( ptInitInfo->pfnResponseAvailable )
        l_tInitInfo.pfnResponseAvailable = ptInitInfo->pfnResponseAvailable;
    if( ptInitInfo->pfnSerialState )
        l_tInitInfo.pfnSerialState = ptInitInfo->pfnSerialState;
    if( ptInitInfo->pfnSendData )
        l_tInitInfo.pfnSendData = ptInitInfo->pfnSendData;
    if( ptInitInfo->pfnReciveData )
        l_tInitInfo.pfnReciveData = ptInitInfo->pfnReciveData;

    /* Initialize of COMM Driver */
#if 1
    iStat = _GRUSB_COMD_Setting(ptInitInfo->pstUsbPid);
#else
    iStat = _GRUSB_COMD_Setting();
#endif

    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x00, 0x01, END_FUNC );
        return GRUSB_COMD_ERROR;
    }

    GRCOMD_LOG( GRDBG_COMD, 0x00, 0x00, END_FUNC );
    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_Setting                                         */
/*                                                                          */
/* DESCRIPTION: Initial value registration to a Peripheral Driver.          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : iRet                Initialization result of USBD driver    */
/*                                                                          */
/****************************************************************************/
#if 1
LOCAL INT  _GRUSB_COMD_Setting( PGRUSB_PID pstUsb_pid)
#else
LOCAL INT  _GRUSB_COMD_Setting( VOID )
#endif
{
    GRUSB_DEV_INITINFO      tIniPrm;
    GRUSB_DEV_EPINITINFO*   ptEpInfo;
#ifdef _GRUSB_COMD_DEBUG
    UINT16                  usQue[GRCOMD_NUM_ENDPOINTS];
#endif
    INT                     iRet;

    GRCOMD_LOG( GRDBG_COMD, 0x01, 0x00, 0x00 );

    /* Parameter setup */
#if 1
    tIniPrm.ptDeviceDesc[0] = _GRUSB_COMD_GetDevDescFS(pstUsb_pid);   /* Device Descriptor value          */
    tIniPrm.ptDeviceDesc[1] = _GRUSB_COMD_GetDevDescHS(pstUsb_pid);   /* Device Descriptor value          */
#else
    tIniPrm.ptDeviceDesc[0] = _GRUSB_COMD_GetDevDescFS();   /* Device Descriptor value          */
    tIniPrm.ptDeviceDesc[1] = _GRUSB_COMD_GetDevDescHS();   /* Device Descriptor value          */
#endif
    tIniPrm.usConfigDescNum = GRCOMD_NUM_CONFIGURATIONS;    /* Configuration Descriptor number  */
    tIniPrm.ptConfigDesc[0] = _GRUSB_COMD_GetCnfgDescFS();  /* Configuration Descriptor value   */
    tIniPrm.ptConfigDesc[1] = _GRUSB_COMD_GetCnfgDescHS();  /* Configuration Descriptor value   */

    /* FullとHiで違いがないので便宜上Fullの情報に基づいて設定する */
    l_ptAbstructDesc   = &(((GRCOMD_CLASS_DESC *)tIniPrm.ptConfigDesc[0])->tAbstract);
    l_ptCallManageDesc = &(((GRCOMD_CLASS_DESC *)tIniPrm.ptConfigDesc[0])->tCall);

#ifdef _GRUSB_COMD_DEBUG
    usQue[0] = GRCOMD_CTRL_BUF;                     /* The number of Control transfer buffer    */
    usQue[1] = GRCOMD_EP1_DATABUF;                  /* The number of Data transfer buffer       */
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_CFGDESC_NUMINTERFACES != 0x01 )
    usQue[2] = GRCOMD_EP2_DATABUF;                  /* The number of Data transfer buffer       */
    usQue[3] = GRCOMD_EP3_DATABUF;                  /* The number of Data transfer buffer       */
#endif

    /* The check of control area size */
    if (!(GRUSB_DEV_ControlAreaSize(GRCOMD_NUM_ENDPOINTS, usQue) < GRCOMD_CTRLAREASIZE))
    {
        /* GRCOMD_CTRLAREASIZE is not a suitable value when it enters here. */
        /*  Please improve GRCOMD_CTRLAREASIZE                              */
        while(1);   /* error */
    }
#endif /* _GRUSB_COMD_DEBUG */

    tIniPrm.pucCntrlArea    = (UINT8 *)l_aulCntlArea;           /* The pointer of Control Area      */
    tIniPrm.iEpNum          = GRCOMD_NUM_ENDPOINTS;             /* The number of EndPoints          */
    tIniPrm.ucStringDescNum = GRCOMD_STRING_DESC_NUM;           /* String Descriptor number         */
    if( tIniPrm.ucStringDescNum == 0 )
    {
        /* No String Descriptor */
        tIniPrm.pStringDesc     = GRUSB_NULL;
    }
    else
    {   /* Get String Descriptor */
        tIniPrm.pStringDesc = GRUSB_COMD_GetStrDesc();          /* String Descriptor value          */
    }
    
    /* Save the pointer of the String Descriptor */
    l_pucStrDesc = (UINT8*)tIniPrm.pStringDesc;

    /* Setup of Endpoint information */
    ptEpInfo = &tIniPrm.atEpInfo[0];

    /*----------------------------------*/
    /* Endpoint 0 Information           */
    ptEpInfo->usNumOfQue    = GRCOMD_CTRL_BUF;              /* The number of Buffer                 */
    ptEpInfo->ucPages       = GRCOMD_EP0_PAGENUM;           /* FIFO Size                            */
    ptEpInfo->usMaxPktSz[0] = GRCOMD_EP0_MAXPKTSIZE_FS;     /* Max Packet Size                      */
    ptEpInfo->usMaxPktSz[1] = GRCOMD_EP0_MAXPKTSIZE_HS;     /* Max Packet Size                      */
    ptEpInfo->ucEpType      = EPTYPE_CONTROL;               /* End Point Type                       */
    ptEpInfo->pfnTransEnd   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  the completion of transmitting      */
    ptEpInfo->pfnTransErr   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  Transmission Error                  */
    ptEpInfo++;                                             /*-- Shift to the following point --*/
#if (0) /* V1.22 */
    /*----------------------------------*/
    /* End Point 1 information          */
    ptEpInfo->usNumOfQue    = GRCOMD_EP1_DATABUF;           /* The number of Transmission Buffer    */
    ptEpInfo->ucPages       = GRCOMD_EP1_PAGENUM;           /* FIFO Size                            */
    ptEpInfo->usMaxPktSz[0] = GRCOMD_EP1_MAXPKTSIZE_FS;     /* Max Packet Size                      */
    ptEpInfo->usMaxPktSz[1] = GRCOMD_EP1_MAXPKTSIZE_HS;     /* Max Packet Size                      */
    ptEpInfo->ucEpType      = EPTYPE_INTERRUPT              /* End Point Type                       */
                              | EPTYPE_IN;                  /* The transmission direction           */
    ptEpInfo->pfnTransEnd   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  the completion of transmitting      */
    ptEpInfo->pfnTransErr   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  Transmission Error                  */
    ptEpInfo++;                                             /*--- Shift to the following point ---*/
#endif   /* V1.22 */
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_CFGDESC_NUMINTERFACES != 0x01 )
    /*----------------------------------*/
    /* End Point 1 information          */                                                              /* V1.22 */
    ptEpInfo->usNumOfQue    = GRCOMD_EP1_DATABUF;           /* The number of Transmission Buffer    */  /* V1.22 */
    ptEpInfo->ucPages       = GRCOMD_EP1_PAGENUM;           /* FIFO Size                            */  /* V1.22 */
    ptEpInfo->usMaxPktSz[0] = GRCOMD_EP1_MAXPKTSIZE_FS;     /* Max Packet Size                      */  /* V1.22 */
    ptEpInfo->usMaxPktSz[1] = GRCOMD_EP1_MAXPKTSIZE_HS;     /* Max Packet Size                      */  /* V1.22 */
    ptEpInfo->ucEpType      = EPTYPE_BULK                   /* End Point Type                       */
                              | GRCOMD_EP1_EPTYPE;          /* The transmission direction           */  /* V1.22 */
    ptEpInfo->pfnTransEnd   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  the completion of transmitting      */
    ptEpInfo->pfnTransErr   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  Transmission Error                  */
    ptEpInfo++;                                             /*--- Shift to the following point ---*/
    /*----------------------------------*/
    /* End Point 2 information          */                                                              /* V1.22 */
    ptEpInfo->usNumOfQue    = GRCOMD_EP2_DATABUF;           /* The number of Transmission Buffer    */  /* V1.22 */
    ptEpInfo->ucPages       = GRCOMD_EP2_PAGENUM;           /* FIFO Size                            */  /* V1.22 */
    ptEpInfo->usMaxPktSz[0] = GRCOMD_EP2_MAXPKTSIZE_FS;     /* Max Packet Size                      */  /* V1.22 */
    ptEpInfo->usMaxPktSz[1] = GRCOMD_EP2_MAXPKTSIZE_HS;     /* Max Packet Size                      */  /* V1.22 */
    ptEpInfo->ucEpType      = EPTYPE_BULK                   /* End Point Type                       */
                              | GRCOMD_EP2_EPTYPE;          /* The transmission direction           */  /* V1.22 */
    ptEpInfo->pfnTransEnd   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  the completion of transmitting      */
    ptEpInfo->pfnTransErr   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  Transmission Error                  */
    ptEpInfo++;                                             /*--- Shift to the following point ---*/
#endif
#if (1)  /* V1.22 */
    /*----------------------------------*/
    /* End Point 3 information          */
    ptEpInfo->usNumOfQue    = GRCOMD_EP3_DATABUF;           /* The number of Transmission Buffer    */
    ptEpInfo->ucPages       = GRCOMD_EP3_PAGENUM;           /* FIFO Size                            */
    ptEpInfo->usMaxPktSz[0] = GRCOMD_EP3_MAXPKTSIZE_FS;     /* Max Packet Size                      */
    ptEpInfo->usMaxPktSz[1] = GRCOMD_EP3_MAXPKTSIZE_HS;     /* Max Packet Size                      */
    ptEpInfo->ucEpType      = EPTYPE_INTERRUPT              /* End Point Type                       */
                              | EPTYPE_IN;                  /* The transmission direction           */
    ptEpInfo->pfnTransEnd   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  the completion of transmitting      */
    ptEpInfo->pfnTransErr   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  Transmission Error                  */
    ptEpInfo++;                                             /*--- Shift to the following point ---*/
#endif   /* V1.22 */
    tIniPrm.usReqCodeMap    = 0x0000;       /* The Device Request code notified to a upper layer    */
    tIniPrm.pfnDevRquest    = _GRUSB_COMD_CbDevRequest;
                                                /* Notice function of Device Request reception      */
    tIniPrm.pfnSndCancel    = GRUSB_NULL;       /* Notice function of Control transmitting abort    */
    tIniPrm.pfnRcvCancel    = GRUSB_NULL;       /* Notice function of Control reception abort       */
    tIniPrm.pfnBusReset     = _GRUSB_COMD_CbDisConn;
                                                /* Notice function of Bus Reset reception           */
    tIniPrm.pfnClearFeature = GRUSB_NULL;       /* Notice function of Clear Future reception        */
    tIniPrm.pfnResume       = GRUSB_NULL;       /* Notice function of Resume reception              */
    tIniPrm.pfnSuspend      = GRUSB_NULL;       /* Notice function of Suspend reception             */
    tIniPrm.pfnEnbRmtWkup   = GRUSB_NULL;       /* Notice function of Remote Wakeup reception       */

    tIniPrm.pfnCmpSetInterface      = GRUSB_NULL;
                                            /* Notice function of Set Interface reception           */
    tIniPrm.pfnCmpSetConfiguration  = _GRUSB_COMD_CbConn;
                                            /* Notice function of Set Configuration reception       */
    tIniPrm.pfnCngStateNoConfigured = _GRUSB_COMD_CbDisConn;
                                            /* Notice function of the state changes from Configured */

    /* Initialization of USB Driver */
    iRet = GRUSB_DEV_ApInit(&tIniPrm);

    /* Callback function called at the time of the notice of connection/disconnection is registered */
    /* A connection notice is given at the time of SetConfiguration reception                       */
    GRUSB_DEV_ApCallbackConnect( GRUSB_NULL );                                      /* Connect      */
    GRUSB_DEV_ApCallbackDisconnect(_GRUSB_COMD_CbDisConn);                          /* Disconnect   */

    GRCOMD_LOG( GRDBG_COMD, 0x01, 0x00, END_FUNC );
    return iRet;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_GetDevDescFS                                    */
/*                                                                          */
/* DESCRIPTION: Device Descriptor is returned.                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Device descriptor information           */
/*                                                                          */
/****************************************************************************/
#if 1
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescFS( PGRUSB_PID pstUsbpid )
#else
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescFS( VOID )
#endif
{
#if 1
    DLOCAL const union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                  /* Device Descriptor                        */
        UINT32                  ulDummy;                /* In order to assign to a 4-byte boundary  */
    } uconData = {
		GRCOMD_DEVDESC_LENGTH                   ,       /* bLength                                  */
		GRCOMD_DEVDESC_DESCRIPTORTYPE           ,       /* bDescriptorType                          */
		GRCOMD_CNF_USB_LSB, GRCOMD_CNF_USB_MSB  ,       /* bcdUSB                                   */
		GRCOMD_DEVDESC_DEVICECLASS              ,       /* bDeviceClass (Communication Device Class)*/
		GRCOMD_DEVDESC_DEVICESUBCLASS           ,       /* bDeviceSubClass                          */
		GRCOMD_DEVDESC_DEVICEPROTOCOL           ,       /* bDeviceProtocol                          */
		GRCOMD_DEVDESC_MAXPKTSIZE0_FS           ,       /* bMaxPacketSize0                          */
		GRCOMD_CNF_VID_LSB, GRCOMD_CNF_VID_MSB  ,       /* idVendor                                 */
		0, 0  ,       									/* idProduct                                */
		GRCOMD_CNF_DEV_LSB, GRCOMD_CNF_DEV_MSB  ,       /* bcdDevice                                */
		GRCOMD_CNF_MANUFACTURER                 ,       /* iManufacturer                            */
		GRCOMD_CNF_PRODUCT                      ,       /* iProduct                                 */
		GRCOMD_CNF_SERIALNUMBER                 ,       /* iSerialNumber                            */
		GRCOMD_DEVDESC_NUMCONFIGURATIONS                /* bNumConfigurations                       */
        };
    DLOCAL union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                  /* Device Descriptor                        */
        UINT32                  ulDummy;                /* In order to assign to a 4-byte boundary  */
    } uData;
    memcpy(&uData, &uconData, sizeof(uData));
    uData.tDesc.auc_idProduct[0] = pstUsbpid->LSB;
    uData.tDesc.auc_idProduct[1] = pstUsbpid->MSB;
#else
    DLOCAL const union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                  /* Device Descriptor                        */
        UINT32                  ulDummy;                /* In order to assign to a 4-byte boundary  */
    } uData = {
        GRCOMD_DEVDESC_LENGTH                   ,       /* bLength                                  */
        GRCOMD_DEVDESC_DESCRIPTORTYPE           ,       /* bDescriptorType                          */
        GRCOMD_CNF_USB_LSB, GRCOMD_CNF_USB_MSB  ,       /* bcdUSB                                   */
        GRCOMD_DEVDESC_DEVICECLASS              ,       /* bDeviceClass (Communication Device Class)*/
        GRCOMD_DEVDESC_DEVICESUBCLASS           ,       /* bDeviceSubClass                          */
        GRCOMD_DEVDESC_DEVICEPROTOCOL           ,       /* bDeviceProtocol                          */
        GRCOMD_DEVDESC_MAXPKTSIZE0_FS           ,       /* bMaxPacketSize0                          */
        GRCOMD_CNF_VID_LSB, GRCOMD_CNF_VID_MSB  ,       /* idVendor                                 */
        GRCOMD_CNF_PID_LSB, GRCOMD_CNF_PID_MSB  ,       /* idProduct                                */
        GRCOMD_CNF_DEV_LSB, GRCOMD_CNF_DEV_MSB  ,       /* bcdDevice                                */
        GRCOMD_CNF_MANUFACTURER                 ,       /* iManufacturer                            */
        GRCOMD_CNF_PRODUCT                      ,       /* iProduct                                 */
        GRCOMD_CNF_SERIALNUMBER                 ,       /* iSerialNumber                            */
        GRCOMD_DEVDESC_NUMCONFIGURATIONS                /* bNumConfigurations                       */
    };
#endif
    return  (GRUSB_DEV_DEVICE_DESC*)&(uData.tDesc);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_GetDevDescHS                                    */
/*                                                                          */
/* DESCRIPTION: Device Descriptor is returned.                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Device descriptor information           */
/*                                                                          */
/****************************************************************************/
#if 1
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescHS( PGRUSB_PID pstUsbpid )
#else
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_GetDevDescHS( VOID )
#endif
{
#ifdef GRUSB_DEV_FIXED_FS

    return GRUSB_NULL;
#else /* GRUSB_DEV_FIXED_FS */

#if 1
    DLOCAL const union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                  /* Device Descriptor                        */
        UINT32                  ulDummy;                /* In order to assign to a 4-byte boundary  */
    } uconData = {
        GRCOMD_DEVDESC_LENGTH                   ,       /* bLength                                  */
        GRCOMD_DEVDESC_DESCRIPTORTYPE           ,       /* bDescriptorType                          */
        GRCOMD_CNF_USB_LSB, GRCOMD_CNF_USB_MSB  ,       /* bcdUSB                                   */
        GRCOMD_DEVDESC_DEVICECLASS              ,       /* bDeviceClass (Communication Device Class)*/
        GRCOMD_DEVDESC_DEVICESUBCLASS           ,       /* bDeviceSubClass                          */
        GRCOMD_DEVDESC_DEVICEPROTOCOL           ,       /* bDeviceProtocol                          */
        GRCOMD_DEVDESC_MAXPKTSIZE0_HS           ,       /* bMaxPacketSize0                          */
        GRCOMD_CNF_VID_LSB, GRCOMD_CNF_VID_MSB  ,       /* idVendor                                 */
        0, 0  ,       /* idProduct                                */
        GRCOMD_CNF_DEV_LSB, GRCOMD_CNF_DEV_MSB  ,       /* bcdDevice                                */
        GRCOMD_CNF_MANUFACTURER                 ,       /* iManufacturer                            */
        GRCOMD_CNF_PRODUCT                      ,       /* iProduct                                 */
        GRCOMD_CNF_SERIALNUMBER                 ,       /* iSerialNumber                            */
        GRCOMD_DEVDESC_NUMCONFIGURATIONS                /* bNumConfigurations                       */
    };
    DLOCAL union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                  /* Device Descriptor                        */
        UINT32                  ulDummy;                /* In order to assign to a 4-byte boundary  */
    } uData;
    memcpy(&uData, &uconData, sizeof(uData));
    uData.tDesc.auc_idProduct[0] = pstUsbpid->LSB;
    uData.tDesc.auc_idProduct[1] = pstUsbpid->MSB;
#else
    DLOCAL const union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                  /* Device Descriptor                        */
        UINT32                  ulDummy;                /* In order to assign to a 4-byte boundary  */
    } uData = {
        GRCOMD_DEVDESC_LENGTH                   ,       /* bLength                                  */
        GRCOMD_DEVDESC_DESCRIPTORTYPE           ,       /* bDescriptorType                          */
        GRCOMD_CNF_USB_LSB, GRCOMD_CNF_USB_MSB  ,       /* bcdUSB                                   */
        GRCOMD_DEVDESC_DEVICECLASS              ,       /* bDeviceClass (Communication Device Class)*/
        GRCOMD_DEVDESC_DEVICESUBCLASS           ,       /* bDeviceSubClass                          */
        GRCOMD_DEVDESC_DEVICEPROTOCOL           ,       /* bDeviceProtocol                          */
        GRCOMD_DEVDESC_MAXPKTSIZE0_HS           ,       /* bMaxPacketSize0                          */
        GRCOMD_CNF_VID_LSB, GRCOMD_CNF_VID_MSB  ,       /* idVendor                                 */
        GRCOMD_CNF_PID_LSB, GRCOMD_CNF_PID_MSB  ,       /* idProduct                                */
        GRCOMD_CNF_DEV_LSB, GRCOMD_CNF_DEV_MSB  ,       /* bcdDevice                                */
        GRCOMD_CNF_MANUFACTURER                 ,       /* iManufacturer                            */
        GRCOMD_CNF_PRODUCT                      ,       /* iProduct                                 */
        GRCOMD_CNF_SERIALNUMBER                 ,       /* iSerialNumber                            */
        GRCOMD_DEVDESC_NUMCONFIGURATIONS                /* bNumConfigurations                       */
    };
#endif
    return  (GRUSB_DEV_DEVICE_DESC*)&(uData.tDesc);
#endif /* GRUSB_DEV_FIXED_FS */
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_GetCnfgDescFS                                   */
/*                                                                          */
/* DESCRIPTION: Configuration Descriptor is returned.                       */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Configuration descriptor information    */
/*                                                                          */
/****************************************************************************/
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_GetCnfgDescFS( VOID )
{
    DLOCAL const union {
        struct {
        GRUSB_DEV_CONFIG_DESC       tConfig;            /* Configuration Descriptor                 */
        GRUSB_DEV_INTERFACE_DESC    tInterface0;        /* Interface Descriptor 0                   */
        GRCOMD_HEADER_DESC          tHeader0;           /* Header Functional Descriptor 0           */
        GRCOMD_ABSTRACT_DESC        tAbstract; /* Abstract Control Management Functional Descriptor */
        GRCOMD_UNION_DESC           tUnion;             /* Union Functional Descriptor              */
        GRCOMD_CALL_DESC            tCall;              /* Call Management Functional Descriptor    */
        GRUSB_DEV_ENDPOINT_DESC     tEndPoint1;         /* Endpoint Descriptor 1                    */
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_CFGDESC_NUMINTERFACES != 0x01 )
        GRUSB_DEV_INTERFACE_DESC    tInterface1;        /* Interface Descriptor 1                   */
        GRCOMD_HEADER_DESC          tHeader1;           /* Header Functional Descriptor 1           */
        GRUSB_DEV_ENDPOINT_DESC     tEndPoint2;         /* Endpoint Descriptor 2                    */
        GRUSB_DEV_ENDPOINT_DESC     tEndPoint3;         /* Endpoint Descriptor 3                    */
#endif
        } tDesc;
        UINT32                      dummy;              /* In order to assign to a 4-byte boundary  */
    } uData = {
            /*--- Standard Configuration Descriptor ---*/
            GRCOMD_CFGDESC_LENGTH               ,   /* bLength                      (0x09)          */
            GRCOMD_CFGDESC_DESCRIPTORTYPE       ,   /* bDescriptorType              (0x02)          */
            GRCOMD_CFGDESC_TOTALLENGTH_L        ,   /* wTotalLength (The size of                    */
             GRCOMD_CFGDESC_TOTALLENGTH_H       ,   /*  a tDesc structure object)                   */
            GRCOMD_CFGDESC_NUMINTERFACES        ,   /* bNumInterfaces               (0x01 or 0x02)  */
            GRCOMD_CFGDESC_CONFIGURATIONVALUE   ,   /* bConfigurationValue          (0x01)          */
            GRCOMD_CFGDESC_CONFIGURATION        ,   /* iConfiguration               (0x00)          */
            GRCOMD_CNF_ATTRIBUTES               ,   /* bmAttributes                                 */
            GRCOMD_CNF_MAXPOWER                 ,   /* MaxPower                                     */
            /*--- Standard Interface Descriptor 0 ---*/
            GRCOMD_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_IFDESC0_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x00)          */
            GRCOMD_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_IFDESC0_NUMENDPOINTS         ,   /* bNumEndpoints(EP3)           (0x01)          */
            GRCOMD_IFDESC0_INTERFACECLASS       ,   /* bInterfaceClass              (0x02)          */
            GRCOMD_IFDESC0_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x02)          */
            GRCOMD_IFDESC_INTERFACEPROTOCOL     ,   /* bInterfaceProtocol           (0x00)  */
            GRCOMD_IFDESC_INTERFACE             ,   /* iInterface                   (0x00)          */
            /*--- Header Functional Descriptor 0 ---*/
            GRCOMD_HFDESC_FUNCLENGTH            ,   /* bFunctionLength              (0x05)          */
            GRCOMD_HFDESC_DESCTYPE              ,   /* bDescriptorType              (0x24)          */
            GRCOMD_HFDESC_DESCSUBTYPE           ,   /* bDescriptorSubtype           (0x00)          */
            GRCOMD_CNF_HF_CDC_LSB               ,   /* bcdCDC                                       */
             GRCOMD_CNF_HF_CDC_MSB              ,
            /*--- Abstract Control Management Functional Descriptor ---*/
            GRCOMD_ACMFDESC_FUNCLENGTH          ,   /* bFunctionLength              (0x04)          */
            GRCOMD_ACMFDESC_DESCTYPE            ,   /* bDescriptorType              (0x24)          */
            GRCOMD_ACMFDESC_DESCSUBTYPE         ,   /* bDescriptorSubtype           (0x02)          */
            GRCOMD_CNF_ACMF_CAPABILITIES        ,   /* bmCapabilities                               */
            /*--- Union Functional Descriptor ---*/
            GRCOMD_UFDESC_FUNCLENGTH            ,   /* bFunctionLength              (0x05)          */
            GRCOMD_UFDESC_DESCTYPE              ,   /* bDescriptorType              (0x24)          */
            GRCOMD_UFDESC_DESCSUBTYPE           ,   /* bDescriptorSubtype           (0x06)          */
            GRCOMD_UFDESC_MASTERIF              ,   /* bMasterInterface             (0x00)          */
            GRCOMD_UFDESC_SLAVEIF0              ,   /* bSlaveInterface0             (0x01)          */
            /*--- Call Management Functional Descriptor ---*/
            GRCOMD_CMFDESC_FUNCLENGTH           ,   /* bFunctionLength              (0x05)          */
            GRCOMD_CMFDESC_DESCTYPE             ,   /* bDescriptorType              (0x24)          */
            GRCOMD_CMFDESC_DESCSUBTYPE          ,   /* bDescriptorSubtype           (0x01)          */
            GRCOMD_CNF_CMF_CAPABILITIES         ,   /* bmCapabilities                               */
            GRCOMD_CNF_CMF_DATAIF               ,   /* bDataInterface               (0x01)          */
            /*--- Endpoint Descriptor 3 ---*/                                                           /* V1.22 */
            GRCOMD_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_EPDESC_EP3_ADDRESS           ,   /* bEndpointAddress             (0x83)          */  /* V1.22 */
            GRCOMD_EPDESC_ATTRIBUTES_INT        ,   /* bmAttributes                 (0x03)          */
            GRCOMD_EPDESC_EP3_MAXPKTSIZE_L_FS   ,   /* wMaxPacketSize                               */  /* V1.22 */
             GRCOMD_EPDESC_EP3_MAXPKTSIZE_H_FS  ,                                                       /* V1.22 */
            GRCOMD_CNF_INTERVAL_INTERRUPT_FS        /* bInterval                    (0x01 - 0xFF)   */
#if( GRCOMD_CFGDESC_NUMINTERFACES != 0x01 )
            ,/* If Interface number is not 1, this comma is required because of the following data  */
            /*--- Standard Interface Descriptor 1 ---*/
            GRCOMD_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_IFDESC1_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x01)          */
            GRCOMD_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_IFDESC1_NUMENDPOINTS         ,   /* bNumEndpoints(EP1,EP2)       (0x02)          */
            GRCOMD_IFDESC1_INTERFACECLASS       ,   /* bInterfaceClass              (0x0A)          */
            GRCOMD_IFDESC1_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x00)          */
            GRCOMD_IFDESC_INTERFACEPROTOCOL     ,   /* bInterfaceProtocol           (0x00)          */
            GRCOMD_IFDESC_INTERFACE             ,   /* iInterface                   (0x00)          */
            /*--- Header Functional Descriptor 1 ---*/
            GRCOMD_HFDESC_FUNCLENGTH            ,   /* bFunctionLength              (0x05)          */
            GRCOMD_HFDESC_DESCTYPE              ,   /* bDescriptorType              (0x24)          */
            GRCOMD_HFDESC_DESCSUBTYPE           ,   /* bDescriptorSubtype           (0x00)          */
            GRCOMD_CNF_HF_CDC_LSB               ,   /* bcdCDC                                       */
             GRCOMD_CNF_HF_CDC_MSB              ,
            /*--- Endpoint Descriptor 1 ---*/                                                           /* V1.22 */
            GRCOMD_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_EPDESC_EP1_ADDRESS           ,   /* bEndpointAddress             (0x81 or 0x01)  */  /* V1.22 */
            GRCOMD_EPDESC_ATTRIBUTES_BULK       ,   /* bmAttributes                 (0x02)          */
            GRCOMD_EPDESC_EP1_MAXPKTSIZE_L_FS   ,   /* wMaxPacketSize                               */  /* V1.22 */
             GRCOMD_EPDESC_EP1_MAXPKTSIZE_H_FS  ,                                                       /* V1.22 */
            GRCOMD_EPDESC_INTERVAL_BULK         ,   /* bInterval                    (0x00)          */
            /*--- Endpoint Descriptor 2 ---*/                                                           /* V1.22 */
            GRCOMD_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_EPDESC_EP2_ADDRESS           ,   /* bEndpointAddress             (0x02 or 0x82)  */  /* V1.22 */
            GRCOMD_EPDESC_ATTRIBUTES_BULK       ,   /* bmAttributes                 (0x02)          */
            GRCOMD_EPDESC_EP2_MAXPKTSIZE_L_FS   ,   /* wMaxPacketSize                               */  /* V1.22 */
             GRCOMD_EPDESC_EP2_MAXPKTSIZE_H_FS  ,                                                       /* V1.22 */
            GRCOMD_EPDESC_INTERVAL_BULK             /* bInterval                    (0x00)          */
#endif
    };

    return (GRUSB_DEV_CONFIG_DESC*)&(uData.tDesc);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_GetCnfgDescHS                                   */
/*                                                                          */
/* DESCRIPTION: Configuration Descriptor is returned.                       */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Configuration descriptor information    */
/*                                                                          */
/****************************************************************************/
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_GetCnfgDescHS( VOID )
{
#ifdef GRUSB_DEV_FIXED_FS

    return GRUSB_NULL;
#else /* GRUSB_DEV_FIXED_FS */

    DLOCAL const union {
        struct {
        GRUSB_DEV_CONFIG_DESC       tConfig;            /* Configuration Descriptor                 */
        GRUSB_DEV_INTERFACE_DESC    tInterface0;        /* Interface Descriptor 0                   */
        GRCOMD_HEADER_DESC          tHeader0;           /* Header Functional Descriptor 0           */
        GRCOMD_ABSTRACT_DESC        tAbstract; /* Abstract Control Management Functional Descriptor */
        GRCOMD_UNION_DESC           tUnion;             /* Union Functional Descriptor              */
        GRCOMD_CALL_DESC            tCall;              /* Call Management Functional Descriptor    */
        GRUSB_DEV_ENDPOINT_DESC     tEndPoint1;         /* Endpoint Descriptor 1                    */
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_CFGDESC_NUMINTERFACES != 0x01 )
        GRUSB_DEV_INTERFACE_DESC    tInterface1;        /* Interface Descriptor 1                   */
        GRCOMD_HEADER_DESC          tHeader1;           /* Header Functional Descriptor 1           */
        GRUSB_DEV_ENDPOINT_DESC     tEndPoint2;         /* Endpoint Descriptor 2                    */
        GRUSB_DEV_ENDPOINT_DESC     tEndPoint3;         /* Endpoint Descriptor 3                    */
#endif
        } tDesc;
        UINT32                      dummy;              /* In order to assign to a 4-byte boundary  */
    } uData = {
            /*--- Standard Configuration Descriptor ---*/
            GRCOMD_CFGDESC_LENGTH               ,   /* bLength                      (0x09)          */
            GRCOMD_CFGDESC_DESCRIPTORTYPE       ,   /* bDescriptorType              (0x02)          */
            GRCOMD_CFGDESC_TOTALLENGTH_L        ,   /* wTotalLength (The size of                    */
             GRCOMD_CFGDESC_TOTALLENGTH_H       ,   /*  a tDesc structure object)                   */
            GRCOMD_CFGDESC_NUMINTERFACES        ,   /* bNumInterfaces               (0x01 or 0x02)  */
            GRCOMD_CFGDESC_CONFIGURATIONVALUE   ,   /* bConfigurationValue          (0x01)          */
            GRCOMD_CFGDESC_CONFIGURATION        ,   /* iConfiguration               (0x00)          */
            GRCOMD_CNF_ATTRIBUTES               ,   /* bmAttributes                                 */
            GRCOMD_CNF_MAXPOWER                 ,   /* MaxPower                                     */
            /*--- Standard Interface Descriptor 0 ---*/
            GRCOMD_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_IFDESC0_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x00)          */
            GRCOMD_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_IFDESC0_NUMENDPOINTS         ,   /* bNumEndpoints(EP3)           (0x01)          */
            GRCOMD_IFDESC0_INTERFACECLASS       ,   /* bInterfaceClass              (0x02)          */
            GRCOMD_IFDESC0_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x02)          */
            GRCOMD_IFDESC_INTERFACEPROTOCOL     ,   /* bInterfaceProtocol           (0x00)  */
            GRCOMD_IFDESC_INTERFACE             ,   /* iInterface                   (0x00)          */
            /*--- Header Functional Descriptor 0 ---*/
            GRCOMD_HFDESC_FUNCLENGTH            ,   /* bFunctionLength              (0x05)          */
            GRCOMD_HFDESC_DESCTYPE              ,   /* bDescriptorType              (0x24)          */
            GRCOMD_HFDESC_DESCSUBTYPE           ,   /* bDescriptorSubtype           (0x00)          */
            GRCOMD_CNF_HF_CDC_LSB               ,   /* bcdCDC                                       */
             GRCOMD_CNF_HF_CDC_MSB              ,
            /*--- Abstract Control Management Functional Descriptor ---*/
            GRCOMD_ACMFDESC_FUNCLENGTH          ,   /* bFunctionLength              (0x04)          */
            GRCOMD_ACMFDESC_DESCTYPE            ,   /* bDescriptorType              (0x24)          */
            GRCOMD_ACMFDESC_DESCSUBTYPE         ,   /* bDescriptorSubtype           (0x02)          */
            GRCOMD_CNF_ACMF_CAPABILITIES        ,   /* bmCapabilities                               */
            /*--- Union Functional Descriptor ---*/
            GRCOMD_UFDESC_FUNCLENGTH            ,   /* bFunctionLength              (0x05)          */
            GRCOMD_UFDESC_DESCTYPE              ,   /* bDescriptorType              (0x24)          */
            GRCOMD_UFDESC_DESCSUBTYPE           ,   /* bDescriptorSubtype           (0x06)          */
            GRCOMD_UFDESC_MASTERIF              ,   /* bMasterInterface             (0x00)          */
            GRCOMD_UFDESC_SLAVEIF0              ,   /* bSlaveInterface0             (0x01)          */
            /*--- Call Management Functional Descriptor ---*/
            GRCOMD_CMFDESC_FUNCLENGTH           ,   /* bFunctionLength              (0x05)          */
            GRCOMD_CMFDESC_DESCTYPE             ,   /* bDescriptorType              (0x24)          */
            GRCOMD_CMFDESC_DESCSUBTYPE          ,   /* bDescriptorSubtype           (0x01)          */
            GRCOMD_CNF_CMF_CAPABILITIES         ,   /* bmCapabilities                               */
            GRCOMD_CNF_CMF_DATAIF               ,   /* bDataInterface               (0x01)          */
            /*--- Endpoint Descriptor 3 ---*/                                                           /* V1.23 */
            GRCOMD_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_EPDESC_EP3_ADDRESS           ,   /* bEndpointAddress             (0x81)          */  /* V1.23 */
            GRCOMD_EPDESC_ATTRIBUTES_INT        ,   /* bmAttributes                 (0x03)          */
            GRCOMD_EPDESC_EP3_MAXPKTSIZE_L_HS   ,   /* wMaxPacketSize                               */  /* V1.23 */
             GRCOMD_EPDESC_EP3_MAXPKTSIZE_H_HS  ,                                                       /* V1.23 */
            GRCOMD_CNF_INTERVAL_INTERRUPT_HS        /* bInterval                    (0x01 - 0xFF)   */
#if( GRCOMD_CFGDESC_NUMINTERFACES != 0x01 )
            ,/* If Interface number is not 1, this comma is required because of the following data  */
            /*--- Standard Interface Descriptor 1 ---*/
            GRCOMD_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_IFDESC1_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x01)          */
            GRCOMD_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_IFDESC1_NUMENDPOINTS         ,   /* bNumEndpoints(EP1,EP2)       (0x02)          */
            GRCOMD_IFDESC1_INTERFACECLASS       ,   /* bInterfaceClass              (0x0A)          */
            GRCOMD_IFDESC1_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x00)          */
            GRCOMD_IFDESC_INTERFACEPROTOCOL     ,   /* bInterfaceProtocol           (0x00)          */
            GRCOMD_IFDESC_INTERFACE             ,   /* iInterface                   (0x00)          */
            /*--- Header Functional Descriptor 1 ---*/
            GRCOMD_HFDESC_FUNCLENGTH            ,   /* bFunctionLength              (0x05)          */
            GRCOMD_HFDESC_DESCTYPE              ,   /* bDescriptorType              (0x24)          */
            GRCOMD_HFDESC_DESCSUBTYPE           ,   /* bDescriptorSubtype           (0x00)          */
            GRCOMD_CNF_HF_CDC_LSB               ,   /* bcdCDC                                       */
             GRCOMD_CNF_HF_CDC_MSB              ,
            /*--- Endpoint Descriptor 1 ---*/                                                           /* V1.23 */
            GRCOMD_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_EPDESC_EP1_ADDRESS           ,   /* bEndpointAddress             (0x82 or 0x02)  */  /* V1.23 */
            GRCOMD_EPDESC_ATTRIBUTES_BULK       ,   /* bmAttributes                 (0x02)          */
            GRCOMD_EPDESC_EP1_MAXPKTSIZE_L_HS   ,   /* wMaxPacketSize                               */  /* V1.23 */
             GRCOMD_EPDESC_EP1_MAXPKTSIZE_H_HS  ,                                                       /* V1.23 */
            GRCOMD_EPDESC_INTERVAL_BULK         ,   /* bInterval                    (0x00)          */
            /*--- Endpoint Descriptor 2 ---*/                                                           /* V1.23 */
            GRCOMD_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_EPDESC_EP2_ADDRESS           ,   /* bEndpointAddress             (0x03 or 0x83)  */  /* V1.23 */
            GRCOMD_EPDESC_ATTRIBUTES_BULK       ,   /* bmAttributes                 (0x02)          */
            GRCOMD_EPDESC_EP2_MAXPKTSIZE_L_HS   ,   /* wMaxPacketSize                               */  /* V1.23 */
             GRCOMD_EPDESC_EP2_MAXPKTSIZE_H_HS  ,                                                       /* V1.23 */
            GRCOMD_EPDESC_INTERVAL_BULK             /* bInterval                    (0x00)          */
#endif
    };

    return (GRUSB_DEV_CONFIG_DESC*)&(uData.tDesc);
#endif /* GRUSB_DEV_FIXED_FS */
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_SetSerialNumber                                  */
/*                                                                          */
/* DESCRIPTION: Set the serial number of the user-specified.                */
/*--------------------------------------------------------------------------*/
/* INPUT      : pusSerialNumber         Pointer of the serial number to     */
/*                                      overwrite                           */
/*              ucSerialNumberChars     Number of characters in the         */
/*                                      serial number                       */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_PRM_ERR      Parameter error                     */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_SetSerialNumber( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars )
{
    INT     i;
    INT     iSerialNumber      = GRCOMD_CNF_SERIALNUMBER;
    UINT32  ulPos              = 0;
    UINT16* pusSerialNumberStr = GRUSB_NULL;
    UINT8   ucStringDescNum    = GRCOMD_STRING_DESC_NUM;
    UINT8   ucDescSize;
    UINT8   ucCurrentChars;
    UINT8   ucCnt;
    
    /* Check the current device state */
    if( GRUSB_DEV_STATE_IDLE != GRUSB_DEV_ApGetDeviceState() )
    {
        /* Unusual end (Not IDLE(Attached/Powered) state) */
        return GRUSB_COMD_ERROR;
    }
    
    /* Check the String Descriptor */
    if( ( 0 == ucStringDescNum ) || ( GRUSB_NULL == l_pucStrDesc ) )
    {
        /* Unusual end (String Descriptor not set) */
        return GRUSB_COMD_ERROR;
    }
    
    /* Check the index of serial number */
    if( 0 == iSerialNumber )
    {
        /* Unusual end (Index of serial number not set) */
        return GRUSB_COMD_ERROR;
    }
    
    /* Check the parameters */
    if( ( GRUSB_NULL == pusSerialNumber ) || ( 0 == ucSerialNumberChars ) )
    {
        /* Parameter error (No serial number to overwrite) */
        return GRUSB_COMD_PRM_ERR;
    }

    /* Calculated start position of the String Descriptor for serial number */
    for( i = 0; i < iSerialNumber; i++ )
    {
        /* Added the size of the String Descriptor */
        ulPos += *( l_pucStrDesc + ulPos );
    }
    
    /* Get the size of the String Descriptor for serial number */
    ucDescSize = *( l_pucStrDesc + ulPos );
    /* Calculated the number of characters in the current serial number */
    ucCurrentChars = ( ( ucDescSize - 2 ) / sizeof(UINT16) );
    /* Check the number of characters in the serial number */
    if( ucCurrentChars != ucSerialNumberChars )
    {
        /* Parameter error (Number of characters is mismatch) */
        return GRUSB_COMD_PRM_ERR;
    }
    
    /* Get the start position of the serial number */
    pusSerialNumberStr = (UINT16*)( l_pucStrDesc + ulPos + 2 );
    
    /* Overwrite the serial number */
    for( ucCnt = 0; ucCnt < ucSerialNumberChars; ucCnt++ )
    {
        *( pusSerialNumberStr + ucCnt ) = GRCOMD_MAC_SWAPUS( *( pusSerialNumber + ucCnt ) );
    }
    
    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_Notification_NetworkConnection                   */
/*                                                                          */
/* DESCRIPTION: Connection/Disonnection state is notified to a host using   */
/*              an Interrupt-In endpoint.                                   */
/*--------------------------------------------------------------------------*/
/* INPUT      : usConn              Network connection state                */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_DESC_ERROR   Discriptor setting error            */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_Notification_NetworkConnection( UINT16 usConn )
{
    INT                     iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x02, 0x00, 0x00 );

    /* It investigates whether Network Connection is supported from the Abstract descriptor */
    if ((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_NETWORK) == 0)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x02, 0x01, END_FUNC );
        return(GRUSB_COMD_DESC_ERROR);
    }

    /* Parameter setup */
    l_tNetworkConnection.bmRequestType = GRUSB_DEV_DATA_TRNS_DIR_DH         /* V1.32 */
                                       | GRUSB_DEV_CLASS_TYPE
                                       | GRUSB_INTERFACE_STATUS;
    l_tNetworkConnection.bNotification = GRCOMD_NETWORK_CONNECTION;         /* V1.32 */
    l_tNetworkConnection.wValue        = GRCOMD_MAC_SWAPUS(usConn);
    l_tNetworkConnection.wIndex        = GRCOMD_MAC_SWAPUS(GRCOMD_NOTIFICATION_IF);
    l_tNetworkConnection.wLength       = GRCOMD_MAC_SWAPUS(GRCOMD_NETWORK_CONNECTION_LENGTH);

    /* A connection state is notified to a host */
    iStat = GRUSB_DEV_ApTransferSend(GRCOMD_EP3,                            /* V1.22 */
                                     (UINT8 *)&l_tNetworkConnection,        /* V1.32 */
                                     (UINT32)GRCOMD_NOTIFICATION_SIZE,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     _GRUSB_COMD_CbNtwrkConn);
    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x02, 0x02, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x02, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbNtwrkConn                                     */
/*                                                                          */
/* DESCRIPTION: The callback function of                                    */
/*              GRUSB_COMD_Notification_NetworkConnection.                  */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbNtwrkConn( INT     iEpNo,
                                    UINT8*  pucBuf,
                                    UINT32  ulSize,
                                    VOID*   pAplInfo,
                                    INT     iStat )
{
    GRCOMD_LOG( GRDBG_COMD, 0x03, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnNetworkConnection)
        (*l_tInitInfo.pfnNetworkConnection)();

    GRCOMD_LOG( GRDBG_COMD, 0x03, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_Notification_ResponseAvailable                   */
/*                                                                          */
/* DESCRIPTION: An end (GRCOMD_RESPONSE_AVAILABLE) is notified to a host    */
/*              using an Interrupt-In endpoint.                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_Notification_ResponseAvailable( VOID )
{
    INT                     iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x04, 0x00, 0x00 );

    /* Since it is indispensable, an Abstract descriptor is not investigated */

    /* Parameter setup */
    l_tResponseAvailable.bmRequestType = GRUSB_DEV_DATA_TRNS_DIR_DH         /* V1.32 */
                                       | GRUSB_DEV_CLASS_TYPE
                                       | GRUSB_INTERFACE_STATUS;
    l_tResponseAvailable.bNotification = GRCOMD_RESPONSE_AVAILABLE;         /* V1.32 */
    l_tResponseAvailable.wValue        = GRCOMD_MAC_SWAPUS(GRCOMD_RESPONSE_AVAILABLE_VALUE);
    l_tResponseAvailable.wIndex        = GRCOMD_MAC_SWAPUS(GRCOMD_NOTIFICATION_IF);
    l_tResponseAvailable.wLength       = GRCOMD_MAC_SWAPUS(GRCOMD_RESPONSE_AVAILABLE_LENGTH);

    /* An end (GRCOMD_RESPONSE_AVAILABLE) is notified to a host */
    iStat = GRUSB_DEV_ApTransferSend(GRCOMD_EP3,                            /* V1.22 */
                                     (UINT8 *)&l_tResponseAvailable,        /* V1.32 */
                                     (UINT32)GRCOMD_NOTIFICATION_SIZE,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     _GRUSB_COMD_CbRespAvab);
    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x04, 0x01, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x04, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbRespAvab                                      */
/*                                                                          */
/* DESCRIPTION: The callback function of                                    */
/*              GRUSB_COMD_Notification_ResponseAvailable.                  */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbRespAvab( INT      iEpNo,
                                   UINT8*   pucBuf,
                                   UINT32   ulSize,
                                   VOID*    pAplInfo,
                                   INT      iStat )
{
    GRCOMD_LOG( GRDBG_COMD, 0x05, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnResponseAvailable)
        (*l_tInitInfo.pfnResponseAvailable)();

    GRCOMD_LOG( GRDBG_COMD, 0x05, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_Notification_SerialState                         */
/*                                                                          */
/* DESCRIPTION: The state information on a cable is notified to a host      */
/*              using an Interrupt-In endpoint.                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : usStatBmap          UART state bitmap value                 */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_DESC_ERROR   Discriptor setting error            */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_Notification_SerialState( UINT16 usStatBmap )
{
    INT                                 iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x06, 0x00, 0x00 );

    /* It investigates whether SerialState is supported from the Abstract descriptor */
    if( ( l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_LINE_SERIAL ) == 0 )
        return(GRUSB_COMD_DESC_ERROR);

    l_tSerialState.tNotice.bmRequestType = GRUSB_DEV_DATA_TRNS_DIR_DH       /* V1.32 */
                                         | GRUSB_DEV_CLASS_TYPE
                                         | GRUSB_INTERFACE_STATUS;
    l_tSerialState.tNotice.bNotification = GRCOMD_SERIAL_STATE;             /* V1.32 */
    l_tSerialState.tNotice.wValue        = GRCOMD_MAC_SWAPUS(GRCOMD_SERIAL_STATE_VALUE);
    l_tSerialState.tNotice.wIndex        = GRCOMD_MAC_SWAPUS(GRCOMD_NOTIFICATION_IF);
    l_tSerialState.tNotice.wLength       = GRCOMD_MAC_SWAPUS(GRCOMD_SERIAL_STATE_LENGTH);
    l_tSerialState.usState               = GRCOMD_MAC_SWAPUS(usStatBmap);               /* 1.30 */ /* 1.31 */ /* V1.32 */

    /* A connection state is notified to a host */
    iStat = GRUSB_DEV_ApTransferSend(GRCOMD_EP3,                            /* V1.22 */
                                     (UINT8*)&l_tSerialState,               /* V1.32 */
                                     (UINT32)GRCOMD_NOTIFICATION_SERIAL_STATE_SIZE,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     _GRUSB_COMD_CbSrlStat);
    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x06, 0x01, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x06, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSrlStat                                       */
/*                                                                          */
/* DESCRIPTION: The callback function of                                    */
/*              GRUSB_COMD_Notification_SerialState.                        */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbSrlStat( INT       iEpNo,
                                  UINT8*    pucBuf,
                                  UINT32    ulSize,
                                  VOID*     pAplInfo,
                                  INT       iStat )
{
    GRCOMD_LOG( GRDBG_COMD, 0x07, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnSerialState)
        (*l_tInitInfo.pfnSerialState)();

    GRCOMD_LOG( GRDBG_COMD, 0x07, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_ReciveData                                       */
/*                                                                          */
/* DESCRIPTION: The request to receive of data.                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data buffer                     */
/*              pucData             pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_DESC_ERROR   Discriptor setting error            */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_ReciveData( UINT32   ulSize,
                           UINT8*   pucData,
                           VOID*    pAplInfo )
{
    INT     iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x08, 0x00, 0x00 );

    /* It investigates whether Data Class Interface is used from a Call Management descriptor */
    if ((l_ptCallManageDesc->bmCapabilities & GRCOMD_BM_CAPA_DATA_CLASS) == 0)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x08, 0x01, END_FUNC );
        return(GRUSB_COMD_DESC_ERROR);
    }

    /* Bulk reception is started */
    iStat = GRUSB_DEV_ApTransferRecv(GRCOMD_BULKOUT_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     pAplInfo,
                                     _GRUSB_COMD_CbRcvData);
    if (iStat != GRUSB_COMD_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x08, 0x02, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x08, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbRcvData                                       */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_ReciveData.             */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_COMD_CbRcvData( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    GRCOMD_LOG( GRDBG_COMD, 0x09, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnReciveData)
    {
#ifdef GRCOMD_COMP_STATUS_USE
        (*l_tInitInfo.pfnReciveData)(ulSize, pucBuf, pAplInfo, iStat);
#else
        (*l_tInitInfo.pfnReciveData)(ulSize, pucBuf, pAplInfo);
#endif
    }

    GRCOMD_LOG( GRDBG_COMD, 0x09, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_SendData                                         */
/*                                                                          */
/* DESCRIPTION: The request to send of data.                                */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucData             pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_DESC_ERROR   Discriptor setting error            */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_SendData( UINT32     ulSize,
                         UINT8*     pucData,
                         VOID*      pAplInfo )
{
    INT iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x0A, 0x00, 0x00 );

    /* It investigates whether Data Class Interface is used from a Call Management descriptor */
    if ((l_ptCallManageDesc->bmCapabilities & GRCOMD_BM_CAPA_DATA_CLASS) == 0)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0A, 0x01, END_FUNC );
        return(GRUSB_COMD_DESC_ERROR);
    }

    /* Bulk transmission */
    iStat = GRUSB_DEV_ApTransferSend(GRCOMD_BULKIN_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     GRUSB_TRUE,
                                     pAplInfo,
                                     _GRUSB_COMD_CbSndData);
    if (iStat != GRUSB_COMD_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0A, 0x02, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x0A, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSndData                                       */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_SendData.               */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbSndData( INT       iEpNo,
                                  UINT8*    pucBuf,
                                  UINT32    ulSize,
                                  VOID*     pAplInfo,
                                  INT       iStat )
{
    GRCOMD_LOG( GRDBG_COMD, 0x0B, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnSendData)
    {
#ifdef GRCOMD_COMP_STATUS_USE
        (*l_tInitInfo.pfnSendData)(ulSize, pucBuf, pAplInfo, iStat);
#else
        (*l_tInitInfo.pfnSendData)(ulSize, pucBuf, pAplInfo);
#endif
    }

    GRCOMD_LOG( GRDBG_COMD, 0x0B, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_Set_GetCommFeature                               */
/*                                                                          */
/* DESCRIPTION: The data transmitted to a host is set up at the time of     */
/*              GET_COMM_FEATURE request reception.                         */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucData             pointer of data buffer                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_SIZE_ERROR   Size error                          */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_Set_GetCommFeature( UINT32   ulSize,
                                   UINT8*   pucData )
{
    INT iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x0C, 0x00, 0x00 );

    /* Is the size of send data right? */
    if (ulSize != GRCOMD_FEATURE_STATUS_SIZE)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0C, 0x01, END_FUNC );
        return(GRUSB_COMD_SIZE_ERROR);
    }

    /* Send data is setup */
    iStat = GRUSB_DEV_ApControlSend(GRCOMD_EP0,
                                    pucData,
                                    ulSize,
                                    GRUSB_TRUE,
                                    GRUSB_NULL,
                                    GRUSB_NULL);

    if(iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0C, 0x02, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x0C, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_Set_GetEncapsulatedResponse                      */
/*                                                                          */
/* DESCRIPTION: The data transmitted to a host is set up at the time of     */
/*              GET_ENCAPSULATED_RESPONSE request reception.                */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucData             pointer of data buffer                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_SIZE_ERROR   Size error                          */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_Set_GetEncapsulatedResponse( UINT32  ulSize,
                                            UINT8*  pucData )
{
    INT iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x0D, 0x00, 0x00 );

    /* Is the size of send data right? */
    if (ulSize != l_usRespSz)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0D, 0x01, END_FUNC );
        return(GRUSB_COMD_SIZE_ERROR);
    }

    /* Send data is setup */
    iStat = GRUSB_DEV_ApControlSend(GRCOMD_EP0,
                                    pucData,
                                    ulSize,
                                    GRUSB_TRUE,
                                    GRUSB_NULL,
                                    GRUSB_NULL);

    if(iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0D, 0x02, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x0D, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_Set_GetLineCoding                                */
/*                                                                          */
/* DESCRIPTION: The data transmitted to a host is set up at the time of     */
/*              GET_LINE_CODING request reception.                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucData             pointer of data buffer                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS      Successfully end                    */
/*              GRUSB_COMD_ERROR        Unusual end                         */
/*              GRUSB_COMD_SIZE_ERROR   Size error                          */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_Set_GetLineCoding( UINT32    ulSize,
                                  UINT8*    pucData )
{
    INT iStat;

    GRCOMD_LOG( GRDBG_COMD, 0x0E, 0x00, 0x00 );

    /* Is the size of send data right? */
    if (ulSize != GRCOMD_LINE_CODING_SIZE)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0E, 0x01, END_FUNC );
        return(GRUSB_COMD_SIZE_ERROR);
    }

    /* Send data is setup */
    iStat = GRUSB_DEV_ApControlSend(GRCOMD_EP0,
                                    pucData,
                                    ulSize,
                                    GRUSB_TRUE,
                                    GRUSB_NULL,
                                    GRUSB_NULL);

    if(iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0E, 0x02, END_FUNC );
        return(GRUSB_COMD_ERROR);
    }

    GRCOMD_LOG( GRDBG_COMD, 0x0E, 0x00, END_FUNC );
    return(GRUSB_COMD_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbDevRequest                                    */
/*                                                                          */
/* DESCRIPTION: Processing at the time of Communication Class device        */
/*              request reception.                                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ucRequestType           bmRequestType                       */
/*              ucRequest               bRequest                            */
/*              usValue                 wValue                              */
/*              usIndex                 wIndex                              */
/*              usLength                wLength                             */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbDevRequest( UINT8      ucRequestType,
                                     UINT8      ucRequest,
                                     UINT16     usValue,
                                     UINT16     usIndex,
                                     UINT16     usLength )
{
    GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x00 );

    /* Is a request type effective ? */
    if (ucRequestType == (GRUSB_DEV_DATA_TRNS_DIR_DH
                        | GRUSB_DEV_CLASS_TYPE
                        | GRUSB_INTERFACE_STATUS))
    {
        /* Is the request code defined ? */
        switch(ucRequest)
        {
        case GRCOMD_GET_ENCAPSULATED_RESPONSE:
            /* Since it is indispensable, an Abstract descriptor is not investigated        */
            /* Are the value of a request being effective and the callback function setup?  */
            if ((usValue == 0)
             && (usIndex == GRCOMD_MANAGEMENT_IF))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x01 );

                l_usRespSz = usLength;                                      /* Response size is saved       */
                if (l_tInitInfo.pfnGetEncapsulatedRes)
                    (*l_tInitInfo.pfnGetEncapsulatedRes)(usLength);         /* Callback function is called  */
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x01 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_GET_COMM_FEATURE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_FEATURE) != 0)
             && (usValue  == GRCOMD_ABSTRACT_STATE)
             && (usIndex  == GRCOMD_MANAGEMENT_IF)
             && (usLength == GRCOMD_FEATURE_STATUS_SIZE))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x02 );

                if (l_tInitInfo.pfnGetCommFeature)
                    (*l_tInitInfo.pfnGetCommFeature)(usLength);             /* Callback function is called  */
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x02 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_GET_LINE_CODING:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_LINE_SERIAL) != 0)
             && (usValue  == 0)
             && (usIndex  == GRCOMD_MANAGEMENT_IF)
             && (usLength == GRCOMD_LINE_CODING_SIZE))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x03 );

                if (l_tInitInfo.pfnGetLineCoding)
                    (*l_tInitInfo.pfnGetLineCoding)(usLength);              /* Callback function is called  */
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x03 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        default:
            GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x04 );

            GRUSB_DEV_ApSetStall(GRCOMD_EP0);                               /* Endpoint0 is set to STALL    */
            break;
        }
    }
    /* Is a request type effective ? */
    else if (ucRequestType == (GRUSB_DEV_DATA_TRNS_DIR_HD
                             | GRUSB_DEV_CLASS_TYPE
                             | GRUSB_INTERFACE_STATUS))
    {
        /* Is the request code defined ? */
        switch(ucRequest)
        {
        case GRCOMD_CLEAR_COMM_FEATURE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_FEATURE) != 0)
             && (usValue  == GRCOMD_ABSTRACT_STATE)
             && (usIndex  == GRCOMD_MANAGEMENT_IF)
             && (usLength == 0))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x05 );

                if (l_tInitInfo.pfnClearCommFeature)
                    (*l_tInitInfo.pfnClearCommFeature)();                   /* Callback function is called  */
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x05 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_SET_CONTROL_LINE_STATE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_LINE_SERIAL) != 0)
             && (usIndex  == GRCOMD_MANAGEMENT_IF)
             && (usLength == 0))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x06 );

                if (l_tInitInfo.pfnSetControlLineState)
                    (*l_tInitInfo.pfnSetControlLineState)(usValue);         /* Callback function is called  */
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x06 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_SEND_BREAK:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_BREAK) != 0)
             && (usIndex  == GRCOMD_MANAGEMENT_IF)
             && (usLength == 0))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x07 );

                if (l_tInitInfo.pfnSendBreak)
                    (*l_tInitInfo.pfnSendBreak)(usValue);                   /* Callback function is called  */
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x07 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_SEND_ENCAPSULATED_COMMAND:
            /* Since it is indispensable, an Abstract descriptor is not investigated */
            /* Are the value of a request being effective and the callback function setup? */
            if ((usValue == 0)
             && (usIndex == GRCOMD_MANAGEMENT_IF)
             && (usLength > 0))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x08 );

                if (GRUSB_DEV_SUCCESS != (GRUSB_DEV_ApControlRecv(
                                                        GRCOMD_EP0,
                                                        l_aucCommDt,
                                                        (UINT32)usLength,
                                                        GRUSB_NULL,
                                                        _GRUSB_COMD_CbSndEncpslCmnd)))
                {
                    GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x08 );

                    /* When a response is an error */
                    GRUSB_DEV_ApSetStall(GRCOMD_EP0);                       /* Endpoint0 is set to STALL    */
                }
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x02, 0x08 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_SET_COMM_FEATURE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_FEATURE) != 0)
             && (usValue == GRCOMD_ABSTRACT_STATE)
             && (usIndex == GRCOMD_MANAGEMENT_IF)
             && (usLength == GRCOMD_FEATURE_STATUS_SIZE))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x09 );

                if (GRUSB_DEV_SUCCESS != (GRUSB_DEV_ApControlRecv(
                                                        GRCOMD_EP0,
                                                        l_aucCommDt,
                                                        (UINT32)usLength,
                                                        GRUSB_NULL,
                                                        _GRUSB_COMD_CbSetCommFeature)))
                {
                    GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x09 );

                    /* When a response is an error */
                    GRUSB_DEV_ApSetStall(GRCOMD_EP0);                       /* Endpoint0 is set to STALL    */
                }
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x02, 0x09 );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_SET_LINE_CODING:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDesc->bmCapabilities & GRCOMD_BM_CAPA_LINE_SERIAL) != 0)
             && (usValue == 0)
             && (usIndex == GRCOMD_MANAGEMENT_IF)
             && (usLength == GRCOMD_LINE_CODING_SIZE))
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x0A );

                if (GRUSB_DEV_SUCCESS != (GRUSB_DEV_ApControlRecv(
                                                        GRCOMD_EP0,
                                                        l_aucCommDt,
                                                        (UINT32)usLength,
                                                        GRUSB_NULL,
                                                        _GRUSB_COMD_CbSetLineCoding)))
                {
                    GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x01, 0x0A );

                    /* When a response is an error */
                    GRUSB_DEV_ApSetStall(GRCOMD_EP0);                       /* Endpoint0 is set to STALL    */
                }
            }
            else
            {
                GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x02, 0x0A );

                GRUSB_DEV_ApSetStall(GRCOMD_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        default:
            GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x0B );

            GRUSB_DEV_ApSetStall(GRCOMD_EP0);                               /* Endpoint0 is set to STALL    */
            break;
        }
    }
    else
    {
        GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, 0x0C );

        GRUSB_DEV_ApSetStall(GRCOMD_EP0);                                   /* Endpoint0 is set to STALL    */
    }

    GRCOMD_LOG( GRDBG_COMD, 0x0F, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSndEncpslCmnd                                 */
/*                                                                          */
/* DESCRIPTION: The notice of data stage completion at the time of          */
/*              SEND_ENCAPSULATED_COMMAND reception.                        */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE          Successfully end                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_COMD_CbSndEncpslCmnd( INT          iEpNo,
                                       UINT8*       pucBuf,
                                       UINT32       ulSize,
                                       VOID*        pAplInfo,
                                       INT          iStat )
{
    GRCOMD_LOG( GRDBG_COMD, 0x10, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnSendEncapsulatedCmd)
        (*l_tInitInfo.pfnSendEncapsulatedCmd)(ulSize, pucBuf);

    GRCOMD_LOG( GRDBG_COMD, 0x10, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSetCommFeature                                */
/*                                                                          */
/* DESCRIPTION: The notice of data stage completion at the time of          */
/*              SET_COMM_FEATURE reception.                                 */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE          Successfully end                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_COMD_CbSetCommFeature( INT         iEpNo,
                                        UINT8*      pucBuf,
                                        UINT32      ulSize,
                                        VOID*       pAplInfo,
                                        INT         iStat )
{
    GRCOMD_LOG( GRDBG_COMD, 0x11, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnSetCommFeature)
        (*l_tInitInfo.pfnSetCommFeature)(ulSize, pucBuf);

    GRCOMD_LOG( GRDBG_COMD, 0x11, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSetLineCoding                                 */
/*                                                                          */
/* DESCRIPTION: The notice of data stage completion at the time of          */
/*              SET_LINE_CODING reception.                                  */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE          Successfully end                        */
/*                                                                          */
/****************************************************************************/
LOCAL INT _GRUSB_COMD_CbSetLineCoding( INT          iEpNo,
                                       UINT8*       pucBuf,
                                       UINT32       ulSize,
                                       VOID*        pAplInfo,
                                       INT          iStat)
{
    GRCOMD_LOG( GRDBG_COMD, 0x12, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo.pfnSetLineCoding)
        (*l_tInitInfo.pfnSetLineCoding)(ulSize, pucBuf);

    GRCOMD_LOG( GRDBG_COMD, 0x12, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbConn                                          */
/*                                                                          */
/* DESCRIPTION: Notice processing of Connection.                            */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _GRUSB_COMD_CbConn( VOID )
{
    GRCOMD_LOG( GRDBG_COMD, 0x13, 0x00, 0x00 );

    /* check local usb state */
    if (l_iUsbStat != GRUSB_COMD_CON)                   /* if state is the disconnect       */
    {
        l_iUsbStat = GRUSB_COMD_CON;                    /* state is set to the connect      */

        GRCOMD_LOG( GRDBG_COMD, 0x13, 0x00, 0x01 );
        if (l_tInitInfo.pfnConnStat)
        {
            l_tInitInfo.pfnConnStat(GRUSB_COMD_CON);    /* Callback function is called      */
    }
    }
    GRCOMD_LOG( GRDBG_COMD, 0x13, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbDisConn                                       */
/*                                                                          */
/* DESCRIPTION: Notice processing of Disconnection.                         */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbDisConn( VOID )
{
    GRCOMD_LOG( GRDBG_COMD, 0x14, 0x00, 0x00 );

    /* check local usb state */
    if (l_iUsbStat != GRUSB_COMD_DISC)                  /* if state is the connect          */
    {
        l_iUsbStat = GRUSB_COMD_DISC;                   /* state is set to the disconnect   */

        GRCOMD_LOG( GRDBG_COMD, 0x14, 0x00, 0x01 );

        /* All transfers are aborted */
        GRUSB_DEV_ApAbort( GRCOMD_EP1 );
        GRUSB_DEV_ApAbort( GRCOMD_EP2 );
        GRUSB_DEV_ApAbort( GRCOMD_EP3 );
        if (l_tInitInfo.pfnConnStat)
        {
            l_tInitInfo.pfnConnStat(GRUSB_COMD_DISC);   /* Callback function is called      */
    }
    }
    GRCOMD_LOG( GRDBG_COMD, 0x14, 0x00, END_FUNC );
}

