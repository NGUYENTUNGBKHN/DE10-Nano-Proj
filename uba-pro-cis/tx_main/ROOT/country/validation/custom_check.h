#ifndef	_CUSTOMCHECK_INCLUDE_H
#define	_CUSTOMCHECK_INCLUDE_H

/****************************************************************************/
/**
 * @file customcheck.h
 * @brief カスタムチェック
 * @date 2023/02/16　ver1.1
 */
 /****************************************************************************/

#define CUSTOMCHECK_CALCULATION_NUM 16
#define CUSTOMCHECK_USEDPLANE 8
#define CUSTOMCHECK_CALCULATION_USEDPLANE_NUM 5
#define CUSTOMCHECK_PARA_NUM 20
#define CUSTOMCHECK_2CL_BIAS 127
#define CUSTOMCHECK_3CL_RATIO 255
#define CUSTOMCHECK_X_SKIP 2
#define CUSTOMCHECK_Y_SKIP 2

#define CUSTOMCHECK_CF_JUDGE_LEVEL 40

typedef struct//カスタムチェック　計算内容構造体
{
	s16 type;//判定種類　0：エリア割合　1：1色平均　2：2色差　3：3色比
	s16 margin;//
	s16 min;//
	s16 max;//
	s8 usedplane[CUSTOMCHECK_CALCULATION_USEDPLANE_NUM];//計算で使用するプレーン
	s8 padding[3];

}CUSTOMCHECK_PARA_CAL;

typedef struct//カスタムチェック　プレーン構造体
{
	s16 plane;//チェックするプレーン
	s16 level;//閾値

}CUSTOMCHECK_PARA_USEDPLANE;

typedef struct //カスタムチェック　エリア構造体
{
	s16 startx;//左上
	s16 starty;
	s16 endx;//右下
	s16 endy;
	
}CUSTOMCHECK_PARA_AREA;

//カスタムチェックパラメタ構造体
typedef struct
{
	CUSTOMCHECK_PARA_AREA cca;
	CUSTOMCHECK_PARA_USEDPLANE ccu[CUSTOMCHECK_USEDPLANE];
	CUSTOMCHECK_PARA_CAL ccc[CUSTOMCHECK_CALCULATION_NUM];

	u8 usedplane_num;
	u8 cal_num;	
	
}CUSTOMCHECK_PARA;

//カスタムチェック構造体
typedef struct
{
	CUSTOMCHECK_PARA para[CUSTOMCHECK_PARA_NUM];
	u8 paranum;

	//結果
	u16 judge[CUSTOMCHECK_PARA_NUM];//エリアごとの判定結果
	u8 result_num;//最小出力レベルの判定番号
	u8 result_area;//最小出力レベルのエリア
	u8 sum_num;//CF判定になった判定数
	u8 sum_area;//CF判定になったエリア数
	s32 value;//最小出力計算結果
	u8 level;//最小出力レベル

}ST_CUSTOMCHECK_PARA;

//テンプレートからパラメタを受け取る
typedef struct
{
	s16 startx;
	s16 starty;
	s16 endx;
	s16 endy;
	u16	usedplane_num;
	u16	cal_num;
	u32	usedplane_ofs;
	u32	cal_ofs;

}ST_CUSTOMCHECK;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

u8 customcheck_judge(s32 result, s16 max, s16 min, s16 margin);
s32 customcheck_calculation(u32* sum, u32 usedpoint, u8 type, s8* usedplane, u32 area);
s16 customcheck(u8 buf_num, ST_CUSTOMCHECK_PARA* cc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
