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
* @file soiling.c
* @brief 汚れ検知
* @date 2019// Created.
*/
/****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define EXT
#include "../common/global.h"

#define SOIL_MODE_HSV	1
#define SOIL_MODE_RGB	0




//汚れ検知のメイン関数
// in	バッファ番号
//		専用構造体
s16	soiling(u8 buf_num ,ST_SOILING *soi)
{
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	ST_SPOINT spt;

	s16 x = 0;
	s16 y = 0;

	u8 uf_count = 0;
	u8 genuine_count = 0;

	u8 area_num = 0;		//エリア数
	u8 plane_num = 0;		//プレーン数

	s16 pix_count = 0;		//参照画素数 = abs(start_y - end_y) * abs(start_x - end_x);

	//float debug_total[3][3];		//各プレーン重みの計算結果

	float total = 0.0f;		//各プレーン重みの計算結果
	float coordinate[3] = {{0}};		//各プレーンの座標
	float normalize_num = 1.0f / 255.0f;	//正規化パラメタ


	//float distance[5] = 0.0f	//点と平面の距離

	spt.way = pbs->insertion_direction;		//方向設定

	soi->output_level = 0;

	for(area_num = 0; area_num < soi->area_num; ++area_num)		//参照エリアのループ
	{

		if(soi->params[area_num].st_1_level_num == 0 && soi->params[area_num].uf_1_level_num == 0)
		{
			break;
		}

		soi->res_out_put_val[area_num] = 0.0f;			//出力値初期化

		for(plane_num = 0; plane_num < soi->params[area_num].plane_num; ++plane_num)	//プレーン数分のループ
		{
			spt.l_plane_tbl = soi->params[area_num].plane[plane_num];					//プレーンの設定

			for ( y = soi->params[area_num].start_y; y > soi->params[area_num].end_y; y = y - soi->params[area_num].sampling_pich_y )			//採取エリア内ループ
			{
				for ( x = soi->params[area_num].start_x; x < soi->params[area_num].end_x; x = x + soi->params[area_num].sampling_pich_x )
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
			soi->avg_val[area_num][plane_num] = (u8)total;
			//c_total = (u8)total;
			//total = (float)c_total;
			if(soi->select_color_mode == SOIL_MODE_RGB)
			{
				total = total * normalize_num ;							//255で割って1に正規化
			}

			pix_count = 0;

			soi->total[area_num][plane_num] = total;				//平均値を記録する

			if(soi->select_color_mode == SOIL_MODE_RGB)
			{
				coordinate[plane_num] = total;							//座標（平均値）を記録


				//計算結果と係数を計算
				soi->res_out_put_val[area_num] = (float)(total * soi->params[area_num].weight[plane_num]) + soi->res_out_put_val[area_num];
			}


		}
		if(soi->select_color_mode == SOIL_MODE_HSV)
		{
			rgb2hsv(&soi->total[area_num][0],
				&soi->total[area_num][1],
				&soi->total[area_num][2]);

			coordinate[0] = soi->total[area_num][0];
			coordinate[1] = soi->total[area_num][1];
			coordinate[2] = soi->total[area_num][2];

			soi->res_out_put_val[area_num] = (float)(coordinate[0] * soi->params[area_num].weight[0]) +
				(coordinate[1] * soi->params[area_num].weight[1]) +
				(coordinate[2] * soi->params[area_num].weight[2]);
		}

		//切片を計算
		soi->res_out_put_val[area_num] = soi->res_out_put_val[area_num] + soi->params[area_num].bias;

		//点と平面の距離を計算する(float) 分子の絶対値は外してある。　平面に対して±でSTorUFを判断したいから。
		soi->distance[area_num] = ( (		soi->params[area_num].weight[0] * coordinate[0] + 
											soi->params[area_num].weight[1] * coordinate[1] + 
											soi->params[area_num].weight[2] * coordinate[2] +
											soi->params[area_num].bias)) /
								(sqrtf	(	soi->params[area_num].weight[0] * soi->params[area_num].weight[0] +
											soi->params[area_num].weight[1] * soi->params[area_num].weight[1] +
											soi->params[area_num].weight[2] * soi->params[area_num].weight[2]));


		soi->level[area_num] = stain_level_dicision(soi->distance[area_num] ,soi->params[area_num].st_1_level_num ,soi->params[area_num].uf_1_level_num);
		//soi->level[area_num] = stain_level_dicision(soi->distance[area_num] ,soi->st_1_level_num ,soi->uf_1_level_num);
		soi->output_level += soi->level[area_num];

		if(soi->level[area_num] >= pbs->fitness[FITNESS_SOILING].bit.threshold_2)
		{
			uf_count++;
		}
		else
		{
			genuine_count++;
		}
	}

	soi->uf_count = uf_count;
	soi->output_level = (soi->output_level / soi->area_num);
	if(soi->output_level == 0)
	{
		soi->output_level = 1;
	}

	if(soi->comparison_method == 0)
	{
		//多数決
		if (uf_count > genuine_count) 
		{
			soi->res_judge = 1;
			return 1;	//UF券に発火
		}
		else
		{
			soi->res_judge = 0;
			return 0;	//正券に発火
		}

	}
	else if(soi->comparison_method == 1)
	{

		//and
		if (uf_count == soi->area_num) 
		{
			soi->res_judge = 1;
			return 1;	//UF券に発火
		}
		else
		{
			soi->res_judge = 0;
			return 0;	//正券に発火
		}
	}


	return 0;	//正券に発火
	////出力比較
	//if (0 > soi->params[area_num].res_out_put_val) 
	//{
	//	soi->res_judge = 1;
	//	return 1;	//UF券に発火
	//}
	//else
	//{
	//	soi->res_judge = 0;
	//	return 0;	//正券に発火
	//}


	//float image_histogram[256] = {0};

	////エリア決定
	////エリア内採取
	////ヒストグラム作成
	//return get_soiling_histogram(
	//	pbs, 
	//	pnote_param ,
	//	soi->start_x ,
	//	soi->start_y,
	//	soi->end_x, 
	//	soi->end_y ,
	//	soi->sampling_pich ,
	//	soi->plane  ,
	//	soi->plane_num,
	//	&soi->res_out_put_val);

	//////NNに入力
	//////出力
	////return de_ink_nn(
	////	image_histogram,
	////	soi->normalize_num,
	////	soi->input_node_num,
	////	soi->hidden_node_num,
	////	soi->output_node_num,
	////	soi->phidden_weight,
	////	soi->pout_weight,
	////	&soi->fitness_out_put_val,
	////	&soi->de_ink_out_put_val);

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
u8 stain_level_dicision(float distance ,float level_fit,float level_uf)
{
	u8  value = 1;
	s32 temp = 0;
	float num = 0.0f;

	//level_fit = level_fit * 2;
	//level_uf = level_uf * 2;

    if(distance > 0)
	{ 
		num = distance / (level_fit * 1);
        temp = (s32)((50 -  (num)) + 0.5f);
        if(temp < 1)
		{
            value = 1;
		}
		else
		{
			value = (u8)temp;
		}
	}
    else
	{
		num = distance / (level_uf * 1);
        temp = (s32)((50 - (num)) + 0.5f);
        if(temp > 100)
		{
            value = 100;
		}
		else
		{
			value = (u8)temp;
		}

	}

	    return 101 - value;
}

float min3(float x, float y, float z)
{
    float min = x;
    
    if (y < min) min = y;
    if (z < min) min = z;
    return (min);
}



/****************************************************************/
/**
 * @brief		RGBをHSVに変換する
 *				HSVのHの範囲は0～100です。
 *				最終結果は0～1に正規化された数値です。
 * 
 *@param[in]	h	赤
				s	緑
				v	青

 *@param[out]	h	hue
				s	saturation
				v	value
 * 
 * @return		なし
 */
 /****************************************************************/
void rgb2hsv(float* h, float* s, float* v)
{
#define normalize_360 (0.0027778f)
#define normalize_100 (0.01f)



	float	diff = 0.0f;
	float	mx	 = 0.0f;
	float	mn	 = 0.0f;
	float   r = 0.0f;
	float   g = 0.0f;
	float   b = 0.0f;
	u8 i;
	float ary[3] = {0.0f ,0.0f ,0.0f};

	r = *h;
	g = *s;
	b = *v;

	ary[0] = r;
	ary[1] = g;
	ary[2] = b;


	mx = r;
	mn = r;

	//最大
	for(i = 0; i < 3; i++)
	{
		if(mx < ary[i])  
		{
			mx = ary[i]; 
		}
	}

	//最小
	for(i = 0; i < 3; i++)
	{
		if(mn > ary[i]) 
		{
			mn = ary[i];   
		}
	}

	//最大と最小の差
	diff = mx - mn;

	//Hを計算
	if(mx == mn)
	{
		*h = 0;
	}
	else if( mx == r)
	{
		*h = (60 * ((g - b) / diff) + 360) * normalize_360 * 100;
	}
	else if( mx == g)
	{ 
		*h = (60 * ((b - r) / diff) + 120 + 360) * normalize_360 * 100;
	}
	else if( mx == b)
	{
		*h = (60 * ((r - g) / diff) + 240 + 360) * normalize_360 * 100;
	}

	if (*h < 0) 
	{ 
		*h = *h + 100;
	}
	if (*h > 100) 
	{
		*h = *h - 100;
	}

	//Sを計算
	if(mx != 0)
	{
		*s = diff / mx * 100;
	}
    else
	{
		*s = 0;
	}
    
	//Vを計算
	*v = mx / 255 * 100;

	*h = *h * normalize_100;
	*s = *s * normalize_100;
	*v = *v * normalize_100;

	return ;

}


#define M 1

//中央値を取得する関数
s32 getMedian(s32 array[], s32 num) 
{
	//最初に配列を並び替える maxソートで行った
	s32 damy = num;
	s32 i = 0;
	s32 v = 0;
	s32 tmp = 0;
	s32 median = 0;

	for (i = 0; i < num; i++) 
	{
		for (v = 0; v < damy - 1; v++) 
		{
			if (array[v] < array[v + 1]) {

				tmp = array[v + 1];
				array[v + 1] = array[v];
				array[v] = tmp;
			}
		}
		damy = damy - 1;
	}

	median = array[(u32)(num * 0.5f) + 1];
	
	return  median;
}

//　END


