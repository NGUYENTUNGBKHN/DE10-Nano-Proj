/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_pixel.c
* Contents: handle image transformation for deskewing
*
*
*******************************************************************************/

#define EXT
#include "common.h"
#include "tem_global.c"

#include "tem_pixel.h"

//pixel map for circular mesh areas
const int circle_map[MESH_CIRCLE_DIAMETER][MESH_CIRCLE_DIAMETER] =
{
	{	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	0,	0,	0,	0,	0,},
	{	0,	0,	0,	1,	1,	0,	2,	0,	2,	0,	1,	1,	0,	0,	0,},
	{	0,	0,	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	0,	0,},
	{	0,	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	0,},
	{	0,	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	0,},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,},
	{	0,	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	0,},
	{	0,	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	0,},
	{	0,	0,	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	0,	0,},
	{	0,	0,	0,	1,	1,	0,	2,	0,	2,	0,	1,	1,	0,	0,	0,},
	{	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	0,	0,	0,	0,	0,},
};


//pixel map for circular mesh areas
const int square_map[MESH_SQUARE_DIAMETER][MESH_SQUARE_DIAMETER] =
{
	{	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	},
	{	1,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	1,	},
	{	1,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	2,	0,	1,	},
	{	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	},
};


//find raw pixel coordinates from deskewed ones
void find_raw_pixel_coordinates(const int sensor_index, int* x_point, int* y_point)
{
	float rotated_point_x = 0.0;
	float rotated_point_y = 0.0;

	float temp_x_point = (float)*x_point;
	float temp_y_point = (float)*y_point;

	if(bill_info->sensor_info[sensor_index].side == CIS_SIDE_UP) //up direction
	{
		//shift coordinates to a center origin point
		temp_x_point -= (float)bill_info->sensor_info[sensor_index].deskew_center_x;
		temp_y_point -= (float)bill_info->sensor_info[sensor_index].deskew_center_y;

		rotated_point_x = temp_x_point * bill_info->bottom_skew_cosine_correction_x + temp_y_point * bill_info->bottom_skew_sine_correction_x;
		rotated_point_y = temp_y_point * bill_info->bottom_skew_cosine_correction_x - temp_x_point * bill_info->bottom_skew_sine_correction_x;
		//shift coordinates away from center origin
		rotated_point_x += (float)bill_info->sensor_info[sensor_index].deskew_center_x;
		rotated_point_y += (float)bill_info->sensor_info[sensor_index].deskew_center_y;

		//account for shifting all the data to the center
		rotated_point_x += (float)bill_info->center_offset_x * PITCH_200DPI / bill_info->sensor_info[sensor_index].pitch;
		rotated_point_y += (float)bill_info->center_offset_y * PITCH_200DPI / bill_info->sensor_info[sensor_index].pitch;

		//account for sensor alignment differences
		rotated_point_x += (float)bill_info->sensor_info[sensor_index].deskew_align_x;
		rotated_point_y += (float)bill_info->sensor_info[sensor_index].deskew_align_y;

		*x_point = (int)(rotated_point_x + 0.5);
		*y_point = (int)(rotated_point_y + 0.5);
	}
	else if(bill_info->sensor_info[sensor_index].side == CIS_SIDE_DOWN || bill_info->sensor_info[sensor_index].side == CIS_SIDE_TRANSPARENT) //down and transparent directions
	{
		//shift coordinates to a center origin point
		temp_x_point -= (float)bill_info->sensor_info[sensor_index].deskew_center_x;
		temp_y_point -= (float)bill_info->sensor_info[sensor_index].deskew_center_y;

		rotated_point_x = temp_x_point * bill_info->top_skew_cosine_correction_x + temp_y_point * bill_info->top_skew_sine_correction_x;
		rotated_point_y = temp_y_point * bill_info->top_skew_cosine_correction_x - temp_x_point * bill_info->top_skew_sine_correction_x;

		//shift coordinates away from center origin
		rotated_point_x += (float)bill_info->sensor_info[sensor_index].deskew_center_x;
		rotated_point_y += (float)bill_info->sensor_info[sensor_index].deskew_center_y;

		//account for shifting all the data to the center
		rotated_point_x += (float)bill_info->center_offset_x * PITCH_200DPI / bill_info->sensor_info[sensor_index].pitch;
		rotated_point_y -= (float)bill_info->center_offset_y * PITCH_200DPI / bill_info->sensor_info[sensor_index].pitch;

		//account for sensor alignment differences
		rotated_point_x += (float)bill_info->sensor_info[sensor_index].deskew_align_x;
		rotated_point_y += (float)bill_info->sensor_info[sensor_index].deskew_align_y;

		*x_point = (int)(rotated_point_x + 0.5);
		*y_point = (int)(rotated_point_y + 0.5);
	}
}




//get an average of a rectangle of cis data
int get_deskewed_rectangle_average(const int sensor_index, const int x_origin, const int y_origin,
	const int x_length, const int y_length)
{
	int total = 0;

	int x_count = 0;
	int y_count = 0;

	int x_point = 0;
	int y_point = 0;

	for(x_count = 0; x_count < x_length; x_count++)
	{
		x_point = x_origin + x_count;

		for(y_count = 0; y_count < y_length; y_count++)
		{
			y_point = y_origin + y_count;

			total += (int)get_deskewed_pixel(sensor_index, x_point, y_point);
		}
	}

	total /= (x_length * y_length);

	return total;
}

//get an average of a circle of cis data
int get_deskewed_circle_average(const int sensor_index, const int x_origin, const int y_origin)
{
	int total = 0;
	int points_used = 0;

	int x_count = 0;
	int y_count = 0;

	int center_x_point = x_origin + MESH_CIRCLE_DIAMETER / 2;
	int center_y_point = y_origin + MESH_CIRCLE_DIAMETER / 2;

	int x_start = 0;
	int y_start = 0;

	int x_point = 0;
	int y_point = 0;

	find_raw_pixel_coordinates(sensor_index, &center_x_point, &center_y_point);

	x_start = center_x_point - MESH_CIRCLE_DIAMETER / 2;
	y_start = center_y_point - MESH_CIRCLE_DIAMETER / 2;

	for(x_count = 0; x_count < MESH_CIRCLE_DIAMETER; x_count++)
	{
		x_point = x_start + x_count;

		for(y_count = 0; y_count < MESH_CIRCLE_DIAMETER; y_count++)
		{
			y_point = y_start + y_count;

			if(circle_map[x_count][y_count] != 0)
			{
				total += (get_raw_pixel(sensor_index, x_point, y_point) * circle_map[x_count][y_count]);
				points_used += circle_map[x_count][y_count];
			}
		}
	}

	total = get_calibrated_mesh_area(sensor_index, total, points_used);

	return total;
}



//find deskewed pixel coordinates from raw ones
void find_deskewed_pixel_coordinates(const int sensor_index, int* x_point, int* y_point)
{
	float rotated_point_x = 0.0;
	float rotated_point_y = 0.0;

	if(bill_info->sensor_info[sensor_index].side == CIS_SIDE_UP)
	{
		//account for shifting all the data to the center
		(*x_point) -= bill_info->center_offset_x;
		(*y_point) -= bill_info->center_offset_y;

		//shift coordinates to a center origin point
		(*x_point) -= bill_info->sensor_info[sensor_index].deskew_center_x;
		(*y_point) -= bill_info->sensor_info[sensor_index].deskew_center_y;

		rotated_point_x = (float)(*x_point) * bill_info->bottom_skew_cosine_correction_x + (float)(*y_point) * bill_info->bottom_skew_sine_correction_x;
		rotated_point_y = (float)(*y_point) * bill_info->bottom_skew_cosine_correction_x - (float)(*x_point) * bill_info->bottom_skew_sine_correction_x;

		(*x_point) = (int)(rotated_point_x + 0.5);
		(*y_point) = (int)(rotated_point_y + 0.5);

		//shift coordinates away from center origin
		(*x_point) += bill_info->sensor_info[sensor_index].deskew_center_x;
		(*y_point) += bill_info->sensor_info[sensor_index].deskew_center_y;
	}
}


//remap the template coordinate system to the RBA-40C data buffer, converts all cis data to 200x200dpi
int get_calibrated_raw_pixel(const int sensor_index, const int x_point, const int y_point)
{
	int result = 0;

	result = get_raw_pixel(sensor_index, x_point, y_point);

	return result;
}
