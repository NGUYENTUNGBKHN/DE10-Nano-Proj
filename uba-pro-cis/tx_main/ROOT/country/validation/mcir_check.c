
#include <stdio.h>

#define EXT
#define DEBUG_RBA

#include "../common/global.h"
#include "convert_data.h"
 
#include "mcir_check.h"
#include <stdlib.h>
//demomiと方向をNNの出力ノードと対応させた方が良い？
int get_mcir_invalid_count(u8 buf_num ,ST_MCIR *mcir)
{

	u32 color_sum = 0;		//パラ：三色の合計
	s32 val1;				//パラ：ポイント値
	s32 val2;	
	s32 val3;
	s32 res;				//パラ：計算結果
	u16 point_cnt = 0;		//パラ：カレントポイント
	u8 plane1=0;			//パラ：プレーン番号
	u8 plane2=0;
	u8 plane3=0;
	u16 mode_cnt = 0;			//パラ：モード
	u16 mode = 0;			//パラ：モード
	u16 plane_side;			//パラ：プレーンのサイド
	u16 invalid_count = 0;	//出力：無効カウント
	s32 level = 100; // 算出レベル格納

	for(mode_cnt = 0; mode_cnt < CIR_3COLOR_CHECK_COUNT; ++mode_cnt)
	{
		//モードが-1なら終わり
		if(mcir->cal_mode[mode_cnt] == -1)
		{
			break;
		}

		//モードの設定
		mode = mcir->cal_mode[mode_cnt];

		plane_side = mode % 2;

		//モードに応じてテーブルからプレーンを取得
		//偶数が表　奇数が裏
		plane1 = mcir_plane_tbl_ir1[plane_side];
		plane2 = mcir_plane_tbl_red[plane_side];
		plane3 = mcir_plane_tbl_grn[plane_side];

		//計算する　0,0,0,0のエンドマークが来るまで繰り返す
		while(!(mcir->ppoint[point_cnt].x == -1 && mcir->ppoint[point_cnt].y == -1 &&
			mcir->ppoint[point_cnt].limit_min == -1 && mcir->ppoint[point_cnt].limit_max == -1))
		{
			//ポイントデータ採取
			val1 = get_mesh_data(mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, plane1 ,mcir->ir1_mask_ptn_diameter_x ,mcir->ir1_mask_ptn_diameter_y ,mcir->pir1_mask_ptn ,buf_num ,mcir->ir1_mask_ptn_divide_num);
			val2 = get_mesh_data(mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, plane2 ,mcir->red_mask_ptn_diameter_x ,mcir->red_mask_ptn_diameter_y ,mcir->pred_mask_ptn ,buf_num ,mcir->red_mask_ptn_divide_num);
			val3 = get_mesh_data(mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, plane3 ,mcir->grn_mask_ptn_diameter_x ,mcir->grn_mask_ptn_diameter_y ,mcir->pgrn_mask_ptn ,buf_num ,mcir->grn_mask_ptn_divide_num);

			//計算
			color_sum = val1 + val2 + val3;

			if (color_sum < 1) 
			{
				color_sum = 1;
			}
			//fprintf(test, "%d, %d,", mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y);

			if(mode == 0 || mode == 1)
			{
				res = (s32)(abs(val1 * 2 - val2) * MCIR_RATIO_MULTIPLIER / color_sum);
			}
			else if(mode == 2 || mode == 3)
			{
				res = (s32)(abs(val1 * 2 - val3) * MCIR_RATIO_MULTIPLIER / color_sum);
			}
			else
			{
				res = (s32)(abs(val3 * 2 - val2) * MCIR_RATIO_MULTIPLIER / color_sum);	//式間違えていた　190920　furuta 
			}


			//閾値比較
			if (res < mcir->ppoint[point_cnt].limit_min - NOMAL_THERSHOLD || res > mcir->ppoint[point_cnt].limit_max + NOMAL_THERSHOLD) 
			{
				//無効ポイントカウント
				invalid_count++;
			}
			// メッシュポイント確認用
			//fprintf(test, "%d, %d, %d, %d, %d, %d, %d,\n", mode, mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, val1, val2, val3, invalid_count);
			//次のポイントへ
			point_cnt++;

		}//while

		//次のポイントへ
		point_cnt++;

	}//for
	
	level = MCIR_STANDARD_LEVEL - (MCIR_CONSTANT * invalid_count);

	if (level > 100)
	{
		level = 100;
	}
	else if (level <= 0)
	{
		level = 1;
	}
	mcir->level = (u8)level;
	return invalid_count;
}
