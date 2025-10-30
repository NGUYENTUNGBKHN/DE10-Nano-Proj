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
/*  snd_mbx.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Send mailbox                                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_enque_mbx_pri                                                   */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Set priority message to mailbox list.                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      pMppTop             Mailbox control pointer                             */
/*      pNode               Data element to be sent                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2012/06/12      M. Miyashita    version 2.2f, Modified comment(s),          */
/*                                  Since the pointer operation at the time of  */
/*                                  a TA_MPRI attribute was wrong, it corrected.*/
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
void _kernel_enque_mbx_pri(T_MSG_PRI** ppTop, T_MSG_PRI* pNode)
{
    T_MSG_PRI* pPreNode;
    T_MSG_PRI* pFindNode;

    if(*ppTop==NULL)
    {
        /* Insert tail node. */
        pNode->msgque.next = NULL;
        *ppTop = pNode;
    }
    else
    {
        /* pNode is inserted by priority. */
        pFindNode = (T_MSG_PRI*)*ppTop;
        pPreNode = NULL;
        do
        {
            if(pNode->msgpri<pFindNode->msgpri)
            {
                /* pNode is inserted before pFindNode. */
                pNode->msgque.next = (T_MSG*)pFindNode;

                if(pFindNode==*ppTop)
                {
                    *ppTop = pNode;
                }
                else if(pPreNode!=NULL)
                {
                    pPreNode->msgque.next = (T_MSG*)pNode;
                }
                return;
            }
            pPreNode = pFindNode;
            pFindNode = (T_MSG_PRI*)pFindNode->msgque.next;
        } while(pFindNode!=NULL);

        /* pNode is inserted after pPreNode. */
        pPreNode->msgque.next = (T_MSG*)pNode;
        pNode->msgque.next = NULL;
    }
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __snd_mbx                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Send mailbox(Internal function).                                        */
/*                                                                              */
/* INPUT                                                                        */
/*      mbxid               ID number of the mailbox.                           */
/*      pk_msg              Message pointer for send message                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_resume_task         Resume a task with error code               */
/*      _kernel_enque_mbx_pri       Set priority message                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
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
ER __snd_mbx(ID mbxid, T_MSG* pk_msg)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MBX* pMbx;
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    pMbx = (_KERNEL_UITRON_MBX*)ID_LIST(_kernel_mbx_id, mbxid);
    if(pMbx!=NULL)
    {
        if((pMbx->cmbx.mbxatr&TA_MPRI)!=0 &&
            (((T_MSG_PRI*)pk_msg)->msgpri<TMIN_MPRI || pMbx->cmbx.maxmpri<((T_MSG_PRI*)pk_msg)->msgpri))
        {
            rtn = E_PAR;    /* Parameter error */
        }
        else
        {
            if(pMbx->wtsk!=NULL)
            {
                /* Remove from waiting list */
                pTcb = (_KERNEL_UITRON_TSK*)pMbx->wtsk->data;
                _kernel_deque(&pTcb->wait);

                *((T_MSG**)pTcb->waitdata) = pk_msg;

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
                if((pMbx->cmbx.mbxatr&TA_MPRI)!=0)
                {
                    _kernel_enque_mbx_pri((T_MSG_PRI**)&pMbx->mbx, (T_MSG_PRI*)pk_msg);
                }
                else
                {
                    pk_msg->next = NULL;

                    if(pMbx->mbx_tail==NULL)
                    {
                        /* Insert first node. */
                        pMbx->mbx = pk_msg;
                    }
                    else
                    {
                        /* Insert tail node. */
                        pMbx->mbx_tail->next = pk_msg;
                    }

                    /* Update tail node pointer. */
                    pMbx->mbx_tail = pk_msg;
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
/*      snd_mbx                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Send mailbox.                                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      mbxid               ID number of the mailbox.                           */
/*      pk_msg              Message pointer for send message                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __snd_mbx                   Send mailbox(Internal function)             */
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
ER snd_mbx(ID mbxid, T_MSG* pk_msg)
{
    CHECK_ID(1, _kernel_mbx_maxid, mbxid)
    CHECK_PARAM(pk_msg!=NULL)
    CHECK_TSK_CONTEXT()

    return __snd_mbx(mbxid, pk_msg);
}

