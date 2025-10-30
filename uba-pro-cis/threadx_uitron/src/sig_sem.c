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
/*  sig_sem.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Signal to semaphore                                                         */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __sig_sem                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Signal to semaphore(Internal function).                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      semid               ID number of the semaphore.                         */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_NOEXS             Non-existent object                                 */
/*      E_QOVR              Queue overflow                                      */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_resume_task         Resume a task with error code               */
/*                                                                              */
/* CALLED BY                                                                    */
/*      sig_sem                                                                 */
/*      isig_sem                                                                */
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
ER __sig_sem(ID semid)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_SEM* pSem;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pSem = (_KERNEL_UITRON_SEM*)ID_LIST(_kernel_sem_id, semid);
    if(pSem!=NULL)
    {
        if(pSem->wtsk!=NULL)
        {
            /* Remove from waiting list */
            pTcb = (_KERNEL_UITRON_TSK*)pSem->wtsk->data;
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

                _tx_thread_preempt_disable++;   /* Temporarily disable preemption. */
                TX_RESTORE  /* Restore interrupts. */
                _tx_thread_system_resume(&pTcb->thread);
                TX_DISABLE  /* Lockout interrupts. */
            }
        }
        else
        {
            if(pSem->semcnt<pSem->csem.maxsem)
            {
                pSem->semcnt++;
            }
            else
            {
                rtn = E_QOVR;   /* Overflow of semaphore count. */
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
/*      sig_sem                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Signal to semaphore.                                                    */
/*                                                                              */
/* INPUT                                                                        */
/*      semid               ID number of the semaphore.                         */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_NOEXS             Non-existent object                                 */
/*      E_QOVR              Queue overflow                                      */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __sig_sem                   Signal to semaphore.                        */
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
ER sig_sem(ID semid)
{
    CHECK_ID(1, _kernel_sem_maxid, semid)
    CHECK_TSK_CONTEXT()

    return __sig_sem(semid);
}

