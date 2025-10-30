#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#define EXT
#include "../common/global.h"

u32 calc_sumchk(float* p, int n);

//u32 deb_float_sum[10] = { 0 };


		static float	SigmoidLookupNegative[] = {
			  2.449187e-001F, 5.000000e-001F,		/* - 0.500000	*/
			  2.171985e-001F, 4.861399e-001F,		/* - 1.000000	*/
			  1.730318e-001F, 4.419732e-001F,		/* - 1.500000	*/
			  1.264452e-001F, 3.720933e-001F,		/* - 2.000000	*/
			  8.668949e-002F, 2.925819e-001F,		/* - 2.500000	*/
			  5.686462e-002F, 2.180197e-001F,		/* - 3.000000	*/
			  3.622729e-002F, 1.561077e-001F,		/* - 3.500000	*/
			  2.265204e-002F, 1.085944e-001F,		/* - 4.000000	*/
			  1.399853e-002F, 7.398035e-002F,		/* - 4.500000	*/
			  8.588184e-003F, 4.963377e-002F,		/* - 5.000000	*/
			  5.245426e-003F, 3.291998e-002F,		/* - 5.500000	*/
			  3.195029e-003F, 2.164280e-002F,		/* - 6.000000	*/
			  1.942882e-003F, 1.412991e-002F,		/* - 6.500000	*/
			  1.180262e-003F, 9.172886e-003F,		/* - 7.000000	*/
			  7.165451e-004F, 5.926867e-003F,		/* - 7.500000	*/
			  4.348570e-004F, 3.814206e-003F,		/* - 8.000000	*/
			  2.638463e-004F, 2.446121e-003F,		/* - 8.500000	*/
			  1.600648e-004F, 1.563978e-003F,		/* - 9.000000	*/
			  9.709669e-005F, 9.972649e-004F,		/* - 9.500000	*/
			  5.889672e-005F, 6.343650e-004F,		/* -10.000000	*/
			  3.572436e-005F, 4.026414e-004F,		/* -10.500000	*/
			  2.166854e-005F, 2.550553e-004F,		/* -11.000000	*/
			  1.314286e-005F, 1.612729e-004F,		/* -11.500000	*/
			  7.971633e-006F, 1.018038e-004F,		/* -12.000000	*/
			  4.835071e-006F, 6.416503e-005F,		/* -12.500000	*/
			  2.932630e-006F, 4.038452e-005F,		/* -13.000000	*/
			  1.778734e-006F, 2.538387e-005F,		/* -13.500000	*/
			  1.078858e-006F, 1.593555e-005F,		/* -14.000000	*/
			  6.543613e-007F, 9.992586e-006F,		/* -14.500000	*/
			  3.968904e-007F, 6.259258e-006F,		/* -15.000000	*/
			  2.407263e-007F, 3.916796e-006F,		/* -15.500000	*/
			  1.460079e-007F, 2.448661e-006F,		/* -16.000000	*/
		};

		static float	SigmoidLookupPositive[] = {
			  2.449187e-001F, 5.000000e-001F,		/*	 0.000000	*/
			  2.171985e-001F, 5.138601e-001F,		/*	 0.500000	*/
			  1.730318e-001F, 5.580268e-001F,		/*	 1.000000	*/
			  1.264452e-001F, 6.279067e-001F,		/*	 1.500000	*/
			  8.668949e-002F, 7.074181e-001F,		/*	 2.000000	*/
			  5.686462e-002F, 7.819803e-001F,		/*	 2.500000	*/
			  3.622729e-002F, 8.438923e-001F,		/*	 3.000000	*/
			  2.265204e-002F, 8.914056e-001F,		/*	 3.500000	*/
			  1.399853e-002F, 9.260197e-001F,		/*	 4.000000	*/
			  8.588184e-003F, 9.503663e-001F,		/*	 4.500000	*/
			  5.245426e-003F, 9.670800e-001F,		/*	 5.000000	*/
			  3.195029e-003F, 9.783572e-001F,		/*	 5.500000	*/
			  1.942882e-003F, 9.858701e-001F,		/*	 6.000000	*/
			  1.180262e-003F, 9.908271e-001F,		/*	 6.500000	*/
			  7.165451e-004F, 9.940732e-001F,		/*	 7.000000	*/
			  4.348570e-004F, 9.961858e-001F,		/*	 7.500000	*/
			  2.638463e-004F, 9.975539e-001F,		/*	 8.000000	*/
			  1.600648e-004F, 9.984360e-001F,		/*	 8.500000	*/
			  9.709669e-005F, 9.990028e-001F,		/*	 9.000000	*/
			  5.889672e-005F, 9.993656e-001F,		/*	 9.500000	*/
			  3.572436e-005F, 9.995974e-001F,		/*	10.000000	*/
			  2.166854e-005F, 9.997450e-001F,		/*	10.500000	*/
			  1.314286e-005F, 9.998387e-001F,		/*	11.000000	*/
			  7.971633e-006F, 9.998982e-001F,		/*	11.500000	*/
			  4.835071e-006F, 9.999358e-001F,		/*	12.000000	*/
			  2.932630e-006F, 9.999596e-001F,		/*	12.500000	*/
			  1.778734e-006F, 9.999746e-001F,		/*	13.000000	*/
			  1.078858e-006F, 9.999841e-001F,		/*	13.500000	*/
			  6.543613e-007F, 9.999900e-001F,		/*	14.000000	*/
			  3.968904e-007F, 9.999937e-001F,		/*	14.500000	*/
			  2.407263e-007F, 9.999961e-001F,		/*	15.000000	*/
			  1.460079e-007F, 9.999976e-001F,		/*	15.500000	*/
		};


float sigmoid(float x) 
{
	int		no;
    float	a;
    float	b;

	if (x >= 16.0F)
	{
		return 1.0F;
	}
	else if (x <= -16.0F)
	{
		return 0.0F;
	}

	//if (x < 0){
	//	no = (int)((x * -1) / 0.5f);
	//	a = sigmoid_m_table[no][0];
	//	b = sigmoid_m_table[no][1];
	//}
	//else {
	//	no = (int)(x / 0.5f);
	//	a = sigmoid_p_table[no][0];
	//	b = sigmoid_p_table[no][1];
	//}

	// 入力が負数の場合
	if ( x < 0.0F ) {
		//				tableIndex = ( int )( input * -1.0F / 0.5F );
		no = ( int )( x * -2.0F );
		a  = SigmoidLookupNegative[no*2];
		b  = SigmoidLookupNegative[no*2 + 1];
	}

	// 入力が正数の場合
	else {
		//				tableIndex = ( int )( input / 0.5F );
		no = ( int )( x * 2.0F );
		a  = SigmoidLookupPositive[no*2];
		b  = SigmoidLookupPositive[no*2 + 1];
	}


	return ((a * x) + b);
}

void NN(ST_NN *nn_cfg, ST_NN_RESULT *nn_res)
{
	u16 cen_num = 0;
	u32 in_num  = 0;
	u32 out_num = 0;

	float val = 0.0f;
	float th3 = 0.0f;
	float th5 = 0.0f;
	
	float InputSum;
	//下記二つをローカルからグローバルへ定義を変更
	//float input[960];	
	//float hidden1[120];
	float output;
	//float output_res[50];

	float output_max = 0;
	s16 max_node_num = -1;
	float output_2nd = 0;
	s16 max_2nd_node_num = -1;
	u32 in_num2 = 0;

	float nor = 1.0f / 255.0f;				//正規化用
	float note_size_x_nor = 1.0f / 255.0f;	//サイズ正規化用 長手身近手で正規化の仕方が変わるのでそれぞれ必要
    float note_size_y_nor = 1.0f / 255.0f;	//ｙ

	//float inputsum_deb[300];	// deb_float

	//インプットデータ0~255を0~1に変換
	if (nn_cfg->do_normalize == 1)
	{
		for (in_num = 0; in_num < nn_cfg->in_node; ++in_num)
	{
		//nn_cfg->in_put[in_num] = ((float)data_extraction.data_extra_out_put[in_num] * nor);	//メンバの配列に変更しないといけない。　19/07/11
		nn_cfg->in_put[in_num] = ((float)nn_cfg->p_input_rawdata[in_num] * nor);	//メンバの配列に変更しないといけない。　19/07/11
	}
	}

	//紙幣サイズありの設定
	if(nn_cfg->sizeflag == 1)
	{
		//in_num = in_num + 1;
		nn_cfg->in_put[in_num++] = nn_cfg->width * note_size_x_nor;	//	末尾にbias項
		nn_cfg->in_put[in_num] = nn_cfg->length * note_size_y_nor;	//	末尾にbias項
		nn_cfg->in_node = nn_cfg->in_node + 2;
	}
	//バイアス項ありの設定
	if(nn_cfg->biasflag == 1)
	{
		//in_num = in_num + 1;
		nn_cfg->in_put[in_num] = 1.0f;	//	末尾にbias項
		nn_cfg->in_node++;		//　入力層数を１追加
	}


	//nn_res->deb_float_sum[0] = calc_sumchk(&nn_cfg->in_put[0], nn_cfg->in_node);	// float chk sum 生成 入力層値
	//nn_res->deb_float_sum[1] = calc_sumchk(&nn_cfg->pcenter_wit_offset[0], nn_cfg->center_node * nn_cfg->in_node);	// float chk sum 生成 ウェイト値1



	//hidden1 leyer
	for(cen_num = 0; cen_num < nn_cfg->center_node; cen_num++)
	{
		InputSum = 0.0;
		in_num2 = cen_num * nn_cfg->in_node;

		for(in_num = 0; in_num < nn_cfg->in_node; ++in_num)
		{

			InputSum += nn_cfg->in_put[in_num] * nn_cfg->pcenter_wit_offset[ in_num2 + in_num];


			/*data_extraction.data_extra_out_put[in_num]に抽出したデータが入っています*/
		}
		//inputsum_deb[cen_num] = InputSum;	// deb_float
		nn_cfg->hidden1[cen_num] = sigmoid(InputSum);
	}


	//nn_res->deb_float_sum[2] = calc_sumchk(&inputsum_deb[0], nn_cfg->center_node);	// float chk sum 積和値1
	//nn_res->deb_float_sum[3] = calc_sumchk(&nn_cfg->hidden1[0], nn_cfg->center_node);	// float chk sum シグモイド1出力値
	//nn_res->deb_float_sum[4] = calc_sumchk(&nn_cfg->pout_wit_offset[0], nn_cfg->center_node * nn_cfg->out_nide);	// float chk sum 生成 ウェイト値2


	//中間層の末尾にバイアス項を追加する
	if(nn_cfg->biasflag == 1)
	{
		nn_cfg->center_node++;	//	中間層数を1追加
		cen_num = cen_num + 1; 
		nn_cfg->hidden1[cen_num] = 1.0f;
	}

	//output layer
	for(out_num = 0; out_num < nn_cfg->out_nide; ++out_num)
	{
		InputSum = 0.0;
		in_num2 = out_num * nn_cfg->center_node;

		for(cen_num = 0; cen_num < nn_cfg->center_node; ++cen_num)
		{
			InputSum += nn_cfg->hidden1[cen_num] * nn_cfg->pout_wit_offset[in_num2 + cen_num];
		}
		//inputsum_deb[cen_num] = InputSum;	// deb_float
		output = sigmoid(InputSum);

		nn_cfg->output[out_num] =  output;

		/*最大発火値と２nd発火値*/
		if(output_max < output)//最大発火
		{
			output_2nd = output_max;
			max_2nd_node_num = max_node_num;

			output_max = output;
			max_node_num = (s16)out_num;
		}

		else if(output > output_2nd)//2nd発火
		{
			output_2nd = output;
			max_2nd_node_num = (s16)out_num;
		}

	}

	//nn_res->deb_float_sum[5] = calc_sumchk(&inputsum_deb[0], nn_cfg->out_nide);	// float chk sum 積和値2
	//nn_res->deb_float_sum[6] = calc_sumchk(&nn_cfg->output[0], nn_cfg->out_nide);	// float chk sum シグモイド2出力値



	/*最大発火値と２nd発火値*/
	nn_res->output_max_val = output_max;
	nn_res->output_max_node = max_node_num;

	nn_res->output_2nd_val = output_2nd;
	nn_res->output_2nd_node = max_2nd_node_num;

	//TH3(softmax)を計算
	for(out_num = 0; out_num < nn_cfg->out_nide; out_num++)
	{
		val = val + (float)(exp(nn_cfg->output[out_num]));	
	}

	th3 = (float)(exp(nn_res->output_max_val)) / val;	

	//TH5(err)を計算	
	for(out_num = 0; out_num < nn_cfg->out_nide; out_num++)
	{
		if(nn_res->output_max_node == out_num)	//目標ノードは(１－N)^2
		{
			val = (1 - nn_cfg->output[out_num]);
			th5 = val * val + th5;
		}
		else							//それ以外は(N)^2
		{
			th5 = nn_cfg->output[out_num] * nn_cfg->output[out_num] + th5;
		}
		
	}

	nn_res->th3 = th3;
	nn_res->th5 = th5;
	
}

//分類器の発火値の計算を行います。
//nn_thrsが閾値
//nn_resが計算値
u32 out_put_value_cal(ST_NN_NODE_THRESHOLD* nn_thrs ,ST_NN_RESULT *nn_res)
{
	float delta_out;

	delta_out = nn_res->output_max_val - nn_res->output_2nd_val;

	if(nn_thrs->th1 >= nn_res->output_max_val)	//th1
	{
		return ERR_FAILURE_NN_TH1;
	}
	if(nn_thrs->th2 >= delta_out)				//th2
	{
		return ERR_FAILURE_NN_TH2;
	}
	if(nn_thrs->th3 > nn_res->th3)				//th3
	{
		//return ERR_FAILURE_NN_TH3;	// 理論が不安定なのでコメントアウト 191216furuta 
	}
	if(nn_thrs->th5 < nn_res->th5)				//th5
	{
		//return ERR_FAILURE_NN_TH5;	// 理論が不安定なのでコメントアウト 191216furuta 
	}

	return 0;

}//out_put_value_cal end


u32 calc_sumchk(float* p, int n)
{
	int ii;
	u32 chksum = 0;

	// floatをバイナリレベルでsum値計算
	for (ii = 0; ii < n; ii++)
	{

		chksum += *((u32*)(&p[ii]));
	}
	return chksum;
}
