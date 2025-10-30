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
/*  ref_mtx.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference mutex                                                             */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_mtx                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference mutex.                                                        */
/*                                                                              */
/* INPUT                                                                        */
/*      mtxid               ID number of the mutex.                             */
/*      pk_rmtx             Pointer to the packet returning the mutex state.    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Task context                                                            */
/*      Non-task context                                                        */
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
ER ref_mtx(ID mtxid, T_RMTX* pk_rmtx)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MTX* pMtx;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mtx_maxid, mtxid)
    CHECK_PARAM(pk_rmtx!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pMtx = (_KERNEL_UITRON_MTX*)ID_LIST(_kernel_mtx_id, mtxid);
    if(pMtx!=NULL)
    {
        if(pMtx->lock_tsk!=NULL)
        {
            pk_rmtx->htskid = pMtx->lock_tsk->tskid;
        }
        else
        {
            pk_rmtx->htskid = TSK_NONE;
        }

        if(pMtx->wtsk!=NULL)
        {
            pk_rmtx->wtskid = ((_KERNEL_UITRON_TSK*)pMtx->wtsk->data)->tskid;
        }
        else
        {
            pk_rmtx->wtskid = TSK_NONE;
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

