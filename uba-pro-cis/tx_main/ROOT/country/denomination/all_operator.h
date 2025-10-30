#ifndef _ALL_OPERATOR_H_
#define _ALL_OPERATOR_H_

#ifdef __cplusplus
extern "C"
{
#endif/*__cplusplus*/

//#include "MLPModule.h"

//void op_relu(NN_Tensor *input, NN_Tensor *output);
//void op_sigmoid(NN_Tensor *input, NN_Tensor *output);
//void op_identity(NN_Tensor *input, NN_Tensor *output);
//void op_tanh(NN_Tensor *input, NN_Tensor *output);
void op_softmax(NN_Tensor *input, NN_Tensor *output);



#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif // _ALL_OPERATOR_H_
