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
/*      grp_vos_flg.c                                                           2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Event Flag Function Submodule                                                           */
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
/*                              to grp_vos_CreateFlag.                                          */
/*                              The changed from "wai_option" to "ulWaitOption" in              */
/*                              grp_vos_WaitFlag.                                               */
/*                              The changed from "( grp_u32 )ulTimeout" to                      */
/*                              "_grp_vos_ConvertTimeUnit( ulTimeout )" in grp_vos_WaitFlag.    */
/*                              Change if statement after the tx_event_flags_create             */
/*                              in grp_vos_CreateFlag.                                          */
/*                              Deleted "set Event Flag ID" process.                            */
/*                              Added preprocessor for GRP_VOS_MAX_FLG.                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"
#include "tx_event_flags.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/
#if (GRP_VOS_MAX_FLG)
grp_vos_t_flag                  _tFlagTable[GRP_VOS_MAX_FLG];
#endif

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/

/************************************************************************************************/
/* FUNCTION   : grp_vos_CreateFlag                                                              */
/*                                                                                              */
/* DESCRIPTION: Create Event Flag.                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pptFlag                         Pointer of Event Flag Information Block Area    */
/*              pucName                         Event Flag Name                                 */
/* OUTPUT     : pptFlag                         Event Flag Information Block                    */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_CreateFlag(grp_vos_t_flag **pptFlag,           /* Event Flag Information Block */
                            grp_u8         *pucName)            /* Event Flag Name              */
{
#if (GRP_VOS_MAX_FLG)
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;
grp_si                          i;

    /* Search empty record from Semaphore Table */
    for (i = 0; i < GRP_VOS_MAX_FLG; i++)
    {
        if (_tFlagTable[i].aucName[0] == 0)
        {
            break;
        }
    }

    /* If found empty record */
    if (i < GRP_VOS_MAX_FLG)
    {
        /* Parameter check */
        if (((void *)pucName != NULL) &&
           (grp_std_strlen( (grp_u8 *)pucName ) <= GRP_VOS_NAME_SIZE))
        {
            /* Initialize Event Flag Information Block */
            grp_std_strncpy( _tFlagTable[i].aucName, (grp_u8 *)pucName, GRP_VOS_NAME_SIZE );
            _tFlagTable[i].aucName[GRP_VOS_NAME_SIZE] = 0;

            /* Create Event Flag */
            uiErcd = tx_event_flags_create( &_tFlagTable[i].group_ptr, (CHAR *)_tFlagTable[i].aucName );

            /* Error Change to VOS ErrorCode */
            if (uiErcd == TX_SUCCESS)
            {
                *pptFlag = &_tFlagTable[i];
            }
            /* Not TX_SUCCESS */
            else
            {
                /* Clear event flag information */
                grp_std_memset( &_tFlagTable[i], 0, sizeof(grp_vos_t_flag) );

                /* TX ErrorCode Change (tx_event_flags_create) */
                switch (uiErcd)
                {
                    case    TX_GROUP_ERROR:
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
/* FUNCTION   : grp_vos_DeleteFlag                                                              */
/*                                                                                              */
/* DESCRIPTION: Delete Event Flag.                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFlag                          Event Flag Information Block                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*              GRP_VOS_NEG_NO_SUPPORT          Not Support.                                    */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_DeleteFlag(grp_vos_t_flag *ptFlag)             /* Event Flag Information Block */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

#if (GRP_VOS_DEL_FLG == GRP_VOS_USE)
grp_ui                          uiErcd;

    /* Clear Event Flag */
    uiErcd = tx_event_flags_delete( &ptFlag->group_ptr );

    /* Error Change to VOS ErrorCode */
    if (uiErcd == TX_SUCCESS)
    {
        /* Clear event flag information */
        grp_std_memset( ptFlag, 0, sizeof(grp_vos_t_flag) );
    }
    else
    {
        /* TX ErrorCode Change (tx_event_flags_delete) */
        switch (uiErcd)
        {
            case    TX_GROUP_ERROR:

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
#endif  /* GRP_VOS_DEL_FLG */

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_SetFlag                                                                 */
/*                                                                                              */
/* DESCRIPTION: Set Event Flag.                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFlag                          Event Flag Information Block                    */
/*              ulFlag                          Bit Pattern                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_SetFlag(grp_vos_t_flag *ptFlag,                /* Event Flag Information Block */
                         grp_u32        ulFlag)                 /* Bit Pattern                  */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Check event flag id */
    if (ptFlag->group_ptr.tx_event_flags_group_id != TX_EVENT_FLAGS_ID)
    {
        return(GRP_VOS_NEG_ILL_PARAMETER);
    }

    /* Set Event Flag */
    uiErcd = tx_event_flags_set( &ptFlag->group_ptr, (grp_u32)ulFlag, TX_OR );

    /* Error Change to VOS ErrorCode */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErroCode Change (tx_event_flags_set) */
        switch (uiErcd)
        {
            case    TX_GROUP_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_OPTION_ERROR:
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_iSetFlag                                                                */
/*                                                                                              */
/* DESCRIPTION: Set Event Flag for interrupt routine.                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFlag                          Event Flag Information Block                    */
/*              ulFlag                          Bit Pattern                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_iSetFlag(grp_vos_t_flag *ptFlag,               /* Event Flag Information Block */
                          grp_u32        ulFlag)                /* Bit Pattern                  */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Set Event Flag */
    uiErcd = tx_event_flags_set( &ptFlag->group_ptr, (grp_u32)ulFlag, TX_OR );

    /* Error Change to VOS ErrorCode */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErroCode Change (tx_event_flags_set) */
        switch (uiErcd)
        {
            case    TX_GROUP_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_OPTION_ERROR:
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_ClearFlag                                                               */
/*                                                                                              */
/* DESCRIPTION: Clear Event Flag.                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFlag                          Event Flag Information Block                    */
/*              ulFlag                          Bit Pattern                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_ClearFlag(grp_vos_t_flag *ptFlag,              /* Event Flag Information Block */
                           grp_u32        ulFlag)               /* Bit Pattern                  */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Clear Event Flag */
    uiErcd = tx_event_flags_set( &ptFlag->group_ptr, (grp_u32)ulFlag, TX_AND );

    /* Error Change to VOS ErrorCode */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErroCode Change (tx_event_flags_set) */
        switch (uiErcd)
        {
            case    TX_GROUP_ERROR:

                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
            case    TX_OPTION_ERROR:
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_WaitFlag                                                                */
/*                                                                                              */
/* DESCRIPTION: Wait Event Flag (mode is OR).                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFlag                          Event Flag Information Block                    */
/*              ulFlag                          Wait Bit Pattern                                */
/*              pulWflg                         Pointer of Current Bit Pattern Area             */
/*              ulTimeout                       Wait Timeout(ms)                                */
/* OUTPUT     : pulWflg                         Current Bit Pattern                             */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_ILL_PARAMETER       Illegal Parameter.                              */
/*              GRP_VOS_NEG_TIMEOUT             Timeout is occurred.                            */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_WaitFlag(grp_vos_t_flag *ptFlag,               /* Event Flag Information Block */
                          grp_u32        ulFlag,                /* Wait Bit Pattern             */
                          grp_u32        *pulWflg,              /* Current Bit Pattern          */
                          grp_u32        ulTimeout)             /* Wait Timeout(ms)             */
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

    uiErcd = tx_event_flags_get( &ptFlag->group_ptr, ulFlag, TX_OR, pulWflg, ulWaitOption );

    /* Error Change to VOS ErrorCode */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErroCode Change (tx_event_flags_get) */
        switch (uiErcd)
        {
            case    TX_WAIT_ABORTED:

                lResult = GRP_VOS_POS_RESULT;
                break;
            case    TX_NO_EVENTS:

                lResult = GRP_VOS_NEG_TIMEOUT;
                break;
            case    TX_DELETED:
            case    TX_GROUP_ERROR:
            case    TX_PTR_ERROR:
            case    TX_WAIT_ERROR:
            case    TX_OPTION_ERROR:
            default:
                /* Error No Object */
                lResult = GRP_VOS_NEG_ILL_PARAMETER;
                break;
        }
    }

    return(lResult);
}
