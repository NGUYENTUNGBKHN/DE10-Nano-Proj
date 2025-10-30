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
/*      grp_cyclonevd_cfg.h                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/DEVICE for CycloneV(OTG) - User configuration file                               */
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
#ifndef _GRP_CYCLONEVD_CFG_H_
#define _GRP_CYCLONEVD_CFG_H_

/**** EXTERNAL DATA DEFINES *********************************************************************/


/************************************************************************************************/
/* FIFO buffer size                                                                             */
/************************************************************************************************/
#define GRP_CYCLONEVD_RX_FIFO_SIZE          (512)                              /* RxFIFO size   */
#define GRP_CYCLONEVD_TX_FIFO_0_SIZE        (64)                               /* TxFIFO 0 size */
#define GRP_CYCLONEVD_TX_FIFO_1_SIZE        (256)                              /* TxFIFO 1 size */
#define GRP_CYCLONEVD_TX_FIFO_2_SIZE        (256)                              /* TxFIFO 2 size */
#define GRP_CYCLONEVD_TX_FIFO_3_SIZE        (32)                                /* TxFIFO 3 size */
#define GRP_CYCLONEVD_TX_FIFO_4_SIZE        (0)                                /* TxFIFO 4 size */
#define GRP_CYCLONEVD_TX_FIFO_5_SIZE        (256)                              /* TxFIFO 5 size */
#define GRP_CYCLONEVD_TX_FIFO_6_SIZE        (32)                               /* TxFIFO 6 size */
#define GRP_CYCLONEVD_TX_FIFO_7_SIZE        (0)                                /* TxFIFO 7 size */
#define GRP_CYCLONEVD_TX_FIFO_8_SIZE        (0)                                /* TxFIFO 8 size */
#define GRP_CYCLONEVD_TX_FIFO_9_SIZE        (0)                                /* TxFIFO 9 size */
#define GRP_CYCLONEVD_TX_FIFO_10_SIZE       (0)                                /* TxFIFO 10 size */
#define GRP_CYCLONEVD_TX_FIFO_11_SIZE       (0)                                /* TxFIFO 11 size */
#define GRP_CYCLONEVD_TX_FIFO_12_SIZE       (0)                                /* TxFIFO 12 size */
#define GRP_CYCLONEVD_TX_FIFO_13_SIZE       (0)                                /* TxFIFO 13 size */
#define GRP_CYCLONEVD_TX_FIFO_14_SIZE       (0)                                /* TxFIFO 14 size */
#define GRP_CYCLONEVD_TX_FIFO_15_SIZE       (0)                                /* TxFIFO 15 size */


#endif  /* _GRP_CYCLONEVD_CFG_H_ */
