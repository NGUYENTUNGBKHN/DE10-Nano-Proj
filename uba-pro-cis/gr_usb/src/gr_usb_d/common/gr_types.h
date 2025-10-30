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
/*      gr_types.h                                              1.10        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      Common data type definition for GRAPEWARE Products                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*  Y.Sato      2002/12/12  Created initial version 1.00                    */
/*  Y.Sato      2003/03/19  It changes a type definition into #define       */
/*                          from typedef.                                   */
/*  K.Handa     2006/12/27  Version 1.10                                    */
/*                          Updated style of header.                        */
/*                                                                          */
/****************************************************************************/

#ifndef __GR_TYPES_H__
#define __GR_TYPES_H__


/**** DATA TYPE & CONSTANT DEFINES ***************************************/

/* integral types declarations */
#undef      INT8
#undef      INT16
#undef      INT32
#undef      UINT8
#undef      UINT16
#undef      UINT32

#define     INT8        signed char
#define     INT16       signed short
#define     INT32       signed long
#define     UINT8       unsigned char
#define     UINT16      unsigned short
#define     UINT32      unsigned long

#endif  /* __GR_TYPES_H__ */

/**** END OF FILE gr_types.h *********************************************/
