// global.h
#ifndef _GLOBAL_H
#define _GLOBAL_H

#if defined(EXT)
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif
#else
#undef EXTERN
#define EXTERN
#endif

#include "jcm_typedef.h"
//#define W0_A W0
//#define W1_B W1
//#define W3_C W3
//#define W2_D W2

//#define VS_DEBUG		//処理をトレースしたいとき
//#define VS_DEBUG_EDGE	//外形検知のトレース
//#define I_AND_A		//I&Aに提出するとき
//#define BAU_LE17		//LE17専用処理を用いるとき
#define _RM_			//実機で用いるとき（どの機種も

#define MAX_ALGORITHM_NUM	(64)	// アルゴの項目の最大数。
									// 識鑑別正損すべて合わせた数。

#define SAMPLINGDATA_BUFFAR_MAX_SIZE (10) //機種による

#ifndef _RM_
//生成
#define F_CREATE_1(fp) fp = fopen("param.csv", "a+");
//生成
#define F_CREATE(fp) fp = fopen("image.csv", "w");
//生成
#define F_CREATE_BIN(fp) fp = fopen("cutoutimage.bin", "w");
//書き込み　
#define F_WRITE(fp , a) fprintf(fp,"%d,",a);
//書き込み　
#define F_WRITE_LF(fp , a) fprintf(fp,"%lf,",a);
//書き込み　
#define F_WRITE_S(fp , a) fprintf(fp,"%s,",a);
//改行
#define F_N(fp) fprintf(fp,"\n");
//クローズ
#define F_CLOSE(fp) fclose(fp);
#if 0
typedef	signed char		s8;
typedef	short			s16;
typedef	signed long		s32;
typedef	long long		s64;

typedef	unsigned char	u8;
typedef	unsigned short	u16;
typedef	unsigned long	u32;
typedef	unsigned long long u64;
#endif
#endif

//実機用
#ifdef _RM_

#ifdef BAU_LE17		//LE17専用処理を用いるとき

#define TEMP_BILL_NOTE_IMAGE_TOP		((void*)0x00a00000)

#include "jcm_typedef.h"			//
#include "template_file_structure.h"
#include "baule17_image_inc.h"		// 18/10/25
#include "time_measure.h"
#include "parameter_set.h"
#include "currency_table.h"
#include "systemdef.h"			//LE17 MRX用？　FPGAのタイプスタンプ情報を取得する為
#define _not_MScplusplus
#include "make_bmp.h"


#endif

typedef u32 T_POINTER;

#endif

//シミュレータ用
#ifndef _RM_

EXTERN int get_count;
EXTERN int get_mat_counter;

typedef u64 T_POINTER;

EXTERN T_POINTER over_address_up;
EXTERN T_POINTER over_address_down;

#define TEMPLATE_HEADER_SIZE ((u32)9984)

//#include "../debug.h"

#endif

#define TEMPLATE_HEADER_SIZE ((u32)9984)

//共通
#include "getdt.h"
//#include "../barcode/barcode.h"
#include "replay_fitness.h"

//識別
#include "../ident/extradt.h"
#include "../ident/EdgeDetect.h"
#include "../ident/nn.h"

//フィットネス
#include "../fitness/dog_ear.h"
#include "../fitness/dye_note.h"
#include "../fitness/tear.h"
#include "../fitness/size_judge.h"
#include "../fitness/de_inked_note.h"
#include "../fitness/stain.h"
#include "../fitness/soiling_MLP.h"
#include "../fitness/soiling.h"
//#include "../fitness/soiling_ver2.h"
//#include "../fitness/soiling_ver3.h"
//#include "../fitness/soiling_ver4.h"
#include "../fitness/hole.h"
#include "../fitness/capacitance_tape.h"
#include "../fitness/tape.h"
//#include "../fitness/graffiti_C/graffiti.h"



//鑑別
#include "../validation/convert_data.h"
#include "../validation/cir_3color_check.h"
#include "../validation/cir_4color_check.h"
#include "../validation/double_check.h"
#include "../validation/double_check_mecha.h"
#include "../validation/ir_check.h"
#include "../validation/mcir_check.h"
#include "../validation/neural_network_1color.h"
#include "../validation/neural_network_2color.h"
//#include "../validation/cir_2color_check.h"
#include "../validation/custom_check.h"
#include "../validation/ir2wave_check.h"
#include "../validation/mag_check.h"
#include "../validation/uv_check.h"
#include "../fitness/folding.h"
//#include "../validation/capacitance_thread.h"
#include	"../validation/neomag.h"
#include	"../validation/thread.h"
#include	"../validation/hologram.h"
#include	"../validation/special_a.h"
#include	"../validation/special_b.h"
#include "../validation/double_check_new.h"
//OCR
#include "../ocr/ocr_time_test.h"
#include "../ocr/cod_config.h"

//テンプレートシェル関係
#include "../template/template.h"
#include "../template/template_result.h"
#include "../template/template_src_version.h"

//新識別
//#include "../denomination/test/simple_test.h"
#include "../ocr/cod_malloc.h"
#include "../denomination/config_structure.h"
#include "../denomination/outlier_checker.h"
#include "../denomination/utilities.h"
#include "../denomination/MLPClassifier.h"
#include "../denomination/mblbp.h"

typedef struct
{
	u8	fit_level[MAX_ALGORITHM_NUM];		//Fitの閾値レベル
	u8	uf_level[MAX_ALGORITHM_NUM];		// UFの閾値レベル
	u8	rj_level[MAX_ALGORITHM_NUM];		// リジェクトの閾値レベル
	u8	level_margin[MAX_ALGORITHM_NUM];	// 閾値レベルのマージン

} ST_FITNESS_LEVEL_THRESHOLDS;

typedef struct
{
	u8	fit_level;	//Fitの閾値レベル
	u8	uf_level;	//UFの閾値レベル
	u8	rj_level;	//リジェクトの閾値レベル
	u8	margin;		//閾値レベルのマージン

} ST_DEFAULT_FITNESS_LEVEL_THRESHOLDS;

// サイズチェックの値を保存
typedef struct
{
	u16 x_size; // xのサイズ
	u16 y_size; // yのサイズ

	u16 x_min_size;  // xの最小サイズしきい値
	u16 x_max_size; // xの最大サイズしきい値
	u16 y_min_size;  // yの最小サイズしきい値
	u16 y_max_size; // yの最大サイズしきい値
}ST_SIZE_LIMIT;

// バーコードリード処理　上位通信用構造体
// 上位で設定されたバーコードリード処理のパラメータを設定する。
typedef struct
{
	u8	setting_change_flg;			//上位で設定された情報を用いるか否か　1：用いる　0：用いらない
	u8  execute_flg;				//実行フラグ　１：許可　０：禁止
	u8	scan_side;					//スキャンする面　2:両面 0:表のみ　1：裏のみ
	u8	character_num_limits_max;	//最大桁数 指定の桁のみの読み取りにしたい場合、
	u8	character_num_limits_min;	//最小桁数 最大最小おなじ桁数を設定すればよい
	u8  padding[3];					//

}ST_TOP_CONFIG_BARCODE_READ_PARAMS;

// 外形検知処理　上位通信用構造体
// 上位で設定された外形検知処理のパラメータを設定する。
typedef struct
{
	float disable_area_a;				//無効エリアA
	float disable_area_b;				//無効エリアB

}ST_TOP_CONFIG_EDGEDETECT_PARAMS;

//テンプレート内券種ID - JCMID 対応テーブル情報を記録する。
typedef struct
{
#ifdef _RM_
	u32	tbl_ofs;							// テーブルオフセットアドレス
#else
	u64	tbl_ofs;							// テーブルオフセットアドレス
#endif

	u16	tbl_num;							// テーブルの構造体配列要素数
	u8	padding[2];							// padding
} ST_JCMID_TBL;

EXTERN ST_WORK work[SAMPLINGDATA_BUFFAR_MAX_SIZE];					//サンプリングデータ用構造体
//EXTERN ST_ANGLE ang;						//傾き情報構造体
//EXTERN ST_DATA_EXTRACTION data_extraction;	/*構造体定義*/
EXTERN s32 deb_para[10];					
//EXTERN ST_TWO_DIFF two_diff;
//EXTERN ST_FOUR_COMPARISON four_comp;
//EXTERN ST_EdgEDetecT ED;					//外形検知構造体
//EXTERN ST_UV UV;							//
//EXTERN ST_MAG Mag;							//
EXTERN ST_NOTE_PARAMETER note_param[SAMPLINGDATA_BUFFAR_MAX_SIZE];	//座標変換パラメタ構造体
//EXTERN ST_NN_RESULT nn_res;
//EXTERN ST_NN nn_cfg;						//NN用構造体
//EXTERN ST_DOG_EAR dog;						//角折れ検知構造体
//EXTERN ST_DYE_NOTE dy;						//ダイノート検知構造体
//EXTERN ST_TEAR tr;							//裂け検知構造体
//EXTERN ST_HOLE ho;							//穴検知構造体
//EXTERN ST_SOILING soi;						//汚れ検知構造体
//EXTERN ST_DEINKED deink;					//脱色検知構造体
//EXTERN ST_STAIN sta;						//染み検知構造体
//EXTERN ST_2CIR cir2;
//EXTERN ST_3CIR cir3;
//EXTERN ST_4CIR cir4;
//EXTERN ST_MCIR mcir;
//EXTERN ST_IR_CHECK ir_check;
//EXTERN ST_DOUBLE_CHECK double_check;
//EXTERN ST_VALI_NN1 vali_nn1;
//EXTERN ST_VALI_NN2 vali_nn2;
//EXTERN ST_CUSNTOM custom_check;
//EXTERN ST_SOFT_CASH soft_cash[100];

#ifdef _RM_
//EXTERN TEMPLATE_DATA *ptemplate_data;
//EXTERN ST_TIME st_time;

//EXTERN T_DECISION_PARAMETER_JCMID_TBL p_temp_currenct_table_top;
EXTERN void* p_main_currenct_table_top;

#endif

EXTERN ST_JCMID_TBL p_temp_currenct_table_top;							//テンプレート内券種IDのアドレス記録用
extern uint8_t ex_algorithm_version_no[];

#define MAX_CURRENCY_TABLE_NUM	(128)														//通貨テーブルの最大対応数
EXTERN ST_FITNESS_LEVEL_THRESHOLDS	fit_level_thrs[MAX_CURRENCY_TABLE_NUM];					//通貨テーブルから受け取る各判定のレベルの閾値。
EXTERN ST_DEFAULT_FITNESS_LEVEL_THRESHOLDS	default_fit_level_thrs[MAX_VALIDATION_COUNT];	//通貨テーブルに設定する各アルゴの閾値のデフォルト値
EXTERN ST_TOP_CONFIG_BARCODE_READ_PARAMS barcode_read_top_config;							//上位で設定されたバーコードリード処理のパラメタを格納する。
EXTERN ST_TOP_CONFIG_EDGEDETECT_PARAMS edgedetect_top_config;								//上位で設定された外形検知処理のパラメタを格納する。
#ifdef OCR_DEBUG
EXTERN u8 global_denomi;
#endif
#ifdef OCR_PARAM_DEBUG
EXTERN int ocr_app_flag;
#endif

// ichijo
EXTERN ST_SIZE_LIMIT	size_limit;					//通貨テーブルから受け取る各判定のレベルの閾値。


/********************************************************************
 * RBA-40C ADD define
 ********************************************************************/
#include "../src/country_custom.h"
#include "parameter_set.h"
#include "../ident/EdgeDetectDC.h"
#include "../validation/data_diameter.h"
#include "../validation/rba_custom_check.h"
#include "../validation/point_uv_check.h"
#include "jdl_conf.h"

EXTERN ST_EdgEDetecT_DC ED_DC;
EXTERN ST_EdgEDetecT_DC rev_ED;
EXTERN ST_RBA_CUSTOM_CHECK cs_chk;
EXTERN ST_POINT_CHECK pt_chk;
EXTERN ST_REF_CHECK rf_chk;
EXTERN ST_PEN_CHECK pen_chk;
EXTERN u8 point_uv_chk_invalid_count;
EXTERN u8 point_mag_chk_invalid_count;

enum NN_CHECK
{
	NN_CHECK_OK,
	NN_CHECK_EDGE_DETECT_ERROR,
	NN_CHECK_EXTRACTION_ERROR,
	NN_CHECK_SHORT_ERROR,
	NN_CHECK_LONG_ERROR,
};

EXTERN ST_DATA_EXTRACTION rev_data_extraction;
EXTERN ST_NN_RESULT rev_nn_res;
EXTERN ST_NN rev_nn_cfg;

#define THRESHOLD_1		0.7//<-0.8		//a 最大発火値用すれっしゅ
#define THRESHOLD_2		0.5//<-0.7		//a Δ発火値用すれっしゅ

#define DIVIDE_200DPI		0.0020449897f
#define DIVIDE_100DPI		0.0038910506f
#define DIVIDE_50DPI		0.0071942448f

#define MCIR_CHECK_COUNT 6

#define NETWORK_INPUT_NODE_COUNT	80
#define NETWORK_HIDDEN_NODE_COUNT	16
#define NETWORK_OUTPUT_NODE_COUNT	2

EXTERN float input[NETWORK_INPUT_NODE_COUNT];
EXTERN float hidden[NETWORK_HIDDEN_NODE_COUNT];
EXTERN float output[NETWORK_OUTPUT_NODE_COUNT];


#define BILL_NOTE_IMAGE_MAX		1		//何枚保持できるかにより変動　メモリとの相談
#define SENSOR_DATA_OFFSET		9984	//センサデータまでのオフセット
/**** Sampling Data Parameter ****/
#define INFO_VERSION 						0x00000005 //Version : 5 (2020-03-09)
#define INFO_SHORT_EDGE						1
#define INFO_SUB_BLOCK_SIZE					4
#define INFO_SUB_BLOCK_DPI					200

enum PLANE_REF_OR_TRANS
{
	PLANE_REFLECTIVE,
	PLANE_TRANSPARENT
};
enum PLANE_NOTE_SCAN_SIDE
{
	PLANE_NOTE_SCAN_SIDE_UP,
	PLANE_NOTE_SCAN_SIDE_DOWN
};
enum PLANE_ENABLE_OR_DISABLE
{
	PLANE_DISABLE,
	PLANE_ENABLE
};
enum PLANE_NOTE_SCAN_DIR
{
	PLANE_NOTE_SCAN_DIR_POSITIVE,
	PLANE_NOTE_SCAN_DIR_REVERSE
};

#define MAX_LEVEL			1
#define MIN_LEVEL			100
#define THREAD_LOW_LEVEL	40

#define PLANE_MAIN_ELEMENT_PICH				0.127

#define	PLANE_PITCH_200DPI					1
#define	PLANE_PITCH_100DPI					2
#define	PLANE_PITCH_50DPI					4
#define PLANE_SUB_ELEMENT_PICH_200DPI		0.127//<-0.1245
#define PLANE_SUB_ELEMENT_PICH_100DPI		0.254//<-0.249
										//new//old
#define PLANE_SUB_OFFSET_UP_R_R				0//<--63
#define PLANE_SUB_OFFSET_UP_R_G				0//<--63
#define PLANE_SUB_OFFSET_UP_R_IR1			0//<--63
#define PLANE_SUB_OFFSET_UP_R_IR2			0//<--64
#define PLANE_SUB_OFFSET_DOWN_R_R			0//<--1
#define PLANE_SUB_OFFSET_DOWN_R_G			0//<--1
#define PLANE_SUB_OFFSET_DOWN_R_IR1			0//<--1
#define PLANE_SUB_OFFSET_DOWN_R_IR2			0//<--2
#define PLANE_SUB_OFFSET_DOWN_T_R			0//<--0
#define PLANE_SUB_OFFSET_DOWN_T_G			0//<--0
#define PLANE_SUB_OFFSET_DOWN_T_IR1			0//<--1

#define PLANE_SUB_OFFSET_MAG				-32//?

#define RESULT_TRANSPORT_DIRECTION_INSERT_BILL	0
#define RESULT_TRANSPORT_DIRECTION_PAYOUT_BILL	1

/* 外形検知パラメータ */
#define USE_EDGE_DETECT_SENSOR				DOWN_T_G
#define ST_EDGE_DETECT_MINSE				10
#define ST_EDGE_DETECT_MAXSE				2000
#define ST_EDGE_DETECT_MINLE				10
#define ST_EDGE_DETECT_MAXLE				2000
#define ST_EDGE_DETECT_STEP_MOVE			4
#define ST_EDGE_DETECT_SMALL_BACKTRACK		25

#define PLANE_ADDRESS_PERIOD_UP_R_R			5112
#define PLANE_ADDRESS_PERIOD_UP_R_G			5112
#define PLANE_ADDRESS_PERIOD_UP_R_IR1		10224
#define PLANE_ADDRESS_PERIOD_UP_R_IR2		10224
#define PLANE_ADDRESS_PERIOD_DOWN_R_R		5112
#define PLANE_ADDRESS_PERIOD_DOWN_R_G		5112
#define PLANE_ADDRESS_PERIOD_DOWN_R_IR1		10224
#define PLANE_ADDRESS_PERIOD_DOWN_R_IR2		10224
#define PLANE_ADDRESS_PERIOD_DOWN_T_R		20448
#define PLANE_ADDRESS_PERIOD_DOWN_T_G		10224
#define PLANE_ADDRESS_PERIOD_DOWN_T_IR1		20448

#define PLANE_ADDRESS_PERIOD_MAG			5112

#define PLANE_ADDRESS_OFFSET_UP_R_R			8
#define PLANE_ADDRESS_OFFSET_UP_R_G			1464
#define PLANE_ADDRESS_OFFSET_UP_R_IR1		2920
#define PLANE_ADDRESS_OFFSET_UP_R_IR2		8032
#define PLANE_ADDRESS_OFFSET_DOWN_R_R		736
#define PLANE_ADDRESS_OFFSET_DOWN_R_G		2192
#define PLANE_ADDRESS_OFFSET_DOWN_R_IR1		3648
#define PLANE_ADDRESS_OFFSET_DOWN_R_IR2		8760
#define PLANE_ADDRESS_OFFSET_DOWN_T_R		4376
#define PLANE_ADDRESS_OFFSET_DOWN_T_G		9488
#define PLANE_ADDRESS_OFFSET_DOWN_T_IR1		14600

#define PLANE_ADDRESS_OFFSET_MAG			5096


#endif /* _GLOABAL_H */
