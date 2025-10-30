/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_sensors_iVIZION.c
* Contents: sensor information tables for the iVIZION product
*
*
*******************************************************************************/
#include <math.h>

#include "common.h"
#define EXT
#include "../common/global.h"
#include "tem_global.c"
#include "cis_ram.c"


/*******************************************************************************
* Sensor Adjustment Settings
*
*******************************************************************************/
const int calibration_level[CALIBRATION_SENSOR_COUNT] =
{
	128,	//CIS Red Up
	128,	//CIS Green Up
	128,	//CIS IR Up
	128,	//CIS Red Down
	128,	//CIS Green Down
	128,	//CIS IR Down
};

const int sensor_normalize_table[CIS_SENSOR_COUNT][DIRECTION_COUNT] =
{
	{CIS_RED_UP,				CIS_RED_UP,					CIS_RED_DOWN,				CIS_RED_DOWN},  			//CIS Red Up
	{CIS_GREEN_UP,				CIS_GREEN_UP,				CIS_GREEN_DOWN,				CIS_GREEN_DOWN},			//CIS Green Up
	{CIS_IR1_UP,				CIS_IR1_UP,					CIS_IR1_DOWN,				CIS_IR1_DOWN},   			//CIS IR Up
	{CIS_IR2_UP,				CIS_IR2_UP,					CIS_IR2_DOWN,				CIS_IR2_DOWN},   			//CIS IR Up

	{CIS_RED_DOWN,				CIS_RED_DOWN,				CIS_RED_UP,					CIS_RED_UP},  				//CIS Red Down
	{CIS_GREEN_DOWN,			CIS_GREEN_DOWN,				CIS_GREEN_UP,				CIS_GREEN_UP},				//CIS Green Down
	{CIS_IR1_DOWN,				CIS_IR1_DOWN,				CIS_IR1_UP,					CIS_IR1_UP},   				//CIS IR Down
	{CIS_IR2_DOWN,				CIS_IR2_DOWN,				CIS_IR2_UP,					CIS_IR2_UP},   				//CIS IR Down

	{CIS_RED_TRANSPARENT_DOWN,		CIS_RED_TRANSPARENT_DOWN,		CIS_RED_TRANSPARENT_DOWN,		CIS_RED_TRANSPARENT_DOWN},		//CIS Red Transparent
	{CIS_GREEN_TRANSPARENT_DOWN,		CIS_GREEN_TRANSPARENT_DOWN,		CIS_GREEN_TRANSPARENT_DOWN,		CIS_GREEN_TRANSPARENT_DOWN},		//CIS Green Transparent
	{CIS_IR1_TRANSPARENT_DOWN,		CIS_IR1_TRANSPARENT_DOWN,			CIS_IR1_TRANSPARENT_DOWN,			CIS_IR1_TRANSPARENT_DOWN},		//CIS IR Transparent
};


void set_sensor_information(void)
{
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *p_data_collection = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *p_data_collection = (ST_BS *)BILL_NOTE_IMAGE_TOP;
#endif
	/* CIS Barcode Up */
	bill_info->sensor_info[CIS_BARCODE_UP].pitch = PITCH_200DPI;
	bill_info->sensor_info[CIS_BARCODE_UP].side = CIS_SIDE_UP;
	bill_info->sensor_info[CIS_BARCODE_UP].color = CIS_COLOR_BARCODE;
	bill_info->sensor_info[CIS_BARCODE_UP].x_pixels = p_data_collection->block_count * 4;
	bill_info->sensor_info[CIS_BARCODE_UP].y_pixels = CIS_BARCODE_PIXEL_COUNT_Y;
	bill_info->sensor_info[CIS_BARCODE_UP].deskew_center_x = p_data_collection->center_y;//p_data_collection->block_count * 2;
	bill_info->sensor_info[CIS_BARCODE_UP].deskew_center_y = p_data_collection->center_x;//CIS_BARCODE_PIXEL_COUNT_Y / 2;
	bill_info->sensor_info[CIS_BARCODE_UP].deskew_align_x = CIS_BARCODE_UP_PIXEL_ALIGN_X;
	bill_info->sensor_info[CIS_BARCODE_UP].deskew_align_y = CIS_BARCODE_UP_PIXEL_ALIGN_Y;

	/* CIS Barcode Down */
	bill_info->sensor_info[CIS_BARCODE_DOWN].pitch = PITCH_200DPI;
	bill_info->sensor_info[CIS_BARCODE_DOWN].side = CIS_SIDE_DOWN;
	bill_info->sensor_info[CIS_BARCODE_DOWN].color = CIS_COLOR_BARCODE;
	bill_info->sensor_info[CIS_BARCODE_DOWN].x_pixels = p_data_collection->block_count * 4;
	bill_info->sensor_info[CIS_BARCODE_DOWN].y_pixels = CIS_BARCODE_PIXEL_COUNT_Y;
	bill_info->sensor_info[CIS_BARCODE_DOWN].deskew_center_x = p_data_collection->center_y;//p_data_collection->block_count * 2;
	bill_info->sensor_info[CIS_BARCODE_DOWN].deskew_center_y = p_data_collection->center_x;//CIS_BARCODE_PIXEL_COUNT_Y / 2;
	bill_info->sensor_info[CIS_BARCODE_DOWN].deskew_align_x = CIS_BARCODE_DOWN_PIXEL_ALIGN_X;
	bill_info->sensor_info[CIS_BARCODE_DOWN].deskew_align_y = CIS_BARCODE_DOWN_PIXEL_ALIGN_Y;

	/* CIS IR1 UP*/
	bill_info->sensor_info[CIS_IR1_UP].pitch = PITCH_100DPI;
	bill_info->sensor_info[CIS_IR1_UP].side = CIS_SIDE_UP;
	bill_info->sensor_info[CIS_IR1_UP].color = CIS_COLOR_IR;
	bill_info->sensor_info[CIS_IR1_UP].x_pixels = p_data_collection->block_count * 2;
	bill_info->sensor_info[CIS_IR1_UP].y_pixels = CIS_BARCODE_PIXEL_COUNT_Y;
	bill_info->sensor_info[CIS_IR1_UP].deskew_center_x = p_data_collection->center_y / 2;//p_data_collection->block_count / 1;
	bill_info->sensor_info[CIS_IR1_UP].deskew_center_y = p_data_collection->center_x;//CIS_BARCODE_PIXEL_COUNT_Y / 2;
	bill_info->sensor_info[CIS_IR1_UP].deskew_align_x = CIS_BARCODE_UP_PIXEL_ALIGN_X;
	bill_info->sensor_info[CIS_IR1_UP].deskew_align_y = CIS_BARCODE_UP_PIXEL_ALIGN_Y;
	/* CIS IR1 DW */
	bill_info->sensor_info[CIS_IR1_DOWN].pitch = PITCH_100DPI;
	bill_info->sensor_info[CIS_IR1_DOWN].side = CIS_SIDE_DOWN;
	bill_info->sensor_info[CIS_IR1_DOWN].color = CIS_COLOR_IR;
	bill_info->sensor_info[CIS_IR1_DOWN].x_pixels = p_data_collection->block_count * 2;
	bill_info->sensor_info[CIS_IR1_DOWN].y_pixels = CIS_BARCODE_PIXEL_COUNT_Y;
	bill_info->sensor_info[CIS_IR1_DOWN].deskew_center_x = p_data_collection->center_y / 2;//p_data_collection->block_count / 1;
	bill_info->sensor_info[CIS_IR1_DOWN].deskew_center_y = p_data_collection->center_x;//CIS_BARCODE_PIXEL_COUNT_Y / 2;
	bill_info->sensor_info[CIS_IR1_DOWN].deskew_align_x = CIS_BARCODE_DOWN_PIXEL_ALIGN_X;
	bill_info->sensor_info[CIS_IR1_DOWN].deskew_align_y = CIS_BARCODE_DOWN_PIXEL_ALIGN_Y;

	/* CIS_IR_TRANSPARENT */
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].pitch = PITCH_50DPI;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].side = CIS_SIDE_TRANSPARENT;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].color = CIS_COLOR_IR;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].x_pixels = p_data_collection->block_count;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].y_pixels = CIS_BARCODE_PIXEL_COUNT_Y;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].deskew_center_x = p_data_collection->center_y / 4;//p_data_collection->block_count / 2;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].deskew_center_y = p_data_collection->center_x;//CIS_BARCODE_PIXEL_COUNT_Y / 2;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].deskew_align_x = CIS_BARCODE_DOWN_PIXEL_ALIGN_X;
	bill_info->sensor_info[CIS_IR1_TRANSPARENT_DOWN].deskew_align_y = CIS_BARCODE_DOWN_PIXEL_ALIGN_Y;
}


int get_sensor_at_side(const int sensor_index, const int side)
{
	return sensor_index + side * 3;
}
