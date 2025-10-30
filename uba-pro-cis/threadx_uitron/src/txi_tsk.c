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
/*  txi_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Task entry function                                                         */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_mutex.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_task_entry                                                      */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Entry function of the task.                                             */
/*                                                                              */
/* INPUT                                                                        */
/*      thread_input                                                            */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      User task entry                                                         */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Thread handler                                                          */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  Before using ThreadX API, it changed so     */
/*                                  that interruption might be enabled.         */
/*  2013/12/05      M. Miyashita    version 2.2m, Modified comment(s),          */
/*                                  At the time of an end of task, a problem may*/
/*                                  arise by interruption.                      */
/*                                  The TX_DISABLE was added.                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
VOID _kernel_task_entry(ULONG thread_input)
{
    _KERNEL_UITRON_TSK* pTcb = (_KERNEL_UITRON_TSK*)thread_input;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pTcb->tskstat = TTS_RDY;
    TX_RESTORE  /* Restore interrupts. */

    ((void (*)(VP_INT))pTcb->ctsk.task)(pTcb->stacd);

    TX_DISABLE  /* Lockout interrupts. */
    if(DISPATCH_ENABLE)
    {
        TX_RESTORE  /* Restore interrupts. */

        _kernel_system_request(_kernel_system_task_terminate, pTcb);
    }

    TXI_SYSTEM_ERROR
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_system_task_terminate                                           */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Terminate task from system thread.                                      */
/*                                                                              */
/* INPUT                                                                        */
/*      param               Pointer of task control block                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_system_task_reset   Reset control block                         */
/*      _tx_thread_reset            Reset control block                         */
/*      _tx_thread_resume           Resume thread                               */
/*                                                                              */
/* CALLED BY                                                                    */
/*      System task                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      System task                                                             */
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
void _kernel_system_task_terminate(void* param)
{
    _KERNEL_UITRON_TSK* pTcb = param;

    _kernel_system_task_reset(pTcb);

    if(pTcb->tskstat==TTS_RDY)
    {
        /* Restart this task */
        _tx_thread_reset(&pTcb->thread);
        _tx_thread_resume(&pTcb->thread);
    }
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_system_task_reset                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      This function resets a task control block.                              */
/*                                                                              */
/* INPUT                                                                        */
/*      pTcb                Task TCB for reset                                  */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_release_mutex       Release mutex                               */
/*      _tx_mutex_priority_change   Priority change for mutex                   */
/*      _tx_thread_terminate        Terminate thread                            */
/*                                                                              */
/* CALLED BY                                                                    */
/*      _kernel_system_task_delete                                              */
/*      _kernel_system_task_terminate                                           */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      System task                                                             */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2007/12/03      M. Miyashita    version 2.1a, Modified comment(s),          */
/*                                  It changed so that task exception handler   */
/*                                  might not be cleared.                       */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2013/12/04      M. Miyashita    version 2.2l, Modified comment(s),          */
/*                                  At the time of an end of task, a problem may*/
/*                                  arise by interruption.                      */
/*                                  The TX_DISABLE was added.                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Renamed some definitions.                   */
/*                                                                              */
/********************************************************************************/
void _kernel_system_task_reset(_KERNEL_UITRON_TSK* pTcb)
{
    _KERNEL_UITRON_MTX* pMtx;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pTcb->tskbpri = pTcb->ctsk.itskpri;

    if(0<pTcb->actcnt)
    {
        pTcb->actcnt--;
        pTcb->stacd = pTcb->ctsk.exinf;

        /* This task status is set to ready. */
        pTcb->tskstat = TTS_RDY;
    }
    else
    {
        /* This task status is set to dormant. */
        pTcb->tskstat = TTS_DMT;
    }

    pTcb->wupcnt = 0;
    pTcb->suscnt = 0;

    pTcb->tskwait = TTW_NONE;
    pTcb->wobjid = 0;
    pTcb->wait_obj = NULL;
    pTcb->wait_pri = FALSE;
    pTcb->wait_rtn = E_OK;
    pTcb->waitdata = NULL;

    /* Remove from waiting list */
    _kernel_deque(&pTcb->wait);

    pTcb->texstat = TTEX_DIS;
    pTcb->rasptn = 0;

    if(pTcb->tx_stack_ptr!=NULL)
    {
        /* Recover old stack pointer. */
        pTcb->thread.tx_thread_stack_ptr = pTcb->tx_stack_ptr;
        pTcb->thread.tx_thread_stack_start = pTcb->tx_stack_start;
        pTcb->thread.tx_thread_stack_end = pTcb->tx_stack_end;

        pTcb->tx_stack_ptr = NULL;
    }

    _tx_thread_preempt_disable++;   /* Disable preemption. */

    while(pTcb->locked_mutex!=NULL)
    {
        /* Remove from locked mutex list */
        pMtx = (_KERNEL_UITRON_MTX*)pTcb->locked_mutex->data;
        _kernel_deque(&pMtx->lock);

        TX_RESTORE  /* Restore interrupts. */
        _kernel_release_mutex(pMtx, pTcb->thread.tx_thread_priority);
        TX_DISABLE  /* Lockout interrupts. */
    }

    /* Change to initial priority. */
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
/*    _tx_mutex_priority_change(&pTcb->thread, (UINT)pTcb->ctsk.itskpri); */
    {
        UINT old_priority;
        pTcb->thread.tx_thread_inherit_priority = (UINT)pTcb->tskbpri;
        _tx_thread_priority_change(&pTcb->thread, (UINT)pTcb->ctsk.itskpri, &old_priority);
    }
#else
    _tx_mutex_priority_change(&pTcb->thread, (UINT)pTcb->ctsk.itskpri, (UINT)pTcb->ctsk.itskpri);
#endif
#else
    _tx_mutex_priority_change(&pTcb->thread, (UINT)pTcb->ctsk.itskpri, (UINT)pTcb->ctsk.itskpri);
#endif

    _tx_thread_terminate(&pTcb->thread);

    TXI_TSK_TARGET_DEFINE_INIT

    _tx_thread_preempt_disable--;   /* Release preemption. */

    TX_RESTORE  /* Restore interrupts. */
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_release_mutex                                                   */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      This function actually releases mutex.                                  */
/*                                                                              */
/* INPUT                                                                        */
/*      pMtx                Mutex control block pointer for release             */
/*      priority            The priority inherited when an attribute is         */
/*                          TA_INHERIT.                                         */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_mutex_priority_change   Priority change for mutex                   */
/*      _kernel_resume_task         Resume a task with error code               */
/*                                                                              */
/* CALLED BY                                                                    */
/*      _kernel_system_task_reset                                               */
/*      unl_mtx                                                                 */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  The preemption control method was arranged. */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Renamed some definitions, add the process   */
/*                                  to clear the pTcb->wait_pri.                */
/*                                                                              */
/********************************************************************************/
void _kernel_release_mutex(_KERNEL_UITRON_MTX* pMtx, UINT priority)
{
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    if(pMtx->wtsk!=NULL)
    {
        /* Remove from waiting list */
        pTcb = (_KERNEL_UITRON_TSK*)pMtx->wtsk->data;
        _kernel_deque(&pTcb->wait);

        /* Set to a locked mutex list of owner task. */
        pMtx->lock_tsk = pTcb;
        _kernel_enque(&pTcb->locked_mutex, &pMtx->lock);

        if(pMtx->cmtx.mtxatr==TA_CEILING && (UINT)pMtx->cmtx.ceilpri<pTcb->thread.tx_thread_priority)
        {
            /* Change to ceil priority. */
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
            _tx_mutex_priority_change(&pTcb->thread, (UINT)pMtx->cmtx.ceilpri);
#else
            _tx_mutex_priority_change(&pTcb->thread, (UINT)pMtx->cmtx.ceilpri, (UINT)pMtx->cmtx.ceilpri);
#endif
#else
            _tx_mutex_priority_change(&pTcb->thread, (UINT)pMtx->cmtx.ceilpri, (UINT)pMtx->cmtx.ceilpri);
#endif
        }
        else if(pMtx->cmtx.mtxatr==TA_INHERIT && priority<pTcb->thread.tx_thread_priority)
        {
            /* Change to inherit priority. */
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
            _tx_mutex_priority_change(&pTcb->thread, priority);
#else
            _tx_mutex_priority_change(&pTcb->thread, priority, priority);
#endif
#else
            _tx_mutex_priority_change(&pTcb->thread, priority, priority);
#endif
        }

        pTcb->wait_rtn = E_OK;
        pTcb->wait_obj = NULL;
        pTcb->wait_pri = FALSE;

        /* Clear cleanup routine pointer. */
        pTcb->thread.tx_thread_suspend_cleanup = TX_NULL;

        if(pTcb->tskstat==TTS_WAS)
        {
            pTcb->wobjid = 0;
            pTcb->tskwait = TTW_NONE;
            pTcb->tskstat = TTS_SUS;
        }
        else
        {
            pTcb->wobjid = 0;
            pTcb->tskwait = TTW_NONE;
            pTcb->tskstat = TTS_RDY;

            _tx_thread_preempt_disable++;   /* Temporarily disable preemption. */
            TX_RESTORE  /* Restore interrupts. */
            _tx_thread_system_resume(&pTcb->thread);
            TX_DISABLE  /* Lockout interrupts. */
        }
    }
    else
    {
        pMtx->lock_tsk = NULL;
    }
    TX_RESTORE  /* Restore interrupts. */
}

