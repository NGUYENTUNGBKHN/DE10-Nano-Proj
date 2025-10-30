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
* @file		customcheck.c
* @brief	カスタムチェックの実装ファイル
* @date		2023/02/16　Created.
 * @Version	1.0.0
 * @updata	
*	https://app.box.com/s/bvql68hibyf8f5oncoktl4zrf6gvaptp
*/
/****************************************************************************/

#include <stdlib.h>

#define EXT
#include "../common/global.h"


#ifdef VS_DEBUG
EXTERN	int debug_logi_view;	//トレースするかしないか
#endif



enum CAL_NAME {

	UsedPoint,
	Color1,
	Color2,
	Color3,
};

/*
	2023/01/17 ver1.0 初版
	2023/02/16 ver1.1 解像度変更対応・紙幣外の選択防止
*/

//#define USED_MALLOC


/****************************************************************/
/**
* @brief		カスタムチェック ver1.0
*@param[in]		buf:バッファ番号
*@param[in]		CC:カスタムチェック検知構造体
*@param[out]
* @return		0
*/
/****************************************************************/
s16 customcheck(u8 buf, ST_CUSTOMCHECK_PARA* CC)
{
	ST_BS* pbs = work[buf].pbs;
	ST_NOTE_PARAMETER* pnote = &work[buf].note_param;
	s8 judmin = MIN_LEVEL;
	s32 result = 0;
	s16 level = 0;
	u8 typenum = 0;
	u8 sum_num = 0;
	u8 sum_area = 0;
	u8 judge_flag = 0;
	u8 type = 0;
	int i = 0;
	u32 ii = 0;
	u32 iii = 0;
	s16 xs = 0;
	s16 xe = 0;
	s16 ys = 0;
	s16 ye = 0;
	u32 sumplane[MAX_PLANE_COUNT] = { 0 };
	u8 plane_dt[MAX_PLANE_COUNT] = { 0 };
	u8 flag = 0;
	u32 pointcount = 0;
	u32 area = 0;
	u8 usedplane_num = 0;
	u8 num = 0;
	u8 xskip = CUSTOMCHECK_X_SKIP;
	u8 yskip = CUSTOMCHECK_Y_SKIP;
	s16 xx = 0;
	s16 yy = 0;
	s16 dt = 0;
	u8 pp = 0;
	u8 plane = 0;
	s8 usedplane[CUSTOMCHECK_USEDPLANE] = { 0 };
	unsigned char*** plane_matrix;
	unsigned char** y_matrix;
	unsigned char* x_matrix;
	u8 way = work[buf].pbs->insertion_direction;
	ST_VERTEX ver = { 0 };
	s16 lux;
	s16 luy;
	s16 rdx;
	s16 rdy;

#ifdef VS_DEBUG
	s16 temp[CUSTOMCHECK_PARA_NUM][CUSTOMCHECK_CALCULATION_NUM] = { 0 };
#endif

#ifndef USED_MALLOC
	ST_SPOINT spt;
#endif

	CC->level = judmin;
	way = work[buf].pbs->insertion_direction;

	get_vertex(&ver, buf);
	//外形内か判定
	//左上
	if (ver.left_up_x % xskip != 0)
	{
		lux = ver.left_up_x - (ver.left_up_x % xskip);//被除数で符号が決定
	}
	else
	{
		lux = ver.left_up_x;
	}
	if (ver.left_up_y % yskip != 0)
	{
		luy = ver.left_up_y - (ver.left_up_y % yskip);
	}
	else
	{
		luy = ver.left_up_y;
	}

	//右下
	if (ver.right_down_x % xskip != 0)
	{
		rdx = ver.right_down_x - (ver.right_down_x % xskip);
	}
	else
	{
		rdx = ver.right_down_x;
	}
	if (ver.right_down_y % yskip != 0)
	{
		rdy = ver.right_down_y  - (ver.right_down_y % yskip);
	}
	else
	{
		rdy = ver.right_down_y;
	}

	for (num = 0; num < CC->paranum; num++)
	{
		xs = CC->para[num].cca.startx;
		xe = CC->para[num].cca.endx;
		ys = CC->para[num].cca.starty;
		ye = CC->para[num].cca.endy;
		flag = 0;
		pointcount = 0;
		usedplane_num = 0;

		if (xs < lux)
		{
			xs = lux;
		}
		if (ys > luy)
		{
			ys = luy;
		}
		if (xe > rdx)
		{
			xe = rdx;
		}
		if (ye < rdy)
		{
			ye = rdy;
		}
		if ((xe - xs) % xskip != 0)
		{
			xe -= (xe - xs) % xskip;
		}
		if ((ys - ye) % yskip != 0)
		{
			ye -= (ys - ye) % yskip;
		}

		area = (xe - xs) / xskip * (ys - ye) /yskip;
		
		for (ii = 0; ii < MAX_PLANE_COUNT; ++ii)
		{
			sumplane[ii] = 0;
			plane_dt[ii] = 0;
		}

#ifdef USED_MALLOC

		for (ii = 0; ii < CUSTOMCHECK_USEDPLANE && CC->para[num].ccu[ii].plane != -1; ii++)
		{
			usedplane[ii] = CC->para[num].ccu[ii].plane;
			usedplane_num++;
		}

		 plane_matrix = (u8***)malloc_void(sizeof(unsigned char**) * usedplane_num);
		 y_matrix = (u8**)malloc_void(sizeof(unsigned char*) * usedplane_num * (ys - ye) / yskip);
		 x_matrix = (u8*)malloc_char(sizeof(unsigned char) * usedplane_num * (ys - ye) / yskip * (xe - xs) / xskip);

		 for (i = 0; i < (ys - ye) / yskip * usedplane_num; i++)
			 y_matrix[i] = (x_matrix + i * (xe - xs) / xskip);

		 for (i = 0; i < usedplane_num; i++)
			 plane_matrix[i] = (y_matrix + (i * (ys - ye) / yskip));

		 ocr_get_cutout_img(pnote,pbs, buf, x_matrix, xs, ys, (xe - xs), (ys - ye), usedplane, usedplane_num, xskip, yskip);
#else
		spt.way = way;
#endif
		
#ifndef USED_MALLOC
		for (xx = xs; xx < xe; xx += xskip)
#else
			for (xx = 0; xx < (xe - xs) / xskip; xx++)
#endif
			{

#ifndef USED_MALLOC
			for (yy = ys; yy > ye; yy -= yskip)
#else
			for (yy = 0; yy < (ys - ye) / yskip; yy++)
#endif
			{
				flag = 0;

				for (pp = 0; CC->para[num].ccu[pp].plane != -1; pp++)
				{
					plane = CC->para[num].ccu[pp].plane;
#ifndef USED_MALLOC
					
					spt.x = xx;
					spt.y = yy;
					spt.l_plane_tbl = plane;
					new_L_Getdt(&spt, work[buf].pbs, &work[buf].note_param);
					dt = spt.sensdt;
#else
					dt = plane_matrix[pp][yy][xx];
#endif
					plane_dt[plane] = dt;

					if (dt <= CC->para[num].ccu[pp].level)
					{
						flag = 1;
					}
				}
				if (flag == 1)
				{
					pointcount++;
					for (pp = 0; CC->para[num].ccu[pp].plane != -1; pp++)
					{
						sumplane[CC->para[num].ccu[pp].plane] += plane_dt[CC->para[num].ccu[pp].plane];
					}
				}
			}//yy
		}//xx

#ifdef USED_MALLOC
			free_proc(plane_matrix);
			free_proc(y_matrix);
			free_proc(x_matrix);
#endif		
		for (type = 0; CC->para[num].ccc[type].type != -1; type++)
		{
			judge_flag = 0;
			result = customcheck_calculation(sumplane, pointcount, CC->para[num].ccc[type].type, CC->para[num].ccc[type].usedplane, area);
			level = customcheck_judge(result, CC->para[num].ccc[type].max, CC->para[num].ccc[type].min, CC->para[num].ccc[type].margin);
			if (level < CUSTOMCHECK_CF_JUDGE_LEVEL)
			{
				judge_flag = 1;
				CC->judge[num] |= (u16)1 << type;
				sum_num++;
			}
			
			if (judmin > level)
			{
				judmin = level;
				CC->result_num = type;
				CC->result_area = num;
				CC->level = level;
				CC->value = result;
			}
#ifdef VS_DEBUG
			temp[num][type] = result;
#endif
		}

		if (judge_flag == 1)
		{
			sum_area++;
		}

	}//num

	CC->sum_area = sum_area;
	CC->sum_num = sum_num;

#ifdef VS_DEBUG
	FILE* fp;
	if (work[buf].pbs->blank3 != 0)
	{
		fp = fopen("customcheck.csv", "a");
		fprintf(fp, "%s,%x,%d,%s,", work[buf].pbs->blank4, work[buf].pbs->mid_res_nn.result_jcm_id, work[buf].pbs->insertion_direction, work[buf].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf].pbs->blank0[20], work[buf].pbs->ser_num1, work[buf].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf].pbs->spec_code.model_calibration, work[buf].pbs->spec_code.sensor_conf, work[buf].pbs->spec_code.mode);
		
		fprintf(fp, "%d,", CC->level);
		for (ii = 0; ii < CC->paranum; ii++)
		{
			for (iii = 0; CC->para[ii].ccc[iii].type != -1; iii++)
			{
				fprintf(fp, "%d,", temp[ii][iii]);
			}
			
		}
		
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif


	return 0;
}

/****************************************************************/
/**
* @brief		カスタムチェック判定　
* 
*@param[in]		result
*@param[in]		max
*@param[in]		min
*@param[in]		margin
* @return		level <40:CF >40:Gen
* @detail		max-minの範囲に収まっているか判定する
*				範囲外ならlevelが40以上になる				
*/
/****************************************************************/
u8 customcheck_judge(s32 result, s16 max, s16 min, s16 margin)
{
	float judge = 0;
	float ori = (int)((min + max) / 2.0f);
	float thr;
	u8 level;
	if (result > ori)
	{
		judge = result;
	}
	else if (result < ori)
	{
		judge = (ori - result) + ori;
	}
	else
	{
		level = 100;
		return level;
	}

	thr = max + margin;
	level = level_detect(&judge, &thr, 1, ori, thr + margin);

	return level;
}

/****************************************************************/
/**
* @brief		カスタムチェック計算
*@param[in]		sum
*@param[in]		usedpoint
*@param[in]		type
*@param[in]		usedplane
*@param[in]		area
*@param[out]
*@return		計算結果
*/
/****************************************************************/
s32 customcheck_calculation(u32* sum, u32 count, u8 type, s8* plane, u32 area)
{
	u32 p0 = 0;
	u32 p1 = 0;
	u32 p2 = 0;
	s32 res;
	if (count == 0)
	{
		return 0;
	}

	if (type == Color1)
	{
		p0 = sum[plane[0]] / count;

		return p0;
	}
	else if (type == Color2)
	{
		p0 = sum[plane[0]] / count;
		p1 = sum[plane[1]] / count;
		res = (p0 - p1) + CUSTOMCHECK_2CL_BIAS;

		return res;
	}
	else if (type == Color3)
	{
		p0 = sum[plane[0]] / count;
		p1 = sum[plane[1]] / count;
		p2 = sum[plane[2]] / count;

		return p0 * CUSTOMCHECK_3CL_RATIO / (p0 + p1 + p2);
	}
	else if (type == UsedPoint)
	{
		return 100 * count / area;
	}
	else
	{
		return 0;
	}
}
