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
 * @file template.h
 * @brief  TEMPLATEのヘッダファイルです。
 * @date 2018.08.22
 * @author JCM. OSAKA TECHNICAL RESARCH 1 GROUP ELEMENTAL TECHNOLOGY RECERCH DEPT.
 */
/****************************************************************************/

#ifndef _TEMPLATE_H_INCLUDED_
#define _TEMPLATE_H_INCLUDED_

//typedef unsigned char	u8;
//typedef	signed char		s8;
//typedef	unsigned short	u16;
//typedef	signed short	s16;
//typedef	unsigned long	u32;
//typedef	signed long		s32;

enum
{
	category1 = 0xF0,
	category2 = 0xF1,
	category3 = 0xF2,
	category4a = 0xF3,
	category4b = 0xF4
};

enum {
	SET_DYENOTE_ONLY = 1, 
	SET_CURRENCY_THRS, 
	SET_DEFAULT_THRS,
	SET_FROM_SUMPLING_DATA
};	

enum {
	MAX_LEVEL = 1,
	MIN_LEVEL = 100
};	

enum {
	RES_ALLOVER_COUNTERFEIT			 = 0x241c,	// 偽造券・変造券
	RES_ALLOVER_COUNTERFEIT_DOUBT	 = 0x241d,	// 偽造券・変造券疑い
	RES_ALLOVER_FEATURE_PEELING		 = 0x241e,	// 特徴欠損券
	RES_ALLOVER_WASH				 = 0x241f,	// 洗濯券
	RES_ALLOVER_COPY				 = 0x2420,	// コピー券
	RES_ALLOVER_UNFIT				 = 0x2421,	// 損券
	RES_ALLOVER_UF_COUNTERFEIT_DOUBT = 0x2422,	// 損券の偽造券・変造券疑い
	RES_ALLOVER_TATTERED			 = 0x2423,	// ボロボロ券
	RES_ALLOVER_REJECT_NODE			 = 0x2424,	// NN識別リジェクト判定
	RES_ALLOVER_BLACK_LIST			 = 0x2425,	// ブラックリストに該当 
	RES_ALLOVER_BIG_UNFIT			 = 0x242D,	// 大損券によるリジェクト判定
	RES_ALLOVER_DIFF_SERIES			 = 0x2434	// 異番号判定

	//RES_ALLOVER_TRUE				 = 0x2424	// 正券
};	

enum {							//損券状態における、鑑別処理への影響度順
	UF_NO_IMPACT   = 0,			//正券
	UF_IMPACT_TAPE = 1,			//テープ	
	UF_IMPACT_TEAR = 2,			//裂け
	UF_IMPACT_DOG_EAR = 2,		//角折れ
	UF_IMPACT_WASH = 3,			//洗濯
	UF_IMPACT_HOLE = 4,			//穴
	UF_IMPACT_STAIN = 5,		//染み
	UF_IMPACT_SOILING = 6,		//汚れ
	UF_IMPACT_DEINKED = 7,		//脱色
	UF_IMPACT_MUTILATION = 8,	//欠損
	UF_IMPACT_FOLDING = 8,		//折り畳み
	UF_IMPACT_PROC_NUM
};	

enum {								//正損判定結果を基に閾値レベルに補正を受ける鑑別処理の並び
	OVERALL_VALIDATION_MCIR = 0,	//MCIR
	OVERALL_VALIDATION_NN1,			//NN1
	OVERALL_VALIDATION_NN2,			//NN2
	OVERALL_VALIDATION_MAG,			//磁気検知
	OVERALL_VALIDATION_UV,			//UV検知
	OVERALL_VALIDATION_IR2_WAVE,	//IR2波長差　シクパトーク
	OVERALL_VALIDATION_NEOMAG,		//ネオマグ
	OVERALL_VALIDATION_SP_A,		//特殊A
	OVERALL_VALIDATION_SP_B,		//特殊B
	OVERALL_VALIDATION_MAG_THREAD,	//磁気スレッド
	OVERALL_VALIDATION_METAL_THREAD,//金属スレッド
	OVERALL_VALIDATION_HOLOGRAM,	//ホログラム
	OVERALL_VALIDATION_CF_NN_1COLOR,//CF-NN1Color
	OVERALL_VALIDATION_CF_NN_2COLOR,//CF-NN2Color
	OVERALL_VALIDATION_CUSTOMCHECK, //ｶｽﾀﾑﾁｪｯｸ
	OVERALL_VALIDATION_PROC_NUM		//総数


};	

enum {								//カウンター種別
	JUDGE_INVALID = 0,
	JUDGE_CF      = 1,
	JUDGE_UF      = 2,
	JUDGE_FP      = 3
};

//総合判定で損券結果に応じて鑑別レベルを変更するときの値	閾値が40の場合
#define  C_NO	0		//鑑別処理の閾値レベル補正値(無)　	0
#define  CSML2	0.125	//鑑別処理の閾値レベル補正値(最小)　5 
#define  CSML	0.250	//鑑別処理の閾値レベル補正値(小)　	10 
#define  CMED	0.375	//鑑別処理の閾値レベル補正値(中)　	15
#define  CBIG	0.500	//鑑別処理の閾値レベル補正値(大)	20
#define  CBIG2	0.875	//鑑別処理の閾値レベル補正値(最大)　35
#define  CDIS	-1		//考慮しない



//#define	EN_10ns_CO						// シミュレーション時はコメントアウトする。

#define	BUF_NUM			(100)			// Bufferの数
#define	RESULT_TBL_SIZE		(0x10000)	// 判定結果テーブル容量　64KB
//#define	FILE_INFO_SIZE	(0x1b04)		// ファイル情報のサイズ
#define	FILE_INFO_SIZE	(0x100)		// ファイル情報のサイズ
#define	MAX_TEMPLATE_BLOCK_NAM	(0xffff)	//
#define FITNESS_REPAIRS_MECHA_THRESH_MIN	15			// 修復テープ(メカ)閾値最小値
#define FITNESS_REPAIRS_MECHA_THRESH_MAX	60			// 修復テープ(メカ)閾値最大値
#define TEMPLATE_OVERALL_NOT_INCLUDED_IN_RESULTS 0		//総合判定にて、閾値レベルが0と設定されていた場合、カテゴリ判定にはその処理は考慮しないこととする。
#define BARCODE_USE_PARAMS_SET_AT_THE_TOP	1		//バーコードリード処理において、上位で設定されたパラメタを用いる


// 対象ID予約値
#define	CPID_ALL		(0x0001)
#define	CPID_RANGE		(0x0002)
#define	TLPT_ID_INIT	(0xf001)

// 機能コード
#define	FNC_END				    (0x0000)	// ENDマーク
#define	FNC_BRK				    (0x0001)	// 中断マーク
#define	FNC_OUTLINE_DETECT	    (0x0010)	// 外形検知
#define	FNC_PDT_EXTRACTION	    (0x0021)	// ポイントデータ抽出ver2
#define	FNC_NN_JUDGE		    (0x0030)	// NN判定
#define FNC_SHIN_SHIKI_BETSU    (0x0031)    // 新識別 0x0213->0x031	add by thinh
#define	FNC_NN_OVALL_JUDGE	    (0x0035)	// 複合NN総合判定
#define	FNC_OL_SIZE_CHECK	    (0x0040)	// 外形サイズチェック
#define	FNC_OL_SIZE_IDENT	    (0x004C)	// 外形サイズ識別 
#define	FNC_OVERALL_JUDGE	    (0x0050)	// 総合判定結果
#define	FNC_CIR_3COLOR		    (0x0060)	// ３色比
#define	FNC_CIR_4COLOR		    (0x0061)	// ４色比
#define	FNC_IR_CHECK		    (0x0062)	// IRチェック
#define	FNC_MCIR			    (0x0063)	// MCIR
#define	FNC_DOUBLE_CHECK	    (0x0064)	// 重券チェック
#define	FNC_VALI_NN_1COLOR	    (0x0065)	// １色NN
#define	FNC_VALI_NN_2COLOR	    (0x0066)	// ２色NN
#define	FNC_CUSTOM_CHECK	    (0x0067)	// カスタムチェック
#define	FNC_CIR_2COLOR		    (0x0068)	// ２色差
#define	FNC_DOG_EAR			    (0x0070)	// 角折れ検知
#define	FNC_TEAR			    (0x0080)	// 裂け検知
#define	FNC_DYE_NOTE		    (0x0090)	// ダイノート検知
#define	FNC_OCR				    (0x00a0)	// OCR
#define FNC_HOLE			    (0x00b0)	// 穴検知
#define	FNC_SOILING			    (0x00c0)	// 汚れ検知
#define	FNC_DE_INK			    (0x00d0)	// 脱色検知
#define	FNC_TAPE			    (0x00e0)	// テープ検知
#define	FNC_FOLDING			    (0x00f0)	// 折りたたみ
#define	FNC_MUTILATION		    (0x0110)	// 欠損
#define	FNC_STAIN			    (0x0120)	// 染み
#define	FNC_DUAL_IR			    (0x0130)	// IR2
#define	FNC_MAG				    (0x0140)	// 磁気
#define	FNC_UV_VALIDATE		    (0x0150)	// 蛍光鑑別
#define	FNC_CAP_TAPE		    (0x0160)	// 静電テープ検知
#define FNC_NEOMAG			    (0x0170)	// NEOMAG検知
#define FNC_MAGTHREAD		    (0x0180)	// 磁気スレッド検知
#define FNC_METALTHREAD		    (0x0190)	// 金属スレッド検知
#define FNC_SPECIAL_A		    (0x01A0)	// 特殊A検知
#define FNC_SPECIAL_B		    (0x01B0)	// 特殊B検知
#define FNC_HOLOGRAM		    (0x01C0)	// ホログラム検知
#define FNC_BAR_CODE		    (0x01D0)	// バーコードリード
#define	FNC_WASH			    (0x01E0)	// 蛍光正損
#define FNC_DBL_CHK_MECHA	    (0x01F0)	// 重券検知（メカ厚）
#define FNC_JUKEN			    (0x0200)	// 重券検知 SVM
#define FNC_JUKEN_NN		    (0x0201)	// 重券検知 NN
#define FNC_GRAFFITI_FACE		(0x0210)	// 落書き検知_顔落書き検知 add by thinh
#define FNC_GRAFFITI_ABNORMAL   (0x0211)    // 落書き検知_異常検知 add by thinh
#define FNC_GRAFFITI_TEXT		(0x0212)    // 落書き検知_文字検知 add by thinh
#define FNC_CF_NN_1COLOR		(0x0220)    // 鑑別CF-NN 1 Color add by furuta
#define FNC_CF_NN_2COLOR		(0x0230)    // 鑑別CF-NN 2 Color add by furuta




#define	FNC_JMP				(0x0100)	// JMP
//#define	FNC_JCMID_TBL		(0xF000)	// テンプレート内券種　JCM ID　対応表　ver1
//#define	FNC_JCMID_TBL		(0xF001)	// テンプレート内券種　JCM ID　対応表　ver2
//#define	FNC_JCMID_TBL		(0xF002)	// テンプレート内券種　JCM ID　対応表　ver3
#define	FNC_JCMID_TBL		(0xF003)	// テンプレート内券種　JCM ID　対応表　ver4




// エラーコード
#define	E_UNDEFIND_FNC_CODE		 (0xF02E)	// 定義されていない機能コード
#define	E_RESULT_AREA_OVER		 (0xF02F)	// 結果領域が足りない
#define	E_FITNESS_THRESHOLD1	 (0xF030)	// フィットネス閾値1が異常値
#define	E_FITNESS_THRESHOLD2	 (0xF031)	// フィットネス閾値2が異常値
#define	E_FITNESS_THRESHOLD_OVER (0xF032)	// フィットネス閾値1が2以上の値
#define	E_VALIDATION_THRESHOLD	 (0xF033)	// 鑑別閾値レベルが異常値
#define	E_ID_TRANS_ERR			 (0xF034)	// 鑑別閾値レベルが異常値
#define	E_NOT_FIND_CURRENCY_TABLE (0xF035)	// テンプレート内にあるはずの通貨テーブルが見つからなかった。


//リザルトコード
#define ERR_MULTI_NN			 (0x0c2a)	//コード：複数発火


// 構造体宣言　pack 4

//************************************************************
//判定パラメタブロック構造体群
//************************************************************
//****************************************************
//共通項

#if 1
typedef struct							// 判定パラメタ　共通項	48byte
{
	u32	next_block_ofs;						// 次ブロックへのオフセットアドレス値
	u16	serial_number;						// 判定パラメタブロック連番
	u16	function_code;						// 機能コード
	u32	ID_lst_ofs;							// 処理対象とするテンプレート内券種IDリスト実体へのオフセット値
	u8	ID_lst_num;							// 処理対象とするテンプレート内券種IDの要素数
	u8	trans_mode;							// 処理対象とするモード　andして0ならその処理は無視
	u8	padding[2];							// パディングデータ
	u8	tool_area[32];						// ツール使用領域
} T_DECISION_PARAMETER_COMMON;
#else
typedef struct							// 判定パラメタ　共通項
{
	u32	next_block_ofs;						// 次ブロックへのオフセットアドレス値
	u16	serial_number;						// 判定パラメタブロック連番
	u16	function_code;						// 機能コード
	u32	ID_lst_ofs;							// 処理対象とするテンプレート内券種IDリスト実体へのオフセット値
	u8	ID_lst_num;							// 処理対象とするテンプレート内券種IDの要素数
	u8	padding[3];							// パディングデータ
} T_DECISION_PARAMETER_COMMON;
#endif

typedef struct							// 判定パラメタ　JMP　処理　	
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u32	jmp_ofs;							// jmp先オフセットアドレス
} T_DECISION_PARAMETER_JMP;

typedef struct							// 判定パラメタ　テンプレート内券種ID - JCMID 対応テーブル　	
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
#ifdef _RM_
	u32	tbl_ofs;							// テーブルオフセットアドレス
#else
	u32	tbl_ofs;							// テーブルオフセットアドレス
#endif

	u16	tbl_num;							// テーブルの構造体配列要素数
	u16	padding;							// padding
} T_DECISION_PARAMETER_JCMID_TBL;

typedef struct							// 判定パラメタ　テンプレート内券種ID - JCMID 対応テーブル　可変長部　	
{
	u16	tmpltID;							// テンプレート内券種ID
	u8	jcmID_num;							// 対応するJCMIDの要素数
	u8	padding;							// padding
	u32	jcmID[16];							// JCM ID券種IDリスト
} T_DECISION_PARAMETER_JCMID_TBL_OBJ;

typedef struct							// 判定結果ブロック　共通項	
{
	u32	next_block_ofs;					// 次ブロックへのオフセットアドレス値
	u32	source_blk_ofs;					// 対応元の判定パラメタブロックの先頭オフセット値
	u16	serial_number;					// 結果ブロック連番
	u16	function_code;					// 機能コード
	u16	cnd_code;						// 終了状態コード　基本的にエラー時は、続行不可とし、workのエラーコードをセットする。
	u16	tmplt_ID;						// テンプレート内券種ID
	u32	proc_time;						// 処理時間
} T_DECISION_RESULT_COMMON;

//-------------------------------------------------------------------------
typedef struct	// テンプレートシーケンス　ワークメモリ
{
	T_DECISION_PARAMETER_COMMON*	now_para_blk_pt;// 現在の判定ブロックのポインタ	
	u16	ID_template;					// 現在のテンプレート内券種ID	
	u16	e_code;							// エラーコード　この紙幣の識鑑別は中断する。
	void*	p_result_blk_top;			// 判定結果ブロックの先頭ポインタ
	T_DECISION_RESULT_COMMON*	now_result_blk_pt;	// 現在の判定結果ブロックのポインタ
	u32	proc_tm_co_s;					// 処理時間計測用開始時のカウント値
	u32	proc_tm_co_e;					// 　　　　　　　終了時のカウンタ値
	u8	buf_n;							// Buffer番号
	u8	current_mode;					//　現在のモードを設定する。　19/7/22追加
	u8	template_shell_completion;		// テンプレートシェルの処理が終了したかを判断する。
	u8  thrs_set_complete;				// 閾値レベルの設定が完了しているかを判断する。
	u32 JCMID;							// 現在のJCMIDを記録する。
	u8	side_masking_area;				// マスキングエリアの範囲（横
	u8	vertical_masking_area;			// マスキングエリアの範囲（縦 
	
} T_TEMPLATE_WORK;

typedef struct							// 判定パラメタ　総合判定	
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	
    //鑑別
	u8	_3cir;	                //
	u8	_4cir;	                //
	u8	ircheck;	            //
	u8	mcir;	                //
	u8	nn1;	                //
	u8	nn2;	                //
	u8	ir2;	                //
	u8	mag;	                //
	u8	uv;	                    //
	u8	neomag;                 //	
	u8	spa;                    //
	u8	spb;                    //
	u8	cf_nn1;                 //
	u8	cf_nn2;                 //
	u8  customcheck;
	u8  padding1[9];           //

	//特徴剥がれ
	u8	magthread;	            // 
	u8	metalthread;            // 
	u8	hologram;	            // 
	u8	padding2[9];            // 

	//正損
	u8	dog_ear;                // 
	u8	tear;	                // 
	u8	soiling;                // 
	u8	deink;		            // 
	u8	stain;                  // 
	u8	hole;                   // 
	u8	fold;	                // 
	u8	mutilation;		        // 
	u8	tape_mecha;		        //  
	u8	tape_cap;               // 
	u8	wash;                   // 
	u8	padding3[13];			// 

	//制御フラグ
	u8	exclusion_priority;		// 排除優先モード　0：OFF　1：ON
	u8	padding4[7];			// 

} T_DECISION_OVER_ALL;


//************************************************************
//前処理部　識別部
typedef struct							// 判定パラメタ　外形検知	
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u8	ei_wr;								// 中間情報にも書き込む
	u8  plane;								// 用いる色情報
	s16 Step_Movement ;						// 探索のステップ幅(mm) ※暫定値
	s16 Small_Backtrack ;					// 次の探索開始位置への戻り幅(dot) ※暫定値
	u8	max_skew_thr;						// 最大スキューの値
	u8	vertex_err_determination;			// 頂点が搬送路外かどうかのエラー判定を省略する　1:実行　0：実行しない
	u8	abnormal_position_of_ticket_edge;	// スキャン開始位置に券端があり続けたときにエラーとする判定を省略する。　1:実行　0：実行しない
	u8  th_dynamic_decision_flg;			// 閾値動的決定　1:実行　0：実行しない
	u8  multiple_transport;					// 多重搬送検知　BAULE17用 1:実行　0：実行しない
	u8  padding;							// 

} T_DECISION_PARAMETER_OUTLINE;

#if 1		// Ver2
// ポイントデータ抽出
// マスク情報構造体
typedef struct
{
	u8 plane_no;							// プレーン番号
	u8 mask_size_x;							// マスクサイズXdot
	u8 mask_size_y;							// マスクサイズYdot
	u8	padding1;
	u16	devdt;								// 割る数		こっちが本物
	u8	padding[2];
	u32	planes_lst_ofs;						// マスクパターンの先頭オフセット値 (サイズはX*Y byte)
} T_POINTDT_MASK_INFO;

typedef struct							// 判定パラメタ　ポイントデータ抽出パラメタ
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u8	x_dev_num;							// 横分割数
	u8	y_dev_num;							// 縦分割数
	u8	ei_corner;							// コーナー無効に　する:0/しない:1
	u8	after_ident;						// 識別後フラグ 0：識別前 / 1：識別後
	u8	padding;							// 空
	u8  num;								// 有効要素数　MAX16
	T_POINTDT_MASK_INFO	mask_info[16];
} T_DECISION_PARAMETER_POINTDT;

//#else
//typedef struct							// 判定パラメタ　ポイントデータ抽出パラメタ　	
//{
//	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
//	u16	outline_blk_num;					// 外形検知の判定結果ブロック連番を指定する。
//	u8	x_dev_num;							// 横分割数
//	u8	y_dev_num;							// 縦分割数
//	u8	ei_corner;							// コーナー無効に　する:0/しない:1
//	u8	padding[1];
//	u8	mask_size_x;
//	u8	mask_size_y;
//	u32	mask_dt_ofs;
//	u16	mask_dt_num;
//	u16	devdt;
//	u32	planes_lst_ofs;
//	u8	planes_lst_num;
//	u8	padding2[3];
//} T_DECISION_PARAMETER_POINTDT;
#endif

typedef struct							// 判定パラメタ　NN判定パラメタ　	
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u16	NN_in_num;							// 入力層ノード数
	u16	NN_midle_num;						// 中間層ノード数
	u16	NN_out_num;							// 出力層ノード数
	u32	weight_dt_ofs;						// ウェイトデータの先頭オフセット値
	u32	weight_dt_num;						// ウェイトデータの要素数
	u32	node_inf_ofs;						// 出力ノードごとの割り当て先頭オフセット値
	u16	node_inf_num;						// 出力ノードごとの割り当て情報の要素数
	u8	no_change_ID;						// テンプレート内券種IDを　1:更新しない 0:更新する
	u8	use_bias;							// バイアスノードを　1:使用する 0:使用しない
	u8	add_size;							// 入力ノードにサイズデータを　1:加える 0:加えない
	u8	padding[3];							// padding

} T_DECISION_PARAMETER_NN;

typedef struct							// 判定パラメタ　NN判定パラメタ　出力ノードごとの情報　可変長部	
{
	float	first_val;						// 第一発火値の制限値
	float	deff_val;						// 第二発火値との差の制限値
	float	th3_val;						// th3(softmax)の閾値		19/07/11追加
	float	th5_val;						// th5(err)の閾値		19/07/11追加
	u16	tmpkenID;							// 券種ID
	u8	padding[2];
} T_DECISION_PARAMETER_NODE_INF;

typedef struct							// 判定パラメタ　サイズ識別パラメタ	
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u16	x_mini;								// 長手方向サイズ　最小値　単位は0.1ｍｍ
	u16	x_max;								// 長手方向サイズ　最大値
	u16	y_mini;								// 短手方向サイズ　最小値
	u16	y_max;								// 短手方向サイズ　最大値
	u16 template_id;						// テンプレート内券種ID
} T_DECISION_PARAMETER_SIZ_IDENT;

//***********************************************************
//鑑別部
typedef struct
{
	int check;
	int index;
	int x;
	int y;
	int min_limit;
	int max_limit;

}DLL_MESH_CHECK;

#define CIR3_MODE 6
typedef struct							// 判定パラメタ　3色比
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	s8 calculation_mode[CIR3_MODE];			// 計算モード
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16	level_standard_value;				// 判定基準値

	u16 red_mask_ptn_diameter_x;			// 赤色マスクパターンの直径x
	u16 red_mask_ptn_diameter_y;			// 赤色マスクパターンの直径y
	float red_mask_ptn_divide_num;			// 赤色マスクパターンの割る数
	u32	red_mask_ptn_ofs;					// 赤色マスクパターンの配列オフセット
	u16	red_mask_ptn_num;					// 赤色マスクパターンの配列要素数
	u8	padding2[2];						// paddingdata

	u16 grn_mask_ptn_diameter_x;			// 緑色マスクパターンの直径x
	u16 grn_mask_ptn_diameter_y;			// 緑色マスクパターンの直径y
	float grn_mask_ptn_divide_num;			// 緑色マスクパターンの割る数
	u32	grn_mask_ptn_ofs;					// 緑色マスクパターンの配列オフセット
	u16	grn_mask_ptn_num;					// 緑色マスクパターンの配列要素数
	u8	padding3[2];						// paddingdata

	u16 ir1_mask_ptn_diameter_x;			// IR1マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1マスクパターンの割る数
	u32	ir1_mask_ptn_ofs;					// IR1マスクパターンの配列オフセット
	u16	ir1_mask_ptn_num;					// IR1マスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

} T_DECISION_PARAMETER_CIR3;

#define CIR4_MODE 8
typedef struct							// 判定パラメタ　4色比
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	s8 calculation_mode[CIR4_MODE];			// 計算モード
	u8	padding1[2];						// paddingdata
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16	level_standard_value;				// 判定基準値

	u16 red_mask_ptn_diameter_x;			// 赤色マスクパターンの直径x
	u16 red_mask_ptn_diameter_y;			// 赤色マスクパターンの直径y
	float red_mask_ptn_divide_num;			// 赤色マスクパターンの割る数
	u32	red_mask_ptn_ofs;					// 赤色マスクパターンの配列オフセット
	u16	red_mask_ptn_num;					// 赤色マスクパターンの配列要素数
	u8	padding3[2];						// paddingdata

	u16 grn_mask_ptn_diameter_x;			// 緑色マスクパターンの直径x
	u16 grn_mask_ptn_diameter_y;			// 緑色マスクパターンの直径y
	float grn_mask_ptn_divide_num;			// 緑色マスクパターンの割る数
	u32	grn_mask_ptn_ofs;					// 緑色マスクパターンの配列オフセット
	u16	grn_mask_ptn_num;					// 緑色マスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

	u16 ir1_mask_ptn_diameter_x;			// IR1マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1マスクパターンの割る数
	u32	ir1_mask_ptn_ofs;					// IR1マスクパターンの配列オフセット
	u16	ir1_mask_ptn_num;					// IR1マスクパターンの配列要素数
	u8	padding5[2];						// paddingdata

	u16 ir2_mask_ptn_diameter_x;			// IR2マスクパターンの直径x
	u16 ir2_mask_ptn_diameter_y;			// IR2マスクパターンの直径y
	float ir2_mask_ptn_divide_num;			// IR2マスクパターンの割る数
	u32	ir2_mask_ptn_ofs;					// IR2マスクパターンの配列オフセット
	u16	ir2_mask_ptn_num;					// IR2マスクパターンの配列要素数
	u8	padding6[2];						// paddingdata

} T_DECISION_PARAMETER_CIR4;

#define IR_MODE 2
typedef struct							// 判定パラメタ　IRチェック
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	s8 calculation_mode[IR_MODE];			// 計算モード
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16	level_standard_value;				// 判定基準値

	u16 ir1_mask_ptn_diameter_x;			// IR1マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1マスクパターンの割る数
	u32	ir1_mask_ptn_ofs;					// IR1マスクパターンの配列オフセット
	u16	ir1_mask_ptn_num;					// IR1マスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

} T_DECISION_PARAMETER_IR_CHECK;

#define MCIR_MODE 6
typedef struct							// 判定パラメタ　MCIRチェック
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	s8 calculation_mode[MCIR_MODE];			// 計算モード
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16	level_standard_value;				// 判定基準値

	u16 red_mask_ptn_diameter_x;			// 赤色マスクパターンの直径x
	u16 red_mask_ptn_diameter_y;			// 赤色マスクパターンの直径y
	float red_mask_ptn_divide_num;			// 赤色マスクパターンの割る数
	u32	red_mask_ptn_ofs;					// 赤色マスクパターンの配列オフセット
	u16	red_mask_ptn_num;					// 赤色マスクパターンの配列要素数
	u8	padding2[2];						// paddingdata

	u16 grn_mask_ptn_diameter_x;			// 緑色マスクパターンの直径x
	u16 grn_mask_ptn_diameter_y;			// 緑色マスクパターンの直径y
	float grn_mask_ptn_divide_num;			// 緑色マスクパターンの割る数
	u32	grn_mask_ptn_ofs;					// 緑色マスクパターンの配列オフセット
	u16	grn_mask_ptn_num;					// 緑色マスクパターンの配列要素数
	u8	padding3[2];						// paddingdata

	u16 ir1_mask_ptn_diameter_x;			// IR1マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1マスクパターンの割る数
	u32	ir1_mask_ptn_ofs;					// IR1マスクパターンの配列オフセット
	u16	ir1_mask_ptn_num;					// IR1マスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

} T_DECISION_PARAMETER_MCIR;

typedef struct							// 判定パラメタ　重券
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u16 thrshold;							// 閾値
	//u16	level_standard_value;				// 判定基準値
	//u8	padding[2];							

	u16 red_mask_ptn_diameter_x;			// 赤色マスクパターンの直径x
	u16 red_mask_ptn_diameter_y;			// 赤色マスクパターンの直径y
	float red_mask_ptn_divide_num;			// 赤色マスクパターンの割る数
	u32	red_mask_ptn_ofs;					// 赤色マスクパターンの配列オフセット
	u16	red_mask_ptn_num;					// 赤色マスクパターンの配列要素数
	u8	padding2[2];						// paddingdata

	u16 grn_mask_ptn_diameter_x;			// 緑色マスクパターンの直径x
	u16 grn_mask_ptn_diameter_y;			// 緑色マスクパターンの直径y
	float grn_mask_ptn_divide_num;			// 緑色マスクパターンの割る数
	u32	grn_mask_ptn_ofs;					// 緑色マスクパターンの配列オフセット
	u16	grn_mask_ptn_num;					// 緑色マスクパターンの配列要素数
	u8	padding3[2];						// paddingdata

	u16 tred_mask_ptn_diameter_x;			// 透過赤マスクパターンの直径x
	u16 tred_mask_ptn_diameter_y;			// 透過赤マスクパターンの直径y
	float tred_mask_ptn_divide_num;			// 透過赤マスクパターンの割る数
	u32	tred_mask_ptn_ofs;					// 透過赤マスクパターンの配列オフセット
	u16	tred_mask_ptn_num;					// 透過赤マスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

} T_DECISION_PARAMETER_DOUBLE_CHECK;


typedef struct								// 新重券検知
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項

	u16 reference_blk_num;					// 参照入力結果連番
	u8	x_dev_num;							// 横分割数
	u8	y_dev_num;							// 縦分割数
	u8	ei_corner;							// コーナー無効に　する:0/しない:1
	u8	padding[2];
	u8  num;								// 有効要素数　MAX16

	u16 input_num;
	u16 hidden_num;
	u16 output_num;
	u8 biasflag;
	u8 padding1;
	u16 label[8];
	u32 offset_weight;

	T_POINTDT_MASK_INFO	mask_info[16];

} T_DECISION_PARAMETER_DOUBLE_OPTICS;

typedef struct							// 判定パラメタ　NN1
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番

	u16	NN_in_num;							// 入力層ノード数
	u16	NN_midle_num;						// 中間層ノード数
	u16	NN_out_num;							// 出力層ノード数

	u32	input_prm_ofs;						// 入力座標構造体の先頭オフセット値
	u16 input_prm_num;						// 入力座標構造体の要素数
	u8	padding1[2];							// padding

	u32	hidden_weight_dt_ofs;				// 中間層ウェイトデータの先頭オフセット値
	u16	hidden_weight_dt_num;				// 中間層ウェイトデータの要素数
	u8	padding2[2];							// padding

	u32	out_weight_dt_ofs;					// 出力ウェイトデータの先頭オフセット値
	u16	out_weight_dt_num;					// 出力ウェイトデータの要素数
	u8 	level_standard_value_1;				// 判定基準値	*0.01して使う
	u8	level_standard_value_2;				// 判定基準値	*0.01して使う

	u16 ir1_mask_ptn_diameter_x;			// IR1マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1マスクパターンの割る数
	u32	ir1_mask_ptn_ofs;					// IR1マスクパターンの配列オフセット
	u16	ir1_mask_ptn_num;					// IR1マスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

} T_DECISION_PARAMETER_NN1;


typedef struct							// 判定パラメタ　NN2
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番

	u16	NN_in_num;							// 入力層ノード数
	u16	NN_midle_num;						// 中間層ノード数
	u16	NN_out_num;							// 出力層ノード数

	u32	input_prm_ofs;						// 入力座標構造体の先頭オフセット値
	u16 input_prm_num;						// 入力座標構造体の要素数
	u8	padding1[2];						// padding

	u32	hidden_weight_dt_ofs;				// 中間層ウェイトデータの先頭オフセット値
	u16	hidden_weight_dt_num;				// 中間層ウェイトデータの要素数
	u8	padding2[2];						// padding

	u32	out_weight_dt_ofs;					// 出力ウェイトデータの先頭オフセット値
	u16	out_weight_dt_num;					// 出力ウェイトデータの要素数
	u8 	level_standard_value_1;				// 判定基準値	*0.01して使う
	u8	level_standard_value_2;				// 判定基準値	*0.01して使う

	u16 red_mask_ptn_diameter_x;			// 赤色マスクパターンの直径x
	u16 red_mask_ptn_diameter_y;			// 赤色マスクパターンの直径y
	float red_mask_ptn_divide_num;			// 赤色マスクパターンの割る数
	u32	red_mask_ptn_ofs;					// 赤色マスクパターンの配列オフセット
	u16	red_mask_ptn_num;					// 赤色マスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

	u16 ir1_mask_ptn_diameter_x;			// IR1マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1マスクパターンの直径y
	float ir1_mask_ptn_divide_num;			// IR1マスクパターンの割る数
	u32	ir1_mask_ptn_ofs;					// IR1マスクパターンの配列オフセット
	u16	ir1_mask_ptn_num;					// IR1マスクパターンの配列要素数
	u8	padding5[2];						// paddingdata

} T_DECISION_PARAMETER_NN2;

typedef struct							// 判定パラメタ　カスタムチェック
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16 padding;
} T_DECISION_PARAMETER_CUSTOM;

#define CIR2_MODE 30
typedef struct							// 判定パラメタ　2色差
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	s8 calculation_mode[CIR2_MODE];			// 計算モード
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u8	padding[2];							// paddingdata
} T_DECISION_PARAMETER_CIR2;

typedef struct							// 判定パラメタ　IR2波長
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項

	u16 reference_blk_num;					// 参照入力結果連番
	u8	padding1[2];						// padding1
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16 ir1_mask_ptn_diameter_x;			// IR1マスクパターンの直径x
	u16 ir1_mask_ptn_diameter_y;			// IR1マスクパターンの直径y
	u8	padding2[2];						// padding2
	float ir1_mask_ptn_divide_num;			// IR1マスクパターンの割る数
	u32	ir1_mask_ptn_ofs;					// IR1マスクパターンの配列オフセット
	u16	ir1_mask_ptn_num;					// IR1マスクパターンの配列要素数
	u8	padding3[2];						// paddingdata

} T_DECISION_PARAMETER_IR2WAVE;

typedef struct							// 判定パラメタ　磁気　修正後 size76
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u8	padding1[2];						// padding1

	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16 mag_mask_ptn_diameter_x;			// MAGマスクパターンの直径x

	u16 mag_mask_ptn_diameter_y;			// MAGマスクパターンの直径y
	u8	padding2[2];						// padding1

	float mag_mask_ptn_divide_num;			// MAGマスクパターンの割る数
	u32	mag_mask_ptn_ofs;					// MAGマスクパターンの配列オフセット
	u16	mag_mask_ptn_num;					// MAGマスクパターンの配列要素数
	u8	padding4[2];						// paddingdata

} T_DECISION_PARAMETER_MAG;

typedef struct							// 判定パラメタ　蛍光
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u8	padding1[2];						// padding1
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16 uv_mask_ptn_diameter_x;			// UVマスクパターンの直径x
	u16 uv_mask_ptn_diameter_y;			// UVマスクパターンの直径y
	u8	padding2[2];						// padding1
	float uv_mask_ptn_divide_num;			// UVマスクパターンの割る数
	u32	uv_mask_ptn_ofs;					// UVマスクパターンの配列オフセット
	u16	uv_mask_ptn_num;					// UVマスクパターンの配列要素数
	u8	padding3[2];						// paddingdata

} T_DECISION_PARAMETER_UV;

typedef struct							// 判定パラメタ　NEOMAG
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u8 step;								// 探索間隔
	u8	param_dt_num;						// 構造体の要素数
	u32	param_dt_ofs;						// 構造体の先頭オフセット値
	u8 split_mag_thr;						// 分割判定MAG閾値
	u8 stain_ir1_thr;						// 分割判定IR1閾値
	u8 stain_ir2_thr;						// 分割判定IR2閾値
	u8 split_point_thr;					// MAGなし範囲個数
	u16 stain_raito_thr;					// シミ面積割合
	u8	yobi[2];							// 

} T_DECISION_PARAMETER_NEOMAG;

typedef struct							// 判定パラメタ　磁気スレッド
{
#ifdef NEW_THREAD
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;	// 参照入力結果連番
	u8 x_step;				// x方向探索間隔
	u8 y_step;				// y方向探索間隔
	u8 y_margin;			// y方向マージン
	u8 restart_margin;		// 再探索マージン
	u8 standard_lack_rate;	// 基準レベル：欠損率
	u8 min_lack_rate;		// 基準レベル：最小欠損率
	u8 search_tir_thr;		// TIR探索閾値
	u8 t_plane;				// 使用プレーン
	u8 search_wid;			// 探索幅
	u8 search_lack_thr;		// 欠損探索閾値
	u8 padding;				// 
	u8 padding1[3];			// パディングデータ
	
	s16 x_start;			// 始点X
	s16 x_end;				// 終点X
	s16 padding3;			// 
	u8	padding2[2];		// 

	float outlier_count;	// スレッド座標外れ値個数上限
	float outlier_dev;		// スレッド座標外れ値閾値
	float min_length;		// スレッドとみなす最低長さ
	float mag_dev_thr;		// MAG標準偏差閾値

#else

	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;	// 参照入力結果連番
	u8 x_step;				// x方向探索間隔
	u8 y_step;				// y方向探索間隔
	u8 y_margin;			// y方向マージン
	u8 restart_margin;		// 再探索マージン
	u8 standard_lack_rate;	// 基準レベル：欠損率
	u8 min_lack_rate;		// 基準レベル：最小欠損率
	u8 search_tir_thr;		// TIR探索閾値
	u8 search_mag_thr;		// MAG探索閾値
	u8 search_diff_thr;		// 差分探索閾値
	u8 search_lack_thr;		// 欠損探索閾値
	u8 magnectic_check_dt;	// 反射IRチェック
	u8 padding1[3];			// パディングデータ
	
	s16 x_start;			// 始点X
	s16 x_end;				// 終点X
	s16 diff_plane;			// 差分使用プレーン

	u8	padding2[2];		// 

	float left_dev_thr;		// スレッド左標準偏差
	float right_dev_thr;	// スレッド右標準偏差
	float tir_ave_thr;		// TIR平均値閾値
	float mag_dev_thr;		// MAG標準偏差閾値

#endif
} T_DECISION_PARAMETER_MAGTHREAD;

typedef struct							// 判定パラメタ　金属スレッド
{
#ifdef NEW_THREAD
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;	// 参照入力結果連番
	u8 x_step;				// x方向探索間隔
	u8 y_step;				// y方向探索間隔
	u8 y_margin;			// y方向マージン
	u8 restart_margin;		// 再探索マージン
	u8 standard_lack_rate;	// 基準レベル：欠損率
	u8 min_lack_rate;		// 基準レベル：最小欠損率
	u8 search_tir_thr;		// TIR探索閾値
	u8 search_wid;			// 探索幅
	u8 search_lack_thr;		// 欠損探索閾値
	u8 t_plane;				// 使用プレーン

	s16 x_start;			// 始点X
	s16 x_end;				// 終点X
	s16 padding3;			// 
	u8 padding2[2];			//

	float min_length;		// TIR平均値閾値
	float outlier_count;	// スレッド座標外れ値個数上限
	float outlier_dev;		// スレッド座標外れ値閾値
#else

	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;	// 参照入力結果連番
	u8 x_step;				// x方向探索間隔
	u8 y_step;				// y方向探索間隔
	u8 y_margin;			// y方向マージン
	u8 restart_margin;		// 再探索マージン
	u8 standard_lack_rate;	// 基準レベル：欠損率
	u8 min_lack_rate;		// 基準レベル：最小欠損率
	u8 search_tir_thr;		// TIR探索閾値
	u8 search_diff_thr;		// 差分探索閾値
	u8 search_lack_thr;		// 欠損探索閾値
	u8 padding;				// パディングデータ

	s16 x_start;			// 始点X
	s16 x_end;				// 終点X
	s16 diff_plane;			// 差分使用プレーン
	u8 padding2[2];

	float tir_ave_thr;		// TIR平均値閾値
	float left_dev_thr;		// スレッド左標準偏差
	float right_dev_thr;	// スレッド右標準偏差
#endif

} T_DECISION_PARAMETER_METALTHREAD;

typedef struct							// 判定パラメタ　
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;	//	参照入力結果連番
	u8	search_line_step;	//	ホログラム探索間隔
	u8	param_dt_num;		//	構造体要素数
	u32	param_dt_ofs;		//	構造体の先頭オフセット値

	u8 search_area_tirthr;	//	面積探索時閾値
	u8 restert;				//	再探索マージン
	
	u8 search_area_step;	//	面積探索間隔
	s8 x_margin;			//	x軸方向マージン
	s8 y_margin;			//	y軸方向マージン
	u8 padding[3];			//	パディングデータ

} T_DECISION_PARAMETER_HOLOGRAM;

typedef struct							// 判定パラメタ　重券検知（メカ厚）
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;			//	参照入力結果連番
	u16 double_check_threshold;		// 重券検出厚み閾値
	u16 double_area_ratio;			// 面積比率閾値(%)
	u16 bill_check_threshold;		// 紙幣検出厚み閾値
	u16 exclude_length;				// 先頭判定除外範囲(mm)
	u16 reserve1;
	u16 reserve2;
	u16 reserve3;

} T_DECISION_PARAMETER_DBL_CHK_MECHA;

typedef struct							// 判定パラメタ　特殊A
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項

	u16 reference_blk_num;	//	参照入力結果連番
	u8 total_spa_thr;		//	特殊Aセンサ値合計閾値
	u8 param_dt_num;		//	構造体要素数
	u32	param_dt_ofs;		//	構造体の先頭オフセット値
	s16 x_dis_thr;			//	x軸方向距離閾値
	s16 y_dis_thr;			//	y軸方向距離閾値
	s16 dis_thr;			//	距離閾値
	u8 padding[2];			//
	s16 yobi1;
	s16 yobi2;
	s16 yobi3;
	s16 yobi4;

} T_DECISION_PARAMETER_SPECIAL_A;

typedef struct							// 判定パラメタ　特殊B
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項

	u16 reference_blk_num;	//	参照入力結果連番
	u8 margin;				//	y座標マージン
	s8 padding;				//	パディングデータ
	s16	x_area[2];			//	除外範囲x座標
	float coef1;			//	全体平均係数
	float coef2;			//	全体標準偏差係数
	float intercept;		//	切片
	float thr;				//	特殊B閾値
	s16 yobi1;				//　予備1
	s16 yobi2;				//	予備2

} T_DECISION_PARAMETER_SPECIAL_B;

typedef struct				// 判定パラメタ　CF-NN 1 Color
{
	T_DECISION_PARAMETER_COMMON	comn;// 共通項

	u16 reference_blk_num1;	  // 参照入力結果連番
	u16 reference_blk_num2;	  // 参照入力結果連番
	u16	input_note_num;		  // 入力層ノード数
	u16	hidden_note_num;	  // 中間層ノード数
	u8	output_note_num;	  // 出力層ノード数
	u8	layer_num;			  // レイヤー番号
	u8	padding1[10];		  // 
	u32	hidden_weight_offset; // 中間層の重み配列のオフセット
	u16	hidden_weight_num;    // 中間層の重み配列の要素数
	u16	padding2;			  // 
	u32	output_weight_offset; // 中間層の重み配列のオフセット
	u16	output_weight_num;	  // 中間層の重み配列の要素数
	u16	padding3;			  // padding

} T_DECISION_PARAMETER_CF_NN_1COLOR;

typedef struct				// 判定パラメタ　CF-NN 2 Color
{
	T_DECISION_PARAMETER_COMMON	comn;// 共通項

	u16 reference_blk_num1;	    // 参照入力結果連番
	u16 reference_blk_num2;	    // 参照入力結果連番
	u16	input_note_num;		    // 入力層ノード数
	u16	hidden_note_num;	    // 中間層ノード数
	u8	output_note_num;	    // 出力層ノード数
	u8	layer_num;			    // レイヤー番号
	u8	padding1[10];		    // 
	u32	hidden_weight_offset;   // 中間層の重み配列のオフセット
	u16	hidden_weight_num;		// 中間層の重み配列の要素数
	u16	padding2;			    // 
	u32	output_weight_offset;   // 中間層の重み配列のオフセット
	u16	output_weight_num;		// 中間層の重み配列の要素数
	u16	padding3;			    // padding

} T_DECISION_PARAMETER_CF_NN_2COLOR;



//************************************************************

//***********************************************************
//フィットネス関係
typedef struct							// 判定パラメタ　サイズチェックパラメタ　	
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u16	x_mini;								// 長手方向サイズ　最小値　単位は0.1ｍｍ
	u16	x_max;								// 長手方向サイズ　最大値
	u16	y_mini;								// 短手方向サイズ　最小値
	u16	y_max;								// 短手方向サイズ　最大値
	u8	padding[2];							// padding
} T_DECISION_PARAMETER_SIZ;

typedef struct								// 判定パラメタ　角折れ検知
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u8 comp_flg;							//どちらの基準で比較するか	1：両辺のながさ　2：面積＋短辺
	u8 garbage_or_note_range;		//ポイント採取の際ぶつかったものが媒体かごみかどうかを調べる範囲
	u32 threshold_area;					//面積の基準
	float threshold_short_side_dot;		//短手の基準　単位dot
	float threshold_long_side_dot;		//長手の基準　単位dot
	u8 scan_move_y;					//頂点を求めるときのy座標の移動量
	u8 scan_move_x;					//ｘ座標が戻る量
	u8 scan_move_x_thr_line;		//頂点＋この数字で閾値
	u8 scan_minimum_points;			//このポイント以下ならば角折れなし
	s8 tear_scan_renge;				//破れ検知の際、どれだけ探索すれば破れと判断するか
	u8 tear_scan_start_x;			//裂け検知の際、斜辺を何分割するか
	u8 plane;						//用いるプレーン
	u8 padding;						//パディングデータ
	u8 histogram_qp_param;			//量子化パラメタ
	u8 histogram_peak_width;		//ピーク階層の幅
	u8 histogram_2nd_peak_thr;		//第二ピークの閾値係数
	u8 not_seen_area_count;			//見ないエリアの数
	u32	param_dt_ofs;				// 見ないエリアの座標配列の先頭オフセット値
	u16	param_dt_num;				// 見ないエリアの座標配列の要素数
	u8	padding_2[2];				//パディングデータa


} ST_T_DOG_EAR;


typedef struct								// 判定パラメタ　裂け検知
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num1;					// 参照入力結果連番1
	u16 reference_blk_num2;					// 参照入力結果連番2

	u8	plane;							//用いるプレーン
	u8	width_detection;				//幅検知を実行するか否か　１：実行　０：実行しない
	u8	total_depth_judge;				//深さの合計値で判定する　１：実行　０：実行しない
	u8  padding;

	float threshold_width;				//検知すべき裂けの幅　		　単位㎜で指定する。
	float threshold_vertical_depth;		//検知すべき裂けの深さ(垂直)　単位㎜で指定する。
	float threshold_horizontal_depth;	//検知すべき裂けの深さ(水平)　単位㎜で指定する。
	float threshold_diagonal_depth;		//検知すべき裂けの深さ(斜め)　単位㎜で指定する。

	//u32	param_dt_ofs;				// 見ないエリアの座標配列の先頭オフセット値
	//u8	param_dt_num;				// 見ないエリアの座標配列の要素数
	//u8  not_seen_area_count;		//見ないエリアの数
	//u8	padding_2[2];				//パディングデータa

} ST_T_TEAR;

typedef struct								// 判定パラメタ 穴検知
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項

	u16 reference_blk_num;				//参照入力結果連番
	u8	threshold;						//閾値
	u8	t_plane;						//使用する透過プレーン番号
	u16 threshold_level_area;			//穴面積基準 mm^2
	u8	edge_margin;					//外形マージン　mm
	u8	exclude_area_count;				//除外範囲個数
	u16	threshold_total_area;			//合計検出面積基準 mm^2
	u16	threshold_min_area;				//最小検出面積基準 mm^2
	u32	exclude_areas_ofs;				//除外範囲配列へのオフセット
	float	x_step;						//主走査方向探索間隔 mm
	float	y_step;						//副走査方向探索間隔 mm
	u8 skip;							//輪郭検知探索間隔 dot
	u8	padding2[3];					//

} ST_T_HOLE;


typedef struct							// 判定パラメタ ダイノート
{
	T_DECISION_PARAMETER_COMMON	comn;	// 共通項
	u16 reference_blk_num;				// 参照入力結果連番1
	u8	ink_area_thr;					//インク面積の閾値	
	u8	scan_interval;					//スキャン間隔
	u8	not_scan_area;					//外周の使用不可エリアの範囲
	u8	padding[3];						//
	float old_ink_thr;					//旧インクの閾値
	float new_ink_thr;					//新インクの閾値

} ST_T_DYENOTE;


typedef struct							// 判定パラメタ 汚れ
{
	T_DECISION_PARAMETER_COMMON	comn;	// 共通項
	u32 reference_area_inf_address;		//参照エリア情報構造体のオフセット
	//float fit_normal_vector_distance;	//fit券群と平面との距離	
	//float uf_normal_vector_distance;	//uf券群と平面との距離
	u8	referemce_area_count;			//参照エリア数
	u8	comparison_method;				//uf決定方法
	u8	select_color_mode;				//色空間モデルの設定 0：RGB　1：HSV
	u8	cal_mode;						//計算の設定 1:加重平均 その他：通常平均

} ST_T_SOILING;

typedef struct								// 判定パラメタ 脱色
{
	T_DECISION_PARAMETER_COMMON	comn;	// 共通項
	u32 reference_area_inf_address;		//参照エリア情報構造体のオフセット
	//float fit_normal_vector_distance;	//fit券群と平面との距離
	//float uf_normal_vector_distance;	//uf券群と平面との距離
	u8	referemce_area_count;			//参照エリア数
	u8	comparison_method;				//uf決定方法
	u8	select_color_mode;				//色空間モデルの設定 0：RGB　1：HSV
	u8	padding;						//

} ST_T_DEINK;

typedef struct								// 判定パラメタ 染み
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;			// 参照入力結果連番1
	u8	plane_omote;				//用いる表面のプレーン番号
	u8	plane_ura;					//用いる裏面のプレーン番号
	u8	conter_not_scan_range;		//外周からこの値はスキャン対象外　単位㎜
	u8	thr_lower;					//閾値の下限
	u8	comp_flg;					//基準比較方法
	u8	repeat_num;					//再スキャン回数
	s8	thr_adjust_val;				//白紙と染みを区別する閾値を調節する値
	u8	x_split;					//分割数ｘ
	u8	y_split;					//分割数ｙ
	u8	padding;					//パディング

	float stain_size_thr;			//1つの染みの面積の閾値 単位 mm^2
	float stain_diameter_thr;		//染みの直径の閾値 単位 mm
	float total_stain_thr;			//染みの合計面積の閾値 単位 mm^2

	u16	mask_size_x;				//マスクサイズｘ
	u16	mask_size_y;				//マスクサイズｙ
	float divide_val;				//乗数

	u32	mask_ofs;					// マスクパターンの配列先頭オフセット
	u32	mask_num;					// マスクパターンの要素数

	u32	omote_thr_ofs;				// 表面閾値配列の先頭オフセット
	u32	omote_thr_num;				// 表面閾値配列の要素数

	u32	ura_thr_ofs;				// 裏面閾値配列の先頭オフセット
	u32 ura_thr_num;				// 裏面閾値配列の要素数


} ST_T_STAIN;

typedef struct							// 判定パラメタ　折り畳み
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;					// 参照入力結果連番
	u8	padding1[2];						// パディング

	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u16 tir1_mask_ptn_diameter_x;			// TIR1マスクパターンの直径x
	u16 tir1_mask_ptn_diameter_y;			// TIR1マスクパターンの直径y
	u8	padding2[2];						// パディング

	float tir1_mask_ptn_divide_num;			// TIR1マスクパターンの割る数
	u32	tir1_mask_ptn_ofs;					// TIR1マスクパターンの配列オフセット
	u16	tir1_mask_ptn_num;					// TIR1マスクパターンの配列要素数
	u8	padding3[2];						// パディング

} ST_T_FOLDING;

typedef struct						//メカ式テープ検知
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u32 reference_mapdata_address;	// 全面の基準データ配列への先頭オフセット
	u16 data_size;					// 全面の基準データの要素数
	u8 bill_size_x;					// 紙幣サイズX
	u8 bill_size_y;					// 紙幣サイズY
	u8 mesh_size_x;					// メッシュサイズX
	u8 mesh_size_y;					// メッシュサイズY

	u16 base_value;					// 紙幣厚み最小値
	u16 judge_count;				// 判定終了ポイント数
	u16	threshold;					// 閾値補助値（LSB 0.01）
	u8 threshold_type;				// 閾値種別

	u8 moving_average;				// 移動平均（する／しない）
	u8 moving_ave_value_a;			// 移動平均A値
	u8 divide_ab;					// 分割A-B（する／しない）
	u16 divide_line_ab;				// 分割ラインA-B値
	u8 moving_ave_value_b;			// 移動平均B値
	u8 divide_bc;					// 分割B-C（する／しない）
	u16 divide_line_bc;				// 分割ラインB-C値
	u8 moving_ave_value_c;			// 移動平均C値

	u8 exclude_mesh_top;			// 除外範囲　先端　メッシュ数
	u8 exclude_mesh_bottom;			// 除外範囲　後端　メッシュ数
	u8 exclude_mesh_left;			// 除外範囲　左端　メッシュ数
	u8 exclude_mesh_right;			// 除外範囲　右端　メッシュ数

	u8 continuous_judge;			// 連続判定閾値

	u8 tc1_tc2_corr;				// TC1-TC2の補正フラグ
	u8 black_corr;					// 黒補正フラグ
	u8 padding[2];					// パディング

} ST_T_TAPE;

typedef struct						//静電テープ検知　28バイト+不定サイズパラメタ
{

	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16 reference_blk_num;		// 参照入力結果連番
	u8  black_correction_s;		//黒補正計算ライン開始
	u8	black_correction_e;		//黒補正計算終了
	float tape_judg_thr;		//テープ判定閾値レベル
	u16 x_interval;				//x座標の間引きの間隔
	u16 y_interval;				//y座標の間引きの間隔
	u8  reference_area_num;		//参照エリア数
	u8  padding[3];				//よび
	u32 reference_area_offset;	//参照エリア構造体のオフセット
	u32 first_thrs_num;			//第一閾値配列の要素数
	u32 first_thrs_offset;		//第一閾値配列のオフセット

} ST_T_CAP_TAPE;

//***********************************************************
//OCR
typedef struct								// 判定パラメタ　OCR
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	s16 starting_x;							//左上の座標
	s16 starting_y;							//
	u16 width;								//エリアの幅
	u16 height;								//エリアの高さ
	u8	side;								//用いる色の面 U or 
	u8  color;								//用いる色 G or R
	u8  series_length;						//シリアルの文字数
	u8  padding1;
	u32 serise_ary_offset;					//シリアルタイプ配列までのオフセット
	u16 switch_binarization_value;			//平均画素値を適応的に切り替える為の閾値
	u8  binarization_threshold;				//文字と背景を分ける二値化の閾値
	u8  threshold_on_x;						//X軸の
	u8  threshold_on_y;						//Y軸の
	u8  padding2;
	u16 threshold_on_offset;				//検出できた上下のオフセットを削除するための値
	u8	flip_x;								//x軸を基に反転する場合は１　しないなら0
	u8	flip_y;								//y軸を基に反転する場合は１　しないなら0
	u8	rotate;								//切り出したエリアを回転させる
	u8  padding3;
	u16	dpi;								//用いるプレーンのdpi
	u8  model_num;							//モデル番号
	u8	series_area_num;					//シリアルの位置　１と２
	u8  cutout_size;
	u8  padding4[3];						//

	u32 composite_weight_data_offset;		//複合判定ウェイトデータまでのオフセット
	u32 composite_weight_data_num;			//複合判定ウェイトデータの要素数
	u32 composite_label_data_offset;		//複合判定のラベルデータまでのオフセット
	u16 composite_label_data_num;			//複合判定のラベルデータの要素数
	u8	padding5[2];						//
	u32 numbers_weight_data_offset;			//数字判定ウェイトデータまでのオフセット
	u32 numbers_weight_data_num;			//数字判定ウェイトデータの要素数
	u32 numbers_label_data_offset;			//数字判定のラベルデータまでのオフセット
	u16 numbers_label_data_num;				//数字判定のラベルデータの要素数
	u8	padding6[2];						//
	u32 letter_weight_data_offset;			//文字判定ウェイトデータまでのオフセット
	u32 letter_weight_data_num;				//文字判定ウェイトデータの要素数
	u32 letter_label_data_offset;			//文字判定のラベルデータまでのオフセット
	u16 letter_label_data_num;				//文字判定のラベルデータの要素数
	u8	padding7[2];						//

} ST_T_OCR;

typedef struct							// 判定パラメタ　
{
	T_DECISION_PARAMETER_COMMON	comn;		// 共通項
	u16	read_type;					//読み取り種別
	u8	character_num_limits_min;	//読み取り制限　最小
	u8  character_num_limits_max;	//				最大
	u8	execute_flg;				//実行フラグ　１：許可　０：禁止
	u8	input_scan_side;			//スキャンする面　2:両面 0:表のみ　1：裏のみ
	u8	itf_chk_digit_mode;			//チェックデジットの有無	0：無　1：有
	u8	padding;					//パディング
	float itf_thin_bar_width_min;	//細バーの幅　最小
	float itf_thin_bar_width_max;	//細バーの幅　最大
	float itf_width_of_bar_ratio_min;	//比率　最小
	float itf_width_of_bar_ratio_max;	//比率　最大
	u32	itf_scan_range_y;			//スキャン範囲y　中心基準の単位㎜
	u32	itf_scan_interval_y;		//スキャン間隔を設定する　単位はdot	
	s32	start_scan_x;				//スキャン開始位置x　中心を0とした座標系　単位は㎜
	s32	start_scan_y;				//スキャン開始位置y

} T_DECISION_PARAMETER_BARCODE;

typedef struct
{
	T_DECISION_PARAMETER_COMMON	comn;	// 共通項


}  T_DECISION_PARAMETER_GRAFFITI_FACE;

typedef struct
{
	T_DECISION_PARAMETER_COMMON	comn;	// 共通項


}  T_DECISION_PARAMETER_GRAFFITI_ABNORMAL;

typedef struct
{
	T_DECISION_PARAMETER_COMMON	comn;	// 共通項

	u8 plane;			//使用プレーン1
	u8 padding2;
	s16 start_x;		//OCR範囲開始x2
	s16 start_y;		//OCR範囲開始y2
	s16 end_x;			//OCR範囲終了x2
	s16 end_y;			//OCR範囲終了y2

	// 裏面
	u8 plane_1;			//使用プレーン1
	u8 padding_1;
	s16 start_x_1;		//OCR範囲開始x2
	s16 start_y_1;		//OCR範囲開始y2
	s16 end_x_1;			//OCR範囲終了x2
	s16 end_y_1;			//OCR範囲終了y2

	// Huong add 20230111
	u32 n_input;		//特徴量
	u32 n_hidden;		//
	u32 n_output;		//
	u32 activation_hidden;		//
	u32 activation_output;		//
	u32 offset_Hidden;		//
	u32 offset_Bias_Hidden;		//
	u32 offset_Output;		//
	u32 offset_Bias_Output;		//
	u32 offset_label;		//


} T_DECISION_PARAMETER_GRAFFITI_TEXT;

typedef struct
{
	T_DECISION_PARAMETER_COMMON	comn;	// 共通項

	u16 reference_blk_num;					// 参照入力結果連番
	u16	param_dt_num;						// 各モードパラメタ構造体の要素数
	u32	param_dt_ofs;						// 各モードパラメタ構造体の先頭オフセット値
	

}  T_DECISION_PARAMETER_CUSTOMCHECK;


////////////////////////////////////////////////////////////////////////////////////////
// 判定結果ブロック

//************************************************************
//前処理部　識別部
typedef struct							// 判定結果ブロック　外形検知	
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	s16	note_x_length;					// 主走査方向札幅
	s16	note_y_length;					// 搬送方向札幅
	s16	skew_val;						// スキュー角
	s16	center_x;						// 中心座標ｘ
	s16	center_y;						// 中心座標ｙ
	s16	vertex_top_left_x;				// 頂点座標　論理座標 top left x
	s16	vertex_top_left_y;				// 頂点座標　論理座標 top left y
	s16	vertex_top_right_x;				// 頂点座標　論理座標 top right x
	s16	vertex_top_right_y;				// 頂点座標　論理座標 top right y
	s16	vertex_bottom_left_x;			// 頂点座標　論理座標 bottom left x
	s16	vertex_bottom_left_y;			// 頂点座標　論理座標 bottom left y
	s16	vertex_bottom_right_x;			// 頂点座標　論理座標 bottom right x
	s16	vertex_bottom_right_y;			// 頂点座標　論理座標 bottom right y
	u8	padding[2];						// padding
} T_OUTLINE_RESULT;

typedef struct							// 判定結果ブロック　ポイントデータ抽出
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u32	extract_point_dt_ofs;			// 抽出ポイントデータの先頭オフセット値
	s16	extract_point_dt_num;			// 抽出ポイントデータの要素数
	u8	ei_corner;						// コーナー無効に　する:0/しない:1
	u8	plane_count;					// プレーン数
	u8	plane_lst[PLANE_LST_MAX];		// プレーンリスト 
} T_EXTRACT_POINT_DT;

typedef struct
{
	T_DECISION_RESULT_COMMON	comn;		// 共通項
	u16	result_1st_node;					// 第一発火ノード
	u16	result_2nd_node;					// 第二発火ノード
	float max_node_val;						// 最大発火値
	float diff;								// 最大と第二の差
	float softmax;							// ソフトマックス値
	float error;							// エラー値

	u32 output_node_ofs;					//	出力ノード先頭オフセット
	u16 output_node_num;					//	出力ノード要素数
	u16	res_code;							//  リザルトコード

} T_NN_RESULT;

typedef struct								//外形サイズチェック
{
	T_DECISION_RESULT_COMMON	comn;		// 共通項
	u8 hazure;								// はずれレベル
	u8 padding[3];							// パディングデータ
} T_SIZ_RESULT;


typedef struct								//外形サイズ識別
{
	T_DECISION_RESULT_COMMON	comn;		// 共通項
	u8 hazure;								// はずれレベル
	u8 padding[3];							// パディングデータ
} T_SIZ_IDENT_RESULT;


//************************************************************
//鑑別関係
typedef struct							// 結果ブロック　3色比
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16 invalid_data_count;				// 無効なポイントの数
	u8  level;							//レベル
	u8  padding;						//パディング

} T_CIR3_RESULT;

typedef struct							// 結果ブロック　4色比
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16 invalid_data_count;				// 無効なポイントの数
	u8  level;							//レベル
	u8  padding;						//パディング

} T_CIR4_RESULT;

typedef struct							// 結果ブロック　IRチェック
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16 invalid_data_count;				// 無効なポイントの数
	u8  level;							//レベル
	u8  padding;						//パディング

} T_IR_CHECK_RESULT;

typedef struct							// 結果ブロック　MCIR
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16 invalid_data_count;				// 無効なポイントの数
	u8  level;							//レベル
	u8  padding;						//パディング

} T_MCIR_RESULT;

typedef struct							// 結果ブロック　重券
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	s32 invalid_data_count;				// 無効なポイントの数
	u8  level;							//レベル
	u8  padding[3];						//パディング
	float all_prob;						//全体紙幣の重券確率（重券レベル)
	float each_prob[4];					//4部分の重券確率

} T_DOUBLE_CHECK_RESULT;

typedef struct							// 結果ブロック　NN1
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8	classified_res;					// 分類結果
	u8	level;							//レベル
	u8	padding[2];						//
	float genuine_out_put_val;			//真券の発火値
	float counterfeit_out_put_val;		//偽券の発火値

} T_NN1_RESULT;

typedef struct							// 結果ブロック　NN2
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8	classified_res;					// 分類結果
	u8	level;							//レベル
	u8	padding[2];						//
	float genuine_out_put_val;			//真券の発火値
	float counterfeit_out_put_val;		//偽券の発火値

} T_NN2_RESULT;

typedef struct							// 結果ブロック　2色差
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u32 invalid_data_count;				// 無効なポイントの数

} T_CIR2_RESULT;

typedef struct							// 結果ブロック　IR2波長
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	//u32 ir1_peak_res[IMUF_POINT_NUMBER];				// Peak差 IR1
	//u32 ir2_peak_res[IMUF_POINT_NUMBER];				// Peak差 IR2

	u8 output[IMUF_POINT_NUMBER];
	u8 feature[IMUF_POINT_NUMBER];
	
	u8 result;									// 判定結果（IR2色あり：１、IR2色なし：０
	u8 level;
	u8 padding[2];				// パディング

} T_IR2WAVE_RESULT;

typedef struct							// 結果ブロック　磁気
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	float mag_ave;		// 磁気平均値
	float percent;		//磁気過多量
	u8 ave_num;
	u8 per_num;
	u8 result;							// 判定結果（磁気あり：１、磁気なし：０）
	u8 level;							// レベル


} T_MAG_RESULT;

typedef struct							// 結果ブロック　蛍光
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	float uv_ave[IMUF_POINT_NUMBER];				// 蛍光平均値
	u8 result;							// 判定結果（UVあり：１、UVなし：０）
	u8 level;							// レベル
	u8 padding[2];				// パディング

} T_UV_RESULT;

typedef struct							// 結果ブロック　NEOMAG
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	float mag_dev[NEOMAG_MAX_COUNT];	// MAG標準偏差
	float ir1_ave[NEOMAG_MAX_COUNT];	// IR1平均値
	float ir2_ave[NEOMAG_MAX_COUNT];	// IR2平均値
	u16 split_point[NEOMAG_MAX_COUNT];	// MAGなし範囲数
	s8 result[NEOMAG_MAX_COUNT];		// 結果
	s8 judge;							// 判定
	s16 err_code;						// エラーコード
	u8 yobi[2];							// 予備

} T_NEOMAG_RESULT;

typedef struct							// 結果ブロック　磁気スレッド
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	float tir_z_score;	//No.1 4
	float tir_total_ave;//No.2 4
	float tir_total_dev;//No.3 4
	float mag_total_ave;//No.4 4
	float mag_total_dev;//No.5 4
	float res_mag_max;//No.6 4
	u8 remain_percent;//No.7 1
	u8 thread_num;//No.8 1
	s16 thread_center;//No.9 2
	u16 mag_max_count;//No.10 2
	u16 tir_count;//No.11 2
	u8 level;//No.12 1
	s8 judge;//No.13 1
	s16 err_code;//No.14 2
	
} T_MAGTHREAD_RESULT;

typedef struct							// 結果ブロック　メタルスレッド
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	float tir_z_score;//No.1 4
	float tir_total_ave;//No.2 4
	float tir_total_dev;//No.3 4
	u8 remain_percent;//No.4 4
	u8 thread_num;//No.5 4
	s16 thread_center;//No.6 4
	u16 tir_count;//No.7 4
	u8 level;//No.8 4
	s8 judge;//No.9 4
	s16 err_code;//No.10 4
	u16 padding;

} T_METALTHREAD_RESULT;

typedef struct							// 結果ブロック　ホログラム
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	float tir_ave[HOLO_NUM_LIMIT];	//TIR平均値
	u16	area[HOLO_NUM_LIMIT];
	u16	heid[HOLO_NUM_LIMIT];
	u16 wid[HOLO_NUM_LIMIT];
	s16 result[HOLO_NUM_LIMIT];
	s16 err_code;
	s16 judge;
	u8 level;
	u8 padding[3];

} T_HOLOGRAM_RESULT;

typedef struct							// 結果ブロック　重券検知（メカ厚）
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8  result;					// 判定結果
	u8  double_check_ratio;		// 重券面積比率
	u16 double_check_count;		// 重券判定箇所ポイント数
	u16 bill_check_count;		// 紙幣判定箇所ポイント数
	u16 reserve;				// 予備
} T_DBL_CHK_MECHA_RESULT;

typedef struct							// 結果ブロック	特殊A
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16 sum[SPA_NUM_LIMIT];				//合計値
	u8 count[SPA_NUM_LIMIT];			//検知数
	s8 judge;							//判定
	u16 total_sum;						//全体合計値
	u16 yobi1;
	s16 yobi2;
	u8 padding[2];

} T_SPECIAL_A_RESULT;

typedef struct							// 結果ブロック	特殊B
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	float total_ave;		//合計値
	float total_dev;		//検知数
	float predict;			//判定
	s16 judge;				//全体合計値
	s16 yobi1;				//予備1
	s16 yobi2;				//予備2
	s16 yobi3;				//予備3

} T_SPECIAL_B_RESULT;

typedef struct							// 判定結果ブロック　CFNN1
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u32	extract_point_dt_ofs;			// 抽出ポイントデータの先頭オフセット値
	s16	extract_point_dt_num;			// 抽出ポイントデータの要素数
	u8	result;							// 判定結果 0：真	1：偽
	u8	do_flg;							// 実行判定	0：未実行　1：実行
	float genuine_value;				// 真券発火値
	float counterfeit_value;			// 偽造券発火値
	u32 layer;							// 現在のレイヤー 1からカウントする

} T_EXTRACT_CF_NN_1COLOR;

typedef struct							// 判定結果ブロック　CFNN2
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u32	extract_point_dt_ofs;			// 抽出ポイントデータの先頭オフセット値
	s16	extract_point_dt_num;			// 抽出ポイントデータの要素数
	u8	result;							// 判定結果 0：真	1：偽
	u8	do_flg;							// 実行判定	0：未実行　1：実行
	float genuine_value;				// 真券発火値
	float counterfeit_value;			// 偽造券発火値
	u32 layer;							// 現在のレイヤー 1からカウントする

} T_EXTRACT_CF_NN_2COLOR;


typedef struct							// 判定結果ブロック：カスタムチェック
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項

	u16 judge;		//CF判定されたエリアの判定　16進
	u8 result_num;	//CF判定された判定番号
	u8 result_area;	//CF判定されたエリア番号
	u8 sum_num;		//全エリアでのCF判定数
	u8 sum_area;	//CF判定されたエリア数
	u8 level;		//出力レベル
	u8 padding;		//パディング
	s32 value;		//計算結果

}T_CUSTOMCHECK_RESULT;

//************************************************************
//フィットネス関係
typedef struct							//角折れ
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項

	u32	area[4];		//角折れの面積
	u32	area_mm[4];		//角折れの面積 単位mm

	float len_x[4];		//x辺の長さ
	float len_y[4];		//y辺の長さ

	float len_x_mm[4];	//辺の長さ　単位mm
	float len_y_mm[4];	//辺の長さ　単位mm

	float short_side[4];//角折れの長手
	float long_side[4];	//角折れの短手

	float short_side_mm[4];//角折れの長手
	float long_side_mm[4];	//角折れの短手

	s16 triangle_vertex_x[4][2];	//角折れ三角形の頂点ｘ座標を記録する
									//基準緒店からｘが変化している頂点から記録する
	s16 triangle_vertex_y[4][2];	//角折れ三角形の頂点ｙ座標を記録する

	u8	judge[4];			//各頂点の判定結果
	u16 judge_reason[4];	//結果の理由

	u8  level;	
	u8  padding[3];


} T_DOG_EAR_RESULT;

#define LIMIT_TEAR_COUNT		20		// 結果を格納する配列の要素数
typedef struct							//裂け検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項

	float res_tear_width[LIMIT_TEAR_COUNT];			//検知した裂けの幅
	float res_tear_depth[LIMIT_TEAR_COUNT];			//検知した裂けの深さ
	float res_tear_total_depth;						//検知した裂けの深さの合計
	u8 res_tear_type[LIMIT_TEAR_COUNT];				//検知した裂けの形状
	u8 res_tear_count;								//検知した裂けの数
	u8 res_judge_tear;								//検知した裂けの中で閾値を超えるものがあった。　1：有　0：無
	u16 res_judge_reason;							//検知した裂けの理由　
	u16 res_each_judge_reason[LIMIT_TEAR_COUNT];	//検知した裂けの理由　
	u16 err_code;
	u8  level;
	u8  padding;

} T_TEAR_RESULT;


typedef struct							//穴検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項

	u8	result;					//出力結果
	u8	hole_count;				//穴個数
	u16	total_hole_area;		//合計検出面積 mm^2
	u16	max_hole_area;			//検出最大面積 mm^2
	u8	level;					//レベル
	u8	padding;				//
	s16	holes[HOLE_MAX_COUNT];	//各穴の面積 mm^2
	u16	err_code;				//エラーコード
	u8	padding2[2];			//

} T_HOLE_RESULT;

typedef struct							//ダイノート検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16	err_code;						//エラーコード
	u8 result;							//検知結果
	u8 level;							//
	u32 ink_area;						//インク面積dot
	float ink_rate;						//インク面積割合

} T_DYENOTE_RESULT;

typedef struct							//汚れ検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8 uf_count;						//UFエリアの数
	u8 result;							//検知結果
	u8 level;							//レベル
	u8 padding;
	float each_area_distance[5];		//各エリアの平面との距離

} T_SOILING_RESULT;

typedef struct							//脱色検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8 uf_count;						//UFエリアの数
	u8 result;							//検知結果
	u8 level;							//レベル
	u8 padding;
	float each_area_distance[5];		//各エリアの平面との距離

} T_DEINK_RESULT;

typedef struct							//染み検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16 err_code;						//エラーコード
	u8  result;							//検知結果
	u8  level;							//
	float max_stain_area;				//最大面積　単位mm^2
	float max_stain_diameter;			//最大直径　単位mm^2
	float max_stain_total_area;			//面積の合計　単位mm^2

} T_STAIN_RESULT;

typedef struct							//OCR
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8 series_1[16];		//220->16					//求めた文字列
	u8 series_2[16];							//求めた文字列
	u16 series_num_1;					//求めた文字数
	u16 series_num_2;					//求めた文字数

} T_OCR_RESULT;

typedef struct							// 結果ブロック　重券
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u16 err_code;					// エラーコード
	u8 up_res;						// 上端の折り畳み状態　    1：折りたたまれている、０：折りたたまれていない
	u8 down_res;					// 下端の折り畳み状態　  10：折りたたまれている、０：折りたたまれていない
	u8 left_res;						// 左端の折り畳み状態　  50：折りたたまれている、０：折りたたまれていない
	u8 right_res;					// 右端の折り畳み状態    100：折りたたまれている、０：折りたたまれていない
	u8 level[4];						// レベル
	u8 padding[2];
} T_FOLDING_RESULT;

typedef struct							// テープ検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8  result;							//検知結果
	u8  level;							//
	u16 err_code;						//エラーコード
	u16 detect_num;						// テープ検知数
	u8 reserve[2];
} T_TAPE_RESULT;

typedef struct							// 静電テープ検知
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u8  result;							//検知結果
	u8  level;							//
	u16 err_code;						//エラーコード

} T_CAP_TAPE_RESULT;

typedef struct							// バーコードリード
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項
	u32	itf_res_states;			//ステータス
	u32	itf_res_character_num;	//要素数
	u32	itf_res_ary_offset;		//結果配列オフセット
	s32	itf_res_max_bar;		//検知したバーの最大幅
	u8	itf_res_check_digit;	//チェックデジットの結果
	u8	padding[3];				//パディング

	u16	qr_res_states;			//ステータス
	u16	qr_res_character_num;	//要素数
	u32	qr_res_ary_offset;		//結果配列オフセット
	u8	qr_res_version;			//バージョン
	u8	qr_res_err_cor_level;	//誤り訂正レベル
	u8	qr_res_err_cor_num;		//誤り訂正の数
	u8	qr_res_encoding_mode;	//文字エンコーディングモード

} T_BARCODE_RESULT;

typedef struct
{
	T_DECISION_RESULT_COMMON	comn;	// 共通項

	s16 result;
	s16 code;
	u8 res_num;
	u8 cut_num;
	u8 padding[2];

}T_GRAFFITI_TEXT_RESULT;


//****************************************************
//総合判定
typedef struct
{
	T_DECISION_RESULT_COMMON	comn;		// 共通項
	u32	total_time;							// 総合処理時間
	s16	skew;								//スキュー角
	u8	long_edge_size;						//サイズ（長手方向）
	u8	short_edge_size;					//サイズ（短手方向）
	s8	err_size;							//サイズチェックエラー
	s8	dbl;								//重券
	s8	ir;									//IR鑑別
	s8	uv;									//UV鑑別
	s8	mag;								//MAG鑑別
	s8	thread_mag;							//スレッド鑑別
	s8	thread_metal;						//スレッド鑑別
	s8	gsi_air;							//シクパ、GSI-Air鑑別
	s8	neomag;								//ネオマグ鑑別
	s8	sp_a;								//特殊センサ鑑別
	s8	sp_b;								//特殊センサ鑑別
	s8	de_inked_note;						//ダイノート
	s8	ocr;								//OCR
	s8	balcklist;							//ブラックリスト該当
	u8	article6_category;					//Article6 カテゴリ
	s8	fitness;							//フィットネス結果　切れ
	s8	hole;								//穴
	//s8	long_edge_tear;						//破れ　長辺
	//s8	short_edge_tear;					//破れ短辺
	s8  tear;								//裂け
	s8	dog;								//角折れ
	s8	soil;								//汚れ
	s8	deinkd;								//脱色
	s8	repair;								//補修
	s8	stain;								//inkの汚れ
	s8	graffiti;							//落書き
	s8	long_edge_fold;						//折れ長辺
	s8	short_edge_fold;					//折れ短辺
	s8	cir3;								//
	s8	cir4;								//
	s8	mcir;								//
	s8	ir_ck;								//
	s8	vali_nn1;							//
	s8	vali_nn2;							//
	s8	folding;
	s8	mutilation;
	s8  tape_mecha;
	s8  tape_cap;
	s8	hologram;
	u8 padding[3];


} T_NN_TOTAL_RESULT;

#ifndef _RM_
typedef struct CORR_ID_TABLE
{
	u32 jcm_id;							//JCM券種ID
	u16	template_id;					//テンプレート内券種ID
	u8	template_rev;					//テンプレートのリビジョン
	u8	currency[3];					//通貨コード	ユーロの時は‘EUR’（0x45, 0x55, 0x52）
	u8	index_num;						//インデックス番号
	u8	padding;						//パディング
	u8	denomi_code[32];				//金種コード
	u8	denomi_code_version[32];		//金種コードのバージョン
	u8	emission[64];					//エミッション
	s8	price_exp;						//金額情報　指数部
	u8	price_sign;						//金額情報　仮数部
	u8	note_size_x_mm;					//長手方向の紙幣サイズ　単位は㎜
	u8	note_size_y_mm;					//短手方向の紙幣サイズ　単位は㎜

} ST_CORR_ID_TABLE;
#endif

//
//#ifdef GLOBAL_VALUE_DEFINE
//  #define EXTERN extern
//#else
//  #define EXTERN
//  #define GLOBAL_VALUE_DEFINE
//#endif
//

// グローバル変数宣言
EXTERN void*	template_top_pt;										// 判定ブロックテーブルの先頭ポインタ　（あらかじめセットしておくこと）
EXTERN T_TEMPLATE_WORK	st_work[BUF_NUM];								// テンプレート処理ワークメモリ
EXTERN u8	decision_result_tbl[BUF_NUM][RESULT_TBL_SIZE];				// 判定結果領域 （可変長データが入る）


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// プロトタイプ宣言

//****************************************************
// テンプレートシェル標準関数
void	start_template_sequence(u8 buf_n);									                         //テンプレートシェルのスタート関数
void	restart_template_sequence(u8 buf_n);								                         //
void	deecision_block_sequence_loop(T_TEMPLATE_WORK*	pst_work);	                                 //テンプレートシェルの本体
u32		ID_chk(T_TEMPLATE_WORK*	pst_work);							                                 //IDチェック
u32		get_10ns_co(void);										                                     //タイマー関数
T_POINTER search_blk_address_of_seq_num (u16 seq_num, T_DECISION_RESULT_COMMON* start_address);	     //
void	fnc_jmp(T_TEMPLATE_WORK*	pst_work);					                                     //ジャンプ関数
void	overall_judge_proc(T_TEMPLATE_WORK* pst_work);			                                     //総合判定
u8		level_result_detect(u8 level ,u8 thr1, u8 thr2, u8 thr3);					                 //レベルから判定結果を決定する。
u32		set_fitness_thrs(T_TEMPLATE_WORK*	pst_work, u32 cry_num, u8 state);		                 //テンプレートタスクからもらったフィットネスの閾値情報をサンプリングデータにセットする
u32		templateID_2_noteID(u32 id);															     //
u8      category_decision(T_TEMPLATE_WORK* pst_work);											     //
u32		write_thrlevels_2_samplingdata(ST_BS *pbs ,u32 algo_idx, u8 thr1, u8 thr2, u8 thr3, u8 margin);
u8*		offset_address_cal(u8* base_point, u32 offset);											     //
void	set_each_alog_level(T_TEMPLATE_WORK* pst_work, u8* level_ary);	                             //result_block_reference
s8      change_validation_thr_level(u8 must_impact_uf, u8 algo_idx, u8* thr2 ,u8 on_off);		     //
u8		multi_nn(T_TEMPLATE_WORK* pst_work);			                                             //複合NN判定関数
void	set_dir_params(T_TEMPLATE_WORK* pst_work, s8 flg);	                                         //方向情報の設定と座標変換のパラメータ計算関数
u8		validation_each_final_judge(u8 must_impact_uf, u8 correction_func_index					     //鑑別処理の閾値判定関数
	,u8 result_bit_func_index, u8 res_level,	u8 judge_params1, u8 judge_params2, u8 thr3
	,u8 vali_level_mode																			     //
	,u8* presult, u8* pthr2, u8* pcounter1, u8* pcounter2, u8* pcounter3,u32 *pvalidation_result_flg);//

u8		unfit_each_final_judge(ST_BS* pbs, u8* original_res, u8* overall_res						 //正損処理の閾値判定関数
	,u8 replay_func_index, u8 algo_index, u8 impact_uf_index, u8 judge_params1,	u8 judge_params2	 //
	,u8 vali_index, u8 vali_level_mode,																 //
	u8* pmust_impact_uf, u8* pcf_counter, u8* puf_counter, u8* pfp_counter);						 //
void	judge_counter(u8 judge_params, u8* cf_counter, u8* uf_counter, u8* fp_counter);				 //カテゴリカウンター用関数
void	big_uf_relief(ST_BS* pbs, T_TEMPLATE_WORK* pst_work);										 //損券のカテゴリ2/3救済関数							
s8 template_function_existence_detect(u16 note_id, u16 func_id);									 // パラメータブロックを検索する

//#ifndef _RM_
s8	template_currenct_table_top_detect(T_TEMPLATE_WORK* pst_work);				            //テンプレート内券種IDの先頭アドレスを見つける。
//#endif

void	template_log1(s32 code ,s32 buf_num, s32 proc_num);											//デバッグログ用の関数
void	template_log2(s32 code, s32 buf_num, s32 proc_num);											//引数等はその機種の担当者が自由に設定してよい。


//u8		level_result_detect(u8 level ,u8 thr1, u8 thr2);					//レベルから判定結果を決定する。
//EXTERN	void	jcmID_tbl_proc(T_TEMPLATE_WORK* pst_work);
//EXTERN	void	add_val_blk(void* destp,	void* source, u16 siz, u8 padsiz, T_TEMPLATE_WORK* pst_work);
//EXTERN	void*	add_result_blk(u16	siz, T_TEMPLATE_WORK* pst_work, u16 cnd_code, u16 tmplt_ID);
//EXTERN	u16		calculate_size_including_padding_data(u16 body_siz);

//****************************************************
//前処理関係
u32		ts_otline_det(T_TEMPLATE_WORK* pst_work);				//外形検知
u32		ts_extraction_pointdata(T_TEMPLATE_WORK* pst_work);		//ポイント抽出
void	ts_NN_judge_proc(T_TEMPLATE_WORK* pst_work);			//NN

void denomination_test(T_TEMPLATE_WORK* pst_work);	//新識別

void	ts_oline_size_ident(T_TEMPLATE_WORK* pst_work);			//サイズ識別
void	ts_barcode(T_TEMPLATE_WORK* pst_work);

//****************************************************
//鑑別処理関係
void	ts_oline_size_check(T_TEMPLATE_WORK* pst_work);			//サイズチェック
void	ts_cir_3color(T_TEMPLATE_WORK* pst_work);				//3色比
void	ts_cir_4color(T_TEMPLATE_WORK* pst_work);				//4色比
void	ts_ir_check(T_TEMPLATE_WORK* pst_work);					//IRチェック
void	ts_mcir(T_TEMPLATE_WORK* pst_work);						//MCIR
void	ts_double_check(T_TEMPLATE_WORK* pst_work);				//重券
void	ts_double_check_mecha(T_TEMPLATE_WORK* pst_work);		// 重券検知（メカ厚）
void	ts_double_chk_nn(T_TEMPLATE_WORK* pst_work);				//新光学式重券
void	ts_neural_network_1color(T_TEMPLATE_WORK* pst_work);	//1色NN鑑別
void	ts_neural_network_2color(T_TEMPLATE_WORK* pst_work);	//2色NN鑑別
void	ts_customcheck(T_TEMPLATE_WORK* pst_work);				//カスタムチェック
void	ts_cir_2color(T_TEMPLATE_WORK* pst_work);				//2色差
void	ts_ir2wave(T_TEMPLATE_WORK* pst_work);					// IR2波長
void	ts_mag(T_TEMPLATE_WORK* pst_work);						// MAG鑑別
void	ts_uv_validate(T_TEMPLATE_WORK* pst_work);				// UV鑑別
void	ts_uv_fitness(T_TEMPLATE_WORK* pst_work);				// UV正損 洗濯券
void	ts_special_a(T_TEMPLATE_WORK* pst_work);				// 特殊A
void	ts_special_b(T_TEMPLATE_WORK* pst_work);				// 特殊B
void	ts_cf_nn_1color(T_TEMPLATE_WORK* pst_work);				// CF-NN1Color
void	ts_cf_nn_2color(T_TEMPLATE_WORK* pst_work);				// CF-NN2Color
//****************************************************
//フィットネス関係
void    ts_dog_ear(T_TEMPLATE_WORK* pst_work);					//角折れ検知
void    ts_tear(T_TEMPLATE_WORK* pst_work);						//裂け検知
void    ts_dye_note(T_TEMPLATE_WORK* pst_work);					//ダイノート検知
void    ts_stain(T_TEMPLATE_WORK* pst_work);					//染み検知
void    ts_de_ink(T_TEMPLATE_WORK* pst_work);					//脱色検知
void    ts_soiling(T_TEMPLATE_WORK* pst_work);					//汚れ検知
void	ts_hole(T_TEMPLATE_WORK* pst_work);						//穴検知
void	ts_folding(T_TEMPLATE_WORK* pst_work);					//折り畳み欠損検知
void	ts_tape(T_TEMPLATE_WORK* pst_work);						//テープ検知
void	ts_cap_tape(T_TEMPLATE_WORK* pst_work);					//静電テープ検知
void	ts_neomag(T_TEMPLATE_WORK* pst_work);					//NEOMAG検知
void	ts_magthread(T_TEMPLATE_WORK* pst_work);				//磁気スレッド検知
void	ts_metalthread(T_TEMPLATE_WORK* pst_work);				//金属スレッド検知
void	ts_hologram(T_TEMPLATE_WORK* pst_work);					//ホログラム検知

void	ts_graffiti_text(T_TEMPLATE_WORK* pst_work);			//落書き検知
//****************************************************
//OCR
void	ts_ocr(T_TEMPLATE_WORK* pst_work);						//OCR

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // _TEMPLATE_H_INCLUDED_

