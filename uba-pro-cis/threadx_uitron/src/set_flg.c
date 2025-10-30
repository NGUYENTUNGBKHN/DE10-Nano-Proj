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
/*  set_flg.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Set event flag                                                              */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __set_flg                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Set event flag(Internal function).                                      */
/*                                                                              */
/* INPUT                                                                        */
/*      flgid               ID number of the event flag                         */
/*      setptn              Setting bit pattern                                 */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_resume_task         Resume a task with error code               */
/*      _tx_thread_system_return    Return to system                            */
/*                                                                              */
/* CALLED BY                                                                    */
/*      set_flg                                                                 */
/*      iset_flg                                                                */
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
/*  2013/12/05      H. Tousaki      version 2.2m-sp1, Modified comment(s).      */
/*                                  When iset_flg() was called by interrupt     */
/*                                  processing in the process of this function, */
/*                                  an endless loop occurred. This problem has  */
/*                                  been corrected.                             */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER __set_flg(ID flgid, FLGPTN setptn)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_FLG* pFlg;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_TSK* pTcbNext;
    _KERNEL_UITRON_TSK* pTcbTail;
    _KERNEL_UITRON_FLG_WAIT* waitdata;
    _KERNEL_UITRON_OBJ* resume_list;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pFlg = (_KERNEL_UITRON_FLG*)ID_LIST(_kernel_flg_id, flgid);
    if(pFlg!=NULL)
    {
        resume_list = NULL;

        /* Set bit pattern. */
        pFlg->curflgptn |= setptn;

        if(pFlg->wtsk!=NULL)
        {
            _tx_thread_preempt_disable++;   /* Disable preemption. */

            pTcbNext = (_KERNEL_UITRON_TSK*)pFlg->wtsk->data;
            pTcbTail = (_KERNEL_UITRON_TSK*)pTcbNext->wait.prev->data;

            do
            {
                pTcb = pTcbNext;
                pTcbNext = (_KERNEL_UITRON_TSK*)pTcb->wait.next->data;

                waitdata = (_KERNEL_UITRON_FLG_WAIT*)pTcb->waitdata;
                if(((waitdata->wfmode==TWF_ANDW) && ((waitdata->waiptn&pFlg->curflgptn)==waitdata->waiptn)) ||
                    ((waitdata->wfmode==TWF_ORW) && ((waitdata->waiptn&pFlg->curflgptn)!=0)))
                {
                    *waitdata->p_flgptn = pFlg->curflgptn;

                    /* Remove from waiting list */
                    _kernel_deque(&pTcb->wait);

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

                        _kernel_enque(&resume_list, &pTcb->wait);
                    }

                    if((pFlg->cflg.flgatr&TA_CLR)!=0)
                    {
                        pFlg->curflgptn = 0;    /* Clear current bit pattern. */
                        break;  /* Exit this loop. */
                    }
                }
            } while(pFlg->wtsk!=NULL && pTcb!=pTcbTail);

            while(resume_list!=NULL)
            {
                pTcb = (_KERNEL_UITRON_TSK*)resume_list->data;
                _kernel_deque(&pTcb->wait);

                _tx_thread_preempt_disable++;   /* Temporarily disable preemption. */
                TX_RESTORE  /* Restore interrupts. */
                _tx_thread_system_resume(&pTcb->thread);
                TX_DISABLE  /* Lockout interrupts. */
            }
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
/*      set_flg                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Set event flag.                                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      flgid               ID number of the event flag                         */
/*      setptn              Setting bit pattern                                 */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __set_flg                  Set event flag(Internal function)            */
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
ER set_flg(ID flgid, FLGPTN setptn)
{
    CHECK_ID(1, _kernel_flg_maxid, flgid)
    CHECK_TSK_CONTEXT()

    return __set_flg(flgid, setptn);
}

