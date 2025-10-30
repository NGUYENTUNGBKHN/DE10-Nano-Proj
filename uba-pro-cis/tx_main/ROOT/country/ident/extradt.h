// extradt.h
#ifndef _EXTRADT_H
#define _EXTRADT_H

#define	PLANE_LST_MAX	16
#define PT_SIZE_MAX 10000//抽出データ配列の大きさ

#define ERR_EXCEEDS_THE_VALUE_OF_THE_SET_PT_SIZE_MAX			(0x080a)	/*設定したPT_SIZE_MAXの値を超えている*/
#define ERR_MASK_SIZE_IS_LARGE_AND_IT_REFERS_TO_OUTSIDE_OF_AREA (0x080b)	//マスクサイズが大きすぎて紙幣エリア外を参照している


// 抽出データ配列
typedef struct
{
		u8	outline;		// 0:外形基準  1:印刷枠基準
	u8	buf_num;		// バッファ番号
	u16	split_x;		// 横分割数
	u16	split_y;		// 縦分割数
	s16	count;			// ポイント数の合計値
	u8	corner_flg;	// コーナーのポイントを  0:使わない,  1:使う
	u8	plane_count;	// プレーンリストの有効数	
	u8  cord_trans_select; // 座標変換指定		
	u8  side_masking_area; // マスキングエリア(横
	u8  vertical_masking_area; // マスキングエリア(縦

#ifndef _RM_
////******pitch modeのパラメータ*******
	u16 point_count_x;	//ポイント数x　マイナスからカウントした全体 例:-10～10がポイントならば10がこの変数に入る
	u16 point_count_y;	//ポイント数y　
	u8 pitch_mode_out_put[100][100];	//ポイント抽出結果が格納される配列
#endif

	u8	plane_lst[PLANE_LST_MAX];	// プレーンリスト 
	u16 data_extra_out_put[PT_SIZE_MAX];	//ポイント抽出結果が格納される配列

	//物理をスキャンする際、dpiが異なるので、プレーンごとにマスクパターンとサイズを持つこととする。
	u16	filter_size_x[PLANE_LST_MAX];		// 横フィルターサイズ
	u16	filter_size_y[PLANE_LST_MAX];		// 縦フィルターサイズ
	u8	*pfilter_pat[PLANE_LST_MAX];		// フィルターパターンの先頭アドレス　
	float divide_val[PLANE_LST_MAX];		// フィルター領域内の合計値を割る値


} ST_DATA_EXTRACTION;

typedef struct
{
	u16	filter_size_x;		// 横フィルターサイズ
	u16	filter_size_y;		// 縦フィルターサイズ
	u8	*pfilter_pat;		// フィルターパターンの先頭アドレス　
	float	divide_val;		// フィルター領域内の合計値を割る値
	s16	x;		// 抽出ポイント横　中心基準での座標
	s16	y;		// 抽出ポイント縦　中心基準での座標
	u16 output;		// この抽出ポイント領域のデータ
	u8	plane;		// Plane_tblのセンサー番号
	u8  padding;

} ST_POINT_VICINITY;

typedef struct
{
	u8 plane[16];
	u8 y[100];
	u8 x[100];
	u8 data[100];

	u16 regist_count;

} ST_SOFT_CASH;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	s16 Data_extraction(ST_DATA_EXTRACTION *de ,u8 buf_num);
	//__inline s16 point_vicinity_cal(ST_POINT_VICINITY *__restrict pv ,u8 buf_num)

#ifdef I_AND_A
	s16 point_vicinity_cal(ST_POINT_VICINITY * pv ,u8 buf_num);
#else
	s16 point_vicinity_cal(ST_POINT_VICINITY* pv, u8 buf_num);
#endif

	s16 logi_point_vicinity_cal(ST_POINT_VICINITY *pv ,u8 buf_num);
	s16 data_extraction_pitch_mode(ST_DATA_EXTRACTION * de, ST_POINT_VICINITY * pv ,u8 buf_num);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _EXTRADT_H */
