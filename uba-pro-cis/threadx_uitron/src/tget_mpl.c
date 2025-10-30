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
/*  tget_mpl.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Get memory from variable sized memory pool with timeout                     */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_byte_pool.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __tget_mpl                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Get memory from variable sized memory pool with timeout.                */
/*      (Internal function)                                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      mplid               ID number of the variable sized memory pool.        */
/*      blksz               Memory block size                                   */
/*      p_blk               Pointer for start address of the memory block.      */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_byte_allocate           Allocate memory block                       */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*      _kernel_suspend_by_api      Suspend task for API                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      get_mpl                                                                 */
/*      pget_mpl                                                                */
/*      tget_mpl                                                                */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Internal                                                                */
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
/*  2010/01/26      M. Miyashita    version 2.2a, Modified comment(s),          */
/*                                  A means to go into a waiting state was      */
/*                                  changed into the form which uses ThreadX API*/
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER __tget_mpl(ID mplid, UINT blksz, VP* p_blk, TMO tmout)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MPL* pMpl;
    _KERNEL_UITRON_TSK* pTcb;
    VOID* memory_ptr;
    _KERNEL_UITRON_MPL_WAIT waitdata;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pMpl = (_KERNEL_UITRON_MPL*)ID_LIST(_kernel_mpl_id, mplid);
    if(pMpl!=NULL)
    {
        TX_RESTORE  /* Restore interrupts. */
        if(_tx_byte_allocate(&pMpl->bpool, &memory_ptr, (ULONG)blksz, TX_NO_WAIT)==TX_SUCCESS)
        {
            TX_DISABLE  /* Lockout interrupts. */
            /* Set pointer. */
            *p_blk = memory_ptr;
        }
        else
        {
            if(tmout!=TMO_POL)
            {
                TX_DISABLE  /* Lockout interrupts. */
                pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
                if(pTcb!=NULL && DISPATCH_ENABLE)
                {
                    /* Wait for semaphore. */
                    if(pMpl->cmpl.mplatr==TA_TFIFO)
                    {
                        _kernel_enque(&pMpl->wtsk, &pTcb->wait);
                    }
                    else
                    {
                        _kernel_enque_task_pri(&pMpl->wtsk, &pTcb->wait);
                    }

                    pTcb->tskstat = TTS_WAI;
                    pTcb->tskwait = TTW_MPL;
                    pTcb->wobjid = mplid;
                    pTcb->wait_obj = pMpl;
                    pTcb->wait_pri = (pMpl->cmpl.mplatr!=TA_TFIFO);

                    waitdata.blksz = blksz;
                    waitdata.p_blk = p_blk;
                    pTcb->waitdata = &waitdata;

                    pTcb->wait_rtn = E_OK;

                    /* Setup cleanup routine pointer.  */
                    pTcb->thread.tx_thread_suspend_cleanup = _kernel_task_cleanup;

                    /* Set the state to suspended.  */
                    pTcb->thread.tx_thread_state = TX_SUSPENDED;

                    /* Set the suspending flag.  */
                    pTcb->thread.tx_thread_suspending =  TX_TRUE;

                    /* Setup the timeout period.  */
                    if(tmout==TMO_FEVR)
                    {
                        pTcb->thread.tx_thread_timer.tx_timer_internal_remaining_ticks = TX_WAIT_FOREVER;
                    }
                    else
                    {
                        if(0<TIM2TIC(tmout))
                        {
                            pTcb->thread.tx_thread_timer.tx_timer_internal_remaining_ticks = TIM2TIC(tmout);
                        }
                        else
                        {
                            pTcb->thread.tx_thread_timer.tx_timer_internal_remaining_ticks = 1;
                        }
                    }

                    _tx_thread_preempt_disable++;   /* Temporarily disable preemption.  */
                    TX_RESTORE  /* Restore interrupts. */

                    _tx_thread_system_suspend(&pTcb->thread);

                    TX_DISABLE  /* Lockout interrupts. */
                    rtn = pTcb->wait_rtn;
                }
                else
                {
                    rtn = E_CTX;    /* No support contexts. */
                }
            }
            else
            {
                TX_DISABLE  /* Lockout interrupts. */
                rtn = E_TMOUT;
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

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      tget_mpl                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Get memory from variable sized memory pool with timeout.                */
/*                                                                              */
/* INPUT                                                                        */
/*      mplid               ID number of the variable sized memory pool.        */
/*      blksz               Memory block size                                   */
/*      p_blk               Pointer for start address of the memory block.      */
/*      tmout               Timeout value                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __tget_mpl                  Get memory from variable sized memory pool. */
/*                                  (Internal function)                         */
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
ER tget_mpl(ID mplid, UINT blksz, VP* p_blk, TMO tmout)
{
    CHECK_ID(1, _kernel_mpl_maxid, mplid)
    CHECK_PARAM(p_blk!=NULL);
    CHECK_TMOUT(tmout)
    CHECK_TSK_CONTEXT()

    return __tget_mpl(mplid, blksz, p_blk, tmout);
}

