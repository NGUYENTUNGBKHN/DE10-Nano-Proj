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
/*  rpl_rdv.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Forward rendezvous                                                          */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      rpl_rdv                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Terminate rendezvous.                                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      rdvno               Rendezvous number for forward                       */
/*      msg                 Data pointer for return message                     */
/*      rmsgsz              Data size for return message                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_TMOUT             Timeout error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_rendezvous_find     Find waiting task by rendezvous number      */
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
ER rpl_rdv(RDVNO rdvno, VP msg, UINT rmsgsz)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pRdvnoTcb;
    _KERNEL_UITRON_POR_WAIT* pwait;
TX_INTERRUPT_SAVE_AREA

    CHECK_PARAM(0<rmsgsz && msg!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    if((pRdvnoTcb=_kernel_rendezvous_find(_kernel_rendezvous_tsk, rdvno))!=NULL)
    {
        pwait = (_KERNEL_UITRON_POR_WAIT*)pRdvnoTcb->waitdata;
        if(rmsgsz<=pwait->msgsz)
        {
            /* Remove from waiting list */
            _kernel_deque(&pRdvnoTcb->wait);

            _kernel_copy((char*)pwait->msg, (char*)msg, rmsgsz);
            pwait->msgsz = rmsgsz;

            pRdvnoTcb->wait_rtn = E_OK;
            pRdvnoTcb->wait_obj = NULL;
            pRdvnoTcb->wait_pri = FALSE;

            /* Clear cleanup routine pointer. */
            pRdvnoTcb->thread.tx_thread_suspend_cleanup = TX_NULL;

            if(pRdvnoTcb->tskstat==TTS_WAS)
            {
                pRdvnoTcb->wobjid = 0;
                pRdvnoTcb->tskwait = TTW_NONE;
                pRdvnoTcb->tskstat = TTS_SUS;
            }
            else
            {
                pRdvnoTcb->wobjid = 0;
                pRdvnoTcb->tskwait = TTW_NONE;
                pRdvnoTcb->tskstat = TTS_RDY;

                _tx_thread_preempt_disable++;   /* Temporarily disable preemption. */
                TX_RESTORE  /* Restore interrupts. */
                _tx_thread_system_resume(&pRdvnoTcb->thread);
                TX_DISABLE  /* Lockout interrupts. */
            }
        }
        else
        {
            rtn = E_PAR;    /* Parameter error */
        }
    }
    else
    {
        rtn = E_OBJ;    /* Object error */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

