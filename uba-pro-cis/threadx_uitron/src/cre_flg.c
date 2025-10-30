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
/*  cre_flg.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create event flag                                                           */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_flg                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create event flag(Internal function).                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      flgid               ID number of the event flag.                        */
/*      pk_cflg             Pointer to the packet containing the event flag     */
/*                          creation information.                               */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_CTX               Context error                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_clear_waitlist      Clear waiting task list                     */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_flg                                                                 */
/*      acre_flg                                                                */
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
ER __cre_flg(ID flgid, T_CFLG* pk_cflg)
{
    _KERNEL_UITRON_FLG* pFlg = &ID_LIST(_kernel_flg, flgid);

    /* Copy initial data. */
    pFlg->cflg.flgatr = pk_cflg->flgatr;
    pFlg->cflg.iflgptn = pk_cflg->iflgptn;

    /* Set first data. */
    pFlg->curflgptn = pk_cflg->iflgptn;

    pFlg->wtsk = NULL;

    ID_LIST(_kernel_flg_id, flgid) = pFlg;

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_flg                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create event flag.                                                      */
/*                                                                              */
/* INPUT                                                                        */
/*      flgid               ID number of the event flag.                        */
/*      pk_cflg             Pointer to the packet containing the event flag     */
/*                          creation information.                               */
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
/*      __cre_flg           Create event flag(Internal function).               */
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
ER cre_flg(ID flgid, T_CFLG* pk_cflg)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_flg_maxid, flgid)
    CHECK_PARAM(pk_cflg!=NULL)
    CHECK_ATR(pk_cflg->flgatr, TA_TFIFO|TA_TPRI|TA_WSGL|TA_WMUL|TA_CLR)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_flg_id, flgid)==NULL)
    {
        rtn = __cre_flg(flgid, pk_cflg);
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

