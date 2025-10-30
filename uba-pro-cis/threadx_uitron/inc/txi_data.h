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
/*      txi_data.h                                                              */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*                                                                              */
/*      Kernel management data defines.                                         */
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
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s),           */
/*                                  Prototype declaration was added.            */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/

#ifndef _TXI_DATA_H_
#define _TXI_DATA_H_

#include "itron.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef TXI_OBJECT_NAME_LENGTH
    #define TXI_OBJECT_NAME_LENGTH  10
#endif

#ifndef TXI_TSK_TARGET_DEFINE
    #define TXI_TSK_TARGET_DEFINE
#endif

#ifndef TXI_TSK_TARGET_DEFINE_INIT
    #define TXI_TSK_TARGET_DEFINE_INIT
#endif

/***** Data types definition *****/
/* List data */
typedef struct _kernel_uitron_obj
{
    struct _kernel_uitron_obj* prev;    /* Previus list */
    struct _kernel_uitron_obj* next;    /* Next list */
    struct _kernel_uitron_obj** plist;  /* Control list */

    void*   data;                           /* Node data */
} _KERNEL_UITRON_OBJ;

/* Definition of a task data type. */
typedef struct _kernel_uitron_tsk
{
    TX_THREAD   thread;
    ID      tskid;
    char    name[TXI_OBJECT_NAME_LENGTH];

    T_CTSK  ctsk;

    VP_INT  stacd;      /* Start code */
    STAT    tskstat;    /* Task status */
    PRI     tskbpri;    /* base priority */

    UINT    actcnt;     /* activate count */

    UINT    wupcnt;     /* queuing count of wake up demand */
    UINT    suscnt;     /* nest count of suspend */

    STAT    tskwait;    /* waiting factor */
    ID      wobjid;     /* waiting object id */
    void*   wait_obj;   /* waiting object pointer */
    BOOL    wait_pri;   /* waiting object priority change request */
    ER      wait_rtn;   /* wait return */
    void*   waitdata;   /* wait data */
    _KERNEL_UITRON_OBJ wait;    /* Wait data */

    STAT    texstat;    /* task exception status */
    FP      texrtn;     /* task exception address */
    TEXPTN  rasptn;     /* task exception pattern */

    VOID*   tx_stack_ptr;   /* Thread's stack pointer   */
    VOID*   tx_stack_start; /* Stack starting address   */
    VOID*   tx_stack_end;   /* Stack ending address     */

    _KERNEL_UITRON_OBJ* locked_mutex;   /* locked mutex */

    TXI_TSK_TARGET_DEFINE
} _KERNEL_UITRON_TSK;

/* Synchronization communication objects data. */

/* Definition of a semaphore data type. */
typedef struct _kernel_uitron_sem
{
    T_CSEM  csem;

    UINT    semcnt;                 /* Semaphore count */

    _KERNEL_UITRON_OBJ* wtsk;       /* Waiting task LIST */
} _KERNEL_UITRON_SEM;

/* Definition of a eventflag data type. */
typedef struct _kernel_uitron_flg
{
    T_CFLG  cflg;

    FLGPTN  curflgptn;              /* Current bit pattern */

    _KERNEL_UITRON_OBJ* wtsk;       /* Waiting task LIST */
} _KERNEL_UITRON_FLG;

typedef struct
{
    FLGPTN  waiptn; /* wait flag pattern of this task */
    MODE    wfmode; /* wait flag mode of this task */
    FLGPTN* p_flgptn;
} _KERNEL_UITRON_FLG_WAIT;

/* Definition of a data queue data type. */
typedef struct _kernel_uitron_dtq
{
    T_CDTQ  cdtq;

    VP_INT* dtqptr;
    UINT    setpos;
    UINT    getpos;
    UINT    dtqcnt;

    _KERNEL_UITRON_OBJ* rtsk;       /* Receive waiting tasks list. */
    _KERNEL_UITRON_OBJ* stsk;       /* Send waiting tasks list. */
} _KERNEL_UITRON_DTQ;

/* Definition of a mailbox data type. */
typedef struct _kernel_uitron_mbx
{
    T_CMBX  cmbx;

    T_MSG*  mbx;                    /* Top of message */
    T_MSG*  mbx_tail;               /* Tail of message */

    _KERNEL_UITRON_OBJ* wtsk;       /* Waiting task LIST */
} _KERNEL_UITRON_MBX;

/* Extended synchronization communication objects data. */

/* Definition of a mutex data type. */
typedef struct _kernel_uitron_mtx
{
    T_CMTX  cmtx;
    _KERNEL_UITRON_OBJ lock;        /* Lock data */

    _KERNEL_UITRON_TSK* lock_tsk;   /* Locked task */

    _KERNEL_UITRON_OBJ* wtsk;       /* Waiting task list */
} _KERNEL_UITRON_MTX;

/* Definition of a message buffer data type. */
typedef struct _kernel_uitron_mbf
{
    T_CMBF  cmbf;

    char*   mbfptr;
    UINT    setpos;
    UINT    getpos;
    UINT    smsgcnt;
    UINT    mbfcnt;

    _KERNEL_UITRON_OBJ* rtsk;       /* Receive waiting tasks list. */
    _KERNEL_UITRON_OBJ* stsk;       /* Send waiting tasks list. */
} _KERNEL_UITRON_MBF;

typedef struct
{
    VP msg;
    UINT msgsz;
} _KERNEL_UITRON_MBF_WAIT;

/* Definition of a rendezvous data type. */
typedef struct _kernel_uitron_por
{
    T_CPOR  cpor;

    _KERNEL_UITRON_OBJ* ctsk;       /* Call waiting tasks. */
    _KERNEL_UITRON_OBJ* atsk;       /* Accept waiting tasks. */
} _KERNEL_UITRON_POR;

typedef struct _kernel_uitron_por_wait
{
    RDVPTN ptn;
    VP msg;
    UINT msgsz;
    RDVNO rdvno;
} _KERNEL_UITRON_POR_WAIT;

/* Definition of a fixed sized memory pool data type. */
typedef struct _kernel_uitron_mpf
{
    TX_BLOCK_POOL   bpool;
    char    name[TXI_OBJECT_NAME_LENGTH];

    T_CMPF  cmpf;

    void*   mpfptr;

    _KERNEL_UITRON_OBJ* wtsk;       /* Waiting task list */
} _KERNEL_UITRON_MPF;

/* Definition of a variable sized memory pool data type. */
typedef struct _kernel_uitron_mpl
{
    TX_BYTE_POOL    bpool;
    char    name[TXI_OBJECT_NAME_LENGTH];

    T_CMPL  cmpl;

    void*   mplptr;

    _KERNEL_UITRON_OBJ* wtsk;       /* Waiting task list */
} _KERNEL_UITRON_MPL;

typedef struct
{
    UINT blksz;
    VP* p_blk;
} _KERNEL_UITRON_MPL_WAIT;

/* Definition of a cyclic handler data type. */
typedef struct _kernel_uitron_cyc
{
    TX_TIMER    timer;
    char    name[TXI_OBJECT_NAME_LENGTH];

    T_CCYC  ccyc;

    BOOL    call_hdr;
    ULONG   handler_time;
    ULONG   lapsed_time;    /* Lapsed_time from from last time. */
} _KERNEL_UITRON_CYC;

/* Definition of a alarm handler data type. */
typedef struct _kernel_uitron_alm
{
    TX_TIMER    timer;
    char    name[TXI_OBJECT_NAME_LENGTH];

    T_CALM  calm;
} _KERNEL_UITRON_ALM;


void txi_init(void* txi_memory);


#ifdef __cplusplus
}
#endif 

#endif /* _TXI_DATA_H_*/

