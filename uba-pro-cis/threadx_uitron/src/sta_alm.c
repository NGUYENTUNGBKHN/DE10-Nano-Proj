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
/*  sta_alm.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Start alarm handler                                                         */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      sta_alm                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Start alarm handler.                                                    */
/*                                                                              */
/* INPUT                                                                        */
/*      almid               ID number of the alarm handler.                     */
/*      almtim              Time of alarm.                                      */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_timer_deactivate        Deactivate internal timer                   */
/*      _tx_timer_change            Change timer count                          */
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
/*  2013/12/09      H. Tousaki      version 2.2m-sp2, Modified comment(s).      */
/*                                  There was a problem that the actual time    */
/*                                  was shorter than the specified time. This   */
/*                                  problem has been corrected.                 */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER sta_alm(ID almid, RELTIM almtim)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_ALM* pAlm;
    ULONG set_tic;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_alm_maxid, almid)
    CHECK_PARAM(0<almtim);

    TX_DISABLE  /* Lockout interrupts. */
    pAlm = (_KERNEL_UITRON_ALM*)ID_LIST(_kernel_alm_id, almid);
    if(pAlm!=NULL)
    {
        set_tic = (ULONG)(almtim*TIC_DENO+TIC_NUME-1)/TIC_NUME+1;

        TX_RESTORE
        _tx_timer_deactivate(&pAlm->timer);
        _tx_timer_change(&pAlm->timer, set_tic, set_tic);
        _tx_timer_activate(&pAlm->timer);
        TX_DISABLE
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

