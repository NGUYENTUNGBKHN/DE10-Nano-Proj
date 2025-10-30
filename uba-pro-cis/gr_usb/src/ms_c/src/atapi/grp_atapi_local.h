/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2008 Grape Systems, Inc.                          */
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
/*      grp_atapi_local.h                                                       1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Atapi Sub Class.                                 */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created new policy initial version                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_ATAPI_LOCAL_H_
#define _GRP_ATAPI_LOCAL_H_


/**** INTERNAL DATA DEFINES *********************************************************************/
/*--- ATAPI Command ID -------------------------------------------------------------------------*/
#define ATAPI_INQUIRY_CMD               0x12                /* INQUIRY command                  */
#define ATAPI_MODE_SENSE06_CMD          0x1A                /* MODE SENSE (6) command           */
#define ATAPI_MODE_SENSE10_CMD          0x5A                /* MODE SENSE (10) command          */
#define ATAPI_READ10_CMD                0x28                /* READ(10) command                 */
#define ATAPI_READ12_CMD                0xA8                /* READ(12) command                 */
#define ATAPI_READ_CAPACITY_CMD         0x25                /* READ CAPACITY command            */
#define ATAPI_READ_FORMAT_CAPA_CMD      0x23                /* READ FORMAT CAPA command         */
#define ATAPI_REQUEST_SENSE_CMD         0x03                /* REQUEST SENSE command            */
#define ATAPI_TEST_UNIT_READY_CMD       0x00                /* TEST UNIT READY command          */
#define ATAPI_WRITE10_CMD               0x2A                /* WRITE(10) command                */
#define ATAPI_WRITE12_CMD               0xAA                /* WRITE(12) command                */

/*--- ATAPI Command SIZE -----------------------------------------------------------------------*/
#define ATAPI_INQUIRY_SZ                6
#define ATAPI_MODE_SENSE06_SZ           6 
#define ATAPI_MODE_SENSE10_SZ           10
#define ATAPI_READ10_SZ                 10
#define ATAPI_READ12_SZ                 12
#define ATAPI_READ_CAPACITY_SZ          10
#define ATAPI_READ_FORMAT_CAPA_SZ       10
#define ATAPI_REQUEST_SENSE_SZ          6
#define ATAPI_TEST_UNIT_READY_SZ        6
#define ATAPI_WRITE10_SZ                10
#define ATAPI_WRITE12_SZ                12
 
/*--- ATAPI Command Structure ------------------------------------------------------------------*/
/* INQUIRY Command Descriptor Block */
typedef struct  _ATAPI_INQUIRY_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (12h)     */
    grp_u8                  ucCmddtEvpd;                            /* CmdDt - EVPD             */
    grp_u8                  ucPageOperation;                        /* Page or Operation Code   */
    grp_u8                  ucReserve;                              /* Reserved                 */
    grp_u8                  ucAllocationLen;                        /* Allocation Length        */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[10];                            /* PAD                      */
} ATAPI_INQUIRY_STR;

/* MODE SENSE (6)  Command Descriptor Block */
typedef struct  _ATAPI_MODE_SENSE06_STR
{
    grp_u8                  ucCommand;                              /* Operation code (1Ah)     */
    grp_u8                  ucDbd;                                  /* Bit3:DBD(0/1)            */
    grp_u8                  ucPcPage;                               /* Bit7-6:Page Control      */
                                                                    /* Bit5-0:Page code         */
    grp_u8                  ucReserved;                             /* Reserved                 */
    grp_u8                  ucAllocationLen;                        /* Allocation Length        */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[6];                             /* Padding                  */
} ATAPI_MODE_SENSE06_STR;

/* MODE SENSE (10)  Command Descriptor Block */
typedef struct  _ATAPI_MODE_SENSE10_STR
{
    grp_u8                  ucCommand;                              /* Operation code (5Ah)     */
    grp_u8                  ucDbd;                                  /* Bit3:DBD(0/1)            */
    grp_u8                  ucPcPage;                               /* Bit7-6:Page Control      */
                                                                    /* Bit5-0:Page code         */
    grp_u8                  aucReserved[4];                         /* Reserved                 */
    grp_u8                  aucAllocationLen[2];                    /* Allocation Length        */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[2];                             /* Padding                  */
} ATAPI_MODE_SENSE10_STR;

/* READ(10) Command Descriptor Block */
typedef struct  _ATAPI_READ10_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (28h)     */
    grp_u8                  ucDpoFuaReladr;                         /* DPO(0) - FUA - RelAdr    */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  ucReserve;                              /* Reserved                 */
    grp_u8                  aucTransferLen[2];                      /* Transfer Length          */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[6];                             /* PAD                      */
} ATAPI_READ10_STR;

/* READ(12) Command Descriptor Block */
typedef struct  _ATAPI_READ12_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (A8h)     */
    grp_u8                  ucDpoFuaReladr;                         /* DPO(0) - FUA - RelAdr    */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  aucTransferLen[4];                      /* Transfer Length          */
    grp_u8                  ucStreaming;                            /* Streaming                */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} ATAPI_READ12_STR;

/* READ CAPACITY Command Descriptor Block */
typedef struct  _ATAPI_READ_CAPACITY_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (25h)     */
    grp_u8                  ucRelAdr;                               /* RelAdr(0)                */
    grp_u8                  aucReserve[6];                          /* Reserved                 */
    grp_u8                  ucPmi;                                  /* PMI(0)                   */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[6];                             /* PAD                      */
} ATAPI_READ_CAPACITY_STR;

/* READ FORMAT CAPA Command Descriptor Block */
typedef struct  _ATAPI_READ_FORMAT_CAPA_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (23h)     */
    grp_u8                  aucReserve[6];                          /* Reserved                 */
    grp_u8                  aucAllocationLen[2];                    /* Allocation length        */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[6];                             /* PAD                      */
} ATAPI_READ_FORMAT_CAPA_STR;

/* REQUEST SENSE Command Descriptor Block */
typedef struct  _ATAPI_REQUEST_SENSE_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (03h)     */
    grp_u8                  aucReserve[3];                          /* Reserved                 */
    grp_u8                  ucAllocationLen;                        /* Allocation length        */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[10];                            /* PAD                      */
} ATAPI_REQUEST_SENSE_STR;

/* TEST UNIT READY Command Descriptor Block */
typedef struct  _ATAPI_TEST_UNIT_READY_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (00h)     */
    grp_u8                  aucReserve[4];                          /* Reserved                 */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[10];                            /* PAD                      */
} ATAPI_TEST_UNIT_READY_STR;

/* WRITE(10) Command Descriptor Block */
typedef struct  _ATAPI_WRITE10_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (2Ah)     */
    grp_u8                  ucDpoFuaEbpReladr;                      /* DPO -FUA -EBP - RelAdr   */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  ucReserve;                              /* Reserved                 */
    grp_u8                  aucTransferLen[2];                      /* Transfer Length          */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[6];                             /* PAD                      */
} ATAPI_WRITE10_STR;

/* WRITE(12) Command Descriptor Block */
typedef struct  _ATAPI_WRITE12_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (AAh)     */
    grp_u8                  ucDpoFuaEbpReladr;                      /* DPO -FUA -EBP - RelAdr   */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  aucTransferLen[4];                      /* Transfer Length          */
    grp_u8                  ucStreaming;                            /* Streaming                */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} ATAPI_WRITE12_STR;

#endif /* _GRP_ATAPI_LOCAL_H_ */
