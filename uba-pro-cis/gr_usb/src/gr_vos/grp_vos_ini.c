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
/*      grp_vos_ini.c                                                           2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Initialize/Terminate Function Submodule                                                 */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*   M.Kitahara     2015/01/31  2.01                                                            */
/*                              Added tx_byte_pool_delete in grp_vos_Exit for pass a test       */
/*                              Changed the variable name in Internal Data Definitions.         */
/*                              Deleted Search empty record initialize process.                 */
/*                              Added preprocessor for GRP_VOS_MAX_xxx.                         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/
grp_s32                         _GRP_VOS_OsInitializeState = 0;/* Uninitialized=0/Initialized=1 */
TX_BYTE_POOL                    grp_vos_systemmemory;

DLOCAL grp_u8                   l_aucGrpVosSysMemBuffer[GRP_VOS_SYSTEM_MEMORY_SIZE] ={0,};
void                            *g_vpGrpVosFirstUnusedMemory = (void *)l_aucGrpVosSysMemBuffer;

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/


/************************************************************************************************/
/* FUNCTION   : grp_vos_Init                                                                    */
/*                                                                                              */
/* DESCRIPTION: Initialize VOS.                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_LOW_RESOURCE        Insufficient Resources.                         */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_Init(void)
{
void                            *vpFirstMemory = g_vpGrpVosFirstUnusedMemory;
grp_ui                          uiErcd;
grp_s32                         lResult = GRP_VOS_POS_RESULT;

    /* Initialize VOS object table */
#if (GRP_VOS_MAX_FLG)
    grp_std_memset( &_tFlagTable         , 0, sizeof(_tFlagTable) );
#endif
#if (GRP_VOS_MAX_MPL)
    grp_std_memset( &_tMemoryPoolTable   , 0, sizeof(_tMemoryPoolTable) );
#endif
#if (GRP_VOS_MAX_PPL)
    grp_std_memset( &_tPartitionPoolTable, 0, sizeof(_tPartitionPoolTable) );
#endif
#if (GRP_VOS_MAX_QUE)
    grp_std_memset( &_tQueueTable        , 0, sizeof(_tQueueTable) );
#endif
#if (GRP_VOS_MAX_SEM)
    grp_std_memset( &_tSemaphoreTable    , 0, sizeof(_tSemaphoreTable) );
#endif
#if (GRP_VOS_MAX_TSK)
    grp_std_memset( &_tTaskTable         , 0, sizeof(_tTaskTable) );
#endif

    uiErcd = tx_byte_pool_create( &grp_vos_systemmemory,
                                  (CHAR *)"VOS_MEM",
                                  vpFirstMemory,
                                  GRP_VOS_SYSTEM_MEMORY_SIZE );

    /* TX ErrorCode Change (tx_byte_pool_create) */
    if (uiErcd != TX_SUCCESS)
    {
        lResult = GRP_VOS_NEG_LOW_RESOURCE;
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_Exit                                                                    */
/*                                                                                              */
/* DESCRIPTION: Terminate VOS.                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_Exit(void)
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

    /* Delete VOS_MEM that was created in grp_vos_Init */
    tx_byte_pool_delete( &grp_vos_systemmemory );

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_OsInitialized                                                           */
/*                                                                                              */
/* DESCRIPTION: Base OS initialize complete.                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void    grp_vos_OsInitialized(void)
{
    _GRP_VOS_OsInitializeState = 1;

    return;
}

/************************************************************************************************/
/* FUNCTION   : grp_vos_IsOsInitialized                                                         */
/*                                                                                              */
/* DESCRIPTION: Query Base OS initialize state.                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              OS initialize completed.                        */
/*              GRP_VOS_NEG_NOT_INIT            OS not initialized.                             */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_IsOsInitialized(void)
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;

    if (_GRP_VOS_OsInitializeState != 1)
        lResult = GRP_VOS_NEG_NOT_INIT;

    return(lResult);
}

