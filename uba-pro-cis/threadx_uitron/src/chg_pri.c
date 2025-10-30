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
/*  chg_pri.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Change priority                                                             */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_mutex.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_ceiling_check                                                   */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Priority check of ceiling mutex                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      pTcb                Task control block                                  */
/*      tskpri              Task priority                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      TRUE                This priority can be used.                          */
/*      FALSE               This priority cannot be used.                       */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tskwait        Get task waiting factor                     */
/*                                                                              */
/* CALLED BY                                                                    */
/*      chg_pri                                                                 */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      chg_pri                                                                 */
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
static BOOL _kernel_ceiling_check(_KERNEL_UITRON_TSK* pTcb, PRI tskpri)
{
    _KERNEL_UITRON_OBJ* pFindNode;
    _KERNEL_UITRON_MTX* pMtx;

    pFindNode = pTcb->locked_mutex;
    if(pFindNode!=NULL)
    {
        do
        {
            pMtx = (_KERNEL_UITRON_MTX*)pFindNode->data;
            if(pMtx->cmtx.mtxatr==TA_CEILING && tskpri<pMtx->cmtx.ceilpri)
            {
                return FALSE;   /* locked mutex error */
            }
            pFindNode = pFindNode->next;
        } while(pFindNode!=(_KERNEL_UITRON_OBJ*)pTcb->locked_mutex);
    }

    if(_kernel_task_tskwait(pTcb)==TTW_MTX)
    {
        pMtx = (_KERNEL_UITRON_MTX*)pTcb->wait_obj;
        if(pMtx->cmtx.mtxatr==TA_CEILING && tskpri<pMtx->cmtx.ceilpri)
        {
            return FALSE;   /* locked mutex error */
        }
    }

    return TRUE;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      chg_pri                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Change task priority                                                    */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               Task ID                                             */
/*      tskpri              Task priority                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_NOEXS             Non-existent object                                 */
/*      E_ILUSE             Illegal use                                         */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*      _kernel_ceiling_check       Ceiling check                               */
/*      _tx_mutex_priority_change   Priority change for mutex                   */
/*      tx_thread_priority_change   Thread priority change                      */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Renamed some definitions.                   */
/*                                                                              */
/********************************************************************************/
ER chg_pri(ID tskid, PRI tskpri)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
    UINT old_priority;
    STAT tskwait;
    _KERNEL_UITRON_MTX* pMtx;
    _KERNEL_UITRON_OBJ** plist; /* Control list */
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    if(tskpri!=TPRI_INI)
    {
        CHECK_PRIORITY(tskpri)
    }
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
        if(tskpri==TPRI_INI)
        {
            tskpri = pTcb->ctsk.itskpri;
        }

        if(pTcb->tskstat!=TTS_DMT)
        {
            /* Check ceiling mutex information. */
            if(!_kernel_ceiling_check(pTcb, tskpri))
            {
                rtn = E_ILUSE;  /* Illegal use */
            }
            else
            {
                tskwait = _kernel_task_tskwait(pTcb);
                if(tskwait==TTW_MTX)
                {
                    /* Check locked task priority. */
                    pMtx = (_KERNEL_UITRON_MTX*)pTcb->wait_obj;
                    if(pMtx->cmtx.mtxatr==TA_INHERIT)
                    {
                        if((UINT)tskpri<pMtx->lock_tsk->thread.tx_thread_priority)
                        {
                            /* Change to inherit priority of locked task priority. */
                            _tx_thread_preempt_disable++;   /* Disable preemption. */
                            TX_RESTORE
#if defined(THREADX_MAJOR_VERSION) && defined(THREADX_MINOR_VERSION)
#if 5<=THREADX_MAJOR_VERSION && 5<=THREADX_MINOR_VERSION
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    (UINT)tskpri);
#else
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    (UINT)tskpri, (UINT)tskpri);
#endif
#else
                            _tx_mutex_priority_change(&pMtx->lock_tsk->thread,
                                    (UINT)tskpri, (UINT)tskpri);
#endif
                            TX_DISABLE
                            _tx_thread_preempt_disable--;   /* Release preemption. */
                        }

                    }
                }

                pTcb->tskbpri = tskpri; /* Update base priority */

                if(tskwait!=TTW_NONE && pTcb->wait_pri)
                {
                    /* Rearrangement of the waiting list of the order of a priority */
                    plist = pTcb->wait.plist;
                    _kernel_deque(&pTcb->wait);
                    _kernel_enque_task_pri(plist, &pTcb->wait);
                }

                if(pTcb->locked_mutex==NULL || (UINT)tskpri<pTcb->thread.tx_thread_priority)
                {
                    TX_RESTORE
                    pTcb->thread.tx_thread_inherit_priority = (UINT)tskpri;
                    _tx_thread_priority_change(&pTcb->thread, (UINT)tskpri, &old_priority);
                    TX_DISABLE
                }
            }
        }
        else
        {
            rtn = E_OBJ;    /* Object error */
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

