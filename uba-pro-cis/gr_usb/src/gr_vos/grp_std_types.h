/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2007 Grape Systems, Inc.                            */
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
/*      grp_std_types.h                                                         2.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Common standard type definition for GrapeWare                                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_STD_TYPES_H_
#define _GRP_STD_TYPES_H_


/**** DATA TYPE & CONSTANT DEFINITIONS **********************************************************/

/* integral types declarations */

typedef unsigned int    grp_ui;
typedef signed int      grp_si;

typedef unsigned long   grp_u32;
typedef signed long     grp_s32;

typedef unsigned short  grp_u16;
typedef signed short    grp_s16;

typedef unsigned char   grp_u8;
typedef signed char     grp_s8;


/* place types define */

#undef  LOCAL
#undef  DLOCAL
#undef  EXTERN

#define LOCAL           static
#define DLOCAL          static
#define EXTERN          extern


#endif  /* _GRP_STD_TYPES_H_ */

