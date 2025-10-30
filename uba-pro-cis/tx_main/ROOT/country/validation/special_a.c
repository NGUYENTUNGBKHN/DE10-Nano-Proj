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
* MODEL NAME : 識別共有
* @file special_a.c
* @brief 特殊Aセンサ検知の実装ファイルです。
* @date Created. 2020/12/28
* @author JCM. OSAKA TECHNICAL RESARCH 1 GROUP ELEMENTAL TECHNOLOGY RECERCH DEPT.
*/
/****************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define EXT
#include "../common/global.h"

#ifdef VS_DEBUG
EXTERN int debug_logi_view;
#endif
/*
	ver1.0
	　初回
*/

/****************************************************************/
/**
* @brief		special_a.c
*@param[in]		buf	:	バッファ番号
				spa	:	特殊Aセンサ検知構造体
*@param[out]	特殊Aセンサ検知結果
* @return		0
*/
/****************************************************************/
s16 special_a(u8 buf, ST_SPECIAL_A* spa)
{
	ST_SPOINT spt = {0};
	ST_SPOINT spt_r_r = {0};
	ST_VERTEX outline = {0};
	
	u8 sp_num;
	u8 sp_num_min;
	u8 sp_num_max;
	float sub;
	float pitch;

	u16 ii;
	s16 yy;
	//s16 x_end;
	//s16 x_start;
	s16 y_end;
	s16 y_start;
	s16 py_start;
	s16 py_end;
	float upper;
	float bottom;
	float right;
	float left;

	s16 dt;
	float dis;
	float min_dis;
	u8 min_dis_num = 0;

	u16 ch_x[16] = 
		{	SP_A0,	SP_A1,	SP_A2,	SP_A3,
			SP_A4,	SP_A5,	SP_A6,	SP_A7,
			SP_A8,	SP_A9,	SP_A10,	SP_A11,
			SP_A12,	SP_A13,	SP_A14,	SP_A15	};

	s16 center_x[SPA_NUM_LIMIT] = {0};
	s16 center_y[SPA_NUM_LIMIT] = {0};
	u32 tempx[SPA_NUM_LIMIT] = {0};

	spt.way = work[buf].pbs->insertion_direction;
	spt.l_plane_tbl = UP_SP_A;
	spt.p_plane_tbl = OMOTE_SP_A;
	spt.trace_flg = 1;

	spt_r_r.way = work[buf].pbs->insertion_direction;
	spt_r_r.l_plane_tbl = OMOTE_R_R;
	spt_r_r.p_plane_tbl = UP_R_R;

	sp_num_min = (u8)work[buf].pbs->PlaneInfo[UP_SP_A].main_effective_range_min;
	sp_num_max = (u8)work[buf].pbs->PlaneInfo[UP_SP_A].main_effective_range_max;
	sub = work[buf].note_param.sub_dpi_cor[UP_SP_A];
	pitch = work[buf].pbs->PlaneInfo[UP_SP_A].sub_sampling_pitch;

	//外形情報
	outline.left_up_x = work[buf].pbs->left_up_x;
	outline.left_up_y = work[buf].pbs->left_up_y;
	outline.left_down_x = work[buf].pbs->left_down_x;
	outline.left_down_y = work[buf].pbs->left_down_y;
	outline.right_up_x = work[buf].pbs->right_up_x;
	outline.right_up_y = work[buf].pbs->right_up_y;
	outline.right_down_x = work[buf].pbs->right_down_x;
	outline.right_down_y = work[buf].pbs->right_down_y;

	//傾き
	left = (float)(outline.left_down_y - outline.left_up_y) / (outline.left_down_x - outline.left_up_x);
	right = (float)(outline.right_down_y - outline.right_up_y) / (outline.right_down_x - outline.right_up_x);
	upper = (float)(outline.left_up_y - outline.right_up_y) / (outline.left_up_x - outline.right_up_x);
	bottom = (float)(outline.left_down_y - outline.right_down_y) / (outline.left_down_x - outline.right_down_x);

	//範囲設定
	for(ii = 0; ii < spa->num; ii++)
	{
		spt_r_r.x = spa->para[ii].x;
		spt_r_r.y = spa->para[ii].y;
		new_L2P_Coordinate(&spt_r_r, work[buf].pbs, &work[buf].note_param);
		center_x[ii] = (s16)spt_r_r.x;
		center_y[ii] = (s16)spt_r_r.y;

#ifdef VS_DEBUG
		if(debug_logi_view != 0)
		{
			deb_para[0] = 3;
			deb_para[1] = center_x[ii];
			deb_para[2] = center_y[ii];
			callback(deb_para);
		}
#endif
	}

	//特殊A各ラインの探索開始位置を求める
	for(sp_num = sp_num_min; sp_num <= sp_num_max; sp_num++)
	{
		y_start = 0;
		y_end = 0;

		//紙幣外の場合は無視
		if (ch_x[sp_num - sp_num_min] < outline.left_up_x && ch_x[sp_num - sp_num_min] < outline.left_down_x)
		{
			continue;
		}
		else if (ch_x[sp_num - sp_num_min] > outline.right_up_x && ch_x[sp_num - sp_num_min] > outline.right_down_x)
		{
			continue;
		}
		//ラインが短辺にかかっている場合は無視
		else if( ch_x[sp_num - sp_num_min] >= outline.left_down_x && ch_x[sp_num - sp_num_min] <= outline.left_up_x )
		{
			//y_start = (s16)(left*(ch_x[sp_num - sp_num_min] - outline.left_down_x) + outline.left_down_y);
			//y_end = (s16)(bottom*(ch_x[sp_num - sp_num_min] - outline.left_down_x) + outline.left_down_y);
			continue;
		}
		else if(ch_x[sp_num - sp_num_min] <= outline.left_down_x && ch_x[sp_num - sp_num_min] >= outline.left_up_x)
		{
			//y_start = (s16)(upper*(ch_x[sp_num - sp_num_min] - outline.left_up_x) + outline.left_up_y);
			//y_end = (s16)(left*(ch_x[sp_num - sp_num_min] - outline.left_down_x) + outline.left_down_y);
			continue;
		}
		else if( ch_x[sp_num - sp_num_min] >= outline.right_up_x && ch_x[sp_num - sp_num_min] <= outline.right_down_x )
		{
			//y_start = (s16)(right*(ch_x[sp_num - sp_num_min] - outline.right_down_x) + outline.right_down_y);
			//y_end = (s16)(bottom*(ch_x[sp_num - sp_num_min] - outline.right_down_x) + outline.right_down_y);
			continue;
		}
		else if(ch_x[sp_num - sp_num_min] <= outline.right_up_x && ch_x[sp_num - sp_num_min] >= outline.right_down_x)
		{
			//y_start = (s16)(upper*(ch_x[sp_num - sp_num_min] - outline.right_up_x) + outline.right_up_y);
			//y_end = (s16)(right*(ch_x[sp_num - sp_num_min] - outline.right_down_x) + outline.right_down_y);
			continue;
		}
		else
		{
			y_start = (s16)(upper*(ch_x[sp_num - sp_num_min] - outline.left_up_x) + outline.left_up_y);
			y_end = (s16)(bottom*(ch_x[sp_num - sp_num_min] - outline.left_down_x) + outline.left_down_y);
		}


		//特殊A物理座標に変換
		py_start = (s16)(y_start * sub);
		py_end = (s16)((y_end) * sub);
					
		for(ii = 0; ii < spa->num; ii++)
		{	//先にx座標を計算
			tempx[ii] = (ch_x[sp_num - sp_num_min] - center_x[ii]) * (ch_x[sp_num - sp_num_min] - center_x[ii]);
		}

		for(yy = py_start; yy < py_end; yy++)
		{
			//初期化
			min_dis = SPA_INI_DISTANCE;
			dis = 0;
			
			spt.x = sp_num;
			spt.y = yy;
			P_Getdt_8bit_only(&spt, work[buf].pbs);

			dt = spt.sensdt;
		
			//センサ値0で戻る
			if(dt == 0)
			{
				continue;
			}
				
			//一番近い距離の設定範囲を求める
			for(ii = 0; ii < spa->num; ii++)
			{
				dis = sqrtf( tempx[ii] + (yy * pitch - center_y[ii])*(yy * pitch - center_y[ii]) );
				if(min_dis > dis)
				{
					min_dis = dis;
					min_dis_num = (u8)ii;
				}
			}

			if((min_dis < spa->dis_thr))
			{

#ifdef VS_DEBUG
				if(debug_logi_view != 0)
				{
					deb_para[0] = 2;
					deb_para[1] = center_x[min_dis_num];
					deb_para[2] = center_y[min_dis_num];
					deb_para[3] = center_x[min_dis_num];
					deb_para[4] = center_y[min_dis_num];
					deb_para[5] = ch_x[sp_num - sp_num_min];
					deb_para[6] = yy * pitch;
					deb_para[7] = ch_x[sp_num - sp_num_min];
					deb_para[8] = yy * pitch;
					callback(deb_para);
				}

				if (debug_logi_view != 0)
				{
					deb_para[0] = 8;
					deb_para[1] = dt;
					deb_para[2] = ch_x[sp_num - sp_num_min];
					deb_para[3] = yy * pitch;
					callback(deb_para);
				}
#endif
				spa->res_sum[min_dis_num] += dt;
				spa->res_count[min_dis_num]++;
			}
		}//y
	}//x

	//合計を求める
	for(ii = 0; ii < spa->num; ii++)
	{
		spa->res_total_sum += spa->res_sum[ii];
	}
	
	

	//結果判定
	if(spa->res_total_sum > spa->total_spa_thr)
	{
		spa->judge = SPA_OK;
	}
	else
	{
		spa->judge = SPA_NG;
	}

#ifdef VS_DEBUG
	//FILE *fp;
	//fp = fopen("spa.csv", "a");
	//
	//fclose(fp);
#endif
	

	return 0;
}

