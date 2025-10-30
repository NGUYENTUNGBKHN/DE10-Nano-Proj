#ifndef _CIR_3COLOR_CHECK_H_
#define _CIR_3COLOR_CHECK_H_


static enum P_Plane_tbl cir3_plane_tbl_ir[] = {UP_R_IR1, DOWN_R_IR1};

static enum P_Plane_tbl cir3_plane_tbl_red[] = {UP_R_R,   DOWN_R_R };

static enum P_Plane_tbl cir3_plane_tbl_grn[] = {UP_R_G,   DOWN_R_G };

/*
static enum P_Plane_tbl cir3_plane_tbl1[] = {UP_R_IR1, DOWN_R_IR1, UP_R_R,   DOWN_R_R,   UP_R_G,   DOWN_R_G };

static enum P_Plane_tbl cir3_plane_tbl2[] = {UP_R_R,   DOWN_R_R,   UP_R_G,   DOWN_R_G,   UP_R_IR1, DOWN_R_IR1 };

static enum P_Plane_tbl cir3_plane_tbl3[] = {UP_R_G,   DOWN_R_G,   UP_R_IR1, DOWN_R_IR1, UP_R_R,   DOWN_R_R };
*/

#define CIR_3COLOR_CHECK_COUNT 6		//モード数
#define CIR_3COLOR_CHECK_MAX_POINT 120	//最大ポイント数


enum CIR_3COLOR_CHECK
{
	CIR_3COLOR_IR_UP = 0,
	CIR_3COLOR_IR_DOWN,
	CIR_3COLOR_RED_UP,
	CIR_3COLOR_RED_DOWN,
	CIR_3COLOR_GREEN_UP,
	CIR_3COLOR_GREEN_DOWN
};


//3CIR用の配列
typedef struct
{
	s8 cal_mode[CIR_3COLOR_CHECK_COUNT];						//計算モード
	u8 blank[2];
	//ST_LIM_AND_POINTS	*ppoint[CIR_3COLOR_CHECK_COUNT][CIR_3COLOR_CHECK_MAX_POINT];	//各モードの配列
	ST_LIM_AND_POINTS	*ppoint;	//各モードの配列

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



} ST_3CIR;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_cir_3color_invalid_count(u8 buf_num ,ST_3CIR *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
