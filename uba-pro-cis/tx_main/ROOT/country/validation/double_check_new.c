/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
* MODEL NAME : 識別共有
* @file		double_check_new.c
* @brief	
* @date		2023/7/5
* @Version	1.0.0
* @updata
*	https://app.box.com/file/1251785891387
*/
/****************************************************************************/

//#define VS_DEBUG_DOUBLE

#include <stdio.h>
#include <string.h>
#include <math.h>

#define EXT
#include "../common/global.h"


#define max_num(a, b) (a > b ? a : b)

#define PART_A 0
#define PART_B 1
#define PART_C 2
#define PART_D 3

/*

2023/7/3 変更：中間ノード追加

*/

/// <summary>
/// B,C,D方向はA方向に変換
/// </summary>
/// <param name="insert_way"></param>
/// <param name="data_extraction"></param>
void convert_direction_data_juuken(u8 insertion_direction, ST_DATA_EXTRACTION *data_extraction)
{
	// Convert 方向
	u16 plane_count = data_extraction->plane_count;	// so plane
	u16 size_plane = data_extraction->count / plane_count; // so MP cua moi plane
	u8 split_y = (u8)data_extraction->split_y;
	u8 split_x = (u8)data_extraction->split_x;

	u16 i;
	u16 j;
	u8 x;
	u8 y;

	u16 juuken_data[PT_SIZE_MAX];
	if (insertion_direction != 0)
	{
		if (insertion_direction == 1)	// B方向 thi voi moi plane diem cuoi convert thanh diem dau
		{
			for (i = 0; i < plane_count; i++)
			{
				for (j = 0; j < size_plane; j++)
				{
					juuken_data[i*size_plane + j] = data_extraction->data_extra_out_put[i*size_plane + size_plane - j - 1];
				}
			}
		}
		else if (insertion_direction == 2)// Neu la huong C thi mat truoc thanh mat sau
		{
			for (i = 0; i < plane_count; i++)
			{
				for (y = 0; y < split_y; y++)
				{
					for (x = 0; x < split_x; x++)
					{
						juuken_data[i*size_plane + y * split_x + x] = data_extraction->data_extra_out_put[(i + 1)*size_plane + (-y - 1) * split_x + x];
					}
				}
			}
		}
		else if (insertion_direction == 3)
		{
			for (i = 0; i < plane_count; i++)
			{
				for (y = 0; y < split_y; y++)
				{
					for (x = 0; x < split_x; x++)
					{
						juuken_data[i*size_plane + y * split_x + x] = data_extraction->data_extra_out_put[i*size_plane + (y + 1) * split_x - x - 1];
					}
				}
			}
		}

		for (i = 0; i < data_extraction->count; i++)
		{
			data_extraction->data_extra_out_put[i] = juuken_data[i];
		}
	}

}

/// <summary>
/// 抽出データを4分割する
/// </summary>
/// <param name="data_extraction"></param>
/// <param name="jukenWeights"></param>
void split_4parts_NN(ST_DATA_EXTRACTION* data_extraction, ST_DOUBLE_OPTICS_NN* jukenWeights)
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

	u16 plane_count = data_extraction->plane_count;
	u16 split_y = data_extraction->split_y;
	u16 split_x = data_extraction->split_x;

	u16 plane_size = split_y * split_x;
	u16 countA = 0;
	u16 countB = 0;
	u16 countC = 0;
	u16 countD = 0;

	u8 split_xCenter = (u8)(split_x / 2);
	u8 split_yCenter = (u8)(split_y / 2);

	u8 plane;
	u16 y=0;
	u16 x=0;
	u16 data_ex = 0;
	u16 count = 0;

	for (plane = 0; plane < data_extraction->plane_count; plane++)
	{
		for (y = 0; y < split_y; y++)
		{
			for (x = 0; x < split_x; x++)
			{
				// コーナーフラグ確認
				if ((0 == data_extraction->corner_flg) && 
					( ((0 == x) && (0 == y)) || ((split_x == x + 1) && (split_y == y + 1)) || ((0 == x) && (split_y == y + 1)) || ((split_x == x + 1) && (0 == y)) ) )	// コーナーなら、
					{
						continue;// 何もしない
					}

				data_ex = data_extraction->data_extra_out_put[plane_size * plane + y * split_x + x];

				if ((y < split_yCenter) && (x < split_xCenter))
				{
					jukenWeights->Aface_features[countA] = (u8)data_ex;
					countA += 1;
				}
				else if ((y < split_yCenter) && (x >= split_xCenter))
				{
					jukenWeights->Bface_features[countB] = (u8)data_ex;
					countB += 1;
				}
				else if ((y >= split_yCenter) && (x < split_xCenter))
				{
					jukenWeights->Cface_features[countC] = (u8)data_ex;
					countC += 1;
				}
				else
				{
					jukenWeights->Dface_features[countD] = (u8)data_ex;
					countD += 1;
				}

				count++;
		}
		}
	}

	data_extraction->count = count;

}

/// <summary>
/// 光学重券NNの実行
/// </summary>
/// <param name="inputdata"></param>
/// <param name="weight"></param>
/// <param name="outputdata"></param>
/// <param name="inputnum"></param>
/// <param name="hiddennum"></param>
/// <param name="outputnum"></param>
/// <param name="biasflag"></param>
void double_optics_nn_predict_split(u8* inputdata, float* weight, float* outputdata, u16 inputnum, u16 hiddennum, u16 outputnum, u8 biasflag)
{
	float nor = 1.0f / 255.0f;

	float input_node[DOUBLE_OPTICS_NODE_MAX_NUM];
	float hidden_node[DOUBLE_OPTICS_NODE_MAX_NUM];
	float output_node[2];

	u16 in_num;
	u16 hidden_num;
	u16 out_num;
	u16 in_num2;

	float sum;
	float* we;
	float output;

	//
	for (in_num = 0; in_num < inputnum; ++in_num)
		{
		input_node[in_num] = ((float)inputdata[in_num] * nor);
		}

	//バイアス項ありの設定
	if (biasflag == 1)
			{
		input_node[inputnum] = 1.0f;	//	末尾にbias項
		inputnum++;
		}

	//hidden1 leyer
	for (hidden_num = 0; hidden_num < hiddennum; hidden_num++)
			{
		sum = 0.0;
		in_num2 = hidden_num * inputnum;

		for (in_num = 0; in_num < inputnum; ++in_num)
				{
			sum += input_node[in_num] * (*(weight + in_num2 + in_num));
				}
		hidden_node[hidden_num] = sigmoid(sum);
			}

	we = weight + (hiddennum * inputnum);

	//中間層の末尾にバイアス項を追加する
	if (biasflag == 1)
			{
		hidden_node[hiddennum] = 1.0f;
		hiddennum++;
			}

	//output layer
	for (out_num = 0; out_num < outputnum; ++out_num)
			{
		sum = 0.0;
		in_num2 = out_num * hiddennum;
			
		for (hidden_num = 0; hidden_num < hiddennum; ++hidden_num)
				{
			sum += hidden_node[hidden_num] * (*(we + in_num2 + hidden_num));
				}
			
		output = sigmoid(sum);

		outputdata[out_num] = output;
					}

	return;
}


/// <summary>
/// 光学重券アルゴ
/// </summary>
/// <param name="jukenWeights"></param>
/// <param name="double_result"></param>
void double_optics_nn_predict(ST_DOUBLE_OPTICS_NN* jukenWeights, ST_DOUBLE_OPTICS_NN_RESULT* double_result)
{
	u16 hidden_num = jukenWeights->hidden_num;
	u16 input_num = jukenWeights->input_num;
	u16 output_num = jukenWeights->output_num;

	float* we = jukenWeights->weight;
	u8 flag = jukenWeights->biasflag;

	u8 max = 0;
	float resultA[2] = { 0.0f };
	float resultB[2] = { 0.0f };
	float resultC[2] = { 0.0f };
	float resultD[2] = { 0.0f };
	float prob[4] = { 0.0f };
	u16 ii;

	u16 offset = hidden_num * (input_num + flag) + output_num * (hidden_num + flag);

	//分割位置に応じてNN実行
	double_optics_nn_predict_split(jukenWeights->Aface_features, we + offset * 0, resultA, input_num, hidden_num, output_num, flag);
	double_optics_nn_predict_split(jukenWeights->Bface_features, we + offset * 1, resultB, input_num, hidden_num, output_num, flag);
	double_optics_nn_predict_split(jukenWeights->Cface_features, we + offset * 2, resultC, input_num, hidden_num, output_num, flag);
	double_optics_nn_predict_split(jukenWeights->Dface_features, we + offset * 3, resultD, input_num, hidden_num, output_num, flag);

					max = resultA[0] > resultA[1] ? 0 : 1;
					jukenWeights->result[PART_A] = (u8)jukenWeights->A_Label[max];
					max = resultB[0] > resultB[1] ? 0 : 1;
					jukenWeights->result[PART_B] = (u8)jukenWeights->B_Label[max];
					max = resultC[0] > resultC[1] ? 0 : 1;
					jukenWeights->result[PART_C] = (u8)jukenWeights->C_Label[max];
					max = resultD[0] > resultD[1] ? 0 : 1;
	jukenWeights->result[PART_D] = (u8)jukenWeights->D_Label[max];


					prob[PART_A] = max_num(resultA[0], resultA[1]);
					prob[PART_B] = max_num(resultB[0], resultB[1]);
					prob[PART_C] = max_num(resultC[0], resultC[1]);
					prob[PART_D] = max_num(resultD[0], resultD[1]);

	for (ii = 0; ii < 4; ii++)
					{
		if (jukenWeights->result[ii] % 2 != 0)//部分重券
						{
							double_result->dbl_count += 1;
			double_result->prob_dbl_part[ii] = prob[ii];	//重券確率
						}
						else
						{
			double_result->prob_dbl_part[ii] = 1 - prob[ii];//重券確率
						}
					}

					double_result->prob_bill = (double_result->prob_dbl_part[0] + double_result->prob_dbl_part[3] - double_result->prob_dbl_part[0] * double_result->prob_dbl_part[3])
						* (double_result->prob_dbl_part[1] + double_result->prob_dbl_part[2] - double_result->prob_dbl_part[1] * double_result->prob_dbl_part[2]);

					// 紙幣をA、B、C、Dの4つの部分に分割し、各部分を重券検知する。隣り合う2部分以上が重券の場合、出力の結果は重券である。
					if ((double_result->dbl_count > 2) || ((double_result->dbl_count == 2) && ((jukenWeights->result[PART_A] + jukenWeights->result[PART_D]) % 2 != 0)))
					{
						double_result->label_predict = 1;
					}

#ifdef VS_DEBUG_DOUBLE
	FILE* fp;
	u8 buf_n = 0;
	if (work[buf_n].pbs->blank3 != 0)
	{
		fp = fopen("double.csv", "a");
		fprintf(fp, "%s,%x,%d,%s,", work[buf_n].pbs->blank4, work[buf_n].pbs->mid_res_nn.result_jcm_id, work[buf_n].pbs->insertion_direction, work[buf_n].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf_n].pbs->blank0[20], work[buf_n].pbs->ser_num1, work[buf_n].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf_n].pbs->spec_code.model_calibration, work[buf_n].pbs->spec_code.sensor_conf, work[buf_n].pbs->spec_code.mode);

		fprintf(fp, "%d,", double_result->dbl_count);
		fprintf(fp, "%f,", double_result->prob_bill);
		fprintf(fp, "%f,", double_result->prob_dbl_part[0]);
		fprintf(fp, "%f,", double_result->prob_dbl_part[1]);
		fprintf(fp, "%f,", double_result->prob_dbl_part[2]);
		fprintf(fp, "%f,", double_result->prob_dbl_part[3]);
		fprintf(fp, "%d,", double_result->label_predict);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif


	return;
}