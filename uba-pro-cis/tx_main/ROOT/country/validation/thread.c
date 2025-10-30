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
/* �{�\�t�g�E�F�A�Ɋ܂܂��\�[�X�R�[�h�ɂ͓��{���K�@�B������ЌŗL��       */
/* ��Ƌ@�����܂�ł��܂��B                                               */
/* �閧�ێ��_�񖳂��Ƀ\�t�g�E�F�A�Ƃ����Ɋ܂܂����̑S�̂������͈ꕔ��   */
/* ���J���������s���܂���B                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
* MODEL NAME : ���ʋ��L
* @file thread.c
* @brief  �X���b�h���m�̎����t�@�C���ł��B
* @date Created. 2021/4/7
* @author JCM. OSAKA TECHNICAL RESARCH 1 GROUP ELEMENTAL TECHNOLOGY RECERCH DEPT.
* * https://app.box.com/s/lnmr049begov1u5kpansbljbfstavxvf
*/
/****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define EXT
#include "../common/global.h"

#ifdef VS_DEBUG
EXTERN	int debug_logi_view;	//�g���[�X���邩���Ȃ���
#endif

//#define THREAD_MASKCHECK

/*
	2021/4/7 ver2.0

	2021/5/17 ver2.1

	2021/6/13 ver2.2

	2021/12/2 ver2.3
		�s�v�ȕ����폜
		wid�̌^��ύX

	2022/6/7 ver2.4
		�g�p�Z���T���p�����^�ɕύX
*/
#ifdef NEW_THREAD
/****************************************************************/
/**
* @brief		thread.c ver2.2
*@param[in]		buf:�o�b�t�@�ԍ�
				thr:�X���b�h���m�\����
*@param[out]	�X���b�h���m����
* @return		0
*/
/****************************************************************/
s16 thread(u8 buf, ST_THREAD* thr)
{
	ST_VERTEX ver = { 0 };
	ST_THREAD_PARA para[THREAD_PARA_COUNT] = { 0 };
	u8 thread_num = 0;
	u8 plane;
	u8 ii;

	s16 err = 0;
	s16 res = 0;
	s16 x_start;
	s16 x_end;
	s16 y_start;
	s16 y_end;
	s16 xx_start;
	s16 xx_end;
	s16 yy_start;
	s16 yy_end;
	s16 margin = 0;
	float temp_level;
	float temp_thr = thr->standard_lack_rate;

	if (thr->t_plane == 0xff)
	{
		plane = UP_T_IR1;
		if (work[buf].pbs->PlaneInfo[plane].Enable_or_Disable == PLANE_DISABLED)
		{
			plane = DOWN_T_IR1;
		}
	}
	else
	{
		plane = thr->t_plane;
	}

	//�O�`����̃}�[�W���ݒ�
	margin = (s16)(thr->y_margin / work[buf].pbs->PlaneInfo[plane].main_element_pitch + 0.5);

	//�_�����W���_�擾
	get_vertex(&ver, buf);

	//���_�ݒ�
	y_start = ver.left_up_y - margin;
	y_end = ver.left_down_y + margin;
	x_start = thr->x_start;
	x_end = thr->x_end;

//#ifdef VS_DEBUG
//	//�͈͕\��
//	ii = 0;
//	if (debug_logi_view != 0)
//	{
//		if (work[buf].pbs->LEorSE == SE)
//		{
//			deb_para[ii++] = 2;
//			deb_para[ii++] = x_start + (((work[0].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_start;
//			deb_para[ii++] = x_end + (((work[0].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_start;
//			deb_para[ii++] = x_end + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_end;
//			deb_para[ii++] = x_start + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_end;
//			callback(deb_para);
//		}
//		else
//		{
//			deb_para[ii++] = 2;
//			deb_para[ii++] = x_start + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_start;
//			deb_para[ii++] = x_end + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_start;
//			deb_para[ii++] = x_end + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_end;
//			deb_para[ii++] = x_start + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_end;
//			callback(deb_para);
//		}
//	}
//
//#endif


	//�\���̏����l�ݒ�
	for (ii = 0; ii < THREAD_PARA_COUNT; ii++)
	{
		para[ii].x_start = THREAD_INI;
		para[ii].x_end = THREAD_INI;
	}


	//�X���b�h�̌������m
	thread_num = (u8)thread_search(thr, para, x_start, x_end, y_start, y_end, buf);

	//�X���b�h���m�{��
	thr->thread_num = thread_num;

	//�������m������I��
	if (thread_num > 1)
	{
		thr->result |= THREAD_NOT_ONE;
		thr->level = MAX_LEVEL;
		return 0;
	}

	//�X���b�h�����L�^
	thr->pre_percent = para[0].max_remain;

	//�X���b�h�����m�ł��Ȃ�������I��
	if (thr->pre_percent == 0)
	{
		thr->result |= THREAD_NOT_FIND;
		thr->level = MAX_LEVEL;
		return 0;
	}
	else if (thr->pre_percent > 100)
	{
		thr->pre_percent = 100;
	}

	/*if(para[thread_num].x_end - para[thread_num].x_start > thr->wid_limit)
	{
		thr->result |= THREAD_WIDTH_OVER;

		return 0;
	}*/

	//
	err = thread_detail_search(thr, para, y_start, y_end, thread_num, buf);
	//�X���b�h������0�A���S���W���o�Ȃ�
	if (err < 0)
	{
		thr->level = MAX_LEVEL;
		return 0;
	}

	//���C�X���b�h�Ȃ�
	if (thr->type == THREAD_TYPE_MAG)
	{
		yy_start = y_start;
		yy_end = y_end;
		xx_start = thr->lx_ave - thr->restart_margin;
		xx_end = thr->rx_ave + thr->restart_margin;

		err = thread_outline_cheak_setup(thr->logi_y_step, buf, &xx_start, xx_end, &yy_start, &yy_end);
		if (err < 0)
		{
			thr->err_code = THREAD_MAG_EFFECT_RANGE_ERR;
			thr->level = MAX_LEVEL;
			return 0;
		}


		//���C�`�F�b�N
		res = thread_mag_cheak(thr, para, yy_start, yy_end, xx_start, xx_end, buf);

		if (thr->res_mag_max < thr->mag_cheak)
		{
			thr->result |= THREAD_MAG_LESS;
			thr->level = MAX_LEVEL;
			return 0;
		}

	}
	else
	{
		//�����X���b�h�A�������͎��C���g�p���Ȃ��ꍇ


	}

	temp_level = 100.0f - thr->remain_percent;
	thr->level = level_detect(&temp_level, &temp_thr, 1, (float)thr->min_lack_rate, 100.0f);

	return 0;

}

/****************************************************************/
/**
* @brief		臒l�ȏ�̒����̃X���b�h���������邩���m����
*@param[in]		thr			�X���b�h�\����
				para		�X���b�h�p�����[�^�\����
				x_start		�T���J�nx���W
				x_end		�T���I��x���W
				y_start		�T���J�ny���W
				y_end		�T���I��y���W
				buf			�o�b�t�@�ԍ�

*@param[out]

* @return		thr_num �X���b�h�{��
*/
/****************************************************************/
s16 thread_search(ST_THREAD* thr, ST_THREAD_PARA* para,
					s16 x_start, s16 x_end, s16 y_start, s16 y_end, u8 buf)
{

	ST_THEAD_SEARCH_PARA tsp = { 0 };
	u8 plane;
	u8 dt = 0;
	u8 x_count = 0;
	s16 xx;
	s16 yy;
	s16 x_step = thr->x_step;
	s16 y_step = thr->y_step;
	u16 thread_percent[THRAED_MAIN_MEMO] = { 0 };
	u16 len = 0;
	u16 len_max = 0;
	s8 last = 0;
	u8 thr_num = 0;
	s16 thread_length;

	thread_length = (y_start - (y_end)) / y_step;

	//�v���[���ݒ�
	if (thr->t_plane == 0xff)
	{
		plane = UP_T_IR1;
		if (work[buf].pbs->PlaneInfo[plane].Enable_or_Disable == PLANE_DISABLED)
		{
			plane = DOWN_T_IR1;
		}
	}
	else
	{
		plane = thr->t_plane;
	}

	tsp.spt.l_plane_tbl = plane;
	tsp.spt.p_plane_tbl = plane;
	tsp.spt.way = work[buf].pbs->insertion_direction;

	//LE�ESE�Ń}�X�N�̕����ύX
	if (work[buf].pbs->LEorSE == LE)
	{
		tsp.filter_size_x = thr->mask_len;
		tsp.filter_size_y = 1;
	}
	else
	{
		tsp.filter_size_x = 1;
		tsp.filter_size_y = thr->mask_len;
	}

	//�}�X�N�̃T�C�Y
	tsp.end = (u8)(tsp.filter_size_x * tsp.filter_size_y);

	//
	for (xx = x_start; xx < x_end; xx += x_step)
	{
		len = 0;
		for (yy = y_start; yy > y_end; yy -= y_step)
		{
			dt = (u8)point_vicinity_cal_min(&tsp, xx, yy, buf);

			if (dt < thr->search_tir)
			{
				len++;

			}//if
		}//y

		//���m�����X���b�h�������L�^
		if (len == 0)
		{
			//�Ȃɂ����Ȃ�
		}
		else
		{
			//thread_percent[x_count] = (u16)((float)(len / thread_length) * 100.0f + 0.5f);
			thread_percent[x_count] = (u16)((float)len / thread_length * 100 + 0.5f);
		}

		//���m�����ł��X���b�h�������W���L�^����
		if (len_max < thread_percent[x_count])
		{
			len_max = thread_percent[x_count];
			para[0].max_remain = len_max;
			para[0].center = xx;
		}

		//�X���b�h�Ƃ݂Ȃ�臒l�ȏォ
		if (thread_percent[x_count] > thr->percent_cheak)
		{
			if (last == thr_num)
			{
				thr_num++;

				//�L�^����ȏ�Ȃ�
				if (thr_num >= THREAD_PARA_COUNT)
				{
					return last;
				}
			}

			//�J�n�ʒu
			if (para[thr_num].x_start == THREAD_INI)
			{
				para[thr_num].x_start = xx;
			}

			//�I���ʒu
			if (para[thr_num].x_end == THREAD_INI
				|| para[thr_num].x_end < xx)
			{
				para[thr_num].x_end = xx;
			}

			//�X���b�h�����ő�
			if (para[thr_num].max_remain <= thread_percent[x_count])
			{
				para[thr_num].max_remain = thread_percent[x_count];
				para[thr_num].center = xx;
			}
		}
		else
		{
			if (last != thr_num)
			{
				last = thr_num;
			}
		}

		x_count++;

	}//x



	return thr_num;

}///TIR

int thread_compare_int(const void* a, const void* b)
{
	//����
    return *(s16*)a - *(s16*)b;
}

s32 thread_outlier_cheak(s16* xx, s32 sum, s16 len, u8 outlier_count, u16 outlier_dev)
{
	u16 ii = 0;
	s32 ave;
	s16 mid;
	u8 count = 0;

	//�\�[�g
	qsort(xx, THRAED_SUB_MEMO, sizeof(s16), thread_compare_int);

	//�����l�����߂�
	mid = xx[len / 2];

	for (ii = 0; ii < len; ii++)
	{
		if (xx[ii] == THREAD_INI)
		{
			continue;
		}
		if ((xx[ii] - mid) * (xx[ii] - mid) > outlier_dev)
		{
			sum -= xx[ii];
			count++;
		}
	}

	if (len - count < outlier_count)
	{
		return THREAD_INI;
	}

	ave = sum / (len - count);

	return ave;
}

/****************************************************************/
/**
* @brief		臒l�ȏ�̒����̃X���b�h���������邩���m����
*@param[in]		thr			�X���b�h�\����
				para		�X���b�h�p�����[�^�\����
				x_start		�T���J�nx���W
				x_end		�T���I��x���W
				y_start		�T���J�ny���W
				y_end		�T���I��y���W
				buf			�o�b�t�@�ԍ�

*@param[out]

* @return		-1�F�X���b�h�Ȃ�
				0�F����
*/
/****************************************************************/
s16 thread_detail_search(ST_THREAD* thr, ST_THREAD_PARA* para, s16 y_start, s16 y_end, u8 thr_num, u8 buf)
{
	ST_SPOINT spt = { 0 };
	s16 logi_lx[THRAED_SUB_MEMO];
	s16 logi_rx[THRAED_SUB_MEMO];
	u16 lx_count = 0;
	u16 rx_count = 0;
	s16 yy;
	s16 xx;
	u8 dt;
	u16 ii;
	u16 lx_len = 0;
	u16 rx_len = 0;
	s32 lx_ave = 0;
	s32 rx_ave = 0;
	s32 lx_sum = 0;
	s32 rx_sum = 0;
	u8 plane;
	s16 x_start;
	s16 x_end;
	u8 y_step = thr->logi_y_step;
	u8 yy_step;
	u8 y_shift;
	u16 len = 0;
	u16 count = 0;
	s8 temp = 0;

	//�v���[���ݒ�
	if (thr->t_plane == 0xff)
	{
		plane = UP_T_IR1;
		if (work[buf].pbs->PlaneInfo[plane].Enable_or_Disable == PLANE_DISABLED)
		{
			plane = DOWN_T_IR1;
		}
	}
	else
	{
		plane = thr->t_plane;
	}

	spt.l_plane_tbl = plane;
	spt.p_plane_tbl = plane;
	spt.way = work[buf].pbs->insertion_direction;

	y_shift = y_step;
	yy_step = y_step * 2;

	//�����l
	for (ii = 0; ii < THRAED_SUB_MEMO; ii++)
	{
		logi_lx[ii] = THREAD_INI;
		logi_rx[ii] = THREAD_INI;
	}

	//
	if (thr_num == 0)
	{
		x_start = para[thr_num].center - thr->restart_margin;
		x_end = para[thr_num].center + thr->restart_margin;
	}
	else
	{
		x_start = para[thr_num].x_start - thr->restart_margin;
		x_end = para[thr_num].x_end + thr->restart_margin;
	}

	//����
	for (yy = y_start; yy > y_end; yy -= yy_step)
	{
		for (xx = x_start; xx < x_end; xx++)
		//for(xx = para[1].center - thr->mask_len; xx < para[1].center + thr->mask_len; xx++)
		{
			dt = (u8)thread_dt_get(&spt, xx, yy, buf);
			if (dt < thr->search_tir)
			{
				logi_lx[lx_count] = xx;
				lx_len++;
				lx_sum += xx;
				break;
			}
		}
		lx_count++;
	}

	//�E��
	for (yy = y_start + y_shift; yy > y_end + y_shift; yy -= yy_step)
	{
		for (xx = x_end; xx > x_start; xx--)
		//for(xx = para[1].center + thr->mask_len; xx > para[1].center - thr->mask_len; xx--)
		{
			dt = (u8)thread_dt_get(&spt, xx, yy, buf);
			if (dt < thr->search_tir)
			{
				logi_rx[rx_count] = xx;
				rx_len++;
				rx_sum += xx;
				break;
			}
		}
		rx_count++;
	}

	//�X���b�h���S���W
	lx_ave = thread_outlier_cheak(logi_lx, lx_sum, lx_len, thr->outlier_count, thr->outlier_dev);
	rx_ave = thread_outlier_cheak(logi_rx, rx_sum, rx_len, thr->outlier_count, thr->outlier_dev);

	//���S���W���o�Ȃ����
	if (lx_ave == THREAD_INI || rx_ave == THREAD_INI)
	{
		thr->remain_percent = 0;
		thr->result |= THREAD_NOT_FIND;
		thr->level = MAX_LEVEL;
		return -1;
	}

	//���ʋL�^
	//thr->thread_center = (float)(lx_ave + rx_ave) / 2 + 0.5;
	thr->thread_center = (lx_ave + rx_ave) / 2;
	thr->wid = (s8)(rx_ave - lx_ave);
	thr->lx_ave = lx_ave;
	thr->rx_ave = rx_ave;

	temp = thr->wid / 3;
	if (temp < 4)
	{
		temp = 4;
	}
	else if (temp > 32)
	{
		temp = 32;
	}

	//�X���b�h���S�������m�F����
	for (yy = y_start; yy > y_end; yy -= y_step)
	{
		for (xx = thr->thread_center - temp; xx <= thr->thread_center + temp; xx++)
		{
			//dt = thread_dt_get(&spt, thr->thread_center, yy, buf);
			dt = thread_dt_get(&spt, xx, yy, buf);
			if (dt < thr->tir_cheack)
			{
				len++;
				break;
			}
		}
		count++;
	}

	//�X���b�h����0�Ȃ�
	if (len == 0)
	{
		thr->remain_percent = 0;
		thr->result |= THREAD_NOT_FIND;
		thr->level = MAX_LEVEL;
		return -1;
	}

	//thr->remain_percent = (u8)((float)(len / count) * 100.0f);
	thr->remain_percent = (float)len / count * 100;
	if (thr->remain_percent > 100)
	{
		thr->remain_percent = 100;
	}

//#ifdef VS_DEBUG
//	ii = 0;
//	if (debug_logi_view != 0)
//	{
//		if (work[buf].pbs->LEorSE == SE)
//		{
//			deb_para[ii++] = 2;
//			deb_para[ii++] = lx_ave + (((work[0].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_start;
//			deb_para[ii++] = rx_ave + (((work[0].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_start;
//			deb_para[ii++] = rx_ave + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_end;
//			deb_para[ii++] = lx_ave + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
//			deb_para[ii++] = work[buf].note_param.main_eff_range / 2 - y_end;
//			callback(deb_para);
//		}
//		else
//		{
//			deb_para[ii++] = 2;
//			deb_para[ii++] = lx_ave + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_start;
//			deb_para[ii++] = rx_ave + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_start;
//			deb_para[ii++] = rx_ave + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_end;
//			deb_para[ii++] = lx_ave + work[buf].note_param.main_eff_range / 2;
//			deb_para[ii++] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - y_end;
//			callback(deb_para);
//		}
//	}
//
//#endif

	return 0;
}


/****************************************************************/
/**
* @brief		���C���������邩���m����
*@param[in]		thr			�X���b�h�\����
				para		�X���b�h�p�����[�^�\����
				x_start		�T���J�nx���W
				x_end		�T���I��x���W
				y_start		�T���J�ny���W
				y_end		�T���I��y���W
				buf			�o�b�t�@�ԍ�

*@param[out]

* @return		0
*/
/****************************************************************/
s16 thread_mag_cheak(ST_THREAD* thr, ST_THREAD_PARA* para, s16 y_start, s16 y_end, s16 x_start, s16 x_end, u8 buf)
{
	ST_SPOINT spt = { 0 };
	u8 x_count = 0;
	s16 xx;
	s16 yy;
	s16 dt;
	u8 x_step = thr->logi_x_step;
	u8 y_step = thr->logi_y_step;
	u8 count;
	float max = 0;
	float ave;

	//�v���[���ݒ�
	spt.l_plane_tbl = UP_MAG;
	spt.p_plane_tbl = UP_MAG;
	spt.way = work[buf].pbs->insertion_direction;

	//x_step = (1 / work[buf].note_param.main_dpi_cor[UP_MAG]) + 0.5;
	//y_step = (1 / work[buf].note_param.sub_dpi_cor[UP_MAG]) + 0.5;

	for (xx = x_start; xx < x_end; xx += x_step)
	{
		count = 0;
		ave = 0;
		for (yy = y_start; yy > y_end; yy -= y_step)
		{
			dt = thread_mag_get(&spt, xx, yy, buf);
			if (dt < THREAD_MAG_MID)
			{
				ave += THREAD_MAG_MID - dt;
			}
			else
			{
				ave += dt - THREAD_MAG_MID;
			}

			count++;

		}//y

		ave /= count;
		if (max < ave)
		{
			max = ave;
		}

	}//x

	thr->res_mag_max = max;

	return 0;

}

/****************************************************************/
/**
* @brief		���C�Z���T�l�����߂�i16bit�j
*@param[in]		spt�@�\����
				xx�@�_��x���W
				yy�@�_��y���W
				buf	�o�b�t�@�ԍ�

*@param[out]

* @return		16bit�@���C�Z���T�l
*/
/****************************************************************/
s16	thread_mag_get(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf)
{
	s16 err;

	spt->x = xx;
	spt->y = yy;
	err = new_L_Getdt_16bit_only(spt, work[buf].pbs, &work[buf].note_param);

	return spt->sensdt;
}

/****************************************************************/
/**
* @brief		�Z���T�l�����߂�
*@param[in]		spt�@�\����
				xx�@�_��x���W
				yy�@�_��y���W
				buf	�o�b�t�@�ԍ�
*@param[out]

* @return		spt�Ŏw�肵���v���[���̃Z���T�l
*/
/****************************************************************/
s16	thread_dt_get(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf)
{
	s16 err;

	spt->x = xx;
	spt->y = yy;
	err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);

	return spt->sensdt;
}

/****************************************************************/
/**
* @brief		�}�X�N�͈͂̍ŏ��Z���T�l�����߂�
*@param[in]		para		�X���b�h�p�����[�^�\����
				logi_x		�T���J�nx���W
				logi_y		�T���J�ny���W
				buf			�o�b�t�@�ԍ�

*@param[out]

* @return		min			���m�����ŏ��l
*/
/****************************************************************/
s16 point_vicinity_cal_min(ST_THEAD_SEARCH_PARA* tsp, s16 logi_x, s16 logi_y, u8 buf_num)
{

	ST_BS* pbs = work[buf_num].pbs;
	ST_SPOINT* spt = &tsp->spt;

	s16 y = 0;
	s16 x = 0;
	u8* ppixel;
	u8	masksize_x = (u8)tsp->filter_size_x;
	u8	masksize_y = (u8)tsp->filter_size_y;
	u8 min = 255;
	u8 ii;
	u8 end = tsp->end;
	u8 dts[THREAD_MASK_LIMIT];
	u8* pdts = dts;

	s16 phy_x;
	s16 phy_y;
	s16 err_code;
	u32 ofs;

	spt->trace_flg = 1;
	tsp->spt.x = logi_x;
	tsp->spt.y = logi_y;
	err_code = new_L2P_Coordinate(spt, work[buf_num].pbs, &work[buf_num].note_param);
	phy_x = (s16)spt->x - (tsp->filter_size_x >> 1);
	phy_y = (s16)spt->y - (tsp->filter_size_y >> 1);

#ifdef THREAD_MASKCHECK
	for (y = 0; y < masksize_y; y++)
	{
		for (x = 0; x < masksize_x; x++)
		{
				spt->x = phy_x + x;
				spt->y = phy_y + y;

			err_code = P_Getdt_8bit_only(spt, pbs);
			if (min > spt->sensdt)
				{
					//min = spt->sensdt;
				}
		}
	}

	//tsp->output = min;
	//min = 255;
#endif


	spt->x	= phy_x;
	spt->y	= phy_y;
	err_code = P_Getdt_8bit_only(spt, pbs);
	ppixel = spt->p;
	ofs		= spt->period;
	ofs		= ofs - masksize_x;

	for (y = 0; y < masksize_y; y++)
	{
		for (x = 0; x < masksize_x; x++)
		{
			*pdts++ = *ppixel++;
		}
		ppixel = ppixel + ofs;
	}

	pdts = dts;
	
	for (ii = 0; ii < end; ii++)
	{
		if (min > *pdts)
		{
			min = *pdts;
		}
		*pdts++;
	}

	tsp->output = min;

	return min;
}

/****************************************************************/
/**
* @brief		MAG���L���͈͓����`�F�b�N���鏀��
*@param[in]		skip
				buf			�o�b�t�@�ԍ�
				x_current	����x���W
				x_end		�T���I��x���W
				y_start		�T���J�ny���W
				y_end		�T���I��y���W

*@param[out]	x_current	�L�����W���Ɉړ���������x���W
				y_start		�L�����W���Ɉړ������T���J�ny���W
				y_end		�L�����W���Ɉړ������T���I��y���W

* @return		0	����I��
-1	�͈͑S�̂��L���͈͊O
*/
/****************************************************************/
s16 thread_outline_cheak_setup(u8 skip, u8 buf, s16* x_current, s16 x_end, s16* y_start, s16* y_end)
{
	ST_SPOINT spt = { 0 };

	u8 count = 0;
	s16 outline_flag = 0;
	s16 temp_y[2];
	s16 temp_x;
	s16 ystep_count[2] = { 0 };
	s16 xstep_count = 0;

	temp_y[0] = *y_start;
	temp_y[1] = *y_end;
	temp_x = *x_current;

	spt.way = work[buf].pbs->insertion_direction;
	spt.l_plane_tbl = OMOTE_MAG;
	spt.p_plane_tbl = UP_MAG;

	//0:start 1:end
	while (count < 2)
	{
		spt.x = temp_x + xstep_count;
		spt.y = temp_y[count] + ystep_count[count];
		outline_flag = thread_outline_cheak(&spt, buf);
		if (outline_flag < 0)
		{
			if (count == 0)
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
		else if (outline_flag > 0)
		{
			xstep_count += skip;
			if (temp_x + xstep_count >= x_end)
			{
				return -1;
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

/****************************************************************/
/**
* @brief		MAG���L���͈͓����`�F�b�N����
*@param[in]		spt	�\����
				buf	�o�b�t�@�ԍ�

*@param[out]	�Ȃ�

* @return		-1	y���L���͈͊O�@
				1	x���L���͈͊O�@
				0	�L���͈͓�
*/
/****************************************************************/
s16 thread_outline_cheak(ST_SPOINT* spt, u8 buf)
{
	s16 err = 0;

	err = new_L2P_Coordinate(spt, work[buf].pbs, &work[buf].note_param);

	// �L���������W�O�Ȃ��
	if ((spt->x < work[buf].pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_min)
		|| (spt->x > work[buf].pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_max))
	{
		if (work[buf].pbs->LEorSE == SE)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	else if ((spt->y < 0)
		|| (spt->y > work[buf].note_param.sub_eff_range[spt->l_plane_tbl]))
	{
		if (work[buf].pbs->LEorSE == SE)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}
#else	//ver1.36

/****************************************************************/
/**
* @brief		thread.c
*@param[in]		buf:�o�b�t�@�ԍ�
				thr:�X���b�h���m�\����
*@param[out]	�X���b�h���m����
* @return		0
*/
/****************************************************************/
s16 thread(u8 buf, ST_THREAD* thr)
{
	ST_VERTEX ver = { 0 };

	s16 err = 0;
	u16 ii;
	u8 lx_count = 0;
	u8 rx_count = 0;
	u8 restart = 0;
	s16 lx[THREAD_DEV] = { 0 };
	s16 rx[THREAD_DEV] = { 0 };
	s16 ly[THREAD_DEV];
	s16 ry[THREAD_DEV];
	s16 x_start;
	s16 x_end;
	s16 y_start;
	s16 y_end;
	s16 lx_ave = 0;
	s16 rx_ave = 0;
	s16 thread_center = 0;
	s16 margin = 0;
	//s16 lack_len = 0;
	s16 last_lx;
	s16 last_rx;
	u8 plane;
	s16 res1 = 0;
	s16 res2 = 0;

	plane = UP_T_IR1;
	if (work[buf].pbs->PlaneInfo[plane].Enable_or_Disable == PLANE_DISABLED)
	{
		plane = DOWN_T_IR1;
	}

	restart = (u8)(thr->restart_margin / work[buf].pbs->PlaneInfo[plane].main_element_pitch);
	margin = (s16)(thr->y_margin / work[buf].pbs->PlaneInfo[plane].main_element_pitch);

	//�_�����W���_�擾
	get_vertex(&ver, buf);

	y_start = ver.left_up_y - margin;
	y_end = ver.left_down_y + margin;
	x_start = thr->x_start;
	x_end = thr->x_end;

	last_lx = x_start + restart;
	last_rx = x_end - restart;

	//MAG���L���͈͓����`�F�b�N����
	if (thr->type == THREAD_TYPE_MAG)
	{
		err = thread_outline_cheak_setup(thr->y_step, buf, &x_start, x_end, &y_start, &y_end);
		if (err < 0)
		{
			thr->err_code = THREAD_MAG_SEARCH_FAILURE;
			thr->judge |= THREAD_MAG_SEARCH_FAILURE;
			thr->level = MAX_LEVEL;
			return 0;
		}
	}

	//�X���b�h�ʒu�T��
	for (ii = 0; ii < THREAD_DEV; ii++)
	{
		ly[ii] = y_start + (ii + 1) * (y_end - y_start) / THREAD_DEV;
		ry[ii] = ly[ii] - ((y_end - y_start) / (2 * THREAD_DEV));

		res1 = thread_xsearch(thr, buf, last_lx - restart, last_rx + restart, &ly[ii], &lx[ii]);

		res2 = thread_xsearch(thr, buf, last_rx + restart, last_lx - restart, &ry[ii], &rx[ii]);

		//�X���b�h���m����@���@�O��E�X���b�h���m�ʒu�����O�������m���Ă���ꍇ
		if (res1 == 0 && lx[ii] < last_rx)
		{
			last_lx = lx[ii];
		}
		//�X���b�h���m����@���@�O�񍶃X���b�h���m�ʒu�����O�������m���Ă���ꍇ
		if (res2 == 0 && rx[ii] > last_lx)
		{
			last_rx = rx[ii];
		}
	}

	//�X���b�h�ʒu���ς����߂�
	thr->left_dev = thread_xavecheck(lx, &lx_ave, &lx_count);
	thr->right_dev = thread_xavecheck(rx, &rx_ave, &rx_count);

	//���m�ʒu�΍����傫���@�܂������łȂ��ꍇ�A�I��
	if (thr->left_dev > thr->left_dev_thr || thr->right_dev > thr->right_dev_thr)
	{
		thr->judge |= THREAD_NOT_STRAIGHT;
		thr->level = MAX_LEVEL;
		return 0;
	}

	//�X���b�h�Ȃ�
	if (lx_count == 0 || rx_count == 0)
	{
		thr->judge |= THREAD_NOTHR;
		thr->level = MAX_LEVEL;
		return  0;
	}

	//�X���b�h���S���W
	thread_center = ((lx_ave + rx_ave) / 2);

	thr->thread_wid = rx_ave - lx_ave;
	thr->thread_center = thread_center;

	//�����̃`�F�b�N
	if (thr->type == THREAD_TYPE_MAG)
	{
		err = thread_magnetic_material_check(thread_center, ly, buf, thr->magnectic_check_dt);
		if (err > 0)
		{
			thr->judge |= THREAD_MAGNETIC;
			thr->level = MAX_LEVEL;
			return 0;
		}
	}

	//�X���b�h���S�̃Z���T�l�E���������߂�
	err = thread_ysearch(thr, buf, thread_center, ver.left_up_y - margin, ver.left_down_y + margin);

	if (thr->judge == 0)
	{
		if (thr->type == THREAD_TYPE_METAL)//���^���X���b�h�Ȃ�
		{
			//��������Ɣ��̂�TIR����͂��Ȃ�
		}
		else if (thr->type == THREAD_TYPE_MAG)//���C�X���b�h�Ȃ�
		{
			if (thr->res_mag_dev < thr->mag_dev_thr)
			{
				thr->judge |= THREAD_MAG_LESS_THRESHOLD;
				thr->level = MAX_LEVEL;
			}
		}
	}


	return 0;

}
/****************************************************************/
/**
* @brief		�X���b�h���S�̃Z���T�l�����߂�

*@param[in]		thr				�X���b�h���m�\����
				buf				�o�b�t�@�ԍ�
				thread_center	�X���b�h���Sx���W
				y_start			�T��y���W�ő���W
				y_end			�T��y���W�ŏ����W

*@param[out]	ave				TIR����
				mag_dev			MAG�W���΍�
				lack_len		��������(dot)
				lack_per_len	�����p�[�Z���e�[�W

* @return		1
*/
/****************************************************************/
s16 thread_ysearch(ST_THREAD* thr, u8 buf, s16 thread_center, s16 y_start, s16 y_end)
{
	u8 lack_thr = thr->search_lack_dt_thr;
	u8 y_step = thr->y_step;
	u8 plane;

	u16 ii;
	u16 lack_len = 0;
	u16 count = 0;
	s16 y_current;
	s16 temp_dt;
	float sum = 0;
	float ave;

	u8 magdt[256] = { 0 };
	u16 mag_sum = 0;
	u8 mag_temp = 0;
	float mag_ave = 0;
	float mag_var_sum = 0;
	float mag_var = 0;
	float mag_dev = 0;

	ST_SPOINT spt = { 0 };
	ST_SPOINT mag = { 0 };

	float setup_min_level;
	float setup_standart;
	float setup_level;

	plane = UP_T_IR1;
	if (work[buf].pbs->PlaneInfo[plane].Enable_or_Disable == PLANE_DISABLED)
	{
		plane = DOWN_T_IR1;
	}
	spt.l_plane_tbl = plane;
	spt.p_plane_tbl = plane;
	spt.way = work[buf].pbs->insertion_direction;

	mag.l_plane_tbl = UP_MAG;
	mag.p_plane_tbl = OMOTE_MAG;
	mag.way = work[buf].pbs->insertion_direction;

	for (y_current = y_start; y_current > y_end; y_current -= y_step)
	{
		//TIR
		temp_dt = thread_center_dt_check(&spt, thread_center, y_current, buf);
		sum += temp_dt;

		//MAG
		if (thr->type == THREAD_TYPE_MAG)
		{
			mag_temp = thread_center_magdt_check(&mag, thread_center, y_current, buf);

			magdt[mag_temp]++;
			mag_sum += mag_temp;
		}

		//����
		if (temp_dt > lack_thr)
		{
			lack_len++;

#ifdef VS_DEBUG
			if (debug_logi_view == 1)
			{
				if (work[buf].pbs->LEorSE == SE)
				{
					deb_para[0] = 3;
					deb_para[1] = thread_center + (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2;
					deb_para[2] = work[buf].note_param.main_eff_range / 2 - (y_current);
					callback(deb_para);
				}
				else
				{
					deb_para[0] = 3;
					deb_para[1] = thread_center + work[buf].note_param.main_eff_range / 2;
					deb_para[2] = (((work[buf].pbs->Blocksize / work[buf].pbs->PlaneInfo[0].sub_sampling_pitch) * work[buf].pbs->block_count)) / 2 - (y_current);
					callback(deb_para);
				}
			}
#endif
		}

		count++;

	}//y

	if (thr->type == THREAD_TYPE_MAG)
	{
		mag_ave = (float)mag_sum / count;

		for (ii = 0; ii < 256; ii++)
		{
			mag_var_sum += (ii - mag_ave) * (ii - mag_ave) * (magdt[ii]);
		}

		mag_var = mag_var_sum / count;
		mag_dev = (float)sqrt(mag_var);

		thr->res_mag_dev = mag_dev;
		thr->res_mag_ave = mag_ave;
	}

	ave = (float)sum / count;

	thr->res_tir_ave = ave;
	thr->res_lack_len = lack_len * y_step;

	//�������@100�Ō����Ȃ��iver1.32�܂�)
	//�������@100�őS����
	thr->lack_per_len = (u8)(((float)thr->res_lack_len / (y_start - y_end) * 100) + 0.5);

	setup_standart = thr->standard_lack_rate;
	setup_level = thr->lack_per_len;
	setup_min_level = thr->min_lack_rate;

	//���x������
	thr->level = level_detect(&setup_level, &setup_standart, 1, setup_min_level, 100);


	return 1;
}

/****************************************************************/
/**
* @brief		�X���b�h��T������
*@param[in]		thr			�X���b�h���m�\����
				buf			�o�b�t�@�ԍ�
				x_start		�T��x���W�J�n�ʒu
				x_end		�T��x���W�I���ʒu
				y_point		�X���b�hy���W
				x_point		�X���b�hx���W

*@param[out]	y_point		�X���b�hy���W
				x_point		�X���b�hx���W

* @return		1
*/
/****************************************************************/
s16 thread_xsearch(ST_THREAD* thr, u8 buf, s16 x_start, s16 x_end, s16* y_point, s16* x_point)
{
	ST_SPOINT spt = { 0 };
	ST_SPOINT diff = { 0 };
	ST_SPOINT mag = { 0 };

	u8 plane;
	u8 mag_temp = 0;
	s8 flag = 1; //���E�ϊ��t���O
	s8 cis_flag = -1;
	s8 mag_flag = -1;
	s8 x_step = thr->x_step;
	u16 ii;
	s16 tir_res = 0;
	s16 diff_res = 0;
	s16 mag_res = 0;
	s16 x_current;
	s16 y_current = *y_point;
	s16 diff_dt = 0;
	float mag_sum = 0;
	float cis_sum = 0;
	s8 temp[2] = { 1,2 };	//check

	plane = UP_T_IR1;
	//
	if (work[buf].pbs->PlaneInfo[plane].Enable_or_Disable == PLANE_DISABLED)
	{
		plane = DOWN_T_IR1;
	}

	spt.l_plane_tbl = plane;
	spt.p_plane_tbl = plane;
	spt.way = work[buf].pbs->insertion_direction;

	mag.l_plane_tbl = UP_MAG;
	mag.p_plane_tbl = OMOTE_MAG;
	mag.way = work[buf].pbs->insertion_direction;

	//�������g�p���邩
	if (thr->diff_plane >= 0)
	{
		diff.l_plane_tbl = (u8)thr->diff_plane;
		diff.p_plane_tbl = (u8)thr->diff_plane;
		diff.way = work[buf].pbs->insertion_direction;
	}

	//�E���獶�ւ̒T���̂Ƃ�
	if (x_start > x_end)
	{
		x_step = x_step * (-1);
		flag = -1;
		temp[0] = -1;
		temp[1] = -2;
	}

	//�T��
	for (x_current = x_start; x_current * flag <= x_end * flag; x_current += x_step)
	{
		//TIR
		tir_res = thread_dt_check(&spt, x_current, y_current, buf);

		//����
		if (thr->diff_plane >= 0)
		{
			diff_res = thread_dt_check(&diff, x_current, y_current, buf);
			diff_dt = diff_res - tir_res;
			if (diff_dt > thr->search_diffthr)
			{
				cis_flag = 1;
			}
			else
			{
				cis_flag = 0;
			}
		}
		else if (tir_res < thr->search_tirthr)
		{
			cis_flag = 1;
		}
		else
		{
			cis_flag = 0;
		}

		//MAG
		if (thr->type == THREAD_TYPE_MAG)
		{
			mag_res = thread_dt_check(&mag, x_current, y_current, buf);

			if (THREAD_MAG_OFFSET_DT - mag_res > 0)
			{
				mag_temp = (u8)(THREAD_MAG_OFFSET_DT - mag_res);
			}
			else
			{
				mag_temp = (u8)(mag_res - THREAD_MAG_OFFSET_DT);
			}

			if (mag_temp > thr->search_magthr)
			{
				mag_flag = 1;
			}
			else
			{
				mag_flag = 0;
			}
		}//MAG

		if (cis_flag != 0 && mag_flag != 0)
		{
			mag_sum = 0;
			cis_sum = 0;

			mag_sum += mag_temp;

			if (thr->diff_plane >= 0)
			{
				cis_sum += diff_dt;
			}
			else
			{
				cis_sum += tir_res;
			}

			//check 2dot��܂ŃZ���T�l���߂āA���ϒl��臒l�ȉ��ŃX���b�h����
			for (ii = 0; ii < 2; ii++)
			{
				tir_res = thread_dt_check(&spt, x_current + temp[ii], y_current, buf);

				if (thr->diff_plane >= 0)
				{
					diff_res = thread_dt_check(&diff, x_current + temp[ii], y_current, buf);

					diff_dt = diff_res - tir_res;
					cis_sum += diff_dt;
				}
				else
				{
					cis_sum += tir_res;
				}

				if (thr->type == THREAD_TYPE_MAG)
				{
					mag_res = thread_dt_check(&mag, x_current + temp[ii], y_current, buf);

					if (THREAD_MAG_OFFSET_DT - mag_res > 0)
					{
						mag_temp = (u8)(THREAD_MAG_OFFSET_DT - mag_res);
					}
					else
					{
						mag_temp = (u8)(mag_res - THREAD_MAG_OFFSET_DT);
					}

					mag_sum += mag_temp;
				}
			}//check

			//�����g�p�̏ꍇ
			if (thr->diff_plane >= 0)
			{
				//���ϒl��臒l�ȏォ
				if (cis_sum / 3 > thr->search_diffthr)
				{
					cis_flag = 1;
				}
				else
				{
					cis_flag = 0;
				}
			}
			else
			{
				//���ϒl��臒l�ȏォ
				if (cis_sum / 3 < thr->search_tirthr)
				{
					cis_flag = 1;
				}
				else
				{
					cis_flag = 0;
				}
			}

			if (thr->type == THREAD_TYPE_MAG)
			{
				//���ϒl��臒l�ȏォ
				if (mag_sum / 3 > thr->search_magthr)
				{
					mag_flag = 1;
				}
				else
				{
					mag_flag = 0;
				}
			}

			if (cis_flag != 0 && mag_flag != 0)
			{
				//�X���b�h����
				*x_point = x_current;
				return 0;
			}

		}//check
	}//x

	//�X���b�h�Ȃ�
	*y_point = THREAD_INI;
	*x_point = THREAD_INI;
	return 1;
}

/****************************************************************/
/**
* @brief		sort�p�֐��i�~���j
*@param[in]
*@param[out]
* @return
*/
/****************************************************************/

int thread_compare_int(const void* a, const void* b)
{
	return *(s16*)a - *(s16*)b;
}

/****************************************************************/
/**
* @brief		�X���b�h�̉E�Ӎ��ӂ̕���x���W�����߂�
*@param[in]		ex		���m���W�z��
				ave		�O��l�����O�������m���W���ϒl
				count	�O��l�����O�������m��

*@param[out]	ave		�O��l�����O�������m���W���ϒl
				count	�O��l�����O�������m��

* @return		�X���b�h�ʒu�W���΍�
*/
/****************************************************************/
float thread_xavecheck(s16* ex, s16* ave, u8* count)
{
	s8 ii;

	s8 excount = 0;
	s16 bottom[8] = { 0 };
	s16 top[8] = { 0 };
	u8 mid = 0;
	s16 mid_dt;
	u8 num = 0;
	u8 top_count = 0;
	u8 bottom_count = 0;
	float sum = 0;
	float dev = 0;
	float var = 0;

	//�\�[�g�~��
	qsort(ex, 32, sizeof(s16), thread_compare_int);

	//�X���b�h���m�������߂�
	for (ii = 0; ii < 32; ii++)
	{
		if (ex[ii] == 0x0fff)
		{
			break;
		}
		else
		{
			sum += ex[ii];
			excount = ii;
		}
	}

	//�����l�����߂�
	mid = (excount) / 2;
	mid_dt = ex[mid];

	//���m���ɉ����ď��O�������ς���
	if (excount > 15)
	{
		num = 8;
	}
	else if (excount > 8 && excount <= 15)
	{
		num = 4;
	}
	else if (excount > 3 && excount <= 8)
	{
		num = 1;
	}
	else //0 1 2 3
	{
		//���m����4�����ŃX���b�h�Ȃ�
		dev = -1;
		return dev;
	}

	//���m����x�����W�̒����l����΍��̑傫�����ɏ��O���Ă���
	for (ii = 0; ii < num; ii++)
	{
		bottom[ii] = (ex[excount - ii] - mid_dt);
		top[ii] = (ex[ii] - mid_dt);

		if (abs(top[top_count]) > abs(bottom[bottom_count]))
		{
			sum -= (top[top_count] + mid_dt);
			top_count++;
		}
		else
		{
			sum -= (bottom[bottom_count] + mid_dt);
			bottom_count++;
		}
	}

	//���ϒl�����߂�
	*ave = (s16)(sum / (excount - num + 1));

	//�W���΍������߂�
	for (ii = top_count; ii < excount - bottom_count; ii++)
	{
		var += (ex[ii] - *ave) * (ex[ii] - *ave);
	}

	*count = excount - num + 1;
	dev = (float)sqrt(var / (excount - num));

	return dev;
}

/****************************************************************/
/**
* @brief		�X���b�h�̃Z���T�l�����߂�
*@param[in]		spt	�\����
				xx	�_��x���W
				yy	�_��y���W
				buf	�o�b�t�@�ԍ�

*@param[out]	�Ȃ�

* @return		�Z���T�l
*/
/****************************************************************/
s16	thread_dt_check(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf)
{
	//s16 err;

	spt->x = xx;
	spt->y = yy;
	//err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	return spt->sensdt;
}
/****************************************************************/
/**
* @brief		�X���b�h����IR�̃Z���T�l�̂����Ƃ��Ⴂ�Z���T�l���o�͂���
*@param[in]		spt	�\����
				xx	�_��x���W
				yy	�_��y���W
				buf	�o�b�t�@�ԍ�

*@param[out]	�Ȃ�

* @return		�Z���T�l
*/
/****************************************************************/
u8	thread_center_dt_check(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf)
{
	//s16 err;
	s16 temp;

	spt->x = xx;
	spt->y = yy;
	//err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	temp = spt->sensdt;

	//
	spt->x = xx + THREAD_TILT_SKIP;
	spt->y = yy;
	//err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	if (spt->sensdt < temp)
	{
		temp = spt->sensdt;
	}

	spt->x = xx - THREAD_TILT_SKIP;
	spt->y = yy;
	//err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	if (spt->sensdt < temp)
	{
		temp = spt->sensdt;
	}

	return (u8)temp;
}

/****************************************************************/
/**
* @brief		�X���b�h�����Z���T�l�̂����Ƃ������Z���T�l���o�͂���
*@param[in]		spt		TIR�\����
				diff	�����g�p�\����
				xx		�_��x���W
				yy		�_��y���W
				buf		�o�b�t�@�ԍ�

*@param[out]	�Ȃ�

* @return		�Z���T�l
*/
/****************************************************************/
s16	thread_center_diffdt_check(ST_SPOINT* spt, ST_SPOINT* diff, s16 xx, s16 yy, u8 buf)
{
	//s16 err;
	s16 temp;

	spt->x = xx;
	spt->y = yy;
	//err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);

	diff->x = xx;
	diff->y = yy;
	//err = new_L_Getdt(diff, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);

	temp = diff->sensdt - spt->sensdt;

	//
	spt->x = xx + THREAD_TILT_SKIP;
	spt->y = yy;
	//err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	diff->x = xx + THREAD_TILT_SKIP;
	diff->y = yy;
	//err = new_L_Getdt(diff, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);

	if (diff->sensdt - spt->sensdt > temp)
	{
		temp = diff->sensdt - spt->sensdt;
	}

	spt->x = xx - THREAD_TILT_SKIP;
	spt->y = yy;
	//err = new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);

	diff->x = xx - THREAD_TILT_SKIP;
	diff->y = yy;
	//err = new_L_Getdt(diff, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(spt, work[buf].pbs, &work[buf].note_param);

	if (diff->sensdt - spt->sensdt > temp)
	{
		temp = diff->sensdt - spt->sensdt;
	}

	return temp;
}

/****************************************************************/
/**
* @brief		�X���b�h���C�Z���T�l�̂����Ƃ������Z���T�l���o�͂���
*@param[in]		mag	�\����
				xx	�_��x���W
				yy	�_��y���W
				buf	�o�b�t�@�ԍ�

*@param[out]	�Ȃ�

* @return		�Z���T�l
*/
/****************************************************************/
u8	thread_center_magdt_check(ST_SPOINT* mag, s16 xx, s16 yy, u8 buf)
{
	//s16 err;
	s16 mag_temp;
	s16 temp = 0;

	//
	mag->x = xx;
	mag->y = yy;
	//err = new_L_Getdt(mag, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(mag, work[buf].pbs, &work[buf].note_param);

	if (THREAD_MAG_OFFSET_DT - mag->sensdt > 0)
	{
		mag_temp = THREAD_MAG_OFFSET_DT - mag->sensdt;
	}
	else
	{
		mag_temp = -(THREAD_MAG_OFFSET_DT - mag->sensdt);
	}

	temp = mag_temp;

	//
	mag->x = xx - THREAD_TILT_SKIP;
	mag->y = yy;
	//err = new_L_Getdt(mag, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(mag, work[buf].pbs, &work[buf].note_param);
	if (THREAD_MAG_OFFSET_DT - mag->sensdt > 0)
	{
		mag_temp = THREAD_MAG_OFFSET_DT - mag->sensdt;
	}
	else
	{
		mag_temp = -(THREAD_MAG_OFFSET_DT - mag->sensdt);
	}

	if (mag_temp > temp)
	{
		temp = mag_temp;
	}

	//
	mag->x = xx + THREAD_TILT_SKIP;
	mag->y = yy;
	//err = new_L_Getdt(mag, work[buf].pbs, &work[buf].note_param);
	new_L_Getdt(mag, work[buf].pbs, &work[buf].note_param);
	if (THREAD_MAG_OFFSET_DT - mag->sensdt > 0)
	{
		mag_temp = THREAD_MAG_OFFSET_DT - mag->sensdt;
	}
	else
	{
		mag_temp = -(THREAD_MAG_OFFSET_DT - mag->sensdt);
	}

	if (mag_temp > temp)
	{
		temp = mag_temp;
	}

	return (u8)temp;
}

/****************************************************************/
/**
* @brief		MAG���L���͈͓����`�F�b�N���鏀��
*@param[in]		skip
				buf			�o�b�t�@�ԍ�
				x_current	����x���W
				x_end		�T���I��x���W
				y_start		�T���J�ny���W
				y_end		�T���I��y���W

*@param[out]	x_current	�L�����W���Ɉړ���������x���W
				y_start		�L�����W���Ɉړ������T���J�ny���W
				y_end		�L�����W���Ɉړ������T���I��y���W

* @return		0	����I��
				-1	�͈͑S�̂��L���͈͊O
*/
/****************************************************************/
s16 thread_outline_cheak_setup(u8 skip, u8 buf, s16* x_current, s16 x_end, s16* y_start, s16* y_end)
{
	ST_SPOINT spt = { 0 };

	u8 count = 0;
	s16 outline_flag = 0;
	s16 temp_y[2];
	s16 temp_x;
	s16 ystep_count[2] = { 0 };
	s16 xstep_count = 0;

	temp_y[0] = *y_start;
	temp_y[1] = *y_end;
	temp_x = *x_current;

	spt.way = work[buf].pbs->insertion_direction;
	spt.l_plane_tbl = OMOTE_MAG;
	spt.p_plane_tbl = UP_MAG;

	//0:start 1:end
	while (count < 2)
	{
		spt.x = temp_x + xstep_count;
		spt.y = temp_y[count] + ystep_count[count];
		outline_flag = thread_outline_cheak(&spt, buf);
		if (outline_flag < 0)
		{
			if (count == 0)
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
		else if (outline_flag > 0)
		{
			xstep_count += skip;
			if (temp_x + xstep_count >= x_end)
			{
				return -1;
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

/****************************************************************/
/**
* @brief		MAG���L���͈͓����`�F�b�N����
*@param[in]		spt	�\����
				buf	�o�b�t�@�ԍ�

*@param[out]	�Ȃ�

* @return		-1	y���L���͈͊O�@
				1	x���L���͈͊O�@
				0	�L���͈͓�>
*/
/****************************************************************/
s16 thread_outline_cheak(ST_SPOINT* spt, u8 buf)
{
	//s16 err = 0;

	//err = new_L2P_Coordinate(spt, work[buf].pbs, &work[buf].note_param);
	new_L2P_Coordinate(spt, work[buf].pbs, &work[buf].note_param);

	// �L���������W�O�Ȃ��
	if ((spt->x < work[buf].pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_min)
		|| (spt->x > work[buf].pbs->PlaneInfo[spt->l_plane_tbl].main_effective_range_max))
	{
		if (work[buf].pbs->LEorSE == SE)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	else if ((spt->y < 0)
		|| (spt->y > work[buf].note_param.sub_eff_range[spt->l_plane_tbl]))
	{
		if (work[buf].pbs->LEorSE == SE)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

/****************************************************************/
/**
* @brief		MAG���m�ӏ��̔���IR�Z���T�l���Ⴂ�����m����
*@param[in]		xx	�X���b�hx���W
				yy	�X���b�hy���W
				buf	�o�b�t�@�ԍ�

*@param[out]	�Ȃ�

* @return		1:��������
				0�F�㎥����
*/
/****************************************************************/
s16 thread_magnetic_material_check(s16 xx, s16* yy, u8 buf, u8 dt)
{
	ST_SPOINT uir;
	ST_SPOINT dir;

	u8 ii;
	u8 iii;
	u8 count = 0;
	u8 count2 = 0;
	s16 res;
	u16 uir_min;
	u16 dir_min;

	s8 tempx[THREAD_MAGNECTIC_CHECK_POINT] =
	{ THREAD_CHECK_SKIP_MINUS, 0, THREAD_CHECK_SKIP_PULS,
		THREAD_CHECK_SKIP_MINUS, 0, THREAD_CHECK_SKIP_PULS,
		THREAD_CHECK_SKIP_MINUS, 0, THREAD_CHECK_SKIP_PULS };

	s8 tempy[THREAD_MAGNECTIC_CHECK_POINT] =
	{ THREAD_CHECK_SKIP_PULS, THREAD_CHECK_SKIP_PULS, THREAD_CHECK_SKIP_PULS,
		0,	0,	0,
		THREAD_CHECK_SKIP_MINUS, THREAD_CHECK_SKIP_MINUS, THREAD_CHECK_SKIP_MINUS };

	s16 check = THREAD_INI;	//���m�Ȃ��̒l�@4095

	uir.l_plane_tbl = UP_R_IR1;
	uir.p_plane_tbl = OMOTE_R_IR1;
	uir.way = work[buf].pbs->insertion_direction;

	dir.l_plane_tbl = DOWN_R_IR1;
	dir.p_plane_tbl = URA_R_IR1;
	dir.way = work[buf].pbs->insertion_direction;

	for (ii = 0; ii < THREAD_DEV; ii++)
	{
		//���m�Ȃ��̒l�@4095
		uir_min = THREAD_INI;
		dir_min = THREAD_INI;

		if (yy[ii] == check)
		{
			//�Ȃɂ����Ȃ�
		}
		else
		{
			for (iii = 0; iii < THREAD_MAGNECTIC_CHECK_POINT; iii++)
			{
				res = thread_dt_check(&uir, xx + tempx[iii], yy[ii] + tempy[iii], buf);
				if (res < uir_min)
				{
					uir_min = res;
				}

				res = thread_dt_check(&dir, xx + tempx[iii], yy[ii] + tempy[iii], buf);
				if (res < dir_min)
				{
					dir_min = res;
				}
			}

			if ((uir_min <= dt)
				|| (dir_min <= dt))
			{
				//��������
				count++;
			}

			//
			count2++;
		}
	}

	//����Ԕ��������
	if (count == count2)
	{
		return 1;
	}

	return 0;
}
#endif // NEW_THREAD