#ifndef	_HOLE_INCLUDE_H
#define	_HOLE_INCLUDE_H

/****************************************************************************/
/**
 * @file hole.h
 * @brief 穴検知 ver2.5
 * @date 2020/8/18 Created.
 */
/****************************************************************************/

/*
	 2020/9/7

*/


//設定値
#define HOLE_ROI_BUFF 800
#define HOLE_ROI_LIST 200
#define HOLE_MAX_COUNT 8
#define EXCLUDE_COUNT 5
#define CONTOUR_SEARCH_MOVE_LIMIT 2000
#define MOVE_FOR_CONTOUR_MOVE_LIMIT 50
#define INI_MAX -4096//0xf000
#define INI_MIN 4095//0x0fff
#define HOLE_AREA_LIMIT 300

//エラーコード
#define ERR_CONTOUR_LOST 0x1814//6164
#define ERR_CONTOUR_LOOP 0x1815//6165
#define ERR_LIST_OVER 0x1817//6167

//検知結果
#define LEVEL_OVER 0x01
#define TOTAL_OVER 0x02
#define TOO_MANY_HOLE 0x03
#define TOTAL_LIMIT_OVER 0x04

//
#define NORMAL_HOLE 0
#define UNFIT_HOLE 1
#define	ERR_HOLE -1

#define OUTLINE_FLAG_HOLE 1
#define ONE_DOT_FLAG_HOLE 2
#define IN_HOLE 1
#define OUT_HOLE 0


typedef struct
{
	s16 exclude_point_x1;
	s16 exclude_point_x2;//x1＜x2
	s16 exclude_point_y1;
	s16 exclude_point_y2;//y1＜y2

} ST_EXCLUDE_HOLE;

typedef struct
{
	u8 t_plane;//使用する透過プレーン番号　透過緑
	u8 threshold;//閾値
	u8 skip;//輪郭検知探索間隔 dot
	u8 edge_margin;	//外形マージン mm
	float x_step;//主走査方向探索間隔 mm
	float y_step;//副走査方向探索間隔 mm
	
	u16 threshold_level_area;//穴と判定する面積基準 mm^2
	u16 threshold_total_area;//合計検出面積基準 mm^2
	u16 threshold_min_area;//最小検出面積基準 mm^2
	
	u8	exclude_area_count;	//除外範囲個数
	ST_EXCLUDE_HOLE exclude_hole[EXCLUDE_COUNT];//除外範囲座標の構造体
	
	//中間情報
	s16 hole_min_list[HOLE_ROI_LIST];//ROI用配列
	s16 hole_max_list[HOLE_ROI_LIST];//ROI用配列
	s16 hole_max_buff[HOLE_ROI_BUFF];//最大x座標記録用配列
	s16 hole_min_buff[HOLE_ROI_BUFF];//最小x座標記録用配列

	u8 result;//出力結果
	u8 hole_count;//穴個数
	u16 total_hole_area;//合計検出面積
	u16 max_hole_area;//検出最大面積
	u16 err_code;//エラーコード
	s16 holes[HOLE_MAX_COUNT];//各穴の面積
	u8 level;//レベル
	u8 padding[3];

} ST_HOLE;

typedef struct
{
	ST_HOLE* ho;

	ST_SPOINT t_spt;//透過プレーン
	ST_EXCLUDE_HOLE outline;//頂点座標
	ST_EXCLUDE_HOLE outline_margin;//輪郭探索時の裂け・角折れ判定開始位置用
	ST_EXCLUDE_HOLE exlude[EXCLUDE_COUNT];//除外範囲座標

	ST_BS* pbs;
	ST_NOTE_PARAMETER* para;
	
	float mmfordot;//mmからdotへ
	float mm2fordot;//mm2からdotへ

	s8 x_step;//x方向間隔
	s8 y_step;//y方向間隔
	u8 buf;
	u8 skip;//輪郭探索間隔

	s16 margin;
	s16 y_offset;//論理座標を配列に記録する

	u16 total_judge;//最大合計面積
	u16 min_area_dot;//最小検知面積

	u16 total_limit;
	u16 res_total_judge;//検知した合計面積

	u8 res_hole_count;//検知した穴の個数
	u8 padding;
	s16 roi_list_last;//記録したROI配列の最後の要素番号

	s16 y_max_list[HOLE_MAX_COUNT];//記録した穴ごとの最大y座標
	s16 y_min_list[HOLE_MAX_COUNT];//記録した穴ごとの最小y座標
	s16* x_min_list[HOLE_MAX_COUNT];//記録した穴ごとの最大x座標
	s16* x_max_list[HOLE_MAX_COUNT];//記録した穴ごとの最小x座標
	s16	list_num[HOLE_MAX_COUNT];//記録したROI配列の最初の要素番号

} ST_HOLE_SETTING;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 hole(u8 buf_num, ST_HOLE* ho);

	s8 hole_lsearch(ST_HOLE_SETTING* setting);
	s8 contour_setup(ST_HOLE_SETTING* setting, s16 lx, s16 ly);
	s8 contour_lsearch(ST_HOLE_SETTING* setting, s16 x_start, s16 y_start, s16* ly_max, s16* ly_min, s8 check_count);
	s8 move_for_contour(ST_HOLE_SETTING* setting, s16* lx, s16* ly);

	u8 hole_thershold(ST_HOLE_SETTING* setting, s16 lx, s16 ly, u8 thr);
	s8 check_outline(ST_HOLE_SETTING* setting, s16 lx, s16 ly, s8 vnum);
	s8 check_exlude(ST_EXCLUDE_HOLE* exclude, s16 lx, s16 ly, u8 num);
	s8 check_area(ST_EXCLUDE_HOLE* rectangle, s16 lx, s16 ly);
	s16 set_max_remainder_0(s16 value, s8 step);
	s16 set_min_remainder_0(s16 value, s8 step);
	void initialize_hole_roi_buff(s16 from, s16 to, ST_HOLE* ho);
	void set_hole_lvertex(ST_HOLE_SETTING* setting, ST_EXCLUDE_HOLE* outline, ST_EXCLUDE_HOLE* outline_margin);

	u8 hole_level_detect(ST_HOLE* ho, ST_HOLE_SETTING* setting);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*  */

