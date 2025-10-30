// getdt.h
#ifndef	GETDT_H_INCLUDED
#define	GETDT_H_INCLUDED

#include "cis_calibration.h"

//#define _RM_	//LE17用？　シミュレータの時はコメントアウトすること
//#define BAU_LE17	//LE17用

#ifdef _RM_
//#include "jcm_typedef.h"		// 18/09/13
// ファイル情報を定義, 19/05/14
enum {
	FILE_INFO_SIZE			= 256,
	PLANE_INFO_SIZE			= 1536,
	RESULT_INFO_SIZE		= 8192,
	ALL_INFO_SIZE			= (FILE_INFO_SIZE + PLANE_INFO_SIZE + RESULT_INFO_SIZE),
	BLOCK_MAX				= 424,														// 最大サンプルブロック数,
	SUB_BLOCK_NUM			= 4,  														// 1ブロックあたりのサブブロック数

	SUB_BLOCK_BYTE_SIZE		= 8880,													// 1サブブロックのデータ数(8800バイト),
	BLOCK_BYTE_SIZE			= (SUB_BLOCK_BYTE_SIZE * SUB_BLOCK_NUM),					// 1ブロックのサイズ(35200バイト)
	SENS_SIZE				= (BLOCK_BYTE_SIZE * (BLOCK_MAX+1)),						// センササイズ(8800 * 4 * (424+1) = 14960000),
	FILE_SIZE				= (ALL_INFO_SIZE + SENS_SIZE),

	SUB_BLOCK_BYTE_SIZE_RAW	= (SUB_BLOCK_BYTE_SIZE*2),									// 1サブブロックのデータ数(17600バイト),
	BLOCK_BYTE_SIZE_RAW		= (SUB_BLOCK_BYTE_SIZE_RAW * SUB_BLOCK_NUM),				// 1ブロックのサイズ(70400バイト), 19/12/03
	SENS_SIZE_RAW			= (BLOCK_BYTE_SIZE_RAW * BLOCK_MAX),						// センササイズ(SUB_BLOCK_BYTE_SIZE * 4 * (424+1) = 29849600),
	FILE_SIZE_RAW			= (ALL_INFO_SIZE + SENS_SIZE_RAW),
};
#endif

enum Way { W0, W1, W2, W3 };
enum Sensor { R_R, R_G, R_B, R_IR1, R_IR2, R_FL, T_IR, T_G, T_R, T_B ,MG ,Res};
enum Side { OMOTE, URA, NUM_SIDES };
enum CIS_U_D { UP, DOWN };
enum MAG_L_R { LEFT, RIGHT };
enum TRANS_ORDER { O2U, U2O };
enum Trans_or_Reflection {REFLECTION , TRANSMISSION};
enum LE_or_SE {LE , SE};
enum Enable_or_Disable {PLANE_DISABLED , PLANE_ENABLED};
enum note_scan_dir_Right_or_Left {L2R , R2L}; //スキャン方向 L2R 左から右へ R2L 逆
enum transport_direction_flg {DEPOSIT, WITHDRAWAL};
enum Fitness_Result {ATM, FIT, UF ,REJ};	
enum DebugCoordinate {dbg_codi_Physical = -1 , dbg_codi_Logic = -2};


//マクロ
//変数入れ替え
#define SWAP(a,b) (a ^= b,b = a ^ b,a ^= b)
//絶対値
#define ABS(IN) ((IN) < 0 ? - (IN) : (IN))

				//絶対に配列の順番を変えないでください
enum P_Plane_tbl {
	UP_R_R,   UP_R_G,   UP_R_B,   UP_R_IR1,   UP_R_IR2,   UP_R_FL,     P_Reserve1,  P_Reserve2,
	DOWN_R_R, DOWN_R_G, DOWN_R_B, DOWN_R_IR1, DOWN_R_IR2, DOWN_R_FL,   P_Reserve4,  P_Reserve5,
	UP_T_R,   UP_T_G,   UP_T_B,   UP_T_IR1,   UP_T_IR2,   P_Reserve7,  P_Reserve8,  
	DOWN_T_R, DOWN_T_G, DOWN_T_B, DOWN_T_IR1, DOWN_T_IR2, P_Reserve10, P_Reserve11, 
	UP_MAG,   UP_MAG2,  UP_TC1,   UP_TC2,     UP_SP_A,    UP_SP_A2,    UP_SP_B,     UP_SP_B2 , UP_CAP1 , UP_CAP2
};

				//絶対に配列の順番を変えないでください
enum L_Plane_tbl {
	OMOTE_R_R, OMOTE_R_G,  OMOTE_R_B, OMOTE_R_IR1, OMOTE_R_IR2, OMOTE_R_FL,  L_Reserve1,  L_Reserve2,
	URA_R_R,   URA_R_G,    URA_R_B,   URA_R_IR1,   URA_R_IR2,   URA_R_FL,    L_Reserve4,  L_Reserve5,
	OMOTE_T_R, OMOTE_T_G,  OMOTE_T_B, OMOTE_T_IR1, OMOTE_T_IR2, L_Reserve7,  L_Reserve8,  
	URA_T_R,   URA_T_G,    URA_T_B,   URA_T_IR1,   URA_T_IR2,   L_Reserve10, L_Reserve11, 
	OMOTE_MAG, OMOTE_MAG2, OMOTE_TC1, OMOTE_TC2,   OMOTE_SP_A,  OMOTE_SP_A2, OMOTE_SP_B,  OMOTE_SP_B2 , OMOTE_CAP1 , OMOTE_CAP2
};

//エラーコード
#define ERR_NON_COMPLIANT_PLANE		0x001		//無効なプレーン	
#define ERR_OUT_OF_PIXEL_RANGE		0x002		//物理座標が有効範囲外
#define ERR_REFERENCE_OUT_OF_AREA	0x003		//参照した場所がメモリ範囲外

//定義
#define LOGICAL_COORDINATE_DPI		200			//論理座標系での基準dpi
#define MAX_PLANE_COUNT				42			//プレーン数
#define MAX_FITNESS_COUNT			33			//フィットネス情報+ダイノート情報の数
#define MAX_VALIDATION_COUNT		64			//アルゴの処理の総数

// ichijo
#define IMUF_POINT_NUMBER 4 // パラメータの指定領域の数 Ir Mag Uv Fold

#define THICKNESS_SENSOR_MAX_NUM	16			// 厚みセンサー最大数

//上位通信用共有体のインデックス。
enum{
	DYE_NOTE_		= 0x00,				// ダイノート
	FITNESS_SOILING,					// 汚れ（全体）
	FITNESS_LIMPNESS,					// よれよれ
	FITNESS_DOGEARS,					// ドッグイヤー
	FITNESS_TEARS,						// 裂け
	FITNESS_HOLES,						// 穴
	FITNESS_MUTILATIONS,				// 欠損
	FITNESS_REPAIRS_MECHA,				// 修復テープ(メカ)
	FITNESS_REPAIRS_CAP,				// 修復テープ(静電)
	FITNESS_STAINS,						// 染み
	FITNESS_GRAFFITI,					// 落書き
	FITNESS_CRUMPLES,					// しわ、折れ
	FITNESS_DE_INKED_NOTE,				// 脱色
	FITNESS_FOLDS,						// 折れ（折りたたまれている）
	FITNESS_MISSING_MAG_THREAD,			// 磁気スレッド剥がれ
	FITNESS_MISSING_METAL_THREAD,		// 金属スレッド剥がれ
	FITNESS_MISSING_SECURITY_FEATURE,	// 特徴剥がれ
	FITNESS_WASHED,						// 洗濯券

	FITNESS_ALGO_NUM,					// 最大値
};

//アルゴリズムのインデックス
enum{

	ALGO_DOG_EARS = 0x00,	// 角折れ
	ALGO_TEAR,				// 裂け
	ALGO_DYENOTE,			// ダイノート
	ALGO_SOILING,			// 汚れ
	ALGO_DEINKD,			// 脱色
	ALGO_STAIN,				// 染み、部分汚れ
	ALGO_HOLE,				// 穴
	ALGO_CIR3,				// 3色比
	ALGO_CIR4,				// 4色比
	ALGO_MCIR,				// MCIR
	ALGO_IR_CHECK,			// IRチェック
	ALGO_DOUBLE_CHECK,		// 重券検知
	ALGO_NN1,				// 鑑別NN1色
	ALGO_NN2,				// 鑑別NN2色
	ALGO_IR2_SICPA,			// シクパトーク検知　IR2波長
	ALGO_MAG,				// 磁気
	ALGO_UV_VALIDATE,		// UV鑑別
	ALGO_FOLD,				// 折り畳み
	ALGO_MUTILATION,		// 欠損
	ALGO_BARCODE,			// バーコード
	ALGO_TAPE_MECHA,		// テープ検知（メカ式）テープ面積の閾値
	ALGO_TAPE_CAP,			// テープ検知（静電容量式）
	ALGO_NEOMAG,			// ネオマグ検知
	ALGO_MAG_THREAD,		// スレッド検知（磁気）
	ALGO_METAL_THREAD,		// スレッド検知（金属）
	ALGO_HOLOGRAM,			// ホログラム
	ALGO_SP_A,				// 特殊A
	ALGO_SP_B,				// 特殊B
	ALGO_LIMPNESS,			// よれ
	ALGO_GRAFFITI,			// 落書き
	ALGO_CRUMPLE,			// しわ
	ALGO_TAPE_MECHA_TC,		// テープ検知（メカ式）厚みの閾値
	ALGO_WASHED,			// 洗濯
	ALGO_OCR,				// Optical Character Reader
	ALGO_CF_NN_1COLOR,		// CF-NN 1Color
	ALGO_CF_NN_2COLOR,		// CF-NN 2Color
	ALGO_CUSTOM,			// カスタムチェック
	ALGO_DIFF_SERIES,		// 異番号検知
	
	ALGO_MAX,				// 使用アルゴの最大数

	ALGO_OUTLINE = -1,	// 外形検知
	ALGO_IDENT_NN = -2,	// 金種識別NN
};

//鑑別結果bitのインデックス
enum
{
	VALIDATION_RES_3CIR = 0x00,	//3色比
	VALIDATION_RES_4CIR,		//4色比
	VALIDATION_RES_MCIR,		//MCIR
	VALIDATION_RES_IR_CHECK,	//IRチェック
	VALIDATION_RES_DOUBLE_CHECK,//重券
	VALIDATION_RES_NN1,			//NN1
	VALIDATION_RES_NN2,			//NN2
	VALIDATION_RES_IR2_WAVE,	//IR2波長差　シクパトーク
	VALIDATION_RES_MAG,			//磁気検知
	VALIDATION_RES_UV,			//UV検知
	VALIDATION_RES_NEOMAG,		//ネオマグ
	VALIDATION_RES_SP_A,		//特殊A
	VALIDATION_RES_SP_B,		//特殊B
	VALIDATION_RES_CF_NN1,		//CF-NN1
	VALIDATION_RES_CF_NN2,		//CF-NN2
	VALIDATION_RES_CUSTOM,		//カスタムチェック

};

#ifdef _RM_
#define MAX_SENSDATA_SIZE			(FILE_SIZE)	//センサーデータサイズ、13844480をFILE_SIZE(13527040)に変更, 19/07/29
#else
#define MAX_SENSDATA_SIZE			13844480	//センサーデータサイズ
#endif


typedef struct	//14byte
{
	s32		x;
	s32		y;

	u8  way;				// 挿入方向 (挿入方向不定時はW0にしておく)
	u8 p_plane_tbl;	//物理情報でのプレーン情報(UP_R_Rなど)
	u8 l_plane_tbl;	//論理情報でのプレーン情報(OMOTE_R_Rなど)
	u8  trace_info;			//センサデータの詳細書き込む用

	s16  sensdt;			//センサーデータ(画素値)
	u8 padding1[2];
	
	//******高速化検討********
	u8*	p;				//そのピクセルアドレス
	u32	period;			//period
	u8 trace_flg;		//デバッグ用
	
} ST_SPOINT;


typedef struct
{
	u8		Ref_or_Trans;		//透過か反射か 0反射　1透過
	u8		note_scan_side;		//識別前の面 0:上CISで受像 1:下で受像
	u8		Enable_or_Disable;	//0有効：1無効
	u8      note_scan_dir;		//スキャン方向 0正方　1逆
	float	main_element_pitch;	//素子ピッチ
	u16		main_effective_range_min;	//主走査方向有効画素範囲開始位置
	u16		main_effective_range_max;	//主走査方向有効画素範囲終了位置
	s16		main_offset;		//搬送路センターの画素番号を設定
	u8		sub_sampling_pitch;	//副走査方向のサンプリングピッチ
	s8  	sub_offset;			//サブブロック単位のドット数
	u8		data_type;			//データ長
	u8		planeblank1;		//ブランク
	u16		main_all_pix;		//このプレーンの全素子数
	u8		planeblank2[4];		//ブランク4バイト
	u32		Address_Period;		//そのセンサーデータのPeriod
	u32		Address_Offset;		//そのセンサーデータのオフセット

} ST_PLANE_INFOMATION;

//識鑑別処理、中間情報記録用の構造体****
//外形検知
typedef struct
{
	u32	proc_time;				//処理時間
	u32 proc_time_point_extra;	//処理時間
	u32 proc_time_ocr1;			//処理時間
	u32 proc_time_ocr2;			//処理時間
	u32 proc_time_color;		//処理時間//add by furuta 20/8/21
	u32 proc_time_replay_fitness_list;
	u32 proc_time_replay_fitness;

	u8	reserve[36];


} ST_MID_RES_OUTLINE;

//識別NN
typedef struct
{
	u32	proc_time;			//処理時間
	float output_max_val;	//最大発火値
	float output_2nd_val;	//第2発火値
	float softmax_val;		//ソフトマックス値	
	float error_val;		//エラー値
	u16   max_node_num;		//最大発火のノード番号
	u16   _2nd_node_num;	//最大発火のノード番号
	u32	  result_jcm_id;	//識別結果のJCM-ID
	float th1;				//最大発火値の閾値
	float th2;				//第2発火値の閾値
	//float th3;				//ソフトマックス値の閾値	
	//float th5;				//エラー値の閾値
	u32 chk_sums[7];

} ST_MID_RES_NN_IDENT;


//角折れ
typedef struct
{
	u32	proc_time;			//処理時間
	u16	long_side_left_up;	//長辺
	u16	short_side_left_up;	//短辺
	u32	area_left_up;		//面積

	u16	long_side_left_down;//長辺
	u16	short_side_left_down;//短辺
	u32	area_left_down;		//面積

	u16	long_side_right_up;	//長辺
	u16	short_side_right_up;//短辺
	u32	area_right_up;		//面積

	u16	long_side_right_down;//長辺
	u16	short_side_right_down;//短辺
	u32	area_right_down;	//面積

	u8	judge_left_up;		//判定結果
	u8	judge_left_down;	//
	u8	judge_right_up;		//
	u8	judge_right_down;	//

	u8	reserve[24];

} ST_MID_RES_DOR_EAR;

//裂け
typedef struct
{
	u32	proc_time;			//処理時間
	float width;			//幅
	float depth;			//深さ
	float total_depth;		//深さの合計
	u8	type;				//形状
	u8	judge;				//判定結果
	u8	reserve[46];

} ST_MID_RES_TEAR;

//ダイノート
typedef struct
{
	u32	proc_time;	//処理時間
	u32	area;		//面積
	float raito;	//割合
	u8 judge;		//判定結果
	u8	reserve[51];

} ST_MID_RES_DYE_NOTE;

//汚れ
typedef struct
{
	u32	proc_time;				//処理時間
	u32 plane_distance[5];	//平面との距離
	u8 avg_val[5][3];			//RGBの平均値
	u8 result;
	float concentration;
	u8	reserve[20];

} ST_MID_RES_SOILING;

//脱色
typedef struct
{
	u32	proc_time;	//処理時間
	float plane_distance[5];	//平面との距離
	u8 result;
	u8	reserve[39];

} ST_MID_RES_DE_INKD;

//染み
typedef struct
{
	u32	proc_time;	//処理時間
	float area;		//面積
	float diameter;	//直径
	float total_area;//合計面積
	u16   err_code;	//エラーコード
	u8    judge;	//判定結果
	u8	reserve[45];

} ST_MID_RES_STAIN;

//折り畳み・欠損
typedef struct
{
	u32	proc_time;	//処理時間
	u8 up_res; //上端の結果
	u8 down_res; //下端の位置
	u8 left_res; //左端の位置
	u8 right_res; //右端の位置
	u8 level[4]; // レベル
	u16   err_code;	//エラーコード
	u8 reserve[50];
} ST_MID_RES_FOLDING;

//3色比　4色比　IRチェック　MCIR
typedef struct
{
	u32	proc_time;	//処理時間
	u16 invalid_count;	//インバリカウント
	u8	level;
	u8  thr_level;
	u8  result;
	u8	reserve[55];

} ST_MID_RES_CIR_SERIES;


//重券
typedef struct
{
	u32	proc_time;	//処理時間
	s32	invalid_count;	//インバリカウント
	u8	level;			//
	u8  result;			//
	u8  thr_level;		//閾値レベル
	u8	padding;		//パディング
	float all_prob;		//全体紙幣の重券確率（重券レベル)
	float each_prob[4];	//4部分の重券確率
	u8	reserve[32];

} ST_MID_RES_DOUBLE_CHECK;

//NN1　NN2
typedef struct
{
	u32	proc_time;	//処理時間
	float genuine_out_put_val;		//本物発火値
	float counterfeit_out_put_val;	//偽物発火値
	u8	result;
	u8	calc_res_level;
	u8  thr_level;		//閾値レベル
	u8	reserve[49];

} ST_MID_RES_VALI_NN;

// IR2波長
typedef struct
{
	u32	proc_time;	//処理時間4

	//u32 ir1_peak[IMUF_POINT_NUMBER];
	//u32 ir2_peak[IMUF_POINT_NUMBER];

	u8 feature;//5
	u8 ir1peak;//6
	u8 ir2peak;//7
	u8 err_code;//8

	u32 ir1freq;//12
	u32 ir2freq;//16
	float ir1pro;//20
	float ir2pro;//24

	u8 output[IMUF_POINT_NUMBER];//28

	u8 result;//29
	u8 thr_level;//閾値レベル30
	u8 level; // 出力レベル31
	u8 temp;//32
	u8 reserve[32];//64
}ST_MID_RES_IR2WAVE;

// MAG
typedef struct
{
	u32	proc_time;	//処理時間4
	float average;//8
	float percent;//12
	u8 ave_number;//13
	u8 per_number;//14
	u8 result;//15
	u8 level;	// 出力レベル 16

	u8 fit_thr;	// 閾値レベル（Fit 17
	u8 uf_thr;	// 閾値レベル（Uf 18
	u8 level_result; //判定結果0~2 19
	u8 padding;//20
	u8 reserve[44];//64
}ST_MID_RES_MAG;

// UV
typedef struct
{
	u32	proc_time;	//処理時間4
	float average[IMUF_POINT_NUMBER];//20
	
	u8 result;//21
	u8 level;	// 出力レベル 22

	u8 fit_thr;	// 閾値レベル（Fit 23
	u8 uf_thr;	// 閾値レベル（Uf 24
	u8 level_result; //判定結果0~2 25
	u8 padding[3];//28
	u8 reserve[36];//
}ST_MID_RES_UV_VALI;

// UV(Wash)
typedef struct
{
	u32	proc_time;	//処理時間4
	float average[IMUF_POINT_NUMBER];//16

	u8 result;//17
	u8 level;	// 出力レベル 18

	u8 fit_thr;	// 閾値レベル（Fit 19
	u8 uf_thr;	// 閾値レベル（Uf 20
	u8 level_result; //判定結果0~2 21
	u8 padding[3];//24
	u8 reserve[36];//64
}ST_MID_RES_UV_FITNESS;

//穴
typedef struct
{
	u32	proc_time;	//処理時間
	s16 each_hole_area[8];
	u16 hole_total_area;
	u16 err_code;	
	u8	result;		//検知結果
	u8	hole_num;	//穴の数
	u8	reserve[38];

} ST_MID_RES_HOLE;

//テープ検知
typedef struct
{
	u32	proc_time;		// 処理時間
	u32 err_code;		// エラーコード
	u32 result;			// 検知結果
	u8	level;			// レベル
	u8	threshold;		// 厚み閾値（?）
	u16	detect_num;		// テープ検知数
	u8  ch_tape_num[THICKNESS_SENSOR_MAX_NUM];	// 各圧検センサ毎のテープ判定箇所数
	u8	reserve[32];

} ST_MID_RES_TAPE;

//静電テープ検知
typedef struct
{
	u32	proc_time;	//処理時間
	u16 err_code;
	u8 result;
	u8	level;
	u8	reserve[56];

} ST_MID_RES_CAP_TAPE;

//バーコード
typedef struct
{
	u32	proc_time;	//処理時間
	u8 code_type;		//バーコードの種類　0：読み取りなし、1：1次元、2：2次元
	u8 code_type_rsv;	//バーコードの種類の予備
	u8 read_data_type;	//読み取りデータ種別　0：ASCII　1：BCD
	u8 read_data_len;	//読み取りデータの長さ 0:なし　1~127有効なキャラ数
	u8 read_data[64];	//読み取りデータの実体
	u32 result_code;	//読み取り結果の状態コード
	u8	max_bar;		//バーの最大幅
	u8 padding[51];		//予備

} ST_MID_RES_BAR_CODE;

//NEOMAG
typedef struct
{
	u32	proc_time;		//処理時間
	s8 result[5];		//検知結果
	u8 ir1ave[5];		//IR1平均値
	u8 ir2ave[5];		//IR2平均値
	u8 split_point[5];	//MAGなし範囲数
	u8 ir1_below[5];	//IR1シミ個数
	u8 ir2_below[5];	//IR2シミ個数
	u16 count;
	float magdev[5];	//MAG標準偏差
	s16 err_code;		//エラーコード
	u8 judge;			//判定
	u8  thr_level;		//閾値レベル
	u8 padding[4];		//予備

} ST_MID_RES_NEOMAG;

//磁気スレッド
typedef struct
{
	u32	proc_time;			//処理時間4
	float tir_z_score;		//TIR判定値8
	float tir_total_ave;	//TIR平均値12
	float tir_total_dev;	//TIR標準偏差16
	float mag_total_ave;	//MAG平均値20
	float mag_total_dev;	//MAG標準偏差24
	float res_mag_max;		//MAG28
	u16 tir_count;			//TIR判定個数30
	u16 mag_max_count;		//MAG判定個数32
	s16 err_code;			//エラーコード234
	s16 thraed_center;		//欠損位置　論理x座標	2 36
	u8 remain_precent;		//スレッド長さパーセント	1 37
	u8 thread_num;			//スレッド個数138
	s8 judge;				//検知結果139
	u8 padding[25];			//予備


} ST_MID_RES_MAGTHREAD;

//金属スレッド
typedef struct
{
	u32	proc_time;			//処理時間4
	float tir_z_score;		//TIR判定値
	float tir_total_ave;	//TIR平均値
	float tir_total_dev;	//TIR標準偏差
	u16 tir_count;			//TIR判定個数
	s16 err_code;			//エラーコード2
	s16 thraed_center;		//欠損位置　論理x座標	2
	u8 remain_precent;		//スレッド長さパーセント
	u8 thread_num;			//スレッド個数1
	s8 judge;				//検知結果1
	u8 padding[39];			//予備

} ST_MID_RES_METALTHREAD;

//ホログラム
typedef struct
{
	u32	proc_time;	//処理時間
	float tir_ave[5];	//TIR平均値
	u16 area[5];
	s16 judge;
	u16 wid[5];
	s16 err_code;
	u16 heid[5];
	u8 result[5];
	u8 padding;

} ST_MID_RES_HOLOGRAM;

// 重券検知（メカ厚）
typedef struct
{
	u32	proc_time;										// 処理時間
	u8  result;											// 判定結果
	u8  double_check_ratio;								// 重券面積比率
	u16 bill_thickness_average;							// 紙幣判定箇所の厚み平均値
	u8  double_check_point[THICKNESS_SENSOR_MAX_NUM];	// 重券判定箇所数　（各圧検センサ毎　最大16センサ分）
	u8  bill_check_point[THICKNESS_SENSOR_MAX_NUM];		// 紙幣判定箇所数　（各圧検センサ毎　最大16センサ分）
	u8  bill_top_point[THICKNESS_SENSOR_MAX_NUM];		// 紙幣先端判定箇所　（各圧検センサ毎　最大16センサ分）
	u16 double_check_threshold;							// 重券検出厚み閾値
	u8  double_area_ratio;								// 面積比率閾値(%)
	u8  bill_check_threshold;							// 紙幣検出厚み閾値
	u8  exclude_point;									// 先頭判定除外範囲(ポイント)
	u8  sensor_num;										// 厚検センサCH数
	u8  check_point_top;								// 検知対象範囲先端
	u8  check_point_end;								// 検知対象範囲後端

} ST_MID_RES_DBL_CHK_MECHA;

//特殊A
typedef struct
{
	u32	proc_time;	//処理時間	4
	u16 sum[5];		//10
	u16 total_sum;	//2
	u8 count[5];	//5
	s8 judge;		//1
	u8  thr_level;	//閾値レベル
	u8  reserve[41];		// 予備
} ST_MID_RES_SPECIAL_A;

//特殊B
typedef struct
{
	u32	proc_time;	//処理時間	4
	float total_ave;//4
	float total_dev;//4
	float predict;	//4
	s16	judge;		//2
	s16 ys[6];		//12
	s16 ye[6];		//12
	u8  thr_level;	//閾値レベル1
	u8  reserve[21];		// 予備
} ST_MID_RES_SPECIAL_B;

//落書き検知
typedef struct
{
	u32 up_proc_time;	     //処理時間	4
	s16 up_result;		     //処理結果 2	０＝真券、１＝落書き、２＝法輪功
	s16	up_err;			     //2
	u8 up_res_num;		     //1
	u8 up_cut_num;		     //1
	u8 padding[2];           //2
	u8 up_res_character[8];  //8

	u32 down_proc_time;	     //処理時間	4
	s16 down_result;		 //処理結果 2	０＝真券、１＝落書き、２＝法輪功
	s16	down_err;		     //2
	u8 down_res_num;		 //1
	u8 down_cut_num;		 //1
	u8 padding2[2];          //2
	u8 down_res_character[8];//8

	//u32 face_proc_time;//4
	//u32 up_whole_proc_time;//4
	//u32 down_whole_proc_time;//4

	//u8 face_result;//1
	//u8 up_whole_result;//1
	//u8 down_whole_result;//1
	//u8 padding3;//1

	u8 reserve[24];//

}ST_MID_RES_GRAFFITI_TEXT;

//機種毎データ 128 byte
typedef struct
{
	u32	cap_info_array_base;	//0-3
	u32	cap_info_array_set;		//4-7
	u32	cap_info_array_blid;	//8-11
	u32	cap_info_array_capst;	//12-15

	u8 cisdt_set_prebk;			//16
	u8 cisdt_st_oldestbk;		//17
	u8 reserve1[2];				//18-19

	float skew_edge;			//20-23
	float length_edge;			//23-27
	u16 mlti_edge[32];			//27+64=91

	u8  reserve[36];			//92+36=128 予備
} ST_MODEL_AREA;


//CF_NN_1COLOR
#define CF_NN_LAYER_NUM 3
typedef struct
{
	u32	  proc_time[CF_NN_LAYER_NUM];		//処理時間
	float genuine_val[CF_NN_LAYER_NUM];		//真券発火値
	float counterfeit_val[CF_NN_LAYER_NUM];	//偽造券発火値
	u8	  each_result[CF_NN_LAYER_NUM];		//各レイヤーの判定結果 0:真券 1:偽造券 (スキップ時は0)
	u8    result;	                        //総合的な判定結果 0or1:真券 2:偽造券 総合判定で求める
	u8    output_level[CF_NN_LAYER_NUM];    //出力レベル
	u8    thr_level;	                    //閾値レベル
	u8	  reserve[20];		                // 予備
} ST_MID_RES_CF_NN_1COLOR;

//CF_NN_2COLOR
typedef struct
{
	u32	  proc_time[CF_NN_LAYER_NUM];		//処理時間
	float genuine_val[CF_NN_LAYER_NUM];		//真券発火値
	float counterfeit_val[CF_NN_LAYER_NUM];	//偽造券発火値
	u8	  each_result[CF_NN_LAYER_NUM];		//各レイヤーの判定結果 0:真券 1:偽造券 (スキップ時は0)
	u8    result;	                        //総合的な判定結果 0or1:真券 2:偽造券 総合判定で求める
	u8    output_level[CF_NN_LAYER_NUM];    //出力レベル
	u8    thr_level;	                    //閾値レベル
	u8	  reserve[20];		                // 予備
} ST_MID_RES_CF_NN_2COLOR;

//ｶｽﾀﾑﾁｪｯｸ
typedef struct
{
	u32	proc_time;	//処理時間	4
	u8 result_num;  //出力レベルの判定番号　5
	u8 result_area; //出力レベルのエリア番号　6
	u8 sum_num;     //全エリアの合計CF判定数　7
	u8 sum_area;    //CF判定されたエリア数　8
	u8 level;       //出力レベル　9
	u8 temp;        //10
	u8 temp1;		//11
	u8 temp2;		//12
	s32 value;		//16　出力レベルの計算結果
	u16 judge[20];  //エリアごとの判定結果（16進）　56
	u8 thr_level;	//57 閾値レベル
	u8 result;	    //58 総合的な判定結果 0or1:真券 2:偽造券
	u8 reserve[6];

} ST_MID_RES_CUSTOMCHECK;

//**************************************


typedef struct
{
	u32 plane_num;
	ST_PLANE_INFOMATION PlaneInfo;
} ST_PLANE_INFOMATION_DATA;

#if 1

#define CAP_SNSDAT_SIZE			(FILE_SIZE)		// センサーデータサイズを追記、13844480をFILE_SIZE(13527040)に変更, 19/05/14
#define CAP_SNSDAT_SIZE_RAW		(FILE_SIZE_RAW)		// CIS生データ(10ビット)採取のセンサーデータサイズを追記, 20/07/28

typedef struct
{
	// ファイル情報「256バイト」
	u32		version;					//バージョン
	u32		file_size;					//ファイルサイズ
	u8		model_code[8];				//機種コード
	u8		currency_code[4];			//通貨コード
	u32		denomi_code;				//金種コード	19/05/27 K.YABU

	struct {
		u16 mode:4;						// スキャンモード, 20/01/20
		u16 sensor_conf:4;				// センサ構成, 20/01/20
		u16 model_calibration:8;		// モデル調整値, 20/01/20
	} spec_code;						//仕様コード	19/05/27 K.YABU

	u8		blank0[21];					//予備		19/05/27 K.YABU
	u8		insert_way;					//方向コードファイル情報 データコレクション時に設定される
	u8		category[8];				//分類コード
	u8		collected_comments[64];		//採取コメント
	u8      LEorSE;						//0:ロングエッジ, 1:ショートエッジ
	u8		blank1;						//ブランク
	u8		Blocksize;					//1ブロックは何サブブロックで構成されているか
	u8		Subblock_dpi;				//サブブロックのdpi
	u32		Blockbytesize;				//1ブロックあたりのバイト数
	float	pitch_ratio;				//搬送ピッチ補正
	u8		dct_ser_num1[16];			//記番号情報１	データコレクション時に設定する
	u8		dct_ser_num2[16];			//記番号情報２	データコレクション時に設定する
	u8		dct_ser_ref_num1;			//記番号1の整理番号	データコレクション時に設定する
	u8		dct_ser_ref_num2;			//記番号2の整理番号	データコレクション時に設定する
	u16		bill_sub_category_code;		//媒体サブ分類コード
	u8		reserve[88];				//予備
	
	//プレーン情報「1536バイト」
	ST_PLANE_INFOMATION PlaneInfo[MAX_PLANE_COUNT];
	u8		reserve_PlaneInfo2[192];	//ブランク
	
	// 処理結果・中間情報8192
	u16		proc_num;				//処理連番
	u16		buf_num;				//バッファ番号
	u16		blank3;					//予備
	s16		temperature;			//温度
	s64		epoch_time;				//時刻
	u16		block_count;			//有効ブロック数		
	u16		padding;				//TBD
	u8		insertion_direction;	//方向コード　識別結果が設定される。
	u8		transport_direction;	//搬送方向　0正方工：１逆
	s16		angle;					//スキュー角
	s16		center_x;				//紙幣の中心座標x
	s16		center_y;				//紙幣の中心座標y
	s16		note_x_size;			//主走査方向幅
	s16		note_y_size;			//副走査方向幅
	s16		result_e_code;			//判定結果情報
	u8		category_ecb;			//カテゴリ
	u8		blank5;					//予備
	u8		ser_num1[16];			//記番号１	OCR結果が設定される
	u8		ser_num2[16];			//記番号２	OCR結果が設定される
	u8		soft_ver[16];			//ソフトバージョン
	u8		fpga_ver[16];			//FPGAバージョン
	u8		template_ver[16];		//テンプレートバージョン
	u8		unit_ser[16];			//ユニットシリアル番号
	u16		processing_time[10];	//識鑑別処理時間記録用
	u8		series_color_1;			//記番号の色情報１ 0x20色なし,0x31黒,0x32茶,0x33青,0x3F不明	add furuta 0903
	u8		series_color_2;			//記番号の色情報２ 0x20色なし,0x31黒,0x32茶,0x33青,0x3F不明	add furuta 0903
	u16		padding2;				//TBD
	s16		left_up_x;				//頂点左上ｘ
	s16		left_up_y;				//頂点左上ｙ
	s16		left_down_x;			//頂点左下ｘ
	s16		left_down_y;			//頂点左下ｙ
	s16		right_up_x;				//頂点右上ｘ
	s16		right_up_y;				//頂点右上ｙ
	s16		right_down_x;			//頂点右下ｘ
	s16		right_down_y;			//頂点右下ｙ

	union fitness_t
	{
		u32 all;
		struct
		{
			u32 result		: 8;	//結果 	(0:ATMフィット／1:フィット／2:アンフィット)
			u32 level		: 8;	//レベル(0:無し／1～100:設定値)
			u32 threshold_1	: 8;	//閾値1	(ATMフィットとフィット間)
			u32 threshold_2	: 8;	//閾値2	(フィットとアンフィット間)
		} bit;
	} fitness[MAX_FITNESS_COUNT];			//ダイノート＋フィットネス

	u8		threshold_3[MAX_FITNESS_COUNT - 1];			//フィットネス閾値3 (アンフィットとリジェクト間)
	u8		threshold_4[MAX_FITNESS_COUNT - 1];			//フィットネス閾値4（マージン値）


	u8		blank2[20];				//BVの処理結果。上位に送信した情報など。

	u16		tled[48];				//LEDの発光時間	21.07.19 MRX-CISより
	u16		cis_base_value[48];		//cis調整の基準データ	21.07.19 MRX-CISより
	u8		sensor_info_blank[3072];//センサー調整情報を記録するメモリの空き	21.07.19 MRX-CISより

											//中間情報	合計2880byte 45個分
	ST_MID_RES_OUTLINE	mid_res_outline;	        // 外形検知				0
	ST_MID_RES_NN_IDENT	mid_res_nn;			        // 識別					1
	ST_MID_RES_DOR_EAR	mid_res_dog;		        // 角折れ				2
	ST_MID_RES_TEAR		mid_res_tear;		        // 裂け					3
	ST_MID_RES_DYE_NOTE	mid_res_dyenote;	        // ダイノート			4
	ST_MID_RES_SOILING	mid_res_soiling;	        // 汚れ					5
	ST_MID_RES_DE_INKD	mid_res_de_ink;		        // 脱色					6
	ST_MID_RES_STAIN	mid_res_stain;		        // 染み					7
	ST_MID_RES_HOLE		mid_res_hole;		        // 穴					8
	ST_MID_RES_CIR_SERIES	mid_res_3cir;	        // ３色比				9
	ST_MID_RES_CIR_SERIES	mid_res_4cir;	        // ４色比				10
	ST_MID_RES_CIR_SERIES	mid_res_mcir;	        // MCIR					11
	ST_MID_RES_CIR_SERIES	mid_res_ir_ck;	        // IRチェック			12
	ST_MID_RES_DOUBLE_CHECK	mid_res_double_ck;      // 重券					13
	ST_MID_RES_VALI_NN	mid_res_nn1;		        // NN1					14
	ST_MID_RES_VALI_NN	mid_res_nn2;		        // NN2					15
	ST_MID_RES_IR2WAVE	mid_res_ir2wave;	// IR2波長			16
	ST_MID_RES_MAG		mid_res_mag;		        // MAG					17
	ST_MID_RES_UV_VALI	mid_res_uv_validate;        // UV鑑別				18
	ST_MID_RES_FOLDING	mid_res_folding;	        // 折り畳み				19
	ST_MID_RES_BAR_CODE mid_res_bar_code;	// バーコード		20 *2
	ST_MID_RES_TAPE mid_res_tape;			// テープ			21
	ST_MID_RES_CAP_TAPE mid_res_cap_tape;	// 静電				22
	ST_MID_RES_NEOMAG mid_res_neomag;		// neomag			23
	ST_MID_RES_MAGTHREAD mid_res_magthread;	// 磁気スレッド		24
	ST_MID_RES_METALTHREAD mid_res_metalthread;	// 金属スレッド 25
	ST_MID_RES_HOLOGRAM mid_res_hologram;	// ホログラム　		26
	ST_MID_RES_SPECIAL_A mid_res_special_a;	// 特殊A			27
	ST_MID_RES_SPECIAL_B mid_res_special_b;	// 特殊B　			28
	u8 mid_res_reserve30[64];				// よれ				29
	ST_MID_RES_GRAFFITI_TEXT mid_res_graffiti_text;	// 落書き検知　文字検知	30
	u8 mid_res_reserve32[64];				// しわ				31
	ST_MID_RES_UV_FITNESS mid_res_uv;			// UV正損			32
	ST_MID_RES_DBL_CHK_MECHA mid_res_dbl_ck_mecha;	// 重券検知（メカ厚）	33
	ST_MID_RES_CF_NN_1COLOR mid_res_cf_nn_1;	    // cf-nn1color			34
	ST_MID_RES_CF_NN_1COLOR mid_res_cf_nn_2;	    // cf-nn1color			35
	ST_MID_RES_CUSTOMCHECK mid_res_customcheck;		// カスタムチェック		36

	u8		mid_res_reserve[448];			//中間結果の予約領域

	u8		fitness_result_original[MAX_FITNESS_COUNT - 1];	//正損再現モードによって書き換えれる前の
															//フィットネスレベル結果の元々の値

	u32		validation_result_flg;	//鑑別結果の"OK","NG"を記録する。

	float	disable_area_a;			//無効エリアA
	float	disable_area_b;			//無効エリアB

	//OCRで求めた記番号の左上と右下の座標　add by furuta 20220114
	s16     series_1_top_x;
	s16     series_1_top_y;
	s16     series_1_bottom_x;
	s16     series_1_bottom_y;
	//２カ所目
	s16     series_2_top_x;
	s16     series_2_top_y;
	s16     series_2_bottom_x;
	s16     series_2_bottom_y;


	u8		bottom_edge[324];		//予備

	u8		mri_code[16];			//MRIコード　CNY5.2など

	u8		blank4[1132];			//その他情報、予備

#ifdef BAU_LE17
	ST_MODEL_AREA st_switch_model;	//機種ごとに異なる定義部分(128byte分)
#else
	#if 1
		ST_MODEL_AREA st_model_area;
	#else
	u8		blank6[128];			//デバッグ用として自由な値を書き込める領域。
	#endif
#endif

	// センサーデータ
#if 1//#ifdef _RM_
	u8		sens_dt[CAP_SNSDAT_SIZE];		//センサーデータ、13844480をCAP_SNSDAT_SIZE(13527040)に変更, 19/07/29
#else
	u8		sens_dt[13844480];		//センサーデータ13844480
#endif

} ST_BS;	// sample data 一枚分

#endif

typedef struct
{
	float	tan_th;
	float	cos_th;
	float	sin_th;
} ST_ANGLE;

typedef struct
{
	s16		left_up_x;				//頂点左上ｘ
	s16		left_up_y;				//頂点左上ｙ
	s16		left_down_x;			//頂点左下ｘ
	s16		left_down_y;			//頂点左下ｙ
	s16		right_up_x;				//頂点右上ｘ
	s16		right_up_y;				//頂点右上ｙ
	s16		right_down_x;			//頂点右下ｘ
	s16		right_down_y;			//頂点右下ｙ

} ST_VERTEX;

typedef struct
{
	s8 transport_flg;			//搬送方向フラグ　1順　－１逆
	s8  x_y_change_flg;		//SEとLEを共通させる
	s16 main_eff_range;		//有効画素数

	s8  scan_dir_skew_coefficient[MAX_PLANE_COUNT];	//スキャン方向補正用係数
	u8  padding[2];

	s16	temp_center_x[MAX_PLANE_COUNT];		//補正した中心座標
	float  insart_multi_x[MAX_PLANE_COUNT];		//挿入方向補正用ｘ
	float  insart_multi_y[MAX_PLANE_COUNT];		//挿入方向補正用ｘ
	s16 scan_diy_flg[MAX_PLANE_COUNT];		//スキャン方向を補正する値
	//float main_offset[MAX_PLANE_COUNT];	//主走査方向オフセット
	s16 main_offset[MAX_PLANE_COUNT];	//主走査方向オフセット
	float main_dpi_cor[MAX_PLANE_COUNT];	//主走査dpi補正
	float sub_dpi_cor[MAX_PLANE_COUNT];	//副走査dpi補正

	float coordinate_param_x[MAX_PLANE_COUNT];				//画素値が必要な時に使う（L_Getdt内で使用する
	float coordinate_param_x_non_ofs[MAX_PLANE_COUNT];		//座標のみが必要な時に使う（L2P内で使用する

	float coordinate_param_y[MAX_PLANE_COUNT];				//
	float coordinate_param_y_non_ofs[MAX_PLANE_COUNT];		//
	
	float sin_x[MAX_PLANE_COUNT];		//
	float cos_x[MAX_PLANE_COUNT];		//

	float sin_y[MAX_PLANE_COUNT];		//
	float cos_y[MAX_PLANE_COUNT];		//

	u16	sub_eff_range[MAX_PLANE_COUNT];	//搬送方向の有効範囲

	u32 *pplane_tbl;			//

	float pitch_reciprocal;									//搬送ピッチ補正値の逆数
	u32   image_coordinate_system_main;						//表示イメージの座標系への変換値（主走査方向

} ST_NOTE_PARAMETER;

//紙幣一枚分の情報を保存
typedef struct
{
	ST_NOTE_PARAMETER note_param;
	ST_BS *pbs;

	ST_NOTE_PARAMETER p2l_params;
	
} ST_WORK;


//typedef struct
//{
//	// ファイル情報「256バイト」
//	u32		version;					//バージョン
//	}ST_BS_TEST;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	u16 P_Getdt(ST_SPOINT *__restrict spt ,u8 buf_num);
	u16 P_Getdt_8bit_only(ST_SPOINT *__restrict spt ,ST_BS * __restrict pbs);
	u16 P_Getdt_16bit_only(ST_SPOINT *__restrict spt ,u8 buf_num);
	u16 L_Getdt(ST_SPOINT *spt ,u8 buf_num);
	u16 P_Getdt_dpi_and_offset(ST_SPOINT *spt ,u8 buf_num);
	s16 P2L_Coordinate(ST_SPOINT* spt ,u8 buf_num);
	s16 L2P_Coordinate(ST_SPOINT* spt ,u8 buf_num);
	u16 scanP_Getdt(ST_SPOINT *spt ,u8 buf_num);

	void x_y_change(s32 *x ,s32 *y);
	void skew_correction(s32 *x ,s32 *y, ST_ANGLE* ang);
	void insert_correction(ST_SPOINT *spt ,ST_BS *pbs);
	u16 insert_and_scan(ST_SPOINT *spt ,u8 buf_num);
	void set_skew_info(ST_BS* pbs, u8 Plane, ST_ANGLE* ang);
	s16 get_threshold_background(s16 safe_val ,ST_BS *pbs ,u8 plane,u8 buf_num);
	s16 get_threshold_note(s16 safe_val ,ST_BS *pbs ,u8 plane,u8 buf_num);
	s16 cal_note_parameter(u8 buf_num);
	s16 cal_P2L_parameter(u8 buf_num);
	s16 callback(s32 para[]);			// debug
	void p2l_insert_correction(ST_SPOINT *spt ,ST_BS *pbs);
	void set_planetbl_(u8 buf_num);

	u16 L_Getdt_unstable(ST_SPOINT *spt ,u8 buf_num);
	void reset_insart_multi_num(u8 buf_num);
	void advance_insert_correction(u8 buf_num);
	u16 new_L_Getdt(ST_SPOINT * __restrict spt , ST_BS * __restrict pbs , ST_NOTE_PARAMETER * __restrict pnote_param );
	u16 new_L_Getdt_16bit_only(ST_SPOINT * __restrict spt , ST_BS * __restrict pbs , ST_NOTE_PARAMETER * __restrict pnote_param );
	u16 new_L_Getdt_08bit_only(ST_SPOINT * __restrict spt , ST_BS * __restrict pbs , ST_NOTE_PARAMETER * __restrict pnote_param );
	s16 new_L2P_Coordinate(ST_SPOINT *__restrict spt , ST_BS * __restrict pbs , ST_NOTE_PARAMETER * __restrict pnote_param );
	s16 new_P2L_Coordinate(ST_SPOINT *__restrict spt , ST_BS * __restrict pbs );
	void decision_plane_side_in_insertion_direction(u8 buf_num);

	s16	get_vertex(ST_VERTEX *pver ,u8 buf_num);
	void quick_sort( u8 ListToSort[500], s32 left, s32 right );

	u8  level_detect(float *detect_res_ary, float *thr_ary,	u8 detect_res_num ,float min ,float max);
	u32 get_cutout_color_img(u8 buf_num, u8 *p_pln, u8 n, u16 hid, u16 wid, s16 xs, s16 ys, u8 coord, u8* p_img, u8 dpi);
	u16 P_GetThicknessdata(ST_BS * __restrict pbs, s32 posx, s32 posy, s8 single_mode);

#ifdef VS_DEBUG_ITOOL
	void for_Dbg_start_algo_label_setting(int algo_index, int coordinate_index);
#else
	void for_Dbg_start_algo_label_setting(void);
#endif
	void for_iTool_trace_callback(ST_SPOINT* __restrict spt, ST_BS* __restrict pbs, ST_NOTE_PARAMETER* __restrict pnote_param, u16 xsize, u16 ysize, u8* mask, u8 half_point_flg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GETDT_H_INCLUDED */
/* End of file */

