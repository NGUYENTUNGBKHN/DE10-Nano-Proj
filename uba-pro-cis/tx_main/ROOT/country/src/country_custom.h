/*
 * custom.h
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */
#pragma once

#ifndef COUNTRY_SRC_CUSTOM_H_
#define COUNTRY_SRC_CUSTOM_H_

/**** Host I/F type ****/
//#define _PROTOCOL_ENABLE_ID003 1

/**** skew ****/
/* tanΘ * 4096 */
/* 8°:575,10°:720 */
/* 5°:355, 3°:215, 2.5°:180 */
#define SKEW_THRESHOLD_10_0DGREE 720	/* 10.0° */
#define SKEW_THRESHOLD_8_0DGREE 575	/* 8.0° */
#define SKEW_THRESHOLD_5_0DGREE 355	/* 5.0° */
#define SKEW_THRESHOLD_3_0DGREE 215	/* 3.0° */
#define SKEW_THRESHOLD_2_5DGREE 180	/* 2.5° */

/**** Clear Window Size ****/
#define CLEAR_WINDOW_SIZE 40 //2024-03-08 10-> 40



/**** Cleaning Card ****/
//#define CLEANING_CARD 1


#define UV_MISSING_DATA		20	//UVデータ両端未使用データ長	
#define MAG_MISSING_DATA	10	//MAGデータ両端未使用データ長	

/**** denom size ****/
#define DENOMI_MAX 120
#define DENOMI_SIZE 120
#define DENOMI 120
#define BAR_INDX (DENOMI_MAX + 1)

#define EDGE_DETECT_ENABLE				1 //edge_detect enable DC or reversecompare only

#define COMPARE_ENABLE					1 //compare execution
#define TEST_COMPARE_ENABLE				0 //not identification　denomi increment (This define is enable and COMPARE_ENABLE = 1)

#define SKEW_CHECK_ENABLE				1 //escrow skew check
#define SIZE_CHECK_ENABLE				1 //escrow size check (long and short)

#define DOG_CHECK_ENABLE				0 //fitness dog-ear
#define TRAR_CHECK_ENABLE				0 //fitness tear (double dog-ear only)
#define DYE_CHECK_ENABLE				0 //fitness dye-note
#define HOLE_CHECK_ENABLE				0 //fitness hole

#define DOUBLE_CHECK_ENABLE				1 //block_compare double-paper
#define MCIR_CHECK_ENABLE				0 //block_compare MCIR
#define CIR_3COLOR_CHECK_ENABLE		0 //block_compare CIR3		Not used due to processing time
#define CIR_4COLOR_CHECK_ENABLE		0 //block_compare CIR4		Not used due to processing time
#define IR_CHECK_ENABLE					0 //block_compare IR		Not used due to processing time
#define CUSTOM_CHECK_ENABLE			0 //block_compare Custon	dbl note and cp cf check
#define POINT_CHECK_ENABLE				0 //block_compare Point		cp cf check
#define REF_CHECK_ENABLE				0 //block_compare Ref		EUR50.1 Rigth fake note only
#define PEN_CHECK_ENABLE				0 //block_compare Pen		Cut note check

#if 1 //UBA710 2023-09-28
	#define POINT_UV_CHECK_ENABLE			1 //block_compare Point-UV	CP check
	#define POINT_MAG_CHECK_ENABLE			1 //block_compare Point-MAG	CF check
#else
	#define POINT_UV_CHECK_ENABLE			0 //block_compare Point-UV	CP check
	#define POINT_MAG_CHECK_ENABLE			0 //block_compare Point-MAG	CF check
#endif

#define OTHER_CHECK_ENABLE				0 //block_compare Other	CP check
#define NN_1COLOR_CHECK_ENABLE			0 //block_compare NN1
#define NN_2COLOR_CHECK_ENABLE			0 //block_compare NN2
#define HOLOGRAM_CHECK_ENABLE			0 //block_compare hologram
#define THREAD_CHECK_ENABLE				0 //block_compare thread
#define MISSING_SECURITY_ENABLE			0
#define CATEGORY_CHECK_ENABLE			1 //category_ecb check

#define CLIP_CHECK_ENABLE				1 // clip check

#define PICH_MODE

#define MAG_DISABLE						 1	/* 磁気センサー無効： */

#define VARIDATION_RESULT_BIT			0xFF00
enum COMPARE_RESULT_FLG{//compare_flag
	BIT_EDGE_DETECT						= 0x0400,		// 外形検知
	BIT_COMPARE							= 0x0C00,	// 識別
	BIT_LENGTH_SHORT						= 0x1000,	// 短券
	BIT_LENGTH_LONG						= 0x1000,	// 長券
	BIT_CALC_MASH						= 0x0800,	// メッシュ計算失敗
	BIT_CALC_SYNC						= 0xF200,	// プログラムエラー　FPGAエラー 各種閾値異常
};
enum BLOCK_COMPARE_RESULT_FLG{//block_compare_flag
	BIT_DOUBLE_NOTE						= 0x00000001,	// 重券
	BIT_COUNTER_FAIT_MCIR				= 0x00000002,	// 偽造券(MCIR)
	BIT_COUNTER_FAIT_M3C				= 0x00000004,	// 偽造券(M3C)
	BIT_COUNTER_FAIT_M4C				= 0x00000008,	// 偽造券(M4C)
	BIT_COUNTER_FAIT_IR					= 0x00000010,	// 偽造券(IR)
	BIT_COUNTER_FAIT_CUSTOM			= 0x00000020,	// 偽造券(CUSTOM)
	BIT_COUNTER_FAIT_NN1				= 0x00000040,	// 偽造券(NN1)
	BIT_COUNTER_FAIT_NN2				= 0x00000080,	// 偽造券(NN2)
	BIT_DOUBLE_NOTE_CUSTOM			= 0x00000100,	// 重券(CUSTOM)
	BIT_COUNTER_FAIT_HOLOGRAM		= 0x00000200,	// ホログラム
	BIT_COUNTER_FAIT_THREAD			= 0x00000400,	// スレッド
	BIT_COUNTER_FAIT_POINT				= 0x00000800,	// 偽造券(POINT)
	BIT_COUNTER_FAIT_REF					= 0x00001000,	// 偽造券(REF)
	BIT_COUNTER_FAIT_PEN					= 0x00002000,	// 偽造券(PEN)
	BIT_COUNTER_FAIT_POINT_UV			= 0x00004000,	// 偽造券(Point-UV)
	BIT_COUNTER_FAIT_POINT_MAG			= 0x00008000,	// 偽造券(Point-MAG)
	BIT_COUNTER_FAIT_OTHER				= 0x10000000,	// 
};
enum CATEGORY_TYPE
{
	CATEGORY_ECB_NOT_SUPPORT	= 0xF0,
	CATEGORY_ECB_COUNTERFEIT	= 0xF1,
	CATEGORY_ECB_UNKNOWN		= 0xF2,
	CATEGORY_ECB_ST				= 0xF3,
	CATEGORY_ECB_UNFIT			= 0xF4,
};


/**** Country function ****/
#define		OPTION_NON_ILLEGAL_CREDIT		1		/*	0:With illegal Credit function、 1:Without illegal Credit function	*/

#if defined(UBA_RTQ)
/*******************************************************************************
* Double Paper Settings
*
*******************************************************************************/
	#define	RC1_DENOMI			0	/*   5 Euro	*//* rcTbl_dt1[0] */
	#define	RC2_DENOMI			1	/*  10 Euro	*//* rcTbl_dt1[1] */
	#define	RC3_DENOMI			2	/*  20 Euro	*//* rcTbl_dt1[2] */
	#define	RC4_DENOMI			3	/*  50 Euro	*//* rcTbl_dt1[3] */
	#define	RC5_DENOMI			4	/* 100 Euro	*//* rcTbl_dt1[4] */
	#define	RC6_DENOMI			5	/* 200 Euro	*//* rcTbl_dt1[5] */
	#define	RC7_DENOMI			6	/* 500 Euro	*//* rcTbl_dt1[6] */
	#define	RC8_DENOMI			7	/* 			*//* rcTbl_dt1[7] */
#endif

#endif /* COUNTRY_SRC_CUSTOM_H_ */
