#ifndef _IR2WAVE_CHECK_H_
#define _IR2WAVE_CHECK_H_


/*
2021/12/2
IR2なしで1番目のみ判定していたのを修正
デバッグ用にCSV出力できるように

2022/07/14
IR2波長差から変更
*/

//IR2wave用の配列
typedef struct
{
	ST_IMUF_POITNS	*ppoint;		// 座標パラメータ配列

	u16 ir1_mask_ptn_diameter_x;	// IRマスクパターンの直径（IR1/2共通）
	u16 ir1_mask_ptn_diameter_y;	// IRマスクパターンの直径（IR1/2共通）
	float ir1_mask_ptn_divide_num;	// IRマスクパターンの割る値（IR1/2共通）
	u8* pir1_mask_ptn;				// IRマスクパターンのポインタ（IR1/2共通）
	
	u8 point_number;

	//
	u8 ir1peak[IMUF_POINT_NUMBER];
	u8 ir2peak[IMUF_POINT_NUMBER];
	u32 ir1freq[IMUF_POINT_NUMBER];
	u32 ir2freq[IMUF_POINT_NUMBER];
	float ir1pro[IMUF_POINT_NUMBER];
	float ir2pro[IMUF_POINT_NUMBER];
	u8 feature[IMUF_POINT_NUMBER];

	//output
	u8 number;
	u8 output[IMUF_POINT_NUMBER];

	u8 err_code;
	u8 result;
	u8 level;

} ST_IR2WAVE;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s32 get_ir2wave_invalid_count(u8 buf_num, ST_IR2WAVE *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
