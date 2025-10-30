// getdt.c
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

#define ACTUAL_MODE 0

#define EXT
#include "../common/global.h"
EXTERN int debug_logi_view;			//トレースするかしないか
EXTERN int debug_itool_trace_view;	//トレースするかしないか

#define NEW_DIR
#define NEW_DPI

#define GETDT_ROUNDING 0.5f

//挿入方向CかDの時のプレーン番号テーブル
//絶対に配列の順番を変えないでください	
u32 REV_P_Plane_tbl[] = {
	DOWN_R_R, DOWN_R_G, DOWN_R_B, DOWN_R_IR1, DOWN_R_IR2, DOWN_R_FL,  P_Reserve1,  P_Reserve2,
	UP_R_R,   UP_R_G,   UP_R_B,   UP_R_IR1,   UP_R_IR2,   UP_R_FL,    P_Reserve4,  P_Reserve5,
	UP_T_R,   UP_T_G,   UP_T_B,   UP_T_IR1,   UP_T_IR2,   P_Reserve7, P_Reserve8,  
	DOWN_T_R, DOWN_T_G, DOWN_T_B, DOWN_T_IR1, DOWN_T_IR2, P_Reserve10,P_Reserve11, 
	UP_MAG,   UP_MAG2,  UP_TC1,   UP_TC2,     UP_SP_A,    UP_SP_A2,   UP_SP_B,     UP_SP_B2 , UP_CAP1 , UP_CAP2
};

//挿入方向AかBの時のプレーン番号テーブル
u32 p_plane_tbl_[] = {
	UP_R_R,   UP_R_G,   UP_R_B,   UP_R_IR1,   UP_R_IR2,   UP_R_FL,     P_Reserve1,  P_Reserve2,
	DOWN_R_R, DOWN_R_G, DOWN_R_B, DOWN_R_IR1, DOWN_R_IR2, DOWN_R_FL,   P_Reserve4,  P_Reserve5,
	UP_T_R,   UP_T_G,   UP_T_B,   UP_T_IR1,   UP_T_IR2,   P_Reserve7,  P_Reserve8,  
	DOWN_T_R, DOWN_T_G, DOWN_T_B, DOWN_T_IR1, DOWN_T_IR2, P_Reserve10, P_Reserve11, 
	UP_MAG,   UP_MAG2,  UP_TC1,   UP_TC2,     UP_SP_A,    UP_SP_A2,    UP_SP_B,     UP_SP_B2 , UP_CAP1 , UP_CAP2
};


//////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 物理座標系から入力物理座標に対応するセンサーデータを得る
/// 有効物理座標外なら、エラーコードを返す。
/// </summary>
/// <param name="sp"></param>
/// <returns></returns>
///////////////////////////////////////////////////////////////////////////////////
u16 P_Getdt(ST_SPOINT *__restrict spt ,u8 buf_num)
{	
	ST_BS* pbs = work[buf_num].pbs;
	
#ifndef _RM_
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;

	if(pbs->PlaneInfo[spt->p_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
	{
		spt->sensdt = 0;				//0書き込み
		return ERR_NON_COMPLIANT_PLANE;//エラーコード返す
	}

#ifdef I_AND_A
	if ((spt->x < pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_min) || (spt->x > pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max) // 有効物理座標外ならば、
		|| (spt->y < 0) || (spt->y > pnote_param->sub_eff_range[spt->p_plane_tbl]))
	{
		spt->sensdt = 0;				//0書き込み
		return ERR_OUT_OF_PIXEL_RANGE; // エラーコードを返す
	}
#endif

	////参照したアドレスがメモリ範囲外？
	//if((u32)(&pbs->sens_dt[(u32)spt->y * pbs->PlaneInfo[spt->p_plane_tbl].Address_Period + (u32)spt->x  + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset]) > over_address_up || 
	//	(u32)(&pbs->sens_dt[(u32)spt->y * pbs->PlaneInfo[spt->p_plane_tbl].Address_Period + (u32)spt->x  + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset]) < over_address_down)
	//{
	//	//return ERR_REFERENCE_OUT_OF_AREA;
	//}
#endif


	//アドレス式
	/*データ長が8bit（1バイト）より大きい場合*/
	if(pbs->PlaneInfo[spt->p_plane_tbl].data_type > 8)		/*新*/
	//if(pbs->PlaneInfo[spt->p_plane_tbl].data_type > 1)	/*旧*/
	{
		/*2バイト分のデータを取り出します*/
		memcpy(&spt->sensdt,&pbs->sens_dt[pbs->PlaneInfo[spt->p_plane_tbl].Address_Period * spt->y + (spt->x * 2) + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset],2);
		/* xbitシフトします。(x = bit - 8 ) 　*/
		spt->sensdt = spt->sensdt >> (pbs->PlaneInfo[spt->p_plane_tbl].data_type - 8);	/*新*/
		//spt->sensdt = spt->sensdt >> (12 - 8);	/*旧*/
	}
	/*通常*/
	else
	{
		//アドレス式
		spt->sensdt = pbs->sens_dt[pbs->PlaneInfo[spt->p_plane_tbl].Address_Period * spt->y + spt->x + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset];
	}


#ifdef VS_DEBUG
	if(debug_logi_view == 1)
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = spt->x;	//
		deb_para[2] = spt->y;	//
		deb_para[3] = 1;		// plane
		deb_para[4] = spt->p_plane_tbl;		// plane number
		callback(deb_para);		// debug
	}
#endif

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 物理座標系から入力物理座標に対応するセンサーデータを得る
/// 有効物理座標外なら、エラーコードを返す。
/// 1byteのサイズのプレーンしか扱わないPgetdtでこちらの方が処理速度が速い。
/// </summary>
/// <param name="sp"></param>
/// <returns></returns>
///////////////////////////////////////////////////////////////////////////////////
u16 P_Getdt_8bit_only(ST_SPOINT *__restrict spt ,ST_BS * __restrict pbs)
{
#ifdef VS_DEBUG1
	//アドレス式
	spt->sensdt = pbs->sens_dt[pbs->PlaneInfo[spt->p_plane_tbl].Address_Period * spt->y + spt->x + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset];

#else
	//アドレス式
	spt->period = pbs->PlaneInfo[spt->p_plane_tbl].Address_Period;
	spt->p = &pbs->sens_dt[spt->period * spt->y + spt->x + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset];
	spt->sensdt = *spt->p;

#endif

#ifdef VS_DEBUG_ITOOL	//UI表示用　実機の時は消す
	new_P2L_Coordinate(spt, pbs);
	return 0;
#endif

#ifdef VS_DEBUG	//UI表示用　実機の時は消す
#ifndef VS_DEBUG_ITOOL	//UI表示用　実機の時は消す
	if(debug_logi_view == 1)
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = spt->x;	//
		deb_para[2] = spt->y;	//
		deb_para[3] = 1;		// plane
		deb_para[4] = spt->p_plane_tbl;		// plane number
		callback(deb_para);		// debugaa
	}
#endif
#endif


	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 物理座標系から入力物理座標に対応するセンサーデータを得る
/// 有効物理座標外なら、エラーコードを返す。
/// 1byteのサイズのプレーンしか扱わないPgetdtでこちらの方が処理速度が速い。
/// </summary>
/// <param name="sp"></param>
/// <returns></returns>
///////////////////////////////////////////////////////////////////////////////////
u16 P_Getdt_16bit_only(ST_SPOINT *__restrict spt ,u8 buf_num)
{
	ST_BS* pbs = work[buf_num].pbs;
	/*2バイト分のデータを取り出します*/
	memcpy(&spt->sensdt,&pbs->sens_dt[pbs->PlaneInfo[spt->p_plane_tbl].Address_Period * spt->y + (spt->x * 2) + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset],2);
	/* xbitシフトします。(x = bit - 8 ) 　*/
	//spt->sensdt = spt->sensdt >> (pbs->PlaneInfo[spt->p_plane_tbl].data_type - 8);	/*新*/
	//spt->sensdt = spt->sensdt >> (12 - 8);	/*旧*/

#ifdef VS_DEBUG_ITOOL	//UI表示用　実機の時は消す
	new_P2L_Coordinate(spt, pbs);
	return 0;
#endif

#ifdef VS_DEBUG	//UI表示用　実機の時は消す
	if (debug_logi_view == 1)
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = spt->x;	//
		deb_para[2] = spt->y;	//
		deb_para[3] = 1;		// plane
		deb_para[4] = spt->p_plane_tbl;		// plane number
		callback(deb_para);		// debugaa
	}
#endif
	return 0;
}


//SE,LEの違いを吸収するためのの関数     (x,y)を入れ替える
void x_y_change(s32 *x ,s32 *y){

	s32 temp;

	temp = *y;
	*y = -*x;
	*x = temp;

}



//傾き設定
void set_skew_info(ST_BS *pbs ,u8 Plane , ST_ANGLE* ang)
{
	//スキャン方向補正
		if(pbs->PlaneInfo[Plane].note_scan_dir == R2L)
		{
			//内部で扱うスキューデータの符号反転
			ang->tan_th = -pbs->angle / 4096.0f;
		}
		else
		{
			//内部で扱うスキューデータの符号反転
			ang->tan_th = pbs->angle / 4096.0f;
		}


	ang->cos_th = (float)sqrt(1 / (ang->tan_th * ang->tan_th + 1));
    ang->sin_th = ang->tan_th * ang->cos_th;

}



//************************************************************
//@brief		論理座標から物理座標を得る。
//@param[in]	spt 入力座標パラメタ
//@param[in]	pbs サンプリングデータ
//@param[in]	pnote_param 座標変換パラメータ
//@param[out]	spt.xとy　変換した座標 方向補正したプレーン番号
//return 0
//備考
//高速化したL2P
//たぶん引数に　pbsとnote_paramを持ってきた方が良い
//************************************************************
s16 new_L2P_Coordinate(ST_SPOINT * __restrict spt , ST_BS * __restrict pbs, ST_NOTE_PARAMETER* __restrict pnote_param)
{
	//処理中の紙幣情報

	float current_sin_x =0;
	float current_cos_x =0;
	float current_sin_y =0;
	float current_cos_y =0;
	float current_res_x =0;
	float current_res_y =0;
	u8 plane;
	float midst_res_x;
	float midst_res_y;
	float constant_x,constant_y;
	float input_x,input_y;

	float sin_x,cos_x ,sin_y,cos_y;
#ifdef VS_DEBUG
	int testx;
	int testy;

	if (debug_logi_view == 1)
	{

		//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
		if (pbs->LEorSE == SE)  //SEならば
		{
			testx = spt->x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2; 	//
			testy = pnote_param->main_eff_range / 2 - spt->y;	//

			deb_para[0] = 1;		// function code
			deb_para[1] = testx;
			deb_para[2] = testy;	//
			deb_para[3] = 1;		// color
			deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number

			callback(deb_para);		// debug
		}
		else
		{
			deb_para[0] = 1;		// function code
			deb_para[1] = spt->x + pnote_param->main_eff_range / 2;	//
			deb_para[2] = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - spt->y;	//
			deb_para[3] = 1;		// color
			deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
			callback(deb_para);		// debug
		}
	}
#endif

	//反射プレーンを挿入方向に応じて
	//表裏を入れ替え
	spt->p_plane_tbl = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];
	plane = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];

	sin_x = pnote_param->sin_x[plane];

	cos_x = pnote_param->cos_x[plane];

	sin_y = pnote_param->sin_y[plane];

	cos_y = pnote_param->cos_y[plane];

	if(pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
	{
		input_x = spt->x * pnote_param->insart_multi_x[plane];
		input_y = (spt->y) * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x_non_ofs[plane] ;
		constant_y = pnote_param->coordinate_param_y[plane];

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_x =  constant_x - midst_res_x;
		current_res_y = (midst_res_y + constant_y);

	}

	else  //座標変換　+　画素参照
	{
		input_x = (spt->x) * pnote_param->insart_multi_x[plane];
		input_y = spt->y * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x[plane];
		constant_y = pnote_param->coordinate_param_y_non_ofs[plane] ;

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_y =  constant_x - midst_res_x;
		current_res_x = (midst_res_y + constant_y);

	}

	spt->x = (s32)(current_res_x + GETDT_ROUNDING);
	spt->y = (s32)(current_res_y + GETDT_ROUNDING);

	return 0;

}

//************************************************************
//@brief		論理座標から物理座標に変換して画素値を返す
//　　　　　　　decision_plane_side_in_insertion_direction
//				advance_insert_correction
//				cal_note_parameter
//				以上3つの関数が終了していることが条件
//@param[in]	spt 入力座標パラメタ
//@param[in]	pbs サンプリングデータ
//@param[in]	pnote_param 座標変換パラメータ
//@param[out]	spt.xとy　変換した座標
//@param[out]	spt.sensdt 画素値
//return 0
//備考
//高速化したLgetdt関数,呼び出しはやめてL2PとPgetを関数内にべた書きしてます。
//************************************************************
u16 new_L_Getdt(ST_SPOINT * __restrict spt , ST_BS * __restrict pbs , ST_NOTE_PARAMETER * __restrict pnote_param)
{
	//u16 err_code = 0;
	float current_sin_x =0;
	float current_cos_x =0;
	float current_sin_y =0;
	float current_cos_y =0;
	float current_res_x =0;
	float current_res_y =0;
	u8 plane;
	float midst_res_x;
	float midst_res_y;
	float constant_x,constant_y;
	float input_x,input_y;
	u32 period;
	u32 offset;

	float sin_x,cos_x ,sin_y,cos_y;

#ifdef VS_DEBUG
	int testx;
	int testy;

	if(debug_logi_view == 1)
	{

		//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
		if(pbs->LEorSE == SE)  //SEならば
		{
			testx = spt->x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2; 	//
			testy = pnote_param->main_eff_range / 2 - spt->y;	//

			deb_para[0] = 1;		// function code
			deb_para[1] = testx;
			deb_para[2] = testy;	//
			deb_para[3] = 1;		// plane
			deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
			callback(deb_para);		// debug
		}
		else
		{
			deb_para[0] = 1;		// function code
			deb_para[1] = spt->x + pnote_param->main_eff_range / 2;	//
			deb_para[2] = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - spt->y;	//
			deb_para[3] = 1;		// plane
			deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
			callback(deb_para);		// debug
		}
	}
#endif


	spt->p_plane_tbl = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];
	plane = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];

	sin_x = pnote_param->sin_x[plane];
	cos_x = pnote_param->cos_x[plane];
	sin_y = pnote_param->sin_y[plane];
	cos_y = pnote_param->cos_y[plane];
	period = pbs->PlaneInfo[plane].Address_Period;
	offset = pbs->PlaneInfo[plane].Address_Offset;

	if(pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
	{
		input_x = spt->x * pnote_param->insart_multi_x[plane];
		input_y = (spt->y) * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x_non_ofs[plane] ;
		constant_y = pnote_param->coordinate_param_y[plane];

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_x =  constant_x - midst_res_x;
		current_res_y = (midst_res_y + constant_y);

	}

		else  //座標変換　+　画素参照
	{
		input_x = (spt->x ) * pnote_param->insart_multi_x[plane];
		input_y = spt->y * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x[plane];
		constant_y = pnote_param->coordinate_param_y_non_ofs[plane] ;

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_y =  constant_x - midst_res_x;
		current_res_x = (midst_res_y + constant_y);

	}

	spt->x = (s32)(current_res_x + GETDT_ROUNDING);
	spt->y = (s32)(current_res_y + GETDT_ROUNDING);


#ifndef _RM_
	//計算した物理座標が搬送エリア外？
	if ((spt->x < pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_min) || (spt->x > pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_max) // 有効物理座標外ならば、
		|| (spt->y < 0) || (spt->y > pnote_param->sub_eff_range[spt->l_plane_tbl]))
	{
		spt->sensdt = 255;				//0書き込み
		return ERR_OUT_OF_PIXEL_RANGE; // エラーコードを返す
	}
#endif
#ifdef I_AND_A
	//参照アドレスがメモリ範囲外？
	if((T_POINTER)(&pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset]) > over_address_up || 
		(T_POINTER)(&pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset]) < over_address_down)
	{
		return ERR_REFERENCE_OUT_OF_AREA;
	}
#endif

	// 磁気のときはifに入るでokのはず
	//アドレス式
	//データ長が8bit（1バイト）より大きい場合
	if(pbs->PlaneInfo[plane].data_type > 8)
	{

		/*2バイト分のデータを取り出します*/
		memcpy(&spt->sensdt,&pbs->sens_dt[(u32)spt->y * period + (u32)(spt->x * 2) + offset],2);
		spt->sensdt = spt->sensdt >> (pbs->PlaneInfo[plane].data_type - 8);
	}
	//通常
	else
	{
		spt->sensdt = (s16)pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset];
	}

	return 0;
}

//************************************************************
//@brief		論理座標から物理座標に変換して画素値を返す
//　　　　　　　decision_plane_side_in_insertion_direction
//				advance_insert_correction
//				cal_note_parameter
//				以上3つの関数が終了していることが条件
//@param[in]	spt 入力座標パラメタ
//@param[in]	pbs サンプリングデータ
//@param[in]	pnote_param 座標変換パラメータ
//@param[out]	spt.xとy　変換した座標
//@param[out]	spt.sensdt 画素値
//return 0
//備考
//8bitデータにのみ対応したLgetです。
//************************************************************
u16 new_L_Getdt_08bit_only(ST_SPOINT * __restrict spt , ST_BS * __restrict pbs , ST_NOTE_PARAMETER * __restrict pnote_param)
{
	//u16 err_code = 0;
	float current_sin_x =0;
	float current_cos_x =0;
	float current_sin_y =0;
	float current_cos_y =0;
	float current_res_x =0;
	float current_res_y =0;
	u8 plane;
	float midst_res_x;
	float midst_res_y;
	float constant_x,constant_y;
	float input_x,input_y;
	u32 period;
	u32 offset;

	float sin_x,cos_x ,sin_y,cos_y;

#ifdef VS_DEBUG
	int testx;
	int testy;

	if(debug_logi_view == 1)
	{

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == SE)  //SEならば
	{
		testx = spt->x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2; 	//
		testy = pnote_param->main_eff_range / 2 - spt->y;	//

		deb_para[0] = 1;		// function code
		deb_para[1] = testx;
		deb_para[2] = testy;	//
		deb_para[3] = 1;		// plane
		deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
		callback(deb_para);		// debug
	}
	else
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = spt->x + pnote_param->main_eff_range / 2;	//
		deb_para[2] = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - spt->y;	//
		deb_para[3] = 1;		// plane
		deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
		callback(deb_para);		// debug
	}
	}
#endif


	spt->p_plane_tbl = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];
	plane = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];

	sin_x = pnote_param->sin_x[plane];
	cos_x = pnote_param->cos_x[plane];
	sin_y = pnote_param->sin_y[plane];
	cos_y = pnote_param->cos_y[plane];
	period = pbs->PlaneInfo[plane].Address_Period;
	offset = pbs->PlaneInfo[plane].Address_Offset;

	if(pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
	{
		input_x = spt->x * pnote_param->insart_multi_x[plane];
		input_y = (spt->y) * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x_non_ofs[plane] ;
		constant_y = pnote_param->coordinate_param_y[plane];

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_x =  constant_x - midst_res_x;
		current_res_y = (midst_res_y + constant_y);

	}
		else  //座標変換　+　画素参照
	{
		input_x = (spt->x) * pnote_param->insart_multi_x[plane];
		input_y = spt->y * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x[plane];
		constant_y = pnote_param->coordinate_param_y_non_ofs[plane] ;

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_y =  constant_x - midst_res_x;
		current_res_x = (midst_res_y + constant_y);

	}

	spt->x = (s32)(current_res_x + GETDT_ROUNDING);
	spt->y = (s32)(current_res_y + GETDT_ROUNDING);

#ifndef _RM_
	//計算した物理座標が搬送エリア外？
	if ((spt->x < pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_min) || (spt->x > pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_max) // 有効物理座標外ならば、
		|| (spt->y < 0) || (spt->y > pnote_param->sub_eff_range[spt->l_plane_tbl]))
	{
		spt->sensdt = 255;				//0書き込み
		return ERR_OUT_OF_PIXEL_RANGE; // エラーコードを返す
	}
#endif

#ifdef I_AND_A
	//参照アドレスがメモリ範囲外？
	if((T_POINTER)(&pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset]) > over_address_up || 
		(T_POINTER)(&pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset]) < over_address_down)
	{
		return ERR_REFERENCE_OUT_OF_AREA;
	}
#endif

	//アドレス式
	//データ長が8bit（1バイト）より大きい場合
	//if(pbs->PlaneInfo[plane].data_type > 8)
	//{
		/*2バイト分のデータを取り出します*/
	//memcpy(&spt->sensdt,&pbs->sens_dt[(u32)spt->y * period + (u32)(spt->x * 2) + offset],2);
	//spt->sensdt = spt->sensdt >> (pbs->PlaneInfo[plane].data_type - 8);
	//}
	////通常
	//else
	//{
	spt->sensdt = pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset];
	//}

	return 0;
}

//************************************************************
//@brief		論理座標から物理座標に変換して画素値を返す
//　　　　　　　decision_plane_side_in_insertion_direction
//				advance_insert_correction
//				cal_note_parameter
//				以上3つの関数が終了していることが条件
//@param[in]	spt 入力座標パラメタ
//@param[in]	pbs サンプリングデータ
//@param[in]	pnote_param 座標変換パラメータ
//@param[out]	spt.xとy　変換した座標
//@param[out]	spt.sensdt 画素値
//return 0
//備考
//16bitデータにのみ対応したLgetです。
//************************************************************
u16 new_L_Getdt_16bit_only(ST_SPOINT * __restrict spt , ST_BS * __restrict pbs , ST_NOTE_PARAMETER * __restrict pnote_param)
{
	//u16 err_code = 0;
	float current_sin_x =0;
	float current_cos_x =0;
	float current_sin_y =0;
	float current_cos_y =0;
	float current_res_x =0;
	float current_res_y =0;
	u8 plane;
	float midst_res_x;
	float midst_res_y;
	float constant_x,constant_y;
	float input_x,input_y;

//#ifdef I_AND_A
	u32 period;
	u32 offset;
//#endif

	float sin_x,cos_x ,sin_y,cos_y;

#ifdef VS_DEBUG
	int testx;
	int testy;

	if(debug_logi_view == 1)
	{

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == SE)  //SEならば
	{
		testx = spt->x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2; 	//
		testy = pnote_param->main_eff_range / 2 - spt->y;	//

		deb_para[0] = 1;		// function code
		deb_para[1] = testx;
		deb_para[2] = testy;	//
		deb_para[3] = 1;		// plane
		deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
		callback(deb_para);		// debug
	}
	else
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = spt->x + pnote_param->main_eff_range / 2;	//
		deb_para[2] = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - spt->y;	//
		deb_para[3] = 1;		// plane
		deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
		callback(deb_para);		// debug
	}
	}
#endif


	spt->p_plane_tbl = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];
	plane = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];

	sin_x = pnote_param->sin_x[plane];
	cos_x = pnote_param->cos_x[plane];
	sin_y = pnote_param->sin_y[plane];
	cos_y = pnote_param->cos_y[plane];
//#ifdef I_AND_A
	period = pbs->PlaneInfo[plane].Address_Period;
	offset = pbs->PlaneInfo[plane].Address_Offset;
//#endif
	if(pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
	{
		input_x = spt->x * pnote_param->insart_multi_x[plane];
		input_y = (spt->y) * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x_non_ofs[plane] ;
		constant_y = pnote_param->coordinate_param_y[plane];

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_x =  constant_x - midst_res_x;
		current_res_y = (midst_res_y + constant_y);

	}

		else  //座標変換　+　画素参照
	{
		input_x = (spt->x) * pnote_param->insart_multi_x[plane];
		input_y = spt->y * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x[plane];
		constant_y = pnote_param->coordinate_param_y_non_ofs[plane] ;

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x =  (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_y =  constant_x - midst_res_x;
		current_res_x = (midst_res_y + constant_y);

	}

	spt->x = (s32)(current_res_x + GETDT_ROUNDING);
	spt->y = (s32)(current_res_y + GETDT_ROUNDING);

#ifndef _RM_
	//計算した物理座標が搬送エリア外？
	if ((spt->x < pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_min) || (spt->x > pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_max) // 有効物理座標外ならば、
		|| (spt->y < 0) || (spt->y > pnote_param->sub_eff_range[spt->l_plane_tbl]))
	{
		spt->sensdt = 255;				//0書き込み
		return ERR_OUT_OF_PIXEL_RANGE; // エラーコードを返す
	}
#endif

#ifdef I_AND_A
	//参照アドレスがメモリ範囲外？
	if((T_POINTER)(&pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset]) > over_address_up || 
		(T_POINTER)(&pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset]) < over_address_down)
	{
		return ERR_REFERENCE_OUT_OF_AREA;
	}
#endif
	//アドレス式
	//データ長が8bit（1バイト）より大きい場合
	//if(pbs->PlaneInfo[plane].data_type > 8)
	//{
		/*2バイト分のデータを取り出します*/
	memcpy(&spt->sensdt,&pbs->sens_dt[(u32)spt->y * period + (u32)(spt->x * 2) + offset],2);
	//spt->sensdt = spt->sensdt >> (pbs->PlaneInfo[plane].data_type - 8);
	//}
	////通常
	//else
	//{
	//	spt->sensdt = pbs->sens_dt[(u32)spt->y * period + (u32)spt->x  + offset];
	//}

	return 0;
}




//************************************************************
//@brief		挿入方向に応じて参照物理プレーンの表裏を切り替える　識別処理終了後に実行する
//				例）OMOTE_R_Rの場合、挿入方向0か1　→　UP_R_R
//									 挿入方向2か3　→　DOWN_R_R
//@param		バッファ番号
//************************************************************
void decision_plane_side_in_insertion_direction(u8 buf_num)
{
	//処理中の紙幣情報
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;

	if(pbs->insertion_direction == 0 || pbs->insertion_direction == 1)
	{
		pnote_param->pplane_tbl = (u32 *)p_plane_tbl_;
	}
	else
	{
		pnote_param->pplane_tbl = (u32 *)REV_P_Plane_tbl;
	}
}

//************************************************************
//@brief		物理⇔論理座標変換に必要なパラメタをあらかじめ計算する関数
//@param		バッファ番号
//************************************************************
s16 cal_note_parameter(u8 buf_num)
{
	u8 plane;
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	ST_ANGLE ang;

	float x_dpi = 0.0;

	//float test = 0.0;

	//傾き情報設定
	set_skew_info(pbs ,0, &ang);

	//有効画素数計算
	pnote_param->main_eff_range = pbs->PlaneInfo[0].main_effective_range_max - pbs->PlaneInfo[0].main_effective_range_min ;

	//搬送方向フラグ初期化
	pnote_param->transport_flg = 1; //1

	//プレーンテーブル初期化
	pnote_param->pplane_tbl = (u32 *)p_plane_tbl_;

	//入出金方向補正    
	if(pbs->transport_direction == WITHDRAWAL)
	{
		pnote_param->transport_flg = -1;
	}

	for(plane = 0; plane < MAX_PLANE_COUNT; plane++)
	{
		
		//if(!(pbs->PlaneInfo[plane].Enable_or_Disable == 1 || pbs->PlaneInfo[plane].Enable_or_Disable == 0) )
		//{
		//	return -1;
		//}
		

		
		//if(plane == 30)
		//{
		//	pnote_param->transport_flg = 1;
		//}
		


		//プレーン有無の確認
		if(pbs->PlaneInfo[plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効ならば
		{
			continue;
		}

		//スキャン方向に応じて中心x座標を反転
		//またｘ座標を反転する変数（scan_diy_flg）の値を決定する
		if(pbs->PlaneInfo[plane].note_scan_dir == R2L)	//逆スキャン
		{
			//中心x座標を反転
			pnote_param->temp_center_x[plane] = pnote_param->main_eff_range- pbs->center_x;
			pnote_param->scan_diy_flg[plane] = pnote_param->main_eff_range;
			pnote_param->scan_dir_skew_coefficient[plane] = -1;					//処理中のスキャンする順版を決定する	反転してるなら物理座標を右から読む

		}
		else	//順スキャン
		{
			pnote_param->temp_center_x[plane] = pbs->center_x;
			pnote_param->scan_dir_skew_coefficient[plane] = 1;					//処理中のスキャンする順版を決定する	反転してないから物理座標を左から読む
			pnote_param->scan_diy_flg[plane] = 0;
		}

		//pnote_param->main_offset[plane] = (pnote_param->main_eff_range / 2 + pbs->PlaneInfo[0].main_effective_range_min) - (pbs->PlaneInfo[plane].main_offset / 2);


		//主走査オフセット計算
		//pnote_param->main_offset[plane] = (s16)(pbs->PlaneInfo[plane].main_offset * 0.5f) - pnote_param->temp_center_x[plane];

		//x:dpi補正値
		//pnote_param->main_dpi_cor[plane] = (1 / (LOGICAL_COORDINATE_DPI / (25.4 / pbs->PlaneInfo[plane].main_element_pitch)));
		pnote_param->main_dpi_cor[plane] = 25.4f / (LOGICAL_COORDINATE_DPI * pbs->PlaneInfo[plane].main_element_pitch);

		//200との比
		x_dpi = 200.0f / (25.4f / pbs->PlaneInfo[plane].main_element_pitch);
		//x_dpi = 1;

		//y:dpi補正値
		pnote_param->sub_dpi_cor[plane] = 1.0f / (LOGICAL_COORDINATE_DPI / pbs->pitch_ratio /( pbs->Subblock_dpi/pbs->PlaneInfo[plane].sub_sampling_pitch));
		//pnote_param->sub_dpi_cor[plane] =  1 / (pbs->pitch_ratio * pbs->PlaneInfo[plane].sub_sampling_pitch / (LOGICAL_COORDINATE_DPI / pbs->Subblock_dpi));

		//主走査オフセット計算 単位は論理座標（200ｘ200dpi）
		//CIS	　：搬送路の真ん中とセンサーの中心素子の位置合わせる
		//それ以外：CISの中心素子とそのセンサーの中心位置を合わせる
		pnote_param->main_offset[plane] = (s16)(((((pbs->PlaneInfo[plane].main_effective_range_max - pbs->PlaneInfo[plane].main_effective_range_min) * 0.5f)
			+ pbs->PlaneInfo[plane].main_effective_range_min) - (pbs->PlaneInfo[plane].main_offset * 0.5f)) * x_dpi);

		//副走査方向有効範囲を計算
		//pnote_param->sub_eff_range[plane] = pbs->Blocksize * pbs->block_count / pbs->PlaneInfo[plane].sub_sampling_pitch - 1;
		pnote_param->sub_eff_range[plane] =	(u16)(pbs->block_count * ((float)pbs->Blocksize / (float)pbs->PlaneInfo[plane].sub_sampling_pitch));	// 副走査方向の最大有効範囲
		
		//座標変換に用いる係数を計算
		if(pbs->LEorSE == LE)
		{
			//方向補正値1で初期化
			pnote_param->insart_multi_x[plane] = (float)pnote_param->scan_dir_skew_coefficient[plane];
			pnote_param->insart_multi_y[plane] = 1;

			pnote_param->coordinate_param_x[plane]         =  ((float)(abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pnote_param->main_offset[plane]) ) * pnote_param->main_dpi_cor[plane])+ pbs->PlaneInfo[plane].Address_Offset;

			pnote_param->coordinate_param_x_non_ofs[plane] = (((float)(abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pnote_param->main_offset[plane]) ) * pnote_param->main_dpi_cor[plane]));

			pnote_param->coordinate_param_y[plane] = (pbs->center_y + pbs->PlaneInfo[plane].sub_offset) * pnote_param->sub_dpi_cor[plane];
			//pnote_param->coordinate_param_y[plane] = (pbs->center_y) * pnote_param->sub_dpi_cor[plane];

			pnote_param->sin_x[plane] = ang.sin_th * pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_x[plane] = ang.cos_th * pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;

			pnote_param->sin_y[plane] = ang.sin_th * pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_y[plane] = ang.cos_th * pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
		}

		else
		{

			//方向補正値1で初期化
			pnote_param->insart_multi_x[plane] = -1;
			pnote_param->insart_multi_y[plane] = (float)(-1 * pnote_param->scan_dir_skew_coefficient[plane]);

#ifdef NEW_DPI
			pnote_param->coordinate_param_y[plane] = ((abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pnote_param->main_offset[plane] ) * pnote_param->main_dpi_cor[plane]))+ pbs->PlaneInfo[plane].Address_Offset;

			pnote_param->coordinate_param_y_non_ofs[plane] = ((abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pnote_param->main_offset[plane] ) * pnote_param->main_dpi_cor[plane]));

			pnote_param->coordinate_param_x[plane] = (((pbs->center_y + pbs->PlaneInfo[plane].sub_offset) * pnote_param->sub_dpi_cor[plane]));
#else
			pnote_param->coordinate_param_y[plane] = ((abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pbs->PlaneInfo[plane].sub_offset ) * pnote_param->main_dpi_cor[plane]))+ pbs->PlaneInfo[plane].Address_Offset;
			pnote_param->coordinate_param_y_non_ofs[plane] = ((abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pbs->PlaneInfo[plane].sub_offset ) * pnote_param->main_dpi_cor[plane]));
			pnote_param->coordinate_param_x[plane] = (((pbs->center_y +  pnote_param->main_offset[plane]) * pnote_param->sub_dpi_cor[plane]));
#endif

			pnote_param->sin_x[plane] = (ang.sin_th * pnote_param->scan_dir_skew_coefficient[plane]) * pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_x[plane] = (ang.cos_th) * pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;

			pnote_param->sin_y[plane] = (ang.sin_th * pnote_param->scan_dir_skew_coefficient[plane]) * pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_y[plane] = (ang.cos_th) * pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;

			
			//if(plane == 30)//そのプレーンが無効ならば
			//{
			//	pnote_param->insart_multi_x[plane] = -1;
			//}
		}
	}
#ifdef VS_DEBUG_ITOOL
	if (debug_itool_trace_view == 1)
	{
		cal_P2L_parameter(buf_num);
	}
#endif
	return 0;

}
//************************************************************
//@brief		insart_multi_xとｙの値をリセットする
//				方向Ａとはまた別のパラメータとなっているので
//				
//@param		バッファ番号
//************************************************************
void reset_insart_multi_num(u8 buf_num) 
{
	u8 plane;
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;


	for (plane = 0; plane < MAX_PLANE_COUNT; plane++)
	{
		if (pbs->PlaneInfo[plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効ならば
		{
			continue;
		}

		if (pbs->PlaneInfo[plane].note_scan_dir == R2L)	//逆スキャン
		{
			pnote_param->scan_dir_skew_coefficient[plane] = -1;					//処理中のスキャンする順版を決定する	反転してるなら物理座標を右から読む
		}
		else	//順スキャン
		{
			pnote_param->scan_dir_skew_coefficient[plane] = 1;					//処理中のスキャンする順版を決定する	反転してないから物理座標を左から読む
		}

		if (pbs->LEorSE == LE)
		{
			//方向補正値1で初期化
			pnote_param->insart_multi_x[plane] = (float)pnote_param->scan_dir_skew_coefficient[plane];
			pnote_param->insart_multi_y[plane] = 1;
		}
		else
		{

			//方向補正値1で初期化
			pnote_param->insart_multi_x[plane] = -1;
			pnote_param->insart_multi_y[plane] = (float)(-1 * pnote_param->scan_dir_skew_coefficient[plane]);

		}
	}
}

//************************************************************
//@brief		挿入方向補正用乗数を決定する関数　識別処理終了後に実行する
//				方向補正以外にもスキャン方向補正も行う。補正係数 → pnote_param->transport_flg
//				この関数内ではスキャン方向は考慮していない。
//				スキャン方向は
//				
//@param		バッファ番号
//************************************************************
void advance_insert_correction(u8 buf_num){

	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	u8 plane = 0;

	for(plane = 0; plane < MAX_PLANE_COUNT; plane++)
	{

		//プレーン有無の確認
		if(pbs->PlaneInfo[plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効ならば
		{
			continue;
		}

		if(pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
		{
			switch (pbs->insertion_direction)//方向座標変換
			{
			case W0:
				pnote_param->insart_multi_x[plane] = 1.0f;
				pnote_param->insart_multi_y[plane] = 1.0f* pnote_param->transport_flg;
				break;

			case W1:	//厚みは関係なし

				if(plane == OMOTE_TC1 || plane == OMOTE_TC2)
				{
					pnote_param->insart_multi_x[plane] = -1.0f;
					pnote_param->insart_multi_y[plane] = 1.0f * pnote_param->transport_flg;

				}
				else
				{
					pnote_param->insart_multi_x[plane] = -1.0f;
					pnote_param->insart_multi_y[plane] = -1.0f * pnote_param->transport_flg;

				}

				break;

#ifdef NEW_DIR

			case W2:	//厚みは関係なし
				
				if(plane == OMOTE_TC1 || plane == OMOTE_TC2)
				{
					pnote_param->insart_multi_x[plane] = 1.0f;
					pnote_param->insart_multi_y[plane] = 1.0f * pnote_param->transport_flg;

				}
				else
				{
					pnote_param->insart_multi_x[plane] = 1.0f;
					pnote_param->insart_multi_y[plane] = -1.0f * pnote_param->transport_flg;

				}

				break;

			case W3:
				pnote_param->insart_multi_x[plane] = -1.0f;
				pnote_param->insart_multi_y[plane] = 1.0f* pnote_param->transport_flg;
				break;

#else
			case W2:
				pnote_param->insart_multi_x[plane] = -1.0f;
				pnote_param->insart_multi_y[plane] = 1.0f * pnote_param->transport_flg;
				break;

			case W3:	//厚みは関係なし
				pnote_param->insart_multi_x[plane] = 1.0f;
				pnote_param->insart_multi_y[plane] = -1.0f * pnote_param->transport_flg;
				break;

#endif
			}
		}

		else
		{
			switch (pbs->insertion_direction)//方向座標変換
			{
			case W0:
				pnote_param->insart_multi_x[plane] = -1.0f * pnote_param->transport_flg;
				pnote_param->insart_multi_y[plane] = (float)(-1.0f * pnote_param->scan_dir_skew_coefficient[plane]);
				break;

			case W1:
				pnote_param->insart_multi_x[plane] = 1.0f * pnote_param->transport_flg;
				pnote_param->insart_multi_y[plane] = (float)(1.0f * pnote_param->scan_dir_skew_coefficient[plane]);
				break;
#ifdef NEW_DIR
			case W2:
				pnote_param->insart_multi_x[plane] = -1.0f * pnote_param->transport_flg;
				pnote_param->insart_multi_y[plane] = (float)(1.0f * pnote_param->scan_dir_skew_coefficient[plane]) ;
				break;

			case W3:
				pnote_param->insart_multi_x[plane] = 1.0f * pnote_param->transport_flg;
				pnote_param->insart_multi_y[plane] = (float)(-1.0f * pnote_param->scan_dir_skew_coefficient[plane]);
				break;
#else
			case W2:
				pnote_param->insart_multi_x[plane] = 1.0f * pnote_param->transport_flg;
				pnote_param->insart_multi_y[plane] = (float)(-1.0f * pnote_param->scan_dir_skew_coefficient[plane]) ;
				break;

			case W3:
				pnote_param->insart_multi_x[plane] = -1.0f * pnote_param->transport_flg;
				pnote_param->insart_multi_y[plane] = (float)(1.0f * pnote_param->scan_dir_skew_coefficient[plane] );
				break;
#endif
			}
		}
	}
}

//4つの頂点の論理座標を取得します。
s16	get_vertex(ST_VERTEX *pver ,u8 buf_num)
{
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	//ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;

//#if botu
//	//P2L設定
//	spt.p_plane_tbl = DOWN_R_R;
//	spt.way = pbs->insertion_direction;
//	//pt.way = 0;
//
//	/*頂点座標を論理座標系に変換*/
//	//左上
//	spt.x = pbs->left_up_x;
//	spt.y = pbs->left_up_y;//
//
//	P2L_Coordinate(&spt ,dog.buf_num);
//
//	set_vertex(&pver->left_up_x , &pver->left_up_y , &spt ,pbs,1 ,pnote_param);
//
//	//左下
//	spt.x = pbs->left_down_x;
//	spt.y = pbs->left_down_y;
//
//	P2L_Coordinate(&spt ,dog.buf_num);
//
//	set_vertex(&pver->left_down_x , &pver->left_down_y , &spt ,pbs ,2 ,pnote_param);
//
//
//	//右上
//	spt.x = pbs->right_up_x;
//	spt.y = pbs->right_up_y;
//
//	P2L_Coordinate(&spt ,dog.buf_num);
//
//	set_vertex(&pver->right_up_x , &pver->right_up_y , &spt ,pbs ,3 ,pnote_param);
//
//
//	//右下
//	spt.x = pbs->right_down_x;
//	spt.y = pbs->right_down_y;
//
//	P2L_Coordinate(&spt ,dog.buf_num);
//
//	set_vertex(&pver->right_down_x , &pver->right_down_y , &spt ,pbs ,4 ,pnote_param);
//#endif

	s16 hx = 0;
	s16 hy = 0;

	if(pbs->LEorSE == SE)
	{
		hx = (s16)(pbs->note_y_size * 0.5f + 0.5f);
		hy = (s16)(pbs->note_x_size * 0.5f + 0.5f);
	}
	else
	{
		hx = (s16)(pbs->note_x_size * 0.5f + 0.5f);
		hy = (s16)(pbs->note_y_size * 0.5f + 0.5f);
	}

	pver->left_up_x = -hx;
	pver->left_up_y = hy;

	pver->left_down_x = -hx;
	pver->left_down_y = -hy;

	pver->right_up_x = hx;
	pver->right_up_y = hy;

	pver->right_down_x = hx;
	pver->right_down_y = -hy;

	return 0;

}

//クイックソートを行います
void quick_sort( u8 ListToSort[], s32 left, s32 right )
{
	s32 i = left, j = right;
	s16 num = (s16)(( left + right ) * 0.5f);
	u8 pivot = ListToSort[num];

	u8 c;

	while ( i <= j )
	{
		while ( ListToSort[i] < pivot )
		{
			i++;
		}
		while ( ListToSort[j] > pivot )
		{
			j--;
		}
		if ( i <= j )
		{
			c =  ListToSort[j];
			ListToSort[j] = ListToSort[i];
			ListToSort[i] = c;

			//SWAP( ListToSort[i], ListToSort[j] );
			i++;
			j--;
		}
	}
	/* recursion */
	if ( left < j )
	{
		quick_sort( ListToSort, left, j );
	}
	if ( i < right )
	{
		quick_sort( ListToSort, i, right );
	}
}

//傾き補正
void skew_correction(s32 *x ,s32 *y, ST_ANGLE* ang){

	float xx,yy;

	xx = (*x * ang->cos_th + *y * ang->sin_th );
	yy = (*y * ang->cos_th - *x * ang->sin_th );

	*x = (s32)xx;
	*y = (s32)yy;

}

/*******************************
レベル変換関数
検知結果の値と、レベル40の閾値を与えることで、その検知結果のレベルを出力する。(1~100)
検知結果が複数ある場合、各要素に対してレベルを計算し、最小値のレベルを結果として出力する。
例）面積60レベル、辺50レベルの場合、60レベルが出力される。
in	
float	*detect_res_ary		検知結果を格納した配列
float	*thr_ary			閾値を格納した配列
u8		detect_res_num		検知結果の要素数
float   min					その判定の最低値レベル90に相当する値
float	max					その判定の最大値レベル１に相当する値

out
u8		level				レベル

*/
u8  level_detect(float *detect_res_ary, float *thr_ary,	u8 detect_res_num ,float min ,float max)
{

#define THR_RECIPROCAL_50	(0.02f)	//50の逆数
#define THR_RECIPROCAL_40	(0.025f)	//40の逆数
#define THR_UF				(40)			//UFの基準値

	u8	level = MAX_LEVEL;
	u8	num;
	s32	temp_level;
	float min_temp = 0.0;
	float max_temp = 0.0;
	float val_temp = 0.0;

	//u32 limit = MIN_LEVEL - 1;

	if(detect_res_ary[0] < 0)	//nn1,2用
	{
		return MIN_LEVEL;
	}


	for(num = 0; num < detect_res_num; ++num)
	{

		if(thr_ary[num] < min)	//最小値より閾値が小さい場合
		{			
			return MIN_LEVEL;	//計算できないので100を返す。
		}
		else if (thr_ary[num] > max)
		{
			return MAX_LEVEL;
		}

		max_temp = max - thr_ary[num];
		min_temp = thr_ary[num] - min;

		max_temp = max_temp * THR_RECIPROCAL_40;
		min_temp = min_temp * THR_RECIPROCAL_50;
		
		//0除算回避用
		if(max_temp == 0)		
		{
			max_temp = 0.00001f;	
		}

		if(min_temp == 0)
		{
			min_temp = 0.00001f;
		}

		if(detect_res_ary[num] >= thr_ary[num])	//閾値以上の場合
		{
			val_temp = detect_res_ary[num] - thr_ary[num];
			val_temp = val_temp / max_temp;
			temp_level = (s32)(THR_UF - val_temp);

			if(temp_level <= 0)
			{
				temp_level = 1;
			}
		}
		else									//閾値未満の場合
		{
			val_temp = thr_ary[num] - detect_res_ary[num];
			val_temp = val_temp / min_temp;
			temp_level = (s32)(THR_UF + val_temp);

			if(temp_level > 90)
			{
				temp_level = 100;
			}

		}

		//temp_level = MIN_LEVEL - temp_level; 

		//最大値更新
		if(level < temp_level)
		{
			level = (u8)temp_level;
		}
	}





#ifdef old_model
	for(num = 0; num < detect_res_num; ++num)
	{
		//検知結果の値をレベルに変換
		x = thr_ary[num] * THR_RECIPROCAL;
		temp_level = (u32)(detect_res_ary[num] / x + 0.5f);

		if(temp_level > limit)
		{
			temp_level = limit;
		}

		temp_level = MIN_LEVEL - temp_level; 


		//最大値更新
		if(level < temp_level)
		{
			level = (u8)temp_level;
	}

	}
#endif

	return level;

}


/****************************************************************/
/**
* @brief		サンプリングデータの紙幣イメージを作成する。
*@param[in]		buf		バッファ番号
				*p_pln	プレーン
				pln_num	プレーンの数
				hid		高さ			作成するdpiのdotで指定
				wid		幅				作成するdpiのdotで指定
				xs		スタート位置ｘ	200dpi単位でのスタート地点
				ys		スタート位置ｙ	200dpi単位でのスタート地点
				coord	座標系の指定	0：物理	1：論理
				*p_img	イメージデータのポインタ
				dpi		作成するイメージのスケール（dpi）
*@param[out]	*p_img	イメージデータのポインタ
* @return		参照回数
*/
/****************************************************************/
u32 get_cutout_color_img(u8 buf_num, u8 *p_pln, u8 pln_num, u16 hid, u16 wid, s16 xs, s16 ys, u8 coord, u8* p_img, u8 dpi)
{
	s16 x;
	s16 y;
	float yc = 0;
	float xc = 0;

	u32 i = 0;
	u8  pln_counter;
	//u8 pln_num = sizeof(&p_pln);
	ST_SPOINT	spt;
	float pitch;
	float  dpi_mutl_y;
	float  dpi_mutl_x;
	u16		temp;
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;

	if(pbs->LEorSE == SE)
	{
		temp = hid;
		hid = wid;
		wid = temp;

		temp = xs;
		xs = ys;
		ys = temp;
	}

	if(coord == 0)		//物理
	{
		for(pln_counter = 0; pln_counter < pln_num; ++pln_counter)
		{	
			spt.p_plane_tbl = p_pln[pln_counter];								//指定されたプレーンを設定
			pitch = (float)(200.0f / pbs->PlaneInfo[spt.p_plane_tbl].sub_sampling_pitch);	//副走査dpiを取得
			dpi_mutl_y = pitch / dpi;											//イメージのdpiとの比を求める

			pitch = (float)(25.4f / pbs->PlaneInfo[spt.p_plane_tbl].main_element_pitch);//主走査dpiを取得
			dpi_mutl_x = pitch / dpi;											//イメージのdpiとの比を求める
			//dpi_mutl_x = 4;
			i = pln_counter;				//イメージ配列のプレーンの先頭
			spt.y = ys;						//参照開始座標
			yc = xs;						//

			for(y = 0; y < hid; ++y)
			{
				xc = xs;					//参照開始座標
				spt.x = xs;					//

				for(x = 0; x < wid; ++x)	
				{
					P_Getdt(&spt ,buf_num);	//画素参照
					p_img[i] = (u8)spt.sensdt;	//
					//i += pln_num + 1;		//C#デバッグ用
					i += pln_num;			//

					xc += dpi_mutl_x;		//dpi比に応じてインクリメント
					spt.x = (s32)xc;		

				}

				yc += dpi_mutl_y;			//dpi比に応じてインクリメント
				spt.y = (s32)yc;
			}
		}

		
		////C#デバッグ用
		//i = pln_num;
		//for(y = ys; y < hid; ++y)
		//{
		//	for(x = xs; x < wid; ++x)
		//	{
		//		p_img[i] = 255;
		//		i += pln_num+1;
		//	}

		//}
		
	}
	else if(coord == 1)	//論理
	{
		for(pln_counter = 0; pln_counter < pln_num; ++pln_counter)
		{	
			spt.l_plane_tbl = p_pln[pln_counter];								//指定されたプレーンを設定
			dpi_mutl_x = (float)(200 / dpi);												//論理なので200のみ
			i = pln_counter;

			spt.y = ys;
			yc = ys;

			for(y = 0; y < hid; ++y)
			{
				xc = xs;
				spt.x = xs;

				for(x = 0; x < wid; ++x)
				{
					new_L_Getdt(&spt ,pbs ,pnote_param);
					p_img[i] = (u8)spt.sensdt;
					//i += pln_num + 1;		//C#デバッグ用
					i += pln_num;

					xc += dpi_mutl_x;
					spt.x = (s32)xc;
					spt.y = (s32)yc;

				}

				yc -= dpi_mutl_x;
				spt.y = (s32)yc;
			}
		}

		
		////C#デバッグ用
		//i = pln_num;
		//for(y = 0; y < hid; ++y)
		//{
		//	for(x = 0; x < wid; ++x)
		//	{
		//		p_img[i] = 255;
		//		i += pln_num+1;
		//	}

		//}
		
	}

	//return i - (pln_num);		//C#デバッグ用
	return i - (pln_num - 1);

}

//メカ厚用Pget関数
//入力座標のTC1-TC2の結果を返す。
u16 P_GetThicknessdata(ST_BS * __restrict pbs, s32 posx, s32 posy, s8 tc_corr_mode)
{
	u16 tc_sensdt[2];			//センサーデータ(画素値)
	u16 thicknessdt;
	u8 tc_plane[2] = { UP_TC1, UP_TC2 };
	u8 tc_offset[2] = { 4, 0 };
	u8 i;
	u8 lim = 2;

	tc_sensdt[0] = 0;
	tc_sensdt[1] = 0;


	if(tc_corr_mode == 0)	//tc1-tc2をしない場合
	{
		lim = 1;
	}

	for (i = 0; i < lim; i++)
	{
		u16  sensdt;
		s32 ch;
		u32 period;
		u32 offset;
		u8 plane = tc_plane[i];

		ch = posx + tc_offset[i];
		period = pbs->PlaneInfo[plane].Address_Period;
		offset = pbs->PlaneInfo[plane].Address_Offset;

		//アドレス式
		//データ長が8bit（1バイト）より大きい場合
		if (pbs->PlaneInfo[plane].data_type > 8)
		{
			/*2バイト分のデータを取り出します*/
			memcpy(&sensdt, &pbs->sens_dt[(u32)posy * period + (u32)(ch * 2) + offset], 2);
//			sensdt = sensdt >> (pbs->PlaneInfo[plane].data_type - 8);
		}
		//通常
		else
		{
			sensdt = pbs->sens_dt[(u32)posy * period + (u32)ch + offset];
		}
		tc_sensdt[i] = sensdt;
	}

	if (tc_sensdt[0] > tc_sensdt[1])	//TC1-TC2
	{
		thicknessdt = tc_sensdt[0] - tc_sensdt[1];
	}
	else
	{
		thicknessdt = 0;
	}
	return thicknessdt;
//	return tc_sensdt[0];
}

//デバッグ検証用のコールバック
#ifdef VS_DEBUG_ITOOL
void for_Dbg_start_algo_label_setting(int algo_index, int coordinate_index)
#else
void for_Dbg_start_algo_label_setting()
#endif
{
#ifdef VS_DEBUG_ITOOL
	deb_para[0] = 0;		
	deb_para[1] = algo_index;
	deb_para[2] = coordinate_index;	
	deb_para[3] = 0;		
	callback(deb_para);
#endif
}



#ifdef VS_DEBUG

// デバッグ検証用のコールバック
// Point_vicinity_calで参照する画素の座標を論理座標系に変換してコールバックする。
// ITOOLのビューワに用いる。
void for_iTool_trace_callback(ST_SPOINT* __restrict spt, ST_BS* __restrict pbs, ST_NOTE_PARAMETER* __restrict pnote_param, u16 xsize, u16 ysize, u8* mask, u8 half_point_flg)
{
	const u8 For_UV = 1;

	//s32 testx;						
	//s32 testy;
	s32 x = 0;		            //現在の座標
	s32 y = 0;		            //現在の座標
	float xCounter = 0;	        //マスク配列参照用
	float yCounter = 0;	        //マスク配列参照用
	float xAddVal = 0;	        //マスク配列のインクリメント値
	float yAddVal = 0;	        //マスク配列のインクリメント値
	s32 tmp = 0;		        //マスク配列の参照値計算用
	u16 filter_half_size_x =0;	//マスクのハーフサイズ
	u16 filter_half_size_y =0;  //マスクのハーフサイズ
	s32 xStart = 0;             //画素参照スタート位置　
	s32 yStart = 0;             //画素参照スタート位置　
	s32	ori_ysize = ysize;		//オリジナルのマスクサイズy方向

	ST_NOTE_PARAMETER* pnote_param_P2L = &work[pbs->buf_num].p2l_params;
	u8 plane = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];

	if (debug_itool_trace_view == 0)
	{
		return;
	}
	
	//dpi比を補正する
	//xsize = xsize * pnote_param_P2L->main_dpi_cor[plane];

	if (half_point_flg != For_UV)
	{
		ysize = (u16)(ysize * pnote_param_P2L->sub_dpi_cor[plane]);

		//端数を除去
		if (pnote_param_P2L->sub_dpi_cor[plane] != 1)
		{
			ysize -= (u16)(pnote_param_P2L->sub_dpi_cor[plane] - 1);
		}
	}

	//マスクのハーフサイズを計算する
	if (half_point_flg == For_UV)	//UV検知場合の仕様
	{
		filter_half_size_x = xsize;
		filter_half_size_y = ysize;

		//インクリメント値を計算（uvはマスクサイズがdpiに関わらず7固定なので・・・）
		xAddVal = 1.0f / pnote_param_P2L->main_dpi_cor[plane];
		yAddVal = 1.0f / (s32)(25.0f / (float)ysize + 0.5f);

		ysize = 25;
	}
	else
	{
		filter_half_size_x = xsize >> 1;
		filter_half_size_y = ysize >> 1;

		//インクリメント値を計算（100dpiならインクリメント値は0.5になる
		xAddVal = 1 / pnote_param_P2L->main_dpi_cor[plane];
		yAddVal = 1.0f / (25.0f / ori_ysize);
	}

	//スタート位置に移動　（論理座標系で計算してる
	xStart = spt->x - filter_half_size_x; //左上へ
	yStart = spt->y + filter_half_size_y; //


	//座標計算を行う
	for (y = yStart; y > yStart - ysize; --y)	//上から下へ
	{
		xCounter = 0;

		for (x = xStart; x < xStart + xsize; ++x)	//左から右へ
		{
			tmp = (int)yCounter * xsize;	//y座標オフセット計算
			/*フィルターの要素を取得*/
			u8 mlt = *(mask + tmp + (int)xCounter);	// 参照マスク配列のアドレス計算

			//0以外なら演算
			if (mlt != 0)
			{
#ifdef VS_DEBUG

				//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
				if (pbs->LEorSE == SE)  //SEならば
				{
					deb_para[0] = 1;		// function code
					deb_para[1] = pnote_param_P2L->image_coordinate_system_main + x;
					deb_para[2] = (s32)(pnote_param_P2L->main_eff_range * 0.5f - y);	//
					deb_para[3] = 1;		// color
					deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
					callback(deb_para);		// debug
				}
				else
				{
					deb_para[0] = 1;		// function code
					deb_para[1] = (s32)(x + pnote_param_P2L->main_eff_range * 0.5f);	//
					deb_para[2] = pnote_param_P2L->image_coordinate_system_main - y;	//
					deb_para[3] = 1;		// color
					deb_para[4] = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];		// plane number
					callback(deb_para);		// debug
				}
#endif
			}

			xCounter += xAddVal;	//インクリメント
		}

		yCounter += yAddVal;		//インクリメント
	}
}


//************************************************************
//@brief		物理⇔論理座標変換に必要なパラメタをあらかじめ計算する関数
//@param		バッファ番号
//************************************************************
s16 cal_P2L_parameter(u8 buf_num)
{
	u8 plane;
	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].p2l_params;
	ST_ANGLE ang;

	//float x_dpi_ratio = 0.0f;
	//float y_dpi_ratio = 0.0f;
	float x_dpi = 0.0f;

	//傾き情報設定
	set_skew_info(pbs, 0, &ang);

	//有効画素数計算
	pnote_param->main_eff_range = pbs->PlaneInfo[0].main_effective_range_max - pbs->PlaneInfo[0].main_effective_range_min;

	//搬送方向フラグ初期化
	pnote_param->transport_flg = 1; //1

	//プレーンテーブル初期化
	pnote_param->pplane_tbl = (u32*)p_plane_tbl_;

	//入出金方向補正    
	if (pbs->transport_direction == WITHDRAWAL)
	{
		pnote_param->transport_flg = -1;
	}

	//搬送ピッチ補正値の逆数を計算
	pnote_param->pitch_reciprocal = 1 / pbs->pitch_ratio;

	//イメージ座標系変換値を計算
	pnote_param->image_coordinate_system_main = (u32)(pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch * pbs->block_count * 0.5f);

	for (plane = 0; plane < MAX_PLANE_COUNT; plane++)
	{
		//プレーン有無の確認
		if (pbs->PlaneInfo[plane].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効ならば
		{
			continue;
		}

		x_dpi = 200.0f / (25.4f / pbs->PlaneInfo[plane].main_element_pitch);

		//スキャン方向に応じて中心x座標を反転
		//またｘ座標を反転する変数（scan_diy_flg）の値を決定する
		if (pbs->PlaneInfo[plane].note_scan_dir == R2L)	//逆スキャン
		{
			//中心x座標を反転
			pnote_param->temp_center_x[plane] = pnote_param->main_eff_range - pbs->center_x;
			pnote_param->scan_diy_flg[plane] = pnote_param->main_eff_range;
			pnote_param->scan_dir_skew_coefficient[plane] = -1;					//処理中のスキャンする順版を決定する	反転してるなら物理座標を右から読む

		}
		else	//順スキャン
		{
			pnote_param->temp_center_x[plane] = pbs->center_x;
			pnote_param->scan_dir_skew_coefficient[plane] = 1;					//処理中のスキャンする順版を決定する	反転してないから物理座標を左から読む
			pnote_param->scan_diy_flg[plane] = 0;
		}

		//主走査オフセット計算 単位は論理座標（200ｘ200dpi）
		//CIS	　：搬送路の真ん中とセンサーの中心素子の位置合わせる
		//それ以外：CISの中心素子とそのセンサーの中心位置を合わせる
		pnote_param->main_offset[plane] = (s16)(((((pbs->PlaneInfo[plane].main_effective_range_max - pbs->PlaneInfo[plane].main_effective_range_min) * 0.5f)
			+ pbs->PlaneInfo[plane].main_effective_range_min) - (pbs->PlaneInfo[plane].main_offset * 0.5f)) * x_dpi);

		//x:dpi補正値
		//pnote_param->main_dpi_cor[plane] = (1 / (LOGICAL_COORDINATE_DPI / (25.4 / pbs->PlaneInfo[plane].main_element_pitch)));
		pnote_param->main_dpi_cor[plane] = pbs->PlaneInfo[plane].main_element_pitch / 0.127f;

		//y:dpi補正値
		pnote_param->sub_dpi_cor[plane] = (float)(LOGICAL_COORDINATE_DPI /** pbs->pitch_ratio*/ / (pbs->Subblock_dpi / pbs->PlaneInfo[plane].sub_sampling_pitch));

		//座標変換に用いる係数を計算
		if (pbs->LEorSE == LE)
		{
			//方向補正値1で初期化
			pnote_param->insart_multi_x[plane] = (float)pnote_param->scan_dir_skew_coefficient[plane];
			pnote_param->insart_multi_y[plane] = 1;

			//方向補正値1で初期化
			pnote_param->coordinate_param_x[plane] = ((float)(abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] - pnote_param->main_offset[plane])) * pnote_param->main_dpi_cor[plane]) + pbs->PlaneInfo[plane].Address_Offset;
			pnote_param->coordinate_param_x_non_ofs[plane] = (((float)(abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pnote_param->main_offset[plane]))/* * pnote_param->main_dpi_cor[plane]*/));
			pnote_param->coordinate_param_y[plane] = (float)(pbs->center_y + pbs->PlaneInfo[plane].sub_offset);// / pnote_param->sub_dpi_cor[plane];

			pnote_param->sin_x[plane] = ang.sin_th;// / pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_x[plane] = ang.cos_th;// / pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->sin_y[plane] = ang.sin_th;// / pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_y[plane] = ang.cos_th;// / pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
		}
		else
		{
			//方向補正値1で初期化
			pnote_param->insart_multi_x[plane] = -1;
			pnote_param->insart_multi_y[plane] = (float)(-1 * pnote_param->scan_dir_skew_coefficient[plane]);

			pnote_param->coordinate_param_y[plane] = ((abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] - pnote_param->main_offset[plane]) / pnote_param->main_dpi_cor[plane])) + pbs->PlaneInfo[plane].Address_Offset;
			pnote_param->coordinate_param_y_non_ofs[plane] = (float)((abs(pnote_param->scan_diy_flg[plane] - pnote_param->temp_center_x[plane] + pnote_param->main_offset[plane]) /*/ pnote_param->main_dpi_cor[plane]*/));
			pnote_param->coordinate_param_x[plane] = (float)((pbs->center_y + pbs->PlaneInfo[plane].sub_offset));// *pnote_param->sub_dpi_cor[plane]));

			pnote_param->sin_x[plane] = (ang.sin_th);// / pnote_param->scan_dir_skew_coefficient[plane]) / pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_x[plane] = (ang.cos_th);// / pnote_param->sub_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->sin_y[plane] = (ang.sin_th);// / pnote_param->scan_dir_skew_coefficient[plane]) / pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
			pnote_param->cos_y[plane] = (ang.cos_th);// / pnote_param->main_dpi_cor[plane];// * pbs->PlaneInfo[plane].Address_Period + 0.5;
		}
	}
	return 0;
}

//************************************************************
//@brief		物理座標→論理座標を得る。
//@param[in]	spt 入力座標パラメータ
//@param[in]	pbs サンプリングデータ
//@param[in]	pnote_param 座標変換パラメータ
//@param[out]	spt.xとy　変換した座標 方向補正したプレーン番号
//return 0
//備考
//高速化したL2P
//たぶん引数に　pbsとnote_paramを持ってきた方が良い
//************************************************************
s16 new_P2L_Coordinate(ST_SPOINT* __restrict spt, ST_BS* __restrict pbs)
{
	//処理中の紙幣情報

	float current_sin_x = 0;
	float current_cos_x = 0;
	float current_sin_y = 0;
	float current_cos_y = 0;
	//float current_res_x = 0;
	//float current_res_y = 0;
	u8 plane;
	//float midst_res_x;
	//float midst_res_y;
	float constant_x, constant_y;
	float input_x, input_y;
	s32 x, y;
	s8 insert_x;
	s8 insert_y;


	float sin_x, cos_x, sin_y, cos_y;

	ST_NOTE_PARAMETER* pnote_param		= &work[pbs->buf_num].p2l_params;
	ST_NOTE_PARAMETER* pnote_param_ori	= &work[pbs->buf_num].note_param;

	if (spt->trace_flg == 0)
	{
		return 0;
	}
	if (debug_itool_trace_view == 0)
	{
		return 0;
	}

	if (spt->p_plane_tbl < 42)
	{
		plane = (u8)pnote_param_ori->pplane_tbl[spt->p_plane_tbl];
	}
	else
	{
		plane = (u8)pnote_param_ori->pplane_tbl[spt->l_plane_tbl];
	}

	//反射プレーンを挿入方向に応じて
	//表裏を入れ替え
	sin_x = pnote_param->sin_x[plane];
	cos_x = pnote_param->cos_x[plane];
	sin_y = pnote_param->sin_y[plane];
	cos_y = pnote_param->cos_y[plane];

	if (pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
	{
		input_x = spt->x * pnote_param->main_dpi_cor[plane];
		input_y = spt->y * pnote_param->sub_dpi_cor[plane] * pnote_param->pitch_reciprocal;

		constant_x = pnote_param->coordinate_param_x_non_ofs[plane];
		constant_y = pnote_param->coordinate_param_y[plane];
		insert_x = (s8)pnote_param_ori->insart_multi_x[plane];
		insert_y = (s8)pnote_param_ori->insart_multi_y[plane];
	}

	else  //座標変換　+　画素参照
	{
		input_y = spt->x * pnote_param->main_dpi_cor[plane];
		input_x = spt->y * pnote_param->sub_dpi_cor[plane] * pnote_param->pitch_reciprocal;

		constant_x = pnote_param->coordinate_param_x[plane];
		constant_y = pnote_param->coordinate_param_y_non_ofs[plane];

		//insert_y = pnote_param_ori->insart_multi_x[plane];
		//insert_x = pnote_param_ori->insart_multi_y[plane];
	}

	x = (s32)(input_x - constant_x);
	y = (s32)(-(input_y - constant_y));

	current_cos_y = y * cos_y;
	current_sin_x = y * sin_x;

	current_cos_x = x * cos_x;
	current_sin_y = x * sin_y;

	x = (s32)(-(current_cos_x + current_sin_x));
	y = (s32)((current_cos_y - current_sin_y));

	x = (s32)(x * pnote_param_ori->insart_multi_x[plane]);
	y = (s32)(y * pnote_param_ori->insart_multi_y[plane]);

	//x = midst_res_x * insert_x;
	//y = midst_res_y * insert_y;

#ifdef VS_DEBUG

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if (pbs->LEorSE == SE)  //SEならば
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = pnote_param->image_coordinate_system_main + x;
		deb_para[2] = (s32)(pnote_param->main_eff_range * 0.5f - y);	//
		deb_para[3] = 1;		// color
		deb_para[4] = plane;		// plane number
		callback(deb_para);		// debug
	}
	else
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = (s32)(x + pnote_param->main_eff_range * 0.5f);	//
		deb_para[2] = pnote_param->image_coordinate_system_main - y;	//
		deb_para[3] = 1;		// color
		deb_para[4] = plane;		// plane number
		callback(deb_para);		// debug
	}

#endif

	return 0;

}


//ビューアー用　実機には入らない
//スキュー補正をせずにオフセット補正、dpi補正を行って画素値を返す
u16 P_Getdt_dpi_and_offset(ST_SPOINT *spt ,u8 bufnum)
{
	ST_BS* pbs = work[bufnum].pbs;

	//u16 err_code;
	s16 main_offset;

	s32 temp;

	//	x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == SE)  //SEならば
	{
			temp = spt->y;
	spt->y = spt->x;
	spt->x = pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max - temp;

	}

	//プレーン有無の確認
	if(pbs->PlaneInfo[spt->p_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
	{
		spt->sensdt = 0;				//0書き込み
		return ERR_NON_COMPLIANT_PLANE;	//エラーコード返す
	}

	//ｘ座標スキャン方向補正
	if(pbs->PlaneInfo[spt->p_plane_tbl].note_scan_dir == R2L)
	{
		//ｘ座標反転
		spt->x = pbs->PlaneInfo[0].main_effective_range_max - pbs->PlaneInfo[0].main_effective_range_min - spt->x;
	}

	//主走査オフセット計算
	main_offset= (((pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max - pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_min) / 2
		+ pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_min) - (pbs->PlaneInfo[spt->p_plane_tbl].main_offset / 2));

	//dpi補正とオフセット補正　x　note:ｘとｙのオフセット補正値の扱いが違うので分けている
	spt->x = spt->x + main_offset;
	//spt->x = (spt->x / (LOGICAL_COORDINATE_DPI / (25.4 / pbs->PlaneInfo[spt->p_plane_tbl].main_element_pitch)));	/*旧型*/
	spt->x = (spt->x / ((pbs->PlaneInfo[0].main_effective_range_max - pbs->PlaneInfo[0].main_effective_range_min) /
						((pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max)  - pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_min)));	/*新型*/
						//(pbs->PlaneInfo[spt->p_plane_tbl].main_all_pix - pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_min)));	/*新型*/
	//dpi補正とオフセット補正　y
	spt->y = spt->y + pbs->PlaneInfo[spt->p_plane_tbl].sub_offset;
	spt->y = (s32)((spt->y / (LOGICAL_COORDINATE_DPI * pbs->pitch_ratio /( pbs->Subblock_dpi/pbs->PlaneInfo[spt->p_plane_tbl].sub_sampling_pitch))));


	//アドレス式
	P_Getdt(spt ,0);

	return 0;
}



//論理座標を物理座標に変換する関数
//試験用関数　こちらで動作を確認し　New_L2Pに組み込む
s16 L2P_Coordinate(ST_SPOINT *spt , u8 buf_num)
{//論理座標から物理座標へ

	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	ST_ANGLE ang;
	//挿入方向補正
	insert_correction(spt ,pbs);

	//プレーン有無の確認
	if(pbs->PlaneInfo[spt->p_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効ならば
	{
		//エラーの場合0で初期化
		spt->x = 0;
		spt->y = 0;
		return ERR_NON_COMPLIANT_PLANE;//エラーコード返す
	}

	//入出金方向補正
	if(pbs->transport_direction == WITHDRAWAL)
	{
		if(pbs->LEorSE == SE)  //SEならば
		{
			spt->x = -spt->x;
		}
		else	//LE
		{
			spt->y = -spt->y;
		}
	}

	// 傾き補正
	skew_correction(&spt->x, &spt->y, &ang);

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == SE)  //SEならば
	{
		x_y_change(&spt->x, &spt->y);//反転
	}



	//中心基準
	spt->x = spt->x + pnote_param->temp_center_x[spt->p_plane_tbl];
	spt->y = (-spt->y + pbs->center_y);

	//ｘ座標スキャン方向補正
	if(pbs->PlaneInfo[spt->p_plane_tbl].note_scan_dir == R2L)
	{
		//ｘ座標反転
		spt->x = pnote_param->main_eff_range - spt->x;
	}


	//dpi補正とオフセット補正　x　note:ｘとｙのオフセット補正値の扱いが違うので分けている
	spt->x = (s32)(spt->x + pnote_param->main_offset[spt->p_plane_tbl]);
	spt->x = (s32)(spt->x * pnote_param->main_dpi_cor[spt->p_plane_tbl]);

	//dpi補正とオフセット補正　y
	spt->y = (s32)(spt->y + pbs->PlaneInfo[spt->p_plane_tbl].sub_offset);
	spt->y = (s32)(spt->y * pnote_param->sub_dpi_cor[spt->p_plane_tbl]);


	//補正後の座標が範囲外かどうかの確認
	if ((spt->x < 0) || (spt->x > pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max -1) // 有効物理座標外ならば、
		|| (spt->y < 0) || (spt->y > pnote_param->sub_eff_range[spt->p_plane_tbl]))
	{
		//エラーの場合0で初期化
		spt->x = 0;
		spt->y = 0;
		return ERR_OUT_OF_PIXEL_RANGE; // エラーコードを返す
	}

	return 0;
}

//物理座標から論理座標に変換
void set_planetbl_ (u8 buf_num)
{
	work[buf_num].p2l_params.pplane_tbl = (u32*)p_plane_tbl_;
	debug_itool_trace_view = 1;
}

//物理座標から論理座標に変換
s16 P2L_Coordinate(ST_SPOINT* spt ,u8 buf_num)
{

	ST_BS* pbs = work[buf_num].pbs;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	ST_ANGLE ang;

	////有効範囲かを確認
	//if ((spt->x < pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_min) || (spt->x > pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max -1) // 有効物理座標外ならば、
	//	|| (spt->y < 0) || (spt->y > (pbs->Blocksize * pbs->block_count / pbs->PlaneInfo[spt->p_plane_tbl].sub_sampling_pitch - 1)))
	//{
	//	エラーの場合0で初期化
	//	spt->x = 0;
	//	spt->y = 0;
	//	return ERR_OUT_OF_PIXEL_RANGE; // エラーコードを返す
	//}

	//プレーン有無の確認
	if(pbs->PlaneInfo[spt->p_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが無効ならば
	{
		//エラーの場合0で初期化
		spt->x = 0;
		spt->y = 0;
		return ERR_NON_COMPLIANT_PLANE;//エラーコード返す
	}

	//ｘ座標スキャン方向補正
	if(pbs->PlaneInfo[spt->p_plane_tbl].note_scan_dir == R2L)
	{
		//ｘ座標反転
		spt->x = pnote_param->main_eff_range - spt->x;
	}

	//中心基準
	spt->x = spt->x - pnote_param->temp_center_x[spt->p_plane_tbl];
	spt->y = (pbs->center_y - spt->y);

	////オフセット補正
	//spt->x = spt->x / pnote_param->main_dpi_cor[spt->p_plane_tbl];
	spt->x = spt->x - pnote_param->main_offset[spt->p_plane_tbl];

	//spt->y = spt->y / pnote_param->sub_dpi_cor[spt->p_plane_tbl];
	spt->y = spt->y - pbs->PlaneInfo[spt->p_plane_tbl].sub_offset;


	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == SE)  //SEならば
	{
		x_y_change(&spt->x, &spt->y);//反転
	}

		// 傾き補正
	skew_correction(&spt->y, &spt->x, &ang);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 入力物理座標に対応する生データを返す
/// 有効物理座標外なら、エラーコードを返す。
///　不要かもしれない(2/1)
/// </summary>
/// <param name="sp"></param>
/// <returns></returns>
///////////////////////////////////////////////////////////////////////////////////
u16 scanP_Getdt(ST_SPOINT *spt ,u8 buf_num)
{
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;

	if(pbs->PlaneInfo[spt->p_plane_tbl].Enable_or_Disable != PLANE_ENABLED)//そのプレーンが向こうならば
	{
		spt->sensdt = 0;				//0書き込み
		return ERR_NON_COMPLIANT_PLANE;//エラーコード返す
	}

	//if ((spt->x < 0) || (spt->x > pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max -1) // 有効物理座標外ならば、
	//	|| (spt->y < 0) || (spt->y > (pbs->Blocksize * pbs->block_count / pbs->PlaneInfo[spt->p_plane_tbl].sub_sampling_pitch - 1)))
	//{
	//	spt->sensdt = 0;				//0書き込み
	//	return ERR_OUT_OF_PIXEL_RANGE; // エラーコードを返す
	//}

			//スキャン方向補正
	if(pbs->PlaneInfo[spt->p_plane_tbl].note_scan_dir == R2L)
	{
		//ｘ座標反転
		spt->x = pbs->PlaneInfo[spt->p_plane_tbl].main_effective_range_max - spt->x;
	}

	P_Getdt(spt ,buf_num);

	return 0;
}



//挿入＆スキャン補正
u16 insert_and_scan(ST_SPOINT *spt ,u8 buf_num)
{
	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;

	//ｘ座標スキャン方向補正
	if(pbs->PlaneInfo[spt->p_plane_tbl].note_scan_dir == R2L)
	{
		//ｘ座標反転
		spt->x = pbs->PlaneInfo[0].main_effective_range_max - pbs->PlaneInfo[0].main_effective_range_min - spt->x;
	}

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == SE)  //SEならば
	{
		//x_y_change(&spt->x, &spt->y);//反転
	}

	/*変換した座標を用いて物理座標を取得*/
	P_Getdt(spt ,buf_num);

	return 0;
}

/// 有効物理座標外なら、エラーコードを返す。
/// </summary>
/// <param name="sp"></param>
/// <returns></returns>
///////////////////////////////////////////////////////////////////////////////////
u16 L_Getdt(ST_SPOINT *spt ,u8 buf_num)
{

	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	u16 err_code = 0;

#ifdef VS_DEBUG
	int testx;
	int testy;

	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;

	if(debug_logi_view == 1)
	{

	//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
	if(pbs->LEorSE == SE)  //SEならば
	{
		testx = spt->x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2; 	//
		testy = pnote_param->main_eff_range / 2 - spt->y;	//

		deb_para[0] = 1;		// function code
		deb_para[1] = testx;
		deb_para[2] = testy;	//
		deb_para[3] = 1;		// plane
		callback(deb_para);		// debug
	}
	else
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = spt->x + pnote_param->main_eff_range / 2;	//
		deb_para[2] = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - spt->y;	//
		deb_para[3] = 1;		// plane
		//callback(deb_para);		// debug
	}
	}
#endif

	//テスト版L2Pを実行
	err_code = L2P_Coordinate(spt , buf_num);

	//変換に問題はないか？
	if(err_code == 0)
	{
		/*変換した座標を用いて物理座標を取得*/
		P_Getdt(spt ,buf_num);

		return 0;
	}

	else if (err_code != 0 &&  pbs->PlaneInfo[spt->p_plane_tbl].Ref_or_Trans  == REFLECTION)
	{
		//問題アリの場合変換時に出たエラーコードを返す。
		spt->sensdt = 000;	//反射データの場合　0書き込み
		return err_code;
	}

	else
	{

		//問題アリの場合変換時に出たエラーコードを返す。
		spt->sensdt = 255;	//透過データの場合　255書き込み
		return err_code;
	}
}
//画面上下三か所を採取し最大レベルを返す。
//閾値などに用いる。
s16 get_threshold_background( s16 safe_val ,ST_BS *pbs	,u8 plane ,u8 buf_num)
{
	s32 x[3];		//三か所分
	s16 max = 0;
	s16 cmax = 0;

	ST_SPOINT top_spt;	//上部分採取
	ST_SPOINT bottom_spt;	//下部分採取
	u16 i;

	s32 height = 0;
	s32 width  = 0;

	height = (((pbs->Blocksize / pbs->PlaneInfo[plane].sub_sampling_pitch) * pbs->block_count));	//副走査最大
	width  = pbs->PlaneInfo[plane].main_effective_range_max;				 						//主走査最大

	/*プレーン決定*/
	top_spt.p_plane_tbl = (enum P_Plane_tbl)plane;
	bottom_spt.p_plane_tbl = (enum P_Plane_tbl)plane;

	//エリア選定式
	x[0] = width * 1/ 4 - 10;
	x[1] = width * 2/ 4 - 10;
	x[2] = width * 3/ 4 - 10;


	//上部・下部それぞれ三か所ずつ採取
	for ( i = 0; i < 3; i++ )
	{
		for ( top_spt.x = x[i], bottom_spt.x = x[i]; top_spt.x < x[i] + 10; top_spt.x++, bottom_spt.x++ )
		{
			for ( top_spt.y = 10, bottom_spt.y = height - 10; top_spt.y < 15; top_spt.y++, bottom_spt.y++ )
			{

				P_Getdt(&top_spt ,buf_num);		//上部ポイント
				P_Getdt(&bottom_spt ,buf_num);	//下部ポイント

				if(pbs->PlaneInfo[plane].Ref_or_Trans == REFLECTION)
				{
					cmax = ( top_spt.sensdt < bottom_spt.sensdt ) ? bottom_spt.sensdt : top_spt.sensdt;	// 明るい方

					if ( cmax > max )
					{
						max = cmax;		// 最大値保存
					}
				}
				else
				{
					cmax = ( top_spt.sensdt > bottom_spt.sensdt ) ? bottom_spt.sensdt : top_spt.sensdt;	// 暗い方

					if ( cmax < max )
					{
						max = cmax;		// 最小値保存
					}
				}

			}
		}
	}

	if ( max > safe_val )	// 返り値の最大値を制限する
	{
		return safe_val;
	}

	return max;


}

//紙幣上下三か所を採取し最大レベルを返す。
//閾値などに用いる。
s16 get_threshold_note( s16 safe_val ,ST_BS *pbs ,u8 plane,u8 buf_num)
{
	s16 x[3];		//三か所分
	s16 max = 0;
	s16 min = 255;
	s16 cmax = 0;
	s16 top_x = 0;
	s16 top_y = 0;
	s16 bottom_x = 0;
	s16 bottom_y = 0;

	ST_SPOINT top_spt;	//上部分採取
	ST_SPOINT bottom_spt;	//下部分採取
	u16 i;

	s16 height = 0;
	s16 width  = 0;
	s16 y_area = 0;

	height = pbs->note_y_size;
	width  = pbs->note_x_size;

	/*プレーン決定*/
	top_spt.l_plane_tbl = (enum L_Plane_tbl)plane;
	bottom_spt.l_plane_tbl = (enum L_Plane_tbl)plane;

	//エリア選定式
	x[0] = -width / 3;
	x[1] = 0;
	x[2] = width / 3;

	y_area = height / 3;


	//上部・下部それぞれ三か所ずつ採取
	for ( i = 0; i < 3; i++ )
	{
		for ( top_x = x[i], bottom_x = x[i]; top_x < x[i] + 10; top_x++, bottom_x++ )
		{
			for ( top_y = y_area, bottom_y = -y_area; top_y > y_area - 15; top_y--, bottom_y++ )
			{

				top_spt.x = top_x;
				top_spt.y = top_y;
				bottom_spt.x = bottom_x;
				bottom_spt.y = bottom_y;

				L_Getdt(&top_spt ,buf_num);		//上部ポイント
				L_Getdt(&bottom_spt ,buf_num);	//下部ポイント

				if(pbs->PlaneInfo[plane].Ref_or_Trans == REFLECTION)
				{
					cmax = ( top_spt.sensdt < bottom_spt.sensdt ) ? bottom_spt.sensdt : top_spt.sensdt;	// 明るい方

					if ( cmax > max )
					{
						max = cmax;		// 最大値保存
					}
				}
				else
				{
					cmax = ( top_spt.sensdt > bottom_spt.sensdt ) ? bottom_spt.sensdt : top_spt.sensdt;	// 暗い方

					if ( cmax < min )
					{
						min = cmax;		// 最小値保存
					}
				}

			}
		}
	}

	//if ( max > safe_val )	// 返り値の最大値を制限する
	//{
	//	return safe_val;
	//}
		if ( min < safe_val )	// 返り値の最大値を制限する
	{
		return safe_val;
	}


	return min;


}

//挿入方向補正
void insert_correction(ST_SPOINT *spt ,ST_BS *pbs){


	//反射か透過かMAGか？
	//反射かつMAG以外のプレーン
	if(pbs->PlaneInfo[spt->l_plane_tbl].Ref_or_Trans == REFLECTION && spt->l_plane_tbl < 28)
	{

		switch (pbs->PlaneInfo[spt->l_plane_tbl].note_scan_side)
		{
		case OMOTE:
			switch (spt->way)//方向座標変換
			{
			case W0:
				spt->p_plane_tbl = ( enum P_Plane_tbl )spt->l_plane_tbl;	//論理プレーンから物理プレーンへ
				spt->x = spt->x;
				spt->y = spt->y;
				break;
			case W1:
				spt->p_plane_tbl = ( enum P_Plane_tbl )spt->l_plane_tbl;	//論理プレーンから物理プレーンへ
				spt->x = -spt->x;
				spt->y = -spt->y;
				break;
			case W2:
				spt->p_plane_tbl = (u8)REV_P_Plane_tbl[spt->l_plane_tbl];	//面の反対側参照 物理プレーンから物理プレーンへ
				spt->x = -spt->x;
				spt->y = spt->y;
				break;
			case W3:
				spt->p_plane_tbl = (u8)REV_P_Plane_tbl[spt->l_plane_tbl];	//面の反対側参照 物理プレーンから物理プレーンへ
				spt->x = spt->x;
				spt->y = -spt->y;
				break;
			}
			break;

		case URA:
			switch (spt->way)
			{
			case W0:
				spt->p_plane_tbl = ( enum P_Plane_tbl )spt->l_plane_tbl;
				spt->x = spt->x;
				spt->y = spt->y;
				break;
			case W1:
				spt->p_plane_tbl = ( enum P_Plane_tbl )spt->l_plane_tbl;
				spt->x = -spt->x;
				spt->y = -spt->y;
				break;
			case W2:
				spt->p_plane_tbl = (u8)REV_P_Plane_tbl[spt->l_plane_tbl];//面の反対側参照
				spt->x = -spt->x;
				spt->y = spt->y;
				break;
			case W3:
				spt->p_plane_tbl = (u8)REV_P_Plane_tbl[spt->l_plane_tbl];//面の反対側参照
				spt->x = spt->x;
				spt->y = -spt->y;
				break;
			}
			break;
		}
	}

	// 透過のプレーン
	//trace_infoには光が通過した順番をが入る
	else if(pbs->PlaneInfo[spt->l_plane_tbl].Ref_or_Trans == TRANSMISSION)
	{
		spt->p_plane_tbl = ( enum P_Plane_tbl )spt->l_plane_tbl;	//論理プレーンから物理プレーンへ

		switch (pbs->PlaneInfo[spt->p_plane_tbl].note_scan_side)	//媒体を光が通過した順
		{
		case UP://センサーの位置を表すUP
			switch (spt->way)
			{
			case W0:
				spt->trace_info = O2U;
				spt->x = spt->x;
				spt->y = spt->y;
				break;

			case W1:
				spt->trace_info = O2U;
				spt->x = -spt->x;
				spt->y = -spt->y;
				break;

			case W2:
				spt->trace_info = U2O;
				spt->x = -spt->x;
				spt->y = spt->y;
				break;

			case W3:
				spt->trace_info = U2O;
				spt->x = spt->x;
				spt->y = -spt->y;
				break;

			}
			break;

		case DOWN:
			switch (spt->way)
			{
			case W0:
				spt->trace_info = U2O;
				spt->x = spt->x;
				spt->y = spt->y;
				break;
			case W1:
				spt->trace_info = U2O;
				spt->x = -spt->x;
				spt->y = -spt->y;
				break;
			case W2:
				spt->trace_info = O2U;
				spt->x = -spt->x;
				spt->y = spt->y;
				break;
			case W3:
				spt->trace_info = O2U;
				spt->x = spt->x;
				spt->y = -spt->y;
				break;
			}
			break;
		}

	}
	//MAGの場合
	//センサーで受像した媒体面を書き込む
	else
	{
		spt->p_plane_tbl = ( enum P_Plane_tbl )spt->l_plane_tbl;

		switch (pbs->PlaneInfo[spt->p_plane_tbl].note_scan_side)
		{
		case UP:
			switch (spt->way)
			{
			case W0:
				spt->trace_info = OMOTE;
				spt->x = spt->x;
				spt->y = spt->y;
				break;

			case W1:
				spt->trace_info = OMOTE;
				spt->x = -spt->x;
				spt->y = -spt->y;
				break;

			case W2:
				spt->trace_info = URA;
				spt->x = -spt->x;
				spt->y = spt->y;
				break;

			case W3:
				spt->trace_info = URA;
				spt->x = spt->x;
				spt->y = -spt->y;
				break;

			}
			break;

		case DOWN:

			switch (spt->way)
			{
			case W0:
				spt->trace_info = URA;
				spt->x = spt->x;
				spt->y = spt->y;
				break;
			case W1:
				spt->trace_info = URA;
				spt->x = -spt->x;
				spt->y = -spt->y;
				break;
			case W2:
				spt->trace_info = OMOTE;
				spt->x = -spt->x;
				spt->y = spt->y;
				break;
			case W3:
				spt->trace_info = OMOTE;
				spt->x = spt->x;
				spt->y = -spt->y;
				break;
			}
			break;

		}
	}

}





#endif
