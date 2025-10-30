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
/*  def_inh.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  interrupt                                                                   */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

T_DINH _kernel_inhno[SYSTEM_INHNO_MAX+1];

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _txi_inh_handler_entry                                                  */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Entry function of interrupt                                             */
/*                                                                              */
/* INPUT                                                                        */
/*      inhno                       Interrupt number                            */
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
/*      Interrupt process                                                       */
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
#if 0
void save_intr_log(unsigned char inhno);
#endif
int _txi_inh_handler_entry(int inhno)
{
#ifdef TXI_RECOVER_LOCK_CPU_STATUS
TX_INTERRUPT_SAVE_AREA
#endif

    if(0<=inhno && inhno<=SYSTEM_INHNO_MAX)
    {
        if(_kernel_inhno[inhno].inthdr!=NULL)
        {
#if 0
        	save_intr_log(inhno);
#endif
            (*((void (*)(void))(_kernel_inhno[inhno].inthdr)))();
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
/*      def_inh                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Define interrupt handler.                                               */
/*                                                                              */
/* INPUT                                                                        */
/*      inhno               Interrupt handler number.                           */
/*      pk_dinh             Pointer to the packet containing the interrupt      */
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
ER def_inh(INHNO inhno, T_DINH* pk_dinh)
{
TX_INTERRUPT_SAVE_AREA

    CHECK_PARAM(0<=inhno && inhno<=SYSTEM_INHNO_MAX)
    CHECK_PARAM((pk_dinh==NULL) || (pk_dinh!=NULL && pk_dinh->inthdr!=NULL))
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    if(pk_dinh!=NULL)
    {
        _kernel_inhno[inhno].inhatr = pk_dinh->inhatr;
        _kernel_inhno[inhno].inthdr = pk_dinh->inthdr;
    }
    else
    {
        _kernel_inhno[inhno].inthdr = NULL;
    }
    TX_RESTORE  /* Restore interrupts. */

    return E_OK;
}

