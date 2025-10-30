/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2015 Grape Systems, Inc.                          */
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
/*      grp_msc_config.h                                                        1.02            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Configuration header file.                       */
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
/*   K.Kaneko       2015/12/21  V1.02                                                           */
/*                            - Changed the following default values.                           */
/*                              - GRP_MSC_REG_MAX                                               */
/*                              - GRP_MSC_USE_CBI                                               */
/*                              - GRP_MSC_USE_ATAPI                                             */
/*                              - GRP_MSC_USE_SFF8070I                                          */
/*                              - GRP_MSC_USE_UFI                                               */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_MSC_CONFIG_H_
#define _GRP_MSC_CONFIG_H_

/***** INCLUDE FILES ****************************************************************************/


/***** USER CONFIGURATIONS **********************************************************************/
/* Need BOT Protocol */
#define GRP_MSC_USE_BOT                                             /* If comment out, no use   */

/* Need CBI Protocol */
/* #define GRP_MSC_USE_CBI */                                       /* If comment out, no use   */

/* Need ATAPI Sub Class */
/* #define GRP_MSC_USE_ATAPI */                                     /* If comment out, no use   */

/* Need SCSI Sub Class */
#define GRP_MSC_USE_SCSI                                            /* If comment out, no use   */

/* Need SFF8070i Sub Class */
/* #define GRP_MSC_USE_SFF8070I */                                  /* If comment out, no use   */

/* Need UFI Sub Class */
/* #define GRP_MSC_USE_UFI */                                       /* If comment out, no use   */

/* Used MS Reset in BOT */
#define GRP_MSC_BOT_MS_RESET                                        /* If comment out, no use   */
/* Used Reset in CBI */
#define GRP_MSC_CBI_MS_RESET                                        /* If comment out, no use   */

/* Hoped number of connection */
#define GRP_MSC_DEVICE_MAX              GRP_USB_HOST_DEVICE_MAX

/* Hoped number of register */
#define GRP_MSC_REG_MAX                 (1)           /* Normally, atapi, scsi, sff of bot only */

/* Retry max if error */
#define GRP_MSC_MAX_RETRY               3


/***** SYSTEM CONFIGURATIONS *************************************************/
/* Vendor subclass valid on fsif */
#define GRP_MSC_USE_VENDOR_SUBCLASS                 /* If you want to use the vendor class      */
                                                    /*  through gr_fsif. In this case call      */
                                                    /*  grp_fsif_Init before grp_msc_Register   */

/* Need exclusive transfer between interfaces */
/* #define  GRP_MSC_EXCLUSIVE_TRANSFER */           /* If comment out, concurrently transfer    */


#endif /* _GRP_MSC_CONFIG_H_ */
