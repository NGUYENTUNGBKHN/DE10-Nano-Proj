

#define EXT
#include "../common/global.h"

/****************************************************************/
/*
* @brief		活性化関数（relu）
*               f(x) = 0, x <=0
*                      x, x > 0
* 
* @param[in]	input 入力
*
* @param[out]	output  計算結果
* @return
*/
/****************************************************************/
void op_relu(Tensor *input, Tensor *output) {
    size_t i;

    float *x = input->data;
    float *y = output->data;

    for (i = 0; i < (u32)input->len; ++i)
        y[i] = (x[i] > 0) ? x[i] : 0;
}

void op_sigmoid(Tensor *input, Tensor *output) {
    float *x = input->data;
    float *y = output->data;

    int len = input->len;
    size_t i;

    for (i = 0; i < (u32)len; ++i)
        y[i] = 1 / (exp(-x[i]) + 1);
}

void op_identity(Tensor *input, Tensor *output) {
    size_t i;

    float *x = input->data;
    float *y = output->data;

    for (i = 0; i < input->len; ++i)
        y[i] = x[i];
}

void op_tanh(Tensor *input, Tensor *output) {
    float *x = input->data;
    float *y = output->data;

    int len = input->len;
    size_t i;

    for (i = 0; i < len; ++i)
        y[i] = tanhf(x[i]);
}

/****************************************************************/
/*
* @brief		NNの積和演算
* @param[in]	input   入力層
*               weights 重み
* 
* @param[out]	output  出力層
* @return
*/
/****************************************************************/
void matMul(Tensor *input, Tensor *output, Tensor *weights) {
    float *x = input->data;
    float *y = output->data;
    float *w = weights->data;

    size_t i = 0;
    size_t j = 0;
    // printf("output len = %d\n", output->len);

    for (i = 0; i < output->len; ++i) {
        float tmp = 0;
        w = weights->data + i;
        for (j = 0; j < input->len; ++j) {
            tmp += x[j] * w[0];
            w += output->len;
        }

        y[i] = tmp;
        // printf("out %d = %f\n", i, y[i]);
    }
}

/****************************************************************/
/*
* @brief		活性化関数前の切片の計算
* @param[in]	a  中間層
*               b  切片
*
* @param[out]	c  計算結果
* @return
*/
/****************************************************************/
void tensorAdd(Tensor *a, Tensor *b, Tensor *c) {
    float *x = a->data;
    float *y = b->data;
    float *z = c->data;

    size_t len = a->len;
    size_t i;
    
    for (i = 0; i < len; ++i) {
        z[i] = x[i] + y[i];
    }

}

void get_activation_func(PerceptronLayer *l) {
    //printf("Not implemented yet\n");
}


/****************************************************************/
/*
* @brief		MLP処理
* @param[in]	clf     MLPの設定
*               input   入力データ
*
* @param[out]	clf->result　予測濃度値
* @return
*/
/****************************************************************/
s8 _forward_pass_fast(MLP *clf, Tensor *input/*, ST_BS* pbs*/) {

    PerceptronLayer *cur_layer = clf->layers;
    Tensor *cur_tensor = input;
    u8 i = 1;

    while (cur_layer) {
        matMul(cur_tensor, cur_layer->tensor, cur_layer->w);
        //pbs->mid_res_soiling.plane_distance[i] = (u32)(calc_sumchk(cur_layer->tensor->data, cur_layer->tensor->len));
        //i++;
        tensorAdd(cur_layer->tensor, cur_layer->b, cur_layer->tensor);
        //pbs->mid_res_soiling.plane_distance[i] = (u32)(calc_sumchk(cur_layer->tensor->data, cur_layer->tensor->len));
        //i++;
        cur_tensor = cur_layer->tensor;
        if (cur_layer->activation_func == NULL)
        {
            return -1;
        }
        cur_layer->activation_func(cur_tensor, cur_tensor);
        
        cur_layer = cur_layer->next_layer;
        
    }
    memcpy(clf->result, cur_tensor, sizeof(Tensor));

    return 0;
}