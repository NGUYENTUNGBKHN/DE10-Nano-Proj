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
/*  ref_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference task                                                              */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_tsk                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference task.                                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*      pk_rtsk             Pointer to the packet returning the task state.     */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Task context                                                            */
/*      Non-task context                                                        */
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
/*  2017/10/11      M. Miyashita    version 2.3, Modified comment(s),          */
/*                  T. Shibuya      Add the status setting process if the task  */
/*                                  has become a waiting state by ThreadX of    */
/*                                  the API.                                    */
/*                                  Timer change the method of calculating the  */
/*                                  elapsed time from the start.                */
/*                                                                              */
/********************************************************************************/
ER ref_tsk(ID tskid, T_RTSK* pk_rtsk)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
    TX_TIMER_INTERNAL* timer_list;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_PARAM(pk_rtsk!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
#ifdef TXI_SMP
        ULONG i;

#undef _tx_thread_current_ptr

        for(i=0; i<TX_THREAD_SMP_MAX_CORES; i++)
        {
            if(&pTcb->thread==_tx_thread_current_ptr[i])
            {
                pk_rtsk->tskstat = TTS_RUN;
                break;
            }
        }

        if(TX_THREAD_SMP_MAX_CORES<=i)
        {
            pk_rtsk->tskstat = pTcb->tskstat;
        }
#else
        if(&pTcb->thread==_tx_thread_current_ptr)
        {
            pk_rtsk->tskstat = TTS_RUN;
        }
        else
        {
            pk_rtsk->tskstat = pTcb->tskstat;
        }
#endif

        pk_rtsk->tskpri = (PRI)pTcb->thread.tx_thread_priority;
        pk_rtsk->tskbpri = pTcb->tskbpri;

        if((pTcb->tskstat==TTS_WAI || pTcb->tskstat==TTS_WAS) && (pTcb->thread.tx_thread_state==TX_SUSPENDED))
        {
            pk_rtsk->tskwait = pTcb->tskwait;
            pk_rtsk->wobjid = pTcb->wobjid;
            pk_rtsk->lefttmo = 0;

            /* Pickup address of internal timer structure. */
            timer_list = &(pTcb->thread.tx_thread_timer);

            if((timer_list->tx_timer_internal_list_head>=_tx_timer_list_start) &&
               (timer_list->tx_timer_internal_list_head<_tx_timer_list_end))
            {
                /* Calculate the amount of time that has elapsed since the timer
                    was activated. */
                
                /* Is this timer's entry after the current timer pointer? */
                if(timer_list->tx_timer_internal_list_head>=_tx_timer_current_ptr)
                {
                    /* Calculate ticks left to expiration - just the difference between this 
                        timer's entry and the current timer pointer.  */
                    pk_rtsk->lefttmo = (timer_list->tx_timer_internal_list_head - _tx_timer_current_ptr);
                }
                else
                {
                    /* Calculate the ticks left with a wrapped list condition.  */
                    pk_rtsk->lefttmo  = (timer_list->tx_timer_internal_list_head - _tx_timer_list_start);
                    pk_rtsk->lefttmo += (_tx_timer_list_end - _tx_timer_current_ptr);
                }
                
                /* Adjust the remaining ticks accordingly.  */
                if(timer_list->tx_timer_internal_remaining_ticks>TX_TIMER_ENTRIES)
                {
                    /* Subtract off the last full pass throught the timer list and add the
                        time left.  */
                    pk_rtsk->lefttmo += (timer_list->tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES);
                }
                
                pk_rtsk->lefttmo = TIC2TIM(pk_rtsk->lefttmo);
            }
        }
        else
        {
            pk_rtsk->tskwait = TTW_NONE;
            pk_rtsk->wobjid = 0;
            pk_rtsk->lefttmo = 0;

            /* check of ThreadX status */
            if(pk_rtsk->tskstat==TTS_RUN || pk_rtsk->tskstat==TTS_RDY)
            {
                if(TX_SLEEP<=pTcb->thread.tx_thread_state)
                {
                    if(pTcb->thread.tx_thread_delayed_suspend==TX_FALSE)
                    {
                        pk_rtsk->tskstat = TTS_WAI;
                    }
                    else
                    {
                        pk_rtsk->tskstat = TTS_WAS;
                    }
                }
                else if(pTcb->thread.tx_thread_state==TX_SUSPENDED)
                {
                    pk_rtsk->tskstat = TTS_SUS;
                }
            }
        }

        pk_rtsk->actcnt = pTcb->actcnt;
        pk_rtsk->wupcnt = pTcb->wupcnt;
        pk_rtsk->suscnt = pTcb->suscnt;
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

