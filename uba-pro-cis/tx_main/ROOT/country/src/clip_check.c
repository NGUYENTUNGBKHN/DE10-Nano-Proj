/*
 * clip_check.c
 *
 *  Created on: 2023/2/28
 *      Author: suzuki-hiroyuki
 */

#include "kernel.h"
#include "country_custom.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "cis_ram.c"

#define CISB_AREA_MAX			(695+8)		// CISB有効範囲　大(数値指定703をシンボルに変更, 20/04/24)
#define CISB_AREA_MIN			(26+8)
#define CLIP_DETECT_UNUSE_AREA 40
#define CLIP_DETECT_VALUE_LIMIT 64
#define CLIP_DETECT_POINT_LIMIT 2	/* 1 ~ 3 */
#define CLIP_DETECT_LINE_LIMIT 2

int is_clip_detected(void)
{
#if CLIP_CHECK_ENABLE
	ST_BS *pbs = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
	ST_SPOINT spt;

	int x_start;
	int x_end;
	int y_start;
	int y_end;
	int x_point;
	int y_point;
	int x_diff;
	int y_diff;
	int sub_sampling_pitch;
	int line_detect = 0;
	int point_detect = 0;

	sub_sampling_pitch = pbs->PlaneInfo[DOWN_T_G].sub_sampling_pitch;
	spt.p_plane_tbl = DOWN_T_G;
	spt.p = 0;

	/*
	left side check
	sensor : CISB_T_G
	x_min:CISB_AREA_MIN			(26+8)
	start x	:	left_up_x - 5.0mm (200dpi)
	start y	:	left_up_y / 2 (100dpi)
	end x:	:	left_down_x - 5.0mm (200dpi)
	end y:	:	left_down_y / 2 (100dpi)
	*/
	x_start = pbs->left_up_x - CLIP_DETECT_UNUSE_AREA;
	x_end = pbs->left_down_x - CLIP_DETECT_UNUSE_AREA;
	y_start = pbs->left_up_y / 2;
	y_end = pbs->left_down_y / 2;

	x_diff = x_end - x_start;
	y_diff = y_end - y_start;

	if((y_start < 0)
	|| (y_start > y_end)
	|| (y_end > BLOCK_MAX * SUB_BLOCK_NUM)
	){
		return false;
	}

	for(y_point = y_start; y_point < y_end; y_point++)
	{
		point_detect  = 0;
		x_point = x_start + y_point * x_diff / y_diff;

		if(x_point > CISB_AREA_MIN)
		{
			spt.x = x_point;
			spt.y = y_point;
			P_Getdt_8bit_only(&spt,pbs);
			if(CLIP_DETECT_VALUE_LIMIT > spt.sensdt)
			{
				point_detect++;
				//-1
				spt.x = x_point - 1;
				spt.y = y_point;
				P_Getdt_8bit_only(&spt,pbs);
				if(CLIP_DETECT_VALUE_LIMIT > spt.sensdt)
				{
					point_detect++;
				}
				//+1
				spt.x = x_point + 1;
				spt.y = y_point;
				P_Getdt_8bit_only(&spt,pbs);
				if(CLIP_DETECT_VALUE_LIMIT > spt.sensdt)
				{
					point_detect++;
				}
				if(point_detect >= CLIP_DETECT_POINT_LIMIT)
				{
					line_detect ++;
				}
			}
		}
	}
	if(line_detect >= CLIP_DETECT_LINE_LIMIT)
	{
		return true;
	}
	line_detect = 0;

	/*
	right side check
	sensor : CISB_T_G
	x_max:CISB_AREA_MAX			(695+8)
	start x	:	right_up_x + 5.0mm (200dpi)
	start y	:	right_up_y / 2 (100dpi)
	end x:	:	right_down_x + 5.0mm (200dpi)
	end y:	:	right_down_y / 2 (100dpi)
	*/
	x_start = pbs->right_up_x + CLIP_DETECT_UNUSE_AREA;
	x_end = pbs->right_down_x + CLIP_DETECT_UNUSE_AREA;
	y_start = pbs->right_up_y / 2;
	y_end = pbs->right_down_y / 2;

	x_diff = x_end - x_start;
	y_diff = y_end - y_start;

	if((y_start < 0)
	|| (y_start > y_end)
	|| (y_end > BLOCK_MAX * SUB_BLOCK_NUM)
	){
		return false;
	}

	for(y_point = y_start; y_point < y_end; y_point++)
	{
		point_detect  = 0;
		x_point = x_start + y_point * x_diff / y_diff;

		if(x_point < CISB_AREA_MAX)
		{
			spt.x = x_point;
			spt.y = y_point;
			P_Getdt_8bit_only(&spt,pbs);
			if(CLIP_DETECT_VALUE_LIMIT > spt.sensdt)
			{
				point_detect++;
				//-1
				spt.x = x_point - 1;
				spt.y = y_point;
				P_Getdt_8bit_only(&spt,pbs);
				if(CLIP_DETECT_VALUE_LIMIT > spt.sensdt)
				{
					point_detect++;
				}
				//+1
				spt.x = x_point + 1;
				spt.y = y_point;
				P_Getdt_8bit_only(&spt,pbs);
				if(CLIP_DETECT_VALUE_LIMIT > spt.sensdt)
				{
					point_detect++;
				}
				if(point_detect >= CLIP_DETECT_POINT_LIMIT)
				{
					line_detect ++;
				}
			}
		}
	}
	if(line_detect >= CLIP_DETECT_LINE_LIMIT)
	{
		return true;
	}
#endif

	return false;
}


/* EOF */
