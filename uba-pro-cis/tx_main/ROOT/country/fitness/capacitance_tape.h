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
 * @file capacitance_tape.h
 * @brief　静電容量テープ検知
 * @date 2020/06/29 Created.
 */
/****************************************************************************/

#ifndef	_Cap_Tp_H
#define	_Cap_Tp_H

//コード追加していく
#define RES_ERR						(0x0010)	//エラー
#define RES_SUB_SCANLINE_SHORAGE	(0x0010)	//副走査方向のデータ数が不足している。

#define CAP_TAPE_MAX_ELEMENT_COUNT	(5000)		//最大要素数


enum karnel_type
{
	cap_rbf = 0x0,
	cap_sigmoid,
	cap_polynomial,
	cap_liner,
};

typedef struct
{
	s16 start_x;
	s16 start_y;
	s16 end_x;
	s16 end_y;

} ST_CAPACITANCE_TAPE_REFERENCE_AREA;


typedef struct
{
	//入力
	u8  black_correction_s;		                                //黒補正計算ライン開始
	u8	black_correction_e;		                                //黒補正計算終了
	u8  reference_area_num;		                                //参照エリア数
	u8	padding;

	float tape_judg_thr;		                                //テープ判定閾値レベル
	u16 x_interval;				                                //x座標の間引きの間隔
	u16 y_interval;				                                //y座標の間引きの間隔
	u32 first_thrs_num;			                                //第一閾値配列の要素数
	float *p_first_thrs;								        //第一閾値配列
	ST_CAPACITANCE_TAPE_REFERENCE_AREA *p_reference_area;		//参照エリア構造体のオフセット

	//中間
	float input_data[CAP_TAPE_MAX_ELEMENT_COUNT];		//入力データ
	float diff_data[CAP_TAPE_MAX_ELEMENT_COUNT];		//差分データ
	//float thrs_data[CAP_TAPE_MAX_ELEMENT_COUNT];		//差分データ
	float tape_total_area;
	float bc_val[128];			//黒補正値


	//出力
	u8 level;					//レベル
	u8 result;					//結果
	u16 err_code;				//エラーコード
	
} ST_CAPACITANCE_TAPE;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	capacitance_tape(u8 buf_num ,ST_CAPACITANCE_TAPE *st);
	//float get_median(u16 ary[], s32 num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _Cap_Tp_H */
