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
/*      perid.h                                                 1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file composes the GR-USB/DEVICE Driver User Interface.         */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  A.Yoshida   2002/12/13  Created initial version 0.01                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
/*  Y.Sato      2003/05/15  It changes Remote Wakeup function               */
/*  Y.Sato      2003/05/30  It changes below functions                      */
/*  K.Takagi    2003/07/16  version 0.92, modified variable names.          */
/*  Y.Sato      2003/07/24  version 0.93, add/rename API function           */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V1.01                                           */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V1.02                                           */
/*                              added new API functions.                    */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              version is updated.                         */
/*  K.Handa     2005/03/18  V 1.11                                          */
/*                              below descriptors supported.                */
/*                              - Device Qualifier Descriptor               */
/*                              - Other Speed Configuration Descriptor      */
/*  K.Handa     2005/03/18  V 1.12                                          */
/*                              hybrid version (full/high-speed)            */
/*  K.Handa     2006/12/27  V 1.20                                          */
/*                              version was updated.                        */
/*  S.Tomizawa  2007/01/24  V 1.30                                          */
/*                              added the definition for MTP(OS descriptor) */
/*                              added function for getting MaxPacketSize    */
/*                              added function for Media Transfer Protocol  */
/*  K.Handa     2007/03/12  V 1.40                                          */
/*                              added selector for fixed full-speed.        */
/*  K.Handa     2007/06/12  V 1.41                                          */
/*                              version was updated.                        */
/*  K.Handa     2008/01/21  V 1.42                                          */
/*                              version was updated.                        */
/*                                                                          */
/****************************************************************************/
#ifndef __PERID_H__
#define __PERID_H__

/**** INCLUDE FILES *********************************************************/
#include    "grusbtyp.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* 2007/01/24 [1.30] Added for MTP */
/* Vendor code to fetch other OS Feature Descriptors */
//#define GRUSB_MTP_MS_VENDORCODE             0xFD

/* 1.40 : added selector for fixed full-speed. */
/* if only full-speed, please validate below define sentence. */
//#define GRUSB_DEV_FIXED_FS

/* error code */
#define GRUSB_DEV_SUCCESS                   0       /* Successfully end                     */
#define GRUSB_DEV_BUSY_ENDPOINT             (-1)    /* Endpoint is busy                     */
#define GRUSB_DEV_CALLBACK_OVER             (-2)    /* Callback table is full               */
#define GRUSB_DEV_INIT_FAILED               (-3)    /* Initialize failed                    */
#define GRUSB_DEV_INVALID_ENDPOINT          (-4)    /* Invalid endpoint no.                 */
#define GRUSB_DEV_INVALID_PARAM             (-5)    /* Invalid Parameter                    */
#define GRUSB_DEV_INVALID_PROTOCOL          (-6)    /* Invalid protocol                     */
#define GRUSB_DEV_INVALID_SIZE              (-7)    /* Invalid size                         */
#define GRUSB_DEV_UNDEFINED_ENDPOINT        (-8)    /* Undefined endpoint no.               */
#define GRUSB_DEV_MAKE_CALLBACK_FAILED      (-9)    /* Make callback error                  */
#define GRUSB_DEV_MAKE_DESCRIPTOR_FAILED    (-10)   /* Make descriptor error                */
#define GRUSB_DEV_NOT_CONFIGURED            (-11)   /* Endpoint isn't CONFIGURED            */
#define GRUSB_DEV_OPEN_FAILED               (-12)   /* GR-USB(DEVICE) can't open            */
#define GRUSB_DEV_STALLED                   (-13)   /* Endpoint is stalled                  */
#define GRUSB_DEV_STALLING                  (-14)   /* Now stalling                         */
#define GRUSB_DEV_TIMEOUT                   (-15)   /* Timeout                              */
#define GRUSB_DEV_NO_DATA                   (-16)   /* non-existence of the reception data  */
#define GRUSB_DEV_DMA_BUSY                  (-17)   /* DMA CHANNEL is already assigned      */
#define GRUSB_DEV_WAKEUP_NOT_EXECUTED       (-18)   /* Remote Wakeup was NOT Executed       */
#define GRUSB_DEV_CANCELED                  (-19)   /* Canceled (queuing)                   */
#define GRUSB_DEV_TRAN_CANCELED             (-20)   /* Canceled (now transmitting)          */

/* The definition of the constant which opts for the composition of Driver */
#define GRUSB_DEV_MAX_EP                    7       /* Number of End Points                 */
#define GRUSB_DEV_MAX_ADDR                  127     /* The maximum address number           */
#define GRUSB_EP0_MAX_SIZE                  8       /* The maximum packet size of endpoint_0*/

/* The definition of endpoint */
#define GRUSB_DEV_EP0                       0x00    /* endpoint_0                       */

/* The definition value about Standard Device Request */
#define GRUSB_DEV_GET_STATUS                0x00    /* GET_STATUS                       */
#define GRUSB_DEV_CLEAR_FEATURE             0x01    /* CLEAR_FEATURE                    */
#define GRUSB_DEV_SET_FEATURE               0x03    /* SET_FEATURE                      */
#define GRUSB_DEV_SET_ADDRESS               0x05    /* SET_ADDRESS                      */
#define GRUSB_DEV_GET_DESCRIPTOR            0x06    /* GET_DESCRIPTOR                   */
#define GRUSB_DEV_SET_DESCRIPTOR            0x07    /* SET_DESCRIPTOR                   */
#define GRUSB_DEV_GET_CONFIGURATION         0x08    /* GET_CONFIGURATION                */
#define GRUSB_DEV_SET_CONFIGURATION         0x09    /* SET_CONFIGURATION                */
#define GRUSB_DEV_GET_INTERFACE             0x0A    /* GET_INTERFACE                    */
#define GRUSB_DEV_SET_INTERFACE             0x0B    /* SET_INTERFACE                    */
#define GRUSB_DEV_SYNCH_FRAME               0x0C    /* SYNCH_FRAME                      */

/* The definition value about bmRequestType */
#define GRUSB_DEV_DATA_TRNS_DIR             0x80    /* Data Transfer Direction          */
#define GRUSB_DEV_DATA_TRNS_DIR_HD          0x00    /* Host->Device                     */
#define GRUSB_DEV_DATA_TRNS_DIR_DH          0x80    /* Device->Host                     */

#define GRUSB_DEV_REQUEST_TYPE              0x60    /* Type                             */
#define GRUSB_DEV_STANDARD_TYPE             0x00    /* standard                         */
#define GRUSB_DEV_CLASS_TYPE                0x20    /* class                            */
#define GRUSB_DEV_VENDER_TYPE               0x40    /* vender                           */

#define GRUSB_REQUEST_RECIPIENT             0x1F    /* recipient                        */
#define GRUSB_DEVICE_STATUS                 0x00    /* device                           */
#define GRUSB_INTERFACE_STATUS              0x01    /* interface                        */
#define GRUSB_ENDPOINT_STATUS               0x02    /* endpoint                         */
#define GRUSB_OTHER_STATUS                  0x03    /* other                            */

/* The Bit value which shows the standard device request notified to a higher rank */
#define GRUSB_DEV_DevReq_GET_STATUS         (0x01 <<  0)    /* GET_STATUS               */
#define GRUSB_DEV_DevReq_CLEAR_FEATURE      (0x01 <<  1)    /* CLEAR_FEATURE            */
#define GRUSB_DEV_DevReq_SET_FEATURE        (0x01 <<  3)    /* SET_FEATURE              */
#define GRUSB_DEV_DevReq_SET_ADDRESS        (0x01 <<  5)    /* SET_ADDRESS              */
#define GRUSB_DEV_DevReq_GET_DESCRIPTOR     (0x01 <<  6)    /* GET_DESCRIPTOR           */
#define GRUSB_DEV_DevReq_SET_DESCRIPTOR     (0x01 <<  7)    /* SET_DESCRIPTOR           */
#define GRUSB_DEV_DevReq_GET_CONFIGURATION  (0x01 <<  8)    /* GET_CONFIGURATION        */
#define GRUSB_DEV_DevReq_SET_CONFIGURATION  (0x01 <<  9)    /* SET_CONFIGURATION        */
#define GRUSB_DEV_DevReq_GET_INTERFACE      (0x01 << 10)    /* GET_INTERFACE            */
#define GRUSB_DEV_DevReq_SET_INTERFACE      (0x01 << 11)    /* SET_INTERFACE            */
#define GRUSB_DEV_DevReq_SYNCH_FRAME        (0x01 << 12)    /* SYNCH_FRAME              */

/* Notice classification of a transmission error */
#define GRUSB_DEV_TranErr_IN                (UINT16)1
#define GRUSB_DEV_TranErr_OUT               (UINT16)2
#define GRUSB_DEV_TranErr_UNDERRUN          (UINT16)3

/* Notice classification which can be remote Wake raised */
#define GRUSB_DEV_RmtWkup_ENABLE            (UINT16)1
#define GRUSB_DEV_RmtWkup_DISABLE           (UINT16)0


/* Structure of Device Descriptor */
typedef struct grusb_dev_device_desc_tag
{
    UINT8       uc_bLength;                     /* bLength              */
    UINT8       uc_bDescriptorType;             /* bDescriptorType      */
    UINT8       uc_bcdUSB[2];                   /* bcdUSB               */
    UINT8       uc_bDeviceClass;                /* bDeviceClass         */
    UINT8       uc_bDeviceSubClass;             /* bDeviceSubClass      */
    UINT8       uc_bDeviceProtocol;             /* bDeviceProtocol      */
    UINT8       uc_bMaxPacketSize0;             /* bMaxPacketSize0      */
    UINT8       auc_idVendor[2];                /* idVendor             */
    UINT8       auc_idProduct[2];               /* idProduct            */
    UINT8       auc_bcdDevice[2];               /* bcdDevice            */
    UINT8       uc_iManufacturer;               /* iManufacturer        */
    UINT8       uc_iProduct;                    /* iProduct             */
    UINT8       uc_iSerialNumber;               /* iSerialNumber        */
    UINT8       uc_bNumConfigurations;          /* bNumConfigurations   */
} GRUSB_DEV_DEVICE_DESC;

/* Structure of Configuration Descriptor */
typedef struct grusb_dev_config_desc_tag
{
    UINT8       uc_bLength;                     /* bLength                      */
    UINT8       uc_bDescriptorType;             /* bDescriptorType              */
    UINT8       auc_wTotalLength[2];            /* wTotalLength [0]Low [1] High */
    UINT8       uc_bNumInterfaces;              /* bNumInterfaces               */
    UINT8       uc_bConfigurationValue;         /* bConfigurationValue          */
    UINT8       uc_iConfiguration;              /* iConfiguration               */
    UINT8       uc_bmAttributes;                /* bmAttributes                 */
    UINT8       uc_bMaxPower;                   /* bMaxPower                    */
} GRUSB_DEV_CONFIG_DESC;

/* Structure of Interface Descriptor */
typedef struct grusb_dev_interface_desc_tag
{
    UINT8       uc_bLength;                     /* bLength              */
    UINT8       uc_bDescriptorType;             /* bDescriptorType      */
    UINT8       uc_bInterfaceNumber;            /* bInterfaceNumber     */
    UINT8       uc_bAlternateSetting;           /* bAlternateSetting    */
    UINT8       uc_bNumEndpoints;               /* bNumEndpoints        */
    UINT8       uc_bInterfaceClass;             /* bInterfaceClass      */
    UINT8       uc_bInterfaceSubClass;          /* bInterfaceSubClass   */
    UINT8       uc_bInterfaceProtocol;          /* bInterfaceProtocol   */
    UINT8       uc_iInterface;                  /* iInterface           */
} GRUSB_DEV_INTERFACE_DESC;

/* Structure of endpoint Descriptor */
typedef struct grusb_dev_endpoint_desc_tag
{
    UINT8       uc_bLength;                     /* bLength                          */
    UINT8       uc_bDescriptorType;             /* bDescriptorType                  */
    UINT8       uc_bEndPointAddress;            /* bEndPointAddress                 */
    UINT8       uc_bmAttributes;                /* bmAttributes                     */
    UINT8       auc_wMaxPacketSize[2];          /* wMaxPacketSize [0]:Low [1]High   */
    UINT8       uc_bInterval;                   /* bInterval                        */
} GRUSB_DEV_ENDPOINT_DESC;

/* Structure of Stringt Descriptor */
/*typedef VOID* GRUSB_DEV_STRING_DESC;*/
typedef UINT8*  GRUSB_DEV_STRING_DESC;

/* Structure of Device Qualifier Descriptor */
typedef struct grusb_dev_device_qualifier_desc_tag
{
    UINT8       uc_bLength;                     /* bLength              */
    UINT8       uc_bDescriptorType;             /* bDescriptorType      */
    UINT8       uc_bcdUSB[2];                   /* bcdUSB               */
    UINT8       uc_bDeviceClass;                /* bDeviceClass         */
    UINT8       uc_bDeviceSubClass;             /* bDeviceSubClass      */
    UINT8       uc_bDeviceProtocol;             /* bDeviceProtocol      */
    UINT8       uc_bMaxPacketSize0;             /* bMaxPacketSize0      */
    UINT8       uc_bNumConfigurations;          /* bNumConfigurations   */
    UINT8       uc_Reserved;                    /* Reserved             */
} GRUSB_DEV_DEVICE_QUALIFIER_DESC;

/* Structure of On-The-Go Descriptor */
typedef struct grusb_dev_otg_desc_tag
{
    UINT8       uc_bLength;                     /* bLength                          */
    UINT8       uc_bDescriptorType;             /* bDescriptorType                  */
    UINT8       uc_bmAttributes;                /* bmAttributes                     */
} GRUSB_DEV_OTG_DESC;

/* The definition value and MACRO for specifying the classification and */
/* the direction of End Point                                           */
typedef UINT8   GRUSB_DEV_EP_TYPE;                      /* End Point Type                   */
#define EPTYPE_IN               (0x80)                  /* IN End Point                     */
#define EPTYPE_OUT              (0x00)                  /* OUT End Point                    */
#define EPTYPE_CONTROL          (0x00)                  /* CONTROL End Point                */
#define EPTYPE_ISOCRONOUS       (0x10)                  /* ISOCRONOUS End Point             */
#define EPTYPE_BULK             (0x20)                  /* BULK End Point                   */
#define EPTYPE_INTERRUPT        (0x30)                  /* INTERRUPT End Point              */

/* check IN End Point           */
#define GRUSB_DEV_MAC_IsEpTypeIN(type)          (((type) & 0x80) != 0)
/* check OUT End Point          */
#define GRUSB_DEV_MAC_IsEpTypeOUT(type)         (((type) & 0x80) == 0)
/* check CONTROL End Point      */
#define GRUSB_DEV_MAC_IsEpTypeCTRL(type)        (((type) & 0x30) == EPTYPE_CONTROL)
/* check ISOCRONOUS End Point   */
#define GRUSB_DEV_MAC_IsEpTypeISO(type)         (((type) & 0x30) == EPTYPE_ISOCRONOUS)
/* check BULK End Point         */
#define GRUSB_DEV_MAC_IsEpTypeBLK(type)         (((type) & 0x30) == EPTYPE_BULK)
/* check INTERRUPT End Point    */
#define GRUSB_DEV_MAC_IsEpTypeINTR(type)        (((type) & 0x30) == EPTYPE_INTERRUPT)


/* The model of a variable in which the present state of peripheral driver is shown */
typedef enum
{
    GRUSB_DEV_STATE_IDLE,                   /* Idle(Attached/Powerd)        */
    GRUSB_DEV_STATE_DEFAULT,                /* Default                      */
    GRUSB_DEV_STATE_ADDRESS,                /* Address                      */
    GRUSB_DEV_STATE_CONFIGURED,             /* Configured                   */
    GRUSB_DEV_STATE_SUSPEND                 /* Suspend                      */
} GRUSB_DEV_STATE;


/* The model of the function called when it finishes storing the specified data in FIFO */
typedef VOID    (*GRUSB_DataPushEnd)( INT       iEpNo,
                                      UINT8*    pucBuf,
                                      UINT32    ulSize,
                                      VOID*     pAplInfo,
                                      INT       iStat);

/* The model of the function called when receiving data is stored in specified buffer */
typedef BOOLEAN (*GRUSB_DataReceiveEnd)( INT    iEpNo,
                                         UINT8* pucBuf,
                                         UINT32 ulSize,
                                         VOID*  pAplInfo,
                                         INT    iStat);

/* The function which carries out the notice stripes of the reception Cancel */
/* at the time of reception by Control transmission                          */
typedef VOID    (*GRUSB_CtrTransferCancel)(VOID);

/* The function which notifies Transmission Cancel      */
/* at the time of transmission by Control transmission  */
typedef VOID    (*GRUSB_CtrReceiveCancel)(VOID);

/* The function called when Device Request specified that   */
/* it notifies to a higher rank Layer is received           */
typedef VOID    (*GRUSB_DeviceRequest)( UINT8   uc_bmRequestType,
                                        UINT8   uc_bRequest,
                                        UINT16  us_wValue,
                                        UINT16  us_wIndex,
                                        UINT16  us_wLength);

/* The form which has 16-bit information in an argument among Call Back          */
/* for every End Point notified to a higher rank layer from USB Driver is shown. */
typedef VOID    (*GRUSB_NoticeEndPoint16)( INT      iEpNo,
                                           UINT16   usParm);

/* A general form of Call Back for every End Point notified to a higher rank layer  */
/* from USB Driver is shown.                                                        */
typedef VOID    (*GRUSB_NoticeEndPoint)( INT    iEpNo);

/* The form which has 16-bit information in an argument among Call Back notified    */
/* to a higher rank layer from USB Driver is shown.                                 */
typedef VOID    (*GRUSB_Notice16)( UINT16   usParm);

/* A general form of Call Back notified to a higher rank layer from USB Driver is shown. */
typedef VOID    (*GRUSB_Notice)(VOID);

/* The model which stores the information on End Point at the time of initial setting is shown.*/
typedef struct grusb_dev_epinitinfo_tag
{
    UINT16                  usNumOfQue;     /* Number of Sned/Recv Queues                   */
    UINT8                   ucPages;        /* FIFO Page number                             */
    UINT16                  usMaxPktSz[2];  /* Max Packet Size                  [0]FS [1]HS */
    GRUSB_DEV_EP_TYPE       ucEpType;       /* types                                        */
    GRUSB_NoticeEndPoint    pfnTransEnd;    /* The function called when the specified       */
                                            /* transmission Data carries out the completion */
                                            /* of transmitting altogether                   */
    GRUSB_NoticeEndPoint16  pfnTransErr;    /* The function called when a transmission error*/
                                            /* is notified                                  */
} GRUSB_DEV_EPINITINFO;

/* It is the model of a variable in which parameter specified at the time of */
/* initial setting is shown.                                                 */
typedef struct grusb_dev_initinfo_tag
{
    GRUSB_DEV_DEVICE_DESC*      ptDeviceDesc[2];    /* Device Descriptor        [0]FS [1]HS */
    UINT16                      usConfigDescNum;    /* Number of Configuration Descriptor   */
    GRUSB_DEV_CONFIG_DESC*      ptConfigDesc[2];    /* Configuration Descriptor [0]FS [1]HS */
    UINT8*                      pucCntrlArea;       /* Area for Driver control              */
    INT                         iEpNum;             /* endpoint number                      */
    UINT8                       ucStringDescNum;    /* Number of String Descriptors         */
    GRUSB_DEV_STRING_DESC*      pStringDesc;        /* String Descriptors                   */
    GRUSB_DEV_EPINITINFO        atEpInfo[GRUSB_DEV_MAX_EP]; /* EP setup specified at the time*/
                                                    /* of initial setting                   */
    /* for Device Request Callback function */
    UINT16                      usReqCodeMap;       /* request code map                     */
    GRUSB_DeviceRequest         pfnDevRquest;       /* The function which notifies Device   */
                                                    /* Request reception                    */
    GRUSB_NoticeEndPoint        pfnSndCancel;       /* The function which notifies Control  */
                                                    /* transmitting discontinuation         */
    GRUSB_NoticeEndPoint        pfnRcvCancel;       /* The function which notifies Control  */
                                                    /* reception discontinuation number     */

    /* for Callback function */
    GRUSB_Notice                pfnBusReset;        /* The function which notifies Bus Reset*/
                                                    /* detection                            */
    GRUSB_Notice                pfnResume;          /* The function which notifies Resume   */
                                                    /* detection                            */
    GRUSB_Notice                pfnSuspend;         /* The function which notifies Suspend  */
                                                    /* detection                            */
    GRUSB_NoticeEndPoint16      pfnClearFeature;    /* The function called when Host to     */
                                                    /* CLEAR_FEATURE is notified            */
    GRUSB_Notice16              pfnEnbRmtWkup;      /* Remote Wakeup functional effective   */
                                                    /* notice function                      */
    GRUSB_Notice                pfnCmpSetInterface; /* notice completely of SET_INTERFACE   */
    GRUSB_Notice                pfnCmpSetConfiguration;
                                                    /* notice completely SET_CONFIGURATION  */
    GRUSB_Notice                pfnCngStateNoConfigured;
                                                    /* notice to change the state from      */
                                                    /* CONFIGURED                           */
} GRUSB_DEV_INITINFO;


INT             GRUSB_DEV_ControlAreaSize( INT, UINT16*);
INT             GRUSB_DEV_ApInit( GRUSB_DEV_INITINFO*);
GRUSB_DEV_STATE GRUSB_DEV_ApGetDeviceState(VOID);
INT             GRUSB_DEV_ApControlSend( INT, UINT8*, UINT32, INT, VOID*, GRUSB_DataPushEnd);
INT             GRUSB_DEV_ApTransferSend( INT, UINT8*, UINT32, INT, VOID*, GRUSB_DataPushEnd);
INT             GRUSB_DEV_ApTransferSendQueue( INT, UINT8*, UINT32, INT, VOID*, GRUSB_DataPushEnd);
INT             GRUSB_DEV_ApTransferSendStart( INT );
INT             GRUSB_DEV_ApAbort( INT);
INT             GRUSB_DEV_ApSetStall( INT);
INT             GRUSB_DEV_ApControlRecv( INT, UINT8*, UINT32, VOID*, GRUSB_DataReceiveEnd );
INT             GRUSB_DEV_ApTransferRecv( INT, UINT8*, UINT32, VOID*, GRUSB_DataReceiveEnd );
INT             GRUSB_DEV_ApTransferRecvQueue( INT, UINT8*, UINT32, VOID*, GRUSB_DataReceiveEnd );
INT             GRUSB_DEV_ApTransferRecvStart( INT );
INT             GRUSB_DEV_ApCallbackConnect( GRUSB_Notice);
INT             GRUSB_DEV_ApCallbackDisconnect( GRUSB_Notice);
INT             GRUSB_DEV_ApRmtWkup(VOID);
VOID            GRUSB_DEV_Connect(VOID) ;
VOID            GRUSB_DEV_Disconnect(VOID) ;
INT             GRUSB_DEV_ApClearStall(INT);
INT             GRUSB_DEV_ApClearToggle(INT);
VOID            GRUSB_DEV_ApReqPullupRegister(INT);
UINT16          GRUSB_DEV_ApGetMaxPacketSize(INT);      /* 2007/01/24 [1.30] Added for getting MaxPacketSize */
#ifdef GRUSB_MTP_MS_VENDORCODE
VOID            GRUSB_DEV_ApSetMTPSupportFlg(BOOLEAN);  /* 2007/01/24 [1.30] Added for MTP                   */
#endif


INT             GRUSB_DEV_ControlAreaSize2( INT, UINT16*);
INT             GRUSB_DEV_ApInit2( GRUSB_DEV_INITINFO*);
GRUSB_DEV_STATE GRUSB_DEV_ApGetDeviceState2(VOID);
INT             GRUSB_DEV_ApControlSend2( INT, UINT8*, UINT32, INT, VOID*, GRUSB_DataPushEnd);
INT             GRUSB_DEV_ApTransferSend2( INT, UINT8*, UINT32, INT, VOID*, GRUSB_DataPushEnd);
INT             GRUSB_DEV_ApTransferSendQueue2( INT, UINT8*, UINT32, INT, VOID*, GRUSB_DataPushEnd);
INT             GRUSB_DEV_ApTransferSendStart2( INT );
INT             GRUSB_DEV_ApAbort2( INT);
INT             GRUSB_DEV_ApSetStall2( INT);
INT             GRUSB_DEV_ApControlRecv2( INT, UINT8*, UINT32, VOID*, GRUSB_DataReceiveEnd );
INT             GRUSB_DEV_ApTransferRecv2( INT, UINT8*, UINT32, VOID*, GRUSB_DataReceiveEnd );
INT             GRUSB_DEV_ApTransferRecvQueue2( INT, UINT8*, UINT32, VOID*, GRUSB_DataReceiveEnd );
INT             GRUSB_DEV_ApTransferRecvStart2( INT );
INT             GRUSB_DEV_ApCallbackConnect2( GRUSB_Notice);
INT             GRUSB_DEV_ApCallbackDisconnect2( GRUSB_Notice);
INT             GRUSB_DEV_ApRmtWkup2(VOID);
VOID            GRUSB_DEV_Connect2(VOID) ;
VOID            GRUSB_DEV_Disconnect2(VOID) ;
INT             GRUSB_DEV_ApClearStall2(INT);
INT             GRUSB_DEV_ApClearToggle2(INT);
VOID            GRUSB_DEV_ApReqPullupRegister2(INT);
UINT16          GRUSB_DEV_ApGetMaxPacketSize2(INT);      /* 2007/01/24 [1.30] Added for getting MaxPacketSize */
#ifdef GRUSB_MTP_MS_VENDORCODE
VOID            GRUSB_DEV_ApSetMTPSupportFlg2(BOOLEAN);  /* 2007/01/24 [1.30] Added for MTP                   */
#endif
#if 1  /* HID */
VOID            GRUSB_DEV_ApSetHIDSupportFlg2(BOOLEAN);  /* 2022/06/21 Added for HID                          */
#endif /* HID */

#endif  /* __PERID_H__  */

