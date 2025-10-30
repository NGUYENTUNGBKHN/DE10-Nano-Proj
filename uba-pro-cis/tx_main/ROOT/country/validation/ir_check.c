#define EXT
#include "../common/global.h"
#include "ir_check.h"


int get_ir_check_invalid_count(u8 buf_num ,ST_IR_CHECK *ir_check)
{

	//u8 base_i = 0;			//パラ:ベースデータ作成for分用

	//u32 color_sum = 0;		//パラ：合計
	u32 val;				//パラ：ポイント値
	u16 res;				//パラ：計算結果
	u16 point_cnt = 0;		//パラ：カレントポイント
	u8 plane=0;				//パラ：プレーン番号
	u32 ir_base_value;		//パラ：ベースデータ

	u16 mode_cnt = 0;			//パラ：モード
	u16 mode = 0;			//パラ：モード

	u16 invalid_count = 0;	//出力：無効カウント

	for(mode_cnt = 0; mode_cnt < IR_CHECK_COUNT; ++mode_cnt)
	{
		//モードが-1なら終わり
		if(ir_check->cal_mode[mode_cnt] == -1)
		{
			break;
		}

		if(mode_cnt > 0)
		{
			//アドレスを更新する
			point_cnt++;
			ir_check->pmode = (ST_IR_MODES *)(&ir_check->ppoint[point_cnt]);
			ir_check->ppoint = (ST_LIM_AND_POINTS *)((u8*)ir_check->pmode + sizeof(ST_IR_MODES));
		}

		mode = ir_check->cal_mode[mode_cnt];
		plane = ir_plane_tbl[mode];
		ir_base_value = 0;
		point_cnt = 0;

		//ベースデータ採取
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x1,ir_check->pmode->base_point_y1 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x2,ir_check->pmode->base_point_y2 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x3,ir_check->pmode->base_point_y3 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x4,ir_check->pmode->base_point_y4 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);

		if (ir_base_value == 0)
		{
			ir_base_value = 1;
		}

		//計算する　0,0,0,0のエンドマークが来るまで繰り返す
		while(!(ir_check->ppoint[point_cnt].x == -1 && ir_check->ppoint[point_cnt].y == -1 &&
			ir_check->ppoint[point_cnt].limit_min == -1 && ir_check->ppoint[point_cnt].limit_max == -1))
		{
			//ポイントデータ採取
			val = get_mesh_data(ir_check->ppoint[point_cnt].x, ir_check->ppoint[point_cnt].y, plane, ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
			//計算
			res = (u16)(val * CIR_RATIO_MULTIPLIER / ir_base_value);

			//閾値比較
			if (res < ir_check->ppoint[point_cnt].limit_min - NOMAL_THERSHOLD || res > ir_check->ppoint[point_cnt].limit_max + NOMAL_THERSHOLD)
			{
				//無効ポイントカウント
				invalid_count++;
			}

			//次のポイントへ
			point_cnt++;

		}//while

	}//for

	return invalid_count;
}

