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
/*      grp_mdl_id.h                                                            1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# module identifier file                                                     */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  0.01                                                            */
/*                            - Created first version 1.00                                      */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Kaneko       2011/03/17  V1.03                                                           */
/*                            - Added ECM module number                                         */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_MDL_ID_H_
#define _GRP_MDL_ID_H_

/* Main module number */
#define GRP_ANY_MDL_ID                  0x00                        /* Any module               */
#define GRP_HCM_MDL_ID                  0x10                        /* HCD,USBCTR               */
#define GRP_USBC_MDL_ID                 0x20                        /* USBD,CNFSFT              */
#define GRP_HUB_MDL_ID                  0x30                        /* HUB                      */
#define GRP_HID_MDL_ID                  0x40                        /* HID                      */
#define GRP_MSC_MDL_ID                  0x50                        /* MSC                      */
#define GRP_FSIF_MDL_ID                 0x60                        /* FSIF                     */
#define GRP_SICD_MDL_ID                 0x70                        /* SICD                     */
#define GRP_CDC_MDL_ID                  0x80                        /* CDC                      */
#define GRP_ECM_MDL_ID                  0x81                        /* ECM                      */
#define GRP_PDC_MDL_ID                  0x90                        /* PRINTER                  */
#define GRP_AUDIO_MDL_ID                0xA0                        /* AUDIO                    */
#define GRP_VIDEO_MDL_ID                0xB0                        /* VIDEO                    */
#define GRP_CWL_MDL_ID                  0xC0                        /* Certified Wireless USB   */


/* Error code base */
#define GRP_RET_ERROR_TAG               0x80010000
#define GRP_TR_FAIL_TAG                 0x80030000
#define GRP_RET_ERROR_BASE(x)           (grp_s32)(GRP_RET_ERROR_TAG | ((grp_u32)(x) << 20))
#define GRP_TR_FAIL_BASE(x)             (grp_s32)(GRP_TR_FAIL_TAG | ((grp_u32)(x) << 20))


#endif /* _GRP_MDL_ID_H_ */
