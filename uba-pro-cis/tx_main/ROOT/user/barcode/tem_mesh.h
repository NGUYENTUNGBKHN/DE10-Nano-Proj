/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_mesh.h
* Contents: mesh area processing
*
*
*******************************************************************************/
#ifndef TEM_MESH_H
#define TEM_MESH_H

void run_mesh_routine(void);
void find_area_limits(void);
void find_average_values(void);
void normalize_mesh_area_by_direction(const int direction, int* sensor_index, int* area_x, int* area_y);
void normalize_network_area_by_direction(const int direction, int* sensor_index, int* area_x, int* area_y);
void normalize_pixel_by_direction(const int direction, int* sensor_index, int* x_point, int* y_point);
void normalize_mesh_square_pixel_by_direction(const int direction, int* sensor_index, int* x_point, int* y_point);

int get_mesh_area_average(const int sensor_index, const int x_area, const int y_area);
int get_unnormalized_mesh_area_average(const int sensor_index, const int x_area, const int y_area);

int get_mesh_cross_area_x(const int area_x, const int area_y);
int get_mesh_cross_area_y(const int area_x, const int area_y);


#endif
