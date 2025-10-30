
/******************************************************************************/
/*! @addtogroup Group1
    @file       tem_itf_barcode.c
    @brief      interleaved 2 of 5 barcode format
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
#include "com_ram.c"
#include "cis_ram.c"
#include "tem_global.c"

#include "tem_main.h"
#include "tem_main_barcode.h"
#include "tem_1d_barcode.h"
#include "tem_itf_barcode.h"
#include "tem_network.h"
#include "data_itf_barcode.h"


void itf_barcode_search(const int sensor, const int orientation)
{
	int search_count = 0;

	memset(&bill_info->itf_barcode_result, 0, sizeof(BARCODE_1D_RESULT));

	for(search_count = 0; search_count < bill_info->barcode_search_point_count; search_count++)
	{
		reset_watchdog_timer();
		clear_itf_barcode_values();

		bill_info->itf_barcode_result_temp.sensor = sensor;
		bill_info->itf_barcode_result_temp.x_point = bill_info->barcode_search_points[search_count].x;
		bill_info->itf_barcode_result_temp.y_point = bill_info->barcode_search_points[search_count].y;
		bill_info->itf_barcode_result_temp.orientation = orientation;

		if(orientation == BARCODE_1D_ORIENATION_HORIZONTAL)
		{
			bill_info->itf_barcode_result_temp.wave_length = bill_info->sensor_info[sensor].x_pixels;
			bill_info->itf_barcode_result_temp.wave_point = bill_info->itf_barcode_result_temp.x_point;
		}
		else if(orientation == BARCODE_1D_ORIENATION_VERTICAL)
		{
			bill_info->itf_barcode_result_temp.wave_length = bill_info->sensor_info[sensor].y_pixels;
			bill_info->itf_barcode_result_temp.wave_point = bill_info->itf_barcode_result_temp.y_point;
		}

		//convert cis data to 1 dimensional wave
		create_1d_barcode_wave(&bill_info->itf_barcode_result_temp);

		//first edge check
		itf_barcode_find_edges(BARCODE_1D_EDGE_LEVEL);
		if(bill_info->itf_barcode_result_temp.error_code != 0)
		{
			if(bill_info->itf_barcode_result_temp.error_code > bill_info->itf_barcode_result.error_code)
			{
				memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
			}

			continue;
		}

		bill_info->itf_barcode_result_temp.black_white_average = get_1d_barcode_black_white_average(bill_info->itf_barcode_result_temp.left_edge,
				bill_info->itf_barcode_result_temp.right_edge);

		//second edge check
		itf_barcode_find_edges(bill_info->itf_barcode_result_temp.black_white_average);
		if(bill_info->itf_barcode_result_temp.error_code != 0)
		{
			if(bill_info->itf_barcode_result_temp.error_code > bill_info->itf_barcode_result.error_code)
			{
				memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
			}

			continue;
		}

		parse_1d_barcode(bill_info->itf_barcode_result_temp.left_edge, bill_info->itf_barcode_result_temp.right_edge,
			bill_info->itf_barcode_result_temp.black_white_average);

		itf_barcode_check_bars();
		if(bill_info->itf_barcode_result_temp.error_code != 0)
		{
			if(bill_info->itf_barcode_result_temp.error_code > bill_info->itf_barcode_result.error_code)
			{
				memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
			}

			continue;
		}

		itf_barcode_find_area_averages();
		if(bill_info->itf_barcode_result_temp.error_code != 0)
		{
			if(bill_info->itf_barcode_result_temp.error_code > bill_info->itf_barcode_result.error_code)
			{
				memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
			}

			continue;
		}

		itf_barcode_find_ratios();

		itf_barcode_find_direction();
		if(bill_info->itf_barcode_result_temp.error_code != 0)
		{
			if(bill_info->itf_barcode_result_temp.error_code > bill_info->itf_barcode_result.error_code)
			{
				memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
			}

			continue;
		}

		itf_barcode_decode();
		if(bill_info->itf_barcode_result_temp.error_code != 0)
		{
			if(bill_info->itf_barcode_result_temp.digits_decoded > bill_info->itf_barcode_result.digits_decoded)
			{
				memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
			}

			continue;
		}

		//successfull in finding a result
		if(bill_info->itf_barcode_result_temp.error_code == 0)
		{
			bill_info->itf_barcode_result_temp.try_count = search_count + 1;
			memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
		}

		break;
	}
}


//only search 1 line down the center for bar widths
void itf_barcode_search_test(const int sensor, const int orientation)
{
	memset(&bill_info->itf_barcode_result, 0, sizeof(BARCODE_1D_RESULT));

	clear_itf_barcode_values();

	bill_info->itf_barcode_result_temp.sensor = sensor;
	bill_info->itf_barcode_result_temp.x_point = bill_info->barcode_search_points[0].x;
	bill_info->itf_barcode_result_temp.y_point = bill_info->barcode_search_points[0].y;
	bill_info->itf_barcode_result_temp.orientation = orientation;

	if(orientation == BARCODE_1D_ORIENATION_HORIZONTAL)
	{
		bill_info->itf_barcode_result_temp.wave_length = bill_info->sensor_info[sensor].x_pixels;
		bill_info->itf_barcode_result_temp.wave_point = bill_info->itf_barcode_result_temp.x_point;
	}
	else if(orientation == BARCODE_1D_ORIENATION_VERTICAL)
	{
		bill_info->itf_barcode_result_temp.wave_length = bill_info->sensor_info[sensor].y_pixels;
		bill_info->itf_barcode_result_temp.wave_point = bill_info->itf_barcode_result_temp.y_point;
	}

	//convert cis data to 1 dimensional wave
	create_1d_barcode_wave(&bill_info->itf_barcode_result_temp);

	//first edge check
	itf_barcode_find_edges(BARCODE_1D_EDGE_LEVEL);
	if(bill_info->itf_barcode_result_temp.error_code != 0)
	{
		if(bill_info->itf_barcode_result_temp.error_code > bill_info->itf_barcode_result.error_code)
		{
			memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
		}

		return;
	}

	bill_info->itf_barcode_result_temp.black_white_average = get_1d_barcode_black_white_average(bill_info->itf_barcode_result_temp.left_edge,
			bill_info->itf_barcode_result_temp.right_edge);

	//second edge check
	itf_barcode_find_edges(bill_info->itf_barcode_result_temp.black_white_average);
	if(bill_info->itf_barcode_result_temp.error_code != 0)
	{
		if(bill_info->itf_barcode_result_temp.error_code > bill_info->itf_barcode_result.error_code)
		{
			memcpy(&bill_info->itf_barcode_result, &bill_info->itf_barcode_result_temp, sizeof(BARCODE_1D_RESULT));
		}

		return;
	}

	parse_1d_barcode(bill_info->itf_barcode_result_temp.left_edge, bill_info->itf_barcode_result_temp.right_edge,
		bill_info->itf_barcode_result_temp.black_white_average);
}


void itf_barcode_find_edges(const int edge_level)
{
	const int edge_white_area_pixel_count = (int)(BARCODE_1D_WHITE_EDGE_AREA_MM / bill_info->sensor_info[bill_info->itf_barcode_result_temp.sensor].pitch);
	const int minimum_barcode_pixel_width = (int)(BARCODE_1D_MINIMUM_WIDTH_MM / bill_info->sensor_info[bill_info->itf_barcode_result_temp.sensor].pitch);

	bill_info->itf_barcode_result_temp.left_edge = get_1d_barcode_left_edge(bill_info->itf_barcode_result_temp.wave_point,
		0, edge_level, edge_white_area_pixel_count);

	bill_info->itf_barcode_result_temp.right_edge = get_1d_barcode_right_edge(bill_info->itf_barcode_result_temp.wave_point,
		bill_info->itf_barcode_result_temp.wave_length - 1, edge_level, edge_white_area_pixel_count);

	//check result
	if(bill_info->itf_barcode_result_temp.left_edge == 0
		|| bill_info->itf_barcode_result_temp.right_edge == 0
		|| bill_info->itf_barcode_result_temp.right_edge - bill_info->itf_barcode_result_temp.left_edge < minimum_barcode_pixel_width)
	{
		bill_info->itf_barcode_result_temp.error_code = ITF_BARCODE_RESULT_EDGE_ERROR;
	}
}


void itf_barcode_check_bars()
{
	int bar_count = 0;

	//check that the areas for all bars are within limits
	for(bar_count = 0; bar_count < bill_info->barcode_1d_bar_count; bar_count++)
	{
		if(bill_info->barcode_1d_areas[bar_count] > BARCODE_1D_MAXIMUM_BAR_AREA
			|| bill_info->barcode_1d_areas[bar_count] < BARCODE_1D_MINIMUM_BAR_AREA)
		{
			bill_info->itf_barcode_result_temp.error_code = ITF_BARCODE_RESULT_AREA_ERROR;

			return;
		}
	}

	//calculate information about the barcode ticket
	bill_info->itf_barcode_result_temp.character_length = (bill_info->barcode_1d_bar_count - ITF_BARCODE_START_CODE_LENGTH - ITF_BARCODE_STOP_CODE_LENGTH) / ITF_BARCODE_BARS_PER_DIGIT;
	bill_info->itf_barcode_wide_bar_count = bill_info->itf_barcode_result_temp.character_length * 2 + 1;
	bill_info->itf_barcode_narrow_bar_count = bill_info->itf_barcode_result_temp.character_length * 3 + 6;

	//check for the correct amount of bars
	if((bill_info->barcode_1d_bar_count - ITF_BARCODE_START_CODE_LENGTH - ITF_BARCODE_STOP_CODE_LENGTH) % ITF_BARCODE_BARS_PER_DIGIT != 0
		|| bill_info->barcode_1d_bar_count < ITF_BARCODE_MINIMUM_BAR_COUNT
		|| bill_info->barcode_1d_bar_count > ITF_BARCODE_MAXIMUM_BAR_COUNT)
	{
		bill_info->itf_barcode_result_temp.error_code = ITF_BARCODE_RESULT_COUNT_ERROR;
	}
}


void itf_barcode_find_area_averages()
{
	static int area_totals[BARCODE_1D_MAXIMUM_BAR_AREA]; //static for large allocation

	int area_count = 0;
	int bar_count = 0;

	int area_total = 0;

	for(area_count = 0; area_count < BARCODE_1D_MAXIMUM_BAR_AREA; area_count++)
	{
		area_totals[area_count] = 0;
	}

	for(bar_count = 0; bar_count < bill_info->barcode_1d_bar_count; bar_count++)
	{
		area_totals[bill_info->barcode_1d_areas[bar_count]]++;
	}

	//find narrow bar maximum size
	area_total = 0;
	bill_info->itf_barcode_narrow_area_average = 0.0;
	for(area_count = 0; area_count < BARCODE_1D_MAXIMUM_BAR_AREA; area_count++)
	{
		area_total += area_totals[area_count];

		if(area_total >= bill_info->itf_barcode_narrow_bar_count) //clear separation of wide and narrow bars
		{
			bill_info->itf_barcode_narrow_area_average += (float)area_count * ((float)area_total - (float)bill_info->itf_barcode_narrow_bar_count);

			break;
		}
		else
		{
			bill_info->itf_barcode_narrow_area_average += (float)area_count * (float)area_totals[area_count];
		}
	}

	bill_info->itf_barcode_narrow_area_average /= (float)bill_info->itf_barcode_narrow_bar_count;

	//find wide bar minimum size
	area_total = 0;
	bill_info->itf_barcode_wide_area_average = 0.0;
	for(area_count = BARCODE_1D_MAXIMUM_BAR_AREA - 1; area_count > 0; area_count--)
	{
		area_total += area_totals[area_count];

		if(area_total >= bill_info->itf_barcode_wide_bar_count) //clear separation of wide and narrow bars
		{
			bill_info->itf_barcode_wide_area_average += (float)area_count * ((float)area_total - (float)bill_info->itf_barcode_wide_bar_count);

			break;
		}
		else
		{
			bill_info->itf_barcode_wide_area_average += (float)area_count * (float)area_totals[area_count];
		}
	}
	bill_info->itf_barcode_wide_area_average /= (float)bill_info->itf_barcode_wide_bar_count;

	if(bill_info->itf_barcode_wide_area_average / bill_info->itf_barcode_narrow_area_average < ITF_BARCODE_AREA_RATIO_MIN)
	{
		bill_info->itf_barcode_result_temp.error_code = ITF_BARCODE_RESULT_WIDTH_ERROR;
	}
	else if(bill_info->itf_barcode_wide_area_average / bill_info->itf_barcode_narrow_area_average > ITF_BARCODE_AREA_RATIO_MAX)
	{
		bill_info->itf_barcode_result_temp.error_code = ITF_BARCODE_RESULT_WIDTH_ERROR;
	}
}


void itf_barcode_find_ratios()
{
	int bar_count = 0;

	//find area ratios
	for(bar_count = 0; bar_count < BARCODE_1D_MAXIMUM_BAR_COUNT; bar_count++)
	{
		bill_info->barcode_1d_ratios[bar_count] = (float)bill_info->barcode_1d_areas[bar_count] / (bill_info->itf_barcode_narrow_area_average / ITF_BAR_RATIO_NORMALIZE);
	}
}


void itf_barcode_find_direction()
{
	const int left_indicator_bar = bill_info->barcode_1d_areas[2];
	const int right_indicator_bar = bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count - 3];

	if(left_indicator_bar > bill_info->barcode_1d_areas[0]
		&& left_indicator_bar > bill_info->barcode_1d_areas[1]
		&& left_indicator_bar > bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count - 4]
		&& left_indicator_bar > bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count - 3]
		&& left_indicator_bar > bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count - 2]
		&& left_indicator_bar > bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count - 1]) //check for reverse direction
	{
		bill_info->itf_barcode_result_temp.direction = BARCODE_1D_REVERSE_DIRECTION;
	}
	else if(right_indicator_bar > bill_info->barcode_1d_areas[0]
		&& right_indicator_bar > bill_info->barcode_1d_areas[1]
		&& right_indicator_bar > bill_info->barcode_1d_areas[2]
		&& right_indicator_bar > bill_info->barcode_1d_areas[3]
		&& right_indicator_bar > bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count - 2]
		&& right_indicator_bar > bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count - 1]) //check for forward direction
	{
		bill_info->itf_barcode_result_temp.direction = BARCODE_1D_FORWARD_DIRECTION;
	}
	else //cannot find start and stop code case
	{
		bill_info->itf_barcode_result_temp.error_code = ITF_BARCODE_RESULT_START_STOP_CODE_ERROR;
	}
}


void itf_barcode_decode()
{
	int bar_count = 0;
	int character_count = 0;

	float temp_ratio = 0;

	float black_bar_ratios[ITF_BARCODE_BARS_PER_DIGIT];
	float white_bar_ratios[ITF_BARCODE_BARS_PER_DIGIT];

	//reverse ratios so the start sequence is always at the beginning
	if(bill_info->itf_barcode_result_temp.direction == BARCODE_1D_REVERSE_DIRECTION)
	{
		for(bar_count = 0; bar_count < bill_info->barcode_1d_bar_count / 2; bar_count++)
		{
			temp_ratio = bill_info->barcode_1d_ratios[bar_count];
			bill_info->barcode_1d_ratios[bar_count] = bill_info->barcode_1d_ratios[bill_info->barcode_1d_bar_count - 1 - bar_count];
			bill_info->barcode_1d_ratios[bill_info->barcode_1d_bar_count - 1 - bar_count] = temp_ratio;
		}
	}

	//decode 2 digits at a time
	for(character_count = 0; character_count < bill_info->itf_barcode_result_temp.character_length; character_count += 2)
	{
		for(bar_count = 0; bar_count < ITF_BARCODE_BARS_PER_DIGIT; bar_count++)
		{
			black_bar_ratios[bar_count] = bill_info->barcode_1d_ratios[ITF_BARCODE_START_CODE_LENGTH + ITF_BARCODE_BARS_PER_DIGIT * character_count + bar_count * 2];
			white_bar_ratios[bar_count] = bill_info->barcode_1d_ratios[ITF_BARCODE_START_CODE_LENGTH + ITF_BARCODE_BARS_PER_DIGIT * character_count + bar_count * 2 + 1];
		}

		bill_info->itf_barcode_result_temp.characters[character_count] = itf_barcode_character_lookup[itf_barcode_network_decode(black_bar_ratios)];
		bill_info->itf_barcode_result_temp.characters[character_count + 1] = itf_barcode_character_lookup[itf_barcode_network_decode(white_bar_ratios)];

#if DEBUG_VALIDATION_RESULT
		ex_validation.barcode_result.barcode_1d_characters[character_count] = bill_info->itf_barcode_result_temp.characters[character_count];
		ex_validation.barcode_result.barcode_1d_characters[character_count + 1] = bill_info->itf_barcode_result_temp.characters[character_count + 1];
	}
	ex_validation.barcode_result.barcode_1d_character_length = bill_info->itf_barcode_result_temp.character_length;
#else
	}
#endif

	//check if any digits could not be decoded
	for(character_count = 0; character_count < bill_info->itf_barcode_result_temp.character_length; character_count++)
	{
		if(bill_info->itf_barcode_result_temp.characters[character_count] == ITF_BARCODE_UNKNOWN_DIGIT)
		{
			bill_info->itf_barcode_result_temp.error_code = ITF_BARCODE_RESULT_UNKNOWN_DIGITS_ERROR;
		}
		else
		{
			bill_info->itf_barcode_result_temp.digits_decoded++;
		}
	}
}


