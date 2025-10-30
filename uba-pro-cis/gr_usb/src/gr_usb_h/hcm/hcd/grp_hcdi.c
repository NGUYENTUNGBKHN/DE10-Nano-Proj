/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2019 Grape Systems, Inc.                             */
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
/*      grp_hcdi.c                                                              1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB Host controller driver initialize function                                       */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kaneko       2019/04/26  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                              (based on GR-USB/HOST# for STM32Fx(OTG_HS) V1.01)               */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_std_types.h"
#include "grp_usbc.h"

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
extern grp_s32 grp_cyclonevh_Init(grp_hcdi_system_init *ptHcdInfo);

/************************************************************************************************/
/* FUNCTION   : GRP_HCD_Init                                                                    */
/*                                                                                              */
/* DESCRIPTION: Host controller driver initialization                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptHcdInit                       System information                              */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCDI_OK                     Success                                         */
/*              GRP_HCDI_ILLEGAL_ERROR          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_hcm_Init( grp_hcdi_system_init *ptHcdInit)
{
grp_s32                         lStatus = GRP_HCDI_OK;

    /* STM32Fx(OTG_HS) host controller driver initialize */
    lStatus = grp_cyclonevh_Init(ptHcdInit);
    if (lStatus != GRP_HCDI_OK) {
        /* error */
        lStatus = GRP_HCDI_ILLEGAL_ERROR;
    }

    return lStatus;
}

