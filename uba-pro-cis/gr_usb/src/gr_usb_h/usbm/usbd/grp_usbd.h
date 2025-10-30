/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2018 Grape Systems, Inc.                          */
/*                                     All Rights Reserved.                                     */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      grp_usbd.h                                                              1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# USB driver header file                                                     */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/06  V1.01                                                           */
/*                            - Correction by member change in structure.                       */
/*                              - grp_usbdi_request                                             */
/*                                  pfnCallbackFunc -> pfnNrCbFunc                              */
/*                              - grp_usbdi_device_request                                      */
/*                                  pfnCallbackFunc -> pfnDvCbFunc                              */
/*                              - grp_usbdi_st_device_request                                   */
/*                                  pfnCallbackFunc -> pfnStCbFunc                              */
/*   K.Takagi       2009/04/02  V1.02                                                           */
/*                            - Added following new structure (for the work for the reduction   */
/*                              in the memory area)                                             */
/*                              - grp_usbdi_req_pool                                            */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                            - Added prototype declaration of the following function.          */
/*                              - grp_usbd_GetIsochronousInfo                                   */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_USBD_H_
#define _GRP_USBD_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_hcd.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
/* Device base address(Root hub not included) */
#define GRP_USBD_DEVICE_BASE_ADDRESS    1

/* Host controller number for index of the array which the base number is 0 */
#define GRP_USBC_HOST_QTY_CONTROLLER    (GRP_USB_HOST_CONTROLLER_NUM)

/* Device number for index of the array which the base address is considered */
#define GRP_USBD_HOST_MAX_DEVICE        (GRP_USB_HOST_DEVICE_MAX + GRP_USBD_DEVICE_BASE_ADDRESS)

/* Clasee number for index of the array which the base number is 0 */
#define GRP_USBD_HOST_MAX_CLASS         (GRP_USB_HOST_CLASS_MAX)

/* Clasee number for index of the array which the base number is 0 */
#define GRP_USBD_HOST_MAX_REGISTER      (GRP_USB_HOST_REGS_MAX)

/* Pipe characteristic for selection */
#define GRP_USBD_SELECT_PIPE            4

/* Error codes(Response from functions) */

/* Error codes */
#define GRP_USBD_OK                     0                           /* Success                  */

#define GRP_USBD_RET_ERROR_BASE         GRP_RET_ERROR_BASE(GRP_USBD_MDL_ID)

#define GRP_USBD_INVALIED_ADR           GRP_USBD_RET_ERROR_BASE-1   /* Invalid address          */
#define GRP_USBD_INVALIED_IF            GRP_USBD_RET_ERROR_BASE-2   /* Invalid device interface */
#define GRP_USBD_INVALIED_EP            GRP_USBD_RET_ERROR_BASE-3   /* Invalid device endpoint  */
#define GRP_USBD_INVALIED_PH            GRP_USBD_RET_ERROR_BASE-4   /* Invalid pipe information */
#define GRP_USBD_INVALIED_PMTR          GRP_USBD_RET_ERROR_BASE-5   /* Invalid parameters       */
#define GRP_USBD_INVALIED_CLIANT        GRP_USBD_RET_ERROR_BASE-6   /* Not master client        */
#define GRP_USBD_PIPE_HALT_ERROR        GRP_USBD_RET_ERROR_BASE-7   /* Pipe HALTed              */
#define GRP_USBD_ALREADY_XFER           GRP_USBD_RET_ERROR_BASE-8   /* Transfer already         */
                                                                    /* started/completed        */
#define GRP_USBD_ALREADY_OPERATED       GRP_USBD_RET_ERROR_BASE-9   /* Operation have already   */
                                                                    /* done                     */
#define GRP_USBD_UNAVAILABLE            GRP_USBD_RET_ERROR_BASE-10  /* Master client right not  */
                                                                    /* available                */
#define GRP_USBD_NO_DESCRITOR           GRP_USBD_RET_ERROR_BASE-11  /* Descriptor not exist     */
#define GRP_USBD_NO_BANDWIDTH           GRP_USBD_RET_ERROR_BASE-12  /* Not enough commnunication*/
                                                                    /* bandwidth                */
#define GRP_USBD_TOO_MANY_DEVICE        GRP_USBD_RET_ERROR_BASE-13  /* Too many connection      */
                                                                    /* devices                  */
                                                                    /* Exceeded 127 units       */
#define GRP_USBD_PIPE_ACTIVE_FAIL       GRP_USBD_RET_ERROR_BASE-14  /* Failure to activate pipe */
#define GRP_USBD_PIPE_HALT_FAIL         GRP_USBD_RET_ERROR_BASE-15  /* Failure of pip HALT      */
#define GRP_USBD_CONFIGURATION_ERROR    GRP_USBD_RET_ERROR_BASE-16  /* Device configuration     */
                                                                    /* error                    */
#define GRP_USBD_INIT_ERROR             GRP_USBD_RET_ERROR_BASE-17  /* USB driver initialization*/
                                                                    /* error                    */
#define GRP_USBD_HOST_INIT_ERROR        GRP_USBD_RET_ERROR_BASE-18  /* Initialization failure   */
                                                                    /* for host controller      */
#define GRP_USBD_ILLEGAL_ERROR          GRP_USBD_RET_ERROR_BASE-19  /* Illegal function         */
#define GRP_USBD_BUS_SUSPEND            GRP_USBD_RET_ERROR_BASE-20  /* USB bus suspended        */
#define GRP_USBD_HOST_HALT              GRP_USBD_RET_ERROR_BASE-21  /* Host controller HALTed   */
#define GRP_USBD_CANNOT_RELEASE         GRP_USBD_RET_ERROR_BASE-22  /* Cannot release client    */
                                                                    /* right                    */
#define GRP_USBD_CANNOT_OPERATE         GRP_USBD_RET_ERROR_BASE-23  /* Can not operate          */
#define GRP_USBD_NO_FUNCTION            GRP_USBD_RET_ERROR_BASE-24  /* No function              */

#define GRP_USBD_OS_RELATED_ERROR       GRP_USBD_RET_ERROR_BASE-25  /* OS related error         */
#define GRP_USBD_NO_SUPPORT_PMTR        GRP_USBD_RET_ERROR_BASE-26  /* Paraemter is not         */
                                                                    /* supported                */

/* Connection state of lower host controller driver */
#define GRP_USBD_TR_NOT_PROCESS         0                           /* Connection request not   */
                                                                    /* processed                */
#define GRP_USBD_TR_NO_FAIL             1                           /* TR success               */
#define GRP_USBD_TR_CANCEL              2                           /* TR cancelled             */

#define GRP_USBD_TR_FAIL_BASE           GRP_TR_FAIL_BASE(GRP_USBD_MDL_ID)

#define GRP_USBD_TR_TIMEOUT             GRP_USBD_TR_FAIL_BASE-1     /* TR timeout               */
#define GRP_USBD_TR_DATA_OVERRUN        GRP_USBD_TR_FAIL_BASE-2     /* TR receive data overrun  */
#define GRP_USBD_TR_DATA_UNDERRUN       GRP_USBD_TR_FAIL_BASE-3     /* TR receive data underrun */
#define GRP_USBD_TR_CRC_ERROR           GRP_USBD_TR_FAIL_BASE-4     /* CRC error                */
#define GRP_USBD_TR_INVALID_PID         GRP_USBD_TR_FAIL_BASE-5     /* Invalid PID              */
#define GRP_USBD_TR_EOP                 GRP_USBD_TR_FAIL_BASE-6     /* EOP detection failure    */
#define GRP_USBD_TR_BITSTAFFING         GRP_USBD_TR_FAIL_BASE-7     /* Bit staffing error       */
#define GRP_USBD_TR_STALL               GRP_USBD_TR_FAIL_BASE-8     /* STALL end                */
#define GRP_USBD_TR_BUFFER_OVERRUN      GRP_USBD_TR_FAIL_BASE-9     /* HCD buffer overrun       */
#define GRP_USBD_TR_BUFFER_UNDERRUN     GRP_USBD_TR_FAIL_BASE-10    /* HCD output does not      */
                                                                    /* overtake                 */
                                                                    /* USB data transfer speed  */
#define GRP_USBD_TR_BABBLE_DETECTED     GRP_USBD_TR_FAIL_BASE-11    /* Babble Detected          */
#define GRP_USBD_TR_HC_DATATOGGLE_ERROR GRP_USBD_TR_FAIL_BASE-12    /* DataToggleError          */
#define GRP_USBD_TR_SPRIT_TX_FAIL       GRP_USBD_TR_FAIL_BASE-13    /* Sprit transaction error  */
#define GRP_USBD_TR_PING_TX_FAIL        GRP_USBD_TR_FAIL_BASE-14    /* Ping transaction error   */
#define GRP_USBD_TR_HC_HALTED           GRP_USBD_TR_FAIL_BASE-15    /* Host controller halted   */
#define GRP_USBD_TR_TX_FAIL             GRP_USBD_TR_FAIL_BASE-16    /* Transaction error        */
#define GRP_USBD_TR_PIPE_ERROR          GRP_USBD_TR_FAIL_BASE-17    /* Pipe insufficient error  */
#define GRP_USBD_OTHER_FAIL             GRP_USBD_TR_FAIL_BASE-20    /* Other error              */

/* Constants for device request (Default) */
/* bmRequestType */
#define GRP_USBD_TYPE_HOST2DEV          0x0                     /* Host->Device                 */
#define GRP_USBD_TYPE_DEV2HOST          0x80                    /* Device->Host                 */

#define GRP_USBD_TYPE_STANDARD          0x0                     /* Type : Standard              */
#define GRP_USBD_TYPE_CLASS             0x20                    /* Type : Class                 */
#define GRP_USBD_TYPE_VENDOR            0x40                    /* Type : Vendor                */

#define GRP_USBD_TYPE_DEVICE            0x0                     /* Receiving party : Device     */
#define GRP_USBD_TYPE_INTERFACE         0x1                     /* Receiving party : Interface  */
#define GRP_USBD_TYPE_ENDPOINT          0x2                     /* Receiving party : Endpoint   */
#define GRP_USBD_TYPE_OTHER             0x3                     /* Receiving party : Others     */

/* bRequest */
#define GRP_USBD_GET_STATUS             0                           /* GET_STATUS               */
#define GRP_USBD_CLEAR_FEATURE          1                           /* CLEAR_FEATURE            */
#define GRP_USBD_SET_FEATURE            3                           /* SET_FEATURE              */
#define GRP_USBD_SET_ADDRESS            5                           /* SET_ADDRESS              */
#define GRP_USBD_GET_DESCRIPTOR         6                           /* GET_DESCRIPTOR           */
#define GRP_USBD_SET_DESCRIPTOR         7                           /* SET_DESCRIPTOR           */
#define GRP_USBD_GET_CONFIGURATION      8                           /* GET_CONFIGURATION        */
#define GRP_USBD_SET_CONFIGURATION      9                           /* SET_CONFIGURATION        */
#define GRP_USBD_REQ_GET_INTERFACE      10                          /* GET_INTERFACE            */
#define GRP_USBD_REQ_SET_INTERFACE      11                          /* SET_INTERFACE            */
#define GRP_USBD_REQ_SYNCH_FRAME        12                          /* SET_FRAME                */

/* wValue when  bRequest is related to descriptor */
#define GRP_USBD_DESC_DEVICE            1                       /* DEVICE                       */
#define GRP_USBD_DESC_CONFIGURATION     2                       /* CONFIG                       */
#define GRP_USBD_DESC_STRING            3                       /* STRING                       */
#define GRP_USBD_DESC_INTERFACE         4                       /* INTERFACE                    */
#define GRP_USBD_DESC_ENDPOINT          5                       /* ENDPOINT                     */
#define GRP_USBD_DESC_DEVICE_QUAL       6                       /* DEVICE_QUALIFIER             */
#define GRP_USBD_DESC_OTHER_CONF        7                       /* OTHER_SPEED_CONFIGURATION    */
#define GRP_USBD_DESC_INTERFACE_PWR     8                       /* INTERFACE_POWER              */
#define GRP_USBD_DESC_OTG               9                       /* OTG                          */
#define GRP_USBD_DESC_DEBUG             10                      /* DEBUG                        */
#define GRP_USBD_DESC_INTERFACE_ASS     11                      /* INTERFACE_ASSOCIATION (IAD)  */

/* wValue of feature selector when  bRequest is CLEAR FEATURE or SET FEATURE */
#define GRP_USBD_REMOTE_WAKEUP          1                           /* DEVICE REMOTE WAKEUP     */
#define GRP_USBD_ENDPOINT_HALT          0                           /* ENDPOINT_HALT            */
#define GRP_USBD_TEST_MODE              2                           /* TEST_MODE                */

/* wIndex when bRequest is SET FEATURE and wValue is TEST MODE */
#define GRP_USBD_TEST_J                 0x01                        /* Test_J                   */
#define GRP_USBD_TEST_K                 0x02                        /* Test_K                   */
#define GRP_USBD_TEST_SE0_NAK           0x03                        /* Test_SE0_NAK             */
#define GRP_USBD_TEST_PACKET            0x04                        /* Test_Packet              */
#define GRP_USBD_TEST_FORCE_ENABLE      0x05                        /* Test_Force_Enable        */


/* Constants for descriptor */

/* For configuration descriptor */
/* bmAttributes */
#define GRP_USBD_CD_BMATTR_SELF_POWER   0x40                        /* Self-powerd Bit          */
#define GRP_USBD_CD_BMATTR_REMOTE_WKUP  0x20                        /* Remote Wakeup Bit        */

/* For endpoint descriptor */
/* bEndpointAddress */
#define GRP_USBD_ED_BEPADDRESS_PID_BIT  0x80                        /* Direction bit            */
                                                                    /* 0:OUT 1:IN               */
#define GRP_USBD_ED_BEPADDRESS_MASK     0xF                         /* Endpoint number          */
                                                                    /* Mask                     */

/* bmAttributes */
#define GRP_USBD_ED_BMATTR_CONTROL      0x00                    /* Control                      */
#define GRP_USBD_ED_BMATTR_ISOCRONOUS   0x01                    /* Isochronous                  */
#define GRP_USBD_ED_BMATTR_BULK         0x02                    /* Bulk                         */
#define GRP_USBD_ED_BMATTR_INTERRUPT    0x03                    /* Interrupt                    */
#define GRP_USBD_ED_BMATTR_NO_SYNC      0x00                    /* No Synchronization           */
#define GRP_USBD_ED_BMATTR_ASYNC        0x04                    /* Asynchronous                 */
#define GRP_USBD_ED_BMATTR_ADAPTIVE     0x08                    /* Adaptive                     */
#define GRP_USBD_ED_BMATTR_SYNC         0x0C                    /* Synchronous                  */
#define GRP_USBD_ED_BMATTR_DATA         0x00                    /* Data endpoint                */
#define GRP_USBD_ED_BMATTR_FEEDBACK     0x10                    /* Feedback endpoint            */
#define GRP_USBD_ED_BMATTR_IMPLICIT     0x20                    /* Implicit feedback endpoint   */

#define GRP_USBD_ED_BMATTR_MASK         0x03                    /* Transfer type mask           */
#define GRP_USBD_ED_BMATTR_SYNC_MASK    0x0C                    /* Synchronization type mask    */
#define GRP_USBD_ED_BMATTR_ETYPE_MASK   0x30                    /* Endpoint usage type          */

/* wMaxPacketSize */
#define GRP_USBD_ED_WMAXPKTSIZ_MPS_MASK 0x07FF                  /* Max Packet Size              */
#define GRP_USBD_ED_WMAXPKTSIZ_ADDTR_0  0x0000                  /* no additional                */
#define GRP_USBD_ED_WMAXPKTSIZ_ADDTR_1  0x0800                  /* 1 additional                 */
#define GRP_USBD_ED_WMAXPKTSIZ_ADDTR_2  0x1000                  /* 2 additional                 */
#define GRP_USBD_ED_WMAXPKTSIZ_ADD_MASK 0x1800                  /* additional transaction mask  */

/* Actual size of structure defined */
/* (for cuttting out the padded bits to get the actual memory size) */
#define GRP_USBD_DEVICE_DESC_SIZE       18
#define GRP_USBD_INTERFACE_DESC_SIZE    9
#define GRP_USBD_CONFIG_DESC_SIZE       9
#define GRP_USBD_ENDPOINT_DESC_SIZE     7

/* Function parameters */

/* Communication speed (grp_usbd__ConnectDevice function)*/
#define GRP_USBD_UNKNOWN_SPEED          0                           /* Unknown Speed            */
#define GRP_USBD_LOW_SPEED              1                           /* Low Speed                */
#define GRP_USBD_FULL_SPEED             2                           /* Full Speed               */
#define GRP_USBD_HIGH_SPEED             3                           /* High Speed               */

/* Frame control(grp_usbd_FrameWidthControl function) */
#define GRP_USBD_GET_FRAME_WIDTH        0                           /* Get framewidth           */
#define GRP_USBD_SET_FRAME_WIDTH        1                           /* Set framewidth           */

/* Frame number control(grp_usbd_FrameNumberControl function) */
#define GRP_USBD_GET_FRAME_NUM          0                           /* Get frame number         */
#define GRP_USBD_SET_FRAME_NUM          1                           /* Set frame number         */

/* bmRequestRecipient (grp_usbd_GetStatus function) */
#define GRP_USBD_DEVICE                 GRP_USBD_TYPE_DEVICE
#define GRP_USBD_INTERFACE              GRP_USBD_TYPE_INTERFACE
#define GRP_USBD_ENDPOINT               GRP_USBD_TYPE_ENDPOINT

/* Pipe state(grp_usbd_PipeCheck function) */
#define GRP_USBD_PIPE_ACTIVE            0                           /* Pipe ACTIVE              */
#define GRP_USBD_PIPE_HALT              1                           /* Pipe HALT                */

/* Constants for structures */
/* grp_usbdi_pipe(iTransferMode) */
#define GRP_USBD_CONTROL                0                           /* Control transfer         */
#define GRP_USBD_ISOCHRONOUS            1                           /* Isochronous transfer     */
#define GRP_USBD_BULK                   2                           /* Bulk transfer            */
#define GRP_USBD_INTERRUPT              3                           /* Interrupt transfer       */
#define GRP_USBD_UNKNOWN_MODE           9                           /* Unknown transfer         */

/* grp_usbdi_pipe(ulInterval) */
#define GRP_USBD_INTERVAL_1ms           0                           /* 1ms                      */
#define GRP_USBD_INTERVAL_2ms           1                           /* 2ms                      */
#define GRP_USBD_INTERVAL_4ms           2                           /* 4ms                      */
#define GRP_USBD_INTERVAL_8ms           3                           /* 8ms                      */
#define GRP_USBD_INTERVAL_16ms          4                           /* 16ms                     */
#define GRP_USBD_INTERVAL_32ms          5                           /* 32ms                     */
#define GRP_USBD_INTERVAL_125us         6                           /* 125us (1uFrame)          */
#define GRP_USBD_INTERVAL_250us         7                           /* 250us (2uFrame)          */
#define GRP_USBD_INTERVAL_500us         8                           /* 500us (4uFrame)          */
#define GRP_USBD_NO_INTERVAL            255                    

/* grp_usbdi_request(ucTransferDirection) */
#define GRP_USBD_TX_OUT                 0                           /* OUT                      */
#define GRP_USBD_TX_IN                  1                           /* IN                       */
#define GRP_USBD_TX_INOUT               2                           /* Bi-directional           */
#define GRP_USBD_TX_NODATA              GRP_USBD_TX_INOUT           /* NO DATA                  */

/* Specify the number of additional transaction opportunities per microframe */
#define GRP_USBD_NO_ADD_TX_PER_FRAME    0                   /* No additional transaction        */
#define GRP_USBD_1_ADD_TX_PER_FRAME     1                   /* 1 additional transaction         */
#define GRP_USBD_2_ADD_TX_PER_FRAME     2                   /* 2 additional transaction         */

/* Port Control */
#define GRP_USBD_PORT_SUSPEND_CONTROL   0                           /* port suspend             */
#define GRP_USBD_PORT_RESUME_CONTROL    1                           /* port resume              */
#define GRP_USBD_PORT_RESET_CONTROL     2                           /* port reset               */

/* Root hub address */
#define GRP_USBD_ROOTHUB_ADDRESS        0

/* Max. number of pipes per device */
#define GRP_USBD_MAX_PIPE               16

/* Max. number of interface per device */
#define GRP_USBD_MAX_IF                 16

/* Control transfer max packet size */
#define GRP_USBD_HIGH_SPEED_CTRL_MPS    64                          /* High speed               */
#define GRP_USBD_FULL_SPEED_CTRL_MPS    64                          /* Full speed               */
#define GRP_USBD_LOW_SPEED_CTRL_MPS     8                           /* Low speed                */

/* Port request */
#define GRP_USBD_GLOBAL_SUSPEND         0                           /* Global suspend           */
#define GRP_USBD_GLOBAL_RESUME          1                           /* Global resume            */
#define GRP_USBD_PORT_SUSPEND           2                           /* Port suspend             */
#define GRP_USBD_PORT_RESUME            3                           /* Port resume              */
#define GRP_USBD_PORT_RESET             4                           /* Port reset               */

/* Notify event */
#define GRP_USBD_HC_HALT_EVENT          0
#define GRP_USBD_HC_RESUME_EVENT        1
#define GRP_USBD_PORT_HALT_EVENT        2
#define GRP_USBD_PORT_RESUME_EVENT      3

/* Port information */
#define GRP_USBD_USB_1X                 0                           /* USB 1.x                  */
#define GRP_USBD_USB_20                 1                           /* USB 2.0                  */
#define GRP_USBD_USB_20_OTG             2                           /* USB 2.0 OTG              */

/* Device identifier macro declaration */
#define GRP_USBD_DEVID_ADDR_MASK        0x00ff
#define GRP_USBD_DEVID2ADDR(id)         (grp_u8)((grp_u16)(id) & GRP_USBD_DEVID_ADDR_MASK)

/* Device identifier and address */
#define GRP_USBD_DEFAULT_DEVID          0x8000                  /* default device identifier    */
#define GRP_USBD_DEFAULT_ADDRESS        0                       /* default device address       */
#define GRP_USBD_DEFAULT_ENDPOINT       0                       /* default endpoint             */

/* Isochronous buffer mode */
#define GRP_USBD_ISO_BUF_WHOLE_MODE     0                   /* Address is max packet alignment  */
                                                            /* Length is max packet size        */
#define GRP_USBD_ISO_BUF_ADDRESS_MODE   3                   /* Address is specify               */
                                                            /* Length is distance to the next   */
#define GRP_USBD_ISO_BUF_NATURAL_MODE   4                   /* Address is max packet alignment  */
                                                            /* Length is specify                */
#define GRP_USBD_ISO_BUF_ASSIGN_MODE    5                   /* Address and length specify       */
#define GRP_USBD_ISO_BUF_COUNT_MODE     6                   /* Address is following transfer    */
                                                            /* Length is specify                */
                                                            /* Number of transfer is specify    */
#define GRP_USBD_ISO_BUF_PACKED_MODE    7                   /* Address is following transfer    */
                                                            /* Length is specify                */
                                                            /* Until the buffer becomes full    */


/* Structure for USB driver interface */
/* Pipe handler structure */

typedef struct grp_usbdi_request grp_usbdi_request;

typedef struct
{
    grp_u16                 usUsbDevId;                         /* Device ID(Device Address+Tag)*/
    grp_u16                 usMaxPacketSize;                    /* Max. packet size of the pipe */
    grp_u8                  ucEndpointNumber;                   /* Endpoint number of the pipe  */
    grp_u8                  ucBelongInterface;                  /* Interface to which belongs   */
    grp_u8                  ucBelongAlternate;                  /* Alternate to which belongs   */
    grp_u8                  ucTransferDirection;                /* Pipe transfer direction      */
                                                                /* 0 : OUT    1 : IN            */
    grp_si                  iTransferMode;                      /* Transfer type of the pipe    */
                                                                /* 0 : Control                  */
                                                                /* 1 : Isochronous              */
                                                                /* 2 : Bulk                     */
                                                                /* 3 : Interrupt                */
    grp_u32                 ulInterval;                         /* Pipe polling interval        */
    grp_u32                 ulBandWidth;                        /* Pipe bandwidth               */
                                                                /* (Unit : ns)                  */
    grp_si                  iStatus;                            /* Current status of the pipe   */
                                                                /* 0 : ACTIVE  1 : HALT         */
    grp_hcdi_endpoint       tEndpoint;                          /* Endpoint Area                */

} grp_usbdi_pipe;

/* Communication request structure */
struct grp_usbdi_request
{
    grp_usbdi_pipe          *ptPipe;                            /* Pipe to transfer             */
    grp_u8                  *pucBuffer;                         /* Communication buffer         */
                                                                /* (for data)                   */
    grp_u32                 ulBufferLength;                     /* Length of buffer for data    */
                                                                /* phase                        */
    grp_u32                 ulActualLength;                     /* Data size actually           */
    grp_si                  iShortXferOK;                       /* transmitted Judgement flag   */
                                                                /* for underrun                 */
                                                                /* TRUE  : Success              */
                                                                /* FALSE : Error                */
    grp_s32                 lStatus;                            /* Communication result         */
                                                                /* returned from HCD            */
    grp_s32                 (*pfnNrCbFunc)(grp_usbdi_request *);/* Callback function called at  */
                                                                /* the end of transfer          */
    void                    *pvReferenceP;                      /* Pointer available for user   */

    /* for Internal Use */
    grp_hcdi_tr_request     tIrp;                               /* Inernal request packet Area  */

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
    /* Used at Isochronous transfer */
    grp_u32                 ulIsoStartFrame;                    /* Start frame number           */
    grp_u32                 ulIsoErrorCount;                    /* Number of errors at          */
                                                                /* Isochronous transfer         */
    grp_si                  iIsoImmediatelyFlag;                /* Start Isochronous transfer   */
                                                                /* immediately                  */
    grp_hcdi_iso_status     *ptIsoStatus;                       /* Status array pointer         */
    grp_u32                 ulIsoStatusNum;                     /* Number of arrey for status   */
    grp_u32                 ulIsoBufferMode;                    /* Select the buffer mode       */
#endif

#ifdef GRP_USB_HOST_USE_CTRL_NML_PIPE
    /* Used at Control transfer in endpoint xx */
    grp_u8                  *pucSetup;                          /* Communication buffer         */
                                                                /* (for SETUP)                  */
    grp_u8                  ucTransferDirection;                /* Transfer direction           */
                                                                /* 0 : OUT                      */
                                                                /* 1 : IN                       */
                                                                /* (Used only at Control        */
                                                                /*   transfer)                  */
    grp_u8                  aucPadding[3];
#endif  /* GRP_USB_HOST_USE_CTRL_NML_PIPE */

};

/* Send request(Device request communication) structure */
typedef struct grp_usbdi_device_request grp_usbdi_device_request;

struct grp_usbdi_device_request
{
    grp_u8                  bmRequestType;                      /* Request information          */
    grp_u8                  bRequest;                           /* Request                      */
    grp_u16                 wValue;                             /* Send value                   */
    grp_u16                 wIndex;                             /* Send index                   */
    grp_u16                 wLength;                            /* Number of byte at data stage */
    grp_u8                  *pucBuffer;                         /* Buffer for data stage        */
    grp_s32                 (*pfnDvCbFunc)(grp_usbdi_device_request *);
                                                                /* Callback function called at  */
                                                                /* communication end            */
    grp_s32                 lStatus;                            /* Transfer result returned     */
                                                                /* from HCD                     */
    grp_u32                 ulActualLength;                     /* Data size actually           */
    void                    *pvReferenceP;                      /* Pointer available for user   */
    grp_u16                 usUsbDevId;                         /* Device ID(Device Address+Tag)*/

    grp_u8                  aucPadding[2];                      /* padding                      */

    /* for Internal Use */
    grp_hcdi_tr_request     tIrp;                               /* Inernal request packet Area  */
};

/* Send request structure(COMMAND INTERFACE communication) */
typedef struct grp_usbdi_st_device_request grp_usbdi_st_device_request;

struct grp_usbdi_st_device_request
{
    grp_u16                 usUsbDevId;                 /* Device ID (Device address + Tag)     */
    grp_u8                  aucRetStatus[2];            /* Get Status Return Status             */
    grp_u8                  ucConfiguration;            /* Configuration number for setting or  */
                                                        /* making a request                     */
    grp_u8                  ucInterface;                /* Interface number for setting or      */
                                                        /* requesting                           */
    grp_u8                  ucAlternate;                /* Alternative set number for setting or*/
                                                        /* making a request                     */
    grp_u8                  ucEndpoint;                 /* Set Feature Endpoint                 */
    grp_u8                  bmRequestRecipient;         /* Characteristics of request           */
    grp_u8                  ucFeatureSelector;          /* Set Feature Feature Selector         */
    grp_u8                  ucIndex;                    /* Descriptor index number to get       */
    grp_u8                  ucEp0size;                  /* Max. communication size of endpoint 0*/
                                                        /* which is opened at SetAddress        */
    grp_u8                  ucPipeNumber;               /* Number of pipe handler opened at     */
                                                        /* SetInterface                         */
    grp_u8                  aucReserved[3];             /* Reserved                             */
    grp_usbdi_pipe          *ptPipe;                    /* Area to store the pipe handler opened*/
                                                        /* at SetInterface                      */
    void                    *pvDescriptor;              /* Descriptor information               */
    grp_u32                 ulSize;                     /* Descriptor size                      */
    grp_s32                 lStatus;                    /* Transfer result returned from HCD    */
    grp_u32                 ulActualLength;                     /* Data size actually           */
    grp_s32                 (*pfnStCbFunc)(grp_usbdi_st_device_request *);
                                                        /* Called at the end of transfer        */
                                                        /* Callback function                    */
    void                    *pvReferenceP;              /* Pointer available for user           */

    /* for Internal Use */
    grp_hcdi_tr_request     tIrp;                       /* Inernal request packet Area          */
    
    grp_u16                 usLangID;                   /* Langedge ID                          */
    grp_u8                  aucPadding[2];              /* Padding                              */
};

/* Memory allocation for request */
typedef struct grp_usbdi_req_pool grp_usbdi_req_pool;

struct grp_usbdi_req_pool
{
    grp_u32                             ulStatus;

    union{
        grp_usbdi_st_device_request     tStd;
        grp_usbdi_device_request        tDev;
        grp_usbdi_request               tNml;
    }tReq;
};

/* Pipe selection structure */
typedef struct grp_usbdi_pipe_select grp_usbdi_pipe_select;

struct grp_usbdi_pipe_select
{
    grp_u8                  ucTransferMode;                     /* Transfer type of the pipe    */
                                                                /* 0 : Control                  */
                                                                /* 1 : Isochronous              */
                                                                /* 2 : Bulk                     */
                                                                /* 3 : Interrupt                */
    grp_u8                  ucTransferDirection;                /* Pipe transfer direction      */
    grp_u8                  ucInternalFlag;                     /* Flag used internally         */
    grp_u8                  aucReserved[1];                     /* Reserved                     */
};

/* Pipe operation structure */
typedef struct grp_usbdi_pipe_operate grp_usbdi_pipe_operate;

struct grp_usbdi_pipe_operate
{
    grp_u16                 usUsbDevId;                 /* Device ID (Device address + Tag)     */
                                                        /* making a request                     */
    grp_u8                  ucInterface;                /* Interface number for setting or      */
                                                        /* requesting                           */
    grp_u8                  ucPipeNumber;               /* Number of pipe handler opened at     */
                                                        /* SetInterface                         */
    grp_usbdi_pipe          *ptPipe;                    /* Area to store the pipe handler opened*/
                                                        /* at SetInterface                      */
    void                    *pvDescriptor;              /* Descriptor information               */
    grp_u32                 ulSize;                     /* Descriptor size                      */
    grp_usbdi_pipe_select   atSelPipe[GRP_USBD_SELECT_PIPE];/* Pipe selection information       */
};

/* Descriptor structure */
/* String descriptor (unicode array) */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u16                 wLANGID[127];                       /* UNICODE string               */
} grp_usbdi_str_desc_array;

/* String descriptor */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u8                  bString[254];                       /* UNICODE string               */
} grp_usbdi_str_desc;

/* Standard configuration descriptor */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u16                 wTotalLength;                       /* Total descriptor length      */
    grp_u8                  bNumInterface;                      /* Number of interfaces to      */
                                                                /* be configured                */
    grp_u8                  bConfigurationValue;                /* Values to select             */
                                                                /* configuration                */
    grp_u8                  iConfiguration;                     /* Indes of string descriptor   */
                                                                /* which represents             */
                                                                /* configuration                */
    grp_u8                  bmAttributes;                       /* Configuration attributes     */
    grp_u8                  MaxPower;                           /* Max. power consumption for   */
                                                                /* bus                          */
}grp_usbdi_config_desc;

/* Standard configuration descriptor (byte order) */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u8                  wTotalLength_Low;                   /* Total descriptor length (low)*/
    grp_u8                  wTotalLength_High;                  /* Total descriptor length      */
                                                                /* (high)                       */
    grp_u8                  bNumInterface;                      /* Number of interfaces to      */
                                                                /* be configured                */
    grp_u8                  bConfigurationValue;                /* Values to select             */
                                                                /* configuration                */
    grp_u8                  iConfiguration;                     /* Indes of string descriptor   */
                                                                /* which represents             */
                                                                /* configuration                */
    grp_u8                  bmAttributes;                       /* Configuration attributes     */
    grp_u8                  MaxPower;                           /* Max. power consumption for   */
                                                                /* bus                          */
} grp_usbdi_config_desc_b;

/* Standard device descriptor */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u16                 bcdUSB;                             /* USB release nubmer           */
                                                                /* (BCD representaion)          */
    grp_u8                  bDeviceClass;                       /* Class code                   */
    grp_u8                  bDeviceSubClass;                    /* Sub-class code               */
    grp_u8                  bDeviceProtocol;                    /* Protocol code                */
    grp_u8                  bMaxPacketSize;                     /* Max.packet size of endpoint 0*/
    grp_u16                 idVendor;                           /* Vendor ID                    */
    grp_u16                 idProduct;                          /* Product ID                   */
    grp_u16                 bcdDevice;                          /* Device release number(BCD)   */
    grp_u8                  iManufacturer;                      /* Index of string descriptor   */
                                                                /* which represents a           */
                                                                /* manufacturer                 */
    grp_u8                  iProduct;                           /* Index of string descriptor   */
                                                                /* which represents a product   */
    grp_u8                  iSerialNumber;                      /* Index of string descriptor   */
                                                                /* which represents a device    */
                                                                /* manufacture number           */
    grp_u8                  bNumConfiguration;                  /* Number of possible           */
                                                                /* configuration                */
} grp_usbdi_dev_desc;

/* Standard device descriptor (byte order) */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u8                  bcdUSB_Low;                         /* USB release nubmer (low)     */
                                                                /* (BCD representaion)          */
    grp_u8                  bcdUSB_High;                        /* USB release nubmer (high)    */
                                                                /* (BCD representaion)          */
    grp_u8                  bDeviceClass;                       /* Class code                   */
    grp_u8                  bDeviceSubClass;                    /* Sub-class code               */
    grp_u8                  bDeviceProtocol;                    /* Protocol code                */
    grp_u8                  bMaxPacketSize;                     /* Max.packet size of endpoint 0*/
    grp_u8                  idVendor_Low;                       /* Vendor ID (low)              */
    grp_u8                  idVendor_High;                      /* Vendor ID (high)             */
    grp_u8                  idProduct_Low;                      /* Product ID (low)             */
    grp_u8                  idProduct_High;                     /* Product ID (high)            */
    grp_u8                  bcdDevice_Low;                      /* Device release number        */
                                                                /* (BCD)(low)                   */
    grp_u8                  bcdDevice_High;                     /* Device release number        */
                                                                /* (BCD)(high)                  */
    grp_u8                  iManufacturer;                      /* Index of string descriptor   */
                                                                /* which represents a           */
                                                                /* manufacturer                 */
    grp_u8                  iProduct;                           /* Index of string descriptor   */
                                                                /* which represents a product   */
    grp_u8                  iSerialNumber;                      /* Index of string descriptor   */
                                                                /* which represents a device    */
                                                                /* manufacture number           */
    grp_u8                  bNumConfiguration;                  /* Number of possible           */
                                                                /* configuration                */
} grp_usbdi_dev_desc_b;

/* Device qualifier descriptor */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u16                 bcdUSB;                             /* USB release nubmer           */
                                                                /* (BCD representaion)          */
    grp_u8                  bDeviceClass;                       /* Class code                   */
    grp_u8                  bDeviceSubClass;                    /* Sub-class code               */
    grp_u8                  bDeviceProtocol;                    /* Protocol code                */
    grp_u8                  bMaxPacketSize;                     /* Max.packet size of endpoint 0*/
    grp_u8                  bNumConfiguration;                  /* Number of possible           */
                                                                /* configuration                */
    grp_u8                  bReserved;                          /* Reserved                     */

} grp_usbdi_dev_qual_desc;

/* Device qualifier descriptor (byte order) */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u8                  bcdUSB_Low;                         /* USB release nubmer (low)     */
                                                                /* (BCD representaion)          */
    grp_u8                  bcdUSB_High;                        /* USB release nubmer (high)    */
                                                                /* (BCD representaion)          */
    grp_u8                  bDeviceClass;                       /* Class code                   */
    grp_u8                  bDeviceSubClass;                    /* Sub-class code               */
    grp_u8                  bDeviceProtocol;                    /* Protocol code                */
    grp_u8                  bMaxPacketSize;                     /* Max.packet size of endpoint 0*/
    grp_u8                  bNumConfiguration;                  /* Number of possible           */
                                                                /* configuration                */
    grp_u8                  bReserved;                          /* Reserved                     */

} grp_usbdi_dev_qual_desc_b;

/* Standard endpoint descriptor */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u8                  bEndpointAddress;                   /* Endpoint address             */
    grp_u8                  bmAttributes;                       /* Attributes                   */
    grp_u16                 wMaxPacketSize;                     /* Max. packet size             */
    grp_u8                  bInterval;                          /* Polling interval             */
} grp_usbdi_ep_desc;

/* Standard endpoint descriptor (byte order) */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u8                  bEndpointAddress;                   /* Endpoint address             */
    grp_u8                  bmAttributes;                       /* Attributes                   */
    grp_u8                  wMaxPacketSize_Low;                 /* Max. packet size (low)       */
    grp_u8                  wMaxPacketSize_High;                /* Max. packet size (high)      */
    grp_u8                  bInterval;                          /* Polling interval             */
} grp_usbdi_ep_desc_b;

/* Standard interface descriptor */
typedef struct
{
    grp_u8                  bLength;                            /* Descriptor size              */
    grp_u8                  bDescriptor;                        /* Descriptor type              */
    grp_u8                  bInterfaceNumber;                   /* Index number of this         */
                                                                /* interface                    */
    grp_u8                  bAlternateSetting;                  /* Parameter for alternatoive   */
                                                                /* setting                      */
    grp_u8                  bNumEndpoints;                      /* Number of endpoints for      */
                                                                /* interface (except 0)         */
    grp_u8                  bInterfaceClass;                    /* Class code                   */
    grp_u8                  bInterfaceSubClass;                 /* Sub-class code               */
    grp_u8                  bInterfaceProtocol;                 /* Protocol code                */
    grp_u8                  iInterface;                         /* Index of string descriptor   */
                                                                /* which represents this        */
                                                                /* interface                    */
} grp_usbdi_if_desc;

/* Standard interface association descriptor */
typedef struct
{
    grp_u8                  bLength;                /* Descriptor size                          */
    grp_u8                  bDescriptor;            /* Descriptor type                          */
    grp_u8                  bFirstInterface;        /* Interface number of the first interface  */
    grp_u8                  bInterfaceCount;        /* Number of contiguous interface           */
    grp_u8                  bFunctionClass;         /* Class code                               */
    grp_u8                  bFunctionSubClass;      /* Sub-class code                           */
    grp_u8                  bFunctionProtocol;      /* Protocol code                            */
    grp_u8                  iFunction;              /* Index of string descriptor               */
} grp_usbdi_iad_desc;

/* Descriptor analysis information structure */
typedef struct
{
    grp_u8                  ucInterface;                /* Descriptor number for analysis       */
    grp_u8                  ucAlternate;                /* Alternatively set number to analyze  */
    grp_u8                  ucEndpoint;                 /* Endpoint number for analysis         */
    grp_u8                  ucPadding;                  /* Dummy(Padding)                       */
    grp_u32                 ulSize;                     /* Descriptor size for analysis         */
    void                    *pvDesc;                    /* Descriptor to analyze                */
} grp_usbdi_descriptor_info;

/* Device (Port) Information */
typedef struct
{
    grp_u16                 usUsbDevId;                 /* Device ID (Device address + Tag)     */
    grp_u16                 usHcIndexNum;               /* Host controller index number         */
    grp_u8                  ucHubAddr;                  /* Hub Address (Root Hub Address:1)     */
    grp_u8                  ucPortNum;                  /* Port Number                          */
    grp_u8                  ucPortInfo;                 /* Port Information                     */
    grp_u8                  aucPadding[1];              /* padding                              */

} grp_usbdi_device_info;

/* Bus Control Structure */
typedef struct
{
    grp_usbdi_device_info   tDev;                   /* Device information                       */
    grp_si                  iReq;                   /* Request                                  */
                                                    /* GRP_USBD_GLOBAL_SUSPEND  : Global Suspend*/
                                                    /* GRP_USBD_GLOBAL_RESUME   : Global Resume */
                                                    /* GRP_USBD_PORT_SUSPEND    : Port Suspend  */
                                                    /* GRP_USBD_PORT_RESUME     : Port Resume   */
                                                    /* GRP_USBD_PORT_RESET      : Port Reset    */
} grp_usbdi_bus_control;

/* USB driver initialization structure */
typedef struct
{
    void                    *pvPrm;                     /* Parameter (for furture use)          */

} grp_usbdi_system_init;


/* Prototype declaration */
EXTERN grp_s32 grp_usbd_Init(grp_usbdi_system_init *);
EXTERN grp_s32 grp_usbd_IdToAddress(grp_u16,grp_u8 *);
EXTERN grp_s32 grp_usbd_BusPowerControl(grp_usbdi_bus_control *);
EXTERN grp_s32 grp_usbd_ClearFeature(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_ConnectDevice(grp_si,grp_usbdi_device_info *);
EXTERN grp_s32 grp_usbd_DeviceRequest(grp_usbdi_device_request *);
EXTERN grp_s32 grp_usbd_DeviceRequestCancel(grp_usbdi_device_request *);
EXTERN grp_s32 grp_usbd_DisconnectDevice(grp_u16);
EXTERN grp_s32 grp_usbd_FrameWidthControl(grp_u16,grp_si,grp_s32 *);
EXTERN grp_s32 grp_usbd_FrameNumberControl(grp_u16,grp_si,grp_s32 *);
EXTERN grp_s32 grp_usbd_GetConfigDescriptor(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_GetDeviceDescriptor(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_GetDeviceInfo(grp_u16,grp_usbdi_device_info *);
EXTERN grp_s32 grp_usbd_GetStatus(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_GetMasterClient(grp_u16);
EXTERN grp_s32 grp_usbd_GetStringDescriptor(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_InterfaceOpen(grp_usbdi_pipe_operate *);
EXTERN grp_s32 grp_usbd_InterfaceClose(grp_usbdi_pipe_operate *);
EXTERN grp_s32 grp_usbd_InterfaceCheck( grp_usbdi_pipe_operate *ptOper, grp_u8 ucAlternate );
EXTERN grp_s32 grp_usbd_NormalRequest(grp_usbdi_request *);
EXTERN grp_s32 grp_usbd_NormalRequestCancel(grp_usbdi_request *);
EXTERN grp_s32 grp_usbd_NotifyEvent(grp_u16,grp_u16,grp_si);
EXTERN grp_s32 grp_usbd_SearchEpDescriptor(grp_usbdi_descriptor_info *,grp_usbdi_ep_desc *);
EXTERN grp_s32 grp_usbd_SearchIfDescriptor(grp_usbdi_descriptor_info *,grp_usbdi_if_desc *);
EXTERN grp_s32 grp_usbd_SearchIfClass(grp_usbdi_descriptor_info *,grp_u8,grp_u8,grp_u8,grp_usbdi_if_desc **);
EXTERN grp_s32 grp_usbd_PipeAbort(grp_usbdi_pipe *);
EXTERN grp_s32 grp_usbd_PipeActive(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_PipeCheck(grp_usbdi_pipe *,grp_si *);
EXTERN grp_s32 grp_usbd_PipeClose(grp_usbdi_pipe *);
EXTERN grp_s32 grp_usbd_PipeHalt(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_PipeOpen(grp_u16,grp_usbdi_ep_desc *,grp_usbdi_pipe *);
EXTERN grp_s32 grp_usbd_PipeReset(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_ReleaseMasterClient(grp_u16);
EXTERN grp_s32 grp_usbd_SearchDescriptor(grp_usbdi_config_desc *,grp_u8,void *,void **,void **);
EXTERN grp_s32 grp_usbd_SetAddress(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_SetConfiguration(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_SetConfigDescriptor(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_SetDeviceDescriptor(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_SetFeature(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_SetStringDescriptor(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_SetInterface(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_UnSetConfiguration(grp_usbdi_st_device_request *);
EXTERN grp_s32 grp_usbd_Pipe0Abort(grp_u16);
EXTERN grp_s32 grp_usbd_CheckFreeAddress(void);
EXTERN grp_s32 grp_usbd_StDeviceRequestCancel(grp_usbdi_st_device_request *);
EXTERN grp_si  grp_usbd_GetDevicePortStatus(grp_u16);
EXTERN grp_si  grp_usbd_GetInterfaceStatus( grp_u16 usDevId, grp_u8 ucIfNum );
EXTERN grp_s32 grp_usbd_SetDevicePortStatus(grp_u16,grp_si);
EXTERN grp_s32 grp_usbd_LockEnumeration(void);
EXTERN grp_s32 grp_usbd_UnlockEnumeration(void);
EXTERN grp_s32 grp_usbd_LockOpen(void);
EXTERN grp_s32 grp_usbd_UnlockOpen(void);
EXTERN grp_s32 grp_usbd_LockAddress(void);
EXTERN grp_s32 grp_usbd_UnlockAddress(void);
EXTERN grp_s32 grp_usbd_GetIsochronousInfo(grp_usbdi_pipe *, grp_u32 *, grp_u32 *, grp_u8 *);
EXTERN grp_s32 grp_usbd_SynchFrame(grp_usbdi_st_device_request *);

#endif /* _GRP_USBD_H_ */
