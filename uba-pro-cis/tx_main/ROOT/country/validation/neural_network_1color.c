#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define EXT
#include "../common/global.h"
#include "neural_network_1color.h"
#include <math.h>

#define sigmoid(x) (1.0 / (1 + exp(-x)))

int predict_neural_network_1color(u8 buf_num ,ST_VALI_NN1* vali_nn1)
{

	
	int i = 0;
	int j = 0;

	int plane = 0;

	int node=0;

	float sum = 0;

	float input[80];
	float hidden[16];
	float output[2] = {0};

	int input_count = 0;
	int hidden_count = 0;
	int output_count = 0;

	int hidden_weight = 0;
	int output_weight = 0;

	float data = 0;

	float norz = 1.0f / 255.0f;

	s32 res = 0;
	//float norz = 255.0f;
	input_count = vali_nn1->in_node_count;
	hidden_count = vali_nn1->hidden_node_count;
	output_count = vali_nn1->out_node_count;

	//“ü—Íƒf[ƒ^Ìæ
	for (node = 0; node < input_count; node++) 
	{
		if (vali_nn1->ppoint[node].side == 0) 
		{
			plane = UP_R_IR1;
		}
		else 
		{
			plane = DOWN_R_IR1;
		}

		//input[node] = (float)(get_mesh_data(vali_nn1->ppoint[node].x, vali_nn1->ppoint[node].y, 
		//	plane,vali_nn1->ir1_mask_ptn_diameter_x ,vali_nn1->ir1_mask_ptn_diameter_y ,vali_nn1->pir1_mask_ptn ,buf_num ,vali_nn1->ir1_mask_ptn_divide_num) * (float) norz);

		data = (float)(get_mesh_data(vali_nn1->ppoint[node].x, vali_nn1->ppoint[node].y, 
			plane,vali_nn1->ir1_mask_ptn_diameter_x ,vali_nn1->ir1_mask_ptn_diameter_y ,vali_nn1->pir1_mask_ptn ,buf_num ,vali_nn1->ir1_mask_ptn_divide_num));

		input[node] = data * (float) norz;

	}

	//’†ŠÔ‘w‰Šú‰»
	//for (j = 0; j < hidden_count; j++) 
	//{
	//	hidden[j] = 0;
	//}

	////‡“`”ÀŒvZ
	//for (i = 0; i < input_count; i++) 
	//{
	//	for (j = 0; j < hidden_count; j++) 
	//	{
	//		hidden[j] += (float)(input[i] * vali_nn1->phidden_weight[hidden_weight]);
	//		hidden_weight++;
	//	}
	//}

	//for (i = 0; i < hidden_count; i++)
	//{
	//	//hidden[i] = (float)(1.0 / (1.0 + exp((double)-hidden[i])));
	//	hidden[i] = (float)sigmoid(hidden[i]);
	//}

	for (j = 0; j < hidden_count; j++) 
	{
		sum = 0;

		for (i = 0; i < input_count; i++) 
		{
			sum += input[i] * vali_nn1->phidden_weight[hidden_weight];
			hidden_weight++;

		}

		hidden[j] = (float)sigmoid(sum);
	}

	
	//o—Í‘w‚Ì‰Šú‰»
	for (i = 0; i < output_count; i++) 
	{
		output[i] = 0;
	}

	////‡“`”À‚ÌŒvZ
	//for (i = 0; i < hidden_count; i++)
	//{
	//	for (j = 0; j < output_count; j++)
	//	{
	//		output[j] += (float)(hidden[i] * vali_nn1->pout_weight[output_weight]);
	//		output_weight++;
	//	}
	//}
	//for (i = 0; i < output_count; i++) 
	//{
	//	//output[i] = (float)(1.0 / (1.0 + exp((double)-output[i])));
	//	output[i] = (float)sigmoid(output[i]);
	//}

	for (j = 0; j < output_count; j++) 
	{
		sum = 0;
		for (i = 0; i < hidden_count; i++) 
		{
			sum += hidden[i] * vali_nn1->pout_weight[output_weight];
			output_weight++;
		}
		output[j] = (float)sigmoid(sum);
	}

	//o—Í”äŠr
	//i = 0;
	//for (j = 1; j < output_count; j++) 
	//{
	//	if (output[0] < output[1]) 
	//	{
	//		i = j;
	//	}
	//}
	if (output[0] < output[1])
	{
		res = j;
	}
	else
	{
		res = i;
	}


	vali_nn1->genuine_out_put_val = (float)output[0];  
	vali_nn1->counterfeit_out_put_val = (float)output[1];  

	return i;
}
