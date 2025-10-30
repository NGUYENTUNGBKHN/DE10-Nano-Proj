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
 * @file hole.c
 * @brief 穴検知 ver2.61
 * @date 2020/9/18 Created.
 */
 /****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define EXT
#include "../common/global.h"

#ifdef VS_DEBUG
EXTERN	int debug_logi_view;	//トレースするかしないか
#endif

/*
	ver2.6
	・レベル設定MAX、MIN対応

	ver2.61
	個別デバッグ表示できるように
*/

/****************************************************************/
/**
* @brief		穴検知を開始します。
 *@param[in]	バッファ番号
 *@param[out]	なし
 * @return		なし
 */
 /****************************************************************/
s16 hole(u8 buf_num, ST_HOLE* ho)
{
	u8 ii;
	s16 temp = 0;
	s16	res = 0;
	ST_HOLE_SETTING setting = { 0 };

	if (ho->threshold_level_area == 0)	//レベル0で何もせず戻る　正券判定
	{
		return 0;//NORMAL_HOLE
	}

	//設定
	setting.pbs = work[buf_num].pbs;
	setting.para = &work[buf_num].note_param;
	setting.buf = buf_num;

	setting.ho = ho;

	setting.t_spt.l_plane_tbl = ho->t_plane;	//透過プレーン
	setting.t_spt.p_plane_tbl = ho->t_plane;
	setting.t_spt.way = work[buf_num].pbs->insertion_direction;

	setting.mmfordot = 1 / setting.pbs->PlaneInfo[ho->t_plane].main_element_pitch;	//探索間隔をmmからdotに変換する
	setting.mm2fordot = setting.mmfordot / setting.pbs->PlaneInfo[ho->t_plane].main_element_pitch;//mm2からdotへ

	setting.x_step = (s8)(ho->x_step * setting.mmfordot + 0.5);//穴位置探索間隔
	setting.y_step = (s8)(ho->y_step * setting.mmfordot + 0.5);
	setting.skip = ho->skip;
	setting.margin = ho->edge_margin;

	//検出座標記録をクリア
	initialize_hole_roi_buff(0, HOLE_ROI_BUFF, ho);

	//最大合計穴面積の設定
	setting.total_judge = (u16)(ho->threshold_total_area * setting.mm2fordot / setting.skip / setting.skip + 0.5);

	//検出合計面積上限
	setting.total_limit = (u16)(HOLE_AREA_LIMIT * setting.mm2fordot / setting.skip / setting.skip + 0.5);

	if (setting.total_judge > setting.total_limit)
	{
		setting.total_judge = setting.total_limit;
	}

	//最小検出穴面積の設定
	setting.min_area_dot = (u16)(ho->threshold_min_area * setting.mm2fordot / setting.skip / setting.skip + 0.5);

	//除外範囲の設定
	for (ii = 0; ii < ho->exclude_area_count; ii++)
	{
		setting.exlude[ii] = ho->exclude_hole[ii];
	}

	//論理座標での頂点座標を求める
	set_hole_lvertex(&setting, &setting.outline, &setting.outline_margin);

	setting.y_offset = setting.outline_margin.exclude_point_y1;

	//穴位置探索開始
	res = hole_lsearch(&setting);
	if (res < 0)
	{
		return 0;//ERR_HOLE;	//エラーで終了
	}

	//結果をdot
	ho->hole_count = setting.res_hole_count;
	ho->total_hole_area = setting.res_total_judge;

	//検知結果を入力　レベル判定がMAX_LEVELでUFになる
	if (setting.res_hole_count >= HOLE_MAX_COUNT)
	{
		//穴個数が設定値以上
		ho->result = TOO_MANY_HOLE;
		ho->level = MAX_LEVEL;
	}
	else if (setting.res_total_judge >= setting.total_limit)
	{
		//合計面積が設定値
		ho->result = TOTAL_LIMIT_OVER;
		ho->level = MAX_LEVEL;
	}

	//
	if (ho->result == 0)
	{
		//検出した穴で最大のものを求める
		for (ii = 0; ii < setting.res_hole_count; ii++)
		{
			if (ho->holes[ii] > temp)
			{
				temp = ho->holes[ii];
			}
		}

		if (temp > 0)
		{
			ho->max_hole_area = temp;
			ho->level = hole_level_detect(ho, &setting);//レベル計算
		}
		else
		{
			ho->max_hole_area = 0;
			ho->level = MIN_LEVEL;
			ho->result = 0;
		}
	}
	
	return 0;
}

/****************************************************************/
/**
* @brief		穴位置探索を開始する
 *@param[in]	穴検知設定値構造体
 *@param[out]	なし
 * @return		正券：0
				穴券：1
				エラー：-1
 */
 /****************************************************************/
s8 hole_lsearch(ST_HOLE_SETTING* setting)
{
	s8 res = 0;
	u8 temp = 0;
	s8 flag = 0;
	u8 vv = 0;
	u8 loop_out = 0;
	s8 x_step = setting->x_step;
	s8 y_step = setting->y_step;
	u8 thr = setting->ho->threshold;
	u8 exclude_area_count = setting->ho->exclude_area_count;
	s16 margin = (s16)(setting->margin * setting->mmfordot + 0.5);
	s16 y_current;
	s16 x_current;

	s16 y_start = set_min_remainder_0(setting->outline.exclude_point_y2 - margin, y_step);
	s16 y_end = set_max_remainder_0(setting->outline.exclude_point_y1 + margin, y_step);
	s16 x_start = set_max_remainder_0(setting->outline.exclude_point_x1 + margin, x_step);
	s16 x_end = set_min_remainder_0(setting->outline.exclude_point_x2 - margin, x_step);

	if (y_start > setting->outline_margin.exclude_point_y2 - y_step)
	{
		y_start = setting->outline_margin.exclude_point_y2 - y_step;
		y_end = setting->outline_margin.exclude_point_y1 + y_step;
	}
	if (x_start < setting->outline_margin.exclude_point_x1 + x_step)
	{
		x_start = setting->outline_margin.exclude_point_x1 + x_step;
		x_end = setting->outline_margin.exclude_point_x2 - x_step;
	}

	for (y_current = y_start; y_current >= y_end; y_current -= y_step)
	{
		for (x_current = x_start; x_current <= x_end; x_current += x_step)
		{

#ifndef VS_DEBUG
			temp = hole_thershold(setting, x_current, y_current, thr);

			if (temp == IN_HOLE)//穴と判定
			{
#endif
				//ROI範囲内なら、探索続行
				for (vv = 0; vv < setting->res_hole_count; vv++)
				{
					if (y_current >= setting->y_min_list[vv]
						&& y_current <= setting->y_max_list[vv]
						&& x_current >= setting->x_min_list[vv][(setting->y_max_list[vv] - y_current) / y_step]
						&& x_current <= setting->x_max_list[vv][(setting->y_max_list[vv] - y_current) / y_step])
					{
						loop_out = IN_HOLE;
						break;
					}
				}//for
				if (loop_out == IN_HOLE)
				{
					//x座標の現在地を検知した穴の範囲外まで移動して探索続行
					x_current = setting->x_max_list[vv][(setting->y_max_list[vv] - y_current) / y_step];
					loop_out = 0;
					continue;
				}

				//除外範囲判定
				if (exclude_area_count != 0)
				{
					flag = check_exlude(setting->exlude, x_current, y_current, exclude_area_count);
					if (flag == IN_HOLE)
					{
						continue;
					}
				}

#ifdef VS_DEBUG

				//穴判定
				temp = hole_thershold(setting, x_current, y_current, thr);
				if (temp == IN_HOLE)//穴と判定
				{
#endif
					//輪郭探索へ
					res = contour_setup(setting, x_current, y_current);
					if (res < 0)//エラー
					{
						return ERR_HOLE;
					}
					else if (res == UNFIT_HOLE)//穴券判定で探索終了
					{
						return UNFIT_HOLE;
					}
				}

			}//for_x

		}//for_y

		return 0;

	}

/****************************************************************/
/**
* @brief		輪郭探索準備・記録・結果の判定を行う
*@param[in]		穴検知設定値構造体、論理座標x、論理座標y
*@param[out]	検知した穴の範囲座標
* @return		0:正常終了　(穴位置探索を続行)
				1:穴判定　(穴位置探索を終了)
				-1:エラー終了　(穴位置探索を終了)
*/
/****************************************************************/
s8 contour_setup(ST_HOLE_SETTING* setting, s16 lx, s16 ly)
{
	u8 temp = 0;
	s8 res = 0;	//輪郭探索結果
	s8 check_count = 0;	//開始位置の通過回数
	s8 x_step = setting->x_step;
	s8 y_step = setting->y_step;
	u8 skip = setting->skip;
	u8 thr = setting->ho->threshold;

	s16 x_start = lx;	//輪郭探索開始座標
	s16 y_start = ly;
	s16 offset = setting->y_offset;
	s16 y_max = INI_MAX;//-4096
	s16 y_min = INI_MIN;//4095
	s16 xx, yy;
	u16 area_dot = 0;	//面積　dot
	s16 ii = 0;
	s16 count = 0;
	s16 yo_max, yo_min, temp_max, temp_min, t1, t2, num;

	//輪郭まで移動
	check_count = move_for_contour(setting, &x_start, &y_start);
	if (check_count < 0)
	{
		//エラー終了
		return ERR_HOLE;
	}

	//輪郭探索
	res = contour_lsearch(setting, x_start, y_start, &y_max, &y_min, check_count);
	if (res < 0)
	{
		//エラー終了
		return ERR_HOLE;
	}
	else if (res == ONE_DOT_FLAG_HOLE)
	{
		//輪郭探索終了
		return NORMAL_HOLE;
	}
	else if (res == OUTLINE_FLAG_HOLE)
	{
		//外形
		setting->ho->holes[setting->res_hole_count] = -(OUTLINE_FLAG_HOLE);
	}
	else
	{
		//面積を求める
		for (yy = y_min; yy <= y_max; yy++)
		{
			if (setting->ho->hole_min_buff[yy - offset] == INI_MIN || setting->ho->hole_max_buff[yy - offset] == INI_MAX)
			{
				continue;
			}

			for (xx = setting->ho->hole_min_buff[yy - offset]; xx <= setting->ho->hole_max_buff[yy - offset]; xx += skip)
			{
				temp = hole_thershold(setting, xx, yy, thr);
				if (temp == IN_HOLE)
				{
					area_dot++;
				}

			}//xfor

			//最大記録面積を超えた場合、穴券として穴検知終了
			if (area_dot + setting->res_total_judge >= setting->total_limit)
			{
				//合計面積記録
				setting->res_total_judge += area_dot;
				return UNFIT_HOLE;
			}
		}//yfor

		//合計面積記録
		setting->res_total_judge += area_dot;

		//最小検出面積以上の場合
		if (area_dot > setting->min_area_dot)
		{
			//検出面積記録
			setting->ho->holes[setting->res_hole_count] = area_dot;
		}
		else
		{
			//検出座標を初期化
			initialize_hole_roi_buff(y_min - offset, y_max - offset + 1, setting->ho);
			return NORMAL_HOLE;
		}
	}

	/*探索範囲を記録*/
	yo_max = set_max_remainder_0(y_max - offset, y_step);
	yo_min = set_min_remainder_0(y_min - offset, y_step);
	num = (yo_max - yo_min) / y_step + 1;

	//ROI記録用配列が埋まっていたらエラー終了
	if (setting->roi_list_last + num >= HOLE_ROI_LIST)
	{
		setting->ho->err_code = ERR_LIST_OVER;
		return ERR_HOLE;
	}

	for (ii = 0; ((yo_max - yo_min) / y_step - ii) >= 0; ii++)
	{
		temp_max = INI_MAX;
		temp_min = INI_MIN;
		if (yo_min + ((yo_max - yo_min) / y_step - ii) * y_step - (y_step - 1) < 0)
		{
			num--;
			break;
		}
		for (count = 0; count < y_step; count++)
		{
			t1 = (setting->ho->hole_max_buff[yo_min + ((yo_max - yo_min) / y_step - ii) * y_step - count]);
			t2 = (setting->ho->hole_min_buff[yo_min + ((yo_max - yo_min) / y_step - ii) * y_step - count]);
			if (t1 > temp_max)
			{
				temp_max = t1;
			}
			if (t2 < temp_min)
			{
				temp_min = t2;
			}
		}

		if ((temp_max == INI_MAX) && (temp_min == INI_MIN))
		{
			num--;
			break;
		}

		temp_max = set_max_remainder_0(temp_max, x_step);
		temp_min = set_min_remainder_0(temp_min, x_step);

		setting->ho->hole_max_list[setting->roi_list_last + ii] = temp_max;
		setting->ho->hole_min_list[setting->roi_list_last + ii] = temp_min;
	}

	//ROI用配列に探索範囲を記録
	setting->y_max_list[setting->res_hole_count] = set_max_remainder_0(y_max, y_step);
	setting->y_min_list[setting->res_hole_count] = set_min_remainder_0(y_min, y_step);
	setting->x_max_list[setting->res_hole_count] = &setting->ho->hole_max_list[setting->roi_list_last];
	setting->x_min_list[setting->res_hole_count] = &setting->ho->hole_min_list[setting->roi_list_last];
	setting->list_num[setting->res_hole_count] = setting->roi_list_last;
	setting->roi_list_last += num;
	/*記録終了*/

	//記録個数を増やす
	setting->res_hole_count++;

	//記録個数が最大値以上で穴検知終了
	if (setting->res_hole_count >= HOLE_MAX_COUNT)
	{
		return UNFIT_HOLE;
	}

	//検出座標を初期化
	initialize_hole_roi_buff(y_min - offset, y_max - offset + 1, setting->ho);

	return NORMAL_HOLE;

}

/****************************************************************/
/**
* @brief		輪郭探索を実行する
*@param[in]		穴検知設定値構造体
				輪郭探索開始座標x
				輪郭探索開始座標x
				最大・最小y座標
				開始位置通過回数
*@param[out]	検知した輪郭座標
				検知した座標の最大・最小y座標
* @return		 0:正常終了
				1:外形判定
				2:検知した穴が1dotのみ
				-1:エラー終了　(穴位置探索を終了)
*/
/****************************************************************/
s8 contour_lsearch(ST_HOLE_SETTING* setting, s16 x_start, s16 y_start, s16* ly_max, s16* ly_min, s8 check_count)
{
	u8 temp = 0;
	u8 thr = setting->ho->threshold;
	u8 skip = setting->skip;
	u8 exclude_area_count = setting->ho->exclude_area_count;
	s8 x_map[4] = { 0 };	//探索先座標　（論理座標）
	s8 y_map[4] = { 0 };	//［0］上方向［1］右方向［2］下方向［3］左方向
	u8 vlast[4] = { 3,0,1,2 };	//前の探索先番号
	u8 vnext[4] = { 1,2,3,0 };	//次の探索先番号
	s16 x_current = x_start;	//現在位置
	s16 y_current = y_start;
	s16 x_search = 0;			//探索先
	s16 y_search = 0;
	s16 y_max = *ly_max;
	s16 y_min = *ly_min;

	u8 vnum = 0;	//探索先番号
	u8 vrev = 0;	//一か所での探索数
	u8 flag = 0;
	s8 output = 0;	//
	u16 count = 0;	//探索移動回数
	s16 offset = setting->y_offset;

	x_map[0] = 0;
	x_map[1] = skip;
	x_map[2] = 0;
	x_map[3] = -skip;

	y_map[0] = skip;
	y_map[1] = 0;
	y_map[2] = -skip;
	y_map[3] = 0;

	while ((count < CONTOUR_SEARCH_MOVE_LIMIT) && (vrev < 4))
	{
		x_search = x_current + x_map[vnum];
		y_search = y_current + y_map[vnum];
		temp = hole_thershold(setting, x_search, y_search, thr);
		if (temp == IN_HOLE)
		{
			//外形判定
			flag = check_area(&setting->outline_margin, x_search, y_search);
			if (flag == OUT_HOLE)
			{
				if (output != OUTLINE_FLAG_HOLE)
				{
					flag = check_outline(setting, x_search, y_search, vnum);
					if (flag == OUT_HOLE)
					{
						output = OUTLINE_FLAG_HOLE;
					}
				}
				vnum = vnext[vnum];
				vrev++;
				continue;
			}

			//除外範囲判定
			if (exclude_area_count != 0)
			{
				flag = check_exlude(setting->exlude, x_search, y_search, exclude_area_count);
				if (flag == IN_HOLE)
				{
					vnum = vnext[vnum];
					vrev++;
					continue;
				}
			}

			//max,min更新
			if (y_max < y_current)
			{
				y_max = y_current;
			}
			if (y_min > y_current)
			{
				y_min = y_current;
			}
			if (setting->ho->hole_min_buff[(y_current - offset)] > x_current)
			{
				setting->ho->hole_min_buff[(y_current - offset)] = x_current;
			}
			if (setting->ho->hole_max_buff[(y_current - offset)] < x_current)
			{
				setting->ho->hole_max_buff[(y_current - offset)] = x_current;
			}

			//終了判定
			if (x_search == x_start && y_search == y_start)
			{
				//通過フラグが0以下の場合、終了
				if (check_count <= 0)
				{
					*ly_max = y_max;
					*ly_min = y_min;
					return output;
				}
				else
				{
					check_count--;
				}
			}

			x_current = x_search;	//探索先に移動
			y_current = y_search;
			vnum = vlast[vnum];		//探索番号を一つ前に
			vrev = 0;				//回転数リセット
			count++;
		}
		else
		{
			vnum = vnext[vnum];	//探索番号を進める
			vrev++;
		}

	}//while

	//エラー
	if (vrev >= 4)
	{
		//1dotのみだった場合
		if ((x_current == x_start) && (y_current == y_start))
		{
			return ONE_DOT_FLAG_HOLE;
		}

		//輪郭から外れたため、エラー
		setting->ho->err_code = ERR_CONTOUR_LOST;
		return ERR_HOLE;
	}

	if (count >= CONTOUR_SEARCH_MOVE_LIMIT)
	{
		//輪郭探索移動回数が規定以上でエラー
		setting->ho->err_code = ERR_CONTOUR_LOOP;
		return ERR_HOLE;
	}

	setting->ho->err_code = ERR_CONTOUR_LOOP;
	return ERR_HOLE;

}

/****************************************************************/
/**
* @brief		開始位置を輪郭まで移動する
				開始位置周辺の閾値をチェックし、特定の箇所が閾値以下なら輪郭探索での通過フラグを立てる
*@param[in]		穴検知設定値構造体、論理座標x、論理座標y
*@param[out]	輪郭まで移動した論理座標x、論理座標y
* @return		輪郭探索開始位置通過フラグ：0～3
				-1:エラー終了　(穴検知を終了)
*/
/****************************************************************/
s8 move_for_contour(ST_HOLE_SETTING* setting, s16* lx, s16* ly)
{
	u8 res = 0;
	u8 skip = setting->skip;
	u8 flag = 0;
	u8 ii = 0;
	s16 x_search = 0;
	s16 y_search = 0;
	s16 x_current = *lx;
	s16 y_current = *ly;

	s8 x_map[6] = { 0 };
	s8 y_map[6] = { 0 };

	s8 check_map[6] = { 0 };
	s8 check_flag = 0;
	u8 thr = setting->ho->threshold;
	u8 exclude_area_count = setting->ho->exclude_area_count;
	u16 count = 0;

	x_map[0] = 0;
	x_map[1] = skip;
	x_map[2] = 0;
	x_map[3] = -skip;
	x_map[4] = skip;
	x_map[5] = -skip;

	y_map[0] = skip;
	y_map[1] = 0;
	y_map[2] = -skip;
	y_map[3] = 0;
	y_map[4] = -skip;
	y_map[5] = -skip;

	/*輪郭まで移動*/
	while (count < MOVE_FOR_CONTOUR_MOVE_LIMIT)
	{
		x_search = x_current + x_map[0];
		y_search = y_current + y_map[0];
		res = hole_thershold(setting, x_search, y_search, thr);
		if (res == IN_HOLE)
		{
			flag = check_area(&setting->outline_margin, x_search, y_search);
			if (flag == OUT_HOLE)
			{
				break;
			}

			if (exclude_area_count != 0)
			{
				flag = check_exlude(setting->exlude, x_search, y_search, exclude_area_count);
				if (flag == IN_HOLE)
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
		x_current = x_search;
		y_current = y_search;
		count++;

	}//while

	//輪郭から外れたため、エラー
	if (count >= MOVE_FOR_CONTOUR_MOVE_LIMIT)
	{
		setting->ho->err_code = ERR_CONTOUR_LOST;
		return ERR_HOLE;
	}

	//開始位置変更
	*lx = x_current;
	*ly = y_current;

	/*周辺チェック*/
	//開始位置の上下左右　右下左下が穴の閾値以上かチェック
	for (ii = 0; ii < 6; ii++)
	{
		x_search = x_current + x_map[ii];
		y_search = y_current + y_map[ii];
		res = hole_thershold(setting, x_search, y_search, thr);
		if (res == IN_HOLE)
		{
			//
			check_map[ii] = 1;

			//外形
			flag = check_area(&setting->outline_margin, x_search, y_search);
			if (flag == OUT_HOLE)
			{
				check_map[ii] = 2;
			}

			//除外範囲
			if (exclude_area_count != 0)
			{
				flag = check_exlude(setting->exlude, x_search, y_search, exclude_area_count);
				if (flag == IN_HOLE)
				{
					check_map[ii] = 2;
				}
			}
		}
	}//for

	//上下左右で閾値以上の箇所が1つ以上
	if ((check_map[0] == 1) || (check_map[1] == 1) || (check_map[2] == 1) || (check_map[3] == 1))
	{
		//輪郭探索が開始位置を複数回通過しないかチェック
		//右下が閾値以下かつ右が閾値以上
		if (check_map[4] == 0 && check_map[1] == 1)
		{
			check_flag++;
		}
		//左下が閾値以下かつ左が閾値以上
		if (check_map[5] == 0 && check_map[3] == 1)
		{
			check_flag++;
		}
		//右下が閾値以下かつ左下が閾値以下かつ下が閾値以上
		if (check_map[4] == 0 && check_map[5] == 0 && check_map[2] == 1)
		{
			check_flag++;
		}
	}

	return check_flag;

}

/****************************************************************/
/**
* @brief		座標が角折れか裂けか判定する
*@param[in]		設定範囲(ST_EXCLUDE_HOLE*)、論理座標x、論理座標y
*@param[out]	なし
* @return		1:　範囲内
				0:　範囲外
*/
/****************************************************************/
s8 check_outline(ST_HOLE_SETTING* setting, s16 lx, s16 ly, s8 vnum)
{
	u8 res = 0;
	u8 skip = setting->skip;
	u8 flag = 0;
	u16 count = 0;
	s16 x_search = 0;
	s16 y_search = 0;
	s16 x_current = lx;
	s16 y_current = ly;
	s8 x_map[4] = { 0 };
	s8 y_map[4] = { 0 };
	s8 margin_1mm = (s8)(setting->mmfordot + 0.5);
	u8 step;
	u8 thr = setting->ho->threshold;

	x_map[0] = 0;
	x_map[1] = skip;
	x_map[2] = 0;
	x_map[3] = -skip;

	y_map[0] = skip;
	y_map[1] = 0;
	y_map[2] = -skip;
	y_map[3] = 0;


	if (vnum == 0 || vnum == 2)
	{
		step = setting->y_step + margin_1mm;
	}
	else
	{
		step = setting->x_step + margin_1mm;
	}

	/*外形まで移動*/
	while (count < step)
	{
		x_search = x_current + x_map[vnum];
		y_search = y_current + y_map[vnum];
		res = hole_thershold(setting, x_search, y_search, thr);
		if (res == IN_HOLE)//閾値以上
		{
			flag = check_area(&setting->outline, x_search, y_search);
			if (flag == OUT_HOLE)//外形外
			{
				//閾値以上かつ外形外
				return OUT_HOLE;
			}
		}
		else//閾値以下
		{
			return IN_HOLE;
		}

		x_current = x_search;
		y_current = y_search;
		count++;

	}//while

	return OUT_HOLE;

}

/****************************************************************/
/**
* @brief		座標がST_EXCLUDE_HOLEで設定した矩形内に存在するか判定する
*@param[in]		設定範囲(ST_EXCLUDE_HOLE*)、論理座標x、論理座標y
*@param[out]	なし
* @return		1:　範囲内
				0:　範囲外
*/
/****************************************************************/
s8 check_area(ST_EXCLUDE_HOLE* rectangle, s16 lx, s16 ly)
{
	if ((lx > rectangle->exclude_point_x1) && (lx < rectangle->exclude_point_x2)
		&& (ly > rectangle->exclude_point_y1) && (ly < rectangle->exclude_point_y2))
	{
		//in
		return IN_HOLE;
	}
	else
	{
		//out
		return OUT_HOLE;
	}
}

/****************************************************************/
/**
* @brief		座標が除外範囲内に存在するか判定する
*@param[in]		設定範囲(ST_EXCLUDE_HOLE*)、論理座標x、論理座標y
*@param[out]	なし
* @return		1:　範囲内
				0:　範囲外
*/
/****************************************************************/
s8 check_exlude(ST_EXCLUDE_HOLE* exclude, s16 lx, s16 ly, u8 num)
{
	u8 ii;
	u8 flag;

	for (ii = 0; ii < num; ii++)
	{
		flag = check_area(&exclude[ii], lx, ly);

		if (flag == IN_HOLE)
		{
			//in
			return IN_HOLE;
		}
	}

	//out
	return OUT_HOLE;
}

/****************************************************************/
/**
* @brief		座標が穴とする閾値以上か判定する
*@param[in]		穴検知設定値構造体、論理座標x、論理座標y
*@param[out]	なし
* @return		0:　閾値以下
				1:　閾値以上
*/
/****************************************************************/
u8 hole_thershold(ST_HOLE_SETTING* setting, s16 lx, s16 ly, u8 thr)
{
	u8 temp = OUT_HOLE;
	//u16 res;

	setting->t_spt.x = lx;
	setting->t_spt.y = ly;
	new_L_Getdt_08bit_only(&setting->t_spt, setting->pbs, setting->para);

	if (setting->t_spt.sensdt >= thr)
	{
		temp = IN_HOLE;
	}

	return temp;
}

/****************************************************************/
/**
* @brief		論理座標上での媒体頂点を求めて記録する。
				また、マージンを引いた媒体頂点を求めて記録する。
*@param[in]		穴検知設定値構造体, 頂点座標記録構造体,頂点座標記録構造体
*@param[out]	論理座標での頂点座標
			マージンを引いた分の論理座標での頂点座標
* @return		なし
*/
/****************************************************************/
void set_hole_lvertex(ST_HOLE_SETTING* setting, ST_EXCLUDE_HOLE* outline, ST_EXCLUDE_HOLE* outline_margin)
{
	//s16 res;
	u8 margin_1mm = (u8)(setting->mmfordot + 0.5);
	ST_VERTEX pver = { 0 };

	//外形を記録
	get_vertex(&pver, setting->buf);

	outline->exclude_point_x1 = pver.left_up_x;
	outline->exclude_point_x2 = pver.right_up_x;
	outline->exclude_point_y1 = pver.left_down_y;
	outline->exclude_point_y2 = pver.left_up_y;

	outline_margin->exclude_point_x1 = set_max_remainder_0(pver.left_up_x + margin_1mm, setting->x_step);
	outline_margin->exclude_point_x2 = set_min_remainder_0(pver.right_up_x - margin_1mm, setting->x_step);
	outline_margin->exclude_point_y1 = set_max_remainder_0(pver.left_down_y + margin_1mm, setting->y_step);
	outline_margin->exclude_point_y2 = set_min_remainder_0(pver.left_up_y - margin_1mm, setting->y_step);

}

/****************************************************************/
/**
* @brief		輪郭座標を記録した配列を初期化する
*@param[in]	初期化する開始番号、終了番号
*@param[out]	なし
* @return		なし
*/
/****************************************************************/
void initialize_hole_roi_buff(s16 from, s16 to, ST_HOLE* ho)
{
	s16 ii;
	s16* p_max = ho->hole_max_buff;
	s16* p_min = ho->hole_min_buff;

	for (ii = from; ii < to; ii++)
	{
		p_max[ii] = INI_MAX;//-4096
		p_min[ii] = INI_MIN;//4095
	}

}

/****************************************************************/
/**
* @brief		商を増加させて剰余を0にする
*@param[in]	被除数、除数
*@param[out]	なし
* @return		増加させた被除数
*/
/****************************************************************/
s16 set_max_remainder_0(s16 value, s8 step)
{
	s16 temp;

	if (value % step == 0)
	{
		return value;
	}

	temp = value;
	if (value > 0)
	{
		temp += step - (temp % step);
	}
	else
	{
		temp -= (temp % step);
	}

	return temp;
}

/****************************************************************/
/**
* @brief		商を減少させて剰余を0にする
*@param[in]	被除数、除数
*@param[out]	なし
* @return		減少させた被除数
*/
/****************************************************************/
s16 set_min_remainder_0(s16 value, s8 step)
{
	s16 temp;

	if (value % step == 0)
	{
		return value;
	}

	temp = value;
	if (value > 0)
	{
		temp -= (temp % step);
	}
	else
	{
		temp -= step + (temp % step);
	}

	return temp;
}


/****************************************************************/
/**
* @brief		検出した面積のレベルを求める
*@param[in]	穴検知構造体、穴検知設定値構造体
*@param[out]	なし
* @return		穴面積出力レベル
*/
/****************************************************************/
u8 hole_level_detect(ST_HOLE* ho, ST_HOLE_SETTING* setting)
{

#define MIN_LIMIT_NUM_HOLE_AREA 3.4
#define MAX_LIMIT_NUM_HOLE_AREA 16.6

#define MIN_LIMIT_NUM_HOLE_TOTAL_AREA 3.4

	float detect_res_ary;
	float thr_ary;

	u8 hole_level = 0;
	u8 total_level = 0;
	u8 temp = 0;
	u16 hole_level_area_dot = 0;
	u16 min_limit_hole_area = 0;
	u16 max_limit_hole_area = 0;
	u16 min_limit_hole_total_area = 0;
	u16 max_limit_hole_total_area = 0;

	//mm^2をdotに変換
	hole_level_area_dot = (u16)(ho->threshold_level_area * setting->mm2fordot / setting->skip / setting->skip + 0.5);
	min_limit_hole_area = (u16)(MIN_LIMIT_NUM_HOLE_AREA * setting->mm2fordot / setting->skip / setting->skip + 0.5);
	max_limit_hole_area = (u16)(MAX_LIMIT_NUM_HOLE_AREA * setting->mm2fordot / setting->skip / setting->skip + 0.5);
	min_limit_hole_total_area = (u16)(MIN_LIMIT_NUM_HOLE_TOTAL_AREA * setting->mm2fordot / setting->skip / setting->skip + 0.5);
	max_limit_hole_total_area = (u16)(HOLE_AREA_LIMIT * setting->mm2fordot / setting->skip / setting->skip + 0.5);

	//一番大きい穴のみレベルを求める
	detect_res_ary = (float)ho->max_hole_area;
	thr_ary = (float)hole_level_area_dot;

	//レベルを求める
	hole_level = level_detect(&detect_res_ary, &thr_ary, 1 , min_limit_hole_area, max_limit_hole_area);

	//
	detect_res_ary = (float)setting->res_total_judge;
	thr_ary = (float)setting->total_judge;

	//レベルを求める
	total_level = level_detect(&detect_res_ary, &thr_ary, 1, min_limit_hole_total_area, max_limit_hole_total_area);
	if (total_level == MIN_LEVEL && hole_level == MIN_LEVEL)
	{
		temp = MIN_LEVEL;
		ho->result = 0;
	}
	else if (total_level < hole_level)
	{
		temp = total_level;
		ho->result = TOTAL_OVER;
	}
	else if (total_level >= hole_level)
	{
		temp = hole_level;
		ho->result = LEVEL_OVER;
	}

	return temp;
}
