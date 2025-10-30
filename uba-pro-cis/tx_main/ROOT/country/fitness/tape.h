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
* @file tape.h
* @brief　テープ検知
* @date 2020 Created.
*/
/****************************************************************************/

#ifndef	_TAPE_H
#define	_TAPE_H

#define EVEN_NUM(IN) ((IN%2) != 0 ? (IN+1) : (IN))	//入力値を偶数にする。
#define DOT_SIZE_MM		(127)		/* 1dot = 0.127mm */

#ifndef THHKNS_CH_LENGTH
#define THHKNS_CH_LENGTH (256)
#endif

typedef struct					// CIS座標系で厚みセンサがトレースする座標
{
	u16 roller_a_left_pos;		//1つ目のローラーの左端
	u16 roller_a_right_pos;		//1つ目のローラーの右端
	u16 roller_b_left_pos;      //2つ目のローラーの左端
	u16 roller_b_right_pos;     //2つ目のローラーの右端
} ST_THKNS_ROLLER_RANGE;

typedef struct
{
	u32 chdata;
	u16 ave_count;
	u16 padding;
//	u8 ave_count;
} ST_THKNS_CH_DATA_AVE;

typedef struct
{
	//入力
	u16* plearn_data;			// 全面の基準データ配列への先頭オフセット
	u16 data_size;				// 全面の基準データの要素数
	u8 bill_size_x;				// 紙幣サイズX
	u8 bill_size_y;				// 紙幣サイズY
	u8 mesh_size_x;				// メッシュサイズX
	u8 mesh_size_y;				// メッシュサイズY

	u16 base_value;				// 紙幣厚み最小値
	u16 judge_count;			// 判定終了ポイント数
	u16	threshold;				// 閾値補助値（LSB 0.01）
	u8 threshold_type;			// 閾値種別

	u8 moving_average;			// 移動平均（する／しない）
	u8 moving_ave_value_a;		// 移動平均A値
	u8 divide_ab;				// 分割A-B（する／しない）
	u16 divide_line_ab;			// 分割ラインA-B値
	u8 moving_ave_value_b;		// 移動平均B値
	u8 divide_bc;				// 分割B-C（する／しない）
	u16 divide_line_bc;			// 分割ラインB-C値
	u8 moving_ave_value_c;		// 移動平均C値

	u8 exclude_mesh_top;		// 除外範囲　先端　メッシュ数
	u8 exclude_mesh_bottom;		// 除外範囲　後端　メッシュ数
	u8 exclude_mesh_left;		// 除外範囲　左端　メッシュ数
	u8 exclude_mesh_right;		// 除外範囲　右端　メッシュ数
	
	u8 continuous_judge;		// 連続判定閾値

	u8 mesh_skip_x;				// 
	u8 mesh_skip_y;				// 

	u8 tc1_tc2_corr;			// TC1-TC2の補正フラグ
	u8 black_corr;				// 黒補正フラグ

	//出力
	u16 err_code;						// エラーコード
	u8  result;							// 検知結果
	u8  level;							// レベル
	u16 detect_num;						// テープ検知数
	u8	ch_tape_num[THICKNESS_SENSOR_MAX_NUM];	// 各圧検センサ毎のテープ判定箇所数

} ST_TAPE;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	tape(u8 buf_num, ST_TAPE *st);
	u8 get_thickness_map(ST_BS *pbs, ST_NOTE_PARAMETER * pnote_param, u16* pthkns_map, u16 map_size_x, u16 map_size_y, u8 mesh_size_x, u8 mesh_size_y, u8 tc_corr, u8 bc_mode);
	u16 get_thickness_data(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, float* bc_val, s16 multiplier, u8 tc_corr);

	u8 get_thickness_ch_coordinate(ST_BS * pbs, ST_NOTE_PARAMETER * pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, s8* pthkns_ch, s16* pthikns_actual_posy);
	u8 new_get_thickness_ch_coordinate(ST_BS * pbs, ST_NOTE_PARAMETER * pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, s8* pthkns_ch, s16* pthikns_actual_posy);
	void calc_map_size(u8 bill_size_x, u8 bill_size_y, u8 mesh_size_x, u8 mesh_size_y, u16* pmap_size_x, u16* pmap_size_y);
	u8 convert_bill_actual_coordinate(ST_BS *pbs, ST_NOTE_PARAMETER * pnote_param, s16 logical_posx, s16 logical_posy, s16* pbill_actual_posx, s16* pbill_actual_posy, s16* ptc_actual_posx, s16* ptc_actual_posy);
	s8 get_thickness_sensor_channel(ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 actual_posx);
	void calc_moving_average(u32 input_data[][THHKNS_CH_LENGTH], u32 out_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, ST_TAPE *st, u16* pbill_top_edge, u16 bill_lengh_y);
	void moving_average_sub(u32 input_data[][THHKNS_CH_LENGTH], u32 out_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, u8 moving_ave_value, int start, int end, u16* pbill_top_edge);
	void convert_thickness_sensor_data(ST_BS *pbs, ST_NOTE_PARAMETER * pnote_param, u16* learn_map_data, u16 map_size_x, u16 map_size_y, ST_TAPE * st, ST_THKNS_ROLLER_RANGE* proller_range, ST_THKNS_CH_DATA_AVE plearn_ch_data[][THHKNS_CH_LENGTH], u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 array_size, u16 height);
	u16 compare_thickness_sensor_data(int array_size, u32 pinput_ch_data[][THHKNS_CH_LENGTH], ST_THKNS_CH_DATA_AVE plearn_ch_data[][THHKNS_CH_LENGTH], u32 compared_ch_data[][THHKNS_CH_LENGTH], u16 threshold, u8 continuous_judge);

	u8 get_thkns_data(ST_BS * pbs, u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 height, u8 bc_mode, u8 tc_corr_mode);

	u8 get_roller_range(ST_BS * pbs, ST_THKNS_ROLLER_RANGE** proller_range);
	u16 pre_judge_sensor_data(u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, u16 threshold);
	u8 calculate_billedge(ST_BS* pbs, ST_THKNS_ROLLER_RANGE* proller_range, u16 ch_num, u16* pbill_top_edge, u16* pbill_lengh_y);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TAPE_H */
