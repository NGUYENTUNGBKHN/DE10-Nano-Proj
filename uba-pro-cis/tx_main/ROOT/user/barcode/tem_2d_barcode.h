/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_2d_barcode.h
* Contents: common 2 dimensional barcode
*
*
*******************************************************************************/
#ifndef TEM_2D_BARCODE_H
#define TEM_2D_BARCODE_H


int get_2d_barcode_left_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length);
int get_2d_barcode_right_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length);
int get_2d_barcode_top_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length);
int get_2d_barcode_bottom_edge(P_BARCODE_2D_RESULT const barcode_result, const int edge_level,
	const int edge_white_area_length);
int get_2d_barcode_edge_search_value_x(const int sensor, const int x_point, const int y_point);
int get_2d_barcode_edge_search_value_y(const int sensor, const int x_point, const int y_point);
int get_2d_barcode_rotated_pixel(P_BARCODE_2D_RESULT const barcode_result,
	const int x_point, const int y_point);
int get_2d_barcode_cell_average(P_BARCODE_2D_RESULT const barcode_result,
	const int x_cell, const int y_cell);
int get_2d_barcode_enhanced_cell_average(P_BARCODE_2D_RESULT const barcode_result,
	const int x_cell, const int y_cell);

#endif
