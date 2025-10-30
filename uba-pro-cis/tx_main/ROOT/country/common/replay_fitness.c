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
 * @file replay_fitness.c
 * @brief 正損再現性に関するソース
 * @date 2020/11/30 Created.
 *  Author: 
 */
/****************************************************************************/
//#include "systemdef.h"
#include <string.h>

#define EXT
#include "../common/global.h"

/****************************************************************/
/**
 * @brief 正損再現リストの登録レベルと今回の出力レベルを比較して、現在のデータを書き換える
 * IN : idx 				参照するリスト内のインデックス
 * IN : current_level_list	処理中のレベルデータリスト
 * IN : buf_num				処理データのバッファ番号
 * ret: なし
 */
/****************************************************************/
s16	rewrite_currentdata_uf_level(u16 idx, u8* current_level_list, u8 buf_num)
{
	//u8 thr_count;
	u32 i = 0;
	//u8 thr[REPLAY_FITNESS_THR_LEVEL_NUM][MAX_VALIDATION_COUNT] = {0};
	u8 margin[MAX_VALIDATION_COUNT] = {0}; 
	u8 noteID;
	s16 range_up  = 0;
	s16 range_low = 0;

	noteID = replay_fitness_work.noteid[buf_num];
	//memcpy(&thr[0][0],&fit_level_thrs[noteID].fit_level[0],MAX_VALIDATION_COUNT);
	//memcpy(&thr[1][0],&fit_level_thrs[noteID].uf_level[0],MAX_VALIDATION_COUNT);
	//memcpy(&thr[2][0],&fit_level_thrs[noteID].rj_level[0],MAX_VALIDATION_COUNT);
	memcpy(&margin[0],&fit_level_thrs[noteID].level_margin[0],MAX_VALIDATION_COUNT);

	for (i = 0; i < MAX_VALIDATION_COUNT; ++i)
	{
		if (margin[i] != 0)											//マージン値 "0" は再現処理の対象外
		{															//
			range_up  = replay_fitness_list.level[idx][i] + margin[i];	//登録レベル範囲を計算
			range_low = replay_fitness_list.level[idx][i] - margin[i];	//

			if(range_up > 100)										//値が範囲外（1~100）にならないように
			{
				range_up = 100;
			}

			if(range_low < 1)
			{
				range_low = 1;
			}
			
			if (range_up >= current_level_list[i] &&				//登録レベル範囲内？
				range_low <= current_level_list[i])					//
			{														//
				//
				current_level_list[i] = replay_fitness_list.level[idx][i];//登録レベルで出力レベルを更新する。
			}														//
		}
	}

	return 0;
}

/****************************************************************/
/**
 * @brief 現在のデータを正損再現リストへ登録する。
 * 		  既に登録済みの場合は上書きする。
 * 		  登録されていない場合はリストに追加する。
 * IN : idx 				参照するリスト内のインデックス
 * IN : current_level_list	処理中のレベルデータリスト
 * IN : buf_num				処理データのバッファ番号
 * OUT：replay_fitness_work.registration_number		登録されている数
 * OUT：replay_fitness_work.list_poling_buffer_offset	現在のテーブルのオフセット
 * ret: なし
 */
/****************************************************************/
s16	set_uf_level_data(u32 idx, u8* current_level_list, u8 buf_num)
{

#define DELETE_REGIST_SERIAL '0'	//リストから削除するときに記番号情報を初期化する値

	//u8  thr_count;
	u32 i = 0;
	//u8 thr[REPLAY_FITNESS_THR_LEVEL_NUM][MAX_VALIDATION_COUNT] = {0};
	u8 margin[MAX_VALIDATION_COUNT] = {0}; 
	//u8 set_flg = 0;
	u8 noteID;
	u32	JCMID;

	//s16 range_up  = 0;
	//s16 range_low = 0;

	JCMID = replay_fitness_work.jcmid[buf_num];
	noteID = replay_fitness_work.noteid[buf_num];
	//memcpy(&thr[0][0],&fit_level_thrs[noteID].fit_level[0],MAX_VALIDATION_COUNT);
	//memcpy(&thr[1][0],&fit_level_thrs[noteID].uf_level[0],MAX_VALIDATION_COUNT);
	//memcpy(&thr[2][0],&fit_level_thrs[noteID].rj_level[0],MAX_VALIDATION_COUNT);
	memcpy(&margin[0],&fit_level_thrs[noteID].level_margin[0],MAX_VALIDATION_COUNT);

	if(replay_fitness_work.hit[buf_num] == REPLAY_FITNESS_LIST_MISS_HIT)				 //ヒットなしの場合
	{																		 //
		idx = replay_fitness_work.list_poling_buffer_offset;						 //オフセット値でインデックスを更新
	}																		 //
																			 //
	for (i = 0; i < MAX_VALIDATION_COUNT; ++i)								 //正損再現リストへ現在の値を登録する。
	{																		 //
		replay_fitness_list.level[idx][i] = 0; 									 //リストの値を”0”で初期化
		//
		//for (thr_count = 0; thr_count < REPLAY_FITNESS_THR_LEVEL_NUM; ++thr_count) //どのレベルの閾値が記録されているかを調べる
		//{																	 //3種類のレベルにマージン値を±した値の
		//	range_up  = thr[thr_count][i] + margin[i];						 //閾値レベル範囲を計算
		//	range_low = thr[thr_count][i] - margin[i];						 //

		//	if(range_up >= 100)												 //値が範囲外（1~100）にならないように
		//	{
		//		range_up = 101;
		//	}

		//	if(range_low < 1)
		//	{
		//		range_low = 1;
		//	}

		//	if ((u8)range_up  > current_level_list[i] &&					 //レベルが分かったので今回のデータがこのレベル内に
		//		(u8)range_low <= current_level_list[i])						 //入っているかを調べる。
		//	{																 //
		//		replay_fitness_list.level[idx][i] = current_level_list[i];		 //入っていた場合、現在の値でリストの値を更新する。
		//		set_flg = 1;												 //リストが更新されたこと記録
		//		break;														 //
		//	}																 //
		//}

		if(margin[i] == 0)													//マージン値が0ならば
		{																	//正損再現リストへの登録はされない。
			continue;
		}

		replay_fitness_list.level[idx][i] = current_level_list[i];				//現在の値でリストの値を更新する。
	}

	if (replay_fitness_work.hit[buf_num] == REPLAY_FITNESS_LIST_MISS_HIT)	//ミスヒットの場合は新規登録なので
	{											   							//オフセット値および登録数を更新する
		replay_fitness_work.registration_number++;					  		//登録数をインクリメント。
		if (replay_fitness_work.registration_number > REPLAY_FITNESS_LIST_MAX) 	//最大値を上回らないようにする。
		{										   							//
			replay_fitness_work.registration_number = REPLAY_FITNESS_LIST_MAX; 			//
		}										   							//

		replay_fitness_work.list_poling_buffer_offset++; 					//オフセット値をインクリメント
		if (replay_fitness_work.list_poling_buffer_offset > REPLAY_FITNESS_LIST_MAX - 1)//リングバッファに対応するために
		{																	//最大値となったら0に初期化
			replay_fitness_work.list_poling_buffer_offset = 0;
			replay_fitness_work.laps++;										//周回数をインクリメント
		}

		//記番号情報と金種情報を登録する。
		memcpy(replay_fitness_list.replay_fitness_data[idx].list_1_serial, work[buf_num].pbs->ser_num1, REPLAY_FITNESS_SERIAL_LEN);
		replay_fitness_list.replay_fitness_data[idx].revision = 0;		//記番号の色情報
		replay_fitness_list.replay_fitness_data[idx].index = JCMID	;	//金種情報
	}
	//else if(set_flg == 0 && replay_fitness_work.hit[buf_num] == REPLAY_FITNESS_LIST_HIT)			// 正損再現リストへの登録が発生しなかったが
	//{																		// ヒットはしている場合
	//	//記番号情報と金種情報を削除する。
	//	memset(replay_fitness_list.replay_fitness_data[idx].list_1_serial,DELETE_REGIST_SERIAL, REPLAY_FITNESS_SERIAL_LEN);
	//	replay_fitness_list.replay_fitness_data[idx].revision = 0;
	//	replay_fitness_list.replay_fitness_data[idx].index = 0;
	//}

	return 0;
}


/***************************************************************/
/**
 * @brief 正損再現リスト照合処理
 *        照合したいデータ列と正損再現リスト内の全桁全データと照合する。
 *        replay_fitness_work.hit[buf_num]に合致有/無がセットされる。
 *        また、ヒットした場合、結果返り値でヒットしたリストのインデックスを返す。
 * IN : preplay_fitness_list:ST_UF_REPRO_EACH_INFO型の正損再現リストデータ。	高速化の為、構造体実体は4byte境界にアライメントしてください。
 * IN : psttar:照合したいデータのポインタ								高速化の為、構造体実体は4byte境界にアライメントしてください。
 * IN : lst_num:正損再現リストの登録件数
 * IN : buf_num:サンプリングバッファー番号
 * ret: 該当有時：その正損再現リストインデックス番号　　　該当無し時：意味なし
 */
/****************************************************************/
s16	replay_fitness_list_check(ST_REPLAY_FITNESS_EACH_INFO* preplay_fitness_list, ST_REPLAY_FITNESS_EACH_INFO* psttar, u32 lst_num, u8 buf_num)
{
	s32	flag = 0;
	u32	ii = 0;

	// 全ブラックリスト合致チェック
	for(ii = 0; ii < lst_num; ii++)
	{
		flag = memcmp(&preplay_fitness_list[ii], psttar, REPLAY_FITNESS_SERIAL_LEN + 4);	// index は比較対象外

		if( 0 == flag)
		{
			// 合致
			replay_fitness_work.hit[buf_num] = REPLAY_FITNESS_LIST_HIT;	//ヒットフラグ
			return (s16)ii;									// ヒットした正損再現リストインデックス番号
		}
	}

	replay_fitness_work.hit[buf_num] = REPLAY_FITNESS_LIST_MISS_HIT;		//ミスヒットフラグ
	return 0;													// 意味なし
}


/****************************************************************/
/**
 * @brief 文字のワイルドカード文字'*'のところを0x00、それ以外は0xffにする。
 *        作成したマスクデータは引数pmaskで示されるポインタにセットされる。
 *			（index部は除く）
 *
 * IN : pd:文字列ポインタ
 * OUT: pmask:マスクデータをセットするポインタ
 * ret: 無し
 */
/****************************************************************/

// OCR読み取り文字のワイルドカード文字のところを0x00、それ以外は0xffにする
void replay_fitness_make_wildcard_maskdt(u8* pd, u8* pmask)
{
	u8 jj;

	for(jj = 0; jj < REPLAY_FITNESS_LIST_ONE_SIZE - 4; jj++)	// indexを除くメンバーを対象とする為　-4
	{
		if( '*' == pd[jj])
		{
			pmask[jj] = 0;			// ワイルドカード文字ならば0をセットする、
		}else
		{
			pmask[jj] = 0xff;		// ワイルドカード文字でなければ0xffをセットする、
		}
	}
}

/****************************************************************/
/**
 * @brief 正損再現処理に関するメモリを初期化します
 */
/****************************************************************/
void replay_fitness_data_initialize(void)
{
	memset(&replay_fitness_list, 0, sizeof(ST_REPLAY_FITNESS_LIST_DATA));
	memset(&replay_fitness_work, 0, sizeof(ST_REPLAY_FITNESS_WORK));
}

//end of file

