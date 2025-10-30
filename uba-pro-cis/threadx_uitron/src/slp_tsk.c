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
/*  slp_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Sleep task                                                                  */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      slp_tsk                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Sleep task.                                                             */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_CTX               Context error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _tx_thread_suspend          Suspend thread                              */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER slp_tsk(void)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
    if(pTcb!=NULL)
    {
        if(0<pTcb->wupcnt)
        {
            pTcb->wupcnt--; /* Decrement of a wake up demand. */
        }
        else
        {
            if(DISPATCH_ENABLE)
            {
                pTcb->tskstat = TTS_WAI;
                pTcb->tskwait = TTW_SLP;

                pTcb->wait_rtn = E_OK;

                /* Setup cleanup routine pointer.  */
                pTcb->thread.tx_thread_suspend_cleanup = _kernel_task_cleanup;

                /* Set the state to suspended.  */
                pTcb->thread.tx_thread_state = TX_SUSPENDED;

                /* Set the suspending flag.  */
                pTcb->thread.tx_thread_suspending = TX_TRUE;

                /* Setup the timeout period.  */
                pTcb->thread.tx_thread_timer.tx_timer_internal_remaining_ticks = TX_WAIT_FOREVER;

                _tx_thread_preempt_disable++;   /* Temporarily disable preemption.  */
                TX_RESTORE

                _tx_thread_system_suspend(&pTcb->thread);

                TX_DISABLE

                rtn = pTcb->wait_rtn;
            }
            else
            {
                rtn = E_CTX;    /* Context error. */
            }
        }
    }
    else
    {
        rtn = E_CTX;    /* No support contexts. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

