#ifndef _SOIL_MLP_H_
#define _SOIL_MLP_H_

#include <string.h>

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    int len;
    float *data;
}Tensor;


struct PerceptronLayer {

    Tensor *w;
    Tensor *b;
    Tensor *tensor;
    struct PerceptronLayer *next_layer;
    int act_func_type;
    void (*activation_func)(Tensor *input, Tensor *output);
    int len_feature;
};
typedef struct PerceptronLayer PerceptronLayer;


struct OperationResolver {

    void (*relu)(Tensor *input, Tensor *output);
    void (*sigmoid)(Tensor *input, Tensor *output);
    void (*identity)(Tensor *input, Tensor *output);
    void (*tanh)(Tensor *input, Tensor *output);
};
typedef struct OperationResolver OperationResolver;


struct MLP {
    PerceptronLayer *layers;
    Tensor *result;
};
typedef struct MLP MLP;

void op_relu(Tensor *input, Tensor *output);
void op_sigmoid(Tensor *input, Tensor *output);
void op_identity(Tensor *input, Tensor *output);
void op_tanh(Tensor *input, Tensor *output);

s8 _forward_pass_fast(MLP *clf, Tensor *input/*, ST_BS *pbs*/);

#ifdef __cplusplus
}
#endif

#endif


