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
/*      grusbcom.h                                              1.10        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      GR-USB Products Common definition header file                       */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*  S.Tamaki    2003/09/11  Created initial version 1.00                    */
/*  Y.Hasegawa  2003/12/05  Version 1.00a                                   */
/*                          Modified for logging routine.                   */
/*  Y.Sato      2004/04/28  Version 1.00b                                   */
/*                          Add SICD Class.                                 */
/*  K.Handa     2006/01/31  Version 1.00c                                   */
/*                          Deleted debug module including.                 */
/*  K.Handa     2006/12/27  Version 1.10                                    */
/*                          Updated style of header.                        */
/*                                                                          */
/****************************************************************************/

#ifndef _GRUSB_DEFINE_
#define _GRUSB_DEFINE_

/* Deleted debug module including. 1.00c */
/* #define  GR_DBG_LOGGING */

/*********************/
/* Common definition */
/*********************/

/* Mode */
#define     GRUSB_COMMON_USE_USB20             /* Use USB2.0                       */
/*#define     GRUSB_COMMON_USE_OTG*/               /* Use OTG                          */

/* Memory allocation */
#define     GRUSB_COMMON_ENDIAN             0   /* 0 : Little endian  1 : Big endian */

/* Parametor */
#define     GRUSB_COMMON_DEFAULT_ADDRESS    0   /* default address                  */
#define     GRUSB_COMMON_DEFAULT_ENDPOINT   0   /* default endpoint                 */

#define     GRUSB_COMMON_100ms              100 /* system tic count 100ms           */
#define     GRUSB_COMMON_20ms               20  /* system tic count 20ms            */
#define     GRUSB_COMMON_10ms               10  /* system tic count 10ms            */

/**************************/
/* Host Driver definition */
/**************************/

/* Parametor */
#define     GRUSB_HOST_CONTROLLER_NUM       3       /* Controller number */
#define     GRUSB_HOST_MAX_DEVICE           10      /* Max Connect Device */

#define     GRUSB_HOST_CBTSK_PRI            3       /* Host Callback Task Priority */
#define     GRUSB_HOST_RHTSK_PRI            4       /* Host RootHub Task Priority */
#define     GRUSB_HOST_HBTSK_PRI            4       /* Host Hub Task Priority */
#define     GRUSB_HOST_CBTSK_STK            8092    /* Host Callback Task Stack Size */
#define     GRUSB_HOST_RHTSK_STK            4096    /* Host RootHub Task Stack Size */
#define     GRUSB_HOST_HBTSK_STK            4096    /* Host Hub Task Stack Size */

/* Transfer mode */
#define     GRUSB_HOST_USE_INTERRUPT                /* Use interrupt transfer */
#define     GRUSB_HOST_USE_ISOCHRONOUS             /* Use isochronous transfer */
/*#define    GRUSB_HOST_USE_CONTROL_NORMALPIPE*/        /* Use normal pipe control transfer */

/* Parameter Checking */
/*#define      GRUSB_HOST_NO_PARAM_CHECKING*/

/* Class Driver */
#define     GRUSB_HOST_USE_HUBDRIVER        /* Hub driver */
/*#define     GRUSB_HOST_USE_HIDDRIVER*/    /* hid class driver */
/*#define     GRUSB_HOST_USE_CDCDRIVER*/    /* Communication class driver */
/*#define     GRUSB_HOST_USE_PCDRIVER*/     /* Printer class driver */
/*#define     GRUSB_HOST_USE_AUDIORIVER*/   /* Audio device class driver */
#define     GRUSB_HOST_USE_SBC_BOT_DRIVER   /* SBC over BOT class driver */
#define     GRUSB_HOST_USE_SICDDRIVER       /* Still Image Capture Device driver */

/********************************/
/* Peripheral Driver definition */
/********************************/

/* Parameter */
#define GRUSB_DEV_CALLBK_MAX        10      /* Max Callback functions */
#define GRUSB_DEV_BUFF_MAX          1296    /* Temp buffer max size */
#define GRUSB_DEV_EP0_FIFO_SIZE     64      /* Buffer memory size of Endpoint 0 */
#define GRUSB_DEV_ENDPOINT_MAX      7       /* Endpoint number */

/*************************/
/* OTG Driver definition */
/*************************/
#define     GROTG_CB_USE        GRUSB_TRUE
#define     GROTG_CB_NON_USE    GRUSB_FALSE

/* Please set up GROTG_CB_USE, if a callback is required. */
/*--- A setup of the callback by state change ---*/
#define     GROTG_CB_A_IDLE             GROTG_CB_USE
#define     GROTG_CB_A_WAITVRISE        GROTG_CB_USE
#define     GROTG_CB_A_WAITBCON         GROTG_CB_USE
#define     GROTG_CB_A_HOST             GROTG_CB_USE
#define     GROTG_CB_A_SUSPEND          GROTG_CB_USE
#define     GROTG_CB_A_PERIPHERAL       GROTG_CB_USE
#define     GROTG_CB_A_WAITVFALL        GROTG_CB_USE
#define     GROTG_CB_A_VBUSERR          GROTG_CB_USE
#define     GROTG_CB_B_IDLE             GROTG_CB_USE
#define     GROTG_CB_B_SRPINIT          GROTG_CB_USE
#define     GROTG_CB_B_PERIPHERAL       GROTG_CB_USE
#define     GROTG_CB_B_WAITACON         GROTG_CB_USE
#define     GROTG_CB_B_HOST             GROTG_CB_USE
/*--- A setup of the callback by hardwear error ---*/
#define     GROTG_CB_HW_ERR             GROTG_CB_NON_USE
/*--- A setup of the callback by timeout occure ---*/
#define     GROTG_CB_A_WAIT_VRISE       GROTG_CB_USE
#define     GROTG_CB_A_WAIT_BCON        GROTG_CB_USE
#define     GROTG_CB_A_AIDI_BDIS        GROTG_CB_USE
#define     GROTG_CB_B_ASE0_BRST        GROTG_CB_USE
#define     GROTG_CB_B_SRP_FAIL         GROTG_CB_USE

#ifdef  GR_DBG_LOGGING
#include "dbg_mdl.h"     /* ログ機能ヘッダファイル */
#endif  /* GR_DBG_LOGGING */

#endif

