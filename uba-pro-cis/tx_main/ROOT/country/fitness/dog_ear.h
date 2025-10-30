/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file dog_ear.h
 * @brief 角折れ検知
 * @date 2018/06/29 Created.
 */
/****************************************************************************/

#ifndef	_DE_H
#define	_DE_H

#define DOG_EAR_LEARNING 1

//パラメタ
#define MAX_SEARCH_RANGE 80			//探索ポイント数
#define REV_MAX_NUM 10				//この回数頂点に戻ってきたらサーチ打ち切り
#define VERTEX_POINT_NUM 4			//頂点の数
#define DOG_EAR_TRI_VERTEX_NUM 2	//角折れ三角の頂点の数
#define SCAN_THR 250				//閾値:媒体と背景を区別
#define X_Y 4						//見ないエリアの座標数ｘｙ
#define MAX_NOT_SEEN_AREA_COUNT 5	//見ないエリアの最大個所数






//コード追加していく
#define RES_OUT_OF_AREA		        0x0010	//記録したエリアが搬送エリア外
#define RES_SLOPE_OP		        0x0020	//求めた傾きが逆　(破れと判定します。)
#define RES_COME_BACK_VERTER        0x0030	//頂点に戻ってきた回数が規定値を超えた。
#define RES_SLOPE_IS_ABNORMAL       0x0040	//斜辺が近似直線に沿っていない(破れと判断します。)
#define RES_HISTOGRAM_PEAKS_MATCH	0x0050	//ヒストグラムのピークが一致(破れと判断します。)
#define RES_POINT_COUNT_LESS		0x0060	//記録できた斜辺の座標が5ポイント以下(正券と判断する)
#define RES_HISTOGRAM_POINT_LESS	0x0070	//ヒストグラム処理の際見れないエリアを参照した(角折れ券と判断する)



enum dog_ear
	{
		upper_left,
		lower_left,
		upper_right,
		lower_right,
};

enum dog_ear_status
    {
		NORMAL = 0x00,	//角折れなし
		LITTLE_DOG_EAR,	//角折れはあるが基準をクリア
        DOG_EAR,		//角折れ
        TEAR,			//破れ


};

typedef struct
{
	//共通？
	u8 buf_num;						//処理中のバッファ番号を格納する
	u8 plane;						//用いるプレーン
	u8 jcm_id;

	//入力
	u8 comp_flg;					//どちらの基準で比較するか	1：両辺のながさ　2：面積＋短辺
	u8 scan_move_x;					//ｘ座標が戻る量
	u8 scan_move_x_thr_line;		//頂点＋この数字で閾値
	u8 scan_minimum_points;			//このポイント以下ならば角折れなし
	s8 tear_scan_renge;				//破れ検知の際、どれだけ探索すれば破れと判断するか
	u8 garbage_or_note_range;		//ポイント採取の際ぶつかったものが媒体かごみかどうかを調べる範囲
	u8 scan_move_y;					//頂点を求めるときのy座標の移動量
	u8 tear_scan_start_x;			//裂け検知の際、斜辺を何分割するか
	u8 not_seen_area_count;			//見ないエリアの数
	s16 not_seen_areas[MAX_NOT_SEEN_AREA_COUNT][X_Y];			//見れないエリアの範囲

	u16 tear_scan_point_decide;		//対称三角形のスキャンエリアを決定する　4より大きな数字にすること

	u8 qp;								//ヒストグラム量子化パラメタ
	u8 peak_width;						//ヒストグラム：ピーク階層比較幅
	u8 peak_margin;					//第2ピークの閾値
	u8 padding[3];					//第2ピークの閾値


	float ele_pitch_x;				//主走査素子ピッチ
	float ele_pitch_y;				//副走査素子ピッチ
	u16 main_effective_range_max;	//搬送エリアの範囲
	u16 main_effective_range_min;	//搬送エリアの範囲

	float threshold_short_side_dot;		//短手の基準　単位dot
	float threshold_long_side_dot;		//長手の基準　単位dot
	float threshold_short_side_mm;		//短手の基準　単位mm
	float threshold_long_side_mm;		//長手の基準　単位mm
	u32 threshold_area;					//面積の基準

	//出力
	u32	area[VERTEX_POINT_NUM];			//角折れの面積
	u32	area_mm[VERTEX_POINT_NUM];		//角折れの面積 単位mm
	float len_x[VERTEX_POINT_NUM];		//x辺の長さ
	float len_y[VERTEX_POINT_NUM];		//y辺の長さ
	float len_x_mm[VERTEX_POINT_NUM];	//辺の長さ　単位mm
	float len_y_mm[VERTEX_POINT_NUM];	//辺の長さ　単位mm
	float short_side[VERTEX_POINT_NUM];	//角折れの長手
	float long_side[VERTEX_POINT_NUM];	//角折れの短手
	float short_side_mm[VERTEX_POINT_NUM];//角折れの長手 単位mm
	float long_side_mm[VERTEX_POINT_NUM];	//角折れの短手 単位mm

	s16 triangle_vertex_1[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//角折れ三角形の折れ初めの座標を記録する
																		//基準頂点からｘが変化している頂点から記録する
	s16 triangle_vertex_2[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//角折れ三角形の折れ初めの座標を記録する

	
	u8	judge[VERTEX_POINT_NUM];			//各頂点の判定結果
	u16 judge_reason[VERTEX_POINT_NUM];		//結果の理由
	float judge_pix[VERTEX_POINT_NUM];		//検証用
	u8	level;
	u8	padding1[3];


} ST_DOG_EAR;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	dog_ear(u8 buf_num ,ST_DOG_EAR *dog);
	u8 left_upper(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs ,ST_DOG_EAR *dog );
	u8 left_lower(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs  ,ST_DOG_EAR *dog);
	u8 right_upper(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs  ,ST_DOG_EAR *dog);
	u8 right_lower(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs  ,ST_DOG_EAR *dog);
	s16 cal_symmetric_center_point(s16 pver_a_x, s16 pver_a_y, s16 pver_b_x, s16 pver_b_y, s16 pver_c_x, s16 pver_ac_y, float slope ,float offset, ST_BS *pbs, u8 location ,ST_DOG_EAR *dog);
	void unit_convert(float ele_pitch_x, float ele_pitch_y ,ST_DOG_EAR *dog);
	s16 judge_dog_ear(float shot_side, float long_side ,u32 *area ,ST_DOG_EAR *dog);
	s16 triangle_inspection(ST_BS *pbs , float tan, float offset ,s16 input_a_x ,s16 input_b_x  ,s8 increment_dir_x ,ST_DOG_EAR *dog );
	void approx_line(s16 x[],s16 y[], s32 n, float *a0, float *a1);
	void ini_dog(ST_DOG_EAR *dog);

	s16	set_vertex(s16 *x ,s16 *y ,ST_SPOINT *spt ,ST_BS *pbs ,u16 vertex ,ST_NOTE_PARAMETER* pnote_param ,ST_DOG_EAR *dog);

	s16 judge_dogear_tear(ST_BS *pbs ,u8 plane ,double sym_trai_cen_x ,double sym_trai_cen_y ,s16 ver_b_x ,s16 ver_c_y
		,s8 increment_y ,s8 increment_x ,double sym_slope1 ,double sym_y_ofs1 ,double sym_slope2 ,double sym_y_ofs2
		,s16 scan_out_a ,s16 scan_out_b ,s16 scan_x_range , s16 scan_x_start ,s16 scan_y_start, s16 scan_y_range ,u8 location ,ST_DOG_EAR *dog);

	s16 dog_ear_level_indicator(ST_DOG_EAR *dog);

	s32 area_calculation(float *x , float *y , u16 point_count ,ST_DOG_EAR *dog);

	u8	dog_level_detect(ST_DOG_EAR *dog);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DE_H */
