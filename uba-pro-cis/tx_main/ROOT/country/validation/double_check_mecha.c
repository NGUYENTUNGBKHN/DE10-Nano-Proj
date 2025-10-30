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
* @file double_check_mecha.c
* @brief 重券検知（メカ厚）
* @date 2020/12/ Created.
* @updata
*/
/****************************************************************************/
#include <stdio.h>
#include <string.h>

#define EXT
#include "../common/global.h"
#include "double_check_mecha.h"

#define THICKNESS_SENSOR_NUM (32)

//メカ式重券検知のメイン関数
s16	double_check_mecha(u8 buf_num, ST_DBL_CHK_MECHA *st)
{
	u8 ret = 0;

	u16 double_check_threshold = st->double_check_threshold;	// 重券検出厚み閾値
	u8  double_area_ratio = st->double_area_ratio;				// 面積比率閾値(%)
	u16 bill_check_threshold = st->bill_check_threshold;		// 紙幣検出厚み閾値
	u8  exclude_length = st->exclude_length;					// 先頭判定除外範囲(mm)

	u8  result = 0;
	u8  double_check_ratio = 0;
	u16 double_check_count = 0;
	u16 bill_check_count = 0;
	u8  double_check_point[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	u8  bill_check_point[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	u8  bill_top_point[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	u32 bill_integrated_value = 0;
	u16 bill_thickness_average = 0;

	ST_BS* pbs = work[buf_num].pbs;
	u8 sub_sampling_pitch = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch;
	u16 height = pbs->block_count / (sub_sampling_pitch / pbs->Blocksize);		// 副走査方向最大データ数
	u16 sensor_count = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_all_pix - pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min;	// +1;

	u16 thkns_ch_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };

	s16 bill_top_edge = 0;
	s16 bill_end_edge = 0;
	u8 exlude_count = 0;
	u8 continuous_flg = 0;
	// 先頭除外範囲分のポイント数算出
	u8 top_edge_exclude_offset = (u8)((25.4 / (pbs->Subblock_dpi / sub_sampling_pitch) ) * exclude_length);

	// 紙幣先端角、後端角の取得
	s8 sub_offset = pbs->PlaneInfo[UP_TC1].sub_offset;
	s16 left_up_y = pbs->left_up_y;			//頂点左上ｙ
	s16 left_down_y = pbs->left_down_y;		//頂点左下ｙ
	s16 right_up_y = pbs->right_up_y;		//頂点右上ｙ
	s16 right_down_y = pbs->right_down_y;	//頂点右下ｙ

	s32 ch = 0;
	s32 idx = 0;
	s32 j = 0;
	s32 i = 0;

#ifndef _RM_
	s32 array_length = THICKNESS_SENSOR_MAX_NUM * THHKNS_CH_LENGTH;
	s32 x;

	u32 raw_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };
	u32 bill_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };
	u32 bill_top_point_callback[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };
#endif

	if (left_up_y > right_up_y)			// 紙幣先端角
	{
		bill_top_edge = (right_up_y + sub_offset)/ sub_sampling_pitch;
	}
	else
	{
		bill_top_edge = (left_up_y + sub_offset) / sub_sampling_pitch;
	}

	if (bill_top_edge < 0)
	{
		bill_top_edge = 0;
	}

	if (left_down_y > right_down_y)		// 紙幣後端角
	{
		bill_end_edge = (left_down_y + sub_offset) / sub_sampling_pitch;
	}
	else
	{
		bill_end_edge = (right_down_y + sub_offset) / sub_sampling_pitch;
	}
	if (bill_end_edge > height)
	{
		bill_end_edge = height;
	}

	// 厚みセンサデータ(TC1 - TC2)取得 → thkns_ch_data
	ret = get_double_check_thkns_data(pbs, thkns_ch_data, height, sensor_count);

	if (ret == 0)
	{
		// 各CH毎に処理
		for (ch = 0; ch < sensor_count; ch++)
		{
			exlude_count = 1;
			// 紙幣先端角頭から、紙幣後端角まで走査
			for (idx = bill_top_edge; idx < bill_end_edge+1; idx++)
			{
				u16 thkns_value = thkns_ch_data[ch][idx];
				if (thkns_value > bill_check_threshold)			// 紙幣厚み閾値を超える
				{
					// 最初に紙幣厚み閾値を超えたポイント
					if (bill_top_point[ch] == 0)
					{
						continuous_flg = 1;
						// 除外範囲分連続して紙幣厚み閾値を超えているか確認する
						for (j = 0; j < top_edge_exclude_offset+1; j++)
						{
							//検知した位置から除外範囲の間の画素値を紙幣閾値と比較
							if (thkns_ch_data[ch][idx + j] <= bill_check_threshold)
							{
								//1度でも下回ればその位置から再度紙幣スキャンを開始するようにフラグを設定する。
								continuous_flg = 0;
								idx += j;
								break;
							}
						}
						if (continuous_flg == 1)			//先端と判定された場合
						{
							bill_top_point[ch] = (u8)idx;	//先端座標を記録する。
						}
						else
						{
							break;	//引き続き先端検知スキャンを行う。
						}
					}
					// 先頭除外範囲より後ろ 10回以上紙幣有判定をしないと、センサと被ったとは言えない。
					if (top_edge_exclude_offset < exlude_count)
					{
						bill_check_count += 1;						// 全紙幣検出ポイント数
						bill_check_point[ch] += 1;					// CH毎紙幣検出ポイント数
						bill_integrated_value += thkns_value;		// 厚み積算値
						if (thkns_value > double_check_threshold)	// 重券厚み閾値を超える
						{
							double_check_count += 1;				// 全重券検出ポイント数
							double_check_point[ch] += 1;			// CH毎重券検出ポイント数
						}
					}
					exlude_count += 1;	//このChの紙幣をスキャンできたライン数
				}
			}
		}

		if (bill_check_count == 0)		// 紙幣判定箇所なし
		{
			// 面積比率算出
			double_check_ratio = 0;
			// 判定結果
			result = 2;
			// 紙幣判定箇所の厚み平均値
			bill_thickness_average = 0;
		}
		else
		{
			// 面積比率算出
			double_check_ratio = (u8)((double_check_count * 100) / bill_check_count);
			// 判定結果
			if (double_check_ratio >= double_area_ratio)
			{
				result = 1;
			}
			// 紙幣判定箇所の厚み平均値
			bill_thickness_average = (u16)(bill_integrated_value / bill_check_count);
		}
	}

	// 出力
	// 結果ブロック
	st->result = result;										// 判定結果
	st->double_check_ratio = double_check_ratio;				// 重券面積比率
	st->double_check_count = double_check_count;				// 重券判定箇所ポイント数
	st->bill_check_count = bill_check_count;					// 紙幣判定箇所ポイント数

	// 中間情報
	st->bill_thickness_average = bill_thickness_average;		// 紙幣判定箇所の厚み平均値
	for (ch = 0; ch < THICKNESS_SENSOR_MAX_NUM; ch++)
	{
		st->double_check_point[ch] = double_check_point[ch]; 	// 重券判定箇所数　（各圧検センサ毎　最大16センサ分）
		st->bill_check_point[ch] = bill_check_point[ch];		// 紙幣判定箇所数　（各圧検センサ毎　最大16センサ分）
		st->bill_top_point[ch] = bill_top_point[ch];			// 紙幣先端判定箇所　（各圧検センサ毎　最大16センサ分）
	}
	st->exclude_point = top_edge_exclude_offset;				// 先頭判定除外範囲(point)
	st->sensor_num = (u8)sensor_count;							// 厚検センサCH数
	st->check_point_top = (u8)bill_top_edge;					// 検知対象範囲先端
	if (bill_end_edge < 256)
	{
		st->check_point_end = (u8)bill_end_edge;				// 検知対象範囲後端
	}
	else
	{
		st->check_point_end = 0xFF;
	}

#ifndef _RM_
	// 各CH毎に処理
	for (ch = 0; ch < sensor_count; ch++)
	{
		bill_top_point_callback[ch][0] = bill_top_point[ch];
		exlude_count = 0;
		// 紙幣先端角頭から、紙幣後端角まで走査
		for (idx = bill_top_edge, x=0; idx < bill_end_edge+1; idx++,x++)
		{
			u16 thkns_value = thkns_ch_data[ch][idx];
			bill_data[ch][x] = thkns_value;
		}
		for (i = 0; i < THHKNS_CH_LENGTH; i++)
		{
			raw_data[ch][i] = thkns_ch_data[ch][i];
		}
	}
	//コールバック関数　デバッグ情報
	callback_tape_len((u32*)raw_data, array_length, (u32*)bill_data, array_length, (u32*)bill_top_point_callback, array_length, THHKNS_CH_LENGTH, bill_end_edge+1, sensor_count, top_edge_exclude_offset, 0);
#endif

	return 0;

}

/****************************************************************/
/**
* @brief		TC1－TC2を計算して、thkns_ch_dataに格納する。
*@param[in]		pbs				サンプリングデータ
*				height			Y方向の有効画素範囲
*				sensor_count	Chの数
*
*@param[out]	thkns_ch_data	TC1-TC2の結果
*
* @return		ret　			0：固定
*/
/****************************************************************/
u8 get_double_check_thkns_data(ST_BS * pbs, u16 thkns_ch_data[][THHKNS_CH_LENGTH], u16 height, u16 sensor_count)
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
	u16 sensor_ofs = 0;
	u16 tc1_idx_x = 0;

	sensor_ofs = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min;
	period = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].Address_Period;
	offset_tc1 = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].Address_Offset;
	offset_tc2 = pbs->PlaneInfo[THICKNESS_SENSOR_NUM + 1].Address_Offset;
	data_type = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].data_type;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < sensor_count; x++)
		{
			tc1_idx_x = x + sensor_ofs;
			if (data_type > 8)
			{
				/*2バイト分のデータを取り出します*/
				memcpy(&sens_dt_tc1, &pbs->sens_dt[y * period + (tc1_idx_x * 2) + offset_tc1], 2);
				memcpy(&sens_dt_tc2, &pbs->sens_dt[y * period + (x * 2) + offset_tc2], 2);
			}
			//通常
			else
			{
				sens_dt_tc1 = (s16)pbs->sens_dt[y * period + tc1_idx_x + offset_tc1];
				sens_dt_tc2 = (s16)pbs->sens_dt[y * period + x + offset_tc2];
			}

			if (sens_dt_tc1 > sens_dt_tc2)
			{
				thkns_ch_data[x][y] = (sens_dt_tc1 - sens_dt_tc2);
			}
		}
	}

	return ret;
}


//End of File
