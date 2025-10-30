#include <stdlib.h>
#define EXT

#include "../common/global.h"
#include "uv_check.h"
#include "mag_check.h"
//#include "stdio.h"
//#ifdef VS_DEBUG
//EXTERN int debug_logi_view;	//トレースするかしないか
//#endif

/*
	2021/12/2
	デバッグ用にCSV出力できるように

	2022/6/7
	UV_Fitnessレベル出力方法変更

	2022/7/15
	中間情報と結果ブロック変更

*/

u8 uv_maskpat[] = {// マスクパターン 25*25
				0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	0,	0,	0,	0,	0,	0,
				0,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	0,
				1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
				1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
				1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
				0,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	0,
				0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	0,	0,	0,	0,	0,	0,
};

s16 point_vicinity_cal_uv(ST_POINT_VICINITY *__restrict pv, u8 buf_num)
{
	ST_SPOINT spt = { 0 }; /*ピクセル関数に入れる*/
	ST_BS* pbs = work[buf_num].pbs;

	/*フィルタハーフサイズ*/
	u8 filter_half_size_x = (u8)pv->filter_size_x;// >> 1;
	u8 filter_half_size_y = (u8)pv->filter_size_y;// >> 1;
	
	u32 total = 0;	/*積和格納*/
	u32 addresult = 0;
	s16 current_y = 0;
	s16 current_x = 0;

	u8 mlt = 0;

	s16 err_code;

	spt.trace_flg = 0;

	/*入力論理プレーンを設定*/
	spt.p_plane_tbl = (enum P_Plane_tbl)pv->plane;

	//抽出エリアの中心点を中心座標に変換
	pv->x = pv->x - filter_half_size_x; //左上へ
	pv->y = pv->y - filter_half_size_y; //

	/*ポイント内のセンサーデータとフィルターの要素数で積和演算を行う*/
	for (current_y = 0; current_y < filter_half_size_y; current_y++)
	{
		for (current_x = 0; current_x < pv->filter_size_x; current_x++)
		{
			/*フィルターの要素数を取得*/
			mlt = *(pv->pfilter_pat + (u32)(current_y * pv->filter_size_x) + current_x);	

			/*0以外なら演算*/
			if (mlt != 0)
			{
				/*フィルター内位置決定*/
				spt.x = pv->x + current_x;
				spt.y = (pv->y + current_y);										// 	この行もloopの外に持っていける

				//傾き補正なし
				err_code = P_Getdt_8bit_only_MagUv(&spt, pbs);//画素値取得
				if (err_code)
				{
					spt.sensdt = 0;
				}
				if (err_code < 0)//大スキューでエラー回避　2021/07/28
				{
					spt.sensdt = 0;
				}

				addresult = spt.sensdt * mlt;/*センサーデータと要素数*/
				total += addresult;
			}
		}
	}

	/*出力計算*/
	pv->output = (u16)(total * pv->divide_val);
	return 0;
}
s32 get_mesh_data_uv(s32 x, s32 y, s32 plane, s32 diameter_x, s32 diameter_y, u8* mask, u8 buf_num, float divide_num)
{
	ST_POINT_VICINITY pv;
	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	ST_BS* pbs = work[buf_num].pbs;

	//L2P用のパラメタセット
	spt.l_plane_tbl = (u8)pnote_param->pplane_tbl[plane];

	/*論理座標から物理に変換*/
	//diameterはxyともに200dpi単位を乗算する
	spt.x = (x * diameter_x);		//seとleで式が異なる　修正必要　190920　furuta 
	spt.y = (y * diameter_x);
#ifdef VS_DEBUG_ITOOL
	for_iTool_trace_callback(&spt, pbs, pnote_param, diameter_x, diameter_y, mask,1);
#endif
	new_L2P_Coordinate_MagUv(&spt, pbs, pnote_param);
	pv.x = (s16)spt.x;
	pv.y = (s16)spt.y;

	//論理座標planeをテーブル引き
	pv.plane = (u8)pnote_param->pplane_tbl[plane];

	//マスクサイズとポインタの変更をお願い致します
	pv.filter_size_x = (u16)diameter_x;
	pv.filter_size_y = (u16)diameter_y;
	pv.pfilter_pat = (u8*)mask;

	//divide_valも予め計算して引数で渡すことにします。
	//その渡す値は逆数とする。　19/02/08 furuta
	pv.divide_val = divide_num;

	point_vicinity_cal_uv(&pv, buf_num);	//物理モード

	return pv.output;
}

//つかってない
//s32 get_uv_invalid_count(u8 buf_num, ST_UV* uv)
//{
//	s16 invalid_count[4] = { 0 };
//	s32 uv_p = 0;			// uv値の格納、しきい値との比較用
//	u8 plane = 0;			// パラ：プレーン番号
//	s32 x = 0;				// x座標
//	s32 y = 0;				// y座標
//	u16 plane_side = 0;			// パラ：プレーンのサイド
//	u32 average = 0;	// 中間情報用平均
//	s32 tmp = 0;
//	s32 err_count[4] = { 0 };
//	//s32 tmp_cnt[4] = { 0 };
//	s32 cnt[4] = { 0 };
//	s32 adjust_pt = 0;
//	u16 sid_ct = 0;
//	float divide = 0.0;
//	float level[4] = { 0.0 };
//	s16 coordinate[4] = { 0 };
//	u16 another_area = 0;
//	u16 area = 0;
//	u16 checked = 0;
//	u16 flag = 0;
//
//	for (plane_side = 0; plane_side < 2; plane_side++)
//	{
//		if (uv->ppoint[plane_side + sid_ct].x_s < -30 || uv->ppoint[plane_side + sid_ct].y_s < -30 || uv->ppoint[plane_side + sid_ct].x_e < -30 || uv->ppoint[plane_side + sid_ct].y_e < -30)
//		{
//			checked++;
//			break;
//		}
//			
//		if (uv->ppoint[plane_side].threshold3 == 2)
//		{
//			// UVが真券にある場合、こちらに入る（例：CNY） = VALIDATE
//			for (another_area = 0; another_area < 2; another_area++)
//			{
//				area = plane_side + sid_ct;
//				plane = uv_plane_tbl[plane_side];
//
//				if (uv->ppoint[area].y_s > uv->ppoint[area].y_e)
//				{
//					tmp = uv->ppoint[area].y_s;
//					coordinate[1] = uv->ppoint[area].y_e;
//					coordinate[3] = (s16)tmp;
//				}
//				else
//				{
//					coordinate[1] = uv->ppoint[area].y_s;
//					coordinate[3] = uv->ppoint[area].y_e;
//				}
//
//				if (uv->ppoint[area].x_s > uv->ppoint[area].x_e)
//				{
//					tmp = uv->ppoint[area].x_s;
//					coordinate[0] = uv->ppoint[area].x_e;
//					coordinate[2] = (s16)tmp;
//				}
//				else
//				{
//					coordinate[0] = uv->ppoint[area].x_s;
//					coordinate[2] = uv->ppoint[area].x_e;
//				}
//				//表裏の分岐
//				for (y = coordinate[1]; y <= coordinate[3]; y++)
//				{
//					for (x = coordinate[0] + adjust_pt; x <= coordinate[2] - adjust_pt; x++)
//					{
//						uv_p = get_mesh_data_uv(x, y, plane, uv->uv_mask_ptn_diameter_x, uv->uv_mask_ptn_diameter_y, uv_maskpat, buf_num, uv->uv_mask_ptn_divide_num);
//						if (uv_p < 0)
//						{
//							uv->result = (u8)3;
//							uv->level = (u8)1;
//							return -1;
//						}
//						average += uv_p;
//						cnt[area]++;
//					}
//				}
//				uv->average[area] = (float)average / (float)cnt[area];
//				uv->point_number++;
//				average = 0;
//				sid_ct++;
//			}// for another_area
//			
//			divide = uv->average[area - 1] / uv->average[area]; // divide = 蛍光反応のある領域 / 蛍光反応のない領域 
//			if (uv->average[area - 1] - uv->average[area] < UV_DIFFERENCE)
//			{
//				level[flag] = 1;
//			}
//			else
//			{
//				level[flag] = UV_STANDARD_LEVEL + (divide - uv->ppoint[plane_side].threshold1) * UV_MULTI_LEVEL;
//			}
//			
//			sid_ct--;
//		}
//		// UV反応がない紙幣を扱う場合、こっちに入る = FITNESS
//		else
//		{
//			if (uv->ppoint[area].y_s > uv->ppoint[area].y_e)
//			{
//				tmp = uv->ppoint[area].y_s;
//				coordinate[1] = uv->ppoint[area].y_e;
//				coordinate[3] = (s16)tmp;
//			}
//			else
//			{
//				coordinate[1] = uv->ppoint[area].y_s;
//				coordinate[3] = uv->ppoint[area].y_e;
//			}
//
//			if (uv->ppoint[area].x_s > uv->ppoint[area].x_e)
//			{
//				tmp = uv->ppoint[area].x_s;
//				coordinate[0] = uv->ppoint[area].x_e;
//				coordinate[2] = (s16)tmp;
//			}
//			else
//			{
//				coordinate[0] = uv->ppoint[area].x_s;
//				coordinate[2] = uv->ppoint[area].x_e;
//			}
//			// UV特徴が紙幣にない（または小さい）とき、こちらに入る（例：EUR）
//			if (area > 1)
//			{
//				plane = uv_plane_tbl[plane_side];
//			}
//			else
//			{
//				plane = uv_plane_tbl[plane_side];
//			}
//			//表裏の分岐
//			for (y = coordinate[1] ; y <= coordinate[3]; y++)
//			{
//				for (x = coordinate[0]; x <= coordinate[2]; x++)
//				{
//					uv_p = get_mesh_data_uv(x, y, plane, uv->uv_mask_ptn_diameter_x, uv->uv_mask_ptn_diameter_y, uv_maskpat, buf_num, uv->uv_mask_ptn_divide_num);
//					average += uv_p;
//					if ((float)uv_p > uv->ppoint[area].threshold1)
//					{
//						invalid_count[area]++;
//						err_count[area]++;
//					}
//					else if (uv_p < 0)
//					{
//						uv->result = (u8)4; // 配列に負の数が書き込まれたエラー
//						uv->level = (u8)1;
//						return -1;
//					}
//					cnt[area]++;
//					x += 3;
//				}
//				y++;
//			}
//			uv->average[area] = (float)average / (float)cnt[area];
//			uv->point_number++;
//			average = 0;
//
//			level[flag] = UV_STANDARD_LEVEL + (uv->ppoint[area].threshold1 - uv->average[area]) * UV_MULTI_LEVEL;
//		}
//		area++;
//		flag++;
//	}// for
//
//	if (checked == 1)
//	{
//		if (level[0] < 40)
//		{
//			uv->result = 3; // 両面異常を返す エラーなしを返す
//			level[0] = 1;
//		}
//		else if (level[0] > 100 )
//		{
//			uv->result = 0; // 正常
//			level[0] = 100;
//		}
//		else
//		{
//			uv->result = 0;
//		}
//		uv->level = (u8)level[0];
//	}
//	else
//	{
//		if (level[0] > 100.0)
//		{
//			level[0] = (float)100;
//		}
//		else if (level[0] <= 0)
//		{
//			level[0] = (float)1;
//		}
//
//		if (level[1] > 100.0)
//		{
//			level[1] = (float)100;
//		}
//		else if (level[1] <= 0)
//		{
//			level[1] = (float)1;
//		}
//
//		level[2] = (level[0] + level[1]) / 2;
//		if (level[2] < UV_STANDARD_LEVEL)
//		{
//			uv->result = (u8)3; // 両面異常を返す エラーなしを返す
//		}
//		else if (level[0] < UV_STANDARD_LEVEL && level[1] > UV_STANDARD_LEVEL)
//		{
//			uv->result = (u8)1; // 表面異常を返す
//		}
//		else if (level[1] < UV_STANDARD_LEVEL && level[0] > UV_STANDARD_LEVEL)
//		{
//			uv->result = (u8)1; // 裏面異常を返す
//		}
//		else
//		{
//			uv->result = (u8)0;
//		}
//		uv->level = (u8)level[2];
//	}
//	
//	return uv->result;
//}

// UV特徴が紙幣にない（または小さい）とき、こちらに入る（例：EUR）
s32 get_uv_fitness(u8 buf_num, ST_UV_VALIDATE* uv)
{
	s16 invalid_count[IMUF_POINT_NUMBER] = { 0 };
	s32 uv_p = 0;			// uv値の格納、しきい値との比較用
	u8 plane = 0;			// パラ：プレーン番号
	s32 x = 0;				// x座標
	s32 y = 0;				// y座標
	u16 point_cnt = 0;			// パラ：プレーンのサイド
	s32 average = 0;	// 中間情報用平均
	s32 tmp = 0;
	s32 err_count[IMUF_POINT_NUMBER] = { 0 };
	s32 cnt[IMUF_POINT_NUMBER] = { 0 };
	//s32 adjust_pt = 0;
	float level[IMUF_POINT_NUMBER] = { 0.0,0.0,0.0,0.0 };
	s16 coordinate[IMUF_POINT_NUMBER] = { 0 };
	u8 point_number = 0;
	u16 flag = 0;

#ifdef VS_DEBUG
	FILE *fp;
	if (work[buf_num].pbs->blank3 != 0)
	{
		fp = fopen("uv_fitness.csv", "a");
		fprintf(fp, "%s,%x,%d,%s,", work[buf_num].pbs->blank4, work[buf_num].pbs->mid_res_nn.result_jcm_id, work[buf_num].pbs->insertion_direction, work[buf_num].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf_num].pbs->blank0[20], work[buf_num].pbs->ser_num1, work[buf_num].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf_num].pbs->spec_code.model_calibration, work[buf_num].pbs->spec_code.sensor_conf, work[buf_num].pbs->spec_code.mode);
	}

#endif

	for (point_cnt = 0; point_cnt < 2; point_cnt++)
	{
		if (uv->ppoint[point_cnt].x_s < OUT_OF_RANGE|| uv->ppoint[point_cnt].y_s < OUT_OF_RANGE|| uv->ppoint[point_cnt].x_e < OUT_OF_RANGE|| uv->ppoint[point_cnt].y_e < OUT_OF_RANGE)
		{
			break;
		}
		// 検知する面の数を数える
		if (uv->ppoint[point_cnt].side == 0 || uv->ppoint[point_cnt].side == 1)
		{
			point_number++;
		}
		else
		{
			break;
		}

		if (uv->ppoint[point_cnt].y_s > uv->ppoint[point_cnt].y_e)
		{
			tmp = uv->ppoint[point_cnt].y_s;
			coordinate[1] = uv->ppoint[point_cnt].y_e;
			coordinate[3] = (s16)tmp;
		}
		else
		{
			coordinate[1] = uv->ppoint[point_cnt].y_s;
			coordinate[3] = uv->ppoint[point_cnt].y_e;
		}

		if (uv->ppoint[point_cnt].x_s > uv->ppoint[point_cnt].x_e)
		{
			tmp = uv->ppoint[point_cnt].x_s;
			coordinate[0] = uv->ppoint[point_cnt].x_e;
			coordinate[2] = (s16)tmp;
		}
		else
		{
			coordinate[0] = uv->ppoint[point_cnt].x_s;
			coordinate[2] = uv->ppoint[point_cnt].x_e;
		}
		
		plane = uv_plane_tbl[point_cnt];

		//表裏の分岐
		for (y = coordinate[1]; y <= coordinate[3]; y++)
		{
			for (x = coordinate[0]; x <= coordinate[2]; x++)
			{
				uv_p = get_mesh_data_uv(x, y, plane, uv->uv_mask_ptn_diameter_x, uv->uv_mask_ptn_diameter_y, uv_maskpat, buf_num, uv->uv_mask_ptn_divide_num);
				average += uv_p;
				if ((float)uv_p > uv->ppoint[point_cnt].threshold)
				{
					invalid_count[point_cnt]++;
					err_count[point_cnt]++;
				}
				else if (uv_p < 0)
				{
					uv->result = (u8)4; // 配列に負の数が書き込まれたエラー
					uv->level = (u8)1;
					return -1;
				}
				cnt[point_cnt]++;
				x += 3;
			}
			y++;
		}
		uv->average[point_cnt] = (float)average / (float)cnt[point_cnt];
		uv->point_number++;

#ifdef VS_DEBUG
		if (work[buf_num].pbs->blank3 != 0)
		{
			fprintf(fp, "%d,%d,%f,", average, cnt[point_cnt], uv->average[point_cnt]);
		}
#endif
		average = 0;

		level[point_cnt] = UV_STANDARD_LEVEL + (uv->ppoint[point_cnt].threshold - uv->average[point_cnt]) * UV_MULTI_LEVEL;

#ifdef VS_DEBUG
		if (work[buf_num].pbs->blank3 != 0)
		{
			fprintf(fp, "%f,", level[point_cnt]);
		}
#endif

		flag++;
	}
	uv->level = calc_uv_fitness_level(level[0], level[1], point_number, uv, flag);

#ifdef VS_DEBUG
	if (work[buf_num].pbs->blank3 != 0)
	{
		fprintf(fp, "%d,", uv->level);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif
	return uv->result;
}

// UVが真券にある場合、こちらに入る（例：CNY）
s32 get_uv_validate(u8 buf_num, ST_UV_VALIDATE* uv)
{
	//s16 invalid_count[4] = { 0 };
	s32 uv_p = 0;			// uv値の格納、しきい値との比較用
	u8 plane = 0;			// パラ：プレーン番号
	s32 x = 0;				// x座標
	s32 y = 0;				// y座標
	u16 point_cnt = 0;			// パラ：プレーンのサイド
	s32 average = 0;	// 中間情報用平均
	s32 tmp = 0;
	s32 cnt[IMUF_POINT_NUMBER] = { 0 };
	s32 adjust_pt = 0;
	float divide = 0.0;
	float level[IMUF_POINT_NUMBER] = { 0.0,0.0,0.0,0.0 };
	s16 coordinate[IMUF_POINT_NUMBER] = { 0 };
	u16 another_area = 0;
	u16 area = 0;
	u16 flag = 0;
	u8 point_number = 0;

#ifdef VS_DEBUG
	FILE *fp;
	if (work[buf_num].pbs->blank3 != 0)
	{
		fp = fopen("uv_validate.csv", "a");
		fprintf(fp, "%s,%x,%d,%s,", work[buf_num].pbs->blank4, work[buf_num].pbs->mid_res_nn.result_jcm_id, work[buf_num].pbs->insertion_direction,work[buf_num].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf_num].pbs->blank0[20], work[buf_num].pbs->ser_num1, work[buf_num].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf_num].pbs->spec_code.model_calibration, work[buf_num].pbs->spec_code.sensor_conf, work[buf_num].pbs->spec_code.mode);
	}
#endif

	for (point_cnt = 0; point_cnt < 2; point_cnt++)
	{
		if (uv->ppoint[point_cnt + 1].x_s < OUT_OF_RANGE|| uv->ppoint[point_cnt + 1].y_s < OUT_OF_RANGE|| uv->ppoint[point_cnt + 1].x_e < OUT_OF_RANGE|| uv->ppoint[point_cnt + 1].y_e < OUT_OF_RANGE)
		{
			break;
		}
		// 検知する面の数を数える
		if (uv->ppoint[point_cnt].side == 0 || uv->ppoint[point_cnt].side == 1)
		{
			point_number++;
		}
		else
		{
			break;
		}
		for (another_area = 0; another_area < 2; another_area++)
		{
			area = point_cnt + another_area;
			plane = uv_plane_tbl[uv->ppoint[area].side];

			if (uv->ppoint[area].y_s > uv->ppoint[area].y_e)
			{
				tmp = uv->ppoint[area].y_s;
				coordinate[1] = uv->ppoint[area].y_e;
				coordinate[3] = (s16)tmp;
			}
			else
			{
				coordinate[1] = uv->ppoint[area].y_s;
				coordinate[3] = uv->ppoint[area].y_e;
			}

			if (uv->ppoint[area].x_s > uv->ppoint[area].x_e)
			{
				tmp = uv->ppoint[area].x_s;
				coordinate[0] = uv->ppoint[area].x_e;
				coordinate[2] = (s16)tmp;
			}
			else
			{
				coordinate[0] = uv->ppoint[area].x_s;
				coordinate[2] = uv->ppoint[area].x_e;
			}
			//表裏の分岐
			for (y = coordinate[1]; y <= coordinate[3]; y++)
			{
				for (x = coordinate[0] + adjust_pt; x <= coordinate[2] - adjust_pt; x++)
				{
					uv_p = get_mesh_data_uv(x, y, plane, uv->uv_mask_ptn_diameter_x, uv->uv_mask_ptn_diameter_y, uv_maskpat, buf_num, uv->uv_mask_ptn_divide_num);
					if (uv_p < 0)
					{
						uv->result = (u8)3;
						uv->level = (u8)1;
						return -1;
					}
					average += uv_p;
					cnt[area]++;
				}
			}
			uv->average[area] = (float)average / (float)cnt[area];
			uv->point_number++;

#ifdef VS_DEBUG
			if (work[buf_num].pbs->blank3 != 0)
			{
				fprintf(fp, "%d,%d,%f,", average, cnt[area], uv->average[area]);
			}
#endif

			average = 0;
			cnt[area] = 0;
		}
		divide = uv->average[area - 1] / uv->average[area]; // divide = 蛍光反応のある領域 / 蛍光反応のない領域 
		if (uv->average[area - 1] - uv->average[area] < UV_DIFFERENCE)
		{
			level[flag] = 1;
		}
		else
		{
			level[flag] = UV_STANDARD_LEVEL + (divide - uv->ppoint[point_cnt].threshold) * UV_MULTI_LEVEL;
		}

#ifdef VS_DEBUG
		if (work[buf_num].pbs->blank3 != 0)
		{
			fprintf(fp, "%f,%f,", divide,level[flag]);
		}
#endif
		area++;
		flag++;
	}

	uv->level= calc_uv_level(level[0], level[1], point_number - 1, uv, flag);

#ifdef VS_DEBUG
	if (work[buf_num].pbs->blank3 != 0)
	{
		fprintf(fp, "%d,", uv->level);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif

	return uv->result;
}

u8 calc_uv_fitness_level(float level1, float level2, u8 point_number, ST_UV_VALIDATE* uv, u16 flag)
{
	float output = 0;

	if (point_number == 1)
	{
		output = level1;
		uv->result = (u8)1;
	}
	else
	{
		if (level1 <= 0 && level2 <= 0)
		{
			uv->result = (u8)3;
			output = 1;
		}
		else if (level1 <= level2)
		{
			uv->result = (u8)1;
			output = level1;
		}
		else if (level2 < level1)
		{
			uv->result = (u8)2;
			output = level2;
		}
		else
		{
			uv->result = (u8)4;
			output = level1;
		}
	}

	if (output > 100)
	{
		output = 100;
	}
	else if (output < 1)
	{
		output = 1;
	}
	

	return (u8)output;
}


u8 calc_uv_level(float level1, float level2, u8 point_number, ST_UV_VALIDATE* uv, u16 flag)
{
	float output = 0;
	// 不正な値を整理
	if (level1 > 100.0)
	{
		level1 = (float)100;
	}
	else if (level1 <= 0)
	{
		level1 = (float)1;
	}

	if (level2 > 100.0)
	{
		level2 = (float)100;
	}
	else if (level2 <= 0)
	{
		level2 = (float)1;
	}

	if (flag < 2)
	{
		level2 = 0;
		point_number = 1;
	}
	//レベルの平均を取る
	output = (level1 + level2) / point_number;

	if (output < UV_STANDARD_LEVEL)
	{
		uv->result = (u8)3; // 両面異常を返す エラーなしを返す
	}
	else if (level1 < UV_STANDARD_LEVEL && level2 > UV_STANDARD_LEVEL)
	{
		uv->result = (u8)1; // 表面異常を返す
	}
	else if (level2 < UV_STANDARD_LEVEL && level1 > UV_STANDARD_LEVEL)
	{
		uv->result = (u8)1; // 裏面異常を返す
	}
	else
	{
		uv->result = (u8)0;
	}

	if(output > 100)
	{
		output = 100;
	}
	else if(output < 1)
	{
		output = 1;
	}

	return (u8)output;
}
