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
/*  ref_mbf.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference message buffer                                                    */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_mbf                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference data queue.                                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      mbfid               ID number of the message buffer.                    */
/*      pk_rmbf             Pointer to the packet returning the message buffer  */
/*                          state.                                              */
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
ER ref_mbf(ID mbfid, T_RMBF* pk_rmbf)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MBF* pMbf;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mbf_maxid, mbfid)
    CHECK_PARAM(pk_rmbf!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pMbf = (_KERNEL_UITRON_MBF*)ID_LIST(_kernel_mbf_id, mbfid);
    if(pMbf!=NULL)
    {
        if(pMbf->stsk!=NULL)
        {
            pk_rmbf->stskid = ((_KERNEL_UITRON_TSK*)pMbf->stsk->data)->tskid;
        }
        else
        {
            pk_rmbf->stskid = TSK_NONE;
        }
        if(pMbf->rtsk!=NULL)
        {
            pk_rmbf->rtskid = ((_KERNEL_UITRON_TSK*)pMbf->rtsk->data)->tskid;
        }
        else
        {
            pk_rmbf->rtskid = TSK_NONE;
        }

        pk_rmbf->smsgcnt = pMbf->smsgcnt;

        if(0<pMbf->cmbf.mbfsz)
        {
            pk_rmbf->fmbfsz = pMbf->cmbf.mbfsz - pMbf->mbfcnt;
        }
        else
        {
            pk_rmbf->fmbfsz = 0;
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

