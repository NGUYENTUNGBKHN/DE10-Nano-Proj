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
/*      grp_usbd_local.h                                                        1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# USB driver local file                                                      */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "FILE NAME" and "DESCRIPTION" of the file header.  */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_USBD_LOCAL_H_
#define _GRP_USBD_LOCAL_H_

/* Address control */
#define GRP_USBD_ADDRESS_NO_ASIGNED     0
#define GRP_USBD_ADDRESS_ASIGNED        1
#define GRP_USBD_NO_ADDRESS             255

#define GRP_USBD_DEVID_NO_ASIGNED       0
#define GRP_USBD_DEVID_TAG_MIN          0x0001
#define GRP_USBD_DEVID_TAG_MAX          0x007f

#define GRP_USBD_NO_CLIANT_ADDRESS      0

#define GRP_USBD_ADDRESS_MASK           0x7F

/* USB driver physical connection status */
#define GRP_USBD_STATE_BLANK            (0)             /* Port No connect and close            */
#define GRP_USBD_STATE_CONNECT          (1)             /* Port Attach but not open             */
#define GRP_USBD_STATE_DISCONNECT       (2)             /* Port Detach but not close            */
#define GRP_USBD_STATE_OPEN             (3)             /* Port Attach and open                 */
#define GRP_USBD_STATE_HIBERNATE        (5)             /* Port Suspend when connect            */
#define GRP_USBD_STATE_SUSPEND          (6)             /* Port Suspend when open               */
#define GRP_USBD_STATE_HALTED           (7)             /* Port Halted                          */
#define GRP_USBD_STATE_UNKNOWN          (9)             /* Unknown Port Status                  */

/* Host Controller Status */
#define GRP_USBD_HC_ACTIVE              0               /* Host Controller Active State         */
#define GRP_USBD_HC_HALTED              1               /* Host Controller Halted State         */
#define GRP_USBD_HC_SUSPEND             2               /* Host Controller Suspend State        */
#define GRP_USBD_HC_UNKNOWN             9               /* Unknown Host Controller State        */

/* Bandwidth */
#define GRP_USBD_HC_MAXBANDWIDTH        900000          /* Max Bandwidth (900000ns)             */

/* Setup data */
#define GRP_USBD_SETUP_BMREQUESTTYPE    0               /* offset 0 : bmRequestType             */
#define GRP_USBD_SETUP_BREQUEST         1               /* offset 1 : bRequest                  */
#define GRP_USBD_SETUP_WVALUE_LOW       2               /* offset 2 : wValue (Low Byte)         */
#define GRP_USBD_SETUP_WVALUE_HIGH      3               /* offset 3 : wValue (High Byte)        */
#define GRP_USBD_SETUP_WINDEX_LOW       4               /* offset 4 : wIndex (Low Byte)         */
#define GRP_USBD_SETUP_WINDEX_HIGH      5               /* offset 5 : wIndex (High Byte)        */
#define GRP_USBD_SETUP_WLENGTH_LOW      6               /* offset 6 : wLength (Low Byte)        */
#define GRP_USBD_SETUP_WLENGTH_HIGH     7               /* offset 7 : wLength (High Byte)       */

#define GRP_USBD_SETUP_DATASIZE         8               /* Setup data size (8Byte)              */

/* Frame number */
#define GRP_USBD_FRAMENUMBER_MASK       (0x7FF)

/* Device Request */
#define GRP_USBD_DREQ_GET_STAT_DATALEN  2               /* Get Status Data Length               */
#define GRP_USBD_DREQ_SYNC_FRM_DATALEN  2               /* Synch Frame Data Length              */

/* OS Resourse (Semaphore) */
#define GRP_USBD_MAX_SEMAPHORE          3               /* Max Semaphore                        */
#define GRP_USBD_ADDRESS_SEM            0               /* Address semaphore ID                 */
#define GRP_USBD_DEFAULTPIPE_SEM        1               /* Default pipe semaphore ID            */
#define GRP_USBD_OPENPIPE_SEM           2               /* Open pipe semaphore ID               */

/* Host Controller Data Area */
typedef struct _grp_usbd_host_data
{
    grp_hcdi_request        tHcdiFunc;                  /* HCD I/O Request Functions            */
    grp_u32                 ulBandWidth;                /* Band Width                           */
    grp_u32                 aulMicroWidth[8];           /* Micro Frame Band Width               */
    grp_u8                  ucMasterCliantAddr;         /* Master Cliant Address                */
    grp_u8                  ucHcState;                  /* Host Controller Status               */
    grp_u16                 usHcIndexNum;               /* Host Controller Index number         */
    grp_u32                 ulHostDelay;                /* Host Controller transfer delay time  */
    grp_u32                 ulLsSetup;                  /* Host Controller low speed setup time */
    void                    *pvHcdworkPtr;              /* Host Controller Work                 */
                                                        /* Area Pointer                         */
} grp_usbd_host_data;

/* Interface Information */
typedef struct _grp_usbd_if_info
{
    grp_u8                  ucState;                    /* Open or Close State of Interface     */
    grp_u8                  ucAltNum;                   /* Alternate Number                     */
} grp_usbd_if_info;

/* Device Table */
typedef struct _grp_usbd_device_table
{
    grp_si                  iPortState;                 /* Connect Status                       */
    grp_si                  iSpeed;                     /* Current device speed                 */
                                                        /* 0:FullSpeed 1:LowSpeed               */
    grp_usbdi_pipe          tDefaultPipe;               /* Default Pipe Area                    */
    grp_usbdi_device_info   tDeviceInfo;                /* Device Information                   */
    grp_usbd_host_data      *ptHcIndex;                 /* Host Controller Data                 */

    /* for configuration and interface control use */
    grp_u32                 ulConfNum;                      /* Current device configuration     */
    grp_usbd_if_info        atIfInfo[GRP_USBD_MAX_IF];      /* Interface Information of current */
                                                            /* device can handle up to 16       */
    grp_usbdi_pipe          *patPipe[2][GRP_USBD_MAX_PIPE]; /* Pipe information(pointer)        */
                                                            /* Area Pointer                     */
    grp_u32                 ulOpenPipeCount;                /* Number of opened pipes excluding */
                                                            /* the default pipe.                */
} grp_usbd_device_table;

/* Internal Data Area */
typedef struct _grp_usbd_cb
{
    grp_vos_t_semaphore     *ptSem[GRP_USBD_MAX_SEMAPHORE];         /* OS Resourse (Semaphore)  */
    grp_usbd_device_table   atDevTable[GRP_USBD_HOST_MAX_DEVICE];   /* Device Table             */
    grp_usbd_host_data      atHcData[GRP_USBC_HOST_QTY_CONTROLLER]; /* Host Controller          */
    grp_u16                 ausDevIdTable[GRP_USBD_HOST_MAX_DEVICE];/* Device ID Table          */

} grp_usbd_cb;

#endif  /* _GRP_USBD_LOCAL_H_ */

