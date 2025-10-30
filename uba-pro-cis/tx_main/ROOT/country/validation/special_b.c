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
* MODEL NAME : 識別共有
* @file special_b.c
* @brief 特殊Bセンサ検知の実装ファイルです。
* @date Created. 2021/2/
* @author JCM. OSAKA TECHNICAL RESARCH 1 GROUP ELEMENTAL TECHNOLOGY RECERCH DEPT.
*/
/****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define EXT
#include "../common/global.h"

#ifdef VS_DEBUG
EXTERN int debug_logi_view;
#endif

/*
	ver1.0 初回 HF検知

	2021/6/13 ver1.1

	2022/2/22　ver1.2

*/

/****************************************************************/
/**
* @brief		special_b.c
*@param[in]		buf	:	バッファ番号
				spa	:	特殊Bセンサ検知構造体
*@param[out]	特殊Bセンサ検知結果
* @return		0
*/
/****************************************************************/
s16 special_b(u8 buf, ST_SPECIAL_B* spb)
{
	typedef struct
	{
		s16 x;
		s16 y;

	}ST_XY;

	typedef struct
	{
		ST_XY *a;
		ST_XY *b;

	}ST_LINE;

	ST_SPOINT spt = {0};
	ST_SPOINT spt2 = {0};
	ST_VERTEX outline = {0};
	ST_VERTEX ver = {0};

	u8 buf_num = buf;
	s16 err = 0;
	u8 sp_num = 0;
	u8 sp_num_min = 0;
	u8 sp_num_max = SPB_NUM;
	float sub = 0;
	float pitch = 0;
	s16 ii;
	//u16 jj;
	u16 num = 0;
	s16 y_end,y_start,y_current;
	float upper,bottom,right,left,exclude_right,exclude_left;
	s16 dt = 0;
	u16 ch_x[SPB_NUM] = {SPB_0,SPB_1,SPB_2,SPB_3,SPB_4,SPB_5};
	u8 spb_ch[SPB_NUM] = {2,3,4,10,11,12};
	u16 py_start,py_end;
	//float ave;
	float total_sum = 0;
	u32 total_count = 0;
	//float total_ave = 0;
	float total_dev = 0;
	//float sum = 0;
	u16 his[SPB_HIS] = {0};
	s16 yyy[4] = {0};
	s16 xxx[4] = {0};
	ST_LINE *left_xy,*right_xy;
	ST_LINE aa,bb;
	ST_XY xy[4];

	spt.way = work[buf_num].pbs->insertion_direction;
	spt.l_plane_tbl = UP_SP_B;
	spt.p_plane_tbl = OMOTE_SP_B;
	spt.trace_flg = 1;

	sub = work[buf_num].note_param.sub_dpi_cor[UP_SP_B];
	pitch = work[buf_num].pbs->PlaneInfo[UP_SP_B].sub_sampling_pitch;

	//紙幣範囲設定
	outline.left_up_x = work[buf_num].pbs->left_up_x;
	outline.left_up_y = work[buf_num].pbs->left_up_y;
	outline.left_down_x = work[buf_num].pbs->left_down_x;
	outline.left_down_y = work[buf_num].pbs->left_down_y;
	outline.right_up_x = work[buf_num].pbs->right_up_x;
	outline.right_up_y = work[buf_num].pbs->right_up_y;
	outline.right_down_x = work[buf_num].pbs->right_down_x;
	outline.right_down_y = work[buf_num].pbs->right_down_y;

	left = (float)(outline.left_down_y - outline.left_up_y) / (outline.left_down_x - outline.left_up_x);
	right = (float)(outline.right_down_y - outline.right_up_y) / (outline.right_down_x - outline.right_up_x);
	upper = (float)(outline.left_up_y - outline.right_up_y) / (outline.left_up_x - outline.right_up_x);
	bottom = (float)(outline.left_down_y - outline.right_down_y) / (outline.left_down_x - outline.right_down_x);

	err = get_vertex(&ver,buf_num);

	//除外範囲設定
	yyy[0] = ver.left_up_y;
	yyy[1] = ver.left_down_y;
	yyy[2] = ver.left_up_y;
	yyy[3] = ver.left_down_y;
	xxx[0] = spb->xx[0];
	xxx[1] = spb->xx[0];
	xxx[2] = spb->xx[1];
	xxx[3] = spb->xx[1];
	
	spt2.way = work[buf_num].pbs->insertion_direction;
	spt2.l_plane_tbl = OMOTE_R_R;
	spt2.p_plane_tbl = OMOTE_R_R;
	
	for(ii = 0; ii < 4; ii++)
	{
		spt2.x = xxx[ii];
		spt2.y = yyy[ii];
		err = new_L2P_Coordinate(&spt2, work[buf].pbs, &work[buf].note_param);
		xy[ii].x = (s16)spt2.x;
		xy[ii].y = (s16)spt2.y;
	}
	

	if(xy[0].x < xy[1].x)
	{
		aa.a = &xy[0];
		aa.b = &xy[1];
	}
	else
	{
		aa.a = &xy[1];
		aa.b = &xy[0];
	}

	if(xy[2].x < xy[3].x)
	{
		bb.a = &xy[2];
		bb.b = &xy[3];
	}
	else
	{
		bb.a = &xy[3];
		bb.b = &xy[2];
	}

	if(aa.a->x < bb.a->x)
	{
		left_xy = &aa;
		right_xy = &bb;
	}
	else
	{
		left_xy = &bb;
		right_xy = &aa;
	}
#ifdef VS_DEBUG
	if(debug_logi_view != 0)
	{
		deb_para[0] = 2;
		deb_para[1] = left_xy->a->x;
		deb_para[2] = left_xy->a->y;
		deb_para[3] = left_xy->b->x;
		deb_para[4] = left_xy->b->y;
		deb_para[5] = right_xy->b->x;
		deb_para[6] = right_xy->b->y;
		deb_para[7] = right_xy->a->x;
		deb_para[8] = right_xy->a->y;
		callback(deb_para);
	}
#endif // DEBUG


	exclude_left = (float)(left_xy->b->y - left_xy->a->y) / (left_xy->b->x - left_xy->a->x);
	exclude_right = (float)(right_xy->b->y - right_xy->a->y) / (right_xy->b->x - right_xy->a->x);


	for(sp_num = sp_num_min; sp_num < sp_num_max; sp_num++)
	{
		y_start = 0;
		y_end = 0;
		num = sp_num - sp_num_min;

		if(ch_x[num] < outline.left_up_x && ch_x[num] < outline.left_down_x)
		{
			spb->ys[num] = SPB_LEFT_OVER;
			spb->ye[num] = SPB_LEFT_OVER;
			continue;
		}
		else if(ch_x[num] > outline.right_up_x && ch_x[num] > outline.right_down_x)
		{
			spb->ys[num] = SPB_RIGHT_OVER;
			spb->ye[num] = SPB_RIGHT_OVER;
			continue;
		}
		else if( ch_x[num] >= outline.left_down_x && ch_x[num] <= outline.left_up_x )
		{
			y_start = (s16)(left*(ch_x[num] - outline.left_down_x) + outline.left_down_y);
			y_end = (s16)(bottom*(ch_x[num] - outline.left_down_x) + outline.left_down_y);
		}
		else if(ch_x[num] <= outline.left_down_x && ch_x[num] >= outline.left_up_x)
		{
			y_start = (s16)(upper*(ch_x[num] - outline.left_up_x) + outline.left_up_y);
			y_end = (s16)(left*(ch_x[num] - outline.left_down_x) + outline.left_down_y);
		}
		else if( ch_x[num] >= outline.right_up_x && ch_x[num] <= outline.right_down_x )
		{
			y_start = (s16)(right*(ch_x[num] - outline.right_down_x) + outline.right_down_y);
			y_end = (s16)(bottom*(ch_x[num] - outline.right_down_x) + outline.right_down_y);
		}
		else if(ch_x[num] <= outline.right_up_x && ch_x[num] >= outline.right_down_x)
		{
			y_start = (s16)(upper*(ch_x[num] - outline.right_up_x) + outline.right_up_y);
			y_end = (s16)(right*(ch_x[num] - outline.right_down_x) + outline.right_down_y);
		}
		else if(ch_x[num] >= left_xy->b->x && ch_x[num] <= right_xy->a->x)
		{
			spb->ys[num] = SPB_AREA_IN;
			spb->ye[num] = SPB_AREA_IN;
			continue;
		}
		else if(ch_x[num] < left_xy->b->x && ch_x[num] >= left_xy->a->x)
		{
			//continue;
			y_start = (s16)(upper*(ch_x[num] - outline.left_up_x) + outline.left_up_y);
			y_end = (s16)(bottom*(ch_x[num] - outline.left_down_x) + outline.left_down_y);

			if(left_xy->a->y > left_xy->b->y)
			{
				y_end = (s16)(exclude_left*(ch_x[num] - left_xy->a->x) + left_xy->a->y);
			}
			else
			{
				y_start =  (s16)(exclude_left*(ch_x[num] - left_xy->a->x) + left_xy->a->y);
			}

		}
		else if(ch_x[num] < right_xy->b->x && ch_x[num] >= right_xy->a->x)
		{
			//continue;
			y_start = (s16)(upper*(ch_x[num] - outline.left_up_x) + outline.left_up_y);
			y_end = (s16)(bottom*(ch_x[num] - outline.left_down_x) + outline.left_down_y);

			if(right_xy->a->y > right_xy->b->y)
			{
				y_start = (s16)(exclude_right*(ch_x[num] - right_xy->a->x) + right_xy->a->y);
			}
			else
			{
				y_end = (s16)(exclude_right*(ch_x[num] - right_xy->a->x) + right_xy->a->y);
			}
		}
		else
		{
			y_start = (s16)(upper*(ch_x[num] - outline.left_up_x) + outline.left_up_y);
			y_end = (s16)(bottom*(ch_x[num] - outline.left_down_x) + outline.left_down_y);
		}

		py_start = (u16)(y_start * sub);
		py_end = (u16)(y_end * sub);

		spb->ys[num] = py_start;
		spb->ye[num] = py_end;

		if ((py_start < 0) ||
			(py_end > work[buf].note_param.sub_eff_range[UP_SP_B]))
		{
			spb->ys[num] = SPB_OUT_PIX;
			spb->ye[num] = SPB_OUT_PIX;
			continue;
		}

		for(y_current = py_start + spb->margin; y_current < py_end - spb->margin; y_current++)
		{
			spt.x = spb_ch[num];
			spt.y = y_current;
			err = P_Getdt_8bit_only(&spt, work[buf_num].pbs);

			dt = spt.sensdt;
			total_sum += dt;
			his[dt]++;
			total_count++;
#ifdef VS_DEBUG
			if (debug_logi_view != 0)
			{
				deb_para[0] = 8;
				deb_para[1] = dt;
				deb_para[2] = ch_x[num];
				deb_para[3] = y_current * pitch;
				callback(deb_para);
			}
#endif // DEBUG
			
		}//y
	}//x

	spb->total_ave = (float)total_sum / total_count;

	for(ii = 0; ii < SPB_HIS; ii++)
	{
		total_dev += (ii - spb->total_ave)*(ii - spb->total_ave)*his[ii];
	}
	
	spb->total_dev = sqrtf((float)total_dev / total_count);
	spb->predict = (spb->coef1 * spb->total_ave) + (spb->coef2 * spb->total_dev) + spb->intercept;

	if ((spb->coef1 * spb->total_ave) + (spb->coef2 * spb->total_dev) + spb->intercept < 0)
	{
		spb->predict = (spb->coef1 * spb->total_ave) + (spb->coef2 * spb->total_dev) + spb->intercept;
		spb->judge = SPB_OK;
	}
	else if (spb->total_dev < spb->thr && spb->thr != SPB_INI)
	{
		spb->judge = SPB_OK;
	}
	else
	{
		spb->judge = SPB_NG;
	}


	///revel

	return 0;
}
