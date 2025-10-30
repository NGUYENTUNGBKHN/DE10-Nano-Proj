/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_pixel.h
* Contents: handle image transformation for deskewing
*
*
*******************************************************************************/
#ifndef TEM_PIXEL_H
#define TEM_PIXEL_H

void find_deskewed_pixel_coordinates(const int sensor_index, int* x_point, int* y_point);
void find_raw_pixel_coordinates(const int sensor_index, int* x_point, int* y_point);

int get_raw_pixel(const int sensor_index, const int x_point, const int y_point);
int get_calibrated_raw_pixel(const int sensor_index, const int x_point, const int y_point);
int get_deskewed_pixel(const int sensor_index, int x_point, int y_point);
int get_deskewed_rectangle_average(const int sensor_index, const int x_origin, const int y_origin,
	const int x_length, const int y_length);
int get_deskewed_circle_average(const int sensor_index, const int x_origin, const int y_origin);
int get_calibrated_pixel(const int sensor_index, const int x_point, int value);
int get_calibrated_mesh_area(const int sensor_index, const int total, const int points_used);

unsigned char* get_raw_pixel_address(const int sensor_index, const int x_point, const int y_point);
unsigned char* get_deskewed_pixel_address(const int sensor_index, int x_point, int y_point);
#endif
