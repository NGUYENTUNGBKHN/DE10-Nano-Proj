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
/*  cre_alm.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create alarm handler                                                        */
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
/*      _tx_timer_deactivate        Timer stop                                  */
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
static VOID _kernel_alm_entry(ULONG timer_input)
{
    _KERNEL_UITRON_ALM* pAlm = (_KERNEL_UITRON_ALM*)timer_input;

    /* Stop this timer. */
    _tx_timer_deactivate(&pAlm->timer);

    /* This user handler is called. */
    ((void (*)(VP_INT))pAlm->calm.almhdr)(pAlm->calm.exinf);
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_alm                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create alarm handler(Internal function).                                */
/*                                                                              */
/* INPUT                                                                        */
/*      almid               ID number of the alarm handler.                     */
/*      pk_calm             Pointer to the packet containing the alarm handler  */
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
/*      cre_alm                                                                 */
/*      acre_alm                                                                */
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
ER __cre_alm(ID almid, T_CALM* pk_calm)
{
    _KERNEL_UITRON_ALM* pAlm = &ID_LIST(_kernel_alm, almid);

#ifndef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    _kernel_object_name(pAlm->name, "uiALM", almid);
#else
    strcpy(pAlm->name, (const char *)pk_calm->name);
#endif

    /* Copy T_CALM data. */
    pAlm->calm.almatr = pk_calm->almatr;
    pAlm->calm.exinf = pk_calm->exinf;
    pAlm->calm.almhdr = pk_calm->almhdr;

    ID_LIST(_kernel_alm_id, almid) = pAlm;

    _tx_timer_create(&pAlm->timer, pAlm->name,
                    _kernel_alm_entry, (ULONG)pAlm, 1, 1,
                    TX_NO_ACTIVATE);

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_alm                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create alarm handler.                                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      almid               ID number of the alarm handler.                     */
/*      pk_calm             Pointer to the packet containing the alarm handler  */
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
/*      __cre_alm           Create alarm handler(Internal function).            */
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
ER cre_alm(ID almid, T_CALM* pk_calm)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_alm_maxid, almid)
    CHECK_PARAM(pk_calm!=NULL && pk_calm->almhdr!=NULL)
    CHECK_ATR(pk_calm->almatr, TA_ASM)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_alm_id, almid)==NULL)
    {
        rtn = __cre_alm(almid, pk_calm);
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

