#include <math.h>


#define EXT
#include "../common/global.h"

#include "all_operator.h"

//#include "../../ocr/cod_malloc.h"
//void op_relu(NN_Tensor *input, NN_Tensor *output) {
//    size_t i;
//
//    scalar *x = input->data;
//    scalar *y = output->data;
//
//    for (i = 0; i < input->len; ++i)
//        y[i] = (x[i] > 0) ? x[i] : 0;
//}
//
//void op_sigmoid(NN_Tensor *input, NN_Tensor *output) {
//    scalar *x = input->data;
//    scalar *y = output->data;
//    
//    u16 len = input->len;
//    size_t i;
//
//    for (i = 0; i < len; ++i)
//        y[i] = 1 / (exp(-x[i]) + 1);
//}
//
//void op_identity(NN_Tensor *input, NN_Tensor *output) {
//    size_t i;
//
//    scalar *x = input->data;
//    scalar *y = output->data;
//
//    for (i = 0; i < input->len; ++i)
//        y[i] = x[i];
//}
//
//void op_tanh(NN_Tensor *input, NN_Tensor *output) {
//    scalar *x = input->data;
//    scalar *y = output->data;
//
//	u16 len = input->len;
//    size_t i;
//
//    for (i = 0; i < len; ++i)
//        y[i] = tanhf(x[i]);
//}

void op_softmax(NN_Tensor *input, NN_Tensor *output) {
    float *x = input->data;
    float *y = output->data;

	u16 len = input->len;
    size_t i;
    float sum = 0;
    for (i = 0; i < len; ++i)
    {
        y[i] = exp(x[i]);
        sum += y[i];
    }

    for (i = 0; i < len; ++i)
        y[i] = y[i] / sum;
}






