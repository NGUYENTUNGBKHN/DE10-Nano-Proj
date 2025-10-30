/********************************************************************************/
/*              Copyright (C) 2013-2017 Grape Systems, Inc.                     */
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
/*  txi_smp.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  SMP functions                                                               */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      cur_cor                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Acquisition of the core which is operating                              */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*     ID of currently executing core                                           */
/*                                                                              */
/* CALLS                                                                        */
/*      tx_thread_smp_core_get      Retrieve the currently executing core of    */
/*                                  caller                                      */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Initialization                                                          */
/*      Task context                                                            */
/*      Non-task context                                                        */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2013/05/27      M. Miyashita    Initial version 2.2i/smp                    */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
UINT cur_cor(void)
{
    return (UINT)tx_thread_smp_core_get();
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      get_cor                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Get the core information to which a task operates.                      */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*      map                 Map of core.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*      tx_thread_smp_core_exclude_get                                          */
/*                                  Gets the thread's current core exclusion    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Initialization                                                          */
/*      Task context                                                            */
/*      Non-task context                                                        */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2013/06/07      M. Miyashita    Initial version 2.2i/smp                    */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER get_cor(ID tskid, UINT* map)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
    ULONG exclusion_map;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_PARAM(map!=NULL);

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
        TX_RESTORE  /* Restore interrupts. */
        if(tx_thread_smp_core_exclude_get(&pTcb->thread, &exclusion_map)!=TX_SUCCESS)
        {
            rtn = E_OBJ;
        }
        else
        {
            *map = (UINT)((~exclusion_map)&TX_THREAD_SMP_CORE_MASK);
        }
        TX_DISABLE  /* Lockout interrupts. */
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      set_cor                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Set the core information to which a task operates.                      */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*      map                 Map of core.                                        */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_CTX               Context error                                       */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_task_tcb            Get tcb pointer                             */
/*      tx_thread_smp_core_exclude  Exclude thread execution on a set of cores  */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Initialization                                                          */
/*      Task context                                                            */
/*      Non-task context                                                        */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2013/06/07      M. Miyashita    Initial version 2.2i/smp                    */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER set_cor(ID tskid, UINT map)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_TSK* pTcb;
    ULONG exclusion_map;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(TSK_SELF, _kernel_tsk_maxid, tskid)
    CHECK_PARAM((map&TX_THREAD_SMP_CORE_MASK)!=0);
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pTcb = _kernel_task_tcb(tskid);
    if(pTcb!=NULL)
    {
        exclusion_map = (ULONG)((~map)&TX_THREAD_SMP_CORE_MASK);
        TX_RESTORE  /* Restore interrupts. */
        if(tx_thread_smp_core_exclude(&pTcb->thread, exclusion_map)!=TX_SUCCESS)
        {
            rtn = E_OBJ;
        }
        TX_DISABLE  /* Lockout interrupts. */
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

