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
/*  sta_cyc.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Start cyclic handler                                                        */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      sta_cyc                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Start cyclic handler.                                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      cycid               ID number of the cyclic handler.                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_timer_deactivate        Deactivate internal timer                   */
/*      _tx_timer_activate          Activate internal timer                     */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  Before using ThreadX API, it changed so     */
/*                                  that interruption might be enabled.         */
/*  2013/12/06      H. Tousaki      version 2.2m-sp2, Modified comment(s).      */
/*                                  There was a problem that the actual time    */
/*                                  was shorter than the specified time. This   */
/*                                  problem has been corrected.                 */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER sta_cyc(ID cycid)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_CYC* pCyc;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_cyc_maxid, cycid)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pCyc = (_KERNEL_UITRON_CYC*)ID_LIST(_kernel_cyc_id, cycid);
    if(pCyc!=NULL)
    {
        pCyc->call_hdr = TRUE;

        if((pCyc->ccyc.cycatr&TA_PHS)==0)
        {
            /* Non phase mode. */
            TX_RESTORE
            _tx_timer_deactivate(&pCyc->timer);
            TX_DISABLE
            pCyc->lapsed_time = 0;
            pCyc->handler_time = pCyc->ccyc.cyctim+TIC2TIM(1);
            TX_RESTORE
            _tx_timer_activate(&pCyc->timer);
            TX_DISABLE
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

