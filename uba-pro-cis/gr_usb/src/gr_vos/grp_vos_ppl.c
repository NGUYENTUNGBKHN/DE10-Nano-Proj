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
/*      grp_vos_ppl.c                                                           2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Partition Pool (Fixed Length Memory Pool) Function Submodule                            */
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
/*                              to grp_vos_CreatePartitionPool.                                 */
/*                              The changed from "GRP_VOS_DEL_MPL" to "GRP_VOS_DEL_PPL" in      */
/*                              grp_vos_DeletePartitionPool.                                    */
/*                              Changed to search by name an empty record from Partition Table. */
/*                              Added preprocessor for GRP_VOS_MAX_PPL.                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"
#include "tx_block_pool.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/
#if (GRP_VOS_MAX_PPL)
grp_vos_t_partition_pool        _tPartitionPoolTable[GRP_VOS_MAX_PPL];
#endif

/**** SYSTEN MEMORRY EXTERN *********************************************************************/
EXTERN  TX_BYTE_POOL            grp_vos_systemmemory;

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/

/************************************************************************************************/
/* FUNCTION   : grp_vos_CreatePartitionPool                                                     */
/*                                                                                              */
/* DESCRIPTION: Create Partition Pool (Fixed Length Memory Pool).                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptPartPool                     Pointer of Partition Pool Information Block Area*/
/*              pucName                         Partition Pool Name                             */
/*              ulPartSize                      Memory Block Size                               */
/*              ulPartCount                     Memory Block Count                              */
/* OUTPUT     : pptPartPool                     Partition Pool Information Block                */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_LOW_SYSTEM_HEAP     Low System Heap.                                */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_CreatePartitionPool(grp_vos_t_partition_pool **pptPartPool,/* Partition Pool Information Block */
                                     grp_u8                   *pucName,     /* Partition Pool Name  */
                                     grp_u32                  ulPartSize,   /* Memory Block Size    */
                                     grp_u32                  ulPartCount)  /* Memory Block Count   */
{
#if (GRP_VOS_MAX_PPL)
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_si                          i;
grp_ui                          uiErcd;
grp_u32                         ulAlignedBlkSize = ((ulPartSize + sizeof(grp_u32) - 1) / sizeof(grp_u32)) * sizeof(grp_u32);
grp_u32                         ulPoolSize = (ulAlignedBlkSize + sizeof(grp_u8 *)) * ulPartCount;

    /* Search empty record from Partition Table */
    for (i = 0; i < GRP_VOS_MAX_PPL; i++)
    {
        if (_tPartitionPoolTable[i].aucName[0] == 0)
        {
            break;
        }
    }

    /* If found empty record */
    if (i < GRP_VOS_MAX_PPL)
    {
        /* Parameter check */
        if (((void *)pucName != NULL) &&
           (grp_std_strlen( (grp_u8 *)pucName ) <= GRP_VOS_NAME_SIZE))
        {
            /* Register Partition Pool Name */
            grp_std_strncpy( _tPartitionPoolTable[i].aucName, (grp_u8 *)pucName, GRP_VOS_NAME_SIZE );
            _tPartitionPoolTable[i].aucName[GRP_VOS_NAME_SIZE] = 0;

            /* Allocate VOS System Memory */
            uiErcd = tx_byte_allocate( &grp_vos_systemmemory,
                                       &_tPartitionPoolTable[i].vpMemoryAdd,
                                       (ULONG)ulPoolSize,
                                       TX_NO_WAIT );

            if (uiErcd == TX_SUCCESS)
            {
                /* Create Partition Pool */
                uiErcd = tx_block_pool_create( &_tPartitionPoolTable[i].cmpf,
                                               (CHAR *)_tPartitionPoolTable[i].aucName,
                                               ulPartSize,
                                               (VOID *)_tPartitionPoolTable[i].vpMemoryAdd,
                                               ulPoolSize );

                /* If SUCCESS then Store Partition Pool pointer */
                if (uiErcd == TX_SUCCESS)
                {
                    *pptPartPool = &_tPartitionPoolTable[i];
                }
                /* If not SUCCESS then error return */
                else
                {
                    /* Release Byte Memory pool */
                    tx_byte_release( _tPartitionPoolTable[i].vpMemoryAdd );

                    /* Clear Pool Information */
                    grp_std_memset( &_tPartitionPoolTable[i], 0, sizeof(grp_vos_t_partition_pool) );

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
            else
            {
                /* Clear memory information */
                _tPartitionPoolTable[i].vpMemoryAdd = 0;
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
/* FUNCTION   : grp_vos_DeletePartitionPool                                                     */
/*                                                                                              */
/* DESCRIPTION: Delete Partition Pool (Fixed Length Memory Pool).                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPartPool                      Partition Pool Information Block                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_DeletePartitionPool(grp_vos_t_partition_pool *ptPartPool) /* Partition Pool Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_DEL_PPL == GRP_VOS_USE)
grp_ui                          uiErcd;

    /* Release Partition Pool Area */
    uiErcd = tx_block_pool_delete( &ptPartPool->cmpf );

    /* If SUCCESS then Release Memory Pool Info. */
    if (uiErcd == TX_SUCCESS)
    {
        /* Release Byte Memory pool */
        tx_byte_release( ptPartPool->vpMemoryAdd );

        /* Release Memory Pool Info. Block */
        grp_std_memset( ptPartPool, 0, sizeof(grp_vos_t_partition_pool) );
    }
    /* If not SUCCESS then error return */
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
#else
    lResult = GRP_VOS_NEG_NO_SUPPORT;
#endif  /* GRP_VOS_DEL_PPL */

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_GetPartitionPool                                                        */
/*                                                                                              */
/* DESCRIPTION: Get Fixed Length Memory Block from Partition Pool.                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPartPool                      Partition Pool Information Block                */
/*              ppucMem                         Pointer of Memory Block Address Area            */
/*              ulTimeout                       Wait Timeout(ms)                                */
/* OUTPUT     : ppucMem                         Memory Block Address                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_TIMEOUT             Timeout is occurred.                            */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_GetPartitionPool(grp_vos_t_partition_pool *ptPartPool, /* Partition Pool Information Block */
                                  void                     **ppucMem,   /* Memory Block Address */
                                  grp_u32                  ulTimeout)   /* Wait Timeout(ms)     */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_u32                         ulWaitOption;
grp_ui                          uiErcd;

    /* Convert to suspend from ulTimeout */
    if (ulTimeout == GRP_VOS_NOWAIT)
        ulWaitOption = TX_NO_WAIT;
    else if (ulTimeout == GRP_VOS_INFINITE)
        ulWaitOption = TX_WAIT_FOREVER;
    else
        ulWaitOption = _grp_vos_ConvertTimeUnit( ulTimeout );

    /* Allocate Partition */
    uiErcd = tx_block_allocate( &ptPartPool->cmpf,
                                (VOID **)ppucMem,
                                ulWaitOption );

    /* TX ErrorCode Change (tx_block_allocate) */
    if (uiErcd != TX_SUCCESS)
    {
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
/* FUNCTION   : grp_vos_ReleasePartitionPool                                                    */
/*                                                                                              */
/* DESCRIPTION: Release Fixed Length Memory Block to Partition Pool.                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPartPool                      Partition Pool Information Block                */
/*              pucMem                          Memory Block Address                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_ReleasePartitionPool(grp_vos_t_partition_pool *ptPartPool, /* Partition Pool Information Block */
                                      void                     *pucMem)     /* Memory Block Address */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Check block pool id */
    if (ptPartPool->cmpf.tx_block_pool_id != TX_BLOCK_POOL_ID)
    {
        return(GRP_VOS_NEG_ILL_PARAMETER);
    }

    /* Deallocate Partition */
    uiErcd = tx_block_release( (VOID *)pucMem );

    /* If not SUCCESS then error return */
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

    return(lResult);
}
