/************************************************************************************************/
/*                                                                                              */
/*                           Copyright(C) 2007-2015 Grape Systems, Inc.                         */
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
/*      grp_vos_local.h                                                         2.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Virtual OS interface for ThreadX.                                                       */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*   M.Kitahara     2015/01/31  2.01                                                            */
/*                              Changed from vpGRVOS_FirstUnusedMemory to                       */
/*                              g_vpGrpVosFirstUnusedMemory.                                    */
/*                              Changed from extern to EXTERN.                                  */
/*                              Added preprocessor for GRP_VOS_MAX_xxx.                         */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_VOS_LOCAL_H_
#define _GRP_VOS_LOCAL_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos_cfg.h"

/**** VOS PRIVATE CONSTANT DEFINITIONS **********************************************************/


/**** VOS PRIVATE STRUCTURE TYPES ***************************************************************/


/**** VOS PRIVATE DATA DEFINITIONS **************************************************************/

#if (GRP_VOS_MAX_TSK)
    EXTERN grp_vos_t_task              _tTaskTable[GRP_VOS_MAX_TSK];
#endif
#if (GRP_VOS_MAX_QUE)
    EXTERN grp_vos_t_queue             _tQueueTable[GRP_VOS_MAX_QUE];
#endif
#if (GRP_VOS_MAX_SEM)
    EXTERN grp_vos_t_semaphore         _tSemaphoreTable[GRP_VOS_MAX_SEM];
#endif
#if (GRP_VOS_MAX_PPL)
    EXTERN grp_vos_t_partition_pool    _tPartitionPoolTable[GRP_VOS_MAX_PPL];
#endif
#if (GRP_VOS_MAX_MPL)
    EXTERN grp_vos_t_memory_pool       _tMemoryPoolTable[GRP_VOS_MAX_MPL];
#endif
#if (GRP_VOS_MAX_FLG)
    EXTERN grp_vos_t_flag              _tFlagTable[GRP_VOS_MAX_FLG];
#endif

EXTERN  void    *g_vpGrpVosFirstUnusedMemory;

/**** VOS PRIVATE FUNCTION PROTOTYPES ***********************************************************/

grp_u32         _grp_vos_ConvertTimeUnit(grp_u32 ulMilliSecondTime);

#endif  /* _GRP_VOS_LOCAL_H_ */

