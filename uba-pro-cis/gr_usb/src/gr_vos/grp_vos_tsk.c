/************************************************************************************************/
/*                                                                                              */
/*                           Copyright(C) 2007-2020 Grape Systems, Inc.                         */
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
/*      grp_vos_tsk.c                                                           2.02            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Task Function Submodule                                                                 */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*   M.kitahara     2015/01/31  2.01                                                            */
/*                              Review and Change the error code for all of the functions.      */
/*                              Add "Check the NULL and the number of characters"               */
/*                              to grp_vos_CreateTask and grp_vos_RegisterTask.                 */
/*                              Modified position of if statement for the                       */
/*                              grp_vos_GetTask and grp_vos_RegisterTask.                       */
/*                              The corresponding about TODO : GRP_VOS_TSK_TIME_SLICE           */
/*                              Changed to search by name an empty record from Task Table.      */
/*                              Added preprocessor for GRP_VOS_MAX_TSK.                         */
/*   K.Kaneko       2020/03/24  2.02                                                            */
/*                              Changed time slice definition for task creation to              */
/*                              TX_NO_TIME_SLICE.                                               */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/
#if (GRP_VOS_MAX_TSK)
grp_vos_t_task                  _tTaskTable[GRP_VOS_MAX_TSK];
#endif

/**** SYSTEN MEMORRY EXTERN *********************************************************************/
EXTERN TX_BYTE_POOL             grp_vos_systemmemory;

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/

/************************************************************************************************/
/* FUNCTION   : grp_vos_CreateTask                                                              */
/*                                                                                              */
/* DESCRIPTION: Create Task.                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptTask                         Pointer of Task Information Block Area          */
/*              pucName                         Task Name                                       */
/*              pfnStartAddr                    Start Address                                   */
/*              ulStackSize,                    Stack Size                                      */
/*              ucPriority,                     Task Priority                                   */
/*              ucAutoStart,                    Task Initial Start                              */
/*              ulArg                           Task Start Parameter                            */
/* OUTPUT     : pptTask                         Task Information Block                          */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_LOW_SYSTEM_HEAP     Low System Heap.                                */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_CreateTask(grp_vos_t_task **pptTask,               /* Task Information Block   */
                            grp_u8         *pucName,                /* Task Name                */
                            void           (*pfnStartAddr)(grp_u32),/* Start Address            */
                            grp_u32        ulStackSize,             /* Stack Size               */
                            grp_u8         ucPriority,              /* Task Priority            */
                            grp_u8         ucAutoStart,             /* Task Initial Start       */
                            grp_u32        ulArg)                   /* Task Start Parameter     */
{
#if (GRP_VOS_MAX_TSK)
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_si                          i;
grp_ui                          uiErcd;
grp_ui                          uiAutoStart;

    /* Search empty record from Task Table */
    for (i = 0; i < GRP_VOS_MAX_TSK; i++)
    {
        if (_tTaskTable[i].aucName[0] == 0)
        {
            break;
        }
    }

    /* If found empty record */
    if (i < GRP_VOS_MAX_TSK)
    {
        /* Parameter check */
        if (((ucAutoStart == GRP_VOS_READY) || (ucAutoStart == GRP_VOS_SUSPEND)) &&
           ((void *)pucName != NULL) &&
           (grp_std_strlen( (grp_u8 *)pucName ) <= GRP_VOS_NAME_SIZE))
        {
            /* Register task Name */
            grp_std_strncpy( _tTaskTable[i].aucName, (grp_u8 *)pucName, GRP_VOS_NAME_SIZE );
            _tTaskTable[i].aucName[GRP_VOS_NAME_SIZE] = 0;
            
            /* Create task stack area */
            uiErcd = tx_byte_allocate( &grp_vos_systemmemory,
                                       &_tTaskTable[i].vpStackStart,
                                       (ULONG)ulStackSize,
                                       TX_NO_WAIT );
            if (uiErcd == TX_SUCCESS)
            {
                if (ucAutoStart == GRP_VOS_READY)
                {
                    uiAutoStart = TX_AUTO_START;
                }
                else
                {
                    uiAutoStart = TX_DONT_START;
                }

                /* Task Information Block pointer */
                _tTaskTable[i].pctsk = &_tTaskTable[i].ctsk;

                /* Create Task */
                uiErcd = tx_thread_create( _tTaskTable[i].pctsk,
                                           (CHAR *)_tTaskTable[i].aucName,
                                           pfnStartAddr,
                                           ulArg,
                                           _tTaskTable[i].vpStackStart,
                                           (ULONG)ulStackSize,
                                           (UINT)ucPriority,
                                           (UINT)ucPriority,
                                           TX_NO_TIME_SLICE,
                                           uiAutoStart );

                /* If SUCCESS then Store task pointer */
                if (uiErcd == TX_SUCCESS)
                {
                    /* Initialize Task Information Block */
                    *pptTask = &_tTaskTable[i];
                }
                else
                {
                    /* Delete Stack area */
                    tx_byte_release( &_tTaskTable[i].vpStackStart );

                    /* Clear Task Info. Block */
                    grp_std_memset( &_tTaskTable[i], 0, sizeof(grp_vos_t_task) );

                    /* TX ErrorCode Change (tx_thread_create) */
                    switch (uiErcd)
                    {
                        case    TX_THREAD_ERROR:
                        case    TX_PTR_ERROR:
                        case    TX_SIZE_ERROR:
                        case    TX_PRIORITY_ERROR:
                        case    TX_THRESH_ERROR:
                        case    TX_START_ERROR:

                            lResult = GRP_VOS_NEG_ILL_PARAMETER;
                            break;
                        case    TX_CALLER_ERROR:
                        default:

                            lResult = GRP_VOS_NEG_OS_ERROR;
                            break;
                    }
                }
            }
            /* If not SUCCESS then error return */
            else
            {
                /* TX ErrorCode Change (tx_byte_allocate) */
                switch (uiErcd)
                {
                    case    TX_POOL_ERROR:
                    case    TX_PTR_ERROR:
                    case    TX_SIZE_ERROR:
                    case    TX_WAIT_ERROR:

                        lResult = GRP_VOS_NEG_ILL_PARAMETER;
                        break;
                    case    TX_DELETED:
                    case    TX_DELETE_ERROR:
                    case    TX_WAIT_ABORTED:

                        lResult = GRP_VOS_NEG_INV_STAT;
                        break;
                    case    TX_NO_MEMORY:

                        lResult = GRP_VOS_NEG_LOW_SYSTEM_HEAP;
                        break;
                    case    TX_CALLER_ERROR:
                    default:

                        lResult = GRP_VOS_NEG_OS_ERROR;
                        break;
                }
            }
        }
        /* If illegal parameter */
        else
        {
            lResult = GRP_VOS_NEG_ILL_PARAMETER;
        }
    }
    /* If not found empty record */
    else
    {
        lResult = GRP_VOS_NEG_LOW_RESOURCE;
    }

    return(lResult);
#else

    return(GRP_VOS_NEG_NO_SUPPORT);
#endif
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_DeleteTask                                                              */
/*                                                                                              */
/* DESCRIPTION: Delete Task.                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTask                          Task Information Block                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_DeleteTask(grp_vos_t_task *ptTask)                   /* Task Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_DEL_TSK == GRP_VOS_USE)
grp_ui                          uiErcd;

    /* Delete Task */
    uiErcd = tx_thread_delete( ptTask->pctsk );

    /* If SUCCESS then Release Task Info. */
    if (uiErcd == TX_SUCCESS)
    {
        /* Delete Stack area */
        uiErcd = tx_byte_release( ptTask->vpStackStart );
        if (uiErcd == TX_SUCCESS)
        {
            /* Release Task Info. Block */
            grp_std_memset( ptTask, 0, sizeof(grp_vos_t_task) );
        }
        else
        {
            /* TX ErrorCode Change (tx_byte_release) */
            switch (uiErcd)
            {
                case    TX_PTR_ERROR:

                    lResult = GRP_VOS_NEG_ILL_PARAMETER;
                    break;
                case    TX_CALLER_ERROR:
                default:

                    lResult = GRP_VOS_NEG_OS_ERROR;
                    break;
            }
        }
    }
    /* If not SUCCESS then error return */
    else
    {
        /* TX ErrorCode Change (tx_thread_delete) */
        switch (uiErcd)
        {
            case    TX_THREAD_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_DELETE_ERROR:

                lResult = GRP_VOS_NEG_INV_STAT;
                break;
            case    TX_CALLER_ERROR:
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }
#else
    lResult = GRP_VOS_NEG_NO_SUPPORT;
#endif  /* GRP_VOS_DEL_TSK */

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_ExitTask                                                                */
/*                                                                                              */
/* DESCRIPTION: Exit Task.                                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Do not return form this function.                                               */
/*                                                                                              */
/************************************************************************************************/
void  grp_vos_ExitTask(void)
{
    TX_THREAD   *pTread = 0;

    /* Get current task */
    pTread = tx_thread_identify();

    /* Exit Task */
    if (pTread != TX_NULL)
    {
        tx_thread_terminate( pTread );
    }

    /* do not return form this function */
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_SuspendTask                                                             */
/*                                                                                              */
/* DESCRIPTION: Suspend Task.                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTask                          Task Information Block                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_SuspendTask(grp_vos_t_task *ptTask)                  /* Task Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Suspend Task */
    uiErcd = tx_thread_suspend( ptTask->pctsk );

    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErrorCode Change (tx_thread_suspend) */
        switch (uiErcd)
        {
            case    TX_THREAD_ERROR:
            case    TX_CALLER_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_SUSPEND_ERROR:

                lResult = GRP_VOS_NEG_INV_STAT;
                break;
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }
 
    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_ResumeTask                                                              */
/*                                                                                              */
/* DESCRIPTION: Resume Task.                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptTask                          Task Information Block                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_ResumeTask(grp_vos_t_task *ptTask)                   /* Task Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    uiErcd = tx_thread_resume( ptTask->pctsk );

    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErrorCode Change (tx_thread_resume) */
        switch (uiErcd)
        {
            case    TX_THREAD_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_SUSPEND_LIFTED:
            case    TX_RESUME_ERROR:

                lResult = GRP_VOS_NEG_INV_STAT;
                break;
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_GetTask                                                                 */
/*                                                                                              */
/* DESCRIPTION: Get Current Task Information Block.                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptTask                         Pointer of Task Information Block Area          */
/* OUTPUT     : pptTask                         Task Information Block                          */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_NO_TASK             Not Task.                                       */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_GetTask(grp_vos_t_task **pptTask)                    /* Task Information Block */
{
#if (GRP_VOS_MAX_TSK)
grp_s32                         lResult = GRP_VOS_NEG_NO_TASK;
TX_THREAD                       *pTread = 0;
grp_si                          i;

    /* Get current task */
    pTread = tx_thread_identify();

    /* If SUCCESS then search free table  */
    if (pTread != TX_NULL)
    {
        /* Search match task ID from Task ID Table */
        for (i = 0; i < GRP_VOS_MAX_TSK; i++)
        {
            if (_tTaskTable[i].pctsk == pTread)
            {
                *pptTask = &_tTaskTable[i];
                lResult = GRP_VOS_POS_RESULT;
                break;
            }
        }
    }
    else
    {
        lResult = GRP_VOS_NEG_OS_ERROR;
    }

    return(lResult);
#else

    return(GRP_VOS_NEG_NO_SUPPORT);
#endif
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_GetTaskArg                                                              */
/*                                                                                              */
/* DESCRIPTION: Get Current Task Argument.                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pulArg                          Pointer of Argument Area                        */
/* OUTPUT     : pulArg                          Argument                                        */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_NO_TASK             Not Task.                                       */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_GetTaskArg(grp_u32 *pulArg)                            /* Task Start Parameter */
{
grp_s32                         lResult;
grp_vos_t_task                  *ptTask;

    lResult = grp_vos_GetTask( &ptTask );

    if (lResult == GRP_VOS_POS_RESULT)
    {
        *pulArg = (grp_u32)ptTask->pctsk->tx_thread_entry_parameter;
    }
    else
    {
        *pulArg = 0;
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_RegisterTask                                                            */
/*                                                                                              */
/* DESCRIPTION: Register Task.                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptTask                         Pointer of Task Information Block Area          */
/*              pucName                         Task Name                                       */
/*              ucPriority                      Task Priority                                   */
/* OUTPUT     : pptTask                         Task Information Block                          */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_RegisterTask(grp_vos_t_task **pptTask,             /* Task Information Block   */
                              grp_u8         *pucName,              /* Task Name                */
                              grp_u8         ucPriority)            /* Task Priority            */
{
#if (GRP_VOS_MAX_TSK)
grp_s32                         lResult = GRP_VOS_POS_RESULT;
TX_THREAD                       *pTread = 0;
grp_si                          i;

    /* Warning correspondence */
    pucName = pucName;
    ucPriority = ucPriority;

    /* Search empty record from Task Table */
    for (i = 0; i < GRP_VOS_MAX_TSK; i++)
    {
        if (_tTaskTable[i].pctsk == NULL)
        {
            break;
        }
    }
    /* If found empty record */
    if (i < GRP_VOS_MAX_TSK)
    {
        /* Parameter check */
        if (((void *)pucName != NULL) &&
           (grp_std_strlen( (grp_u8 *)pucName ) <= GRP_VOS_NAME_SIZE))
        {

            /* Get current task */
            pTread = tx_thread_identify();

            if (pTread != TX_NULL)
            {
                /* Set Task ID */
                _tTaskTable[i].pctsk = pTread;

                /* Set Task stack start */
                _tTaskTable[i].vpStackStart = pTread->tx_thread_stack_start;

                /* Get current task pointer */
                *pptTask = &_tTaskTable[i];

                /* Register task Name */
                grp_std_strncpy( _tTaskTable[i].aucName, (grp_u8 *)pucName, GRP_VOS_NAME_SIZE );
                _tTaskTable[i].aucName[GRP_VOS_NAME_SIZE] = 0;
            }
            else
            {
                /* Clear information */
                grp_std_memset( &_tTaskTable[i], 0, sizeof(grp_vos_t_task) );

                /* Set return value */
                lResult = GRP_VOS_NEG_OS_ERROR;
            }
        }
        /* If illegal parameter */
        else
        {
            lResult = GRP_VOS_NEG_ILL_PARAMETER;
        }
    }
    /* No system Resources */
    else
    {
        lResult = GRP_VOS_NEG_LOW_RESOURCE;
    }

    return(lResult);
#else

    return(GRP_VOS_NEG_NO_SUPPORT);
#endif
}

