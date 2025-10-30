/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2020 Grape Systems, Inc.                            */
/*                                     All Rights Reserved.                                     */
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
/*      grp_usbc_reinit.c                                                       1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB re-initialization related functions                                              */
/*                                                                                              */
/* DEPENDENCIES                                                                                 */
/*                                                                                              */
/*      All GR-USB Header file                                                                  */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Kato         2020/01/10  V1.00                                                           */
/*                            - Created first version                                           */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_std_tools.h" /* memset, memcpy, memcmp */
#include "grp_usbc_types.h"
#include "grp_usbc_reinit.h"
#include "grp_usbc.h" /* grp_usbc_HostInit */
#include <ctype.h>  /* isprint */
#include <stdio.h> /* sprintf */

#include "grp_cyclonevh.h" /* grp_cyclonevh_SetVbusPower */

#define GRDBG_PRINT(x, ...)


/* External variable declaration */
grp_si _not_initialized = 1; /* reinit */


LOCAL grp_si _grp_reinit_is_connected();

/***** LOCAL PARAMETER DEFINITIONS **************************************************************/


/************************************************************************************************/
/* FUNCTION   : grp_reinit_HostReInit                                                           */
/*                                                                                              */
/* DESCRIPTION: Save register data used of USB controller                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : REINIT_STATUS_OK                Success                                         */
/*              REINIT_STATUS_NOT_INITIALIZED   Error                                           */
/*              REINTT_STATUS_INIT_ERROR        Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_reinit_HostReInit(void)
{

    if( 1==_not_initialized ) {
        return REINIT_STATUS_NOT_INITIALIZED;
    }

    /* stop interrupt */
    if (grp_target_StopIntr(GRP_CYCLONEV_MODE_HOST) != GRP_TARGET_OK) {
        return REINTT_STATUS_INIT_ERROR;
    }

    /* disable interrupt */
    if (grp_usbc_Disable() != GRP_USBC_OK) {
        return REINTT_STATUS_INIT_ERROR;
    }

    /* Port power off */
    if (grp_cyclonevh_SetVbusPower(GRP_CYCLONEVH_VBUS_POWER_OFF) != GRP_HCDI_OK)
    {
        return REINTT_STATUS_INIT_ERROR;
    }

    /* reinitialize DLOCAL variables */
    _grp_dlocal_init_usbd_c(); /* grp_usbc.c */
    if( GRP_USBC_OK!=grp_usbc_HostInit() ) {
        return REINTT_STATUS_INIT_ERROR;
    }

    /* enable interrupt */
    if (grp_usbc_Enable() != GRP_USBC_OK) {
        return REINTT_STATUS_INIT_ERROR;
    }

    /* start interrupt */
    if (grp_target_StartIntr(GRP_CYCLONEV_MODE_HOST) != GRP_TARGET_OK) {
        return REINTT_STATUS_INIT_ERROR;
    }

    return REINIT_STATUS_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_reinit_is_connected                                                        */
/*                                                                                              */
/* DESCRIPTION: check if USB device is connected to USB port or not                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : 1                               device is connected                             */
/*              0                               device is not connected                         */
/************************************************************************************************/
grp_si _grp_reinit_is_connected()
{
    grp_si                           connected = 0;
    grp_s32                         ulData;

    return connected;
}
