/*
 * compare.c
 *
 *  Created on: 2018/02/27
 *      Author: suzuki-hiroyuki
 */
//#include "stdafx.h"
#include <string.h>
#include <math.h>
//#include "common.h"
#include "country_custom.h"

#define EXT
#include "../common/global.h"
#ifndef SIMURATION
	#include "com_ram.c"
	#include "cis_ram.c"
#else
	#include "com_ram.h"
	#include "cis_ram.h"
#endif

#if DEBUG_COMPERE_TIME
#include "xtime_l.h"
#include "js_oswapi.h"
#endif

extern const float skew_limits[];
extern const unsigned char template_data[];
extern const u32 length_limits[];
extern const u32 width_limits[];
extern const u32 length_over_margin;
extern const u32 length_less_margin;
extern const u32 width_over_margin;
extern const u32 width_less_margin;
extern int is_clip_detected(void);


#if 0//RBA-40C
void parameter_set(u8 buf_num);
#endif
#if BANKNOTE_CYCLIC_ENABLE
void cyclic_data_fix(u8 buf_num);
#endif
s16 edge_detect(u8 buf_num);
int skew_check(u8 buf_num);
int bar_skew_check(u8 buf_num);
int compare(void);
int	rd_smpldt(void *p, int aryCount ,int flg[] , unsigned char *temp_ary);
#if 0
s32 nn_identification(u8 buf_num);
#endif
s8  nn_res_size_check(u8 buf_num);
s8  nn_res_width_check(u8 buf_num);
s8  nn_res_length_check(u8 buf_num);

extern void create_mag_wave(u8 buf_num, u8 way);
extern u8 mag_amount_chk_invalid_count(u8 buf_num, u32 denomination, u32 way);

int	rd_smpldt(void *p, int aryCount ,int flg[] , unsigned char *temp_ary)
{	
	//--ポイント抽出用*----------------------------
	int ii;
	int i;
	u8 buf_num = 0;
	
	// ichijo for OCR result
#ifdef OCR_DEBUG
	int ocr_length = 0;
	int ser_length = 0;
#endif

#ifdef VALID_CNY_OCR
	int ser_length = 10;
#endif
#ifdef VALID_EUR_OCR
	int ser_length = 12;
#endif
#ifdef VALID_JPY_OCR
#endif

	memset(work[0].pbs->ser_num1, '\0' ,16);
	memset(work[0].pbs->ser_num2, '\0' ,16);

	//外形検知紙粉対策コード
	work[buf_num].pbs->disable_area_a = 0;
	work[buf_num].pbs->disable_area_b = 0;

	work[buf_num].pbs->insertion_direction = 0;
	work[buf_num].pbs->insert_way = 0;
	//work[buf_num].pbs->transport_direction = 0;
	st_work[buf_num].e_code = 0;

	//簡易的にテンプレートシェルが読み込まれているかどうかを調べる
	if((temp_ary[0] != 0) && (work[0].pbs->block_count != 0) && (work[0].pbs->block_count <= BLOCK_MAX))
	{
		//テンプレートパラメタを構造体にセット
		//memcpy( &st_work[0] , (temp_ary) + 0x1b04 , sizeof(st_work[0]) );
		template_top_pt = (void*)(temp_ary);

		for(ii = 0; ii < 16; ++ii)
		{
			work[buf_num].pbs->fitness[ii].bit.result = 0;
			work[buf_num].pbs->fitness[ii].bit.level = 0;
		}

		//アルゴレベル設定 64金種分
		for(i = 0; i < 64; ++i )
		{
			memset(&fit_level_thrs[i].fit_level[0],70,64);
			memset(&fit_level_thrs[i].uf_level[0],40,64);
			memset(&fit_level_thrs[i].rj_level[0],0,64);
			memset(&fit_level_thrs[i].level_margin,10,64);

		}

		//方向フラグ初期化
		work[buf_num].pbs->insertion_direction = 0xff;

		//中間情報初期化
		memset(&work[buf_num].pbs->mid_res_outline, 0 ,2880);

		//replay_fitness_work.mode = REPLAY_FITNESS_MODE_ON;
		replay_fitness_work.mode = 0;


		//バーコードリード上位通信パラメタ
		barcode_read_top_config.character_num_limits_max = 30;
		barcode_read_top_config.execute_flg = 1;
		barcode_read_top_config.scan_side = 2;
		barcode_read_top_config.setting_change_flg = 0;


		//シェル実行
		start_template_sequence(buf_num);
		replay_fitness_work.mode = 0;
		if(st_work[buf_num].template_shell_completion == 0)	//外形検知完了
		{
			restart_template_sequence(buf_num);
		}

		if(st_work[buf_num].template_shell_completion == 0)	//CIS処理完了
		{
			restart_template_sequence(buf_num);
		}
	}
	else
	{
		st_work[buf_num].e_code = E_UNDEFIND_FNC_CODE;
	}

	return st_work[buf_num].e_code;
}

void cyclic_data_fix(u8 buf_num)
{
	ST_BS *pbs = work[buf_num].pbs;
	u32 size1 = 0;
	u8 *src1;
	u8 *dst1;
	u32 size2 = 0;
	u8 *src2;
	u8 *dst2;

	//blank6[16] -> FPGA_REG.CISDT_SET.BIT.PREBK
	//blank6[17] -> FPGA_REG.CISDT_ST.BIT.OLDESTBK
	//サイクリックデータずれ修正処理
	if((pbs->st_model_area.cisdt_st_oldestbk != 0) && (pbs->st_model_area.cisdt_set_prebk >= pbs->st_model_area.cisdt_st_oldestbk))
	{
		//サイクリック領域一時退避
		size1 = (pbs->st_model_area.cisdt_set_prebk + 1) * pbs->Blockbytesize;
		src1 = &pbs->sens_dt[0];
#if (_DEBUG_CIS_MULTI_IMAGE==1)
		dst1 = (u8*)(BILL_NOTE_IMAGE_TOP+(sizeof(ST_BS)*(BILL_NOTE_IMAGE_MAX_COUNT+1)));
#else
		dst1 = (u8*)(BILL_NOTE_IMAGE_TOP+(sizeof(ST_BS)));
#endif
		memcpy(dst1, src1, size1);

		//先頭を後方へ移動
		size2 = (pbs->st_model_area.cisdt_st_oldestbk) * pbs->Blockbytesize;
		src2 = dst1;
		dst2 = &pbs->sens_dt[(pbs->st_model_area.cisdt_set_prebk - (pbs->st_model_area.cisdt_st_oldestbk - 1)) * pbs->Blockbytesize];

		memcpy(dst2, src2, size2);

		//後方を先頭へ移動
		size2 = (pbs->st_model_area.cisdt_set_prebk - (pbs->st_model_area.cisdt_st_oldestbk - 1)) * pbs->Blockbytesize;
		src2 = dst1 + pbs->st_model_area.cisdt_st_oldestbk * pbs->Blockbytesize;
		dst2 = &pbs->sens_dt[0];
		memcpy(dst2, src2, size2);
	}
}

/* 外形検知 */
s16 edge_detect(u8 buf_num)
{
#if EDGE_DETECT_ENABLE
	ST_BS *pbs = work[buf_num].pbs;

	/*初期化*/
	pbs->angle		= 0;						// 傾き (tan_th *4096)
	pbs->center_x	= 0;// 中心座標x
	pbs->center_y	= 0;// 中心座標y
	pbs->note_y_size	= 0;// 札幅
	pbs->note_x_size	= 0;// 札長

	// 外形検知のデバッグためのパラメタ仮設定
	ED_DC.Plane = USE_EDGE_DETECT_SENSOR;			 //用いるプレーン指定
	ED_DC.Main_scan_max_val = pbs->PlaneInfo[ED_DC.Plane].main_effective_range_max;	// イメージデータの主走査方向ドット数
	ED_DC.Main_scan_min_val = pbs->PlaneInfo[ED_DC.Plane].main_effective_range_min;	// イメージデータの主走査方向ドット数
	ED_DC.Sub_scan_max_val = (((pbs->Blocksize / pbs->PlaneInfo[ED_DC.Plane].sub_sampling_pitch) * pbs->block_count));	// 副走査方向の最大有効範囲
	ED_DC.Sub_scan_min_val = 0;
	ED_DC.MinSE = ST_EDGE_DETECT_MINSE;			// 札のSE側の最小許容値 (mm)※暫定値
	ED_DC.MaxSE = ST_EDGE_DETECT_MAXSE;			// 札のSE側の最大許容値 (mm)※暫定値
	ED_DC.MinLE = ST_EDGE_DETECT_MINLE;			// 札のLE側の最小許容値 (mm)※暫定値
	ED_DC.MaxLE = ST_EDGE_DETECT_MAXLE;			// 札のLE側の最大許容値 (mm)※暫定値
	ED_DC.Step_Movement = ST_EDGE_DETECT_STEP_MOVE;// 探索のステップ幅(mm)	 ※暫定値
	ED_DC.Small_Backtrack = ST_EDGE_DETECT_SMALL_BACKTRACK;//次の探索開始位置への戻り幅(dot)	※暫定値

	ED_DC.th_dynamic_decision_flg = 1;	//閾値を動的に決定するかのフラグ　0：オフ　1：オン

	//-----------------------------------

	ED_DC.edge_err_code = (u16)OPdet_Start_DC(buf_num);								// 外形検出ルーチン

	//------------------------------------
	if(ED_DC.edge_err_code != 0)	/*外形検知失敗*/
	{
		// 結果確認
		pbs->angle		= 0xFFFF;		// 傾き
		pbs->center_x	= 0xFFFF;		// 中心座標x
		pbs->center_y	= 0xFFFF;		// 中心座標y
		pbs->note_y_size = 0xFFFF;		// 札幅
		pbs->note_x_size = 0xFFFF;		// 札長
		cal_note_parameter(buf_num);

		return ED_DC.edge_err_code;
	}

	// 結果確認
	pbs->angle		= ED_DC.SKEW;						// 傾き (tan_th *4096)
	pbs->center_x	= ED_DC.AlignedBillCenterPoint.x;	// 中心座標x
	pbs->center_y	= ED_DC.AlignedBillCenterPoint.y;	// 中心座標y
	pbs->note_y_size = ED_DC.width_dot;					// 札幅
	pbs->note_x_size = ED_DC.length_dot;				// 札長

	//座標変換に必要なパラメタを計算。
	cal_note_parameter(buf_num);

	/*頂点座標を200dpi座標系に変換*/
	/*左上*/
	pbs->right_up_x = (s16)ED_DC.AlignedUpperRight.x;
	work[buf_num].pbs->right_up_y = (s16)ED_DC.AlignedUpperRight.y;

	/*左下*/
	pbs->right_down_x = (s16)ED_DC.AlignedLowerRight.x;
	pbs->right_down_y = (s16)ED_DC.AlignedLowerRight.y;

	/*右上*/
	pbs->left_up_x = (s16)ED_DC.AlignedUpperLeft.x;
	pbs->left_up_y = (s16)ED_DC.AlignedUpperLeft.y;

	//右下
	pbs->left_down_x = (s16)ED_DC.AlignedLowerLeft.x;
	pbs->left_down_y = (s16)ED_DC.AlignedLowerLeft.y;


	ex_validation.bill_length = pbs->note_y_size;
	return 0;
#else
	return 0;
#endif
}

/* 識別 */
int compare(void)
{
	int result = 0;
#if COMPARE_ENABLE
	ST_BS *pbs = work[0].pbs;

	result = rd_smpldt(pbs, 1 , 0, (u8*)template_data);

	if(result == 0)
	{
		if((pbs->mid_res_nn.max_node_num/4 >= 0) && (pbs->mid_res_nn.max_node_num/4 < DENOMI_MAX))
		{
			ex_validation.denomi = pbs->mid_res_nn.max_node_num/4;
			ex_validation.direction = pbs->mid_res_nn.max_node_num%4;
		}
		else
		{
			// 金種IDエラー
			result = ERR_FAILURE_NN_TH1;
		}
	}

	ex_validation.bill_length = pbs->note_y_size;
#endif
	return result;
}


int skew_check(u8 buf_num)
{
#if SKEW_CHECK_ENABLE
	ST_BS *pbs = work[buf_num].pbs;
	double t = 0.0f;
	s16 tmpvalue = 0;

	t = tan((double)(skew_limits[pbs->mid_res_nn.max_node_num / 4]/PIRAD));
	tmpvalue = (s16)(t * 4096);

	if((pbs->angle > tmpvalue) || (pbs->angle < -tmpvalue))
	{
		return BIT_EDGE_DETECT;
	}
#endif
	return 0;
}
int bar_skew_check(u8 buf_num)
{
#if SKEW_CHECK_ENABLE
	ST_BS *pbs = work[buf_num].pbs;

	if((pbs->angle > SKEW_THRESHOLD_10_0DGREE) || (pbs->angle < -SKEW_THRESHOLD_10_0DGREE))
	{
		return BIT_EDGE_DETECT;
	}
#endif
	return 0;
}
/* 鑑別 */
int block_compare(void)
{
	int result = 0;
	ST_BS *pbs = work[0].pbs;

	cal_note_parameter(0);//add 2020/03/10　方向補正解除

#if CUSTOM_CHECK_ENABLE
	cs_chk.note_size_x = pbs->note_x_size;
	cs_chk.note_size_y = pbs->note_y_size;
	cs_chk.ir_mask_ptn_diameter_x = DIAMETER_100DPI_X;			// 赤外色マスクパターンの直径x
	cs_chk.ir_mask_ptn_diameter_y = DIAMETER_100DPI_Y;			// 赤外色マスクパターンの直径y
	cs_chk.ir_mask_ptn_divide_num = DIVIDE_100DPI;		// 赤外色マスクパターンの割る数
	cs_chk.pir_mask_ptn = MASK_PAT_100DPI;						// 赤外色マスクパターン1のポインタ

	cs_chk.tir_mask_ptn_diameter_x = DIAMETER_50DPI_X;			// 透過赤外マスクパターンの直径x
	cs_chk.tir_mask_ptn_diameter_y = DIAMETER_50DPI_Y;			// 透過赤外マスクパターンの直径y
	cs_chk.tir_mask_ptn_divide_num = DIVIDE_50DPI;		// 透過赤外マスクパターンの割る数
	cs_chk.ptir_mask_ptn = MASK_PAT_50DPI;						// 透過赤外マスクパターン1のポインタ

	cs_chk.rba_custom_check_value = get_rba_custom_check_error(0, &cs_chk, pbs->mid_res_nn.max_node_num/4, pbs->insertion_direction);
	switch(cs_chk.rba_custom_check_value)
	{
	case CUSTOM_CHECK_RESULT_DBL_NOTE:
		ex_validation.block_compare_flag |= BIT_DOUBLE_NOTE_CUSTOM;
		result = 1;
		break;
	case CUSTOM_CHECK_RESULT_FAKE_NOTE:
		ex_validation.block_compare_flag |= BIT_COUNTER_FAIT_CUSTOM;
		result = 1;
		break;
	case CUSTOM_CHECK_RESULT_GENUINE_NOTE:
	default:
		break;
	}
#endif

#if POINT_CHECK_ENABLE
	pt_chk.note_size_x = pbs->note_x_size;
	pt_chk.note_size_y = pbs->note_y_size;

	pt_chk.ir2_mask_ptn_diameter_x = DIAMETER_100DPI_X;			// 赤外2マスクパターンの直径x
	pt_chk.ir2_mask_ptn_diameter_y = DIAMETER_100DPI_Y;			// 赤外2マスクパターンの直径y
	pt_chk.ir2_mask_ptn_divide_num = DIVIDE_100DPI;		// 赤外2マスクパターンの割る数
	pt_chk.pir2_mask_ptn = MASK_PAT_100DPI;						// 赤外2マスクパターン1のポインタ

	pt_chk.point_check_value = 0;

	pt_chk.point_check_value = get_point_check_error(0, &pt_chk, pbs->mid_res_nn.max_node_num/4, pbs->insertion_direction);
	if(pt_chk.point_check_value > 0)
	{
		ex_validation.block_compare_flag |= BIT_COUNTER_FAIT_POINT;
		result = 1;
	}
#endif

#if REF_CHECK_ENABLE
	rf_chk.note_size_x = pbs->note_x_size;
	rf_chk.note_size_y = pbs->note_y_size;

	rf_chk.ref_check_min = 0;
	rf_chk.ref_check_value = 0;

	rf_chk.ref_check_value = get_ref_check_error(0, &rf_chk, pbs->mid_res_nn.max_node_num/4, pbs->insertion_direction);
	if(rf_chk.ref_check_value > 0)
	{
		ex_validation.block_compare_flag |= BIT_COUNTER_FAIT_REF;
		result = 1;
	}
#endif

#if PEN_CHECK_ENABLE
	pen_chk.note_size_x = pbs->note_x_size;
	pen_chk.note_size_y = pbs->note_y_size;

	pen_chk.threshold = 5;
	pen_chk.pen_check_value = 0;

	pen_chk.pen_check_value = get_pen_check_error(0, &pen_chk, pbs->mid_res_nn.max_node_num/4, pbs->insertion_direction);
	if(pen_chk.pen_check_value > 0)
	{
		ex_validation.block_compare_flag |= BIT_COUNTER_FAIT_PEN;
		result = 1;
	}
#endif

#if POINT_UV_CHECK_ENABLE
	if(ex_uba710 == 1)
	{
		create_uv_wave(0, pbs->insertion_direction);
		point_uv_chk_invalid_count = get_point_uv_invalid_count(0, pbs->mid_res_nn.max_node_num/4, pbs->insertion_direction);
		if(point_uv_chk_invalid_count > 0)
		{
			ex_validation.block_compare_flag |= BIT_COUNTER_FAIT_POINT_UV;
			result = 1;
		}
	}
#endif

#if POINT_MAG_CHECK_ENABLE
	if(ex_uba710 == 1)
	{
		create_mag_wave(0, pbs->insertion_direction);
		point_mag_chk_invalid_count = mag_amount_chk_invalid_count(0, pbs->mid_res_nn.max_node_num/4, pbs->insertion_direction);
		if(point_mag_chk_invalid_count > 0)
		{
			ex_validation.block_compare_flag |= BIT_COUNTER_FAIT_POINT_MAG;
			result = 1;
		}
	}
#endif
	//return 0;
	return result;
}
//券種ごとの札長をチェックします。
s8  nn_res_length_check_exit(u8 buf_num)
{
	s8 result = 0;
#if 0 //delete iVIZION2
	extern const u32 length_limits[];
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	const u32 reverse_length_over_margin = 2;
	const u32 reverse_length_less_margin = 2;

#if 1
	float note_x = 0;
	float length_exit = 0;
	float length_pbo = 0;
	float length_pbi = 0;
	float length_slip_pbo = 0;
	float length_slip_pbi = 0;
#else
	u16 note_x = 0;
	u16 length_exit = 0;
	u16 length_pbo = 0;
	u16 length_pbi = 0;
	s16 length_slip_pbo = 0;
	s16 length_slip_pbi = 0;
#endif

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == LE)  //SEならば
	{
		// not support
	}
	else
	{
		if(SENSOR_EXIT)
		{
			// Exit Sensor ON:Over165.8mm
			note_x = (DIST_CEN_TO_EXI - DIST_CEN_TO_PAP);
		}
		else
		{
			length_exit = (ex_position_pulse_count.exit.end - ex_position_pulse_count.exit.start) * PITCH;
			length_pbo = (ex_position_pulse_count.apb_out.end - ex_position_pulse_count.apb_out.start) * PITCH;
			length_pbi = (ex_position_pulse_count.apb_in.end - ex_position_pulse_count.apb_in.start) * PITCH;
			length_slip_pbo = (DIST_ENT_TO_EXI - DIST_ENT_TO_PBO) - ((ex_position_pulse_count.apb_out.start - ex_position_pulse_count.exit.start) * PITCH);
			length_slip_pbi = (DIST_ENT_TO_EXI - DIST_ENT_TO_PBI) - ((ex_position_pulse_count.apb_in.start - ex_position_pulse_count.exit.start) * PITCH);
			note_x = length_exit + length_slip_pbo;
		}
	}

	if(((length_limits[pbs->mid_res_nn.max_node_num/4] + reverse_length_over_margin) < roundf(note_x)))
	{
		result = NN_CHECK_LONG_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else if(((length_limits[pbs->mid_res_nn.max_node_num/4] - reverse_length_less_margin) > roundf(note_x)))
	{
		result = NN_CHECK_SHORT_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else
	{
		result = NN_CHECK_OK;	//サイズ異常なし
	}
#endif
	return result;
}//nn_res_length_check_exit end
//券種ごとの札長をチェックします。
s8  nn_res_length_check_pbi(u8 buf_num)
{
	s8 result = 0;
#if 0 //delete iVIZION2
	extern const u32 length_limits[];
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	const float reverse_length_over_margin = 2.0;
	const float reverse_length_less_margin = 2.0;

	float note_x = 0;

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == LE)  //SEならば
	{
		// not support
	}
	else
	{
		note_x = (float)((ex_position_pulse_count.apb_in.end - ex_position_pulse_count.apb_in.start) * PITCH);
	}

	if(((length_limits[pbs->mid_res_nn.max_node_num/4] + reverse_length_over_margin) < note_x))
	{
		result = NN_CHECK_LONG_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else if(((length_limits[pbs->mid_res_nn.max_node_num/4] - reverse_length_less_margin) > note_x))
	{
		result = NN_CHECK_SHORT_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else
	{
		result = NN_CHECK_OK;	//サイズ異常なし
	}
#endif
	return result;
}//nn_res_length_check_pbi end

/* 外形検知 */
s16 reverse_edge_detect(u8 buf_num)
{
	ST_BS *pbs = work[buf_num].pbs;

	/*初期化*/
	pbs->angle		= 0;						// 傾き (tan_th *4096)
	pbs->center_x	= 0;// 中心座標x
	pbs->center_y	= 0;// 中心座標y
	pbs->note_y_size	= 0;// 札幅
	pbs->note_x_size	= 0;// 札長

	// 外形検知のデバッグためのパラメタ仮設定
	rev_ED.Plane = USE_EDGE_DETECT_SENSOR;			 //用いるプレーン指定
	rev_ED.Main_scan_max_val = pbs->PlaneInfo[rev_ED.Plane].main_effective_range_max;	// イメージデータの主走査方向ドット数
	rev_ED.Main_scan_min_val = pbs->PlaneInfo[rev_ED.Plane].main_effective_range_min;	// イメージデータの主走査方向ドット数
	rev_ED.Sub_scan_max_val = (((pbs->Blocksize / pbs->PlaneInfo[rev_ED.Plane].sub_sampling_pitch) * pbs->block_count)) + 50;	// 副走査方向の最大有効範囲
	rev_ED.Sub_scan_min_val = 0;
	rev_ED.MinSE = ST_EDGE_DETECT_MINSE;			// 札のSE側の最小許容値 (mm)※暫定値
	rev_ED.MaxSE = ST_EDGE_DETECT_MAXSE;			// 札のSE側の最大許容値 (mm)※暫定値
	rev_ED.MinLE = ST_EDGE_DETECT_MINLE;			// 札のLE側の最小許容値 (mm)※暫定値
	rev_ED.MaxLE = ST_EDGE_DETECT_MAXLE;			// 札のLE側の最大許容値 (mm)※暫定値
	rev_ED.Step_Movement = ST_EDGE_DETECT_STEP_MOVE;// 探索のステップ幅(mm)	 ※暫定値
	rev_ED.Small_Backtrack = ST_EDGE_DETECT_SMALL_BACKTRACK;//次の探索開始位置への戻り幅(dot)	※暫定値

	rev_ED.th_dynamic_decision_flg = 1;	//閾値を動的に決定するかのフラグ　0：オフ　1：オン

	//-----------------------------------

	rev_ED.edge_err_code = (u16)OPdet_Start_DC(buf_num);		// 外形検出ルーチン.

	//------------------------------------
	if(rev_ED.edge_err_code != 0)	/*外形検知失敗*/
	{
		// 結果確認
		pbs->angle		= 0xFFFF;		// 傾き
		pbs->center_x	= 0xFFFF;		// 中心座標x
		pbs->center_y	= 0xFFFF;		// 中心座標y
		pbs->note_y_size = 0xFFFF;		// 札幅
		pbs->note_x_size = 0xFFFF;		// 札長
		cal_note_parameter(buf_num);

		return rev_ED.edge_err_code;
	}

	// 結果確認
	pbs->angle		= rev_ED.SKEW;						// 傾き (tan_th *4096)
	pbs->center_x	= rev_ED.AlignedBillCenterPoint.x;	// 中心座標x
	pbs->center_y	= rev_ED.AlignedBillCenterPoint.y;	// 中心座標y
	pbs->note_y_size = rev_ED.width_dot;					// 札幅
	pbs->note_x_size = rev_ED.length_dot;				// 札長

	//座標変換に必要なパラメタを計算。
	cal_note_parameter(buf_num);
	//advance_insert_correction(buf_num);

	/*頂点座標を200dpi座標系に変換*/
	/*左上*/
	work[buf_num].pbs->right_up_x = (s16)rev_ED.AlignedUpperRight.x;
	work[buf_num].pbs->right_up_y = (s16)rev_ED.AlignedUpperRight.y;

	/*左下*/
	work[buf_num].pbs->right_down_x = (s16)rev_ED.AlignedLowerRight.x;
	work[buf_num].pbs->right_down_y = (s16)rev_ED.AlignedLowerRight.y;

	/*右上*/
	work[buf_num].pbs->left_up_x = (s16)rev_ED.AlignedUpperLeft.x;
	work[buf_num].pbs->left_up_y = (s16)rev_ED.AlignedUpperLeft.y;

	//右下
	work[buf_num].pbs->left_down_x = (s16)rev_ED.AlignedLowerLeft.x;
	work[buf_num].pbs->left_down_y = (s16)rev_ED.AlignedLowerLeft.y;

	return 0;
}

/*****************************************************************************/
/**
* Set reverse direction parameter.
*
* @param	ST_BS *pbs : サンプリングデータのポインタ  (work[buf_num].pbs)
*
* @return	None
*
* @note		払出前/後のパラメータセット(スキャン方向、オフセットの逆転）
*
******************************************************************************/
/*ヘッダー情報設定関数*/
void reverse_parameter_set(u8 buf_num, u8 is_reverse)
{
	ST_BS *pbs;
	u8 plane_num = 0;	//プレーン番号

#if (_DEBUG_CIS_MULTI_IMAGE==1)
	work[buf_num].pbs = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	work[buf_num].pbs = (ST_BS *)BILL_NOTE_IMAGE_TOP;
#endif
	pbs = work[buf_num].pbs;

	if(is_reverse == 1)
	{
		// outside
		plane_num = UP_R_R;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_UP_R_R;
		plane_num = UP_R_G;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_UP_R_G;
		plane_num = UP_R_IR1;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_UP_R_IR1;
		plane_num = UP_R_IR2;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_UP_R_IR2;
		// inside
		plane_num = DOWN_R_R;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_DOWN_R_R;
		plane_num = DOWN_R_G;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_DOWN_R_G;
		plane_num = DOWN_R_IR1;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_DOWN_R_IR1;
		plane_num = DOWN_R_IR2;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_DOWN_R_IR2;
		// inside
		plane_num = DOWN_T_R;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_DOWN_T_R;
		plane_num = DOWN_T_G;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = -PLANE_SUB_OFFSET_DOWN_T_G;
		plane_num = DOWN_T_IR1;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_POSITIVE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_T_IR1;
	}
	else
	{
		// outside
		plane_num = UP_R_R;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_UP_R_R;
		plane_num = UP_R_G;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_UP_R_G;
		plane_num = UP_R_IR1;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_UP_R_IR1;
		plane_num = UP_R_IR2;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_UP_R_IR2;
		// inside
		plane_num = DOWN_R_R;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_R_R;
		plane_num = DOWN_R_G;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_R_G;
		plane_num = DOWN_R_IR1;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_R_IR1;
		plane_num = DOWN_R_IR2;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_R_IR2;
		// inside
		plane_num = DOWN_T_R;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_T_R;
		plane_num = DOWN_T_G;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_T_G;
		plane_num = DOWN_T_IR1;
		pbs->PlaneInfo[plane_num].note_scan_dir = PLANE_NOTE_SCAN_DIR_REVERSE;
		pbs->PlaneInfo[plane_num].sub_offset = PLANE_SUB_OFFSET_DOWN_T_IR1;
	}
}

/* 逆識別 */
int reverse_compare(void)
{
	// TODO:逆識別処理実装
	s16 result = 0;
#if 0 //delete iVIZION2
	s8 out_nn = 0;
	ST_BS *pbs = work[0].pbs;

#if BV_UNIT_TYPE >= CS_MODEL
	if (get_dipsw2() & DIPSW2_ENABLE_RBA40C_NEW_FEATURES)
#else
	if (ex_dipsw1 & DIPSW2_ENABLE_RBA40C_NEW_FEATURES)
#endif
	{
		result = reverse_edge_detect(0);
	}
	else
	{
		reverse_parameter_set(0,1);
		result = edge_detect(0);
		reverse_parameter_set(0,0);
	}

	if(result == 0)//外形検知おｋ
	{
	#if BV_UNIT_TYPE >= CS_MODEL
		if (get_dipsw2() & DIPSW2_ENABLE_RBA40C_NEW_FEATURES)
	#else
		if (ex_dipsw1 & DIPSW2_ENABLE_RBA40C_NEW_FEATURES)
	#endif
		{
			for(int cnt = 0; cnt < 16; cnt++)
			{
				pbs->mid_res_nn.max_node_num = cnt*4;
				out_nn = nn_res_width_check(0); //幅チェック
				if(out_nn == NN_CHECK_OK)	//サイズおk
				{
					out_nn = nn_res_length_check_exit(0); //長さチェック
					if(out_nn == NN_CHECK_OK)	//サイズおk
					{
						ex_validation.denomi = pbs->mid_res_nn.max_node_num/4;
						break;
					}
				}

				if(cnt == 15)
				{
					//サイズ不一致
					ex_validation.reject_code = REJECT_CODE_PRECOMP;
				}
			}
		}
		else
		{
#if 1
			result = compare();
			if(result == 1)
			{
				ex_validation.reject_code = REJECT_CODE_PRECOMP;
			}
			/*** ex_validation copy ***/
			//ex_validation.validate_result.nn1_genuine_out_put_val = nn_res.output_max_val;
			//ex_validation.validate_result.nn2_genuine_out_put_val = nn_res.output_2nd_val;
#else
			for(int cnt = 0; cnt < 16; cnt++)
			{
				nn_res.note_id = cnt;
				out_nn = nn_res_width_check(0); //幅チェック
				if(out_nn == NN_CHECK_OK)	//サイズおk
				{
#if BV_UNIT_TYPE >= CS_MODEL
					out_nn = nn_res_length_check(0); //サイズチェック
#else
					// 暫定：CISライン抜けの為、センサーによる長さチェック
					out_nn = nn_res_length_check_pbi(0); //長さチェック
#endif
					if(out_nn == NN_CHECK_OK)	//サイズおk
					{
						ex_validation.denomi = nn_res.note_id;
						break;
					}
				}

				if(cnt == 15)
				{
					//サイズ不一致
					ex_validation.reject_code = REJECT_CODE_PRECOMP;
				}
			}
#endif
		}
	}
	else
	{
		//外形検知失敗
		ex_validation.reject_code = REJECT_CODE_SKEW;
	}
#endif
	return 0;
}

//券種ごとのサイズをチェックします。
s8  nn_res_size_check(u8 buf_num)
{
	s8 result = 0;
	ST_BS *pbs = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[buf_num];

	u16 note_x = 0;
	u16 note_y = 0;

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == LE)  //SEならば
	{
		note_x = pbs->note_x_size * PLANE_SUB_ELEMENT_PICH_200DPI + 0.5;
		note_y = pbs->note_y_size * CIS_MAIN_PITCH + 0.5;
	}
	else
	{
		note_y = pbs->note_x_size * CIS_MAIN_PITCH + 0.5;
		note_x = pbs->note_y_size * PLANE_SUB_ELEMENT_PICH_200DPI + 0.5;
	}

	if(((length_limits[pbs->mid_res_nn.max_node_num/4] + length_over_margin) < note_x)
	|| ((width_limits[pbs->mid_res_nn.max_node_num/4] + width_over_margin) < note_y))
	{
		result = NN_CHECK_LONG_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else if(((length_limits[pbs->mid_res_nn.max_node_num/4] - length_less_margin) > note_x)
			||((width_limits[pbs->mid_res_nn.max_node_num/4] - width_less_margin) > note_y))
	{
		result = NN_CHECK_SHORT_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else
	{
		result = NN_CHECK_OK;	//サイズ異常なし
	}

	return result;

}//nn_res_size_check end


//券種ごとの幅をチェックします。
s8  nn_res_width_check(u8 buf_num)
{
	s8 result = 0;
#ifndef SIMURATION
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	const u32 reverse_width_over_margin = 1;
	const u32 reverse_width_less_margin = 1;

	float note_y = 0;

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == LE)  //SEならば
	{
		note_y = pbs->note_y_size * CIS_MAIN_PITCH + 0.5;
	}
	else
	{
		note_y = pbs->note_x_size * CIS_MAIN_PITCH + 0.5;
	}

	if((width_limits[pbs->mid_res_nn.max_node_num/4] + reverse_width_over_margin) < roundf(note_y))
	{
		result = NN_CHECK_LONG_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else if((width_limits[pbs->mid_res_nn.max_node_num/4] - reverse_width_less_margin) > roundf(note_y))
	{
		result = NN_CHECK_SHORT_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else
	{
		result = NN_CHECK_OK;	//サイズ異常なし
	}
#endif
	return result;

}//nn_res_width_check end

//券種ごとの長さをチェックします。
s8  nn_res_length_check(u8 buf_num)
{
	s8 result = 0;
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	const u32 reverse_length_over_margin = 3;
	const u32 reverse_length_less_margin = 3;

	u16 note_x = 0;

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == LE)  //SEならば
	{
		note_x = pbs->note_x_size * PLANE_SUB_ELEMENT_PICH_200DPI + 0.5;
	}
	else
	{
		note_x = pbs->note_y_size * PLANE_SUB_ELEMENT_PICH_200DPI + 0.5;
	}

	if((length_limits[pbs->mid_res_nn.max_node_num/4] + reverse_length_over_margin) < note_x)
	{
		result = NN_CHECK_LONG_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else if((length_limits[pbs->mid_res_nn.max_node_num/4] - reverse_length_less_margin) > note_x)
	{
		result = NN_CHECK_SHORT_ERROR;	//サイズ不一致としてその他の券とみなします
	}
	else
	{
		result = NN_CHECK_OK;	//サイズ異常なし
	}

	return result;

}//nn_res_length_check end

/*********************************************************************//**
 * @brief edge detected result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_edge_detected(s16 result)
{
#if SKEW_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if((pbs->angle == 0xFFFF)
	|| (result == BIT_EDGE_DETECT))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief compare result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_compare_detected(s16 result)
{
#if COMPARE_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if((pbs->mid_res_nn.max_node_num > BAR_INDX)
	|| (result == ERR_FAILURE_NN_TH1)
	|| (result == ERR_FAILURE_NN_TH2)
	|| (result == ERR_FAILURE_NN_TH3)
	|| (result == ERR_FAILURE_NN_TH5))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief short note result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_short_note_detected(s16 result)
{
#if SIZE_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	u32 xsize = pbs->note_y_size * PLANE_SUB_ELEMENT_PICH_200DPI;
	u32 ysize = pbs->note_x_size * PLANE_MAIN_ELEMENT_PICH;

	if((xsize <= (length_limits[pbs->mid_res_nn.max_node_num/4] - length_less_margin))
	|| (ysize <= (width_limits[pbs->mid_res_nn.max_node_num/4] - width_less_margin)))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief long note result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_long_note_detected(s16 result)
{
#if SIZE_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	u32 xsize = pbs->note_y_size * PLANE_SUB_ELEMENT_PICH_200DPI;
	u32 ysize = pbs->note_x_size * PLANE_MAIN_ELEMENT_PICH;

	if((xsize >= (length_limits[pbs->mid_res_nn.max_node_num/4] + length_over_margin))
	|| (ysize >= (width_limits[pbs->mid_res_nn.max_node_num/4] + length_over_margin)))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief sync result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_sync_detected(s16 result)
{
	return 0;
}
/*********************************************************************//**
 * @brief dye note result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_dye_note_detected(void)
{
#if DYE_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if((pbs->fitness[FITNESS_DE_INKED_NOTE].bit.result != ATM) && (pbs->fitness[FITNESS_DE_INKED_NOTE].bit.result != FIT))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief dog ear result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_dog_ear_detected(void)
{
#if DOG_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if((pbs->fitness[FITNESS_DOGEARS].bit.result != ATM) && (pbs->fitness[FITNESS_DOGEARS].bit.result != FIT))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief category counterfeit result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_category_counterfeit_detected(void)
{
#if CATEGORY_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if(pbs->category_ecb == CATEGORY_ECB_COUNTERFEIT)
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief category unfit result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_category_unfit_detected(void)
{
#if CATEGORY_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if(pbs->category_ecb == CATEGORY_ECB_UNFIT)
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief category not support result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_category_not_support_detected(void)
{
#if CATEGORY_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if(pbs->category_ecb == CATEGORY_ECB_NOT_SUPPORT)
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief tear result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_tear_detected(void)
{
#if TRAR_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if((pbs->fitness[FITNESS_TEARS].bit.result != ATM)
	&& (pbs->fitness[FITNESS_TEARS].bit.result != FIT))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief hole result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_hole_detected(void)
{
#if HOLE_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if((pbs->fitness[FITNESS_HOLES].bit.result != ATM) && (pbs->fitness[FITNESS_HOLES].bit.result != FIT))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief double note result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_double_note_detected(void)
{
#if DOUBLE_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if((pbs->mid_res_double_ck.result != ATM) && (pbs->mid_res_double_ck.result != FIT))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief fake mcir result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_fake_mcir_detected(void)
{
#if MCIR_CHECK_ENABLE
	ST_BS* pbs = (ST_BS*)work[0].pbs;
	if(((pbs->mid_res_mcir.result != ATM) && (pbs->mid_res_mcir.result != FIT))
	|| (pbs->mid_res_mcir.invalid_count != 0))
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief counterfeit result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_counterfeit_detected(void)
{
	ST_BS* pbs = (ST_BS*)work[0].pbs;
#if NN_1COLOR_CHECK_ENABLE
	if((pbs->mid_res_nn1.result != ATM) && (pbs->mid_res_nn1.result != FIT))
	{
		return 1;
	}
#endif
#if NN_2COLOR_CHECK_ENABLE
	if((pbs->mid_res_nn2.result != ATM) && (pbs->mid_res_nn2.result != FIT))
	{
		return 1;
	}
#endif
#if THREAD_CHECK_ENABLE
	if((pbs->fitness[FITNESS_MISSING_METAL_THREAD].bit.result != ATM) && (pbs->fitness[FITNESS_MISSING_METAL_THREAD].bit.result != FIT))
	{
		return 1;
	}
#endif
#if MISSING_SECURITY_ENABLE
	if((pbs->fitness[FITNESS_MISSING_SECURITY_FEATURE].bit.result != ATM) && (pbs->fitness[FITNESS_MISSING_SECURITY_FEATURE].bit.result != FIT))
	{
		return 1;
	}
#endif
#if CUSTOM_CHECK_ENABLE
	if(ex_validation.block_compare_flag & BIT_COUNTER_FAIT_CUSTOM)
	{
		return 1;
	}
#endif
#if POINT_CHECK_ENABLE
	if(ex_validation.block_compare_flag & BIT_COUNTER_FAIT_POINT)	// 2020-12-18 add
	{
		return 1;
	}
#endif
#if REF_CHECK_ENABLE
	if(ex_validation.block_compare_flag & BIT_COUNTER_FAIT_REF)	// 2020-12-21 add
	{
		return 1;
	}
#endif
#if PEN_CHECK_ENABLE
	if(ex_validation.block_compare_flag & BIT_COUNTER_FAIT_PEN)	// 2020-12-22 add
	{
		return 1;
	}
#endif
#if OTHER_CHECK_ENABLE
	if(ex_validation.block_compare_flag & BIT_COUNTER_FAIT_OTHER)	// 2020-12-18 add
	{
		return 1;
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief uv result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_uv_detected(void)
{
#if POINT_UV_CHECK_ENABLE
	if(ex_uba710 == 1)
	{
		if(ex_validation.block_compare_flag & BIT_COUNTER_FAIT_POINT_UV)
		{
			return 1;
		}
	}
#endif
	return 0;
}
/*********************************************************************//**
 * @brief mag result check
 *
 * @param[in]	None
 * @return 		true : error,  false : no error
 **********************************************************************/
int is_mag_detected(void)
{
#if POINT_MAG_CHECK_ENABLE
	if(ex_uba710 == 1)
	{
		if(ex_validation.block_compare_flag & BIT_COUNTER_FAIT_POINT_MAG)
		{
			return 1;
		}
	}
#endif
	return 0;
}
/* テープ検知 */
#if 1
#define TAPE_DETECT_BUFF_LENGTH 600
#define TAPE_DETECT_VALUE_LIMIT 20
#define TAPE_DETECT_UNUSE_AREA 40
#define TAPE_DETECT_COUNT_LIMIT 16
#define DEBUG_SKIP_TAPE	1
u8 tape_detect_buffer[TAPE_DETECT_BUFF_LENGTH];
int is_tape_detected(void)
{
#if DEBUG_SKIP_TAPE
	return false;
#else
	// 札先端、後端テープ検知処理実装
	u8 buf_num = 0;
	ST_BS *pbs = work[buf_num].pbs;
	// 上下CISのオフセット調整機能による変更(NEW_FPGA)
	// use DOWN_R_R
	// point X:4,8,12,16,20,~end
	//       Y:70-669
	const u32 pos = 70;
	u32 size = pbs->block_count;		//a 有効ブロック数
	u8 *p_Data;
	u32 line;
	u32 i,j;
	u32 detect_count = 0;
	u32 detect_length = 0;
	u32 detect_length_max = 0;
	s16 diff;
	u32 start_y, end_y;
	u32 time_start;
	u32 time_end;

//	time_start = OSW_TIM_value();

	start_y = (pbs->center_y - (pbs->note_y_size/2)) * pbs->pitch_ratio;
	end_y = ((pbs->center_y + (pbs->note_y_size/2)) * pbs->pitch_ratio) ;
	// data buffer
	p_Data = (unsigned char *)&pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Offset + pbs->PlaneInfo[UP_R_R].Address_Period * 1 + pos];
	for (i = 0; i < TAPE_DETECT_BUFF_LENGTH; i++)
	{
		tape_detect_buffer[i] = *(p_Data + i);
	}
	// 先端
	for (line = 1; line < 5 ; line++)
	{
		detect_length = 0;
		if((line+1)*4 < start_y - TAPE_DETECT_UNUSE_AREA)
		{
			p_Data = (unsigned char *)&pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period * ((line + 1)*4 + 0) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos];
			for (i = 0; i < TAPE_DETECT_BUFF_LENGTH; i++)
			{
				diff = tape_detect_buffer[i] - *(p_Data + i);
				if(abs(diff) > TAPE_DETECT_VALUE_LIMIT)
				{
					detect_count++;
					detect_length++;
					for (j = 1; j < 4; j++)
					{
						if(j == 1)
							// block1
							diff = tape_detect_buffer[i] - pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period *  ((line + 1)*4 + 1) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos + i];
						else if(j == 2)
							// block2
							diff = tape_detect_buffer[i] - pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period *  ((line + 1)*4 + 2) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos + i];
						else if(j == 3)
							// block3
							diff = tape_detect_buffer[i] - pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period *  ((line + 1)*4 + 3) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos + i];
						if(abs(diff) > TAPE_DETECT_VALUE_LIMIT)
						{
							detect_count++;
							detect_length++;
						}
						else
						{
							break;
						}
					}
					if(detect_length > detect_length_max)
					{
						detect_length_max = detect_length;
					}
				}
				else
				{
					detect_length = 0;
				}
			}
		}
		if(detect_length_max > TAPE_DETECT_COUNT_LIMIT)
		{
			break;
		}
	}
	// 後端
	for (line = (end_y+TAPE_DETECT_UNUSE_AREA)/4 + 1; line < size - 4; line++)
	{
		detect_length = 0;
		p_Data = (unsigned char *)&pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period * (line*4) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos];
		for (i = 0; i < TAPE_DETECT_BUFF_LENGTH; i++)
		{
			diff = tape_detect_buffer[i] - *(p_Data + i);
			if(abs(diff) > TAPE_DETECT_VALUE_LIMIT)
			{
				detect_count++;
				detect_length++;
				for (j = 1; j < 4; j++)
				{
					if(j == 1)
						// block1
						diff = tape_detect_buffer[i] - pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period * (line*4 + 1) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos];
					else if(j == 2)
						// block2
						diff = tape_detect_buffer[i] - pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period * (line*4 + 2) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos];
					else if(j == 3)
						// block3
						diff = tape_detect_buffer[i] - pbs->sens_dt[pbs->PlaneInfo[UP_R_R].Address_Period * (line*4 + 3) + pbs->PlaneInfo[UP_R_R].Address_Offset + pos];
					if(abs(diff) > TAPE_DETECT_VALUE_LIMIT)
					{
						detect_count++;
						detect_length++;
					}
					else
					{
						break;
					}
				}
				if(detect_length > detect_length_max)
				{
					detect_length_max = detect_length;
				}
			}
			else
			{
				detect_length = 0;
			}
		}
		if(detect_length_max > TAPE_DETECT_COUNT_LIMIT)
		{
			break;
		}
	}
//	time_end = OSW_TIM_value();
	if(detect_length_max > TAPE_DETECT_COUNT_LIMIT)
	{
		return true;
	}

	return false;
#endif
}
#endif


/*********************************************************************//**
 * @brief set reject code
 *
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void set_reject_code(s16 result)
{
	if(is_edge_detected(result))
	{
		ex_validation.reject_code = REJECT_CODE_SKEW;
	}
	else if(is_tape_detected())
	{
		/* 2017-06-12 adp RBA-40 possible manipulation */
		/* reject banknote if tape detected */
		ex_validation.reject_code = REJECT_CODE_LENGTH;
	}
	else if(is_dye_note_detected())
	{
		ex_validation.reject_code = REJECT_CODE_DYENOTE;
	}
	else if(is_compare_detected(result))
	{
		ex_validation.reject_code = REJECT_CODE_PRECOMP;
	}
	else if(is_short_note_detected(result))
	{
		ex_validation.reject_code = REJECT_CODE_PAPER_SHORT;
	}
	else if(is_long_note_detected(result))
	{
		ex_validation.reject_code = REJECT_CODE_PAPER_LONG;
	}
	else if(is_sync_detected(result))
	{
		ex_validation.reject_code = REJECT_CODE_SYNC;
	}
	else if(is_tear_detected())
	{
		ex_validation.reject_code = REJECT_CODE_DOG_EAR;
		//ex_validation.reject_code = REJECT_CODE_TEAR;
	}
	else if(is_dog_ear_detected())
	{
		ex_validation.reject_code = REJECT_CODE_DOG_EAR;
	}
	else if(is_hole_detected())
	{
		ex_validation.reject_code = REJECT_CODE_HOLE;
	}
	else if(is_double_note_detected())
	{
		ex_validation.reject_code = REJECT_CODE_PHOTO_LEVEL;
	}
	else if(is_fake_mcir_detected())
	{
		ex_validation.reject_code = REJECT_CODE_FAKE_MCIR;
	}
	else if(is_counterfeit_detected())
	{
		ex_validation.reject_code = REJECT_CODE_COUNTERFEIT;
	}
	else if(is_uv_detected())
	{
		ex_validation.reject_code = REJECT_CODE_UV;
	}
	else if(is_mag_detected())
	{
		ex_validation.reject_code = REJECT_CODE_MAG_AMOUNT;
	}
	else if(is_category_not_support_detected())
	{
		ex_validation.reject_code = REJECT_CODE_PRECOMP;
	}
	else if(is_category_unfit_detected())
	{
		ex_validation.reject_code = REJECT_CODE_DOG_EAR;
	}
	else if(is_category_counterfeit_detected())
	{
		ex_validation.reject_code = REJECT_CODE_COUNTERFEIT;
	}
	else if(is_clip_detected())
	{
		ex_validation.reject_code = REJECT_CODE_THREAD;
	}
	else if(result)
	{//その他はすべて識別エラーとする
		ex_validation.reject_code = REJECT_CODE_PRECOMP;
	}
}
#if 0
/****************************************************************/
/**
 * @brief 識別NN実行
 */
/****************************************************************/
s32 nn_identification(u8 buf_num)
{
	ST_BS *pbs = work[buf_num].pbs;
	s16 err_code = NN_CHECK_OK;


	float val = 0.0f;
	register u32 ii;
	s8 out_nn = 0;
	u8 planelist[4] =
	{
		OMOTE_R_R,
		OMOTE_R_G,
		URA_R_R,
		URA_R_G
	};//プレーンリストの例

	if(pbs->note_x_size == -1 || pbs->note_y_size == -1)
	{
		//外形失敗時
		pbs->tx_denomi_code = -3;
		pbs->insertion_direction = 0;
		ex_validation.compare_flag |= BIT_EDGE_DETECT;
		return NN_CHECK_EDGE_DETECT_ERROR;
	}

	//ポイント抽出パラメタ
	//*****************************************
	//識別用に等分割でポイント抽出を行う。
	work[buf_num].pbs->insertion_direction = W0;		//方向コード　w0固定
	data_extraction.outline = 0;						// 0:外形基準   1:印刷枠基準(未実装)
	data_extraction.split_x = 10;						//分割数ｘ
	data_extraction.split_y = 6;						//分割数ｙ
	data_extraction.corner_flg = 0;						//コーナーフラグ 0:使わない,  1:使う
	data_extraction.plane_count = sizeof(planelist);	// プレーンリストの有効要素数
	memcpy(data_extraction.plane_lst, planelist, sizeof(planelist));	// プレーンリスト

	//マスクパターンの総和を計算
	//ここでは200*200DPIのみなので1種類のマスクパターンの総和を求めている
	for(ii = 0; ii<sizeof(MASK_PAT_200DPI); ++ii)
	{
		val += MASK_PAT_200DPI[ii];	// 割る値
	}

	val = 1 / val;	//逆数計算

	//プレーンごとにマスク情報をセットする。
	//基本は赤緑の200*200DPIを使うので1つのマスクパターンでよい
	//それ以外のプレーンを使う場合はそれ用のマスクパターンを用意すること。
	for(ii = 0; ii < data_extraction.plane_count; ++ii)
	{
		data_extraction.filter_size_x[ii] = DIAMETER_200DPI_X;		//フィルターサイズ
		data_extraction.filter_size_y[ii] = DIAMETER_200DPI_Y;		//
		data_extraction.pfilter_pat[ii] = MASK_PAT_200DPI;//マスクパターンのポインタ設定
		data_extraction.divide_val[ii] = val;
	}

	err_code = Data_extraction(&data_extraction , buf_num);	/*抽出ポイント決定＆ポイント内計算実行*/
	err_code = 0;
	if(err_code != 0)
	{
		ex_validation.compare_flag |= BIT_CALC_MASH;
		return NN_CHECK_EXTRACTION_ERROR;
	}
	//識別nn
	nn_cfg.sizeflag = 0; //サイズデータを使用する場合 0：使用しない
	nn_cfg.biasflag = 0; //バイアス項 0：使用しない

	nn_cfg.in_node = data_extraction.count;	/*ポイント抽出時にカウントしたポイント数*/
	nn_cfg.center_node = last_id * 4;		/*NNツールから*/
	nn_cfg.out_nide = last_id * 4;			/*NNツールから　中間と同じ*/

	nn_cfg.pcenter_wit_offset = (float *)cen_weight_ary; //重み設定
	nn_cfg.pout_wit_offset = (float *)end_weight_ary;

	NN(); //識別NN
	pbs->tx_denomi_code = -1;
	pbs->insertion_direction = 0;

	//発火不良の調査
	nn_res.reject = out_put_value_cal();
	out_nn = nn_res.reject;

	if(out_nn == 0 && nn_res.output_max_node < nn_cfg.out_nide )	//正しくEURのノードが発火し場合
	{
		//出力ノード照らし合わせ
		node_comparison();
		out_nn = nn_res_size_check(buf_num); //サイズチェック

		if(out_nn == NN_CHECK_OK)	//size ok
		{
			/*サンプリングデータに書き込み*/
			pbs->tx_denomi_code = nn_res.note_id;
			pbs->insertion_direction = nn_res.way;
		}
		else if(out_nn == NN_CHECK_LONG_ERROR)		// size over
		{
			pbs->insertion_direction = way_end;	//方向セット とりあえずセット
			pbs->tx_denomi_code = out_nn;	//エラーコード書き込み
			nn_res.note_id = 0xff;
			ex_validation.compare_flag |= BIT_LENGTH_LONG;
		}
		else if(out_nn == NN_CHECK_SHORT_ERROR)		// size over
		{
			pbs->insertion_direction = way_end;	//方向セット とりあえずセット
			pbs->tx_denomi_code = out_nn;	//エラーコード書き込み
			nn_res.note_id = 0xff;
			ex_validation.compare_flag |= BIT_LENGTH_SHORT;
		}
	}
	else //発火不良もしくはその他の券とみなされた場合
	{
		pbs->insertion_direction = way_end;		//方向セット とりあえずセット
		pbs->tx_denomi_code = out_nn;		//エラーコード書き込み
		nn_res.note_id = 0xff;
		ex_validation.compare_flag |= BIT_COMPARE;
	}
	return out_nn;
}

#endif


#if 1	//2023-09-28
u8 check_mag_aging(void)
{
	u8 result;
	result = create_mag_wave_test(0, 0);

	return result;
}


#endif