/******************************************************************************/
/*! @addtogroup Group1
    @file       tem_main_barcode.c
    @brief      main barcode validation processing
    @date       2018/02/26
    @author     T.Yokoyama
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "custom.h"
#include "bv_errcode.h"
#include <string.h>

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "cis_ram.c"
#include "tem_global.c"

//1D barcodes
#if (defined ENABLE_TITO_TICKET)
	#include "tem_1d_barcode.h"
	#include "tem_itf_barcode.h"
#endif

//2D barcodes
#include "tem_polynomial.h"
#include "tem_pixel.h"
#include "tem_mesh.h"
#if (defined ENABLE_QR_TICKET)
	#include "tem_rs256_correction.h"
	#include "tem_2d_barcode.h"
	#include "tem_qr_barcode.h"
#endif

const float barcode_1d_search_offsets_mm[BARCODE_1D_OFFSET_COUNT] =
{
	0.0,
	1.2,
	-1.2,
	2.4,
	-2.4,
	3.6,
	-3.6,
	4.8,
	-4.8,
};


void set_1d_barcode_center_points_horizontal(const int sensor)
{
	int search_count = 0;

	u32 is_down;

	bill_info->barcode_search_point_count = 0;
	switch(sensor)
	{
	case CIS_RED_UP:
		is_down = 0;
		break;
	case CIS_RED_DOWN:
		is_down = 1;
		break;
	case CIS_GREEN_UP:
		is_down = 0;
		break;
	case CIS_GREEN_DOWN:
		is_down = 1;
		break;
	};
	if(is_down)
	{
		for(search_count = 0; search_count < BARCODE_1D_OFFSET_COUNT; search_count++)
		{
			bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = bill_info->sensor_info[sensor].deskew_center_x;
			bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = (bill_info->sensor_info[sensor].y_pixels - 1 - bill_info->sensor_info[sensor].deskew_center_y)
				+ (int)(barcode_1d_search_offsets_mm[search_count] / bill_info->sensor_info[sensor].pitch);

			bill_info->barcode_search_point_count++;
		}
	}
	else
	{
		for(search_count = 0; search_count < BARCODE_1D_OFFSET_COUNT; search_count++)
		{
			bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = bill_info->sensor_info[sensor].deskew_center_x;
			bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = bill_info->sensor_info[sensor].deskew_center_y
				+ (int)(barcode_1d_search_offsets_mm[search_count] / bill_info->sensor_info[sensor].pitch);

			bill_info->barcode_search_point_count++;
		}
	}
}


void set_1d_barcode_center_points_vertical(const int sensor)
{
	int search_count = 0;

	bill_info->barcode_search_point_count = 0;

	for(search_count = 0; search_count < BARCODE_1D_OFFSET_COUNT; search_count++)
	{
		bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = bill_info->sensor_info[sensor].x_pixels / 2
			+ (int)(barcode_1d_search_offsets_mm[search_count] / bill_info->sensor_info[sensor].pitch)
			+ (int)(45.0 / bill_info->sensor_info[sensor].pitch);
		bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = bill_info->sensor_info[sensor].y_pixels / 2;

		bill_info->barcode_search_point_count++;
	}
}


void set_2d_barcode_search_points_middle(const int sensor)
{
	#define BARCODE_2D_SEARCH_X_MM			(float)96.0
	#define BARCODE_2D_SEARCH_Y_MM			(float)45.0

	#define BARCODE_2D_SEARCH_PITCH_MM		(float)2.5

	int start_pixel_x;
	int start_pixel_y;
	u32 is_down;
	switch(sensor)
	{
	case CIS_RED_UP:
		is_down = 0;
		break;
	case CIS_RED_DOWN:
		is_down = 1;
		break;
	case CIS_GREEN_UP:
		is_down = 0;
		break;
	case CIS_GREEN_DOWN:
		is_down = 1;
		break;
	};
	start_pixel_x = bill_info->sensor_info[sensor].deskew_center_x - (int)(BARCODE_2D_SEARCH_X_MM / bill_info->sensor_info[sensor].pitch / 2.0);
	if(is_down)
	{
		start_pixel_y = (bill_info->sensor_info[sensor].y_pixels - 1 - bill_info->sensor_info[sensor].deskew_center_y) - (int)(BARCODE_2D_SEARCH_Y_MM / bill_info->sensor_info[sensor].pitch / 2.0);
	}
	else
	{
		start_pixel_y = bill_info->sensor_info[sensor].deskew_center_y - (int)(BARCODE_2D_SEARCH_Y_MM / bill_info->sensor_info[sensor].pitch / 2.0);
	}

	const int width_lines = (int)(BARCODE_2D_SEARCH_X_MM /  BARCODE_2D_SEARCH_PITCH_MM) + 1;
	const int height_lines = (int)(BARCODE_2D_SEARCH_Y_MM /  BARCODE_2D_SEARCH_PITCH_MM) + 1;
	const int pixel_interval = (int)(BARCODE_2D_SEARCH_PITCH_MM / bill_info->sensor_info[sensor].pitch);

	int search_count_x = 0;
	int search_count_y = 0;

	bill_info->barcode_search_point_count = 0;

	for(search_count_x = 0; search_count_x < width_lines / 2; search_count_x++)
	{
		for(search_count_y = 0; search_count_y < height_lines; search_count_y++)
		{
			if((search_count_x + search_count_y) % 2 == 0) //spread out search points
			{
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = start_pixel_x + search_count_x * pixel_interval;
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = start_pixel_y + search_count_y * pixel_interval;

				bill_info->barcode_search_point_count++;
			}
			else
			{
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = start_pixel_x + (width_lines - search_count_x - 1) * pixel_interval;
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = start_pixel_y + search_count_y * pixel_interval;

				bill_info->barcode_search_point_count++;
			}
		}
	}
}


void set_2d_barcode_search_points_tito(const int sensor)
{
	#define BARCODE_2D_SEARCH_X_MM_EDGE		(float)126.0
	#define BARCODE_2D_SEARCH_Y_MM_EDGE		(float)18.0

	#define BARCODE_2D_SEARCH_PITCH_MM_EDGE	(float)6.0

	const int start_pixel_x = bill_info->sensor_info[sensor].deskew_center_x - (int)(BARCODE_2D_SEARCH_X_MM_EDGE / bill_info->sensor_info[sensor].pitch / 2.0);
	//const int end_pixel_x = bill_info->sensor_info[sensor].deskew_center_x + (int)(BARCODE_2D_SEARCH_X_MM_EDGE / bill_info->sensor_info[sensor].pitch / 2.0);
	const int start_pixel_y = bill_info->sensor_info[sensor].deskew_center_y - (int)(BARCODE_2D_SEARCH_Y_MM_EDGE / bill_info->sensor_info[sensor].pitch / 2.0);
	//const int end_pixel_y = bill_info->sensor_info[sensor].deskew_center_y + (int)(BARCODE_2D_SEARCH_Y_MM_EDGE / bill_info->sensor_info[sensor].pitch / 2.0);

	const int width_lines = (int)(BARCODE_2D_SEARCH_X_MM_EDGE /  BARCODE_2D_SEARCH_PITCH_MM_EDGE) + 1;
	const int height_lines = (int)(BARCODE_2D_SEARCH_Y_MM_EDGE /  BARCODE_2D_SEARCH_PITCH_MM_EDGE) + 1;
	const int pixel_interval = (int)(BARCODE_2D_SEARCH_PITCH_MM_EDGE / bill_info->sensor_info[sensor].pitch);

	int search_count_x = 0;
	int search_count_y = 0;

	bill_info->barcode_search_point_count = 0;

	for(search_count_x = 0; search_count_x < width_lines / 6; search_count_x++)
	{
		for(search_count_y = 0; search_count_y < height_lines; search_count_y++)
		{
			if((search_count_x + search_count_y) % 2 == 0) //spread out search points
			{
				if(bill_info->ticket_direction == A_DIRECTION
					|| bill_info->ticket_direction == C_DIRECTION)
				{
					bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = start_pixel_x + search_count_x * pixel_interval;
					bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = start_pixel_y + search_count_y * pixel_interval;

					bill_info->barcode_search_point_count++;
				}
			}
			else
			{
				if(bill_info->ticket_direction == B_DIRECTION
					|| bill_info->ticket_direction == D_DIRECTION)
				{
					bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = start_pixel_x + (width_lines - search_count_x - 1) * pixel_interval;
					bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = start_pixel_y + search_count_y * pixel_interval;

					bill_info->barcode_search_point_count++;
				}
			}
		}
	}
}


void set_2d_barcode_search_points_full(const int sensor)
{
	#define BARCODE_2D_SEARCH_X_MM_FULL		(float)138.0
	#define BARCODE_2D_SEARCH_Y_MM_FULL		(float)48.0

	#define BARCODE_2D_SEARCH_PITCH_MM_FULL	(float)6.0

	const int start_pixel_x = bill_info->sensor_info[sensor].deskew_center_x - (int)(BARCODE_2D_SEARCH_X_MM_FULL / bill_info->sensor_info[sensor].pitch / 2.0);
	//const int end_pixel_x = bill_info->sensor_info[sensor].deskew_center_x + (int)(BARCODE_2D_SEARCH_X_MM_FULL / bill_info->sensor_info[sensor].pitch / 2.0);
	const int start_pixel_y = bill_info->sensor_info[sensor].deskew_center_y - (int)(BARCODE_2D_SEARCH_Y_MM_FULL / bill_info->sensor_info[sensor].pitch / 2.0);
	//const int end_pixel_y = bill_info->sensor_info[sensor].deskew_center_y + (int)(BARCODE_2D_SEARCH_Y_MM_FULL / bill_info->sensor_info[sensor].pitch / 2.0);

	const int width_lines = (int)(BARCODE_2D_SEARCH_X_MM_FULL /  BARCODE_2D_SEARCH_PITCH_MM_FULL) + 1;
	const int height_lines = (int)(BARCODE_2D_SEARCH_Y_MM_FULL /  BARCODE_2D_SEARCH_PITCH_MM_FULL) + 1;
	const int pixel_interval = (int)(BARCODE_2D_SEARCH_PITCH_MM_FULL / bill_info->sensor_info[sensor].pitch);

	int search_count_x = 0;
	int search_count_y = 0;

	bill_info->barcode_search_point_count = 0;

	for(search_count_x = 0; search_count_x < width_lines / 2; search_count_x++)
	{
		for(search_count_y = 0; search_count_y < height_lines; search_count_y++)
		{
			if((search_count_x + search_count_y) % 2 == 0) //spread out search points
			{
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = start_pixel_x + search_count_x * pixel_interval;
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = start_pixel_y + search_count_y * pixel_interval;

				bill_info->barcode_search_point_count++;
			}
			else
			{
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].x = start_pixel_x + (width_lines - search_count_x - 1) * pixel_interval;
				bill_info->barcode_search_points[bill_info->barcode_search_point_count].y = start_pixel_y + search_count_y * pixel_interval;

				bill_info->barcode_search_point_count++;
			}
		}
	}
}

void tito_ticket_ir_check()
{
	int area_count_x = 0;
	int area_count_y = 0;

	int total = 0;
	int points_used = 0;

	for(area_count_x = MESH_AREA_COUNT_X / 2 - 2; area_count_x <= MESH_AREA_COUNT_X / 2 + 1; area_count_x++)
	{
		for(area_count_y = MESH_AREA_COUNT_Y / 2 - 1; area_count_y <= MESH_AREA_COUNT_Y / 2; area_count_y++)
		{
			if(bill_info->sensor_info[bill_info->tito_ticket_result.sensor].side == CIS_SIDE_UP)
			{
				total += get_unnormalized_mesh_area_average(CIS_IR1_UP, area_count_x, area_count_y);
				points_used++;
			}
			else if(bill_info->sensor_info[bill_info->tito_ticket_result.sensor].side == CIS_SIDE_DOWN)
			{
				total += get_unnormalized_mesh_area_average(CIS_IR1_DOWN, area_count_x, area_count_y);
				points_used++;
			}
		}
	}

	total /= points_used;

	if(total < BARCODE_IR_LIMIT)
	{
		bill_info->itf_barcode_has_ir = TRUE;
	}
	else
	{
		bill_info->itf_barcode_has_ir = FALSE;
	}
}

const int double_ticket_mesh_offsets[DOUBLE_TICKET_OFFSET_COUNT] =
{
	MESH_AREA_COUNT_Y / 2 - 8,
	MESH_AREA_COUNT_Y / 2 - 7,
	MESH_AREA_COUNT_Y / 2 - 6,
	MESH_AREA_COUNT_Y / 2 - 5,
	MESH_AREA_COUNT_Y / 2 - 4,
	MESH_AREA_COUNT_Y / 2 - 3,
	MESH_AREA_COUNT_Y / 2 - 2,
	MESH_AREA_COUNT_Y / 2 - 1,
	//
	MESH_AREA_COUNT_Y / 2 + 1,
	MESH_AREA_COUNT_Y / 2 + 2,
	MESH_AREA_COUNT_Y / 2 + 3,
	MESH_AREA_COUNT_Y / 2 + 4,
	MESH_AREA_COUNT_Y / 2 + 5,
	MESH_AREA_COUNT_Y / 2 + 6,
	MESH_AREA_COUNT_Y / 2 + 7,
	MESH_AREA_COUNT_Y / 2 + 8,
};


int is_double_ticket()
{
#if 0//#if (_DEBUG_BAR_DOUBLE_RESULT==1) //デバッグ用
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
	int area_max = 0;
#endif
	int offset_count = 0;
	int area_count_x = 0;

	int temp_area = 0;

	int limit = 0;

	if(bill_info->itf_barcode_has_ir == FALSE) //thermal paper ticket
	{
		limit = THERMAL_TICKET_DOUBLE_LIMIT;
    }
	else if(bill_info->itf_barcode_has_ir == TRUE) //promo ticket
	{
    	limit = PROMO_TICKET_DOUBLE_LIMIT;
	}
	for(offset_count = 0; offset_count < DOUBLE_TICKET_OFFSET_COUNT; offset_count++)
	{
		for(area_count_x = bill_info->mesh_start_area_x + offset_count % 2 + DOUBLE_TICKET_EDGE_SKIP; area_count_x <= bill_info->mesh_end_area_x - DOUBLE_TICKET_EDGE_SKIP; area_count_x += 2)
		{
			temp_area = get_mesh_area_average(CIS_IR1_TRANSPARENT_DOWN, area_count_x, double_ticket_mesh_offsets[offset_count]);

#if 0//#if (_DEBUG_BAR_DOUBLE_RESULT==1) //デバッグ用
			if(area_max < temp_area)
			{
				area_max = temp_area;
				pbill_data->st_model_area.length_edge = area_max;
			}
#endif
			if(temp_area > limit)
			{
				return FALSE;
			}
		}
    }

 	return TRUE;
}


int ticket_size_check()
{
	if(bill_info->width_mm < BARCODE_TICKET_MIN_WIDTH
		|| bill_info->width_mm > BARCODE_TICKET_MAX_WIDTH
		|| bill_info->length_mm < BARCODE_TICKET_MIN_LENGTH
		|| bill_info->length_mm > BARCODE_TICKET_MAX_LENGTH)
	{
		return FALSE;
	}

	return TRUE;
}


const CIS_RECTANGLE index_bottom =
{
	{178, 56}, //use 110 pixels in x direction
	{287, 70},
};

const CIS_RECTANGLE index_top =
{
	{515, 264}, //use 110 pixels in x direction
	{624, 280},
};


void ticket_indexmark_check()
{
	const int white_area1_pixel_width = (int)(INDEXMARK_WHITE_AREA1_WIDTH_MM / bill_info->sensor_info[CIS_RED_UP].pitch);
	const int black_area_pixel_width = (int)(INDEXMARK_BLACK_AREA_WIDTH_MM / bill_info->sensor_info[CIS_RED_UP].pitch);
	const int white_area2_pixel_width = (int)(INDEXMARK_WHITE_AREA2_WIDTH_MM / bill_info->sensor_info[CIS_RED_UP].pitch);
	const int search_pattern_pixel_count = white_area1_pixel_width + black_area_pixel_width + white_area2_pixel_width;

	int pixel_data[DIRECTION_COUNT][INDEXMARK_TOTAL_SEARCH_AREA];

	int index_value_final[DIRECTION_COUNT];
	int index_value_temp;

	int values_found = 0;

	int pixel_count = 0;
	int sub_pixel_count = 0;
	int direction_count = 0;

	int points_used = 0;

	memset(pixel_data, 0, sizeof(pixel_data));
	memset(index_value_final, 0, sizeof(index_value_final));

	//convert CIS to line data
	for(pixel_count = 0; pixel_count < INDEXMARK_TOTAL_SEARCH_AREA; pixel_count++)
	{
		pixel_data[A_DIRECTION][pixel_count] = get_deskewed_rectangle_average(
			CIS_RED_DOWN,
			pixel_count + index_top.start.x,
			index_top.start.y,
			1,
			index_top.end.y - index_top.start.y + 1);

		pixel_data[B_DIRECTION][pixel_count] = get_deskewed_rectangle_average(
			CIS_RED_DOWN,
			pixel_count + index_bottom.start.x,
			index_bottom.start.y,
			1,
			index_bottom.end.y - index_bottom.start.y + 1);

		pixel_data[C_DIRECTION][pixel_count] = get_deskewed_rectangle_average(
			CIS_RED_UP,
			pixel_count + index_top.start.x,
			index_top.start.y,
			1,
			index_top.end.y - index_top.start.y + 1);

		pixel_data[D_DIRECTION][pixel_count] = get_deskewed_rectangle_average(
			CIS_RED_UP,
			pixel_count + index_bottom.start.x,
			index_bottom.start.y,
			1,
			index_bottom.end.y - index_bottom.start.y + 1);
	}

	for(direction_count = 0; direction_count < DIRECTION_COUNT; direction_count++)
	{
		for(pixel_count = 0; pixel_count < INDEXMARK_TOTAL_SEARCH_AREA - search_pattern_pixel_count; pixel_count++)
		{
			index_value_temp = 0;
			points_used = 0;

			//first white area
			for(sub_pixel_count = 0; sub_pixel_count < white_area1_pixel_width - 1; sub_pixel_count++)
			{
				index_value_temp += pixel_data[direction_count][pixel_count + sub_pixel_count];
				points_used++;
			}

			//black area
			for(sub_pixel_count = white_area1_pixel_width + 1; sub_pixel_count < white_area1_pixel_width + black_area_pixel_width - 1; sub_pixel_count++)
			{
				index_value_temp -= (pixel_data[direction_count][pixel_count + sub_pixel_count] * 4);
				points_used += 4;
			}

			//second white area
			for(sub_pixel_count = white_area1_pixel_width + black_area_pixel_width + 1; sub_pixel_count < search_pattern_pixel_count; sub_pixel_count++)
			{
				index_value_temp += pixel_data[direction_count][pixel_count + sub_pixel_count];
				points_used++;
			}

			index_value_temp /= points_used;

			if(index_value_temp > index_value_final[direction_count])
			{
				index_value_final[direction_count] = index_value_temp;
			}
		}
	}

	for(direction_count = 0; direction_count < DIRECTION_COUNT; direction_count++)
	{
		if(index_value_final[direction_count] > TICKET_INDEXMARK_LIMIT)
		{
			bill_info->ticket_direction = direction_count;
			values_found++;
		}
	}

	if(values_found != 1)
	{
		bill_info->ticket_direction = UNKNOWN_DIRECTION;
	}
}

void clear_itf_barcode_values()
{
	int count = 0;

	for(count = 0; count < CIS_MAX_PIXEL_COUNT_X; count++)
	{
		bill_info->barcode_1d_wave[count] = 0;
	}

	for(count = 0; count < BARCODE_1D_MAXIMUM_BAR_COUNT; count++)
	{
		bill_info->barcode_1d_areas[count] = 0;
		bill_info->barcode_1d_widths[count] = 0;
		bill_info->barcode_1d_ratios[count] = 0.0;
	}

	bill_info->barcode_1d_bar_count = 0;

	bill_info->itf_barcode_narrow_area_average = 0.0;
	bill_info->itf_barcode_wide_area_average = 0.0;

	bill_info->itf_barcode_narrow_bar_count = 0;
	bill_info->itf_barcode_wide_bar_count = 0;

	bill_info->itf_barcode_has_ir = FALSE;

	memset(&bill_info->itf_barcode_result_temp, 0, sizeof(BARCODE_1D_RESULT));
}

