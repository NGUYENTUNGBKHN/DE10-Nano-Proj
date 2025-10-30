#ifndef _MAG_CHECK_H_
#define _MAG_CHECK_H_

/*
2021/12/2
デバッグ用にCSV出力できるように

2022/2/22
カウント数を0.8倍するように変更

2022/6/21
磁気過多をパラメタに変更

2022/7/15
変数名変更　不要部分削除
中間情報と結果ブロックを変更

*/

#define OUT_OF_RANGE1 40
#define OUT_OF_RANGE2 -40
#define STANDARD_LEVEL 100.0f
#define MAG_STANDARD_LEVEL 40.0f
#define MAG_AREA_PERCENTAGE 20.0f
//#define EXCESS_MAG_PERCENTAGE 50.0f//3.29f 2021/07/28 CNYエラー回避
#define MAG_WHITE 245
#define MAG_BLACK 10
#define CONSTANT_VALUE 5.0f // 磁気検知領域内で4%の磁気がある時にレベルの計算結果が40となる定数

#define MAG_MAX_STRUCT 10 //MAG構造体数
//MAG用の配列
typedef struct
{
	ST_IMUF_POITNS	*ppoint;		// 座標パラメータ配列
	
	u16 mag_mask_ptn_diameter_x;		// IRマスクパターンの直径
	u16 mag_mask_ptn_diameter_y;		// IRマスクパターンの直径
	float mag_mask_ptn_divide_num;	// IRマスクパターンの割る値
	u8* pmag_mask_ptn;				// IRマスクパターンのポインタ
	float average[MAG_MAX_STRUCT];	// 平均

	u8 point_number; // パラメータで指定した領域の数
	u8 result; // 結果
	u8 level; // レベル
	u8 padding;

	u8 ave_number;
	u8 per_number;
	float percent[MAG_MAX_STRUCT];

} ST_MAG;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s32 get_mag_valid_count(u8 buf_num, ST_MAG *st);
	s16 new_L2P_Coordinate_MagUv(ST_SPOINT * __restrict spt, ST_BS * __restrict pbs, ST_NOTE_PARAMETER * __restrict pnote_param);
	s16 P_Getdt_8bit_only_MagUv(ST_SPOINT *__restrict spt, ST_BS *__restrict pbs);
	s32 point_vicinity_cal_Mag(ST_POINT_VICINITY* __restrict pv, u8 buf_num, float std_para1, float std_para2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
