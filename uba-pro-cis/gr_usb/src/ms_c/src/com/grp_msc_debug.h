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
/*      grp_msc_debug.h                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Debug module.                                    */
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

#ifndef _GRP_MSC_DEBUG_H_
#define _GRP_MSC_DEBUG_H_


/**** INCLUDE FILES *****************************************************************************/
#include "grp_usbc_dbg.h"


/**** DEBUG LOGGING OPTION **********************************************************************/
#ifdef GRP_USBC_DBG_MDL
#define GRP_MSC_DEBUG_LOGGING
#endif


/* Debug module number. GRP_DBG_MSC_BASE( 0x50 ) */
#define GRP_DBG_MSC_DRV             (GRP_DBG_MSC_BASE+1)
#define GRP_DBG_MSC_WRAP            (GRP_DBG_MSC_BASE+2)
#define GRP_DBG_MSC_BOT             (GRP_DBG_MSC_BASE+3)
#define GRP_DBG_MSC_CBI             (GRP_DBG_MSC_BASE+4)
#define GRP_DBG_MSC_SCSI            (GRP_DBG_MSC_BASE+5)
#define GRP_DBG_MSC_ATAPI           (GRP_DBG_MSC_BASE+6)
#define GRP_DBG_MSC_SFF8070i        (GRP_DBG_MSC_BASE+7)
#define GRP_DBG_MSC_UFI             (GRP_DBG_MSC_BASE+8)

#ifdef  GRP_MSC_DEBUG_LOGGING

#define _TRACE_MSC_DRV_(n,x,y)      GRDBG_TRACE(GRP_DBG_MSC_DRV,n,x,y)
#define _TRACE_MSC_WRAP_(n,x,y)     GRDBG_TRACE(GRP_DBG_MSC_WRAP,n,x,y)
#define _TRACE_MSC_BOT_(n,x,y)      GRDBG_TRACE(GRP_DBG_MSC_BOT,n,x,y)
#define _TRACE_MSC_CBI_(n,x,y)      GRDBG_TRACE(GRP_DBG_MSC_CBI,n,x,y)
#define _TRACE_MSC_SCSI_(n,x,y)     GRDBG_TRACE(GRP_DBG_MSC_SCSI,n,x,y)
#define _TRACE_MSC_ATAPI_(n,x,y)    GRDBG_TRACE(GRP_DBG_MSC_ATAPI,n,x,y)
#define _TRACE_MSC_SFF8070i_(n,x,y) GRDBG_TRACE(GRP_DBG_MSC_SFF8070i,n,x,y)
#define _TRACE_MSC_UFI_(n,x,y)      GRDBG_TRACE(GRP_DBG_MSC_UFI,n,x,y)

#define _F_END                      END_FUNC

#else /* GRP_MSC_DEBUG_LOGGING */

#define _TRACE_MSC_DRV_(n,x,y)
#define _TRACE_MSC_WRAP_(n,x,y)
#define _TRACE_MSC_BOT_(n,x,y)
#define _TRACE_MSC_CBI_(n,x,y)
#define _TRACE_MSC_SCSI_(n,x,y)
#define _TRACE_MSC_ATAPI_(n,x,y)
#define _TRACE_MSC_SFF8070i_(n,x,y)
#define _TRACE_MSC_UFI_(n,x,y)

#define _F_END

#endif /* GRP_MSC_DEBUG_LOGGING */


#endif  /* _GRP_MSC_DEBUG_H_ */
