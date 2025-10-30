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
/*  rsm_tsk.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Resume task                                                                 */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      rsm_tsk                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Resume task.                                                            */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*      _tx_thread_resume           Resume thread                               */
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
/*  2017/10/11      M. Miyashita    version 2.3, Modified comment(s),           */
/*                  T. Shibuya      The task is a state waiting by ThreadX of   */
/*                                  the API to suspend state, fix the problem   */
/*                                  that is released also waiting state of      */
/*                                  ThreadX of the API when they resume.        */
/*                                                                              */
/********************************************************************************/
ER rsm_tsk(ID tskid)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
        switch(pTcb->tskstat)
        {
        case TTS_SUS:
            if(0<pTcb->suscnt)
            {
                pTcb->suscnt--; /* Decrement of a suspend demand. */
            }

            if(pTcb->suscnt==0)
            {
                pTcb->tskstat = TTS_RDY;

                TX_RESTORE
                _tx_thread_resume(&pTcb->thread);
                TX_DISABLE
            }
            break;

        case TTS_WAS:
            if(0<pTcb->suscnt)
            {
                pTcb->suscnt--; /* Decrement of a suspend demand. */
            }

            if(pTcb->suscnt==0)
            {
                pTcb->tskstat = TTS_WAI;

                if(pTcb->thread.tx_thread_delayed_suspend!=TX_FALSE)
                {
                    /* Clear the delayed suspension.  */
                    pTcb->thread.tx_thread_delayed_suspend = TX_FALSE;
                }

                if((pTcb->thread.tx_thread_state==TX_SUSPENDED)
                 &&(pTcb->tskwait!=TTW_SLP))
                {
                    pTcb->tskstat = TTS_RDY;

                    TX_RESTORE
                    _tx_thread_resume(&pTcb->thread);
                    TX_DISABLE
                }
            }
            break;

        case TTS_RUN:
        case TTS_RDY:
        case TTS_WAI:
        case TTS_DMT:
        case TTS_NONE:
        default:
            rtn = E_OBJ;    /* Object error */
            break;
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

