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
/*  unl_cpu.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Unlock cpu                                                                  */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      unl_cpu                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Unlock cpu.                                                             */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_thread_preemption_change                                            */
/*                                  Change thread preemption level              */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER unl_cpu(void)
{
#ifdef TXI_SMP
TX_INTERRUPT_SAVE_AREA

    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable = 0; /* Release preemption. */
    _kernel_loc_cpu = 0;            /* CPU lock status */

    if(DISPATCH_ENABLE)
    {
        /* Return to the system so the higher priority thread can be scheduled. */
        _tx_thread_preempt_disable++;   /* Disable preemption. */
        TX_RESTORE  /* Restore interrupts. */

        _tx_thread_interrupt_control(TX_INT_ENABLE);
        _tx_thread_system_return();
    }
    else
    {
        TX_RESTORE  /* Restore interrupts. */
        _tx_thread_interrupt_control(TX_INT_ENABLE);
    }
#else
TX_INTERRUPT_SAVE_AREA

    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable = 0; /* Release preemption. */
    _kernel_loc_cpu = 0;            /* CPU lock status */

    if(DISPATCH_ENABLE)
    {
        /* Return to the system so the higher priority thread can be scheduled. */
        TX_RESTORE  /* Restore interrupts. */

        _tx_thread_interrupt_control(TX_INT_ENABLE);
        _tx_thread_system_return();
    }
    else
    {
        TX_RESTORE  /* Restore interrupts. */
        _tx_thread_interrupt_control(TX_INT_ENABLE);
    }
#endif

    return E_OK;
}

