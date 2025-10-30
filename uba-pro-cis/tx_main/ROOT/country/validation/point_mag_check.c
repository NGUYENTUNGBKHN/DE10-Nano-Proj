//#include "stdafx.h"		//モニター以外ではコメントアウト

#if MONITOR
	#include "point_mag_check.h"
#elif SIMURATION
	#include "jcm_typedef.h"
	#include "struct.h"
	#define EXT
	#include "../common/global.h"
	
	#include "../src/data_point_mag_check.h"
	#include "point_mag_check.h"
	
	u8 mag_limit_flag[DENOMI_SIZE];
	mag_area_limit ul_mag_limit[5][2][DENOMI_SIZE];
	mag_area_limit ur_mag_limit[5][2][DENOMI_SIZE];
#else
	#include <stdlib.h>
	#define EXT
	#include "../common/global.h"
	#include "com_ram.c"
	#include "cis_ram.c"
	#include "jsl_ram.c"
	
	#include "sensor.h"
	#include "../src/data_point_mag_check.h"
	#include "point_mag_check.h"
#endif

//extern ST_WORK work[100];						//モニター以外ではコメントアウト
//extern u8 mag_limit_flag[120];					//モニター以外ではコメントアウト
//extern mag_area_limit ur_mag_limit[5][2][120];	//モニター以外ではコメントアウト
//extern mag_area_limit ul_mag_limit[5][2][120];	//モニター以外ではコメントアウト
extern BV_MEMORY ex_validation;

void create_mag_wave(u8 buf_num, u8 way)
{
	ST_BS* pbs = work[0].pbs;
	u32 cnt = 0;
	u32 shift = 0;
	u32 start = 0;
	u32 end = 0;
	u8 plane = 0;
	u32 offset = 0;
	u32 period = 0;
	u8* p_Data;
	u32 line = 0;

	memset(ex_validation.ul_mag ,0, sizeof(ex_validation.ul_mag));
	memset(ex_validation.ur_mag ,0, sizeof(ex_validation.ur_mag));
	ex_validation.mag_enable[LEFT] = 1;
	ex_validation.mag_enable[RIGHT] = 1;

	shift = MAG_SAMPLING_DELAY + MAG_DATA_SHIFT;
	
	switch (way)
	{
	case W0:
	case W2:
		line = 0;
		plane = UP_MAG;
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			p_Data = pbs->sens_dt;
			start = ((pbs->right_up_y - pbs->left_up_y) / (pbs->right_up_x - pbs->left_up_x)
					 * (240 - pbs->left_up_x) + pbs->left_up_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			end = ((pbs->right_down_y - pbs->left_down_y) / (pbs->right_down_x - pbs->left_down_x)
				   * (240 - pbs->left_down_x) + pbs->left_down_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			offset = pbs->PlaneInfo[plane].Address_Offset + pbs->PlaneInfo[plane].main_effective_range_min;
			period = pbs->PlaneInfo[plane].Address_Period;

			//最初だけサンプリング自体されているかチェックする
			if (((u8)(*(p_Data + (offset - 1) + period * start)) == 0xFF)
				&& ((u8)(*(p_Data + (offset - 2) + period * start)) == 0xFF))
			{
				ex_validation.mag_enable[LEFT] = 0;
				ex_validation.mag_enable[RIGHT] = 0;
				return;
			}
			for (cnt = start + shift; cnt < end + shift; cnt++)
			{
				ex_validation.ul_mag[line] = *(p_Data + offset + period * cnt);
				line++;
			}
		}
		line = 0;
		plane = UP_MAG2;
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			p_Data = pbs->sens_dt;
			start = ((pbs->right_up_y - pbs->left_up_y) / (pbs->right_up_x - pbs->left_up_x)
					 * (480 - pbs->left_up_x) + pbs->left_up_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			end = ((pbs->right_down_y - pbs->left_down_y) / (pbs->right_down_x - pbs->left_down_x)
				   * (480 - pbs->left_down_x) + pbs->left_down_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			offset = pbs->PlaneInfo[plane].Address_Offset + pbs->PlaneInfo[plane].main_effective_range_min;
			period = pbs->PlaneInfo[plane].Address_Period;
			for (cnt = start + shift; cnt < end + shift; cnt++)
			{
				ex_validation.ur_mag[line] = *(p_Data + offset + period * cnt);
				line++;
			}
		}
		break;
	case W1:
	case W3:
		line = 0;
		plane = UP_MAG;
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			p_Data = pbs->sens_dt;
			start = ((pbs->right_up_y - pbs->left_up_y) / (pbs->right_up_x - pbs->left_up_x)
					 * (240 - pbs->left_up_x) + pbs->left_up_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			end = ((pbs->right_down_y - pbs->left_down_y) / (pbs->right_down_x - pbs->left_down_x)
				   * (240 - pbs->left_down_x) + pbs->left_down_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			offset = pbs->PlaneInfo[plane].Address_Offset + pbs->PlaneInfo[plane].main_effective_range_min;
			period = pbs->PlaneInfo[plane].Address_Period;

			//最初だけサンプリング自体されているかチェックする
			if (((u8)(*(p_Data + (offset - 1) + period * start)) == 0xFF)
				&& ((u8)(*(p_Data + (offset - 2) + period * start)) == 0xFF))
			{
				ex_validation.mag_enable[LEFT] = 0;
				ex_validation.mag_enable[RIGHT] = 0;
				return;
			}
			for (cnt = end + shift; cnt > start + shift; cnt--)
			{
				ex_validation.ur_mag[line] = *(p_Data + offset + period * cnt);
				line++;
			}
		}
		line = 0;
		plane = UP_MAG2;
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			p_Data = pbs->sens_dt;
			start = ((pbs->right_up_y - pbs->left_up_y) / (pbs->right_up_x - pbs->left_up_x)
					 * (480 - pbs->left_up_x) + pbs->left_up_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			end = ((pbs->right_down_y - pbs->left_down_y) / (pbs->right_down_x - pbs->left_down_x)
				   * (480 - pbs->left_down_x) + pbs->left_down_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			offset = pbs->PlaneInfo[plane].Address_Offset + pbs->PlaneInfo[plane].main_effective_range_min;
			period = pbs->PlaneInfo[plane].Address_Period;
			for (cnt = end + shift; cnt > start + shift; cnt--)
			{
				ex_validation.ul_mag[line] = *(p_Data + offset + period * cnt);
				line++;
			}
		}
		break;
	}
}

u16 mag_amount_calc(mag_area *pos, u8 *mag_ptr, u16 bill_len)
{
	u16 cnt;
	u16 end;
	u8 differ;
	u8 curr_val;
	u8 prev_val;
	u8 noize_cut;
	u16 amount;

	amount = 0;
	noize_cut = 1;
	end = pos->end;
	if (end > bill_len)
	{
		end = bill_len;
	}

	for (cnt = pos->start; cnt < end; cnt++)
	{
		differ = 0;
		curr_val = (*(u8 *)(mag_ptr + cnt));
		if ((cnt < (pos->start + MAG_CALC_NOIZE_CUT_POINT))
			&& (noize_cut == 1))
		{
			if (curr_val >= MAG_AD_RV)
			{
				/* if this mag value is less than MAG_AD_RV, it is not counted. */
				differ = (curr_val - MAG_AD_RV);
				noize_cut = 0;
			}
		}
		else if ((cnt == (pos->start + MAG_CALC_NOIZE_CUT_POINT))
				 && (noize_cut == 1))
		{
			if (curr_val >= MAG_AD_RV)
			{
				differ = (curr_val - MAG_AD_RV);
			}
			else
			{
				differ = (MAG_AD_RV - curr_val);
			}
		}
		else
		{
			prev_val = (*(u8 *)(mag_ptr + (cnt - 1)));
			if (curr_val >= prev_val)
			{
				differ = (curr_val - prev_val);
			}
			else
			{
				differ = (prev_val - curr_val);
			}
		}
		amount += differ;
	}

	return amount;
}

/******************************************************************************/
/*! @brief check mag amount count
@par            Refer
- 参照するグローバル変数 ex_validation.ul_mag_amount ur_mag_amount ul_mag_reult ur_mag_reult
@par            Modify
- 変更するグローバル変数 none
@return         ACCEPT:0, REJECT:~0
@exception      none
******************************************************************************/
u8 mag_amount_chk_invalid_count(u8 buf_num, u32 denomination, u32 way)
{
	ST_BS* pbs = work[buf_num].pbs;
	u8 table;
	mag_value *limit;
	u8 *ul_mag_ptr;
	u8 *ur_mag_ptr;
	u8 invalid_count = 0;
	u16 length = 0;
	u8 dir = UP;

	memset(ex_validation.ul_mag_amount ,0, sizeof(ex_validation.ul_mag_amount));
	memset(ex_validation.ur_mag_amount ,0, sizeof(ex_validation.ur_mag_amount));
	memset(ex_validation.ul_mag_reult ,0, sizeof(ex_validation.ul_mag_reult));
	memset(ex_validation.ur_mag_reult ,0, sizeof(ex_validation.ur_mag_reult));

	length = pbs->note_y_size;
	ul_mag_ptr = &ex_validation.ul_mag[0];
	ur_mag_ptr = &ex_validation.ur_mag[0];

	if (mag_limit_flag[denomination])
	{
		if ((pbs->PlaneInfo[UP_MAG].Enable_or_Disable)
			&& ((pbs->PlaneInfo[UP_MAG2].Enable_or_Disable)))
		{
			if ((ex_validation.mag_enable[LEFT])
				&& (ex_validation.mag_enable[RIGHT]))
			{
		switch (way)
		{
		case W0://UP
		case W1:
			dir = UP;
			break;
		case W2://DOWN
		case W3:
			dir = DOWN;
			break;
		default:
			break;
		}
		
		for (table = 0; table < 5; table++)
		{
					ex_validation.ul_mag_amount[table] = mag_amount_calc((mag_area*)&ul_mag_limit[table][dir][denomination].pos, ul_mag_ptr, length);
					ex_validation.ur_mag_amount[table] = mag_amount_calc((mag_area*)&ur_mag_limit[table][dir][denomination].pos, ur_mag_ptr, length);
		}

		for (table = 0; table < 5; table++)
		{
					limit = (mag_value*)&ul_mag_limit[table][dir][denomination].limit;
			if (ex_validation.ul_mag_amount[table] < limit->min)
				//if ((ex_validation.ul_mag_amount[table] < limit->min)
				// || (limit->max < ex_validation.ul_mag_amount[table]))
			{
				/*----- Up/Left MAG error ----*/
				invalid_count++;			/*reject bill[MAG ERROR]*/
				ex_validation.ul_mag_reult[table] = 1;
			}

					limit = (mag_value*)&ur_mag_limit[table][dir][denomination].limit;
			if (ex_validation.ur_mag_amount[table] < limit->min)
				//if ((ex_validation.ur_mag_amount[table] < limit->min)
				// || (limit->max < ex_validation.ur_mag_amount[table]))
			{
				/*----- Up/Right MAG error ----*/
				invalid_count++;			/*reject bill[MAG ERROR]*/
				ex_validation.ur_mag_reult[table] = 1;
			}
		}
			}
			else
			{//Sampling Error
				invalid_count = 0xFF;
			}
		}

#if !(SIMURATION || MONITOR)
		pbs->mid_res_mag.result = invalid_count;
#endif
	}
	return invalid_count;
}


#if 1 //2023-09-28
u8 create_mag_wave_test(u8 buf_num, u8 way)
{
	ST_BS* pbs = work[0].pbs;
	u32 cnt = 0;
	u32 shift = 0;
	u32 start = 0;
	u32 end = 0;
	u8 plane = 0;
	u32 offset = 0;
	u32 period = 0;
	u8* p_Data;
	u32 line = 0;

	u8 test_data[20];
	u8 aa;
	u32 mag_count=400;

	memset(ex_validation.ul_mag ,0, sizeof(ex_validation.ul_mag));
	memset(ex_validation.ur_mag ,0, sizeof(ex_validation.ur_mag));

	shift = MAG_SAMPLING_DELAY + MAG_DATA_SHIFT;
	
	switch (way)
	{
	case W0:
		line = 0;
		plane = UP_MAG;
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			p_Data = pbs->sens_dt;
			start = ((pbs->right_up_y - pbs->left_up_y) / (pbs->right_up_x - pbs->left_up_x)
					 * (240 - pbs->left_up_x) + pbs->left_up_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			end = ((pbs->right_down_y - pbs->left_down_y) / (pbs->right_down_x - pbs->left_down_x)
				   * (240 - pbs->left_down_x) + pbs->left_down_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			offset = pbs->PlaneInfo[plane].Address_Offset + pbs->PlaneInfo[plane].main_effective_range_min;
			period = pbs->PlaneInfo[plane].Address_Period;

			for (cnt = start + shift; cnt < 450 - shift - 1; cnt++)
			{
				ex_validation.ul_mag[line] = *(p_Data + offset + period * cnt);

				for (aa = 0; aa < 20; aa++)
				{
					test_data[aa] = *(p_Data + offset  + period * cnt - 10 + aa );
				}

				if(
					test_data[5] == 0xFF
					&&
					test_data[6] == 0xFF
					&&
					test_data[7] == 0xFF
					&&
					test_data[8] == 0xFF						
					)
				{
					break;
				}

				line++;
			}
		}

		mag_count = line - 1;					

		for (cnt = 0; cnt < mag_count; cnt++)
		{
			//新規追加 データの確認
			if( ex_validation.ul_mag[cnt] <= 125 || ex_validation.ul_mag[cnt] >= 145 )
			{
				/* Error*/
				return 0;
			}
		}
		

		line = 0;
		plane = UP_MAG2;
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			p_Data = pbs->sens_dt;
			start = ((pbs->right_up_y - pbs->left_up_y) / (pbs->right_up_x - pbs->left_up_x)
					 * (480 - pbs->left_up_x) + pbs->left_up_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			end = ((pbs->right_down_y - pbs->left_down_y) / (pbs->right_down_x - pbs->left_down_x)
				   * (480 - pbs->left_down_x) + pbs->left_down_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
			offset = pbs->PlaneInfo[plane].Address_Offset + pbs->PlaneInfo[plane].main_effective_range_min;
			period = pbs->PlaneInfo[plane].Address_Period;
			
			for (cnt = start + shift; cnt < 450 - shift - 1; cnt++)
			{
				ex_validation.ur_mag[line] = *(p_Data + offset + period * cnt);
				//新規追加 データの確認

				for (aa = 0; aa < 20; aa++)
				{
					test_data[aa] = *(p_Data + offset  + period * cnt - 10 + aa );
				}

				if(
					test_data[5] == 0xFF
					&&
					test_data[6] == 0xFF
					&&
					test_data[7] == 0xFF
					&&
					test_data[8] == 0xFF						
					)
				{
					break;
				}

				line++;
			}
		}

		mag_count = line - 1;					

		for (cnt = 0; cnt < mag_count; cnt++)
		{
			//新規追加 データの確認
			if( ex_validation.ur_mag[cnt] <= 125 || ex_validation.ur_mag[cnt] >= 145 )
			{
				/* Error*/
				return 0;
			}
		}
		break;
	}

	
	return 1;

}
#endif
