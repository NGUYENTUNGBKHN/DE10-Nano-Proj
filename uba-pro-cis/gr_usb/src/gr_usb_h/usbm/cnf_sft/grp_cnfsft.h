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
/*      grp_cnfsft.h                                                            1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# configuring software module header file                                    */
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
#ifndef _GRP_CNFSFT_H_
#define _GRP_CNFSFT_H_

/**** INCLUDE FILES *****************************************************************************/
#include    "grp_usbc_types.h"


/**** INTERNAL DATA DEFINES *********************************************************************/

/* error code */
#define GRP_CNFSFT_OK                   (0)
#define GRP_CNFSFT_MULTIPLE_SETTING     (1)

#define GRP_CNFSFT_ERROR_BASE           GRP_RET_ERROR_BASE(GRP_CNFSFT_MDL_ID)
#define GRP_CNFSFT_ERROR                (GRP_CNFSFT_ERROR_BASE-1)

/* Event */
#define GRP_CNFSFT_DEVICE_ATTACHED      ((grp_u16) 0x0001)
#define GRP_CNFSFT_DEVICE_DETACHED      ((grp_u16) 0x0002)
#define GRP_CNFSFT_DEVICE_DISABLED      ((grp_u16) 0x0003)
#define GRP_CNFSFT_DEVICE_ENABLED       ((grp_u16) 0x0004)

/* Device search options */
#define GRP_CNFSFT_VENDOR_SPECIFIED     ((grp_u16) 0x0000)
#define GRP_CNFSFT_DEVCLASS_SPECIFIED   ((grp_u16) 0x0001)
#define GRP_CNFSFT_INFCLASS_SPECIFIED   ((grp_u16) 0x0002)
#define GRP_CNFSFT_SUBCLASS_SPECIFIED   ((grp_u16) 0x0003)
#define GRP_CNFSFT_PROTOCOL_SPECIFIED   ((grp_u16) 0x0004)
#define GRP_CNFSFT_MTP_DEV_SPECIFIED    ((grp_u16) 0x0005)
#define GRP_CNFSFT_INVALID_SPECIFIED    ((grp_u16) 0x0006)


/* Special configuraiton index number */
#define GRP_CNFSFT_SPECIAL_NUMBER       ((grp_u8) 0xFF)

/* Structure of Notification informations */
typedef struct grp_cnfsft_notify_tag
{
    grp_u16     usEvent;
    grp_u16     usUsbDevId;
    grp_u16     usConfigIdx;
    grp_u8      ucInterfaceNum;
    grp_u8      ucTotalIfNum;
    grp_u8      *pucInterfaceDesc;
    grp_u8      *pucDevDesc;
    grp_u8      *pucConfDesc;
    void        *pvReference;
    
} grp_cnfsft_notify;


/* Structure of Registration informations */
typedef struct grp_cnfsft_registration_tag
{
    grp_u16     usVendorID;
    grp_u16     usProductID;
    grp_u8      ucInterfaceClass;
    grp_u8      ucInterfaceSubClass;
    grp_u8      ucInterfaceProtocol;
    grp_u8      ucDeviceClass;
    grp_u16     usLoadOption;
    grp_u8      aucPad[2];
    grp_s32     (*pfnEventNotification)(grp_cnfsft_notify *);
    void        *pvReference;

} grp_cnfsft_registration;


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
EXTERN grp_s32  grp_cnfsft_Init(void);
EXTERN grp_s32  grp_cnfsft_SetNotification( grp_cnfsft_registration *ptInput );
EXTERN grp_s32  grp_cnfsft_GetDescFromUsbDevId( grp_u16 usUsbDevId, void **ppvDevDesc, void **ppvConfDesc);
EXTERN grp_s32  grp_cnfsft_ChangeConfiguration( grp_u16 usDevID, grp_u8 ucConfigIdx);
EXTERN void     grp_cnfsft_ConfigSoftware( grp_u16 usEvent, grp_u16 usUsbDevId );


#endif  /* _GRP_CNFSFT_H_ */

