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
/*      grp_msc_cbi.h                                                           1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Control/Bulk/Interrupt Transport.                */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                              Created new policy initial version                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_MSC_CBI_H_
#define _GRP_MSC_CBI_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_msc_local.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
/* Command */
#define GRUSB_CBI_REQ_COMMAND           0x00                                /* bRequest         */

/* Status */
#define GRP_CBI_COMMAND_PASSED          0x00                                /* Pass             */
#define GRP_CBI_COMMAND_FAILED          0x01                                /* Fail             */
#define GRP_CBI_PHASE_ERROR             0x02                                /* Phase error      */
#define GRP_CBI_PERSIST_FAILED          0x03                                /* Persistent Fail  */

/* Function Prototypes */
EXTERN grp_s32 grp_msc_CbiInit( grp_msc_seq *, void * );


#endif /* _GRP_MSC_CBI_H_ */
