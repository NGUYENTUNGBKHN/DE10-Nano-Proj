#include <stdio.h>

#define EXT
#include "../common/global.h"

#include "all_operator.h"
#include "data_struct.h"
#include "hog.h"
#include "mblbp.h"
#include "outlier_checker.h"


#define NUM_ITER 1
#define NUM_FEATS 560 // 

u16 integrate_test(u8 buf_num, u8* image_data, u16 height, u16 width, new_nn_config* nn_config, ST_DATA_EXTRACTION* data_extraction, u8* start_block, ST_NEW_NN_RESULT* nn_res)
{
	u16 err = 0;
#if 0
	u16 label;
	u16 i;
	float* aime_feat;
	u16 num_feat = 0;
	//int num_feat_bvtools = 0;
	float confidence = 0;
	u8 is_outlier;

	float X[NUM_FEATS];//num_feat_bvtools + num_feat


	for (i = 0; i < NUM_ITER; ++i)
	{
		aime_feat = computeHOG(image_data, width, height, &num_feat,
			nn_config->orientations,
			nn_config->pixels_per_cell,
			nn_config->cells_per_block,
			96, 96);

	}

	for (i = 0; i < data_extraction->count; ++i)
		X[i] = 1.0f * data_extraction->data_extra_out_put[i] / 255.0f;

	for (i = 0; i < num_feat; ++i)
		X[i + data_extraction->count] = aime_feat[i];

	NN_Tensor input = {
		.len = data_extraction->count + num_feat,
		.data = (float*)X
	};

	MLPClassifier nn;

	// update data for MLPClassifier;
	float* output_l_1;
	output_l_1 = (float*)malloc_float(nn_config->n_output * sizeof(float));
	for (int i = 0; i < nn_config->n_output; i++)
	{
		output_l_1[i] = 0.0;
	}
	NN_Tensor w_tensor_1 = {
		.len = nn_config->n_output * nn_config->n_hidden,
		.data = (float*)(start_block + nn_config->Offset_Output)
	};
	NN_Tensor b_tensor_1 = {
		.len = nn_config->n_output,
		.data = (float*)(start_block + nn_config->Offset_Bias_Output),
	};
	NN_Tensor output_tensor_1 = {
		.len = nn_config->n_output,
		.data = output_l_1,
	};
	void* activation_func_l1;
	//0 ident,1 relu ,2 softmax,3 tanh
	switch (nn_config->activation_output)
	{
	case 0:
		activation_func_l1 = op_identity;
		break;
	case 1:
		activation_func_l1 = op_relu;
		break;
	case 2:
		activation_func_l1 = op_softmax;
		break;
	case 3:
		activation_func_l1 = op_tanh;
		break;
	default:
		activation_func_l1 = op_softmax;
		break;
	}
	NN_PerceptronLayer l_1 = {
	.w = &w_tensor_1,
	.b = &b_tensor_1,
	.tensor = &output_tensor_1,
	.next_layer = NULL,
	.act_func_type = -1,
	.activation_func = activation_func_l1
	};
	float* output_l_0;
	output_l_0 = (float*)malloc_float(nn_config->n_hidden * sizeof(float));
	for (int i = 0; i < nn_config->n_output; i++)
	{
		output_l_0[i] = 0.0;
	}
	NN_Tensor w_tensor_0 = {
		.len = nn_config->n_input * nn_config->n_hidden,
		.data = (float*)(start_block + nn_config->Offset_Hidden),
	};
	NN_Tensor b_tensor_0 = {
		.len = nn_config->n_hidden,
		.data = (float*)(start_block + nn_config->Offset_Bias_Hidden),
	};
	NN_Tensor output_tensor_0 = {
		.len = nn_config->n_hidden,
		.data = output_l_0,
	};

	void* activation_func_l0;
	switch (nn_config->activation_hidden)
	{
	case 0:
		activation_func_l0 = op_identity;
		break;
	case 1:
		activation_func_l0 = op_relu;
		break;
	case 2:
		activation_func_l0 = op_softmax;
		break;
	case 3:
		activation_func_l0 = op_tanh;
		break;
	default:
		activation_func_l0 = op_softmax;
		break;
	}
	NN_PerceptronLayer l_0 = {
	.w = &w_tensor_0,
	.b = &b_tensor_0,
	.tensor = &output_tensor_0,
	.next_layer = &l_1,
	.act_func_type = -1,
	.activation_func = activation_func_l0

	};

	float* result = (float*)malloc_float(nn_config->n_output * sizeof(float));
	for (int i = 0; i < nn_config->n_output; i++)
	{
		result[i] = 0.0;
	}
	NN_Tensor result_tensor = {
		.len = nn_config->n_output,
		.data = result,
	};
	nn.layers = &l_0;
	nn.result = &result_tensor;

	//MLPClassifier *model =& mlp_clf;
	//inference_MLP(model, &input);
	for (i = 0; i < NUM_ITER; ++i)
	{
		inference_MLP(&nn, &input);
	}

	nn_res->output_max_val = 0;
	nn_res->output_max_node = 0;
	nn_res->output_2nd_val = 0;
	nn_res->output_2nd_node = 0;

	err = check_is_outlier(nn.result);
	//confidence = nn.result->data[0] * 10000;

	if (err == 0)	//エラーが無ければ金種を記録する。
	{
		if (nn.result->len > 1)
		{
			DenomiInfoLabel* Label = (DenomiInfoLabel*)(start_block + nn_config->Offset_label);
			label = (u16)nn.result->data[1];
			DenomiInfoLabel actual_label = Label[label];

			u32 way_code = 0;
			switch (actual_label.insert_way)
			{
			case 0:
				way_code = 0x1000;
				break;
			case 1:
				way_code = 0x2000;
				break;
			case 2:
				way_code = 0x4000;
				break;
			case 3:
				way_code = 0x8000;
				break;
			default:
				way_code = 0x1000;
				break;
			}
			//pst_work->ID_template = (0x100 + label / 4) + way_code;
			nn_res->note_id = actual_label.denomi_code + way_code;			//テンプレートIDを設定する	
			//pst_work->ID_template = actual_label.denomi_code;

		}
	}

	nn_res->output_max_val = nn.result->data[0];
	nn_res->output_max_node = nn.result->data[1];
	nn_res->output_2nd_val = nn.result->data[2];
	nn_res->output_2nd_node = nn.result->data[3];

	free_proc(aime_feat);
	free_proc(output_l_1);
	free_proc(output_l_0);
	free_proc(result);
#endif
	return err;

}


