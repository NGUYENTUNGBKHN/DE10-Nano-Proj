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
 * @file stain.h
 * @brief　染み検知
 * @date 2019 Created.
 */
/****************************************************************************/

#ifndef	_STAIN_H
#define	_STAIN_H

#define ERR_ARRAY_OVER		0x0101	// ポイント数が限界
#define ERR_WHILE_OVER		0x0102	// while分がリミット値を超える回数実行された。


#define RES_THRSHOLD_OVER	0x0001	// 閾値以上
#define RES_TOTALAREA_OVER	0x0002	// トータル面積

#define STAIN_MAX_PALNE_NUM	2			// 最大プレーン数
#define STAIN_MAX_NOT_SCAN_AREA_NUM	100	// 最大除外エリア数
#define STAIN_MAX_NOT_SCAN_AREA_DETECT_NUM	300	// 最大除外エリアのスキャン数

#define STAIN_LEVEL_1 -1
#define STAIN_LEVEL_2 -2
#define STAIN_LEVEL_3 -3

typedef struct
{
	s16		left_up_x;				//頂点左上ｘ
	s16		left_up_y;				//頂点左上ｙ
	s16		right_down_x;			//頂点右下ｘ
	s16		right_down_y;			//頂点右下ｙ

} ST_STAIN_NOT_SCAN_AREA;

//染みスキャン時の進行方向の名前
enum stain_scan_direction_of_travel
{
	stain_right_down = 0,	//右下
	stain_down,	        //下
	stain_left_down,	    //左下
	stain_left,           //左
	stain_left_up,        //左上
	stain_up,             //上
	stain_right_up,       //右上
	stain_right           //右
};

typedef struct
{
	//入力
	u8 plane[STAIN_MAX_PALNE_NUM];					//プレーン
	u8 plane_num;					//プレーン数
	u8 comparison_method;			//比較方法 0面積 １面積と直径 2直径

	float divide_val;				//割る数
	u16 point_num;					//ポイント数
	u16	mask_size_x;				//マスクサイズｘ

	u16	mask_size_y;				//ｙ
	u16 padding;

	u8*	p_mask;						//マスクポインタ
	u8*	thr[2];						//ポイントマップ
	float stain_size_thr;			//1つの染みの面積の閾値 単位 mm^2
	float total_stain_thr;			//染みの合計面積の閾値 単位 mm^2
	float stain_diameter_thr;		//染みの直径の閾値 単位 mm
	u16 min_thr_val;				//閾値の最低限値　これ以下の数値の閾値のポイントは処理対象としない。
	u8 conter_not_scan_range;		//外周からN㎜はスキャン対象から外す
	u8 repeat_num;					//染みスキャンに失敗したとき閾値を変更してやり直す回数

	u8 x_split;						//分割数ｘ
	u8 y_split;						//ｙ
	s16 note_size_x;

	s16 note_size_y;
	s8 thr_adjust_val;				//閾値調節用
	u8 padding2;

	//中間情報
	u16 note_size_x_one_third;				//紙幣サイズの1/3　スキャン回数の閾値などに用いる
	u16 note_size_y_one_third;
	u16 not_scan_margin_x;					//
	u16 not_scan_margin_y;					//
	float up_and_down_edge_slope;			//上下辺の傾き
	float up_edge_offset;					//上辺の切片
	float down_edge_offset;					//下辺の切片
	float left_and_right_edge_slope;		//左右辺の傾き
	float left_edge_offset;					//左辺の切片
	float right_edge_offset;				//右辺の切片
	u8 raito;								//200dpiとの比率
	u8 coef_raito;							//dpi毎の係数
	u8 not_scan_area_count[STAIN_MAX_PALNE_NUM];		//除外エリアの数　
	u8 not_scan_safe_area_count[STAIN_MAX_PALNE_NUM][STAIN_MAX_NOT_SCAN_AREA_NUM];	//除外エリアの中で有効なエリアの数
	ST_STAIN_NOT_SCAN_AREA notscan2[STAIN_MAX_PALNE_NUM][STAIN_MAX_NOT_SCAN_AREA_NUM];			//除外エリアの座標
	ST_STAIN_NOT_SCAN_AREA notscan_in_vali[STAIN_MAX_PALNE_NUM][STAIN_MAX_NOT_SCAN_AREA_NUM][5];	//除外エリア内の有効エリアの座標

	u16 half_size_x;						//
	u16 half_size_y;						//

	u16 main_scan_max_val;
	u16 main_scan_min_val;
	u16 sub_scan_max_val;
	u16 sub_scan_min_val;

	u8  security_mode;	//波長よって決定する　1：ON（IR1,IR2）　0:OFF(R,G,B)

	//外形からの不使用エリア座標
	s16	not_scan_range_x_left_up;
	s16	not_scan_range_y_left_up;
	s16	not_scan_range_x_right_down;
	s16	not_scan_range_y_right_down;

	//出力
	u8 res_judge;					//検知結果
	u8	level;
	u16 res_stain_err;				//エラーコード
	float res_max_stain_area;		//検知した染みで最も大きい染みの面積
	float res_max_stain_diameter;	//検知した染みで最も大きい染みの直径
	float res_total_stain_area;		//検知した染みの合計面積


} ST_STAIN;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	u8 check_area_(ST_STAIN* sta, ST_SPOINT* spt);
	s16	stain(u8 buf_num ,ST_STAIN *st);
	s16 contor_detect(ST_STAIN *sta ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x, s16 y,s16 *conter_x ,s16 *conter_y ,u8 thr,u16 *conter_point_counter, u8 plane_num ,float multi ,s32 *center_x ,s32 *center_y , u16* max_x ,u16* max_y ,u16* min_x ,u16* min_y);
	s8 pix_scan_module(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs , u16 *origin_x ,u16 *origin_y ,u8 thr, ST_STAIN* sta);
	float stain_area(s16 *x , s16 *y , u16 point_count ,s16 origin_x ,s16 origin_y);
	s16 hole_check(s16 x, s16 y ,u8 buf_num ,u8 plane, ST_STAIN* sta);
	void stain_ini_and_params_set(ST_STAIN *st, ST_BS *pbs , u8 palne);
	u8 note_contor_scan(ST_STAIN *st ,ST_BS *pbs, u16 *x, u16 *y ,u8 location, u16 thr, u8 plane ,u16 finish_x ,s16 *conter_x ,s16 *conter_y ,u16 *conter_point_counter ,u16 *conter_point_counter2);
	s8 not_scan_area_dision(ST_BS *pbs ,ST_STAIN *st,ST_NOTE_PARAMETER* pnote_param, s16 x[], s16 y[]);
	s8 thr_scan_module(u8 dir[] ,s32 *x ,s32 *y ,u8 thr[] , s16 x_split , s16 y_split, u8 tot);
	void stain_quick_sort( s16 list[], s16 left, s16 right );
	s8	not_scan_area_serch(ST_STAIN *sta ,s32 start_x, s32 start_y, s32 end_x, s32 end_y , u8 tot, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], u8 *area_nu);
	s8	validity_area_serch(ST_STAIN *sta ,s32 start_x, s32 start_y, s32 end_x, s32 end_y , u8 tot, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], u8 *area_num);

	s8	set_area_info(ST_BS *pbs, ST_STAIN *sta, ST_NOTE_PARAMETER* pnote_param, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2],s16 extra_point_x[], s16 extra_point_y[], u16 num);
	s8	set_safearea_info(ST_BS *pbs, ST_STAIN *sta, ST_NOTE_PARAMETER* pnote_param, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2],s16 extra_point_x[], s16 extra_point_y[], u16 num ,u8 area_c);

	u8	stain_level_detect(ST_STAIN *sta);
	float get_right_direction( float x, float y, float dx, float dy, float tx, float ty );
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _STAIN_H */
