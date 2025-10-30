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
/*  fsnd_dtq.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Forced send data queue                                                      */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __fsnd_dtq                                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Forced send data queue(Internal function).                              */
/*                                                                              */
/* INPUT                                                                        */
/*      dtqid               ID number of the data queue.                        */
/*      data                Data element to be sent                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_ILUSE             Illegal use                                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_resume_task         Resume a task with error code               */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER __fsnd_dtq(ID dtqid, VP_INT data)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_DTQ* pDtq;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pDtq = (_KERNEL_UITRON_DTQ*)ID_LIST(_kernel_dtq_id, dtqid);
    if(pDtq!=NULL)
    {
        if(pDtq->cdtq.dtqcnt==0)
        {
            rtn = E_ILUSE;  /* Illegal use */
        }
        else
        {
            if(pDtq->rtsk!=NULL)
            {
                /* Remove from waiting list */
                pTcb = (_KERNEL_UITRON_TSK*)pDtq->rtsk->data;
                _kernel_deque(&pTcb->wait);

                *((VP_INT*)pTcb->waitdata) = data;

                pTcb->wait_rtn = E_OK;
                pTcb->wait_obj = NULL;

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
                /* Set data to queue. */
                if(pDtq->dtqcnt<pDtq->cdtq.dtqcnt)
                {
                    pDtq->dtqptr[pDtq->setpos] = data;
                    pDtq->dtqcnt++;
                    pDtq->setpos = (pDtq->setpos+1)%pDtq->cdtq.dtqcnt;
                }
                else
                {
                    pDtq->dtqptr[pDtq->setpos] = data;
                    pDtq->setpos = (pDtq->setpos+1)%pDtq->cdtq.dtqcnt;

                    /* Remove top of a data */
                    pDtq->getpos = (pDtq->getpos+1)%pDtq->cdtq.dtqcnt;
                }
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
/*      fsnd_dtq                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Forced send data queue.                                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      dtqid               ID number of the data queue.                        */
/*      data                Data element to be sent                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_ILUSE             Illegal use                                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __fsnd_dtq                  Forced send data queue(Internal function).  */
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
ER fsnd_dtq(ID dtqid, VP_INT data)
{
    CHECK_ID(1, _kernel_dtq_maxid, dtqid)
    CHECK_TSK_CONTEXT()

    return __fsnd_dtq(dtqid, data);
}

