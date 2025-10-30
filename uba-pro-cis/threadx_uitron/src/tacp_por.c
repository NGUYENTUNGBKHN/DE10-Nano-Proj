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
/*  tacp_por.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Accrept rendezvous with timeout                                             */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __tacp_por                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Accrept rendezvous(Internal function).                                  */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      acpptn              Accept bit pattern.                                 */
/*      p_rdvno             Pointer for rendezvous number                       */
/*      msg                 Data pointer to be call                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      received size                                                           */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_rendezvous_pattern  Find waiting task by pattern                */
/*      _kernel_rendezvous_number   Get new rendezvous number                   */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      acp_por                                                                 */
/*      tacp_por                                                                */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Add a branch process of determining the     */
/*                                  enqueue of ways, depending on the           */
/*                                  pPor->cpor.poratr.                          */
/*                                                                              */
/********************************************************************************/
ER_UINT __tacp_por(ID porid, RDVPTN acpptn, RDVNO* p_rdvno, VP msg, TMO tmout)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_POR* pPor;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_TSK* pCallTcb;
    _KERNEL_UITRON_POR_WAIT* pwait;
    _KERNEL_UITRON_POR_WAIT wait;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pPor = (_KERNEL_UITRON_POR*)ID_LIST(_kernel_por_id, porid);
    if(pPor!=NULL)
    {
        if((pCallTcb=_kernel_rendezvous_pattern(pPor->ctsk, acpptn))!=NULL)
        {
            /* Remove from waiting list */
            _kernel_deque(&pCallTcb->wait);
            pwait = (_KERNEL_UITRON_POR_WAIT*)pCallTcb->waitdata;

            _kernel_copy((char*)msg, (char*)pwait->msg, pwait->msgsz);
            *p_rdvno = _kernel_rendezvous_number(pCallTcb);
            pwait->rdvno = *p_rdvno;

            /* Wait for rendezvous completion. */
            pCallTcb->tskwait = TTW_RDV;
            _kernel_enque(&_kernel_rendezvous_tsk, &pCallTcb->wait);

            rtn = (ER_UINT)pwait->msgsz;

            pwait->msgsz = pPor->cpor.maxrmsz;
        }
        else
        {
            if(tmout!=TMO_POL)
            {
                pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
                if(pTcb!=NULL && DISPATCH_ENABLE)
                {
                    /* Wait for accept rendezvous. */
                    if(pPor->cpor.poratr==TA_TFIFO)
                    {
                        _kernel_enque(&pPor->atsk, &pTcb->wait);
                    }
                    else
                    {
                        _kernel_enque_task_pri(&pPor->atsk, &pTcb->wait);
                    }

                    pTcb->tskstat = TTS_WAI;
                    pTcb->tskwait = TTW_ACP;
                    pTcb->wobjid = porid;
                    pTcb->wait_obj = pPor;
                    pTcb->wait_pri = (pPor->cpor.poratr!=TA_TFIFO);

                    wait.ptn = acpptn;
                    wait.msg = msg;
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

                    if(rtn==E_OK)
                    {
                        *p_rdvno = wait.rdvno;
                        rtn = (ER_UINT)wait.msgsz;
                    }
                }
                else
                {
                    rtn = (ER_UINT)E_CTX;   /* No support contexts. */
                }
            }
            else
            {
                rtn = E_TMOUT;
            }
        }
    }
    else
    {
        rtn = (ER_UINT)E_NOEXS; /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      tacp_por                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Accrept rendezvous with timeout.                                        */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      acpptn              Accept bit pattern.                                 */
/*      p_rdvno             Pointer for rendezvous number                       */
/*      msg                 Data pointer to be call                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      received size                                                           */
/*                                                                              */
/* CALLS                                                                        */
/*      __tacp_por                  Accrept rendezvous(Internal function)       */
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
ER_UINT tacp_por(ID porid, RDVPTN acpptn, RDVNO* p_rdvno, VP msg, TMO tmout)
{
    CHECK_ID(1, _kernel_por_maxid, porid)
    CHECK_PARAM(acpptn!=0 && p_rdvno!=NULL && msg!=NULL)
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __tacp_por(porid, acpptn, p_rdvno, msg, tmout);
}

