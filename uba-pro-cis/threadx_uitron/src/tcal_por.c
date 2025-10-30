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
/*  tcal_por.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Call rendezvous with timeout                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

_KERNEL_UITRON_OBJ* _kernel_rendezvous_tsk = NULL;  /* waiting for rendezvous completion list */

static RDVNO _kernel_rdvno = 0;

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_rendezvous_number                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Get new rendezvous number.                                              */
/*                                                                              */
/* INPUT                                                                        */
/*      pTcb                Task control block                                  */
/*                                                                              */
/* OUTPUT                                                                       */
/*      rendezvous number                                                       */
/*                                                                              */
/* CALLS                                                                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      fwd_por                                                                 */
/*      __tacp_por                                                              */
/*      __tcal_por                                                              */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                                                              */
/********************************************************************************/
RDVNO _kernel_rendezvous_number(_KERNEL_UITRON_TSK* pTcb)
{
    RDVNO new_rdvno;

    new_rdvno = (_kernel_rdvno<<(sizeof(RDVNO)*4)) | (pTcb->tskid);
    _kernel_rdvno++;

    return new_rdvno;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_rendezvous_pattern                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Find waiting task by pattern.                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      pTop                Pointer of control list                             */
/*      ptn                 Check pattern                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      contol block of waiting task                                            */
/*                                                                              */
/* CALLS                                                                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      fwd_por                                                                 */
/*      __tacp_por                                                              */
/*      __tcal_por                                                              */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                                                              */
/********************************************************************************/
_KERNEL_UITRON_TSK* _kernel_rendezvous_pattern(_KERNEL_UITRON_OBJ* pTop, RDVPTN ptn)
{
    _KERNEL_UITRON_OBJ* pFindNode = pTop;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_POR_WAIT* pwait;

    if(pFindNode!=NULL)
    {
        do
        {
            pTcb = (_KERNEL_UITRON_TSK*)pFindNode->data;
            pwait = (_KERNEL_UITRON_POR_WAIT*)pTcb->waitdata;
            if((pwait->ptn&ptn)!=0)
            {
                return pTcb;
            }
            pFindNode = pFindNode->next;
        } while(pFindNode!=pTop);
    }

    return NULL;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_rendezvous_find                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Find waiting task by rendezvous number.                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      pTop                Pointer of control list                             */
/*      rdvno               Check rendezvous number                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      contol block of waiting task                                            */
/*                                                                              */
/* CALLS                                                                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      fwd_por                                                                 */
/*      ref_rdv                                                                 */
/*      rpl_rdv                                                                 */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                                                              */
/********************************************************************************/
_KERNEL_UITRON_TSK* _kernel_rendezvous_find(_KERNEL_UITRON_OBJ* pTop, RDVNO rdvno)
{
    _KERNEL_UITRON_OBJ* pFindNode = pTop;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_POR_WAIT* pwait;

    if(pFindNode!=NULL)
    {
        do
        {
            pTcb = (_KERNEL_UITRON_TSK*)pFindNode->data;
            pwait = (_KERNEL_UITRON_POR_WAIT*)pTcb->waitdata;
            if(pwait->rdvno==rdvno)
            {
                return pTcb;
            }
            pFindNode = pFindNode->next;
        } while(pFindNode!=pTop);
    }

    return NULL;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __tcal_por                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Call rendezvous(Internal function).                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      calptn              Calling bit pattern.                                */
/*      msg                 Data pointer to be call                             */
/*      cmsgsz              Data size for call                                  */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      received size                                                           */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_rendezvous_pattern  Find waiting task by pattern                */
/*      _kernel_rendezvous_number   Get new rendezvous number                   */
/*      _kernel_resume_task         Resume a task with error code               */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cal_por                                                                 */
/*      tcal_por                                                                */
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
/*  2012/12/06      M. Miyashita    version 2.2m-sp2, Modified comment(s),      */
/*                                  When a priority is lower than the call      */
/*                                  side's of rendezvous receptionist side,     */
/*                                  the call side corrects the problem which    */
/*                                  may not be waiting-canceled normally.       */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER_UINT __tcal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz, TMO tmout)
{
    ER_UINT rtn;
    _KERNEL_UITRON_POR* pPor;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_TSK* pAcceptTcb;
    _KERNEL_UITRON_POR_WAIT* pwait;
    _KERNEL_UITRON_POR_WAIT wait;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pPor = (_KERNEL_UITRON_POR*)ID_LIST(_kernel_por_id, porid);
    if(pPor!=NULL)
    {
        if(cmsgsz<=pPor->cpor.maxcmsz)
        {
            if(tmout!=TMO_POL)
            {
                pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
                if(pTcb!=NULL && DISPATCH_ENABLE)
                {
                    if((pAcceptTcb=_kernel_rendezvous_pattern(pPor->atsk, calptn))!=NULL)
                    {
                        /* Remove from waiting list */
                        _kernel_deque(&pAcceptTcb->wait);
                        pwait = (_KERNEL_UITRON_POR_WAIT*)pAcceptTcb->waitdata;

                        _kernel_copy((char*)pwait->msg, (char*)msg, cmsgsz);
                        pwait->msgsz = cmsgsz;
                        pwait->rdvno = _kernel_rendezvous_number(pTcb);

                        /* Wait for rendezvous completion. */
                        _kernel_enque(&_kernel_rendezvous_tsk, &pTcb->wait);

                        pTcb->tskstat = TTS_WAI;
                        pTcb->tskwait = TTW_RDV;
                        pTcb->wobjid = porid;
                        pTcb->wait_obj = pPor;
                        pTcb->wait_pri = FALSE;

                        wait.msg = msg;
                        wait.msgsz = pPor->cpor.maxrmsz;
                        wait.rdvno = pwait->rdvno;
                        pTcb->waitdata = &wait;

                        pAcceptTcb->wait_rtn = E_OK;
                        pAcceptTcb->wait_obj = NULL;
                        pAcceptTcb->wait_pri = FALSE;

                        /* Clear cleanup routine pointer. */
                        pAcceptTcb->thread.tx_thread_suspend_cleanup = TX_NULL;

                        if(pAcceptTcb->tskstat==TTS_WAS)
                        {
                            pAcceptTcb->wobjid = 0;
                            pAcceptTcb->tskwait = TTW_NONE;
                            pAcceptTcb->tskstat = TTS_SUS;
                        }
                        else
                        {
                            pAcceptTcb->wobjid = 0;
                            pAcceptTcb->tskwait = TTW_NONE;
                            pAcceptTcb->tskstat = TTS_RDY;

                            _tx_thread_preempt_disable++;
                            _tx_thread_preempt_disable++;   /* Temporarily disable preemption. */
                            TX_RESTORE  /* Restore interrupts. */
                            _tx_thread_system_resume(&pAcceptTcb->thread);
                            TX_DISABLE  /* Lockout interrupts. */
                            _tx_thread_preempt_disable--;
                        }
                    }
                    else
                    {
                        /* Wait for call rendezvous. */
                        if(pPor->cpor.poratr==TA_TFIFO)
                        {
                            _kernel_enque(&pPor->ctsk, &pTcb->wait);
                        }
                        else
                        {
                            _kernel_enque_task_pri(&pPor->ctsk, &pTcb->wait);
                        }

                        pTcb->tskstat = TTS_WAI;
                        pTcb->tskwait = TTW_CAL;
                        pTcb->wobjid = porid;
                        pTcb->wait_obj = pPor;
                        pTcb->wait_pri = (pPor->cpor.poratr!=TA_TFIFO);

                        wait.ptn = calptn;
                        wait.msg = msg;
                        wait.msgsz = cmsgsz;
                        pTcb->waitdata = &wait;
                    }

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
        else
        {
            rtn = (ER_UINT)E_PAR;   /* Parameter error */
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
/*      tcal_por                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Call rendezvous with timeout.                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      calptn              Calling bit pattern.                                */
/*      msg                 Data pointer to be call                             */
/*      cmsgsz              Data size for call                                  */
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
/*      __tcal_por                  Call rendezvous(Internal function)          */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                                                              */
/********************************************************************************/
ER_UINT tcal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz, TMO tmout)
{
    CHECK_ID(1, _kernel_por_maxid, porid)
    CHECK_PARAM(calptn!=0 && !(0<cmsgsz && msg==NULL))
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __tcal_por(porid, calptn, msg, cmsgsz, tmout);
}

