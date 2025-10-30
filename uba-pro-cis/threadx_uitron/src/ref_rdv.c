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
/*  ref_rdv.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference rendezvous number                                                 */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_rdv                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference rendezvous number.                                            */
/*                                                                              */
/* INPUT                                                                        */
/*      rdvno               Rendezvous number.                                  */
/*      pk_rrdv             Pointer to the packet returning the rendezvous      */
/*                          number state.                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_rendezvous_find     Find waiting task by rendezvous number      */
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
ER ref_rdv(RDVNO rdvno, T_RRDV* pk_rrdv)
{
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    CHECK_PARAM(pk_rrdv!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    if((pTcb=_kernel_rendezvous_find(_kernel_rendezvous_tsk, rdvno))!=NULL)
    {
        pk_rrdv->wtskid = pTcb->tskid;
    }
    else
    {
        pk_rrdv->wtskid = TSK_NONE;
    }
    TX_RESTORE  /* Restore interrupts. */

    return E_OK;
}

