#ifndef _MCIR_CHECK_H_
#define _MCIR_CHECK_H_

static enum P_Plane_tbl mcir_plane_tbl_ir1[] = {UP_R_IR1, DOWN_R_IR1};
static enum P_Plane_tbl mcir_plane_tbl_red[] = {UP_R_R,   DOWN_R_R};
static enum P_Plane_tbl mcir_plane_tbl_grn[] = {UP_R_G,   DOWN_R_G};

/*
static enum P_Plane_tbl mcir_plane_tbl1[] = {UP_R_IR1, DOWN_R_IR1,UP_R_IR1, DOWN_R_IR1, UP_R_G,   DOWN_R_G };
static enum P_Plane_tbl mcir_plane_tbl2[] = {UP_R_R,   DOWN_R_R,  UP_R_G,   DOWN_R_G,   UP_R_R,   DOWN_R_R };
static enum P_Plane_tbl mcir_plane_tbl3[] = {UP_R_G,   DOWN_R_G,  UP_R_R,   DOWN_R_R,   UP_R_IR1, DOWN_R_IR1, };

*/

#define MCIR_RATIO_MULTIPLIER	 1000
#define MCIR_MAX_POINT			 120	//最大ポイント数
#define MCIR_MAX_MODE			 6	//モード数
#define MCIR_CONSTANT			13 // レベル計算用の定数 = invalid_cntが3のときにレベルが60程度になるようにするためのもの
#define MCIR_STANDARD_LEVEL 100 // 最大レベル（ここから減点していく）

enum MCIR_CHECK
{
	IR_MINUS_R_UP = 0,
	IR_MINUS_R_DOWN,
	IR_MINUS_G_UP,
	IR_MINUS_G_DOWN,
	G_MINUS_R_UP,
	G_MINUS_R_DOWN,
};

//MCIR用の配列
typedef struct
{
	s8 cal_mode[MCIR_MAX_MODE];			//計算モード
	u8 padding[2];
	ST_LIM_AND_POINTS *ppoint;					//各モードの配列

	u16 red_mask_ptn_diameter_x;			// 赤色マスクパターンの直径x
	u16 red_mask_ptn_diameter_y;			// 赤色マスクパターンの直径y
	float red_mask_ptn_divide_num;			// 赤色マスクパターンの割る数
	u8*	pred_mask_ptn;						// 赤色マスクパターン1のポインタ

	u16 grn_mask_ptn_diameter_x;			// 緑色マスクパターンの直径x
	u16 grn_mask_ptn_diameter_y;			// 緑色マスクパターンの直径y
	float grn_mask_ptn_divide_num;			// 緑色マスクパターンの割る数
	u8*	pgrn_mask_ptn;						// 緑色マスクパターン1のポインタ

	u16 ir1_mask_ptn_diameter_x;			// IR1色マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1色マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1色マスクパターンの割る数
	u8*	pir1_mask_ptn;						// IR1色マスクパターン1のポインタ
	u8 level; // レベル
	u8 padding2[3];

} ST_MCIR;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	int get_mcir_invalid_count(u8 buf_num, ST_MCIR *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
