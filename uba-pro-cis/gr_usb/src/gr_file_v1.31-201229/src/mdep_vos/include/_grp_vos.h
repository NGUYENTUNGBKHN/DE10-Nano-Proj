/*************************************************************************/
/*                                                                       */
/* Copyright(C) 2000-2003 Grape Systems, Inc.                            */
/*                        All Rights Reserved                            */
/*                                                                       */
/* This software is furnished under a license and may be used and copied */
/* only in accordance with the terms of such license and with the        */
/* inclusion of the above copyright notice. No title to and ownership of */
/* the software is transferred.                                          */
/* Grape Systems Inc. makes no representation or warranties with respect */
/* to the performance of this computer program, and specifically         */
/* disclaims any responsibility for any damages, special or              */
/* consequential, connected with the use of this program.                */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      grp_vos.h                                       VOS/1.05-TRON    */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      Dammy header file for GR-FILE                                    */
/*                                                                       */
/* AUTHOR:                                                               */
/*                                                                       */
/*      Hiroshi Yamakawa,  GRAPE SYSTEMS, INC.                           */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      gr_vos.h                                                         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*   NAME         DATE        REMARKS                                    */
/*                                                                       */
/*   H.Yamakawa 2004/03/03    Created initial version 1.00               */
/*                                                                       */
/*************************************************************************/

#ifndef __GRP_VOS_H__
#define __GRP_VOS_H__

#include "gr_vos.h"

/**** DATA TYPE & CONSTANT DEFINES ***************************************/

/* Function Result Codes */
#define GRP_VOS_POS_RESULT              GRVOS_POS_RESULT
#define GRP_VOS_NEG_BASE                GRVOS_NEG_BASE
#define GRP_VOS_NEG_LOW_RESOURCE        GRVOS_NEG_LOW_RESOURCE
#define GRP_VOS_NEG_ILL_PARAMETER       GRVOS_NEG_ILL_PARAMETER
#define GRP_VOS_NEG_NO_MEMORY           GRVOS_NEG_NO_MEMORY
#define GRP_VOS_NEG_NO_MESSAGE          GRVOS_NEG_NO_MESSAGE
#define GRP_VOS_NEG_NO_SEMAPHORE        GRVOS_NEG_NO_SEMAPHORE
#define GRP_VOS_NEG_NO_TASK             GRVOS_NEG_NO_TASK
#define GRP_VOS_NEG_INV_STAT            GRVOS_NEG_INV_STAT
#define GRP_VOS_NEG_FULL_MESSAGE        GRVOS_NEG_FULL_MESSAGE
#define GRP_VOS_NEG_OS_ERROR            GRVOS_NEG_OS_ERROR
#define GRP_VOS_NEG_LOW_SYSTEM_HEAP     GRVOS_NEG_LOW_SYSTEM_HEAP
#define GRP_VOS_NEG_NOT_INIT            GRVOS_NEG_NOT_INIT
#define GRP_VOS_NEG_OVER_FLOW           GRVOS_NEG_OVER_FLOW
#define GRP_VOS_NEG_NO_SUPPORT          GRVOS_NEG_NO_SUPPORT


/* Wait Timeout Value */
#define GRP_VOS_NOWAIT                  GRVOS_NOWAIT
#define GRP_VOS_INFINITE                GRVOS_INFINITE


/* Task Initial Status */
#define GRP_VOS_READY                   GRVOS_READY
#define GRP_VOS_SUSPEND                 GRVOS_SUSPEND

/* Task Priority for uITRON4.0 */
#define GRP_VOS_PRI_CRITICAL            GRVOS_PRI_CRITICAL
#define GRP_VOS_PRI_HIGHEST             GRVOS_PRI_HIGHEST
#define GRP_VOS_PRI_HIGH                GRVOS_PRI_HIGH
#define GRP_VOS_PRI_NORMAL              GRVOS_PRI_NORMAL
#define GRP_VOS_PRI_LOW                 GRVOS_PRI_LOW
#define GRP_VOS_PRI_LOWEST              GRVOS_PRI_LOWEST
#define GRP_VOS_PRI_IDLE                GRVOS_PRI_IDLE


/* VOS Object Name Length */
#define GRP_VOS_NAME_SIZE               GRVOS_NAME_SIZE


/* OS Object information block for uITRON4.0 */

typedef GRVOS_tMemoryPool               GRP_VOS_tMemoryPool;
typedef GRVOS_tPartitionPool            GRP_VOS_tPartitionPool;
//typedef                                 GRP_VOS_tMPF;
typedef GRVOS_tQueue                    GRP_VOS_tQueue;
typedef GRVOS_tSemaphore                GRP_VOS_tSemaphore;
typedef GRVOS_tTask                     GRP_VOS_tTask;


/**** PUBLIC FUNCTION PROTOTYPES *****************************************/

/* Initialize/Terminate Function define */
extern  UINT16  GRVOS_Init(void);
extern  UINT16  GRVOS_Exit(void);
extern  void    GRVOS_OsInitialized(void);
extern  UINT16  GRVOS_IsOsInitialized(void);
extern  void    GRVOS_ShowAll(void);

/* Memory Pool Function define */
#define GRP_VOS_CreateMemoryPool        GRVOS_CreateMemoryPool
#define GRP_VOS_DeleteMemoryPool        GRVOS_DeleteMemoryPool
#define GRP_VOS_GetMemoryPool           GRVOS_GetMemoryPool
#define GRP_VOS_ReleaseMemoryPool       GRVOS_ReleaseMemoryPool

/* Partition Pool Function define */
#define GRP_VOS_CreatePartitionPool     GRVOS_CreatePartitionPool
#define GRP_VOS_DeletePartitionPool     GRVOS_DeletePartitionPool
#define GRP_VOS_GetPartitionPool        GRVOS_GetPartitionPool
#define GRP_VOS_ReleasePartitionPool    GRVOS_ReleasePartitionPool

/* Queue Function define */
#define GRP_VOS_CreateQueue             GRVOS_CreateQueue
#define GRP_VOS_DeleteQueue             GRVOS_DeleteQueue
#define GRP_VOS_SendQueue               GRVOS_SendQueue
#define GRP_VOS_ReceiveQueue            GRVOS_ReceiveQueue

/* Semaphore Function define */
#define GRP_VOS_CreateSemaphore         GRVOS_CreateSemaphore
#define GRP_VOS_DeleteSemaphore         GRVOS_DeleteSemaphore
#define GRP_VOS_GetSemaphore            GRVOS_GetSemaphore
#define GRP_VOS_ReleaseSemaphore        GRVOS_ReleaseSemaphore
#define GRP_VOS_iReleaseSemaphore       GRVOS_iReleaseSemaphore

/* Timer Function define */
#define GRP_VOS_DelayTask               GRVOS_DelayTask

/* Task Function define */
#define GRP_VOS_CreateTask              GRVOS_CreateTask
#define GRP_VOS_DeleteTask              GRVOS_DeleteTask
#define GRP_VOS_ExitTask                GRVOS_ExitTask
#define GRP_VOS_SuspendTask             GRVOS_SuspendTask
#define GRP_VOS_ResumeTask              GRVOS_ResumeTask
#define GRP_VOS_GetTask                 GRVOS_GetTask
#define GRP_VOS_GetTaskArg              GRVOS_GetTaskArg
#define GRP_VOS_RegisterTask            GRVOS_RegisterTask

#endif  /* __GRP_VOS_H__ */

/**** END OF FILE gr_vos.h ************************************************/
