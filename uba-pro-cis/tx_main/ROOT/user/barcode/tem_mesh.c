/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_mesh.c
* Contents: mesh area processing
*
*
*******************************************************************************/
#define EXT
#include "tem_global.c"

#include "tem_pixel.h"
#include "tem_mesh.h"

//find which mesh areas actually cover the bill data
void find_area_limits()
{
	//x direction edges
	if(bill_info->deskewed_left_edge < MESH_X_PIXEL_START)
	{
		bill_info->mesh_start_area_x = 0;
	}
	else
	{
		bill_info->mesh_start_area_x = ((bill_info->deskewed_left_edge - MESH_X_PIXEL_START) / MESH_CIRCLE_DIAMETER) + 1;
		//bill_info->mesh_start_area_x = ((bill_info->deskewed_left_edge/4 - MESH_X_PIXEL_START) / MESH_CIRCLE_DIAMETER) + 1;
	}
	bill_info->mesh_end_area_x = ((bill_info->deskewed_right_edge - MESH_X_PIXEL_START) / MESH_CIRCLE_DIAMETER) - 1;
	//bill_info->mesh_end_area_x = ((bill_info->deskewed_right_edge/4 - MESH_X_PIXEL_START) / MESH_CIRCLE_DIAMETER) - 1;

	//y direction edges
	if(bill_info->deskewed_bottom_edge < MESH_Y_PIXEL_START)
	{
		bill_info->mesh_start_area_y = 0;
	}
	else
	{
		bill_info->mesh_start_area_y = ((bill_info->deskewed_bottom_edge - MESH_Y_PIXEL_START) / MESH_CIRCLE_DIAMETER) + 1;
	}
	bill_info->mesh_end_area_y = ((bill_info->deskewed_top_edge - MESH_Y_PIXEL_START) / MESH_CIRCLE_DIAMETER) - 1;

	if(bill_info->mesh_start_area_x < 0)
	{
		bill_info->mesh_start_area_x = 0;
	}
	else if(bill_info->mesh_start_area_x > MESH_AREA_COUNT_X - 1)
	{
		bill_info->mesh_start_area_x = MESH_AREA_COUNT_X - 1;
	}

	if(bill_info->mesh_end_area_x < 0)
	{
		bill_info->mesh_end_area_x = 0;
	}
	else if(bill_info->mesh_end_area_x > MESH_AREA_COUNT_X - 1)
	{
		bill_info->mesh_end_area_x = MESH_AREA_COUNT_X - 1;
	}

	if(bill_info->mesh_start_area_y < 0)
	{
		bill_info->mesh_start_area_y = 0;
	}
	else if(bill_info->mesh_start_area_y > MESH_AREA_COUNT_Y - 1)
	{
		bill_info->mesh_start_area_y = MESH_AREA_COUNT_Y - 1;
	}

	if(bill_info->mesh_end_area_y < 0)
	{
		bill_info->mesh_end_area_y = 0;
	}
	else if(bill_info->mesh_end_area_y > MESH_AREA_COUNT_Y - 1)
	{
		bill_info->mesh_end_area_y = MESH_AREA_COUNT_Y - 1;
	}
}


//remap the mesh area as if the bill was inserted in the A direction
void normalize_mesh_area_by_direction(const int direction, int* sensor_index, int* area_x, int* area_y)
{
	switch(direction)
	{
		case A_DIRECTION:
			//do nothing since already A direction
			break;
		case B_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			*area_x = MESH_AREA_COUNT_X - 1 - *area_x;
			*area_y = MESH_AREA_COUNT_Y - 1 - *area_y;

			break;
		case C_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			if(bill_info->sensor_info[*sensor_index].side == CIS_SIDE_TRANSPARENT)
			{
				*area_y = MESH_AREA_COUNT_Y - 1 - *area_y;
			}

			break;
		case D_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			if(bill_info->sensor_info[*sensor_index].side == CIS_SIDE_UP || bill_info->sensor_info[*sensor_index].side == CIS_SIDE_DOWN)
			{
				*area_x = MESH_AREA_COUNT_X - 1 - *area_x;
				*area_y = MESH_AREA_COUNT_Y - 1 - *area_y;
			}
			else if(bill_info->sensor_info[*sensor_index].side == CIS_SIDE_TRANSPARENT)
			{
				*area_x = MESH_AREA_COUNT_X - 1 - *area_x;
			}

			break;
		default:

			break;
	}
}


//remap a pixel as if the bill was inserted in the A direction
void normalize_pixel_by_direction(const int direction, int* sensor_index, int* x_point, int* y_point)
{
	switch(direction)
	{
		case A_DIRECTION:
			//do nothing since already A direction
			break;
		case B_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			if(x_point != 0 && y_point != 0)
			{
				*y_point = bill_info->sensor_info[*sensor_index].deskew_center_y * 2 - 1 - *y_point;
				*x_point = bill_info->sensor_info[*sensor_index].deskew_center_x * 2 - 1 - *x_point;
			}

			break;
		case C_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			if(x_point != 0 && y_point != 0)
			{
				if(bill_info->sensor_info[*sensor_index].side == CIS_SIDE_TRANSPARENT)
				{
					*y_point = bill_info->sensor_info[*sensor_index].deskew_center_y * 2 - 1 - *y_point;
				}
			}

			break;
		case D_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			if(x_point != 0 && y_point != 0)
			{
				if(bill_info->sensor_info[*sensor_index].side == CIS_SIDE_UP || bill_info->sensor_info[*sensor_index].side == CIS_SIDE_DOWN)
				{
					*y_point = bill_info->sensor_info[*sensor_index].deskew_center_y * 2 - 1 - *y_point;
					*x_point = bill_info->sensor_info[*sensor_index].deskew_center_x * 2 - 1 - *x_point;
				}
				else if(bill_info->sensor_info[*sensor_index].side == CIS_SIDE_TRANSPARENT)
				{
					*x_point = bill_info->sensor_info[*sensor_index].deskew_center_x * 2 - 1 - *x_point;
				}
			}

			break;
		default:

			break;
	}
}


//remap the neural network area as if the bill was inserted in the A direction
void normalize_network_area_by_direction(const int direction, int* sensor_index, int* area_x, int* area_y)
{
	switch(direction)
	{
		case A_DIRECTION:
			//do nothing since already A direction
			break;
		case B_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			*area_x = NETWORK_AREA_COUNT_X - 1 - *area_x;
			*area_y = NETWORK_AREA_COUNT_Y - 1 - *area_y;

			break;
		case C_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			break;
		case D_DIRECTION:
			*sensor_index = sensor_normalize_table[*sensor_index][direction];

			*area_x = NETWORK_AREA_COUNT_X - 1 - *area_x;
			*area_y = NETWORK_AREA_COUNT_Y - 1 - *area_y;

			break;
		default:

			break;
	}
}


//find the mesh area averages for all areas
void find_average_values()
{
	int sensor_count = 0;
	int x_count = 0;
	int y_count = 0;

	for(sensor_count = 0; sensor_count < CIS_SENSOR_COUNT; sensor_count++)
	{
		if(bill_info->sensor_info[sensor_count].color == CIS_COLOR_BARCODE)
		{
			continue;
		}

		for(x_count = bill_info->mesh_start_area_x; x_count <= bill_info->mesh_end_area_x; x_count++)
		{
			for(y_count = bill_info->mesh_start_area_y; y_count <= bill_info->mesh_end_area_y; y_count++)
			{
				get_mesh_area_average(sensor_count, x_count, y_count);
			}
		}
	}
}


//normalizes every direction to A direction
int get_mesh_area_average(const int sensor_index, const int x_area, const int y_area)
{
	int sensor_index_temp = sensor_index;
	int x_area_temp = x_area;
	int y_area_temp = y_area;

	if(bill_info->area_average[sensor_index][x_area][y_area] == 0) //check that this area has not been calculated before to save time
	{
		normalize_mesh_area_by_direction(bill_info->denomination_direction, &sensor_index_temp, &x_area_temp, &y_area_temp);

		bill_info->area_average[sensor_index][x_area][y_area]
			= get_deskewed_circle_average(sensor_index_temp, x_area_temp * MESH_CIRCLE_DIAMETER + MESH_X_PIXEL_START,
				y_area_temp * MESH_CIRCLE_DIAMETER + MESH_Y_PIXEL_START);
	}

	return bill_info->area_average[sensor_index][x_area][y_area];
}


//do not normalize to the A direction
int get_unnormalized_mesh_area_average(const int sensor_index, const int x_area, const int y_area)
{
	int result = 0;

	result = get_deskewed_circle_average(sensor_index, x_area * MESH_CIRCLE_DIAMETER + MESH_X_PIXEL_START,
			y_area * MESH_CIRCLE_DIAMETER + MESH_Y_PIXEL_START);

	return result;
}


int get_mesh_cross_area_x(const int area_x, const int area_y)
{
	int result = 0;

	if(area_x < MESH_AREA_MIDDLE_X && area_y >= MESH_AREA_MIDDLE_Y) //quadrant 1
	{
		result = MESH_AREA_MIDDLE_X + (MESH_AREA_MIDDLE_X - area_x - 1);
	}
	else if(area_x >= MESH_AREA_MIDDLE_X && area_y >= MESH_AREA_MIDDLE_Y) //quadrant 2
	{
		result = MESH_AREA_MIDDLE_X - (area_x - MESH_AREA_MIDDLE_X + 1);
	}
	else if(area_x < MESH_AREA_MIDDLE_X && area_y < MESH_AREA_MIDDLE_Y) //quadrant 3
	{
		result = MESH_AREA_MIDDLE_X + (MESH_AREA_MIDDLE_X - area_x - 1);
	}
	else if(area_x >= MESH_AREA_MIDDLE_X && area_y < MESH_AREA_MIDDLE_Y) //quadrant 4
	{
		result = MESH_AREA_MIDDLE_X - (area_x - MESH_AREA_MIDDLE_X + 1);
	}

	return result;
}


int get_mesh_cross_area_y(const int area_x, const int area_y)
{
	int result = 0;

	if(area_x < MESH_AREA_MIDDLE_X && area_y >= MESH_AREA_MIDDLE_Y) //quadrant 1
	{
		result = area_y;
	}
	else if(area_x >= MESH_AREA_MIDDLE_X && area_y >= MESH_AREA_MIDDLE_Y) //quadrant 2
	{
		result = MESH_AREA_MIDDLE_Y - (area_y - MESH_AREA_MIDDLE_Y + 1);
	}
	else if(area_x < MESH_AREA_MIDDLE_X && area_y < MESH_AREA_MIDDLE_Y) //quadrant 3
	{
		result = area_y;
	}
	else if(area_x >= MESH_AREA_MIDDLE_X && area_y < MESH_AREA_MIDDLE_Y) //quadrant 4
	{
		result = MESH_AREA_MIDDLE_Y + (MESH_AREA_MIDDLE_Y - area_y - 1);
	}

	return result;
}
