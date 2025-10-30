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
/*  trcv_mbx.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Receive mailbox with timeout                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __trcv_mbx                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Receive mailbox(Internal function).                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      mbxid               ID number of the mailbox                            */
/*      ppk_msg             Message pointer for receive message                 */
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
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      rcv_mbx                                                                 */
/*      prcv_mbx                                                                */
/*      trcv_mbx                                                                */
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
/*                                  Modify the waiting how the branch condition */
/*                                  by mbxatr.                                  */
/*                                  Modify the assignment expression to         */
/*                                  wait_pri.                                   */
/*                                                                              */
/********************************************************************************/
ER __trcv_mbx(ID mbxid, T_MSG** ppk_msg, TMO tmout)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MBX* pMbx;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pMbx = (_KERNEL_UITRON_MBX*)ID_LIST(_kernel_mbx_id, mbxid);
    if(pMbx!=NULL)
    {
        if(pMbx->mbx!=NULL)
        {
            *ppk_msg = pMbx->mbx;
            pMbx->mbx = (*ppk_msg)->next;
            (*ppk_msg)->next = NULL;

            if(pMbx->mbx==NULL)
            {
                pMbx->mbx_tail = NULL;
            }
        }
        else
        {
            if(tmout!=TMO_POL)
            {
                pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
                if(pTcb!=NULL && DISPATCH_ENABLE)
                {
                    /* Wait for data queue. */
                    if((pMbx->cmbx.mbxatr&TA_TPRI)==0)
                    {
                        _kernel_enque(&pMbx->wtsk, &pTcb->wait);
                    }
                    else
                    {
                        _kernel_enque_task_pri(&pMbx->wtsk, &pTcb->wait);
                    }

                    pTcb->tskstat = TTS_WAI;
                    pTcb->tskwait = TTW_MBX;
                    pTcb->wobjid = mbxid;
                    pTcb->wait_obj = pMbx;
                    pTcb->wait_pri = ((pMbx->cmbx.mbxatr&TA_TPRI)!=0);

                    pTcb->waitdata = ppk_msg;

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
/*      trcv_mbx                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Receive mailbox with timeout.                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      mbxid               ID number of the mailbox                            */
/*      ppk_msg             Message pointer for receive message                 */
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
/*      __trcv_mbx                  Receive mailbox(Internal function)          */
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
ER trcv_mbx(ID mbxid, T_MSG** ppk_msg, TMO tmout)
{
    CHECK_ID(1, _kernel_mbx_maxid, mbxid)
    CHECK_PARAM(ppk_msg!=NULL)
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __trcv_mbx(mbxid, ppk_msg, tmout);
}

