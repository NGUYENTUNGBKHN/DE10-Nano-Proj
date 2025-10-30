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
/*      com_hid2.c                                                1.00      */
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
#include    "com_hid_def.h"
#include    "com_hid_cnf.h"
#include    "com_hid.h"

#include "grusbtyp.h"

#ifdef GRCOMD_HID_DEBUG
#include    "dbg_mdl.h"
#define GRCOMD_HID_LOG2(m,n,x,y)  GRDBG_TRACE2(m,n,x,y)
#else
#define GRCOMD_HID_LOG2(m,n,x,y)
#define GRDBG_COMD                      (0x00)
#define END_FUNC                        (0x00)
#endif

/**** INTERNAL DATA DEFINES *************************************************/
/* Descriptor Information */
GRCOMD_HID_ABSTRACT_DESC*       l_ptAbstructDescHid2;
GRCOMD_HID_CALL_DESC*           l_ptCallManageDescHid2;

GRCOMD_HID_NOTIFICATION                 l_tNetworkConnectionHid2 __attribute__ ((section (".uncache"), aligned (4), zero_init));                   /* V1.32 */
GRCOMD_HID_NOTIFICATION                 l_tResponseAvailableHid2 __attribute__ ((section (".uncache"), aligned (4), zero_init));                   /* V1.32 */
GRCOMD_HID_NOTIFICATION_SERIAL_STATE    l_tSerialStateHid2 __attribute__ ((section (".uncache"), aligned (4), zero_init));                         /* V1.32 */

DLOCAL INT16    l_usRespSz2;
DLOCAL UINT32   l_aulCntlArea2[GRCOMD_HID_CTRLAREASIZE/GRCOMD_HID_BOUNDERY];
DLOCAL UINT8    l_aucCommDt2[GRCOMD_HID_MAX_DT_SZ];
DLOCAL INT      l_iUsbStat2;
DLOCAL UINT8*   l_pucStrDesc2 = GRUSB_NULL;

/* APPLICATION CALLBACK FUNCTION PROTOTYPE */
LOCAL GRUSB_COMD_HID_INITINFO   l_tInitInfo2;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
LOCAL INT                       _GRUSB_COMD_HID_Setting2( VOID );

LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_HID_GetDevDescFS2( VOID );
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_HID_GetDevDescHS2( VOID );
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_HID_GetCnfgDescFS2( VOID );
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_HID_GetCnfgDescHS2( VOID );


LOCAL VOID _GRUSB_COMD_HID_CbNtwrkConn2( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_HID_CbRespAvab2( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_HID_CbSrlStat2( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_HID_CbSndData2( INT, UINT8*, UINT32, VOID*, INT );
INT  _GRUSB_COMD_HID_CbRcvData2( INT, UINT8*, UINT32, VOID*, INT );
LOCAL VOID _GRUSB_COMD_HID_CbDevRequest2( UINT8, UINT8, UINT16, UINT16, UINT16 );
LOCAL INT  _GRUSB_COMD_HID_CbSetCommFeature2( INT, UINT8*, UINT32, VOID*, INT );
LOCAL INT  _GRUSB_COMD_HID_CbSndEncpslCmnd2( INT, UINT8*, UINT32, VOID*, INT );
LOCAL INT  _GRUSB_COMD_HID_CbSetLineCoding2( INT, UINT8*, UINT32, VOID*, INT );

LOCAL VOID _GRUSB_COMD_HID_CbConn2( VOID );
LOCAL VOID _GRUSB_COMD_HID_CbDisConn2( VOID );

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Init2                                        */
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
INT GRUSB_COMD_HID_Init2( GRUSB_COMD_HID_INITINFO*   ptInitInfo )
{
    INT iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x00, 0x00, 0x00 );

    /* Initialization of an internal variable */
    l_usRespSz2 = 0;
    l_iUsbStat2 = GRUSB_COMD_HID_DISC;

    /* Initialization of a Callback Function */
    l_tInitInfo2.pfnConnStat                         = GRUSB_NULL;
    l_tInitInfo2.pfnSendEncapsulatedCmd              = GRUSB_NULL;
    l_tInitInfo2.pfnGetEncapsulatedRes               = GRUSB_NULL;
    l_tInitInfo2.pfnSetCommFeature                   = GRUSB_NULL;
    l_tInitInfo2.pfnGetCommFeature                   = GRUSB_NULL;
    l_tInitInfo2.pfnClearCommFeature                 = GRUSB_NULL;
    l_tInitInfo2.pfnSetLineCoding                    = GRUSB_NULL;
    l_tInitInfo2.pfnGetLineCoding                    = GRUSB_NULL;
    l_tInitInfo2.pfnSetControlLineState              = GRUSB_NULL;
    l_tInitInfo2.pfnSendBreak                        = GRUSB_NULL;
    l_tInitInfo2.pfnNetworkConnection                = GRUSB_NULL;
    l_tInitInfo2.pfnResponseAvailable                = GRUSB_NULL;
    l_tInitInfo2.pfnSerialState                      = GRUSB_NULL;
    l_tInitInfo2.pfnSendData                         = GRUSB_NULL;
    l_tInitInfo2.pfnReciveData                       = GRUSB_NULL;

    /* set of Callback Functions */
    if( ptInitInfo->pfnConnStat )
        l_tInitInfo2.pfnConnStat = ptInitInfo->pfnConnStat;
    if( ptInitInfo->pfnSendEncapsulatedCmd )
        l_tInitInfo2.pfnSendEncapsulatedCmd = ptInitInfo->pfnSendEncapsulatedCmd;
    if( ptInitInfo->pfnGetEncapsulatedRes )
        l_tInitInfo2.pfnGetEncapsulatedRes = ptInitInfo->pfnGetEncapsulatedRes;
    if( ptInitInfo->pfnSetCommFeature )
        l_tInitInfo2.pfnSetCommFeature = ptInitInfo->pfnSetCommFeature;
    if( ptInitInfo->pfnGetCommFeature )
        l_tInitInfo2.pfnGetCommFeature = ptInitInfo->pfnGetCommFeature;
    if( ptInitInfo->pfnClearCommFeature )
        l_tInitInfo2.pfnClearCommFeature = ptInitInfo->pfnClearCommFeature;
    if( ptInitInfo->pfnSetLineCoding )
        l_tInitInfo2.pfnSetLineCoding = ptInitInfo->pfnSetLineCoding;
    if( ptInitInfo->pfnGetLineCoding )
        l_tInitInfo2.pfnGetLineCoding = ptInitInfo->pfnGetLineCoding;
    if( ptInitInfo->pfnSetControlLineState )
        l_tInitInfo2.pfnSetControlLineState = ptInitInfo->pfnSetControlLineState;
    if( ptInitInfo->pfnSendBreak )
        l_tInitInfo2.pfnSendBreak = ptInitInfo->pfnSendBreak;
    if( ptInitInfo->pfnNetworkConnection )
        l_tInitInfo2.pfnNetworkConnection = ptInitInfo->pfnNetworkConnection;
    if( ptInitInfo->pfnResponseAvailable )
        l_tInitInfo2.pfnResponseAvailable = ptInitInfo->pfnResponseAvailable;
    if( ptInitInfo->pfnSerialState )
        l_tInitInfo2.pfnSerialState = ptInitInfo->pfnSerialState;
    if( ptInitInfo->pfnSendData )
        l_tInitInfo2.pfnSendData = ptInitInfo->pfnSendData;
    if( ptInitInfo->pfnReciveData )
        l_tInitInfo2.pfnReciveData = ptInitInfo->pfnReciveData;
    //2022-08-05
    if( ptInitInfo->pfnHidDetach )
        l_tInitInfo2.pfnHidDetach = ptInitInfo->pfnHidDetach;


    /* Initialize of COMM Driver */
    iStat = _GRUSB_COMD_HID_Setting2();

    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x00, 0x01, END_FUNC );
        return GRUSB_COMD_HID_ERROR;
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x00, 0x00, END_FUNC );
    return GRUSB_COMD_HID_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_Setting2                                    */
/*                                                                          */
/* DESCRIPTION: Initial value registration to a Peripheral Driver.          */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : iRet                Initialization result of USBD driver    */
/*                                                                          */
/****************************************************************************/
LOCAL INT  _GRUSB_COMD_HID_Setting2( VOID )
{
    GRUSB_DEV_INITINFO      tIniPrm;
    GRUSB_DEV_EPINITINFO*   ptEpInfo;
#ifdef _GRUSB_COMD_HID_DEBUG
    UINT16                  usQue[GRCOMD_HID_NUM_ENDPOINTS];
#endif
    INT                     iRet;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x01, 0x00, 0x00 );

    /* Parameter setup */
    tIniPrm.ptDeviceDesc[0] = _GRUSB_COMD_HID_GetDevDescFS2();   /* Device Descriptor value          */
    tIniPrm.ptDeviceDesc[1] = _GRUSB_COMD_HID_GetDevDescHS2();   /* Device Descriptor value          */
    tIniPrm.usConfigDescNum = GRCOMD_HID_NUM_CONFIGURATIONS;    /* Configuration Descriptor number  */
    tIniPrm.ptConfigDesc[0] = _GRUSB_COMD_HID_GetCnfgDescFS2();  /* Configuration Descriptor value   */
    tIniPrm.ptConfigDesc[1] = _GRUSB_COMD_HID_GetCnfgDescHS2();  /* Configuration Descriptor value   */

    /* FullとHigh違いがないので、Full */
    l_ptAbstructDescHid2   = &(((GRCOMD_HID_CLASS_DESC *)tIniPrm.ptConfigDesc[0])->tAbstract);
    l_ptCallManageDescHid2 = &(((GRCOMD_HID_CLASS_DESC *)tIniPrm.ptConfigDesc[0])->tCall);

#ifdef _GRUSB_COMD_HID_DEBUG
    usQue[0] = GRCOMD_HID_CTRL_BUF;                 /* The number of Control transfer buffer    */
    usQue[1] = GRCOMD_HID_EP1_DATABUF;              /* The number of Data transfer buffer       */
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_CFGDESC_NUMINTERFACES != 0x01 )
    usQue[2] = GRCOMD_HID_EP2_DATABUF;                  /* The number of Data transfer buffer       */
    usQue[3] = GRCOMD_HID_EP3_DATABUF;                  /* The number of Data transfer buffer       */
#endif

    /* The check of control area size */
    if (!(GRUSB_DEV_ControlAreaSize2(GRCOMD_HID_NUM_ENDPOINTS, usQue) < GRCOMD_HID_CTRLAREASIZE))
    {
        /* GRCOMD_HID_CTRLAREASIZE is not a suitable value when it enters here. */
        /*  Please improve GRCOMD_HID_CTRLAREASIZE                              */
        while(1);   /* error */
    }
#endif /* _GRUSB_COMD_DEBUG */

    tIniPrm.pucCntrlArea    = (UINT8 *)l_aulCntlArea2;           /* The pointer of Control Area      */
    tIniPrm.iEpNum          = GRCOMD_HID_NUM_ENDPOINTS;          /* The number of EndPoints          */
    tIniPrm.ucStringDescNum = GRCOMD_HID_STRING_DESC_NUM;        /* String Descriptor number         */
    if( tIniPrm.ucStringDescNum == 0 )
    {
        /* No String Descriptor */
        tIniPrm.pStringDesc     = GRUSB_NULL;
    }
    else
    {   /* Get String Descriptor */
        tIniPrm.pStringDesc = GRUSB_COMD_HID_GetStrDesc2();         /* String Descriptor value          */
    }
    
    /* Save the pointer of the String Descriptor */
    l_pucStrDesc2 = (UINT8*)tIniPrm.pStringDesc;

    /* Setup of Endpoint information */
    ptEpInfo = &tIniPrm.atEpInfo[0];

    /*----------------------------------*/
    /* Endpoint 0 Information           */
    ptEpInfo->usNumOfQue    = GRCOMD_HID_CTRL_BUF;          /* The number of Buffer                 */
    ptEpInfo->ucPages       = GRCOMD_HID_EP0_PAGENUM;       /* FIFO Size                            */
    ptEpInfo->usMaxPktSz[0] = GRCOMD_HID_EP0_MAXPKTSIZE_FS; /* Max Packet Size                      */
    ptEpInfo->usMaxPktSz[1] = GRCOMD_HID_EP0_MAXPKTSIZE_HS; /* Max Packet Size                      */
    ptEpInfo->ucEpType      = EPTYPE_CONTROL;               /* End Point Type                       */
    ptEpInfo->pfnTransEnd   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  the completion of transmitting      */
    ptEpInfo->pfnTransErr   = GRUSB_NULL;                   /* The function which notifies          */
                                                            /*  Transmission Error                  */
    ptEpInfo++;                                             /*-- Shift to the following point --*/
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_HID_CFGDESC_NUMINTERFACES != 0x01 )
    /*----------------------------------*/
    /* End Point 1 information          */                                                              /* V1.22 */
    ptEpInfo->usNumOfQue    = GRCOMD_HID_EP1_DATABUF;           /* The number of Transmission Buffer    */  /* V1.22 */
    ptEpInfo->ucPages       = GRCOMD_HID_EP1_PAGENUM;           /* FIFO Size                            */  /* V1.22 */
    ptEpInfo->usMaxPktSz[0] = GRCOMD_HID_EP1_MAXPKTSIZE_FS;     /* Max Packet Size                      */  /* V1.22 */
    ptEpInfo->usMaxPktSz[1] = GRCOMD_HID_EP1_MAXPKTSIZE_HS;     /* Max Packet Size                      */  /* V1.22 */
    ptEpInfo->ucEpType      = EPTYPE_INTERRUPT                  /* End Point Type                       */
                              | EPTYPE_IN;                      /* The transmission direction           */  /* V1.22 */
    ptEpInfo->pfnTransEnd   = GRUSB_NULL;                       /* The function which notifies          */
                                                                /*  the completion of transmitting      */
    ptEpInfo->pfnTransErr   = GRUSB_NULL;                       /* The function which notifies          */
                                                                /*  Transmission Error                  */
    ptEpInfo++;                                                 /*--- Shift to the following point ---*/
#endif
    tIniPrm.usReqCodeMap    = 0x0000;       /* The Device Request code notified to a upper layer    */
    tIniPrm.pfnDevRquest    = _GRUSB_COMD_HID_CbDevRequest2;
                                                /* Notice function of Device Request reception      */
    tIniPrm.pfnSndCancel    = GRUSB_NULL;       /* Notice function of Control transmitting abort    */
    tIniPrm.pfnRcvCancel    = GRUSB_NULL;       /* Notice function of Control reception abort       */
    tIniPrm.pfnBusReset     = _GRUSB_COMD_HID_CbDisConn2;
                                                /* Notice function of Bus Reset reception           */
    tIniPrm.pfnClearFeature = GRUSB_NULL;       /* Notice function of Clear Future reception        */
    tIniPrm.pfnResume       = GRUSB_NULL;       /* Notice function of Resume reception              */
    tIniPrm.pfnSuspend      = GRUSB_NULL;       /* Notice function of Suspend reception             */
    tIniPrm.pfnEnbRmtWkup   = GRUSB_NULL;       /* Notice function of Remote Wakeup reception       */

    tIniPrm.pfnCmpSetInterface      = GRUSB_NULL;
                                            /* Notice function of Set Interface reception           */
    tIniPrm.pfnCmpSetConfiguration  = _GRUSB_COMD_HID_CbConn2;
                                            /* Notice function of Set Configuration reception       */
    tIniPrm.pfnCngStateNoConfigured = _GRUSB_COMD_HID_CbDisConn2;
                                            /* Notice function of the state changes from Configured */

    /* Initialization of USB Driver */
    iRet = GRUSB_DEV_ApInit2(&tIniPrm);

    /* Callback function called at the time of the notice of connection/disconnection is registered */
    /* A connection notice is given at the time of SetConfiguration reception                       */
    GRUSB_DEV_ApCallbackConnect2( GRUSB_NULL );                                      /* Connect      */
    GRUSB_DEV_ApCallbackDisconnect2(_GRUSB_COMD_HID_CbDisConn2);                          /* Disconnect   */

    /* Enable HID */
    GRUSB_DEV_ApSetHIDSupportFlg2( GRUSB_TRUE );

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x01, 0x00, END_FUNC );
    return iRet;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_GetDevDescFS2                                   */
/*                                                                          */
/* DESCRIPTION: Device Descriptor is returned.                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Device descriptor information           */
/*                                                                          */
/****************************************************************************/
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_HID_GetDevDescFS2( VOID )
{
    DLOCAL const union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                     /* Device Descriptor                        */
        UINT32                  ulDummy;                   /* In order to assign to a 4-byte boundary  */
    } uData = {
        GRCOMD_HID_DEVDESC_LENGTH                        , /* bLength                                  */
        GRCOMD_HID_DEVDESC_DESCRIPTORTYPE                , /* bDescriptorType                          */
        GRCOMD_HID_CNF_USB_LSB, GRCOMD_HID_CNF_USB_MSB   , /* bcdUSB                                   */
        GRCOMD_HID_DEVDESC_DEVICECLASS                   , /* bDeviceClass (Communication Device Class)*/
        GRCOMD_HID_DEVDESC_DEVICESUBCLASS                , /* bDeviceSubClass                          */
        GRCOMD_HID_DEVDESC_DEVICEPROTOCOL                , /* bDeviceProtocol                          */
        GRCOMD_HID_DEVDESC_MAXPKTSIZE0_FS                , /* bMaxPacketSize0                          */
        GRCOMD_HID_CNF_VID_LSB, GRCOMD_HID_CNF_VID_MSB   , /* idVendor                                 */
        GRCOMD_HID_CNF_PID_LSB2, GRCOMD_HID_CNF_PID_MSB2 , /* idProduct                                */
        GRCOMD_HID_CNF_DEV_LSB, GRCOMD_HID_CNF_DEV_MSB   , /* bcdDevice                                */
        GRCOMD_HID_CNF_MANUFACTURER                      , /* iManufacturer                            */
        GRCOMD_HID_CNF_PRODUCT                           , /* iProduct                                 */
        GRCOMD_HID_CNF_SERIALNUMBER                      , /* iSerialNumber                            */
        GRCOMD_HID_DEVDESC_NUMCONFIGURATIONS               /* bNumConfigurations                       */
    };

    return  (GRUSB_DEV_DEVICE_DESC*)&(uData.tDesc);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_GetDevDescHS2                               */
/*                                                                          */
/* DESCRIPTION: Device Descriptor is returned.                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Device descriptor information           */
/*                                                                          */
/****************************************************************************/
LOCAL GRUSB_DEV_DEVICE_DESC*    _GRUSB_COMD_HID_GetDevDescHS2( VOID )
{
#ifdef GRUSB_DEV_FIXED_FS

    return GRUSB_NULL;
#else /* GRUSB_DEV_FIXED_FS */

    DLOCAL const union {
        GRUSB_DEV_DEVICE_DESC   tDesc;                      /* Device Descriptor                        */
        UINT32                  ulDummy;                    /* In order to assign to a 4-byte boundary  */
    } uData = {
        GRCOMD_HID_DEVDESC_LENGTH                        , /* bLength                                  */
        GRCOMD_HID_DEVDESC_DESCRIPTORTYPE                , /* bDescriptorType                          */
        GRCOMD_HID_CNF_USB_LSB, GRCOMD_HID_CNF_USB_MSB   , /* bcdUSB                                   */
        GRCOMD_HID_DEVDESC_DEVICECLASS                   , /* bDeviceClass (Communication Device Class)*/
        GRCOMD_HID_DEVDESC_DEVICESUBCLASS                , /* bDeviceSubClass                          */
        GRCOMD_HID_DEVDESC_DEVICEPROTOCOL                , /* bDeviceProtocol                          */
        GRCOMD_HID_DEVDESC_MAXPKTSIZE0_HS                , /* bMaxPacketSize0                          */
        GRCOMD_HID_CNF_VID_LSB, GRCOMD_HID_CNF_VID_MSB   , /* idVendor                                 */
        GRCOMD_HID_CNF_PID_LSB2, GRCOMD_HID_CNF_PID_MSB2 , /* idProduct                                */
        GRCOMD_HID_CNF_DEV_LSB, GRCOMD_HID_CNF_DEV_MSB   , /* bcdDevice                                */
        GRCOMD_HID_CNF_MANUFACTURER                      , /* iManufacturer                            */
        GRCOMD_HID_CNF_PRODUCT                           , /* iProduct                                 */
        GRCOMD_HID_CNF_SERIALNUMBER                      , /* iSerialNumber                            */
        GRCOMD_HID_DEVDESC_NUMCONFIGURATIONS               /* bNumConfigurations                       */
    };
    return  (GRUSB_DEV_DEVICE_DESC*)&(uData.tDesc);
#endif /* GRUSB_DEV_FIXED_FS */
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_GetCnfgDescFS2                              */
/*                                                                          */
/* DESCRIPTION: Configuration Descriptor is returned.                       */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Configuration descriptor information    */
/*                                                                          */
/****************************************************************************/
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_HID_GetCnfgDescFS2( VOID )
{
    DLOCAL const union {
        struct {
        GRUSB_DEV_CONFIG_DESC        tConfig;            /* Configuration Descriptor                 */
        GRUSB_DEV_INTERFACE_DESC     tInterface0;        /* Interface Descriptor 0                   */
        GRUSB_HID_DEV_DFU_DESC       tDfu;               /* DFU Functional escriptor                 */
        GRUSB_DEV_INTERFACE_DESC     tInterface2;        /* Interface Descriptor 0                   */
        GRUSB_HID_DEV_HID_DESC       tHid;               /* HID Functional escriptor                 */
        GRUSB_DEV_ENDPOINT_DESC      tEndPoint1;         /* Endpoint Descriptor 1                    */
        } tDesc;
        UINT32                      dummy;              /* In order to assign to a 4-byte boundary  */
    } uData = {
            /*--- Standard Configuration Descriptor ---*/
            GRCOMD_HID_CFGDESC_LENGTH               ,   /* bLength                      (0x09)          */
            GRCOMD_HID_CFGDESC_DESCRIPTORTYPE       ,   /* bDescriptorType              (0x02)          */
            GRCOMD_HID_CFGDESC_TOTALLENGTH_L        ,   /* wTotalLength (The size of                    */
            GRCOMD_HID_CFGDESC_TOTALLENGTH_H        ,   /*  a tDesc structure object)                   */
            GRCOMD_HID_CFGDESC_NUMINTERFACES        ,   /* bNumInterfaces               (0x01 or 0x02)  */
            GRCOMD_HID_CFGDESC_CONFIGURATIONVALUE   ,   /* bConfigurationValue          (0x01)          */
            GRCOMD_HID_CFGDESC_CONFIGURATION        ,   /* iConfiguration               (0x04)          */
            GRCOMD_HID_CNF_ATTRIBUTES               ,   /* bmAttributes                                 */
            GRCOMD_HID_CNF_MAXPOWER                 ,   /* MaxPower                                     */
            /*--- Standard Interface Descriptor 0 ---*/
            GRCOMD_HID_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_HID_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_HID_IFDESC0_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x00)          */
            GRCOMD_HID_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_HID_IFDESC0_NUMENDPOINTS         ,   /* bNumEndpoints(EP3)           (0x00)          */
            GRCOMD_HID_IFDESC0_INTERFACECLASS       ,   /* bInterfaceClass              (0xFE)          */
            GRCOMD_HID_IFDESC0_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x01)          */
            GRCOMD_HID_IFDESC0_INTERFACEPROTOCOL    ,   /* bInterfaceProtocol           (0x01)          */
            GRCOMD_HID_IFDESC0_INTERFACE            ,   /* iInterface                   (0x05)          */
            /*--- DFU Functional Descriptor ---*/
            GRCOMD_HID_DFUFDESC_LENGTH              ,   /* bLength                      (0x09)          */
            GRCOMD_HID_DFUFDESC_DESCRIPTORTYPE      ,   /* bDescriptorType              (0x21)          */
            GRCOMD_HID_DFUFDESC_ATTRIBUTES          ,   /* bmAttributes                 (0x03)          */
            GRCOMD_HID_DFUFDESC_DETACHTIMEOUT_L     ,   /* wDetachTimeOut_lo            (0x10)          */
            GRCOMD_HID_DFUFDESC_DETACHTIMEOUT_H     ,   /* wDetachTimeOut_hi            (0x27)          */
            GRCOMD_HID_DFUFDESC_TRANFERSIZE_L       ,   /* wMaxTranferSize_lo           (0x00)          */
            GRCOMD_HID_DFUFDESC_TRANFERSIZE_H       ,   /* wMaxTranferSize_hi           (0x10)          */
            GRCOMD_HID_DFUFDESC_DFUVERSION_L        ,   /* bcdDFUVersion_lo             (0x10)          */
            GRCOMD_HID_DFUFDESC_DFUVERSION_H        ,   /* bcdDFUVersion_hi             (0x01)          */
            /*--- Standard Interface Descriptor 1 ---*/
            GRCOMD_HID_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_HID_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_HID_IFDESC1_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x01)          */
            GRCOMD_HID_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_HID_IFDESC1_NUMENDPOINTS         ,   /* bNumEndpoints(EP3)           (0x01)          */
            GRCOMD_HID_IFDESC1_INTERFACECLASS       ,   /* bInterfaceClass              (0x03)          */
            GRCOMD_HID_IFDESC1_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x00)          */
            GRCOMD_HID_IFDESC_INTERFACEPROTOCOL     ,   /* bInterfaceProtocol           (0x00)          */
            GRCOMD_HID_IFDESC1_INTERFACE            ,   /* iInterface                   (0x06)          */
            /*--- HID Descriptor ---*/
            GRCOMD_HID_HIDDESC_LENGTH               ,   /* bLength                      (0x09)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORTYPE       ,   /* bDescriptorType              (0x21)          */
            GRCOMD_HID_HIDDESC_HID_L                ,   /* bcdHID_lo                    (0x11)          */
            GRCOMD_HID_HIDDESC_HID_H                ,   /* bcdHID_hi                    (0x01)          */
            GRCOMD_HID_HIDDESC_COUNTRYCODE          ,   /* bCountryCode                 (0x00)          */
            GRCOMD_HID_HIDDESC_NUMDESCRIPTORS       ,   /* bNumDescriptors              (0x01)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORTYPE0      ,   /* bDescriptorType[0]           (0x22)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORLENGTH_L   ,   /* wDescriptorLength[0]_lo      (0x5E)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORLENGTH_H   ,   /* wDescriptorLength[0]_hi      (0x03)          */
            /*--- Endpoint Descriptor ---*/
            GRCOMD_HID_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_HID_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_HID_EPDESC_EP1_ADDRESS           ,   /* bEndpointAddress             (0x81)          */
            GRCOMD_HID_EPDESC_ATTRIBUTES_INT        ,   /* bmAttributes                 (0x03)          */
            GRCOMD_HID_EPDESC_EP1_MAXPKTSIZE_L_FS   ,   /* wMaxPacketSize                               */
            GRCOMD_HID_EPDESC_EP1_MAXPKTSIZE_H_FS   ,   /*                                              */
            GRCOMD_HID_CNF_INTERVAL_INTERRUPT_FS        /* bInterval                    (0x01 - 0xFF)   */
    };

    return (GRUSB_DEV_CONFIG_DESC*)&(uData.tDesc);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_GetCnfgDescHS2                              */
/*                                                                          */
/* DESCRIPTION: Configuration Descriptor is returned.                       */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc         Configuration descriptor information    */
/*                                                                          */
/****************************************************************************/
LOCAL GRUSB_DEV_CONFIG_DESC*    _GRUSB_COMD_HID_GetCnfgDescHS2( VOID )
{
#ifdef GRUSB_DEV_FIXED_FS

    return GRUSB_NULL;
#else /* GRUSB_DEV_FIXED_FS */

    DLOCAL const union {
        struct {
        GRUSB_DEV_CONFIG_DESC        tConfig;            /* Configuration Descriptor                 */
        GRUSB_DEV_INTERFACE_DESC     tInterface0;        /* Interface Descriptor 0                   */
        GRUSB_HID_DEV_DFU_DESC       tDfu;               /* DFU Functional escriptor                 */
        GRUSB_DEV_INTERFACE_DESC     tInterface2;        /* Interface Descriptor 0                   */
        GRUSB_HID_DEV_HID_DESC       tHid;               /* HID Functional escriptor                 */
        GRUSB_DEV_ENDPOINT_DESC      tEndPoint1;         /* Endpoint Descriptor 1                    */
        } tDesc;
        UINT32                      dummy;              /* In order to assign to a 4-byte boundary  */
    } uData = {
            /*--- Standard Configuration Descriptor ---*/
            GRCOMD_HID_CFGDESC_LENGTH               ,   /* bLength                      (0x09)          */
            GRCOMD_HID_CFGDESC_DESCRIPTORTYPE       ,   /* bDescriptorType              (0x02)          */
            GRCOMD_HID_CFGDESC_TOTALLENGTH_L        ,   /* wTotalLength (The size of                    */
            GRCOMD_HID_CFGDESC_TOTALLENGTH_H        ,   /*  a tDesc structure object)                   */
            GRCOMD_HID_CFGDESC_NUMINTERFACES        ,   /* bNumInterfaces               (0x01 or 0x02)  */
            GRCOMD_HID_CFGDESC_CONFIGURATIONVALUE   ,   /* bConfigurationValue          (0x01)          */
            GRCOMD_HID_CFGDESC_CONFIGURATION        ,   /* iConfiguration               (0x04)          */
            GRCOMD_HID_CNF_ATTRIBUTES               ,   /* bmAttributes                                 */
            GRCOMD_HID_CNF_MAXPOWER                 ,   /* MaxPower                                     */
            /*--- Standard Interface Descriptor 0 ---*/
            GRCOMD_HID_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_HID_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_HID_IFDESC0_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x00)          */
            GRCOMD_HID_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_HID_IFDESC0_NUMENDPOINTS         ,   /* bNumEndpoints(EP3)           (0x00)          */
            GRCOMD_HID_IFDESC0_INTERFACECLASS       ,   /* bInterfaceClass              (0xFE)          */
            GRCOMD_HID_IFDESC0_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x01)          */
            GRCOMD_HID_IFDESC0_INTERFACEPROTOCOL    ,   /* bInterfaceProtocol           (0x01)          */
            GRCOMD_HID_IFDESC0_INTERFACE            ,   /* iInterface                   (0x05)          */
            /*--- DFU Functional Descriptor ---*/
            GRCOMD_HID_DFUFDESC_LENGTH              ,   /* bLength                      (0x09)          */
            GRCOMD_HID_DFUFDESC_DESCRIPTORTYPE      ,   /* bDescriptorType              (0x21)          */
            GRCOMD_HID_DFUFDESC_ATTRIBUTES          ,   /* bmAttributes                 (0x03)          */
            GRCOMD_HID_DFUFDESC_DETACHTIMEOUT_L     ,   /* wDetachTimeOut_lo            (0x10)          */
            GRCOMD_HID_DFUFDESC_DETACHTIMEOUT_H     ,   /* wDetachTimeOut_hi            (0x27)          */
            GRCOMD_HID_DFUFDESC_TRANFERSIZE_L       ,   /* wMaxTranferSize_lo           (0x00)          */
            GRCOMD_HID_DFUFDESC_TRANFERSIZE_H       ,   /* wMaxTranferSize_hi           (0x10)          */
            GRCOMD_HID_DFUFDESC_DFUVERSION_L        ,   /* bcdDFUVersion_lo             (0x10)          */
            GRCOMD_HID_DFUFDESC_DFUVERSION_H        ,   /* bcdDFUVersion_hi             (0x01)          */
            /*--- Standard Interface Descriptor 1 ---*/
            GRCOMD_HID_IFDESC_LENGTH                ,   /* bLength                      (0x09)          */
            GRCOMD_HID_IFDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x04)          */
            GRCOMD_HID_IFDESC1_INTERFACENUMBER      ,   /* bInterfaceNumber             (0x01)          */
            GRCOMD_HID_IFDESC_ALTERNATESETTING      ,   /* bAlternateSetting            (0x00)          */
            GRCOMD_HID_IFDESC1_NUMENDPOINTS         ,   /* bNumEndpoints(EP3)           (0x01)          */
            GRCOMD_HID_IFDESC1_INTERFACECLASS       ,   /* bInterfaceClass              (0x03)          */
            GRCOMD_HID_IFDESC1_INTERFACESUBCLASS    ,   /* bInterfaceSubClass           (0x00)          */
            GRCOMD_HID_IFDESC_INTERFACEPROTOCOL     ,   /* bInterfaceProtocol           (0x00)          */
            GRCOMD_HID_IFDESC1_INTERFACE            ,   /* iInterface                   (0x06)          */
            /*--- HID Descriptor ---*/
            GRCOMD_HID_HIDDESC_LENGTH               ,   /* bLength                      (0x09)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORTYPE       ,   /* bDescriptorType              (0x21)          */
            GRCOMD_HID_HIDDESC_HID_L                ,   /* bcdHID_lo                    (0x11)          */
            GRCOMD_HID_HIDDESC_HID_H                ,   /* bcdHID_hi                    (0x01)          */
            GRCOMD_HID_HIDDESC_COUNTRYCODE          ,   /* bCountryCode                 (0x00)          */
            GRCOMD_HID_HIDDESC_NUMDESCRIPTORS       ,   /* bNumDescriptors              (0x01)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORTYPE0      ,   /* bDescriptorType[0]           (0x22)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORLENGTH_L   ,   /* wDescriptorLength[0]_lo      (0x5E)          */
            GRCOMD_HID_HIDDESC_DESCRIPTORLENGTH_H   ,   /* wDescriptorLength[0]_hi      (0x03)          */
            /*--- Endpoint Descriptor ---*/
            GRCOMD_HID_EPDESC_LENGTH                ,   /* bLength                      (0x07)          */
            GRCOMD_HID_EPDESC_DESCRIPTORTYPE        ,   /* bDescriptorType              (0x05)          */
            GRCOMD_HID_EPDESC_EP1_ADDRESS           ,   /* bEndpointAddress             (0x81)          */
            GRCOMD_HID_EPDESC_ATTRIBUTES_INT        ,   /* bmAttributes                 (0x03)          */
            GRCOMD_HID_EPDESC_EP1_MAXPKTSIZE_L_HS   ,   /* wMaxPacketSize                               */
            GRCOMD_HID_EPDESC_EP1_MAXPKTSIZE_H_HS   ,   /*                                              */
            GRCOMD_HID_CNF_INTERVAL_INTERRUPT_HS        /* bInterval                    (0x01 - 0xFF)   */
    };

    return (GRUSB_DEV_CONFIG_DESC*)&(uData.tDesc);
#endif /* GRUSB_DEV_FIXED_FS */
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_SetSerialNumber2                             */
/*                                                                          */
/* DESCRIPTION: Set the serial number of the user-specified.                */
/*--------------------------------------------------------------------------*/
/* INPUT      : pusSerialNumber         Pointer of the serial number to     */
/*                                      overwrite                           */
/*              ucSerialNumberChars     Number of characters in the         */
/*                                      serial number                       */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_PRM_ERR      Parameter error                 */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_SetSerialNumber2( UINT16* pusSerialNumber, UINT8 ucSerialNumberChars )
{
    INT     i;
    INT     iSerialNumber      = GRCOMD_HID_CNF_SERIALNUMBER;
    UINT32  ulPos              = 0;
    UINT16* pusSerialNumberStr = GRUSB_NULL;
    UINT8   ucStringDescNum    = GRCOMD_HID_STRING_DESC_NUM;
    UINT8   ucDescSize;
    UINT8   ucCurrentChars;
    UINT8   ucCnt;
    
    /* Check the current device state */
    if( GRUSB_DEV_STATE_IDLE != GRUSB_DEV_ApGetDeviceState2() )
    {
        /* Unusual end (Not IDLE(Attached/Powered) state) */
        return GRUSB_COMD_HID_ERROR;
    }
    
    /* Check the String Descriptor */
    if( ( 0 == ucStringDescNum ) || ( GRUSB_NULL == l_pucStrDesc2 ) )
    {
        /* Unusual end (String Descriptor not set) */
        return GRUSB_COMD_HID_ERROR;
    }
    
    /* Check the index of serial number */
    if( 0 == iSerialNumber )
    {
        /* Unusual end (Index of serial number not set) */
        return GRUSB_COMD_HID_ERROR;
    }
    
    /* Check the parameters */
    if( ( GRUSB_NULL == pusSerialNumber ) || ( 0 == ucSerialNumberChars ) )
    {
        /* Parameter error (No serial number to overwrite) */
        return GRUSB_COMD_HID_PRM_ERR;
    }

    /* Calculated start position of the String Descriptor for serial number */
    for( i = 0; i < iSerialNumber; i++ )
    {
        /* Added the size of the String Descriptor */
        ulPos += *( l_pucStrDesc2 + ulPos );
    }
    
    /* Get the size of the String Descriptor for serial number */
    ucDescSize = *( l_pucStrDesc2 + ulPos );
    /* Calculated the number of characters in the current serial number */
    ucCurrentChars = ( ( ucDescSize - 2 ) / sizeof(UINT16) );
    /* Check the number of characters in the serial number */
    if( ucCurrentChars != ucSerialNumberChars )
    {
        /* Parameter error (Number of characters is mismatch) */
        return GRUSB_COMD_HID_PRM_ERR;
    }
    
    /* Get the start position of the serial number */
    pusSerialNumberStr = (UINT16*)( l_pucStrDesc2 + ulPos + 2 );
    
    /* Overwrite the serial number */
    for( ucCnt = 0; ucCnt < ucSerialNumberChars; ucCnt++ )
    {
        *( pusSerialNumberStr + ucCnt ) = GRCOMD_HID_MAC_SWAPUS( *( pusSerialNumber + ucCnt ) );
    }
    
    return GRUSB_COMD_HID_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_SetInterface2                                */
/*                                                                          */
/* DESCRIPTION: Set the interface of the user-specified.                    */
/*--------------------------------------------------------------------------*/
/* INPUT      : pusInterface            Pointer of the interface to         */
/*                                      overwrite                           */
/*              ucInterfaceChars        Number of characters in the         */
/*                                      interface                           */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_PRM_ERR      Parameter error                 */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_SetInterface2( UINT16* pusInterface, UINT8 ucInterfaceChars )
{
    INT     i;
    INT     iInterface       = GRCOMD_HID_CNF_INTERFACE;
    UINT32  ulPos            = 0;
    UINT16* pusInterfaceStr  = GRUSB_NULL;
    UINT8   ucStringDescNum  = GRCOMD_HID_STRING_DESC_NUM;
    UINT8   ucDescSize;
    UINT8   ucCurrentChars;
    UINT8   ucCnt;
    
    /* Check the current device state */
    if( GRUSB_DEV_STATE_IDLE != GRUSB_DEV_ApGetDeviceState2() )
    {
        /* Unusual end (Not IDLE(Attached/Powered) state) */
        return GRUSB_COMD_HID_ERROR;
    }
    
    /* Check the String Descriptor */
    if( ( 0 == ucStringDescNum ) || ( GRUSB_NULL == l_pucStrDesc2 ) )
    {
        /* Unusual end (String Descriptor not set) */
        return GRUSB_COMD_HID_ERROR;
    }
    
    /* Check the index of serial number */
    if( 0 == iInterface )
    {
        /* Unusual end (Index of serial number not set) */
        return GRUSB_COMD_HID_ERROR;
    }
    
    /* Check the parameters */
    if( ( GRUSB_NULL == pusInterface ) || ( 0 == ucInterfaceChars ) )
    {
        /* Parameter error (No serial number to overwrite) */
        return GRUSB_COMD_HID_PRM_ERR;
    }

    /* Calculated start position of the String Descriptor for serial number */
    for( i = 0; i < iInterface; i++ )
    {
        /* Added the size of the String Descriptor */
        ulPos += *( l_pucStrDesc2 + ulPos );
    }
    
    /* Get the size of the String Descriptor for serial number */
    ucDescSize = *( l_pucStrDesc2 + ulPos );
    /* Calculated the number of characters in the current serial number */
    ucCurrentChars = ( ( ucDescSize - 2 ) / sizeof(UINT16) );
    /* Check the number of characters in the serial number */
    //if( ucCurrentChars != ucInterfaceChars )
    if( ucCurrentChars < ucInterfaceChars )
    {
        /* Parameter error (Number of characters is mismatch) */
        return GRUSB_COMD_HID_PRM_ERR;
    }
    
    /* Get the start position of the serial number */
    pusInterfaceStr = (UINT16*)( l_pucStrDesc2 + ulPos + 2 );
    
    /* Overwrite the serial number */
    for( ucCnt = 0; ucCnt < ucInterfaceChars; ucCnt++ )
    {
        *( pusInterfaceStr + ucCnt ) = GRCOMD_HID_MAC_SWAPUS( *( pusInterface + ucCnt ) );
    }
    
    return GRUSB_COMD_HID_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Notification_NetworkConnection2              */
/*                                                                          */
/* DESCRIPTION: Connection/Disonnection state is notified to a host using   */
/*              an Interrupt-In endpoint.                                   */
/*--------------------------------------------------------------------------*/
/* INPUT      : usConn              Network connection state                */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_DESC_ERROR   Discriptor setting error        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_Notification_NetworkConnection2( UINT16 usConn )
{
    INT                     iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x02, 0x00, 0x00 );

    /* It investigates whether Network Connection is supported from the Abstract descriptor */
    if ((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_NETWORK) == 0)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x02, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_DESC_ERROR);
    }

    /* Parameter setup */
    l_tNetworkConnectionHid2.bmRequestType = GRUSB_DEV_DATA_TRNS_DIR_DH         /* V1.32 */
                                       | GRUSB_DEV_CLASS_TYPE
                                       | GRUSB_INTERFACE_STATUS;
    l_tNetworkConnectionHid2.bNotification = GRCOMD_HID_NETWORK_CONNECTION;         /* V1.32 */
    l_tNetworkConnectionHid2.wValue        = GRCOMD_HID_MAC_SWAPUS(usConn);
    l_tNetworkConnectionHid2.wIndex        = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_NOTIFICATION_IF);
    l_tNetworkConnectionHid2.wLength       = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_NETWORK_CONNECTION_LENGTH);

    /* A connection state is notified to a host */
    iStat = GRUSB_DEV_ApTransferSend2(GRCOMD_HID_EP1,                            /* V1.22 */
                                     (UINT8 *)&l_tNetworkConnectionHid2,        /* V1.32 */
                                     (UINT32)GRCOMD_HID_NOTIFICATION_SIZE,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     _GRUSB_COMD_HID_CbNtwrkConn2);
    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x02, 0x02, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x02, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbNtwrkConn2                                */
/*                                                                          */
/* DESCRIPTION: The callback function of                                    */
/*              GRUSB_COMD_HID_Notification_NetworkConnection2.             */
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
LOCAL VOID _GRUSB_COMD_HID_CbNtwrkConn2( INT     iEpNo,
                                    UINT8*  pucBuf,
                                    UINT32  ulSize,
                                    VOID*   pAplInfo,
                                    INT     iStat )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x03, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnNetworkConnection)
        (*l_tInitInfo2.pfnNetworkConnection)();

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x03, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Notification_ResponseAvailable2              */
/*                                                                          */
/* DESCRIPTION: An end (GRCOMD_HID_RESPONSE_AVAILABLE) is notified to a     */
/*              host using an Interrupt-In endpoint.                        */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_Notification_ResponseAvailable2( VOID )
{
    INT                     iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x04, 0x00, 0x00 );

    /* Since it is indispensable, an Abstract descriptor is not investigated */

    /* Parameter setup */
    l_tResponseAvailableHid2.bmRequestType = GRUSB_DEV_DATA_TRNS_DIR_DH         /* V1.32 */
                                       | GRUSB_DEV_CLASS_TYPE
                                       | GRUSB_INTERFACE_STATUS;
    l_tResponseAvailableHid2.bNotification = GRCOMD_HID_RESPONSE_AVAILABLE;         /* V1.32 */
    l_tResponseAvailableHid2.wValue        = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_RESPONSE_AVAILABLE_VALUE);
    l_tResponseAvailableHid2.wIndex        = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_NOTIFICATION_IF);
    l_tResponseAvailableHid2.wLength       = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_RESPONSE_AVAILABLE_LENGTH);

    /* An end (GRCOMD_RESPONSE_AVAILABLE) is notified to a host */
    iStat = GRUSB_DEV_ApTransferSend2(GRCOMD_HID_EP1,                            /* V1.22 */
                                     (UINT8 *)&l_tResponseAvailableHid2,        /* V1.32 */
                                     (UINT32)GRCOMD_HID_NOTIFICATION_SIZE,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     _GRUSB_COMD_HID_CbRespAvab2);
    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x04, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x04, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbRespAvab2                                 */
/*                                                                          */
/* DESCRIPTION: The callback function of                                    */
/*              GRUSB_COMD_HID_Notification_ResponseAvailable2.             */
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
LOCAL VOID _GRUSB_COMD_HID_CbRespAvab2( INT      iEpNo,
                                        UINT8*   pucBuf,
                                        UINT32   ulSize,
                                        VOID*    pAplInfo,
                                        INT      iStat )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x05, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnResponseAvailable)
        (*l_tInitInfo2.pfnResponseAvailable)();

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x05, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Notification2                                */
/*                                                                          */
/* DESCRIPTION: The Request to send data                                    */
/*              using an Interrupt-In endpoint.                             */
/*              (Event inforamtion is notified to a host)                   */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucBuf              pointer of data buffer                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_Notification2( UINT32    ulSize,
                                  UINT8*    pucBuf )
{
    INT                                 iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x06, 0x00, 0x00 );

    /* A connection state is notified to a host */
    iStat = GRUSB_DEV_ApTransferSend2(GRCOMD_HID_EP1,
                                      pucBuf,
                                      ulSize,
                                      GRUSB_TRUE,
                                      GRUSB_NULL,
                                     _GRUSB_COMD_HID_CbSrlStat2);
    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x06, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x06, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Notification_SerialState2                    */
/*                                                                          */
/* DESCRIPTION: The state information on a cable is notified to a host      */
/*              using an Interrupt-In endpoint.                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : usStatBmap          UART state bitmap value                 */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_DESC_ERROR   Discriptor setting error        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_Notification_SerialState2( UINT16 usStatBmap )
{
    INT                                 iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x06, 0x00, 0x00 );

    /* It investigates whether SerialState is supported from the Abstract descriptor */
    if( ( l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_LINE_SERIAL ) == 0 )
        return(GRUSB_COMD_HID_DESC_ERROR);

    l_tSerialStateHid2.tNotice.bmRequestType = GRUSB_DEV_DATA_TRNS_DIR_DH       /* V1.32 */
                                          | GRUSB_DEV_CLASS_TYPE
                                          | GRUSB_INTERFACE_STATUS;
    l_tSerialStateHid2.tNotice.bNotification = GRCOMD_HID_SERIAL_STATE;             /* V1.32 */
    l_tSerialStateHid2.tNotice.wValue        = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_SERIAL_STATE_VALUE);
    l_tSerialStateHid2.tNotice.wIndex        = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_NOTIFICATION_IF);
    l_tSerialStateHid2.tNotice.wLength       = GRCOMD_HID_MAC_SWAPUS(GRCOMD_HID_SERIAL_STATE_LENGTH);
    l_tSerialStateHid2.usState               = GRCOMD_HID_MAC_SWAPUS(usStatBmap);               /* 1.30 */ /* 1.31 */ /* V1.32 */

    /* A connection state is notified to a host */
    iStat = GRUSB_DEV_ApTransferSend2(GRCOMD_HID_EP1,                            /* V1.22 */
                                     (UINT8*)&l_tSerialStateHid2,                  /* V1.32 */
                                     (UINT32)GRCOMD_HID_NOTIFICATION_SERIAL_STATE_SIZE,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     _GRUSB_COMD_HID_CbSrlStat2);
    if (iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x06, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x06, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbSrlStat2                                  */
/*                                                                          */
/* DESCRIPTION: The callback function of                                    */
/*              GRUSB_COMD_HID_Notification_SerialState2.                   */
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
LOCAL VOID _GRUSB_COMD_HID_CbSrlStat2( INT       iEpNo,
                                       UINT8*    pucBuf,
                                       UINT32    ulSize,
                                       VOID*     pAplInfo,
                                       INT       iStat )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x07, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnSerialState)
        (*l_tInitInfo2.pfnSerialState)();

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x07, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_ReciveData2                                  */
/*                                                                          */
/* DESCRIPTION: The request to receive of data.                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data buffer                     */
/*              pucData             pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_DESC_ERROR   Discriptor setting error        */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_ReciveData2( UINT32   ulSize,
                                UINT8*   pucData,
                                VOID*    pAplInfo )
{
#if 0  /* Unsupported */
    INT     iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x08, 0x00, 0x00 );

    /* It investigates whether Data Class Interface is used from a Call Management descriptor */
    if ((l_ptCallManageDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_DATA_CLASS) == 0)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x08, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_DESC_ERROR);
    }

    /* Bulk reception is started */
    iStat = GRUSB_DEV_ApTransferRecv2(GRCOMD_HID_BULKOUT_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     pAplInfo,
                                     _GRUSB_COMD_HID_CbRcvData2);
    if (iStat != GRUSB_COMD_HID_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x08, 0x02, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x08, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
#else  /* Unsupported */
    return(GRUSB_COMD_HID_DESC_ERROR);
#endif /* Unsupported */
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbRcvData2                                  */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_HID_ReciveData2.        */
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
INT _GRUSB_COMD_HID_CbRcvData2( INT        iEpNo,
                                UINT8*     pucBuf,
                                UINT32     ulSize,
                                VOID*      pAplInfo,
                                INT        iStat )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x09, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnReciveData)
    {
#ifdef GRCOMD_HID_COMP_STATUS_USE
        (*l_tInitInfo2.pfnReciveData)(ulSize, pucBuf, pAplInfo, iStat);
#else
        (*l_tInitInfo2.pfnReciveData)(ulSize, pucBuf, pAplInfo);
#endif
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x09, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_SendData2                                    */
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
INT GRUSB_COMD_HID_SendData2( UINT32     ulSize,
                              UINT8*     pucData,
                              VOID*      pAplInfo )
{
#if 0  /* Unsupported */
    INT iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0A, 0x00, 0x00 );

    /* It investigates whether Data Class Interface is used from a Call Management descriptor */
    if ((l_ptCallManageDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_DATA_CLASS) == 0)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0A, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_DESC_ERROR);
    }

    /* Bulk transmission */
    iStat = GRUSB_DEV_ApTransferSend2(GRCOMD_HID_BULKIN_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     GRUSB_TRUE,
                                     pAplInfo,
                                     _GRUSB_COMD_HID_CbSndData2);
    if (iStat != GRUSB_COMD_HID_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0A, 0x02, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0A, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
#else  /* Unsupported */
    return(GRUSB_COMD_HID_DESC_ERROR);
#endif /* Unsupported */
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbSndData2                                  */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_SendData2.              */
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
LOCAL VOID _GRUSB_COMD_HID_CbSndData2( INT       iEpNo,
                                       UINT8*    pucBuf,
                                       UINT32    ulSize,
                                       VOID*     pAplInfo,
                                       INT       iStat )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0B, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnSendData)
    {
#ifdef GRCOMD_HID_COMP_STATUS_USE
        (*l_tInitInfo2.pfnSendData)(ulSize, pucBuf, pAplInfo, iStat);
#else
        (*l_tInitInfo2.pfnSendData)(ulSize, pucBuf, pAplInfo);
#endif
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0B, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Set_GetCommFeature2                          */
/*                                                                          */
/* DESCRIPTION: The data transmitted to a host is set up at the time of     */
/*              GET_COMM_FEATURE request reception.                         */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucData             pointer of data buffer                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_SIZE_ERROR   Size error                      */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_Set_GetCommFeature2( UINT32   ulSize,
                                        UINT8*   pucData )
{
    INT iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0C, 0x00, 0x00 );

    /* Is the size of send data right? */
    if (ulSize != GRCOMD_HID_FEATURE_STATUS_SIZE)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0C, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_SIZE_ERROR);
    }

    /* Send data is setup */
    iStat = GRUSB_DEV_ApControlSend2(GRCOMD_HID_EP0,
                                     pucData,
                                     ulSize,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     GRUSB_NULL);

    if(iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0C, 0x02, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0C, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Set_GetEncapsulatedResponse2                 */
/*                                                                          */
/* DESCRIPTION: The data transmitted to a host is set up at the time of     */
/*              GET_ENCAPSULATED_RESPONSE request reception.                */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucData             pointer of data buffer                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_SIZE_ERROR   Size error                      */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_Set_GetEncapsulatedResponse2( UINT32  ulSize,
                                                 UINT8*  pucData )
{
    INT iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0D, 0x00, 0x00 );

    /* Is the size of send data right? */
    if (ulSize != l_usRespSz2)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0D, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_SIZE_ERROR);
    }

    /* Send data is setup */
    iStat = GRUSB_DEV_ApControlSend2(GRCOMD_HID_EP0,
                                     pucData,
                                     ulSize,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     GRUSB_NULL);

    if(iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0D, 0x02, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0D, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_Set_GetLineCoding2                           */
/*                                                                          */
/* DESCRIPTION: The data transmitted to a host is set up at the time of     */
/*              GET_LINE_CODING request reception.                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data                            */
/*              pucData             pointer of data buffer                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_HID_SUCCESS      Successfully end                */
/*              GRUSB_COMD_HID_ERROR        Unusual end                     */
/*              GRUSB_COMD_HID_SIZE_ERROR   Size error                      */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_HID_Set_GetLineCoding2( UINT32    ulSize,
                                       UINT8*    pucData )
{
    INT iStat;

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0E, 0x00, 0x00 );

    /* Is the size of send data right? */
    if (ulSize != GRCOMD_HID_LINE_CODING_SIZE)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0E, 0x01, END_FUNC );
        return(GRUSB_COMD_HID_SIZE_ERROR);
    }

    /* Send data is setup */
    iStat = GRUSB_DEV_ApControlSend2(GRCOMD_HID_EP0,
                                     pucData,
                                     ulSize,
                                     GRUSB_TRUE,
                                     GRUSB_NULL,
                                     GRUSB_NULL);

    if(iStat != GRUSB_DEV_SUCCESS)
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0E, 0x02, END_FUNC );
        return(GRUSB_COMD_HID_ERROR);
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0E, 0x00, END_FUNC );
    return(GRUSB_COMD_HID_SUCCESS);
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbDevRequest2                               */
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
LOCAL VOID _GRUSB_COMD_HID_CbDevRequest2( UINT8      ucRequestType,
                                          UINT8      ucRequest,
                                          UINT16     usValue,
                                          UINT16     usIndex,
                                          UINT16     usLength )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x00 );

    /* Is a request type effective ? */
    if (ucRequestType == (GRUSB_DEV_DATA_TRNS_DIR_DH
                        | GRUSB_DEV_CLASS_TYPE
                        | GRUSB_INTERFACE_STATUS))
    {
        /* Is the request code defined ? */
        switch(ucRequest)
        {
        case GRCOMD_HID_GET_ENCAPSULATED_RESPONSE:
            /* Since it is indispensable, an Abstract descriptor is not investigated        */
            /* Are the value of a request being effective and the callback function setup?  */
            if ((usValue == 0)
             && (usIndex == GRCOMD_HID_MANAGEMENT_IF))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x01 );

                l_usRespSz2 = usLength;                                      /* Response size is saved       */
                if (l_tInitInfo2.pfnGetEncapsulatedRes)
                    (*l_tInitInfo2.pfnGetEncapsulatedRes)(usLength);         /* Callback function is called  */
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x01 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_HID_GET_COMM_FEATURE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_FEATURE) != 0)
             && (usValue  == GRCOMD_HID_ABSTRACT_STATE)
             && (usIndex  == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength == GRCOMD_HID_FEATURE_STATUS_SIZE))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x02 );

                if (l_tInitInfo2.pfnGetCommFeature)
                    (*l_tInitInfo2.pfnGetCommFeature)(usLength);             /* Callback function is called  */
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x02 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                           /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_HID_GET_LINE_CODING:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_LINE_SERIAL) != 0)
             && (usValue  == 0)
             && (usIndex  == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength == GRCOMD_HID_LINE_CODING_SIZE))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x03 );

                if (l_tInitInfo2.pfnGetLineCoding)
                    (*l_tInitInfo2.pfnGetLineCoding)(usLength);              /* Callback function is called  */
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x03 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                       /* Endpoint0 is set to STALL    */
            }
            break;
        case 0x22:
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x03 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                       /* Endpoint0 is set to STALL    */

            break;

        default:
            GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x04 );

            GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                           /* Endpoint0 is set to STALL    */
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
#if 1  /* HID */
        case GRUSB_HID_DFU_DETACH:
            if (l_tInitInfo2.pfnReciveData)
            {
        #if 1 //2022-08-05
                if (l_tInitInfo2.pfnHidDetach)
                    (*l_tInitInfo2.pfnHidDetach)(usValue);                   /* Callback function is called  */
        #else
                l_aucCommDt2[0] = ucRequest;
                l_aucCommDt2[1] = ((usValue >> 8) & 0xFF);
                l_aucCommDt2[2] = (usValue & 0xFF);
                l_aucCommDt2[3] = ((usIndex >> 8) & 0xFF);
                l_aucCommDt2[4] = (usIndex & 0xFF);
                l_aucCommDt2[5] = ((usLength >> 8) & 0xFF);
                l_aucCommDt2[6] = (usLength & 0xFF);

            #ifdef GRCOMD_HID_COMP_STATUS_USE
                (*l_tInitInfo2.pfnReciveData)(7, l_aucCommDt2, GRUSB_NULL, GRUSB_DEV_SUCCESS);
            #else
                (*l_tInitInfo2.pfnReciveData)(7, l_aucCommDt2, GRUSB_NULL);
            #endif
        #endif
            }
            break;
        case GRUSB_HID_REQ_ABORT:
            /* TODO : DEBUG �ʒm�K�v�H */
            break;
        case GRUSB_HID_SET_IDLE:
            /* TODO : DEBUG �ʒm�K�v�H */
            break;
        case GRUSB_HID_SET_REPORT:
                if (GRUSB_DEV_SUCCESS != (GRUSB_DEV_ApControlRecv2(
                                                        GRCOMD_HID_EP0,
                                                        l_aucCommDt2,
                                                        (UINT32)usLength,
                                                        GRUSB_NULL,
                                                        _GRUSB_COMD_HID_CbRcvData2)))
                {
                    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x09 );

                    /* When a response is an error */
                    GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                 /* Endpoint0 is set to STALL    */
                }

            break;
        case GRCOMD_HID_CLEAR_COMM_FEATURE:
            break;
#else  /* HID */
        case GRCOMD_HID_CLEAR_COMM_FEATURE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_FEATURE) != 0)
             && (usValue  == GRCOMD_HID_ABSTRACT_STATE)
             && (usIndex  == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength == 0))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x05 );

                if (l_tInitInfo2.pfnClearCommFeature)
                    (*l_tInitInfo2.pfnClearCommFeature)();                   /* Callback function is called  */
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x05 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                       /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_HID_SET_CONTROL_LINE_STATE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_LINE_SERIAL) != 0)
             && (usIndex  == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength == 0))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x06 );

                if (l_tInitInfo2.pfnSetControlLineState)
                    (*l_tInitInfo2.pfnSetControlLineState)(usValue);         /* Callback function is called  */
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x06 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                       /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_HID_SEND_BREAK:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_BREAK) != 0)
             && (usIndex  == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength == 0))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x07 );

                if (l_tInitInfo2.pfnSendBreak)
                    (*l_tInitInfo2.pfnSendBreak)(usValue);                   /* Callback function is called  */
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x07 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                       /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_HID_SEND_ENCAPSULATED_COMMAND:
            /* Since it is indispensable, an Abstract descriptor is not investigated */
            /* Are the value of a request being effective and the callback function setup? */
            if ((usValue == 0)
             && (usIndex == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength > 0))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x08 );

                if (GRUSB_DEV_SUCCESS != (GRUSB_DEV_ApControlRecv2(
                                                        GRCOMD_HID_EP0,
                                                        l_aucCommDt2,
                                                        (UINT32)usLength,
                                                        GRUSB_NULL,
                                                        _GRUSB_COMD_HID_CbSndEncpslCmnd2)))
                {
                    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x08 );

                    /* When a response is an error */
                    GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                  /* Endpoint0 is set to STALL    */
                }
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x02, 0x08 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                      /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_HID_SET_COMM_FEATURE:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_FEATURE) != 0)
             && (usValue == GRCOMD_HID_ABSTRACT_STATE)
             && (usIndex == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength == GRCOMD_HID_FEATURE_STATUS_SIZE))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x09 );

                if (GRUSB_DEV_SUCCESS != (GRUSB_DEV_ApControlRecv2(
                                                        GRCOMD_HID_EP0,
                                                        l_aucCommDt2,
                                                        (UINT32)usLength,
                                                        GRUSB_NULL,
                                                        _GRUSB_COMD_HID_CbSetCommFeature2)))
                {
                    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x09 );

                    /* When a response is an error */
                    GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                 /* Endpoint0 is set to STALL    */
                }
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x02, 0x09 );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                     /* Endpoint0 is set to STALL    */
            }
            break;

        case GRCOMD_HID_SET_LINE_CODING:
            /* Is the Abstract descriptor supported and are the value of    */
            /*  a request being effective and the callback function set up? */
            if (((l_ptAbstructDescHid2->bmCapabilities & GRCOMD_HID_BM_CAPA_LINE_SERIAL) != 0)
             && (usValue == 0)
             && (usIndex == GRCOMD_HID_MANAGEMENT_IF)
             && (usLength == GRCOMD_HID_LINE_CODING_SIZE))
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x0A );

                if (GRUSB_DEV_SUCCESS != (GRUSB_DEV_ApControlRecv2(
                                                        GRCOMD_HID_EP0,
                                                        l_aucCommDt2,
                                                        (UINT32)usLength,
                                                        GRUSB_NULL,
                                                        _GRUSB_COMD_HID_CbSetLineCoding2)))
                {
                    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x01, 0x0A );

                    /* When a response is an error */
                    GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                 /* Endpoint0 is set to STALL    */
                }
            }
            else
            {
                GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x02, 0x0A );

                GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                    /* Endpoint0 is set to STALL    */
            }
            break;
#endif /* HID */

        default:
            GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x0B );

            GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                         /* Endpoint0 is set to STALL    */
            break;
        }
    }
    else
    {
        GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, 0x0C );

        GRUSB_DEV_ApSetStall2(GRCOMD_HID_EP0);                             /* Endpoint0 is set to STALL    */
    }

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x0F, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbSndEncpslCmnd2                            */
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
LOCAL INT _GRUSB_COMD_HID_CbSndEncpslCmnd2( INT          iEpNo,
                                            UINT8*       pucBuf,
                                            UINT32       ulSize,
                                            VOID*        pAplInfo,
                                            INT          iStat )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x10, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnSendEncapsulatedCmd)
        (*l_tInitInfo2.pfnSendEncapsulatedCmd)(ulSize, pucBuf);

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x10, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbSetCommFeature2                           */
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
LOCAL INT _GRUSB_COMD_HID_CbSetCommFeature2( INT         iEpNo,
                                             UINT8*      pucBuf,
                                             UINT32      ulSize,
                                             VOID*       pAplInfo,
                                             INT         iStat )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x11, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnSetCommFeature)
        (*l_tInitInfo2.pfnSetCommFeature)(ulSize, pucBuf);

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x11, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbSetLineCoding2                            */
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
LOCAL INT _GRUSB_COMD_HID_CbSetLineCoding2( INT          iEpNo,
                                            UINT8*       pucBuf,
                                            UINT32       ulSize,
                                            VOID*        pAplInfo,
                                            INT          iStat)
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x12, 0x00, 0x00 );

    /* Callback function is called */
    if (l_tInitInfo2.pfnSetLineCoding)
        (*l_tInitInfo2.pfnSetLineCoding)(ulSize, pucBuf);

    GRCOMD_HID_LOG2( GRDBG_COMD, 0x12, 0x00, END_FUNC );
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbConn2                                     */
/*                                                                          */
/* DESCRIPTION: Notice processing of Connection.                            */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _GRUSB_COMD_HID_CbConn2( VOID )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x13, 0x00, 0x00 );

    /* check local usb state */
    if (l_iUsbStat2 != GRUSB_COMD_HID_CON)                   /* if state is the disconnect       */
    {
        l_iUsbStat2 = GRUSB_COMD_HID_CON;                    /* state is set to the connect      */

        GRCOMD_HID_LOG2( GRDBG_COMD, 0x13, 0x00, 0x01 );

        if (l_tInitInfo2.pfnConnStat)
            l_tInitInfo2.pfnConnStat(GRUSB_COMD_HID_CON);    /* Callback function is called      */
    }
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x13, 0x00, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_HID_CbDisConn2                                      */
/*                                                                          */
/* DESCRIPTION: Notice processing of Disconnection.                         */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_HID_CbDisConn2( VOID )
{
    GRCOMD_HID_LOG2( GRDBG_COMD, 0x14, 0x00, 0x00 );

    /* check local usb state */
    if (l_iUsbStat2 != GRUSB_COMD_HID_DISC)                  /* if state is the connect          */
    {
        l_iUsbStat2 = GRUSB_COMD_HID_DISC;                   /* state is set to the disconnect   */

        GRCOMD_HID_LOG2( GRDBG_COMD, 0x14, 0x00, 0x01 );

        /* All transfers are aborted */
        GRUSB_DEV_ApAbort2( GRCOMD_HID_EP1 );
#if 0  /* HID */
        GRUSB_DEV_ApAbort2( GRCOMD_EP2 );
        GRUSB_DEV_ApAbort2( GRCOMD_EP3 );
#endif /* HID */

        if (l_tInitInfo2.pfnConnStat)
            l_tInitInfo2.pfnConnStat(GRUSB_COMD_HID_DISC);   /* Callback function is called      */
    }
    else    // NEW_BUS_RESET 2022-07-19 関数名はDisconnectだが、設定はBus Reset,USB断、Set configuraitonのvalue 0で発生する
    {

        if (l_tInitInfo2.pfnConnStat)
            l_tInitInfo2.pfnConnStat(2);   /* Callback function is called Bus Reset      */

    }


    GRCOMD_HID_LOG2( GRDBG_COMD, 0x14, 0x00, END_FUNC );
}

