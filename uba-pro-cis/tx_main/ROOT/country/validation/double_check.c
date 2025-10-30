#define EXT
#include "../common/global.h"
#include "double_check.h"

int get_double_check_error(u8 buf_num ,ST_DOUBLE_CHECK* double_check)
{
	s16 x = 0;
	s16 y = 0;
	s16 x_boudary;
	s16 y_boudary;

#ifndef NEW_DOUBLE_CHECK
	int total_value = 0;
	int ReflectiveUpValue = 0;
	int ReflectiveDownValue = 0;
	float average = 0.0;
#endif

	int TransparentValue = 0;
	int invalid_count = 0;
	float percent = 0.0;
	int level = 0;
	u8 trans_plane = OMOTE_T_R;
	int total_count = 0;
#ifdef NEW_DOUBLE_CHECK
	
	trans_plane = OMOTE_T_IR1;
	if(work[buf_num].pbs->PlaneInfo[trans_plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効なら
	{
		trans_plane = URA_T_IR1;
	}

	x_boudary = (s16)((((double_check->note_size_x - 1) * 0.5f) - double_check->red_mask_ptn_diameter_x * 0.5f) / double_check->red_mask_ptn_diameter_x);
	y_boudary = (s16)(((((double_check->note_size_y - 1) * 0.5f) - double_check->red_mask_ptn_diameter_x * 0.5f) / double_check->red_mask_ptn_diameter_x) * 0.5f);

	for (y = -y_boudary; y <= y_boudary; y += y_boudary)
	{
		for (x = -x_boudary + 1; x < x_boudary; x++) 
		{
			// 2020.09.08 ichijo 学習時に標準偏差を計算済みなのでそのしきい値を超ええるかどうかだけを見る。超えない場合、重券カウンタを上げる
			TransparentValue = get_mesh_data(x, y, trans_plane ,     double_check->tred_mask_ptn_diameter_x ,double_check->tred_mask_ptn_diameter_y ,double_check->ptred_mask_ptn ,buf_num ,double_check->tred_mask_ptn_divide_num);

			if (TransparentValue < double_check->threshold)
			{
				invalid_count++;
			}
			total_count++;
		}
	}
	
	percent = ((float)invalid_count / (float)total_count * (float)100);
	
	level = DOUBLE_LEVEL - (int)percent;
	if (level <= 0)
	{
		level = 1;
	}
	else if (level > 100)
	{
		level = 100;
	}

	double_check->level = (u8)level;

	if (DOUBLE_UNFIT < level)
	{
		return 0; // 正常
	}
	else
	{
		return 1; // 異常検知
	}
#else
	
	if(work[buf_num].pbs->PlaneInfo[trans_plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効なら
	{
		trans_plane = URA_T_R;
	}

	x_boudary = (s16)((((double_check->note_size_x - 1) * 0.5f) - double_check->red_mask_ptn_diameter_x * 0.5f) / double_check->red_mask_ptn_diameter_x);
	y_boudary = (s16)(((((double_check->note_size_y - 1) * 0.5f) - double_check->red_mask_ptn_diameter_x * 0.5f) / double_check->red_mask_ptn_diameter_x) * 0.5f);

	for (y = -y_boudary; y <= y_boudary; y += y_boudary)
	{
		for (x = -x_boudary + 1; x < x_boudary; x++) 
		{
			ReflectiveUpValue = get_mesh_data(x, y, OMOTE_R_R ,    double_check->red_mask_ptn_diameter_x ,double_check->red_mask_ptn_diameter_y ,double_check->pred_mask_ptn ,buf_num ,double_check->red_mask_ptn_divide_num);
			ReflectiveDownValue = get_mesh_data(x, y, URA_R_R ,double_check->red_mask_ptn_diameter_x ,double_check->red_mask_ptn_diameter_y ,double_check->pred_mask_ptn ,buf_num ,double_check->red_mask_ptn_divide_num);
			TransparentValue = get_mesh_data(x, y, trans_plane ,     double_check->tred_mask_ptn_diameter_x ,double_check->tred_mask_ptn_diameter_y ,double_check->ptred_mask_ptn ,buf_num ,double_check->tred_mask_ptn_divide_num);

			if (ReflectiveUpValue < 1) 
			{
				ReflectiveUpValue = 1;
			}
			if (ReflectiveDownValue < 1) 
			{
				ReflectiveDownValue = 1;
			}
			if (TransparentValue < 1) 
			{
				TransparentValue = 1;
			}
			total_value += (TransparentValue * 4 - ReflectiveUpValue - ReflectiveDownValue + 300) / 2;
			total_count++;
		}
	}

	if (total_count == 0)
	{
		total_count = 1;
	}

	total_value /= total_count;

	return double_check->threshold - (total_value + DOUBLE_CHECK_THERSHOLD); //式間違えていた　190920　furuta 
#endif
}
