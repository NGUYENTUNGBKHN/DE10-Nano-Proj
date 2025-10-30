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
/*      grp_vos_que.c                                                           2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Queue Function Submodule                                                                */
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
/*                              to grp_vos_CreateQueue.                                         */
/*                              Changed to search by name an empty record from Queue Table.     */
/*                              Deleted process of iUseFlg.                                     */
/*                              Added preprocessor for GRP_VOS_MAX_QUE.                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/
#if (GRP_VOS_MAX_QUE)
grp_vos_t_queue                 _tQueueTable[GRP_VOS_MAX_QUE];
#endif

/**** SYSTEN MEMORRY EXTERN *********************************************************************/
EXTERN  TX_BYTE_POOL            grp_vos_systemmemory;

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/


/************************************************************************************************/
/* FUNCTION   : grp_vos_CreateQueue                                                             */
/*                                                                                              */
/* DESCRIPTION: Create Queue.                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptQueue                        Pointer of Queue Information Block Area         */
/*              pucName                         Queue name                                      */
/*              ulMsgSize                       Message size(byte)                              */
/*              ulMsgCount                      Message Count                                   */
/* OUTPUT     : pptQueue                        Queue Information Block                         */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_LOW_SYSTEM_HEAP     Low System Heap.                                */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_CreateQueue(grp_vos_t_queue **pptQueue,            /* Queue Information Block  */
                             grp_u8          *pucName,              /* Queue name               */
                             grp_u32         ulMsgSize,             /* Message size(byte)       */
                             grp_u32         ulMsgCount)            /* Message Count            */
{
#if (GRP_VOS_MAX_QUE)
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;
grp_si                          i;

    /* Search empty record from Queue Table */
    for (i = 0; i < GRP_VOS_MAX_QUE; i++)
    {
        if (_tQueueTable[i].aucName[0] == 0)
        {
            break;
        }
    }

    /* If found empty record */
    if (i < GRP_VOS_MAX_QUE)
    {
        /* Parameter check */
        if (((void *)pucName != NULL) &&
           (grp_std_strlen( (grp_u8 *)pucName ) <= GRP_VOS_NAME_SIZE))
        {
            /* Register Queue Name */
            grp_std_strncpy( _tQueueTable[i].aucName, (grp_u8 *)pucName, GRP_VOS_NAME_SIZE );
            _tQueueTable[i].aucName[GRP_VOS_NAME_SIZE] = 0;
            
            /* Create queue stack pool */
            uiErcd = tx_byte_allocate( &grp_vos_systemmemory,
                                       &_tQueueTable[i].ptQueue,
                                       (ULONG)(sizeof(void *) * ulMsgCount),
                                       TX_NO_WAIT );

            if (uiErcd == TX_SUCCESS)
            {
                /* Create Queue */
                uiErcd = tx_queue_create( &_tQueueTable[i].cdtq,
                                          (CHAR *)_tQueueTable[i].aucName,
                                          TX_1_ULONG,
                                          _tQueueTable[i].ptQueue,
                                          (ULONG)(sizeof(void *) * ulMsgCount) );

                if (uiErcd == TX_SUCCESS)
                {
                    uiErcd = tx_byte_allocate( &grp_vos_systemmemory,
                                               &_tQueueTable[i].ptMessage,
                                               ((ulMsgSize + sizeof(void *)) * ulMsgCount),
                                               TX_NO_WAIT );

                    if (uiErcd == TX_SUCCESS)
                    {
                        /* Create queue message buffer */
                        uiErcd = tx_block_pool_create( &_tQueueTable[i].cmpf_message,
                                                       (CHAR *)_tQueueTable[i].aucName,
                                                       (ULONG)ulMsgSize,
                                                       _tQueueTable[i].ptMessage,
                                                       (ULONG)((ulMsgSize + sizeof(void *)) * ulMsgCount) );

                        /* If SUCCESS then Store queue pointer */
                        if (uiErcd == TX_SUCCESS) 
                        {
                                /* Initialize Queue Information Block */
                                _tQueueTable[i].ulMsgSize = ulMsgSize;

                                *pptQueue = &_tQueueTable[i];
                        }
                        else
                        {
                            /* Delete Byte Pool */
                            tx_byte_release( _tQueueTable[i].ptMessage );

                            /* Delete Queue */
                            tx_queue_delete( &_tQueueTable[i].cdtq );

                            /* Delete queue stack area */
                            tx_byte_release( _tQueueTable[i].ptQueue );

                            /* Release Queue Info. Block */
                            grp_std_memset( &_tQueueTable[i], 0, sizeof(grp_vos_t_queue) );

                            /* TX ErrorCode Change (tx_block_pool_create) */
                            switch (uiErcd)
                            {
                                case    TX_POOL_ERROR:
                                case    TX_PTR_ERROR:
                                case    TX_SIZE_ERROR:

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
                        /* Delete Queue */
                        tx_queue_delete( &_tQueueTable[i].cdtq );

                        /* Release queue stack area */
                        tx_byte_release( _tQueueTable[i].ptQueue );

                        /* Release Queue Info. Block */
                        grp_std_memset( &_tQueueTable[i], 0, sizeof(grp_vos_t_queue) );
                    }
                }
                /* If illegal parameter */
                else
                {
                    /* Release queue stack area */
                    tx_byte_release( _tQueueTable[i].ptQueue );

                    /* TX ErrorCode Change (tx_queue_create) */
                    switch (uiErcd)
                    {
                        case    TX_QUEUE_ERROR:
                        case    TX_PTR_ERROR:
                        case    TX_SIZE_ERROR:

                            lResult = GRP_VOS_NEG_ILL_PARAMETER;
                            break;
                        case    TX_CALLER_ERROR:
                        default:

                            lResult = GRP_VOS_NEG_OS_ERROR;
                            break;
                    }
                }
            }
            /* Queue area create error */
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
/* FUNCTION   : grp_vos_DeleteQueue                                                             */
/*                                                                                              */
/* DESCRIPTION: Delete Queue.                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptQueue                         Queue Information Block                         */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_DeleteQueue(grp_vos_t_queue *ptQueue)               /* Queue Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_DEL_QUE == GRP_VOS_USE)
grp_ui                          uiErcd;

    /* Delete Queue */
    uiErcd = tx_queue_delete( &ptQueue->cdtq );

    if (uiErcd == TX_SUCCESS)
    {
        /* Delete Massage Pool Area */
        uiErcd = tx_block_pool_delete( &ptQueue->cmpf_message );
        if (uiErcd == TX_SUCCESS)
        {
            /* Release Massage Pool Area */
            uiErcd = tx_byte_release( ptQueue->ptMessage );

            if (uiErcd == TX_SUCCESS)
            {
                /* Release queue stack area */
                uiErcd = tx_byte_release( ptQueue->ptQueue );

                if (uiErcd == TX_SUCCESS)
                {
                    /* Release Queue Info. Block */
                    grp_std_memset( ptQueue, 0, sizeof(grp_vos_t_queue) );
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
        else
        {
            /* TX ErrorCode Change (tx_block_pool_delete) */
            switch (uiErcd)
            {
                case    TX_POOL_ERROR:

                    lResult = GRP_VOS_NEG_ILL_PARAMETER;
                    break;
                case    TX_CALLER_ERROR:
                default:

                    lResult = GRP_VOS_NEG_OS_ERROR;
                    break;
            }
        }
    }
    else
    {
        /* TX ErrorCode Change (tx_queue_delete) */
        switch (uiErcd)
        {
            case    TX_QUEUE_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_CALLER_ERROR:
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }
#else
    lResult = GRP_VOS_NEG_NO_SUPPORT;
#endif  /* GRP_VOS_DEL_QUE */

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_SendQueue                                                               */
/*                                                                                              */
/* DESCRIPTION: Send Message to Queue.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptQueue                         Queue Information Block                         */
/*              pvMsg                           Message Pointer                                 */
/*              ulTimeout                       Wait Timeout(ms)                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_TIMEOUT             Timeout is occurred.                            */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_SendQueue(grp_vos_t_queue  *ptQueue,               /* Queue Information Block  */
                           void             *pvMsg,                 /* Message Pointer          */
                           grp_u32          ulTimeout)              /* Wait Timeout(ms)         */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;
grp_u32                         ulWaitOption;
void                            *pvBuf;

    /* Convert to suspend from ulTimeout */
    if (ulTimeout == GRP_VOS_NOWAIT)
        ulWaitOption = TX_NO_WAIT;
    else if (ulTimeout == GRP_VOS_INFINITE)
        ulWaitOption = TX_WAIT_FOREVER;
    else
        ulWaitOption = _grp_vos_ConvertTimeUnit( ulTimeout );

    /* Get partition pool */
    uiErcd = tx_block_allocate( &ptQueue->cmpf_message,
                                (VOID **)&pvBuf,
                                ulWaitOption );

    if (uiErcd == TX_SUCCESS)
    {
        /* Copy message */
        grp_std_memcpy( pvBuf, pvMsg, ptQueue->ulMsgSize );

        /* Send to Queue */
        uiErcd = tx_queue_send( &ptQueue->cdtq,
                                (VOID *)&pvBuf,
                                ulWaitOption );
        if (uiErcd != TX_SUCCESS)
        {
            /* TX ErrorCode Change (tx_queue_send) */
            switch (uiErcd)
            {
                case    TX_QUEUE_ERROR:
                case    TX_PTR_ERROR:
                case    TX_WAIT_ERROR:

                    lResult = GRP_VOS_NEG_ILL_PARAMETER;
                    break;
                case    TX_DELETED:
                case    TX_WAIT_ABORTED:

                    lResult = GRP_VOS_NEG_INV_STAT;
                    break;
                case    TX_QUEUE_FULL:

                    lResult = GRP_VOS_NEG_TIMEOUT;
                    break;
                default:

                    lResult = GRP_VOS_NEG_OS_ERROR;
                    break;
            }
            /* release partition pool */
            tx_block_release( (VOID *)pvBuf );
        }
    }
    /* If not SUCCESS then error return */
    else
    {
        /* TX ErrorCode Change (tx_block_allocate) */
        switch (uiErcd)
        {
            case    TX_POOL_ERROR:
            case    TX_PTR_ERROR:
            case    TX_WAIT_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_DELETED:
            case    TX_WAIT_ABORTED:

                lResult = GRP_VOS_NEG_INV_STAT;
                break;
            case    TX_NO_MEMORY:

                lResult = GRP_VOS_NEG_TIMEOUT;
                break;
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_ReceiveQueue                                                            */
/*                                                                                              */
/* DESCRIPTION: Receive Message from Queue.                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptQueue                         Queue Information Block                         */
/*              pvMsg                           Message Pointer                                 */
/*              ulTimeout                       Wait Timeout(ms)                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_TIMEOUT             Timeout is occurred.                            */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_ReceiveQueue(grp_vos_t_queue *ptQueue,             /* Queue Information Block  */
                              void            *pvMsg,               /* Message Pointer          */
                              grp_u32         ulTimeout)            /* Wait Timeout(ms)         */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;
grp_u32                         ulWaitOption;
void                            *pvBuf;

    /* Convert to suspend from ulTimeout */
    if (ulTimeout == GRP_VOS_NOWAIT)
        ulWaitOption = TX_NO_WAIT;
    else if (ulTimeout == GRP_VOS_INFINITE)
        ulWaitOption = TX_WAIT_FOREVER;
    else
        ulWaitOption = _grp_vos_ConvertTimeUnit( ulTimeout );

   /* Receive from Queue */
    uiErcd = tx_queue_receive( &ptQueue->cdtq,
                               (VOID *)&pvBuf,
                               ulWaitOption );

    /* If SUCCESS then copy received data */
    if (uiErcd == TX_SUCCESS)
    {
        grp_std_memcpy( pvMsg, (void *)pvBuf, ptQueue->ulMsgSize );

        uiErcd = tx_block_release( (VOID *)pvBuf );

        if (uiErcd != TX_SUCCESS)
        {
            /* TX ErrorCode Change (tx_block_release) */
            switch (uiErcd)
            {
                case    TX_PTR_ERROR:

                    lResult = GRP_VOS_NEG_ILL_PARAMETER;
                    break;
                default:

                    lResult = GRP_VOS_NEG_OS_ERROR;
                    break;
            }
        }
    }
    else
    {
        /* TX ErrorCode Change (tx_queue_receive) */
        switch (uiErcd)
        {
            case    TX_QUEUE_ERROR:
            case    TX_PTR_ERROR:
            case    TX_WAIT_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_DELETED:
            case    TX_WAIT_ABORTED:

                lResult = GRP_VOS_NEG_INV_STAT;
                break;
            case    TX_QUEUE_EMPTY:

                lResult = GRP_VOS_NEG_TIMEOUT;
                break;
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

