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
/*      grp_cyclonevh_config.h                                                  1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# CYCLONEV Driver configuration file                                         */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kaneko       2019/04/26  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                              (based on GR-USB/HOST# for STM32Fx(OTG_HS) V1.01)               */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CYCLONEVH_CONFIG_H_
#define _GRP_CYCLONEVH_CONFIG_H_

/*** INCLUDE FILES ******************************************************************************/
#include "grp_cyclonevh.h"

/*** INTERNAL DATA DEFINES **********************************************************************/
/* maximum pipes to use by host controller */
#define GRP_CYCLONEVH_MAX_PIPE_NUM          (16)

/* default pipe information */
#define GRP_CYCLONEVH_PIPE0_TYPE            GRP_USBD_CONTROL    /* NOT CHANGED!! */
#define GRP_CYCLONEVH_PIPE0_DIR             0                   /* NOT CHANGED!! */
#define GRP_CYCLONEVH_PIPE1_TYPE            GRP_USBD_CONTROL    /* NOT CHANGED!! */
#define GRP_CYCLONEVH_PIPE1_DIR             GRP_USBD_TX_IN      /* NOT CHANGED!! */
#define GRP_CYCLONEVH_PIPE2_TYPE            GRP_USBD_CONTROL    /* NOT CHANGED!! */
#define GRP_CYCLONEVH_PIPE2_DIR             GRP_USBD_TX_OUT     /* NOT CHANGED!! */
#define GRP_CYCLONEVH_PIPE3_TYPE            GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE3_DIR             GRP_USBD_TX_IN
#define GRP_CYCLONEVH_PIPE4_TYPE            GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE4_DIR             GRP_USBD_TX_OUT
#define GRP_CYCLONEVH_PIPE5_TYPE            GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE5_DIR             GRP_USBD_TX_IN
#define GRP_CYCLONEVH_PIPE6_TYPE            GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE6_DIR             GRP_USBD_TX_OUT
#define GRP_CYCLONEVH_PIPE7_TYPE            GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE7_DIR             GRP_USBD_TX_IN
#define GRP_CYCLONEVH_PIPE8_TYPE            GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE8_DIR             GRP_USBD_TX_OUT
#define GRP_CYCLONEVH_PIPE9_TYPE            GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE9_DIR             GRP_USBD_TX_IN
#define GRP_CYCLONEVH_PIPE10_TYPE           GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE10_DIR            GRP_USBD_TX_OUT
#define GRP_CYCLONEVH_PIPE11_TYPE           GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE11_DIR            GRP_USBD_TX_IN
#define GRP_CYCLONEVH_PIPE12_TYPE           GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE12_DIR            GRP_USBD_TX_OUT
#define GRP_CYCLONEVH_PIPE13_TYPE           GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE13_DIR            GRP_USBD_TX_IN
#define GRP_CYCLONEVH_PIPE14_TYPE           GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE14_DIR            GRP_USBD_TX_OUT
#define GRP_CYCLONEVH_PIPE15_TYPE           GRP_USBD_BULK
#define GRP_CYCLONEVH_PIPE15_DIR            GRP_USBD_TX_IN

/* maximum management informations */
#define GRP_CYCLONEVH_MAX_INFO_NUM          (16)

/* maximum interrupt pipes to use by host controller */
#define GRP_CYCLONEVH_MAX_INTERRUPT_PIPE    (2)

/* maximum number to request at same time */
#define GRP_CYCLONEVH_MAX_TR_NUM            (20)

/* Polling time of RHUB task */
#define GRP_CYCLONEVH_RH_CHECK_TIME         (200)

/* Operate speed mode */
#define GRP_CYCLONEVH_SPEED_MODE            (1)                     /* 0 : Full Speed           */
                                                                    /* 1 : High Speed           */

/* Channel validation check upper limit */
#if (GRP_CYCLONEVH_SPEED_MODE == 0)
#define GRP_CYCLONEVH_MAX_CHK_CH_ENA        (2000)
#elif (GRP_CYCLONEVH_SPEED_MODE == 1)
#define GRP_CYCLONEVH_MAX_CHK_CH_ENA        (200)
#else
#error "There is an error in the setting of GRP_CYCLONEVH_SPEED_MODE."
#endif

/* Task parametor */
#define GRP_CYCLONEVH_CBTSK_PRI             GRP_VOS_PRI_HIGHEST /* Callback Task Priority       */
#define GRP_CYCLONEVH_RHTSK_PRI             GRP_VOS_PRI_HIGH    /* RootHub Task Priority        */

/* Task stack size */
#define GRP_CYCLONEVH_CBTSK_STK             1024                /* Host Callback Task Stack Size*/
#define GRP_CYCLONEVH_RHTSK_STK             1024                /* Host RootHub Task Stack Size */

/* definitions of FIFO buffer size */
#define GRP_CYCLONEVH_GRXFSIZ               4096
#define GRP_CYCLONEVH_GNPTXFSIZ             2048
#define GRP_CYCLONEVH_HPTXFSIZ              2048

#endif /* _GRP_CYCLONEVH_CONFIG_H_ */

