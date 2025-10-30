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
* @file de_inked_note.c
* @brief 脱色検知
* @date 2019// Created.
*/
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>

#define EXT
#include "../common/global.h"

#define DEINK_MODE_HSV	1
#define DEINK_MODE_RGB	0

//レベル計算時の重みテーブル
float deink_weight_when_culculating_level[][DEINK_MAX_AREA] =
{
	{1.00f,0.00f,0.0f,0.0f,0.0f},	
	{0.70f,0.30f,0.0f,0.0f,0.0f},
	{0.65f,0.25f,0.1f,0.0f,0.0f},
	{0.65f,0.25f,0.1f,0.0f,0.0f},
	{0.65f,0.25f,0.1f,0.0f,0.0f}
};

//#define CSV_DB

//脱色検知のメイン関数
// in	バッファ番号
//		専用構造体
s16	de_inked_note(u8 buf_num ,ST_DEINKED *d_ink)
{
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	ST_SPOINT spt;
	s16 x = 0;
	s16 y = 0;

	u8 uf_count = 0;
	u8 genuine_count = 0;

	u8 area_num = 0;		//エリア数
	u8 i, j;
	u8 plane_num = 0;		//プレーン数

	s16 pix_count = 0;		//参照画素数 = abs(start_y - end_y) * abs(start_x - end_x);

	float total = 0.0f;		//各プレーン重みの計算結果
	float coordinate[3] = {{0}};		//各プレーンの座標

	float normalize_num = 1.0f / 255.0f;	//正規化パラメタ

	float tmp_level = 0.0f;
	u8 tmp_level2 = 0;
	u8 level_ary[DEINK_MAX_AREA] = { {0} };

#ifdef CSV_DB
	FILE *fp;			//filedebug
	F_CREATE_1(fp);		//filedebug
#endif

	spt.way = pbs->insertion_direction;		//方向設定

	d_ink->output_level = 0;

	for(area_num = 0; area_num < d_ink->area_num; ++area_num)		//参照エリアのループ
	{
		d_ink->res_out_put_val[area_num] = 0.0f;			//出力値初期化

		for(plane_num = 0; plane_num < d_ink->params[area_num].plane_num; ++plane_num)	//プレーン数分のループ
		{
			spt.l_plane_tbl = d_ink->params[area_num].plane[plane_num];					//プレーンの設定

			for ( y = d_ink->params[area_num].start_y; y > d_ink->params[area_num].end_y; y = y - d_ink->params[area_num].sampling_pich_y )			//採取エリア内ループ
			{
				for ( x = d_ink->params[area_num].start_x; x < d_ink->params[area_num].end_x; x = x + d_ink->params[area_num].sampling_pich_x )
				{
					spt.x = x;
					spt.y = y;
					new_L_Getdt(&spt ,pbs ,pnote_param);			//画素参照
					total += spt.sensdt;							//合計値計算
					pix_count++;									//参照数計算
					//histogram[spt.sensdt]++;						//ヒストグラム作成
				}
			}

			total = total / pix_count;								//参照数で割って255に正規化
			d_ink->avg_val[area_num][plane_num] = (u8)total;

			if(d_ink->select_color_mode == DEINK_MODE_RGB)
			{
				total = total * normalize_num;							//255で割って1に正規化
			}
			pix_count = 0;
#ifdef CSV_DB
			F_WRITE_LF(fp,total);	//filedebug
#endif

			d_ink->total[area_num][plane_num] = total;				//平均値を記録する

			if(d_ink->select_color_mode == DEINK_MODE_RGB)
			{

				coordinate[plane_num] = total;							//座標（平均値）を記録

				//計算結果と係数を計算
				d_ink->res_out_put_val[area_num] = (float)(total * d_ink->params[area_num].weight[plane_num]) + d_ink->res_out_put_val[area_num];
			}
		}

		//RGBをHSVに変換する
		if(d_ink->select_color_mode == DEINK_MODE_HSV)	
		{
			rgb2hsv(&d_ink->total[area_num][0],
				&d_ink->total[area_num][1],
				&d_ink->total[area_num][2]);

			coordinate[0] = d_ink->total[area_num][0];
			coordinate[1] = d_ink->total[area_num][1];
			coordinate[2] = d_ink->total[area_num][2];

			d_ink->res_out_put_val[area_num] = (float)(coordinate[0] * d_ink->params[area_num].weight[0]) +
				(coordinate[1] * d_ink->params[area_num].weight[1]) +
				(coordinate[2] * d_ink->params[area_num].weight[2]);
		}

		//切片を計算
		d_ink->res_out_put_val[area_num] = d_ink->res_out_put_val[area_num] + d_ink->params[area_num].bias;

		//点と平面の距離を計算する(float)
		d_ink->distance[area_num] = (    (	d_ink->params[area_num].weight[0] * coordinate[0] + 
			d_ink->params[area_num].weight[1] * coordinate[1] + 
			d_ink->params[area_num].weight[2] * coordinate[2] +
			d_ink->params[area_num].bias)) /
			(sqrtf	(	d_ink->params[area_num].weight[0] * d_ink->params[area_num].weight[0] +
			d_ink->params[area_num].weight[1] * d_ink->params[area_num].weight[1] +
			d_ink->params[area_num].weight[2] * d_ink->params[area_num].weight[2]));

		d_ink->level[area_num] = deink_level_dicision(d_ink->distance[area_num] ,d_ink->params[area_num].st_1_level_num ,d_ink->params[area_num].uf_1_level_num);
		level_ary[area_num] = d_ink->level[area_num];
		d_ink->output_level += d_ink->level[area_num];

		if(d_ink->level[area_num] >= pbs->fitness[FITNESS_DE_INKED_NOTE].bit.threshold_2)
		{
			uf_count++;
		}
		else
		{
			genuine_count++;
		}
	}

	
	for (i = 0; i < d_ink->area_num; ++i)		//参照エリアのループ
	{
		for (j = 0; j < d_ink->area_num; ++j)		//参照エリアのループ
		{
			if (level_ary[i] < level_ary[j])
			{
				tmp_level2 = level_ary[i];
				level_ary[i] = level_ary[j];
				level_ary[j] = tmp_level2;
			}

		}
	}

	for (area_num = 0; area_num < d_ink->area_num; ++area_num)		//参照エリアのループ
	{
		tmp_level += (float)level_ary[area_num] * deink_weight_when_culculating_level[d_ink->area_num-1][area_num];
	}

	d_ink->output_level = (u8)(tmp_level + 0.5f);

	//d_ink->uf_count = uf_count;
	//d_ink->output_level = (d_ink->output_level / d_ink->area_num);
	if(d_ink->output_level == 0)
	{
		d_ink->output_level = 1;
	}

	if(d_ink->comparison_method == 0)
	{
		//多数決
		if (uf_count > genuine_count) 
		{
			d_ink->res_judge = 1;
#ifndef CSV_DB
			return 1;	//UF券に発火
#endif
		}
		else
		{
			d_ink->res_judge = 0;
#ifndef CSV_DB
			return 0;	//正券に発火
#endif
		}

	}
	else if(d_ink->comparison_method == 1)
	{

		//and
		if (uf_count == d_ink->area_num) 
		{
			d_ink->res_judge = 1;
#ifndef CSV_DB
			return 1;	//UF券に発火
#endif
		}
		else
		{
			d_ink->res_judge = 0;
#ifndef CSV_DB
			return 0;	//正券に発火
#endif
		}
	}
#ifdef CSV_DB
	F_WRITE(fp,d_ink->res_judge);	//filedebug
	F_N(fp);				//filedebug
	F_CLOSE(fp);			//filedebug
#endif

	return 0;	//正券に発火

	////float image_histogram[255] = {0};

	////エリア決定
	////エリア内採取
	////ヒストグラム作成
	//return get_de_ink_histogram(
	//	pbs, 
	//	pnote_param ,
	//	d_ink->start_x ,
	//	d_ink->start_y,
	//	d_ink->end_x, 
	//	d_ink->end_y ,
	//	d_ink->sampling_pich ,
	//	d_ink->plane ,
	//	d_ink->plane_num,
	//	&d_ink->res_out_put_val);

	//NNに入力
	//出力
	//return de_ink_nn(
	//	image_histogram,
	//	d_ink->normalize_num,
	//	d_ink->input_node_num,
	//	d_ink->hidden_node_num,
	//	d_ink->output_node_num,
	//	d_ink->phidden_weight,
	//	d_ink->pout_weight,
	//	&d_ink->fitness_out_put_val,
	//	&d_ink->de_ink_out_put_val);

}

/****************************************************************/
/**
* @brief		レベルを計算する
*@param[in]	distance  平面との距離
level_fit Fitレベル
level_uf  UFレベル
*@param[out]	なし
* @return		レベル
*/
/****************************************************************/
u8 deink_level_dicision(float distance ,float level_fit,float level_uf)
{
	u8  value = 1;
	s32 temp = 0;

	if(distance > 0)
	{
		temp = (s32)(50 -  (distance / level_fit)+0.5f);
		if(temp < 0)
		{
			value = 0;
		}
		else
		{
			value = (u8)temp;
		}
	}
	else
	{
		temp = (s32)((50 - (distance / level_uf))+0.5f);
		if(temp > 99)
		{
			value = 99;
		}
		else
		{
			value = (u8)temp;
		}

	}

	return 100 - value;
}

//********************************
//指定あエリア内の画素を指定のピッチで採取して
//ヒストグラムを作成する。
//
//in
//
//s16 get_de_ink_histogram(ST_BS *pbs ,ST_NOTE_PARAMETER *pnote_param ,s16 start_x, s16 start_y ,s16 end_x ,s16 end_y ,s16 pich ,u8 *plane, float *histogram ,u8 plane_max_num ,float *out_put_val)
//{
//	ST_SPOINT spt;
//
//	s16 x = 0;
//	s16 y = 0;
//
//	u8 plane_num = 0;
//
//	s16 pix_count = abs(start_y - end_y) * abs(start_x - end_x);
//
//	float total[3] = {0};	//B,G,R
//	float res = 0.0;
//
//	spt.way = pbs->insertion_direction;
//
//	for(plane_num = 0; plane_num < plane_max_num; ++plane_num)	//プレーン数分のループ
//	{
//		spt.l_plane_tbl = plane[plane_num];
//	
//		for ( y = start_y; y > end_y; y -=1 )//y - pich )			//採取エリア内ループ
//		{
//			for ( x = start_x; x < end_x; x += 1)//x + pich )
//			{
//				spt.x = x;
//				spt.y = y;
//				new_L_Getdt(&spt ,pbs ,pnote_param);
//				total[plane_num] += spt.sensdt;
//
//				//histogram[spt.sensdt]++;						//ヒストグラム作成
//			}
//		}
//
//		total[plane_num] /= pix_count;	
//		total[plane_num] /= 255;	
//
//	}
//
//	*out_put_val = (float)((total[0] * 31.282 + total[1] * -76.577 + total[2] * -112.484 + 18.173 ));	//線形分離の識別式
//
//	//出力比較
//	if (0 > out_put_val) 
//	{
//		return 1;	//UF券に発火
//	}
//	else
//	{
//		return 0;	//正券に発火
//	}
//
//}
//
////********************************
////NNで分類する。
////
////in
////	
//u8 de_ink_nn(float *histogram , float normaliza_num ,u16 input_num ,u16 hidden_num , u16 output_num, float *phidden_weight ,float *pout_weight ,float *fitness_out_put_val ,float *de_ink_out_put_val)
//{
//	//float input[255];
//	float hidden[16];
//	float output[2];
//
//	u16 hidden_weight = 0;
//	u16 output_weight = 0;
//	u16 i = 0;
//	u16 j = 0;
//
//	//入力層の正規化
//	for (j = 0; j < input_num; j++) 
//	{
//		histogram[j] = histogram[j] * normaliza_num * 0.99f + 0.01;
//	}
//
//
//	//中間層初期化
//	for (j = 0; j < hidden_num; j++) 
//	{
//		hidden[j] = 0.0f;
//	}
//
//	//順伝搬計算
//	for (i = 0; i < hidden_num; i++) 
//	{
//		for (j = 0; j < input_num; j++) 
//		{
//			hidden[i] += (histogram[j] * phidden_weight[hidden_weight]);
//			hidden_weight++;
//		}
//	}
//
//	for (i = 0; i < hidden_num; i++)
//	{
//		hidden[i] = 1.0f / (1.0f + exp(-hidden[i]));
//	}
//	
//	//出力層の初期化
//	for (i = 0; i < output_num; i++) 
//	{
//		output[i] = 0.0f;
//	}
//
//	//順伝搬の計算
//	for (i = 0; i < output_num; i++)
//	{
//		for (j = 0; j < hidden_num; j++)
//		{
//			output[i] += (hidden[j] * pout_weight[output_weight]);
//			output_weight++;
//		}
//	}
//	for (i = 0; i < output_num; i++) 
//	{
//		output[i] = 1.0f / (1.0f + exp(-output[i]));
//	}
//
//	*fitness_out_put_val = output[0];
//	*de_ink_out_put_val = output[1];
//
//	//出力比較
//	if (output[0] < output[1]) 
//	{
//		return 1;	//UF券に発火
//	}
//	else
//	{
//		return 0;	//正券に発火
//	}
//
//}




//　END
