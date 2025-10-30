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
/*  cre_cyc.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create cyclic handler                                                       */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_alm_entry                                                       */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Entry function of the alarm handler                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      timer_input                                                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      User handler                                                            */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Timer handler                                                           */
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
static VOID _kernel_cyc_entry(ULONG timer_input)
{
    _KERNEL_UITRON_CYC* pCyc = (_KERNEL_UITRON_CYC*)timer_input;

    pCyc->lapsed_time += TIC2TIM(1);

    while(pCyc->handler_time<=pCyc->lapsed_time)
    {
        if(pCyc->call_hdr)
        {
            /* This user handler is called. */
            ((void (*)(VP_INT))pCyc->ccyc.cychdr)(pCyc->ccyc.exinf);
        }
        pCyc->lapsed_time -= pCyc->handler_time;
        pCyc->handler_time = pCyc->ccyc.cyctim;
    }
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_cyc                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create cyclic handler(Internal function).                               */
/*                                                                              */
/* INPUT                                                                        */
/*      cycid               ID number of the cyclic handler.                    */
/*      pk_ccyc             Pointer to the packet containing the cyclic handler */
/*                          creation information.                               */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_object_name         Make object name                            */
/*      _tx_timer_create            Create internal timer                       */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_cyc                                                                 */
/*      acre_cyc                                                                */
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
/*  2013/12/06      H. Tousaki      version 2.2m-sp2, Modified comment(s).      */
/*                                  There was a problem that the actual time    */
/*                                  was shorter than the specified time. This   */
/*                                  problem has been corrected.                 */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER __cre_cyc(ID cycid, T_CCYC* pk_ccyc)
{
    _KERNEL_UITRON_CYC* pCyc = &ID_LIST(_kernel_cyc, cycid);

#ifndef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    _kernel_object_name(pCyc->name, "uiCYC", cycid);
#else
    strcpy(pCyc->name, (const char *)pk_ccyc->name);
#endif

    /* Copy T_CCYC data. */
    pCyc->ccyc.cycatr = pk_ccyc->cycatr;
    pCyc->ccyc.exinf = pk_ccyc->exinf;
    pCyc->ccyc.cychdr = pk_ccyc->cychdr;
    pCyc->ccyc.cyctim = pk_ccyc->cyctim;
    pCyc->ccyc.cycphs = pk_ccyc->cycphs;

    pCyc->call_hdr = (pk_ccyc->cycatr&TA_STA)!=0;

    pCyc->handler_time = pk_ccyc->cycphs+TIC2TIM(1);
    pCyc->lapsed_time = 0;

    ID_LIST(_kernel_cyc_id, cycid) = pCyc;

    _tx_timer_create(&pCyc->timer, pCyc->name,
                    _kernel_cyc_entry, (ULONG)pCyc, 1, 1,
                    ((pk_ccyc->cycatr&(TA_STA|TA_PHS))!=0)?TX_AUTO_ACTIVATE:TX_NO_ACTIVATE);

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_cyc                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create cyclic handler.                                                  */
/*                                                                              */
/* INPUT                                                                        */
/*      cycid               ID number of the cyclic handler.                    */
/*      pk_ccyc             Pointer to the packet containing the cyclic handler */
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
/*      __cre_cyc           Create cyclic handler(Internal function).           */
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
ER cre_cyc(ID cycid, T_CCYC* pk_ccyc)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_cyc_maxid, cycid)
    CHECK_PARAM(pk_ccyc!=NULL && pk_ccyc->cychdr!=NULL &&
                pk_ccyc->cycphs<=pk_ccyc->cyctim && pk_ccyc->cyctim!=0)
    CHECK_ATR(pk_ccyc->cycatr, TA_ASM|TA_STA|TA_PHS)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_cyc_id, cycid)==NULL)
    {
        rtn = __cre_cyc(cycid, pk_ccyc);
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

