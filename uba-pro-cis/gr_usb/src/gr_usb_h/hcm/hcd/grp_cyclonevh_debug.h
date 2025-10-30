/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2019 Grape Systems, Inc.                             */
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
/*      grp_cyclonevh_debug.h                                                   1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# for CYCLONEV DBG_LOG module header file                                    */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kaneko       2019/04/26  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                              (based on GR-USB/HOST# for STM32Fx(OTG_HS) V1.01)               */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CYCLONEVH_DEBUG_H_
#define _GRP_CYCLONEVH_DEBUG_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_ctr.h"

/* module code */
#define MC_HCDI                         GRP_HCD_MDL_ID      /* defined in the grp_ctr.h file    */
#define MC_RHB                          GRP_RHUB_MDL_ID     /* defined in the grp_ctr.h file    */
#define MC_CTRL                         GRP_CTRL_MDL_ID     /* defined in the grp_ctr.h file    */
#define MC_MGR                          GRP_MGR_MDL_ID      /* defined in the grp_ctr.h file    */

/**** LOGGING ROUTINE DEFINITIONS ***************************************************************/
/*----------------------------------------------------------------------------------------------*/
#define GRP_DBG_CYCLONEVH_LOGGING
#ifdef  GRP_DBG_CYCLONEVH_LOGGING
#include    "grp_usbc_dbg.h"

    /* trace function */
    #define _TRACE_HCDI_(n,x,y)             GRDBG_TRACE(MC_HCDI, n,x,y)
    #define _TRACE_RHB_( n,x,y)             GRDBG_TRACE(MC_RHB,  n,x,y)
    #define _TRACE_CTRL_(n,x,y)             GRDBG_TRACE(MC_CTRL, n,x,y)
    #define _TRACE_MGR_(n,x,y)              GRDBG_TRACE(MC_MGR,  n,x,y)

    /* function end code */
    #define _F_END_                         END_FUNC

#else   /* GRP_DBG_CYCLONEVH_LOGGING */

    /* trace function */
    #define _TRACE_HCDI_(n,x,y)
    #define _TRACE_RHB_( n,x,y)
    #define _TRACE_CTRL_(n,x,y)
    #define _TRACE_MGR_( n,x,y)

    /* function end code */
    #define _F_END_

#endif  /* GRP_DBG_CYCLONEVH_LOGGING */


/**** ANOMALY ROUTINE DEFINITIONS ****************************************************************/
#define GRP_ANOMALY_CYCLONEVH
/*-----------------------------------------------------------------------------------------------*/
#ifdef  GRP_ANOMALY_CYCLONEVH
#include    "grp_usbc_anmly.h"

    /*--- HCD -----------------------------------------------------------------------------------*/
    #define _FATAL_HCD_(y,z)            GRP_ANOM_FATAL_(MC_HCDI,y,z)
    #define _ERROR_HCD_(y,z)            GRP_ANOM_ERROR_(MC_HCDI,y,z)
    #define _WARN_HCD_( y,z)            GRP_ANOM_WARN_( MC_HCDI,y,z)
    #define _INFO_HCD_( y,z)            GRP_ANOM_INFO_( MC_HCDI,y,z)

    /*--- RHUB ----------------------------------------------------------------------------------*/
    #define _FATAL_RHB_(y,z)            GRP_ANOM_FATAL_(MC_RHB,y,z)
    #define _ERROR_RHB_(y,z)            GRP_ANOM_ERROR_(MC_RHB,y,z)
    #define _WARN_RHB_( y,z)            GRP_ANOM_WARN_( MC_RHB,y,z)
    #define _INFO_RHB_( y,z)            GRP_ANOM_INFO_( MC_RHB,y,z)

#else   /* GRP_ANOMALY_CYCLONEVH */

    /*--- HCD -----------------------------------------------------------------------------------*/
    #define _FATAL_HCD_(y,z)
    #define _ERROR_HCD_(y,z)
    #define _WARN_HCD_( y,z)
    #define _INFO_HCD_( y,z)

    /*--- RHUB ----------------------------------------------------------------------------------*/
    #define _FATAL_RHB_(y,z)
    #define _ERROR_RHB_(y,z)
    #define _WARN_RHB_( y,z)
    #define _INFO_RHB_( y,z)

#endif  /* GRP_ANOMALY_CYCLONEVH */

#endif  /* _GRP_CYCLONEVH_DEBUG_H_ */
