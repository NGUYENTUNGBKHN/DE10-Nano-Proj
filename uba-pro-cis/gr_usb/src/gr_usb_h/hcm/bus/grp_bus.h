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
/*      grp_bus.h                                                               1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      These functions supports BUS I/O interface.                                             */
/*      !!! In GR-USB/HOST# for CycloneV, this module is not necessary. !!!                     */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kaneko       2019/04/26  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                              (based on GR-USB/HOST# for STM32FxHS V1.00)                     */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_BUS_H_
#define _GRP_BUS_H_

/**** INCLUDE FILES *****************************************************************************/

/**** INTERNAL DATA DEFINES *********************************************************************/

/************************************************************************************************/
/* Ctrl error definition                                                                        */
#define GRP_BUS_OK                      0


/**** FUNCTION PROTOTYPES ***********************************************************************/
EXTERN grp_s32 grp_bus_Init(void);

#endif /* _GRP_BUS_H_ */

