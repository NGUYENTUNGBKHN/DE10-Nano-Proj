
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXT
#define cod_max(a, b) (a > b ? a : b)
#include "../common/global.h"
EXTERN int debug_logi_view;	//トレースするかしないか
#include "cod_config.h"	//COD_Parameters
//#include "printf_enable_disable.h"
#include "cod_getseries.h"
#include "cod_getcolor.h"

#include "cod_malloc.h"

#include "JPY_weights/cod_parameters_BAU_LE17_JPY.h"

//void get_cutout_img2(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, u8* matrix, s16 start_x, s16 start_y, s16 width, s16 height, u8 plane[], u8 plane_num);
//void test_get_cutout_img2(ST_BS* pbs);

typedef	unsigned char	u8;
u8* matrix_temp_img0;
u8* matrix_temp_img1;
int* matrix_temp_img_int;
float* matrix_temp_float0;
float* matrix_temp_float1;
float* matrix_temp_float2;
unsigned char* matrix_temp_width;
unsigned char* matrix_temp_height;

void ocr_test_1(u8 buf_num, char* result, float* result_prob, void* st_cod)
{
	//const COD_Parameters *para2 = &JPY_COD_Parameters[0];
	COD_Parameters* para2 = (COD_Parameters*)st_cod;
	s16 start_x;// = para->starting_x; // -585
	s16 start_y;// = para->starting_y;  // -120
	s16 wid;// = para->width;
	s16 hid;// = para->height;
	u8 plane[1];
	u8 plane_num = sizeof(plane);
	u8 tmp = 0;
	u8* x_matrix = 0;	//1次元　ｘ　2次元ｙ　3次元プレーン
	u8* tmp_matrix;
	u16 i = 0, j = 0;
	u8 r;
	u8 window;
	int len;

	start_x = (s16)para2->starting_x; // -585
	start_y = (s16)para2->starting_y;  // -120

	wid = (s16)para2->width;
	hid = (s16)para2->height;

	if (start_x == 0)
	{
		return;
	}

	if (para2->rotate)
	{
		hid = (s16)para2->width;
		wid = (s16)para2->height;
	}

	if (para2->color == 'G')
	{
		plane[0] = (para2->side == 'U') ? UP_R_G : DOWN_R_G;
	}
	else
	{
		plane[0] = (para2->side == 'U') ? UP_R_R : DOWN_R_R;
	}

	//メモリの確保
	//x_matrix = (u8*)malloc_char(sizeof(u8) * plane_num * hid * wid);

	// Huong add 20220719 tạo mảng động trung gian
	matrix_temp_img0 = (u8*)malloc_char(sizeof(u8) * hid * wid);
	matrix_temp_img1 = (u8*)malloc_char(sizeof(u8) * hid * wid);

	r = 11;
	window = r + r + 1;
	matrix_temp_img_int = (int*)malloc_int((hid + window) * (wid + window));

	len = cod_max(wid, hid);
	matrix_temp_float0 = malloc_float(len + 101 + 101 + 1);
	matrix_temp_float1 = malloc_float(len);
	matrix_temp_float2 = malloc_float(len);

	matrix_temp_width = (unsigned char*)malloc_char(wid);
	matrix_temp_height = (unsigned char*)malloc_char(hid);

	x_matrix = &(*matrix_temp_img0);
	//処理実行
	//start_time(pro6);		//処理時間測定　開始
	ocr_get_cutout_img(&work[buf_num].note_param, work[buf_num].pbs, buf_num, x_matrix, start_x, start_y, wid, hid, plane, plane_num, 1, 1);
	//end_time(pro6 ,NULL);	//処理時間測定　終了

	//test_get_cutout_img2(work[buf_num].pbs);


	//回転
	if (para2->rotate)
	{
		// đã sửa malloc này
		//tmp_matrix = (u8*)malloc_char(sizeof(u8) * plane_num * hid * wid);
		tmp_matrix = &(*matrix_temp_img1);

		for (i = 0; i < hid; i++)
		{
			for (j = 0; j < wid; j++)
			{
				tmp_matrix[i + hid * j] = x_matrix[i * (wid - 0) + j];
			}
		}

		memcpy(x_matrix, tmp_matrix, (sizeof(u8) * hid * wid));
		//free_proc(tmp_matrix);

		wid = (s16)para2->width;
		hid = (s16)para2->height;
	}


	//反転ｘ
	if (para2->flip_x)
	{
		for (i = 0; i < hid; i++)
		{
			for (j = 0; j < wid / 2; j++)
			{
				tmp = (u32)x_matrix[i * wid + j];
				x_matrix[i * wid + j] = x_matrix[i * wid + wid - j - 1];
				x_matrix[i * wid + wid - j - 1] = (u8)tmp;
			}
		}
	}

	//反転ｙ
	if (para2->flip_y)
	{
		for (i = 0; i < hid / 2; i++)
		{
			for (j = 0; j < wid; j++)
			{
				tmp = (u32)x_matrix[i * wid + j];
				x_matrix[i * wid + j] = x_matrix[((hid - 1 - i) * wid) + j];
				x_matrix[((hid - 1 - i) * wid) + j] = (u8)tmp;
			}
		}
	}

	cod_getseries(x_matrix, para2, result, result_prob);	// Huong add 20210820
	//free_proc(x_matrix);
	free_proc(matrix_temp_img0);
	free_proc(matrix_temp_img1);
	free_proc(matrix_temp_img_int);
	free_proc(matrix_temp_float0);
	free_proc(matrix_temp_float1);
	free_proc(matrix_temp_float2);
	free_proc(matrix_temp_width);
	free_proc(matrix_temp_height);

}

void ocr_color(u8 buf_num, char* result, char* color_detect, void* st_cod)
{

	//const COD_Parameters *para = &JPY_COD_Parameters[denomination];
	const COD_Parameters* para = (COD_Parameters*)st_cod;

	//ここのパラメータを設定することで様々な部分を切り出すことができます。
	s16 start_x;// = para->starting_x; // -585
	s16 start_y;// = para->starting_y;  // -120
	s16 wid;// = para->width;
	s16 hid;// = para->height;


	u8 plane_r[1];// add by hoan
	u8 plane_g[1];// add by hoan
	u8 plane_b[1];// add by hoan


	u8 plane_num_r = sizeof(plane_r);
	u8 plane_num_g = sizeof(plane_g);
	u8 plane_num_b = sizeof(plane_b);

	//格納用配列

	u8* r_matrix, * b_matrix, * g_matrix;	//1次元　ｘ　2次元ｙ　3次元プレーン
	//COD_Parameters* st_coD = (COD_Parameters*)st_cod;

	s32 count_0 = 0;
	s32 count_1 = 0;
	s32 count_2 = 0;
	s32 ii = 0;
	s32 length_result = 0;
	u8 r = 11;
	u8 window = r + r + 1;
	s32 len = 0;

	start_x = (s16)para->starting_x; // -585
	start_y = (s16)para->starting_y;  // -120
	wid = (s16)para->width;
	hid = (s16)para->height;
	len = cod_max(wid, hid);

	if (work[buf_num].pbs->insertion_direction == 0 || work[buf_num].pbs->insertion_direction == 1)
	{
		plane_r[0] = (para->side == 'U') ? UP_R_R : DOWN_R_R;
		plane_g[0] = (para->side == 'U') ? UP_R_G : DOWN_R_G;
		plane_b[0] = (para->side == 'U') ? UP_R_B : DOWN_R_B;
	}
	else
	{
		plane_r[0] = (para->side == 'U') ? UP_R_R : DOWN_R_R;
		plane_g[0] = (para->side == 'U') ? UP_R_G : DOWN_R_G;
		plane_b[0] = (para->side == 'U') ? UP_R_B : DOWN_R_B;
	}

	r_matrix = (u8*)malloc_char(sizeof(u8) * plane_num_r * hid * wid);
	g_matrix = (u8*)malloc_char(sizeof(u8) * plane_num_g * hid * wid);
	b_matrix = (u8*)malloc_char(sizeof(u8) * plane_num_b * hid * wid);


	matrix_temp_float0 = malloc_float(len + 101 + 101 + 1);
	matrix_temp_float1 = malloc_float(len);
	matrix_temp_float2 = malloc_float(len);
	matrix_temp_img_int = (int*)malloc_int((hid + window) * (wid + window));
	matrix_temp_img0 = (u8*)malloc_char(sizeof(u8) * hid * wid);
	matrix_temp_img1 = (u8*)malloc_char(sizeof(u8) * hid * wid);
	matrix_temp_width = (u8*)malloc_char(wid);
	matrix_temp_height = (u8*)malloc_char(hid);

	ocr_get_cutout_img(&work[buf_num].note_param, work[buf_num].pbs, buf_num, r_matrix, start_x, start_y, wid, hid, plane_r, plane_num_r, 1, 1);
	ocr_get_cutout_img(&work[buf_num].note_param, work[buf_num].pbs, buf_num, g_matrix, start_x, start_y, wid, hid, plane_g, plane_num_g, 1, 1);
	ocr_get_cutout_img(&work[buf_num].note_param, work[buf_num].pbs, buf_num, b_matrix, start_x, start_y, wid, hid, plane_b, plane_num_b, 1, 1);

	cod_getcolor(para, result, r_matrix, g_matrix, b_matrix);

	length_result = (s32)(strlen(result));

	for (ii = 0; ii < length_result; ii++)
	{
		if (result[ii] == '0')
		{
			count_0++;
		}
		else if (result[ii] == '1')
		{
			count_1++;
		}
		else if (result[ii] == '2')
		{
			count_2++;
		}
	}

	if (count_0 >= length_result / 2)
	{
		color_detect[0] = '1';	//黒
	}
	else if (count_1 >= length_result / 2)
	{
		color_detect[0] = '2';	//茶
	}
	else
	{
		color_detect[0] = '3';	//青
	}

	free_proc(r_matrix);
	free_proc(g_matrix);
	free_proc(b_matrix);

	free_proc(matrix_temp_img0);
	free_proc(matrix_temp_img1);
	free_proc(matrix_temp_img_int);
	free_proc(matrix_temp_float0);
	free_proc(matrix_temp_float1);
	free_proc(matrix_temp_float2);
	free_proc(matrix_temp_width);
	free_proc(matrix_temp_height);
}

/****************************************************************/
/**
*
* @brief		指定された範囲のイメージを切り出します。
*
* @param[in]	pnote_param	座標変換用パラメータ
*				pbs			イメージデータのポインタ
*				buf_num		バッファ番号
*				start_x		スタートx座標（左上）
*				start_y		スタートy座標（左上）
*				width		切り出す幅
*				height		切り出す高さ
*				plane		プレーン
*				plane_num	プレーンの数
*				x_interval	切り出し時のdotの間隔（1=200dpi, 2=100dpi）
*				y_interval
*
* @param[out]	matrix		切り出したイメージ
* @return		なし
*
*/
/****************************************************************/
void ocr_get_cutout_img(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, u8* matrix, s16 start_x, s16 start_y, s16 width, s16 height, u8 plane[], u8 plane_num, u8 x_interval, u8 y_interval)//デバック用パラメタ設定関数
{
#define GETDT_ROUNDING 0.5f
	s16 x = 0;
	s16 y = 0;
	s16 p = 0;

	float sin_x, cos_x, sin_y, cos_y;
	u32 period;
	u8 pro_plane;
	float constant_x, constant_y;
	float input_x, input_y;
	float midst_res_x;
	float midst_res_y;
	float current_sin_x = 0;
	float current_cos_x = 0;
	float current_sin_y = 0;
	float current_cos_y = 0;
	float current_res_x = 0;
	float current_res_y = 0;
	float insart_multi_x = 0;
	float insart_multi_y = 0;
	float temp_x_interval = 0;
	float temp_y_interval = 0;

	u32 i = 0;
	u32 index;

#ifdef VS_DEBUG
	u32 debugx = 0;
	u32 debugy = 0;
#endif

	//buf_num = buf_num;
	//デバッグ用---------------------------
	//FILE* fp;
	//F_CREATE_BIN(fp)
	//-------------------------------------

	for (p = 0; p < plane_num; ++p)
	{
		//プレーン情報ごとのパラメタ
		pro_plane = (u8)pnote_param->pplane_tbl[plane[p]];
		constant_x = pnote_param->coordinate_param_x[pro_plane];
		constant_y = pnote_param->coordinate_param_y[pro_plane];
		period = pbs->PlaneInfo[pro_plane].Address_Period;
		sin_x = pnote_param->sin_x[pro_plane];
		cos_x = pnote_param->cos_x[pro_plane];
		sin_y = pnote_param->sin_y[pro_plane];
		cos_y = pnote_param->cos_y[pro_plane];
		insart_multi_x = pnote_param->insart_multi_x[pro_plane];
		insart_multi_y = pnote_param->insart_multi_y[pro_plane];

		input_x = start_x * insart_multi_x;
		input_y = start_y * insart_multi_y;

		temp_x_interval = x_interval * insart_multi_x;
		temp_y_interval = y_interval * insart_multi_y;

		for (y = 0; y < (height); y = y + y_interval)
		{
			current_cos_y = input_y * cos_y;
			current_sin_x = input_y * sin_x;
			for (x = 0; x < (width); x = x + x_interval)
			{
				current_cos_x = input_x * cos_x;
				current_sin_y = input_x * sin_y;

				midst_res_x = (current_cos_x + current_sin_x);
				midst_res_y = -(current_cos_y - current_sin_y);

				current_res_x = constant_x - midst_res_x + GETDT_ROUNDING;
				current_res_y = (midst_res_y + constant_y) + GETDT_ROUNDING;

				if (pbs->LEorSE == LE)
				{
					index = (u32)current_res_x + (u32)current_res_y * period;
				}
				else
				{
					index = (u32)current_res_y + (u32)current_res_x * period;		
				}

#ifdef VS_DEBUG	
				if (pbs->LEorSE == LE)
				{
					if (debug_logi_view == 1)
					{
						debugx = start_x + x + pnote_param->main_eff_range / 2;	//
						debugy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (start_y - y);	//

						deb_para[0] = 1;
						deb_para[1] = debugx;
						deb_para[2] = debugy;
						deb_para[3] = 1;
						deb_para[4] = pro_plane;
						callback(deb_para);
					}
				}
				else
				{
					if (debug_logi_view == 1)
					{
						debugx = start_x + x + (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2;
						debugy = pnote_param->main_eff_range / 2 - (start_y - y);	//

						deb_para[0] = 1;
						deb_para[1] = debugx;
						deb_para[2] = debugy;
						deb_para[3] = 1;
						deb_para[4] = pro_plane;
						callback(deb_para);
					}
				}
#endif	
				/* 取得したセンサデータを格納 */
				matrix[i++] = pbs->sens_dt[index];
				input_x = input_x + temp_x_interval;
			}

			input_y = input_y - temp_y_interval;
			input_x = start_x * insart_multi_x;
		}
	}

	return;


}//記番号

/****************************************************************/
/**
*
* @brief		指定された範囲のイメージを切り出します。
*				また、切り出し時のマスクを指定することができます。
*				格納するのはマスクの範囲のみで、マスクの余白部分は配列には格納しません。
*
* @param[in]	pnote_param	座標変換用パラメータ
*				pbs			イメージデータのポインタ
*				buf_num		バッファ番号
*				start_x		スタートx座標（左上）
*				start_y		スタートy座標（左上）
*				width		切り出す幅
*				height		切り出す高さ
*				plane		プレーン
*				plane_num	プレーンの数
*				x_interval	切り出し時のdotの間隔（1=200dpi, 2=100dpi）
*				y_interval
*				mask_patten	切り出しに対するマスクパターンの配列　要素が0の場合はスキップする
*				mask_num	マスクパターン配列の要素数
*
* @param[out]	matrix		切り出したイメージ
* @return		なし
*
*/
/****************************************************************/
//void ocr_get_cutout_img (ST_NOTE_PARAMETER* pnote_param,  ST_BS* pbs ,u8 buf_num ,u8 *** matrix ,s16 start_x, s16 start_y ,s16 width, s16 height ,u8 plane[] ,u8 plane_num)//デバック用パラメタ設定関数
u16 ocr_get_cutout_img_with_maskpatten(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, u8* matrix, s16 start_x, s16 start_y, s16 width, s16 height, u8 plane[], u8 plane_num, u8 x_interval, u8 y_interval, u8* mask_patten, u8 mask_num)//デバック用パラメタ設定関数
{

	s16 x = 0;
	s16 y = 0;
	s16 p = 0;
	u16 mask_counter = 0;

	//	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	//	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	float sin_x, cos_x, sin_y, cos_y;
	u32 period;
	//u32 offset; // add
	u8 pro_plane;
	float constant_x, constant_y;
	float input_x, input_y;
	float midst_res_x;
	float midst_res_y;
	float current_sin_x = 0;
	float current_cos_x = 0;
	float current_sin_y = 0;
	float current_cos_y = 0;
	float current_res_x = 0;
	float current_res_y = 0;
	float insart_multi_x = 0;
	float insart_multi_y = 0;
	float temp_x_interval = 0;
	float temp_y_interval = 0;


	u32 i = 0;

	u32 index;
#ifdef VS_DEBUG
	u32 debugx = 0;
	u32 debugy = 0;
#endif
	//buf_num = buf_num;
	//デバッグ用---------------------------
	//FILE* fp;
	//F_CREATE_BIN(fp)
	//-------------------------------------

	//プレーン情報とは関係のないパラメタ
	//start_x = start_x * *pnote_param->insart_multi_x;
	//start_y = start_y * *pnote_param->insart_multi_y


	if (pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
	{
		for (p = 0; p < plane_num; ++p)
		{
			//プレーン情報ごとのパラメタ
			pro_plane = (u8)pnote_param->pplane_tbl[plane[p]];
			constant_x = pnote_param->coordinate_param_x[pro_plane];
			constant_y = pnote_param->coordinate_param_y[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			//offset = pbs->PlaneInfo[pro_plane].Address_Offset;// add
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];
			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];
			insart_multi_x = pnote_param->insart_multi_x[pro_plane];
			insart_multi_y = pnote_param->insart_multi_y[pro_plane];


			input_x = start_x * insart_multi_x;
			input_y = start_y * insart_multi_y;

			temp_x_interval = x_interval * insart_multi_x;
			temp_y_interval = y_interval * insart_multi_y;

			//temp_x_interval = insart_multi_x;
			//temp_y_interval = insart_multi_y;

			for (y = 0; y < (height); y = y + y_interval)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (width); x = x + x_interval)
				{

					if (mask_patten[mask_counter] != 0)	//マスクの要素数が0以外の時計算する
					{

						current_cos_x = input_x * cos_x;
						current_sin_y = input_x * sin_y;

						midst_res_x = (current_cos_x + current_sin_x);
						midst_res_y = -(current_cos_y - current_sin_y);

						current_res_x = constant_x - midst_res_x;
						current_res_y = (midst_res_y + constant_y);


						index = (u32)current_res_x + (u32)current_res_y * period;

						///* 取得したセンサデータを格納 */
						//matrix[p][y][x] = pbs->sens_dt[index];

#ifdef VS_DEBUG
					//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
						if (pbs->LEorSE == LE)  //SEならば
						{
							if (debug_logi_view == 1)
							{
								debugx = start_x + x + pnote_param->main_eff_range / 2;	//
								debugy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (start_y - y);	//

								deb_para[0] = 1;		// function code
								deb_para[1] = debugx;
								deb_para[2] = debugy;//(((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (debugy -y);	//
								deb_para[3] = 1;		// plane
								callback(deb_para);		// debug
							}
						}
#endif
						matrix[i++] = pbs->sens_dt[index];
					}

					input_x = input_x + temp_x_interval;
					mask_counter++;
				}

				input_y = input_y - temp_y_interval;
				input_x = start_x * insart_multi_x;
			}
		}
		//F_WRITE_S(fp, matrix)
		//F_CLOSE(fp)

	}
	else  //座標変換　+　画素参照
	{
		for (p = 0; p < plane_num; ++p)
		{
			//プレーン情報ごとのパラメタ
			pro_plane = (u8)pnote_param->pplane_tbl[plane[p]];
			constant_x = pnote_param->coordinate_param_x[pro_plane];
			constant_y = pnote_param->coordinate_param_y[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			//offset = pbs->PlaneInfo[pro_plane].Address_Offset;// add
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];
			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];
			insart_multi_x = pnote_param->insart_multi_x[pro_plane];
			insart_multi_y = pnote_param->insart_multi_y[pro_plane];


			input_x = start_x * insart_multi_x;
			input_y = start_y * insart_multi_y;

			//temp_x_interval = x_interval * insart_multi_x;
			//temp_y_interval = y_interval * insart_multi_y;

			temp_x_interval = x_interval * insart_multi_x;
			temp_y_interval = y_interval * insart_multi_y;

			for (y = 0; y < (height); y = y + y_interval)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (width); x = x + x_interval)
				{
					if (mask_patten[mask_counter] != 0)	//マスクの要素数が0以外の時計算する
					{
						current_cos_x = input_x * cos_x;
						current_sin_y = input_x * sin_y;

						midst_res_x = (current_cos_x + current_sin_x);
						midst_res_y = -(current_cos_y - current_sin_y);

						current_res_x = constant_x - midst_res_x;
						current_res_y = (midst_res_y + constant_y);

						index = (u32)current_res_y + (u32)current_res_x * period;

#ifdef VS_DEBUG
						//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
						if (pbs->LEorSE == SE)  //SEならば
						{
							if (debug_logi_view == 1)
							{
								debugy = start_y + y + pnote_param->main_eff_range / 2;	//
								debugx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (start_x - x);	//

								deb_para[0] = 1;		// function code
								deb_para[1] = debugx;
								deb_para[2] = debugy;//(((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (debugy -y);	//
								deb_para[3] = 1;		// plane
								callback(deb_para);		// debug
							}
						}
#endif
						//index = (u32)current_res_x + (u32)current_res_y * period;

						///* 取得したセンサデータを格納 */
						//matrix[p][y][x] = pbs->sens_dt[index];

						matrix[i++] = pbs->sens_dt[index];
					}

					input_x = input_x + temp_x_interval;
					mask_counter++;
				}
				input_y = input_y - temp_y_interval;
				input_x = start_x * insart_multi_x;
			}

		}
	}

	return i;


}//記番号

//void __inline get_cutout_img_v3(ST_NOTE_PARAMETER* __restrict__ pnote_param, ST_BS* __restrict__ pbs, u8 buf_num, u8* __restrict__ matrix, s16 start_x, s16 start_y, s16 width, s16 height, u8 plane[], u8 plane_num, s16 step_x, s16 step_y)
void get_cutout_img_v3(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, u8* matrix, s16 start_x, s16 start_y, s16 width, s16 height, u8 plane[], u8 plane_num, s16 step_x, s16 step_y)
{

	s16 x = 0;
	s16 y = 0;
	s16 p = 0;


	//	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	//	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	float sin_x, cos_x, sin_y, cos_y;
	u32 period;
	u8 pro_plane;
	float constant_x, constant_y;
	float input_x, input_y;
	float midst_res_x;
	float midst_res_y;
	float current_sin_x = 0;
	float current_cos_x = 0;
	float current_sin_y = 0;
	float current_cos_y = 0;
	float current_res_x = 0;
	float current_res_y = 0;

	u32 index;
	u32 i = 0;

	start_x = start_x * *pnote_param->insart_multi_x;
	start_y = start_y * *pnote_param->insart_multi_y;


	if (pbs->LEorSE == LE)
	{
		for (p = 0; p < plane_num; ++p)
		{

			pro_plane = pnote_param->pplane_tbl[plane[p]];
			constant_x = pnote_param->coordinate_param_x[pro_plane];
			constant_y = pnote_param->coordinate_param_y[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];
			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];

			input_x = start_x;
			input_y = start_y;

			for (y = 0; y < (height); ++y)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (width); ++x)
				{
					current_cos_x = input_x * cos_x;
					current_sin_y = input_x * sin_y;

					midst_res_x = (current_cos_x + current_sin_x);
					midst_res_y = -(current_cos_y - current_sin_y);

					current_res_x = constant_x - midst_res_x;
					current_res_y = (midst_res_y + constant_y);


					index = (u32)current_res_x + (u32)current_res_y * period;

					//					if ((int)(&matrix[i]) > 1899935064) {
					//						printf("add %d %d %x\n", i, &matrix[i], &matrix[i]);
					//					}
					matrix[i++] = pbs->sens_dt[index];
					//					buf_data[buf_index++] = pbs->sens_dt[index];

					input_x += *pnote_param->insart_multi_x * step_x;
				}
				input_y -= *pnote_param->insart_multi_y * step_y;
				input_x = start_x;
			}
		}

	}
	else
	{
		for (p = 0; p < plane_num; ++p)
		{

			pro_plane = pnote_param->pplane_tbl[plane[p]];
			constant_x = pnote_param->coordinate_param_x[pro_plane];
			constant_y = pnote_param->coordinate_param_y[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];
			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];

			input_x = start_x;
			input_y = start_y;

			for (y = 0; y < (height); ++y)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (width); ++x)
				{
					current_cos_x = input_x * cos_x;
					current_sin_y = input_x * sin_y;

					midst_res_x = (current_cos_x + current_sin_x);
					midst_res_y = -(current_cos_y - current_sin_y);

					current_res_x = constant_x - midst_res_x;
					current_res_y = (midst_res_y + constant_y);

					matrix[i++] = pbs->sens_dt[(u32)current_res_y + (u32)current_res_x * period];

					input_x += *pnote_param->insart_multi_x;
				}
				input_y -= *pnote_param->insart_multi_y;
				input_x = start_x;
			}

		}
	}
}



#ifdef Aime




void get_cutout_img2(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, u8* matrix, s16 start_x, s16 start_y, s16 width, s16 height, u8 plane[], u8 plane_num)
{

	s16 x = 0;
	s16 y = 0;
	s16 p = 0;


	//	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	//	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	float sin_x, cos_x, sin_y, cos_y;
	u32 period;
	u32 offset; // add
	u8 pro_plane;
	float constant_x, constant_y;
	float input_x, input_y;
	float midst_res_x;
	float midst_res_y;
	float current_sin_x = 0;
	float current_cos_x = 0;
	float current_sin_y = 0;
	float current_cos_y = 0;
	float current_res_x = 0;
	float current_res_y = 0;
	u32 i = 0;
	u32 index;
	FILE* fp;
	F_CREATE_BIN(fp);




	if (pbs->LEorSE == LE)
	{
		for (p = 0; p < plane_num; ++p)
		{

			pro_plane = pnote_param->pplane_tbl[plane[p]];
			constant_x = pnote_param->coordinate_param_x_non_ofs[pro_plane];
			constant_y = pnote_param->coordinate_param_y[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			offset = pbs->PlaneInfo[pro_plane].Address_Offset;
			//printf("period: %d %d %d\n", period, pro_plane, offset);
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];			//offset = pbs->PlaneInfo[pro_plane].Address_Offset;// add

			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];
			input_x = start_x * pnote_param->insart_multi_x[plane[0]];
			input_y = start_y * pnote_param->insart_multi_y[plane[0]];


			//input_x = start_x;
			//input_y = start_y;

			for (y = 0; y < (height); ++y)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (width); ++x)
				{
					current_cos_x = input_x * cos_x;
					current_sin_y = input_x * sin_y;

					midst_res_x = (current_cos_x + current_sin_x);
					midst_res_y = -(current_cos_y - current_sin_y);

					current_res_x = constant_x - midst_res_x;
					current_res_y = (midst_res_y + constant_y);


					index = (u32)current_res_x + (u32)current_res_y * period + offset;// add offset

					//index = (u32)current_res_x + (u32)current_res_y * period + offset;


					//matrix[p][y][x] = pbs->sens_dt[index];
					matrix[i++] = pbs->sens_dt[index];
					//i++;

					input_x += pnote_param->insart_multi_x[plane[0]];
				}
				input_y -= pnote_param->insart_multi_y[plane[0]];
				input_x = start_x * pnote_param->insart_multi_x[plane[0]];
			}
		}

	}
	else
	{
		for (p = 0; p < plane_num; ++p)
		{

			pro_plane = pnote_param->pplane_tbl[plane[p]];
			constant_x = pnote_param->coordinate_param_x[pro_plane];
			constant_y = pnote_param->coordinate_param_y[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];
			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];

			input_x = start_x;
			input_y = start_y;

			for (y = 0; y < (height); ++y)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (width); ++x)
				{
					current_cos_x = input_x * cos_x;
					current_sin_y = input_x * sin_y;

					midst_res_x = (current_cos_x + current_sin_x);
					midst_res_y = -(current_cos_y - current_sin_y);

					current_res_x = constant_x - midst_res_x;
					current_res_y = (midst_res_y + constant_y);
					index = (u32)current_res_y + (u32)current_res_x * period;// add offset

					//matrix[p][y][x] = pbs->sens_dt[index];
					matrix[i++] = pbs->sens_dt[index];
					//i++;

					input_x += pnote_param->insart_multi_x[plane[0]];
				}
				input_y -= pnote_param->insart_multi_y[plane[0]];
				input_x = start_x;
			}

		}
	}
	//F_WRITE_S(fp, &pbs->sens_dt[index]);
	fwrite(matrix, i, 1, fp);
	F_CLOSE(fp);
}

void test_get_cutout_img2(ST_BS* pbs)
{
	int denomination = 0;
	u8 buf_num = 0;

	s16 start_x = -work[buf_num].pbs->note_x_size / 2;
	s16 start_y = work[buf_num].pbs->note_y_size / 2;
	//	s16 wid = work[buf_num].pbs->note_x_size - 1;
	s16 wid = work[buf_num].pbs->note_x_size;
	//	s16 wid = 500;
	s16 hid = work[buf_num].pbs->note_y_size;
	//	s16 hid = 400;
	//	s16 hid = 180;
	u8 plane[1];

	u8 plane_num = sizeof(plane);


	int label;
	u8 tmp;
	u8* buf;
	u8*** plane_matrix, ** y_matrix, * x_matrix;
	u16 y_matrix_size = 0;
	u16 i = 0, j = 0, p = 0;
	u16 c = 0;


#ifndef GETSERIES_V2
	plane[0] = 3;	//colo
#else
	// Color
	plane[0] = para->front_color;
	plane[0] = para->back_color;
	//	plane[1] = para->back_color;
#endif

	//plane_matrix = (u8***)malloc(sizeof(u8**) * plane_num);
	//y_matrix = (u8**)malloc(sizeof(u8*) * plane_num * hid);
	x_matrix = (u8*)malloc(sizeof(u8) * plane_num * hid * wid);

	get_cutout_img2(&work[buf_num].note_param, work[buf_num].pbs, buf_num, x_matrix, start_x, start_y, wid, hid, plane, 1);
	free_proc(x_matrix);

}

#endif // DEBUG

