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
* @file EdgeDetect.h
* @brief 外形検ル―チンのヘッダ
* @date 2018/03/14 Created.
*/
/****************************************************************************/

#ifndef EdgeDetectH
#define EdgeDetectH

enum BillEdge
{
	TopEdge    = 0,
	BottomEdge = 1,
	LeftEdge   = 2,
	RightEdge  = 3
};

enum
{
	CategoryCheckOutline = 0,
	CategoryCheckIdentify,
	CategoryCheckEnabled,
	CategoryCheckSize,
	CategoryCheckJuken,
	CategoryCheckHard_Mag_Low,
	CategoryCheckHard_Mag_High,
	CategoryCheckSoft_Mag_Low,
	CategoryCheckSoft_Mag_High,

	CategoryCheckUVTopArea,
	CategoryCheckUVBottomArea,
	CategoryCheckUVTopAverage,
	CategoryCheckUVBottomAverage,

	CategoryCheckDoubleAreaLow,
	CategoryCheckDoubleAreaHigh,
	CategoryCheckDoubleAverageLow,
	CategoryCheckDoubleAverageHigh,

	CategoryCheckCIS_IR_Point_Omote,
	CategoryCheckCIS_IR_Point_Ura,
	CategoryCheckCIS_wave_rate,
	CategoryCheckTPH,
	CategoryCheckDyeNote,
	CategoryCheckCIS_NN,
	CategoryCheckCIS_MCIR,
	CategoryCheckCIS_MCIR_CF,
	CategoryCheckFitnessSoiling,
	CategoryCheckFitnessLimpness,
	CategoryCheckFitnessDogEars,
	CategoryCheckFitnessTears,
	CategoryCheckFitnessHoles,
	CategoryCheckFitnessMutilations,
	CategoryCheckFitnessRepairs,
	CategoryCheckFitnessComposed_note,
	CategoryCheckFitnessDyeStain,
	CategoryCheckFitnessGraffiti,
	CategoryCheckFitnessCrumples,
	CategoryCheckFitnessDeInked,
	CategoryCheckFitnessFolds,
	CategoryBlacklist,
	CategoryCheckFeedJam,
	CategoryCheckCount
};
//enum
//{
//	ED_Category1 = 0xf0,
//	ED_Category2,
//	ED_Category3,
//	ED_Category4a,
//	ED_Category4b
//};

enum
{
	ED_not_run = 0x00,
	ED_run
};

#define PIRAD   57.29577951289617f //(180.0f/3.1415926536f)

#define ERR_TOO_MANY_EDGE_POINTS_FOUND		        0x0401	//検出された端点が多すぎる
#define ERR_TOO_FEW_EDGE_POINTS_FOUND               0x0402	//検出された端点が少なすぎる
#define ERR_SKEW_TOO_BIG                            0x0403	//検出された傾き角が大きすぎる
#define ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA    0x0404	//端点の位置が異常
#define ERR_TO_THE_RIGHT							0x0405	//紙幣が右に寄りすぎ
#define ERR_TO_THE_LEFT								0x0406	//紙幣が左に寄りすぎ
#define ERR_TO_THE_UPPER							0x0407	//紙幣が上に寄りすぎ
#define ERR_TO_THE_LOWER							0x0408	//紙幣が下に寄りすぎ
#define ERR_CONTINUOUS_TRANSPORTATION	            0x0409	//連続搬送された　LE17のみのエラー

//#define ERR_VIR_AREA		                        0x0105	
//#define ERR_SIZE_MISMATCH	                        0x0106	//検出された媒体のサイズが異常													   
#define WARN_EDGE_NOT_FOUND						    0x01ff	//端点が検出されませんでした
#define WARN_EDGE_FOUND_TOO_SOON	                0x01fe	//上端か下端の端点検出が速い




#define	MAX_POINTS	100
#define E_Edge_Too_Few_Points    0x11  //Outline Detection Error: Bill Edge Detection occurs less than assumption
#define E_Edge_Too_Many_Points   0x10  //Outline Detection Error: Bill Edge Detection occurs more than assumption
#define E_Edge_On_Test_Edge      0x13  //Outline Detection Error: Can not detect because of over Bill Edge
#define E_Edge_Skew_Too_Big      0x12  //Outline Detection Error: Variation of 4 Bill Edge Angles is bigger than assumption
#define E_Edge_BillSize          0x14  //Outline Detection Error: Bill Size either too big or too small

#define E_Success                0x00  //No Error Occurred
#define E_Failure                0x01  //General Failure



typedef struct
{
	int  x;
	int  y;
} T_Point;

typedef struct
{
	float  x;
	float  y;
} T_Point_float;

/*-------------------------------------------------------------------------*/

typedef struct
{
	

	float Top_Edge_Deg;
	float Top_Edge_tan_t;
	float Top_Edge_sin_t;
	float Top_Edge_cos_t;
	float Top_Edge_offs;

	float Bottom_Edge_Deg;
	float Bottom_Edge_tan_t;
	float Bottom_Edge_sin_t;
	float Bottom_Edge_cos_t;
	float Bottom_Edge_offs;

	float Left_Edge_Deg;
	float Left_Edge_sin_t;
	float Left_Edge_cos_t;
	float Left_Edge_tan_t;
	float Left_Edge_offs;

	float Right_Edge_Deg;
	float Right_Edge_sin_t;
	float Right_Edge_cos_t;
	float Right_Edge_tan_t;
	float Right_Edge_offs;

	int length_dot;
	int width_dot;


	float tan_t;
	float sin_t;
	float cos_t;
	float offs;

	float skew;   // Skew of Bill based on Pixels
	float tan_th;
	float sin_th;
	float cos_th;
	float adj_pitch;
	float length_mm;
	float width_mm;
	float adj_skew; // Real Life Skew of Bill
	float ofs_dbld_L1;	// vartual alimennt ?
	float ofs_dbld_L2;



	short	mResults_skew_deg;
	char	mResults_width;
	char	mResults_height;
	unsigned short	mResults_offset1;
	unsigned short	mResults_offset2;

	int	mMinSubScanLines;              // The Minimum of mSubScanLines[OMOTE] and mSubScanLines[URA]
	float mBillSEPixelLength, mBillLEPixelLength, _deg, _tan_t, _sin_t, _cos_t;

	float TopLeftCornerX;
	float TopLeftCornerY;
	float BottomLeftCornerX;
	float BottomLeftCornerY;
	float TopRightCornerX;
	float TopRightCornerY;
	float BottomRightCornerX;
	float BottomRightCornerY;

	int mJamCode;

	int Main_scan_max_val;
	int Main_scan_min_val;				// イメージデータの主走査方向ドット数（200dpi)
	int Main_scan_max_val_ori;
	int Main_scan_min_val_ori;			//外形検知紙粉対策されるまえの値

	int Sub_scan_max_val;
	int Sub_scan_min_val;

	float MinSE;
	float MaxSE;
	float MinLE;
	float MaxLE;

	//透過反射区別用につかいした


	short BottemEnd;	//下端検知に用いるパラメ
	short edge_err_code;

	float	Edge_xpt[4][MAX_POINTS];
	float	Edge_ypt[4][MAX_POINTS];
	int		Edge_Num_Points[4];

	float	ScanEdgeXPoints[4][MAX_POINTS];
	float	ScanEdgeYPoints[4][MAX_POINTS];
	int		ScanEdgeNumPoints[4];
	int  mMainScanBytes[4];     // The Number of Scan Lines for the Long Edge of the Bill
	T_Point _o;

	T_Point AlignedBillCenterPoint;

	T_Point_float AlignedUpperLeft;
	T_Point_float AlignedLowerLeft;
	T_Point_float AlignedUpperRight;
	T_Point_float AlignedLowerRight;
	unsigned short mCategory_Level[MAX_POINTS];
	unsigned short mCategory_Error[MAX_POINTS];

	enum P_Plane_tbl Plane;	//用いる色情報
	short Step_Movement ;	//探索のステップ幅(mm) ※暫定値
	short Small_Backtrack ;	//次の探索開始位置への戻り幅(dot) ※暫定値
	short SKEW;
	short dummy2;
	char th_dynamic_decision_flg;	//閾値を255で固定するか　背景から動的に決定するか	反射だと背景の値にばらつきがあったが透過だとほぼ255固定なので動的決定をスキップする
	char dpi_Correction; //dpi補正用
	u8	max_skew_thr;						//最大スキューの値
	u8	vertex_err_determination;			//頂点が搬送路外かどうかのエラー判定を省略する　1:実行　0：実行しない
	u8	abnormal_position_of_ticket_edge;	//スキャン開始位置に券端があり続けたときにエラーとする判定を省略する。　1:実行　0：実行しない
	u8  multiple_transport;					//多重搬送検知　BAULE17用 1:実行　0：実行しない
	u8  threshold;
	u8  padding;

} ST_EdgEDetecT;

//#define NOT_VIEW_MODE										//コメントアウトすることで頂点が搬送路外かどうかのエラー判定を省略する
//#define NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA		//コメントアウトすることでスキャン開始位置に券端があり続けたときにエラーとする判定を省略する。



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	OPdet_Start(u8 buf_num ,ST_EdgEDetecT* ED);
	void Eg_det_ClearData(ST_EdgEDetecT* ED);
	u16	getEdgeDetectionThreshold( s16 MaxValue, ST_BS *pbs ,ST_EdgEDetecT* ED);
	u16	DetectTopEdge( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold ,ST_BS *pbs ,ST_EdgEDetecT* ED);
    u16	DetectTopEdgeYLoop( int x, int *yStart, int yEnd, int *EdgePointFoundCount, int FindThreshold, float *XPoint, float *YPoint, int *ErrorCount, int ErrorCountMax ,ST_BS *pbs ,ST_EdgEDetecT* ED);
	u16	DetectBottomEdge( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold   ,ST_BS *pbs  ,ST_EdgEDetecT* ED);
	u16	DetectBottomEdgeYLoop(	int x, // The x offset for this line
											int *yStart, // The starting point for scanning. This is passed back since the next scan line is based on the detection of this scan line
										    int yEnd, // The last point to look for an edge
										    int *EdgePointFoundCount, // Running total of the number of edges found
										    int FindThreshold, // Pixel value threshold for identifying the edge
											float *XPoint, // X from List of (X,Y) points where the edge was detected
										    float *YPoint, // Y from List of (X,Y) points where the edge was detected
										    int *ErrorCount, // Running total of edge starting too soon errors
										    int ErrorCountMax 
											,ST_BS *pbs ,ST_EdgEDetecT* ED);// Maximum number of ErrorCount before an actual error is thrown

	u8	GetAveTopBottomPixel(int GetDataFilter, const T_Point AlignedPoint ,ST_EdgEDetecT* ED);
    void SortAndRemoveOutliers( float *ListToAnalyze, float *SecondaryList, int *NumPoints);
	void mask(float* xpt, float* ypt, int num, ST_BS *pbs ,ST_EdgEDetecT* ED);
    u8	GetAveTopBottomPixel(int GetDataFilter, const T_Point pt ,ST_EdgEDetecT* ED);
    void FixEdge( float *Bad_Edge_tan_t,
                  float *Bad_Edge_sin_t,
                  float *Bad_Edge_cos_t,
                  float *Bad_Edge_Deg,
                  float *Bad_Edge_offs,
                  const int Bad_Edge_Num_Points,
                  const float * const BadEdgePointsX,
                  const float * const BadEdgePointsY,
                  const float Good_Edge_tan_t,
                  const float Good_Edge_sin_t,
                  const float Good_Edge_cos_t,
                  const float Good_Edge_Deg,
				  const float Good_Edge_offset,
				  u8 flg
				  );

    void	calu_kaiki(float* xpt, float* ypt, int num ,ST_EdgEDetecT* ED);
    int 	mask_errdt(float* xpt, float* ypt, int num, float tan_t, int offset, float deflim ,float main_dot ,float sub_dot);
    void QuickSort_float(float *ListToSort, int m, int n );
    void RemoveOutlier( float *List1, float *List2, int Index, int *NumPoints );
    u16	DetectLeftEdge   (int xStart, int yStart, int xEnd, int yEnd, int yStep, int edgcp ,ST_BS *pbs  ,ST_EdgEDetecT* ED );
    u16	DetectRightEdge  (int xStart, int yStart, int xEnd, int yEnd, int yStep, int edgcp ,ST_BS *pbs  ,ST_EdgEDetecT* ED  );
    T_Point_float calcu_xoyo(float tan_t1, float ofsx, float tan_t2, float ofsy ,ST_BS *pbs ,ST_EdgEDetecT* ED);
    void	calcu_size(float A_x, float A_y, float B_x, float B_y, float C_x, float C_y, float D_x, float D_y ,u8 a ,ST_EdgEDetecT* ED);
    void SetBillEdgeDetectionResults( unsigned short LELengthPixel, unsigned short SELengthPixel, float BillTopLeftCornerX,
    float BillTopLeftCornerY, float TopTan, float BottomTan, float LeftTan, float RightTan  ,ST_EdgEDetecT* ED);
	unsigned short Edg_Deg_def_chk(ST_EdgEDetecT* ED);
	float getPixelSizeBillShortEdge(ST_EdgEDetecT* ED);
	float getPixelSizeBillLongEdge(ST_EdgEDetecT* ED);
    int rndup(float d);
	u32 Eg_status_change(ST_BS *pbs ,ST_EdgEDetecT* ED);
	u16 ED_Pget(ST_SPOINT spt ,ST_BS *pbs);
	u16 deg_chk(ST_EdgEDetecT* ED);

	u16	line_chk_pix_val(s32 line_num ,ST_BS *pbs ,ST_EdgEDetecT* ED);
	u8	line_chk_note_presence(s32 line_num ,ST_BS *pbs ,ST_EdgEDetecT* ED, s32* start_x, s32* end_x, u8 thr);
	u16  chk_cont_trans(ST_BS *pbs ,ST_EdgEDetecT* ED ,u8 thr);
	s8	contor_scan(ST_BS *pbs ,ST_EdgEDetecT* ED, ST_SPOINT spt, s32 *x , s32 *y);
	s8 edge_detect_pix_scan_module(ST_EdgEDetecT* ED, ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs , s32*x , s32*y);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // EdgeDetectH
