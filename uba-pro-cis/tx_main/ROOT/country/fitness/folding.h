#ifndef _FOLDING_CHECK_H_
#define _FOLDING_CHECK_H_

static enum P_Plane_tbl tir1_plane_tbl_tir1[] = { UP_T_IR1, DOWN_T_IR1 };

#define HOLOGRAM_THRESHOLD 210 // 背景等の明るすぎる箇所を無視するためのしきい値
#define WINDOW_THRESHOLD 33 // 暗すぎるところを無視するためのしきい値 ホログラムなど（名前が逆）
#define EDGE_NUMBER 4 // エッジ（端）の数
#define FOLDED_LEVEL 2// 折畳レベル計算用の定数
#define DECREASE_THRESH 0.6f // 折畳レベル計算用の定数
#define CONSTANT_FOR_FOLDING 5 // 折畳レベル計算用の定数　各国共通のため、受取拒否等の設定はレベル設定で対応すること
#define CONSTANT_FOR_MUTILATION 10 // 欠損レベル計算用の定数　各国共通のため、受取拒否等の設定はレベル設定で対応すること
#define FOLDING_LEVEL 60 // 折畳の標準レベル
#define MUTILATION_LEVEL 80 // 欠損の標準レベル
//折り畳み検知用の配列
typedef struct
{
	ST_IMUF_POITNS *ppoint;			// 座標パラメータ配列
	u16 tir1_mask_ptn_diameter_x;	// T_IRマスクパターンの直径
	u16 tir1_mask_ptn_diameter_y;	// T_IRマスクパターンの直径
	float tir1_mask_ptn_divide_num;	// T_IRマスクパターンの割る値
	u8* ptir1_mask_ptn;				// T_IRマスクパターンのポインタ
	u8 folded[EDGE_NUMBER];
	u8 folded_level[EDGE_NUMBER];
	u8 err_code;					// エラーコード ０：上、１：下、２：左、３：右
	u8 padding[3];

	float tir_diff[EDGE_NUMBER];

}ST_FOLDING;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	u8 size_level(u16 size);
	s32 get_folding_invalid_count(u8 buf_num, ST_FOLDING *st, u16 x_sz, u16 y_sz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
