/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2015 Grape Systems, Inc.                        */
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
/*      grp_cyclonev_types.h                                                    1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV type definition conversion file           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Kobayashi    2014/12/24  V0.90                                                           */
/*                            - Created beta release version.                                   */
/*   T.Kobayashi    2015/01/21  V1.00                                                           */
/*                            - 1st release version.                                            */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CYCLONEV_TYPES_H_
#define _GRP_CYCLONEV_TYPES_H_

/**** INCLUDE FILES *****************************************************************************/
/* Use a type definition file of GR-USB/DEVICE */
#include "grusbtyp.h"

/* Convert a type definition of GR-USB/HOST# in the common module */
/* into a type definition of GR-USB/DEVICE                        */
#define grp_ui          UINT
#define grp_si          INT

#define grp_u32         UINT32
#define grp_s32         INT32

#define grp_u16         UINT16
#define grp_s16         INT16

#define grp_u8          UINT8
#define grp_s8          INT8

#endif /* _GRP_CYCLONEV_TYPES_H_ */
