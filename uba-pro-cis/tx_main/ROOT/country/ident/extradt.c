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
* MODEL NAME : 識別共有
* @file		extradt.c
* @brief	ポイント抽出の実装ファイル
* @date		
* @Version	0.2.0
* @updata
*	https://app.box.com/s/ehrd36k55tx0iohrvjr26ilr48ojp6fq
*/
/****************************************************************************/

#define EXT
#include "../common/global.h"
#include <stdlib.h>

#define EXTDT_2BOTHDOT 15.7480315f // 2 / 0.127
#define EXTDT_MASKING_LIMIT 21 //mm
//extern int debug_itool_trace_view;

//************************************************************
//@brief等分割でポイント抽出を行う
//@param in *pv　1ポインタに対するパラメータ
//@param in *de　分割パラメータ
//@param in buf_num　バッファ番号
//@param out　*de->data_extra_out_put 抽出結果の画素値
//return 0
s16 Data_extraction(ST_DATA_EXTRACTION *de ,u8 buf_num)
{
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	ST_POINT_VICINITY pv;

	u16 err_code = 0;
	//u32 point_count = 0;
	s16 y=0;
	s16 x=0;
	u8 plane=0;
	s16 ey = 0;
	s32 note_size_x = pbs->note_x_size;
	s32 note_size_y = pbs->note_y_size;
	s32 temp_note_size_y = 0;
	s8  trans_flg_x = 1;
	s8  trans_flg_y = 1;

	

	ST_SPOINT spt;

	//s16 step_x = 0;
	//s16 step_y = 0;

	s16 end_coordinates_x;
	s16 end_coordinates_y;

	//s32 half_size_x = note_size_x/2;
	//s32 half_size_y = note_size_y/2;


	de->buf_num = buf_num;

	de->count = 0;	/*合計ポイントカウント初期化*/
	spt.way = W0;	
	
	if(de->split_x * de->split_y * de->plane_count >= PT_SIZE_MAX)//配列参照外防止
	{
		return ERR_EXCEEDS_THE_VALUE_OF_THE_SET_PT_SIZE_MAX;
	}

	/*縦流し横流し補正*/
	if(pbs->LEorSE == SE)
	{
		x_y_change(&note_size_x ,&note_size_y);

		ey = de->split_y + 1;

	}

	//側面マスキングエリア対応
	//論理座標系のX方向サイズを意図的に小さくして、側面のエリアを参照しないようにする。
	//side_masking_areaには単位㎜で、1辺あたりのマスキング範囲が競って入れているので、
	//*2したのちdotに変換する
	if (de->side_masking_area < EXTDT_MASKING_LIMIT)
	{
		note_size_x -= (s32)(de->side_masking_area * EXTDT_2BOTHDOT);
	}

	if (de->vertical_masking_area < EXTDT_MASKING_LIMIT)
	{
		if (pbs->LEorSE == SE)
		{
			note_size_y += (s32)(de->vertical_masking_area * EXTDT_2BOTHDOT);
		}
		else
		{
			note_size_y -= (s32)(de->vertical_masking_area * EXTDT_2BOTHDOT);
		}
	}

	//搬送方向補正
	if(pbs->transport_direction == WITHDRAWAL && pbs->LEorSE == SE)
	{
		trans_flg_x = -1;
	}
	else if(pbs->transport_direction == WITHDRAWAL && pbs->LEorSE == LE)
	{
		trans_flg_y = -1;
	}

	temp_note_size_y = abs(note_size_y);

	//分割の間隔を調べる
	end_coordinates_x = (s16)(abs((1) * note_size_x  / (de->split_x+1) - (note_size_x/2)));
	end_coordinates_y = (s16)(-abs(ey - (1)) * note_size_y  / (de->split_y+1) + (note_size_y/2));

	for (plane = 0; plane < de->plane_count; plane++)//プレーン数
	{
		pv.plane = de->plane_lst[plane];	/*用いるプレーンの設定*/
		spt.l_plane_tbl = (enum L_Plane_tbl)pv.plane;

		//マスクサイズエラーチェック
		if((note_size_x * 0.5) < end_coordinates_x + de->filter_size_x[plane] * 0.5 || temp_note_size_y * 0.5 < end_coordinates_y + de->filter_size_y[plane] * 0.5)
		{
			return ERR_MASK_SIZE_IS_LARGE_AND_IT_REFERS_TO_OUTSIDE_OF_AREA;
		}


		for (y = 0; y < de->split_y; y++)		//分割数y
		{
			for (x = 0; x < de->split_x; x++)	//分割数x
			{
				if(0 == de->corner_flg)	// コーナーフラグ確認
				{
					if(((0==x) && (0==y)) || ((de->split_x==x+1) && (de->split_y==y+1)) ||
						((0==x) && (de->split_y==y+1)) || ((de->split_x==x+1) && (0==y)))	// コーナーなら、
					{
						continue;					// 何もしない
					}
				}

				/*初期化*/
				de->data_extra_out_put[de->count] = 0;
				
				//ポイント座標決定
				pv.x = (s16)((x+1) * note_size_x  / (de->split_x+1) - (note_size_x * 0.5f));	//
				pv.y = (s16)(-abs(ey - (y+1)) * note_size_y  / (de->split_y+1) + (note_size_y * 0.5f));	//

				/*論理座標から物理に変換*/
				spt.x = pv.x * trans_flg_x;	//spt.x = -pv.x * trans_flg_x;
				spt.y = pv.y * trans_flg_y;

#ifdef VS_DEBUG_ITOOL
				for_iTool_trace_callback(&spt, pbs, pnote_param, de->filter_size_x[plane], de->filter_size_y[plane], de->pfilter_pat[plane], 0);
#endif
				spt.trace_flg = 1;
				new_L2P_Coordinate(&spt ,pbs, pnote_param);
				//new_P2L_Coordinate(&spt, pbs);
				pv.x = (s16)spt.x;
				pv.y = (s16)spt.y;

				//プレーンごとにマスク情報をセットする
				pv.filter_size_x = de->filter_size_x[plane];		//フィルターサイズ
				pv.filter_size_y = de->filter_size_y[plane];		//
				pv.pfilter_pat = de->pfilter_pat[plane];			//マスクパターンのポインタ設定
				pv.divide_val = de->divide_val[plane];			//割る数の設定

				/*ポイント内計算*/
				point_vicinity_cal(&pv ,de->buf_num);

				/*配列に結果格納*/
				de->data_extra_out_put[de->count] = pv.output;

				/*ポイント有効数カウントアップ*/
				de->count++;
			}
		}
	}
	/*カウント値調整*/
	//de->count--;
	return err_code;
}

//************************************************************
//@brief入力座標を中心にフィルターパターンに従って画素を参照し、平均値を求める。
//      入力座標は物理座標なのでその分処理が速い
//@param in *pv　1ポインタに対するパラメータ
//@param in buf_num　バッファ番号
//@param out　*pv->output 抽出結果の画素値
//return 0
//__inline
#ifdef I_AND_A
s16 point_vicinity_cal(ST_POINT_VICINITY *__restrict pv ,u8 buf_num)
{
	ST_SPOINT spt; /*ピクセル関数に入れる*/
	ST_BS* pbs = work[buf_num].pbs;

	/*フィルタハーフサイズ*/
	u16 filter_half_size_x = pv->filter_size_x >> 1;
	u16 filter_half_size_y = pv->filter_size_y >> 1;

	u32 total=0;	/*積和格納*/
	u32 addresult=0;
	s16 current_y=0;
	s16 current_x=0;
	//s16 msk_y=0;

	u8 mlt=0;

	u16 err_code;

	/*入力論理プレーンを設定*/
	spt.p_plane_tbl = (enum P_Plane_tbl)pv->plane;
	spt.p = 0;
	spt.trace_flg = 0;

	//スタート位置に移動　（物理座標系で計算してる
	pv->x = pv->x - filter_half_size_x; // 左上へ
	pv->y = pv->y - filter_half_size_y; //

	/*ポイント内のセンサーデータとフィルターの要素数で積和演算を行う*/
	for (current_y = 0; current_y < pv->filter_size_y; current_y++)
	{
		for (current_x = 0; current_x < pv->filter_size_x; current_x++)
		{
			/*フィルターの要素数を取得*/
			mlt = *(pv->pfilter_pat + current_y * pv->filter_size_x + current_x);	// pv->pfilter_pat + current_y * pv->filter_size_x はforloopの外に持っていける

			/*0以外なら演算*/
			if(mlt != 0)	
			{
				/*フィルター内位置決定*/
				spt.x = pv->x + current_x;
				spt.y = (pv->y + current_y);										// 	この行もloopの外に持っていける

				//傾き補正なし
				err_code = P_Getdt_8bit_only(&spt ,pbs);//画素値取得
				//err_code = P_Getdt(&spt ,buf_num);//画素値取得

				addresult = spt.sensdt * mlt;/*センサーデータと要素数*/
				total += addresult;
				
			}
		}
	}
	/*出力計算*/
	pv->output = (u16)(total * pv->divide_val);
	//pv->output = (u16)(total / pv->divide_val);
	return 0;
}

#else 
//__inline s16 point_vicinity_cal(ST_POINT_VICINITY *__restrict pv ,u8 buf_num)
s16 point_vicinity_cal(ST_POINT_VICINITY *pv ,u8 buf_num)
{
	ST_SPOINT	spt; /*ピクセル関数に入れる*/
	ST_BS*		pbs;
	u8	*ppixel;
	u8	*pmskdt;
	u8	masksize_x;
	u8	masksize_y;
	u8	x;
	u8	y;
	u32 ofs;
	u32 totaldt = 0;	/*積和格納*/

	spt.p = 0;

	//マスクデータのポインタ、Xサイズ、Yサイズ、をセットする。
	pmskdt			= pv->pfilter_pat;
	masksize_x	= (u8)pv->filter_size_x;
	masksize_y	= (u8)pv->filter_size_y;

	//もらう座標は物理座標抽出エリアの中心座標なのでエリアの左上座標に変換
	pv->x = pv->x - (pv->filter_size_x >> 1);		//フィルタハーフサイズ; //左上へ
	pv->y = pv->y - (pv->filter_size_y >> 1);		//

	//その物理座標のセンサーデータメモリアドレスをセットする。
	pbs = work[buf_num].pbs;
	spt.p_plane_tbl = (enum P_Plane_tbl)pv->plane;		/*入力論理プレーンを設定*/
	//spt.x	= 0;
	//spt.y	= 0;
	spt.x	= pv->x;
	spt.y	= pv->y;
	P_Getdt_8bit_only(&spt ,pbs);
	ppixel	= spt.p;						//	P_Getdt_8bit_only(&spt ,pbs)画素値取得を改造して、アドレスとperiod情報も取れるようにするか、別関数を作るかする。
	ofs		= spt.period;					//あらかじめ副走査方向の加算アドレスからマスクXサイズを引いておく。
	ofs		= ofs - masksize_x;
	/*ポイント内のセンサーデータとフィルターの要素数で積和演算を行う*/

	for(y = 0; y < masksize_y; y++)			// *** 全体の処理速度に大きく影響する部分 ***
	{										//
		for(x = 0; x< masksize_x; x++)		//
		{									//
			totaldt = *ppixel++ * *pmskdt++ + totaldt;	// センサーデータとマスクデータの積　+= を使うより　こっちの方が早い
		}									//
		ppixel = ppixel + ofs;				//
	}										//

	/*出力計算*/
	pv->output = (u16)(totaldt * pv->divide_val);
	return 0;
}


#endif

//************************************************************
//@brief入力座標を中心にフィルターパターンに従って画素を参照し、平均値を求める。
//      入力座標は論理座標なので処理は遅いが精度が良い
//@param in *pv　1ポインタに対するパラメータ
//@param in buf_num　バッファ番号
//@param out　*pv->output 抽出結果の画素値
//return 0
s16 logi_point_vicinity_cal(ST_POINT_VICINITY *pv ,u8 buf_num)
{
	ST_SPOINT spt; /*ピクセル関数に入れる*/
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	ST_BS* pbs = work[buf_num].pbs;
	/*フィルタハーフサイズ*/
	u16 filter_half_size_x = pv->filter_size_x / 2;
	u16 filter_half_size_y = pv->filter_size_y / 2;

	u32 total=0;	/*積和格納*/
	u32 addresult=0;
	s16 current_y=0;
	s16 current_x=0;
//	s16 msk_y=0;

	u8 mlt=0;

	u16 err_code=0;
	spt.way = pbs->insertion_direction;	

	/*入力論理プレーンを設定*/
	spt.l_plane_tbl = (enum L_Plane_tbl)pv->plane;
	spt.p_plane_tbl = (enum P_Plane_tbl)pv->plane;

	//抽出エリアの中心点を中心座標に変換
	pv->x = pv->x - filter_half_size_x; //左上へ
	pv->y = pv->y + filter_half_size_y;// * pv->dpi; //

	

	/*ポイント内のセンサーデータとフィルターの要素数で積和演算を行う*/
	for (current_y = 0; current_y < pv->filter_size_y; current_y++)
	{
		for (current_x = 0; current_x < pv->filter_size_x; current_x++)
		{
			/*フィルターの要素数を取得*/
			mlt = *(pv->pfilter_pat + current_y * pv->filter_size_x + current_x);

			/*0以外なら演算*/
			if(mlt != 0)	
			{
				/*フィルター内位置決定*/
				spt.x = pv->x + current_x;
				spt.y = (pv->y - current_y /** pv->dpi*/);

				//傾き補正あり
				err_code = new_L_Getdt(&spt ,pbs ,pnote_param);//画素値取得
				//spt.sensdt;

				/*エラー有なら即終了*/
				if(err_code != 0)
				{
					return err_code;
				}

				addresult = spt.sensdt * mlt;/*センサーデータと要素数*/
				total += addresult;
				
			}
		}
	}
	/*出力計算*/
	pv->output = (u16)(total / pv->divide_val);

	return 0;
}

#ifndef _RM_
//************************************************************
//@brief設定したマスクパターンのサイズのピッチでポイント抽出を行う。
//		紙幣全体に対して行う処理で、現状使わない予定(19/02/14)
//@param in *pv　1ポインタに対するパラメータ
//@param in *de　分割パラメータ
//@param in buf_num　バッファ番号
//@param out　*de->pitch_mode_out_put 抽出結果の画素値
//return 0
s16 data_extraction_pitch_mode(ST_DATA_EXTRACTION * de, ST_POINT_VICINITY * pv ,u8 buf_num)
{

	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	ST_SPOINT spt;

	u16 err_code = 0;
	s32 y=0;
	s32 x=0;
	s32 xx=0;
	s32 yy=0;

	s16 point_range_x;
	s16 point_range_y;
	s16 note_size_x = 0;
	s16 note_size_y = 0;
	s16 note_halfsize_x = 0;
	s16 note_halfsize_y = 0;

	u8 plane=0;

	spt.way = 0;

	if(pbs->LEorSE == LE)  //LEならば
	{
		note_size_x = pbs->note_x_size;
		note_size_y = pbs->note_y_size;

		if (de->side_masking_area < EXTDT_MASKING_LIMIT)
		{
			note_size_x -= (s32)(de->side_masking_area * EXTDT_2BOTHDOT);
		}

		if (de->vertical_masking_area < EXTDT_MASKING_LIMIT)
		{
			note_size_y -= (s32)(de->vertical_masking_area * EXTDT_2BOTHDOT);
		}

		note_halfsize_x = note_size_x / 2;
		note_halfsize_y = note_size_y / 2;
		//ポイントの座標を求める
		point_range_x = note_halfsize_x - (pv->filter_size_x / 2);
		de->point_count_x = point_range_x / pv->filter_size_x ;
		//point_count_x = point_count_x * 2 + 1; 

		point_range_y = note_halfsize_y - (pv->filter_size_x / 2);
		de->point_count_y = point_range_y / pv->filter_size_x;
		//point_count_y = point_count_y * 2 + 1; 
		//pv->tool_plane = 0;

		for (plane = 0; plane < de->plane_count; plane++)//プレーン数
		{
			//pv->plane = de->plane_lst[plane];	/*用いるプレーンの設定*/
			spt.l_plane_tbl = pv->plane;

			for(x = -de->point_count_x; x <= de->point_count_x; ++x)
			{
				for(y = -de->point_count_y; y <= de->point_count_y; ++y)
				{
					//下記で縮小させた場合ここで引き延ばす
					spt.x = x * pv->filter_size_x;
					spt.y = y * pv->filter_size_x;

					new_L2P_Coordinate(&spt ,pbs ,pnote_param);

					pv->x = (s16)spt.x;
					pv->y = (s16)spt.y;
					pv->plane = spt.p_plane_tbl;
					//MAGの場合(主走査dpiが足りない場合)ここでxマスクサイズを比に応じて伸縮させる。
					point_vicinity_cal(pv, 0);

					de->pitch_mode_out_put[yy][xx] = (u8)pv->output;
					xx++;
				}	
				xx = 0;
				yy++;
			}
			yy = 0;
		}

	}
	else	//SE
	{
		note_size_y = pbs->note_x_size;
		note_size_x = pbs->note_y_size;

		if (de->side_masking_area < EXTDT_MASKING_LIMIT)
		{
			note_size_x -= (s32)(de->side_masking_area * EXTDT_2BOTHDOT);
		}

		if (de->vertical_masking_area < EXTDT_MASKING_LIMIT)
		{
			note_size_y -= (s32)(de->vertical_masking_area * EXTDT_2BOTHDOT);
		}

		note_halfsize_x = note_size_x / 2;
		note_halfsize_y = note_size_y / 2;
		//ポイントの座標を求める
		point_range_x = note_halfsize_x - (pv->filter_size_x / 2);
		de->point_count_x = point_range_x / pv->filter_size_x ;
		//point_count_x = point_count_x * 2 + 1; 

		point_range_y = note_halfsize_y - (pv->filter_size_x / 2);
		de->point_count_y = (point_range_y / pv->filter_size_x);
		//point_count_y = point_count_y * 2 + 1; 

		for (plane = 0; plane < de->plane_count; plane++)//プレーン数
		{
			//pv->plane = data_extraction.plane_lst[plane];
			//pv->plane = de->plane_lst[plane];	/*用いるプレーンの設定*/
			spt.l_plane_tbl = pv->plane;
			//pv->tool_plane = 0;

			//ここ逆 →　直した
			//for(x = de->point_count_x; x >= -de->point_count_x; --x)
			//{
			//	for(y = de->point_count_y; y >= -de->point_count_y; --y)
			//	{
			for(x = -de->point_count_x; x <= de->point_count_x; ++x)
			{
				for(y = -de->point_count_y; y <= de->point_count_y; ++y)
				{

					spt.x = x * pv->filter_size_x;
					spt.y = y * pv->filter_size_x;

					new_L2P_Coordinate(&spt ,pbs ,pnote_param);

					pv->x = (s16)spt.x;
					pv->y = (s16)spt.y;
					pv->plane = spt.p_plane_tbl;

					point_vicinity_cal(pv, 0);

					de->pitch_mode_out_put[yy][xx] = (u8)pv->output;
					xx++;
				}	
				xx = 0;
				yy++;
			}
		}
		yy = 0;
	}

	return err_code;
}
#endif

