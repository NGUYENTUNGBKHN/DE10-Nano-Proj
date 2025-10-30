#ifndef _TEMPLATE_RESULT_H_INCLUDED_
#define _TEMPLATE_RESULT_H_INCLUDED_

////実機向け
//#include "systemdef.h"				// 19/07/09
//#include "template.h"
//#include "baule17_image_inc.h"		// 18/10/25
//#include "jcm_typedef.h"			//

typedef struct							// 判定パラメタ　テンプレート内券種ID - JCMID 対応テーブル　可変長部　	
{
	u32 jcm_id;							//JCM券種ID
	u16	template_id;					//テンプレート内券種ID
	u8	template_rev;					//テンプレートのリビジョン
	u8	currency[3];					//通貨コード	ユーロの時は‘EUR’（0x45, 0x55, 0x52）
	u8	index_num;						//インデックス番号
	u8	padding;						//パディング
	u8	denomi_code[32];				//金種コード
	u8	denomi_code_version[32];		//金種コードのバージョン
	u8	emission[64];					//エミッション
	s8	price_exp;						//金額情報　指数部
	u8	price_sign;						//金額情報　仮数部
	u8	note_size_x_mm;					//長手方向の紙幣サイズ　単位は㎜
	u8	note_size_y_mm;					//短手方向の紙幣サイズ　単位は㎜
	u8  info[24];						//必要ないので配列で定義
	//ST_SERIAL_INFO serial_info1;		//シリアル番号の情報1	  12
	//ST_SERIAL_INFO serial_info2;		//シリアル番号の情報2     12
	//ST_CLEAR_WINDOW_INFO clear_info1;	//クリアウィンドウの情報1 8
	//ST_CLEAR_WINDOW_INFO clear_info2;	//クリアウィンドウの情報2 8

} T_TEMP_SHELL_JCMID_TBL;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
// プロトタイプ宣言
//前処理関係
void	oline_det_result(u8 buf_num ,ST_EdgEDetecT *ED);
void	extraction_pointdate_result(u8 buf_num ,ST_DATA_EXTRACTION *data_extraction);
u16		NN_judeg_proc_result(u8 buf ,ST_NN* nn_cfg, ST_NN_RESULT *nn_res);
//u16		New_NN_judeg_proc_result(u8 buf, new_nn_config* nn_cfg, ST_NEW_NN_RESULT* nn_res);
void	oline_size_check_res(u8 buf_num ,ST_SJ sj);
void	oline_size_ident_res(u8 buf_num ,ST_SJ sj ,u16 temp_id);
//void	 barcode_read_result(u8 buf, ST_BARCODE_READ barcode);


//鑑別関係
void	cir_3color_result(u8 buf_n ,u16 invalid_count, u8 level);	//3色比
void	cir_4color_result(u8 buf_n ,u16 invalid_count, u8 level);	//4色比
void	ir_check_result(u8 buf_n ,u16 invalid_count, u8 level);	//ircheck
void	mcir_result(u8 buf_n ,u16 invalid_count, u8 level);	//mcir
void	double_check_result(u8 buf_n ,s16 invalid_count, u8 level);	//重券
void	double_check_optical_result(u8 buf_n, ST_DOUBLE_OPTICS_NN_RESULT* res);	//新重券
void	double_check_mecha_result(u8 buf, ST_DBL_CHK_MECHA st);	// 重券検知（メカ厚）
void	nn1_result(u8 buf_n ,u8 invalid_count ,float gen_v ,float con_v , u8 level);	//鑑別1色NN
void	nn2_result(u8 buf_n ,u8 invalid_count ,float gen_v ,float con_v , u8 level);	//鑑別2色NN
void	ir2wave_result(u8 buf_n, ST_IR2WAVE);
void	mag_result(u8 buf_n, ST_MAG);
void	neomag_result(u8 buf_n, ST_NEOMAG neomag);
void	magthread_result(u8 buf_n, ST_THREAD thr);
void	metalthread_result(u8 buf_n, ST_THREAD thr);
void	hologram_result(u8 buf_n, ST_HOLOGRAM holo);
void	uv_validate_result(u8 buf, ST_UV_VALIDATE uv);
void	special_a_result(u8 buf, ST_SPECIAL_A spa);
void	special_b_result(u8 buf, ST_SPECIAL_B spa);
void	customcheck_result(u8 buf, ST_CUSTOMCHECK_PARA ccp);
void	cf_nn_1color_result(u8 buf, ST_NN* nn_cfg, ST_NN_RESULT* nn_res);
void	cf_nn_2color_result(u8 buf, ST_NN* nn_cfg, ST_NN_RESULT* nn_res);

//フィットネス関係
void	dog_ear_result(u8 buf ,ST_DOG_EAR st);	//角折れ
void	tear_result(u8 buf ,ST_TEAR st);			//裂け
void	hole_result(u8 buf  ,ST_HOLE ho );				//穴
void	dyenote_result(u8 buf ,ST_DYE_NOTE st );			//ダイノート
void	soiling_result(u8 buf ,ST_SOILING st );			//汚れ
void	deink_result(u8 buf ,ST_DEINKED st );				//脱色
void	stain_result(u8 buf  ,ST_STAIN st );				//染み
void	folding_result(u8 buf ,ST_FOLDING st );				//染み
void	tape_result(u8 buf, ST_TAPE st);				// テープ
void	cap_tape_result(u8 buf, ST_CAPACITANCE_TAPE st);				// 静電テープ
void	uv_result(u8 buf_n, ST_UV_VALIDATE a);
//void	graffiti_text_result(u8 buf, ST_GRAFFITI_TEXT st);

//OCR
void	ocr_result(u8 buf ,char* res_1 ,char* res_2, COD_Parameters* st ,u32 color_time ,u8 color);			//OCR

//共通
void	overall_judge_proc_result(u8 buf_num);
void*	add_result_blk(u16 siz, T_TEMPLATE_WORK* pst_work, u16 cnd_code, u16 tmplt_ID);
u16		calculate_size_including_padding_data(u16 body_siz);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
