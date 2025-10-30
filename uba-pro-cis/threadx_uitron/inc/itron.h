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
/*      itron.h                                                                 */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*                                                                              */
/*      Data,structure,prototype and macros define for uITRON                   */
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
/*  NAME            DATE            DESCRIPTION                                 */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s),           */
/*                                  Prototype declaration was added.            */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/

#ifndef _ITRON_H_
#define _ITRON_H_

#include "tx_api.h"

#ifdef __cplusplus
extern "C"{
#endif

/****************************************************************************/
/* Data types                                                               */
/****************************************************************************/

typedef signed char     B;
typedef signed short    H;
typedef signed long     W;

typedef unsigned char   UB;
typedef unsigned short  UH;
typedef unsigned long   UW;

typedef signed char     VB;
typedef signed short    VH;
typedef signed long     VW;

#if defined(_MSC_VER)
    /* for Windows */
    typedef _int64          D;
    typedef unsigned _int64 UD;
    typedef unsigned _int64 VD;
#endif

typedef void*   VP;

#ifndef  TX_API
    #define INT     signed int
    #define UINT    unsigned int
#endif

typedef void*   VP_INT;

typedef signed int      BOOL;

typedef signed int     FN;
typedef signed int     ER;
typedef signed int     ID;

typedef unsigned int    ATR;
typedef unsigned int    STAT;
typedef unsigned int    MODE;
typedef signed int      PRI;

#define SIZE        unsigned long

typedef signed long     TMO;
typedef unsigned long   RELTIM;
typedef unsigned long   SYSTIM;

typedef signed int      ER_BOOL;
typedef signed int      ER_ID;
typedef signed int      ER_UINT;

typedef unsigned int    TEXPTN;
typedef unsigned long   FLGPTN;

typedef void (*FP)();

typedef unsigned int    RDVPTN;
typedef signed int      RDVNO;
typedef unsigned long   OVRTIM;
typedef signed int      INHNO;
typedef signed int      INTNO;
typedef signed int      EXCNO;

/*---------- Structure define ----------*/

/*** Task management ***/
typedef struct t_ctsk {
    ATR         tskatr;   /* attribute */
    VP_INT      exinf;    /* extended information */
    FP          task;     /* start address */
    PRI         itskpri;  /* start priority */
    SIZE        stksz;    /* stack size */
    VP          stk;      /* stack top address */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif
} T_CTSK;

typedef struct t_rtsk
{
    STAT    tskstat;    /* status */
    PRI     tskpri;     /* now priority */
    PRI     tskbpri;    /* base priority */
    STAT    tskwait;    /* waiting factor */
    ID      wobjid;     /* waiting object id */
    TMO     lefttmo;    /* timeout time */
    UINT    actcnt;     /* queuing count of start demand */
    UINT    wupcnt;     /* queuing count of wake up demand */
    UINT    suscnt;     /* nest count of suspend */

    /* and other */
} T_RTSK;

typedef struct t_rtst
{
    STAT    tskstat;    /* status */
    STAT    tskwait;    /* waiting factor */
} T_RTST;

/*** Task exception handling ***/
typedef struct t_dtex
{
    ATR texatr; /* attribute */
    FP  texrtn; /* start address */

    /* and other */
} T_DTEX;

typedef struct t_rtex
{
    STAT    texstat;    /* status */
    TEXPTN  pndptn;     /* suspension exception factor */

    /* and other */
} T_RTEX;

/*** Synchronization service ***/
typedef struct t_csem
{
    ATR     sematr;     /* attribute */
    UINT    isemcnt;    /* start resources count */
    UINT    maxsem;     /* max resources count */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CSEM;

typedef struct t_rsem
{
    ID      wtskid;     /* head task id of waiting queue */
    UINT    semcnt;     /* now resources count */

    /* and other */
} T_RSEM;

typedef struct t_cflg
{
    ATR     flgatr;     /* attribute */
    FLGPTN  iflgptn;    /* start value of eventflag */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CFLG;

typedef struct t_rflg
{
    ID      wtskid;     /* head task id of waiting queue */
    FLGPTN  flgptn;     /* now value of eventflag */

    /* and other */
} T_RFLG;

typedef struct t_cdtq
{
    ATR     dtqatr;     /* attribute */
    UINT    dtqcnt;     /* capacity of dataqueue */

    VP      dtq;        /* start address of dataqueue */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CDTQ;

typedef struct t_rdtq
{
    ID      stskid;     /* head task id of send waiting queue */
    ID      rtskid;     /* head task id of receive waiting queue */
    UINT    sdtqcnt;    /* data count of dataqueue */

    /* and other */
} T_RDTQ;

/* Mailbox packet structure */
typedef struct _tagT_MSG
{
    struct _tagT_MSG*   next;   /* next pointer */
} T_MSG;

typedef struct t_cmbx
{
    ATR     mbxatr;     /* attribute */
    PRI     maxmpri;    /* max value of priority */
    VP      mprihd;     /* start address of message queue header domain by the priority */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CMBX;

typedef struct t_rmbx
{
    ID      wtskid;     /* head task id of waiting queue */
    T_MSG*  pk_msg;     /* address of top message packet */

    /* and other */
} T_RMBX;

typedef struct
{
    T_MSG   msgque;     /* message header */
    PRI     msgpri;     /* message priority */
} T_MSG_PRI;

/*** Extended synchronization ***/
typedef struct t_cmtx
{
    ATR     mtxatr;     /* attribute */
    PRI     ceilpri;    /* max value of priority */

    /* and other */
} T_CMTX;

typedef struct t_rmtx
{
    ID      htskid;     /* task id of mutex lock */
    ID      wtskid;     /* head task id of waiting queue */

    /* and other */
} T_RMTX;

typedef struct t_cmbf
{
    ATR     mbfatr;     /* attribute */
    UINT    maxmsz;     /* max size of message(byte) */
    SIZE    mbfsz;      /* size of message buffer */
    VP      mbf;        /* start address of message buffer */

    /* and other */
} T_CMBF;

typedef struct t_rmbf
{
    ID      stskid;     /* head task id of send waiting queue */
    ID      rtskid;     /* head task id of receive waiting queue */
    UINT    smsgcnt;    /* data count of message buffer */
    SIZE    fmbfsz;     /* free size of message buffer(byte) */

    /* and other */
} T_RMBF;

typedef struct t_cpor
{
    ATR     poratr;     /* attribute */
    UINT    maxcmsz;    /* max size of calling message(byte) */
    UINT    maxrmsz;    /* max size of reply message(byte) */

    /* and other */
} T_CPOR;

typedef struct t_rpor
{
    ID      ctskid;     /* head task id of call waiting queue */
    ID      atskid;     /* head task id of reply waiting queue */

    /* and other */
} T_RPOR;

typedef struct t_rrdv
{
    ID      wtskid;     /* task id of waiting */

    /* and other */
} T_RRDV;


/*** Memory pool ***/
typedef struct t_cmpf
{
    ATR     mpfatr;     /* attribute */
    UINT    blkcnt;     /* number of memory blocks which can be gained */
    UINT    blksz;      /* size of memory blocks(byte) */
    VP      mpf;        /* start address of fixed length memory pool */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CMPF;

typedef struct t_rmpf
{
    ID      wtskid;     /* head task id of waiting queue */
    UINT    fblkcnt;    /* number of free memory blocks */

    /* and other */
} T_RMPF;

typedef struct t_cmpl
{
    ATR     mplatr;     /* attribute */
    SIZE    mplsz;      /* size of memory pool(byte) */
    VP      mpl;        /* start address of variable length memory pool */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CMPL;

typedef struct t_rmpl
{
    ID      wtskid;     /* head task id of waiting queue */
    SIZE    fmplsz;     /* size of free memory pool(byte) */
    UINT    fblksz;     /* max size of free memory blocks(byte) */

    /* and other */
} T_RMPL;

/*** Time management ***/
typedef struct t_ccyc
{
    ATR     cycatr;     /* attribute */
    VP_INT  exinf;      /* extended information */
    FP      cychdr;     /* start address */
    RELTIM  cyctim;     /* starting cycle */
    RELTIM  cycphs;     /* starting phase */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CCYC;

typedef struct t_rcyc
{
    STAT    cycstat;    /* status */
    RELTIM  lefttim;    /* time to next starting */

    /* and other */
} T_RCYC;

typedef struct t_calm
{
    ATR     almatr;     /* attribute */
    VP_INT  exinf;      /* extended information */
    FP      almhdr;     /* start address */
#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME
    VB const    *name;    /* JCM add: name */
#endif

    /* and other */
} T_CALM;

typedef struct t_ralm
{
    STAT    almstat;    /* status */
    RELTIM  lefttim;    /* time to next starting */

    /* and other */
} T_RALM;

typedef struct t_dovr
{
    ATR     ovratr;     /* attribute */
    FP      ovrhdr;     /* start address */

    /* and other */
} T_DOVR;

typedef struct t_rovr
{
    STAT    ovrstat;    /* status */
    OVRTIM  leftotm;    /* remaining processor time */

    /* and other */
} T_ROVR;

/*** Interrupt management ***/
typedef struct t_dinh
{
    ATR     inhatr;     /* attribute */
    FP      inthdr;     /* start address */

    /* and other */
} T_DINH;

/*** System configuration management ***/
typedef struct t_dexc
{
    ATR     excatr;     /* attribute */
    FP      exchdr;     /* start address */

    /* and other */
} T_DEXC;

typedef struct t_rcfg
{
    /* and other */
    int dmmy;
} T_RCFG;

typedef struct t_rver
{
    UH  maker;      /* maker code */
    UH  prid;       /* kernel id */
    UH  spver;      /* ITRON version */
    UH  prver;      /* kernel version */
    UH  prno[4];    /* management information */
} T_RVER;

/****************************************************************************/
/* Data defines                                                             */
/****************************************************************************/

/* Core define */
#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE   0
#endif
#ifndef NULL
    #define NULL    ((void*)0)
#endif

/* Object attribute */
#define TA_HLNG     0x00    /* C language */
#define TA_ASM      0x01    /* assembler language */

#define TA_TFIFO    0x00    /* task by first in first out */
#define TA_TPRI     0x01    /* task by priority */

#define TA_MFIFO    0x00    /* message by first in first out */
#define TA_MPRI     0x02    /* message by message priority */

#define TA_ACT      0x02    /* immidiate task start */
#define TA_RSTR     0x04    /* limited task */

#define TA_WSGL     0x00    /* can't shared event-flag */
#define TA_WMUL     0x02    /* shared event-flag */

#define TA_CLR      0x04    /* clear event-flag when resume */

#define TA_INHERIT  0x02    /* mutex priority inheritance protcol */
#define TA_CEILING  0x03    /* mutex priority limited protcol */

#define TA_STA      0x02    /* cyclic handler working status */
#define TA_PHS      0x04    /* cyclic handler phase save */

/* Service call operation mode */
#define TWF_ANDW    0x00    /* wait for AND */
#define TWF_ORW     0x01    /* wait for OR */

/* Task status */
#define TTS_NONE    0x00    /* non-existent */
#define TTS_RUN     0x01    /* running */
#define TTS_RDY     0x02    /* ready */
#define TTS_WAI     0x04    /* waiting */
#define TTS_SUS     0x08    /* suspended */
#define TTS_WAS     0x0c    /* waiting-suspended */
#define TTS_DMT     0x10    /* dormant */

/* Task waiting factor */
#define TTW_NONE    0x0000  /* waiting none */
#define TTW_SLP     0x0001  /* waiting for wake up */
#define TTW_DLY     0x0002  /* waiting for time */
#define TTW_SEM     0x0004  /* waiting for semaphore */
#define TTW_FLG     0x0008  /* waiting for event flag */
#define TTW_SDTQ    0x0010  /* waiting for data queue send */
#define TTW_RDTQ    0x0020  /* waiting for data queue receive */
#define TTW_MBX     0x0040  /* waiting for mailbox receive */
#define TTW_MTX     0x0080  /* waiting for mutex lock */
#define TTW_SMBF    0x0100  /* waiting for message buffer send */
#define TTW_RMBF    0x0200  /* waiting for message buffer receive */
#define TTW_CAL     0x0400  /* waiting for rendezvous formation */
#define TTW_ACP     0x0800  /* waiting for rendezvous reception */
#define TTW_RDV     0x1000  /* waiting for rendezvous completion */
#define TTW_MPF     0x2000  /* waiting for fixed memory block */
#define TTW_MPL     0x4000  /* waiting for free memory block */

#define TTEX_ENA    0x00    /* task exception enable state */
#define TTEX_DIS    0x01    /* task exception disable state */

#define TCYC_STP    0x00    /* cyclic handler not operating */
#define TCYC_STA    0x01    /* cyclic handler operating */

#define TALM_STP    0x00    /* alarm handler not operating */
#define TALM_STA    0x01    /* alarm handler operating */

#define TOVR_STP    0x00    /* overrun handler not set up */
#define TOVR_STA    0x01    /* overrun handler set up */

/* other */
#define TSK_SELF    0   /* select self-task */
#define TSK_NONE    0   /* task none */
#define TPRI_SELF   0   /* base priority of self-task */
#define TPRI_INI    0   /* starting priority of task */

/* priority limit */
#define TMIN_TPRI   1   /* minimum priority for task */
#define TMAX_TPRI   (TX_MAX_PRIORITIES-1)   /* max priority for task */
#define TMIN_MPRI   1   /* minimum priority for message */
#define TMAX_MPRI   (TX_MAX_PRIORITIES-1)   /* max priority for message */

/* version */
#define TKERNEL_MAKER   0x0113  /* Grape Systems, Inc. */
#define TKERNEL_PRID    0x0000  /* ThreadX-uITRON Genelic */
#define TKERNEL_SPVER   0x5402  /* uITRON 4.02  */
#define TKERNEL_PRVER   0x0200  /* v2.0  */

/* queuing/nest */
#define TMAX_ACTCNT     255 /* Action request queued MAX */
#define TMAX_WUPCNT     255 /* Wake up request queued MAX */
#define TMAX_SUSCNT     255 /* Suspend request queued MAX */

#define TMAX_MAXSEM     32767   /* Semaphore request MAX */

/* timeout value */
#define TMO_POL         0   /* poling */
#define TMO_FEVR        -1  /* forever */
#define TMO_NBLK        -2  /* nonblocking */

/*---------- Error code ----------*/
#define E_OK        0       /* operation successful */
#define E_SYS       -5      /* system error */
#define E_NOSPT     -9      /* no support */
#define E_RSFN      -10     /* reserved function code */
#define E_RSATR     -11     /* reserved attribute */
#define E_PAR       -17     /* parameter */
#define E_ID        -18     /* injustice ID */
#define E_CTX       -25     /* context */
#define E_MACV      -26     /* memory access */
#define E_OACV      -27     /* object access */
#define E_ILUSE     -28     /* illegal use */
#define E_NOMEM     -33     /* insufficient memory */
#define E_NOID      -34     /* insufficient ID */
#define E_OBJ       -41     /* object state */
#define E_NOEXS     -42     /* object no generating */
#define E_QOVR      -43     /* overflow queuing */
#define E_RLWAI     -49     /* compulsive release of waiting state */
#define E_TMOUT     -50     /* timeout */
#define E_DLT       -51     /* compulsive delete of waiting object */
#define E_CLS       -52     /* state change of waiting object */
#define E_WBLK      -57     /* nonblocking receptionist */
#define E_BOVR      -58     /* buffer over flow */

/* error code macros */
#define ERCD(mercd, sercd)      ((((ER)(sercd)<<8) | ((ER)(mercd)&0x00ff)))
#define MERCD(ercd)             ((ER)(B)(ercd))
#define SERCD(ercd)             ((ercd)>>8)

/*---------- Data size macros ----------*/

#define TBIT_TEXPTN     (sizeof(TEXPTN)*8)  /* Number of TEXPTN bits */
#define TBIT_FLGPTN     (sizeof(FLGPTN)*8)  /* Number of FLGPTN bits */

#define TSZ_DTQ(dtqcnt)         (sizeof(VP_INT)*(dtqcnt))
#define TSZ_MPRIHD(maxmpri)     ((sizeof(void*)*TMAX_MPRI)+ \
                                (sizeof(unsigned long)*(TMAX_MPRI+1)/(sizeof(unsigned long)*8))+    \
                                    sizeof(unsigned long))

#define TBIT_RDVPTN             (sizeof(RDVPTN)*8)        /* Number of RDVPTN bits */

#define REAL_BLKSZ(blksz)       ((((blksz)+sizeof(UW)-1)/sizeof(UW))*sizeof(UW))
#define TSZ_MPF(blkcnt, blksz)  ((blkcnt)*(REAL_BLKSZ(blksz) + sizeof(char*)))
#define TSZ_MPL(blkcnt, blksz)  ((blkcnt)*(REAL_BLKSZ(blksz) + sizeof(char*) + sizeof(unsigned long)) \
                                        + sizeof(char*) + sizeof(unsigned long))

#define TSZ_MBF(msgcnt, msgsz)  ((msgcnt)*(sizeof(UINT)+(msgsz)))


/*** Task management service calls ***/
/* CRE_TSK */
ER act_tsk(ID tskid);
ER iact_tsk(ID tskid);
ER_UINT can_act(ID tskid);
void ext_tsk(void);
ER ter_tsk(ID tskid);
ER chg_pri(ID tskid, PRI tskpri);
ER get_pri(ID tskid, PRI* p_tskpri);

#ifndef STANDARD_PROFILE
    ER cre_tsk(ID tskid, T_CTSK* pk_ctsk);
    ER_ID acre_tsk(T_CTSK* pk_ctsk);
    ER del_tsk(ID tskid);
    ER sta_tsk(ID tskid, VP_INT stacd);
    void exd_tsk(void);
    ER ref_tsk(ID tskid, T_RTSK* pk_rtsk);
    ER ref_tst(ID tskid, T_RTST* pk_rtst);
#endif

/*** Task synchronization service calls ***/
ER slp_tsk(void);
ER tslp_tsk(TMO tmout);
ER wup_tsk(ID tskid);
ER iwup_tsk(ID tskid);
ER_UINT can_wup(ID tskid);
ER rel_wai(ID tskid);
ER irel_wai(ID tskid);
ER sus_tsk(ID tskid);
ER rsm_tsk(ID tskid);
ER frsm_tsk(ID tskid);
ER dly_tsk(RELTIM dlytim);

#if 0
/*** Exception handling service calls ***/
/* DEF_TEX */
ER ras_tex(ID tskid, TEXPTN rasptn);
ER iras_tex(ID tskid, TEXPTN rasptn);
ER dis_tex(void);
ER ena_tex(void);
BOOL sns_tex(void);

#ifndef STANDARD_PROFILE
    ER def_tex(ID tskid, T_DTEX* pk_dtex);
    ER ref_tex(ID tskid, T_RTEX* pk_rtex);
#endif
#endif

/*** Synchronization service calls ***/
/* Semaphore */
/* CRE_SEM */
ER sig_sem(ID semid);
ER isig_sem(ID semid);
ER wai_sem(ID semid);
ER pol_sem(ID semid);
ER twai_sem(ID semid, TMO tmout);

#ifndef STANDARD_PROFILE
    ER cre_sem(ID semid, T_CSEM* pk_csem);
    ER_ID acre_sem(T_CSEM* pk_csem);
    ER del_sem(ID semid);
    ER ref_sem(ID semid, T_RSEM* pk_rsem);
#endif

/* Event flag */
/* CRE_FLG */
ER set_flg(ID flgid, FLGPTN setptn);
ER iset_flg(ID flgid, FLGPTN setptn);
ER clr_flg(ID flgid, FLGPTN clrptn);
ER wai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN* p_flgptn);
ER pol_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN* p_flgptn);
ER twai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN* p_flgptn, TMO tmout);

#ifndef STANDARD_PROFILE
    ER cre_flg(ID flgid, T_CFLG* pk_cflg);
    ER_ID acre_flg(T_CFLG* pk_cflg);
    ER del_flg(ID flgid);
    ER ref_flg(ID flgid, T_RFLG* pk_rflg);
#endif

/* Data queue */
/* CRE_DTQ */
ER snd_dtq(ID dtqid, VP_INT data);
ER psnd_dtq(ID dtqid, VP_INT data);
ER ipsnd_dtq(ID dtqid, VP_INT data);
ER tsnd_dtq(ID dtqid, VP_INT data, TMO tmout);
ER fsnd_dtq(ID dtqid, VP_INT data);
ER ifsnd_dtq(ID dtqid, VP_INT data);
ER rcv_dtq(ID dtqid, VP_INT* p_data);
ER prcv_dtq(ID dtqid, VP_INT* p_data);
ER trcv_dtq(ID dtqid, VP_INT* p_data, TMO tmout);

#ifndef STANDARD_PROFILE
    ER cre_dtq(ID dtqid, T_CDTQ* pk_cdtq);
    ER_ID acre_dtq(T_CDTQ* pk_cdtq);
    ER del_dtq(ID dtqid);
    ER ref_dtq(ID dtqid, T_RDTQ* pk_rdtq);
#endif

/* Mail box */
/* CRE_MBX */
ER snd_mbx(ID mbxid, T_MSG* pk_msg);
ER rcv_mbx(ID mbxid, T_MSG** ppk_msg);
ER prcv_mbx(ID mbxid, T_MSG** ppk_msg);
ER trcv_mbx(ID mbxid, T_MSG** ppk_msg, TMO tmout);

#ifndef STANDARD_PROFILE
    ER cre_mbx(ID mbxid, T_CMBX* pk_cmbx);
    ER_ID acre_mbx(T_CMBX* pk_cmbx);
    ER del_mbx(ID mbxid);
    ER ref_mbx(ID mbxid, T_RMBX* pk_rmbx);
#endif

#ifndef STANDARD_PROFILE
    /*** Extended synchronization service calls ***/
    /* Mutex */
    ER cre_mtx(ID mtxid, T_CMTX* pk_cmtx);
    ER_ID acre_mtx(T_CMTX* pk_cmtx);
    ER del_mtx(ID mtxid);
    ER loc_mtx(ID mtxid);
    ER ploc_mtx(ID mtxid);
    ER tloc_mtx(ID mtxid, TMO tmout);
    ER unl_mtx(ID mtxid);
    ER ref_mtx(ID mtxid, T_RMTX* pk_rmtx);
#endif

#ifndef STANDARD_PROFILE
    /* Message buffer */
    ER cre_mbf(ID mbfid, T_CMBF* pk_cmbf);
    ER_ID acre_mbf(T_CMBF* pk_cmbf);
    ER del_mbf(ID mbfid);
    ER snd_mbf(ID mbfid, VP msg, UINT msgsz);
    ER psnd_mbf(ID mbfid, VP msg, UINT msgsz);
    ER tsnd_mbf(ID mbfid, VP msg, UINT msgsz, TMO tmout);
    ER_UINT rcv_mbf(ID mbfid, VP msg);
    ER_UINT prcv_mbf(ID mbfid, VP msg);
    ER_UINT trcv_mbf(ID mbfid, VP msg, TMO tmout);
    ER ref_mbf(ID mbfid, T_RMBF* pk_rmbf);
#endif

#ifndef STANDARD_PROFILE
    /* Rendezvous */
    ER cre_por(ID porid, T_CPOR* pk_cpor);
    ER_ID acre_por(T_CPOR* pk_cpor);
    ER del_por(ID porid);
    ER_UINT cal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz);
    ER_UINT tcal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz, TMO tmout);
    ER_UINT acp_por(ID porid, RDVPTN acpptn, RDVNO* p_rdvno, VP msg);
    ER_UINT pacp_por(ID porid, RDVPTN acpptn, RDVNO* p_rdvno, VP msg);
    ER_UINT tacp_por(ID porid, RDVPTN acpptn, RDVNO* p_rdvno, VP msg, TMO tmout);
    ER fwd_por(ID porid, RDVPTN calptn, RDVNO rdvno, VP msg, UINT cmsgsz);
    ER rpl_rdv(RDVNO rdvno, VP msg, UINT rmsgsz);
    ER ref_por(ID porid, T_RPOR* pk_rpor);
    ER ref_rdv(RDVNO rdvno, T_RRDV* pk_rrdv);
#endif

/*** Memory pool service calls ***/
/* Fixed sized memory pool */
/* CRE_MPF */
ER get_mpf(ID mpfid, VP* p_blk);
ER pget_mpf(ID mpfid, VP* p_blk);
ER tget_mpf(ID mpfid, VP* p_blk, TMO tmout);
ER rel_mpf(ID mpfid, VP blk);

#ifndef STANDARD_PROFILE
    ER ipget_mpf(ID mpfid, VP* p_blk);

    ER cre_mpf(ID mpfid, T_CMPF* pk_cmpf);
    ER_ID acre_mpf(T_CMPF* pk_cmpf);
    ER del_mpf(ID mpfid);
    ER ref_mpf(ID mpfid, T_RMPF* pk_rmpf);
#endif

#ifndef STANDARD_PROFILE
    /* Variable sized memory pool */
    ER cre_mpl(ID mplid, T_CMPL* pk_cmpl);
    ER_ID acre_mpl(T_CMPL* pk_cmpl);
    ER del_mpl(ID mplid);
    ER get_mpl(ID mplid, UINT blksz, VP* p_blk);
    ER pget_mpl(ID mplid, UINT blksz, VP* p_blk);
    ER tget_mpl(ID mplid, UINT blksz, VP* p_blk, TMO tmout);
    ER rel_mpl(ID mplid, VP blk);
    ER ref_mpl(ID mplid, T_RMPL* pk_rmpl);
#endif

/*** Time management service calls ***/
/* System timer */
ER set_tim(SYSTIM* p_systim);
ER get_tim(SYSTIM* p_systim);

#ifndef STANDARD_PROFILE
    ER isig_tim(void);
#endif

/* Cyclic handler */
/* CRE_CYC */
ER sta_cyc(ID cycid);
ER stp_cyc(ID cycid);

#ifndef STANDARD_PROFILE
    ER cre_cyc(ID cycid, T_CCYC* pk_ccyc);
    ER_ID acre_cyc(T_CCYC* pk_ccyc);
    ER del_cyc(ID cycid);
    ER ref_cyc(ID cycid, T_RCYC* pk_rcyc);
#endif

#ifndef STANDARD_PROFILE
    /* Alarm handler */
    ER cre_alm(ID almid, T_CALM* pk_calm);
    ER_ID acre_alm(T_CALM* pk_calm);
    ER del_alm(ID almid);
    ER sta_alm(ID almid, RELTIM almtim);
    ER stp_alm(ID almid);
    ER ref_alm(ID almid, T_RALM* pk_ralm);

    #define ista_alm    sta_alm
    #define istp_alm    stp_alm
#endif

/*** System state service calls ***/
ER rot_rdq(PRI tskpri);
ER irot_rdq(PRI tskpri);
ER get_tid(ID* p_tskid);
ER iget_tid(ID* p_tskid);
ER loc_cpu(void);
ER iloc_cpu(void);
ER unl_cpu(void);
ER iunl_cpu(void);
ER dis_dsp(void);
ER ena_dsp(void);
BOOL sns_ctx(void);
BOOL sns_loc(void);
BOOL sns_dsp(void);
BOOL sns_dpn(void);

/*** Interrupt management service calls ***/
/* DEF_INH */

#ifndef STANDARD_PROFILE
    ER def_inh(INHNO inhno, T_DINH* pk_dinh);

    UINT vdis_int(void);
    UINT vena_int(void);
    UINT vset_int(UINT posture);
    #define ivdis_int       vdis_int
    #define ivena_int       vena_int
    #define ivset_int       vset_int
#endif

#ifndef STANDARD_PROFILE
    /*** System configuration management service calls ***/
    ER def_exc(EXCNO excno, T_DEXC* pk_dexc);
    ER ref_ver(T_RVER* pk_rver);
#endif

/*** System function ***/

#ifdef TX_THREAD_SMP_MAX_CORES
    UINT cur_cor(void);
    ER get_cor(ID tskid, UINT* map);
    ER set_cor(ID tskid, UINT map);
#endif

#ifdef __cplusplus
}
#endif 

#endif /* _ITRON_H_ */

