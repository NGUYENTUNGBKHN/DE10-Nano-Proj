#ifndef _MLP_CLASSIFIER_H_
#define _MLP_CLASSIFIER_H_

#include <stdio.h>
//#define EXT
//#include "../../common/global.h"

//#include "MLPModule.h"

typedef struct //Identタスク出力
{
	u32 note_id;	//券種ID
	u8 way;			//方向コード
	u8 ident_err_code;	//エラーコード
	u16 reject;		//リジェクトコード
	float output_max_val;	//最大発火値
	float output_2nd_val;	//2nd発火値
	u32 output_max_node;	//最大発火のノード
	u32 output_2nd_node;	//2nd発火のノード
	float th3;				//th3(softmax)の計算結果
	float th5;				//th5(err)の計算結果
	u32 deb_float_sum[10];
} ST_NEW_NN_RESULT;

struct MLPClassifier {
	NN_PerceptronLayer* layers;
	NN_Tensor* result;
};

typedef struct MLPClassifier MLPClassifier;

typedef struct
{
	T_DECISION_PARAMETER_COMMON	comn;

	//Point
	u8	x_dev_num;							// 横分割数
	u8	y_dev_num;							// 縦分割数
	u8	ei_corner;							// コーナー無効に　する:0/しない:1
	u8	padding[4];
	u8  num;								// 有効要素数　MAX16
	T_POINTDT_MASK_INFO	mask_info[16];


	u8 plane;
	u8 Reserved0[1];
	u16 Is_Symmetric;
	u32 OffsetROI;
	u16 orientations;
	u16 pixels_per_cell;
	u16 cells_per_block;
	u8 Reserved1[2];
	u32 n_input;

	u32 n_hidden;//24
	u32 n_output;//28
	u32 n_label;//32
	u32 activation_hidden;//36  //0 ident,1 relu ,2 softmax
	u32 activation_output;//40   //0 ident,1 relu ,2 softmax
	u32 Offset_Hidden;//44
	u32 Offset_Bias_Hidden;//48
	u32 Offset_Output;//52
	u32 Offset_Bias_Output;//56
	u32 Offset_label;//60
}new_nn_config;



#ifdef __cplusplus
extern "C"
{
#endif/*__cplusplus*/

	void inference_MLP(MLPClassifier* clf, NN_Tensor* input);
	u16 integrate_test(u8 buf_num, u8* image_data, u16 height, u16 width, new_nn_config* nn_config, ST_DATA_EXTRACTION* data_extraction, u8* start_block, ST_NEW_NN_RESULT* nn_res);
	u16	New_NN_judeg_proc_result(u8 buf, new_nn_config* nn_cfg, ST_NEW_NN_RESULT* nn_res);

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*_MLP_CLASSIFIER_H_*/
