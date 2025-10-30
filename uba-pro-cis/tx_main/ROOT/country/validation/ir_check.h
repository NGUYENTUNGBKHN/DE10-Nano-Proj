#ifndef _IR_CHECK_H_
#define _IR_CHECK_H_

static enum P_Plane_tbl ir_plane_tbl[] = {UP_R_IR1, DOWN_R_IR1};

#define IR_CHECK_COUNT 2

#define IR_BASE_COUNT 4
#define IR_CHECK_MAX_POINT 120	//最大ポイント数

enum IR_CHECK
{
	IR_CHECK_UP = 0,
	IR_CHECK_DOWN,
};

//IRチェック用配列
typedef struct
{	
	s8  base_point_x1;	//ベースデータ作成用
	s8  base_point_y1;	//ベースデータ作成用
	s8  base_point_x2;	//ベースデータ作成用
	s8  base_point_y2;	//ベースデータ作成用
	s8  base_point_x3;	//ベースデータ作成用
	s8  base_point_y3;	//ベースデータ作成用
	s8  base_point_x4;	//ベースデータ作成用
	s8  base_point_y4;	//ベースデータ作成用
	
} ST_IR_MODES;

typedef struct
{

	s8 cal_mode[IR_CHECK_COUNT];            //計算モード
	u8 blank[2];
	ST_IR_MODES	*pmode;						//ベースデータのパラメタ
	ST_LIM_AND_POINTS*	ppoint;				//ポイントデータのパラメタ

	u16 ir1_mask_ptn_diameter_x;			// IR1色マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1色マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1色マスクパターンの割る数
	u8*	pir1_mask_ptn;						// IR1色マスクパターン1のポインタ
	
} ST_IR_CHECK;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_ir_check_invalid_count(u8 buf_num ,ST_IR_CHECK *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
