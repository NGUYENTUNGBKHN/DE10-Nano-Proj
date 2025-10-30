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
 * @file de_inkde_note.h
 * @brief　脱色検知
 * @date 2019 Created.
 */
/****************************************************************************/

#ifndef	_Di_H
#define	_Di_H

#define DEINK_MAX_AREA 5	
#define DEINK_MAX_PLANE 3	

typedef struct
{
	u8 plane[DEINK_MAX_PLANE];
	u8 plane_num;

	s16 start_x;			//画素採取範囲の左上と右下の座標
	s16 start_y;
	s16 end_x;
	s16 end_y;
	u8 sampling_pich_x;		//範囲内をサンプリングするピッチ
	u8 sampling_pich_y;		//範囲内をサンプリングするピッチ
	u8 padding[2];
	float bias;					//切片
	float weight[3];			//重み
	float uf_1_level_num;	//平面とufベクトルの距離/50の値（レベル１）
	float st_1_level_num;	//平面とstベクトルの距離/50の値（レベル１）

} ST_DEINK_EACH_AREA_PARAMS;

typedef struct
{
	//入力

	//u16 input_node_num;		//ノード数
	//u16 hidden_node_num;
	//u16 output_node_num;

	//double *phidden_weight;	//重み
	//double *pout_weight;
	//double *p_weight;			//重み
	//double bias;					//切片

	//s16 start_x;			//画素採取範囲の左上と右下の座標
	//s16 start_y;
	//s16 end_x;
	//s16 end_y;
	//s16 sampling_pich;		//範囲内をサンプリングするピッチ

	//ST_DEINK_EACH_AREA_PARAMS params[DEINK_MAX_AREA];
	ST_DEINK_EACH_AREA_PARAMS *params;
	u8 area_num;

	u8 comparison_method;	//基準比較方法 0：多数決　1：a11
	u8	select_color_mode;				//色空間モデルの設定 0：RGB　1：HSV
	u8	padding;

	//中間情報
	float res_out_put_val[5];	//計算結果
	float total[DEINK_MAX_AREA][DEINK_MAX_PLANE];
	float distance[5];	//点と平面の距離
	u8	level[5];		//レベル
	u8 avg_val[5][3];			//RGBの平均値

	//出力
	//double fitness_out_put_val;		//真券の発火値
	//double de_ink_out_put_val;	//偽券の発火値
	u8 res_judge;			//判定結果
	u8 uf_count;			//ufとと判定されたエリアの数
	u16 output_level;		//出力されたレベル



} ST_DEINKED;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	de_inked_note(u8 buf_num ,ST_DEINKED *st);
	u8  deink_level_dicision(float distance ,float level_fit,float level_uf);
	//s16 get_de_ink_histogram(ST_BS *pbs ,ST_NOTE_PARAMETER *pnote_param ,s16 start_x, s16 start_y ,s16 end_x ,s16 end_y ,s16 sampling_pich ,u8* plane, u8 plane_max_num ,double *out_put_val);
	//u8 de_ink_nn(double *histogram , double normaliza_num ,u16 input_num ,u16 hidden_num , u16 output_num, double *phidden_weight ,double *pout_weight ,double *fitness_out_put_val ,double *de_ink_out_put_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _Di_H */
