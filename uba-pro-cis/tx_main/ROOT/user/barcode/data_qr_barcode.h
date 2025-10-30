/*******************************************************************************
* Project: CIS Bill Acceptor
* File: data_qr_barcode.h
* Contents: qr barcode tables
*
*
*******************************************************************************/
#ifndef DATA_QR_BARCODE_H
#define DATA_QR_BARCODE_H




struct qr_information
{
	int width_size;
	int height_size;
	int data_codeword_count_low;
	int correction_codeword_count_low;
	int rs_block_count_low;
	int data_codeword_count_medium;
	int correction_codeword_count_medium;
	int rs_block_count_medium;
	int data_codeword_count_quality;
	int correction_codeword_count_quality;
	int rs_block_count_quality;
	int data_codeword_count_high;
	int correction_codeword_count_high;
	int rs_block_count_high;
	int* byte_map;
};
typedef struct qr_information QR_INFORMATION;

extern const int qr_21x21_byte_map[21][21];
extern const int qr_25x25_byte_map[25][25];
extern const int qr_29x29_byte_map[29][29];
extern const QR_INFORMATION qr_info[];
extern const int qr_logarithm_table[256] ;
extern const int qr_exponent_table[256];
extern const int qr_alphanumeric_table[QR_BARCODE_ALPHANUMERIC_TABLE_SIZE];

#endif /* DATA_QR_BARCODE_H */

