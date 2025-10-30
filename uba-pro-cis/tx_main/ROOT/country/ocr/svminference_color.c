
#include "cod_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "svminference.h"
#include "cod_malloc.h"
#include "cod_utilities.h"
#include "cod_getseries.h"


#ifdef VALID_CNY_OCR
#endif
#ifdef VALID_EUR_OCR
#endif

#include "JPY_weights/jpy_color_SVM_all.h"


#define NOF_REF -1

static const MODEL _model[3] = { {3, COLOR_MODEL_INPUT_SIZE*COLOR_MODEL_INPUT_SIZE*3 + 1, global_1000_model_ws, global_1000_model_labels, 0},
{2, COLOR_MODEL_INPUT_SIZE*COLOR_MODEL_INPUT_SIZE*3 + 1, global_5000_model_ws, global_5000_model_labels, 0},
{2, COLOR_MODEL_INPUT_SIZE*COLOR_MODEL_INPUT_SIZE*3 + 1, global_10000_model_ws, global_10000_model_labels, 0} };


char svm_predict_color(float *x, int model_type)
{
	const MODEL *model_ = &_model[model_type];
	float *dec_values = malloc_float(model_->nof_class);
	char label;
	label = svm_predict_values_color(model_, x, dec_values);
#ifndef _COD_BAU_SE_
	free_proc(dec_values);
#endif
	return label;
}

char svm_predict_values_color(const MODEL *model_, const float *x, float *dec_values)
{
	int nof_class = model_->nof_class;
	int nof_feature = model_->nof_feature;
	float *w = model_->w;
	int i = 0, j = 0;
	int dec_max_idx = 0;
	float xx = 0;

	for (i = 0; i < nof_class; i++) {
		dec_values[i] = 0;
	}
	for (i = 0; i < nof_class; i++) {
		for (j = 0; j < nof_feature - 1; j++) {
			xx = x[j];
			dec_values[i] += w[j*nof_class + i] * xx;
		}
		dec_values[i] += w[(nof_feature - 1)*nof_class + i] * 1; // add bias
	}

	for (i = 1; i < nof_class; i++)
	{
		if (dec_values[i] > dec_values[dec_max_idx])
			dec_max_idx = i;
	}

	return model_->label[dec_max_idx];
}
#if 0
void cod_getcolor(const COD_Parameters *para, char* predicted_text, unsigned char* r_data, unsigned char* g_data, unsigned char* b_data)
{


	int region_r_on_offset = (int)(1.5 * (para->dpi / 100));
	int top, bottom;
	int *left = malloc_int(para->series_length);
	int *right = malloc_int(para->series_length);
	unsigned char* img = (unsigned char*)malloc_char(para->height * para->width);
	unsigned char *roi_1, *roi_2, *roi_3, *roi_r, *roi_g, *roi_b, *cut_roi_g;
	float roi_svm[MODEL_INPUT_SIZE* COLOR_MODEL_INPUT_SIZE * 3];
	int i;
	int roi_width;
	int roi_height;

	int series_length = para->series_length;

	int num_cc = detect_series_cc_JPY(para->height, para->width, g_data, img, &series_length,
		para->switch_binarization_value, para->binarization_threshold, para->threshold_on_y, para->threshold_on_x,
		para->threshold_on_offset, para->dpi, &top, &bottom, left, right, NOF_REF ,para->jpy_flg);

	int t, b;
	char color_predict;
	int ii = 0;

	for (i = 0; i < num_cc; i++)
	{

		int roi_width_g = right[i] - left[i] + 2;
		int roi_height_g = bottom - top + 1;
		cut_roi_g = get_roi(img, para->width, para->height, left[i] - 1, top, right[i], bottom);
		cut_bottom_top_offset(cut_roi_g, roi_height_g, roi_width_g, &t, &b, region_r_on_offset, para->threshold_on_offset);

		roi_width = right[i] - left[i];
		roi_height = b - t + 1;

		roi_g = get_roi(g_data, para->width, para->height, left[i], top + t, right[i] - 1, top + b);
		roi_b = get_roi(b_data, para->width, para->height, left[i], top + t, right[i] - 1, top + b);
		roi_r = get_roi(r_data, para->width, para->height, left[i], top + t, right[i] - 1, top + b);

		roi_1 = (unsigned char*)malloc_char(COLOR_MODEL_INPUT_SIZE* COLOR_MODEL_INPUT_SIZE);
		roi_2 = (unsigned char*)malloc_char(COLOR_MODEL_INPUT_SIZE* COLOR_MODEL_INPUT_SIZE);
		roi_3 = (unsigned char*)malloc_char(COLOR_MODEL_INPUT_SIZE* COLOR_MODEL_INPUT_SIZE);

		bilinear_resize(roi_b, roi_1, roi_width, roi_height, COLOR_MODEL_INPUT_SIZE, COLOR_MODEL_INPUT_SIZE);
		bilinear_resize(roi_g, roi_2, roi_width, roi_height, COLOR_MODEL_INPUT_SIZE, COLOR_MODEL_INPUT_SIZE);
		bilinear_resize(roi_r, roi_3, roi_width, roi_height, COLOR_MODEL_INPUT_SIZE, COLOR_MODEL_INPUT_SIZE);

		for (ii = 0; ii < COLOR_MODEL_INPUT_SIZE* COLOR_MODEL_INPUT_SIZE * 3; ii++)
		{
			if ((ii + 3) % 3 == 0)
			{
				roi_svm[ii] = (float)roi_1[ii / 3] / 255;
			}
			else if ((ii + 3) % 3 == 1)
			{
				roi_svm[ii] = (float)roi_2[ii / 3] / 255;
			}
			else
			{
				roi_svm[ii] = (float)roi_3[ii / 3] / 255;
			}
		}

		color_predict = svm_predict_color(roi_svm, para->model_color);
		predicted_text[i] = color_predict;

		free_proc(roi_r);
		free_proc(roi_g);
		free_proc(roi_b);
		free_proc(cut_roi_g);
		//free_proc(roi_svm);	//err

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
#endif
