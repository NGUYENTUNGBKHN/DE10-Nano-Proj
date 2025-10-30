
#define EXTERN extern
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "cod_config.h"
#include "cod_malloc.h"
#include "cod_utilities.h"

#define cod_max(a, b) (a > b ? a : b)
#define cod_min(a, b) (a < b ? a : b)

typedef	unsigned char	u8;
extern u8* matrix_temp_img0;
extern u8* matrix_temp_img1;
extern int* matrix_temp_img_int;
extern float* matrix_temp_float0;
extern float* matrix_temp_float1;
extern float* matrix_temp_float2;
extern unsigned char* matrix_temp_width;
extern unsigned char* matrix_temp_height;

float* uniform_filter_1D(int* arr, float* means, int len, int r) {
	// đã sửa malloc này
	//float *integral = malloc_float(len + r + r + 1);
	float* integral = &(*matrix_temp_float0);

	// đã sửa malloc này // do hàm region_avarage_bradley_threshold_1D có 2 biến dùng mảng này nên phải sửa lại code
	//float *means = malloc_float(len);
	//float *means = matrix_temp_float1;


	float* start = integral + r + 1;
	int i, j;

	for (i = 1, j = 0; i <= r; i++) {
		start[-i] = (float)arr[j++];
		start[len + i - 1] = (float)arr[len - j];
		if (j == len) {
			j = 0;
		}
	}
	for (i = 0; i < len; i++) {
		start[i] = (float)arr[i];
	}
	start[-r - 1] = 0;
	for (i = -r; i < len + r; i++) {
		start[i] += start[i - 1];
	}
	j = 2 * r + 1;
	for (i = 0; i < len; i++) {
		means[i] = (float)(start[i + r] - start[i - r - 1]) / j;
	}

#ifndef _COD_BAU_SE_
	//	free_proc(integral);
#endif

	return means;
}
#ifdef huyou
float* uniform_filter_1D_new(int* arr, int len, int r) {
	int* integral = malloc_int(len + r + r + 1);
	float* means = malloc_float(len);
	int* start = integral + r + 1;
	int i, j;

	for (i = 1, j = 0; i <= r; ++i) {
		start[-i] = arr[j++];
		start[len + i - 1] = arr[len - j];
		if (j == len) {
			j = 0;
		}
	}
	for (i = 0; i < len; ++i) {
		start[i] = arr[i];
	}
	start[-r - 1] = 0;
	for (i = -r; i < len + r; ++i) {
		start[i] += start[i - 1];
	}
	j = 2 * r + 1;
	for (i = 0; i < len; ++i) {
		means[i] = (float)(start[i + r] - start[i - r - 1]) / j;
	}

	//#ifndef _COD_BAU_SE_
	free_proc(integral);
	//#endif

	return means;
}


/***
	Uniform filter 2D - Padding r, value in padding get symmetry in row-0, col-0, row-height, col-width

	@param *arr: Data of image.
	@param height: Number row of image.
	@param width: Number col of image.
	@param r: Radius of kernel. Size of kernel = 2*r
	@return: Mean data after uniform.
***/

float* uniform_filter_2D(unsigned char* arr, int height, int width, int r) {

	int window = r + r + 1;
	int integral_height = height + window;
	int integral_width = width + window;
	float* integral = malloc_float(integral_height * integral_width);

	float* res = malloc_float(height * width);
	float sum;
	int i, j, k, x, y;

	memset(integral, 0, sizeof(float) * (integral_width));

	for (i = 1, j = 0; i <= r; i++) {
		for (k = 0; k < width; k++) {
			integral[(r + 1 - i) * integral_width + r + 1 + k] = arr[j * width + k];
		}
		j++;
		for (k = 0; k < width; k++) {
			integral[(r + 1 + height + i - 1) * integral_width + r + 1 + k] = arr[(height - j) * width + k];
		}
		if (j == height) {
			j = 0;
		}
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			integral[(r + 1 + i) * integral_width + r + 1 + j] = arr[i * width + j];
		}
	}
	for (i = 1, j = 0; i <= r; i++) {
		for (k = -r; k < height + r; k++) {
			integral[(r + 1 + k) * integral_width + r + 1 - i] = integral[(r + 1 + k) * integral_width + r + 1 + j];
		}
		j++;
		for (k = -r; k < height + r; k++) {
			integral[(r + 1 + k) * integral_width + r + width + i] = integral[(r + 1 + k) * integral_width + r + 1 + width - j];
		}
		if (j == width) {
			j = 0;
		}
	}
	for (i = -r; i < height + r; i++) {
		sum = 0;
		integral[(r + 1 + i) * integral_width] = 0;
		for (j = -r; j < width + r; j++) {
			sum += integral[(r + 1 + i) * integral_width + r + 1 + j];
			integral[(r + 1 + i) * integral_width + r + 1 + j] = sum + integral[(r + i) * integral_width + r + 1 + j];
		}
	}
	k = window * window;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			sum = integral[(r + 1 + i + r) * integral_width + r + 1 + j + r]//start[i + r][j + r] 
				- integral[i * integral_width + r + 1 + j + r]//start[i - r - 1][j + r] 
				- integral[(r + 1 + i + r) * integral_width + j]//start[i + r][j - r - 1]
				+ integral[i * integral_width + j];//start[i - r - 1][j - r - 1];
			res[i * width + j] = sum / k;
		}
	}

	//#ifndef _COD_BAU_SE_
	free_proc(integral);
	//#endif
	return res;
}
#endif

float* uniform_filter_2D_new(unsigned char* arr, int height, int width, int r) {
	int window = r + r + 1;
	int integral_height = height + window;
	int integral_width = width + window;

	// đã sửa malloc này
	//int* integral = malloc_int(integral_height * integral_width);
	int* integral = &(*matrix_temp_img_int);
	float* res = malloc_float(height * width);
	float sum;
	int i, j, k;
	int ii, jj;
	int r1, rr1, index, rwidth, r1width;

	memset(integral, 0, sizeof(int) * (integral_width));


	r1 = r + 1;
	rr1 = r + r + 1;
	rwidth = r + width;
	r1width = r + 1 + width;

	for (i = 1, j = 0; i <= r; i++) {
		for (k = 0; k < width; k++) {
			integral[(r1 - i) * integral_width + r1 + k] = arr[j * width + k];
		}
		j++;
		for (k = 0; k < width; k++) {
			integral[(r + height + i) * integral_width + r1 + k] = arr[(height - j) * width + k];
		}
		if (j == height) {
			j = 0;
		}
	}
	index = 0;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			integral[(r1 + i) * integral_width + r1 + j] = arr[index++];
		}
	}

	for (i = 1, j = 0; i <= r; i++) {
		for (k = -r; k < height + r; k++) {
			integral[(r1 + k) * integral_width + r1 - i] = integral[(r1 + k) * integral_width + r1 + j];
		}
		j++;
		for (k = -r; k < height + r; k++) {
			integral[(r1 + k) * integral_width + rwidth + i] = integral[(r1 + k) * integral_width + r1width - j];
		}
		if (j == width) {
			j = 0;
		}
	}
	index = integral_width;
	for (i = 1; i < height + rr1; ++i) {
		sum = 0;
		integral[index++] = 0;
		for (j = 1; j < width + rr1; ++j) {
			sum += integral[index];
			integral[index] = (int)(sum + integral[index - integral_width]);
			++index;
		}
	}

	k = window * window;


	for (i = 0; i < height; i++) {
		ii = rr1 + i;
		jj = rr1;
		for (j = 0; j < width; j++) {
			sum = (float)(integral[ii * integral_width + jj]//start[i + r][j + r] 
				- integral[i * integral_width + jj]//start[i - r - 1][j + r] 
				- integral[ii * integral_width + j]//start[i + r][j - r - 1]
				+ integral[i * integral_width + j]);//start[i - r - 1][j - r - 1];
			++jj;
			res[i * width + j] = sum / k;
		}
	}

	//#ifndef _COD_BAU_SE_
	//free_proc(integral);
	//#endif
	return res;
}


void region_avarage_bradley_threshold_1D(int* sum, unsigned char* bin, int len, int r1, int r2, float threshold) {

	// đã sửa chỗ này
	float* region_means = &(*matrix_temp_float1);
	float* global_means = &(*matrix_temp_float2);
	int i;

	uniform_filter_1D(sum, region_means, len, r1);
	uniform_filter_1D(sum, global_means, len, r2);

	threshold /= 100;
	for (i = 0; i < len; ++i) {
		if (region_means[i] > threshold * global_means[i]) {
			bin[i] = 255;
		}
		else {
			bin[i] = 0;
		}
	}
#ifndef _COD_BAU_SE_
	/*free_proc(region_means);
	free_proc(global_means);*/
#endif
}

void triple_faster_bradley_threshold(int height, int width, unsigned char* gray_img,
	unsigned char* img1, unsigned char* img2, unsigned char* img3, float threshold, int window_r, float ratio1, float ratio2) {

	float percentage1 = threshold / 100;
	float percentage2 = percentage1 * ratio1;
	float percentage3 = percentage1 * ratio2;
	float* means;
	int i;

#ifndef _COD_BAU_SE_
	gauss_filter_new(gray_img, height, width);
#endif
	means = uniform_filter_2D_new(gray_img, height, width, window_r);

	for (i = 0; i < height * width; i++) {
		img1[i] = img2[i] = img3[i] = 0;
	}
	for (i = 0; i < height * width; i++) {
		if (((float)gray_img[i]) < percentage3 * means[i]) {
			img3[i] = 255;
			if (((float)gray_img[i]) < percentage2 * means[i]) {
				img2[i] = 255;
				if (((float)gray_img[i]) < percentage1 * means[i]) {
					img1[i] = 255;
				}
			}
		}
	}
#ifndef _COD_BAU_SE_
	free_proc(means);
#endif
}

void project_on_x(unsigned char* image, int* sum, int top, int bottom, int width) {
	/*int x, y;
	for (x = 0; x < width; ++x) {
		sum[x] = 0;
		for (y = top; y <= bottom; ++y) {
			sum[x] += image[y * width + x];
		}
	}*/

	// Huong 変更 20220818
	int x, y;
	int sum_temp;
	for (x = 0; x < width; ++x) {
		sum_temp = 0;
		for (y = top; y <= bottom; ++y) {
			sum_temp += image[y * width + x];
		}
		sum[x] = sum_temp;
	}

}

void project_on_y(unsigned char* image, int* sum, int height, int width) {
	/*int x, y;
	for (y = 0; y < height; ++y) {
		sum[y] = 0;
		for (x = 0; x < width; ++x) {
			sum[y] += image[y * width + x];
		}
	}*/

	// Huong 変更 20220818
	int x, y;
	int sum_temp;
	for (y = 0; y < height; ++y) {
		sum_temp = 0;
		for (x = 0; x < width; ++x) {
			sum_temp += image[y * width + x];
		}
		sum[y] = sum_temp;
	}
}

int get_largest_binarized_CC1D(unsigned char* bin, int* histogram, int len, int num, int* left, int* right) {
	int i, j;
	int k = 0;
	int sum;
	int idx, m_idx;

	// đã sửa malloc này
	//int* seg_long = malloc_int(num);
	int* seg_long = &(*matrix_temp_img_int);

	for (i = 0; i < len; ++i) {
		if (bin[i] > 0) {
			j = i + 1;
			sum = histogram[i];
			while (j < len && bin[j] > 0) {
				sum += histogram[j];
				j++;
			}
			if (k == num) {
				m_idx = 0;
				for (idx = 1; idx < num; ++idx) {
					if (seg_long[idx] < seg_long[m_idx]) {
						m_idx = idx;
					}
				}
				if (seg_long[m_idx] < sum) {
					for (idx = m_idx; idx < num - 1; ++idx) {
						left[idx] = left[idx + 1];
						right[idx] = right[idx + 1];
						seg_long[idx] = seg_long[idx + 1];
					}
					left[num - 1] = i;
					right[num - 1] = j - 1;
					seg_long[num - 1] = sum;
				}
			}
			else {
				left[k] = i;
				right[k] = j - 1;
				seg_long[k] = sum;
				k++;
			}
			i = j;
		}
	}
	//#ifndef _COD_BAU_SE_
	//free_proc(seg_long);
	//#endif
	return k;
}

void remove_boundary_noise(unsigned char* bin, int len) {
	int i = 0;
	while (i < len && bin[i] > 0) {
		bin[i] = 0;
		++i;
	}
	i = len - 1;
	while (i > 0 && bin[i] > 0) {
		bin[i] = 0;
		i--;
	}
}

void filter_noise_segmentation(unsigned char* bin, int len, int threshold) {
	int i, j;
	for (i = 2; i < len - 2; ++i) {
		if (bin[i]) {
			j = i + 1;
			while (j < len && bin[j] > 0) {
				++j;
			}
			if (j - i <= threshold) {
				while (i < j) {
					bin[i] = 0;
					++i;
				}
			}
			else {
				i = j;
			}
		}
	}
}

void merge_H_segmentation(unsigned char* bin, int len, int threshold, int delta, int nof_ref) {
	int i, j, k;
	int prev_i_seg = -1;
	int prev_j_seg = -1;
	int max_one_seg = 0;
	if (nof_ref < 0) {
		nof_ref = len;
	}
	for (i = 2; i < len - 2 && nof_ref > 0; ++i) {
		if (bin[i]) {
			j = i + 1;
			while (j < len && bin[j] > 0) {
				j++;
			}
			max_one_seg = cod_max(max_one_seg, j - i);
			i = j;
			nof_ref--;
		}
	}
	max_one_seg += delta;
	for (i = 2; i < len - 2; ++i) {
		if (bin[i]) {
			j = i + 1;
			while (j < len && bin[j] > 0) {
				++j;
			}

			if (prev_i_seg >= 0) {
				if (i - prev_j_seg <= threshold && j - prev_i_seg <= max_one_seg) {
					for (k = prev_j_seg; k < i; ++k) {
						bin[k] = 255;
					}

					prev_j_seg = j;
				}
				else {
					prev_i_seg = i;
					prev_j_seg = j;
				}
			}
			else {
				prev_i_seg = i;
				prev_j_seg = j;
			}
			i = j;
		}
	}
}

void cut_bottom_top_offset(unsigned char* img, int height, int width, int* top, int* bottom, int region_r, int threshold) {
	// đã sửa malloc này
	//int* histogram_on_y = malloc_int(height);
	int* histogram_on_y = &(*matrix_temp_img_int);

	// đã sửa malloc này
	//unsigned char* binarized_his_on_y = (unsigned char*)malloc_char(height);
	unsigned char* binarized_his_on_y = &(*matrix_temp_height);

	project_on_y(img, histogram_on_y, height, width);
	region_avarage_bradley_threshold_1D(histogram_on_y, binarized_his_on_y, height, region_r, 101, (float)threshold); // 200dpi: r=4, threshold=40. 100dpi: r=2, threshold=60
	if (get_largest_binarized_CC1D(binarized_his_on_y, histogram_on_y, height, 1, top, bottom) == 0) {
		*top = 0;
		*bottom = height - 1;
	}
	*top = cod_max(0, *top - 1);
	//#ifndef _COD_BAU_SE_
	//free_proc(histogram_on_y);
	//free_proc(binarized_his_on_y);
	//#endif
}

unsigned char* copy_make_border(unsigned char* img, int height, int width) {
	int sz = cod_max(height, width);
	int gap = abs(height - width);
	int half = gap / 2;
	int y;
	unsigned char* ret;

	//ret = (unsigned char*)malloc_char(sz * sz);
	ret = &(*matrix_temp_img0);
	memset(ret, 0, sz * sz);
	if (height < width) {
		memcpy(ret + half * sz, img, height * width);
	}
	else {
		for (y = 0; y < height; ++y) {
			memcpy(ret + y * sz + half, img + y * width, width);
		}
	}
	return ret;
}

unsigned char* get_roi(unsigned char* img, unsigned char* roi, int width, int height,
	int roi_left, int roi_top, int roi_right, int roi_bottom) {
	// select region of interest
	int roi_width = roi_right - roi_left + 1;
	int roi_height = roi_bottom - roi_top + 1;

	//// đã sửa malloc này	// matrix_temp_img0 hiện đang không sử dụng // do mảng ROI dùng 2 lần đang bị trùng nên phải sửa code
	//unsigned char* roi = (unsigned char*)malloc_char(roi_height * roi_width);//(unsigned char*)malloc(roi_height * roi_width);
	////unsigned char* roi = matrix_temp_img0;

	int roi_idx = 0;
	int r, c;
	for (r = roi_top; r <= roi_bottom; ++r)
	{
		for (c = roi_left; c <= roi_right; ++c)
		{
			roi[roi_idx++] = img[r * width + c];
		}
	}
	return roi;
}


/**
	Gaussian Filter

	@param img: Data of image.
	@param height: Number row of image.
	@param width: Number col of image.
	@return: Mean data after filter.
*/
void gauss_filter(unsigned char* img, int height, int width) {
	static float gauss_matrix[5][5] = {
			{1.0 / 256, 1.0 / 256, 1.0 / 256			, 1.0 / 256, 1.0 / 256},
			{1.0 / 256, 1.0 / 256, 1.0 / 256			, 1.0 / 256, 1.0 / 256},
			{1.0 / 256, 1.0 / 256, 1.0 - 24.0 / 256	, 1.0 / 256, 1.0 / 256},
			{1.0 / 256, 1.0 / 256, 1.0 / 256			, 1.0 / 256, 1.0 / 256},
			{1.0 / 256, 1.0 / 256, 1.0 / 256			, 1.0 / 256, 1.0 / 256},
	};
	unsigned char* tmp = (unsigned char*)malloc_char(height * width);
	int i, j, x, y, xx, yy;
	float sum;
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			sum = 0;
			for (i = -2; i <= 2; i++) {
				for (j = -2; j <= 2; j++) {
					xx = x + i;
					yy = y + j;
					if (xx < 0) {
						continue;
					}
					if (xx >= width) {
						continue;
					}
					// Error condition in if statement
					if (yy < 0) {
						continue;
					}
					if (yy >= height) {
						continue;
					}
					sum += 1.0f * gauss_matrix[i + 2][j + 2] * img[yy * width + xx];
				}
			}
			tmp[y * width + x] = (unsigned char)sum;
		}
	}
	for (i = 0; i < height * width; i++) {
		img[i] = tmp[i];
	}
	//#ifndef _COD_BAU_SE_
	free_proc(tmp);
	//#endif
}

void gauss_filter_new(unsigned char* img, int height, int width) {
	// đã sửa malloc này
	//unsigned char* tmp = (unsigned char*)malloc_char(height * width);
	unsigned char* tmp = &(*matrix_temp_img1);
	// đã sửa malloc này
	//int *integral = (int *)malloc_int(height * width);
	int* integral = &(*matrix_temp_img_int);


	int i, j, x1, x2, y1, y2;

	int sumi = 0, cur_idx = width, pre_idx = 0;
	integral[0] = img[0];
	for (j = 1; j < width; ++j)
		integral[j] = integral[j - 1] + img[j];
	for (i = 1; i < height; ++i) {
		sumi = 0;
		for (j = 0; j < width; ++j) {
			sumi += img[cur_idx];
			integral[cur_idx++] = integral[pre_idx++] + sumi;
		}
	}
	y1 = 0;
	y2 = 1;
	for (j = 0; j < width; ++j) {
		if (j > 2) ++y1;
		if (j < width - 3) ++y2;
		x1 = 0; x2 = 1;
		for (i = 0; i < height; ++i) {
			cur_idx = i * width + j;

			if (i > 2) ++x1;
			if (i < height - 3) ++x2;

			sumi = integral[x2 * width + y2];
			if (x1) sumi -= integral[(x1 - 1) * width + y2];
			if (y1) sumi -= integral[x2 * width + y1 - 1];
			if (x1 && y1) sumi += integral[(x1 - 1) * width + y1 - 1];


			sumi += 231 * img[cur_idx];
			tmp[cur_idx] = (unsigned char)(sumi / 256);


		}
	}
	for (i = 0; i < height * width; i++) {
		img[i] = tmp[i];
	}
	//#ifndef _COD_BAU_SE_
		//free_proc(tmp);
		//free_proc(integral);
	//#endif
}


void bilinear_resize(unsigned char* in_img, unsigned char* out_img, int in_width, int in_height, int out_width, int out_height) {
	const float x_ratio = (float)(in_width - 1) / (out_width - 1);
	const float y_ratio = (float)(in_height - 1) / (out_height - 1);

	int* int_j = malloc_int(out_width);
	float* delta_j = malloc_float(out_width);
	float* p;

	int i, j;
	int i_out;
	int j_in;
	int int_i;
	float delta_i;

	///printf("%d %d %d %d", in_width, in_height, out_width, out_height);
	if ((out_width < 2) || (out_height < 2)) {
		return;
	}


	// Calculate intermediate points
	p = malloc_float((out_height + 2) * (in_width + 2)); // in_width max = 30. TODO: remove this magic number
	for (i_out = 0; i_out < out_height; ++i_out)
	{
		int_i = (int)(i_out * y_ratio);
		delta_i = i_out * y_ratio - int_i;
		for (j_in = 0; j_in < in_width; ++j_in)
		{
			p[i_out * in_width + j_in] = in_img[int_i * in_width + j_in] * (1 - delta_i) + \
				in_img[(int_i + 1) * in_width + j_in] * delta_i;
		}
	}
	// Calculate coordinates of out_img pixels in in_img coordinates

	//int_j = (int*) malloc(sizeof(int) * BOX_SIZE);//Malloc(int, BOX_SIZE);
	//delta_j = Malloc(float, BOX_SIZE);

	for (j = 0; j < out_width; ++j)
	{
		int_j[j] = (int)(j * x_ratio);
		delta_j[j] = j * x_ratio - int_j[j];
	}
	// Calculate out_img pixel's value
	for (i = 0; i < out_height; ++i)
	{
		for (j = 0; j < out_width; ++j)
		{
			if (j == 0) {
				out_img[i * out_width + j] = (unsigned char)(p[i * in_width] + 0.5);
			}
			else if (j == out_width - 1) {
				out_img[i * out_width + j] = (unsigned char)(p[(i + 1) * in_width - 1] + 0.5);
			}
			else {
				out_img[i * out_width + j] = (unsigned char)(delta_j[j] * p[i * in_width + int_j[j] + 1] + \
					(1 - delta_j[j]) * p[i * in_width + int_j[j]] + 0.5);
			}
		}
	}
	//#ifndef _COD_BAU_SE_
	free_proc(p);
	free_proc(int_j);
	free_proc(delta_j);
	//#endif
}

void bilinear_resize_new(unsigned char* in_img, unsigned char* out_img, int in_width, int in_height, int out_width, int out_height)
{
	const float x_ratio = (float)(in_width - 1) / (out_width - 1);
	const float y_ratio = (float)(in_height - 1) / (out_height - 1);

	float* p;

	int i, j;
	int i_out;
	int j_in;
	int int_i, int_j;
	float delta_i, delta_j;
	int inwidth, inwidth1, index, out_width1;


	if ((out_width < 2) || (out_height < 2)) {
		return;
	}

	p = malloc_float((out_height + 2) * (in_width + 2)); // in_width max = 30. TODO: remove this magic number
	for (i_out = 0; i_out < out_height; ++i_out)
	{
		int_i = (int)(i_out * y_ratio);
		delta_i = i_out * y_ratio - int_i;
		for (j_in = 0; j_in < in_width; ++j_in)
		{
			p[i_out * in_width + j_in] = in_img[int_i * in_width + j_in] * (1 - delta_i) + \
				in_img[(int_i + 1) * in_width + j_in] * delta_i;

		}
	}

	// Calculate out_img pixel's value
	out_width1 = out_width - 1;
	inwidth = 0;
	inwidth1 = 1;
	index = 0;
	for (i = 0; i < out_height; ++i)
	{
		out_img[index++] = (unsigned char)(p[i * in_width] + 0.5);

		for (j = 1; j < out_width1; ++j)
		{
			delta_j = j * x_ratio;
			int_j = (int)(delta_j);
			delta_j -= int_j;

			out_img[index++] = (unsigned char)(delta_j * p[inwidth1 + int_j] + \
				(1 - delta_j) * p[inwidth + int_j] + 0.5);
		}

		inwidth += in_width;
		inwidth1 += in_width;

		out_img[index++] = (unsigned char)(p[inwidth - 1] + 0.5);
	}


	free_proc(p);
}
