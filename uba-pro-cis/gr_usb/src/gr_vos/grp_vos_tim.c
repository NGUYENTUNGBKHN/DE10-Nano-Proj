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
/*      grp_vos_tim.c                                                           2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*      Timer Function Submodule                                                                */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*   M.kitahara     2015/01/31  2.01                                                            */
/*                              Review and Change the error code for grp_vos_DelayTask.         */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_vos_local.h"

/**** INTERNAL DATA DEFINITIONS *****************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/


/************************************************************************************************/
/* FUNCTION   : grp_vos_DelayTask                                                               */
/*                                                                                              */
/* DESCRIPTION: Delay Task.                                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulDelayTime                     Delay Time(ms)                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Function Complete.                              */
/*              GRP_VOS_NEG_INV_STAT            Invalid Status.                                 */
/*              GRP_VOS_NEG_OS_ERROR            OS Error.                                       */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_vos_DelayTask(grp_u32 ulDelayTime)                               /* Delay Time(ms) */
{
grp_s32                         lResult = GRP_VOS_POS_RESULT;
grp_ui                          uiErcd;

    /* Delay Task */
    uiErcd = tx_thread_sleep( _grp_vos_ConvertTimeUnit(ulDelayTime) );
    /* If not SUCCESS then error return */
    if (uiErcd != TX_SUCCESS)
    {
        /* TX ErrorCode Change (tx_thread_sleep) */
        switch (uiErcd)
        {
            case    TX_WAIT_ABORTED:

                lResult = GRP_VOS_NEG_INV_STAT;
                break;
            case    TX_CALLER_ERROR:
            default:

                lResult = GRP_VOS_NEG_OS_ERROR;
                break;
        }
    }

    return(lResult);
}

/************************************************************************************************/
/* FUNCTION   : _grp_vos_ConvertTimeUnit                                                        */
/*                                                                                              */
/* DESCRIPTION: Convert to OS depend time unit from milliseconds time unit.                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulMilliSecondTime               Milli Second Time                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Tick                                                                            */
/*                                                                                              */
/************************************************************************************************/
grp_u32  _grp_vos_ConvertTimeUnit(grp_u32 ulMilliSecondTime)               /* Milli Second Time */
{
    return((grp_u32)(ulMilliSecondTime / GRP_VOS_TICK_INTERVAL));
}
