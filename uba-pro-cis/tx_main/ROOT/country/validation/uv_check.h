#ifndef _UV_CHECK_H_
#define _UV_CHECK_H_

/*
2021/12/2
デバッグ用にCSV出力できるように

2022/6/7
UV_Fitnessレベル出力方法変更

2022/7/15
中間情報と結果ブロック変更

*/


#define UV_STANDARD_LEVEL 40 // レベル計算用①
#define UV_MULTI_LEVEL 10 // レベル計算用②
#define UV_DIFFERENCE -2
#define OUT_OF_RANGE -30
static enum P_Plane_tbl uv_plane_tbl[] = { UP_R_FL, DOWN_R_FL };

//UV用の配列
// ここは全体・一部検知の両方がST_UV_VALIDATE配列に変わる。ST_UVの方は変更前なので残しておく
typedef struct
{
	ST_UV_POINTS	*ppoint;		// 座標パラメータ配列
	
	u16 uv_mask_ptn_diameter_x;		// UVマスクパターンの直径（固定）
	u16 uv_mask_ptn_diameter_y;		// UVマスクパターンの直径（固定）
	float uv_mask_ptn_divide_num;	// UVマスクパターンの割る値（固定）
	u8* puv_mask_ptn;				// UVマスクパターンのポインタ（固定）

	float average[IMUF_POINT_NUMBER];	// 平均

	u8 point_number; // パラメータで指定した領域の数
	u8 result; // 結果
	u8 level; // レベル
	u8 padding;
} ST_UV_VALIDATE;

//つかってない
//typedef struct
//{
//	ST_IMUF_POITNS	*ppoint;		// 座標パラメータ配列
//	
//	u16 uv_mask_ptn_diameter_x;		// UVマスクパターンの直径（固定）
//	u16 uv_mask_ptn_diameter_y;		// UVマスクパターンの直径（固定）
//	float uv_mask_ptn_divide_num;	// UVマスクパターンの割る値（固定）
//	u8* puv_mask_ptn;				// UVマスクパターンのポインタ（固定）
//
//	float average[IMUF_POINT_NUMBER];	// 平均
//
//	u8 point_number; // パラメータで指定した領域の数
//	u8 result; // 結果
//	u8 level; // レベル
//	u8 padding;
//} ST_UV;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	//s32 get_uv_invalid_count(u8 buf_num, ST_UV *st); // 旧関数
	s32 get_uv_fitness(u8 buf_num, ST_UV_VALIDATE* uv); // 蛍光検知（全体）＝正損
	s32 get_uv_validate(u8 buf_num, ST_UV_VALIDATE* uv);// 蛍光検知（一部）＝鑑別
	u8 calc_uv_level(float level1, float level2, u8 point_number, ST_UV_VALIDATE* uv, u16 flag); // 蛍光検知のレベル計算
	u8 calc_uv_fitness_level(float level1, float level2, u8 point_number, ST_UV_VALIDATE* uv, u16 flag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
