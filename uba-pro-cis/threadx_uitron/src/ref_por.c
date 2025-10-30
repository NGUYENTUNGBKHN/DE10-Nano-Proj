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
/*  ref_por.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference rendezvous port                                                   */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_por                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference rendezvous port.                                              */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      pk_rpor             Pointer to the packet returning the rendezvous port */
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
ER ref_por(ID porid, T_RPOR* pk_rpor)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_POR* pPor;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_por_maxid, porid)
    CHECK_PARAM(pk_rpor!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pPor = (_KERNEL_UITRON_POR*)ID_LIST(_kernel_por_id, porid);
    if(pPor!=NULL)
    {
        if(pPor->ctsk!=NULL)
        {
            pk_rpor->ctskid = ((_KERNEL_UITRON_TSK*)pPor->ctsk->data)->tskid;
        }
        else
        {
            pk_rpor->ctskid = TSK_NONE;
        }
        if(pPor->atsk!=NULL)
        {
            pk_rpor->atskid = ((_KERNEL_UITRON_TSK*)pPor->atsk->data)->tskid;
        }
        else
        {
            pk_rpor->atskid = TSK_NONE;
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

