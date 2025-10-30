/*************************************************************************
 * svminference.c - simple inference code for linear svm in BAU machine
 * Copyright@JCM
 * AUTHOR:
 * NGUYEN Hong Chau (@aimesoft)
 * CREATION DATE:
 * 25 Feb 2019
 *
 * ADDITIONS, CHANGES
 *
 */
#define EXTERN extern
#include "svminference.h"
#include "cod_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "cod_malloc.h"
#include <math.h>

extern float* matrix_temp_float0;

#ifdef _OLD_SVM_INFERENCE_

// 0 all model
// 1 digit model
// // 2 alphabet model
//#include "printf_enable_disable.h"
//#include "jcm_getseries_v1_weights/svmweights.h"

#ifdef VALID_CNY_OCR
//#include "CNY_weights/cny_svmweights_0.h"
//#define MODEL_INPUT_SIZE 28

#endif
#ifdef VALID_EUR_OCR
#include "EUR_weights/svmweights.h"
//#define MODEL_INPUT_SIZE 14

#endif
#ifdef VALID_JPY_OCR
//#include "JPY_weights//jpy_0_genchar_0729_svmweights.h"
//#include "JPY_weights//jpy_1_genchar_0729_svmweights.h" // 実機で書き込み切れなさそうなのでテンプレート化してから使う
#endif

//add by furuta 200827
MODEL model_28[3];
//EXTERN MODEL model_14[3];
//EXTERN MODEL model_40[3];


/*
	static struct model model_28[3] = {
		{ 36, 784 + 1, global_all_model_28_ws_0		, global_all_model_28_labels_0		, 0 },
		{ 10, 784 + 1, global_digit_model_28_ws_0	, global_digit_model_28_labels_0	, 0 },
		{ 26, 784 + 1, global_alphabet_model_28_ws_0, global_alphabet_model_28_labels_0	, 0 } };
		*/
		/*static struct model model_14[3] = { { 36, 196 + 1, global_all_model_14_ws, global_all_model_14_labels, 0 },
		{ 10, 196 + 1, global_digit_model_14_ws, global_digit_model_14_labels, 0 },
		{ 26, 196 + 1,  global_alphabet_model_14_ws, global_alphabet_model_14_labels, 0 } };
	*/
	//static struct model model_40[3] = { { 36, 1600 + 1, global_all_model_40_ws, global_all_model_40_labels, 0 },
	//{ 10, 1600 + 1, global_digit_model_40_ws, global_digit_model_40_labels, 0 },
	//{ 26, 1600 + 1,  global_alphabet_model_40_ws, global_alphabet_model_40_labels, 0 } };
#ifdef VALID_CNY_OCR
char CNY_model = 'A';
void get_model(char model)
{
	CNY_model = model;
}
#endif


char svm_predict(char character_type, unsigned char* x, int output_size)
{
	MODEL* model_ = &model_28[character_type];
	//const struct model *model_ = (output_size == 28) ? &model_28[character_type] : (output_size == 14 ? &model_14[character_type] : &model_40[character_type]);
	//float *dec_values = (float*)malloc(sizeof(float) * (model_->nof_class));

//		float *dec_values = malloc_float(model_->nof_class); //JCM用Mallocに変更　20200713
	float* dec_values = &(*matrix_temp_float0); //Huong変更　20220720
	char label = svm_predict_values(model_, x, dec_values);

	//		free_proc(dec_values);	//解放がなかったので追加　20200713
	return label;
}

void svm_predict_new(ir* result, char character_type, const unsigned char* x, int output_size)	// Huong add 20210820
{
	MODEL* model_ = &model_28[character_type];
	float* dec_values = malloc_float(model_->nof_class); //JCM用Mallocに変更　20200713
	svm_predict_values_new(result, model_, x, dec_values);
}

char svm_predict_values(MODEL* model_, const unsigned char* x, float* dec_values)
{
	//int nof_class = model_->nof_class;
	//int nof_feature = model_->nof_feature;
	//float *w = model_->w;
	//int i = 0, j = 0;
	//int dec_max_idx = 0;
	//float xx = 0;
	//for (i = 0; i < nof_class; i++) {
	//	dec_values[i] = 0;
	//}
	//for (i = 0; i < nof_class; i++) {
	//	for (j = 0; j < nof_feature - 1; j++) {
	//		xx = x[j];
	//		dec_values[i] += w[j*nof_class + i] * xx;

	//	}
	//	dec_values[i] += w[(nof_feature - 1)*nof_class + i] * 1; // add bias
	//}

	//for (i = 1; i < nof_class; i++)
	//{
	//	if (dec_values[i] > dec_values[dec_max_idx]) {

	//		dec_max_idx = i;
	//	}
	//}
	////float test = exp(dec_values[dec_max_idx]);

	//return model_->label[dec_max_idx];

	// Huong 変更20221808
	int nof_class = model_->nof_class;
	int nof_feature = model_->nof_feature;
	float* w = model_->w;
	int i = 0;
	int j = 0;
	int dec_max_idx = 0;
	//float xx = 0;
	float dec_temp = 0;
	/*for (i = 0; i < nof_class; i++) {
		dec_values[i] = 0;
	}*/
	for (i = 0; i < nof_class; i++) {
		dec_temp = 0;
		for (j = 0; j < nof_feature - 1; j++) {
			//xx = x[j];
			dec_temp += w[j * nof_class + i] * x[j];

		}
		dec_temp += w[(nof_feature - 1) * nof_class + i] * 1; // add bias
		dec_values[i] = dec_temp;
	}

	for (i = 1; i < nof_class; i++)
	{
		if (dec_values[i] > dec_values[dec_max_idx]) {

			dec_max_idx = i;
		}
	}
	//float test = exp(dec_values[dec_max_idx]);
	return model_->label[dec_max_idx];
}

void svm_predict_values_new(ir* result, MODEL* model_, const unsigned char* x, float* dec_values)	// Huong add 20210820
{
	int nof_class = model_->nof_class;
	int nof_feature = model_->nof_feature;
	float* w = model_->w;
	int i, j;
	int dec_max_idx = 0;
	float xx;
	int cnt = 0;
	float test_dec;
	float test_exp_dec;
	float sum_prob;

	for (i = 0; i < nof_class; i++) {
		dec_values[i] = 0;
	}

	for (i = 0; i < nof_class; i++) {
		for (j = 0; j < nof_feature - 1; j++) {
			xx = x[j];
			dec_values[i] += w[j * nof_class + i] * xx;

		}
		dec_values[i] += w[(nof_feature - 1) * nof_class + i] * 1; // add bias
	}

	test_dec = dec_values[0];
	test_exp_dec = exp(dec_values[0]);

	sum_prob = exp(dec_values[0]);
	for (i = 1; i < nof_class; i++)
	{
		test_dec = dec_values[i];
		test_exp_dec = exp(dec_values[i]);
		sum_prob += exp(dec_values[i]);
		if (dec_values[i] > dec_values[dec_max_idx]) {
			dec_max_idx = i;
		}
	}


	test_dec = exp(dec_values[dec_max_idx]);
	result->label = model_->label[dec_max_idx];
	result->prob = exp(dec_values[dec_max_idx]) / sum_prob;
}

int get_cod_weight_size(void) {
	return MODEL_INPUT_SIZE;
}

//add by furuta 200827
//構造体model_にテンプレートデータから読み込んだ
//svmのパラメタをセットします。
void set_svm_wights(void* ag_para)
{
	COD_Parameters* para = (COD_Parameters*)ag_para;

#define FRAMESIZE_28x28 (785)
#define FRAMESIZE_14x14 (197)
#define FRAMESIZE_40x40 (1601)

	/* original
		static struct model model_28[3] = {
	{ 36, 784 + 1, global_all_model_28_ws_0		, global_all_model_28_labels_0		, 0 },
	{ 10, 784 + 1, global_digit_model_28_ws_0	, global_digit_model_28_labels_0	, 0 },
	{ 26, 784 + 1, global_alphabet_model_28_ws_0, global_alphabet_model_28_labels_0	, 0 } };
	1:文字の種類数
	2：28*28+1
	3：重みのポインタ
	4：ラベルのポインタ
	5：バイアス
	static struct model model_14[3]
	static struct model model_40[3]

	*/

	switch (para->cutout_size)
	{
	case 28:
		model_28[0].nof_class = para->composite_labels_num;
		model_28[0].nof_feature = FRAMESIZE_28x28;
		model_28[0].w = para->p_composite_weight;
		model_28[0].label = para->p_composite_labels;
		model_28[0].bias = 0;

		model_28[1].nof_class = para->numbers_labels_num;
		model_28[1].nof_feature = FRAMESIZE_28x28;
		model_28[1].w = para->p_numbers_weight;
		model_28[1].label = para->p_numbers_labels;
		model_28[1].bias = 0;

		model_28[2].nof_class = para->letter_labels_num;
		model_28[2].nof_feature = FRAMESIZE_28x28;
		model_28[2].w = para->p_letter_weight;
		model_28[2].label = para->p_letter_labels;
		model_28[2].bias = 0;
		break;

	case 14:
		model_28[0].nof_class = para->composite_labels_num;
		model_28[0].nof_feature = FRAMESIZE_14x14;
		model_28[0].w = para->p_composite_weight;
		model_28[0].label = para->p_composite_labels;
		model_28[0].bias = 0;

		model_28[1].nof_class = para->numbers_labels_num;
		model_28[1].nof_feature = FRAMESIZE_14x14;
		model_28[1].w = para->p_numbers_weight;
		model_28[1].label = para->p_numbers_labels;
		model_28[1].bias = 0;

		model_28[2].nof_class = para->letter_labels_num;
		model_28[2].nof_feature = FRAMESIZE_14x14;
		model_28[2].w = para->p_letter_weight;
		model_28[2].label = para->p_letter_labels;
		model_28[2].bias = 0;
		break;

	case 40:
		model_28[0].nof_class = para->composite_labels_num;
		model_28[0].nof_feature = FRAMESIZE_40x40;
		model_28[0].w = para->p_composite_weight;
		model_28[0].label = para->p_composite_labels;
		model_28[0].bias = 0;

		model_28[1].nof_class = para->numbers_labels_num;
		model_28[1].nof_feature = FRAMESIZE_40x40;
		model_28[1].w = para->p_numbers_weight;
		model_28[1].label = para->p_numbers_labels;
		model_28[1].bias = 0;

		model_28[2].nof_class = para->letter_labels_num;
		model_28[2].nof_feature = FRAMESIZE_40x40;
		model_28[2].w = para->p_letter_weight;
		model_28[2].label = para->p_letter_labels;
		model_28[2].bias = 0;
		break;
	}

}

#else

#include "svmweights.h"

// 0 all model
// 1 digit model
// // 2 alphabet model

static const struct model _model[3] = { {36, MODEL_INPUT_SIZE * MODEL_INPUT_SIZE + 1, global_all_model_ws, global_all_model_labels, 0},
										{10, MODEL_INPUT_SIZE * MODEL_INPUT_SIZE + 1, global_digit_model_ws, global_digit_model_labels, 0},
										{26, MODEL_INPUT_SIZE * MODEL_INPUT_SIZE + 1,  global_alphabet_model_ws, global_alphabet_model_labels, 0} };

char svm_predict(char character_type, unsigned char* x, int size)
{
	const struct model* model_ = &_model[character_type];
	float* dec_values = malloc_float(model_->nof_class);
	char label;
	label = svm_predict_values(model_, x, dec_values);
#ifndef _COD_BAU_SE_
	free(dec_values);
#endif
	return label;
}

char svm_predict_values(const struct model* model_, const unsigned char* x, float* dec_values)
{
	int nof_class = model_->nof_class;
	int nof_feature = model_->nof_feature;
	float* w = model_->w;
	int i, j;
	int dec_max_idx = 0;
	float xx;

	for (i = 0; i < nof_class; i++) {
		dec_values[i] = 0;
	}
	for (i = 0; i < nof_class; i++) {
		for (j = 0; j < nof_feature - 1; j++) {
			xx = x[j];
			dec_values[i] += w[j * nof_class + i] * xx;
		}
		dec_values[i] += w[(nof_feature - 1) * nof_class + i] * 1; // add bias
	}

	for (i = 1; i < nof_class; i++)
	{
		if (dec_values[i] > dec_values[dec_max_idx])
			dec_max_idx = i;
	}
	return model_->label[dec_max_idx];
}

int get_cod_weight_size(void) {
	return MODEL_INPUT_SIZE;
}
#endif
