/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2015 Grape Systems, Inc.                          */
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
/*      grp_fsif_config.h                                                       1.03            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      File system interface modules configuration file                                        */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2006/12/28  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Name change and addition of definition concerning retrying.     */
/*                              - GRP_FSIF_RETRY_CNT -> GRP_FSIF_CLS_RETRY_CNT                  */
/*                              - GRP_FSIF_MC_RETRY_CNT                                         */
/*   K.Kaneko       2015/12/21  V1.03                                                           */
/*                            - Changed the following default values.                           */
/*                              - GRP_FSIF_MAX_UINT                                             */
/*                              - GRP_FSIF_MAX_NTC_QUEUE                                        */
/*                              - GRP_FSIF_MAX_UINT                                             */
/*                              - GRP_FSIF_ATA_BOT                                              */
/*                              - GRP_FSIF_SFF_BOT                                              */
/*                              - GRP_FSIF_UFI_BOT                                              */
/*                              - GRP_FSIF_ATA_CBI                                              */
/*                              - GRP_FSIF_SFF_CBI                                              */
/*                              - GRP_FSIF_UFI_CBI                                              */
/*                              - GRP_FSIF_SFF_CB                                               */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_FSIF_CONFIG_H_
#define _GRP_FSIF_CONFIG_H_

/**** INTERNAL DATA DEFINES *********************************************************************/
/* Internal Use Constant */
/* The maximum number of management units */
#define GRP_FSIF_MAX_UINT               1                       /* don't set 0                  */

/* The maximum number accessed at the same time */
#define GRP_FSIF_MAX_ACCESS             1                       /* don't set 0                  */

/* The maximum queue number for the Nortice List */
#define GRP_FSIF_MAX_NTC_QUEUE          1                       /* don't set 0                  */

/* Teh maximum data length */
#define GRP_FSIF_MAX_DATA_LEN           (64*1024)               /* 64Kbyte                      */


/* Retry Count of close sequence */
#define GRP_FSIF_CLS_RETRY_CNT          3

/* Retry Count of media check sequence */
#define GRP_FSIF_MC_RETRY_CNT           3

/* wait time */
#define GRP_FSIF_WAIT_TIME              ((grp_u32)10000)                            /* 10sec    */

/* wait time for MSC close */
#define GRP_FSIF_CLOSE_WAIT             ((grp_u32)10)                               /* 10msec   */

/* Wait time to request TestUnitReady command */
#define GRP_FSIF_TEST_UNIT_WAIT_TIME    ((grp_u32)2000)                             /* 2sec     */

/* Stacke size for the task */
#define GRP_FSIF_MAIN_TASK_STK          ((grp_u32)2048)

/* Set GRP_USB_TURE if pairings will be used */
#define GRP_FSIF_SCS_BOT                GRP_USB_TRUE    /* or GRP_USB_FALSE */
#define GRP_FSIF_ATA_BOT                GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_SFF_BOT                GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_UFI_BOT                GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_SCS_CBI                GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_ATA_CBI                GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_SFF_CBI                GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_UFI_CBI                GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_SCS_CB                 GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_ATA_CB                 GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_SFF_CB                 GRP_USB_FALSE   /* or GRP_USB_TRUE */
#define GRP_FSIF_UFI_CB                 GRP_USB_FALSE   /* or GRP_USB_TRUE */

/*  DLOCAL  grp_u8  l_aucRegistClass[GRP_FSIF_MAX_CLASS][GRP_FSIF_MAX_SUBCLASS] = {             */
/*  /+          SCSI                ATAPI               SFF8070i            UFI         +/      */
/*  /+ BOT +/  {GRP_FSIF_SCS_BOT,   GRP_FSIF_ATA_BOT,   GRP_FSIF_SFF_BOT,   GRP_FSIF_UFI_BOT    */
/*  /+ CBI +/  {GRP_FSIF_SCS_CBI,   GRP_FSIF_ATA_CBI,   GRP_FSIF_SFF_CBI,   GRP_FSIF_UFI_CBI},  */
/*  /+ CB  +/  {GRP_FSIF_SCS_CB,    GRP_FSIF_ATA_CB,    GRP_FSIF_SFF_CB,    GRP_FSIF_UFI_CB}};  */

#endif  /* _GRP_FSIF_CONFIG_H_ */
