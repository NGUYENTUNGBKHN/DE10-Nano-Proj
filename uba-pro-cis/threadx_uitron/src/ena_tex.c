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
/*  ena_tex.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Enable task exception                                                       */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

#ifndef TXI_SMP
/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ena_tex                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Enable task exceptions.                                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_task_exception_entry                                            */
/*                                  Task exception entry function               */
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
ER ena_tex(void)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
    if(pTcb!=NULL)
    {
        if(pTcb->texrtn!=NULL && pTcb->tskstat!=TTS_DMT)
        {
            pTcb->texstat &= ~TTEX_DIS;

            if(pTcb->rasptn!=0 && (pTcb->texstat&TTEX_DISABLE_BIT)==0)
            {
                pTcb->texstat |= TTEX_ACT;
                TX_RESTORE  /* Restore interrupts. */

                _kernel_task_exception_entry();

                TX_DISABLE  /* Lockout interrupts. */
                pTcb->texstat &= ~TTEX_ACT;
            }
        }
        else
        {
            rtn = E_OBJ;    /* Object error */
        }
    }
    else
    {
        rtn = E_CTX;    /* No support contexts. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}
#endif

