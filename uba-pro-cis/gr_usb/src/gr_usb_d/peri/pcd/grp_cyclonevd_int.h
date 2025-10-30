/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2015 Grape Systems, Inc.                        */
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
/*      grp_cyclonevd_int.h                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/DEVICE for CycloneV(OTG) - Interrupt process module header file                  */
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
#ifndef _GRP_CYCLONEVD_INT_H_
#define _GRP_CYCLONEVD_INT_H_

/**** INCLUDE FILES *****************************************************************************/
#include    "grusbtyp.h"


/**** EXTERNAL FUNCTION PROTOTYPES **************************************************************/
extern VOID grp_cyclonevd_Interrupt( VOID );


extern VOID grp_cyclonevd_Interrupt2( VOID );

#endif  /* _GRP_CYCLONEVD_INT_H_ */
