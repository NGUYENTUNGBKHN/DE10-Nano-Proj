#ifndef _COD_CONFIG_H_
#define _COD_CONFIG_H_

//#define _COD_GETSERIES_RULES_FOR_INR_BANKNOTE_
#define _COD_GETSERIES_RULES_FOR_CNY_BANKNOTE_
//#define _COD_GETSERIES_RULES_FOR_CNY_BANKNOTE_char_type_0000000000
#define _OLD_SVM_INFERENCE_

//#define VALID_CNY_OCR
//#define VALID_JPY_OCR 1
//#define VALID_EUR_OCR



#ifdef VALID_EUR_OCR
#define MODEL_INPUT_SIZE 14
#else
#define MODEL_INPUT_SIZE 28
#endif

#define COLOR_MODEL_INPUT_SIZE 28

//#define visualize1	//Huong add 20210818

// Huong add 20220719 tạo mảng động trung gian
//typedef	unsigned char	u8;
//extern u8* matrix_temp_img0;
//extern u8* matrix_temp_img1;
//extern int* matrix_temp_img_int;
//extern float* matrix_temp_float0;
//extern float* matrix_temp_float1;
//extern float* matrix_temp_float2;
//extern unsigned char* matrix_temp_width;
//extern unsigned char* matrix_temp_height;

typedef struct
{

	int starting_x;					//スタート座標ｘ
	int starting_y;					//スタート座標ｙ
	int width;						//横のサイズ
	int height;						//縦のサイズ
	char side;						//UP/DOWN
	char color;						//色
	int series_length;				//シリアルの長さ
	const char* character_types;	//文字タイプ　0：どちらも　1：数字　2：アルファベット
	int switch_binarization_value;	//平均ピクセル値
	int binarization_threshold;		//2値化の閾値
	int threshold_on_y;				//Y座標用閾値
	int threshold_on_x;				//X座標用
	int threshold_on_offset;		//文字絞り込みの為の閾値
	char flip_x;					//反転
	char flip_y;					//
	char rotate;					//回転
	int dpi;						//dpi
	char area_num;					//シリアル箇所番号
	char model_num;					//モデルの番号		//add by furua 200827
	char cutout_size;				//切り出しサイズ	//add by furua 200827

	//記番号の箇所番号
	//テンプレート対応ができたら
	float	*p_composite_weight;
	int		composite_weight_num;
	char	*p_composite_labels;
	char	composite_labels_num;

	float	*p_numbers_weight;
	int		numbers_weight_num;
	char	*p_numbers_labels;
	char	numbers_labels_num;
	
	float	*p_letter_weight;
	int		letter_weight_num;
	char	*p_letter_labels;
	char	letter_labels_num;
	int model_color;				// model type of model color detect (add by hoan)
	char jpy_flg;					// 日本円の場合処理が分岐するのでその条件用 0:jpy以外 1:JPY (add by furua 200908

	short top;						// 上位通知用
	short bottom;					// 記番号座標
	short left;						// add by furuta 20220114
	short right;					//

} COD_Parameters;

//typedef struct cod_parameters COD_Parameters;


#endif
