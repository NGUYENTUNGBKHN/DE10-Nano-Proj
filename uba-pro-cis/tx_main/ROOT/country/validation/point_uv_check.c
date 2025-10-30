//#include "stdafx.h"		//モニター以外ではコメントアウト

#if MONITOR
	#include "point_uv_check.h"
#elif SIMURATION
	#include "jcm_typedef.h"
	#include "struct.h"

	#define EXT
	#include "../common/global.h"
	#include "../src/data_point_uv_check.h"

	ST_POINT_UV_CHECK POINT_UV_LIMIT_MAX[DENOMI_SIZE];
#else
	#include <stdlib.h>
	#define EXT

	#include "../common/global.h"
	#include "../src/data_point_uv_check.h"
#endif

//extern ST_WORK work[100];							//モニター以外ではコメントアウト
//extern ST_POINT_UV_CHECK POINT_UV_LIMIT_MAX[120];	//モニター以外ではコメントアウト
extern BV_MEMORY ex_validation;

void create_uv_wave(u8 buf_num, u8 way)
{
	ST_BS* pbs = work[buf_num].pbs;
	u32 total = 0;
	u8 plane,plane2;
	u32 l = 0;
	u8* p_Data;
	u32 period = 0;
	u32	offset = 0;
	u16 start = 0;
	u16 end = 0;
	u16 length = 0;
	u32 retry = 0;

	memset(&ex_validation.uv_wave, 0x00, sizeof(ex_validation.uv_wave));
	
	plane = UP_R_FL;
	plane2 = DOWN_R_FL;

	start = ((pbs->right_up_y - pbs->left_up_y) / (pbs->right_up_x - pbs->left_up_x)
			 * (360 - pbs->left_up_x) + pbs->left_up_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;
	end = ((pbs->right_down_y - pbs->left_down_y) / (pbs->right_down_x - pbs->left_down_x)
		   * (360 - pbs->left_down_x) + pbs->left_down_y) / pbs->PlaneInfo[plane].sub_sampling_pitch;

	if((end <= start) || (end < UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch)))
	{
		return;
	}
	length = end - start;

	p_Data = (unsigned char*)pbs->sens_dt;
	
	switch (way)
	{
	case W0:	//A
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane].Address_Offset + 8;
			period = pbs->PlaneInfo[plane].Address_Period;
			for (int i = start; i < end; i++)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
				{
					if (!retry--)
					{
						return;
					}
					continue;
				}
				ex_validation.uv_wave.uv_wave[UP][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[UP] = pbs->PlaneInfo[plane].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[UP] = length;
		}
		if (pbs->PlaneInfo[plane2].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane2].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane2].Address_Offset + 8;
			period = pbs->PlaneInfo[plane2].Address_Period;
			for (int i = start; i < end; i++)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
				{
					l++;
					if (!retry--)
					{
						return;
					}
					continue;
				}
				ex_validation.uv_wave.uv_wave[DOWN][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[DOWN] = pbs->PlaneInfo[plane2].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[DOWN] = length;
		}
		break;
	case W1:	//B
		if (!pbs->PlaneInfo[plane2].Enable_or_Disable)
		{
			start += UV_SHIFT_LNGTH;
			end += UV_SHIFT_LNGTH;
		}
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane].Address_Offset + 8;
			period = pbs->PlaneInfo[plane].Address_Period;
			for (int i = end; i > start; i--)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
				{
					l++;
					if (!retry--)
					{
						return;
					}
					continue;
				}
				ex_validation.uv_wave.uv_wave[UP][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[UP] = pbs->PlaneInfo[plane].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[UP] = length;
		}
		if (pbs->PlaneInfo[plane2].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane2].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane2].Address_Offset + 8;
			period = pbs->PlaneInfo[plane2].Address_Period;
			for (int i = end; i > start; i--)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
				{
					l++;
					if (!retry--)
					{
						return;
					}
					continue;
				}
				ex_validation.uv_wave.uv_wave[DOWN][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[DOWN] = pbs->PlaneInfo[plane2].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[DOWN] = length;
		}
		break;
	case W2:	//C
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane].Address_Offset + 8;
			period = pbs->PlaneInfo[plane].Address_Period;
			for (int i = start; i < end; i++)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
		{
					l++;
					if (!retry--)
			{
						return;
			}
					continue;
		}
				ex_validation.uv_wave.uv_wave[DOWN][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[DOWN] = pbs->PlaneInfo[plane].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[DOWN] = length;
		}
		if (pbs->PlaneInfo[plane2].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane2].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane2].Address_Offset + 8;
			period = pbs->PlaneInfo[plane2].Address_Period;
			for (int i = start; i < end; i++)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
				{
					l++;
					if (!retry--)
					{
						return;
					}
					continue;
				}
				ex_validation.uv_wave.uv_wave[UP][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[UP] = pbs->PlaneInfo[plane2].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[UP] = length;
		}
		break;
	case W3:	//D
		if (!pbs->PlaneInfo[plane2].Enable_or_Disable)
		{
			start += UV_SHIFT_LNGTH;
			end += UV_SHIFT_LNGTH;
		}
		if (pbs->PlaneInfo[plane].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane].Address_Offset + 8;
			period = pbs->PlaneInfo[plane].Address_Period;
			for (int i = end; i > start; i--)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
				{
					l++;
					if (!retry--)
					{
						return;
					}
					continue;
				}
				ex_validation.uv_wave.uv_wave[DOWN][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[DOWN] = pbs->PlaneInfo[plane].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[DOWN] = length;
		}
		if (pbs->PlaneInfo[plane2].Enable_or_Disable)
		{
			l = 0;
			retry = (u32)UV_MINIMUM_LNGTH(work[buf_num].pbs->PlaneInfo[plane2].sub_sampling_pitch);
			offset = pbs->PlaneInfo[plane2].Address_Offset + 8;
			period = pbs->PlaneInfo[plane2].Address_Period;
			for (int i = end; i > start; i--)
			{
				if (((u8)(*(p_Data + (offset - 1) + period * i)) == 0xFF)
					&& ((u8)(*(p_Data + (offset - 2) + period * i)) == 0xFF))
				{
					l++;
					if (!retry--)
					{
						return;
					}
					continue;
				}
				ex_validation.uv_wave.uv_wave[UP][l] = (u8)(*(p_Data + offset + period * i));
				l++;
			}
			ex_validation.uv_wave.enable[UP] = pbs->PlaneInfo[plane2].Enable_or_Disable;
			ex_validation.uv_wave.wave_length[UP] = length;
		}
		break;
	}
}


u8 get_mount_data_point_uv(u16 start, u16 end, u8* uv_ptr, u8 division)
{
	u16 i = 0;
	u32 total;
	u8 average[3];
	u16 length = (end-start)/3;

	memset(average, 0, sizeof(average));

	switch(division)
	{
	case 0:
		total = 0;
		for(i = 0; i < length; i++)
		{
			total += (u8)(uv_ptr[start + i]);
		}
		//average
		average[0] = (u8)(total/length);
		break;
	case 1:
		total = 0;
		for(i = 0; i < length; i++)
		{
			total += (u8)(uv_ptr[start + length + i]);
		}
		//average
		average[1] = (u8)(total/length);
		break;
	case 2:
		total = 0;
		for(i = 0; i < length; i++)
		{
			total += (u8)(uv_ptr[start + length + length + i]);
		}
		//average
		average[2] = (u8)(total/length);
		break;
	default:
		break;
	}

	return average[division];
}

u8 get_point_uv_invalid_count(u8 buf_num, u32 denomination, u32 way)
{
	ST_POINT_UV_CHECK *point_uv = (ST_POINT_UV_CHECK *)&POINT_UV_LIMIT_MAX[denomination];
	
	ST_BS* pbs = work[buf_num].pbs;
	u8 *uv_ptr;
	u8 plane;
	u16 start = 0;
	u16 end = 0;
	u32 invalid_count = 0;
	u8 division = 0;
	u8 average = 0;
	
	plane = UP_R_FL;
	if(pbs->PlaneInfo[plane].Enable_or_Disable)
	{
		switch (way)
		{
		case W0:
		case W1:
			start = (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			end = ex_validation.uv_wave.wave_length[UP] - (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch)*2;

			uv_ptr = ex_validation.uv_wave.uv_wave[UP];
			if (ex_validation.uv_wave.enable[UP])
			{
			for (division = 0; division < 3; division++)
			{
				average = get_mount_data_point_uv(start, end, uv_ptr, division);
				if (point_uv->o_max_uv[division] <= average)
				{
					invalid_count++;
				}
			}
			}
			else
			{
				invalid_count = 0xFF;
			}
			break;
		case W2:
		case W3:
			start = (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			end = ex_validation.uv_wave.wave_length[DOWN] - (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch)*2;

			uv_ptr = ex_validation.uv_wave.uv_wave[DOWN];
			if (ex_validation.uv_wave.enable[DOWN])
			{
			for (division = 0; division < 3; division++)
			{
				average = get_mount_data_point_uv(start, end, uv_ptr, division);
				if (point_uv->i_max_uv[division] <= average)
				{
					invalid_count++;
				}
			}
			}
			else
			{
				invalid_count = 0xFF;
			}
			break;
		}
	}

	plane = DOWN_R_FL;
	if(pbs->PlaneInfo[plane].Enable_or_Disable)
	{
		switch(way)
		{
		case W0:
		case W1:
			start = (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			end = ex_validation.uv_wave.wave_length[DOWN] - (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch)*2;

			uv_ptr = ex_validation.uv_wave.uv_wave[DOWN];
			if (ex_validation.uv_wave.enable[DOWN])
			{
			for (division = 0; division < 3; division++)
			{
				average = get_mount_data_point_uv(start, end, uv_ptr, division);
				if (point_uv->i_max_uv[division] <= average)
				{
					invalid_count++;
				}
			}
			}
			else
			{
				invalid_count = 0xFF;
			}
			break;
		case W2:
		case W3:
			start = (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch);
			end = ex_validation.uv_wave.wave_length[UP] - (u16)UV_MISSING_DATA(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch)*2;

			uv_ptr = ex_validation.uv_wave.uv_wave[UP];
			if (ex_validation.uv_wave.enable[UP])
			{
			for (division = 0; division < 3; division++)
			{
				average = get_mount_data_point_uv(start, end, uv_ptr, division);
				if (point_uv->o_max_uv[division] <= average)
				{
					invalid_count++;
				}
			}
			}
			else
			{
				invalid_count = 0xFF;
			}
			break;
		}
	}

#if SIMURATION | MONITOR
	if (invalid_count > 6)
	{//Sampling Error
		return 0xFF;
	}
	else if(invalid_count < point_uv->invalid_count)
	{//ST-note
		return 0;
	}
	else
	{//CP-note
		return 1;
	}
#else
	/* 判定結果代入 */
	pbs->mid_res_uv_validate.uf_thr = (u8)point_uv->invalid_count;
	pbs->mid_res_uv_validate.level_result = (u8)invalid_count;

	if (invalid_count > 6)
	{//Sampling Error
		return 0xFF;
	}
	else if(invalid_count < point_uv->invalid_count)
	{//ST-note
		pbs->mid_res_uv_validate.result = 0;
		return 0;
	}
	else
	{//CP-note
		pbs->mid_res_uv_validate.result = 1;
		return 1;
	}
#endif
}
