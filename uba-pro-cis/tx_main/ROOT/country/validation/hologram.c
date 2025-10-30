//****************************************************************************/
//*                                                                          */
//*                                                                          */
//*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
//*  ALL RIGHTS RESERVED                                                     */
//*                                                                          */
//****************************************************************************/
//*                                                                          */
//* This software contains proprietary, trade secret information and is      */
//* the property of Japan Cash Machine. This software and the information    */
//* contained therein may not be disclosed, used, transferred or             */
//* copied in whole or in part without the express, prior written            */
//* consent of Japan Cash Machine.                                           */
//*                                                                          */
//****************************************************************************/
//****************************************************************************/
//*                                                                          */
//* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
//* 企業機密情報含んでいます。                                               */
//* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
//* 公開も複製も行いません。                                                 */
//*                                                                          */
//****************************************************************************/
//****************************************************************************/
//**
// * @file	hologram.c
// * @brief	ホログラム検知　ver1.3
// * @date	2023/01/17 Created.
// * @Version	0.1.0
// * @updata	https://app.box.com/file/1117363630533
// */
//****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define EXT
#include "../common/global.h"

#ifdef VS_DEBUG
EXTERN	int debug_logi_view;	//トレースするかしないか
#endif

/*

ver1.0	初回

ver1.1	テンプレート項目追加

ver1.11	修正

ver1.12 個別デバッグ表示ができるように
テンプレートパラメタ項目修正対応
TIR裏で検知できるように

ver1.2 角折れ・裂けの誤検知に対応した
テンプレート構造は変更しない

ver1.21 レベル判定追加

ver1.3 レベル判定の下限にパラメタ設定できるようにした 2023/01/17

*/

/****************************************************************/
/**
* @brief		ホログラムを検知する
*@param[in]		buf		バッファ番号
				holo	ホログラム検知構造体
*@param[out]	judge	判定結果
* @return		0
*/
/****************************************************************/
s16 hologram(u8 buf, ST_HOLOGRAM* holo)
{
	s16 res;
	u16 ii;

	u8 step = holo->search_area_step;
	u8 margin = holo->margin;
	u8 sumhisper_thr = holo->sumhisper_thr;
	u8 sumhisnum_thr = holo->sumhisnum_thr;
	float minthr = (float)holo->para[0].search_type;

	s16 y_current;
	s16 x_current;
	s16 y_start;
	s16 y_end;
	s16 x_start;
	s16 x_end;
	u16 count = 0;
	float sum = 0;
	u16 his[HOLO_HIS_LIMIT] = { 0 };
	u8 sumhiscount_per_totalcount[HOLO_HIS_LIMIT] = { 0 };

	ST_SPOINT spt = { 0 };
	ST_VERTEX ver = { 0 };
	u8 plane;
	float temp1;
	float temp2;

	plane = UP_T_IR1;
	if (work[buf].pbs->PlaneInfo[plane].Enable_or_Disable == PLANE_DISABLED)
	{
		plane = DOWN_T_IR1;
	}

	spt.l_plane_tbl = plane;
	spt.p_plane_tbl = plane;
	spt.way = work[buf].pbs->insertion_direction;

	//頂点論理座標
	get_vertex(&ver, buf);

	//探索範囲設定
	y_start = holo->para[0].y2;
	y_end = holo->para[0].y1;
	x_start = holo->para[0].x1;
	x_end = holo->para[0].x2;

	//スレッドのホログラムの場合、外形からy軸設定
	if (holo->outline_flag == 1)
	{
		y_start = ver.left_up_y - margin;
		y_end = ver.left_down_y + margin;
	}

	//パラメタ設定範囲が紙幣範囲外なら、紙幣外形からx軸設定
	if (x_end > ver.right_up_x)
	{
		x_end = ver.right_up_x - margin;
	}
	if (x_start < ver.left_up_x)
	{
		x_start = ver.left_up_x + margin;
	}

#ifdef VS_DEBUG
	//デバッグ用
	if (debug_logi_view != 0)
	{
		//設定範囲表示
		if (work[buf].pbs->LEorSE == SE)
		{
			deb_para[0] = 2;
			deb_para[1] = x_start + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
			deb_para[2] = work[buf].note_param.main_eff_range / 2 - y_start;
			deb_para[3] = x_end + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
			deb_para[4] = work[buf].note_param.main_eff_range / 2 - y_start;
			deb_para[5] = x_end + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
			deb_para[6] = work[buf].note_param.main_eff_range / 2 - y_end;
			deb_para[7] = x_start + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
			deb_para[8] = work[buf].note_param.main_eff_range / 2 - y_end;
		}
		else
		{
			deb_para[0] = 2;
			deb_para[1] = x_start + work[buf].note_param.main_eff_range / 2;
			deb_para[2] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_start;
			deb_para[3] = x_end + work[buf].note_param.main_eff_range / 2;
			deb_para[4] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_start;
			deb_para[5] = x_end + work[buf].note_param.main_eff_range / 2;
			deb_para[6] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_end;
			deb_para[7] = x_start + work[buf].note_param.main_eff_range / 2;
			deb_para[8] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_end;
		}

		callback(deb_para);

		deb_para[0] = 0;
		deb_para[1] = 0;
		deb_para[2] = 0;
		deb_para[3] = 0;
		deb_para[4] = 0;
		deb_para[5] = 0;
		deb_para[6] = 0;
		deb_para[7] = 0;
		deb_para[8] = 0;
	}
#endif

	//ヒストグラム作成
	for (x_current = x_start; x_current < x_end; x_current += step)
	{
		for (y_current = y_start; y_current > y_end; y_current -= step)
		{
			spt.x = x_current;
			spt.y = y_current;
			res = new_L_Getdt(&spt, work[buf].pbs, &work[buf].note_param);

			sum += spt.sensdt;
			his[spt.sensdt]++;
			count++;
		}
	}

	holo->tir_ave = (float)sum / count;

	sum = 0;

	for (ii = 0; ii < HOLO_HIS_LIMIT; ii++)
	{
		sum += his[ii];
		sumhiscount_per_totalcount[ii] = (u8)((sum / count) * 100);
	}

	holo->count = count;

	//判定
	if (sumhiscount_per_totalcount[sumhisnum_thr] > sumhisper_thr)
	{
		holo->judge = HOLO_OK;
	}
	else
	{
		holo->judge = HOLO_NG;
	}

	holo->sumhiscount_per_totalcount = sumhiscount_per_totalcount[sumhisnum_thr];

	//level
	temp1 = (float)(100 - sumhiscount_per_totalcount[sumhisnum_thr]);
	temp2 = (float)(100 - sumhisper_thr);
	holo->level = level_detect(&temp1, &temp2, 1, minthr, 100.0f);



#ifdef VS_DEBUG
	FILE* fp;
	if (work[buf].pbs->blank3 != 0)
	{
		fp = fopen("hologram.csv", "a");
		fprintf(fp, "%s,%x,%d,%s,", work[buf].pbs->blank4, work[buf].pbs->mid_res_nn.result_jcm_id, work[buf].pbs->insertion_direction, work[buf].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf].pbs->blank0[20], work[buf].pbs->ser_num1, work[buf].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf].pbs->spec_code.model_calibration, work[buf].pbs->spec_code.sensor_conf, work[buf].pbs->spec_code.mode);
		fprintf(fp, "%f,%d,%d,", holo->tir_ave,sumhiscount_per_totalcount[sumhisnum_thr],holo->level);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif

	return 0;

}

//End of file
