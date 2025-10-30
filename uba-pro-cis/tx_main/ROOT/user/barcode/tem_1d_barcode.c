/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_1d_barcode.h
* Contents: common 1 dimensional barcode
*
*
*******************************************************************************/

#define EXT
#include "../common/global.h"
#include "cis_ram.c"
#include "tem_global.c"

#include "tem_pixel.h"

void create_1d_barcode_wave(P_BARCODE_1D_RESULT const barcode_result)
{
	int y_count = 0;
	int x_count = 0;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *p_data_collection = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *p_data_collection = (ST_BS *)BILL_NOTE_IMAGE_TOP;
#endif
	int data_offset = 0;
	data_offset = p_data_collection->PlaneInfo[barcode_result->sensor].Address_Period;
	if(barcode_result->orientation == BARCODE_1D_ORIENATION_HORIZONTAL)
	{
		for (x_count = 0; x_count < bill_info->sensor_info[barcode_result->sensor].x_pixels; x_count++)
		{
			bill_info->barcode_1d_wave[x_count] = (unsigned char)get_raw_pixel(barcode_result->sensor, x_count, barcode_result->y_point);
		}
	}
	else if(barcode_result->orientation == BARCODE_1D_ORIENATION_VERTICAL)
	{
		for (y_count = 0; y_count < bill_info->sensor_info[barcode_result->sensor].y_pixels; y_count++)
		{
			bill_info->barcode_1d_wave[y_count] = (unsigned char)get_raw_pixel(barcode_result->sensor, barcode_result->x_point, y_count);
		}
	}
}


int get_1d_barcode_left_edge(const int start_point, const int end_point, const int edge_level,
	const int edge_white_area_length)
{
	int pixel_count = 0;

	int white_area_counter = 0;
	int last_black_pixel = start_point;

	int result = 0;

	//search from the center to the left side
	white_area_counter = 0;
	for(pixel_count = start_point; pixel_count >= end_point; pixel_count--)
	{
		if((int)bill_info->barcode_1d_wave[pixel_count] >= edge_level) //white area
		{
			white_area_counter++;
		}
		else //black area
		{
			last_black_pixel = pixel_count;

			white_area_counter = 0;
		}

		if(white_area_counter > edge_white_area_length) //sufficient white area found
		{
			result = last_black_pixel;

			break;
		}
	}

	return result;
}


int get_1d_barcode_right_edge(const int start_point, const int end_point, const int edge_level,
	const int edge_white_area_length)
{
	int pixel_count = 0;

	int white_area_counter = 0;
	int last_black_pixel = start_point;

	int result = 0;

	//search from the center to the left side
	white_area_counter = 0;
	for(pixel_count = start_point; pixel_count <= end_point; pixel_count++)
	{
		if((int)bill_info->barcode_1d_wave[pixel_count] >= edge_level) //white area
		{
			white_area_counter++;
		}
		else //black area
		{
			last_black_pixel = pixel_count;

			white_area_counter = 0;
		}

		if(white_area_counter > edge_white_area_length) //sufficient white area found
		{
			result = last_black_pixel;

			break;
		}
	}

	return result;
}


int get_1d_barcode_black_white_average(const int start_point, const int end_point)
{
    int pixel_count = 0;

	int result = 0;

	for(pixel_count = start_point; pixel_count <= end_point; pixel_count++)
	{
		result += (int)bill_info->barcode_1d_wave[pixel_count];
	}

	 result /= (end_point - start_point + 1);

	return result;
}


void parse_1d_barcode(const int left_edge, const int right_edge, const int black_white_average)
{
	int pixel_count = 0;

	int area_total = 0;

	int current_area = BLACK_BAR;

	int previous_transition_point = left_edge;

	bill_info->barcode_1d_bar_count = 0;

	for(pixel_count = left_edge; pixel_count <= right_edge; pixel_count++)
	{
		if(current_area == BLACK_BAR)
		{
			area_total += black_white_average - (int)bill_info->barcode_1d_wave[pixel_count];

			if((int)bill_info->barcode_1d_wave[pixel_count + 1] >= black_white_average) //transition to white area
			{
				bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count] = area_total;
				bill_info->barcode_1d_widths[bill_info->barcode_1d_bar_count] = pixel_count + 1 - previous_transition_point;

				area_total = 0;
				current_area = WHITE_BAR;

				previous_transition_point = pixel_count + 1;

				bill_info->barcode_1d_bar_count++;
			}
		}
		else if(current_area == WHITE_BAR)
		{
			area_total += (int)bill_info->barcode_1d_wave[pixel_count] - black_white_average;

			if(bill_info->barcode_1d_wave[pixel_count + 1] < black_white_average) //transition to black area
			{
				bill_info->barcode_1d_areas[bill_info->barcode_1d_bar_count] = area_total;
				bill_info->barcode_1d_widths[bill_info->barcode_1d_bar_count] = pixel_count + 1 - previous_transition_point;

				area_total = 0;
				current_area = BLACK_BAR;

				previous_transition_point = pixel_count + 1;

				bill_info->barcode_1d_bar_count++;
			}
		}
	}
}
