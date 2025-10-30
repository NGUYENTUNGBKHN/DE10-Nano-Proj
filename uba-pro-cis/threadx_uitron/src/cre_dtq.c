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
/*  cre_dtq.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create data queue                                                           */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_dtq                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create data queue(Internal function).                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      dtqid               ID number of the data queue.                        */
/*      pk_cdtq             Pointer to the packet containing the data queue     */
/*                          creation information.                               */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_sysmem_alloc        Get system memory                           */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_dtq                                                                 */
/*      acre_dtq                                                                */
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
ER __cre_dtq(ID dtqid, T_CDTQ* pk_cdtq)
{
    _KERNEL_UITRON_DTQ* pDtq = &ID_LIST(_kernel_dtq, dtqid);

    /* Copy T_CDTQ data. */
    pDtq->cdtq.dtqatr = pk_cdtq->dtqatr;
    pDtq->cdtq.dtqcnt = pk_cdtq->dtqcnt;
    pDtq->cdtq.dtq = pk_cdtq->dtq;

    if(pDtq->cdtq.dtq==NULL)
    {
        if((pDtq->dtqptr=(VP_INT*)_kernel_sysmem_alloc(TSZ_DTQ(pk_cdtq->dtqcnt)))==NULL)
        {
            return E_NOMEM; /* Memory none */
        }
    }
    else
    {
        pDtq->dtqptr = (VP_INT*)pDtq->cdtq.dtq;
    }

    pDtq->setpos = 0;
    pDtq->getpos = 0;
    pDtq->dtqcnt = 0;

    pDtq->rtsk = NULL;
    pDtq->stsk = NULL;

    ID_LIST(_kernel_dtq_id, dtqid) = pDtq;

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_dtq                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create data queue.                                                      */
/*                                                                              */
/* INPUT                                                                        */
/*      dtqid               ID number of the data queue.                        */
/*      pk_cdtq             Pointer to the packet containing the data queue     */
/*                          creation information.                               */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __cre_dtq           Create data queue(Internal function).               */
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
ER cre_dtq(ID dtqid, T_CDTQ* pk_cdtq)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_dtq_maxid, dtqid)
    CHECK_PARAM(pk_cdtq!=NULL)
    CHECK_ATR(pk_cdtq->dtqatr, TA_TPRI|TA_TFIFO)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_dtq_id, dtqid)==NULL)
    {
        rtn = __cre_dtq(dtqid, pk_cdtq);
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

