#ifndef _NEURAL_NETWORK_1COLOR_H_
#define _NEURAL_NETWORK_1COLOR_H_

#define INPUT_NODE 240
#define HIDDEN_NODE 16
#define OUTPUT_NODE 2



//NN用構造体
typedef struct
{	
	u16 in_node_count;		//入力ノード数	
	u8 hidden_node_count;	//中間ノード数
	u8 out_node_count;		//出力ノード数

    float genuine_out_put_val;		//真券の発火値
	float counterfeit_out_put_val;	//偽券の発火値

	ST_NN_POINTS *ppoint;					//ポイントデータ

	float *phidden_weight;
	float *pout_weight;

	u16 ir1_mask_ptn_diameter_x;			// IR1色マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1色マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1色マスクパターンの割る数
	u8*	pir1_mask_ptn;						// IR1色マスクパターン1のポインタ
	
} ST_VALI_NN1;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	int predict_neural_network_1color(u8 buf_num ,ST_VALI_NN1* nn1);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
