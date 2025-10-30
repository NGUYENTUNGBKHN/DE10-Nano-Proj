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
 * @file capacitance_tape.c
 * @brief　静電容量テープ検知
 * @date 2020/06/29 Created.
 */
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>

#define EXT
#include "../common/global.h"

#define WINDOW_SIZE 5

#define OFFSET_VAL	500.0f
#define DATA_SIZE	1024.0f

//静電容量テープ検知のメイン関数
// in	バッファ番号
//		専用構造体
s16	capacitance_tape(u8 buf_num ,ST_CAPACITANCE_TAPE *cap)
{
	//サンプリングデータ構造体
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;

	//頂点座標用構造体
	ST_VERTEX pver;

	//パラメタ初期化
	s32 i = 0;
	//u32 ii = 0;
	u16 x = 0;
	//u16 y = 0;
//	u16 size_x = 0;
	u16 size_y = 0;

	//u16 x_length = 0;
	//s16 y_length = 0;

	//u32 input_data_num = 0;
	//float decision = 0;

	float bc_val = 0.0f;
	float bc_num_rev;
	//float a = 0.0f;
	//s16 filter_fraction = (s16)(WINDOW_SIZE * 0.5f);
	//float dev_num = 1.0f / WINDOW_SIZE;
	float dev_num = 1.0f / DATA_SIZE;
	ST_SPOINT spt;
	//float sum = 0;
	//s32 i_offset = 0;
	u8  area_i = 0;
	s16  x_i = 0;
	s16  y_i = 0;
	u32  input_data_counter = 0;
	u32  elem_i = 0;
	float tmp_num = 0;

	u16 x_size = 0;
	u16 y_size = 0;

	spt.p_plane_tbl = UP_CAP1;
	spt.l_plane_tbl = UP_CAP1;
	spt.trace_flg = 1;


	//FILE* fp;	//デバッグ用後で消す

	//頂点座標取得
	get_vertex(&pver, buf_num);

	//----------------------
	//+　黒補正値の計算
	//----------------------

	//y座標のサイズを計算
	size_y = (u16)(pbs->block_count * (float)(pbs->Blocksize / pbs->PlaneInfo[spt.p_plane_tbl].sub_sampling_pitch));
	
	//黒補正値のスキャンライン数の逆数
	bc_num_rev = 1.0f / (cap->black_correction_e - cap->black_correction_s);	

	if (cap->black_correction_e < cap->black_correction_s)	//エラーチェック１
	{
		cap->err_code = RES_ERR;                            //黒補正エリア指定で、終了座標より開始座標のほうが大きい場合エラー
		return -1;
	}

	if (size_y < cap->black_correction_e)                   //エラーチェック２
	{
		cap->err_code = RES_ERR;	                        //副走査のデータ数不足エラー
		return -1;
	}
	//黒補正値の計算
	for (x = pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_min; x < pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_max+1; ++x)
	{
		bc_val = 0.0f;

		for (i = cap->black_correction_s; i < cap->black_correction_e; ++i)//CHごと(1列ごと)に黒補正計算
		{
			spt.x = x;
			spt.y = i;                               
			P_Getdt_16bit_only(&spt, buf_num);                             //画素採取
			bc_val = bc_val + spt.sensdt;	                               //総和計算
		}

		cap->bc_val[x] = bc_val * bc_num_rev;		                       //平均計算（黒補正値
	}

	//----------------------------------------------
	// エリア抽出＆間引き＆黒補正計算＆オフセット計算
	//----------------------------------------------
	// 

	//ステップ幅の整合性
	if (pbs->PlaneInfo[UP_R_R].main_all_pix < cap->x_interval || 
		size_y * pbs->PlaneInfo[spt.p_plane_tbl].sub_sampling_pitch < cap->y_interval)
	{
		cap->err_code = RES_ERR;	                                        //ステップ幅が大きすぎる
		return -1;
	}

	if (cap->x_interval == 0 || cap->y_interval == 0)
	{
		cap->err_code = RES_ERR;	                                        //ステップ幅が0になっている。
		return -1;
	}
	
	for (area_i = 0; area_i < cap->reference_area_num; area_i++)
	{
		if (cap->p_reference_area[area_i].start_x < pver.left_up_x ||       //エリア指定の整合性
			pver.left_up_y < cap->p_reference_area[area_i].start_y ||
			pver.right_up_x < cap->p_reference_area[area_i].end_x  ||
			cap->p_reference_area[area_i].end_y < pver.left_down_y)
		{
			cap->err_code = RES_ERR;	                                    //参照エリアが紙幣エリアを超えている
			return -1;
		}

		for (y_i = cap->p_reference_area[area_i].start_y; y_i > cap->p_reference_area[area_i].end_y; y_i = y_i - cap->y_interval)			//ｙ
		{
			y_size++;

			for (x_i = cap->p_reference_area[area_i].start_x; x_i < cap->p_reference_area[area_i].end_x; x_i = x_i + cap->x_interval)		//x
			{
				x_size++;

				spt.x = x_i;
				spt.y = y_i;

				new_L_Getdt_16bit_only(&spt, pbs, pnote_param);		    //論理座標で画素採取

				tmp_num = spt.sensdt - cap->bc_val[spt.x] + OFFSET_VAL;		//黒補正とオフセット補正
																			//Spt.xには物理に変換されたx座標が格納されているので、テーブル引きに使う
				cap->input_data[input_data_counter] = tmp_num * dev_num;	//正規化して値を格納
				input_data_counter++;	                                    //要素数カウンター更新

				if (CAP_TAPE_MAX_ELEMENT_COUNT < input_data_counter)		//when input data counter is max element count or more
				{
					cap->err_code = RES_ERR;	                            //input data counter is exceeds max count 
					return -1;
				}
			}

			x_size = 0;
		}
	}

	//input_data_counter = 0;
	/////デバッグ用後で消す------------------------------------------
	//F_CREATE(fp);
	//for (y_i = pver.left_up_y; pver.left_down_y < y_i ; y_i = y_i - cap->y_interval)			//ｙ
	//{
	//	for (x_i = pver.left_up_x; x_i < pver.right_up_x; x_i = x_i + cap->x_interval)		//x
	//	{
	//		spt.x = x_i;
	//		spt.y = y_i;

	//		new_L_Getdt_16bit_only(&spt, pbs, pnote_param);		    //論理座標で画素採取

	//		tmp_num = spt.sensdt;// -cap->bc_val[spt.x] + OFFSET_VAL;		//黒補正とオフセット補正

	//		F_WRITE_LF(fp, tmp_num);

	//		cap->input_data[input_data_counter] = tmp_num;// * dev_num;	//正規化して値を格納
	//		input_data_counter++;	                                    //要素数カウンター更新
	//																	
	//	}

	//	F_N(fp);
	//}
	//F_CLOSE(fp);

	/////デバッグ用後で消す------------------------------------------



	//---------------------------------------
	// 第一閾値とサンプリングデータの差分値の計算
	// 差分値の積分の計算
	//---------------------------------------

	//要素数の整合確認
	if (input_data_counter != cap->first_thrs_num)
	{
		cap->err_code = RES_ERR;	                                                  //要素数がテンプレートデータと一致しない
		return -1;
	}

	cap->tape_total_area = 0;
	for (elem_i = 0; elem_i < input_data_counter; elem_i++)
	{
		//cap->diff_data[elem_i] = cap->input_data[elem_i] - cap->p_first_thrs[elem_i]; //画素値と第一閾値を計算
		cap->diff_data[elem_i] = cap->input_data[elem_i] - (cap->p_first_thrs[elem_i]); //画素値と第一閾値を計算
		if (cap->diff_data[elem_i] < 0)                                               //値が０未満の場合
		{
			cap->diff_data[elem_i] = 0;	                                              //クリアする
		}
		else
		{
			cap->tape_total_area += cap->diff_data[elem_i];							  //正の値のみ積分
		}
		//if (tmp_num > 0)                                               //値が０未満の場合
		//{
		//	cap->tape_total_area += tmp_num;
		//}
	
	}

	//---------------------------------------
	// 判定
	//---------------------------------------
	if (cap->tape_judg_thr <= cap->tape_total_area) //積分結果が閾値以上のとき
	//if (0.02 <= cap->tape_total_area) //積分結果が閾値以上のとき
	{
		cap->result = 1;                            //テープ有
		cap->level = 1;
		return 0;
	}
	else                                            //小さいとき
	{
		cap->result = 0;                            //テープ無し
		cap->level = 100;
		return 0;
	}

#if 0
	//イメージの横幅計算
	//size_x = pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_max - pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_min + 1;
	size_y = (u16)(pbs->block_count * (float)(pbs->Blocksize / pbs->PlaneInfo[spt.p_plane_tbl].sub_sampling_pitch));	

	if(size_y < cap->end_line)
	{
		//副走査のデータ数不足
		cap->err_code = RES_SUB_SCANLINE_SHORAGE;
		return -1;
	}

	bc_num_rev = 1.0f / (cap->black_correction_e - cap->black_correction_s);	//黒補正値のスキャンラインの逆数

	//特徴ベクトル抽出	決められた範囲の静電容量イメージを1次元の配列にする。 
	//MRXの場合0~16あるうちの　1 ~ 15しか使わない。
	//そのためminに+1、max-1している。
	for(x = pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_min+1; x < pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_max; ++x)
	{
		spt.x = x;
		bc_val = 0.0f;

		//CHごと(1列ごと)に黒補正計算
		//指定の画素をスキャンして平均を求める。
		for(i = cap->black_correction_s; i < cap->black_correction_e; ++i)
		{
			spt.y = i;
			P_Getdt_16bit_only(&spt, buf_num);
			bc_val = bc_val + spt.sensdt;	//総和計算
		}

		bc_val = bc_val * bc_num_rev;		//平均計算（黒補正値
		//bc_val = bc_val / (cap->black_correction_e - cap->black_correction_s);

		//データ抽出と黒補正
		for(y = cap->start_line; y < cap->end_line; ++y)
		{
			spt.y = y;

			P_Getdt_16bit_only(&spt, buf_num);							//画素採取
			cap->input_data[ii++] = spt.sensdt - bc_val + OFFSET_VAL;	//黒補正とオフセット補正

		}
	}

	input_data_num = ii;	//要素数を記録


	//抽出した特徴ベクトルの配列の並びを縦から横向きに変えるのと、正規化。
	y_length =  cap->end_line - cap->start_line;
	x_length = (pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_max) - (pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_min+1);
	ii = 0;
	a = 1.0f / DATA_SIZE;
	//抽出した特徴ベクトルの配列の並びを縦から横向きに変える。
	//for(y = 0; y < y_length; ++y)
	//{
	//	for(x = 0; x < x_length; ++x)
	//	{
	//		cap->input_data2[ii++] = cap->input_data[x * y_length + y] * a;	//黒補正とオフセット補正
	//	}
	//}

	////ノーマル-----------------------------------
	//for(x = 0; x < x_length; ++x)
	//{
	//	for(y = 0; y < y_length; ++y)
	//	{

	//		cap->input_data2[ii] = cap->input_data[ii] * a;	//黒補正とオフセット補正
	//		ii++;
	//	}
	//}
	////--------------------------------------------

	//移動平均-----------------------------------
	for(x = 0; x < x_length; ++x)
	{
		i_offset = x * y_length;	//iのオフセット計算
		sum = 0;					//合計値初期化

		//最初の移動平均を計算
		for(i = (s32)(i_offset - filter_fraction); i < (s32)(WINDOW_SIZE + i_offset - filter_fraction); ++i)
		{
			if(i_offset > i)
			{
				continue;
			}
			else
			{
				sum = sum + cap->input_data[i];
			}
		}

		cap->input_data2[ii] = sum * dev_num * a;	//平均と1~0に正規化
		ii++;

		for(i = i_offset - filter_fraction + 1; i < i_offset + y_length - filter_fraction; ++i)
		{
			if(i_offset <= i - 1)
			{
				sum = sum - cap->input_data[i - 1];
			}
			
			if(i_offset + y_length > i + WINDOW_SIZE - 1)
			{
				sum = sum + cap->input_data[i + WINDOW_SIZE - 1];
			}

			cap->input_data2[ii] = sum * dev_num * a;
			ii++;
		}
	}
	//--------------------------------------------


	if(input_data_num == 0)	//0除算防止
	{
		return -1;
	}
	
	////標準化	サンプリングデータごとにμ/σを求める
	//input_data_num_rev = 1.0f / input_data_num;	//要素数の逆数

	//for (i = 0; i < input_data_num; i++) 
	//{
	//	sum  = sum + cap->input_data[i];						//μ
	//	sum2 = sum2 + cap->input_data[i] * cap->input_data[i];	//σ
	//}

	////for (i = 0; i < input_data_num; i++) //高速化？
	////{
	////	sum2 = sum2 + cap->input_data[i] * cap->input_data[i];	//σ
	////}

 //   avg = sum * input_data_num_rev;
 //   dev = sqrt(sum2 * input_data_num_rev - avg * avg);

	//	//標準偏差の逆数
	//sigma_rev = 1.0f / dev;

	//for(i = 0; i < input_data_num; ++i)
	//{
	//	cap->input_data[i] = (cap->input_data[i] - avg) * sigma_rev;
	//}

	//if(dev == 0)	//0除算防止
	//{
	//	return -1;
	//}


	////正規化
	//a = 1.0f / 1024.0f;
	//for (i = 0; i < input_data_num; i++) 
	//{
	//	cap->input_data2[i] = cap->input_data2[i] * a;						//μ
	//	//cap->input_data[i] = cap->input_data[i] / 1024.0f;						//μ
	//	//sum2 = sum2 + cap->input_data[i] * cap->input_data[i];	//σ
	//}

	

	////sv計算
	////選択されたカーネル関数で計算する。
	//switch(cap->select_karnel)
	//{
	//case cap_rbf:
	//	//for(i = 0; i < cap->support_vecter_num; ++i)	//サポートベクトル数
	//	//{
	//	//	ary_num = i * cap->n;						//配列の番号計算

	//	//	for(ii = 0; ii < cap->n; ++ii)				//次元数ループ
	//	//	{
	//	//		num = cap->input_data[ii] - cap->p_support_vecter[ary_num + ii];	//サーポートベクトルとの差
	//	//		total = num * num + total;											//2乗の総和
	//	//	}

	//	//	karnel_res[i] = expf(total * -cap->gamma);	//-γと総和の乗算のexp
	//	//}

	//	////重み計算
	//	//for(i = 0; i < cap->waight_data_num; ++i)
	//	//{
	//	//	decision += karnel_res[i] * cap->p_waight_data[i];
	//	//}

	//	//decision += cap->bias;

	//	break;

	//case cap_sigmoid:

	//	break;


	//case cap_polynomial:

	//	break;
	//	

	//case cap_liner:

	//	//重み計算
	//	for(i = 0; i < input_data_num; ++i)
	//	{

	//		decision += cap->input_data[i] * cap->p_waight_data[i];
	//	}

	//	decision = decision + cap->bias;

	//	break;
	//}
	
	//重み計算
	for(i = 0; i < (s32)input_data_num; ++i)
	{
		decision += cap->input_data2[i] * cap->p_waight_data[i];
	}

	decision = decision + cap->bias;

	//出力の計算
	if(decision > 0)	//あり
	{
		cap->result = 1;
 		cap->level = 1;
		return 1;
	}
	else				//なし
	{
		cap->result = 0;
		cap->level = 100;
		return 0;
	}
#endif

}

////中央値を取得する関数
//float get_median(u16 ary[], s32 num)
//{
//    //最初に配列を並び替える
//	//maxソートで行った
//	u16 ary_num;
//    s32 damy = num;
//    s32 i;
//    s32 v;
//	s32 tmp = 0;
//
//    for(i = 0; i < num; ++i)			//要素数分ループ
//	{
//        for(v = 0; v < damy - 1; ++v)	//ここのループで
//		{								//小さい値を配列の後ろの方に持っていく
//            if(ary[v] < ary[v + 1])
//			{
//                tmp = ary[v + 1];
//                ary[v + 1] = ary[v];
//                ary[v] = tmp;
//            }
//        }
//
//        damy = damy - 1;				//後ろのn番目から
//										//アクセスする必要がなくなるのでデクリメント
//    }
//
//	ary_num = num * 0.5f;
//
//    return  (float)ary[ary_num];	//真ん中の値を返す
//}



//　END

