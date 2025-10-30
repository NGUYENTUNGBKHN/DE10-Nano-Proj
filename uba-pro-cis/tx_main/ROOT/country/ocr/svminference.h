/*************************************************************************
 * svminference.h - simple inference code for linear svm in BAU machine
 * Copyright@JCM
 * AUTHOR:
 * NGUYEN Hong Chau (@aimesoft)
 * CREATION DATE:
 * 25 Feb 2019
 *
 * ADDITIONS, CHANGES
 *
 */


#ifndef _SVMINFERENCE_H
#define _SVMINFERENCE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
	int nof_class;		/* number of classes */
	int nof_feature;
	float *w;
	char *label;		/* label of each class */
	float bias;
} MODEL;

struct inference_result {
	char label;
	float prob;
};

typedef struct inference_result ir;

char svm_predict_values(MODEL *model_, const unsigned char *x, float *dec_values);
char svm_predict(char character_type, unsigned char *x, int size);
int get_cod_weight_size(void);
void get_model(char model);
void set_svm_wights(void *para);	//add by furuta 200827

// êFîªíË
char svm_predict_values_color(const MODEL *model_, const float *x, float *dec_values);
char svm_predict_color(float *x, int model_type);

// new SVM
void svm_predict_new(ir *result, char character_type, const unsigned char *x, int output_size);
void svm_predict_values_new(ir* result, MODEL* model_, const unsigned char* x, float* dec_values);
#ifdef __cplusplus
}
#endif

#endif /* _SVMINFERENCE_H */
