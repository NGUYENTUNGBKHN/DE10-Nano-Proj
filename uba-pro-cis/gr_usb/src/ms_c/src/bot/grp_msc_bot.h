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
/*      grp_msc_bot.h                                                           1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver  Bulk Only Protocol.                             */
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

#ifndef _GRP_MSC_BOT_H_
#define _GRP_MSC_BOT_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_msc_local.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
/* Csw Status */
#define GRP_CSW_COMMAND_PASSED          0x00                        /* Command success          */
#define GRP_CSW_COMMAND_FAILED          0x01                        /* Command failure          */
#define GRP_CSW_PHASE_ERROR             0x02                        /* Phase error              */

/* Bot Command */
#define GRP_BOT_RESET                   0xff                /* Reset of MassStorage             */
#define GRP_BOT_GETLUN                  0xfe                /* Maximum LUN number is acquired   */


/* Function Prototypes */
EXTERN grp_s32 grp_msc_BotInit( grp_msc_seq *, void * );
#ifdef  _GRP_INTERNAL_TEST_
EXTERN grp_s32 grp_msc_BotSetTag( grp_u32 );
#endif  /* _GRP_INTERNAL_TEST_ */

#endif /* _GRP_MSC_BOT_H_ */
