/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_main.c
* Contents: main validation processing
*
*
*******************************************************************************/

#include <string.h>
#include <math.h>

#define EXT
#include "tem_global.c"
#include "tem_main.h"
#include "tem_edge.h"
#include "tem_pixel.h"
#include "tem_mesh.h"
#include "tem_network.h"

void set_sensor_information(void);

void set_bill_information(P_BILL_INFORMATION const bill_info_in)
{
	bill_info = bill_info_in;
	memset(bill_info, 0, sizeof(BILL_INFORMATION));

	set_sensor_information();
	run_init_edge_routine();
	find_area_limits();
}

void clear_bill_validation_values()
{
	int sensor_count = 0;

	bill_info->validation_mode = VALIDATION_MODE_FULL;

	memset(&bill_info->bottom_edge, 0, sizeof(bill_info->bottom_edge));
	memset(&bill_info->top_edge, 0, sizeof(bill_info->top_edge));
	memset(&bill_info->left_edge, 0, sizeof(bill_info->left_edge));
	memset(&bill_info->right_edge, 0, sizeof(bill_info->right_edge));

	bill_info->calibration_up_white_x_start = 0;
	bill_info->calibration_up_white_x_end = 0;
	bill_info->calibration_up_white_y_start = 0;
	bill_info->calibration_up_white_y_end = 0;

	bill_info->calibration_up_black_x_start = 0;
	bill_info->calibration_up_black_x_end = 0;
	bill_info->calibration_up_black_y_start = 0;
	bill_info->calibration_up_black_y_end = 0;

	bill_info->calibration_down_white_x_start = 0;
	bill_info->calibration_down_white_x_end = 0;
	bill_info->calibration_down_white_y_start = 0;
	bill_info->calibration_down_white_y_end = 0;

	bill_info->calibration_down_black_x_start = 0;
	bill_info->calibration_down_black_x_end = 0;
	bill_info->calibration_down_black_y_start = 0;
	bill_info->calibration_down_black_y_end = 0;

	for(sensor_count = 0; sensor_count < CALIBRATION_SENSOR_COUNT; sensor_count++)
	{
		bill_info->calibration_difference[sensor_count] = 0;
		bill_info->calibration_value[sensor_count] = 0;
		bill_info->calibration_adjust[sensor_count] = 0;
	}

	bill_info->top_slope = 0.0;
	bill_info->top_y_intercept = 0.0;
	bill_info->bottom_slope = 0.0;
	bill_info->bottom_y_intercept = 0.0;
	bill_info->left_slope = 0.0;
	bill_info->left_x_intercept = 0.0;
	bill_info->right_slope = 0.0;
	bill_info->right_x_intercept = 0.0;

	bill_info->final_slope = 0.0;

	bill_info->raw_top_left_corner.x = 0;
	bill_info->raw_top_left_corner.y = 0;
	bill_info->raw_top_right_corner.x = 0;
	bill_info->raw_top_right_corner.y = 0;
	bill_info->raw_bottom_left_corner.x = 0;
	bill_info->raw_bottom_left_corner.y = 0;
	bill_info->raw_bottom_right_corner.x = 0;
	bill_info->raw_bottom_right_corner.y = 0;

	bill_info->raw_center.x = 0;
	bill_info->raw_center.y = 0;

	bill_info->center_offset_x = 0;
	bill_info->center_offset_y = 0;
	bill_info->bill_offset_mm = 0.0;

	bill_info->deskewed_top_left_corner.x = 0;
	bill_info->deskewed_top_left_corner.y = 0;
	bill_info->deskewed_top_right_corner.x = 0;
	bill_info->deskewed_top_right_corner.y = 0;
	bill_info->deskewed_bottom_left_corner.x = 0;
	bill_info->deskewed_bottom_left_corner.y = 0;
	bill_info->deskewed_bottom_right_corner.x = 0;
	bill_info->deskewed_bottom_right_corner.y = 0;

	bill_info->skew_degree_x = 0.0;
	bill_info->skew_radian_x = 0.0;
	bill_info->skew_degree_y = 0.0;
	bill_info->skew_radian_y = 0.0;
	bill_info->top_skew_sine_correction_x = 0.0;
	bill_info->top_skew_cosine_correction_x = 0.0;
	bill_info->bottom_skew_sine_correction_x = 0.0;
	bill_info->bottom_skew_cosine_correction_x = 0.0;

	bill_info->width_pixel = 0;
	bill_info->length_pixel = 0;
	bill_info->width_mm = 0.0;
	bill_info->length_mm = 0.0;
	bill_info->possible_ticket_detected = FALSE;

	bill_info->ticket_direction = 0;
}

void run_init_edge_routine()
{
	reset_watchdog_timer();
	clear_bill_validation_values();

	reset_watchdog_timer();
	run_edge_routine();
}
/* EOF */
