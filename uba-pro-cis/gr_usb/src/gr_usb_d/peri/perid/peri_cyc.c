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
/*      peri_cyc.c                                              1.42        */
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
/*                              Add logging routine.                        */
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
/**** INCLUDE FILES *********************************************************/
#include    "grusbtyp.h"
#include    "peri_cyc.h"

/* Logging routine */
#ifdef GRUSB_CYC_DEBUG
#include    "dbg_mdl.h"
#define     GRCYC_DBG_TRACE(m,n,x,y)    GRDBG_TRACE(m,n,x,y)
#else
#define     GRCYC_DBG_TRACE(m,n,x,y)
#endif

/************************************************************************/
/*  MACRO       : GRLIB_MAC_INC_PTR                                     */
/*                                                                      */
/*  DESCRIPTION : Renewal of a pointer                                  */
/*----------------------------------------------------------------------*/
/*  INPUT       : iPnt                  value of a pointer              */
/*                iMax                  number of elements              */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : updated pointer value                                 */
/************************************************************************/
#define GRLIB_MAC_INC_PTR( iPnt, iMax)  (((iPnt) + 1) % (iMax))

/************************************************************************/
/*  FUNCTION    : GRLIB_Cyclic_Init                                     */
/*                                                                      */
/*  DESCRIPTION : Init CyclicBuffer                                     */
/*----------------------------------------------------------------------*/
/*  INPUT       : ptCycInfo             CyclicBuffer Information        */
/*                  iMax                number of elements              */
/*                  iReadP              read pointer                    */
/*                  iWriteP             write pointer                   */
/*                iNumBuff              Number of CyclicBuffer          */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
VOID GRLIB_Cyclic_Init( GRLIB_CYCLIC_INFO*      ptCycInfo,
                        INT                     iNumBuff)
{
    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x01, 0, 0 );

    /* initialized paramaters */
    ptCycInfo->iMax    = iNumBuff;
    ptCycInfo->iReadP  = 0;
    ptCycInfo->iWriteP = 0;

    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x01, 0, END_FUNC );
}

/************************************************************************/
/*  FUNCTION    : GRLIB_Cyclic_CheckRead                                */
/*                                                                      */
/*  DESCRIPTION : It is confirmed whether there is any data read to     */
/*                Buffer.                                               */
/*----------------------------------------------------------------------*/
/*  INPUT       : ptCycInfo             CyclicBuffer Information        */
/*                  iMax                number of elements              */
/*                  iReadP              read pointer                    */
/*                  iWriteP             write pointer                   */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : GRLIB_NONBUFFER       there is no information to read */
/*                other GRLIB_NONBUFFER next position to read           */
/************************************************************************/
INT GRLIB_Cyclic_CheckRead( GRLIB_CYCLIC_INFO*      ptCycInfo)
{
    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x02, 0, 0 );

    /* check data in buffer */
    if (ptCycInfo->iReadP == ptCycInfo->iWriteP)
    {
        GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x02, 0x01, END_FUNC );

        /* no data */
        return GRLIB_NONBUFFER;
    }
    else
    {
        GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x02, 0x02, END_FUNC );

        /* exist buffer */
        return ptCycInfo->iReadP;
    }
}

/************************************************************************/
/*  FUNCTION    : GRLIB_Cyclic_CheckWrite                               */
/*                                                                      */
/*  DESCRIPTION : It is confirmed whether data can be written in Buffer */
/*----------------------------------------------------------------------*/
/*  INPUT       : ptCycInfo             CyclicBuffer Information        */
/*                  iMax                number of elements              */
/*                  iReadP              read pointer                    */
/*                  iWriteP             write pointer                   */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : GRLIB_NONBUFFER       there is no information to write*/
/*                other GRLIB_NONBUFFER next position to write          */
/************************************************************************/
INT GRLIB_Cyclic_CheckWrite( GRLIB_CYCLIC_INFO*     ptCycInfo)
{
    INT iNextWrtPnt;

    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x03, 0, 0 );

    iNextWrtPnt = GRLIB_MAC_INC_PTR( ptCycInfo->iWriteP, ptCycInfo->iMax);

    /* check data in buffer */
    if(iNextWrtPnt == ptCycInfo->iReadP)
    {
        GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x03, 0x01, END_FUNC );

        /* no buffer */
        return GRLIB_NONBUFFER;
    }
    else
    {
        GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x03, 0x02, END_FUNC );

        /* exist buffer */
        return ptCycInfo->iWriteP;
    }
}

/************************************************************************/
/*  FUNCTION    : GRLIB_Cyclic_IncRead                                  */
/*                                                                      */
/*  DESCRIPTION : ReadPointer is updated.                               */
/*----------------------------------------------------------------------*/
/*  INPUT       : ptCycInfo             CyclicBuffer Information        */
/*                  iMax                number of elements              */
/*                  iReadP              read pointer                    */
/*                  iWriteP             write pointer                   */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
VOID GRLIB_Cyclic_IncRead( GRLIB_CYCLIC_INFO*       ptCycInfo)
{
    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x04, 0, 0 );

    /* next reading position is calculated. */
    ptCycInfo->iReadP = GRLIB_MAC_INC_PTR( ptCycInfo->iReadP, ptCycInfo->iMax);

    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x04, 0, END_FUNC );
}

/************************************************************************/
/*  FUNCTION    : GRLIB_Cyclic_IncWrite                                 */
/*                                                                      */
/*  DESCRIPTION : WritePointer is updated.                              */
/*----------------------------------------------------------------------*/
/*  INPUT       : ptCycInfo             CyclicBuffer Information        */
/*                  iMax                number of elements              */
/*                  iReadP              read pointer                    */
/*                  iWriteP             write pointer                   */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
VOID GRLIB_Cyclic_IncWrite( GRLIB_CYCLIC_INFO*      ptCycInfo)
{
    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x05, 0, 0 );

    /* next writing position is calculated. */
    ptCycInfo->iWriteP = GRLIB_MAC_INC_PTR( ptCycInfo->iWriteP, ptCycInfo->iMax);

    GRCYC_DBG_TRACE( GRDBG_PERI_CYC, 0x05, 0, END_FUNC );
}

