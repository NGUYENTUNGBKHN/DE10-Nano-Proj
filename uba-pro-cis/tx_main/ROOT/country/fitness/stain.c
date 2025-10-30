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
* @file stain.c
* @brief 染み検知
* @date 2019// Created.
* @Version 1.2.0
* @updata
* https://app.box.com/s/vnghqv8qofu6u2s4gz6dtbcu2znv6hba
*/
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>

#define EXT
#include "../common/global.h"

EXTERN int debug_logi_view;	//トレースするかしないか
#define POINT_MAX_NUM 500		//配列に記録できる最大座標数 
#define STAIN_MAX_WHILE	945		//下記のwhileの最大回数	おおよそ30mmｘ30㎜まで対応	200dpi x 200dpi換算
//#define STAIN_MAX_WHILE	995		//下記のwhileの最大回数	おおよそ30mmｘ30㎜まで対応	200dpi x 200dpi換算


#define RECIP_PI 0.3183098f
#define NOT_SEARCH_POINT_MAX_NUM 35		//重複検知を避けるために登録できる座標数　1プレーンごとの数
#define SCAN_CONTER_THR_RATE 0.1f		//閾値を下げる割合
#define EXTRACTION_POINT_NUM_X 500		//抽出ポイントの座標を格納する。
#define EXTRACTION_POINT_NUM_Y 500		//抽出ポイントの座標を格納する。
#define STATE_LIMIT_X 3					//ステートを切り替える際のチェックスキャン回数
#define STATE_LIMIT_Y 3					//
#define SCAN_STEP_X 1					//スキャンステップ　固定
#define SCAN_STEP_Y 1					//
#define SCAN_ERR_LIMIT	10				//同じ方向へのスキャンがN回続いたらスキャン打ち切り


#define NEW_DIR

#define MAX_LIMIT_WHILE 5000

//#define STAIN_SCAN_PERFORMANCE_UP_TEST 1

//#define CSV_DB_STAIN


//******************************
/*
染み検知のメイン関数

	in	u8 buf_num ,	バッファ番号
	ST_STAIN *sta	専用構造体

	out u8 　　res_judge;				検知結果　0：無　1：有
	u16	　res_stain_err;			エラーコード
	float res_max_stain_area;		検知した染みで最も大きい染みの面積
	float res_max_stain_diameter;	検知した染みで最も大きい染みの直径
	float res_total_stain_area;		検知した染みの合計面積

*/
s16	stain(u8 buf_num, ST_STAIN* sta)
{
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	ST_SPOINT spt;
	ST_POINT_VICINITY pv;

	s16 conter_x[STAIN_MAX_WHILE];
	s16 conter_y[STAIN_MAX_WHILE];
	u16 conter_point_count = 0;
	s16 x = 0;			//現在の座標
	s16 y = 0;			//

	s16 extra_point_x[EXTRACTION_POINT_NUM_X];
	s16 extra_point_y[EXTRACTION_POINT_NUM_Y];

	u32 stain_area_thr = 0;
	u32 stain_total_area_thr = 0;
	u32 total_area = 0;
	u16 stain_diameter_thr = 0;

	s16 res_code = 0;

	u16	size_x = 0;		//紙幣サイズx
	u16	size_y = 0;		//
	u32 th_count = 0;		//debug用
	u8 plane_num = 0;	//プレーン数
	s16 th = 0;			//debug用
	u8 not_hole_flg = 0;
	float curent_diameter;	//求めた直径

	//u16 x_point_num = 0;	//分割したときのポイント数

	u16 i = 0;
	u8  j = 0;
	s32 center_x = 0;	//中心座標
	s32 center_y = 0;	//

	float area = 0;			//染みの面積
	float temp_area = 0;		//一時保存用染みの面積


	u16 max_x = 0;		//染み内の最大座標x
	u16 max_y = 0xffff;		//染み内の最大座標y
	u16 min_x = 0xffff;		//染み内の最小座標x
	u16 min_y = 0;		//染み内の最小座標y

	u32 center_check_x_right = 0;
	u16 center_check_count_x_right = 0;

	u32 center_check_x_left = 0;
	u16 center_check_count_x_left = 0;

	u32 center_check_y_up = 0;
	u16 center_check_count_y_up = 0;

	u32 center_check_y_down = 0;
	u16 center_check_count_y_down = 0;

	u8 skip = 0;		//重複エリア　0no　１yes

	u8 hole_scan_flg = 0; //穴チェックを行ったか
	u8 notscan_flg = 0;
	u8 notscan_flg2 = 0;

	s8 res = 0;

	//int testx ,testy;


	//重複スキャン防止用の座標記録構造体
	ST_STAIN_NOT_SCAN_AREA notscan[NOT_SEARCH_POINT_MAX_NUM];
	u8 not_scan_area_count = 0;

	//再スキャン関係
	u8 repeart_conter_scan;									//再スキャン回数
	float conter_thr_multi = SCAN_CONTER_THR_RATE;
	float multi_diff = 0;

#ifdef CSV_DB_STAIN
	FILE* fp;			//filedebug
	F_CREATE_1(fp);		//filedebug
#endif

	if (sta->repeat_num != 0)
	{
		multi_diff = conter_thr_multi / (sta->repeat_num);
	}

	if (pbs->LEorSE == SE)	//
	{
		sta->note_size_y = pbs->note_x_size;
		sta->note_size_x = pbs->note_y_size;
	}
	else
	{
		sta->note_size_x = pbs->note_x_size;
		sta->note_size_y = pbs->note_y_size;
	}

	//紙幣サイズを偶数にする。

	if (sta->note_size_x % 2 == 1)
	{
		size_x = (u16)((sta->note_size_x + 1) * 0.5f);
		size_y = (u16)((sta->note_size_y + 1) * 0.5f);
	}
	else
	{
		size_x = (u16)((sta->note_size_x) * 0.5f);
		size_y = (u16)((sta->note_size_y) * 0.5f);
	}

	sta->note_size_x_one_third = size_x / 2;
	sta->note_size_y_one_third = size_y / 2;
	sta->half_size_x = size_x;
	sta->half_size_y = size_y;

	//マスクパターンの設定
	pv.filter_size_y = sta->mask_size_y;		//x
	pv.filter_size_x = sta->mask_size_x;		//フィルターサイズy
	pv.pfilter_pat = sta->p_mask;				//マスクパターンのポインタ設定
	pv.divide_val = sta->divide_val;			//割る数の設定

	//外形からの除外エリアの設定
	sta->conter_not_scan_range = (u8)(sta->conter_not_scan_range / 0.127f);
	sta->not_scan_range_x_left_up = -size_x + sta->conter_not_scan_range;
	sta->not_scan_range_y_left_up = size_y - sta->conter_not_scan_range;
	sta->not_scan_range_x_right_down = size_x - sta->conter_not_scan_range;
	sta->not_scan_range_y_right_down = -size_y + sta->conter_not_scan_range;

	stain_ini_and_params_set(sta, pbs, sta->plane[0]);

	//採取間隔を計算
	spt.way = pbs->insertion_direction;
	sta->not_scan_margin_x = (u16)(((sta->note_size_x * 2) / sta->x_split) - (sta->note_size_x / sta->x_split));
	sta->not_scan_margin_y = (u16)(((sta->note_size_y * 2) / sta->y_split) - (sta->note_size_y / sta->y_split));


	//閾値　mmからdotに変換
	stain_area_thr = (u32)(sta->stain_size_thr / 0.127f / 0.127f);
	stain_total_area_thr = (u32)(sta->total_stain_thr / 0.127f / 0.127f);
	stain_diameter_thr = (u16)(sta->stain_diameter_thr / 0.127f);

	//ポイントリストと除外エリアの設定
	res = not_scan_area_dision(pbs, sta, pnote_param, extra_point_x, extra_point_y);
	if (res == -1)
	{
		sta->res_stain_err = ERR_WHILE_OVER;
		return 0;
	}

	//スキャン開始
	for (plane_num = 0; plane_num < sta->plane_num; ++plane_num)
	{
		//色情報をセット
		pv.plane = (u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]);
		spt.l_plane_tbl = (u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]);
		th_count = 0;
		sta->security_mode = 0;

		//IR1,2はセキュリティモードON
		//可視光とIRとで意味合いが異なる。
		//可視光の場合	：ターゲットの明るさの染みを検出する。その値以下の染みは検知できない。
		//IRシリーズ	：IRを吸光するある程度の染みを検知可能。
		if (pv.plane == OMOTE_R_IR1 || pv.plane == URA_R_IR1 ||
			pv.plane == OMOTE_R_IR2 || pv.plane == URA_R_IR2)
		{
			sta->security_mode = 1;
		}

		sta->main_scan_max_val = (work[buf_num].pbs->PlaneInfo[sta->plane[plane_num]].main_effective_range_max) - 1;	// イメージデータの主走査方向ドット数
		sta->main_scan_min_val = (work[buf_num].pbs->PlaneInfo[sta->plane[plane_num]].main_effective_range_min) + 1;	// イメージデータの主走査方向ドット数
		sta->sub_scan_max_val = ((((work[buf_num].pbs->Blocksize / work[buf_num].pbs->PlaneInfo[sta->plane[plane_num]].sub_sampling_pitch) * work[buf_num].pbs->block_count))) - 1;	// 副走査方向の最大有効範囲
		sta->sub_scan_min_val = 1;

		//重複座標リストをクリアする。
		for (i = 0; i < not_scan_area_count; i++)
		{
			notscan[i].left_up_x = 0;
			notscan[i].right_down_x = 0;
			notscan[i].left_up_y = 0;
			notscan[i].right_down_y = 0;
		}

		not_scan_area_count = 0;

		//dpi比を計算
		sta->raito = pbs->PlaneInfo[spt.l_plane_tbl].sub_sampling_pitch;
		sta->not_scan_margin_y = sta->not_scan_margin_x / sta->raito;

		if (sta->raito == 1)
		{
			sta->coef_raito = 5;
		}
		else if (sta->raito == 2)
		{
			sta->coef_raito = 4;
		}
		else if (sta->raito == 4)
		{
			sta->coef_raito = 3;
		}
		else if (sta->raito == 8)
		{
			sta->coef_raito = 2;
		}


		for (y = 0; y < sta->y_split - 1; y++)		//分割数y
		{
			for (x = 0; x < sta->x_split - 1; x++)	//分割数x
			{
				//閾値採取
				th = sta->thr[plane_num][th_count];

				th_count = th_count + 1;
				hole_scan_flg = 0;
				notscan_flg = 0;
				temp_area = 0;

				//閾値の値が低い
				if (th < sta->min_thr_val)
				{
					continue;
				}

				//外形範囲外か
				if (extra_point_y[y] == 0x0FFF || extra_point_x[x] == 0x0FFF)
				{
					continue;
				}

				//ポイント座標決定
				spt.x = (s32)extra_point_x[x];
				spt.y = (s32)extra_point_y[y];

				//座標変換
#ifdef VS_DEBUG_ITOOL
				for_iTool_trace_callback(&spt, pbs, pnote_param, pv.filter_size_x, pv.filter_size_y, pv.pfilter_pat, 0);
				spt.trace_flg = 0;
#endif
				new_L2P_Coordinate(&spt, pbs, pnote_param);

				//エリアの調査　すでにスキャン済みなら
				for (i = 0; i < not_scan_area_count; i++)
				{
					if ((notscan[i].left_up_x <= spt.x && notscan[i].right_down_x >= spt.x) &&
						(notscan[i].left_up_y <= spt.y && notscan[i].right_down_y >= spt.y))
					{
						skip = 1;
						break;
					}
				}

				//スキャン済みならスキップする
				if (skip == 1)
				{
					skip = 0;
					continue;
				}

				//除外エリアのスキャン確認plane_num
				for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
				{
					if ((sta->notscan2[plane_num][i].left_up_x <= spt.x && sta->notscan2[plane_num][i].right_down_x >= spt.x) &&
						(sta->notscan2[plane_num][i].left_up_y <= spt.y && sta->notscan2[plane_num][i].right_down_y >= spt.y))
					{
						skip = 1;

						//有効エリアの確認
						if (sta->not_scan_safe_area_count[plane_num][i] != 0)
						{
							for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
							{
								if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= spt.x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= spt.x) &&
									(sta->notscan_in_vali[plane_num][i][j].left_up_y <= spt.y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= spt.y))
								{
									skip = 0;
									break;
								}
							}
						}

						break;
					}
				}

				//除外エリアならスキップする
				if (skip == 1)
				{
					skip = 0;
					continue;
				}

				//for(i=0;i<3;++i)
				//{
				//	for(j=0;j<3;++j)
				//	{
				//		deb_para[0] = 1;		// function code
				//		deb_para[1] = testx + pnote_param->main_eff_range / 2 + i;	//
				//		deb_para[2] = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - testy +j;	//
				//		deb_para[3] = 1;		// plane
				//		callback(deb_para);		// debug
				//	}
				//}



				//マスクを用いての画素採取
				pv.x = (s16)spt.x;
				pv.y = (s16)spt.y;
				point_vicinity_cal(&pv, buf_num);

#ifdef CSV_DB_STAIN
				if (110 < th && plane_num == 1)
				{
					F_WRITE(fp, pv.output);	//filedebug
				}
#endif
				//閾値以下の場合染みありにする
				//下げた方がより濃い染みのみを検知する。
				th = th - (sta->thr_adjust_val - 0);
				//th = th + 20; 
				//th = th + 30; 
				//th = th + 15; 

				if (th < 0)
				{
					th = 0;
				}

				//ポイントの閾値と比較する
				if (pv.output < th)	//閾値比較
				{
					//輪郭スキャン時の閾値の倍率設定
					if (sta->security_mode == 1)
					{
						conter_thr_multi = SCAN_CONTER_THR_RATE;
						conter_thr_multi = conter_thr_multi + multi_diff;
						//th = th * 0.1f + pv.output * 0.9f;
					}
					else
					{
						th = (s16)(th * 0.1f + pv.output * 0.9f);
					}

					//再スキャンのループ　
					for (repeart_conter_scan = 0; repeart_conter_scan < sta->repeat_num + 1; repeart_conter_scan++)
					{

						//初期化する
						min_x = 0xffff;
						max_x = 0;
						min_y = 0;
						max_y = 0xffff;
						not_hole_flg = 0;
						notscan_flg2 = 0;
						conter_point_count = 0;
						center_x = 0;
						center_y = 0;
						area = 0;
						center_check_x_right = 0;
						center_check_count_x_right = 0;
						center_check_x_left = 0;
						center_check_count_x_left = 0;
						center_check_y_up = 0;
						center_check_count_y_up = 0;
						center_check_y_down = 0;
						center_check_count_y_down = 0;

						//穴かどうかの判定を行う。
						if (hole_scan_flg == 0)
						{
							not_hole_flg = (u8)hole_check(pv.x, pv.y, buf_num, spt.l_plane_tbl, sta);
						}

						if (not_hole_flg == 0)	//穴じゃなかったら
						{
							hole_scan_flg = 1;

							//輪郭検知閾値の倍率更新
							if (sta->security_mode == 1)
							{
								conter_thr_multi = conter_thr_multi - multi_diff;
							}
							else
							{
								conter_thr_multi = conter_thr_multi + multi_diff;
							}
							//輪郭線を検知する														  /*ここ変える*/
							res_code = contor_detect(sta, pbs, pnote_param, pv.x, pv.y, conter_x, conter_y, (u8)th, &conter_point_count, plane_num, conter_thr_multi, &center_x, &center_y, &max_x, &max_y, &min_x, &min_y);

							//検知エラーチェック
							if (res_code == STAIN_LEVEL_2)
							{
								//最大スキャン回数を超えた
								if (sta->security_mode == 1)
								{
									sta->thr_adjust_val += 3;	//白紙と染みを区別する閾値を調節する値

									if (sta->thr_adjust_val > 50)
									{
										sta->thr_adjust_val = 50;
									}
								}

								notscan_flg = 0;
								break;
							}

							//検知エラーチェック
							if (res_code == STAIN_LEVEL_3)
							{
								//除外エリアをスキャンした
								notscan_flg = 0;
								break;
							}

							//エラーチェック
							else if (res_code == STAIN_LEVEL_1)
							{
								//輪郭スキャンが正常に終了しなかった
								//閾値を変更してもういちど
								continue;
							}

							//エラーチェック
							else if (conter_point_count <= 5)
							{
								//ポイント数が少なかった
								continue;
							}

							//エラーチェック：頂点内に最初に検知した位置が入っているかの確認
							//少しずれることがあるのでマージンを持たせる
							if (max_x + 3 <= pv.x || max_y - 3 >= pv.y || min_x - 3 >= pv.x || min_y + 3 <= pv.y)
							{
								//入っていないので誤検知
								notscan_flg = 1;
								break;
							}

							//染みの中心座標が面積の範囲内かを調査する
							for (i = 0; i < conter_point_count; ++i)
							{
								if ((center_x + 5 >= conter_x[i] && center_x - 5 <= conter_x[i]) && center_y <= conter_y[i])
								{
									center_check_y_down = center_check_y_down + conter_y[i];
									center_check_count_y_down = center_check_count_y_down + 1;
								}

								if ((center_x + 5 >= conter_x[i] && center_x - 5 <= conter_x[i]) && center_y >= conter_y[i])
								{
									center_check_y_up = center_check_y_up + conter_y[i];
									center_check_count_y_up = center_check_count_y_up + 1;
								}

								if ((center_y + 5 >= conter_y[i] && center_y - 5 <= conter_y[i]) && center_x <= conter_x[i])
								{
									center_check_x_right = center_check_x_right + conter_x[i];
									center_check_count_x_right = center_check_count_x_right + 1;
								}

								if ((center_y + 5 >= conter_y[i] && center_y - 5 <= conter_y[i]) && center_x >= conter_x[i])
								{
									center_check_x_left = center_check_x_left + conter_x[i];
									center_check_count_x_left = center_check_count_x_left + 1;
								}
							}

							//エラーチェック：上下左右いずれかが欠けている場合
							if (center_check_count_x_right == 0 ||
								center_check_count_x_left == 0 ||
								center_check_count_y_up == 0 ||
								center_check_count_y_down == 0)
							{
								continue;
							}

							//頂点座標の周囲の座標を計算
							center_check_x_right = center_check_x_right / center_check_count_x_right;
							center_check_x_left = center_check_x_left / center_check_count_x_left;

							center_check_y_up = center_check_y_up / center_check_count_y_up;
							center_check_y_down = center_check_y_down / center_check_count_y_down;

							//エラーチェック：範囲外なら未検出とする
							if (!(center_check_x_right >= center_x &&
								center_check_x_left <= center_x &&
								center_check_y_up <= center_y &&
								center_check_y_down >= center_y))
							{
								continue;
							}

							//面積を求める
							area = stain_area(conter_x, conter_y, conter_point_count, (s16)center_x, (s16)center_y) * sta->raito;

							if (area < 0)
							{
								area = 0;
							}

							//輪郭検知の結果が2周してる判定なら
							if (res_code == 1)
							{
								area = area * 0.5f;//値を/2
							}

							//最大面積を記録
							if (sta->res_max_stain_area < area)
							{
								sta->res_max_stain_area = area;
							}


							//面積比較
							if (stain_area_thr <= area && sta->comparison_method == 0)
							{

#ifdef VS_DEBUG
								if (debug_logi_view == 1)
								{
									deb_para[0] = 3;		// function code
									deb_para[1] = center_x;		//
									deb_para[2] = center_y;		//
									deb_para[3] = 1;		// plane
									callback(deb_para);		// debug
								}
#endif	
								//閾値以上ならスキャン打ち切り
								//sta->res_max_stain_area = area * 0.127f * 0.127f;
								//sta->res_max_stain_area = area;
								//累計面積を更新
								total_area = (u32)(total_area + area);
								sta->res_total_stain_area = (float)total_area;

								stain_level_detect(sta);
								sta->res_judge = RES_THRSHOLD_OVER;
#ifdef CSV_DB_STAIN
								F_WRITE(fp, sta->res_judge);	//filedebug
								F_N(fp);				//filedebug
								F_CLOSE(fp);			//filedebug
#endif
								return 1;
							}

							else if (sta->comparison_method == 1)
							{
								//直径の計算
								curent_diameter = (float)(sqrtf(area * RECIP_PI) * 2.0f);

								//最大直径を記録
								if (sta->res_max_stain_diameter < curent_diameter)
								{
									sta->res_max_stain_diameter = curent_diameter;
								}

								//面積および直径での比較
								if (stain_area_thr <= area || stain_diameter_thr <= curent_diameter)
								{
									//直径を格納
									//sta->res_max_stain_diameter = curent_diameter * 0.127f;

									//閾値以上ならスキャン打ち切り
									//sta->res_max_stain_area = area * 0.127f * 0.127f;
									total_area = (u32)(total_area + area);
									sta->res_total_stain_area = (float)total_area;

									stain_level_detect(sta);
									sta->res_judge = RES_THRSHOLD_OVER;
#ifdef CSV_DB_STAIN
									F_WRITE(fp, sta->res_judge);	//filedebug
									F_N(fp);				//filedebug
									F_CLOSE(fp);			//filedebug
#endif
									return 1;

								}
							}

							//面積が小さかった
							//閾値設定のミスによる誤検知かもしれないのでもう一度スキャンする。
							//小さい染みの場合　左上右上の座標が分かるのでその範囲をスキャンできなくする。
							//重複しないように座標を記録する
							notscan[not_scan_area_count].left_up_x = min_x - 1;
							notscan[not_scan_area_count].left_up_y = max_y - 1;

							notscan[not_scan_area_count].right_down_x = max_x + 1;
							notscan[not_scan_area_count].right_down_y = min_y + 1;

							not_scan_area_count++;

							if (not_scan_area_count >= NOT_SEARCH_POINT_MAX_NUM)
							{
								sta->res_stain_err = ERR_ARRAY_OVER;
								sta->res_judge = RES_TOTALAREA_OVER;
								stain_level_detect(sta);
#ifdef CSV_DB_STAIN
								F_WRITE(fp, sta->res_judge);	//filedebug
								F_N(fp);				//filedebug
								F_CLOSE(fp);			//filedebug
#endif
								return 0;
							}

							notscan_flg = 1;
							notscan_flg2 = 1;

							if (temp_area < area)
							{
								temp_area = area;
							}

							continue;


						}
						else//穴だったら終了
						{
							break;
						}

					}

					if (notscan_flg == 0 && notscan_flg2 == 0)	//穴とスキャンエラーの際は検知ポイントから周囲Ndotをスキャンできなくする
					{
						//重複しないように座標を記録する
						notscan[not_scan_area_count].left_up_x = min_x - 1;
						notscan[not_scan_area_count].left_up_y = max_y - 1;

						notscan[not_scan_area_count].right_down_x = max_x + 1;
						notscan[not_scan_area_count].right_down_y = min_y + 1;

						//重複ポイント数を増やす
						not_scan_area_count = not_scan_area_count + 1;

						if (not_scan_area_count >= NOT_SEARCH_POINT_MAX_NUM)
						{
							sta->res_stain_err = ERR_ARRAY_OVER;
							sta->res_judge = RES_TOTALAREA_OVER;
							stain_level_detect(sta);
#ifdef CSV_DB_STAIN
							F_WRITE(fp, sta->res_judge);	//filedebug
							F_N(fp);				//filedebug
							F_CLOSE(fp);			//filedebug
#endif
							return 0;
						}
					}
					else if (notscan_flg == 1)
					{
						//重複ポイント数を増やす
						not_scan_area_count = not_scan_area_count + 1;
						if (not_scan_area_count >= NOT_SEARCH_POINT_MAX_NUM)
						{
							sta->res_stain_err = ERR_ARRAY_OVER;
							sta->res_judge = RES_TOTALAREA_OVER;
							stain_level_detect(sta);
#ifdef CSV_DB_STAIN
							F_WRITE(fp, sta->res_judge);	//filedebug
							F_N(fp);				//filedebug
							F_CLOSE(fp);			//filedebug
#endif
							return 0;
						}
					}

					//累計面積を更新
					total_area = (u32)(total_area + temp_area);
					sta->res_total_stain_area = (float)total_area;

					//トータルの面積比較 処理を早期に終わらせるため
					if (stain_total_area_thr <= total_area)
					{
						//sta->res_total_stain_area = total_area;
						//sta->res_max_stain_area = area ;//* 0.127 * 0.127;
						sta->res_judge = RES_TOTALAREA_OVER;
						stain_level_detect(sta);
#ifdef CSV_DB_STAIN
						F_WRITE(fp, sta->res_judge);	//filedebug
						F_N(fp);				//filedebug
						F_CLOSE(fp);			//filedebug
#endif
						return 0;
						//break;
					}

					//検知した染みの数が規定値以上
					if (NOT_SEARCH_POINT_MAX_NUM <= not_scan_area_count)
					{
						sta->res_stain_err = ERR_ARRAY_OVER;
						sta->res_judge = RES_TOTALAREA_OVER;
						stain_level_detect(sta);
#ifdef CSV_DB_STAIN
						F_WRITE(fp, sta->res_judge);	//filedebug
						F_N(fp);				//filedebug
						F_CLOSE(fp);			//filedebug
#endif
						return 0;
					}
				}
			}//ｘ
		}//ｙ
	}//plane_count

	stain_level_detect(sta);

#ifdef CSV_DB_STAIN
	F_WRITE(fp, sta->res_judge);	//filedebug
	F_N(fp);				//filedebug
	F_CLOSE(fp);			//filedebug
#endif

	return 0;

}



//************************************************************
/*
透過データを用いて穴かどうかを判定する。

	*in		s16 x,		現在の座標ｘ
	s16 y ,		ｙ
	u8 buf_num ,バッファ番号
	u8 plane	プレーン番号
	*
	*out	穴判定　0：染み　1：穴
*/
s16 hole_check(s16 x, s16 y, u8 buf_num, u8 plane, ST_STAIN* sta)
{

#define TRANS_THR 250

	ST_BS* pbs = work[buf_num].pbs;
	ST_SPOINT spt;

	s16 hole_check_x = 0;
	s16 hole_check_y = 0;
	float dpi_rate;

	//pv.plane = OMOTE_T_R;
	spt.p_plane_tbl = OMOTE_T_R;
	spt.p = 0;
	if (pbs->PlaneInfo[spt.p_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
	{
		spt.p_plane_tbl = DOWN_T_R;			 //用いるプレーン指定
	}

	dpi_rate = ((float)pbs->PlaneInfo[plane].sub_sampling_pitch / (float)pbs->PlaneInfo[spt.p_plane_tbl].sub_sampling_pitch);

	//求めた染みの中心座標の周辺をスキャンして1つでも閾値以上ならば穴とする。
	for (hole_check_y = y + 2; hole_check_y >= y - 2; hole_check_y = hole_check_y - 2)
	{
		for (hole_check_x = x - 6; hole_check_x <= x + 6; hole_check_x = hole_check_x + 6)
		{
			spt.x = hole_check_x;
			spt.y = (u32)(hole_check_y * dpi_rate);

			//有効画素範囲の確認
			if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
				sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
			{
				return STAIN_LEVEL_3;
			}
			P_Getdt_8bit_only(&spt, pbs);

			if ((u8)spt.sensdt > TRANS_THR)	//穴だったら
			{
				return 1;
			}
		}
	}
	return 0;
}


//************************************************************
/*
	全面スキャンで発見した染みと思われる場所の輪郭スキャンを行う。
	初めに輪郭位置まで移動して、そこから8近傍探索で右回りで1周スキャンする。
	閾値は全面スキャン時で使っていた時の閾値を用いる。

	*in		ST_BS *pbs ,					サンプリングデータ
	ST_NOTE_PARAMETER* pnote_param ,座標変換パラメタ
	s16 start_x,					全面スキャンで検知した染みの座標ｘ
	s16 start_y,					ｙ
	u8 thr,							この時の閾値
	u8 plane_num ,					プレーン番号
	float multi ,					閾値の乗数
	*
	*out	s16 *conter_x ,					中心座標ｘ
	s16 *conter_y ,					ｙ
	u16 *conter_point_counter,		輪郭座標数
	s32 *center_x ,					輪郭座標リストｘ
	s32 *center_y ,					ｙ
	u16* max_x ,					輪郭内の左上の座標ｘ
	u16* max_y ,					ｙ
	u16* min_x ,					輪郭内の右下の座標ｘ
	u16* min_y						ｙ

	検知結果
	-1：再スキャン		スキャンを途中で失敗した。
	-3：スキャン終了	スキャン回数が上限以上、除外エリアをスキャンした。
*/
s16 contor_detect(ST_STAIN* sta, ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, s16 start_x, s16 start_y, s16* conter_x,
	s16* conter_y, u8 thr, u16* conter_point_counter, u8 plane_num, float multi, s32* center_x, s32* center_y, u16* max_x,
	u16* max_y, u16* min_x, u16* min_y)
{

#define STAIN_MODE_	//NO	：上辺からスキャンを開始する。
	//OFF	：左辺から

	ST_SPOINT spt;
	u8 scan_dir[8] = { 0 };			//スキャンの実行or実行しないを決定する。
	u8 scan_res = 0;				//スキャン結果を格納する。
	//スキャン方向リスト
	u8 scan_dir_list[8][8] = {
		{5,6,7,0,1,2,3,4},	//0	
		{6,7,0,1,2,3,4,5},	//1
		{7,0,1,2,3,4,5,6},	//2
		{0,1,2,3,4,5,6,7},	//3
		{1,2,3,4,5,6,7,0},	//4
		{2,3,4,5,6,7,0,1},	//5
		{3,4,5,6,7,0,1,2},	//6
		{4,5,6,7,0,1,2,3} };	//7

	u16 x = 0;						//現在の座標ｘ
	u16 y = 0;						//ｙ
	u16 conter_start_x = 0;			//ステートが切り替わるときの座標を記録するｘ
	u16 conter_start_y = 0;			//ｙ
	u16 point_count = 0;			//輪郭の座標数
	u16 point_count2 = 0;			//輪郭の座標数 / 2
	u32 max_point_count = 0;		//無限ループ防止用

	u8  state_change_counter = 0;	//ステートを切り替えるまでのカウント数
	s32 temp_x = 0;					//
	s32 temp_y = 0;					//
	u32 i = 0;
	u16 j = 0;
	u8  skip = 0;
	s16 previous_y = 0;				//コンテニュー時（res=10）の前回の座標を記録する

	u8  loop_flg = 0;

	u32 outline = 0;

	u8  orbit_x = 0;

	u16 conter_start_x_left = 0;
	u16 conter_start_x_right = 0;

	s8 res_code = 0;

	s8 stert_miss_recovery = 0;
	s8 loewer_flg = 0;

	u32 limit_while = 0;

	u8 scan_flg_[STAIN_MAX_WHILE];	//スキャン結果のフラグを格納していく。

	//傾きの逆数
	float r_slope = 1 / sta->left_and_right_edge_slope;

	spt.p = 0;
	spt.trace_flg = 1;

	spt.p_plane_tbl = (u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]);
	spt.l_plane_tbl = (u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]);
	spt.way = pbs->insertion_direction;
	x = start_x;
	y = start_y;

	//スキャン閾値の調節
	//暗くすると誤検知率は下がるが、
	//面積精度が落ちる
	if (sta->security_mode == 1)
	{
		thr = thr - (u8)(thr * multi);
	}
	else
	{
		thr = thr + (u8)(thr * multi);
	}

	scan_dir[0] = 0;
	scan_dir[1] = 0;
	scan_dir[2] = 0;
	scan_dir[3] = 0;
	scan_dir[4] = 0;
	scan_dir[5] = 1;
	scan_dir[6] = 0;
	scan_dir[7] = 0;

	//コンテニューの部分も直すこと
#ifdef STAIN_MODE_
		//スタート位置選定処理
		//検知した位置から左にスキャンしてぶつかった位置を記録する
		//左辺の座標
		//temp_x = (u16)((y - sta->right_edge_offset) * r_slope);
	temp_x = (s32)((y - sta->left_edge_offset) * r_slope);
	while (limit_while < MAX_LIMIT_WHILE)
	{
		limit_while++;
		scan_res = 0;
		x = x - SCAN_STEP_Y;
		spt.x = x;
		spt.y = y;

		//有効画素範囲の確認
		if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
			sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
		{
			return STAIN_LEVEL_3;
		}

		if (limit_while > sta->note_size_x_one_third)
		{
			return STAIN_LEVEL_3;
		}

		//skip = 0;
		////除外エリアのスキャン確認
		//for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
		//{
		//	if ((sta->notscan2[plane_num][i].left_up_x <= x && sta->notscan2[plane_num][i].right_down_x >= x) &&
		//		(sta->notscan2[plane_num][i].left_up_y <= y && sta->notscan2[plane_num][i].right_down_y >= y))
		//	{
		//		//スキャンした時点で終了フラグを立てる
		//		skip = 1;

		//		//有効エリアの確認
		//		if (sta->not_scan_safe_area_count[plane_num][i] != 0)
		//		{
		//			for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
		//			{
		//				if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= x) &&
		//					(sta->notscan_in_vali[plane_num][i][j].left_up_y <= y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= y))
		//				{
		//					//有効エリアだった場合、終了フラグをキャンセル
		//					skip = 0;
		//					break;
		//				}
		//			}
		//		}

		//		break;
		//	}
		//}

		//if (skip == 1)
		//{
		//	//除外エリアだったので終了する。
		//	//座標を記録
		//	conter_start_x_left = x;
		//	break;
		//}

		//画素採取
		P_Getdt_8bit_only(&spt, pbs);
		if (thr < spt.sensdt)
		{
			scan_res = 1;
		}

		if (scan_res == 1 && state_change_counter != 0)	//ノイズかもしれないので白紙がNdot続くまでスキャン
		{
			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else if (scan_res == 1)							//初白紙スキャン
		{
			//座標を記録
			conter_start_x_left = x;

			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else
		{
			//紙幣エリア外判定
			if (temp_x >= x)
			{
				state_change_counter = STATE_LIMIT_Y + 1;
				conter_start_x_left = x;
			}
			else
			{
				//スキャン続行
				state_change_counter = 0;
			}
		}

		if (state_change_counter > STATE_LIMIT_Y)	//ステート変更　座標更新　スキャン方向の変更
		{
			state_change_counter = 0;
			break;

		}

	}

	//スタート位置選定処理
	//検知した位置から右にスキャンしてぶつかった位置を記録する
	x = start_x;
	y = start_y;
	//右辺の座標
	temp_x = (s32)((y - sta->right_edge_offset) * r_slope);
	//temp_x = (u16)((y - sta->left_edge_offset) * r_slope);
	limit_while = 0;
	while (limit_while < MAX_LIMIT_WHILE)
	{
		limit_while++;
		scan_res = 0;
		x = x + SCAN_STEP_Y;
		spt.x = x;
		spt.y = y;

		//有効画素範囲の確認
		if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
			sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
		{
			return STAIN_LEVEL_3;
		}


		if (limit_while > sta->note_size_x_one_third)
		{
			return STAIN_LEVEL_3;
		}

		//skip = 0;
		////除外エリアのスキャン確認
		//for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
		//{
		//	if ((sta->notscan2[plane_num][i].left_up_x <= x && sta->notscan2[plane_num][i].right_down_x >= x) &&
		//		(sta->notscan2[plane_num][i].left_up_y <= y && sta->notscan2[plane_num][i].right_down_y >= y))
		//	{
		//		//スキャンした時点で終了フラグを立てる
		//		skip = 1;

		//		//有効エリアの確認
		//		if (sta->not_scan_safe_area_count[plane_num][i] != 0)
		//		{
		//			for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
		//			{
		//				if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= x) &&
		//					(sta->notscan_in_vali[plane_num][i][j].left_up_y <= y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= y))
		//				{
		//					//有効エリアだった場合、終了フラグをキャンセル
		//					skip = 0;
		//					break;
		//				}
		//			}
		//		}

		//		break;
		//	}
		//}

		//if (skip == 1)
		//{
		//	//除外エリアだったので終了する。
		//	//座標を記録
		//	conter_start_x_right = x;
		//	break;
		//}

		//画素採取
		P_Getdt_8bit_only(&spt, pbs);
		if (thr < spt.sensdt)
		{
			scan_res = 1;
		}

		if (scan_res == 1 && state_change_counter != 0)	//ノイズかもしれないので白紙がNdot続くまでスキャン
		{
			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else if (scan_res == 1)							//初白紙スキャン
		{
			//座標を記録
			conter_start_x_right = x;

			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else
		{
			//紙幣エリア外判定
			if (temp_x <= x)
			{
				state_change_counter = STATE_LIMIT_Y + 1;
				conter_start_x_right = x;
			}
			else
			{
				//スキャン続行
				state_change_counter = 0;
			}
		}

		if (state_change_counter > STATE_LIMIT_Y)	//ステート変更　座標更新　スキャン方向の変更
		{
			state_change_counter = 0;
			break;

		}

	}

	if (conter_start_x_right - conter_start_x_left > 250)
	{
		return STAIN_LEVEL_2;	//予想の直径が範囲以上
	}

	if ((conter_start_x_right - conter_start_x_left) == 2)	//幅が少ない場合　白紙エリア上に居ると考えられる
	{

		//上記で求めた座標から下方向にスキャンする。
		//染みにぶち当たるまでスキャンする。
		//紙幣端の場合はさらに右にスキャンしてぶち当たった点からスキャンを開始する。
		x = (u16)((conter_start_x_left + conter_start_x_right) * 0.5f);
		limit_while = 0;
		while (limit_while < MAX_LIMIT_WHILE)
		{
			limit_while++;
			scan_res = 0;
			y = y + SCAN_STEP_Y;
			spt.x = x;
			spt.y = y;

			//有効画素範囲の確認
			if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
				sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
			{
				return STAIN_LEVEL_3;
			}


			if (limit_while > sta->note_size_y_one_third)
			{
				return STAIN_LEVEL_3;
			}

			//skip = 0;
			////除外エリアのスキャン確認
			//for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
			//{
			//	if ((sta->notscan2[plane_num][i].left_up_x <= x && sta->notscan2[plane_num][i].right_down_x >= x) &&
			//		(sta->notscan2[plane_num][i].left_up_y <= y && sta->notscan2[plane_num][i].right_down_y >= y))
			//	{
			//		//スキャンした時点で終了フラグを立てる
			//		skip = 1;

			//		//有効エリアの確認
			//		if (sta->not_scan_safe_area_count[plane_num][i] != 0)
			//		{
			//			for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
			//			{
			//				if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= x) &&
			//					(sta->notscan_in_vali[plane_num][i][j].left_up_y <= y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= y))
			//				{
			//					//有効エリアだった場合、終了フラグをキャンセル
			//					skip = 0;
			//					break;
			//				}
			//			}
			//		}

			//		break;
			//	}
			//}

			//if (skip == 1)
			//{
			//	//除外エリアだったので終了する。
			//	return STAIN_LEVEL_3;
			//}

			//画素採取
			P_Getdt_8bit_only(&spt, pbs);
			if (thr >= spt.sensdt)
			{
				scan_res = 1;
			}

			if (scan_res == 1 && state_change_counter != 0)	//ノイズかもしれないので白紙がNdot続くまでスキャン
			{
				//ステート変更カウンター
				state_change_counter = state_change_counter + 1;
			}
			else if (scan_res == 1)							//初白紙スキャン
			{
				//座標を記録
				conter_start_x = x;
				conter_start_y = y - 1;//

				//ステート変更カウンター
				state_change_counter = state_change_counter + 1;
			}
			else
			{
				//上辺の座標
				outline = (u32)(sta->up_and_down_edge_slope * x + sta->down_edge_offset);

				//紙幣エリア外判定
				if (outline <= y)
				{
					state_change_counter = 2 + 1;
					conter_start_x = x;
					conter_start_y = y - 1;//

				}
				else
				{
					//スキャン続行
					state_change_counter = 0;
				}
			}

			if (state_change_counter > 2)	//ステート変更　座標更新　スキャン方向の変更
			{
				x = conter_start_x;
				y = conter_start_y;

				//スキャン方向リストをセット
				memcpy(&scan_dir, scan_dir_list[1], sizeof(scan_dir));	//右の面をスキャンしたいから１
				state_change_counter = 0;

				//座標を記録する
				conter_x[point_count] = x;
				conter_y[point_count] = y;

				//無限ループ防止用
				point_count++;

				break;

			}

		}
	}
	else
	{
		//上記で求めた座標から上方向にスキャンする。
		//白紙部分もしくは紙幣端にぶち当たるまでスキャンする。
		//白紙部分の場合はその場所から輪郭スキャンを行う
		//紙幣端の場合はさらにま右にスキャンしてぶち当たった点からスキャンを開始する。
		x = (u16)((conter_start_x_left + conter_start_x_right) * 0.5f);
		limit_while = 0;
		while (limit_while < MAX_LIMIT_WHILE)
		{
			limit_while++;
			scan_res = 0;
			y = y - SCAN_STEP_Y;
			spt.x = x;
			spt.y = y;

			//有効画素範囲の確認
			if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
				sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
			{
				return STAIN_LEVEL_3;
			}
			if (limit_while > sta->note_size_y_one_third)
			{
				return STAIN_LEVEL_3;
			}


			skip = 0;
			//除外エリアのスキャン確認
			for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
			{
				if ((sta->notscan2[plane_num][i].left_up_x <= x && sta->notscan2[plane_num][i].right_down_x >= x) &&
					(sta->notscan2[plane_num][i].left_up_y <= y && sta->notscan2[plane_num][i].right_down_y >= y))
				{
					//スキャンした時点で終了フラグを立てる
					skip = 1;

					//有効エリアの確認
					if (sta->not_scan_safe_area_count[plane_num][i] != 0)
					{
						for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
						{
							if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= x) &&
								(sta->notscan_in_vali[plane_num][i][j].left_up_y <= y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= y))
							{
								//有効エリアだった場合、終了フラグをキャンセル
								skip = 0;
								break;
							}
						}
					}

					break;
				}
			}

			if (skip == 1)
			{
				//除外エリアだったので終了する。
				x = conter_start_x;
				y = conter_start_y;

				//スキャン方向リストをセット
				memcpy(&scan_dir, scan_dir_list[1], sizeof(scan_dir));	//右の面をスキャンしたいから１
				state_change_counter = 0;

				//座標を記録する
				conter_x[point_count] = x;
				conter_y[point_count] = y;

				//無限ループ防止用
				point_count++;

				break;
			}

			//画素採取
			P_Getdt_8bit_only(&spt, pbs);
			if (thr < spt.sensdt)
			{
				scan_res = 1;
			}

			if (scan_res == 1 && state_change_counter != 0)	//ノイズかもしれないので白紙がNdot続くまでスキャン
			{
				//ステート変更カウンター
				state_change_counter = state_change_counter + 1;
			}
			else if (scan_res == 1)							//初白紙スキャン
			{
				//座標を記録
				conter_start_x = x;
				conter_start_y = y + 1;//

				//ステート変更カウンター
				state_change_counter = state_change_counter + 1;
			}
			else
			{
				//上辺の座標
				outline = (u32)(sta->up_and_down_edge_slope * x + sta->up_edge_offset);

				//紙幣エリア外判定
				if (outline >= y)
				{
					state_change_counter = 2 + 1;
					conter_start_x = x;
					conter_start_y = y + 1;//

				}
				else
				{
					//スキャン続行
					state_change_counter = 0;
				}
			}

			if (state_change_counter > 2)	//ステート変更　座標更新　スキャン方向の変更
			{
				x = conter_start_x;
				y = conter_start_y;

				//スキャン方向リストをセット
				memcpy(&scan_dir, scan_dir_list[1], sizeof(scan_dir));	//右の面をスキャンしたいから１
				state_change_counter = 0;

				//座標を記録する
				conter_x[point_count] = x;
				conter_y[point_count] = y;

				//無限ループ防止用
				point_count++;

				break;

			}

		}
	}
#else

		//スタート位置選定処理
		//検知した位置から左にスキャンしてぶつかった位置を記録する
		//上辺の座標
	temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->up_edge_offset);
	while (limit_while < MAX_LIMIT_WHILE)
	{
		limit_while++;
		scan_res = 0;
		y = y - SCAN_STEP_Y;
		spt.x = x;
		spt.y = y;

		//有効画素範囲の確認
		if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
			sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
		{
			return STAIN_LEVEL_3;
		}
		//画素採取
		P_Getdt_8bit_only(&spt, pbs);
		if (thr < spt.sensdt)
		{
			scan_res = 1;
		}

		if (scan_res == 1 && state_change_counter != 0)	//ノイズかもしれないので白紙がNdot続くまでスキャン
		{
			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else if (scan_res == 1)							//初白紙スキャン
		{
			//座標を記録
			conter_start_y_up = y;

			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else
		{
			//紙幣エリア外判定
			if (temp_y >= y)
			{
				state_change_counter = STATE_LIMIT_Y + 1;
				conter_start_y_up = y;
			}
			else
			{
				//スキャン続行
				state_change_counter = 0;
			}
		}

		if (state_change_counter > STATE_LIMIT_Y)	//ステート変更　座標更新　スキャン方向の変更
		{
			state_change_counter = 0;
			break;

		}

	}

	//スタート位置選定処理
	//検知した位置から右にスキャンしてぶつかった位置を記録する
	x = start_x;
	y = start_y;
	//下辺の座標
	temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->down_edge_offset);
	limit_while = 0;
	while (limit_while < MAX_LIMIT_WHILE)
	{
		limit_while++;
		scan_res = 0;
		y = y + SCAN_STEP_Y;
		spt.x = x;
		spt.y = y;

		//有効画素範囲の確認
		if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
			sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
		{
			return STAIN_LEVEL_3;
		}
		//画素採取
		P_Getdt_8bit_only(&spt, pbs);
		if (thr < spt.sensdt)
		{
			scan_res = 1;
		}

		if (scan_res == 1 && state_change_counter != 0)	//ノイズかもしれないので白紙がNdot続くまでスキャン
		{
			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else if (scan_res == 1)							//初白紙スキャン
		{
			//座標を記録
			conter_start_y_down = y;

			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else
		{
			//紙幣エリア外判定
			if (temp_y <= y)
			{
				state_change_counter = STATE_LIMIT_Y + 1;
				conter_start_y_down = y;
			}
			else
			{
				//スキャン続行
				state_change_counter = 0;
			}
		}

		if (state_change_counter > STATE_LIMIT_Y)	//ステート変更　座標更新　スキャン方向の変更
		{
			state_change_counter = 0;
			break;

		}

	}

	if (conter_start_y_down - conter_start_y_up > (250 / sta->raito))
	{
		return STAIN_LEVEL_2;	//予想の直径が範囲以上
	}

	//上記で求めた座標から左方向にスキャンする。
	//白紙部分もしくは紙幣端にぶち当たるまでスキャンする。
	//白紙部分の場合はその場所から輪郭スキャンを行う
	y = (u16)((conter_start_y_down + conter_start_y_up) * 0.5f);
	limit_while = 0;
	while (limit_while < MAX_LIMIT_WHILE)
	{
		limit_while++;
		scan_res = 0;
		x = x - SCAN_STEP_Y;
		spt.x = x;
		spt.y = y;

		//有効画素範囲の確認
		if (sta->main_scan_min_val > x || sta->main_scan_max_val < x ||
			sta->sub_scan_min_val > y || sta->sub_scan_max_val < y)
		{
			return STAIN_LEVEL_3;
		}
		//画素採取
		P_Getdt_8bit_only(&spt, pbs);
		if (thr < spt.sensdt)
		{
			scan_res = 1;
		}

		if (scan_res == 1 && state_change_counter != 0)	//ノイズかもしれないので白紙がNdot続くまでスキャン
		{
			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else if (scan_res == 1)							//初白紙スキャン
		{
			//座標を記録
			conter_start_x = x + 1;
			conter_start_y = y;//

			//ステート変更カウンター
			state_change_counter = state_change_counter + 1;
		}
		else
		{
			//上辺の座標
			//outline = (u32)(sta->up_and_down_edge_slope * x + sta->up_edge_offset);
			outline = (s32)((y - sta->left_edge_offset) * r_slope);

			//紙幣エリア外判定
			if (outline >= x)
			{
				state_change_counter = 2 + 1;
				conter_start_x = x + 1;
				conter_start_y = y;//

			}
			else
			{
				//スキャン続行
				state_change_counter = 0;
			}
		}

		if (state_change_counter > 2)	//ステート変更　座標更新　スキャン方向の変更
		{
			x = conter_start_x;
			y = conter_start_y;

			//スキャン方向リストをセット
			memcpy(&scan_dir, scan_dir_list[7], sizeof(scan_dir));	//右の面をスキャンしたいから１
			state_change_counter = 0;

			//座標を記録する
			conter_x[point_count] = x;
			conter_y[point_count] = y;

			//無限ループ防止用
			point_count++;

			break;

		}

	}

#endif

	stert_miss_recovery = 0;

	//右回りで輪郭スキャンを行う
	while (max_point_count < MAX_LIMIT_WHILE)
	{
		max_point_count = max_point_count + 1;	//無限ループ防止用

		if (point_count > STAIN_MAX_WHILE)
		{
			break;
		}

		//現在のポイントの周辺をスキャンして座標を更新
		scan_res = pix_scan_module(spt, scan_dir, pbs, &x, &y, thr, sta);

		if (sta->security_mode == 1)
		{
			skip = 0;
			//除外エリアのスキャン確認
			for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
			{
				if ((sta->notscan2[plane_num][i].left_up_x <= x && sta->notscan2[plane_num][i].right_down_x >= x) &&
					(sta->notscan2[plane_num][i].left_up_y <= y && sta->notscan2[plane_num][i].right_down_y >= y))
				{
					//スキャンした時点で終了フラグを立てる
					skip = 1;

					//有効エリアの確認
					if (sta->not_scan_safe_area_count[plane_num][i] != 0)
					{
						for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
						{
							if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= x) &&
								(sta->notscan_in_vali[plane_num][i][j].left_up_y <= y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= y))
							{
								//有効エリアだった場合、終了フラグをキャンセル
								skip = 0;
								break;
							}
						}
					}

					break;
				}
			}
		}

		if (skip == 1)
		{
			//除外エリアだったので終了する。
			return STAIN_LEVEL_3;
		}

#ifdef STAIN_MODE_

		//スタート位置y座標に来た
		if (conter_start_y == y)
		{
			//スタート地点だったら
			if (conter_start_x + sta->coef_raito > x && conter_start_x - sta->coef_raito < x && point_count > 40)
			{
				break; //正常
			}

			//スタート地点より少し遠い位置だったら
			if (conter_start_x + sta->coef_raito * 2 - 1 > x && conter_start_x - sta->coef_raito * 2 + 1 < x && point_count > 40 * 3)
			{
				orbit_x++;

				//そこを2回通った時点で終了
				if (orbit_x == 2)
				{
					res_code = 0;	//1
					break;
				}
			}
		}

#else 



		//スタート位置x座標に来た
		if (conter_start_x == x)
		{
			//スタート地点だったら
			if (conter_start_y + sta->coef_raito > y && conter_start_y - sta->coef_raito < y && point_count > sta->coef_raito)
			{
				break; //正常
			}

			//スタート地点より少し遠い位置だったら
			if (conter_start_y + sta->coef_raito * 2 - 1 > y && conter_start_y - sta->coef_raito * 2 + 1 < y && point_count > sta->coef_raito * 5)
			{
				orbit_x++;

				//そこを2回通った時点で終了
				if (orbit_x == 2)
				{
					res_code = 0;	//1
					break;
				}
			}
		}
#endif



		loewer_flg = 0;

		//紙幣エリア外チェック
		//検知した方向に応じてエリア外チェックを行う。
		switch (scan_res)
		{

		case stain_right_down:		//右下
			loewer_flg = 0;
			temp_x = (s32)((y - sta->right_edge_offset) * r_slope) - 5;
			temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->down_edge_offset) - 5;

			if (temp_x <= x)	//右辺がエリア外だった場合
			{
				//x = x - 1;
				//location 1上　2右　3下　4左
				scan_res = note_contor_scan(sta, pbs, &x, &y,/*location*/2, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			if (temp_y <= y)	//下辺
			{
				//y = y - 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 3, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case stain_down:		//下
			loewer_flg = 1;
			temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->down_edge_offset) - 5;

			//if(temp_xL >= x)	//左辺がエリア外だった場合
			//{
			//	//x = x - 1;
			//	//location 1上　2右　3下　4左
			//	scan_res = note_contor_scan(sta, pbs ,&x ,&y ,4 ,thr, spt.p_plane_tbl ,conter_start_x ,conter_x , conter_y,	&point_count ,&point_count2);
			//}
			//if(temp_xR <= x)	//右辺がエリア外だった場合
			//{
			//	//x = x - 1;
			//	//location 1上　2右　3下　4左
			//	scan_res = note_contor_scan(sta, pbs ,&x ,&y ,2 ,thr, spt.p_plane_tbl ,conter_start_x ,conter_x , conter_y,	&point_count ,&point_count2);
			//}
			if (temp_y <= y)	//下辺
			{
				//y = y - 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 3, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case stain_left_down:		//左下
			loewer_flg = 2;
			temp_x = (s32)((y - sta->left_edge_offset) * r_slope) + 5;
			temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->down_edge_offset) - 5;

			if (temp_x >= x)	//左辺
			{
				//x = x + 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 4, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			if (temp_y <= y)	//下辺
			{
				//y = y - 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 3, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case stain_left:		//左
			temp_x = (s32)((y - sta->left_edge_offset) * r_slope);
			loewer_flg = 2;

			if (temp_x >= x)	//左辺
			{
				//x = x + 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 4, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}
			if (temp_x >= x)	//左辺
			{
				//x = x + 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 4, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case stain_left_up:		//左上
			loewer_flg = 2;
			temp_x = (s32)((y - sta->left_edge_offset) * r_slope) + 5;
			temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->up_edge_offset) + 5;

			if (temp_x >= x)	//左辺
			{
				//x = x + 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 4, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			if (temp_y >= y)	//上辺
			{
				//y = y + 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 1, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case stain_up:		//上
			loewer_flg = 3;
			temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->up_edge_offset) + 5;

			if (temp_y >= y)	//上辺
			{
				//y = y + 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 1, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case stain_right_up:		//右上
			loewer_flg = 0;
			temp_x = (s32)((y - sta->right_edge_offset) * r_slope) - 5;
			temp_y = (s32)(sta->up_and_down_edge_slope * x + sta->up_edge_offset) + 5;
			if (temp_x <= x)	//右辺がエリア外だった場合
			{
				//x = x - 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y,/*location*/2, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			if (temp_y >= y)	//上辺
			{
				//y = y + 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y, 1, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case stain_right:		//右
			loewer_flg = 0;
			temp_x = (s32)((y - sta->right_edge_offset) * r_slope) - 5;
			if (temp_x <= x)	//右辺がエリア外だった場合
			{
				//x = x - 1;
				scan_res = note_contor_scan(sta, pbs, &x, &y,/*location*/2, thr, spt.p_plane_tbl, conter_start_x, conter_x, conter_y, &point_count, &point_count2);
			}

			break;

		case 10:		//当てはまらない
			stert_miss_recovery = stert_miss_recovery + 1;
			if (stert_miss_recovery <= 4)
			{
				return STAIN_LEVEL_1;	//移動できる画素がなかった。　
			}

			break;

		}



		//スタートに失敗したら何度Y座標を下げてやり直す為のコンテニュー
		if (scan_res == 10)
		{
#ifdef STAIN_MODE_
			y = y + 1;
#else
			x = x - 1;
#endif
			if (previous_y == y)
			{
				return STAIN_LEVEL_3;	//同じ座標でコンテニューする場合は無限ループに入ったとみなす。
			}
			previous_y = y;
			//thr = thr + 5;
			continue;
		}

		if (scan_res == 11)
		{
			return STAIN_LEVEL_1;	//移動できる画素がなかった。　
		}

#ifdef STAIN_SCAN_PERFORMANCE_UP_TEST
		skip = 0;
		//除外エリアのスキャン確認
		for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
		{
			if ((sta->notscan2[plane_num][i].left_up_x <= x && sta->notscan2[plane_num][i].right_down_x >= x) &&
				(sta->notscan2[plane_num][i].left_up_y <= y && sta->notscan2[plane_num][i].right_down_y >= y))
			{
				//スキャンした時点で終了フラグを立てる
				skip = 1;

				//有効エリアの確認
				if (sta->not_scan_safe_area_count[plane_num][i] != 0)
				{
					for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
					{
						if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= x) &&
							(sta->notscan_in_vali[plane_num][i][j].left_up_y <= y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= y))
						{
							//有効エリアだった場合、終了フラグをキャンセル
							skip = 0;
							break;
						}
					}
				}

				break;
			}
		}
		//除外エリアの場合スキャン方向を修正する
		if (skip == 1)
		{
			if (scan_res == 4 || scan_res == 3)
			{
				scan_res = 0;
			}
			else if (scan_res == 5 || scan_res == 6)
			{
				scan_res = 2;
			}
			else if (scan_res == 7 || scan_res == 0)
			{
				scan_res = 4;
			}
			else if (scan_res == 2 || scan_res == 1)
			{
				scan_res = 6;
			}
		}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST

		//座標を記録する
		if (loewer_flg == 0 && skip == 0)
		{
			conter_x[point_count2] = x;
			conter_y[point_count2] = y - 1;
		}
		else if (loewer_flg == 1 && skip == 0)
		{
			conter_x[point_count2] = x + 1;
			conter_y[point_count2] = y;
		}
		else if (loewer_flg == 2 && skip == 0)
		{
			conter_x[point_count2] = x;
			conter_y[point_count2] = y + 1;
		}
		else if (loewer_flg == 3 && skip == 0)
		{
			conter_x[point_count2] = x - 1;
			conter_y[point_count2] = y;
		}
		else
		{
			x = conter_x[point_count2 - 1];
			y = conter_y[point_count2 - 1];


			conter_x[point_count2] = x;
			conter_y[point_count2] = y;
		}

		//フラグも保存する
		scan_flg_[point_count2] = scan_res;


		stert_miss_recovery = 5;	//開始地点補正の処理を無効にする。

		if (point_count2 > 10)
		{
			for (i = 1; i < 10; i++)
			{
				//同じ座標を繰り返し何回も通っていたら。
				if (conter_x[point_count2 - i] == conter_x[point_count2] &&
					conter_y[point_count2 - i] == conter_y[point_count2] && 
					scan_flg_[point_count2 - i] == scan_flg_[point_count2])
				{
					loop_flg++;	//エラーカウントアップ
					//return STAIN_LEVEL_1;	//ループ回数が上限以上
				}
			}
		}

		if (loop_flg > 2)	//一定数以上でエラー
		{
			return STAIN_LEVEL_2;	//ループ回数が上限以上
		}

		//座標ポイントを更新
		point_count = point_count + 1;
		point_count2 = (u16)(point_count * 0.5f);


		//スキャンパターンをスキャンした方向に応じて更新
		memcpy(&scan_dir, scan_dir_list[scan_res], sizeof(scan_dir));


	}

	//エラーチェック
	if ((point_count >= STAIN_MAX_WHILE) || max_point_count >= MAX_LIMIT_WHILE)
	{
		return STAIN_LEVEL_2;	//ループ回数が上限以上
	}

	//エラーチェック
	if (point_count < 5)
	{
		return STAIN_LEVEL_1;	//ポイント数が少ない
	}


	//中心座標、左上右下座標を求める
	for (i = 0; i < point_count2; ++i)
	{
		*center_x = *center_x + conter_x[i];
		*center_y = *center_y + conter_y[i];

		//直径と大まかなエリアを調べるためにｘとｙの最小最大を調べる
		if (conter_x[i] > *max_x)
		{
			*max_x = conter_x[i];
		}
		if (conter_y[i] < *max_y)
		{
			*max_y = conter_y[i];
		}
		if (conter_x[i] < *min_x)
		{
			*min_x = conter_x[i];
		}
		if (conter_y[i] > *min_y)
		{
			*min_y = conter_y[i];
		}
	}

	*center_x = *center_x / point_count2;
	*center_y = *center_y / point_count2;
	*conter_point_counter = point_count2;

	return res_code;
}

//************************************************************
/*
染みの輪郭スキャンを行う。
*	8近傍探索方式になっている。
*	現在位置から画素値が閾値以下になっている方向を返す
*
*in	ST_SPOINT spt ,					座標変換構造体
*	u8 dir[] ,						スキャン方向リスト
ST_BS *pbs ,					サンプリングデータ
ST_NOTE_PARAMETER* pnote_param	座標変換パラメタ
u16 *x ,						現在の座標ｘ
u16 *y ,						ｙ
u8 thr							画素値の閾値
*
*out	進む方向
*		0：右下　1：下　2：左下　3：左　4：左上　5：上　6：右上　7：右
*		更新した座標ｘ
ｙ
*/
s8 pix_scan_module(ST_SPOINT spt, u8 dir[], ST_BS* pbs, u16* x, u16* y, u8 thr, ST_STAIN* sta)
{
	u8 dir_idx = 0;

	for (dir_idx = 0; dir_idx < 8; ++dir_idx)
	{

		//左下
		if (dir[dir_idx] == stain_left_down)
		{
			spt.x = (s32)*x - 1;
			spt.y = (s32)*y + 1;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#ifdef STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST

			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;

				return 2;
			}
		}

		//下
		else if (dir[dir_idx] == stain_down)
		{
			spt.x = (s32)*x;
			spt.y = (s32)*y + 1;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#if STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST
			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;

				return 1;
			}
		}

		//右下
		else if (dir[dir_idx] == stain_right_down)
		{
			spt.x = (s32)*x + 1;
			spt.y = (s32)*y + 1;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#if STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST
			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;

				return 0;
			}
		}

		//左
		else if (dir[dir_idx] == stain_left)
		{
			spt.x = (s32)*x - 1;
			spt.y = (s32)*y;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#if STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST
			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;

				return 3;
			}
		}

		//右
		else if (dir[dir_idx] == stain_right)
		{
			spt.x = (s32)*x + 1;
			spt.y = (s32)*y;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#if STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST
			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;

				return 7;
			}
		}


		//左上
		else if (dir[dir_idx] == stain_left_up)
		{
			spt.x = (s32)*x - 1;
			spt.y = (s32)*y - 1;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#if STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST
			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;

				return 4;
			}
		}

		//上
		else if (dir[dir_idx] == stain_up)
		{
			spt.x = (s32)*x;
			spt.y = (s32)*y - 1;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#if STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST
			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;

				return 5;
			}
		}

		//右上
		else if (dir[dir_idx] == stain_right_up)
		{
			spt.x = (s32)*x + 1;
			spt.y = (s32)*y - 1;
			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val > spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 10;
			}
#if STAIN_SCAN_PERFORMANCE_UP_TEST
			if (check_area_(sta, &spt))
			{
				continue;
			}
#endif // STAIN_SCAN_PERFORMANCE_UP_TEST
			P_Getdt_8bit_only(&spt, pbs);
			if (thr > spt.sensdt)
			{
				*x = (u16)spt.x;
				*y = (u16)spt.y;
				return 6;
			}
		}

	}

	return 10;
}


u8 check_area_(ST_STAIN* sta, ST_SPOINT* spt)
{
	u8 skip = 0;
	u32 i, j = 0;
	u8 plane_num = spt->p_plane_tbl;
	//除外エリアのスキャン確認
	for (i = 0; i < sta->not_scan_area_count[plane_num]; i++)
	{
		if ((sta->notscan2[plane_num][i].left_up_x <= spt->x && sta->notscan2[plane_num][i].right_down_x >= spt->x) &&
			(sta->notscan2[plane_num][i].left_up_y <= spt->y && sta->notscan2[plane_num][i].right_down_y >= spt->y))
		{
			//スキャンした時点で終了フラグを立てる
			skip = 1;

			//有効エリアの確認
			if (sta->not_scan_safe_area_count[plane_num][i] != 0)
			{
				for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; j++)
				{
					if ((sta->notscan_in_vali[plane_num][i][j].left_up_x <= spt->x && sta->notscan_in_vali[plane_num][i][j].right_down_x >= spt->x) &&
						(sta->notscan_in_vali[plane_num][i][j].left_up_y <= spt->y && sta->notscan_in_vali[plane_num][i][j].right_down_y >= spt->y))
					{
						//有効エリアだった場合、終了フラグをキャンセル
						skip = 0;
						break;
					}
				}
			}

			break;
		}
	}

	return skip;
}


//************************************************************
/*
*	染みの輪郭スキャン中に紙幣端に到達したとき、
到達した紙幣端の辺に従って水平移動する。
*
* in	ST_STAIN *sta				専用構造体
ST_BS *pbs,					サンプリングデータ
u16 *x,						検知した位置の座標ｘ
u16 *y ,					ｙ
u8 location,				辺の場所
u16 thr ,					その時の画素値の閾値
u8 plane ,					その時のプレーン番号
u16 finish_x ,				探索開始位置ｘ
s16 *conter_x ,				座標リストｘ
s16 *conter_y ,				ｙ
u16 *conter_point_counter	輪郭座標数
*
* out	s16 *conter_x ,				追記した座標リストｘ
s16 *conter_y ,				ｙ
*		u16 *x,						紙幣端でなくなった座標ｘ
u16 *y ,					ｙ
*/

u8 note_contor_scan(ST_STAIN* sta, ST_BS* pbs, u16* x, u16* y, u8 location, u16 thr, u8 plane, u16 finish_x, s16* conter_x, s16* conter_y, u16* conter_point_counter, u16* conter_point_counter2)
{
#define OFFSRT_CORRECTION 0
	ST_SPOINT spt;
	s32 temp;
	float r_slope;
	float temp_ofs;

	s32 temp_y = 0;
	s32 temp_x = 0;

	u16 input_x = *x;
	u16 input_y = *y;

	u8  scan_res = 0;
	u32 while_limit_count = *conter_point_counter;

	float loca_res = 0;

	spt.p_plane_tbl = plane;
	spt.p = 0;

	//location 1上　2右　3下　4左
	switch (location)
	{
	case 1://上辺

		temp_ofs = sta->up_edge_offset + OFFSRT_CORRECTION;
		r_slope = 1 / sta->left_and_right_edge_slope;

		while (while_limit_count < STAIN_MAX_WHILE)
		{
			while_limit_count++;

			//右辺の座標
			temp = (s32)((*y - sta->right_edge_offset) * r_slope);

			//1dot移動
			*x = *x + 1;
			//*y = (u16)(sta->up_and_down_edge_slope * *x + temp_ofs);

			temp_y = (s32)(sta->up_and_down_edge_slope * *x + temp_ofs);

			if (temp_y < 0)
			{
				*y = 0;
			}
			else
			{
				*y = (u16)temp_y;
			}

			if (temp < 0)
			{
				temp = 0;
			}

			loca_res = get_right_direction((float)input_x, (float)input_y, 1, r_slope, (float)*x, (float)*y);
			if (loca_res < -5)	//右側なら終了
			{
				return 10;
			}

			spt.x = (s32)*x;
			spt.y = (s32)*y;

			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val >spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 11;
			}
			P_Getdt_8bit_only(&spt, pbs);
			if (thr < spt.sensdt || temp <= *x || finish_x == *x)
			{
				scan_res = 4;
				break;
			}

			conter_x[*conter_point_counter2] = (s16)spt.x;
			conter_y[*conter_point_counter2] = (s16)spt.y;
			*conter_point_counter = *conter_point_counter + 1;
			*conter_point_counter2 = (u16)(*conter_point_counter * 0.5f);
		}

		break;

	case 2://右辺
		temp_ofs = sta->right_edge_offset + OFFSRT_CORRECTION;
		r_slope = 1 / sta->left_and_right_edge_slope;

		while (while_limit_count < STAIN_MAX_WHILE)
		{
			while_limit_count++;

			//下辺の座標
			temp = (s32)(sta->up_and_down_edge_slope * *x + sta->down_edge_offset);

			//1dot移動
			*y = *y + 1;
			temp_x = (s32)((*y - temp_ofs) * r_slope);

			if (temp_x < 0)
			{
				*x = 0;
			}
			else
			{
				*x = (u16)temp_x;
			}

			if (temp < 0)
			{
				temp = 0;
			}

			loca_res = get_right_direction((float)input_x, (float)input_y, 1, r_slope, (float)*x, (float)*y);
			if (loca_res < -5)	//右側なら終了
			{
				return 10;
			}

			spt.x = (s32)*x;
			spt.y = (s32)*y;

			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val >spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 11;
			}
			P_Getdt_8bit_only(&spt, pbs);
			if (thr < spt.sensdt || temp <= *y)
			{
				scan_res = 6;
				break;
			}
			conter_x[*conter_point_counter2] = (s16)spt.x;
			conter_y[*conter_point_counter2] = (s16)spt.y;
			*conter_point_counter = *conter_point_counter + 1;
			*conter_point_counter2 = (u16)(*conter_point_counter * 0.5f);

		}

		break;

	case 3://下辺
		temp_ofs = sta->down_edge_offset - OFFSRT_CORRECTION;
		r_slope = 1 / sta->left_and_right_edge_slope;

		while (while_limit_count < STAIN_MAX_WHILE)
		{
			while_limit_count++;

			//左辺の座標
			//temp = (s32)((*y - temp_ofs) * r_slope);										//下辺の直線式のx座標	閾値用
			temp = (s32)((*y - sta->left_edge_offset) * r_slope);					//左辺の直線式のx座標	閾値用
																							//
			//1dot移動																		//
			*x = *x - 1;																	//
			temp_y = (s32)(sta->up_and_down_edge_slope * *x + sta->down_edge_offset);		//下辺の直線式のy座標	閾値用
																							//
			if (temp_y < 0)																	//
			{
				*y = 0;
			}
			else
			{
				*y = (u16)temp_y;
			}

			if (temp < 0)
			{
				temp = 0;
			}

			loca_res = get_right_direction((float)input_x, (float)input_y, 1, r_slope, (float)*x, (float)*y);
			if (loca_res > 5)	//右側なら終了
			{
				return 10;
			}

			spt.x = (s32)*x;
			spt.y = (s32)*y;

			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val >spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 11;
			}
			P_Getdt_8bit_only(&spt, pbs);
			if (thr < spt.sensdt || temp >= *x)
			{
				scan_res = 0;
				break;
			}

			conter_x[*conter_point_counter2] = (s16)spt.x;
			conter_y[*conter_point_counter2] = (s16)spt.y;
			*conter_point_counter = *conter_point_counter + 1;
			*conter_point_counter2 = (u16)(*conter_point_counter * 0.5f);
		}

		break;

	case 4://左辺
		temp_ofs = sta->left_edge_offset - OFFSRT_CORRECTION;
		r_slope = 1 / sta->left_and_right_edge_slope;

		while (while_limit_count < STAIN_MAX_WHILE)
		{
			while_limit_count++;

			//上辺の座標
			temp = (s32)(sta->up_and_down_edge_slope * *x + sta->up_edge_offset);

			//1dot移動
			*y = *y - 1;
			temp_x = (s32)((*y - temp_ofs) * r_slope);

			if (temp_x < 0)
			{
				*x = 0;
			}
			else
			{
				*x = (u16)temp_x;
			}

			if (temp < 0)
			{
				temp = 0;
			}

			loca_res = get_right_direction((float)input_x, (float)input_y, 1, r_slope, (float)*x, (float)*y);
			if (loca_res > 5)	//右側なら終了
			{
				return 10;
			}

			spt.x = (s32)*x;
			spt.y = (s32)*y;

			//有効画素範囲の確認
			if (sta->main_scan_min_val > spt.x || sta->main_scan_max_val < spt.x ||
				sta->sub_scan_min_val >spt.y || sta->sub_scan_max_val < spt.y)
			{
				return 11;
			}
			P_Getdt_8bit_only(&spt, pbs);
			if (thr < spt.sensdt || temp >= *y)
			{
				scan_res = 2;
				break;
			}

			conter_x[*conter_point_counter2] = (s16)spt.x;
			conter_y[*conter_point_counter2] = (s16)spt.y;
			*conter_point_counter = *conter_point_counter + 1;
			*conter_point_counter2 = (u16)(*conter_point_counter * 0.5f);
		}

		break;

	}

	//入力座標が変わらなかったら
	if (input_x == *x && input_y == *y)
	{
		return 10;	//ループ中断信号を出す
	}
	else
	{
		return scan_res;
	}

}

//************************************************************
//多角形の面積を求める
//複数の三角形の面積の合計を求める 
//
//in	s16 *x				輪郭座標リストｘ
//		s16 *y				輪郭座標リストｙ
//		u16 point_count ,	座標数
//		s16 origin_x ,		中心座標ｘ	
//		s16 origin_y		中心座標ｙ
//		u8 buf_num			バッファ番号
//
//out	面積（dot）
//
float stain_area(s16* x, s16* y, u16 point_count, s16 origin_x, s16 origin_y)
{

#define STEP 1
	u32 i = 0;
	s32 sarea = 0;
	s32 temp;

	point_count = point_count - 1;

	for (i = 0; i < point_count; i = i + STEP)
	{
		temp = (x[i] - origin_x) * (y[i + STEP] - origin_y) - (x[i + STEP] - origin_x) * (y[i] - origin_y);
		sarea += temp;
	}

	temp = (x[i] - origin_x) * (y[0] - origin_y) - (x[0] - origin_x) * (y[i] - origin_y);
	sarea += temp;

	return (float)(sarea * 0.5f);
}

//************************************************************
//学習で求めたポイントマップを基に除外エリアの設定を行う。
//マップ上では除外ポイントは0となっているので、
//それの連結成分を調査して決定する。
//
//in	ST_BS *pbs,						サンプリングデータ
//		ST_STAIN *sta,					専用構造体
//		ST_NOTE_PARAMETER* pnote_param,	座標変換パラメタ
//		s16 extra_point_x[],			抽出ポイント座標ｘリスト
//		s16 extra_point_y[] ,			ｙ
//
//out	s16 extra_point_x[],			抽出ポイント座標ｘリスト（論理座標）
//		s16 extra_point_y[] ,			ｙ
//		sta->notscan2					除外エリアリスト
//		sta->not_scan_area_count		除外エリアカウント
//
s8 not_scan_area_dision(ST_BS* pbs, ST_STAIN* sta, ST_NOTE_PARAMETER* pnote_param, s16 extra_point_x[], s16 extra_point_y[])
{
	u8 x = 0;
	u8 y = 0;
	u8 plane_num = 0;

	//除外エリアを記録する配列
	s16 lu_rd_x[STAIN_MAX_NOT_SCAN_AREA_NUM][2];
	s16 lu_rd_y[STAIN_MAX_NOT_SCAN_AREA_NUM][2];

	s16 lu_rd_x_vali[STAIN_MAX_NOT_SCAN_AREA_NUM][2];
	s16 lu_rd_y_vali[STAIN_MAX_NOT_SCAN_AREA_NUM][2];

	u8 area_count = 0;
	u8 i = 0;
	u8 j = 0;

	s8 res = 0;

	//座標リスト作成
	for (y = 0; y < sta->y_split - 1; y++)	//分割数y
	{
		extra_point_y[y] = (s16)(-((y + 1) * sta->note_size_y / (sta->y_split)) + (sta->half_size_y));	//

	}
	for (x = 0; x < sta->x_split - 1; x++)	//分割数x
	{
		extra_point_x[x] = (s16)(((x + 1) * sta->note_size_x / (sta->x_split)) - (sta->half_size_x));
	}

	for (plane_num = 0; plane_num < sta->plane_num; plane_num++)
	{
		//IR1,2以外は除外エリア特定を行わない。　
		if (!((u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]) == OMOTE_R_IR1 || (u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]) == URA_R_IR1 ||
			  (u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]) == OMOTE_R_IR2 || (u8)(pnote_param->pplane_tbl[sta->plane[plane_num]]) == URA_R_IR2))
		{
			continue;
		}

		memset(&lu_rd_x, 0, sizeof(lu_rd_x));
		memset(&lu_rd_y, 0, sizeof(lu_rd_y));
		area_count = 0;

		not_scan_area_serch(sta, 0, 0, (s32)sta->x_split - 1, (s32)sta->y_split - 1, 0, plane_num, lu_rd_x, lu_rd_y, &area_count);		//紙幣の除外エリアを選定
		sta->not_scan_area_count[plane_num] = area_count;	//除外エリア数を格納
		area_count = 0;
		for (i = 0; i < sta->not_scan_area_count[plane_num]; ++i)				//求めた各エリアの座標を物理に変換
		{
			set_area_info(pbs, sta, pnote_param, plane_num, lu_rd_x, lu_rd_y, extra_point_x, extra_point_y, i);	//求めた座標を物理に変換
			res = validity_area_serch(sta, (s32)lu_rd_x[i][0], (s32)lu_rd_y[i][0], (s32)lu_rd_x[i][1], (s32)lu_rd_y[i][1], 1, plane_num, lu_rd_x_vali, lu_rd_y_vali, &area_count);	//除外エリア内の有効エリアを選定する
			if (res == -1)	//エラー判定の場合は終了する。
			{
				return res;
			}

			sta->not_scan_safe_area_count[plane_num][i] = area_count;	//この除外エリア内の有効エリア数をカウント
			area_count = 0;
			for (j = 0; j < sta->not_scan_safe_area_count[plane_num][i]; ++j)				//求めた座標を物理に変換
			{
				set_safearea_info(pbs, sta, pnote_param, plane_num, lu_rd_x_vali, lu_rd_y_vali, extra_point_x, extra_point_y, j, i);	//求めた座標を物理に変換
			}
		}
	}

	//外周N㎜からは全体スキャンから取り除く処理
	//除外エリアの選定が終わってからでないと実行できない
	for (y = 0; y < sta->y_split - 1; y++)	//分割数y
	{
		//外周スキャンチェック
		if ((sta->not_scan_range_y_left_up < extra_point_y[y] || sta->not_scan_range_y_right_down > extra_point_y[y]))
		{
			extra_point_y[y] = 0x0FFF;
		}
	}

	for (x = 0; x < sta->x_split - 1; x++)	//分割数x
	{
		//外周スキャンチェック
		if ((sta->not_scan_range_x_left_up > extra_point_x[x] || sta->not_scan_range_x_right_down < extra_point_x[x]))
		{
			extra_point_x[x] = 0x0FFF;
		}
	}


	return 0;
}

//除外エリアを求める。
s8	not_scan_area_serch(ST_STAIN* sta, s32 start_x, s32 start_y, s32 end_x, s32 end_y, u8 tot, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], u8* area_num)
{
	u8 i = 0;
	u8 j = 1;
	u8 k = 2;
	u8 point_th = 4;

	u32 th_count = 0;
	s32 x = 0;
	s32 y = 0;
	u8 th = 0;
	s16 conter_start_x;
	s16 conter_start_y;
	u16 point_count = 0;

	s32 scan_x = 0;
	s32 scan_y = 0;

	s16 conter_x[STAIN_MAX_NOT_SCAN_AREA_DETECT_NUM];
	s16 conter_y[STAIN_MAX_NOT_SCAN_AREA_DETECT_NUM];

	u8 skip = 0;
	u8 go = 0;

	u8 scan_dir[4] = { 0 };			//スキャンの実行or実行しないを決定する。
	u8 scan_res = 0;				//スキャン結果を格納する。
	u32 limit_while = 0;


	//スキャン方向リスト
	u8 scan_dir_list[4][4] = {
		{3,0,1,2},	//0	
		{0,1,2,3},	//1
		{1,2,3,0},	//2
		{2,3,0,1},	//3
	};

	//スキャンパターンをスキャンした方向に応じて更新
	memcpy(&scan_dir, scan_dir_list[3], sizeof(scan_dir));

	////表裏で除外エリアを作成する
	//for(plane_num = 0; plane_num < sta->plane_num; ++plane_num)
	//{
	//	th_count = 0;
	//	i = 0;

	//sta->not_scan_area_count[plane_num] = 0;

	if (tot == 1)
	{
		//除外内の有効エリアの際のパラメタ
		j = 10;
		k = j + 1;
		point_th = 15;
	}

	//ラスタースキャンでエリアの始点を見つける
	for (scan_y = start_y; scan_y < end_y; scan_y++)	//分割数y
	{
		for (scan_x = start_x; scan_x < end_x; scan_x++)	//分割数x
		{
			x = scan_x;
			y = scan_y;

			//学習の閾値参照
			th_count = x + y * (sta->x_split - 1);
			th = sta->thr[plane_num][th_count];
			//th_count = th_count + 1;

			if (tot == 0)
			{
				if (th == 0)
				{
					go = 1;
				}
			}
			else if (tot == 1)
			{
				if (th != 0)
				{
					go = 1;
				}
			}

			//基点判定
			if (go == 1)
			{
				go = 0;
				//除外エリアの重複スキャン確認
				for (i = 0; i < *area_num; i++)
				{
					if ((lu_rd_x[i][0] <= x && lu_rd_x[i][1] >= x) &&
						(lu_rd_y[i][0] <= y && lu_rd_y[i][1] >= y))
					{
						skip = 1;
					}
				}

				//スキャン済みならスキップする
				if (skip == 1)
				{
					skip = 0;
					continue;
				}

				conter_start_x = (s16)x;
				conter_start_y = (s16)y;

				//スキャンパターンをスキャンした方向に応じて更新
				memcpy(&scan_dir, scan_dir_list[3], sizeof(scan_dir));

				//座標を記録する
				conter_x[point_count] = (s16)x;
				conter_y[point_count] = (s16)y;

				point_count++;

				//連結成分を調査
				limit_while = 0;
				while (limit_while < MAX_LIMIT_WHILE)
				{
					limit_while++;

					if (x == -1)
					{
						return 0;
					}
					scan_res = thr_scan_module(scan_dir, &x, &y, sta->thr[plane_num], sta->x_split - 1, sta->y_split - 1, tot);

					//スキャンできる方向がないなら
					if (scan_res == 10)
					{
						break;	//終了
					}

					//除外エリアの重複スキャン確認
					skip = 0;
					for (i = 0; i < *area_num; i++)
					{
						if ((lu_rd_x[i][0] <= x && lu_rd_x[i][1] >= x) &&
							(lu_rd_y[i][0] <= y && lu_rd_y[i][1] >= y))
						{
							skip = 1;
							break;
						}
					}

					//スキャン範囲外に出たら
					if ((end_x < x || start_x > x) ||
						(end_y < y || start_y > y))
					{
						skip = 1;
					}

					//スキャン済みならスキップする
					if (skip == 1)
					{
						//ぶつかった際のスキャン方向に応じて進む方向を決定する
						switch (scan_res)
						{
						case 0:	//下0なので左1へ
							scan_res = 2;
							//x = x - 1;
							y = y - 1;
							break;

						case 1:	//左1なので上2へ
							scan_res = 3;
							x = x + 1;
							//y = y - 1;
							break;

						case 2:	//上2なので右3へ
							scan_res = 0;
							//x = x + 1;
							y = y + 1;
							break;

						case 3:	//右3なので下0へ
							scan_res = 1;
							x = x - 1;
							//y = y + 1;
							break;
						}

					}

					//座標を記録する
					conter_x[point_count] = (s16)x;
					conter_y[point_count] = (s16)y;

					point_count++;

					//スタート位置に戻ってきたら終了
					if (conter_start_x == x && conter_start_y == y)
					{
						break;

					}

					//ポイント数が上限なら
					if (point_count >= STAIN_MAX_NOT_SCAN_AREA_DETECT_NUM)
					{
						break;	//終了
					}

					//スキャンパターンをスキャンした方向に応じて更新
					memcpy(&scan_dir, scan_dir_list[scan_res], sizeof(scan_dir));
				}

				//折り返しは含めないので/2する
				if (point_count * 0.5 < point_th)
				{
					memset(&conter_x, 0, sizeof(conter_x));
					memset(&conter_y, 0, sizeof(conter_y));
					point_count = 0;
					continue;
				}

				//ソートして
				stain_quick_sort(conter_x, 0, (s16)point_count - 1);
				stain_quick_sort(conter_y, 0, (s16)point_count - 1);

				//左上ｘｙ　右下ｘｙの順で座標を書き込む
				lu_rd_x[*area_num][0] = conter_x[j];
				lu_rd_x[*area_num][1] = conter_x[point_count - k];
				lu_rd_y[*area_num][0] = conter_y[j + 1];
				lu_rd_y[*area_num][1] = conter_y[point_count - k - 1];

				//リストを初期化
				memset(&conter_x, 0, sizeof(conter_x));
				memset(&conter_y, 0, sizeof(conter_y));
				point_count = 0;


				//除外エリア数をカウント
				//sta->not_scan_area_count[plane_num]++;
				*area_num = *area_num + 1;

				if (*area_num == (u8)(NOT_SEARCH_POINT_MAX_NUM - 1))
				{
					break;	//規定数以上になりかけたら終わらせる。
				}

			}

		}
	}
	//}
	return 0;
}

//有効エリアを求める。
s8	validity_area_serch(ST_STAIN* sta, s32 start_x, s32 start_y, s32 end_x, s32 end_y, u8 tot, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], u8* area_num)
{
	u8 i = 0;
	u8 j = 0;
	//u8 point_th = 4;

	u32 th_count = 0;
	s32 x = 0;
	s32 y = 0;
	u8 th = 0;
	//s16 conter_start_x;
	//s16 conter_start_y;


	u16 upwer_side_point_count = 0;
	u16 right_side_point_count = 0;
	u16 lower_side_point_count = 0;
	u16 left_side_point_count = 0;

	s32 scan_x = 0;
	s32 scan_y = 0;
	u8 scan_end_flg = 1;
	u8 side_scan_end_flg = 0;

	u32 limit_while = 0;

	//u8  upwer_side_y_cv = 0;

	s16 upwer_side_end_x[3];
	s16 upwer_side_end_y[3];
	s16 right_side_end_x[3];
	s16 right_side_end_y[3];
	s16 lower_side_end_x[3];
	s16 lower_side_end_y[3];
	s16  left_side_end_x[3];
	s16  left_side_end_y[3];

	u8 skip = 0;
	u8 last_mode;	//状態
	u8 side_scan_end_count = 0;	//各辺のスキャンが終了した回数を記録

	//スキャン方向リスト
	u8 scan_dir_list[4][4] = {
		{0,5,5,5},	//0	
		{1,5,5,5},	//1
		{2,5,5,5},	//2
		{3,5,5,5},	//3
	};

	u8 scan_dir[4] = { 0 };			//スキャンの実行or実行しないを決定する。
	u8 scan_res = 0;				//スキャン結果を格納する。

	//縦ラスタースキャンでエリアの始点を見つける
	for (scan_x = start_x; scan_x < end_x; scan_x++)	//分割数y
	{
		for (scan_y = start_y; scan_y < end_y; scan_y++)	//分割数x
		{
			x = scan_x;
			y = scan_y;

			//学習の閾値参照
			th_count = x + y * (sta->x_split - 1);
			th = sta->thr[plane_num][th_count];

			//基点発見
			if (th != 0)
			{
				//除外エリアの重複スキャン確認
				for (i = 0; i < *area_num; i++)
				{
					if ((lu_rd_x[i][0] <= x && lu_rd_x[i][1] >= x) &&
						(lu_rd_y[i][0] <= y && lu_rd_y[i][1] >= y))
					{
						skip = 1;
					}
				}

				//スキャン済みならスキップする
				if (skip == 1)
				{
					skip = 0;
					continue;
				}

				//スキャンパターンをスキャンした方向に応じて更新
				memcpy(&scan_dir, scan_dir_list[3], sizeof(scan_dir));

				last_mode = 3;
				x = scan_x - 1;
				if (x < 0)
				{
					x = 0;
				}

				scan_end_flg = 1;

				upwer_side_point_count = 0;
				right_side_point_count = 0;
				lower_side_point_count = 0;
				left_side_point_count = 0;

				memset(&upwer_side_end_x, -1, 6);
				memset(&upwer_side_end_y, -1, 6);
				memset(&right_side_end_x, -1, 6);
				memset(&right_side_end_y, -1, 6);
				memset(&lower_side_end_x, -1, 6);
				memset(&lower_side_end_y, -1, 6);
				memset(&left_side_end_x, -1, 6);
				memset(&left_side_end_y, -1, 6);

				//連結成分を調査
				limit_while = 0;
				while (scan_end_flg)
				{
					limit_while++;
					if (limit_while > MAX_LIMIT_WHILE)
					{
						return -1;	//リミットを超えたらエラー判定
					}

					if (x == -1 && y == -1)
					{
						//return 0;
						break;
					}

					//スキャンを行う座標も更新する（ｘｙ）
					scan_res = thr_scan_module(scan_dir, &x, &y, sta->thr[plane_num], sta->x_split - 1, sta->y_split - 1, tot);

					//除外エリアの重複スキャン確認
					skip = 0;
					for (i = 0; i < *area_num; i++)
					{
						if ((lu_rd_x[i][0] <= x && lu_rd_x[i][1] >= x) &&
							(lu_rd_y[i][0] <= y && lu_rd_y[i][1] >= y))
						{
							skip = 1;
							break;
						}
					}

					//スキャン範囲外に出たら
					if ((end_x < x || start_x > x) ||
						(end_y < y || start_y > y))
					{
						skip = 1;
					}

					//スキャン済みならスキップする
					if (skip == 1)
					{

						//ぶつかった際のスキャン方向に応じて進む方向を決定する
						switch (scan_res)
						{
						case 0:	//下0
							scan_res = 10;
							y++;
							//last_mode = 0;

							break;

						case 1:	//左1
							scan_res = 10;
							x++;
							//last_mode = 1;

							break;

						case 2:	//上2
							scan_res = 10;
							y--;
							//last_mode = 2;

							break;

						case 3:	//右3
							scan_res = 10;
							x--;
							//last_mode = 3;

							break;
						}

					}

					//スキャンできる方向がないなら
					if (scan_res == 10)
					{
						if (last_mode == 3)	//上辺スキャン
						{
							//ポイント数が規定以上の場合
							//到達座標を記録する
							upwer_side_end_x[side_scan_end_count] = (s16)x;
							upwer_side_end_y[side_scan_end_count] = (s16)y;

							//到達回数をカウント
							side_scan_end_count++;

							upwer_side_point_count = 0;

							//スキャン回数が一定以上　もしくは　スキャンラインがこれ以上ない場合
							//○○○○○
							//×○○○○
							//×○○○○
							//－－→　スキャン方向
							//こういう場合でも次に行く
							if (side_scan_end_count > 1)
							{
								//次の辺に移行
								memcpy(&scan_dir, scan_dir_list[0], sizeof(scan_dir));

								//最も長い高さの辺を記録する　x座標において、1と2が0と同等の場合は0を採用する
								for (i = 1; i < side_scan_end_count; i++)
								{
									if (upwer_side_end_x[0] < upwer_side_end_x[i])
									{
										upwer_side_end_x[0] = upwer_side_end_x[i];
										upwer_side_end_y[0] = upwer_side_end_y[i];
										//upwer_side_y_cv = i;
									}
								}
								//最も長い辺は[0]に格納される

								//次のスキャンのスタート座標を書きこむ
								x = (s8)upwer_side_end_x[0];
								y = (s8)upwer_side_end_y[0];
								side_scan_end_count = 0;
								last_mode = 0;
							}
							else
							{
								//位置を変更してスキャン
								x = scan_x;
								y = scan_y + (side_scan_end_count);
							}
						}


						else if (last_mode == 0)	//右辺スキャン
						{
							j++;	//到達回数
							//６５４３２１　　　
							//○○○○○○｜
							//○○××××｜
							//○○××××↓
							//　　　　　スキャン方向
							//こういう場合は５の列にたどり着くまで再試行する。
							if (right_side_point_count != 0)
							{
								//ポイント数が規定以上の場合
								//到達座標を記録する
								right_side_end_x[side_scan_end_count] = (s16)x;
								right_side_end_y[side_scan_end_count] = (s16)y;

								//有効到達回数をカウント
								side_scan_end_count++;

								right_side_point_count = 0;

								//スキャン回数が一定以上　もしくは　スキャンラインがこれ以上ない場合
								if (side_scan_end_count > 1)
								{
									side_scan_end_flg = 1;

								}
								else
								{
									//位置を変更してスキャン
									x = (s8)upwer_side_end_x[0] - (j);
									y = (s8)upwer_side_end_y[0];
								}
							}
							else
							{
								//位置を変更してスキャン
								x = (s8)upwer_side_end_x[0] - (j);
								y = (s8)upwer_side_end_y[0];
							}

							if (scan_x > x)
							{
								if (side_scan_end_count != 0)
								{
									side_scan_end_flg = 1;
								}
								else
								{
									break;
								}
							}


							if (side_scan_end_flg == 1)
							{
								j = 0;
								side_scan_end_flg = 0;
								//次の辺に移行
								memcpy(&scan_dir, scan_dir_list[1], sizeof(scan_dir));

								//最も長い高さの辺を記録する　x座標において、1と2が0と同等の場合は0を採用する
								for (i = 1; i < side_scan_end_count; i++)
								{
									if (right_side_end_y[0] < right_side_end_y[i])
									{
										right_side_end_y[0] = right_side_end_y[i];
										right_side_end_x[0] = right_side_end_x[i];
									}
								}
								//最も長い辺は[0]に格納される
								//次のスキャンのスタート座標を書きこむ
								x = (s8)right_side_end_x[0];
								y = (s8)right_side_end_y[0];
								side_scan_end_count = 0;
								last_mode = 1;
							}


						}

						else if (last_mode == 1)	//下辺
						{
							j++;	//到達回数

							if (lower_side_point_count != 0)
							{
								//ポイント数が規定以上の場合
								//到達座標を記録する
								lower_side_end_x[side_scan_end_count] = (s16)x;
								lower_side_end_y[side_scan_end_count] = (s16)y;

								//有効到達回数をカウント
								side_scan_end_count++;

								lower_side_point_count = 0;

								//スキャン回数が一定以上　もしくは　スキャンラインがこれ以上ない場合
								if (side_scan_end_count > 1)
								{
									side_scan_end_flg = 1;

								}
								else
								{
									//位置を変更してスキャン
									x = (s8)right_side_end_x[0];
									y = (s8)right_side_end_y[0] - (j);
								}
							}
							else
							{
								//位置を変更してスキャン
								x = (s8)right_side_end_x[0];
								y = (s8)right_side_end_y[0] - (j);

							}


							if (upwer_side_end_y[0] > y)
							{
								if (side_scan_end_count != 0)
								{
									side_scan_end_flg = 1;
								}
								else
								{
									break;
								}
							}

							if (side_scan_end_flg == 1)
							{
								j = 0;
								//次の辺に移行
								memcpy(&scan_dir, scan_dir_list[2], sizeof(scan_dir));

								//最も長い高さの辺を記録する　x座標において、1と2が0と同等の場合は0を採用する
								for (i = 1; i < side_scan_end_count; i++)
								{
									if (lower_side_end_x[0] > lower_side_end_x[i])
									{
										lower_side_end_x[0] = lower_side_end_x[i];
										lower_side_end_y[0] = lower_side_end_y[i];
									}
								}
								//最も長い辺は[0]に格納される
								//次のスキャンのスタート座標を書きこむ
								y = (s8)lower_side_end_y[0];
								x = (s8)lower_side_end_x[0];
								side_scan_end_count = 0;
								last_mode = 2;
							}
						}

						else if (last_mode == 2)	//左辺
						{
							j++;	//到達回数

							//位置を変更してスキャン
							x = (s8)(lower_side_end_x[0] + (j));
							y = (s8)lower_side_end_y[0];

							if (right_side_end_x[0] < x)
							{

								break;

							}

						}

					}



					else if (scan_res == 3)
					{
						//upwer_side_conter_x[upwer_side_point_count] = x;
						//upwer_side_conter_y[upwer_side_point_count] = y;
						upwer_side_point_count++;			//ポイント数をカウント
					}

					else if (scan_res == 0)
					{
						right_side_point_count++;			//ポイント数をカウント

						//if(scan_x > x)	//上にエリア外判定があるのでコメントアウト
						//{
						//	side_scan_end_flg = 1;
						//}
					}

					else if (scan_res == 1)
					{
						lower_side_point_count++;			//ポイント数をカウント

						//if(upwer_side_end_y[0] > y)
						//{
						//	side_scan_end_flg = 1;
						//}
					}

					else if (scan_res == 2)
					{
						if (scan_y == y)	//スキャンを開始した高さにたどり着いた
						{
							//到達座標を記録する
							left_side_end_x[0] = (s16)x;
							left_side_end_y[0] = (s16)y;

							left_side_point_count = 0;

							j = 0;

							side_scan_end_count = 0;

							scan_end_flg = 0;	//終了フラグ書き込む
						}


						else
						{
							left_side_point_count++;			//ポイント数をカウント

							if (right_side_end_x[0] < x)
							{
								side_scan_end_flg = 1;
							}
						}


					}


				}
				//値が書き換えられてなかった場合
				if (upwer_side_end_x[0] == -1 ||
					upwer_side_end_y[0] == -1 ||
					right_side_end_x[0] == -1 ||
					right_side_end_y[0] == -1 ||
					lower_side_end_x[0] == -1 ||
					lower_side_end_y[0] == -1 ||
					left_side_end_x[0] == -1 ||
					left_side_end_y[0] == -1
					)
				{
					continue;
				}

				//座標を求める
				if (left_side_end_x[0] > upwer_side_end_x[0])
				{
					x = (s8)upwer_side_end_x[0];
				}
				else
				{
					x = (s8)left_side_end_x[0];
				}

				//左上ｘｙ　右下ｘｙの順で座標を書き込む
				lu_rd_x[*area_num][0] = (s16)x;
				lu_rd_x[*area_num][1] = right_side_end_x[0];
				lu_rd_y[*area_num][0] = upwer_side_end_y[0];
				lu_rd_y[*area_num][1] = lower_side_end_y[0];

				if ((lu_rd_x[*area_num][1] - lu_rd_x[*area_num][0] + 1) * (lu_rd_y[*area_num][1] - lu_rd_y[*area_num][0] + 1) < 15)
				{
					continue;
				}

				//リストを初期化



				//除外エリア数をカウント
				//sta->not_scan_area_count[plane_num]++;
				*area_num = *area_num + 1;

				if (*area_num == (u8)(NOT_SEARCH_POINT_MAX_NUM - 1))
				{
					break;	//規定数以上になりかけたら終わらせる。
				}

			}

		}

	}
	return 0;
}

//求めた除外・有効エリアを物理座標に変換する。
s8	set_area_info(ST_BS* pbs, ST_STAIN* sta, ST_NOTE_PARAMETER* pnote_param, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], s16 extra_point_x[], s16 extra_point_y[], u16 num)
{
	s16 tempx1;
	s16 tempy1;
	s16 tempx2;
	s16 tempy2;
	ST_SPOINT spt;


	tempx1 = extra_point_x[lu_rd_x[num][0]];
	tempy1 = extra_point_y[lu_rd_y[num][0]];
	tempx2 = extra_point_x[lu_rd_x[num][1]];
	tempy2 = extra_point_y[lu_rd_y[num][1]];

	spt.l_plane_tbl = sta->plane[plane_num];

	//挿入方向やLESEに応じて場合分けで座標を変数に格納する。
	//左上　もしくは右下
#ifdef NEW_DIR
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.x = tempx1;
	}
	else
	{
		spt.x = tempx2;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.y = tempy2;
	}
	else
	{
		spt.y = tempy1;
	}

#else
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.x = tempx1;
	}
	else
	{
		spt.x = tempx2;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.y = tempy2;
	}
	else
	{
		spt.y = tempy1;
	}
#endif
	new_L2P_Coordinate(&spt, pbs, pnote_param);

	if (pbs->LEorSE == SE)
	{
		sta->notscan2[plane_num][num].left_up_x = (s16)spt.x;
		sta->notscan2[plane_num][num].left_up_y = (s16)spt.y;
	}
	else
	{
		sta->notscan2[plane_num][num].right_down_x = (s16)spt.x;
		sta->notscan2[plane_num][num].right_down_y = (s16)spt.y;
	}

	//左下もしくは右上
#ifdef NEW_DIR
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.x = tempx2;
	}
	else
	{
		spt.x = tempx1;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.y = tempy1;
	}
	else
	{
		spt.y = tempy2;
	}
#else
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.x = tempx2;
	}
	else
	{
		spt.x = tempx1;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.y = tempy1;
	}
	else
	{
		spt.y = tempy2;
	}
#endif
	new_L2P_Coordinate(&spt, pbs, pnote_param);

	if (pbs->LEorSE == SE)
	{
		sta->notscan2[plane_num][num].right_down_x = (s16)spt.x;
		sta->notscan2[plane_num][num].right_down_y = (s16)spt.y;
	}
	else
	{
		sta->notscan2[plane_num][num].left_up_x = (s16)spt.x;
		sta->notscan2[plane_num][num].left_up_y = (s16)spt.y;
	}

	return 0;
}

s8	set_safearea_info(ST_BS* pbs, ST_STAIN* sta, ST_NOTE_PARAMETER* pnote_param, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], s16 extra_point_x[], s16 extra_point_y[], u16 num, u8 area_c)
{
	s16 tempx1;
	s16 tempy1;
	s16 tempx2;
	s16 tempy2;
	ST_SPOINT spt;


	tempx1 = extra_point_x[lu_rd_x[num][0]];
	tempy1 = extra_point_y[lu_rd_y[num][0]];
	tempx2 = extra_point_x[lu_rd_x[num][1]];
	tempy2 = extra_point_y[lu_rd_y[num][1]];
	spt.l_plane_tbl = sta->plane[plane_num];

	//挿入方向やLESEに応じて場合分けで座標を変数に格納する。
	//左上　もしくは右下
#ifdef NEW_DIR
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.x = tempx1;
	}
	else
	{
		spt.x = tempx2;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.y = tempy2;
	}
	else
	{
		spt.y = tempy1;
	}

#else
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.x = tempx1;
	}
	else
	{
		spt.x = tempx2;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.y = tempy2;
	}
	else
	{
		spt.y = tempy1;
	}
#endif
	new_L2P_Coordinate(&spt, pbs, pnote_param);

	if (pbs->LEorSE == SE)
	{
		sta->notscan_in_vali[plane_num][area_c][num].left_up_x = (s16)spt.x;
		sta->notscan_in_vali[plane_num][area_c][num].left_up_y = (s16)spt.y;
	}
	else
	{
		sta->notscan_in_vali[plane_num][area_c][num].right_down_x = (s16)spt.x;
		sta->notscan_in_vali[plane_num][area_c][num].right_down_y = (s16)spt.y;
	}

	//左下もしくは右上
#ifdef NEW_DIR
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.x = tempx2;
	}
	else
	{
		spt.x = tempx1;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.y = tempy1;
	}
	else
	{
		spt.y = tempy2;
	}
#else
	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W3)
	{
		spt.x = tempx2;
	}
	else
	{
		spt.x = tempx1;
	}

	if (pbs->insertion_direction == W0 || pbs->insertion_direction == W2)
	{
		spt.y = tempy1;
	}
	else
	{
		spt.y = tempy2;
	}
#endif
	new_L2P_Coordinate(&spt, pbs, pnote_param);

	if (pbs->LEorSE == SE)
	{
		sta->notscan_in_vali[plane_num][area_c][num].right_down_x = (s16)spt.x;
		sta->notscan_in_vali[plane_num][area_c][num].right_down_y = (s16)spt.y;
	}
	else
	{
		sta->notscan_in_vali[plane_num][area_c][num].left_up_x = (s16)spt.x;
		sta->notscan_in_vali[plane_num][area_c][num].left_up_y = (s16)spt.y;
	}

	return 0;
}


//************************************************************
//クイックソートを行います
//
//in	s16 list[]	ソート対象配列 
//		u16 left	ソート範囲開始位置
//		u16 right	ソート範囲終了位置
//
//out	list		昇順ソートされたリスト
//
void stain_quick_sort(s16 list[], s16 left, s16 right)
{
	s16 i = left;
	s16 j = right;
	s16 num = (s16)((left + right) * 0.5f);
	s16 pivot = list[num];

	s16 c = 0;

	while (i <= j)
	{
		while (list[i] < pivot)
		{
			i++;
		}
		while (list[j] > pivot)
		{
			j--;
		}
		if (i <= j)
		{
			c = list[j];
			list[j] = list[i];
			list[i] = c;

			i++;
			j--;
		}
	}

	if (left < j)
	{
		stain_quick_sort(list, left, j);
	}
	if (i < right)
	{
		stain_quick_sort(list, i, right);
	}
}

//************************************************************
//ポイントマップから除外ポイントの連結成分を調査する。
//４近傍探索方式になっている。
//現在位置から閾値が0になっている方向を返す
//
//in	u8 dir[]	方向リスト
//		u8 *x		現在の座標ｘ
//		u8 *y		ｙ
//		u8 thr[]	ポイントマップ
//		s16 x_split	分割数ｘ
//		s16 y_split	分割数ｙ
//
//out	進む方向
//		0：↓　1：←　2：↑　3：→
//
s8 thr_scan_module(u8 dir[], s32* x, s32* y, u8 thr[], s16 x_split, s16 y_split, u8 tot)
{
	u8 dir_idx = 0;
	s32 th_p = 0;
	s32 tempx;
	s32 tempy;
	u8 th = 0;
	u8 go = 0;

	tempx = *x;
	tempy = *y;

	th_p = tempx + tempy * x_split;

	if (x_split * (y_split + 1) < th_p)
	{
		return 10;
	}


	th = thr[th_p];
	if (0 != th && tot == 0)
	{
		return 10;
	}

	for (dir_idx = 0; dir_idx < 4; ++dir_idx)
	{

		//下
		if (dir[dir_idx] == 0)
		{
			tempx = *x;
			tempy = *y + 1;

			if (tempy > y_split - 1)
			{
				continue;
			}

			th_p = tempx + tempy * x_split;
			th = thr[th_p];

			if (tot == 0)
			{
				if (th == 0)
				{
					go = 1;
				}
			}
			else if (tot == 1)
			{
				if (th != 0)
				{
					go = 1;
				}
			}

			//基点判定
			if (go == 1)
			{
				go = 0;

				//	if(tot == th)
				//{
				*x = (u8)tempx;
				*y = (u8)tempy;
				return 0;
			}
		}

		//左
		else if (dir[dir_idx] == 1)
		{
			tempx = *x - 1;
			tempy = *y;


			if (tempx < 0)
			{
				continue;
			}

			th_p = tempx + tempy * x_split;
			th = thr[th_p];
			if (tot == 0)
			{
				if (th == 0)
				{
					go = 1;
				}
			}
			else if (tot == 1)
			{
				if (th != 0)
				{
					go = 1;
				}
			}

			//基点判定
			if (go == 1)
			{
				go = 0;

				//	if(tot == th)
				//{
				*x = (u8)tempx;
				*y = (u8)tempy;
				return 1;
			}
		}

		//右
		else if (dir[dir_idx] == 3)
		{
			tempx = *x + 1;
			tempy = *y;


			if (tempx > x_split - 1)
			{
				continue;
			}

			th_p = tempx + tempy * x_split;
			th = thr[th_p];
			if (tot == 0)
			{
				if (th == 0)
				{
					go = 1;
				}
			}
			else if (tot == 1)
			{
				if (th != 0)
				{
					go = 1;
				}
			}

			//基点判定
			if (go == 1)
			{
				go = 0;

				//	if(tot == th)
				//{
				*x = (u8)tempx;
				*y = (u8)tempy;
				return 3;
			}
		}

		//上
		else if (dir[dir_idx] == 2)
		{
			tempx = *x;
			tempy = *y - 1;

			if (tempy < 0)
			{
				continue;
			}

			th_p = tempx + tempy * x_split;
			th = thr[th_p];
			if (tot == 0)
			{
				if (th == 0)
				{
					go = 1;
				}
			}
			else if (tot == 1)
			{
				if (th != 0)
				{
					go = 1;
				}
			}

			//基点判定
			if (go == 1)
			{
				go = 0;

				//	if(tot == th)
				//{
				*x = (u8)tempx;
				*y = (u8)tempy;
				return 2;
			}
		}

	}

	return 10;
}

//************************************************************
//変数の初期化とパラメータの設定を行う。
void stain_ini_and_params_set(ST_STAIN* sta, ST_BS* pbs, u8 palne)
{
	//副走査方向のサンプリング比を設定
	float sub_pitch = pbs->PlaneInfo[palne].sub_sampling_pitch;

	//比に応じてy座標を調整
	s16 left_up_y = (s16)(pbs->left_up_y / sub_pitch);
	s16 left_down_y = (s16)(pbs->left_down_y / sub_pitch);
	s16 right_up_y = (s16)(pbs->right_up_y / sub_pitch);
	//	s16 right_down_y = (s16)(pbs->right_down_y / sub_pitch);

		//4辺の直線式のマージン　
	s8 margin_y = 8;
	s8 margin_x = (u8)(margin_y / sub_pitch);

	//4辺の直線式を求める。
	sta->up_and_down_edge_slope = ((float)((left_up_y - right_up_y)) / (pbs->left_up_x - pbs->right_up_x + 0.000001f));
	if (sta->up_and_down_edge_slope < 0)
	{
		margin_x *= -1;
	}

	sta->up_edge_offset = -(sta->up_and_down_edge_slope * pbs->left_up_x) + (left_up_y)+margin_x;
	sta->down_edge_offset = -(sta->up_and_down_edge_slope * pbs->left_down_x) + (left_down_y)-margin_x;

	sta->left_and_right_edge_slope = ((float)((left_up_y - left_down_y)) / (pbs->left_up_x - pbs->left_down_x + 0.000001f));

	if (sta->left_and_right_edge_slope < 0)
	{
		margin_y *= -1;
	}

	sta->left_edge_offset = -(sta->left_and_right_edge_slope * pbs->left_up_x) + (left_up_y)-margin_y;
	sta->right_edge_offset = -(sta->left_and_right_edge_slope * pbs->right_up_x) + (right_up_y)+margin_y;

	//変数初期化
	sta->not_scan_margin_x = 0;
	sta->not_scan_margin_y = 0;
	sta->raito = 0;
	sta->res_stain_err = 0;
	sta->res_max_stain_area = 0;
	sta->res_max_stain_diameter = 0;
	sta->res_total_stain_area = 0;
	sta->level = 0;
	sta->res_judge = 0;
	sta->not_scan_area_count[0] = 0;
	sta->not_scan_area_count[1] = 0;

}

//レベルに変換する。
u8	stain_level_detect(ST_STAIN* sta)
{

#define MIN_LIMIT_NUM_STAIN_AREA 20
#define MAX_LIMIT_NUM_STAIN_AREA 250

#define MIN_LIMIT_NUM_STAIN_DIAMETER 5
#define MAX_LIMIT_NUM_STAIN_DIAMETER 25

#define MIN_LIMIT_NUM_STAIN_TOTAL_AREA 200
#define MAX_LIMIT_NUM_STAIN_TOTAL_AREA 1000

	float detect_res_ary[2];
	float thr_ary[2];
	u8	tmp_level[2];
	u8	level = 2;

	sta->res_max_stain_area = sta->res_max_stain_area * 0.127f * 0.127f;
	sta->res_max_stain_diameter = sta->res_max_stain_diameter * 0.127f;
	sta->res_total_stain_area = sta->res_total_stain_area * 0.127f * 0.127f;


	if (sta->comparison_method == 0 && sta->res_judge != RES_TOTALAREA_OVER)			//面積のみ
	{
		detect_res_ary[0] = sta->res_max_stain_area;
		thr_ary[0] = sta->stain_size_thr;
		level = level_detect(detect_res_ary, thr_ary, 1, MIN_LIMIT_NUM_STAIN_AREA, MAX_LIMIT_NUM_STAIN_AREA);

	}
	else if (sta->comparison_method == 1 && sta->res_judge != RES_TOTALAREA_OVER)	//面積か直径
	{
		//面積のレベルを計算
		detect_res_ary[0] = sta->res_max_stain_area;
		thr_ary[0] = sta->stain_size_thr;
		tmp_level[0] = level_detect(detect_res_ary, thr_ary, 1, MIN_LIMIT_NUM_STAIN_AREA, MAX_LIMIT_NUM_STAIN_AREA);

		//直径のレベルを計算
		detect_res_ary[0] = sta->res_max_stain_diameter;
		thr_ary[0] = sta->stain_diameter_thr;
		tmp_level[1] = level_detect(detect_res_ary, thr_ary, 1, MIN_LIMIT_NUM_STAIN_DIAMETER, MAX_LIMIT_NUM_STAIN_DIAMETER);

		//最小値を計算
		if (tmp_level[0] > tmp_level[1])
		{
			level = tmp_level[1];
		}
		else
		{
			level = tmp_level[0];
		}
	}
	else if (sta->res_judge == RES_TOTALAREA_OVER)	//合計面積
	{
		//面積のレベルを計算
		detect_res_ary[0] = sta->res_total_stain_area;
		thr_ary[0] = sta->total_stain_thr;
		level = level_detect(detect_res_ary, thr_ary, 1, MIN_LIMIT_NUM_STAIN_AREA, MAX_LIMIT_NUM_STAIN_AREA);
	}

	/*
	//合計面積のレベル計算
	//sta->res_judge = RES_TOTALAREA_OVER
	detect_res_ary[0] = sta->res_total_stain_area;
	thr_ary[0] = sta->total_stain_thr;
	level = level_detect(detect_res_ary , thr_ary ,1);
	*/
	sta->level = level;
	return level;

}


float get_right_direction(float x, float y, float dx, float dy, float tx, float ty)
{
	//ベクトル( dx, dy )
	//ベクトル( tx - x, ty - y ) 
	//の外積(法線)をとりその方向により判定する

	float cross = dx * (ty - y) - dy * (tx - x);

	return cross;
}


//　END
