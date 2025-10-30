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
/*      grp_usbc.h                                                              1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# common header file                                                         */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created first version based on the 1.00b                        */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "FILE NAME" and "DESCRIPTION" of the file header.  */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_USBC_H_
#define _GRP_USBC_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_usbc_types.h"
#include "grp_usbc_cfg.h"
#include "grp_usbc_anmly.h"
#include "grp_usbd.h"
#include "grp_cnfsft.h"
#include "grp_mdl_id.h"
#include "grp_cmem.h"


/**** Sub module id definition ******************************************************************/
#define GRP_USBD_MDL_ID                 (GRP_USBC_MDL_ID + 0x01)
#define GRP_COMMON_MDL_ID               (GRP_USBC_MDL_ID + 0x02)
#define GRP_CNFSFT_MDL_ID               (GRP_USBC_MDL_ID + 0x03)


/**** Error Code *******************************************************************************/
#define GRP_USBC_OK                     0
#define GRP_USBC_ILLEGAL_ERROR          -1
#define GRP_USBC_INIT_ERROR             -2


/**** Function Prototype ***********************************************************************/
EXTERN grp_s32 grp_usbc_HostInit(void);
EXTERN grp_s32 grp_usbc_Enable(void);
EXTERN grp_s32 grp_usbc_Disable(void);


#endif /* _GRP_USBC_H_ */
