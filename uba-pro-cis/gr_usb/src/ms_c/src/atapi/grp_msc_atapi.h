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
/*      grp_msc_atapi.h                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Atapi Sub Class.                                 */
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
#ifndef _GRP_MSC_ATAPI_H_
#define _GRP_MSC_ATAPI_H_


/**** FUNCTION PROTOTYPES ***********************************************************************/
EXTERN grp_s32 grp_msc_AtapiInit( void );
EXTERN grp_s32 _msc_AtapiReadSector( grp_msc_cmd *ptCmd, grp_u32 ulLba, grp_u32 ulSize, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_AtapiWriteSector( grp_msc_cmd *ptCmd, grp_u32 ulLba, grp_u32 ulSize, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_AtapiInquiry( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_AtapiReadFormatCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_AtapiReadCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_AtapiModeSense( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_AtapiTestUnitReady( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_AtapiRequestSense( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );


#endif /* _GRP_MSC_ATAPI_H_ */
