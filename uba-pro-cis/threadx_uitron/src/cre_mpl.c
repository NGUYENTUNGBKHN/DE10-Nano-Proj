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
/*  cre_mpl.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create variable sized memory pool                                           */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_byte_pool.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_mpl                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create variable sized memory pool(Internal function).                   */
/*                                                                              */
/* INPUT                                                                        */
/*      mplid               ID number of the variable sized memory pool.        */
/*      pk_cmpl             Pointer to the packet containing the variable sized */
/*                          memory pool creation information.                   */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_object_name         Make object name                            */
/*      _kernel_sysmem_alloc        Get system memory                           */
/*      _tx_byte_pool_create        Create byte memory pool                     */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_mpl                                                                 */
/*      acre_mpl                                                                */
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
ER __cre_mpl(ID mplid, T_CMPL* pk_cmpl)
{
    _KERNEL_UITRON_MPL* pMpl = &ID_LIST(_kernel_mpl, mplid);

#ifndef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    _kernel_object_name(pMpl->name, "uiMPL", mplid);
#else
    strcpy(pMpl->name, (const char *)pk_cmpl->name);
#endif

    /* Copy T_CMPL data. */
    pMpl->cmpl.mplatr = pk_cmpl->mplatr;
    pMpl->cmpl.mplsz = pk_cmpl->mplsz;
    pMpl->cmpl.mpl = pk_cmpl->mpl;

    if(pMpl->cmpl.mpl==NULL)
    {
        if((pMpl->mplptr=_kernel_sysmem_alloc(pk_cmpl->mplsz))==NULL)
        {
            return E_NOMEM; /* Memory none */
        }
    }
    else
    {
        pMpl->mplptr = pMpl->cmpl.mpl;
    }

    _tx_byte_pool_create(&pMpl->bpool, pMpl->name, pMpl->mplptr, pk_cmpl->mplsz);

    pMpl->wtsk = NULL;

    ID_LIST(_kernel_mpl_id, mplid) = pMpl;

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_mpl                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create variable sized memory pool.                                      */
/*                                                                              */
/* INPUT                                                                        */
/*      mplid               ID number of the variable sized memory pool.        */
/*      pk_cmpl             Pointer to the packet containing the variable sized */
/*                          memory pool creation information.                   */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_RSATR             Reserved attribute error                            */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_CTX               Context error                                       */
/*      E_OBJ               Object state error                                  */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __cre_mpl           Create variable sized memory pool.                  */
/*                          (Internal function)                                 */
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
ER cre_mpl(ID mplid, T_CMPL* pk_cmpl)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mpl_maxid, mplid)
    CHECK_PARAM(pk_cmpl!=NULL && TX_BYTE_POOL_MIN<=pk_cmpl->mplsz)
    CHECK_ATR(pk_cmpl->mplatr, TA_TPRI)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_mpl_id, mplid)==NULL)
    {
        rtn = __cre_mpl(mplid, pk_cmpl);
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

