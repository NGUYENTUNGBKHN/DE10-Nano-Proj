/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2008 Grape Systems, Inc.                          */
/*                                     All Rights Reserved.                                     */
/*                                                                                              */
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
/*      grp_hcd.h                                                               1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# host controller interface's header file                                    */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created initial version based on the V1.06                      */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Added "grp_usbc_cfg.h" header file.                             */
/*                            - Corrected wrong comments.                                       */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_HCD_H_
#define _GRP_HCD_H_

#include "grp_usbc_cfg.h"

/**** INTERNAL DATA DEFINES *********************************************************************/
/* Return error codes */
#define GRP_HCDI_OK                     0                   /* success                          */

#define GRP_HCDI_ERROR                  GRP_RET_ERROR_BASE(GRP_HCM_MDL_ID)
#define GRP_HCDI_INVALIED_PMTR          GRP_HCDI_ERROR-1    /* Invalied parameter               */
#define GRP_HCDI_NO_FUNCTION            GRP_HCDI_ERROR-2    /* no fucntion                      */
#define GRP_HCDI_ILLEGAL_ERROR          GRP_HCDI_ERROR-3    /* Illegal error                    */
#define GRP_HCDI_HC_HALTED              GRP_HCDI_ERROR-4    /* Host Controller Halted           */
#define GRP_HCDI_GLOBAL_SUSPEND         GRP_HCDI_ERROR-5    /* Global Suspned status            */
#define GRP_HCDI_CANNOT_CANCEL          GRP_HCDI_ERROR-6    /* Could not cancel                 */
#define GRP_HCDI_PORT_SUSPEND           GRP_HCDI_ERROR-7    /* Port suspended                   */
#define GRP_HCDI_RESOURCE_SHORT         GRP_HCDI_ERROR-8    /* Resource short error             */

/* structure */

/* Isochronous transaction unit status */
typedef struct
{
    grp_s32                 lStatus;                /* status of each uframe transaction         */
    grp_u32                 ulLength;               /* actual length of each uframe transaction  */
    grp_u8                  *pucBuffer;             /* buffer address of each uframe transaction */
} grp_hcdi_iso_status;

/* Host controller handler */
typedef struct
{
    grp_u16                 usHcIndexNum;                   /* Host controller index number     */
    grp_u8                  aucPadding[2];                  /* padding                          */

} grp_hcdi_hc_hdr;

/* Frame control */
typedef struct
{
    grp_s32                 lFrameNum;                      /* Frame number                     */
    grp_s32                 lFrameWidth;                    /* Frame width                      */
    grp_hcdi_hc_hdr         tHcHdr;                         /* Host controller handler          */

} grp_hcdi_frame_control;

/* Endpoint information */
typedef struct
{
    grp_u8                  ucAddress;                      /* USB device address                */
    grp_u8                  ucEpNum;                        /* Endpoint number                   */
    grp_u8                  ucTxMode;                       /* Endpoint data transfer mode       */
    grp_u8                  ucTxDir;                        /* End point data transfer direction */
    grp_u8                  ucTxInterval;                   /* Polling intervals at interrupt    */
    grp_u8                  ucTxSpeed;                      /* Low/Full/High Speed               */
    grp_u16                 usMaxPacketSize;                /* Endpoint Max. packet size         */
    grp_u8                  ucMicroPerPkt;                  /* High speed interrupt,isochronous  */
                                                            /* frame per packet number           */
    grp_u8                  ucNakRate;                      /* High speed control,bulk NAK rate  */
    grp_u8                  ucHubAddress;                   /* Hub address                       */
    grp_u8                  ucPortNum;                      /* Hub port number                   */
    grp_hcdi_hc_hdr         tHcHdr;                         /* Host controller handler           */
    void                    *pvHcdworkPtr;                  /* HCD Work Pointer                  */
/*  void                    *pHcdExtraPtr; */               /* HCD Extra Pointer                 */
#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
    grp_u8                  ucIsoStartMicroFrm;             /* When it is not one transaction to */
                                                            /* a microframe, specified beginning */
                                                            /* microframe in milliframe.         */
#endif /* GRP_USB_HOST_USE_ISOCHRONOUS */

} grp_hcdi_endpoint;

/* Request structure for host controller driver */
typedef struct grp_hcdi_tr_request grp_hcdi_tr_request;
struct grp_hcdi_tr_request
{
    grp_hcdi_endpoint       *ptEndpoint;            /* Endpoint for request                     */
    grp_u8                  *pucBufferPtr;          /* Start buffer at request                  */
    grp_u8                  *pucSetupPtr;           /* Packet at SETUP buffer                   */
    grp_u32                 ulBufferLength;         /* Buffer length                            */
    grp_u32                 ulActualLength;         /* Data length actually sent/received       */
    void                    (*pfnCompFunc)(grp_hcdi_tr_request *);
                                                    /* Completion Routine                       */
    grp_si                  iRefCon;                /* General-purpose value passed             */
                                                    /* back to the completion routine           */
    grp_si                  iShortXferOK;           /* Flag to show if Short Packet             */
                                                    /* is error or not                          */
                                                    /*  1: HCDI_TRUE                            */
                                                    /*     Short Packet not error               */
                                                    /*  0: HCDI_FALSE                           */
                                                    /*     Short Packet is error                */
    grp_s32                 lStatus;                /* Current status for the request           */
                                                    /*  0: Not process yet                      */
                                                    /*  1: No error                             */
                                                    /*  2: Cancel(The request is                */
                                                    /*     aborted due to cancel request from   */
                                                    /*     USBD.)                               */
                                                    /****************** Errors ******************/
                                                    /* -1: Time out error                       */
                                                    /* -2: Data overrun                         */
                                                    /* -3: Data underrun                        */
                                                    /* -4: CRC error                            */
                                                    /* -5: Invalid PID                          */
                                                    /* -6: False EOP                            */
                                                    /* -7: Bit stuffing error                   */
                                                    /* -8: STALL                                */
                                                    /* -9: Buffer overrun                       */
                                                    /* -10:Buffer underrun                      */
                                                    /* -11:Host Controller is Halted            */
    grp_u32                 ulXferinfo;             /* Send direction                           */
    void                    *pvUrbPtr;              /* URB Pointer                              */
    void                    *pvHcdworkPtr;          /* Host Controller Driver Work Pointer      */
/*  void                    *pHcdExtraPtr; */       /* Host Controller Driver Extra Pointer     */
    grp_hcdi_hc_hdr         tHcHdr;                 /* Host Controller Handler                  */

#ifdef GRP_USB_HOST_USE_ISOCHRONOUS
    grp_u32                 ulErrorCnt;             /* Number of error at Isochronous transfer  */
    grp_si                  iImmediate;             /* Isochronous transfer start option flag   */
                                                    /*  1: TRUE                                 */
                                                    /*     Start Isochronous transfer           */
                                                    /*     immediately regardless of "frameNo"  */
                                                    /*  0: FALSE                                */
                                                    /*     Start Isochronous transfer           */
                                                    /*     specified by "frameNo"               */
    grp_u16                 usFrameNo;              /* Isochronous transfer start Frame Number  */
    grp_u8                  aucPadding[2];          /* padding                                  */
    grp_hcdi_iso_status     *ptIsoStatus;
    grp_u32                 ulIsoNum;
    grp_u32                 ulIsoIndexM;            /* The index of status array for making iTD */
    grp_u32                 ulIsoIndexC;            /* The index of status array for complete   */
                                                    /* Status information only for isochronous  */
    grp_u32                 ulIsoMode;              /* Isochronous buffer mode                  */
#endif  /* GRP_USB_HOST_USE_ISOCHRONOUS */
};

/* Port Handle */
typedef struct
{
    grp_u8                  ucPort;                     /* Port Number                          */
    grp_u8                  aucPadding[3];              /* Padding                              */
    grp_hcdi_hc_hdr         tHcHdr;                     /* Host controller handler              */

} grp_hcdi_port_hdr;

/* Port Control Structure */
typedef struct
{
    grp_hcdi_port_hdr
                            tPortHdr;           /* Port Handle                                  */
    grp_si                  iPortReq;           /* Port Request                                 */
                                                /* GRP_USBD_PORT_SUSPEND_CONTROL: Port Suspend  */
                                                /* GRP_USBD_PORT_RESUME_CONTROL : Port Resume   */
                                                /* GRP_USBD_PORT_RESET_CONTROL  : Port Reset    */
    void                    *pvReqPrm;          /* Request Parameter                            */
} grp_hcdi_port_control;

/* USB driver access functions */
typedef struct _grp_hcdi_request
{
    /* Controller control functions */
    grp_s32 (*pfnHcReset)(grp_hcdi_hc_hdr *);           /* Host controller reset function       */
    grp_s32 (*pfnHcGlobalSuspend)(grp_hcdi_hc_hdr *);   /* Global Suspend request function      */
    grp_s32 (*pfnHcGlobalResume)(grp_hcdi_hc_hdr *);    /* Global resume request function       */
    grp_s32 (*pfnHcSetSof)(grp_hcdi_frame_control *);   /* Host controller SOF frame number     */
                                                        /* set function                         */
    grp_s32 (*pfnHcGetSof)(grp_hcdi_frame_control *);   /* Host controller SOF frame number     */
                                                        /* get function                         */
    grp_s32 (*pfnHcSetFrame)(grp_hcdi_frame_control *); /* Host controller frame width set      */
                                                        /* function                             */
    grp_s32 (*pfnHcGetFrame)(grp_hcdi_frame_control *); /* Host controller frame width get      */
                                                        /* function                             */
    /* Endpoint control functions */
    grp_s32 (*pfnHcEpOpen)(grp_hcdi_endpoint *);    /* Host controller Endpoint open function   */
    grp_s32 (*pfnHcEpClose)(grp_hcdi_endpoint *);   /* Host controller Endpoint close function  */
    grp_s32 (*pfnHcEpActive)(grp_hcdi_endpoint *);  /* Host controller Endpoint active functions*/
    grp_s32 (*pfnHcEpHalt)(grp_hcdi_endpoint *);    /* Host controller Endpoint halt function   */
    grp_s32 (*pfnHcEpFlash)(grp_hcdi_endpoint *);   /* Host controller Endpoint flash function  */

    /* Communication functions(Normal) */
    grp_s32 (*pfnHcTrRun)(grp_hcdi_tr_request *);   /* Host controller Communication data set   */
                                                    /* + Send function                          */
    grp_s32 (*pfnHcTrDel)(grp_hcdi_tr_request *);   /* Host controller Communication data delete*/
                                                    /* function                                 */

    /* Communication functins(Isochronous) */
    grp_s32 (*pfnHcItrRun)(grp_hcdi_tr_request *);  /* Host controller Communicatin data set    */
                                                    /* + Send function                          */
    grp_s32 (*pfnHcItrDel)(grp_hcdi_tr_request*);   /* Host controller Communication dta delete */
                                                    /* function                                 */

    /* Port Control Function */
    grp_s32 (*pfnHcPortControl)(grp_hcdi_port_control *);
                                                    /* Host controller roothub control function */
} grp_hcdi_request;

/* Host controller interface from upper driver for initialization functions */
typedef struct
{
    grp_hcdi_request        *ptHcIoFunc;            /* Host controller I/O function             */
    grp_u16                 usHcIndexNum;           /* Host controller index number             */
    grp_u8                  aucPadding[2];          /* Padding (2byte)                          */
    grp_u32                 ulHostDelay;            /* Host controller delay time               */
    grp_u32                 ulLsSetup;              /* Host controller low speed setup time     */
    grp_si                  iStatus;                /* Initialize status                        */
    void                    *pvHcdworkPtr;          /* Host Controller Driver Working Area      */

} grp_hcdi_system_init;


/* prototypes */
EXTERN grp_s32 grp_hcm_Init( grp_hcdi_system_init* ptHcdInfo);

#endif  /* _GRP_HCD_H_ */
