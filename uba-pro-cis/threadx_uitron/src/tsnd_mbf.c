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
/*  tsnd_mbf.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Send message buffer with timeout                                            */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __tsnd_mbf                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Send message buffer(Internal function).                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      mbfid               ID number of the message buffer.                    */
/*      msg                 Data pointer to be sent                             */
/*      msgsz               Data size for sent                                  */
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
/*      __kernel_copy_to_mbf        Copy to message buffer                      */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      snd_mbf                                                                 */
/*      psnd_mbf                                                                */
/*      tsnd_mbf                                                                */
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
/*                                  Current task information is acquired        */
/*                                  directly.                                   */
/*                                  Before using ThreadX API, it changed so     */
/*                                  that interruption might be enabled.         */
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER __tsnd_mbf(ID mbfid, VP msg, UINT msgsz, TMO tmout)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MBF* pMbf;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_MBF_WAIT* pwait;
    _KERNEL_UITRON_MBF_WAIT wait;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pMbf = (_KERNEL_UITRON_MBF*)ID_LIST(_kernel_mbf_id, mbfid);
    if(pMbf!=NULL)
    {
        if(msgsz<=pMbf->cmbf.maxmsz)
        {
            if(pMbf->rtsk!=NULL)
            {
                /* Remove from waiting list */
                pTcb = (_KERNEL_UITRON_TSK*)pMbf->rtsk->data;
                _kernel_deque(&pTcb->wait);

                pwait = (_KERNEL_UITRON_MBF_WAIT*)pTcb->waitdata;
                _kernel_copy(pwait->msg, msg, msgsz);
                pwait->msgsz = msgsz;

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
                if(__kernel_copy_to_mbf(pMbf, (char*)msg, msgsz)==0)
                {
                    if(tmout!=TMO_POL)
                    {
                        pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
                        if(pTcb!=NULL && DISPATCH_ENABLE)
                        {
                            /* Wait for message buffer. */
                            if(pMbf->cmbf.mbfatr==TA_TFIFO)
                            {
                                _kernel_enque(&pMbf->stsk, &pTcb->wait);
                            }
                            else
                            {
                                _kernel_enque_task_pri(&pMbf->stsk, &pTcb->wait);
                            }

                            pTcb->tskstat = TTS_WAI;
                            pTcb->tskwait = TTW_SMBF;
                            pTcb->wobjid = mbfid;
                            pTcb->wait_obj = pMbf;
                            pTcb->wait_pri = ((pMbf->cmbf.mbfatr&TA_TPRI)!=0);

                            wait.msg = msg;
                            wait.msgsz = msgsz;
                            pTcb->waitdata = &wait;

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
            rtn = E_PAR;    /* Message length is big. */
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
/*      tsnd_mbf                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Send message buffer with timeout.                                       */
/*                                                                              */
/* INPUT                                                                        */
/*      mbfid               ID number of the message buffer.                    */
/*      msg                 Data pointer to be sent                             */
/*      msgsz               Data size for sent                                  */
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
/*      __tsnd_mbf                  Send message buffer(Internal function)      */
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
ER tsnd_mbf(ID mbfid, VP msg, UINT msgsz, TMO tmout)
{
    CHECK_ID(1, _kernel_mbf_maxid, mbfid)
    CHECK_PARAM(msg!=NULL && 0<msgsz)
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __tsnd_mbf(mbfid, msg, msgsz, tmout);
}

