/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2008 Grape Systems, Inc.                          */
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
/*      grp_msc_local.h                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Local Header.                                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created new policy initial version                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_MSC_LOCAL_H_
#define _GRP_MSC_LOCAL_H_


/***** INCLUDE FILES ****************************************************************************/
#include "grp_vos.h"
#include "grp_usbc.h"
#include "grp_msc_config.h"
#include "grp_msc_drv.h"
#include "grp_msc_debug.h"

#ifdef GRP_MSC_USE_BOT
#include "grp_msc_bot.h"
#endif /* GRP_MSC_USE_BOT */

#ifdef GRP_MSC_USE_CBI
#include "grp_msc_cbi.h"
#endif /* GRP_MSC_USE_BOT */

#ifdef GRP_MSC_USE_ATAPI
#include "grp_msc_atapi.h"
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
#include "grp_msc_sff8070i.h"
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
#include "grp_msc_scsi.h"
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
#include "grp_msc_ufi.h"
#endif /* GRP_MSC_USE_UFI */


/***** MACRO DEFINITIONS ************************************************************************/

#if GRP_USB_COMMON_ENDIAN   /* Big endian       */
    #define     grp_msc_swap16(x)   grp_std_swap16(x)
    #define     grp_msc_swap32(x)   grp_std_swap32(x)

#else                       /* Little endian    */
    #define     grp_msc_swap16(x)   (x)
    #define     grp_msc_swap32(x)   (x)

#endif  /* GRP_USB_COMMON_ENDIAN */


#endif /* _GRP_MSC_LOCAL_H_ */
