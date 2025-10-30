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
/*  exd_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Exit and delete task                                                        */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_system_task_delete                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Exit and delete task from system thread.                                */
/*                                                                              */
/* INPUT                                                                        */
/*      param               Pointer of task control block                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_system_task_reset   Reset control block                         */
/*      _tx_thread_delete           Delete thread                               */
/*      _kernel_sysmem_release      Release system memory                       */
/*                                                                              */
/* CALLED BY                                                                    */
/*      System thread                                                           */
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
void _kernel_system_task_delete(void* param)
{
    _KERNEL_UITRON_TSK* pTcb = param;

    ID_LIST(_kernel_tsk_id, pTcb->tskid) = NULL;
    _kernel_system_task_reset(pTcb);
    _tx_thread_delete(&pTcb->thread);

    if(pTcb->ctsk.stk==NULL)
    {
        _kernel_sysmem_release(pTcb->thread.tx_thread_stack_start);
    }
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      exd_tsk                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Exit and delete task.                                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_system_request      Processing is required of system thread     */
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
/*                                  Before using ThreadX API, it changed so     */
/*                                  that interruption might be enabled.         */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
void exd_tsk(void)
{
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    if((pTcb=_kernel_thread_to_task(_tx_thread_current_ptr))!=NULL && DISPATCH_ENABLE)
    {
        pTcb->actcnt = 0;
        pTcb->tskstat = TTS_DMT;    /* This task status is set to dormant. */
        TX_RESTORE

        _kernel_system_request(_kernel_system_task_delete, pTcb);
    }

    TXI_SYSTEM_ERROR
}

