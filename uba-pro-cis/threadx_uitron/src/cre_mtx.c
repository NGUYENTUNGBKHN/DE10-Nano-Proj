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
/*  cre_mtx.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create mutex                                                                */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_mtx                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create mutex(Internal function).                                        */
/*                                                                              */
/* INPUT                                                                        */
/*      mbxid               ID number of the mutex.                             */
/*      pk_cmtx             Pointer to the packet containing the mutex creation */
/*                          information.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_mtx                                                                 */
/*      acre_mtx                                                                */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER __cre_mtx(ID mtxid, T_CMTX* pk_cmtx)
{
    _KERNEL_UITRON_MTX* pMtx = &ID_LIST(_kernel_mtx, mtxid);

    /* Copy initial data. */
    pMtx->cmtx.mtxatr = pk_cmtx->mtxatr;
    pMtx->cmtx.ceilpri = pk_cmtx->ceilpri;

    pMtx->lock.plist = NULL;
    pMtx->lock.data = pMtx;

    pMtx->lock_tsk = NULL;

    pMtx->wtsk = NULL;

    ID_LIST(_kernel_mtx_id, mtxid) = pMtx;

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_mtx                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create mutex.                                                           */
/*                                                                              */
/* INPUT                                                                        */
/*      mbxid               ID number of the mutex.                             */
/*      pk_cmtx             Pointer to the packet containing the mutex creation */
/*                          information.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __cre_mtx           Create mutex(Internal function).                    */
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
ER cre_mtx(ID mtxid, T_CMTX* pk_cmtx)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mtx_maxid, mtxid)
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

    if(ID_LIST(_kernel_mtx_id, mtxid)==NULL)
    {
        rtn = __cre_mtx(mtxid, pk_cmtx);
    }
    else
    {
        rtn = E_OBJ;    /* The object is already registered. */
    }

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable--;   /* Release preemption. */
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

