/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file baule17_led_def.h"
 * @brief BAU-17 LED定義 ヘッダファイル
 * @date 2018.03.05 Created.
 */
/****************************************************************************/
#ifndef HAL_GPIO_REG_H
#define HAL_GPIO_REG_H

#include "js_gpio.h"		// GpioIDを定義
#include "common.h"			// モデル定義

/* GIPIO0 */
#define GPIO_00 GpioID(0, 0)
#define GPIO_01 GpioID(0, 1)
#define GPIO_02 GpioID(0, 2)
#define GPIO_03 GpioID(0, 3)
#define GPIO_04 GpioID(0, 4)
#define GPIO_05 GpioID(0, 5)
#define GPIO_06 GpioID(0, 6)
#define GPIO_07 GpioID(0, 7)
#define GPIO_08 GpioID(0, 8)
#define GPIO_09 GpioID(0, 9)
#define GPIO_10 GpioID(0, 10)
#define GPIO_11 GpioID(0, 11)
#define GPIO_12 GpioID(0, 12)
#define GPIO_13 GpioID(0, 13)
#define GPIO_14 GpioID(0, 14)
#define GPIO_15 GpioID(0, 15)
#define GPIO_16 GpioID(0, 16)
#define GPIO_17 GpioID(0, 17)
#define GPIO_18 GpioID(0, 18)
#define GPIO_19 GpioID(0, 19)
#define GPIO_20 GpioID(0, 20)
#define GPIO_21 GpioID(0, 21)
#define GPIO_22 GpioID(0, 22)
#define GPIO_23 GpioID(0, 23)
#define GPIO_24 GpioID(0, 24)
#define GPIO_25 GpioID(0, 25)
#define GPIO_26 GpioID(0, 26)
#define GPIO_27 GpioID(0, 27)
#define GPIO_28 GpioID(0, 28)

/* GIPIO1 */
#define GPIO_29 GpioID(1, 0)
#define GPIO_30 GpioID(1, 1)
#define GPIO_31 GpioID(1, 2)
#define GPIO_32 GpioID(1, 3)
#define GPIO_33 GpioID(1, 4)
#define GPIO_34 GpioID(1, 5)
#define GPIO_35 GpioID(1, 6)
#define GPIO_36 GpioID(1, 7)
#define GPIO_37 GpioID(1, 8)
#define GPIO_38 GpioID(1, 9)
#define GPIO_39 GpioID(1, 10)
#define GPIO_40 GpioID(1, 11)
#define GPIO_41 GpioID(1, 12)
#define GPIO_42 GpioID(1, 13)
#define GPIO_43 GpioID(1, 14)
#define GPIO_44 GpioID(1, 15)
#define GPIO_45 GpioID(1, 16)
#define GPIO_46 GpioID(1, 17)
#define GPIO_47 GpioID(1, 18)
#define GPIO_48 GpioID(1, 19)
#define GPIO_49 GpioID(1, 20)
#define GPIO_50 GpioID(1, 21)
#define GPIO_51 GpioID(1, 22)
#define GPIO_52 GpioID(1, 23)
#define GPIO_53 GpioID(1, 24)
#define GPIO_54 GpioID(1, 25)
#define GPIO_55 GpioID(1, 26)
#define GPIO_56 GpioID(1, 27)
#define GPIO_57 GpioID(1, 28)

/* GIPIO2 */
#define GPIO_58 GpioID(2, 0)
#define GPIO_59 GpioID(2, 1)
#define GPIO_60 GpioID(2, 2)
#define GPIO_61 GpioID(2, 3)
#define GPIO_62 GpioID(2, 4)
#define GPIO_63 GpioID(2, 5)
#define GPIO_64 GpioID(2, 6)
#define GPIO_65 GpioID(2, 7)
#define GPIO_66 GpioID(2, 8)
#define GPIO_67	GpioID(2, 9)
#define GPIO_68	GpioID(2, 10)
#define GPIO_69	GpioID(2, 11)
#define GPIO_70	GpioID(2, 12)

/* GIPI2 */
#define GPI_00	GpioID(2, 13)
#define GPI_01	GpioID(2, 14)
#define GPI_02	GpioID(2, 15)
#define GPI_03	GpioID(2, 16)
#define GPI_04	GpioID(2, 17)
#define GPI_05	GpioID(2, 18)
#define GPI_06	GpioID(2, 19)
#define GPI_07	GpioID(2, 20)
#define GPI_08	GpioID(2, 21)
#define GPI_09	GpioID(2, 22)
#define GPI_10	GpioID(2, 23)
#define GPI_11	GpioID(2, 24)
#define GPI_12	GpioID(2, 25)
#define GPI_13	GpioID(2, 26)


/****************************************************************/
/*						GPIO 信号									*/
/****************************************************************/
#define __HAL_FUSB_ONOFF	(GPIO_00)
#define __HAL_USB0_RESET	(GPIO_09)
#define __HAL_CENT_OPEN_PS	(GPIO_14)
#define __HAL_DET_RES		(GPIO_15)
#define __HAL_BOX_HOME_PS	(GPIO_16)
#define __HAL_SD_SET		(GPIO_19)
#define __HAL_PF			(GPIO_22)
#define __HAL_PBIN_PS		(GPIO_23)
#define __HAL_USB1_RESET	(GPIO_24)
#define __HAL_FUSB_SELECT	(GPIO_25)
#define __HAL_SU_SELECT		(GPIO_27)
#define __HAL_IF_SEL2		(GPIO_28)
#define __HAL_PBOUT_PS		(GPIO_35)

#if defined(PRJ_IVIZION2)
	#define __HAL_IF_LED		(GPIO_48)
	#define __HAL_IF_SET		(GPI_04)
	#define __HAL_CENT_CLOSE_PS	(GPIO_18)
#else
	#define __HAL_GPIO48		(GPIO_48)
#endif

#define __HAL_PSLED_ONOFF	(GPIO_49)
#define __HAL_STR_LEDONOFF	(GPIO_53)
#define __HAL_IF_SEL1		(GPIO_66)


//20210730 SPI STB GPIO化
#define __HAL_SPI0_STB		(GPIO_60)
#define	GPIO_SPI0_STB		(UINT16)(__HAL_SPI0_STB)

#if defined(PRJ_IVIZION2)
	#if (BV_UNIT_TYPE == ES_MODEL) && defined(PRJ_IVIZION2)
	//20210625 配線ミス修正
	#define __HAL_24V_DET		(GPIO_17)
	#elif (BV_UNIT_TYPE >= WS_MODEL) && defined(PRJ_IVIZION2)
		#define __HAL_5V_PG_N		(GPIO_17)
	#endif
	#if 1
	#define __HAL_N_FULL_PS		(GPIO_26)
	#define __HAL_BOX1_PS		(GPIO_50)
	#define __HAL_ENT_PS		(GPIO_51)
	#define __HAL_BOX2_PS		(GPIO_52)
	#define __HAL_EXIT_PS		(GPIO_54)
	#define __HAL_CENT_PS		(GPIO_65)
	#else
	#define __HAL_EXIT_PS		(GPIO_17)
	#define __HAL_24V_DET		(GPIO_26)
	#define __HAL_N_FULL_PS		(GPIO_50)
	#define __HAL_BOX1_PS		(GPIO_51)
	#define __HAL_BOX2_PS		(GPIO_52)
	#define __HAL_CENT_PS		(GPIO_54)
	#define __HAL_ENT_PS		(GPIO_65)
	#endif
#else
	//GIPOの番号で確認、excell資料準拠 UBAで使用していない番号の場合保留
	#define __HAL_EXIT_PS		(GPIO_54)	//use exit sensor 17->54
	#define __HAL_BOX1_PS		(GPIO_50)	//use Box検知 51->50
	#define __HAL_CENT_PS		(GPIO_65)	//use 幅よせポジションセンサ 54->65
	#define __HAL_ENT_PS		(GPIO_51)	//use 入口センサ 65->51
	//#define __HAL_24V_DET		(GPIO_26)	//not use
	#define __HAL_N_FULL_PS		(GPIO_26)	//not useUBA_GPIO 将来的には削除
	#define __HAL_PB_HOME		(GPIO_52)	/* PB Homeの*/
	#define __HAL_SHUTTER_HOME	(GPIO_18)	/* Shutter Home */
#endif

/****************************************************************/
/*						USB PHYリセット信号							*/
/****************************************************************/
#define	GPIO_RESETB0		(__HAL_USB0_RESET)
#define	GPIO_RESETB1		(__HAL_USB1_RESET)
#define	GPIO_USB_SELECT		(__HAL_USB_SELECT)

#define RESET_ASSERT		(0)
#define RESET_DEASSERT		(1)

#define USB_SELECT_REAR		(0)
#define USB_SELECT_FRONT	(1)

/****************************************************************/
/*						フェースプレートLED							*/
/****************************************************************/

/****************************************************************/
/*						ステータスLED								*/
/****************************************************************/

/****************************************************************/
/*						RFID制御信号							*/
/****************************************************************/

/****************************************************************/
/*						ポジションセンサー						*/
/****************************************************************/
#define	GPIO_SEN_LED		(__HAL_PSLED_ONOFF)
#define	GPIO_SEN_ENT		(__HAL_ENT_PS)
#define	GPIO_SEN_CEN		(__HAL_CENT_PS)
#define	GPIO_SEN_PBI		(__HAL_PBIN_PS)
#define	GPIO_SEN_PBO		(__HAL_PBOUT_PS)
#define	GPIO_SEN_EXT		(__HAL_EXIT_PS)
#define	GPIO_SEN_CEN_OPEN	(__HAL_CENT_OPEN_PS)

#if defined(PRJ_IVIZION2)
	#define	GPIO_SEN_CEN_CLOSE	(__HAL_CENT_CLOSE_PS)
#else
	#define	GPIO_SEN_PB_HOM		(__HAL_PB_HOME)
#endif

#define	GPIO_SEN_BOX_HOM	(__HAL_BOX_HOME_PS)
#define	GPIO_SEN_500BOX		(__HAL_BOX1_PS)
#define	GPIO_SEN_1000BOX	(__HAL_BOX2_PS)

/****************************************************************/
/*						糸検知センサー								*/
/****************************************************************/

/****************************************************************/
/*						割込み信号									*/
/****************************************************************/
#define	GPIO_VDET			(__HAL_PF)
#define	GPIO_FUSB_ONOFF		(__HAL_FUSB_ONOFF)

/****************************************************************/
/*						スイッチ信号								*/
/****************************************************************/
#define	GPIO_SU_SELECT		(__HAL_SU_SELECT)
#define SELECT_SS_MODE		(1)
#define SELECT_SU_MODE		(0)



/****************************************************************/
/*						IF LED信号								*/
/****************************************************************/
#define GPIO_IF_SW		(__HAL_IF_SET)
#define	GPIO_IF_LED		(__HAL_IF_LED)
#define GPIO_IF_SEL2	(__HAL_IF_SEL2)
#define GPIO_IF_SEL1	(__HAL_IF_SEL1)

/****************************************************************/
/*						テスト信号									*/
/****************************************************************/
//20210812 UART0テストモード時GPIO
#define __HAL_TEST_TRIGGER	(GPIO_61)
#define __HAL_TEST_LED		(GPIO_62)
#define	GPIO_TEST_TRIGGER	(__HAL_TEST_TRIGGER)
#define	GPIO_TEST_LED		(__HAL_TEST_LED)

/****************************************************************/
/*						SD Card信号								*/
/****************************************************************/
#define	SDMMC_DETECT		(__HAL_SD_SET)

/****************************************************************/
#endif		// HAL_GPIO_REG_H
