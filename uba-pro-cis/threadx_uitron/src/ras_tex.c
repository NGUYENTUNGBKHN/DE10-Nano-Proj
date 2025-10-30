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
/*  ras_tex.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Raise task exception                                                        */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

#ifndef TXI_SMP
/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_task_exception_entry                                            */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Entry function of the task exception.                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      User task exception handler                                             */
/*      _tx_thread_schedule         Thread scheduler                            */
/*                                                                              */
/* CALLED BY                                                                    */
/*      System                                                                  */
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
/*                                  Before calling a scheduler, the processing  */
/*                                  which clears a current task was added.      */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
VOID  _kernel_task_exception_entry(VOID)
{
    _KERNEL_UITRON_TSK* pTcb;
    TEXPTN rasptn;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
    if(pTcb!=NULL)
    {
        pTcb->texstat |= TTEX_ACT;
        if(pTcb && pTcb->texrtn!=NULL)
        {
            rasptn = pTcb->rasptn;
            while(rasptn!=0 && pTcb->texrtn!=NULL && (pTcb->texstat&TTEX_DIS)==0)
            {
                pTcb->rasptn = 0;
                TX_RESTORE  /* Restore interrupts. */

                (*((void (*)(TEXPTN, VP_INT))pTcb->texrtn))(rasptn, pTcb->stacd);

                TX_DISABLE  /* Lockout interrupts. */
                rasptn = pTcb->rasptn;
            }
        }

        if(pTcb->tx_stack_ptr!=NULL)
        {
            /* Recover old stack pointer. */
            pTcb->thread.tx_thread_stack_ptr = pTcb->tx_stack_ptr;
            pTcb->thread.tx_thread_stack_start = pTcb->tx_stack_start;
            pTcb->thread.tx_thread_stack_end = pTcb->tx_stack_end;

            pTcb->tx_stack_ptr = NULL;

            pTcb->texstat &= ~TTEX_ACT;

            _tx_thread_execute_ptr = &pTcb->thread;
            _tx_thread_current_ptr = TX_NULL;

            /* return control to the scheduler */
            _tx_thread_schedule();
        }
    }
    TX_RESTORE  /* Restore interrupts. */
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_task_exception_set                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Set task exception information.                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_thread_stack_build      Build thread stack                          */
/*                                                                              */
/* CALLED BY                                                                    */
/*      System                                                                  */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
void _kernel_task_exception_set(_KERNEL_UITRON_TSK* pTcb)
{
    if(pTcb->tx_stack_ptr==NULL)
    {
        /* The stack for task exceptions is built. */
        pTcb->tx_stack_ptr = pTcb->thread.tx_thread_stack_ptr;
        pTcb->tx_stack_start = pTcb->thread.tx_thread_stack_start;
        pTcb->tx_stack_end = pTcb->thread.tx_thread_stack_end;

        pTcb->thread.tx_thread_stack_end = pTcb->thread.tx_thread_stack_ptr;

        _tx_thread_stack_build(&pTcb->thread, _kernel_task_exception_entry);
    }
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ras_tex                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Raise task exception.                                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*      rasptn              Raise patern data.                                  */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*      _kernel_task_exception_entry                                            */
/*                                  Task exception entry function               */
/*      _kernel_task_exception_set  Set task exception information              */
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
/*                                  Current task information is acquired        */
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER ras_tex(ID tskid, TEXPTN rasptn)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_PARAM(rasptn!=0)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
        if(pTcb->texrtn!=NULL && pTcb->tskstat!=TTS_DMT)
        {
            pTcb->rasptn |= rasptn;

            if((pTcb->texstat&TTEX_DISABLE_BIT)==0)
            {
                if(_tx_thread_current_ptr==&pTcb->thread)
                {
                    pTcb->texstat |= TTEX_ACT;
                    TX_RESTORE  /* Restore interrupts. */

                    _kernel_task_exception_entry();

                    TX_DISABLE  /* Lockout interrupts. */
                    pTcb->texstat &= ~TTEX_ACT;
                }
                else
                {
                    _kernel_task_exception_set(pTcb);
                }
            }
        }
        else
        {
            rtn = E_OBJ;    /* Object error */
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}
#endif

