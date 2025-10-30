#ifndef _DOUBLE_CHECK_H_
#define _DOUBLE_CHECK_H_

#define DOUBLE_CHECK_THERSHOLD 10	//閾値 重券

//判定コード
#define REJ_DOUBLE_NOTE			(0x1813)	//重券判定

// ichijo 2020.08.31
#define DOUBLE_UNFIT 40 // unfitレベル
#define DOUBLE_LEVEL 100 // この値と計算結果の差分でレベルを決定する 減点方式
//#define NEW_DOUBLE_CHECK // 新しい重券アルゴを使用する場合に有効にする。※一時的な対応で、すべての国で再学習ができたら過去のソースは排除する。
//重券チェック
typedef struct
{
	u16 max_x;	//等分割最大
	u16 max_y;	
	u16 blank;
	u16 threshold;		//閾値

	s16 note_size_x;
	s16 note_size_y;

	u16 red_mask_ptn_diameter_x;			// 赤色マスクパターンの直径x
	u16 red_mask_ptn_diameter_y;			// 赤色マスクパターンの直径y
	float red_mask_ptn_divide_num;			// 赤色マスクパターンの割る数
	u8*	pred_mask_ptn;						// 赤色マスクパターン1のポインタ

	u16 tred_mask_ptn_diameter_x;			// 透過赤マスクパターンの直径x
	u16 tred_mask_ptn_diameter_y;			// 透過赤マスクパターンの直径y
	float tred_mask_ptn_divide_num;			// 透過赤マスクパターンの割る数
	u8*	ptred_mask_ptn;						// 透過赤マスクパターン1のポインタ
	u8 level; // レベル
	u8 padding[3]; 

} ST_DOUBLE_CHECK;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_double_check_error(u8 buf_num ,ST_DOUBLE_CHECK *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif 
