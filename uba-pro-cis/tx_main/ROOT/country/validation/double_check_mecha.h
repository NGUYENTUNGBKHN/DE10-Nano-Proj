#ifndef _DOUBLE_CHECK_MECHA_H_
#define _DOUBLE_CHECK_MECHA_H_

#ifndef THHKNS_CH_LENGTH
#define THHKNS_CH_LENGTH (256)
#endif

// 重券検知（メカ厚）
typedef struct
{
	// 入力
	u16 double_check_threshold;		// 重券検出厚み閾値
	u16 bill_check_threshold;		// 紙幣検出厚み閾値
	u8  double_area_ratio;			// 面積比率閾値(%)
	u8  exclude_length;				// 先頭判定除外範囲(mm)

	// 出力
	// 結果ブロック
	u8  result;					// 判定結果
	u8  double_check_ratio;		// 重券面積比率
	u16 double_check_count;		// 重券積分値
	u16 bill_check_count;								// 紙幣判定積分値
	u8  padding[2];

	// 中間情報
	u16 bill_thickness_average;							// 紙幣判定箇所の厚み平均値
	u8  double_check_point[THICKNESS_SENSOR_MAX_NUM];	// 重券判定箇所数　（各圧検センサ毎　最大16センサ分）
	u8  bill_check_point[THICKNESS_SENSOR_MAX_NUM];		// 紙幣判定箇所数　（各圧検センサ毎　最大16センサ分）
	u8  bill_top_point[THICKNESS_SENSOR_MAX_NUM];		// 紙幣先端判定箇所　（各圧検センサ毎　最大16センサ分）
	u8  exclude_point;									// 先頭判定除外範囲(point)
	u8  sensor_num;										// 厚検センサCH数
	u8  check_point_top;								// 検知対象範囲先端
	u8  check_point_end;								// 検知対象範囲後端

} ST_DBL_CHK_MECHA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	s16	double_check_mecha(u8 buf_num, ST_DBL_CHK_MECHA *st);
	u8 get_double_check_thkns_data(ST_BS * pbs, u16 thkns_ch_data[][THHKNS_CH_LENGTH], u16 height, u16 sensor_count);
	
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif 
