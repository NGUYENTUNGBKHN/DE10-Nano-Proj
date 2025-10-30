/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2010 Grape Systems, Inc.                            */
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
/*      grp_ctr.h                                                               1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# controller dependent function.                                             */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2010/08/02  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CTRI_H_
#define _GRP_CTRI_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_std_types.h"
#include "grp_mdl_id.h"


/**** INTERNAL DATA DEFINES *********************************************************************/

/************************************************************************************************/
/* Sub module id definition                                                                     */
#define GRP_HCD_MDL_ID                  ((GRP_HCM_MDL_ID) + 0x01)
#define GRP_USBCTL_MDL_ID               ((GRP_HCM_MDL_ID) + 0x02)
#define GRP_CMEM_MDL_ID                 ((GRP_HCM_MDL_ID) + 0x03)
#define GRP_TARGET_MDL_ID               ((GRP_HCM_MDL_ID) + 0x04)
#define GRP_BUS_MDL_ID                  ((GRP_HCM_MDL_ID) + 0x05)
#define GRP_RHUB_MDL_ID                 ((GRP_HCM_MDL_ID) + 0x06)
#define GRP_CTRL_MDL_ID                 ((GRP_HCM_MDL_ID) + 0x07)
#define GRP_MGR_MDL_ID                  ((GRP_HCM_MDL_ID) + 0x08)


/************************************************************************************************/
/* Error base definition                                                                        */
#define GRP_HCD_RET_ERROR_BASE          GRP_RET_ERROR_BASE(GRP_HCD_MDL_ID)
#define GRP_USBCTR_RET_ERROR_BASE       GRP_RET_ERROR_BASE(GRP_USBCTL_MDL_ID)
#define GRP_TARGET_RET_ERROR_BASE       GRP_RET_ERROR_BASE(GRP_TARGET_MDL_ID)
#define GRP_CMEM_RET_ERROR_BASE         GRP_RET_ERROR_BASE(GRP_CMEM_MDL_ID)
#define GRP_BUS_RET_ERROR_BASE          GRP_RET_ERROR_BASE(GRP_BUS_MDL_ID)


/************************************************************************************************/
/* Ctrl error definition                                                                        */
#define GRP_HCMCTR_OK                   0
#define GRP_HCMCTR_ERROR                GRP_USBCTR_RET_ERROR_BASE-1


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
EXTERN grp_s32 grp_hcmctr_Init(void);
EXTERN grp_s32 grp_hcmctr_StartIntr(void);
EXTERN grp_s32 grp_hcmctr_StopIntr(void);

#endif /* _GRP_CTRI_H_ */

