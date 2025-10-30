/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_pixel_iVIZION.c
* Contents: pixel accquisition for the iVIZION product
*
*
*******************************************************************************/

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "cis_ram.c"
#include "tem_global.c"

#include "tem_pixel.h"

//remap the template coordinate system to the RBA-40C data buffer, converts all cis data to 200x200dpi
unsigned char* get_raw_pixel_address(const int sensor_index, const int x_point, const int y_point)
{
	unsigned char* data_pointer = NULL;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *p_data_collection = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *p_data_collection = (ST_BS *)BILL_NOTE_IMAGE_TOP;
#endif
	unsigned char* block_data = NULL;
	unsigned char* final_result = 0;

	int is_down = 0;
	int data_offset = 0;
	//verify that the data is a correct value
	if(x_point >= bill_info->sensor_info[sensor_index].x_pixels
		|| y_point >= bill_info->sensor_info[sensor_index].y_pixels)
	{
        return 0;
    }
	if(sensor_index <= CIS_GREEN_DOWN) //reflective sensor 200 dpi case
	{
		block_data = &p_data_collection->sens_dt[x_point/SUB_BLOCK_NUM * BLOCK_BYTE_SIZE];

		switch(sensor_index)
		{
		case CIS_RED_UP:
			data_pointer = (unsigned char*)(&block_data[CISA_R_R_OFFSET]);
			break;
		case CIS_RED_DOWN:
			is_down = 1;
			data_pointer = (unsigned char*)(&block_data[CISB_R_R_OFFSET]);
			break;
		case CIS_GREEN_UP:
			data_pointer = (unsigned char*)(&block_data[CISA_R_G_OFFSET]);
			break;
		case CIS_GREEN_DOWN:
			is_down = 1;
			data_pointer = (unsigned char*)(&block_data[CISB_R_G_OFFSET]);
			break;
		};
		if(x_point%4)
		{
			// set sub block1 (offset sub block0 size)
			data_offset = SUB_BLOCK_BYTE_SIZE * ((x_point % 4));
			data_pointer += data_offset;
		}
		if(is_down)
		{
			// DOWN SIDE
			if((bill_info->sensor_info[sensor_index].y_pixels - 1 - y_point) <= 35)
			{
				// y:20~35　exist white plate
				final_result = (int)0;
			}
			else
			{
				final_result = (unsigned char *)(data_pointer + HDRTBL_SIZE + y_point);
			}
		}
		else
		{
			// UP SIDE
			if(y_point <= 35)
			{
				// y:20~35　exist white plate
				final_result = (int)0;
			}
			else
			{
				final_result = (unsigned char *)(data_pointer + HDRTBL_SIZE + (bill_info->sensor_info[sensor_index].y_pixels - 1 - y_point));
			}
		}
	}

	return final_result;
}
//return the deskewed pixel when given deskewed coordinates, also calibrates the pixel
unsigned char* get_deskewed_pixel_address(const int sensor_index, int x_point, int y_point)
{
	unsigned char* result = 0;

	find_raw_pixel_coordinates(sensor_index, &x_point, &y_point);

	//check if the deskewed pixel is off the grid
	if((x_point < 0) || (x_point > bill_info->sensor_info[sensor_index].x_pixels))
	{
		return 0;
	}

	if((y_point < 0) || (y_point > bill_info->sensor_info[sensor_index].y_pixels))
	{
		return 0;
	}

	result = get_raw_pixel_address(sensor_index, x_point, y_point);

	return result;
}

//remap the template coordinate system to the RBA-40C data buffer, converts all cis data to 200x200dpi
int get_raw_pixel(const int sensor_index, const int x_point, const int y_point)
{
	unsigned char* data_pointer = NULL;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *p_data_collection = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *p_data_collection = (ST_BS *)BILL_NOTE_IMAGE_TOP;
#endif
	unsigned char* block_data = NULL;


	int temp_result1 = 0;
	int temp_result2 = 0;
    int final_result = 0;

	int x_temp1 = 0;
	int x_temp2 = 0;

	int is_down = 0;
	int data_offset = 0;
	if((sensor_index == CIS_RED_UP)
	||(sensor_index == CIS_GREEN_UP)
	||(sensor_index == CIS_RED_DOWN)
	||(sensor_index == CIS_GREEN_DOWN))
	{
		//verify that the data is a correct value
		if(x_point >= bill_info->sensor_info[sensor_index].x_pixels
			|| y_point >= bill_info->sensor_info[sensor_index].y_pixels)
		{
	        return 0;
	    }
		//reflective sensor 200 dpi case
		block_data = &p_data_collection->sens_dt[x_point/SUB_BLOCK_NUM * BLOCK_BYTE_SIZE];
		switch(sensor_index)
		{
		case CIS_RED_UP:
			data_pointer = (unsigned char*)(&block_data[CISA_R_R_OFFSET]);
			break;
		case CIS_RED_DOWN:
			is_down = 1;
			data_pointer = (unsigned char*)(&block_data[CISB_R_R_OFFSET]);
			break;
		case CIS_GREEN_UP:
			data_pointer = (unsigned char*)(&block_data[CISA_R_G_OFFSET]);
			break;
		case CIS_GREEN_DOWN:
			is_down = 1;
			data_pointer = (unsigned char*)(&block_data[CISB_R_G_OFFSET]);
			break;
		};
		if(x_point%4)
		{
			// set sub block1 (offset sub block0 size)
			data_offset = SUB_BLOCK_BYTE_SIZE * ((x_point % 4));
			data_pointer += data_offset;
		}
		if(is_down)
		{
			// DOWN SIDE
			if((y_point) <= 35)
			{
				// y:20~35　exist white plate
				final_result = (int)0;
			}
			else
			{
				final_result = (int)((unsigned char)(*(data_pointer + HDRTBL_SIZE + (bill_info->sensor_info[sensor_index].y_pixels - 1 - y_point))));
			}
		}
		else
		{
			// UP SIDE
			if((bill_info->sensor_info[sensor_index].y_pixels - 1 - y_point) <= 35)
			{
				// y:20~35　exist white plate
				final_result = (int)0;
			}
			else
			{
				final_result = (int)((unsigned char)(*(data_pointer + HDRTBL_SIZE + y_point)));
			}
		}
	}
	else if((sensor_index == CIS_RED_TRANSPARENT_DOWN)
	||(sensor_index == CIS_IR1_TRANSPARENT_DOWN))
	{
		//verify that the data is a correct value
		if(x_point >= bill_info->sensor_info[sensor_index].x_pixels * CYCTBL_CYC_VALUE
			|| y_point >= bill_info->sensor_info[sensor_index].y_pixels)
		{
	        return 0;
	    }
		//transparent sensor(50dpi)
		// convert 200dpi
		x_temp1 = x_point / CYCTBL_CYC_VALUE;
		x_temp2 = x_temp1 + 1;

		block_data = &p_data_collection->sens_dt[x_temp1 * BLOCK_BYTE_SIZE];

		//find first pixel
		switch(sensor_index)
		{
		case CIS_RED_TRANSPARENT_DOWN:
			data_pointer = (unsigned char*)(&block_data[CISB_T_R_OFFSET]);
			break;
		case CIS_IR1_TRANSPARENT_DOWN:
			data_pointer = (unsigned char*)(&block_data[CISB_T_IR1_OFFSET]);
			break;
		};
		temp_result1 = (int)((unsigned char)(*(data_pointer + HDRTBL_SIZE + y_point)));

		block_data = &p_data_collection->sens_dt[x_temp2 * BLOCK_BYTE_SIZE];

		//find second pixel
		switch(sensor_index)
		{
		case CIS_RED_TRANSPARENT_DOWN:
			data_pointer = (unsigned char*)(&block_data[CISB_T_R_OFFSET]);
			break;
		case CIS_IR1_TRANSPARENT_DOWN:
			data_pointer = (unsigned char*)(&block_data[CISB_T_IR1_OFFSET]);
			break;
		};
		temp_result2 = (int)((unsigned char)(*(data_pointer + HDRTBL_SIZE + y_point)));

		if(x_point >= bill_info->sensor_info[sensor_index].x_pixels - 2)
		{
			final_result = temp_result1;
		}
		else if(x_point % 2 == 0)
		{
			final_result = temp_result1;
		}
		else
		{
			final_result = (temp_result1 + temp_result2) / 2;
		}
	}

	return final_result;
}


//return the deskewed pixel when given deskewed coordinates, also calibrates the pixel
int get_deskewed_pixel(const int sensor_index, int x_point, int y_point)
{
	int result = 0;

	find_raw_pixel_coordinates(sensor_index, &x_point, &y_point);

	//check if the deskewed pixel is off the grid
	if((x_point < 0) || (x_point > bill_info->sensor_info[sensor_index].x_pixels))
	{
		return 0;
	}

	if((y_point < 0) || (y_point > bill_info->sensor_info[sensor_index].y_pixels))
	{
		return 0;
	}

	result = get_raw_pixel(sensor_index, x_point, y_point);

#if OPTION_ENABLE_CALIBRATION == 1
	result = get_calibrated_pixel(sensor_index, x_point, result);
#endif

	return result;
}


//adjust the pixel value based on cis data taken from the calibration squares
int get_calibrated_pixel(const int sensor_index, const int x_point, int value)
{
	return 0;
}


int get_calibrated_mesh_area(const int sensor_index, const int total, const int points_used)
{
	int temp = total;

#if OPTION_ENABLE_CALIBRATION == 1
	//calibrate the final total
	if(sensor_index <= CIS_IR_TRANSPARENT)
	{
		temp *= bill_info->calibration_adjust[sensor_index];
	}

	temp /= CALIBRATION_RATIO_MULTIPLIER;
#endif

	temp /= points_used;

	if(temp < 0)
	{
		temp = 0;
	}
	else if(temp > 255)
	{
		temp = 255;
	}

	return temp;
}
