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
/*  cre_mpf.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create fixed sized memory pool                                              */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_block_pool.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_mpf                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create fixed sized memory pool(Internal function).                      */
/*                                                                              */
/* INPUT                                                                        */
/*      mpfid               ID number of the fixed sized memory pool.           */
/*      pk_cmpf             Pointer to the packet containing the fixed sized    */
/*                          memory pool creation information.                   */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_object_name         Make object name                            */
/*      _kernel_sysmem_alloc        Get system memory                           */
/*      _tx_block_pool_create       Create block memory pool                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_mpf                                                                 */
/*      acre_mpf                                                                */
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
ER __cre_mpf(ID mpfid, T_CMPF* pk_cmpf)
{
    _KERNEL_UITRON_MPF* pMpf = &ID_LIST(_kernel_mpf, mpfid);

#ifndef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    _kernel_object_name(pMpf->name, "uiMPF", mpfid);
#else
    strcpy(pMpf->name, (const char *)pk_cmpf->name);
#endif

    /* Copy T_CMPF data. */
    pMpf->cmpf.mpfatr = pk_cmpf->mpfatr;
    pMpf->cmpf.blkcnt = pk_cmpf->blkcnt;
    pMpf->cmpf.blksz = pk_cmpf->blksz;
    pMpf->cmpf.mpf = pk_cmpf->mpf;

    if(pMpf->cmpf.mpf==NULL)
    {
        if((pMpf->mpfptr=_kernel_sysmem_alloc(TSZ_MPF(pk_cmpf->blkcnt, pk_cmpf->blksz)))==NULL)
        {
            return E_NOMEM; /* Memory none */
        }
    }
    else
    {
        pMpf->mpfptr = pMpf->cmpf.mpf;
    }

    _tx_block_pool_create(&pMpf->bpool, pMpf->name, (ULONG)pk_cmpf->blksz,
                        pMpf->mpfptr, TSZ_MPF(pk_cmpf->blkcnt, pk_cmpf->blksz));

    pMpf->wtsk = NULL;

    ID_LIST(_kernel_mpf_id, mpfid) = pMpf;

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_mpf                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create fixed sized memory pool.                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      mpfid               ID number of the fixed sized memory pool.           */
/*      pk_cmpf             Pointer to the packet containing the fixed sized    */
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
/*      __cre_mpf           Create fixed sized memory pool(Internal function).  */
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
ER cre_mpf(ID mpfid, T_CMPF* pk_cmpf)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mpf_maxid, mpfid)
    CHECK_PARAM(pk_cmpf!=NULL && 0<pk_cmpf->blkcnt && 0<pk_cmpf->blksz)
    CHECK_ATR(pk_cmpf->mpfatr, TA_TPRI)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_mpf_id, mpfid)==NULL)
    {
        rtn = __cre_mpf(mpfid, pk_cmpf);
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

