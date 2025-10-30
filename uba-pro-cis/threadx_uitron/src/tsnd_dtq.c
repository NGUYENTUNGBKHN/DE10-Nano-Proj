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
/*  tsnd_dtq.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Send data queue with timeout                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __tsnd_dtq                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Send data queue(Internal function).                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      dtqid               ID number of the data queue.                        */
/*      data                Data element to be sent                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_resume_task         Resume a task with error code               */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      snd_dtq                                                                 */
/*      psnd_dtq                                                                */
/*      tsnd_dtq                                                                */
/*      ipsnd_dtq_sys                                                           */
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
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER __tsnd_dtq(ID dtqid, VP_INT data, TMO tmout)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_DTQ* pDtq;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pDtq = (_KERNEL_UITRON_DTQ*)ID_LIST(_kernel_dtq_id, dtqid);
    if(pDtq!=NULL)
    {
        if(pDtq->rtsk!=NULL)
        {
            /* Remove from waiting list */
            pTcb = (_KERNEL_UITRON_TSK*)pDtq->rtsk->data;
            _kernel_deque(&pTcb->wait);

            *((VP_INT*)pTcb->waitdata) = data;

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
            if(pDtq->dtqcnt<pDtq->cdtq.dtqcnt)
            {
                /* Set data to queue. */
                pDtq->dtqptr[pDtq->setpos] = data;
                pDtq->dtqcnt++;
                pDtq->setpos = (pDtq->setpos+1)%pDtq->cdtq.dtqcnt;
            }
            else
            {
                if(tmout!=TMO_POL)
                {
                    pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
                    if(pTcb!=NULL && DISPATCH_ENABLE)
                    {
                        /* Wait for data queue. */
                        if(pDtq->cdtq.dtqatr==TA_TFIFO)
                        {
                            _kernel_enque(&pDtq->stsk, &pTcb->wait);
                        }
                        else
                        {
                            _kernel_enque_task_pri(&pDtq->stsk, &pTcb->wait);
                        }

                        pTcb->tskstat = TTS_WAI;
                        pTcb->tskwait = TTW_SDTQ;
                        pTcb->wobjid = dtqid;
                        pTcb->wait_obj = pDtq;
                        pTcb->wait_pri = (pDtq->cdtq.dtqatr!=TA_TFIFO);

                        pTcb->waitdata = data;

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
                    rtn = E_TMOUT;
                }
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

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      tsnd_dtq                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Send data queue with timeout.                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      dtqid               ID number of the data queue.                        */
/*      data                Data element to be sent                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __tsnd_dtq                  Send data queue(Internal function)          */
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
ER tsnd_dtq(ID dtqid, VP_INT data, TMO tmout)
{
    CHECK_ID(1, _kernel_dtq_maxid, dtqid)
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __tsnd_dtq(dtqid, data, tmout);
}

