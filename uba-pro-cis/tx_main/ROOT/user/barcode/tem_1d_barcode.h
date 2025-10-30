/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_1d_barcode.h
* Contents: common 1 dimensional barcode
*
*
*******************************************************************************/
#ifndef TEM_1D_BARCODE_H
#define TEM_1D_BARCODE_H

void create_1d_barcode_wave(P_BARCODE_1D_RESULT const barcode_result);
int get_1d_barcode_left_edge(const int start_point, const int end_point, const int edge_level,
	const int edge_white_area_length);
int get_1d_barcode_right_edge(const int start_point, const int end_point, const int edge_level,
	const int edge_white_area_length);
int get_1d_barcode_black_white_average(const int start_point, const int end_point);
void parse_1d_barcode(const int left_edge, const int right_edge, const int black_white_average);

#endif
