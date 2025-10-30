/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_qr_barcode.c
* Contents: handle QR barcode recognition
*
*
*******************************************************************************/

#include <string.h>

#define EXT
#include "com_ram.c"
#include "tem_global.c"


#include "tem_main.h"
#include "tem_2d_barcode.h"
#include "tem_qr_barcode.h"
#include "tem_rs256_correction.h"

#include "data_qr_barcode.h"


void clear_qr_barcode_values()
{
	memset(&bill_info->qr_barcode_result_temp, 0, sizeof(BARCODE_2D_RESULT));
}

void qr_barcode_search(const int sensor)
{
	int search_count = 0;

	memset(&bill_info->qr_barcode_result, 0, sizeof(BARCODE_2D_RESULT));

	for(search_count = 0; search_count < bill_info->barcode_search_point_count; search_count++)
	{
		reset_watchdog_timer();
		clear_qr_barcode_values();

		bill_info->qr_barcode_result_temp.sensor = sensor;
		bill_info->qr_barcode_result_temp.x_point = bill_info->barcode_search_points[search_count].x;
		bill_info->qr_barcode_result_temp.y_point = bill_info->barcode_search_points[search_count].y;

		// qr_barcode_find_left_right_edges faster than qr_barcode_find_bottom_top_edges
		//search left and right
		qr_barcode_find_left_right_edges();
		if(bill_info->qr_barcode_result_temp.error_code != 0)
		{
			if(bill_info->qr_barcode_result_temp.error_code > bill_info->qr_barcode_result.error_code)
			{
				memcpy(&bill_info->qr_barcode_result, &bill_info->qr_barcode_result_temp, sizeof(BARCODE_2D_RESULT));
			}

			continue;
		}

		//search bottom and top
		qr_barcode_find_bottom_top_edges();
		if(bill_info->qr_barcode_result_temp.error_code != 0)
		{
			if(bill_info->qr_barcode_result_temp.error_code > bill_info->qr_barcode_result.error_code)
			{
				memcpy(&bill_info->qr_barcode_result, &bill_info->qr_barcode_result_temp, sizeof(BARCODE_2D_RESULT));
			}

			continue;
		}

		//search border
		qr_barcode_search_border();
		if(bill_info->qr_barcode_result_temp.orientation == BARCODE_2D_ORIENTATION_UNKNOWN)
		{
			bill_info->qr_barcode_result_temp.error_code = QR_BARCODE_RESULT_ORIENTATION_ERROR;

			if(bill_info->qr_barcode_result_temp.error_code > bill_info->qr_barcode_result.error_code)
			{
				memcpy(&bill_info->qr_barcode_result, &bill_info->qr_barcode_result_temp, sizeof(BARCODE_2D_RESULT));
			}

			continue;
		}

		qr_barcode_search_format();
		if(bill_info->qr_barcode_result_temp.error_code != 0)
		{
			if(bill_info->qr_barcode_result_temp.error_code > bill_info->qr_barcode_result.error_code)
			{
				memcpy(&bill_info->qr_barcode_result, &bill_info->qr_barcode_result_temp, sizeof(BARCODE_2D_RESULT));
			}

			continue;
		}
		qr_barcode_search_cells();

		qr_barcode_read_cells();

		//run reed solomon error correction
		if(run_rs256_error_correction(&bill_info->qr_barcode_result_temp.codewords,
			bill_info->qr_barcode_result_temp.correction_codeword_count, (int*)&qr_logarithm_table[0],
				(int*)&qr_exponent_table[0], QR_BARCODE_FIRST_CONSECUTIVE_ROOT) == FALSE)
		{
			bill_info->qr_barcode_result_temp.error_code = QR_BARCODE_RESULT_CORRECTION_ERROR;

			if(bill_info->qr_barcode_result_temp.error_code > bill_info->qr_barcode_result.error_code)
			{
				memcpy(&bill_info->qr_barcode_result, &bill_info->qr_barcode_result_temp, sizeof(BARCODE_2D_RESULT));
			}

			continue;
		}
		bill_info->qr_barcode_result_temp.codeword_error_count = bill_info->rs_correction_error_count;

		qr_barcode_decode_message();
		if(bill_info->qr_barcode_result_temp.error_code != 0)
		{
			if(bill_info->qr_barcode_result_temp.error_code > bill_info->qr_barcode_result.error_code)
			{
				memcpy(&bill_info->qr_barcode_result, &bill_info->qr_barcode_result_temp, sizeof(BARCODE_2D_RESULT));
			}

			continue;
		}

		//successfull decoding
		bill_info->qr_barcode_result_temp.try_count = search_count;

		memcpy(&bill_info->qr_barcode_result, &bill_info->qr_barcode_result_temp, sizeof(BARCODE_2D_RESULT));

#if DEBUG_VALIDATION_RESULT
		ex_validation.barcode_result.barcode_2d_error_correction = bill_info->qr_barcode_result_temp.codeword_error_count;
		ex_validation.barcode_result.barcode_2d_character_length = bill_info->qr_barcode_result_temp.character_length;
		memcpy(&ex_validation.barcode_result.barcode_2d_characters, bill_info->qr_barcode_result_temp.characters, sizeof(ex_validation.barcode_result.barcode_2d_characters));
#endif

		return;
	}
}


void qr_barcode_find_bottom_top_edges()
{
	const int edge_white_area_pixel_count = (int)(BARCODE_2D_WHITE_EDGE_AREA_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);
	const int min_cell_pixel_size = (int)(QR_BARCODE_MIN_CELL_SIZE_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);
	const int max_cell_pixel_size = (int)(QR_BARCODE_MAX_CELL_SIZE_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);

	bill_info->qr_barcode_result_temp.bottom_edge = get_2d_barcode_bottom_edge(&bill_info->qr_barcode_result_temp,
		BARCODE_2D_EDGE_LEVEL, edge_white_area_pixel_count);

	bill_info->qr_barcode_result_temp.top_edge = get_2d_barcode_top_edge(&bill_info->qr_barcode_result_temp,
		BARCODE_2D_EDGE_LEVEL, edge_white_area_pixel_count);

	bill_info->qr_barcode_result_temp.pixel_height = bill_info->qr_barcode_result_temp.top_edge
		- bill_info->qr_barcode_result_temp.bottom_edge + 1;

	//check result
	if(bill_info->qr_barcode_result_temp.pixel_height < QR_BARCODE_MIN_CELLS * min_cell_pixel_size
		|| bill_info->qr_barcode_result_temp.pixel_height > QR_BARCODE_MAX_CELLS * max_cell_pixel_size)
	{
		bill_info->qr_barcode_result_temp.error_code = QR_BARCODE_RESULT_EDGE_ERROR;
	}
}


void qr_barcode_find_left_right_edges()
{
	const int edge_white_area_pixel_count = (int)(BARCODE_2D_WHITE_EDGE_AREA_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);
	const int min_cell_pixel_size = (int)(QR_BARCODE_MIN_CELL_SIZE_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);
	const int max_cell_pixel_size = (int)(QR_BARCODE_MAX_CELL_SIZE_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);

	bill_info->qr_barcode_result_temp.left_edge = get_2d_barcode_left_edge(&bill_info->qr_barcode_result_temp,
		BARCODE_2D_EDGE_LEVEL, edge_white_area_pixel_count);

	bill_info->qr_barcode_result_temp.right_edge = get_2d_barcode_right_edge(&bill_info->qr_barcode_result_temp,
		BARCODE_2D_EDGE_LEVEL, edge_white_area_pixel_count);

	bill_info->qr_barcode_result_temp.pixel_width = bill_info->qr_barcode_result_temp.right_edge
		- bill_info->qr_barcode_result_temp.left_edge + 1;

	//check result
	if(bill_info->qr_barcode_result_temp.pixel_width < QR_BARCODE_MIN_CELLS * min_cell_pixel_size
		|| bill_info->qr_barcode_result_temp.pixel_width > QR_BARCODE_MAX_CELLS * max_cell_pixel_size)
	{
		bill_info->qr_barcode_result_temp.error_code = QR_BARCODE_RESULT_EDGE_ERROR;
	}
}


void qr_barcode_search_border()
{
	const int min_cell_pixel_size = (int)(QR_BARCODE_MIN_CELL_SIZE_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);
	const int max_cell_pixel_size = (int)(QR_BARCODE_MAX_CELL_SIZE_MM / bill_info->sensor_info[bill_info->qr_barcode_result_temp.sensor].pitch);

	int size_count = 0;

	int top_left_black_total = 0;
	int top_left_white_total = 0;
	int top_right_black_total = 0;
	int top_right_white_total = 0;
	int bottom_left_black_total = 0;
	int bottom_left_white_total = 0;
	int bottom_right_black_total = 0;
	int bottom_right_white_total = 0;

	int top_left_black_count = 0;
	int top_left_white_count = 0;
	int top_right_black_count = 0;
	int top_right_white_count = 0;
	int bottom_left_black_count = 0;
	int bottom_left_white_count = 0;
	int bottom_right_black_count = 0;
	int bottom_right_white_count = 0;

	int top_left_found = FALSE;
	int top_right_found = FALSE;
	int bottom_left_found = FALSE;
	int bottom_right_found = FALSE;

	int x_cell_count = 0;
	int y_cell_count = 0;

	int x_cell_mapped = 0;
	int y_cell_mapped = 0;

	int width_cell_count = 0;
	int height_cell_count = 0;

	while(qr_info[size_count].width_size != 0) //search through the supported sizes
	{
		width_cell_count = qr_info[size_count].width_size;
		height_cell_count = qr_info[size_count].height_size;

		bill_info->qr_barcode_result_temp.orientation = BARCODE_2D_ORIENTATION_1;

		bill_info->qr_barcode_result_temp.rotated_pixel_width = bill_info->qr_barcode_result_temp.pixel_width;
		bill_info->qr_barcode_result_temp.rotated_pixel_height = bill_info->qr_barcode_result_temp.pixel_height;
		bill_info->qr_barcode_result_temp.rotated_width_cell_count = width_cell_count;
		bill_info->qr_barcode_result_temp.rotated_height_cell_count = height_cell_count;
		bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width = (float)bill_info->qr_barcode_result_temp.pixel_width / (float)width_cell_count;
		bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height = (float)bill_info->qr_barcode_result_temp.pixel_height / (float)height_cell_count;

		//check size of cells in pixels
		if(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width < min_cell_pixel_size
			|| bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width > max_cell_pixel_size)
		{
			size_count++;

			continue;
		}

		if(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height < min_cell_pixel_size
			|| bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height > max_cell_pixel_size)
		{
			size_count++;

			continue;
		}

		top_left_black_total = 0;
		top_left_white_total = 0;
		top_right_black_total = 0;
		top_right_white_total = 0;
		bottom_left_black_total = 0;
		bottom_left_white_total = 0;
		bottom_right_black_total = 0;
		bottom_right_white_total = 0;

		top_left_black_count = 0;
		top_left_white_count = 0;
		top_right_black_count = 0;
		top_right_white_count = 0;
		bottom_left_black_count = 0;
		bottom_left_white_count = 0;
		bottom_right_black_count = 0;
		bottom_right_white_count = 0;

		top_left_found = FALSE;
		top_right_found = FALSE;
		bottom_left_found = FALSE;
		bottom_right_found = FALSE;

		//check top left corner
		for(x_cell_count = 0; x_cell_count < QR_BARCODE_FINDER_AREA_SIZE; x_cell_count++)
		{
			for(y_cell_count = height_cell_count - QR_BARCODE_FINDER_AREA_SIZE; y_cell_count < height_cell_count; y_cell_count++)
			{
				//don't search the very edge to speed up
				if((x_cell_count == 0)
					|| (x_cell_count == width_cell_count - 1)
					|| (y_cell_count == 0)
					|| (y_cell_count == height_cell_count - 1))
				{
					continue;
				}

				x_cell_mapped = x_cell_count;
				y_cell_mapped = height_cell_count - 1 - y_cell_count;

				if((x_cell_mapped == 7)
					|| (y_cell_mapped == 7)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 1)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 5)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 1)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 5))
				{
					top_left_white_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					top_left_white_count++;
				}
				else
				{
					top_left_black_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					top_left_black_count++;
				}
			}
		}

		//check top right corner
		for(x_cell_count = height_cell_count - QR_BARCODE_FINDER_AREA_SIZE; x_cell_count < height_cell_count; x_cell_count++)
		{
			for(y_cell_count = height_cell_count - QR_BARCODE_FINDER_AREA_SIZE; y_cell_count < height_cell_count; y_cell_count++)
			{
				//don't search the very edge to speed up
				if((x_cell_count == 0)
					|| (x_cell_count == width_cell_count - 1)
					|| (y_cell_count == 0)
					|| (y_cell_count == height_cell_count - 1))
				{
					continue;
				}

				x_cell_mapped = height_cell_count - 1 - x_cell_count;
				y_cell_mapped = height_cell_count - 1 - y_cell_count;

				if((x_cell_mapped == 7)
					|| (y_cell_mapped == 7)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 1)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 5)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 1)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 5))
				{
					top_right_white_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					top_right_white_count++;
				}
				else
				{
					top_right_black_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					top_right_black_count++;
				}
			}
		}

		//check bottom left corner
		for(x_cell_count = 0; x_cell_count < QR_BARCODE_FINDER_AREA_SIZE; x_cell_count++)
		{
			for(y_cell_count = 0; y_cell_count < QR_BARCODE_FINDER_AREA_SIZE; y_cell_count++)
			{
				//don't search the very edge to speed up
				if((x_cell_count == 0)
					|| (x_cell_count == width_cell_count - 1)
					|| (y_cell_count == 0)
					|| (y_cell_count == height_cell_count - 1))
				{
					continue;
				}

				x_cell_mapped = x_cell_count;
				y_cell_mapped = y_cell_count;

				if((x_cell_mapped == 7)
					|| (y_cell_mapped == 7)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 1)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 5)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 1)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 5))
				{
					bottom_left_white_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					bottom_left_white_count++;
				}
				else
				{
					bottom_left_black_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					bottom_left_black_count++;
				}
			}
		}

		//check bottom right corner
		for(x_cell_count = height_cell_count - QR_BARCODE_FINDER_AREA_SIZE; x_cell_count < height_cell_count; x_cell_count++)
		{
			for(y_cell_count = 0; y_cell_count < QR_BARCODE_FINDER_AREA_SIZE; y_cell_count++)
			{
				//don't search the very edge to speed up
				if((x_cell_count == 0)
					|| (x_cell_count == width_cell_count - 1)
					|| (y_cell_count == 0)
					|| (y_cell_count == height_cell_count - 1))
				{
					continue;
				}

				x_cell_mapped = height_cell_count - 1 - x_cell_count;
				y_cell_mapped = y_cell_count;

				if((x_cell_mapped == 7)
					|| (y_cell_mapped == 7)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 1)
					|| (x_cell_mapped >= 1 && x_cell_mapped <= 5 && y_cell_mapped == 5)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 1)
					|| (y_cell_mapped >= 1 && y_cell_mapped <= 5 && x_cell_mapped == 5))
				{
					bottom_right_white_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					bottom_right_white_count++;
				}
				else
				{
					bottom_right_black_total += get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp,
						x_cell_count, y_cell_count);
					bottom_right_black_count++;
				}
			}
		}

		top_left_black_total /= top_left_black_count;
		top_left_white_total /= top_left_white_count;
		top_right_black_total /= top_right_black_count;
		top_right_white_total /= top_right_white_count;
		bottom_left_black_total /= bottom_left_black_count;
		bottom_left_white_total /= bottom_left_white_count;
		bottom_right_black_total /= bottom_right_black_count;
		bottom_right_white_total /= bottom_right_white_count;

		if(top_left_black_total < QR_BARCODE_FINDER_BLACK_LIMIT && top_left_white_total > QR_BARCODE_FINDER_WHITE_LIMIT)
		{
			top_left_found = TRUE;
		}

		if(top_right_black_total < QR_BARCODE_FINDER_BLACK_LIMIT && top_right_white_total > QR_BARCODE_FINDER_WHITE_LIMIT)
		{
			top_right_found = TRUE;
		}

		if(bottom_left_black_total < QR_BARCODE_FINDER_BLACK_LIMIT && bottom_left_white_total > QR_BARCODE_FINDER_WHITE_LIMIT)
		{
			bottom_left_found = TRUE;
		}

		if(bottom_right_black_total < QR_BARCODE_FINDER_BLACK_LIMIT && bottom_right_white_total > QR_BARCODE_FINDER_WHITE_LIMIT)
		{
			bottom_right_found = TRUE;
		}

		if(top_left_found == TRUE && top_right_found == TRUE && bottom_left_found == TRUE && bottom_right_found == TRUE)
		{
			//error case do nothing
		}
		else if(top_left_found == TRUE && top_right_found == TRUE && bottom_left_found == TRUE && bottom_right_found == FALSE)
		{
			bill_info->qr_barcode_result_temp.orientation = BARCODE_2D_ORIENTATION_1;
			bill_info->qr_barcode_result_temp.size_version = size_count;

			bill_info->qr_barcode_result_temp.black_white_average = top_left_black_total + top_left_white_total + top_right_black_total + top_right_white_total
				+ bottom_left_black_total + bottom_left_white_total;

			bill_info->qr_barcode_result_temp.black_white_average /= 6;

			return;
		}
		else if(top_left_found == TRUE && top_right_found == TRUE && bottom_left_found == FALSE && bottom_right_found == TRUE)
		{
			bill_info->qr_barcode_result_temp.orientation = BARCODE_2D_ORIENTATION_2;
			bill_info->qr_barcode_result_temp.size_version = size_count;

			bill_info->qr_barcode_result_temp.rotated_pixel_width = bill_info->qr_barcode_result_temp.pixel_height;
			bill_info->qr_barcode_result_temp.rotated_pixel_height = bill_info->qr_barcode_result_temp.pixel_width;
			bill_info->qr_barcode_result_temp.rotated_width_cell_count = height_cell_count;
			bill_info->qr_barcode_result_temp.rotated_height_cell_count = width_cell_count;
			bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width = (float)bill_info->qr_barcode_result_temp.pixel_height / (float)height_cell_count;
			bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height = (float)bill_info->qr_barcode_result_temp.pixel_width / (float)width_cell_count;

			bill_info->qr_barcode_result_temp.black_white_average = top_left_black_total + top_left_white_total + top_right_black_total + top_right_white_total
				+ bottom_right_black_total + bottom_right_white_total;

			bill_info->qr_barcode_result_temp.black_white_average /= 6;

			bill_info->qr_barcode_result_temp.black_white_average = bill_info->qr_barcode_result_temp.black_white_average
				* QR_BARCODE_BLACK_WHITE_AVERAGE / 100;

			return;
		}
		else if(top_left_found == FALSE && top_right_found == TRUE && bottom_left_found == TRUE && bottom_right_found == TRUE)
		{
			bill_info->qr_barcode_result_temp.orientation = BARCODE_2D_ORIENTATION_3;
			bill_info->qr_barcode_result_temp.size_version = size_count;

			bill_info->qr_barcode_result_temp.black_white_average = top_right_black_total + top_right_white_total + bottom_left_black_total + bottom_left_white_total
				+ bottom_right_black_total + bottom_right_white_total;

			bill_info->qr_barcode_result_temp.black_white_average /= 6;

			bill_info->qr_barcode_result_temp.black_white_average = bill_info->qr_barcode_result_temp.black_white_average
				* QR_BARCODE_BLACK_WHITE_AVERAGE / 100;

			return;
		}
		else if(top_left_found == TRUE && top_right_found == FALSE && bottom_left_found == TRUE && bottom_right_found == TRUE)
		{
			bill_info->qr_barcode_result_temp.orientation = BARCODE_2D_ORIENTATION_4;
			bill_info->qr_barcode_result_temp.size_version = size_count;

			bill_info->qr_barcode_result_temp.rotated_pixel_width = bill_info->qr_barcode_result_temp.pixel_height;
			bill_info->qr_barcode_result_temp.rotated_pixel_height = bill_info->qr_barcode_result_temp.pixel_width;
			bill_info->qr_barcode_result_temp.rotated_width_cell_count = height_cell_count;
			bill_info->qr_barcode_result_temp.rotated_height_cell_count = width_cell_count;
			bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width = (float)bill_info->qr_barcode_result_temp.pixel_height / (float)height_cell_count;
			bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height = (float)bill_info->qr_barcode_result_temp.pixel_width / (float)width_cell_count;

			bill_info->qr_barcode_result_temp.black_white_average = top_left_black_total + top_left_white_total + bottom_left_black_total + bottom_left_white_total
				+ bottom_right_black_total + bottom_right_white_total;

			bill_info->qr_barcode_result_temp.black_white_average /= 6;

			bill_info->qr_barcode_result_temp.black_white_average = bill_info->qr_barcode_result_temp.black_white_average
				* QR_BARCODE_BLACK_WHITE_AVERAGE / 100;

			return;
		}

		size_count++;
	}

	//clear values if not successful
	bill_info->qr_barcode_result_temp.rotated_pixel_width = 0;
	bill_info->qr_barcode_result_temp.rotated_pixel_height = 0;
	bill_info->qr_barcode_result_temp.rotated_width_cell_count = 0;
	bill_info->qr_barcode_result_temp.rotated_height_cell_count = 0;
	bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width = 0.0;
	bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height = 0.0;

	bill_info->qr_barcode_result_temp.orientation = BARCODE_2D_ORIENTATION_UNKNOWN;
}


void qr_barcode_search_format()
{
	int format1 = 0;
	int format2 = 0;
	int format1_decoded = 0;
	int format2_decoded = 0;

	int format1_is_correct = FALSE;
	int format2_is_correct = FALSE;

	//get first format information
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 1);
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 2) << 1;
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 3) << 2;
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 4) << 3;
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 5) << 4;
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 6) << 5;
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 8) << 6;
	format1 |= qr_barcode_get_cell_value(8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 7;
	format1 |= qr_barcode_get_cell_value(7, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 8;
	format1 |= qr_barcode_get_cell_value(5, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 9;
	format1 |= qr_barcode_get_cell_value(4, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 10;
	format1 |= qr_barcode_get_cell_value(3, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 11;
	format1 |= qr_barcode_get_cell_value(2, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 12;
	format1 |= qr_barcode_get_cell_value(1, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 13;
	format1 |= qr_barcode_get_cell_value(0, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 14;

	//get second format information
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 1, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9);
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 2, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 1;
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 3, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 2;
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 4, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 3;
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 5, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 4;
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 6, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 5;
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 7, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 6;
	format2 |= qr_barcode_get_cell_value(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 8, bill_info->qr_barcode_result_temp.rotated_height_cell_count - 9) << 7;
	format2 |= qr_barcode_get_cell_value(8, 6) << 8;
	format2 |= qr_barcode_get_cell_value(8, 5) << 9;
	format2 |= qr_barcode_get_cell_value(8, 4) << 10;
	format2 |= qr_barcode_get_cell_value(8, 3) << 11;
	format2 |= qr_barcode_get_cell_value(8, 2) << 12;
	format2 |= qr_barcode_get_cell_value(8, 1) << 13;
	format2 |= qr_barcode_get_cell_value(8, 0) << 14;

	//unmaks the format information
	format1 ^= QR_BARCODE_FORMAT_MASK;
	format2 ^= QR_BARCODE_FORMAT_MASK;

	//introduce error for testing
	//format1 = 1;
	//format2 = 10197;

	format1_is_correct = qr_barcode_format_is_correct(format1);
	format2_is_correct = qr_barcode_format_is_correct(format2);

	if(format1_is_correct == TRUE)
	{
		//remove error correction
		format1 >>= 10;

		bill_info->qr_barcode_result_temp.error_correction_level = (format1 & 0x0018) >> 3; //11000b

		bill_info->qr_barcode_result_temp.mask_pattern = format1 & 0x0007; //00111b
	}
	else if(format2_is_correct == TRUE)
	{
		format2 >>= 10;

		bill_info->qr_barcode_result_temp.error_correction_level = (format2 & 0x0018) >> 3; //11000b

		bill_info->qr_barcode_result_temp.mask_pattern = format2 & 0x0007; //00111b
	}
	else //case that both format values are not correct
	{
		format1_decoded = qr_barcode_decode_format(format1);
		format2_decoded = qr_barcode_decode_format(format2);

		if(format1_decoded == format2_decoded
			&& format1_decoded != -1)
		{
			bill_info->qr_barcode_result_temp.error_correction_level = (format1_decoded & 0x0018) >> 3; //11000b

			bill_info->qr_barcode_result_temp.mask_pattern = format1_decoded & 0x0007; //00111b
		}
		else
		{
			bill_info->qr_barcode_result_temp.error_code = QR_BARCODE_RESULT_FORMAT_ERROR;
        }
	}

	//determine the data and correction codewords based on the error correction level
	if(bill_info->qr_barcode_result_temp.error_code == 0)
	{
		switch(bill_info->qr_barcode_result_temp.error_correction_level)
		{
			case QR_BARCODE_ERROR_CORRECTION_LOW:
				bill_info->qr_barcode_result_temp.correction_codeword_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_low;
				bill_info->qr_barcode_result_temp.codewords.length
					= qr_info[bill_info->qr_barcode_result_temp.size_version].data_codeword_count_low
						+ qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_low;
				bill_info->qr_barcode_result_temp.rs_block_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].rs_block_count_low;
				break;
			case QR_BARCODE_ERROR_CORRECTION_MEDIUM:
				bill_info->qr_barcode_result_temp.correction_codeword_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_medium;
				bill_info->qr_barcode_result_temp.codewords.length
					= qr_info[bill_info->qr_barcode_result_temp.size_version].data_codeword_count_medium
						+ qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_medium;
				bill_info->qr_barcode_result_temp.rs_block_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].rs_block_count_medium;
				break;
			case QR_BARCODE_ERROR_CORRECTION_QUALITY:
				bill_info->qr_barcode_result_temp.correction_codeword_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_quality;
				bill_info->qr_barcode_result_temp.codewords.length
					= qr_info[bill_info->qr_barcode_result_temp.size_version].data_codeword_count_quality
						+ qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_quality;
				bill_info->qr_barcode_result_temp.rs_block_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].rs_block_count_quality;
				break;
			case QR_BARCODE_ERROR_CORRECTION_HIGH:
				bill_info->qr_barcode_result_temp.correction_codeword_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_high;
				bill_info->qr_barcode_result_temp.codewords.length
					= qr_info[bill_info->qr_barcode_result_temp.size_version].data_codeword_count_high
						+ qr_info[bill_info->qr_barcode_result_temp.size_version].correction_codeword_count_high;
				bill_info->qr_barcode_result_temp.rs_block_count
					= qr_info[bill_info->qr_barcode_result_temp.size_version].rs_block_count_high;
				break;
			default:

				break;
		}
	}
}


int qr_barcode_format_is_correct(int format)
{
	if(format == 0)
	{
        return FALSE;
    }

	if(qr_barcode_check_format(format) == 0)
	{
		return TRUE;
	}

	return FALSE;
}


int qr_barcode_check_format(int format)
{
	int count = 0;

	for(count = 4; count >= 0; count--)
	{
		if(format & (1 << (count + 10)))
		{
			format ^= QR_BARCODE_FORMAT_GENERATOR << count;
		}
	}

	return format;
}


int qr_barcode_get_hamming_weight(int value)
{
	int result = 0;

	while(value > 0)
	{
		result += value & 1;

		value >>= 1;
	}

	return result;
}


int qr_barcode_decode_format(int format)
{
	int best_format = -1;
	int best_distance = 15;

	int test_code = 0;
	int test_distance = 0;

	int count = 0;

	for(count = 0; count < 32; count++)
	{
		test_code = (count << 10) ^ qr_barcode_check_format(count << 10);
		test_distance = qr_barcode_get_hamming_weight(format ^ test_code);

		if(test_distance < best_distance)
		{
			best_distance = test_distance;
			best_format = count;
		}
		else if(test_distance == best_distance)
		{
			best_format = -1;
		}
	}

	return best_format;
}


int qr_barcode_get_mask_value(const int x_cell, const int y_cell)
{
	int result = 0;

	int column = x_cell;
	int row = bill_info->qr_barcode_result_temp.rotated_height_cell_count - 1 - y_cell;

	switch(bill_info->qr_barcode_result_temp.mask_pattern)
	{
		case QR_BARCODE_MASK_0:
			if((row + column) % 2 == 0)
			{
				result = 1;
			}

			break;
		case QR_BARCODE_MASK_1:
			if(row % 2 == 0)
			{
				result = 1;
			}

			break;
		case QR_BARCODE_MASK_2:
			if(column % 3 == 0)
			{
				result = 1;
			}

			break;
		case QR_BARCODE_MASK_3:
			if((row + column) % 3 == 0)
			{
				result = 1;
			}

			break;
		case QR_BARCODE_MASK_4:
			if((row / 2 + column / 3) % 2 == 0)
			{
				result = 1;
			}

			break;
		case QR_BARCODE_MASK_5:
			if((row * column) % 2 + (row * column) % 3 == 0)
			{
				result = 1;
			}

			break;
		case QR_BARCODE_MASK_6:
			if(((row * column) % 2 + (row * column) % 3) % 2 == 0)
			{
				result = 1;
			}

			break;
		case QR_BARCODE_MASK_7:
			if(((row + column) % 2 + (row * column) % 3) % 2 == 0)
			{
				result = 1;
			}

			break;
		default:

			break;
	}

	return result;
}


void qr_barcode_search_cells()
{
	const int border_offset_pixels = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width
		+ bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height) / 2;

	const int transition_white = bill_info->qr_barcode_result_temp.black_white_average;
	const int transition_black = bill_info->qr_barcode_result_temp.black_white_average;

	int pixel_count = 0;

	int average = 0;
	int next_average = 0;

	int current_area = 0;

	int x_cells_found = 7;
	int y_cells_found = 7;

	//find width cell start points
	current_area = BARCODE_2D_BLACK_AREA;

	//add beginning edge line
	bill_info->qr_barcode_result_temp.cell_start_x[0] = 0;

	const int y_search_point = (int)(((float)bill_info->qr_barcode_result_temp.rotated_height_cell_count - 7.0 + 0.5) * bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height);

	for(pixel_count = border_offset_pixels; pixel_count <= bill_info->qr_barcode_result_temp.rotated_pixel_width - border_offset_pixels - 1; pixel_count++)
	{
		average = (get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, pixel_count, y_search_point)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, pixel_count, y_search_point - 1)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, pixel_count, y_search_point + 1)) / 3;

		next_average = (get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, pixel_count + 1, y_search_point)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, pixel_count + 1, y_search_point - 1)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, pixel_count + 1, y_search_point + 1)) / 3;

		if(current_area == BARCODE_2D_BLACK_AREA) //black area
		{
			if(average > transition_white && next_average > transition_white)
			{
				current_area = BARCODE_2D_WHITE_AREA;

				bill_info->qr_barcode_result_temp.cell_start_x[x_cells_found] = pixel_count;

				x_cells_found++;
				pixel_count++;
			}
		}
		else if(current_area == BARCODE_2D_WHITE_AREA)
		{
			if(average < transition_black && next_average < transition_black)
			{
				current_area = BARCODE_2D_BLACK_AREA;

				bill_info->qr_barcode_result_temp.cell_start_x[x_cells_found] = pixel_count;

				x_cells_found++;
				pixel_count++;
			}
		}
	}

	//add end edge line
	bill_info->qr_barcode_result_temp.cell_start_x[bill_info->qr_barcode_result_temp.rotated_width_cell_count]
		= bill_info->qr_barcode_result_temp.rotated_pixel_width + 1;

	//add left side start points
	bill_info->qr_barcode_result_temp.cell_start_x[1] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * 1.0);
	bill_info->qr_barcode_result_temp.cell_start_x[2] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * 2.0);
	bill_info->qr_barcode_result_temp.cell_start_x[3] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * 3.0);
	bill_info->qr_barcode_result_temp.cell_start_x[4] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * 4.0);
	bill_info->qr_barcode_result_temp.cell_start_x[5] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * 5.0);
	bill_info->qr_barcode_result_temp.cell_start_x[6] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * 6.0);

	//add right side start points
	bill_info->qr_barcode_result_temp.cell_start_x[bill_info->qr_barcode_result_temp.rotated_width_cell_count - 1]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 1));
	bill_info->qr_barcode_result_temp.cell_start_x[bill_info->qr_barcode_result_temp.rotated_width_cell_count - 2]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 2));
	bill_info->qr_barcode_result_temp.cell_start_x[bill_info->qr_barcode_result_temp.rotated_width_cell_count - 3]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 3));
	bill_info->qr_barcode_result_temp.cell_start_x[bill_info->qr_barcode_result_temp.rotated_width_cell_count - 4]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 4));
	bill_info->qr_barcode_result_temp.cell_start_x[bill_info->qr_barcode_result_temp.rotated_width_cell_count - 5]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 5));
	bill_info->qr_barcode_result_temp.cell_start_x[bill_info->qr_barcode_result_temp.rotated_width_cell_count - 6]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_width_cell_count - 6));

	//find height cell start points
	current_area = BARCODE_2D_BLACK_AREA;

	//add beginning edge line
	bill_info->qr_barcode_result_temp.cell_start_y[0] = 0;

	const int x_search_point = (int)((6.0 + 0.5) * bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width);

	for(pixel_count = border_offset_pixels; pixel_count <= bill_info->qr_barcode_result_temp.rotated_pixel_height - border_offset_pixels - 1; pixel_count++)
	{
		average = (get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, x_search_point, pixel_count)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, x_search_point - 1, pixel_count)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, x_search_point + 1, pixel_count)) / 3;

		next_average = (get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, x_search_point, pixel_count + 1)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, x_search_point - 1, pixel_count + 1)
			+ get_2d_barcode_rotated_pixel(&bill_info->qr_barcode_result_temp, x_search_point + 1, pixel_count + 1)) / 3;

		if(current_area == BARCODE_2D_BLACK_AREA) //black area
		{
			if(average > transition_white && next_average > transition_white)
			{
				current_area = BARCODE_2D_WHITE_AREA;

				bill_info->qr_barcode_result_temp.cell_start_y[y_cells_found] = pixel_count;

				y_cells_found++;
				pixel_count++;
			}
		}
		else if(current_area == BARCODE_2D_WHITE_AREA)
		{
			if(average < transition_black && next_average < transition_black)
			{
				current_area = BARCODE_2D_BLACK_AREA;

				bill_info->qr_barcode_result_temp.cell_start_y[y_cells_found] = pixel_count;

				y_cells_found++;
				pixel_count++;
			}
		}
	}

	//add end edge line
	bill_info->qr_barcode_result_temp.cell_start_y[bill_info->qr_barcode_result_temp.rotated_height_cell_count]
		= bill_info->qr_barcode_result_temp.rotated_pixel_height + 1;

	//add bottom side start points
	bill_info->qr_barcode_result_temp.cell_start_y[1] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height * 1.0);
	bill_info->qr_barcode_result_temp.cell_start_y[2] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height * 2.0);
	bill_info->qr_barcode_result_temp.cell_start_y[3] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height * 3.0);
	bill_info->qr_barcode_result_temp.cell_start_y[4] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height * 4.0);
	bill_info->qr_barcode_result_temp.cell_start_y[5] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height * 5.0);
	bill_info->qr_barcode_result_temp.cell_start_y[6] = (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_height * 6.0);

	//add top side start points
	bill_info->qr_barcode_result_temp.cell_start_y[bill_info->qr_barcode_result_temp.rotated_height_cell_count - 1]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_height_cell_count - 1));
	bill_info->qr_barcode_result_temp.cell_start_y[bill_info->qr_barcode_result_temp.rotated_height_cell_count - 2]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_height_cell_count - 2));
	bill_info->qr_barcode_result_temp.cell_start_y[bill_info->qr_barcode_result_temp.rotated_height_cell_count - 3]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_height_cell_count - 3));
	bill_info->qr_barcode_result_temp.cell_start_y[bill_info->qr_barcode_result_temp.rotated_height_cell_count - 4]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_height_cell_count - 4));
	bill_info->qr_barcode_result_temp.cell_start_y[bill_info->qr_barcode_result_temp.rotated_height_cell_count - 5]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_height_cell_count - 5));
	bill_info->qr_barcode_result_temp.cell_start_y[bill_info->qr_barcode_result_temp.rotated_height_cell_count - 6]
		= (int)(bill_info->qr_barcode_result_temp.rotated_pixels_per_cell_width * (float)(bill_info->qr_barcode_result_temp.rotated_height_cell_count - 6));
}


void qr_barcode_read_cells()
{
	int x_cell_count = 0;
	int y_cell_count = 0;

	int codeword_index = 0;
	int bit_shift = 0;

	int table_value = 0;

	const int* const byte_map_pointer = qr_info[bill_info->qr_barcode_result_temp.size_version].byte_map;

	for(x_cell_count = 0; x_cell_count < bill_info->qr_barcode_result_temp.rotated_width_cell_count; x_cell_count++)
	{
		for(y_cell_count = 0; y_cell_count < bill_info->qr_barcode_result_temp.rotated_height_cell_count; y_cell_count++)
		{
			table_value = *(byte_map_pointer + (bill_info->qr_barcode_result_temp.rotated_height_cell_count - y_cell_count - 1)
				* bill_info->qr_barcode_result_temp.rotated_width_cell_count + x_cell_count);

			if(table_value != XX)
			{
				codeword_index = table_value / QR_BARCODE_CELLS_PER_CODEWORD;

				bit_shift = QR_BARCODE_CELLS_PER_CODEWORD - table_value % QR_BARCODE_CELLS_PER_CODEWORD - 1;

				bill_info->qr_barcode_result_temp.codewords.value[codeword_index]
					|= (qr_barcode_get_enhanced_cell_value(x_cell_count, y_cell_count) ^ qr_barcode_get_mask_value(x_cell_count, y_cell_count)) << bit_shift;
			}
		}
	}
}


//determine if a cell is 0 or 1 based on the average of the pixels inside
int qr_barcode_get_cell_value(const int x_cell, const int y_cell)
{
	const int average = get_2d_barcode_cell_average(&bill_info->qr_barcode_result_temp, x_cell, y_cell);

	int result = 0;

	if(average < bill_info->qr_barcode_result_temp.black_white_average)
	{
		result = 1; //black cell
	}
	else
	{
		result = 0; //white cell
	}

	return result;
}


//determine if a cell is 0 or 1 based on the average of the pixels inside
int qr_barcode_get_enhanced_cell_value(const int x_cell, const int y_cell)
{
	const int average = get_2d_barcode_enhanced_cell_average(&bill_info->qr_barcode_result_temp, x_cell, y_cell);

	int result = 0;

	if(average < bill_info->qr_barcode_result_temp.black_white_average)
	{
		result = 1; //black cell
	}
	else
	{
		result = 0; //white cell
	}

	return result;
}


void qr_barcode_decode_message()
{
	int encoding_mode = 0;
	int total_characters = 0;
	int total_groups = 0;

	int group_count = 0;
	int characters_in_current_group = 0;

	int current_bit_index = 0;

	int group_value = 0;

	//find first encoding mode (first 4 bits of first codeword)
	encoding_mode = qr_barcode_get_group_from_polynomial(current_bit_index, 4);
	current_bit_index += 4;
	int data_count = bill_info->qr_barcode_result_temp.codewords.length;
	int data_bit_count = data_count * 8;

	while(current_bit_index < data_bit_count)
	{
		if(encoding_mode == QR_BARCODE_MODE_NUMERIC)
		{
			total_characters = qr_barcode_get_group_from_polynomial(current_bit_index, 10);
			current_bit_index += 10;
			if(current_bit_index > data_bit_count)
			{
				break;
			}

			//decode characters by groups of 3
			if(total_characters % 3 == 0)
			{
				total_groups = total_characters / QR_BARCODE_NUMERIC_CHARACTERS_PER_GROUP;
			}
			else
			{
				total_groups = total_characters / QR_BARCODE_NUMERIC_CHARACTERS_PER_GROUP + 1;
			}

			for(group_count = 0; group_count < total_groups; group_count++)
			{
				if(group_count * QR_BARCODE_NUMERIC_CHARACTERS_PER_GROUP <=  total_characters - QR_BARCODE_NUMERIC_CHARACTERS_PER_GROUP)
				{
					characters_in_current_group = QR_BARCODE_NUMERIC_CHARACTERS_PER_GROUP;
				}
				else
				{
					characters_in_current_group = total_characters - group_count * QR_BARCODE_NUMERIC_CHARACTERS_PER_GROUP;
				}

				if(characters_in_current_group == 3) //represented by 10 bits
				{
					group_value = qr_barcode_get_group_from_polynomial(current_bit_index, QR_BARCODE_NUMERIC_GROUP_SIZE_IN_BITS);
					current_bit_index += QR_BARCODE_NUMERIC_GROUP_SIZE_IN_BITS;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length] = group_value / 100 + ASCII_NUMBER_OFFSET;
					bill_info->qr_barcode_result_temp.character_length++;
					group_value %= 100;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length] = group_value / 10 + ASCII_NUMBER_OFFSET;
					bill_info->qr_barcode_result_temp.character_length++;
					group_value %= 10;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length] = group_value + ASCII_NUMBER_OFFSET;
					bill_info->qr_barcode_result_temp.character_length++;
				}
				else if(characters_in_current_group == 2) //represented by 7 bits
				{
					group_value = qr_barcode_get_group_from_polynomial(current_bit_index, 7);
					current_bit_index += 7;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length] = group_value / 10 + ASCII_NUMBER_OFFSET;
					bill_info->qr_barcode_result_temp.character_length++;
					group_value %= 10;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length] = group_value + ASCII_NUMBER_OFFSET;
					bill_info->qr_barcode_result_temp.character_length++;
				}
				else if(characters_in_current_group == 1) //represented by 4 bits
				{
					group_value = qr_barcode_get_group_from_polynomial(current_bit_index, 4);
					current_bit_index += 4;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length] = group_value + ASCII_NUMBER_OFFSET;
					bill_info->qr_barcode_result_temp.character_length++;
				}
			}

			//get next encoding mode
			encoding_mode = qr_barcode_get_group_from_polynomial(current_bit_index, 4);
			current_bit_index += 4;
		}
		else if(encoding_mode == QR_BARCODE_MODE_ALPHANUMERIC)
		{
			//alphanumeric mode uses 9 bits for character count
			total_characters = qr_barcode_get_group_from_polynomial(current_bit_index, 9);
			current_bit_index += 9;

			//decode characters by groups of 2
			if(total_characters % 2 == 0)
			{
				total_groups = total_characters / QR_BARCODE_ALPHANUMERIC_CHARACTERS_PER_GROUP;
			}
			else
			{
				total_groups = total_characters / QR_BARCODE_ALPHANUMERIC_CHARACTERS_PER_GROUP + 1;
			}

			for(group_count = 0; group_count < total_groups; group_count++)
			{
				if(group_count * QR_BARCODE_ALPHANUMERIC_CHARACTERS_PER_GROUP <= total_characters - QR_BARCODE_ALPHANUMERIC_CHARACTERS_PER_GROUP)
				{
					characters_in_current_group = QR_BARCODE_ALPHANUMERIC_CHARACTERS_PER_GROUP;
				}
				else
				{
					characters_in_current_group = total_characters - group_count * QR_BARCODE_ALPHANUMERIC_CHARACTERS_PER_GROUP;
				}

				if(characters_in_current_group == 2) //represented by 11 bits
				{
					group_value = qr_barcode_get_group_from_polynomial(current_bit_index, QR_BARCODE_ALPHANUMERIC_GROUP_SIZE_IN_BITS);
					current_bit_index += QR_BARCODE_ALPHANUMERIC_GROUP_SIZE_IN_BITS;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length]
						= qr_alphanumeric_table[group_value / QR_BARCODE_ALPHANUMERIC_TABLE_SIZE];
					bill_info->qr_barcode_result_temp.character_length++;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length]
						= qr_alphanumeric_table[group_value % QR_BARCODE_ALPHANUMERIC_TABLE_SIZE];
					bill_info->qr_barcode_result_temp.character_length++;
				}
				else if(characters_in_current_group == 1) //represented by 6 bits
				{
					group_value = qr_barcode_get_group_from_polynomial(current_bit_index, 6);
					current_bit_index += 6;

					bill_info->qr_barcode_result_temp.characters[bill_info->qr_barcode_result_temp.character_length] = qr_alphanumeric_table[group_value];
					bill_info->qr_barcode_result_temp.character_length++;
				}
			}

			//get next encoding mode
			encoding_mode = qr_barcode_get_group_from_polynomial(current_bit_index, 4);
			current_bit_index += 4;
		}
		else if(encoding_mode == QR_BARCODE_MODE_BYTE)
		{
			bill_info->qr_barcode_result_temp.error_code = QR_BARCODE_RESULT_DECODE_ERROR;

			return;
		}
		else if(encoding_mode == QR_BARCODE_MODE_NONE) //finished decoding
		{
			return;
		}
		else //unsupported encoding mode case
		{
			bill_info->qr_barcode_result_temp.error_code = QR_BARCODE_RESULT_DECODE_ERROR;

			return;
		}
	}
}


int qr_barcode_get_group_from_polynomial(const int polynomial_bit_index, const int length)
{
	int result = 0;

	int bit_count = 0;

	for(bit_count = 0; bit_count < length; bit_count++)
	{
		result |= qr_barcode_get_bit_from_polynomial(polynomial_bit_index + bit_count) << (length - bit_count - 1);
	}

	return result;
}


int qr_barcode_get_bit_from_polynomial(const int polynomial_bit_index)
{
	const int byte_index = polynomial_bit_index / 8;
	const int bit_index = polynomial_bit_index % 8;

	const int mask = 1 << (7 - bit_index);

	int result = 0;

	result = bill_info->qr_barcode_result_temp.codewords.value[byte_index] & mask;

	if(result)
	{
		result = 1;
	}
	else
	{
        result = 0;
    }

	return result;
}
