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
/*      peri_sts.h                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      Status Control Module                                               */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  S.Baba      2002/12/09  Created initial version 0.00                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
/*  K.Takagi    2003/07/16  version 0.92, modified variable names.          */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              modified following function                 */
/*                              - GRUSB_DEV_StateInit                       */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              old debuging routine is deleted.            */
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
#ifndef _PERI_STS_H_
#define _PERI_STS_H_

#include    "grusbtyp.h"
#include    "perid.h"

VOID            GRUSB_DEV_StateInit( GRUSB_Notice );
VOID            GRUSB_DEV_StateIdle( VOID );
VOID            GRUSB_DEV_StateReset( VOID );
VOID            GRUSB_DEV_StateSetAddress( INT );
VOID            GRUSB_DEV_StateSetConfigure( INT );
VOID            GRUSB_DEV_StateSuspend( BOOLEAN );
GRUSB_DEV_STATE GRUSB_DEV_StateGetDeviceState( VOID );
VOID            GRUSB_DEV_StateSetStall( INT , BOOLEAN );
BOOLEAN         GRUSB_DEV_StateGetStall( INT );
INT             GRUSB_DEV_StateGetInterface( VOID );
VOID            GRUSB_DEV_StateSetInterface( INT );


VOID            GRUSB_DEV_StateInit2( GRUSB_Notice );
VOID            GRUSB_DEV_StateIdle2( VOID );
VOID            GRUSB_DEV_StateReset2( VOID );
VOID            GRUSB_DEV_StateSetAddress2( INT );
VOID            GRUSB_DEV_StateSetConfigure2( INT );
VOID            GRUSB_DEV_StateSuspend2( BOOLEAN );
GRUSB_DEV_STATE GRUSB_DEV_StateGetDeviceState2( VOID );
VOID            GRUSB_DEV_StateSetStall2( INT , BOOLEAN );
BOOLEAN         GRUSB_DEV_StateGetStall2( INT );
INT             GRUSB_DEV_StateGetInterface2( VOID );
VOID            GRUSB_DEV_StateSetInterface2( INT );

#endif /* _PERI_STS_H_ */
