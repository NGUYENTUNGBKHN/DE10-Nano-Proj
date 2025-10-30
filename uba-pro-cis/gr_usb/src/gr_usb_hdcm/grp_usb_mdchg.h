/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2020 Grape Systems, Inc.                            */
/*                                       All Rights Reserved                                    */
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
/*      grp_usb_mdchg.h                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB DEVICE/HOST mode change module header                                            */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Kato         2020/01/10  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_USB_MDCHG_H_
#define _GRP_USB_MDCHG_H_

#include "grusbtyp.h"

/**** DEFINES ***********************************************************************************/
/* Return Code */
#define GRP_MDCHG_OK            (0)
#define GRP_MDCHG_NG            (-1)

/* Message */
typedef enum
{
    GRP_MODE_HOST               = 0,                            /* �z�X�g�N��                   */
    GRP_MODE_DEVICE,                                            /* �f�o�C�X�N��                 */
    GRP_MEDIA_ATTACHED,                                         /* ���f�B�A�ڑ��i�z�X�g���j     */
    GRP_MEDIA_DETACHED                                          /* ���f�B�A�ؒf�i�z�X�g���j     */
} GRP_USB_MDCHG_MSG;

/* Function type */
typedef VOID (*GRP_USB_MDCHG_USR_1ST_DEVINIT)( VOID );
typedef VOID (*GRP_USB_MDCHG_USR_2ND_DEVINIT)( VOID );
//typedef VOID (*GRP_USB_MDCHG_USR_1ST_HOSTINIT)( VOID );
typedef INT (*GRP_USB_MDCHG_USR_1ST_HOSTINIT)( VOID );


/**** STRUCTURES ********************************************************************************/
typedef struct t_grp_usb_mdchg_initparam {
    GRP_USB_MDCHG_USR_1ST_DEVINIT   pfn1stDevInitFunc;
    GRP_USB_MDCHG_USR_2ND_DEVINIT   pfn2ndDevInitFunc;
    GRP_USB_MDCHG_USR_1ST_HOSTINIT  pfn1stHostInitFunc;
} GRP_USB_MDCHG_INITPARAM;

/**** EXTERNAL FUNCTIONS ************************************************************************/
INT grp_usb_mdchg_Init( VOID );
INT grp_usb_mdchg_SetFunc( GRP_USB_MDCHG_INITPARAM *ptParam );
INT grp_usb_mdchg_WaitMdChg( UINT32 *pulMode );
//INT grp_usb_mdchg_BootDevice(VOID);
INT grp_usb_mdchg_BootDevice(UINT32 Port2nd);
INT grp_usb_mdchg_BootHost(VOID);
INT grp_usb_mdchg_SendMediaState(GRP_USB_MDCHG_MSG eMedia);
//VOID grp_lpc43xx_Intr_Entry( VOID );
void grp_usb_mdchg_ConidIsr( void );
#endif



