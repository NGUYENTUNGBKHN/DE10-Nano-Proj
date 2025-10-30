/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_network.h
* Contents: neural network processing
*
*
*******************************************************************************/
#ifndef TEM_NETWORK_H
#define TEM_NETWORK_H

int itf_barcode_network_decode(float bar_ratios[ITF_BARCODE_BARS_PER_DIGIT]);
void find_network_points(void);
float get_sigmoid_value(const float input);
void run_forward_pass(const int input_nodes, const int hidden_nodes, const int output_nodes, const float* hidden_weight_pointer,
	const float* output_weight_pointer);

#endif
