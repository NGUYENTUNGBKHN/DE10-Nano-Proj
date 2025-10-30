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
* @file dog_ear.c
* @brief 角折れ検知
* @date 2018/06/29 Created.
*
*	19/11/22：マルチコア対応の為、専用構造体をグローバルからローカル変数にする。　機能的な変更はない。
*/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EXT
#include "../common/global.h"

#define DOG_LEVEL_ON	//有効にすることで、レベル計算を行う。


//#define DOG_EAR_LEARNING 1		//実機の時はコメントアウトしてください。



//#define CSV_DB_DOG

s16 dog_ear(u8 buf_num ,ST_DOG_EAR *dog)
{
	//頂点座標用構造体
	ST_VERTEX pver;					//16byte
	ST_BS* pbs = work[buf_num].pbs;	//20byte

	//関数の戻り値格納用
	//s8 status=0;					//21byte

	//基準設定
	//基準折れ範囲のmmをdotに変換
	dog->threshold_short_side_dot = dog->threshold_short_side_mm / dog->ele_pitch_x;
	dog->threshold_long_side_dot = dog->threshold_long_side_mm / dog->ele_pitch_x;

	//面積の設定
	dog->threshold_area = (u32)(dog->threshold_area / dog->ele_pitch_x / dog->ele_pitch_y);

	//バッファ番号設定
	dog->buf_num = buf_num;

	//変数の初期化
	ini_dog(dog);

	//頂点座標取得
	get_vertex(&pver ,dog->buf_num);

	//左上
	dog->judge[upper_left] = left_upper(pver.left_up_x ,pver.left_up_y ,SCAN_THR ,pbs ,dog);

	//左下
	dog->judge[lower_left] = left_lower(pver.left_down_x ,pver.left_down_y ,SCAN_THR ,pbs ,dog);

	//右上
	dog->judge[upper_right] = right_upper(pver.right_up_x ,pver.right_up_y ,SCAN_THR ,pbs ,dog);

	//右下
	dog->judge[lower_right] = right_lower(pver.right_down_x ,pver.right_down_y ,SCAN_THR ,pbs ,dog);

	//結果の単位をdotからミリに変換します。
	unit_convert(dog->ele_pitch_x, dog->ele_pitch_y ,dog);

#ifdef DOG_LEVEL_ON
	//レベルを計算します
	dog->level = dog_level_detect(dog);

#endif

	return 0;	//角折れ検知終了

}//dog_ear end

//左上の角折れを検知します
u8 left_upper(s16 ver_a_x, s16 ver_a_y, s16 threshold, ST_BS *pbs ,ST_DOG_EAR *dog)
{
	s16 current_x = 0;
	s16 x = 0;
	s16 y = 0;
	u16 side_x_size = 0;
	u16 side_y_size = 0;
	u8 point_count = 0;	//11byte

	s16 ver_b_x = 0;
	s16 ver_b_y = 0;

	s16 ver_c_x = 0;
	s16 ver_c_y = 0;	//19

	u8 movement_y = 0;
	s16	status = 0;		//21

	s16 search_x = 0;
	s16 physical_x = 0;	//25
	u8  gaba_flg = 0;	//26

	u8  i = 0;
	float slope = 0;
	float offset = 0;	//35
	//u8 current_rev_stayt = 0;	//36
	u8 rev_count = 0;	//すぐにぶつかったフラぐ

	ST_SPOINT spt;		//14
	ST_NOTE_PARAMETER* pnote_param =  &work[dog->buf_num].note_param;	//4

	s16 count_list_cord_x[MAX_SEARCH_RANGE] = {0};	//55
	s16 count_list_cord_y[MAX_SEARCH_RANGE] = {0};	//375

	//Lgetdtのパラメタ設定
	spt.l_plane_tbl = dog->plane;
	spt.way = pbs->insertion_direction;

	//頂点座標を変数に代入
	x = ver_a_x;
	y = ver_a_y;

	//ｙ方向の移動量
	movement_y = dog->scan_move_y;

	y -= (s16)((float)movement_y * 1.5f);	//開始位置をさげる

	//頂点から探索　x方向に探索します
	for(i = 1; i < MAX_SEARCH_RANGE; i++)
	{
		y -= movement_y;	//開始位置をさらにさげる
		gaba_flg = 0;		//ゴミかどうかのフラグ初期化
		//current_rev_stayt = 0;	//探索開始ポイントに紙幣が存在するフラグ

		//x方向へ探索
		for(current_x = x; current_x < 0; current_x++)
		{

			spt.x = current_x;
			spt.y = y;

			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			physical_x = (s16)spt.x;			//物理座標を記録

			//媒体にぶつかるまで探索します。
			if(threshold > spt.sensdt)
			{
				gaba_flg = 0;	//ゴミフラグ初期化

				if(i >  1 && current_x == x )	//探索開始後すぐにぶつかった場合
				{
					x = ver_a_x;				//ポジションを頂点へ戻す
					//current_rev_stayt = 1;		//探索開始ポイントに紙幣が存在するフラグ
					rev_count += 1;				//すぐにぶつかったフラグをインクリメント
					break;
				}

				//ぶつかったものが媒体なのかゴミなのかを判定する
				////GARBAGE_OR_NOTE_RANGEの分だけX方向に探索し閾値1度でも上回ったらぶつかったものはゴミだとみなし再び探索を再開する。
				for(search_x = current_x + 1 ; search_x < current_x + dog->garbage_or_note_range; search_x++)
				{
					spt.x = search_x;
					spt.y = y;

					new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

					if(threshold < spt.sensdt)	//一度でも上回れば
					{
						gaba_flg = 1;			//ごみとみなす
					}
				}
				////ゴミじゃなかったとき
				if(gaba_flg == 0 )
				{
					point_count++;		//ポイント数をカウントします

					//ぶつかった位置を記録します
					count_list_cord_x[point_count] = current_x - 2;
					count_list_cord_y[point_count] = y;

					//次の探索開始位置をぶつかった位置からマイナスscan_move_x（dot）ずらします。処理時間短縮を図ります
					x = count_list_cord_x[point_count] - dog->scan_move_x;
					break;
				}

			}
		}

		if(rev_count == REV_MAX_NUM)	//
		{

			dog->judge_reason[upper_left] = RES_COME_BACK_VERTER;
			break;	//頂点に帰ってきた回数がREVERSE_MAX_COUNT（5回目）と同じになった
		}

		//記録した座標が搬送エリア外か？
		if(pbs->PlaneInfo[dog->plane].main_effective_range_max - 1 <= physical_x || pbs->PlaneInfo[dog->plane].main_effective_range_min >= physical_x)	//搬送外に出た角を破れと判定しないように
		{
			dog->judge_reason[upper_left] = RES_OUT_OF_AREA;
			break;	//搬送エリア外ならばブレイク
		}

		//閾値ラインを超えたか？
		if(ver_a_x - dog->scan_move_x_thr_line >= x)	//頂点より左？
		{
			break;	//探索終了
		}

		//if(i > 5 && current_rev_stayt == 1)	//探索開始n回目で探索開始位置に紙幣があったらそれはもう裂けと判断する
		//{
		//	//破れと判定	引きちぎられたもの
		//	//return TEAR;
		//}
	}

	if(point_count < dog->scan_minimum_points)	//ポイント数5以下	5
	{
		//角折れなしと判定
		dog->judge_reason[upper_left] = RES_POINT_COUNT_LESS;
		return NORMAL;
	}

	// 最小二乗法の計算	斜辺の式を求めます
    approx_line(count_list_cord_x, count_list_cord_y, point_count - 1, &offset, &slope);

	if(slope <= 0)	//傾きチェック　傾きが負ならば
	{
		//破れと判定	引きちぎられたもの
		dog->judge_reason[upper_left] = RES_SLOPE_OP;
		return TEAR;
	}

	//頂点計算　点C
	ver_c_x = (s16)((ver_a_y - offset) / slope);
	ver_c_y = ver_a_y;

	dog->triangle_vertex_1[upper_left][0] = ver_c_x;
	dog->triangle_vertex_1[upper_left][1] = ver_c_y;

	//頂点計算　点B
	ver_b_x = ver_a_x;
	ver_b_y = (s16)(slope * ver_b_x + offset);

	dog->triangle_vertex_2[upper_left][0] = ver_b_x;
	dog->triangle_vertex_2[upper_left][1] = ver_b_y;

	//辺の長さ
	side_x_size = (u16)(abs(ver_a_x - ver_c_x));
	side_y_size = (u16)(abs(ver_a_y - ver_b_y));

	//計測した長さをdotで格納
	dog->len_x[upper_left] = side_x_size;
	dog->len_y[upper_left] = side_y_size;

	//短辺長辺比較
	if(side_x_size >= side_y_size)
	{
		dog->long_side[upper_left] = side_x_size;
		dog->short_side[upper_left] = side_y_size;
	}
	else
	{
		dog->short_side[upper_left] = side_x_size;
		dog->long_side[upper_left] = side_y_size;
	}

	//綺麗な角折れか破れを検知します。
	//斜辺が近似直線に沿っているかを調べる
	//返り値 0：正常な角折れ -3：破れ
	status = triangle_inspection(pbs ,slope ,offset , ver_b_x ,ver_c_x ,1 ,dog);
	if(status != DOG_EAR)
	{
		dog->judge_reason[upper_left] = RES_SLOPE_IS_ABNORMAL;
		//ここで多角形の面積
		//dog->area[upper_left] = area_calculation(count_list_cord_x ,count_list_cord_y ,point_count);
		return TEAR;
	}

	//三角形だとみなされたので
	//この頂点の角折れもしくは角裂けの面積を求める
	dog->area[upper_left] = (u32)(dog->short_side[upper_left] * dog->long_side[upper_left] / 2);


	//角折れか破れかを調べます
	//返り値 0：正常な角折れ 3：破れ
	status = cal_symmetric_center_point(ver_a_x ,ver_a_y ,ver_b_x ,ver_b_y ,ver_c_x ,ver_c_y ,slope,offset, pbs ,upper_left ,dog);
	if(status != DOG_EAR)
	{
		dog->judge_reason[upper_left] = RES_HISTOGRAM_PEAKS_MATCH;
		return TEAR;
	}

	//基準比較
	status = judge_dog_ear(dog->short_side[upper_left], dog->long_side[upper_left] , &dog->area[upper_left] ,dog);
	if(status != DOG_EAR)	//角折れはあるが基準をクリア
	{
		return LITTLE_DOG_EAR;
	}

	//角折れ
	return DOG_EAR;

}

//左下の角折れを検知します
u8 left_lower(s16 ver_a_x, s16 ver_a_y, s16 threshold, ST_BS *pbs ,ST_DOG_EAR *dog)
{
	s16 physical_x = 0;
	s16 current_x = 0;
	s16 x = 0;
	s16 y = 0;
	u16 side_x_size = 0;
	u16 side_y_size = 0;
	u8 point_count = 0;

	s16 ver_b_x = 0;
	s16 ver_b_y = 0;

	s16 ver_c_x = 0;
	s16 ver_c_y = 0;

	u8 movement_y = 0;
	s16	status = 0;
	s16 count_list_cord_x[MAX_SEARCH_RANGE] = {0};	//55
	s16 count_list_cord_y[MAX_SEARCH_RANGE] = {0};	//375

	u8  i = 0;
	float slope = 0;
	float offset = 0;
	u8 rev_count = 0;
	u8  gaba_flg = 0;
	s16 search_x = 0;
	ST_SPOINT spt;
	//u8 current_rev_stayt = 0;

	ST_NOTE_PARAMETER* pnote_param =  &work[dog->buf_num].note_param;

	//Lgetdtのパラメタ設定
	spt.l_plane_tbl = dog->plane;
	spt.way = pbs->insertion_direction;

	////x頂点座標を正確に検知します。
	//for(current_x = ver_a_x; current_x < 0; current_x++)
	//{
	//	spt.x = current_x;
	//	spt.y = 0;

	//	new_L_Getdt(&spt,pbs ,note_para);

	//	//媒体にぶつかるまで探索します。
	//	if(threshold < spt.sensdt)
	//	{
	//		x = current_x;
	//		break;
	//	}
	//}

	////ｙ頂点座標を正確に検知します。
	//for(current_y = ver_a_y; current_y < 0; current_y++)
	//{
	//	spt.x = 0;
	//	spt.y = current_y;

	//	new_L_Getdt(&spt,pbs ,note_para);

	//	//媒体にぶつかるまで探索します。
	//	if(threshold < spt.sensdt)
	//	{
	//		y = current_y;
	//		break;
	//	}

	//}

	////頂点を更新します。
	//if(x != 0 && y != 0)
	//{
	//	ver_a_x = x;
	//	ver_a_y = y;
	//}

	x = ver_a_x;
	y = ver_a_y;

	//ｙ方向の移動量
	movement_y = dog->scan_move_y;
	y += (s16)((float)movement_y * 1.5f);	//

	//頂点から探索　x方向に探索します
	for(i = 1; i < MAX_SEARCH_RANGE; i++)
	{
		y += movement_y;	//
		gaba_flg = 0;
		//current_rev_stayt = 0;	//探索開始ポイントに紙幣が存在するフラグ

		//x方向へ探索
		for(current_x = x; current_x < 0; current_x++)
		{
			spt.x = current_x;
			spt.y = y;

			new_L_Getdt(&spt ,pbs ,pnote_param);

			physical_x = (s16)spt.x;

			//媒体にぶつかるまで探索します。
			if(threshold > spt.sensdt)
			{
				gaba_flg = 0;

				if(i >  1 && current_x == x )	//すぐにぶつかった場合
				{
					x = ver_a_x;	//ポジションをリセット
					//current_rev_stayt = 1;		//探索開始ポイントに紙幣が存在するフラグ
					rev_count += 1;
					break;
				}

				//ぶつかったものが媒体なのかゴミなのかを判定する
				//GARBAGE_OR_NOTE_RANGEの分だけX方向に探索し閾値1度でも上回ったらぶつかったものはゴミだとみなし再び探索を再開する。
				for(search_x = current_x + 1 ; search_x < current_x + dog->garbage_or_note_range; search_x++)
				{
					spt.x = search_x;
					spt.y = y;

					new_L_Getdt(&spt ,pbs ,pnote_param);

					if(threshold < spt.sensdt)	//一度も上回らなければ
					{
						gaba_flg = 1;
					}
				}

				if(gaba_flg == 0 )
				{
					point_count++;		//ポイント数をカウントします

					//ぶつかった位置を記録します
					count_list_cord_x[point_count] = current_x;
					count_list_cord_y[point_count] = y;

					//次の探索開始位置をぶつかった位置 - scan_move_xとし、処理時間短縮を図ります
					x = count_list_cord_x[point_count] - dog->scan_move_x;
					break;
				}

			}
		}

		if(rev_count == REV_MAX_NUM)
		{
			dog->judge_reason[lower_left] = RES_COME_BACK_VERTER;
			break;	//頂点に帰ってきた回数がREVERSE_MAX_COUNTと同じになった
		}


		if(pbs->PlaneInfo[dog->plane].main_effective_range_max - 1 <= physical_x || pbs->PlaneInfo[dog->plane].main_effective_range_min >= physical_x)	//搬送外に出た角を破れと判定しないように
		{
			dog->judge_reason[lower_left] = RES_OUT_OF_AREA;
			break;	//搬送エリア外ならばブレイク
		}

		if(ver_a_x - dog->scan_move_x_thr_line > x)	//頂点より左？
		{
			break;	//探索終了
		}

		//if(i > 5 && current_rev_stayt == 1)	//探索開始n回目で探索開始位置に紙幣があったらそれはもう裂けと判断する
		//{
		//	//破れと判定	引きちぎられたもの
		//	//return TEAR;
		//}
	}



	if(point_count < dog->scan_minimum_points)	//ポイント数が設定値以下なら
	{
		//角折れなしと判定
		dog->judge_reason[lower_left] = RES_POINT_COUNT_LESS;
		return NORMAL;
	}

	// 最小二乗法の計算	斜辺の式
    approx_line(count_list_cord_x, count_list_cord_y, point_count, &offset, &slope);

	if(slope >= 0)	//傾きチェック　傾きが正
	{
		//破れと判定	引きちぎられたもの
		dog->judge_reason[lower_left] = RES_SLOPE_OP;
		return TEAR;
	}

	//頂点計算　点C
	ver_c_x = (s16)((ver_a_y - offset) / slope);
	ver_c_y = ver_a_y;

	dog->triangle_vertex_1[lower_left][0] = ver_c_x;
	dog->triangle_vertex_1[lower_left][1] = ver_c_y;

	//頂点計算　点B
	ver_b_x = ver_a_x;
	ver_b_y = (s16)(slope * ver_b_x + offset);

	dog->triangle_vertex_2[lower_left][0] = ver_b_x;
	dog->triangle_vertex_2[lower_left][1] = ver_b_y;

	//辺の長さ
	side_x_size = (u16)(abs(ver_a_x - ver_c_x));
	side_y_size = (u16)(abs(ver_a_y - ver_b_y));


	//計測した長さをdotで格納
	dog->len_x[lower_left] = side_x_size;
	dog->len_y[lower_left] = side_y_size;

	//短辺長辺比較
	if(side_x_size >= side_y_size)
	{
		dog->long_side[lower_left] = side_x_size;
		dog->short_side[lower_left] = side_y_size;
	}
	else
	{
		dog->short_side[lower_left] = side_x_size;
		dog->long_side[lower_left] = side_y_size;
	}

	//綺麗な角折れか破れを検知します。
	//返り値 0：正常な角折れ 4：破れ
	status = triangle_inspection(pbs ,slope ,offset , ver_b_x ,ver_c_x ,1 ,dog);
	if(status != DOG_EAR)
	{
		dog->judge_reason[lower_left] = RES_SLOPE_IS_ABNORMAL;

		//ここで多角形の面積
		//dog->area[lower_left] = area_calculation(count_list_cord_x ,count_list_cord_y ,point_count);
		return TEAR;	//破損
	}

	//三角形だとみなされたので
	//この頂点の角折れもしくは角裂けの面積を求める
	dog->area[lower_left] = (u32)(dog->short_side[lower_left] * dog->long_side[lower_left] / 2);


	//角折れか破れかを調べます
	//返り値 0：正常な角折れ 3：破れ
	status = cal_symmetric_center_point(ver_a_x ,ver_a_y ,ver_b_x ,ver_b_y ,ver_c_x ,ver_c_y ,slope,offset, pbs ,lower_left ,dog);
	if(status != DOG_EAR)
	{
		dog->judge_reason[lower_left] = RES_HISTOGRAM_PEAKS_MATCH;
		return TEAR;
	}

	//角折れ検知
	status = judge_dog_ear(dog->short_side[lower_left], dog->long_side[lower_left] , &dog->area[lower_left] ,dog);
	if(status != DOG_EAR)	//角折れはあるが基準をクリア
	{
		return LITTLE_DOG_EAR;
	}

	//角折れ
	return DOG_EAR;
}

//右上の角折れを検知します
u8 right_upper(s16 ver_a_x, s16 ver_a_y, s16 threshold, ST_BS *pbs ,ST_DOG_EAR *dog)
{
	s16 physical_x = 0;
	s16 current_x = 0;
	s16 x = 0;
	s16 y = 0;
	u16 side_x_size = 0;
	u16 side_y_size = 0;
	u8 point_count = 0;

	s16 ver_b_x = 0;
	s16 ver_b_y = 0;

	s16 ver_c_x = 0;
	s16 ver_c_y = 0;

	u8 movement_y = 0;
	s16	status = 0;
	s16 count_list_cord_x[MAX_SEARCH_RANGE] = {0};	//55
	s16 count_list_cord_y[MAX_SEARCH_RANGE] = {0};	//375

	u8  i = 0;
	float slope = 0;
	float offset = 0;
	u8 rev_count = 0;

	s16 search_x = 0;
	u8  gaba_flg = 0;
	//u8 current_rev_stayt = 0;

	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param =  &work[dog->buf_num].note_param;

	//Lgetdtのパラメタ設定
	spt.l_plane_tbl = dog->plane;
	spt.way = pbs->insertion_direction;

	////x頂点座標を正確に検知します。
	//for(current_x = ver_a_x; current_x > 0; current_x--)
	//{
	//	spt.x = current_x;
	//	spt.y = 0;

	//	new_L_Getdt(&spt,pbs ,note_para);

	//	//媒体にぶつかるまで探索します。
	//	if(threshold < spt.sensdt)
	//	{
	//		x = current_x;
	//		break;
	//	}
	//}

	////ｙ頂点座標を正確に検知します。
	//for(current_y = ver_a_y; current_y > 0; current_y--)
	//{
	//	spt.x = 0;
	//	spt.y = current_y;

	//	new_L_Getdt(&spt,pbs ,note_para);

	//	//媒体にぶつかるまで探索します。
	//	if(threshold < spt.sensdt)
	//	{
	//		y = current_y;
	//		break;
	//	}

	//}

	////頂点を更新します。
	//if(x != 0 && y != 0)
	//{
	//	ver_a_x = x;
	//	ver_a_y = y;

	//}

	x = ver_a_x;
	y = ver_a_y;

	//ｙ方向の移動量（
	//movement_y = pbs->note_y_size / NOTE_DEV_NUM / SEARCH_MARGIN;
	movement_y = dog->scan_move_y;
	y -= (s16)((float)movement_y * 1.5f);	//

	//頂点から探索　x方向に探索します
	for(i = 1; i < MAX_SEARCH_RANGE; i++)
	{
		y -= movement_y;	//
		gaba_flg = 0;
		//current_rev_stayt = 0;	//探索開始ポイントに紙幣が存在するフラグ


		//x方向へ探索
		for(current_x = x; current_x > 0; current_x--)
		{
			spt.x = current_x;
			spt.y = y;

			new_L_Getdt(&spt ,pbs ,pnote_param);

			physical_x = (s16)spt.x;

			//媒体にぶつかるまで探索します。
			if(threshold > spt.sensdt)
			{
				gaba_flg = 0;

				if(i >  1 && current_x == x )	//すぐにぶつかった場合
				{
					x = ver_a_x;	//ポジションをリセット
					//current_rev_stayt = 1;		//探索開始ポイントに紙幣が存在するフラグ
					rev_count += 1;
					break;
				}

				//ぶつかったものが媒体なのかゴミなのかを判定する
				//GARBAGE_OR_NOTE_RANGEの分だけX方向に探索し閾値1度でも上回ったらぶつかったものはゴミだとみなし再び探索を再開する。
				for(search_x = current_x - 1 ; search_x > current_x - dog->garbage_or_note_range; search_x--)
				{
					spt.x = search_x;
					spt.y = y;

					new_L_Getdt(&spt ,pbs ,pnote_param);

					if(threshold < spt.sensdt)	//一度も上回らなければ
					{
						gaba_flg = 1;
					}
				}

				if(gaba_flg == 0 )
				{
					point_count++;		//ポイント数をカウントします

					//ぶつかった位置を記録します
					count_list_cord_x[point_count] = current_x;
					count_list_cord_y[point_count] = y;

					//次の探索開始位置をぶつかった位置-20とし、処理時間短縮を図ります
					x = count_list_cord_x[point_count] + dog->scan_move_x;	// -20
					break;
				}
			}
		}

		if(rev_count == REV_MAX_NUM)
		{
			dog->judge_reason[upper_right] = RES_COME_BACK_VERTER;
			break;	//頂点に帰ってきた回数がREVERSE_MAX_COUNTと同じになった
		}

		if(pbs->PlaneInfo[dog->plane].main_effective_range_max - 1 <= physical_x || pbs->PlaneInfo[dog->plane].main_effective_range_min >= physical_x)	//搬送外に出た角を破れと判定しないように
		{
			dog->judge_reason[upper_right] = RES_OUT_OF_AREA;
			break;	//搬送エリア外ならばブレイク
		}

		if(ver_a_x + dog->scan_move_x_thr_line < x)	//頂点より左？　-10
		{
			break;	//探索終了
		}

		//if(i > 5 && current_rev_stayt == 1)	//探索開始n回目で探索開始位置に紙幣があったらそれはもう裂けと判断する
		//{
		//	//破れと判定	引きちぎられたもの
		//	//return TEAR;
		//}
	}

	if(point_count < dog->scan_minimum_points)	//ポイント数5以下	5
	{
		//角折れなしと判定
		dog->judge_reason[upper_right] = RES_POINT_COUNT_LESS;
		return NORMAL;
	}

	// 最小二乗法の計算	斜辺の式
    approx_line(count_list_cord_x, count_list_cord_y, point_count, &offset, &slope);

	if(slope >= 0)	//傾きチェック　傾きが負ならば
	{
		//破れと判定	引きちぎられたもの
		dog->judge_reason[upper_right] = RES_SLOPE_OP;
		return TEAR;
	}

	//頂点計算　点C
	ver_c_x = (s16)((ver_a_y - offset) / slope);
	ver_c_y = ver_a_y;

	dog->triangle_vertex_1[upper_right][0] = ver_c_x;
	dog->triangle_vertex_1[upper_right][1] = ver_c_y;

	//頂点計算　点B
	ver_b_x = ver_a_x;
	ver_b_y = (s16)(slope * ver_b_x + offset);

	dog->triangle_vertex_2[upper_right][0] = ver_b_x;
	dog->triangle_vertex_2[upper_right][1] = ver_b_y;

	//辺の長さ
	side_x_size = (u16)(abs(ver_a_x - ver_c_x));
	side_y_size = (u16)(abs(ver_a_y - ver_b_y));

	//計測した長さをdotで格納
	dog->len_x[upper_right] = side_x_size;
	dog->len_y[upper_right] = side_y_size;

	//短辺長辺比較
	if(side_x_size >= side_y_size)
	{
		dog->long_side[upper_right] = side_x_size;
		dog->short_side[upper_right] = side_y_size;
	}
	else
	{
		dog->short_side[upper_right] = side_x_size;
		dog->long_side[upper_right] = side_y_size;
	}

	//綺麗な角折れか破れを検知します。
	//返り値 0：正常な角折れ -3：破れ
	status = triangle_inspection(pbs  ,slope ,offset , ver_c_x ,ver_b_x ,-1 ,dog);
	if(status != DOG_EAR)
	{
		dog->judge_reason[upper_right] = RES_SLOPE_IS_ABNORMAL;
		//ここで多角形の面積
		//dog->area[upper_right] = area_calculation(count_list_cord_x ,count_list_cord_y ,point_count);
		return TEAR;	//破損
	}

	//三角形だとみなされたので
	//この頂点の角折れもしくは角裂けの面積を求める
	dog->area[upper_right] = (u32)(dog->short_side[upper_right] * dog->long_side[upper_right] / 2);

	status = cal_symmetric_center_point(ver_a_x ,ver_a_y ,ver_b_x ,ver_b_y ,ver_c_x ,ver_c_y ,slope,offset,pbs, upper_right ,dog);
	if(status != DOG_EAR)
	{
		dog->judge_reason[upper_right] = RES_HISTOGRAM_PEAKS_MATCH;

		return TEAR;
	}



	//角折れ検知
	status = judge_dog_ear(dog->short_side[upper_right], dog->long_side[upper_right] , &dog->area[upper_right] ,dog);
	if(status != DOG_EAR)	//角折れはあるが基準をクリア
	{
		return LITTLE_DOG_EAR;
	}

	//角折れ
	return DOG_EAR;
}

//右下の角折れを検知します
u8 right_lower(s16 ver_a_x, s16 ver_a_y, s16 threshold, ST_BS *pbs ,ST_DOG_EAR *dog)
{

	s16 physical_x = 0;
	u8 movement_y = 0;
	s16 current_x = 0;

	s16 x = 0;
	s16 y = 0;

	u16 side_x_size = 0;
	u16 side_y_size = 0;


	s16 ver_b_x = 0;
	s16 ver_b_y = 0;

	s16 ver_c_x = 0;
	s16 ver_c_y = 0;

	u8 point_count = 0;
	u8 rev_count = 0;

	s16 count_list_cord_x[MAX_SEARCH_RANGE] = {0};	//55
	s16 count_list_cord_y[MAX_SEARCH_RANGE] = {0};	//375
	float slope = 0;
	float offset = 0;

	s16 search_x = 0;
	u8  gaba_flg = 0;
	//u8 current_rev_stayt = 0;

	s16	status = 0;
	u8  i = 0;

	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param =  &work[dog->buf_num].note_param;

	//Lgetdtのパラメタ設定
	spt.l_plane_tbl = dog->plane;
	spt.way = pbs->insertion_direction;

	x = ver_a_x;
	y = ver_a_y;

	//ｙ方向の移動量（
	//movement_y = pbs->note_y_size / NOTE_DEV_NUM / SEARCH_MARGIN;
	movement_y = dog->scan_move_y;
	y += (s16)((float)movement_y * 1.5f);	//

	//頂点から探索　x方向に探索します
	for(i = 1; i < MAX_SEARCH_RANGE; i++)
	{
		y += movement_y;	//
		gaba_flg = 0;
		//current_rev_stayt = 0;	//探索開始ポイントに紙幣が存在するフラグ

		//x方向へ探索
		for(current_x = x; current_x > 0; current_x--)
		{
			spt.x = current_x;
			spt.y = y;

			new_L_Getdt(&spt ,pbs ,pnote_param);

			physical_x = (s16)spt.x;

			//媒体にぶつかるまで探索します。
			if(threshold > spt.sensdt)
			{
				gaba_flg = 0;

				if(i >  1 && current_x == x )	//すぐにぶつかった場合
				{
					x = ver_a_x;	//ポジションをリセット
					//current_rev_stayt = 1;		//探索開始ポイントに紙幣が存在するフラグ
					rev_count += 1;
					break;
				}

				//ぶつかったものが媒体なのかゴミなのかを判定する
				//GARBAGE_OR_NOTE_RANGEの分だけX方向に探索し閾値1度でも上回ったらぶつかったものはゴミだとみなし再び探索を再開する。
				for(search_x = current_x - 1 ; search_x > current_x - dog->garbage_or_note_range; search_x--)
				{
					spt.x = search_x;
					spt.y = y;

					new_L_Getdt(&spt ,pbs ,pnote_param);

					if(threshold < spt.sensdt)	//一度も上回らなければ
					{
						gaba_flg = 1;
					}
				}

				if(gaba_flg == 0 )	//紙幣と判断
				{
					point_count++;		//ポイント数をカウントします

					//ぶつかった位置を記録します
					count_list_cord_x[point_count] = current_x;
					count_list_cord_y[point_count] = y;

					//次の探索開始位置をぶつかった位置-20とし、処理時間短縮を図ります
					x = count_list_cord_x[point_count] + dog->scan_move_x;	// -20
					break;
				}
			}
		}

		if(rev_count == REV_MAX_NUM)
		{
			dog->judge_reason[lower_right] = RES_COME_BACK_VERTER;
			break;	//頂点に帰ってきた回数がREVERSE_MAX_COUNTと同じになった
		}

		if(pbs->PlaneInfo[dog->plane].main_effective_range_max - 1 <= physical_x || pbs->PlaneInfo[dog->plane].main_effective_range_min >= physical_x)	//搬送外に出た角を破れと判定しないように
		{
			dog->judge_reason[lower_right] = RES_OUT_OF_AREA;
			break;	//搬送エリア外ならばブレイク
		}

		if(ver_a_x + dog->scan_move_x_thr_line < x)	//頂点より左？
		{
			break;	//探索終了
		}

		//if(i > 5 && current_rev_stayt == 1)	//探索開始n回目で探索開始位置に紙幣があったらそれはもう裂けと判断する
		//{
		//	//破れと判定	引きちぎられたもの
		//	//return TEAR;
		//}


	}



	if(point_count < dog->scan_minimum_points)	//ポイント数5以下	5
	{
		//角折れなしと判定
		dog->judge_reason[lower_right] = RES_POINT_COUNT_LESS;
		return NORMAL;
	}

	// 最小二乗法の計算	斜辺の式
	approx_line(count_list_cord_x, count_list_cord_y, point_count, &offset, &slope);

	if(slope <= 0)	//傾きチェック　傾きが負ならば
	{
		//破れと判定	引きちぎられたもの
		dog->judge_reason[lower_right] = RES_SLOPE_OP;
		return TEAR;
	}

	//頂点計算　点C
	ver_c_x = (s16)((ver_a_y - offset) / slope);
	ver_c_y = ver_a_y;

	dog->triangle_vertex_1[lower_right][0] = ver_c_x;
	dog->triangle_vertex_1[lower_right][1] = ver_c_y;

	//頂点計算　点B
	ver_b_x = ver_a_x;
	ver_b_y = (s16)(slope * ver_b_x + offset);

	dog->triangle_vertex_2[lower_right][0] = ver_b_x;
	dog->triangle_vertex_2[lower_right][1] = ver_b_y;

	//辺の長さ
	side_x_size = (u16)(abs(ver_a_x - ver_c_x));
	side_y_size = (u16)(abs(ver_a_y - ver_b_y));

	//計測した長さをdotで格納
	dog->len_x[lower_right] = side_x_size;
	dog->len_y[lower_right] = side_y_size;

	//短辺長辺比較
	if(side_x_size >= side_y_size)
	{
		dog->long_side[lower_right] = side_x_size;
		dog->short_side[lower_right] = side_y_size;
	}
	else
	{
		dog->short_side[lower_right] = side_x_size;
		dog->long_side[lower_right] = side_y_size;
	}

	//斜辺と近似直線を比較する
	status = triangle_inspection(pbs ,slope ,offset , ver_c_x ,ver_b_x ,-1 ,dog);

	if(status != DOG_EAR)
	{
		dog->judge_reason[lower_right] = RES_SLOPE_IS_ABNORMAL;

		//ここで多角形の面積
		//dog->area[lower_right] = area_calculation(count_list_cord_x ,count_list_cord_y ,point_count);
		return TEAR;	//破損
	}

	//三角形だとみなされたので
	//この頂点の角折れもしくは角裂けの面積を求める
	dog->area[lower_right] = (u32)(dog->short_side[lower_right] * dog->long_side[lower_right] * 0.5f);

	//綺麗な角折れか破れを検知します。
	//返り値 0：正常な角折れ -3：破れ
	status = cal_symmetric_center_point(ver_a_x ,ver_a_y ,ver_b_x ,ver_b_y ,ver_c_x ,ver_c_y ,slope,offset,pbs ,lower_right ,dog);
	if(status != DOG_EAR)
	{
		dog->judge_reason[lower_right] = RES_HISTOGRAM_PEAKS_MATCH;
		return TEAR;
	}

	//角折れ検知
	status = judge_dog_ear(dog->short_side[lower_right], dog->long_side[lower_right] , &dog->area[lower_right] ,dog);
	if(status != DOG_EAR)	//角折れはあるが基準をクリア
	{
		return LITTLE_DOG_EAR;
	}

	//角折れ
	return DOG_EAR;
}

//折れか破れかを判断します。
s16 cal_symmetric_center_point(s16 ver_a_x, s16 ver_a_y, s16 ver_b_x, s16 ver_b_y, s16 ver_c_x, s16 ver_c_y, float slope ,float offset, ST_BS *pbs ,u8 location ,ST_DOG_EAR *dog)
{
	//ST_SPOINT spt;
//	ST_NOTE_PARAMETER* pnote_param =  &work[dog->buf_num].note_param;

	//ポイント数
	//u16 in_count = 0;
	//u16 out_count = 0;

	u8 plane = 0;

	float b1,c1,a2,b2,c2; //方程式の値を格納する変数

	//角折れ三角形の中心
	float trai_cen_x;
	float trai_cen_y;

	//対称三角形の頂点
	float sym_trai_cen_x;
	float sym_trai_cen_y;

	//対称三角形の辺の係数１
	float sym_slope1;
	float sym_y_ofs1;

	//対称三角形の辺の係数２
	float sym_slope2;
	float sym_y_ofs2;

	s16 res=0;

	float scan_point_x;
	float scan_point_y;

	//スキャン範囲
	s16 scan_x_range;
	s16 scan_y_range;

	//スキャン開始位置
	s16 scan_x_start;
	s16 scan_y_start;

	//スキャン位置
//	s16 scan_in__a;
//	s16 scan_in__b;
	s16 scan_out_a;
	s16 scan_out_b;

	//s16 cm1 =0;
	//s16 cm2 =0;

	s8 increment_x = 1;
	s8 increment_y = 1;

	//u8 not_seen_area_flg = 0;	//見ないエリアのフラグ

	//Lgetdtのパラメタ設定
	//spt.l_plane_tbl = dog->plane;

	//角折れ三角形の中心点
	trai_cen_x = ver_a_x;
	trai_cen_y = ver_a_y;

	//角折れ三角形斜辺の中心点を利用して対称三角形の頂点を求めます。
	//式②
	a2 = slope / 2;
	b2 = -0.5;
	c2 = -((slope * trai_cen_x / 2) + -(trai_cen_y / 2) + offset);

	//式①
	b1 = slope;
	c1 = slope * trai_cen_y + trai_cen_x;

	//対称の点
	sym_trai_cen_x = (c1 * b2 - b1 * c2) / (b2 - a2 * b1);
	sym_trai_cen_y = (c2 - a2 * c1) / (b2 - a2 * b1);


	scan_point_y = (float)abs(ver_b_y - ver_a_y);
	scan_point_x = (float)abs(ver_c_x - ver_a_x);

	//scan_in__a = (s16)-scan_point_x;
	scan_out_a = (s16)scan_point_x;

	//scan_in__b = (s16)scan_point_y;
	scan_out_b = (s16)-scan_point_y;

	//頂点に応じてインクリメントデクリメントを変更する
	if(location == 1)	//左下
	{
		//scan_in__b = (s16)-scan_point_y;
		scan_out_b = (s16)scan_point_y;
		increment_y = -1;
	}
	else if(location == 2)	//右上
	{
		//scan_in__a = (s16)scan_point_x;
		scan_out_a = (s16)-scan_point_x;
		increment_x = -1;
	}
	else if(location == 3)	//右下
	{
		//scan_in__a = (s16)scan_point_x;
		scan_out_a = (s16)-scan_point_x;

		//scan_in__b = (s16)-scan_point_y;
		scan_out_b = (s16)scan_point_y;

		increment_x = -1;
		increment_y = -1;
	}

	//三角形エリア外のスキャン範囲の計算
	scan_y_range = (s16)ABS(ver_c_y - sym_trai_cen_y);
	scan_x_range = (s16)ABS(ver_b_x - sym_trai_cen_x);

	scan_x_start = (s16)(scan_x_range * 0.333333333f);
	scan_y_start = (s16)(scan_y_range * 0.333333333f);

	//直線式2
	if(sym_trai_cen_x == ver_b_x)
	{
		sym_slope2 = (sym_trai_cen_y - ver_b_y) / 0.00001f;
	}
	else
	{
		sym_slope2 = (sym_trai_cen_y - ver_b_y) / (sym_trai_cen_x - ver_b_x);
	}

	//直線式1
	if(sym_trai_cen_x == ver_c_x){
		sym_slope1 = (sym_trai_cen_y - ver_c_y) / 0.00001f;
	}
	else
	{
		sym_slope1 = (sym_trai_cen_y - ver_c_y) / (sym_trai_cen_x - ver_c_x);
	}

	//y切片を求める
	sym_y_ofs2 = ver_b_y - sym_slope2 * ver_b_x;
	sym_y_ofs1 = ver_c_y - sym_slope1 * ver_c_x;


	//逆数を計算　処理時間を考慮
	sym_slope1 = 1 / sym_slope1;


	//角折れ三角形の中心点
	trai_cen_x = (ver_a_x + ver_b_x + ver_c_x) * 0.333333333f;
	trai_cen_y = (ver_a_y + ver_b_y + ver_c_y) * 0.333333333f;

	//角折れ三角形斜辺の中心点を利用して対称三角形の頂点を求めます。
	//式②
	a2 = slope * 0.5f;
	b2 = -0.5f;
	c2 = -((slope * trai_cen_x * 0.5f) + -(trai_cen_y * 0.5f) + offset);

	//式①
	b1 = slope;
	c1 = slope * trai_cen_y + trai_cen_x;

	//対称の点
	sym_trai_cen_x = (c1 * b2 - b1 * c2) / (b2 - a2 * b1);
	sym_trai_cen_y = (c2 - a2 * c1) / (b2 - a2 * b1);


	if(pbs->PlaneInfo[UP_T_IR1].Enable_or_Disable == PLANE_ENABLED)
	{
		plane = UP_T_IR1;
	}

	else if(pbs->PlaneInfo[DOWN_T_IR1].Enable_or_Disable == PLANE_ENABLED)
	{
		plane = DOWN_T_IR1;
	}

	//ヒストグラムによる比較
	res = judge_dogear_tear(pbs, plane , sym_trai_cen_x, sym_trai_cen_y, ver_b_x, ver_c_y ,increment_y ,increment_x ,sym_slope1 ,sym_y_ofs1
		,sym_slope2 ,sym_y_ofs2 ,scan_out_a ,scan_out_b , scan_x_range, scan_x_start, scan_y_start, scan_y_range ,location ,dog);
	if(res == DOG_EAR)
	{
		//return TEAR;
		return DOG_EAR;	//角折れ
	}

	//res = judge_dogear_tear(pbs, OMOTE_T_R  , sym_trai_cen_x, sym_trai_cen_y, ver_a_x, ver_a_y, ver_b_x, ver_b_y, ver_c_x, ver_c_y ,increment_y ,increment_x ,sym_slope1 ,sym_y_ofs1
	//	,sym_slope2 ,sym_y_ofs2 ,scan_in__a ,scan_out_a ,scan_in__b ,scan_out_b , scan_x_range, scan_x_start, scan_y_start, scan_y_range ,location);
	//if(res == DOG_EAR)
	//{
	//	//return TEAR;
	//	return DOG_EAR;	//角折れ
	//}

	//return DOG_EAR;	//角折れ
	return TEAR;	//角裂け
	
}

//三角形か破れかどうか調べます
//斜辺の直線に対し垂直に交わる線を引きそこのレベルで評価します。
s16 triangle_inspection(ST_BS *pbs , float tan, float offset ,s16 input_1_x ,s16 input_2_x ,s8 increment_dir_x ,ST_DOG_EAR *dog )
{
	//Lgetdt用構造体
	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param =  &work[dog->buf_num].note_param;

	//斜辺に直交する線の式

	float offset_r;
	float tan_r;
	float len_x;	//斜辺の長さ

	float pitch;	//サーチのピッチ
	s16 i = 0;
	//s16 j = 0;

	float y;
	float scan_y;

	float start_x;
	float current_x;

	u8 point_count = 0;
	u8 ok_count = 0;

	//ST_SPOINTのパラメタ
	spt.l_plane_tbl = dog->plane;

	tan_r = -(1 / tan);

	

	//探索開始位置の決定
	len_x = (float)abs(input_1_x - input_2_x);
	pitch = dog->tear_scan_start_x;
	len_x = len_x / pitch;

	//dog->tear_scan_renge = 4;

	if(ABS(tan_r) > 5)
	{
		dog->tear_scan_renge = 1;
	}
	//len = 1;

	for(i = 2; i < len_x - 2; i++)
	{
	//for(i = 0; i < 100; i++ )
	//{
		//pitch = 1;
		//探索開始位置
		start_x = (float)(input_1_x + pitch * i);
		//start_x = input_1_x + i;
		y = tan * start_x + offset;

		//直交する線の式
		offset_r = y - (tan_r * start_x);	//切片

		point_count += 2;

		//for(j = 0; j < dog->tear_scan_renge; j++)	// 内側
		//{
			current_x = start_x + (dog->tear_scan_renge * increment_dir_x);
			//y += ((dog->tear_scan_renge )* increment_dir_y);
			scan_y = tan_r * current_x + offset_r;
			spt.x = (s16)current_x;
			spt.y = (s16)scan_y;

			new_L_Getdt(&spt ,pbs ,pnote_param);

			//閾値と平均値を比較します。
			if(/*dog->threshold_trans*/250 > spt.sensdt)
			{
				ok_count++;	//正常
				//break;
			}
//		}

			//y = tan * start_x + offset;

		//for(j = 0; j < dog->tear_scan_renge; j++)	// 外側
		//{
			current_x = start_x + (dog->tear_scan_renge * (-increment_dir_x));
			//y = tan_r * current_x + offset_r;
			//y += ((dog->tear_scan_renge + 2) * (-increment_dir_y));	//y座標のみ2で補正する。
			scan_y = tan_r * current_x + offset_r;
			spt.x = (s16)current_x;
			spt.y = (s16)scan_y;

			new_L_Getdt(&spt ,pbs ,pnote_param);

			//閾値と平均値を比較します。
			if(/*dog->threshold_trans*/250 < spt.sensdt)
			{
				ok_count++;	//正常
				//break;
			}
		//}


	}

	if(point_count == ok_count)
	{
		return DOG_EAR;	//正常な角折れ
	}

	return TEAR;	//破れ
}

//単位変換を行います。
void unit_convert(float ele_pitch_x, float ele_pitch_y ,ST_DOG_EAR *dog)
{
	u8 i;

	for(i = 0; i < 4; i++)
	{
		dog->len_x_mm[i] = dog->len_x[i] * ele_pitch_x;
		dog->len_y_mm[i] = dog->len_y[i] * ele_pitch_y;

		dog->short_side_mm[i] = dog->short_side[i] * ele_pitch_x;
		dog->long_side_mm[i] = dog->long_side[i] * ele_pitch_y;

		dog->area_mm[i] = (u32)(dog->area[i] * ele_pitch_x * ele_pitch_y);
	}
	return ;
}

//ステータスの初期化を行います
void ini_dog( ST_DOG_EAR *dog)
{
	u8 i;

	for(i = 0; i < 4; i++)
	{
		dog->len_x_mm[i] = 0;
		dog->len_y_mm[i] = 0;
		dog->area_mm[i] = 0;
		dog->judge_reason[i] = 0;
		dog->len_x[i] = 0;
		dog->len_y[i] = 0;
		dog->area[i] = 0;

		dog->short_side[i] = 0;
		dog->long_side[i] = 0;

		dog->judge[i] = 0;
		dog->judge_pix[i] = 0;
	}
	return ;
}

//規定に従って角折れを判断します。
s16 judge_dog_ear(float shot_side, float long_side , u32 *area ,ST_DOG_EAR *dog)
{
	//角折れがあった場合にその面積を求めます
	//また、設定された閾値と比較します。
	if(shot_side != 0 && long_side != 0)
	{

		////この頂点の角折れの面積
		//*area = shot_side * long_side / 2;

		//フィットネス基準と比較します
		//規定の面積を超えている　かつ　短手辺が規定の長さ以上の場合
		if(dog->comp_flg == 0)
		{

			if(dog->threshold_area < *area && dog->threshold_short_side_dot < shot_side)
			{
				return DOG_EAR;	//角折れと判定
			}
		}

		else{
			//規定長さよりの短手辺・長手辺ともに長い場合
			if(dog->threshold_short_side_dot < shot_side && dog->threshold_long_side_dot < long_side)
			{
				return DOG_EAR;	//角折れと判定
			}
		}
	}

	return 0;	//当てはまらなければ0を返す
}

// 最小二乗法の計算
void approx_line(s16 x[],s16 y[], s32 n, float *a0, float *a1)
{
        int i;

        float A00 = 0 ,A01 = 0, A02 = 0, A11 = 0, A12 = 0;

		float up = 0,down =0;

        for (i = 1; i < n; i++)
		{
                A00 += 1.0;
                A01 += x[i];
                A02 += y[i];
                A11 += x[i] * x[i];
                A12 += x[i] * y[i];
        }

		up = (A02 * A11 -A01 * A12);
		down = (A00 * A11 - A01 * A01);

		if(up == 0 && down == 0)
		{
			a0 = 0;
			a1 = 0;
			return ;
		}
		else
		{
			*a0 = up / down;
		}


		up = (A00 * A12 -A01 * A02);

		if(up == 0 && down == 0)
		{
			a0 = 0;
			a1 = 0;
			return ;
		}
		else
		{
			*a1 = up / down;
		}
}

s16 judge_dogear_tear(ST_BS *pbs ,u8 plane ,double sym_trai_cen_x ,double sym_trai_cen_y ,s16 ver_b_x ,s16 ver_c_y
		,s8 increment_y ,s8 increment_x ,double sym_slope1 ,double sym_y_ofs1 ,double sym_slope2 ,double sym_y_ofs2
		,s16 scan_out_a , s16 scan_out_b ,s16 scan_x_range , s16 scan_x_start ,s16 scan_y_start, s16 scan_y_range ,u8 location ,ST_DOG_EAR *dog)
{
	//#define QP 10.0f				//5～30
	//#define MARGIN 1				//ピーク比較0～5
	//#define PEAK_MARGIN 0.33333f	//1/2～1/5
	#define MIN_POINT 5
	#define FREQUENCY 2

	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param =  &work[dog->buf_num].note_param;

	u8 hist_pix_val_ary_in[255]= {0};
	u8 hist_pix_val_ary_out[255]= {0};

	u8 hist_pix_val_ary_in_peak[255]= {0};
	u8 hist_pix_val_ary_out_peak[255]= {0};

	//u8 hist_pix_val_ary_in_2nd_peak[255]= {0};
	//u8 hist_pix_val_ary_out_2nd_peak[255]= {0};


	//float pix_val = 0;
	double peak_2nd_thr = 0;

	u8 i = 0;
	u8 j = 0;
	s32 ii=0;
	//u32 jj=0;
	u32 I=0;
	u32 J=0;

	u8 ary_num;

	//ポイント数
	u16 in_count = 0;
	u16 out_count = 0;

	//スキャン座標
	double Y;
	double X;
	s16 cy;
	s16 cx;

	s32 xx = 0;
	s32 yy = 0;

	s16 move_x = 0;
	s16 move_y = 0;

	s16 cm1 =0;
	s16 cm2 =0;

	s16 in__count_2nd =0;
	s16 out_count_2nd =0;


	double out_x_scan_main=0;
	double out_y_scan_main=0;

	u8 not_seen_area_flg = 0;	//見ないエリアのフラグ

	s16 QP = dog->qp;
	u32 MARGIN = dog->peak_width;
	double PEAK_MARGIN = 1.0 / dog->peak_margin;

	u32 bin_num = 255 / QP;

	double qp = 1.0 / QP;

	//検知範囲の大きさ
	u8 point_around_size = 3;

	#ifdef CSV_DB_DOG
	FILE *fp;			//filedebug
	F_CREATE_1(fp);		//filedebug
#endif

	//Lgetdtのパラメタ設定
	spt.l_plane_tbl = plane;



	//************************************************************
	//三角形内のスキャンポイントの位置を設定
	move_x = (s16)(dog->len_x[location] * 0.111111f * 0.5);
	move_y = (s16)(dog->len_y[location] * 0.111111f * 0.5);

	//場所に応じてインクリメントを変更
	if(location == lower_left || location == upper_right)
		move_x = move_x * -1;

	sym_trai_cen_x -= move_x * 2;
	sym_trai_cen_y -= move_y * 2;

	//辺の長さに応じてポイントのサイズを変更 9mmより大きければ
	//探索範囲を拡大
	if(dog->len_x[location] > 102 && dog->len_y[location] > 102)
		point_around_size = 5;


	//平行移動した直線に沿って角折れ対称三角形のエリアを採取します
	for(i = 0; i < 5; i++)
	{
		not_seen_area_flg = 0;

		for(j = 0; j < dog->not_seen_area_count; j++)
		{
			if((dog->not_seen_areas[j][0] < sym_trai_cen_x && dog->not_seen_areas[j][2] > sym_trai_cen_x) &&
				(dog->not_seen_areas[j][1] > sym_trai_cen_y && dog->not_seen_areas[j][3] < sym_trai_cen_y))
			{
				not_seen_area_flg = 1;
			}
		}

		if(not_seen_area_flg == 1)
		{
			continue;
		}

		yy = (s32)(sym_trai_cen_y - point_around_size);
		xx = (s32)(sym_trai_cen_x - point_around_size);

		//中心点から採取します
		//採取範囲は周囲8近傍（暫定
		for(yy = yy; yy < sym_trai_cen_y + point_around_size; yy++)
		{
			for(xx = xx; xx < sym_trai_cen_x + point_around_size; xx++)
			{
				spt.x = xx;
				spt.y = yy;

				new_L_Getdt(&spt ,pbs ,pnote_param);

#ifdef CSV_DB_DOG
			F_WRITE(fp,spt.sensdt);	//filedebug
#endif

				ary_num = (u8)(spt.sensdt * qp);
				hist_pix_val_ary_in[ary_num]++;
				in_count++;
			}
		}

		sym_trai_cen_x += move_x;
		sym_trai_cen_y += move_y;
	}
	//************************************************************


#ifdef CSV_DB_DOG
			F_WRITE(fp,7777);	//filedebug
#endif



	//in_x_scan_main = scan_in__a * 0.8;
	//in_y_scan_main = scan_in__b * 0.8;

	out_x_scan_main = scan_out_a * 0.1f;
	out_y_scan_main = scan_out_b * 0.1f;

	////内側の探索ポイントを増やすか増やさないか
	//if(2 <= abs(in_x_scan_main - scan_in__a))
	//{
	//	in_x_scan_sub = scan_in__a;
	//}
	//if(4 <= abs(in_y_scan_main - scan_in__b))
	//{
	//	in_y_scan_sub = scan_in__b;
	//}

	////外側の探索ポイントを増やすか増やさないか
	//if(2 <= abs(out_x_scan_main - scan_out_a * 0.8 ))
	//{
	//	out_x_scan_sub = scan_out_a * 0.8;
	//}
	//if(4 <= abs(out_y_scan_main - scan_out_b * 0.8))
	//{
	//	out_y_scan_sub = scan_out_b * 0.8;
	//}

	//スキャン開始
	for(ii = scan_y_start; ii < scan_y_range - scan_y_start; ii += 4)
	{
		//y座標をインクリメント
		cy = (s16)(ver_c_y - ii * increment_y);

		//ｘ座標の特定 逆数はここで使う
		X = (cy - sym_y_ofs1) * sym_slope1;

		not_seen_area_flg = 0;

		//if(in_count == 0)
		//{

		//	//画素採取	内側
		//	spt.x = (s16)X + in_x_scan_main;
		//	spt.y = cy;

		//	for(j = 0; j < dog->not_seen_area_count; j++)
		//	{
		//		if((dog->not_seen_areas[j][0][0] < spt.x && dog->not_seen_areas[j][1][0] > spt.x) &&
		//			(dog->not_seen_areas[j][0][1] > spt.y && dog->not_seen_areas[j][1][1] < spt.y))
		//		{
		//			not_seen_area_flg = 1;
		//		}
		//	}

		//	if(not_seen_area_flg == 0)
		//	{
		//		new_L_Getdt(&spt ,pbs ,pnote_param);

		//		ary_num = spt.sensdt * qp;
		//		hist_pix_val_ary_in[ary_num]++;
		//		in_count++;
		//	}

		//	//画素採取	内側

		//	if(in_x_scan_sub != 0)
		//	{
		//		spt.x = (s16)X + in_x_scan_sub;
		//		spt.y = cy;
		//		not_seen_area_flg = 0;

		//		for(j = 0; j < dog->not_seen_area_count; j++)
		//		{
		//			if((dog->not_seen_areas[j][0][0] < spt.x && dog->not_seen_areas[j][1][0] > spt.x) &&
		//				(dog->not_seen_areas[j][0][1] > spt.y && dog->not_seen_areas[j][1][1] < spt.y))
		//			{
		//				not_seen_area_flg = 1;
		//			}
		//		}

		//		if(not_seen_area_flg == 0)
		//		{
		//			new_L_Getdt(&spt ,pbs ,pnote_param);
		//			ary_num = spt.sensdt * qp;
		//			hist_pix_val_ary_in[ary_num]++;
		//			in_count++;
		//		}

		//	}
		//}

		//外側 @1
		spt.x = (s32)(X + out_x_scan_main);
		spt.y = (s32)cy;
		not_seen_area_flg = 0;

		for(j = 0; j < dog->not_seen_area_count; j++)
		{
			if((dog->not_seen_areas[j][0] < spt.x && dog->not_seen_areas[j][2] > spt.x) &&
				(dog->not_seen_areas[j][1] > spt.y && dog->not_seen_areas[j][3] < spt.y))
				{
					not_seen_area_flg = 1;
				}
			}
		if(not_seen_area_flg == 0)
		{

			new_L_Getdt(&spt ,pbs ,pnote_param);

#ifdef CSV_DB_DOG
			F_WRITE(fp,spt.sensdt);	//filedebug
#endif
			ary_num = (u8)(spt.sensdt * qp);
			hist_pix_val_ary_out[ary_num]++;

			out_count++;

		}

		//if(out_x_scan_sub != 0)
		//{
			//外側@2
			spt.x = (s32)(X + out_x_scan_main + out_x_scan_main) ;
			spt.y = (s32)cy;
			not_seen_area_flg = 0;

			for(j = 0; j < dog->not_seen_area_count; j++)
			{
			if((dog->not_seen_areas[j][0] < spt.x && dog->not_seen_areas[j][2] > spt.x) &&
				(dog->not_seen_areas[j][1] > spt.y && dog->not_seen_areas[j][3] < spt.y))
				{
					not_seen_area_flg = 1;
				}
			}
			if(not_seen_area_flg == 0)
			{

				new_L_Getdt(&spt ,pbs ,pnote_param);

#ifdef CSV_DB_DOG
			F_WRITE(fp,spt.sensdt);	//filedebug
#endif

				ary_num = (u8)(spt.sensdt * qp);
				hist_pix_val_ary_out[ary_num]++;

				out_count++;


			}
	//	}

		//if(out_x_scan_sub != 0)
		//{
			//外側@3
			spt.x = (s32)(X + out_x_scan_main + out_x_scan_main+ out_x_scan_main);
			spt.y = (s32)cy;
			not_seen_area_flg = 0;

			for(j = 0; j < dog->not_seen_area_count; j++)
			{
			if((dog->not_seen_areas[j][0] < spt.x && dog->not_seen_areas[j][2] > spt.x) &&
				(dog->not_seen_areas[j][1] > spt.y && dog->not_seen_areas[j][3] < spt.y))
				{
					not_seen_area_flg = 1;
				}
			}
			if(not_seen_area_flg == 0)
			{

				new_L_Getdt(&spt ,pbs ,pnote_param);

#ifdef CSV_DB_DOG
			F_WRITE(fp,spt.sensdt);	//filedebug
#endif

				ary_num = (u8)(spt.sensdt * qp);
				hist_pix_val_ary_out[ary_num]++;

				out_count++;


			}
		//}
	}

	for(ii = scan_x_start; ii < scan_x_range - scan_x_start; ii += 2)
	{
		cx = (s16)(ver_b_x + ii*increment_x);

		//y座標の特定
		Y = sym_slope2 * cx + sym_y_ofs2;
		//if(in_count == 0)
		//{
		//	//内側
		//	spt.x = cx;
		//	spt.y = (s16)Y + in_y_scan_main;
		//	not_seen_area_flg = 0;

		//	for(j = 0; j < dog->not_seen_area_count; j++)
		//	{
		//		if((dog->not_seen_areas[j][0][0] < spt.x && dog->not_seen_areas[j][1][0] > spt.x) &&
		//			(dog->not_seen_areas[j][0][1] > spt.y && dog->not_seen_areas[j][1][1] < spt.y))
		//		{
		//			not_seen_area_flg = 1;
		//		}
		//	}

		//	if(not_seen_area_flg == 0)
		//	{
		//		new_L_Getdt(&spt ,pbs ,pnote_param);
		//		ary_num = spt.sensdt * qp;
		//		hist_pix_val_ary_in[ary_num]++;
		//		in_count++;
		//	}

		//	//内側
		//	if(in_y_scan_sub != 0)
		//	{
		//		spt.x = cx;
		//		spt.y = (s16)Y + in_y_scan_sub;
		//		not_seen_area_flg = 0;

		//		for(j = 0; j < dog->not_seen_area_count; j++)
		//		{
		//			if((dog->not_seen_areas[j][0][0] < spt.x && dog->not_seen_areas[j][1][0] > spt.x) &&
		//				(dog->not_seen_areas[j][0][1] > spt.y && dog->not_seen_areas[j][1][1] < spt.y))
		//			{
		//				not_seen_area_flg = 1;
		//			}
		//		}

		//		if(not_seen_area_flg == 0)
		//		{
		//			new_L_Getdt(&spt ,pbs ,pnote_param);
		//			ary_num = spt.sensdt * qp;
		//			hist_pix_val_ary_in[ary_num]++;
		//			in_count++;
		//		}
		//	}
		//}

		//外側 @1
		spt.x = (s32)cx;
		spt.y = (s32)(Y + out_y_scan_main);
		not_seen_area_flg = 0;

		for(j = 0; j < dog->not_seen_area_count; j++)
		{
			if((dog->not_seen_areas[j][0] < spt.x && dog->not_seen_areas[j][2] > spt.x) &&
				(dog->not_seen_areas[j][1] > spt.y && dog->not_seen_areas[j][3] < spt.y))
				{
					not_seen_area_flg = 1;
				}
			}

		if(not_seen_area_flg == 0)
		{
			new_L_Getdt(&spt ,pbs ,pnote_param);


				ary_num = (u8)(spt.sensdt * qp);
			hist_pix_val_ary_out[ary_num]++;

#ifdef CSV_DB_DOG
			F_WRITE(fp,spt.sensdt);	//filedebug
#endif
			out_count++;

		}



		//外側 @2
		//if(out_y_scan_sub != 0)
		//{
			spt.x = (s32)cx;
			spt.y = (s32)(Y  + out_y_scan_main + out_y_scan_main);
			not_seen_area_flg = 0;

			for(j = 0; j < dog->not_seen_area_count; j++)
			{
			if((dog->not_seen_areas[j][0] < spt.x && dog->not_seen_areas[j][2] > spt.x) &&
				(dog->not_seen_areas[j][1] > spt.y && dog->not_seen_areas[j][3] < spt.y))
				{
					not_seen_area_flg = 1;
				}
			}

			if(not_seen_area_flg == 0)
			{
				new_L_Getdt(&spt ,pbs ,pnote_param);

#ifdef CSV_DB_DOG
			F_WRITE(fp,spt.sensdt);	//filedebug
#endif

				ary_num = (u8)(spt.sensdt * qp);
				hist_pix_val_ary_out[ary_num]++;

				out_count++;

			}
		//}

		//外側 @3
		//if(out_y_scan_sub * 1.2 >= 4)
		//{
			spt.x = (s32)cx;
			spt.y = (s32)(Y  + out_y_scan_main + out_y_scan_main + out_y_scan_main);
			not_seen_area_flg = 0;

			for(j = 0; j < dog->not_seen_area_count; j++)
			{
			if((dog->not_seen_areas[j][0] < spt.x && dog->not_seen_areas[j][2] > spt.x) &&
				(dog->not_seen_areas[j][1] > spt.y && dog->not_seen_areas[j][3] < spt.y))
				{
					not_seen_area_flg = 1;
				}
			}

			if(not_seen_area_flg == 0)
			{
				new_L_Getdt(&spt ,pbs ,pnote_param);

				ary_num = (u8)(spt.sensdt * qp);

#ifdef CSV_DB_DOG
			F_WRITE(fp,spt.sensdt);	//filedebug
#endif

				hist_pix_val_ary_out[ary_num]++;

				out_count++;


			}
		//}
	}

	//ピークを発見する	内側
	cm1 = hist_pix_val_ary_in[0];
	in__count_2nd = hist_pix_val_ary_in[0];
    for (i = 1; i < bin_num; ++i)
	{
		//ピーク
		if (cm1 < hist_pix_val_ary_in[i])
		{
			in__count_2nd = cm1;
			//hist_pix_val_ary_in_2nd_peak[0] =  hist_pix_val_ary_in_peak[0];
			hist_pix_val_ary_in_peak[0] = i;
			cm1 = hist_pix_val_ary_in[i];
        }

		//2ndピーク
		else if(in__count_2nd < hist_pix_val_ary_in[i])
		{
			//hist_pix_val_ary_in_2nd_peak[0] = i;
			in__count_2nd = hist_pix_val_ary_in[i];
		}
    }

	//同じ数の層がないか調べる
	I = 1;

	for (i = 1; i < bin_num; ++i)
	{
		if (cm1 == hist_pix_val_ary_in[i] && hist_pix_val_ary_in_peak[0] != i)
		{
			hist_pix_val_ary_in_peak[I++] = i;
		}
	}

	//第二ピークの基準を計算
	peak_2nd_thr = cm1 - (cm1 * PEAK_MARGIN + 0.5);

	//抽出した第二ピークと基準を比較する。
	if(in__count_2nd > peak_2nd_thr)
	{
		//もし基準よりも大きければ同じ度数の階層はないか調べる
		for (i = 1; i < bin_num; ++i)
		{
			if (in__count_2nd == hist_pix_val_ary_in[i])
			{
				hist_pix_val_ary_in_peak[I++] = i;
			}
		}
	}

	//ピークを発見する	外側
	cm2 = hist_pix_val_ary_out[0];
	out_count_2nd = hist_pix_val_ary_out[0];
	for (i = 1; i < bin_num; ++i)
	{
		if (cm2 < hist_pix_val_ary_out[i])
		{
			//hist_pix_val_ary_out_2nd_peak[0] = hist_pix_val_ary_out_peak[0];
			out_count_2nd = cm2;
			hist_pix_val_ary_out_peak[0] = i;
			cm2 = hist_pix_val_ary_out[i];
		}

		//2ndピーク
		else if(out_count_2nd < hist_pix_val_ary_out[i])
		{
			//hist_pix_val_ary_out_2nd_peak[0] = i;
			out_count_2nd = hist_pix_val_ary_out[i];
		}

	}

	//同じ数の層がないか調べる
	J=1;
	for (i = 1; i < bin_num; ++i) {
		if (cm2 == hist_pix_val_ary_out[i] && hist_pix_val_ary_out_peak[0] != i)
		{
			hist_pix_val_ary_out_peak[J++] = i;
		}
	}

		peak_2nd_thr = cm2 - (cm2 * PEAK_MARGIN + 0.5);


	//ピークの差が5以上ならば実行しない
	if(out_count_2nd > peak_2nd_thr)
	{
		J=1;
		for (i = 1; i < bin_num; ++i)
		{
			if (out_count_2nd == hist_pix_val_ary_out[i])
			{
				hist_pix_val_ary_out_peak[J++] = i;
			}
		}
	}

#ifdef CSV_DB_DOG
	F_N(fp);				//filedebug
	F_CLOSE(fp);			//filedebug
#endif

	//カウントが5以下かつピーク階層の要素数が2以下ならば角折れとする。
	if(MIN_POINT > in_count || MIN_POINT > out_count || FREQUENCY > cm1 || FREQUENCY > cm2 )
	{
		//location
		dog->judge_reason[location] = RES_HISTOGRAM_POINT_LESS;
		return DOG_EAR;	//角折れ
	}

	//外側と内側のヒストグラムのピークを比較する。
	for (i = 0; i < I; ++i)
	{
			for (j = 0; j < J; ++j)
		{

			//ヒストグラムのピークが同じなら裂け
			if( (hist_pix_val_ary_in_peak[i] > hist_pix_val_ary_out_peak[j] - MARGIN) && 
				(hist_pix_val_ary_in_peak[i] < hist_pix_val_ary_out_peak[j] + MARGIN))
			{
				return TEAR;	//破れ
			}

			//内側のヒストグラムのピークが外側のピークを上回る場合は裂け
			if( hist_pix_val_ary_in_peak[i] > hist_pix_val_ary_out_peak[j])
			{
				return TEAR;	//破れ
			}

		}
	}

	return DOG_EAR;	//角折れ
}

//レベル計算を行う
u8	dog_level_detect(ST_DOG_EAR *dog)
{
#define MIN_LIMIT_NUM_DOG_SIZE 5
#define MAX_LIMIT_NUM_DOG_SIZE 20

#define MIN_LIMIT_NUM_DOG_AREA 806		// = 13/0.127/0.127
#define MAX_LIMIT_NUM_DOG_AREA 12400	// = 200/0.127/0.127




	float detect_res_ary[2];
	float thr_ary[2];
	u8 level_ary[4] = {MIN_LEVEL, MIN_LEVEL, MIN_LEVEL, MIN_LEVEL};
	u8 level = MIN_LEVEL;
	u8 tmp_level = 100;

	u8 tmp_level_area = 100;
	u8 tmp_level_side = 100;

	u8 i = 0;

	if(dog->comp_flg == 1)	//長辺、短辺の比較
	{
		//閾値読み込み
		thr_ary[0] = dog->threshold_long_side_mm;
		thr_ary[1] = dog->threshold_short_side_mm;

		for(i = 0; i < 4; ++i)
		{
			if(dog->judge[i] == 3)
			{
				continue;
			}

			//検知結果読み込み（1箇所
			detect_res_ary[0] = dog->long_side_mm[i];
			detect_res_ary[1] = dog->short_side_mm[i];

			//レベル計算
			tmp_level = level_detect(detect_res_ary , thr_ary ,2 ,MIN_LIMIT_NUM_DOG_SIZE ,MAX_LIMIT_NUM_DOG_SIZE);

			//最小値計算
			if(level_ary[i] > tmp_level)
			{
				level_ary[i] = tmp_level;
			}

			
		}
	}

	else if(dog->comp_flg == 0)	//面積と短辺の比較
	{

		for(i = 0; i < 4; ++i)
		{
			if(dog->judge[i] == 3)
			{
				continue;
			}

			thr_ary[0] = (float)dog->threshold_area;
			detect_res_ary[0] = (float)dog->area[i];
			tmp_level_area = level_detect(detect_res_ary , thr_ary ,1 ,MIN_LIMIT_NUM_DOG_AREA ,MAX_LIMIT_NUM_DOG_AREA);

			thr_ary[0] = dog->threshold_short_side_mm;
			detect_res_ary[0] = dog->short_side_mm[i];
			tmp_level_side = level_detect(detect_res_ary , thr_ary ,1 ,MIN_LIMIT_NUM_DOG_SIZE ,MAX_LIMIT_NUM_DOG_SIZE);

			if(tmp_level_side > tmp_level_area)
			{
				tmp_level = tmp_level_area;
			}
			else
			{
				tmp_level = tmp_level_side;
			}

			//最小値計算
			if(level_ary[i] > tmp_level)
			{
				level_ary[i] = tmp_level;
			}
		}
	}

	for(i = 0; i < 4; ++i)
	{
		//if(level_ary[i] == 101)
		//{
		//	continue;
		//}

		//最小値計算
		if(level > level_ary[i])
		{
			level = level_ary[i];
		}
	}

	return level;

}

//　END
