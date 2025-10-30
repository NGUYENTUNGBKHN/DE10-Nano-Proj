#ifndef	_NN_H
#define	_NN_H


#define NN_INPUT_NODE_MAX_NUM 960	//最大入力ノード数　
#define	NN_HIDDEN_NODE_MAX_NUM 360	//最大入力ノード数　

#define ERR_FAILURE_NN_TH1	0x0c0c	//th1が閾値以下
#define ERR_FAILURE_NN_TH2	0x0c0d	//th2が閾値以下	
#define ERR_FAILURE_NN_TH3	0x0c0e	//th3が閾値以下	
#define ERR_FAILURE_NN_TH5	0x0c0f	//th5が閾値以上



typedef struct IDENT_RESULT_STRUCTURE //Identタスク出力
{
	u32 note_id;	//券種ID
	u8 way;			//方向コード
	u8 ident_err_code;	//エラーコード
	u16 reject;		//リジェクトコード
	float output_max_val;	//最大発火値
	float output_2nd_val;	//2nd発火値
	u32 output_max_node;	//最大発火のノード
	u32 output_2nd_node;	//2nd発火のノード
	float th3;				//th3(softmax)の計算結果
	float th5;				//th5(err)の計算結果
	u32 deb_float_sum[10];

	////CF_NN用
	u8 do_flg;				//実行確認フラグ
	u8 layer_num;			//現在のレイヤー番号

} ST_NN_RESULT;

typedef struct 				//NNの出力の発火値を閾値を格納する
{
	float th1;				//最大発火値
	float th2;				//2nd発火値
	float th3;				//th3(softmax)
	float th5;				//th5(err)
} ST_NN_NODE_THRESHOLD;


typedef struct//識別のテンプレート
{
	//u16 ID;						//機能番号
	//u8 outline;					//外形基準
	//u8 planenum;				//色数
	u8 length;					//札の大きさ(搬送)
	u8 width;					//札の大きさ(主走査）
	u8 corner;					//コーナー
	//u8 plane[16];				//色
	//u8 filtersize_x;			//フィルタサイズ
	//u8 filtersize_y;			//フィルタサイズ
	//u16 warunum;				//割る数
	//u32 filter_p_offset;		//フィルタパターン設定先頭位置オフセット
	u8 sizeflag;				//サイズデータを使用する場合 0：使用しない　1：使用する
	u8 biasflag;				//バイアス項 0：使用しない　1：使用する
	u8 do_normalize;			//正規化をスキップする。 0:スキップ　1：実行
	u8 padding[2];

	u16* p_input_rawdata;		//インプットの元データ

	float in_put[NN_INPUT_NODE_MAX_NUM];	//入力データ格納変数
	float hidden1[NN_HIDDEN_NODE_MAX_NUM];	//中間データ格納変数
	float output[NN_HIDDEN_NODE_MAX_NUM];	//中間データ格納変数


	u16 in_node;				//入力ノード数
	u16 center_node;			//中間ノード数
	u16 out_nide;				//出力ノード数
	u8 padding2[2];

	float *pcenter_wit_offset;	//中間ウェイトデータ先頭位置オフセット
	float *pout_wit_offset;		//出力ウェイトデータ先頭位置オフセット
	float *pout2_wit_offset;		//出力ノード設定先頭位置オフセット
	//u8 filter_p[255];			//フィルタデータ
	//ST_NN_RESULT outputs[100];		//アウトプットデータの配列
	//float center_wit[115200];	//中間ウェイトデータ
	//float out_wit[14400];		//出力ウェイトデータ

} ST_NN;

typedef struct//NN識別用★
{
	float *OutArray;		//出力配列
	float *ImagePointer;	//イメージデータポインタ
	float OutputVal;		//最大発火値
	char NoteID;			//券種ID
	char padding[3];

} ST_NN_Result;


typedef struct//NN識別用★
{
	s16 size_x;
	s16 size_y;

} ST_NN_RESULT_SIZE_CHECK;




#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

float sigmoid(float x);
void NN(ST_NN *cfg, ST_NN_RESULT *res);
void node_comparison(void);
u32 out_put_value_cal(ST_NN_NODE_THRESHOLD* st, ST_NN_RESULT *res);
s8  nn_res_size_check(u8 buf_num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NN_H */
