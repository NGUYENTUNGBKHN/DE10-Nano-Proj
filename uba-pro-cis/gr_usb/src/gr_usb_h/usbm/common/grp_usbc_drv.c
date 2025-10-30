/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2018 Grape Systems, Inc.                          */
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
/*      grp_usbc_drv.c                                                          1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# common function                                                            */
/*                                                                                              */
/* DEPENDENCIES                                                                                 */
/*                                                                                              */
/*      All GR-USB Header file                                                                  */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created first version based on the 1.00b                        */
/*                              of grusb.c                                                      */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_ctr.h"
#include "grp_usbc.h"

#include "grp_usbc_reinit.h" /* reinit */

#ifdef GRP_USB_HOST_USE_HUB_DRIVER
#include "grp_hubd.h"
#endif  /* GRP_USB_HOST_USE_HUB_DRIVER */

/************************************************************************************************/
/* FUNCTION   : grp_usbc_HostInit                                                               */
/*                                                                                              */
/* DESCRIPTION: GR-USB Class Driver Initialize function                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK                     Success                                         */
/*              GRP_USBC_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbc_HostInit(void)
{
grp_usbdi_system_init           tInit;
grp_s32                         lStatus;

    /* initialize common memory module */
    if(_not_initialized){
        lStatus = grp_cmem_PoolInit();
        if (lStatus != GRP_CMEM_OK) {
            return GRP_USBC_INIT_ERROR;
        }
    } /* reinit */

    /* configuring software Initialize */
    lStatus = grp_cnfsft_Init();
    if (lStatus != GRP_CNFSFT_OK) {
        return GRP_USBC_INIT_ERROR;
    }

    /* Initialize USB controller (Common Initialize) */
    lStatus = grp_hcmctr_Init();
    if (lStatus != GRP_HCMCTR_OK) {
        return GRP_USBC_INIT_ERROR;
    }

    /* initialize host driver */
    lStatus = grp_usbd_Init(&tInit);
    if (lStatus != GRP_USBD_OK) {
        return GRP_USBC_INIT_ERROR;
    }

#ifdef GRP_USB_HOST_USE_HUB_DRIVER
    /* initialeze HUB class driver, if it is used */
    if(_not_initialized){
        lStatus = grp_hubd_Init();
        if (lStatus != GRP_HUBD_OK) {
            return GRP_USBC_INIT_ERROR;
        }
    } /* reinit */
#endif  /* GRP_USB_HOST_USE_HUB_DRIVER */

#ifdef REINIT_TEST
    /* save register/memory values */
    if( REINIT_STATUS_OK != grp_reinit_RegSave( E_REINIT_SAVE_INIT ) ) { /* reinit */
        return GRP_USBC_INIT_ERROR;
    }
    if( REINIT_STATUS_OK != grp_reinit_MemSave( E_REINIT_SAVE_INIT ) ) { /* reinit */
        return GRP_USBC_INIT_ERROR;
    }
#endif /* REINIT_TEST */

    _not_initialized = 0; /* reinit */

    return GRP_USBC_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbc_Enable                                                                 */
/*                                                                                              */
/* DESCRIPTION: GR-USB Enable Function                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK                     Success                                         */
/*              GRP_USBC_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbc_Enable(void)
{
grp_s32                         lStatus;


    /* start USB controller interrupt (Common Interrupt) */
    lStatus = grp_hcmctr_StartIntr();
    if (lStatus != GRP_HCMCTR_OK) {
        return GRP_USBC_ILLEGAL_ERROR;
    }

    return GRP_USBC_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_usbc_Disable                                                                */
/*                                                                                              */
/* DESCRIPTION: GR-USB Disable Function                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK                     Success                                         */
/*              GRP_USBC_ERROR                  Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_usbc_Disable(void)
{
grp_s32                         lStatus;


    /* stop USB controller interrupt (Common Interrupt) */
    lStatus = grp_hcmctr_StopIntr();
    if (lStatus != GRP_HCMCTR_OK) {
        return GRP_USBC_ILLEGAL_ERROR;
    }

    return GRP_USBC_OK;
}

