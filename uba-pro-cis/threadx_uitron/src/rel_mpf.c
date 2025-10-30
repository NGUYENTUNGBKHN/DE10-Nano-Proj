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
/*  rel_mpf.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Release memory to fixed sized memory pool                                   */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_block_pool.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      rel_mpf                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Release memory to fixed sized memory pool.                              */
/*                                                                              */
/* INPUT                                                                        */
/*      mpfid               ID number of the fixed sized memory pool.           */
/*      blk                 Start address of the memory block.                  */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_resume_task         Resume a task with error code               */
/*      _tx_block_release           Release memory block                        */
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
/*                                  The preemption control method was arranged. */
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER rel_mpf(ID mpfid, VP blk)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MPF* pMpf;
    _KERNEL_UITRON_TSK* pTcb;

    TX_BLOCK_POOL *pool_ptr;
    UCHAR *work_ptr;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mpf_maxid, mpfid)
    CHECK_PARAM(blk!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pMpf = (_KERNEL_UITRON_MPF*)ID_LIST(_kernel_mpf_id, mpfid);
    if(pMpf!=NULL)
    {
        /* Pickup the pool pointer which is just previous to the starting 
        address of the block that the caller sees.  */
        work_ptr =  ((UCHAR *) blk) - sizeof(UCHAR *);
        pool_ptr =  *((TX_BLOCK_POOL **) work_ptr);
        if(pool_ptr!=&pMpf->bpool)
        {
            rtn = E_PAR;    /* Unknow memory pool. */
        }
        else
        {
            if(pMpf->wtsk!=NULL)
            {
                /* Remove from waiting list */
                pTcb = (_KERNEL_UITRON_TSK*)pMpf->wtsk->data;
                _kernel_deque(&pTcb->wait);

                *((VP*)pTcb->waitdata) = blk;

                pTcb->wait_rtn = E_OK;
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
            else
            {
                TX_RESTORE  /* Restore interrupts. */
                _tx_block_release(blk);
                TX_DISABLE  /* Lockout interrupts. */
            }
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

