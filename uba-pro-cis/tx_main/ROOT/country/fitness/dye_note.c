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
* @file dye_note.c
* @brief ダイノート検知
* @date 2018/12/28 Created.
* @update　
* https://app.box.com/s/npf92crwxfbrkr4ww7160oqd9pa5pdbv
	2020/1/27:	面積計算関数areaを修正 そのほか型を修正
	2020/1/30:	暗明暗のパターンに対応した	
	2022/12/20:	１辺がすべてインクの場合に座標を記録しないように変更（誤った座標を記録していた）	
*/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//#define VS_DEBUG1

//再起呼び出しで使うようの変数
//float gcal_val;
int gcal_val;

//#define PIX_DIFF_THR		200000		//外部と内部の差分
//#define PIX_DIFF_THR		130000		//外部と内部の差分 R
#define PIX_DIFF_THR		2			//外部と内部の差分

//#define PIX_DIFF_THR		160000		//外部と内部の差分 G
//#define PIX_DIFF_THR		10			//外部と内部の差分 G


//#define dy->thr			50000		//外部の画素値の閾値
//#define dy->thr			2.5		//外部の画素値の閾値 R
//#define dy->thr			150000		//外部の画素値の閾値 R
//#define dy->thr			200000		//外部の画素値の閾値 G
//#define dy->thr			2.5		//外部の画素値の閾値 G
//#define dy->thr			25000		//外部の画素値の閾値
//#define MIN_PIX_MARGIN		30000		//外部の画素値に与えるマージン

#define NORMALIZ_VAL		0.00000384467f	//1 / (510 * 510)
//#define NORMALIZ_VAL		0.00000000753f	//1 / (510 * 510 *510)
//#define NORMALIZ_VAL		0.00000000038f	//1 / (510 * 510 * 10000)
//#define NORMALIZ_VAL		0.000000000153787f

#define AVERAGE_NUM			0.111111112f


//スキャンステップ
//#define dy->scan_pitch	64		//スキャン幅
//#define scan_pitch_y	64		//
//#define dy->note_corner_margin		12		//紙幣１㎜から
#define MASK_SIZE	3		//

//機能用
#define ADD_DETECT_TRANSMISSION //有効にすることで透過による裂けや角折れとインクを区別する。	
//#define ADD_NOT_DETECT_SMALL	//有効にすることで小さなエリアは合計面積に含めない　　


//debug用
#ifdef VS_DEBUG_2
int debug_ii = 0;
int debug_iii = 0;
char fname[10];
int debug_val_ary[100];
FILE *pfdebug;
#endif

//ビューワ用
//#define DYE_NOTE_SWAP_PLANE 1	//change processing order from Red to Green -> from Green to Red.
//#define DYE_NOTE_VIEWER_MODE 1	//

#define EXT
#include "../common/global.h"
//#include "global.h"

#ifdef VS_DEBUG
EXTERN int debug_logi_view;	//トレースするかしないか
#endif

//************************************************************
//メイン関数
s16 dye_note(u8 buf_num ,ST_DYE_NOTE *dy) //テンプレートシェル対応のために専用構造体を引数に追加した　19/11/7
{

	//u32 res = 0;

	//処理中の紙幣情報
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;

	u8 level_newink = 100;

	s32 center_x = 0;
	s32 center_y = 0;
	s16 i = 0;

	u32 temp_area = 0;
	float temp_ratio = 0;

#if DYE_NOTE_SWAP_PLANE
	u8 debug_thr = 0;
#endif

#ifdef VS_DEBUG_2
	//debug_iii++;
	//strcpy(fname, (char *)debug_iii);
	//strcat(fname, ".csv");
	F_CREATE_1(pfdebug);
	//pfdebug = fopen(fname , "w");
	debug_ii = 0;
#endif

	//テンプレートシェル対応のために関数外に出しました。　2019/11/07
	//dy->note_size_x = pbs->note_x_size;
	//dy->note_size_y = pbs->note_y_size;

	//SEとLEでサイズの入れ替え
	if(pbs->LEorSE == SE)
	{
		SWAP(dy->note_size_x,dy->note_size_y);
	}

	//紙幣内部の画素値をあらかじめ計算する。
	//cal_hora_and_verti_val();

	//ワーク画像の大きさ
	//dy->work_image_size_x = dy->note_size_x / dy->work_image_dpi;
	//dy->work_image_size_y = dy->note_size_y / dy->work_image_dpi;

	//傾き左上から右下
	//dy->slope_leftup_rightdown = (0.0f - dy->work_image_size_y) / (0.0f - dy->work_image_size_x);


	//傾き右上から左下
	//dy->slope_leftdown_rightup = -dy->slope_leftup_rightdown;
	//dy->slope_leftdown_rightup = (dy->work_image_size_y - 0.0f) / (0.0f - dy->work_image_size_x);

	dy->rej_code = 0;

	//面積計算
	dy->note_area = (u32)(dy->note_size_x   * dy->note_size_y);

	//基準面積 単位ドット
	dy->area_ratio = (float)(dy->note_area * (dy->ratio / 100.0f));// / (dy->work_image_dpi * dy->work_image_dpi);

	//マージン計算
	dy->note_corner_margin = dy->note_corner_margin + 4;


	//特異点排除フラグの設定
	if (dy->scan_pitch <= 24)
	{
		dy->singularity_exclusion_flg = 1;
	}
	else
	{
		dy->singularity_exclusion_flg = 0;
	}

	//紙幣の分割数
	//dy->x_split = dy->note_size_x / dy->scan_pitch + 2;
	//dy->y_split = dy->note_size_y / dy->scan_pitch + 2;

	//平均画像を作成する
	//ave_img_create(dy->buf_num , dy->work_image_dpi, dy->plane1 ,dy->plane2 ,dy->plane_num);

	//インクの探索
	//res = search_ink(pbs, pnote_param);

	//インク探索結果
	//if(res == 0)
	//{
	//	dy->note_area = 0.0f;
	//	return 0;	//0ならば終了
	//}


	//面積を求めに行きます
	//res = search_area(pbs, pnote_param);

	//色数
	dy->plane_num = 2;

	//初期化
	status_ini(dy);

	if (dy->thr != 0)
	{

		//用いるプレーン
#if DYE_NOTE_SWAP_PLANE
		dy->plane1 = OMOTE_R_G;
		dy->plane2 = URA_R_G;
		debug_thr = (u8)dy->thr;
		dy->thr = dy->thr2;
#else
		dy->plane1 = OMOTE_R_R;
		dy->plane2 = URA_R_R;
#endif
		//dy->thr = dy->thr2;

		//面積計算phase2
		//上
		search_ink_2_up(pbs, pnote_param, dy);//1

		//右
		search_ink_2_right(pbs, pnote_param, dy);//2

		//下
		search_ink_2_down(pbs, pnote_param, dy);

		//左
		search_ink_2_left(pbs, pnote_param, dy);

		//インクを1箇所でも検知した場合
		if (dy->record_ink_count != 0)
		{
			for (i = 0; i < dy->record_count; ++i)
			{
				center_x = center_x + dy->record_x[i];
				center_y = center_y + dy->record_y[i];
			}

			center_x = center_x / dy->record_count;
			center_y = center_y / dy->record_count;


			dy->res_ink_area = (u32)area(dy->record_x, dy->record_y, dy->record_count, (s16)center_x, (s16)center_y);	//面積計算
			if (dy->note_area < dy->res_ink_area)
			{
				dy->res_ink_area = 0;
			}

			dy->res_ink_area = (dy->note_area - dy->res_ink_area);

			//データ整理
			//面積　デバッグ用
			//dy->res_ink_area = dy->res_ink_area * (dy->work_image_dpi * dy->work_image_dpi);	//ndpiから200dpiに変換
			dy->res_ink_ratio = ((float)dy->res_ink_area * 100.0f) / (float)dy->note_area;						//面積割合（％）を計算　
		}

		dy->level = dye_level_detect(dy);
		level_newink = dy->level;
		//if (dy->res_ink_area > dy->area_ratio)	//面積(単位dot)で比較する
		if (pbs->fitness[DYE_NOTE_].bit.threshold_2 >= level_newink)
		{
#ifdef VS_DEBUG_2
			F_CLOSE(pfdebug);
#endif
			dy->res_ink_area = (u32)(dy->res_ink_area * 0.127f * 0.127f);
			dy->judge = 1;
			dy->rej_code = REJ_NEW_DYE_INK;
			//REJ_DYE_NOTE

			return 1;	//あり
		}
		else
		{
			dy->res_ink_area = (u32)(dy->res_ink_area * 0.127f * 0.127f);
		}

		temp_area = dy->res_ink_area;
		temp_ratio = dy->res_ink_ratio;
		dy->level = 100;


#ifdef VS_DEBUG_2
		F_CLOSE(pfdebug);
#endif
	}
	//return 0;

	//旧インクを求めます
	if (dy->thr2 != 0)
	{
		//初期化
		status_ini(dy);

		dy->thr = dy->thr2;

		//用いるプレーン
#if DYE_NOTE_SWAP_PLANE
		dy->plane1 = OMOTE_R_R;
		dy->plane2 = URA_R_R;
		dy->thr = debug_thr;
#else
		dy->plane1 = OMOTE_R_G;
		dy->plane2 = URA_R_G;
#endif

		//面積計算phase2
		//上
		search_ink_2_up(pbs, pnote_param, dy);

		//右
		search_ink_2_right(pbs, pnote_param, dy);

		//下
		search_ink_2_down(pbs, pnote_param, dy);

		//左
		search_ink_2_left(pbs, pnote_param, dy);

		//インクを1箇所でも検知した場合
		if (dy->record_ink_count != 0)
		{
			for (i = 0; i < dy->record_count; ++i)
			{
				center_x = center_x + dy->record_x[i];
				center_y = center_y + dy->record_y[i];
			}

			center_x = center_x / dy->record_count;
			center_y = center_y / dy->record_count;


			dy->res_ink_area = (u32)area(dy->record_x, dy->record_y, dy->record_count, (s16)center_x, (s16)center_y);	//面積計算
			if (dy->note_area < dy->res_ink_area)
			{
				dy->res_ink_area = 0;
			}

			dy->res_ink_area = (dy->note_area - dy->res_ink_area);

			//データ整理
			//面積　デバッグ用
			//dy->res_ink_area = dy->res_ink_area * (dy->work_image_dpi * dy->work_image_dpi);	//ndpiから200dpiに変換
			dy->res_ink_ratio = ((float)dy->res_ink_area * 100.0f) / (float)dy->note_area;						//面積割合（％）を計算　
		}


		dy->level = dye_level_detect(dy);

		//if (dy->res_ink_area > dy->area_ratio)	//面積(単位dot)で比較する
		if (pbs->fitness[DYE_NOTE_].bit.threshold_2 >= dy->level)
		{
#ifdef VS_DEBUG_2
			F_CLOSE(pfdebug);
#endif
			dy->res_ink_area = (u32)(dy->res_ink_area * 0.127f * 0.127f);
			dy->judge = 2;
			dy->rej_code = REJ_OLD_DYE_INK;

			//REJ_DYE_NOTE

			return 2;	//あり
		}
		else
		{
			dy->res_ink_area = (u32)(dy->res_ink_area * 0.127f * 0.127f);
		}

#ifdef VS_DEBUG_2
		F_CLOSE(pfdebug);
#endif

		if (level_newink < dy->level)
		{
			dy->level = level_newink;
			dy->res_ink_area = temp_area;
			dy->res_ink_ratio = temp_ratio;
		}

		return 0;	//なし
	}

	return 0;	//なし

}//end


//************************************************************
//phase2の関数
//紙幣の枠から広幅でスキャンを開始して、前画素との差が生じたら細かく分析する。
//return 0　インクなし
//		１　あり
//out：dy->side1 と　side2
u32 search_ink_2_up(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param  ,ST_DYE_NOTE *dy)
{

	//券端情報
	s16 start_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 end_x	=  (dy->note_size_x >> 1) - dy->note_corner_margin;
	s16 start_y =  (dy->note_size_y >> 1) - dy->note_corner_margin;
	s16 end_y   = -(dy->note_size_y >> 1) + dy->note_corner_margin;

	u8  scan_location_flg[128] = { 0 };

	s16 half_note_size_y = dy->note_size_y >> 1;

	s16 x = 0;
	s16 y = 0;

	s16 noise_out_x;
	s16 noise_out_y;

		u8 current_point_num = 0;
	u8 current_record_num = 0;
	//s16 rounding =  start_y + dy->note_corner_margin;
	u16 start_offset = 0;

	//u8 record_count = 1;

#ifdef VS_DEBUG
	s16 tmpx;
	s16 tmpy;
#endif

	float temp_pix = 0;
	float privi_pix = 0;
	float current_pix = 0;
	float diff_pix = 0;

	u8 i = 0;

	ST_SPOINT spt1;
	ST_SPOINT spt2;

	spt1.l_plane_tbl = dy->plane1;
	spt2.l_plane_tbl = dy->plane2;

	spt1.way = pbs->insertion_direction;
	spt2.way = pbs->insertion_direction;

	//紙幣をスキャンし差分が大きい場合インク有と判断する
	for(x = start_x; x < end_x; x = x + dy->scan_pitch)
	{
		y = 0;
		current_pix = 0;
		privi_pix = 0;
		current_point_num++;


#ifdef ADD_DETECT_TRANSMISSION
		//透過イメージを用いて裂け、角折れなどをインクと区別する。
		spt1.x = x;
		spt1.y = start_y;

		spt1.l_plane_tbl = OMOTE_T_G;
		if (pbs->PlaneInfo[spt1.l_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
		{
			spt1.l_plane_tbl = URA_T_G;			 //用いるプレーン指定
			//ED.Plane = DOWN_T_R;			 //用いるプレーン指定
		}
		new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
		spt1.l_plane_tbl = dy->plane1;
		if(spt1.sensdt > 245)
		{
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = start_y;
			dy->record_count++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count-1], dy->record_y[dy->record_count-1],0,0,0,0,0,0);
#endif
			continue;
		}
#endif

		//ノイズを除去するフィルター関数を走らせると
		//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
		for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
		{
			for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
			{

				spt1.x = x		 + noise_out_x;
				spt1.y = start_y + noise_out_y;

				spt2.x = x       + noise_out_x;
				spt2.y = start_y + noise_out_y;

				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((s16)(spt1.sensdt*1) + (s16)(spt2.sensdt*1.0));
				//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//privi_pix = temp_pix >> 1;
				//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
				privi_pix = privi_pix + temp_pix;
				//privi_pix = cal_val * 0.0000153787 * 255;	//正規化

				spt1.x = x + noise_out_x;
				spt1.y = y + noise_out_y;

				spt2.x = x + noise_out_x;
				spt2.y = y + noise_out_y;

				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((s16)(spt1.sensdt*1.0) + (s16)(spt2.sensdt*1.0));
				//temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
				//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//privi_pix = temp_pix >> 1;
				//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
				current_pix = current_pix + temp_pix;
				//temp_pix = cal_val * 0.0000153787 * 255;
				//current_pix *= temp_pix;
			}
		}

		privi_pix = privi_pix * NORMALIZ_VAL;
		current_pix = current_pix * NORMALIZ_VAL;

		privi_pix = privi_pix * 255.0f;
		current_pix = current_pix * 255.0f;

		privi_pix = privi_pix *AVERAGE_NUM;
		current_pix = current_pix *AVERAGE_NUM;

		diff_pix = current_pix - privi_pix;

		//privi_pix 外側
		//current_pix　内側


		//min_pix = MIN(current_pix , privi_pix);

#ifdef VS_DEBUG_2
		F_WRITE(pfdebug ,(int)debug_ii++);
		F_WRITE(pfdebug ,(int)privi_pix);
		F_WRITE(pfdebug ,(int)diff_pix);
#endif
		if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
		{

#ifdef VS_DEBUG1
			F_WRITE(pfdebug ,(int)debug_ii++);
			F_WRITE(pfdebug ,(int)privi_pix);
			F_WRITE(pfdebug ,(int)diff_pix);
#endif
			//privi_pix = privi_pix + (s32)(privi_pix * 1.0f);

			//境界線検知
			search_boundary_y(spt1 ,spt2 ,half_note_size_y ,up ,pbs ,pnote_param ,&x ,&y ,dy->thr ,1 ,dy);

			//座標を記録する
			scan_location_flg[current_point_num - 1] = 1;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = y;
			dy->record_ink_count++;
			dy->record_count++;
			current_record_num++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], start_y-10, end_y,1, dynote_side_up, 0, 0);
#endif

		}
		else if(privi_pix > dy->thr)	//差があるが縁が暗くない or 差もなく縁も暗くない
			//デザインだと判断　　　　　ついていない
		{


			//券端座標を記録
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = start_y + dy->note_corner_margin;

			dy->record_count++;
			//current_record_num++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], start_y, end_y, 1, dynote_side_up, 0, 0);
#endif

		}

		else if(privi_pix < dy->thr)	//差はないが暗い場合　紙幣の半分以上にインクの境界線がある場合の対応
		{
			//上半分で行ったことをした半分でも行う。
			//privi_pix = current_pix;
			current_pix = 0;
			y = -start_y / 2;

			for(i = 0; i < 2; ++i)
			{
				if(i == 1)
				{
					y = end_y;
				}


				for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
				{
					for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
					{

						spt1.x = x		+ noise_out_x;
						spt1.y = y  + noise_out_y;

						spt2.x = x		+ noise_out_x;
						spt2.y = y  + noise_out_y;

						new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
						new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

						temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
						//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
						//cal_val = temp_pix * temp_pix * 10000;
						temp_pix = temp_pix * temp_pix;
						//privi_pix = temp_pix >> 1;
						//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
						current_pix = current_pix + temp_pix;
						//temp_pix = cal_val * 0.0000153787 * 255;
						//current_pix *= temp_pix;
					}
				}
				//privi_pix = privi_pix * NORMALIZ_VAL;
				current_pix = current_pix * NORMALIZ_VAL;

				//privi_pix = privi_pix * 255.0f;
				current_pix = current_pix * 255.0f;

				//privi_pix = privi_pix *AVERAGE_NUM;
				current_pix = current_pix *AVERAGE_NUM;

				diff_pix = current_pix - privi_pix;

#ifdef VS_DEBUG_2
				F_WRITE(pfdebug ,(int)debug_ii);
				F_WRITE(pfdebug ,(int)privi_pix);
				F_WRITE(pfdebug ,(int)diff_pix);
#endif
				if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
				{

#ifdef VS_DEBUG1
					F_WRITE(pfdebug ,(int)debug_ii++);
					F_WRITE(pfdebug ,(int)privi_pix);
					F_WRITE(pfdebug ,(int)diff_pix);
#endif
					//privi_pix = privi_pix + (s32)(privi_pix * 0.1f);
					//境界線検知
					search_boundary_y(spt1 ,spt2 ,half_note_size_y ,up ,pbs ,pnote_param ,&x ,&y ,dy->thr ,1 ,dy);

					if (y > end_y + dy->note_corner_margin + dy->note_corner_margin)	//終点まで探索した場合は記録しない マージンを念のため２回計算している
					{
						scan_location_flg[current_point_num - 1] = 2;
						dy->record_x[dy->record_count] = x;
						dy->record_y[dy->record_count] = y;
						dy->record_ink_count++;
						dy->record_count++;

						current_record_num++;
					}
					i = 2;

#ifdef VS_DEBUG
					debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], start_y, (s32)(end_y * 0.75f), 1, dynote_side_up, 0, 0);
#endif

				}

				else	//差があるが縁が暗くない or 差もなく縁も暗くない
					//デザインだと判断　　　　　ついていない
				{

//#ifdef VS_DEBUG
//					//F_N(pfdebug);
//					if(pbs->LEorSE == LE)
//					{
//						tmpx = x + pnote_param->main_eff_range / 2;	//
//						tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - start_y;	//
//					}
//					else
//					{
//						tmpy = pnote_param->main_eff_range / 2 - start_y;
//						tmpx = x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2;	//
//
//					}
//					deb_para[0] = 4;		// function code
//					deb_para[1] = tmpx;
//					deb_para[2] = tmpy;	//
//					deb_para[3] = 1;		// plane
//					callback(deb_para);		// debug
//
//#endif
					////券端座標を記録
					//dy->record_x[dy->record_count] = x;
					//dy->record_y[dy->record_count] = start_y + dy->note_corner_margin;
					//dy->record_count++;
				}
			}


#ifdef VS_DEBUG_2
		else	//縁が暗いが内部との差がない
		{
			F_N(pfdebug);
		}
#endif
		}

	}
//#ifdef botu
//
//	y = 0;
//	privi_pix = 0;
//	current_pix = 0;
//
//	//ノイズを除去するフィルター関数を走らせると
//	//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
//	for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
//	{
//		for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
//		{
//			spt1.x = end_x  + noise_out_x;
//			spt1.y = start_y+ noise_out_y;
//
//			spt2.x = end_x  + noise_out_x;
//			spt2.y = start_y+ noise_out_y;
//
//			new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//			new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//			temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//			//cal_val = temp_pix * temp_pix  * 100000;
//			//cal_val = temp_pix * temp_pix * 10000;
//			temp_pix = temp_pix * temp_pix;
//			//privi_pix = temp_pix >> 1;
//			//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//			temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//			privi_pix = privi_pix + temp_pix;
//			//temp_pix = cal_val * 0.0000153787 * 255;
//			//current_pix *= temp_pix;
//
//			spt1.x = end_x + noise_out_x;
//			spt1.y = y     + noise_out_y;
//
//			spt2.x = end_x + noise_out_x;
//			spt2.y = y     + noise_out_y;
//
//			new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//			new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//
//			temp_pix = spt1.sensdt + spt2.sensdt;
//			//cal_val = temp_pix * temp_pix  * 100000;
//			//cal_val = temp_pix * temp_pix * 10000;
//			temp_pix = temp_pix * temp_pix;
//			//privi_pix = temp_pix >> 1;
//			//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//			temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//			current_pix = current_pix + temp_pix;
//			//temp_pix = cal_val * 0.0000153787 * 255;
//			//current_pix *= temp_pix;
//		}
//	}
//
//	//current_pix =current_pix * 0.111111111111f;
//	//privi_pix = privi_pix * 0.111111111111f;
//	privi_pix = privi_pix *AVERAGE_NUM;
//	current_pix = current_pix *AVERAGE_NUM;
//	diff_pix = current_pix - privi_pix;
//
//	//min_pix = MIN(current_pix , privi_pix);
//
//#ifdef VS_DEBUG_2
//	F_WRITE(pfdebug ,(int)debug_ii++);
//	F_WRITE(pfdebug ,(int)privi_pix);
//	F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//
//	if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
//	{
//#ifdef VS_DEBUG1
//		F_WRITE(pfdebug ,(int)debug_ii++);
//		F_WRITE(pfdebug ,(int)privi_pix);
//		F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//		//privi_pix = privi_pix + (s32)(diff_pix * 0.1f);
//		//境界線検知
//		search_boundary_y(spt1 ,spt2 ,half_note_size_y ,up ,pbs ,pnote_param ,&end_x ,&y ,dy->thr ,1);
//
//#ifdef VS_DEBUG_2
//		F_N(pfdebug);
//		if(pbs->LEorSE == LE)
//		{
//			tmpx = x + pnote_param->main_eff_range / 2;	//
//			tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - y;	//
//		}
//		else
//		{
//			tmpy = pnote_param->main_eff_range / 2 - y;	//
//			tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//		}
//		deb_para[0] = 4;		// function code
//		deb_para[1] = tmpx;
//		deb_para[2] = tmpy;	//
//		deb_para[3] = 1;		// plane
//		callback(deb_para);		// debug
//
//#endif
//
//		dy->record_x[dy->record_count] = x;
//		dy->record_y[dy->record_count] = y;
//		dy->record_ink_count++;
//		dy->record_count++;
//		current_record_num++;
//
//	}
//	else if(privi_pix > dy->thr)	//差があるが縁が暗くない or 差もなく縁も暗くない
//	{
//
//#ifdef VS_DEBUG_2
//		F_N(pfdebug);
//		if(pbs->LEorSE == LE)
//		{
//			tmpx = x + pnote_param->main_eff_range / 2;	//
//			tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - start_y;	//
//		}
//		else
//		{
//			tmpy = start_y - pnote_param->main_eff_range / 2;	//
//			tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//		}
//		deb_para[0] = 4;		// function code
//		deb_para[1] = tmpx;
//		deb_para[2] = tmpy;	//
//		deb_para[3] = 1;		// plane
//		callback(deb_para);		// debug
//
//#endif
//
//		//券端座標を記録
//		dy->record_x[dy->record_count] = end_x + dy->note_corner_margin;
//		dy->record_y[dy->record_count] = start_y + dy->note_corner_margin;
//
//		dy->record_count++;
//		current_record_num++;
//
//	}
//
//	else if(privi_pix < dy->thr)	//差はないが暗い場合
//	{
//		//上半分で行ったことをした半分でも行う。
//		//privi_pix = current_pix;
//		current_pix = 0;
//		y = end_y;
//		x = end_x;
//
//		for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
//		{
//			for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
//			{
//
//				spt1.x = x	+ noise_out_x;
//				spt1.y = y  + noise_out_y;
//
//				spt2.x = x	+ noise_out_x;
//				spt2.y = y  + noise_out_y;
//
//				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//				temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//				//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
//				//cal_val = temp_pix * temp_pix * 10000;
//				temp_pix = temp_pix * temp_pix;
//				//privi_pix = temp_pix >> 1;
//				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//				current_pix = current_pix + temp_pix;
//				//temp_pix = cal_val * 0.0000153787 * 255;
//				//current_pix *= temp_pix;
//			}
//		}
//		current_pix = current_pix *AVERAGE_NUM;
//		diff_pix = current_pix - privi_pix;
//
//#ifdef VS_DEBUG_2
//		F_WRITE(pfdebug ,(int)debug_ii);
//		F_WRITE(pfdebug ,(int)privi_pix);
//		F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//		if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
//		{
//
//#ifdef VS_DEBUG1
//			F_WRITE(pfdebug ,(int)debug_ii++);
//			F_WRITE(pfdebug ,(int)privi_pix);
//			F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//			//privi_pix = privi_pix + (s32)(diff_pix * 0.1f);
//			//境界線検知
//			search_boundary_y(spt1 ,spt2 ,half_note_size_y ,up ,pbs ,pnote_param ,&x ,&y ,dy->thr ,1);
//
//#ifdef VS_DEBUG_2
//			F_N(pfdebug);
//			if(pbs->LEorSE == LE)
//			{
//				tmpx = x + pnote_param->main_eff_range / 2;	//
//				tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - y;	//
//			}
//			else
//			{
//				tmpy = y - pnote_param->main_eff_range / 2;	//
//				tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//			}
//			deb_para[0] = 4;		// function code
//			deb_para[1] = tmpx;
//			deb_para[2] = tmpy;	//
//			deb_para[3] = 1;		// plane
//			callback(deb_para);		// debug
//
//#endif
//
//			dy->record_x[dy->record_count] = x;
//			dy->record_y[dy->record_count] = y;
//			dy->record_ink_count++;
//			dy->record_count++;
//			current_record_num++;
//
//		}
//
//		else	//差があるが縁が暗くない or 差もなく縁も暗くない
//			//デザインだと判断　　　　　ついていない
//		{
//
//			//券端座標を記録
//			dy->record_x[dy->record_count] = end_x + dy->note_corner_margin;
//			dy->record_y[dy->record_count] = start_y + dy->note_corner_margin;
//
//			dy->record_count++;
//			current_record_num++;
//
//		}
//
//
//#ifdef VS_DEBUG_2
//	else	//縁が暗いが内部との差がない
//	{
//		F_N(pfdebug);
//	}
//#endif
//	}
//
//#ifdef VS_DEBUG_2
//	else	//縁が暗いが内部との差がない
//	{
//		F_N(pfdebug);
//	}
//#endif
//#endif
	//else	//縁が暗いが内部との差がない
	//{
	//continue
	//}
	//１辺ずつ求める用
	////左上の座標を追加
	//dy->record_x[0] = record_x[1];
	//dy->record_y[0] = start_y - dy->note_corner_margin;

	////右上の座標を追加
	//record_x[dy->record_count] = record_x[record_count-1];
	//record_y[dy->record_count] = start_y - dy->note_corner_margin;
	//record_count++;
	////インクを検知したポイントが一定数ならば
	//if(dy->x_split - 5 <= record_count)
	//{
	//	//面積を計算する
	//	return area(record_x , record_y , record_count,  record_x[record_count / 2] ,start_y - dy->note_corner_margin);	//面積計算
	//}

#ifdef ADD_NOT_DETECT_SMALL

	if(current_record_num == 2 ||current_record_num == 1)
	{
		for(i = 0; i < current_point_num; i++)
		{
			dy->record_y[start_offset + i] = rounding;
		}
	}

#endif

	//上辺と下辺のみ、特異点削除を実行する。　（スレッドなどを誤検知しないようにするための処理）
	if (dy->singularity_exclusion_flg == 1)
	{
		singularity_exclusion(dy, scan_location_flg, start_offset, dynote_side_up);
	}


	return 0;
}

//************************************************************
//phase2の関数
//紙幣の枠から広幅でスキャンを開始して、前画素との差が生じたら細かく分析する。
//return 0　インクなし
//		１　あり
//out：dy->side1 と　side2
u32 search_ink_2_down(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param  ,ST_DYE_NOTE *dy)
{
#if DYE_NOTE_VIEWER_MODE
	//券端情報
//s16 start_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 start_x = (dy->note_size_x >> 1) - dy->note_corner_margin;
	s16 end_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 start_y = -(dy->note_size_y >> 1) + dy->note_corner_margin;
	s16 end_y = (dy->note_size_y >> 1) - dy->note_corner_margin;
#else
	//券端情報
	//s16 start_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 start_x =  dy->record_x[dy->record_count - 1] - dy->note_corner_margin;
	s16 end_x	=  -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 start_y =  -(dy->note_size_y >> 1) + dy->note_corner_margin;
	s16 end_y =  (dy->note_size_y >> 1) - dy->note_corner_margin;
#endif

	s16 half_note_size_y = dy->note_size_y >> 1;

	s16 x = 0;
	s16 y = 0;

	u8 i = 0;

	u8 scan_location_flg[128] = { 0 };


	s16 noise_out_x;
	s16 noise_out_y;
#ifdef VS_DEBUG
	s16 tmpx;
	s16 tmpy;
#endif

	//float cal_val = 0.0f;
	//float temp_pix = 0.0f;

	//float privi_pix = 0.0f;
	//float current_pix = 0.0f;
	float temp_pix = 0;
	float privi_pix = 0;
	float current_pix = 0;
	float diff_pix = 0;

	u8 current_point_num = 0;
	u8 current_record_num = 0;
	//s16 rounding =  start_y - dy->note_corner_margin;
	u16 start_offset = dy->record_count;	//開始時の検知数

	ST_SPOINT spt1;
	ST_SPOINT spt2;

	//u8 thr = 0;

	spt1.l_plane_tbl = dy->plane1;
	spt2.l_plane_tbl = dy->plane2;

	spt1.way = pbs->insertion_direction;
	spt2.way = pbs->insertion_direction;

	//紙幣をスキャンし差分が大きい場合インク有と判断する
	for(x = start_x; x > end_x; x = x - dy->scan_pitch)
	{
		y = 0;
		current_pix = 0;
		privi_pix = 0;

		
		current_point_num++;


#ifdef ADD_DETECT_TRANSMISSION
		//透過イメージを用いて裂け、角折れなどをインクと区別する。
		spt1.x = x;
		spt1.y = start_y;

		spt1.l_plane_tbl = OMOTE_T_G;
		if (pbs->PlaneInfo[spt1.l_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
		{
			spt1.l_plane_tbl = URA_T_G;			 //用いるプレーン指定
			//ED.Plane = DOWN_T_R;			 //用いるプレーン指定
		}
		new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
		spt1.l_plane_tbl = dy->plane1;
		if(spt1.sensdt > 250)
		{
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = start_y;
			dy->record_count++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, (s32)dy->record_x[dy->record_count - 1], (s32)dy->record_y[dy->record_count - 1], 0, 0, 0, dynote_side_down, (s32)dy->record_x[start_offset - 1], (u32)&dy->record_count);
#endif

			continue;
		}
#endif

		//ノイズを除去するフィルター関数を走らせると
		//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
		for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
		{
			for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
			{
				spt1.x = x		 + noise_out_x;
				spt1.y = start_y + noise_out_y;

				spt2.x = x		 + noise_out_x;
				spt2.y = start_y + noise_out_y;

				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
				//cal_val = temp_pix * temp_pix  * 100000;
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//privi_pix = temp_pix >> 1;
				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
				//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
				privi_pix = privi_pix + temp_pix;
				//privi_pix = cal_val * 0.0000153787 * 255;	//正規化

				spt1.x = x + noise_out_x;
				spt1.y = y + noise_out_y;

				spt2.x = x + noise_out_x;
				spt2.y = y + noise_out_y;


				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
				//cal_val = temp_pix * temp_pix  * 100000;
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//privi_pix = temp_pix >> 1;
				//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
				current_pix = current_pix + temp_pix;
				//temp_pix = cal_val * 0.0000153787 * 255;
				//current_pix *= temp_pix;
			}
		}

		privi_pix = privi_pix * NORMALIZ_VAL;
		current_pix = current_pix * NORMALIZ_VAL;

		privi_pix = privi_pix * 255.0f;
		current_pix = current_pix * 255.0f;

		privi_pix = privi_pix *AVERAGE_NUM;
		current_pix = current_pix *AVERAGE_NUM;

		diff_pix = current_pix - privi_pix;

		//min_pix = MIN(current_pix , privi_pix);
#ifdef VS_DEBUG_2
		F_WRITE(pfdebug ,(int)debug_ii++);
		F_WRITE(pfdebug ,(int)privi_pix);
		F_WRITE(pfdebug ,(int)diff_pix);
#endif

		if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
		{
#ifdef VS_DEBUG1
			F_WRITE(pfdebug ,(int)debug_ii++);
			F_WRITE(pfdebug ,(int)privi_pix);
			F_WRITE(pfdebug ,(int)diff_pix);
#endif
			//privi_pix = privi_pix + (s32)(privi_pix * 0.1f);
			//thr = (u8)(((float)dy->thr * 0.2 + (float)privi_pix * 0.8)+0.5);
			search_boundary_y(spt1 ,spt2 ,half_note_size_y ,down ,pbs ,pnote_param ,&x ,&y ,dy->thr ,-1 ,dy);

			scan_location_flg[current_point_num - 1] = 1;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = y;
			dy->record_ink_count++;
			dy->record_count++;
			current_record_num++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, (s32)dy->record_x[dy->record_count - 1], (s32)dy->record_y[dy->record_count - 1], end_y,start_y, 1, dynote_side_down, (s32)dy->record_x[start_offset - 1], (u32)&dy->record_count);
#endif

		}
		else if(privi_pix > dy->thr)	//差があるが縁が暗くない or 差もなく縁も暗くない
		{
			//券端座標を記録
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = start_y;

			dy->record_count++;
			//current_record_num++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, (s32)dy->record_x[dy->record_count - 1], (s32)dy->record_y[dy->record_count - 1], end_y, start_y, 1, dynote_side_down, (s32)dy->record_x[start_offset - 1], (u32)&dy->record_count);
#endif


		}

		else if(privi_pix < dy->thr)	//差はないが暗い場合
		{
			//上半分で行ったことをした半分でも行う。
			//privi_pix = current_pix;
			current_pix = 0;
			//x = end_x;
			//y = end_y;
			y = (s16)(end_y * 0.5f);

			for(i = 0; i < 2; ++i)
			{
				if(i == 1)
				{
					y = end_y;
				}

				for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
				{
					for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
					{

						spt1.x = x		+ noise_out_x;
						spt1.y = y  + noise_out_y;

						spt2.x = x		+ noise_out_x;
						spt2.y = y  + noise_out_y;

						new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
						new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

						temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
						//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
						//cal_val = temp_pix * temp_pix * 10000;
						temp_pix = temp_pix * temp_pix;
						//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;

						//privi_pix = temp_pix >> 1;
						//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
						current_pix = current_pix + temp_pix;
						//temp_pix = cal_val * 0.0000153787 * 255;
						//current_pix *= temp_pix;
					}
				}
				//privi_pix = privi_pix * NORMALIZ_VAL;
				current_pix = current_pix * NORMALIZ_VAL;

				//privi_pix = privi_pix * 255.0f;
				current_pix = current_pix * 255.0f;

				//privi_pix = privi_pix *AVERAGE_NUM;
				current_pix = current_pix *AVERAGE_NUM;

				diff_pix = current_pix - privi_pix;

#ifdef VS_DEBUG_2
				F_WRITE(pfdebug ,(int)debug_ii);
				F_WRITE(pfdebug ,(int)privi_pix);
				F_WRITE(pfdebug ,(int)diff_pix);
#endif
				if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
				{

#ifdef VS_DEBUG1
					F_WRITE(pfdebug ,(int)debug_ii++);
					F_WRITE(pfdebug ,(int)privi_pix);
					F_WRITE(pfdebug ,(int)diff_pix);
#endif
					//privi_pix = privi_pix + (s32)(privi_pix * 0.1f);
					//境界線検知
					search_boundary_y(spt1 ,spt2 ,half_note_size_y ,down ,pbs ,pnote_param ,&x ,&y ,dy->thr ,-1 ,dy);
					if (y < end_y - dy->note_corner_margin - dy->note_corner_margin)	//終点まで探索した場合は記録しない マージンを念のため２回計算している
					{
						scan_location_flg[current_point_num - 1] = 2;
						dy->record_x[dy->record_count] = x;
						dy->record_y[dy->record_count] = y;
						dy->record_ink_count++;
						dy->record_count++;
						current_record_num++;
					}
#ifdef VS_DEBUG
					debug_circle_write(pbs, pnote_param, (s32)dy->record_x[dy->record_count - 1], (s32)dy->record_y[dy->record_count - 1], end_y, (s32)(start_y * 0.75f), 1, dynote_side_down, (s32)dy->record_x[start_offset - 1], (u32)&dy->record_count);
#endif

					i = 2;
				}
				else	//差があるが縁が暗くない or 差もなく縁も暗くない
					//デザインだと判断　　　　　ついていない
				{

					////券端座標を記録
					//dy->record_x[dy->record_count] = x;
					//dy->record_y[dy->record_count] = start_y - dy->note_corner_margin;

					//dy->record_count++;
				}
			}


#ifdef VS_DEBUG_2
		else	//縁が暗いが内部との差がない
		{
			F_N(pfdebug);
		}
#endif
		}


#ifdef VS_DEBUG_2
		else	//縁が暗いが内部との差がない
		{
			F_N(pfdebug);
		}
#endif



	}
//#ifdef botu
//
//	y = 0;
//	privi_pix = 0;
//	current_pix = 0;
//
//	//ノイズを除去するフィルター関数を走らせると
//	//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
//	for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
//	{
//		for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
//		{
//
//			spt1.x = end_x	 + noise_out_x;
//			spt1.y = start_y + noise_out_y;
//
//			spt2.x = end_x	 + noise_out_x;
//			spt2.y = start_y + noise_out_y;
//
//			new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//			new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//			temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//			//cal_val = temp_pix * temp_pix  * 100000;
//			//cal_val = temp_pix * temp_pix * 10000;
//			temp_pix = temp_pix * temp_pix;
//
//			//privi_pix = temp_pix >> 1;
//			//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//			temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//			privi_pix = privi_pix + temp_pix;
//			//temp_pix = cal_val * 0.0000153787 * 255;
//			//current_pix *= temp_pix;
//
//			spt1.x = end_x + noise_out_x;
//			spt1.y = y	   + noise_out_y;
//
//			spt2.x = end_x + noise_out_x;
//			spt2.y = y     + noise_out_y;
//
//			temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//			//cal_val = temp_pix * temp_pix  * 100000;
//			//cal_val = temp_pix * temp_pix * 10000;
//			temp_pix = temp_pix * temp_pix;
//			//privi_pix = temp_pix >> 1;
//			//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//			temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//			current_pix = current_pix + temp_pix;
//			//temp_pix = cal_val * 0.0000153787 * 255;
//			//current_pix *= temp_pix;
//		}
//	}
//	privi_pix = privi_pix *AVERAGE_NUM;
//	current_pix = current_pix *AVERAGE_NUM;
//
//	//current_pix =current_pix * 0.111111111111f;
//	//privi_pix = privi_pix * 0.111111111111f;
//	diff_pix = current_pix - privi_pix;
//
//#ifdef VS_DEBUG1
//	F_WRITE(pfdebug ,(int)debug_ii++);
//	F_WRITE(pfdebug ,(int)privi_pix);
//	F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//
//	if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
//	{
//#ifdef VS_DEBUG_2
//		F_WRITE(pfdebug ,(int)debug_ii++);
//		F_WRITE(pfdebug ,(int)privi_pix);
//		F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//
//		//privi_pix = privi_pix + (s32)(privi_pix * 0.1f);
//		search_boundary_y(spt1 ,spt2 ,half_note_size_y ,down ,pbs ,pnote_param ,&end_x ,&y ,dy->thr,-1);
//
//#ifdef VS_DEBUG_2
//		F_N(pfdebug);
//		if(pbs->LEorSE == LE)
//		{
//			tmpx = x + pnote_param->main_eff_range / 2;	//
//			tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - y;	//
//		}
//		else
//		{
//			tmpy = y - pnote_param->main_eff_range / 2;	//
//			tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//		}
//		deb_para[0] = 4;		// function code
//		deb_para[1] = tmpx;
//		deb_para[2] = tmpy;	//
//		deb_para[3] = 1;		// plane
//		callback(deb_para);		// debug
//
//#endif
//
//		dy->record_x[dy->record_count] = x;
//		dy->record_y[dy->record_count] = y;
//		dy->record_ink_count++;
//		dy->record_count++;
//		current_record_num++;
//
//	}
//	else if(privi_pix > dy->thr)	//差があるが縁が暗くない or 差もなく縁も暗くない
//	{
//
//#ifdef VS_DEBUG_2
//		F_N(pfdebug);
//		if(pbs->LEorSE == LE)
//		{
//			tmpx = x + pnote_param->main_eff_range / 2;	//
//			tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - start_y;	//
//		}
//		else
//		{
//			tmpy = start_y - pnote_param->main_eff_range / 2;	//
//			tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//		}
//		deb_para[0] = 4;		// function code
//		deb_para[1] = tmpx;
//		deb_para[2] = tmpy;	//
//		deb_para[3] = 1;		// plane
//		callback(deb_para);		// debug
//
//#endif
//
//		//券端座標を記録
//		dy->record_x[dy->record_count] = end_x - dy->note_corner_margin;
//		dy->record_y[dy->record_count] = start_y - dy->note_corner_margin;
//
//		dy->record_count++;
//		current_record_num++;
//
//
//	}
//
//	else if(privi_pix < dy->thr)	//差はないが暗い場合
//	{
//		//上半分で行ったことをした半分でも行う。
//		//privi_pix = current_pix;
//		current_pix = 0;
//		x = end_x;
//		y = end_y;
//
//		for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
//		{
//			for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
//			{
//
//				spt1.x = x		+ noise_out_x;
//				spt1.y = y  + noise_out_y;
//
//				spt2.x = x		+ noise_out_x;
//				spt2.y = y  + noise_out_y;
//
//				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//				temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//				//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
//				//cal_val = temp_pix * temp_pix * 10000;
//				temp_pix = temp_pix * temp_pix;
//				//privi_pix = temp_pix >> 1;
//				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//				temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//				current_pix = current_pix + temp_pix;
//				//temp_pix = cal_val * 0.0000153787 * 255;
//				//current_pix *= temp_pix;
//			}
//		}
//		current_pix = current_pix *AVERAGE_NUM;
//		diff_pix = current_pix - privi_pix;
//
//#ifdef VS_DEBUG_2
//		F_WRITE(pfdebug ,(int)debug_ii);
//		F_WRITE(pfdebug ,(int)privi_pix);
//		F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//		if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
//		{
//
//#ifdef VS_DEBUG1
//			F_WRITE(pfdebug ,(int)debug_ii++);
//			F_WRITE(pfdebug ,(int)privi_pix);
//			F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//			//privi_pix = privi_pix + (s32)(privi_pix * 0.1f);
//			//境界線検知
//			search_boundary_y(spt1 ,spt2 ,half_note_size_y ,down ,pbs ,pnote_param ,&x ,&y ,dy->thr ,-1);
//
//#ifdef VS_DEBUG_2
//			F_N(pfdebug);
//			if(pbs->LEorSE == LE)
//			{
//				tmpx = x + pnote_param->main_eff_range / 2;	//
//				tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - y;	//
//			}
//			else
//			{
//				tmpy = y - pnote_param->main_eff_range / 2;	//
//				tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//			}
//			deb_para[0] = 4;		// function code
//			deb_para[1] = tmpx;
//			deb_para[2] = tmpy;	//
//			deb_para[3] = 1;		// plane
//			callback(deb_para);		// debug
//
//#endif
//
//			dy->record_x[dy->record_count] = x;
//			dy->record_y[dy->record_count] = y;
//			dy->record_ink_count++;
//			dy->record_count++;
//			current_record_num++;
//
//		}
//		else	//差があるが縁が暗くない or 差もなく縁も暗くない
//			//デザインだと判断　　　　　ついていない
//		{
//
//			//券端座標を記録
//			dy->record_x[dy->record_count] = end_x - dy->note_corner_margin;
//			dy->record_y[dy->record_count] = start_y - dy->note_corner_margin;
//
//
//			dy->record_count++;
//			current_record_num++;
//
//		}
//
//#ifdef VS_DEBUG_2
//	else	//縁が暗いが内部との差がない
//	{
//		F_N(pfdebug);
//	}
//#endif
//	}
//
//
//#ifdef VS_DEBUG_2
//	else	//縁が暗いが内部との差がない
//	{
//		F_N(pfdebug);
//	}
//#endif
//#endif

	//record_x[0] = record_x[1];
	//record_y[0] = end_y + dy->note_corner_margin;
	//record_x[record_count] = record_x[record_count-1];
	//record_y[record_count] = end_y + dy->note_corner_margin;
	//record_count++;

#ifdef ADD_NOT_DETECT_SMALL

	if(current_record_num == 2 ||current_record_num == 1)
	{
		for(i = 0; i < current_point_num; i++)
		{
			dy->record_y[start_offset + i] = rounding;
		}
	}

#endif

	//特異点削除を実行する。（スレッドなどを誤検知しないようにするための処理）
	if (dy->singularity_exclusion_flg == 1)
	{
		singularity_exclusion(dy, scan_location_flg, start_offset, dynote_side_down);
	}

	return 0;
}


//************************************************************
//phase2の関数
//紙幣の枠から広幅でスキャンを開始して、前画素との差が生じたら細かく分析する。
//return 0　インクなし
//		１　あり
//out：dy->side1 と　side2
u32 search_ink_2_left(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param  ,ST_DYE_NOTE *dy)
{
#ifdef DYE_NOTE_VIEWER_MODE
	//券端情報
	s16 start_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 end_x	=  (dy->note_size_x >> 1) - dy->note_corner_margin;
	s16 start_y = -(dy->note_size_y >> 1) + dy->note_corner_margin;
	s16 end_y	= (dy->note_size_y >> 1) - dy->note_corner_margin;
#endif
	//券端情報
	s16 start_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 end_x	=  (dy->note_size_x >> 1) - dy->note_corner_margin;
	s16 start_y =  dy->record_y[dy->record_count - 1] + dy->note_corner_margin;
	s16 end_y	=  dy->record_y[0] - dy->note_corner_margin;
	s16 half_note_size_x = dy->note_size_x >> 1;
	u8  i = 0;

	s16 x = 0;
	s16 y = 0;

	u8 scan_location_flg[128] = { 0 };

	s16 noise_out_x;
	s16 noise_out_y;
#ifdef VS_DEBUG
	s16 tmpx;
	s16 tmpy;
#endif
	float temp_pix = 0;
	float privi_pix = 0;
	float current_pix = 0;
	float diff_pix = 0;
	
	u8 current_point_num = 0;
	u8 current_record_num = 0;
	//s16 rounding = start_x - dy->note_corner_margin;
	u16 start_offset = dy->record_count;

	ST_SPOINT spt1;
	ST_SPOINT spt2;

	spt1.l_plane_tbl = dy->plane1;
	spt2.l_plane_tbl = dy->plane2;

	spt1.way = pbs->insertion_direction;
	spt2.way = pbs->insertion_direction;

#ifndef DYE_NOTE_VIEWER_MODE
	for(i = 1; i < 4; ++i)	//最も小さい値を設定する。
	{
		if(end_y > dy->record_y[i] - dy->note_corner_margin)
		{
			end_y	=  dy->record_y[i] - dy->note_corner_margin;
		}
	}
#endif

	//紙幣をスキャンし差分が大きい場合インク有と判断する
	for(y = start_y; y < end_y; y = y + dy->scan_pitch)
	{
		x = 0;

		current_pix = 0;
		privi_pix = 0;
		current_point_num++;

#ifdef ADD_DETECT_TRANSMISSION
		//透過イメージを用いて裂け、角折れなどをインクと区別する。
		spt1.x = start_x;
		spt1.y = y;

		spt1.l_plane_tbl = OMOTE_T_G;
		if (pbs->PlaneInfo[spt1.l_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
		{
			spt1.l_plane_tbl = URA_T_G;			 //用いるプレーン指定
			//ED.Plane = DOWN_T_R;			 //用いるプレーン指定
		}
		new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
		spt1.l_plane_tbl = dy->plane1;
		if(spt1.sensdt > 245)
		{
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = start_x;
			dy->record_y[dy->record_count] = y;
			dy->record_count++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, (s32)dy->record_x[dy->record_count - 1], (s32)dy->record_y[dy->record_count - 1], 0, 0, 0, dynote_side_left, (s32)dy->record_y[start_offset - 1], (u32)&dy->record_count);
#endif
			continue;

		}
#endif

		//ノイズを除去するフィルター関数を走らせると
		//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
		for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
		{
			for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
			{
				spt1.x = start_x + noise_out_x;
				spt1.y = y		 + noise_out_y;

				spt2.x = start_x + noise_out_x;
				spt2.y = y		 + noise_out_y;

				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
				//cal_val = temp_pix * temp_pix  * 100000;
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
				//privi_pix = temp_pix >> 1;
				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
				privi_pix = privi_pix + temp_pix;
				//privi_pix = cal_val * 0.0000153787 * 255;	//正規化

				spt1.x = x + noise_out_x;
				spt1.y = y + noise_out_y;

				spt2.x = x + noise_out_x;
				spt2.y = y + noise_out_y;

				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
				//cal_val = temp_pix * temp_pix  * 100000;
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
				//privi_pix = temp_pix >> 1;
				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
				current_pix = current_pix + temp_pix;
				//temp_pix = cal_val * 0.0000153787 * 255;
				//current_pix *= temp_pix;
			}
		}
		privi_pix = privi_pix * NORMALIZ_VAL;
		current_pix = current_pix * NORMALIZ_VAL;

		privi_pix = privi_pix * 255.0f;
		current_pix = current_pix * 255.0f;

		privi_pix = privi_pix *AVERAGE_NUM;
		current_pix = current_pix *AVERAGE_NUM;

		diff_pix = current_pix - privi_pix;

		//min_pix = MIN(current_pix , privi_pix);
#ifdef VS_DEBUG_2
		F_WRITE(pfdebug ,(int)debug_ii++);
		F_WRITE(pfdebug ,(int)privi_pix);
		F_WRITE(pfdebug ,(int)diff_pix);
#endif
		if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
		{
#ifdef VS_DEBUG1
			F_WRITE(pfdebug ,(int)debug_ii++);
			F_WRITE(pfdebug ,(int)privi_pix);
			F_WRITE(pfdebug ,(int)diff_pix);
#endif
			//privi_pix = privi_pix + (s32)(diff_pix * 0.1f);
			search_boundary_x(spt1 ,spt2 ,half_note_size_x ,left ,pbs ,pnote_param ,&x ,&y ,dy->thr, 1 ,dy);

			scan_location_flg[current_point_num - 1] = 1;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = y;
			dy->record_ink_count++;
			dy->record_count++;
			current_record_num++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], end_x, start_x, 0, dynote_side_left, dy->record_y[start_offset - 1], &dy->record_count);
#endif

		}
		else if(privi_pix > dy->thr)	//差があるが縁が暗くない or 差もなく縁も暗くない
		{

			//券端座標を記録
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = start_x - dy->note_corner_margin;
			dy->record_y[dy->record_count] = y;

			dy->record_count++;
#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], end_x, start_x, 0, dynote_side_left, dy->record_y[start_offset - 1], &dy->record_count);
#endif

		}



		else if(privi_pix < dy->thr)	//差はないが暗い場合	真ん中が明るい場合（暗明暗）の場合誤検知するので変更必要
		{

			//（暗明暗）をチェックする

			//上半分で行ったことをした半分でも行う。
			//座標更新
			current_pix = 0;
#ifdef VS_DEBUG
			x = end_x;
#endif
			x = (s16)(start_x * 0.5f);

			for(i = 0; i < 2; ++i)	//暗明暗パターン用
			{
				if(i == 1)
				{
					x = start_x;
				}


				for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
				{
					for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
					{

						spt1.x = x	+ noise_out_x;
						spt1.y = y  + noise_out_y;

						spt2.x = x	+ noise_out_x;
						spt2.y = y  + noise_out_y;

						new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
						new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

						temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
						//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
						//cal_val = temp_pix * temp_pix * 10000;
						temp_pix = temp_pix * temp_pix;
						//temp_pix = temp_pix * NORMALIZ_VAL;	//正規化;

						//privi_pix = temp_pix >> 1;
						//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
						current_pix = current_pix + temp_pix;
						//temp_pix = cal_val * 0.0000153787 * 255;
						//current_pix *= temp_pix;
					}
				}
				//privi_pix = privi_pix * NORMALIZ_VAL;
				current_pix = current_pix * NORMALIZ_VAL;

				//privi_pix = privi_pix * 255.0f;
				current_pix = current_pix * 255.0f;

				//privi_pix = privi_pix *AVERAGE_NUM;
				current_pix = current_pix *AVERAGE_NUM;

				diff_pix = current_pix - privi_pix;

				if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
				{
					//境界線検知
					search_boundary_x(spt1 ,spt2 ,half_note_size_x ,left ,pbs ,pnote_param ,&x ,&y ,dy->thr, 1 ,dy);

					if (x < end_x - dy->note_corner_margin - dy->note_corner_margin)	//終点まで探索した場合は記録しない マージンを念のため２回計算している
					{
						scan_location_flg[current_point_num - 1] = 2;
						dy->record_x[dy->record_count] = x;
						dy->record_y[dy->record_count] = y;
						dy->record_ink_count++;
						dy->record_count++;
						current_record_num++;
					}
#ifdef VS_DEBUG
					debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], end_x, start_x, 0, dynote_side_left, dy->record_y[start_offset - 1], &dy->record_count);
#endif

					i = 2;
				}
				else	//差があるが縁が暗くない or 差もなく縁も暗くない
					//デザインだと判断　　　　　ついていない
				{

				}
			}

#ifdef VS_DEBUG_2
		else	//縁が暗いが内部との差がない
		{
			F_N(pfdebug);
		}
#endif

		}
#ifdef VS_DEBUG_2
		else	//縁が暗いが内部との差がない
		{
			F_N(pfdebug);
		}
#endif
	}

#ifdef ADD_NOT_DETECT_SMALL

	if(current_record_num == 2 ||current_record_num == 1)
	{
		for(i = 0; i < current_point_num; i++)
		{
			dy->record_x[start_offset + i] = rounding;
		}
	}

#endif

	//特異点削除を実行する。（スレッドなどを誤検知しないようにするための処理）
	if (dy->singularity_exclusion_flg == 1)
	{
		singularity_exclusion(dy, scan_location_flg, start_offset, dynote_side_left);
	}

	return 0;
}


//************************************************************
//phase2の関数
//紙幣の枠から広幅でスキャンを開始して、前画素との差が生じたら細かく分析する。
//return 0　インクなし
//		１　あり
//out：dy->side1 と　side2
u32 search_ink_2_right(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param  ,ST_DYE_NOTE *dy)
{


	//券端情報
#if DYE_NOTE_VIEWER_MODE
	s16 end_x = (dy->note_size_x >> 1) - dy->note_corner_margin;
	s16 start_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	s16 start_y =  (dy->note_size_y >> 1) - dy->note_corner_margin;
	//s16 start_y = dy->record_y[dy->record_count - 1] - dy->note_corner_margin;
	s16 end_y = -(dy->note_size_y >> 1) + dy->note_corner_margin;
#else
	s16 end_x = (dy->note_size_x >> 1) - dy->note_corner_margin;
	s16 start_x = -(dy->note_size_x >> 1) + dy->note_corner_margin;
	//s16 start_y =  (dy->note_size_y >> 1) - dy->note_corner_margin;
	s16 start_y = dy->record_y[dy->record_count - 1] - dy->note_corner_margin;
	s16 end_y = -(dy->note_size_y >> 1) + dy->note_corner_margin;
#endif
	s16 half_note_size_x = dy->note_size_x >> 1;

	s16 x = 0;
	s16 y = 0;

	s16 noise_out_x;
	s16 noise_out_y;

	u8 scan_location_flg[128] = { 0 };

#ifdef VS_DEBUG
	s16 tmpx;
	s16 tmpy;
#endif

	u8 i = 0;
	float temp_pix = 0;
	float privi_pix = 0;
	float current_pix = 0;
	float diff_pix = 0;
	
	u8 current_point_num = 0;
	u8 current_record_num = 0;
	//s16 rounding = end_x + dy->note_corner_margin;
	u16 start_offset = dy->record_count;

	ST_SPOINT spt1;
	ST_SPOINT spt2;

	spt1.l_plane_tbl = dy->plane1;
	spt2.l_plane_tbl = dy->plane2;

	spt1.way = pbs->insertion_direction;
	spt2.way = pbs->insertion_direction;

	//紙幣をスキャンし差分が大きい場合インク有と判断する
	for(y = start_y; y > end_y; y = y - dy->scan_pitch)
	{
		x = 0;
		current_point_num++;

		current_pix = 0;
		privi_pix = 0;

#ifdef ADD_DETECT_TRANSMISSION
		//透過イメージを用いて裂け、角折れなどをインクと区別する。
		spt1.x = end_x;
		spt1.y = y;

		spt1.l_plane_tbl = OMOTE_T_G;
		if (pbs->PlaneInfo[spt1.l_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
		{
			spt1.l_plane_tbl = URA_T_G;			 //用いるプレーン指定
			//ED.Plane = DOWN_T_R;			 //用いるプレーン指定
		}
		new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
		spt1.l_plane_tbl = dy->plane1;
		if(spt1.sensdt > 245)
		{
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = end_x;
			dy->record_y[dy->record_count] = y;
			dy->record_count++;
#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, (s32)dy->record_x[dy->record_count - 1], (s32)dy->record_y[dy->record_count - 1], 0, 0, 0, dynote_side_right, (s32)dy->record_y[start_offset - 1], (u32)&dy->record_count);
#endif
			continue;
		}
#endif

		//ノイズを除去するフィルター関数を走らせると
		//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
		for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
		{
			for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
			{

				spt1.x = end_x	+ noise_out_x;
				spt1.y = y		+ noise_out_y;

				spt2.x = end_x	+ noise_out_x;
				spt2.y = y		+ noise_out_y;

				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
				//cal_val = temp_pix * temp_pix  * 100000;
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//privi_pix = temp_pix >> 1;
				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
				//temp_pix = temp_pix * NORMALIZ_VAL;	//正規化;
				privi_pix = privi_pix + temp_pix;
				//privi_pix = cal_val * 0.0000153787 * 255;	//正規化

				spt1.x = x + noise_out_x;
				spt1.y = y + noise_out_y;

				spt2.x = x + noise_out_x;
				spt2.y = y + noise_out_y;

				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

				temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
				//cal_val = temp_pix * temp_pix  * 100000;
				//cal_val = temp_pix * temp_pix * 10000;
				temp_pix = temp_pix * temp_pix;
				//privi_pix = temp_pix >> 1;
				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
				//temp_pix = temp_pix * NORMALIZ_VAL;	//正規化;
				current_pix = current_pix + temp_pix;
				//temp_pix = cal_val * 0.0000153787 * 255;
				//current_pix *= temp_pix;

			}
		}



		privi_pix = privi_pix * NORMALIZ_VAL;
		current_pix = current_pix * NORMALIZ_VAL;

		privi_pix = privi_pix * 255.0f;
		current_pix = current_pix * 255.0f;

		privi_pix = privi_pix *AVERAGE_NUM;
		current_pix = current_pix *AVERAGE_NUM;

		diff_pix = current_pix - privi_pix;


		//min_pix = MIN(current_pix , privi_pix);
#ifdef VS_DEBUG_2
		F_WRITE(pfdebug ,(int)debug_ii++);
		F_WRITE(pfdebug ,(int)privi_pix);
		F_WRITE(pfdebug ,(int)diff_pix);
#endif
		if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
		{
#ifdef VS_DEBUG1
			F_WRITE(pfdebug ,(int)debug_ii++);
			F_WRITE(pfdebug ,(int)privi_pix);
			F_WRITE(pfdebug ,(int)diff_pix);
#endif
			//privi_pix = privi_pix + (s32)(diff_pix * 0.1f);
			search_boundary_x(spt1 ,spt2 ,half_note_size_x ,right ,pbs ,pnote_param ,&x ,&y ,dy->thr ,-1 ,dy);

			scan_location_flg[current_point_num - 1] = 1;
			dy->record_x[dy->record_count] = x;
			dy->record_y[dy->record_count] = y;
			dy->record_ink_count++;
			dy->record_count++;
			current_record_num++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], end_x, start_x, 0, dynote_side_right, dy->record_y[start_offset - 1], &dy->record_count);
#endif

		}
		else if(privi_pix > dy->thr)	//差があるが縁が暗くない or 差もなく縁も暗くない
		{
			//券端座標を記録
			scan_location_flg[current_point_num - 1] = 0;
			dy->record_x[dy->record_count] = end_x + dy->note_corner_margin;
			dy->record_y[dy->record_count] = y;
			dy->record_count++;

#ifdef VS_DEBUG
			debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], end_x, start_x, 0, dynote_side_right, dy->record_y[start_offset - 1],&dy->record_count);
#endif

		}
		else if(privi_pix < dy->thr)	//差はないが暗い場合
		{
			//上半分で行ったことをした半分でも行う。
			//privi_pix = current_pix;
			current_pix = 0;
			x = (s16)(-end_x * 0.5f);

			for(i = 0; i < 2; ++i)
			{
				if(i == 1)
				{
					x = start_x;
				}

				for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
				{
					for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
					{

						spt1.x = x	+ noise_out_x;
						spt1.y = y  + noise_out_y;

						spt2.x = x	+ noise_out_x;
						spt2.y = y  + noise_out_y;

						new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
						new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

						temp_pix = (float)((u16)spt1.sensdt + (u16)spt2.sensdt);
						//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
						//cal_val = temp_pix * temp_pix * 10000;
						temp_pix = temp_pix * temp_pix;
						//privi_pix = temp_pix >> 1;
						//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
						//temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
						current_pix = current_pix + temp_pix;
						//temp_pix = cal_val * 0.0000153787 * 255;
						//current_pix *= temp_pix;

					}
				}
				current_pix = current_pix * NORMALIZ_VAL;
				current_pix = current_pix * 255.0f;
				current_pix = current_pix * AVERAGE_NUM;


				diff_pix = current_pix - privi_pix;

#ifdef VS_DEBUG_2
				F_WRITE(pfdebug ,(int)debug_ii);
				F_WRITE(pfdebug ,(int)privi_pix);
				F_WRITE(pfdebug ,(int)diff_pix);
#endif
				if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
				{

#ifdef VS_DEBUG1
					F_WRITE(pfdebug ,(int)debug_ii++);
					F_WRITE(pfdebug ,(int)privi_pix);
					F_WRITE(pfdebug ,(int)diff_pix);
#endif
					//privi_pix = privi_pix + (s32)(diff_pix * 0.1f);
					//境界線検知
					search_boundary_x(spt1 ,spt2 ,half_note_size_x ,right ,pbs ,pnote_param ,&x ,&y ,dy->thr ,-1 ,dy);
					if (x > start_x + dy->note_corner_margin + dy->note_corner_margin)	//終点まで探索した場合は記録しない マージンを念のため２回計算している
					{

						scan_location_flg[current_point_num - 1] = 2;
						dy->record_x[dy->record_count] = x;
						dy->record_y[dy->record_count] = y;
						dy->record_ink_count++;
						dy->record_count++;
						current_record_num++;
					}
#ifdef VS_DEBUG
					debug_circle_write(pbs, pnote_param, dy->record_x[dy->record_count - 1], dy->record_y[dy->record_count - 1], end_x, start_x, 0, dynote_side_right, dy->record_y[start_offset - 1], &dy->record_count);
#endif

					i = 2;
				}
				else	//差があるが縁が暗くない or 差もなく縁も暗くない
					//デザインだと判断　　　　　ついていない
				{

					//	//券端座標を記録
					//dy->record_x[dy->record_count] = end_x + dy->note_corner_margin;
					//dy->record_y[dy->record_count] = y;

					//	dy->record_count++;
				}
			}


#ifdef VS_DEBUG_2
		else	//縁が暗いが内部との差がない
		{
			F_N(pfdebug);
		}
#endif
		}
#ifdef VS_DEBUG_2
		else	//縁が暗いが内部との差がない
		{
			F_N(pfdebug);
		}
#endif

		//else	//縁が暗いが内部との差がない
		//{
		//continue
		//}
	}

//#ifdef botu
//
//	//最終券端のスキャン
//	x = 0;
//	privi_pix = 0;
//	current_pix = 0;
//
//	//ノイズを除去するフィルター関数を走らせると
//	//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
//	for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
//	{
//		for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
//		{
//
//			spt1.x = end_x + noise_out_x;
//			spt1.y = end_y + noise_out_y;
//
//			spt2.x = end_x + noise_out_x;
//			spt2.y = end_y + noise_out_y;
//
//			new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//			new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//			temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//			//cal_val = temp_pix * temp_pix  * 100000;
//			//cal_val = temp_pix * temp_pix * 10000;
//			temp_pix = temp_pix * temp_pix;
//			//privi_pix = temp_pix >> 1;
//			//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//			temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//			privi_pix = privi_pix + temp_pix;
//			//temp_pix = cal_val * 0.0000153787 * 255;
//			//current_pix *= temp_pix;
//
//			spt1.x = x		 + noise_out_x;
//			spt1.y = end_y	 + noise_out_y;
//
//			spt2.x = x		 + noise_out_x;
//			spt2.y = end_y	 + noise_out_y;
//
//			new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//			new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//			temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//			//cal_val = temp_pix * temp_pix  * 100000;
//			//cal_val = temp_pix * temp_pix * 10000;
//			temp_pix = temp_pix * temp_pix;
//			//privi_pix = temp_pix >> 1;
//			//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//			temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//			current_pix = current_pix + temp_pix;
//			//temp_pix = cal_val * 0.0000153787 * 255;
//			//current_pix *= temp_pix;
//		}}
//	//current_pix =current_pix * 0.111111111111f;
//	//privi_pix = privi_pix * 0.111111111111f;
//	privi_pix = privi_pix *AVERAGE_NUM;
//	current_pix = current_pix *AVERAGE_NUM;
//	diff_pix = current_pix - privi_pix;
//
//	//min_pix = MIN(current_pix , privi_pix);
//#ifdef VS_DEBUG1
//	F_WRITE(pfdebug ,(int)debug_ii++);
//	F_WRITE(pfdebug ,(int)privi_pix);
//	F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//
//	if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
//	{
//#ifdef VS_DEBUG_2
//		F_WRITE(pfdebug ,(int)debug_ii++);
//		F_WRITE(pfdebug ,(int)privi_pix);
//		F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//
//		//privi_pix = privi_pix + (s32)(diff_pix * 0.1f);
//		search_boundary_x(spt1 ,spt2 ,half_note_size_x ,right ,pbs ,pnote_param ,&x ,&end_y ,dy->thr,-1);
//
//#ifdef VS_DEBUG
//		//F_N(pfdebug);
//		if(pbs->LEorSE == LE)
//		{
//			tmpx = x + pnote_param->main_eff_range / 2;	//
//			tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - end_y;	//
//		}
//		else
//		{
//			tmpy = end_y - pnote_param->main_eff_range / 2;	//
//			tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//		}
//		deb_para[0] = 4;		// function code
//		deb_para[1] = tmpx;
//		deb_para[2] = tmpy;	//
//		deb_para[3] = 1;		// plane
//		callback(deb_para);		// debug
//
//#endif
//
//		dy->record_x[dy->record_count] = x;
//		dy->record_y[dy->record_count] = end_y - dy->note_corner_margin;
//		dy->record_ink_count++;
//		dy->record_count++;
//		current_record_num++;
//
//	}
//	else if(privi_pix > dy->thr)	//差があるが縁が暗くない or 差もなく縁も暗くない
//	{
//
//#ifdef VS_DEBUG_2
//		F_N(pfdebug);
//		if(pbs->LEorSE == LE)
//		{
//			tmpx = end_x + pnote_param->main_eff_range / 2;	//
//			tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - end_y;	//
//		}
//		else
//		{
//			tmpy = end_y - pnote_param->main_eff_range / 2;	//
//			tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - end_x;	//
//
//		}
//		deb_para[0] = 4;		// function code
//		deb_para[1] = tmpx;
//		deb_para[2] = tmpy;	//
//		deb_para[3] = 1;		// plane
//		callback(deb_para);		// debug
//
//#endif
//
//		//券端座標を記録
//		dy->record_x[dy->record_count] = end_x + dy->note_corner_margin;
//		dy->record_y[dy->record_count] = end_y - dy->note_corner_margin;
//
//		dy->record_count++;
//		current_record_num++;
//
//	}
//
//	else if(privi_pix < dy->thr)	//差はないが暗い場合
//	{
//		//上半分で行ったことをした半分でも行う。
//		//privi_pix = current_pix;
//		current_pix = 0;
//		x = start_x;
//		y = end_y;
//
//
//		for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
//		{
//			for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
//			{
//
//				spt1.x = x	+ noise_out_x;
//				spt1.y = y  + noise_out_y;
//
//				spt2.x = x	+ noise_out_x;
//				spt2.y = y  + noise_out_y;
//
//				new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
//				new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取
//
//				temp_pix = ((u16)spt1.sensdt + (u16)spt2.sensdt);
//				//cal_val = temp_pix * temp_pix * 100000;				//合計値増幅
//				//cal_val = temp_pix * temp_pix * 10000;
//				temp_pix = temp_pix * temp_pix;
//				//privi_pix = temp_pix >> 1;
//				//cal_val = cal_val * NORMALIZ_VAL * 255.0f;	//正規化;
//				temp_pix = temp_pix * NORMALIZ_VAL * 255.0f;	//正規化;
//
//				current_pix = current_pix + temp_pix;
//				//temp_pix = cal_val * 0.0000153787 * 255;
//				//current_pix *= temp_pix;
//			}
//		}
//
//		current_pix = current_pix *AVERAGE_NUM;
//
//		diff_pix = current_pix - privi_pix;
//
//#ifdef VS_DEBUG_2
//		F_WRITE(pfdebug ,(int)debug_ii);
//		F_WRITE(pfdebug ,(int)privi_pix);
//		F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//		if(diff_pix > PIX_DIFF_THR && privi_pix < dy->thr)	//明らかに縁と内部との差がある　さらに縁が暗い
//		{
//
//#ifdef VS_DEBUG1
//			F_WRITE(pfdebug ,(int)debug_ii++);
//			F_WRITE(pfdebug ,(int)privi_pix);
//			F_WRITE(pfdebug ,(int)diff_pix);
//#endif
//			//privi_pix = privi_pix + (s32)(diff_pix * 0.1f);
//			//境界線検知
//			search_boundary_x(spt1 ,spt2 ,half_note_size_x ,right ,pbs ,pnote_param ,&x ,&y ,dy->thr ,-1);
//
//#ifdef VS_DEBUG_2
//			F_N(pfdebug);
//			if(pbs->LEorSE == LE)
//			{
//				tmpx = x + pnote_param->main_eff_range / 2;	//
//				tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - y;	//
//			}
//			else
//			{
//				tmpy = y - pnote_param->main_eff_range / 2;	//
//				tmpx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - x;	//
//
//			}
//			deb_para[0] = 4;		// function code
//			deb_para[1] = tmpx;
//			deb_para[2] = tmpy;	//
//			deb_para[3] = 1;		// plane
//			callback(deb_para);		// debug
//
//#endif
//
//			dy->record_x[dy->record_count] = x;
//			dy->record_y[dy->record_count] = end_y - dy->note_corner_margin;
//			dy->record_ink_count++;
//			dy->record_count++;
//			current_record_num++;
//
//		}
//
//		else	//差があるが縁が暗くない or 差もなく縁も暗くない
//			//デザインだと判断　　　　　ついていない
//		{
//
//			//券端座標を記録
//			dy->record_x[dy->record_count] = end_x + dy->note_corner_margin;
//			dy->record_y[dy->record_count] = end_y - dy->note_corner_margin;
//
//			dy->record_count++;
//			current_record_num++;
//
//		}
//
//
//#ifdef VS_DEBUG_2
//	else	//縁が暗いが内部との差がない
//	{
//		F_N(pfdebug);
//	}
//#endif
//	}
//
//#ifdef VS_DEBUG_2
//	else	//縁が暗いが内部との差がない
//	{
//		F_N(pfdebug);
//	}
//#endif
//
//#endif
//
	////券端位置の座標を配列に追加
	//record_x[0] = end_x + dy->note_corner_margin;
	//record_y[0] = record_y[1];

	//record_x[record_count] = end_x + dy->note_corner_margin;
	//record_y[record_count] = record_y[record_count-1];

	//record_count++;

	//return area(record_x , record_y , record_count, end_x + dy->note_corner_margin, record_y[record_count / 2]);	//面積計算

#ifdef ADD_NOT_DETECT_SMALL

	if(current_record_num == 2 ||current_record_num == 1)
	{
		for(i = 0; i < current_point_num; i++)
		{
			dy->record_x[start_offset + i] = rounding;
		}
	}

#endif
	//特異点削除を実行する。（スレッドなどを誤検知しないようにするための処理）
	if (dy->singularity_exclusion_flg == 1)
	{
		singularity_exclusion(dy, scan_location_flg, start_offset, dynote_side_right);
	}

	return 0;
}

//****************************************************
//再起呼び出しで境界点を求める
s16 search_boundary_x(ST_SPOINT spt1 ,ST_SPOINT spt2 , s16 width ,s8 flg ,ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,s16* x ,s16* y ,float thr ,s8 left_or_right  ,ST_DYE_NOTE *dy)
{
	float temp_pix = 0;
	float temp_val = 0;
	//s16 dir  = 0;
	s8 noise_out_x = 0;
	s8 noise_out_y = 0;
	//gcal_val = 0;

	//移動できなくなったら終わり
	if(width < 5)
	{
		return 0;
	}

	//前回の幅の半分を求める
	width = (s16)(width * 0.5f);
	//右が左かで符号入れ替え
	//dir = width * flg;
	//x座標を移動
	*x = *x +  width * flg;

	//ノイズを除去するフィルター関数を走らせると
	//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
	for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
	{
		for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
		{
			//２プレーン分のパラメタをセットし画素採取
			spt1.x = *x + noise_out_x;
			spt1.y = *y + noise_out_y;
			spt2.x = *x + noise_out_x;
			spt2.y = *y + noise_out_y;

			new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
			new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

			//画素の平均値を求める
			temp_pix = (float)(spt1.sensdt + spt2.sensdt);
			//temp_pix = temp_pix >> 1;
			//gcal_val += temp_pix;
			//gcal_val += temp_pix * temp_pix * 10000;//  * 100000;
			temp_pix = temp_pix * temp_pix;
			temp_val = temp_val + temp_pix;	//正規化;
			//temp_val = cal_ + temp_val;
			//temp_val += gcal_val * NORMALIZ_VAL * 255;
			//diff_pix = abs(temp_pix - privi_pix);

		}
	}
	temp_val = temp_val * NORMALIZ_VAL;
	temp_val = temp_val *AVERAGE_NUM;
	temp_val = temp_val * 255.0f;

#ifdef VS_DEBUG_2
	F_WRITE(pfdebug ,temp_val);
#endif

	//平均値が閾値以上の場合
	if(temp_val >= thr /*+ MIN_PIX_MARGIN*/)	//マージンは必要で紙幣の縁より内部の方が浸透の関係で薄くなる
	{
		//現在のポイントよりインクは左側にあると判断する
		search_boundary_x(spt1 ,spt2 ,width ,left * left_or_right ,pbs ,pnote_param ,x ,y ,thr ,left_or_right ,dy);
	}
	//閾値未満の場合
	else if(temp_val < thr /*+ MIN_PIX_MARGIN*/)
	{
		//右側にあると判断する。
		search_boundary_x(spt1 ,spt2 ,width ,right * left_or_right ,pbs ,pnote_param ,x ,y ,thr,left_or_right ,dy);
	}
	else
	{
		return 0;
	}

	return 0;
}

//****************************************************
//再起呼び出しで境界点を求める
s16 search_boundary_y(ST_SPOINT spt1 ,ST_SPOINT spt2 , s16 width ,s8 flg ,ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,s16* x ,s16* y ,float thr ,s8 up_or_down  ,ST_DYE_NOTE *dy)
{
	float temp_pix = 0;
	float temp_val = 0;

	//s16 dir  = 0;
	s8 noise_out_x = 0;
	s8 noise_out_y = 0;
	//gcal_val = 0;

	//移動できなくなったら終わり
	if(width < 5)
	{
		return 0;
	}

	//前回の幅の半分を求める
	width = (s16)(width * 0.5f);
	//右が左かで符号入れ替え
	//dir = width * flg;
	//x座標を移動
	*y = *y +  width * flg;

	//ノイズを除去するフィルター関数を走らせると
	//処理時間が間に合わないので、大きめに見てノイズ除去を行う。
	for(noise_out_y = -MASK_SIZE; noise_out_y < MASK_SIZE +1 ; noise_out_y = noise_out_y + MASK_SIZE)
	{
		for(noise_out_x = -MASK_SIZE; noise_out_x < MASK_SIZE + 1; noise_out_x = noise_out_x + MASK_SIZE)
		{
			//２プレーン分のパラメタをセットし画素採取
			//*x = *x + noise_out_x;
			//*y = *y + noise_out_y;

			spt1.x = *x + noise_out_x;
			spt1.y = *y + noise_out_y;
			spt2.x = *x + noise_out_x;
			spt2.y = *y + noise_out_y;

			new_L_Getdt(&spt1 ,pbs ,pnote_param);	//画素採取
			new_L_Getdt(&spt2 ,pbs ,pnote_param);	//画素採取

			//画素の平均値を求める
			temp_pix = (float)(spt1.sensdt + spt2.sensdt);
			//temp_pix = temp_pix >> 1;
			//gcal_val += temp_pix;
			//gcal_val += temp_pix * temp_pix * 10000;//  * 100000;
			temp_pix = temp_pix * temp_pix;
			temp_val = temp_val + temp_pix;	//正規化;
			//temp_val = cal_ + temp_val;
			//temp_val += gcal_val * NORMALIZ_VAL * 255;
			//diff_pix = abs(temp_pix - privi_pix);

		}
	}
	temp_val = temp_val * NORMALIZ_VAL;

	temp_val = temp_val *AVERAGE_NUM;
	temp_val = temp_val * 255.0f;


#ifdef VS_DEBUG_2
	F_WRITE(pfdebug ,temp_val);
#endif

	//平均値が閾値以上の場合
	if(temp_val >= thr/* - MIN_PIX_MARGIN*/)	//マージンは必要で紙幣の縁より内部の方が浸透の関係で薄くなる 下げると厳しくなる
	{
		//現在のポイントよりインクは上側にあると判断する
		search_boundary_y(spt1 ,spt2 ,width ,up *up_or_down  ,pbs ,pnote_param ,x ,y ,thr ,up_or_down ,dy);
	}
	//閾値未満の場合
	else if(temp_val < thr /*+ MIN_PIX_MARGIN*/)
	{
		//下側にあると判断する。
		search_boundary_y(spt1 ,spt2 ,width ,down *up_or_down ,pbs ,pnote_param ,x ,y ,thr ,up_or_down ,dy);
	}
	else
	{
		return 0;

	}

	return 0;

}

//************************************************************
//変数の初期化を行う
void status_ini(ST_DYE_NOTE *dy)
{
	dy->res_ink_ratio = 0;
	dy->res_ink_area = 0;
	dy->record_count = 0;
	dy->record_ink_count = 0;
	dy->judge = 0;

	memset( &dy->record_x[0] ,0x00 ,sizeof(dy->record_x));
	memset( &dy->record_y[0] ,0x00 ,sizeof(dy->record_y));
	//memset( &dy->note_center_val_horaizontal[0] ,0x00 ,sizeof(dy->note_center_val_horaizontal));
	//memset( &dy->note_center_val_vertical[0] ,0x00 ,sizeof(dy->note_center_val_vertical));

}

//************************************************************
//多角形の面積を求める
//複数の三角形の面積の合計を求める 
//float area(s16 *x , s16 *y , u16 point_count ,s16 origin_x ,s16 origin_y) {
u32 area(s16 *x , s16 *y , u16 point_count ,s16 origin_x ,s16 origin_y) {

	u32 i = 0;
	u32 sarea = 0;
	u32 temp;

	//float element1 = 0;
	//float element2 = 0;

	point_count = point_count - 1;

	for (i = 0; i < point_count; i = i+1)
	{
		//temp = abs(((x[i] - origin_x) + (x[i+1] - origin_x)) * ((y[i] - origin_y) - (y[i+1] - origin_y)));
		temp = ((x[i] - origin_x) + (x[i+1] - origin_x)) * ((y[i] - origin_y) - (y[i+1] - origin_y));
		sarea += temp;
	}

	temp = abs(((x[i] - origin_x) + (x[0] - origin_x)) * ((y[i] - origin_y) - (y[0] - origin_y)));
	sarea += temp;

	sarea = (u32)(sarea * 0.5f);

	return sarea;
}

//レベル計算
u8 dye_level_detect(ST_DYE_NOTE *dy)
{

#define MIN_LIMIT_NUM_DYENOTE 1
#define MAX_LIMIT_NUM_DYENOTE 20

	float detect_res_ary[1];
	float thr_ary[1];
	u8	level = 0;
	
	//検知結果読み込み（1箇所
	detect_res_ary[0] = dy->res_ink_ratio;
	thr_ary[0] = dy->ratio;
	level = level_detect(detect_res_ary , thr_ary ,1 ,MIN_LIMIT_NUM_DYENOTE ,MAX_LIMIT_NUM_DYENOTE);

	return level;

}

//特異点を削除する。
void singularity_exclusion(ST_DYE_NOTE* dy, u8* scan_location, u16 start_offset, u8 side)
{
	s32 i = 0;

	for (i = 1; i < dy->record_count - start_offset - 1; ++i)
	{																								//スキャン結果配列をみて、
		if ((scan_location[i - 1] == 0 && scan_location[i] == 2 && scan_location[i + 1] == 0))		//「0,2,2」の場合はマスクする
		{
			if (side == dynote_side_up || side == dynote_side_down)
			{
				dy->record_y[start_offset + i] = dy->record_y[start_offset + i - 1];				//1つ前のｙ座標を使う。
			}
			else
			{
				dy->record_x[start_offset + i] = dy->record_x[start_offset + i - 1];				//1つ前のx座標を使う。
			}
		}
	}
	
}
#ifdef VS_DEBUG
//Functions of debugging
//Writing for circle of ink boundary
#if DYE_NOTE_VIEWER_MODE
void debug_circle_write(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, s32 x, s32 y, s32 limit_max, s32 limit_min , u8 XorY, u8 side, s32 base_coordinate, u32* recode_num)
#else
void debug_circle_write(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, s32 x, s32 y, s32 limit_max, s32 limit_min, u8 XorY)
#endif
{
	s32 tmpx, tmpy;

	if (limit_max != 0 && limit_min != 0)
	{
		limit_max = limit_max + 10;
		limit_min = limit_min - 10;

		if ((limit_max < x || limit_min > x) && XorY == 0)
		{
			return;
		}

		if ((limit_max < y || limit_min > y) && XorY == 1)
		{
			return;
		}

	}

	if (pbs->LEorSE == LE)
	{
		tmpx = x + pnote_param->main_eff_range / 2;	//
		tmpy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - y;	//
	}
	else
	{
		tmpy = pnote_param->main_eff_range / 2 - y;	//
		tmpx = x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2;

	}
#ifdef VS_DEBUG
	if (debug_logi_view == 1)
	{
		deb_para[0] = 4;		// function code
		deb_para[1] = tmpx;
		deb_para[2] = tmpy;	//
		deb_para[3] = 1;		// plane
		callback(deb_para);		// debug
	}
#endif
#if DYE_NOTE_VIEWER_MODE
	switch (side)
	{
	case dynote_side_down:

		if (x > base_coordinate)
		{
			*recode_num -= 1;
		}

		break;
	case dynote_side_right:
		if (y > base_coordinate)
		{
			*recode_num -= 1;
		}
		break;
	case dynote_side_left:
		if (y < base_coordinate)
		{
			*recode_num -= 1;
		}
		break;
	default:
		break;
	}
#endif
}
#endif

//End
