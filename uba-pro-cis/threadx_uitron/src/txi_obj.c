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
/*  txi_obj.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Kernel common functions                                                     */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_byte_pool.h"

int _kernel_loc_cpu = 1;    /* CPU lock status */
int _kernel_dis_dsp = 1;    /* Dispatch status */

#ifdef SYSTEM_MEMORY_SIZE
    #if 0<SYSTEM_MEMORY_SIZE
        static TX_BYTE_POOL txi_system_pool;
        static void* txi_sysmem;
    #endif
#endif

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_object_name                                                     */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create object name.                                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      name                Pointer of name string                              */
/*      pre_name            Object name                                         */
/*      id                  Object ID number                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_alm                                                                 */
/*      cre_cyc                                                                 */
/*      cre_mpf                                                                 */
/*      cre_mpl                                                                 */
/*      cre_tsk                                                                 */
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
void _kernel_object_name(char* name, const char* pre_name, ID id)
{
    int len = 0;

    while(len<(TXI_OBJECT_NAME_LENGTH-1) && *pre_name!='\0')
    {
        *name = *pre_name;
        name++;
        pre_name++;
        len++;
    }

    *name = (char)('0' + ((id/100)%10));
    name++;
    *name = (char)('0' + ((id/10)%10));
    name++;
    *name = (char)('0' + (id%10));
    name++;
    *name = '\0';
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_task_tcb                                                        */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Get tcb pointer                                                         */
/*                                                                              */
/* INPUT                                                                        */
/*      tskid               ID number of the task.                              */
/*                                                                              */
/* OUTPUT                                                                       */
/*      TCB pointer                                                             */
/*                                                                              */
/* CALLS                                                                        */
/*      _kernel_thread_to_task      Get tcb pointer from thread control block   */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
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
/*                                  Current task information is acquired        */
/*                                  directly.                                   */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
_KERNEL_UITRON_TSK* _kernel_task_tcb(ID tskid)
{
    _KERNEL_UITRON_TSK* pTcb;

    if(tskid==TSK_SELF)
    {
        pTcb = _kernel_thread_to_task(_tx_thread_current_ptr);
    }
    else
    {
        pTcb = (_KERNEL_UITRON_TSK*)ID_LIST(_kernel_tsk_id, tskid);
    }

    return pTcb;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_thread_to_task                                                  */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Get tcb pointer from thread control block.                              */
/*                                                                              */
/* INPUT                                                                        */
/*      pThread             Control block pointer of thread                     */
/*                                                                              */
/* OUTPUT                                                                       */
/*      TCB pointer                                                             */
/*                                                                              */
/* CALLS                                                                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
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
_KERNEL_UITRON_TSK* _kernel_thread_to_task(TX_THREAD* pThread)
{
    _KERNEL_UITRON_TSK* pTcb;
    ID tskid;

    if(pThread!=NULL)
    {
        tskid = ((_KERNEL_UITRON_TSK*)pThread)->tskid;
        if(0<tskid && tskid<=_kernel_tsk_maxid)
        {
            pTcb = (_KERNEL_UITRON_TSK*)ID_LIST(_kernel_tsk_id, tskid);
            if(pThread==&pTcb->thread)
            {
                return pTcb;
            }
        }
    }

    return NULL;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_task_cleanup                                                    */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      This function is called from ThreadX, when a waiting state is canceled. */
/*                                                                              */
/* INPUT                                                                        */
/*      pThread             Control block pointer of thread                     */
/*                                                                              */
/* OUTPUT                                                                       */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_thread_system_resume    Resume thread                               */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
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
/*                                  The preemption control method was arranged. */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  Added processing to clear pTcb -> wait_pri  */
/*                                  when timeout occurs,                        */
/*                                  Added argument to correspond to             */
/*                                  ThreadX 5.8 or later.                       */
/*                                                                              */
/********************************************************************************/
#if 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION
VOID _kernel_task_cleanup(TX_THREAD *pThread, ULONG suspension_sequence)
#else /* 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION */
VOID _kernel_task_cleanup(TX_THREAD *pThread)
#endif /* 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION */
{
    _KERNEL_UITRON_TSK* pTcb;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
    if((pTcb=_kernel_thread_to_task(pThread))!=NULL && pTcb->thread.tx_thread_suspend_cleanup!=TX_NULL)
    {
#if 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION
        if(pTcb->thread.tx_thread_suspension_sequence==suspension_sequence)
        {
#endif /* 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION */
            pTcb->wait_obj = NULL;

        /* Remove from waiting list */
            _kernel_deque(&pTcb->wait);

            /* Clear cleanup routine pointer. */
            pTcb->thread.tx_thread_suspend_cleanup = TX_NULL;

            switch(pTcb->tskstat)
            {
            case TTS_WAS:
                pTcb->wobjid = 0;
                pTcb->tskwait = TTW_NONE;
                pTcb->tskstat = TTS_SUS;
                pTcb->wait_rtn = E_TMOUT;
                pTcb->wait_pri = FALSE;
                TX_RESTORE
                break;
            case TTS_WAI:
                pTcb->wobjid = 0;
                pTcb->tskwait = TTW_NONE;
                pTcb->tskstat = TTS_RDY;
                pTcb->wait_rtn = E_TMOUT;
                pTcb->wait_pri = FALSE;

                _tx_thread_preempt_disable++;   /* Temporarily disable preemption.  */
                TX_RESTORE

                _tx_thread_system_resume(&pTcb->thread);
                break;
            default:
                TX_RESTORE
        }
#if 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION
        }
#endif /* 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION */
    }
    else
    {
        TX_RESTORE  /* Restore interrupts. */
    }
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_task_tskwait                                                    */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Get task waiting factor                                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      pTcb                Task TCB                                            */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
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
STAT _kernel_task_tskwait(_KERNEL_UITRON_TSK* pTcb)
{
    switch(pTcb->thread.tx_thread_state)
    {
    case TX_SUSPENDED:
        return pTcb->tskwait;
    }

    return TTW_NONE;
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_sysmem_alloc                                                    */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Allocate memory from system memory pool.                                */
/*                                                                              */
/* INPUT                                                                        */
/*      size                Allocate size                                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      Allocated memory block pointer                                          */
/*                                                                              */
/* CALLS                                                                        */
/*      tx_byte_allocate            Allocate memory block                       */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_dtq                                                                 */
/*      cre_mpf                                                                 */
/*      cre_mpl                                                                 */
/*      cre_tsk                                                                 */
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
/*  2008/08/05      M. Miyashita    version 2.1b, Modified comment(s),          */
/*                                  When SYSTEM_MEMORY_SIZE was not defined,    */
/*                                  it changed so that NULL might be returned.  */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
#if defined(SYSTEM_MEMORY_SIZE) && 0<SYSTEM_MEMORY_SIZE
void* _kernel_sysmem_alloc(ULONG size)
{
    void* mem_ptr;
    UINT status;

    if(txi_sysmem!=NULL)
    {
        status = _tx_byte_allocate(&txi_system_pool, &mem_ptr, size, TX_NO_WAIT);
        if(status==TX_SUCCESS)
        {
            return mem_ptr;
        }
    }

    return NULL;
}
#else
void* _kernel_sysmem_alloc(ULONG size)
{
	return NULL;
}
#endif

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_sysmem_release                                                  */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Release memory to system memory pool.                                   */
/*                                                                              */
/* INPUT                                                                        */
/*      mem_ptr             Start address of the memory block                   */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      tx_byte_release             Release memory block                        */
/*                                                                              */
/* CALLED BY                                                                    */
/*      cre_mpf                                                                 */
/*      cre_mpl                                                                 */
/*      cre_tsk                                                                 */
/*      del_dtq                                                                 */
/*      del_mpf                                                                 */
/*      del_mpl                                                                 */
/*      del_tsk                                                                 */
/*      exd_tsk                                                                 */
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
#if defined(SYSTEM_MEMORY_SIZE) && 0<SYSTEM_MEMORY_SIZE
void _kernel_sysmem_release(void* mem_ptr)
{
    if(txi_sysmem!=NULL)
    {
        _tx_byte_release(mem_ptr);
    }
}
#else
void _kernel_sysmem_release(void* mem_ptr)
{
    ;
}
#endif

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      txi_init                                                                */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Initialization of Library.                                              */
/*                                                                              */
/* INPUT                                                                        */
/*      txi_memory          System memory pointer                               */
/*      txi_memory_size     Memory size which can be used                       */
/*                                                                              */
/* OUTPUT                                                                       */
/*      Initialize satus                                                        */
/*                                                                              */
/* CALLS                                                                        */
/*      tx_byte_pool_create                                                     */
/*      _kernel_system_thread_init                                              */
/*                                                                              */
/* CALLED BY                                                                    */
/*      tx_application_define                                                   */
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
void txi_init(void* txi_memory)
{
#if defined(SYSTEM_MEMORY_SIZE) && 0<SYSTEM_MEMORY_SIZE
    if(txi_memory!=NULL)
    {
        txi_sysmem = txi_memory;
        _tx_byte_pool_create(&txi_system_pool, "uiSYSM", txi_memory, SYSTEM_MEMORY_SIZE);
    }
#endif

    _kernel_system_thread_init();

    _kernel_loc_cpu = 0;
    _kernel_dis_dsp = 0;
}

