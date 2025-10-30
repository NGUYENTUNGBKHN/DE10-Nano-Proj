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
/*  wup_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Wake up task                                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      wup_tsk                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Wake up task.                                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_QOVR              Queue overflow                                      */
/*      E_OBJ               Object state error                                  */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*      _kernel_task_tskwait        Get task waiting factor                     */
/*      _kernel_resume_task         Resume a task with error code               */
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
/*                                  Before using ThreadX API, it changed so     */
/*                                  that interruption might be enabled.         */
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER wup_tsk(ID tskid)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
        if(pTcb->tskstat!=TTS_DMT)
        {
            if(_kernel_task_tskwait(pTcb)==TTW_SLP)
            {
                pTcb->wait_rtn = E_OK;
                pTcb->wait_obj = NULL;

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
                if(pTcb->wupcnt<TMAX_WUPCNT)
                {
                    pTcb->wupcnt++; /* Increment of a wake up demand. */
                }
                else
                {
                    rtn = E_QOVR;   /* Overflow of a demand number. */
                }
            }
        }
        else
        {
            rtn = E_OBJ;    /* The target task is dormant. */
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

