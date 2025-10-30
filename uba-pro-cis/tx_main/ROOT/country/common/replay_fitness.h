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
 * @file replay_fitness.h
 * @brief 正損再現性に関するソースのヘッダファイルです
 * @date 2020/11/30 Created.
 */
/****************************************************************************/
#if !defined(__REPLAY_FITNESS_H_INCLUDED__)
#define __REPLAY_FITNESS_H_INCLUDED__

#include <stdint.h>

#define BLACKLIST_SUCCESS				0x0000		//

#define REPLAY_FITNESS_LIST_MAX			1000	//正損再現リスト最大登録可能件数
#define REPLAY_FITNESS_SERIAL_LEN		16		//記番号長さ
#define REPLAY_FITNESS_THR_LEVEL_NUM	3		//閾値レベルの数

#define REPLAY_FITNESS_LIST_MISS_HIT	0		//ミスヒット
#define REPLAY_FITNESS_LIST_HIT		1		//ヒット

#define REPLAY_FITNESS_OCR_SUCCESS	1		//OCR成功
#define REPLAY_FITNESS_OCR_FAILURE	0		//OCR失敗

#define REPLAY_FITNESS_MODE_ON		1		//正損再現モードオン

#define REPLAY_FITNESS_LIST_ONE_SIZE	24		//正損再現リスト構造体一件のサイズ（バイト）

#define REPLAY_FITNESS_BUFF_NUM_MAX		100		//そのうち　機の情報から取ってくる　今はLE17
#define REPLAY_FITNESS_HEADER			32		//HEADERサイズ



/****************************************************************/
/**
 * @brief 正損再現リスト1件分(バイナリ
 */
/****************************************************************/
typedef struct
{
	u8	list_1_serial[REPLAY_FITNESS_SERIAL_LEN];	//記番号
	u8	revision;							//リビジョン
	u8	dummy[3];							//予備
	u32	index;								//通貨テーブル情報から取ってきたNoteIDを設定する
}ST_REPLAY_FITNESS_EACH_INFO;


/****************************************************************/
/**
 * @brief 正損再現リストの構造体
 */
/****************************************************************/
typedef struct
{
	u8	header[REPLAY_FITNESS_HEADER];										//ヘッダ情報　カテゴリなど
	u32	record_data_size;													//replay_fitness_dataのサイズ
	u32 record_num;															//登録件数
	u32 record_offset;														//現在のレコードの位置
	u32 record_laps;														//周回数
	u32 record_level_size;													//level配列のサイズ
	u8	padding[12];														//予備
	ST_REPLAY_FITNESS_EACH_INFO	replay_fitness_data[REPLAY_FITNESS_LIST_MAX];//正損再現リスト
	u8 level[REPLAY_FITNESS_LIST_MAX][MAX_VALIDATION_COUNT];				//各アルゴのレベル

}ST_REPLAY_FITNESS_LIST_DATA;

/****************************************************************/
/**
 * @brief 正損再現のワークメモリ構造体
 */
/****************************************************************/
typedef struct
{
	
	u8	hit[REPLAY_FITNESS_BUFF_NUM_MAX];			//ヒットの有無
	u8	ocr_res[REPLAY_FITNESS_BUFF_NUM_MAX];		//ocrの成功・失敗を記録する。0：失敗　1：成功
	u8	noteid[REPLAY_FITNESS_BUFF_NUM_MAX];		//現在のノートIDを記録する。バッファ分用意する
	u32	jcmid[REPLAY_FITNESS_BUFF_NUM_MAX];			//現在のJCMIDを記録する。バッファ分用意する
	s16 hit_list_index[REPLAY_FITNESS_BUFF_NUM_MAX];//ヒットしたリストのインデックスを記録する。
	u8	padding;
	u8	mode;										//正損再現モードのON/OFF
	s16 registration_number;						//登録件数
	u32	laps;										//リングバッファを周回した回数
	u32 list_poling_buffer_offset;	//現在の記録メモリのオフセット　範囲：0~999
									//登録が1000未満：現状使用されていないインデックスを指定する
									//登録が1000以上：リングバッファ上で最も若い位置を指定する	
}ST_REPLAY_FITNESS_WORK;


EXTERN ST_REPLAY_FITNESS_LIST_DATA	replay_fitness_list;	//記番号とそのデータのアルゴレベルを記録する
EXTERN ST_REPLAY_FITNESS_WORK		replay_fitness_work;	//正損再現処理のフラグ関係を記録する

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

s16	rewrite_currentdata_uf_level(u16 idx, u8* current_level_list, u8 buf_num);
s16	set_uf_level_data(u32 idx, u8* current_level_list, u8 buf_num);
s16	replay_fitness_list_check(ST_REPLAY_FITNESS_EACH_INFO* preplay_fitness_list, ST_REPLAY_FITNESS_EACH_INFO* psttar, u32 blk_num, u8 buf_num);
void replay_fitness_make_wildcard_maskdt(u8* pd, u8* pmask);
void replay_fitness_data_initialize(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __REPLAY_FITNESS_H_INCLUDED__ */


//end of file


