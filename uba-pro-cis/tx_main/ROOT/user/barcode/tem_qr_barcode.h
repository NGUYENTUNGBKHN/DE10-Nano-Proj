/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_matrix_barcode.h
* Contents: handle QR barcode recognition
*
*
*******************************************************************************/
#ifndef TEM_QR_BARCODE_H
#define TEM_QR_BARCODE_H

void qr_barcode_search(const int sensor);
void qr_barcode_find_bottom_top_edges(void);
void qr_barcode_find_left_right_edges(void);
void qr_barcode_search_border(void);
void qr_barcode_search_format(void);
int qr_barcode_format_is_correct(int format);
int qr_barcode_check_format(int format);
int qr_barcode_get_hamming_weight(int value);
int qr_barcode_decode_format(int format);
int qr_barcode_get_mask_value(const int x_cell, const int y_cell);
void qr_barcode_search_cells(void);
void qr_barcode_read_cells(void);
int qr_barcode_get_cell_value(const int x_cell, const int y_cell);
int qr_barcode_get_enhanced_cell_value(const int x_cell, const int y_cell);
void qr_barcode_decode_message(void);
int qr_barcode_get_group_from_polynomial(const int polynomial_bit_index, const int length);
int qr_barcode_get_bit_from_polynomial(const int polynomial_bit_index);

#endif
