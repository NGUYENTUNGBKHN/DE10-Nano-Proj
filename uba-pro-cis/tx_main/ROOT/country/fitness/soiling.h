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
 * @file soiling.h
 * @brief　汚れ検知
 * @date 2019 Created.
 */
/****************************************************************************/

#ifndef	_SOILING_H
#define	_SOILING_H

#define SOILING_MAX_AREA 5	
#define SOILING_MAX_PLANE 3	

typedef struct
{
	u8 plane[SOILING_MAX_PLANE];
	u8 plane_num;

	s16 start_x;			//画素採取範囲の左上と右下の座標
	s16 start_y;
	s16 end_x;
	s16 end_y;
	u8 sampling_pich_x;		//範囲内をサンプリングするピッチ
	u8 sampling_pich_y;		//範囲内をサンプリングするピッチ
	u8 padding[2];
	float bias;				//切片
	float weight[3];			//重み
	float uf_1_level_num;	//平面とufベクトルの距離/50の値（レベル１）
	float st_1_level_num;	//平面とstベクトルの距離/50の値（レベル１）


} ST_SOILING_EACH_AREA_PARAMS;

typedef struct
{
	//入力

	//u16 input_node_num;		//ノード数
	//u16 hidden_node_num;
	//u16 output_node_num;

	//float *phidden_weight;	//重み
	//float *pout_weight;
	//float *p_weight;			//重み
	//float bias;					//切片

	//s16 start_x;			//画素採取範囲の左上と右下の座標
	//s16 start_y;
	//s16 end_x;
	//s16 end_y;
	//s16 sampling_pich;		//範囲内をサンプリングするピッチ

	//ST_SOILING_EACH_AREA_PARAMS params[SOILING_MAX_AREA];	//参照エリア構造体
	ST_SOILING_EACH_AREA_PARAMS *params;	//参照エリア構造体
	u8 area_num;											//参照エリア数
	//float st_1_level_num;	//fit券群と平面との距離	
	//float uf_1_level_num;

	u8 comparison_method;	//基準比較方法 0：多数決　1：all
	u8	select_color_mode;				//色空間モデルの設定 0：RGB　1：HSV
	u8 padding;

	//中間情報
	float res_out_put_val[5];	//計算結果
	float total[SOILING_MAX_AREA][SOILING_MAX_PLANE];
	float distance[5];	//点と平面の距離
	u8	level[5];		//レベル

	u8 avg_val[5][3];			//RGBの平均値
	//出力
	//float fitness_out_put_val;		//真券の発火値
	//float de_ink_out_put_val;	//偽券の発火値
	u8 res_judge;			//判定結果
	u8 uf_count;			//ufとと判定されたエリアの数
	u16 output_level;		//出力されたレベル

} ST_SOILING;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	soiling(u8 buf_num ,ST_SOILING *st);
	u8  stain_level_dicision(float distance ,float level_fit,float level_uf);
	//s16 get_soiling_histogram(ST_BS *pbs ,ST_NOTE_PARAMETER *pnote_param ,s16 start_x, s16 start_y ,s16 end_x ,s16 end_y ,s16 pich ,u8 *plane ,u8 plane_max_num ,float *out_put_val);
	//u8 soiling_nn(float *histogram , float normaliza_num ,u16 input_num ,u16 hidden_num , u16 output_num, float *phidden_weight ,float *pout_weight ,float *fitness_out_put_val ,float *de_ink_out_put_val);

	float min3(float x, float y, float z);
	void rgb2hsv(float* h, float* s, float* v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SOILING_H */
