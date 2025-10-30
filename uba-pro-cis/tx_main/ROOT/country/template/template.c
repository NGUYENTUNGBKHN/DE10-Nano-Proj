/****************************************************************************/
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
* MODEL NAME : 識別共有
* @file template.c
* @brief  テンプレートシーケンスの実装ファイルです。
* @date 2018.08.22
* @author JCM. OSAKA TECHNICAL RESARCH 1 GROUP ELEMENTAL TECHNOLOGY RECERCH DEPT.
* @Version 1.5.0
* @updata
*	https://app.box.com/s/miz957a00xui7atlphaci17ceaz6gc5f
* 
*/
/****************************************************************************/
//#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <inttypes.h> 

//新識別
//#define DEBUG_DENOMINATION
#ifdef DEBUG_DENOMINATION
#include "../denomination/test/simple_test.h"
#include "../denomination/config/denomi_config.h"
#include "../denomination/mlp_inference/outlier_checker.h"
#include "../denomination/denomi_cpp/utilities.h"
#include "../denomination/mlp_inference/MLPClassifier.h"
#include "../ocr/cod_malloc.h"
//#include <time.h>
#endif

#define EXT
#include "../common/global.h"

#ifndef _RM_

#include <windows.h>


#endif

#define DSCRM_4to1				//コメントアウトすることで鑑別を4方向1方向に切り替える
#define STRACTURE_DEFINITION	//コメントアウトすることで各処理の専用構造体をグローバルで宣言する。

#define TEMPLATE_MASKING_LIMIT	20

#ifdef VS_DEBUG
EXTERN int debug_logi_view;	//トレースするかしないか
EXTERN int debug_itool_trace_view;	//トレースするかしないか
//下記を有効にすることで、その処理をビューアでトレースする。
//#define VS_EXETRA_POINTS	//ポイント抽出
//#define VS_DEBUG_STAIN	    //染み
//#define VS_DEBUG_TEAR	    //裂け
//#define VS_DEBUG_SOILING	//汚れ
//#define VS_DEBUG_DE_INKD	//脱色
//#define VS_DEBUG_DYENOTE	//ダイノート
//#define VS_DEBUG_DOGEAR		//角折れ
//
//#define VS_DEBUG_NEOMAG	    //NEOMAG
//#define VS_DEBUG_THREAD	    //スレッド
//#define VS_DEBUG_HOLOGRAM	//ホログラム
//#define VS_DEBUG_HOLE	    //穴
//#define VS_DEBUG_SPECIAL_A  //特殊A
//#define VS_DEBUG_SPECIAL_B	//特殊B
//#define VS_DEBUG_ICHIJO	    //Rba鑑別
//#define VS_DEBUG_FOLD       //折り畳み欠損
//#define VS_DEBUG_IR2WAVE    //IR2波長
//#define VS_DEBUG_UV_FITNESS //洗濯券 UV正損
//#define VS_DEBUG_UV_VALI    //UV鑑別
//#define VS_DEBUG_MAG        //MAG鑑別
//#define VS_DEBUG_CONVENTIONAL_VALIDATION //従来鑑別
//#define VS_DEBUG_CUSTOMCHECK //カスタムチェック
//#define VS_DEBUG_DOUBLE //光学重券

//#define VS_DEBUG_OVERALL	//総合判定
//#define VS_DEBUG_OCR		//OCR
//#define VS_DEBUG_BARCODE	//バーコード

#endif

#ifndef STRACTURE_DEFINITION
ST_EdgEDetecT ED;
ST_DATA_EXTRACTION data_extraction;
ST_NN_RESULT nn_res;
ST_NN nn_cfg;						//NN用構造体
ST_SJ sj;
ST_3CIR cir3;
ST_4CIR cir4;
ST_IR_CHECK irc;
ST_MCIR mcir;
ST_VALI_NN1 nn1;
ST_VALI_NN2 nn2;
ST_DOG_EAR st_dog;
ST_TEAR st_tr;
ST_DYE_NOTE st_dy;
ST_SOILING st_soi;
ST_DEINKED st_deink;
ST_STAIN st_sta;
COD_Parameters st_cod;
ST_DOUBLE_CHECK dbl_chk;
ST_FOLDING folding;
ST_TAPE st_tape;
ST_CAPACITANCE_TAPE st_cap_tape;
ST_SPECIAL_A spa;
ST_SPECIAL_B spb;
#endif

/****************************************************************/
/**
* @brief		テンプレートシーケンスを開始する
*@param[in]	<入力引数arg0の説明>
*@param[out]	<出力引数arg1の説明]>
* @return		<戻り値の説明>
*/
/****************************************************************/
void	start_template_sequence(u8 buf_n)
{
	T_TEMPLATE_WORK* pst_work;
	u16 err = 0;

	// イニシャル
	pst_work = &st_work[buf_n];								// バッファ番号が示すデータのポインタ
	memset(pst_work, 0, sizeof(T_TEMPLATE_WORK));			// 現在のワークメモリのエリアを0クリア
	pst_work->buf_n = buf_n;								// バッファ番号保存 
	pst_work->e_code = 0;									// エラーコードのクリア
	pst_work->now_para_blk_pt = (T_DECISION_PARAMETER_COMMON*)((u8*)template_top_pt + FILE_INFO_SIZE);	// 現在の判定ブロックポインタをブロックテーブルの先頭にリセット
	pst_work->ID_template = TLPT_ID_INIT;					// ID 初期設定
	pst_work->p_result_blk_top = decision_result_tbl[buf_n];// 結果ブロックのポインタ先頭をセット
	pst_work->now_result_blk_pt = (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top;	// 現在の結果ブロックのポインタをリセット
	//memset(pst_work->now_result_blk_pt, 0, RESULT_TBL_SIZE);// 現在の結果ブロックのエリアを0クリア   ---> 時間がかかりそうなので、随時　ブロック追加時に必要な便だけ０クリアしていく。
	pst_work->current_mode |= (1 << work[buf_n].pbs->transport_direction);	//搬送方向
	//pst_work->current_mode = work[buf_n]->pbs->spec_code;	//仕様コードセット

	//側面対応
	pst_work->side_masking_area		= (u8)*((u8*)template_top_pt + 88);	//値の取得
	pst_work->vertical_masking_area = (u8)*((u8*)template_top_pt + 89);	//
	if (pst_work->side_masking_area > TEMPLATE_MASKING_LIMIT)		//整合確認
	{
		pst_work->side_masking_area = 0;
	}
	if (pst_work->vertical_masking_area > TEMPLATE_MASKING_LIMIT)	//整合確認
	{
		pst_work->vertical_masking_area = 0;
	}

//#ifndef _RM_
	err = template_currenct_table_top_detect(pst_work);			//テンプレート内券種IDの先頭アドレスを記録する。
	if (err == -1)
	{
		return;						//テンプレート内券種IDが見つからなかった場合エラーとする。
	}

//#endif

#ifdef _RM_
	if (p_temp_currenct_table_top.tbl_num == 0)	// 閾値レベルテーブルがないときは
	{
		err = set_fitness_thrs(pst_work, 0, SET_DEFAULT_THRS);	// ここでデフォルトの閾値レベルを読み込む
		if (0 != err)
		{
			pst_work->e_code = err;				// エラーならば終了
			return;
		}
	}
	else										// 閾値レベルテーブルがあるときは
	{
		err = set_fitness_thrs(pst_work, 0, SET_DYENOTE_ONLY);	// ここでダイノートのみの閾値レベルを読み込む
		if (0 != err)
		{
			pst_work->e_code = err;								//エラーならば終了
			return;
		}
	}
#else
	err = (u16)set_fitness_thrs(pst_work, 0, SET_DEFAULT_THRS);	//フィットネス判定用閾値を読み込む
	//err = (u16)set_fitness_thrs(pst_work, 0, SET_CURRENCY_THRS);	//フィットネス判定用閾値を読み込む
	if (0 != err)
	{
		pst_work->e_code = err;								//エラーならば終了
		return;
	}
#endif

	//記番号位置情報クリア
	memset(&work[buf_n].pbs->series_1_top_x, 0, sizeof(s16) * 8);

	deecision_block_sequence_loop(pst_work);				// シーケンスループ実行

}

/****************************************************************/
/**
* @brief		テンプレートシーケンスを続行する st_work.p_now_blockのブロックから続行する
*@param[in]	<入力引数arg0の説明>
*@param[out]	<出力引数arg1の説明]>
* @return		<戻り値の説明>
*/
/****************************************************************/
void	restart_template_sequence(u8 buf_n)
{
	T_TEMPLATE_WORK* pst_work;

	// 続行イニシャル
	pst_work = &st_work[buf_n];
	deecision_block_sequence_loop(pst_work);			// シーケンスループ実行
}

/****************************************************************/
/**
* @brief		テンプレートシーケンスの実行ループ
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return		<戻り値の説明>
*/
/****************************************************************/
void	deecision_block_sequence_loop(T_TEMPLATE_WORK* pst_work)
{
	//u16 temp_functioncode = 0;
	u8 flag = 1;
	//u8 result;
	//u32 start = FPGA_REG.TIMSTMP;
	u32 block_count = 0;
	//u32 dyenote_err_code = 0;
	//u8  dyenote_err_code_flg = 0;
	//pst_work->now_para_blk_pt->function_code = FNC_DUAL_IR;
	/***/
	while (MAX_TEMPLATE_BLOCK_NAM > block_count)		// 判定ブロック実行シーケンス　ループ
	{
		block_count++;									//機能ブロック数をカウント

		if (ID_chk(pst_work))							// 処理対象ID なら、
		{
			if (flag == 1) //restartでの機能コードセット用
			{
				//temp_functioncode = pst_work->now_para_blk_pt->function_code;
				flag = 0;
			}

			template_log1(pst_work->now_para_blk_pt->function_code, pst_work->buf_n, work[pst_work->buf_n].pbs->proc_num);

			switch (pst_work->now_para_blk_pt->function_code)// 機能コードに従った処理を行う
			{
			case FNC_BAR_CODE:								//バーコードリード
				ts_barcode(pst_work);
				break;

				//識別処理--------------------------------------------------------------------------
			case FNC_OUTLINE_DETECT:
				ts_otline_det(pst_work);					// 外形検知の処理関数();					//
				break;

			case FNC_PDT_EXTRACTION:						// Ver2
				ts_extraction_pointdata(pst_work);			// ポイントデータ抽出関数();
				break;

			case FNC_NN_JUDGE:
				ts_NN_judge_proc(pst_work);					// NN 判定関数();
				break;

			case FNC_SHIN_SHIKI_BETSU:						// 新識別
				//start_time(0);
				denomination_test(pst_work);
				//u32 tt = end_time(0, NULL);
				//printf("time0 FNC_SHIN_SHIKI_BETSU %d\n", tt);
				break;

			case FNC_OL_SIZE_CHECK:
				ts_oline_size_check(pst_work);				// 外形サイズチェック関数();
				break;

			case FNC_DOUBLE_CHECK:
				ts_double_check(pst_work);					// 重券
				break;

			case FNC_JUKEN_NN:
				ts_double_chk_nn(pst_work);					//新重券検知（光学式）
				break;

			case FNC_DBL_CHK_MECHA:
				ts_double_check_mecha(pst_work);			// 重券検知（メカ厚）
				break;

			case FNC_OL_SIZE_IDENT:
				ts_oline_size_ident(pst_work);				// 外形サイズ識別;
				break;

				//鑑別処理--------------------------------------------------------------------------
			case FNC_CIR_3COLOR:
				ts_cir_3color(pst_work);					// ３色比
				break;

			case FNC_CIR_4COLOR:
				ts_cir_4color(pst_work);					// ４色比
				break;

			case FNC_IR_CHECK:
				ts_ir_check(pst_work);						// IRチェック
				break;

			case FNC_MCIR:
				ts_mcir(pst_work);							// MCIR
				break;

			case FNC_MAG:
				ts_mag(pst_work);							//MAG
				break;

			case FNC_DUAL_IR:
				ts_ir2wave(pst_work);						//IR2
				break;

			case FNC_VALI_NN_1COLOR:
				ts_neural_network_1color(pst_work);			// 鑑別１色NN
				break;

			case FNC_VALI_NN_2COLOR:
				ts_neural_network_2color(pst_work);			// 鑑別２色NN
				break;

			case FNC_OCR:
				ts_ocr(pst_work);							//OCR
				break;

			case FNC_NEOMAG:
				ts_neomag(pst_work);						//NEOMAG
				break;

			case FNC_MAGTHREAD:
				ts_magthread(pst_work);						//磁気スレッド
				break;

			case FNC_METALTHREAD:
				ts_metalthread(pst_work);					//金属スレッド
				break;

			case FNC_DYE_NOTE:
				ts_dye_note(pst_work);						//ダイノート x
				break;

			case FNC_HOLOGRAM:
				ts_hologram(pst_work);						//ホログラム
				break;

			case FNC_UV_VALIDATE:
				ts_uv_validate(pst_work);					//鑑別UV
				break;

			case FNC_SPECIAL_A:
				ts_special_a(pst_work);						//特殊A
				break;

			case FNC_SPECIAL_B:
				//ts_special_b(pst_work);						//特殊B
				break;

			case FNC_CF_NN_1COLOR:
				ts_cf_nn_1color(pst_work);						//CF-NN1
				break;

			case FNC_CF_NN_2COLOR:
				ts_cf_nn_2color(pst_work);						//CF-NN2
				break;

			case FNC_CUSTOM_CHECK:
				ts_customcheck(pst_work);				// カスタムチェック
				break;

				//正損判定--------------------------------------------------------------------------
			case FNC_FOLDING:
				ts_folding(pst_work);						//折り畳み・欠損
				break;

			case FNC_DOG_EAR:
				ts_dog_ear(pst_work);						//角折れ
				break;

			case FNC_TEAR:
				ts_tear(pst_work);							//裂け
				break;

			case FNC_STAIN:
				ts_stain(pst_work);							//染み x
				break;

			case FNC_DE_INK:
				ts_de_ink(pst_work);						//脱色 x
				break;

			case FNC_SOILING:
				ts_soiling(pst_work);						//汚れ x
				break;

			case FNC_HOLE:
				ts_hole(pst_work);							//穴
				break;

			case FNC_TAPE:
				ts_tape(pst_work);							// テープ(メカ式)
				break;

			case FNC_CAP_TAPE:
				ts_cap_tape(pst_work);						//静電テープ
				break;

			case FNC_WASH:
				ts_uv_fitness(pst_work);					//蛍光正損
				break;

			case FNC_GRAFFITI_TEXT:
				ts_graffiti_text(pst_work);					//落書き
				break;

				//対応予定なし------------------------------------------------------------------------

			case FNC_CIR_2COLOR:
				//overall_judge_proc(pst_work);				//２色差
				break;

				//case FNC_CAP_THREAD:
				//	ts_hole(pst_work);							//静電スレッド
				//	break;

				//シェル専用関数--------------------------------------------------------------------------
			case FNC_OVERALL_JUDGE:
				overall_judge_proc(pst_work);				// 総合判定
				break;

			case FNC_NN_OVALL_JUDGE:
				multi_nn(pst_work);							//複合NN判定
				break;

			case FNC_JCMID_TBL:
				//jcmID_tbl_proc(pst_work);					// テンプレート内券種　JCM ID　対応表処理関数();
				break;

			case FNC_JMP:
				fnc_jmp(pst_work);							// JMP関数
				continue;									// 既に次のブロックのポインタがセットされているので、このままそれを実行する。

			case FNC_BRK:									// 中断
				// 一時停止の時、続行できるように次ブロックのポインタをセットしておく。完全ENDの時は次ブロックオフセットが0なので変化なし
				pst_work->now_para_blk_pt = (T_DECISION_PARAMETER_COMMON*)((u8*)pst_work->now_para_blk_pt + pst_work->now_para_blk_pt->next_block_ofs);	// 次ブロックのポインタをセット
				return;										// 正常終了
				//break;

			case FNC_END:									// 終了
				pst_work->template_shell_completion = 1;
				return;										// 正常終了

			default:									// エラー
				pst_work->e_code = E_UNDEFIND_FNC_CODE;	// 定義されていない機能コード
				return;									// ここで終了
				//pst_work->now_para_blk_pt = (T_DECISION_PARAMETER_COMMON*)((u32)pst_work->now_para_blk_pt + pst_work->now_para_blk_pt->next_block_ofs);		// 次ブロックのポインタをセット
			}

			template_log2(pst_work->now_para_blk_pt->function_code, pst_work->buf_n, work[pst_work->buf_n].pbs->proc_num);
		}

		if (pst_work->e_code != 0)															// 続行不可のエラー有り？
		{
			if ((pst_work->e_code & 0xFF00) != 0x2400)	//総合判定結果のリザルトコード以外？
			{
				if (pst_work->e_code != REJ_NEW_DYE_INK && pst_work->e_code != REJ_OLD_DYE_INK)	//ダイノート以外ならば
				{
					work[pst_work->buf_n].pbs->category_ecb = category_decision(pst_work);		//エラー判定時のカテゴリ決定
					pst_work->template_shell_completion = 1;									//テンプレートシェル終了フラグを立てる
					return;																		//終了
				}

				else																			//ダイノートだったら
				{
					//dyenote_err_code_flg = 1;													//ダイノートエラーフラグをセットして
					//dyenote_err_code = pst_work->e_code;										//エラーコードを記録する。

					work[pst_work->buf_n].pbs->category_ecb = category_decision(pst_work);		//エラー判定時のカテゴリ決定
					pst_work->template_shell_completion = 1;									//テンプレートシェル終了フラグを立てる
					return;																		//終了
				}
			}
		}

		pst_work->now_para_blk_pt = (T_DECISION_PARAMETER_COMMON*)((u8*)pst_work->now_para_blk_pt + pst_work->now_para_blk_pt->next_block_ofs);			// 次ブロックのポインタをセット
		//temp_functioncode = FNC_BRK; //一時停止

	}
}

/****************************************************************/
/**
* @brief		10ns カウンター値取得
				ハード依存部
				処理時間計測用なので最悪必要なし。
*@param[in]	なし
*@param[out]	なし
* @return
*/
/****************************************************************/
u32	get_10ns_co(void)
{
#ifndef SIMURATION
	uint64_t time = alt_globaltmr_get64();

	time *= 0.5;
	return	(u32)time;
#else
	return 0;
#endif
}

/****************************************************************/
/**
* @brief		識別デバッグログ用関数①
				機種依存部分
				引数、処理内容は機種担当者が自由に編集してもよい

*@param[in]		機種依存
*@param[out]	なし
*@return		なし
*/
/****************************************************************/
void template_log1(s32 code, s32 buf_num, s32 proc_num)
{
#ifdef _RM_			/* シミュレータと実機用を切りかえるため */

#ifdef BAU_LE17		//BAULE17
	jdl_debug_record_template_log2(code, buf_num, proc_num);
#endif

#else
	return;

#endif
}

/****************************************************************/
/**
* @brief		識別デバッグログ用関数②
				機種依存部分
				引数、処理内容は機種担当者が自由に編集してもよい

*@param[in]		機種依存
*@param[out]	なし
*@return		なし
*/
/****************************************************************/
void template_log2(s32 code, s32 buf_num, s32 proc_num)
{
#ifdef _RM_			/* シミュレータと実機用を切りかえるため */

#ifdef BAU_LE17		//BAULE17
	jdl_debug_record_template_log2(code, buf_num, proc_num);
#endif

#else
	return;

#endif
}

/****************************************************************/
/**
* @brief		JMP処理を行う
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return		void
*/
/****************************************************************/
void	fnc_jmp(T_TEMPLATE_WORK* pst_work)
{
	T_DECISION_PARAMETER_JMP* p_jmp_blk;

	p_jmp_blk = (T_DECISION_PARAMETER_JMP*)pst_work->now_para_blk_pt;
	pst_work->now_para_blk_pt = (T_DECISION_PARAMETER_COMMON*)((u8*)pst_work->now_para_blk_pt + p_jmp_blk->jmp_ofs); // 現ブロックポインタにJMP先アドレスをセット
}


/****************************************************************/
/**
* @brief		処理対象IDのチェックを行う。
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return		処理対象なら1を、対象外なら0を返す。
*/
/****************************************************************/
u32	ID_chk(T_TEMPLATE_WORK* pst_work)
{
	u16* p_ID_list = 0;
	u8	n = 0;					// 要素数
	u16	cpID = 0;				// 比較するID
	u16	cpIDbase = 0;			// 比較するIDの本体部
	u16	cpIDdir = 0;			// 比較するIDの方向部
	u8  cpmode = 0;				// 比較するモード

	u16	para1 = 0;
	u16	para2 = 0;
	u16	para1base = 0;
	u16	para2base = 0;

	cpID = pst_work->ID_template;			// 現在のテンプレート内券種ID
	cpIDbase = cpID & 0x0fff;				//券種ID抽出
	cpIDdir = cpID & 0xf000;				//方向抽出
	cpmode = pst_work->now_para_blk_pt->trans_mode;		//このブロックの処理対象モード

	p_ID_list = (u16*)((u8*)pst_work->now_para_blk_pt + pst_work->now_para_blk_pt->ID_lst_ofs);	// ID list の先頭ポインタ
	n = pst_work->now_para_blk_pt->ID_lst_num;	// 要素数

	//19/7/22追加
	cpmode = pst_work->current_mode & cpmode;			//サンプリングデータに書き込まれているモードと比較。

	if (cpmode == 0)	//現在のモードと対象のモードのANDが０なら
	{
		return 0;	//対象外とする
	}

	// loop
	while (n >= 1)							// 残要素数が0でなければ、
	{										// 比較処理を続行
		if (*p_ID_list == CPID_ALL)			// ------- 全指定チェック -------
		{
			return	1;						// 対象とする
		}
		else if (*p_ID_list == CPID_RANGE)	// ------- 範囲指定チェック -------
		{
			n--;
			para1 = *++p_ID_list;
			n--;
			para2 = *++p_ID_list;

			para1base = para1 & 0x0fff;
			para2base = para2 & 0x0fff;

			// 範囲チェック
			if (para1base <= para2base)									// para1base <= para2base の時、
			{
				if ((para1base <= cpIDbase) && (cpIDbase <= para2base))	// 範囲内なら、
				{
					if (para1 & 0xf000 & cpIDdir)						// 方向フラグのチェックを行い、条件を満たしていれば、
					{
						return	1;										// 対象とする
					}
				}
			}
			else														// para2base < para1base の時、
			{
				if ((para2base <= cpIDbase) && (cpIDbase <= para1base))	// 範囲内なら、
				{
					if (para1 & 0xf000 & cpIDdir)	// para2ではない	// 方向フラグのチェックを行い、条件を満たしていれば、
					{
						return	1;										// 対象とする
					}
				}
			}
		}
		else
		{																// ------- 単純IDチェック -------
			if (cpIDbase == (*p_ID_list & 0x0fff))						// ID本体が同じで、
			{
				if (*p_ID_list & 0xf000 & cpIDdir)			// 方向フラグのチェックを行い、条件を満たしていれば、
				{
					return	1;										// 対象とする
				}


			}
		}
		// 次ID読み取り準備
		n--;
		p_ID_list++;
	}

	return	0;								// どれにも条件が当てはまらない時は、対象外とする。
}


/****************************************************************/
/**
* @brief		外形検知ブロック処理
* @param[in]	work メモリーのポインタ
* @param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
u32 ts_otline_det(T_TEMPLATE_WORK* pst_work)
{
	//T_OUTLINE_RESULT*	oline_blk_p;
	T_DECISION_PARAMETER_OUTLINE* p_oline;
#ifdef STRACTURE_DEFINITION
	ST_EdgEDetecT ED;
#endif
	u16 err_code = 0;
	u8 buf_num = pst_work->buf_n;
	s32 disable_area_thr;

	p_oline = (T_DECISION_PARAMETER_OUTLINE*)pst_work->now_para_blk_pt;

	// パラメタ取得
	if (p_oline->ei_wr)	// 中間情報に書くフラグのチェック。有効なら、
	{
	}

	ED.Plane = (enum P_Plane_tbl)p_oline->plane;			 //用いるプレーン指定
	//ED.Plane = UP_R_R;			 //用いるプレーン指定
	//if (work[buf_num].pbs->PlaneInfo[ED.Plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
	//{
	//	ED.Plane = DOWN_T_G;			 //用いるプレーン指定
	//	//ED.Plane = DOWN_T_R;			 //用いるプレーン指定
	//}

	//ED.Plane = (enum P_Plane_tbl)p_oline->plane;											// 用いる色情報
	ED.Step_Movement = p_oline->Step_Movement;							// 探索のステップ幅(mm) ※暫定値
	ED.Small_Backtrack = p_oline->Small_Backtrack;						// 次の探索開始位置への戻り幅(dot) ※暫定値
	ED.max_skew_thr = p_oline->max_skew_thr;							// 最大スキューの値
	ED.vertex_err_determination = p_oline->vertex_err_determination;	// 頂点が搬送路外かどうかのエラー判定を省略する　1:実行　0：実行しない
	ED.abnormal_position_of_ticket_edge = p_oline->abnormal_position_of_ticket_edge;	// 券端の異常位置検知　1:実行　0：実行しない
	ED.abnormal_position_of_ticket_edge = 0;	// 券端の異常位置検知　1:実行　0：実行しない
	ED.th_dynamic_decision_flg = p_oline->th_dynamic_decision_flg;		// 閾値動的決定　1:実行　0：実行しない
	ED.multiple_transport = p_oline->multiple_transport;				// 多重搬送検知　BAULE17用 1:実行　0：実行しない 

	//紙粉対策パラメタ----
	if (work[buf_num].pbs->disable_area_a == 0 && work[buf_num].pbs->disable_area_b == 0)	//0が設定されて行場合はそのままの値を使う
	{
		//
		ED.Main_scan_max_val = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max;	// イメージデータの主走査方向ドット数
		ED.Main_scan_min_val = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min;	// イメージデータの主走査方向ドット数
		ED.Main_scan_max_val_ori = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max;
		ED.Main_scan_min_val_ori = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min;
	}
	else	//値が設定されてる場合
	{
		//上限値を計算する　有効画素範囲÷2の値
		disable_area_thr = (s32)((work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max - work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min) * 0.5f * 0.127f);

		//設定値が上限値を超えていた場合は無効にし、本来の値を設定する。
		if (disable_area_thr <  work[buf_num].pbs->disable_area_a || disable_area_thr <  work[buf_num].pbs->disable_area_b || 0 > work[buf_num].pbs->disable_area_b || 0 > work[buf_num].pbs->disable_area_a)
		{
			ED.Main_scan_max_val = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max;	// イメージデータの主走査方向ドット数
			ED.Main_scan_min_val = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min;	// イメージデータの主走査方向ドット数
			ED.Main_scan_max_val_ori = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max;
			ED.Main_scan_min_val_ori = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min;
		}
		else	//設定値に問題が無ければ
		{
			//計算される前の値を保存する
			ED.Main_scan_max_val_ori = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max;
			ED.Main_scan_min_val_ori = work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min;

			//スキャン方向によって計算を変更する。
			if (work[buf_num].pbs->PlaneInfo[ED.Plane].note_scan_dir == R2L)	//
			{
				//上位で設定された無効エリアの幅と本来の開始終了位置から新たな位置を計算する。
				ED.Main_scan_max_val = (int)(work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max - work[buf_num].pbs->disable_area_a / 0.127f);
				ED.Main_scan_min_val = (int)(work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min + work[buf_num].pbs->disable_area_b / 0.127f);
			}
			else
			{
				ED.Main_scan_max_val = (int)(work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_max - work[buf_num].pbs->disable_area_b / 0.127f);
				ED.Main_scan_min_val = (int)(work[buf_num].pbs->PlaneInfo[ED.Plane].main_effective_range_min + work[buf_num].pbs->disable_area_a / 0.127f);
			}
		}
	}
	//--------------------

	if (ED.Main_scan_min_val < 0)
	{
		ED.Main_scan_min_val = ED.Main_scan_min_val_ori;
	}
	if (ED.Main_scan_max_val < 0)
	{
		ED.Main_scan_max_val = ED.Main_scan_max_val_ori;
	}


	ED.Sub_scan_max_val = (((work[buf_num].pbs->Blocksize / work[buf_num].pbs->PlaneInfo[ED.Plane].sub_sampling_pitch) * work[buf_num].pbs->block_count));	// 副走査方向の最大有効範囲
	ED.Sub_scan_min_val = 1;

	ED.MinSE = 10;		// 札のSE側の最小許容値 (mm)※暫定値
	ED.MaxSE = 2000;	// 札のSE側の最大許容値 (mm)※暫定値
	ED.MinLE = 10;		// 札のLE側の最小許容値 (mm)※暫定値
	ED.MaxLE = 2000;	// 札のLE側の最大許容値 (mm)※暫定値

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_OUTLINE, dbg_codi_Physical);		// デバッグ用コールバック
#endif
#ifdef VS_DEBUG_ITOOL
	if (debug_itool_trace_view == 1)
	{
		cal_P2L_parameter(buf_num);
		cal_note_parameter(buf_num);

	}
#endif

	pst_work->proc_tm_co_s = get_10ns_co();									// 処理時間計測用カウンタ値取得 
	err_code = (u16)OPdet_Start(buf_num, &ED);								// 外形検出ルーチン
	pst_work->proc_tm_co_e = get_10ns_co();									// 処理時間計測用カウンタ値取得

	// 結果確認
	if (err_code != 0)	/*外形検知失敗*/
	{

		// 結果確認
		work[buf_num].pbs->angle = err_code;		// 傾き
		work[buf_num].pbs->center_x = 0xFFFF;			// 中心座標x
		work[buf_num].pbs->center_y = 0xFFFF;			// 中心座標y
		work[buf_num].pbs->note_y_size = 0xFFFF;		// 札幅
		work[buf_num].pbs->note_x_size = 0xFFFF;		// 札長

		oline_det_result(buf_num, &ED);					// 結果ブロックをセット(外形検知)
		pst_work->e_code = err_code;
		return (u32)err_code;
	}

	//成功時　サンプリングデータに書き込み
	work[buf_num].pbs->angle = ED.SKEW;							// 傾き (tan_th *4096)
	work[buf_num].pbs->center_x = (s16)ED.AlignedBillCenterPoint.x;	// 中心座標x
	work[buf_num].pbs->center_y = (s16)ED.AlignedBillCenterPoint.y;	// 中心座標y
	work[buf_num].pbs->note_y_size = (s16)ED.width_dot;				// 札幅
	work[buf_num].pbs->note_x_size = (s16)ED.length_dot;				// 札長

	work[buf_num].pbs->left_up_x = (s16)ED.AlignedUpperLeft.x;
	work[buf_num].pbs->left_up_y = (s16)ED.AlignedUpperLeft.y;
	work[buf_num].pbs->left_down_x = (s16)ED.AlignedLowerLeft.x;
	work[buf_num].pbs->left_down_y = (s16)ED.AlignedLowerLeft.y;
	work[buf_num].pbs->right_up_x = (s16)ED.AlignedUpperRight.x;
	work[buf_num].pbs->right_up_y = (s16)ED.AlignedUpperRight.y;
	work[buf_num].pbs->right_down_x = (s16)ED.AlignedLowerRight.x;
	work[buf_num].pbs->right_down_y = (s16)ED.AlignedLowerRight.y;

	//指定座標の色を変える　デバッグ用
	//work[buf_num].pbs->sens_dt[work[buf_num].pbs->PlaneInfo[ED.Plane].Address_Period * work[buf_num].pbs->left_up_y + work[buf_num].pbs->left_up_x + work[buf_num].pbs->PlaneInfo[ED.Plane].Address_Offset] = 0xff;
	//work[buf_num].pbs->sens_dt[work[buf_num].pbs->PlaneInfo[ED.Plane].Address_Period * work[buf_num].pbs->left_down_y + work[buf_num].pbs->left_down_x + work[buf_num].pbs->PlaneInfo[ED.Plane].Address_Offset] = 0xff;

	//////////////外形検知を実行しない場合----------------------------------------------------
	//ED.SKEW = work[buf_num].pbs->angle;
	//ED.AlignedBillCenterPoint.x = work[buf_num].pbs->center_x;
	//ED.AlignedBillCenterPoint.y = work[buf_num].pbs->center_y;
	//ED.width_dot = work[buf_num].pbs->note_y_size;
	//ED.length_dot = work[buf_num].pbs->note_x_size;

	//ED.AlignedUpperLeft.x = work[buf_num].pbs->left_up_x;
	//ED.AlignedUpperLeft.y = work[buf_num].pbs->left_up_y;
	//ED.AlignedLowerLeft.x = work[buf_num].pbs->left_down_x;
	//ED.AlignedLowerLeft.y = work[buf_num].pbs->left_down_y;
	//ED.AlignedUpperRight.x = work[buf_num].pbs->right_up_x;
	//ED.AlignedUpperRight.y = work[buf_num].pbs->right_up_y;
	//ED.AlignedLowerRight.x = work[buf_num].pbs->right_down_x;
	//ED.AlignedLowerRight.y = work[buf_num].pbs->right_down_y;
	//////////////----------------------------------------------------

	//座標変換に必要なパラメタを計算。
	work[pst_work->buf_n].pbs->insertion_direction = W0;		// A方向で初期化　add by furuta 2022/4/4
	cal_note_parameter(pst_work->buf_n);
	oline_det_result(buf_num, &ED);								// 結果ブロックをセット(外形検知)

//#ifdef VS_DEBUG_SOILING
//	debug_logi_view = 1;
//#endif
//	work[buf_num].pbs->insertion_direction = 0;
//	work[buf_num].pbs->insert_way = work[buf_num].pbs->insertion_direction;
//
//	advance_insert_correction(pst_work->buf_n);
//	decision_plane_side_in_insertion_direction(pst_work->buf_n);
//
//	ST_INFERENCE_RESULT ress;
//
//	soiling_ver4(pst_work->buf_n, 15, &ress);
//
//#ifdef VS_DEBUG_SOILING
//	debug_logi_view = 0;
//#endif
	return 0;
}

/****************************************************************/
/**
* @brief		ポイントデータ抽出処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
u32	ts_extraction_pointdata(T_TEMPLATE_WORK* pst_work)
{
	const u32 EXRPOINTDATA_AFTER_IDENT = 1;

	s16 ii = 0;
	s16 err_code;
	//T_EXTRACT_POINT_DT*	extr_point_blk = 0;
	T_DECISION_PARAMETER_POINTDT* p_extr_point = 0;
	T_POINTDT_MASK_INFO* p_mask;
	//u8 temp_way = 0;


#ifdef STRACTURE_DEFINITION
	ST_DATA_EXTRACTION data_extraction;
#endif

	err_code = 0;
	// パラメタ取得
	p_extr_point = (T_DECISION_PARAMETER_POINTDT*)pst_work->now_para_blk_pt;

	// CFNN時で用いる 座標系をリセットする
	if (p_extr_point->after_ident == EXRPOINTDATA_AFTER_IDENT)
	{
		reset_insart_multi_num(pst_work->buf_n);
	}

	//機能セット
	data_extraction.outline = 0;//p_extr_point->outline_blk_num;	// 外形検知の判定結果ブロック連番を指定する。0:外形基準  1:印刷枠基準(未実装)
	data_extraction.split_x = p_extr_point->x_dev_num;		// 横分割数
	data_extraction.split_y = p_extr_point->y_dev_num;		// 縦分割数
	data_extraction.corner_flg = p_extr_point->ei_corner;	// コーナー無効に　する:0/しない:1
	data_extraction.plane_count = p_extr_point->num;		// プレーンの要素数

	//マスキングエリアをテンプレートシーケンスのワークメモリから取得
	data_extraction.side_masking_area = pst_work->side_masking_area;
	data_extraction.vertical_masking_area = pst_work->vertical_masking_area;

	//マスクの設定
	for (ii = 0; ii < data_extraction.plane_count; ii++)
	{
		p_mask = &p_extr_point->mask_info[ii];
		data_extraction.plane_lst[ii] = p_mask->plane_no;      					// プレーンリスト

		//物理をスキャンする際、dpiが異なるので、プレーンごとにマスクパターンとサイズを持つこととする。
		data_extraction.filter_size_x[ii] = p_mask->mask_size_x;				// 横フィルターサイズ
		data_extraction.filter_size_y[ii] = p_mask->mask_size_y;				// 縦フィルターサイズ
		data_extraction.pfilter_pat[ii] = (u8*)(pst_work->now_para_blk_pt) + (p_mask->planes_lst_ofs);			// フィルターパターンの先頭アドレス
		//data_extraction.divide_val[ii] = (float)(1.0 / (p_mask->devdt[1] * 0x100 + p_mask->devdt[0]));			// オフセットがおかしいので暫定対応
		data_extraction.divide_val[ii] = (float)(1.0f / p_mask->devdt);							// フィルター領域内の合計値を割る値(逆数をセット)+

	}


#ifdef VS_EXETRA_POINTS
	debug_logi_view = 1;
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_IDENT_NN, dbg_codi_Physical);		// デバッグ用コールバック
#endif

	//実行
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得 
	// ポイントデータ抽出ルーチン本体
	err_code = Data_extraction(&data_extraction, pst_work->buf_n);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//DLL_Data_extraction((void*)work[pst_work->buf_n].pbs, (DLL_ST_DATA_EXTRACTION*)&data_extraction);


#ifdef VS_EXETRA_POINTS
	debug_logi_view = 0;
#endif


	extraction_pointdate_result(pst_work->buf_n, &data_extraction);			// 結果ブロックをセット(ポイントデータ抽出Ver2)

	// extraction_pointdate_result関数で実行
#if 0
	blksiz = sizeof(T_EXTRACT_POINT_DT);					// 固定長部と,
	//blksiz += calculate_size_including_padding_data(sizeof());	// 可変長部（パディングを含む）の合計サイズを計算する。

	extr_point_blk = (T_EXTRACT_POINT_DT*)add_result_blk(	// 結果追加
		sizeof(T_OUTLINE_RESULT),				//// 追加する領域サイズ
		pst_work,								//// workのポインタ
		0,										//// 終了状態
		pst_work->ID_template					//// 出力券種ID
	);


	// 機能固有項固定長部情報を書き込む
	extr_point_blk->ei_corner = p_extr_point->ei_corner;	// コーナー有効無効フラグのコピー
	extr_point_blk->extract_point_dt_ofs = sizeof(T_EXTRACT_POINT_DT);	// 可変長データである抽出ポイントデータのこの結果ブロック先頭からのオフセット値
	//extr_point_blk->extract_point_dt_num = DEB_NUM;			// 可変長データである抽出ポイントデータの要素数

	// 可変長部1をコピーする。（パディング含まず）
	//memcpy(extr_point_blk->extract_point_dt_ofs + extr_point_blk, , sizeof());					// 可変長データを追加
#endif
	pst_work->e_code = err_code;

	// CFNN時で用いる リセットした座標系を元に戻す
	if (p_extr_point->after_ident == EXRPOINTDATA_AFTER_IDENT)
	{
		set_dir_params(pst_work, 0);
	}

	return err_code;
}


/****************************************************************/
/**
* @brief		NN 判定処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_NN_judge_proc(T_TEMPLATE_WORK* pst_work)
{
	T_DECISION_PARAMETER_NN* p_nn_blk;
	T_EXTRACT_POINT_DT* p_reference;

#ifdef STRACTURE_DEFINITION
	ST_NN_RESULT nn_res;
	ST_NN nn_cfg;						//NN用構造体
#endif

	u16 input_node_num = 0;

	//	u16 err_code = 0;
	u8 buf_num = pst_work->buf_n;

	// パラメタ取得
	p_nn_blk = (T_DECISION_PARAMETER_NN*)pst_work->now_para_blk_pt;


	//参照する結果ブロックをサーチする
	p_reference = (T_EXTRACT_POINT_DT*)search_blk_address_of_seq_num(p_nn_blk->reference_blk_num, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);


	input_node_num = p_nn_blk->NN_in_num;	//バイアスや札サイズノード関係で必要

	//機能セット
	nn_cfg.in_node = p_nn_blk->NN_in_num;        //入力層ノード数
	nn_cfg.center_node = p_nn_blk->NN_midle_num; //中間層ノード数
	nn_cfg.out_nide = p_nn_blk->NN_out_num;      //出力層ノード数

	nn_cfg.sizeflag = p_nn_blk->add_size;        //入力ノードにサイズデータを使用する場合 0：使用しない　1：使用する
	nn_cfg.biasflag = p_nn_blk->use_bias;        //バイアスノードを使用する場合 0：使用しない　1：使用する

	nn_cfg.length = (u8)work[buf_num].pbs->note_y_size;
	nn_cfg.width = (u8)work[buf_num].pbs->note_x_size;

	nn_cfg.do_normalize = 1;		             //正規化実行フラグを設定する。
	nn_cfg.p_input_rawdata = (u16*)((u8*)p_reference + p_reference->extract_point_dt_ofs);


	//紙幣サイズありの設定
	if (p_nn_blk->add_size == 1)
	{
		//p_nn_blk->NN_in_num = p_nn_blk->NN_in_num + 2;
		input_node_num += 2;
	}
	//バイアス項ありの設定
	if (p_nn_blk->use_bias == 1)
	{
		//p_nn_blk->NN_in_num = p_nn_blk->NN_in_num + 1;
		input_node_num += 1;
	}

	nn_cfg.pcenter_wit_offset = (float*)((u8*)p_nn_blk + p_nn_blk->weight_dt_ofs); 					//中間ウェイトデータ先頭位置オフセット
	nn_cfg.pout_wit_offset = (nn_cfg.pcenter_wit_offset + input_node_num * p_nn_blk->NN_midle_num);	//出力ウェイトデータ先頭位置オフセット

	//p_nn_node_inf = (T_DECISION_PARAMETER_NODE_INF *)((u32)p_nn_blk + p_nn_blk->node_inf_ofs/* + (nn_res.output_max_node * sizeof(T_DECISION_PARAMETER_NODE_INF)) */);
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	NN(&nn_cfg, &nn_res);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//結果ブロックへの書き込みとエラー判定
	pst_work->e_code = NN_judeg_proc_result(buf_num, &nn_cfg, &nn_res);

}


/****************************************************************/
/**
* @brief		新識別
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void denomination_test(T_TEMPLATE_WORK* pst_work)
{
	//u8* x_matrix = 0;	//1次元　ｘ　2次元ｙ　3次元プレーン
	u8* x_matrix_symmetric = 0;	//1次元　ｘ　2次元ｙ　3次元プレーン

	u8 buf_num = pst_work->buf_n;
	//int POINT_POS = 2;// point block at index 2

	s16 ii = 0;
	s16 ori_x = 0;
	s16 ori_y = 0;
	s16 ori_wid = 0;
	s16 ori_hid = 0;
	s16 start_x = 0;
	s16 start_y = 0;
	s16 temp = 0;

	s16 wid = 0;
	s16 hid = 0;
	u8 plane[1];
	//u8 plane_num = sizeof(plane);
	u16 i = 0;
	s16 y_t = 0;
	s16 x_l = 0;
	s16 y_b = 0;
	s16 x_r = 0;

	u16 new_width = 0;
	u16 new_hight = 0;
	u16 pixels_per_cell = 0;
	u16 good_width = 0;
	u16 good_hight = 0;

	u8* img_resize = 0;
	u16 in_width = 0;
	u16 in_height = 0;
	u16 out_width = 0;
	u16 out_height = 0;

	u32 x_ratio = 0;
	u32 y_ratio = 0;
	u16 i_out, j_out;
	u16 i_in, j_in;
	u32 rat;
	u32 idx_col;
	u32 idx_in_img;
	s16 logic_x_l[4] = { 0 }, logic_x_r[4] = { 0 };
	s16 logic_y_t[4] = { 0 }, logic_y_b[4] = { 0 };
	s16 logic_w = 0;
	s16 logic_h = 0;

	float* start_roi_idx;

	u32 k = 0;
	u32 x_ROI = 0;
	u32 y_ROI = 0;

	s16 x_banknotes = 0;
	s16 y_banknotes = 0;
	u8 matrix_k;
	u8* start_block = 0;
	//u16 dst_h = 0;
	u16 dst_w = 0;
	u16 row = 0;
	u16 col = 0;

	u64 idx;
	u64 idx_in;
	u16 m;
	u16 n;

	new_nn_config* nn_config = (new_nn_config*)pst_work->now_para_blk_pt;
	ST_DATA_EXTRACTION data_extraction;
	T_POINTDT_MASK_INFO* p_mask;
	ST_NEW_NN_RESULT nn_res;

	data_extraction.outline = 0;//p_extr_point->outline_blk_num;	// 外形検知の判定結果ブロック連番を指定する。0:外形基準  1:印刷枠基準(未実装)
	data_extraction.split_x = nn_config->x_dev_num;		// 横分割数
	data_extraction.split_y = nn_config->y_dev_num;		// 縦分割数
	data_extraction.corner_flg = nn_config->ei_corner;		// コーナー無効に　する:0/しない:1
	data_extraction.plane_count = nn_config->num;				// プレーンの要素数

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//マスクの設定
	for (ii = 0; ii < data_extraction.plane_count; ii++)
	{
		p_mask = &nn_config->mask_info[ii];
		data_extraction.plane_lst[ii] = p_mask->plane_no;      					// プレーンリスト

		//物理をスキャンする際、dpiが異なるので、プレーンごとにマスクパターンとサイズを持つこととする。
		data_extraction.filter_size_x[ii] = p_mask->mask_size_x;				// 横フィルターサイズ
		data_extraction.filter_size_y[ii] = p_mask->mask_size_y;				// 縦フィルターサイズ
		data_extraction.pfilter_pat[ii] = (u8*)(pst_work->now_para_blk_pt) + (p_mask->planes_lst_ofs);			// フィルターパターンの先頭アドレス
		data_extraction.divide_val[ii] = (float)(1.0f / p_mask->devdt);							// フィルター領域内の合計値を割る値(逆数をセット)+

	}
	Data_extraction(&data_extraction, pst_work->buf_n);
	// Huong add
	work[pst_work->buf_n].pbs->insertion_direction = W0;
	////方向補正関係の係数を決定する。
	advance_insert_correction(pst_work->buf_n);

	//float cc = 3.1234241234165234;

	ori_x = -work[buf_num].pbs->note_x_size / 2;
	ori_y = work[buf_num].pbs->note_y_size / 2;
	ori_wid = work[buf_num].pbs->note_x_size;
	ori_hid = work[buf_num].pbs->note_y_size;
	start_x = -work[buf_num].pbs->note_x_size / 2;
	start_y = work[buf_num].pbs->note_y_size / 2;

	// Huong add 20220316
	if (work[buf_num].pbs->LEorSE != LE)
	{
		temp = ori_x;
		ori_x = -ori_y;
		ori_y = -temp;

		temp = ori_wid;
		ori_wid = ori_hid;
		ori_hid = temp;

		start_x = ori_x;
		start_y = ori_y;
	}

	wid = 0;
	hid = 0;
	plane[0] = nn_config->plane;
	//plane_num = sizeof(plane);
	i = 0;

	logic_w = 0;
	logic_h = 0;

	start_roi_idx = (float*)((u8*)nn_config + nn_config->OffsetROI);

	if (nn_config->Is_Symmetric)
	{

		for (i = 0; i < 4; ++i)
		{
			y_t = ori_hid * (*start_roi_idx++);//top
			x_l = ori_wid * (*start_roi_idx++);//left
			y_b = ori_hid * (*start_roi_idx++);//bottom
			x_r = ori_wid * (*start_roi_idx++);//right

			// JCM座標系に変換
			logic_x_l[i] = ori_x + x_l;
			logic_x_r[i] = ori_x + x_r;
			logic_y_t[i] = ori_y - y_t;
			logic_y_b[i] = ori_y - y_b;

		}

		logic_w = logic_x_r[0] - logic_x_l[0];
		logic_h = -logic_y_b[0] + logic_y_t[0];	// he toa do JCM nguoc nen doi dau

		// resize
		new_width = 96 / 2;
		new_hight = 96 / 2;
		pixels_per_cell = nn_config->pixels_per_cell;
		good_width = (new_width / pixels_per_cell) * pixels_per_cell;
		good_hight = (new_hight / pixels_per_cell) * pixels_per_cell;

		x_matrix_symmetric = (u8*)malloc_char(sizeof(u8) * 4 * good_hight * good_width);
		img_resize = (u8*)malloc_char(sizeof(u8) * good_width * good_hight);
		in_width = logic_w;
		in_height = logic_h;
		out_width = good_width;
		out_height = good_hight;

		hid = out_height * 2;
		wid = out_width * 2;

		x_ratio = (u32)((in_width << 16) / (out_width)) + 1;
		y_ratio = (u32)((in_height << 16) / (out_height)) + 1;
		if ((out_width < 2) || (out_height < 2)) {
			return;
		}
		for (i = 0; i < 4; ++i)
		{
			for (i_out = 0; i_out < out_height; ++i_out)
			{
				i_in = ((i_out * y_ratio) >> 16);
				idx_in_img = i_in * in_width;

				rat = 0;
				idx_col = i_out * out_width;

				for (j_out = 0; j_out < out_width; ++j_out)
				{
					j_in = rat >> 16;
					k = idx_in_img + j_in;
					x_ROI = k % in_width;	// tren ROI
					y_ROI = k / in_width;// tren ROI

					// cho nay can check lai
					x_banknotes = x_ROI + logic_x_l[i];
					y_banknotes = -y_ROI + logic_y_t[i];
					matrix_k;
					matrix_k = get_pixel(&work[buf_num].note_param, work[buf_num].pbs, buf_num, x_banknotes, y_banknotes, plane);
					img_resize[idx_col + j_out] = matrix_k;	// gio tinh gia tri matrix_k
					rat += x_ratio;
				}
			}

			//dst_h = out_height * 2;
			dst_w = out_width * 2;
			row = i / 2;
			col = i % 2;

			// ghep 4 ROI lai
			idx = (u64)row * (u64)out_height * (u64)dst_w + (u64)col * (u64)out_width;
			for (m = 0; m < out_height; ++m) {
				idx_in = (u64)out_width * (u64)m;
				for (n = 0; n < out_width; ++n)
				{
					x_matrix_symmetric[idx + n] = img_resize[idx_in + n];
				}
				idx += dst_w;
			}


		}
		free_proc(img_resize);

	}
	else
	{
		logic_y_t[0] = ori_hid * (*start_roi_idx++);
		logic_x_l[0] = ori_wid * (*start_roi_idx++);
		logic_y_b[0] = ori_hid * (*start_roi_idx++);
		logic_x_r[0] = ori_wid * (*start_roi_idx++);
		logic_w = logic_x_r[0] - logic_x_l[0];
		logic_h = logic_y_b[0] - logic_y_t[0];

		// resize
		new_width = 96;
		new_hight = 96;
		pixels_per_cell = nn_config->pixels_per_cell;
		good_width = (new_width / pixels_per_cell) * pixels_per_cell;
		good_hight = (new_hight / pixels_per_cell) * pixels_per_cell;
		x_matrix_symmetric = (u8*)malloc_char(sizeof(u8) * good_width * good_hight);
		//u8 *img_resize = (u8*)malloc_char(sizeof(u8) * good_width * good_hight);
		in_width = logic_w;
		in_height = logic_h;
		out_width = good_width;
		out_height = good_hight;

		hid = out_height;
		wid = out_width;

		x_ratio = (u32)((in_width << 16) / (out_width)) + 1;
		y_ratio = (u32)((in_height << 16) / (out_height)) + 1;
		if ((out_width < 2) || (out_height < 2))
		{
			return;
		}

		for (i = 0; i < 1; ++i) {
			for (i_out = 0; i_out < out_height; ++i_out) {
				i_in = ((i_out * y_ratio) >> 16);
				idx_in_img = i_in * in_width;

				rat = 0;
				idx_col = i_out * out_width;
				for (j_out = 0; j_out < out_width; ++j_out) {
					j_in = rat >> 16;
					k = idx_in_img + j_in;
					x_ROI = k % in_width;	// tren ROI
					y_ROI = k / in_width;// tren ROI

					// cho nay can check lai
					x_banknotes = x_ROI + logic_x_l[i];
					y_banknotes = -y_ROI + logic_y_t[i];
					matrix_k;
					matrix_k = get_pixel(&work[buf_num].note_param, work[buf_num].pbs, buf_num, x_banknotes, y_banknotes, plane);
					x_matrix_symmetric[idx_col + j_out] = matrix_k;	// gio tinh gia tri matrix_k
					rat += x_ratio;
				}
			}
		}

	}

	start_block = ((u8*)nn_config);
	nn_res.reject = integrate_test(buf_num, x_matrix_symmetric, hid, wid, nn_config, &data_extraction, start_block, &nn_res);
	//free_proc(x_matrix);
	free_proc(x_matrix_symmetric);

	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//u8 plane0;
	//ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;

	//for (plane0 = 0; plane0 < 42; plane0++)
	//{
	//	if (work[buf_num].pbs->LEorSE == LE)
	//	{
	//		//再初期化
	//		pnote_param->insart_multi_x[plane0] = (float)pnote_param->scan_dir_skew_coefficient[plane0];
	//		pnote_param->insart_multi_y[plane0] = 1;
	//	}

	//	else
	//	{
	//		//再初期化
	//		pnote_param->insart_multi_x[plane0] = -1;
	//		pnote_param->insart_multi_y[plane0] = (float)(-1 * pnote_param->scan_dir_skew_coefficient[plane0]);
	//	}
	//}
	//set_dir_params(pst_work, 0);

	//add by furuta 2022/4/4

	//nn_res.output_max_val
	//結果ブロックへの書き込みとエラー判定
	pst_work->e_code = New_NN_judeg_proc_result(buf_num, nn_config, &nn_res);
}

/****************************************************************/
/**
* @brief		外形サイズチェック　処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_oline_size_check(T_TEMPLATE_WORK* pst_work)
{

	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_SJ sj;
#endif

	//参照ブロック構造体
	T_OUTLINE_RESULT* p_reference;

	T_DECISION_PARAMETER_SIZ* p_siz_chk_blk;
	// パラメタ取得
	p_siz_chk_blk = (T_DECISION_PARAMETER_SIZ*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする
	p_reference = (T_OUTLINE_RESULT*)search_blk_address_of_seq_num(p_siz_chk_blk->reference_blk_num, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);

	//パラメタセット
	sj.thr_note_x_size_max = p_siz_chk_blk->x_max;
	sj.thr_note_y_size_max = p_siz_chk_blk->y_max;
	sj.thr_note_x_size_min = p_siz_chk_blk->x_mini;
	sj.thr_note_y_size_min = p_siz_chk_blk->y_mini;
	sj.note_x_size = p_reference->note_x_length;
	sj.note_y_size = p_reference->note_y_length;

	if (work[pst_work->buf_n].pbs->LEorSE == SE)
	{
		sj.note_y_size = p_reference->note_x_length;
		sj.note_x_size = p_reference->note_y_length;
	}
	sj.res = 0;

	pst_work->proc_tm_co_s = get_10ns_co();
	size_judge(&sj);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	pst_work->e_code = (u16)sj.res;

	oline_size_check_res(pst_work->buf_n, sj);
}

/****************************************************************/
/**
* @brief		外形サイズチェック　処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_oline_size_ident(T_TEMPLATE_WORK* pst_work)
{

	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_SJ sj;
#endif

	//参照ブロック構造体
	T_OUTLINE_RESULT* p_reference;

	T_DECISION_PARAMETER_SIZ_IDENT* p_siz_chk_blk;
	// パラメタ取得
	p_siz_chk_blk = (T_DECISION_PARAMETER_SIZ_IDENT*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする
	p_reference = (T_OUTLINE_RESULT*)search_blk_address_of_seq_num(p_siz_chk_blk->reference_blk_num, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);

	//パラメタセット
	sj.thr_note_x_size_max = p_siz_chk_blk->x_max;
	sj.thr_note_y_size_max = p_siz_chk_blk->y_max;
	sj.thr_note_x_size_min = p_siz_chk_blk->x_mini;
	sj.thr_note_y_size_min = p_siz_chk_blk->y_mini;
	sj.note_x_size = p_reference->note_x_length;
	sj.note_y_size = p_reference->note_y_length;
	sj.res = 0;

	if (work[pst_work->buf_n].pbs->LEorSE == SE)
	{
		sj.note_y_size = p_reference->note_x_length;
		sj.note_x_size = p_reference->note_y_length;
	}

	pst_work->proc_tm_co_s = get_10ns_co();
	size_judge(&sj);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//pst_work->e_code = (u16)sj.res;
	oline_size_ident_res(pst_work->buf_n, sj, p_siz_chk_blk->template_id);

}

/****************************************************************/
/**
* @brief		三色比
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_cir_3color(T_TEMPLATE_WORK* pst_work)
{

	s32 invalid_count = 0;
	u8  level = 0;

#ifdef DSCRM_4to1 
	u8 temp_way;
	ST_BS* pbs = work[pst_work->buf_n].pbs;
#endif

	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_3CIR cir3;
#endif
	float value;
	//判定パラメタ構造体
	T_DECISION_PARAMETER_CIR3* p_params;

	//参照ブロック構造体
	//T_OUTLINE_RESULT* p_reference;

	p_params = (T_DECISION_PARAMETER_CIR3*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//参照する結果ブロックをサーチする
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット
	memcpy(&cir3.cal_mode, &p_params->calculation_mode, 6);
	cir3.ppoint = (ST_LIM_AND_POINTS*)((u8*)(p_params)+(p_params->param_dt_ofs));

	cir3.red_mask_ptn_diameter_x = p_params->red_mask_ptn_diameter_x;
	cir3.red_mask_ptn_diameter_y = p_params->red_mask_ptn_diameter_y;
	cir3.red_mask_ptn_divide_num = p_params->red_mask_ptn_divide_num;
	cir3.pred_mask_ptn = (u8*)((u8*)(p_params)+p_params->red_mask_ptn_ofs);


	cir3.grn_mask_ptn_diameter_x = p_params->grn_mask_ptn_diameter_x;
	cir3.grn_mask_ptn_diameter_y = p_params->grn_mask_ptn_diameter_y;
	cir3.grn_mask_ptn_divide_num = p_params->grn_mask_ptn_divide_num;
	cir3.pgrn_mask_ptn = (u8*)((u8*)(p_params)+p_params->grn_mask_ptn_ofs);

	cir3.ir1_mask_ptn_diameter_x = p_params->ir1_mask_ptn_diameter_x;
	cir3.ir1_mask_ptn_diameter_y = p_params->ir1_mask_ptn_diameter_y;
	cir3.ir1_mask_ptn_divide_num = p_params->ir1_mask_ptn_divide_num;
	cir3.pir1_mask_ptn = (u8*)((u8*)(p_params)+p_params->ir1_mask_ptn_ofs);

#ifdef DSCRM_4to1
	//方向補正を解除するために再度処理を行う。
	temp_way = pbs->insertion_direction;
	pbs->insertion_direction = 0;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif


	//処理実行
	pst_work->proc_tm_co_s = get_10ns_co();
	invalid_count = get_cir_3color_invalid_count(pst_work->buf_n, &cir3);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	pst_work->e_code = (u16)0;

	//レベルを計算
	if (p_params->level_standard_value == 255)
	{
		value = 2;
	}
	else
	{
		value = p_params->level_standard_value;
	}

	if (invalid_count > value)
	{
		level = MAX_LEVEL;
	}
	else
	{
		level = MIN_LEVEL;
	}



#ifdef DSCRM_4to1
	//方向補正値再計算
	pbs->insertion_direction = temp_way;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

	//結果ブロックに書き込む
	cir_3color_result(pst_work->buf_n, (u16)invalid_count, level);

}

/****************************************************************/
/**
* @brief		4色比
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_cir_4color(T_TEMPLATE_WORK* pst_work)
{

	s32 invalid_count = 0;
	u8 level;
	float value;

#ifdef DSCRM_4to1 
	u8 temp_way;
	ST_BS* pbs = work[pst_work->buf_n].pbs;
#endif

	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_4CIR cir4;
#endif
	//判定パラメタ構造体
	T_DECISION_PARAMETER_CIR4* p_params;

	//参照ブロック構造体
	//T_OUTLINE_RESULT* p_reference;

	p_params = (T_DECISION_PARAMETER_CIR4*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//参照する結果ブロックをサーチする
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット
	memcpy(&cir4.cal_mode, &p_params->calculation_mode, 8);
	cir4.ppoint = (ST_LIM_AND_POINTS*)((u8*)(p_params)+(p_params->param_dt_ofs));
	//cir4.ppoint = (ST_LIM_AND_POINTS *)((u32)(p_params) + (p_params->param_dt_ofs));

	cir4.red_mask_ptn_diameter_x = p_params->red_mask_ptn_diameter_x;
	cir4.red_mask_ptn_diameter_y = p_params->red_mask_ptn_diameter_y;
	cir4.red_mask_ptn_divide_num = p_params->red_mask_ptn_divide_num;
	cir4.pred_mask_ptn = (u8*)((u8*)(p_params)+p_params->red_mask_ptn_ofs);

	cir4.grn_mask_ptn_diameter_x = p_params->grn_mask_ptn_diameter_x;
	cir4.grn_mask_ptn_diameter_y = p_params->grn_mask_ptn_diameter_y;
	cir4.grn_mask_ptn_divide_num = p_params->grn_mask_ptn_divide_num;
	cir4.pgrn_mask_ptn = (u8*)((u8*)(p_params)+p_params->grn_mask_ptn_ofs);

	cir4.ir1_mask_ptn_diameter_x = p_params->ir1_mask_ptn_diameter_x;
	cir4.ir1_mask_ptn_diameter_y = p_params->ir1_mask_ptn_diameter_y;
	cir4.ir1_mask_ptn_divide_num = p_params->ir1_mask_ptn_divide_num;
	cir4.pir1_mask_ptn = (u8*)((u8*)(p_params)+p_params->ir1_mask_ptn_ofs);

	cir4.ir2_mask_ptn_diameter_x = p_params->ir2_mask_ptn_diameter_x;
	cir4.ir2_mask_ptn_diameter_y = p_params->ir2_mask_ptn_diameter_y;
	cir4.ir2_mask_ptn_divide_num = p_params->ir2_mask_ptn_divide_num;
	cir4.pir2_mask_ptn = (u8*)((u8*)(p_params)+p_params->ir2_mask_ptn_ofs);

#ifdef DSCRM_4to1
	//方向補正を解除するために再度処理を行う。
	temp_way = pbs->insertion_direction;
	pbs->insertion_direction = 0;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif


	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	//処理実行
	invalid_count = get_cir_4color_invalid_count(pst_work->buf_n, &cir4);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	pst_work->e_code = (u16)0;

	//レベルを計算
	if (p_params->level_standard_value == 255)
	{
		value = 2;
	}
	else
	{
		value = p_params->level_standard_value;
	}

	if (invalid_count > value)
	{
		level = MAX_LEVEL;
	}
	else
	{
		level = MIN_LEVEL;
	}

#ifdef DSCRM_4to1
	//方向補正値再計算
	pbs->insertion_direction = temp_way;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

	//結果ブロックに書き込む
	cir_4color_result(pst_work->buf_n, (u16)invalid_count, level);

}

/****************************************************************/
/**
* @brief		irチェック
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_ir_check(T_TEMPLATE_WORK* pst_work)
{

	s32 invalid_count = 0;
	u8 level;
	float value;

#ifdef DSCRM_4to1 
	u8 temp_way;
	ST_BS* pbs = work[pst_work->buf_n].pbs;
#endif

#ifdef STRACTURE_DEFINITION
	//メイン構造体を定義する
	ST_IR_CHECK irc;
#endif

	//判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_IR_CHECK* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	//T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_IR_CHECK*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->comn.function_code, pst_work->p_result_blk_top);

	//パラメタセット
	memcpy(&irc.cal_mode, &p_params->calculation_mode, 2);
	irc.pmode = (ST_IR_MODES*)((u8*)(p_params)+(p_params->param_dt_ofs));
	irc.ppoint = (ST_LIM_AND_POINTS*)((u8*)irc.pmode + sizeof(ST_IR_MODES));

	irc.ir1_mask_ptn_diameter_x = p_params->ir1_mask_ptn_diameter_x;
	irc.ir1_mask_ptn_diameter_y = p_params->ir1_mask_ptn_diameter_y;
	irc.ir1_mask_ptn_divide_num = p_params->ir1_mask_ptn_divide_num;
	irc.pir1_mask_ptn = (u8*)((u8*)(p_params)+p_params->ir1_mask_ptn_ofs);

#ifdef DSCRM_4to1
	//方向補正を解除するために再度処理を行う。
	temp_way = pbs->insertion_direction;
	pbs->insertion_direction = 0;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif


	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//処理実行
	invalid_count = get_ir_check_invalid_count(pst_work->buf_n, &irc);

	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	pst_work->e_code = (u16)0;

	//レベルを計算
	if (p_params->level_standard_value == 255)
	{
		value = 2;
	}
	else
	{
		value = p_params->level_standard_value;
	}

	if (invalid_count > value)
	{
		level = MAX_LEVEL;
	}
	else
	{
		level = MIN_LEVEL;
	}

	//結果ブロックに書き込む
	ir_check_result(pst_work->buf_n, (u16)invalid_count, level);

#ifdef DSCRM_4to1
	//方向補正値再計算
	pbs->insertion_direction = temp_way;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

}

/****************************************************************/
/**
* @brief		mcir
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_mcir(T_TEMPLATE_WORK* pst_work)
{
#ifdef VS_DEBUG_ICHIJO
	FILE* test;// ichijo 一時的
	long size;//
#endif

	s32 invalid_count = 0;
	u8 level;
	//float value;

#ifdef DSCRM_4to1 
	u8 temp_way;
	ST_BS* pbs = work[pst_work->buf_n].pbs;
#endif

#ifdef STRACTURE_DEFINITION
	//メイン構造体を定義する
	ST_MCIR mcir;
#endif

	//判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_MCIR* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	//T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_MCIR*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->comn.function_code, pst_work->p_result_blk_top);

	//パラメタセット
	memcpy(&mcir.cal_mode, &p_params->calculation_mode, 6);
	mcir.ppoint = (ST_LIM_AND_POINTS*)((u8*)(p_params)+(p_params->param_dt_ofs));

	mcir.red_mask_ptn_diameter_x = p_params->red_mask_ptn_diameter_x;
	mcir.red_mask_ptn_diameter_y = p_params->red_mask_ptn_diameter_y;
	mcir.red_mask_ptn_divide_num = p_params->red_mask_ptn_divide_num;
	mcir.pred_mask_ptn = (u8*)((u8*)(p_params)+p_params->red_mask_ptn_ofs);


	mcir.grn_mask_ptn_diameter_x = p_params->grn_mask_ptn_diameter_x;
	mcir.grn_mask_ptn_diameter_y = p_params->grn_mask_ptn_diameter_y;
	mcir.grn_mask_ptn_divide_num = p_params->grn_mask_ptn_divide_num;
	mcir.pgrn_mask_ptn = (u8*)((u8*)(p_params)+p_params->grn_mask_ptn_ofs);

	mcir.ir1_mask_ptn_diameter_x = p_params->ir1_mask_ptn_diameter_x;
	mcir.ir1_mask_ptn_diameter_y = p_params->ir1_mask_ptn_diameter_y;
	mcir.ir1_mask_ptn_divide_num = p_params->ir1_mask_ptn_divide_num;
	mcir.pir1_mask_ptn = (u8*)((u8*)(p_params)+p_params->ir1_mask_ptn_ofs);

#ifdef DSCRM_4to1
	//方向補正を解除するために再度処理を行う。
	temp_way = pbs->insertion_direction;
	pbs->insertion_direction = 0;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_MCIR, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_CONVENTIONAL_VALIDATION
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();							// 処理時間計測用カウンタ値取得
	invalid_count = get_mcir_invalid_count(pst_work->buf_n, &mcir);	// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();							// 処理時間計測用カウンタ値取得
	pst_work->e_code = (u16)0;

#ifdef VS_DEBUG_CONVENTIONAL_VALIDATION
	debug_logi_view = 0;
#endif

	//レベルを計算
	// mcir_check.cで計算している
	level = mcir.level;
	//レベルを計算
/*if(p_params->level_standard_value == 255 || p_params->level_standard_value == 0)
{
	value = 1;
}
else
{
	value = p_params->level_standard_value;
}*/

/*if(invalid_count > value)
{
	level = MAX_LEVEL;
}
else
{
	level = MIN_LEVEL;
}*/

//結果ブロックに書き込む
	mcir_result(pst_work->buf_n, (u16)invalid_count, level);

	//#ifdef VS_DEBUG_ICHIJO
	//// ichijo 一時的
	//test = fopen("simulation_resul_mcir.csv", "a");
	//fseek(test, 0, SEEK_END);
	//size = ftell(test);
	//if (size == 0)
	//{
	//	fprintf(test, "FileNum, Way, InvalidCount, Level\n");
	//}
	//fprintf(test, "%d, %d, %d, %d, \n", (u32)pst_work->buf_n, (u32)temp_way, (u32)invalid_count, (u32)level);
	//fclose(test);
	//#endif

#ifdef DSCRM_4to1
	//方向補正値再計算
	pbs->insertion_direction = temp_way;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

}

/****************************************************************/
/**
* @brief		新重券（光学式）
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void ts_double_chk_nn(T_TEMPLATE_WORK* pst_work)
{
	T_DECISION_PARAMETER_DOUBLE_OPTICS* para = (T_DECISION_PARAMETER_DOUBLE_OPTICS*)pst_work->now_para_blk_pt;
	ST_DOUBLE_OPTICS_NN dbl_weights;
	ST_DOUBLE_OPTICS_NN_RESULT double_result = { 0 };
	T_POINTDT_MASK_INFO* p_mask;
	ST_DATA_EXTRACTION data_extraction_double;
	s16 ii = 0;
	u32 plane = 0;

	dbl_weights.weight = (float*)((u8*)para + para->offset_weight);
	dbl_weights.input_num = para->input_num;
	dbl_weights.hidden_num = para->hidden_num;
	dbl_weights.output_num = para->output_num;
	dbl_weights.biasflag = para->biasflag;
	dbl_weights.A_Label[0] = para->label[0];
	dbl_weights.A_Label[1] = para->label[1];
	dbl_weights.B_Label[0] = para->label[2];
	dbl_weights.B_Label[1] = para->label[3];
	dbl_weights.C_Label[0] = para->label[4];
	dbl_weights.C_Label[1] = para->label[5];
	dbl_weights.D_Label[0] = para->label[6];
	dbl_weights.D_Label[1] = para->label[7];

	//機能セット　コーナーは有効にする
	data_extraction_double.outline = 0;	// 外形検知の判定結果ブロック連番を指定する。0:外形基準  1:印刷枠基準(未実装)
	data_extraction_double.split_x = para->x_dev_num;		// 横分割数
	data_extraction_double.split_y = para->y_dev_num;		// 縦分割数
	data_extraction_double.corner_flg = 1;		// コーナー無効に　する:0/しない:1
	data_extraction_double.plane_count = para->num;				// プレーンの要素数

	//マスクの設定
	for (ii = 0; ii < data_extraction_double.plane_count; ii++)
	{
		p_mask = &para->mask_info[ii];
		data_extraction_double.plane_lst[ii] = p_mask->plane_no;      					// プレーンリスト

		//物理をスキャンする際、dpiが異なるので、プレーンごとにマスクパターンとサイズを持つこととする。
		data_extraction_double.filter_size_x[ii] = p_mask->mask_size_x;				// 横フィルターサイズ
		data_extraction_double.filter_size_y[ii] = p_mask->mask_size_y;				// 縦フィルターサイズ
		data_extraction_double.pfilter_pat[ii] = (u8*)(pst_work->now_para_blk_pt) + (p_mask->planes_lst_ofs);
		data_extraction_double.divide_val[ii] = (float)(1.0f / p_mask->devdt);
	}

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//座標系をリセットする
	reset_insart_multi_num(pst_work->buf_n);

	//反射センサ設定
	decision_plane_side_in_insertion_direction(pst_work->buf_n);

	for (ii = 0; ii < data_extraction_double.plane_count; ii++)
	{
		plane = data_extraction_double.plane_lst[ii];
		if (work[pst_work->buf_n].pbs->PlaneInfo[plane].Ref_or_Trans == REFLECTION)
	{
			data_extraction_double.plane_lst[ii] = (u8)work[pst_work->buf_n].note_param.pplane_tbl[plane];
		}
	}

	//ポイント抽出実行　コーナーは必ず使用する
	pst_work->e_code = Data_extraction(&data_extraction_double, pst_work->buf_n);

	//抽出したデータを方向ごとに並び替え
	convert_direction_data_juuken(work[pst_work->buf_n].pbs->insertion_direction, &data_extraction_double);

	//ポイントコーナー設定
	data_extraction_double.corner_flg = para->ei_corner;
	split_4parts_NN(&data_extraction_double, &dbl_weights);

	double_optics_nn_predict(&dbl_weights, &double_result);

	pst_work->proc_tm_co_e = get_10ns_co();

	// リセットした座標系を元に戻す
	set_dir_params(pst_work, 0);

	//結果ブロックに書き込む
	double_check_optical_result(pst_work->buf_n, &double_result);

}


/****************************************************************/
/**
* @brief		重券チェック
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_double_check(T_TEMPLATE_WORK* pst_work)
{
#ifdef VS_DEBUG_ICHIJO
	FILE* test;// ichijo 一時的
	long size;//

#endif

	s32 invalid_count = 0;

#ifdef DSCRM_4to1 
	u8 temp_way;
	ST_BS* pbs = work[pst_work->buf_n].pbs;
#endif

#ifdef STRACTURE_DEFINITION
	//メイン構造体を定義する
	ST_DOUBLE_CHECK dbl_chk;
#endif

	//判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_DOUBLE_CHECK* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_DOUBLE_CHECK*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	p_reference = (T_OUTLINE_RESULT*)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);

	//パラメタセット
	dbl_chk.red_mask_ptn_diameter_x = p_params->red_mask_ptn_diameter_x;
	dbl_chk.red_mask_ptn_diameter_y = p_params->red_mask_ptn_diameter_y;
	dbl_chk.red_mask_ptn_divide_num = p_params->red_mask_ptn_divide_num;
	dbl_chk.pred_mask_ptn = (u8*)((u8*)(p_params)+(p_params->red_mask_ptn_ofs));

	dbl_chk.tred_mask_ptn_diameter_x = p_params->tred_mask_ptn_diameter_x;
	dbl_chk.tred_mask_ptn_diameter_y = p_params->tred_mask_ptn_diameter_y;
	dbl_chk.tred_mask_ptn_divide_num = p_params->tred_mask_ptn_divide_num;
	dbl_chk.ptred_mask_ptn = (u8*)((u8*)(p_params)+(p_params->tred_mask_ptn_ofs));

	dbl_chk.note_size_x = p_reference->note_x_length;
	dbl_chk.note_size_y = p_reference->note_y_length;


	if (work[pst_work->buf_n].pbs->LEorSE == SE)
	{
		dbl_chk.note_size_y = p_reference->note_x_length;
		dbl_chk.note_size_x = p_reference->note_y_length;
	}


#ifdef DSCRM_4to1
	//方向補正を解除するために再度処理を行う。
	temp_way = pbs->insertion_direction;
	pbs->insertion_direction = 0;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif


	dbl_chk.threshold = p_params->thrshold;


	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//処理実行
	invalid_count = get_double_check_error(pst_work->buf_n, &dbl_chk);
	// 正券：-値　、重券：+値

	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得+

#ifdef NEW_DOUBLE_CHECK

	if (dbl_chk.level < 55)
	{
		if (dbl_chk.level > 40)
		{
			dbl_chk.level = 40;
		}

		pbs->mid_res_double_ck.result = UF;		//０未満の場合は重券
		pst_work->e_code = (u16)REJ_DOUBLE_NOTE;
		pbs->validation_result_flg |= 1 << VALIDATION_RES_DOUBLE_CHECK;
	}
	else
	{
		pbs->mid_res_double_ck.result = ATM;		//０未満の場合は正券

	}

#else

	if (invalid_count > 0)
	{
		pbs->mid_res_double_ck.result = UF;		//０未満の場合は重券
		pbs->mid_res_double_ck.level = MAX_LEVEL;
		pst_work->e_code = (u16)REJ_DOUBLE_NOTE;
		pbs->validation_result_flg |= 1 << VALIDATION_RES_DOUBLE_CHECK;
	}
	else
	{
		pbs->mid_res_double_ck.result = ATM;		//０未満の場合は正券
		pbs->mid_res_double_ck.level = MIN_LEVEL;
	}

#endif


#ifdef DSCRM_4to1
	//方向補正値再計算
	pbs->insertion_direction = temp_way;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

	//結果ブロックに書き込む
	double_check_result(pst_work->buf_n, (s16)invalid_count, dbl_chk.level);
}



/****************************************************************/
/**
* @brief		 重券検知（メカ厚）
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_double_check_mecha(T_TEMPLATE_WORK* pst_work)
{
	ST_BS* pbs = work[pst_work->buf_n].pbs;
	//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_DBL_CHK_MECHA st_dbl_chack_mecha;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	T_DECISION_PARAMETER_DBL_CHK_MECHA* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)


	// 判定ブロックからパラメタを取得 ※
	p_params = (T_DECISION_PARAMETER_DBL_CHK_MECHA*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) 
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット ※
	st_dbl_chack_mecha.double_check_threshold = p_params->double_check_threshold;	// 重券検出厚み閾値
	st_dbl_chack_mecha.double_area_ratio = (u8)p_params->double_area_ratio;				// 面積比率閾値(%)
	st_dbl_chack_mecha.bill_check_threshold = p_params->bill_check_threshold;		// 紙幣検出厚み閾値
	st_dbl_chack_mecha.exclude_length = (u8)p_params->exclude_length;					// 先頭判定除外範囲(mm)

#ifdef _RM_		/* シミュレータと実機を切りかえる */
#else
#endif

//#ifdef VS_DEBUG_ITOOL
//	for_Dbg_start_algo_label_setting(ALGO_TAPE_MECHA_TC, dbg_codi_Physical);		// デバッグ用コールバック
//#endif

	//************************************************************
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	double_check_mecha(pst_work->buf_n, &st_dbl_chack_mecha);									// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	if (st_dbl_chack_mecha.result != 0)						//重券判定の場合
	{
		//pbs->mid_res_dbl_ck_mecha.result = UF;
		//pbs->mid_res_dbl_ck_mecha.level  = MAX_LEVEL;	
		pst_work->e_code = (u16)REJ_DOUBLE_NOTE;			//リザルトコードにセットする。
		pbs->validation_result_flg |= 1 << VALIDATION_RES_DOUBLE_CHECK;
	}
	//else
	//{
	//	pbs->mid_res_double_ck.result = ATM;		//０未満の場合は正券
	//	pbs->mid_res_double_ck.level  = MIN_LEVEL;	
	//}


	//結果ブロックに書き込む
	double_check_mecha_result(pst_work->buf_n, st_dbl_chack_mecha);


}

/****************************************************************/
/**
* @brief		鑑別１色NN
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_neural_network_1color(T_TEMPLATE_WORK* pst_work)
{
#ifdef VS_DEBUG_ICHIJO
	FILE* test;// ichijo 一時的
	long size;//
	u32 invalid = 0;
#endif

	s32 res = 0;
	float flevel_standard_value;
	float valres;

	u8 level[3];

#ifdef DSCRM_4to1 
	u8 temp_way;
	ST_BS* pbs = work[pst_work->buf_n].pbs;
#endif

#ifdef STRACTURE_DEFINITION
	//メイン構造体を定義する
	ST_VALI_NN1 nn1;
#endif

	//判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_NN1* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	//T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_NN1*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->comn.function_code, pst_work->p_result_blk_top);

	//パラメタセット
	nn1.ppoint = (ST_NN_POINTS*)((u8*)(p_params)+(p_params->input_prm_ofs));

	nn1.ir1_mask_ptn_diameter_x = p_params->ir1_mask_ptn_diameter_x;
	nn1.ir1_mask_ptn_diameter_y = p_params->ir1_mask_ptn_diameter_y;
	nn1.ir1_mask_ptn_divide_num = p_params->ir1_mask_ptn_divide_num;
	nn1.pir1_mask_ptn = (u8*)((u8*)(p_params)+p_params->ir1_mask_ptn_ofs);

	nn1.in_node_count = p_params->NN_in_num;
	nn1.hidden_node_count = (u8)p_params->NN_midle_num;
	nn1.out_node_count = (u8)p_params->NN_out_num;

	nn1.phidden_weight = (float*)((u8*)(p_params)+p_params->hidden_weight_dt_ofs);
	nn1.pout_weight = (float*)((u8*)(p_params)+p_params->out_weight_dt_ofs);

#ifdef DSCRM_4to1
	//方向補正を解除するために再度処理を行う。
	temp_way = pbs->insertion_direction;
	pbs->insertion_direction = 0;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_NN1, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_CONVENTIONAL_VALIDATION
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//処理実行
	res = predict_neural_network_1color(pst_work->buf_n, &nn1);

	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	pst_work->e_code = (u16)0;

#ifdef DSCRM_4to1
	//方向補正値再計算
	pbs->insertion_direction = temp_way;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif


#ifdef VS_DEBUG_CONVENTIONAL_VALIDATION
	debug_logi_view = 0;
#endif

	//レベルを計算
	if (p_params->level_standard_value_1 == 255 || p_params->level_standard_value_1 == 0)
	{
		flevel_standard_value = 0.65f;
	}
	else
	{
		flevel_standard_value = p_params->level_standard_value_1 * 0.01f;
	}
	level[0] = level_detect(&nn1.counterfeit_out_put_val, &flevel_standard_value, 1, 0, 1);


	//レベルを計算
	if (p_params->level_standard_value_2 == 255 || p_params->level_standard_value_2 == 0)
	{
		flevel_standard_value = 0.3f;
	}
	else
	{
		flevel_standard_value = p_params->level_standard_value_2 * 0.01f;
	}
	valres = nn1.counterfeit_out_put_val - nn1.genuine_out_put_val;
	level[1] = level_detect(&valres, &flevel_standard_value, 1, 0, 1);

	//大きい方の値を設定
	if (level[0] > level[1])
	{
		level[2] = level[1];
	}
	else
	{
		level[2] = level[0];
	}

	//結果ブロックに書き込む
	nn1_result(pst_work->buf_n, (u8)res, nn1.genuine_out_put_val, nn1.counterfeit_out_put_val, level[2]);

	//#ifdef VS_DEBUG_ICHIJO
	//// ichijo 一時的
	//test = fopen("simulation_resul_nn1.csv", "a");
	//fseek(test, 0, SEEK_END);
	//size = ftell(test);
	//if (size == 0)
	//{
	//	fprintf(test, "FileNum, Temp_ID, Way, genuine_out_put_val, counterfeit_out_put_val, invalid\n");
	//}
	//if (nn1.genuine_out_put_val - nn1.counterfeit_out_put_val > 0.0)
	//{
	//	invalid = 0;
	//}
	//else
	//{
	//	invalid = 1;
	//}
	//fprintf(test, "%d, %d, %d, %f, %f, %d,\n", (u32)pst_work->buf_n, (u32)pst_work->ID_template, (u32)temp_way, nn1.genuine_out_put_val, nn1.counterfeit_out_put_val, invalid);
	//fclose(test);
	//#endif

}

/****************************************************************/
/**
* @brief		鑑別2色NN
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_neural_network_2color(T_TEMPLATE_WORK* pst_work)
{
#ifdef VS_DEBUG_ICHIJO
	FILE* test;// ichijo 一時的
	//long size;//
	u32 invalid = 0;
#endif

	s32 res = 0;
	float flevel_standard_value;
	float valres;
	u8 level[3];

#ifdef DSCRM_4to1 
	u8 temp_way;
	ST_BS* pbs = work[pst_work->buf_n].pbs;
#endif

#ifdef STRACTURE_DEFINITION
	//メイン構造体を定義する
	ST_VALI_NN2 nn2;
#endif

	//判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_NN2* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	//T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_NN2*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, pst_work->p_result_blk_top);

	//パラメタセット
	nn2.ppoint = (ST_NN_POINTS*)((u8*)(p_params)+(p_params->input_prm_ofs));

	nn2.ir1_mask_ptn_diameter_x = p_params->ir1_mask_ptn_diameter_x;
	nn2.ir1_mask_ptn_diameter_y = p_params->ir1_mask_ptn_diameter_y;
	nn2.ir1_mask_ptn_divide_num = p_params->ir1_mask_ptn_divide_num;
	nn2.pir1_mask_ptn = (u8*)((u8*)(p_params)+p_params->ir1_mask_ptn_ofs);

	nn2.red_mask_ptn_diameter_x = p_params->red_mask_ptn_diameter_x;
	nn2.red_mask_ptn_diameter_y = p_params->red_mask_ptn_diameter_y;
	nn2.red_mask_ptn_divide_num = p_params->red_mask_ptn_divide_num;
	nn2.pred_mask_ptn = (u8*)((u8*)(p_params)+p_params->red_mask_ptn_ofs);

	nn2.in_node_count = p_params->NN_in_num;
	nn2.hidden_node_count = (u8)p_params->NN_midle_num;
	nn2.out_node_count = (u8)p_params->NN_out_num;

	nn2.phidden_weight = (float*)((u8*)(p_params)+p_params->hidden_weight_dt_ofs);
	nn2.pout_weight = (float*)((u8*)(p_params)+p_params->out_weight_dt_ofs);

#ifdef DSCRM_4to1
	//方向補正を解除するために再度処理を行う。
	temp_way = pbs->insertion_direction;
	pbs->insertion_direction = 0;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_NN2, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_CONVENTIONAL_VALIDATION
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//処理実行
	res = predict_neural_network_2color(pst_work->buf_n, &nn2);

	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	pst_work->e_code = (u16)0;

#ifdef DSCRM_4to1
	//方向補正値再計算
	pbs->insertion_direction = temp_way;
	advance_insert_correction(pst_work->buf_n);
	decision_plane_side_in_insertion_direction(pst_work->buf_n);
#endif


#ifdef VS_DEBUG_CONVENTIONAL_VALIDATION
	debug_logi_view = 0;
#endif

	//レベルを計算
	if (p_params->level_standard_value_1 == 255 || p_params->level_standard_value_1 == 0)
	{
		flevel_standard_value = 0.65f;
	}
	else
	{
		flevel_standard_value = p_params->level_standard_value_1 * 0.01f;
	}
	level[0] = level_detect(&nn2.counterfeit_out_put_val, &flevel_standard_value, 1, 0, 1);


	//レベルを計算
	if (p_params->level_standard_value_2 == 255 || p_params->level_standard_value_2 == 0)
	{
		flevel_standard_value = 0.3f;
	}
	else
	{
		flevel_standard_value = p_params->level_standard_value_2 * 0.01f;
	}
	valres = nn2.counterfeit_out_put_val - nn2.genuine_out_put_val;
	level[1] = level_detect(&valres, &flevel_standard_value, 1, 0, 1);


	if (level[0] > level[1])
	{
		level[2] = level[1];
	}
	else
	{
		level[2] = level[0];
	}

	//結果ブロックに書き込む
	nn2_result(pst_work->buf_n, (u8)res, nn2.genuine_out_put_val, nn2.counterfeit_out_put_val, level[2]);

	//#ifdef VS_DEBUG_ICHIJO
	//// ichijo 一時的
	//test = fopen("simulation_resul_nn2.csv", "a");
	//fseek(test, 0, SEEK_END);
	//size = ftell(test);
	//if (size == 0)
	//{
	//	fprintf(test, "FileNum, Way, genuine_out_put_val, counterfeit_out_put_val, invalid,\n");
	//}
	//if (nn2.genuine_out_put_val - nn2.counterfeit_out_put_val > 0.0)
	//{
	//	invalid = 0;
	//}
	//else
	//{
	//	invalid = 1;
	//}
	//fprintf(test, "%d, %d, %f, %f, %d,\n", (u32)pst_work->buf_n, (u32)temp_way, nn2.genuine_out_put_val, nn2.counterfeit_out_put_val, invalid);
	//fclose(test);
	//#endif
}

/****************************************************************/
/**
* @brief		IR2波長
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_ir2wave(T_TEMPLATE_WORK* pst_work)
{
	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_IR2WAVE ir2wave = { 0 };
#endif

	//判定パラメタ構造体
	T_DECISION_PARAMETER_IR2WAVE* p_params;

	p_params = (T_DECISION_PARAMETER_IR2WAVE*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//パラメタセット
	ir2wave.ppoint = (ST_IMUF_POITNS*)((u8*)(p_params)+(p_params->param_dt_ofs));
	ir2wave.ir1_mask_ptn_diameter_x = 25;//p_params->ir1_mask_ptn_diameter_x;//固定
	ir2wave.ir1_mask_ptn_diameter_y = 25;//p_params->ir1_mask_ptn_diameter_y;//固定
	ir2wave.ir1_mask_ptn_divide_num = 1.0f / (float)441.0f;// p_params->ir1_mask_ptn_divide_num;//固定

	ir2wave.point_number = (u8)p_params->param_dt_num;

#ifdef VS_DEBUG_IR2WAVE
	debug_logi_view = 1;
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_IR2_SICPA, dbg_codi_Logic);		// デバッグ用コールバック
#endif

	//処理実行
	pst_work->proc_tm_co_s = get_10ns_co();
	get_ir2wave_invalid_count(pst_work->buf_n, &ir2wave);
	pst_work->proc_tm_co_e = get_10ns_co();

#ifdef VS_DEBUG_IR2WAVE
	debug_logi_view = 0;
#endif

	ir2wave_result(pst_work->buf_n, ir2wave);
}

/****************************************************************/
/**
* @brief		MAG
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_mag(T_TEMPLATE_WORK* pst_work)
{
	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_MAG mag = { 0 };
#endif

	//判定パラメタ構造体
	T_DECISION_PARAMETER_MAG* p_params;

	p_params = (T_DECISION_PARAMETER_MAG*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//パラメタセット
	mag.ppoint = (ST_IMUF_POITNS*)((u8*)(p_params)+(p_params->param_dt_ofs));
	mag.mag_mask_ptn_diameter_x = 25;//p_params->ir1_mask_ptn_diameter_x;//固定
	mag.mag_mask_ptn_diameter_y = 4;//p_params->ir1_mask_ptn_diameter_y;//固定
	mag.mag_mask_ptn_divide_num = 1.0f / (float)100.0f;// p_params->ir1_mask_ptn_divide_num;//固定

	mag.point_number = (u8)p_params->param_dt_num;


#ifdef VS_DEBUG_MAG
	debug_logi_view = 1;
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_MAG, dbg_codi_Logic);		// デバッグ用コールバック
#endif

	//処理実行
	pst_work->proc_tm_co_s = get_10ns_co();
	get_mag_valid_count(pst_work->buf_n, &mag);
	pst_work->proc_tm_co_e = get_10ns_co();

#ifdef VS_DEBUG_MAG
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	mag_result(pst_work->buf_n, mag);
}

/****************************************************************/
/**
* @brief		UV識別※蛍光反応がない金種のみ使用する
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_uv_fitness(T_TEMPLATE_WORK* pst_work)
{
	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_UV_VALIDATE uv = { 0 };
#endif

	//判定パラメタ構造体
	T_DECISION_PARAMETER_UV* p_params;

	p_params = (T_DECISION_PARAMETER_UV*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//パラメタセット
	uv.ppoint = (ST_UV_POINTS*)((u8*)(p_params)+(p_params->param_dt_ofs));
	uv.uv_mask_ptn_diameter_x = 25;//p_params->ir1_mask_ptn_diameter_x;//固定
	uv.uv_mask_ptn_diameter_y = 7;//p_params->ir1_mask_ptn_diameter_y;//固定
	uv.uv_mask_ptn_divide_num = 1.0f / 139.0f;// p_params->ir1_mask_ptn_divide_num;//固定


#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_UV_VALIDATE, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_UV_FITNESS
	debug_logi_view = 1;
#endif

	//処理実行
	pst_work->proc_tm_co_s = get_10ns_co();
	get_uv_fitness(pst_work->buf_n, &uv);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_UV_FITNESS
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	uv_result(pst_work->buf_n, uv);
}

/****************************************************************/
/**
* @brief		UV鑑別※蛍光反応がある金種のみ使用する
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_uv_validate(T_TEMPLATE_WORK* pst_work)
{
	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_UV_VALIDATE uv = { 0 };
#endif

	//判定パラメタ構造体
	T_DECISION_PARAMETER_UV* p_params;

	p_params = (T_DECISION_PARAMETER_UV*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//パラメタセット
	uv.ppoint = (ST_UV_POINTS*)((u8*)(p_params)+(p_params->param_dt_ofs));
	uv.uv_mask_ptn_diameter_x = 25;//p_params->ir1_mask_ptn_diameter_x;//固定
	uv.uv_mask_ptn_diameter_y = 7;//p_params->ir1_mask_ptn_diameter_y;//固定
	uv.uv_mask_ptn_divide_num = 1.0f / 139.0f;// p_params->ir1_mask_ptn_divide_num;//固定


#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_WASHED, dbg_codi_Logic);		// デバッグ用コールバック
#endif


#ifdef VS_DEBUG_UV_VALI
	debug_logi_view = 1;
#endif
	//処理実行
	pst_work->proc_tm_co_s = get_10ns_co();
	get_uv_validate(pst_work->buf_n, &uv);
	pst_work->proc_tm_co_e = get_10ns_co();

#ifdef VS_DEBUG_UV_VALI
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	uv_validate_result(pst_work->buf_n, uv);
}


/****************************************************************/
/**
* @brief		角折れ
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_dog_ear(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

	//関数専用の構造体を定義する
#ifdef STRACTURE_DEFINITION
	ST_DOG_EAR st_dog;
#endif

	//関数専用の判定パラメタ構造体を定義する
	ST_T_DOG_EAR* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	//T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得
	p_params = (ST_T_DOG_EAR*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット
	st_dog.threshold_area = p_params->threshold_area;						//基準面積	a
	st_dog.threshold_short_side_mm = p_params->threshold_short_side_dot;	//短手の許容範囲
	st_dog.threshold_long_side_mm = p_params->threshold_long_side_dot;		//長手の許容範囲
	st_dog.comp_flg = p_params->comp_flg;									//基準比較方法　0:面積と短手 1:長手と短手

#if 0
	dog.scan_move_y = 4;					//頂点を求めるときのy座標の移動量
	dog.scan_move_x = 9;					//ｘ座標が戻る量	小さすぎると斜め欠損券の場合に誤検知につながる
	dog.scan_move_x_thr_line = 1;			//頂点＋この数字で閾値
	dog.scan_minimum_points = 5;			//このポイント以下ならば角折れなし
	dog.tear_scan_renge = 4;				//破れ検知の際、どれだけ探索すれば破れと判断するか
	dog.tear_scan_start_x = 4;				//裂け検知何dotピッチでスキャンするか
	dog.garbage_or_note_range = 5;			//ポイント採取の際ぶつかったものが媒体かごみかどうかを調べる範囲
#endif
	st_dog.scan_move_y = p_params->scan_move_y;					//頂点を求めるときのy座標の移動量
	st_dog.scan_move_x = p_params->scan_move_x;					//ｘ座標が戻る量	小さすぎると斜め欠損券の場合に誤検知につながる
	st_dog.scan_move_x_thr_line = p_params->scan_move_x_thr_line;	//頂点＋この数字で閾値
	st_dog.scan_minimum_points = p_params->scan_minimum_points;	//このポイント以下ならば角折れなし
	st_dog.tear_scan_renge = p_params->tear_scan_renge;			//破れ検知の際、どれだけ探索すれば破れと判断するか
	st_dog.tear_scan_start_x = p_params->tear_scan_start_x;		//裂け検知何dotピッチでスキャンするか
	st_dog.garbage_or_note_range = p_params->garbage_or_note_range;//ポイント採取の際ぶつかったものが媒体かごみかどうかを調べる範囲

	st_dog.qp = p_params->histogram_qp_param;						//ヒストグラム量子化パラメタ
	st_dog.peak_width = p_params->histogram_peak_width;			//ピークの幅　例えばこの値が1でピークが１５ならば１４と１６も許容するということ
	st_dog.peak_margin = p_params->histogram_2nd_peak_thr;			//第2ピークとみなすときの値の基準。　第一ピーク/この値が閾値になる。

	st_dog.not_seen_area_count = p_params->not_seen_area_count;	//見ないエリアの数
	st_dog.plane = p_params->plane;					//プレーン

	//素子ピッチ
	st_dog.ele_pitch_x = 25.4f / LOGICAL_COORDINATE_DPI;	//ｘ
	st_dog.ele_pitch_y = 25.4f / LOGICAL_COORDINATE_DPI;	//y


	memcpy(&st_dog.not_seen_areas, (u8*)((u8*)(p_params)+p_params->param_dt_ofs), (u64)2 * (u32)4 * (u32)st_dog.not_seen_area_count);

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_DOG_EARS, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_DOGEAR
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	res = dog_ear(pst_work->buf_n, &st_dog);							// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_DOGEAR
	debug_logi_view = 0;
#endif

	//st_dog.judge;

	res = (u32)res;
	//pst_work->e_code = (u16)res;

	//結果ブロックに書き込む
	dog_ear_result(pst_work->buf_n, st_dog);


}

/****************************************************************/
/**
* @brief		裂け
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_tear(T_TEMPLATE_WORK* pst_work)
{
	//u32 res = 0;
	u8 i = 0;

	//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_TEAR st_tr;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_TEAR* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい) ※
	//T_OUTLINE_RESULT* p_reference_outline;
	T_DOG_EAR_RESULT* p_reference_dog;

	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_TEAR*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) ※
	//p_reference_outline = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num1, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);
	p_reference_dog = (T_DOG_EAR_RESULT*)search_blk_address_of_seq_num(p_params->reference_blk_num2, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);

	//パラメタセット ※
	//使うプレーンの設定
	st_tr.plane = p_params->plane;

	if (work[pst_work->buf_n].pbs->PlaneInfo[st_tr.plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
	{
		st_tr.plane = st_tr.plane + 7;			 //用いるプレーン指定
		//ED.Plane = DOWN_T_R;			 //用いるプレーン指定
	}

	st_tr.width_detection = p_params->width_detection;				//裂け幅検知を実行するかしないか	 1: する　０：しない
	st_tr.total_depth_judge = p_params->total_depth_judge;			//裂けの深さの合計値でUFの判定を行う 1: する　０：しない

	st_tr.threshold_width = p_params->threshold_width;			//検知すべき裂けの幅　		　単位㎜で指定する。
	st_tr.threshold_vertical_depth = p_params->threshold_vertical_depth;	//検知すべき裂けの深さ(垂直)　単位㎜で指定する。
	st_tr.threshold_horizontal_depth = p_params->threshold_horizontal_depth;	//検知すべき裂けの深さ(水平)　単位㎜で指定する。
	st_tr.threshold_diagonal_depth = p_params->threshold_diagonal_depth;	//検知すべき裂けの深さ(斜め)　単位㎜で指定する。

	//************************************************************

	//0：左上　１：左下　２：右上　３：右下
	for (i = 0; i < 4; i++)
	{
		st_tr.corner_triangle_vertex_1[i][0] = 0x0fff;
		st_tr.corner_triangle_vertex_1[i][1] = 0x0fff;
		st_tr.corner_triangle_vertex_2[i][0] = 0x0fff;
		st_tr.corner_triangle_vertex_2[i][1] = 0x0fff;

		if ((p_reference_dog->judge[i] == 2 || p_reference_dog->judge[i] == 1))
		{
			st_tr.corner_triangle_vertex_1[i][0] = p_reference_dog->triangle_vertex_x[i][0];
			st_tr.corner_triangle_vertex_1[i][1] = p_reference_dog->triangle_vertex_y[i][0];

			st_tr.corner_triangle_vertex_2[i][0] = p_reference_dog->triangle_vertex_x[i][1];
			st_tr.corner_triangle_vertex_2[i][1] = p_reference_dog->triangle_vertex_y[i][1];
		}

		st_tr.dog_ear_res[i] = p_reference_dog->judge[i];
	}

	st_tr.not_seen_area_count = 0;


#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_TEAR, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_TEAR
	debug_logi_view = 1;
#endif
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	tear(pst_work->buf_n, &st_tr);									// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得
#ifdef VS_DEBUG_TEAR
	debug_logi_view = 0;
#endif

	//res = (u32)st_tr.res_judge_tear;							//エラーセット ※
	//pst_work->e_code = (u16)res;

	//結果ブロックに書き込む
	tear_result(pst_work->buf_n, st_tr);


}

/****************************************************************/
/**
* @brief		ダイノート
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_dye_note(T_TEMPLATE_WORK* pst_work)
{
	//u32 res = 0;
	//u8 i=0;

	//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_DYE_NOTE st_dy;
#endif


	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_DYENOTE* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_DYENOTE*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) 
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);
	p_reference = (T_OUTLINE_RESULT*)search_blk_address_of_seq_num(1, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);
	//パラメタセット ※
	st_dy.ratio = p_params->ink_area_thr;					//インク面積の閾値	
	st_dy.scan_pitch = p_params->scan_interval;				//スキャン間隔

	st_dy.note_corner_margin = p_params->not_scan_area;		//外周の使用不可エリアの範囲
	//st_dy.note_corner_margin = 1;

	st_dy.thr2 = p_params->old_ink_thr;						//旧インクの閾値
	st_dy.thr = p_params->new_ink_thr;						//新インクの閾値

	////itoolの不具合で今は直でパラメタを入れる
	//st_dy.ratio = 6;					//インク面積の閾値	
	//st_dy.scan_pitch = 64;				//スキャン間隔
	//st_dy.note_corner_margin = 10;		//外周の使用不可エリアの範囲
	//st_dy.thr2 = 5.0f;//p_params->old_ink_thr;						//旧インクの閾値
	//st_dy.thr = 6.0f;//p_params->new_ink_thr;						//新インクの閾値

	st_dy.note_size_x = p_reference->note_x_length;
	st_dy.note_size_y = p_reference->note_y_length;

	//************************************************************
#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_DYENOTE, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_DYENOTE
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	dye_note(pst_work->buf_n, &st_dy);						// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_DYENOTE
	debug_logi_view = 0;
#endif

	//if( pst_work->e_code != REJ_NEW_DYE_INK  && pst_work->e_code != REJ_OLD_DYE_INK)	//ダイノート以外ならば

	//結果ブロックに書き込む
	dyenote_result(pst_work->buf_n, st_dy);

	if (work[pst_work->buf_n].pbs->fitness[DYE_NOTE_].bit.result == UF)
	{
		pst_work->e_code = (u16)st_dy.rej_code;
	}


}

/****************************************************************/
/**
* @brief		汚れ
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_soiling(T_TEMPLATE_WORK* pst_work)
{
	//u32 res = 0;
	//u8 i=0;
	//u8 denomi_id = 0xff;
	//ST_INFERENCE_RESULT ress;

	//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_SOILING st_soi;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_SOILING* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)


	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_SOILING*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) 
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット ※
	st_soi.area_num = p_params->referemce_area_count;	//参照エリア数
	st_soi.params = (ST_SOILING_EACH_AREA_PARAMS*)((u8*)(p_params)+(p_params->reference_area_inf_address));	//参照エリア情報
	st_soi.comparison_method = p_params->comparison_method;
	st_soi.select_color_mode = p_params->select_color_mode;
	//st_soi.select_color_mode = 0;

	//st_soi.st_1_level_num = p_params->fit_normal_vector_distance;	//fit券群と平面との距離	
	//st_soi.uf_1_level_num = p_params->uf_normal_vector_distance;

	//st_soi.st_1_level_num = 0.01561f;	//fit券群と平面との距離	
	//st_soi.uf_1_level_num = 0.00351f;


	//************************************************************

	/*
	st_soi.params[0].sampling_pich_x = 2;
	st_soi.params[0].sampling_pich_y = 2;
	st_soi.params[1].sampling_pich_x = 2;
	st_soi.params[1].sampling_pich_y = 2;
	*/

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_SOILING, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_SOILING
	debug_logi_view = 1;
#endif

#if 0

	if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010001)		//1.2
	{
		denomi_id = 0;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x0001000D)	//1.3
	{
		denomi_id = 1;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010003)	//5.2
	{
		denomi_id = 2;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010002)	//5.3
	{
		denomi_id = 3;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00011207)	//5.4
	{
		denomi_id = 4;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010005)	//10.2
	{
		denomi_id = 5;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010004)	//10.3
	{
		denomi_id = 6;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x0001000E)	//10.5
	{
		denomi_id = 7;
	}

	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010007)	//20.1
	{
		denomi_id = 8;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010006)	//20.2
	{
		denomi_id = 9;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x0001000F)	//20.3
	{
		denomi_id = 10;
	}

	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010009)	//50.1
	{
		denomi_id = 11;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010008)	//50.3
	{
		denomi_id = 12;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x00010011)	//50.5
	{
		denomi_id = 13;
	}

	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x0001000A)	//100.1
	{
		denomi_id = 14;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x0001000B)	//100.3
	{
		denomi_id = 15;
	}
	else if (work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id == 0x0001000C)	//100.4
	{
		denomi_id = 16;
	}

	// add by VnHoan 2022/02/11 for all denomi
	//Cny1.2, Cny5.3, Cny10.3, Cny20.2, Cny50.3, Cny100.3,
	if (denomi_id == 15 || denomi_id == 9 || denomi_id == 6 || denomi_id == 0 || denomi_id == 3 || denomi_id == 12)
	{
		pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
		st_soi.output_level = (u16)soiling_ver4(pst_work->buf_n, denomi_id, &ress);
		pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得
	}
	else
	{
		pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
		soiling(pst_work->buf_n, &st_soi);						// 処理実行　 ※
		pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得
	}

#else
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	soiling(pst_work->buf_n, &st_soi);						// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得
#endif


#ifdef VS_DEBUG_SOILING
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	soiling_result(pst_work->buf_n, st_soi);

}


/****************************************************************/
/**
* @brief		脱色
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_de_ink(T_TEMPLATE_WORK* pst_work)
{
	//u32 res = 0;
	//u8 i=0;

	//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_DEINKED st_deink;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_DEINK* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)


	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_DEINK*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) 
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット ※
	st_deink.area_num = p_params->referemce_area_count;															//参照エリア数
	st_deink.params = (ST_DEINK_EACH_AREA_PARAMS*)((u8*)(p_params)+(p_params->reference_area_inf_address));	//参照エリア情報
	st_deink.comparison_method = p_params->comparison_method;
	st_deink.select_color_mode = p_params->select_color_mode;
	//st_deink.select_color_mode = 0;
	//************************************************************

//	st_deink.params[0].sampling_pich_x = 4;
//	st_deink.params[0].sampling_pich_y = 4;

//	st_deink.params[0].start_x = -405;
//	st_deink.params[0].end_x = -405+119;

//	st_deink.params[0].start_y = 225;
//	st_deink.params[0].end_y = 235-78;

//	st_deink.params[1].sampling_pich_x = 2;
//	st_deink.params[1].sampling_pich_y = 2;
//	st_deink.params[2].sampling_pich_x = 2;
//	st_deink.params[2].sampling_pich_y = 2;

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_DEINKD, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_DE_INKD
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	de_inked_note(pst_work->buf_n, &st_deink);									// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_DE_INKD
	debug_logi_view = 0;
#endif


	//res = (u32)st_soi.;							//エラーセット ※
	//pst_work->e_code = (u16)st_dy.judge;

	//結果ブロックに書き込む
	deink_result(pst_work->buf_n, st_deink);

}

/****************************************************************/
/**
* @brief		染み
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_stain(T_TEMPLATE_WORK* pst_work)
{
#if 0
	//u32 res = 0;

	//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_STAIN st_sta;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_STAIN* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	//T_OUTLINE_RESULT* p_reference;

	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_STAIN*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) 
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);


	//パラメタセット ※
	st_sta.plane_num = 2;								//用いるプレーン数　固定値

	st_sta.plane[0] = p_params->plane_omote;			//用いる表面のプレーン番号
	st_sta.plane[1] = p_params->plane_ura;				//用いる裏面のプレーン番号
	st_sta.conter_not_scan_range = p_params->conter_not_scan_range;	//外周からの除外範囲指定
	st_sta.min_thr_val = p_params->thr_lower;				//閾値の下限
	st_sta.comparison_method = p_params->comp_flg;				//基準比較方法
	st_sta.repeat_num = p_params->repeat_num;				//再スキャン回数
	st_sta.thr_adjust_val = p_params->thr_adjust_val;			//閾値調節値 +だと真っ黒しか検知しない　-だとその逆
	//st_sta.thr_adjust_val	= 0;			//閾値調節値

	st_sta.x_split = p_params->x_split;				//分割数ｘ
	st_sta.y_split = p_params->y_split;				//ｙ

	st_sta.stain_size_thr = p_params->stain_size_thr;			//1つの染みの面積の閾値 単位 mm^2

	st_sta.total_stain_thr = p_params->total_stain_thr;		//染みの合計面積の閾値 単位 mm^2
	st_sta.stain_diameter_thr = p_params->stain_diameter_thr;		//染みの直径の閾値 単位 mm
	st_sta.mask_size_x = p_params->mask_size_x;			//マスクサイズｘ
	st_sta.mask_size_y = p_params->mask_size_y;			//マスクサイズｙ
	st_sta.divide_val = p_params->divide_val;				//乗数

	//マスキングエリアをテンプレートシーケンスのワークメモリから取得
	st_sta.side_masking_area = pst_work->side_masking_area;
	st_sta.vertical_masking_area = pst_work->vertical_masking_area;

	st_sta.p_mask = (u8*)((u8*)(p_params)+(p_params->mask_ofs));			// マスクパターンの配列先頭オフセット
	//st_sta.p_mask = stain_mask;
	//st_sta. = p_params->mask_num;											// マスクパターンの要素数
	st_sta.thr[0] = (u8*)((u8*)(p_params)+(p_params->omote_thr_ofs));		// 表面閾値配列の先頭オフセット
	//st_sta. = p_params->omote_thr_num;									// 表面閾値配列の要素数
	st_sta.thr[1] = (u8*)((u8*)(p_params)+(p_params->ura_thr_ofs));		// 裏面閾値配列の先頭オフセット
	//st_sta. = p_params->ura_thr_num;										// 裏面閾値配列の要素数

	//************************************************************
#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_STAIN, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_STAIN
	debug_logi_view = 1;
#endif
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	stain(pst_work->buf_n, &st_sta);									// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_STAIN
	debug_logi_view = 0;
#endif
	//res = (u32)st_soi.;							//エラーセット ※
	//pst_work->e_code = (u16)st_dy.judge;

	//結果ブロックに書き込む
	stain_result(pst_work->buf_n, st_sta);
#endif
}

/****************************************************************/
/**
* @brief		穴
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_hole(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

#ifdef STRACTURE_DEFINITION
	//関数専用の構造体を定義する
	ST_HOLE st_hole = { 0 };
#endif

	//関数専用の判定パラメタ構造体を定義する
	ST_T_HOLE* p_params;

	// 判定ブロックからパラメタを取得
	p_params = (ST_T_HOLE*)pst_work->now_para_blk_pt;

	//パラメタセット
	st_hole.t_plane = p_params->t_plane;
	st_hole.threshold_level_area = p_params->threshold_level_area;
	st_hole.edge_margin = p_params->edge_margin;
	st_hole.x_step = p_params->x_step;
	st_hole.y_step = p_params->y_step;
	st_hole.threshold = p_params->threshold;
	st_hole.threshold_min_area = p_params->threshold_min_area;
	st_hole.threshold_total_area = p_params->threshold_total_area;
	st_hole.exclude_area_count = p_params->exclude_area_count;
	st_hole.skip = p_params->skip;

	memcpy(&st_hole.exclude_hole, (u8*)p_params + p_params->exclude_areas_ofs, sizeof(s16) * 4 * st_hole.exclude_area_count);

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_HOLE, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_HOLE
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	res = hole(pst_work->buf_n, &st_hole);							// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_HOLE
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	hole_result(pst_work->buf_n, st_hole);

}

/****************************************************************/
/**
* @brief		折り畳み
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_folding(T_TEMPLATE_WORK* pst_work)
{
	u16 x_sz = 0; // 基準との差（長手）、1mm刻みで小数点はすべて切り落とし
	u16 y_sz = 0; // 基準との差（短手）、1mm刻みで小数点はすべて切り落とし
	u16 x_limit = 0; // 紙幣の基準サイズ（長手）
	u16 y_limit = 0; // 紙幣の基準サイズ（短手）
	u16 i = 0; // カウンタ
	u16 code = 0; // 券種ID計算用
	u16 constant = 10; // サイズ計算用の定数
	//メイン構造体
#ifdef STRACTURE_DEFINITION
	ST_FOLDING folding = { 0 };
#endif

	//判定パラメタ構造体
	ST_T_FOLDING* p_params;

	// 基準サイズの構造体
	T_TEMP_SHELL_JCMID_TBL* id_tbl;
	id_tbl = (T_TEMP_SHELL_JCMID_TBL*)p_temp_currenct_table_top.tbl_ofs;

	code = pst_work->ID_template & 0x0fff;
	for (i = 0; i < p_temp_currenct_table_top.tbl_num; ++i)
	{
		if (id_tbl[i].template_id == code)
		{
			break;
		}
	}

	x_limit = (u16)id_tbl[i].note_size_x_mm * (u16)constant;
	y_limit = (u16)id_tbl[i].note_size_y_mm * (u16)constant;

	folding.folded_level[0] = (u8)100;
	folding.folded_level[1] = (u8)100;
	folding.folded_level[2] = (u8)100;
	folding.folded_level[3] = (u8)100;

	// 0：サイズが正常な時（折り畳み検知関数内でフラグとして使用し、不要な計算を避ける）
	if (size_limit.x_size >= x_limit)
	{
		x_sz = 0;
	}
	// 実測値が本来の長さよりも小さい場合入る：長手
	else
	{
		// 小数点以下四捨五入
		x_sz = (u16)((x_limit + 5) * 0.1) - (u16)((size_limit.x_size + 5) * 0.1);
	}
	// 0：サイズが正常な時（折り畳み検知関数内でフラグとして使用し、不要な計算を避ける）
	if (size_limit.y_size >= y_limit)
	{
		y_sz = 0;
	}
	// 実測値が本来の長さよりも小さい場合入る：短手
	else
	{
		// 小数点以下四捨五入
		y_sz = (u16)((y_limit + 5) * 0.1) - (u16)((size_limit.y_size + 5) * 0.1);
	}
	// 長手と短手でより差が大きい方のみ評価するため、差が小さい方は0を入れて処理しないようにする
	if (x_sz > y_sz)
	{
		y_sz = 0;
	}
	else
	{
		x_sz = 0;
	}


	p_params = (ST_T_FOLDING*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//パラメタセット
	folding.ppoint = (ST_IMUF_POITNS*)((u8*)(p_params)+(p_params->param_dt_ofs));
	folding.tir1_mask_ptn_diameter_x = 25;// p_params->tir1_mask_ptn_diameter_x;
	folding.tir1_mask_ptn_diameter_y = 25;//p_params->tir1_mask_ptn_diameter_y;
	folding.tir1_mask_ptn_divide_num = 1.0f / (float)441.0f;//p_params->tir1_mask_ptn_divide_num;
	//folding.ptir1_mask_ptn = (u8*)((u32)(p_params)+p_params->tir1_mask_ptn_ofs);


#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_FOLD, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_FOLD
	debug_logi_view = 1;
#endif

	//処理実行
	pst_work->proc_tm_co_s = get_10ns_co();	// 処理時間計測用カウンタ値取得
	get_folding_invalid_count(pst_work->buf_n, &folding, x_sz, y_sz);
	pst_work->proc_tm_co_e = get_10ns_co();	// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_FOLD
	debug_logi_view = 0;
#endif
	//結果ブロックに書き込む
	folding_result(pst_work->buf_n, folding);
}

/****************************************************************/
/**
* @brief		テープ検知
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void ts_tape(T_TEMPLATE_WORK* pst_work)
{
	//	u32 res = 0;
		//u8 i=0;

		//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_TAPE st_tape;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_TAPE* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)


	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_TAPE*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) 
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット ※
	st_tape.plearn_data = (u16*)((u8*)(p_params)+(p_params->reference_mapdata_address));	// 全面の基準データ配列

	st_tape.data_size = p_params->data_size;						// 全面の基準データの要素数
	st_tape.bill_size_x = p_params->bill_size_x;					// 紙幣サイズX
	st_tape.bill_size_y = p_params->bill_size_y;					// 紙幣サイズY
	st_tape.mesh_size_x = p_params->mesh_size_x;					// メッシュサイズX
	st_tape.mesh_size_y = p_params->mesh_size_y;					// メッシュサイズY

	st_tape.base_value = p_params->base_value;						// 紙幣厚み基準値
	st_tape.judge_count = p_params->judge_count;					// 判定終了ポイント数
#ifdef _RM_		/* シミュレータと実機を切りかえる */
	st_tape.threshold = work[pst_work->buf_n].pbs->mid_res_tape.threshold * 100;	// 閾値補助値（LSB 0.01）
#else
	//st_tape.threshold = p_params->threshold;						// 閾値補助値（LSB 0.01）	
	//work[pst_work->buf_n].pbs->mid_res_tape.threshold = p_params->threshold / 100;
	st_tape.threshold = work[pst_work->buf_n].pbs->mid_res_tape.threshold * 100;	// 閾値補助値（LSB 0.01）
#endif
	st_tape.threshold_type = p_params->threshold_type;				// 閾値種別

	st_tape.moving_average = p_params->moving_average;				// 移動平均（する／しない）
	st_tape.moving_ave_value_a = p_params->moving_ave_value_a;		// 移動平均A値
	st_tape.divide_ab = p_params->divide_ab;						// 分割A-B（する／しない）
	st_tape.divide_line_ab = p_params->divide_line_ab;				// 分割ラインA-B値
	st_tape.moving_ave_value_b = p_params->moving_ave_value_b;		// 移動平均B値
	st_tape.divide_bc = p_params->divide_bc;						// 分割B-C（する／しない）
	st_tape.divide_line_bc = p_params->divide_line_bc;				// 分割ラインB-C値
	st_tape.moving_ave_value_c = p_params->moving_ave_value_c;		// 移動平均C値

	st_tape.exclude_mesh_top = p_params->exclude_mesh_top;			// 除外範囲　先端　メッシュ数
	st_tape.exclude_mesh_bottom = p_params->exclude_mesh_bottom;	// 除外範囲　後端　メッシュ数
	st_tape.exclude_mesh_left = p_params->exclude_mesh_left;		// 除外範囲　左端　メッシュ数
	st_tape.exclude_mesh_right = p_params->exclude_mesh_right;		// 除外範囲　右端　メッシュ数

	st_tape.continuous_judge = p_params->continuous_judge;			// 連続判定閾値

	st_tape.tc1_tc2_corr = p_params->tc1_tc2_corr;					// tc1-tc2の補正
	st_tape.black_corr = p_params->black_corr;						// 黒補正	

	st_tape.mesh_skip_x = 24;			// サンプリングメッシュ飛ばし
	st_tape.mesh_skip_y = 24;			// サンプリングメッシュ飛ばし


//#ifdef VS_DEBUG_ITOOL
//	for_Dbg_start_algo_label_setting(ALGO_TAPE_MECHA_TC, dbg_codi_Logic);		// デバッグ用コールバック
//#endif

	//************************************************************
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	tape(pst_work->buf_n, &st_tape);									// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

															//res = (u32)st_soi.;							//エラーセット ※
															//pst_work->e_code = (u16)st_dy.judge;

	//結果ブロックに書き込む
	tape_result(pst_work->buf_n, st_tape);

}

/****************************************************************/
/**
* @brief		静電テープ検知
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void ts_cap_tape(T_TEMPLATE_WORK* pst_work)
{
	//	u32 res = 0;
		//u8 i=0;

		//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	ST_CAPACITANCE_TAPE st_cap_tape;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_CAP_TAPE* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)


	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_CAP_TAPE*)pst_work->now_para_blk_pt;

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい) 
	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);

	//パラメタセット ※
	st_cap_tape.tape_judg_thr = p_params->tape_judg_thr;		//テープ判定閾値
	st_cap_tape.x_interval = p_params->x_interval;		    //X座用の間引きの間隔
	st_cap_tape.y_interval = p_params->y_interval;		    //Y座用の間引きの間隔
	st_cap_tape.black_correction_s = p_params->black_correction_s;  //黒補正ライン
	st_cap_tape.black_correction_e = p_params->black_correction_e;  //黒補正ライン
	st_cap_tape.first_thrs_num = p_params->first_thrs_num;		//第一閾値配列の要素数
	st_cap_tape.reference_area_num = p_params->reference_area_num;	//参照エリア数

	st_cap_tape.p_reference_area = (ST_CAPACITANCE_TAPE_REFERENCE_AREA*)((u8*)(p_params)+(p_params->reference_area_offset)); // 参照エリア
	st_cap_tape.p_first_thrs = (float*)((u8*)(p_params)+(p_params->first_thrs_offset));									 // 第一閾値

	//memcpy(st_cap_tape.thrs_data, st_cap_tape.p_first_thrs,901);

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_TAPE_CAP, dbg_codi_Logic);		// デバッグ用コールバック
#endif

	//************************************************************
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	capacitance_tape(pst_work->buf_n, &st_cap_tape);		// 処理実行　 ※
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//結果ブロックに書き込む
	cap_tape_result(pst_work->buf_n, st_cap_tape);

}

/****************************************************************/
/**
* @brief		NEOMAG
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_neomag(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

#ifdef STRACTURE_DEFINITION
	//関数専用の構造体を定義する
	ST_NEOMAG st_neomag = { 0 };
#endif

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_NEOMAG* p_params;


	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_NEOMAG*)pst_work->now_para_blk_pt;

	//パラメタセット
	st_neomag.step = p_params->step;
	st_neomag.num = p_params->param_dt_num;
	st_neomag.split_mag_thr = p_params->split_mag_thr;
	st_neomag.stain_ir1_thr = p_params->stain_ir1_thr;
	st_neomag.stain_ir2_thr = p_params->stain_ir2_thr;
	st_neomag.stain_raito_thr = p_params->stain_raito_thr;
	st_neomag.split_point_thr = p_params->split_point_thr;

	memcpy(&st_neomag.para, (u8*)p_params + p_params->param_dt_ofs, sizeof(ST_NEOMAG_PARA) * p_params->param_dt_num);

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_NEOMAG, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_NEOMAG
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	res = neomag(pst_work->buf_n, &st_neomag);							// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_NEOMAG
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	neomag_result(pst_work->buf_n, st_neomag);


}

/****************************************************************/
/**
* @brief		磁気スレッド
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_magthread(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

#ifdef STRACTURE_DEFINITION
	//関数専用の構造体を定義する
	ST_THREAD magthr = { 0 };
#endif
	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_MAGTHREAD* p_params;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_MAGTHREAD*)pst_work->now_para_blk_pt;

#ifdef NEW_THREAD
	//パラメタセット ver2.1
	magthr.type = THREAD_TYPE_MAG;
	magthr.x_step = p_params->x_step;
	magthr.y_step = p_params->y_step;
	magthr.y_margin = p_params->y_margin;
	magthr.logi_y_step = THREAD_LOGI_Y;
	magthr.logi_x_step = THREAD_LOGI_X;
	magthr.restart_margin = p_params->restart_margin;
	magthr.standard_lack_rate = p_params->standard_lack_rate;
	magthr.min_lack_rate = p_params->min_lack_rate;
	magthr.x_start = p_params->x_start;
	magthr.x_end = p_params->x_end;
	magthr.t_plane = p_params->t_plane;
	magthr.mag_cheak = p_params->mag_dev_thr;

	magthr.search_tir = p_params->search_tir_thr;
	magthr.tir_cheack = p_params->search_lack_thr;
	magthr.mask_len = p_params->search_wid;
	magthr.percent_cheak = (u8)p_params->min_length;
	magthr.outlier_count = (u8)p_params->outlier_count;
	magthr.outlier_dev = (u8)p_params->outlier_dev;
#else
	//パラメタセット ver1.3
	magthr.type = 0;
	magthr.x_step = p_params->x_step;
	magthr.y_step = p_params->y_step;
	magthr.y_margin = p_params->y_margin;
	magthr.restart_margin = p_params->restart_margin;
	magthr.standard_lack_rate = p_params->standard_lack_rate;
	magthr.min_lack_rate = p_params->min_lack_rate;
	magthr.x_start = p_params->x_start;
	magthr.x_end = p_params->x_end;
	magthr.diff_plane = p_params->diff_plane;
	magthr.search_tirthr = p_params->search_tir_thr;
	magthr.search_magthr = p_params->search_mag_thr;
	magthr.search_diffthr = p_params->search_diff_thr;
	magthr.search_lack_dt_thr = p_params->search_lack_thr;
	magthr.tir_ave_thr = p_params->tir_ave_thr;
	magthr.mag_dev_thr = p_params->mag_dev_thr;
	magthr.left_dev_thr = p_params->left_dev_thr;
	magthr.right_dev_thr = p_params->right_dev_thr;
	magthr.magnectic_check_dt = p_params->magnectic_check_dt;
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_MAG_THREAD, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_THREAD
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	res = thread(pst_work->buf_n, &magthr);				// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_THREAD
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	magthread_result(pst_work->buf_n, magthr);

}

/****************************************************************/
/**
* @brief		金属スレッド
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_metalthread(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

#ifdef STRACTURE_DEFINITION
	//関数専用の構造体を定義する
	ST_THREAD metalthr = { 0 };
#endif

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_METALTHREAD* p_params;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_METALTHREAD*)pst_work->now_para_blk_pt;

#ifdef NEW_THREAD
	//パラメタセットver2.1
	metalthr.type = THREAD_TYPE_METAL;
	metalthr.x_step = p_params->x_step;
	metalthr.y_step = p_params->y_step;
	metalthr.y_margin = p_params->y_margin;
	metalthr.logi_y_step = THREAD_LOGI_Y;
	metalthr.logi_x_step = THREAD_LOGI_X;
	metalthr.restart_margin = p_params->restart_margin;
	metalthr.standard_lack_rate = p_params->standard_lack_rate;
	metalthr.min_lack_rate = p_params->min_lack_rate;
	metalthr.x_start = p_params->x_start;
	metalthr.x_end = p_params->x_end;
	metalthr.t_plane = p_params->t_plane;
	metalthr.search_tir = p_params->search_tir_thr;
	metalthr.tir_cheack = p_params->search_lack_thr;
	metalthr.mask_len = p_params->search_wid;
	metalthr.percent_cheak = (u8)p_params->min_length;
	metalthr.outlier_count = (u8)p_params->outlier_count;
	metalthr.outlier_dev = (u8)p_params->outlier_dev;
#else
	//パラメタセット
	metalthr.type = 1;
	metalthr.x_step = p_params->x_step;
	metalthr.y_step = p_params->y_step;
	metalthr.y_margin = p_params->y_margin;
	metalthr.restart_margin = p_params->restart_margin;
	metalthr.standard_lack_rate = p_params->standard_lack_rate;
	metalthr.min_lack_rate = p_params->min_lack_rate;
	metalthr.x_start = p_params->x_start;
	metalthr.x_end = p_params->x_end;
	metalthr.diff_plane = p_params->diff_plane;
	metalthr.search_tirthr = p_params->search_tir_thr;
	metalthr.search_diffthr = p_params->search_diff_thr;
	metalthr.search_lack_dt_thr = p_params->search_lack_thr;
	metalthr.tir_ave_thr = p_params->tir_ave_thr;
	metalthr.left_dev_thr = p_params->left_dev_thr;
	metalthr.right_dev_thr = p_params->right_dev_thr;
#endif

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_METAL_THREAD, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_THREAD
	debug_logi_view = 1;
#endif
	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	res = thread(pst_work->buf_n, &metalthr);				// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_THREAD
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	metalthread_result(pst_work->buf_n, metalthr);

}

/****************************************************************/
/**
* @brief		ホログラム検知
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_hologram(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

#ifdef STRACTURE_DEFINITION
	//関数専用の構造体を定義する
	ST_HOLOGRAM holo = { 0 };
#endif

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_HOLOGRAM* p_params;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_HOLOGRAM*)pst_work->now_para_blk_pt;

	//パラメタセット Ver1.2
	holo.num = p_params->param_dt_num;	//個数
	holo.margin = p_params->restert;	//外形からのマージン
	holo.search_area_step = p_params->search_area_step;	//探索間隔
	holo.sumhisper_thr = p_params->search_area_tirthr;	//有無判定をする閾値
	holo.sumhisnum_thr = p_params->search_line_step;	//センサ閾値
	holo.outline_flag = p_params->x_margin;	//外形を使用するフラグ

	memcpy(&holo.para, (u8*)p_params + p_params->param_dt_ofs, sizeof(ST_HOLOGRAM_PARA) * p_params->param_dt_num);


#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_HOLOGRAM, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_HOLOGRAM
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();		// 処理時間計測用カウンタ値取得
	res = hologram(pst_work->buf_n, &holo);		// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();		// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_HOLOGRAM
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	hologram_result(pst_work->buf_n, holo);

}

/****************************************************************/
/**
* @brief		特殊A検知
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_special_a(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

#ifdef STRACTURE_DEFINITION
	//関数専用の構造体を定義する
	ST_SPECIAL_A spa = { 0 };
#endif

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_SPECIAL_A* p_params;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_SPECIAL_A*)pst_work->now_para_blk_pt;

	//パラメタセット
	spa.num = p_params->param_dt_num;
	spa.total_spa_thr = p_params->total_spa_thr;
	spa.x_dis_thr = p_params->x_dis_thr;
	spa.y_dis_thr = p_params->y_dis_thr;
	spa.dis_thr = p_params->dis_thr;

	memcpy(&spa.para, (u8*)p_params + p_params->param_dt_ofs, sizeof(ST_SPECIAL_A_PARA) * p_params->param_dt_num);

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_SP_A, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_SPECIAL_A
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	res = special_a(pst_work->buf_n, &spa);				// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_SPECIAL_A
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	special_a_result(pst_work->buf_n, spa);

}

/****************************************************************/
/**
* @brief		特殊B検知
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_special_b(T_TEMPLATE_WORK* pst_work)
{
	u32 res = 0;

#ifdef STRACTURE_DEFINITION
	//関数専用の構造体を定義する
	ST_SPECIAL_B spb = { 0 };
#endif

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_SPECIAL_B* p_params;

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_SPECIAL_B*)pst_work->now_para_blk_pt;

	//パラメタセット
	spb.margin = p_params->margin;
	spb.xx[0] = p_params->x_area[0];
	spb.xx[1] = p_params->x_area[1];
	spb.coef1 = p_params->coef1;
	spb.coef2 = p_params->coef2;
	spb.intercept = p_params->intercept;
	spb.thr = p_params->thr;

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_SP_B, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_SPECIAL_B
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	res = special_b(pst_work->buf_n, &spb);				// 処理実行
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_SPECIAL_B
	debug_logi_view = 0;
#endif

	//結果ブロックに書き込む
	special_b_result(pst_work->buf_n, spb);

}

/****************************************************************/
/**
* @brief		CN-NN 1 Color
*				使用するスタックメモリは20897byteです
* * Ver 1.0.0
* https://app.box.com/s/jttqh72ouyi5ewbka8tsdauwjxrxgrny
*@param[in]	work メモリーのポインタ
*@param[out]
* @return
*/
/****************************************************************/
void	ts_cf_nn_1color(T_TEMPLATE_WORK* pst_work)
{

#define CFNN1COLOR_NORMALIZE_SKIP				(0)
#define CFNN1COLOR_NORMALIZE_DO					(1)
#define CFNN1COLOR_PREVIOUS_LAYER_CF			(1)
#define CFNN1COLOR_CURRENT_LAYER_EXECUTED		(1)
#define CFNN1COLOR_CURRENT_LAYER_NOT_EXECUTED	(0)
#define CFNN1COLOR_CURRENT_EFFECT_PLANE			(1)
#define CFNN1COLOR_PLANE_LST_MAX				(30)


	u32 i = 0;
	u32 write_plane_count = 0;
	u32 one_data_count = 0;
	u32 buf_num = pst_work->buf_n;
	u16* p_pint_ex1;
	u16* p_pint_ex2;
	float* p_pint_ex3;
	u16 tmp_input_data[NN_INPUT_NODE_MAX_NUM] = { 0 };			// 入力データ格納変数 
	u16 each_input_datas[PLANE_LST_MAX][200] = { 0 };			// プレーン毎のポイントデータ
	u8 each_input_datas_writeflg[PLANE_LST_MAX] = { 0 };	// 上の配列の有効無効フラグ

	//関数専用の構造体を定義する
	ST_NN			nn_cfg = { 0 };
	ST_NN_RESULT	nn_res = { 0 };

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_CF_NN_1COLOR* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	T_EXTRACT_POINT_DT* p_reference1;
	T_EXTRACT_POINT_DT* p_reference2;
	T_EXTRACT_CF_NN_1COLOR* p_reference3;

	// 判定ブロックからパラメ-タを取得
	p_params = (T_DECISION_PARAMETER_CF_NN_1COLOR*)pst_work->now_para_blk_pt;

	nn_res.layer_num = p_params->layer_num;	//レイヤー番号を設定

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	if (p_params->layer_num == 1)
	{
		//レイヤー１の時
		//識別と不足分のポイント抽出結果を参照し、テンポラリ配列にコピーして、その配列のポインタを設定する。

		//識別NNのポイント抽出結果を参照
		p_reference1 = (T_EXTRACT_POINT_DT*)search_blk_address_of_seq_num(p_params->reference_blk_num1, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);
		p_pint_ex1 = (u16*)((u8*)p_reference1 + p_reference1->extract_point_dt_ofs);	//ポインタ取得
		//memcpy(&tmp_input_data[0], p_pint_ex1, p_reference1->extract_point_dt_num * sizeof(u16));		//コピーを実施

		//データ数とプレーン数を取得
		one_data_count = p_reference1->extract_point_dt_num / p_reference1->plane_count;

		//各プレーンのポイントデータを格納
		for (i = 0; i < p_reference1->plane_count; ++i)
		{
			each_input_datas_writeflg[p_reference1->plane_lst[i]] = CFNN1COLOR_CURRENT_EFFECT_PLANE;
			memcpy(each_input_datas[p_reference1->plane_lst[i]], &p_pint_ex1[one_data_count * i], one_data_count * sizeof(u16));
		}

		//追加分のポイント抽出結果を参照
		if (p_params->reference_blk_num2 != 0)	//設定されていなければ実行しない
		{
			p_reference2 = (T_EXTRACT_POINT_DT*)search_blk_address_of_seq_num(p_params->reference_blk_num2, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);
			p_pint_ex2 = (u16*)((u8*)p_reference2 + p_reference2->extract_point_dt_ofs);									//ポインタ取得
			//memcpy(&tmp_input_data[p_reference1->extract_point_dt_num], p_pint_ex2, p_reference2->extract_point_dt_num * sizeof(u16));	//コピーを実施

			//各プレーンのポイントデータを格納
			for (i = 0; i < p_reference2->plane_count; ++i)
			{
				each_input_datas_writeflg[p_reference2->plane_lst[i]] = CFNN1COLOR_CURRENT_EFFECT_PLANE;
				memcpy(each_input_datas[p_reference2->plane_lst[i]], &p_pint_ex2[one_data_count * i], one_data_count * sizeof(u16));
			}
		}

		for (i = 0; i < PLANE_LST_MAX; ++i)
		{
			if (each_input_datas_writeflg[i] == CFNN1COLOR_CURRENT_EFFECT_PLANE)
			{
				memcpy(&tmp_input_data[write_plane_count * one_data_count], each_input_datas[i], one_data_count * sizeof(u16));
				write_plane_count++;
			}
		}

		nn_cfg.p_input_rawdata = tmp_input_data;			//正規化されていない入力データのポインタを設定する。
		nn_cfg.do_normalize = CFNN1COLOR_NORMALIZE_DO;		//正規化実行フラグを設定する。
		nn_res.do_flg = CFNN1COLOR_CURRENT_LAYER_EXECUTED;	//実行確認

	}
	else
	{
		//レイヤー２/３のとき
		//前レイヤーの結果を参照して、正規化済みの入力データを取得する。
		//前レイヤーで既に偽造券判定となっている場合は、今回の処理をスキップする。

		p_reference3 = (T_EXTRACT_CF_NN_1COLOR*)search_blk_address_of_seq_num(p_params->reference_blk_num1, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);

		//前レイヤーが偽造券判定している場合　もしくは、　前のレイヤーがスキップされている場合
		if (p_reference3->result == CFNN1COLOR_PREVIOUS_LAYER_CF || p_reference3->do_flg == CFNN1COLOR_CURRENT_LAYER_NOT_EXECUTED)
		{
			//処理を実行しない
			p_reference3->result = 1;								//CF扱い
			nn_res.do_flg = CFNN1COLOR_CURRENT_LAYER_NOT_EXECUTED;  // 未実行

			//結果ブロックへの書き込み
			cf_nn_1color_result((u8)buf_num, &nn_cfg, &nn_res);
			return;
		}

		//正規化されている入力データを配列へ上書きする。
		p_pint_ex3 = (float*)((u8*)p_reference3 + p_reference3->extract_point_dt_ofs);	            //ポインタ取得
		memcpy(&nn_cfg.in_put[0], p_pint_ex3, p_reference3->extract_point_dt_num * sizeof(float));	//コピーを実施（正規化済みの入力データ）
		nn_cfg.do_normalize = CFNN1COLOR_NORMALIZE_SKIP;								            //正規化をスキップする
		nn_res.do_flg = CFNN1COLOR_CURRENT_LAYER_EXECUTED;								            //実行確認

	}

	//パラメータを設定
	nn_cfg.in_node = p_params->input_note_num;		                                    //入力層ノード数
	nn_cfg.center_node = p_params->hidden_note_num;	                                    //中間層ノード数
	nn_cfg.out_nide = p_params->output_note_num;	                                    //出力層ノード数

	nn_cfg.sizeflag = 0;								                                    //入力ノードにサイズデータを使用する場合 0：使用しない　1：使用する
	nn_cfg.biasflag = 0;								                                    //バイアスノードを使用する場合 0：使用しない　1：使用する

	nn_cfg.pcenter_wit_offset = (float*)((u8*)p_params + p_params->hidden_weight_offset);	//中間ウェイトデータ先頭位置オフセット
	nn_cfg.pout_wit_offset = (float*)((u8*)p_params + p_params->output_weight_offset);	//出力ウェイトデータ先頭位置オフセット

	pst_work->proc_tm_co_s = get_10ns_co();					                                // 処理時間計測用カウンタ値取得
	NN(&nn_cfg, &nn_res);
	pst_work->proc_tm_co_e = get_10ns_co();					                                // 処理時間計測用カウンタ値取得

	//結果ブロックへの書き込み
	cf_nn_1color_result((u8)buf_num, &nn_cfg, &nn_res);

}

/****************************************************************/
/**
* @brief		CN-NN 2 Color
* *				使用するスタックメモリは20960byteです
* Ver 1.0.0
* https://app.box.com/s/jttqh72ouyi5ewbka8tsdauwjxrxgrny
* @param[in]	work メモリーのポインタ
* @param[out]
* @return
*/
/****************************************************************/
void	ts_cf_nn_2color(T_TEMPLATE_WORK* pst_work) {

#define CFNN2COLOR_NORMALIZE_SKIP				(0)
#define CFNN2COLOR_NORMALIZE_DO					(1)
#define CFNN2COLOR_PREVIOUS_LAYER_CF			(1)
#define CFNN2COLOR_CURRENT_LAYER_EXECUTED		(1)
#define CFNN2COLOR_CURRENT_LAYER_NOT_EXECUTED	(0)
#define CFNN2COLOR_CURRENT_EFFECT_PLANE			(1)
#define CFNN2COLOR_PLANE_LST_MAX				(30)

	u32 i = 0;
	u32 j = 0;
	u32 offset = 0;
	u8  plane_count = 0;
	s16 val;
	u32 one_data_count = 0;
	u32 buf_num = pst_work->buf_n;
	u16* p_pint_ex1;
	u16* p_pint_ex2;
	float* p_pint_ex3;
	u16 tmp_input_data[NN_INPUT_NODE_MAX_NUM] = { 0 };			// 入力データ格納変数 
	u16 each_input_datas[PLANE_LST_MAX][200] = { 0 };			// プレーン毎のポイントデータ

	u8 each_input_datas_writeflg[PLANE_LST_MAX] = { 0 };	// 上の配列の有効無効フラグ

	u8 terms1_old[PLANE_LST_MAX] = { UP_R_IR1 ,UP_R_IR1 ,UP_R_R ,DOWN_R_IR1 ,DOWN_R_IR1 ,DOWN_R_R };	// 計算プレーン 1項目
	u8 terms2_old[PLANE_LST_MAX] = { UP_R_R   ,UP_R_G   ,UP_R_G ,DOWN_R_R   ,DOWN_R_G   ,DOWN_R_G };	// 計算プレーン 2項目

	u8 terms1_new[PLANE_LST_MAX] = { UP_R_R   ,UP_R_IR1 ,UP_R_IR2 ,DOWN_R_R ,DOWN_R_IR1 ,DOWN_R_IR2 };	// 計算プレーン 1項目
	u8 terms2_new[PLANE_LST_MAX] = { UP_R_G   ,UP_R_G   ,UP_R_R   ,DOWN_R_G ,DOWN_R_G   ,DOWN_R_R   };	// 計算プレーン 2項目

	//u8 terms1[PLANE_LST_MAX] = { DOWN_R_R ,UP_R_G ,UP_R_R ,UP_R_R ,UP_R_G ,DOWN_R_R };	// 計算プレーン 1項目
	//u8 terms2[PLANE_LST_MAX] = { UP_R_R   ,UP_R_G   ,UP_R_G ,DOWN_R_R   ,DOWN_R_G   ,DOWN_R_G };	// 計算プレーン 2項目

	u8 term1 = 0;// 計算プレーン 1項目
	u8 term2 = 0;// 計算プレーン 2項目

	//関数専用の構造体を定義する
	ST_NN			nn_cfg = { 0 };
	ST_NN_RESULT	nn_res = { 0 };

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_CF_NN_2COLOR* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい)
	T_EXTRACT_POINT_DT* p_reference1;
	T_EXTRACT_POINT_DT* p_reference2;
	T_EXTRACT_CF_NN_2COLOR* p_reference3;

	// 判定ブロックからパラメ-タを取得
	p_params = (T_DECISION_PARAMETER_CF_NN_2COLOR*)pst_work->now_para_blk_pt;

	nn_res.layer_num = p_params->layer_num;	//レイヤー番号を設定

	//参照する結果ブロックをサーチする(必要なければ書かなくてよい)
	if (p_params->layer_num == 1)
	{
		//レイヤー１の時
		//識別と不足分のポイント抽出結果を参照し、テンポラリ配列にコピーして、その配列のポインタを設定する。
		// 
		//識別NNのポイント抽出結果を参照
		p_reference1 = (T_EXTRACT_POINT_DT*)search_blk_address_of_seq_num(p_params->reference_blk_num1, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);
		p_pint_ex1 = (u16*)((u8*)p_reference1 + p_reference1->extract_point_dt_ofs);	//ポインタ取得

		//データ数とプレーン数を取得
		one_data_count = p_reference1->extract_point_dt_num / p_reference1->plane_count;
		plane_count += p_reference1->plane_count;

		//各プレーンのポイントデータを格納
		for (i = 0; i < p_reference1->plane_count; ++i)
		{
			each_input_datas_writeflg[p_reference1->plane_lst[i]] = CFNN2COLOR_CURRENT_EFFECT_PLANE;
			memcpy(each_input_datas[p_reference1->plane_lst[i]], &p_pint_ex1[one_data_count * i], one_data_count * sizeof(u16));
		}

		//追加分のポイント抽出結果を参照
		if (p_params->reference_blk_num2 != 0)	//設定されていなければ実行しない
		{
			p_reference2 = (T_EXTRACT_POINT_DT*)search_blk_address_of_seq_num(p_params->reference_blk_num2, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);
			p_pint_ex2 = (u16*)((u8*)p_reference2 + p_reference2->extract_point_dt_ofs);									//ポインタ取得
			//memcpy(&tmp_input_data[p_reference1->extract_point_dt_num], p_pint_ex2, p_reference2->extract_point_dt_num * sizeof(u16));	//コピーを実施
			plane_count += p_reference2->plane_count;

			//各プレーンのポイントデータを格納
			for (i = 0; i < p_reference2->plane_count; ++i)
			{
				each_input_datas_writeflg[p_reference2->plane_lst[i]] = CFNN2COLOR_CURRENT_EFFECT_PLANE;
				memcpy(each_input_datas[p_reference2->plane_lst[i]], &p_pint_ex2[one_data_count * i], one_data_count * sizeof(u16));
			}
		}

		// 2colorの特徴ベクトル計算
		for (i = 0; i < plane_count; ++i)
		{
			//新
			if (each_input_datas_writeflg[UP_R_IR2] == 1)
			{
				term1 = terms1_new[i];
				term2 = terms2_new[i];
			}
			//旧
			else
			{
				term1 = terms1_old[i];
				term2 = terms2_old[i];
			}

			//プレーンが有効でなければエラー
			if (each_input_datas_writeflg[term1] != CFNN2COLOR_CURRENT_EFFECT_PLANE)
			{
				return;
			}
			if (each_input_datas_writeflg[term2] != CFNN2COLOR_CURRENT_EFFECT_PLANE)
			{
				return;
			}

			//１ポイントずつ計算する
			for (j = 0; j < one_data_count; ++j)
			{
				val = (s16)each_input_datas[term1][j] - (s16)each_input_datas[term2][j] + 128;
				if (val < 0)
				{
					val = 0;
				}
				else if (val > 255)
				{
					val = 255;
				}
				tmp_input_data[offset + j] = (u16)val;
			}

			offset = offset + one_data_count;	//オフセットをインクリメント
		}

		nn_cfg.p_input_rawdata = tmp_input_data;			//正規化されていない入力データのポインタを設定する。
		nn_cfg.do_normalize = CFNN2COLOR_NORMALIZE_DO;		//正規化実行フラグを設定する。
		nn_res.do_flg = CFNN2COLOR_CURRENT_LAYER_EXECUTED;	//実行確認
	}
	else
	{
		//レイヤー２/３のとき
		//前レイヤーの結果を参照して、正規化済みの入力データを取得する。
		//前レイヤーで既に偽造券判定となっている場合は、今回の処理をスキップする。

		p_reference3 = (T_EXTRACT_CF_NN_2COLOR*)search_blk_address_of_seq_num(p_params->reference_blk_num1, (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top);

		//前レイヤーが偽造券判定している場合　もしくは、　前のレイヤーがスキップされている場合
		if (p_reference3->result == CFNN2COLOR_PREVIOUS_LAYER_CF || p_reference3->do_flg == CFNN2COLOR_CURRENT_LAYER_NOT_EXECUTED)
		{
			//処理を実行しない
			p_reference3->result = 1;	//CF扱い
			nn_res.do_flg = CFNN2COLOR_CURRENT_LAYER_NOT_EXECUTED;  // 未実行

			//結果ブロックへの書き込み
			cf_nn_2color_result((u8)buf_num, &nn_cfg, &nn_res);
			return;
		}

		//正規化されている入力データを配列へ上書きする。
		p_pint_ex3 = (float*)((u8*)p_reference3 + p_reference3->extract_point_dt_ofs);	           //ポインタ取得
		memcpy(&nn_cfg.in_put[0], p_pint_ex3, p_reference3->extract_point_dt_num * sizeof(float)); //コピーを実施（正規化済みの入力データ）
		nn_cfg.do_normalize = CFNN2COLOR_NORMALIZE_SKIP;								           //正規化をスキップする
		nn_res.do_flg = CFNN2COLOR_CURRENT_LAYER_EXECUTED;								           //実行確認

	}

	//パラメータを設定
	nn_cfg.in_node = p_params->input_note_num;		                                    //入力層ノード数
	nn_cfg.center_node = p_params->hidden_note_num;	                                    //中間層ノード数
	nn_cfg.out_nide = p_params->output_note_num;	                                    //出力層ノード数

	nn_cfg.sizeflag = 0;								                                    //入力ノードにサイズデータを使用する場合 0：使用しない　1：使用する
	nn_cfg.biasflag = 0;								                                    //バイアスノードを使用する場合 0：使用しない　1：使用する

	nn_cfg.pcenter_wit_offset = (float*)((u8*)p_params + p_params->hidden_weight_offset);	//中間ウェイトデータ先頭位置オフセット
	nn_cfg.pout_wit_offset = (float*)((u8*)p_params + p_params->output_weight_offset);		//出力ウェイトデータ先頭位置オフセット

	pst_work->proc_tm_co_s = get_10ns_co();					                                // 処理時間計測用カウンタ値取得
	NN(&nn_cfg, &nn_res);
	pst_work->proc_tm_co_e = get_10ns_co();					                                // 処理時間計測用カウンタ値取得

	//結果ブロックへの書き込み
	cf_nn_2color_result((u8)buf_num, &nn_cfg, &nn_res);

}

/****************************************************************/
/**
* @brief		OCR
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_ocr(T_TEMPLATE_WORK* pst_work)
{
	//u32 res = 0;
	char res_series_1[20] = { 0 };
	char res_series_2[20] = { 0 };
	char color_detect[1] = { 0 };
	char res_series_color[20] = { 0 }; // add by hoan
	u32  cod_color_time = 0;			//add by furuta
	float prob;

	//関数専用の構造体を定義する ※
#ifdef STRACTURE_DEFINITION
	COD_Parameters st_cod;
#endif

	//関数専用の判定パラメタ構造体を定義する ※
	ST_T_OCR* p_params;

	//参照ブロック構造体(必要なければ書かなくてよい) ※

	// 判定ブロックからパラメタを取得 ※
	p_params = (ST_T_OCR*)pst_work->now_para_blk_pt;

	////パラメタセット ※
	st_cod.starting_x = p_params->starting_x;
	st_cod.starting_y = p_params->starting_y;
	st_cod.width = p_params->width;
	st_cod.height = p_params->height;
	st_cod.side = p_params->side;
	st_cod.color = p_params->color;
	st_cod.series_length = p_params->series_length;
	st_cod.switch_binarization_value = p_params->switch_binarization_value;
	st_cod.binarization_threshold = p_params->binarization_threshold;
	st_cod.threshold_on_x = p_params->threshold_on_x;
	st_cod.threshold_on_y = p_params->threshold_on_y;
	st_cod.threshold_on_offset = p_params->threshold_on_offset;

	st_cod.flip_x = p_params->flip_x;
	st_cod.flip_y = p_params->flip_y;
	st_cod.rotate = p_params->rotate;
	st_cod.dpi = p_params->dpi;
	st_cod.model_num = p_params->model_num;
	st_cod.area_num = p_params->series_area_num;
	st_cod.cutout_size = p_params->cutout_size;
	st_cod.character_types = (const char*)((u8*)(p_params)+p_params->serise_ary_offset);

	//テンプレート対応ができたら 有効にする

	//st_cod.p_composite_weight	= (float*) ((u8*)(p_params) + p_params->composite_weight_data_offset);
	//st_cod.p_composite_labels	= (char *)  ((u8*)(p_params) + p_params->composite_label_data_offset);
	//st_cod.p_numbers_weight		= (float*) ((u8*)(p_params) + p_params->numbers_weight_data_offset);
	//st_cod.p_numbers_labels		= (char *)  ((u8*)(p_params) + p_params->numbers_label_data_offset);
	//st_cod.p_letter_weight		= (float*)((u8*)(p_params) + p_params->letter_weight_data_offset);	
	//st_cod.p_letter_labels		= (char *)  ((u8*)(p_params) + p_params->letter_label_data_offset);

	st_cod.p_composite_weight = (float*)offset_address_cal((u8*)(p_params), p_params->composite_weight_data_offset);
	st_cod.p_composite_labels = (char*)offset_address_cal((u8*)(p_params), p_params->composite_label_data_offset);
	st_cod.p_numbers_weight = (float*)offset_address_cal((u8*)(p_params), p_params->numbers_weight_data_offset);
	st_cod.p_numbers_labels = (char*)offset_address_cal((u8*)(p_params), p_params->numbers_label_data_offset);
	st_cod.p_letter_weight = (float*)offset_address_cal((u8*)(p_params), p_params->letter_weight_data_offset);
	st_cod.p_letter_labels = (char*)offset_address_cal((u8*)(p_params), p_params->letter_label_data_offset);

	st_cod.composite_weight_num = p_params->composite_weight_data_num;
	st_cod.composite_labels_num = (u8)p_params->composite_label_data_num;
	st_cod.numbers_weight_num = p_params->numbers_weight_data_num;
	st_cod.numbers_labels_num = (u8)p_params->numbers_label_data_num;
	st_cod.letter_weight_num = p_params->letter_weight_data_num;
	st_cod.letter_labels_num = (u8)p_params->letter_label_data_num;

	//日本円のみ
	if ((work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id & 0xffff0000) == 0x00050000)
	{
		st_cod.jpy_flg = 1;
	}
	else
	{
		st_cod.jpy_flg = 0;
	}

	//テンプレート動作対応までの暫定対応
	if ((pst_work->ID_template & 0x0fff) == 0x0200)
	{
		st_cod.model_color = 0;
	}
	else if ((pst_work->ID_template & 0x0fff) == 0x0201)
	{
		st_cod.model_color = 1;
	}
	else
	{
		st_cod.model_color = 2;
	}
	//ここまで―――――――――――――――



	//************************************************************

	//暫定----------
	//denomi = (u8)p_params->switch_binarization_value;
	//----------
	//advance_insert_correction(pst_work->buf_n);
	//decision_plane_side_in_insertion_direction(pst_work->buf_n);

	// 色判定処理

	if (st_cod.jpy_flg == 1)
	{
		if (st_cod.area_num == 0)			//1箇所目
		{
			//st_cod.series_length	 = 5;
			//st_cod.binarization_threshold = 45;

			pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
			ocr_color(pst_work->buf_n, res_series_color, color_detect, &st_cod); // add by hoan
			pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得
			cod_color_time = pst_work->proc_tm_co_e - pst_work->proc_tm_co_s;	//add by furuta

			//st_cod.series_length	 = p_params->series_length;
			//st_cod.binarization_threshold = p_params->binarization_threshold;
		}
	}
	else
	{
		color_detect[0] = 0xff;
	}
#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_OCR, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_OCR
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得

	if (st_cod.area_num == 0)			//1箇所目
	{
		ocr_test_1(pst_work->buf_n, res_series_1, &prob, &st_cod);								// 処理実行　 ※
	}
	else if (st_cod.area_num == 1)		//2箇所目
	{
		ocr_test_1(pst_work->buf_n, res_series_2, &prob, &st_cod);								// 処理実行　 ※//
	}

	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef VS_DEBUG_OCR
	debug_logi_view = 0;
#endif

	pst_work->e_code = 0;

	//結果ブロックに書き込む
	ocr_result(pst_work->buf_n, res_series_1, res_series_2, &st_cod, cod_color_time, color_detect[0]);

}

/****************************************************************/
/**
* @brief		バーコードリード処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_barcode(T_TEMPLATE_WORK* pst_work)
{
#if 0
	//u32 res = 0;
	//u32 write_template_id = 0;


	ST_BARCODE_READ ts_barcode = { 0 };

	T_DECISION_PARAMETER_BARCODE* p_params;


	p_params = (T_DECISION_PARAMETER_BARCODE*)pst_work->now_para_blk_pt;

	//p_reference = (T_OUTLINE_RESULT *)search_blk_address_of_seq_num(p_params->reference_blk_num, (T_DECISION_RESULT_COMMON *)pst_work->p_result_blk_top);


	ts_barcode.code_type = (u8)p_params->read_type;
	ts_barcode.buf_num = pst_work->buf_n;
	ts_barcode.err_code = 0;
	ts_barcode.character_num_limits_max = p_params->character_num_limits_max;		//桁数
	//ts_barcode.character_num_limits_max = 28;
	ts_barcode.character_num_limits_min = p_params->character_num_limits_min;
	ts_barcode.max_bar = 0;
	ts_barcode.itf_scan_interval_y = p_params->itf_scan_interval_y;					//y方向のスキャン間隔を指定する　基本が5　精度と処理時間のトレードオフ

	ts_barcode.execute_flg = p_params->execute_flg;									//実行許可:1 /禁止:0
	ts_barcode.input_scan_side = p_params->input_scan_side;							//スキャンする面　2:両面 0:表のみ　1：裏のみ

	ts_barcode.itf_thin_bar_width_min = p_params->itf_thin_bar_width_min;			//細バーの幅　最小
	ts_barcode.itf_thin_bar_width_max = p_params->itf_thin_bar_width_max;			//細バーの幅　最大

	ts_barcode.itf_width_of_bar_ratio_min = p_params->itf_width_of_bar_ratio_min;	//太バーの比
	ts_barcode.itf_width_of_bar_ratio_max = p_params->itf_width_of_bar_ratio_max;	//太バーの比

	ts_barcode.itf_scan_range_y = p_params->itf_scan_range_y;						//スキャン範囲	単位は㎜
	ts_barcode.itf_chk_digit_mode = p_params->itf_chk_digit_mode;					//チェックデジットの実行フラグ

	ts_barcode.start_scan_x = p_params->start_scan_x;								//スキャン開始位置x　中心を0とした座標系　単位は㎜
	ts_barcode.start_scan_y = p_params->start_scan_y;								//スキャン開始位置y　中心を0とした座標系　単位は㎜



	//ts_barcode.code_type = (u8)p_params->read_type;
	//ts_barcode.buf_num = pst_work->buf_n;
	//ts_barcode.err_code = 0;
	//ts_barcode.character_num_limits_max = p_params->character_num_limits_max;	//桁数
	//ts_barcode.character_num_limits_min = p_params->character_num_limits_min;	
	//ts_barcode.max_bar = 0;
	//ts_barcode.itf_scan_interval_y = ;			//y方向のスキャン間隔を指定する　基本が5　精度と処理時間のトレードオフ
	//
	//ts_barcode.execute_flg = 1;					//実行許可:1 /禁止:0
	//ts_barcode.input_scan_side = 2;				//スキャンする面　2:両面 0:表のみ　1：裏のみ

	//ts_barcode.itf_thin_bar_width_min = 0.5;	//細バーの幅　最小
	//ts_barcode.itf_thin_bar_width_max = 0.6;	//細バーの幅　最大

	//ts_barcode.itf_width_of_bar_ratio_min = 2;	//太バーの比
	//ts_barcode.itf_width_of_bar_ratio_max = 3;	//太バーの比

	//ts_barcode.itf_scan_range_y = 10;			//スキャン範囲							　単位は㎜
	//ts_barcode.itf_chk_digit_mode = 0;			//チェックデジットの実行フラグ

	//ts_barcode.start_scan_x = -48;				//スキャン開始位置x　中心を0とした座標系　単位は㎜
	//ts_barcode.start_scan_y = 4;				//スキャン開始位置y　中心を0とした座標系　単位は㎜


	if (barcode_read_top_config.setting_change_flg == BARCODE_USE_PARAMS_SET_AT_THE_TOP)			// 上位の設定を用いる場合	
	{
		ts_barcode.character_num_limits_max = barcode_read_top_config.character_num_limits_max;	//最大桁数
		ts_barcode.character_num_limits_min = barcode_read_top_config.character_num_limits_max;	//最大桁数
		ts_barcode.input_scan_side = barcode_read_top_config.scan_side;							//スキャンする面
		ts_barcode.execute_flg = barcode_read_top_config.execute_flg;							//実行許可フラグ
	}

	//write_template_id = p_params->reserve;

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_BARCODE, dbg_codi_Logic);		// デバッグ用コールバック
#endif

#ifdef VS_DEBUG_BARCODE
	debug_logi_view = 1;
#endif
	pst_work->proc_tm_co_s = get_10ns_co();
	bar_check(&ts_barcode);
	pst_work->proc_tm_co_e = get_10ns_co();

#ifdef VS_DEBUG_BARCODE
	debug_logi_view = 0;
#endif

	barcode_read_result(pst_work->buf_n, ts_barcode);
#endif
}

/****************************************************************/
/**
* @brief		落書き検知　文字検知
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	ts_graffiti_text(T_TEMPLATE_WORK* pst_work)
{
#if 0
	//u32 res = 0;

#ifdef STRACTURE_DEFINITION
	ST_GRAFFITI_TEXT para = { 0 };
#endif

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_GRAFFITI_TEXT* p_params = { 0 };

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_GRAFFITI_TEXT*)pst_work->now_para_blk_pt;

	//パラメタセット
	para.plane = p_params->plane;
	para.start_x = p_params->start_x;
	para.start_y = p_params->start_y;
	para.end_x = p_params->end_x;
	para.end_y = p_params->end_y;
	para.sum_thr = p_params->sum_thr;
	para.binari_min = p_params->binari_min;
	para.cut_wid_max = p_params->cut_wid_max;
	para.cut_wid_min = p_params->cut_wid_min;
	para.cut_hid_max = p_params->cut_hid_max;
	para.cut_hid_min = p_params->cut_hid_min;
	para.cut_gap = p_params->cut_gap;
	para.pre_bi_thr = p_params->pre_bi_thr;
	para.pre_sum_thr = p_params->pre_sum_thr;
	para.cut_rows_line = p_params->cut_rows_line;
	para.cut_cols_line = p_params->cut_cols_line;
	para.nr_class = p_params->nr_class;
	para.nr_feature = p_params->nr_feature;
	para.bias = p_params->bias;
	para.ng_list_num = p_params->ng_list_num;
	para.label = (u8*)(p_params->label_offset + (u8*)pst_work->now_para_blk_pt);
	para.w = (double*)(p_params->w_offset + (u8*)pst_work->now_para_blk_pt);
	para.ng_list = (u8*)(p_params->ng_list_offset + (u8*)pst_work->now_para_blk_pt);

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_GRAFFITI, dbg_codi_Logic);		// デバッグ用コールバック
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	//graffiti_text_detection(pst_work->buf_n, &para);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

	//graffiti_text_result(pst_work->buf_n, para);			//結果ブロックに書き込む
#endif
}

/****************************************************************/
/**
* @brief		カスタムチェック
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void ts_customcheck(T_TEMPLATE_WORK* pst_work)
{
#ifdef STRACTURE_DEFINITION
	ST_CUSTOMCHECK para[CUSTOMCHECK_PARA_NUM] = { 0 };
	ST_CUSTOMCHECK_PARA custompara = { 0 };
#endif
	u8 ii  = 0;
	s16 err = 0;

	//関数専用の判定パラメタ構造体を定義する
	T_DECISION_PARAMETER_CUSTOMCHECK* p_params = { 0 };

	// 判定ブロックからパラメタを取得
	p_params = (T_DECISION_PARAMETER_CUSTOMCHECK*)pst_work->now_para_blk_pt;

	memcpy(&para, (u8*)p_params + p_params->param_dt_ofs, sizeof(ST_CUSTOMCHECK) * p_params->param_dt_num);

	custompara.paranum = p_params->param_dt_num;
	for (ii = 0; ii < p_params->param_dt_num; ii++)
	{
		custompara.para[ii].cca.startx = para[ii].startx;
		custompara.para[ii].cca.starty = para[ii].starty;
		custompara.para[ii].cca.endx = para[ii].endx;
		custompara.para[ii].cca.endy = para[ii].endy;
		custompara.para[ii].usedplane_num = para[ii].usedplane_num;
		custompara.para[ii].cal_num = para[ii].cal_num;

		memcpy(custompara.para[ii].ccu, (u8*)p_params + para[ii].usedplane_ofs, sizeof(CUSTOMCHECK_PARA_USEDPLANE) * para[ii].usedplane_num);

		memcpy(custompara.para[ii].ccc, (u8*)p_params + para[ii].cal_ofs, sizeof(CUSTOMCHECK_PARA_CAL) * para[ii].cal_num);
	}

#ifdef VS_DEBUG_ITOOL
	for_Dbg_start_algo_label_setting(ALGO_CUSTOM, dbg_codi_Logic);		// デバッグ用コールバック ITOOLでトレース表示するために必要
#endif

#ifdef	VS_DEBUG_CUSTOMCHECK
	debug_logi_view = 1;
#endif

	pst_work->proc_tm_co_s = get_10ns_co();					// 処理時間計測用カウンタ値取得
	err = customcheck(pst_work->buf_n, &custompara);
	pst_work->proc_tm_co_e = get_10ns_co();					// 処理時間計測用カウンタ値取得

#ifdef	VS_DEBUG_CUSTOMCHECK
	debug_logi_view = 0;
#endif


	customcheck_result(pst_work->buf_n, custompara);

}



///****************************************************************/
///**
// * @brief		テンプレート内券種IDに対するJCM　IDの対応リスト　処理
// *@param[in]	work メモリーのポインタ 
// *@param[out]	<出力引数arg1の説明]>
// * @return			 
// */
///****************************************************************/
//void	jcmID_tbl_proc(T_TEMPLATE_WORK* pst_work)
//{
//	T_DECISION_PARAMETER_JCMID_TBL*	p_jcmID_tbl_blk = 0;	
//	T_DECISION_PARAMETER_JCMID_TBL_OBJ*	p_tbl_obj = 0;															// パラメタ取得
//
//	p_jcmID_tbl_blk = (T_DECISION_PARAMETER_JCMID_TBL*)pst_work->now_para_blk_pt;
//	p_tbl_obj = (T_DECISION_PARAMETER_JCMID_TBL_OBJ*)(p_jcmID_tbl_blk->tbl_ofs + (u32)p_jcmID_tbl_blk);
//}

/****************************************************************/
/**
* @brief		総合判定　処理
*				各機種で必要な情報を結果ブロックから抽出する.
*
*				金種識別、サイズチェック、重券が失敗している場合は到達しない。
*				真偽鑑別で偽札と判定していても、正損判定は実行されるが、その結果は切り捨てられる。
*
*				サンプリングデータ内の中間情報及びフィットネス情報に結果を書き込むが、閾値(UFレベル制御側の通貨テーブルからもらう情報)が0の場合は、
				無条件で判定結果に0(ATMレベル)を書き込む。(各処理のレベルは各処理の関数内で求める。)
*
*				前提処理の結果によっては、その判定結果に0(ATMレベル)を書き込む。(汚れ判定がある場合、染み検知の結果は考慮しないなど)
*
*
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return		なし
*
* バージョン　	Ver1.0	UF判定の場合一律で偽造券判定基準を緩める
				Ver2.0	各UF判定に応じて、各鑑別レベルを調節する
				Ver2.1  特徴剥がれのレベルに応じてカテゴリ分けを行う


*/
/****************************************************************/
void	overall_judge_proc(T_TEMPLATE_WORK* pst_work)
{

#define OVERALL_CHECK_UF_PROC_PHASE 0	//正損判定の結果を確認する段階
	//NG判定の中で最も影響度が大きい処理を調べる。



#define OA_MRX_MODE						//総合判定ver2.1
//#define FEATURE_LOSS_LEVEL_CATEGORY	//特徴剥がれのレベルによるカテゴリ分け
#define OVERALL_MODE_CORR 1				//オンにすることで総合判定のパラメタありきの動作
#define OVERALL_BIG_UF					//ON:大損券をカテ１にする

#define OVERALL_EXCLUSION_PRIORITY	(1)	//排除優先モード
#define OVERALL_RBA_VALI_MITIGATION	(1)	//RBA鑑別緩和


#ifdef OA_MRX_MODE
#define OVERALL_DETECT_CATEGOR_PHASE 1	//カテゴリー判定をする段階。
#define OVERALL_PHASE 2					//新総合判定対応

#else
#define OVERALL_DETECT_CATEGOR_PHASE 0	//カテゴリー判定をする段階。
#define OVERALL_PHASE 1					//旧総合判定

#endif
	//結果ブロックの先頭アドレス
	T_DECISION_RESULT_COMMON* p_nowpara_res_blk = (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top;

	//各処理の専用構造体	随時追加する

	//T_OUTLINE_RESULT* res_outline;	//外形検知
	//T_EXTRACT_POINT_DT* res_point;	//ポイント抽出
	//T_NN_RESULT* res_nn;			//NN
	//T_OCR_RESULT* res_ocr;			//OCR
	//T_HOLE_RESULT* res_hole;		//穴
	//T_TEAR_RESULT* res_tear;		//裂け
	//T_DOG_EAR_RESULT* res_dog;		//角折れ
	//T_STAIN_RESULT* res_sta;		//染み
	//T_SOILING_RESULT* res_soi;		//汚れ
	//T_DEINK_RESULT* res_deink;		//脱色
	//T_DYENOTE_RESULT* res_dye;		//ダイノート
	//T_CIR3_RESULT* res_3cir;		//三色比
	//T_CIR4_RESULT* res_4cir;		//４色比
	//T_IR_CHECK_RESULT* res_irc;		//irチェック
	//T_FOLDING_RESULT* res_folding;	//折れ
	//T_MAG_RESULT* res_mag;			//磁気
	//T_TAPE_RESULT* res_tape;		//メカ式テープ
	//T_CAP_TAPE_RESULT* res_cap_tape;//静電テープ
	//T_MAGTHREAD_RESULT* res_magthr;	//磁気スレッド
	//T_METALTHREAD_RESULT* res_metalthr;	//金属スレッド
	//T_HOLOGRAM_RESULT* res_hologram;	//ホログラム

	T_MCIR_RESULT*          pres_mcir;		//mcir
	T_NN1_RESULT*           pres_nn1;		//NN1
	T_NN2_RESULT*           pres_nn2;		//NN2
	T_DOUBLE_CHECK_RESULT*  pres_dbc;		//重券
	T_IR2WAVE_RESULT*       pres_ir2wave;	//IR2
	T_UV_RESULT*            pres_uv;		//UV
	T_NEOMAG_RESULT*        pres_neomag;	//neomag
	T_SPECIAL_A_RESULT*     pres_special_a;	//特殊A
	T_SPECIAL_B_RESULT*     pres_special_b;	//特殊B
	T_EXTRACT_CF_NN_1COLOR* pres_cf_nn1;	//CF-NN1Color
	T_EXTRACT_CF_NN_2COLOR* pres_cf_nn2;	//CF-NN2Color
	T_CUSTOMCHECK_RESULT*	pres_custom;	//ｶｽﾀﾑﾁｪｯｸ

	ST_BS* pbs = work[pst_work->buf_n].pbs;	//サンプリングデータ
	//u16 total_proc_time = 0;	//合計時間
	u32 block_count = 0;			//ブロック数カウンター
	//u16 err;					//エラーコード

	u8 graffiti_count = 0;	//２回以上実行されることがあるので、結果ブロックも２つ以上作られてしまうことで、カウント処理が２回以上実行されるのを防ぐ

	u8	fake_count = 0;				//一般鑑別カウント
	u8	old_fake_count = 0;				//一般鑑別カウント
	u8	uv = 0;					//UVの結果
	u8  feature_count = 0;			//特徴剥がれのカウント
	u8	uf_count = 0;					//正損カウント
	u8* prba_validation_count;

	//各処理の結果　ビット対応
	u32	fake_deta = 0;				//MCIR　NN1　NN2　IR2　MAG　NEOMAG　
	//u32	uf_deta = 0;				//角折れ　裂け　汚れ　脱色　染み　穴　折り畳み　欠損　テープ(メカ)　静電
	//u32  feature_deta = 0;			//磁気　金属　ホロ

	//u16 temp_th1_level = 0; 


	u8 feature_cate1_flg = 0;
	u8 check_phase = 0;

	u8	overall_res[MAX_VALIDATION_COUNT];
	u8  original_res[MAX_VALIDATION_COUNT];

	u32 reject_id_check = 0;
	u8  must_impact_uf = 0;
	u8  temp_th2 = 0;
	//u8  end_flg = 0;

	u8 vali_level_mode = 0;		//1：正損判定結果による鑑別閾値を変更する。
	u8 rba_validation_mode = 0;		// RBA鑑別のFAKEカウント方法


	u32 layer = 0;
	u8 for_cfnn_output_level = 100;

	//判定パラメタ構造体
	T_DECISION_OVER_ALL* p_params;

	p_params = (T_DECISION_OVER_ALL*)pst_work->now_para_blk_pt;	// 判定ブロックからパラメタを取得

	//排除優先モード ON
	if (p_params->exclusion_priority == OVERALL_EXCLUSION_PRIORITY)
	{
		vali_level_mode = 0;		//損券救済モードOFF
		rba_validation_mode = 0;	//RBA鑑別緩和OFF

	}
	else //OFF
	{
		vali_level_mode = 1;		//損券救済モードON
		rba_validation_mode = 1;	//RBA鑑別緩和ON
	}


	reject_id_check = pst_work->ID_template & 0x0F00;

	//リジェクトノード判定
	if (reject_id_check == 0)
	{
		work[pst_work->buf_n].pbs->category_ecb = category1;
		pst_work->e_code = RES_ALLOVER_REJECT_NODE;
		return;
	}

	//正損再現前処理
	memset(overall_res, 0, MAX_VALIDATION_COUNT);
	memset(original_res, 0, MAX_VALIDATION_COUNT);

	set_each_alog_level(pst_work, overall_res);				//配列に各処理の結果を格納する。

	memcpy(original_res, overall_res, MAX_VALIDATION_COUNT);	//元データとして別の配列にコピー

	//正損再現処理
	if (replay_fitness_work.mode == REPLAY_FITNESS_MODE_ON)
	{
		if (replay_fitness_work.ocr_res[pst_work->buf_n] == REPLAY_FITNESS_OCR_SUCCESS)
		{
			//照合ヒットの確認
			if (replay_fitness_work.hit[pst_work->buf_n] == REPLAY_FITNESS_LIST_HIT)
			{
				//レベル書き換え
				rewrite_currentdata_uf_level(replay_fitness_work.hit_list_index[pst_work->buf_n], overall_res, pst_work->buf_n);
			}

			//レベル登録
			set_uf_level_data(replay_fitness_work.hit_list_index[pst_work->buf_n], overall_res, pst_work->buf_n);
		}
	}

	for (check_phase = 0; check_phase < OVERALL_PHASE; ++check_phase)		//1周目で正損の結果を確認して2周目での鑑別結果を確認する。
	{                                                                       //正損券救済の為、損券度合いに応じて鑑別の閾値レベルを変更するため。

		//end_flg = 0;
		block_count = 0;
		p_nowpara_res_blk = (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top;

		while (MAX_TEMPLATE_BLOCK_NAM > block_count)			// 結果ブロック実行シーケンス　ループ
		{
			block_count++;

			switch (p_nowpara_res_blk->function_code)			// 機能コードでマッチングするまで実行
			{

				//-------------------------鑑別---------------------------------------

			case FNC_DYE_NOTE:									//ダイノート
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)	// 2周目
				{
					pbs->fitness[DYE_NOTE_].bit.level = overall_res[ALGO_DYENOTE];
					pbs->fitness[DYE_NOTE_].bit.result = level_result_detect((u8)pbs->fitness[DYE_NOTE_].bit.level,
						(u8)pbs->fitness[DYE_NOTE_].bit.threshold_1,
						(u8)pbs->fitness[DYE_NOTE_].bit.threshold_2,
						0);
				}
				break;

			case FNC_DOUBLE_CHECK:								//重券 正券：-値　、重券：+値
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)	// 2周目
				{
					pres_dbc = (T_DOUBLE_CHECK_RESULT*)p_nowpara_res_blk;

					if (pres_dbc->invalid_data_count > 0)
					{
						overall_res[ALGO_DOUBLE_CHECK] = 1;
						pbs->mid_res_double_ck.result = UF;			//負の数：重券
						pbs->mid_res_double_ck.level = MAX_LEVEL;
						pbs->validation_result_flg |= 1 << VALIDATION_RES_DOUBLE_CHECK;
					}
					else
					{
						overall_res[ALGO_DOUBLE_CHECK] = 100;
						pbs->mid_res_double_ck.result = ATM;			//正の数：正券
						pbs->mid_res_double_ck.level = MIN_LEVEL;

					}
				}
				break;

			case FNC_MCIR:										// MCIR	強弱設定完
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)	// 2周目
				{
					pres_mcir = (T_MCIR_RESULT*)p_nowpara_res_blk;

					if (rba_validation_mode == OVERALL_RBA_VALI_MITIGATION)
					{
						prba_validation_count = &old_fake_count;
					}
					else
					{
						prba_validation_count = &fake_count;
					}

					validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_MCIR, VALIDATION_RES_MCIR, pres_mcir->level, p_params->mcir, p_params->mcir, 0, vali_level_mode,
						&pbs->mid_res_mcir.result, &pbs->mid_res_mcir.thr_level, prba_validation_count, &uf_count, &feature_count, &pbs->validation_result_flg);

				}
				break;

			case FNC_VALI_NN_1COLOR:								// 鑑別１色NN
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{													//
					pres_nn1 = (T_NN1_RESULT*)p_nowpara_res_blk;	//
																	//
					if (rba_validation_mode == OVERALL_RBA_VALI_MITIGATION)
					{
						prba_validation_count = &old_fake_count;
					}
					else
					{
						prba_validation_count = &fake_count;
					}

					validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_NN1, VALIDATION_RES_NN1, pres_nn1->level, p_params->nn1, p_params->nn1, 0, vali_level_mode,
						&pbs->mid_res_nn1.result, &pbs->mid_res_nn1.thr_level, prba_validation_count, &uf_count, &feature_count, &pbs->validation_result_flg);

				}
				break;

			case FNC_VALI_NN_2COLOR:								// 鑑別２色NN
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_nn2 = (T_NN2_RESULT*)p_nowpara_res_blk;

					validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_NN2, VALIDATION_RES_NN2, pres_nn2->level, p_params->nn2, p_params->nn2, 0, vali_level_mode,
						&pbs->mid_res_nn2.result, &pbs->mid_res_nn2.thr_level, &fake_count, &uf_count, &feature_count, &pbs->validation_result_flg);
				}
				break;

			case FNC_DUAL_IR:										//IR2
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_ir2wave = (T_IR2WAVE_RESULT*)p_nowpara_res_blk;
					pbs->mid_res_ir2wave.result = pres_ir2wave->result;
					pbs->mid_res_ir2wave.level = overall_res[ALGO_IR2_SICPA];

					validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_IR2_WAVE, VALIDATION_RES_IR2_WAVE, pbs->mid_res_ir2wave.level, p_params->ir2, p_params->ir2, 0, vali_level_mode,
						&pbs->mid_res_ir2wave.result, &pbs->mid_res_ir2wave.thr_level, &fake_count, &uf_count, &feature_count, &pbs->validation_result_flg);
				}
				break;

			case FNC_MAG:											//MAG
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					//res_mag = (T_MAG_RESULT*)p_nowpara_res_blk;

					pbs->mid_res_mag.level = overall_res[ALGO_MAG];

					validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_MAG, VALIDATION_RES_MAG, pbs->mid_res_mag.level, p_params->mag, p_params->mag, 0, vali_level_mode,
						&pbs->mid_res_mag.level_result, &pbs->mid_res_mag.uf_thr, &fake_count, &uf_count, &feature_count, &pbs->validation_result_flg);
				}
				break;

			case FNC_NEOMAG:										// NEOMAG レベル未対応
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_neomag = (T_NEOMAG_RESULT*)p_nowpara_res_blk;
					/*validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_NEOMAG, VALIDATION_RES_NEOMAG, pbs->mid_res_mag.level, p_params->mag, p_params->mag, 0,
						&pbs->mid_res_mag.level_result, &pbs->mid_res_mag.uf_thr, &fake_count, &uf_count, &feature_deta, &pbs->validation_result_flg);*/

						//正損結果に応じてth2の値を変更する。
	//change_validation_thr_level(must_impact_uf, OVERALL_VALIDATION_NEOMAG ,&pbs->mid_res_neomag.level);
					temp_th2 = 0;
					change_validation_thr_level(must_impact_uf, OVERALL_VALIDATION_NEOMAG, &temp_th2, vali_level_mode);

					if (pres_neomag->judge != 0 && temp_th2 != 0)
					{
						fake_deta |= 1 << 5;
						fake_count += 1;
						pbs->validation_result_flg |= 1 << VALIDATION_RES_NEOMAG;
					}

				}
				break;

			case FNC_UV_VALIDATE:									//UV鑑別
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_uv = (T_UV_RESULT*)p_nowpara_res_blk;

					validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_UV, VALIDATION_RES_UV, pres_uv->level, p_params->uv, p_params->uv, 0, vali_level_mode,
						&pbs->mid_res_uv_validate.level_result, &pbs->mid_res_uv_validate.uf_thr, &fake_count, &uf_count, &feature_count, &pbs->validation_result_flg);
				}
				break;

			case FNC_SPECIAL_A:										// 特殊A　レベル未対応
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_special_a = (T_SPECIAL_A_RESULT*)p_nowpara_res_blk;

					//正損結果に応じてth2の値を変更する。
					//change_validation_thr_level(must_impact_uf, OVERALL_VALIDATION_SP_A ,&res_special_a->thr_level);
					if (p_params->spa != JUDGE_INVALID)
					{
						if (pres_special_a->judge != SPA_OK)
							//if(res_special_a->level <= pbs->mid_res_special_a.thr_level && pbs->mid_res_special_a.uf_thr != TEMPLATE_OVERALL_NOT_INCLUDED_IN_RESULTS)
						{
							fake_deta |= 1 << 6;
							fake_count += 1;
							pbs->validation_result_flg |= 1 << VALIDATION_RES_SP_A;
						}
					}
				}
				break;

			case FNC_SPECIAL_B:										// 特殊B　レベル未対応
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_special_b = (T_SPECIAL_B_RESULT*)p_nowpara_res_blk;

					//正損結果に応じてth2の値を変更する。
					//change_validation_thr_level(must_impact_uf, OVERALL_VALIDATION_SP_B ,&res_special_b->thr_level);
					if (p_params->spb != JUDGE_INVALID)
					{
						if (pres_special_b->judge != SPB_OK)
							//if(res_special_b->level <= pbs->mid_res_special_b.thr_level && pbs->mid_res_special_b.uf_thr != TEMPLATE_OVERALL_NOT_INCLUDED_IN_RESULTS)
						{
							fake_deta |= 1 << 7;
							fake_count += 1;
							pbs->validation_result_flg |= 1 << VALIDATION_RES_SP_B;
						}
					}
				}
				break;

			case FNC_CF_NN_1COLOR:										// CF-NN1Color
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_cf_nn1 = (T_EXTRACT_CF_NN_1COLOR*)p_nowpara_res_blk;

					if (pres_cf_nn1->layer == 1)
					{

						for_cfnn_output_level = 100;

						for (layer = 0; layer < CF_NN_LAYER_NUM; ++layer)	//最小レベルを見つける
						{
							if (for_cfnn_output_level > pbs->mid_res_cf_nn_1.output_level[layer] &&
								pbs->mid_res_cf_nn_1.output_level[layer] > 0)
							{
								for_cfnn_output_level = pbs->mid_res_cf_nn_1.output_level[layer];
							}
						}

						validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_CF_NN_1COLOR, VALIDATION_RES_CF_NN1, for_cfnn_output_level, p_params->cf_nn1, p_params->cf_nn1, 0, vali_level_mode,
							&pbs->mid_res_cf_nn_1.result, &pbs->mid_res_cf_nn_1.thr_level, &fake_count, &uf_count, &feature_count, &pbs->validation_result_flg);
					}
				}
				break;

			case FNC_CF_NN_2COLOR:										// CF-NN2Color
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_cf_nn2 = (T_EXTRACT_CF_NN_2COLOR*)p_nowpara_res_blk;
					if (pres_cf_nn2->layer == 1)
					{
						for_cfnn_output_level = 100;

						for (layer = 0; layer < CF_NN_LAYER_NUM; ++layer)	//最小レベルを見つける
						{
							if (for_cfnn_output_level > pbs->mid_res_cf_nn_2.output_level[layer] &&
								pbs->mid_res_cf_nn_2.output_level[layer] > 0)
							{
								for_cfnn_output_level = pbs->mid_res_cf_nn_2.output_level[layer];
							}
						}

						validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_CF_NN_2COLOR, VALIDATION_RES_CF_NN2, for_cfnn_output_level, p_params->cf_nn2, p_params->cf_nn2, 0, vali_level_mode,
							&pbs->mid_res_cf_nn_2.result, &pbs->mid_res_cf_nn_2.thr_level, &fake_count, &uf_count, &feature_count, &pbs->validation_result_flg);
					}
				}
				break;

			case FNC_CUSTOM_CHECK:										// カスタムチェック
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					pres_custom = (T_CUSTOMCHECK_RESULT*)p_nowpara_res_blk;

					validation_each_final_judge(must_impact_uf, OVERALL_VALIDATION_CUSTOMCHECK, VALIDATION_RES_CUSTOM, pres_custom->level, p_params->customcheck, p_params->customcheck, 0, vali_level_mode,
						&pbs->mid_res_customcheck.result, &pbs->mid_res_customcheck.thr_level, &fake_count, &uf_count, &feature_count, &pbs->validation_result_flg);

				}
				break;



				//--------------正損＆鑑別-----------------
			case FNC_MAGTHREAD:										//磁気スレッド FITNESS_MISSING_MAG_THREAD
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{

					//res_magthr = (T_MAGTHREAD_RESULT*)p_nowpara_res_blk;

					//unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_MISSING_MAG_THREAD, ALGO_MAG_THREAD, 0, p_params->magthread, p_params->magthread, OVERALL_VALIDATION_MAG_THREAD,
					//	&must_impact_uf, &fake_count, &uf_count, &feature_deta);

					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_MISSING_MAG_THREAD, ALGO_MAG_THREAD, 0, p_params->magthread, p_params->magthread, OVERALL_VALIDATION_MAG_THREAD, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);

				}
				break;

			case FNC_METALTHREAD:									//金属スレッド FITNESS_MISSING_METAL_THREAD
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					//unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_MISSING_METAL_THREAD, ALGO_METAL_THREAD, 0, p_params->metalthread, p_params->metalthread, OVERALL_VALIDATION_METAL_THREAD,
					//	&must_impact_uf, &fake_count, &uf_count, &feature_deta);

					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_MISSING_METAL_THREAD, ALGO_METAL_THREAD, 0, p_params->metalthread, p_params->metalthread, OVERALL_VALIDATION_METAL_THREAD, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_HOLOGRAM:										//ホログラム
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					//unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_MISSING_SECURITY_FEATURE, ALGO_HOLOGRAM, 0, p_params->hologram, p_params->hologram, OVERALL_VALIDATION_HOLOGRAM,
					//	&must_impact_uf, &fake_count, &uf_count, &feature_deta);
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_MISSING_SECURITY_FEATURE, ALGO_HOLOGRAM, 0, p_params->hologram, 3, OVERALL_VALIDATION_HOLOGRAM, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);

				}
				break;

				//--------------正損-----------------
			case FNC_DOG_EAR:										//角折れ
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					//出力レベル、閾値レベルを用いて
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_DOGEARS, ALGO_DOG_EARS, UF_IMPACT_DOG_EAR, p_params->dog_ear, p_params->dog_ear, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_TEAR:											// 裂け
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_TEARS, ALGO_TEAR, UF_IMPACT_TEAR, p_params->tear, p_params->tear, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_SOILING:										//汚れ
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_SOILING, ALGO_SOILING, UF_IMPACT_SOILING, p_params->soiling, p_params->soiling, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_DE_INK:										// 脱色
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_DE_INKED_NOTE, ALGO_DEINKD, UF_IMPACT_DEINKED, p_params->deink, p_params->deink, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_STAIN:											//染み
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_STAINS, ALGO_STAIN, UF_IMPACT_STAIN, p_params->stain, p_params->stain, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_HOLE:											// 穴
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_HOLES, ALGO_HOLE, UF_IMPACT_HOLE, p_params->hole, p_params->hole, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_FOLDING:										// 折り畳み&欠損
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_FOLDS, ALGO_FOLD, UF_IMPACT_FOLDING, p_params->fold, p_params->fold, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);

					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_MUTILATIONS, ALGO_MUTILATION, UF_IMPACT_MUTILATION, p_params->mutilation, p_params->mutilation, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_MUTILATION:		//欠損
				break;


			case FNC_TAPE:											// テープ(メカ式)
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{

					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_REPAIRS_MECHA, ALGO_TAPE_MECHA, UF_IMPACT_TAPE, p_params->tape_mecha, p_params->tape_mecha, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_CAP_TAPE:										//静電テープ
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_REPAIRS_CAP, ALGO_TAPE_CAP, UF_IMPACT_TAPE, p_params->tape_cap, p_params->tape_cap, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);
				}
				break;

			case FNC_WASH:											//洗濯券
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_WASHED, ALGO_WASHED, UF_IMPACT_WASH, p_params->wash, p_params->wash, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uv, &feature_count);
				}
				break;

			case FNC_GRAFFITI_TEXT:									//落書き検知
				if (check_phase == OVERALL_CHECK_UF_PROC_PHASE && graffiti_count == 0)		// 1周目
				{
					unfit_each_final_judge(pbs, original_res, overall_res, FITNESS_GRAFFITI, ALGO_GRAFFITI, UF_IMPACT_STAIN, p_params->stain, p_params->stain, 0, vali_level_mode,
						&must_impact_uf, &fake_count, &uf_count, &feature_count);

					graffiti_count += 1;
				}
				break;



			case FNC_OVERALL_JUDGE:
				break;

			case FNC_END:
				if (check_phase == OVERALL_DETECT_CATEGOR_PHASE)		// 2周目
				{
					//排除優先
					if (p_params->exclusion_priority == OVERALL_EXCLUSION_PRIORITY)
					{
						//特徴剥がれ
						if (feature_count != 0)
						{

							if (feature_cate1_flg == 1)
							{
								pbs->category_ecb = category1;	//条件によってはCat1
							}
							else
							{
								pbs->category_ecb = category2;
							}

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_FEATURE_PEELING;
						}

						//変造券・偽造券
						else if (fake_count != 0)
						{
							pbs->category_ecb = category2;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_COUNTERFEIT;
						}

						//洗濯券
						else if (uv == 1)
						{
							pbs->category_ecb = category4b;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_WASH;
						}

						//損券
						else if (uf_count != 0)
						{
							pbs->category_ecb = category4b;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_UNFIT;
						}

						//正券判定
						else
						{
							pbs->category_ecb = category4a;
						}

						return;

					}
					//フルスペック版
					else
					{

						if (old_fake_count >= 2)	//MCIRとNN1/2の偽造券判定が2/3だった時、fakeカウントをアップする。
						{
							fake_count += 1;
						}

						//特殊な特徴剥がれ
						//if(uv == 0 && uf_count == 0 && fake_count == 0 && feature_count != 0)
						if (feature_count != 0)
						{

							if (feature_cate1_flg == 1)
							{
								pbs->category_ecb = category1;	//条件によってはCat1
							}
							else
							{
								pbs->category_ecb = category2;
							}

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_FEATURE_PEELING;
						}

						//変造券・偽造券
						else if (uv == 0 && uf_count == 0 && fake_count >= 2)
						{
							pbs->category_ecb = category2;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_COUNTERFEIT;
						}

						//変造券・偽造券疑い
						else if (uv == 0 && uf_count == 0 && fake_count == 1)
						{
							pbs->category_ecb = category3;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_COUNTERFEIT_DOUBT;
						}

						//洗濯券
						else if (uv == 1 && fake_count == 0 && feature_count == 0)
						{
							pbs->category_ecb = category4b;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_WASH;
						}

						//コピー券
						else if (uv == 1 && fake_count >= 1)
						{
							pbs->category_ecb = category2;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_COPY;
						}

						//損券
						else if (uv == 0 && uf_count != 0 && fake_count == 0 && feature_count == 0)
						{
							pbs->category_ecb = category4b;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_UNFIT;
						}

						//損券 偽造変造券疑い
						else if (uv == 0 && uf_count != 0 && fake_count == 1)
						{
							pbs->category_ecb = category3;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_UF_COUNTERFEIT_DOUBT;
						}

						//ぼろい偽造券・変造券
						else if (uv == 0 && uf_count != 0 && fake_count >= 2)
						{
							pbs->category_ecb = category2;

							//エラーコードではなくリザルトコード
							pst_work->e_code = RES_ALLOVER_TATTERED;
						}
						else
						{
							//当てはまらない
							pbs->category_ecb = category4a;
						}
#ifdef OVERALL_BIG_UF
						big_uf_relief(pbs, pst_work);	//大損券のカテゴリ救済関数実行
#endif
						return;
					}

				}
				else
				{
					//1周目の時はカウント値を最大にしてwhile文を終わらせる。
					block_count = MAX_TEMPLATE_BLOCK_NAM;
				}
				break;

			}


			//次の処理の先頭アドレスへ
			p_nowpara_res_blk = (T_DECISION_RESULT_COMMON*)((u8*)p_nowpara_res_blk + p_nowpara_res_blk->next_block_ofs);

		}//while
	}//for

	//当てはまらない
	pbs->category_ecb = category1;
	return;

}

/****************************************************************/
/**
 * @brief	th2を補正する。
			元々th2が0の時は何も行わない
			計算した結果th2が0になった場合は1に修正する

 *@param[in]	損券番号
				アルゴ番号
				th2
 *@param[out]	th2
 * @return		なし
 */
 /****************************************************************/
s8	change_validation_thr_level(u8 must_impact_uf, u8 algo_idx, u8* thr2, u8 on_off)
{
#define CVTL_OFF (0)	//正損結果による鑑別閾値の変更を行わない場合


	//鑑別の閾値レベルの補正値テーブル
	float corr_vals[UF_IMPACT_PROC_NUM][OVERALL_VALIDATION_PROC_NUM] =
	{ // mcir  NN1   NN2   MAG   UV    IR2  NeoMag spA   spB   MgTh MtlTh  holo CFNN1 CFNN2  ｶｽﾀﾑ    
		{C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO, C_NO},	//補正なし
		{C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO, C_NO},	//テープ
		{CSML2,CSML2,CSML2,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO, C_NO},	//角折れ、裂け
		{CSML ,CSML ,CSML ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO, C_NO},	//洗濯
		{CSML ,CSML ,CSML ,C_NO ,C_NO ,CBIG ,CSML ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO, C_NO},	//穴
		{CSML ,CSML ,CSML ,C_NO ,C_NO ,CDIS ,CBIG ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO, C_NO},	//染み
		{CBIG ,CBIG ,CBIG ,CSML ,C_NO ,CDIS ,CBIG ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,CBIG ,CBIG, CBIG},	//汚れ
		{CDIS ,CDIS ,CBIG ,CDIS ,C_NO ,CDIS ,CDIS ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,CBIG ,CBIG, CBIG},	//脱色
		{CDIS ,CDIS ,CBIG ,CBIG ,CDIS ,CDIS ,CBIG ,C_NO ,C_NO ,C_NO ,C_NO ,C_NO ,CBIG ,CBIG, CBIG},	//折り畳み、欠損
	};

	if (on_off == CVTL_OFF)	//この機能がOFFの場合
	{
		return 0;	//終了
	}

	if (*thr2 == 0)	//閾値がもともと0ならば変更する必要はないので
	{
		return 0;	//終了
	}

	if (corr_vals[must_impact_uf][algo_idx] == CDIS)	//無効を示す場合は
	{
		*thr2 = 0;									//閾値を0に設定する
		return 0;
	}
	else											//無効以外は
	{
		*thr2 = (u8)(*thr2 - (*thr2 * corr_vals[must_impact_uf][algo_idx]) + 0.5f);	//閾値を補正する。
	}

	if (*thr2 > MIN_LEVEL)	//閾値の整合確認
	{
		*thr2 = MIN_LEVEL;
	}

	if (*thr2 < MAX_LEVEL)	//閾値の整合確認
	{
		*thr2 = MAX_LEVEL;
	}


	return 0;
}


///****************************************************************/
///**
// * @brief		現在の結果ブロック最後尾に、新たな結果ブロック領域を確保できるかチェックし、
//				OKなら、共通項に値をセットする。
// *@param[in]	新たな結果ブロックのサイズ (可変長部、パディングを含む)、　work メモリーのポインタ、　終了状態コード、　このブロックが返すテンプレート内券種ID
// *@param[out]	<出力引数arg1の説明]>
// * @return		追加中のブロックの先頭アドレス	 
// */
///****************************************************************/
//void*	add_result_blk(u16	siz, T_TEMPLATE_WORK* pst_work, u16 cnd_code, u16 tmplt_ID)
//{
//	T_DECISION_RESULT_COMMON*	ablkp;
//	u32	ovalcap = 0;
//
//	ablkp = pst_work->now_result_blk_pt;
//
//	// 結果ブロック領域外のチェック
//	ovalcap =  (u32)pst_work->now_result_blk_pt - (u32)pst_work->p_result_blk_top + siz;		// 追加した後の容量を計算し、
//	if( ovalcap >= RESULT_TBL_SIZE)										// 容量がMAXを超える事が判れば、
//	{																	// エラーとする。
//		if(pst_work->e_code == 0)										//
//		{
//			pst_work->e_code = E_RESULT_AREA_OVER;						// 結果領域が足りないエラーとして、結果ブロックを更新せずにRET
//			return ablkp;
//		}
//	}
//	memset(pst_work->now_result_blk_pt, 0, siz);						// 固定長分と可変長部の領域を0クリアする
//
//	ablkp->next_block_ofs = siz;										// 次ブロックオフセット値をセット 
//	ablkp->source_blk_ofs = (u32)pst_work->now_para_blk_pt - (u32)ablkp;// 対応元の判定パラメタブロックの先頭オフセット値
//	ablkp->serial_number = pst_work->now_para_blk_pt->serial_number;	// 結果ブロック連番
//	ablkp->function_code = pst_work->now_para_blk_pt->function_code;	// 機能コード
//	ablkp->cnd_code	= cnd_code;											// 状態コード
//	ablkp->tmplt_ID = tmplt_ID;											// テンプレート内券種ID
//	ablkp->proc_time = pst_work->proc_tm_co_e - pst_work->proc_tm_co_s;	// 処理時間計算
//
//	pst_work->now_result_blk_pt = (T_DECISION_RESULT_COMMON*)((u32)pst_work->now_result_blk_pt + siz);	// 現在の判定結果ブロックのポインタを更新する。
//
//	return (void*)ablkp;
//}


///****************************************************************/
///**
//* @brief		パディングデータを追加した場合のデータ長を計算して返す。		
// *@param[in]	データ本体のデータ長　　
// *@param[out]	なし
// * @return		パディングデータを追加した場合のデータ長	 
// */
///****************************************************************/
//u16	calculate_size_including_padding_data(u16 body_siz)
//{
//	return	body_siz + ((4 - body_siz % 4) % 4);	
//}


/****************************************************************/
/**
* @brief		検索開始アドレスから後ろ方向へ順にチェインをたどりながら、ENDマークまで
指定した結果ブロック連番のブロックアドレスを探します。
*@param[in]	ブロック連番
検索開始アドレス　　
*@param[out]	なし
* @return		見つかった場合は、ブロックアドレスを返す。
				見つからなかったときは、0 を返す。
*/
/****************************************************************/
T_POINTER search_blk_address_of_seq_num(u16 seq_num, T_DECISION_RESULT_COMMON* start_address)
{
#define MAX_LIMIT_WHILE 100000
	T_DECISION_RESULT_COMMON* t_com_p;
	u32 max_while_count = 0;

	t_com_p = (T_DECISION_RESULT_COMMON*)start_address;

	while (t_com_p->function_code != FNC_END)
	{
		max_while_count++;

		if (max_while_count > MAX_LIMIT_WHILE)
		{
			return 0;
		}

		if (t_com_p->serial_number == seq_num)
		{
			return	(T_POINTER)t_com_p;
		}

		t_com_p = (T_DECISION_RESULT_COMMON*)((u8*)t_com_p + t_com_p->next_block_ofs);
	}

	return	0;
}

/****************************************************************/
/**
* @brief		閾値１と２とレベルから判定結果を返します
*@param[in]		レベル	level
				閾値１　Fit
				閾値２　UF　
				閾値３　REJ

*@param[out]	なし
* @return		判定結果　0：ATM　1：Fit　2：UF　3；REJ
*/
/****************************************************************/
//u8	level_result_detect(u8 level, u8 th1, u8 th2)
u8	level_result_detect(u8 level, u8 th1, u8 th2, u8 th3)
{

	if (th2 == 0 && th1 == 0)
	{
		return ATM;	//判定には含まない
	}

	if (level > th1)
	{
		return ATM;	//ATM
	}
	else if (level <= th1 && level > th2)
	{
		return FIT;	//Fit
	}
	//else if(level <= th2)
	//{
	//	return UF;	//UF
	//}
	else if (level <= th2 && level > th3)
	{
		return UF;	//UF
	}
	else if (level <= th3)
	{
		return REJ;	//REJ
	}

	return ATM;
}
//#ifndef _RM_
///****************************************************************/
///**
//* @brief		テンプレート内券種ID対応表の先頭アドレスを見つけてグローバル変数にセットする 
//				シミュレータのみで動作する
//*@param[in]		pst_work　メモリーのポインタ
//*@param[out]	p_temp_currenct_table_top.tbl_ofs	IDテーブルまでのオフセット
//				p_temp_currenct_table_top.tbl_num	テーブルの要素数
//													エラーの時は両変数に0をセットする
//* @return		-1 エラー
//*/
///****************************************************************/
s8 template_currenct_table_top_detect(T_TEMPLATE_WORK* pst_work)
{
	T_DECISION_PARAMETER_COMMON* p_id_proc;			//判定パラメタの共通項
	T_DECISION_PARAMETER_JCMID_TBL* p_id_table;		//対応表ブロックの専用構造体

	u32 max_while_count = 0;

	//テンプレート内券種　JCMID　対応表の先頭アドレスを見つける。
	p_id_proc = (T_DECISION_PARAMETER_COMMON*)((u8*)template_top_pt + 0x100);

	while (p_id_proc->function_code != FNC_END)
	{
		max_while_count++;

		if (max_while_count > MAX_LIMIT_WHILE)
		{
			p_temp_currenct_table_top.tbl_ofs = 0;
			p_temp_currenct_table_top.tbl_num = 0;
			return -1;
		}

		if (p_id_proc->function_code == FNC_JCMID_TBL)
		{
			break;	//見つけたら終了
		}

		//アドレスインクリメント
		p_id_proc = (T_DECISION_PARAMETER_COMMON*)((u8*)p_id_proc + p_id_proc->next_block_ofs);
	}

	//見つからなかった
	if (p_id_proc->function_code == FNC_END)
	{
		//パラメタをセット
		p_temp_currenct_table_top.tbl_ofs = 0;	//オフセットと
		p_temp_currenct_table_top.tbl_num = 0;	//要素数共に0をセット
	}
	else//見つかった
	{
		//先頭アドレスをセット
		p_id_table = (T_DECISION_PARAMETER_JCMID_TBL*)p_id_proc;

		//パラメタをセット
#ifdef _RM_
		p_temp_currenct_table_top.tbl_ofs = (u32)((u8*)p_id_table + p_id_table->tbl_ofs);	//オフセット
#else
		p_temp_currenct_table_top.tbl_ofs = (u64)((u8*)p_id_table + (u32)p_id_table->tbl_ofs);	//オフセット
#endif
		p_temp_currenct_table_top.tbl_num = p_id_table->tbl_num;		//要素数
	}

	pst_work->now_para_blk_pt = (T_DECISION_PARAMETER_COMMON*)((u8*)template_top_pt + FILE_INFO_SIZE);	// 現在の判定ブロックポインタをブロックテーブルの先頭にリセット

	return 0;
}
//#endif

///****************************************************************/
///**
//* @brief		指定のパラメータブロックが存在するかをチェックする。
//*@param[in]	note_id		指定のノートID
//				func_id		指定の機能コード				
// 
//*@param[out]	なし
//* @return		1	有
//				0	無
//*/
///****************************************************************/
s8 template_function_existence_detect(u16 note_id, u16 func_id)
{
	T_DECISION_PARAMETER_COMMON* p_id_proc;			//判定パラメタの共通項
	//T_DECISION_PARAMETER_JCMID_TBL* p_id_table;		//対応表ブロックの専用構造体

	u32 max_while_count = 0;

	u16* p_ID_list = 0;
	u16 cpIDbase = note_id & 0x0fff;				//券種ID抽出
	u16 cpIDtmp = 0;

	//テンプレートパラメータの先頭アドレスを設定
	p_id_proc = (T_DECISION_PARAMETER_COMMON*)((u8*)template_top_pt + 0x100);

	while (p_id_proc->function_code != FNC_END)
	{
		max_while_count++;

		if (max_while_count > MAX_LIMIT_WHILE)
		{
			return 0;
		}

		p_ID_list = (u16*)((u8*)p_id_proc + p_id_proc->ID_lst_ofs);
		//n = pst_work->now_para_blk_pt->ID_lst_num;	// 要素数
		cpIDtmp = *p_ID_list & 0x0fff;				//券種ID抽出
		if (p_id_proc->function_code == func_id && cpIDtmp == cpIDbase)
		{
			return 1;	//見つけたら終了
		}

		//アドレスインクリメント
		p_id_proc = (T_DECISION_PARAMETER_COMMON*)((u8*)p_id_proc + p_id_proc->next_block_ofs);
	}

	return 0;
}



/****************************************************************/
/**
* @brief		テンプレートタスクからもらったフィットネスの閾値情報をサンプリングデータにセットする。

*@param[in]		pst_work　						メモリーのポインタ
				cry_num							通貨テーブルのインデックス
				state							状態管理のフラグ　1：ダイノートのみ 2：金種判定後　3：デフォルト値の設定
*@param[out]	pbs->fitness[n].bit.threshold_1	Fit判定閾値
				pbs->fitness[n].bit.threshold_2	UF判定閾値
* @return		エラーコード	０：正常
								１：th1の値が異常
								２：th2の値が異常
								３：tth1がth2以上
	例えばth1=59,th2=60の場合で、
	結果が58だったらATM
	59だったらFIT
	60だったらUF
	th1=60,th2=60の場合はエラーコード３

	*/
	/****************************************************************/
u32	set_fitness_thrs(T_TEMPLATE_WORK* pst_work, u32 cry_num, u8 state)
{
#define SKIP_DEFAULT_NUM	(0x0)
#define FIT_DEFAULT_LEVEL	(70)
#define UF_DEFAULT_LEVEL	(40)
#define RJ_DEFAULT_LEVEL	(0)
#define DEFAULT_MARGIN		(0)		//noteidがないシミュレータ上では正損再現処理は実行できない。

	ST_BS* pbs = work[pst_work->buf_n].pbs;

	u8 algo_idx = 0;
	u32 err_code = 0;


	switch (state)// 機能コードでマッチングするまで実行
	{
	case SET_DYENOTE_ONLY:	//ダイノートの閾値取得のみ

		//サンプリングデータへダイノートの閾値レベルを書き込む
		err_code = write_thrlevels_2_samplingdata(
			pbs,
			ALGO_DYENOTE,
			fit_level_thrs[0].fit_level[ALGO_DYENOTE],
			fit_level_thrs[0].uf_level[ALGO_DYENOTE],
			fit_level_thrs[0].rj_level[ALGO_DYENOTE],
			fit_level_thrs[0].level_margin[ALGO_DYENOTE]
		);
		pst_work->thrs_set_complete = 0;

		break;

	case SET_CURRENCY_THRS: // 金種判定後の閾値取得

		//サンプリングデータへ閾値レベルを書き込む
		for (algo_idx = 0; algo_idx < MAX_ALGORITHM_NUM; ++algo_idx)
		{
			err_code = write_thrlevels_2_samplingdata(
				pbs,
				algo_idx,
				fit_level_thrs[cry_num].fit_level[algo_idx],
				fit_level_thrs[cry_num].uf_level[algo_idx],
				fit_level_thrs[cry_num].rj_level[algo_idx],
				fit_level_thrs[cry_num].level_margin[algo_idx]
			);

		}

		pst_work->thrs_set_complete = 1;

		break;

	case SET_DEFAULT_THRS:	//閾値レベル情報がないので、デフォルト値を設定する。

		for (algo_idx = 0; algo_idx < MAX_ALGORITHM_NUM; ++algo_idx)
		{

			if (ALGO_TAPE_MECHA_TC == algo_idx)
			{
				//メカ式テープ検知の厚みは通常のUFとFITの意味合いが異なる。
				err_code = write_thrlevels_2_samplingdata(pbs, algo_idx, 25 , 1, 0 ,0);	//LE17 通常
				//err_code = write_thrlevels_2_samplingdata(pbs, algo_idx, 40, 1, 0, 0);		//MRXCIS
			}
			else if (ALGO_TAPE_MECHA == algo_idx)
			{
				//メカ式テープ検知の閾値レベル設定
				err_code = write_thrlevels_2_samplingdata(pbs, algo_idx, 70, 40, 1, 1);	//通常
				//err_code = write_thrlevels_2_samplingdata(pbs, algo_idx, 91, 90, 1, 1);		//MRX用 チャネルが１つしかないので
			}
			else if (ALGO_MUTILATION == algo_idx)
			{
				//欠損
				err_code = write_thrlevels_2_samplingdata(pbs, algo_idx, 70, 40, 1, 1);	//通常
				//err_code = write_thrlevels_2_samplingdata(pbs, algo_idx, 70, 60, 0, 0);		//MRXのCZE用
			}
			else if( ALGO_DIFF_SERIES == algo_idx )	
			{
				//異番号検知
				err_code = write_thrlevels_2_samplingdata( pbs , algo_idx , 50 , 0 , 0 , 0 );	//通常
			}

			else
			{
				err_code = write_thrlevels_2_samplingdata(pbs, algo_idx, FIT_DEFAULT_LEVEL, UF_DEFAULT_LEVEL, RJ_DEFAULT_LEVEL, DEFAULT_MARGIN);	//サンプリングデータへ閾値レベルを書き込む
			}
		}

		pst_work->thrs_set_complete = 1;
		break;


	}
	return err_code;
}

/****************************************************************/
/**
* @brief	入力された値に対応するアルゴのレベルをサンプリングデータに設定する。

* @param[in]	サンプリングデータ
				アルゴのインデックス
				fit,Ufの閾値

* @param[out]	無
* @return		エラーコード
*/
/****************************************************************/
u32 write_thrlevels_2_samplingdata(ST_BS* pbs, u32 algo_idx, u8 thr1, u8 thr2, u8 thr3, u8 margin)
{

	if (ALGO_DYENOTE != algo_idx)			//値のエラーチェック(ダイノートは例外)
	{
		//fit
		if (MIN_LEVEL < thr1)
		{
			return E_FITNESS_THRESHOLD1;	//値が異常
		}
		else if (thr2 > thr1)
		{
			return E_FITNESS_THRESHOLD_OVER;	//1が2より大きい
		}

		//uf
		if (MIN_LEVEL < thr2)
		{
			return E_FITNESS_THRESHOLD2;	//値が異常
		}
	}

	//uf
	if (MIN_LEVEL < thr2)
	{
		return E_FITNESS_THRESHOLD2;	//値が異常
	}

	switch (algo_idx)
	{

	case ALGO_DOG_EARS:
		pbs->fitness[FITNESS_DOGEARS].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_DOGEARS].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_DOGEARS] = thr3;
		pbs->threshold_4[FITNESS_DOGEARS] = margin;

		break;

	case ALGO_TEAR:
		pbs->fitness[FITNESS_TEARS].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_TEARS].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_TEARS] = thr3;
		pbs->threshold_4[FITNESS_TEARS] = margin;
		break;

	case ALGO_DYENOTE:
		pbs->fitness[DYE_NOTE_].bit.threshold_1 = thr1;
		pbs->fitness[DYE_NOTE_].bit.threshold_2 = thr2;
		//pbs->threshold_3[DYE_NOTE_] = thr3;

		break;

	case ALGO_SOILING:
		pbs->fitness[FITNESS_SOILING].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_SOILING].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_SOILING] = thr3;
		pbs->threshold_4[FITNESS_SOILING] = margin;
		break;

	case ALGO_DEINKD:
		pbs->fitness[FITNESS_DE_INKED_NOTE].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_DE_INKED_NOTE].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_DE_INKED_NOTE] = thr3;
		pbs->threshold_4[FITNESS_DE_INKED_NOTE] = margin;
		break;

	case ALGO_STAIN:
		pbs->fitness[FITNESS_STAINS].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_STAINS].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_STAINS] = thr3;
		pbs->threshold_4[FITNESS_STAINS] = margin;
		break;

	case ALGO_HOLE:
		pbs->fitness[FITNESS_HOLES].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_HOLES].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_HOLES] = thr3;
		pbs->threshold_4[FITNESS_HOLES] = margin;
		break;

	case ALGO_CIR3:
		pbs->mid_res_3cir.thr_level = thr2;
		break;

	case ALGO_CIR4:
		pbs->mid_res_4cir.thr_level = thr2;
		break;

	case ALGO_MCIR:
		pbs->mid_res_mcir.thr_level = thr2;
		break;

	case ALGO_IR_CHECK:
		pbs->mid_res_ir_ck.thr_level = thr2;
		break;

	case ALGO_DOUBLE_CHECK:
		pbs->mid_res_double_ck.thr_level = thr2;
		break;

	case ALGO_NN1:
		pbs->mid_res_nn1.thr_level = thr2;
		break;

	case ALGO_NN2:
		pbs->mid_res_nn2.thr_level = thr2;
		break;

	case ALGO_IR2_SICPA:
		pbs->mid_res_ir2wave.thr_level = thr2;
		break;

	case ALGO_MAG:
		pbs->mid_res_mag.fit_thr = thr1;
		pbs->mid_res_mag.uf_thr = thr2;

		break;

	case ALGO_UV_VALIDATE:
		pbs->mid_res_uv_validate.fit_thr = thr1;
		pbs->mid_res_uv_validate.uf_thr = thr2;

		break;

	case ALGO_FOLD:
		pbs->fitness[FITNESS_FOLDS].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_FOLDS].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_FOLDS] = thr3;
		pbs->threshold_4[FITNESS_FOLDS] = margin;
		break;

	case ALGO_MUTILATION:
		pbs->fitness[FITNESS_MUTILATIONS].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_MUTILATIONS].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_MUTILATIONS] = thr3;
		pbs->threshold_4[FITNESS_MUTILATIONS] = margin;
		break;

	case ALGO_TAPE_MECHA:
		pbs->fitness[FITNESS_REPAIRS_MECHA].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_REPAIRS_MECHA].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_REPAIRS_MECHA] = thr3;
		pbs->threshold_4[FITNESS_REPAIRS_MECHA] = margin;
		break;

	case ALGO_TAPE_CAP:
		pbs->fitness[FITNESS_REPAIRS_CAP].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_REPAIRS_CAP].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_REPAIRS_CAP] = thr3;
		pbs->threshold_4[FITNESS_REPAIRS_CAP] = margin;
		break;

	case ALGO_NEOMAG:
		pbs->mid_res_neomag.thr_level = thr2;
		break;

	case ALGO_METAL_THREAD:
		pbs->fitness[FITNESS_MISSING_METAL_THREAD].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_MISSING_METAL_THREAD].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_MISSING_METAL_THREAD] = thr3;
		pbs->threshold_4[FITNESS_MISSING_METAL_THREAD] = margin;
		break;

	case ALGO_MAG_THREAD:
		pbs->fitness[FITNESS_MISSING_MAG_THREAD].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_MISSING_MAG_THREAD].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_MISSING_MAG_THREAD] = thr3;
		pbs->threshold_4[FITNESS_MISSING_MAG_THREAD] = margin;
		break;

	case ALGO_HOLOGRAM:
		pbs->fitness[FITNESS_MISSING_SECURITY_FEATURE].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_MISSING_SECURITY_FEATURE].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_MISSING_SECURITY_FEATURE] = thr3;
		pbs->threshold_4[FITNESS_MISSING_SECURITY_FEATURE] = margin;
		break;

	case ALGO_SP_A:
		pbs->mid_res_special_a.thr_level = thr2;
		break;

	case ALGO_SP_B:
		//pbs->mid_res_special_b.thr_level = thr2;
		break;

	case ALGO_LIMPNESS:
		pbs->fitness[FITNESS_LIMPNESS].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_LIMPNESS].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_LIMPNESS] = thr3;
		pbs->threshold_4[FITNESS_LIMPNESS] = margin;
		break;

	case ALGO_GRAFFITI:
		pbs->fitness[FITNESS_GRAFFITI].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_GRAFFITI].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_GRAFFITI] = thr3;
		pbs->threshold_4[FITNESS_GRAFFITI] = margin;
		break;

	case ALGO_CRUMPLE:
		pbs->fitness[FITNESS_CRUMPLES].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_CRUMPLES].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_CRUMPLES] = thr3;
		pbs->threshold_4[FITNESS_CRUMPLES] = margin;
		break;

	case ALGO_TAPE_MECHA_TC:
		if (thr1 < FITNESS_REPAIRS_MECHA_THRESH_MIN)
		{
			pbs->mid_res_tape.threshold = FITNESS_REPAIRS_MECHA_THRESH_MIN;
		}
		else if (FITNESS_REPAIRS_MECHA_THRESH_MAX < thr1)
		{
			pbs->mid_res_tape.threshold = FITNESS_REPAIRS_MECHA_THRESH_MAX;
		}
		else
		{
			pbs->mid_res_tape.threshold = thr1;
		}
		break;

	case ALGO_WASHED:
		pbs->mid_res_uv.fit_thr = thr1;
		pbs->mid_res_uv.uf_thr = thr2;

		pbs->fitness[FITNESS_WASHED].bit.threshold_1 = thr1;
		pbs->fitness[FITNESS_WASHED].bit.threshold_2 = thr2;
		pbs->threshold_3[FITNESS_WASHED] = thr3;
		pbs->threshold_4[FITNESS_WASHED] = margin;
		break;

	case ALGO_CF_NN_1COLOR:
		pbs->mid_res_cf_nn_1.thr_level = thr2;
		break;

	case ALGO_CF_NN_2COLOR:
		pbs->mid_res_cf_nn_2.thr_level = thr2;
		break;

	case ALGO_CUSTOM:
		pbs->mid_res_customcheck.thr_level = thr2;
		break;

	default:
		return	0;
	}

	return 0;

}

#ifdef _RM_
/****************************************************************/
/**
* @brief	入力されたテンプレート内券種IDに対応する通貨テーブルのインデックスを返す
			前提条件
			p_temp_currenct_table_top.tbl_ofsにtemplate内券種ID表のポインタを
			p_main_currenct_table_topにMain側通貨テーブルのポインタをセットしていること。

* @param[in]	テンプレート内券種ID
* @param[out]	無
* @return		インデックス ただし0xffは該当なし。
*/
/****************************************************************/
u32 templateID_2_noteID(u32 id)
{
#if 0
	CURRENCY_TABLE_ITEM* p_main_crncy;            //メイン側のカレンシーテーブル
	ST_CORR_ID_TABLE* p_temp_crncy;               //テンプレート側のカレンシーテーブル
	//u32 ret = 0;
	u8 num = p_temp_currenct_table_top.tbl_num; //カレンシーテーブルの要素数
	u8 main_curncy_idx = 255;                          //
	u8 idx = 0;                                 //
	u16 template_decision_id;                                     //
	//u32 current_note_id = 0x0;                  //ノートID格納用
	u8 	current_index = 0xff;                  //インデックス格納用

	u16 temp_id;                                //

	//2種類のテーブルのアドレスをセット
	p_temp_crncy = (ST_CORR_ID_TABLE*)p_temp_currenct_table_top.tbl_ofs;
	p_main_crncy = (CURRENCY_TABLE_ITEM*)p_main_currenct_table_top;

	//識別で求めたテンプレート内券種IDをセット
	template_decision_id = id;

	//配列0番目のtemplateIDをセットします。
	temp_id = p_temp_crncy[0].template_id;


	if (template_decision_id != TLPT_ID_INIT) //初期ID以外なら
	{                       //テンプレート側のカレンシーテーブルのIDと識別結果のIDを照合
		//方向情報を取ったテンプレート内券種ID
		//id =  pparams->note_id & 0x0fff;
		template_decision_id = template_decision_id & 0x0fff;
		main_curncy_idx = 0;

		//機種によってはカレンシーテーブルが存在しないので編集可能なtemplatetaskで照合を行う
		//1つのテンプレート内IDに複数のJCMIDが割り当てられてることを想定して、カウント変数(main_curncy_idx)を追加している。
		for (idx = 0; idx < num; ++idx)
		{
			if (temp_id != p_temp_crncy[idx].template_id)                   //記録した1つ前のIDと異なる場合
			{
				main_curncy_idx = main_curncy_idx + 1;                                //カウント変数をインクリメント
			}

			if (p_temp_crncy[idx].template_id == template_decision_id)                    //テンプレートidとid対応表が一致したら
			{
				//current_note_id = (u32)p_main_crncy[main_curncy_idx].note_id;    //メインの通貨テーブルからNoteIDを求める。
				current_index = (u32)p_main_crncy[main_curncy_idx].index;   	 //メインの通貨テーブルからインデックスを求める。
				break;
			}
			else                                                            //マッチしなかったら
			{
				temp_id = p_temp_crncy[idx].template_id;                    //現在のIDを記録する
			}
		}
	}

	return current_index;
#else
	return 0;
#endif
}

#endif


/****************************************************************/
/**
* @brief	引数を基に、カテゴリを判断する

* @param[in]	T_TEMPLATE_WORK *pst_work テンプレートシーケンス情報
* @param[out]	無
* @return		カテゴリ
*/
/****************************************************************/
u8 category_decision(T_TEMPLATE_WORK* pst_work)
{
	//ST_BS *pbs = work[pst_work->buf_n].pbs;

	if (pst_work->e_code != 0)
	{
		if (pst_work->e_code == REJ_NEW_DYE_INK || pst_work->e_code == REJ_OLD_DYE_INK)	//ダイノートならば
		{
			if (pst_work->ID_template != 0xF001)
			{
				return category2;	//category3
			}
			else
			{
				return category2;
			}
		}

		return category1;
	}
	else
	{
		return 0;
	}
}

/****************************************************************/
/**
* @brief	判定パラメタブロック内のオフセット計算を行う。
			実機（32bit）とシミュレータ（64bit）のオフセット計算方法の違いを吸収する。

* @param[in]	base_address
				offset
* @param[out]	無し
* @return		計算結果アドレス
*/
/****************************************************************/
u8* offset_address_cal(u8* base_point, u32 offset)
{
	u8* res_address;

#ifdef _RM_	//実機

	res_address = base_point + offset;

#else		//シミュレータ

	res_address = base_point + ((s32)offset);

#endif

	return res_address;
}

/****************************************************************/
/**
 * @brief 各処理の結果ブロックなどを参照して、レベル情報を配列に設定する。
 * IN : pst_work	テンプレートシェルのワークメモリ
 * IN : overall_res	各処理のレベルを設定する配列
 * OUT: overall_res	各処理のレベル　
 * ret: なし
 */
 /****************************************************************/
void	set_each_alog_level(T_TEMPLATE_WORK* pst_work, u8* overall_res)
{
	//結果ブロックの先頭アドレス
	T_DECISION_RESULT_COMMON* p_nowpara_res_blk = (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top;

	//各処理の専用構造体	随時追加する
	T_HOLE_RESULT*          pres_hole;		//穴
	T_TEAR_RESULT*          pres_tear;		//裂け
	T_DOG_EAR_RESULT*       pres_dog;		//角折れ
	T_STAIN_RESULT*         pres_sta;		//染み
	T_SOILING_RESULT*       pres_soi;		//汚れ
	T_DEINK_RESULT*         pres_deink;		//脱色
	T_DYENOTE_RESULT*       pres_dye;		//ダイノート
	T_DOUBLE_CHECK_RESULT*	pres_dbc;		//重券
	T_FOLDING_RESULT*		pres_folding;	//折れ
	T_UV_RESULT*            pres_uv;		//UV
	T_TAPE_RESULT*          pres_tape;		//メカ式テープ
	T_CAP_TAPE_RESULT*      pres_cap_tape;	//静電テープ
	T_NEOMAG_RESULT*        pres_neomag;	//neomag
	T_MAGTHREAD_RESULT*     pres_magthr;	//磁気スレッド
	T_METALTHREAD_RESULT*   pres_metalthr;	//金属スレッド
	T_HOLOGRAM_RESULT*      pres_hologram;	//ホログラム
	T_EXTRACT_CF_NN_1COLOR* pres_cfnn1;		//cfnn1
	T_EXTRACT_CF_NN_2COLOR* pres_cfnn2;		//cfnn2
	T_CUSTOMCHECK_RESULT*	pres_custom;	//ｶｽﾀﾑﾁｪｯｸ
	////T_IR2WAVE_RESULT* res_ir2wave;	//IR2
	//T_MAG_RESULT* res_mag;			//磁気

	ST_BS* pbs = work[pst_work->buf_n].pbs;	//サンプリングデータ

	u16 block_count = 0;
	u16 max = 0;
	s16 max_idx = -1; //20210729

	while (MAX_TEMPLATE_BLOCK_NAM > block_count)// 結果ブロック実行シーケンス　ループ
	{
		block_count++;

		switch (p_nowpara_res_blk->function_code)// 機能コードでマッチングするまで実行
		{
			//-------------------------鑑別---------------------------------------

		case FNC_CIR_3COLOR:								// ３色比
			//res_3cir = (T_CIR3_RESULT*)p_nowpara_res_blk;

			break;

		case FNC_CIR_4COLOR:								// 4色比
			//res_4cir = (T_CIR4_RESULT*)p_nowpara_res_blk;

			break;

		case FNC_IR_CHECK:									// IRCheck
			//res_irc = (T_IR_CHECK_RESULT*)p_nowpara_res_blk;
			break;


		case FNC_CUSTOM_CHECK:
			pres_custom = (T_CUSTOMCHECK_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_CUSTOM] = pres_custom->level;					// カスタムチェック
			break;

		case FNC_CIR_2COLOR:
			//overall_judge_proc(pst_work);					//２色差
			break;


		case FNC_DYE_NOTE:									//ダイノート
			pres_dye = (T_DYENOTE_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_DYENOTE] = pres_dye->level;
			break;


		case FNC_DOUBLE_CHECK:								//重券 正券：-値　、重券：+値
			pres_dbc = (T_DOUBLE_CHECK_RESULT*)p_nowpara_res_blk;

#ifndef NEW_DOUBLE_CHECK
			if (pres_dbc->invalid_data_count > 0)
			{
				overall_res[ALGO_DOUBLE_CHECK] = 1;
			}
			else
			{
				overall_res[ALGO_DOUBLE_CHECK] = 100;
			}
#else
			if (pres_dbc->invalid_data_count > 0)
			{
				overall_res[ALGO_DOUBLE_CHECK] = 1;
			}
			else
			{
				overall_res[ALGO_DOUBLE_CHECK] = 100;
			}
#endif
			break;



		case FNC_MCIR:										// MCIR
			overall_res[ALGO_MCIR] = pbs->mid_res_mcir.level;
			break;

		case FNC_VALI_NN_1COLOR:							// 鑑別１色NN
			overall_res[ALGO_NN1] = pbs->mid_res_nn1.calc_res_level;

			break;

		case FNC_VALI_NN_2COLOR:							// 鑑別２色NN
			overall_res[ALGO_NN2] = pbs->mid_res_nn2.calc_res_level;

			break;


		case FNC_DUAL_IR:		//IR2
			//overall_res[ALGO_IR2_SICPA] = 100;

			//if(0 != pbs->mid_res_ir2wave.result)	//偽造券判定なら
			//{
			//	overall_res[ALGO_IR2_SICPA] = 1;
			//}

			overall_res[ALGO_IR2_SICPA] = pbs->mid_res_ir2wave.level;

			break;

		case FNC_MAG:			//MAG
			overall_res[ALGO_MAG] = pbs->mid_res_mag.level;

			break;

		case FNC_NEOMAG:	//NEOMAG
			pres_neomag = (T_NEOMAG_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_NEOMAG] = 100;

			if (pres_neomag->judge != 0)
			{
				overall_res[ALGO_NEOMAG] = 1;
			}
			//overall_res[ALGO_NEOMAG] = ;
			break;

		case FNC_UV_VALIDATE:			//UV
			overall_res[ALGO_UV_VALIDATE] = pbs->mid_res_uv_validate.level;
			break;

		case FNC_CF_NN_1COLOR:    // 鑑別CF-NN 1 
			pres_cfnn1 = (T_EXTRACT_CF_NN_1COLOR*)p_nowpara_res_blk;
			overall_res[ALGO_CF_NN_1COLOR] = 100;
			if (pres_cfnn1->result == 1)
			{
				overall_res[ALGO_CF_NN_1COLOR] = 1;
			}

			break;

		case FNC_CF_NN_2COLOR:	 // 鑑別CF-NN 2 
			pres_cfnn2 = (T_EXTRACT_CF_NN_2COLOR*)p_nowpara_res_blk;
			overall_res[ALGO_CF_NN_2COLOR] = 100;
			if (pres_cfnn2->result == 1)
			{
				overall_res[ALGO_CF_NN_2COLOR] = 1;
			}
			break;

			//--------------正損-----------------


		case FNC_DOG_EAR:	//角折れ
			pres_dog = (T_DOG_EAR_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_DOG_EARS] = pres_dog->level;

			break;

		case FNC_TEAR:		//裂け
			pres_tear = (T_TEAR_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_TEAR] = pres_tear->level;
			break;

		case FNC_SOILING:							//汚れ
			pres_soi = (T_SOILING_RESULT*)p_nowpara_res_blk;
			//pbs->fitness[FITNESS_SOILING].bit.level  = res_soi->level;
			overall_res[ALGO_SOILING] = pres_soi->level;
			break;

		case FNC_DE_INK:							//脱色
			pres_deink = (T_DEINK_RESULT*)p_nowpara_res_blk;
			//pbs->fitness[FITNESS_DE_INKED_NOTE].bit.level  = res_deink->level;
			overall_res[ALGO_DEINKD] = pres_deink->level;
			break;

		case FNC_STAIN:								//染み
			pres_sta = (T_STAIN_RESULT*)p_nowpara_res_blk;
			//pbs->fitness[FITNESS_STAINS].bit.level  = res_sta->level;
			overall_res[ALGO_STAIN] = pres_sta->level;
			break;

		case FNC_HOLE:	//穴
			pres_hole = (T_HOLE_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_HOLE] = pres_hole->level;
			break;

		case FNC_FOLDING:		//折り畳み&欠損
			pres_folding = (T_FOLDING_RESULT*)p_nowpara_res_blk;
			max = 100;
			max_idx = -1;
			if (max > pres_folding->level[0] && pres_folding->up_res == 1)
			{
				max = pres_folding->level[0];
				max_idx = 0;
			}
			if (max > pres_folding->level[1] && pres_folding->down_res == 1)
			{
				max = pres_folding->level[1];
				max_idx = 1;
			}
			if (max > pres_folding->level[2] && pres_folding->left_res == 1)
			{
				max = pres_folding->level[2];
				max_idx = 2;
			}
			if (max > pres_folding->level[3] && pres_folding->right_res == 1)
			{
				max = pres_folding->level[3];
				max_idx = 3;
			}
			//pbs->fitness[FITNESS_FOLDS].bit.level = pres_folding->level[max_idx];
			if (max_idx == -1)
			{
				overall_res[ALGO_FOLD] = 100;
			}
			else
			{
				overall_res[ALGO_FOLD] = pres_folding->level[max_idx];
			}

			max = 100;
			max_idx = -1;
			if (max > pres_folding->level[0] && pres_folding->up_res == 4)
			{
				max = pres_folding->level[0];
				max_idx = 0;
			}
			if (max > pres_folding->level[1] && pres_folding->down_res == 4)
			{
				max = pres_folding->level[1];
				max_idx = 1;
			}
			if (max > pres_folding->level[2] && pres_folding->left_res == 4)
			{
				max = pres_folding->level[2];
				max_idx = 2;
			}
			if (max > pres_folding->level[3] && pres_folding->right_res == 4)
			{
				max = pres_folding->level[3];
				max_idx = 3;
			}
			//pbs->fitness[FITNESS_MUTILATIONS].bit.level = pres_folding->level[max_idx];
			if (max_idx == -1)
			{
				overall_res[ALGO_MUTILATION] = 100;
			}
			else
			{
				overall_res[ALGO_MUTILATION] = pres_folding->level[max_idx];
			}
			break;

		case FNC_MUTILATION:		//欠損
			break;


		case FNC_TAPE:// テープ(メカ式)
			pres_tape = (T_TAPE_RESULT*)p_nowpara_res_blk;
			//pbs->fitness[FITNESS_REPAIRS_MECHA].bit.level = res_tape->level;
			overall_res[ALGO_TAPE_MECHA] = pres_tape->level;
			break;

		case FNC_CAP_TAPE://静電テープ
			pres_cap_tape = (T_CAP_TAPE_RESULT*)p_nowpara_res_blk;
			//pbs->fitness[FITNESS_REPAIRS_CAP].bit.level = pres_cap_tape->level;
			overall_res[ALGO_TAPE_CAP] = pres_cap_tape->level;
			break;

		case FNC_MAGTHREAD:	//磁気スレッド FITNESS_MISSING_MAG_THREAD
			pres_magthr = (T_MAGTHREAD_RESULT*)p_nowpara_res_blk;
			//pbs->fitness[FITNESS_MISSING_MAG_THREAD].bit.level = pres_magthr->level;
			overall_res[ALGO_MAG_THREAD] = pres_magthr->level;
			break;

		case FNC_METALTHREAD:	//金属スレッド FITNESS_MISSING_METAL_THREAD
			pres_metalthr = (T_METALTHREAD_RESULT*)p_nowpara_res_blk;
			//pbs->fitness[FITNESS_MISSING_METAL_THREAD].bit.level = pres_metalthr->level;
			overall_res[ALGO_METAL_THREAD] = pres_metalthr->level;
			break;

		case FNC_HOLOGRAM:		//ホログラム
			pres_hologram = (T_HOLOGRAM_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_HOLOGRAM] = pres_hologram->level;
			break;

		case FNC_WASH:			//UV
			pres_uv = (T_UV_RESULT*)p_nowpara_res_blk;
			overall_res[ALGO_WASHED] = pres_uv->level;
			break;

		case FNC_GRAFFITI_TEXT:			//落書き

			overall_res[ALGO_GRAFFITI] = (u8)pbs->fitness[FITNESS_GRAFFITI].bit.level;
			break;

		case FNC_OVERALL_JUDGE:
			break;

		case FNC_END:
			return;

		default:
			break;
		}

		//次の処理の先頭アドレスへ
		p_nowpara_res_blk = (T_DECISION_RESULT_COMMON*)((u8*)p_nowpara_res_blk + p_nowpara_res_blk->next_block_ofs);
	}
}

/****************************************************************/
/**
* @brief	複合NN判定関数
			複数あるNN判定の結果から正しい金種を判定する。
			個々のNNからの出力は、正常発火と発火不良の判定結果。正常発火の場合は発火値とノード、テンプレート内券種IDを結果ブロックに記録する。
			前提としてNNは対象金種以外はすべてリジェクトノードになることとする。
			事前に実行した識別NNの内1つだけが正常発火でリジェクトノード判定でなければ、正常終了とする。
			発火不良が複数ある場合、最後に参照した識別NNの結果のエラーコードがが出力される。
			発火不良とリジェクトノード判定両方がある場合は、リジェクトノード判定が出力される。

* @param[in]	pst_work
* @param[out]	正常　：pst_work->ID_templateにテンプレート内券種IDをセットする。
				エラー：pst_work->e_code にエラーコードをセットする。
* @return		無し
*/
/****************************************************************/
u8 multi_nn(T_TEMPLATE_WORK* pst_work)
{
	//#define TEMPLATE_MULTI_NN_MAX_COUNT 20
#define TEMPLATE_MULTI_NN_SIZE_ERR		0	//発火したNNが無い。サイズエラー
#define TEMPLATE_MULTI_NN_NORMAL		1	//正常発火			
#define TEMPLATE_MULTI_NN_OUT_OUT_ERR	2	//発火が複数ある。	発火エラー

	//結果ブロックの先頭アドレス
	T_DECISION_RESULT_COMMON* p_nowpara_res_blk = (T_DECISION_RESULT_COMMON*)pst_work->p_result_blk_top;
	u32	block_count = 0;
	T_NN_RESULT empty_block = { 0 };
	T_NN_RESULT* nn_res;
	T_NN_RESULT* ans_nn_res = &empty_block;
	u8	normal_nn_count = 0;
	u8	end_flg = 0;
	u32 reject_id_check = 0;
	u16 temp_res_code = 0;
	u8	reject_note_flg = 0;

	while (MAX_TEMPLATE_BLOCK_NAM != block_count && end_flg != 1)	// 結果ブロック実行シーケンス　ループ
	{																//
		block_count++;												//無限ループ防止用
																	//
		switch (p_nowpara_res_blk->function_code)					// 機能コードでマッチングするまで実行
		{															//
		case FNC_NN_JUDGE:											// 識別NNの結果ブロック参照
			nn_res = (T_NN_RESULT*)p_nowpara_res_blk;				// 結果ブロック参照
			if (nn_res->res_code == 0)								// 識別NNのリザルトコードが0（エラー無）の時
			{														//
				reject_id_check = nn_res->comn.tmplt_ID & 0x0F00;	// テンプレート内券種IDのリジェクトノードチェック
				if (reject_id_check != 0)							// リジェクトノード判定の以外の場合
				{													//
					ans_nn_res = nn_res;							// 正常発火の情報を保存する。
					normal_nn_count++;								// 正常発火NN数をカウントする
				}													//
				else												// リジェクト判定の場合
				{													//
					reject_note_flg = 1;							// リジェクトノード判定フラグを立てる。
				}													//
			}														//
			else													// 識別NNのリザルトコードが0以外（エラーあり）の時
			{														//
				temp_res_code = nn_res->res_code;					// NNのリザルトコードを更新する。
			}														//
			break;													//
																	//
		case FNC_END:												// エンドマークの時
			end_flg = 1;											// ループ終了フラグを立てる。
			break;
		}

		//次の処理の先頭アドレスへ
		p_nowpara_res_blk = (T_DECISION_RESULT_COMMON*)((u8*)p_nowpara_res_blk + p_nowpara_res_blk->next_block_ofs);

	}

	if (TEMPLATE_MULTI_NN_SIZE_ERR == normal_nn_count			//NN判定を1回も実行しなかったとき
		&& reject_note_flg == 0 && temp_res_code == 0)			//
	{
		pst_work->e_code = ERR_SIZE_MISS_MACH;					//サイズエラーとする
	}
	else if (TEMPLATE_MULTI_NN_NORMAL == normal_nn_count)		//正常発火したNNが１つだけの場合
	{
		pst_work->ID_template = ans_nn_res->comn.tmplt_ID;		//テンプレート内券種IDを更新
		set_dir_params(pst_work, 0);								//方向情報を設定する。
	}
	else if (TEMPLATE_MULTI_NN_OUT_OUT_ERR <= normal_nn_count)	//正常発火したNNが２つ以上の場合
	{
		pst_work->e_code = ERR_MULTI_NN;						//発火エラーとする。
	}
	else if (reject_note_flg == 1)								//リジェクトノード判定の場合
	{
		work[pst_work->buf_n].pbs->category_ecb = category1;	//カテゴリ１を設定し
		pst_work->e_code = RES_ALLOVER_REJECT_NODE;				//リザルトコードを設定する。
	}
	else if (temp_res_code != 0)									//発火不良がある場合
	{
		pst_work->e_code = temp_res_code;						//最後に参照したブロックのエラーコードをセットする。
	}

	return 0;
}

/****************************************************************/
/**
* @brief		方向パラメタ設定関数
				NNで得られた方向情報を基にパラメタを設定する。

* @param[in]	pst_work
*				flg			座標変換パラメータの計算実行の有無　1：しない　0：する
*
* @param[out]	正常　：work[pst_work->buf_n].pbs->insertion_directionに方向コードをセットします。
				エラー：pst_work->e_code にエラーコードをセットする。
* @return		無し
*/
/****************************************************************/
void set_dir_params(T_TEMPLATE_WORK* pst_work, s8 flg)
{
#ifdef _RM_
	u32 index = 255;
#endif
	s32 i = 0;
	T_TEMP_SHELL_JCMID_TBL* id_tbl;
	id_tbl = (T_TEMP_SHELL_JCMID_TBL*)p_temp_currenct_table_top.tbl_ofs;

	//方向を決定する
	if (pst_work->e_code == 0)
	{
		switch (pst_work->ID_template & 0xf000)
		{
		case 0x1000:
			work[pst_work->buf_n].pbs->insertion_direction = W0;
			break;

		case 0x2000:
			work[pst_work->buf_n].pbs->insertion_direction = W1;
			break;

		case 0x4000:
			work[pst_work->buf_n].pbs->insertion_direction = W2;
			break;

		case 0x8000:
			work[pst_work->buf_n].pbs->insertion_direction = W3;
			break;

		default:
			work[pst_work->buf_n].pbs->insertion_direction = W0;
			break;
		}
	}
	else
	{
		work[pst_work->buf_n].pbs->insertion_direction = 0xff;
		return;
	}

#ifdef _RM_
	//Main通貨テーブルの閾値情報をサンプリングデータ内に書き込む
	if (pst_work->thrs_set_complete == 0 && pst_work->ID_template != TLPT_ID_INIT)	//既に設定済み、
	{																				//または、初期ID（識別失敗）ならばスキップ
		index = templateID_2_noteID(pst_work->ID_template);
		replay_fitness_work.noteid[pst_work->buf_n] = index;

		//if(255 != index)
		//{
		pst_work->e_code = set_fitness_thrs(pst_work, index, SET_CURRENCY_THRS);
		//}
		//else
		//{
		//	pst_work->e_code = E_ID_TRANS_ERR;	//ID変換エラー
		//}
	}
#endif

	//JCMIDをサンプリングデータに書き込む
	for (i = 0; i < p_temp_currenct_table_top.tbl_num; ++i)
	{
		if (id_tbl[i].template_id == (pst_work->ID_template & 0x0fff))
		{
			work[pst_work->buf_n].pbs->mid_res_nn.result_jcm_id = id_tbl[i].jcm_id;
			replay_fitness_work.jcmid[pst_work->buf_n] = id_tbl[i].jcm_id;
			break;
		}
	}

	//方向補正関係の係数を決定する。
	if (flg == 0)
	{
		advance_insert_correction(pst_work->buf_n);
		decision_plane_side_in_insertion_direction(pst_work->buf_n);
	}
}


/****************************************************************/
/**
* @brief		総合判定用の関数で個々の鑑別結果と閾値レベルを計算して、
*				最終結果を求める。
*
* @param[in]	must_impact_uf			損券判定結果最も影響のある処理番号
*				correction_func_index	閾値補正処理に用いる処理中のアルゴ番号
*				result_bit_func_index	鑑別結果のビット対応処理に用いる処理番号
*				res_level				各処理が求めた出力レベル
*				judge_params1/2			カテゴリ判定を制御するフラグ
*				thr3					閾値３レベル
*
* @param[out]	presult					判定結果（0~2）
*				pthr2					その処理の閾値2レベル
*				pcf/uf/fp_counter		カウントアップすべき判定のカウンター
*				pvalidation_result_flg	鑑別結果のビットを書き込むメモリ
* @return
*/
/****************************************************************/
u8 validation_each_final_judge(u8 must_impact_uf, u8 correction_func_index, u8 result_bit_func_index, u8 res_level, u8 judge_params1, u8 judge_params2, u8 thr3, u8 vali_level_mode,
	u8* presult, u8* pthr2, u8* pcf_counter, u8* puf_counter, u8* pfp_counter, u32* pvalidation_result_flg)
{
	u8 temp_th1_level = 0;

	//正損結果に応じてth2の値を変更する。
	change_validation_thr_level(must_impact_uf, correction_func_index, pthr2, vali_level_mode);

	if (*pthr2 == TEMPLATE_OVERALL_NOT_INCLUDED_IN_RESULTS)	                 //th2が0の時
	{												                         //
		temp_th1_level = 0;							                         //th1も0にする　
	}												                         //
	else											                         //0以外の時は
	{												                         //
		temp_th1_level = MIN_LEVEL;					                         //通常に処理する。鑑別はTELLERがないのTH1は最小レベルで良い
	}

	*presult = level_result_detect(res_level, temp_th1_level, *pthr2, thr3); //判定結果を求める（0~2）

	if (UF == *presult)				                                         //偽造券判定なら
	{                                                                        //
		judge_counter(judge_params1, pcf_counter, puf_counter, pfp_counter); //設定に応じてカウンターをインクリメント
		*pvalidation_result_flg |= 1 << result_bit_func_index;               //鑑別結果ビット対応処理
	}
	else if (REJ == *presult)		                                         //リジェクト判定なら
	{                                                                        //
		judge_counter(judge_params2, pcf_counter, puf_counter, pfp_counter); //設定に応じてカウンターをインクリメント
		*pvalidation_result_flg |= 1 << result_bit_func_index;               //鑑別結果ビット対応処理
	}
	return 0;
}

/****************************************************************/
/**
* @brief		総合判定用の関数で個々の正損結果と閾値レベルを計算して、
*				最終結果を求める。
*
* @param[in]	pbs					サンプリングデータ
*				original_res		正損再現処理前のレベル
*				overall_res			正損再現処理後のレベル
*				replay_func_index	正損再現処理時のアルゴ番号
*				algo_index			アルゴ番号
*				impact_uf_index		損券判定結果最も影響のある処理番号
*				judge_params1/2		カテゴリ判定を制御するフラグ
*				vali_index			特徴剥がれの際に用いる閾値緩和処理用のアルゴ番号
*
* @param[out]	pmust_impact_uf		影響の大きい正損処理のアルゴ番号
*				pcf/uf/fp_counter	カウントアップすべき判定のカウンター
* @return		なし
*/
/****************************************************************/
u8 unfit_each_final_judge(ST_BS* pbs, u8* original_res, u8* overall_res, u8 replay_func_index, u8 algo_index, u8 impact_uf_index, u8 judge_params1, u8 judge_params2, u8 vali_index, u8 vali_level_mode,
	u8* pmust_impact_uf, u8* pcf_counter, u8* puf_counter, u8* pfp_counter)
{
	u8 temp_th2 = 0;
	//u8 deink_th3;

	pbs->fitness_result_original[replay_func_index - 1] = original_res[algo_index];		//元レベルをサンプリングデータに書き込む
	pbs->fitness[replay_func_index].bit.level = overall_res[algo_index];		//正損再現で書き換えたレベルをサンプリングデータに書き込む

	//特例の処理を行う場合はここに記載する。
	switch (algo_index)
	{
	case ALGO_DEINKD:
		temp_th2 = (u8)(pbs->fitness[replay_func_index].bit.threshold_2	//UF判定だけでは厳しすぎるので
			- (pbs->fitness[replay_func_index].bit.threshold_2 / 4));	//鑑別を甘くする閾値を計算する

		if (temp_th2 == 0)
		{
			temp_th2 = 1;
		}

		//deink_th3 = pbs->threshold_3[replay_func_index];
		//pbs->threshold_3[replay_func_index] = temp_th2;

		break;

	case ALGO_MAG_THREAD:
	case ALGO_METAL_THREAD:
	case ALGO_HOLOGRAM:

		//正損結果に応じてth2の値を変更する。
		temp_th2 = (u8)pbs->fitness[replay_func_index].bit.threshold_2;			//ビットフィールドではポインタ渡しできないので変数に置き換える
		change_validation_thr_level(*pmust_impact_uf, vali_index, &temp_th2, vali_level_mode);
		pbs->fitness[replay_func_index].bit.threshold_2 = temp_th2;

		////正損結果に応じてth3の値を変更する。
		//temp_th2 = (u8)pbs->threshold_3[replay_func_index];					//ビットフィールドではポインタ渡しできないので変数に置き換える
		//change_validation_thr_level(*pmust_impact_uf, vali_index ,&temp_th2);
		//pbs->threshold_3[replay_func_index] = temp_th2;

		break;

	default:
		break;
	}

	//書き換えたデータで閾値判定を行う。resultに0~2が入る。
	pbs->fitness[replay_func_index].bit.result = level_result_detect(
		(u8)pbs->fitness[replay_func_index].bit.level,			//出力レベル
		(u8)pbs->fitness[replay_func_index].bit.threshold_1,	//閾値１
		(u8)pbs->fitness[replay_func_index].bit.threshold_2,    //閾値２
		pbs->threshold_3[replay_func_index]);                   //閾値３

	if (UF == pbs->fitness[replay_func_index].bit.result)						//偽造券判定なら
	{
		judge_counter(judge_params1, pcf_counter, puf_counter, pfp_counter);	//設定に応じてカウンターをインクリメント
		if ((*pmust_impact_uf < impact_uf_index) && algo_index != ALGO_DEINKD)	//影響度の高い損券ならば（脱色以外）
		{																		//
			*pmust_impact_uf = impact_uf_index;									//値を更新する
		}                                                                       //

		else if ((*pmust_impact_uf < impact_uf_index) && algo_index == ALGO_DEINKD && temp_th2 >= (u8)pbs->fitness[replay_func_index].bit.level)
			//脱色のみ少し条件をきびしくしてます。
		{																		//
			*pmust_impact_uf = impact_uf_index;									//値を更新する
		}                                                                       //

	}
	else if (REJ == pbs->fitness[replay_func_index].bit.result)					//リジェクト判定なら
	{
		judge_counter(judge_params2, pcf_counter, puf_counter, pfp_counter);	//設定に応じてカウンターをインクリメント
		if (*pmust_impact_uf < impact_uf_index)									//影響度の高い損券ならば
		{																		//
			*pmust_impact_uf = impact_uf_index;									//値を更新する
		}																		//
	}

	return 0;
}

/****************************************************************/
/**
* @brief		judge_paramsの設定に応じてcf/uf/fpカウンターをインクリメントする
* @param[in]	judge_params		カテゴリ判定を制御するフラグ
* @param[out]	cf/uf/fp_counter	各カテゴリのカウンタ
* @return
*/
/****************************************************************/
void judge_counter(u8 judge_params, u8* pcf_counter, u8* puf_counter, u8* pfp_counter)
{
	if (judge_params == JUDGE_INVALID)
	{
		return;
	}
	else if (judge_params == JUDGE_CF)
	{
		*pcf_counter = *pcf_counter + 1;
		return;
	}
	else if (judge_params == JUDGE_UF)
	{
		*puf_counter = *puf_counter + 1;
		return;
	}
	else if (judge_params == JUDGE_FP)
	{
		*pfp_counter = *pfp_counter + 1;
		return;
	}
}

/****************************************************************/
/**
* @brief		ダメージ系の大損券（角折れ、裂け、折り畳み、欠損、穴）が
*				カテゴリ2/3になってしまう場合があるので、カテゴリ1に修正する。
*				サンプリングデータ内にある出力レベルがTH3以下の場合、書き換えが適応される。
*
* @param[in]	pbs					サンプリングデータ
*				pst_work			テンプレートシェル用構造体
* @param[out]	pbs->category_ecb	カテゴリ判定
* @return		なし
*/
/****************************************************************/
void	big_uf_relief(ST_BS* pbs, T_TEMPLATE_WORK* pst_work)
{
	s32 fitness_algo_idx;

	if (!((pbs->category_ecb == category3)		        //カテ2or3以外の場合
		|| (pbs->category_ecb == category2)))	        //
	{	                                                //
		return;	                                        //処理しない。
	}

	for (fitness_algo_idx = 0; fitness_algo_idx < FITNESS_ALGO_NUM; ++fitness_algo_idx)
	{
		switch (fitness_algo_idx)
		{
		case FITNESS_MISSING_MAG_THREAD:		              //磁気スレッド、
		case FITNESS_MISSING_METAL_THREAD:		              //金属スレッド、
		case FITNESS_MISSING_SECURITY_FEATURE:	              //ホログラム剥がれ
		case FITNESS_REPAIRS_MECHA:							  //メカ式テープ検知
		case FITNESS_DE_INKED_NOTE:							  //脱色検知、
															  //この処理の対象外
			break;

		default:									          //対象の処理で
															  //TH3が0より大きい場合
			if (pbs->threshold_3[fitness_algo_idx] != TEMPLATE_OVERALL_NOT_INCLUDED_IN_RESULTS)
			{									              //出力レベルがTH3以下の場合 かつ 出力レベルが１以上の時
				if (pbs->fitness[fitness_algo_idx].bit.level <= pbs->threshold_3[fitness_algo_idx] &&
					pbs->fitness[fitness_algo_idx].bit.level >= 1)
				{									          //
					pbs->category_ecb = category1;	          //カテ1に書き換える
					pst_work->e_code = RES_ALLOVER_BIG_UNFIT; //リザルトコードをセット
					return;
				}
			}
			break;
			
		}
	}

	return;
}



//End of file

