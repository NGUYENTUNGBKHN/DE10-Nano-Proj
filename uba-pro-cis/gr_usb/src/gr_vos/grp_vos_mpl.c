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
/*      grp_vos_mpl.c                                                           2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Memory Pool (Variable Length Memory Pool) Function Submodule                            */
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
/*                              to grp_vos_CreateMemoryPool.                                    */
/*                              Changed to search by name an empty record                       */
/*                              from Memory Pool Table.                                         */
/*                              Added preprocessor for GRP_VOS_MAX_MPL.                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/
#if (GRP_VOS_MAX_MPL)
grp_vos_t_memory_pool           _tMemoryPoolTable[GRP_VOS_MAX_MPL];
#endif

/**** SYSTEN MEMORRY EXTERN *********************************************************************/
EXTERN  TX_BYTE_POOL            grp_vos_systemmemory;

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/

/************************************************************************************************/
/* FUNCTION   : grp_vos_CreateMemoryPool                                                        */
/*                                                                                              */
/* DESCRIPTION: Create Memory Pool (Variable Length Memory Pool).                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptMemPool                      Pointer of Memory Pool Information Block Area   */
/*              pucName                         Memory pool name                                */
/*              ulPoolSize                      Pool size                                       */
/* OUTPUT     : pptMemPool                      Memory Pool Information Block                   */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_LOW_SYSTEM_HEAP     Low System Heap.                                */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_CreateMemoryPool(grp_vos_t_memory_pool **pptMemPool,/* Memory Pool Information Block */
                                  grp_u8                *pucName,    /* Memory pool name        */
                                  grp_u32               ulPoolSize)  /* Pool size               */
{
#if (GRP_VOS_MAX_MPL)
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_CRE_MPL == GRP_VOS_USE)
grp_ui                          uiErcd;
grp_si                          i;

    /* Search empty record from Memory Pool Table */
    for (i = 0; i < GRP_VOS_MAX_MPL; i++)
    {
        if (_tMemoryPoolTable[i].aucName[0] == 0)
        {
            break;
        }
    }

    /* If found empty record */
    if (i < GRP_VOS_MAX_MPL)
    {
        /* Parameter check */
        if (((void *)pucName != NULL) &&
           (grp_std_strlen( (grp_u8 *)pucName ) <= GRP_VOS_NAME_SIZE))
        {
            /* Register Memory Pool Name */
            grp_std_strncpy( _tMemoryPoolTable[i].aucName, (grp_u8 *)pucName, GRP_VOS_NAME_SIZE );
            _tMemoryPoolTable[i].aucName[GRP_VOS_NAME_SIZE] = 0;

            /* Allocate from VOS System Memory */
            uiErcd = tx_byte_allocate( &grp_vos_systemmemory,
                                       &_tMemoryPoolTable[i].vpMemoryAdd,
                                       (ULONG)ulPoolSize,
                                       TX_NO_WAIT );

            /* If SUCCESS then Create Memory Pool */
            if (uiErcd == TX_SUCCESS)
            {
                /* Create Memory Pool */
                uiErcd = tx_byte_pool_create( &_tMemoryPoolTable[i].cmpl,
                                              (CHAR *)_tMemoryPoolTable[i].aucName,
                                              (VOID *)_tMemoryPoolTable[i].vpMemoryAdd,
                                              ulPoolSize );

                /* If SUCCESS then Store Memory Pool pointer */
                if (uiErcd == TX_SUCCESS)
                {
                    *pptMemPool = &_tMemoryPoolTable[i];
                }
                /* If not SUCCESS then error return */
                else
                {
                    /* Release Byte Memory pool */
                    tx_byte_release( _tMemoryPoolTable[i].vpMemoryAdd );

                    /* Clear memory information */
                    grp_std_memset( &_tMemoryPoolTable[i], 0, sizeof(grp_vos_t_memory_pool) );

                    /* TX ErrorCode Change (tx_byte_pool_create) */
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
            else
            {
                /* Clear memory information */
                _tMemoryPoolTable[i].vpMemoryAdd = 0;

                /* TX ErrorCode Change (tx_byte_allocate) */
                switch (uiErcd)
                {
                    case    TX_POOL_ERROR:
                    case    TX_PTR_ERROR:
                    case    TX_SIZE_ERROR:
                    case    TX_WAIT_ERROR:

                        lResult = GRP_VOS_NEG_ILL_PARAMETER;
                        break;
                    case    TX_WAIT_ABORTED:
                    case    TX_DELETED:

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
#else
    lResult = GRP_VOS_NEG_NO_SUPPORT;
#endif  /* GRP_VOS_CRE_MPL */

    return(lResult);
#else

    return(GRP_VOS_NEG_NO_SUPPORT);
#endif
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_DeleteMemoryPool                                                        */
/*                                                                                              */
/* DESCRIPTION: Delete Memory Pool (Variable Length Memory Pool).                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMemPool                       Memory Pool Information Block                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_DeleteMemoryPool(grp_vos_t_memory_pool *ptMemPool) /* Memory Pool Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_DEL_MPL == GRP_VOS_USE)
grp_ui                          uiErcd;

    /* Delete Memory Pool */
    uiErcd = tx_byte_pool_delete( &ptMemPool->cmpl );

    /* If SUCCESS then Release Memory Pool Info. */
    if (uiErcd == TX_SUCCESS)
    {
        /* Release Byte Memory pool */
        tx_byte_release( ptMemPool->vpMemoryAdd );

        /* Release Memory Pool Info. Block */
        grp_std_memset( ptMemPool, 0, sizeof(grp_vos_t_memory_pool) );
    }
    /* If not SUCCESS then error return */
    else
    {
        /* TX ErrorCode Change (tx_byte_pool_delete) */
        switch (uiErcd)
        {
            case     TX_POOL_ERROR: 

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }
#else
    lResult = GRP_VOS_NEG_NO_SUPPORT;
#endif  /* GRP_VOS_DEL_MPL */

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_GetMemoryPool                                                           */
/*                                                                                              */
/* DESCRIPTION: Get Variable Length Memory Block from Memory Pool.                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMemPool                       Memory Pool Information Block                   */
/*              ppucMem                         Pointer of Memory Block Address Area            */
/*              ulSize                          Memory Block Size(byte)                         */
/*              ulTimeout                       Wait Timeout(ms)                                */
/* OUTPUT     : ppucMem                         Memory Block Address                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_TIMEOUT             Timeout is occurred.                            */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_GetMemoryPool(grp_vos_t_memory_pool *ptMemPool,/* Memory Pool Information Block */
                               void                  **ppucMem, /* Memory Block Address         */
                               grp_u32               ulSize,    /* Memory Block Size(byte)      */
                               grp_u32               ulTimeout) /* Wait Timeout(ms)             */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_TGET_MPL == GRP_VOS_USE)
grp_ui                          uiErcd;
grp_u32                         ulWaitOption;

    /* Convert to suspend from ulTimeout */
    if (ulTimeout == GRP_VOS_NOWAIT)
        ulWaitOption = TX_NO_WAIT;
    else if (ulTimeout == GRP_VOS_INFINITE)
        ulWaitOption = TX_WAIT_FOREVER;
    else
        ulWaitOption = _grp_vos_ConvertTimeUnit( ulTimeout );

    /* Allocate Memory */
    uiErcd = tx_byte_allocate( &ptMemPool->cmpl,
                               (VOID **)ppucMem,
                               (ULONG)ulSize,
                               ulWaitOption );

    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
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

                lResult = GRP_VOS_NEG_TIMEOUT;
                break;
            case    TX_CALLER_ERROR:
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }
#else
    lResult = GRP_VOS_NEG_NO_SUPPORT;
#endif  /* GRP_VOS_TGET_MPL */

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_ReleaseMemoryPool                                                       */
/*                                                                                              */
/* DESCRIPTION: Release Variable Length Memory Block from Memory Pool.                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMemPool                       Memory Pool Information Block                   */
/*              pucMem                          Memory Block Address                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_ReleaseMemoryPool(grp_vos_t_memory_pool *ptMemPool,/* Memory Pool Information Block */
                                   void                  *pucMem)   /* Memory Block Address     */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_REL_MPL == GRP_VOS_USE)
grp_ui                          uiErcd;

    /* Deallocate Partition */
    uiErcd = tx_byte_release( (VOID *)pucMem );

    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
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
#else
    lResult = GRP_VOS_NEG_NO_SUPPORT;
#endif  /* GRP_VOS_REL_MPL */

    return(lResult);
}
