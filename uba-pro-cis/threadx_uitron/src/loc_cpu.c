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
/*  loc_cpu.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Lock cpu                                                                    */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      loc_cpu                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Lock cpu.                                                               */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
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
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s),           */
/*                                  TX_RESTORE is deleted.                      */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  TX_INTERRUPT_SAVE_AREA and TX_DISABLE which */
/*                                  do not have a meaning in using it were      */
/*                                  deleted.                                    */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER loc_cpu(void)
{
#ifdef TXI_SMP
TX_INTERRUPT_SAVE_AREA

    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    if(_kernel_loc_cpu==0)
    {
        _tx_thread_preempt_disable++;   /* Disable preemption. */
        _kernel_loc_cpu++;              /* CPU lock status */
    }
    TX_RESTORE  /* Restore interrupts. */

    _tx_thread_interrupt_control(TX_INT_DISABLE);
#else
    CHECK_TSK_CONTEXT()

    _tx_thread_interrupt_control(TX_INT_DISABLE);

    _tx_thread_preempt_disable++;   /* Disable preemption. */
    _kernel_loc_cpu++;              /* CPU lock status */
#endif

    return E_OK;
}

