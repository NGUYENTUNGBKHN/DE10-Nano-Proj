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
/*  ref_cyc.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference cyclic handler                                                    */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_cyc                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference cyclic handler.                                               */
/*                                                                              */
/* INPUT                                                                        */
/*      cycid               ID number of the cyclic handler.                    */
/*      pk_ralm             Pointer to the packet returning the cyclic handler  */
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
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Add a computation expression of             */
/*                                  pk_rcyc->lefttim.                           */
/*                                                                              */
/********************************************************************************/
ER ref_cyc(ID cycid, T_RCYC* pk_rcyc)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_CYC* pCyc;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_cyc_maxid, cycid)
    CHECK_PARAM(pk_rcyc!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pCyc = (_KERNEL_UITRON_CYC*)ID_LIST(_kernel_cyc_id, cycid);
    if(pCyc!=NULL)
    {
        pk_rcyc->cycstat = (pCyc->call_hdr)?TCYC_STA:TCYC_STP;

        pk_rcyc->lefttim = 0;
        if(pCyc->call_hdr)
        {
            if(pCyc->lapsed_time<pCyc->handler_time)
            {
                pk_rcyc->lefttim = pCyc->handler_time - pCyc->lapsed_time;
            }
            else
            {
                pk_rcyc->lefttim = pCyc->ccyc.cyctim;
            }
        }

        if(pk_rcyc->lefttim>TIC2TIM(1))
        {
             pk_rcyc->lefttim = pk_rcyc->lefttim-TIC2TIM(1);
        }
        else
        {
             pk_rcyc->lefttim = 0;
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

