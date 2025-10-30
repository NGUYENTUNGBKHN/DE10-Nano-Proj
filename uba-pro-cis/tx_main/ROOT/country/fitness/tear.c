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
* @file tear.c
* @brief 裂け検知
* @date 2019/6/23 Created.
*
*	19/11/22：マルチコア対応の為、専用構造体をグローバルからローカル変数にする。　機能的な変更はない。
*	20/01/30：角折れ検知からの情報引継ぎのタイミングを変更した。
*			：角折れ検知からの情報引継ぎの有無を見分ける式の条件を変更した。
	20/4/22	：中国用に裂けの深さの累計を検知する処理を追加
*/
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>

#define EXT
#include "../common/global.h"

#define TR_SCAN_THR 200


#define CORNER_TEAR_JUDGE	//有効にすることで、角裂けの場合のtr->res_judge_tearの出力を2に変更する
							//通常の裂けは1のまま	2019/11/6

#define MIN_LIMIT_NUM_TAER 2	//深さの最小値　単位㎜
#define MAX_LIMIT_NUM_TAER 20	//深さの最大値　単位㎜

#define MAX_LIMIT_NUM_TAER_DOT 158	//深さの最大値(≒20mm)　単位dot 

#define TEAR_MAX_WHILE_COUNT_WIDTH (2000)	//裂けの幅検知の最大ループ回数


//裂け検知のメイン関数
// in バッファ番号
s16 tear(u8 buf_num, ST_TEAR *tr)
{
	//頂点座標用構造体
	ST_VERTEX pver;

	//サンプリングデータ構造体
	ST_BS* pbs = work[buf_num].pbs;

	//座標変換パラメタ構造体
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;

	//int a = sizeof(tr);

	//バッファ番号設定
	tr->buf_num = buf_num;

	//パラメタの初期化
	ini_tear_param( tr );

	if(pbs->LEorSE == SE)
	{
		tr->note_size_x = pbs->note_y_size;
		tr->note_size_y = pbs->note_x_size;
	}
	else if(pbs->LEorSE == LE)
	{
		tr->note_size_x = pbs->note_x_size;
		tr->note_size_y = pbs->note_y_size;
	}

	//紙幣の1/3の位置を計算する
	//輪郭スキャンの際に用いるパラメタ
	tr->a_third_note_size_x = tr->note_size_x / 3;
	tr->a_third_note_size_y = tr->note_size_y / 3;

	//閾値をmmからdotに変換する
	tr->threshold_width_dot = (u16)(tr->threshold_width / 0.127f);						//幅の閾値
	tr->threshold_diagonal_depth_dot = (u16)(tr->threshold_diagonal_depth / 0.127f);		//斜めの深さの閾値
	tr->threshold_horizontal_depth_dot = (u16)(tr->threshold_horizontal_depth / 0.127f);	//水平の深さの閾値
	tr->threshold_vertical_depth_dot = (u16)(tr->threshold_vertical_depth / 0.127f);		//垂直の深さの閾値

	//頂点座標取得
	get_vertex(&pver ,buf_num);

	//dpiによって異なる係数を取得する
	get_cof(pbs ,tr);


	//紙幣の輪郭部分をスキャンする
	search_tear_contour(pver ,pbs, pnote_param ,tr);

	//裂けの合計値で比較
	if( tr->total_depth_judge == 1)
	{
		tr->res_judge_tear = 0;
		tr->res_judge_tear = tear_total_depth(tr);
	}

	tr->level = tear_level_detect(tr);

	return 0;	//検知終了

}//tear end\


//輪郭部分をスキャンして、裂けの有無を調査する。
//
//[in]
//	pver　			頂点座標構造体
//　pbs				サンプリングデータ構造体
//	pnote_param		座標変換パラメタ構造体
//
//[out]
//	res_tear_depth	検知した裂けの深さ
//	res_tear_width	検知した裂けの幅
//	res_tear_count	検知した裂けの数

void search_tear_contour(ST_VERTEX pver , ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param, ST_TEAR *tr)
{
	ST_SPOINT spt;

	s16 x = 0;
	s16 y = 0;

	s16 start_x;
	s16 start_y;

	s16 ori_start_x;
	s16 ori_start_y;

	s16 end_x;
	s16 end_y;

	u8 width = 0;
	u8 depth = 0;

	s16 next = 0;

	u8 start_point_reset = 0;

	u8 corner_flg = 0;

	//u8 slope_start_flg = 0;

	s16 rec_c[5] = {0};

	s8  not_seen_area_flg = 0;
	u8  i = 0;
	spt.l_plane_tbl = tr->plane;
	spt.way = pbs->insertion_direction;


	//上辺-----------------------------------------------------------------------------------------
	//角折れの結果を参照
	//左上頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_1[0][0] != 0x0fff && tr->corner_triangle_vertex_1[0][1] != 0x0fff)
	{
		start_x = tr->corner_triangle_vertex_1[0][0] + _1mm;	//座標を裂けの頂点に変更
		start_y = pver.left_up_y;
	}
	else
	{
		start_x = pver.left_up_x + _1mm;
		start_y = pver.left_up_y;
	}

	//右上頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_1[2][0] != 0x0fff && tr->corner_triangle_vertex_1[2][1] != 0x0fff)
	{
		end_x =  tr->corner_triangle_vertex_1[2][0] - _1mm;	//座標を裂けの頂点に変更
	}
	else
	{
		end_x = pver.right_up_x - _1mm;
	}

	//ori_start_y = pver.left_up_y - tr->temporary_depth_thr;
	ori_start_y = pver.left_up_y;
	next = -tr->search_pitch;

	////券端の正確な座標を得る
	////３箇所調べて最も高い部分とする。
	////１箇所だとそこに裂けがあった場合、誤検知になる為
	//if (tr->a_third_note_size_x != 0)	//0だと無限ループになるので
	//{
	//	for (x = -tr->a_third_note_size_x; x <= tr->a_third_note_size_x; x = x + tr->a_third_note_size_x)
	//	{
	//		for (y = start_y; y > 0; --y)
	//		{
	//			spt.y = y;
	//			spt.x = x;
	//			new_L_Getdt(&spt, pbs, pnote_param);	//画素採取

	//			//媒体にぶつかるまで探索します。
	//			if (TR_SCAN_THR > spt.sensdt)
	//			{
	//				if (ori_start_y < y)
	//				{
	//					ori_start_y = y;
	//				}

	//				break;
	//			}

	//		}
	//	}
	//}


	ori_start_y = (pver.left_up_y + pver.right_up_y) / 2;

	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_y;
	rec_c[1] = ori_start_y;
	rec_c[2] = ori_start_y;
	rec_c[3] = ori_start_y;
	rec_c[4] = ori_start_y;

	//start_y = ori_start_y;

	//輪郭スキャン
	//上辺―右
	for(x = tr->search_pitch; x < end_x; x = x + tr->search_pitch)
	{

		for(y = start_y; y > -start_y; --y)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}		

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{

				////連続で下っている場合その連続数をカウントする
				//if(rec_c[1] - y > 0)
				//{
				//	cont_flg = cont_flg + 1;


				//}
				//else if(rec_c[1] - y < 0)
				//{
				//	cont_flg = 0;
				//}

				////下り開始の座標を記録する
				//if(rec_c[1] - y > 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(rec_c[1] - y <= 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(y < start_y - tr->temporary_depth_thr )
				//頂点の高さから閾値以上離れているかつ、連続して券端座標が下がっているもしくは、１つ前の高さとの差が閾値以上
				if(y <  pver.left_up_y - tr->temporary_depth_thr && (/*cont_flg >= 3 ||*/ rec_c[0] - y >= tr->temporary_depth_thr_diff_y))
				{
					//start_point_reset = 1;

					//基本紙幣外から出ないようにする。
					if(pver.left_up_y + 5 < rec_c[4])
					{
						rec_c[4] = pver.left_up_y;
					}


					//裂けの幅を調べる
					//width = search_tear_width_upper_and_lower(spt ,pver, pbs ,pnote_param ,  x , pver.left_up_y - tr->width_search_num_y ,pver.left_up_x ,pver.right_up_x ,-1);
					//width = search_tear_width_upper_and_lower(spt ,pver, pbs ,pnote_param ,  x , start_y - tr->width_search_num_y ,pver.left_up_x ,pver.right_up_x ,-1);
					corner_flg = 0;	//初期化
					width = search_tear_width_upper_and_lower(spt , pbs ,pnote_param ,  x , rec_c[4] - tr->width_search_num_y ,pver.left_up_x ,pver.right_up_x ,-1 ,&corner_flg, tr);
					if((width == 0 && tr->width_detection == 1) || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその時点で終了する
						//仮裂けだったのカウントアップはしないこととする。
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;
						x = tr->tear_width_coordinate_start;// + tr->search_pitch;

						//過去高さ情報を保存
						rec_c[4] = rec_c[3];
						rec_c[3] = rec_c[2];
						rec_c[2] = rec_c[1];
						rec_c[1] = rec_c[0];
						rec_c[0] = y;
						break;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					//左をスキャンする際、重複スキャンにならないように開始位置を設定する。
					if(tr->tear_width_coordinate_end < 0)
					{
						next = tr->tear_width_coordinate_end - tr->search_pitch;
					}

					//裂けの深さを調べる
					//depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y + 1 ,pver.left_up_y, upper_side ,pver.left_up_x ,pver.right_up_x);
					//depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y + 1 ,start_y, upper_side ,pver.left_up_x ,pver.right_up_x);
					depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y + 1 ,rec_c[4], upper_side ,pver.left_up_x ,pver.right_up_x, tr);
					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_judge_tear = 1;
						tr->res_tear_count = tr->res_tear_count + 1;

#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}
					}

					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif

					//裂けだった基準以下だった、場合でもカウントアップする
					tr->res_tear_count = tr->res_tear_count + 1;

					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					x = tr->tear_width_coordinate_start + tr->search_pitch;

					//検知した裂けの数が配列の要素数を超えた
					if(LIMIT_TEAR_COUNT-1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}


				}

				//else if(tr->edge_coordinates_detected[0] == 0)	//初回のみ
				//{
				//	tr->edge_coordinates_detected[0] = y - 1;	//実際に検知した頂点の座標を記録する
				//}

				else	//仮裂けでないポイントの座標を記録して側面スキャンの際のリミット値とする。
				{
					//最終のみ
					tr->edge_coordinates_detected[1] = y;	//実際に検知した頂点の座標を記録する

					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = y;
				}


				break;
			}

		}//y

		if (y == -start_y)	//貫通していると判断される
		{
			tr->penetration = 1;
			return;
		}

		//start_y = y + 2;
		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_y = y + 2;
		}
		else
		{
			start_point_reset = 0;
			//start_y = y+1;
			//start_y = pver.left_up_y;
		}


	}//x

	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_y;
	rec_c[1] = ori_start_y;
	rec_c[2] = ori_start_y;
	rec_c[3] = ori_start_y;
	rec_c[4] = ori_start_y;

	start_y = ori_start_y;

	//cont_flg = 0;

	//slope_start_flg = 0;

	//輪郭スキャン
	//上辺―左
	for(x = next; x > start_x; x = x - tr->search_pitch)
	{

		for(y = start_y; y > -start_y; --y)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}		

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{

				//if(rec_c[1] - y > 0)
				//{
				//	cont_flg = cont_flg + 1;
				//}
				//else if(rec_c[1] - y < 0)
				//{
				//	cont_flg = 0;
				//}

				//if(rec_c[1] - y > 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(rec_c[1] - y <= 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(y < start_y - tr->temporary_depth_thr )
				if(y <  pver.left_up_y - tr->temporary_depth_thr && (/*cont_flg >= 3 ||*/ rec_c[0] - y >= tr->temporary_depth_thr_diff_y))
				{
					//start_point_reset = 1;


					if(pver.left_up_y + 5 < rec_c[4])
					{
						rec_c[4] = pver.left_up_y;
					}

					//裂けの幅を調べる
					//width = search_tear_width_upper_and_lower(spt ,pver, pbs ,pnote_param ,  x , pver.left_up_y - tr->width_search_num_y ,pver.left_up_x ,pver.right_up_x ,-1);
					//width = search_tear_width_upper_and_lower(spt ,pver, pbs ,pnote_param ,  x , start_y - tr->width_search_num_y ,pver.left_up_x ,pver.right_up_x ,-1);
					corner_flg = 0;	//初期化
					width = search_tear_width_upper_and_lower(spt , pbs ,pnote_param ,  x , rec_c[4] - tr->width_search_num_y ,pver.left_up_x ,pver.right_up_x ,-1 ,&corner_flg, tr);
					if((width == 0 && tr->width_detection == 1)  || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその地点で終了する
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;
						x = tr->tear_width_coordinate_end;//- tr->search_pitch;


						//過去高さ情報を保存
						rec_c[4] = rec_c[3];
						rec_c[3] = rec_c[2];
						rec_c[2] = rec_c[1];
						rec_c[1] = rec_c[0];
						rec_c[0] = y;

						break;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					//裂けの深さを調べる
					//depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y + 1 ,pver.left_up_y, upper_side ,pver.left_up_x ,pver.right_up_x);
					//depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y + 1 ,start_y, upper_side ,pver.left_up_x ,pver.right_up_x);
					depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y + 1 ,rec_c[4], upper_side ,pver.left_up_x ,pver.right_up_x, tr);
					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_judge_tear = 1;
						tr->res_tear_count = tr->res_tear_count + 1;
#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}
					}

					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif
					//裂けだった基準以下だった、
					tr->res_tear_count = tr->res_tear_count + 1;

					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					//tr->res_tear_count = tr->res_tear_count + 1;
					x = tr->tear_width_coordinate_end - tr->search_pitch;;


					if(LIMIT_TEAR_COUNT - 1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}

				}

				//else if(tr->edge_coordinates_detected[0] == 0)	//初回のみ
				//{
				//	tr->edge_coordinates_detected[0] = y - 1;	//実際に検知した頂点の座標を記録する
				//}

				else
				{
					//最終のみ
					tr->edge_coordinates_detected[0] = y;	//実際に検知した頂点の座標を記録する


					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = y;
				}


				break;
			}

		}//y

		if (y == -start_y)	//貫通していると判断される
		{
			tr->penetration = 1;
			return;
		}

		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_y = y + 2;
		}
		else	//裂けがあった場合、紙幣の頂点からにする。
		{
			start_point_reset = 0;
			//start_y = y+1;
			//start_y = pver.left_up_y;
		}


	}//x


	//-----------------------------------------------------------------------------------------


	//下辺-----------------------------------------------------------------------------------------
	//角折れの結果を参照
	//左下頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_1[1][0] != 0x0fff && tr->corner_triangle_vertex_1[1][1] != 0x0fff)
	{
		start_x = tr->corner_triangle_vertex_1[1][0] + _1mm;	//座標を裂けの頂点に変更
		start_y = pver.left_down_y;
	}
	else
	{
		start_x = pver.left_down_x + _1mm;
		start_y = pver.left_down_y;
	}

	//右下頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_1[3][0] != 0x0fff && tr->corner_triangle_vertex_1[3][1] != 0x0fff)
	{
		end_x =  tr->corner_triangle_vertex_1[3][0] - _1mm;	//座標を裂けの頂点に変更
	}
	else
	{
		end_x = pver.right_down_x - _1mm;
	}


	//ori_start_y = pver.left_down_y + tr->temporary_depth_thr;
	ori_start_y = pver.left_down_y;
	next = -tr->search_pitch;
	//if (tr->a_third_note_size_x != 0)	//0だと無限ループになるので
	//{
	//	for (x = -tr->a_third_note_size_x; x <= tr->a_third_note_size_x; x = x + tr->a_third_note_size_x)
	//	{
	//		for (y = start_y; y < 0; ++y)
	//		{
	//			spt.y = y;
	//			spt.x = x;
	//			new_L_Getdt(&spt, pbs, pnote_param);	//画素採取

	//			//媒体にぶつかるまで探索します。
	//			if (TR_SCAN_THR > spt.sensdt)
	//			{
	//				if (ori_start_y > y)
	//				{
	//					ori_start_y = y;
	//				}

	//				break;
	//			}

	//		}
	//	}
	//}


	ori_start_y = (pver.left_down_y + pver.right_down_y) / 2;


	//下辺―右
	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_y;
	rec_c[1] = ori_start_y;
	rec_c[2] = ori_start_y;
	rec_c[3] = ori_start_y;
	rec_c[4] = ori_start_y;


	start_y = ori_start_y;
	//cont_flg = 0;
	//slope_start_flg = 0;


	//輪郭スキャン
	for(x = tr->search_pitch; x < end_x; x = x + tr->search_pitch)
	{
		for(y = start_y; y < 0; ++y)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}		

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{
				//rec_c[5] = rec_c[4];
				//rec_c[4] = rec_c[3];
				//rec_c[3] = rec_c[2];
				//rec_c[2] = rec_c[1];
				//rec_c[1] = rec_c[0];
				//rec_c[0] = y;

				//if(rec_c[1] - y < 0)
				//{
				//	cont_flg = cont_flg + 1;
				//}
				//else if(rec_c[1] - y > 0)
				//{
				//	cont_flg = 0;
				//}

				////下り開始の座標を記録する
				//if(rec_c[1] - y < 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(rec_c[1] - y > 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(y > pver.left_down_y + tr->temporary_depth_thr )
				if(y >  pver.left_down_y + tr->temporary_depth_thr && (/*cont_flg >= 3 ||*/ y - rec_c[0] >= tr->temporary_depth_thr_diff_y))
				{

					//y = y - 1;

					//start_point_reset = 1;


					if(pver.left_down_y - 5  > rec_c[4])
					{
						rec_c[4] = pver.left_down_y;
					}

					//裂けの幅を調べる
					corner_flg = 0;	//初期化
					//width = search_tear_width_upper_and_lower(spt ,pver, pbs ,pnote_param ,  x , pver.left_down_y + tr->width_search_num_y ,pver.left_down_x ,pver.right_down_x ,1);
					width = search_tear_width_upper_and_lower(spt , pbs ,pnote_param ,  x , rec_c[4]  + tr->width_search_num_y ,pver.left_down_x ,pver.right_down_x ,1 ,&corner_flg, tr);
					if((width == 0 && tr->width_detection == 1)  || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその地点で終了する
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;
						x = tr->tear_width_coordinate_start;// + tr->search_pitch;


						//過去高さ情報を保存
						rec_c[4] = rec_c[3];
						rec_c[3] = rec_c[2];
						rec_c[2] = rec_c[1];
						rec_c[1] = rec_c[0];
						rec_c[0] = y;
						break;
					}


					if(tr->tear_width_coordinate_end < 0)
					{
						next = tr->tear_width_coordinate_end - tr->search_pitch;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					//裂けの深さを調べる
					//depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y - 1 ,pver.left_down_y ,lower_side ,pver.left_up_x ,pver.right_up_x);
					depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y - 1 ,rec_c[4]  ,lower_side ,pver.left_up_x ,pver.right_up_x, tr);
					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_judge_tear = 1;
						tr->res_tear_count = tr->res_tear_count + 1;
#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}
					}



					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif
					//裂けだった基準以下だった、
					tr->res_tear_count = tr->res_tear_count + 1;

					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					//tr->res_tear_count = tr->res_tear_count + 1;
					x = tr->tear_width_coordinate_start + tr->search_pitch;

					if(LIMIT_TEAR_COUNT - 1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}

				}

				//else if(tr->edge_coordinates_detected[3] == 0)	//初回のみ
				//{
				//	tr->edge_coordinates_detected[3] = y - 1;	//実際に検知した頂点の座標を記録する
				//}

				else
				{
					//最終のみ
					tr->edge_coordinates_detected[3] = y;	//実際に検知した頂点の座標を記録する


					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = y;
				}

				break;
			}

		}//y

		//start_x = x + 2;
		//start_y = y - 2;

		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_y = y - 2;
		}
		else	//裂けがあった場合、紙幣の頂点からにする。
		{
			start_point_reset = 0;
			//start_y = pver.left_down_y;
		}

	}//x



	//下辺―左
	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_y;
	rec_c[1] = ori_start_y;
	rec_c[2] = ori_start_y;
	rec_c[3] = ori_start_y;
	rec_c[4] = ori_start_y;

	start_y = ori_start_y;
	//cont_flg = 0;
	//slope_start_flg = 0;

	//輪郭スキャン
	for(x = next; x > start_x; x = x - tr->search_pitch)
	{
		for(y = start_y; y < 0; ++y)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{
				//rec_c[5] = rec_c[4];
				//rec_c[4] = rec_c[3];
				//rec_c[3] = rec_c[2];
				//rec_c[2] = rec_c[1];
				//rec_c[1] = rec_c[0];
				//rec_c[0] = y;

				//if(rec_c[1] - y < 0)
				//{
				//	cont_flg = cont_flg + 1;
				//}
				//else if(rec_c[1] - y > 0)
				//{
				//	cont_flg = 0;
				//}

				//下り開始の座標を記録する
				//if(rec_c[1] - y < 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(rec_c[1] - y >= 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(y > pver.left_down_y + tr->temporary_depth_thr )
				if(y >  pver.left_down_y + tr->temporary_depth_thr && (y - rec_c[0] >= tr->temporary_depth_thr_diff_y))
				{

					//y = y - 1;

					//start_point_reset = 1;

					if(pver.left_down_y > rec_c[4])
					{
						rec_c[4] = pver.left_down_y;
					}

					//裂けの幅を調べる
					corner_flg = 0;	//初期化
					//width = search_tear_width_upper_and_lower(spt ,pver, pbs ,pnote_param ,  x , pver.left_down_y + tr->width_search_num_y ,pver.left_down_x ,pver.right_down_x ,1);
					width = search_tear_width_upper_and_lower(spt, pbs ,pnote_param ,  x , rec_c[4]  + tr->width_search_num_y ,pver.left_down_x ,pver.right_down_x ,1 ,&corner_flg, tr);
					if((width == 0 && tr->width_detection == 1)  || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその地点で終了する
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;
						x = tr->tear_width_coordinate_end;// - tr->search_pitch;


						//過去高さ情報を保存
						rec_c[4] = rec_c[3];
						rec_c[3] = rec_c[2];
						rec_c[2] = rec_c[1];
						rec_c[1] = rec_c[0];
						rec_c[0] = y;
						break;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					//裂けの深さを調べる
					//depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y - 1 ,pver.left_down_y ,lower_side ,pver.left_up_x ,pver.right_up_x);
					depth = search_tear_depth_upper_and_lower(spt , pver ,pbs ,pnote_param ,  x , y - 1 ,rec_c[4]  ,lower_side ,pver.left_up_x ,pver.right_up_x, tr);
					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_judge_tear = 1;
						tr->res_tear_count = tr->res_tear_count + 1;
#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}
					}
					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif
					//裂けだった基準以下だった、
					tr->res_tear_count = tr->res_tear_count + 1;

					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					//tr->res_tear_count = tr->res_tear_count + 1;
					x = tr->tear_width_coordinate_end - tr->search_pitch;


					if(LIMIT_TEAR_COUNT - 1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}

				}

				else
				{
					tr->edge_coordinates_detected[2] = y;	//実際に検知した頂点の座標を記録する


					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = y;
				}

				//else
				//{
				//	//最終のみ
				//	tr->edge_coordinates_detected[2] = y + 1;	//実際に検知した頂点の座標を記録する
				//}

				break;
			}

		}//y

		//start_y = y - 2;

		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_y = y - 2;
		}
		else	//裂けがあった場合、紙幣の頂点からにする。
		{
			start_point_reset = 0;
			//start_y = pver.left_down_y;
		}

	}//x



	//-----------------------------------------------------------------------------------------


	//左辺-----------------------------------------------------------------------------------------
	//角折れの結果を参照
	//左上頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_2[0][0] != 0x0fff && tr->corner_triangle_vertex_2[0][1] != 0x0fff)
	{
		start_y = tr->corner_triangle_vertex_2[0][1] - _1mm;	//座標を裂けの頂点に変更
		start_x = pver.left_up_x;
	}
	else
	{
		start_y = tr->edge_coordinates_detected[0] - 4;
		start_x = pver.left_up_x;
	}

	//右上頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_2[1][0] != 0x0fff && tr->corner_triangle_vertex_2[1][1] != 0x0fff)
	{
		end_y = tr->corner_triangle_vertex_2[1][1] + _1mm;	//座標を裂けの頂点に変更
	}
	else
	{
		end_y = tr->edge_coordinates_detected[2] + 4;
	}



	ori_start_x = pver.left_up_x + tr->temporary_depth_thr;
	next = tr->search_pitch;
	//if (tr->a_third_note_size_y != 0)	//0だと無限ループになるので
	//{
	//	for (y = -tr->a_third_note_size_y; y <= tr->a_third_note_size_y; y = y + tr->a_third_note_size_y)
	//	{
	//		for (x = start_x; x < 0; ++x)
	//		{
	//			spt.y = y;
	//			spt.x = x;
	//			new_L_Getdt(&spt, pbs, pnote_param);	//画素採取

	//			//媒体にぶつかるまで探索します。
	//			if (TR_SCAN_THR > spt.sensdt)
	//			{
	//				if (ori_start_x > x)
	//				{
	//					ori_start_x = x + 1;
	//				}

	//				break;
	//			}

	//		}
	//	}
	//}

	ori_start_x = (pver.left_up_x + pver.left_down_x) / 2;

	//左辺ー下
	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_x;
	rec_c[1] = ori_start_x;
	rec_c[2] = ori_start_x;
	rec_c[3] = ori_start_x;
	rec_c[4] = ori_start_x;
	//rec_c[5] = ori_start_x;

	start_x = ori_start_x;
	//cont_flg = 0;
	//slope_start_flg = 0;

	//輪郭スキャン
	for(y = -tr->search_pitch; y > end_y; y = y - tr->search_pitch)
	{
		for(x = start_x; x < (start_x*-1); ++x)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}	

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{
				//rec_c[5] = rec_c[4];
				//rec_c[4] = rec_c[3];
				//rec_c[3] = rec_c[2];
				//rec_c[2] = rec_c[1];
				//rec_c[1] = rec_c[0];
				//rec_c[0] = x;

				//if(x - rec_c[1] > 0)
				//{
				//	cont_flg = cont_flg + 1;
				//}
				//else if(x - rec_c[1] < 0)
				//{
				//	cont_flg = 0;
				//}

				////下り開始の座標を記録する
				//if(x - rec_c[1] > 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(x - rec_c[1] <= 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(x > pver.left_up_x + tr->temporary_depth_thr )
				//頂点の高さから閾値より低いかつ　連続で下がっていっているもしくは、前回との差が閾値以上
				if(x >  pver.left_up_x + tr->temporary_depth_thr && (/*cont_flg >= 3 ||*/ x - rec_c[0] >= tr->temporary_depth_thr_diff_x))
				{
					//start_point_reset = 1;

					if(pver.left_up_x > rec_c[4])
					{
						rec_c[4] = pver.left_up_x;
					}

					//輪郭位置より2ミリ以上深くスキャンされているので裂けありと判断する
					//裂けの幅を調べる
					corner_flg = 0;	//初期化
					//width = search_tear_width_left_and_right(spt , pbs ,pnote_param ,pver.left_up_x + tr->width_search_num_x ,y ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0], 1 );
					width = search_tear_width_left_and_right(spt , pbs ,pnote_param ,rec_c[4] + tr->width_search_num_x ,y ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0], 1  ,&corner_flg, tr);
					if((width == 0 && tr->width_detection == 1)  || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその時点で終了する
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;
						y = tr->tear_width_coordinate_end;// - tr->search_pitch;


						//過去高さ情報を保存
						rec_c[4] = rec_c[3];
						rec_c[3] = rec_c[2];
						rec_c[2] = rec_c[1];
						rec_c[1] = rec_c[0];
						rec_c[0] = x;
						break;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					if(tr->tear_width_coordinate_start > 0)
					{
						next = tr->tear_width_coordinate_start + tr->search_pitch;
					}

					//裂けの深さを調べる
					//depth = search_tear_depth_left_and_right(spt , pver, pbs ,pnote_param ,  x - 1 , y ,pver.left_up_x, left_side ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0]);
					depth = search_tear_depth_left_and_right(spt , pver, pbs ,pnote_param ,  x - 1 , y ,rec_c[4], left_side ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0], tr);

					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_tear_count = tr->res_tear_count + 1;
						tr->res_judge_tear = 1;
#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}
					}

					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif
					//裂けだった基準以下だった、
					tr->res_tear_count = tr->res_tear_count + 1;

					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					//tr->res_tear_count = tr->res_tear_count + 1;
					y = tr->tear_width_coordinate_end - tr->search_pitch;

					if(LIMIT_TEAR_COUNT - 1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}

				}

				else
				{

					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = x;
				}

				break;
			}

		}//y

		if (x == (start_x * -1))	//貫通していると判断される
		{
			tr->penetration = 2;
			return;
		}

		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_x = x - 2;
		}
		else	//裂けがあった場合、紙幣の頂点からにする。
		{
			start_point_reset = 0;
			//start_x = pver.left_up_x;
		}

	}//x


	//左辺ー上
	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_x;
	rec_c[1] = ori_start_x;
	rec_c[2] = ori_start_x;
	rec_c[3] = ori_start_x;
	rec_c[4] = ori_start_x;
	//rec_c[5] = ori_start_x;
	start_x = ori_start_x;
	//cont_flg = 0;
	//slope_start_flg = 0;

	//輪郭スキャン
	for(y = next; y < start_y; y = y + tr->search_pitch)
	{
		for(x = start_x; x < (start_x * -1); ++x)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}		

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{
				////rec_c[5] = rec_c[4];
				//rec_c[4] = rec_c[3];
				//rec_c[3] = rec_c[2];
				//rec_c[2] = rec_c[1];
				//rec_c[1] = rec_c[0];
				//rec_c[0] = x;

				//if(x - rec_c[1] > 0)
				//{
				//	cont_flg = cont_flg + 1;
				//}
				//else if(x - rec_c[1] < 0)
				//{
				//	cont_flg = 0;
				//}

				////下り開始の座標を記録する
				//if(x - rec_c[1] > 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(x - rec_c[1] <= 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(x > pver.left_up_x + tr->temporary_depth_thr )
				//頂点の高さから閾値より低いかつ　連続で下がっていっているもしくは、前回との差が閾値以上
				if(x >  pver.left_up_x + tr->temporary_depth_thr && (/*cont_flg >= 3 ||*/ x - rec_c[0] >= tr->temporary_depth_thr_diff_x))
				{
					//start_point_reset = 1;

					if(pver.left_up_x > rec_c[4])
					{
						rec_c[4] = pver.left_up_x;
					}

					//輪郭位置より2ミリ以上深くスキャンされているので裂けありと判断する
					//裂けの幅を調べる
					corner_flg = 0;	//初期化
					//width = search_tear_width_left_and_right(spt , pbs ,pnote_param ,pver.left_up_x + tr->width_search_num_x ,y ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0], 1 );
					width = search_tear_width_left_and_right(spt , pbs ,pnote_param ,rec_c[4] + tr->width_search_num_x ,y ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0], 1 ,&corner_flg , tr);
					if((width == 0 && tr->width_detection == 1)  || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその時点で終了する
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;
						y = tr->tear_width_coordinate_start;// + tr->search_pitch;

					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = x;

						break;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					//裂けの深さを調べる
					//depth = search_tear_depth_left_and_right(spt , pver, pbs ,pnote_param ,  x - 1 , y ,pver.left_up_x, left_side ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0]);
					depth = search_tear_depth_left_and_right(spt , pver, pbs ,pnote_param ,  x - 1 , y ,rec_c[4], left_side ,tr->edge_coordinates_detected[2] ,tr->edge_coordinates_detected[0], tr);

					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_tear_count = tr->res_tear_count + 1;
						tr->res_judge_tear = 1;
#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}

					}

					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif

					//裂けだった基準以下だった、
					tr->res_tear_count = tr->res_tear_count + 1;

					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					//tr->res_tear_count = tr->res_tear_count + 1;
					y = tr->tear_width_coordinate_start + _1mm;

					if(LIMIT_TEAR_COUNT - 1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}

				}

				else
				{

					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = x;
				}



				break;
			}

		}//y

		if (x == (start_x * -1))	//貫通していると判断される
		{
			tr->penetration = 2;
			return;
		}

		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_x = x - 2;
		}
		else	//裂けがあった場合、紙幣の頂点からにする。
		{
			start_point_reset = 0;
			//start_x = pver.left_up_x;
		}

	}//x

	//-----------------------------------------------------------------------------------------



	//右辺-----------------------------------------------------------------------------------------
	//角折れの結果を参照
	//左上頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_2[2][0] != 0x0fff && tr->corner_triangle_vertex_2[2][1] != 0x0fff)
	{
		start_y = tr->corner_triangle_vertex_2[2][1] - _1mm;	//座標を裂けの頂点に変更
		start_x = pver.right_up_x;
	}
	else
	{
		start_y = tr->edge_coordinates_detected[1] - 4;
		start_x = pver.right_up_x;
	}

	//右上頂点に角裂けがある場合
	if(tr->corner_triangle_vertex_2[3][0] != 0x0fff && tr->corner_triangle_vertex_2[3][1] != 0x0fff)
	{
		end_y = tr->corner_triangle_vertex_2[3][1] + _1mm;	//座標を裂けの頂点に変更
	}
	else
	{
		end_y = tr->edge_coordinates_detected[3] + 4;
	}

	for(x = start_x; x > 0; --x)
	{
		spt.x = x;
		spt.y = 0;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

		//媒体にぶつかるまで探索します。
		if(TR_SCAN_THR > spt.sensdt)
		{
			ori_start_x = x;
			break;
		}

	}

	ori_start_x = pver.right_up_x - tr->temporary_depth_thr;
	next = tr->search_pitch;
	//if (tr->a_third_note_size_y != 0)	//0だと無限ループになるので
	//{
	//	for (y = -tr->a_third_note_size_y; y <= tr->a_third_note_size_y; y = y + tr->a_third_note_size_y)
	//	{
	//		for (x = start_x; x > 0; --x)
	//		{
	//			spt.y = y;
	//			spt.x = x;
	//			new_L_Getdt(&spt, pbs, pnote_param);	//画素採取

	//			//媒体にぶつかるまで探索します。
	//			if (TR_SCAN_THR > spt.sensdt)
	//			{
	//				if (ori_start_x < x)
	//				{
	//					ori_start_x = x - 1;
	//				}

	//				break;
	//			}

	//		}
	//	}
	//}

	ori_start_x = (pver.right_up_x + pver.right_down_x) / 2;

	//右辺ー下
	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_x;
	rec_c[1] = ori_start_x;
	rec_c[2] = ori_start_x;
	rec_c[3] = ori_start_x;
	rec_c[4] = ori_start_x;
	//rec_c[5] = ori_start_x;
	start_x = ori_start_x;
	//cont_flg = 0;
	//slope_start_flg = 0;

	//輪郭スキャン
	for(y = -tr->search_pitch; y > end_y; y = y - tr->search_pitch)
	{
		for(x = start_x; x > 0; --x)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}		

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{
				//rec_c[5] = rec_c[4];
				//rec_c[4] = rec_c[3];
				//rec_c[3] = rec_c[2];
				//rec_c[2] = rec_c[1];
				//rec_c[1] = rec_c[0];
				//rec_c[0] = x;

	/*			if(rec_c[1] - x > 0)
				{
					cont_flg = cont_flg + 1;
				}
				else if(rec_c[1] - x < 0)
				{
					cont_flg = 0;
				}*/

				////下り開始の座標を記録する
				//if(rec_c[1] - x > 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(rec_c[1] - x <= 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(x < pver.right_up_x - tr->temporary_depth_thr )
				//頂点の高さから閾値より低いかつ　連続で下がっていっているもしくは、前回との差が閾値以上
				if(x <  pver.right_up_x - tr->temporary_depth_thr && (/*cont_flg >= 3 ||*/ rec_c[0] - x >= tr->temporary_depth_thr_diff_x))
				{
					//start_point_reset = 1;

					if(pver.right_up_x < rec_c[4])
					{
						rec_c[4] = pver.right_up_x;
					}

					//裂けの幅を調べる
					corner_flg = 0;	//初期化
					//width = search_tear_width_left_and_right(spt, pbs ,pnote_param ,pver.right_up_x - tr->width_search_num_x ,y ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1] ,-1);
					width = search_tear_width_left_and_right(spt , pbs ,pnote_param ,rec_c[4] - tr->width_search_num_x ,y ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1] ,-1 ,&corner_flg, tr);
					if((width == 0 && tr->width_detection == 1)  || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその時点で終了する
						y = tr->tear_width_coordinate_end;// - tr->search_pitch;
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;


					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = x;

						break;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					if(tr->tear_width_coordinate_start > 0)
					{
						next = tr->tear_width_coordinate_start + tr->search_pitch;
					}

					//裂けの深さを調べる
					//depth = search_tear_depth_left_and_right(spt , pver ,pbs ,pnote_param ,  x + 1 , y ,pver.right_up_x, right_side ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1]);
					depth = search_tear_depth_left_and_right(spt , pver ,pbs ,pnote_param ,  x + 1 , y , rec_c[4], right_side ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1], tr);
					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_tear_count = tr->res_tear_count + 1;
						tr->res_judge_tear = 1;
#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}
					}

					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif

					//裂けだった基準以下だった、
					tr->res_tear_count = tr->res_tear_count + 1;

					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					//tr->res_tear_count = tr->res_tear_count + 1;
					y = tr->tear_width_coordinate_end - tr->search_pitch;

					if(LIMIT_TEAR_COUNT - 1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}
				}

								else
				{

					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = x;
				}

				break;
			}

		}//y

		//start_x = x + 2;

		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_x = x + 2;
		}
		else	//裂けがあった場合、紙幣の頂点からにする。
		{
			start_point_reset = 0;
			//start_x = pver.right_up_x;
		}

	}//x

	//右辺ー上
	//座標レコード配列を頂点のｙ座標で初期化
	//memsetの動作は安定しないのでやめる
	//memset(&rec_c[0], start_y ,sizeof(rec_c));
	rec_c[0] = ori_start_x;
	rec_c[1] = ori_start_x;
	rec_c[2] = ori_start_x;
	rec_c[3] = ori_start_x;
	rec_c[4] = ori_start_x;
	//rec_c[5] = ori_start_x;
	start_x = ori_start_x;
//	cont_flg = 0;

	//slope_start_flg = 0;

	//輪郭スキャン
	for(y = next; y < start_y; y = y + tr->search_pitch)
	{
		for(x = start_x; x > 0; --x)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			not_seen_area_flg = 0;
			for(i = 0; i < tr->not_seen_area_count; i++)
			{
				if((tr->not_seen_areas[i][0] < x && tr->not_seen_areas[i][2] > x) &&
					(tr->not_seen_areas[i][1] > y && tr->not_seen_areas[i][3] < y))
				{
					not_seen_area_flg = 1;
				}
			}		

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt || not_seen_area_flg == 1)
			{
				//rec_c[5] = rec_c[4];
	/*			rec_c[4] = rec_c[3];
				rec_c[3] = rec_c[2];
				rec_c[2] = rec_c[1];
				rec_c[1] = rec_c[0];
				rec_c[0] = x;*/

				//if(rec_c[1] - x > 0)
				//{
				//	cont_flg = cont_flg + 1;
				//}
				//else if(rec_c[1] - x < 0)
				//{
				//	cont_flg = 0;
				//}

				////下り開始の座標を記録する
				//if(rec_c[1] - x > 0 && slope_start_flg == 0)
				//{
				//	slope_start_flg = 1;
				//	rec_c[4] = rec_c[1];

				//}
				//else if(rec_c[1] - x <= 0 && slope_start_flg == 1)
				//{
				//	slope_start_flg = 0;
				//	rec_c[4] = rec_c[1];
				//}

				//if(x < pver.right_up_x - tr->temporary_depth_thr )
				//頂点の高さから閾値より低いかつ　連続で下がっていっているもしくは、前回との差が閾値以上
				if(x < pver.right_up_x - tr->temporary_depth_thr && (/*cont_flg >= 3 ||*/ rec_c[0] - x >= tr->temporary_depth_thr_diff_x))
				{
					//start_point_reset = 1;

					if(pver.right_up_x < rec_c[4])
					{
						rec_c[4] = pver.right_up_x;
					}

					//裂けの幅を調べる
					corner_flg = 0;	//初期化
					//width = search_tear_width_left_and_right(spt, pbs ,pnote_param ,pver.right_up_x - tr->width_search_num_x ,y ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1] ,-1);
					width = search_tear_width_left_and_right(spt, pbs ,pnote_param ,rec_c[4] - tr->width_search_num_x ,y ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1] ,-1 ,&corner_flg, tr);
					if((width == 0 && tr->width_detection == 1)  || tr->res_tear_width[tr->res_tear_count] == 0)
					{
						//幅が閾値以下ならばその時点で終了する
						y = tr->tear_width_coordinate_start;// + tr->search_pitch;
						//tr->res_each_judge_reason[tr->res_tear_count] = RES_WIDTH_BELOW_STANDARD;
						//tr->res_tear_count = tr->res_tear_count + 1;

					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = x;

						break;
					}

					//仮裂けありフラグ、次のスキャン座標の位置決めに用いる
					start_point_reset = 1;

					//裂けの深さを調べる
					//depth = search_tear_depth_left_and_right(spt , pver ,pbs ,pnote_param ,  x + 1 , y ,pver.right_up_x, right_side ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1]);
					depth = search_tear_depth_left_and_right(spt , pver ,pbs ,pnote_param ,  x + 1 , y , rec_c[4], right_side ,tr->edge_coordinates_detected[3] ,tr->edge_coordinates_detected[1], tr);
					if(depth == 1)	//裂けアリ
					{
						//閾値より大きい場合　処理を終了する
						tr->res_judge_reason = RES_DEPTH_AND_WIDTH_PASS;
						tr->res_tear_count = tr->res_tear_count + 1;
						tr->res_judge_tear = 1;

#ifdef CORNER_TEAR_JUDGE
						//角裂けと判定した場合
						if(corner_flg == 1)
						{
							tr->res_judge_tear = 2;
						}
#endif

						//裂けの合計値で比較しないなら
						if( tr->total_depth_judge == 0)
						{
							return ;	//終了する
						}
					}

					//whileの最大回数を超えた
					else if(depth == ERR_MAX_WHILE_OVER)
					{
						tr->res_err_code = ERR_MAX_WHILE_OVER;
						return ;
					}
#ifdef VS_DEBUG1
					else
						tr->res_tear_count = tr->res_tear_count + 1;
#endif


					//裂けを検知した際、次のスキャンポイントがその裂けをスキャンしないようにする。
					tr->res_tear_count = tr->res_tear_count + 1;
					y = tr->tear_width_coordinate_start + tr->search_pitch;

					if(LIMIT_TEAR_COUNT-1 <= tr->res_tear_count)
					{
						tr->res_err_code = ERR_MANY_DETECTION_TEARS;
						tr->res_judge_tear = 1;
						return ;
					}


				}

				else
				{

					//過去高さ情報を保存
					rec_c[4] = rec_c[3];
					rec_c[3] = rec_c[2];
					rec_c[2] = rec_c[1];
					rec_c[1] = rec_c[0];
					rec_c[0] = x;
				}

				break;
			}

		}//y

		//start_x = x + 2;

		//裂けがなかった時、次の探索開始ポイントを今回の探索ポイントから少し戻った位置からにする
		if(start_point_reset == 0)
		{
			start_x = x + 2;
		}
		else	//裂けがあった場合、紙幣の頂点からにする。
		{
			start_point_reset = 0;
			//start_x = pver.right_up_x;
		}

	}//x

	//-----------------------------------------------------------------------------------------
}




//裂けの深さを検知する関数(上、下辺用)
//検知した裂けの深さと形状を出力する。
//
//[in]
//	spt				Lgetdt用構造体
//　pbs				サンプリングデータ構造体
//	pnote_param		座標変換パラメタ構造体
//	origin_x,y		スキャン開始座標
//	vartex_x		頂点のx座標
//	location		検知するのは度の辺か
//	limit_min,max	スキャン範囲を制限する。
//
//[out]
//	tr->res_tear_type	裂けがどの形状かを格納する　vertical か diagonal
//	tr->res_tear_depth	裂けの深さ　単位は㎜
//
//[return]
//	検知結果　１：深さが閾値を超えた　０：超えてない
//
u8 search_tear_depth_upper_and_lower(ST_SPOINT spt ,ST_VERTEX pver ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 origin_x ,s16 origin_y ,s16 vartex_y ,s8 location  , s16 limit_min ,s16 limit_max, ST_TEAR *tr )
{
	s16 left_x = origin_x;
	s16 left_y = origin_y;

	s16 right_x = origin_x;
	s16 right_y = origin_y;

	u8 scan_dir[8] = {0,0,1,0,1,0,0,0};	//スキャンの実行or実行しないを決定する。
	u8 scan_res[8] = {0};				//スキャン結果を格納する。
	//上記2つの配列の要素対応
	//上辺の場合　0：下　1:右下　2：左下　3：右　4：左　5：右上　6：左上　7：上
	//下辺の場合　0：上　1:右上　2：左上　3：右　4：左　5：右下　6：左下　7：下

	s8 inc_num = 0;
	u8 climb = 0;						//

	s16 right_local_minimum_y = 0;		//右側スキャンの際の局所最小座標を記録する。
	s16 right_local_minimum_x = 0;

	s16 left_local_minimum_y = 0;		//左側スキャンの際の局所最小座標を記録する。
	s16 left_local_minimum_x = 0;

	s16 most_depth_y = 0;					//最も深い座標
	s16 most_depth_x = 0;

	u16 scan_finish_res_left = 0;
	u16 scan_finish_res_right = 0;

	s8 continuation_flg = 0;			//局所検知の結果スキャン継続を決める　1：終　0：継続

	u32	while_max_of_excution = 0;		//whileが無限ループにならないように
	u32	while_max_of_excution2 = 0;		//whileが無限ループにならないように

	ST_NOT_SECURITY_AREA nsa;			//頂点から５㎜*５㎜は検知しない

	u8 set_cord = 0;

	s8 not_seen_area_flg = 0;
	u8	i = 0;
	u8 not_scan_end_flg = 0;

	limit_min = limit_min + _1mm;		//スキャン範囲を制限する。
	limit_max = limit_max - _1mm;

	//場所に応じて係数を変更する。
	if(location == upper_side)				//上辺
	{
		nsa.area1_left_up_x		= pver.left_up_x;
		nsa.area1_left_up_y		= pver.left_up_y;
		nsa.area1_right_down_x	= pver.left_up_x + NOT_SERCH_VERTEX_AROUND;
		nsa.area1_right_down_y	= pver.left_up_y - NOT_SERCH_VERTEX_AROUND;

		nsa.area2_left_up_x		= pver.right_up_x - NOT_SERCH_VERTEX_AROUND;
		nsa.area2_left_up_y		= pver.right_up_y;
		nsa.area2_right_down_x	= pver.right_up_x;
		nsa.area2_right_down_y	= pver.right_up_y - NOT_SERCH_VERTEX_AROUND;


		right_local_minimum_y = 0x0fff;
		left_local_minimum_y = 0x0fff;
		inc_num = -1;
	}
	else									//下辺
	{
		nsa.area1_left_up_x		= pver.left_down_x;
		nsa.area1_left_up_y		= pver.left_down_y + NOT_SERCH_VERTEX_AROUND;
		nsa.area1_right_down_x	= pver.left_down_x + NOT_SERCH_VERTEX_AROUND;
		nsa.area1_right_down_y	= pver.left_down_y;

		nsa.area2_left_up_x		= pver.right_down_x - NOT_SERCH_VERTEX_AROUND;
		nsa.area2_left_up_y		= pver.right_down_y + NOT_SERCH_VERTEX_AROUND;
		nsa.area2_right_down_x	= pver.right_down_x;
		nsa.area2_right_down_y	= pver.right_down_y;

		right_local_minimum_y = 0xf000;
		left_local_minimum_y = 0xf000;
		inc_num = 1;
	}

	//※コメント分は上辺の場合で解釈してください。
	//原点から左側へスキャンしていく
	while_max_of_excution = 0;
	while(while_max_of_excution < MAX_WHILE)
	{
		//除外エリアのチェック
		not_seen_area_flg = 0;
		for(i = 0; i < tr->not_seen_area_count; i++)
		{
			if((tr->not_seen_areas[i][0] <= left_x && tr->not_seen_areas[i][2] >= left_x) &&
				(tr->not_seen_areas[i][1] >= left_y && tr->not_seen_areas[i][3] <= left_y))
			{
				not_seen_area_flg = 1;
			}
		}
		if(not_seen_area_flg == 1)
		{
			left_local_minimum_x = left_x;
			left_local_minimum_y = left_y;
			if(while_max_of_excution == 0)
			{
				not_scan_end_flg += 1; 
			}
			break;
		}

		while_max_of_excution = while_max_of_excution + 1;


		depth_scan_module_upper_and_lower(spt ,scan_dir, pbs ,pnote_param , left_x, left_y , scan_res ,inc_num);	//現在の点の周囲をスキャンする

		//スキャン結果に応じて画素を進める
		if(scan_res[0])			//真下　左下や左よりも真下の方が優先
		{
			left_y = left_y + inc_num;
		}
		else if(scan_res[2])	//左下　左よりも左下の方が優先
		{
			left_x = left_x - 1;
			left_y = left_y + inc_num;
		}
		else if(scan_res[4])	//左
		{
			left_x = left_x - 1;
		}
		else 					//進む画素がない。
		{
			//ローカルミニマム調査用にスキャン範囲を設定
			scan_dir[0] = 0;	//下
			scan_dir[1] = 0;	//右下
			scan_dir[2] = 0;	//左下
			scan_dir[3] = 0;	//右
			scan_dir[4] = 0;	//左
			scan_dir[5] = 0;	//右上
			scan_dir[6] = 1;	//左上
			scan_dir[7] = 1;	//上

			//実行する前に現状の最深の座標を記録する
			//上下で値が反転する
			if(location == upper_side)					//上
			{
				if(left_local_minimum_y > left_y)
				{
					left_local_minimum_x = left_x;
					left_local_minimum_y = left_y;
				}
			}
			else if(location == lower_side)				//下
			{
				if(left_local_minimum_y < left_y)
				{
					left_local_minimum_x = left_x;
					left_local_minimum_y = left_y;
				}
			}

			//座標がセットされました
			set_cord = 1;

			//フラグをりセット
			continuation_flg = 1;
			climb = 0;
			while_max_of_excution2 = 0;
			while(climb != LIMIT_CLIMB)	//LIMIT回実行して左上への遷移がなければ処理を終了する。
			{
				while_max_of_excution2++;
				if (while_max_of_excution2 > MAX_WHILE)	//無限ループ防止用
				{
					return ERR_MAX_WHILE_OVER;
				}

				//除外エリアのチェック
				not_seen_area_flg = 0;
				for(i = 0; i < tr->not_seen_area_count; i++)
				{
					if((tr->not_seen_areas[i][0] <= left_x && tr->not_seen_areas[i][2] >= left_x) &&
						(tr->not_seen_areas[i][1] >= left_y && tr->not_seen_areas[i][3] <= left_y))
					{
						not_seen_area_flg = 1;
					}
				}
				if(not_seen_area_flg == 1)
				{
					break;
				}

				depth_scan_module_upper_and_lower(spt ,scan_dir, pbs ,pnote_param , left_x, left_y , scan_res ,inc_num);	//現在の点の周囲をスキャンする

				//スキャン結果に応じて画素を進める
				if(scan_res[6])			//左上
				{
					//左上に遷移した場合現状のポイントは局所最小とみなし、再び最深部を求めて検知を再開します。
					left_x = left_x - 1;
					left_y = left_y - inc_num;

					//再び検知を再開するのでスキャン範囲をセット
					scan_dir[0] = 1;	//下
					scan_dir[1] = 0;	//右下
					scan_dir[2] = 1;	//左下
					scan_dir[3] = 0;	//右
					scan_dir[4] = 1;	//左
					scan_dir[5] = 0;	//右上
					scan_dir[6] = 0;	//左上
					scan_dir[7] = 0;	//上

					continuation_flg = 0;
					break;
				}
				else if(scan_res[7])	//上
				{
					left_y = left_y - inc_num;
					climb = climb + 1;				//上にｎ回スキャンした場合、ここが最深部とみなす
				}
				else
				{
					break;							//左上にも上にも行けなかった。　最深部とみなす
				}

				//結果をリセット
				memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
			}

		}

		//スキャン範囲が設定閾値を超えたなら
		//ｘ座標をみて形状を確認し、適切な閾値と比較する。
		most_depth_y = ABS(vartex_y - left_y);
		//if(most_depth_y > tr->threshold_diagonal_depth_dot * 1.6)	//
		if(most_depth_y > MAX_LIMIT_NUM_TAER_DOT)	//
		{
			//頂点から5x5㎜以内なら終了しない
			if(!(((nsa.area1_left_up_x < left_x && nsa.area1_right_down_x > left_x) &&
				(nsa.area1_left_up_y > left_y && nsa.area1_right_down_y < left_y))||
				((nsa.area2_left_up_x < left_x && nsa.area2_right_down_x > left_x) &&
				(nsa.area2_left_up_y > left_y && nsa.area2_right_down_y < left_y))))
			{
				tr->res_tear_type[tr->res_tear_count] =not_required;
				tr->res_tear_depth[tr->res_tear_count] = most_depth_y * 0.127f;	//深さ
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_PASS_SCANING;
				return 1;
			}
		}

		if(continuation_flg == 1)
		{
			//ローカルミニマム調査でもスキャン続行にならなかったら
			//最深部とみなして処理を終了する。
			break;
		}


		if(ABS(left_y) > ABS(origin_y) + 1/*- tr->y_scan_limit_margin*/)	//ｙ座標が頂点のy座標の高さと同じになったら
		{

			//scan_finish_res_left = RES_SCAN_OUT_OF_AREA_DEPTH;
			scan_finish_res_left = RES_SCAN_OUT_OF_AREA_DEPTH;
			//処理を終了する。
			break;
		}


		//幅方向に紙幣エリアから出た場合
		if(limit_min > left_x)
		{

			//最終地点が最深部かもしれないので記録する
			if(location == upper_side)					//上
			{
				if(left_local_minimum_y > left_y)
				{
					left_local_minimum_x = left_x;
					left_local_minimum_y = left_y;
				}
			}
			else if(location == lower_side)				//下
			{
				if(left_local_minimum_y < left_y)
				{
					left_local_minimum_x = left_x;
					left_local_minimum_y = left_y;
				}
			}

			//座標がセットされました
			set_cord = 1;

			scan_finish_res_left = RES_SCAN_OUT_OF_AREA_WIDTH;
			//出たらその時点で終了する。
			break;
		}

		//結果をリセット
		memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
		scan_dir[0] = 1;

	}

	//whileの最大回数を超えた
	if(while_max_of_excution > MAX_WHILE)
	{
		return ERR_MAX_WHILE_OVER;
	}

	//右側スキャンの準備
	scan_dir[0] = 0;
	scan_dir[1] = 1;
	scan_dir[3] = 1;
	scan_dir[2] = 0;
	scan_dir[4] = 0;
	scan_dir[5] = 0;
	scan_dir[6] = 0;
	scan_dir[7] = 0;
	continuation_flg = 0;

	//原点から右側
	while_max_of_excution = 0;
	while(while_max_of_excution < MAX_WHILE)
	{	//除外エリアのチェック
		not_seen_area_flg = 0;
		for(i = 0; i < tr->not_seen_area_count; i++)
		{
			if((tr->not_seen_areas[i][0] <= right_x && tr->not_seen_areas[i][2] >= right_x) &&
				(tr->not_seen_areas[i][1] >= right_y && tr->not_seen_areas[i][3] <= right_y))
			{
				not_seen_area_flg = 1;
			}
		}
		if(not_seen_area_flg == 1)
		{
			right_local_minimum_x = right_x;
			right_local_minimum_y = right_y;
			if(while_max_of_excution == 0)
			{
				not_scan_end_flg += 1; 
			}
			break;
		}
		while_max_of_excution = while_max_of_excution + 1;

		depth_scan_module_upper_and_lower(spt ,scan_dir, pbs ,pnote_param , right_x, right_y , scan_res ,inc_num);//現在の点の周囲をスキャンする

		//スキャン結果に応じて画素を進める
		if(scan_res[0])			//真下　右下や右よりも真下の方が優先
		{
			right_y = right_y + inc_num;
		}
		else if(scan_res[1])	//右下　右よりも右下の方が優先
		{
			right_x = right_x + 1;
			right_y = right_y + inc_num;
		}
		else if(scan_res[3])	//右
		{
			right_x = right_x + 1;
		}
		else //if(one_down_flg == 1)					//進む画素がない、かつスキャン中１度は下った。
		{
			//ローカルミニマム調査
			scan_dir[0] = 0;	//下
			scan_dir[1] = 0;	//右下
			scan_dir[2] = 0;	//左下
			scan_dir[3] = 0;	//右
			scan_dir[4] = 0;	//左
			scan_dir[5] = 1;	//右上
			scan_dir[6] = 0;	//左上
			scan_dir[7] = 1;	//上

			//実行する前に現状の最深の座標を記録する
			if(location == upper_side)
			{
				if(right_local_minimum_y > right_y)
				{
					right_local_minimum_y = right_y;
					right_local_minimum_x = right_x;
				}
			}
			else if(location == lower_side)
			{
				if(right_local_minimum_y < right_y)
				{
					right_local_minimum_y = right_y;
					right_local_minimum_x = right_x;
				}
			}

			//座標がセットされました
			set_cord = 1;

			//フラグをセット
			continuation_flg = 1;
			climb = 0;
			while_max_of_excution2 = 0;
			while(climb != LIMIT_CLIMB)	//ｎ回実行して左上への遷移がなければ処理を終了する。
			{

				while_max_of_excution2++;
				if (while_max_of_excution2 > MAX_WHILE)	//無限ループ防止用
				{
					return ERR_MAX_WHILE_OVER;
				}

				//除外エリアのチェック
				not_seen_area_flg = 0;
				for(i = 0; i < tr->not_seen_area_count; i++)
				{
					if((tr->not_seen_areas[i][0] <= right_x && tr->not_seen_areas[i][2] >= right_x) &&
						(tr->not_seen_areas[i][1] >= right_y && tr->not_seen_areas[i][3] <= right_y))
					{
						not_seen_area_flg = 1;
					}
				}
				if(not_seen_area_flg == 1)
				{
					break;
				}

				depth_scan_module_upper_and_lower(spt ,scan_dir, pbs ,pnote_param , right_x, right_y , scan_res ,inc_num);//現在の点の周囲をスキャンする

				//スキャン結果に応じて画素を進める
				if(scan_res[5])			//右上
				{
					//左上に遷移した場合現状のポイントは局所最小とみなし検知を続行します。
					right_x = right_x + 1;
					right_y = right_y - inc_num;

					scan_dir[0] = 1;	//下
					scan_dir[1] = 1;	//右下
					scan_dir[2] = 0;	//左下
					scan_dir[3] = 1;	//右
					scan_dir[4] = 0;	//左
					scan_dir[5] = 0;	//右上
					scan_dir[6] = 0;	//左上
					scan_dir[7] = 0;	//上

					continuation_flg = 0;
					break;
				}
				else if(scan_res[7])	//上
				{
					right_y = right_y - inc_num;
					climb = climb + 1;
				}
				else
				{
					break;
				}

				//結果をリセット
				memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
			}

		}

		//スキャン範囲が設定閾値を超えていて
		most_depth_y = ABS(vartex_y - right_y);
		//if(most_depth_y > tr->threshold_diagonal_depth_dot * 1.6)	//レベル100を出力できるように変更
		if(most_depth_y > MAX_LIMIT_NUM_TAER_DOT)	//レベル1を出力できるように変更
		{
			//頂点の周囲5㎜いないじゃなかったら
			if(!(((nsa.area1_left_up_x < right_x && nsa.area1_right_down_x > right_x) &&
				(nsa.area1_left_up_y > right_y && nsa.area1_right_down_y < right_y))||
				((nsa.area2_left_up_x < right_x && nsa.area2_right_down_x > right_x) &&
				(nsa.area2_left_up_y > right_y && nsa.area2_right_down_y < right_y))))
			{
				//裂けアリと判断して処理終了
				tr->res_tear_type[tr->res_tear_count] =not_required;
				tr->res_tear_depth[tr->res_tear_count] = most_depth_y * 0.127f;	//深さ
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_PASS_SCANING;
				return 1;
			}
		}

		if(continuation_flg == 1)
		{
			//ローカルミニマム調査でもスキャン続行にならなかったら
			//最深部とみなして処理を終了する。
			break;
		}


		if(ABS(right_y) > ABS(origin_y) + 1/*  - tr->y_scan_limit_margin*/)	//ｙ座標が頂点のy座標の高さと同じになったら
		{
			//処理を終了する。
			scan_finish_res_right = RES_SCAN_OUT_OF_AREA_DEPTH;
			break;
		}


		//紙幣エリア外から出ないようにする
		if(limit_max < right_x)
		{
			//最終地点が最深部かもしれないので記録する
			if(location == upper_side)
			{
				if(right_local_minimum_y > right_y)
				{
					right_local_minimum_y = right_y;
					right_local_minimum_x = right_x;
				}
			}
			else if(location == lower_side)
			{
				if(right_local_minimum_y < right_y)
				{
					right_local_minimum_y = right_y;
					right_local_minimum_x = right_x;
				}
			}

			//座標がセットされました
			set_cord = 1;

			scan_finish_res_right = RES_SCAN_OUT_OF_AREA_WIDTH;
			break;
		}
		memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
		scan_dir[0] = 1;

	}

	//whileの最大回数を超えた
	if(while_max_of_excution > MAX_WHILE)
	{
		return ERR_MAX_WHILE_OVER;
	}

	//両方とも探索開始位置がエリア外だった
	if(scan_finish_res_left == RES_SCAN_OUT_OF_AREA_DEPTH && scan_finish_res_right == RES_SCAN_OUT_OF_AREA_DEPTH && set_cord == 0
		/*&& left_local_minimum_x == 0 && left_local_minimum_y == 0 && right_local_minimum_x == 0 && right_local_minimum_y == 0*/)
	{
		return 0;
	}

	//右側と左側でスキャンして最も深い位置の座標を比較する。
	if(location == upper_side)
	{
		//most_depth = ABS(MIN(right_local_minimum ,left_local_minimum));
		if(right_local_minimum_y > left_local_minimum_y)
		{
			most_depth_y = left_local_minimum_y;		//左側
			most_depth_x = left_local_minimum_x;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_left;

		}
		else if(right_local_minimum_y < left_local_minimum_y)
		{
			most_depth_y = right_local_minimum_y;	//右側
			most_depth_x = right_local_minimum_x;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_right;

		}
		else	//同じ深さ
		{
			if(scan_finish_res_left != 0)	//フラグがある方を優先
			{
				most_depth_y = left_local_minimum_y;		//左側
				most_depth_x = left_local_minimum_x;
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_left;
			}


			else
			{
				most_depth_y = right_local_minimum_y;	//右側
				most_depth_x = right_local_minimum_x;
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_right;
			}

		}
	}
	else if(location == lower_side)
	{
		//most_depth_y = ABS(MAX(right_local_minimum ,left_local_minimum));
		if(right_local_minimum_y < left_local_minimum_y)
		{
			most_depth_y = left_local_minimum_y;		//左側
			most_depth_x = left_local_minimum_x;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_left;
		}
		else if(right_local_minimum_y > left_local_minimum_y)
		{
			most_depth_y = right_local_minimum_y;	//右側
			most_depth_x = right_local_minimum_x;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_right;

		}

		else	//同じ深さ
		{
			if(scan_finish_res_left != 0)
			{
				most_depth_y = left_local_minimum_y;		//左側
				most_depth_x = left_local_minimum_x;
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_left;
			}


			else
			{
				most_depth_y = right_local_minimum_y;	//右側
				most_depth_x = right_local_minimum_x;
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_right;
			}

		}
	}

	//スキャン結果を用いて深さを算出
	tr->res_tear_depth[tr->res_tear_count] = (ABS(vartex_y - most_depth_y)) * 0.127f;	//深さ

	//if(not_scan_end_flg == 2)
	//{
	//	tr->res_tear_type[tr->res_tear_count] = vertical;	//垂直
	//	tr->res_each_judge_reason[tr->res_tear_count] = RES_SCAN_NOT_REFERENCE_AREA;
	//	return 0;		//垂直
	//}

	//形状が垂直かどうかを調べ、深部の座標を閾値と比較します。
	if((tr->tear_width_coordinate_start < most_depth_x || tr->tear_width_coordinate_end > most_depth_x) && not_scan_end_flg != 2)
	{	
		//斜めだった場合基準を満たすことはないので処理を終了する。
		//超えていたらスキャン途中に終わっている。斜め。
		tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_BELOW_STANDARD;
		tr->res_tear_type[tr->res_tear_count] = diagonal;
		return 0;

	}
	else
	{
		//垂直だった場合基準と比較します。
		tr->res_tear_type[tr->res_tear_count] = vertical;	//垂直

		if(not_scan_end_flg == 2)
		{
			tr->res_each_judge_reason[tr->res_tear_count] = RES_SCAN_NOT_REFERENCE_AREA;
			return 0;		//垂直
		}

		//基準と比較する
		if(tr->res_tear_depth[tr->res_tear_count] > tr->threshold_vertical_depth)
		{
			//スキャン中に紙幣エリア外に出たか否か
			if(tr->res_each_judge_reason[tr->res_tear_count] != RES_SCAN_OUT_OF_AREA_WIDTH)
			{
				//出ていなければ 普通の裂け
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_AND_WIDTH_PASS;
			}
			else
			{
				//角裂けチェック
				matching_tear_res_and_dog_ear_res(most_depth_x, most_depth_y, tr);

				if(tr->res_each_judge_reason[tr->res_tear_count] == RES_DEPTH_AND_WIDTH_PASS)
				{
					return 1;	//基準以上の角裂け
				}
				else if(tr->res_each_judge_reason[tr->res_tear_count] == RES_FALSE_DETECT_OF_DOG_EAR)
				{
					return 0;	//角折れを誤ってスキャンした
				}

			}
		}

		//基準以下だった場合
		else
		{
			//スキャン中に輪郭線に触れていた？
			if(tr->res_each_judge_reason[tr->res_tear_count] == 0 || tr->res_each_judge_reason[tr->res_tear_count] == RES_SCAN_OUT_OF_AREA_DEPTH)
			{
				//普通の小さな裂け
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_BELOW_STANDARD;
			}
			else
			{
				//角裂けチェック
				matching_tear_res_and_dog_ear_res(most_depth_x, most_depth_y, tr);

				//小さな角裂けもしくは折れ
				tr->res_each_judge_reason[tr->res_tear_count] = RES_OUT_OF_AREA_DEPTH_BELOW_STANDARD;
			}

			return 0;	//基準以下
		}
	}
	return 0;	//基準以下
}

//現在の画素の周囲をスキャンする関数(上辺と下辺用
//dirのフラグに従ってスキャンする方向を限定する。
//resのフラグでどこに紙幣があってどこが背景かを確認できる
//上記の配列の対応は以下の通り、
//上辺の場合　0：下　1:右下　2：左下　3：右　4：左　5：右上　6：左上　7：上
//下辺の場合　0：上　1:右上　2：左上　3：右　4：左　5：右下　6：左下　7：下
//
//[in]
//	spt				Lgetdt用構造体
//	dir				スキャンする方向を決定する配列。
//　pbs				サンプリングデータ構造体
//	pnote_param		座標変換パラメタ構造体
//	origin_x,y		スキャン開始座標
//	inc_num			インクリメント係数
//
//[out]
//	res				スキャンした結果を格納する配列　0：紙幣無　１：紙幣有
//
void depth_scan_module_upper_and_lower(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 origin_x ,s16 origin_y, u8 *res ,s8 inc_num)
{
	//上辺の場合　0：下　1:右下　2：左下　3：右　4：左　5：右上　6：左上　7：上
	//下辺の場合　0：上　1:右上　2：左上　3：右　4：左　5：右下　6：左下　7：下

	//※コメント分は上辺の場合で解釈してください。
	//下
	if(dir[0] == 1)
	{
		spt.x = origin_x;
		spt.y = origin_y + inc_num;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

		if(TR_SCAN_THR < spt.sensdt)
		{
			res[0] = 1;
		}
	}

	//右下
	if(dir[1] == 1)
	{
		spt.x = origin_x + 1;
		spt.y = origin_y + inc_num;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[1] = 1;
		}
	}

	//左下
	if(dir[2] == 1)
	{
		spt.x = origin_x - 1;
		spt.y = origin_y + inc_num;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[2] = 1;
		}
	}

	//右
	if(dir[3] == 1)
	{
		spt.x = origin_x + 1;
		spt.y = origin_y;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[3] = 1;
		}
	}

	//左
	if(dir[4] == 1)
	{
		spt.x = origin_x - 1;
		spt.y = origin_y;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[4] = 1;
		}
	}

	//右上
	if(dir[5] == 1)
	{
		spt.x = origin_x + 1;
		spt.y = origin_y - inc_num;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[5] = 1;
		}
	}

	//左上
	if(dir[6] == 1)
	{
		spt.x = origin_x - 1;
		spt.y = origin_y - inc_num;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[6] = 1;
		}
	}

	//上
	if(dir[7] == 1)
	{
		spt.x = origin_x;
		spt.y = origin_y - inc_num;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[7] = 1;
		}
	}




}

//裂けの幅を検知する(上辺と下辺用
//検知した裂けの幅と始まりと終わりの座標を出力する
//
//[in]
//	spt				Lgetdt用構造体
//	pver　			頂点座標構造体
//　pbs				サンプリングデータ構造体
//	pnote_param		座標変換パラメタ構造体
//	origin_x,y		スキャン開始座標
//	limit_min,max	スキャンのリミット
//
//[out]
//	tear_width_coordinate_start,end	裂け初めと終わりの座標　start＞end
//	tr->res_tear_depth	裂けの深さ　単位は㎜
//
//[return]
//	検知結果　１：深さが閾値を超えた　０：超えてない
//
u8 search_tear_width_upper_and_lower(ST_SPOINT spt ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 origin_x ,s16 origin_y , s16 limit_min ,s16 limit_max ,s8 increment ,u8* corner_flg , ST_TEAR *tr)
{
	s16 x = 0;
	s16 x_left = limit_min;
	s16 x_right = limit_max;
	s16 y = 0;
	u8 i = 0;
	u32 while_max_of_excution = 0;
	s16 size = (s16)(tr->note_size_x * 0.125f * 0.127f);
	//float detect_size = 0.0;
	//s16 temp_start;
	//s16 temp_end;
	//s16 size = tr->threshold_width_dot;
	tr->res_tear_width[tr->res_tear_count] = 0;
	tr->tear_width_coordinate_start = 0;
	tr->tear_width_coordinate_end = 0;



	while(i != 3)
	{
		while_max_of_excution++;
		if (TEAR_MAX_WHILE_COUNT_WIDTH < while_max_of_excution)
		{
			return 0;	//幅が閾値以下
		}

		y = origin_y + (i * tr->width_search_num_y * increment);

		for(x = origin_x; x < limit_max; x = x + 2)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt)
			{
				x_right = x;
				break;
			}

		}



		for(x = origin_x; x > limit_min;x = x - 2)
		{
			spt.x = x;
			spt.y = y;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt)
			{
				x_left = x;
				break;
			}
		}


		//temp_start = tr->tear_width_coordinate_start;
		//temp_end = tr->tear_width_coordinate_end;

		tr->tear_width_coordinate_start = x_right;
		tr->tear_width_coordinate_end = x_left;

		////前回のを記録する
		//detect_size = tr->res_tear_width[tr->res_tear_count];

		//幅を求める
		tr->res_tear_width[tr->res_tear_count] = ABS(x_right - x_left) * 0.127f;

		//紙幣サイズ1/5以上の裂けは、誤検知の可能性もあるので位置を変えてもう一度、
		if(size > tr->res_tear_width[tr->res_tear_count])
		{
			break;
		}

		i = i + 1;

	}

	////２回目以降の検知サイズが０の場合、前回のもを使う
	//if(tr->res_tear_width[tr->res_tear_count] == 0 && i != 0)
	//{
	//	//tr->res_tear_width[tr->res_tear_count] = detect_size;
	//}

	//スキャン中の裂けが角裂けか通常裂けかを分類する。
	//幅のどちらかの座標がリミット(券端)と同じ座標なら角裂けとみなす。
	if(x_left == limit_min || x_right == limit_max)
	{
		*corner_flg = 1;
	}


	if(tr->res_tear_width[tr->res_tear_count] > tr->threshold_width && tr->width_detection == 1)
	{
		return 1;	//幅が閾値以上
	}
	else
	{
		return 0;	//幅が閾値以下
	}

}




//裂けの深さを検知する関数(左辺右辺用)
//検知した裂けの深さと形状を出力する。
//
//[in]
//	spt				Lgetdt用構造体
//　pbs				サンプリングデータ構造体
//	pnote_param		座標変換パラメタ構造体
//	origin_x,y		スキャン開始座標
//	vartex_x		頂点のx座標
//	location		検知するのは度の辺か
//	limit_min,max	スキャン範囲を制限する。
//
//[out]
//	res_tear_type	裂けがどの形状かを格納する　horizonal か diagonal
//	tr->res_tear_depth	裂けの深さ　単位は㎜
//
//[return]
//	検知結果　１：深さが閾値を超えた　０：超えてない
//
u8 search_tear_depth_left_and_right(ST_SPOINT spt ,ST_VERTEX pver ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 origin_x ,s16 origin_y ,s16 vartex_x ,s8 location  , s16 limit_min ,s16 limit_max, ST_TEAR *tr )
{
	s16 up_x = origin_x;
	s16 up_y = origin_y;

	s16 down_x = origin_x;
	s16 down_y = origin_y;

	u8 scan_dir[8] = {0,0,1,0,1,0,0,0};	//スキャンの実行or実行しないを決定する。
	u8 scan_res[8] = {0};				//スキャン結果を格納する。
	//上記2つの配列の要素対応
	//左辺の場合　0：右　1:右下　2：右上　3：下　4：上　5：左下　6：左上　7：左
	//右辺の場合　0：左　1:左下　2：左上　3：下　4：上　5：右下　6：右上　7：右

	s8 inc_num = 0;
	u8 climb = 0;

	s16 up_local_minimum_y = 0;			//上側スキャンの際の局所最小座標を記録する。
	s16 up_local_minimum_x = 0;

	s16 down_local_minimum_y = 0;		//下側スキャンの際の局所最小座標を記録する。
	s16 down_local_minimum_x = 0;

	s16 most_depth_y = 0;					//最も深い座標
	s16 most_depth_x = 0;

	s8 continuation_flg = 0;			//局所検知の結果スキャン継続を決める　1：終　0：継続
	s16	while_max_of_excution = 0;
	s16	while_max_of_excution2 = 0;

	u16 scan_finish_res_up = 0;
	u16 scan_finish_res_down = 0;

	ST_NOT_SECURITY_AREA nsa = {0};

	u8 set_cord = 0;
	u8 i = 0;
	u8 not_seen_area_flg = 0;

	u8 not_scan_end_flg = 0;

	limit_min = limit_min + _1mm;
	limit_max = limit_max - _1mm;



	//場所に応じて係数を変更する。
	if(location == left_side)
	{
		nsa.area1_left_up_x		= pver.left_up_x;
		nsa.area1_left_up_y		= pver.left_up_y;
		nsa.area1_right_down_x	= pver.left_up_x + NOT_SERCH_VERTEX_AROUND;
		nsa.area1_right_down_y	= pver.left_up_y - NOT_SERCH_VERTEX_AROUND;

		nsa.area2_left_up_x		= pver.left_down_x;
		nsa.area2_left_up_y		= pver.left_down_y + NOT_SERCH_VERTEX_AROUND;
		nsa.area2_right_down_x	= pver.left_down_x + NOT_SERCH_VERTEX_AROUND;
		nsa.area2_right_down_y	= pver.left_down_y;

		up_local_minimum_x = 0xf000;
		down_local_minimum_x = 0xf000;
		inc_num = 1;
	}
	else if(location == right_side)
	{
		nsa.area1_left_up_x		= pver.right_up_x - NOT_SERCH_VERTEX_AROUND;
		nsa.area1_left_up_y		= pver.right_up_y;
		nsa.area1_right_down_x	= pver.right_up_x;
		nsa.area1_right_down_y	= pver.right_up_y - NOT_SERCH_VERTEX_AROUND;

		nsa.area2_left_up_x		= pver.right_down_x - NOT_SERCH_VERTEX_AROUND;
		nsa.area2_left_up_y		= pver.right_down_y + NOT_SERCH_VERTEX_AROUND;
		nsa.area2_right_down_x	= pver.right_down_x;
		nsa.area2_right_down_y	= pver.right_down_y;

		up_local_minimum_x = 0x0fff;
		down_local_minimum_x = 0x0fff;
		inc_num = -1;
	}

	//※コメント分は左辺の場合で解釈してください。
	//原点から上側へスキャンしていく
	while_max_of_excution = 0;
	while(while_max_of_excution != MAX_WHILE)
	{
		//除外エリアのチェック
		not_seen_area_flg = 0;
		for(i = 0; i < tr->not_seen_area_count; i++)
		{
			if((tr->not_seen_areas[i][0] <= up_x && tr->not_seen_areas[i][2] >= up_x) &&
				(tr->not_seen_areas[i][1] >= up_y && tr->not_seen_areas[i][3] <= up_y))
			{
				not_seen_area_flg = 1;
			}
		}
		if(not_seen_area_flg == 1)
		{
			up_local_minimum_x = up_x;
			up_local_minimum_y = up_y;

			if(while_max_of_excution == 0)
			{
				not_scan_end_flg += 1;
			}

			break;
		}

		while_max_of_excution = while_max_of_excution + 1;


		depth_scan_module_left_and_right(spt ,scan_dir, pbs ,pnote_param , up_x, up_y , scan_res ,inc_num);	//現在の点の周囲をスキャンする

		//スキャン結果に応じて画素を進める
		if(scan_res[0])			//真右　上や右上よりも真右の方が優先
		{
			up_x = up_x + inc_num;
		}

		else if(scan_res[2])	//右上　右よりも右上の方が優先
		{
			up_x = up_x + inc_num;
			up_y = up_y + 1;
		}

		else if(scan_res[4])	//上
		{
			up_y = up_y + 1;
		}

		else //if(one_right_flg == 1)					//進む画素がない、かつスキャン中１度は下った。
		{
			//ローカルミニマム調査
			scan_dir[0] = 0;	//右
			scan_dir[1] = 0;	//右下
			scan_dir[2] = 0;	//右上
			scan_dir[3] = 0;	//下
			scan_dir[4] = 0;	//上
			scan_dir[5] = 0;	//左下
			scan_dir[6] = 1;	//左上
			scan_dir[7] = 1;	//左

			//実行する前に現状の最深の座標を記録する
			//上下で値が反転する
			if(location == right_side)
			{
				if(up_local_minimum_x > up_x)
				{
					up_local_minimum_x = up_x;
					up_local_minimum_y = up_y;
				}
			}
			else if(location == left_side)
			{
				if(up_local_minimum_x < up_x)
				{
					up_local_minimum_x = up_x;
					up_local_minimum_y = up_y;
				}
			}

			//座標がセットされました
			set_cord = 1;

			//フラグをセット
			continuation_flg = 1;
			climb = 0;

			while_max_of_excution2 = 0;
			while(climb != LIMIT_CLIMB)	//ｎ回実行して左上への遷移がなければ処理を終了する。
			{
				while_max_of_excution2++;
				if (while_max_of_excution2 > MAX_WHILE)	//無限ループ防止用
				{
					return ERR_MAX_WHILE_OVER;
				}

				//除外エリアのチェック
				not_seen_area_flg = 0;
				for(i = 0; i < tr->not_seen_area_count; i++)
				{
					if((tr->not_seen_areas[i][0] <= up_x && tr->not_seen_areas[i][2] >= up_x) &&
						(tr->not_seen_areas[i][1] >= up_y && tr->not_seen_areas[i][3] <= up_y))
					{
						not_seen_area_flg = 1;
					}
				}
				if(not_seen_area_flg == 1)
				{
					break;
				}

				depth_scan_module_left_and_right(spt ,scan_dir, pbs ,pnote_param , up_x, up_y , scan_res ,inc_num);	//現在の点の周囲をスキャンする

				//スキャン結果に応じて画素を進める
				if(scan_res[6])			//左上
				{
					//左上に遷移した場合現状のポイントは局所最小とみなし検知を続行します。
					up_x = up_x - inc_num;
					up_y = up_y + 1;

					scan_dir[0] = 1;	//右
					scan_dir[1] = 0;	//右下
					scan_dir[2] = 1;	//右上
					scan_dir[3] = 0;	//下
					scan_dir[4] = 1;	//上
					scan_dir[5] = 0;	//左下
					scan_dir[6] = 0;	//左上
					scan_dir[7] = 0;	//左
					continuation_flg = 0;
					break;
				}
				else if(scan_res[7])	//左
				{
					up_x = up_x - inc_num;
					climb = climb + 1;
				}
				else
				{
					break;
				}

				//結果をリセット
				memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
			}

		}

		//スキャン範囲が設定閾値を超えたなら
		//ｘ座標をみて形状を確認し、適切な閾値と比較する。
		most_depth_x = ABS(vartex_x - up_x);
		//if(most_depth_x > tr->threshold_diagonal_depth_dot * 1.6)	//
		if(most_depth_x > MAX_LIMIT_NUM_TAER_DOT)	//
		{
			if(!(((nsa.area1_left_up_x < up_x && nsa.area1_right_down_x > up_x) &&
				(nsa.area1_left_up_y > up_y && nsa.area1_right_down_y < up_y))||
				((nsa.area2_left_up_x < up_x && nsa.area2_right_down_x > up_x) &&
				(nsa.area2_left_up_y > up_y && nsa.area2_right_down_y < up_y))))
			{
				tr->res_tear_type[tr->res_tear_count] =not_required;
				tr->res_tear_depth[tr->res_tear_count] = most_depth_x * 0.127f;	//深さ
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_PASS_SCANING;
				return 1;
			}

		}

		if(continuation_flg == 1)
		{
			//ローカルミニマム調査でもスキャン続行にならなかったら
			//最深部とみなして処理を終了する。
			break;
		}


		if(ABS(up_x) > ABS(origin_x) + 1/*- LIMIT_X*/)	//ｙ座標が頂点のy座標の高さと同じになったら
		{
			//処理を終了する。
			//scan_finish_res_up = RES_SCAN_OUT_OF_AREA_DEPTH;
			scan_finish_res_up = RES_SCAN_OUT_OF_AREA_DEPTH;
			break;
		}

		//紙幣エリア外から出ないようにする
		if(limit_max < up_y)
		{

			//最終地点が最深部かもしれないので記録する
			if(location == right_side)
			{
				if(up_local_minimum_x > up_x)
				{
					up_local_minimum_x = up_x;
					up_local_minimum_y = up_y;
				}
			}
			else if(location == left_side)
			{
				if(up_local_minimum_x < up_x)
				{
					up_local_minimum_x = up_x;
					up_local_minimum_y = up_y;
				}
			}

			//座標がセットされました
			set_cord = 1;

			scan_finish_res_up = RES_SCAN_OUT_OF_AREA_WIDTH;
			break;
		}

		//結果をリセット
		memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
		scan_dir[0] = 1;

	}

	//whileの最大回数を超えた
	if(while_max_of_excution > MAX_WHILE)
	{
		return ERR_MAX_WHILE_OVER;
	}



	//下側スキャンの準備
	scan_dir[0] = 0;
	scan_dir[1] = 1;
	scan_dir[3] = 1;
	scan_dir[2] = 0;
	scan_dir[4] = 0;
	scan_dir[5] = 0;
	scan_dir[6] = 0;
	scan_dir[7] = 0;
	continuation_flg = 0;

	//原点から下側
	while_max_of_excution = 0;
	while(while_max_of_excution != MAX_WHILE)
	{
		//除外エリアのチェック
		not_seen_area_flg = 0;
		for(i = 0; i < tr->not_seen_area_count; i++)
		{
			if((tr->not_seen_areas[i][0] <= down_x && tr->not_seen_areas[i][2] >= down_x) &&
				(tr->not_seen_areas[i][1] >= down_y && tr->not_seen_areas[i][3] <= down_y))
			{
				not_seen_area_flg = 1;
			}
		}
		if(not_seen_area_flg == 1)
		{
			down_local_minimum_x = down_x;
			down_local_minimum_y = down_y;

			if(while_max_of_excution == 0)
			{
				not_scan_end_flg += 1;
			}
			break;
		}

		while_max_of_excution = while_max_of_excution + 1;


		depth_scan_module_left_and_right(spt ,scan_dir, pbs ,pnote_param , down_x, down_y , scan_res ,inc_num);//現在の点の周囲をスキャンする

		//スキャン結果に応じて画素を進める
		if(scan_res[0])			//真右　下や右下よりも真右の方が優先
		{
			down_x = down_x + inc_num;
			//one_right_flg = 1;
		}
		else if(scan_res[1])	//右下　右よりも右下の方が優先
		{
			down_x = down_x + inc_num;
			down_y = down_y - 1;
			//one_right_flg = 1;
		}
		else if(scan_res[3])	//下
		{
			down_y = down_y - 1;
		}
		else //if(one_right_flg == 1)					//進む画素がない、かつスキャン中１度は下った。
		{
			//ローカルミニマム調査
			scan_dir[0] = 0;	//右
			scan_dir[1] = 0;	//右下
			scan_dir[2] = 0;	//右上
			scan_dir[3] = 0;	//下
			scan_dir[4] = 0;	//上
			scan_dir[5] = 1;	//左下
			scan_dir[6] = 0;	//左上
			scan_dir[7] = 1;	//左

			//実行する前に現状の最深の座標を記録する
			if(location == right_side)
			{
				if(down_local_minimum_x > down_x)
				{
					down_local_minimum_x = down_x;
					down_local_minimum_y = down_y;
				}
			}
			else if(location == left_side)
			{
				if(down_local_minimum_x < down_x)
				{
					down_local_minimum_x = down_x;
					down_local_minimum_y = down_y;
				}
			}

			//座標がセットされました
			set_cord = 1;

			//フラグをセット
			continuation_flg = 1;
			climb = 0;
			while_max_of_excution2 = 0;

			while(climb != LIMIT_CLIMB)	//ｎ回実行して左上への遷移がなければ処理を終了する。
			{
				while_max_of_excution2++;
				if (while_max_of_excution2 > MAX_WHILE)	//無限ループ防止用
				{
					return ERR_MAX_WHILE_OVER;
				}

				//除外エリアのチェック
				not_seen_area_flg = 0;
				for(i = 0; i < tr->not_seen_area_count; i++)
				{
					if((tr->not_seen_areas[i][0] <= down_x && tr->not_seen_areas[i][2] >= down_x) &&
						(tr->not_seen_areas[i][1] >= down_y && tr->not_seen_areas[i][3] <= down_y))
					{
						not_seen_area_flg = 1;
					}
				}
				if(not_seen_area_flg == 1)
				{
					break;
				}

				depth_scan_module_left_and_right(spt ,scan_dir, pbs ,pnote_param , down_x, down_y , scan_res ,inc_num);//現在の点の周囲をスキャンする

				//スキャン結果に応じて画素を進める
				if(scan_res[5])			//左下
				{
					//左下に遷移した場合現状のポイントは局所最小とみなし検知を続行します。
					down_x = down_x - inc_num;
					down_y = down_y - 1;

					scan_dir[0] = 1;	//右
					scan_dir[1] = 1;	//右下
					scan_dir[2] = 0;	//右上
					scan_dir[3] = 1;	//下
					scan_dir[4] = 0;	//上
					scan_dir[5] = 0;	//左下
					scan_dir[6] = 0;	//左上
					scan_dir[7] = 0;	//左
					continuation_flg = 0;
					break;

				}
				else if(scan_res[7])	//左
				{
					down_x = down_x - inc_num;
					climb = climb + 1;
				}
				else
				{
					break;
				}

				//結果をリセット
				memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
			}

		}

		//スキャン範囲が設定閾値を超えたならかす
		//ｘ座標をみて形状を確認し、適切な閾値と比較する。
		most_depth_x = ABS(vartex_x - down_x);
		//if(most_depth_x > tr->threshold_diagonal_depth_dot * 1.6)	//
		if(most_depth_x > MAX_LIMIT_NUM_TAER_DOT)
		{
			if(!(((nsa.area1_left_up_x < down_x && nsa.area1_right_down_x > down_x) &&
				(nsa.area1_left_up_y > down_y && nsa.area1_right_down_y < down_y))||
				((nsa.area2_left_up_x < down_x && nsa.area2_right_down_x > down_x) &&
				(nsa.area2_left_up_y > down_y && nsa.area2_right_down_y < down_y))))
			{
				tr->res_tear_type[tr->res_tear_count] =not_required;
				tr->res_tear_depth[tr->res_tear_count] = most_depth_x * 0.127f;	//深さ
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_PASS_SCANING;
				return 1;
			}
		}

		if(continuation_flg == 1)
		{
			//ローカルミニマム調査でもスキャン続行にならなかったら
			//最深部とみなして処理を終了する。
			//最深部まで来ても基準以下だったということ
			break;
		}


		if(ABS(down_x) > ABS(origin_x) + 1/* - LIMIT_X*/)	//ｙ座標が頂点のy座標の高さと同じになったら
		{
			//処理を終了する。
			//scan_finish_res_down = RES_SCAN_OUT_OF_AREA_DEPTH;
			scan_finish_res_down = RES_SCAN_OUT_OF_AREA_DEPTH;
			break;
		}

		//紙幣エリア外から出ないようにする
		if(limit_min > down_y)
		{
			//最終地点が最深部かもしれないので記録する
			if(location == right_side)
			{
				if(down_local_minimum_x > down_x)
				{
					down_local_minimum_x = down_x;
					down_local_minimum_y = down_y;
				}
			}
			else if(location == left_side)
			{
				if(down_local_minimum_x < down_x)
				{
					down_local_minimum_x = down_x;
					down_local_minimum_y = down_y;
				}
			}

			//座標がセットされました
			set_cord = 1;

			scan_finish_res_down = RES_SCAN_OUT_OF_AREA_WIDTH;
			break;
		}

		//結果をリセット
		memset( &scan_res[0] , 0x00 , sizeof(scan_res) );
		scan_dir[0] = 1;

	}

	//whileの最大回数を超えた
	if(while_max_of_excution >= MAX_WHILE)
	{
		return ERR_MAX_WHILE_OVER;
	}

	//両方とも探索開始位置がエリア外だった
	if(scan_finish_res_down == RES_SCAN_OUT_OF_AREA_DEPTH && scan_finish_res_up == RES_SCAN_OUT_OF_AREA_DEPTH && set_cord == 0)
	{
		return 0;
	}

	//上側と下側でスキャンして最も深い位置の座標を比較する。
	if(location == right_side)
	{
		//most_depth = ABS(MIN(right_local_minimum ,left_local_minimum));
		if(down_local_minimum_x < up_local_minimum_x)
		{
			most_depth_x = down_local_minimum_x;	//上
			most_depth_y = down_local_minimum_y;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_down;
		}
		else if(down_local_minimum_x > up_local_minimum_x)
		{
			most_depth_x = up_local_minimum_x;	//下
			most_depth_y = up_local_minimum_y;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_up;

		}

		else	//同じ深さならフラグがある方を優先
		{
			if(scan_finish_res_up != 0)
			{
				most_depth_x = up_local_minimum_x;	//上
				most_depth_y = up_local_minimum_y;
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_up;
			}


			else
			{
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_down;
				most_depth_x = down_local_minimum_x;	//下
				most_depth_y = down_local_minimum_y;
			}

		}
	}
	else if(location == left_side)
	{
		if(down_local_minimum_x < up_local_minimum_x)
		{
			most_depth_x = up_local_minimum_x;	//上
			most_depth_y = up_local_minimum_y;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_up;
		}
		else if(down_local_minimum_x > up_local_minimum_x)
		{
			most_depth_x = down_local_minimum_x;	//下
			most_depth_y = down_local_minimum_y;
			tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_down;
		}

		else	//同じ深さ
		{
			if(scan_finish_res_up != 0)
			{
				most_depth_x = up_local_minimum_x;	//上
				most_depth_y = up_local_minimum_y;
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_up;
			}


			else
			{
				tr->res_each_judge_reason[tr->res_tear_count] = scan_finish_res_down;
				most_depth_x = down_local_minimum_x;	//下
				most_depth_y = down_local_minimum_y;
			}

		}
	}

	//スキャン結果を用いて深さを算出
	//most_depth_x = vartex_x - most_depth_x;
	tr->res_tear_depth[tr->res_tear_count] = ABS(vartex_x - most_depth_x) * 0.127f;	//深さ

	//if(not_scan_end_flg == 2)
	//{
	//	tr->res_tear_type[tr->res_tear_count] = horizontal;	//水平
	//	tr->res_each_judge_reason[tr->res_tear_count] = RES_SCAN_NOT_REFERENCE_AREA;
	//	return 0;		//水平
	//}

	//形状を決定する
	if((tr->tear_width_coordinate_start < most_depth_y || tr->tear_width_coordinate_end > most_depth_y) && not_scan_end_flg != 2)
	{
		tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_BELOW_STANDARD;
		tr->res_tear_type[tr->res_tear_count] = diagonal;	//斜め
		return 0;		//斜め
	}
	else
	{
		//水平だった場合基準と比較します。
		tr->res_tear_type[tr->res_tear_count] = horizontal;	//水平

		if(not_scan_end_flg == 2)
		{
			tr->res_each_judge_reason[tr->res_tear_count] = RES_SCAN_NOT_REFERENCE_AREA;
			return 0;		//水平
		}

		if(tr->res_tear_depth[tr->res_tear_count] > tr->threshold_horizontal_depth)
		{
			//スキャン中に紙幣エリア外に出たか否か
			if(tr->res_each_judge_reason[tr->res_tear_count] != RES_SCAN_OUT_OF_AREA_DEPTH)
			{
				//普通の裂け
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_AND_WIDTH_PASS;
				return 1;	//基準以上
			}
			else
			{
				//角裂けチェック
				matching_tear_res_and_dog_ear_res(most_depth_x, most_depth_y, tr);

				if(tr->res_each_judge_reason[tr->res_tear_count] == RES_DEPTH_AND_WIDTH_PASS)
				{
					return 1;	//基準以上の角裂け
				}
				else if(tr->res_each_judge_reason[tr->res_tear_count] == RES_FALSE_DETECT_OF_DOG_EAR)
				{
					return 0;	//角折れを誤ってスキャンした
				}

			}

		}
		else
		{
			if(tr->res_each_judge_reason[tr->res_tear_count] == 0 || tr->res_each_judge_reason[tr->res_tear_count] == RES_SCAN_OUT_OF_AREA_DEPTH)
			{
				//普通の小さな裂け
				tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_BELOW_STANDARD;
			}
			else
			{
				//角裂けチェック
				matching_tear_res_and_dog_ear_res(most_depth_x, most_depth_y, tr);

				//小さな角裂けもしくは折れ
				tr->res_each_judge_reason[tr->res_tear_count] = RES_OUT_OF_AREA_DEPTH_BELOW_STANDARD;
			}
			return 0;	//基準以下
		}
	}

	return 0;	//基準以下
}

//現在の画素の周囲をスキャンする関数(左辺と右辺用
//dirのフラグに従ってスキャンする方向を限定する。
//resのフラグでどこに紙幣があってどこが背景かを確認できる
//上記の配列の対応は以下の通り、
//左辺の場合　0：右　1:右下　2：右上　3：下　4：上　5：左下　6：左上　7：左
//右辺の場合　0：左　1:左下　2：左上　3：下　4：上　5：右下　6：右上　7：右
//
//[in]
//	spt				Lgetdt用構造体
//	dir				スキャンする方向を決定する配列。
//　pbs				サンプリングデータ構造体
//	pnote_param		座標変換パラメタ構造体
//	origin_x,y		スキャン開始座標
//	inc_num			インクリメント係数
//
//[out]
//	res				スキャンした結果を格納する配列　0：紙幣無　１：紙幣有
//
void depth_scan_module_left_and_right(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 origin_x ,s16 origin_y, u8 *res ,s8 inc_num)
{
	//左辺の場合　0：右　1:右下　2：右上　3：下　4：上　5：左下　6：左上　7：左
	//右辺の場合　0：左　1:左下　2：左上　3：下　4：上　5：右下　6：右上　7：右

	//※コメント分は左辺の場合で解釈してください。
	//右
	if(dir[0] == 1)
	{
		spt.x = origin_x + inc_num;
		spt.y = origin_y;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

		if(TR_SCAN_THR < spt.sensdt)
		{
			res[0] = 1;
		}
	}

	//右下
	if(dir[1] == 1)
	{
		spt.x = origin_x + inc_num;
		spt.y = origin_y - 1;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[1] = 1;
		}
	}

	//右上
	if(dir[2] == 1)
	{
		spt.x = origin_x + inc_num;
		spt.y = origin_y + 1;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[2] = 1;
		}
	}

	//下
	if(dir[3] == 1)
	{
		spt.x = origin_x;
		spt.y = origin_y - 1;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[3] = 1;
		}
	}

	//上
	if(dir[4] == 1)
	{
		spt.x = origin_x;
		spt.y = origin_y + 1;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[4] = 1;
		}
	}


	//左下
	if(dir[5] == 1)
	{
		spt.x = origin_x - inc_num;
		spt.y = origin_y - 1;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[5] = 1;
		}
	}

	//左上
	if(dir[6] == 1)
	{
		spt.x = origin_x - inc_num;
		spt.y = origin_y + 1;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[6] = 1;
		}
	}

	//左
	if(dir[7] == 1)
	{
		spt.x = origin_x - inc_num;
		spt.y = origin_y;
		new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取
		if(TR_SCAN_THR < spt.sensdt)
		{
			res[7] = 1;
		}
	}
}

//裂けの幅を検知する(左辺と右辺用
//検知した裂けの幅と始まりと終わりの座標を出力する
//
//[in]
//	spt				Lgetdt用構造体
//	pver　			頂点座標構造体
//　pbs				サンプリングデータ構造体
//	pnote_param		座標変換パラメタ構造体
//	origin_x,y		スキャン開始座標
//	limit_min,max	スキャンのリミット
//
//[out]
//	tear_width_coordinate_start,end	裂け初めと終わりの座標　start＞end
//	tr->res_tear_width				裂けの幅　　単位は㎜
//
//[return]
//	検知結果　１：幅が閾値を超えた　０：超えてない
//
u8 search_tear_width_left_and_right(ST_SPOINT spt ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 origin_x ,s16 origin_y , s16 limit_min ,s16 limit_max, s8 increment ,u8* corner_flg , ST_TEAR *tr)
{
	s16 x = 0;
	s16 y = 0;

	s16 y_upper = limit_max;
	s16 y_lower = limit_min;

	u8 i = 0;
	s16 size = (s16)(tr->note_size_y * 0.125f * 0.127f);
	u32 while_max_of_excution = 0;
	//float	detect_size = 0;
	//s16 temp_start = 0;
	//s16 temp_end = 0;

	tr->res_tear_width[tr->res_tear_count] = 0;
	tr->tear_width_coordinate_start = 0;
	tr->tear_width_coordinate_end = 0;

	while(i != 3)
	{
		while_max_of_excution++;
		if (TEAR_MAX_WHILE_COUNT_WIDTH < while_max_of_excution)
		{
			return 0;	//幅が閾値以下
		}

		x = origin_x + (i * 1 * increment);


		for(y = origin_y; y < limit_max; y = y + 2)
		{
			spt.y = y;
			spt.x = x;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt)
			{
				y_upper = y;

				break;
			}

		}



		for(y = origin_y; y > limit_min; y = y - 2)
		{

			spt.y = y;
			spt.x = x;
			new_L_Getdt(&spt ,pbs ,pnote_param);	//画素採取

			//媒体にぶつかるまで探索します。
			if(TR_SCAN_THR > spt.sensdt)
			{
				y_lower = y;
				break;
			}

		}

		//temp_start = tr->tear_width_coordinate_start;
		//temp_end = tr->tear_width_coordinate_end;

		tr->tear_width_coordinate_start = y_upper;
		tr->tear_width_coordinate_end = y_lower;

		//前回のを記録する
		//detect_size = tr->res_tear_width[tr->res_tear_count];

		//幅を求める
		tr->res_tear_width[tr->res_tear_count] = ABS(y_upper - y_lower) * 0.127f;

		//紙幣サイズ1/5以上の裂けは、誤検知の可能性もあるので位置を変えてもう一度、
		if(size > tr->res_tear_width[tr->res_tear_count])
		{
			break;
		}

		i = i + 1;


	}

	////２回目以降の検知サイズが０の場合、前回のもを使う
	//if(tr->res_tear_width[tr->res_tear_count] == 0 && i != 0)
	//{
	//	//tr->tear_width_coordinate_start = temp_start;
	//	//tr->tear_width_coordinate_end = temp_end;
	//	//tr->res_tear_width[tr->res_tear_count] = detect_size;

	//}

	
	//スキャン中の裂けが角裂けか通常裂けかを分類する。
	//幅のどちらかの座標がリミット(券端)と同じ座標なら角裂けとみなす。
	if(y_upper == limit_max || y_lower == limit_min)
	{
		*corner_flg = 1;
	}

	if(tr->res_tear_width[tr->res_tear_count] > tr->threshold_width && tr->width_detection == 1)
	{
		return 1;	//あり
	}
	else
	{
		return 0;	//なし
	}

}

//裂け検知関数のパラメタをリセットします。
//入出力なし
void ini_tear_param(ST_TEAR *tr)
{
	//中間情報のリセット

	//memset(&tr->corner_triangle_vertex_1[0][0], 0x00 , sizeof(tr->corner_triangle_vertex_1));
	//memset(&tr->corner_triangle_vertex_2[0][0], 0x00 , sizeof(tr->corner_triangle_vertex_2));
	tr->threshold_width_dot = 0;
	tr->threshold_diagonal_depth_dot = 0;
	tr->threshold_horizontal_depth_dot = 0;
	tr->threshold_vertical_depth_dot = 0;
	tr->search_pitch = 0;
	tr->tear_width_coordinate_start = 0;
	tr->tear_width_coordinate_end = 0;
	tr->res_tear_depth_total = 0;
	tr->res_err_code = 0;								//エラーコード
	tr->level = 0;



	memset(&tr->edge_coordinates_detected[0], 0x00 , sizeof(tr->edge_coordinates_detected));

	//結果をリセット
	memset( &tr->res_tear_depth[0] , 0x00 , sizeof(tr->res_tear_depth) );
	memset( &tr->res_tear_width[0] , 0x00 , sizeof(tr->res_tear_width) );
	memset( &tr->res_tear_type[0] ,0x00 ,sizeof(tr->res_tear_type));
	memset( &tr->res_each_judge_reason[0] ,0x00 ,sizeof(tr->res_each_judge_reason));
	memset( &tr->res_corner_tear_type[0] ,0x00 ,sizeof(tr->res_corner_tear_type));
	tr->res_tear_count = 0;
	tr->res_judge_tear = 0;						//検知した裂けの中で閾値を超えるものがあった。
	tr->res_judge_reason = 0;					//検知した裂けの理由
	tr->penetration = 0;
}

//裂けの深さの累計を計算する。
//中国のみ
u8 tear_total_depth(ST_TEAR *tr)
{
	u32 i = 0;
	u32 ii = 0;
	float total_depth = 0;

	//角裂けを重複して計算しないようにする。
	for(ii = 0; ii < tr->res_tear_count; ++ii)
	{
		for(i = 0; i < tr->res_tear_count; ++i)
		{
			if(ii != i)
			{
				if(tr->res_corner_tear_type[ii] == tr->res_corner_tear_type[i] && tr->res_corner_tear_type[ii] != 0)
				{
					if(tr->res_tear_depth[ii] < tr->res_tear_depth[i])
					{
						tr->res_tear_depth[ii] = 0;
					}
					else
					{
						tr->res_tear_depth[i] = 0;
					}

				}
			}
		}
	}

	//閾値以上の裂けの合計値を求める
	for(i = 0; i < tr->res_tear_count; ++i)
	{
		//if(tr->res_tear_depth[i] > tr->threshold_vertical_depth)
		//{
		total_depth = total_depth + tr->res_tear_depth[i];
		//}
	}

	tr->res_tear_depth_total = total_depth;

	//合計値の裂けが閾値以上か判定する
	if(total_depth > tr->threshold_diagonal_depth)
	{
		tr->res_judge_reason = RES_TOTAL_DEPTH;
		return 1;
	}
	else
	{
		return 0;
	}

}

//検知した角裂けを角折れ検知の結果と照合して
//裂けかどうかを調べる
//	[in]
//		x,y　最深部の座標
//
void matching_tear_res_and_dog_ear_res(s16 x, s16 y , ST_TEAR *tr)
{

	if( 0 > x && 0 < y)	//左上
	{
		tr->res_corner_tear_type[tr->res_tear_count] = upper_left_tr;

		if(tr->dog_ear_res[0] == 3)
		{
			tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_AND_WIDTH_PASS;
			return ;
		}
	}
	else if( 0 < x && 0 < y)	//右上
	{
		tr->res_corner_tear_type[tr->res_tear_count] = upper_right_tr;

		if(tr->dog_ear_res[2] == 3)
		{
			tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_AND_WIDTH_PASS;
			return ;
		}

	}
	else if( 0 > x && 0 > y)	//左下
	{
		tr->res_corner_tear_type[tr->res_tear_count] = lower_left_tr;

		if(tr->dog_ear_res[1] == 3)
		{
			tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_AND_WIDTH_PASS;
			return ;
		}

	}
	else if( 0 < x && 0 > y)	//右下
	{
		tr->res_corner_tear_type[tr->res_tear_count] = lower_right_tr;

		if(tr->dog_ear_res[3] == 3)
		{
			tr->res_each_judge_reason[tr->res_tear_count] = RES_DEPTH_AND_WIDTH_PASS;
			return ;
		}

	}


	tr->res_each_judge_reason[tr->res_tear_count] = RES_FALSE_DETECT_OF_DOG_EAR;
	return ;
}

//dpiの違いに応じて、異なる係数を取得する。+
//1.幅検知の際の検知座標特定に用いる係数を決定する。
//2.輪郭スキャンの際に用いる、一時的な深さの閾値を得る
//　深さの閾値に応じて値を動的に決定する。
//　小さくしないと小さな裂けを検知できないが、
//　大きくしないと辺が傾いていて、輪郭線に沿っていない時誤検知になる。
//3.輪郭スキャンの間隔を決定する。
//  処理時間的に問題ないので、どんな幅でも０．５㎜間隔でスキャンする
//  今よりの処理時間短縮が求められたときに復活させればよい 2019年6月17日
//  処理に用いるプレーンは50dpiを最低条件とする。
void get_cof(ST_BS *pbs, ST_TEAR *tr)
{
	u8 val = pbs->Blocksize / pbs->PlaneInfo[tr->plane].sub_sampling_pitch;

	//スキャン間隔　0.5mm固定
	tr->search_pitch = 4;

	tr->temporary_depth_thr			= 6;	//仮裂け検知の閾値
	tr->width_search_num_x			= 1;	//幅検知の際に下げる量
	tr->temporary_depth_thr_diff_x	= 1;	//前座標との差分

	if(val == 1)		//200dpi
	{
		//tr->temporary_depth_thr		= 6;	//仮裂け検知の閾値
		tr->width_search_num_y			= 1;	//幅検知の際に下げる量
		tr->temporary_depth_thr_diff_y	= 1;	//前座標との差分
		//tr->y_scan_limit_margin	= 2 - 1;	//深さスキャンの際の高さの閾値のマージン
	}

	else if(val == 4)	//100dpi
	{
		//tr->temporary_depth_thr		= 6;
		tr->width_search_num_y			= 2;
		tr->temporary_depth_thr_diff_y	= 2;	//前座標との差分
		//tr->y_scan_limit_margin	= 2 - 1;
	}

	else if(val == 2)	//50dpi
	{
		//tr->temporary_depth_thr		= 6;
		tr->width_search_num_y			= 4;
		tr->temporary_depth_thr_diff_y	= 4;	//前座標との差分
		//tr->y_scan_limit_margin	= 4 - 1;
	}

}

//裂けのレベルを計算する。
//斜めの裂けは途中でスキャンを終了するので、60レベル以上のレベルは出力されない　2020/01/31　Furuta
u8	tear_level_detect(ST_TEAR *tr)
{
	float detect_res_ary[2];
	float thr_ary[2];
	u8	level_ary[20] = {0};
	u8	level = 100;
	u8	tmp_tear_num;

	u8  ele_num = 1;

	memset(level_ary, 101, 20);

	//貫通券の場合はレベル１
	if (tr->penetration != 0)
	{
		tr->res_tear_type[0] = PENETRATION;
		tr->res_tear_count=1;								//検知した裂けの数
		tr->res_judge_tear=1;								//検知した裂けの中で閾値を超えるものがあった。　1：有　0：無
		tr->res_judge_reason= RES_TEAR_PENETRATION;			//検知した裂けの理由　
		return 1;
	}


	if(tr->total_depth_judge == 0)
	{
		if(tr->width_detection)
		{
			ele_num = 2;
		}

		for(tmp_tear_num = 0; tmp_tear_num < tr->res_tear_count; ++tmp_tear_num)
		{
			//検知結果読み込み（1箇所
			detect_res_ary[1] = tr->res_tear_width[tmp_tear_num];
			detect_res_ary[0] = tr->res_tear_depth[tmp_tear_num];

			switch(tr->res_tear_type[tmp_tear_num])
			{
			case vertical:
				thr_ary[1] = tr->threshold_width;
				thr_ary[0] = tr->threshold_vertical_depth;

				//レベル計算
				level_ary[tmp_tear_num] = level_detect(detect_res_ary , thr_ary ,ele_num ,MIN_LIMIT_NUM_TAER ,MAX_LIMIT_NUM_TAER);
				break;

			case horizontal:
				thr_ary[1] = tr->threshold_width;
				thr_ary[0] = tr->threshold_horizontal_depth;

				//レベル計算
				level_ary[tmp_tear_num] = level_detect(detect_res_ary , thr_ary ,ele_num ,MIN_LIMIT_NUM_TAER ,MAX_LIMIT_NUM_TAER);
				break;


			case diagonal:
			case not_required:
				thr_ary[1] = tr->threshold_width;
				thr_ary[0] = tr->threshold_diagonal_depth;

				//レベル計算
				level_ary[tmp_tear_num] = level_detect(detect_res_ary , thr_ary ,ele_num ,MIN_LIMIT_NUM_TAER ,MAX_LIMIT_NUM_TAER);
				break;

			default:
				break;
			}
		}

		//if(tr->total_depth_judge)
		//{
		//	//detect_res_ary[0] = tr->res_total_depth;
		//	//thr_ary[0] = tr->threshold_total_depth;
		//
		//	//レベル計算
		//	level_ary[tmp_tear_num + 1] = level_detect(detect_res_ary , thr_ary ,1);	
		//	tr->res_tear_count++;
		//}

		for(tmp_tear_num = 0; tmp_tear_num < tr->res_tear_count; ++tmp_tear_num)
		{
			//最小値計算
			if(level > level_ary[tmp_tear_num])
			{
				level = level_ary[tmp_tear_num];
			}
		}
	}
	else
	{
		detect_res_ary[0] = tr->res_tear_depth_total;
		thr_ary[0] = tr->threshold_diagonal_depth;

		//レベル計算
		level_ary[0] = level_detect(detect_res_ary , thr_ary ,1 ,MIN_LIMIT_NUM_TAER ,MAX_LIMIT_NUM_TAER);

		level = level_ary[0];
	}

	return level;

}

//　END
