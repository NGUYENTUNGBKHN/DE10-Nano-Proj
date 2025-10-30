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
 * @file tear.h
 * @brief 裂け検知
 * @date 2019/06/23 Created.
 */
/****************************************************************************/

#ifndef	_TEAR_H
#define	_TEAR_H

//パラメタ
#define LIMIT_TEAR_COUNT		20		// 結果を格納する配列の要素数
#define _1mm					8		// マージン値
#define LIMIT_CLIMB				5		// 局所最小検知の際上へ進める回数
#define MAX_WHILE				10000	// スキャンループの最大回数
#define NOT_SERCH_VERTEX_AROUND	39		// 頂点座標の周囲5㎜を検知対象外とする　dotで指定 

//each
#define RES_WIDTH_BELOW_STANDARD				0x01	//幅が閾値以下
#define RES_DEPTH_BELOW_STANDARD				0x02	//深さが基閾値下
#define RES_DEPTH_AND_WIDTH_PASS				0x03	//閾値を満たす 閾値以上の角裂けもこれに含まれる
#define RES_DEPTH_PASS_SCANING					0x04	//スキャン中に閾値を超えた
#define RES_OUT_OF_AREA_DEPTH_BELOW_STANDARD	0x05	//スキャン中に幅方向で紙幣エリアから出たが基準を満たしいない。	
														//小さい角裂けか角折れ区別不可能
#define RES_OUT_OF_AREA_WIDTH_AND_DEPTH_PASS	0x07	//スキャン中に幅方向で紙幣エリアから出たが基準を満たしている。
														//角裂け角折れの区別可能　サイズが大きい場合角折れ検知で区別しているから
#define RES_FALSE_DETECT_OF_DOG_EAR				0x06	//角折れ部分を誤ってスキャンした								
#define RES_SCAN_OUT_OF_AREA_WIDTH				0x08	//スキャン中に幅方向で紙幣エリアから出た
#define RES_SCAN_OUT_OF_AREA_DEPTH				0x09	//スキャン中に縦方向で紙幣エリアから出た
#define RES_SCAN_NOT_REFERENCE_AREA				0x0A	//スキャン中に除外エリアをスキャンした。


//fitness
#define	RES_TOTAL_DEPTH				0x01	//合計の裂けが閾値以上
#define	RES_SPECIFIC_PART			0x02	//特定の部分への裂けが閾値以上
#define	RES_TEAR_PENETRATION		0x03	//紙幣が貫通している　意地悪券の疑惑がある
//#define RES_DEPTH_AND_WIDTH_PASS	0x03	//閾値を満たす

#define	ERR_MANY_DETECTION_TEARS	0x01	//多くの裂けが見つかった
#define ERR_MAX_WHILE_OVER			0x02	//whileの最大回数を超えた


enum tear_type	//形状
{
		vertical,		//垂直
		horizontal,		//水平
		diagonal,		//斜め
		PENETRATION,
		not_required,	//不明だがこれが書き込まれているならば閾値以上となる
		
};

enum tear_side	//辺
{
		upper_side,	//上辺
		lower_side,	//下辺
		left_side,	//左辺
		right_side,	//右辺
};


enum corner_tear
{
	not_corner,
	upper_left_tr,
	lower_left_tr,
	upper_right_tr,
	lower_right_tr,
};




typedef struct
{


	//入力情報	関数外で設定するパラメタ　（template）
	float threshold_width;				//検知すべき裂けの幅　		　単位㎜で指定する。
	float threshold_vertical_depth;		//検知すべき裂けの深さ(垂直)　単位㎜で指定する。
	float threshold_horizontal_depth;	//検知すべき裂けの深さ(水平)　単位㎜で指定する。
	float threshold_diagonal_depth;		//検知すべき裂けの深さ(斜め)　単位㎜で指定する。
	//float	threshold_total_depth;		//検知した裂けの深さの合計値の閾値　単位㎜で指定する。
	u8 width_detection;					//幅検知を実行するか否か　１：実行　０：実行しない
	u8 total_depth_judge;				//深さの合計値で判定する　１：実行　０：実行しない
	u8 plane;							//用いるプレーン

	//中間情報	
	u8 buf_num;							//処理中のバッファ番号を格納する
	s16 corner_triangle_vertex_1[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//角折れ三角形の折れ初めの座標を記録する
																			//基準頂点からｘが変化している頂点から記録する
	s16 corner_triangle_vertex_2[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//角折れ三角形の折れ初めの座標を記録する

	s16 threshold_width_dot;			//検知すべき裂けの幅　単位を㎜からdotに変換したもの
	s16 threshold_vertical_depth_dot;	//検知すべき裂けの深さ(垂直)　単位を㎜からdotに変換したもの
	s16 threshold_horizontal_depth_dot;	//検知すべき裂けの深さ(水平)　単位を㎜からdotに変換したもの
	s16 threshold_diagonal_depth_dot;	//検知すべき裂けの深さ(斜め)　単位を㎜からdotに変換したもの
	

	s16	tear_width_coordinate_start;	//裂けはじめのx座標を記録する。上辺下辺：右側　側面：上側を記録する
	s16	tear_width_coordinate_end;		//裂けおわりのx座標を記録する。上辺下辺：左側　側面：下側を記録する

	s16 edge_coordinates_detected[VERTEX_POINT_NUM];	//実際に検知した四隅のエッジｙ座標を記録する

	u8	temporary_depth_thr;			//輪郭スキャンの際に用いる仮裂決定の閾値
	u8	temporary_depth_thr_diff_x;		//輪郭スキャンの際に用いる仮裂決定の閾値 １つ前との差分
	u8	temporary_depth_thr_diff_y;		//輪郭スキャンの際に用いる仮裂決定の閾値 １つ前との差分
	u8 search_pitch;					//輪郭部分のスキャンの際のスキャンピッチ　
	u8  width_search_num_x;				//幅検知の際に下げる量
	u8  width_search_num_y;				//幅検知の際に下げる量
	u8 penetration;						//０：通常券　１：縦貫通券　２：横貫通券
	u8 padding2;

	s16 note_size_x;
	s16 note_size_y;

	s16 a_third_note_size_x;
	s16 a_third_note_size_y;

	u8 dog_ear_res[4];					//角折れ検知の結果を格納する

	s16 not_seen_areas[MAX_NOT_SEEN_AREA_COUNT][X_Y];	//見れないエリアの範囲
	u8	not_seen_area_count;							//見れないエリアの数
	u8 padding3[3];


	//出力情報
	float res_tear_width[LIMIT_TEAR_COUNT];			//検知した裂けの幅
	float res_tear_depth[LIMIT_TEAR_COUNT];			//検知した裂けの深さ
	float res_tear_depth_total;						//検知した裂けの深さの合計値
	u8 res_tear_type[LIMIT_TEAR_COUNT];				//検知した裂けの形状
	u8 res_corner_tear_type[LIMIT_TEAR_COUNT];		//検知した角裂けの場所
	u16 res_each_judge_reason[LIMIT_TEAR_COUNT];	//検知した裂けの理由　
	u8 res_tear_count;								//検知した裂けの数
	u8 res_judge_tear;								//検知した裂けの中で閾値を超えるものがあった。　1：有　0：無
	u16 res_judge_reason;							//検知した裂けの理由　
	u16 res_err_code;								//エラーコード
	u8	level;
	u8 padding4;


	//float	res_total_depth;						//検知した裂けの深さの合計値　単位㎜で指定する。




} ST_TEAR;

//各頂点の５㎜ｘ５㎜の座標を計算し記録する
typedef struct
{
	s16 area1_left_up_x;
	s16 area1_left_up_y;

	s16 area1_right_down_x;
	s16 area1_right_down_y;

	s16 area2_left_up_x;
	s16 area2_left_up_y;
			
	s16 area2_right_down_x;
	s16 area2_right_down_y;

} ST_NOT_SECURITY_AREA;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	tear(u8 buf_num, ST_TEAR *tr);
	void ini_tear_param(ST_TEAR *tr);

	void get_dog_ear_res( ST_TEAR *tr );

	void search_tear_contour(ST_VERTEX pver , ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param, ST_TEAR *tr);

	u8 search_tear_depth_upper_and_lower(ST_SPOINT spt ,ST_VERTEX pver ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y ,s16 vartex_y ,s8 location  , s16 limit_min ,s16 limit_max, ST_TEAR *tr );
	u8 search_tear_width_upper_and_lower(ST_SPOINT spt ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y , s16 limit_min ,s16 limit_max, s8 increment ,u8* corner_flg, ST_TEAR *tr);
	void depth_scan_module_upper_and_lower(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y, u8 res[] ,s8 inc_num);

	u8 search_tear_depth_left_and_right(ST_SPOINT spt ,ST_VERTEX pver ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y ,s16 vartex_x ,s8 location  , s16 limit_min ,s16 limit_max, ST_TEAR *tr);
	u8 search_tear_width_left_and_right(ST_SPOINT spt ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y , s16 limit_min ,s16 limit_max, s8 increment ,u8* corner_flg , ST_TEAR *tr);
	void depth_scan_module_left_and_right(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y, u8 res[] ,s8 inc_num);

	u8	tear_total_depth(ST_TEAR *tr);

	//u8  get_temporary_depth(ST_BS *pbs);
	//u8  get_width_search_num(ST_BS *pbs);
	//s16 get_search_pitch(void);

	void get_cof(ST_BS *pbs, ST_TEAR *tr);

	void matching_tear_res_and_dog_ear_res(s16 x, s16, ST_TEAR *tr);

	u8	tear_level_detect(ST_TEAR *tr);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TEAR_H */
