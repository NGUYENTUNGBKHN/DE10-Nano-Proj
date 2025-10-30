#include	<math.h>
#include    <string.h>
#include    <float.h>
#include <stdlib.h>
#include	<stdio.h>
#define EXT
#include "../common/global.h"

/*
* 
* https://app.box.com/s/9bnecaoqt53p7i8p721b9k3i445iu5os
* 
2021/12/2
IR2なしで1番目のみ判定していたのを修正
デバッグ用にCSV出力できるように

2022/07/14
IR2波長差から変更
*/



s32 get_ir2wave_invalid_count(u8 buf_num, ST_IR2WAVE* ir2wave)
{
	u32 ir1his[256] = { 0 };	// IR1値のデータ格納
	u32 ir2his[256] = { 0 };	// IR2値のデータ格納
	u16 plane_side = 0;
	u16 plane_ir1 = 0;
	u16 plane_ir2 = 0;
	u16 point_number = 0;
	u32 feature = 0;
	u32 nonir[2] = { 0 };
	u32 all_cnt = 0;
	float propotion[2] = { 0 };

	s16 coordinate[4] = { 0 };
	u8 cnt = 0;
	u8 ir2wave_plane_tbl_ir1[2] = { OMOTE_R_IR1,URA_R_IR1 };
	u8 ir2wave_plane_tbl_ir2[2] = { OMOTE_R_IR2,URA_R_IR2 };

	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;

	s16 y = 0;
	s16 x = 0;
	s16 yp = 0;
	s16 xp = 0;
	float ir1_thr = 0;
	float ir2_thr = 0;
	u8 peak_search = 0;
	u16 ii = 0;
	u16 jj = 0;
	float thr_distance = 0;
	float ir1peak_ave = 0.0f;
	float ir1peak_std =	0.0f;
	float ir2peak_ave =	0.0f;
	float ir2peak_std =	0.0f;
	float ir1freq_ave =	0.0f;
	float ir1freq_std =	0.0f;
	float ir2freq_ave =	0.0f;
	float ir2freq_std =	0.0f;
	float corr[4][4];
	float ir1freqtemp = 0;
	float ir2freqtemp = 0;
	u8 ir1peak = 0;
	u8 ir2peak = 0;
	float movingave = 0;
	u8 temp = 0;
	u8 tmp1 = 0;
	u8 tmp2 = 0;
	float ir1peak_standard = 0;
	float ir2peak_standard = 0;
	float ir1freq_standard = 0;
	float ir2freq_standard = 0;
	float standard[4];
	float dis = 0;
	u8 tmplevel = 0;
	u8 tempcnt = 0;
	float level = 0;

	spt.way = work[buf_num].pbs->insertion_direction;		//方向設定

#ifdef VS_DEBUG
	FILE *fp;
	if (work[buf_num].pbs->blank3 != 0)
	{
		fp = fopen("ir2wave.csv", "a");
		fprintf(fp, "%s,%x,%d,%s,", work[buf_num].pbs->blank4, work[buf_num].pbs->mid_res_nn.result_jcm_id, work[buf_num].pbs->insertion_direction, work[buf_num].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf_num].pbs->blank0[20], work[buf_num].pbs->ser_num1, work[buf_num].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf_num].pbs->spec_code.model_calibration, work[buf_num].pbs->spec_code.sensor_conf, work[buf_num].pbs->spec_code.mode);
	}
#endif

	for (point_number = 0; point_number < ir2wave->point_number; point_number += 7)
	{
		if (ir2wave->ppoint[point_number].x_s > ir2wave->ppoint[point_number].x_e)
		{
			coordinate[0] = ir2wave->ppoint[point_number].x_e;
			coordinate[2] = ir2wave->ppoint[point_number].x_s;
		}
		else
		{
			coordinate[0] = ir2wave->ppoint[point_number].x_s;
			coordinate[2] = ir2wave->ppoint[point_number].x_e;
		}
		if (ir2wave->ppoint[point_number].y_s > ir2wave->ppoint[point_number].y_e)
		{
			coordinate[1] = ir2wave->ppoint[point_number].y_e;
			coordinate[3] = ir2wave->ppoint[point_number].y_s;
		}
		else
		{
			coordinate[1] = ir2wave->ppoint[point_number].y_s;
			coordinate[3] = ir2wave->ppoint[point_number].y_e;
		}

		feature = (u32)ir2wave->ppoint[point_number + 1].x_s;
		plane_side = (u16)ir2wave->ppoint[point_number + 1].x_e;
		plane_ir1 = ir2wave_plane_tbl_ir1[plane_side];
		plane_ir2 = ir2wave_plane_tbl_ir2[plane_side];

		ir2wave->feature[cnt] = feature;

		// IRなし判定が==2のとき入る。
		if (feature == 2)
		{
			ir1_thr = ir2wave->ppoint[point_number].threshold1;
			ir2_thr = ir2wave->ppoint[point_number].threshold2;
			peak_search = ir2wave->ppoint[point_number + 1].y_e;

			for (y = coordinate[1]; y <= coordinate[3]; y++)
			{
				for (x = coordinate[0]; x <= coordinate[2]; x++)
				{
					for (yp = y * ir2wave->ir1_mask_ptn_diameter_y; yp < (y * ir2wave->ir1_mask_ptn_diameter_y) + ir2wave->ir1_mask_ptn_diameter_y; yp++)
					{
						for (xp = x * ir2wave->ir1_mask_ptn_diameter_x; xp < (x * ir2wave->ir1_mask_ptn_diameter_x) + ir2wave->ir1_mask_ptn_diameter_x; xp++)
						{
							// IR1
							spt.x = xp;
							spt.y = yp;
							spt.l_plane_tbl = (u8)plane_ir1;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							ir1his[spt.sensdt]++;

							// IR2
							spt.x = xp;
							spt.y = yp;
							spt.l_plane_tbl = (u8)plane_ir2;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							ir2his[spt.sensdt]++;

							all_cnt++;
							xp++;
						}
					}
				}
			}

			for (ii = 0; ii < peak_search; ii++)
			{
				nonir[0] += ir1his[ii];
				nonir[1] += ir2his[ii];
			}

			if (nonir[0] == 0)
			{
				nonir[0] = 1;
			}
			if (nonir[1] == 0)
			{
				nonir[1] = 1;
			}

			propotion[0] = (float)nonir[0] / (float)all_cnt;
			propotion[1] = (float)nonir[1] / (float)all_cnt;

			//中間情報
			ir2wave->ir1pro[cnt] = propotion[0];
			ir2wave->ir2pro[cnt] = propotion[1];

#ifdef VS_DEBUG
			if (work[buf_num].pbs->blank3 != 0)
			{
				fprintf(fp, "%d,%f,%f,", feature, propotion[0], propotion[1]);
			}
#endif

			if (propotion[0] > ir1_thr &&
				propotion[1] < ir2_thr &&
				propotion[0] >= propotion[1])
			{
				ir2wave->output[cnt] = 90;
			}
			else
			{
				ir2wave->output[cnt] = 1;
			}

#ifdef VS_DEBUG
			if (work[buf_num].pbs->blank3 != 0)
			{
				fprintf(fp, "%d,", ir2wave->output[cnt]);//レベル出力
			}
#endif

			// 解放
			for (ii = 0; ii < 256; ii++)
			{
				ir1his[ii] = 0;
				ir2his[ii] = 0;
			}

			nonir[0] = 0;
			nonir[1] = 0;
			all_cnt = 0;
			propotion[0] = 0;
			propotion[1] = 0;
		}
		else//IR2反応なし以外
		{
			thr_distance = (float)ir2wave->ppoint[point_number + 6].threshold1;
			peak_search = (u8)ir2wave->ppoint[point_number + 1].y_e;

			ir1peak_ave = ir2wave->ppoint[point_number].threshold1;
			ir1peak_std = ir2wave->ppoint[point_number].threshold2;
			ir2peak_ave = ir2wave->ppoint[point_number].threshold3;
			ir2peak_std = ir2wave->ppoint[point_number].threshold4;
			ir1freq_ave = ir2wave->ppoint[point_number + 1].threshold1;
			ir1freq_std = ir2wave->ppoint[point_number + 1].threshold2;
			ir2freq_ave = ir2wave->ppoint[point_number + 1].threshold3;
			ir2freq_std = ir2wave->ppoint[point_number + 1].threshold4;

			corr[0][0] = ir2wave->ppoint[point_number + 2].threshold1;
			corr[0][1] = ir2wave->ppoint[point_number + 2].threshold2;
			corr[0][2] = ir2wave->ppoint[point_number + 2].threshold3;
			corr[0][3] = ir2wave->ppoint[point_number + 2].threshold4;
			corr[1][0] = ir2wave->ppoint[point_number + 3].threshold1;
			corr[1][1] = ir2wave->ppoint[point_number + 3].threshold2;
			corr[1][2] = ir2wave->ppoint[point_number + 3].threshold3;
			corr[1][3] = ir2wave->ppoint[point_number + 3].threshold4;
			corr[2][0] = ir2wave->ppoint[point_number + 4].threshold1;
			corr[2][1] = ir2wave->ppoint[point_number + 4].threshold2;
			corr[2][2] = ir2wave->ppoint[point_number + 4].threshold3;
			corr[2][3] = ir2wave->ppoint[point_number + 4].threshold4;
			corr[3][0] = ir2wave->ppoint[point_number + 5].threshold1;
			corr[3][1] = ir2wave->ppoint[point_number + 5].threshold2;
			corr[3][2] = ir2wave->ppoint[point_number + 5].threshold3;
			corr[3][3] = ir2wave->ppoint[point_number + 5].threshold4;

			for (y = coordinate[1]; y <= coordinate[3]; y++)
			{
				for (x = coordinate[0]; x <= coordinate[2]; x++)
				{
					for (yp = y * ir2wave->ir1_mask_ptn_diameter_y; yp < (y * ir2wave->ir1_mask_ptn_diameter_y) + ir2wave->ir1_mask_ptn_diameter_y; yp++)
					{
						for (xp = x * ir2wave->ir1_mask_ptn_diameter_x; xp < (x * ir2wave->ir1_mask_ptn_diameter_x) + ir2wave->ir1_mask_ptn_diameter_x; xp++)
						{
							// IR1
							spt.x = xp;
							spt.y = yp;
							spt.l_plane_tbl = (u8)plane_ir1;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							ir1his[spt.sensdt]++;

							// IR2
							spt.x = xp;
							spt.y = yp;
							spt.l_plane_tbl = (u8)plane_ir2;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							ir2his[spt.sensdt]++;

							xp++;
						}
					}
				}
			}

			ir1freqtemp = 0;
			ir2freqtemp = 0;
			ir1peak = 0;
			ir2peak = 0;
			for (ii = 1; ii < peak_search - 1; ii++)
			{
				movingave = (float)(ir1his[ii] + ir1his[ii - 1] + ir1his[ii + 1]) / 3;

				if (movingave > ir1freqtemp)
				{
					ir1freqtemp = movingave;
					ir1peak = ii;
				}

				movingave = (float)(ir2his[ii] + ir2his[ii - 1] + ir2his[ii + 1]) / 3;
				if (movingave > ir2freqtemp)
				{
					ir2freqtemp = movingave;
					ir2peak = ii;
				}

			}

			tmp1 = ir1peak;
			tmp2 = ir2peak;

			//中間情報
			ir2wave->ir1peak[cnt] = ir1peak;
			ir2wave->ir2peak[cnt] = ir2peak;
			ir2wave->ir1freq[cnt] = ir1his[ir1peak];
			ir2wave->ir2freq[cnt] = ir2his[ir2peak];

			// SikpaならIR2の方が値が高くなる。GSIならIR1の方が値が高くなるのでfeatureで入れ替え。
			if (feature == 1)
			{
				temp = tmp1;
				tmp1 = tmp2;
				tmp2 = temp;
			}

			/*tests1 = (u8)ir2wave->ppoint[tmp_plane].threshold1 + 1;
			tests2 = (u8)ir2wave->ppoint[tmp_plane].threshold2;
			float mid = (tests1 + tests2) / 2;
			float a;
			float b;
			if (mid > other_result)
			{
			a = 50 / (mid - tests1);
			b = 90 - a*mid;
			}
			else
			{
			a = 50 / (mid - tests2);
			b = 90 - a*mid;
			}

			level[point_number] = a*other_result + b;

			if (tmp1 > tmp2)
			{
			level[point_number] = 0;
			}

			if (level[point_number] <= 0)
			{
			level[point_number] = 1;
			}
			else if (level[point_number] > 90)
			{
			level[point_number] = 90;
			}*/

			ir1peak_standard = (ir1peak - ir1peak_ave) / ir1peak_std;
			ir2peak_standard = (ir2peak - ir2peak_ave) / ir2peak_std;
			ir1freq_standard = (ir1his[ir1peak] - ir1freq_ave) / ir1freq_std;
			ir2freq_standard = (ir2his[ir2peak] - ir2freq_ave) / ir2freq_std;
			standard[0] = ir1peak_standard;
			standard[1] = ir2peak_standard;
			standard[2] = ir1freq_standard;
			standard[3] = ir2freq_standard;
			dis = 0;

			for (ii = 0; ii < 4; ii++)
			{
				for (jj = 0; jj < 4; jj++)
				{
					dis += standard[ii] * standard[jj] * corr[ii][jj];
				}
			}

			level = sqrt(dis / 4);

			if (tmp1 <= tmp2)
			{
				ir2wave->output[cnt] = level_detect(&level, &thr_distance, 1, 0, thr_distance / 0.4);
			}
			else
			{
				ir2wave->output[cnt] = 1;
			}

#ifdef VS_DEBUG
			if (work[buf_num].pbs->blank3 != 0)
			{
				fprintf(fp, "%d,%d,%d,%d,%d,%f,%d,", feature,ir1peak, ir2peak, ir1his[ir1peak], ir2his[ir2peak], level, ir2wave->output[cnt]);//各ポイントのレベル
			}
#endif

			// 解放
			for (ii = 0; ii < 256; ii++)
			{
				ir1his[ii] = 0;
				ir2his[ii] = 0;
			}

		}//feature

		cnt++;

	}//for

	tmplevel = 100;
	tempcnt = 0;
	for (ii = 0; ii < cnt; ii++)
	{
		if (ir2wave->output[ii] < tmplevel && ir2wave->output[ii] != 0)
		{
			tmplevel = ir2wave->output[ii];
			tempcnt = ii;
		}
	}

	ir2wave->level = tmplevel;
	ir2wave->number = tempcnt;

#ifdef VS_DEBUG
	if (work[buf_num].pbs->blank3 != 0)
	{
		fprintf(fp, "%d,", ir2wave->level);//レベル出力
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif

	return 0;
}
