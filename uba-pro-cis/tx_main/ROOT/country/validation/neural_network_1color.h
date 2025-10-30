#ifndef _NEURAL_NETWORK_1COLOR_H_
#define _NEURAL_NETWORK_1COLOR_H_

#define INPUT_NODE 240
#define HIDDEN_NODE 16
#define OUTPUT_NODE 2



//NN�p�\����
typedef struct
{	
	u16 in_node_count;		//���̓m�[�h��	
	u8 hidden_node_count;	//���ԃm�[�h��
	u8 out_node_count;		//�o�̓m�[�h��

    float genuine_out_put_val;		//�^���̔��Βl
	float counterfeit_out_put_val;	//�U���̔��Βl

	ST_NN_POINTS *ppoint;					//�|�C���g�f�[�^

	float *phidden_weight;
	float *pout_weight;

	u16 ir1_mask_ptn_diameter_x;			// IR1�F�}�X�N�p�^�[���̒��ax
	u16 ir1_mask_ptn_diameter_y;			// IR1�F�}�X�N�p�^�[���̒��ay
	float ir1_mask_ptn_divide_num;			// IR1�F�}�X�N�p�^�[���̊��鐔
	u8*	pir1_mask_ptn;						// IR1�F�}�X�N�p�^�[��1�̃|�C���^
	
} ST_VALI_NN1;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	int predict_neural_network_1color(u8 buf_num ,ST_VALI_NN1* nn1);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
