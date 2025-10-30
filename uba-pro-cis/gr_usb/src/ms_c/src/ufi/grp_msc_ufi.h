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
/*      GR-USB/HOST# Mass Storage Class Driver Ufi Sub Class.                                   */
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
#ifndef _GRP_MSC_UFI_H_
#define _GRP_MSC_UFI_H_


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
EXTERN grp_s32 grp_msc_UfiInit( void );
EXTERN grp_s32 _msc_UfiReadSector( grp_msc_cmd *ptCmd, grp_u32 ulLba, grp_u32 ulSize, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_UfiWriteSector( grp_msc_cmd *ptCmd, grp_u32 ulLba, grp_u32 ulSize, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_UfiInquiry( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_UfiReadFormatCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_UfiReadCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_UfiModeSense( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_UfiTestUnitReady( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );
EXTERN grp_s32 _msc_UfiRequestSense( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent );


#endif /* _GRP_MSC_UFI_H_ */
