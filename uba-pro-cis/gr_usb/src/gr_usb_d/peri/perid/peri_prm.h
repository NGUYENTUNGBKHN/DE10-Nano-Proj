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
/*      peri_prm.h                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This module performs parameter management.                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  Y.Sano      2002/07/24  Created initial version 0.01                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
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
/*  K.Handa     2005/03/18  V 1.12                                          */
/*                              hybrid version (full/high-speed)            */
/*  K.Handa     2005/03/18  V 1.13                                          */
/*                              for override maxpower                       */
/*  K.Handa     2005/11/10  V 1.14                                          */
/*                              chaged default value GRUSB_PRM_ORCDBS       */
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
#ifndef __PERI_PRM_H__
#define __PERI_PRM_H__

#include    "peri_cyc.h"

/* Bus information */
#define GRUSB_PRM_BUS_FS        0x00            /* full-speed               */
#define GRUSB_PRM_BUS_HS        0x01            /* high-speed               */

/* Override Configuration Descriptor Buffer Size */
#define GRUSB_PRM_ORCDBS        0x0100          /* V 1.14 */

/* Transmitting information structure */
typedef struct  grusb_sndinfo_tag
{
    BOOLEAN                     iFlag;          /* It is a flag during transmission.            */
    GRUSB_NoticeEndPoint        pfnFunc;        /* The CallBack function notified after the     */
                                                /* completion of transmitting Data transmitting */
    UINT32                      ulSndSize;      /* Data Size which transmitted                  */
} GRUSB_SndInfo;

/* Receiving information structure */
typedef struct  grusb_rcvinfo_tag
{
    UINT32                      ulRcvSize;      /* Data Size which received */
} GRUSB_RcvInfo;

/* Transmitting Data information structure */
typedef struct  grusb_snddatainfo_tag
{
    UINT32                      ulDataSz;       /* Transmission Data Size                       */
    UINT8*                      pucBuf;         /* Pointer to Transmission Data                 */
    BOOLEAN                     i0Len;          /* 0-Length addition setup                      */
    VOID*                       pAplInfo;       /* Application Data notified after transmitting */
                                                /* Data storing                                 */
    GRUSB_DataPushEnd           pfnFunc;        /* The CallBack function notified after         */
                                                /* transmitting Data storing                    */
} GRUSB_SndDataInfo;

/* Receiving Data information structure */
typedef struct  grusb_rcvdatainfo_tag
{
    UINT32                      ulRcvBufferSz;  /* Reception Buffer Size                        */
    UINT8*                      pucBuf;         /* Pointer to Reception Buffer                  */
    VOID*                       pAplInfo;       /* Application Data notified after receiving    */
                                                /* Data storing                                 */
    GRUSB_DataReceiveEnd        pfnFunc;        /* The CallBack function notified after         */
                                                /* receiving Data storing                       */
} GRUSB_RcvDataInfo;

/* Transmission information structure */
typedef union   grusb_TransmitInfo_tag
{
    GRUSB_SndDataInfo           tSndDtInfo;     /* Transmitting Data information */
    GRUSB_RcvDataInfo           tRcvDtInfo;     /* Receiving Data information    */
} GRUSB_TransmitInfo;

/* Endpoint information structure */
typedef struct  grusb_endpointinfo_tag
{
    INT                         iEpNo;          /* endpoint number                  */
    GRUSB_DEV_EP_TYPE           ucEpType;       /* endpoint type                    */
    UINT16                      usMaxPktSz[2];  /* FIFO Max Packet Size [0]FS [1]HS */
    UINT8                       ucPages;        /* FIFO Page number                 */
    GRLIB_CYCLIC_INFO           tCycBufInfo;    /* Cyclic Buffer informaiton        */
    GRUSB_TransmitInfo*         ptTrnsInfo;     /* pointer of the structure         */
    GRUSB_TransmitInfo          tCrntTrnsInfo;  /* current sending informaiton      */
    GRUSB_NoticeEndPoint16      pfnTrnsErrFnc;  /* Notice CallBack function of      */
                                                /* a transmission error             */
    union
    {
        GRUSB_SndInfo           tSndInfo;       /* Transmitting information structure */
        GRUSB_RcvInfo           tRcvInfo;       /* Receiving information structure    */
    } uInfo;
} GRUSB_EndPointInfo;

INT                 GRUSB_Prm_Init( GRUSB_DEV_INITINFO * );
VOID                GRUSB_Prm_ReInit(VOID);
BOOLEAN             GRUSB_Prm_SetSntFunc( INT, GRUSB_NoticeEndPoint );
GRUSB_EndPointInfo* GRUSB_Prm_GetEndPointInfo( INT );
BOOLEAN             GRUSB_Prm_GetFIFOInfo( INT, UINT8*, UINT16* );
BOOLEAN             GRUSB_Prm_GetEndPointType( INT, GRUSB_DEV_EP_TYPE* );
UINT8*              GRUSB_Prm_GetDeviceDescriptor( UINT16 * );
UINT8*              GRUSB_Prm_GetStringDescriptor( UINT8, UINT16* );
UINT8*              GRUSB_Prm_GetConfigurationDescriptor( UINT8, UINT16* );
BOOLEAN             GRUSB_Prm_GetInterface( UINT16, UINT8* );
BOOLEAN             GRUSB_Prm_SetInterface( UINT16, UINT16 );
UINT8*              GRUSB_Prm_GetDeviceQualifierDescriptor( UINT16* );
UINT8*              GRUSB_Prm_GetOtherSpeedConfigurationDescriptor( UINT8, UINT16* );
VOID                GRUSB_Prm_SetBusSpeed( BOOLEAN );
UINT8*              GRUSB_Prm_GetConfigurationDescriptorOverride( UINT8*, UINT16 );


INT                 GRUSB_Prm_Init2( GRUSB_DEV_INITINFO * );
VOID                GRUSB_Prm_ReInit2(VOID);
BOOLEAN             GRUSB_Prm_SetSntFunc2( INT, GRUSB_NoticeEndPoint );
GRUSB_EndPointInfo* GRUSB_Prm_GetEndPointInfo2( INT );
BOOLEAN             GRUSB_Prm_GetFIFOInfo2( INT, UINT8*, UINT16* );
BOOLEAN             GRUSB_Prm_GetEndPointType2( INT, GRUSB_DEV_EP_TYPE* );
UINT8*              GRUSB_Prm_GetDeviceDescriptor2( UINT16 * );
UINT8*              GRUSB_Prm_GetStringDescriptor2( UINT8, UINT16* );
UINT8*              GRUSB_Prm_GetConfigurationDescriptor2( UINT8, UINT16* );
BOOLEAN             GRUSB_Prm_GetInterface2( UINT16, UINT8* );
BOOLEAN             GRUSB_Prm_SetInterface2( UINT16, UINT16 );
UINT8*              GRUSB_Prm_GetDeviceQualifierDescriptor2( UINT16* );
UINT8*              GRUSB_Prm_GetOtherSpeedConfigurationDescriptor2( UINT8, UINT16* );
VOID                GRUSB_Prm_SetBusSpeed2( BOOLEAN );
UINT8*              GRUSB_Prm_GetConfigurationDescriptorOverride2( UINT8*, UINT16 );
#if 1  /* HID */
UINT8*              GRUSB_Prm_GetReportDescriptor2( UINT8, UINT16 * );
#endif /* HID */

#endif  /* __PERI_PRM_H__ */
