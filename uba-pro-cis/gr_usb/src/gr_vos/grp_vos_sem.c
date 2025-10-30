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
/*      grp_vos_sem.c                                                           2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Semaphore Function Submodule                                                            */
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
/*                              to grp_vos_CreateSemaphore.                                     */
/*                              Changed to search by name an empty record from Semaphore Table. */
/*                              Deleted process of iUseFlg.                                     */
/*                              Added preprocessor for GRP_VOS_MAX_SEM.                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"
#include "tx_semaphore.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/
#if (GRP_VOS_MAX_SEM)
grp_vos_t_semaphore             _tSemaphoreTable[GRP_VOS_MAX_SEM];
#endif
/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/

/************************************************************************************************/
/* FUNCTION   : grp_vos_CreateSemaphore                                                         */
/*                                                                                              */
/* DESCRIPTION: Create Semaphore.                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptSemaphore                    Pointer of Semaphore Information Block Area     */
/*              pucName                         Semaphore Name                                  */
/*              ulSemCount                      Initial Count                                   */
/* OUTPUT     : pptSemaphore                    Semaphore Information Block                     */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_CreateSemaphore(grp_vos_t_semaphore **pptSemaphore,/* Semaphore Information Block */
                                 grp_u8              *pucName,      /* Semaphore Name           */
                                 grp_u32             ulSemCount)    /* Initial Count            */
{
#if (GRP_VOS_MAX_SEM)
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_si                          i;
grp_ui                          uiErcd;

    /* Search empty record from Semaphore Table */
    for (i = 0; i < GRP_VOS_MAX_SEM; i++)
    {
        if (_tSemaphoreTable[i].aucName[0] == 0)
        {
            break;
        }
    }

    /* If found empty record */
    if (i < GRP_VOS_MAX_SEM)
    {
        /* Parameter check */
        if (((void *)pucName != NULL) &&
           (grp_std_strlen( (grp_u8 *)pucName ) <= GRP_VOS_NAME_SIZE))
        {
            /* Register Semaphore Name */
            grp_std_strncpy( _tSemaphoreTable[i].aucName, (grp_u8 *)pucName, GRP_VOS_NAME_SIZE );
            _tSemaphoreTable[i].aucName[GRP_VOS_NAME_SIZE] = 0;

            /* Create Semaphore */
            uiErcd = tx_semaphore_create( &_tSemaphoreTable[i].csem,
                                          (CHAR *)_tSemaphoreTable[i].aucName,
                                          ulSemCount );

            /* If SUCCESS then Store semaphore pointer */
            if (uiErcd == TX_SUCCESS)
            {
                *pptSemaphore = &_tSemaphoreTable[i];
            }
            /* If not SUCCESS then error return */
            else
            {
                /* Clear semaphore information */
                grp_std_memset( &_tSemaphoreTable[i], 0, sizeof(grp_vos_t_semaphore) );

                /* TX ErrorCode Change (tx_semaphore_create) */
                switch (uiErcd)
                {
                    case    TX_SEMAPHORE_ERROR:

                        lResult = GRP_VOS_NEG_ILL_PARAMETER;
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
/* FUNCTION   : grp_vos_DeleteSemaphore                                                         */
/*                                                                                              */
/* DESCRIPTION: Delete Semaphore.                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptSemaphore                     Semaphore Information Block                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_DeleteSemaphore(grp_vos_t_semaphore *ptSemaphore)/* Semaphore Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_DEL_SEM == GRP_VOS_USE)
grp_ui                          uiErcd;

    /* Delete Semaphore */
    uiErcd = tx_semaphore_delete( &ptSemaphore->csem );

    /* If SUCCESS then Delete Queue Info. */
    if (uiErcd == TX_SUCCESS)
    {
        /* Delete Semaphore Info. Block */
        grp_std_memset( ptSemaphore, 0, sizeof(grp_vos_t_semaphore) );
    }
    /* If not SUCCESS then error return */
    else
    {
        /* TX ErrorCode Change (tx_semaphore_delete) */
        switch (uiErcd)
        {
            case    TX_SEMAPHORE_ERROR:

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
#endif  /* GRP_VOS_DEL_SEM */

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_GetSemaphore                                                            */
/*                                                                                              */
/* DESCRIPTION: Get Semaphore.                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptSemaphore                     Semaphore Information Block                     */
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
grp_s32  grp_vos_GetSemaphore(grp_vos_t_semaphore *ptSemaphore, /* Semaphore Information Block  */
                              grp_u32             ulTimeout)    /* Wait Timeout(ms)             */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;
grp_u32                         ulWaitOption;

    /* Convert to suspend from ulTimeout */
    if (ulTimeout == GRP_VOS_NOWAIT)
        ulWaitOption = TX_NO_WAIT;
    else if (ulTimeout == GRP_VOS_INFINITE)
        ulWaitOption = TX_WAIT_FOREVER;
    else
        ulWaitOption = _grp_vos_ConvertTimeUnit( ulTimeout );

    /* Obtain Semaphore */
    uiErcd = tx_semaphore_get( &ptSemaphore->csem,
                               ulWaitOption );

    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErrorCode Change (tx_semaphore_get) */
        switch (uiErcd)
        {
            case    TX_SEMAPHORE_ERROR:
            case    TX_WAIT_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_DELETED:
            case    TX_WAIT_ABORTED:

                lResult = GRP_VOS_NEG_INV_STAT;
                break;
            case    TX_NO_INSTANCE:

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
/* FUNCTION   : grp_vos_ReleaseSemaphore                                                        */
/*                                                                                              */
/* DESCRIPTION: Release Semaphore.                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptSemaphore                     Semaphore Information Block                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_ReleaseSemaphore(grp_vos_t_semaphore *ptSemaphore) /* Semaphore Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Check semaphore id */
    if (ptSemaphore->csem.tx_semaphore_id != TX_SEMAPHORE_ID)
    {
        return(GRP_VOS_NEG_ILL_PARAMETER);
    }

    /* Release Semaphore */
    uiErcd = tx_semaphore_put( &ptSemaphore->csem );

    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErrorCode Change (tx_semaphore_put) */
        switch (uiErcd)
        {
            case    TX_SEMAPHORE_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_iReleaseSemaphore                                                       */
/*                                                                                              */
/* DESCRIPTION: Release Semaphore for interrupt routine.                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptSemaphore                     Semaphore Information Block                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_iReleaseSemaphore(grp_vos_t_semaphore *ptSemaphore) /* Semaphore Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Release Semaphore */
    uiErcd = tx_semaphore_put( &ptSemaphore->csem );

    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErrorCode Change (tx_semaphore_put) */
        switch (uiErcd)
        {
            case    TX_SEMAPHORE_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}
