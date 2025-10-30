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
/*  cre_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create task                                                                 */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_tsk                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create task(Internal function).                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*      pk_ctsk             Pointer to the packet containing the task creation  */
/*                          information.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_object_name         Make object name                            */
/*      _kernel_sysmem_alloc        Get system memory                           */
/*      _tx_thread_create           Create thread                               */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_tsk                                                                 */
/*      acre_tsk                                                                */
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
ER __cre_tsk(ID tskid, T_CTSK* pk_ctsk)
{
    _KERNEL_UITRON_TSK* pTcb = &ID_LIST(_kernel_tsk, tskid);

    /* Set create data. */
    pTcb->tskid = tskid;

#ifndef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    _kernel_object_name(pTcb->name, "uiTSK", tskid);
#else
    strcpy(pTcb->name, (const char *)pk_ctsk->name);
#endif

    pTcb->ctsk.tskatr = pk_ctsk->tskatr;
    pTcb->ctsk.exinf = pk_ctsk->exinf;
    pTcb->ctsk.task = pk_ctsk->task;
    pTcb->ctsk.itskpri = pk_ctsk->itskpri;
    pTcb->ctsk.stksz = pk_ctsk->stksz;
    pTcb->ctsk.stk = pk_ctsk->stk;

    pTcb->stacd = pk_ctsk->exinf;
    pTcb->tskstat = (pk_ctsk->tskatr&TA_ACT)?TTS_RDY:TTS_DMT;
    pTcb->tskbpri = pk_ctsk->itskpri;

    pTcb->actcnt = 0;

    pTcb->wupcnt = 0;
    pTcb->suscnt = 0;

    pTcb->tskwait = TTW_NONE;
    pTcb->wobjid = 0;
    pTcb->wait_obj = NULL;
    pTcb->wait_pri = FALSE;
    pTcb->wait_rtn = E_OK;
    pTcb->waitdata = NULL;
    pTcb->wait.plist = NULL;
    pTcb->wait.data = pTcb;

    pTcb->texstat = TTEX_DIS;
    pTcb->texrtn = NULL;
    pTcb->rasptn = 0;

    pTcb->tx_stack_ptr = NULL;
    pTcb->tx_stack_start = NULL;
    pTcb->tx_stack_end = NULL;

    pTcb->locked_mutex = NULL;

    TXI_TSK_TARGET_DEFINE_INIT

    if(pTcb->ctsk.stk==NULL)
    {
        if((pTcb->thread.tx_thread_stack_start=_kernel_sysmem_alloc(pTcb->ctsk.stksz))==NULL)
        {
            return E_NOMEM; /* Memory none */
        }
    }
    else
    {
        pTcb->thread.tx_thread_stack_start = pTcb->ctsk.stk;
    }

    ID_LIST(_kernel_tsk_id, tskid) = pTcb;

    _tx_thread_create(&pTcb->thread, pTcb->name, _kernel_task_entry, (ULONG)pTcb,
        pTcb->thread.tx_thread_stack_start, pTcb->ctsk.stksz, (UINT)pTcb->ctsk.itskpri, (UINT)pTcb->ctsk.itskpri,
        TX_NO_TIME_SLICE, (pk_ctsk->tskatr&TA_ACT)!=0?TX_AUTO_START:TX_DONT_START);

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_tsk                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create task.                                                            */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*      pk_ctsk             Pointer to the packet containing the task creation  */
/*                          information.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __cre_tsk           Create task(Internal function).                     */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Initialization                                                          */
/*      Task context                                                            */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  The preemption control method was arranged. */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER cre_tsk(ID tskid, T_CTSK* pk_ctsk)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_tsk_maxid, tskid)
    CHECK_PARAM(pk_ctsk!=NULL && pk_ctsk->task!=NULL && TX_MINIMUM_STACK<=pk_ctsk->stksz)
    CHECK_PRIORITY(pk_ctsk->itskpri)
    CHECK_ATR(pk_ctsk->tskatr, TA_ACT|TA_ASM)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_tsk_id, tskid)==NULL)
    {
        rtn = __cre_tsk(tskid, pk_ctsk);
    }
    else
    {
        rtn = E_OBJ;    /* The object is already registered. */
    }

    TX_DISABLE  /* Lockout interrupts. */
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
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

