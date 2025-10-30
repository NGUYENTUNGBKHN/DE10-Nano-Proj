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
/*  ref_mpl.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference variable sized memory pool                                        */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_byte_pool.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_mpl                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference variable sized memory pool.                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      mplid               ID number of the variable sized memory pool.        */
/*      pk_rmpl             Pointer to the packet returning the variable sized  */
/*                          memory pool state.                                  */
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
/*  2011/04/19      M. Miyashita    version 2.2b, Modified comment(s),          */
/*                                  From all the memory blocks, it changed so   */
/*                                  that the maximum memory block size might    */
/*                                  be searched.                                */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER ref_mpl(ID mplid, T_RMPL* pk_rmpl)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_MPL* pMpl;
    UCHAR*     current_ptr;               /* Current block pointer      */
    UCHAR*     next_ptr;                  /* Next block pointer         */
    ULONG     available_bytes;            /* Calculate bytes available  */
    ULONG     examine_blocks;             /* Blocks to be examined      */
    ULONG     current_bytes;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_mpl_maxid, mplid)
    CHECK_PARAM(pk_rmpl!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pMpl = (_KERNEL_UITRON_MPL*)ID_LIST(_kernel_mpl_id, mplid);
    if(pMpl!=NULL)
    {
        if(pMpl->wtsk!=NULL)
        {
            pk_rmpl->wtskid = ((_KERNEL_UITRON_TSK*)pMpl->wtsk->data)->tskid;
        }
        else
        {
            pk_rmpl->wtskid = TSK_NONE;
        }

        pk_rmpl->fmplsz = pMpl->bpool.tx_byte_pool_available;

        /* Walk through the memory pool in search for a large enough block.  */
        current_ptr = pMpl->bpool.tx_byte_pool_search;
        examine_blocks =   pMpl->bpool.tx_byte_pool_fragments + 1;
        available_bytes =  0;
        do
        {
            /* Check to see if this block is free.  */
            if (*((ULONG *) (current_ptr + sizeof(UCHAR *))) == TX_BYTE_BLOCK_FREE)
            {
                /* Block is free, see if it is large enough.  */

                /* Pickup the next block's pointer.  */
                next_ptr =  *((UCHAR **) current_ptr);

                /* Calculate the number of byte available in this block.  */
                current_bytes = next_ptr - current_ptr - sizeof(CHAR*) - sizeof(ULONG);
                if(available_bytes<current_bytes)
                {
                    available_bytes = current_bytes;
                }
            }

            /* Block is not free, move to next block.  */
            current_ptr =      *((UCHAR **) current_ptr);

            /* Another block has been searched... decrement counter.  */
            if (examine_blocks)
                examine_blocks--;
        } while(examine_blocks);

        pk_rmpl->fblksz = (UINT)available_bytes;
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

