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
/*  rel_mpl.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Release memory to variable sized memory pool                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_byte_pool.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      rel_mpl                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Release memory to variable sized memory pool.                           */
/*                                                                              */
/* INPUT                                                                        */
/*      mplid               ID number of the variable sized memory pool.        */
/*      blk                 Start address of the memory block.                  */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_byte_release            Release memory block                        */
/*      _tx_byte_allocate           Allocate memory block                       */
/*      _kernel_resume_task         Resume a task with error code               */
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
/*                                  The preemption control method was arranged. */
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2013/12/06      H. Tousaki      version 2.2m-sp2, Modified comment(s).      */
/*                                  When irel_wai() was called by interrupt     */
/*                                  processing in the process of this function, */
/*                                  an endless loop occurred. This problem has  */
/*                                  been corrected.                             */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER rel_mpl(ID mplid, VP blk)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MPL* pMpl;
    UCHAR *work_ptr;
    TX_BYTE_POOL *pool_ptr;
    _KERNEL_UITRON_TSK* pTcb;
    _KERNEL_UITRON_MPL_WAIT* waitdata;
    VOID* memory_ptr;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mpl_maxid, mplid)
    CHECK_PARAM(blk!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pMpl = (_KERNEL_UITRON_MPL*)ID_LIST(_kernel_mpl_id, mplid);
    if(pMpl!=NULL)
    {
        /* Back off the memory pointer to pickup its header.  */
        work_ptr = (UCHAR*)blk;
        work_ptr = work_ptr - sizeof(UCHAR *) - sizeof(ULONG);

        /* Pickup the pool pointer.  */
        pool_ptr =  (TX_BYTE_POOL *) *((TX_BYTE_POOL **) (work_ptr + sizeof(UCHAR *)));
        if(pool_ptr==&pMpl->bpool)
        {
            TX_RESTORE  /* Restore interrupts. */
            /* Release block. */
            if(_tx_byte_release(blk)==TX_SUCCESS)
            {
                TX_DISABLE  /* Lockout interrupts. */
                if(pMpl->wtsk!=NULL)
                {
                    _tx_thread_preempt_disable++;   /* Disable preemption. */

                    do
                    {
                        pTcb = (_KERNEL_UITRON_TSK*)pMpl->wtsk->data;
                        waitdata = (_KERNEL_UITRON_MPL_WAIT*)pTcb->waitdata;

                        TX_RESTORE  /* Restore interrupts. */
                        if(_tx_byte_allocate(&pMpl->bpool, &memory_ptr,
                                            (ULONG)waitdata->blksz, TX_NO_WAIT)==TX_SUCCESS)
                        {
                            TX_DISABLE  /* Lockout interrupts. */

                            if(pTcb->wait_obj!=pMpl)
                            {
                                TX_RESTORE  /* Restore interrupts. */
                                _tx_byte_release(memory_ptr);
                                TX_DISABLE  /* Lockout interrupts. */

                                continue;
                            }

                            /* Set pointer. */
                            *(waitdata->p_blk) = memory_ptr;

                            /* Remove from waiting list */
                            _kernel_deque(&pTcb->wait);

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
                            TX_DISABLE  /* Lockout interrupts. */
                            break;
                        }
                    } while(pMpl->wtsk!=NULL);

                    /* Remove temporary disable preemption.  */
                    _tx_thread_preempt_disable--;

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
            }
            else
            {
                TX_DISABLE  /* Lockout interrupts. */
                rtn = E_PAR;    /* Unknow memory pool. */
            }
        }
        else
        {
            rtn = E_PAR;    /* Unknow memory pool. */
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

