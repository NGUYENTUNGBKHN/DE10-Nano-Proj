#ifndef	_GRP_TYPES_H_
#define	_GRP_TYPES_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_types.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Basic type definitions for grp_xxx series							*/
/* FUNCTIONS:																*/
/*		None																*/
/* DEPENDENCIES:															*/
/*		None																*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/02/26	Created inital version 1.0				*/
/*		T.Imashiki		2007/02/20	Added grp_size_t						*/
/*						2007/02/20	Fixed comment for grp_int16_t			*/
/*		K.Kaneko		2016/03/17	Added type grp_uioffset_t				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2016 Grape Systems, Inc.,  All Rights Reserved.        */
/*                                                                          */
/* This software is furnished under a license, and may be used only in      */
/* accordance with the terms of such license and with the inclusion of the  */
/* above copyright notice.  No title to and the ownership of the software   */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* non-infringement of any intellectual property rights and the performance */
/* of this computer program, and specifically disclaims any responsibility  */
/* for any damages, special or consequential, connected  with the use of    */
/* this program.                                                            */
/****************************************************************************/

typedef	unsigned int	grp_uint32_t;	/* 32 bit unsigned integer */
typedef	unsigned short	grp_uint16_t;	/* 16 bit unsigned integer */
typedef	unsigned char	grp_uint8_t;	/* 8 bit unsigned integer */
typedef	int				grp_int32_t;	/* 32 bit signed integer */
typedef	short			grp_int16_t;	/* 16 bit signed integer */
typedef	char			grp_int8_t;		/* 8 bit signed integer */
typedef unsigned int	grp_uint_t;		/* unsigned int */
typedef unsigned short	grp_ushort_t;	/* unsigned short */
typedef unsigned char	grp_uchar_t;	/* unsigned char */

typedef grp_uint32_t	grp_uisize_t;	/* unsigned size/length */
typedef grp_int32_t		grp_isize_t;	/* signed size/length */
typedef grp_uint32_t	grp_uioffset_t;	/* unsigned offset */
typedef grp_int32_t		grp_ioffset_t;	/* offset */
typedef grp_uisize_t	grp_size_t;		/* size/length information */

#ifndef	NULL
#define	NULL			((void *)0)		/* NULL pointer */
#endif

#endif	/* _GRP_TYPES_H_ */
