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
/*      peri_ctl.h                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      Processing to the device request of control transmission            */
/*      is performed.                                                       */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  Y.Sano      2002/12/12  Created initial version 0.01                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
/*  Y.Sato      2003/05/15  It changes Remote Wakeup function               */
/*  K.Takagi    2003/07/16  version 0.92, modified variable names.          */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              version is updated.                         */
/*  K.Handa     2005/03/18  V 1.11                                          */
/*                              below descriptors supported.                */
/*                              - Device Qualifier Descriptor               */
/*                              - Other Speed Configuration Descriptor      */
/*  K.Handa     2005/04/08  V 1.12                                          */
/*                              for supporting test mode                    */
/*  K.Handa     2006/12/27  V 1.20                                          */
/*                              version was updated.                        */
/*  S.Tomizawa  2007/01/24  V 1.30                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/03/12  V 1.40                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/06/12  V 1.41                                          */
/*                              version was updated.                        */
/*  K.Handa     2008/01/21  V 1.42                                          */
/*                              version was updated.                        */
/*                                                                          */
/****************************************************************************/

#ifndef __PERI_CTL_H__
#define __PERI_CTL_H__

#define GRUSB_DEV_SELF_POWER                0x40
#define GRUSB_DEV_RM_WAKEUP                 0x20

/* Feature Selector */
#define GRUSB_DEV_DEVICE_REMOTE_WAKEUP      1
#define GRUSB_DEV_ENDPOINT_HALT             0
#define GRUSB_DEV_TEST_MODE                 2

/* Descriptor Types */
#define GRUSB_DEV_DEVICE                    0x01
#define GRUSB_DEV_CONFIGURATION             0x02
#define GRUSB_DEV_STRING                    0x03
#define GRUSB_DEV_INTERFACE                 0x04
#define GRUSB_DEV_ENDPOINT                  0x05
#define GRUSB_DEV_DEVICE_QUALIFIER          0x06
#define GRUSB_DEV_OTHER_SPEED_CONFIGURATION 0x07
#if 1  /* HID */
#define GRUSB_DEV_HID                       0x21
#define GRUSB_DEV_REPORT                    0x22
#endif /* HID */

/* Notice type of Device Request */
typedef struct device_request_info_tag
{
    UINT8   ucRequestType;                  /* bmRequestType    */
    UINT8   ucRequest;                      /* bRequest         */
    UINT16  usValue;                        /* wValue           */
    UINT16  usIndex;                        /* wIndex           */
    UINT16  usLength;                       /* wLength          */
} DEVICE_REQUEST_INFO;

VOID    GRUSB_DEV_CtrlInit(VOID);
VOID    GRUSB_DEV_CtrlReInit(VOID);
VOID    GRUSB_DEV_CtrlSetCallBack( UINT16, GRUSB_DeviceRequest, GRUSB_NoticeEndPoint,
                                   GRUSB_NoticeEndPoint);
VOID    GRUSB_DEV_CtrlCallbackClearFeature( GRUSB_NoticeEndPoint16);
VOID    GRUSB_DEV_CtrlRcvReq( DEVICE_REQUEST_INFO*);
VOID    GRUSB_DEV_CtrlCallbackEnRmtWkup( GRUSB_Notice16);
VOID    GRUSB_DEV_CtrlRmtWkup(VOID);


VOID    GRUSB_DEV_CtrlInit2(VOID);
VOID    GRUSB_DEV_CtrlReInit2(VOID);
VOID    GRUSB_DEV_CtrlSetCallBack2( UINT16, GRUSB_DeviceRequest, GRUSB_NoticeEndPoint,
                                   GRUSB_NoticeEndPoint);
VOID    GRUSB_DEV_CtrlCallbackClearFeature2( GRUSB_NoticeEndPoint16);
VOID    GRUSB_DEV_CtrlRcvReq2( DEVICE_REQUEST_INFO*);
VOID    GRUSB_DEV_CtrlCallbackEnRmtWkup2( GRUSB_Notice16);
VOID    GRUSB_DEV_CtrlRmtWkup2(VOID);

#endif  /* __PERI_CTL_H__ */
