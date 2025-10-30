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
/*      grp_sff8070i_local.h                                                    1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Sff8070i Sub Class.                              */
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

#ifndef _GRP_SFF8070I_LOCAL_H_
#define _GRP_SFF8070I_LOCAL_H_


/**** INTERNAL DATA DEFINES *********************************************************************/
/*--- SFF8070I Command ID ----------------------------------------------------------------------*/
#define SFF8070I_INQUIRY_CMD            0x12                        /* INQUIRY command          */
#define SFF8070I_MODE_SENSE_CMD         0x5A                        /* MODE SENSE command       */
#define SFF8070I_READ10_CMD             0x28                        /* READ(10) command         */
#define SFF8070I_READ12_CMD             0xA8                        /* READ(12) command         */
#define SFF8070I_READ_CAPACITY_CMD      0x25                        /* READ CAPACITY command    */
#define SFF8070I_READ_FORMAT_CAPA_CMD   0x23                        /* READ FORMAT CAPA command */
#define SFF8070I_REQUEST_SENSE_CMD      0x03                        /* REQUEST SENSE command    */
#define SFF8070I_TEST_UNIT_READY_CMD    0x00                        /* TEST UNIT READY command  */
#define SFF8070I_WRITE10_CMD            0x2A                        /* WRITE(10) command        */
#define SFF8070I_WRITE12_CMD            0xAA                        /* WRITE(12) command        */

/*--- SFF8070I Command SIZE --------------------------------------------------------------------*/
#define SFF8070I_INQUIRY_SZ             12  /* 16 */
#define SFF8070I_MODE_SENSE_SZ          12  /* 16 */
#define SFF8070I_READ10_SZ              12  /* 16 */
#define SFF8070I_READ12_SZ              12  /* 16 */
#define SFF8070I_READ_CAPACITY_SZ       12  /* 16 */
#define SFF8070I_READ_FORMAT_CAPA_SZ    12  /* 16 */
#define SFF8070I_REQUEST_SENSE_SZ       12  /* 16 */
#define SFF8070I_TEST_UNIT_READY_SZ     12  /* 16 */
#define SFF8070I_WRITE10_SZ             12  /* 16 */
#define SFF8070I_WRITE12_SZ             12  /* 16 */

/*--- SFF8070I Command Structure ---------------------------------------------------------------*/
/* INQUIRY Command Descriptor Block */
typedef struct  _SFF8070I_INQUIRY_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (12h)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucReserve1[2];                         /* Reserved                 */
    grp_u8                  ucAllocationLen;                        /* Allocation Length        */
    grp_u8                  aucReserve2[7];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_INQUIRY_STR;

/* MODE SENSE Command Descriptor Block */
typedef struct  _SFF8070I_MODE_SENSE_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (5Ah)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  ucPcPage;                               /* PC - Page Code           */
    grp_u8                  aucReserve1[4];                         /* Reserved                 */
    grp_u8                  aucAllocationLen[2];                    /* Allocation Length        */
    grp_u8                  aucReserve2[3];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_MODE_SENSE_STR;

/* READ(10) Command Descriptor Block */
typedef struct  _SFF8070I_READ10_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (28h)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  ucReserve1;                             /* Reserved                 */
    grp_u8                  aucTransferLen[2];                      /* Transfer Length          */
    grp_u8                  aucReserve2[3];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_READ10_STR;

/* READ(12) Command Descriptor Block */
typedef struct  _SFF8070I_READ12_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (A8h)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  aucTransferLen[4];                      /* Transfer Length          */
    grp_u8                  aucReserve[2];                          /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_READ12_STR;

/* READ CAPACITY Command Descriptor Block */
typedef struct  _SFF8070I_READ_CAPACITY_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (25h)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucReserve[10];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_READ_CAPACITY_STR;

/* READ FORMAT CAPA Command Descriptor Block */
typedef struct  _SFF8070I_READ_FORMAT_CAPA_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (23h)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucReserve1[5];                         /* Reserved                 */
    grp_u8                  aucAllocationLen[2];                    /* Allocation length        */
    grp_u8                  aucReserve2[3];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_READ_FORMAT_CAPA_STR;

/* REQUEST SENSE Command Descriptor Block */
typedef struct  _SFF8070I_REQUEST_SENSE_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (03h)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucReserve1[2];                         /* Reserved                 */
    grp_u8                  ucAllocationLen;                        /* Allocation length        */
    grp_u8                  aucReserve2[7];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_REQUEST_SENSE_STR;

/* TEST UNIT READY Command Descriptor Block */
typedef struct  _SFF8070I_TEST_UNIT_READY_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (00h)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucReserve[10];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_TEST_UNIT_READY_STR;

/* WRITE(10) Command Descriptor Block */
typedef struct  SFF8070I_WRITE10_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (2Ah)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  ucReserve1;                             /* Reserved                 */
    grp_u8                  aucTransferLen[2];                      /* Transfer Length          */
    grp_u8                  aucReserve2[3];                         /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_WRITE10_STR;

/* WRITE(12) Command Descriptor Block */
typedef struct  SFF8070I_WRITE12_STR
{
    grp_u8                  ucCommand;                              /* Operation Code (AAh)     */
    grp_u8                  ucLun;                                  /* Logical unit number      */
    grp_u8                  aucLba[4];                              /* Logical Block Address    */
    grp_u8                  aucTransferLen[4];                      /* Transfer Length          */
    grp_u8                  aucReserve[2];                          /* Reserved                 */
    grp_u8                  aucPadd[4];                             /* PAD                      */
} SFF8070I_WRITE12_STR;


#endif /* _GRP_SFF8070I_LOCAL_H_ */
