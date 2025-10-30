/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_network.c
* Contents: neural network processing
*
*
*******************************************************************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "custom.h"
#include "bv_errcode.h"
#include <string.h>

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
#include "tem_global.c"

#include "tem_network.h"
#if (defined ENABLE_TITO_TICKET)
#include "data_itf_network.h"
#endif


static float input_result[ITF_BARCODE_INPUT_NODE_COUNT];
static float hidden_result[600];
static float output_result[600];


#if (defined ENABLE_TITO_TICKET)
int itf_barcode_network_decode(float bar_ratios[ITF_BARCODE_BARS_PER_DIGIT])
{
	int bar_count = 0;
	int output_count = 0;

	float highest_value = 0.0;
	float second_highest_value = 0.0;

	int highest_index = 0;

	for(bar_count = 0; bar_count < ITF_BARCODE_BARS_PER_DIGIT; bar_count++)
	{
		input_result[bar_count] = bar_ratios[bar_count];
	}

	run_forward_pass(ITF_BARCODE_INPUT_NODE_COUNT, ITF_BARCODE_HIDDEN_NODE_COUNT, ITF_BARCODE_OUTPUT_NODE_COUNT,
		&itf_barcode_hidden_weights[0][0], &itf_barcode_output_weights[0][0]);

	//find highest value
	for(output_count = 0; output_count < ITF_BARCODE_OUTPUT_NODE_COUNT; output_count++)
	{
		if(output_result[output_count] > highest_value)
		{
			highest_value = output_result[output_count];
			highest_index = output_count;
		}
	}

	//find second highest value
	for(output_count = 0; output_count < ITF_BARCODE_OUTPUT_NODE_COUNT; output_count++)
	{
		if(output_result[output_count] > second_highest_value && output_result[output_count] != highest_value)
		{
			second_highest_value = output_result[output_count];
		}
	}

#if DEBUG_VALIDATION_RESULT
	ex_validation.barcode_result.highest_value[ex_barcode_charactor_count] = highest_value;
	ex_validation.barcode_result.second_highest_value[ex_barcode_charactor_count] = second_highest_value;
	if(ex_barcode_charactor_count < 28)
	{
		ex_barcode_charactor_count++;
	}
#endif

	//only identify as a digit if there is a margin
	if(highest_value - second_highest_value < ITF_BARCODE_NET_MARGIN)
	{
		return 10; //unknown digit
	}

	return highest_index;
}
#endif



const float sigmoid_lookup_negative[][2] =
{/*		value 1			value 2								*/
	{	2.449187e-001F, 5.000000e-001F	},	/* - 0.500000	*/
	{	2.171985e-001F, 4.861399e-001F	},	/* - 1.000000	*/
	{	1.730318e-001F, 4.419732e-001F	},	/* - 1.500000	*/
	{	1.264452e-001F, 3.720933e-001F	},	/* - 2.000000	*/
	{	8.668949e-002F, 2.925819e-001F	},	/* - 2.500000	*/
	{	5.686462e-002F, 2.180197e-001F	},	/* - 3.000000	*/
	{	3.622729e-002F, 1.561077e-001F	},	/* - 3.500000	*/
	{	2.265204e-002F, 1.085944e-001F	},	/* - 4.000000	*/
	{	1.399853e-002F, 7.398035e-002F	},	/* - 4.500000	*/
	{	8.588184e-003F, 4.963377e-002F	},	/* - 5.000000	*/
	{	5.245426e-003F, 3.291998e-002F	},	/* - 5.500000	*/
	{	3.195029e-003F, 2.164280e-002F	},	/* - 6.000000	*/
	{	1.942882e-003F, 1.412991e-002F	},	/* - 6.500000	*/
	{	1.180262e-003F, 9.172886e-003F	},	/* - 7.000000	*/
	{	7.165451e-004F, 5.926867e-003F	},	/* - 7.500000	*/
	{	4.348570e-004F, 3.814206e-003F	},	/* - 8.000000	*/
	{	2.638463e-004F, 2.446121e-003F	},	/* - 8.500000	*/
	{	1.600648e-004F, 1.563978e-003F	},	/* - 9.000000	*/
	{	9.709669e-005F, 9.972649e-004F	},	/* - 9.500000	*/
	{	5.889672e-005F, 6.343650e-004F	},	/* -10.000000	*/
	{	3.572436e-005F, 4.026414e-004F	},	/* -10.500000	*/
	{	2.166854e-005F, 2.550553e-004F	},	/* -11.000000	*/
	{	1.314286e-005F, 1.612729e-004F	},	/* -11.500000	*/
	{	7.971633e-006F, 1.018038e-004F	},	/* -12.000000	*/
	{	4.835071e-006F, 6.416503e-005F	},	/* -12.500000	*/
	{	2.932630e-006F, 4.038452e-005F	},	/* -13.000000	*/
	{	1.778734e-006F, 2.538387e-005F	},	/* -13.500000	*/
	{	1.078858e-006F, 1.593555e-005F	},	/* -14.000000	*/
	{	6.543613e-007F, 9.992586e-006F	},	/* -14.500000	*/
	{	3.968904e-007F, 6.259258e-006F	},	/* -15.000000	*/
	{	2.407263e-007F, 3.916796e-006F	},	/* -15.500000	*/
	{	1.460079e-007F, 2.448661e-006F	},	/* -16.000000	*/
};

const float sigmoid_lookup_positive[][2] =
{/*		value 1			value 2								*/
	{	2.449187e-001F, 5.000000e-001F	},	/*	 0.000000	*/
	{	2.171985e-001F, 5.138601e-001F	},	/*	 0.500000	*/
	{	1.730318e-001F, 5.580268e-001F	},	/*	 1.000000	*/
	{	1.264452e-001F, 6.279067e-001F	},	/*	 1.500000	*/
	{	8.668949e-002F, 7.074181e-001F	},	/*	 2.000000	*/
	{	5.686462e-002F, 7.819803e-001F	},	/*	 2.500000	*/
	{	3.622729e-002F, 8.438923e-001F	},	/*	 3.000000	*/
	{	2.265204e-002F, 8.914056e-001F	},	/*	 3.500000	*/
	{	1.399853e-002F, 9.260197e-001F	},	/*	 4.000000	*/
	{	8.588184e-003F, 9.503663e-001F	},	/*	 4.500000	*/
	{	5.245426e-003F, 9.670800e-001F	},	/*	 5.000000	*/
	{	3.195029e-003F, 9.783572e-001F	},	/*	 5.500000	*/
	{	1.942882e-003F, 9.858701e-001F	},	/*	 6.000000	*/
	{	1.180262e-003F, 9.908271e-001F	},	/*	 6.500000	*/
	{	7.165451e-004F, 9.940732e-001F	},	/*	 7.000000	*/
	{	4.348570e-004F, 9.961858e-001F	},	/*	 7.500000	*/
	{	2.638463e-004F, 9.975539e-001F	},	/*	 8.000000	*/
	{	1.600648e-004F, 9.984360e-001F	},	/*	 8.500000	*/
	{	9.709669e-005F, 9.990028e-001F	},	/*	 9.000000	*/
	{	5.889672e-005F, 9.993656e-001F	},	/*	 9.500000	*/
	{	3.572436e-005F, 9.995974e-001F	},	/*	10.000000	*/
	{	2.166854e-005F, 9.997450e-001F	},	/*	10.500000	*/
	{	1.314286e-005F, 9.998387e-001F	},	/*	11.000000	*/
	{	7.971633e-006F, 9.998982e-001F	},	/*	11.500000	*/
	{	4.835071e-006F, 9.999358e-001F	},	/*	12.000000	*/
	{	2.932630e-006F, 9.999596e-001F	},	/*	12.500000	*/
	{	1.778734e-006F, 9.999746e-001F	},	/*	13.000000	*/
	{	1.078858e-006F, 9.999841e-001F	},	/*	13.500000	*/
	{	6.543613e-007F, 9.999900e-001F	},	/*	14.000000	*/
	{	3.968904e-007F, 9.999937e-001F	},	/*	14.500000	*/
	{	2.407263e-007F, 9.999961e-001F	},	/*	15.000000	*/
	{	1.460079e-007F, 9.999976e-001F	},	/*	15.500000	*/
};


//return the value of the sigmoid function
float get_sigmoid_value(const float input)
{
	float output = 0.0;
	float value_1 = 0.0;
	float value_2 = 0.0;

	int table_index = 0;

	//overflow case
	if(input > 16.0)
	{
		return 1.0;
	}

	//underflow case
	if(input < -16.0)
	{
		return 0.0;
	}

	if(input < 0.0) //negative case
	{
		table_index = (int)((input * -1.0) / 0.5);
		value_1 = sigmoid_lookup_negative[table_index][0];
		value_2 = sigmoid_lookup_negative[table_index][1];
	}
	else //positive case
	{
		table_index = (int)(input / 0.5);
		value_1 = sigmoid_lookup_positive[table_index][0];
		value_2 = sigmoid_lookup_positive[table_index][1];
	}

	output = value_1 * input + value_2;

	return output;
}


//calculate neural network output values
void run_forward_pass(const int input_nodes, const int hidden_nodes, const int output_nodes, const float* hidden_weight_pointer, const float* output_weight_pointer)
{
	int input_count = 0;
	int hidden_count = 0;
	int output_count = 0;

	float input_sum = 0.0;

	//find result of hidden layer
	for(hidden_count = 0; hidden_count < hidden_nodes; hidden_count++)
	{
		input_sum = 0.0;

		for(input_count = 0; input_count < input_nodes; input_count++)
		{
			input_sum += input_result[input_count] * (*hidden_weight_pointer);
			hidden_weight_pointer++;
		}

		hidden_result[hidden_count] = get_sigmoid_value(input_sum);
	}

	//find result of output layer
	for(output_count = 0; output_count < output_nodes; output_count++)
	{
		input_sum = 0.0;

		for(hidden_count = 0; hidden_count < hidden_nodes; hidden_count++)
		{
			input_sum += hidden_result[hidden_count] * (*output_weight_pointer);
			output_weight_pointer++;
		}

		output_result[output_count] = get_sigmoid_value(input_sum);
#if DEBUG_VALIDATION_RESULT
		ex_validation.barcode_result.output_layerinput[ex_barcode_charactor_count][output_count] = input_sum;
#endif
	}
}
