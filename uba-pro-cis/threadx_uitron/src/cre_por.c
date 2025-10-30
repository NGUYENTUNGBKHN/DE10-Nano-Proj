/********************************************************************************/
/*                Copyright (C) 2002-2017 Grape Systems, Inc.                   */
/*                     All Rights Reserved.                                     */
/*                                                                              */
/*  This software is furnished under a license and may be used and copied only  */
/*  in accordance with the terms of such license and with the inclusion of the  */
/*  above copyright notice. No title to and ownership of the software is        */
/*  transferred.                                                                */
/*  Grape Systems Inc. makes no warranties with respect to the performance of   */
/*  this computer program, and specifically disclaims any responsibility for    */
/*  any damages, special or consequential, connected with the use of this       */
/*  program.                                                                    */
/*                                                                              */
/********************************************************************************/

/********************************************************************************/
/*  cre_por.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create rendezvous port                                                      */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_por                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create rendezvous port(Internal function).                              */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      pk_cpor             Pointer to the packet containing the rendezvous port*/
/*                          creation information.                               */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_por                                                                 */
/*      acre_por                                                                */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Internal                                                                */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER __cre_por(ID porid, T_CPOR* pk_cpor)
{
    _KERNEL_UITRON_POR* pPor = &ID_LIST(_kernel_por, porid);

    /* Copy initial data. */
    pPor->cpor.poratr = pk_cpor->poratr;
    pPor->cpor.maxcmsz = pk_cpor->maxcmsz;
    pPor->cpor.maxrmsz = pk_cpor->maxrmsz;

    pPor->ctsk = NULL;
    pPor->atsk = NULL;

    ID_LIST(_kernel_por_id, porid) = pPor;

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_por                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create rendezvous port.                                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      pk_cpor             Pointer to the packet containing the rendezvous port*/
/*                          creation information.                               */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __cre_por           Create rendezvous port(Internal function).          */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Initialization                                                          */
/*      Task context                                                            */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  The preemption control method was arranged. */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER cre_por(ID porid, T_CPOR* pk_cpor)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_por_maxid, porid)
    CHECK_PARAM(pk_cpor!=NULL)
    CHECK_ATR(pk_cpor->poratr, TA_TPRI|TA_TFIFO)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_por_id, porid)==NULL)
    {
        rtn = __cre_por(porid, pk_cpor);
    }
    else
    {
        rtn = E_OBJ;    /* The object is already registered. */
    }

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable--;   /* Release preemption. */
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

