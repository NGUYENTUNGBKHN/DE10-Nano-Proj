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
/*      grp_usbc_cfg.h                                                          1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# configuration file                                                         */
/*                                                                                              */
/* DEPENDENCIES                                                                                 */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                              Created initial version based on the 1.00b of grusbcom.h        */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Deleted definitions of peripheral controller driver.            */
/*   K.Kaneko       2015/12/07  V1.07                                                           */
/*                            - Changed the following default values.                           */
/*                              - GRP_USB_HOST_DEVICE_MAX                                       */
/*                              - GRP_USB_HOST_CLASS_MAX                                        */
/*                              - GRP_USB_HOST_REGS_MAX                                         */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_USBC_CFG_H_
#define _GRP_USBC_CFG_H_

/************************************************************************************************/
/*      Common definition                                                                       */
/************************************************************************************************/
/* --- CPU endian --- */
#define GRP_USB_COMMON_ENDIAN           0               /* 0 : Little endian                    */
                                                        /* 1 : Big endian                       */


/************************************************************************************************/
/*      Host Driver definition                                                                  */
/************************************************************************************************/
/* --- Parameter Checking --- */
#if(0)
#define GRP_USB_HOST_NO_PARAM_CHECKING                  /* Parameter checking, if comment       */
#endif

/* --- Number of items --- */
#define GRP_USB_HOST_CONTROLLER_NUM     1               /* Number of Controller                 */
#define GRP_USB_HOST_DEVICE_MAX         1               /* Max Connect Device                   */
#define GRP_USB_HOST_CLASS_MAX          1               /* Number of Max management classes     */
#define GRP_USB_HOST_REGS_MAX           2/*1*/               /* Number of Max registration           */


/* --- Transfer mode --- */
#if(0)
#define GRP_USB_HOST_USE_ISOCHRONOUS                    /* Use isochronous transfer             */
#endif
#if(0)
#define GRP_USB_HOST_USE_CTRL_NML_PIPE                  /* Use normal pipe control transfer     */
#endif


/* --- HUB Class Driver --- */
#if(0)
#define GRP_USB_HOST_USE_HUB_DRIVER                     /* Hub driver                           */
#endif


#endif /* _GRP_USBC_CFG_H_ */

