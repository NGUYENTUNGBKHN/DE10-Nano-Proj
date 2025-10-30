#include <string.h>

#define EXT
#include "../common/global.h"

#include "MLPClassifier.h"

//void matMul(NN_Tensor *input, NN_Tensor *output, NN_Tensor *weights) {
//    scalar *x = input->data;
//    scalar *y = output->data;
//    scalar *w = weights->data;
//
//    size_t i = 0;
//    size_t j = 0;
//    // printf("output len = %d\n", output->len);
//
//    for (i = 0; i < output->len; ++i) {
//        scalar tmp = 0;
//        w = weights->data + i;
//        for (j = 0; j < input->len; ++j) {
//            tmp += x[j] * w[0];
//            w += output->len;
//        }
//
//        y[i] = tmp;
//        // printf("out %d = %f\n", i, y[i]);
//    }
//}
//
//void tensorAdd(NN_Tensor *a, NN_Tensor *b, NN_Tensor *c) {
//    scalar *x = a->data;
//    scalar *y = b->data;
//    scalar *z = c->data;
//
//    size_t len = a->len;
//    size_t i;
//    
//    for (i = 0; i < len; ++i) {
//        z[i] = x[i] + y[i];
//    }
//
//}
//
//void get_activation_func(PerceptronLayer *l) {
//    printf("Not implemented yet\n");
//}

void get_max_softmax(NN_Tensor *x) {
	size_t i;
	u16 id = 0;
	for (i = 1; i < x->len; ++i)
	if (x->data[id] < x->data[i]) 
		id = i;
	x->data[0] = x->data[id];
	x->data[1] = id;
}

void get_top3_softmax(NN_Tensor *x) {
	u16 i;
	u16 fidx = 0;
	u16 sidx = 0;
	u16 tidx = 0;
	float fmax, smax, tmax;

	if (x->data[0] > x->data[1]) sidx = 1;
	else fidx = 1;

	if (x->data[2] > x->data[fidx]) 
	{
		tidx = sidx;
		sidx = fidx;
		fidx = 2;
	}
	else if (x->data[2] > x->data[sidx])
	{
		tidx = sidx;
		sidx = 2;
	}
	else tidx = 2;

	for (i = 3; i < x->len; i++)
	{
		if (x->data[i] > x->data[fidx]) 
		{
			tidx = sidx;
			sidx = fidx;
			fidx = i;
		}
		else if (x->data[i] > x->data[sidx])
		{
			tidx = sidx;
			sidx = i;
		}
		else if (x->data[i] > x->data[tidx])
		{
			tidx = i;
		}
	}

	fmax = x->data[fidx];
	smax = x->data[sidx];
	tmax = x->data[tidx];

	x->data[0] = fmax;
	x->data[1] = fidx;

	x->data[2] = smax;
	x->data[3] = sidx;
	
	x->data[4] = tmax;
	x->data[5] = tidx;
}

void inference_MLP(MLPClassifier *clf, NN_Tensor *input) {
#if 0
	NN_PerceptronLayer*cur_layer = clf->layers;
	NN_Tensor *cur_tensor = input;

	while (cur_layer) {
		matMul(cur_tensor, cur_layer->tensor, cur_layer->w);
		tensorAdd(cur_layer->tensor, cur_layer->b, cur_layer->tensor);
		cur_tensor = cur_layer->tensor;
		if (cur_layer->activation_func == NULL)
			get_activation_func(cur_layer);
		cur_layer->activation_func(cur_tensor, cur_tensor);
		
		cur_layer = cur_layer->next_layer;
	}
	memcpy(clf->result, cur_tensor, sizeof(NN_Tensor));

	if (clf->result->len > 2)
		get_top3_softmax(clf->result);
#endif
}
