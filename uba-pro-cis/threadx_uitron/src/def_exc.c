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
/*  def_exc.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  CPU exception                                                               */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

T_DEXC _kernel_exc[SYSTEM_EXC_MAX+1];

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _txi_exc_handler_entry                                                  */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Entry function of CPU exception error                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      excno                       CPU exception number                        */
/*                                  (CPU peculiar)                              */
/*                                                                              */
/* OUTPUT                                                                       */
/*      =0                          Unsettled                                   */
/*      !=0                         Finishing                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      User handler                                                            */
/*                                                                              */
/* CALLED BY                                                                    */
/*      CPU exception process                                                   */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Internal(Assembler)                                                     */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s),           */
/*                                  The processing which clears _kernel_loc_cpu */
/*                                  and _tx_thread_preempt_disable was added at */
/*                                  the time of the end of handler processing.  */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  It changed so that handler number might be  */
/*                                  received from 0.                            */
/*                                  Recovery processing was corrected so that   */
/*                                  it could change by a macro definition.      */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
int _txi_exc_handler_entry(int excno)
{
#ifdef TXI_RECOVER_LOCK_CPU_STATUS
TX_INTERRUPT_SAVE_AREA
#endif

    if(0<=excno && excno<=SYSTEM_EXC_MAX)
    {
        if(_kernel_exc[excno].exchdr!=NULL)
        {
            (*((void (*)(void))(_kernel_exc[excno].exchdr)))();

#ifdef TXI_RECOVER_LOCK_CPU_STATUS
            TX_DISABLE  /* Lockout interrupts. */
            if(_tx_thread_system_state==1 && _kernel_loc_cpu!=0)
            {
                if(0<_tx_thread_preempt_disable)
                {
                    _tx_thread_preempt_disable--; /* Release preemption. */
                }
                _kernel_loc_cpu = 0;            /* CPU lock status */
            }
            TX_RESTORE  /* Restore interrupts. */
#endif

            return 1;
        }
    }

    return 0;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      def_exc                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Define CPU exception handler.                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      excno               CPU exception handler number.                       */
/*      pk_dexc             Pointer to the packet containing the CPU exception  */
/*                          handler definition information.                     */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
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
/*                                  It changed so that handler number might be  */
/*                                  received from 0.                            */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER def_exc(EXCNO excno, T_DEXC* pk_dexc)
{
TX_INTERRUPT_SAVE_AREA

    CHECK_PARAM(0<=excno && excno<=SYSTEM_EXC_MAX)
    CHECK_PARAM((pk_dexc==NULL) || (pk_dexc!=NULL && pk_dexc->exchdr!=NULL))
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    if(pk_dexc!=NULL)
    {
        _kernel_exc[excno].excatr = pk_dexc->excatr;
        _kernel_exc[excno].exchdr = pk_dexc->exchdr;
    }
    else
    {
        _kernel_exc[excno].exchdr = NULL;
    }
    TX_RESTORE  /* Restore interrupts. */

    return E_OK;
}

