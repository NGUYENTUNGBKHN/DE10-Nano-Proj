/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_edge.c
* Contents: edge detection and deskew processing
*
*
*******************************************************************************/
#include "math.h"
#include "string.h"
#include    <float.h>
#include	<stdlib.h>
#include "common.h"

#define EXT
#include "../common/global.h"
#include "tem_global.c"

#include "tem_pixel.h"
#include "tem_edge.h"

void run_edge_routine()
{
	edge_level_check();
	remove_corner_points();
	remove_invalid_slopes();

	find_edge_slope();
	find_linear_regression();

	remove_outliers();
	find_edge_slope();
	find_linear_regression();

	find_skew_values();
	find_raw_corner_coordinates();
	find_deskewed_corner_coordinates();

	find_bill_size();
#if OPTION_ENABLE_CALIBRATION == 1

	find_calibration_areas();
	find_calibration_values();
#endif
}


void edge_level_check()
{

	int x_count = 0;
	int y_count = 0;

	float ratio = 0.0;
	int x_start = 0;
	int x_end = 0;
	int y_start = 0;
	int y_end = 0;
	const int y_pixels = bill_info->sensor_info[EDGE_DETECTION_SENSOR].y_pixels;
	const int x_pixels = bill_info->sensor_info[EDGE_DETECTION_SENSOR].x_pixels;
	const int x_offset = work[0].pbs->PlaneInfo[EDGE_DETECTION_SENSOR].sub_offset;

	//top edge
	// x_start :right_up_y
	// x_end   :right_down_y
	x_start = work[0].pbs->right_up_y;
	x_end = work[0].pbs->right_down_y;
	y_start = work[0].pbs->right_up_x;
	y_end = work[0].pbs->right_down_x;
	ratio = (float)(y_end - y_start)/(x_end + 1 - x_start);
	for(x_count = EDGE_SEARCH_START_X; x_count < x_pixels; x_count += EDGE_PIXEL_SKIP)
	{
		if(x_count > x_end || x_count < x_start)
		{
			continue;
		}
		bill_info->top_edge[x_count] = y_pixels - (y_start + (x_count -  x_start) * ratio);
	}

	//bottom edge
	x_start = work[0].pbs->left_up_y;
	x_end = work[0].pbs->left_down_y;
	y_start = work[0].pbs->left_up_x;
	y_end = work[0].pbs->left_down_x;
	ratio = (float)(y_end - y_start)/(x_end + 1 - x_start);
	for(x_count = EDGE_SEARCH_START_X; x_count < x_pixels; x_count += EDGE_PIXEL_SKIP)
	{
		if(x_count > x_end || x_count < x_start)
		{
			continue;
		}
		bill_info->bottom_edge[x_count] = y_pixels - (y_start + (x_count -  x_start) * ratio);
	}

	//left edge
	x_start = work[0].pbs->right_up_y;
	x_end = work[0].pbs->left_up_y;
	y_start = y_pixels - 1 - work[0].pbs->right_up_x;
	y_end = y_pixels - 1 - work[0].pbs->left_up_x;
	ratio = (float)(x_end - x_start)/(y_end + 1 - y_start);
	for(y_count = EDGE_SEARCH_START_Y; y_count < y_pixels; y_count += EDGE_PIXEL_SKIP)
	{
		if(y_count > y_end || y_count < y_start)
		{
			continue;
		}
		bill_info->left_edge[y_count] = x_offset + x_start + (y_count -  y_start) * ratio;
	}

	//right edge
	x_start = work[0].pbs->right_down_y;
	x_end = work[0].pbs->left_down_y;
	y_start = y_pixels - 1 - work[0].pbs->right_down_x;
	y_end = y_pixels - 1 - work[0].pbs->left_down_x;
	ratio = (float)(x_end - x_start)/(y_end + 1 - y_start);
	for(y_count = EDGE_SEARCH_START_Y; y_count < y_pixels; y_count += EDGE_PIXEL_SKIP)
	{
		if(y_count > y_end || y_count < y_start)
		{
			continue;
		}
		bill_info->right_edge[y_count] = x_offset + x_start + (y_count -  y_start) * ratio;
	}
}


void remove_corner_points()
{
	remove_corner_points_x(bill_info->top_edge);
	remove_corner_points_x(bill_info->bottom_edge);
	remove_corner_points_y(bill_info->left_edge);
	remove_corner_points_y(bill_info->right_edge);
}


void remove_corner_points_x(int data_set[EDGE_PIXEL_COUNT_X])
{
	int data_size = 0;
	int data_to_remove = 0;
	int data_removed = 0;

	int x_count = 0;

	//find the amount of data points in this set
	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[x_count] != 0)
		{
			data_size++;
		}
	}

	data_to_remove = (data_size * CORNER_REMOVE_PERCENT_X) / 100;

	//remove left side corner values
	data_removed = 0;
	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[x_count] != 0)
		{
			data_set[x_count] = 0;
			data_removed++;

			if(data_to_remove == data_removed)
			{
				break;
			}
		}
	}

	//remove right side corner values
	data_removed = 0;
	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[EDGE_PIXEL_COUNT_X - 1 - x_count] != 0)
		{
			data_set[EDGE_PIXEL_COUNT_X - 1 - x_count] = 0;
			data_removed++;

			if(data_to_remove == data_removed)
			{
				break;
			}
		}
	}
}


void remove_corner_points_y(int data_set[EDGE_PIXEL_COUNT_Y])
{
	int data_size = 0;
	int data_to_remove = 0;
	int data_removed = 0;

	int y_count = 0;

	//find the amount of data points in this set
	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[y_count] != 0)
		{
			data_size++;
		}
	}

	data_to_remove = (data_size * CORNER_REMOVE_PERCENT_Y) / 100;

	//remove bottom side corner values
	data_removed = 0;
	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[y_count] != 0)
		{
			data_set[y_count] = 0;
			data_removed++;

			if(data_to_remove == data_removed)
			{
				break;
			}
		}
	}

	//remove top side corner values
	data_removed = 0;
	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[EDGE_PIXEL_COUNT_Y - 1 - y_count] != 0)
		{
			data_set[EDGE_PIXEL_COUNT_Y - 1 - y_count] = 0;
			data_removed++;

			if(data_to_remove == data_removed)
			{
				break;
			}
		}
	}
}


void remove_invalid_slopes()
{
	remove_invalid_slopes_x(bill_info->top_edge);
	remove_invalid_slopes_x(bill_info->bottom_edge);
	remove_invalid_slopes_y(bill_info->left_edge);
	remove_invalid_slopes_y(bill_info->right_edge);
}


void remove_invalid_slopes_x(int data_set[EDGE_PIXEL_COUNT_X])
{
	int x_count = 0;
	int x_count_2 = 0;

	int first_pixel_x = 0;
	int first_pixel_y = 0;

	int second_pixel_x = 0;
	int second_pixel_y = 0;

	int data_size = 0;

	float slope = 0.0;

	int first_pixel_found = FALSE;
	int pixels_removed = FALSE;

	//find the amount of data points in this set
	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[x_count] != 0)
		{
			data_size++;
		}
	}

	for(x_count_2 = 0; x_count_2 < data_size; x_count_2++)
	{
		first_pixel_found = FALSE;

		pixels_removed = FALSE;

		for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
		{
			if(data_set[x_count] != 0 && first_pixel_found == FALSE)
			{
				first_pixel_x = x_count;
				first_pixel_y = data_set[x_count];

				first_pixel_found = TRUE;

				continue;
			}

			if(data_set[x_count] != 0 && first_pixel_found == TRUE)
			{
				second_pixel_x = x_count;
				second_pixel_y = data_set[x_count];

				slope = ((float)second_pixel_y - (float)first_pixel_y) / ((float)second_pixel_x - (float)first_pixel_x);

				if(slope < MIN_POINT_SLOPE_X || slope > MAX_POINT_SLOPE_X)
				{
					//remove these data points
					data_set[first_pixel_x] = 0;
					data_set[second_pixel_x] = 0;

					pixels_removed = TRUE;
				}

				first_pixel_x = second_pixel_x;
				first_pixel_y = second_pixel_y;
			}
		}

		if(pixels_removed == FALSE)
		{
			break;
		}
	}
}


void remove_invalid_slopes_y(int data_set[EDGE_PIXEL_COUNT_Y])
{
	int y_count = 0;
	int y_count_2 = 0;

	int first_pixel_y = 0;
	int first_pixel_x = 0;

	int second_pixel_y = 0;
	int second_pixel_x = 0;

	int data_size = 0;

	float slope = 0.0;

	int first_pixel_found = FALSE;
	int pixels_removed = FALSE;

	//find the amount of data points in this set
	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[y_count] != 0)
		{
			data_size++;
		}
	}

	for(y_count_2 = 0; y_count_2 < data_size; y_count_2++)
	{
		first_pixel_found = FALSE;

		pixels_removed = FALSE;

		for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
		{
			if(data_set[y_count] != 0 && first_pixel_found == FALSE)
			{
				first_pixel_y = y_count;
				first_pixel_x = data_set[y_count];

				first_pixel_found = TRUE;

				continue;
			}

			if(data_set[y_count] != 0 && first_pixel_found == TRUE)
			{
				second_pixel_y = y_count;
				second_pixel_x = data_set[y_count];

				slope = ((float)second_pixel_x - (float)first_pixel_x) / ((float)second_pixel_y - (float)first_pixel_y);

				if(slope < MIN_POINT_SLOPE_X || slope > MAX_POINT_SLOPE_X)
				{
					//remove these data points
					data_set[first_pixel_y] = 0;
					data_set[second_pixel_y] = 0;

					pixels_removed = TRUE;
				}

				first_pixel_y = second_pixel_y;
				first_pixel_x = second_pixel_x;
			}
		}

		if(pixels_removed == FALSE)
		{
			break;
		}
	}
}


void find_edge_slope()
{
	static float slopes[EDGE_MAX_SLOPE_COUNT]; //static for large allocation
	float temp_value = 0;

	int slope_count = 0;

	int item_count1 = 0;
	int item_count2 = 0;

	int lower_percentile_index = 0;
	int upper_percentile_index = 0;

	float min_slope_use_limit = 0.0;
	float max_slope_use_limit = 0.0;

	int used_slopes = 0;

	memset(&slopes, 0, sizeof(slopes));

	//get all slope values
	add_slopes_x(&slope_count, slopes, bill_info->bottom_edge);
	add_slopes_x(&slope_count, slopes, bill_info->top_edge);
	add_slopes_y(&slope_count, slopes, bill_info->left_edge);
	add_slopes_y(&slope_count, slopes, bill_info->right_edge);

	//sort in descending order
	for(item_count1 = 0; item_count1 < slope_count - 1; item_count1++)
	{
		for(item_count2 = 0; item_count2 < slope_count - 1; item_count2++)
		{
			if(slopes[item_count2] > slopes[item_count2 + 1])
			{
				temp_value = slopes[item_count2];
				slopes[item_count2] = slopes[item_count2 + 1];
				slopes[item_count2 + 1] = temp_value;
			}
		}
	}

	//find average of used slopes
	bill_info->final_slope = 0.0;

	lower_percentile_index = slope_count * EDGE_SLOPE_SKIP_PERCENT / 100;
	upper_percentile_index = slope_count * (100 - EDGE_SLOPE_SKIP_PERCENT) / 100;

	min_slope_use_limit = slopes[lower_percentile_index];
	max_slope_use_limit = slopes[upper_percentile_index];

	for(item_count1 = 0; item_count1 < slope_count; item_count1++)
	{
		if(slopes[item_count1] >= min_slope_use_limit
			&& slopes[item_count1] <= max_slope_use_limit)
		{
			bill_info->final_slope += slopes[item_count1];
			used_slopes++;
		}
		else
		{
			int i = 0;
			i++;
		}
	}

	if(used_slopes != 0)
	{
		bill_info->final_slope /= (float)used_slopes;
	}
}


void add_slopes_x(int* const slope_count, float slopes[EDGE_MAX_SLOPE_COUNT], int data_set[EDGE_PIXEL_COUNT_X])
{
	int x_count = 0;

	int first_pixel_x = 0;
	int first_pixel_y = 0;

	int second_pixel_x = 0;
	int second_pixel_y = 0;

	int first_pixel_found = FALSE;

	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[x_count] != 0 && first_pixel_found == FALSE)
		{
			first_pixel_x = x_count;
			first_pixel_y = data_set[x_count];

			first_pixel_found = TRUE;

			continue;
		}

		if(data_set[x_count] != 0 && first_pixel_found == TRUE)
		{
			second_pixel_x = x_count;
			second_pixel_y = data_set[x_count];

			slopes[*slope_count] = ((float)second_pixel_y - (float)first_pixel_y) / ((float)second_pixel_x - (float)first_pixel_x);
			(*slope_count)++;

			first_pixel_x = second_pixel_x;
			first_pixel_y = second_pixel_y;
		}
	}
}


void add_slopes_y(int* const slope_count, float slopes[EDGE_MAX_SLOPE_COUNT], int data_set[EDGE_PIXEL_COUNT_Y])
{
	int y_count = 0;

	int first_pixel_x = 0;
	int first_pixel_y = 0;

	int second_pixel_x = 0;
	int second_pixel_y = 0;

	int first_pixel_found = FALSE;

	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[y_count] != 0 && first_pixel_found == FALSE)
		{
			first_pixel_y = y_count;
			first_pixel_x = data_set[y_count];

			first_pixel_found = TRUE;

			continue;
		}

		if(data_set[y_count] != 0 && first_pixel_found == TRUE)
		{
			second_pixel_y = y_count;
			second_pixel_x = data_set[y_count];

			slopes[*slope_count] = ((float)second_pixel_x - (float)first_pixel_x) / ((float)second_pixel_y - (float)first_pixel_y) * -1.0;
			(*slope_count)++;

			first_pixel_y = second_pixel_y;
			first_pixel_x = second_pixel_x;
		}
	}
}


void find_linear_regression()
{
	find_linear_regression_x(bill_info->top_edge, &bill_info->top_slope, &bill_info->top_y_intercept);
	find_linear_regression_x(bill_info->bottom_edge, &bill_info->bottom_slope, &bill_info->bottom_y_intercept);
	find_linear_regression_y(bill_info->left_edge, &bill_info->left_slope, &bill_info->left_x_intercept);
	find_linear_regression_y(bill_info->right_edge, &bill_info->right_slope, &bill_info->right_x_intercept);
}


//search for the formula y = mx + b
void find_linear_regression_x(int data_set[EDGE_PIXEL_COUNT_X], float* const slope, float* const y_intercept)
{
	float average_x = 0.0;
	float average_y = 0.0;

	int data_size = 0;

	int x_count = 0;

	//find the averages
	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[x_count] != 0)
		{
			average_x += (float)x_count;
			average_y += (float)data_set[x_count];

			data_size++;
		}
	}

	//error data
	if(data_size <= 1)
	{
		return;
	}

	average_x /= (float)data_size;
	average_y /= (float)data_size;

	//find the final slope and intercept
	(*slope) = bill_info->final_slope;
	(*y_intercept) = average_y - (*slope) * average_x;
}


//search for the formula x = my + b
void find_linear_regression_y(int data_set[EDGE_PIXEL_COUNT_Y], float* const slope, float* const x_intercept)
{
	float average_x = 0.0;
	float average_y = 0.0;

	int data_size = 0;

	int y_count = 0;

	//find the averages
	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[y_count] != 0)
		{
			average_x += (float)y_count;
			average_y += (float)data_set[y_count];

			data_size++;
		}
	}

	//error data
	if(data_size <= 1)
	{
		return;
	}

	average_x /= (float)data_size;
	average_y /= (float)data_size;

	//find the final slope and intercept
	(*slope) = bill_info->final_slope * -1.0;
	(*x_intercept) = average_y - (*slope) * average_x;
}


void remove_outliers()
{
	remove_outliers_x(bill_info->top_edge, bill_info->top_slope, bill_info->top_y_intercept);
	remove_outliers_x(bill_info->bottom_edge, bill_info->bottom_slope, bill_info->bottom_y_intercept);
	remove_outliers_y(bill_info->left_edge, bill_info->left_slope, bill_info->left_x_intercept);
	remove_outliers_y(bill_info->right_edge, bill_info->right_slope, bill_info->right_x_intercept);
}


//remove outliers using y = mx + b
void remove_outliers_x(int data_set[EDGE_PIXEL_COUNT_X], const float slope, const float y_intercept)
{
	int data_size = 0;
	int data_to_remove = 0;

	int x_count = 0;
	int remove_count = 0;

	int furthest_outlier_index = 0;
	static int data_set_error[EDGE_PIXEL_COUNT_X]; //static for large allocation

	float current_y_value = 0.0;
	float desired_y_value = 0.0;

	//find the amount of data points in this set
	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[x_count] != 0)
		{
			data_size++;
		}

		data_set_error[x_count] = 0;
	}

	data_to_remove = (data_size * OUTLIER_REMOVE_PERCENT) / 100;

	//find the error associated with each point
	for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
	{
		if(data_set[x_count] != 0)
		{
			current_y_value = (float)data_set[x_count];
			desired_y_value = slope * (float)x_count + y_intercept;

			data_set_error[x_count] = (int)((current_y_value - desired_y_value) * (current_y_value - desired_y_value));
		}
	}

	for(remove_count = 0; remove_count < data_to_remove; remove_count++)
	{
		furthest_outlier_index = 0;

		for(x_count = 0; x_count < EDGE_PIXEL_COUNT_X; x_count++)
		{
			if(data_set_error[x_count] > data_set_error[furthest_outlier_index])
			{
				furthest_outlier_index = x_count;
			}
		}

		data_set[furthest_outlier_index] = 0;
		data_set_error[furthest_outlier_index] = 0;
	}
}


//remove outliers using x = my + b
void remove_outliers_y(int data_set[EDGE_PIXEL_COUNT_Y], const float slope, const float x_intercept)
{
	int data_size = 0;
	int data_to_remove = 0;

	int y_count = 0;
	int remove_count = 0;

	int furthest_outlier_index = 0;
	static int data_set_error[EDGE_PIXEL_COUNT_Y]; //static for large allocation

	float current_x_value = 0.0;
	float desired_x_value = 0.0;

	//find the amount of data points in this set
	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[y_count] != 0)
		{
			data_size++;
		}

		data_set_error[y_count] = 0;
	}

	data_to_remove = (data_size * OUTLIER_REMOVE_PERCENT) / 100;

	//find the error associated with each point
	for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
	{
		if(data_set[y_count] != 0)
		{
			current_x_value = (float)data_set[y_count];
			desired_x_value = slope * (float)y_count + x_intercept;

			data_set_error[y_count] = (int)((current_x_value - desired_x_value) * (current_x_value - desired_x_value));
		}
	}

	for(remove_count = 0; remove_count < data_to_remove; remove_count++)
	{
		furthest_outlier_index = 0;

		for(y_count = 0; y_count < EDGE_PIXEL_COUNT_Y; y_count++)
		{
			if(data_set_error[y_count] > data_set_error[furthest_outlier_index])
			{
				furthest_outlier_index = y_count;
			}
		}

		data_set[furthest_outlier_index] = 0;
		data_set_error[furthest_outlier_index] = 0;
	}
}


void find_raw_corner_coordinates()
{
	float top_left_x = 0.0;
	float top_left_y = 0.0;
	float top_right_x = 0.0;
	float top_right_y = 0.0;
	float bottom_left_x = 0.0;
	float bottom_left_y = 0.0;
	float bottom_right_x = 0.0;
	float bottom_right_y = 0.0;

	float center_x = 0.0;
	float center_y = 0.0;

	float center_offset_x = 0.0;
	float center_offset_y = 0.0;

	//top left corner
	top_left_x = ((bill_info->left_slope * bill_info->top_y_intercept) + bill_info->left_x_intercept)
		/ (1.0 - (bill_info->left_slope * bill_info->top_slope));

	top_left_y = (bill_info->top_slope * top_left_x) + bill_info->top_y_intercept;

	bill_info->raw_top_left_corner.x = (int)(top_left_x + 0.5);
	bill_info->raw_top_left_corner.y = (int)(top_left_y + 0.5);

	//top right corner
	top_right_x = ((bill_info->right_slope * bill_info->top_y_intercept) + bill_info->right_x_intercept)
		/ (1.0 - (bill_info->right_slope * bill_info->top_slope));

	top_right_y = (bill_info->top_slope * top_right_x) + bill_info->top_y_intercept;

	bill_info->raw_top_right_corner.x = (int)(top_right_x + 0.5);
	bill_info->raw_top_right_corner.y = (int)(top_right_y + 0.5);

	//bottom left corner
	bottom_left_x = ((bill_info->left_slope * bill_info->bottom_y_intercept) + bill_info->left_x_intercept)
		/ (1.0 - (bill_info->left_slope * bill_info->bottom_slope));

	bottom_left_y = (bill_info->bottom_slope * bottom_left_x) + bill_info->bottom_y_intercept;

	bill_info->raw_bottom_left_corner.x = (int)(bottom_left_x + 0.5);
	bill_info->raw_bottom_left_corner.y = (int)(bottom_left_y + 0.5);

	//bottom right corner
	bottom_right_x = ((bill_info->right_slope * bill_info->bottom_y_intercept) + bill_info->right_x_intercept)
		/ (1.0 - (bill_info->right_slope * bill_info->bottom_slope));

	bottom_right_y = (bill_info->bottom_slope * bottom_right_x) + bill_info->bottom_y_intercept;

	bill_info->raw_bottom_right_corner.x = (int)(bottom_right_x + 0.5);
	bill_info->raw_bottom_right_corner.y = (int)(bottom_right_y + 0.5);

	//find center pixel
	center_x = (top_left_x + top_right_x + bottom_left_x + bottom_right_x) / 4.0;
	center_y = (top_left_y + top_right_y + bottom_left_y + bottom_right_y) / 4.0;

	bill_info->raw_center.x = (int)(center_x + 0.5);
	bill_info->raw_center.y = (int)(center_y + 0.5);

	//find the difference between the bill center and the image center
	center_offset_x = (center_x - (float)bill_info->sensor_info[EDGE_DETECTION_SENSOR].deskew_center_x);
	center_offset_y = (center_y - (float)(728 - bill_info->sensor_info[EDGE_DETECTION_SENSOR].deskew_center_y));

	bill_info->center_offset_x = (int)(center_offset_x + 0.5);
	bill_info->center_offset_y = (int)(center_offset_y + 0.5);

	//find the offset in millimeters
	bill_info->bill_offset_mm = center_offset_y * PITCH_200DPI;
}


void find_skew_values()
{
	bill_info->skew_radian_x = atan(bill_info->final_slope);
	bill_info->skew_degree_x = bill_info->skew_radian_x * DEGREES_PER_RADIAN;

	bill_info->skew_radian_y = atan(bill_info->final_slope);
	bill_info->skew_degree_y = bill_info->skew_radian_y * DEGREES_PER_RADIAN;

	bill_info->top_skew_sine_correction_x = sin(bill_info->skew_radian_x * -1.0);
	bill_info->top_skew_cosine_correction_x = cos(bill_info->skew_radian_x * -1.0);

	bill_info->bottom_skew_sine_correction_x = sin(bill_info->skew_radian_x * 1.0);
	bill_info->bottom_skew_cosine_correction_x = cos(bill_info->skew_radian_x * 1.0);
}


void find_deskewed_corner_coordinates()
{
	//top left corner
	bill_info->deskewed_top_left_corner.x = bill_info->raw_top_left_corner.x;
	bill_info->deskewed_top_left_corner.y = bill_info->raw_top_left_corner.y;
	find_deskewed_pixel_coordinates(0, &bill_info->deskewed_top_left_corner.x, &bill_info->deskewed_top_left_corner.y);

	//top right corner
	bill_info->deskewed_top_right_corner.x = bill_info->raw_top_right_corner.x;
	bill_info->deskewed_top_right_corner.y = bill_info->raw_top_right_corner.y;
	find_deskewed_pixel_coordinates(0, &bill_info->deskewed_top_right_corner.x, &bill_info->deskewed_top_right_corner.y);

	//bottom left corner
	bill_info->deskewed_bottom_left_corner.x = bill_info->raw_bottom_left_corner.x;
	bill_info->deskewed_bottom_left_corner.y = bill_info->raw_bottom_left_corner.y;
	find_deskewed_pixel_coordinates(0, &bill_info->deskewed_bottom_left_corner.x, &bill_info->deskewed_bottom_left_corner.y);

	//bottom right corner
	bill_info->deskewed_bottom_right_corner.x = bill_info->raw_bottom_right_corner.x;
	bill_info->deskewed_bottom_right_corner.y = bill_info->raw_bottom_right_corner.y;
	find_deskewed_pixel_coordinates(0, &bill_info->deskewed_bottom_right_corner.x, &bill_info->deskewed_bottom_right_corner.y);

	bill_info->deskewed_left_edge = (bill_info->deskewed_top_left_corner.x + bill_info->deskewed_bottom_left_corner.x) / 2;
	bill_info->deskewed_right_edge = (bill_info->deskewed_top_right_corner.x + bill_info->deskewed_bottom_right_corner.x) / 2;

	bill_info->deskewed_top_edge = (bill_info->deskewed_top_left_corner.y + bill_info->deskewed_top_right_corner.y) / 2;
	bill_info->deskewed_bottom_edge = (bill_info->deskewed_bottom_left_corner.y + bill_info->deskewed_bottom_right_corner.y) / 2;
}


void find_bill_size()
{
	bill_info->length_pixel = bill_info->deskewed_right_edge - bill_info->deskewed_left_edge + 1;
	bill_info->width_pixel = bill_info->deskewed_bottom_edge - bill_info->deskewed_top_edge + 1;
	bill_info->length_mm = (float)(bill_info->length_pixel) * PITCH_200DPI;
	bill_info->width_mm = (float)(bill_info->width_pixel) * PITCH_200DPI;

	//length is always off slightly maybe related to data sampling?
	bill_info->length_mm += (PITCH_200DPI * 6);
}


void find_calibration_areas()
{
	//up direction calibration will always use the same square
	bill_info->calibration_up_white_x_start = CALIBRATION_UP_X_START;
	bill_info->calibration_up_white_x_end = CALIBRATION_UP_X_END;
	bill_info->calibration_up_white_y_start = CALIBRATION_UP_WHITE_Y_START;
	bill_info->calibration_up_white_y_end = CALIBRATION_UP_WHITE_Y_END;

	bill_info->calibration_up_black_x_start = CALIBRATION_UP_X_START;
	bill_info->calibration_up_black_x_end = CALIBRATION_UP_X_END;
	bill_info->calibration_up_black_y_start = CALIBRATION_UP_BLACK_Y_START;
	bill_info->calibration_up_black_y_end = CALIBRATION_UP_BLACK_Y_END;

	//first find area 1 at the bottom right of the image after the bill edge
	bill_info->calibration_down_white_x_start = bill_info->raw_bottom_right_corner.x + CALIBRATION_DOWN_AREA_1_X_BUFFER + CIS_BILL_PIXEL_ALIGN_X;
	bill_info->calibration_down_white_x_end = bill_info->calibration_down_white_x_start + CALIBRATION_DOWN_AREA_1_X_WIDTH - 1;
	bill_info->calibration_down_white_y_start = CALIBRATION_DOWN_WHITE_Y_START;
	bill_info->calibration_down_white_y_end = CALIBRATION_DOWN_WHITE_Y_END;

	bill_info->calibration_down_black_x_start = bill_info->raw_bottom_right_corner.x + CALIBRATION_DOWN_AREA_1_X_BUFFER + CIS_BILL_PIXEL_ALIGN_X;
	bill_info->calibration_down_black_x_end = bill_info->calibration_down_black_x_start + CALIBRATION_DOWN_AREA_1_X_WIDTH - 1;
	bill_info->calibration_down_black_y_start = CALIBRATION_DOWN_BLACK_Y_START;
	bill_info->calibration_down_black_y_end = CALIBRATION_DOWN_BLACK_Y_END;
}


void find_calibration_values()
{
	int sensor_count = 0;

	int x_count = 0;
	int y_count = 0;

	int white_average = 0;
	int black_average = 0;
	int white_point_count = 0;
	int black_point_count = 0;

	int final_result = 0;

	//find white areas
	for(sensor_count = 0; sensor_count < CALIBRATION_SENSOR_COUNT; sensor_count++)
	{
		white_average = 0;
		black_average = 0;
		white_point_count = 0;
		black_point_count = 0;

		if(bill_info->sensor_info[sensor_count].side == CIS_SIDE_UP)
		{
			for(x_count = bill_info->calibration_up_white_x_start; x_count <= bill_info->calibration_up_white_x_end; x_count++)
			{
				for(y_count = bill_info->calibration_up_white_y_start; y_count <= bill_info->calibration_up_white_y_end; y_count++)
				{
					white_average += get_raw_pixel(sensor_count, x_count, y_count);
					white_point_count++;
				}
			}

			white_average /= white_point_count;

			for(x_count = bill_info->calibration_up_black_x_start; x_count <= bill_info->calibration_up_black_x_end; x_count++)
			{
				for(y_count = bill_info->calibration_up_black_y_start; y_count <= bill_info->calibration_up_black_y_end; y_count++)
				{
					black_average += get_raw_pixel(sensor_count, x_count, y_count);
					black_point_count++;
				}
			}

			black_average /= black_point_count;
		}
		else if(bill_info->sensor_info[sensor_count].side == CIS_SIDE_DOWN)
		{
			for(x_count = bill_info->calibration_down_white_x_start; x_count <= bill_info->calibration_down_white_x_end; x_count++)
			{
				for(y_count = bill_info->calibration_down_white_y_start; y_count <= bill_info->calibration_down_white_y_end; y_count++)
				{
					white_average += get_raw_pixel(sensor_count, x_count, y_count);
					white_point_count++;
				}
			}

			white_average /= white_point_count;

			for(x_count = bill_info->calibration_down_black_x_start; x_count <= bill_info->calibration_down_black_x_end; x_count++)
			{
				for(y_count = bill_info->calibration_down_black_y_start; y_count <= bill_info->calibration_down_black_y_end; y_count++)
				{
					black_average += get_raw_pixel(sensor_count, x_count, y_count);
					black_point_count++;
				}
			}

			black_average /= black_point_count;
		}

		final_result = white_average/* + black_average*/;

		//prevent division by zero later on
		if(final_result <= 0)
		{
            final_result = 1;
        }
		else if(final_result > 255)
		{
			final_result = 255;
		}

		bill_info->calibration_difference[sensor_count] = final_result - calibration_level[sensor_count];
		bill_info->calibration_value[sensor_count] = final_result;
		bill_info->calibration_value[sensor_count] = bill_info->calibration_value[sensor_count] - (bill_info->calibration_difference[sensor_count] / 2);
		bill_info->calibration_adjust[sensor_count] = (calibration_level[sensor_count] * CALIBRATION_RATIO_MULTIPLIER) / bill_info->calibration_value[sensor_count];
	}
}
