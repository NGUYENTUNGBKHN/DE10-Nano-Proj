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
/*  del_mpf.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Delete fixed sized memory pool                                              */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_block_pool.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      del_mpf                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Delete fixed sized memory pool.                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      mpfid               ID number of the fixed sized memory pool.           */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_block_pool_delete       Delete block memory pool                    */
/*      _kernel_resume_task         Resume a task with error code               */
/*      _kernel_sysmem_release      Release system memory                       */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  Before using ThreadX API, it changed so     */
/*                                  that interruption might be enabled.         */
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER del_mpf(ID mpfid)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MPF* pMpf;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mpf_maxid, mpfid)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pMpf = (_KERNEL_UITRON_MPF*)ID_LIST(_kernel_mpf_id, mpfid);
    if(pMpf!=NULL)
    {
        _tx_block_pool_delete(&pMpf->bpool);

        _tx_thread_preempt_disable++;   /* Disable preemption. */

        ID_LIST(_kernel_mpf_id, mpfid) = NULL;

        while(pMpf->wtsk!=NULL)
        {
            /* Remove from waiting list */
            pTcb = (_KERNEL_UITRON_TSK*)pMpf->wtsk->data;
            _kernel_deque(&pTcb->wait);

            pTcb->wait_rtn = E_DLT;
            pTcb->wait_obj = NULL;
            pTcb->wait_pri = FALSE;

            /* Clear cleanup routine pointer. */
            pTcb->thread.tx_thread_suspend_cleanup = TX_NULL;

            if(pTcb->tskstat==TTS_WAS)
            {
                pTcb->wobjid = 0;
                pTcb->tskwait = TTW_NONE;
                pTcb->tskstat = TTS_SUS;
            }
            else
            {
                pTcb->wobjid = 0;
                pTcb->tskwait = TTW_NONE;
                pTcb->tskstat = TTS_RDY;

                _tx_thread_preempt_disable++;   /* Temporarily disable preemption. */
                TX_RESTORE  /* Restore interrupts. */
                _tx_thread_system_resume(&pTcb->thread);
                TX_DISABLE  /* Lockout interrupts. */
            }
        }

        if(pMpf->cmpf.mpf==NULL)
        {
            _kernel_sysmem_release(pMpf->mpfptr);
        }

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
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

