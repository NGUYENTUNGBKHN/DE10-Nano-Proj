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
/*  twai_flg.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Wait event flag with timeout                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __twai_flg                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Wait event flag(Internal function).                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      flgid               ID number of the event flag.                        */
/*      waiptn              Waiting bit pattern                                 */
/*      wfmode              Waiting flag mode                                   */
/*      p_flgptn            Pointer of flag pattern                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_ILUSE             Illegal using error                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      wai_flg                                                                 */
/*      pol_flg                                                                 */
/*      twai_flg                                                                */
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
/*                                  by flgatr.                                  */
/*                                  Modify the assignment expression to         */
/*                                  wait_pri.                                   */
/*                                                                              */
/********************************************************************************/
ER __twai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN* p_flgptn, TMO tmout)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_FLG* pFlg;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_FLG_WAIT waitdata;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pFlg = (_KERNEL_UITRON_FLG*)ID_LIST(_kernel_flg_id, flgid);
    if(pFlg!=NULL)
    {
        /* Error check of multi wait. */
        if((pFlg->cflg.flgatr&TA_WMUL)==0 && pFlg->wtsk!=NULL)
        {
            rtn = E_ILUSE;  /* Illegal use */
        }
        else
        {
            if(((wfmode==TWF_ANDW) && ((waiptn&pFlg->curflgptn)==waiptn)) ||
                ((wfmode==TWF_ORW) && ((waiptn&pFlg->curflgptn)!=0)))
            {
                *p_flgptn = pFlg->curflgptn;
                if((pFlg->cflg.flgatr&TA_CLR)!=0)
                {
                    pFlg->curflgptn = 0;    /* Clear current bit pattern. */
                }
            }
            else
            {
                if(tmout!=TMO_POL)
                {
                    pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
                    if(pTcb!=NULL && DISPATCH_ENABLE)
                    {
                        /* Wait for event flag. */
                        if((pFlg->cflg.flgatr&TA_TPRI)==0)
                        {
                            _kernel_enque(&pFlg->wtsk, &pTcb->wait);
                        }
                        else
                        {
                            _kernel_enque_task_pri(&pFlg->wtsk, &pTcb->wait);
                        }

                        pTcb->tskstat = TTS_WAI;
                        pTcb->tskwait = TTW_FLG;
                        pTcb->wobjid = flgid;
                        pTcb->wait_obj = pFlg;
                        pTcb->wait_pri = ((pFlg->cflg.flgatr&TA_TPRI)!=0);

                        waitdata.waiptn = waiptn;
                        waitdata.wfmode = wfmode;
                        waitdata.p_flgptn = p_flgptn;
                        pTcb->waitdata = &waitdata;

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
/*      twai_flg                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Wait event flag with timeout.                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      flgid               ID number of the event flag.                        */
/*      waiptn              Waiting bit pattern                                 */
/*      wfmode              Waiting flag mode                                   */
/*      p_flgptn            Pointer of flag pattern                             */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_CTX               Context error                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_ILUSE             Illegal using error                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __twai_flg                  Wait event flag.                            */
/*                                  (Internal function)                         */
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
ER twai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN* p_flgptn, TMO tmout)
{
    CHECK_ID(1, _kernel_flg_maxid, flgid)
    CHECK_PARAM(waiptn!=0 && wfmode==(wfmode&(TWF_ANDW|TWF_ORW)) && p_flgptn!=NULL);
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __twai_flg(flgid, waiptn, wfmode, p_flgptn, tmout);
}

