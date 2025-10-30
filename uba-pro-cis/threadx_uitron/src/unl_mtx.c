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
/*  unl_mtx.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Unlock mutex                                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_mutex.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      unl_mtx                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Unlock mutex.                                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      mtxid               ID number of the mutex.                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_ILUSE             Illegal use                                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _tx_mutex_priority_change   Priority change for mutex                   */
/*      _kernel_release_mutex       Release mutex                               */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Task context                                                            */
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
/*                                  Before using ThreadX API, it changed so     */
/*                                  that interruption might be enabled.         */
/*  2012/11/27      H. Tousaki      version 2.2g, Modified comment(s).          */
/*                                  When the current priority was the same as   */
/*                                  the base priority, the thread was rotated   */
/*                                  and the scheduler was not called.           */
/*                                  This problem was corrected.                 */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Renamed some definitions.                   */
/*                                                                              */
/********************************************************************************/
ER unl_mtx(ID mtxid)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MTX* pMtx;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mtx_maxid, mtxid)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pMtx = (_KERNEL_UITRON_MTX*)ID_LIST(_kernel_mtx_id, mtxid);
    if(pMtx!=NULL)
    {
        pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
        if(pTcb!=NULL)
        {
            if(pMtx->lock_tsk==pTcb)
            {
                /* Remove from locked mutex list */
                _kernel_deque(&pMtx->lock);

                _tx_thread_preempt_disable++;   /* Disable preemption. */
                TX_RESTORE

#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
                _kernel_release_mutex(pMtx, pTcb->thread.tx_thread_priority);

                if(pTcb->locked_mutex==NULL && (UINT)pTcb->tskbpri!=pTcb->thread.tx_thread_priority)
                {
                    /* Change to base priority. */
                    UINT old_priority;
                    pTcb->thread.tx_thread_inherit_priority = (UINT)pTcb->tskbpri;

                    TX_DISABLE
                    _tx_thread_preempt_disable--;   /* Release preemption. */
                    if(DISPATCH_ENABLE)
                    {
                        TX_RESTORE
                        _tx_mutex_priority_change(&pTcb->thread, (UINT)pTcb->tskbpri);
                        TX_DISABLE
                    }
                    else
                    {
                        _tx_thread_preempt_disable++;   /* Disable preemption. */
                        TX_RESTORE
                        _tx_thread_priority_change(&pTcb->thread, (UINT)pTcb->tskbpri, &old_priority);
                        TX_DISABLE
                        _tx_thread_preempt_disable--;   /* Release preemption. */
                    }
                }
                else
                {
                    TX_DISABLE
                    _tx_thread_preempt_disable--;   /* Release preemption. */

                    if(DISPATCH_ENABLE)
                    {
                        /* Return to the system so the higher priority thread can be scheduled. */
#ifdef TXI_SMP
                        _tx_thread_preempt_disable++;   /* Disable preemption. */
#endif
                        TX_RESTORE
                        _tx_thread_system_return();
                        TX_DISABLE
                    }
                }
#else
                if(pTcb->locked_mutex==NULL && (UINT)pTcb->tskbpri!=pTcb->thread.tx_thread_priority)
                {
                    /* Change to base priority. */
                    _tx_mutex_priority_change(&pTcb->thread, (UINT)pTcb->tskbpri, (UINT)pTcb->tskbpri);
                }
                _kernel_release_mutex(pMtx, pTcb->thread.tx_thread_priority);

                TX_DISABLE
                _tx_thread_preempt_disable--;   /* Release preemption. */

                if(DISPATCH_ENABLE)
                {
                    /* Return to the system so the higher priority thread can be scheduled. */
#ifdef TXI_SMP
                    _tx_thread_preempt_disable++;   /* Disable preemption. */
#endif
                    TX_RESTORE
                    _tx_thread_system_return();
                    TX_DISABLE
                }
#endif
#else
                if(pTcb->locked_mutex==NULL && (UINT)pTcb->tskbpri!=pTcb->thread.tx_thread_priority)
                {
                    /* Change to base priority. */
                    _tx_mutex_priority_change(&pTcb->thread, (UINT)pTcb->tskbpri, (UINT)pTcb->tskbpri);
                }
                _kernel_release_mutex(pMtx, pTcb->thread.tx_thread_priority);

                TX_DISABLE
                _tx_thread_preempt_disable--;   /* Release preemption. */

                if(DISPATCH_ENABLE)
                {
                    /* Return to the system so the higher priority thread can be scheduled. */
#ifdef TXI_SMP
                    _tx_thread_preempt_disable++;   /* Disable preemption. */
#endif
                    TX_RESTORE
                    _tx_thread_system_return();
                    TX_DISABLE
                }
#endif
            }
            else
            {
                rtn = E_ILUSE;  /* Illegal use */
            }
        }
        else
        {
            rtn = E_CTX;    /* No support contexts. */
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

