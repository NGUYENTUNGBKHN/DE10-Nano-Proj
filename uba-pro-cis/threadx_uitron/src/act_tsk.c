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
/*  act_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Activate task                                                               */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_system_task_restart                                             */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Restart task from system thread.                                        */
/*                                                                              */
/* INPUT                                                                        */
/*      param               Pointer of task control block                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_thread_reset            Reset control block                         */
/*      _tx_thread_resume           Resume thread                               */
/*                                                                              */
/* CALLED BY                                                                    */
/*      System task                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      System task                                                             */
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
void _kernel_system_task_restart(void* param)
{
    _KERNEL_UITRON_TSK* pTcb = param;

    pTcb->tskstat = TTS_RDY;

    /* Restart this task */
    _tx_thread_reset(&pTcb->thread);
    _tx_thread_resume(&pTcb->thread);
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      act_tsk                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Activate task                                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               Task ID                                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_QOVR              Queue overflow                                      */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*      _kernel_system_request      Processing is required of system thread     */
/*      _tx_thread_system_return    Return to system                            */
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
/*  2014/01/07      M. Miyashita    version 2.2m-sp4, Modified comment(s),      */
/*                                  It changes so that it may restart directly. */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER act_tsk(ID tskid)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    if((pTcb=_kernel_task_tcb(tskid))!=NULL)
    {
        if(pTcb->tskstat==TTS_DMT)
        {
            _tx_thread_preempt_disable++;   /* Disable preemption. */
            _kernel_system_task_restart(pTcb);
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
        else
        {
            if(pTcb->actcnt<TMAX_ACTCNT)
            {
                pTcb->actcnt++;
            }
            else
            {
                rtn = E_QOVR;   /* Overflow of activation queue. */
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

