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
/*  ref_tst.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference task(Simplified version)                                          */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_tst                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference task(Simplified version).                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*      pk_rtst             Pointer to the packet returning the task state.     */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_get_tcb             Get tcb pointer                             */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Task context                                                            */
/*      Non-task context                                                        */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  Current task information is acquired        */
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),          */
/*                  T. Shibuya      Add the status setting process if the task  */
/*                                  has become a waiting state by ThreadX of    */
/*                                  the API.                                    */
/*                                                                              */
/********************************************************************************/
ER ref_tst(ID tskid, T_RTST* pk_rtst)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_PARAM(pk_rtst!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
#ifdef TXI_SMP
        ULONG i;

#undef _tx_thread_current_ptr

        for(i=0; i<TX_THREAD_SMP_MAX_CORES; i++)
        {
            if(&pTcb->thread==_tx_thread_current_ptr[i])
            {
                pk_rtst->tskstat = TTS_RUN;
                break;
            }
        }

        if(TX_THREAD_SMP_MAX_CORES<=i)
        {
            pk_rtst->tskstat = pTcb->tskstat;
        }
#else
        if(&pTcb->thread==_tx_thread_current_ptr)
        {
            pk_rtst->tskstat = TTS_RUN;
        }
        else
        {
            pk_rtst->tskstat = pTcb->tskstat;
        }
#endif

        if((pTcb->tskstat==TTS_WAI || pTcb->tskstat==TTS_WAS) && (pTcb->thread.tx_thread_state==TX_SUSPENDED))
        {
            pk_rtst->tskwait = pTcb->tskwait;
        }
        else
        {
            pk_rtst->tskwait = TTW_NONE;
        }

        /* check of ThreadX status */
        if(pk_rtst->tskstat==TTS_RUN || pk_rtst->tskstat==TTS_RDY)
        {
            if(TX_SLEEP<=pTcb->thread.tx_thread_state)
            {
                if(pTcb->thread.tx_thread_delayed_suspend==TX_FALSE)
                {
                    pk_rtst->tskstat = TTS_WAI;
                }
                else
                {
                    pk_rtst->tskstat = TTS_WAS;
                }
            }
            else if(pTcb->thread.tx_thread_state==TX_SUSPENDED)
            {
                pk_rtst->tskstat = TTS_SUS;
            }
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

