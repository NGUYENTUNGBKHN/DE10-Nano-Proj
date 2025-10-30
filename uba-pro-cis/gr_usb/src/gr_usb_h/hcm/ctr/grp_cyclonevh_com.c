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
/*      grp_cyclonevh_com.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Controller common function                                                 */
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
#include "grp_ctr.h"
#include "grp_usbc.h"
#include "grp_cyclonevh.h"

/**** INTERNAL DATA DEFINES *********************************************************************/

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/


/************************************************************************************************/
/* FUNCTION   : grp_hcmctr_Init                                                                 */
/*                                                                                              */
/* DESCRIPTION: GR-USB USB controller initialize function                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCMCTR_OK                   success                                         */
/*              GRP_HCMCTR_ERROR                error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_hcmctr_Init(void)
{
grp_s32                 lStat;

    /*--- Initialize of host controller ---*/
    lStat = grp_cyclonevh_HcInit();
    if (lStat != GRP_HCDI_OK) {
        /* error */
        return GRP_HCMCTR_ERROR;                                        /* DIRECT RETURN    */
    }
    
    return GRP_HCMCTR_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_hcmctr_StartIntr                                                            */
/*                                                                                              */
/* DESCRIPTION: GR-USB USB controller start of interrupt function                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCMCTR_OK                   success                                         */
/*              GRP_HCMCTR_ERROR                error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_hcmctr_StartIntr(void)
{
    /* The processing of the RHUB task is permitted */
    grp_cyclonevh_RhEnable(GRP_USB_TRUE);

    return GRP_HCMCTR_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_hcmctr_StopIntr                                                             */
/*                                                                                              */
/* DESCRIPTION: GR-USB USB controller stop of interrupt function                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_HCMCTR_OK                   success                                         */
/*              GRP_HCMCTR_ERROR                error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32  grp_hcmctr_StopIntr(void)
{
    /* The processing of the RHUB task is not permitted */
    grp_cyclonevh_RhEnable(GRP_USB_FALSE);

    return GRP_HCMCTR_OK;
}
