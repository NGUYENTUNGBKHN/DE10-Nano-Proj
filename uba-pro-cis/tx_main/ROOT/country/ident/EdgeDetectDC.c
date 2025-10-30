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
 * @file EdgeDetectDC.c
 * @brief 外形検知ルーチン
 * @date 2018/03/14 Created.
 */
/****************************************************************************/

#include	<math.h>
#include    <string.h>
#include    <float.h>
#include	<stdlib.h>
#define EXT
#include "../common/global.h"

// This is pixel distance between the checks for the edge.
// Note that this value was set to 15 base on Tear checking. Tears that are
// 4mm long need to be checked. This value of 15 should find those tears


// This is the minimum number non-rejected points for a given side required to pass
#define MINIMUM_NUMBERS_OF_POINTS_DC   3

// Once an edge is found, four more pixels (NUM_ADDITIONAL_PIXEL_TEST) beyond the
// found edge will be checked to make certain it is a bill edge and not corrupt data
#define NUM_ADDITIONAL_PIXEL_TEST_DC   4

#define OVERALL_SAFE_DEGREE_VAL_DC		43		//最大スキュー	不要

#define ONE_SIDE_SAFE_DEGREE_VAL_DC	12		//各辺の傾き(台形になった時など)の許容範囲

#define SIDE_OUT_AREA_RANGE_DC	1				//側面からこの値内にエッジがあった場合右か左に寄りすぎとしてエラーとする。





s16	OPdet_Start_DC(u8 buf_num)
{

	ST_BS* pbs = work[buf_num].pbs;

	u16 EdgeDetectionThreshold;
	u16 status = 0; /*エラーステータス*/

	s32 HalfLE;
	s32 HalfSE;
	float TopBottomDiff;
	//u32 threshold = 0; // スレッシュレベルの最大値制限

	Eg_det_ClearData_DC( );							//初期化
	Eg_status_change_DC(pbs);	//解像度（dpi）に従ってステータスを変更します。

	//閾値決めてるとこ
	if(ED_DC.th_dynamic_decision_flg == 1)
	{
		//EdgeDetectionThreshold = getEdgeDetectionThreshold((s16)threshold ,pbs);
		EdgeDetectionThreshold = line_chk_DC(ED_DC.Sub_scan_min_val + 5 ,pbs);
	}
	else
	{
		EdgeDetectionThreshold = 0;	//255反転している
	}
	status = 0;
	//Main_scan_max_valはmainの最大ピクセル数、mMinSubScanLinesはsubの最大ピクセル数
	HalfLE = ED_DC.Main_scan_max_val / 2;
	HalfSE = ED_DC.Sub_scan_max_val / 2;


	// 上端検知
	status |= DetectTopEdge_DC( HalfLE,		//xs
		ED_DC.Sub_scan_min_val,			//ys 0
		HalfLE,			//xe
		HalfSE + 20,	//ye //+20 to increase the search height
		(u16)(ED_DC.Step_Movement / pbs->PlaneInfo[ED_DC.Plane].main_element_pitch),//ステップ幅
		EdgeDetectionThreshold  
		,pbs);
	if ( status != 0 )
	{
		return status;
	}

	//下端
	status |= DetectBottomEdge_DC( HalfLE + 2,		// 上端の探索痕跡とデバッグ時に区別しやすいので+2している
		(ED_DC.Sub_scan_max_val-1),//ブロック数拾ってきている200dpi仕様 
		HalfLE,//100だったのでハーフに変更しました
		HalfSE,//HalfSE - 20,//-20 to increase the search height // -20 検索の高さを上げるために20
		(u16)(ED_DC.Step_Movement /pbs->PlaneInfo[ED_DC.Plane].main_element_pitch),
		EdgeDetectionThreshold ,pbs);
	if ( status != 0 )
	{
		return status;
	}

	//左端
	status |= DetectLeftEdge_DC( ED_DC.Main_scan_min_val+1,
		(int)(ED_DC.Top_Edge_offs + 15.0f),			//上端y切片
		HalfLE + 60,								//+40 to increase the search length
		(int)(ED_DC.Bottom_Edge_offs),					//下端y切片
		(u16)(ED_DC.Step_Movement / (25.4/(pbs->Subblock_dpi / pbs->PlaneInfo[ED_DC.Plane].sub_sampling_pitch))),
		EdgeDetectionThreshold ,pbs);
	if ( status != 0 )
	{
		return status;
	}

	//右端
	status |= DetectRightEdge_DC( ED_DC.Main_scan_max_val,
		(int)(ED_DC.Top_Edge_offs + (ED_DC.Main_scan_max_val * ED_DC.Top_Edge_tan_t) + 0) -10,
		HalfLE,//-40 to increase the search length
		(int)(ED_DC.Bottom_Edge_offs + ED_DC.Main_scan_max_val * ED_DC.Bottom_Edge_tan_t) + 0,
		(u16)(ED_DC.Step_Movement / (25.4/(pbs->Subblock_dpi / pbs->PlaneInfo[ED_DC.Plane].sub_sampling_pitch))),
		EdgeDetectionThreshold ,pbs);
	if ( status != 0 )
	{
		return status;
	}

	TopBottomDiff =( ED_DC.Top_Edge_tan_t - ED_DC.Bottom_Edge_tan_t)* ED_DC.dpi_Correction;	//上辺と下辺の傾きの差分

	if ( fabs(TopBottomDiff) > EDGE_DIFF_LIMIT_DC ) //差分絶対値が閾値よりも大きい？
	{
		float TopLeftAdd = (ED_DC.Top_Edge_tan_t* ED_DC.dpi_Correction)+ (ED_DC.Left_Edge_tan_t / ED_DC.dpi_Correction);	//上辺と左辺の傾きの差分を計算

		if ( fabs(TopLeftAdd) > EDGE_DIFF_LIMIT_DC )	
		{
			// Top Edge is bad
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Top_Edge_tan_t, Top_Edge_offs, 2 );
			FixEdge_DC( &ED_DC.Top_Edge_tan_t, &ED_DC.Top_Edge_sin_t, &ED_DC.Top_Edge_cos_t, &ED_DC.Top_Edge_Deg, &ED_DC.Top_Edge_offs, ED_DC.Edge_Num_Points[TopEdgeDC],
				ED_DC.Edge_xpt[TopEdgeDC], ED_DC.Edge_ypt[TopEdgeDC], ED_DC.Bottom_Edge_tan_t, ED_DC.Bottom_Edge_sin_t, ED_DC.Bottom_Edge_cos_t, ED_DC.Bottom_Edge_Deg,
				ED_DC.Bottom_Edge_offs, -1 ,ED_DC.Edge_xpt[BottomEdgeDC],ED_DC.Edge_ypt[BottomEdgeDC] );  // -1 -> top line is in the negative direction from bottom line
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Top_Edge_tan_t, Top_Edge_offs, 0xFF );
		}
		else
		{
			// Bottom Edge is bad
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Bottom_Edge_tan_t, Bottom_Edge_offs, 2 );
			FixEdge_DC( &ED_DC.Bottom_Edge_tan_t, &ED_DC.Bottom_Edge_sin_t, &ED_DC.Bottom_Edge_cos_t, &ED_DC.Bottom_Edge_Deg, &ED_DC.Bottom_Edge_offs,
				ED_DC.Edge_Num_Points[BottomEdgeDC], ED_DC.Edge_xpt[BottomEdgeDC], ED_DC.Edge_ypt[BottomEdgeDC], ED_DC.Top_Edge_tan_t, ED_DC.Top_Edge_sin_t, ED_DC.Top_Edge_cos_t,
				ED_DC.Top_Edge_Deg, ED_DC.Top_Edge_offs, 1,ED_DC.Edge_xpt[TopEdgeDC],ED_DC.Edge_ypt[TopEdgeDC]);  // 1 -> bottom line is in the positive direction from top line
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Bottom_Edge_tan_t, Bottom_Edge_offs, 0xFF );
		}
	}

	else
	{
		float LeftRightDiff =(ED_DC. Left_Edge_tan_t -ED_DC. Right_Edge_tan_t)/ ED_DC.dpi_Correction;	//左辺と右辺の差分を計算
		if ( fabs(LeftRightDiff) > EDGE_DIFF_LIMIT_DC)
		{
			float TopLeftAdd = ED_DC.Top_Edge_tan_t* ED_DC.dpi_Correction  + ED_DC.Left_Edge_tan_t/ ED_DC.dpi_Correction;	//上辺と左辺の差分を計算
			if ( fabs(TopLeftAdd) > EDGE_DIFF_LIMIT_DC)
			{
				// Left Edge is bad
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Left_Edge_tan_t, Left_Edge_offs, 2 );
				FixEdge_DC( &ED_DC.Left_Edge_tan_t, &ED_DC.Left_Edge_sin_t, &ED_DC.Left_Edge_cos_t, &ED_DC.Left_Edge_Deg, &ED_DC.Left_Edge_offs,
					ED_DC.Edge_Num_Points[LeftEdgeDC], ED_DC.Edge_ypt[LeftEdgeDC], ED_DC.Edge_xpt[LeftEdgeDC], ED_DC.Right_Edge_tan_t, ED_DC.Right_Edge_sin_t,
					ED_DC.Right_Edge_cos_t, ED_DC.Right_Edge_Deg, ED_DC.Right_Edge_offs, -1,ED_DC.Edge_ypt[RightEdgeDC],ED_DC.Edge_xpt[RightEdgeDC] );
				// -1 -> left line is in the negative direction from right line
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Left_Edge_tan_t, Left_Edge_offs, 0xFF );
			}
			else
			{
				// Right edge is bad
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Right_Edge_tan_t, Right_Edge_offs, 2 );
				FixEdge_DC( &ED_DC.Right_Edge_tan_t, &ED_DC.Right_Edge_sin_t, &ED_DC.Right_Edge_cos_t, &ED_DC.Right_Edge_Deg, &ED_DC.Right_Edge_offs,
					ED_DC.Edge_Num_Points[RightEdgeDC], ED_DC.Edge_ypt[RightEdgeDC], ED_DC.Edge_xpt[RightEdgeDC], ED_DC.Left_Edge_tan_t, ED_DC.Left_Edge_sin_t,
					ED_DC.Left_Edge_cos_t, ED_DC.Left_Edge_Deg, ED_DC.Left_Edge_offs, 1,ED_DC.Edge_ypt[LeftEdgeDC],ED_DC.Edge_xpt[LeftEdgeDC] );
				// 1 -> right line is in the positive direction from left line
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Right_Edge_tan_t, Right_Edge_offs, 0xFF );
			}
		}
	}

	// Vertex coordinate calculation:頂点座標計算
	ED_DC.AlignedUpperLeft = calcu_xoyo_DC( ED_DC.Left_Edge_tan_t, ED_DC.Left_Edge_offs, ED_DC.Top_Edge_tan_t, ED_DC.Top_Edge_offs ,pbs);// Upper Left
	ED_DC.AlignedLowerLeft = calcu_xoyo_DC( ED_DC.Left_Edge_tan_t, ED_DC.Left_Edge_offs, ED_DC.Bottom_Edge_tan_t, ED_DC.Bottom_Edge_offs ,pbs);// Lower Left
	ED_DC.AlignedUpperRight = calcu_xoyo_DC( ED_DC.Right_Edge_tan_t, ED_DC.Right_Edge_offs, ED_DC.Top_Edge_tan_t, ED_DC.Top_Edge_offs ,pbs);// Upper Right
	ED_DC.AlignedLowerRight = calcu_xoyo_DC( ED_DC.Right_Edge_tan_t, ED_DC.Right_Edge_offs, ED_DC.Bottom_Edge_tan_t,ED_DC. Bottom_Edge_offs ,pbs); // Lower Right

#ifdef NOT_VIEW_MODE
	//頂点座標が搬送エリア外だった場合処理を終了する
	if( ED_DC.AlignedUpperLeft.x < ED_DC.Main_scan_min_val + SIDE_OUT_AREA_RANGE ||
		ED_DC.AlignedLowerLeft.x < ED_DC.Main_scan_min_val + SIDE_OUT_AREA_RANGE)
	{
		return ERR_TO_THE_LEFT;
	}
	
	if( ED_DC.AlignedUpperRight.x > ED_DC.Main_scan_max_val - SIDE_OUT_AREA_RANGE ||
		ED_DC.AlignedLowerRight.x > ED_DC.Main_scan_max_val - SIDE_OUT_AREA_RANGE)
	{
		return ERR_TO_THE_RIGHT;
	}

	if( ED_DC.AlignedUpperLeft.y < ED_DC.Sub_scan_min_val + SIDE_OUT_AREA_RANGE ||
		ED_DC.AlignedUpperRight.y < ED_DC.Sub_scan_min_val + SIDE_OUT_AREA_RANGE)
	{
		return ERR_TO_THE_UPPER;
	}

	if( ED_DC.AlignedLowerLeft.y > (ED_DC.Sub_scan_max_val - SIDE_OUT_AREA_RANGE) * ED_DC.dpi_Correction ||
		ED_DC.AlignedLowerRight.y > (ED_DC.Sub_scan_max_val - SIDE_OUT_AREA_RANGE) * ED_DC.dpi_Correction)
	{
		return ERR_TO_THE_LOWER;
	}
#endif

	//出力計算(主幅、副幅、傾き、中心座標)
	calcu_size_DC( ED_DC.AlignedUpperLeft.x, ED_DC.AlignedUpperLeft.y, ED_DC.AlignedLowerLeft.x,ED_DC.AlignedLowerLeft.y, ED_DC.AlignedUpperRight.x,
		ED_DC.AlignedUpperRight.y, ED_DC.AlignedLowerRight.x, ED_DC.AlignedLowerRight.y ,pbs->LEorSE);


	ED_DC.length_dot = (int)ED_DC.mBillLEPixelLength;
	ED_DC.width_dot = (int)ED_DC.mBillSEPixelLength; // pbs->pitch_ratio;
	ED_DC.skew = ED_DC._deg;
	ED_DC.tan_th = ED_DC._tan_t;
	ED_DC.sin_th = ED_DC._sin_t;
	ED_DC.cos_th = ED_DC._cos_t;//

	ED_DC.AlignedBillCenterPoint.x = ED_DC._o.x;

	//求めた中心座標yに対して　dpi&offset補正を与える
	ED_DC.AlignedBillCenterPoint.y = (ED_DC._o.y); // pbs->pitch_ratio;

	// Slope validation of the four sides
	//１辺の傾きの差の整合性
	status |= Edg_Deg_def_chk_DC( );
	if ( status != 0 )
	{
		return status;
	}

	//傾きの整合性
	status |= deg_chk_DC( );
	if ( status != 0 )
	{
		return status;
	}

	ED_DC.SKEW = rndup(ED_DC.tan_th * 4096.0f);

	return status;

}

void SetSkew_DC( short Skew )
{
	ED_DC.mResults_skew_deg = Skew;
}

void SetWidth_DC( unsigned short Width )
{
	ED_DC.mResults_width = (s8)Width;
}

void SetHeight_DC( unsigned short Height )
{
	ED_DC.mResults_height = (s8)Height;
}

void Eg_det_ClearData_DC( )
{

	ED_DC.skew = 0.0f;
	ED_DC.tan_th = 0.0f;
	ED_DC.sin_th = 0.0f;
	ED_DC.cos_th = 0.0f;

	ED_DC.length_dot = 0;
	ED_DC.width_dot = 0;

	ED_DC.AlignedBillCenterPoint.x = 0;
	ED_DC.AlignedBillCenterPoint.y = 0;

	ED_DC.length_dot = 0;
	ED_DC.width_dot = 0;

	ED_DC.AlignedBillCenterPoint.x = 0;
	ED_DC.AlignedBillCenterPoint.y = 0;

	ED_DC.length_mm = 0.0f;
	ED_DC.width_mm = 0.0f;
	ED_DC.adj_skew = 0.0f;

	ED_DC.ofs_dbld_L1 = 0.0f;
	ED_DC.ofs_dbld_L2 = 0.0f;

	ED_DC.AlignedUpperLeft.x = 0;
	ED_DC.AlignedUpperLeft.y = 0;

	ED_DC.AlignedLowerRight.x = 0;
	ED_DC.AlignedLowerRight.y = 0;

	ED_DC.AlignedLowerLeft.x = 0;
	ED_DC.AlignedLowerLeft.y = 0;

	ED_DC.AlignedLowerRight.x = 0;
	ED_DC.AlignedLowerRight.y = 0;

	ED_DC.Top_Edge_Deg = 0.0f;
	ED_DC.Bottom_Edge_Deg = 0.0f;
	ED_DC.Left_Edge_Deg = 0.0f;
	ED_DC.Right_Edge_Deg = 0.0f;

	ED_DC.tan_t = 0.0f;
	ED_DC.sin_t = 0.0f;
	ED_DC.cos_t = 0.0f;
	ED_DC.offs = 0.0f;

	ED_DC.Top_Edge_tan_t = 0.0f;
	ED_DC.Top_Edge_sin_t = 0.0f;
	ED_DC.Top_Edge_cos_t = 0.0f;
	ED_DC.Top_Edge_offs = 0.0f;

	ED_DC.Bottom_Edge_tan_t = 0.0f;
	ED_DC.Bottom_Edge_sin_t = 0.0f;
	ED_DC.Bottom_Edge_cos_t = 0.0f;
	ED_DC.Bottom_Edge_offs = 0.0f;

	ED_DC.Left_Edge_tan_t = 0.0f;
	ED_DC.Left_Edge_sin_t = 0.0f;
	ED_DC.Left_Edge_cos_t = 0.0f;
	ED_DC.Left_Edge_offs = 0.0f;

	ED_DC.Right_Edge_tan_t = 0.0f;
	ED_DC.Right_Edge_sin_t = 0.0f;
	ED_DC.Right_Edge_cos_t = 0.0f;
	ED_DC.Right_Edge_offs = 0.0f;

	ED_DC.TopLeftCornerX = 0.0f;
	ED_DC.TopLeftCornerY = 0.0f;
	ED_DC.BottomLeftCornerX = 0.0f;
	ED_DC.BottomLeftCornerY = 0.0f;
	ED_DC.TopRightCornerX = 0.0f;
	ED_DC.TopRightCornerY = 0.0f;
	ED_DC.BottomRightCornerX = 0.0f;
	ED_DC.BottomRightCornerY = 0.0f;
	ED_DC._o.x = 0;
	ED_DC._o.y = 0;
	ED_DC.mBillSEPixelLength = 0.0f;
	ED_DC.mBillLEPixelLength = 0.0f;
	ED_DC._deg = 0.0f;
	ED_DC._tan_t = 0.0f;
	ED_DC._sin_t = 0.0f;
	ED_DC._cos_t = 0.0f;
	ED_DC.SKEW =0;

	memset( ED_DC.Edge_xpt, 0, sizeof( ED_DC.Edge_xpt ) );
	memset( ED_DC.Edge_ypt, 0, sizeof( ED_DC.Edge_ypt ) );
	memset( ED_DC.Edge_Num_Points, 0, sizeof( ED_DC.Edge_Num_Points ) );
}

/*
//                 _______            ______    _
//                |__   __|          |  ____|  | |
//                   | | ___  _ __   | |__   __| | __ _  ___
//                   | |/ _ \| '_ \  |  __| / _` |/ _` |/ _ \
//                   | | (_) | |_) | | |___| (_| | (_| |  __/
//                   |_|\___/| .__/  |______\__,_|\__, |\___|
//                           | |                   __/ |
//                           |_|                  |___/
//http://www.network-science.de/ascii/   (big Font)
*/

// xStart = X Position Start Value
// yStart = Y Position Start Value
// xHalfLength = Distnace in one direction to travel for edge detection ( xHalfLength * 2 should be greater than length of bill)
// ye = Y Position End Value
// step = number of pixels to step for next scan to the right
// FindThreshold = threshold for detecting the edge

// xStart = X位置の開始値
// yStart = Y位置開始値
// xHalfLength =エッジ検出のために移動する一方向の距離（xHalfLength * 2は札長より大きくする必要があります）
// ye = Y位置終了値
// step =次のスキャンを右に進めるためのピクセル数
// FindThreshold =エッジを検出するためのしきい値

/*
Start
(xStart, yStart)
.
|
|
(xStart - xStep, yss) |  (xStart + xStep, yss)
.          |          .
|<- xStep->|<- xStep->|<- xStep->|<- xStep->|
|          |          |          |          |
|          |          |          |          |
----------------------------------------------------     |
| $                                              $ |     |
|                                                  |     |
|                      o   o                       |     |
|                                                  |  (xStart + xHalfLength, yEnd)
|                      \___/                       |    End
|                                                  |
| $                                              $ |
----------------------------------------------------
*/
u16 DetectTopEdge_DC( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold ,ST_BS *pbs)
{

	int n = 0;
	int ErrorCount = 0;
	int ErrorCountMax = 10; // 探査エリアY方向開始位置で初めから券有を検出した時のエラーとする回数


	int yss;// = yStart;
	int Index =	MAX_POINTS_DC / 2;//　50 / 2
	int i;
	int x;
	u16 Status;
	int FirstValue = 0;




	// 探索範囲内で境目を探す
	yss = yStart;
	for ( x = xStart; x < xStart + xHalfLength; x += xStep )	// 右側半分の探査
	{
		if ( yss < yStart )		// 上端がイメージデータの上端近辺にあるとき、次の開始位置が　yStart位置より前になりイメージデータ外になる可能性があるためyssをここで制限している
		{
			yss = yStart;
		}

		Status = DetectTopEdgeYLoop_DC(x,
			&yss,	// Y 開始座標
			yEnd,	// Y 終了座標
			&n,
			FindThreshold,
			&ED_DC.Edge_xpt[TopEdgeDC][Index],
			&ED_DC.Edge_ypt[TopEdgeDC][Index],
			&ErrorCount, ErrorCountMax
			,pbs);
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND_DC )
			{
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				//ポイントを見つけられなかった場合は、札のx方向の最後にあると仮定します
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON_DC )
			{
				// If we found the edge too soon, we may need to rescan from the yStart Point
				//エッジが見つからない場合は、yStart Pointから再スキャンする必要があるかもしれません
				if ( yss > yStart )
				{
					ErrorCount--;
					yss = yStart;
					x -= xStep;		// もう一度同じラインでY開始位置を最初に戻したところからスキャンする。
					continue;
				}
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yStart, false );
				break;
			}
			return Status;
		}
		if ( Index >= MAX_POINTS_DC-1 )
		{
			break;
		}
		Index++;
	}

	yss = (int)ED_DC.Edge_ypt[TopEdgeDC][MAX_POINTS_DC / 2] - ED_DC.Small_Backtrack;	// 前の結果を使って算出
	//    yss = (int)Edge_ypt[TopEdgeDC][0] - ED_DC.Small_Backtrack;	// 前の結果を使って算出
	Index = MAX_POINTS_DC / 2 - 1;
	for ( x = xStart - xStep; x > xStart - xHalfLength; x -= xStep )	// 左側半分の探査
	{
		if ( yss < yStart )
		{
			yss = yStart;
		}

		Status = DetectTopEdgeYLoop_DC( x, &yss, yEnd, &n, FindThreshold, &ED_DC.Edge_xpt[TopEdgeDC][Index], &ED_DC.Edge_ypt[TopEdgeDC][Index], &ErrorCount, ErrorCountMax ,pbs );
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND_DC )
			{
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON_DC )
			{
				// If we found the edge too soon, we may need to rescan from the yStart Point
				if ( yss > yStart )
				{
					ErrorCount--;
					yss = yStart;
					x += xStep;
					continue;
				}
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yStart, false );
				break;
			}
			return Status;
		}
		//       if ( Index >= MAX_POINTS_DC )
		if ( Index <= 0 )
		{
			break;
		}
		Index--;
	}

	ED_DC.ScanEdgeNumPoints[TopEdgeDC] = n;	// 券端位置リストの外れ値削除前の有効数のセット
	ED_DC.Edge_Num_Points[TopEdgeDC] = n;	// 券端位置リストの有効数のセット

	// Because the Index of Edge_xpt and Edge_ypt starts in the middle, the first values should be zero.
	// Find the first real value, and copy from that point of the array to the beginning of the array
	// Edge_xptとEdge_yptのインデックスが途中から始まるため、最初の値はゼロになるはずです。
	//最初の実数値を見つけ、配列のその点から配列の先頭にコピーする
	FirstValue = 0;
	for ( i = 0; i < MAX_POINTS_DC; i++ )
	{
		if ( ED_DC.Edge_xpt[TopEdgeDC][i] != 0 )
		{
			FirstValue = i;
			break;
		}
	}
	memmove( ED_DC.Edge_xpt[TopEdgeDC], &ED_DC.Edge_xpt[TopEdgeDC][FirstValue], sizeof( ED_DC.Edge_xpt[TopEdgeDC][0] ) * ( MAX_POINTS_DC - FirstValue ) );	// 配列内データを前に詰める
	memmove( ED_DC.Edge_ypt[TopEdgeDC], &ED_DC.Edge_ypt[TopEdgeDC][FirstValue], sizeof( ED_DC.Edge_ypt[TopEdgeDC][0] ) * ( MAX_POINTS_DC - FirstValue ) );	// 配列内データを前に詰める

	memcpy( ED_DC.ScanEdgeXPoints[TopEdgeDC], ED_DC.Edge_xpt[TopEdgeDC], sizeof(ED_DC. Edge_xpt[TopEdgeDC][0] ) * MAX_POINTS_DC );	// 外れ値排除前のデータを保存　MAXよりｎの方がいいのでは？
	memcpy( ED_DC.ScanEdgeYPoints[TopEdgeDC], ED_DC.Edge_ypt[TopEdgeDC], sizeof(ED_DC. Edge_ypt[TopEdgeDC][0] ) * MAX_POINTS_DC );	// 外れ値排除前のデータを保存

	SortAndRemoveOutliers_DC( ED_DC.Edge_ypt[TopEdgeDC], ED_DC.Edge_xpt[TopEdgeDC], 1/*true*/, &ED_DC.Edge_Num_Points[TopEdgeDC] ,1);

	if (ED_DC. Edge_Num_Points[TopEdgeDC] < MINIMUM_NUMBERS_OF_POINTS_DC )
	{
		SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Few_Points_DC, 0 );
		return ERR_TOO_FEW_EDGE_POINTS_FOUND_DC;
	}

	mask_DC(ED_DC. Edge_xpt[TopEdgeDC], ED_DC.Edge_ypt[TopEdgeDC], ED_DC.Edge_Num_Points[TopEdgeDC] ,pbs);

	ED_DC.Top_Edge_tan_t = ED_DC.tan_t;
	ED_DC.Top_Edge_sin_t = ED_DC.sin_t;
	ED_DC.Top_Edge_cos_t = ED_DC.cos_t;
	ED_DC.Top_Edge_offs = ED_DC.offs + 1;

	ED_DC.Top_Edge_Deg = (float)(atan( ED_DC.Top_Edge_tan_t  * ED_DC.dpi_Correction) * PIRAD_DC);

	return 0;
}

// handles edge checking in the y direction, going up (DetectBottomEdge) or down (DetectTopEdge)
unsigned short DetectTopEdgeYLoop_DC(
	int x,						// The x offset for this line　
	// Yスキャンを行うX座標
	int *yStart,				// The starting point for scanning. This is passed back since the next scan line is based on the detection of this scan line
	// スキャンの開始点。 これは、次の走査線のために変更される
	int yEnd,					// The last point to look for an edge
	// エッジを探す最後のポイント
	int *EdgePointFoundCount,	// Running total of the number of edges found
	// 見つかった券端の数で変更される
	int FindThreshold,			// Pixel value threshold for identifying the edge
	// エッジを識別するためのピクセル値のしきい値
	float *XPoint,				// X from List of (X,Y) points where the edge was detected
	// エッジが検出された（X、Y）点のXのリスト
	float *YPoint,				// Y from List of (X,Y) points where the edge was detected
	// エッジが検出された（X、Y）点のYのリスト
	int *ErrorCount,			// Running total of edge starting too soon errors
	// あまりにも早い時期に開始するエッジの合計の実行
	int ErrorCountMax,			// Maximum number of ErrorCount before an actual error is thrown
	// 実際のエラーがスローされる前のErrorCountの最大数
	ST_BS *pbs)
{
	unsigned short Result = WARN_EDGE_NOT_FOUND_DC;
	//    T_Point AlignedPoint;
	//    T_Point FollowUpTest;
	int LastPixelValue = 0;
	const int yStep = 1;
	ST_SPOINT AlignedPoint;
	ST_SPOINT FollowUpTest;

	//プレーンだけセット
	AlignedPoint.p_plane_tbl = ED_DC.Plane;
	FollowUpTest.p_plane_tbl = ED_DC.Plane;


	AlignedPoint.x = x;
	for ( AlignedPoint.y = *yStart; AlignedPoint.y < yEnd; AlignedPoint.y += yStep )
	{
		int PixelValue = ED_Pget_DC(AlignedPoint ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  AlignedPoint );
		if ( PixelValue > FindThreshold )	// 券有りレベル検出の時				
		{
			// Test next few pixels to ensure we are not looking at corrupted data
			//次の数ピクセルをテストして、破損したデータを確認しないようにする
			u8 FalseEdgeDetected = 0;
			FollowUpTest.x = AlignedPoint.x;
			for ( FollowUpTest.y = AlignedPoint.y + yStep; ( FollowUpTest.y <= AlignedPoint.y + ( yStep * NUM_ADDITIONAL_PIXEL_TEST_DC ) ) && ( FollowUpTest.y < yEnd );
				FollowUpTest.y += yStep )
			{
				int NewPixelValue = ED_Pget_DC(FollowUpTest ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  FollowUpTest );
				if ( NewPixelValue <= FindThreshold )
				{
					FalseEdgeDetected = 1;//true	// 券無が見つかったとき、
					break;							// ループを抜ける
				}
			}

			if ( FalseEdgeDetected )			// 券無が見つかったとき
			{
				LastPixelValue = PixelValue;	// その時のピクセルのデータを保存する
				continue;
			}

			// さらに数ドット探査して券無が見つからなかったとき（正常）
			if ( AlignedPoint.y == *yStart )	//しょっぱなから券有りのとき
			{
				Result = WARN_EDGE_FOUND_TOO_SOON_DC;//WARN EDGEがすぐに見つかりました。
				*ErrorCount += 1;
				break;
			}
			// Found a point
			//            SIM_INTERFACE_TraceAlignedLine( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, x, yStart, x, AlignedPoint.y, 0xFF );

			// Scales the pixel point back to a float value
			// スレッシュポイントの隣接濃度比率からポイントの位置を実数値で算出する
			*YPoint = AlignedPoint.y - ( PixelValue - FindThreshold ) * yStep / ( ( float )( PixelValue - LastPixelValue ) );
			*XPoint = (float)AlignedPoint.x;

			// For the next round start 10 pixels above the detection point of this scan
			// 次のラウンドでは、この走査の検出点のED.Small_Backtrackピクセル上から開始する
			*yStart = AlignedPoint.y - ED_DC.Small_Backtrack;
			if ( ++*EdgePointFoundCount >= MAX_POINTS_DC )
			{
				// err
				SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Many_Points_DC, 0);
				return ERR_TOO_MANY_EDGE_POINTS_FOUND_DC;	// MAX_POINTS_DC 以上券端が見つかったときは、配列容量を超えるのでエラーとする。
			}
			return 0;
		}
		LastPixelValue = PixelValue;
	}
	//Failed to find the point
	//ポイントを見つけることができませんでした
	//    SIM_INTERFACE_TraceAlignedLine( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, x, yStart, x, AlignedPoint.y, 0xFF );
	if ( *ErrorCount >= ErrorCountMax )
	{
		// Too many points are on the edge of the test range.
		//テスト範囲の端に点が多すぎりとき。
		//SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_On_Test_Edge, 0 );
		return ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA_DC;
	}
	return Result;
}



/*        ____        _   _                    ______    _
//       |  _ \      | | | |                  |  ____|  | |
//       | |_) | ___ | |_| |_ ___  _ __ ___   | |__   __| | __ _  ___
//       |  _ < / _ \| __| __/ _ \| '_ ` _ \  |  __| / _` |/ _` |/ _ \
//       | |_) | (_) | |_| || (_) | | | | | | | |___| (_| | (_| |  __/
//       |____/ \___/ \__|\__\___/|_| |_| |_| |______\__,_|\__, |\___|
//                                                          __/ |
//                                                         |___/
//http://www.network-science.de/ascii/   (big Font) */

// xs = X Position End
// ys = Y Position Start
// xe = X Position Start
// ye = Y Position End
// step = number of pixels to step for next scan to the right
// FindThreshold = threshold for detecting the edge
/*
----------------------------------------------------
| $                                              $ |
|                                                  |
|                      o   o                       |
|                                                  |   End
|                      -----                       |  (xStart + xLength, yEnd)
|                                                  |     |
| $                                              $ |     |
----------------------------------------------------     |
|          |          |          |          |
|          |          |          |          |
|<- step ->|<- step ->|<- step ->|<- step ->|
|  (xStart + step, yss)
|
|
(xStart, yStart)
Start
*/
unsigned short DetectBottomEdge_DC( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold ,ST_BS *pbs)
{
	int n = 0;
	int ErrorCount = 0, ErrorCountMax = 10;

	int yss = yStart;
	int Index = MAX_POINTS_DC / 2;
	int x;
	unsigned short Status;
	int FirstValue = 0;
	int i;

	for ( x = xStart; x < xStart + xHalfLength; x += xStep )
	{
		if ( yss > yStart )
		{
			yss = yStart;
		}

		Status = DetectBottomEdgeYLoop_DC( x, &yss, yEnd, &n, FindThreshold, &ED_DC.Edge_xpt[BottomEdgeDC][Index], &ED_DC.Edge_ypt[BottomEdgeDC][Index], &ErrorCount, ErrorCountMax ,pbs);
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND_DC )
			{
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON_DC )
			{
				// If we found the edge too soon, we may need to rescan from the yStart Point
				if ( yss < yStart )
				{
					ErrorCount--;
					yss = yStart;
					x -= xStep;
					continue;
				}
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yStart, false );
				break;
			}
			return Status;
		}
		Index++;
		if ( Index >= MAX_POINTS_DC )
		{
			break;
		}
	}

	yss = (int)ED_DC.Edge_ypt[BottomEdgeDC][MAX_POINTS_DC / 2] + ED_DC.Small_Backtrack;
	//    yss = (int)Edge_ypt[BottomEdgeDC][0] + ED_DC.Small_Backtrack;   // 前の結果を使って算出
	Index = MAX_POINTS_DC / 2 - 1;
	for ( x = xStart - xStep; x > xStart - xHalfLength; x -= xStep )
	{
		if ( yss > yStart )
		{
			yss = yStart;
		}

		Status = DetectBottomEdgeYLoop_DC( x, &yss, yEnd, &n, FindThreshold, &ED_DC.Edge_xpt[BottomEdgeDC][Index], &ED_DC.Edge_ypt[BottomEdgeDC][Index], &ErrorCount, ErrorCountMax ,pbs);
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND_DC )
			{
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON_DC )
			{
				// If we found the edge too soon, we may need to rescan from the yStart Point
				if ( yss < yStart )
				{
					ErrorCount--;
					yss = yStart;
					x += xStep;
					continue;
				}
				//               SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yStart, false );
				break;
			}
			return Status;
		}
		Index--;
		//        if ( Index >= MAX_POINTS_DC )
		if ( Index < 0 )
		{
			break;
		}
	}

	ED_DC.ScanEdgeNumPoints[BottomEdgeDC] = ED_DC.Edge_Num_Points[BottomEdgeDC] = n;

	// Because the Index of Edge_xpt and Edge_ypt starts in the middle, the first values should be zero.
	// Find the first real value, and copy from that point of the array to the beginning of the array
	FirstValue = 0;
	for ( i = 0; i < MAX_POINTS_DC; i++ )
	{
		if ( ED_DC.Edge_xpt[BottomEdgeDC][i] != 0 )
		{
			FirstValue = i;
			break;
		}
	}

	memmove( ED_DC.Edge_xpt[BottomEdgeDC], &ED_DC.Edge_xpt[BottomEdgeDC][FirstValue], sizeof( ED_DC.Edge_xpt[BottomEdgeDC][0] ) * ( MAX_POINTS_DC - FirstValue ) );
	memmove( ED_DC.Edge_ypt[BottomEdgeDC], &ED_DC.Edge_ypt[BottomEdgeDC][FirstValue], sizeof( ED_DC.Edge_ypt[BottomEdgeDC][0] ) * ( MAX_POINTS_DC - FirstValue ) );
	memcpy( ED_DC.ScanEdgeXPoints[BottomEdgeDC], ED_DC.Edge_xpt[BottomEdgeDC], sizeof( ED_DC.Edge_xpt[BottomEdgeDC][0] ) * MAX_POINTS_DC );
	memcpy( ED_DC.ScanEdgeYPoints[BottomEdgeDC], ED_DC.Edge_ypt[BottomEdgeDC], sizeof( ED_DC.Edge_ypt[BottomEdgeDC][0] ) * MAX_POINTS_DC );
	SortAndRemoveOutliers_DC( ED_DC.Edge_ypt[BottomEdgeDC], ED_DC.Edge_xpt[BottomEdgeDC], 1, &ED_DC.Edge_Num_Points[BottomEdgeDC] ,1);

	if ( ED_DC.Edge_Num_Points[BottomEdgeDC] < MINIMUM_NUMBERS_OF_POINTS_DC )
	{
		SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Few_Points_DC ,0);
		return ERR_TOO_FEW_EDGE_POINTS_FOUND_DC;
	}
	mask_DC( ED_DC.Edge_xpt[BottomEdgeDC], ED_DC.Edge_ypt[BottomEdgeDC], ED_DC.Edge_Num_Points[BottomEdgeDC] ,pbs);

	ED_DC.Bottom_Edge_tan_t = ED_DC.tan_t;
	ED_DC.Bottom_Edge_sin_t = ED_DC.sin_t;
	ED_DC.Bottom_Edge_cos_t = ED_DC.cos_t;
	ED_DC.Bottom_Edge_offs = ED_DC.offs + 1;

	ED_DC.Bottom_Edge_Deg = (float)(atan( ED_DC.Bottom_Edge_tan_t * ED_DC.dpi_Correction ) * PIRAD_DC);

	return 0;
}

// handles edge checking in the y direction, going up (DetectBottomEdge) or down (DetectTopEdge)
unsigned short DetectBottomEdgeYLoop_DC( int x, // The x offset for this line
	int *yStart, // The starting point for scanning. This is passed back since the next scan line is based on the detection of this scan line
	int yEnd, // The last point to look for an edge
	int *EdgePointFoundCount, // Running total of the number of edges found
	int FindThreshold, // Pixel value threshold for identifying the edge
	float *XPoint, // X from List of (X,Y) points where the edge was detected
	float *YPoint, // Y from List of (X,Y) points where the edge was detected
	int *ErrorCount, // Running total of edge starting too soon errors
	int ErrorCountMax // Maximum number of ErrorCount before an actual error is thrown
	,ST_BS *pbs)
{
	unsigned short Result = WARN_EDGE_NOT_FOUND_DC;
	//    T_Point AlignedPoint;
	ST_SPOINT AlignedPoint;
	ST_SPOINT FollowUpTest;
	int LastPixelValue = 0;
	const int yStep = 1;
	u8 FalseEdgeDetected = 0;

	AlignedPoint.p_plane_tbl = ED_DC.Plane; // 200*200dpi Ref Red
	FollowUpTest.p_plane_tbl = ED_DC.Plane; // 200*200dpi Ref Red

	AlignedPoint.x = x;
	for ( AlignedPoint.y = *yStart; AlignedPoint.y > yEnd; AlignedPoint.y -= yStep )
	{
		int PixelValue =  ED_Pget_DC(AlignedPoint ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  AlignedPoint );
		if ( PixelValue > FindThreshold )
		{
			// Test next few pixels to ensure we are not looking at corrupted data
			//            T_Point FollowUpTest;
			FalseEdgeDetected = 0;//false
			FollowUpTest.x = AlignedPoint.x;
			for ( FollowUpTest.y = AlignedPoint.y - yStep;
				( FollowUpTest.y >= AlignedPoint.y - ( yStep * NUM_ADDITIONAL_PIXEL_TEST_DC ) ) && ( FollowUpTest.y > yEnd );
				FollowUpTest.y -= yStep )
			{
				int NewPixelValue = ED_Pget_DC(FollowUpTest,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  FollowUpTest );
				if ( NewPixelValue <= FindThreshold )
				{
					FalseEdgeDetected = 1;//true
					break;
				}
			}
			if ( FalseEdgeDetected )
			{
				LastPixelValue = PixelValue;
				continue;
			}

			if ( AlignedPoint.y == *yStart )
			{
				Result = WARN_EDGE_FOUND_TOO_SOON_DC;
				*ErrorCount += 1;
				//*ErrorCount++;
				break;
			}
			// Found a pooint

			//         SIM_INTERFACE_TraceAlignedLine( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, x, yStart, x, AlignedPoint.y, 0xFF );

			// Scales the pixel point back to a float value
			*YPoint = AlignedPoint.y + 1 - ( FindThreshold - LastPixelValue ) * yStep / ( ( float )( PixelValue - LastPixelValue ) );
			*XPoint = (float)AlignedPoint.x;

			*yStart = AlignedPoint.y + ED_DC.Small_Backtrack;
			if ( ++*EdgePointFoundCount >= MAX_POINTS_DC )
			{
				SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Many_Points_DC, 0 );
				return ERR_TOO_MANY_EDGE_POINTS_FOUND_DC;
			}
			return 0;
		}
		LastPixelValue = PixelValue;
	}
	//Failed to find the point
	//    SIM_INTERFACE_TraceAlignedLine( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, x, AlignedPoint.y, x, yStart, 0xFF );

	if ( *ErrorCount >= ErrorCountMax )
	{
		// Too many points are on the edge of the test range.
		//　探査開始Y座標に媒体有状態がErrorCountMax以上見つかった。
		SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_On_Test_Edge_DC, 0);
		return *ErrorCount;//ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
	}
	return Result;
}

/*                _           __ _     ______    _
//               | |         / _| |   |  ____|  | |
//               | |     ___| |_| |_  | |__   __| | __ _  ___
//               | |    / _ \  _| __| |  __| / _` |/ _` |/ _ \
//               | |___|  __/ | | |_  | |___| (_| | (_| |  __/
//               |______\___|_|  \__| |______\__,_|\__, |\___|
//                                                  __/ |
//                                                 |___/
//http://www.network-science.de/ascii/   (big Font) */

// xStart = X Position Start
// yStart = Y Position End
// xEnd = X Position End
// yEnd = Y Position Start
// yStep = number of pixels to step for next scan to the right
// FindThreshold = threshold for detecting the edge
//     Start
//(xStart, yStart)          ----------------------------------------------------
//         -----------------| $                                              $ |
//                          |                                                  |
//(xss, yStart + step) -----|                       o o                        |
//                          |                                                  |
//                     -----|                      \___/                       |
//                          |                                                  |
//                     -----|------ (xEnd, yEnd) End                         $ |
//                          ----------------------------------------------------
unsigned short DetectLeftEdge_DC( int xStart, int yStart, int xEnd, int yEnd, int yStep, int FindThreshold,ST_BS *pbs )
{
	//    T_Point pt;
	//    T_Point FollowUpTest;
	ST_SPOINT pt;
	ST_SPOINT FollowUpTest;
	int n = 0;
#ifdef NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA
	int e_co = 0;
	int e_co_max = 10;
#endif
	int xss;
	u8 FoundEdge = 0;//false;
	u8 FalseEdgeDetected = 0;//false;
	float dx;

	pt.p_plane_tbl = ED_DC.Plane;
	FollowUpTest.p_plane_tbl = ED_DC.Plane; 

	//This is to ensure the minimum starting point is 20 pixels down
	//from the starting of the effective subscan line.
	yStart = yStart < 20 ? 20 : yStart;
	yStart = yStart > ED_DC.Sub_scan_max_val - (ED_DC.Sub_scan_max_val / 4) ? (int)(ED_DC.Sub_scan_max_val / 1.5f) : yStart;


	//This is to ensure searching only inside of the effective subscan lines.
	yEnd = yEnd > ED_DC.Sub_scan_max_val ? ED_DC.Sub_scan_max_val : yEnd;
	yEnd = yEnd < (ED_DC.Sub_scan_max_val / 4) ? (int)(ED_DC.Sub_scan_max_val / 3.5f) : yEnd;
	xss = xStart;

	//    JCMassert( xEnd - xStart + 1 < MAX_GET_PIXEL_RANGE );
	for ( pt.y = yStart; pt.y < yEnd; pt.y += yStep )
	{

		int LastPixelValue = 0;
		if ( xss < xStart )
		{
			xss = xStart;
		}
		FoundEdge = 0;//false;
		for ( pt.x = xss; pt.x < xEnd; pt.x++ )
		{
			int PixelValue = ED_Pget_DC(pt,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  pt );

			if ( PixelValue > FindThreshold )
			{
				// Test next few pixels to ensure we are not looking at corrupted data
				FalseEdgeDetected = 0;//false;
				FollowUpTest.y = pt.y;
				for ( FollowUpTest.x = pt.x + 1; ( FollowUpTest.x <= pt.x + NUM_ADDITIONAL_PIXEL_TEST_DC ) && ( FollowUpTest.x < xEnd ); FollowUpTest.x++ )
				{
					int NewPixelValue = ED_Pget_DC(FollowUpTest,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  FollowUpTest );
					if ( NewPixelValue <= FindThreshold )
					{
						FalseEdgeDetected = 1;//true;
						break;
					}
				}
				if ( FalseEdgeDetected )
				{
					LastPixelValue = PixelValue;
					continue;
				}
				if ( pt.x == xss )
				{
					// If we started at the first point, this is an error
					if ( pt.x == xStart )
					{

#ifdef NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA
						e_co++;
						break;
# endif

					}
					else
					{
						// We didn't start at the first point, so retry this scan from the start
						xss = xStart;
						pt.x = xStart - 1; // This will be incremented when we continue
						continue;
					}
				}

				FoundEdge = 1;//true;
				dx = pt.x - ( PixelValue - FindThreshold ) / ( ( float )( PixelValue - LastPixelValue ) );

				ED_DC.Edge_ypt[LeftEdgeDC][n] = dx;
				ED_DC.Edge_xpt[LeftEdgeDC][n] = (float)pt.y;

				xss = pt.x - ED_DC.Small_Backtrack;
				if ( ++n >= MAX_POINTS_DC )
				{
					SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Many_Points_DC, 0 );
					return ERR_TOO_MANY_EDGE_POINTS_FOUND_DC;
				}
				break;
			}
			LastPixelValue = PixelValue;
		}
		if ( !FoundEdge )
		{
			xss = xStart;
		}

#ifdef NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA
		if ( e_co >= e_co_max )
		{
			SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_On_Test_Edge, 0 );
			return ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
		}
# endif
	}

	ED_DC.ScanEdgeNumPoints[LeftEdgeDC] = ED_DC.Edge_Num_Points[LeftEdgeDC] = n;

	memcpy( ED_DC.ScanEdgeXPoints[LeftEdgeDC], ED_DC.Edge_xpt[LeftEdgeDC], sizeof( ED_DC.Edge_xpt[LeftEdgeDC][0] ) * MAX_POINTS_DC );
	memcpy( ED_DC.ScanEdgeYPoints[LeftEdgeDC], ED_DC.Edge_ypt[LeftEdgeDC], sizeof( ED_DC.Edge_ypt[LeftEdgeDC][0] ) * MAX_POINTS_DC );
	SortAndRemoveOutliers_DC( ED_DC.Edge_ypt[LeftEdgeDC], ED_DC.Edge_xpt[LeftEdgeDC],0, &ED_DC.Edge_Num_Points[LeftEdgeDC] ,ED_DC.dpi_Correction);

	if ( ED_DC.Edge_Num_Points[LeftEdgeDC] < MINIMUM_NUMBERS_OF_POINTS_DC )
	{
		SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Few_Points_DC, 0 );
		return ERR_TOO_FEW_EDGE_POINTS_FOUND_DC;
	}

	mask_DC( ED_DC.Edge_xpt[LeftEdgeDC], ED_DC.Edge_ypt[LeftEdgeDC], ED_DC.Edge_Num_Points[LeftEdgeDC],pbs);

	ED_DC.Left_Edge_offs = ED_DC.offs + 1;
	ED_DC.Left_Edge_tan_t = ED_DC.tan_t;
	ED_DC.Left_Edge_sin_t = ED_DC.sin_t;
	ED_DC.Left_Edge_cos_t = ED_DC.cos_t;
	ED_DC.Left_Edge_Deg = (float)(atan( ED_DC.Left_Edge_tan_t   / ED_DC.dpi_Correction) * PIRAD_DC);

	return 0;
}


/*             _____  _       _     _     ______    _
//            |  __ \(_)     | |   | |   |  ____|  | |
//            | |__) |_  __ _| |__ | |_  | |__   __| | __ _  ___
//            |  _  /| |/ _` | '_ \| __| |  __| / _` |/ _` |/ _ \
//            | | \ \| | (_| | | | | |_  | |___| (_| | (_| |  __/
//            |_|  \_\_|\__, |_| |_|\__| |______\__,_|\__, |\___|
//                       __/ |                         __/ |
//                      |___/                         |___/
//http://www.network-science.de/ascii/   (big Font) */

// Starts at (xs, ys) and goes to (xe, ye)
// xs = X Position Start
// ys = Y Position Start
// xe = X Position End
// ye = Y Position End
// step = number of pixels to step for next scan to the right
// FindThreshold = threshold for detecting the edge
/*
Start
----------------------------------------------------         (xStart, yStart)
| $                                              $ |-------------
|                                                  |
|                      o   o                       |----- (xss, yStart + yStep)
|                                                  |
|                      \___/                       |-----
|                                                  |
| $                          End (xEnd, yEnd) -----|-----
----------------------------------------------------
*/
unsigned short DetectRightEdge_DC( int xStart, int yStart, int xEnd, int yEnd, int yStep, int FindThreshold ,ST_BS *pbs)
{
	//    T_Point pt;
	//    T_Point FollowUpTest;
	ST_SPOINT pt;
	ST_SPOINT FollowUpTest;
	int n = 0;
#ifdef NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA
	int e_co = 0;
	int e_co_max = 10;
#endif
	int xss;
	u8 FoundEdge = 0;//false;
	u8 FalseEdgeDetected = 0;//false;
	int NewPixelValue;
	float dx;
	pt.p_plane_tbl = ED_DC.Plane; // 200*200dpi Ref Red
	FollowUpTest.p_plane_tbl = ED_DC.Plane; // 200*200dpi Ref Red


	//This is to ensure the minimum starting point is 20 pixels down
	//from the starting of the effective subscan line.
	yStart = yStart < 20 ? 20 : yStart;
	yStart = yStart > ED_DC.Sub_scan_max_val - (ED_DC.Sub_scan_max_val / 4) ? (int)(ED_DC.Sub_scan_max_val / 1.5f) : yStart;

	//This is to ensure searching only inside of the effective subscan lines.
	yEnd = yEnd > ED_DC.Sub_scan_max_val ? ED_DC.Sub_scan_max_val : yEnd;
	yEnd = yEnd < (ED_DC.Sub_scan_max_val / 4) ? (int)(ED_DC.Sub_scan_max_val / 3.5f) : yEnd;

	xss = xStart;

	for ( pt.y = yStart; pt.y < yEnd; pt.y += yStep )
	{
		int LastPixelValue = 0;
		FoundEdge = 0;//false;
		if ( xss > xStart )
		{
			xss = xStart;
		}
		for ( pt.x = xss; pt.x > xEnd; pt.x-- )
		{
			int PixelValue = ED_Pget_DC(pt,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  pt );
			if ( PixelValue > FindThreshold )
			{
				// Test next few pixels to ensure we are not looking at corrupted data
				FalseEdgeDetected = 0;//false;
				FollowUpTest.y = pt.y;
				for ( FollowUpTest.x = pt.x - 1; ( FollowUpTest.x >= pt.x - NUM_ADDITIONAL_PIXEL_TEST_DC ) && ( FollowUpTest.x > xEnd );FollowUpTest.x-- )
				{
					NewPixelValue = ED_Pget_DC(FollowUpTest ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect, FollowUpTest );
					if ( NewPixelValue <= FindThreshold )
					{
						FalseEdgeDetected = 1;//true;
						break;
					}
				}
				if ( FalseEdgeDetected )
				{
					LastPixelValue = PixelValue;
					continue;
				}
				if ( pt.x == xss )
				{
					// If we started at the first point, this is an error
					if ( pt.x == xStart )
					{
#ifdef NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA
						e_co++;
						break;
# endif
					}
					else
					{
						// We didn't start at the first point, so retry this scan from the start
						xss = xStart;
						pt.x = xStart + 1; // This will be decremented when we continue
						continue;
					}
				}
				FoundEdge = 1;//true;
				//                SIM_INTERFACE_TraceAlignedLine( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, pt.x, pt.y, xss, pt.y, 0xFF );

				dx = pt.x + 1 - ( FindThreshold - LastPixelValue ) / ( ( float )( PixelValue - LastPixelValue ) );

				ED_DC.Edge_ypt[RightEdgeDC][n] = dx;
				ED_DC.Edge_xpt[RightEdgeDC][n] = (float)pt.y;

				xss = pt.x + ED_DC.Small_Backtrack;
				if ( ++n >= MAX_POINTS_DC )
				{
					// err
					SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Many_Points_DC, 0 );
					return ERR_TOO_MANY_EDGE_POINTS_FOUND_DC;
				}
				break;
			}
			LastPixelValue = PixelValue;
		}
		if ( !FoundEdge )
		{
			//            SIM_INTERFACE_TraceAlignedLine( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, pt.x, pt.y, xss, pt.y, 0xFF );
			//            SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, pt.x, pt.y, false );
			xss = xStart;
		}
#ifdef NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA
		if ( e_co >= e_co_max )
		{
			SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_On_Test_Edge, 0 );
			return ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
		}
# endif
	}

	ED_DC.ScanEdgeNumPoints[RightEdgeDC] = ED_DC.Edge_Num_Points[RightEdgeDC] = n;
	memcpy( ED_DC.ScanEdgeXPoints[RightEdgeDC], ED_DC.Edge_xpt[RightEdgeDC], sizeof( ED_DC.Edge_xpt[RightEdgeDC][0] ) * MAX_POINTS_DC );
	memcpy( ED_DC.ScanEdgeYPoints[RightEdgeDC], ED_DC.Edge_ypt[RightEdgeDC], sizeof( ED_DC.Edge_ypt[RightEdgeDC][0] ) * MAX_POINTS_DC );
	SortAndRemoveOutliers_DC( ED_DC.Edge_ypt[RightEdgeDC], ED_DC.Edge_xpt[RightEdgeDC], 0, &ED_DC.Edge_Num_Points[RightEdgeDC],ED_DC.dpi_Correction );

	if ( ED_DC.Edge_Num_Points[RightEdgeDC] < MINIMUM_NUMBERS_OF_POINTS_DC )
	{
		//SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Too_Few_Points_DC, 0 );
		return ERR_TOO_FEW_EDGE_POINTS_FOUND_DC;
	}

	mask_DC( ED_DC.Edge_xpt[RightEdgeDC], ED_DC.Edge_ypt[RightEdgeDC], ED_DC.Edge_Num_Points[RightEdgeDC] ,pbs);

	ED_DC.Right_Edge_offs = ED_DC.offs + 1; 
	ED_DC.Right_Edge_tan_t = ED_DC.tan_t  ;
	ED_DC.Right_Edge_sin_t = ED_DC.sin_t;
	ED_DC.Right_Edge_cos_t = ED_DC.cos_t;
	ED_DC.Right_Edge_Deg = (float)(atan( ED_DC.Right_Edge_tan_t / ED_DC.dpi_Correction) * PIRAD_DC);

	return 0;
}

// Does the calculations to fix an identified bad edge.
// The algorithm finds the maximum distance from the good side and the bad side.
// That maximum distance is used to find the corrected offset.
// The tan_t is copied from the good edge to the bad edge. (They are assumed to be parallel)
//識別された不良エッジを修正するための計算を行います。
//アルゴリズムは、良い側と悪い側からの最大距離を求めます。
//修正されたオフセットを見つけるために、その最大距離が使用されます。
// tan_tは良いエッジから悪いエッジにコピーされます。 （それらは平行であると仮定される）
void FixEdge_DC( float *Bad_Edge_tan_t, float *Bad_Edge_sin_t, float *Bad_Edge_cos_t, float *Bad_Edge_Deg, float *Bad_Edge_offs,
	const int Bad_Edge_Num_Points, const float * const BadEdgePointsX, const float * const BadEdgePointsY, const float Good_Edge_tan_t,
	const float Good_Edge_sin_t, const float Good_Edge_cos_t, const float Good_Edge_Deg, const float Good_Edge_offs,
	const int DiffDirection , const float * const GoodEdgePointsX, const float * const GoodEdgePointsY )
{
	int Index = 0;
	float Distance;
	//float Distance_debug1[100];
	//float Distance_debug2[100];

	float MaxDistance = 0.0;
	float ax[100];
	float by[100];
	float denominator[100];
	float molecule[100];
	float tan = Good_Edge_tan_t;
	//int max_num = 0;
	//int num = 0;
	//int max_point = 0;

	
	////点と点の距離を求める方式
	//for ( Index = 0; Index < Bad_Edge_Num_Points; Index++ )
	//{
	//	if(BadEdgePointsX[Index] < 1)
	//	{
	//		continue;
	//	}

	//	if(BadEdgePointsY[Index] < 1)
	//	{
	//		continue;
	//	}

	//	Distance = sqrtf(((BadEdgePointsX[Index] - GoodEdgePointsX[Index]) * (BadEdgePointsX[Index] - GoodEdgePointsX[Index]))
	//				   +(((BadEdgePointsY[Index] - GoodEdgePointsY[Index]) * (BadEdgePointsY[Index] - GoodEdgePointsY[Index])))); // 直線と点の最短距離

	//	Distance_debug2[Index] = Distance;

	//	if ( Distance > MaxDistance )
	//	{
	//		MaxDistance = Distance;
	//	}
	//}

	//点と線の距離を求める方式
	for ( Index = 0; Index < Bad_Edge_Num_Points; Index++ )
	{
		//特異点削除の際、特異点は-1でマスクされるのでそういったポイントは処理しないためのcontinue
		if(BadEdgePointsX[Index] < 1)
		{
			continue;
		}

		if(BadEdgePointsY[Index] < 1)
		{
			continue;
		}

		//dpiのちがいによりDistanceの値が変になる。(11/21)
		//Distance = (abs((Good_Edge_tan_t * BadEdgePointsX[Index]) - (BadEdgePointsY[Index]) + Good_Edge_offs) ) / (sqrt(Good_Edge_tan_t * Good_Edge_tan_t + 1));
		//Distance_debug1[Index]= (abs((Good_Edge_tan_t * BadEdgePointsX[Index]) + (BadEdgePointsY[Index] * -1) + Good_Edge_offs) ) / (sqrt(Good_Edge_tan_t * Good_Edge_tan_t + 1));

		//Distance = (float)fabs( BadEdgePointsY[Index] - Good_Edge_offs - Good_Edge_tan_t * fabs( BadEdgePointsX[Index] ) ) * Good_Edge_cos_t; // 直線と点の最短距離
		//Distance_debug1[Index]=(float)fabs( BadEdgePointsY[Index] - Good_Edge_offs - Good_Edge_tan_t * fabs( BadEdgePointsX[Index] ) ) * Good_Edge_cos_t; // 直線と点の最短距離
	
		ax[Index] = BadEdgePointsX[Index] * tan;
		by[Index] = BadEdgePointsY[Index];

		denominator[Index] = sqrtf(tan * tan +1.0f);
		molecule[Index] = (float)fabs(ax[Index] - by[Index] + Good_Edge_offs);

		Distance = molecule[Index] / denominator[Index]; // 直線と点の最短距離
		//Distance_debug2[Index] = Distance;

		if ( Distance > MaxDistance )
		{
			MaxDistance = Distance;
		}
	}

	// Copy the angles from the good side
	*Bad_Edge_tan_t = Good_Edge_tan_t;
	*Bad_Edge_sin_t = Good_Edge_sin_t;
	*Bad_Edge_cos_t = Good_Edge_cos_t;
	*Bad_Edge_Deg = Good_Edge_Deg;
	// The Offset of the bad line is the maximum distance +/- the Offset of the good line
	*Bad_Edge_offs = Good_Edge_offs + (DiffDirection * MaxDistance );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Masks the singularity
// 特異点をフィルターする
void mask_DC( float* xpt, float* ypt, int num, ST_BS *pbs )
{
	int i = 0;
	//int ii = 0;

	float main_dot = pbs->PlaneInfo[ED_DC.Plane].main_element_pitch;
	float sub_dot =25.4f / (200 / pbs->PlaneInfo[ED_DC.Plane].sub_sampling_pitch);

	for ( i = 0; i < 100; i++ )
	{
		calu_kaiki_DC( xpt, ypt, num ); // Once back kaesa affiliates, linear approximation// 一次回帰による、直線近似
		if ( 0 == mask_errdt_DC( xpt, ypt, num, ED_DC.tan_t, (int)ED_DC.offs, 0.5 ,main_dot ,sub_dot))
		{

//#ifdef VS_DEBUG
//			for( ii = 0; ii < num; ii++ )
//			{
//
//				deb_para[0] = 4;		// function code
//				deb_para[1] = xpt[ii];	// 
//				deb_para[2] = ypt[ii];	//
//				deb_para[3] = 1;		// plane
//				callback(deb_para);		// debug
//
//				deb_para[1] = ypt[ii];	// 
//				deb_para[2] = xpt[ii];	//
//
//				callback(deb_para);		// debug
//			}
//#endif


			break; // break  mask_DC the data of the maximum value. Break without a mask_DC// 最大差異のデータをフィルターする。 フィルターなしでbreak
		}
	}
}

// In a return kaesa ni Proceeds from the straight-line approximation
// xp, yp, n ---> tan, sin, cos, offset
// 一次回帰による直線近似
// 配列xp, yp, n ---> tan, sin, cos, offset
void calu_kaiki_DC( float* xpt, float* ypt, int num )
{
	int i = 0;
	double x;
	double md;
	double xAve, yAve, xSquaredSum, mxy;
	int NumPoints;

	NumPoints = 0;
	x = 0;
	xAve = 0;
	yAve = 0;
	xSquaredSum = 0;
	mxy = 0;

	for ( i = 0; i < num; i++ )
	{
		if ( xpt[i] < 0 )		// すでにフィルターされているなら、
		{
			continue;			// 無視する
		}

		x = xpt[i];
		md = ypt[i];

		xAve += x;				// 一次回帰　直線近似
		yAve += md;
		xSquaredSum += x * x;
		mxy += x * md;
		NumPoints++;

	}

	xAve = xAve / NumPoints;
	yAve = yAve / NumPoints;



	// See http://mathworld.wolfram.com/LeastSquaresFitting.html, Line 15 (Least Squares Fitting)
	ED_DC.tan_t = (float)(( mxy - NumPoints * ( xAve * yAve ) ) / ( xSquaredSum - NumPoints * xAve * xAve )); // katamuki
	// See http://mathworld.wolfram.com/LeastSquaresFitting.html, Lines 13 (Least Squares Fitting)
	ED_DC.offs = (float)(( yAve * xSquaredSum - xAve * mxy ) / ( xSquaredSum - NumPoints * xAve * xAve )); // offset y

	ED_DC.sin_t = (float)(ED_DC.tan_t / sqrt( 1 + ED_DC.tan_t * ED_DC.tan_t )); // tan_t = sin_t/cos_t
	ED_DC.cos_t = (float)(sqrt( 1 - ED_DC.sin_t * ED_DC.sin_t )); // sin_t * sin_t + cos_t * cos_t = 1
}

//// 最大差異のデータをフィルターする。
// xp, yp, n, tan_t, offset, dfmax
int mask_errdt_DC( float* xpt, float* ypt, int num, float tan_t, int offset, float deflim ,float main_dot ,float sub_dot)
{
	int	i;
	float dfmax = -1;
	float dfthr_main;
	float dfthr_sub;
	int dfpt;
	float df;
	float X;
	float Y;

	for ( i = 0; i < num; i++ )
	{
		if ( xpt[i] < 0 )	// すでにフィルターされている？
		{
			continue;
		}

		//df = ( ypt[i] - tan_t * xpt[i] - offset );
		//df = ( df * df ) / ( 1 + tan_t * tan_t );	// df = (直線と点の最短距離)^2


		X = ( tan_t * (ypt[i] - offset) + xpt[i] ) / (tan_t * tan_t +1);
		Y = (tan_t * (tan_t * (ypt[i] - offset) + xpt[i])) / ( tan_t * tan_t + 1)  + offset ;

		df = ((xpt[i]-X)*(main_dot)*(xpt[i]-X)*(main_dot)) + (( ypt[i]-Y)*(sub_dot)*( ypt[i]-Y)*(sub_dot));

		//df = (abs((tan_t * xpt[i]) + (ypt[i] * -1) + offset) ) / (sqrt(tan_t * tan_t + 1));

		if ( dfmax < df )
		{
			dfmax = df;
			dfpt = i;
		}
	}

	if(deflim /(main_dot) < 2/*dot*/)//距離が2dot未満の場合2dotにする
	{
		dfthr_main = 2;
	}
	else
	{
		dfthr_main = deflim /main_dot;
	}
	//dfthr_main = deflim /main_mm;

	if(deflim /(sub_dot) < 2/*dot*/)//距離が2dot未満の場合2dotにする
	{
		dfthr_sub = 2;
	}
	else
	{
		dfthr_sub = deflim /sub_dot;
	}
	//dfthr_sub = deflim /sub_mm;
	// 最大差異(最も直線から離れている点)データがdeflim (mm)以上離れているならフィルター
	if ( dfmax >  (dfthr_main + dfthr_sub) )	// deflim (mm)　以上離れている？
	{	

		xpt[dfpt] = -1;
		return (int)(dfmax * 10); // 整数型なので10倍
	}
	else
	{
		return 0;	// フィルター無し
	}
}

// Intersection coordinates calculation
// tan_t1 and ofsx should be from left or right edge only!
// tan_t2 and ofsy should be from top or bottom edge only!
//交差点座標計算
// tan_t1とofsxは、左端または右端でなければなりません！
// tan_t2とofsyは、上端または下端にする必要があります。
//                     <------L or R------->      <------U or B------->
T_Point_float_DC calcu_xoyo_DC( float tan_t1, float ofsx,  float tan_t2, float ofsy ,ST_BS *pbs)
{
	T_Point_float_DC Po;

	Po.y = ( tan_t2 * ofsx + ofsy ) / ( 1.0f - tan_t2 * tan_t1 ) * ED_DC.dpi_Correction / pbs->pitch_ratio;
	Po.x = ( tan_t1 * ofsy + ofsx ) / ( 1.0f - tan_t1 * tan_t2 );

	return Po;

}

// Angle consistency check
//角度整合性チェック
unsigned short Edg_Deg_def_chk_DC( )
{
	// the .1 is so that rounding doesn't show a valid angle that is beyond the threshold
	// i.e. 24.04 would fail, but it would round to 24.0 and look valid
	// .1は、丸めが閾値を超えた有効な角度を示さないようにする
	//すなわち24.04は失敗しますが、24.0に丸めて有効です

	const float Deg_def_Lim = ONE_SIDE_SAFE_DEGREE_VAL_DC; // Bound angle differential value (in degrees)// ４辺の角度差　限界値　（度）

	if (( fabs( ED_DC.Top_Edge_Deg    -  ED_DC.Bottom_Edge_Deg) > Deg_def_Lim ) ||
		( fabs( ED_DC.Top_Edge_Deg    - -ED_DC.Left_Edge_Deg  ) > Deg_def_Lim ) ||
		( fabs( ED_DC.Top_Edge_Deg    - -ED_DC.Right_Edge_Deg ) > Deg_def_Lim ) ||
		( fabs( ED_DC.Bottom_Edge_Deg - -ED_DC.Left_Edge_Deg  ) > Deg_def_Lim ) ||
		( fabs( ED_DC.Bottom_Edge_Deg - -ED_DC.Right_Edge_Deg ) > Deg_def_Lim ) ||
		( fabs( ED_DC.Left_Edge_Deg   -  ED_DC.Right_Edge_Deg ) > Deg_def_Lim ) )
	{
		SetResult_DC( CategoryCheckOutlineDC, ED_Category1DC, E_Edge_Skew_Too_Big_DC, 0 );
		return ERR_SKEW_TOO_BIG_DC;
	}
	return 0;
}
// 幅(ドット）	長さ(ドット）  角度（度） 中心座標(ドット) の計算
//	o---------X
//	| A-----C
//	| |     |
//	| B-----D
//  Y
void calcu_size_DC( float A_x, float A_y, float B_x, float B_y, float C_x, float C_y, float D_x, float D_y ,u8 le_or_se )
{
	float AB_x, AB_y, CD_x, CD_y, AC_x, AC_y, BD_x, BD_y;

	//dpiの違いは定数によって吸収する　200dpiに合わせる
	AB_x = ( A_x + B_x ) / 2.0F;
	AB_y = ( A_y + B_y ) / 2.0F;// * ED_DC.dpi_Correction;

	CD_x = ( C_x + D_x ) / 2.0F;
	CD_y = ( C_y + D_y ) / 2.0F;//* ED_DC.dpi_Correction;

	AC_x = ( A_x + C_x ) / 2.0F;
	AC_y = ( A_y + C_y ) / 2.0F;//* ED_DC.dpi_Correction;

	BD_x = ( B_x + D_x ) / 2.0F;
	BD_y = ( B_y + D_y ) / 2.0F;//* ED_DC.dpi_Correction;


	// 幅
	ED_DC.mBillSEPixelLength = (float)sqrt( ( AC_x - BD_x ) * ( AC_x - BD_x ) + ( AC_y - BD_y ) * ( AC_y - BD_y ) ); // a^2 + b^2 = c^2

	// 長さ
	ED_DC.mBillLEPixelLength = (float)sqrt( ( AB_x - CD_x ) * ( AB_x - CD_x ) + ( AB_y - CD_y ) * ( AB_y - CD_y ) );

	//
	if(le_or_se == LE)
	{
		if ( CD_x == AB_x )	//　DEV 0 エラー回避
			ED_DC._tan_t = ( CD_y - AB_y ) / 0.00000001f;
		else
			ED_DC._tan_t = (( CD_y - AB_y ) / ( CD_x - AB_x ));	// AB中点とCD中点を結ぶ直線の傾き
	}

	else    //SE  //2020-09-24 修正 0エラー回避内処理のy->ｘ変数に差し替え　SEのみおかしかった
	{
		if ( AC_x == BD_x ) //　DEV 0 エラー回避
			ED_DC._tan_t = -( BD_x - AC_x ) / 0.00000001f;
		else
			ED_DC._tan_t = -(( BD_x - AC_x ) / ( BD_y - AC_y ) );  // AB中点とCD中点を結ぶ直線の傾き
	}
	
	// 角度（度）
	ED_DC._deg   = (float)(PIRAD_DC * atan( ED_DC._tan_t )) ;
	ED_DC._sin_t = (float)(ED_DC._tan_t / sqrt( 1 + ED_DC._tan_t * ED_DC._tan_t ));
	ED_DC._cos_t = (float)sqrt( 1 - ED_DC._sin_t * ED_DC._sin_t );

	// 中心座標
	ED_DC._o.x = (int)(( A_x + B_x + C_x + D_x ) / 4.0 + 0.5);
	ED_DC._o.y = (int)(( A_y + B_y + C_y + D_y ) / 4.0 + 0.5);


}

// Determine the Outliers and remove them.
// 外れ値を決定してそれらを削除します。
void SortAndRemoveOutliers_DC( float *ListToAnalyze, float *SecondaryList, u8 XYReversed, int *NumPoints, char flg )
{

	
	//閾値設定　差が小さすぎたら定数にしますが、小さすぎるとマージンがなくなり判定基準が厳しすぎることになります
	//			逆に大きすぎると削除される点が少なくなります。
#define DELTA_PIXEL (2.0f)	//dot

	
	float Delta[MAX_POINTS_DC];
	int	i;
	float MedianDelta;
	float DiffPoints;
	float MinPass, MaxPass;
	u8 ListToRemove[MAX_POINTS_DC];
	int NumRemoved = 0;
	float diff_mini_limit;  //(0.5/BILL_PIXEL_SIZE)//(1.0f) // 許容値(±dot)値 この値が小さ過ぎると残らない　0.5ｍｍ
	u8 delet_point_count = 0;
	
	diff_mini_limit = DELTA_PIXEL;// flg;  //(0.5/BILL_PIXEL_SIZE)//(1.0f) // 許容値(±dot)値 この値が小さ過ぎると残らない　0.5ｍｍ

	//return;

	if ( *NumPoints < 5 )	// 券端検出ポイント数が５以下の時はしない　７以下にしないとインデックスが
	{
		return;
	}

	// We have NumPoints data points
	// This leads to NumPoints-1 deltas
	// Get the deltas for each position
	// NumPointsデータポイントを持つ
	//これはNumPoints-1傾きにつながります
	//各位置の傾きを取得する
	memset( Delta, 0, sizeof( Delta ) );

	// If it is the beginning of the bill remove the first value
	//それが券の先頭であれば、最初の値を削除します
	for ( i = 0; i < *NumPoints - 1; i++ )
	{
		Delta[i] = ListToAnalyze[i] - ListToAnalyze[i + 1];
	}

	// Sort the Deltas
	// TBD this should be changed to Quickselect
	//傾きをソートする
	// TBDこれはQuickselectに変更する必要があります
	QuickSort_float_DC( Delta, 0, *NumPoints - 2 );

	// This is just an approximate median delta.
	// Instead of taking a median, get an average of points near median.
	//これはちょうどおおよその中央値傾きです。
	//中央値を取る代わりに、中央値に近い平均点を得る。
	if ( *NumPoints % 2 )	// 奇数個で、
	{
		// Odd Number of NumPoints
		//        JCMassert( NumPoints / 2 - 2 >= 0 );
		//        JCMassert( NumPoints / 2 + 1 <= NumPoints - 1 );
		// NumPointの奇数
		// JCMassert（NumPoints / 2 - 2> = 0）;
		// JCMassert（NumPoints / 2 + 1 <= NumPoints - 1）;
		// Even Number of Delta points becuase Delta points is one less than NumPoints
		// The edge detection has somewhat of a stair step response, averaging points
		// away from the median tries to minimize this effect.
		//デルタポイントがNumPointsよりも1つ少ないため、デルタポイント数が偶数になります。
		//エッジ検出には多少の階段状の応答があり、この効果を最小限に抑えるために
		//中央値から離れた平均点が試行されます。
		if ( *NumPoints >= 7 )	// かつ 7以上なら
		{
			DiffPoints = Delta[*NumPoints / 2 + 2] - Delta[*NumPoints / 2 - 3];
			MedianDelta = ( Delta[*NumPoints / 2 - 3] + Delta[*NumPoints / 2 + 2] ) / 2;

		}
		else
		{
			DiffPoints = Delta[*NumPoints / 2 + 1] - Delta[*NumPoints / 2 - 2];
			MedianDelta = ( Delta[*NumPoints / 2 - 2] + Delta[*NumPoints / 2 + 1] ) / 2;
		}
	}
	else	// 偶数個の時、
	{
		// Even Number of NumPoints
		//        JCMassert( NumPoints / 2 - 3 >= 0 );
		//        JCMassert( NumPoints / 2 + 1 <= NumPoints - 1 );
		// Odd Number of Delta points becuase Delta points is one less than NumPoints
		// The edge detection has somewhat of a stair step response, averaging points
		// away from the median tries to minimize this effect.
		//デルタポイントがNumPointsよりも1小さいため、デルタポイントの奇数
		//エッジ検出には幾分階段状の応答があり、平均点
		//中央値から離れると、この効果が最小限に抑えられます。
		// NumPointの偶数
		// JCMassert（NumPoints / 2 - 3> = 0）;
		// JCMassert（NumPoints / 2 + 1 <= NumPoints - 1）;
		//デルタポイントがNumPointsよりも1小さいため、デルタポイントの奇数
		//エッジ検出には幾分階段状の応答があり、平均点
		//中央値から離れると、この効果が最小限に抑えられます。
		MedianDelta = ( Delta[*NumPoints / 2 - 3] + Delta[*NumPoints / 2 + 1] ) / 2;
		DiffPoints = Delta[*NumPoints / 2 + 1] - Delta[*NumPoints / 2 - 3];
	}

	if ( DiffPoints < diff_mini_limit ) 
	{
		DiffPoints = DELTA_PIXEL;
	}

	MinPass = MedianDelta - DiffPoints;
	MaxPass = MedianDelta + DiffPoints;

	for ( i = 0; i < *NumPoints; i++ )
		ListToRemove[i] = 0;//false;

	for ( i = 0; i < *NumPoints - 1; i++ )
	{
		if ( ( ListToAnalyze[i] - ListToAnalyze[i + 1] > MaxPass ) || ( ListToAnalyze[i] - ListToAnalyze[i + 1] < MinPass ) )
		{
			ListToRemove[i] = 1;		//true;		// 1:削除マーク
			ListToRemove[i + 1] = 1;	//true;		// 次のポイントも削除する
			
		}
	}


	for ( i = 0; i < *NumPoints; i++ )	//削除されたポイント数をカウントします　上のループ内だと i と i+1を同時にマスクするので正確にカウントできません。
	{
		if (ListToRemove[i] == 1)
		{
			delet_point_count++;
		}
	}

	if(*NumPoints - delet_point_count < 5)	//削除されすぎるのを防ぐ　5ポイント以下ならばこの関数はなかったことにします。
	{
		return ;
	}

	for ( i = 0; i < *NumPoints + NumRemoved; i++ )
	{
		if ( ListToRemove[i] )
		{
			RemoveOutlier_DC(						// 券端位置リストから外れ値を削除し残りのデータを詰める
				ListToAnalyze,		// リスト1
				SecondaryList,		// リスト2
				i - NumRemoved,		// 削除データの位置
				XYReversed,			// 未使用
				NumPoints );		// リストの要素数
			NumRemoved++;
		}
	}
}

// Remove the coordinates that are in the Min Max List
// Min Max Listにある座標を削除する
void RemoveOutlier_DC( float *xPoints, float *yPoints, int Index, u8 XYReversed, int *NumPoints )
{
	//    SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, xPoints[Index], yPoints[Index], XYReversed );

	//    JCMassert( NumPoints - Index >= 0 );
	// If this is the last point, then we have nothing to copy. If this is the case, just decrement NumPoints.
	//これが最後の点であれば、コピーするものは何もありません。 これが当てはまる場合は、NumPointsを減らしてください。
	if ( NumPoints - Index > 0 )
	{
		memmove( &xPoints[Index], &xPoints[Index + 1], ( sizeof( xPoints[Index] ) * ( *NumPoints - Index ) ) );	// 削除して詰める
		memmove( &yPoints[Index], &yPoints[Index + 1], ( sizeof( yPoints[Index] ) * ( *NumPoints - Index ) ) );
	}
	(*NumPoints)--;	
}

void Swap_DC( float *ListToSort1, float *ListToSort2 )
{
	float tempFloat;
	tempFloat = *ListToSort1;
	*ListToSort1 = *ListToSort2;
	*ListToSort2 = tempFloat;
}

void QuickSort_float_DC( float ListToSort[MAX_POINTS_DC], int left, int right )
{
	int i = left, j = right;
	float pivot = ListToSort[( left + right ) / 2];

	while ( i <= j )
	{
		while ( ListToSort[i] < pivot )
		{
			i++;
		}
		while ( ListToSort[j] > pivot )
		{
			j--;
		}
		if ( i <= j )
		{
			Swap_DC( &ListToSort[i], &ListToSort[j] );
			i++;
			j--;
		}
	}
	/* recursion */
	if ( left < j )
	{
		QuickSort_float( ListToSort, left, j );
	}
	if ( i < right )
	{
		QuickSort_float( ListToSort, i, right );
	}
}

// 正負に対応した」小数点以下四捨五入
int rndup_DC( float d )
{
	if ( 0 < d )
	{
		return( int )( d + 0.5 );
	}
	return( int )( d - 0.5 );
}


void GetLeftEdgeIntercept_DC( float Slope, float Offset, float *X, float *Y )
{
	*X = (ED_DC.Left_Edge_tan_t * Offset + ED_DC.Left_Edge_offs) / (1 - ED_DC.Left_Edge_tan_t * Slope );
	*Y = Slope * *X + Offset;
}

void GetTopEdgeIntercept_DC( float Slope, float Offset, float *X, float *Y )
{
	*X = (ED_DC.Top_Edge_offs - Offset) / (Slope - ED_DC.Top_Edge_tan_t );
	*Y = Slope * *X + Offset;
}

void GetRightEdgeIntercept_DC( float Slope, float Offset, float *X, float *Y )
{
	*X = (ED_DC.Right_Edge_tan_t * Offset + ED_DC.Right_Edge_offs) / (1 - ED_DC.Right_Edge_tan_t * Slope );
	*Y = Slope * *X + Offset;
}

void GetBottomEdgeIntercept_DC( float Slope, float Offset, float *X, float *Y )
{
	*X = (ED_DC.Bottom_Edge_offs - Offset) / (Slope - ED_DC.Bottom_Edge_tan_t );
	*Y = Slope * *X + Offset;
}


void SetResult_DC( unsigned int Index, unsigned int Level, int Error, int JamCode /* = 0 */)
{
	//Make certain we are getting a known Category
	//    AssertValidLevel(Level);
	ED_DC.mCategory_Level[Index] = Level;
	ED_DC.mCategory_Error[Index] = Error;
	if (JamCode != 0)
	{
		ED_DC.mJamCode = JamCode;
	}
}

void SetOffset1_DC( unsigned short Offset1 )
{
	ED_DC.mResults_offset1 = Offset1;
}

void SetOffset2_DC( unsigned short Offset2 )
{
	ED_DC.mResults_offset2 = Offset2;
}

u16 deg_chk_DC(void)
{
	int deg = 0;

	deg = (	int)(abs((int)ED_DC._deg));

	//設定した規定角度以上ならばエラー
	if(OVERALL_SAFE_DEGREE_VAL_DC < deg)
		return ERR_SKEW_TOO_BIG_DC;
	
	else
		return 0;
}


u32 Eg_status_change_DC(ST_BS *pbs)
{
	s16 subdpi;

	//主走査(実装予定)

	//副走査
	//dpi補正用乗数の決定
	//12.5dpi以下を考慮して{×2}
	subdpi = pbs->Subblock_dpi / pbs->PlaneInfo[ED_DC.Plane].sub_sampling_pitch * 2;
	switch (subdpi)
	{
	case 400://200
		ED_DC.dpi_Correction = 1;
		break;

	case 200://100
		ED_DC.dpi_Correction = 2;
		break;

	case 100://50
		ED_DC.dpi_Correction = 4;
		break;

	case 50://25
		ED_DC.dpi_Correction = 8;
		break;

	case 25://12.5
		ED_DC.dpi_Correction = 16;
		break;
	}

	/*透過か反射で閾値の場合分け*/
	if(pbs->PlaneInfo[ED_DC.Plane].Ref_or_Trans == TRANSMISSION)
	{
		return 60;
	}

	else //Reflection
	{
		return 80;
	}

}

//外形検知専用getdt
u16 ED_Pget_DC(ST_SPOINT spt, ST_BS *pbs)
{
	s16 dt_u=0;

	dt_u = pbs->sens_dt[pbs->PlaneInfo[spt.p_plane_tbl].Address_Period * spt.y + spt.x + pbs->PlaneInfo[spt.p_plane_tbl].Address_Offset];
	return 255 - dt_u;	//値反転
}

//指定された1ラインをスキャンする。
//スキャンした中で最も暗かった画素値を出力する。
//
//in
//　ライン番号
//
//out
//　走査した中で最も明るい画素(255で反転するので)
u8	line_chk_DC(u16 line_num ,ST_BS *pbs)
{
	ST_SPOINT spt;
	u16 x;
	u8 pix;
	u8 max_pix = 0;

	spt.p_plane_tbl = (u8)ED_DC.Plane;
	spt.way = 0;

	for(x = pbs->PlaneInfo[ED_DC.Plane].main_effective_range_min; x < pbs->PlaneInfo[ED_DC.Plane].main_effective_range_max - 1; ++x)
	{
		spt.x = x;
		spt.y = (u32)line_num;
		pix = (u8)ED_Pget_DC(spt ,pbs);

		if(max_pix < pix)
		{
			max_pix = pix;
		}
	}

	return max_pix;



}

//最終ラインをスキャンして、紙幣が連なっていないかを確認する
//BAU-LE17のみで実行される
u8 chk_cont_trans_DC(ST_BS *pbs)
{
	u8 pix = 0;

	pix = line_chk_DC(ED_DC.Sub_scan_max_val - 1 ,pbs);

	if(pix != 0)
	{
		return (u8)ERR_CONTINUOUS_TRANSPORTATION_DC;
	}
	else
	{
		return 0;
	}

}
