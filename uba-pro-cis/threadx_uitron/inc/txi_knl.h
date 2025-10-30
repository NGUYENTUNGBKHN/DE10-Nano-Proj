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
/*                                                                              */
/* FILE NAME                                                                    */
/*                                                                              */
/*      txi_knl.h                                                               */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*                                                                              */
/*      Kernel management macro defines.                                        */
/*      This file is used only by the kernel functions.                         */
/*                                                                              */
/* AUTHOR                                                                       */
/*                                                                              */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DEPENDENCIES                                                                 */
/*                                                                              */
/*      NONE                                                                    */
/*                                                                              */
/* HISTORY                                                                      */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  Current task information is acquired        */
/*                                  directly.                                   */
/*  2017/10/11      T.Shibuya       version 2.3, Modified comment(s),           */
/*                                  It corresponds to                           */
/*                                  TX_THREAD_GET_SYSTEM_STATE macro,           */
/*                                  Changed clean up function declaration to    */
/*                                  correspond to ThreadX 5.8 or later.         */
/*                                                                              */
/********************************************************************************/

#ifndef _TXI_KNL_H_
#define _TXI_KNL_H_

#include "txi_data.h"
#include "txi_port.h"

#include "tx_thread.h"
#include "tx_initialize.h"
#include "tx_timer.h"

#ifdef __cplusplus
extern "C"{
#endif

/*---------- Time tick define ----------*/
#ifdef HARDWARE_TIMETIC
    #if HARDWARE_TIMETIC<=0
        #error HARDWARE_TIMETIC is is not allowed 0
    #endif

    #undef TIC_NUME
    #undef TIC_DENO

    #define TIC_NUME    HARDWARE_TIMETIC
    #define TIC_DENO    1
#else
    #ifndef TIC_NUME
        #define TIC_NUME    10          /* Numerator of time tic (0 is not allowed.) */
    #else
        #if TIC_NUME<=0
            #error TIC_NUME is is not allowed 0
        #endif
    #endif

    #ifndef TIC_DENO
        #define TIC_DENO    1           /* Denominator of time tic (0 is not allowed.) */
    #else
        #if TIC_DENO<=0
            #error TIC_DENO is is not allowed 0
        #endif
    #endif
#endif

#ifndef TXI_MAXOBJ_ID
    #define TXI_MAXOBJ_ID       32766
#endif

#define ID_LIST(lptr,id)        ((lptr)[(id)-1])

#define TTEX_ACT    0x10    /* task exception activate bit */
#define TTEX_DISABLE_BIT    (TTEX_ACT|TTEX_DIS)

#ifdef TX_THREAD_GET_SYSTEM_STATE
    #define DISPATCH_ENABLE     (_kernel_dis_dsp==0 && _kernel_loc_cpu==0 && \
                                (TX_THREAD_GET_SYSTEM_STATE()|(ULONG)_tx_thread_preempt_disable)==0)

    #ifndef TXI_SYSTEM_ERROR
        #define TXI_SYSTEM_ERROR    {while(1);}
    #endif

    /* Context check macros:Check for timer thread or ISR. */
    #ifndef TX_TIMER_PROCESS_IN_ISR
        #define IS_TASK_CONTEXT (_tx_thread_current_ptr!=&_tx_timer_thread && \
                                TX_THREAD_GET_SYSTEM_STATE()==0)
    #else
        #define IS_TASK_CONTEXT (TX_THREAD_GET_SYSTEM_STATE()==0)
    #endif

    #define IS_INIT_CONTEXT     (_tx_thread_current_ptr==TX_NULL && \
                                TX_INITIALIZE_IN_PROGRESS<=TX_THREAD_GET_SYSTEM_STATE())
#else
    #define DISPATCH_ENABLE     (_kernel_dis_dsp==0 && _kernel_loc_cpu==0 && \
                                (_tx_thread_system_state|(ULONG)_tx_thread_preempt_disable)==0)

    #ifndef TXI_SYSTEM_ERROR
        #define TXI_SYSTEM_ERROR    {while(1);}
    #endif

    /* Context check macros:Check for timer thread or ISR. */
    #ifndef TX_TIMER_PROCESS_IN_ISR
        #define IS_TASK_CONTEXT (_tx_thread_current_ptr!=&_tx_timer_thread && \
                                _tx_thread_system_state==0)
    #else
        #define IS_TASK_CONTEXT (_tx_thread_system_state==0)
    #endif

    #define IS_INIT_CONTEXT     (_tx_thread_current_ptr==TX_NULL && \
                                TX_INITIALIZE_IN_PROGRESS<=_tx_thread_system_state)

#endif

#ifndef TXI_TIME_IS_TICK
    #define _TIM2TIC(tim)   ((((ULONG)(tim))*TIC_DENO+TIC_NUME-1)/TIC_NUME +1) /* Time value to tick. */
    #define _TIC2TIM(tic)   (((tic)*TIC_NUME)/TIC_DENO) /* Tick to time value. */

    #define TIM2TIC(tim)    _TIM2TIC(tim)
    #define TIC2TIM(tic)    _TIC2TIM(tic)
#else
    #define TIM2TIC(tim)    (tim)
    #define TIC2TIM(tic)    (tic)
#endif

#ifndef TX_DISABLE_ERROR_CHECKING
    #define CHECK_ID(minid,maxid,id)    if((id)<(minid) || (maxid)<(id) || TXI_MAXOBJ_ID<(id)) { return E_ID; }
    #define CHECK_PARAM(operation)      if(!(operation)) { return E_PAR; }
    #define CHECK_TMOUT(tmout)          if((tmout)<TMO_FEVR) { return E_PAR; }
    #define CHECK_PRIORITY(priority)    if(((priority)<TMIN_TPRI) || (TMAX_TPRI<(priority))) { return E_PAR; }
    #define CHECK_ATR(atr,atr_val)      if(((atr)&(atr_val))!=(atr)) { return E_RSATR; }
    #define CHECK_TSK_CONTEXT()         if(!IS_TASK_CONTEXT) { return E_CTX; }
    #define CHECK_TSK_INIT_CONTEXT()    if(!IS_TASK_CONTEXT && !IS_INIT_CONTEXT) { return E_CTX; }
    #define CHECK_NOT_TSK_CONTEXT()     if(IS_TASK_CONTEXT) { return E_CTX; }
#else
    #define CHECK_ID(minid,maxid,id)
    #define CHECK_PARAM(operation)
    #define CHECK_TMOUT(tmout)
    #define CHECK_PRIORITY(priority)
    #define CHECK_ATR(atr,atr_val)
    #define CHECK_TSK_CONTEXT()
    #define CHECK_TSK_INIT_CONTEXT()
    #define CHECK_NOT_TSK_CONTEXT()
#endif


/* Definition of object data area. */
extern _KERNEL_UITRON_TSK _kernel_tsk[];
extern VP _kernel_tsk_id[];
extern INT _kernel_tsk_maxid;

extern _KERNEL_UITRON_SEM _kernel_sem[];
extern VP _kernel_sem_id[];
extern INT _kernel_sem_maxid;

extern _KERNEL_UITRON_FLG _kernel_flg[];
extern VP _kernel_flg_id[];
extern INT _kernel_flg_maxid;

extern _KERNEL_UITRON_DTQ _kernel_dtq[];
extern VP _kernel_dtq_id[];
extern INT _kernel_dtq_maxid;

extern _KERNEL_UITRON_MBX _kernel_mbx[];
extern VP _kernel_mbx_id[];
extern INT _kernel_mbx_maxid;

extern _KERNEL_UITRON_MTX _kernel_mtx[];
extern VP _kernel_mtx_id[];
extern INT _kernel_mtx_maxid;

extern _KERNEL_UITRON_MBF _kernel_mbf[];
extern VP _kernel_mbf_id[];
extern INT _kernel_mbf_maxid;

extern _KERNEL_UITRON_POR _kernel_por[];
extern VP _kernel_por_id[];
extern INT _kernel_por_maxid;

extern _KERNEL_UITRON_MPF _kernel_mpf[];
extern VP _kernel_mpf_id[];
extern INT _kernel_mpf_maxid;

extern _KERNEL_UITRON_MPL _kernel_mpl[];
extern VP _kernel_mpl_id[];
extern INT _kernel_mpl_maxid;

extern _KERNEL_UITRON_CYC _kernel_cyc[];
extern VP _kernel_cyc_id[];
extern INT _kernel_cyc_maxid;

extern _KERNEL_UITRON_ALM _kernel_alm[];
extern VP _kernel_alm_id[];
extern INT _kernel_alm_maxid;

/* Definition of system data. */
extern int _kernel_loc_cpu; /* CPU lock status */
extern int _kernel_dis_dsp; /* Dispatch status */

extern _KERNEL_UITRON_OBJ* _kernel_rendezvous_tsk;  /* waiting for rendezvous completion list */

/* Definition of internal functions. */
void _kernel_copy(char* dst, char* src, UINT size);
ID _kernel_object_find(VP id_list[], INT maxid);

void _kernel_object_name(char* name, const char* pre_name, ID id);
_KERNEL_UITRON_TSK* _kernel_task_tcb(ID tskid);
_KERNEL_UITRON_TSK* _kernel_thread_to_task(TX_THREAD* pThread);
#if 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION
VOID _kernel_task_cleanup(TX_THREAD *pThread, ULONG suspension_sequence);
#else
VOID _kernel_task_cleanup(TX_THREAD *pThread);
#endif /* 5<=THREADX_MAJOR_VERSION && 8<=THREADX_MINOR_VERSION */
ER _kernel_suspend_by_api(_KERNEL_UITRON_TSK* pTcb, TMO tmout);
void _kernel_resume_task(_KERNEL_UITRON_TSK* pTcb, ER ercd);
STAT _kernel_task_tskwait(_KERNEL_UITRON_TSK* pTcb);
void* _kernel_sysmem_alloc(ULONG size);
void _kernel_sysmem_release(void* mem_ptr);

#ifdef TXI_DISABLE_INLINE
    void _kernel_enque(_KERNEL_UITRON_OBJ** ppTop, _KERNEL_UITRON_OBJ* pNode);
    void _kernel_deque(_KERNEL_UITRON_OBJ* pNode);
#else
    #define _kernel_enque(ppTop, pNode) \
        {                               \
            if(*(ppTop)==NULL)          \
            {                           \
                (pNode)->prev = (pNode);    \
                (pNode)->next = (pNode);    \
                *(ppTop) = (pNode);         \
            }                           \
            else                        \
            {                           \
                (pNode)->prev = (*(ppTop))->prev;   \
                (pNode)->next = *(ppTop);           \
                (*(ppTop))->prev->next = (pNode);   \
                (*(ppTop))->prev = (pNode);         \
            }                           \
            (pNode)->plist = ppTop;     \
        }
    #define _kernel_deque(pNode)    \
        {                               \
            if((pNode)->plist!=NULL)    \
            {                           \
                if((pNode)->next==(pNode))      \
                {                           \
                    (*(pNode)->plist) = NULL;       \
                }                           \
                else                        \
                {                           \
                    ((pNode)->next)->prev = (pNode)->prev;  \
                    ((pNode)->prev)->next = (pNode)->next;  \
                    if((*(pNode)->plist)==(pNode))  \
                    {                       \
                        (*(pNode)->plist) = (pNode)->next;  \
                    }                       \
                }                           \
                (pNode)->plist = NULL;      \
            }                               \
        }
#endif

void _kernel_enque_task_pri(_KERNEL_UITRON_OBJ** ppTop, _KERNEL_UITRON_OBJ* pNode);

BOOL _kernel_system_request(void (*function)(void*), void* param);
BOOL _kernel_system_thread_init(void);

VOID _kernel_task_entry(ULONG thread_input);
void _kernel_system_task_terminate(void* param);
void _kernel_system_task_reset(_KERNEL_UITRON_TSK* pTcb);
void _kernel_release_mutex(_KERNEL_UITRON_MTX* pMtx, UINT priority);


ER_UINT __tacp_por(ID porid, RDVPTN acpptn, RDVNO* p_rdvno, VP msg, TMO tmout);

ER __cre_alm(ID almid, T_CALM* pk_calm);
ER __cre_cyc(ID cycid, T_CCYC* pk_ccyc);
ER __cre_dtq(ID dtqid, T_CDTQ* pk_cdtq);
ER __cre_flg(ID flgid, T_CFLG* pk_cflg);

UINT __kernel_copy_to_mbf(_KERNEL_UITRON_MBF* pMbf, char* src, UINT msgsz);
UINT __kernel_copy_from_mbf(_KERNEL_UITRON_MBF* pMbf, char* dst);
ER __cre_mbf(ID mbfid, T_CMBF* pk_cmbf);

ER __cre_mbx(ID mbxid, T_CMBX* pk_cmbx);
ER __cre_mpf(ID mpfid, T_CMPF* pk_cmpf);
ER __cre_mpl(ID mplid, T_CMPL* pk_cmpl);
ER __cre_mtx(ID mtxid, T_CMTX* pk_cmtx);
ER __cre_por(ID porid, T_CPOR* pk_cpor);
ER __cre_sem(ID semid, T_CSEM* pk_csem);
ER __cre_tsk(ID tskid, T_CTSK* pk_ctsk);

RDVNO _kernel_rendezvous_number(_KERNEL_UITRON_TSK* pTcb);
_KERNEL_UITRON_TSK* _kernel_rendezvous_find(_KERNEL_UITRON_OBJ* pTop, RDVNO rdvno);
_KERNEL_UITRON_TSK* _kernel_rendezvous_pattern(_KERNEL_UITRON_OBJ* pTop, RDVPTN ptn);
ER_UINT __tcal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz, TMO tmout);

VOID  _kernel_task_exception_entry(VOID);
void _kernel_task_exception_set(_KERNEL_UITRON_TSK* pTcb);

ER __tget_mpf(ID mpfid, VP* p_blk, TMO tmout);
ER __tget_mpl(ID mplid, UINT blksz, VP* p_blk, TMO tmout);
ER __tloc_mtx(ID mtxid, TMO tmout);
ER __trcv_dtq(ID dtqid, VP_INT* p_data, TMO tmout);
ER_UINT __trcv_mbf(ID mbfid, VP msg, TMO tmout);
ER __trcv_mbx(ID mbxid, T_MSG** ppk_msg, TMO tmout);
ER __tsnd_dtq(ID dtqid, VP_INT data, TMO tmout);
ER __tsnd_mbf(ID mbfid, VP msg, UINT msgsz, TMO tmout);
ER __twai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN* p_flgptn, TMO tmout);
ER __twai_sem(ID semid, TMO tmout);
ER __fsnd_dtq(ID dtqid, VP_INT data);

ER __sig_sem(ID semid);

void _kernel_system_task_restart(void* param);

ER __set_flg(ID flgid, FLGPTN setptn);

#ifdef __cplusplus
}
#endif 

#endif /* _TXI_KNL_H_ */

