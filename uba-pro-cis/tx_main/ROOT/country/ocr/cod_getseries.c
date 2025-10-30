#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define EXT

#include "cod_config.h"
#include "cod_utilities.h"
#include "cod_malloc.h"
#include "cod_getseries.h"
#include "svminference.h"

//#include "../common/global.h" // Huong add
#ifdef _BAU_SE_DEBUG_

#include "jcmfakeio.h"

#endif

#ifdef _COD_BAU_SE_

#define BAU_DPI 100

#else

#define BAU_DPI 200

#endif

#ifdef _COD_GETSERIES_RULES_FOR_INR_BANKNOTE_

#define NOF_REF 7

#else

#define NOF_REF -1

#endif

#define max_cod(a, b) (a > b ? a : b)
#define min_cod(a, b) (a < b ? a : b)
#define digits_not_10(x) (x > '1' && x <= '9')

//2020/7/8 debug時以外はコメントアウト
//void log_time(char *s, long long time) {	
//	debug_printf(s);
//	debug_printf(" time: %ld microseconds \n", time);
//}
typedef	unsigned char	u8;
extern u8* matrix_temp_img0;
extern u8* matrix_temp_img1;
extern int* matrix_temp_img_int;
extern float* matrix_temp_float0;
extern float* matrix_temp_float1;
extern float* matrix_temp_float2;
extern unsigned char* matrix_temp_width;
extern unsigned char* matrix_temp_height;

int length_seri_new = 9;
unsigned char show = 0;
void visualize(unsigned char num)	//Huong add 20210818
{
	show = num;
}

void cod_getseries(unsigned char* img_data, const COD_Parameters* para, char* predicted_text, float* predicted_prob) {

#ifdef _BAU_SE_DEBUG_
	print_str("ENDOFCLASSIFICATION ");
	print_str(predicted_text);
	print_char('\n');
	print_char('\0');

	reset_print_pointer();

#endif
	int i, j, false_alphabet_idx, verified_alphabet;
	//int output_size = get_cod_weight_size();
	int output_size = para->cutout_size;	//add furuta 0903
	unsigned char* pt;
	unsigned char* padding_series = detect_series(para->height,
		para->width,
		img_data,
		para->series_length,
		para->switch_binarization_value,
		para->binarization_threshold,
		para->threshold_on_y,
		para->threshold_on_x,
		para->threshold_on_offset,
		para->dpi,
		NOF_REF,
		output_size,
		para->jpy_flg,
		(short*)&para->top,
		(short*)&para->bottom,
		(short*)&para->left,
		(short*)&para->right);

	set_svm_wights((void*)para);	//add by furuta 200827

	if (padding_series) {
		pt = padding_series;
		if (para->jpy_flg == 1)		//add by furua 200908
		{
			if (length_seri_new == 8)
			{
				char character_types_JPY8[] = { 2, 1, 1, 1, 1, 1, 1, 2 };
				for (i = 0; i < length_seri_new; i++)
				{
					predicted_text[i] = svm_predict(character_types_JPY8[i], pt, output_size);
					pt += output_size * output_size;
				}
			}
			else
			{
				for (i = 0; i < length_seri_new; i++)
				{
					predicted_text[i] = svm_predict(para->character_types[i], pt, output_size);
					pt += output_size * output_size;
				}
			}
		}
		else
		{
			for (i = 0; i < para->series_length; i++)
			{
				ir predicted;
				predicted_text[i] = svm_predict(para->character_types[i], pt, output_size);

				////Huong add 20210820
				//svm_predict_new(&predicted, para->character_types[i], pt, output_size);
				// predicted_text[i] = predicted.label;
				// predicted_prob[i] = predicted.prob;

				pt += output_size * output_size;
			}

		}

#ifdef _COD_GETSERIES_RULES_FOR_CNY_BANKNOTE_	//Huong 修正	20210913
		// neu ki tu dau tien la 0 hoac 1 thi gan thanh O hoac I.//予測される最初の文字が0または1の場合、それぞれOとIを返します。
		if (predicted_text[0] == '0')
		{
			predicted_text[0] = 'O';
		}
		else if (predicted_text[0] == '1')
		{
			predicted_text[0] = 'I';
		}
		if (para->character_types[1] == 0) {
			verified_alphabet = 0;
			// test chu+so toi vi tri thu 10	
			for (i = 1; i <= 9; i++) {
				if (para->character_types[i] == 0 && predicted_text[i] > '9'
					&& predicted_text[i] != 'O' && predicted_text[i] != 'I') {
					for (j = 1; j <= 9; j++) {
						if (j != i && para->character_types[j] == 0) {
							pt = padding_series + j * output_size * output_size;
							predicted_text[j] = svm_predict(1, pt, output_size);
						}
					}
					verified_alphabet = i;
					break;
				}
			}

			false_alphabet_idx = 0;
			if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3])
				&& digits_not_10(predicted_text[4]) && digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6])
				&& digits_not_10(predicted_text[7]) && digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]))
			{
				false_alphabet_idx = j = 10;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3])
				&& digits_not_10(predicted_text[4]) && digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6])
				&& digits_not_10(predicted_text[7]) && digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[10]))
			{
				j = 9;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}

			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3])
				&& digits_not_10(predicted_text[4]) && digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6])
				&& digits_not_10(predicted_text[7]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 8;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3])
				&& digits_not_10(predicted_text[4]) && digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6])
				&& digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 7;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3])
				&& digits_not_10(predicted_text[4]) && digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[7])
				&& digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 6;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3])
				&& digits_not_10(predicted_text[4]) && digits_not_10(predicted_text[6]) && digits_not_10(predicted_text[7])
				&& digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 5;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3])
				&& digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6]) && digits_not_10(predicted_text[7])
				&& digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 4;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[4])
				&& digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6]) && digits_not_10(predicted_text[7])
				&& digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 3;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[1]) && digits_not_10(predicted_text[3]) && digits_not_10(predicted_text[4])
				&& digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6]) && digits_not_10(predicted_text[7])
				&& digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 2;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			else if (digits_not_10(predicted_text[2]) && digits_not_10(predicted_text[3]) && digits_not_10(predicted_text[4])
				&& digits_not_10(predicted_text[5]) && digits_not_10(predicted_text[6]) && digits_not_10(predicted_text[7])
				&& digits_not_10(predicted_text[8]) && digits_not_10(predicted_text[9]) && digits_not_10(predicted_text[10]))
			{
				j = 1;
				pt = padding_series + j * output_size * output_size;
				predicted_text[j] = svm_predict(2, pt, output_size);
			}
			if (verified_alphabet == 0 && false_alphabet_idx == 0) {
				for (i = 1; i <= 9; i++) {
					if (predicted_text[i] == '1' || predicted_text[i] == 'I') {
						predicted_text[i] = 'I';
						for (j = 1; j <= 9; j++) {
							if (j != i && para->character_types[j] == 0) {
								pt = padding_series + j * output_size * output_size;
								predicted_text[j] = svm_predict(1, pt, output_size);
							}
						}
						break;
					}
					else if (predicted_text[i] == '0' || predicted_text[i] == 'O') {
						predicted_text[i] = 'O';
						for (j = 1; j <= 9; j++) {
							if (j != i && para->character_types[j] == 0) {
								pt = padding_series + j * output_size * output_size;
								predicted_text[j] = svm_predict(1, pt, output_size);
							}
						}
						break;
					}
				}
			}
		}
#endif
	}
	else {
		for (i = 0; i < para->series_length; i++) {
			predicted_text[i] = '?';
		}
	}
	predicted_text[para->series_length] = 0;


	//#ifndef _COD_BAU_SE_
	free_proc(padding_series);
	//#endif

		//reset_heap();
}

unsigned char* detect_series(int height, int width, unsigned char* gray_img, int series_length,
	int switch_binarization_value, int binarization_threshold, int threshold_on_y,
	int threshold_on_x, int threshold_on_offset, int dpi, int nof_ref, int output_size, char jpy_flg,
	short* ptop, short* pbottom, short* pleft, short* pright) {


	int region_r_on_offset = (int)(1.5 * (dpi / 100));
	int top, bottom;
	// sửa malloc này
	int* left = malloc_int(series_length);
	int* right = malloc_int(series_length);

	// đã sửa malloc này
	//unsigned char* img = (unsigned char*)malloc_char(height * width);
	unsigned char* img = &(*matrix_temp_img1);	// anh binary

	unsigned char* roi, * cut_roi, * make_border_roi, * pt;
	unsigned char* padding_img = 0;


	int i = 0;
	int t = 0, b = 0;
	int roi_width = 0;
	int roi_height = 0;

	int sz = 0;
	int num_cc = 0;

	if (jpy_flg)
	{
		num_cc = detect_series_cc_JPY(height, width, gray_img, img, &series_length,
			switch_binarization_value, binarization_threshold, threshold_on_y, threshold_on_x,
			threshold_on_offset, dpi, &top, &bottom, left, right, nof_ref, jpy_flg);
	}
	else
	{
		// đã check trong hàm 
		num_cc = detect_series_cc(height, width, gray_img, img, series_length,
			switch_binarization_value, binarization_threshold, threshold_on_y, threshold_on_x,
			threshold_on_offset, dpi, &top, &bottom, left, right, nof_ref);
	}

	*ptop = top;
	*pbottom = bottom;
	*pleft = left[0];
	*pright = right[series_length - 1];

	if (num_cc == series_length) {

		// đã sửa malloc này
		padding_img = (unsigned char*)malloc_char(series_length * output_size * output_size);

		//padding_img = matrix_temp_img0;
		pt = padding_img;
		for (i = 0; i < series_length; i++) {
			// đã sửa malloc này
			roi = &(*matrix_temp_img0);
			get_roi(img, roi, width, height, left[i] - 1, top, right[i], bottom);
			roi_width = right[i] - left[i] + 2;
			roi_height = bottom - top + 1;
			cut_bottom_top_offset(roi, roi_height, roi_width, &t, &b, region_r_on_offset, threshold_on_offset);
			// chỗ này sẽ sửa sau
			// đã sửa malloc này
			//cut_roi = (unsigned char*)malloc_char(roi_width *(b-t+1));
			cut_roi = &(*matrix_temp_img1);	// matrix_temp_img1 đang lưu mảng "img" đến đây không cần dùng img nữa.
			get_roi(roi, cut_roi, roi_width, roi_height, 0, t, roi_width - 1, b);
			make_border_roi = copy_make_border(cut_roi, b - t + 1, roi_width);
			sz = max_cod(b - t + 1, roi_width);

			//			make_border_roi = copy_make_border(roi, roi_height, roi_width);
			//			sz = max(roi_height, roi_width);
			bilinear_resize_new(make_border_roi, pt, sz, sz, output_size, output_size);

			pt += output_size * output_size;
			//#ifndef _COD_BAU_SE_	
						//SELE問わず実行するように変更 20200713
						//free_proc(roi);
						//free_proc(cut_roi);	//実行するように変更 20200713
						//free_proc(make_border_roi);
			//#endif
		}

	}


	//#ifndef _COD_BAU_SE_
		//free_proc(img);
	free_proc(left);
	free_proc(right);
	//#endif
	return padding_img;
}

//default arguments
//double_binarization=0, binarization_threshold=45, threshold_on_y=40, 
//threshold_on_x=20, threshold_on_offset=20, output_size = 28, dpi=200
//switch_binarization_value=110

int detect_series_cc_JPY(int height, int width, unsigned char* gray_img, unsigned char* ret_img, int* series_length,
	int switch_binarization_value, int binarization_threshold, int threshold_on_y,
	int threshold_on_x, int threshold_on_offset, int dpi,
	int* top, int* bottom, int* left, int* right, int nof_ref, char jpy_flg)
{

	int filter_threshold = (int)(1.0 * (dpi / 100));
	int region_r_on_x = (int)(1.5 * (dpi / 100));  // 1.5 is value for 100dpi
	int region_r_on_y = (int)(2 * (dpi / 100));    // 2 is value for 100 dpi
	unsigned char* img = (unsigned char*)malloc_char(height * width);
	unsigned char* img2 = (unsigned char*)malloc_char(height * width);
	int* histogram_on_y = malloc_int(height);
	int* histogram_on_x = malloc_int(width);
	int* histogram_on_x_length = malloc_int(width);	//Huong
	unsigned char* binarized_his_on_y = (unsigned char*)malloc_char(height);
	unsigned char* binarized_his_on_x = (unsigned char*)malloc_char(width);
	unsigned char* binarized_his_on_x_length = (unsigned char*)malloc_char(width);	//Huong

	int code = 0;
	int r = 0, i = 0;
	int average_density = 0;
	int vertical_len = 0;

	//
	unsigned char r_noise = 5;	//Huong
	int thresh_pixel = 1275;	//Huong: chon nguong delete_noise = r*(1*255)
	int sum_x = 0;
	int x = 0;
	//int ii = 0;

	int left0 = 0;
	int right0 = 0;
	int length_seri = 0;


	triple_faster_bradley_threshold(height, width, gray_img, img, img2, ret_img, (float)binarization_threshold, 11, 1.2f, 1.5f);

	project_on_y(img, histogram_on_y, height, width);
	region_avarage_bradley_threshold_1D(histogram_on_y, binarized_his_on_y, height, region_r_on_y, 101, (float)threshold_on_y); // 200dpi: r=4, threshold=40. 100dpi: r=2, threshold=60
	get_largest_binarized_CC1D(binarized_his_on_y, histogram_on_y, height, 1, top, bottom);

	*top = max_cod(0, *top - 2);
	*bottom = min_cod(height - 1, *bottom + 1);

	average_density = 0;
	for (i = *top; i <= *bottom; i++) {
		average_density += histogram_on_y[i];
	}
	vertical_len = *bottom - *top;
	average_density /= vertical_len * height;
	if (average_density <= switch_binarization_value) {
		for (i = 0; i < height * width; i++) {
			img[i] = img2[i];
		}
	}
	else {
		for (i = 0; i < height * width; i++) {
			ret_img[i] = img2[i];
		}
	}

	project_on_x(img, histogram_on_x, *top, *bottom, width);
	//histogram_on_x_length = &(*histogram_on_x);
	project_on_x(img, histogram_on_x_length, *top, *bottom, width);
	//unsigned char r_noise = 5;	//Huong
	//int thresh_pixel = 1275;	//Huong: chon nguong delete_noise = r*(1*255)
	//int sum_x;
	//Xoa noise phia truoc
	for (x = 0; x < width - r_noise; x += r_noise)
	{
		sum_x = 0;
		for (i = x; i < x + r_noise; i++)
		{
			sum_x = sum_x + histogram_on_x_length[i];
		}
		if (sum_x < thresh_pixel)
		{
			for (i = x; i < x + r_noise; i++)
			{
				histogram_on_x_length[i] = 0;
			}
		}
		else
		{
			break;
		}
	}
	//Xoa noise phia sau
	for (x = width - 1; x > r_noise; x -= r_noise)
	{
		sum_x = 0;
		for (i = x; i > x - r_noise; i--)
		{
			sum_x = sum_x + histogram_on_x_length[i];
		}
		if (sum_x < thresh_pixel)
		{
			for (i = x; i > x - r_noise; i--)
			{
				histogram_on_x_length[i] = 0;
			}
		}
		else
		{
			break;
		}
	}

	for (r = region_r_on_x; r >= 0; r--) {
		region_avarage_bradley_threshold_1D(histogram_on_x, binarized_his_on_x, width, r, vertical_len, (float)threshold_on_x);
		region_avarage_bradley_threshold_1D(histogram_on_x_length, binarized_his_on_x_length, width, r, vertical_len, (float)threshold_on_x);	//Huong
		remove_boundary_noise(binarized_his_on_x, width);
		remove_boundary_noise(binarized_his_on_x_length, width);	//Huong
		filter_noise_segmentation(binarized_his_on_x, width, filter_threshold + 2);
		merge_H_segmentation(binarized_his_on_x, width, filter_threshold + 1 + r, 2, nof_ref);

		if (jpy_flg == 1)
		{
			for (i = 0; i < width; i++) {
				if (binarized_his_on_x_length[i] > 0) //vi tri co ki tu left
				{
					left0 = i;
					break;
				}
			}
			for (i = width - 1; i > left0; i--) {
				if (binarized_his_on_x_length[i] > 0) //vi tri co ki tu right
				{
					right0 = i;
					break;
				}
			}

			length_seri = right0 - left0;//chieu dai so seri
			if (length_seri < 190)
			{
				*series_length = 8;
				length_seri_new = 8;

			}
			else
			{
				*series_length = 9;
				length_seri_new = 9;
			}
		}

		if (get_largest_binarized_CC1D(binarized_his_on_x, histogram_on_x, width, *series_length, left, right) == *series_length) {
			code = *series_length;
			break;
		}
	}

#ifndef _COD_BAU_SE_
	free_proc(img);
	free_proc(img2);
	free_proc(histogram_on_x);
	free_proc(histogram_on_y);
	free_proc(binarized_his_on_x);
	free_proc(binarized_his_on_y);
	free_proc(histogram_on_x_length);	//Huong
	free_proc(binarized_his_on_x_length);	//Huong
#endif

	return code;
}

int detect_series_cc(int height, int width, unsigned char* gray_img, unsigned char* ret_img, int series_length,
	int switch_binarization_value, int binarization_threshold, int threshold_on_y,
	int threshold_on_x, int threshold_on_offset, int dpi,
	int* top, int* bottom, int* left, int* right, int nof_ref) {
	int filter_threshold = (int)(1.0 * (dpi / 100));
	int region_r_on_x = (int)(1.5 * (dpi / 100));  // 1.5 is value for 100dpi
	int region_r_on_y = (int)(2 * (dpi / 100));    // 2 is value for 100 dpi
	unsigned char* img = (unsigned char*)malloc_char(height * width);
	unsigned char* img2 = (unsigned char*)malloc_char(height * width);

	// đã sửa malloc này // mảng matrix_img_int đang không sử dụng nên gán cho mảng dưới
	//int* histogram_on_y = malloc_int(height);
	int* histogram_on_y = &(*matrix_temp_img_int);

	// sửa malloc này
	int* histogram_on_x = malloc_int(width);

	unsigned char* binarized_his_on_y = (unsigned char*)malloc_char(height);
	unsigned char* binarized_his_on_x = (unsigned char*)malloc_char(width);
	/*unsigned char* binarized_his_on_y = matrix_temp_height;
	unsigned char* binarized_his_on_x = matrix_temp_width;*/

	int code = 0;
	int r, i;
	int average_density = 0;
	int vertical_len = 0;

	triple_faster_bradley_threshold(height, width, gray_img, img, img2, ret_img, (float)binarization_threshold, 11, 1.2f, 1.5f);
	project_on_y(img, histogram_on_y, height, width);
	region_avarage_bradley_threshold_1D(histogram_on_y, binarized_his_on_y, height, region_r_on_y, 101, (float)threshold_on_y); // 200dpi: r=4, threshold=40. 100dpi: r=2, threshold=60
	get_largest_binarized_CC1D(binarized_his_on_y, histogram_on_y, height, 1, top, bottom);

	*top = max_cod(0, *top - 2);
	*bottom = min_cod(height - 1, *bottom + 1);

	if (*bottom < *top) return 0;

	average_density = 0;
	for (i = *top; i <= *bottom; ++i) {
		average_density += histogram_on_y[i];
	}
	vertical_len = *bottom - *top;
	average_density /= vertical_len * height;


	if (average_density <= switch_binarization_value) {
		for (i = 0; i < height * width; ++i) {
			img[i] = img2[i];
		}
	}
	else {
		for (i = 0; i < height * width; ++i) {
			ret_img[i] = img2[i];
		}
	}

	project_on_x(img, histogram_on_x, *top, *bottom, width);
	for (r = region_r_on_x; r >= 0; r--) {
		region_avarage_bradley_threshold_1D(histogram_on_x, binarized_his_on_x, width, r, vertical_len, (float)threshold_on_x); // 
		remove_boundary_noise(binarized_his_on_x, width);
		filter_noise_segmentation(binarized_his_on_x, width, filter_threshold + 2);
		merge_H_segmentation(binarized_his_on_x, width, filter_threshold + 1 + r, 2, nof_ref);
		if (get_largest_binarized_CC1D(binarized_his_on_x, histogram_on_x, width, series_length, left, right) == series_length) {
			code = series_length;
			break;
		}
	}

	//#ifndef _COD_BAU_SE_
	free_proc(img);
	free_proc(img2);
	free_proc(histogram_on_x);
	//free_proc(histogram_on_y);
	free_proc(binarized_his_on_x);
	free_proc(binarized_his_on_y);
	//#endif
	return code;
}


void cod_getcolor(const COD_Parameters* para, char* predicted_text, unsigned char* r_data, unsigned char* g_data, unsigned char* b_data)
{
	int region_r_on_offset = (int)(1.5 * (para->dpi / 100));
	int top, bottom;
	int* left = malloc_int(para->series_length);
	int* right = malloc_int(para->series_length);
	unsigned char* img = (unsigned char*)malloc_char(para->height * para->width);
	unsigned char* roi_1, * roi_2, * roi_3, * roi_r, * roi_g, * roi_b, * cut_roi_g;
	float roi_svm[2352];
	//float roi_svm[COLOR_MODEL_INPUT_SIZE* COLOR_MODEL_INPUT_SIZE * 3];
	int i;
	int size = 0;
	int roi_width = 0;
	int roi_height = 0;

	int series_length = para->series_length;
	char color_predict = 0;

	//int num_cc = detect_series_cc_JPY(para->height, para->width, g_data, img, &series_length,
	//	para->switch_binarization_value, para->binarization_threshold, para->threshold_on_y, para->threshold_on_x,
	//	para->threshold_on_offset, para->dpi, &top, &bottom, left, right, NOF_REF);

	/*int num_cc = detect_series_cc_color(para->height, para->width, g_data, img, 8,
		para->switch_binarization_value, para->binarization_threshold, para->threshold_on_y, para->threshold_on_x,
		para->threshold_on_offset, para->dpi, &top, &bottom, left, right, NOF_REF);*/
	int num_cc = detect_series_cc_JPY(para->height, para->width, g_data, img, &series_length,
		para->switch_binarization_value, para->binarization_threshold, para->threshold_on_y, para->threshold_on_x,
		para->threshold_on_offset, para->dpi, &top, &bottom, left, right, NOF_REF, para->jpy_flg);

	int t = 0, b = 0;

	int w = 0;
	int h = 0;
	int roi_width_g = 0;
	int roi_height_g = 0;

	for (i = 0; i < num_cc; i++)
	{

		int roi_width_g = right[i] - left[i] + 2;
		roi_height_g = bottom - top + 1;
		// Huong 変更20220720
		//cut_roi_g = get_roi(img, para->width, para->height, left[i] - 1, top, right[i], bottom);
		cut_roi_g = (unsigned char*)malloc_char((right[i] - (left[i] - 1) + 1) * (bottom - top + 1));
		get_roi(img, cut_roi_g, para->width, para->height, left[i] - 1, top, right[i], bottom);

		cut_bottom_top_offset(cut_roi_g, roi_height_g, roi_width_g, &t, &b, region_r_on_offset, para->threshold_on_offset);

		roi_width = right[i] - left[i];
		roi_height = b - t + 1;

		//Huong chỗ này sẽ tối ưu tốc độ sau
		w = b - t + 1;
		h = right[i] - 1 - left[i] + 1;
		roi_g = (unsigned char*)malloc_char(w * h);
		roi_b = (unsigned char*)malloc_char(w * h);
		roi_r = (unsigned char*)malloc_char(w * h);

		get_roi(g_data, roi_g, para->width, para->height, left[i], top + t, right[i] - 1, top + b);
		get_roi(b_data, roi_b, para->width, para->height, left[i], top + t, right[i] - 1, top + b);
		get_roi(r_data, roi_r, para->width, para->height, left[i], top + t, right[i] - 1, top + b);

		roi_1 = (unsigned char*)malloc_char(COLOR_MODEL_INPUT_SIZE * COLOR_MODEL_INPUT_SIZE);
		roi_2 = (unsigned char*)malloc_char(COLOR_MODEL_INPUT_SIZE * COLOR_MODEL_INPUT_SIZE);
		roi_3 = (unsigned char*)malloc_char(COLOR_MODEL_INPUT_SIZE * COLOR_MODEL_INPUT_SIZE);

		bilinear_resize(roi_b, roi_1, roi_width, roi_height, COLOR_MODEL_INPUT_SIZE, COLOR_MODEL_INPUT_SIZE);
		bilinear_resize(roi_g, roi_2, roi_width, roi_height, COLOR_MODEL_INPUT_SIZE, COLOR_MODEL_INPUT_SIZE);
		bilinear_resize(roi_r, roi_3, roi_width, roi_height, COLOR_MODEL_INPUT_SIZE, COLOR_MODEL_INPUT_SIZE);

		for (size = 0; size < COLOR_MODEL_INPUT_SIZE * COLOR_MODEL_INPUT_SIZE * 3; size++)
		{
			if ((size + 3) % 3 == 0)
			{
				roi_svm[size] = (float)roi_1[size / 3] / 255;
			}
			else if ((size + 3) % 3 == 1)
			{
				roi_svm[size] = (float)roi_2[size / 3] / 255;
			}
			else
			{
				roi_svm[size] = (float)roi_3[size / 3] / 255;
			}
		}

		color_predict = svm_predict_color(roi_svm, para->model_color);
		predicted_text[i] = color_predict;

		free_proc(roi_r);
		free_proc(roi_g);
		free_proc(roi_b);
		free_proc(cut_roi_g);
		//free_proc(roi_svm);	//roi_svmは普通の配列

		free_proc(roi_1);
		free_proc(roi_2);
		free_proc(roi_3);
	}

#ifndef _COD_BAU_SE_
	free_proc(img);
	free_proc(left);
	free_proc(right);
#endif
}


int detect_series_cc_color(int height, int width, unsigned char* gray_img, unsigned char* ret_img, int series_length,
	int switch_binarization_value, int binarization_threshold, int threshold_on_y,
	int threshold_on_x, int threshold_on_offset, int dpi,
	int* top, int* bottom, int* left, int* right, int nof_ref) {
	int filter_threshold = (int)(1.0 * (dpi / 100));
	int region_r_on_x = (int)(1.5 * (dpi / 100));  // 1.5 is value for 100dpi
	int region_r_on_y = (int)(2 * (dpi / 100));    // 2 is value for 100 dpi
	unsigned char* img = (unsigned char*)malloc_char(height * width);
	unsigned char* img2 = (unsigned char*)malloc_char(height * width);

	// sửa malloc này
	int* histogram_on_y = malloc_int(height);
	int* histogram_on_x = malloc_int(width);
	unsigned char* binarized_his_on_y = (unsigned char*)malloc_char(height);
	unsigned char* binarized_his_on_x = (unsigned char*)malloc_char(width);

	int code = 0;
	int r, i;
	int average_density = 0;
	int vertical_len = 0;

	triple_faster_bradley_threshold(height, width, gray_img, img, img2, ret_img, (float)binarization_threshold, 11, 1.2f, 1.5f);

	project_on_y(img, histogram_on_y, height, width);
	region_avarage_bradley_threshold_1D(histogram_on_y, binarized_his_on_y, height, region_r_on_y, 101, (float)threshold_on_y);
	get_largest_binarized_CC1D(binarized_his_on_y, histogram_on_y, height, 1, top, bottom);

	*top = max_cod(0, *top - 2);
	*bottom = min_cod(height - 1, *bottom + 1);

	average_density = 0;
	for (i = *top; i <= *bottom; i++) {
		average_density += histogram_on_y[i];
	}
	vertical_len = *bottom - *top;
	average_density /= vertical_len * height;
	if (average_density <= switch_binarization_value) {
		for (i = 0; i < height * width; i++) {
			img[i] = img2[i];
		}
	}
	else {
		for (i = 0; i < height * width; i++) {
			ret_img[i] = img2[i];
		}
	}

	project_on_x(img, histogram_on_x, *top, *bottom, width);
	for (r = region_r_on_x; r >= 0; r--) {
		region_avarage_bradley_threshold_1D(histogram_on_x, binarized_his_on_x, width, r, vertical_len, (float)threshold_on_x);
		remove_boundary_noise(binarized_his_on_x, width);
		filter_noise_segmentation(binarized_his_on_x, width, filter_threshold + 2);
		merge_H_segmentation(binarized_his_on_x, width, filter_threshold + 1 + r, 2, nof_ref);
		/*
				gbinarized_his_on_x = binarized_his_on_x;
				ghistogram_on_x = histogram_on_x;
				gwidth = width;
				gseries_length = series_length;
				gleft = *left;
				gright = *right;
		*/
		if (get_largest_binarized_CC1D(binarized_his_on_x, histogram_on_x, width, series_length, left, right) == series_length) {
			code = series_length;
			break;
		}
	}

#ifndef _COD_BAU_SE_
	free_proc(img);
	free_proc(img2);
	free_proc(histogram_on_x);
	free_proc(histogram_on_y);
	free_proc(binarized_his_on_x);
	free_proc(binarized_his_on_y);
#endif

	return code;
}

