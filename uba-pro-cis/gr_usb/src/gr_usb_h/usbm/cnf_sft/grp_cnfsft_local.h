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
/*      grp_cnfsft_local.h                                                      1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# configuring software module local file                                     */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2006/08/04  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "DESCRIPTION" of the file header.                  */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_CNFSFT_LOCAL_H_
#define _GRP_CNFSFT_LOCAL_H_

/**** INCLUDE FILES *****************************************************************************/
#include    "grp_vos.h"
#include    "grp_usbc.h"


/**** INTERNAL DATA DEFINES *********************************************************************/

/* internal error code */
#define GRP_CNFSFT_MATCH                (0)
#define GRP_CNFSFT_FOUND_DEVICE         (1)

#define GRP_CNFSFT_VENDOR               (-10)
#define GRP_CNFSFT_DEVCLASS             (-11)
#define GRP_CNFSFT_NOMATCH              (-12)
#define GRP_CNFSFT_EX_ERROR             (-13)

/* Enumeretion Status */
#define GRP_CNFSFT_DEFAULT_PHASE        ((grp_u8) 0x00)     /* Defautlt status                  */
#define GRP_CNFSFT_CONNENCT_PHASE       ((grp_u8) 0x01)     /* Connected status                 */
#define GRP_CNFSFT_GET_LANGID_PHASE     ((grp_u8) 0x02)     /* Get Language ID                  */
#define GRP_CNFSFT_GET_MANUFACT_PHASE   ((grp_u8) 0x03)     /* Get Manufacture                  */
#define GRP_CNFSFT_GET_PRODUCT_PHASE    ((grp_u8) 0x04)     /* Get Product                      */
#define GRP_CNFSFT_GET_CONFIG_PHASE     ((grp_u8) 0x05)     /* Get ConfigDesc status            */
#define GRP_CNFSFT_GET_ALLCONFIG_PHASE  ((grp_u8) 0x06)     /* Get All ConfigDesc status        */
#define GRP_CNFSFT_GET_OS_STRING_PHASE  ((grp_u8) 0x07)     /* Get StringDesc status : for MTP  */
#define GRP_CNFSFT_GET_MS_DESC_PHASE    ((grp_u8) 0x08)     /* Get MSDesc status : for MTP      */
#define GRP_CNFSFT_MTP_DEVICE           ((grp_u8) 0x09)     /* MTP Device                       */
#define GRP_CNFSFT_NOMAL_DEVICE         ((grp_u8) 0x0a)     /* Normal Device                    */

/* Port Connect Status */
#define GRP_CNFSFT_DISCONNECT_STAT      ((grp_u8) 0x00)
#define GRP_CNFSFT_CONNECTED_STAT       ((grp_u8) 0x01)

/* Flag that shows whether to have notified upper layer */
#define GRP_CNFSFT_NOTIFIED_UPPER       ((grp_u8) 0x00)
#define GRP_CNFSFT_NOT_NOTIFY           ((grp_u8) 0x01)

/* Initial value of _grp_cnfsft_atached_dev's iIndex */
#define GRP_CNFSFT_NO_ASSIGNED          (-1)

#define GRP_CNFSFT_EF                   ((grp_u8)0xEF)
#define GRP_CNFSFT_CMN_CLASS            ((grp_u8)0x02)
#define GRP_CNFSFT_IAD                  ((grp_u8)0x01)

/* String Descriptor max size */
#define GRP_CNFSFT_STRING_SIZE          ((grp_u16)255)
/* Specified String Descriptor */
#define GRP_CNFSFT_STR_LANGID           ((grp_u8)0x00)

/* Wrapper structure */
typedef struct _grp_cnfsft_wrap_desc_tag
{
    grp_u8                          bLength;            /* Descriptor size                      */
    grp_u8                          bDescriptor;        /* Descriptor type                      */
    grp_u8                          bOffset2;           /* Offset 2                             */
    grp_u8                          bOffset3;           /* Offset 3                             */

} _grp_cnfsft_wrap_desc;

/* Structure of the Device informations */
typedef struct _grp_cnfsft_atached_dev_tag
{
    grp_si                          iIndex;
    grp_si                          iNotifyFlag;
    grp_cnfsft_notify               tNotify;

} _grp_cnfsft_atached_dev;

/* Structure of the Device informations */
typedef struct _grp_cnfsft_dev_info_tag
{
    grp_u8                          *pucDevDesc;        /* Device Descriptor area               */
    grp_u8                          *pucConfDesc;       /* Configuration Descriptor area        */
    grp_u8                          ucEnumStat;         /* Enumeretion Status                   */
    grp_u8                          ucConnStat;         /* Port Connect Status                  */
    grp_u8                          ucCnfIdx;           /* Configured Index Numnber             */
    grp_u8                          ucMngCnt;
    grp_u8                          ucUnRegFlag;        /* This flag indicate Unregistered      */
                                                        /*  device was attached                 */
    grp_u8                          ucTmpFlag;          /* temporary flag (set by xx_cfg.c file)*/
    grp_u16                         usLangId;           /* Language IDs   (set by xx_cfg.c file)*/
    _grp_cnfsft_atached_dev         atIndex[GRP_USBD_HOST_MAX_CLASS];

} _grp_cnfsft_dev_info;

#ifdef GRP_USB_USE_MTP
/* Structure of the OS string */
typedef struct _grp_cnfsft_os_string_tag
{
    grp_u8                          bLength;            /* Length of the descriptor             */
    grp_u8                          bDescriptorType;    /* String Descriptor                    */
    grp_u8                          qwSignature[14];    /* Signature field 'MSFT100'            */
    grp_u8                          bMS_VendorCode;     /* Vendor code to fetch other OS Feature*/
                                                        /*  Descriptors                         */
    grp_u8                          bPad;               /* Pad field                            */
} _grp_cnfsft_os_string;
#endif  /* GRP_USB_USE_MTP */

/* Structure of Internal informations */
typedef struct _grp_cnfsft_internal_info_tag
{
    grp_vos_t_semaphore             *ptCnfInfoSem;
    grp_vos_t_semaphore             *ptCnfEnmSem;
    grp_u16                         usRegCnt;
    grp_u8                          aucPad[2];
    grp_usbdi_st_device_request     tStDevReq;
#ifdef GRP_USB_USE_MTP
    _grp_cnfsft_os_string           *ptString;
    grp_u8                          *pucMsDesc;
    grp_usbdi_device_request        tDevReq;
#endif  /* GRP_USB_USE_MTP */

} _grp_cnfsft_internal_info;


/* Custome Code */
#define GRP_CNFSFT_CHK_LANGID_PHASE     ((grp_u8)0x01)
#define GRP_CNFSFT_CHK_MANUFACT_PHASE   ((grp_u8)0x02)
#define GRP_CNFSFT_CHK_PRODUCT_PHASE    ((grp_u8)0x03)
#define GRP_CNFSFT_CHK_CONFIG_PHASE     ((grp_u8)0x04)

EXTERN grp_s32 _grp_cnfsft_CustomisedFunction( grp_u8 ucCustomeCode, _grp_cnfsft_dev_info *ptDevInfo);

#endif  /* _GRP_CNFSFT_LOCAL_H_ */

