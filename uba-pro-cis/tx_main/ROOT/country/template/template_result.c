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
/*																			*/
/* MODEL NAME : 識別共有*/
/* @file template_result.c*/
/* @brief  テンプレートシーケンスの実装ファイルです。*/
/* @date 2018.08.220*/
/* @author JCM. OSAKA TECHNICAL RESARCH 1 GROUP ELEMENTAL TECHNOLOGY RECERCH DEPT.*/
/****************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <string.h>

#define EXT

#include "../common/global.h"

// プロトタイプ宣言


/****************************************************************/
/**
 * @brief		外形検知ブロック処理
 *@param[in]	work メモリーのポインタ 
 *@param[out]	<出力引数arg1の説明]>
 * @return			 
 */
/****************************************************************/
void	oline_det_result(u8 buf ,ST_EdgEDetecT *ED)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_OUTLINE_RESULT*	oline_blk_p = 0;
	s8 err_code = 0;

		oline_blk_p = (T_OUTLINE_RESULT*)add_result_blk(	// 結果追加
					sizeof(T_OUTLINE_RESULT),			// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);	

//	if(((T_DECISION_PARAMETER_OUTLINE*)(oline_blk_p->comn.source_blk_ofs + (u32)oline_blk_p))->ei_wr != 0)	// 中間情報に書くフラグのチェック。0以外で書き込む
//	{
//		//処理連番
//		pnote_dt[buf_num]->angle = ED->SKEW;	//スキュー角
//		pnote_dt[buf_num]->center_x = ED->AlignedBillCenterPoint.x;	//紙幣の中心座標x
//		pnote_dt[buf_num]->center_y = ED->AlignedBillCenterPoint.y;	//紙幣の中心座標y
//		pnote_dt[buf_num]->note_x_size = ED->length_dot;	//主走査方向幅
//		pnote_dt[buf_num]->note_y_size = ED->width_dot;	//主走査方向幅
//		pnote_dt[buf_num]->left_up_x = ED->AlignedUpperLeft.x;	//頂点左上ｘ
//		pnote_dt[buf_num]->left_up_y = ED->AlignedUpperLeft.y;	//頂点左上ｙ
//		pnote_dt[buf_num]->left_down_x = ED->AlignedLowerLeft.x;	//頂点左下ｘ
//		pnote_dt[buf_num]->left_down_y = ED->AlignedLowerLeft.y;	//頂点左下ｙ
//		pnote_dt[buf_num]->right_up_x = ED->AlignedUpperRight.x;	//頂点右上ｘ
//		pnote_dt[buf_num]->right_up_y = ED->AlignedUpperRight.y;	//頂点右上ｙ
//		pnote_dt[buf_num]->right_down_x = ED->AlignedLowerRight.x;	//頂点右下ｘ
//		pnote_dt[buf_num]->right_down_y = ED->AlignedLowerLeft.y;	//頂点右下ｙ
//	}
//	else
//	{
//		//何もしない
//	}

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_outline.proc_time = oline_blk_p->comn.proc_time;

	// 機能固有部　結果セット	頂点座標をセットする
	oline_blk_p->note_x_length         = (s16)ED->length_dot;			// 200dpi単位 物理座標
	oline_blk_p->note_y_length         = (s16)ED->width_dot;			// 200dpi単位 物理座標
	oline_blk_p->skew_val              = ED->SKEW;// スキュー角
	oline_blk_p->center_x              = (s16)ED->AlignedBillCenterPoint.x;	// 200dpi単位 物理座標
	oline_blk_p->center_y              = (s16)ED->AlignedBillCenterPoint.y;	// 200dpi単位 物理座標
	oline_blk_p->vertex_top_left_x     = (s16)ED->AlignedUpperLeft.x;// 頂点座標　物理座標 top left x	200dpi
	oline_blk_p->vertex_top_left_y     = (s16)ED->AlignedUpperLeft.y;// 頂点座標　物理座標 top left y	200dpi
	oline_blk_p->vertex_top_right_x    = (s16)ED->AlignedUpperRight.x;// 頂点座標　物理座標 top right x	200dpi
	oline_blk_p->vertex_top_right_y    = (s16)ED->AlignedUpperRight.y;// 頂点座標　物理座標 top right y	200dpi
	oline_blk_p->vertex_bottom_left_x  = (s16)ED->AlignedLowerLeft.x;// 頂点座標　物理座標 bottom left x		200dpi
	oline_blk_p->vertex_bottom_left_y  = (s16)ED->AlignedLowerLeft.y;// 頂点座標　物理座標 bottom left y		200dpi
	oline_blk_p->vertex_bottom_right_x = (s16)ED->AlignedLowerRight.x;// 頂点座標　物理座標 bottom right x	200dpi
	oline_blk_p->vertex_bottom_right_y = (s16)ED->AlignedLowerRight.y;// 頂点座標　物理座標 bottom right y	200dpi
	oline_blk_p->padding[0]            = 0xff;							//padding
	oline_blk_p->padding[1]            = 0xff;							//padding

}

/****************************************************************/
/**
 * @brief		ポイントデータ抽出　結果処理
 *@param[in]	work メモリーのポインタ 
 *@param[out]	<出力引数arg1の説明]>
 * @return			 
 */
/****************************************************************/
void	extraction_pointdate_result(u8 buf, ST_DATA_EXTRACTION *data_extraction)
{	
	T_TEMPLATE_WORK* pst_work = &st_work[buf];
	T_EXTRACT_POINT_DT*	extr_point_blk = 0;			//ポイント抽出　結果ブロック
	u16 blksiz = 0;
	s8 err_code = 0;

	//処理結果	
	blksiz = sizeof(T_EXTRACT_POINT_DT);	// 固定長部と,
	blksiz += calculate_size_including_padding_data(sizeof(s16)*data_extraction->count);	// 可変長部（パディングを含む）の合計サイズを計算する。

	extr_point_blk =(T_EXTRACT_POINT_DT*)add_result_blk(	// 結果追加
					blksiz,									// 追加する領域サイズ
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	// 機能固有項固定長部情報を書き込む
	extr_point_blk->ei_corner = data_extraction->corner_flg;				// コーナー有効無効フラグのコピー
	extr_point_blk->extract_point_dt_ofs = sizeof(T_EXTRACT_POINT_DT);	// 可変長データである抽出ポイントデータのこの結果ブロック先頭からのオフセット値
	extr_point_blk->extract_point_dt_num = data_extraction->count;	// 可変長データである抽出ポイントデータの要素数
	extr_point_blk->plane_count = data_extraction->plane_count;
	memcpy(extr_point_blk->plane_lst, data_extraction->plane_lst, PLANE_LST_MAX);
	//extr_point_blk->padding[1] = 0xff;

	// 可変長部1をコピーする。（パディング含まず）
	memcpy((u8*)((u8*)extr_point_blk + extr_point_blk->extract_point_dt_ofs), data_extraction->data_extra_out_put, sizeof(s16)*data_extraction->count);	// 可変長データを追加

	//サンプリングデータに処理時間を書き込む
	//NNの処理時間にポイント抽出の時間も加える
	work[buf].pbs->mid_res_outline.proc_time_point_extra += extr_point_blk->comn.proc_time;	
}

/****************************************************************/
/**
 * @brief		NN判定　結果処理
 *@param[in]	work メモリーのポインタ 
 *@param[out]	<出力引数arg1の説明]>
 * @return			 
 */
/****************************************************************/
u16	NN_judeg_proc_result(u8 buf ,ST_NN* nn_cfg, ST_NN_RESULT *nn_res)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];

	T_DECISION_PARAMETER_NN*	p_nn_blk = 0;
	T_NN_RESULT* p_nn_result = 0;
	T_DECISION_PARAMETER_NODE_INF*	p_nn_node_inf = 0;
	ST_NN_NODE_THRESHOLD p_nn_node_thrs;
	//T_TEMP_SHELL_JCMID_TBL*  id_tbl;
	//ST_BS* pbs = work[buf].pbs;
	u16 blksiz = 0;
	u16 err_code = 0;
	//u8	i = 0;

	//id_tbl = (T_TEMP_SHELL_JCMID_TBL *)p_temp_currenct_table_top.tbl_ofs;

	//memcpy(&id_tbl, &p_temp_currenct_table_top.tbl_ofs, p_temp_currenct_table_top.tbl_num * sizeof(T_TEMP_SHELL_JCMID_TBL));

	// パラメタ取得
	p_nn_blk = (T_DECISION_PARAMETER_NN*)pst_work->now_para_blk_pt;	//判定パラメタ 共通項

	//可変長部計算
	blksiz = sizeof(T_NN_RESULT);	// 固定長部と,
	blksiz += calculate_size_including_padding_data(sizeof(float) * (p_nn_blk->NN_out_num));// 可変長部（パディングを含む）の合計サイズを計算する。
	
	p_nn_result = (T_NN_RESULT*)add_result_blk(	// 結果追加
					blksiz,						// 追加する領域サイズ
					pst_work,					// workのポインタ
					err_code,					// 終了状態
					pst_work->ID_template		// 出力券種ID
					);

	p_nn_node_inf = (T_DECISION_PARAMETER_NODE_INF*)((u8*)p_nn_blk + p_nn_blk->node_inf_ofs);

	//発火値の確認
	//p_nn_node_thrs.th1 = p_nn_node_inf[p_nn_result->result_1st_node].first_val;
	//p_nn_node_thrs.th2 = p_nn_node_inf[p_nn_result->result_1st_node].deff_val;
	p_nn_node_thrs.th1 = 0.5f; //0.5 ⇒　0.4
	p_nn_node_thrs.th2 = 0.25f;//0.25
	p_nn_node_thrs.th3 = p_nn_node_inf[p_nn_result->result_1st_node].th3_val;
	p_nn_node_thrs.th5 = p_nn_node_inf[p_nn_result->result_1st_node].th5_val;

	//発火不良の調査
	nn_res->reject = (u16)out_put_value_cal(&p_nn_node_thrs , nn_res);
	err_code = nn_res->reject;

	//判定結果書き込み
	p_nn_result->result_1st_node = (u16)nn_res->output_max_node;	// 最大発火ノード
	//p_nn_result->result_1st_node = 36;	// 最大発火ノード
	p_nn_result->result_2nd_node = (u16)nn_res->output_2nd_node;	// 2nd発火ノード
	
	p_nn_result->max_node_val	= nn_res->output_max_val;	// 最大発火値
	p_nn_result->diff			= nn_res->output_max_val - nn_res->output_2nd_val;	// 差分値
	p_nn_result->softmax		= nn_res->th3;				// ソフトマックス
	p_nn_result->error			= nn_res->th5;				// エラー値
	//
	p_nn_result->output_node_ofs = sizeof(T_NN_RESULT);	// 可変長データであるNN出力ノードデータのこの結果ブロック先頭からのオフセット値
	p_nn_result->output_node_num =  nn_cfg->out_nide;	// 可変長データであるNN出力ノードの要素数
	p_nn_result->res_code		 = err_code;				//リザルトコード
	
	//中間情報に書き込む
	work[buf].pbs->mid_res_nn.proc_time		 = p_nn_result->comn.proc_time;	
	work[buf].pbs->mid_res_nn.output_max_val = p_nn_result->max_node_val;	
	work[buf].pbs->mid_res_nn.output_2nd_val = p_nn_result->diff;	
	work[buf].pbs->mid_res_nn.softmax_val	 = p_nn_result->softmax;		
	work[buf].pbs->mid_res_nn.error_val      = p_nn_result->error;		
	work[buf].pbs->mid_res_nn.max_node_num   = p_nn_result->result_1st_node;
	work[buf].pbs->mid_res_nn._2nd_node_num  = p_nn_result->result_2nd_node;
	work[buf].pbs->mid_res_nn.th1			 = p_nn_node_thrs.th1;
	work[buf].pbs->mid_res_nn.th2			 = p_nn_node_thrs.th2;
	//work[buf].pbs->mid_res_nn.th3			 = p_nn_node_thrs.th3;
	//work[buf].pbs->mid_res_nn.th5			 = p_nn_node_thrs.th5;
	work[buf].pbs->mid_res_nn.chk_sums[0]	 = nn_res->deb_float_sum[0];
	work[buf].pbs->mid_res_nn.chk_sums[1]	 = nn_res->deb_float_sum[1];
	work[buf].pbs->mid_res_nn.chk_sums[2]	 = nn_res->deb_float_sum[2];
	work[buf].pbs->mid_res_nn.chk_sums[3]	 = nn_res->deb_float_sum[3];
	work[buf].pbs->mid_res_nn.chk_sums[4]	 = nn_res->deb_float_sum[4];
	work[buf].pbs->mid_res_nn.chk_sums[5]	 = nn_res->deb_float_sum[5];
	work[buf].pbs->mid_res_nn.chk_sums[6]	 = nn_res->deb_float_sum[6];


	// 各出力ノード発火値をコピーする。（パディング含まず）
	memcpy((u8*)((u8*)p_nn_result + p_nn_result->output_node_ofs), (u8*)nn_cfg->output, p_nn_result->output_node_num *  sizeof(float));	// 可変長データを追加
 
	//発火不良でなければ現在のテンプレートIDを更新する
	if (err_code == 0)
	{
		pst_work->now_result_blk_pt->tmplt_ID = p_nn_node_inf[p_nn_result->result_1st_node].tmpkenID; // 結果ブロックにNNで出力したテンプレート内券種IDを

		if (p_nn_blk->no_change_ID != 1) //複合NNブロックの存在を確認
		{
			pst_work->ID_template = pst_work->now_result_blk_pt->tmplt_ID; // テンプレート内券種IDを更新
		}

		//判定結果書き込み
		p_nn_result->comn.tmplt_ID =  p_nn_node_inf[p_nn_result->result_1st_node].tmpkenID; // テンプレート内券種IDを更新

		//方向情報を設定する。複合NNが実行される場合は複合NN内で行う。
		if (p_nn_blk->no_change_ID != 1)
		{
			set_dir_params(pst_work, 0);
		}

		return 0;

	}
	else	//発火不良の場合
	{
		if (p_nn_blk->no_change_ID != 1) //複合NNブロックの存在を確認
		{
			return err_code;	//単一の時
		}
		else
		{
			return 0;			//複合の時
		}
	}
}

/****************************************************************/
/**
 * @brief		新NN判定　結果処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
u16	New_NN_judeg_proc_result(u8 buf, new_nn_config* nn_cfg, ST_NEW_NN_RESULT* nn_res)
{
	T_TEMPLATE_WORK* pst_work = &st_work[buf];

	new_nn_config* p_nn_blk = 0;
	T_NN_RESULT* p_nn_result = 0;
	//T_DECISION_PARAMETER_NODE_INF* p_nn_node_inf = 0;
	ST_NN_NODE_THRESHOLD p_nn_node_thrs;
	u16 blksiz = 0;
	u16 err_code = 0;

	// パラメタ取得
	p_nn_blk = (new_nn_config*)pst_work->now_para_blk_pt;	//判定パラメタ 共通項

	//可変長部計算
	blksiz = sizeof(T_NN_RESULT);	// 固定長部と,

	p_nn_result = (T_NN_RESULT*)add_result_blk(	// 結果追加
		blksiz,						// 追加する領域サイズ
		pst_work,					// workのポインタ
		err_code,					// 終了状態
		pst_work->ID_template		// 出力券種ID
	);

	//p_nn_node_inf = (T_DECISION_PARAMETER_NODE_INF*)((u8*)p_nn_blk + p_nn_blk->node_inf_ofs);

	////発火値の閾値設定
	////p_nn_node_thrs.th1 = p_nn_node_inf[p_nn_result->result_1st_node].first_val;
	////p_nn_node_thrs.th2 = p_nn_node_inf[p_nn_result->result_1st_node].deff_val;
	//p_nn_node_thrs.th1 = 0.4f; //0.5 ⇒　0.4
	//p_nn_node_thrs.th2 = 0.25;//0.25

	////発火不良の調査
	//nn_res->reject = (u16)out_put_value_cal(&p_nn_node_thrs, nn_res);
	//err_code = nn_res->reject;

	//判定結果書き込み
	p_nn_result->result_1st_node = (u16)nn_res->output_max_node;	        // 最大発火ノード
	p_nn_result->result_2nd_node = (u16)nn_res->output_2nd_node;	        // 2nd発火ノード
	p_nn_result->max_node_val = nn_res->output_max_val;	                    // 最大発火値
	p_nn_result->diff = nn_res->output_max_val - nn_res->output_2nd_val;	// 第二との差分値
	p_nn_result->output_node_ofs = sizeof(T_NN_RESULT);	                    // 可変長データであるNN出力ノードデータのこの結果ブロック先頭からのオフセット値
	err_code = nn_res->reject;				                        // リザルトコード

	//中間情報に書き込む
	work[buf].pbs->mid_res_nn.proc_time      = p_nn_result->comn.proc_time;
	work[buf].pbs->mid_res_nn.output_max_val = p_nn_result->max_node_val;
	work[buf].pbs->mid_res_nn.output_2nd_val = p_nn_result->diff;

	work[buf].pbs->mid_res_nn.max_node_num   = p_nn_result->result_1st_node;
	work[buf].pbs->mid_res_nn._2nd_node_num  = p_nn_result->result_2nd_node;
	//work[buf].pbs->mid_res_nn.th1            = p_nn_node_thrs.th1;
	//work[buf].pbs->mid_res_nn.th2            = p_nn_node_thrs.th2;

	//発火不良でなければ現在のテンプレートIDを更新する
	if (err_code == 0)
	{
		pst_work->now_result_blk_pt->tmplt_ID = (u16)nn_res->note_id;	                                            // 現在のワークメモリへテンプレートIDを設定する。
		p_nn_result->comn.tmplt_ID = (u16)nn_res->note_id;                                                           // 結果ブロックのテンプレート内券種IDを更新
		//if (p_nn_blk->no_change_ID != 1){                                                                       // 複合NNブロックの存在しない場合
			pst_work->ID_template = (u16)nn_res->note_id;                                                            // テンプレート内券種IDを更新
			
			set_dir_params(pst_work, 0);
		//}
		return 0;
	}
	else	                                                                                                    //発火不良の場合
	{
		//if (p_nn_blk->no_change_ID != 1)                                                                        //複合NNブロックの存在を確認
		//{
			return err_code;	                                                                                //単一の時
		//}
		//else
		//{
		//	return 0;			                                                                                //複合の時
		//}
	}
}

/****************************************************************/
/**
 * @brief		外形サイズチェック　結果処理
 *@param[in]	work メモリーのポインタ 
 *@param[out]	<出力引数arg1の説明]>
 * @return			 
 */
/****************************************************************/
void	oline_size_check_res(u8 buf ,ST_SJ sj)
{	
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_SIZ_RESULT*	p_siz_chk_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_siz_chk_result = (T_SIZ_RESULT*)add_result_blk(	// 結果追加
					sizeof(T_SIZ_RESULT),			// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	p_siz_chk_result->hazure = (u8)sj.res;		//はずれレベル
	p_siz_chk_result->padding[0] = 0xff;	//padding
	p_siz_chk_result->padding[1] = 0xff;	
	p_siz_chk_result->padding[2] = 0xff;	
}

/****************************************************************/
/**
 * @brief		外形サイズ識別　結果処理
 *@param[in]	work メモリーのポインタ 
 *@param[out]	<出力引数arg1の説明]>
 * @return			 
 */
/****************************************************************/
void	oline_size_ident_res(u8 buf ,ST_SJ sj ,u16 temp_id)
{	
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_SIZ_IDENT_RESULT*	p_siz_chk_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_siz_chk_result = (T_SIZ_IDENT_RESULT*)add_result_blk(	// 結果追加
					sizeof(T_SIZ_IDENT_RESULT),			// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	p_siz_chk_result->hazure = (u8)sj.res;		//はずれレベル
	p_siz_chk_result->padding[0] = 0xff;	//padding
	p_siz_chk_result->padding[1] = 0xff;	
	p_siz_chk_result->padding[2] = 0xff;	

	if(sj.res == 0)		//サイズが一致していたら
	{
		//券種IDを更新する。
		pst_work->now_result_blk_pt->tmplt_ID = temp_id;	// 結果ブロックにNNで出力したテンプレート内券種IDを
		pst_work->ID_template = temp_id;					// テンプレート内券種IDを更新
		p_siz_chk_result->comn.tmplt_ID = temp_id;			// テンプレート内券種IDを更新
	}
}

/****************************************************************/
/**
 * @brief		外形検知ブロック処理
 *@param[in]	work メモリーのポインタ 
 *@param[out]	<出力引数arg1の説明]>
 * @return			 
 */
/****************************************************************/
void	overall_judge_proc_result(u8 buf)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_NN_TOTAL_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_NN_TOTAL_RESULT*)add_result_blk(// 結果追加
					sizeof(T_NN_TOTAL_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);
	
	//
	//p_total_result- = //総合処理時間
	p_total_result->skew = ((T_OUTLINE_RESULT*)pst_work->p_result_blk_top)->skew_val;	//スキュー角
	p_total_result->long_edge_size = (u8)(((T_OUTLINE_RESULT*)pst_work->p_result_blk_top)->note_x_length  * 0.127f);	//長手サイズ㎜
	p_total_result->short_edge_size = (u8)(((T_OUTLINE_RESULT*)pst_work->p_result_blk_top)->note_y_length * 0.127f);	//短手サイズ㎜
	
	//err_code = ((u32)pst_work->p_result_blk_top + ((T_OUTLINE_RESULT*)pst_work->now_para_blk_pt)->comn.next_block_ofs);

	//p_total_result->	//重券
}

/****************************************************************/
/**
 * @brief		３色比のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
/****************************************************************/
void	cir_3color_result(u8 buf ,u16 invalid_data_count, u8 level)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_CIR3_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_CIR3_RESULT *)add_result_blk(// 結果追加
					sizeof(T_CIR3_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	//
	//p_total_result- = //総合処理時間
	p_total_result->invalid_data_count = invalid_data_count;	//ミスマッチカウント
		p_total_result->level			   = level;					//reberu 

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_3cir.proc_time		= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_3cir.invalid_count	= p_total_result->invalid_data_count;
	work[buf].pbs->mid_res_3cir.level			= p_total_result->level;
	
}

/****************************************************************/
/**
 * @brief		４色比のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
/****************************************************************/
void	cir_4color_result(u8 buf ,u16 invalid_data_count, u8 level)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_CIR4_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_CIR4_RESULT *)add_result_blk(// 結果追加
					sizeof(T_CIR4_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	//
	//p_total_result- = //総合処理時間
	p_total_result->invalid_data_count = invalid_data_count;	//ミスマッチカウント
	p_total_result->level			   = level;					//reberu 

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_4cir.proc_time		= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_4cir.invalid_count	= p_total_result->invalid_data_count;
	work[buf].pbs->mid_res_4cir.level			= p_total_result->level;
	
}

/****************************************************************/
/**
 * @brief		ircheckのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
/****************************************************************/
void	ir_check_result(u8 buf ,u16 invalid_data_count, u8 level)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_IR_CHECK_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_IR_CHECK_RESULT *)add_result_blk(// 結果追加
					sizeof(T_IR_CHECK_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	//
	//p_total_result- = //総合処理時間
	p_total_result->invalid_data_count = invalid_data_count;	//ミスマッチカウント
		p_total_result->level			   = level;					//reberu 

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_ir_ck.proc_time		= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_ir_ck.invalid_count	= p_total_result->invalid_data_count;
	work[buf].pbs->mid_res_ir_ck.level			= p_total_result->level;
}

/****************************************************************/
/**
 * @brief		mcirのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
/****************************************************************/
void	mcir_result(u8 buf ,u16 invalid_data_count, u8 level)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_MCIR_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_MCIR_RESULT *)add_result_blk(// 結果追加
					sizeof(T_MCIR_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	//
	//p_total_result- = //総合処理時間
	p_total_result->invalid_data_count = invalid_data_count;	//ミスマッチカウント
	p_total_result->level			   = level;					//reberu 

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_mcir.proc_time		= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_mcir.invalid_count	= p_total_result->invalid_data_count;
	work[buf].pbs->mid_res_mcir.level			= p_total_result->level;

}


/****************************************************************/
/**
 * @brief		重券チェックのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
/****************************************************************/
void	double_check_result(u8 buf ,s16 invalid_data_count, u8 level)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_DOUBLE_CHECK_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_DOUBLE_CHECK_RESULT *)add_result_blk(// 結果追加
					sizeof(T_DOUBLE_CHECK_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	//
	//p_total_result- = //総合処理時間
	p_total_result->invalid_data_count = invalid_data_count;	//ミスマッチカウント
	p_total_result->level = level;					//level

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_double_ck.proc_time		= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_double_ck.invalid_count	= p_total_result->invalid_data_count;
	work[buf].pbs->mid_res_double_ck.level			= p_total_result->level;

}



/****************************************************************/
/**
 * @brief		新光学式重券のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void	double_check_optical_result(u8 buf_n, ST_DOUBLE_OPTICS_NN_RESULT *res)
{
	T_TEMPLATE_WORK* pst_work = &st_work[buf_n];
	T_DOUBLE_CHECK_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 buf = buf_n;

	//結果書き込み
	p_total_result = (T_DOUBLE_CHECK_RESULT*)add_result_blk(// 結果追加
		sizeof(T_DOUBLE_CHECK_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	
	p_total_result->invalid_data_count = res->dbl_count;	//ミスマッチカウント
	p_total_result->all_prob = res->prob_bill;
	p_total_result->each_prob[0] = res->prob_dbl_part[0];
	p_total_result->each_prob[1] = res->prob_dbl_part[1];
	p_total_result->each_prob[2] = res->prob_dbl_part[2];
	p_total_result->each_prob[3] = res->prob_dbl_part[3];
	p_total_result->level = res->label_predict;

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_double_ck.proc_time     = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_double_ck.invalid_count = p_total_result->invalid_data_count;
	work[buf].pbs->mid_res_double_ck.all_prob = res->prob_bill;
	work[buf].pbs->mid_res_double_ck.each_prob[0] = res->prob_dbl_part[0];
	work[buf].pbs->mid_res_double_ck.each_prob[1] = res->prob_dbl_part[1];
	work[buf].pbs->mid_res_double_ck.each_prob[2] = res->prob_dbl_part[2];
	work[buf].pbs->mid_res_double_ck.each_prob[3] = res->prob_dbl_part[3];

	if (res->label_predict == 1)
	{
		work[buf].pbs->mid_res_double_ck.result = UF;		//０未満の場合は重券
		work[buf].pbs->mid_res_double_ck.level = MAX_LEVEL;
		pst_work->e_code = (u16)REJ_DOUBLE_NOTE;
		work[buf].pbs->validation_result_flg |= 1 << VALIDATION_RES_DOUBLE_CHECK;
	}
	else if (res->label_predict == 0)
	{
		work[buf].pbs->mid_res_double_ck.result = ATM;		//０未満の場合は正券
		work[buf].pbs->mid_res_double_ck.level = MIN_LEVEL;
	}




}

/****************************************************************/
/**
* @brief		重券検知（メカ厚）
*@param[in]	    work メモリーのバッファNo
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	double_check_mecha_result(u8 buf, ST_DBL_CHK_MECHA st)
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;

	//専用結果ブロック構造体を定義 ※
	T_DBL_CHK_MECHA_RESULT* p_total_result = 0;
	int i = 0;

	//結果書き込み

	p_total_result = (T_DBL_CHK_MECHA_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
		sizeof(T_DBL_CHK_MECHA_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
		pst_work,								// workのポインタ
		err_code,								// 終了状態
		pst_work->ID_template					// 出力券種ID
	);

	//結果ブロックに書込
	p_total_result->result = st.result;								// 判定結果
	p_total_result->double_check_ratio = st.double_check_ratio;		// 重券面積比率
	p_total_result->double_check_count = st.double_check_count;		// 重券積分値

	//中間情報に書き込む	
	work[buf].pbs->mid_res_dbl_ck_mecha.proc_time = p_total_result->comn.proc_time;				// 処理時間
	work[buf].pbs->mid_res_dbl_ck_mecha.result = st.result;										// 判定結果
	work[buf].pbs->mid_res_dbl_ck_mecha.double_check_ratio = st.double_check_ratio;				// 重券面積比率
	work[buf].pbs->mid_res_dbl_ck_mecha.bill_thickness_average = st.bill_thickness_average;		// 紙幣判定箇所の厚み平均値
				
	for (i = 0; i < sizeof(st.double_check_point) / sizeof(u8); i++)		// Ch毎の検知結果
	{
		work[buf].pbs->mid_res_dbl_ck_mecha.double_check_point[i] = st.double_check_point[i];	// 重券判定箇所数　（各圧検センサ毎　最大16センサ分）
		work[buf].pbs->mid_res_dbl_ck_mecha.bill_check_point[i] = st.bill_check_point[i];		// 紙幣判定箇所数　（各圧検センサ毎　最大16センサ分）
		work[buf].pbs->mid_res_dbl_ck_mecha.bill_top_point[i] = st.bill_top_point[i];			// 紙幣先端判定箇所　（各圧検センサ毎　最大16センサ分）
	}
	work[buf].pbs->mid_res_dbl_ck_mecha.double_check_threshold = st.double_check_threshold;		// 重券検出厚み閾値
	work[buf].pbs->mid_res_dbl_ck_mecha.double_area_ratio = st.double_area_ratio;				// 面積比率閾値(%)
	work[buf].pbs->mid_res_dbl_ck_mecha.bill_check_threshold = (u8)st.bill_check_threshold;		// 紙幣検出厚み閾値
	work[buf].pbs->mid_res_dbl_ck_mecha.exclude_point = st.exclude_point;						// 先頭判定除外範囲(point)

	work[buf].pbs->mid_res_dbl_ck_mecha.sensor_num = st.sensor_num;								// 厚検センサCH数
	work[buf].pbs->mid_res_dbl_ck_mecha.check_point_top = st.check_point_top;					// 検知対象範囲先端
	work[buf].pbs->mid_res_dbl_ck_mecha.check_point_end = st.check_point_end;					// 検知対象範囲後端

}

/****************************************************************/
/**
 * @brief		鑑別1色NNのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
/****************************************************************/
void	nn1_result(u8 buf ,u8 invalid_data_count,float gen_v ,float con_v, u8 level )
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_NN1_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_NN1_RESULT *)add_result_blk(// 結果追加
					sizeof(T_NN1_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	//
	//p_total_result- = //総合処理時間
	p_total_result->classified_res = invalid_data_count;	//分類結果
	p_total_result->counterfeit_out_put_val = con_v;
	p_total_result->genuine_out_put_val = gen_v;
	p_total_result->level = level;

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_nn1.proc_time				= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_nn1.genuine_out_put_val		= p_total_result->genuine_out_put_val;
	work[buf].pbs->mid_res_nn1.counterfeit_out_put_val	= p_total_result->counterfeit_out_put_val;
	work[buf].pbs->mid_res_nn1.result					= p_total_result->classified_res;
	work[buf].pbs->mid_res_nn1.calc_res_level			= p_total_result->level;

}


/****************************************************************/
/**
 * @brief		鑑別2色NNのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
/****************************************************************/
void	nn2_result(u8 buf ,u8 invalid_data_count ,float gen_v ,float con_v, u8 level )
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_NN2_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_NN2_RESULT *)add_result_blk(// 結果追加
					sizeof(T_NN2_RESULT),	// 追加する領域サイズ
					pst_work,							// workのポインタ
					err_code,							// 終了状態
					pst_work->ID_template				// 出力券種ID
					);

	//
	//p_total_result- = //総合処理時間
	p_total_result->classified_res = invalid_data_count;	//分類結果
	p_total_result->counterfeit_out_put_val = con_v;
	p_total_result->genuine_out_put_val = gen_v;
	p_total_result->level = level;


	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_nn2.proc_time				= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_nn2.genuine_out_put_val		= p_total_result->genuine_out_put_val;
	work[buf].pbs->mid_res_nn2.counterfeit_out_put_val	= p_total_result->counterfeit_out_put_val;
	work[buf].pbs->mid_res_nn2.result					= p_total_result->classified_res;
	work[buf].pbs->mid_res_nn2.calc_res_level			= p_total_result->level;

}

/****************************************************************/
/**
 * @brief		鑑別IR2波長のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void ir2wave_result(u8 buf, ST_IR2WAVE ir2wave)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_IR2WAVE_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 num = 0;
	u8 i = 0;

	//結果書き込み
	p_total_result = (T_IR2WAVE_RESULT *)add_result_blk(// 結果追加
		sizeof(T_IR2WAVE_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	for (i = 0; i < IMUF_POINT_NUMBER; i++)
	{
		p_total_result->output[i] = ir2wave.output[i];
		p_total_result->feature[i] = ir2wave.feature[i];
	}
	p_total_result->result = ir2wave.result;
	p_total_result->level = ir2wave.level;

	num = ir2wave.number;

	//中間情報に書き込む
	work[buf].pbs->mid_res_ir2wave.proc_time = p_total_result->comn.proc_time;

	work[buf].pbs->mid_res_ir2wave.ir1peak = ir2wave.ir1peak[num];
	work[buf].pbs->mid_res_ir2wave.ir2peak = ir2wave.ir2peak[num];
	work[buf].pbs->mid_res_ir2wave.ir1freq = ir2wave.ir1freq[num];
	work[buf].pbs->mid_res_ir2wave.ir2freq = ir2wave.ir2freq[num];
	work[buf].pbs->mid_res_ir2wave.ir1pro = ir2wave.ir1pro[num];
	work[buf].pbs->mid_res_ir2wave.ir2pro = ir2wave.ir2pro[num];
	work[buf].pbs->mid_res_ir2wave.feature = ir2wave.feature[num];

	work[buf].pbs->mid_res_ir2wave.err_code = ir2wave.err_code;
	work[buf].pbs->mid_res_ir2wave.result = ir2wave.result;
	work[buf].pbs->mid_res_ir2wave.level = ir2wave.level;
	

}

/****************************************************************/
/**
 * @brief		磁気のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void mag_result(u8 buf, ST_MAG mag)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_MAG_RESULT* p_total_result = 0;
	s8 err_code = 0;
	
	//結果書き込み
	p_total_result = (T_MAG_RESULT *)add_result_blk(// 結果追加
		sizeof(T_MAG_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	p_total_result->mag_ave = mag.average[mag.ave_number];
	p_total_result->percent = mag.percent[mag.per_number];
	p_total_result->ave_num = mag.ave_number;
	p_total_result->per_num = mag.per_number;
	p_total_result->result  = mag.result;
	p_total_result->level   = mag.level;

	//中間情報に書き込む
	work[buf].pbs->mid_res_mag.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_mag.average = mag.average[mag.ave_number];
	work[buf].pbs->mid_res_mag.ave_number = mag.ave_number;
	work[buf].pbs->mid_res_mag.percent = mag.percent[mag.per_number];
	work[buf].pbs->mid_res_mag.per_number = mag.per_number;
	work[buf].pbs->mid_res_mag.result = mag.result;
	work[buf].pbs->mid_res_mag.level = mag.level;

}

/****************************************************************/
/**
 * @brief		蛍光のブロック処理（正損）
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void uv_result(u8 buf, ST_UV_VALIDATE uv)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_UV_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 i = 0;

	//結果書き込み
	p_total_result = (T_UV_RESULT *)add_result_blk(// 結果追加
		sizeof(T_UV_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	for (i = 0; i < IMUF_POINT_NUMBER; i++)
	{
		p_total_result->uv_ave[i] = uv.average[i];
	}
	p_total_result->level = uv.level;

	//中間情報に書き込む
	work[buf].pbs->mid_res_uv.proc_time = p_total_result->comn.proc_time;
	for (i = 0; i < IMUF_POINT_NUMBER; i++)
	{
		work[buf].pbs->mid_res_uv.average[i] = uv.average[i];
	}

	
	work[buf].pbs->mid_res_uv.result = uv.result;
	work[buf].pbs->mid_res_uv.level = uv.level;
}

/****************************************************************/
/**
 * @brief	　蛍光のブロック処理（鑑別）
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void uv_validate_result(u8 buf, ST_UV_VALIDATE uv)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_UV_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 i = 0;

	//結果書き込み
	p_total_result = (T_UV_RESULT *)add_result_blk(// 結果追加
		sizeof(T_UV_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	for (i = 0; i < IMUF_POINT_NUMBER; i++)
	{
		p_total_result->uv_ave[i] = uv.average[i];
	}
	p_total_result->level = uv.level;

	//中間情報に書き込む
	work[buf].pbs->mid_res_uv_validate.proc_time = p_total_result->comn.proc_time;
	for (i = 0; i < IMUF_POINT_NUMBER; i++)
	{
		work[buf].pbs->mid_res_uv_validate.average[i] = uv.average[i];
	}

	
	work[buf].pbs->mid_res_uv_validate.result = uv.result;
	work[buf].pbs->mid_res_uv_validate.level = uv.level;
}

/****************************************************************/
/**
 * @brief		NEOMAGのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void neomag_result(u8 buf, ST_NEOMAG neo)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_NEOMAG_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 ii = 0;
	u8 temp;

	//結果書き込み
	p_total_result = (T_NEOMAG_RESULT *)add_result_blk(// 結果追加
		sizeof(T_NEOMAG_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	for (ii = 0; ii < neo.num; ii++)
	{
		p_total_result->mag_dev[ii] = neo.mag_dev[ii];
		p_total_result->ir1_ave[ii] = neo.ir1_ave[ii];
		p_total_result->ir2_ave[ii] = neo.ir2_ave[ii];
		p_total_result->split_point[ii] = neo.split_point[ii];
		p_total_result->result[ii] = neo.result[ii];
	}
	
	p_total_result->judge = neo.judge;
	p_total_result->err_code = neo.err_code;

	//中間情報に書き込む
	for (ii = 0; ii < neo.num; ii++)
	{
		work[buf].pbs->mid_res_neomag.magdev[ii] = neo.mag_dev[ii];
		work[buf].pbs->mid_res_neomag.ir1ave[ii] = (u8)(neo.ir1_ave[ii] + 0.5f);
		work[buf].pbs->mid_res_neomag.ir2ave[ii] = (u8)(neo.ir2_ave[ii] + 0.5f);
		work[buf].pbs->mid_res_neomag.result[ii] = neo.result[ii];
		work[buf].pbs->mid_res_neomag.split_point[ii] = (u8)neo.split_point[ii];
		if (neo.ir1_below[ii] > 255)
		{
			temp = 255;
		}
		else
		{
			temp = (u8)neo.ir1_below[ii];
		}
		work[buf].pbs->mid_res_neomag.ir1_below[ii] = temp;

		if (neo.ir2_below[ii] > 255)
		{
			temp = 255;
		}
		else
		{
			temp = (u8)neo.ir2_below[ii];
		}
		work[buf].pbs->mid_res_neomag.ir2_below[ii] = temp;

	}
	work[buf].pbs->mid_res_neomag.count = neo.count;
	work[buf].pbs->mid_res_neomag.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_neomag.err_code = neo.err_code;
	work[buf].pbs->mid_res_neomag.judge = neo.judge;

	
}

/****************************************************************/
/**
 * @brief		磁気スレッドのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void magthread_result(u8 buf, ST_THREAD thr)
{
	T_TEMPLATE_WORK* pst_work = &st_work[buf];
	T_MAGTHREAD_RESULT* p_total_result = 0;
	s8 err_code = 0;
	//	u8 ii = 0;
	//	u8 temp;

		//結果書き込み
	p_total_result = (T_MAGTHREAD_RESULT*)add_result_blk(// 結果追加
		sizeof(T_MAGTHREAD_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	work[buf].pbs->mid_res_magthread.proc_time = p_total_result->comn.proc_time;

	//結果ブロックに書き込む
#ifdef NEW_THREAD
	//ver2.1
	p_total_result->tir_z_score = 0;
	p_total_result->tir_total_ave = thr.pre_percent;
	p_total_result->tir_total_dev = 0;
	p_total_result->mag_total_ave = 0;
	p_total_result->mag_total_dev = 0;
	p_total_result->res_mag_max = thr.res_mag_max;
	p_total_result->remain_percent = thr.remain_percent;
	p_total_result->thread_num = thr.thread_num;
	p_total_result->thread_center = thr.thread_center;
	p_total_result->mag_max_count = 0;
	p_total_result->tir_count = thr.result;
	p_total_result->level = thr.level;
	p_total_result->judge = thr.judge;
	p_total_result->err_code = thr.err_code;

	work[buf].pbs->mid_res_magthread.tir_z_score = thr.remain_percent;
	work[buf].pbs->mid_res_magthread.tir_total_ave = thr.pre_percent;
	work[buf].pbs->mid_res_magthread.tir_total_dev = thr.res_mag_max;
	work[buf].pbs->mid_res_magthread.mag_total_ave = thr.thread_num;
	work[buf].pbs->mid_res_magthread.mag_total_dev = thr.thread_center;
	work[buf].pbs->mid_res_magthread.res_mag_max = thr.wid;
	work[buf].pbs->mid_res_magthread.tir_count = thr.result;

	work[buf].pbs->mid_res_magthread.judge = thr.judge;
	work[buf].pbs->mid_res_magthread.err_code = thr.err_code;
#else
	//ver1.3
	p_total_result->level = thr.level;
	p_total_result->judge = thr.judge;
	p_total_result->err_code = thr.err_code;

	work[buf].pbs->mid_res_magthread.thraed_center = thr.thread_center;
	work[buf].pbs->mid_res_magthread.judge = thr.judge;
	work[buf].pbs->mid_res_magthread.err_code = thr.err_code;

#endif

#ifdef VS_DEBUG

	if (work[buf].pbs->blank3 != 0)
	{
		FILE* fp = fopen("magthread.csv", "a");

		fprintf(fp, "%s,%x,%d,%s,", work[buf].pbs->blank4, work[buf].pbs->mid_res_nn.result_jcm_id, work[buf].pbs->insertion_direction, work[buf].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf].pbs->blank0[20], work[buf].pbs->ser_num1, work[buf].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf].pbs->spec_code.model_calibration, work[buf].pbs->spec_code.sensor_conf, work[buf].pbs->spec_code.mode);

		fprintf(fp, "%d,", thr.remain_percent);
		fprintf(fp, "%d,", thr.pre_percent);
		fprintf(fp, "%.3f,", thr.res_mag_max);
		fprintf(fp, "%d,", thr.thread_num);
		fprintf(fp, "%d,", thr.thread_center);
		fprintf(fp, "%d,", thr.wid);

		fprintf(fp, "%d,", thr.result);
		fprintf(fp, "%d,", thr.err_code);
		fprintf(fp, "%d,", thr.level);
		fprintf(fp, "%d,", thr.judge);

		fprintf(fp, "\n");
		fclose(fp);
	}

#endif

}

/****************************************************************/
/**
 * @brief		金属スレッドのブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void metalthread_result(u8 buf, ST_THREAD thr)
{
	T_TEMPLATE_WORK* pst_work = &st_work[buf];
	T_METALTHREAD_RESULT* p_total_result = 0;
	s8 err_code = 0;
	//	u8 ii = 0;
	//	u8 temp;

		//結果書き込み
	p_total_result = (T_METALTHREAD_RESULT*)add_result_blk(// 結果追加
		sizeof(T_METALTHREAD_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	work[buf].pbs->mid_res_metalthread.proc_time = p_total_result->comn.proc_time;

	//結果ブロックに書き込む
#ifdef NEW_THREAD
	//ver2.1
	p_total_result->tir_z_score = 0;
	p_total_result->tir_total_ave = thr.pre_percent;
	p_total_result->tir_total_dev = 0;
	p_total_result->remain_percent = thr.remain_percent;
	p_total_result->thread_num = thr.thread_num;
	p_total_result->thread_center = thr.thread_center;
	p_total_result->tir_count = thr.result;
	p_total_result->level = thr.level;
	p_total_result->judge = thr.judge;
	p_total_result->err_code = thr.err_code;

	work[buf].pbs->mid_res_metalthread.tir_z_score = thr.remain_percent;
	work[buf].pbs->mid_res_metalthread.tir_total_ave = thr.pre_percent;
	work[buf].pbs->mid_res_metalthread.tir_total_dev = thr.res_mag_max;
	work[buf].pbs->mid_res_metalthread.thread_num = thr.thread_num;
	work[buf].pbs->mid_res_metalthread.thraed_center = thr.thread_center;
	work[buf].pbs->mid_res_metalthread.tir_count = thr.wid;

	work[buf].pbs->mid_res_magthread.tir_count = thr.result;
	work[buf].pbs->mid_res_metalthread.judge = thr.judge;
	work[buf].pbs->mid_res_metalthread.err_code = thr.err_code;

#else
	//ver1.3
	p_total_result->thread_center = thr.thread_center;
	p_total_result->level = thr.level;
	p_total_result->judge = thr.judge;
	p_total_result->err_code = thr.err_code;
	work[buf].pbs->mid_res_metalthread.thraed_center = thr.thread_center;
	work[buf].pbs->mid_res_metalthread.judge = thr.judge;
	work[buf].pbs->mid_res_metalthread.err_code = thr.err_code;

#endif

#ifdef VS_DEBUG

	if (work[buf].pbs->blank3 != 0)
	{
		FILE* fp = fopen("metalthread.csv", "a");

		fprintf(fp, "%s,%x,%d,%s,", work[buf].pbs->blank4, work[buf].pbs->mid_res_nn.result_jcm_id, work[buf].pbs->insertion_direction, work[buf].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf].pbs->blank0[20], work[buf].pbs->ser_num1, work[buf].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf].pbs->spec_code.model_calibration, work[buf].pbs->spec_code.sensor_conf, work[buf].pbs->spec_code.mode);

		fprintf(fp, "%d,", thr.remain_percent);
		fprintf(fp, "%d,", thr.pre_percent);
		fprintf(fp, "%d,", thr.thread_num);
		fprintf(fp, "%d,", thr.thread_center);
		fprintf(fp, "%d,", thr.wid);

		fprintf(fp, "%d,", thr.result);
		fprintf(fp, "%d,", thr.err_code);
		fprintf(fp, "%d,", thr.level);
		fprintf(fp, "%d,", thr.judge);

		fprintf(fp, "\n");
		fclose(fp);
	}

#endif

}


/****************************************************************/
/**
 * @brief		ホログラム検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void hologram_result(u8 buf, ST_HOLOGRAM holo)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_HOLOGRAM_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 ii = 0;
	//u8 temp;

	//結果書き込み
	p_total_result = (T_HOLOGRAM_RESULT *)add_result_blk(// 結果追加
		sizeof(T_HOLOGRAM_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む　Ver1.2
	p_total_result->tir_ave[0] = holo.tir_ave;
	p_total_result->area[0] = holo.count;
	p_total_result->wid[0] = holo.sumhiscount_per_totalcount;
	p_total_result->heid[0] = 0;
	p_total_result->result[0] = holo.result;
	p_total_result->judge = holo.judge;
	p_total_result->err_code = holo.err_code;
	p_total_result->level = holo.level;

	for (ii = 1; ii < HOLO_NUM_LIMIT; ii++)
	{
		p_total_result->tir_ave[ii] = 0;
		p_total_result->area[ii] = 0;
		p_total_result->wid[ii] = 0;
		p_total_result->heid[ii] = 0;
		p_total_result->result[ii] = 0;
	}

	//中間情報に書き込む
	work[buf].pbs->mid_res_hologram.tir_ave[0] = holo.tir_ave;
	work[buf].pbs->mid_res_hologram.area[0] = holo.count;
	work[buf].pbs->mid_res_hologram.area[1] = holo.sumhiscount_per_totalcount;
	work[buf].pbs->mid_res_hologram.area[2] = holo.result;
	work[buf].pbs->mid_res_hologram.area[3] = holo.level;
	work[buf].pbs->mid_res_hologram.area[4] = 0;
	work[buf].pbs->mid_res_hologram.wid[0] = 0;
	work[buf].pbs->mid_res_hologram.heid[0] = 0;
	work[buf].pbs->mid_res_hologram.result[0] = 0;

	work[buf].pbs->mid_res_hologram.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_hologram.judge = holo.judge;
	work[buf].pbs->mid_res_hologram.err_code = holo.err_code;

	for (ii = 1; ii < HOLO_NUM_LIMIT; ii++)
	{
		work[buf].pbs->mid_res_hologram.tir_ave[ii] = 0;
		work[buf].pbs->mid_res_hologram.wid[ii] = 0;
		work[buf].pbs->mid_res_hologram.heid[ii] = 0;
		work[buf].pbs->mid_res_hologram.result[ii] = 0;
	}

	//Ver1.2

}

/****************************************************************/
/**
* @brief		特殊A検知のブロック処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	special_a_result(u8 buf, ST_SPECIAL_A spa)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_SPECIAL_A_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 ii = 0;
	//u8 temp;

	//結果書き込み
	p_total_result = (T_SPECIAL_A_RESULT *)add_result_blk(// 結果追加
		sizeof(T_SPECIAL_A_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	for (ii = 0; ii < SPA_NUM_LIMIT; ii++)
	{
		p_total_result->sum[ii] = spa.res_sum[ii];
		p_total_result->count[ii] = spa.res_count[ii];
	}

	p_total_result->total_sum = spa.res_total_sum;
	p_total_result->judge = spa.judge;

	//中間情報に書き込む
	for (ii = 0; ii < SPA_NUM_LIMIT; ii++)
	{
		work[buf].pbs->mid_res_special_a.sum[ii] = spa.res_sum[ii];
		work[buf].pbs->mid_res_special_a.count[ii] = spa.res_count[ii];
	}

	work[buf].pbs->mid_res_special_a.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_special_a.judge = spa.judge;
	work[buf].pbs->mid_res_special_a.total_sum = spa.res_total_sum;

}

/****************************************************************/
/**
* @brief		特殊B検知のブロック処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void	special_b_result(u8 buf, ST_SPECIAL_B spb)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_SPECIAL_B_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 ii = 0;
	//u8 temp;

	//結果書き込み
	p_total_result = (T_SPECIAL_B_RESULT *)add_result_blk(// 結果追加
		sizeof(T_SPECIAL_B_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	p_total_result->total_ave = spb.total_ave;
	p_total_result->total_dev = spb.total_dev;
	p_total_result->predict = spb.predict;
	p_total_result->judge = spb.judge;

	//中間情報に書き込む
	work[buf].pbs->mid_res_special_b.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_special_b.judge = spb.judge;
	work[buf].pbs->mid_res_special_b.total_ave = spb.total_ave;
	work[buf].pbs->mid_res_special_b.total_dev = spb.total_dev;
	work[buf].pbs->mid_res_special_b.predict = spb.predict;

	for (ii = 0; ii < SPB_NUM; ii++)
	{
		work[buf].pbs->mid_res_special_b.ys[ii] = spb.ys[ii];
		work[buf].pbs->mid_res_special_b.ye[ii] = spb.ye[ii];
	}

}

/****************************************************************/
/**
 * @brief		CF-NN1colorの結果ブロック設定
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void	cf_nn_1color_result(u8 buf, ST_NN* nn_cfg, ST_NN_RESULT* nn_res)
{
	T_TEMPLATE_WORK* pst_work = &st_work[buf];
	T_EXTRACT_CF_NN_1COLOR* res_blk = 0;
	ST_BS* pbs = work[buf].pbs;
	u16 blksiz = 0;
	s8 err_code = 0;
	s8 current_layer = 0;
	float flevel_standard_value = 0.5f;
	
	//処理結果	
	blksiz = sizeof(T_EXTRACT_CF_NN_1COLOR);										// 固定長部と,
	blksiz += calculate_size_including_padding_data(sizeof(float) * nn_cfg->in_node);	// 可変長部（パディングを含む）の合計サイズを計算する。

	res_blk = (T_EXTRACT_CF_NN_1COLOR*)add_result_blk(	// 結果追加
		blksiz,									        // 追加する領域サイズ
		pst_work,								        // workのポインタ
		err_code,								        // 終了状態
		pst_work->ID_template					        // 出力券種ID
	);

	// 機能固有項固定長部情報を書き込む
	res_blk->extract_point_dt_ofs = sizeof(T_EXTRACT_CF_NN_1COLOR);	// 可変長データである抽出ポイントデータのこの結果ブロック先頭からのオフセット値
	res_blk->extract_point_dt_num = nn_cfg->in_node;				// 可変長データである抽出ポイントデータの要素数
	res_blk->result               = nn_res->output_max_node;		// 判定結果　0：真券　1：偽造券
	res_blk->genuine_value        = nn_cfg->output[0];				// 真券の発火値
	res_blk->counterfeit_value    = nn_cfg->output[1];				// 偽造券の発火値
	res_blk->do_flg				  = nn_res->do_flg;					// 実行済みフラグ
	res_blk->layer				  = nn_res->layer_num;				// 現在のレイヤー番号（総合判定で用いる

	// 可変長部1をコピーする。（パディング含まず）
	memcpy((u8*)((u8*)res_blk + res_blk->extract_point_dt_ofs), nn_cfg->in_put, sizeof(float) * nn_cfg->in_node);	// 可変長データを追加

	//中間情報に値を設定
	current_layer = nn_res->layer_num - 1;
	if (current_layer < 0)
	{
		current_layer = 0;
	}
	pbs->mid_res_cf_nn_1.proc_time[current_layer]		= res_blk->comn.proc_time;
	pbs->mid_res_cf_nn_1.genuine_val[current_layer]		= res_blk->genuine_value;
	pbs->mid_res_cf_nn_1.counterfeit_val[current_layer] = res_blk->counterfeit_value;
	pbs->mid_res_cf_nn_1.each_result[current_layer]		= res_blk->result;
	pbs->mid_res_cf_nn_1.result							+= res_blk->result;	// 0:真券 0以外:偽造券

	//レベルを計算
	if (nn_res->do_flg == 1)	//処理が実行されているなら計算する。
	{
		pbs->mid_res_cf_nn_1.output_level[current_layer] = level_detect(&res_blk->counterfeit_value, &flevel_standard_value, 1, 0, 1);
	}
	else //処理が実行されていないなら、0で
	{
		pbs->mid_res_cf_nn_1.output_level[current_layer] = 0;
	}

}

/****************************************************************/
/**
 * @brief		CF-NN2colorの結果ブロック設定
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void	cf_nn_2color_result(u8 buf, ST_NN* nn_cfg, ST_NN_RESULT* nn_res)
{
	T_TEMPLATE_WORK* pst_work = &st_work[buf];
	T_EXTRACT_CF_NN_2COLOR* res_blk = 0;
	ST_BS* pbs = work[buf].pbs;
	u16 blksiz = 0;
	s8 err_code = 0;
	s8 current_layer = 0;
	float flevel_standard_value = 0.5f;

	//処理結果	
	blksiz = sizeof(T_EXTRACT_CF_NN_2COLOR);										// 固定長部と,
	blksiz += calculate_size_including_padding_data(sizeof(float) * nn_cfg->in_node);	// 可変長部（パディングを含む）の合計サイズを計算する。

	res_blk = (T_EXTRACT_CF_NN_2COLOR*)add_result_blk(	// 結果追加
		blksiz,									        // 追加する領域サイズ
		pst_work,								        // workのポインタ
		err_code,								        // 終了状態
		pst_work->ID_template					        // 出力券種ID
	);

	// 機能固有項固定長部情報を書き込む
	res_blk->extract_point_dt_ofs = sizeof(T_EXTRACT_CF_NN_2COLOR);	// 可変長データである抽出ポイントデータのこの結果ブロック先頭からのオフセット値
	res_blk->extract_point_dt_num = nn_cfg->in_node;				// 可変長データである抽出ポイントデータの要素数
	res_blk->result = nn_res->output_max_node;		                // 判定結果　0：真券　1：偽造券
	res_blk->genuine_value = nn_cfg->output[0];				        // 真券の発火値
	res_blk->counterfeit_value = nn_cfg->output[1];				    // 偽造券の発火値
	res_blk->do_flg = nn_res->do_flg;								// 実行済みフラグ
	res_blk->layer = nn_res->layer_num;								// 現在のレイヤー番号（総合判定で用いる

	// 可変長部1をコピーする。（パディング含まず）
	memcpy((u8*)((u8*)res_blk + res_blk->extract_point_dt_ofs), nn_cfg->in_put, sizeof(float) * nn_cfg->in_node);	// 可変長データを追加

	//中間情報に値を設定
	current_layer = nn_res->layer_num - 1;
	if (current_layer < 0)
	{
		current_layer = 0;
	}
	pbs->mid_res_cf_nn_2.proc_time[current_layer] = res_blk->comn.proc_time;
	pbs->mid_res_cf_nn_2.genuine_val[current_layer] = res_blk->genuine_value;
	pbs->mid_res_cf_nn_2.counterfeit_val[current_layer] = res_blk->counterfeit_value;
	pbs->mid_res_cf_nn_2.each_result[current_layer] = res_blk->result;
	pbs->mid_res_cf_nn_2.result += res_blk->result;	// 0:真券 0以外:偽造券

	//レベルを計算
	if (nn_res->do_flg == 1)	//処理が実行されているなら計算する。
	{
		pbs->mid_res_cf_nn_2.output_level[current_layer] = level_detect(&res_blk->counterfeit_value, &flevel_standard_value, 1, 0, 1);
	}
	else //処理が実行されていないなら、0で
	{
		pbs->mid_res_cf_nn_2.output_level[current_layer] = 0;
	}

}


/****************************************************************/
/**
 * @brief		フィットネス角折れ検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	dog_ear_result(u8 buf ,ST_DOG_EAR dog )
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	
	//専用結果ブロック構造体を定義
	T_DOG_EAR_RESULT* p_total_result = 0;
	u8 i = 0;

	//結果書き込み
					
	p_total_result = (T_DOG_EAR_RESULT *)add_result_blk(	// 結果追加				キャストを変更
					sizeof(T_DOG_EAR_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	//結果ブロックに書き込む
	for(i = 0; i < 4; ++i)
	{
		p_total_result->len_x[i] = dog.len_x[i];								//x辺の長さ
		p_total_result->len_y[i] = dog.len_y[i];								//y辺の長さ
		p_total_result->len_x_mm[i] = dog.len_x_mm[i];							//x辺の長さ mm
		p_total_result->len_y_mm[i] = dog.len_y_mm[i];							//y辺の長さ mm
		p_total_result->judge[i] = dog.judge[i];								//判定結果
		p_total_result->judge_reason[i] = dog.judge_reason[i];					//判定結果の理由
		p_total_result->area[i] = dog.area[i];									//面積		
		p_total_result->area_mm[i] = dog.area_mm[i];							//面積 mm
		p_total_result->short_side[i] = dog.short_side[i];						//角折れの短手の長さ
		p_total_result->long_side[i] = dog.long_side[i];						//角折れの長手の長さ
		p_total_result->short_side_mm[i] = dog.short_side_mm[i];				//短手　㎜
		p_total_result->long_side_mm[i] = dog.long_side_mm[i];					//長手　㎜
		p_total_result->triangle_vertex_x[i][0] = dog.triangle_vertex_1[i][0];	//頂点座標左上x
		p_total_result->triangle_vertex_y[i][0] = dog.triangle_vertex_2[i][0];	//頂点座標左上y
		p_total_result->triangle_vertex_x[i][1] = dog.triangle_vertex_1[i][1];	//頂点座標右下x
		p_total_result->triangle_vertex_y[i][1] = dog.triangle_vertex_2[i][1];	//頂点座標右下y
	}

	p_total_result->level = dog.level;											//レベル

	//中間情報に書き込む
	work[buf].pbs->mid_res_dog.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_dog.short_side_left_up		= (u16)dog.short_side[0];
	work[buf].pbs->mid_res_dog.short_side_left_down		= (u16)dog.short_side[1];
	work[buf].pbs->mid_res_dog.short_side_right_up		= (u16)dog.short_side[2];
	work[buf].pbs->mid_res_dog.short_side_right_down	= (u16)dog.short_side[3];
	work[buf].pbs->mid_res_dog.long_side_left_up		= (u16)dog.long_side[0];
	work[buf].pbs->mid_res_dog.long_side_left_down		= (u16)dog.long_side[1];
	work[buf].pbs->mid_res_dog.long_side_right_up		= (u16)dog.long_side[2];
	work[buf].pbs->mid_res_dog.long_side_right_down		= (u16)dog.long_side[3];
	work[buf].pbs->mid_res_dog.area_left_up				= dog.area[0];
	work[buf].pbs->mid_res_dog.area_left_down			= dog.area[1];
	work[buf].pbs->mid_res_dog.area_right_up			= dog.area[2];
	work[buf].pbs->mid_res_dog.area_right_down			= dog.area[3];
	work[buf].pbs->mid_res_dog.judge_left_up			= dog.judge[0];
	work[buf].pbs->mid_res_dog.judge_left_down			= dog.judge[1];
	work[buf].pbs->mid_res_dog.judge_right_up			= dog.judge[2];
	work[buf].pbs->mid_res_dog.judge_right_down			= dog.judge[3];
}


/****************************************************************/
/**
 * @brief		フィットネス裂け検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	tear_result(u8 buf ,ST_TEAR tr )
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	
	//専用結果ブロック構造体を定義 ※
	T_TEAR_RESULT* p_total_result = 0;
	u8 i = 0;

	//結果書き込み
					
	p_total_result = (T_TEAR_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
					sizeof(T_TEAR_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	//結果ブロックに書き込む

	p_total_result->res_tear_count	 = tr.res_tear_count;								//検知した裂けの数
	p_total_result->res_judge_tear	 = tr.res_judge_tear;								//検知した裂けの中で閾値を超えるものがあった。　1：有　0：無
	p_total_result->res_judge_reason = tr.res_judge_reason;							//検知した裂けの理由

	for(i = 0; i < p_total_result->res_tear_count; ++i)
	{
		p_total_result->res_tear_width[i]		 = tr.res_tear_width[i];			//検知した裂けの幅
		p_total_result->res_tear_depth[i]		 = tr.res_tear_depth[i];			//検知した裂けの深さ
		p_total_result->res_tear_type[i]		 = tr.res_tear_type[i];				//検知した裂けの形状
		p_total_result->res_each_judge_reason[i] = tr.res_each_judge_reason[i];	//検知した裂けの理由

	}
	p_total_result-> res_tear_total_depth = tr.res_tear_depth_total;				//深さの合計
	p_total_result->level = tr.level;												//レベル

	//中間情報を書き込む
/*work[buf].pbs->mid_res_tear.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_tear.width	= p_total_result->res_tear_width[p_total_result->res_tear_count];
	work[buf].pbs->mid_res_tear.depth	= p_total_result->res_tear_depth[p_total_result->res_tear_count];
	work[buf].pbs->mid_res_tear.type	= p_total_result->res_tear_type[p_total_result->res_tear_count];
*/
	work[buf].pbs->mid_res_tear.proc_time	= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_tear.width		= p_total_result->res_tear_width[0];
	work[buf].pbs->mid_res_tear.depth		= p_total_result->res_tear_depth[0];
	work[buf].pbs->mid_res_tear.total_depth	= p_total_result->res_tear_total_depth;
	work[buf].pbs->mid_res_tear.type		= p_total_result->res_tear_type[0];
	work[buf].pbs->mid_res_tear.judge		= p_total_result->res_judge_tear;
}

/****************************************************************/
/**
 * @brief		フィットネスダイノート検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	dyenote_result(u8 buf ,ST_DYE_NOTE dy )
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	
	//専用結果ブロック構造体を定義 ※
	T_DYENOTE_RESULT* p_total_result = 0;
	//u8 i = 0;

	//結果書き込み
					
	p_total_result = (T_DYENOTE_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
					sizeof(T_DYENOTE_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	//結果ブロックに書き込む
	p_total_result->err_code	= 0;									//エラーコード
	p_total_result->result		= dy.judge;								//検知結果
	p_total_result->ink_area	= dy.res_ink_area;						//インク面積dot
	p_total_result->ink_rate	= dy.res_ink_ratio;						//インク面積割合
	p_total_result->level		= dy.level;								//レベル

	//中間情報に書き込む
	work[buf].pbs->mid_res_dyenote.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_dyenote.area		 = p_total_result->ink_area;
	work[buf].pbs->mid_res_dyenote.raito	 = p_total_result->ink_rate;
	work[buf].pbs->mid_res_dyenote.judge	 = p_total_result->result;
	work[buf].pbs->fitness[DYE_NOTE_].bit.level  = dy.level;


	/*work[buf].pbs->fitness[DYE_NOTE_].bit.result = level_result_detect(
		(u8)work[buf].pbs->fitness[DYE_NOTE_].bit.level ,
		(u8)work[buf].pbs->fitness[DYE_NOTE_].bit.threshold_1 ,
		(u8)work[buf].pbs->fitness[DYE_NOTE_].bit.threshold_2);
*/
	if((u8)work[buf].pbs->fitness[DYE_NOTE_].bit.level <= (u8)work[buf].pbs->fitness[DYE_NOTE_].bit.threshold_2)
	{
		work[buf].pbs->fitness[DYE_NOTE_].bit.result = UF;
	}
	else
	{
		work[buf].pbs->fitness[DYE_NOTE_].bit.result = ATM;
	}
}

/****************************************************************/
/**
 * @brief		フィットネス汚れ検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	soiling_result(u8 buf ,ST_SOILING soi )
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	
	//専用結果ブロック構造体を定義 ※
	T_SOILING_RESULT* p_total_result = 0;
	u8 i = 0;

	//結果書き込み
					
	p_total_result = (T_SOILING_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
					sizeof(T_SOILING_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	//結果ブロックに書き込む
	p_total_result->uf_count	= soi.uf_count;						//UFエリアの数
	p_total_result->result		= soi.res_judge;							//検知結果
	p_total_result->level		= (u8)soi.output_level;

	for(i = 0; i < soi.area_num; ++i)
	{
		p_total_result->each_area_distance[i] = soi.level[i];
	}

	//サンプリングデータにも書き込む
	memcpy(work[buf].pbs->mid_res_soiling.avg_val, soi.avg_val , soi.area_num * 3);

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_soiling.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_soiling.result	 = p_total_result->result;							//検知結果

	//for(i = 0; i < soi.area_num; ++i)
	//{
	//	//work[buf].pbs->mid_res_soiling.plane_distance[i] = p_total_result->each_area_distance[i];
	//}

}

/****************************************************************/
/**
 * @brief		フィットネス脱色検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	deink_result(u8 buf  ,ST_DEINKED deink )
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	u8 i = 0;
	
	//専用結果ブロック構造体を定義 ※
	T_DEINK_RESULT* p_total_result = 0;
	//u8 i = 0;

	//結果書き込み
					
	p_total_result = (T_DEINK_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
					sizeof(T_DEINK_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	//結果ブロックに書き込む
	p_total_result->uf_count	= deink.uf_count;						//UFエリアの数
	p_total_result->result		= deink.res_judge;							//検知結果
	p_total_result->level		= (u8)deink.output_level;

	for(i = 0; i < deink.area_num; ++i)
	{
		p_total_result->each_area_distance[i] = deink.level[i];
	}

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_de_ink.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_de_ink.result	= p_total_result->result;							//検知結果

	for(i = 0; i < deink.area_num; ++i)
	{
		work[buf].pbs->mid_res_de_ink.plane_distance[i] = p_total_result->each_area_distance[i];
	}



}

/****************************************************************/
/**
 * @brief		フィットネス染み検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	stain_result(u8 buf ,ST_STAIN sta )
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	
	//専用結果ブロック構造体を定義 ※
	T_STAIN_RESULT* p_total_result = 0;
	//u8 i = 0;

	//結果書き込み
					
	p_total_result = (T_STAIN_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
					sizeof(T_STAIN_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	//結果ブロックに書込
	p_total_result->err_code				= sta.res_stain_err;			//エラーコード
	p_total_result->result					= sta.res_judge;				//検知結果
	p_total_result->max_stain_area			= sta.res_max_stain_area;		//最大面積　単位mm^2
	p_total_result->max_stain_diameter		= sta.res_max_stain_diameter;	//最大直径　単位mm^2
	p_total_result->max_stain_total_area	= sta.res_total_stain_area;		//面積の合計　単位mm^2
	p_total_result->level					= sta.level;					//レベル

	//結果ブロックに書込
	work[buf].pbs->mid_res_stain.proc_time	= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_stain.err_code	= sta.res_stain_err;			//エラーコード
	work[buf].pbs->mid_res_stain.judge		= sta.res_judge;				//検知結果
	work[buf].pbs->mid_res_stain.area		= sta.res_max_stain_area;		//最大面積　単位mm^2
	work[buf].pbs->mid_res_stain.diameter	= sta.res_max_stain_diameter;	//最大直径　単位mm^2
	work[buf].pbs->mid_res_stain.total_area	= sta.res_total_stain_area;		//面積の合計　単位mm^2
	
}



/****************************************************************/
/**
 * @brief		フィットネス穴検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	hole_result(u8 buf ,ST_HOLE ho )
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	
	//専用結果ブロック構造体を定義 ※
	T_HOLE_RESULT* p_total_result = 0;
	u8 i = 0;

	//結果書き込み
					
	p_total_result = (T_HOLE_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
					sizeof(T_HOLE_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	//結果ブロックに書き込む
	p_total_result->result			= ho.result;
	p_total_result->hole_count		= ho.hole_count;
	p_total_result->total_hole_area	= ho.total_hole_area;
	p_total_result->max_hole_area	= ho.max_hole_area;
	p_total_result->err_code		= ho.err_code;
	p_total_result->level			= ho.level;

	for(i = 0; i < HOLE_MAX_COUNT; ++i)
	{
		p_total_result->holes[i] = ho.holes[i];
	}

	//サンプリングデータに書き込む
	work[buf].pbs->mid_res_hole.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_hole.err_code	=  ho.err_code;
	work[buf].pbs->mid_res_hole.hole_num	=  ho.hole_count;
	work[buf].pbs->mid_res_hole.hole_total_area	=  ho.total_hole_area;
	work[buf].pbs->mid_res_hole.result	=  ho.result;
	
	for(i = 0; i < HOLE_MAX_COUNT; ++i)
	{
		work[buf].pbs->mid_res_hole.each_hole_area[i] = ho.holes[i];
	}

}

/****************************************************************/
/**
 * @brief		フィットネス折り畳み検知のブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 *
 */
 /****************************************************************/
void	folding_result(u8 buf, ST_FOLDING folding)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_FOLDING_RESULT* p_total_result = 0;
	s8 err_code = 0;

	//結果書き込み
	p_total_result = (T_FOLDING_RESULT *)add_result_blk(// 結果追加
		sizeof(T_FOLDING_RESULT),	// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	//結果ブロックに書き込む
	// 上
	p_total_result->up_res		= folding.folded[0];
	p_total_result->level[0]	= folding.folded_level[0];
	// 下
	p_total_result->down_res	= folding.folded[1];
	p_total_result->level[1]	= folding.folded_level[1];
	// 左
	p_total_result->left_res	= folding.folded[2];
	p_total_result->level[2]	= folding.folded_level[2];
	// 右
	p_total_result->right_res	= folding.folded[3];
	p_total_result->level[3]	= folding.folded_level[3];

	p_total_result->err_code = folding.err_code;
	
	//中間情報に書き込む
	work[buf].pbs->mid_res_folding.proc_time		= p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_folding.up_res			= folding.folded[0];
	//work[buf].pbs->mid_res_folding.level[0]			= folding.folded_level[0];
	work[buf].pbs->mid_res_folding.down_res			= folding.folded[1];
	//work[buf].pbs->mid_res_folding.level[1]			= folding.folded_level[1];
	work[buf].pbs->mid_res_folding.left_res			= folding.folded[2];
	//work[buf].pbs->mid_res_folding.level[2]			= folding.folded_level[2];
	work[buf].pbs->mid_res_folding.right_res		= folding.folded[3];
	//work[buf].pbs->mid_res_folding.level[3]			= folding.folded_level[3];
	work[buf].pbs->mid_res_folding.err_code			= folding.err_code;
}

/****************************************************************/
/**
* @brief		テープ検知のブロック処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*
*/
/****************************************************************/
void tape_result(u8 buf, ST_TAPE tape)
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;

	//専用結果ブロック構造体を定義 ※
	T_TAPE_RESULT* p_total_result = 0;
	int i = 0;

	//結果書き込み

	p_total_result = (T_TAPE_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
		sizeof(T_TAPE_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
		pst_work,								// workのポインタ
		err_code,								// 終了状態
		pst_work->ID_template					// 出力券種ID
	);

	//結果ブロックに書込
	p_total_result->err_code = tape.err_code;			// エラーコード
	p_total_result->result = tape.result;				// 検知結果
	p_total_result->level = tape.level;					// レベル
	p_total_result->detect_num = tape.detect_num;		// テープ検知数
	//中間情報に書き込む	
	work[buf].pbs->mid_res_tape.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_tape.result = tape.result;						// 検知結果
	work[buf].pbs->mid_res_tape.level = tape.level;							// レベル
	work[buf].pbs->mid_res_tape.detect_num = tape.detect_num;				// テープ検知数

	for (i = 0; i < sizeof(tape.ch_tape_num) / sizeof(u8); i++)				// Ch毎の検知結果
	{
		work[buf].pbs->mid_res_tape.ch_tape_num[i] = tape.ch_tape_num[i];				
	}

}

/****************************************************************/
/**
* @brief		静電テープ検知のブロック処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*
*/
/****************************************************************/
void cap_tape_result(u8 buf, ST_CAPACITANCE_TAPE cap_tape)
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;

	//専用結果ブロック構造体を定義 ※
	T_CAP_TAPE_RESULT* p_total_result = 0;
//	int i = 0;

	//結果書き込み

	p_total_result = (T_CAP_TAPE_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
		sizeof(T_CAP_TAPE_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
		pst_work,								// workのポインタ
		err_code,								// 終了状態
		pst_work->ID_template					// 出力券種ID
	);

	//結果ブロックに書込
	p_total_result->err_code = cap_tape.err_code;			//エラーコード
	p_total_result->result = cap_tape.result;				//検知結果
	p_total_result->level = cap_tape.level;					//レベル

	//中間情報に書き込む	
	work[buf].pbs->mid_res_cap_tape.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_cap_tape.result = cap_tape.result;				//検知結果
	work[buf].pbs->mid_res_cap_tape.level = cap_tape.level;				//検知結果
}

/****************************************************************/
/**
 * @brief		ocrの結果ブロック
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 * 
 */
/****************************************************************/
void	ocr_result(u8 buf  ,char* res_1 ,char* res_2, COD_Parameters* st ,u32 color_time ,u8 color)
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;
	u32 i = 0;
	//専用結果ブロック構造体を定義 ※
	T_OCR_RESULT* p_total_result = 0;
	ST_BS* pbs = work[buf].pbs;
	//u8 i = 0;

	s16 left = 0;
	s16 right = 0;
	s16 top = 0;
	s16 bottom = 0;
	s32 start_x = 0;
	s32 start_y = 0;
	s32 wid = 0;
	s32	hid = 0;
	s16 tmp = 0;
	s16 ori_start_x;
	s16 ori_start_y;
	s16 ori_end_x;
	s16 ori_end_y;


	//結果書き込み
					
	p_total_result = (T_OCR_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
					sizeof(T_OCR_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
					pst_work,								// workのポインタ
					err_code,								// 終了状態
					pst_work->ID_template					// 出力券種ID
					);

	for ( i = 0; i < strlen(res_1); i++)
	{
		pbs->ser_num1[i] = 0;
	}
	for ( i = 0; i < strlen(res_2); i++)
	{
		pbs->ser_num2[i] = 0;

	}


	//記番号の位置情報を設定する
	if (st->rotate == 1)	//回転の場合　XYを入れ替える
	{
		left = st->top;
		right = st->bottom;
		top = st->left;
		bottom = st->right;

		hid = st->width;
		wid = st->height;


		if (st->flip_y == 1)	//y反転の場合　スタート座標を右からにする
		{
			start_x = st->starting_x + wid;
			tmp = left;
			left = right * -1;
			right = tmp * -1;
		}
		else
		{
			start_x = st->starting_x;
		}

		if (st->flip_x == 1)	//x座標反転の場合　スタート座標をボトムからにする。
		{
			start_y = st->starting_y - hid;
			tmp = top;
			top = bottom *  -1;
			bottom = tmp * -1;
		}
		else
		{
			start_y = st->starting_y;
		}

	}
	else
	{
		left = st->left;
		right = st->right;
		top = st->top;
		bottom = st->bottom;

		wid = st->width;
		hid = st->height;


		if (st->flip_x == 1)	//ｘ反転の場合　スタート座標を右からにする
		{
			start_x = st->starting_x + wid;
			tmp = left;
			left = right * -1;
			right = tmp * -1;
		}
		else
		{
			start_x = st->starting_x;
		}

		if (st->flip_y == 1)	//y座標反転の場合　スタート座標をボトムからにする。
		{
			start_y = st->starting_y - hid;
			tmp = top;
			top = bottom * -1;
			bottom = tmp * -1;
		}
		else
		{
			start_y = st->starting_y;
		}
	}

	//元々の切り出し座標の計算
	ori_start_x = (s16)st->starting_x;
	ori_start_y = (s16)st->starting_y;
	ori_end_x = (s16)st->starting_x + wid;
	ori_end_y = (s16)st->starting_y - hid;

	if(st->area_num == 0)
	{
		//結果ブロックに書き込む
		memcpy(p_total_result->series_1, res_1 , strlen(res_1));
		p_total_result->series_num_1 = (u16)(strlen(res_1));

		//サンプリングデータにも書き込む
		memcpy(pbs->ser_num1, res_1 , strlen(res_1));
		work[buf].pbs->mid_res_outline.proc_time_ocr1 = p_total_result->comn.proc_time;
		work[buf].pbs->mid_res_outline.proc_time_color = color_time;	//色識別の処理時間　add by furuta 20/8/21


		//色情報　add by furuta 20/9/3
		if(color == 0)					//色なし
		{
			work[buf].pbs->series_color_1 = 0x20;
		}
		else if(color != 255)							//色有り　colorが既にASCII
		{
			work[buf].pbs->series_color_1 = color;
		}
		else
		{
			work[buf].pbs->series_color_1 = 0x00;	//色判定なし。
		}

		//長さが0の時は位置情報を記録しない。
		if (p_total_result->series_num_1 != 0)
		{
			//記番号の座標をサンプリングファイルに書き込む	add by furuta 21/1/18
			work[buf].pbs->series_1_top_x = (s16)start_x + left;
			work[buf].pbs->series_1_top_y = (s16)start_y - top;
			work[buf].pbs->series_1_bottom_x = (s16)start_x + right;
			work[buf].pbs->series_1_bottom_y = (s16)start_y - bottom-2;

			if (ori_start_x > work[buf].pbs->series_1_top_x ||	//元の切り出し位置より外の場合
				ori_end_x < work[buf].pbs->series_1_top_x)	//元の切り出し位置を格納する。
			{
				work[buf].pbs->series_1_top_x = ori_start_x;
			}
			if (ori_start_x > work[buf].pbs->series_1_bottom_x ||
				ori_end_x < work[buf].pbs->series_1_bottom_x)
			{
				work[buf].pbs->series_1_bottom_x = ori_end_x;
			}
			if (ori_start_y < work[buf].pbs->series_1_top_y ||
				ori_end_y > work[buf].pbs->series_1_top_y)
			{
				work[buf].pbs->series_1_top_y = ori_start_y;
			}
			if (ori_start_y < work[buf].pbs->series_1_bottom_y ||
				ori_end_y > work[buf].pbs->series_1_bottom_y)
			{
				work[buf].pbs->series_1_bottom_y = ori_end_y;
			}
		}

	}
	else
	{
		//結果ブロックに書き込む
		memcpy(p_total_result->series_2, res_2 , strlen(res_2));
		p_total_result->series_num_2 = (u16)(strlen(res_2));
		//サンプリングデータにも書き込む
		memcpy(pbs->ser_num2, res_2 , strlen(res_2));
		work[buf].pbs->mid_res_outline.proc_time_ocr2 = p_total_result->comn.proc_time;
		work[buf].pbs->series_color_2 = 0x20;	// add by furuta 20200903

		if (p_total_result->series_num_2 != 0)
		{
			//記番号の座標をサンプリングファイルに書き込む
			work[buf].pbs->series_2_top_x = (s16)start_x + left;
			work[buf].pbs->series_2_top_y = (s16)start_y - (top - 3);
			work[buf].pbs->series_2_bottom_x = (s16)start_x + right;
			work[buf].pbs->series_2_bottom_y = (s16)start_y - bottom;

			if (ori_start_x > work[buf].pbs->series_2_top_x ||	//元の切り出し位置より外の場合
				ori_end_x < work[buf].pbs->series_2_top_x)		//元の切り出し位置を格納する。
			{
				work[buf].pbs->series_2_top_x = ori_start_x;
			}
			if (ori_start_x > work[buf].pbs->series_2_bottom_x ||
				ori_end_x < work[buf].pbs->series_2_bottom_x)
			{
				work[buf].pbs->series_2_bottom_x = ori_end_x;
			}
			if (ori_start_y < work[buf].pbs->series_2_top_y ||
				ori_end_y > work[buf].pbs->series_2_top_y)
			{
				work[buf].pbs->series_2_top_y = ori_start_y;
			}
			if (ori_start_y < work[buf].pbs->series_2_bottom_y ||
				ori_end_y > work[buf].pbs->series_2_bottom_y)
			{
				work[buf].pbs->series_2_bottom_y = ori_end_y;
			}
		}

	}

}
#if 0
/****************************************************************/
/**
 * @brief		バーコードリードの結果ブロック処理
 *@param[in]	work メモリーのポインタ
 *@param[out]	<出力引数arg1の説明]>
 * @return
 */
 /****************************************************************/
void barcode_read_result(u8 buf, ST_BARCODE_READ barcode)
{
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	T_BARCODE_RESULT* p_total_result = 0;
	s8 err_code = 0;
	u8 i = 0;

	u16 block_size = 0;

	u16 fraction = barcode.res_itf_digits % 4;	//桁数が4バイトアライメントを満たすように余りを計算
	
	if(fraction != 0)
	{
		fraction = 4 - fraction; 
	}
	
	if(barcode.res_code_type == 1)	//領域サイズ計算
	{
		block_size = sizeof(T_BARCODE_RESULT) + barcode.res_itf_digits + fraction;
	}
	else
	{
		block_size = sizeof(T_BARCODE_RESULT) + barcode.res_qr_digits + fraction;
	}

	//結果書き込み
	p_total_result = (T_BARCODE_RESULT *)add_result_blk(// 結果追加
		block_size,							// 追加する領域サイズ
		pst_work,							// workのポインタ
		err_code,							// 終了状態
		pst_work->ID_template				// 出力券種ID
	);

	work[buf].pbs->mid_res_bar_code.proc_time = p_total_result->comn.proc_time;

	if(barcode.res_code_type == 1)	//コードタイプで条件分岐　1次元
	{
		//結果ブロックに書き込む
		p_total_result->itf_res_ary_offset = sizeof(T_BARCODE_RESULT);	//結果配列のオフセット
		p_total_result->itf_res_character_num = barcode.res_itf_digits;	//桁数
		p_total_result->itf_res_states =  barcode.err_code;		//エラーコード
		p_total_result->itf_res_max_bar = barcode.max_bar;		//バーの最大幅

		memset((p_total_result) + p_total_result->itf_res_ary_offset, barcode.res_itf_read_characters[0] , barcode.res_itf_digits);

		//中間情報に書き込む
		work[buf].pbs->mid_res_bar_code.code_type = barcode.code_type;
		work[buf].pbs->mid_res_bar_code.read_data_len = barcode.res_itf_digits;
		work[buf].pbs->mid_res_bar_code.max_bar = (u8)barcode.max_bar;

		for(i = 0; i < barcode.res_itf_digits; ++i)
		{
			work[buf].pbs->mid_res_bar_code.read_data[i] = (u8)barcode.res_itf_read_characters[i];
		}

	}
	else							//2次元
	{
		//結果ブロックに書き込む
		p_total_result->qr_res_ary_offset = sizeof(T_BARCODE_RESULT);
		p_total_result->qr_res_character_num = barcode.res_qr_digits;
		p_total_result->qr_res_states = (u16)barcode.err_code;

		//中間情報に書き込む
		work[buf].pbs->mid_res_bar_code.code_type = barcode.code_type;
		work[buf].pbs->mid_res_bar_code.read_data_len = barcode.res_qr_digits;

		for(i = 0; i < barcode.res_qr_digits; ++i)
		{
			work[buf].pbs->mid_res_bar_code.read_data[i] = (u8)barcode.res_qr_read_characters[i];
		}

	}


	work[buf].pbs->mid_res_bar_code.result_code = barcode.err_code;	//エラー（結果コード）を入力する。

	////テンプレート内券種IDを更新する
	//if(barcode.err_code == 0)
	//{
	//	pst_work->ID_template = write_template_id;
	//	pst_work->now_result_blk_pt->tmplt_ID = write_template_id;
	//}

	//バーコード券
	if(barcode.err_code == ITF_BARCODE_RESULT_SUCCESSFULL			||
			barcode.err_code == ITF_BARCODE_RESULT_WIDTH_ERROR			||
			barcode.err_code == ITF_BARCODE_RESULT_UNKNOWN_DIGITS_ERROR ||
			barcode.err_code == ITF_BARCODE_RESULT_CHARACTER_LIMIT_ERROR )
	{
		pst_work->e_code = (u16)barcode.err_code;
	}
	//バーコード券でなかった場合
	else
	{
		pst_work->e_code = 0;
	}


}

/****************************************************************/
/**
* @brief		落書き検知文字検知の結果ブロック処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*/
/****************************************************************/
void graffiti_text_result(u8 buf, ST_GRAFFITI_TEXT para)
{
	//共通定義
	T_TEMPLATE_WORK*	pst_work = &st_work[buf];
	s8 err_code = 0;

	//専用結果ブロック構造体を定義 ※
	T_GRAFFITI_TEXT_RESULT* p_total_result = 0;
	u8 i = 0;

	//結果書き込み

	p_total_result = (T_GRAFFITI_TEXT_RESULT *)add_result_blk(	// 結果追加				キャストを変更 ※
		sizeof(T_GRAFFITI_TEXT_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
		pst_work,								// workのポインタ
		err_code,								// 終了状態
		pst_work->ID_template					// 出力券種ID
	);

	p_total_result->result = para.result;
	p_total_result->code = para.code;
	p_total_result->res_num = para.res_num;
	p_total_result->cut_num = para.cut_num;

	//中間情報に書き込む
	if (work[buf].pbs->PlaneInfo[para.plane].note_scan_side == OMOTE)
	{
		work[buf].pbs->mid_res_graffiti_text.up_proc_time = p_total_result->comn.proc_time;
		work[buf].pbs->mid_res_graffiti_text.up_result = para.result;		// 検知結果
		work[buf].pbs->mid_res_graffiti_text.up_res_num = para.res_num;	// 
		work[buf].pbs->mid_res_graffiti_text.up_cut_num = para.cut_num;	// 
		work[buf].pbs->mid_res_graffiti_text.up_err = para.code;

		for (i = 0; i < 8; i++)
		{
			work[buf].pbs->mid_res_graffiti_text.up_res_character[i] = para.res_character[i];
		}

	}
	else
	{
		work[buf].pbs->mid_res_graffiti_text.down_proc_time = p_total_result->comn.proc_time;
		work[buf].pbs->mid_res_graffiti_text.down_result = para.result;		// 検知結果
		work[buf].pbs->mid_res_graffiti_text.down_res_num = para.res_num;	// 
		work[buf].pbs->mid_res_graffiti_text.down_cut_num = para.cut_num;	// 
		work[buf].pbs->mid_res_graffiti_text.down_err = para.code;

		for (i = 0; i < 8; i++)
		{
			work[buf].pbs->mid_res_graffiti_text.down_res_character[i] = para.res_character[i];
		}


		//レベル設定（簡易）　add by furuta 20220405
		if (work[buf].pbs->mid_res_graffiti_text.up_result   == 1 ||
			work[buf].pbs->mid_res_graffiti_text.down_result == 1)      //両面どちらかが法輪功だった場合
		{
			work[buf].pbs->fitness[FITNESS_GRAFFITI].bit.level = 1;     //法輪功
		}
		else if (work[buf].pbs->mid_res_graffiti_text.up_result   == 2 ||
				 work[buf].pbs->mid_res_graffiti_text.down_result == 2)	//両面どちらかに落書きがあった場合
		{
			work[buf].pbs->fitness[FITNESS_GRAFFITI].bit.level = 40;    //落書き
		}
		else	                                                        //異常なしの場合
		{
			work[buf].pbs->fitness[FITNESS_GRAFFITI].bit.level = 100;   //真券
		}

	}

}
#endif
/****************************************************************/
/**
* @brief		カスタムチェックのブロック処理
*@param[in]	work メモリーのポインタ
*@param[out]	<出力引数arg1の説明]>
* @return
*
*/
/****************************************************************/
void customcheck_result(u8 buf, ST_CUSTOMCHECK_PARA ccp)
{
	int ii = 0;
	//共通定義
	T_TEMPLATE_WORK* pst_work = &st_work[buf];
	s8 err_code = 0;

	//専用結果ブロック構造体を定義 ※
	T_CUSTOMCHECK_RESULT* p_total_result = 0;

	//結果書き込み
	p_total_result = (T_CUSTOMCHECK_RESULT*)add_result_blk(	// 結果追加				キャストを変更 ※
		sizeof(T_CUSTOMCHECK_RESULT),				// 追加する領域サイズ	sizeofの対象を変右下x更   ※
		pst_work,								// workのポインタ
		err_code,								// 終了状態
		pst_work->ID_template					// 出力券種ID
	);

	//結果ブロックに書込
	p_total_result->judge = ccp.judge[ccp.result_num];
	p_total_result->level = ccp.level;
	p_total_result->result_num = ccp.result_num;
	p_total_result->result_area = ccp.result_area;
	p_total_result->sum_area = ccp.sum_area;
	p_total_result->sum_num = ccp.sum_num;
	p_total_result->value = ccp.value;

	//中間情報に書き込む	
	for (ii = 0; ii < CUSTOMCHECK_PARA_NUM; ii++)
	{
		work[buf].pbs->mid_res_customcheck.judge[ii] = ccp.judge[ii];
	}
	work[buf].pbs->mid_res_customcheck.proc_time = p_total_result->comn.proc_time;
	work[buf].pbs->mid_res_customcheck.level = ccp.level;
	work[buf].pbs->mid_res_customcheck.result_area = ccp.result_area;
	work[buf].pbs->mid_res_customcheck.result_num = ccp.result_num;
	work[buf].pbs->mid_res_customcheck.sum_area = ccp.sum_area;
	work[buf].pbs->mid_res_customcheck.sum_num = ccp.sum_num;
	work[buf].pbs->mid_res_customcheck.value = ccp.value;

}


/****************************************************************/
/**
 * @brief		現在の結果ブロック最後尾に、新たな結果ブロック領域を確保できるかチェックし、
				OKなら、共通項に値をセットする。
 *@param[in]	新たな結果ブロックのサイズ (可変長部、パディングを含む)、　work メモリーのポインタ、　終了状態コード、　このブロックが返すテンプレート内券種ID
 *@param[out]	<出力引数arg1の説明]>
 * @return		追加中のブロックの先頭アドレス	 
 */
/****************************************************************/
void*	add_result_blk(u16	siz, T_TEMPLATE_WORK* pst_work, u16 cnd_code, u16 tmplt_ID)
{
	T_DECISION_RESULT_COMMON*	ablkp;
	T_DECISION_RESULT_COMMON*	next_ablkp;

	u32	ovalcap = 0;

	ablkp = pst_work->now_result_blk_pt;

	// 結果ブロック領域外のチェック
	ovalcap =  (u32)(((u8*)pst_work->now_result_blk_pt - (T_POINTER)pst_work->p_result_blk_top) + siz);		// 追加した後の容量を計算し、
	if( ovalcap >= RESULT_TBL_SIZE)										// 容量がMAXを超える事が判れば、
	{																	// エラーとする。
		if(pst_work->e_code == 0)										//
		{
			pst_work->e_code = E_RESULT_AREA_OVER;						// 結果領域が足りないエラーとして、結果ブロックを更新せずにRET
			return ablkp;
		}
	}
	memset(pst_work->now_result_blk_pt, 0, siz);						// 固定長分と可変長部の領域を0クリアする

	ablkp->next_block_ofs = siz;										// 次ブロックオフセット値をセット 
	ablkp->source_blk_ofs = (u32)((u8*)pst_work->now_para_blk_pt - (T_POINTER)ablkp);// 対応元の判定パラメタブロックの先頭オフセット値
	ablkp->serial_number = pst_work->now_para_blk_pt->serial_number;	// 結果ブロック連番
	ablkp->function_code = pst_work->now_para_blk_pt->function_code;	// 機能コード
	ablkp->cnd_code	= cnd_code;											// 状態コード
	ablkp->tmplt_ID = tmplt_ID;											// テンプレート内券種ID
	ablkp->proc_time = pst_work->proc_tm_co_e - pst_work->proc_tm_co_s;	// 処理時間計算

	//address = (u32)pst_work->now_result_blk_pt + siz;
	pst_work->now_result_blk_pt = (T_DECISION_RESULT_COMMON*)((u8*)pst_work->now_result_blk_pt + siz);	// 現在の判定結果ブロックのポインタを更新する。
	next_ablkp = pst_work->now_result_blk_pt;
	next_ablkp->function_code = FNC_END;	//次の結果ブロックにエンドマークを書き込む

	return (void*)ablkp;
}

/****************************************************************/
/**
* @brief		パディングデータを追加した場合のデータ長を計算して返す。		
 *@param[in]	データ本体のデータ長
 *@param[out]	なし
 * @return		パディングデータを追加した場合のデータ長	 
 */
/****************************************************************/
u16	calculate_size_including_padding_data(u16 body_siz)
{
	return	body_siz + ((4 - body_siz % 4) % 4);	
}
