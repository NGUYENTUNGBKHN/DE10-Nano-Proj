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
* @file tape.c
* @brief テープ検知
* @date 2020/03/ Created.
* @updata
*/
/****************************************************************************/
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define EXT
#include "../common/global.h"

#define THICKNESS_SENSOR_NUM (32)

#define ONE_MICRO_METER		(1000)
//#define TAPE_OUTPUT 1

// const u16 level_thresh_tbl[] = { 50, 100, 200, 250, 300, 400, 500, 600, 800, 1000 };
const u16 level_thresh_tbl[] = { 50, 100, 150, 200, 250, 300, 400, 500, 800, 1000 };		// for TWD
const u8 fit_level_tbl[] = { MIN_LEVEL , 90, 80, 70, 60, 50, 40, 30, 20, 10, MAX_LEVEL };
const float roller_width = 13.5f;	                                                        // ローラーの幅(mm)

//各機種のCISの座標系におけるメカ厚が通る座標のリスト--------------
//BAU-LE17
const ST_THKNS_ROLLER_RANGE ThknsRangesLe17[] = {
	{ 89,129,156,196 },			 // Ch1
	{ 209,249,276,315 },		 // Ch2
	{ 329,368,396,435 },		 // Ch3
	{ 449,488,515,555 },		 // Ch4
	{ 568,608,635,675 },		 // Ch5
	{ 688,727,755,794 },		 // Ch6
	{ 808,847,875,914 },		 // Ch7
	{ 927,967,994,1034 },		 // Ch8
	{ 1047,1086,1114,1153 },	 // Ch9
	{ 1167,1206,1234,1273 },	 // Ch10
	{ 1286,1326,1353,1393 },	 // Ch11
	{ 1406,1445,1473,1512 }		 // Ch12
};
//MRX-CIS
const ST_THKNS_ROLLER_RANGE ThknsRangesMrx[] = {
	{ 370,  391,  391,  412 }	// Ch1
};
//テンプレ
const ST_THKNS_ROLLER_RANGE ThknsRanges[] = {
	{ 0,  1,  1,  2 }			// Ch1

};
//----------------------------------------------------------------

//メカ厚式テープ検知のメイン関数
s16 tape(u8 buf_num, ST_TAPE * st)
{
	u8 ret=0;
	int i = 0;
	int j = 0;
	u16 tape_count = 0;
	float tape_area = 0.0f;
	u16 tape_area_integer = 0;
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	u8 bill_size_x = st->bill_size_x;
	u8 bill_size_y = st->bill_size_y;
	u8 mesh_size_x = st->mesh_size_x;
	u8 mesh_size_y = st->mesh_size_y;

	u16* plearn_map;
	u16 map_size_x = 0;
	u16 map_size_y = 0;

	u16 pre_count = 0;
	u8 moving_average = st->moving_average;

	ST_THKNS_CH_DATA_AVE learn_ch_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };
	u32 thkns_compared_ch_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH]         = { 0 };
	u32 thkns_ch_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH]                  = { 0 };
	u32 ptemp_map[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH]                      = { 0 };

	u8 ch_tape_sum = 0;
	ST_THKNS_ROLLER_RANGE* proller_range=0;
	u16 bill_top_edge[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	u16 bill_lenght_y=0;

	u16 sensor_count = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_all_pix - pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min;
	u16 height       = pbs->block_count / (pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch / pbs->Blocksize);

	////モード切替
	u8 bc_mode = st->black_corr;
	u8 tc_corr_mode = st->tc1_tc2_corr;

	///-------------------------------------------------------------
#ifdef TAPE_OUTPUT
	FILE* fp;
	char str1[20];
	char str3[30];
	char str5[30];
	char str7[30];
	char str4[10];
	char str6[10];
	//char* str2 = ".bin";
	char* str2 = " c.csv";
	sprintf(str4, "%d", pbs->insertion_direction);
	sprintf(str1, "%s", pbs->ser_num1);
	sprintf(str6, "%d", bc_mode);

	sprintf(str3, "%s %s", str1, &str4);
	sprintf(str5, "%s %s", str3, &str6);
	sprintf(str7, "%s %s", str5, str2);
	fp = fopen(str7, "ab");
#endif
	///-------------------------------------------------------------

	// MAPサイズ取得
	calc_map_size(bill_size_x, bill_size_y, mesh_size_x, mesh_size_y, &map_size_x, &map_size_y);
	
	// センサーデータ取得　TC1-TC2と、黒補正を実行している →　thkns_ch_dataに値を入れる。thkns_ch_dataは物理座標系
	ret = get_thkns_data(pbs, thkns_ch_data, height, bc_mode, tc_corr_mode);

	///-------------------------------------------------------------
#ifdef TAPE_OUTPUT
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_all_pix - pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min; ++j)
		{
			F_WRITE(fp, thkns_ch_data[j][i])
		}
		F_N(fp)		//改行
	}
	F_CLOSE(fp)	//クローズ
#endif
	///-------------------------------------------------------------

	if (ret == 0)
	{
		// 厚みセンサCh毎絶対座標取得
		ret = get_roller_range(pbs, &proller_range);

		// 紙幣先端後端検出
		calculate_billedge(pbs, proller_range, sensor_count, bill_top_edge, &bill_lenght_y);

		// フィルタ処理
		if (moving_average == 1)
		{
			//ptemp_map = (u16*)malloc_short(sizeof(u16) * map_size_x * map_size_y);
			//移動平均を計算する→ptemp_map
			calc_moving_average(thkns_ch_data, ptemp_map, sensor_count, height, st, bill_top_edge, bill_lenght_y);
			//ptemp_map→thkns_ch_dataへコピー
			memcpy(thkns_ch_data, ptemp_map, (sizeof(u32) * THICKNESS_SENSOR_MAX_NUM * THHKNS_CH_LENGTH));
		}
		else
		{
			memcpy(ptemp_map, thkns_ch_data, (sizeof(u32) * THICKNESS_SENSOR_MAX_NUM * THHKNS_CH_LENGTH));

		}

		// 厚みセンサCh毎のデータに変換
		plearn_map = st->plearn_data;

		// 厚みセンサデータを学習データ最小値と比較	
		// 最小値より大きかったポイント数をカウント→pre_count
		pre_count = pre_judge_sensor_data(ptemp_map, sensor_count, height, st->base_value + st->threshold);

		if (pre_count < st->judge_count)	//上限値未満の場合
		{	
			// 厚みセンサデータが”紙幣厚み最小値”を超えているポイントの論理座標を求め、紙幣学習MAP上のデータを取得する
			//　厚みセンサデータが”紙幣厚み最小値”を超えているポイントの論理座標を求め、紙幣学習MAP上のデータ(平均または、最大値)を取得する。→learn_ch_data
			convert_thickness_sensor_data(pbs, pnote_param, plearn_map, map_size_x, map_size_y, st, proller_range, learn_ch_data, ptemp_map, sensor_count, height);

			// 紙幣学習MAPと比較して、閾値を越える連続データの個数をカウントする→tape_count
			// テープが連続している部分の入力データを計算する。→thkns_compared_ch_data
			tape_count = compare_thickness_sensor_data(sensor_count, thkns_ch_data, learn_ch_data, thkns_compared_ch_data, st->threshold, st->continuous_judge);
		}
		else								//上限値以上の場合は
		{
			tape_count = pre_count;			//テープ有と判断する。
		}

	}

	// 結果出力
	st->err_code = 0;

	// 検知結果
	if (tape_count > 0)		//連続データ数が1以上の時
	{
		st->result = 1;		//あり
	}
	else					//連続データが無いとき
	{
		st->result = 0;		//なし
	}

	st->detect_num = tape_count;	// テープ検知数

	tape_area = 0.0f;
	//テープの面積を計算する。
	if (pre_count < st->judge_count)
	{
		for (i = 0; i < THICKNESS_SENSOR_MAX_NUM; i++)		// 各圧検センサ毎のテープ判定箇所数
		{
			ch_tape_sum = 0;

			if (i < sensor_count)
			{
				for (j = 0; j < THHKNS_CH_LENGTH; j++)
				{
					if (thkns_compared_ch_data[i][j] > 0)
					{
						ch_tape_sum += 1;					//連続データのポイント数をカウント
					}
				}
			}
			st->ch_tape_num[i] = ch_tape_sum;
			tape_area += ch_tape_sum * roller_width;		//ポイント数×センサの幅
		}
	}
	else
	{
		tape_area = pre_count * roller_width;
	}

	// レベル設定
	tape_area_integer = (u16)(tape_area+0.5f);
	if (tape_count > 0)		// レベル
	{
		u8 thresh_tbl_size = sizeof(level_thresh_tbl) / sizeof(level_thresh_tbl[0]);
		if (tape_area_integer < level_thresh_tbl[0])	//面積の最小閾値より小さければ
		{
			st->level = MIN_LEVEL;						//レベル100
		}
		else if ((level_thresh_tbl[thresh_tbl_size - 1] <= tape_area_integer))	//面積の最大閾値より大きければ
		{
			st->level = MAX_LEVEL;						//レベル1
		}
		else
		{
			for (i = 0; i < thresh_tbl_size - 1; i++)	//レベル10~90の判定
			{
				if ((level_thresh_tbl[i] <= tape_area_integer) && (tape_area_integer < level_thresh_tbl[i + 1]))
				{
					st->level = fit_level_tbl[i + 1];
					break;
				}
			}
		}
	}
	else
	{
		st->level = MIN_LEVEL;							//テープが無ければレベル100
	}

#ifndef _RM_
	{	//ビューアツール用パラメタの設定
		int data_length = sensor_count * THHKNS_CH_LENGTH;
		int ch, x;

		u32 learn_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };

		for (ch = 0; ch < sensor_count; ch++)
		{
			for (x = 0; x < THHKNS_CH_LENGTH; x++)
			{
				learn_data[ch][x] = learn_ch_data[ch][x].chdata;
			}
		}
		if (height > THHKNS_CH_LENGTH)
		{
			height = THHKNS_CH_LENGTH;
		}

		//コールバック関数　デバッグ情報
		//				　移動平均後　　　　　 データ量	　　学習MAP　　　　　データ量　　  テープ連続データ情報　      　データ量　　 256               高さ　　テープの数　閾値以下のポイント数　レベル
		callback_tape_len((u32*)thkns_ch_data, data_length, (u32*)learn_data, data_length, (u32*)thkns_compared_ch_data, data_length, THHKNS_CH_LENGTH, height, tape_count, pre_count, st->level);
	}
#endif

	return 0;
}

/****************************************************************/
/**
* @brief		MAPのサイズを計算する
*@param[in]		bill_size_x	x方向の紙幣サイズ
*				bill_size_y	y方向の紙幣サイズ
*				mesh_size_y	メッシュサイズ
*
*@param[out]	pmap_size_x	マップサイズ
*				pmap_size_y	マップサイズ
*
* @return		なし
*/
/****************************************************************/
void calc_map_size(u8 bill_size_x, u8 bill_size_y, u8 mesh_size_x, u8 mesh_size_y, u16* pmap_size_x, u16* pmap_size_y)
{
	u16 map_size_x, map_size_y;

	u32 dot_count_x = (bill_size_x * 1000) / DOT_SIZE_MM;		/* 1dot = 0.127mm */
	u32 dot_count_y = (bill_size_y * 1000) / DOT_SIZE_MM;

	dot_count_x = EVEN_NUM(dot_count_x);
	dot_count_y = EVEN_NUM(dot_count_y);

	map_size_x = (u16)(dot_count_x / mesh_size_x);
	if ((dot_count_x % mesh_size_x) != 0)
	{
		map_size_x += 1;
	}
	map_size_y = (u16)(dot_count_y / mesh_size_y);
	if ((dot_count_y % mesh_size_y) != 0)
	{
		map_size_y += 1;
	}

	map_size_x = EVEN_NUM(map_size_x);
	map_size_y = EVEN_NUM(map_size_y);

	*pmap_size_x = map_size_x;
	*pmap_size_y = map_size_y;

	return;
}

/****************************************************************/
/**
* @brief		移動平均を計算する条件の確認。
*@param[in]		input_data		採取した画素値
*				ch_num			Ch数
*				data_count		Y方向の画素ライン数
*				st				テープ検知専用構造体
*				pbill_top_edge	先端座標候補
*				bill_lengh_y	後端座標
*
*@param[out]	out_data		移動平均を計算した結果
*
* @return		なし
*/
/****************************************************************/
void calc_moving_average(u32 input_data[][THHKNS_CH_LENGTH], u32 out_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, ST_TAPE *st, u16* pbill_top_edge, u16 bill_lengh_y)
{
	s32 calc_start = 0;
	s32 calc_end = 0;

	u8 moving_ave_value_a = st->moving_ave_value_a;
	u8 divide_ab          = st->divide_ab;
	u16 divide_line_ab    = st->divide_line_ab;
	u8 moving_ave_value_b = st->moving_ave_value_b;
	u8 divide_bc          = st->divide_bc;
	u16 divide_line_bc    = st->divide_line_bc;
	u8 moving_ave_value_c = st->moving_ave_value_c;

	if ((divide_ab == 0) && (divide_bc == 0))
	{
		calc_start = 0;
		calc_end   = bill_lengh_y;
		moving_average_sub(input_data, out_data, ch_num, data_count, moving_ave_value_a, calc_start, calc_end, pbill_top_edge);
	}
	else if (divide_ab == 1)
	{
		// 0-ab
		calc_start = 0;
		calc_end = divide_line_ab;
		moving_average_sub(input_data, out_data, ch_num, data_count, moving_ave_value_a, calc_start, calc_end, pbill_top_edge);

		if (divide_bc == 0)
		{
			// ab-y_max
			calc_start = divide_line_ab;
			calc_end = bill_lengh_y;
			moving_average_sub(input_data, out_data, ch_num, data_count, moving_ave_value_b, calc_start, calc_end, pbill_top_edge);
		}
		else
		{
			// ab-bc
			calc_start = divide_line_ab;
			calc_end = divide_line_bc;
			moving_average_sub(input_data, out_data, ch_num, data_count, moving_ave_value_b, calc_start, calc_end, pbill_top_edge);

			// bc-y_max
			calc_start = divide_line_bc;
			calc_end = bill_lengh_y;
			moving_average_sub(input_data, out_data, ch_num, data_count, moving_ave_value_c, calc_start, calc_end, pbill_top_edge);
		}
	}
	else
	{
		// 0-bc
		calc_start = 0;
		calc_end = divide_line_bc;
		moving_average_sub(input_data, out_data, ch_num, data_count, moving_ave_value_b, calc_start, calc_end, pbill_top_edge);

		// bc-y_max
		calc_start = divide_line_bc;
		calc_end = bill_lengh_y;
		moving_average_sub(input_data, out_data, ch_num, data_count, moving_ave_value_c, calc_start, calc_end, pbill_top_edge);
	}
}

/****************************************************************/
/**
* @brief		移動平均を計算する。
*@param[in]		input_data		 採取した画素値
*				ch_num			 Ch数
*				data_count		 Y方向の画素ライン数
*				moving_ave_value 移動平均の窓サイズ
*				start			 開始座標
*				end				 終了座標
*				pbill_top_edge	 先端座標
*
*@param[out]	out_data		移動平均を計算した結果
*
* @return		なし
*/
/****************************************************************/
void moving_average_sub(u32 input_data[][THHKNS_CH_LENGTH], u32 out_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, u8 moving_ave_value, int start, int end, u16* pbill_top_edge)
{
	int x = 0;
	int y;
	int kk;
	u32 sum = 0;
	int pt;
	int diff;
	u8 out_range;
	u16 start_offset = 0;

	if (moving_ave_value == 0)
	{
		moving_ave_value = 1;
	}
	for (x = 0; x < ch_num; x++)
	{
		start_offset = pbill_top_edge[x];
		for (y = start+ start_offset; y < end + start_offset; y++)	//先端座標+開始座標の位置からスキャンを始める。
		{
			sum = 0;
			out_range = 0;
			diff = ((moving_ave_value - 1) / 2);

			for (kk = 0; kk < moving_ave_value; kk++)
			{
				pt = y - diff + kk;
				if ((pt < 0) || (pt >= data_count))
				{
					out_range++;
				}
				else
				{
					sum += input_data[x][pt];
				}
			}
			if (moving_ave_value == out_range)
			{
				out_data[x][y] = 0;
			}
			else
			{
				out_data[x][y] = (u16)(sum / (moving_ave_value - out_range));
			}
		}
	}
}

/****************************************************************/
/**
* @brief		厚みセンサデータが”紙幣厚み最小値”を超えているポイントの論理座標を求め、紙幣学習MAP上のデータ(平均または、最大値)を取得する。
*				
*@param[in]		pbs				センサーデータ
*				pnote_param		座標変換に用いるパラメタ
*				learn_map_data	学習で得られたMAPデータ
*				map_size_x		MAPデータのサイズ
*				map_size_y		MAPデータのサイズ
*				st				テープ検知専用構造体
				proller_range	CIS座標系で厚みセンサがトレースする座標
				thkns_ch_data	センサーデータ(移動平均とマスク処理後)
				array_size		チャンネル数
				height			Y方向有効画素範囲
*
*@param[out]	plearn_ch_data	閾値以上だった各CHの各ラインの平均値または、最大値
*
* @return		なし
*/
/****************************************************************/
void convert_thickness_sensor_data(ST_BS * pbs, ST_NOTE_PARAMETER * pnote_param, u16 * learn_map_data, u16 map_size_x, u16 map_size_y, ST_TAPE * st, ST_THKNS_ROLLER_RANGE * proller_range, ST_THKNS_CH_DATA_AVE plearn_ch_data[][THHKNS_CH_LENGTH], u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 array_size, u16 height)
{
	int ch, x, y, map_y;
	u32 learn_data = 0;

	u8 insertion_direction = pbs->insertion_direction;
	u8 sub_sampling_pitch = pbs->PlaneInfo[UP_TC1].sub_sampling_pitch;
	s8 sub_offset = pbs->PlaneInfo[UP_TC1].sub_offset / sub_sampling_pitch;
	u16 map_point_x = 0;
	u16 map_point_y = 0;
	u8 threshold_type = st->threshold_type;					// 閾値種別

	u8 mesh_size_x = st->mesh_size_x;
	u8 mesh_size_y = st->mesh_size_y;
	float div_mesh_size_x = 1.0f / mesh_size_x;
	float div_mesh_size_y = 1.0f / mesh_size_y;
	u16 bill_size_x = map_size_x * mesh_size_x;
	u16 bill_size_y = map_size_y * mesh_size_y;
	s16 half_bill_size_x = (bill_size_x / 2);
	s16 half_bill_size_y = (bill_size_y / 2);

	s16 point_x_min = -half_bill_size_x;
	s16 point_x_max = half_bill_size_x;
	s16 point_y_min = -half_bill_size_y;
	s16 point_y_max = half_bill_size_y;

	s16 offset_x = 0;
	s16 offset_y = 0;
	int mesh_skip_x;
	int mesh_skip_y;

	u8 adjust = 0;	//座標ズレ（計算誤差）を補正する。

	s32 point_x = 0;
	s32 point_y = 0;
	u8 plane_tbl = (enum P_Plane_tbl)UP_R_R;
	
	if (pbs->LEorSE == LE)  //SEならば
	{
		point_x_min += (st->exclude_mesh_left*sub_sampling_pitch);
		point_x_max -= (st->exclude_mesh_right*sub_sampling_pitch);
		point_y_min += (st->exclude_mesh_bottom*sub_sampling_pitch);
		point_y_max -= (st->exclude_mesh_top*sub_sampling_pitch);
	}
	else
	{
		adjust = 1;
		point_x_min += (st->exclude_mesh_bottom*sub_sampling_pitch);
		point_x_max -= (st->exclude_mesh_top*sub_sampling_pitch);
		point_y_min += (st->exclude_mesh_left*sub_sampling_pitch);
		point_y_max -= (st->exclude_mesh_right*sub_sampling_pitch); 
	}

	offset_x = pnote_param->temp_center_x[plane_tbl] - pnote_param->main_offset[plane_tbl];
	offset_y = pbs->center_y - pbs->PlaneInfo[plane_tbl].sub_offset;
#if 1
	mesh_skip_x = st->mesh_skip_x;
	mesh_skip_y = st->mesh_skip_y;

	if (mesh_skip_x == 0)
	{
		mesh_skip_x = 1;
	}
	if (mesh_skip_y == 0)
	{
		mesh_skip_y = 1;
	}
#endif
	//座標変換を行う。
	for (ch = 0; ch < array_size; ch++)			//ch毎に確認、1ch目から
	{
		for (y = 0; y < height; y++)			//物理座標系でのy方向の有効画素範囲
		{
			plearn_ch_data[ch][y].chdata = 0;
			plearn_ch_data[ch][y].ave_count = 0;

			if (thkns_ch_data[ch][y] > 0)		//センサーデータがマスクされているかの確認
			{
				for (map_y = 0; map_y < sub_sampling_pitch; map_y = map_y + mesh_skip_y)	//
				{
					
					for (x = proller_range[ch].roller_a_left_pos; x <= proller_range[ch].roller_b_right_pos; x = x + mesh_skip_x)	//CIS座標系で厚みセンサがトレースする座標をセット
					{
						// ローラーA,Bの間を除外
						if ((proller_range[ch].roller_a_right_pos < x) && (x < proller_range[ch].roller_b_left_pos))
						{
							continue;
						}

						point_x = x;
						point_y = (y * sub_sampling_pitch) + map_y + sub_offset;

						//{
						//	//s16 new_P2L_Coordinate(ST_SPOINT *__restrict spt, ST_BS * __restrict pbs, ST_NOTE_PARAMETER * __restrict pnote_param, s16 offset_x, s16 offset_y)
						//	{
						
						//座標変換を行う
						//ｘ座標スキャン方向補正
						if (pbs->PlaneInfo[plane_tbl].note_scan_dir == R2L)
						{
							//ｘ座標反転
							point_x = pnote_param->main_eff_range - point_x;
						}

						point_x = point_x - offset_x;
						point_y = offset_y - point_y;

						//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
						if (pbs->LEorSE == SE)  //SEならば
						{
							s32 temp;

							temp = point_y;
							point_y = -point_x;
							point_x = temp;
						}

						// 傾き補正
						{
							float xx, yy;

							//	xx = (point_y* ang.cos_th + point_x * ang.sin_th);
							//yy = (point_x * ang.cos_th - point_y * ang.sin_th);
							xx = (point_y * pnote_param->cos_y[plane_tbl] + point_x * pnote_param->sin_x[plane_tbl]) + 0.5f;
							yy = (point_x * pnote_param->cos_x[plane_tbl] - point_y * pnote_param->sin_y[plane_tbl]) + 0.5f;

							point_y = (s32)xx;
							point_x = (s32)yy;
						}
						//	}
						//}

						//トレース座標を変換した結果、紙幣の座標内か？
						if (((point_x_min) <= point_x) && (point_x <= point_x_max) && (point_y_min <= point_y) && (point_y <= point_y_max))
						{
							map_point_x = (u16)((point_x + half_bill_size_x) * div_mesh_size_x + 0.5f);
							map_point_y = (u16)((half_bill_size_y - point_y) * div_mesh_size_y + 0.5f);
							if ((map_size_x < map_point_x) || (map_size_y < map_point_y))
							{
								continue;
							}
	
							// 方向合わせ
							if (insertion_direction == W1)		// B方向(左右、上下反転)
							{
								map_point_x = (map_size_x - 1) - map_point_x;
								map_point_y = (map_size_y - 1) - (map_point_y + adjust);
							}
							else if (insertion_direction == W2)	// C方向(上下反転)
							{
								map_point_y = (map_size_y - 1) - (map_point_y + adjust);
							}
							else if (insertion_direction == W3)	// D方向(左右反転)
							{
								map_point_x = (map_size_x-1) - map_point_x;
							}

							//処理中の座標における、学習MAPデータを参照する。
							learn_data = (u16)learn_map_data[(map_point_y * map_size_x) + map_point_x];

							//処理系に応じて場合分け
							if (threshold_type == 2)		// 2:最大
							{
								if (learn_data > plearn_ch_data[ch][y].chdata)
								{
									plearn_ch_data[ch][y].chdata = learn_data;
								}
							}
							else							// 1:平均 
							{
								if (learn_data != 0)
								{
									plearn_ch_data[ch][y].chdata += learn_data;
									plearn_ch_data[ch][y].ave_count += 1;
								}
							}
						}
					}
				}

				//平均の場合はここで計算
				if (plearn_ch_data[ch][y].ave_count > 0)
				{
					plearn_ch_data[ch][y].chdata = plearn_ch_data[ch][y].chdata / plearn_ch_data[ch][y].ave_count;
				}
			}
		}
	}
}

/****************************************************************/
/**
* @brief		厚みセンサデータと、紙幣学習MAPデータを比較する
*
*@param[in]		array_size			チャンネル数
*				pinput_ch_data		各CHの各ラインの平均値または、最大値
*				plearn_ch_data		紙幣学習MAPデータ
*				threshold			閾値
*				continuous_judge	連続データのサイズ閾値
*
*@param[out]	compared_ch_data	連続データが存在した場合、その部分の情報を記録する。
*
* @return		連続データの数
*/
/****************************************************************/
u16 compare_thickness_sensor_data(int array_size, u32 pinput_ch_data[][THHKNS_CH_LENGTH], ST_THKNS_CH_DATA_AVE plearn_ch_data[][THHKNS_CH_LENGTH], u32 compared_ch_data[][THHKNS_CH_LENGTH], u16 threshold, u8 continuous_judge)
{
	int ch, x, i;
	int start_idx = 0;
	u16 result = 0;
	u8 continuous = 0;
	for (ch = 0; ch < array_size; ch++)
	{
		for (x = 0; x < THHKNS_CH_LENGTH; x++)
		{
			//紙幣学習MAPが0より大きいかつ、センサーデータの値が（紙幣学習MAP+閾値）より大きいとき
			if ((plearn_ch_data[ch][x].chdata >0) && (pinput_ch_data[ch][x] > plearn_ch_data[ch][x].chdata + threshold))
			{
				compared_ch_data[ch][x] = 0;
				continuous = 1;
				start_idx = x;
				// 連続性チェック
				for (i = start_idx + 1; i < THHKNS_CH_LENGTH; i++)
				{
					compared_ch_data[ch][i] = 0;
					if ((plearn_ch_data[ch][i].chdata > 0) && (pinput_ch_data[ch][i] > plearn_ch_data[ch][i].chdata + threshold))
					{
						continuous++;
					}
					else
					{
						x = i;		// インデックスを進める
						break;
					}
				}

				// 連続している場合、データ設定
				if (continuous >= continuous_judge)	// 連続データが閾値以上の場合
				{
					for (i = start_idx; i < start_idx + continuous; i++)
					{
						//差分値をその座標に記録する。
						compared_ch_data[ch][i] = pinput_ch_data[ch][i] - (plearn_ch_data[ch][i].chdata + threshold);
						result++;	//連続データ数をカウント
					}
				}
			}
			else
			{
				compared_ch_data[ch][x] = 0;
			}
		}
	}
	return result;
}

/****************************************************************/
/**
* @brief		厚みセンサの画素値を取得する
*				TC1-TC2と、黒補正を実行している。
*				
* 
*@param[in]		pbs				センサーデータのポインタ
*				height			y方向の有効画素範囲
* 
*@param[out]	thkns_ch_data	厚みセンサの画素値
*
* @return		なし
*/
/****************************************************************/
u8 get_thkns_data(ST_BS * pbs, u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 height, u8 bc_mode, u8 tc_corr_mode)
{
	u8 ret = 0;
	u16 sens_dt_tc1 = 0;
	u16 sens_dt_tc2 = 0;
	u32 period = 0;
	u32 offset_tc1 = 0;
	u32 offset_tc2 = 0;
	u16 y = 0;
	u16 x = 0;
	u8 data_type = 0;
	u16 sensor_count = 0;
	u16 sensor_ofs = 0;
	u16 tc1_idx_x = 0;
	//u16 dpi_adjust;

	//黒補正用パラメータ
	s16 bc_s = 1;
	s16 bc_e = 0;
	float tmp_bc_val = 0.0;
	float bc_val[20];
	s32 cres = 0;
	u16 pre_val;
	u16	val;
	u8 bc_num;
	s32 bc_offset = 0;

	sensor_ofs   = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min;
	sensor_count = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_all_pix - pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min + 1;
	period       = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].Address_Period;
	offset_tc1   = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].Address_Offset;
	offset_tc2   = pbs->PlaneInfo[THICKNESS_SENSOR_NUM+1].Address_Offset;
	data_type    = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].data_type;
	
	//dpi_adjust = (u16)((pbs->Subblock_dpi * pbs->pitch_ratio) / (pbs->Subblock_dpi / pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch));

	if (bc_mode == 1)		//－を表現するために+20000
	{
		bc_offset = 20000;
	}

	if (tc_corr_mode == 1)	//tc1-tc2 あり
	{
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < sensor_count; x++)
			{
				tc1_idx_x = x + sensor_ofs;
				if (data_type > 8)
				{
					/*2バイト分のデータを取り出します*/
					memcpy(&sens_dt_tc1, &pbs->sens_dt[(u32)y * period + (u32)(tc1_idx_x * 2) + offset_tc1], 2);
					memcpy(&sens_dt_tc2, &pbs->sens_dt[(u32)y * period + (u32)(x * 2) + offset_tc2], 2);
				}
				//通常
				else
				{
					sens_dt_tc1 = (s16)pbs->sens_dt[(u32)y * period + (u32)tc1_idx_x + offset_tc1];
					sens_dt_tc2 = (s16)pbs->sens_dt[(u32)y * period + (u32)x + offset_tc2];
				}

				if (sens_dt_tc1 > sens_dt_tc2 && bc_mode == 0)	//Tc1-Tc2
				{
					thkns_ch_data[x][y] = (sens_dt_tc1 - sens_dt_tc2) * 100;
				}
				else if (bc_mode == 1)
				{
					thkns_ch_data[x][y] = ((s32)(sens_dt_tc1 - sens_dt_tc2) * 100) + bc_offset;

					if (x == 7)
					{
						thkns_ch_data[x][y] = ((s32)(sens_dt_tc1 - sens_dt_tc2) * 100) + bc_offset;
					}
				}


			}
		}
	}
	else					//tc1-tc2 なし
	{
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < sensor_count; x++)
			{
				tc1_idx_x = x + sensor_ofs;
				if (data_type > 8)
				{
					/*2バイト分のデータを取り出します*/
					memcpy(&sens_dt_tc1, &pbs->sens_dt[(u32)y * period + (u32)(tc1_idx_x * 2) + offset_tc1], 2);
				}
				//通常
				else
				{
					sens_dt_tc1 = (s16)pbs->sens_dt[(u32)y * period + (u32)tc1_idx_x + offset_tc1];
				}

				thkns_ch_data[x][y] = sens_dt_tc1;
			}
		}
	}


	//////各チャネルの黒補正を行います。	add by furuta 21/1/18
	if (bc_mode == 1)	//　黒補正　あり
	{
		////紙幣の先端座標を求める
		if (pbs->left_up_y < pbs->right_up_y)
		{
			bc_e = pbs->left_up_y / pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch - 1;
		}
		else
		{
			bc_e = pbs->right_up_y / pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch - 1;
		}

		//エラーチェック
		if (bc_e <= bc_s)
		{
			return ret;
		}

		////各チャネルの補正値を求める

		//各チャネルの補正値の計算
		for (x = 0; x < sensor_count; x++)
		{
			tmp_bc_val = 0.0f;
			bc_num = 0;
			pre_val = 0;

			for (y = bc_s; y < bc_e; y++)
			{
				val = (u16)thkns_ch_data[x][y];

				//急激な変化の場合は、ノイズと思われるので計算に含まない
				if ((pre_val + ONE_MICRO_METER >= val && pre_val - ONE_MICRO_METER <= val) && y != bc_s)
				{
					tmp_bc_val = tmp_bc_val + val;	//総和計算
					bc_num++;
				}

				pre_val = val;					//１つ前の値を保存
			}

			if (bc_num == 0)
			{
				bc_num = 1;
			}

			bc_val[x] = tmp_bc_val / bc_num;		        //平均計算（黒補正値
		}


		////各チャネルを補正する
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < sensor_count; x++)
			{
				cres = (s32)(thkns_ch_data[x][y] - bc_val[x]);	//補正
				if (cres < 0)
				{
					thkns_ch_data[x][y] = 0;
				}
				else
				{
					thkns_ch_data[x][y] = (u32)cres;
				}
			}
		}
	}

	return ret;
}

/****************************************************************/
/**
* @brief		各機種のCISの座標系におけるメカ厚が通る座標を取得する。
*@param[in]		pbs				サンプリングデータのポインタ
*
*@param[out]	proller_range	座標リスト
*
* @return		1：対応機種　0：非対応機種
*/
/****************************************************************/
u8 get_roller_range(ST_BS * pbs, ST_THKNS_ROLLER_RANGE** proller_range)
{
	u8 ret = 0;
	if (pbs->model_code[0] == 0x34		//機種番号照合
		&& pbs->model_code[1] == 0x31
		&& pbs->model_code[2] == 0x32
		&& pbs->model_code[3] == 0x30)
	{
		*proller_range = (ST_THKNS_ROLLER_RANGE*)&ThknsRangesLe17;
		ret = 1;
	}
	else if (pbs->model_code[0] == 0x34	//機種番号照合
		&& pbs->model_code[1] == 0x31
		&& pbs->model_code[2] == 0x32
		&& pbs->model_code[3] == 0x31)
	{
		*proller_range = (ST_THKNS_ROLLER_RANGE*)&ThknsRangesMrx;
		ret = 1;
	}
	else
	{
		*proller_range = (ST_THKNS_ROLLER_RANGE*)&ThknsRanges;
	}

	return ret;
}

/****************************************************************/
/**
*@brief			閾値と比較してセンサーデータ配列をマスクする。
*				また、閾値より大きかったポイント数をカウントする。
*@param[in]		thkns_ch_data	センサーデータ(移動平均)
*				ch_num			チャンネル数
*				data_count		Y方向の画素ライン数
*				threshold		閾値(学習で求めた最小値+マージン値)
*
*@param[out]	count			閾値を上回ったポイント数
*
* @return		なし
*/
/****************************************************************/
u16 pre_judge_sensor_data(u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, u16 threshold)
{
	u16 count = 0;
	int i, j;

	for (i = 0; i < ch_num; i++)
	{
		for (j = 0; j < data_count; j++)
		{
			if (thkns_ch_data[i][j] < threshold)	// 閾値と画素値の比較して
			{                                       // 閾値以下ならば、
				thkns_ch_data[i][j] = 0;            // マスクする。
			}                                       // 
			else                                    // 閾値より大きい場合は、
			{                                       // そのポイントをカウントする。
				count++;                            // 
			}                                       // 
		}
	}

	return count;
}

/****************************************************************/
/**
* @brief		紙幣の先端、後端を検知する。
*@param[in]		pbs				サンプリングデータ
*				proller_range	座標リスト
*				ch_num			Ch数
*
*@param[out]	pbill_top_edge	先端座標候補
*				pbill_lengh_y	後端座標
*
* @return		なし
*/
/****************************************************************/
u8 calculate_billedge(ST_BS* pbs, ST_THKNS_ROLLER_RANGE* proller_range, u16 ch_num, u16* pbill_top_edge, u16* pbill_lengh_y)
{
	u8 ret = 0;
	int ch_count = 0;
	s16 left_up_x = pbs->left_up_x;              //頂点左上ｘ
	s16 left_up_y = pbs->left_up_y;              //頂点左上ｙ
	s16 left_down_x = pbs->left_down_x;          //頂点左下ｘ
	s16 left_down_y = pbs->left_down_y;          //頂点左下ｙ
	s16 right_up_x = pbs->right_up_x;            //頂点右上ｘ
	s16 right_up_y = pbs->right_up_y;            //頂点右上ｙ
	s16 right_down_x = pbs->right_down_x;        //頂点右下ｘ
	//s16 right_down_y = pbs->right_down_y;      //頂点右下ｙ

	u8 sub_sampling_pitch = pbs->PlaneInfo[UP_TC1].sub_sampling_pitch;
	s8 sub_offset = pbs->PlaneInfo[UP_TC1].sub_offset / sub_sampling_pitch;

	u16 sensorLeft_X = 0;
	u16 sensorRight_X =0;
	s16 billTopEdge =0;
	//s16 billEndEdge;
	int diff_x =0;
	int diff_y=0;
	s16 lenght_y =0;
	float tan = 0.0f;

	s16 top_edge[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	//s16	end_edge[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	s16 bill_top_edge = 0;

	if (pbs->PlaneInfo[UP_TC1].note_scan_dir == R2L)
	{
		tan = -pbs->angle / 4096.0f;
	}
	else
	{
		tan = pbs->angle / 4096.0f;
	}

	// 後端検出
	if (left_up_x > left_down_x)
	{
		diff_x = left_up_x - left_down_x;
		diff_y = (int)(diff_x * tan);
		lenght_y = (s16)((left_down_y - left_up_y) - diff_y);	//ｙ座標の範囲
	}
	else
	{
		diff_x = left_down_x - left_up_x;
		diff_y = (int)(diff_x * tan);
		lenght_y = (s16)((left_down_y - left_up_y) + diff_y);	//ｙ座標の範囲
	}

	for (ch_count = 0; ch_count < ch_num; ch_count++)
	{
		sensorLeft_X = proller_range[ch_count].roller_a_left_pos;
		sensorRight_X = proller_range[ch_count].roller_b_right_pos;

		// 先端検出
		if ((sensorLeft_X <= right_up_x) && (right_up_x <= sensorRight_X))       // 紙幣右端がかかっている
		{
			billTopEdge = right_up_y;
		}
		else if ((sensorLeft_X <= left_up_x) && (left_up_x <= sensorRight_X))  // 紙幣左端がかかっている
		{
			billTopEdge = left_up_y;
		}
		else if ((left_up_x < sensorLeft_X) && (sensorRight_X < right_up_x))     // 紙幣全体がかかっている
		{
			if (tan > 0)
			{
				diff_x = sensorLeft_X - left_up_x;
				diff_y = (int)(diff_x * tan);
				billTopEdge = (short)(left_up_y - diff_y);
			}
			else if (tan < 0)
			{
				diff_x = sensorRight_X - left_up_x;
				diff_y = (int)(diff_x * tan);
				billTopEdge = (short)(left_up_y - diff_y);
			}
			else
			{
				billTopEdge = left_up_y;
			}
		}
		else if ((sensorLeft_X < left_up_x) && ( sensorRight_X < left_up_x) && (left_down_x <= sensorRight_X))		// 紙幣左端より左
		{
			//diff_x = left_up_x - sensorRight_X;
			//diff_y = (int)(diff_x / tan);
			//billTopEdge = (short)(left_up_y + diff_y);
			billTopEdge = left_up_y;

		}
		else if ((right_up_x < sensorLeft_X) && (right_up_x < sensorRight_X) && ( sensorLeft_X <= right_down_x))		// 紙幣右端より右
		{
			//diff_x = sensorLeft_X - right_up_x;
			//diff_y = (int)(diff_x / tan);
			//billTopEdge = (short)(right_up_y + diff_y);
			billTopEdge = right_up_y;
		}
		else
		{
			billTopEdge = 0;
		}

		if (billTopEdge < 0)
		{
			top_edge[ch_count] = 0;
		}
		else
		{
			top_edge[ch_count] = billTopEdge;
		}
	}

	// 座標変換
	*pbill_lengh_y = (u16)((lenght_y / sub_sampling_pitch));
	if ((lenght_y % sub_sampling_pitch) != 0)
	{
		*pbill_lengh_y += 1;
	}

	for (ch_count = 0; ch_count < ch_num; ch_count++)
	{
		if (top_edge[ch_count] != 0)
		{
			bill_top_edge = (top_edge[ch_count] / sub_sampling_pitch)+ sub_offset ;
			if (bill_top_edge > 0)
			{
				pbill_top_edge[ch_count] = (u16)bill_top_edge;
			}
			else
			{
				pbill_top_edge[ch_count] = 0;
			}
		}
		else
		{
			pbill_top_edge[ch_count] = 0;
		}
	}
	ret = 1;

	return ret;
}

//以下、学習用の関数　実機処理では使わない。

//学習用関数　紙幣学習MAPの作成を行う。
u8 get_thickness_map(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, u16* pthkns_map, u16 map_size_x, u16 map_size_y, u8 mesh_size_x, u8 mesh_size_y,u8 tc_corr, u8 bc_mode)
{
	int map_x, map_y, mesh_x, mesh_y;
	// 
	u32 bill_size_x = map_size_x * mesh_size_x;
	u32 bill_size_y = map_size_y * mesh_size_y;

	// 識別紙幣サイズ
	s16 half_size_x = (s16)(bill_size_x / 2);
	s16 half_size_y = (s16)(bill_size_y / 2);

	s16 start_x, start_y;
	u32 average = 0;
	u32 total_num = 0;
	u32 thkns_data = 0;
	s16 logical_posx = 0;
	s16 logical_posy = 0;

	ST_THKNS_ROLLER_RANGE* proller_range = 0;
	u8 sensor_count;
	u8 ret = 0;
	u8 sens_offset;
	s16 multiplier = 100;

	//黒補正用パラメータ
	s16 bc_s = 1;
	s16 bc_e = 0;
	float tmp_bc_val = 0.0;
	float bc_val[THICKNESS_SENSOR_MAX_NUM];
	//s32 cres = 0;
	s32 x;
	s32 y;
	u16 pre_val;
	u8  bc_num;
	u16 val;

	memset(bc_val, 0, sizeof(float) * THICKNESS_SENSOR_MAX_NUM);

	//モード切替　追々はテンプレートパラメータ化する
	//u8 bc_mode = 1;
	//u8 tc_corr = 1;


	// 厚みセンサCh毎絶対座標取得
	ret = get_roller_range(pbs, &proller_range);

	if (ret == 0)
	{
		return 1;
	}
	sensor_count = (u8)(pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_all_pix - pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min);
	sens_offset = (u8)pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min;

	//if (pbs->LEorSE == LE)
	//{
	start_x = -half_size_x;
	start_y = half_size_y;
	//}
	//else
	//{
	//	start_y = -half_size_x;
	//	start_x = -half_size_y;
	//}


	if (tc_corr == 0)	//tc1-tc2をしない場合
	{	
		multiplier = 1;	//乗数は１
	}

	if(bc_mode == 1)	//黒補正　あり
	{
		//黒補正を計算する。
		//////各チャネルの黒補正を行います。	add by furuta 21/1/18
		////紙幣の先端座標を求める
		if (pbs->left_up_y < pbs->right_up_y)
		{
			bc_e = pbs->left_up_y / pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch - 1;
		}
		else
		{
			bc_e = pbs->right_up_y / pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch - 1;
		}

		//エラーチェック
		if (bc_e <= bc_s)
		{
			return ret;
		}

		////各チャネルの補正値を求める
		//各チャネルの補正値の計算
		for (x = sens_offset; x < sensor_count + sens_offset; x++)
		{
			tmp_bc_val = 0.0f;
			bc_num = 0;
			pre_val = 0;

			for (y = bc_s; y < bc_e; y++)
			{
				val = P_GetThicknessdata(pbs, x, y, tc_corr);

				if ((pre_val + ONE_MICRO_METER >= val && pre_val - ONE_MICRO_METER <= val) && y != bc_s)
				{
					tmp_bc_val = tmp_bc_val + val;	//総和計算
					bc_num++;
				}

				pre_val = val;					//１つ前の値を保存
			}

			if (bc_num == 0)
			{
				bc_num = 1;
			}

			bc_val[x] = tmp_bc_val / bc_num;		        //平均計算（黒補正値
		}
	}
	else //黒補正なし
	{
		for (x = 0; x < THICKNESS_SENSOR_MAX_NUM; x++)
		{
			bc_val[x] = 0;
		}

	}


	// MAPサイズでforループ
	for (map_y = 0; map_y < map_size_y; map_y++)
	{
		for (map_x = 0; map_x < map_size_x; map_x++)
		{
			average = 0;
			total_num = 0;
			thkns_data = 0;

			// メッシュサイズで平均化
			for (mesh_y = 0; mesh_y < mesh_size_y; mesh_y++)
			{
				for (mesh_x = 0; mesh_x < mesh_size_x; mesh_x++)
				{
					logical_posx = (s16)(start_x + (map_x * mesh_size_x) + mesh_x);
					logical_posy = (s16)(start_y - (map_y * mesh_size_y) + mesh_y);

					//座標を入力して画素値を取得する.TC1-TC2した結果→thkns_data
					//ここでの入力座標は論理座標系
					thkns_data = get_thickness_data(pbs, pnote_param, proller_range, sensor_count, logical_posx, logical_posy , bc_val, multiplier, tc_corr);

					if (thkns_data != 0)
					{
						//平均化する
						average = (total_num * average + thkns_data) / (total_num + 1);
						total_num++;
					}
				}
			}
			if (average > USHRT_MAX)
			{
				average = USHRT_MAX;
			}

			//MAPのメモリに記録する。	最小値はC#の方で求める。
			pthkns_map[(map_y * map_size_x) + map_x] = (u16)average;
		}

	}

	return 0;
}

//
u16 get_thickness_data(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy,float* bc_val,s16 multiplier, u8 tc_corr)
{
	u16 thickness_data = 0;
	s16 tc_actual_posy;
	s8 thkns_ch;
	u8 ret;
	s32 cres;

	//　厚みローラー座標判定
	//　結果として入力論理座標に対応するチャンネルとメカ厚センサの座標系でのY座標を得る。
	ret = get_thickness_ch_coordinate(pbs, pnote_param, proller_range, sensor_count, logical_posx, logical_posy, &thkns_ch, &tc_actual_posy);

	if ((ret == 1) && (thkns_ch >= 0) && (tc_actual_posy >= 0))
	{
		// 厚みデータ取得
		//　TC1－TC2した結果を取得する。
		thickness_data = P_GetThicknessdata(pbs, thkns_ch, tc_actual_posy , tc_corr);

		cres = (s32)(thickness_data - bc_val[thkns_ch]);	//黒補正
		if (cres < 0)
		{
			thickness_data = 0;
		}
		else
		{
			thickness_data = (u16)cres;
		}

		thickness_data *= multiplier;

	}

	return thickness_data;
}

//
u8 get_thickness_ch_coordinate(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, s8* pthkns_ch, s16* pthikns_actual_posy)
{
	u8 ret;
	s16 bill_actual_posx;
	s16 bill_actual_posy;
	s16 tc_actual_posx;
	s16 tc_actual_posy;
	s8 thkns_ch;

	/* 論理⇒物理座標変換 */
	ret = convert_bill_actual_coordinate(pbs, pnote_param, logical_posx, logical_posy, &bill_actual_posx, &bill_actual_posy, &tc_actual_posx, &tc_actual_posy);

	/* 厚みローラー座標判定 */
	thkns_ch = get_thickness_sensor_channel(proller_range, sensor_count, bill_actual_posx);

	if ((thkns_ch >= 0) && (tc_actual_posy >= 0))
	{	
		//正しく結果を採取できた時
		*pthkns_ch = thkns_ch;
		*pthikns_actual_posy = tc_actual_posy;
		ret = 1;
	}
	else
	{
		//正しく結果を採取できなかったとき
		*pthkns_ch = -1;
		*pthikns_actual_posy = -1;
		ret = 0;
	}
	return ret;
}

//
u8 new_get_thickness_ch_coordinate(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, s8* pthkns_ch, s16* pthikns_actual_posy)
{
	u8 ret;
	s16 bill_actual_posx;
	s16 bill_actual_posy;
	//s16 tc_actual_posx;
	s16 tc_actual_posy;
	s8 thkns_ch;

	/* 論理⇒物理座標変換 */
//	u8 ret = 0;
	/* 物理座標に変換 */
	// 物理座標X軸取得
	ST_SPOINT		spoint;
	spoint.p_plane_tbl = (enum P_Plane_tbl)UP_R_R;
	spoint.l_plane_tbl = (enum L_Plane_tbl)UP_R_R;
	spoint.x = logical_posx;
	spoint.y = logical_posy;

	ret = (u8)new_L2P_Coordinate(&spoint, pbs, pnote_param);

	bill_actual_posx = (s16)spoint.x;
	bill_actual_posy = (s16)spoint.y;

	/* 厚みローラー座標判定 */
	if (pbs->LEorSE == LE)
	{
		thkns_ch = get_thickness_sensor_channel(proller_range, sensor_count, bill_actual_posx);
	}
	else
	{
		thkns_ch = get_thickness_sensor_channel(proller_range, sensor_count, bill_actual_posy);

	}
	if ((thkns_ch >= 0) /*&& (tc_actual_posy >= 0)*/)
	{
		// 厚みローラーY座標取得
		spoint.p_plane_tbl = (enum P_Plane_tbl)UP_TC1;
		spoint.l_plane_tbl = (enum L_Plane_tbl)UP_TC1;
		spoint.x = logical_posx;
		spoint.y = logical_posy;

		ret = (u8)new_L2P_Coordinate(&spoint, pbs, pnote_param);

		//tc_actual_posx = (s16)spoint.x;
		tc_actual_posy = (s16)spoint.y;

		if ((tc_actual_posy >= 0))
		{
			*pthkns_ch = thkns_ch;
			*pthikns_actual_posy = tc_actual_posy;
			ret = 1;
		}
		else
		{
			*pthkns_ch = -1;
			*pthikns_actual_posy = -1;
			ret = 0;
		}
	}
	else
	{
		*pthkns_ch = -1;
		*pthikns_actual_posy = -1;
		ret = 0;
	}
	return ret;
}

u8 convert_bill_actual_coordinate(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, s16 logical_posx, s16 logical_posy, s16* pbill_actual_posx, s16* pbill_actual_posy, s16* ptc_actual_posx, s16* ptc_actual_posy)
{
	u8 ret = 0;
	/* 物理座標に変換 */
	ST_SPOINT		spoint;
	spoint.p_plane_tbl = (enum P_Plane_tbl)UP_R_R;
	spoint.l_plane_tbl = (enum L_Plane_tbl)UP_R_R;
	spoint.x = logical_posx;
	spoint.y = logical_posy;

	ret = (u8)new_L2P_Coordinate(&spoint, pbs, pnote_param);

	*pbill_actual_posx = (s16)spoint.x;
	*pbill_actual_posy = (s16)spoint.y;

	spoint.p_plane_tbl = (enum P_Plane_tbl)UP_TC1;
	spoint.l_plane_tbl = (enum L_Plane_tbl)UP_TC1;
	spoint.x = logical_posx;
	spoint.y = logical_posy;

	ret = (u8)new_L2P_Coordinate(&spoint, pbs, pnote_param);

	*ptc_actual_posx = (s16)spoint.x;
	*ptc_actual_posy = (s16)spoint.y;

	return ret;
}

//入力物理座標がメカ厚のどのchに対応するかを調べる
s8 get_thickness_sensor_channel(ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 actual_posx)
{
	u8 i = 0;
	u8 channel_count = sensor_count;
	s8 channel = -1;

	for (i = 0; i < channel_count; i++)
	{
		if ((proller_range[i].roller_a_left_pos <= actual_posx && actual_posx <= proller_range[i].roller_a_right_pos)
			|| (proller_range[i].roller_b_left_pos <= actual_posx && actual_posx <= proller_range[i].roller_b_right_pos))
		{
			channel = i;
			break;
		}
	}
	return channel;
}
