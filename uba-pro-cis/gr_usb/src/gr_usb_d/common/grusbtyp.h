/****************************************************************************/
/*                                                                          */
/*              Copyright(C) 2002-2006 Grape Systems, Inc.                  */
/*                        All Rights Reserved                               */
/*                                                                          */
/* This software is furnished under a license and may be used and copied    */
/* only in accordance with the terms of such license and with the inclusion */
/* of the above copyright notice. No title to and ownership of the software */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* the performance of this computer program, and specifically disclaims any */
/* responsibility for any damages, special or consequential, connected with */
/* the use of this program.                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILE NAME                                                    VERSION     */
/*                                                                          */
/*      grusbtyp.h                                              1.10        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      Common data type definition for GR-USB Products                     */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*  S.Tamaki    2003/09/11  Created initial version 1.00                    */
/*  K.Handa     2006/12/27  Version 1.10                                    */
/*                          Updated style of header.                        */
/*                                                                          */
/****************************************************************************/

#ifndef __GRUSB_TYPES_H__
#define __GRUSB_TYPES_H__

#include "gr_types.h"

#if 1
#define GRCOMD_DEBUG
#define GRUSB_PERI_DEBUG
#define GRUSB_CTL_DEBUG
#define GRUSB_CYC_DEBUG
#define GRUSB_PRM_DEBUG
#define GRUSB_STS_DEBUG
#define GRUSB_TRS_DEBUG

#endif

#undef  INT
#undef  UINT
#undef  VOID

#undef  LOCAL
#undef  DLOCAL
#undef  EXTERN

#undef  STATUS
#undef  OPTION
#undef  BOOLEAN
#undef  ADDR

#undef  GRUSB_TRUE
#undef  GRUSB_FALSE
#undef  GRUSB_NULL

#define INT         int
#define UINT        unsigned int
#define VOID        void

#define LOCAL       static
#define DLOCAL      static
#define EXTERN      extern

#define BOOLEAN     int
#define STATUS      int
#define OPTION      int
#define ADDR        unsigned int

#define GRUSB_TRUE        1
#define GRUSB_FALSE       0
#define GRUSB_NULL        0

#include "grusbcom.h"


#endif
