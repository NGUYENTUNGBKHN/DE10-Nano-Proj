/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2008-2018 Grape Systems, Inc.                          */
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
/*      grp_cnfsft_cfg.c                                                        1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# configuring software module configuration file                             */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2008/01/24  V0.90                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include    "grp_usbc.h"
#include    "grp_cnfsft.h"
#include    "grp_cnfsft_local.h"


/************************************************************************************************/
/* FUNCTION   : _grp_cnfsft_CustomisedFunction                                                  */
/*                                                                                              */
/* DESCRIPTION: Please describe processing if necessary.                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucCustomeCode                   Specified code to customise                     */
/*              ptDevInfo                       Device informations                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CNFSFT_OK                   Success                                         */
/*              GRP_CNFSFT_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _grp_cnfsft_CustomisedFunction( grp_u8 ucCustomeCode, _grp_cnfsft_dev_info *ptDevInfo)
{
grp_s32                         lStat = GRP_CNFSFT_OK;

    switch (ucCustomeCode) {
    case GRP_CNFSFT_CHK_LANGID_PHASE:
        break;

    case GRP_CNFSFT_CHK_MANUFACT_PHASE:
        break;

    case GRP_CNFSFT_CHK_PRODUCT_PHASE:
        break;

    case GRP_CNFSFT_CHK_CONFIG_PHASE:
        break;

    default:
        break;
    }

    return lStat;
}

