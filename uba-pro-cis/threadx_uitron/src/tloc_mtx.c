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
/*  tloc_mtx.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Lock mutex with timeout                                                     */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_mutex.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __tloc_mtx                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Lock mutex(Internal function).                                          */
/*                                                                              */
/* INPUT                                                                        */
/*      mtxid               ID number of the mutex.                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_ILUSE             Illegal use                                         */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _tx_mutex_priority_change   Priority change for mutex                   */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      loc_mtx                                                                 */
/*      ploc_mtx                                                                */
/*      tloc_mtx                                                                */
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
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Renamed some definitions.                   */
/*                                                                              */
/********************************************************************************/
ER __tloc_mtx(ID mtxid, TMO tmout)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MTX* pMtx;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pMtx = (_KERNEL_UITRON_MTX*)ID_LIST(_kernel_mtx_id, mtxid);
    if(pMtx!=NULL)
    {
        pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
        if(pTcb!=NULL)
        {
            if(pMtx->lock_tsk==pTcb || (pMtx->cmtx.mtxatr==TA_CEILING &&
                                        (PRI)pTcb->thread.tx_thread_priority<pMtx->cmtx.ceilpri))
            {
                rtn = E_ILUSE;  /* Illegal use */
            }
            else
            {
                if(pMtx->lock_tsk==NULL)
                {
                    /* Set to a locked mutex list of owner task. */
                    pMtx->lock_tsk = pTcb;
                    _kernel_enque(&pTcb->locked_mutex, &pMtx->lock);

                    if(pMtx->cmtx.mtxatr==TA_CEILING && (UINT)pMtx->cmtx.ceilpri<pTcb->thread.tx_thread_priority)
                    {
                        /* Change to ceil priority. */
                        _tx_thread_preempt_disable++;   /* Disable preemption. */
                        TX_RESTORE
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
                        _tx_mutex_priority_change(&pTcb->thread, (UINT)pMtx->cmtx.ceilpri);
#else
                        _tx_mutex_priority_change(&pTcb->thread, (UINT)pMtx->cmtx.ceilpri, (UINT)pMtx->cmtx.ceilpri);
#endif
#else
                        _tx_mutex_priority_change(&pTcb->thread, (UINT)pMtx->cmtx.ceilpri, (UINT)pMtx->cmtx.ceilpri);
#endif
                        TX_DISABLE
                        _tx_thread_preempt_disable--;   /* Release preemption. */
                    }
                }
                else
                {
                    if(tmout!=TMO_POL)
                    {
                        /* Check locked task priority. */
                        if(pMtx->cmtx.mtxatr==TA_INHERIT &&
                            pTcb->thread.tx_thread_priority<pMtx->lock_tsk->thread.tx_thread_priority)
                        {
                            /* Change to inherit priority of locked task priority. */
                            _tx_thread_preempt_disable++;   /* Disable preemption. */
                            TX_RESTORE
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    pTcb->thread.tx_thread_priority);
#else
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    pTcb->thread.tx_thread_priority, pTcb->thread.tx_thread_priority);
#endif
#else
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    pTcb->thread.tx_thread_priority, pTcb->thread.tx_thread_priority);
#endif
                            TX_DISABLE
                            _tx_thread_preempt_disable--;   /* Release preemption. */
                        }

                        if(DISPATCH_ENABLE)
                        {
                            /* Wait for mutex. */
                            if(pMtx->cmtx.mtxatr==TA_TFIFO)
                            {
                                _kernel_enque(&pMtx->wtsk, &pTcb->wait);
                            }
                            else
                            {
                                _kernel_enque_task_pri(&pMtx->wtsk, &pTcb->wait);
                            }

                            pTcb->tskstat = TTS_WAI;
                            pTcb->tskwait = TTW_MTX;
                            pTcb->wobjid = mtxid;
                            pTcb->wait_obj = pMtx;
                            pTcb->wait_pri = (pMtx->cmtx.mtxatr!=TA_TFIFO);

                            pTcb->wait_rtn = E_OK;

                            /* Setup cleanup routine pointer.  */
                            pTcb->thread.tx_thread_suspend_cleanup = _kernel_task_cleanup;

                            /* Set the state to suspended.  */
                            pTcb->thread.tx_thread_state = TX_SUSPENDED;

                            /* Set the suspending flag.  */
                            pTcb->thread.tx_thread_suspending =  TX_TRUE;

                            /* Setup the timeout period.  */
                            if(tmout==TMO_FEVR)
                            {
                                pTcb->thread.tx_thread_timer.tx_timer_internal_remaining_ticks = TX_WAIT_FOREVER;
                            }
                            else
                            {
                                if(0<TIM2TIC(tmout))
                                {
                                    pTcb->thread.tx_thread_timer.tx_timer_internal_remaining_ticks = TIM2TIC(tmout);
                                }
                                else
                                {
                                    pTcb->thread.tx_thread_timer.tx_timer_internal_remaining_ticks = 1;
                                }
                            }

                            _tx_thread_preempt_disable++;   /* Temporarily disable preemption.  */
                            TX_RESTORE  /* Restore interrupts. */

                            _tx_thread_system_suspend(&pTcb->thread);

                            TX_DISABLE  /* Lockout interrupts. */
                            rtn = pTcb->wait_rtn;
                        }
                        else
                        {
                            rtn = E_CTX;    /* No support contexts. */
                        }
                    }
                    else
                    {
                        /* Check locked task priority. */
                        if(pMtx->cmtx.mtxatr==TA_INHERIT &&
                            pTcb->thread.tx_thread_priority<pMtx->lock_tsk->thread.tx_thread_priority)
                        {
                            /* Change to inherit priority of locked task priority. */
                            _tx_thread_preempt_disable++;   /* Disable preemption. */
                            TX_RESTORE
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    pTcb->thread.tx_thread_priority);
#else
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    pTcb->thread.tx_thread_priority, pTcb->thread.tx_thread_priority);
#endif
#else
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    pTcb->thread.tx_thread_priority, pTcb->thread.tx_thread_priority);
#endif
                            TX_DISABLE
                            _tx_thread_preempt_disable--;   /* Release preemption. */
                        }

                        rtn = E_TMOUT;
                    }
                }
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

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      tloc_mtx                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Lock mutex.                                                             */
/*                                                                              */
/* INPUT                                                                        */
/*      mtxid               ID number of the mutex.                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_ILUSE             Illegal use                                         */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __tloc_mtx                  Lock mutex(Internal function)               */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER tloc_mtx(ID mtxid, TMO tmout)
{
    CHECK_ID(1, _kernel_mtx_maxid, mtxid)
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __tloc_mtx(mtxid, tmout);
}

