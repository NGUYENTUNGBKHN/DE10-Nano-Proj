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
/*  cre_mbf.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Create message buffer                                                       */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __kernel_copy_to_mbf                                                    */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Copy to message buffer.                                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      pMbf                Pointer of message buffer control block             */
/*      src                 Pointer of source message                           */
/*      msgsz               Source message size                                 */
/*                                                                              */
/* OUTPUT                                                                       */
/*                                                                              */
/* CALLS                                                                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      trcv_mbf                                                                */
/*      tsnd_mbf                                                                */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  The data type was corrected.                */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
UINT __kernel_copy_to_mbf(_KERNEL_UITRON_MBF* pMbf, char* src, UINT msgsz)
{
    char* p_msgsz = (char*)&msgsz;
    UINT cnt = sizeof(UINT);

    if((pMbf->mbfcnt+sizeof(UINT)+msgsz)<=pMbf->cmbf.mbfsz)
    {
        /* Copy message size */
        while(0<cnt)
        {
            pMbf->mbfptr[pMbf->setpos] = *p_msgsz;
            pMbf->setpos = (pMbf->setpos+1)%(UINT)pMbf->cmbf.mbfsz;

            p_msgsz++;
            cnt--;
        }

        /* Copy message */
        for(; cnt<msgsz; cnt++)
        {
            pMbf->mbfptr[pMbf->setpos] = *src;
            pMbf->setpos = (pMbf->setpos+1)%(UINT)pMbf->cmbf.mbfsz;

            src++;
        }

        pMbf->smsgcnt++;
        pMbf->mbfcnt += (UINT)sizeof(UINT)+msgsz;

        return msgsz;
    }

    return 0;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __kernel_copy_from_mbf                                                  */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Copy from message buffer.                                               */
/*                                                                              */
/* INPUT                                                                        */
/*      pMbf                Pointer of message buffer control block             */
/*      dst                 Pointer of destination message                      */
/*                                                                              */
/* OUTPUT                                                                       */
/*      copied length                                                           */
/*                                                                              */
/* CALLS                                                                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      trcv_mbf                                                                */
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
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  The data type was corrected.                */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
UINT __kernel_copy_from_mbf(_KERNEL_UITRON_MBF* pMbf, char* dst)
{
    UINT msgsz;
    char* p_msgsz = (char*)&msgsz;
    UINT cnt = sizeof(UINT);

    if(sizeof(UINT)<pMbf->mbfcnt)
    {
        /* Copy message size */
        while(0<cnt)
        {
            *p_msgsz = pMbf->mbfptr[pMbf->getpos];
            pMbf->getpos = (pMbf->getpos+1)%(UINT)pMbf->cmbf.mbfsz;

            p_msgsz++;
            cnt--;
        }

        /* Copy message */
        for(; cnt<msgsz; cnt++)
        {
            *dst = pMbf->mbfptr[pMbf->getpos];
            pMbf->getpos = (pMbf->getpos+1)%(UINT)pMbf->cmbf.mbfsz;

            dst++;
        }

        pMbf->mbfcnt -= (UINT)sizeof(UINT)+msgsz;
        pMbf->smsgcnt--;

        return msgsz;
    }

    return 0;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      __cre_mbf                                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create message buffer(Internal function).                               */
/*                                                                              */
/* INPUT                                                                        */
/*      mbfid               ID number of the message buffer.                    */
/*      pk_cdtq             Pointer to the packet containing the message buffer */
/*                          creation information.                               */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_NOMEM             Insufficient memory                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_sysmem_alloc        Get system memory                           */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_mbf                                                                 */
/*      acre_mbf                                                                */
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
ER __cre_mbf(ID mbfid, T_CMBF* pk_cmbf)
{
    _KERNEL_UITRON_MBF* pMbf = &ID_LIST(_kernel_mbf, mbfid);

    /* Copy T_CMBF data. */
    pMbf->cmbf.mbfatr = pk_cmbf->mbfatr;
    pMbf->cmbf.maxmsz = pk_cmbf->maxmsz;
    pMbf->cmbf.mbfsz = pk_cmbf->mbfsz;
    pMbf->cmbf.mbf = pk_cmbf->mbf;

    if(pMbf->cmbf.mbf==NULL)
    {
        if(0<pk_cmbf->mbfsz)
        {
            if((pMbf->mbfptr=(char*)_kernel_sysmem_alloc(pk_cmbf->mbfsz))==NULL)
            {
                return E_NOMEM; /* Memory none */
            }
        }
    }
    else
    {
        pMbf->mbfptr = (char*)pMbf->cmbf.mbf;
    }

    pMbf->setpos = 0;
    pMbf->getpos = 0;
    pMbf->smsgcnt = 0;
    pMbf->mbfcnt = 0;

    pMbf->rtsk = NULL;
    pMbf->stsk = NULL;

    ID_LIST(_kernel_mbf_id, mbfid) = pMbf;

    return E_OK;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cre_mbf                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create message buffer.                                                  */
/*                                                                              */
/* INPUT                                                                        */
/*      mbfid               ID number of the message buffer.                    */
/*      pk_cmbf             Pointer to the packet containing the message buffer */
/*                          creation information.                               */
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
/*      __cre_mbf           Create message buffer(Internal function).           */
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
ER cre_mbf(ID mbfid, T_CMBF* pk_cmbf)
{
    ER rtn;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mbf_maxid, mbfid)
    CHECK_PARAM(pk_cmbf!=NULL)
    CHECK_PARAM(0<pk_cmbf->maxmsz)
    CHECK_PARAM((pk_cmbf->mbfsz==0) || (0<pk_cmbf->mbfsz && TSZ_MBF(1,pk_cmbf->maxmsz)<=pk_cmbf->mbfsz))
    CHECK_ATR(pk_cmbf->mbfatr, TA_TPRI|TA_TFIFO)
    CHECK_TSK_INIT_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    _tx_thread_preempt_disable++;   /* Disable preemption. */
    TX_RESTORE  /* Restore interrupts. */

    if(ID_LIST(_kernel_mbf_id, mbfid)==NULL)
    {
        rtn = __cre_mbf(mbfid, pk_cmbf);
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

