/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_2d_barcode.c
* Contents: common 2 dimensional barcode
*
*
*******************************************************************************/

#define EXT
#include "tem_global.c"

#include "tem_2d_barcode.h"
#include "tem_pixel.h"

int get_2d_barcode_left_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length)
{
	const int maximum_pixel_search = (int)(BARCODE_2D_MAXIMUM_PIXEL_SEARCH_MM / bill_info->sensor_info[barcode_result->sensor].pitch);

	int pixel_count = 0;

	int white_area_counter = 0;
	int last_black_pixel = 0;

	int result = 0;

	//search from the center to the left
	white_area_counter = 0;
	for(pixel_count = barcode_result->x_point; pixel_count >= 0; pixel_count--)
	{
		if(get_2d_barcode_edge_search_value_x(barcode_result->sensor, pixel_count,
			barcode_result->y_point) >= edge_level) //white area
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

		if(barcode_result->x_point - pixel_count > maximum_pixel_search)
		{
			break;
		}
	}

	return result;
}


int get_2d_barcode_right_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length)
{
	const int maximum_pixel_search = (int)(BARCODE_2D_MAXIMUM_PIXEL_SEARCH_MM / bill_info->sensor_info[barcode_result->sensor].pitch);

	int pixel_count = 0;

	int white_area_counter = 0;
	int last_black_pixel = 0;

	int result = 0;

	//search from the center to the right
	white_area_counter = 0;
	for(pixel_count = barcode_result->x_point; pixel_count < bill_info->sensor_info[barcode_result->sensor].x_pixels; pixel_count++)
	{
		if(get_2d_barcode_edge_search_value_x(barcode_result->sensor, pixel_count,
			barcode_result->y_point) >= edge_level) //white area
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

		if(pixel_count - barcode_result->x_point > maximum_pixel_search)
		{
			break;
		}
	}

	return result;
}


int get_2d_barcode_bottom_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length)
{
	const int maximum_pixel_search = (int)(BARCODE_2D_MAXIMUM_PIXEL_SEARCH_MM / bill_info->sensor_info[barcode_result->sensor].pitch);

	int pixel_count = 0;

	int white_area_counter = 0;
	int last_black_pixel = 0;

	int result = 0;

	//search from the center to the bottom
	white_area_counter = 0;
	for(pixel_count = barcode_result->y_point; pixel_count >= 0; pixel_count--)
	{
		if(get_2d_barcode_edge_search_value_y(barcode_result->sensor, barcode_result->x_point,
			pixel_count) >= edge_level) //white area
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

		if(barcode_result->y_point - pixel_count > maximum_pixel_search)
		{
			break;
		}
	}

	return result;
}


int get_2d_barcode_top_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length)
{
	const int maximum_pixel_search = (int)(BARCODE_2D_MAXIMUM_PIXEL_SEARCH_MM / bill_info->sensor_info[barcode_result->sensor].pitch);

	int pixel_count = 0;

	int white_area_counter = 0;
	int last_black_pixel = 0;

	int result = 0;

	//search from the center to the top
	white_area_counter = 0;
	for(pixel_count = barcode_result->y_point; pixel_count < bill_info->sensor_info[barcode_result->sensor].y_pixels; pixel_count++)
	{
		if(get_2d_barcode_edge_search_value_y(barcode_result->sensor, barcode_result->x_point,
			pixel_count) >= edge_level) //white area
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

		if(pixel_count - barcode_result->y_point > maximum_pixel_search)
		{
			break;
		}

	}

	return result;
}


int get_2d_barcode_edge_search_value_x(const int sensor, const int x_point, const int y_point)
{
	const int edge_search_pixel_width = (int)(BARCODE_2D_EDGE_SEARCH_WIDTH_MM / bill_info->sensor_info[sensor].pitch);
	const int edge_search_pixel_skip = (int)(BARCODE_2D_EDGE_SEARCH_PIXEL_SKIP_MM / bill_info->sensor_info[sensor].pitch);

	int pixel_count = 0;

	int temp = 0;
	int darkest_pixel = 255;
	for(pixel_count = y_point - edge_search_pixel_width; pixel_count <= y_point + edge_search_pixel_width; pixel_count += edge_search_pixel_skip)
	{
		temp = get_deskewed_pixel(sensor, x_point, pixel_count);

		if(temp < darkest_pixel)
		{
			darkest_pixel = temp;
		}
	}
	return darkest_pixel;
}


int get_2d_barcode_edge_search_value_y(const int sensor, const int x_point, const int y_point)
{
	const int edge_search_pixel_width = (int)(BARCODE_2D_EDGE_SEARCH_WIDTH_MM / bill_info->sensor_info[sensor].pitch);
	const int edge_search_pixel_skip = (int)(BARCODE_2D_EDGE_SEARCH_PIXEL_SKIP_MM / bill_info->sensor_info[sensor].pitch);

	int pixel_count = 0;

	int temp = 0;
	int darkest_pixel = 255;

	for(pixel_count = x_point - edge_search_pixel_width; pixel_count <= x_point + edge_search_pixel_width; pixel_count += edge_search_pixel_skip)
	{
		temp = get_deskewed_pixel(sensor, pixel_count, y_point);

		if(temp < darkest_pixel)
		{
			darkest_pixel = temp;
		}
	}

	return darkest_pixel;
}


//remap to 0 degree orientation
int get_2d_barcode_rotated_pixel(P_BARCODE_2D_RESULT const barcode_result,
	const int x_point, const int y_point)
{
	int x_point_temp = 0;
	int y_point_temp = 0;

	if(barcode_result->orientation == BARCODE_2D_ORIENTATION_1)
	{
		x_point_temp = barcode_result->left_edge + x_point;
		y_point_temp = barcode_result->bottom_edge + y_point;
	}
	else if(barcode_result->orientation == BARCODE_2D_ORIENTATION_2)
	{
		x_point_temp = barcode_result->left_edge + y_point;
		y_point_temp = barcode_result->top_edge - x_point - 1;
	}
	else if(barcode_result->orientation == BARCODE_2D_ORIENTATION_3)
	{
		x_point_temp = barcode_result->right_edge - x_point - 1;
		y_point_temp = barcode_result->top_edge - y_point - 1;
	}
	else if(barcode_result->orientation == BARCODE_2D_ORIENTATION_4)
	{
		x_point_temp = barcode_result->right_edge - y_point - 1;
		y_point_temp = barcode_result->bottom_edge + x_point;
	}

	return get_deskewed_pixel(barcode_result->sensor, x_point_temp, y_point_temp);
}


int get_2d_barcode_cell_average(P_BARCODE_2D_RESULT const barcode_result, const int x_cell, const int y_cell)
{
	const int x_start_pixel = (int)(barcode_result->rotated_pixels_per_cell_width * (float)x_cell);
	const int x_end_pixel = (int)(barcode_result->rotated_pixels_per_cell_width * (float)(x_cell + 1)) - 1;
	const int y_start_pixel = (int)(barcode_result->rotated_pixels_per_cell_height * (float)y_cell);
	const int y_end_pixel = (int)(barcode_result->rotated_pixels_per_cell_height * (float)(y_cell + 1)) - 1;

	const int edge_pixel_skip_x = (int)(((float)BARCODE_2D_CELL_EDGE_SKIP * barcode_result->rotated_pixels_per_cell_width) / 100.0);
	const int edge_pixel_skip_y = (int)(((float)BARCODE_2D_CELL_EDGE_SKIP * barcode_result->rotated_pixels_per_cell_height) / 100.0);

	int pixel_count_x = 0;
	int pixel_count_y = 0;

	int total = 0;
	int pixels_used = 0;

	for(pixel_count_x = x_start_pixel + edge_pixel_skip_x; pixel_count_x <= x_end_pixel - edge_pixel_skip_x; pixel_count_x++)
	{
		for(pixel_count_y = y_start_pixel + edge_pixel_skip_y; pixel_count_y <= y_end_pixel - edge_pixel_skip_y; pixel_count_y++)
		{
			int temp = get_2d_barcode_rotated_pixel(barcode_result, pixel_count_x, pixel_count_y);

			total += temp;

			pixels_used++;
		}
	}

	if(pixels_used == 0)
	{
		total = 0;
	}
	else
	{
		total /= pixels_used;
	}

	return total;
}


int get_2d_barcode_enhanced_cell_average(P_BARCODE_2D_RESULT const barcode_result, const int x_cell, const int y_cell)
{
	const int x_start_pixel = barcode_result->cell_start_x[x_cell];
	const int x_end_pixel = barcode_result->cell_start_x[x_cell + 1] - 1;
	const int y_start_pixel = barcode_result->cell_start_y[y_cell];
	const int y_end_pixel = barcode_result->cell_start_y[y_cell + 1] - 1;

	const int edge_pixel_skip_x = (BARCODE_2D_CELL_EDGE_SKIP * (x_end_pixel - x_start_pixel + 1)) / 100;
	const int edge_pixel_skip_y = (BARCODE_2D_CELL_EDGE_SKIP * (y_end_pixel - y_start_pixel + 1)) / 100;

	int pixel_count_x = 0;
	int pixel_count_y = 0;

	int total = 0;
	int pixels_used = 0;

	for(pixel_count_x = x_start_pixel + edge_pixel_skip_x; pixel_count_x <= x_end_pixel - edge_pixel_skip_x; pixel_count_x++)
	{
		for(pixel_count_y = y_start_pixel + edge_pixel_skip_y; pixel_count_y <= y_end_pixel - edge_pixel_skip_y; pixel_count_y++)
		{
			int temp = get_2d_barcode_rotated_pixel(barcode_result, pixel_count_x, pixel_count_y);

			total += temp;

			pixels_used++;
		}
	}

	if(pixels_used == 0)
	{
		total = 0;
	}
	else
	{
		total /= pixels_used;
	}

	return total;
}
