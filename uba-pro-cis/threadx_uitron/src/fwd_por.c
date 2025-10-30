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
/*  fwd_por.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Forward rendezvous                                                          */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      fwd_por                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Forward rendezvous.                                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      calptn              Calling bit pattern.                                */
/*      rdvno               Rendezvous number for forward                       */
/*      msg                 Data pointer to be call                             */
/*      cmsgsz              Data size for call                                  */
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
/*      _kernel_rendezvous_find     Find waiting task by rendezvous number      */
/*      _kernel_rendezvous_pattern  Find waiting task by pattern                */
/*      _kernel_rendezvous_number   Get new rendezvous number                   */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  If there is a waiting for the task, add the */
/*                                  process to clear the pTcb->wait_pri.        */
/*                                                                              */
/********************************************************************************/
ER fwd_por(ID porid, RDVPTN calptn, RDVNO rdvno, VP msg, UINT cmsgsz)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_POR* pPor;
    _KERNEL_UITRON_TSK* pRdvnoTcb;
    _KERNEL_UITRON_TSK* pAcceptTcb;
    _KERNEL_UITRON_POR_WAIT* pwait;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_por_maxid, porid)
    CHECK_PARAM(calptn!=0 && rdvno!=0 && !(0<cmsgsz && msg==NULL))
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pPor = (_KERNEL_UITRON_POR*)ID_LIST(_kernel_por_id, porid);
    if(pPor!=NULL)
    {
        if(cmsgsz<=pPor->cpor.maxcmsz)
        {
            if((pRdvnoTcb=_kernel_rendezvous_find(_kernel_rendezvous_tsk, rdvno))!=NULL)
            {
                pwait = (_KERNEL_UITRON_POR_WAIT*)pRdvnoTcb->waitdata;
                if(pPor->cpor.maxrmsz<=pwait->msgsz && cmsgsz<=pwait->msgsz)
                {
                    if((pAcceptTcb=_kernel_rendezvous_pattern(pPor->atsk, calptn))!=NULL)
                    {
                        /* Remove from waiting list */
                        _kernel_deque(&pAcceptTcb->wait);
                        pwait = (_KERNEL_UITRON_POR_WAIT*)pAcceptTcb->waitdata;

                        _kernel_copy((char*)pwait->msg, (char*)msg, cmsgsz);
                        pwait->msgsz = cmsgsz;
                        rdvno = _kernel_rendezvous_number(pRdvnoTcb);
                        pwait->rdvno = rdvno;

                        /* Set new rendezvous number */
                        pwait = (_KERNEL_UITRON_POR_WAIT*)pRdvnoTcb->waitdata;
                        pwait->rdvno = rdvno;


                        pAcceptTcb->wait_rtn = E_OK;
                        pAcceptTcb->wait_obj = NULL;
                        pAcceptTcb->wait_pri = FALSE;

                        /* Clear cleanup routine pointer. */
                        pAcceptTcb->thread.tx_thread_suspend_cleanup = TX_NULL;

                        if(pAcceptTcb->tskstat==TTS_WAS)
                        {
                            pAcceptTcb->wobjid = 0;
                            pAcceptTcb->tskwait = TTW_NONE;
                            pAcceptTcb->tskstat = TTS_SUS;
                        }
                        else
                        {
                            pAcceptTcb->wobjid = 0;
                            pAcceptTcb->tskwait = TTW_NONE;
                            pAcceptTcb->tskstat = TTS_RDY;

                            _tx_thread_preempt_disable++;   /* Temporarily disable preemption. */
                            TX_RESTORE  /* Restore interrupts. */
                            _tx_thread_system_resume(&pAcceptTcb->thread);
                            TX_DISABLE  /* Lockout interrupts. */
                        }
                    }
                    else
                    {
                        /* set data for rendezvous calling */
                        _kernel_copy((char*)pwait->msg, (char*)msg, cmsgsz);
                        pwait->msgsz = cmsgsz;

                        /* Remove from waiting list */
                        _kernel_deque(&pRdvnoTcb->wait);

                        /* Wait for call rendezvous. */
                        if(pPor->cpor.poratr==TA_TFIFO)
                        {
                            _kernel_enque(&pPor->ctsk, &pRdvnoTcb->wait);
                        }
                        else
                        {
                            _kernel_enque_task_pri(&pPor->ctsk, &pRdvnoTcb->wait);
                        }

                        pRdvnoTcb->tskwait = TTW_CAL;
                        pRdvnoTcb->wobjid = porid;
                        pRdvnoTcb->wait_obj = pPor;
                        pRdvnoTcb->wait_pri = (pPor->cpor.poratr!=TA_TFIFO);
                    }
                }
                else
                {
                    rtn = E_ILUSE;  /* Illegal use error */
                }
            }
            else
            {
                rtn = E_OBJ;    /* Object error */
            }
        }
        else
        {
            rtn = E_PAR;    /* Parameter error */
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

