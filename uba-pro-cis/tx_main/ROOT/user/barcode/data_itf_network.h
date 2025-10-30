/*******************************************************************************
* Project: CIS Bill Acceptor
* File: data_itf_network.h
* Contents: nn barcode
*
*
*******************************************************************************/
#ifndef DATA_ITF_NETWORK_H
#define DATA_ITF_NETWORK_H

#define	ITF_BARCODE_INPUT_NODE_COUNT	5
#define	ITF_BARCODE_HIDDEN_NODE_COUNT	20
#define	ITF_BARCODE_OUTPUT_NODE_COUNT	10

extern const float itf_barcode_hidden_weights[ITF_BARCODE_HIDDEN_NODE_COUNT][ITF_BARCODE_INPUT_NODE_COUNT];
extern const float itf_barcode_output_weights[ITF_BARCODE_OUTPUT_NODE_COUNT][ITF_BARCODE_HIDDEN_NODE_COUNT];

#endif	/* DATA_ITF_NETWORK_H */
