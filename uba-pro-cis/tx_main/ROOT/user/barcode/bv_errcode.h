/******************************************************************************/
/*! @addtogroup Group1
    @file       BV_errorcode.h
    @brief      Error code define
    @date       2018/03/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*******************************************************************************
    @par        History
    - 2018/03/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/
#pragma once
/*! @ingroup BV_errorcode.h BV_errorcode.h */
/* @{ */


/*--- ABNOMAL SIGNAL ON (WAIT RESET) ---*/
#define EC_ROM1 	0xb0			/* BOOT ROM-CHECK ERR */
#define EC_ROM2 	0xb1			/* EXT. ROM-CHECK ERR */
#define EC_RAM		0xb2			/* RAM-CHECK ERR */
#define EC_ROM2_W	0xb3			/* EXT. ROM-CHECK WRITE ERR */

#define EC_TMR		0xb4			/* TIMER ERR */
#define EC_EROM		0xb5			/* EEPROM ERROR */
#define EC_DW_F		0xb6			/* Down load file error */


/* SYSTEM ERROR_1	 (WAIT RESET) ---*/
#define EC_SKF		0xa1			/* STACKER FULL(with FULL SIGNAL) */
#define EC_SKJ		0xa2			/* STACKER JAM */
#define EC_OPN		0xa3
#define EC_SMT		0xa4			/* STACKER LOCK */
#define EC_MSE		0xa5			/* MOTOR SPEEED ERR */
#define EC_MTR		0xa6			/* MOTOR STOP ERR */
#define EC_WRG		0xa7			/* SETUP WRONG */
/* Recycle processing for ADP >>>>>>>>>>>>>>>>>>>>>>>>>>*/
#define	EC_DISPENSER_JAM_IN_STACKER (UB)0xa8	/* Dispense jam in stacker */
#define	EC_DISPENSER_NO_NOTE 		(UB)0xa9	/* Bill not come to Head exit sensor */
#define	EC_DISPENSED_BILL_REMOVE 	(UB)0xaa	/* Dispense Bill remove */
/*<<<<<<<<<<<<<<<<<<<<<<<<<< Recycle processing for ADP */

#define EC_S_BOX	0xab			/* スマートカセット通信異常 */
#define EC_S_CHK	0xac			/* スマートカセットチェックサム異常 */
#define EC_S_MIS	0xad			/* 別のゲーム機にセットされていたＢＯＸです */
#define EC_S_RED	0xae			/* 集計データセーブ済みのＢＯＸです */


/*--- SYSTEM ERROR_2	 (AUTO RECOVERY) ---*/
#define EC_SKH		0x92			/* STACKER HOME JAM */
#define EC_SLJ		0x93			/* stacker lever SENSOR JAMMING ERR */
#define EC_JAM		0x94			/* PAPER JAMMING ERR */

#define EC_VAL		0x95			/* バリユニットが無い */

#define EC_SOL		0x98			/* SOL ERR */
#define EC_PBJ		0x99			/* PB-UNIT JAM ERR */
#define EC_CAS		0x9a			/* CASSETTE REMOVE ERR */
#define EC_SKCM		0x9b			/* Stacker bord communication error ( for EBA-30 ) */
#define EC_BSP		0x9C			/* BILL PULL BACK ERR */
#define EC_ROLUP 	0x9d			/* Roller up error ( for EBA-30 ) */
#define EC_WBS		0x9e			/* Width brings system error ( for EBA-30 ) */
#define EC_KEY 		0x9f			/* Box key open ( for EBA-30 ) */

#define EC_I2C 		0xc0			/* I2C communication error */
#define EC_MAG 		0xc1			/* MAG init error */

#define EC_SKL 		0xE0			/* Stacker unit loss  ( for EBA-30 ) */

#define EC_UNKNOWN	0xFE			/* Unknown error */

/* Recycle processing for ADP >>>>>>>>>>>>>>>>>>>>>>>>>>*/
#define	EC_DISPENSER_JAM_IN_ACCEPTOR (UB)0x90	/* Dispense jam in acceptor */
/*<<<<<<<<<<<<<<<<<<<<<<<<<< Recycle processing for ADP */


/*--- REJECT CODE 						 ---*/
#define REJ_SLA		0x01			/* SLANT */
#define REJ_MAG		0x02			/* MAG ERR */
#define REJ_PAP		0x03			/* STAY PAPER */
#define REJ_XRT		0x04			/* X-RATE ERROR */
#define REJ_SYN		0x05			/* SYNC(CSC SENS) DETECT ERR */
#define REJ_PTO		0x06			/* NEAR ERR */
#define REJ_PIR		0x07			/* pattern error */
#define REJ_PHV		0x08			/* PHOTO LEVEL ERR */
#define REJ_INH		0x09			/* INHIBIT */
#define REJ_REJ		0x0A			/* REJECT SIGNAL */

#define REJ_LVR		0x0B			/* LEVER ON ERR */
#define REJ_BSW		0x0C			/* BACK SENSOR "ON" ERR */

#define REJ_LGO		0x0D			/* PAPER LENGTH OUT RENGE */
#define REJ_2CL		0x0E			/* IR-RED ERROR */

#define REJ_CDW		0x0E			/* COMMUNICATION DOWN */
#define REJ_MAG2	0x0F			/* MAG ERR left or rigth*/
#define REJ_FAKE	0x0F			/* 偽造券対策	*/
#define REJ_UV		0x10			/* UV level error */
#define REJ_SS		0x11			/* SS pattern error */
#define REJ_INH_M 	0x19			/* Master Inhibit for ID-0E */



/*--- REJECT CODE (BAR CODE)			 ---*/
#define BAR_TRUE				0x000F		/*	need configuration							*/
#define BAR_NC					0x110F		/*	(i-1) need configuration							*/
#define BAR_UN					0x120F		/*	(i-2) unknown code 								*/
#define BAR_SH					0x130F		/*	short read or 指定ｷｬﾗｸﾀ数ｵｰﾊﾞｰ				*/
#define BAR_ST					0x140F		/*	(i-3) start bit missing 							*/
#define BAR_SP					0x150F		/*	(i-4) stop  bit missing 							*/
#define BAR_TP					0x160F		/*	type  not enable 							*/
#define BAR_LG					0x1D0F		/*	LENGTH OUT OF RANGE 						*/
#define BAR_NG					0x1E0F		/*	無効な設定バークーポン 						*/
#define BAR_LIMIT_WIDE_NARROW   0x1C0F		/*	'14-06-11	*/
#define BAR_LIMIT_LENGTH_DIFF   0x170F		/*	'14-06-11	*/
#define BAR_MC					0x030F		/*	マシンＮＯ．用バークーポン 					*/
#define BAR_PHV					0x180F		/*	(i-5) BAR PHOTO LEVEL ERR ('04-11-02 Ver1.15)		*/
#define BAR_DIR_MISS			0x1B0F		/*	(i-8) dirction miss								*/

/* EOF */
