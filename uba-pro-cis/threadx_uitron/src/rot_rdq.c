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
/*  rot_rdq.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Rotate task precedence                                                      */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_mutex.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      rot_rdq                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Rotate task precedence.                                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      tskpri              Task priority                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_mutex_priority_change   Priority change for mutex                   */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Renamed some definitions.                   */
/*                                                                              */
/********************************************************************************/
ER rot_rdq(PRI tskpri)
{
    TX_THREAD* thread;
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
    UINT new_priority;
#endif
#endif

TX_INTERRUPT_SAVE_AREA

    CHECK_PARAM((tskpri==TPRI_SELF) || (TMIN_TPRI<=tskpri && tskpri<=TMAX_TPRI))
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    if(tskpri==TPRI_SELF)
    {
        tskpri = _tx_thread_current_ptr->tx_thread_priority;
    }

    if(_tx_thread_priority_list[tskpri]!=NULL)
    {
        thread = _tx_thread_priority_list[tskpri];
        TX_RESTORE
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
        new_priority = (thread->tx_thread_priority-1);
        _tx_mutex_priority_change(thread, new_priority);
        new_priority++;
        _tx_mutex_priority_change(thread, new_priority);
#else
        _tx_mutex_priority_change(thread, thread->tx_thread_priority, thread->tx_thread_preempt_threshold);
#endif
#else
        _tx_mutex_priority_change(thread, thread->tx_thread_priority, thread->tx_thread_preempt_threshold);
#endif
        TX_DISABLE
    }
    TX_RESTORE  /* Restore interrupts. */

    return E_OK;
}

