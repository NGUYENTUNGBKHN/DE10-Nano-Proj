/*
 * tem_main_barcode.h
 *
 *  Created on: 2019/05/22
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_MAIN_USER_BARCODE_TEM_MAIN_BARCODE_H_
#define SRC_MAIN_USER_BARCODE_TEM_MAIN_BARCODE_H_


void run_tito_validation_routine(const int search_type);
void run_qr_validation_routine(const int search_type);


void set_1d_barcode_center_points_horizontal(const int sensor);
void set_1d_barcode_center_points_vertical(const int sensor);
void set_2d_barcode_search_points_middle(const int sensor);
void set_2d_barcode_search_points_tito(const int sensor);
void set_2d_barcode_search_points_full(const int sensor);
void tito_ticket_ir_check(void);
int is_double_ticket(void);
int ticket_size_check(void);
void ticket_indexmark_check(void);
void clear_itf_barcode_values(void);

#endif /* SRC_MAIN_USER_BARCODE_TEM_MAIN_BARCODE_H_ */
