/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_itf_barcode.c
* Contents: interleaved 2 of 5 barcode format
*
*
*******************************************************************************/
#ifndef TEM_ITF_BARCODE_H
#define TEM_ITF_BARCODE_H

void itf_barcode_search(const int sensor, const int orientation);
void itf_barcode_search_test(const int sensor, const int orientation);
void itf_barcode_find_edges(const int edge_level);
void itf_barcode_check_bars(void);
void itf_barcode_find_area_averages(void);
void itf_barcode_find_ratios(void);
void itf_barcode_find_direction(void);
void itf_barcode_decode(void);

#endif
