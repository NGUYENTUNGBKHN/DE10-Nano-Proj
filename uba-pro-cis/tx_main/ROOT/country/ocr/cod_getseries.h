#ifndef _GETSERIES_H
#define _GETSERIES_H

/*
class C_Judge;

class GETSERIES
{
	public:
		GETSERIES(void){};
		virtual ~GETSERIES(void){};
};
*/
#ifdef __cplusplus
extern "C" {
#endif

	void visualize(unsigned char num);
	void cod_getseries(unsigned char* img_data, const COD_Parameters* para, char* predicted_text, float* predicted_prob);
	unsigned char* detect_series(int height, int width, unsigned char* gray_img, int series_length,
		int switch_binarization_value, int binarization_threshold, int threshold_on_y,
		int threshold_on_x, int threshold_on_offset, int dpi, int nof_ref, int output_size, char jpy_flg,
		short* top, short* bottom, short* left, short* right);

	int detect_series_cc(int height, int width, unsigned char* gray_img, unsigned char* ret_img, int series_length,
		int switch_binarization_value, int binarization_threshold, int threshold_on_y,
		int threshold_on_x, int threshold_on_offset, int dpi,
		int* top, int* bottom, int* left, int* right, int nof_ref);

	int detect_series_cc_JPY(int height, int width, unsigned char* gray_img, unsigned char* ret_img, int* series_length,
		int switch_binarization_value, int binarization_threshold, int threshold_on_y,
		int threshold_on_x, int threshold_on_offset, int dpi,
		int* top, int* bottom, int* left, int* right, int nof_ref, char jpy_flg);

	// êFîªíË
#ifdef VALID_JPY_OCR
	void cod_getcolor(const COD_Parameters* para, char* predicted_text, unsigned char* r_data, unsigned char* g_data, unsigned char* b_data);
	int detect_series_cc_color(int height, int width, unsigned char* gray_img, unsigned char* ret_img, int series_length,
		int switch_binarization_value, int binarization_threshold, int threshold_on_y,
		int threshold_on_x, int threshold_on_offset, int dpi,
		int* top, int* bottom, int* left, int* right, int nof_ref);
#endif

#ifdef __cplusplus
}
#endif

#endif
