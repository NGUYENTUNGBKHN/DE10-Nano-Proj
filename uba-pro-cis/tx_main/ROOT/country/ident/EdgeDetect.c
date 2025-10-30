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
 * @file EdgeDetect.c
 * @brief 外形検知ルーチン
 * @date 2018/03/14 CreatED->
 */
/****************************************************************************/

#include	<math.h>
#include    <string.h>
#include    <float.h>
#include	<stdlib.h>
#define EXT
#include "../common/global.h"

//#define LE17_Manager 1

// This is pixel distance between the checks for the edge.
// Note that this value was set to 15 base on Tear checking. Tears that are
// 4mm long need to be checkED-> This value of 15 should find those tears


// This is the minimum number non-rejected points for a given side required to pass
#define MINIMUM_NUMBERS_OF_POINTS   3

// Once an edge is found, four more pixels (NUM_ADDITIONAL_PIXEL_TEST) beyond the
// found edge will be checked to make certain it is a bill edge and not corrupt data
#define NUM_ADDITIONAL_PIXEL_TEST   4

//#define OVERALL_SAFE_DEGREE_VAL		40		//最大スキュー	不要

#define ONE_SIDE_SAFE_DEGREE_VAL	12		//各辺の傾き(台形になった時など)の許容範囲

#define SIDE_OUT_AREA_RANGE	1				//側面からこの値内にエッジがあった場合右か左に寄りすぎとしてエラーとする。



#define ERR_MAX_COUNT 10				//辺検知のエラー回数の上限　この回数以上でエラーとする。


#if LE17_Manager	// LE17 Manager
s16 main_offset;
#endif


s16	OPdet_Start(u8 buf_num ,ST_EdgEDetecT* ED)
{

	ST_BS* pbs = work[buf_num].pbs;

	u16 EdgeDetectionThreshold;
	u16 status = 0; /*エラーステータス*/

	s32 HalfLE;
	s32 HalfSE;
	float TopBottomDiff;
	//u32 threshold = 0; // スレッシュレベルの最大値制限

	Eg_det_ClearData(ED);							//初期化
	Eg_status_change(pbs,ED);	//解像度（dpi）に従ってステータスを変更します。

#if LE17_Manager	// LE17 Manager
	{
		//200との比
		float x_dpi = 200 / (25.4f / pbs->PlaneInfo[ED->Plane].main_element_pitch);

		//主走査オフセット計算 単位は論理座標（200ｘ200dpi）
		//CIS	　：搬送路の真ん中とセンサーの中心素子の位置合わせる
		//それ以外：CISの中心素子とそのセンサーの中心位置を合わせる
		main_offset = (s16)(((((pbs->PlaneInfo[ED->Plane].main_effective_range_max - pbs->PlaneInfo[ED->Plane].main_effective_range_min) * 0.5f)
			+ pbs->PlaneInfo[ED->Plane].main_effective_range_min) - (pbs->PlaneInfo[ED->Plane].main_offset * 0.5f)) * x_dpi);
	}
#endif

	//閾値決めてるとこ
	if(ED->th_dynamic_decision_flg == ED_run)
	{
		//EdgeDetectionThreshold = getEdgeDetectionThreshold((s16)threshold ,pbs);
		EdgeDetectionThreshold = line_chk_pix_val(ED->Sub_scan_min_val + 5 ,pbs ,ED);
	}
	else
	{
		if (pbs->PlaneInfo[ED->Plane].Ref_or_Trans == TRANSMISSION)//透過
		{
		EdgeDetectionThreshold = 0;	//255反転している
		}
		else
		{
			EdgeDetectionThreshold = 90;
		}
	}
	status = 0;
	//Main_scan_max_valはmainの最大ピクセル数、mMinSubScanLinesはsubの最大ピクセル数
	HalfLE = ED->Main_scan_max_val / 2;
	HalfSE = ED->Sub_scan_max_val / 2;

	//多重搬送検知 LE17のみ
	if(ED->multiple_transport == ED_run && pbs->PlaneInfo[ED->Plane].Ref_or_Trans == TRANSMISSION)
	{
		status |= chk_cont_trans(pbs, ED, (u8)EdgeDetectionThreshold);
		if ( status != 0 )
		{
			return status;
		}
	}


	// 上端検知
	status |= DetectTopEdge( HalfLE,		//xs
		ED->Sub_scan_min_val,			//ys 0
		HalfLE,			//xe
		HalfSE + 20,	//ye //+20 to increase the search height
		(u16)(ED->Step_Movement / pbs->PlaneInfo[ED->Plane].main_element_pitch),//ステップ幅
		EdgeDetectionThreshold  
		,pbs ,ED);
	if ( status != 0 )
	{
		return status;
	}

	//下端
	status |= DetectBottomEdge( HalfLE + 2,		// 上端の探索痕跡とデバッグ時に区別しやすいので+2している
		(ED->Sub_scan_max_val-1),//ブロック数拾ってきている200dpi仕様 
		HalfLE,//100だったのでハーフに変更しました
		HalfSE,//HalfSE - 20,//-20 to increase the search height // -20 検索の高さを上げるために20
		(u16)(ED->Step_Movement /pbs->PlaneInfo[ED->Plane].main_element_pitch),
		EdgeDetectionThreshold ,pbs ,ED);
	if ( status != 0 )
	{
		return status;
	}

	//左端
	status |= DetectLeftEdge( ED->Main_scan_min_val+1,
		(int)(ED->Top_Edge_offs + 15.0f),			//上端y切片
		HalfLE + 60,								// +40 to increase the search length
		(int)(ED->Bottom_Edge_offs),					//下端y切片
		(u16)(ED->Step_Movement / (25.4/(pbs->Subblock_dpi / pbs->PlaneInfo[ED->Plane].sub_sampling_pitch))),
		EdgeDetectionThreshold ,pbs ,ED);
	if ( status != 0 )
	{
		return status;
	}

	//右端xzxzczc
	status |= DetectRightEdge( ED->Main_scan_max_val,
		(int)(ED->Top_Edge_offs + (ED->Main_scan_max_val * ED->Top_Edge_tan_t) + 0) -10,
		HalfLE,//-40 to increase the search length
		(int)(ED->Bottom_Edge_offs + ED->Main_scan_max_val * ED->Bottom_Edge_tan_t) + 0,
		(u16)(ED->Step_Movement / (25.4/(pbs->Subblock_dpi / pbs->PlaneInfo[ED->Plane].sub_sampling_pitch))),
		EdgeDetectionThreshold ,pbs ,ED);
	if ( status != 0 )
	{
		return status;
	}

#define	EDGE_DIFF_LIMIT (0.025f)	// ４辺の角度差限度値(tan_t) // 0.02 = 1.145°, 0.01 = 0.57°（オリジナルでは3°としていた） 

	TopBottomDiff =( ED->Top_Edge_tan_t - ED->Bottom_Edge_tan_t) * ED->dpi_Correction;	//上辺と下辺の傾きの差分

	if ( fabs(TopBottomDiff) > EDGE_DIFF_LIMIT ) //差分絶対値が閾値よりも大きい？
	{
		float TopLeftAdd = (ED->Top_Edge_tan_t* ED->dpi_Correction)+ (ED->Left_Edge_tan_t / ED->dpi_Correction);	//上辺と左辺の傾きの差分を計算

		if (fabs(TopLeftAdd) > EDGE_DIFF_LIMIT)	
		{
			// Top Edge is bad
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Top_Edge_tan_t, Top_Edge_offs, 2 );
			FixEdge( &ED->Top_Edge_tan_t, &ED->Top_Edge_sin_t, &ED->Top_Edge_cos_t, &ED->Top_Edge_Deg, &ED->Top_Edge_offs, ED->Edge_Num_Points[TopEdge],
				ED->Edge_xpt[TopEdge], ED->Edge_ypt[TopEdge], ED->Bottom_Edge_tan_t, ED->Bottom_Edge_sin_t, ED->Bottom_Edge_cos_t, ED->Bottom_Edge_Deg,
				ED->Bottom_Edge_offs , TopEdge);  // -1 -> top line is in the negative direction from bottom line
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Top_Edge_tan_t, Top_Edge_offs, 0xFF );
		}
		else
		{
			// Bottom Edge is bad
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Bottom_Edge_tan_t, Bottom_Edge_offs, 2 );
			FixEdge( &ED->Bottom_Edge_tan_t, &ED->Bottom_Edge_sin_t, &ED->Bottom_Edge_cos_t, &ED->Bottom_Edge_Deg, &ED->Bottom_Edge_offs,
				ED->Edge_Num_Points[BottomEdge], ED->Edge_xpt[BottomEdge], ED->Edge_ypt[BottomEdge], ED->Top_Edge_tan_t, ED->Top_Edge_sin_t, ED->Top_Edge_cos_t,
				ED->Top_Edge_Deg, ED->Top_Edge_offs,BottomEdge);  // 1 -> bottom line is in the positive direction from top line
			//            SIM_INTERFACE_DrawLineY( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Bottom_Edge_tan_t, Bottom_Edge_offs, 0xFF );
		}
	}

	else
	{
		float LeftRightDiff =(ED-> Left_Edge_tan_t -ED-> Right_Edge_tan_t)/ ED->dpi_Correction;	//左辺と右辺の差分を計算
		if ( fabs(LeftRightDiff) > EDGE_DIFF_LIMIT)
		{
			float TopLeftAdd = ED->Top_Edge_tan_t* ED->dpi_Correction  + ED->Left_Edge_tan_t/ ED->dpi_Correction;	//上辺と左辺の差分を計算
			if ( fabs(TopLeftAdd) > EDGE_DIFF_LIMIT)
			{
				// Left Edge is bad
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Left_Edge_tan_t, Left_Edge_offs, 2 );
				FixEdge( &ED->Left_Edge_tan_t, &ED->Left_Edge_sin_t, &ED->Left_Edge_cos_t, &ED->Left_Edge_Deg, &ED->Left_Edge_offs,
					ED->Edge_Num_Points[LeftEdge], ED->Edge_ypt[LeftEdge], ED->Edge_xpt[LeftEdge], ED->Right_Edge_tan_t, ED->Right_Edge_sin_t,
					ED->Right_Edge_cos_t, ED->Right_Edge_Deg, ED->Right_Edge_offs,LeftEdge );
				// -1 -> left line is in the negative direction from right line
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Left_Edge_tan_t, Left_Edge_offs, 0xFF );
			}
			else
			{
				// Right edge is bad
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Right_Edge_tan_t, Right_Edge_offs, 2 );
				FixEdge( &ED->Right_Edge_tan_t, &ED->Right_Edge_sin_t, &ED->Right_Edge_cos_t, &ED->Right_Edge_Deg, &ED->Right_Edge_offs,
					ED->Edge_Num_Points[RightEdge], ED->Edge_ypt[RightEdge], ED->Edge_xpt[RightEdge], ED->Left_Edge_tan_t, ED->Left_Edge_sin_t,
					ED->Left_Edge_cos_t, ED->Left_Edge_Deg ,ED->Left_Edge_offs ,RightEdge);
				// 1 -> right line is in the positive direction from left line
				//                SIM_INTERFACE_DrawLineX( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, Right_Edge_tan_t, Right_Edge_offs, 0xFF );
			}
		}
	}

	// Vertex coordinate calculation:頂点座標計算
	ED->AlignedUpperLeft = calcu_xoyo( ED->Left_Edge_tan_t, ED->Left_Edge_offs, ED->Top_Edge_tan_t, ED->Top_Edge_offs ,pbs ,ED);// Upper Left
	ED->AlignedLowerLeft = calcu_xoyo( ED->Left_Edge_tan_t, ED->Left_Edge_offs, ED->Bottom_Edge_tan_t, ED->Bottom_Edge_offs ,pbs ,ED);// Lower Left
	ED->AlignedUpperRight = calcu_xoyo( ED->Right_Edge_tan_t, ED->Right_Edge_offs, ED->Top_Edge_tan_t, ED->Top_Edge_offs ,pbs ,ED);// Upper Right
	ED->AlignedLowerRight = calcu_xoyo( ED->Right_Edge_tan_t, ED->Right_Edge_offs, ED->Bottom_Edge_tan_t,ED-> Bottom_Edge_offs ,pbs ,ED); // Lower Right


	if(ED->vertex_err_determination == ED_run)
	{
		//頂点座標が搬送エリア外だった場合処理を終了する
		if( ED->AlignedUpperLeft.x < ED->Main_scan_min_val_ori + SIDE_OUT_AREA_RANGE ||
			ED->AlignedLowerLeft.x < ED->Main_scan_min_val_ori + SIDE_OUT_AREA_RANGE)
		{
			return ERR_TO_THE_LEFT;
		}

		if( ED->AlignedUpperRight.x > ED->Main_scan_max_val_ori - SIDE_OUT_AREA_RANGE ||
			ED->AlignedLowerRight.x > ED->Main_scan_max_val_ori - SIDE_OUT_AREA_RANGE)
		{
			return ERR_TO_THE_RIGHT;
		}

		if( ED->AlignedUpperLeft.y < ED->Main_scan_min_val_ori + SIDE_OUT_AREA_RANGE ||
			ED->AlignedUpperRight.y < ED->Main_scan_min_val_ori + SIDE_OUT_AREA_RANGE)
		{
			return ERR_TO_THE_UPPER;
		}

		if( ED->AlignedLowerLeft.y > (ED->Main_scan_max_val_ori - SIDE_OUT_AREA_RANGE) * ED->dpi_Correction ||
			ED->AlignedLowerRight.y > (ED->Main_scan_max_val_ori - SIDE_OUT_AREA_RANGE) * ED->dpi_Correction)
		{
			return ERR_TO_THE_LOWER;
		}
	}

	//出力計算(主幅、副幅、傾き、中心座標)
	calcu_size( ED->AlignedUpperLeft.x, ED->AlignedUpperLeft.y, ED->AlignedLowerLeft.x,ED->AlignedLowerLeft.y, ED->AlignedUpperRight.x,
		ED->AlignedUpperRight.y, ED->AlignedLowerRight.x, ED->AlignedLowerRight.y ,pbs->LEorSE ,ED);


	ED->length_dot = (int)ED->mBillLEPixelLength;
	ED->width_dot = (int)ED->mBillSEPixelLength; // pbs->pitch_ratio;
	ED->skew = ED->_deg;
	ED->tan_th = ED->_tan_t;
	ED->sin_th = ED->_sin_t;
	ED->cos_th = ED->_cos_t;//

	ED->AlignedBillCenterPoint.x = ED->_o.x;

	//求めた中心座標yに対して　dpi&offset補正を与える
	ED->AlignedBillCenterPoint.y = (ED->_o.y); // pbs->pitch_ratio;

	// Slope validation of the four sides
	//１辺の傾きの差の整合性
	status |= Edg_Deg_def_chk( ED );
	if ( status != 0 )
	{
		return status;
	}

	//傾きの整合性
	status |= deg_chk( ED);
	if ( status != 0 )
	{
		return status;
	}


	ED->SKEW = (short)(rndup(ED->tan_th * 4096.0f));

	return status;

}


void Eg_det_ClearData(ST_EdgEDetecT* ED )
{

	ED->skew = 0.0f;
	ED->tan_th = 0.0f;
	ED->sin_th = 0.0f;
	ED->cos_th = 0.0f;

	ED->length_dot = 0;
	ED->width_dot = 0;

	ED->AlignedBillCenterPoint.x = 0;
	ED->AlignedBillCenterPoint.y = 0;

	ED->length_dot = 0;
	ED->width_dot = 0;

	ED->AlignedBillCenterPoint.x = 0;
	ED->AlignedBillCenterPoint.y = 0;

	ED->length_mm = 0.0f;
	ED->width_mm = 0.0f;
	ED->adj_skew = 0.0f;

	ED->ofs_dbld_L1 = 0.0f;
	ED->ofs_dbld_L2 = 0.0f;

	ED->AlignedUpperLeft.x = 0;
	ED->AlignedUpperLeft.y = 0;

	ED->AlignedLowerRight.x = 0;
	ED->AlignedLowerRight.y = 0;

	ED->AlignedLowerLeft.x = 0;
	ED->AlignedLowerLeft.y = 0;

	ED->AlignedLowerRight.x = 0;
	ED->AlignedLowerRight.y = 0;

	ED->Top_Edge_Deg = 0.0f;
	ED->Bottom_Edge_Deg = 0.0f;
	ED->Left_Edge_Deg = 0.0f;
	ED->Right_Edge_Deg = 0.0f;

	ED->tan_t = 0.0f;
	ED->sin_t = 0.0f;
	ED->cos_t = 0.0f;
	ED->offs = 0.0f;

	ED->Top_Edge_tan_t = 0.0f;
	ED->Top_Edge_sin_t = 0.0f;
	ED->Top_Edge_cos_t = 0.0f;
	ED->Top_Edge_offs = 0.0f;

	ED->Bottom_Edge_tan_t = 0.0f;
	ED->Bottom_Edge_sin_t = 0.0f;
	ED->Bottom_Edge_cos_t = 0.0f;
	ED->Bottom_Edge_offs = 0.0f;

	ED->Left_Edge_tan_t = 0.0f;
	ED->Left_Edge_sin_t = 0.0f;
	ED->Left_Edge_cos_t = 0.0f;
	ED->Left_Edge_offs = 0.0f;

	ED->Right_Edge_tan_t = 0.0f;
	ED->Right_Edge_sin_t = 0.0f;
	ED->Right_Edge_cos_t = 0.0f;
	ED->Right_Edge_offs = 0.0f;

	ED->TopLeftCornerX = 0.0f;
	ED->TopLeftCornerY = 0.0f;
	ED->BottomLeftCornerX = 0.0f;
	ED->BottomLeftCornerY = 0.0f;
	ED->TopRightCornerX = 0.0f;
	ED->TopRightCornerY = 0.0f;
	ED->BottomRightCornerX = 0.0f;
	ED->BottomRightCornerY = 0.0f;
	ED->_o.x = 0;
	ED->_o.y = 0;
	ED->mBillSEPixelLength = 0.0f;
	ED->mBillLEPixelLength = 0.0f;
	ED->_deg = 0.0f;
	ED->_tan_t = 0.0f;
	ED->_sin_t = 0.0f;
	ED->_cos_t = 0.0f;
	ED->SKEW =0;

	memset( ED->Edge_xpt, 0, sizeof( ED->Edge_xpt ) );
	memset( ED->Edge_ypt, 0, sizeof( ED->Edge_ypt ) );
	memset( ED->Edge_Num_Points, 0, sizeof( ED->Edge_Num_Points ) );
}
//
//// Read Blocks of data around the image to determine the Edge Detection Threshold
//// This will get the min value of reading from the top of the image and the same point on the bottom of the image.
//// Because this function uses GetAveTopBottomPixel, the maximum from the top sensor and the bottom senor will be the actual value.
//// So we are actually reading 4 points and getting the min value of the highest of two points (top sensor, bottom sensor).
//// The Maximum values of the minimums will be used to calculate the threshold
//// The calculation for the threashold is 255 - ((255 - x) * 0.80). This inverts the low value to a high value and then inverts it back.
//
///*
//Edge Detection Thresholdを決定するために、画像の周りのデータのブロックを読み込みます。
//これにより、画像の上CISら読み取った最小値と、画像の下CISの同じ点が得られます。
//この関数はGetAveTopBottomPixelを使用するため、上部センサーと下部センサーの値の大きい方が実際の値になります。
//実際には4点を読み取り、2点のうち最高点の最小値を取得しています（上部センサ、下部センサ）。
//最小値の最大値は、しきい値を計算するために使用されます
//threasholdの計算は255 - （（255 - x）* 0.80）です。 これは、低い値を高い値に反転させ、逆にそれを逆転させる。
//*/
//u16 getEdgeDetectionThreshold( s16 MaxValue ,ST_BS *pbs)
//{
//	s32 x[3];
//	u16 Max = -1;
//	//T_Point TopPt, BottomPt;
//	ST_SPOINT TopPt;
//	ST_SPOINT BottomPt;
//	u16 i;
//
//	// Get a height that can be found on both the top and bottom sensors
//	// 上CISと下CISーの両方で使える有効イメージ領域の大きさを設定する。
//	s32 Height = ED->Sub_scan_max_val;	// 最dai副走査幅
//	s32 Width  = ED->Main_scan_max_val;//mMinMainScanBytes; 主走査幅
//
//	TopPt.p_plane_tbl = ED->Plane;
//	BottomPt.p_plane_tbl = ED->Plane;
//
//
//	//エリア選定式
//	x[0] = Width * 1/ 7 - 10;
//	//x[1] = Width * 2.5/ 5 - 10;
//	x[1] = Width * 6/ 7 - 10;
//
//	for ( i = 0; i < 2; i++ )
//	{
//		for ( TopPt.x = x[i], BottomPt.x = x[i]; TopPt.x < x[i] + 10; TopPt.x++, BottomPt.x++ )
//		{
//			for ( TopPt.y = 2, BottomPt.y = Height - 4; TopPt.y < 5; TopPt.y++, BottomPt.y++ )
//			{
//				//These values are intgers instead of unsigned char to handle the
//				//final calculation without overflow.
//
//				u16 ImageTop = ED_Pget(TopPt ,pbs);// GetAveTopBottomPixel( Gd_filter_Edge_Detect,  TopPt );			// 上CISと下CISの明るい方のデータ 上部ポイント
//				u16 ImageBottom = ED_Pget(BottomPt ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  BottomPt );	// 上CISと下CISの明るい方のデータ 下部ポイント
//				u16 Min1;
//
//				Min1 = ( ImageTop > ImageBottom ) ? ImageBottom : ImageTop;	// 暗い方
//
//				if ( Min1 > Max )
//				{
//					Max = Min1;		// 最大値保存
//				}
//			}
//		}
//	}
//
//	// 80% change requirED-> 255 - ((255 - x) * 0.80) optimizes to the following Max値に255値までの量の20％を加えた値をスレッシュレベルとする
//	Max = 20 + Max * 10 / 10;
//
//
//	if ( Max < MaxValue )	// 返り値の最大値を制限する
//	{
//		return Max;
//	}
//	return MaxValue;
//
//
//}

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
u16 DetectTopEdge( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold ,ST_BS *pbs ,ST_EdgEDetecT* ED)
{

	int n = 0;
	int ErrorCount = 0;
	int ErrorCountMax = ERR_MAX_COUNT; // 探査エリアY方向開始位置で初めから券有を検出した時のエラーとする回数


	int yss;// = yStart;
	int Index =	MAX_POINTS / 2;//　50 / 2
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

		Status = DetectTopEdgeYLoop(x,
			&yss,	// Y 開始座標
			yEnd,	// Y 終了座標
			&n,
			FindThreshold,
			&ED->Edge_xpt[TopEdge][Index],
			&ED->Edge_ypt[TopEdge][Index],
			&ErrorCount, ErrorCountMax
			,pbs ,ED);
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND )
			{
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				//ポイントを見つけられなかった場合は、札のx方向の最後にあると仮定します
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON )
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
		if ( Index >= MAX_POINTS-1 )
		{
			break;
		}
		Index++;
	}

	yss = (int)ED->Edge_ypt[TopEdge][MAX_POINTS / 2] - ED->Small_Backtrack;	// 前の結果を使って算出
	//    yss = (int)Edge_ypt[TopEdge][0] - ED->Small_Backtrack;	// 前の結果を使って算出
	Index = MAX_POINTS / 2 - 1;
	for ( x = xStart - xStep; x > xStart - xHalfLength; x -= xStep )	// 左側半分の探査
	{
		if ( yss < yStart )
		{
			yss = yStart;
		}

		Status = DetectTopEdgeYLoop( x, &yss, yEnd, &n, FindThreshold, &ED->Edge_xpt[TopEdge][Index], &ED->Edge_ypt[TopEdge][Index], &ErrorCount, ErrorCountMax ,pbs ,ED);
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND )
			{
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON )
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
		//       if ( Index >= MAX_POINTS )
		if ( Index <= 0 )
		{
			break;
		}
		Index--;
	}

	ED->ScanEdgeNumPoints[TopEdge] = n;	// 券端位置リストの外れ値削除前の有効数のセット
	ED->Edge_Num_Points[TopEdge] = n;	// 券端位置リストの有効数のセット

	// Because the Index of Edge_xpt and Edge_ypt starts in the middle, the first values should be zero.
	// Find the first real value, and copy from that point of the array to the beginning of the array
	// Edge_xptとEdge_yptのインデックスが途中から始まるため、最初の値はゼロになるはずです。
	//最初の実数値を見つけ、配列のその点から配列の先頭にコピーする
	FirstValue = 0;
	for ( i = 0; i < MAX_POINTS; i++ )
	{
		if ( ED->Edge_xpt[TopEdge][i] != 0 )
		{
			FirstValue = i;
			break;
		}
	}
	memmove( ED->Edge_xpt[TopEdge], &ED->Edge_xpt[TopEdge][FirstValue], sizeof( ED->Edge_xpt[TopEdge][0] ) * ( MAX_POINTS - FirstValue ) );	// 配列内データを前に詰める
	memmove( ED->Edge_ypt[TopEdge], &ED->Edge_ypt[TopEdge][FirstValue], sizeof( ED->Edge_ypt[TopEdge][0] ) * ( MAX_POINTS - FirstValue ) );	// 配列内データを前に詰める

	memcpy( ED->ScanEdgeXPoints[TopEdge], ED->Edge_xpt[TopEdge], sizeof(ED-> Edge_xpt[TopEdge][0] ) * MAX_POINTS );	// 外れ値排除前のデータを保存　MAXよりｎの方がいいのでは？
	memcpy( ED->ScanEdgeYPoints[TopEdge], ED->Edge_ypt[TopEdge], sizeof(ED-> Edge_ypt[TopEdge][0] ) * MAX_POINTS );	// 外れ値排除前のデータを保存

	SortAndRemoveOutliers( ED->Edge_ypt[TopEdge], ED->Edge_xpt[TopEdge], &ED->Edge_Num_Points[TopEdge]);

	if (ED-> Edge_Num_Points[TopEdge] < MINIMUM_NUMBERS_OF_POINTS )
	{
		
		return ERR_TOO_FEW_EDGE_POINTS_FOUND;
	}

	mask(ED-> Edge_xpt[TopEdge], ED->Edge_ypt[TopEdge], ED->Edge_Num_Points[TopEdge] ,pbs,ED);

	ED->Top_Edge_tan_t = ED->tan_t;
	ED->Top_Edge_sin_t = ED->sin_t;
	ED->Top_Edge_cos_t = ED->cos_t;
	ED->Top_Edge_offs = ED->offs + 1;

	ED->Top_Edge_Deg = (float)(atanf( (float)(ED->Top_Edge_tan_t  * ED->dpi_Correction)) * PIRAD);

	return 0;
}

// handles edge checking in the y direction, going up (DetectBottomEdge) or down (DetectTopEdge)
unsigned short DetectTopEdgeYLoop(
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
	ST_BS *pbs ,ST_EdgEDetecT* ED)
{
	unsigned short Result = WARN_EDGE_NOT_FOUND;
	//    T_Point AlignedPoint;
	//    T_Point FollowUpTest;
	int LastPixelValue = 0;
	const int yStep = 1;
	ST_SPOINT AlignedPoint;
	ST_SPOINT FollowUpTest;

	//プレーンだけセット
	AlignedPoint.p_plane_tbl = ED->Plane;
	FollowUpTest.p_plane_tbl = ED->Plane;


	AlignedPoint.x = x;
	for ( AlignedPoint.y = *yStart; AlignedPoint.y < yEnd; AlignedPoint.y += yStep )
	{
		int PixelValue = ED_Pget(AlignedPoint ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  AlignedPoint );
		if ( PixelValue > FindThreshold )	// 券有りレベル検出の時				
		{
			// Test next few pixels to ensure we are not looking at corrupted data
			//次の数ピクセルをテストして、破損したデータを確認しないようにする
			u8 FalseEdgeDetected = 0;
			FollowUpTest.x = AlignedPoint.x;
			for ( FollowUpTest.y = AlignedPoint.y + yStep; ( FollowUpTest.y <= AlignedPoint.y + ( yStep * NUM_ADDITIONAL_PIXEL_TEST ) ) && ( FollowUpTest.y < yEnd );
				FollowUpTest.y += yStep )
			{
				int NewPixelValue = ED_Pget(FollowUpTest ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  FollowUpTest );
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
				Result = WARN_EDGE_FOUND_TOO_SOON;//WARN EDGEがすぐに見つかりました。
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
			// 次のラウンドでは、この走査の検出点のED->Small_Backtrackピクセル上から開始する
			*yStart = AlignedPoint.y - ED->Small_Backtrack;
			if ( ++*EdgePointFoundCount >= MAX_POINTS )
			{
				// err
				
				return ERR_TOO_MANY_EDGE_POINTS_FOUND;	// MAX_POINTS 以上券端が見つかったときは、配列容量を超えるのでエラーとする。
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
		
		return ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
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
unsigned short DetectBottomEdge( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold ,ST_BS *pbs ,ST_EdgEDetecT* ED)
{
	int n = 0;
	int ErrorCount = 0, ErrorCountMax = ERR_MAX_COUNT;

	int yss = yStart;
	int Index = MAX_POINTS / 2;
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

		Status = DetectBottomEdgeYLoop( x, &yss, yEnd, &n, FindThreshold, &ED->Edge_xpt[BottomEdge][Index], &ED->Edge_ypt[BottomEdge][Index], &ErrorCount, ErrorCountMax ,pbs,ED);
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND )
			{
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON )
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
		if ( Index >= MAX_POINTS )
		{
			break;
		}
	}

	yss = (int)ED->Edge_ypt[BottomEdge][MAX_POINTS / 2] + ED->Small_Backtrack;
	//    yss = (int)Edge_ypt[BottomEdge][0] + ED->Small_Backtrack;   // 前の結果を使って算出
	Index = MAX_POINTS / 2 - 1;
	for ( x = xStart - xStep; x > xStart - xHalfLength; x -= xStep )
	{
		if ( yss > yStart )
		{
			yss = yStart;
		}

		Status = DetectBottomEdgeYLoop( x, &yss, yEnd, &n, FindThreshold, &ED->Edge_xpt[BottomEdge][Index], &ED->Edge_ypt[BottomEdge][Index], &ErrorCount, ErrorCountMax ,pbs,ED);
		if ( 0 != Status )
		{
			if ( Status == WARN_EDGE_NOT_FOUND )
			{
				// If we didn't find a point, assume we are at the end of the bill in the x direction
				//                SIM_INTERFACE_EdgeDetectShowRemovedPoint( mGraphicsInterface, mGetData, x, yEnd, false );
				break;
			}
			if ( Status == WARN_EDGE_FOUND_TOO_SOON )
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
		//        if ( Index >= MAX_POINTS )
		if ( Index < 0 )
		{
			break;
		}
	}

	ED->ScanEdgeNumPoints[BottomEdge] = ED->Edge_Num_Points[BottomEdge] = n;

	// Because the Index of Edge_xpt and Edge_ypt starts in the middle, the first values should be zero.
	// Find the first real value, and copy from that point of the array to the beginning of the array
	FirstValue = 0;
	for ( i = 0; i < MAX_POINTS; i++ )
	{
		if ( ED->Edge_xpt[BottomEdge][i] != 0 )
		{
			FirstValue = i;
			break;
		}
	}

	memmove( ED->Edge_xpt[BottomEdge], &ED->Edge_xpt[BottomEdge][FirstValue], sizeof( ED->Edge_xpt[BottomEdge][0] ) * ( MAX_POINTS - FirstValue ) );
	memmove( ED->Edge_ypt[BottomEdge], &ED->Edge_ypt[BottomEdge][FirstValue], sizeof( ED->Edge_ypt[BottomEdge][0] ) * ( MAX_POINTS - FirstValue ) );
	memcpy( ED->ScanEdgeXPoints[BottomEdge], ED->Edge_xpt[BottomEdge], sizeof( ED->Edge_xpt[BottomEdge][0] ) * MAX_POINTS );
	memcpy( ED->ScanEdgeYPoints[BottomEdge], ED->Edge_ypt[BottomEdge], sizeof( ED->Edge_ypt[BottomEdge][0] ) * MAX_POINTS );
	SortAndRemoveOutliers( ED->Edge_ypt[BottomEdge], ED->Edge_xpt[BottomEdge], &ED->Edge_Num_Points[BottomEdge] );

	if ( ED->Edge_Num_Points[BottomEdge] < MINIMUM_NUMBERS_OF_POINTS )
	{
		
		return ERR_TOO_FEW_EDGE_POINTS_FOUND;
	}
	mask( ED->Edge_xpt[BottomEdge], ED->Edge_ypt[BottomEdge], ED->Edge_Num_Points[BottomEdge] ,pbs,ED);

	ED->Bottom_Edge_tan_t = ED->tan_t;
	ED->Bottom_Edge_sin_t = ED->sin_t;
	ED->Bottom_Edge_cos_t = ED->cos_t;
	ED->Bottom_Edge_offs = ED->offs + 1;

	ED->Bottom_Edge_Deg = (float)(atanf( (float)(ED->Bottom_Edge_tan_t * ED->dpi_Correction )) * PIRAD);

	return 0;
}

// handles edge checking in the y direction, going up (DetectBottomEdge) or down (DetectTopEdge)
unsigned short DetectBottomEdgeYLoop( int x, // The x offset for this line
	int *yStart, // The starting point for scanning. This is passed back since the next scan line is based on the detection of this scan line
	int yEnd, // The last point to look for an edge
	int *EdgePointFoundCount, // Running total of the number of edges found
	int FindThreshold, // Pixel value threshold for identifying the edge
	float *XPoint, // X from List of (X,Y) points where the edge was detected
	float *YPoint, // Y from List of (X,Y) points where the edge was detected
	int *ErrorCount, // Running total of edge starting too soon errors
	int ErrorCountMax // Maximum number of ErrorCount before an actual error is thrown
	,ST_BS *pbs ,ST_EdgEDetecT* ED)
{
	unsigned short Result = WARN_EDGE_NOT_FOUND;
	//    T_Point AlignedPoint;
	ST_SPOINT AlignedPoint;
	ST_SPOINT FollowUpTest;
	int LastPixelValue = 0;
	const int yStep = 1;
	u8 FalseEdgeDetected = 0;

	AlignedPoint.p_plane_tbl = ED->Plane; // 200*200dpi Ref Red
	FollowUpTest.p_plane_tbl = ED->Plane; // 200*200dpi Ref Red

	AlignedPoint.x = x;
	for ( AlignedPoint.y = *yStart; AlignedPoint.y > yEnd; AlignedPoint.y -= yStep )
	{
		int PixelValue =  ED_Pget(AlignedPoint ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  AlignedPoint );
		if ( PixelValue > FindThreshold )
		{
			// Test next few pixels to ensure we are not looking at corrupted data
			//            T_Point FollowUpTest;
			FalseEdgeDetected = 0;//false
			FollowUpTest.x = AlignedPoint.x;
			for ( FollowUpTest.y = AlignedPoint.y - yStep;
				( FollowUpTest.y >= AlignedPoint.y - ( yStep * NUM_ADDITIONAL_PIXEL_TEST ) ) && ( FollowUpTest.y > yEnd );
				FollowUpTest.y -= yStep )
			{
				int NewPixelValue = ED_Pget(FollowUpTest,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  FollowUpTest );
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
				Result = WARN_EDGE_FOUND_TOO_SOON;
				*ErrorCount += 1;
				//*ErrorCount++;
				break;
			}
			// Found a pooint

			//         SIM_INTERFACE_TraceAlignedLine( mGraphicsInterface, mGetData, Gd_filter_Edge_Detect, x, yStart, x, AlignedPoint.y, 0xFF );

			// Scales the pixel point back to a float value
			//*YPoint = AlignedPoint.y + 1 - ( FindThreshold - LastPixelValue ) * yStep / ( ( float )( PixelValue - LastPixelValue ) );
			*YPoint = AlignedPoint.y - ( FindThreshold - LastPixelValue ) * yStep / ( ( float )( PixelValue - LastPixelValue ) );
			*XPoint = (float)AlignedPoint.x;

			*yStart = AlignedPoint.y + ED->Small_Backtrack;
			if ( ++*EdgePointFoundCount >= MAX_POINTS )
			{
				
				return ERR_TOO_MANY_EDGE_POINTS_FOUND;
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
		
		//return *ErrorCount;//ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
		return ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
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
unsigned short DetectLeftEdge( int xStart, int yStart, int xEnd, int yEnd, int yStep, int FindThreshold,ST_BS *pbs ,ST_EdgEDetecT* ED )
{
	//    T_Point pt;
	//    T_Point FollowUpTest;
	ST_SPOINT pt;
	ST_SPOINT FollowUpTest;
	int n = 0;
	int e_co = 0, e_co_max = 10;
	int xss;
	u8 FoundEdge = 0;//false;
	u8 FalseEdgeDetected = 0;//false;
	float dx;

	pt.p_plane_tbl = ED->Plane;
	FollowUpTest.p_plane_tbl = ED->Plane; 

	//This is to ensure the minimum starting point is 20 pixels down
	//from the starting of the effective subscan line.
	yStart = yStart < 20 ? 20 : yStart;
	yStart = yStart > ED->Sub_scan_max_val - (ED->Sub_scan_max_val / 4) ? (int)(ED->Sub_scan_max_val / 1.5f) : yStart;


	//This is to ensure searching only inside of the effective subscan lines.
	yEnd = yEnd > ED->Sub_scan_max_val ? ED->Sub_scan_max_val : yEnd;
	yEnd = yEnd < (ED->Sub_scan_max_val / 4) ? (int)(ED->Sub_scan_max_val / 3.5f) : yEnd;
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
			int PixelValue = ED_Pget(pt,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  pt );

			if ( PixelValue > FindThreshold )
			{
				// Test next few pixels to ensure we are not looking at corrupted data
				FalseEdgeDetected = 0;//false;
				FollowUpTest.y = pt.y;
				for ( FollowUpTest.x = pt.x + 1; ( FollowUpTest.x <= pt.x + NUM_ADDITIONAL_PIXEL_TEST ) && ( FollowUpTest.x < xEnd ); FollowUpTest.x++ )
				{
					int NewPixelValue = ED_Pget(FollowUpTest,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  FollowUpTest );
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
						if(ED->abnormal_position_of_ticket_edge == ED_run)
						{
							e_co++;
							break;
						}

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

				ED->Edge_ypt[LeftEdge][n] = dx;
				ED->Edge_xpt[LeftEdge][n] = (float)pt.y;

				xss = pt.x - ED->Small_Backtrack;
				if ( ++n >= MAX_POINTS )
				{
					
					return ERR_TOO_MANY_EDGE_POINTS_FOUND;
				}
				break;
			}
			LastPixelValue = PixelValue;
		}
		if ( !FoundEdge )
		{
			xss = xStart;
		}

		if(ED->abnormal_position_of_ticket_edge == ED_run)
		{
			if ( e_co >= e_co_max )
			{

				return ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
			}
		}



	}

	ED->ScanEdgeNumPoints[LeftEdge] = ED->Edge_Num_Points[LeftEdge] = n;

	memcpy( ED->ScanEdgeXPoints[LeftEdge], ED->Edge_xpt[LeftEdge], sizeof( ED->Edge_xpt[LeftEdge][0] ) * MAX_POINTS );
	memcpy( ED->ScanEdgeYPoints[LeftEdge], ED->Edge_ypt[LeftEdge], sizeof( ED->Edge_ypt[LeftEdge][0] ) * MAX_POINTS );
	SortAndRemoveOutliers( ED->Edge_ypt[LeftEdge], ED->Edge_xpt[LeftEdge], &ED->Edge_Num_Points[LeftEdge]);

	if ( ED->Edge_Num_Points[LeftEdge] < MINIMUM_NUMBERS_OF_POINTS )
	{
		
		return ERR_TOO_FEW_EDGE_POINTS_FOUND;
	}

	mask( ED->Edge_xpt[LeftEdge], ED->Edge_ypt[LeftEdge], ED->Edge_Num_Points[LeftEdge],pbs,ED);

	ED->Left_Edge_offs = ED->offs + 1;
	ED->Left_Edge_tan_t = ED->tan_t;
	ED->Left_Edge_sin_t = ED->sin_t;
	ED->Left_Edge_cos_t = ED->cos_t;
	ED->Left_Edge_Deg = (float)(atan( ED->Left_Edge_tan_t   / ED->dpi_Correction) * PIRAD);

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
unsigned short DetectRightEdge( int xStart, int yStart, int xEnd, int yEnd, int yStep, int FindThreshold ,ST_BS *pbs ,ST_EdgEDetecT* ED)
{
	//    T_Point pt;
	//    T_Point FollowUpTest;
	ST_SPOINT pt;
	ST_SPOINT FollowUpTest;
	int n = 0;
	int e_co = 0, e_co_max = 10;
	int xss;
	u8 FoundEdge = 0;//false;
	u8 FalseEdgeDetected = 0;//false;
	int NewPixelValue;
	float dx;
	pt.p_plane_tbl = ED->Plane; // 200*200dpi Ref Red
	FollowUpTest.p_plane_tbl = ED->Plane; // 200*200dpi Ref Red


	//This is to ensure the minimum starting point is 20 pixels down
	//from the starting of the effective subscan line.
	yStart = yStart < 20 ? 20 : yStart;
	yStart = yStart > ED->Sub_scan_max_val - (ED->Sub_scan_max_val / 4) ? (int)(ED->Sub_scan_max_val / 1.5f) : yStart;

	//This is to ensure searching only inside of the effective subscan lines.
	yEnd = yEnd > ED->Sub_scan_max_val ? ED->Sub_scan_max_val : yEnd;
	yEnd = yEnd < (ED->Sub_scan_max_val / 4) ? (int)(ED->Sub_scan_max_val / 3.5f) : yEnd;

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
			int PixelValue = ED_Pget(pt,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect,  pt );
			if ( PixelValue > FindThreshold )
			{
				// Test next few pixels to ensure we are not looking at corrupted data
				FalseEdgeDetected = 0;//false;
				FollowUpTest.y = pt.y;
				for ( FollowUpTest.x = pt.x - 1; ( FollowUpTest.x >= pt.x - NUM_ADDITIONAL_PIXEL_TEST ) && ( FollowUpTest.x > xEnd );FollowUpTest.x-- )
				{
					NewPixelValue = ED_Pget(FollowUpTest ,pbs);//GetAveTopBottomPixel( Gd_filter_Edge_Detect, FollowUpTest );
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
						if(ED->abnormal_position_of_ticket_edge == ED_run)
						{
							e_co++;
							break;
						}

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

				ED->Edge_ypt[RightEdge][n] = dx;
				ED->Edge_xpt[RightEdge][n] = (float)pt.y;

				xss = pt.x + ED->Small_Backtrack;
				if ( ++n >= MAX_POINTS )
				{
					// err
					
					return ERR_TOO_MANY_EDGE_POINTS_FOUND;
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
		if(ED->abnormal_position_of_ticket_edge == ED_run)
		{
			if ( e_co >= e_co_max )
			{

				return ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA;
			}
		}
	}

	ED->ScanEdgeNumPoints[RightEdge] = ED->Edge_Num_Points[RightEdge] = n;
	memcpy( ED->ScanEdgeXPoints[RightEdge], ED->Edge_xpt[RightEdge], sizeof( ED->Edge_xpt[RightEdge][0] ) * MAX_POINTS );
	memcpy( ED->ScanEdgeYPoints[RightEdge], ED->Edge_ypt[RightEdge], sizeof( ED->Edge_ypt[RightEdge][0] ) * MAX_POINTS );
	SortAndRemoveOutliers( ED->Edge_ypt[RightEdge], ED->Edge_xpt[RightEdge], &ED->Edge_Num_Points[RightEdge]);

	if ( ED->Edge_Num_Points[RightEdge] < MINIMUM_NUMBERS_OF_POINTS )
	{
		
		return ERR_TOO_FEW_EDGE_POINTS_FOUND;
	}

	mask( ED->Edge_xpt[RightEdge], ED->Edge_ypt[RightEdge], ED->Edge_Num_Points[RightEdge] ,pbs,ED);

	ED->Right_Edge_offs = ED->offs + 1; 
	ED->Right_Edge_tan_t = ED->tan_t  ;
	ED->Right_Edge_sin_t = ED->sin_t;
	ED->Right_Edge_cos_t = ED->cos_t;
	ED->Right_Edge_Deg = (float)(atan( ED->Right_Edge_tan_t / ED->dpi_Correction) * PIRAD);

	return 0;
}

// Does the calculations to fix an identified bad edge.
// The algorithm finds the maximum distance from the good side and the bad side.
// That maximum distance is used to find the corrected offset.
// The tan_t is copied from the good edge to the bad edge. (They are assumed to be parallel)
//識別された不良エッジを修正するための計算を行います。
//アルゴリズムは、良い側と悪い側からの最大距離を求めます。
//修正されたオフセットを見つけるために、その最大距離が使用されます。
// tan_tは良いエッジから悪いエッジにコピーされます。（それらは平行であると仮定される）
void FixEdge( float *Bad_Edge_tan_t, float *Bad_Edge_sin_t, float *Bad_Edge_cos_t, float *Bad_Edge_Deg, float *Bad_Edge_offs,
	const int Bad_Edge_Num_Points, const float * const BadEdgePointsX, const float * const BadEdgePointsY, const float Good_Edge_tan_t,
	const float Good_Edge_sin_t, const float Good_Edge_cos_t, const float Good_Edge_Deg, const float Good_Edge_offset ,u8 flg)
{
	int Index = 0;
	u8  idx = 0;
	float Distance;
	//float Distance_debug1[100];
	//float Distance_debug2[100]; 
	float offset = 0.0f;
	float MaxDistance = 0.0;
	//float ax[100];
	//float by[100];
	//float denominator;
	//float molecule;
	//float tan = (float)(PIRAD * atan(Good_Edge_Deg)) ;
	//float tan = Good_Edge_tan_t;
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

	//初期値を設定　辺によっては最小値を探すので
	//主走査副走査の最大値を設定する。
	if(flg == TopEdge)
	{
		MaxDistance = 0;

		for ( Index = 0; Index < Bad_Edge_Num_Points; Index++ )
		{
			//特異点削除の際、特異点は-1でマスクされるのでそういったポイントは処理しないためのcontinue
			if(BadEdgePointsX[Index] < 0 || BadEdgePointsY[Index] < 0)
			{
				continue;
			}

			////辺の最大値もしくは最小値を求める
			////辺の座標は既定座標から凹むことはあっても飛び出ることはないと考え、
			////最大値の座標と正しい辺の傾きを用いて正しい切片を計算する。
			//if(BadEdgePointsY[Index] < MaxDistance)
			//{
			//	MaxDistance = BadEdgePointsY[Index];
			//	idx = (u8)Index;
			//}

			Distance = fabsf(Good_Edge_tan_t * BadEdgePointsX[Index] - BadEdgePointsY[Index] + Good_Edge_offset) / sqrtf(Good_Edge_tan_t * Good_Edge_tan_t + 1);

			//if(BadEdgePointsX[Index] < MaxDistance )
			if(Distance > MaxDistance )
			{
				//MaxDistance = BadEdgePointsX[Index];
				idx = (u8)Index;
				MaxDistance = Distance;
			}

		}

	}

	else if(flg == BottomEdge)
	{
		MaxDistance = 0;

		for ( Index = 0; Index < Bad_Edge_Num_Points; Index++ )
		{
			if(BadEdgePointsX[Index] < 0 || BadEdgePointsY[Index] < 0)
			{
				continue;
			}

			Distance = fabsf(Good_Edge_tan_t * BadEdgePointsX[Index] - BadEdgePointsY[Index] + Good_Edge_offset) / sqrtf(Good_Edge_tan_t * Good_Edge_tan_t + 1);

			//if(BadEdgePointsX[Index] < MaxDistance )
			if(Distance > MaxDistance )
			{
				//MaxDistance = BadEdgePointsX[Index];
				idx = (u8)Index;
				MaxDistance = Distance;
			}
		}
	}

	else if(flg == LeftEdge)
	{
		//MaxDistance = (float)ED->Main_scan_max_val;
		MaxDistance = 0;

		for ( Index = 0; Index < Bad_Edge_Num_Points; Index++ )
		{
			if(BadEdgePointsX[Index] < 0 || BadEdgePointsY[Index] < 0)
			{
				continue;
			}

			Distance = fabsf(Good_Edge_tan_t * BadEdgePointsY[Index] - BadEdgePointsX[Index] + Good_Edge_offset) / sqrtf(Good_Edge_tan_t * Good_Edge_tan_t + 1);

			//if(BadEdgePointsX[Index] < MaxDistance )
			if(Distance > MaxDistance )
			{
				//MaxDistance = BadEdgePointsX[Index];
				idx = (u8)Index;
				MaxDistance = Distance;
			}
		}
	}

	else if(flg == RightEdge)
	{
		MaxDistance = 0;

		for ( Index = 0; Index < Bad_Edge_Num_Points; Index++ )
		{
			if(BadEdgePointsX[Index] < 0 || BadEdgePointsY[Index] < 0)
			{
				continue;
			}

			Distance = fabsf(Good_Edge_tan_t * BadEdgePointsY[Index] - BadEdgePointsX[Index] + Good_Edge_offset) / sqrtf(Good_Edge_tan_t * Good_Edge_tan_t + 1);

			//if(BadEdgePointsX[Index] < MaxDistance )
			if(Distance > MaxDistance )
			{
				//MaxDistance = BadEdgePointsX[Index];
				idx = (u8)Index;
				MaxDistance = Distance;
			}

			//if(BadEdgePointsX[Index] > MaxDistance )
			//{
			//	MaxDistance = BadEdgePointsX[Index];
			//	idx = (u8)Index;
			//}
		}
	}


	//点と線の距離を求める方式
	for ( Index = 0; Index < Bad_Edge_Num_Points; Index++ )
	{
		//特異点削除の際、特異点は-1でマスクされるのでそういったポイントは処理しないためのcontinue
		if(BadEdgePointsX[Index] < 0 || BadEdgePointsY[Index] < 0)
		{
			continue;
		}

		//dpiのちがいによりDistanceの値が変になる。(11/21)
		//Distance = (abs((Good_Edge_tan_t * BadEdgePointsX[Index]) - (BadEdgePointsY[Index]) + Good_Edge_offs) ) / (sqrt(Good_Edge_tan_t * Good_Edge_tan_t + 1));
		//Distance_debug1[Index]= (abs((Good_Edge_tan_t * BadEdgePointsX[Index]) + (BadEdgePointsY[Index] * -1) + Good_Edge_offs) ) / (sqrt(Good_Edge_tan_t * Good_Edge_tan_t + 1));

		//Distance = (float)fabs( BadEdgePointsY[Index] - Good_Edge_offs - Good_Edge_tan_t * fabs( BadEdgePointsX[Index] ) ) * Good_Edge_cos_t; // 直線と点の最短距離
		//Distance =(float)fabs( BadEdgePointsY[Index] - Good_Edge_offs - Good_Edge_tan_t * fabs( BadEdgePointsX[Index] ) ) * Good_Edge_cos_t; // 直線と点の最短距離

		//Distance = (float)abs((tan * BadEdgePointsX[Index]) - (BadEdgePointsY[Index]) + Good_Edge_offs) / sqrt(tan * tan + 1); 

		//ax[Index] = BadEdgePointsX[Index] * tan;
		//by[Index] = BadEdgePointsY[Index] * -1.0f;

		//denominator = sqrtf(tan * tan + 1.0f);
		//molecule = (float)fabs((BadEdgePointsX[Index] * tan) - BadEdgePointsY[Index]  + Good_Edge_offs);
		
		//Distance = molecule / denominator; // 直線と点の最短距離
		//Distance_debug2[Index] = Distance;

		//if ( Distance > MaxDistance )
		//{
		//	MaxDistance = Distance;
		//}
	}

	//badedgeの切片を求める
	if(flg == LeftEdge || flg == RightEdge )
	{
		offset = BadEdgePointsX[idx] - (BadEdgePointsY[idx] * Good_Edge_tan_t);
	}
	else
	{
		offset = BadEdgePointsY[idx] - (BadEdgePointsX[idx] * Good_Edge_tan_t);
	}

	// Copy the angles from the good side
	*Bad_Edge_tan_t = Good_Edge_tan_t;
	*Bad_Edge_sin_t = Good_Edge_sin_t;
	*Bad_Edge_cos_t = Good_Edge_cos_t;
	*Bad_Edge_Deg = Good_Edge_Deg;
	// The Offset of the bad line is the maximum distance +/- the Offset of the good line
	//*Bad_Edge_offs = Good_Edge_offs + (DiffDirection * MaxDistance);
	*Bad_Edge_offs = offset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Masks the singularity
// 特異点をフィルターする
void mask( float* xpt, float* ypt, int num, ST_BS *pbs ,ST_EdgEDetecT* ED )
{
	int i = 0;
	#ifdef VS_DEBUG_EDGE
	int ii = 0;
	#endif
	float main_dot = pbs->PlaneInfo[ED->Plane].main_element_pitch;
	float sub_dot =25.4f / (200 / pbs->PlaneInfo[ED->Plane].sub_sampling_pitch);

	for ( i = 0; i < 100; i++ )
	{
		calu_kaiki( xpt, ypt, num ,ED); // Once back kaesa affiliates, linear approximation// 一次回帰による、直線近似
		if ( 0 == mask_errdt( xpt, ypt, num, ED->tan_t, (int)ED->offs, 0.5 ,main_dot ,sub_dot))
			{

#ifdef VS_DEBUG_EDGE
			for( ii = 0; ii < num; ii++ )
			{

				deb_para[0] = 4;		// function code
				deb_para[1] = xpt[ii];	// 
				deb_para[2] = ypt[ii];	//
				deb_para[3] = 1;		// plane
				callback(deb_para);		// debug

				deb_para[1] = ypt[ii];	// 
				deb_para[2] = xpt[ii];	//

				callback(deb_para);		// debug
			}
#endif


			break; // break  mask the data of the maximum value. Break without a mask// 最大差異のデータをフィルターする。 フィルターなしでbreak
		}
	}
}

// In a return kaesa ni Proceeds from the straight-line approximation
// xp, yp, n ---> tan, sin, cos, offset
// 一次回帰による直線近似
// 配列xp, yp, n ---> tan, sin, cos, offset
void calu_kaiki( float* xpt, float* ypt, int num ,ST_EdgEDetecT* ED )
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
	ED->tan_t = (float)(( mxy - NumPoints * ( xAve * yAve ) ) / ( xSquaredSum - NumPoints * xAve * xAve )); // katamuki
	// See http://mathworld.wolfram.com/LeastSquaresFitting.html, Lines 13 (Least Squares Fitting)
	ED->offs = (float)(( yAve * xSquaredSum - xAve * mxy ) / ( xSquaredSum - NumPoints * xAve * xAve )); // offset y

	ED->sin_t = (float)(ED->tan_t / sqrt( 1.0f + ED->tan_t * ED->tan_t )); // tan_t = sin_t/cos_t
	ED->cos_t = (float)(sqrt( 1.0f - ED->sin_t * ED->sin_t )); // sin_t * sin_t + cos_t * cos_t = 1
}

//// 最大差異のデータをフィルターする。
// xp, yp, n, tan_t, offset, dfmax
int mask_errdt( float* xpt, float* ypt, int num, float tan_t, int offset, float deflim ,float main_dot ,float sub_dot)
{
	int	i;
	float dfmax = -1;
	float dfthr_main;
	float dfthr_sub;
	int dfpt = 0;
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
T_Point_float calcu_xoyo( float tan_t1, float ofsx,  float tan_t2, float ofsy ,ST_BS *pbs ,ST_EdgEDetecT* ED)
{
	T_Point_float Po;

	Po.y = ( tan_t2 * ofsx + ofsy ) / ( 1.0f - tan_t2 * tan_t1 ) * ED->dpi_Correction / pbs->pitch_ratio;
	//Po.y = ( tan_t2 * ofsx + ofsy ) / ( 1.0f - tan_t2 * tan_t1 ) / pbs->pitch_ratio;
	Po.x = ( tan_t1 * ofsy + ofsx ) / ( 1.0f - tan_t1 * tan_t2 );

#ifdef VS_DEBUG_EDGE

	deb_para[0] = 3;		// function code
	deb_para[1] = Po.x;	//
	deb_para[2] = Po.y;	//
	deb_para[3] = 1;		// plane
	callback(deb_para);		// debug

#endif


	return Po;

}

// Angle consistency check
//角度整合性チェック
unsigned short Edg_Deg_def_chk(ST_EdgEDetecT* ED )
{
	// the .1 is so that rounding doesn't show a valid angle that is beyond the threshold
	// i.e. 24.04 would fail, but it would round to 24.0 and look valid
	// .1は、丸めが閾値を超えた有効な角度を示さないようにする
	//すなわち24.04は失敗しますが、24.0に丸めて有効です

	const float Deg_def_Lim = ONE_SIDE_SAFE_DEGREE_VAL; // Bound angle differential value (in degrees)// ４辺の角度差　限界値　（度）

	if (( fabsf( ED->Top_Edge_Deg    -  ED->Bottom_Edge_Deg) > Deg_def_Lim ) ||
		( fabsf( ED->Top_Edge_Deg    - -ED->Left_Edge_Deg  ) > Deg_def_Lim ) ||
		( fabsf( ED->Top_Edge_Deg    - -ED->Right_Edge_Deg ) > Deg_def_Lim ) ||
		( fabsf( ED->Bottom_Edge_Deg - -ED->Left_Edge_Deg  ) > Deg_def_Lim ) ||
		( fabsf( ED->Bottom_Edge_Deg - -ED->Right_Edge_Deg ) > Deg_def_Lim ) ||
		( fabsf( ED->Left_Edge_Deg   -  ED->Right_Edge_Deg ) > Deg_def_Lim ) )
	{
		
		return ERR_SKEW_TOO_BIG;
	}
	return 0;
}
// 幅(ドット）	長さ(ドット）  角度（度） 中心座標(ドット) の計算
//	o---------X
//	| A-----C
//	| |     |
//	| B-----D
//  Y
void calcu_size( float A_x, float A_y, float B_x, float B_y, float C_x, float C_y, float D_x, float D_y ,u8 le_or_se  ,ST_EdgEDetecT* ED)
{
	float AB_x, AB_y, CD_x, CD_y, AC_x, AC_y, BD_x, BD_y;

	//dpiの違いは定数によって吸収する　200dpiに合わせる
	AB_x = ( A_x + B_x ) / 2.0F;
	AB_y = ( A_y + B_y ) / 2.0F;// * ED->dpi_Correction;

	CD_x = ( C_x + D_x ) / 2.0F;
	CD_y = ( C_y + D_y ) / 2.0F;//* ED->dpi_Correction;

	AC_x = ( A_x + C_x ) / 2.0F;
	AC_y = ( A_y + C_y ) / 2.0F;//* ED->dpi_Correction;

	BD_x = ( B_x + D_x ) / 2.0F;
	BD_y = ( B_y + D_y ) / 2.0F;//* ED->dpi_Correction;


	// 幅
	ED->mBillSEPixelLength = (float)sqrtf( ( AC_x - BD_x ) * ( AC_x - BD_x ) + ( AC_y - BD_y ) * ( AC_y - BD_y ) ); // a^2 + b^2 = c^2

	// 長さ
	ED->mBillLEPixelLength = (float)sqrtf( ( AB_x - CD_x ) * ( AB_x - CD_x ) + ( AB_y - CD_y ) * ( AB_y - CD_y ) );

	//
	if(le_or_se == LE)
	{
		if ( CD_x == AB_x )	//　DEV 0 エラー回避
			ED->_tan_t = ( CD_y - AB_y ) / 0.00000001f;
		else
			ED->_tan_t = (( CD_y - AB_y ) / ( CD_x - AB_x ));	// AB中点とCD中点を結ぶ直線の傾き
	}

	else	//SE
	{
		if ( AC_x == BD_x )	//　DEV 0 エラー回避
			ED->_tan_t = -( BD_x - AC_x ) / 0.00000001f;
		else
			ED->_tan_t = -(( BD_x - AC_x ) / ( BD_y - AC_y ) );	// AB中点とCD中点を結ぶ直線の傾き
	}

	// 角度（度）
	ED->_deg   = (float)(PIRAD * atan( ED->_tan_t )) ;
	ED->_sin_t = (float)(ED->_tan_t / sqrt( 1.0f + ED->_tan_t * ED->_tan_t ));
	ED->_cos_t = (float)sqrt( 1.0f - ED->_sin_t * ED->_sin_t );

	// 中心座標
	ED->_o.x = (int)(( A_x + B_x + C_x + D_x ) / 4.0f + 0.5f);
	ED->_o.y = (int)(( A_y + B_y + C_y + D_y ) / 4.0f + 0.5f);


}

// Determine the Outliers and remove them.
// 外れ値を決定してそれらを削除します。
void SortAndRemoveOutliers( float *ListToAnalyze, float *SecondaryList, int *NumPoints)
{

	
	//閾値設定　差が小さすぎたら定数にしますが、小さすぎるとマージンがなくなり判定基準が厳しすぎることになります
	//			逆に大きすぎると削除される点が少なくなります。
#define DELTA_PIXEL (2.0f)	//dot

	
	float Delta[MAX_POINTS];
	int	i;
	float MedianDelta;
	float DiffPoints;
	float MinPass, MaxPass;
	u8 ListToRemove[MAX_POINTS];
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
	QuickSort_float( Delta, 0, *NumPoints - 2 );

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
			RemoveOutlier(						// 券端位置リストから外れ値を削除し残りのデータを詰める
				ListToAnalyze,		// リスト1
				SecondaryList,		// リスト2
				i - NumRemoved,		// 削除データの位置
				NumPoints );		// リストの要素数
			NumRemoved++;
		}
	}
}

// Remove the coordinates that are in the Min Max List
// Min Max Listにある座標を削除する
void RemoveOutlier( float *xPoints, float *yPoints, int Index, int *NumPoints )
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

void Swap( float *ListToSort1, float *ListToSort2 )
{
	float tempFloat;
	tempFloat = *ListToSort1;
	*ListToSort1 = *ListToSort2;
	*ListToSort2 = tempFloat;
}

void QuickSort_float( float ListToSort[MAX_POINTS], int left, int right )
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
			Swap( &ListToSort[i], &ListToSort[j] );
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
int rndup( float d )
{
	if ( 0 < d )
	{
		return( int )( d + 0.5 );
	}
	return( int )( d - 0.5 );
}

u16 deg_chk(ST_EdgEDetecT* ED)
{
	int deg = 0;

	deg = (	int)(abs((int)ED->_deg));

	//設定した規定角度以上ならばエラー
	if(ED->max_skew_thr < deg)
		return ERR_SKEW_TOO_BIG;
	
	else
		return 0;
}


u32 Eg_status_change(ST_BS *pbs ,ST_EdgEDetecT* ED)
{
	s16 subdpi;

	//主走査(実装予定)

	//副走査
	//dpi補正用乗数の決定
	//12.5dpi以下を考慮して{×2}
	subdpi = pbs->Subblock_dpi / pbs->PlaneInfo[ED->Plane].sub_sampling_pitch * 2;
	switch (subdpi)
	{
	case 400://200
		ED->dpi_Correction = 1;
		break;

	case 200://100
		ED->dpi_Correction = 2;
		break;

	case 100://50
		ED->dpi_Correction = 4;
		break;

	case 50://25
		ED->dpi_Correction = 8;
		break;

	case 25://12.5
		ED->dpi_Correction = 16;
		break;
	}

	/*透過か反射で閾値の場合分け*/
	if(pbs->PlaneInfo[ED->Plane].Ref_or_Trans == TRANSMISSION)
	{
		return 60;
	}

	else //Reflection
	{
		return 80;
	}

}

//外形検知専用getdt
u16 ED_Pget(ST_SPOINT spt, ST_BS* pbs)
{
	s16 dt_u = 0;
	if (pbs->PlaneInfo[spt.p_plane_tbl].Ref_or_Trans == TRANSMISSION)//透過
	{

#ifdef VS_DEBUG_EDGE
		deb_para[0] = 1;		// function code
		deb_para[1] = spt.x;	// 
		deb_para[2] = spt.y;	//
		deb_para[3] = 1;		// plane
		callback(deb_para);		// debug

#endif
#ifdef VS_DEBUG_ITOOL	//UI表示用　実機の時は消す
		spt.l_plane_tbl = spt.p_plane_tbl;
		spt.trace_flg = 1;
		new_P2L_Coordinate(&spt, pbs);
#endif

		dt_u = pbs->sens_dt[pbs->PlaneInfo[spt.p_plane_tbl].Address_Period * spt.y + spt.x + pbs->PlaneInfo[spt.p_plane_tbl].Address_Offset];
		return 255 - dt_u;	//値反転
	}
	//return 0;

	//反射	今は不要(2/23)
	// #if (0)
#if LE17_Manager	// LE17 Manager
	else
	{
		//CISA側のデータ採取
		//オフセット補正
		s32 x = main_offset + spt.x;
		s32 y = pbs->PlaneInfo[spt.p_plane_tbl].sub_offset + spt.y;

		//オフセット補正によって有効エリア外となる場合を防ぐ
		if (x < 0)
		{
			x = 1;
		}
		if (y < 0)
		{
			y = 1;
		}

		if (x > pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_max)
		{
			x = pbs->PlaneInfo[spt.p_plane_tbl].main_effective_range_max - 1;
		}

		if (y > (pbs->Blocksize * pbs->block_count - 1))
		{
			y = (pbs->Blocksize * pbs->block_count) - 1;
		}

		//画素採取
		dt_u = pbs->sens_dt[pbs->PlaneInfo[spt.p_plane_tbl].Address_Period * y + x + pbs->PlaneInfo[spt.p_plane_tbl].Address_Offset];
		return dt_u;
	}
#endif
	return 0;
}
//指定された1ラインをスキャンする。
//スキャンした中で最も暗かった画素値を出力する。
//
//in
//　ライン番号
//
//out
//　走査した中で最も明るい画素(255で反転するので)
u16	line_chk_pix_val(s32 line_num ,ST_BS *pbs  ,ST_EdgEDetecT* ED)
{
	ST_SPOINT spt;
	s32 x;
	u8 pix;
	u8 max_pix = 0;

	spt.p_plane_tbl = (u8)ED->Plane;
	spt.way = 0;

	for(x = ED->Main_scan_min_val; x < ED->Main_scan_max_val - 1; ++x)
	{
		spt.x = x;
		spt.y = (u32)line_num;
		pix = (u8)ED_Pget(spt ,pbs);

		if(max_pix < pix)
		{
			max_pix = pix;
		}
	}

	return max_pix + 10;



}

//指定された1ラインをスキャンする。
//そのラインに紙幣が存在した場合、開始と終了座標を返す
//
//in
//　ライン番号
//
//out
//　flg 0:１度も上回らなかった　１；上回った。
u8	line_chk_note_presence(s32 line_num ,ST_BS *pbs  ,ST_EdgEDetecT* ED, s32* start_x, s32* end_x ,u8 thr)
{
	ST_SPOINT spt;
	s32 x;
	u8 pix;

	u8 flg = 0;
	u8 count = 0;

	spt.p_plane_tbl = (u8)ED->Plane;
	spt.way = 0;


	for(x = ED->Main_scan_min_val; x < ED->Main_scan_max_val - 1; ++x)
	{
		spt.x = x;
		spt.y = (u32)line_num;
		pix = (u8)ED_Pget(spt ,pbs);

		if(thr < pix && flg == 0)
		{
			*start_x = x;
			flg = 1;
		}
		else if(thr < pix && flg == 1)
		{
			count = count + 1;

			if(count > NUM_ADDITIONAL_PIXEL_TEST)
			{
				flg = 2;
			}
		}
		else if(pix <= thr && flg == 1)
		{
			flg = 0;
			count = 0;
			*start_x = 0;
		}

		else if(pix <= thr && flg == 2)
		{
			*end_x = x;
			flg = 3; 
		}

	}

	return flg;



}

//最終ラインをスキャンして、紙幣が連なっていないかを確認する
//BAU-LE17のみで実行される
u16 chk_cont_trans(ST_BS *pbs ,ST_EdgEDetecT* ED ,u8 thr)
{
	u8 pix = 0;
	s32 start_x = 0;
	s32 end_x = 0;
	s32 x,y;
	ST_SPOINT spt;
	u8 flg = 0;
	u8 count = 0;
	s8 ret_contor = 0;
	s32 temp_x = 0;
	u16 y_half = (u16)((ED->Sub_scan_max_val - 1) * 0.5f);
	u16 x_half = (u16)(((ED->Main_scan_max_val - ED->Main_scan_min_val) * 0.5f) + ED->Main_scan_min_val);


	spt.p_plane_tbl = (u8)ED->Plane;
	spt.way = 0;


	pix = (u16)(line_chk_note_presence(ED->Sub_scan_max_val - 1 ,pbs ,ED ,&start_x ,&end_x, thr));

	if(pix == 0 || pix == 1)
	{
		return 0;	//正常
	}
	else
	{
		//下部からx方向に１ラインずつスキャンする
		//for(y = ED->Sub_scan_max_val - 2; y > y_half; y = y-4)
		//{
		count = 0;
		for(x = ED->Main_scan_min_val; x < (u16)(ED->Main_scan_max_val - 1); x = x + 1)
		{
			spt.x = x;
			spt.y = ED->Sub_scan_max_val - 2;
			y = ED->Sub_scan_max_val - 2;
			pix = (u8)ED_Pget(spt ,pbs);

			if(pix > thr && flg == 0)		//背景から紙幣
			{
				temp_x = x;
				flg = 1;
			}
			else if(pix > thr && flg == 1)	//紙幣　ゴミかもしれないので様子見
			{
				count = count + 1;

				if(count > NUM_ADDITIONAL_PIXEL_TEST)					//紙幣状態がN回続く
				{														//媒体と判定した時
					ret_contor = contor_scan(pbs, ED, spt, &temp_x ,&y);	//輪郭スキャンに移行する。

					break;
				}
			}
			else if(pix <= thr && flg == 1)	//紙幣から背景へ　カウントリセット
			{
				count = 0;
				flg = 0;
			}


		}

		if(ret_contor == 0)	//多重搬送の可能性あり
		{
			//上半分の真ん中スキャンをして紙幣の有無を確認する
			for(y = 0; y < y_half; y = y + 4)
			{
				spt.x = x_half;
				spt.y = y;
				pix = (u8)ED_Pget(spt ,pbs);

				if(pix > thr)		//背景から紙幣
				{
					return ERR_CONTINUOUS_TRANSPORTATION;	//下部の紙幣との間に空間があるので多重搬送エラー
				}
			}
		}

		return ERR_TO_THE_LOWER;	//連結搬送とみなす　通常の外形検知エラー
		}
}

//媒体と衝突した位置から輪郭スキャンを行う。
//スキャン中にy方向にてイメージの半分まで突立つした場合、処理は打ち切る。
//多重搬送ならばスキャン結果右側に到達するが、
//下側に寄っているだけならば、右側に到達しないはずである。
//戻り値 0:スタート位置の高さに戻ってきたら終了 多重搬送判定
//		 1:イメージの半分の高さまでスキャンした場合は終了　下に寄りすぎ
s8	contor_scan(ST_BS *pbs ,ST_EdgEDetecT* ED ,ST_SPOINT spt, s32* x , s32* y)
{
	u32 point_count = 0;
	u8 scan_res = 0;
	u8 scan_dir[8] = {0};			//スキャンの実行or実行しないを決定する。
	s32 start_x = *x - 1;
	s32 start_y = *y;
	u16 y_half = (u16)((ED->Sub_scan_max_val - 1) * 0.5f);


		//スキャン方向リスト
	u8 scan_dir_list[8][8] = {	
		{5,6,7,0,1,2,3,4},	//0	
		{6,7,0,1,2,3,4,5},	//1
		{7,0,1,2,3,4,5,6},	//2
		{0,1,2,3,4,5,6,7},	//3
		{1,2,3,4,5,6,7,0},	//4
		{2,3,4,5,6,7,0,1},	//5
		{3,4,5,6,7,0,1,2},	//6
		{4,5,6,7,0,1,2,3}};	//7

	memcpy(&scan_dir ,scan_dir_list[7] ,sizeof(scan_dir));

	while(point_count < 10000)
	{
		point_count++;	//無限ループ回避用

		//輪郭スキャンを行って進む方向を決定する。
		scan_res = edge_detect_pix_scan_module(ED, spt ,scan_dir ,pbs ,x ,y);

		//スキャンパターンをスキャンした方向に応じて更新
		memcpy(&scan_dir ,scan_dir_list[scan_res] ,sizeof(scan_dir));

		if(*x == start_x && *y == start_y)
		{
			return 1;
		}

		if(*y == start_y) //スタート位置の高さに戻ってきたら終了
		{
			return 0;
		}

		if(*y == y_half)	//イメージの半分の高さまでスキャンした場合は終了
		{
			return 1;
		}

	}

	return 1;

}

//輪郭スキャン用
s8 edge_detect_pix_scan_module(ST_EdgEDetecT* ED, ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs , s32 *x , s32*y)
{
	u8 dir_idx = 0;
	u8 pix = 0;

	for(dir_idx = 0; dir_idx < 8 ; ++dir_idx)
	{

		//左下
		if(dir[dir_idx] == 2)
		{
			spt.x = *x - 1;
			spt.y = *y + 1;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;

				return 2;
			}
		}

		//下
		else if(dir[dir_idx] == 1)
		{
			spt.x = (s32)*x;
			spt.y = (s32)*y + 1;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;

				return 1;
			}
		}

		//右下
		else if(dir[dir_idx] == 0)
		{
			spt.x = (s32)*x + 1;
			spt.y = (s32)*y + 1;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;

				return 0;
			}
		}

		//左
		else if(dir[dir_idx] == 3)
		{
			spt.x = (s32)*x - 1;
			spt.y = (s32)*y ;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;

				return 3;
			}
		}

		//右
		else if(dir[dir_idx] == 7)
		{
			spt.x = (s32)*x + 1;
			spt.y = (s32)*y;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;

				return 7;
			}
		}


		//左上
		else if(dir[dir_idx] == 4)
		{
			spt.x = (s32)*x - 1;
			spt.y = (s32)*y - 1;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;

				return 4;
			}
		}

		//上
		else if(dir[dir_idx] == 5)
		{
			spt.x = (s32)*x;
			spt.y = (s32)*y - 1;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;

				return 5;
			}
		}

		//右上
		else if(dir[dir_idx] == 6)
		{
			spt.x = (s32)*x + 1;
			spt.y = (s32)*y - 1;
			pix = (u8)ED_Pget(spt ,pbs);	//画素採取
			if(ED->threshold < pix)
			{
				*x = spt.x;
				*y = spt.y;
				return 6;
			}
		}

	}

	return 10;
}

//End of file
