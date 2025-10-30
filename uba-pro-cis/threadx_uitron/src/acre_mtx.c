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
/*  acre_mtx.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create mutex(Auto ID)                                                       */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      acre_mtx                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create mutex(ID number automatic assignment)                            */
/*                                                                              */
/* INPUT                                                                        */
/*      pk_cmtx             Pointer to the packet containing the mutex creation */
/*                          information.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_NOID              No ID number available                              */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_object_find         Find free object index                      */
/*      __cre_mtx                   Create mutex(Internal function).            */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Initialization                                                          */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER_ID acre_mtx(T_CMTX* pk_cmtx)
{
    ID mtxid;
    ER_ID rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_PARAM(pk_cmtx!=NULL)
#ifndef TX_DISABLE_ERROR_CHECKING
    switch(pk_cmtx->mtxatr)
    {
    case TA_TFIFO:
    case TA_TPRI:
    case TA_INHERIT:
        break;

    case TA_CEILING:
        CHECK_PRIORITY(pk_cmtx->ceilpri);
        break;

    default:
        /* Reserved attribute error */
        /* EXIT OF FUNCTION */
        return E_RSATR;
    }
#endif
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if((mtxid=_kernel_object_find(_kernel_mtx_id, _kernel_mtx_maxid))!=0)
    {
        if((rtn=(ER_ID)__cre_mtx(mtxid, pk_cmtx))==E_OK)
        {
            rtn = (ER_ID)mtxid;
        }
    }
    else
    {
        rtn = (ER_ID)E_NOID;    /* No ID number available. */
    }

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable--;   /* Release preemption. */
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

