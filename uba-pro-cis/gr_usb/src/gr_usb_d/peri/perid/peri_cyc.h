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
/*      peri_cyc.h                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      Cyclic Buffer FUNCTION                                              */
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
/*                              version is updated.                         */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              version is updated.                         */
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
#ifndef _PERI_CYC_H_
#define _PERI_CYC_H_

#include    "grusbtyp.h"

#define     GRLIB_NONBUFFER -1      /* The reading and writing to a buffer are impossible. */

typedef struct grlib_cyclic_info_tag {
    INT iMax;                       /* The total number of elements + 1 */
    INT iReadP;                     /* Read Pointer                     */
    INT iWriteP;                    /* Write Pointer                    */
} GRLIB_CYCLIC_INFO;

VOID    GRLIB_Cyclic_Init(GRLIB_CYCLIC_INFO* , INT );
INT     GRLIB_Cyclic_CheckRead(GRLIB_CYCLIC_INFO* );
INT     GRLIB_Cyclic_CheckWrite(GRLIB_CYCLIC_INFO* );
VOID    GRLIB_Cyclic_IncRead(GRLIB_CYCLIC_INFO* );
VOID    GRLIB_Cyclic_IncWrite(GRLIB_CYCLIC_INFO* );

VOID    GRLIB_Cyclic_Init2(GRLIB_CYCLIC_INFO* , INT );
INT     GRLIB_Cyclic_CheckRead2(GRLIB_CYCLIC_INFO* );
INT     GRLIB_Cyclic_CheckWrite2(GRLIB_CYCLIC_INFO* );
VOID    GRLIB_Cyclic_IncRead2(GRLIB_CYCLIC_INFO* );
VOID    GRLIB_Cyclic_IncWrite2(GRLIB_CYCLIC_INFO* );

#endif  /* _PERI_CYC_H_ */
