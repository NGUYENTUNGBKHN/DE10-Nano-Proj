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
 * @file		neomag.c
 * @brief
 * @date		2020/10/12 Created.
 */
/****************************************************************************/
/*
	ver1.3
	パラメタ項目追加

	ver1.31
	個別デバッグ表示ができるように

	ver1.32 2020/10/12
	判定結果修正
*/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define EXT
#include "../common/global.h"

#ifdef VS_DEBUG
EXTERN	int debug_logi_view;	//トレースするかしないか
#endif

/****************************************************************/
/**
* @brief			
 *@param[in]	バッファ番号
 *@param[out]	なし
 * @return		なし
 */
/****************************************************************/
s16 neomag(u8 buf, ST_NEOMAG* neomag)
{
	s16 result = 0;
	s16 y_start;
	s16 y_end;
	s16 x_start;
	s16 x_end;
	//u8 num = neomag->num;
	u8 ii;

	for(ii = 0; ii < neomag->num; ii++)
	{
		y_start = neomag->para[ii].y2;
		y_end = neomag->para[ii].y1;
		x_start = neomag->para[ii].x1;
		x_end = neomag->para[ii].x2;

		result = neomag_search(neomag, buf, x_start, x_end, y_start, y_end, ii);
		if(result < 0)
		{
			//err
			neomag->judge = NEOMAG_ERR;
			return 0;
		}

	}//num

	//すべての範囲でNEOMAGありとなったか？
	for (ii = 0; ii < neomag->num; ii++)
	{
		if (neomag->result[ii] == 0)
		{
			neomag->judge |= NEOMAG_OK;
		}
		else
		{
			neomag->judge |= NEOMAG_NO;
		}
	}
	
#ifdef VS_DEBUG
	FILE* fp;
	if (work[buf].pbs->blank3 != 0)
	{
		fp = fopen("neomag.csv", "a");
		fprintf(fp, "%s,%x,%s,", work[buf].pbs->blank4, work[buf].pbs->mid_res_nn.result_jcm_id, work[buf].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf].pbs->blank0[20], work[buf].pbs->ser_num1, work[buf].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf].pbs->spec_code.model_calibration, work[buf].pbs->spec_code.sensor_conf, work[buf].pbs->spec_code.mode);
		fprintf(fp, "%d,", neomag->judge);
		fprintf(fp, "%d,", neomag->result[0]);
		fprintf(fp, "%f,", neomag->mag_dev[0]);
		fprintf(fp, "%f,", neomag->ir1_ave[0]);
		fprintf(fp, "%f,", neomag->ir2_ave[0]);
		fprintf(fp, "%d,", neomag->split_point[0]);
		fprintf(fp, "%d,", neomag->ir1_below[0]);
		fprintf(fp, "%d,", neomag->ir2_below[0]);
		fprintf(fp, "\n");
		fclose(fp);
	}

#endif

	return 0;
}

s16 neomag_search(ST_NEOMAG* neomag, u8 buf, s16 x_start, s16 x_end, s16 y_start, s16 y_end, u8 num)
{
	ST_SPOINT mag_spt = {0};
	ST_SPOINT ir1_spt = {0};
	ST_SPOINT ir2_spt = {0};

	u8 step = neomag->step;
	u8 skip = step * NEOMAG_SKIP;
	u8 split_mag_thr = neomag->split_mag_thr;
	u8 stain_ir1_thr = neomag->stain_ir1_thr;
	u8 stain_ir2_thr = neomag->stain_ir2_thr;
	u16 stain_point_thr =  neomag->stain_raito_thr;
	u8 tempx[NEOMAG_SPLIT] = {0};
	u8 tempy[NEOMAG_SPLIT] = {0};
	s16 err = 0;
	u16 ii = 0;
	u16 iii = 0;
	s16 flag = 0;
	u16 tempdt[NEOMAG_SPLIT];
	
	u16 count = 0;
	u16 split_point = 0;
	//u16 stain_point = 0;
	u16 mag_dt[256] ={0};
	float temp_sum = 0;
	float temp_var = 0;
	float temp_dev = 0;
	float temp_ave = 0;

	u16 ir1_below = 0;
	u16 ir2_below = 0;

	float ir1_sum = 0;
	float ir1_ave = 0;
	float ir2_sum = 0;
	float ir2_ave = 0;
	float mag_sum = 0;
	float mag_ave = 0;
	float mag_var = 0;
	float mag_var_sum = 0;
	float mag_dev = 0;

	s16 y_current = 0;
	s16 x_current = 0;
	s16 yy_start = 0;
	s16 yy_end = 0;
	s16 xx = 0;
	s16 yy = 0;
	u32 yd = 0;
	u32 _01bit = 0x01;

	u32 y_lines[NEOMAG_Y_LINE_BUFF] = {0};
	u32 y_line = 0;
	u32 y_line1 = 0;
	u32 y_line2 = 0;
	u32 y_test = 0;
	u32 split_check = 0;
	u32 check = 0;

	mag_spt.l_plane_tbl = UP_MAG;
	mag_spt.p_plane_tbl = OMOTE_MAG;
	mag_spt.way = work[buf].pbs->insertion_direction;

	ir1_spt.l_plane_tbl = UP_R_IR1;
	ir1_spt.p_plane_tbl = OMOTE_R_IR1;
	ir1_spt.way = work[buf].pbs->insertion_direction;

	ir2_spt.l_plane_tbl = UP_R_IR2;
	ir2_spt.p_plane_tbl = OMOTE_R_IR2;
	ir2_spt.way = work[buf].pbs->insertion_direction;

	tempx[0] = 0;
	tempx[1] = step;
	tempx[2] = step * 2;
	tempx[3] = 0;
	tempx[4] = step;
	tempx[5] = step * 2;
	tempx[6] = 0;
	tempx[7] = step;
	tempx[8] = step * 2;

	tempy[0] = 0;
	tempy[1] = 0;
	tempy[2] = 0;
	tempy[3] = step;
	tempy[4] = step;
	tempy[5] = step;
	tempy[6] = step * 2;
	tempy[7] = step * 2;
	tempy[8] = step * 2;

	for(ii = 0; ii < NEOMAG_SKIP; ii++)
	{
		split_check <<=  1;
		split_check |= _01bit;
	}

	for(x_current = x_start; x_current <= x_end; x_current += skip)
	{
		yy = -1;
		yy_start = y_start;
		yy_end = y_end;

		//MAGが有効範囲内か？
		flag = neomag_outline_cheak_setup(skip, buf, &x_current, x_end, &yy_start, &yy_end);

		if(flag == NEOMAG_X_FINISH)
		{
			neomag->err_code = ERR_NEOMAG_SEARCH_FAILURE;
			return -1;
		}

		for(y_current = yy_start; y_current > yy_end; y_current -= skip)
		{
			yy++;
			temp_sum = 0;
			temp_var = 0;

			for(ii = 0; ii < NEOMAG_SPLIT; ii++)
			{
				mag_spt.x = x_current + tempx[ii];
				mag_spt.y = y_current + tempy[ii];
				err = new_L_Getdt(&mag_spt, work[buf].pbs, &work[buf].note_param);
				if(err != 0)
				{
					neomag->err_code = ERR_NEOMAG_LGET_FAILURE;
					return -1;
				}

				mag_dt[mag_spt.sensdt]++;
				mag_sum += mag_spt.sensdt;
				temp_sum += mag_spt.sensdt;
				tempdt[ii] = mag_spt.sensdt;

				ir1_spt.x = x_current + tempx[ii];
				ir1_spt.y = y_current + tempy[ii];
				err = new_L_Getdt(&ir1_spt, work[buf].pbs, &work[buf].note_param);
				if(err != 0)
				{
					neomag->err_code = ERR_NEOMAG_LGET_FAILURE;
					return -1;
				}
				ir1_sum += ir1_spt.sensdt;

				if(ir1_spt.sensdt < stain_ir1_thr)
				{
					ir1_below++;
				}

				ir2_spt.x = x_current + tempx[ii];
				ir2_spt.y = y_current + tempy[ii];
				err = new_L_Getdt(&ir2_spt, work[buf].pbs, &work[buf].note_param);
				if(err != 0)
				{
					neomag->err_code = ERR_NEOMAG_LGET_FAILURE;
					return -1;
				}
				ir2_sum += ir2_spt.sensdt;

				if(ir2_spt.sensdt < stain_ir2_thr)
				{
					ir2_below++;
				}

				count++;
			}//temp

			temp_ave = temp_sum / NEOMAG_SPLIT;
			for(ii = 0; ii < NEOMAG_SPLIT; ii++)
			{
				temp_var += (tempdt[ii] - temp_ave) * (tempdt[ii] - temp_ave); 
			}

			temp_dev = sqrtf(temp_var / NEOMAG_SPLIT);
			

			if(temp_dev < split_mag_thr)
			{
				yd = _01bit << yy;
				y_lines[xx] |= yd;

#ifdef VS_DEBUG
				if (debug_logi_view == 1)
				{

					if (work[buf].pbs->LEorSE == SE)
					{
						deb_para[0] = 3;
						deb_para[1] = x_current + tempx[4] + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
						deb_para[2] = work[buf].note_param.main_eff_range / 2 - (y_current + tempy[4]);
						callback(deb_para);
					}
					else
					{
						deb_para[0] = 3;
						deb_para[1] = x_current + tempx[4] + work[buf].note_param.main_eff_range / 2;
						deb_para[2] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - (y_current + tempy[4]);
						callback(deb_para);
					}
				}
#endif
			}

		}//y

		xx++;

	}//x
	
	//MAG分割判定
	for(iii = 2; iii < xx; iii++)
	{
		y_line = y_lines[iii];
		y_line1 = y_lines[iii-1];
		y_line2 = y_lines[iii-2];

		y_line &= y_line1;
		y_line &= y_line2;

		check = split_check;
		for(ii = 0; ii < yy; ii++)
		{
			y_test = y_line & check;
			if(y_test == check)
			{
				split_point++;
			}
			check <<= 1;
		}
	}

	//
	ir1_ave = ir1_sum / count;
	ir2_ave = ir2_sum / count;
	mag_ave = mag_sum / count;
	for(ii = 0; ii < 256; ii++)
	{
		mag_var_sum += (ii - mag_ave) * (ii - mag_ave) * (mag_dt[ii]);
	}
	mag_var = mag_var_sum / count;
	mag_dev = sqrtf(mag_var);

	//判定値記録
	neomag->mag_dev[num] = mag_dev;
	neomag->ir1_ave[num] = ir1_ave;
	neomag->ir2_ave[num] = ir2_ave;
	neomag->split_point[num] = split_point;
	neomag->ir1_below[num] = ir1_below;
	neomag->ir2_below[num] = ir2_below;

	if(num == 0)//test
	{
		neomag->count = count;
	}

	//MAGゴミ、変造券判定
	if(split_point > neomag->split_point_thr)
	{
		neomag->result[num] |= NEOMAG_MAG_ABNORMALY;
	}
	//染み　汚れ判定
	if(ir1_below > (count * stain_point_thr / 100)
		|| ir2_below > (count * stain_point_thr / 100))
	{
		neomag->result[num] |= NEOMAG_IR_ABNORMALY;
	}

	//判定
	if(mag_dev < neomag->para[num].magthr)
	{
		neomag->result[num] |= NEOMAG_MAG_LESS_THRESHOLD;
	}

	if(ir1_ave < neomag->para[num].ir1thr)
	{
		neomag->result[num] |= NEOMAG_IR1_LESS_THRESHOLD;
	}

	if(ir2_ave < neomag->para[num].ir2thr)
	{
		neomag->result[num] |= NEOMAG_IR2_LESS_THRESHOLD;
	}

	return 1;

}

s16 neomag_outline_cheak_setup(u8 skip, u8 buf, s16* x_current, s16 x_end, s16* y_start, s16* y_end)
{
	ST_SPOINT spt = {0};
	
	u8 count = 0;
	s16 outline_flag = 0;
	s16 temp_y[2];
	s16 temp_x = 0;
	s16 ystep_count[2] = {0};
	s16 xstep_count = 0;
	
	temp_y[0] = *y_start - skip;
	temp_y[1] = *y_end + skip;
	temp_x = *x_current;

	spt.way = work[buf].pbs->insertion_direction;
	spt.l_plane_tbl = OMOTE_MAG;
	spt.p_plane_tbl = UP_MAG;

	//0:start 1:end
	while(count < 2)
	{
		spt.x = temp_x + xstep_count;
		spt.y = temp_y[count] + ystep_count[count];
		outline_flag = neomag_outline_cheak(&spt, buf);
		if(outline_flag < 0)
		{
			if(count == 0)
			{
				//y_start
				ystep_count[count] -= skip;
			}
			else
			{
				//y_end
				ystep_count[count] += skip;
			}
		}
		else if(outline_flag > 0)
		{
			xstep_count += skip;
			if(temp_x + xstep_count >= x_end)
			{
				return NEOMAG_X_FINISH;
			}
		}
		else
		{
			count++;
		}
	}

	*y_start = temp_y[0] + ystep_count[0];
	*y_end = temp_y[1] + ystep_count[1];
	*x_current = temp_x + xstep_count;

	return 0;
}

s16 neomag_outline_cheak(ST_SPOINT* spt, u8 buf)
{
	//s16 err = 0;

	//err = new_L2P_Coordinate(spt, work[buf].pbs, &work[buf].note_param);
	new_L2P_Coordinate(spt, work[buf].pbs, &work[buf].note_param);
	// 有効物理座標外ならば
	if ( (spt->x < work[buf].pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_min) 
		|| (spt->x > work[buf].pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_max) )
	{
		if(work[buf].pbs->LEorSE == SE)
		{
			return NEOMAG_Y_OUT;
		}
		else
		{
			return NEOMAG_X_OUT;
		}
	}
	else if((spt->y < 0) 
			|| (spt->y > work[buf].note_param.sub_eff_range[spt->l_plane_tbl]) )
	{
		if(work[buf].pbs->LEorSE == SE)
		{
			return NEOMAG_X_OUT;
		}
		else
		{
			return NEOMAG_Y_OUT;
		}
	}

	return 0;
}

