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
* @file EdgeDetectDC.h
* @brief 外形検ル―チンのヘッダ
* @date 2018/03/14 Created.
*/
/****************************************************************************/

#ifndef EdgeDetectDCH
#define EdgeDetectDCH

#define SIDE_OUT_AREA_RANGE	1				//側面からこの値内にエッジがあった場合右か左に寄りすぎとしてエラーとする。

enum BillEdgeDC
{
	TopEdgeDC    = 0,
	BottomEdgeDC = 1,
	LeftEdgeDC   = 2,
	RightEdgeDC  = 3
};

enum
{
	CategoryCheckOutlineDC = 0,
	CategoryCheckIdentifyDC,
	CategoryCheckEnabledDCDC,
	CategoryCheckSizeDC,
	CategoryCheckJukenDC,
	CategoryCheckHard_Mag_LowDC,
	CategoryCheckHard_Mag_HighDC,
	CategoryCheckSoft_Mag_LowDC,
	CategoryCheckSoft_Mag_HighDC,

	CategoryCheckUVTopAreaDC,
	CategoryCheckUVBottomAreaDC,
	CategoryCheckUVTopAverageDC,
	CategoryCheckUVBottomAverageDC,

	CategoryCheckDoubleAreaLowDC,
	CategoryCheckDoubleAreaHighDC,
	CategoryCheckDoubleAverageLowDC,
	CategoryCheckDoubleAverageHighDC,

	CategoryCheckCIS_IR_Point_OmoteDC,
	CategoryCheckCIS_IR_Point_UraDC,
	CategoryCheckCIS_wave_rateDC,
	CategoryCheckTPHDC,
	CategoryCheckDyeNoteDC,
	CategoryCheckCIS_NNDC,
	CategoryCheckCIS_MCIRDC,
	CategoryCheckCIS_MCIR_CFDC,
	CategoryCheckFitnessSoilingDC,
	CategoryCheckFitnessLimpnessDC,
	CategoryCheckFitnessDogEarsDC,
	CategoryCheckFitnessTearsDC,
	CategoryCheckFitnessHolesDC,
	CategoryCheckFitnessMutilationsDC,
	CategoryCheckFitnessRepairsDC,
	CategoryCheckFitnessComposed_noteDC,
	CategoryCheckFitnessDyeStainDC,
	CategoryCheckFitnessGraffitiDC,
	CategoryCheckFitnessCrumplesDC,
	CategoryCheckFitnessDeInkedv,
	CategoryCheckFitnessFoldsDC,
	CategoryBlacklistDC,
	CategoryCheckFeedJamDC,
	CategoryCheckCountDC
};
enum
{
	ED_Category1DC = 0xf0,
	ED_Category2DC,
	ED_Category3DC,
	ED_Category4aDC,
	ED_Category4bDC
};

//#define NOT_VIEW_MODE										//コメントアウトすることで頂点が搬送路外かどうかのエラー判定を省略する
//#define NOT_ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA		//コメントアウトすることでスキャン開始位置に券端があり続けたときにエラーとする判定を省略する。


#define PIRAD_DC   (180/3.1415926536)

#define ERR_TOO_MANY_EDGE_POINTS_FOUND_DC		        0x0001	//検出された端点が多すぎる
#define ERR_TOO_FEW_EDGE_POINTS_FOUND_DC               0x0002	//検出された端点が少なすぎる
#define ERR_SKEW_TOO_BIG_DC                            0x0003	//検出された傾き角が大きすぎる
#define ERR_BILL_EDGE_FOUND_ON_EDGE_OF_TEST_AREA_DC    0x0004	//端点の位置が異常
#define ERR_TO_THE_RIGHT_DC							0x0005	//紙幣が右に寄りすぎ
#define ERR_TO_THE_LEFT_DC								0x0006	//紙幣が左に寄りすぎ
#define ERR_TO_THE_UPPER_DC							0x0007	//紙幣が上に寄りすぎ
#define ERR_TO_THE_LOWER_DC							0x0008	//紙幣が下に寄りすぎ
#define ERR_CONTINUOUS_TRANSPORTATION_DC	            0x0009	//連続搬送された　LE17のみのエラー

//#define ERR_VIR_AREA		                        0x0105	
//#define ERR_SIZE_MISMATCH	                        0x0106	//検出された媒体のサイズが異常													   
#define WARN_EDGE_NOT_FOUND_DC						    0x01ff	//端点が検出されませんでした
#define WARN_EDGE_FOUND_TOO_SOON_DC	                0x01fe	//上端か下端の端点検出が速い

#define	EDGE_DIFF_LIMIT_DC (0.05f)	// ４辺の角度差限度値(tan_t) // 0.02 = 1.145°, 0.01 = 0.57°（オリジナルでは3°としていた） 



#define	MAX_POINTS_DC	100
#define E_Edge_Too_Few_Points_DC    0x11  //Outline Detection Error: Bill Edge Detection occurs less than assumption
#define E_Edge_Too_Many_Points_DC   0x10  //Outline Detection Error: Bill Edge Detection occurs more than assumption
#define E_Edge_On_Test_Edge_DC      0x13  //Outline Detection Error: Can not detect because of over Bill Edge
#define E_Edge_Skew_Too_Big_DC      0x12  //Outline Detection Error: Variation of 4 Bill Edge Angles is bigger than assumption
#define E_Edge_BillSize_DC          0x14  //Outline Detection Error: Bill Size either too big or too small

#define E_Success_DC                0x00  //No Error Occurred
#define E_Failure_DC                0x01  //General Failure



typedef struct
{
	int  x;
	int  y;
} T_Point_DC;

typedef struct
{
	float  x;
	float  y;
} T_Point_float_DC;

/*-------------------------------------------------------------------------*/

typedef struct
{
		enum P_Plane_tbl Plane;	//用いる色情報

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

	short SKEW;


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
	int Sub_scan_max_val;
	int Sub_scan_min_val;

	float MinSE;
	float MaxSE;
	float MinLE;
	float MaxLE;

	//透過反射区別用につかいした


	short BottemEnd;	//下端検知に用いるパラメ
	short Step_Movement ;	//探索のステップ幅(mm) ※暫定値
	short Small_Backtrack ;	//次の探索開始位置への戻り幅(dot) ※暫定値
	short edge_err_code;

	char th_dynamic_decision_flg;	//閾値を255で固定するか　背景から動的に決定するか	反射だと背景の値にばらつきがあったが透過だとほぼ255固定なので動的決定をスキップする
	char dpi_Correction; //dpi補正用
	short dummy2;

	float	Edge_xpt[4][MAX_POINTS_DC];
	float	Edge_ypt[4][MAX_POINTS_DC];
	int		Edge_Num_Points[4];

	float	ScanEdgeXPoints[4][MAX_POINTS_DC];
	float	ScanEdgeYPoints[4][MAX_POINTS_DC];
	int		ScanEdgeNumPoints[4];
	int  mMainScanBytes[4];     // The Number of Scan Lines for the Long Edge of the Bill
	T_Point_DC _o;

	T_Point_DC AlignedBillCenterPoint;

	T_Point_float_DC AlignedUpperLeft;
	T_Point_float_DC AlignedLowerLeft;
	T_Point_float_DC AlignedUpperRight;
	T_Point_float_DC AlignedLowerRight;
	unsigned short mCategory_Level[MAX_POINTS_DC];
	unsigned short mCategory_Error[MAX_POINTS_DC];

} ST_EdgEDetecT_DC;




#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	OPdet_Start_DC(u8 buf_num);
	void Eg_det_ClearData_DC(void);
	u16	getEdgeDetectionThreshold_DC( s16 MaxValue, ST_BS *pbs);
	u16	DetectTopEdge_DC( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold ,ST_BS *pbs);
    u16	DetectTopEdgeYLoop_DC( int x, int *yStart, int yEnd, int *EdgePointFoundCount, int FindThreshold, float *XPoint, float *YPoint, int *ErrorCount, int ErrorCountMax ,ST_BS *pbs);
	u16	DetectBottomEdge_DC( int xStart, int yStart, int xHalfLength, int yEnd, int xStep, int FindThreshold   ,ST_BS *pbs );
	u16	DetectBottomEdgeYLoop_DC(	int x, // The x offset for this line
											int *yStart, // The starting point for scanning. This is passed back since the next scan line is based on the detection of this scan line
										    int yEnd, // The last point to look for an edge
										    int *EdgePointFoundCount, // Running total of the number of edges found
										    int FindThreshold, // Pixel value threshold for identifying the edge
											float *XPoint, // X from List of (X,Y) points where the edge was detected
										    float *YPoint, // Y from List of (X,Y) points where the edge was detected
										    int *ErrorCount, // Running total of edge starting too soon errors
										    int ErrorCountMax 
											,ST_BS *pbs);// Maximum number of ErrorCount before an actual error is thrown
	u8	GetAveTopBottomPixel_DC(int GetDataFilter, const T_Point_DC AlignedPoint);
    void SortAndRemoveOutliers_DC( float *ListToAnalyze, float *SecondaryList, u8 ListToAnalyseIsX, int *NumPoints, char flg);
	void SetResult_DC( unsigned int Index, unsigned int Level, int Error , int JamCode);
	void mask_DC(float* xpt, float* ypt, int num, ST_BS *pbs);
    u8	GetAveTopBottomPixel_DC(int GetDataFilter, const T_Point_DC pt);
    void FixEdge_DC( float *Bad_Edge_tan_t,
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
                  const float Good_Edge_offs,
                  const int DiffDirection,
				  const float * const GoodEdgePointsX,
                  const float * const GoodEdgePointsY
				  );

    void	calu_kaiki_DC(float* xpt, float* ypt, int num);
    int 	mask_errdt_DC(float* xpt, float* ypt, int num, float tan_t, int offset, float deflim ,float main_dot ,float sub_dot);
    void QuickSort_float_DC(float *ListToSort, int m, int n);
    void RemoveOutlier_DC( float *List1, float *List2, int Index, u8 XYReversed, int *NumPoints );
    u16	DetectLeftEdge_DC   (int xStart, int yStart, int xEnd, int yEnd, int yStep, int edgcp ,ST_BS *pbs  );
    u16	DetectRightEdge_DC  (int xStart, int yStart, int xEnd, int yEnd, int yStep, int edgcp ,ST_BS *pbs   );
    T_Point_float_DC calcu_xoyo_DC(float tan_t1, float ofsx, float tan_t2, float ofsy ,ST_BS *pbs);
    void	calcu_size_DC(float A_x, float A_y, float B_x, float B_y, float C_x, float C_y, float D_x, float D_y ,u8 a);
	void SetSkew_DC( short Skew );
	void SetWidth_DC( unsigned short Width );
	void SetHeight_DC( unsigned short Height );
    void SetBillEdgeDetectionResults_DC( unsigned short LELengthPixel, unsigned short SELengthPixel, float BillTopLeftCornerX,
    float BillTopLeftCornerY, float TopTan, float BottomTan, float LeftTan, float RightTan );
	unsigned short Edg_Deg_def_chk_DC(void);
	void SetOffset1_DC( unsigned short Offset1 );
	void SetOffset2_DC( unsigned short Offset2 );
	float getPixelSizeBillShortEdge_DC(void);
	float getPixelSizeBillLongEdge_DC(void);
    int rndup_DC(float d);
	u32 Eg_status_change_DC(ST_BS *pbs);
	u16 ED_Pget_DC(ST_SPOINT spt ,ST_BS *pbs);
	u16 deg_chk_DC(void);

	u8	line_chk_DC(u16 line_num ,ST_BS *pbs);
	u8  chk_cont_trans_DC(ST_BS *pbs);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // EdgeDetectDCH
