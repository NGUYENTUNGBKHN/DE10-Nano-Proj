/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_edge.h
* Contents: edge detection and deskew processing
*
*
*******************************************************************************/
#ifndef TEM_EDGE_H
#define TEM_EDGE_H

void run_edge_routine(void);

void edge_level_check(void);

void remove_corner_points(void);
void remove_corner_points_x(int data_set[EDGE_PIXEL_COUNT_X]);
void remove_corner_points_y(int data_set[EDGE_PIXEL_COUNT_Y]);

void find_edge_slope(void);
void add_slopes_x(int* const slope_count, float slopes[EDGE_MAX_SLOPE_COUNT], int data_set[EDGE_PIXEL_COUNT_X]);
void add_slopes_y(int* const slope_count, float slopes[EDGE_MAX_SLOPE_COUNT], int data_set[EDGE_PIXEL_COUNT_Y]);

void remove_invalid_slopes(void);
void remove_invalid_slopes_x(int data_set[EDGE_PIXEL_COUNT_X]);
void remove_invalid_slopes_y(int data_set[EDGE_PIXEL_COUNT_Y]);

void find_linear_regression(void);
void find_linear_regression_x(int data_set[EDGE_PIXEL_COUNT_X], float* const slope, float* const y_intercept);
void find_linear_regression_y(int data_set[EDGE_PIXEL_COUNT_Y], float* const slope, float* const y_intercept);

void remove_outliers(void);
void remove_outliers_x(int data_set[EDGE_PIXEL_COUNT_X], const float slope, const float y_intercept);
void remove_outliers_y(int data_set[EDGE_PIXEL_COUNT_Y], const float slope, const float x_intercept);

void find_raw_corner_coordinates(void);
void find_skew_values(void);
void find_deskewed_corner_coordinates(void);
void find_uv_coordinates(void);

void find_bill_size(void);

void find_calibration_values(void);
void find_calibration_areas(void);

#endif
