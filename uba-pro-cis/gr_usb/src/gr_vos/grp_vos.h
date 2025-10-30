/************************************************************************************************/
/*                                                                                              */
/*                           Copyright(C) 2007-2015 Grape Systems, Inc.                         */
/*                                       All Rights Reserved                                    */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      grp_vos.h                                                               2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for for ThreadX.                                                   */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*   M.Kitahara     2015/01/31  2.01                                                            */
/*                              Added aucName to structure of Each object.                      */
/*                              Modified the GRP_VOS_PRI_IDLE                                   */
/*                              Deleted the following                                           */
/*                                  - GRP_VOS_WAITING                                           */
/*                                  - GRP_VOS_WAITSUS                                           */
/*                                  - GRP_VOS_DORMANT                                           */
/*                                  - GRP_VOS_UNSTATE                                           */
/*                              Changed from extern to EXTERN.                                  */
/*                              Deleted tsk_stat from structure of Task.                        */
/*                              Deleted iUseFlg from structure of Queue and Semaphore.          */
/*                              Deleted GRP_VOS_INFO_USE and GRP_VOS_INFO_NOT_USE.              */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_VOS_H_
#define _GRP_VOS_H_

/**** INCLUDE FILES *****************************************************************************/
#include "tx_api.h"
#include "grp_std_types.h"
#include "grp_std_tools.h"

/**** DATA TYPE & CONSTANT DEFINITIONS **********************************************************/
/* Function Result Codes */
#define GRP_VOS_POS_RESULT              0
#define GRP_VOS_NEG_LOW_RESOURCE        (-1)
#define GRP_VOS_NEG_ILL_PARAMETER       (-2)
#define GRP_VOS_NEG_NO_MEMORY           (-3)
#define GRP_VOS_NEG_NO_MESSAGE          (-4)
#define GRP_VOS_NEG_NO_SEMAPHORE        (-5)
#define GRP_VOS_NEG_NO_TASK             (-6)
#define GRP_VOS_NEG_INV_STAT            (-7)
#define GRP_VOS_NEG_FULL_MESSAGE        (-8)
#define GRP_VOS_NEG_OS_ERROR            (-9)
#define GRP_VOS_NEG_LOW_SYSTEM_HEAP     (-10)
#define GRP_VOS_NEG_NOT_INIT            (-11)
#define GRP_VOS_NEG_OVER_FLOW           (-12)
#define GRP_VOS_NEG_TIMEOUT             (-13)
#define GRP_VOS_NEG_NO_SUPPORT          (-14)

/* Wait Timeout Value */
#define GRP_VOS_NOWAIT                  0x00000000
#define GRP_VOS_INFINITE                0xFFFFFFFF

/* Task Initial Status */
#define GRP_VOS_READY                   0x00
#define GRP_VOS_SUSPEND                 0x01

/* Task Priority for ThreadX */
#define GRP_VOS_PRI_CRITICAL            0
#define GRP_VOS_PRI_HIGHEST             2
#define GRP_VOS_PRI_HIGH                3
#define GRP_VOS_PRI_NORMAL              4
#define GRP_VOS_PRI_LOW                 5
#define GRP_VOS_PRI_LOWEST              6
#define GRP_VOS_PRI_IDLE                (TX_MAX_PRIORITIES - 1)

/* VOS Object Name Length */
#define GRP_VOS_NAME_SIZE               8

/* OS Object information block for ThreadX */

    /* Memory Pool information block */
    typedef struct  grp_vos_t_memory_pool_tg {
        TX_BYTE_POOL            cmpl;
        void                    *vpMemoryAdd;
        grp_u8                  aucName[GRP_VOS_NAME_SIZE + 1];
    } grp_vos_t_memory_pool;

    /* Partition Pool information block */
    typedef struct  grp_vos_t_partition_pool_tg {
        TX_BLOCK_POOL           cmpf;
        void                    *vpMemoryAdd;
        grp_u8                  aucName[GRP_VOS_NAME_SIZE + 1];
    } grp_vos_t_partition_pool;

    /* Queue information block */
    typedef struct  grp_vos_t_queue_tg {
        TX_QUEUE                cdtq;
        grp_u32                 ulMsgSize;
        void                    *ptQueue;
        void                    *ptMessage;
        grp_u8                  aucName[GRP_VOS_NAME_SIZE + 1];
        TX_BLOCK_POOL           cmpf_message;
    } grp_vos_t_queue;

    /* Semaphore information block */
    typedef struct  grp_vos_t_semaphore_TG {
        TX_SEMAPHORE            csem;
        grp_u8                  aucName[GRP_VOS_NAME_SIZE + 1];
    } grp_vos_t_semaphore;

    /* Task information block */
    typedef struct  grp_vos_t_task_TG {
        TX_THREAD               *pctsk;
        TX_THREAD               ctsk;
        void                    *vpStackStart;
        grp_u8                  aucName[GRP_VOS_NAME_SIZE + 1];
    } grp_vos_t_task;

    /* Event flag information block */
    typedef struct  grp_vos_t_flag_TG {
        TX_EVENT_FLAGS_GROUP    group_ptr;
        grp_u8                  aucName[GRP_VOS_NAME_SIZE + 1];
    } grp_vos_t_flag;


/**** PUBLIC FUNCTION PROTOTYPES ****************************************************************/
/* Initialize/Terminate Function define */
EXTERN  grp_s32 grp_vos_Init(void);
EXTERN  grp_s32 grp_vos_Exit(void);
EXTERN  void    grp_vos_OsInitialized(void);
EXTERN  grp_s32 grp_vos_IsOsInitialized(void);

/* Memory Pool Function define */
EXTERN  grp_s32 grp_vos_CreateMemoryPool(grp_vos_t_memory_pool **pptMemPool, grp_u8 *pucName, grp_u32 ulPoolSize);
EXTERN  grp_s32 grp_vos_DeleteMemoryPool(grp_vos_t_memory_pool *ptMemPool);
EXTERN  grp_s32 grp_vos_GetMemoryPool(grp_vos_t_memory_pool *ptMemPool, void **ppucMem, grp_u32 ulSize, grp_u32 ulTimeout);
EXTERN  grp_s32 grp_vos_ReleaseMemoryPool(grp_vos_t_memory_pool *ptMemPool, void *pucMem);

/* Partition Pool Function define */
EXTERN  grp_s32 grp_vos_CreatePartitionPool(grp_vos_t_partition_pool **pptPartPool, grp_u8 *pucName, grp_u32 ulPartSize, grp_u32 ulPartCount);
EXTERN  grp_s32 grp_vos_DeletePartitionPool(grp_vos_t_partition_pool *ptPartPool);
EXTERN  grp_s32 grp_vos_GetPartitionPool(grp_vos_t_partition_pool *ptPartPool, void **ppucMem, grp_u32 ulTimeout);
EXTERN  grp_s32 grp_vos_ReleasePartitionPool(grp_vos_t_partition_pool *ptPartPool, void *pucMem);

/* Queue Function define */
EXTERN  grp_s32 grp_vos_CreateQueue(grp_vos_t_queue **pptQueue, grp_u8 *pucName, grp_u32 ulMsgSize, grp_u32 ulMsgCount);
EXTERN  grp_s32 grp_vos_DeleteQueue(grp_vos_t_queue *ptQueue);
EXTERN  grp_s32 grp_vos_SendQueue(grp_vos_t_queue *ptQueue, void *pvMsg,  grp_u32 ulTimeout);
EXTERN  grp_s32 grp_vos_ReceiveQueue(grp_vos_t_queue *ptQueue, void *pvMsg, grp_u32 ulTimeout);

/* Semaphore Function define */
EXTERN  grp_s32 grp_vos_CreateSemaphore(grp_vos_t_semaphore **pptSemaphore, grp_u8 *pucName, grp_u32 ulSemCount);
EXTERN  grp_s32 grp_vos_DeleteSemaphore(grp_vos_t_semaphore *ptSemaphore);
EXTERN  grp_s32 grp_vos_GetSemaphore(grp_vos_t_semaphore *ptSemaphore, grp_u32 ulTimeout);
EXTERN  grp_s32 grp_vos_ReleaseSemaphore(grp_vos_t_semaphore *ptSemaphore);
EXTERN  grp_s32 grp_vos_iReleaseSemaphore(grp_vos_t_semaphore *ptSemaphore);

/* Timer Function define */
EXTERN  grp_s32 grp_vos_DelayTask(grp_u32 ulDelayTime);

/* Task Function define */
EXTERN  grp_s32 grp_vos_CreateTask(grp_vos_t_task **pptTask, grp_u8 *pucName, void (*pfnStartAddr)(grp_u32), grp_u32 ulStackSize, grp_u8 ucPriority, grp_u8 ucAutoStart, grp_u32 ulArg);
EXTERN  grp_s32 grp_vos_DeleteTask(grp_vos_t_task *ptTask);
EXTERN  void    grp_vos_ExitTask(void);
EXTERN  grp_s32 grp_vos_SuspendTask(grp_vos_t_task *ptTask);
EXTERN  grp_s32 grp_vos_ResumeTask(grp_vos_t_task *ptTask);
EXTERN  grp_s32 grp_vos_GetTask(grp_vos_t_task **pptTask);
EXTERN  grp_s32 grp_vos_GetTaskArg(grp_u32 *pulArg);
EXTERN  grp_s32 grp_vos_RegisterTask(grp_vos_t_task **pptTask, grp_u8 *pucName, grp_u8 ucPriority);

/* Event Flag Function define */
EXTERN  grp_s32 grp_vos_CreateFlag(grp_vos_t_flag **pptFlag, grp_u8 *pucName);
EXTERN  grp_s32 grp_vos_DeleteFlag(grp_vos_t_flag *ptFlag);
EXTERN  grp_s32 grp_vos_SetFlag(grp_vos_t_flag *ptFlag, grp_u32 ulFlag);
EXTERN  grp_s32 grp_vos_iSetFlag(grp_vos_t_flag *ptFlag, grp_u32 ulFlag);
EXTERN  grp_s32 grp_vos_ClearFlag(grp_vos_t_flag *ptFlag, grp_u32 ulFlag);
EXTERN  grp_s32 grp_vos_WaitFlag(grp_vos_t_flag *ptFlag, grp_u32 ulFlag, grp_u32 *pulWflg, grp_u32 ulTimeout);

#endif  /* _GRP_VOS_H_ */

