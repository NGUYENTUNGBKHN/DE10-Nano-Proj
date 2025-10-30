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
/*      grp_fsif_debug.h                                                        1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      File system interface debug module                                                      */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2006/12/28  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_FSIF_DEBUG_H_
#define _GRP_FSIF_DEBUG_H_


/**** INCLUDE FILES *****************************************************************************/
#ifdef  GRP_FSIF_DEBUG_LOGGING
#include    "grp_usbc_dbg.h"

/**** DEBUG LOGGING OPTION **********************************************************************/
/*--- Use Logging routine ---*/
    #define GRP_DBG_FSIF                (GRP_DBG_FSIF_BASE)
    #define _TRACE_FSIF_(n,x,y)         GRDBG_TRACE( (grp_u8)GRP_DBG_FSIF, (grp_u8)n,(grp_u8)x,(grp_u8)y)

    #define _F_END                      END_FUNC

#else /* GRP_FSIF_DEBUG_LOGGING */
/*--- Not use Logging routine ---*/
    #define _TRACE_FSIF_(n,x,y)

    #define _F_END

#endif /* GRP_FSIF_DEBUG_LOGGING */

#endif  /* _GRP_FSIF_DEBUG_H_ */
