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
/*  acre_tsk.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create task(Auto ID)                                                        */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      acre_tsk                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create task(ID number automatic assignment)                             */
/*                                                                              */
/* INPUT                                                                        */
/*      pk_ctsk             Pointer to the packet containing the task creation  */
/*                          information.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_NOID              No ID number available                              */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_object_find         Find free object index                      */
/*      __cre_tsk                   Create task(Internal function).             */
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
ER_ID acre_tsk(T_CTSK* pk_ctsk)
{
    ID tskid;
    ER_ID rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_PARAM(pk_ctsk!=NULL && pk_ctsk->task!=NULL && TX_MINIMUM_STACK<=pk_ctsk->stksz)
    CHECK_PRIORITY(pk_ctsk->itskpri)
    CHECK_ATR(pk_ctsk->tskatr, TA_ACT|TA_ASM)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if((tskid=_kernel_object_find(_kernel_tsk_id, _kernel_tsk_maxid))!=0)
    {
        if((rtn=(ER_ID)__cre_tsk(tskid, pk_ctsk))==E_OK)
        {
            rtn = (ER_ID)tskid;
        }
    }
    else
    {
        rtn = (ER_ID)E_NOID;    /* No ID number available. */
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

