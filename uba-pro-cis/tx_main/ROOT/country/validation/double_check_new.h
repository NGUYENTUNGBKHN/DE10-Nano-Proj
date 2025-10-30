/****************************************************************************/
/**
* MODEL NAME : ���ʋ��L
* @file		double_check_new.c
* @brief
* @date		2023/7/5
* @Version	1.0.0
* @updata
*
*/
/****************************************************************************/



#include <stdio.h>
#include <string.h>
#include <math.h>


/*

2023/7/3 �ύX�F���ԃm�[�h�ǉ�

*/


#ifndef	_NEW_DOUBLE_CHK_H
#define	_NEW_DOUBLE_CHK_H


#define DOUBLE_OPTICS_NODE_MAX_NUM 400



typedef struct
{
	// _____________________________
	//|              |              |
	//|       A      |       B      |
	//|              |              |
	//|--------------|--------------|
	//|       C      |       D      |
	//|              |              |
	//|______________|______________|
	//

	float* weight;

	u16 A_Label[2];
	u16 B_Label[2];
	u16 C_Label[2];
	u16 D_Label[2];

	u8 result[4]; //result of 4 parts
	u8 Aface_features[DOUBLE_OPTICS_NODE_MAX_NUM];
	u8 Bface_features[DOUBLE_OPTICS_NODE_MAX_NUM];
	u8 Cface_features[DOUBLE_OPTICS_NODE_MAX_NUM];
	u8 Dface_features[DOUBLE_OPTICS_NODE_MAX_NUM];

	u16 input_num;
	u16 hidden_num;
	u16 output_num;
	u8 biasflag;

}ST_DOUBLE_OPTICS_NN;

typedef struct
{
	u8 label_predict;	    // �O�F�^���A�P�F�d��	(�d�����m�̗\�����ʁj
	float prob_bill;	    //�S�̎����̏d���m���i�d�����x��)
	float prob_dbl_part[4];	//4�����̏d���m��
	u8 dbl_count;	        //�\�������̐���juuken�ł���

} ST_DOUBLE_OPTICS_NN_RESULT;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void convert_direction_data_juuken(u8 insert_way, ST_DATA_EXTRACTION *data_extraction);
void split_4parts_NN(ST_DATA_EXTRACTION* data_extraction, ST_DOUBLE_OPTICS_NN* jukenWeights);
void double_optics_nn_predict(ST_DOUBLE_OPTICS_NN* jukenWeights, ST_DOUBLE_OPTICS_NN_RESULT* double_result);
void double_optics_nn_predict_split(u8* inputdata, float* weight, float* outputdata, u16 inputnum, u16 hiddennum, u16 outputnum, u8 biasflag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NEW_DOUBLE_CHK_H */
